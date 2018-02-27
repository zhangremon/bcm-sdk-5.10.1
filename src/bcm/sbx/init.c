/*
 * $Id: init.c 1.80.6.1 Broadcom SDK $
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
 * BCM-SBX Library Initialization
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
 *      PBMP = all switching Ethernet ports (non-fabric) and the CPU.
 *      UBMP = all switching Ethernet ports (non-fabric).
 *   No trunks configured
 *   No mirroring configured
 *   All L2 and L3 tables empty
 *   Ingress VLAN filtering disabled
 *   BPDU reception enabled
 */

#include <sal/types.h>
#include <sal/core/time.h>
#include <sal/core/boot.h>

#include <soc/drv.h>
#include <soc/l2x.h>
#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/counter.h>

#include <bcm_int/control.h>
#include <bcm/error.h>
#include <bcm/cosq.h>
#include <bcm/init.h>
#include <bcm/l2.h>
#include <bcm/link.h>
#include <bcm/rx.h>
#include <bcm/stat.h>
#include <bcm/stg.h>
#include <bcm/vlan.h>
#include <bcm/vswitch.h>
#include <bcm/trunk.h>
#include <bcm/stack.h>
#include <bcm/l3.h>
#include <bcm/ipmc.h>
#include <bcm/mpls.h>
#include <bcm/igmp.h>
#include <bcm/fabric.h>
#include <bcm/policer.h>
#include <bcm/oam.h>
#include <bcm/field.h>
#include <bcm/mirror.h>
#include <bcm/mcast.h>
#include <bcm/qos.h>

#include <soc/sbx/qe2000.h>
#include <soc/sbx/bme3200.h>
#include <soc/sbx/bm9600.h>
#include <soc/sbx/error.h>
#include <bcm_int/common/lock.h>
#include <bcm_int/common/family.h>
#ifdef BCM_FE2000_SUPPORT
#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#endif
#include <bcm_int/fe2000_dispatch.h>
#include <bcm_int/sbx/fe2000/allocator.h>
#include <bcm_int/sbx/fe2000/recovery.h>
#include <bcm_int/sbx/fe2000/switch.h>
#endif /* BCM_FE2000_SUPPORT */
#include <bcm_int/sbx/mbcm.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/failover.h>
#include <bcm_int/sbx_dispatch.h>

#ifdef INCLUDE_MACSEC
#include <bcm_int/common/macsec_cmn.h>
#endif /* INCLUDE_MACSEC */

#ifdef INCLUDE_FCMAP
#include <bcm_int/common/fcmap_cmn.h>
#endif /* INCLUDE_FCMAP */

/* See BCM_SEL_INIT flags in init.h */
STATIC bcm_sbx_state_t _sbx_state[BCM_MAX_NUM_UNITS];

#ifdef BCM_FE2000_SUPPORT

extern int bcm_fe2000_l2cache_detach(int unit);
extern int bcm_fe2000_mirror_detach(int unit);
extern int bcm_fe2000_policer_detach(int unit);
extern int bcm_fe2000_vlan_detach(int unit);
extern int _bcm_fe2000_port_deinit(int unit);
extern int bcm_fe2000_mcast_detach(int unit);
extern int bcm_fe2000_stg_detach(int unit);
#endif

extern int bcm_sbx_port_clear(int unit);

STATIC int
_bcm_sbx_lock_init(int unit)
{
    if (_bcm_lock[unit] == NULL) {
        _bcm_lock[unit] = sal_mutex_create("bcm_sbx_config_lock");
    }

    if (_bcm_lock[unit] == NULL) {
        return BCM_E_MEMORY;
    }

    return BCM_E_NONE;
}

/* assumes unit and rv are declared in local scope */
#define INIT_CALL_CHK( m )             \
  rv = bcm_##m##_init(unit);                                            \
  if (BCM_FAILURE(rv)) {                                               \
      BCM_DEBUG( BCM_DBG_ERR, (                                        \
                "bcm_init(%d): " #m " failed (%d -- %s)\n",            \
                unit, rv, bcm_errmsg(rv)));                            \
      return rv;                                                       \
  }

#ifdef BCM_FE2000_SUPPORT
int
bcm_sbx_fe2000_init(int unit)
{
    int rv;

#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_IS_SBX_FE2000(unit)) {
        int rv = _bcm_fe2000_recovery_init(unit);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    }
#endif  /* BCM_WARM_BOOT_SUPPORT */

    if (SOC_IS_SBX_G2P3(unit)) {



            INIT_CALL_CHK(trunk);
            INIT_CALL_CHK(failover);
            INIT_CALL_CHK(qos);
            INIT_CALL_CHK(stk);
            INIT_CALL_CHK(port);
            INIT_CALL_CHK(stg);
            INIT_CALL_CHK(vlan);
            INIT_CALL_CHK(vswitch);
            INIT_CALL_CHK(l2);
            INIT_CALL_CHK(cosq);
            INIT_CALL_CHK(linkscan);
            INIT_CALL_CHK(rx);
            INIT_CALL_CHK(stat);
            INIT_CALL_CHK(l3);
            INIT_CALL_CHK(mpls);
            INIT_CALL_CHK(ipmc);
            INIT_CALL_CHK(policer);
            INIT_CALL_CHK(field);
            INIT_CALL_CHK(mcast);
            INIT_CALL_CHK(mirror);
            INIT_CALL_CHK(tx);
            INIT_CALL_CHK(oam);
            INIT_CALL_CHK(mim);

    } else if (SOC_IS_SBX_G2XX(unit)) {

        INIT_CALL_CHK(port);
        INIT_CALL_CHK(linkscan);
        INIT_CALL_CHK(rx);
        INIT_CALL_CHK(tx);
    } else {
        return BCM_E_PARAM;
    }


    if (SOC_IS_SBX_G2P3(unit)) {
        INIT_CALL_CHK(igmp_snooping);
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    BCM_IF_ERROR_RETURN(
        _bcm_fe2000_recovery_commit(unit));
#endif  /* BCM_WARM_BOOT_SUPPORT */

    return rv;
}
#endif /* BCM_FE2000_SUPPORT */



#ifdef BCM_BME3200_SUPPORT
int
bcm_sbx_bm3200_init(int unit)
{
    int rv;

    INIT_CALL_CHK(stk);
    INIT_CALL_CHK(fabric);
    INIT_CALL_CHK(port);
    INIT_CALL_CHK(cosq);
    INIT_CALL_CHK(linkscan);
    return BCM_E_NONE;
}
#endif /* BCM_BME3200_SUPPORT */


#ifdef BCM_BM9600_SUPPORT
int
bcm_sbx_bm9600_init(int unit)
{
    int rv;
    INIT_CALL_CHK(stk);
    INIT_CALL_CHK(fabric);
    INIT_CALL_CHK(port);
    INIT_CALL_CHK(cosq);
    INIT_CALL_CHK(linkscan);
    return BCM_E_NONE;
}
#endif /* BCM_BM9600_SUPPORT */


#ifdef BCM_QE2000_SUPPORT
int
bcm_sbx_qe2000_init(int unit)
{
    int rv;

    INIT_CALL_CHK(stk);
    INIT_CALL_CHK(fabric);
    INIT_CALL_CHK(port);
    INIT_CALL_CHK(vlan);
    INIT_CALL_CHK(trunk);
    INIT_CALL_CHK(cosq);
    INIT_CALL_CHK(linkscan);
    INIT_CALL_CHK(rx);
    INIT_CALL_CHK(stat);
    INIT_CALL_CHK(tx);        
    
    return BCM_E_NONE;
}
#endif /* BCM_QE2000_SUPPORT */

#ifdef BCM_SIRIUS_SUPPORT
int
bcm_sbx_sirius_init(int unit)
{
    int rv;

    INIT_CALL_CHK(stk);
    INIT_CALL_CHK(fabric);
    INIT_CALL_CHK(port);
    INIT_CALL_CHK(cosq);
    INIT_CALL_CHK(trunk);
    INIT_CALL_CHK(linkscan);
    INIT_CALL_CHK(rx);
    INIT_CALL_CHK(stat);
    INIT_CALL_CHK(multicast);
    INIT_CALL_CHK(tx);

    return BCM_E_NONE;
}
#endif /* BCM_SIRIUS_SUPPORT */


int
bcm_sbx_init(int unit)
{
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    /* init state pointer */
    SOC_SBX_CONTROL(unit)->state = &(_sbx_state[unit]);

#ifdef BCM_FE2000_SUPPORT
    if (SOC_IS_SBX_FE2000(unit)) {

        /* Initialize the microcode library */
#ifdef BCM_FE2000_P3_SUPPORT
        if (SOC_IS_SBX_G2P3(unit)) {
            BCM_IF_ERROR_RETURN(_sbx_gu2_resource_init(unit));
            BCM_IF_ERROR_RETURN(_sbx_gu2_alloc_wellknown_resources(unit));
            BCM_IF_ERROR_RETURN(_bcm_fe2000_switch_control_init(unit));
        }
#endif /* BCM_FE2000_P3_SUPPORT */

    }
#endif /* BCM_FE2000_SUPPORT */


#ifdef BCM_BME3200_SUPPORT
    if (SOC_IS_SBX_BME3200(unit)) {
        /* Must call mbcm init first to ensure driver properly installed */
        BCM_IF_ERROR_RETURN(mbcm_sbx_init(unit));
    }
#endif /* BCM_BME3200_SUPPORT */


#ifdef BCM_BM9600_SUPPORT
    if (SOC_IS_SBX_BM9600(unit)) {
        /* Must call mbcm init first to ensure driver properly installed */
        BCM_IF_ERROR_RETURN(mbcm_sbx_init(unit));
    }
#endif  /* BCM_BM9600_SUPPORT */


#ifdef BCM_QE2000_SUPPORT
    if (SOC_IS_SBX_QE2000(unit)) {
        /* Must call mbcm init first to ensure driver properly installed */
        BCM_IF_ERROR_RETURN(mbcm_sbx_init(unit));

#ifdef BCM_FE2000_SUPPORT
        /*
         * Initialize the EP microcode library
         */
        if (SOC_SBX_CFG_QE2000(unit)->bEpDisable == FALSE) {
            BCM_IF_ERROR_RETURN(gu2_eplib_init(unit));
        }
#endif /* BCM_FE2000_SUPPORT */

    }
#endif /* BCM_QE2000_SUPPORT */


#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
        /* Must call mbcm init first to ensure driver properly installed */
        BCM_IF_ERROR_RETURN(mbcm_sbx_init(unit));
    }
#endif  /* BCM_SIRIUS_SUPPORT */


    BCM_IF_ERROR_RETURN(_bcm_sbx_lock_init(unit));

    /* If linkscan is running, disable it. */
    BCM_IF_ERROR_RETURN(bcm_linkscan_enable_set(unit, 0));

#ifdef INCLUDE_MACSEC
    BCM_IF_ERROR_RETURN(_bcm_common_macsec_init(unit));
#endif /* INCLUDE_MACSEC */

#ifdef INCLUDE_FCMAP
    BCM_IF_ERROR_RETURN(_bcm_common_fcmap_init(unit));
#endif /* INCLUDE_FCMAP */


    /*  The call sequence below replaces bcm_init_selective calls */
    bcm_chip_family_set(unit, BCM_FAMILY_SBX);
    if (0) {
    }
#ifdef BCM_FE2000_SUPPORT
    else if (SOC_IS_SBX_FE2000(unit)) {
        BCM_IF_ERROR_RETURN(bcm_sbx_fe2000_init(unit));
    }
#endif
#ifdef BCM_BME3200_SUPPORT
    else if (SOC_IS_SBX_BME3200(unit)) {
        BCM_IF_ERROR_RETURN(bcm_sbx_bm3200_init(unit));
    }
#endif
#ifdef BCM_BM9600_SUPPORT
    else if (SOC_IS_SBX_BM9600(unit)) {
        BCM_IF_ERROR_RETURN(bcm_sbx_bm9600_init(unit));
    }
#endif
#ifdef BCM_QE2000_SUPPORT
    else if (SOC_IS_SBX_QE2000(unit)) {
        BCM_IF_ERROR_RETURN(bcm_sbx_qe2000_init(unit));
    }
#endif
#ifdef BCM_SIRIUS_SUPPORT
    else if (SOC_IS_SIRIUS(unit)) {
        BCM_IF_ERROR_RETURN(bcm_sbx_sirius_init(unit));
    }
#endif

    if (!SOC_IS_SBX_FE(unit)) {
        BCM_IF_ERROR_RETURN
            (bcm_sbx_failover_init(unit));
    }

    return BCM_E_NONE;
}

int
bcm_sbx_info_get(int unit,
                 bcm_info_t *info)
{
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }
    if (info == NULL) {
        return BCM_E_PARAM;
    }
    info->vendor = SOC_PCI_VENDOR(unit);
    info->device = SOC_PCI_DEVICE(unit);
    info->revision = SOC_PCI_REVISION(unit);
    info->capability = 0;
#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
        info->capability |= BCM_INFO_FABRIC;
    }
#endif
    if (soc_feature(unit, soc_feature_l3)) {
        info->capability |= BCM_INFO_L3;
    }
    if (soc_feature(unit, soc_feature_ip_mcast)) {
        info->capability |= BCM_INFO_IPMC;
    }
    return BCM_E_NONE;
}

int
bcm_sbx_init_check(int unit)
{
    return BCM_E_UNAVAIL;
}


int
bcm_sbx_init_selective(int unit, uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_clear(int unit)
{
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *         _bcm_sbx_modules_deinit
 * Purpose:
 *         De-initialize bcm modules
 * Parameters:
 *     unit - (IN) BCM device number.
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_sbx_modules_deinit(int unit)
{
    int rv = BCM_E_NONE;
    
    /* Detach, ignoring uninit errors */
#define  SBX_DETACH( detach_f )                                              \
  rv = detach_f(unit);                                                       \
  if (rv == BCM_E_INIT) { rv = BCM_E_NONE; }                                 \
  if (BCM_FAILURE(rv)) {                                                     \
      soc_cm_print(_SBX_D(unit, #detach_f " failed: %s\n"), bcm_errmsg(rv)); \
  }                                                                          \
  BCM_IF_ERROR_RETURN(rv); 


    if (SOC_IS_SBX_FE2000(unit)) {
#ifdef BCM_WARM_BOOT_SUPPORT
        SBX_DETACH(_bcm_fe2000_recovery_deinit);
#endif  /* BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_FE2000_SUPPORT
        SBX_DETACH(bcm_fe2000_oam_detach);

#ifdef INCLUDE_L3
        SBX_DETACH(bcm_fe2000_mpls_cleanup);
        SBX_DETACH(bcm_fe2000_ipmc_detach);
        SBX_DETACH(bcm_fe2000_l3_cleanup);
#endif /* INCLUDE_L3 */

        SBX_DETACH(bcm_fe2000_qos_detach);
        SBX_DETACH(bcm_fe2000_l2_detach);
        SBX_DETACH(bcm_fe2000_mcast_detach);
        SBX_DETACH(bcm_fe2000_mirror_detach);
        SBX_DETACH(bcm_fe2000_field_detach);
        SBX_DETACH(bcm_fe2000_policer_detach);
        SBX_DETACH(bcm_fe2000_vswitch_detach);
        SBX_DETACH(bcm_fe2000_vlan_detach);
        SBX_DETACH(bcm_fe2000_trunk_detach);
        SBX_DETACH(bcm_fe2000_stg_detach);
        SBX_DETACH(_bcm_fe2000_port_deinit);
        SBX_DETACH(_sbx_gu2_resource_uninit);
#endif /* BCM_FE2000_SUPPORT */
    } else {
        SBX_DETACH(bcm_sbx_cosq_detach);
    }

    SBX_DETACH(bcm_common_linkscan_detach);
    SBX_DETACH(bcm_sbx_rx_clear);


#undef SBX_DETACH
    return rv;
}
int
_bcm_sbx_attach(int unit, char *subtype)
{
    int         dunit;

    COMPILER_REFERENCE(subtype);

    BCM_CONTROL(unit)->capability |= BCM_CAPA_LOCAL;
    dunit = BCM_CONTROL(unit)->unit;
    if (SOC_UNIT_VALID(dunit)) {
        BCM_CONTROL(unit)->chip_vendor = SOC_PCI_VENDOR(dunit);
        BCM_CONTROL(unit)->chip_device = SOC_PCI_DEVICE(dunit);
        BCM_CONTROL(unit)->chip_revision = SOC_PCI_REVISION(dunit);

        if (SOC_IS_SBX_FE(dunit)) {
            BCM_CONTROL(unit)->capability |= BCM_CAPA_SWITCH_SBX;
        } else if (SOC_IS_SBX_QE(dunit) ||
                   SOC_IS_SBX_BME(dunit)) {
            BCM_CONTROL(unit)->capability |= BCM_CAPA_FABRIC_SBX;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_sbx_threads_shutdown
 * Purpose:
 *      Terminate all the spawned threads for specific unit.
 * Parameters:
 *      unit - unit being detached
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_sbx_threads_shutdown(int unit)
{
    int rv;     /* Operation return status. */

    rv = bcm_linkscan_enable_set(unit, 0);
    BCM_IF_ERROR_RETURN(rv);

    return rv;
}

int
_bcm_sbx_detach(int unit)
{
    int rv;                    /* Operation return status. */
    int rv_error = BCM_E_NONE; /* Final return status.     */

    /* Shut down all the spawned threads. */
    rv = _bcm_sbx_threads_shutdown(unit);
    BCM_IF_ERROR_RETURN(rv);

    /* 
     *  Don't move up, holding lock or disabling hw operations
     *  might prevent theads clean exit.
     */
    BCM_LOCK(unit);

    rv = _bcm_sbx_modules_deinit(unit);
    if (BCM_FAILURE(rv)) {
        rv_error = rv;
    }
    
    BCM_UNLOCK(unit);

    return rv;
}

/*      _bcm_sbx_match
 * Purpose:
 *      match BCM control subtype strings for SBX types 
 * Parameters:
 *      unit - unit being detached
 * Returns:
 *    0 match
 *    !0 no match
 */
int
_bcm_sbx_match(int unit, char *subtype_a, char *subtype_b)
{
    COMPILER_REFERENCE(unit);
    return sal_strcmp(subtype_a, subtype_b);
}
