/*
 * $Id: bcm-core-symbols.h 1.111.2.30 Broadcom SDK $
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
 */

#include <bcm/debug.h>
#include <bcm/types.h>
#include <bcm/async.h>
#include <bcm/auth.h>
#include <bcm/bcmi2c.h>
#include <bcm/cosq.h>
#include <bcm/custom.h>
#include <bcm/diffserv.h>
#include <bcm/dmux.h>
#include <bcm/error.h>
#include <bcm/field.h>
#include <bcm/filter.h>
#include <bcm/htls.h>
#include <bcm/igmp.h>
#include <bcm/init.h>
#include <bcm/ipmc.h>
#include <bcm/l2.h>
#include <bcm/l3.h>
#include <bcm/link.h>
#include <bcm/mcast.h>
#include <bcm/meter.h>
#include <bcm/mirror.h>
#include <bcm/module.h>
#include <bcm/mpls.h>
#include <bcm/pkt.h>
#include <bcm/port.h>
#include <bcm/proxy.h>
#include <bcm/rate.h>
#include <bcm/rx.h>
#include <bcm/stack.h>
#include <bcm/stat.h>
#include <bcm/stg.h>
#include <bcm/switch.h>
#include <bcm/topo.h>
#include <bcm/trunk.h>
#include <bcm/tx.h>
#include <bcm/vlan.h>
#include <bcm/ipfix.h>

#ifdef INCLUDE_MACSEC
#include <bcm_int/common/macsec_cmn.h>
#endif /* INCLUDE_MACSEC */

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
#include <bcm_int/esw/cosq.h>
#include <bcm_int/esw/diffserv.h>
#include <bcm_int/esw/qos.h>
#include <bcm_int/esw/failover.h>
#include <bcm_int/esw/ipfix.h>
#include <bcm_int/esw/ipmc.h>
#include <bcm_int/esw/l2.h>
#include <bcm_int/esw/l3.h>
#include <bcm_int/esw/link.h>
#include <bcm_int/esw/multicast.h>
#include <bcm_int/esw/mirror.h>
#include <bcm_int/esw/mpls.h>
#include <bcm_int/esw/mcast.h>
#include <bcm_int/esw/niv.h>
#include <bcm_int/esw/oam.h>
#include <bcm_int/esw/port.h>
#include <bcm_int/esw/stat.h>
#include <bcm_int/esw/stg.h>
#include <bcm_int/esw/stack.h>
#include <bcm_int/esw/subport.h>
#include <bcm_int/esw/time.h>
#include <bcm_int/esw/trill.h>
#include <bcm_int/esw/triumph.h>
#include <bcm_int/esw/triumph2.h>
#include <bcm_int/esw/trunk.h>
#include <bcm_int/esw/trx.h>
#include <bcm_int/esw/vlan.h>
#ifdef BCM_SBX_SUPPORT
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/fabric.h>
#include <bcm_int/sbx/port.h>
#endif /* BCM_SBX_SUPPORT */
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */ 

#ifdef INCLUDE_PTP
#ifdef BCM_ESW_SUPPORT
#include <bcm_int/esw/ptp.h>
#endif
#endif

#ifdef BCM_WARM_BOOT_SUPPORT
#include <soc/scache.h>
#endif /* BCM_WARM_BOOT_SUPPORT */

#include <sal/core/dpc.h>
#include <appl/diag/system.h>

int kconfig_get_next(char **name, char **value);
char *kconfig_get(const char *name);
int kconfig_set(char *name, char *value);
char *strtok(char *s, const char *delim) { return strsep(&s,delim); }

/* Export all BCM API symbols */
#include <bcm_export.h>

#include <soc/dport.h>

EXPORT_SYMBOL(soc_dport_to_port);
EXPORT_SYMBOL(soc_dport_from_port);
EXPORT_SYMBOL(soc_dport_from_dport_idx);

#include <bcm_int/api_xlate_port.h>

#ifdef INCLUDE_BCM_API_XLATE_PORT

EXPORT_SYMBOL(_bcm_api_xlate_port_a2p);
EXPORT_SYMBOL(_bcm_api_xlate_port_p2a);
EXPORT_SYMBOL(_bcm_api_xlate_port_pbmp_a2p);
EXPORT_SYMBOL(_bcm_api_xlate_port_pbmp_p2a);

#endif /* INCLUDE_BCM_API_XLATE_PORT */

#ifdef BCM_ESW_SUPPORT
#include <soc/triumph.h>
#include <soc/trident.h>
#include <soc/easyrider.h>
#include <soc/katana.h>
#include <soc/i2c.h>
#include <soc/hercules.h>
#include <soc/l3x.h>
#include <soc/xaui.h>
#include <soc/mcm/driver.h>
#include <soc/er_cmdmem.h>
#include <soc/higig.h>
#include <soc/phyctrl.h>
#ifdef BCM_CMICM_SUPPORT
#include <soc/mspi.h>
#include <soc/uc_msg.h>
#endif
#ifdef BCM_DDR3_SUPPORT
#include <soc/esw_ddr40.h>
#endif

extern int bcm_esw_ces_detach;
extern int bcm_esw_ces_rclock_status_get;
extern int soc_i2c_max127_iterations;
extern int _rlink_nexthop;
extern int  bcm_rlink_start(void);
extern int  bcm_rlink_stop(void);
extern int  bcm_rpc_start(void);
extern int  bcm_rpc_stop(void);
extern void bcm_rpc_dump(void);

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/strata.h>
#include <bcm_int/esw/draco.h>
#include <bcm_int/esw/firebolt.h>
#include <bcm_int/esw/easyrider.h>
#include <bcm_int/esw/triumph.h>
#include <bcm_int/esw/field.h>
#include <bcm_int/esw/ipfix.h>
#include <bcm_int/esw/port.h>
#include <bcm_int/esw/l2.h>
#ifdef BCM_KATANA_SUPPORT
#include <bcm_int/esw/flex_ctr.h>
#endif
#include <bcm_int/control.h>
extern int _rpc_nexthop;
#endif /* BCM_ESW_SUPPORT */ 

#ifdef BCM_SBX_SUPPORT
#include <bcm_int/control.h>
#ifdef BCM_SIRIUS_SUPPORT
#include <bcm_int/sbx_dispatch.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/sirius.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/fabric/sbZfFabBm9600BwAllocCfgBaseEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600BwAllocRateEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600BwFetchDataEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600BwFetchSumEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600BwFetchValidEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600BwR0BagEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600BwR0BwpEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600BwR0WdtEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600BwR1BagEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600BwR1Wct0AEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600BwR1Wct0BEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600BwR1Wct1AEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600BwR1Wct1BEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600BwR1Wct2AEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600BwR1Wct2BEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600BwR1WstEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600BwWredCfgBaseEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600BwWredDropNPart1EntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600BwWredDropNPart2EntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600FoLinkStateTableEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600InaEsetPriEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600InaHi1Selected_0EntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600InaHi1Selected_1EntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600InaHi2Selected_0EntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600InaHi2Selected_1EntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600InaHi3Selected_0EntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600InaHi3Selected_1EntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600InaHi4Selected_0EntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600InaHi4Selected_1EntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600InaPortPriEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600InaRandomNumGenEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600InaSysportMapEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600NmEgressRankerEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600NmEmtEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600NmEmtdebugbank0EntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600NmEmtdebugbank1EntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600NmFullStatusEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600NmIngressRankerEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600NmPortsetInfoEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600NmPortsetLinkEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600NmRandomNumGenEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600NmSysportArrayEntryConsole.hx>
#include <soc/sbx/fabric/sbZfFabBm9600XbXcfgRemapEntryConsole.hx>
#include <shared/idxres_afl.h>
#include <shared/idxres_fl.h>
#include <soc/sbx/bm9600.h>
#include <soc/sbx/bm9600_soc_init.h>
#include <soc/sbx/sirius_ddr23.h>
extern int
phy_reg_ci_read(int unit, uint32 ci, uint32 reg_addr, uint32 *data);
extern int
phy_reg_ci_write(int unit, uint32 ci, uint32 reg_addr, uint32 data);
extern int 
soc_reg_field_valid(int unit, soc_reg_t reg, soc_field_t field);
extern int 
soc_sbx_counter_start(int unit, uint32 flags, int interval, pbmp_t pbmp);
extern int 
soc_sbx_counter_sync(int unit);
extern int 
soc_sbx_counter_stop(int unit);
extern int 
soc_sbx_txrx_sync_rx(int unit, char *buf, int *buflen, int waitusec);
extern int 
soc_sbx_txrx_sync_tx(int unit, char *hdr, int hdrlen, char *buf,
                                int buflen, int waitusec);
extern sbreg thin_read32(sbhandle addr, uint32_t offs);
extern void thin_write32(sbhandle addr, uint32_t offs, sbreg data);
#endif /* def BCM_SIRIUS_SUPPORT */
#endif /* def BCM_SBX_SUPPORT */

#ifdef BCM_ROBO_SUPPORT
#include <soc/mcm/robo/driver.h>
extern int _bcm_robo_auth_sec_mode_set(int unit, bcm_port_t port, int mode,
                 int mac_num);
extern void 
_bcm_robo_l2_from_arl(int unit, bcm_l2_addr_t *l2addr, l2_arl_sw_entry_t *arl_entry);
extern void _robo_field_qset_dump(char *prefix, bcm_field_qset_t qset, char* suffix);
#include <bcm_int/control.h>
#include <bcm_int/robo/field.h>
#include <bcm_int/robo/rx.h>
#include <bcm_int/robo/port.h>
#include <bcm_int/robo/trunk.h>
#include <bcm_int/robo/l2.h>
#include <bcm_int/robo/subport.h>
#include <soc/phy/phyctrl.h>
#include <soc/arl.h>
extern int soc_robo_arl_mode_set(int unit, int mode);
extern void bcm5324_trunk_patch_linkscan(int unit, soc_port_t port, bcm_port_info_t *info);
extern int _bcm_robo_trunk_gport_resolve(int unit, bcm_trunk_add_info_t * t_data);
extern int _robo_field_thread_stop(int unit);
extern int _bcm_robo_gport_resolve(int unit, bcm_gport_t gport, bcm_module_t *modid, bcm_port_t *port,
    bcm_trunk_t *trunk_id, int *id);
extern int _bcm_robo_l2_gport_parse(int unit, bcm_l2_addr_t *l2addr, _bcm_robo_l2_gport_params_t *params);
extern int _bcm_robo_modid_is_local(int unit, bcm_module_t modid, int *result);
extern int bcm_robo_port_phy_power_mode_set(int unit, bcm_port_t port, int mode);
extern int bcm_robo_port_phy_power_mode_get(int unit, bcm_port_t port, int *mode);
extern int _bcm_robo_pbmp_check(int unit, bcm_pbmp_t pbmp);
#endif /* BCM_ROBO_SUPPORT */


/* 
 * Export system-related symbols and symbols
 * required by the diag shell .
 * ESW & ROBO both defined
 */

#ifdef BROADCOM_DEBUG
EXPORT_SYMBOL(bcm_debug_print);
EXPORT_SYMBOL(bcm_debug_level);
EXPORT_SYMBOL(bcm_debug_names);
#endif /* BROADCOM_DEBUG */
EXPORT_SYMBOL(bcore_assert_set_default);
EXPORT_SYMBOL(bcore_debug_register);
EXPORT_SYMBOL(bcore_debug_unregister);
EXPORT_SYMBOL(bde);
EXPORT_SYMBOL(_build_date);
EXPORT_SYMBOL(_build_datestamp);
EXPORT_SYMBOL(_build_host);
EXPORT_SYMBOL(_build_release);
EXPORT_SYMBOL(_build_tree);
EXPORT_SYMBOL(_build_user);
EXPORT_SYMBOL(kconfig_get);
EXPORT_SYMBOL(kconfig_get_next);
EXPORT_SYMBOL(kconfig_set);
EXPORT_SYMBOL(phy_port_info);
EXPORT_SYMBOL(int_phy_ctrl);
EXPORT_SYMBOL(ext_phy_ctrl);
#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
EXPORT_SYMBOL(sal_alloc_resource_usage_get);
EXPORT_SYMBOL(sal_mutex_resource_usage_get);
EXPORT_SYMBOL(sal_sem_resource_usage_get);
EXPORT_SYMBOL(sal_thread_resource_usage_get);
#endif /* INCLUDE_BCM_SAL_PROFILE */
#endif /* BROADCOM_DEBUG */
EXPORT_SYMBOL(sal_alloc);
EXPORT_SYMBOL(sal_assert_set);
EXPORT_SYMBOL(sal_boot_flags_get);
EXPORT_SYMBOL(sal_boot_flags_set);
EXPORT_SYMBOL(sal_boot_script);
EXPORT_SYMBOL(sal_core_init);
EXPORT_SYMBOL(sal_ctoi);
EXPORT_SYMBOL(sal_dpc_term);
EXPORT_SYMBOL(sal_dpc_cancel);
EXPORT_SYMBOL(sal_dpc_time);
#ifdef sal_free
/* Remove remap from <shared/alloc.h> */
#undef sal_free
#endif
EXPORT_SYMBOL(sal_free);
EXPORT_SYMBOL(sal_free_safe);
EXPORT_SYMBOL(sal_int_context);
#if defined(memcpy)
EXPORT_SYMBOL(sal_memcpy_wrapper);
#endif
EXPORT_SYMBOL(sal_memcmp);
EXPORT_SYMBOL(sal_mutex_create);
EXPORT_SYMBOL(sal_mutex_destroy);
EXPORT_SYMBOL(sal_mutex_give);
EXPORT_SYMBOL(sal_mutex_take);
EXPORT_SYMBOL(sal_os_name);
EXPORT_SYMBOL(sal_sem_create);
EXPORT_SYMBOL(sal_sem_destroy);
EXPORT_SYMBOL(sal_sem_give);
EXPORT_SYMBOL(sal_sem_take);
EXPORT_SYMBOL(sal_sleep);
EXPORT_SYMBOL(sal_snprintf);
EXPORT_SYMBOL(sal_spl);
EXPORT_SYMBOL(sal_splhi);
EXPORT_SYMBOL(sal_sprintf);
EXPORT_SYMBOL(sal_strdup);
EXPORT_SYMBOL(sal_thread_create);
EXPORT_SYMBOL(sal_thread_destroy);
EXPORT_SYMBOL(sal_thread_exit);
EXPORT_SYMBOL(sal_thread_main_get);
EXPORT_SYMBOL(sal_thread_main_set);
EXPORT_SYMBOL(sal_thread_name);
EXPORT_SYMBOL(sal_thread_self);
EXPORT_SYMBOL(sal_time);
EXPORT_SYMBOL(sal_time_usecs);
EXPORT_SYMBOL(sal_udelay);
EXPORT_SYMBOL(sal_usleep);
EXPORT_SYMBOL(shr_avl_count);
EXPORT_SYMBOL(shr_avl_delete);
EXPORT_SYMBOL(shr_avl_delete_all);
EXPORT_SYMBOL(shr_avl_insert);
EXPORT_SYMBOL(shr_avl_lookup);
EXPORT_SYMBOL(shr_avl_traverse);
EXPORT_SYMBOL(_sal_assert);
EXPORT_SYMBOL(_shr_atof_exp10);
EXPORT_SYMBOL(_shr_crc32);
EXPORT_SYMBOL(_shr_ctoa);
EXPORT_SYMBOL(_shr_ctoi);
EXPORT_SYMBOL(_shr_errmsg);
EXPORT_SYMBOL(_shr_pbmp_decode);
EXPORT_SYMBOL(_shr_pbmp_format);
EXPORT_SYMBOL(_shr_popcount);
EXPORT_SYMBOL(_shr_swap32);
EXPORT_SYMBOL(_shr_swap16);
EXPORT_SYMBOL(_soc_mac_all_ones);
EXPORT_SYMBOL(_soc_mac_all_zeroes);
EXPORT_SYMBOL(soc_ntohs_load);
EXPORT_SYMBOL(soc_ntohl_load);
EXPORT_SYMBOL(soc_htons_store);
EXPORT_SYMBOL(soc_htonl_store);
EXPORT_SYMBOL(soc_property_get);
EXPORT_SYMBOL(soc_property_get_pbmp);
EXPORT_SYMBOL(soc_property_get_str);
EXPORT_SYMBOL(soc_property_port_get);
EXPORT_SYMBOL(soc_phy_addr_int_of_port);
EXPORT_SYMBOL(soc_phy_addr_of_port);
EXPORT_SYMBOL(soc_phy_an_timeout_get);
EXPORT_SYMBOL(soc_phy_id0reg_get);
EXPORT_SYMBOL(soc_phy_id1reg_get);
EXPORT_SYMBOL(soc_phy_is_c45_miim);
EXPORT_SYMBOL(soc_phy_name_get);
EXPORT_SYMBOL(soc_cm_debug);
EXPORT_SYMBOL(soc_cm_debug_names);
EXPORT_SYMBOL(soc_cm_device);
EXPORT_SYMBOL(soc_cm_display_known_devices);
EXPORT_SYMBOL(soc_cm_get_id);
EXPORT_SYMBOL(soc_cm_p2l);
EXPORT_SYMBOL(soc_cm_l2p);
EXPORT_SYMBOL(soc_cm_sflush);
EXPORT_SYMBOL(soc_cm_sinval);
EXPORT_SYMBOL(soc_cm_print);
EXPORT_SYMBOL(soc_cm_salloc);
EXPORT_SYMBOL(soc_cm_sfree);
EXPORT_SYMBOL(soc_cm_device_create_id);
EXPORT_SYMBOL(soc_cm_device_destroy);
EXPORT_SYMBOL(soc_cm_device_supported);
#ifdef BROADCOM_DEBUG
EXPORT_SYMBOL(soc_debug_level);
EXPORT_SYMBOL(soc_debug_names);
#endif /* BROADCOM_DEBUG */
EXPORT_SYMBOL(soc_dev_name);
EXPORT_SYMBOL(soc_block_names);
EXPORT_SYMBOL(soc_block_port_names);
EXPORT_SYMBOL(soc_block_name_lookup_ext); 
EXPORT_SYMBOL(soc_misc_init);
EXPORT_SYMBOL(soc_feature_name);
EXPORT_SYMBOL(soc_regtypenames);
EXPORT_SYMBOL(soc_control);
EXPORT_SYMBOL(soc_state);
#ifdef BCM_WARM_BOOT_SUPPORT
EXPORT_SYMBOL(soc_wb_mim_state);
EXPORT_SYMBOL(soc_shutdown);
EXPORT_SYMBOL(soc_stable_set);
EXPORT_SYMBOL(soc_stable_size_set);
EXPORT_SYMBOL(soc_scache_ptr_get);
EXPORT_SYMBOL(soc_stable_size_get);
#endif /* BCM_WARM_BOOT_SUPPORT */
EXPORT_SYMBOL(soc_miimc45_read);
EXPORT_SYMBOL(soc_miimc45_write);
EXPORT_SYMBOL(soc_chip_type_names);
EXPORT_SYMBOL(soc_all_ndev);
EXPORT_SYMBOL(soc_ndev);
EXPORT_SYMBOL(soc_attached);
EXPORT_SYMBOL(soc_timeout_check);
EXPORT_SYMBOL(soc_timeout_init);
EXPORT_SYMBOL(bcm_control);
EXPORT_SYMBOL(system_init);
EXPORT_SYMBOL(soc_miim_read);
EXPORT_SYMBOL(soc_miim_write);
EXPORT_SYMBOL(soc_mem_datamask_get);
EXPORT_SYMBOL(SOC_BLOCK_IN_LIST);
EXPORT_SYMBOL(SOC_BLOCK_IS_TYPE);
EXPORT_SYMBOL(SOC_BLOCK_IS_COMPOSITE);

#ifdef BCM_WARM_BOOT_SUPPORT
EXPORT_SYMBOL(_bcm_shutdown);
#endif
#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
#if defined(BCM_TRX_SUPPORT)
EXPORT_SYMBOL(_bcm_common_sw_dump);
EXPORT_SYMBOL(_bcm_tr_ext_lpm_sw_dump);
#endif
EXPORT_SYMBOL(_bcm_cosq_sw_dump);
EXPORT_SYMBOL(_bcm_ds_sw_dump);
EXPORT_SYMBOL(_bcm_esw_qos_sw_dump);
EXPORT_SYMBOL(_bcm_ipfix_sw_dump);
EXPORT_SYMBOL(_bcm_ipmc_sw_dump);
EXPORT_SYMBOL(_bcm_l2_sw_dump);
EXPORT_SYMBOL(_bcm_l3_sw_dump);
EXPORT_SYMBOL(_bcm_link_sw_dump);
EXPORT_SYMBOL(_bcm_mcast_sw_dump);
EXPORT_SYMBOL(_bcm_mirror_sw_dump);
EXPORT_SYMBOL(_bcm_mpls_sw_dump);
EXPORT_SYMBOL(_bcm_multicast_sw_dump);
EXPORT_SYMBOL(_bcm_niv_sw_dump);
EXPORT_SYMBOL(_bcm_oam_sw_dump);
EXPORT_SYMBOL(_bcm_port_sw_dump);
EXPORT_SYMBOL(_bcm_stat_sw_dump);
EXPORT_SYMBOL(_bcm_stg_sw_dump);
EXPORT_SYMBOL(_bcm_stk_sw_dump);
EXPORT_SYMBOL(_bcm_td_trill_sw_dump);
EXPORT_SYMBOL(_bcm_time_sw_dump);
EXPORT_SYMBOL(_bcm_tr2_failover_sw_dump);
EXPORT_SYMBOL(_bcm_tr2_subport_sw_dump);
EXPORT_SYMBOL(_bcm_tr_subport_sw_dump);
EXPORT_SYMBOL(_bcm_trunk_sw_dump);
EXPORT_SYMBOL(_bcm_vlan_sw_dump);
#if defined(BCM_QE2000_SUPPORT) || defined(BCM_BME3200) || defined(BCM_SIRIUS_SUPPORT)
EXPORT_SYMBOL(bcm_sbx_wb_cosq_sw_dump);
EXPORT_SYMBOL(bcm_sbx_wb_fabric_sw_dump);
EXPORT_SYMBOL(bcm_sbx_wb_port_sw_dump);
#endif /* defined(BCM_QE2000_SUPPORT) || defined(BCM_BME3200) || defined(BCM_SIRIUS_SUPPORT) */
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

/* 
 * Export system-related symbols and symbols
 * required by the diag shell .
 * ESW specific 
 */
 
#ifdef BCM_ESW_SUPPORT
#ifdef INCLUDE_PTP
EXPORT_SYMBOL(_bcm_esw_ptp_log_level_set);
EXPORT_SYMBOL(_bcm_esw_ptp_info);
EXPORT_SYMBOL(_bcm_esw_ptp_unit_array);
EXPORT_SYMBOL(_bcm_esw_ptp_input_channels_get);
#endif
EXPORT_SYMBOL(bcm_xgs3_l3_info_dump);
#ifdef BCM_KATANA_SUPPORT
EXPORT_SYMBOL(bcm_esw_stat_group_dump_all);
EXPORT_SYMBOL(bcm_esw_stat_group_dump);
#endif
#ifdef BCM_RPC_SUPPORT
EXPORT_SYMBOL(bcm_rlink_stop);
EXPORT_SYMBOL(bcm_rlink_start);
EXPORT_SYMBOL(bcm_rpc_dump);
EXPORT_SYMBOL(bcm_rpc_start);
EXPORT_SYMBOL(bcm_rpc_stop);
#endif /* BCM_RPC_SUPPORT */
EXPORT_SYMBOL(soc_lynx_l3_ecmp_hash);
EXPORT_SYMBOL(soc_mem_field_valid);

#ifdef  BCM_EASYRIDER_SUPPORT
EXPORT_SYMBOL(_bcm_er_l2_from_l2x);
#endif  /* BCM_EASYRIDER_SUPPORT */
EXPORT_SYMBOL(_bcm_fb_l2_from_l2x);

EXPORT_SYMBOL(_bcm_esw_gport_resolve);
EXPORT_SYMBOL(_bcm_esw_gport_construct); 
EXPORT_SYMBOL(_bcm_esw_modid_is_local); 
EXPORT_SYMBOL(_bcm_esw_l2_gport_parse); 
EXPORT_SYMBOL(_bcm_gport_modport_hw2api_map); 
#ifdef BCM_FIELD_SUPPORT
EXPORT_SYMBOL(_bcm_field_setup_post_ethertype_udf);
#ifdef BROADCOM_DEBUG
EXPORT_SYMBOL(_field_qset_dump);
#endif /* BROADCOM_DEBUG */
EXPORT_SYMBOL(_field_control_get);
EXPORT_SYMBOL(_field_stage_control_get);
EXPORT_SYMBOL(_field_qset_is_subset);
#endif  /* BCM_FIELD_SUPPORT */
EXPORT_SYMBOL(_bcm_l2_from_l2x);
#if defined(BCM_TRIUMPH_SUPPORT) || defined(BCM_SCORPION_SUPPORT)
EXPORT_SYMBOL(_bcm_tr_l2_from_l2x);
#endif /*BCM_TRIUMPH_SUPPORT || BCM_SCORPION_SUPPORT */
EXPORT_SYMBOL(bcm_ipfix_dump_export_entry);
EXPORT_SYMBOL(bcm_ipfix_export_fifo_control);
#ifdef BCM_RPC_SUPPORT
EXPORT_SYMBOL(_rlink_nexthop);
EXPORT_SYMBOL(_rpc_nexthop);
#endif  /* BCM_RPC_SUPPORT */
#ifdef  BCM_XGS_SWITCH_SUPPORT
EXPORT_SYMBOL(_soc_mem_cmp_l3x_sync);
#endif  /* BCM_XGS_SWITCH_SUPPORT */
#ifdef BCM_RPC_SUPPORT
EXPORT_SYMBOL(cpudb_key_format);
EXPORT_SYMBOL(cpudb_key_parse);
#endif  /* BCM_RPC_SUPPORT */
EXPORT_SYMBOL(mbcm_driver);
EXPORT_SYMBOL(soc_anyreg_read);
EXPORT_SYMBOL(soc_anyreg_write);
EXPORT_SYMBOL(soc_base_driver_table);
EXPORT_SYMBOL(soc_bist);
EXPORT_SYMBOL(soc_chip_dump);
EXPORT_SYMBOL(soc_counter_get);
EXPORT_SYMBOL(soc_counter_get_rate);
EXPORT_SYMBOL(soc_counter_idx_get);
EXPORT_SYMBOL(soc_counter_set32_by_port);
EXPORT_SYMBOL(soc_counter_start);
EXPORT_SYMBOL(soc_counter_stop);
EXPORT_SYMBOL(soc_counter_sync);
#ifdef BCM_DDR3_SUPPORT
EXPORT_SYMBOL(soc_ddr_test);
EXPORT_SYMBOL(soc_ddr40_phy_reg_ci_modify);
EXPORT_SYMBOL(soc_ddr40_phy_reg_ci_read);
EXPORT_SYMBOL(soc_ddr40_phy_reg_ci_write);
EXPORT_SYMBOL(soc_ddr40_read);
EXPORT_SYMBOL(soc_ddr40_shmoo_get_init_step);
EXPORT_SYMBOL(soc_ddr40_shmoo_do);
EXPORT_SYMBOL(soc_ddr40_shmoo_calib);
EXPORT_SYMBOL(soc_ddr40_shmoo_plot);
EXPORT_SYMBOL(soc_ddr40_shmoo_set_new_step);
EXPORT_SYMBOL(soc_ddr40_write);
#endif
EXPORT_SYMBOL(soc_dma_abort_dv);
EXPORT_SYMBOL(soc_dma_chan_config);
EXPORT_SYMBOL(soc_dma_desc_add);
EXPORT_SYMBOL(soc_dma_desc_end_packet);
EXPORT_SYMBOL(soc_dma_dump_dv);
EXPORT_SYMBOL(soc_dma_dump_pkt);
EXPORT_SYMBOL(soc_dma_dv_alloc);
EXPORT_SYMBOL(soc_dma_dv_free);
EXPORT_SYMBOL(soc_dma_dv_reset);
EXPORT_SYMBOL(soc_dma_ether_dump);
EXPORT_SYMBOL(soc_dma_higig_dump);
EXPORT_SYMBOL(soc_dma_init);
EXPORT_SYMBOL(soc_dma_rom_dcb_alloc);
EXPORT_SYMBOL(soc_dma_rom_dcb_free);
EXPORT_SYMBOL(soc_dma_rom_detach);
EXPORT_SYMBOL(soc_dma_rom_init);
EXPORT_SYMBOL(soc_dma_rom_rx_poll);
EXPORT_SYMBOL(soc_dma_rom_tx_poll);
EXPORT_SYMBOL(soc_dma_rom_tx_start);
EXPORT_SYMBOL(soc_dma_start);
EXPORT_SYMBOL(soc_dma_wait);
EXPORT_SYMBOL(soc_draco_hash_get);
EXPORT_SYMBOL(soc_draco_hash_set);
EXPORT_SYMBOL(soc_draco_l2_hash);
EXPORT_SYMBOL(soc_draco_l2x_param_to_key);
EXPORT_SYMBOL(soc_draco_l3_ecmp_hash);
EXPORT_SYMBOL(soc_draco_l3_hash);
EXPORT_SYMBOL(soc_draco_l3x_param_to_key);
#ifdef BROADCOM_DEBUG
EXPORT_SYMBOL(soc_dump);
#endif /* BROADCOM_DEBUG */
#ifdef  BCM_EASYRIDER_SUPPORT
EXPORT_SYMBOL(soc_easyrider_ext_sram_enable_set);
EXPORT_SYMBOL(soc_easyrider_ext_sram_op);
EXPORT_SYMBOL(soc_er_celldata_chunk_read);
EXPORT_SYMBOL(soc_er_celldata_chunk_to_slice);
EXPORT_SYMBOL(soc_er_celldata_chunk_write);
EXPORT_SYMBOL(soc_er_celldata_columns_to_slice);
EXPORT_SYMBOL(soc_er_celldata_slice_to_chunk);
EXPORT_SYMBOL(soc_er_celldata_slice_to_columns);
EXPORT_SYMBOL(soc_er_dual_dimensional_array_alloc);
EXPORT_SYMBOL(soc_er_dual_dimensional_array_free);
EXPORT_SYMBOL(soc_er_fp_ext_read);
EXPORT_SYMBOL(soc_er_fp_ext_write);
EXPORT_SYMBOL(soc_er_fp_tcam_ext_read);
EXPORT_SYMBOL(soc_er_fp_tcam_ext_write);
EXPORT_SYMBOL(soc_er_l2_hash);
EXPORT_SYMBOL(soc_er_l2_traverse);
EXPORT_SYMBOL(soc_er_l3v4_base_entry_to_key);
EXPORT_SYMBOL(soc_er_l3v4_hash);
EXPORT_SYMBOL(soc_er_l3v6_base_entry_to_key);
EXPORT_SYMBOL(soc_er_l3v6_hash);
#endif  /* BCM_EASYRIDER_SUPPORT */
#ifdef  BCM_FIREBOLT_SUPPORT
EXPORT_SYMBOL(soc_fb_l2_hash);
EXPORT_SYMBOL(soc_fb_l2x_bank_insert);
EXPORT_SYMBOL(soc_fb_l2x_bank_delete);
EXPORT_SYMBOL(soc_fb_l2x_bank_lookup);
EXPORT_SYMBOL(soc_fb_l3x_bank_delete);
EXPORT_SYMBOL(soc_fb_l3x_bank_insert);
EXPORT_SYMBOL(soc_fb_l3x_bank_lookup);
EXPORT_SYMBOL(soc_fb_l3x_base_entry_to_key);
#endif  /* BCM_FIREBOLT_SUPPORT */
EXPORT_SYMBOL(soc_fb_l3_hash);

#if !defined(SOC_NO_NAMES)
EXPORT_SYMBOL(soc_fieldnames);
#endif /* !defined(SOC_NO_NAMES) */
#ifdef BCM_HERCULES_SUPPORT
EXPORT_SYMBOL(soc_hercules15_mmu_limits_config);
EXPORT_SYMBOL(soc_hercules_mem_read_word);
EXPORT_SYMBOL(soc_hercules_mem_write_word);
EXPORT_SYMBOL(soc_hercules_mmu_limits_config);
#endif  /* BCM_HERCULES_SUPPORT */
#ifdef BCM_XGS_SUPPORT
EXPORT_SYMBOL(soc_higig_dump);
#ifdef BCM_HIGIG2_SUPPORT
EXPORT_SYMBOL(soc_higig2_dump);
#endif /* BCM_HIGIG2_SUPPORT */
EXPORT_SYMBOL(soc_higig_field_set);
#endif /* BCM_XGS_SUPPORT */
#ifdef INCLUDE_I2C
EXPORT_SYMBOL(soc_i2c_attach);
EXPORT_SYMBOL(soc_i2c_clear_log);
EXPORT_SYMBOL(soc_i2c_device);
EXPORT_SYMBOL(soc_i2c_device_count);
EXPORT_SYMBOL(soc_i2c_devname);
EXPORT_SYMBOL(soc_i2c_is_attached);
EXPORT_SYMBOL(soc_i2c_lm75_monitor);
EXPORT_SYMBOL(soc_i2c_lm75_temperature_show);
EXPORT_SYMBOL(soc_i2c_max127_iterations);
EXPORT_SYMBOL(soc_i2c_probe);
EXPORT_SYMBOL(soc_i2c_reset);
EXPORT_SYMBOL(soc_i2c_show);
EXPORT_SYMBOL(soc_i2c_show_log);
EXPORT_SYMBOL(soc_i2c_show_speeds);
EXPORT_SYMBOL(soc_i2c_device_present);
EXPORT_SYMBOL(soc_i2c_saddr_to_string);
#endif /* INCLUDE_I2C */
EXPORT_SYMBOL(soc_detach);
EXPORT_SYMBOL(soc_init);
EXPORT_SYMBOL(soc_intr_disable);
EXPORT_SYMBOL(soc_intr_enable);
#ifdef BCM_XGS_SWITCH_SUPPORT
EXPORT_SYMBOL(soc_l2x_entry_compare_key);
EXPORT_SYMBOL(soc_l2x_entry_dump);
EXPORT_SYMBOL(soc_l2x_software_hash);
EXPORT_SYMBOL(soc_l2x_start);
EXPORT_SYMBOL(soc_l2x_stop);
EXPORT_SYMBOL(soc_l3x_software_hash);
#endif  /* BCM_XGS_SWITCH_SUPPORT */
EXPORT_SYMBOL(_soc_mem_entry_null_zeroes);
EXPORT_SYMBOL(soc_mem_addr);
EXPORT_SYMBOL(soc_mem_cache_get);
EXPORT_SYMBOL(soc_mem_cache_set);
EXPORT_SYMBOL(soc_mem_cfap_init);
EXPORT_SYMBOL(soc_mem_clear);
EXPORT_SYMBOL(soc_mem_debug_get);
EXPORT_SYMBOL(soc_mem_debug_set);
EXPORT_SYMBOL(soc_mem_delete);
EXPORT_SYMBOL(soc_mem_delete_index);
#if !defined(SOC_NO_DESC)
EXPORT_SYMBOL(soc_mem_desc);
#endif /* !defined(SOC_NO_DESC) */
EXPORT_SYMBOL(soc_mem_dmaable);
EXPORT_SYMBOL(soc_mem_entries);
EXPORT_SYMBOL(soc_mem_entry_dump);
EXPORT_SYMBOL(soc_mem_entry_dump_if_changed);
EXPORT_SYMBOL(soc_mem_field32_get);
EXPORT_SYMBOL(soc_mem_field32_set);
EXPORT_SYMBOL(soc_mem_field_get);
EXPORT_SYMBOL(soc_mem_field_length);
EXPORT_SYMBOL(soc_mem_field_set);
EXPORT_SYMBOL(soc_mem_generic_delete);
EXPORT_SYMBOL(soc_mem_generic_insert);
EXPORT_SYMBOL(soc_mem_generic_lookup);
EXPORT_SYMBOL(soc_mem_index_last);
EXPORT_SYMBOL(soc_mem_insert);
EXPORT_SYMBOL(soc_mem_bank_insert);
EXPORT_SYMBOL(soc_mem_ip6_addr_set);
EXPORT_SYMBOL(soc_mem_iterate);
#ifdef BCM_HERCULES_SUPPORT
EXPORT_SYMBOL(soc_mem_lla_init);
#endif /* BCM_HERCULES_SUPPORT */
EXPORT_SYMBOL(soc_mem_mac_addr_set);
EXPORT_SYMBOL(soc_mem_pbmp_field_get);
EXPORT_SYMBOL(soc_mem_pbmp_field_set);
EXPORT_SYMBOL(soc_mem_field64_get);
EXPORT_SYMBOL(soc_mem_field64_set);
EXPORT_SYMBOL(soc_mem_pop);
EXPORT_SYMBOL(soc_mem_push);
EXPORT_SYMBOL(soc_mem_read);
EXPORT_SYMBOL(soc_mem_read_range);
#ifdef INCLUDE_MEM_SCAN
EXPORT_SYMBOL(soc_mem_scan_running);
EXPORT_SYMBOL(soc_mem_scan_start);
EXPORT_SYMBOL(soc_mem_scan_stop);
#endif /* INCLUDE_MEM_SCAN */
EXPORT_SYMBOL(soc_mem_search);
#ifdef BCM_EASYRIDER_SUPPORT
EXPORT_SYMBOL(soc_mem_tcam_op);
#endif  /* BCM_EASYRIDER_SUPPORT */
#if !defined(SOC_NO_NAMES)
EXPORT_SYMBOL(soc_mem_name);
EXPORT_SYMBOL(soc_mem_ufalias);
EXPORT_SYMBOL(soc_mem_ufname);
#endif /* !defined(SOC_NO_NAMES) */
EXPORT_SYMBOL(soc_event_register);
EXPORT_SYMBOL(soc_event_unregister);
EXPORT_SYMBOL(soc_event_generate);
EXPORT_SYMBOL(soc_mem_snoop_register);
EXPORT_SYMBOL(soc_mem_snoop_unregister);
EXPORT_SYMBOL(soc_mem_write);
EXPORT_SYMBOL(soc_mem_write_range);
EXPORT_SYMBOL(soc_mmu_init);
#ifdef BCM_CMICM_SUPPORT
EXPORT_SYMBOL(soc_mspi_init);
EXPORT_SYMBOL(soc_mspi_select_device);
EXPORT_SYMBOL(soc_mspi_writeread8);
EXPORT_SYMBOL(soc_mspi_read8);
EXPORT_SYMBOL(soc_mspi_write8);
#endif
EXPORT_SYMBOL(soc_pci_off2name);
#ifdef BCM_CMICM_SUPPORT
EXPORT_SYMBOL(soc_pci_mcs_getreg);
EXPORT_SYMBOL(soc_pci_mcs_read);
EXPORT_SYMBOL(soc_pci_mcs_write);
#endif /* BCM_CMICM_SUPPORT */
#ifdef  SOC_PCI_DEBUG
EXPORT_SYMBOL(soc_pci_read);
EXPORT_SYMBOL(soc_pci_write);
#endif  /* SOC_PCI_DEBUG */
EXPORT_SYMBOL(soc_persist);
EXPORT_SYMBOL(soc_phyctrl_offset_get);
EXPORT_SYMBOL(soc_phyctrl_diag_ctrl);
EXPORT_SYMBOL(soc_port_cmap_get);
EXPORT_SYMBOL(soc_port_ability_to_mode);
EXPORT_SYMBOL(soc_port_mode_to_ability);
EXPORT_SYMBOL(soc_reg32_read);
EXPORT_SYMBOL(soc_reg32_write);
EXPORT_SYMBOL(soc_reg64_field_get);
EXPORT_SYMBOL(soc_reg64_field_set);
EXPORT_SYMBOL(soc_reg64_read);
EXPORT_SYMBOL(soc_reg_addr);
#ifdef BCM_CMICM_SUPPORT
EXPORT_SYMBOL(soc_cmicm_reg_get);
#endif /* BCM_CMICM_SUPPORT */
EXPORT_SYMBOL(soc_reg32_get);
EXPORT_SYMBOL(soc_reg32_set);
EXPORT_SYMBOL(soc_reg64_get);
EXPORT_SYMBOL(soc_reg_addr_get);
EXPORT_SYMBOL(soc_mem_addr_get);
EXPORT_SYMBOL(_soc_max_blocks_per_entry_get);
#if !defined(SOC_NO_ALIAS)
EXPORT_SYMBOL(soc_reg_alias);
#endif /* !defined(SOC_NO_ALIAS) */
#if !defined(SOC_NO_DESC)
EXPORT_SYMBOL(soc_reg_desc);
#endif /* !defined(SOC_NO_DESC) */
EXPORT_SYMBOL(soc_reg_field_valid);
EXPORT_SYMBOL(soc_reg_field_get);
EXPORT_SYMBOL(soc_reg_field_set);
EXPORT_SYMBOL(soc_reg_iterate);
#if !defined(SOC_NO_NAMES)
EXPORT_SYMBOL(soc_reg_name);
#endif /* !defined(SOC_NO_NAMES) */
EXPORT_SYMBOL(soc_reg_sprint_addr);
EXPORT_SYMBOL(soc_regaddrinfo_get);
EXPORT_SYMBOL(soc_regaddrinfo_extended_get);
EXPORT_SYMBOL(soc_regaddrlist_alloc);
EXPORT_SYMBOL(soc_regaddrlist_free);
EXPORT_SYMBOL(soc_reset);
EXPORT_SYMBOL(soc_reset_init);
EXPORT_SYMBOL(soc_schan_op);
EXPORT_SYMBOL(soc_tcam_init);
EXPORT_SYMBOL(soc_tcam_get_info);
EXPORT_SYMBOL(soc_tcam_mem_index_to_raw_index);
EXPORT_SYMBOL(soc_tcam_part_index_to_mem_index);
EXPORT_SYMBOL(soc_tcam_read_dbreg);
EXPORT_SYMBOL(soc_tcam_read_entry);
EXPORT_SYMBOL(soc_tcam_read_ima);
EXPORT_SYMBOL(soc_tcam_search_entry);
EXPORT_SYMBOL(soc_tcam_set_valid);
EXPORT_SYMBOL(soc_tcam_write_dbreg);
EXPORT_SYMBOL(soc_tcam_write_entry);
EXPORT_SYMBOL(soc_tcam_write_ib);
EXPORT_SYMBOL(soc_tcam_write_ima);
#ifdef BCM_TRX_SUPPORT
EXPORT_SYMBOL(soc_tr_l2x_base_entry_to_key);
EXPORT_SYMBOL(soc_tr_l2x_hash);
#endif /* BCM_TRX_SUPPORT */
#ifdef BCM_TRIUMPH_SUPPORT
EXPORT_SYMBOL(soc_triumph_ext_sram_enable_set);
EXPORT_SYMBOL(soc_triumph_ext_sram_bist_setup);
EXPORT_SYMBOL(soc_triumph_ext_sram_op);
EXPORT_SYMBOL(soc_triumph_tcam_search_bist);
#endif /* BCM_TRIUMPH_SUPPORT */
#ifdef BCM_TRIDENT_SUPPORT
EXPORT_SYMBOL(soc_trident_mmu_config_init);
EXPORT_SYMBOL(soc_trident_show_material_process);
EXPORT_SYMBOL(soc_trident_show_ring_osc);
EXPORT_SYMBOL(soc_trident_show_temperature_monitor);
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
EXPORT_SYMBOL(soc_katana_show_temperature_monitor);
EXPORT_SYMBOL(soc_katana_show_ring_osc);
#endif /* BCM_KATANA_SUPPORT */
#ifdef INCLUDE_CES
EXPORT_SYMBOL(bcm_esw_ces_detach);
EXPORT_SYMBOL(bcm_esw_ces_rclock_status_get);
#endif
EXPORT_SYMBOL(soc_tucana_l3_ecmp_hash);
#ifdef BCM_CMICM_SUPPORT
EXPORT_SYMBOL(soc_cmic_uc_msg_start);
EXPORT_SYMBOL(soc_cmic_uc_msg_stop);
EXPORT_SYMBOL(soc_cmic_uc_msg_uc_start);
#endif /* BCM_CMICM_SUPPORT */
EXPORT_SYMBOL(soc_xaui_config_get);
EXPORT_SYMBOL(soc_xaui_config_set);
EXPORT_SYMBOL(soc_xaui_rxbert_bit_err_count_get);
EXPORT_SYMBOL(soc_xaui_rxbert_byte_count_get);
EXPORT_SYMBOL(soc_xaui_rxbert_byte_err_count_get);
EXPORT_SYMBOL(soc_xaui_rxbert_enable);
EXPORT_SYMBOL(soc_xaui_rxbert_pkt_count_get);
EXPORT_SYMBOL(soc_xaui_rxbert_pkt_err_count_get);
EXPORT_SYMBOL(soc_xaui_txbert_byte_count_get);
EXPORT_SYMBOL(soc_xaui_txbert_enable);
EXPORT_SYMBOL(soc_xaui_txbert_pkt_count_get);
EXPORT_SYMBOL(soc_phy_list_get);
#ifdef BCM_XGS3_SWITCH_SUPPORT
EXPORT_SYMBOL(soc_xgs3_l3_ecmp_hash);
#endif /* BCM_XGS3_SWITCH_SUPPORT */
#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_LIB_CPUDB
EXPORT_SYMBOL(tks_debug_level);
EXPORT_SYMBOL(tks_debug_names);
#endif /* INCLUDE_LIB_CPUDB */
#endif /* BROADCOM_DEBUG */

#endif /* BCM_ESW_SUPPORT */ 

/* 
 * Export system-related symbols and symbols
 * required by the diag shell .
 * ROBO specific 
 */

#ifdef BCM_ROBO_SUPPORT
EXPORT_SYMBOL(soc_robo_base_driver_table);
EXPORT_SYMBOL(soc_eth_dma_desc_add);
EXPORT_SYMBOL(soc_robo_counter_idx_get);
EXPORT_SYMBOL(soc_eth_dma_dump_dv);
EXPORT_SYMBOL(soc_robo_anyreg_write);
EXPORT_SYMBOL(soc_eth_dma_dv_free);
EXPORT_SYMBOL(soc_robo_port_cmap_get);
EXPORT_SYMBOL(soc_robo_counter_set32_by_port);
EXPORT_SYMBOL(soc_robo_mem_ufname);
EXPORT_SYMBOL(soc_robo_reg_name);
EXPORT_SYMBOL(soc_eth_dma_dump_pkt);
EXPORT_SYMBOL(soc_robo_mem_ufalias);
EXPORT_SYMBOL(soc_robo_reset_init);
EXPORT_SYMBOL(soc_robo_regaddrlist_free);
EXPORT_SYMBOL(soc_eth_dma_start);
EXPORT_SYMBOL(soc_robo_counter_get_rate);
EXPORT_SYMBOL(soc_robo_mem_debug_get);
EXPORT_SYMBOL(soc_robo_counter_start);
EXPORT_SYMBOL(soc_robo_anyreg_read);
EXPORT_SYMBOL(soc_robo_mem_name);
EXPORT_SYMBOL(soc_robo_reg_desc);
EXPORT_SYMBOL(soc_robo_regaddrlist_alloc);
EXPORT_SYMBOL(soc_robo_reg_iterate);
EXPORT_SYMBOL(soc_robo_chip_dump);
EXPORT_SYMBOL(soc_robo_reg_sprint_addr);
EXPORT_SYMBOL(soc_robo_mem_desc);
EXPORT_SYMBOL(soc_robo_regaddrinfo_get);
EXPORT_SYMBOL(soc_robo_fieldnames);
EXPORT_SYMBOL(soc_robo_reg_alias);
EXPORT_SYMBOL(soc_robo_dump);
EXPORT_SYMBOL(soc_eth_dma_ether_dump);
EXPORT_SYMBOL(soc_eth_dma_dv_alloc);
EXPORT_SYMBOL(soc_robo_counter_stop);
EXPORT_SYMBOL(soc_robo_counter_get);
EXPORT_SYMBOL(soc_robo_arl_mode_set);
#ifndef BCM_ESW_SUPPORT
EXPORT_SYMBOL(bcm_attach);
EXPORT_SYMBOL(bcm_attach_check);
EXPORT_SYMBOL(bcm_detach);
EXPORT_SYMBOL(bcm_find);
EXPORT_SYMBOL(bcm_l2_addr_t_init);
EXPORT_SYMBOL(bcm_l2_cache_addr_t_init);
EXPORT_SYMBOL(bcm_mcast_addr_t_init);
EXPORT_SYMBOL(bcm_module_name);
EXPORT_SYMBOL(bcm_vlan_action_set_t_init);
EXPORT_SYMBOL(bcm_vlan_block_t_init);
EXPORT_SYMBOL(bcm_vlan_control_vlan_t_init);
EXPORT_SYMBOL(bcm_policer_config_t_init);
#endif
EXPORT_SYMBOL(_bcm_robo_auth_sec_mode_set);
EXPORT_SYMBOL(_robo_field_qset_dump);
EXPORT_SYMBOL(bcm_rx_debug);
EXPORT_SYMBOL(_bcm_robo_l2_from_arl);
EXPORT_SYMBOL(bcm5324_trunk_patch_linkscan);
EXPORT_SYMBOL(_bcm_robo_trunk_gport_resolve);
EXPORT_SYMBOL(soc_robo_counter_prev_get);
EXPORT_SYMBOL(soc_robo_counter_prev_set);
EXPORT_SYMBOL(_robo_field_thread_stop);
EXPORT_SYMBOL(soc_robo_detach);
EXPORT_SYMBOL(soc_robo_init);
EXPORT_SYMBOL(_bcm_robo_gport_resolve);
EXPORT_SYMBOL( _bcm_robo_l2_gport_parse);
EXPORT_SYMBOL( _bcm_robo_modid_is_local);
EXPORT_SYMBOL(bcm_robo_port_phy_power_mode_set);
EXPORT_SYMBOL(bcm_robo_port_phy_power_mode_get);
EXPORT_SYMBOL(_bcm_robo_pbmp_check);
EXPORT_SYMBOL(soc_arl_search_valid);
EXPORT_SYMBOL(_soc_robo_max_blocks_per_entry_get);
#ifdef BCM_TB_SUPPORT
EXPORT_SYMBOL(soc_tb_cmap_get);
#endif
#endif /* BCM_ROBO_SUPPORT */

#ifdef BCM_SBX_SUPPORT
EXPORT_SYMBOL(bcm_sbx_stk_fabric_map_set);
#endif /* BCM_SBX_SUPPORT */

#ifdef BCM_SIRIUS_SUPPORT
EXPORT_SYMBOL(_bcm_sirius_multicast_defrag);
EXPORT_SYMBOL(_bcm_sirius_multicast_oitt_enable_set);
EXPORT_SYMBOL(_bcm_sirius_multicast_dump);
EXPORT_SYMBOL(bcm_sbx_cosq_get_base_queue_from_gport);
EXPORT_SYMBOL(bcm_sbx_cosq_gport_attach_get);
EXPORT_SYMBOL(bcm_sbx_cosq_qinfo_get);
EXPORT_SYMBOL(bcm_sirius_cosq_get_ingress_scheduler);
EXPORT_SYMBOL(bcm_sbx_port_qinfo_get);
EXPORT_SYMBOL(is_state);
EXPORT_SYMBOL(map_ds_id_to_qid);
EXPORT_SYMBOL(map_np_to_qid);
EXPORT_SYMBOL(phy_reg_ci_read);
EXPORT_SYMBOL(phy_reg_ci_write);
EXPORT_SYMBOL(sbZfFabBm9600BwAllocCfgBaseEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600BwAllocCfgBaseEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600BwAllocCfgBaseEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600BwAllocCfgBaseEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600BwAllocRateEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600BwAllocRateEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600BwAllocRateEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600BwAllocRateEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600BwFetchDataEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600BwFetchDataEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600BwFetchDataEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600BwFetchDataEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600BwFetchSumEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600BwFetchSumEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600BwFetchSumEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600BwFetchSumEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600BwFetchValidEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600BwFetchValidEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600BwFetchValidEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600BwFetchValidEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600BwR0BagEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600BwR0BagEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600BwR0BagEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600BwR0BagEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600BwR0BwpEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600BwR0BwpEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600BwR0BwpEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600BwR0BwpEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600BwR0WdtEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600BwR0WdtEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600BwR0WdtEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600BwR0WdtEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600BwR1BagEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600BwR1BagEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600BwR1BagEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600BwR1BagEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600BwR1Wct0AEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600BwR1Wct0AEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600BwR1Wct0AEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600BwR1Wct0AEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600BwR1Wct0BEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600BwR1Wct0BEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600BwR1Wct0BEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600BwR1Wct0BEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600BwR1Wct1AEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600BwR1Wct1AEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600BwR1Wct1AEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600BwR1Wct1AEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600BwR1Wct1BEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600BwR1Wct1BEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600BwR1Wct1BEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600BwR1Wct1BEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600BwR1Wct2AEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600BwR1Wct2AEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600BwR1Wct2AEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600BwR1Wct2AEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600BwR1Wct2BEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600BwR1Wct2BEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600BwR1Wct2BEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600BwR1Wct2BEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600BwR1WstEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600BwR1WstEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600BwR1WstEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600BwR1WstEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600BwWredCfgBaseEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600BwWredCfgBaseEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600BwWredCfgBaseEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600BwWredCfgBaseEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600BwWredDropNPart1Entry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600BwWredDropNPart1Entry_Print);
EXPORT_SYMBOL(sbZfFabBm9600BwWredDropNPart1Entry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600BwWredDropNPart1Entry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600BwWredDropNPart2Entry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600BwWredDropNPart2Entry_Print);
EXPORT_SYMBOL(sbZfFabBm9600BwWredDropNPart2Entry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600BwWredDropNPart2Entry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600FoLinkStateTableEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600FoLinkStateTableEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600FoLinkStateTableEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600FoLinkStateTableEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600InaEsetPriEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600InaEsetPriEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600InaEsetPriEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600InaEsetPriEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600InaHi1Selected_0Entry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600InaHi1Selected_0Entry_Print);
EXPORT_SYMBOL(sbZfFabBm9600InaHi1Selected_0Entry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600InaHi1Selected_0Entry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600InaHi1Selected_1Entry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600InaHi1Selected_1Entry_Print);
EXPORT_SYMBOL(sbZfFabBm9600InaHi1Selected_1Entry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600InaHi1Selected_1Entry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600InaHi2Selected_0Entry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600InaHi2Selected_0Entry_Print);
EXPORT_SYMBOL(sbZfFabBm9600InaHi2Selected_0Entry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600InaHi2Selected_0Entry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600InaHi2Selected_1Entry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600InaHi2Selected_1Entry_Print);
EXPORT_SYMBOL(sbZfFabBm9600InaHi2Selected_1Entry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600InaHi2Selected_1Entry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600InaHi3Selected_0Entry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600InaHi3Selected_0Entry_Print);
EXPORT_SYMBOL(sbZfFabBm9600InaHi3Selected_0Entry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600InaHi3Selected_0Entry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600InaHi3Selected_1Entry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600InaHi3Selected_1Entry_Print);
EXPORT_SYMBOL(sbZfFabBm9600InaHi3Selected_1Entry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600InaHi3Selected_1Entry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600InaHi4Selected_0Entry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600InaHi4Selected_0Entry_Print);
EXPORT_SYMBOL(sbZfFabBm9600InaHi4Selected_0Entry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600InaHi4Selected_0Entry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600InaHi4Selected_1Entry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600InaHi4Selected_1Entry_Print);
EXPORT_SYMBOL(sbZfFabBm9600InaHi4Selected_1Entry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600InaHi4Selected_1Entry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600InaPortPriEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600InaPortPriEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600InaPortPriEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600InaPortPriEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600InaRandomNumGenEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600InaRandomNumGenEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600InaRandomNumGenEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600InaRandomNumGenEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600InaSysportMapEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600InaSysportMapEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600InaSysportMapEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600InaSysportMapEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600NmEgressRankerEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600NmEgressRankerEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600NmEgressRankerEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600NmEgressRankerEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600NmEmtEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600NmEmtEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600NmEmtEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600NmEmtEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600NmEmtdebugbank0Entry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600NmEmtdebugbank0Entry_Print);
EXPORT_SYMBOL(sbZfFabBm9600NmEmtdebugbank0Entry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600NmEmtdebugbank0Entry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600NmEmtdebugbank1Entry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600NmEmtdebugbank1Entry_Print);
EXPORT_SYMBOL(sbZfFabBm9600NmEmtdebugbank1Entry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600NmEmtdebugbank1Entry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600NmFullStatusEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600NmFullStatusEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600NmFullStatusEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600NmFullStatusEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600NmIngressRankerEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600NmIngressRankerEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600NmIngressRankerEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600NmIngressRankerEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600NmPortsetInfoEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600NmPortsetInfoEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600NmPortsetInfoEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600NmPortsetInfoEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600NmPortsetLinkEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600NmPortsetLinkEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600NmPortsetLinkEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600NmPortsetLinkEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600NmRandomNumGenEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600NmRandomNumGenEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600NmRandomNumGenEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600NmRandomNumGenEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600NmSysportArrayEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600NmSysportArrayEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600NmSysportArrayEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600NmSysportArrayEntry_Unpack);
EXPORT_SYMBOL(sbZfFabBm9600XbXcfgRemapEntry_Pack);
EXPORT_SYMBOL(sbZfFabBm9600XbXcfgRemapEntry_Print);
EXPORT_SYMBOL(sbZfFabBm9600XbXcfgRemapEntry_SetField);
EXPORT_SYMBOL(sbZfFabBm9600XbXcfgRemapEntry_Unpack);
EXPORT_SYMBOL(shr_aidxres_list_alloc);
EXPORT_SYMBOL(shr_aidxres_list_alloc_block);
EXPORT_SYMBOL(shr_aidxres_list_create);
EXPORT_SYMBOL(shr_aidxres_list_destroy);
EXPORT_SYMBOL(shr_aidxres_list_elem_state);
EXPORT_SYMBOL(shr_aidxres_list_free);
EXPORT_SYMBOL(shr_aidxres_list_reserve);
EXPORT_SYMBOL(shr_aidxres_list_reserve_block);
EXPORT_SYMBOL(shr_aidxres_list_state);
EXPORT_SYMBOL(shr_idxres_list_alloc);
EXPORT_SYMBOL(shr_idxres_list_create);
EXPORT_SYMBOL(shr_idxres_list_create_scaled);
EXPORT_SYMBOL(shr_idxres_list_destroy);
EXPORT_SYMBOL(shr_idxres_list_elem_state);
EXPORT_SYMBOL(shr_idxres_list_free);
EXPORT_SYMBOL(shr_idxres_list_reserve);
EXPORT_SYMBOL(shr_idxres_list_state_scaled);
EXPORT_SYMBOL(sirius_reg_done_timeout);
EXPORT_SYMBOL(soc_bm9600_BwAllocCfgBaseRead);
EXPORT_SYMBOL(soc_bm9600_BwAllocCfgBaseWrite);
EXPORT_SYMBOL(soc_bm9600_BwAllocRateRead);
EXPORT_SYMBOL(soc_bm9600_BwAllocRateWrite);
EXPORT_SYMBOL(soc_bm9600_BwFetchDataRead);
EXPORT_SYMBOL(soc_bm9600_BwFetchDataWrite);
EXPORT_SYMBOL(soc_bm9600_BwFetchSumRead);
EXPORT_SYMBOL(soc_bm9600_BwFetchSumWrite);
EXPORT_SYMBOL(soc_bm9600_BwFetchValidRead);
EXPORT_SYMBOL(soc_bm9600_BwFetchValidWrite);
EXPORT_SYMBOL(soc_bm9600_BwR0BagRead);
EXPORT_SYMBOL(soc_bm9600_BwR0BagWrite);
EXPORT_SYMBOL(soc_bm9600_BwR0BwpRead);
EXPORT_SYMBOL(soc_bm9600_BwR0BwpWrite);
EXPORT_SYMBOL(soc_bm9600_BwR0WdtRead);
EXPORT_SYMBOL(soc_bm9600_BwR0WdtWrite);
EXPORT_SYMBOL(soc_bm9600_BwR1BagRead);
EXPORT_SYMBOL(soc_bm9600_BwR1BagWrite);
EXPORT_SYMBOL(soc_bm9600_BwR1Wct0ARead);
EXPORT_SYMBOL(soc_bm9600_BwR1Wct0AWrite);
EXPORT_SYMBOL(soc_bm9600_BwR1Wct0BRead);
EXPORT_SYMBOL(soc_bm9600_BwR1Wct0BWrite);
EXPORT_SYMBOL(soc_bm9600_BwR1Wct1ARead);
EXPORT_SYMBOL(soc_bm9600_BwR1Wct1AWrite);
EXPORT_SYMBOL(soc_bm9600_BwR1Wct1BRead);
EXPORT_SYMBOL(soc_bm9600_BwR1Wct1BWrite);
EXPORT_SYMBOL(soc_bm9600_BwR1Wct2ARead);
EXPORT_SYMBOL(soc_bm9600_BwR1Wct2AWrite);
EXPORT_SYMBOL(soc_bm9600_BwR1Wct2BRead);
EXPORT_SYMBOL(soc_bm9600_BwR1Wct2BWrite);
EXPORT_SYMBOL(soc_bm9600_BwR1WstRead);
EXPORT_SYMBOL(soc_bm9600_BwR1WstWrite);
EXPORT_SYMBOL(soc_bm9600_BwWredCfgBaseRead);
EXPORT_SYMBOL(soc_bm9600_BwWredCfgBaseWrite);
EXPORT_SYMBOL(soc_bm9600_BwWredDropNPart1Read);
EXPORT_SYMBOL(soc_bm9600_BwWredDropNPart1Write);
EXPORT_SYMBOL(soc_bm9600_BwWredDropNPart2Read);
EXPORT_SYMBOL(soc_bm9600_BwWredDropNPart2Write);
EXPORT_SYMBOL(soc_bm9600_FoLinkStateTableRead);
EXPORT_SYMBOL(soc_bm9600_FoLinkStateTableWrite);
EXPORT_SYMBOL(soc_bm9600_HwNmEmtRead);
EXPORT_SYMBOL(soc_bm9600_HwNmPortsetInfoRead);
EXPORT_SYMBOL(soc_bm9600_HwNmPortsetLinkRead);
EXPORT_SYMBOL(soc_bm9600_HwNmSysportArrayRead);
EXPORT_SYMBOL(soc_bm9600_InaEsetPriRead);
EXPORT_SYMBOL(soc_bm9600_InaEsetPriWrite);
EXPORT_SYMBOL(soc_bm9600_InaHi1Selected_0Read);
EXPORT_SYMBOL(soc_bm9600_InaHi1Selected_0Write);
EXPORT_SYMBOL(soc_bm9600_InaHi1Selected_1Read);
EXPORT_SYMBOL(soc_bm9600_InaHi1Selected_1Write);
EXPORT_SYMBOL(soc_bm9600_InaHi2Selected_0Read);
EXPORT_SYMBOL(soc_bm9600_InaHi2Selected_0Write);
EXPORT_SYMBOL(soc_bm9600_InaHi2Selected_1Read);
EXPORT_SYMBOL(soc_bm9600_InaHi2Selected_1Write);
EXPORT_SYMBOL(soc_bm9600_InaHi3Selected_0Read);
EXPORT_SYMBOL(soc_bm9600_InaHi3Selected_0Write);
EXPORT_SYMBOL(soc_bm9600_InaHi3Selected_1Read);
EXPORT_SYMBOL(soc_bm9600_InaHi3Selected_1Write);
EXPORT_SYMBOL(soc_bm9600_InaHi4Selected_0Read);
EXPORT_SYMBOL(soc_bm9600_InaHi4Selected_0Write);
EXPORT_SYMBOL(soc_bm9600_InaHi4Selected_1Read);
EXPORT_SYMBOL(soc_bm9600_InaHi4Selected_1Write);
EXPORT_SYMBOL(soc_bm9600_InaPortPriRead);
EXPORT_SYMBOL(soc_bm9600_InaPortPriWrite);
EXPORT_SYMBOL(soc_bm9600_InaRandomNumGenRead);
EXPORT_SYMBOL(soc_bm9600_InaRandomNumGenWrite);
EXPORT_SYMBOL(soc_bm9600_InaSysportMapRead);
EXPORT_SYMBOL(soc_bm9600_InaSysportMapWrite);
EXPORT_SYMBOL(soc_bm9600_NmEgressRankerRead);
EXPORT_SYMBOL(soc_bm9600_NmEgressRankerWrite);
EXPORT_SYMBOL(soc_bm9600_NmEmtRead);
EXPORT_SYMBOL(soc_bm9600_NmEmtWrite);
EXPORT_SYMBOL(soc_bm9600_NmEmtdebugbank0Read);
EXPORT_SYMBOL(soc_bm9600_NmEmtdebugbank0Write);
EXPORT_SYMBOL(soc_bm9600_NmEmtdebugbank1Read);
EXPORT_SYMBOL(soc_bm9600_NmEmtdebugbank1Write);
EXPORT_SYMBOL(soc_bm9600_NmFullStatusRead);
EXPORT_SYMBOL(soc_bm9600_NmFullStatusWrite);
EXPORT_SYMBOL(soc_bm9600_NmIngressRankerRead);
EXPORT_SYMBOL(soc_bm9600_NmIngressRankerWrite);
EXPORT_SYMBOL(soc_bm9600_NmPortsetInfoRead);
EXPORT_SYMBOL(soc_bm9600_NmPortsetInfoWrite);
EXPORT_SYMBOL(soc_bm9600_NmPortsetLinkRead);
EXPORT_SYMBOL(soc_bm9600_NmPortsetLinkWrite);
EXPORT_SYMBOL(soc_bm9600_NmRandomNumGenRead);
EXPORT_SYMBOL(soc_bm9600_NmRandomNumGenWrite);
EXPORT_SYMBOL(soc_bm9600_NmSysportArrayRead);
EXPORT_SYMBOL(soc_bm9600_NmSysportArrayWrite);
EXPORT_SYMBOL(soc_bm9600_XbXcfgRemapSelectRead);
EXPORT_SYMBOL(soc_bm9600_XbXcfgRemapSelectWrite);
EXPORT_SYMBOL(soc_bm9600_lcm_fixed_config);
EXPORT_SYMBOL(soc_bm9600_lcm_fixed_config_validate);
EXPORT_SYMBOL(soc_bm9600_lcm_mode_set);
EXPORT_SYMBOL(soc_bm9600_mdio_hc_read);
EXPORT_SYMBOL(soc_bm9600_mdio_hc_write);
EXPORT_SYMBOL(soc_bm9600_xb_test_pkt_clear);
EXPORT_SYMBOL(soc_bm9600_xb_test_pkt_get);
EXPORT_SYMBOL(soc_sbx_block_names);
EXPORT_SYMBOL(soc_sbx_block_port_names);
EXPORT_SYMBOL(soc_sbx_chip_dump);
EXPORT_SYMBOL(soc_sbx_counter_start);
EXPORT_SYMBOL(soc_sbx_counter_stop);
EXPORT_SYMBOL(soc_sbx_counter_sync);
EXPORT_SYMBOL(soc_sbx_driver_table);
EXPORT_SYMBOL(soc_sbx_dump);
EXPORT_SYMBOL(soc_sbx_init);
EXPORT_SYMBOL(soc_sbx_sirius_ddr23_read);
EXPORT_SYMBOL(soc_sbx_sirius_ddr23_vdl_set);
EXPORT_SYMBOL(soc_sbx_sirius_ddr23_write);
EXPORT_SYMBOL(soc_sbx_txrx_sync_rx);
EXPORT_SYMBOL(soc_sbx_txrx_sync_tx);
EXPORT_SYMBOL(soc_sirius_mdio_hc_read);
EXPORT_SYMBOL(soc_sirius_mdio_hc_write);
EXPORT_SYMBOL(thin_read32);
EXPORT_SYMBOL(thin_write32);
EXPORT_SYMBOL(_soc_sbx_sirius_state);
EXPORT_SYMBOL(soc_sirius_qs_leaf_node_to_queue_get);
EXPORT_SYMBOL(soc_sirius_qs_queue_to_leaf_node_get);
EXPORT_SYMBOL(soc_sirius_ts_node_hierachy_config_get);
#endif /* BCM_SIRIUS_SUPPORT */

#ifdef BCM_EA_SUPPORT
#ifdef BCM_TK371X_SUPPORT
extern int ea_probe_thread_running;
extern int soc_ea_detach(int unit);   
extern int soc_ea_reset_init(int unit);
extern int soc_ea_pre_attach(int unit);
extern int soc_ea_do_init(int count);
extern int soc_chip_reset(int unit);
extern int soc_nvs_erase(int unit);
extern int soc_gpio_read(int unit, uint32 flags, uint32 *data);
extern int soc_gpio_write(int unit, uint32 flags, uint32 mask, uint32 data);
extern void soc_ea_dbg_level_set(uint32 lvl);
extern void soc_ea_dbg_level_dump(void);
extern void soc_ea_private_db_dump(int unit);
extern int _bcm_tk371x_port_selective_get(int unit, bcm_port_t port, bcm_port_info_t *info);
extern void _bcm_tk371x_field_qset_dump(char *prefix, bcm_field_qset_t qset, char *suffix);
EXPORT_SYMBOL(_bcm_tk371x_port_selective_get);
EXPORT_SYMBOL(_bcm_tk371x_field_qset_dump);
EXPORT_SYMBOL(ea_probe_thread_running);
EXPORT_SYMBOL(soc_ea_detach);
EXPORT_SYMBOL(soc_ea_reset_init);
EXPORT_SYMBOL(soc_ea_pre_attach);
EXPORT_SYMBOL(soc_ea_do_init);
EXPORT_SYMBOL(soc_ea_dbg_level_set);
EXPORT_SYMBOL(soc_ea_dbg_level_dump);
EXPORT_SYMBOL(soc_chip_reset);
EXPORT_SYMBOL(soc_gpio_read);
EXPORT_SYMBOL(soc_gpio_write);
EXPORT_SYMBOL(soc_nvs_erase);
EXPORT_SYMBOL(soc_ea_private_db_dump);
#endif
#endif /*end BCM_EA_SUPPORT*/

#include <shared/shr_resmgr.h>

EXPORT_SYMBOL(shr_mres_alloc);
EXPORT_SYMBOL(shr_mres_alloc_align);
EXPORT_SYMBOL(shr_mres_check);
EXPORT_SYMBOL(shr_mres_create);
EXPORT_SYMBOL(shr_mres_destroy);
EXPORT_SYMBOL(shr_mres_dump);
EXPORT_SYMBOL(shr_mres_free);
EXPORT_SYMBOL(shr_mres_get);
EXPORT_SYMBOL(shr_mres_pool_get);
EXPORT_SYMBOL(shr_mres_pool_set);
EXPORT_SYMBOL(shr_mres_type_get);
EXPORT_SYMBOL(shr_mres_type_set);
EXPORT_SYMBOL(shr_res_alloc);
EXPORT_SYMBOL(shr_res_alloc_align);
EXPORT_SYMBOL(shr_res_check);
EXPORT_SYMBOL(shr_res_detach);
EXPORT_SYMBOL(shr_res_dump);
EXPORT_SYMBOL(shr_res_free);
EXPORT_SYMBOL(shr_res_get);
EXPORT_SYMBOL(shr_res_init);
EXPORT_SYMBOL(shr_res_pool_get);
EXPORT_SYMBOL(shr_res_pool_set);
EXPORT_SYMBOL(shr_res_type_get);
EXPORT_SYMBOL(shr_res_type_set);

#ifdef INCLUDE_MACSEC
EXPORT_SYMBOL(bcm_common_macsec_config_print);
#endif /* INCLUDE_MACSEC */
