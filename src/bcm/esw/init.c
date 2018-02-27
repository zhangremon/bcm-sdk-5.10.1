/*
 * $Id: init.c 1.153.2.12 Broadcom SDK $
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
 * BCM Library Initialization
 *
 *   This module calls the initialization routine of each BCM module.
 *
 * Initial System Configuration
 *
 *   Each module should initialize itself without reference to other BCM
 *   library modules to avoid a chicken-and-the-egg problem.  To do
 *   this, each module should initialize its respective internal state
 *   and hardware tables to match the Initial System Configuration.  The
 *   Initial System Configuration is:
 *
 *   STG 1 containing VLAN 1
 *   STG 1 all ports in the DISABLED state
 *   VLAN 1 with
 *	PBMP = all switching Ethernet ports (non-fabric) and the CPU.
 *	UBMP = all switching Ethernet ports (non-fabric).
 *   No trunks configured
 *   No mirroring configured
 *   All L2 and L3 tables empty
 *   Ingress VLAN filtering disabled
 *   BPDU reception enabled
 */

#include <sal/types.h>
#include <sal/core/time.h>
#include <sal/core/boot.h>

#include <soc/cmext.h>
#include <soc/counter.h>
#include <soc/l2x.h>
#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/phyctrl.h>

#include <bcm/init.h>
#include <bcm/error.h>
#include <bcm/rx.h>
#include <bcm/pkt.h>
#include <bcm/ipfix.h>

#include <bcm_int/api_xlate_port.h>
#include <bcm_int/control.h>
#include <bcm_int/common/lock.h>
#include <bcm_int/esw/rcpu.h>
#include <bcm_int/esw/mirror.h>
#include <bcm_int/esw/filter.h>
#include <bcm_int/esw/stat.h>
#include <bcm_int/esw/mcast.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/ipfix.h>
#include <bcm_int/esw/mirror.h>
#include <bcm_int/esw/diffserv.h>
#include <bcm_int/esw/stack.h>
#include <bcm_int/esw_dispatch.h>
#include <bcm_int/esw/stg.h>
#include <bcm_int/esw/switch.h>
#include <bcm_int/esw/vlan.h>
#include <bcm_int/esw/cosq.h>
#include <bcm_int/esw/rx.h>
#include <bcm_int/esw/rate.h>
#ifdef BCM_KATANA_SUPPORT
#include <bcm_int/esw/katana.h>
#include <bcm_int/esw/policer.h>
#endif
#ifdef BCM_TRX_SUPPORT
#include <bcm_int/esw/trx.h>
#endif /* BCM_TRX_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT
#include <bcm_int/esw/vlan.h>
#include <bcm_int/esw/trunk.h>
#include <bcm_int/esw/field.h>
#include <bcm_int/esw/l3.h>
#include <bcm_int/esw/mcast.h>
#include <bcm_int/esw/port.h>
#endif

#ifdef INCLUDE_MACSEC
#include <bcm_int/common/macsec_cmn.h>
#endif /* INCLUDE_MACSEC */

#ifdef INCLUDE_FCMAP
#include <bcm_int/common/fcmap_cmn.h>
#endif /* INCLUDE_FCMAP */

#include <bcm_int/esw_dispatch.h>

#define BCM_CHECK_ERROR_RETURN(op)  \
    if ((op < 0) && (op != BCM_E_UNAVAIL)) { \
         return (op); \
    }

/*
 * Function:
 *	_bcm_lock_init
 * Purpose:
 *	Allocate BCM_LOCK.
 */

STATIC int
_bcm_lock_init(int unit)
{
    if (_bcm_lock[unit] == NULL) {
	_bcm_lock[unit] = sal_mutex_create("bcm_config_lock");
    }

    if (_bcm_lock[unit] == NULL) {
	return BCM_E_MEMORY;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_lock_deinit
 * Purpose:
 *	De-allocate BCM_LOCK.
 */

STATIC int
_bcm_lock_deinit(int unit)
{
    if (_bcm_lock[unit] != NULL) {
        sal_mutex_destroy(_bcm_lock[unit]);
        _bcm_lock[unit] = NULL;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *	   _bcm_esw_modules_deinit
 * Purpose:
 *	   De-initialize bcm modules
 * Parameters:
 *     unit - (IN) BCM device number.
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_esw_modules_deinit(int unit)
{
    int rv;    /* Operation return status. */

    if (soc_feature(unit, soc_feature_oam))
    {
        soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing OAM...\n");
        rv = bcm_esw_oam_detach(unit);
        BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);
    }
    if (soc_feature(unit, soc_feature_failover))
    {
        soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing FAILOVER...\n");
        rv = bcm_esw_failover_cleanup(unit);
        BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);
    }

    if (soc_feature(unit, soc_feature_time_support))
    {
        soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing time...\n");
        rv = bcm_esw_time_deinit(unit);
        BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);
    }
    

#if defined(BCM_FILTER_SUPPORT) || defined(BCM_FIELD_SUPPORT)
    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing auth...\n");
    rv = bcm_esw_auth_detach(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);
#endif

#ifdef BCM_DMUX_SUPPORT
    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing dmux...\n");
    rv = bcm_esw_dmux_detach(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);
#endif

#ifdef INCLUDE_L3
    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing proxy...\n");
    rv = bcm_esw_proxy_cleanup(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing multicast...\n");
    rv = bcm_esw_multicast_detach(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing subport...\n");
    rv = bcm_esw_subport_cleanup(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

#ifdef BCM_MPLS_SUPPORT
    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing MPLS...\n");
    rv = bcm_esw_mpls_cleanup(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);
#endif /* BCM_MPLS_SUPPORT */

    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing IPMC...\n");
    rv = bcm_esw_ipmc_detach(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing L3...\n");
    /*
     * COVERITY
     *
     * Coverity reports a call chain of depth greater than 20, but well
     * before the stack overflows, it calls bcm_esw_switch_control_get
     * with one control selection, then follows another control's path
     * to find more levels of stack.  This is spurious.
     */
    /* coverity[stack_use_overflow : FALSE] */
    rv = bcm_esw_l3_cleanup(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);
#endif /* INCLUDE_L3 */

    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing RX...\n");
    rv = bcm_esw_rx_deinit(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

#if 0
    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing TX...\n");
    rv = bcm_esw_tx_deinit(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);
#endif 

    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing mirror...\n");
    rv = bcm_esw_mirror_deinit(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

#if defined(BCM_FILTER_SUPPORT) || defined(BCM_FIELD_SUPPORT)
    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing diffserv...\n");
    rv = _bcm_esw_ds_deinit(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);
#endif /* BCM_FIELD_SUPPORT || BCM_FILTER_SUPPORT */

#ifdef BCM_FIELD_SUPPORT
    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing FP...\n");
    rv = bcm_esw_field_detach(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);
#endif /* BCM_FIELD_SUPPORT */

#ifdef BCM_FILTER_SUPPORT
    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing filter...\n");
    rv = _bcm_esw_filter_detach(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);
#endif /* BCM_FILTER_SUPPORT*/

#ifdef INCLUDE_KNET
    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing KNET...\n");
    rv = bcm_esw_knet_cleanup(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);
#endif

    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing stacking...\n");
    rv = _bcm_esw_stk_detach(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing stats...\n");
    rv = _bcm_esw_stat_detach(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing linkscan...\n");
    rv = bcm_esw_linkscan_detach(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing mcast...\n");
    rv = _bcm_esw_mcast_detach(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing cosq...\n");
    rv = bcm_esw_cosq_deinit(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing trunk...\n");
    rv = bcm_esw_trunk_detach(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing vlan...\n");
    rv = bcm_esw_vlan_detach(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing STG...\n");
    rv = bcm_esw_stg_detach(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing L2...\n");
    rv = bcm_esw_l2_detach(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing port...\n");
    rv = _bcm_esw_port_deinit(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing ipfix...\n");
    rv = _bcm_esw_ipfix_deinit(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing mbcm...\n");
    rv = mbcm_deinit(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

#ifdef INCLUDE_L3
    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing WLAN...\n");
    rv = bcm_esw_wlan_detach(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing MiM...\n");
    rv = bcm_esw_mim_detach(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);
#endif

    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing QOS...\n");
    rv = bcm_esw_qos_detach(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

    soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing Switch...\n");
    rv = _bcm_esw_switch_detach(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

#ifdef BCM_TRX_SUPPORT
    if (soc_feature(unit, soc_feature_virtual_switching)) {
        soc_cm_debug(DK_VERBOSE, "bcm_detach: Deinitializing Common...\n");
        rv = _bcm_common_cleanup(unit);
        BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);
    }
#endif /* BCM_TRX_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
    /* Service meter */
    if (soc_feature(unit, soc_feature_global_meter)) {
        rv = _bcm_esw_global_meter_cleanup(unit);
        BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);
    }
#endif /*BCM_KATANA_SUPPORT */
    soc_cm_debug(DK_VERBOSE, "bcm_detach: All modules deinitialized.\n");
    BCM_UNLOCK(unit);
    _bcm_lock_deinit(unit);

    return rv;
}

/*
 * Function:
 *      _bcm_esw_threads_shutdown
 * Purpose:
 *      Terminate all the spawned threads for specific unit. 
 * Parameters:
 *      unit - unit being detached
 * Returns:
 *	BCM_E_XXX
 */
STATIC int
_bcm_esw_threads_shutdown(int unit)
{
    int rv;     /* Operation return status. */

    rv = _bcm_esw_port_mon_stop(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

    rv = bcm_esw_linkscan_enable_set(unit, 0);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

#ifdef BCM_XGS_SWITCH_SUPPORT
    rv = soc_l2x_stop(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);
#endif /* BCM_XGS3_SWITCH_SUPPORT */

    rv = soc_counter_detach(unit);

    return (rv);
}

#define _BCM_MOD_INIT_CER(unit, rtn, dispname, init) {                      \
    if (init) {                                                             \
        sal_usecs_t stime;                                                  \
        int         rv;                                                     \
        soc_cm_debug(DK_VERBOSE, "bcm_init: (%s)\n", dispname);             \
        stime = sal_time_usecs();                                           \
        rv = rtn(unit);                                                     \
        BCM_CHECK_ERROR_RETURN(rv);                                         \
        soc_cm_debug(DK_VERBOSE, "bcm_init: %8s took %10d usec\n",          \
            dispname, SAL_USECS_SUB(sal_time_usecs(), stime));              \
    } else {                                                                \
        soc_cm_debug(DK_WARN, "bcm_init: skipped %s init\n", dispname);     \
    }                                                                       \
}

#define _BCM_MOD_INIT_IER(unit, rtn, dispname, init) {                      \
    if (init) {                                                             \
        sal_usecs_t stime;                                                  \
        int         rv;                                                     \
        soc_cm_debug(DK_VERBOSE, "bcm_init: (%s)\n", dispname);             \
        stime = sal_time_usecs();                                           \
        rv = rtn(unit);                                                     \
        BCM_IF_ERROR_RETURN(rv);                                            \
        soc_cm_debug(DK_VERBOSE, "bcm_init: %8s took %10d usec\n",          \
            dispname, SAL_USECS_SUB(sal_time_usecs(), stime));              \
    } else {                                                                \
        soc_cm_debug(DK_WARN, "bcm_init: skipped %s init\n", dispname);     \
    }                                                                       \
}

/*
 * Function:
 *	_bcm_modules_init
 * Purpose:
 * 	Initialize bcm modules
 * Parameters:
 *	unit - StrataSwitch unit #.
 *      flags - Combination of bit selectors (see init.h)
 * Returns:
 *	BCM_E_XXX
 */

STATIC int
_bcm_modules_init(int unit)
{
    int init_cond; /* init condition */
    /*
     * Initialize each bcm module
     */

    if (!SOC_UNIT_VALID(unit)) {
	return BCM_E_UNIT;
    }
    
    /* Must call mbcm init first to ensure driver properly installed */
    BCM_IF_ERROR_RETURN(mbcm_init(unit));

#if defined(BCM_WARM_BOOT_SUPPORT)    
    if (SOC_WARM_BOOT(unit) && !SOC_IS_XGS12_FABRIC(unit)) {
        /* Init local module id. */
        BCM_IF_ERROR_RETURN(bcm_esw_reload_stk_my_modid_get(unit));
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    /* When adding new modules, double check init condition 
     *  TRUE      - init the module always
     *  init_cond - Conditional init based on boot flags/soc properties
     */
#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(unit, soc_feature_virtual_switching) ||
        soc_feature(unit, soc_feature_gport_service_counters)) {
        /* Initialize the common data module here to avoid multiple
         * initializations in the required modules. */
        _BCM_MOD_INIT_CER(unit, _bcm_common_init, "common", TRUE);
    }

#endif /* BCM_TRIUMPH_SUPPORT */
    _BCM_MOD_INIT_CER(unit, bcm_esw_port_init, "port", TRUE);
    init_cond = (SAL_BOOT_BCMSIM ||
                 (!(SAL_BOOT_SIMULATION && 
                    soc_property_get(unit, spn_SKIP_L2_VLAN_INIT, 0))));
    _BCM_MOD_INIT_CER(unit, bcm_esw_l2_init, "l2", init_cond);
    _BCM_MOD_INIT_CER(unit, bcm_esw_stg_init, "stg", TRUE);
    _BCM_MOD_INIT_CER(unit, bcm_esw_vlan_init, "vlan", init_cond);
    init_cond = (SAL_BOOT_BCMSIM || (!SAL_BOOT_SIMULATION));
    _BCM_MOD_INIT_CER(unit, bcm_esw_trunk_init, "trunk", init_cond);
    _BCM_MOD_INIT_CER(unit, bcm_esw_cosq_init, "cosq", TRUE);
    _BCM_MOD_INIT_CER(unit, bcm_esw_mcast_init, "mcast", init_cond);
    _BCM_MOD_INIT_CER(unit, bcm_esw_linkscan_init, "linkscan", TRUE);
    _BCM_MOD_INIT_CER(unit, bcm_esw_stat_init, "stat", TRUE);
    _BCM_MOD_INIT_CER(unit, bcm_esw_stk_init, "stk", TRUE);
    _BCM_MOD_INIT_CER(unit, _bcm_esw_rate_init, "rate", init_cond);
#ifdef INCLUDE_KNET
    _BCM_MOD_INIT_CER(unit, bcm_esw_knet_init, "knet", TRUE);
#endif
#ifdef BCM_FILTER_SUPPORT
    if (soc_feature(unit, soc_feature_filter)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_filter_init, "filter", init_cond);
    }
#endif
#ifdef BCM_FIELD_SUPPORT
    if (!SOC_IS_SHADOW(unit)) {
        if (soc_feature(unit, soc_feature_field)) {
            _BCM_MOD_INIT_IER(unit, bcm_esw_field_init, "field", init_cond);
        }
    }
#endif
#if defined(BCM_FILTER_SUPPORT) || defined(BCM_FIELD_SUPPORT)
    if (soc_feature(unit, soc_feature_filter) ||
        soc_feature(unit, soc_feature_field) ) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_ds_init, "ds", init_cond);
    }
#endif
    _BCM_MOD_INIT_CER(unit, bcm_esw_mirror_init, "mirror", TRUE);
    _BCM_MOD_INIT_CER(unit, bcm_esw_tx_init, "tx", TRUE);
    _BCM_MOD_INIT_CER(unit, bcm_esw_rx_init, "rx", TRUE);
#ifdef INCLUDE_L3
    if (soc_feature(unit, soc_feature_l3)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_l3_init, "l3", init_cond);
    }
    if (soc_feature(unit, soc_feature_ip_mcast)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_ipmc_init, "ipmc", init_cond);
    }
#ifdef BCM_MPLS_SUPPORT
    if (soc_feature(unit, soc_feature_mpls)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_mpls_init, "mpls", init_cond);
    }
#endif
    if (soc_feature(unit, soc_feature_mim)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_mim_init, "mim", init_cond);
    }
    if (soc_feature(unit, soc_feature_subport)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_subport_init, "subport", init_cond);
    }
    if (soc_feature(unit, soc_feature_wlan)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_wlan_init, "wlan", init_cond);
    }
    _BCM_MOD_INIT_IER(unit, bcm_esw_proxy_init, "proxy", init_cond);

    if (soc_feature(unit, soc_feature_qos_profile)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_qos_init, "qos", init_cond);
    }
    if (soc_feature(unit, soc_feature_trill)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_trill_init, "trill", init_cond);
    }
    if (soc_feature(unit, soc_feature_niv)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_niv_init, "niv", init_cond);
    }
    /* This must be after the modules upon which it is build, in order
     * for Warm Boot to operate correctly. */
    _BCM_MOD_INIT_IER(unit, bcm_esw_multicast_init, "multicast", init_cond);
#endif
#ifdef BCM_DMUX_SUPPORT
    if (soc_feature(unit, soc_feature_dmux)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_dmux_init, "dmux", TRUE);
    }
#endif
#if defined(BCM_FILTER_SUPPORT) || defined(BCM_FIELD_SUPPORT)
    if (soc_feature(unit, soc_feature_filter) || \
        soc_feature(unit, soc_feature_field) ) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_auth_init, "auth", init_cond);
    }
#endif
#ifdef INCLUDE_REGEX
    if (soc_feature(unit, soc_feature_regex)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_regex_init, "regex", TRUE);
    }
#endif
    if (soc_feature(unit, soc_feature_time_support)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_time_init, "time", TRUE);
    }
    if (soc_feature(unit, soc_feature_oam)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_oam_init, "oam", init_cond);
    }
    if (soc_feature(unit, soc_feature_failover)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_failover_init, "failover", init_cond);
    }

#ifdef INCLUDE_CES
    if (soc_feature(unit, soc_feature_ces)) {
	_BCM_MOD_INIT_IER(unit, bcm_esw_ces_init, "ces", init_cond);
    }
#endif
#ifdef INCLUDE_PTP
    if (soc_feature(unit, soc_feature_ptp)) {
	_BCM_MOD_INIT_IER(unit, bcm_esw_ptp_init, "ptp", init_cond);
    }
#endif
#ifdef INCLUDE_BFD
    if (soc_feature(unit, soc_feature_bfd)) {
	_BCM_MOD_INIT_IER(unit, bcm_esw_bfd_init, "bfd", init_cond);
    }
#endif
#ifdef BCM_KATANA_SUPPORT
    /* Service meter */
    if (soc_feature(unit, soc_feature_global_meter)) {
        _BCM_MOD_INIT_IER(unit, bcm_esw_global_meter_init, "global_meter", init_cond);
    }
#endif /* BCM_KATANA_SUPPORT */
    if (SOC_WARM_BOOT(unit)) {
        SOC_WARM_BOOT_DONE(unit);
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_esw_init
 * Purpose:
 * 	Initialize the BCM API layer only, without resetting the switch chip.
 * Parameters:
 *	unit - StrataSwitch unit #.
 * Returns:
 *	BCM_E_XXX
 */

STATIC int
_bcm_esw_init(int unit)
{

    BCM_IF_ERROR_RETURN(_bcm_lock_init(unit));

    /* If linkscan is running, disable it. */
    bcm_esw_linkscan_enable_set(unit, 0);

#ifdef INCLUDE_MACSEC
    BCM_IF_ERROR_RETURN(_bcm_common_macsec_init(unit)); 
#endif /* INCLUDE_MACSEC */

#ifdef INCLUDE_FCMAP
    BCM_IF_ERROR_RETURN(_bcm_common_fcmap_init(unit)); 
#endif /* INCLUDE_FCMAP */

#if defined(INCLUDE_RCPU) && defined(BCM_XGS3_SWITCH_SUPPORT)
    BCM_IF_ERROR_RETURN(_bcm_esw_rcpu_master_init(unit)); 
#endif /* INCLUDE_RCPU && BCM_XGS3_SWITCH_SUPPORT */

    BCM_IF_ERROR_RETURN(_bcm_modules_init(unit));

#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit)) {
        BCM_IF_ERROR_RETURN(kt_s3mii_war(unit));
    }
#endif
#if defined(INCLUDE_RCPU) && defined(BCM_XGS3_SWITCH_SUPPORT)
    BCM_IF_ERROR_RETURN(_bcm_esw_rcpu_init(unit)); 
#endif /* INCLUDE_RCPU && BCM_XGS3_SWITCH_SUPPORT */

#if defined(BCM_HAWKEYE_SUPPORT) || defined(BCM_TRIUMPH2_SUPPORT) || defined(BCM_APOLLO_SUPPORT)
    BCM_IF_ERROR_RETURN(_bcm_esw_switch_init(unit));
#endif

    return BCM_E_NONE;
}    

/*
 * Function:
 *	bcm_esw_init
 * Purpose:
 * 	Initialize the BCM API layer only, without resetting the switch chip.
 * Parameters:
 *	unit - StrataSwitch unit #.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_esw_init(int unit)
{
    if (0 == SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    return _bcm_esw_init(unit);
}    

int
bcm_tr3_init(int unit)
{
    if (0 == SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    return _bcm_esw_init(unit);
}   

int
bcm_shadow_init(int unit)
{
    return bcm_esw_init(unit);
}    

/*
 * Function:
 *	_bcm_esw_reinit_s3mii_war
 * Purpose:
 * 	Partially ReInitialize the BCM API layer only.
 * 	Used for the SW Workaround for S3MII.
 * 	Presently used in Katana Only.
 * Parameters:
 *	unit - StrataSwitch unit #.
 * Returns:
 *	BCM_E_XXX
 */

int
_bcm_esw_reinit_ka_war(int unit)
{
    BCM_IF_ERROR_RETURN(_bcm_lock_init(unit));
    bcm_esw_linkscan_enable_set(unit, 0);
#ifdef INCLUDE_MACSEC
    BCM_IF_ERROR_RETURN(_bcm_common_macsec_init(unit)); 
#endif /* INCLUDE_MACSEC */
#ifdef INCLUDE_FCMAP
    BCM_IF_ERROR_RETURN(_bcm_common_fcmap_init(unit)); 
#endif /* INCLUDE_FCMAP */
#if defined(INCLUDE_RCPU) && defined(BCM_XGS3_SWITCH_SUPPORT)
    BCM_IF_ERROR_RETURN(_bcm_esw_rcpu_master_init(unit)); 
#endif /* INCLUDE_RCPU && BCM_XGS3_SWITCH_SUPPORT */
    SOC_IF_ERROR_RETURN(_bcm_modules_init(unit));

    return BCM_E_NONE;
}    

/*      _bcm_esw_attach
 * Purpose:
 *      Attach and initialize bcm device
 * Parameters:
 *      unit - unit being detached
 * Returns:
 *    BCM_E_XXX
 */
int
_bcm_esw_attach(int unit, char *subtype)
{
    int  dunit;
    int  rv;

    COMPILER_REFERENCE(subtype);

    BCM_CONTROL(unit)->capability |= BCM_CAPA_LOCAL;
    dunit = BCM_CONTROL(unit)->unit;

    /* Initialize soc layer */
    if ((NULL == SOC_CONTROL(dunit)) || 
        (0 == (SOC_CONTROL(dunit)->soc_flags & SOC_F_ATTACHED))) {
        return (BCM_E_INIT);
    }
    
    if (SAL_THREAD_ERROR == SOC_CONTROL(dunit)->counter_pid) {
        rv = soc_counter_attach(unit);
        BCM_IF_ERROR_RETURN(rv);
    }

    /* Initialize bcm layer */
    BCM_CONTROL(unit)->chip_vendor = SOC_PCI_VENDOR(dunit);
    BCM_CONTROL(unit)->chip_device = SOC_PCI_DEVICE(dunit);
    BCM_CONTROL(unit)->chip_revision = SOC_PCI_REVISION(dunit);
    if (SOC_IS_XGS_SWITCH(dunit)) {
        BCM_CONTROL(unit)->capability |= BCM_CAPA_SWITCH;
    }
    if (SOC_IS_XGS_FABRIC(dunit)) {
        BCM_CONTROL(unit)->capability |= BCM_CAPA_FABRIC;
    }
    if (soc_feature(dunit, soc_feature_l3)) {
        BCM_CONTROL(unit)->capability |= BCM_CAPA_L3;
    }
    if (soc_feature(dunit, soc_feature_ip_mcast)) {
        BCM_CONTROL(unit)->capability |=
            BCM_CAPA_IPMC;
    }

    /* Initialize port mappings */
    _bcm_api_xlate_port_init(unit);

    rv = _bcm_esw_init(unit);
    return (rv);
}

int
_bcm_tr3_attach(int unit, char *subtype)
{
    return _bcm_esw_attach(unit, subtype);
}

int
_bcm_shadow_attach(int unit, char *subtype)
{
    return _bcm_esw_attach(unit, subtype);
}

/*      _bcm_esw_match
 * Purpose:
 *      match BCM control subtype strings for ESW types 
 * Parameters:
 *      unit - unit being detached
 * Returns:
 *    0 match
 *    !0 no match
 */
int
_bcm_esw_match(int unit, char *subtype_a, char *subtype_b)
{
    COMPILER_REFERENCE(unit);
    return sal_strcmp(subtype_a, subtype_b);
}

int
_bcm_tr3_match(int unit, char *subtype_a, char *subtype_b)
{
    COMPILER_REFERENCE(unit);
    return _bcm_esw_match(unit, subtype_a, subtype_b);
}

int
_bcm_shadow_match(int unit, char *subtype_a, char *subtype_b)
{
    return _bcm_esw_match(unit, subtype_a, subtype_b);
}

/*
 * Function:
 *	bcm_esw_init_selective
 * Purpose:
 * 	Initialize specific bcm modules as desired.
 * Parameters:
 *	unit - StrataSwitch unit #.
 *    module_number - Indicate module number
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_esw_init_selective(int unit, uint32 module_number)
{
    switch (module_number) {
         case BCM_MODULE_PORT     :
                   BCM_IF_ERROR_RETURN(bcm_esw_port_init(unit));
                   break;
         case BCM_MODULE_L2       :
                   BCM_IF_ERROR_RETURN(bcm_esw_l2_init(unit));
                   break;
         case BCM_MODULE_VLAN     :   
                   BCM_IF_ERROR_RETURN(bcm_esw_vlan_init(unit));
                   break;
         case BCM_MODULE_TRUNK    :
                   BCM_IF_ERROR_RETURN(bcm_esw_trunk_init(unit));
                   break;
         case BCM_MODULE_COSQ     :
                   BCM_IF_ERROR_RETURN(bcm_esw_cosq_init(unit));
                   break;
         case BCM_MODULE_MCAST        :
                   BCM_IF_ERROR_RETURN(bcm_esw_mcast_init(unit));
                   break;
         case BCM_MODULE_LINKSCAN  :
                   BCM_IF_ERROR_RETURN(bcm_esw_linkscan_init(unit));
                   break;
         case BCM_MODULE_STAT     :
                   BCM_IF_ERROR_RETURN(bcm_esw_stat_init(unit));
                   break;
#ifdef BCM_FILTER_SUPPORT
         case BCM_MODULE_FILTER   :
                   BCM_IF_ERROR_RETURN(bcm_esw_filter_init(unit));
                   break;
#endif /* BCM_FILTER_SUPPORT */
#if defined(BCM_FILTER_SUPPORT) || defined(BCM_FIELD_SUPPORT)
         case BCM_MODULE_DIFFSERV :
                   BCM_IF_ERROR_RETURN(bcm_esw_ds_init(unit));
                   break;
#endif
         case BCM_MODULE_MIRROR   :
                   BCM_IF_ERROR_RETURN(bcm_esw_mirror_init(unit));
                   break;
#ifdef INCLUDE_L3
         case BCM_MODULE_L3       :
                   BCM_IF_ERROR_RETURN(bcm_esw_l3_init(unit));
                   BCM_IF_ERROR_RETURN(bcm_esw_proxy_init(unit));
                   break;
         case BCM_MODULE_IPMC 	:
                   BCM_IF_ERROR_RETURN(bcm_esw_ipmc_init(unit));
                   break;
#ifdef BCM_MPLS_SUPPORT
         case BCM_MODULE_MPLS	 :
                   BCM_IF_ERROR_RETURN(bcm_esw_mpls_init(unit));
                   break;
#endif /* BCM_MPLS_SUPPORT */
         case BCM_MODULE_MIM 	 :
                   BCM_IF_ERROR_RETURN(bcm_esw_mim_init(unit));
                   break;
         case BCM_MODULE_SUBPORT	:
                   BCM_IF_ERROR_RETURN(bcm_esw_subport_init(unit));
                   break;
         case BCM_MODULE_WLAN	  :
                   BCM_IF_ERROR_RETURN(bcm_esw_wlan_init(unit));
                   break;
         case BCM_MODULE_QOS		:
                   BCM_IF_ERROR_RETURN(bcm_esw_qos_init(unit));
                   break;
#endif /* INCLUDE_L3 */
         case BCM_MODULE_STACK    :
                   BCM_IF_ERROR_RETURN(bcm_esw_stk_init(unit));
                   break;
         case BCM_MODULE_STG      :
                   BCM_IF_ERROR_RETURN(bcm_esw_stg_init(unit));
                   break;
         case BCM_MODULE_TX       :
                   BCM_IF_ERROR_RETURN(bcm_esw_tx_init(unit));
                   break;
#ifdef BCM_DMUX_SUPPORT
         case BCM_MODULE_DMUX     :
                   BCM_IF_ERROR_RETURN(bcm_esw_dmux_init(unit));
                   break;
#endif /* BCM_DMUX_SUPPORT */
#if defined(BCM_FILTER_SUPPORT) || defined(BCM_FIELD_SUPPORT)
         case BCM_MODULE_AUTH     :
                   BCM_IF_ERROR_RETURN(bcm_esw_auth_init(unit));
                   break;
#endif
         case BCM_MODULE_RX       :
                   BCM_IF_ERROR_RETURN(bcm_esw_rx_init(unit));
                   break;
#ifdef BCM_FIELD_SUPPORT
         case BCM_MODULE_FIELD    :
                   BCM_IF_ERROR_RETURN(bcm_esw_field_init(unit));
                   break;
#endif /* BCM_FIELD_SUPPORT */
         case BCM_MODULE_TIME     :
                   BCM_IF_ERROR_RETURN(bcm_esw_time_init(unit));
                   break;
         case BCM_MODULE_FABRIC   :
                   break;
         case BCM_MODULE_POLICER  :
                   break;
         case BCM_MODULE_OAM      :
                   BCM_IF_ERROR_RETURN(bcm_esw_oam_init(unit));
                   break;
         case BCM_MODULE_FAILOVER :
                   BCM_IF_ERROR_RETURN(bcm_esw_failover_init(unit));
                   break;
         case BCM_MODULE_VSWITCH  :
                   break;
#ifdef  INCLUDE_L3
         case BCM_MODULE_MULTICAST:
                   BCM_IF_ERROR_RETURN(bcm_esw_multicast_init(unit));
                   break;
         case BCM_MODULE_TRILL    :
                   BCM_IF_ERROR_RETURN(bcm_esw_trill_init(unit));
                   break;
         case BCM_MODULE_NIV      :
                   BCM_IF_ERROR_RETURN(bcm_esw_niv_init(unit));
                   break;
#endif
#ifdef INCLUDE_REGEX
         case BCM_MODULE_REGEX :
                   BCM_IF_ERROR_RETURN(bcm_esw_regex_init(unit));
                   break;
#endif /* BCM_MPLS_SUPPORT */
#ifdef INCLUDE_CES
         case BCM_MODULE_CES:
                   BCM_IF_ERROR_RETURN(bcm_esw_ces_init(unit));
                   break;
#endif 
#ifdef INCLUDE_PTP
         case BCM_MODULE_PTP:
                   BCM_IF_ERROR_RETURN(bcm_esw_ptp_init(unit));
                   break;
#endif 
#ifdef INCLUDE_BFD
         case BCM_MODULE_BFD:
                   BCM_IF_ERROR_RETURN(bcm_esw_bfd_init(unit));
                   break;
#endif 
         default:
                   return BCM_E_UNAVAIL;
    }
    return BCM_E_NONE;
}

/*
 * ************* Deprecated API ************
 * Function:
 *	bcm_esw_init_check
 * Purpose:
 *	Return TRUE if bcm_esw_init_bcm has already been called and succeeded
 * Parameters:
 *	unit- StrataSwitch unit #.
 * Returns:
 *	TRUE or FALSE
 */
int
bcm_esw_init_check(int unit)
{
    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      _bcm_esw_detach
 * Purpose:
 *      Clean up bcm layer when unit is detached
 * Parameters:
 *      unit - unit being detached
 * Returns:
 *	BCM_E_XXX
 */
int
_bcm_esw_detach(int unit)
{
    int rv;                    /* Operation return status. */

    /* Shut down all the spawned threads. */
    rv = _bcm_esw_threads_shutdown(unit);
    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

    /* 
     *  Don't move up, holding lock or disabling hw operations 
     *  might prevent theads clean exit.
     */
    BCM_LOCK(unit);

    rv = _bcm_esw_modules_deinit(unit);

    BCM_IF_ERROR_NOT_UNAVAIL_RETURN(rv);

    return rv;
}

int
_bcm_tr3_detach(int unit)
{
    return _bcm_esw_detach(unit);
}

int
_bcm_shadow_detach(int unit)
{
    return _bcm_esw_detach(unit);
}

/*
 * Function:
 *	bcm_esw_info_get
 * Purpose:
 *	Provide unit information to caller
 * Parameters:
 *	unit	- switch device
 *	info	- (OUT) bcm unit info structure
 * Returns:
 *	BCM_E_XXX
 */
int
bcm_esw_info_get(int unit, bcm_info_t *info)
{
    uint16 dev_id = 0;
    uint8 rev_id = 0;

    if (!SOC_UNIT_VALID(unit)) {
	return BCM_E_UNIT;
    }
    if (info == NULL) {
	return BCM_E_PARAM;
    }
    soc_cm_get_id(unit, &dev_id, &rev_id);
    info->vendor = SOC_PCI_VENDOR(unit);
    info->device = dev_id;
    info->revision = rev_id;
    info->capability = 0;
    if (SOC_IS_XGS_FABRIC(unit)) {
	info->capability |= BCM_INFO_FABRIC;
    } else {
	info->capability |= BCM_INFO_SWITCH;
    }
    if (soc_feature(unit, soc_feature_l3)) {
	info->capability |= BCM_INFO_L3;
    }
    if (soc_feature(unit, soc_feature_ip_mcast)) {
	info->capability |= BCM_INFO_IPMC;
    }
    return BCM_E_NONE;
}


/* ASSUMES unit PARAMETER which is not in macro's list. */
#define CLEAR_CALL(_rtn, _name) {                                       \
        int rv;                                                         \
        rv = (_rtn)(unit);                                              \
        if (rv < 0 && rv != BCM_E_UNAVAIL) {                            \
            soc_cm_debug(DK_ERR, "bcm_clear %d: %s failed %d. %s\n",    \
                         unit, _name, rv, bcm_errmsg(rv));              \
            return rv;                                                  \
        }                                                               \
}

/*
 * Function:
 *      bcm_esw_clear
 * Purpose:
 *      Initialize a device without a full reset
 * Parameters:
 *      unit        - The unit number of the device to clear
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      For each module, call the underlying init/clear operation
 */
int
bcm_esw_clear(int unit)
{
    return _bcm_esw_init(unit);
}
