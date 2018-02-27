/*
 * $Id: trunk.h 1.40.8.6 Broadcom SDK $
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
 * This file contains TRUNK definitions internal to the BCM library.
 */

#ifndef _BCM_INT_TRUNK_H
#define _BCM_INT_TRUNK_H

#include <bcm/trunk.h>
#include <bcm/switch.h>

/* 6-bit field, MSB indicates trunk */
#define BCM_TGID_TRUNK_INDICATOR(unit)     (1 << SOC_TRUNK_BIT_POS(unit))
#define BCM_TGID_PORT_TRUNK_MASK(unit)     ((1 << SOC_TRUNK_BIT_POS(unit)) - 1)
#define BCM_TGID_PORT_TRUNK_MASK_HI(unit)  (0x3 << SOC_TRUNK_BIT_POS(unit))
#define BCM_TGID_PORT_TRUNK_MASK_HI_SHIFT(unit)  (SOC_TRUNK_BIT_POS(unit))

#define BCM_TRUNK_TO_MODIDf(unit, tid) \
 (((tid) & BCM_TGID_PORT_TRUNK_MASK_HI(unit)) >> BCM_TGID_PORT_TRUNK_MASK_HI_SHIFT(unit))
#define BCM_TRUNK_TO_TGIDf(unit, tid) \
 (((tid) & BCM_TGID_PORT_TRUNK_MASK(unit)) | BCM_TGID_TRUNK_INDICATOR(unit))
#define BCM_MODIDf_TGIDf_TO_TRUNK(unit,mod,tgid) \
 ((((mod)<<BCM_TGID_PORT_TRUNK_MASK_HI_SHIFT(unit))&BCM_TGID_PORT_TRUNK_MASK_HI(unit)) | \
 ((tgid) & BCM_TGID_PORT_TRUNK_MASK(unit)))

/*
 * Define:
 *	TRUNK_CHK_TGID32
 * Purpose:
 *	Causes a routine to return BCM_E_PARAM if the specified
 *	trunk group ID is out of range 0-31.
 */

#define TRUNK_CHK_TGID32(unit, tgid) do { \
	if (tgid < 0 || tgid > 31) return BCM_E_PARAM; \
	} while (0);

/*
 * Define:
 *	TRUNK_CHK_TGID128
 * Purpose:
 *	Causes a routine to return BCM_E_PARAM if the specified
 *	trunk group ID is out of range 0-127.
 */

#define TRUNK_CHK_TGID128(unit, tgid) do { \
	if (tgid < 0 || tgid > 127) return BCM_E_PARAM; \
	} while (0);

/*
 * Define:
 *    TRUNK_CHK_TGID_EXTENDED
 * Purpose:
 *    Causes a routine to return BCM_E_PARAM if the specified
 *    trunk group ID is out of range on this unit.
 */
#define TRUNK_CHK_TGID_EXTENDED(unit, tgid) \
    do {                                    \
        if (tgid < 0 || tgid > (soc_mem_index_max(unit, TRUNK_GROUPm))) { \
            return (BCM_E_PARAM);           \
        }                                   \
    } while (0);

typedef struct trunk_private_s {
    int		tid;			/* trunk group ID */
    int		in_use;
    int		psc;			/* port spec criterion */
    int		ipmc_psc;	        /* IPMC port spec criterion */
    int		rtag;			/* hardware rtag value */
    uint32      flags;                  /* BCM_TRUNK_FLAG_FAILOVER_xxx */
    int		dlf_index_spec;		/* requested by user */
    int		dlf_index_used;		/* actually used */
    int		dlf_port_used;		/* actually used or -1 */
    int		mc_index_spec;		/* requested by user */
    int		mc_index_used;		/* actually used */
    int		mc_port_used;		/* actually used or -1 */
    int		ipmc_index_spec;	/* requested by user */
    int		ipmc_index_used;	/* actually used */
    int		ipmc_port_used;		/* actually used or -1 */
    uint32      dynamic_size;           /* number of flows for
                                           dynamic load balancing */
    uint32      dynamic_age;            /* Inactivity duration, in usec */
} trunk_private_t;

#define BCM_SWITCH_TRUNK_MAX_PORTCNT   8       
            /* Max # front panel ports in trunk on most devices, except BCM56840 */
#define BCM_XGS1_FABRIC_TRUNK_MAX_PORTCNT    4       
#define BCM_XGS23_FABRIC_TRUNK_MAX_PORTCNT   8       
            /* Max # Higig ports in fabric trunk on most devices, except BCM56840 */

extern int _bcm_esw_trunk_gport_resolve(int unit, bcm_trunk_t tid,
                                        bcm_trunk_add_info_t *t_data);

extern int _bcm_esw_trunk_gport_array_resolve(int unit, int fabric_trunk,
                                   int count,
                                   bcm_gport_t *port_array,
                                   bcm_port_t *port_list,
                                   bcm_module_t *modid_list);

extern int _bcm_esw_trunk_gport_construct(int unit, int fabric_trunk,
                               int count,
                               bcm_port_t *port_list,
                               bcm_module_t *modid_list,
                               bcm_gport_t *port_array);

extern int _bcm_esw_trunk_port_property_get(int unit, 
                                            bcm_module_t modid,
                                            bcm_port_t port, 
                                            bcm_trunk_t *tid);

extern int _bcm_nuc_tpbm_get(int unit,
                             int num_ports,
                             bcm_module_t tm[BCM_TRUNK_MAX_PORTCNT],
                             uint32 *nuc_tpbm);

#if defined (BCM_XGS3_SWITCH_SUPPORT)
#define _BCM_XGS3_TRUNK_MOD_PORT_MAP_IDX_COUNT(_unit_)     \
      (SOC_MAX_NUM_PORTS * ((SOC_MODID_MAX(_unit_) > 0) ?  \
                            (1 + SOC_MODID_MAX(_unit_)) : 1))
extern int _bcm_xgs3_trunk_mod_port_map_init(int unit);
extern int _bcm_xgs3_trunk_mod_port_map_deinit(int unit);

extern int  _bcm_xgs3_trunk_swfailover_init(int unit);
extern void _bcm_xgs3_trunk_swfailover_detach(int unit);

extern int  _bcm_xgs3_trunk_member_init(int unit);
extern void _bcm_xgs3_trunk_member_detach(int unit);

extern int bcm_xgs3_trunk_set(int, bcm_trunk_t,
                        bcm_trunk_add_info_t *, trunk_private_t *);
extern int bcm_xgs3_trunk_get(int, bcm_trunk_t,
                        bcm_trunk_add_info_t *, trunk_private_t *);
extern int bcm_xgs3_trunk_destroy(int, bcm_trunk_t, trunk_private_t *);
extern int bcm_xgs3_trunk_mcast_join(int, bcm_trunk_t, bcm_vlan_t, 
                               bcm_mac_t, trunk_private_t *);
extern int _bcm_xgs3_trunk_fabric_find(int unit, 
                                       bcm_port_t port, 
                                       bcm_trunk_t *tid);
extern int _bcm_xgs3_trunk_get_port_property(int unit, 
                                             bcm_module_t mod,
                                             bcm_port_t port, 
                                             bcm_trunk_t *tid);

extern int  _bcm_xgs3_trunk_hwfailover_init(int unit);
extern void _bcm_xgs3_trunk_hwfailover_detach(int unit);

extern int _bcm_xgs3_trunk_hwfailover_set(int unit, bcm_trunk_t tid,
                                          int hg_trunk,
                                          bcm_port_t port,
                                          bcm_module_t modid,
                                          int psc,
                                          uint32 flags, int count, 
                                          bcm_port_t *ftp,
                                          bcm_module_t *ftm);
extern int _bcm_xgs3_trunk_hwfailover_get(int unit, bcm_trunk_t tid,
                                          int hg_trunk,
                                          bcm_port_t port,
                                          bcm_module_t modid,
                                          int *psc,
                                          uint32 *flags, int array_size, 
                                          bcm_port_t *ftp,
                                          bcm_module_t *ftm,
                                          int *array_count);
extern int _bcm_xgs3_trunk_property_migrate(int unit,
                int num_leaving, bcm_gport_t *leaving_arr,
                int num_staying, bcm_gport_t *staying_arr,
                int num_joining, bcm_gport_t *joining_arr);

#ifdef BCM_TRIDENT_SUPPORT
extern int _bcm_trident_trunk_init(int unit);
void _bcm_trident_trunk_deinit(int unit);
extern int _bcm_trident_trunk_override_mcast_set(int unit,
                                                 bcm_trunk_t hgtid,
                                                 int idx,
                                                 int enable);
extern int _bcm_trident_trunk_override_mcast_get(int unit,
                                                 bcm_trunk_t hgtid,
                                                 int idx,
                                                 int *enable);
extern int _bcm_trident_trunk_override_ipmc_set(int unit,
                                                bcm_trunk_t hgtid,
                                                int idx,
                                                int enable);
extern int _bcm_trident_trunk_override_ipmc_get(int unit,
                                                bcm_trunk_t hgtid,
                                                int idx,
                                                int *enable);
extern int _bcm_trident_trunk_override_vlan_set(int unit,
                                                bcm_trunk_t hgtid,
                                                int idx,
                                                int enable);
extern int _bcm_trident_trunk_override_vlan_get(int unit,
                                                bcm_trunk_t hgtid,
                                                int idx,
                                                int *enable);
extern int _bcm_trident_trunk_fabric_find(int unit, 
                                          bcm_port_t port, 
                                          bcm_trunk_t *tid);
extern int _bcm_trident_trunk_get_port_property(int unit, bcm_module_t mod, 
                                                bcm_port_t port, int *tid);
extern int _bcm_trident_trunk_hwfailover_set(int unit,
                                             bcm_trunk_t tid, int hg_trunk,
                                             bcm_port_t port, bcm_module_t modid,
                                             int psc, uint32 flags, int count, 
                                             bcm_port_t *ftp, bcm_module_t *ftm);
extern int _bcm_trident_trunk_hwfailover_get(int unit,
                                             bcm_trunk_t tid, int hg_trunk,
                                             bcm_port_t port, bcm_module_t modid,
                                             int *psc, uint32 *flags, int array_size,
                                             bcm_port_t *ftp, bcm_module_t *ftm, 
                                             int *array_count);
extern int bcm_trident_trunk_member_set(int unit,
                                        bcm_trunk_t tid, 
                                        trunk_private_t *t_info,
                                        int member_count, 
                                        bcm_trunk_member_t *member_array);
extern int bcm_trident_trunk_member_get(int unit,
                                        bcm_trunk_t tid, 
                                        trunk_private_t *t_info,
                                        int member_max, 
                                        bcm_trunk_member_t *member_array,
                                        int *member_count);
extern int _bcm_trident_hg_dlb_config_set(int unit,
                                          bcm_switch_control_t type,
                                          int arg);
extern int _bcm_trident_hg_dlb_config_get(int unit,
                                          bcm_switch_control_t type,
                                          int *arg);
extern int _bcm_trident_hg_dlb_config_port_set(int unit,
                                          bcm_port_t port,
                                          bcm_switch_control_t type,
                                          int arg);
extern int _bcm_trident_hg_dlb_config_port_get(int unit,
                                          bcm_port_t port,
                                          bcm_switch_control_t type,
                                          int *arg);

#endif /* BCM_TRIDENT_SUPPORT */

#endif /* BCM_XGS3_SWITCH_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT
extern int _bcm_trunk_reinit(int unit);
#if defined (BCM_XGS3_SWITCH_SUPPORT)
extern int _xgs3_trunk_reinit(int unit, int ngroups_fp, trunk_private_t *);
extern int _xgs3_trunk_fabric_reinit(int unit, int min_tid, int max_tid,
                                     trunk_private_t *);

#if defined(BCM_TRX_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
extern void _bcm_xgs3_hw_failover_flags_set(int unit, bcm_trunk_t tid,
                                            int port_index, int hg_trunk,
                                            uint8 flags);
extern uint8 _bcm_xgs3_hw_failover_flags_get(int unit, bcm_trunk_t tid,
                                             int port_index, int hg_trunk);
#endif /* BCM_TRX_SUPPORT || BCM_BRADLEY_SUPPORT */

extern void _bcm_xgs3_trunk_member_info_set(int unit, bcm_trunk_t tid,
                                uint8 num_ports,
                                uint16 modport[BCM_TRUNK_MAX_PORTCNT],
                                uint32 member_flags[BCM_TRUNK_MAX_PORTCNT]);
extern void _bcm_xgs3_trunk_member_info_get(int unit, bcm_trunk_t tid,
                                uint8 *num_ports,
                                uint16 modport[BCM_TRUNK_MAX_PORTCNT],
                                uint32 member_flags[BCM_TRUNK_MAX_PORTCNT]);

#ifdef BCM_TRIDENT_SUPPORT
extern int _bcm_trident_trunk_mod_port_map_reinit(int unit, bcm_module_t mod,
                                         int base_index, int num_ports); 
extern int _bcm_trident_trunk_lag_reinit(int unit, int ngroups_fp,
                                         trunk_private_t *);
extern int _bcm_trident_trunk_fabric_reinit(int unit, int min_tid, int max_tid,
                                     trunk_private_t *);
extern void _bcm_trident_hw_failover_num_ports_set(int unit, bcm_trunk_t tid,
                                         int hg_trunk, uint16 num_ports);
extern uint16 _bcm_trident_hw_failover_num_ports_get(int unit, bcm_trunk_t tid,
                                         int hg_trunk);
extern int _bcm_trident_hw_failover_flags_set(int unit, bcm_trunk_t tid,
                                         int port_index, int hg_trunk,
                                         uint8 flags);
extern uint8 _bcm_trident_hw_failover_flags_get(int unit, bcm_trunk_t tid,
                                         int port_index, int hg_trunk);
extern int _bcm_trident_trunk_member_info_set(int unit, bcm_trunk_t tid,
                                uint16 num_ports,
                                uint16 *modport,
                                uint32 *member_flags);
extern int _bcm_trident_trunk_member_info_get(int unit, bcm_trunk_t tid,
                                uint16 array_size,
                                uint16 *modport,
                                uint32 *member_flags,
                                uint16 *num_ports);
#endif /* BCM_TRIDENT_SUPPORT */

#endif /* BCM_XGS3_SWITCH_SUPPORT */
#else
#define _bcm_trunk_reinit(unit)     (BCM_E_NONE)
#if defined (BCM_XGS3_SWITCH_SUPPORT)
#define  _xgs3_trunk_reinit(unit, ng_fp, tp)            (BCM_E_UNAVAIL)
#define  _xgs3_trunk_fabric_reinit(unit, min, max, tp)  (BCM_E_UNAVAIL)
#endif /* BCM_XGS3_SWITCH_SUPPORT */
#endif /* BCM_WARM_BOOT_SUPPORT */

extern int _bcm_esw_trunk_lock(int unit);
extern int _bcm_esw_trunk_unlock(int unit);

#ifdef BCM_WARM_BOOT_SUPPORT
extern int _bcm_esw_trunk_sync(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
extern void _bcm_trunk_sw_dump(int unit);
#if defined(BCM_XGS3_SWITCH_SUPPORT)
extern void _bcm_xgs3_trunk_sw_dump(int unit);

#ifdef BCM_TRIDENT_SUPPORT
extern void _bcm_trident_trunk_sw_dump(int unit);
#endif /* BCM_TRIDENT_SUPPORT */

#endif /* BCM_XGS3_SWITCH_SUPPORT */
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

extern int _bcm_trunk_id_validate(int unit, bcm_trunk_t tid);
extern int _bcm_esw_trunk_local_members_get(int unit, bcm_trunk_t trunk_id, 
                                            int local_member_max, 
                                            bcm_port_t *local_member_array, 
                                            int *local_member_count);
#endif	/* !_BCM_INT_TRUNK_H */
