/*
 * $Id: trunk.c 1.198.2.14 Broadcom SDK $
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
 * File:    trunk.c
 * Purpose: BCM level APIs for trunking (a.k.a. Port Aggregation)
 */

#include <sal/types.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/drv.h>
#if defined(BCM_BRADLEY_SUPPORT)
#include <soc/bradley.h>
#endif /* BCM_BRADLEY_SUPPORT */

#include <bcm/error.h>
#include <bcm/mirror.h>
#include <bcm/trunk.h>

#include <bcm_int/api_xlate_port.h>
#include <bcm_int/common/lock.h>
#include <bcm_int/common/multicast.h>
#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/trunk.h>
#include <bcm_int/esw/port.h>
#include <bcm_int/esw/mirror.h>
#include <bcm_int/esw/firebolt.h>
#include <bcm_int/esw/stack.h>
#include <bcm_int/esw/trident.h>

#include <bcm_int/esw_dispatch.h>
#include <bcm_int/esw/switch.h>

#ifdef BCM_WARM_BOOT_SUPPORT
#include <bcm/types.h>
#endif /* BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
#include <appl/diag/system.h>
#include <appl/diag/parse.h>
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

/*
 * Accesses to the trunk hardware tables and software state
 * are made atomic using the TTR memory table lock.  The TTR lock
 * protects ALL related hardware tables, even if the particular chip
 * doesn't have a TTR table.
 */

#define TRUNK_LOCK(unit)	_bcm_esw_trunk_lock(unit)
#define TRUNK_UNLOCK(unit)	_bcm_esw_trunk_unlock(unit)

typedef struct trunk_cntl_s {
    int                ngroups_fp;  /* number of (front panel) trunk groups */
    int                nports_fp;   /* max ports per trunk group */
    int                ngroups_hg;  /* number of fabric trunk groups */
    int                nports_hg;   /* max ports per fabric trunk group */
    trunk_private_t   *t_info;
    sal_mutex_t        lock;        /* Trunk module lock. */
#ifdef BCM_WARM_BOOT_SUPPORT
    SHR_BITDCL  *trunk_bmp_ptr;         /* Warm Boot trunk use bitmap */
    int         trunk_bmp_alloc;        /* WB trunk use allocation */
    uint8       *hw_failover_flags;     /* WB HW failover flags record */
    int         hw_failover_alloc;      /* WB HW failover allocation */
#endif /* BCM_WARM_BOOT_SUPPORT */
} trunk_cntl_t;

/*
 * One trunk control entry for each SOC device containing trunk book keeping
 * info for that device.
 */
static trunk_cntl_t bcm_trunk_control[BCM_MAX_NUM_UNITS];

#define TRUNK_CNTL(unit)	bcm_trunk_control[unit]
#define TRUNK_INFO(unit, tid)	bcm_trunk_control[unit].t_info[tid]

/*
 * Cause a routine to return BCM_E_INIT if trunking subsystem is not
 * initialized.
 */

#define TRUNK_INIT(unit)	                    \
	if ( (TRUNK_CNTL(unit).ngroups_fp <= 0) && \
	     (TRUNK_CNTL(unit).ngroups_hg <= 0) ) { return BCM_E_INIT; }

#define TRUNK_NUM_GROUPS(unit) \
        (TRUNK_CNTL(unit).ngroups_fp + TRUNK_CNTL(unit).ngroups_hg)

/*
 * Make sure TID is within valid range.
 */

#define TRUNK_CHECK(unit, tid) \
	if ((tid) < 0 || (tid) >= TRUNK_NUM_GROUPS(unit)) \
           { return BCM_E_PARAM; }

#define TRUNK_FP_TID(unit, tid) \
    ( ((tid) >= 0) && ((tid) < TRUNK_CNTL(unit).ngroups_fp) )

#define TRUNK_FABRIC_TID(unit, tid)             \
    ( ((tid) >= TRUNK_CNTL(unit).ngroups_fp) && \
      ((tid) < TRUNK_NUM_GROUPS(unit)) )

/*
 * Function:
 *	_bcm_esw_trunk_gport_construct
 * Purpose:
 *	Converts ports and modules given in t_data structure into gports
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      fabric_trunk - trunk is a fabric trunk
 *      count - number of failover gports in list
 *      port_list - (IN) list of port numbers
 *      modid_list - (IN) list of module ids
 *      port_array - (OUT) list of gports to return
 *
 * Note:
 *      port_list and port_array may be the same list.  This updates
 *      the list in place.
 */
int
_bcm_esw_trunk_gport_construct(int unit, int fabric_trunk, int count,
                               bcm_port_t *port_list,
                               bcm_module_t *modid_list,
                               bcm_gport_t *port_array)
{
    bcm_gport_t         gport;
    int                 i, mod_is_local;
    _bcm_gport_dest_t   dest;

    _bcm_gport_dest_t_init(&dest);
    
    for (i = 0; i < count; i++) {
        gport = 0;
        /* Stacking ports should be encoded as devport */
        if (fabric_trunk) {
            /* Error checking during set functions should guarantee that
             * stack ports iff fabric trunks */
            dest.gport_type = _SHR_GPORT_TYPE_DEVPORT;
            BCM_IF_ERROR_RETURN
                (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET, modid_list[i],
                                        port_list[i], &dest.modid, &dest.port));
        } else {
            BCM_IF_ERROR_RETURN
                (_bcm_esw_modid_is_local(unit, modid_list[i],
                                         &mod_is_local));
            if (mod_is_local && IS_ST_PORT(unit, port_list[i])) {
                dest.gport_type = _SHR_GPORT_TYPE_DEVPORT;
            } else {
                dest.gport_type = _SHR_GPORT_TYPE_MODPORT;
                dest.modid = modid_list[i];
            }
            dest.port = port_list[i];
        }
        BCM_IF_ERROR_RETURN
            (_bcm_esw_gport_construct(unit, &dest, &gport));
        port_array[i] = gport;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_esw_trunk_gport_array_resolve
 * Purpose:
 *	Converts gports list into ports and modules
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      fabric_trunk - trunk is a fabric trunk
 *      count - number of failover gports in list
 *      port_array - (IN) list of gports
 *      port_list - (OUT) list of port numbers to return
 *      modid_list - (OUT) list of module ids to return
 *
 * Note:
 *      port_list and port_array may be the same list.  This updates
 *      the list in place.
 */
int
_bcm_esw_trunk_gport_array_resolve(int unit, int fabric_trunk, int count,
                                   bcm_gport_t *port_array,
                                   bcm_port_t *port_list,
                                   bcm_module_t *modid_list)
                             
{
    bcm_port_t      port;
    bcm_module_t    modid, local_modid;
    bcm_trunk_t     tgid;
    int             i, id, rv;

    rv = bcm_esw_stk_my_modid_get(unit, &local_modid);
    if (BCM_FAILURE(rv) && !SOC_IS_XGS_FABRIC(unit) ) {
        return (rv);
    }

    for (i = 0; i < count; i++) {
        if (BCM_GPORT_IS_SET(port_array[i])) {
            if (fabric_trunk) {
                /* Must be local */
                BCM_IF_ERROR_RETURN
                    (bcm_esw_port_local_get(unit, port_array[i], &port));
                port_list[i] = port;
                modid_list[i] = -1;
            } else {
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_gport_resolve(unit, port_array[i],
                                            &modid, &port, &tgid, &id));
                if ((-1 != tgid) || (-1 != id)) {
                    return BCM_E_PARAM;
                }
                port_list[i] = port;
                modid_list[i] = modid;
            }
        } else {
            if (fabric_trunk) {
                if (SOC_PORT_VALID(unit, port_array[i])) {
                    port_list[i] = port_array[i];
                    modid_list[i] = -1;
                } else {
                    return BCM_E_PORT;
                }
            } else {
                PORT_DUALMODID_VALID(unit, port_array[i]);
                if (port_array != port_list) {
                    port_list[i] = port_array[i];
                    modid_list[i] = local_modid;
                }
            }
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_esw_trunk_gport_resolve
 * Purpose:
 *	Converts gports given in t_data structure into ports and modules
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      tid - The trunk ID to be affected.
 *      t_data -   Information on the trunk group.
 */
int
_bcm_esw_trunk_gport_resolve(int unit, bcm_trunk_t tid,
                             bcm_trunk_add_info_t *t_data)
{
    int             i;
    _bcm_gport_dest_t gp_dest;
    bcm_gport_t gport_list[BCM_TRUNK_MAX_PORTCNT];

    gp_dest.gport_type = _SHR_GPORT_TYPE_MODPORT;
    for (i = 0; i < t_data->num_ports; i++) {
        if (BCM_GPORT_IS_SET(t_data->tp[i]) ||
            (t_data->tm[i] < 0)) {
            gport_list[i] = t_data->tp[i];
        } else {
            gp_dest.port = t_data->tp[i];
            gp_dest.modid = t_data->tm[i];
            BCM_IF_ERROR_RETURN
                (_bcm_esw_gport_construct(unit, &gp_dest, &(gport_list[i])));
        }
    }

    return _bcm_esw_trunk_gport_array_resolve(unit,
                                              TRUNK_FABRIC_TID(unit, tid),
                                              t_data->num_ports, gport_list,
                                              t_data->tp, t_data->tm);
}

/*
 * Function:
 *	_bcm_esw_trunk_port_property_get
 * Purpose:
 *	Determines whether a system port is part of a trunk
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      modid  - Module id.
 *      port   - Port number.
 *      tid - (OUT) Trunk ID if trunk member.
 * Notes:
 *      Currently used only by RX module, gport support not required.
 */
int
_bcm_esw_trunk_port_property_get(int unit,
                                 bcm_module_t modid,
                                 bcm_port_t port,
                                 int *tid)
{
#if defined(BCM_TRIDENT_SUPPORT)
    if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
        return _bcm_trident_trunk_get_port_property(unit, modid, port, tid);
    }
#endif
#if defined(BCM_FIREBOLT_SUPPORT)
    if (SOC_IS_XGS_SWITCH(unit)) {
        return _bcm_xgs3_trunk_get_port_property(unit, modid, port, tid);
    }
#endif
    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *	bcm_trunk_deinit
 * Purpose:
 *	Release allocated trunk data structures.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_XXX
 */
void
_bcm_esw_trunk_deinit(int unit)
{
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (SOC_IS_XGS3_SWITCH(unit)) {
#if defined(BCM_TRIDENT_SUPPORT)
        if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
            _bcm_trident_trunk_deinit(unit);
        } else
#endif /* BCM_TRIDENT_SUPPORT */
        {
            _bcm_xgs3_trunk_member_detach(unit);
            _bcm_xgs3_trunk_swfailover_detach(unit);
            _bcm_xgs3_trunk_mod_port_map_deinit(unit);
#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
            _bcm_xgs3_trunk_hwfailover_detach(unit);
#endif /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */
        }

#ifdef BCM_WARM_BOOT_SUPPORT
        if (NULL != TRUNK_CNTL(unit).trunk_bmp_ptr) {
            sal_free(TRUNK_CNTL(unit).trunk_bmp_ptr);
            TRUNK_CNTL(unit).trunk_bmp_ptr = NULL;
        }
        if (NULL != TRUNK_CNTL(unit).hw_failover_flags) {
            sal_free(TRUNK_CNTL(unit).hw_failover_flags);
            TRUNK_CNTL(unit).hw_failover_flags = NULL;
        }
#endif /* BCM_WARM_BOOT_SUPPORT */
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */
}

#ifdef BCM_WARM_BOOT_SUPPORT

#define BCM_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define BCM_WB_VERSION_1_1                SOC_SCACHE_VERSION(1,1)
#define BCM_WB_VERSION_1_2                SOC_SCACHE_VERSION(1,2)
#define BCM_WB_DEFAULT_VERSION            BCM_WB_VERSION_1_2

#endif /* BCM_WARM_BOOT_SUPPORT */

/*
 * Function:
 *	bcm_trunk_init
 * Purpose:
 *	Initializes the trunk tables to empty (no trunks configured)
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_XXX
 */

int
bcm_esw_trunk_init(int unit)
{
    trunk_private_t	*t_info;
    bcm_trunk_t		tid;

    TRUNK_CNTL(unit).ngroups_fp = 0;
    TRUNK_CNTL(unit).nports_fp = 0;
    TRUNK_CNTL(unit).ngroups_hg = 0;
    TRUNK_CNTL(unit).nports_hg = 0;

    /* Create protection mutex. */
    if (NULL == TRUNK_CNTL(unit).lock) {
        TRUNK_CNTL(unit).lock = sal_mutex_create("Trunk module mutex");
    }

#ifdef BCM_XGS_SWITCH_SUPPORT
    if (SOC_IS_XGS_SWITCH(unit)) {
#ifdef BCM_WARM_BOOT_SUPPORT
        int                 ngroups_fp_max;
        soc_scache_handle_t scache_handle;
        uint32              trunk_scache_size;
        uint8               *trunk_scache;
#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
        int                 trunk_hwf_flags_alloc_size;
#endif /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */
#endif /* BCM_WARM_BOOT_SUPPORT */

        if (SOC_IS_SHADOW(unit)) {
            return BCM_E_UNAVAIL;
        }
        TRUNK_CNTL(unit).ngroups_fp =
            soc_mem_index_count(unit, TRUNK_GROUPm);
#ifdef BCM_WARM_BOOT_SUPPORT
        /* Record maximum HW size */
        ngroups_fp_max = TRUNK_CNTL(unit).ngroups_fp;
#endif /* BCM_WARM_BOOT_SUPPORT */

        if (SOC_IS_TD_TT(unit)) {
            TRUNK_CNTL(unit).nports_fp = 256;
        } else {
            TRUNK_CNTL(unit).nports_fp = 8;
        }

	if (!SOC_WARM_BOOT(unit)) {
	    for (tid = 0; tid < TRUNK_CNTL(unit).ngroups_fp; tid++) {
		if (soc_mem_write(unit, TRUNK_GROUPm, MEM_BLOCK_ALL, tid,
				  soc_mem_entry_null(unit,
						     TRUNK_GROUPm)) < 0) {
		    return BCM_E_INTERNAL;
		}

		if (soc_mem_write(unit, TRUNK_BITMAPm, MEM_BLOCK_ALL, tid,
				  soc_mem_entry_null(unit,
						     TRUNK_BITMAPm)) < 0) {
		    return BCM_E_INTERNAL;
		}

                if (soc_feature(unit, soc_feature_trunk_egress)) {
                    if (soc_mem_write(unit, TRUNK_EGR_MASKm, MEM_BLOCK_ALL, tid,
                        soc_mem_entry_null(unit,
                                           TRUNK_EGR_MASKm)) < 0) {
                        return BCM_E_INTERNAL;
                    }
                }
	    }
	}
#if defined(BCM_XGS3_SWITCH_SUPPORT)
        if (SOC_IS_XGS3_SWITCH(unit)) {
            int rv;

            uint64 config64, oconfig64;
#if defined(BCM_EASYRIDER_SUPPORT)
            uint32 config, oconfig;
#endif /* BCM_EASYRIDER_SUPPORT */

            soc_reg_t reg;
            uint32 addr;

#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
            soc_reg_t hg_trunk_bmap;
            uint32 hg_trunk_bmap_addr;
#endif


            if (SOC_WARM_BOOT(unit)) {

                if (SOC_IS_FBX(unit)) {

#if defined(BCM_TRX_SUPPORT)
                    reg = SOC_IS_TRX(unit) ? ING_CONFIG_64r : ING_CONFIGr;
#else /* BCM_TRX_SUPPORT */
                    reg = ING_CONFIGr;
#endif /* BCM_TRX_SUPPORT */
                    addr = soc_reg_addr(unit, reg, REG_PORT_ANY, 0);
                    SOC_IF_ERROR_RETURN(soc_reg_get(unit, reg, REG_PORT_ANY, 0,
                                &config64));

#if defined(BCM_HAWKEYE_SUPPORT)
                    if (SOC_IS_HAWKEYE(unit)) {
		        TRUNK_CNTL(unit).ngroups_fp = 8;
                    } else
#endif                
                    if (!soc_reg64_field32_get(unit, reg, config64,
                                TRUNKS128f)) {
                        TRUNK_CNTL(unit).ngroups_fp = 32;
                    }

                    if (SOC_IS_XGS3_FABRIC(unit)) {
                        TRUNK_CNTL(unit).ngroups_fp = 0;
                        TRUNK_CNTL(unit).nports_fp = 0;
                    }

#if defined(BCM_HAWKEYE_SUPPORT)
                    if (SOC_IS_HAWKEYE(unit)) {
                        TRUNK_CNTL(unit).ngroups_hg = 0;
                        TRUNK_CNTL(unit).nports_hg = 0;
                    } else
#endif
                    if (SOC_IS_FB_FX_HX(unit) || SOC_IS_RAVEN(unit)) {
                        TRUNK_CNTL(unit).ngroups_hg = 2;
                        TRUNK_CNTL(unit).nports_hg = 4;
                    }

#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
                    else if (SOC_IS_HB_GW(unit) || SOC_IS_TRX(unit)) {
                        if (soc_mem_is_valid(unit, HG_TRUNK_BITMAPm)) {
                            TRUNK_CNTL(unit).ngroups_hg =
                                soc_mem_index_count(unit, HG_TRUNK_BITMAPm);
                        } else {
#if defined(BCM_TRIUMPH2_SUPPORT)
                            if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                                SOC_IS_VALKYRIE2(unit)) {
                                hg_trunk_bmap = HG_TRUNK_BITMAP_64r;
                            } else 
#endif /* BCM_TRIUMPH2_SUPPORT */
                            {
                                hg_trunk_bmap = HG_TRUNK_BITMAPr;
                            }

                            TRUNK_CNTL(unit).ngroups_hg =
                                SOC_REG_NUMELS(unit, hg_trunk_bmap);
                        }

#if defined(BCM_TRIDENT_SUPPORT)
                        if (SOC_IS_TD_TT(unit)) {
                            TRUNK_CNTL(unit).nports_hg = 32;
                        } else
#endif /* BCM_TRIDENT_SUPPORT */
#if defined(BCM_KATANA_SUPPORT)
                        if (SOC_IS_KATANA(unit)) {
                            TRUNK_CNTL(unit).nports_hg = 4;
                        } else
#endif /* BCM_KATANA_SUPPORT */
#if defined(BCM_SCORPION_SUPPORT)
                        if (SOC_IS_SC_CQ(unit)) {
                            TRUNK_CNTL(unit).nports_hg = 16;
                        } else 
#endif /* BCM_SCORPION_SUPPORT */
                        {
                            TRUNK_CNTL(unit).nports_hg = 8;
                        }
                    }
#endif /* BCM_BRADLEY_SUPPORT || BCM_TRX_SUPPORT */
                }
#if defined(BCM_EASYRIDER_SUPPORT)
                else if (SOC_IS_EASYRIDER(unit)) {
                    SOC_IF_ERROR_RETURN(READ_SEER_CONFIGr(unit, &config));
                    if (!soc_reg_field_get(unit, SEER_CONFIGr,
                                           config, TRUNKS128f)) {
                        TRUNK_CNTL(unit).ngroups_fp = 32;
                    }
                }
#endif /* BCM_EASYRIDER_SUPPORT */

            } else {
                int trunk128;

                trunk128 = soc_property_get(unit, spn_TRUNK_EXTEND, 0);

                /* For Raptor and Trident TRUNKS128 field should alwyas be programmed to 1 */
                if (SOC_IS_RAPTOR(unit) || SOC_IS_RAVEN(unit) || 
                    SOC_IS_TD_TT(unit) || SOC_IS_KATANA (unit)) {
                    trunk128 = 1;
                }
 
                if (!trunk128) {
                    if (SOC_IS_HAWKEYE(unit)){
                        TRUNK_CNTL(unit).ngroups_fp = 8;
                    } else {
                        TRUNK_CNTL(unit).ngroups_fp = 32;
                    }
                }

		if (SOC_IS_XGS3_FABRIC(unit)) {
		    TRUNK_CNTL(unit).ngroups_fp = 0;
		    TRUNK_CNTL(unit).nports_fp = 0;
		}

                SOC_IF_ERROR_RETURN(soc_mem_clear(unit, SOURCE_TRUNK_MAP_TABLEm,
                                                  MEM_BLOCK_ALL, 0));

                if (SOC_IS_FBX(unit) || SOC_IS_RAVEN(unit)) { 

#if defined(BCM_TRX_SUPPORT)
                    reg = SOC_IS_TRX(unit) ? ING_CONFIG_64r : ING_CONFIGr;
#else /* BCM_TRX_SUPPORT */
                    reg = ING_CONFIGr;
#endif /* BCM_TRX_SUPPORT */
                    addr = soc_reg_addr(unit, reg, REG_PORT_ANY, 0);
                    SOC_IF_ERROR_RETURN(soc_reg_get(unit, reg, REG_PORT_ANY, 0,
                                &config64));
                    oconfig64 = config64;
                    soc_reg64_field32_set(unit, reg, &config64, TRUNKS128f,
                                          trunk128 ? 1 : 0);
                    if (COMPILER_64_NE(config64, oconfig64)) {
                        SOC_IF_ERROR_RETURN(soc_reg_set(unit, reg, REG_PORT_ANY, 0,
                                    config64));
                    }

                    SOC_IF_ERROR_RETURN
                        (soc_mem_clear(unit, NONUCAST_TRUNK_BLOCK_MASKm,
                                       MEM_BLOCK_ALL, 0));

                    if (soc_mem_is_valid(unit, TRUNK_MEMBERm)) {
                        SOC_IF_ERROR_RETURN
                            (soc_mem_clear(unit, TRUNK_MEMBERm, MEM_BLOCK_ALL, 0));
                    }

                    if (soc_feature(unit, soc_feature_hg_trunking)) {
                        if (soc_mem_is_valid(unit, HIGIG_TRUNK_CONTROLm)) {
                            higig_trunk_control_entry_t entry;
                            soc_mem_pbmp_field_set(unit, HIGIG_TRUNK_CONTROLm,
                                    &entry, ACTIVE_PORT_BITMAPf,
                                    &PBMP_ALL(unit));
                            SOC_IF_ERROR_RETURN
                                (soc_mem_write(unit, HIGIG_TRUNK_CONTROLm,
                                               MEM_BLOCK_ALL, 0, &entry));
                        } else {
                            uint64 val, hg_cntl_entry;
                            soc_reg_t hg_trunk_reg;
                            uint32 hg_trunk_reg_addr;

#if defined(BCM_TRIUMPH2_SUPPORT)
                            if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                                    SOC_IS_VALKYRIE2(unit)) {
                                hg_trunk_reg = HIGIG_TRUNK_CONTROL_64r;
                            } else  
#endif /* BCM_TRIUMPH2_SUPPORT */
                            {
                                hg_trunk_reg = HIGIG_TRUNK_CONTROLr;
                            }

                            hg_trunk_reg_addr =
                                soc_reg_addr(unit, hg_trunk_reg, 
                                        REG_PORT_ANY, 0);   
                            SOC_IF_ERROR_RETURN
                                (soc_reg_get(unit, hg_trunk_reg, 
                                              REG_PORT_ANY, 0,
                                              &hg_cntl_entry));
                            val = soc_reg64_field_get(unit, hg_trunk_reg,
                                    hg_cntl_entry,
                                    ACTIVE_PORT_BITMAPf);
                            COMPILER_64_ZERO(hg_cntl_entry);
                            soc_reg64_field_set(unit, hg_trunk_reg,
                                    &hg_cntl_entry,
                                    ACTIVE_PORT_BITMAPf, val);
                            SOC_IF_ERROR_RETURN
                                (soc_reg_set(unit, hg_trunk_reg, 
                                               REG_PORT_ANY, 0,
                                               hg_cntl_entry));
                        }
                    }

                    if (SOC_IS_FB_FX_HX(unit) || SOC_IS_RAVEN(unit)) {
                        if (soc_feature(unit, soc_feature_hg_trunking)) {
                            TRUNK_CNTL(unit).ngroups_hg = 2;
                            TRUNK_CNTL(unit).nports_hg = 4;
                            SOC_IF_ERROR_RETURN(WRITE_HIGIG_TRUNK_GROUPr(unit, 0));
                        }
                    }
#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
                    else if (SOC_IS_HB_GW(unit) || SOC_IS_TRX(unit)) {
                        uint64      val64;

#if defined(BCM_TRIDENT_SUPPORT)
                        if (SOC_IS_TD_TT(unit)) {
                            TRUNK_CNTL(unit).ngroups_hg =
                                soc_mem_index_count(unit, HG_TRUNK_BITMAPm);
                            TRUNK_CNTL(unit).nports_hg = 32;
                        }
                        if (SOC_IS_KATANA(unit)) {
                            TRUNK_CNTL(unit).ngroups_hg = 2;
                            TRUNK_CNTL(unit).nports_hg = 4;
                        }
                        if (SOC_IS_TD_TT(unit)||SOC_IS_KATANA(unit)) {                                                        
                            SOC_IF_ERROR_RETURN
                                (soc_mem_clear(unit, HG_TRUNK_BITMAPm, MEM_BLOCK_ALL, TRUE));
                            SOC_IF_ERROR_RETURN
                                (soc_mem_clear(unit, HG_TRUNK_GROUPm, MEM_BLOCK_ALL, TRUE));
                            SOC_IF_ERROR_RETURN
                                (soc_mem_clear(unit, HG_TRUNK_MEMBERm, MEM_BLOCK_ALL, TRUE));
                        } else
#endif /* BCM_TRIDENT_SUPPORT */
                        {

#if defined(BCM_TRIUMPH2_SUPPORT)
                            if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                                    SOC_IS_VALKYRIE2(unit)) {
                                hg_trunk_bmap = HG_TRUNK_BITMAP_64r;
                            } else 
#endif /* BCM_TRIUMPH2_SUPPORT */
                            {
                                hg_trunk_bmap = HG_TRUNK_BITMAPr;
                            }

                            TRUNK_CNTL(unit).ngroups_hg =
                                SOC_REG_NUMELS(unit, hg_trunk_bmap);

#if defined(BCM_SCORPION_SUPPORT)
                            if (SOC_IS_SC_CQ(unit)) {
                                TRUNK_CNTL(unit).nports_hg = 16;
                            } else 
#endif /* BCM_SCORPION_SUPPORT */
                            {
                                TRUNK_CNTL(unit).nports_hg = 8;
                            }

                            COMPILER_64_ZERO(val64);
                            for (tid = 0; tid < TRUNK_CNTL(unit).ngroups_hg; tid++) {
                                hg_trunk_bmap_addr = soc_reg_addr(unit, hg_trunk_bmap, 
                                        REG_PORT_ANY, tid);
                                SOC_IF_ERROR_RETURN
                                    (soc_reg_set(unit, hg_trunk_bmap, REG_PORT_ANY, 
                                                 tid, val64));
                                SOC_IF_ERROR_RETURN
                                    (WRITE_HG_TRUNK_GROUPr(unit, tid, val64));
                            }
                        }
                    }
#endif /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */
                }
#if defined(BCM_EASYRIDER_SUPPORT)
                else if (SOC_IS_EASYRIDER(unit)) {
                    int i, num_reg;
                    uint32 trk_blk;

                    num_reg = SOC_REG_NUMELS(unit, NONUCAST_TRUNK_BLOCK_MASKr);
                    trk_blk = 0;
                    for (i = 0; i < num_reg; i++) {
			SOC_IF_ERROR_RETURN
			    (WRITE_NONUCAST_TRUNK_BLOCK_MASKr(unit, i, trk_blk));
                    }

                    SOC_IF_ERROR_RETURN
                        (READ_SEER_CONFIGr(unit, &config));
                    oconfig = config;
                    soc_reg_field_set(unit, SEER_CONFIGr, &config,
                                      TRUNKS128f, (trunk128) ? 1 : 0);
                    if (config != oconfig) {
                        SOC_IF_ERROR_RETURN
                            (WRITE_SEER_CONFIGr(unit, config));
                    }
                }
#endif /* BCM_EASYRIDER_SUPPORT */
            }

#ifdef BCM_TRIDENT_SUPPORT
            if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
                BCM_IF_ERROR_RETURN(_bcm_trident_trunk_init(unit));
            } else 
#endif /* BCM_TRIDENT_SUPPORT */
            {
                rv = _bcm_xgs3_trunk_member_init(unit);
                if (BCM_FAILURE(rv)) {
                    _bcm_esw_trunk_deinit(unit);
                    return rv;
                }
                rv = _bcm_xgs3_trunk_swfailover_init(unit);
                if (BCM_FAILURE(rv)) {
                    _bcm_esw_trunk_deinit(unit);
                    return rv;
                }
                rv = _bcm_xgs3_trunk_mod_port_map_init(unit);
                if (BCM_FAILURE(rv)) {
                    _bcm_esw_trunk_deinit(unit);
                    return rv;
                }
#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
                if (soc_feature(unit, soc_feature_hg_trunk_failover) ||
                    soc_feature(unit, soc_feature_port_lag_failover)) {
                    rv = _bcm_xgs3_trunk_hwfailover_init(unit);
                    if (BCM_FAILURE(rv)) {
                        _bcm_esw_trunk_deinit(unit);
                        return rv;
                    }
                }
#endif /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */
            }

#ifdef BCM_WARM_BOOT_SUPPORT
            TRUNK_CNTL(unit).trunk_bmp_alloc =
                SHR_BITALLOCSIZE(ngroups_fp_max +
                                 TRUNK_CNTL(unit).ngroups_hg);

            /* Num FP trunks, num fabric trunks, bitmap of defined trunks */
            trunk_scache_size = sizeof(uint32) + sizeof(uint32) +
                TRUNK_CNTL(unit).trunk_bmp_alloc;

            TRUNK_CNTL(unit).trunk_bmp_ptr =
                sal_alloc( TRUNK_CNTL(unit).trunk_bmp_alloc, "trunk used bitmap");
            if (NULL == TRUNK_CNTL(unit).trunk_bmp_ptr) {
                _bcm_esw_trunk_deinit(unit);
                return BCM_E_MEMORY;
            }

#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
            trunk_hwf_flags_alloc_size = 0;
            if (soc_feature(unit, soc_feature_port_lag_failover)) {
                /* FP trunk failover flags */
#if defined(BCM_TRIDENT_SUPPORT)
                if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
                    /* For Trident, ngproups_fp_max = 1024, nports_fp = 256,
                     * while the maximum number of front-panel trunk ports is 
                     * limited by the number of entries in
                     * SOURCE_TRUNK_MAP_TABLE, which is 4096. To efficiently 
                     * utilize scache space, for each trunk group, 1 byte of
                     * BCM_TRUNK_FLAG_xxx flags, 2 bytes of trunk size, and
                     * trunk-size bytes of hardware failover flags are stored.
                     */
                    trunk_hwf_flags_alloc_size +=
                        (ngroups_fp_max * 3 +
                         soc_mem_index_count(unit, SOURCE_TRUNK_MAP_TABLEm)) * 
                        sizeof(uint8);
                } else
#endif /* BCM_TRIDENT_SUPPORT */
                {
                    trunk_hwf_flags_alloc_size +=
                        (ngroups_fp_max * (TRUNK_CNTL(unit).nports_fp + 1)) *
                        sizeof(uint8);
                }
            }

            if (soc_feature(unit, soc_feature_hg_trunk_failover)) {
                /* HG trunk failover flags */
                trunk_hwf_flags_alloc_size +=
                    (TRUNK_CNTL(unit).ngroups_hg *
                     (TRUNK_CNTL(unit).nports_hg + 1)) *
                    sizeof(uint8);
            }

            if (0 != trunk_hwf_flags_alloc_size) {
                TRUNK_CNTL(unit).hw_failover_flags =
                    sal_alloc(trunk_hwf_flags_alloc_size, "trunk hwf flags");
                if (NULL == TRUNK_CNTL(unit).hw_failover_flags) {
                    _bcm_esw_trunk_deinit(unit);
                    return BCM_E_MEMORY;
                }
                TRUNK_CNTL(unit).hw_failover_alloc =
                    trunk_hwf_flags_alloc_size;
                trunk_scache_size += trunk_hwf_flags_alloc_size;
            }
#endif /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */

            /* Allocate scache space for BCM_TRUNK_FLAG_IPMC_CLEAVE flag */
            trunk_scache_size += (ngroups_fp_max * sizeof(uint8));

            /* Allocate scache space for ipmc_psc */
            trunk_scache_size += (ngroups_fp_max * sizeof(int));

            /* Allocate scache space to store trunk membership */
#if defined(BCM_TRIDENT_SUPPORT)
            if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
                /* For each front-panel trunk group, store the following info:
                 * - 2 bytes containing the number of members, which can be
                 *   up to 256.
                 * - 2 bytes per member, containing the module and port IDs.
                 * - 1 byte per member, containing member flags.
                 *
                 * Trident can support up to 1k trunk groups. It's not 
                 * necessary to allocate space for 256 members per trunk group,
                 * since the maximum number of members Trident can support is
                 * limited by the TRUNK_MEMBER table.
                 */
                trunk_scache_size += (ngroups_fp_max * 2 +
                        soc_mem_index_count(unit, TRUNK_MEMBERm) * 3) *
                        sizeof(uint8);
            } else
#endif /* BCM_TRIDENT_SUPPORT */
            {
                /* For each front-panel trunk group, store the following info:
                 * - 1 byte containing the number of members, which can be
                 *   up to BCM_TRUNK_MAX_PORTCNT.
                 * - 2 bytes per member, containing the module and port IDs.
                 * - 1 byte per member, containing member flags.
                 */
                trunk_scache_size += 
                    (ngroups_fp_max * (1 + 3 * TRUNK_CNTL(unit).nports_fp))
                    * sizeof(uint8);
            }

            /* Limited scache mode unsupported */
            if (!SOC_WARM_BOOT_SCACHE_IS_LIMITED(unit)) {
                SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_TRUNK, 0);
                rv = _bcm_esw_scache_ptr_get(unit, scache_handle,
                        (0 == SOC_WARM_BOOT(unit)),
                        trunk_scache_size,
                        &trunk_scache, 
                        BCM_WB_DEFAULT_VERSION, NULL);
                if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
                    _bcm_esw_trunk_deinit(unit);
                    return rv;
                }
            }
#endif /* BCM_WARM_BOOT_SUPPORT */
        }
#endif /* BCM_XGS3_SWITCH_SUPPORT */

	if (SOC_IS_DRACO15(unit)) {
	    bcm_port_t		port;
	    egr_mask_entry_t 	egr_mask_entry;
	    int 		index, index_min, index_max;
	    uint32         	configreg = 0;

	    /*
	     * Set number of trunk groups based on TRUNKS128 mode.
	     */
	    if (SOC_WARM_BOOT(unit)) {
		PBMP_PORT_ITER(unit, port) {
		    SOC_IF_ERROR_RETURN(READ_CONFIGr(unit, port, &configreg));
		    break;
		}

		if (!soc_reg_field_get(unit, CONFIGr,
				       configreg, TRUNKS128f)) {
		    TRUNK_CNTL(unit).ngroups_fp = 32;
		}
	    } else {
		if (soc_property_get(unit, spn_TRUNK_EXTEND, 0)) {
		    PBMP_PORT_ITER(unit, port) {
			SOC_IF_ERROR_RETURN
			    (READ_CONFIGr(unit, port, &configreg));
			soc_reg_field_set(unit, CONFIGr,
					  &configreg, TRUNKS128f, 1);
			SOC_IF_ERROR_RETURN
			    (WRITE_CONFIGr(unit, port, configreg));
		    }
		} else {
		    TRUNK_CNTL(unit).ngroups_fp = 32;
		}

                index_min = soc_mem_index_min(unit, EGR_MASKm);
                index_max = soc_mem_index_max(unit, EGR_MASKm);
		/*
		 * Initialize TGIDs in EGR_MASK table
		 */
		for (index = index_max; index >= index_min; index--) {
		    SOC_IF_ERROR_RETURN
			(soc_mem_read(unit, EGR_MASKm, MEM_BLOCK_ANY,
				      index, &egr_mask_entry));

		    if (soc_EGR_MASKm_field32_get(unit,
						  &egr_mask_entry, Tf)) {
			/*
			 * This modid/port entry was a trunk port, so
			 * reinit EGRESS_MASKf field for non-trunk use.
			 */
			soc_EGR_MASKm_field32_set(unit, &egr_mask_entry,
						  EGRESS_MASKf, 0);
		    }

		    soc_EGR_MASKm_field32_set(unit, &egr_mask_entry, Tf, 0);
		    soc_EGR_MASKm_field32_set(unit, &egr_mask_entry, TGIDf, 0);

		    SOC_IF_ERROR_RETURN
			(soc_mem_write(unit, EGR_MASKm, MEM_BLOCK_ALL,
				       index, &egr_mask_entry));
		}

		if (soc_feature(unit, soc_feature_ipmc_lookup)) {
		    mmu_memories2_egr_trunk_map_entry_t etm_entry;

		    /*
		     * Initialize SRC_TRUNK_MAPm and EGR_TRUNK_MAPm
		     */

		    SOC_IF_ERROR_RETURN
			(soc_mem_clear(unit,
				       MMU_MEMORIES2_SRC_TRUNK_MAPm,
				       MEM_BLOCK_ALL, 0));

		    PBMP_E_ITER(unit, port) {
			SOC_IF_ERROR_RETURN
			    (READ_MMU_MEMORIES2_EGR_TRUNK_MAPm(unit,
							       MEM_BLOCK_ALL,
							       port,
							       &etm_entry));
			soc_MMU_MEMORIES2_EGR_TRUNK_MAPm_field32_set
			    (unit, &etm_entry, TGIDf, 0);
			soc_MMU_MEMORIES2_EGR_TRUNK_MAPm_field32_set
			    (unit, &etm_entry, TRUNKf, 0);

			SOC_IF_ERROR_RETURN
			    (WRITE_MMU_MEMORIES2_EGR_TRUNK_MAPm(unit,
								MEM_BLOCK_ALL,
								port,
								&etm_entry));
		    }
		}
	    }
	}
    }
#endif	/* BCM_XGS_SWITCH_SUPPORT */

#ifdef	BCM_XGS12_FABRIC_SUPPORT
    if (SOC_IS_XGS12_FABRIC(unit)) {
	bcm_port_t	port;

        TRUNK_CNTL(unit).ngroups_hg = SOC_REG_NUMELS(unit, ING_HGTRUNKr);
        TRUNK_CNTL(unit).nports_hg = (SOC_IS_HERCULES1(unit)) ?
	    BCM_XGS1_FABRIC_TRUNK_MAX_PORTCNT :
	    BCM_XGS23_FABRIC_TRUNK_MAX_PORTCNT;

	if (!SOC_WARM_BOOT(unit)) {
	    for (tid = 0; tid < TRUNK_CNTL(unit).ngroups_hg; tid++) {
		PBMP_PORT_ITER(unit, port) {
		    SOC_IF_ERROR_RETURN
			(WRITE_ING_HGTRUNKr(unit, port, tid, 0));
		}
	    }
	}
    }
#endif	/* BCM_XGS12_FABRIC_SUPPORT */

    if (TRUNK_CNTL(unit).t_info != NULL) {
        sal_free(TRUNK_CNTL(unit).t_info);
        TRUNK_CNTL(unit).t_info = NULL;

    }

    if (TRUNK_NUM_GROUPS(unit) > 0) {
        TRUNK_CNTL(unit).t_info = 
            sal_alloc(TRUNK_NUM_GROUPS(unit) * sizeof(trunk_private_t),
                      "trunk_priv");
        if (NULL == TRUNK_CNTL(unit).t_info) {
            _bcm_esw_trunk_deinit(unit);
            return (BCM_E_MEMORY);
        }

	t_info = TRUNK_CNTL(unit).t_info;

        /* Clear the state for Cold or Warm Boot.
         * Warm Boot will update below. */
        for (tid = 0; tid < TRUNK_NUM_GROUPS(unit); tid++) {
            t_info->tid = BCM_TRUNK_INVALID;
            t_info->in_use = FALSE;
            t_info->psc = BCM_TRUNK_PSC_DEFAULT;
            t_info->ipmc_psc = 0;
            t_info->rtag = 0;
            t_info->flags = 0;
            t_info->dlf_index_spec = -1;
            t_info->dlf_index_used = -1;
            t_info->mc_index_spec = -1;
            t_info->mc_index_used = -1;
            t_info->ipmc_index_spec = -1;
            t_info->ipmc_index_used = -1;
            t_info->dynamic_size = 0;
            t_info->dynamic_age = 0;
            t_info++;
        }
    }

#if defined(BCM_XGS_FABRIC_SUPPORT) || defined(BCM_FIREBOLT_SUPPORT)
    if (SOC_WARM_BOOT(unit)) {
        return(_bcm_trunk_reinit(unit));
    }
#endif

    return BCM_E_NONE;
}


/*
 * Function:
 *	bcm_trunk_detach
 * Purpose:
 *	Cleans up the trunk tables.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_XXX
 */

int
bcm_esw_trunk_detach(int unit)
{
    /* Return if trunk was not initialized. */
    if (NULL == TRUNK_CNTL(unit).t_info) {
        return (BCM_E_NONE);
    }

    sal_free(TRUNK_CNTL(unit).t_info);
    /* Destroy protection mutex. */
    if (NULL != TRUNK_CNTL(unit).lock) {
        sal_mutex_destroy(TRUNK_CNTL(unit).lock);
        TRUNK_CNTL(unit).lock = NULL;
    }
    sal_memset(&TRUNK_CNTL(unit), 0, sizeof(trunk_cntl_t));

    _bcm_esw_trunk_deinit(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_trunk_create
 * Purpose:
 *      Create the software data structure for a trunk ID.
 *      This function does not update any hardware tables,
 *      must call bcm_trunk_set() to finish trunk setting.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      tid - (Out), The trunk ID.
 * Returns:
 *      BCM_E_NONE     - Success.
 *      BCM_E_FULL - run out of TIDs
 * Note:
 *      For devices that support both front panel and fabric
 *      trunking, this routine creates a front panel TID only.
 *      To create a fabric trunk on such devices,
 *      use bcm_trunk_create_id().
 */

int
bcm_esw_trunk_create(int unit, bcm_trunk_t *tid)
{
    trunk_private_t *t_info;
    int rv, i;
    bcm_trunk_t max_tid;

    TRUNK_INIT(unit);

    TRUNK_LOCK(unit);

    rv = BCM_E_FULL;

    t_info = TRUNK_CNTL(unit).t_info;

    if (TRUNK_CNTL(unit).ngroups_fp == 0) {
        max_tid = TRUNK_CNTL(unit).ngroups_hg - 1;
    }
    else {
        max_tid = TRUNK_CNTL(unit).ngroups_fp - 1;
    }

    for (i = 0; i <= max_tid; i++) {
        if (t_info->tid == BCM_TRUNK_INVALID) {
            t_info->tid = i;
            t_info->in_use = FALSE;
            t_info->psc = BCM_TRUNK_PSC_DEFAULT;
            *tid = i;
            rv = BCM_E_NONE;
            break;
        }
        t_info++;
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    SOC_CONTROL_LOCK(unit);
    SOC_CONTROL(unit)->scache_dirty = 1;
    SOC_CONTROL_UNLOCK(unit);
#endif

    TRUNK_UNLOCK(unit);

    return rv;
}


/*
 * Function:
 *	_bcm_trunk_id_validate
 * Purpose:
 *  Service routine to validate validity of trunk id.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      tid - The trunk ID.
 * Returns:
 *      BCM_E_NONE    - Success.
 *      BCM_E_INIT    - trunking software not initialized
 *      BCM_E_BADID   - TID out of range
 */
int
_bcm_trunk_id_validate(int unit, bcm_trunk_t tid)
{
    TRUNK_CHECK(unit, tid);
    return (BCM_E_NONE);
}

/*
 * Function:
 *	bcm_trunk_create_id
 * Purpose:
 *      Create the software data structure for the specified trunk ID.
 *      This function does not update any hardware tables,
 *      must call bcm_trunk_set() to finish trunk setting.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      tid - The trunk ID.
 * Returns:
 *      BCM_E_NONE       - Success.
 *      BCM_E_INIT       - trunking software not initialized
 *      BCM_E_EXISTS - TID already used
 *      BCM_E_BADID   - TID out of range
 */

int
bcm_esw_trunk_create_id(int unit, bcm_trunk_t tid)
{
    trunk_private_t *t_info;
    int rv;

    TRUNK_INIT(unit);
    TRUNK_CHECK(unit, tid);

    TRUNK_LOCK(unit);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
        t_info->tid = tid;
        t_info->psc = BCM_TRUNK_PSC_DEFAULT;
        t_info->in_use = FALSE;
        rv = BCM_E_NONE;
    } else {
	rv = BCM_E_EXISTS;
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    SOC_CONTROL_LOCK(unit);
    SOC_CONTROL(unit)->scache_dirty = 1;
    SOC_CONTROL_UNLOCK(unit);
#endif

    TRUNK_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *	bcm_trunk_psc_set
 * Purpose:
 *      Set the trunk selection criteria.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      tid - The trunk ID to be affected.
 *      psc - Identify the trunk selection criteria.
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_XXX
 */

int
bcm_esw_trunk_psc_set(int unit, bcm_trunk_t tid, int psc)
{
    trunk_private_t		*t_info;
    bcm_trunk_add_info_t	add_info;

    TRUNK_INIT(unit);
    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
        return BCM_E_NOT_FOUND;
    }

    if (psc <= 0) {
	psc = BCM_TRUNK_PSC_DEFAULT;
    }

    if (t_info->psc == psc) {
        return BCM_E_NONE;
    }

    if (!t_info->in_use) {
	t_info->psc = psc;
	return BCM_E_NONE;
    }

    BCM_IF_ERROR_RETURN(bcm_esw_trunk_get(unit, tid, &add_info));
    add_info.psc = psc;
    add_info.flags = 0; /* Don't change failover config */
    BCM_IF_ERROR_RETURN(bcm_esw_trunk_set(unit, tid, &add_info));
    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_trunk_psc_get
 * Purpose:
 *      Get the trunk selection criteria.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      tid - The trunk ID to be used.
 *      psc - (OUT) Identify the trunk selection criteria.
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_XXX
 */

int
bcm_esw_trunk_psc_get(int unit, bcm_trunk_t tid, int *psc)
{
    trunk_private_t *t_info;
    int rv;

    TRUNK_INIT(unit);
    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
	*psc = 0;
        rv = BCM_E_NOT_FOUND;
    } else {
	*psc = t_info->psc;
	rv = BCM_E_NONE;
    }

    return rv;
}

/*
 * Function:
 *	bcm_trunk_chip_info_get
 * Purpose:
 *      Get the trunk information.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      t_info - (OUT) Identify the trunk selection criteria.
 * Returns:
 *      BCM_E_NONE              Success.
 * Notes:
 *      None.
 */

int
bcm_esw_trunk_chip_info_get(int unit, bcm_trunk_chip_info_t *ta_info)
{
    TRUNK_INIT(unit);

    ta_info->trunk_group_count = TRUNK_CNTL(unit).ngroups_fp;

    if (TRUNK_CNTL(unit).ngroups_fp > 0) {
        ta_info->trunk_id_min = 0;
	ta_info->trunk_id_max = TRUNK_CNTL(unit).ngroups_fp - 1;
    }
    else {
        ta_info->trunk_id_min = -1;
	ta_info->trunk_id_max = -1;
    }
    ta_info->trunk_ports_max = TRUNK_CNTL(unit).nports_fp;

    if (TRUNK_CNTL(unit).ngroups_hg > 0) {
        ta_info->trunk_fabric_id_min = TRUNK_CNTL(unit).ngroups_fp;
	ta_info->trunk_fabric_id_max =
	  TRUNK_CNTL(unit).ngroups_fp + TRUNK_CNTL(unit).ngroups_hg - 1;
    }
    else {
        ta_info->trunk_fabric_id_min = -1;
	ta_info->trunk_fabric_id_max = -1;
    }
    ta_info->trunk_fabric_ports_max = TRUNK_CNTL(unit).nports_hg;

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_trunk_set
 * Purpose:
 *      Adds ports to a trunk group.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      tid - The trunk ID to be affected.
 *      t_add_info - Information on the trunk group.
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_XXX
 * Notes:
 *      The existing ports in the trunk group will be replaced with new ones.
 */

int
bcm_esw_trunk_set(int unit, bcm_trunk_t tid, bcm_trunk_add_info_t *add_info)
{
    trunk_private_t         *t_info;
    bcm_trunk_add_info_t    t_add;
    int rv;

    TRUNK_INIT(unit);
    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
        return (BCM_E_NOT_FOUND);
    }

    if (add_info->num_ports < 1) {
        if (!(SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit))) {
            /* For Trident, add_info->num_ports is allowed to be 0. 
             * Trident trunk configuration can be done in two steps.
             * First, call bcm_trunk_set with num_ports = 0.
             * Then, call bcm_trunk_member_set or bcm_trunk_member_add.
             */
            return (BCM_E_PARAM);
        }
    } else {
        /* Check number of ports in trunk group */
        if ((TRUNK_FP_TID(unit, tid) &&
             (add_info->num_ports > TRUNK_CNTL(unit).nports_fp)) ||
            (TRUNK_FABRIC_TID(unit, tid) &&
             (add_info->num_ports > TRUNK_CNTL(unit).nports_hg))) {
            return (BCM_E_PARAM);
        }
    }

    bcm_trunk_add_info_t_init(&t_add);
    sal_memcpy(&t_add, add_info, sizeof(bcm_trunk_add_info_t));

    BCM_IF_ERROR_RETURN(
        _bcm_esw_trunk_gport_resolve(unit, tid, &t_add));

    TRUNK_LOCK(unit);
    rv = mbcm_driver[unit]->mbcm_trunk_set(unit, tid, &t_add, t_info);
    if (rv >= 0) {
        t_info->flags = add_info->flags;
        t_info->psc = add_info->psc;
        if (add_info->flags & BCM_TRUNK_FLAG_IPMC_CLEAVE) {
            t_info->ipmc_psc = add_info->ipmc_psc;
        }
        if (add_info->num_ports > 0) {
            t_info->in_use = TRUE;
        }
    }

    TRUNK_UNLOCK(unit);

#ifdef BCM_XGS12_SWITCH_SUPPORT
#ifdef INCLUDE_L3
    /* reconcile L3 programming with VLAN tables' state */
    if (SOC_IS_XGS12_SWITCH(unit) && soc_feature(unit, soc_feature_l3)) {
        _bcm_xgs_l3_untagged_update(unit, BCM_VLAN_INVALID, tid, -1);
    }
#endif  /* INCLUDE_L3 */
#endif  /* BCM_XGS12_SWITCH_SUPPORT */

#if defined(BCM_XGS3_SWITCH_SUPPORT)
    /* reconcile Mirror to Port programming with trunk member ports. */
    if (SOC_IS_XGS3_SWITCH(unit) && BCM_SUCCESS(rv)) {
        rv = _bcm_xgs3_mirror_trunk_update(unit, tid);
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */

    return (rv);
}

/*
 * Function:
 *	bcm_trunk_destroy
 * Purpose:
 *	Removes a trunk group.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      tid - Trunk Id.
 * Returns:
 *      BCM_E_NONE     Success.
 *      BCM_E_XXX
 */

int
bcm_esw_trunk_destroy(int unit, bcm_trunk_t tid)
{
    trunk_private_t *t_info;
    int rv;

    TRUNK_INIT(unit);
    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
        return (BCM_E_NOT_FOUND);
    }

    if (!t_info->in_use) {
        t_info->tid = BCM_TRUNK_INVALID;
        return (BCM_E_NONE);
    }

    TRUNK_LOCK(unit);
    rv = mbcm_driver[unit]->mbcm_trunk_destroy(unit, tid, t_info);
    t_info->tid = BCM_TRUNK_INVALID;
    t_info->in_use = FALSE;
    t_info->psc = BCM_TRUNK_PSC_DEFAULT;
    t_info->ipmc_psc = 0;
    t_info->flags = 0;
    t_info->dlf_index_spec = -1;
    t_info->dlf_index_used = -1;
    t_info->dlf_port_used = 0;
    t_info->mc_index_spec = -1;
    t_info->mc_index_used = -1;
    t_info->mc_port_used = 0;
    t_info->ipmc_index_spec = -1;
    t_info->ipmc_index_used = -1;
    t_info->ipmc_port_used = 0;

#ifdef BCM_WARM_BOOT_SUPPORT
    SOC_CONTROL_LOCK(unit);
    SOC_CONTROL(unit)->scache_dirty = 1;
    SOC_CONTROL_UNLOCK(unit);
#endif

    TRUNK_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *	bcm_trunk_get
 * Purpose:
 *      Return a port information of given trunk ID.
 * Parameters:
 *      unit   - StrataSwitch PCI device unit number (driver internal).
 *      tid    - Trunk ID.
 *      t_data - (Out), Place to store returned trunk info.
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_XXX
 */

int
bcm_esw_trunk_get(int unit, bcm_trunk_t tid, bcm_trunk_add_info_t *t_data)
{
    trunk_private_t *t_info;
    int             rv, isGport = 0;

    TRUNK_INIT(unit);
    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
        return (BCM_E_NOT_FOUND);
    }

    bcm_trunk_add_info_t_init(t_data);

    TRUNK_LOCK(unit);
    rv = mbcm_driver[unit]->mbcm_trunk_get(unit, tid, t_data, t_info);
    TRUNK_UNLOCK(unit);

    BCM_IF_ERROR_RETURN(
        bcm_esw_switch_control_get(unit, bcmSwitchUseGport, &isGport));
    if (isGport) {
        BCM_IF_ERROR_RETURN(
            _bcm_esw_trunk_gport_construct(unit, TRUNK_FABRIC_TID(unit, tid),
                                           t_data->num_ports,
                                           t_data->tp, t_data->tm,
                                           t_data->tp));
    }

    return rv;
}

/*
 * Function:
 *	bcm_trunk_mcast_join
 * Purpose:
 *	Add the trunk group to existing MAC multicast entry.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      tid - Trunk Id.
 *      vid - Vlan ID.
 *      mac - MAC address.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *      Applications have to remove the MAC multicast entry and re-add in with
 *      new port bitmap to remove the trunk group from MAC multicast entry.
 */

int
bcm_esw_trunk_mcast_join(int unit, bcm_trunk_t tid, bcm_vlan_t vid, sal_mac_addr_t mac)
{
    trunk_private_t *t_info;
    int	rv;

    TRUNK_INIT(unit);
    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);
    if (t_info->tid == BCM_TRUNK_INVALID) {
        return (BCM_E_NOT_FOUND);
    }

    if (!t_info->in_use) {
        return (BCM_E_NONE);
    }

    TRUNK_LOCK(unit);
    rv = mbcm_driver[unit]->mbcm_trunk_mcast_join(unit, tid, vid, mac, t_info);
    TRUNK_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *	bcm_trunk_bitmap_expand
 * Purpose:
 *      Given a port bitmap, if any of the ports are in a trunk,
 *      add all of the ports of that trunk to the bitmap.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      pbmp_ptr - Input/output port bitmap
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_INTERNAL          Chip access failure.
 * Notes:
 *      Port bitmaps in the vtable and ptable must contain only one of
 *      the trunk ports when multiple ports are aggregated in a trunk.
 *      This is so DLF flooding, etc. goes out only one trunk port.
 */

int
bcm_esw_trunk_bitmap_expand(int unit, pbmp_t *pbmp_ptr)
{
    int		    rv = BCM_E_NONE;
    bcm_trunk_t     tid;
    pbmp_t          tports, tpbm;

    TRUNK_LOCK(unit);

#ifdef	BCM_XGS_SWITCH_SUPPORT
    if (SOC_IS_XGS_SWITCH(unit)) {
        pbmp_t    hg_pbmp, nonhg_pbmp;
        int       hg_trunk = FALSE;
        soc_reg_t hg_trunk_bmap;
#ifdef BCM_TRIUMPH2_SUPPORT
        hg_trunk_bmap = (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                         SOC_IS_VALKYRIE2(unit)) ? HG_TRUNK_BITMAP_64r : 
                                                   HG_TRUNK_BITMAPr;
#else
        hg_trunk_bmap = HG_TRUNK_BITMAPr;
#endif
        SOC_PBMP_ASSIGN(hg_pbmp, PBMP_ST_ALL(unit));
        SOC_PBMP_NEGATE(nonhg_pbmp, hg_pbmp);
        SOC_PBMP_AND(hg_pbmp, *pbmp_ptr);
        SOC_PBMP_AND(nonhg_pbmp, *pbmp_ptr);
        if (SOC_PBMP_NOT_NULL(hg_pbmp) && SOC_PBMP_NOT_NULL(nonhg_pbmp)) {
            TRUNK_UNLOCK(unit);
            return BCM_E_PARAM;
        } else if (SOC_PBMP_NOT_NULL(hg_pbmp)) {
            hg_trunk = TRUE;
        }

        if (hg_trunk && SOC_IS_FBX(unit)) {
            uint32     val;

            if ((SOC_IS_FB_FX_HX(unit) || SOC_IS_RAVEN(unit))  && 
                soc_feature(unit, soc_feature_hg_trunking)) {
                rv = READ_HIGIG_TRUNK_CONTROLr(unit, &val);
                if (rv >= 0) {
                    SOC_PBMP_CLEAR(tports);
                    SOC_PBMP_WORD_SET(tports, 0, soc_reg_field_get(unit,
                                                 HIGIG_TRUNK_CONTROLr, val,
                                                 HIGIG_TRUNK_BITMAP0f)<<24);
                    SOC_PBMP_ASSIGN(tpbm, *pbmp_ptr);
                    SOC_PBMP_AND(tpbm, tports);
                    if (SOC_PBMP_NOT_NULL(tpbm)) {
                        SOC_PBMP_OR(*pbmp_ptr, tports);
                    }

                    SOC_PBMP_CLEAR(tports);
                    SOC_PBMP_WORD_SET(tports, 0, soc_reg_field_get(unit,
                                                 HIGIG_TRUNK_CONTROLr, val,
                                                 HIGIG_TRUNK_BITMAP1f)<<24);
                    SOC_PBMP_ASSIGN(tpbm, *pbmp_ptr);
                    SOC_PBMP_AND(tpbm, tports);
                    if (SOC_PBMP_NOT_NULL(tpbm)) {
                        SOC_PBMP_OR(*pbmp_ptr, tports);
                    }
                }
            } 
#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
            else if (SOC_REG_IS_VALID(unit, hg_trunk_bmap)) {
                uint64 val64;
                uint32    hg_trunk_bmap_addr;
                for (tid = 0; tid < TRUNK_CNTL(unit).ngroups_hg; tid++) {
                    uint64    hg_trunk_bmap_f;
                    hg_trunk_bmap_addr = soc_reg_addr(unit, hg_trunk_bmap, 
                                                      REG_PORT_ANY, tid);
                    rv = soc_reg_get(unit, hg_trunk_bmap, REG_PORT_ANY, tid, &val64);
                    if (rv < 0) {
                        break;
                    }
                    SOC_PBMP_CLEAR(tports);
                    hg_trunk_bmap_f = soc_reg64_field_get(unit,
                                                          hg_trunk_bmap,    
                                                          val64,
                                                          HIGIG_TRUNK_BITMAPf);
                    SOC_PBMP_WORD_SET(tports,
                                      0,
                                      COMPILER_64_LO(hg_trunk_bmap_f));
                    SOC_PBMP_ASSIGN(tpbm, *pbmp_ptr);
                    SOC_PBMP_AND(tpbm, tports);
                    if (SOC_PBMP_NOT_NULL(tpbm)) {
                        SOC_PBMP_OR(*pbmp_ptr, tports);
                    }
                }
            }
#endif /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */
#ifdef BCM_TRIDENT_SUPPORT
            else if (soc_mem_is_valid(unit, HG_TRUNK_BITMAPm)) {
                hg_trunk_bitmap_entry_t hg_trunk_bitmap_entry;

                for (tid = 0; tid < TRUNK_CNTL(unit).ngroups_hg; tid++) {
                    SOC_IF_ERROR_RETURN
                        (READ_HG_TRUNK_BITMAPm(unit, MEM_BLOCK_ANY, tid, &hg_trunk_bitmap_entry));
                    SOC_PBMP_CLEAR(tports);
                    soc_mem_pbmp_field_get(unit, HG_TRUNK_BITMAPm, &hg_trunk_bitmap_entry,
                            HIGIG_TRUNK_BITMAPf, &tports);

                    SOC_PBMP_ASSIGN(tpbm, *pbmp_ptr);
                    SOC_PBMP_AND(tpbm, tports);
                    if (SOC_PBMP_NOT_NULL(tpbm)) {
                        SOC_PBMP_OR(*pbmp_ptr, tports);
                    }
                }
            }
#endif /* BCM_TRIDENT_SUPPORT */
        } else {
	    trunk_bitmap_entry_t trunk_bitmap_entry;

            for (tid = 0; tid < TRUNK_CNTL(unit).ngroups_fp; tid++) {
	        rv = READ_TRUNK_BITMAPm(unit, MEM_BLOCK_ANY, tid,
	    			        &trunk_bitmap_entry);
	        if (rv < 0) {
		    break;
	        }

	        SOC_PBMP_CLEAR(tports);


                if (SOC_IS_TUCANA(unit)) {
                    SOC_PBMP_WORD_SET(tports, 0,
		    	    soc_TRUNK_BITMAPm_field32_get(unit,
			    			          &trunk_bitmap_entry,
						          TRUNK_BITMAP_M0f));
                    SOC_PBMP_WORD_SET(tports, 1,
			    soc_TRUNK_BITMAPm_field32_get(unit,
						          &trunk_bitmap_entry,
						          TRUNK_BITMAP_M1f));
                } else {
                    soc_mem_pbmp_field_get(unit, TRUNK_BITMAPm, &trunk_bitmap_entry,
                            TRUNK_BITMAPf, &tports);
                }
	        SOC_PBMP_ASSIGN(tpbm, *pbmp_ptr);
	        SOC_PBMP_AND(tpbm, tports);
                if (SOC_PBMP_NOT_NULL(tpbm)) {
                    SOC_PBMP_OR(*pbmp_ptr, tports);
                }
            }
        }
    }
#endif	/* BCM_XGS_SWITCH_SUPPORT */

#ifdef	BCM_XGS_FABRIC_SUPPORT
    if (SOC_IS_XGS12_FABRIC(unit)) {
        uint32     val;
        bcm_port_t    port;

        for (tid = 0; tid < TRUNK_CNTL(unit).ngroups_hg; tid++) {
             PBMP_HG_ITER(unit, port) {
                 rv = READ_ING_HGTRUNKr(unit, port, tid, &val);
                 break;
             }
             if (rv < 0) {
                 break;
             }
            SOC_PBMP_CLEAR(tports);
    /*    coverity[uninit_use_in_call : FALSE]    */
            SOC_PBMP_WORD_SET(tports, 0, soc_reg_field_get(unit,
                              ING_HGTRUNKr, val, BMAPf));
            SOC_PBMP_ASSIGN(tpbm, *pbmp_ptr);
            SOC_PBMP_AND(tpbm, tports);
            if (SOC_PBMP_NOT_NULL(tpbm)) {
                SOC_PBMP_OR(*pbmp_ptr, tports);
            }
        }
    }
#endif	/* BCM_XGS_FABRIC_SUPPORT */

    TRUNK_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *	bcm_trunk_egress_set
 * Description:
 *	Set switching only to indicated ports from given trunk.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *      tid - Trunk Id.  Negative trunk id means set all trunks.
 *	pbmp - bitmap of ports to allow egress.
 * Returns:
 *      BCM_E_xxxx
 */
int
bcm_esw_trunk_egress_set(int unit, bcm_trunk_t tid, bcm_pbmp_t pbmp)
{
    trunk_egr_mask_entry_t    tem_entry;
    bcm_trunk_t               tid_min, tid_max;

    TRUNK_INIT(unit);
    if (soc_feature(unit,soc_feature_trunk_egress)) {
        if (tid >= TRUNK_CNTL(unit).ngroups_fp) {
            return BCM_E_BADID;
        }

        if (!SOC_IS_XGS_SWITCH(unit)) {
            if (BCM_PBMP_EQ(pbmp, PBMP_ALL(unit))) {
                return BCM_E_NONE;
            } else {
                return BCM_E_UNAVAIL;
            }
        }

        sal_memset(&tem_entry, 0, sizeof(trunk_egr_mask_entry_t));

        BCM_PBMP_NEGATE(pbmp, pbmp);
        BCM_PBMP_AND(pbmp, PBMP_ALL(unit));
        BCM_PBMP_REMOVE(pbmp, PBMP_LB(unit));
        if (SOC_IS_TUCANA(unit)) {
            soc_TRUNK_EGR_MASKm_field32_set(unit, &tem_entry,
                                            TRUNK_EGRESS_MASK_M0f,
                                            SOC_PBMP_WORD_GET(pbmp, 0));
            soc_TRUNK_EGR_MASKm_field32_set(unit, &tem_entry,
                                            TRUNK_EGRESS_MASK_M1f,
                                            SOC_PBMP_WORD_GET(pbmp, 1));
        } else {
            soc_mem_pbmp_field_set(unit, TRUNK_EGR_MASKm, &tem_entry,
                                   TRUNK_EGRESS_MASKf, &pbmp);
        }

        if (tid < 0) {
            tid_min = 0;
            tid_max = TRUNK_CNTL(unit).ngroups_fp - 1;
        } else if (TRUNK_INFO(unit, tid).tid == BCM_TRUNK_INVALID) {
            return BCM_E_NOT_FOUND;
        } else {
            tid_min = tid_max = tid;
        }

        for (tid = tid_min; tid <= tid_max; tid++) {
            if (TRUNK_INFO(unit, tid).tid == BCM_TRUNK_INVALID) {
                continue;
            }
            BCM_IF_ERROR_RETURN
                (WRITE_TRUNK_EGR_MASKm(unit, MEM_BLOCK_ALL, tid, &tem_entry));

            if (SOC_IS_DRACO15(unit)) {
                /*
                 * The EGR_MASKm table must also hold a copy of the pbmp.
                 *
                 * When TGID is directly supplied (SL stack ports), the
                 * mask from the TRUNK_EGRESS_MASKm table is used.
                 *
                 * When modid/port information is supplied (by TRUNKS128
                 * HiGig headers, and local ports), the mask from the
                 * EGR_MASKm table is used.
                 *
                 * Extract info from TRUNK_GROUPm to get the modid/port
                 * information to index the EGR_MASKm table.
                 *
                 * EGR_MASK T/TGID fields were previously initialized
                 * by bcm_trunk_set().
                 */

                bcm_trunk_add_info_t t_data;
                egr_mask_entry_t     egr_mask_entry;
                int                  rv, size, em_index, isGport;

                BCM_IF_ERROR_RETURN
                    (bcm_esw_trunk_get(unit, tid, &t_data));

                BCM_IF_ERROR_RETURN
                    (bcm_esw_switch_control_get(unit, bcmSwitchUseGport, &isGport));
                if (isGport) {
                    BCM_IF_ERROR_RETURN(
                            _bcm_esw_trunk_gport_resolve(unit, tid, &t_data));
                }

                /* num_ports = 7..0 for trunk sizes 8..1 */
                size = t_data.num_ports-1; 

                rv = BCM_E_NONE;
                soc_mem_lock(unit, EGR_MASKm);
                for (; size>=0; size--) {
                    em_index = t_data.tm[size]*32 + t_data.tp[size];
                    rv = soc_mem_read(unit, EGR_MASKm, MEM_BLOCK_ANY,
                            em_index, &egr_mask_entry);
                    if (!BCM_SUCCESS(rv)) {
                        break;
                    }
                    soc_EGR_MASKm_field32_set(unit, &egr_mask_entry,
                            EGRESS_MASKf,
                            SOC_PBMP_WORD_GET(pbmp, 0));
                    rv = soc_mem_write(unit, EGR_MASKm, MEM_BLOCK_ALL,
                            em_index, &egr_mask_entry);
                    if (!BCM_SUCCESS(rv)) {
                        break;
                    }
                }
                soc_mem_unlock(unit, EGR_MASKm);
                if (!BCM_SUCCESS(rv)) {
                    return rv;
                }
            }
        }
    } else {
        return BCM_E_UNAVAIL;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_trunk_egress_get
 * Description:
 *	Retrieve bitmap of ports for which switching is enabled for trunk.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *      tid - Trunk Id.  Negative trunk id means choose any trunk.
 *	pbmp - (OUT) bitmap of ports where egress allowed.
 * Returns:
 *      BCM_E_xxxx
 */
int
bcm_esw_trunk_egress_get(int unit, bcm_trunk_t tid, bcm_pbmp_t *pbmp)
{
    trunk_egr_mask_entry_t    tem_entry;
    bcm_pbmp_t                tem_pbmp;

    TRUNK_INIT(unit);
    if (soc_feature(unit,soc_feature_trunk_egress)) {
        if (tid >= TRUNK_CNTL(unit).ngroups_fp) {
            return BCM_E_BADID;
        }

        if (!SOC_IS_XGS_SWITCH(unit)) {
            BCM_PBMP_ASSIGN(*pbmp, PBMP_ALL(unit));
            return BCM_E_NONE;
        }

        if (tid < 0) {
    	tid = 0;
        }

        if (TRUNK_INFO(unit, tid).tid == BCM_TRUNK_INVALID) {
            return BCM_E_NOT_FOUND;
        }

        BCM_PBMP_CLEAR(tem_pbmp);

        BCM_IF_ERROR_RETURN
            (READ_TRUNK_EGR_MASKm(unit, MEM_BLOCK_ANY, tid, &tem_entry));

        if (SOC_IS_TUCANA(unit)) {
    	SOC_PBMP_WORD_SET(tem_pbmp, 0,
    			  soc_TRUNK_EGR_MASKm_field32_get(unit, &tem_entry,
    						  TRUNK_EGRESS_MASK_M0f));
    	SOC_PBMP_WORD_SET(tem_pbmp, 1,
    			  soc_TRUNK_EGR_MASKm_field32_get(unit, &tem_entry,
    						  TRUNK_EGRESS_MASK_M1f));
        } else {
            soc_mem_pbmp_field_get(unit, TRUNK_EGR_MASKm, &tem_entry,
                                   TRUNK_EGRESS_MASKf, &tem_pbmp);
        }
        BCM_PBMP_ASSIGN(*pbmp, PBMP_ALL(unit));
        BCM_PBMP_REMOVE(*pbmp, PBMP_LB(unit));
        BCM_PBMP_XOR(*pbmp, tem_pbmp);
    } else{
        return BCM_E_UNAVAIL;
    }

    return BCM_E_NONE;
}

#if defined(BCM_HERCULES15_SUPPORT) || defined(BCM_FIREBOLT_SUPPORT)
#define _TRUNK_OVERRIDE_CHECK(_u, _p, _tid) \
        TRUNK_INIT(_u); \
        if (!SOC_IS_HERCULES15(_u) && (!SOC_IS_FBX(_u) || \
        !soc_feature(_u, soc_feature_hg_trunk_override)))  \
           { return (BCM_E_UNAVAIL); } \
        if (!TRUNK_FABRIC_TID(_u, _tid)) \
           { return (BCM_E_PARAM); } \
        if ((_p >= 0) && !IS_PORT(_u, _p)) \
           { return (BCM_E_PARAM); }
#define _UCAST_RANGE_CHECK(_u, _id) \
        if (SOC_IS_HERCULES15(_u)) { \
            if ((_id < soc_mem_index_min(_u, MEM_UCm) || \
                _id > soc_mem_index_max(_u, MEM_UCm))) \
            { return (BCM_E_PARAM); } \
         } else if (soc_mem_is_valid(_u, MODPORT_MAPm)) { \
            if ((_id < soc_mem_index_min(_u, MODPORT_MAPm) || \
                _id > soc_mem_index_max(_u, MODPORT_MAPm))) \
             { return (BCM_E_PARAM); } \
         } else if (soc_mem_is_valid(_u, MODPORT_MAP_SWm)) { \
            if ((_id < soc_mem_index_min(_u, MODPORT_MAP_SWm) || \
                _id > soc_mem_index_max(_u, MODPORT_MAP_SWm))) \
             { return (BCM_E_PARAM); }}
#define _MCAST_RANGE_CHECK(_u, _id) \
        if (SOC_IS_HERCULES15(_u)) { \
            if ((_id < soc_mem_index_min(_u, MEM_MCm) || \
                _id > soc_mem_index_max(_u, MEM_MCm))) \
            { return (BCM_E_PARAM); } \
        } else { \
            if ((_id < soc_mem_index_min(_u, L2MCm) || \
                _id > soc_mem_index_max(_u, L2MCm))) \
             { return (BCM_E_PARAM); }}
#define _IPMC_RANGE_CHECK(_u, _id) \
        if (SOC_IS_HERCULES15(_u)) { \
            if ((_id < soc_mem_index_min(_u, MEM_IPMCm) || \
                _id > soc_mem_index_max(_u, MEM_IPMCm))) \
            { return (BCM_E_PARAM); } \
        } else { \
	    if ((_id < soc_mem_index_min(_u, L3_IPMCm) || \
                _id > soc_mem_index_max(_u, L3_IPMCm))) \
            { return (BCM_E_PARAM); }}
#define _VLAN_RANGE_CHECK(_u, _id) \
        if (SOC_IS_HERCULES15(_u)) { \
            if ((_id < soc_mem_index_min(_u, MEM_VIDm) || \
                _id > soc_mem_index_max(_u, MEM_VIDm))) \
            { return (BCM_E_PARAM); } \
        } else { \
            if ((_id < soc_mem_index_min(_u, VLAN_TABm) || \
                _id > soc_mem_index_max(_u, VLAN_TABm))) \
             { return (BCM_E_PARAM); }}
#endif  /* BCM_HERCULES15_SUPPORT  || BCM_FIREBOLT_SUPPORT */

/*
 * Function:
 *      bcm_trunk_override_ucast_set
 * Description:
 *      Set the trunk override over UC.
 * Parameters:
 *      unit   - StrataSwitch PCI device unit number (driver internal).
 *      port   - Port number, -1 to all ports.
 *      tid    - Trunk id.
 *      modid  - Module id.
 *      enable - TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_esw_trunk_override_ucast_set(int unit, bcm_port_t port,
                             bcm_trunk_t tid, int modid, int enable)
{
#if defined(BCM_HERCULES15_SUPPORT) || defined(BCM_FIREBOLT_SUPPORT)
    int rv=BCM_E_NONE;

    if (port >= 0) {
        if (BCM_GPORT_IS_SET(port)) {
            bcm_trunk_t     tgid;
            int             id;

            BCM_IF_ERROR_RETURN(
                _bcm_esw_gport_resolve(unit, port, &modid, &port, &tgid, &id));
            if ((-1 != id) || (-1 != tgid)){
                return BCM_E_PARAM;
            }
        } else {
            if (!SOC_PORT_VALID(unit, port)) {
                return BCM_E_PORT;
            }
        }
    }
    _TRUNK_OVERRIDE_CHECK(unit, port, tid);
    _UCAST_RANGE_CHECK(unit, modid);

#ifdef  BCM_HERCULES15_SUPPORT
    if (SOC_IS_HERCULES15(unit)) {
        mem_uc_entry_t      uc;
        soc_field_t tf[] = {TRUNK0_OVER_UCf,TRUNK1_OVER_UCf,
                            TRUNK2_OVER_UCf,TRUNK3_OVER_UCf};
        int bk=-1, blk;

        if (port >= 0) {
            bk = SOC_PORT_BLOCK(unit, port);
        }

        soc_mem_lock(unit, MEM_UCm);
        SOC_MEM_BLOCK_ITER(unit, MEM_UCm, blk) {
            if ((port >= 0) && (bk != blk)) {
                continue;
            }
            rv = READ_MEM_UCm(unit, blk, modid, &uc);
            if (rv >= 0) {
                soc_MEM_UCm_field32_set(unit, &uc,
                                        tf[tid], enable ? 1: 0);
                rv = WRITE_MEM_UCm(unit, blk, modid, &uc);
            }
        }
        soc_mem_unlock(unit, MEM_UCm);
    }
#endif /* BCM_HERCULES15_SUPPORT */

#ifdef BCM_FIREBOLT_SUPPORT

    if ((SOC_IS_FBX(unit) || SOC_IS_RAVEN(unit)) && 
        (soc_feature(unit,soc_feature_hg_trunk_override)) ) { 
        modport_map_entry_t uc;
        uint32 val, hgo_bit;
        int i, min, max, idx, modid_count;
        soc_mem_t modport_map_mem;

        modid_count = SOC_MODID_MAX(unit) + 1;
        if (soc_mem_is_valid(unit, MODPORT_MAP_SWm)) {
            modport_map_mem = MODPORT_MAP_SWm;
        } else { 
            modport_map_mem = MODPORT_MAPm;
        }
        min = soc_mem_index_min(unit, modport_map_mem) / modid_count;
        max = soc_mem_index_max(unit, modport_map_mem) / modid_count;
        if (SOC_IS_HBX(unit)) {
            if (port >= min && port <= max) {
                min = max = port;
            }
        }

#ifdef BCM_TRIUMPH_SUPPORT
        if (soc_feature(unit, soc_feature_modport_map_profile)) {
            bcm_port_t ing_port;
            bcm_trunk_t hgtid;
            hgtid = tid - TRUNK_CNTL(unit).ngroups_fp;
            if (port == BCM_GPORT_INVALID) {
                PBMP_ALL_ITER(unit, ing_port) {
#ifdef BCM_TRIDENT_SUPPORT
                    if (soc_feature(unit,
                                soc_feature_modport_map_dest_is_port_or_trunk)) {
                        BCM_IF_ERROR_RETURN
                            (bcm_td_stk_trunk_override_ucast_set(unit,
                                ing_port, hgtid, modid, enable));
                    } else
#endif /* BCM_TRIDENT_SUPPORT */
                    {
                        BCM_IF_ERROR_RETURN
                            (_bcm_esw_tr_trunk_override_ucast_set(unit,
                                ing_port, hgtid, modid, enable));
                    }
                }
            } else {
#ifdef BCM_TRIDENT_SUPPORT
                if (soc_feature(unit,
                            soc_feature_modport_map_dest_is_port_or_trunk)) {
                    BCM_IF_ERROR_RETURN
                        (bcm_td_stk_trunk_override_ucast_set(unit,
                            port, hgtid, modid, enable));
                } else
#endif /* BCM_TRIDENT_SUPPORT */
                {
                    BCM_IF_ERROR_RETURN
                        (_bcm_esw_tr_trunk_override_ucast_set(unit,
                            port, hgtid, modid, enable));
                }
            }
        } else 
#endif /* BCM_TRIUMPH_SUPPORT */
        {
            soc_mem_lock(unit, MODPORT_MAPm);
            for (i = min; i <= max; i++) {
                idx = i * modid_count + modid;
                rv = READ_MODPORT_MAPm(unit, MEM_BLOCK_ANY, idx, &uc);
                if (rv >= 0) {
                    hgo_bit = 1 << (tid - TRUNK_CNTL(unit).ngroups_fp);
                    val = soc_MODPORT_MAPm_field32_get(unit, &uc,
                            HIGIG_TRUNK_OVERRIDEf);
                    if (enable) {
                        val |= hgo_bit;
                    } else {
                        val &= ~hgo_bit;
                    }
                    soc_MODPORT_MAPm_field32_set(unit, &uc,
                            HIGIG_TRUNK_OVERRIDEf, val);
                    rv = WRITE_MODPORT_MAPm(unit, MEM_BLOCK_ALL, idx, &uc);
                }
            }
            soc_mem_unlock(unit, MODPORT_MAPm);
        }
    }
#endif /* BCM_FIREBOLT_SUPPORT */

    return rv;
#else
    return BCM_E_UNAVAIL;
#endif  /* BCM_HERCULES15_SUPPORT  || BCM_FIREBOLT_SUPPORT */
}

/*
 * Function:
 *      bcm_trunk_override_ucast_get
 * Description:
 *      Get the trunk override over UC.
 * Parameters:
 *      unit   - StrataSwitch PCI device unit number (driver internal).
 *      port   - Port number.
 *      tid    - Trunk id.
 *      modid  - Module id.
 *      enable - (OUT) TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_esw_trunk_override_ucast_get(int unit, bcm_port_t port,
                             bcm_trunk_t tid, int modid, int *enable)
{
#if defined(BCM_HERCULES15_SUPPORT) || defined(BCM_FIREBOLT_SUPPORT)
    int rv=BCM_E_NONE;

    if (BCM_GPORT_IS_SET(port)) {
        bcm_trunk_t     tgid;
        int             id;

        BCM_IF_ERROR_RETURN(
            _bcm_esw_gport_resolve(unit, port, &modid, &port, &tgid, &id));
        if ((-1 != id) || (-1 != tgid)){
            return BCM_E_PARAM;
        }
    } else {
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
    }
    
    _TRUNK_OVERRIDE_CHECK(unit, port, tid);
    _UCAST_RANGE_CHECK(unit, modid);

#ifdef  BCM_HERCULES15_SUPPORT
    if (SOC_IS_HERCULES15(unit)) {
        mem_uc_entry_t      uc;
        soc_field_t tf[] = {TRUNK0_OVER_UCf,TRUNK1_OVER_UCf,
                            TRUNK2_OVER_UCf,TRUNK3_OVER_UCf};
        int blk;

        *enable = 0;

        blk = SOC_PORT_BLOCK(unit, port);
        rv = READ_MEM_UCm(unit, blk, modid, &uc);
        if (rv >= 0) {
            *enable = soc_MEM_UCm_field32_get(unit, &uc, tf[tid]);
        }
    }
#endif /* BCM_HERCULES15_SUPPORT */

#ifdef BCM_FIREBOLT_SUPPORT

    if ((SOC_IS_FBX(unit) || SOC_IS_RAVEN(unit)) && 
        (soc_feature(unit,soc_feature_hg_trunk_override)) ) { 
        modport_map_entry_t uc;
        uint32 val, hgo_bit;
        int idx;

        if (SOC_IS_FB_FX_HX(unit) || SOC_IS_TR_VL(unit)) {
            idx = modid;
        } else {
            idx = (port * (SOC_MODID_MAX(unit) + 1)) + modid;
        }
#ifdef BCM_TRIUMPH_SUPPORT
        if (soc_feature(unit, soc_feature_modport_map_profile)) {
            bcm_trunk_t hgtid;
            hgtid = tid - TRUNK_CNTL(unit).ngroups_fp;
#ifdef BCM_TRIDENT_SUPPORT
            if (soc_feature(unit, soc_feature_modport_map_dest_is_port_or_trunk)) {
                rv = bcm_td_stk_trunk_override_ucast_get(unit, port, hgtid,
                        modid, enable);
            } else
#endif /* BCM_TRIDENT_SUPPORT */
            {
                rv = _bcm_esw_tr_trunk_override_ucast_get(unit, port, hgtid,
                        modid, enable); 
            }
        } else 
#endif /* BCM_TRIUMPH_SUPPORT */
        {
            *enable = 0;

            rv = READ_MODPORT_MAPm(unit, MEM_BLOCK_ANY, idx, &uc);
            if (rv >= 0) {
                val = soc_MODPORT_MAPm_field32_get(unit, &uc,
                                                   HIGIG_TRUNK_OVERRIDEf);
                hgo_bit = 1 << (tid - TRUNK_CNTL(unit).ngroups_fp);
                *enable = (val & hgo_bit) ? 1 : 0;
            }
        }
    }
#endif /* BCM_FIREBOLT_SUPPORT */

    return rv;
#else
    return BCM_E_UNAVAIL;
#endif  /* BCM_HERCULES15_SUPPORT */
}

/*
 * Function:
 *      bcm_trunk_override_mcast_set
 * Description:
 *      Set the trunk override over MC.
 * Parameters:
 *      unit   - StrataSwitch PCI device unit number (driver internal).
 *      port   - Port number, -1 to all ports.
 *      tid    - Trunk id.
 *      idx    - MC index carried in HiGig header.
 *      enable - TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_esw_trunk_override_mcast_set(int unit, bcm_port_t port,
                                 bcm_trunk_t tid, int idx, int enable)
{
#if defined(BCM_HERCULES15_SUPPORT) || defined(BCM_FIREBOLT_SUPPORT)
    int rv=BCM_E_NONE;

    /* Translate multicast group ID's */
    if (_BCM_MULTICAST_IS_L2(idx)) {
        idx = _BCM_MULTICAST_ID_GET(idx);
    } else if (_BCM_MULTICAST_IS_SET(idx)) {
        /* This is a non-L2 multicast group.
         * Redirect to the IPMC version below */
        idx = _BCM_MULTICAST_ID_GET(idx);
        return bcm_esw_trunk_override_ipmc_set(unit, port, tid,
                                               idx, enable);
    }

    if (port >= 0) {
        if (BCM_GPORT_IS_SET(port)) {
            bcm_module_t    modid;
            bcm_trunk_t     tgid;
            int             id;

            BCM_IF_ERROR_RETURN(
                _bcm_esw_gport_resolve(unit, port, &modid, &port, &tgid, &id));
            if ((-1 != id) || (-1 != tgid)){
                return BCM_E_PARAM;
            }
        } else {
            if (!SOC_PORT_VALID(unit, port)) {
                return BCM_E_PORT;
            }
        }
    }

    _TRUNK_OVERRIDE_CHECK(unit, port, tid);
    _MCAST_RANGE_CHECK(unit, idx);

#ifdef BCM_HERCULES15_SUPPORT
    if (SOC_IS_HERCULES15(unit)) {
        mem_mc_entry_t      mc;
        soc_field_t tf[] = {TRUNK0_OVER_MCf,TRUNK1_OVER_MCf,
                            TRUNK2_OVER_MCf,TRUNK3_OVER_MCf};
        int  bk=-1, blk;

        if (port >= 0) {
            bk = SOC_PORT_BLOCK(unit, port);
        }

        soc_mem_lock(unit, MEM_MCm);
        SOC_MEM_BLOCK_ITER(unit, MEM_MCm, blk) {
            if ((port >= 0) && (bk != blk)) {
                continue;
            }
            rv = READ_MEM_MCm(unit, blk, idx, &mc);
            if (rv >= 0) {
                soc_MEM_MCm_field32_set(unit, &mc,
                                        tf[tid], enable ? 1: 0);
                rv = WRITE_MEM_MCm(unit, blk, idx, &mc);
            }
        }
        soc_mem_unlock(unit, MEM_MCm);
    }
#endif /* BCM_HERCULES15_SUPPORT */

#ifdef BCM_FIREBOLT_SUPPORT


    if ((SOC_IS_FBX(unit) || SOC_IS_RAVEN(unit)) && 
        (soc_feature(unit,soc_feature_hg_trunk_override)) ) { 

#ifdef BCM_TRIDENT_SUPPORT
        if (soc_feature(unit, soc_feature_hg_trunk_override_profile)) {
            bcm_trunk_t hgtid;

            hgtid = tid - TRUNK_CNTL(unit).ngroups_fp;
            rv = _bcm_trident_trunk_override_mcast_set(unit, hgtid, idx, enable); 
        } else 
#endif /* BCM_TRIDENT_SUPPORT */
        {
            l2mc_entry_t mc;
            uint32 val, hgo_bit;

            soc_mem_lock(unit, L2MCm);
            rv = READ_L2MCm(unit, MEM_BLOCK_ANY, idx, &mc);
            if (rv >= 0) {
                val = soc_L2MCm_field32_get(unit, &mc,
                        HIGIG_TRUNK_OVERRIDEf);
                hgo_bit = 1 << (tid - TRUNK_CNTL(unit).ngroups_fp);
                if (enable) {
                    val |= hgo_bit;
                } else {
                    val &= ~hgo_bit;
                }
                soc_L2MCm_field32_set(unit, &mc,
                        HIGIG_TRUNK_OVERRIDEf, val);
                rv = WRITE_L2MCm(unit, MEM_BLOCK_ALL, idx, &mc);
            }
            soc_mem_unlock(unit, L2MCm);
        }
    }
#endif /* BCM_FIREBOLT_SUPPORT */

    return rv;
#else
    return BCM_E_UNAVAIL;
#endif  /* BCM_HERCULES15_SUPPORT || BCM_FIREBOLT_SUPPORT */
}

/*
 * Function:
 *      bcm_trunk_override_mcast_get
 * Description:
 *      Get the trunk override over MC.
 * Parameters:
 *      unit   - StrataSwitch PCI device unit number (driver internal).
 *      port   - Port number.
 *      tid    - Trunk id.
 *      idx    - MC index carried in HiGig header.
 *      enable - (OUT) TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_esw_trunk_override_mcast_get(int unit, bcm_port_t port,
                                 bcm_trunk_t tid, int idx, int *enable)
{
#if defined(BCM_HERCULES15_SUPPORT) || defined(BCM_FIREBOLT_SUPPORT)
    int rv=BCM_E_NONE;

    /* Translate multicast group ID's */
    if (_BCM_MULTICAST_IS_L2(idx)) {
        idx = _BCM_MULTICAST_ID_GET(idx);
    } else if (_BCM_MULTICAST_IS_SET(idx)) {
        /* This is a non-L2 multicast group.
         * Redirect to the IPMC version below */
        idx = _BCM_MULTICAST_ID_GET(idx);
        return bcm_esw_trunk_override_ipmc_get(unit, port, tid,
                                               idx, enable);
    }

    if (BCM_GPORT_IS_SET(port)) {
        bcm_module_t    modid;
        bcm_trunk_t     tgid;
        int             id;

        BCM_IF_ERROR_RETURN(
            _bcm_esw_gport_resolve(unit, port, &modid, &port, &tgid, &id));
        if ((-1 != id) || (-1 != tgid)){
            return BCM_E_PARAM;
        }
    } else {
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
    }

    _TRUNK_OVERRIDE_CHECK(unit, port, tid);
    _MCAST_RANGE_CHECK(unit, idx);

#ifdef BCM_HERCULES15_SUPPORT
    if (SOC_IS_HERCULES15(unit)) {
        mem_mc_entry_t      mc;
        soc_field_t tf[] = {TRUNK0_OVER_MCf,TRUNK1_OVER_MCf,
                            TRUNK2_OVER_MCf,TRUNK3_OVER_MCf};
        int blk;

        *enable = 0;

        blk = SOC_PORT_BLOCK(unit, port);
        rv = READ_MEM_MCm(unit, blk, idx, &mc);
        if (rv >= 0) {
            *enable = soc_MEM_MCm_field32_get(unit, &mc, tf[tid]);
        }
    }
#endif /* SOC_IS_HERCULES15 */

#ifdef BCM_FIREBOLT_SUPPORT

    if ((SOC_IS_FBX(unit) || SOC_IS_RAVEN(unit)) && 
            (soc_feature(unit,soc_feature_hg_trunk_override))) { 

#ifdef BCM_TRIDENT_SUPPORT
        if (soc_feature(unit, soc_feature_hg_trunk_override_profile)) {
            int hgtid;

            hgtid = tid - TRUNK_CNTL(unit).ngroups_fp;
            rv = _bcm_trident_trunk_override_mcast_get(unit, hgtid, idx, enable); 
        } else
#endif /* BCM_TRIDENT_SUPPORT */
        {
            l2mc_entry_t mc;
            uint32 val, hgo_bit;

            *enable = 0;

            rv = READ_L2MCm(unit, MEM_BLOCK_ANY, idx, &mc);
            if (rv >= 0) {
                val = soc_L2MCm_field32_get(unit, &mc,
                        HIGIG_TRUNK_OVERRIDEf);
                hgo_bit = 1 << (tid - TRUNK_CNTL(unit).ngroups_fp);
                *enable = (val & hgo_bit) ? 1 : 0;
            }
        }
    }
#endif /* BCM_FIREBOLT_SUPPORT */

    return rv;
#else
    return BCM_E_UNAVAIL;
#endif  /* BCM_HERCULES15_SUPPORT  || BCM_FIREBOLT_SUPPORT */
}

/*
 * Function:
 *      bcm_trunk_override_ipmc_set
 * Description:
 *      Set the trunk override over IPMC.
 * Parameters:
 *      unit   - StrataSwitch PCI device unit number (driver internal).
 *      port   - Port number, -1 to all ports.
 *      tid    - Trunk id.
 *      idx    - IPMC index carried in HiGig header.
 *      enable - TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_esw_trunk_override_ipmc_set(int unit, bcm_port_t port,
                                bcm_trunk_t tid, int idx, int enable)
{
#if defined(BCM_HERCULES15_SUPPORT) || defined(BCM_FIREBOLT_SUPPORT)
    int rv=BCM_E_NONE;

    /* Translate multicast group ID's */
    if (_BCM_MULTICAST_IS_L2(idx)) {
        /* This is an L2 multicast group.
         * Redirect to the mcast version above */
        idx = _BCM_MULTICAST_ID_GET(idx);
        return bcm_esw_trunk_override_mcast_set(unit, port, tid,
                                                idx, enable);
    } else if (_BCM_MULTICAST_IS_SET(idx)) {
        idx = _BCM_MULTICAST_ID_GET(idx);
    }

    if (!soc_feature(unit, soc_feature_ip_mcast)) {
        return BCM_E_UNAVAIL;
    }

    if (port >= 0) {
        if (BCM_GPORT_IS_SET(port)) {
            bcm_module_t    modid;
            bcm_trunk_t     tgid;
            int             id;

            BCM_IF_ERROR_RETURN(
                _bcm_esw_gport_resolve(unit, port, &modid, &port, &tgid, &id));
            if ((-1 != id) || (-1 != tgid)){
                return BCM_E_PARAM;
            }
        } else {
            if (!SOC_PORT_VALID(unit, port)) {
                return BCM_E_PORT;
            }
        }
    }

    _TRUNK_OVERRIDE_CHECK(unit, port, tid);
    _IPMC_RANGE_CHECK(unit, idx);

#ifdef  BCM_HERCULES15_SUPPORT
    if (SOC_IS_HERCULES15(unit)) {
        mem_ipmc_entry_t      ipmc;
        soc_field_t tf[] = {TRUNK0_OVER_IPMCf,TRUNK1_OVER_IPMCf,
                            TRUNK2_OVER_IPMCf,TRUNK3_OVER_IPMCf};
        int bk=-1, blk;

        if (port >= 0) {
            bk = SOC_PORT_BLOCK(unit, port);
        }

        soc_mem_lock(unit, MEM_IPMCm);
        SOC_MEM_BLOCK_ITER(unit, MEM_IPMCm, blk) {
            if ((port >= 0) && (bk != blk)) {
                continue;
            }
            rv = READ_MEM_IPMCm(unit, blk, idx, &ipmc);
            if (rv >= 0) {
                soc_MEM_IPMCm_field32_set(unit, &ipmc,
                                          tf[tid], enable ? 1: 0);
                rv = WRITE_MEM_IPMCm(unit, blk, idx, &ipmc);
            }
        }
        soc_mem_unlock(unit, MEM_IPMCm);
    }
#endif /* BCM_HERCULES15_SUPPORT */

#ifdef BCM_FIREBOLT_SUPPORT
    if ((SOC_IS_FBX(unit) || SOC_IS_RAVEN(unit)) && 
        (soc_feature(unit,soc_feature_hg_trunk_override)) ) { 

#ifdef BCM_TRIDENT_SUPPORT 
        if (soc_feature(unit, soc_feature_hg_trunk_override_profile)) {
            bcm_trunk_t hgtid;

            hgtid = tid - TRUNK_CNTL(unit).ngroups_fp;
            rv = _bcm_trident_trunk_override_ipmc_set(unit, hgtid, idx, enable); 
        } else 
#endif /* BCM_TRIDENT_SUPPORT */
        {
            ipmc_entry_t ipmc;
            uint32 val, hgo_bit;

            soc_mem_lock(unit, L3_IPMCm);
            rv = READ_L3_IPMCm(unit, MEM_BLOCK_ANY, idx, &ipmc);
            if (rv >= 0) {
                val = soc_L3_IPMCm_field32_get(unit, &ipmc,
                        HIGIG_TRUNK_OVERRIDEf);
                hgo_bit = 1 << (tid - TRUNK_CNTL(unit).ngroups_fp);
                if (enable) {
                    val |= hgo_bit;
                } else {
                    val &= ~hgo_bit;
                }
                soc_L3_IPMCm_field32_set(unit, &ipmc,
                        HIGIG_TRUNK_OVERRIDEf, val);
                rv = WRITE_L3_IPMCm(unit, MEM_BLOCK_ALL, idx, &ipmc);
            }
            soc_mem_unlock(unit, L3_IPMCm);

#if defined(BCM_BRADLEY_SUPPORT)
            if (SOC_IS_HBX(unit)) {
                l2mc_entry_t mc;
                uint32 val, hgo_bit;
                int	mc_base, mc_size, mc_index;

                SOC_IF_ERROR_RETURN
                    (soc_hbx_ipmc_size_get(unit, &mc_base, &mc_size));
                if (idx < 0 || idx > mc_size) {
                    return BCM_E_PARAM;
                }
                mc_index = idx + mc_base;

                soc_mem_lock(unit, L2MCm);
                rv = READ_L2MCm(unit, MEM_BLOCK_ANY, mc_index, &mc);
                if (rv >= 0) {
                    val = soc_L2MCm_field32_get(unit, &mc,
                            HIGIG_TRUNK_OVERRIDEf);
                    hgo_bit = 1 << (tid - TRUNK_CNTL(unit).ngroups_fp);
                    if (enable) {
                        val |= hgo_bit;
                    } else {
                        val &= ~hgo_bit;
                    }
                    soc_L2MCm_field32_set(unit, &mc,
                            HIGIG_TRUNK_OVERRIDEf, val);
                    rv = WRITE_L2MCm(unit, MEM_BLOCK_ALL, mc_index, &mc);
                }
                soc_mem_unlock(unit, L2MCm);

                
            }
#endif /* BCM_BRADLEY_SUPPORT */
        }
    }
#endif /* BCM_FIREBOLT_SUPPORT */

    return rv;
#else
    return BCM_E_UNAVAIL;
#endif  /* BCM_HERCULES15_SUPPORT || BCM_FIREBOLT_SUPPORT */
}

/*
 * Function:
 *      bcm_trunk_override_ipmc_get
 * Description:
 *      Get the trunk override over IPMC.
 * Parameters:
 *      unit   - StrataSwitch PCI device unit number (driver internal).
 *      port   - Port number.
 *      tid    - Trunk id.
 *      idx    - IPMC index carried in HiGig header.
 *      enable - (OUT) TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_esw_trunk_override_ipmc_get(int unit, bcm_port_t port,
                                bcm_trunk_t tid, int idx, int *enable)
{
#if defined(BCM_HERCULES15_SUPPORT) || defined(BCM_FIREBOLT_SUPPORT)
    int rv=BCM_E_NONE;

    /* Translate multicast group ID's */
    if (_BCM_MULTICAST_IS_L2(idx)) {
        /* This is an L2 multicast group.
         * Redirect to the mcast version above */
        idx = _BCM_MULTICAST_ID_GET(idx);
        return bcm_esw_trunk_override_mcast_get(unit, port, tid,
                                                idx, enable);
    } else if (_BCM_MULTICAST_IS_SET(idx)) {
        idx = _BCM_MULTICAST_ID_GET(idx);
    }

    if (!soc_feature(unit, soc_feature_ip_mcast)) {
        return BCM_E_UNAVAIL;
    }

    if (BCM_GPORT_IS_SET(port)) {
        bcm_module_t    modid;
        bcm_trunk_t     tgid;
        int             id;

        BCM_IF_ERROR_RETURN(
            _bcm_esw_gport_resolve(unit, port, &modid, &port, &tgid, &id));
        if ((-1 != id) || (-1 != tgid)){
            return BCM_E_PARAM;
        }
    } else {
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
    }

    _TRUNK_OVERRIDE_CHECK(unit, port, tid);
    _IPMC_RANGE_CHECK(unit, idx);

#ifdef  BCM_HERCULES15_SUPPORT
    if (SOC_IS_HERCULES15(unit)) {
        mem_ipmc_entry_t      ipmc;
        soc_field_t tf[] = {TRUNK0_OVER_IPMCf,TRUNK1_OVER_IPMCf,
                            TRUNK2_OVER_IPMCf,TRUNK3_OVER_IPMCf};
        int blk;

        *enable = 0;

        blk = SOC_PORT_BLOCK(unit, port);
        rv = READ_MEM_IPMCm(unit, blk, idx, &ipmc);
        if (rv >= 0) {
            *enable = soc_MEM_IPMCm_field32_get(unit, &ipmc, tf[tid]);
        }
    }
#endif /* BCM_HERCULES15_SUPPORT */

#ifdef BCM_FIREBOLT_SUPPORT

    if ((SOC_IS_FBX(unit) || SOC_IS_RAVEN(unit)) && 
            (soc_feature(unit,soc_feature_hg_trunk_override)) ) { 

#ifdef BCM_TRIDENT_SUPPORT 
        if (soc_feature(unit, soc_feature_hg_trunk_override_profile)) {
            bcm_trunk_t hgtid;

            hgtid = tid - TRUNK_CNTL(unit).ngroups_fp;
            rv = _bcm_trident_trunk_override_ipmc_get(unit, hgtid, idx, enable); 
        } else 
#endif /* BCM_TRIDENT_SUPPORT */
        {
            ipmc_entry_t ipmc;
            uint32 val, hgo_bit;

            *enable = 0;

            rv = READ_L3_IPMCm(unit, MEM_BLOCK_ANY, idx, &ipmc);
            if (rv >= 0) {
                val = soc_L3_IPMCm_field32_get(unit, &ipmc,
                        HIGIG_TRUNK_OVERRIDEf);
                hgo_bit = 1 << (tid - TRUNK_CNTL(unit).ngroups_fp);
                *enable = (val & hgo_bit) ? 1 : 0;
            }
        }
    }
#endif /* BCM_FIREBOLT_SUPPORT */

    return rv;
#else
    return BCM_E_UNAVAIL;
#endif  /* BCM_HERCULES15_SUPPORT  || BCM_FIREBOLT_SUPPORT */
}

/*
 * Function:
 *      bcm_trunk_override_vlan_set
 * Description:
 *      Set the trunk override over VLAN.
 * Parameters:
 *      unit   - StrataSwitch PCI device unit number (driver internal).
 *      port   - Port number, -1 to all ports.
 *      tid    - Trunk id.
 *      vid    - VLAN id.
 *      enable - TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_esw_trunk_override_vlan_set(int unit, bcm_port_t port,
                            bcm_trunk_t tid, bcm_vlan_t vid, int enable)
{
#if defined(BCM_HERCULES15_SUPPORT) || defined(BCM_FIREBOLT_SUPPORT)
    int rv=BCM_E_NONE;

    if (port >= 0) {
        if (BCM_GPORT_IS_SET(port)) {
            bcm_module_t    modid;
            bcm_trunk_t     tgid;
            int             id;

            BCM_IF_ERROR_RETURN(
                _bcm_esw_gport_resolve(unit, port, &modid, &port, &tgid, &id));
            if ((-1 != id) || (-1 != tgid)){
                return BCM_E_PARAM;
            }
        } else {
            if (!SOC_PORT_VALID(unit, port)) {
                return BCM_E_PORT;
            }
        }
    }

    _TRUNK_OVERRIDE_CHECK(unit, port, tid);
    _VLAN_RANGE_CHECK(unit, vid);

#ifdef  BCM_HERCULES15_SUPPORT
    if (SOC_IS_HERCULES15(unit)) {
        mem_vid_entry_t      ve;
        soc_field_t tf[] = {TRUNK0_OVER_VIDf,TRUNK1_OVER_VIDf,
                            TRUNK2_OVER_VIDf,TRUNK3_OVER_VIDf};
        int blk, bk = -1;

        if (port >= 0) {
            bk = SOC_PORT_BLOCK(unit, port);
        }

        soc_mem_lock(unit, MEM_VIDm);
        SOC_MEM_BLOCK_ITER(unit, MEM_VIDm, blk) {
            if ((port >= 0) && (bk != blk)) {
                continue;
            }
            rv = READ_MEM_VIDm(unit, blk, vid, &ve);
            if (rv >= 0) {
                soc_MEM_VIDm_field32_set(unit, &ve,
                                         tf[tid], enable ? 1: 0);
                rv = WRITE_MEM_VIDm(unit, blk, vid, &ve);
            }
        }
        soc_mem_unlock(unit, MEM_VIDm);
    }
#endif /* BCM_HERCULES15_SUPPORT */

#ifdef BCM_FIREBOLT_SUPPORT

    if ((SOC_IS_FBX(unit) || SOC_IS_RAVEN(unit)) && 
            (soc_feature(unit,soc_feature_hg_trunk_override)) ) { 

#ifdef BCM_TRIDENT_SUPPORT 
        if (soc_feature(unit, soc_feature_hg_trunk_override_profile)) {
            bcm_trunk_t hgtid;

            hgtid = tid - TRUNK_CNTL(unit).ngroups_fp;
            rv = _bcm_trident_trunk_override_vlan_set(unit, hgtid, vid, enable); 
        } else 
#endif /* BCM_TRIDENT_SUPPORT */
        {
            vlan_tab_entry_t ve;
            uint32 val, hgo_bit;

            soc_mem_lock(unit, VLAN_TABm);
            rv = READ_VLAN_TABm(unit, MEM_BLOCK_ANY, vid, &ve);
            if (rv >= 0) {
                val = soc_VLAN_TABm_field32_get(unit, &ve,
                        HIGIG_TRUNK_OVERRIDEf);
                hgo_bit = 1 << (tid - TRUNK_CNTL(unit).ngroups_fp);
                if (enable) {
                    val |= hgo_bit;
                } else {
                    val &= ~hgo_bit;
                }
                soc_VLAN_TABm_field32_set(unit, &ve,
                        HIGIG_TRUNK_OVERRIDEf, val);
                rv = WRITE_VLAN_TABm(unit, MEM_BLOCK_ALL, vid, &ve);
            }
            soc_mem_unlock(unit, VLAN_TABm);
        }
    }
#endif /* BCM_FIREBOLT_SUPPORT */

    return rv;
#else
    return BCM_E_UNAVAIL;
#endif  /* BCM_HERCULES15_SUPPORT || BCM_FIREBOLT_SUPPORT */
}

/*
 * Function:
 *      bcm_trunk_override_vlan_get
 * Description:
 *      Get the trunk override over VLAN.
 * Parameters:
 *      unit   - StrataSwitch PCI device unit number (driver internal).
 *      port   - Port number.
 *      tid    - Trunk id.
 *      vid    - VLAN id.
 *      enable - (OUT) TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_esw_trunk_override_vlan_get(int unit, bcm_port_t port,
                            bcm_trunk_t tid, bcm_vlan_t vid, int *enable)
{
#if defined(BCM_HERCULES15_SUPPORT) || defined(BCM_FIREBOLT_SUPPORT)
    int rv=BCM_E_NONE;

    if (BCM_GPORT_IS_SET(port)) {
        bcm_module_t    modid;
        bcm_trunk_t     tgid;
        int             id;

        BCM_IF_ERROR_RETURN(
            _bcm_esw_gport_resolve(unit, port, &modid, &port, &tgid, &id));
        if ((-1 != id) || (-1 != tgid)){
            return BCM_E_PARAM;
        }
    } else {
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
    }

    _TRUNK_OVERRIDE_CHECK(unit, port, tid);
    _VLAN_RANGE_CHECK(unit, vid);

#ifdef  BCM_HERCULES15_SUPPORT
    if (SOC_IS_HERCULES15(unit)) {
        mem_vid_entry_t      ve;
        soc_field_t tf[] = {TRUNK0_OVER_VIDf,TRUNK1_OVER_VIDf,
                            TRUNK2_OVER_VIDf,TRUNK3_OVER_VIDf};
        int blk;

        *enable = 0;

        blk = SOC_PORT_BLOCK(unit, port);
        rv = READ_MEM_VIDm(unit, blk, vid, &ve);
        if (rv >= 0) {
            *enable = soc_MEM_VIDm_field32_get(unit, &ve, tf[tid]);
        }
    }
#endif /* BCM_HERCULES15_SUPPORT */

#ifdef BCM_FIREBOLT_SUPPORT

    if ((SOC_IS_FBX(unit) || SOC_IS_RAVEN(unit)) && 
            (soc_feature(unit,soc_feature_hg_trunk_override)) ) {         

#ifdef BCM_TRIDENT_SUPPORT 
        if (soc_feature(unit, soc_feature_hg_trunk_override_profile)) {
            bcm_trunk_t hgtid;

            hgtid = tid - TRUNK_CNTL(unit).ngroups_fp;
            rv = _bcm_trident_trunk_override_vlan_get(unit, hgtid, vid, enable); 
        } else 
#endif /* BCM_TRIDENT_SUPPORT */
        {
            vlan_tab_entry_t ve;
            uint32 val, hgo_bit;

            *enable = 0;

            rv = READ_VLAN_TABm(unit, MEM_BLOCK_ANY, vid, &ve);
            if (rv >= 0) {
                val = soc_VLAN_TABm_field32_get(unit, &ve,
                        HIGIG_TRUNK_OVERRIDEf);
                hgo_bit = 1 << (tid - TRUNK_CNTL(unit).ngroups_fp);
                *enable = (val & hgo_bit) ? 1 : 0;
            }
        }
    }
#endif /* BCM_FIREBOLT_SUPPORT */

    return rv;
#else
    return BCM_E_UNAVAIL;
#endif  /* BCM_HERCULES15_SUPPORT || BCM_FIREBOLT_SUPPORT */
}

/*
 * Function:
 *      bcm_trunk_pool_set
 * Description:
 *      Set trunk pool table that contains the egress port number
 *      indexed by the hash value.
 * Parameters:
 *      unit    - StrataSwitch PCI device unit number (driver internal).
 *      port    - Port number, -1 to all ports.
 *      tid     - Trunk id.
 *      size    - Trunk pool size.
 *      weights - Weights for each port, all 0 means weighted fair.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_esw_trunk_pool_set(int unit, bcm_port_t port, bcm_trunk_t tid,
                   int size, const int weights[BCM_TRUNK_MAX_PORTCNT])
{
#ifdef  BCM_HERCULES15_SUPPORT
    uint32 val;
    int idx_min, idx_max, weighted, p, idx;
    mem_trunk_port_pool_entry_t trk_pool;
    trunk_private_t *t_info;
    bcm_trunk_add_info_t add_info;
    soc_field_t pf[] = {PORT_NO_0f, PORT_NO_1f, PORT_NO_2f, PORT_NO_3f};
    int token[BCM_TRUNK_MAX_PORTCNT], index;
    int i, reset=0, blk, rv=BCM_E_NONE, lp=0, isGport;

    if (!SOC_IS_HERCULES15(unit)) {
        return BCM_E_UNAVAIL;
    }

    if (port >= 0) {
        if (BCM_GPORT_IS_SET(port)) {
            bcm_trunk_t     tgid;
            bcm_module_t    modid;
            int             id;

            BCM_IF_ERROR_RETURN(
                _bcm_esw_gport_resolve(unit, port, &modid, &port, &tgid, &id));
            if ((-1 != id) || (-1 != tgid)){
                return BCM_E_PARAM;
            }
        } else {
            if (!SOC_PORT_VALID(unit, port) || !IS_HG_PORT(unit, port) ) {
                return BCM_E_PORT;
            }
        }
    }

    TRUNK_INIT(unit);
    TRUNK_CHECK(unit, tid);
    t_info = &TRUNK_INFO(unit, tid);
    if (t_info->tid == BCM_TRUNK_INVALID) {
        return BCM_E_NOT_FOUND;
    }

    idx_min = soc_mem_index_min(unit, MEM_TRUNK_PORT_POOLm);
    idx_max = soc_mem_index_max(unit, MEM_TRUNK_PORT_POOLm);
    if ((size < idx_min) || (size > idx_max)) {
        return BCM_E_PARAM;
    }
    if (size == 0) {
        index = size = idx_max;
    } else {
        index = size - 1;
    }

    BCM_IF_ERROR_RETURN(bcm_esw_trunk_get(unit, tid, &add_info));
    if (add_info.num_ports <= 0) {
        return BCM_E_NOT_FOUND;
    }

    BCM_IF_ERROR_RETURN
        (bcm_esw_switch_control_get(unit, bcmSwitchUseGport, &isGport));
    if (isGport) {
        BCM_IF_ERROR_RETURN(
            _bcm_esw_trunk_gport_resolve(unit, tid, &add_info));
    }

    for (i=0; i < add_info.num_ports; i++) {
         lp |= weights[add_info.tp[i]-1];
         if ((lp & ~0x3f) != 0) {
             return BCM_E_PARAM;
         }
    }
    weighted = (lp) ? 1 : 0;

    soc_mem_lock(unit, MEM_TRUNK_PORT_POOLm);
    PBMP_PORT_ITER(unit, p) {
        if ((port >= 0) && (p != port)) {
            continue;
        }

        if ((rv = READ_ING_HGTRUNKr(unit, p, tid, &val)) >= 0) {
            soc_reg_field_set(unit, ING_HGTRUNKr, &val,
                              TRUNK_POOL_SIZEf, size);
            rv = WRITE_ING_HGTRUNKr(unit, p, tid, val);
        }
        if (rv < 0) {
            break;
        }

        if (weighted) {
            for (i = 0; i < add_info.num_ports; i++) {
                 token[add_info.tp[i]-1] = weights[add_info.tp[i]-1];
            }
        }

        idx = index;
        blk = SOC_PORT_BLOCK(unit, p);

        while (idx >= 0) {
            if (weighted) {
                for (i = 0; i < add_info.num_ports; i++) {
                    if (token[add_info.tp[i]-1]) {
                        reset = 0;
                        break;
                    }
                    reset = 1;
                }
                if (reset) {
                    for (i = 0; i < add_info.num_ports; i++) {
                        token[add_info.tp[i]-1] = weights[add_info.tp[i]-1];
                    }
                }
            }
            for (i = 0; i < add_info.num_ports; i++) {
                if (idx < 0) break;
                sal_memset(&trk_pool, 0, sizeof(trk_pool));
                if (weighted) {
                    if (token[add_info.tp[i]-1]) {
                        if ((rv = READ_MEM_TRUNK_PORT_POOLm(unit, blk, index-idx, &trk_pool)) >= 0) {
                            soc_MEM_TRUNK_PORT_POOLm_field32_set(unit, &trk_pool, pf[tid], add_info.tp[i]);
                            rv = WRITE_MEM_TRUNK_PORT_POOLm(unit, blk, index-idx,  &trk_pool);
                        }
                        token[add_info.tp[i]-1]--;
                        idx--;
                    } else {
                      continue;
                    }
                } else {
                    if ((rv = READ_MEM_TRUNK_PORT_POOLm(unit, blk, index-idx, &trk_pool)) >= 0) {
                        soc_MEM_TRUNK_PORT_POOLm_field32_set(unit, &trk_pool, pf[tid], add_info.tp[i]);
                        rv = WRITE_MEM_TRUNK_PORT_POOLm(unit, blk, index-idx,  &trk_pool);
                    }
                    idx--;
                }
            }
        }
    }
    soc_mem_unlock(unit, MEM_TRUNK_PORT_POOLm);

    return rv;
#else
    return BCM_E_UNAVAIL;
#endif  /* BCM_HERCULES15_SUPPORT */
}

/*
 * Function:
 *      bcm_trunk_pool_get
 * Description:
 *      Get trunk pool table that contains the egress port number
 *      indexed by the hash value.
 * Parameters:
 *      unit    - StrataSwitch PCI device unit number (driver internal).
 *      port    - Port number.
 *      tid     - Trunk id.
 *      size    - (OUT) Trunk pool size.
 *      weights - (OUT) Weights (total count) for each port.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_esw_trunk_pool_get(int unit, bcm_port_t port, bcm_trunk_t tid,
                   int *size, int weights[BCM_TRUNK_MAX_PORTCNT])
{
#ifdef  BCM_HERCULES15_SUPPORT
    uint32 val;
    mem_trunk_port_pool_entry_t trk_pool;
    trunk_private_t *t_info;
    bcm_trunk_add_info_t add_info;
    soc_field_t pf[] = {PORT_NO_0f, PORT_NO_1f, PORT_NO_2f, PORT_NO_3f};
    int i,pool, idx, blk, pool_port, isGport, rv=BCM_E_NONE;

    if (!SOC_IS_HERCULES15(unit)) {
        return BCM_E_UNAVAIL;
    }

    if (BCM_GPORT_IS_SET(port)) {
        bcm_trunk_t     tgid;
        bcm_module_t    modid;
        int             id;

        BCM_IF_ERROR_RETURN(
            _bcm_esw_gport_resolve(unit, port, &modid, &port, &tgid, &id));
        if ((-1 != id) || (-1 != tgid)){
            return BCM_E_PARAM;
        }
    } else {
        if (!SOC_PORT_VALID(unit, port) || !IS_HG_PORT(unit, port) ) {
            return BCM_E_PORT;
        }
    }

    TRUNK_INIT(unit);
    TRUNK_CHECK(unit, tid);
    t_info = &TRUNK_INFO(unit, tid);
    if (t_info->tid == BCM_TRUNK_INVALID) {
        return BCM_E_NOT_FOUND;
    }

    *size = 0;
    for (idx=0; idx < BCM_TRUNK_MAX_PORTCNT; idx++) {
         weights[idx] = 0;
    }

    SOC_IF_ERROR_RETURN(READ_ING_HGTRUNKr(unit, port, tid, &val));
    *size = soc_reg_field_get(unit, ING_HGTRUNKr, val, TRUNK_POOL_SIZEf);
    if (*size == 0) { /* HW meaning, we never program 0 above in _set */
        *size = 1;
    }

    BCM_IF_ERROR_RETURN(bcm_esw_trunk_get(unit, tid, &add_info));

    BCM_IF_ERROR_RETURN
        (bcm_esw_switch_control_get(unit, bcmSwitchUseGport, &isGport));
    if (isGport) {
        BCM_IF_ERROR_RETURN(
            _bcm_esw_trunk_gport_resolve(unit, tid, &add_info));
    }

    blk = SOC_PORT_BLOCK(unit, port);
    pool = *size;
    for (idx = 0; idx < pool; idx++) {
         if ((rv = READ_MEM_TRUNK_PORT_POOLm(unit, blk, idx, &trk_pool)) >= 0) {
              pool_port = soc_MEM_TRUNK_PORT_POOLm_field32_get(unit, &trk_pool, pf[tid]);
              for (i=0; i < add_info.num_ports; i++) {
                   if (pool_port == add_info.tp[i]) {
                       weights[add_info.tp[i]-1]++;
                       break;
                   }
              }
         }
    }

    return rv;
#else
    return BCM_E_UNAVAIL;
#endif  /* BCM_HERCULES15_SUPPORT */
}

int _bcm_nuc_tpbm_get(int unit,
                      int num_ports,
                      bcm_module_t tm[BCM_TRUNK_MAX_PORTCNT],
                      uint32 *nuc_tpbm)
{
    int i, mod = -1;
    uint32 mod_type;
    int all_equal = 1;
    uint32 xgs12_tpbm = 0;
    uint32 xgs3_tpbm = 0;
    uint32 unknown_tpbm = 0;

    *nuc_tpbm = 0x1;
    for (i = 0; i < num_ports; i++) {
        if (i == 0) {
            mod = tm[i];
        } else if (mod != tm[i]) {
            all_equal = 0;
        }
        BCM_IF_ERROR_RETURN
            (_bcm_switch_module_type_get(unit, tm[i], &mod_type));
        switch(mod_type) {
            case BCM_SWITCH_MODULE_XGS1   :
            case BCM_SWITCH_MODULE_XGS2   : xgs12_tpbm |= (1 << i); break;
            case BCM_SWITCH_MODULE_XGS3   : xgs3_tpbm |= (1 << i); break;
            case BCM_SWITCH_MODULE_UNKNOWN:
            default                       : unknown_tpbm |= (1 << i); break;
        }
    }
    if (all_equal) {
        *nuc_tpbm = ((0x1 << num_ports) - 1);
    } else if (unknown_tpbm || xgs12_tpbm) {
        *nuc_tpbm = 0x1;
    } else if (xgs3_tpbm) {
        *nuc_tpbm = xgs3_tpbm;
    }

    return(BCM_E_NONE);
}

/*
 * Function:
 *      bcm_trunk_find
 * Description:
 *      Get trunk id that contains the given system port
 * Parameters:
 *      unit    - StrataSwitch PCI device unit number (driver internal)
 *      modid   - Module ID
 *      port    - Port number
 *      tid     - (OUT) Trunk id
 * Returns:
 *      BCM_E_xxxx
 */
int
bcm_esw_trunk_find(int unit, bcm_module_t modid, bcm_port_t port,
                   bcm_trunk_t *tid)
{
    int          rv = BCM_E_NONE;
    bcm_module_t hw_mod;
    bcm_port_t   hw_port;
    int          tgid, id;
    int          isLocal;

    TRUNK_INIT(unit);

    if (tid == NULL) {
        return BCM_E_PARAM;
    }

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_esw_gport_resolve(unit, port, &hw_mod, &hw_port, &tgid, &id));
        if ((-1 != id) || (-1 != tgid)){
            return BCM_E_PORT;
        }
    } else {
        PORT_DUALMODID_VALID(unit, port);
        BCM_IF_ERROR_RETURN
            (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                    modid, port,
                                    &hw_mod, &hw_port));

        if (!SOC_MODID_ADDRESSABLE(unit, hw_mod)) {
            return BCM_E_BADID;
        }

        if (!SOC_PORT_ADDRESSABLE(unit, hw_port)) {
            return BCM_E_PORT;
        }
    }

    BCM_IF_ERROR_RETURN(_bcm_esw_modid_is_local(unit, hw_mod, &isLocal));

#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (SOC_IS_XGS3_SWITCH(unit)) {
#ifdef BCM_TRIDENT_SUPPORT
        if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
            if (isLocal && IS_ST_PORT(unit, hw_port)) {
                return(_bcm_trident_trunk_fabric_find(unit, hw_port, tid));
            } else {
                return(_bcm_trident_trunk_get_port_property(unit, hw_mod, hw_port, tid));
            } 
        } else 
#endif /* BCM_TRIDENT_SUPPORT */
        {
            if (isLocal && IS_ST_PORT(unit, hw_port)) {
                return(_bcm_xgs3_trunk_fabric_find(unit, hw_port, tid));
            } else {
                return(_bcm_xgs3_trunk_get_port_property(unit, hw_mod, hw_port, tid)); 
            }
        }
    } else
#endif /* BCM_XGS3_SWITCH_SUPPORT */
    {
        int idx = 0, isGport;
        bcm_trunk_add_info_t t_data;

        for (idx = 0; idx < TRUNK_NUM_GROUPS(unit); idx++) {
             if (TRUNK_INFO(unit, idx).tid == BCM_TRUNK_INVALID) {
                continue;
             }
             
             sal_memset(&t_data, 0, sizeof(bcm_trunk_add_info_t));
             rv = bcm_esw_trunk_get(unit, idx, &t_data);

             BCM_IF_ERROR_RETURN
                 (bcm_esw_switch_control_get(unit, bcmSwitchUseGport, &isGport));
             if (isGport) {
                 BCM_IF_ERROR_RETURN(
                     _bcm_esw_trunk_gport_resolve(unit, idx, &t_data));
             }

             if (BCM_SUCCESS(rv)) {
                 int i = 0;

                 for (i = 0; i < t_data.num_ports; i++) {

                     if (isLocal && IS_ST_PORT(unit, hw_port)) {
                         if (TRUNK_FABRIC_TID(unit, idx)) {
                             if (t_data.tp[i] == hw_port) {
                                 *tid = idx;
                                 return BCM_E_NONE;
                             }
                         }
                     } else {
                         if (t_data.tm[i] == hw_mod && t_data.tp[i] == hw_port) {
                             *tid = idx;
                             return BCM_E_NONE;
                         }
                     }
                 }
             }
        }

        return BCM_E_NOT_FOUND;
    }
}

/*
 * Function:
 *      bcm_esw_trunk_failover_set
 * Purpose:
 *      Assign the failover port list for a specific trunk port.
 * Parameters:
 *      unit - (IN) Unit number.
 *      tid - (IN) Trunk id.
 *      failport - (IN) Port in trunk for which to specify failover port list.
 *      psc - (IN) Port selection criteria for failover port list.
 *      flags - (IN) BCM_TRUNK_FLAG_FAILOVER_xxx.
 *      count - (IN) Number of ports in failover port list.
 *      fail_to_array - (IN) Failover port list.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_trunk_failover_set(int unit, bcm_trunk_t tid, bcm_gport_t failport, 
                           int psc, uint32 flags, int count, 
                           bcm_gport_t *fail_to_array)
{
#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
    trunk_private_t *t_info;
    int rv, id, fabric_trunk;
    bcm_port_t port, ftp[BCM_TRUNK_MAX_PORTCNT];
    bcm_module_t modid, ftm[BCM_TRUNK_MAX_PORTCNT];
    bcm_trunk_t tgid;

    TRUNK_INIT(unit);

    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
        return (BCM_E_NOT_FOUND);
    }

    fabric_trunk = TRUNK_FABRIC_TID(unit, tid);

    if (fabric_trunk) {
        if (!soc_feature(unit, soc_feature_hg_trunk_failover)) {
            return BCM_E_UNAVAIL;
        }
        if (BCM_GPORT_IS_SET(failport)) {
            BCM_IF_ERROR_RETURN
                (bcm_esw_port_local_get(unit, failport, &port));
        } else {
            port = failport;
        }
        modid = -1;
    } else {
        if (!soc_feature(unit, soc_feature_port_lag_failover)) {
            return BCM_E_UNAVAIL;
        }
        BCM_IF_ERROR_RETURN
            (_bcm_esw_gport_resolve(unit, failport, &modid, &port, &tgid, &id));
        if ((-1 != tgid) || (-1 != id)) {
            return BCM_E_PARAM;
        }
    }

    if (psc <= 0) {
	psc = BCM_TRUNK_PSC_DEFAULT;
    }

    /* Failover options:
     * 1) Specify plan with a flag
     * 2) Specify plan with count > 0 and fail_to_array listing ports
     * 3) Disable failover with count == 0
     */
    if ((flags == 0) && ((count < 0) ||
                         ((count > 0) && (fail_to_array == NULL)))) {
        return BCM_E_PARAM;
    }

    if (count > 0) {
        if ((TRUNK_FP_TID(unit, tid) &&
             (count > TRUNK_CNTL(unit).nports_fp)) ||
            (TRUNK_FABRIC_TID(unit, tid) &&
             (count > TRUNK_CNTL(unit).nports_hg)) ) {
            return (BCM_E_PARAM);
        } else {
            BCM_IF_ERROR_RETURN
                (_bcm_esw_trunk_gport_array_resolve(unit, fabric_trunk,
                                                    count, fail_to_array,
                                                    ftp, ftm));
        }
    }

    /* Offset of trunk type */
    tgid = tid - (fabric_trunk ? TRUNK_CNTL(unit).ngroups_fp : 0);

    TRUNK_LOCK(unit);
#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
        rv = _bcm_trident_trunk_hwfailover_set(unit, tgid, fabric_trunk,
                port, modid, psc, flags, count, ftp, ftm);
    } else 
#endif /* BCM_TRIDENT_SUPPORT */
    {
        rv = _bcm_xgs3_trunk_hwfailover_set(unit, tgid, fabric_trunk,
                port, modid, psc, flags, count, ftp, ftm);
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    SOC_CONTROL_LOCK(unit);
    SOC_CONTROL(unit)->scache_dirty = 1;
    SOC_CONTROL_UNLOCK(unit);
#endif

    TRUNK_UNLOCK(unit);

    return rv;
#else /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */
    return BCM_E_UNAVAIL; 
#endif
}

/*
 * Function:
 *      bcm_esw_trunk_failover_get
 * Purpose:
 *      Retrieve the failover port list for a specific trunk port.
 * Parameters:
 *      unit - (IN) Unit number.
 *      tid - (IN) Trunk id.
 *      failport - (IN) Port in trunk for which to retrieve failover port list.
 *      psc - (OUT) Port selection criteria for failover port list.
 *      flags - (OUT) BCM_TRUNK_FLAG_FAILOVER_xxx.
 *      array_size - (IN) Maximum number of ports in provided failover port list.
 *      fail_to_array - (OUT) Failover port list.
 *      array_count - (OUT) Number of ports in returned failover port list.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_trunk_failover_get(int unit, bcm_trunk_t tid, bcm_gport_t failport, 
                           int *psc, uint32 *flags, int array_size, 
                           bcm_gport_t *fail_to_array, int *array_count)
{
#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
    trunk_private_t *t_info;
    int rv, id, fabric_trunk;
    bcm_port_t port, ftp[BCM_TRUNK_MAX_PORTCNT];
    bcm_module_t modid, ftm[BCM_TRUNK_MAX_PORTCNT];
    bcm_trunk_t tgid;

    TRUNK_INIT(unit);

    if (!soc_feature(unit, soc_feature_hg_trunk_failover)) {
        return BCM_E_UNAVAIL;
    }

    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
        return (BCM_E_NOT_FOUND);
    }

    fabric_trunk = TRUNK_FABRIC_TID(unit, tid);

    if (fabric_trunk) {
        if (!soc_feature(unit, soc_feature_hg_trunk_failover)) {
            return BCM_E_UNAVAIL;
        }
        if (BCM_GPORT_IS_SET(failport)) {
            BCM_IF_ERROR_RETURN
                (bcm_esw_port_local_get(unit, failport, &port));
        } else {
            port = failport;
        }
        modid = -1;
    } else {
        if (!soc_feature(unit, soc_feature_port_lag_failover)) {
            return BCM_E_UNAVAIL;
        }
        BCM_IF_ERROR_RETURN
            (_bcm_esw_gport_resolve(unit, failport, &modid, &port, &tgid, &id));
        if ((-1 != tgid) || (-1 != id)) {
            return BCM_E_PARAM;
        }
    }

    if ((psc == NULL) || (flags == NULL) || (array_size < 0) ||
        (fail_to_array == NULL) || (array_count == NULL)) {
        return BCM_E_PARAM;
    }

    /* Offset of trunk type */
    tgid = tid - (fabric_trunk ? TRUNK_CNTL(unit).ngroups_fp : 0);

    TRUNK_LOCK(unit);
#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
        rv = _bcm_trident_trunk_hwfailover_get(unit, tgid, fabric_trunk,
                port, modid, psc, flags, array_size, ftp, ftm, array_count);
    } else
#endif /* BCM_TRIDENT_SUPPORT */
    {
        rv = _bcm_xgs3_trunk_hwfailover_get(unit, tgid, fabric_trunk,
                port, modid, psc, flags, array_size, ftp, ftm, array_count);
    }
    TRUNK_UNLOCK(unit);

    BCM_IF_ERROR_RETURN(rv);

    return _bcm_esw_trunk_gport_construct(unit, fabric_trunk, *array_count,
                                          ftp, ftm, fail_to_array);
#else /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */
    return BCM_E_UNAVAIL; 
#endif
}

/*
 * Function:
 *      bcm_esw_trunk_member_set
 * Purpose:
 *      Assign a set of members to a trunk group.
 * Parameters:
 *      unit - (IN) Unit number.
 *      tid  - (IN) Trunk id.
 *      member_count - (IN) Number of trunk members in member_array.
 *      member_array - (IN) Array of trunk member structures.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_trunk_member_set(
    int unit, 
    bcm_trunk_t tid, 
    int member_count, 
    bcm_trunk_member_t *member_array)
{
    int             rv = BCM_E_UNAVAIL;
    trunk_private_t *t_info;

    TRUNK_INIT(unit);
    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
        return BCM_E_NOT_FOUND;
    }

    if ((member_count < 1) && 
            (NULL != member_array)) {
        return BCM_E_PARAM;
    }

    if ((TRUNK_FP_TID(unit, tid) &&
         (member_count > TRUNK_CNTL(unit).nports_fp)) ||
        (TRUNK_FABRIC_TID(unit, tid) &&
         (member_count > TRUNK_CNTL(unit).nports_hg))) {
        return BCM_E_PARAM;
    }

    TRUNK_LOCK(unit);

#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
        if (0 == member_count) {
            if (t_info->in_use) {
                rv = mbcm_driver[unit]->mbcm_trunk_destroy(unit, tid, t_info);
            } else {
                rv = BCM_E_NONE;
            }
        } else {
            rv = bcm_trident_trunk_member_set(unit, tid, t_info,
                    member_count, member_array);
            if (BCM_SUCCESS(rv)) {
                t_info->in_use = TRUE;
            }
        }
    }
#endif /* BCM_TRIDENT_SUPPORT */

    TRUNK_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      bcm_esw_trunk_member_get
 * Purpose:
 *      Retrieve members of a trunk group. If member_max is 0,
 *      the number of member ports in the given trunk group
 *      is returned in member_count.
 * Parameters:
 *      unit - (IN) Unit number.
 *      tid  - (IN) Trunk id.
 *      member_max   - (IN) Size of member_array.
 *      member_array - (OUT) Array of trunk member structures.
 *      member_count - (OUT) Number of trunk members returned.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_trunk_member_get(
    int unit, 
    bcm_trunk_t tid, 
    int member_max, 
    bcm_trunk_member_t *member_array,
    int *member_count) 
{
    int rv = BCM_E_UNAVAIL;
    trunk_private_t *t_info;
    bcm_trunk_add_info_t t_data;
    int i;

    TRUNK_INIT(unit);
    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
        return (BCM_E_NOT_FOUND);
    }

    if ((member_max > 0) && (NULL == member_array)) {
        return BCM_E_PARAM;
    }

    if (NULL == member_count) {
        return BCM_E_PARAM;
    }

    TRUNK_LOCK(unit);

#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
        rv = bcm_trident_trunk_member_get(unit, tid, t_info,
                member_max, member_array, member_count);
    } else 
#endif /* BCM_TRIDENT_SUPPORT */
    {
        bcm_trunk_add_info_t_init(&t_data);
        rv = mbcm_driver[unit]->mbcm_trunk_get(unit, tid, &t_data, t_info);
        if (BCM_FAILURE(rv)) {
            TRUNK_UNLOCK(unit);
            return rv;
        }

        *member_count = t_data.num_ports;

        if (member_max > 0) {
            rv = _bcm_esw_trunk_gport_construct(unit,
                    TRUNK_FABRIC_TID(unit, tid),
                    t_data.num_ports,
                    t_data.tp, t_data.tm,
                    t_data.tp);
            if (BCM_FAILURE(rv)) {
                TRUNK_UNLOCK(unit);
                return rv;
            }
            for (i = 0; i < *member_count; i++) {
                member_array[i].flags = t_data.member_flags[i];
                member_array[i].gport = t_data.tp[i];
            }
        }
    }

    TRUNK_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      bcm_esw_trunk_member_add
 * Purpose:
 *      Add a member to a trunk group.
 * Parameters:
 *      unit   - (IN) Unit number.
 *      tid    - (IN) Trunk id.
 *      member - (IN) Pointer to a trunk member structure.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_trunk_member_add(
    int unit, 
    bcm_trunk_t tid, 
    bcm_trunk_member_t *member)
{
    trunk_private_t *t_info;

    TRUNK_INIT(unit);
    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
        return BCM_E_NOT_FOUND;
    }

#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
        int                rv;
        int                member_count;
        int                array_count;
        bcm_trunk_member_t *member_array;

        /* Get number of trunk group's existing member ports */
        BCM_IF_ERROR_RETURN
            (bcm_esw_trunk_member_get(unit, tid, 0, NULL, &member_count));

        if (member_count == 0) {
            return bcm_esw_trunk_member_set(unit, tid, 1, member);
        } else {
            /* Allocate member array to accomodate existing members and
             * the new member to be added.
             */
            array_count = member_count + 1;
            member_array = sal_alloc(sizeof(bcm_trunk_member_t) * array_count,
                    "trunk_member_array");
            if (member_array == NULL) {
                return BCM_E_MEMORY;
            }
            sal_memset(member_array, 0,
                    sizeof(bcm_trunk_member_t) * array_count);

            /* Retrieve existing member ports */
            rv = bcm_esw_trunk_member_get(unit, tid, array_count,
                                          member_array, &member_count);
            if (BCM_FAILURE(rv)) {
                sal_free(member_array);
                return rv;
            }

            if (member_count < array_count) {
                /* Ths should always be true. Set member to
                 * the last element of the array.
                 */
                member_array[member_count].flags = member->flags;
                member_array[member_count].gport = member->gport;
            } else {
                sal_free(member_array);
                return BCM_E_INTERNAL;
            }

            rv = bcm_esw_trunk_member_set(unit, tid, array_count,
                                          member_array);
            sal_free(member_array);
            return rv;
        }
    }
#endif /* BCM_TRIDENT_SUPPORT */

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_esw_trunk_member_delete
 * Purpose:
 *      Remove a member from a trunk group.
 * Parameters:
 *      unit   - (IN) Unit number.
 *      tid    - (IN) Trunk id.
 *      member - (IN) Pointer to a trunk member structure.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_trunk_member_delete(
    int unit, 
    bcm_trunk_t tid, 
    bcm_trunk_member_t *member)
{
    trunk_private_t *t_info;
    bcm_module_t    modid;
    bcm_port_t      port;
    bcm_trunk_t     tgid;
    int             id;

    TRUNK_INIT(unit);
    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
        return BCM_E_NOT_FOUND;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_esw_gport_resolve(unit, member->gport,
                                &modid, &port, &tgid, &id));
    if ((-1 != tgid) || (-1 != id)) {
        return BCM_E_PARAM;
    }

#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
        int                rv, i, k;
        int                member_count;
        int                array_count;
        bcm_trunk_member_t *member_array;
        bcm_module_t       member_modid;
        bcm_port_t         member_port;
        bcm_trunk_t        member_tgid;
        int                member_id;

        /* Get number of trunk group's existing member ports */
        BCM_IF_ERROR_RETURN
            (bcm_esw_trunk_member_get(unit, tid, 0, NULL, &member_count));

        if (member_count == 0) {
            return BCM_E_NOT_FOUND;
        } else {
            /* Allocate member array to get existing members */
            array_count = member_count;
            member_array = sal_alloc(sizeof(bcm_trunk_member_t) * array_count,
                    "trunk_member_array");
            if (member_array == NULL) {
                return BCM_E_MEMORY;
            }
            sal_memset(member_array, 0,
                    sizeof(bcm_trunk_member_t) * array_count);

            /* Retrieve existing member ports */
            rv = bcm_esw_trunk_member_get(unit, tid, array_count,
                                          member_array, &member_count);
            if (BCM_FAILURE(rv)) {
                sal_free(member_array);
                return rv;
            }

            /* Find the index of the given member in the array */
            for (i = 0; i < member_count; i++) {
                rv = _bcm_esw_gport_resolve(unit, member_array[i].gport,
                                            &member_modid, &member_port,
                                            &member_tgid, &member_id);
                if (BCM_FAILURE(rv) ||
                    (-1 != member_tgid) ||
                    (-1 != member_id)) {
                    sal_free(member_array);
                    return BCM_E_INTERNAL;
                }
                if ((member_modid == modid) && (member_port == port)) {
                    break;
                }
            }

            if (i == member_count) {
                sal_free(member_array);
                return BCM_E_NOT_FOUND;
            }

            /* Delete given member from array, and shift the remaining
             * elements of the array.
             * */
            for (k = i; k < member_count - 1; k++) {
                member_array[k].flags = member_array[k+1].flags;
                member_array[k].gport = member_array[k+1].gport;
            }
            array_count--;

            if (0 == array_count) {
                rv = bcm_esw_trunk_member_set(unit, tid, array_count,
                        NULL);
            } else {
                rv = bcm_esw_trunk_member_set(unit, tid, array_count,
                        member_array);
            }

            sal_free(member_array);
            return rv;
        }
    }
#endif /* BCM_TRIDENT_SUPPORT */

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_esw_trunk_member_delete_all
 * Purpose:
 *      Remove all members of a trunk group.
 * Parameters:
 *      unit   - (IN) Unit number.
 *      tid    - (IN) Trunk id.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_trunk_member_delete_all(
    int unit, 
    bcm_trunk_t tid) 
{
    return bcm_esw_trunk_member_set(unit, tid, 0, NULL);
}

/*
 * Function:
 *      bcm_esw_trunk_member_traverse
 * Purpose:
 *      Traverse through all members of a trunk group and
 *      run the callback for each member.
 * Parameters:
 *      unit - (IN) Unit number.
 *      tid  - (IN) Trunk id.
 *      cb   - (IN) Callback function.
 *      user_data - (IN) Pointer to user data.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_trunk_member_traverse(
    int unit, 
    bcm_trunk_t tid, 
    bcm_trunk_member_traverse_cb cb, 
    void *user_data)
{
    trunk_private_t *t_info;

    TRUNK_INIT(unit);
    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
        return (BCM_E_NOT_FOUND);
    }

    if (NULL == cb) {
        return (BCM_E_PARAM);
    }

#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
        int                rv, i;
        int                member_count;
        int                array_count;
        bcm_trunk_member_t *member_array;

        /* Get number of trunk group's member ports */
        BCM_IF_ERROR_RETURN
            (bcm_esw_trunk_member_get(unit, tid, 0, NULL, &member_count));

        if (member_count == 0) {
            return BCM_E_NOT_FOUND;
        } else {
            /* Allocate member array to get members */
            array_count = member_count;
            member_array = sal_alloc(sizeof(bcm_trunk_member_t) * array_count,
                    "trunk_member_array");
            if (member_array == NULL) {
                return BCM_E_MEMORY;
            }
            sal_memset(member_array, 0,
                    sizeof(bcm_trunk_member_t) * array_count);

            /* Retrieve existing member ports */
            rv = bcm_esw_trunk_member_get(unit, tid, array_count,
                                          member_array, &member_count);
            if (BCM_FAILURE(rv)) {
                sal_free(member_array);
                return rv;
            }
            
            for (i = 0; i < member_count; i++) {
                rv = (*cb)(unit, member_array + i, user_data);
                if (BCM_FAILURE(rv)) {
                    sal_free(member_array);
                    return rv;
                }
            }

            sal_free(member_array);
            return BCM_E_NONE;
        }
    }
#endif /* BCM_TRIDENT_SUPPORT */

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      _bcm_esw_trunk_local_members_get
 * Purpose:
 *      Get local member ports of a trunk group.
 * Parameters:
 *      unit       - (IN) Device Number
 *      trunk_id   - (IN) Trunk Identifier
 *      local_member_max   - (OUT) Size of local_member_array
 *      local_member_array - (OUT) Array of local trunk members 
 *      local_member_count - (OUT) Number of local trunk members
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      If local_member_max = 0 and local_member_array == NULL,
 *      the number of local trunk members will still be returned in
 *      local_member_count. 
 */
int 
_bcm_esw_trunk_local_members_get(int unit, bcm_trunk_t trunk_id, 
                                 int local_member_max, 
                                 bcm_port_t *local_member_array, 
                                 int *local_member_count)
{
    int rv;
    int member_count;
    bcm_trunk_member_t *member_array;
    int i;
    bcm_module_t mod_out;
    bcm_port_t port_out;
    bcm_trunk_t tgid_out;
    int id_out;
    int modid_local;

    if (local_member_max < 0) {
        return BCM_E_PARAM;
    }

    if ((local_member_max == 0) && (NULL != local_member_array)) {
        return BCM_E_PARAM;
    }

    if ((local_member_max > 0) && (NULL == local_member_array)) {
        return BCM_E_PARAM;
    }

    if (NULL == local_member_count) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_bcm_trunk_id_validate(unit, trunk_id));

    /* First, get the number of trunk members. */
    rv = bcm_esw_trunk_member_get(unit, trunk_id, 0, NULL, &member_count);
    if (BCM_FAILURE(rv)) {
        return BCM_E_PORT;
    }

    member_array = sal_alloc(sizeof(bcm_trunk_member_t) * member_count,
            "trunk member array");
    if (member_array == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(member_array, 0, sizeof(bcm_trunk_member_t) * member_count);

    rv = bcm_esw_trunk_member_get(unit, trunk_id, member_count,
            member_array, &member_count);
    if (BCM_FAILURE(rv)) {
        sal_free(member_array);
        return BCM_E_PORT;
    }

    *local_member_count = 0;

    for (i = 0; i < member_count; i++) {
        rv = _bcm_esw_gport_resolve(unit, member_array[i].gport,
                &mod_out, &port_out, &tgid_out, &id_out);
        if (BCM_FAILURE(rv)) {
            sal_free(member_array);
            return rv;
        }

        rv = _bcm_esw_modid_is_local(unit, mod_out, &modid_local);
        if (BCM_FAILURE(rv)) {
            sal_free(member_array);
            return rv;
        }

        if (modid_local) {
            /* Convert system port to physical port */
            if (soc_feature(unit, soc_feature_sysport_remap)) {
                BCM_XLATE_SYSPORT_S2P(unit, &port_out);
            }

            if (NULL != local_member_array) {
                local_member_array[*local_member_count] = port_out;
            }
            (*local_member_count)++;
            if (*local_member_count == local_member_max) {
                break;
            }
        }
    }

    sal_free(member_array);

    return BCM_E_NONE;
}

#ifdef BCM_WARM_BOOT_SUPPORT

#define TRUNK_MIN_FABRIC_TID(unit) \
    (TRUNK_CNTL(unit).ngroups_hg ? TRUNK_CNTL(unit).ngroups_fp : -1)
#define TRUNK_MAX_FABRIC_TID(unit) \
    ( TRUNK_CNTL(unit).ngroups_hg ? \
     (TRUNK_CNTL(unit).ngroups_fp + TRUNK_CNTL(unit).ngroups_hg - 1): -1)

/*
 * _bcm_trunk_lag_reinit
 *
 * Restores sw state for front pannel trunks
 * Note: Application must register modid-port mapping function
 *       using bcm_stk_modmap_register before calling this
 */
STATIC int
_bcm_trunk_lag_reinit(int unit)
{
#if defined(BCM_TRIDENT_SUPPORT)
    if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
        return (_bcm_trident_trunk_lag_reinit(unit, TRUNK_CNTL(unit).ngroups_fp,
                                   TRUNK_CNTL(unit).t_info));
    } else 
#endif /* BCM_TRIDENT_SUPPORT */
#if defined(BCM_FIREBOLT_SUPPORT) || defined(BCM_EASYRIDER_SUPPORT)
    if (SOC_IS_FBX(unit) || SOC_IS_EASYRIDER(unit)) {
        return (_xgs3_trunk_reinit(unit, TRUNK_CNTL(unit).ngroups_fp,
                                   TRUNK_CNTL(unit).t_info));
    }
#endif /* BCM_FIREBOLT_SUPPORT || BCM_EASYRIDER_SUPPORT */
  	 
    return BCM_E_NONE;
}

/*
 * _bcm_trunk_fabric_reinit
 *
 * Restores sw state for fabric trunks
 */
STATIC int
_bcm_trunk_fabric_reinit(int unit)
{
#if defined(BCM_TRIDENT_SUPPORT)
    if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
        int min_tid = TRUNK_MIN_FABRIC_TID(unit);

        return (_bcm_trident_trunk_fabric_reinit(unit, 
                                          TRUNK_MIN_FABRIC_TID(unit),
                                          TRUNK_MAX_FABRIC_TID(unit),
                                          &TRUNK_INFO(unit,min_tid)));
    } else 
#endif /* BCM_TRIDENT_SUPPORT */
#if defined(BCM_FIREBOLT_SUPPORT)
    if (SOC_IS_FBX(unit) && !SOC_IS_RAPTOR(unit)) {
        int min_tid = TRUNK_MIN_FABRIC_TID(unit);

        return (_xgs3_trunk_fabric_reinit(unit, 
                                          TRUNK_MIN_FABRIC_TID(unit),
                                          TRUNK_MAX_FABRIC_TID(unit),
                                          &TRUNK_INFO(unit,min_tid)));
    }
#endif /* BCM_FIREBOLT_SUPPORT */

    return BCM_E_NONE;
}

/*
 * _bcm_trunk_reinit
 *
 * Recovers trunk state from hardware (Reload)
 */
STATIC int
_bcm_trunk_reinit(int unit)
{
    int                 rv;
    bcm_trunk_t         tid;
    trunk_private_t     *t_info;
    soc_scache_handle_t scache_handle;
    uint8               *trunk_scache;
    uint16              recovered_ver;
    uint32              ngroups;
    uint8               flags;
#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
    bcm_trunk_t         hgtid;
    int                 tix, idx;
#endif /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */
    uint32              additional_scache_size;
 	 
    /*
     * Trunk state initialization
     */
    t_info = TRUNK_CNTL(unit).t_info;

    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_TRUNK, 0);
    rv = _bcm_esw_scache_ptr_get(unit, scache_handle, FALSE,
                                 0, &trunk_scache,  
                                 BCM_WB_DEFAULT_VERSION, &recovered_ver);

    if (BCM_E_NOT_FOUND == rv) {
        trunk_scache = NULL;
    } else if (BCM_FAILURE(rv)) {
        return rv;
    } else {
        /* Validate trunk info matches */
        sal_memcpy(&ngroups, trunk_scache, sizeof(ngroups));
        if (ngroups != TRUNK_CNTL(unit).ngroups_fp) {
            return BCM_E_CONFIG;
        }
        trunk_scache += sizeof(ngroups);
        
        sal_memcpy(&ngroups, trunk_scache, sizeof(ngroups));
        if (ngroups != TRUNK_CNTL(unit).ngroups_hg) {
            return BCM_E_CONFIG;
        }
        trunk_scache += sizeof(ngroups);

        sal_memset(TRUNK_CNTL(unit).trunk_bmp_ptr, 0,
                   TRUNK_CNTL(unit).trunk_bmp_alloc);
        sal_memcpy(TRUNK_CNTL(unit).trunk_bmp_ptr, trunk_scache,
                   TRUNK_CNTL(unit).trunk_bmp_alloc);

        for (tid = 0; tid < TRUNK_NUM_GROUPS(unit); tid++) {
            if (SHR_BITGET(TRUNK_CNTL(unit).trunk_bmp_ptr, tid)) {
                t_info->tid = tid;
            }
            t_info++;
        }
        trunk_scache += TRUNK_CNTL(unit).trunk_bmp_alloc;

#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
        if (TRUNK_CNTL(unit).hw_failover_alloc) {
            idx = 0;

            sal_memcpy(TRUNK_CNTL(unit).hw_failover_flags, trunk_scache,
                       TRUNK_CNTL(unit).hw_failover_alloc);
            trunk_scache += TRUNK_CNTL(unit).hw_failover_alloc;

            if (soc_feature(unit, soc_feature_port_lag_failover)) {
                /* FP trunk failover flags */
                for (tid = 0; tid < TRUNK_CNTL(unit).ngroups_fp; tid++) {
                    TRUNK_INFO(unit, tid).flags =
                        TRUNK_CNTL(unit).hw_failover_flags[idx++];
#ifdef BCM_TRIDENT_SUPPORT
                    if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
                        uint16 num_ports;
                        num_ports = TRUNK_CNTL(unit).hw_failover_flags[idx++] << 8;
                        num_ports += TRUNK_CNTL(unit).hw_failover_flags[idx++];
                        _bcm_trident_hw_failover_num_ports_set(unit, tid,
                                FALSE, num_ports);
                        for (tix = 0; tix < num_ports; tix++) {
                            BCM_IF_ERROR_RETURN
                                (_bcm_trident_hw_failover_flags_set(unit, tid,
                                    tix, FALSE, 
                                    TRUNK_CNTL(unit).hw_failover_flags[idx++]));
                        }

                    } else
#endif /* BCM_TRIDENT_SUPPORT */
                    {
                        for (tix = 0; tix < TRUNK_CNTL(unit).nports_fp; tix++) {
                            _bcm_xgs3_hw_failover_flags_set(unit, tid,
                                    tix, FALSE,
                                    TRUNK_CNTL(unit).hw_failover_flags[idx++]);
                        }
                    }
                }
            }

            if (soc_feature(unit, soc_feature_hg_trunk_failover)) {
                /* HG trunk failover flags */
                for (tid = TRUNK_CNTL(unit).ngroups_fp;
                     tid < TRUNK_NUM_GROUPS(unit); tid++) {
                    TRUNK_INFO(unit, tid).flags =
                        TRUNK_CNTL(unit).hw_failover_flags[idx++];
                    hgtid = tid - TRUNK_CNTL(unit).ngroups_fp;
#ifdef BCM_TRIDENT_SUPPORT
                    if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
                        uint16 num_ports;
                        num_ports = TRUNK_CNTL(unit).hw_failover_flags[idx++];
                        _bcm_trident_hw_failover_num_ports_set(unit, hgtid,
                                TRUE, num_ports);
                        for (tix = 0; tix < num_ports; tix++) {
                            _bcm_trident_hw_failover_flags_set(unit, hgtid,
                                    tix, TRUE, 
                                    TRUNK_CNTL(unit).hw_failover_flags[idx++]);
                        }

                    } else
#endif /* BCM_TRIDENT_SUPPORT */
                    {
                        for (tix = 0; tix < TRUNK_CNTL(unit).nports_hg; tix++) {
                            _bcm_xgs3_hw_failover_flags_set(unit, hgtid,
                                    tix, TRUE,
                                    TRUNK_CNTL(unit).hw_failover_flags[idx++]);
                        }
                    }
                }
            }
        }
#endif /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */

        additional_scache_size = 0;

        if (recovered_ver >= BCM_WB_VERSION_1_1) {
            /* Recover BCM_TRUNK_FLAG_IPMC_CLEAVE flag */ 
            for (tid = 0; tid < TRUNK_CNTL(unit).ngroups_fp; tid++) {
                sal_memcpy(&flags, trunk_scache, sizeof(uint8));
                trunk_scache += sizeof(uint8);
                if (flags & 0x1) {
                    t_info = &TRUNK_INFO(unit, tid);
                    t_info->flags |= BCM_TRUNK_FLAG_IPMC_CLEAVE;
                }
            }
            /* Recover ipmc_psc */ 
            for (tid = 0; tid < TRUNK_CNTL(unit).ngroups_fp; tid++) {
                t_info = &TRUNK_INFO(unit, tid);
                sal_memcpy(&t_info->ipmc_psc, trunk_scache, sizeof(int));
                trunk_scache += sizeof(int);
            }
        } else {
            additional_scache_size += (TRUNK_CNTL(unit).ngroups_fp *
                    sizeof(uint8));
            additional_scache_size += (TRUNK_CNTL(unit).ngroups_fp *
                    sizeof(int));
        }

        if (recovered_ver >= BCM_WB_VERSION_1_2) {
            /* Recover front panel trunk membership */
            for (tid = 0; tid < TRUNK_CNTL(unit).ngroups_fp; tid++) {
#if defined(BCM_TRIDENT_SUPPORT)
                if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
                    uint16 num_ports;
                    uint16 *modport;
                    uint32 *member_flags;
                    uint8 truncated_flags;
                    int i;

                    sal_memcpy(&num_ports, trunk_scache, sizeof(uint16));
                    trunk_scache += sizeof(uint16);

                    modport = sal_alloc(sizeof(uint16) * num_ports, "modport");
                    if (NULL == modport) {
                        return BCM_E_MEMORY;
                    }
                    member_flags = sal_alloc(sizeof(uint32) * num_ports,
                            "member_flags");
                    if (NULL == member_flags) {
                        sal_free(modport);
                        return BCM_E_MEMORY;
                    }

                    for (i = 0; i < num_ports; i++) {
                        sal_memcpy(&modport[i], trunk_scache, sizeof(uint16));
                        trunk_scache += sizeof(uint16);
                        sal_memcpy(&truncated_flags, trunk_scache,
                                sizeof(uint8));
                        trunk_scache += sizeof(uint8);
                        member_flags[i] = truncated_flags;
                    }
                    rv = _bcm_trident_trunk_member_info_set(unit, tid,
                            num_ports, modport, member_flags);
                    sal_free(modport);
                    sal_free(member_flags);
                    if (BCM_FAILURE(rv)) {
                        return rv;
                    }
                } else 
#endif /* BCM_TRIDENT_SUPPORT */
#if defined(BCM_XGS3_SWITCH_SUPPORT)
                {
                    uint8 num_ports;
                    uint16 modport[BCM_TRUNK_MAX_PORTCNT];
                    uint32 member_flags[BCM_TRUNK_MAX_PORTCNT];
                    uint8 truncated_flags;
                    int i;

                    sal_memcpy(&num_ports, trunk_scache, sizeof(uint8));
                    trunk_scache += sizeof(uint8);
                    for (i = 0; i < num_ports; i++) {
                        sal_memcpy(&modport[i], trunk_scache, sizeof(uint16));
                        trunk_scache += sizeof(uint16);
                        sal_memcpy(&truncated_flags, trunk_scache,
                                sizeof(uint8));
                        trunk_scache += sizeof(uint8);
                        member_flags[i] = truncated_flags;
                    }
                    _bcm_xgs3_trunk_member_info_set(unit, tid, num_ports,
                            modport, member_flags);
                }
#endif /* BCM_XGS3_SWITCH_SUPPORT */
            }
        } else {
#if defined(BCM_TRIDENT_SUPPORT)
            if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
                additional_scache_size += (TRUNK_CNTL(unit).ngroups_fp * 2 +
                        soc_mem_index_count(unit, TRUNK_MEMBERm) * 3) *
                        sizeof(uint8);
            } else
#endif /* BCM_TRIDENT_SUPPORT */
#if defined(BCM_XGS3_SWITCH_SUPPORT)
            {
                additional_scache_size += (TRUNK_CNTL(unit).ngroups_fp *
                        (1 + 3 * TRUNK_CNTL(unit).nports_fp) * sizeof(uint8));
            }
#endif /* BCM_XGS3_SWITCH_SUPPORT */
        }

        if (additional_scache_size > 0) {
            SOC_IF_ERROR_RETURN
                (soc_scache_realloc(unit, scache_handle,
                                    additional_scache_size));
        }

        /* To add new state:
         * if (recovered_ver >= BCM_WB_VERSION_1_2) {
         *     Recover state for ver 1_2 
         * } else {
         *     Re-alloc scache for 'sync' operation for ver 1_2 state. 
         * }
         */
    }

    if (SOC_IS_XGS12_FABRIC(unit)) {
#ifdef BCM_XGS12_FABRIC_SUPPORT
        bcm_pbmp_t tports;
        bcm_port_t port;
        bcm_trunk_add_info_t *t_data, td;        
        uint32 val;
  	         
        for (tid = 0; tid < TRUNK_CNTL(unit).ngroups_hg; tid++) {
            PBMP_HG_ITER(unit, port) {
                SOC_IF_ERROR_RETURN(READ_ING_HGTRUNKr(unit, port, tid, &val));
                break;
            }
  	             
            SOC_PBMP_CLEAR(tports);
            SOC_PBMP_WORD_SET(tports, 0,
                              soc_reg_field_get(unit, ING_HGTRUNKr, val, BMAPf));
  	             
            sal_memset(&td, 0, sizeof(bcm_trunk_add_info_t));
            t_data = &td;
  	             
            t_data->num_ports = 0;
            PBMP_ITER(tports, port) {
                t_data->tm[t_data->num_ports] = 0; 
                t_data->tp[t_data->num_ports] = port;
                t_data->num_ports += 1;
            }        
  	             
            /*
             * Nothing to update for this TID
             */
            if (t_data->num_ports == 0) {
                continue;
            }
  	             
            switch(soc_reg_field_get(unit, ING_HGTRUNKr, val, ALGORITHMf)) {
            case 0:
                t_data->psc = BCM_TRUNK_PSC_SRCDSTMAC;
  	                 
                if (soc_reg_field_get(unit, ING_HGTRUNKr, val, EN_IPMACSAf)) {
                    t_data->psc |= BCM_TRUNK_PSC_IPMACSA;
                }
                if (soc_reg_field_get(unit, ING_HGTRUNKr, val, EN_IPMACDAf)) {
                    t_data->psc |= BCM_TRUNK_PSC_IPMACDA;
                }
                if (soc_reg_field_get(unit, ING_HGTRUNKr, val, EN_IPTYPEf)) {
                    t_data->psc |= BCM_TRUNK_PSC_IPTYPE;
                }
                if (soc_reg_field_get(unit, ING_HGTRUNKr, val, EN_IPVIDf)) {
                    t_data->psc |= BCM_TRUNK_PSC_IPVID;
                }
                if (soc_reg_field_get(unit, ING_HGTRUNKr, val, EN_IPSAf)) {
                    t_data->psc |= BCM_TRUNK_PSC_IPSA;
                }
                if (soc_reg_field_get(unit, ING_HGTRUNKr, val, EN_IPDAf)) {
                    t_data->psc |= BCM_TRUNK_PSC_IPDA;
                }
                if (soc_reg_field_get(unit, ING_HGTRUNKr, val, EN_L4SSf)) {
                    t_data->psc |= BCM_TRUNK_PSC_L4SS;
                }
                if (soc_reg_field_get(unit, ING_HGTRUNKr, val, EN_L4DSf)) {
                    t_data->psc |= BCM_TRUNK_PSC_L4DS;
                }
                if (soc_reg_field_get(unit, ING_HGTRUNKr, val, EN_MACDAf)) {
                    t_data->psc |= BCM_TRUNK_PSC_MACDA;
                }
                if (soc_reg_field_get(unit, ING_HGTRUNKr, val, EN_MACSAf)) {
                    t_data->psc |= BCM_TRUNK_PSC_MACSA;
                }
                if (soc_reg_field_get(unit, ING_HGTRUNKr, val, EN_TYPEf)) {
                    t_data->psc |= BCM_TRUNK_PSC_TYPE;
                }
                if (soc_reg_field_get(unit, ING_HGTRUNKr, val, EN_VIDf)) {
                    t_data->psc |= BCM_TRUNK_PSC_VID;
                }
                break;
            case 1:
                t_data->psc = BCM_TRUNK_PSC_REDUNDANT;
                break;
            default:
                t_data->psc = -1;
                break;
            }
  	             
            t_data->dlf_index = -1;
            t_data->mc_index = -1;
            t_data->ipmc_index = -1;
  	             
            /*
             * Fill in t_info struct for this tid
             */
            t_info = &TRUNK_INFO(unit, tid);
  	             
            /*
             * Check number of ports in trunk group
             */
            if (t_data->num_ports < 1 ||
                (TRUNK_FP_TID(unit, tid) &&
                 (t_data->num_ports > TRUNK_CNTL(unit).nports_fp)) ||
                (TRUNK_FABRIC_TID(unit, tid) &&
                 (t_data->num_ports > TRUNK_CNTL(unit).nports_hg))) {
                return (BCM_E_PARAM);
            }
  	             
            TRUNK_LOCK(unit);
            t_info->tid = tid;
            t_info->psc = t_data->psc;
            t_info->in_use = TRUE;
            TRUNK_UNLOCK(unit);
        }
#endif /* BCM_XGS12_FABRIC_SUPPORT */
    } else if (SOC_IS_XGS3_FABRIC(unit)) {
#ifdef BCM_XGS3_FABRIC_SUPPORT
        /*
         * recover software state for fabric trunks
         */
        BCM_IF_ERROR_RETURN(_bcm_trunk_fabric_reinit(unit));
#endif /* BCM_XGS3_FABRIC_SUPPORT */
    }

#ifdef BCM_XGS_SWITCH_SUPPORT
    if (SOC_IS_XGS3_SWITCH(unit)) {
        if (SOC_IS_FBX(unit)) { 
            /*
             * recover software state for fabric trunks
             */
            BCM_IF_ERROR_RETURN(_bcm_trunk_fabric_reinit(unit));
  	 
            /*
             * recover software state for ether trunks
             */
            BCM_IF_ERROR_RETURN(_bcm_trunk_lag_reinit(unit));
        } else if (SOC_IS_EASYRIDER(unit)) {
            /*
             * recover software state for ether trunks
             */
            BCM_IF_ERROR_RETURN(_bcm_trunk_lag_reinit(unit));		
        }
    }
#endif
  	     
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_trunk_sync
 * Purpose:
 *      Record Trunk module persisitent info for Level 2 Warm Boot
 * Parameters:
 *      unit - StrataSwitch unit number.
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_esw_trunk_sync(int unit)
{
    bcm_trunk_t tid;
    trunk_private_t *t_info;
    soc_scache_handle_t scache_handle;
    uint8               *trunk_scache;
    uint32              ngroups;
    uint8               flags;
#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
    bcm_trunk_t         hgtid;
    int                 tix, idx;
#endif /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */

    if (SOC_IS_XGS12_FABRIC(unit)) {
        return BCM_E_NONE;
    }

    TRUNK_INIT(unit);

    /* Limited scache mode unsupported */
    if (SOC_WARM_BOOT_SCACHE_IS_LIMITED(unit)) {
        return BCM_E_NONE;
    }

    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_TRUNK, 0);
    BCM_IF_ERROR_RETURN
        (_bcm_esw_scache_ptr_get(unit, scache_handle, FALSE,
                                 0,
                                 &trunk_scache, BCM_WB_DEFAULT_VERSION, NULL));

    ngroups = TRUNK_CNTL(unit).ngroups_fp;
    sal_memcpy(trunk_scache, &ngroups, sizeof(ngroups));
    trunk_scache += sizeof(ngroups);
        
    ngroups = TRUNK_CNTL(unit).ngroups_hg;
    sal_memcpy(trunk_scache, &ngroups, sizeof(ngroups));
    if (ngroups != TRUNK_CNTL(unit).ngroups_hg) {
        return BCM_E_CONFIG;
    }
    trunk_scache += sizeof(ngroups);

    sal_memset(TRUNK_CNTL(unit).trunk_bmp_ptr, 0,
               TRUNK_CNTL(unit).trunk_bmp_alloc);

    for (tid = 0; tid < TRUNK_NUM_GROUPS(unit); tid++) {
        t_info = &TRUNK_INFO(unit, tid);
        if (BCM_TRUNK_INVALID != t_info->tid) {
            SHR_BITSET(TRUNK_CNTL(unit).trunk_bmp_ptr, tid);
        }
        t_info++;
    }
    sal_memcpy(trunk_scache, TRUNK_CNTL(unit).trunk_bmp_ptr,
               TRUNK_CNTL(unit).trunk_bmp_alloc);
    trunk_scache += TRUNK_CNTL(unit).trunk_bmp_alloc;

#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
    if (TRUNK_CNTL(unit).hw_failover_alloc) {
        idx = 0;

        if (soc_feature(unit, soc_feature_port_lag_failover)) {
            /* FP trunk failover flags */
            for (tid = 0; tid < TRUNK_CNTL(unit).ngroups_fp; tid++) {
                TRUNK_CNTL(unit).hw_failover_flags[idx++] =
                    TRUNK_INFO(unit, tid).flags;
#if defined(BCM_TRIDENT_SUPPORT)
                if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
                    uint16 num_ports; 
                    num_ports = _bcm_trident_hw_failover_num_ports_get(unit, tid, FALSE);
                    /* Max number of ports per front-panel trunk group is 256,
                     * requiring two bytes of storage. */
                    TRUNK_CNTL(unit).hw_failover_flags[idx++] = num_ports >> 8;
                    TRUNK_CNTL(unit).hw_failover_flags[idx++] = num_ports & 0xff;
                    for (tix = 0; tix < num_ports; tix++) {
                        TRUNK_CNTL(unit).hw_failover_flags[idx++] = 
                            _bcm_trident_hw_failover_flags_get(unit, tid,
                                    tix, FALSE);
                    }
                } else
#endif /* BCM_TRIDENT_SUPPORT */
                {
                    for (tix = 0; tix < TRUNK_CNTL(unit).nports_fp; tix++) {
                        TRUNK_CNTL(unit).hw_failover_flags[idx++] = 
                            _bcm_xgs3_hw_failover_flags_get(unit, tid,
                                    tix, FALSE);
                    }
                }
            }
        }

        if (soc_feature(unit, soc_feature_hg_trunk_failover)) {
            /* HG trunk failover flags */
            for (tid = TRUNK_CNTL(unit).ngroups_fp;
                    tid < TRUNK_NUM_GROUPS(unit); tid++) {
                TRUNK_CNTL(unit).hw_failover_flags[idx++] =
                    TRUNK_INFO(unit, tid).flags;
                hgtid = tid - TRUNK_CNTL(unit).ngroups_fp;
#if defined(BCM_TRIDENT_SUPPORT)
                if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
                    uint16 num_ports; 
                    num_ports = _bcm_trident_hw_failover_num_ports_get(unit, hgtid, TRUE);
                    /* Max number of ports per front-panel trunk group is 32,
                     * requiring one byte of storage. */
                    TRUNK_CNTL(unit).hw_failover_flags[idx++] = num_ports;
                    for (tix = 0; tix < num_ports; tix++) {
                        TRUNK_CNTL(unit).hw_failover_flags[idx++] = 
                            _bcm_trident_hw_failover_flags_get(unit, hgtid,
                                    tix, TRUE);
                    }
                } else
#endif /* BCM_TRIDENT_SUPPORT */
                {
                    for (tix = 0; tix < TRUNK_CNTL(unit).nports_hg; tix++) { 
                        TRUNK_CNTL(unit).hw_failover_flags[idx++] =  
                            _bcm_xgs3_hw_failover_flags_get(unit, hgtid,
                                    tix, TRUE);
                    }
                }
            }
        }
        sal_memcpy(trunk_scache, TRUNK_CNTL(unit).hw_failover_flags,
                   TRUNK_CNTL(unit).hw_failover_alloc);
        trunk_scache += TRUNK_CNTL(unit).hw_failover_alloc;
    }
#endif /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */

    /* Store BCM_TRUNK_FLAG_IPMC_CLEAVE flag */
    for (tid = 0; tid < TRUNK_CNTL(unit).ngroups_fp; tid++) {
        t_info = &TRUNK_INFO(unit, tid);
        if (t_info->flags & BCM_TRUNK_FLAG_IPMC_CLEAVE) {
            flags = 0x1;
        } else {
            flags = 0x0;
        }
        sal_memcpy(trunk_scache, &flags, sizeof(uint8));
        trunk_scache += sizeof(uint8);
    }

    /* Store ipmc_psc */
    for (tid = 0; tid < TRUNK_CNTL(unit).ngroups_fp; tid++) {
        t_info = &TRUNK_INFO(unit, tid);
        sal_memcpy(trunk_scache, &t_info->ipmc_psc, sizeof(int));
        trunk_scache += sizeof(int);
    }

    /* Store front-panel trunk membership info */
    for (tid = 0; tid < TRUNK_CNTL(unit).ngroups_fp; tid++) {
#if defined(BCM_TRIDENT_SUPPORT)
        if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
            uint16 num_ports;
            uint16 *modport;
            uint32 *member_flags;
            uint8  truncated_flags;
            int rv, i;

            BCM_IF_ERROR_RETURN(_bcm_trident_trunk_member_info_get(unit, tid,
                        0, NULL, NULL, &num_ports));
            modport = sal_alloc(sizeof(uint16) * num_ports, "modport");
            if (NULL == modport) {
                return BCM_E_MEMORY;
            }
            member_flags = sal_alloc(sizeof(uint32) * num_ports,
                    "member_flags");
            if (NULL == member_flags) {
                sal_free(modport);
                return BCM_E_MEMORY;
            }
            rv = _bcm_trident_trunk_member_info_get(unit, tid,
                        num_ports, modport, member_flags, &num_ports);
            if (BCM_FAILURE(rv)) {
                sal_free(modport);
                sal_free(member_flags);
                return rv;
            }

            sal_memcpy(trunk_scache, &num_ports, sizeof(uint16));
            trunk_scache += sizeof(uint16);
            for (i = 0; i < num_ports; i++) {
                sal_memcpy(trunk_scache, &modport[i], sizeof(uint16));
                trunk_scache += sizeof(uint16);

                /* Truncate the flags to 1 byte, as there are only 4 flags */
                truncated_flags = member_flags[i] & 0xff;
                sal_memcpy(trunk_scache, &truncated_flags, sizeof(uint8));
                trunk_scache += sizeof(uint8);
            }

            sal_free(modport);
            sal_free(member_flags);
        } else
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_XGS3_SWITCH_SUPPORT
        {
            uint8  num_ports;
            uint16 modport[BCM_TRUNK_MAX_PORTCNT];
            uint32 member_flags[BCM_TRUNK_MAX_PORTCNT];
            uint8  truncated_flags;
            int i;

            _bcm_xgs3_trunk_member_info_get(unit, tid, &num_ports,
                    modport, member_flags);
            sal_memcpy(trunk_scache, &num_ports, sizeof(uint8));
            trunk_scache += sizeof(uint8);
            for (i = 0; i < num_ports; i++) {
                sal_memcpy(trunk_scache, &modport[i], sizeof(uint16));
                trunk_scache += sizeof(uint16);

                /* Truncate the flags to 1 byte, as there are only 4 flags */
                truncated_flags = member_flags[i] & 0xff;
                sal_memcpy(trunk_scache, &truncated_flags, sizeof(uint8));
                trunk_scache += sizeof(uint8);
            }
        }
#endif /* BCM_XGS3_SWITCH_SUPPORT */
    }

    return BCM_E_NONE;
}
#endif /* BCM_WARM_BOOT_SUPPORT */
  	 
#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
/*
 * Function:
 *     _bcm_trunk_sw_dump
 * Purpose:
 *     Displays trunk information maintained by software.
 * Parameters:
 *     unit - Device unit number
 * Returns:
 *     None
 */
void
_bcm_trunk_sw_dump(int unit)
{
    trunk_cntl_t    *t_cntl;
    trunk_private_t *t_info;
    int             i;
    int             print_trunk_info;

    t_cntl = &TRUNK_CNTL(unit);
    soc_cm_print("\nSW Information TRUNK - Unit %d\n", unit);
    soc_cm_print("  Front  panel trunk groups    : %d\n", t_cntl->ngroups_fp);
    soc_cm_print("  Front  panel trunk max ports : %d\n", t_cntl->nports_fp);
    soc_cm_print("  Fabric panel trunk groups    : %d\n", t_cntl->ngroups_hg);
    soc_cm_print("  Fabric panel trunk max ports : %d\n", t_cntl->nports_hg);

    print_trunk_info = TRUE;
    for (i = 0; i < TRUNK_NUM_GROUPS(unit); i++) {
        t_info = &TRUNK_INFO(unit, i);
#if defined(BCM_TRIDENT_SUPPORT)
        if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
            /* Trident has 1024 front panel trunk groups.
             * Print only the ones that are in use. 
             */
            print_trunk_info = t_info->tid != BCM_TRUNK_INVALID;
        } 
#endif /* BCM_TRIDENT_SUPPORT */
        if (print_trunk_info) {
            soc_cm_print("  Trunk %d\n", i);
            soc_cm_print("      ID              : %d\n", t_info->tid);
            soc_cm_print("      in use          : %d\n", t_info->in_use);
            soc_cm_print("      psc             : 0x%x\n", t_info->psc);
            soc_cm_print("      ipmc_psc        : 0x%x\n", t_info->ipmc_psc);
            soc_cm_print("      rtag            : 0x%x\n", t_info->rtag);
            soc_cm_print("      flags           : 0x%x\n", t_info->flags);
            if (soc_feature(unit, soc_feature_hg_dlb)) {
                soc_cm_print("      dynamic_size    : 0x%x\n", t_info->dynamic_size);
                soc_cm_print("      dynamic_age     : 0x%x\n", t_info->dynamic_age);
            }
            if (!SOC_IS_XGS3_SWITCH(unit)) {
                /* Unused for XGS3 devices */
                soc_cm_print("      dlf index spec  : %d\n",
                        t_info->dlf_index_spec);
                soc_cm_print("      dlf index used  : %d\n",
                        t_info->dlf_index_used);
                soc_cm_print("      dlf port used   : %d\n", 
                        t_info->dlf_port_used);
                soc_cm_print("      mc index spec   : %d\n",
                        t_info->mc_index_spec);
                soc_cm_print("      mc index used   : %d\n",
                        t_info->mc_index_used);
                soc_cm_print("      mc port used    : %d\n",
                        t_info->mc_port_used);
                soc_cm_print("      ipmc index spec : %d\n",
                        t_info->ipmc_index_spec);
                soc_cm_print("      ipmc index used : %d\n",
                        t_info->ipmc_index_used);
                soc_cm_print("      ipmc port used  : %d\n",
                        t_info->ipmc_port_used);
            }
        }
    }

#if defined(BCM_TRIDENT_SUPPORT)
    if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
        _bcm_trident_trunk_sw_dump(unit);
    } else 
#endif /* BCM_TRIDENT_SUPPORT */
#if defined(BCM_XGS3_SWITCH_SUPPORT)
        if (SOC_IS_FBX(unit) && !SOC_IS_RAPTOR(unit)) {
            _bcm_xgs3_trunk_sw_dump(unit);
        }
#endif /* BCM_XGS3_SWITCH_SUPPORT */

    return;
}
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

/*
 * Function:
 *      _bcm_esw_trunk_lock
 * Purpose:
 *     Lock Trunk module - if module was not initialized NOOP
 *    
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_esw_trunk_lock(int unit)
{
    if (NULL != TRUNK_CNTL(unit).lock) {
        return sal_mutex_take(TRUNK_CNTL(unit).lock, 
                              sal_mutex_FOREVER);
    }
    return (BCM_E_NONE);
}
/*
 * Function:
 *      _bcm_esw_trunk_unlock
 * Purpose:
 *     Unlock Trunk module - if module was not initialized NOOP
 *    
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_esw_trunk_unlock(int unit)
{
    if (NULL != TRUNK_CNTL(unit).lock) {
        return sal_mutex_give(TRUNK_CNTL(unit).lock);
    }
    return (BCM_E_NONE);
}
