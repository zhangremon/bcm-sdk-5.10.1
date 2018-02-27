/*
 * $Id: vlan.c 1.374 Broadcom SDK $
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
 * Module: VLAN management
 */

#include <soc/drv.h>
#include <bcm/debug.h>
#include <bcm/error.h>
#include <bcm/vlan.h>
#include <bcm/types.h>
#include <bcm/stg.h>
#include <bcm/pkt.h>
#include <bcm/qos.h>
#include <bcm/trunk.h>

#include <shared/idxres_fl.h>

/*
 *  Some options of which I'm not sure yet...
 *
 *  VLAN_LOCK_ATOMIC_READ must be nonzero use the unit VLAN lock when making
 *  reads from the VLAN globals that can be accomplished atomically.  In
 *  general, this is excessive paranoia unless there is some architectural
 *  reason that x = y can't be done atomically on a register-width memory
 *  operand (perhaps the bus is narrower than the register).
 *
 *  VLAN_DEFAULT_UNTAGGED must be TRUE if ports that are implicitly members of
 *  the default VID are to transmit frames in that VID untagged.  It must be
 *  FALSE if ports that are implicitly members of the default VID are to
 *  transmit frames tagged.  Software can override this setting by explicitly
 *  adding a port to the default VID with the desired tag/untag state.
 *
 *  VLAN_FILTER_DEFAULT is the default state of each port on each VLAN's
 *  802.1q 8.6.2 filter.  If TRUE, frames that come in on a port for VLANs of
 *  which the port is not a member will be dropped by default.  If FALSE, any
 *  frame with a valid tag will be accepted on any port by default.  The actual
 *  value can be changed at run time; this is only the power-on default.
 *
 *  VLAN_COS_AGGREGATE can be nonzero to allow BCM_COS_INVALID to access stats
 *  that imply aggregation of stats across all COS levels (for example, getting
 *  the yellow frame count for all COS instead of just one).  If it is zero,
 *  this feature is disabled.  Where a single maintained stat already includes
 *  all COS levels, this switch does not disable access; it only affects stats
 *  where we aggregate across COS levels in *this* code.
 *
 *  VLAN_STAT_NOT_COS is the error returned when an attempt is made to access a
 *  statistic at a COS level where it is not supported in the mode currently
 *  being used by the counters for the VLAN.
 *
 *  VLAN_STAT_NOT_VAL is the error returned when an attempt is made to access a
 *  statistics that is not available in the current counter mode for the VLAN.
 *
 *  VLAN_EXCESS_VERBOSITY is whether debugging messages are enabled at a level
 *  that will quickly become obnoxious.  May help in debugging, but also slows
 *  things down immensely.  Additional message will be provided at VVERBOSE
 *  level to the console.
 *
 *  VLAN_MAX_VID_VALUE is the largest value supported for VID -- IT IS NOT A
 *  COUNT; don't treat it like it's one-based.
 *
 *  VLAN_MAX_VID_VALUE_SIM is the largest value supported for VID when running
 *  on the simulator.  It is used instead of VLAN_MAX_VID_VALUE in that case.
 *
 *  VLAN_UNKNOWN_TYPE is the error code returned when the unit is of a type
 *  that is not supported by this code.
 *
 *  VLAN_INV_FTE_EXC is currently a placeholder.  It is meant to be the
 *  exception number used in the g2p3 mode to indicate an invalid FTE (any
 *  entry that hits the FTE is not forwarded and optionally can be sent to the
 *  control processor, but more likely just dropped).
 */
#define VLAN_LOCK_ATOMIC_READ    FALSE /* true or false lock on atomic reads */
#define VLAN_DEFAULT_UNTAGGED     TRUE /* true or false dflt VID+port untag */
#define VLAN_FILTER_DEFAULT      FALSE /* true or false dflt 802.1q 8.6.2 */
#define VLAN_COS_AGGREGATE       FALSE /* true or false allow COS aggregation */
#define VLAN_STAT_NOT_COS  BCM_E_PARAM /* error from ctr at wrong COS level */
#define VLAN_STAT_NOT_VAL  BCM_E_PARAM /* error from ctr not in this mode */
#define VLAN_EXCESS_VERBOSITY    TRUE /* true or false more debug output */
#define VLAN_MAX_VID_VALUE        4094 /* start at zero (reserved for drop) */
#define VLAN_MAX_VID_VALUE_SIM    VLAN_MAX_VID_VALUE /* Maximum VID if on sim */
#define VLAN_UNKNOWN_TYPE BCM_E_UNAVAIL /* unknown/disabled microcode error */
#define VLAN_MAX_VLAN_GPORTS      8192 /* max VLAN GPORTs supported */


#if 0
#define BCM_VLAN_NO_DEFAULT_ETHER
#define BCM_VLAN_NO_DEFAULT_CPU
#define BCM_VLAN_NO_DEFAULT_SPI_SUBPORT
#endif

/* VLAN_WB_NUM_VID_SCAN, defines the number of vids needed to be in the same 
 * state for the purposs of determining a  port's discard mode during
 * recovery/warm boot.  10 was chosen arbitrarily
 */
#define VLAN_WB_NUM_VID_SCAN   10



#include <bcm/stack.h>

#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/fe2000/vlan.h>
#include <bcm_int/sbx/fe2000/mpls.h>
#include <bcm_int/sbx/stg.h>
#include <bcm_int/sbx/trunk.h>
#include <bcm_int/sbx/fe2000/policer.h>
#include <bcm_int/sbx/fe2000/mirror.h>
#include <bcm_int/sbx/fe2000/allocator.h>
#include <bcm_int/sbx/stat.h>
#include <bcm_int/sbx/fe2000/recovery.h>
#include <bcm_int/sbx/stack.h>


#include <soc/sbx/sbDq.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbStatus.h>

#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#include <bcm_int/sbx/fe2000/g2p3.h>
#include <bcm_int/sbx/fe2000/port.h>
#include <bcm_int/sbx/fe2000/mpls.h>
#endif /* def BCM_FE2000_P3_SUPPORT */

#include <bcm_int/sbx/fe2000/trunk.h>

/*
 *  Debugging output...
 */
#if 0 
#undef BCM_DEBUG
#define BCM_DEBUG(flags, stuff)   soc_cm_print stuff
#endif 
#define VLAN_OUT(flags, stuff)   BCM_DEBUG(flags | BCM_DBG_VLAN, stuff)
#define VLAN_WARN(stuff)         VLAN_OUT(BCM_DBG_WARN, stuff)
#define VLAN_ERR(stuff)          VLAN_OUT(BCM_DBG_ERR, stuff)
#define VLAN_VERB(stuff)         VLAN_OUT(BCM_DBG_VERBOSE, stuff)
#define VLAN_VVERB(stuff)        VLAN_OUT(BCM_DBG_VVERBOSE, stuff)
#if VLAN_EXCESS_VERBOSITY
#define VLAN_EVERB(stuff)        VLAN_OUT(BCM_DBG_VVERBOSE, stuff)
#else /* VLAN_EXCESS_VERBOSITY */
#define VLAN_EVERB(stuff)
#endif /* VLAN_EXCESS_VERBOSITY */
#if 0 
#define VLAN_FUNCMSG(string)    "%s[%d]%s" string, __FILE__, __LINE__, FUNCTION_NAME()
#define VLAN_FUNCMSG1(string)   "%s[%d]%s: " string, __FILE__, __LINE__, FUNCTION_NAME()
#else 
#define VLAN_FUNCMSG(string)    "%s[%d]%s" string, __FILE__, __LINE__, _VLAN_FUNC_NAME
#define VLAN_FUNCMSG1(string)   "%s[%d]%s: " string, __FILE__, __LINE__, _VLAN_FUNC_NAME
#endif 

#if (_SHR_PBMP_WORD_MAX > 1)
#if (_SHR_PBMP_WORD_MAX > 2)
#if (_SHR_PBMP_WORD_MAX > 3)
#define VLAN_PBMP_FORMAT "%08X %08X %08X %08X"
#define VLAN_PBMP_SHOW(pbmp) \
                             _SHR_PBMP_WORD_GET(pbmp,3), \
                             _SHR_PBMP_WORD_GET(pbmp,2), \
                             _SHR_PBMP_WORD_GET(pbmp,1), \
                             _SHR_PBMP_WORD_GET(pbmp,0)
#else /* (_SHR_PBMP_WORD_MAX > 3) */
#define VLAN_PBMP_FORMAT "%08X %08X %08X"
#define VLAN_PBMP_SHOW(pbmp) \
                             _SHR_PBMP_WORD_GET(pbmp,2), \
                             _SHR_PBMP_WORD_GET(pbmp,1), \
                             _SHR_PBMP_WORD_GET(pbmp,0)
#endif /* (_SHR_PBMP_WORD_MAX > 3) */
#else /* (_SHR_PBMP_WORD_MAX > 2) */
#define VLAN_PBMP_FORMAT "%08X %08X"
#define VLAN_PBMP_SHOW(pbmp) \
                             _SHR_PBMP_WORD_GET(pbmp,1), \
                             _SHR_PBMP_WORD_GET(pbmp,0)
#endif /* (_SHR_PBMP_WORD_MAX > 2) */
#else /* (_SHR_PBMP_WORD_MAX > 1) */
#define VLAN_PBMP_FORMAT "%08X"
#define VLAN_PBMP_SHOW(pbmp) \
                             _SHR_PBMP_WORD_GET(pbmp,0)
#endif /* (_SHR_PBMP_WORD_MAX > 1) */


/*
 *  Supported bcm_vlan_port flags for creation
 */
#define VLAN_SUPPORTED_VLAN_PORT_FLAGS (BCM_VLAN_PORT_REPLACE | \
                                        BCM_VLAN_PORT_WITH_ID | \
                                        BCM_VLAN_PORT_INNER_VLAN_PRESERVE | \
                                        BCM_VLAN_PORT_INNER_VLAN_ADD | \
                                        BCM_VLAN_PORT_ENCAP_WITH_ID | \
                                        BCM_VLAN_PORT_EGRESS_UNTAGGED)
/*
 *  bcm_vlan_port flags that are significant in comparisons
 */
#define VLAN_SIGNIFICANT_VLAN_PORT_FLAGS (BCM_VLAN_PORT_INNER_VLAN_PRESERVE | \
                                          BCM_VLAN_PORT_EGRESS_UNTAGGED)
#define VLAN_ING_SIGNIFICANT_VLAN_PORT_FLAGS (BCM_VLAN_PORT_INNER_VLAN_PRESERVE)
#define VLAN_EGR_SIGNIFICANT_VLAN_PORT_FLAGS (BCM_VLAN_PORT_EGRESS_UNTAGGED)

/*
 *  Making the code more readable but maybe less understandable
 */
#define VLAN_LOCK_TAKE \
    if (sal_mutex_take(_vlan_state[unit]->lock, sal_mutex_FOREVER)) { \
        /* Cound not obtain unit lock  */ \
        VLAN_ERR((VLAN_FUNCMSG1("unable to obtain VLAN lock on unit %d\n"), \
                  unit)); \
        return BCM_E_INTERNAL; \
    }
#define VLAN_LOCK_RELEASE \
    if (sal_mutex_give(_vlan_state[unit]->lock)) { \
        /* Could not release unit lock */ \
        VLAN_ERR((VLAN_FUNCMSG1("unable to release VLAN lock on unit %d\n"), \
                  unit)); \
        return BCM_E_INTERNAL; \
    }
#define VLAN_UNIT_VALID_CHECK \
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) { \
        VLAN_ERR((VLAN_FUNCMSG1("unit %d is not valid\n"), unit)); \
        return BCM_E_UNIT; \
    }
#define VLAN_UNIT_INIT_CHECK \
    if ((!(_primary_lock)) || (!(_vlan_state[unit]))) { \
        VLAN_ERR((VLAN_FUNCMSG1("unit %d not initialised\n"), unit)); \
        return BCM_E_INIT; \
    }

#define VLAN_CTL_SET(u, ctrl, v) \
    _vlan_state[(u)]->vlanControl =                        \
          ((_vlan_state[(u)]->vlanControl & ~(1 << ctrl)) |   \
          (!!(v) << ctrl));
#define VLAN_CTL_GET(u, ctrl) \
    !!(_vlan_state[(u)]->vlanControl & (1 << ctrl))


#define VLAN_GPORT_VALID(u, g) \
   (BCM_GPORT_IS_VLAN_PORT(g)                                         &&   \
    (VLAN_VGPORT_ID_TO_FT_INDEX(u, BCM_GPORT_VLAN_PORT_ID_GET(g) ) >=      \
     SBX_LOCAL_GPORT_FTE_BASE(u))                                    &&    \
    (VLAN_VGPORT_ID_TO_FT_INDEX(u, BCM_GPORT_VLAN_PORT_ID_GET(g) ) <=      \
     SBX_LOCAL_GPORT_FTE_END(u)))


/*
 *  This describes state in the egress translation entry lists.  It is also
 *  used to specify which fields to change in a translation entry.
 */
typedef enum _bcm_sbx_v_e_t_state_e {
    VLAN_TRANS_CHANGE_VID = 0x0001,   /* use specified destination VID */
    VLAN_TRANS_CHANGE_PRI = 0x0002    /* use specified destination priority */
} _bcm_sbx_v_e_t_state_t;

/*
 *  This structure is used for keeping track of egress table entries that are
 *  associated with egress translation.  We need this to be able to more
 *  efficiently use the egress table (without it, each translated port+vid
 *  would require its own egress table entry).
 */
typedef struct _bcm_sbx_vlan_egr_trans_s {
    unsigned int                     eteNum;   /* egress table entry number */
} _bcm_sbx_vlan_egr_trans_t;


/*
 *  This structure contains information necessary to map VLAN GPORT ID to
 *  logical port ID.
 */
typedef struct _bcm_fe2k_vgp_info_s {
    uint16 lpid[VLAN_MAX_VLAN_GPORTS];  /* map GPORT ID to logical port ID */
} _bcm_fe2k_vgp_info_t;

/*
 *  I need more state information for vlan logical ports than is provided at
 *  the BCM layer structure.  Thankfully, these additional data do not need to
 *  be propagated to other units.
 *
 *  WARNING: Order is important; do not reorder this structure carelessly!
 */
typedef enum _bcm_fe2k_vlan_gport_internal_flags_e {
    _BCM_FE2K_VLAN_GP_EXTERN_OHI = 0x0001     /* caller provided initial OHI */
} _bcm_fe2k_vlan_gport_internal_flags_t;
typedef struct _bcm_fe2k_vlan_gport_e_s {
    _bcm_fe2k_vlan_gport_internal_flags_t flags; /* internal flags */
} _bcm_fe2k_vlan_gport_e_t;
typedef struct _bcm_fe2k_vlan_gport_s {
    dq_t            nt;             /* link to next gport in trunk */
    bcm_vlan_port_t p;              /* BCM_VLAN_GPORT description structure */
    _bcm_fe2k_vlan_gport_e_t e;     /* Private additional data structure */
} _bcm_fe2k_vlan_gport_t;
typedef struct _bcm_fe2k_trunk_state_s {
    dq_t                  vlan_port_head;
    bcm_trunk_add_info_t  tinfo;
} _bcm_fe2k_trunk_state_t;
/*
 *  This structure contains the local state information for this module.
 *
 *  It also contains a locking device, on a per unit basis, to prevent more
 *  than one concurrent access to the local data per unit.
 *
 *  Note that the 'standard' tag and strip ETEs do not have reference counters
 *  or other data (see _bcm_sbx_vlan_egr_trans_t) since they are allocated
 *  dynamically but held for the duration, and are used in all 'standard' cases
 *  (no translation applied).
 *
 *  WARNING: 'init' does *NOT* indicate initialisation complete, it indicates
 *  initialisation *IN PROGRESS*.  It is used to disregard certain fields that
 *  always seem to contain random garbage during initialisation (such as the
 *  current ET index used for egrPortVlan2Etc table entries).  If it is not set
 *  back to FALSE once init has completed, the module will assume it is still
 *  in initialisation mode and will NEVER free certain resources.
 *
 *  'Dropping tagged frames' presently only describes the behaviour for single
 *  tagged frames, as the double tagged frames go through a different path.
 */

typedef struct _bcm_sbx_fe2000_vlan_state_s {
    sal_mutex_t               lock;           /* Used for locking these data */
    bcm_pbmp_t                tag_drop;       /* Dropping tagged frames */
    bcm_pbmp_t                untag_drop;     /* Dropping untagged frames */
    bcm_vlan_t                default_vid;    /* Default VID */
    unsigned int              init;           /* indicates init pending */
    _bcm_fe2k_vgp_info_t      gportInfo;      /* VLAN GPORT information */
    int                       vlanControl;    /* vlanControl bits set */
    int                       batch;          /* use batch soc apis */
    int                       batchDirty;     /* commit only dirty batches! */
    _bcm_fe2k_trunk_state_t   trunk[SBX_MAX_TRUNKS]; /* Track trunk
                                                      * membership changes
                                                      */

#ifdef BCM_FE2000_P3_SUPPORT
    _bcm_fe2k_nvid_pvv2e_control_flags_t nvflag[SBX_MAX_PORTS]; /* nvid data */
    _bcm_fe2k_vlan_divergence_t divergence[SBX_MAX_PORTS]; /* divergence */
#endif /* def BCM_FE2000_P3_SUPPORT */
    bcm_vlan_t                nvid[SBX_MAX_PORTS]; /* Port native VIDs */
} _bcm_sbx_fe2000_vlan_state_t;

/*
 *  This defines the modes of statistics that we can keep.  It's mapped during
 *  read/write so that we manipulate the correct entry in the stats pool, and
 *  so we know which stats are available.
 *
 *  Single:
 *     0 - frames and bytes for all frames that touch this VLAN
 *  Typed:
 *     0 - L2 unicast frames and bytes passed
 *     1 - L2 unicast frames and bytes dropped
 *     2 - L2 multicast frames and bytes passed
 *     3 - L2 multicast frames and bytes dropped
 *     4 - L2 flood frames and bytes passed
 *     5 - L2 flood frames and bytes dropped
 *     6 - L3 frames and bytes passed
 *     7 - L3 frames and bytes passed
 *  MEF Single:
 *     0 - DLFDrop (Frames and bytes dropped?)
 *     1 - unused
 *     2 - unused
 *     3 - DP 0 frames and bytes passed
 *     4 - DP 1 frames and bytes passed
 *     5 - DP 2 frames and bytes passed
 *  MEF Typed:
 *     0 - DLFDrop (Frames and bytes dropped?)
 *     1 - unused
 *     2 - unused
 *     3 - COS 0, DP 0 frames and bytes passed
 *     4 - COS 0, DP 1 frames and bytes passed
 *     5 - COS 0, DP 2 frames and bytes passed
 *     6 - COS 1, DP 0 frames and bytes passed
 *     7 - COS 1, DP 1 frames and bytes passed
 *     8 - COS 1, DP 2 frames and bytes passed
 *     9 - COS 2, DP 0 frames and bytes passed
 *     0 - COS 2, DP 1 frames and bytes passed
 *     1 - COS 2, DP 2 frames and bytes passed
 *     2 - COS 3, DP 0 frames and bytes passed
 *     3 - COS 3, DP 1 frames and bytes passed
 *     4 - COS 3, DP 2 frames and bytes passed
 *     5 - COS 4, DP 0 frames and bytes passed
 *     6 - COS 4, DP 1 frames and bytes passed
 *     7 - COS 4, DP 2 frames and bytes passed
 *     8 - COS 5, DP 0 frames and bytes passed
 *     9 - COS 5, DP 1 frames and bytes passed
 *     0 - COS 5, DP 2 frames and bytes passed
 *     1 - COS 6, DP 0 frames and bytes passed
 *     2 - COS 6, DP 1 frames and bytes passed
 *     3 - COS 6, DP 2 frames and bytes passed
 *     4 - COS 7, DP 0 frames and bytes passed
 *     5 - COS 7, DP 1 frames and bytes passed
 *     6 - COS 7, DP 2 frames and bytes passed
 *
 *  Note that SBX layer considers counters in pairs -- each pair includes a
 *  byte count and a frame count.  These pairs are split into separate byte and
 *  frame counts at this layer, per the BCM API, but are still tracked as pairs
 *  for the purposes of managing and accessing them.
 *
 *  Also note that COS is internal COS, not QoS or Pri or Exp or DSCP as such
 *  (these are mapped to the internal COS).  It also appears that DLFDrop is
 *  all frames and bytes dropped for any reason (by policer or FTE, at least).
 *
 *  SBX layer and microcode considers DP0==Green, DP1==Yellow, DP2==Red.  We
 *  therefore follow that convention here when mapping colours.
 */
typedef enum _fe2000VlanStatsType_e {
    _fe2000VlanStatsTypeSingle,   /* counter layout per 'single' in comment */
    _fe2000VlanStatsTypeTyped,    /* counter layout per 'typed' in comment */
    _fe2000VlanStatsTypeMefSingle,/* counter layout per 'mefSingle' above */
    _fe2000VlanStatsTypeMefTyped, /* counter layout per 'mefTyped above */
    _fe2000VlanStatsTypeUnknown   /* counter layout unknown/unverified */
} _fe2000VlanStatsType_t;


/*
 *  This module uses some resources on the chip and considers them private.
 *
 *  FT entries      0..4095
 *
 *  MCGroups        0..4095
 *
 *  pVid2Etc        vid 0..4095 for ports in PBMP_ALL
 *
 *  fabric queues   208..215 (maybe shared, but must be there)
 *
 *  vlan2Etc        0..4095
 *
 *  routedVlan2Etc  0..4095
 *
 *  mcPort2Etc      group 0..4095 for ports in PBMP_ALL
 *
 *  egrPVid2Etc     VIDs 0..4095 for ports in PBMP_ALL (maybe?)
 *
 *  egrVlanPort2Etc VIDs 0..4095 for ports in PBMP_ALL
 *
 *  MC Vectors      0..4095
 *
 *  Note that this module only does VLAN and flooding; individual host
 *  connections are managed through the L2 module.
 */


extern int running_simulator;

/*
 *  Access to functions in other sbx:fe2000 modules...
 */
extern int bcm_fe2000_stg_default_get(int unit, bcm_stg_t *stg_ptr);
extern int bcm_fe2000_stg_vlan_add(int unit, bcm_stg_t stg, bcm_vlan_t vid);
extern int bcm_fe2000_stg_vlan_remove(int unit, bcm_stg_t stg, bcm_vlan_t vid);
extern int bcm_fe2000_stg_stp_get(int unit,
                                  bcm_stg_t stg,
                                  bcm_port_t port,
                                  int *stp_state);
extern int bcm_fe2000_stg_stp_set(int unit,
                                  bcm_stg_t stg,
                                  bcm_port_t port,
                                  int stp_state);
extern int bcm_fe2000_stg_vlan_list(int unit,
                                    bcm_stg_t stg,
                                    bcm_vlan_t **list,
                                    int *count);
extern int bcm_fe2000_stg_vlan_list_destroy(int unit,
                                            bcm_vlan_t *list,
                                            int count);
extern int _bcm_fe2000_stg_vlan_remove(int unit,
                                       bcm_stg_t stg,
                                       bcm_vlan_t vid,
                                       int destroy);
extern int _bcm_fe2000_stg_vlan_port_add(int unit,
                                         bcm_vlan_t vid,
                                         bcm_port_t port);
extern int _bcm_fe2000_stg_vlan_port_remove(int unit,
                                            bcm_vlan_t vid,
                                            bcm_port_t port);

extern int bcm_fe2000_port_untagged_vlan_set(int unit,
                                             bcm_port_t port,
                                             bcm_vlan_t vid);
extern int bcm_fe2000_port_untagged_vlan_get(int unit,
                                             bcm_port_t port,
                                             bcm_vlan_t *vid_ptr);

extern int _bcm_fe2000_qos_map_validate(int unit,
                                        int qosMapId,
                                        uint32 *qosMapRaw);

/*
 *  This is the static array containing pointers to the information structure
 *  described right above.
 */
static _bcm_sbx_fe2000_vlan_state_t *_vlan_state[BCM_MAX_NUM_UNITS];

/*
 *  This locks the table above during the rare intervals when it needs it.
 */
static volatile sal_mutex_t _primary_lock = NULL;

/*
 *  This is used for range checking and operations involving all VIDs.
 *  Putting it here (instead of a #define) costs performance, but...
 */
unsigned int _sbx_vlan_max_vid_value = VLAN_MAX_VID_VALUE;

static int
_bcm_fe2000_vp_match_configure(int unit, _bcm_fe2k_vlan_gport_t *vlanPortData,
                               bcm_port_t tgtPort, bcm_vlan_t vsi, uint32 lpi);

STATIC int
_bcm_fe2000_g2p3_vlan_port_vlan_vector_update(int               unit,
                                              bcm_vlan_port_t  *vlan_port,
                                              bcm_port_t        phy_port,
                                              int               create);

int
_bcm_fe2000_g2p3_vlan_port_vlan_vector_set(int unit,
                                           bcm_vlan_vector_t vlan_vec,
                                           int my_modid,
                                           bcm_vlan_port_t *vlanPort,
                                           bcm_port_t phyPort, bcm_vlan_t vpn);

/*
 *  Local structures, used to reduce the number of odd things that need to
 *  be passed around to a smaller number of unions of associated odd things.
 */
/* forwarding entry (flooding) for a VLAN */
typedef union _bcm_sbx_vlan_ft_u {
#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_ft_t p3ft;
#endif /* def BCM_FE2000_P3_SUPPORT */
} _bcm_sbx_vlan_ft_t;


void
_bcm_fe2000_vlan_dump_state(int unit) 
{
    bcm_port_t port;
    BCM_DEBUG(0,("VLAN Data:\n  default_vid = 0x%03x\n"
                 "  tag_drop = " VLAN_PBMP_FORMAT "\n"
                 "untag_drop = " VLAN_PBMP_FORMAT "\n",
                 _vlan_state[unit]->default_vid,
                 VLAN_PBMP_SHOW(_vlan_state[unit]->tag_drop),
                 VLAN_PBMP_SHOW(_vlan_state[unit]->untag_drop)));
              
    BCM_DEBUG(0,("per-port internal VLAN data:\n"));
    BCM_DEBUG(0,("Port nativeVid   nvflags   divergence\n"));
    BCM_DEBUG(0,("----+---------+-----------+-----------\n"));
    PBMP_ALL_ITER(unit, port) {
        BCM_DEBUG(0,("%4d  0x%03x     0x%08x 0x%08x\n",
                     port, _vlan_state[unit]->nvid[port],
                     _vlan_state[unit]->nvflag[port], 
                     _vlan_state[unit]->divergence[port]));
    }

    PBMP_ALL_ITER(unit, port) {
        int flag = _vlan_state[unit]->nvflag[port];
        
        BCM_DEBUG(0,("nvflag[%d]=", port));
        if (flag & BCM_FE2K_NVID_NVID_OVR_MASK) {
            BCM_DEBUG(0, ("OVR_MASK "));
        }
        if (flag & BCM_FE2K_NVID_OVERRIDE_PVV2E) {
            BCM_DEBUG(0, ("OVERRIDE_PVV2E "));
        }
        if (flag & BCM_FE2K_NVID_USE_PVV2E) {
            BCM_DEBUG(0, ("USE_PVV2E "));
        }
        if (flag & BCM_FE2K_NVID_SET_REPLACE) {
            BCM_DEBUG(0, ("SET_REPLACE "));
        }
        if (flag & BCM_FE2K_NVID_SET_KEEPORSTRIP) {
            BCM_DEBUG(0, ("SET_KEEPORSTRIP "));
        }
        if (flag & BCM_FE2K_NVID_USE_NVID_OVR) {
            BCM_DEBUG(0, ("USE_NVID_OVR "));
        }
        if (flag & BCM_FE2K_NVID_SET_UNTAGGEDSTRIP) {
            BCM_DEBUG(0, ("SET_UNTAGGEDSTRIP "));
        }
        BCM_DEBUG(0,("\n"));
    }
}

#ifdef BCM_FE2000_P3_SUPPORT
/*
 *   Function
 *      _i_soc_sbx_g2p3_pv2e_set
 *   Purpose
 *      Writes a pv2e entry, mirroring to entry 0 if the VID is the native VID
 *      for that port, and adjusting the original VSI as appropriate if the
 *      port is in dropTagged mode, and adjusting the mirror copy according to
 *      dropUntagged mode if making the mirror copy.
 *
 *      Does not apply droptagged or dropuntagged to VSI outside of traditional
 *      bridging range when writing.
 *
 *      Does not perform locking or other checks.
 *
 *      Replicates non-diverged LP data from the native VID to the zero VID if
 *      the native VID LP and zero VID LP have diverged and the write is to the
 *      native VID.
 *
 *      Replicates non-diverged LP data from the zero VID to the native VID if
 *      the native VID LP and zero VID LP have diverged and the write is to the
 *      zero VID.
 *
 *      When in batch mode, the routine to update all tables without commiting
 *      to HW.  When in batch mode, committing to hw is then the caller's
 *      responsibilty.
 */

#define _VLAN_FUNC_NAME "_i_soc_sbx_g2p3_pv2e_set"
static int
_i_soc_sbx_g2p3_pv2e_set(int unit,
                         int iport,
                         int ivid,
                         soc_sbx_g2p3_pv2e_t *e)
{
    int result;
    soc_sbx_g2p3_p2e_t p2e;
    soc_sbx_g2p3_pv2e_t pv2e;
    soc_sbx_g2p3_pvv2e_t pvv2e;
    soc_sbx_g2p3_lp_t lpSrc;
    soc_sbx_g2p3_lp_t lpDst;
    uint32 lpiSrc;
    uint32 lpiDst;
    bcm_vlan_t vidSrc;
    bcm_vlan_t vidDst;
    int needPvvEntry = 0;
    _bcm_fe2k_nvid_pvv2e_control_flags_t flags;
    _bcm_fe2k_vlan_divergence_t divergence;

    if (_vlan_state[unit] && (!(_vlan_state[unit]->init))) {
        VLAN_EVERB((VLAN_FUNCMSG("(%d,%d,%03X,&"
                                 "(vsi=%04X,lp=%08X,stpdstate=%d))\n"),
                    unit,
                    iport,
                    ivid,
                    e->vlan,
                    e->lpi,
                    e->stpstate));
    }

    if (((iport) < 0) || ((iport) >= SBX_MAX_PORTS)) {
        VLAN_ERR((VLAN_FUNCMSG1("port %d invalid on unit %d\n"),
                  iport,
                  unit));
        return BCM_E_PARAM;
    }

    /*
     *  First order of business is to set the requested entry.  We can clean up
     *  other issues and do replication after this is done, particularly since
     *  this is the more likely case and should therefore be the relatively
     *  optimised case.
     */
    pv2e = *e;
    /* maybe condition the pv2e write */
    if ((BCM_PBMP_MEMBER(_vlan_state[unit]->tag_drop, iport)) &&
        (e->vlan <= BCM_VLAN_MAX) &&
        (ivid > 0)) {
        /*
         *  The port is in drop tagged mode, target VSI is in the traditional
         *  bridging range, and we are trying to adjust a tagged pv2e entry;
         *  this VID's target VSI must be drop, so set it to zero.
         */
        pv2e.vlan = 0;
        VLAN_EVERB((VLAN_FUNCMSG1("writing with VSI 0000 since unit %d port %d"
                                  " is in drop tagged mode and VID nonzero\n"),
                    unit,
                    iport));
    } else if ((BCM_PBMP_MEMBER(_vlan_state[unit]->untag_drop, iport)) &&
               (e->vlan <= BCM_VLAN_MAX) &&
               (0 == ivid)) {
        /*
         *  The port is in drop untagged mode, target VSI is in the traditional
         *  bridging range, and we are trying to adjust the untagged pv2e
         *  entry; this VID's target VSI must be drop, so set it to zero.
         */
        pv2e.vlan = 0;
        VLAN_EVERB((VLAN_FUNCMSG1("writing with VSI 0000 since unit %d port %d"
                                  " is in drop untagged mode and VID zero\n"),
                    unit,
                    iport));
    }
    /* commit the pv2e write */
    result = SOC_SBX_G2P3_PV2E_SET(unit, iport, ivid, &pv2e);
    if (BCM_E_NONE != result) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:pv2e[%d,%03X]:"
                                " %d (%s)\n"),
                  unit,
                  iport,
                  ivid,
                  result,
                  _SHR_ERRMSG(result)));
        return result;
    }

    /*
     *  If VID is nonzero and not native VID
     */
    if (((0 < ivid) && (ivid != _vlan_state[unit]->nvid[iport]))) {
        return result;
    }

    /*
     *  Replicate writes from zero to native or native to zero as appropriate.
     *
     *  Need to also make adjustment for VSI here if writing TB VSI and drop
     *  tagged (target is native) or drop untagged (target is zero) set.
     *
     *  If any divergence, must copy LP contents that have not diverged; if
     *  complete divergence, no LP contents need to be copied.
     */
    divergence = _vlan_state[unit]->divergence[iport];
    vidSrc = ivid;
    vidDst = ivid?0:_vlan_state[unit]->nvid[iport];
    if (divergence & _FE2K_VLAN_DIVERGE_ALL) {
        /* at least some divergence; need to preserve old lpi */
        VLAN_EVERB((VLAN_FUNCMSG1("unit %d port %d native/untagged"
                                  " divergence %08X\n"),
                    unit,
                    iport,
                    divergence));
        result = SOC_SBX_G2P3_PV2E_GET(unit,
                                       iport,
                                       vidDst,
                                       &pv2e);
        if (BCM_E_NONE != result) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:pv2e[%d,%03X]:"
                                    " %d (%s)\n"),
                      unit,
                      iport,
                      vidDst,
                      result,
                      _SHR_ERRMSG(result)));
            return result;
        }
        /* fill in the new pv2e properly */
        lpiSrc = e->lpi?e->lpi:iport;
        lpiDst = pv2e.lpi?pv2e.lpi:iport;
        pv2e = *e;
        pv2e.lpi = lpiDst;
        if (_FE2K_VLAN_DIVERGE_ALL !=
            (divergence & _FE2K_VLAN_DIVERGE_ALL)) {
            /* not completely diverged; get new lp data */
            result = soc_sbx_g2p3_lp_get(unit, lpiSrc, &lpSrc);
            if (BCM_E_NONE != result) {
                VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:lp[%08X]:"
                                        " %d (%s)\n"),
                          unit,
                          lpiSrc,
                          result,
                          _SHR_ERRMSG(result)));
                return result;
            }
            /* get old lp data */
            result = soc_sbx_g2p3_lp_get(unit, lpiDst, &lpDst);
            if (BCM_E_NONE != result) {
                VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:lp[%08X]:"
                                        " %d (%s)\n"),
                          unit,
                          lpiDst,
                          result,
                          _SHR_ERRMSG(result)));
                return result;
            }
            /* keep diverged data */
            if (divergence & _FE2K_VLAN_DIVERGE_QOS) {
                lpSrc.qos = lpDst.qos;
            }
            
            /* copy new data */
            lpDst = lpSrc;
            /* now write the updated data */
            result = soc_sbx_g2p3_lp_set(unit, lpiDst, &lpDst);
            if (BCM_E_NONE != result) {
                VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:lp[%08X]:"
                                        " %d (%s)\n"),
                          unit,
                          lpiDst,
                          result,
                          _SHR_ERRMSG(result)));
                return result;
            }
        } /* if (not complete divergence) */
    } else { /* if (any divergence on this port) */
        pv2e = *e;
    } /* if (any divergence on this port) */

    /* check dropTagged/dropUntagged */
    if ((vidDst && BCM_PBMP_MEMBER(_vlan_state[unit]->tag_drop, iport)) ||
        ((!vidDst) && BCM_PBMP_MEMBER(_vlan_state[unit]->untag_drop,
                                       iport))) {

        VLAN_EVERB((VLAN_FUNCMSG1("clearing vsi %03X; dst %03X\n"),
                    pv2e.vlan, vidDst));

        /*
         *  Either: copying to zero VID and drop untagged, or
         *  copying to nonzero VID and drop tagged.  Send to drop VSI.
         */
        pv2e.vlan = 0;

        /* clear the native vid flags to remove any pvv2e entries
         * when dropping, but only if not diverging
         */
        if (!(divergence & _FE2K_VLAN_DIVERGE_ALL)) {
            VLAN_EVERB((VLAN_FUNCMSG1("clearing pvv2e reflection flags %08X\n"),
                        _vlan_state[unit]->nvflag[iport]));

            _vlan_state[unit]->nvflag[iport] = 0;
        }
    } else if (!vidDst &&
               BCM_PBMP_MEMBER(_vlan_state[unit]->tag_drop, iport)) {
        if (e->vlan) {
            pv2e.vlan = e->vlan;
            VLAN_EVERB((VLAN_FUNCMSG1("Setting nativeVid while in "
                                      " drop_tag mode, force VSI to"
                                      " given VSI: %03X\n"),
                        pv2e.vlan));
        } else {
            pv2e.vlan = _vlan_state[unit]->nvid[iport];
            VLAN_EVERB((VLAN_FUNCMSG1("Setting nativeVid while in "
                                      " drop_tag mode, force VSI to"
                                      " NativeVid==VSI: %03X\n"),
                        pv2e.vlan));
        }
    }

    /* now commit the reflected native VID pv2e */
    result = SOC_SBX_G2P3_PV2E_SET(unit,
                                   iport,
                                   vidDst,
                                   &pv2e);

    if (BCM_E_NONE != result) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:pv2e[%d,%03X]:"
                                " %d (%s)\n"),
                  unit,
                  iport,
                  _vlan_state[unit]->nvid[iport],
                  result,
                  _SHR_ERRMSG(result)));
        return result;
    }

    /*
     *  We need to update the special pvv2e[port,0,FFF] entry.  Normally, a
     *  specific entry is obligatory if the port is not in dropUntagged mode
     *  and the port's defStrip setting is FALSE, but it is also possible that
     *  some other module needs to override this behaviour, so we manage that
     *  here as well.
     */
    flags = _vlan_state[unit]->nvflag[iport];
    VLAN_EVERB((VLAN_FUNCMSG1("updated pv2e for native VID or VID 0;"
                              " pvv2e reflection flags %08X\n"),
                flags));
    if (flags & BCM_FE2K_NVID_OVERRIDE_PVV2E) {
        /* some other module needs to override default pvv2e entry */
        if (flags & BCM_FE2K_NVID_USE_PVV2E) {
            /* other module needs a pvv2e entry */
            needPvvEntry = 2;
        } else {
            /* other module needs NO pvv2e entry */
            needPvvEntry = 0;
        }
    } else { /* if (flags & BCM_FE2K_NVID_OVERRIDE_PVV2E) */
        /*
         *  Must check port mode.  If port is not in drop untagged mode,
         *  and p2e[port].defstrip is FALSE, we will need a special pvv2e
         *  entry.
         *
         *  This special pvv2e entry MUST NOT exist in any other case.
         */
        if (!(BCM_PBMP_MEMBER(_vlan_state[unit]->untag_drop, iport))) {
            /* okay, we're not in drop untagged; check defStrip */
            result = soc_sbx_g2p3_p2e_get(unit, iport, &p2e);
            if (BCM_E_NONE == result) {
                /* set need of pvv2e entry according to defstrip */
                /* and only if the port is either provider or customer mode */
                if ((p2e.customer || p2e.provider) && !(p2e.defstrip)) {
                    needPvvEntry = 1;
                }
            } else {
                /* failed to read; assume no need for pvv2e entry */
                needPvvEntry = 0;
                VLAN_ERR((VLAN_FUNCMSG1("unable to get %d:p2e[%d]:"
                                        " %d (%s)\n"),
                          unit,
                          iport,
                          result,
                          _SHR_ERRMSG(result)));
                /*
                 *  Unfortunately, some ports don't have p2e table but do
                 *  have pv2e and pvv2e tables, so ignore the error
                 */
                result = BCM_E_NONE;
            }
        } else { /* if (port is not in drop untagged mode) */
            needPvvEntry = 0;
        } /* if (port is not in drop untagged mode) */
    } /* if (flags & BCM_FE2K_NVID_OVERRIDE_PVV2E) */

    if (0 != needPvvEntry) {
        /* make sure we have current VID 0 information */
        result = SOC_SBX_G2P3_PV2E_GET(unit, iport, 0, &pv2e);
        if (BCM_E_NONE != result) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:pv2e[%d,%03X]:"
                                    " %d (%s)\n"),
                      unit,
                      iport,
                      0,
                      result,
                      _SHR_ERRMSG(result)));
            return result;
        }
        /* need pvv2e[port,0,FFF] entry */
        soc_sbx_g2p3_pvv2e_t_init(&pvv2e);
        /* copy the data from the pv2e entry */
        pvv2e.lpi = pv2e.lpi;
        pvv2e.vlan = pv2e.vlan;
        pvv2e.stpstate = pv2e.stpstate;

        if (SOC_SBX_CFG(unit)->mplstp_ena) {
            pvv2e.vpws = pv2e.vpws;
        }

        /* fill in additional data */
        switch (needPvvEntry) {
        case 2:
            /* need to have custom pvv2e entry */
            if (flags & BCM_FE2K_NVID_USE_NVID_OVR) {
                pvv2e.vid = flags & BCM_FE2K_NVID_NVID_OVR_MASK;
            } else {
                pvv2e.vid = _vlan_state[unit]->nvid[iport];
            }
            pvv2e.replace = (0 != (flags & BCM_FE2K_NVID_SET_REPLACE));
            pvv2e.keeporstrip = (0 != (flags &
                                       BCM_FE2K_NVID_SET_KEEPORSTRIP));
            pvv2e.untagged_strip = !!(flags & BCM_FE2K_NVID_SET_UNTAGGEDSTRIP);
            break;
        case 1:
            /* need to have standard one with replace set */
            pvv2e.vid = _vlan_state[unit]->nvid[iport];
            pvv2e.replace = TRUE;
            pvv2e.keeporstrip = FALSE;
            pvv2e.untagged_strip = !!p2e.provider;
            break;
        }

        if (BCM_E_NONE == result) {
            VLAN_EVERB((VLAN_FUNCMSG1("%s create/update (%d)"
                                      " %d:pvv2e[%d,%03X,%03X] ="
                                      " ((lp=%08X,vlan=%04X,stpstate=%d),"
                                      "vid=%03X,r=%s,kos=%s,ustrip=%d)\n"),
                        _vlan_state[unit]->batch ? "batch" : "immediate",
                        needPvvEntry, unit,
                        iport, 0, 0xFFF, pvv2e.lpi, pvv2e.vlan,
                        pvv2e.stpstate, pvv2e.vid,
                        pvv2e.replace?"TRUE":"FALSE",
                        pvv2e.keeporstrip?"TRUE":"FALSE",
                        pvv2e.untagged_strip));

            if (_vlan_state[unit]->batch) {
                _vlan_state[unit]->batchDirty = 1;
                result = soc_sbx_g2p3_pvv2e_renew(unit, 0xFFF, 0x000,
                                                  iport,  &pvv2e);

                if (result == SOC_E_NOT_FOUND) {
                    result = soc_sbx_g2p3_pvv2e_add (unit, 0xFFF, 0,
                                                     iport, &pvv2e);
                }
            } else {
                result = soc_sbx_g2p3_pvv2e_set(unit, 0xFFF, 0x000,
                                                iport,  &pvv2e);
            }
        }

    } else { /* if (0 != needPvvEntry) */
        /* at least one of {drop tagged; defStrip}; no pvv2e entry */
        VLAN_EVERB((VLAN_FUNCMSG1("destroy"
                                  " %d:pvv2e[%d,%03X,%03X]\n"),
                    unit,
                    iport,
                    0,
                    0xFFF));
        if (_vlan_state[unit]->batch) {
            result = soc_sbx_g2p3_pvv2e_remove(unit, 0xFFF, 0x000, iport);

            if (BCM_SUCCESS(result)) {
                _vlan_state[unit]->batchDirty = 1;
            }
        } else {
            result = soc_sbx_g2p3_pvv2e_delete(unit, 0xFFF, 0x000, iport);
        }

        if (BCM_E_NOT_FOUND == result) {
            /*
             *  Don't really care that we couldn't delete the pvv2e
             *  entry if it's not here to be deleted...
             */
            result = BCM_E_NONE;
        }
    } /* if (needPvvEntry) */

    if (BCM_E_NONE != result) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to %s %d:pvv2e[%d,%03X,%03X]:"
                                " %d (%s)\n"),
                  needPvvEntry?"create/update":"destroy",
                  unit,
                  iport,
                  0xFFF,
                  0x000,
                  result,
                  _SHR_ERRMSG(result)));
    }

    return result;
}
#undef _VLAN_FUNC_NAME
#endif /* def BCM_FE2000_P3_SUPPORT */

#ifdef BCM_FE2000_P3_SUPPORT
/*
 *   Function
 *      _soc_sbx_g2p3_pv2e_set
 *   Purpose
 *      Writes a pv2e entry, mirroring to entry 0 if the VID is the native VID
 *      for that port, and adjusting the original VSI as appropriate if the
 *      port is in dropTagged mode, and adjusting the mirror copy according to
 *      dropUntagged mode if making the mirror copy.
 *
 *      Does not apply droptagged or dropuntagged to VSI outside of traditional
 *      bridging range when writing.
 *
 *      Does not perform locking or other checks.
 *
 *      Replicates non-diverged LP data from the native VID to the zero VID if
 *      the native VID LP and zero VID LP have diverged and the write is to the
 *      native VID.
 *
 *      Replicates non-diverged LP data from the zero VID to the native VID if
 *      the native VID LP and zero VID LP have diverged and the write is to the
 *      zero VID.
 */
#define _VLAN_FUNC_NAME "_soc_sbx_g2p3_pv2e_set"
int
_soc_sbx_g2p3_pv2e_set(int unit,
                       int iport,
                       int ivid,
                       soc_sbx_g2p3_pv2e_t *e)
{
    int result;

    if (!e) {
        VLAN_ERR((VLAN_FUNCMSG1("NULL pointer to pv2e entry data\n")));
        return BCM_E_PARAM;
    }

    /* display warning if trying to manipulate VID 0 directly */
    if (0 == ivid) {
        VLAN_WARN((VLAN_FUNCMSG1("direct write %d:pv2e[%d,000] ="
                                 " {vsi=%04X,lp=%08X,stpstate=%d}\n"),
                   unit,
                   iport,
                   e->vlan,
                   e->lpi,
                   e->stpstate));
    }

    /* Check for proper initialisation */
    if ((!(_primary_lock)) || (!(_vlan_state[unit]))) {
        return SOC_SBX_G2P3_PV2E_SET(unit, iport, ivid, e);
    }

    /* Claim the lock for VID work on this unit */
    if (sal_mutex_take(_vlan_state[unit]->lock, sal_mutex_FOREVER)) {
        /* Cound not obtain unit lock  */
        VLAN_ERR((VLAN_FUNCMSG1("unable to obtain VLAN lock for unit %d\n"),
                  unit));
        return SOC_SBX_G2P3_PV2E_SET(unit, iport, ivid, e);
    }

    result = _i_soc_sbx_g2p3_pv2e_set(unit, iport, ivid, e);

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    return result;
}
#undef _VLAN_FUNC_NAME
#endif /* def BCM_FE2000_P3_SUPPORT */

#ifdef BCM_FE2000_P3_SUPPORT
/*
 *   Function
 *      _bcm_fe2000_lp_get_by_pv
 *   Purpose
 *      Gets a logical port, but does so by checking the pv2e.  Will fall back
 *      to the implied logical port for a port if the specified pv2e does not
 *      have an assigned logical port.
 *   Notes
 *      Assumes lock taken; little parameter checking.
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_lp_get_by_pv"
static int
_bcm_fe2000_lp_get_by_pv(int unit,
                         int iport,
                         int ivid,
                         soc_sbx_g2p3_lp_t *lpData)
{
    soc_sbx_g2p3_pv2e_t pv2e;
    int result;

    if (BCM_VLAN_INVALID > ivid) {
        /* get pv2e so we can find it's lpi */
        result = SOC_SBX_G2P3_PV2E_GET(unit, iport, ivid, &pv2e);
        if (BCM_E_NONE != result) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:pv2e[%d,%03X]: %d (%s)\n"),
                      unit,
                      iport,
                      ivid,
                      result,
                      _SHR_ERRMSG(result)));
            return result;
        }
    } else {
        pv2e.lpi = 0;
    }

    /* read the appropriate logical port */
    result = soc_sbx_g2p3_lp_get(unit, pv2e.lpi?pv2e.lpi:iport, lpData);
    if (BCM_E_NONE != result) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:lp[%08X]: %d (%s)\n"),
                  unit,
                  pv2e.lpi?pv2e.lpi:iport,
                  result,
                  _SHR_ERRMSG(result)));
    }

    return result;
}
#undef _VLAN_FUNC_NAME
#endif /* def BCM_FE2000_P3_SUPPORT */

#ifdef BCM_FE2000_P3_SUPPORT
/*
 *   Function
 *      _bcm_fe2000_lp_set_by_pv
 *   Purpose
 *      Sets a logical port, but does so by checking the pv2e.  Will fall back
 *      to the implied logical port for a port if the specified pv2e does not
 *      have an assigned logical port.
 *
 *      Replicates non-diverged LP data from the native VID to the zero VID if
 *      the native VID LP and zero VID LP have diverged and the write is to the
 *      native VID.
 *
 *      Replicates non-diverged LP data from the zero VID to the native VID if
 *      the native VID LP and zero VID LP have diverged and the write is to the
 *      zero VID.
 *   Notes
 *      Assumes lock taken; little parameter checking.
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_lp_set_by_pv"
static int
_bcm_fe2000_lp_set_by_pv(int unit,
                         int iport,
                         int ivid,
                         soc_sbx_g2p3_lp_t *lpData)
{
    soc_sbx_g2p3_pv2e_t pv2e;
    int result;

    if (BCM_VLAN_INVALID > ivid) {
        /* get the pv2e entry so we know its lpi */
        result = SOC_SBX_G2P3_PV2E_GET(unit, iport, ivid, &pv2e);
        if (BCM_E_NONE != result) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:pv2e[%d,%03X]: %d (%s)\n"),
                      unit,
                      iport,
                      ivid,
                      result,
                      _SHR_ERRMSG(result)));
            return result;
        }

        /* get the vsi from the naive vlan if in drop tag mode, and this is
         * the native vid, of course, it really doesn't make sense to set
         * anyother vid when in tag_drop mode, but validate anyway
         */
        if (ivid == _vlan_state[unit]->nvid[iport] &&
            BCM_PBMP_MEMBER(_vlan_state[unit]->tag_drop, iport))
        {
            soc_sbx_g2p3_pv2e_t npv2e;
            result = SOC_SBX_G2P3_PV2E_GET(unit, iport, 0, &npv2e);

            if (BCM_E_NONE != result) {
                VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:pv2e[%d,%03X]: "
                                        "%d (%s)\n"),
                          unit, iport, 0, result, _SHR_ERRMSG(result)));
                return result;
            }
            pv2e.vlan = npv2e.vlan;
        }

    } else {
        pv2e.lpi = 0;
    }

    /* write the new data to the logicalPort */
    result = soc_sbx_g2p3_lp_set(unit, pv2e.lpi?pv2e.lpi:iport, lpData);
    if (BCM_E_NONE != result) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:lp[%08X]: %d (%s)\n"),
                  unit,
                  pv2e.lpi?pv2e.lpi:iport,
                  result,
                  _SHR_ERRMSG(result)));
        return result;
    }

    if (BCM_VLAN_INVALID > ivid) {
        /* reflect the write appropriately */
        result = _i_soc_sbx_g2p3_pv2e_set(unit, iport, ivid, &pv2e);
        if (BCM_E_NONE != result) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to mirror %d:pv2e[%d,%03X]:"
                                    " %d (%s)\n"),
                      unit,
                      iport,
                      ivid,
                      result,
                      _SHR_ERRMSG(result)));
        }
    }

    return result;
}
#undef _VLAN_FUNC_NAME
#endif /* def BCM_FE2000_P3_SUPPORT */

#ifdef BCM_FE2000_P3_SUPPORT
/*
 *   Function
 *      _bcm_fe2000_lp_set_by_pv_and_diverge
 *   Purpose
 *      Adds logical port divergence between the native VID and the zero VID on
 *      a given port, if writing to the zero VID and some divergence is
 *      indicated. This divergence will allocate (if not already diverged) a
 *      new logicalPort ID and assign it to the zero VID, copying the
 *      nondiverged fields from the native VID's logicalPort.
 *
 *      This does not manage any other resources; caller must have already set
 *      up new resources for diverged logical port as appropriate.  All
 *      diverged resources for zero VID are considered to be owned by the zero
 *      VID and will be freed when reconverging.
 *
 *      The provided LP data are those for the zero VID; the native VID.  This
 *      will reflect any nondiverged fields back to the native VID, so the
 *      logical port data must have originally been read from the native VID.
 *
 *      Currently, the only way to reconverge is to set the native VID.
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_lp_set_by_pv_and_diverge"
static int
_bcm_fe2000_lp_set_by_pv_and_diverge(int unit,
                                     int iport,
                                     int ivid,
                                     _bcm_fe2k_vlan_divergence_t divergence,
                                     soc_sbx_g2p3_lp_t *lpData)
{
    soc_sbx_g2p3_pv2e_t pv2e;
    int result;

    /* only diverge if writing VID 0 and some divergence is requested */
    if ((!(divergence & _FE2K_VLAN_DIVERGE_ALL)) ||
        (0 != ivid) ||
        (BCM_VLAN_MAX <= ivid)) {
        return _bcm_fe2000_lp_set_by_pv(unit, iport, ivid, lpData);
    }

    /* get the pv2e for zero VID so we know its lpi */
    result = SOC_SBX_G2P3_PV2E_GET(unit, iport, ivid, &pv2e);
    if (BCM_E_NONE != result) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:pv2e[%d,%03X]: %d (%s)\n"),
                  unit,
                  iport,
                  ivid,
                  result,
                  _SHR_ERRMSG(result)));
        return result;
    }

    /* if not diverged yet, we'll need a new logicalport */
    if (!(_vlan_state[unit]->divergence[iport] & _FE2K_VLAN_DIVERGE_ALL)) {
        VLAN_VVERB((VLAN_FUNCMSG1("diverging unit %d port %d VID 000\n"),
                    unit,
                    iport));
        result = _sbx_gu2_resource_alloc(unit,
                                         SBX_GU2K_USR_RES_LPORT,
                                         1,
                                         &(pv2e.lpi),
                                         0);
        if (BCM_E_NONE != result) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to allocate logicalport on"
                                    " unit %d: %d (%s)\n"),
                      unit,
                      result,
                      _SHR_ERRMSG(result)));
            return result;
        }
    }

    /* write the new data to the logicalPort */
    result = soc_sbx_g2p3_lp_set(unit, pv2e.lpi?pv2e.lpi:iport, lpData);
    if (BCM_E_NONE != result) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:lp[%08X]: %d (%s)\n"),
                  unit,
                  pv2e.lpi?pv2e.lpi:iport,
                  result,
                  _SHR_ERRMSG(result)));
        return result;
    }

    /* add this divergence */
    _vlan_state[unit]->divergence[iport] |= (divergence &
                                             _FE2K_VLAN_DIVERGE_ALL);

    /* reflect the write appropriately */
    result = _i_soc_sbx_g2p3_pv2e_set(unit, iport, ivid, &pv2e);
    if (BCM_E_NONE != result) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to mirror %d:pv2e[%d,%03X]:"
                                " %d (%s)\n"),
                  unit,
                  iport,
                  ivid,
                  result,
                  _SHR_ERRMSG(result)));
    }

    return result;
}
#undef _VLAN_FUNC_NAME
#endif /* def BCM_FE2000_P3_SUPPORT */

#ifdef BCM_FE2000_P3_SUPPORT
/*
 *   Function
 *      bcm_fe2000_lp_get_by_pv
 *   Purpose
 *      Gets a logical port, but does so by checking the pv2e.  Will fall back
 *      to the implied logical port for a port if the specified pv2e does not
 *      have an assigned logical port.
 *   Notes
 *      Access the port lp by specifying BCM_VLAN_MAX as the VID.
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_lp_get_by_pv"
int
bcm_fe2000_lp_get_by_pv(int unit,
                        int iport,
                        int ivid,
                        soc_sbx_g2p3_lp_t *lpData)
{
    int result;

    if (!lpData) {
        VLAN_ERR((VLAN_FUNCMSG1("NULL pointer to logical port data\n")));
        return BCM_E_PARAM;
    }

    VLAN_UNIT_INIT_CHECK;
    VLAN_LOCK_TAKE;

    result = _bcm_fe2000_lp_get_by_pv(unit, iport, ivid, lpData);

    VLAN_LOCK_RELEASE;

    return result;
}
#undef _VLAN_FUNC_NAME
#endif /* def BCM_FE2000_P3_SUPPORT */

#ifdef BCM_FE2000_P3_SUPPORT
/*
 *   Function
 *      bcm_fe2000_lp_set_by_pv
 *   Purpose
 *      Sets a logical port, but does so by checking the pv2e.  Will fall back
 *      to the implied logical port for a port if the specified pv2e does not
 *      have an assigned logical port.
 *
 *      Replicates non-diverged LP data from the native VID to the zero VID if
 *      the native VID LP and zero VID LP have diverged and the write is to the
 *      native VID.
 *
 *      Replicates non-diverged LP data from the zero VID to the native VID if
 *      the native VID LP and zero VID LP have diverged and the write is to the
 *      zero VID.
 *   Notes
 *      Access the port lp by specifying BCM_VLAN_MAX as the VID.
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_lp_set_by_pv"
int
bcm_fe2000_lp_set_by_pv(int unit,
                        int iport,
                        int ivid,
                        soc_sbx_g2p3_lp_t *lpData)
{
    int result;

    if (!lpData) {
        VLAN_ERR((VLAN_FUNCMSG1("NULL pointer to logical port data\n")));
        return BCM_E_PARAM;
    }

    VLAN_UNIT_INIT_CHECK;
    VLAN_LOCK_TAKE;

    result = _bcm_fe2000_lp_set_by_pv(unit, iport, ivid, lpData);

    VLAN_LOCK_RELEASE;

    return result;
}
#undef _VLAN_FUNC_NAME
#endif /* def BCM_FE2000_P3_SUPPORT */

#ifdef BCM_FE2000_P3_SUPPORT
/*
 *   Function
 *      bcm_fe2000_lp_set_by_pv_and_diverge
 *   Purpose
 *      Adds logical port divergence between the native VID and the zero VID on
 *      a given port, if writing to the zero VID and some divergence is
 *      indicated. This divergence will allocate (if not already diverged) a
 *      new logicalPort ID and assign it to the zero VID, copying the
 *      nondiverged fields from the native VID's logicalPort.
 *
 *      This does not manage any other resources; caller must have already set
 *      up new resources for diverged logical port as appropriate.  All
 *      diverged resources for zero VID are considered to be owned by the zero
 *      VID and will be freed when reconverging.
 *
 *      The provided LP data are those for the zero VID; the native VID.  This
 *      will reflect any nondiverged fields back to the native VID, so the
 *      logical port data must have originally been read from the native VID.
 *
 *      Currently, the only way to reconverge is to set the native VID.
 *   Notes
 *      Access the port lp by specifying BCM_VLAN_MAX as the VID.
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_lp_set_by_pv_and_diverge"
int
bcm_fe2000_lp_set_by_pv_and_diverge(int unit,
                                    int iport,
                                    int ivid,
                                    _bcm_fe2k_vlan_divergence_t divergence,
                                    soc_sbx_g2p3_lp_t *lpData)
{
    int result;

    if (!lpData) {
        VLAN_ERR((VLAN_FUNCMSG1("NULL pointer to logical port data\n")));
        return BCM_E_PARAM;
    }

    VLAN_UNIT_INIT_CHECK;
    VLAN_LOCK_TAKE;

    result = _bcm_fe2000_lp_set_by_pv_and_diverge(unit,
                                                  iport,
                                                  ivid,
                                                  divergence,
                                                  lpData);

    VLAN_LOCK_RELEASE;

    return result;
}
#undef _VLAN_FUNC_NAME
#endif /* def BCM_FE2000_P3_SUPPORT */

#ifdef BCM_FE2000_P3_SUPPORT
/*
 *   Function
 *      bcm_fe2000_lp_check_diverged
 *   Purpose
 *      Checks whether the logical port for the specified port,vid has diverged
 *      on the specified point.  This condition would only be true if the
 *      specified VID is zero and has indeed diverged on the specified point.
 *
 *      The resulting divergence will be the bitwise AND of the actual
 *      divergence and the specified divergence.
 *   Notes
 *      Access the port lp by specifying BCM_VLAN_MAX as the VID.
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_lp_check_diverged"
int
bcm_fe2000_lp_check_diverged(int unit,
                             int iport,
                             int ivid,
                             _bcm_fe2k_vlan_divergence_t *divergence)
{
    if (!divergence) {
        VLAN_ERR((VLAN_FUNCMSG1("NULL pointer to divergence data\n")));
        return BCM_E_PARAM;
    }

    VLAN_UNIT_INIT_CHECK;
    VLAN_LOCK_TAKE;

    if (0 == ivid) {
        *divergence &= _vlan_state[unit]->divergence[iport];
    } else {
        *divergence = 0;
    }

    VLAN_LOCK_RELEASE;

    return BCM_E_NONE;
}
#undef _VLAN_FUNC_NAME
#endif /* def BCM_FE2000_P3_SUPPORT */

#ifdef BCM_FE2000_P3_SUPPORT
/*
 *  Function
 *    _bcm_fe2000_g2p3_vlan_port_nativevid_touch
 *  Purpose
 *    Touch a port's nativeVid's pv2e and pvv2e whenever things that should
 *    affect it have changed.
 *  Parameters
 *    (in) int unit = unit number on which to operate
 *    (in) bcm_port_t port = physical port on which to operate
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* as appropriate otherwise
 *  Notes
 *    Basically just refreshes the pv2e entry using _i_soc_sbx_g2p3_pv2e_set;
 *    that function automatically takes care of the pv2e entry replication and
 *    addition or update or removal of the pvv2e entry.
 *    Setting native VID to zero implies drop untagged but not the mode!
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_g2p3_vlan_port_nativevid_touch"
static int
_bcm_fe2000_g2p3_vlan_port_nativevid_touch(int unit,
                                           bcm_port_t port)
{
    int result;
    soc_sbx_g2p3_pv2e_t pv2e, pnv2e;

    if (0 != _vlan_state[unit]->nvid[port]) {
        /* get the native VID's configuration */
        result = SOC_SBX_G2P3_PV2E_GET(unit,
                                       port,
                                       _vlan_state[unit]->nvid[port],
                                       &pv2e);

        result = SOC_SBX_G2P3_PV2E_GET(unit, port, 0, &pnv2e);

        /* if in tag drop, force vlan to vid */
        if (PBMP_MEMBER(_vlan_state[unit]->tag_drop, port)) {
            pv2e.vlan = (pnv2e.vlan ? 
                         pnv2e.vlan :
                         _vlan_state[unit]->nvid[port]);

            VLAN_VERB((VLAN_FUNCMSG1("port %d in drop_tag mode, force vlan to"
                                     "nativeVid:%03X\n"), port, pv2e.vlan));
            
            
        }

    } else { /* if (0 != _vlan_state[unit]->nvid[port]) */
        /* native VID is zero; just drop it */
        soc_sbx_g2p3_pv2e_t_init(&pv2e);
        result = BCM_E_NONE;
    } /* if (0 != _vlan_state[unit]->nvid[port]) */

    if (BCM_E_NONE == result) {
        result = _i_soc_sbx_g2p3_pv2e_set(unit,
                                          port,
                                          _vlan_state[unit]->nvid[port],
                                          &pv2e);
        if (BCM_E_NONE != result) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to update %d:pv2e[%d,%03X]:"
                                    " %d (%s)\n"),
                      unit,
                      port,
                      _vlan_state[unit]->nvid[port],
                      result,
                      _SHR_ERRMSG(result)));
        }
    } else {
        VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:pv2e[%d,%03X]:"
                                " %d (%s)\n"),
                  unit,
                  port,
                  _vlan_state[unit]->nvid[port],
                  result,
                  _SHR_ERRMSG(result)));
    }

    return result;
}
#undef _VLAN_FUNC_NAME
#endif /* def BCM_FE2000_P3_SUPPORT */

#ifdef BCM_FE2000_P3_SUPPORT
/*
 *  Function
 *    bcm_fe2000_g2p3_vlan_port_nativevid_touch
 *  Purpose
 *    Touch a port's nativeVid's pv2e and pvv2e whenever things that should
 *    affect it have changed.
 *  Parameters
 *    (in) int unit = unit number on which to operate
 *    (in) bcm_port_t port = physical port on which to operate
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* as appropriate otherwise
 *  Notes
 *    Basically just refreshes the pv2e entry using _i_soc_sbx_g2p3_pv2e_set;
 *    that function automatically takes care of the pv2e entry replication and
 *    addition or update or removal of the pvv2e entry.
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_g2p3_vlan_port_nativevid_touch"
int
bcm_fe2000_g2p3_vlan_port_nativevid_touch(int unit,
                                          bcm_port_t port)
{
    int result;

    VLAN_EVERB((VLAN_FUNCMSG("(%d,%d) enter\n"), unit, port));

    VLAN_UNIT_INIT_CHECK;
    VLAN_LOCK_TAKE;

    result = _bcm_fe2000_g2p3_vlan_port_nativevid_touch(unit, port);

    VLAN_LOCK_RELEASE;

    VLAN_EVERB((VLAN_FUNCMSG("(%d,%d) return %d (%s)\n"),
                unit,
                port,
                result,
                _SHR_ERRMSG(result)));

    return result;
}
#undef _VLAN_FUNC_NAME
#endif /* def BCM_FE2000_P3_SUPPORT */

#ifdef BCM_FE2000_P3_SUPPORT
/*
 *   Function
 *      _bcm_fe2000_vlan_port_lp_replicate
 *   Purpose
 *      Replicate changes from a port's logicalport to the associated pv2es'
 *      logicalports.
 *   Parameters
 *      (in) int unit = unit number on which to operate
 *      (in) _bcm_fe2k_lp_copy_flags flags = what field(s) to update
 *      (in) soc_sbx_g2p3_lp_t *oldValues = ptr to the values before the update
 *      (in) soc_sbx_g2p3_lp_t *newValues = ptr to the values after the update
 *   Returns
 *      bcm_error_t cast as int
 *         BCM_E_NONE for success
 *         BCM_E_* otherwise as appropriate
 *   Notes
 *      Only updates pv2e->lp field(s) as selected in the flags, for pv2e with
 *      lp with the field value(s) equal to the oldValues, setting only these
 *      fields to the newValues.  This way, it does not affect any pv2e lp that
 *      has its own values (so port changes apply except where overridden).
 *      Assumes the vlan lock has been taken.
 */


#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_port_lp_replicate"
static int
_bcm_fe2000_vlan_port_lp_replicate(const int unit,
                                   const bcm_port_t port,
                                   const _bcm_fe2k_lp_copy_flags_t flags,
                                   const soc_sbx_g2p3_lp_t *oldValues,
                                   const soc_sbx_g2p3_lp_t *newValues)
{
    int                 rv;
    int                 doUpdate;
    bcm_vlan_t          vIndex;
    soc_sbx_g2p3_pv2e_t pv2e;
    soc_sbx_g2p3_lp_t   tempLp;
    soc_sbx_g2p3_lp_t   localOldValues;
    uint32_t            lpi=0;

    for (vIndex = 1; vIndex < BCM_VLAN_MAX; vIndex++) {
        /* get the pv2e's logcal port index */
        rv = SOC_SBX_G2P3_PV2E_GET(unit, port, vIndex, &pv2e);
        if (BCM_E_NONE == rv) {

            lpi = pv2e.lpi;

            if (SOC_SBX_CFG(unit)->mplstp_ena && pv2e.vpws) {
                lpi += _BCM_FE2000_VPWS_UNI_OFFSET;
            }

            if (pv2e.lpi) {
                /* got pv2e and lp's not 'default'; read logical port */
                rv = soc_sbx_g2p3_lp_get(unit, lpi, &tempLp);
                if (BCM_E_NONE == rv) {
                    /* force update to new values in ALL cases if oldValues is
                     * NULL */
                    if (oldValues == NULL) {
                        memcpy(&localOldValues, &tempLp, sizeof(soc_sbx_g2p3_lp_t));
                    }else{
                        memcpy(&localOldValues, oldValues, sizeof(soc_sbx_g2p3_lp_t));
                    }

                    doUpdate = FALSE;
                    if ((flags & BCM_FE2K_LP_COPY_PID) &&
                        (localOldValues.pid == tempLp.pid)) {
                        tempLp.pid = newValues->pid;
                        doUpdate = TRUE;
                    }
                    if ((flags & BCM_FE2K_LP_COPY_QOS) &&
                        (localOldValues.qos == tempLp.qos)) {
                        tempLp.qos = newValues->qos;
                        tempLp.useexp = newValues->useexp;
                        tempLp.mefcos = newValues->mefcos;
                        doUpdate = TRUE;
                    }
                    if ((flags & BCM_FE2K_LP_COPY_MIRROR) &&
                        (localOldValues.mirror == tempLp.mirror)) {
                        tempLp.mirror = newValues->mirror;
                        doUpdate = TRUE;
                    }
                    if ((flags & BCM_FE2K_LP_COPY_POLICER) &&
                        (localOldValues.policer == tempLp.policer)) {
                        tempLp.policer = newValues->policer;
                        tempLp.typedpolice = newValues->typedpolice;
                        tempLp.mef = newValues->mef;
                        tempLp.mefcos = newValues->mefcos;
                        tempLp.updaterdp = newValues->updaterdp;
                        doUpdate = TRUE;
                    }
                    if ((flags & BCM_FE2K_LP_COPY_COUNTER) &&
                        (localOldValues.counter == tempLp.counter)) {
                        tempLp.counter = newValues->counter;
                        tempLp.typedcount = newValues->typedcount;
                        doUpdate = TRUE;
                    }
                    if (doUpdate) {
                        rv = soc_sbx_g2p3_lp_set(unit, lpi, &tempLp);
                    }
                }
                if (BCM_E_NONE != rv) {
                    VLAN_ERR((VLAN_FUNCMSG1("unable to update %d:lp[%08X]:"
                                            " %d (%s)\n"),
                              unit,
                              lpi,
                              rv,
                              _SHR_ERRMSG(rv)));
                }
            }
        } else {
            VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:pv2e[%d,%03X]:"
                                    " %d (%s)\n"),
                      unit,
                      port,
                      vIndex,
                      rv,
                      _SHR_ERRMSG(rv)));
        }
    }
    return rv;
}
#undef _VLAN_FUNC_NAME
#endif /* def BCM_FE2000_P3_SUPPORT */

#ifdef BCM_FE2000_P3_SUPPORT
/*
 *   Function
 *      bcm_fe2000_vlan_port_lp_replicate
 *   Purpose
 *      Replicate changes from a port's logicalport to the associated pv2es'
 *      logicalports.
 *   Parameters
 *      (in) int unit = unit number on which to operate
 *      (in) _bcm_fe2k_lp_copy_flags flags = what field(s) to update
 *      (in) soc_sbx_g2p3_lp_t *oldValues = ptr to the values before the update
 *      (in) soc_sbx_g2p3_lp_t *newValues = ptr to the values after the update
 *   Returns
 *      bcm_error_t cast as int
 *         BCM_E_NONE for success
 *         BCM_E_* otherwise as appropriate
 *   Notes
 *      Only updates pv2e->lp field(s) as selected in the flags, for pv2e with
 *      lp with the field value(s) equal to the oldValues, setting only these
 *      fields to the newValues.  This way, it does not affect any pv2e lp that
 *      has its own values (so port changes apply except where overridden).
 *      Takes and releases the VLAN lock (for use by other modules).
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_port_lp_replicate"
int
bcm_fe2000_vlan_port_lp_replicate(const int unit,
                                  const bcm_port_t port,
                                  const _bcm_fe2k_lp_copy_flags_t flags,
                                  const soc_sbx_g2p3_lp_t *oldValues,
                                  const soc_sbx_g2p3_lp_t *newValues)
{
    int                 rv;

    VLAN_UNIT_INIT_CHECK;
    VLAN_LOCK_TAKE;

    rv = _bcm_fe2000_vlan_port_lp_replicate(unit,
                                            port,
                                            flags,
                                            oldValues,
                                            newValues);

    VLAN_LOCK_RELEASE;

    return rv;
}
#undef _VLAN_FUNC_NAME


/*
 *   Function
 *      _bcm_fe2000_vlan_trunk_change_handler
 *   Purpose
 *     Invoked by the trunk module when it experinces a port membership change.
 *   Currently, only vlan gports must respond to this event by updating the
 *   logical ports for the new port memebership.
 * 
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_trunk_change_handler"
static int
_bcm_fe2000_vlan_trunk_change_handler(int unit,
                                      bcm_trunk_t tid,
                                      bcm_trunk_add_info_t *add_info,
                                      void *user_data)
{
    enum { trunk_action_add, trunk_action_remove, trunk_action_count };
#ifdef BROADCOM_DEBUG
    const char* taction_strs[trunk_action_count] = { "add", "remove" };
#endif /* BROADCOM_DEBUG */

    int idx, taction, install_idx, raw_gport;
    int rv = BCM_E_NONE;
    dq_t *vp_elt;
    _bcm_fe2k_trunk_state_t *ts;
    bcm_module_t local_modid;
    bcm_port_t   port_list[trunk_action_count][BCM_TRUNK_MAX_PORTCNT];
    bcm_module_t mod_list[trunk_action_count][BCM_TRUNK_MAX_PORTCNT];
    int          num_ports[trunk_action_count];
    _bcm_fe2k_vlan_gport_t  *vlan_gport;
    bcm_port_t   tport;
    uint32       lpi;
    bcm_vlan_t   vsi;
    bcm_vlan_vector_t vector;


    VLAN_VERB((VLAN_FUNCMSG("(%d,0x%04X,*, *) enter\n"), unit, tid));

    for (taction=0; taction < trunk_action_count; taction++) {
        num_ports[taction] = 0;
        sal_memset(port_list[taction], 0, sizeof(port_list[taction]));
        sal_memset(mod_list[taction], 0, sizeof(mod_list[taction]));
    }

    rv = bcm_stk_my_modid_get(unit, &local_modid);
    if (BCM_FAILURE(rv)) {
        return rv;
    }
    
    VLAN_LOCK_TAKE;

    ts = &_vlan_state[unit]->trunk[tid];
    if (DQ_EMPTY(&ts->vlan_port_head)) {
        VLAN_VERB((VLAN_FUNCMSG1("Not tracking any vlan gports on "
                                 "Trunk Id %d; done.\n"), tid));
        VLAN_LOCK_RELEASE;
        return rv; 
    }

    /* mod,ports in given list, not in last known must be added */
    bcm_fe2k_trunk_add_info_cmp(add_info, &ts->tinfo, 
                                &num_ports[trunk_action_add],
                                mod_list[trunk_action_add],
                                port_list[trunk_action_add]);
    
    /* mod,ports in last known, not in given list must be removed */
    bcm_fe2k_trunk_add_info_cmp(&ts->tinfo, add_info, 
                                &num_ports[trunk_action_remove],
                                mod_list[trunk_action_remove], 
                                port_list[trunk_action_remove]);
    
    /* remove non-local port membership changes */
    for (taction=0; taction < trunk_action_count; taction++) {
        install_idx=0;
        for (idx=0; idx < num_ports[taction]; idx++) {
            mod_list[taction][install_idx] = mod_list[taction][idx];
            port_list[taction][install_idx] = port_list[taction][idx];
            if (mod_list[taction][idx] == local_modid) {
                install_idx++;
            }
        }
        num_ports[taction] = install_idx;
    }

    /* Traverse all vlan gports attached to this trunk.  Update physical port
     * changes based on new trunk port membership
     */
    DQ_TRAVERSE(&ts->vlan_port_head, vp_elt) {

        vlan_gport = DQ_ELEMENT_GET(_bcm_fe2k_vlan_gport_t*, vp_elt, nt);

        raw_gport = BCM_GPORT_VLAN_PORT_ID_GET(vlan_gport->p.vlan_port_id);
        if (raw_gport < 0) {
             VLAN_ERR((VLAN_FUNCMSG1("Invalid gport found: 0x%08x\n"),
                       vlan_gport->p.vlan_port_id));
             continue;
        }
        lpi = _vlan_state[unit]->gportInfo.lpid[raw_gport];

        VLAN_VERB((VLAN_FUNCMSG1("Updating gport 0x%08x (lpi 0x%04x) "
                                 "on TID %d\n"),
                   vlan_gport->p.vlan_port_id, lpi, tid));
        for (taction = 0; taction < trunk_action_count; taction++) {
            VLAN_VERB((VLAN_FUNCMSG1("%s %d physical ports: "),
                       taction_strs[taction], num_ports[taction]));
            for (idx = 0; idx < num_ports[taction]; idx++) {
                VLAN_VERB(("%d ", port_list[taction][idx]));
            }
            VLAN_VERB(("\n"));
        }

        for (taction = 0; taction < trunk_action_count; taction++) {
            if (taction == trunk_action_add) {
                vsi = vlan_gport->p.vsi;
                lpi = _vlan_state[unit]->gportInfo.lpid[raw_gport];
            } else {
                vsi = lpi = 0;
            }
            
            for (idx = 0; idx < num_ports[taction]; idx++) {

                tport = port_list[taction][idx];
                rv = _bcm_fe2000_port_strip_tag(unit, tport, 0);
            
                if (BCM_FAILURE(rv)) {
                    VLAN_ERR((VLAN_FUNCMSG1("Failed to update port strip "
                                            "on target port %d\n"), tport));
                    break;
                }
            
                rv = _bcm_fe2000_vp_match_configure(unit, vlan_gport, tport,
                                                    vsi, lpi);

                if (BCM_FAILURE(rv)) {
                    break;
                }
            
                /* Get the vector - this only works if the gport
                 * already exists on the local unit.  If this is the first
                 * time a port is added to the lag membership for the unit,
                 * the application MUST estabilsh the vlan vector.
                 */
                rv = _bcm_fe2000_vlan_port_vlan_vector_get(unit,
                                                           vlan_gport->p.vlan_port_id,
                                                           vector);
                if (BCM_FAILURE(rv)) {
		    /* ignore the error and the user has to explicitly call bcm_vlan_vector_set */
		    rv = BCM_E_NONE;
                    break;
                }

                rv = _bcm_fe2000_g2p3_vlan_port_vlan_vector_set(unit,
                                                                vector,
                                                                local_modid,
                                                                &vlan_gport->p,
                                                                tport,
                                                                vlan_gport->p.vsi);

                if (BCM_FAILURE(rv)) {
                    break;
                }
            } /* for each port changed (added/deleted) */
        } /* for each action - add or delete */

        VLAN_VERB((VLAN_FUNCMSG1("Completed updating gport 0x%08x\n"),
                   vlan_gport->p.vlan_port_id));

    } DQ_TRAVERSE_END(&ts->vlan_port_head, vp_elt);


    /* update to last known trunk membership */
    if (BCM_SUCCESS(rv)) {
        sal_memcpy(&ts->tinfo, add_info, sizeof(ts->tinfo));
    }

    VLAN_LOCK_RELEASE;
    return rv;
}
#undef _VLAN_FUNC_NAME


#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_g2p3_untagged_get"
static int
_bcm_fe2000_vlan_g2p3_untagged_get(int unit, int port, bcm_vlan_t vlan,
                                   int *untagged)
{
    int result;
    soc_sbx_g2p3_evp2e_t  p3egrVlanPort2Etc;
    soc_sbx_g2p3_etel2_t  p3eteL2;
    soc_sbx_g2p3_epv2e_t  p3egrPortVid2Etc;

    result = soc_sbx_g2p3_evp2e_get(unit,
                                    vlan,
                                    port,
                                    &p3egrVlanPort2Etc);
    if (BCM_E_NONE != result) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:evp2e[%d,%04X]:"
                                " %d (%s)\n"),
                  unit,
                  port,
                  vlan,
                  result,
                  _SHR_ERRMSG(result)));
        return result;
    }

    result = soc_sbx_g2p3_etel2_get(unit, p3egrVlanPort2Etc.eteptr, &p3eteL2);
    if (BCM_E_NONE != result) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:eteL2[%08X]:"
                                " %d (%s)\n"),
                  unit,
                  p3egrVlanPort2Etc.eteptr,
                  result,
                  _SHR_ERRMSG(result)));
        return result;
    }

    if (p3eteL2.usevid) {
        vlan = p3eteL2.vid;
    }
    result = SOC_SBX_G2P3_EPV2E_GET(unit, port, vlan, &p3egrPortVid2Etc);
    if (BCM_E_NONE != result) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to read %d"
                                " epv2e[%02x,%03x]:"
                                " %d (%s)\n"),
                  unit,
                  port,
                  vlan,
                  result,
                  _SHR_ERRMSG(result)));
        return result;
    }

    *untagged = p3egrPortVid2Etc.strip;
    return result;
}
#undef _VLAN_FUNC_NAME

#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_g2p3_untagged_set"
static int
_bcm_fe2000_vlan_g2p3_untagged_set(int unit, int port, bcm_vlan_t vlan,
                                   int untagged)
{
    int result;
    soc_sbx_g2p3_epv2e_t  p3egrPortVid2Etc;

    result = SOC_SBX_G2P3_EPV2E_GET(unit, port, vlan, &p3egrPortVid2Etc);
    if (BCM_E_NONE != result) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to read %d"
                                " epv2e[%02x,%03x]:"
                                " %d (%s)\n"),
                  unit,
                  port,
                  vlan,
                  result,
                  _SHR_ERRMSG(result)));
        return result;
    }

    p3egrPortVid2Etc.strip = untagged;
    result = SOC_SBX_G2P3_EPV2E_SET(unit, port, vlan, &p3egrPortVid2Etc);
    if (BCM_E_NONE != result) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to write %d"
                                " epv2e[%02x,%03x]:"
                                " %d (%s)\n"),
                  unit,
                  port,
                  vlan,
                  result,
                  _SHR_ERRMSG(result)));
    }

    return result;
}
#undef _VLAN_FUNC_NAME
#endif /* def BCM_FE2000_P3_SUPPORT */

/*
 *  Function
 *    _bcm_fe2000_vlan_check
 *  Purpose
 *    Check whether a VLAN has been configured and get its FT entry
 *  Parameters
 *    (in) const int unit = target unit
 *    (in) const bcm_vlan_t vid = target VLAN ID
 *    (out) int *valid = pointer to where to put valid flag
 *    (out) _bcm_sbx_vlan_ft_t *ft = pointer to buffer for FT entry
 *  Returns
 *    bcm_error_t = BCM_E_NONE if successful
 *                  BCM_E_* as appropriate otherwise
 *  Notes
 *    Practically no argument checking is performed
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_check"
static int
_bcm_fe2000_vlan_check(const int unit,
                       const bcm_vlan_t vid,
                       int *valid,
                       _bcm_sbx_vlan_ft_t *ft)
{
    int                   result;              /* local result code */

    *valid = FALSE;
    result = BCM_E_NONE;

#ifdef BCM_FE2000_P3_SUPPORT
    if (SOC_IS_SBX_G2P3(unit)) {
        uint32 ftidx = 0;
        BCM_IF_ERROR_RETURN
           (soc_sbx_g2p3_vlan_ft_base_get(unit, &ftidx));
        ftidx += SBX_VSI_FROM_VID(vid);
        result = soc_sbx_g2p3_ft_get(unit,
                                     ftidx,
                                     &(ft->p3ft));
        if (BCM_E_NONE == result) {
            if (VLAN_INV_FTE_EXC != ft->p3ft.excidx) {
                *valid = TRUE;
            }
        } else { /* if (BCM_E_NONE == result) */
            VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:ft[%08X]: %d (%s)\n"),
                      unit,
                      SBX_VSI_FROM_VID(vid),
                      result,
                      _SHR_ERRMSG(result)));
        } /* if (BCM_E_NONE == result) */
    } else /* if (SOC_IS_SBX_G2P3(unit)) */
#endif /* def BCM_FE2000_P3_SUPPORT */
    {
        VLAN_ERR((VLAN_FUNCMSG1("invalid unit %d type\n"), unit));
        result = VLAN_UNKNOWN_TYPE;
    }

    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *  Function
 *    _bcm_fe2000_vlan_check_exists
 *  Purpose
 *    Check whether a VLAN has been configured
 *  Parameters
 *    (in) const int unit = target unit
 *    (in) const bcm_vlan_t vid = target VLAN ID
 *    (out) int *valid = pointer to where to put valid flag
 *  Returns
 *    bcm_error_t = BCM_E_NONE if successful
 *                  BCM_E_* as appropriate otherwise
 *  Notes
 *    Practically no argument checking is performed
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_check_exists"
int
_bcm_fe2000_vlan_check_exists(const int unit,
                              const bcm_vlan_t vid,
                              int *valid)
{
    int                   result;               /* local result code */
    _bcm_sbx_vlan_ft_t    ft;                   /* local FT entry buffer */

    VLAN_EVERB((VLAN_FUNCMSG("(%d,%03X,*) enter\n"), unit, vid));

    VLAN_UNIT_INIT_CHECK;
    VLAN_LOCK_TAKE;

    result = _bcm_fe2000_vlan_check(unit, vid, valid, &ft);

    VLAN_LOCK_RELEASE;

    VLAN_EVERB((VLAN_FUNCMSG("(%d,%03X,&(%s)) enter\n"),
                unit,
                vid,
                (*valid)?"TRUE":"FALSE"));

    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_alloc_ete
 *   Purpose
 *      Allocates an ETE that matches the specified translation
 *   Parameters
 *      (in) int unit = target unit for the search
 *      (in) bcm_vlan_t destVid = target VID for the translation
 *      (in) int priority = target priority for the translation
 *      (in) _bcm_sbx_v_e_t_state_t flags = flags for the translation
 *      (in) int untagged = indicates untagged egress frames
 *      (out) unsigned int *eteId = where to put the allocated ETE number
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* as appropriate otherwise
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_alloc_ete"
static int
_bcm_fe2000_vlan_alloc_ete(const int unit,
                           bcm_port_t port,
                           const bcm_vlan_t destVid,
                           const int priority,
                           const _bcm_sbx_v_e_t_state_t flags,
                           const int untagged,
                           unsigned int* eteId)
{
#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_etel2_t  p3eteL2;               /* G2P3 L2 ETE */
#endif /* def BCM_FE2000_P3_SUPPORT */
    int                   result;                /* this function's result */
    unsigned int          eteNum = ~0;           /* allocated ETE number */

    if (SOC_IS_SBX_G2P3(unit)) {
#ifdef BCM_FE2000_P3_SUPPORT
        /* go ahead and initialise the ET entry buffer */
        soc_sbx_g2p3_etel2_t_init(&p3eteL2);
        p3eteL2.mtu = SBX_DEFAULT_MTU_SIZE;
        if (!untagged) {
            if (flags & VLAN_TRANS_CHANGE_VID) {
                p3eteL2.usevid = 1;
                p3eteL2.vid = destVid;
            }
        } /* if (!untagged) */
#endif /* def BCM_FE2000_P3_SUPPORT */
    } else {
        /* unknown or disabled unit type */
        VLAN_ERR((VLAN_FUNCMSG1("unsupported unit %d type\n"),
                  unit));
        result = VLAN_UNKNOWN_TYPE;
    }

    /* check for standard ETE and use it if it is one */
    if (0 == (flags & (VLAN_TRANS_CHANGE_PRI | VLAN_TRANS_CHANGE_VID))) {
        /* looks like it should be a standard one, pick which one */
        eteNum = untagged?SOC_SBX_PORT_UT_ETE(unit, port):
                          SOC_SBX_PORT_ETE(unit, port);
        /* nothing more to do; short-circuit out of here */
        *eteId = eteNum;
        return BCM_E_NONE;
    }

    /* something isn't standard; request a new ET entry */
    result = _sbx_gu2_resource_alloc(unit,
                                     SBX_GU2K_USR_RES_ETE_L2,
                                     1,
                                     &eteNum,
                                     0);
    if (BCM_E_NONE != result) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to alloc ET entry unit %d: %d (%s)\n"),
                  unit,
                  result,
                  _SHR_ERRMSG(result)));
    }

    /* configure the newly acquired ET entry */
    if (BCM_E_NONE == result) {
#ifdef BCM_FE2000_P3_SUPPORT
        if (SOC_IS_SBX_G2P3(unit)) {
            result = soc_sbx_g2p3_etel2_set(unit, eteNum, &p3eteL2);
            if (BCM_E_NONE != result) {
                VLAN_ERR((VLAN_FUNCMSG1("unable to write to %d:eteL2[%08X]:"
                                        " %d (%s)\n"),
                          unit,
                          eteNum,
                          result,
                          _SHR_ERRMSG(result)));
            }
        }
#endif /* def BCM_FE2000_P3_SUPPORT */
        if (BCM_E_NONE != result) {
            /* could not write the ET entry; just get rid of it */
            _sbx_gu2_resource_free(unit,
                                   SBX_GU2K_USR_RES_ETE_L2,
                                   1,
                                   &eteNum,
                                   0);
        }
    }

    if (BCM_E_NONE == result) {
        /* everything went well; give the ETE number to the caller */
        *eteId = eteNum;
    }

    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_ete_is_standard
 *   Purpose
 *      Determines if passed-in ETE is standard
 *   Parameters
 *      (in) const int unit = target unit for the search
 *      (in) const uint32 eteId = ETE number to check
 *   Returns
 *      bcm_error_t = BCM_E_NOT_FOUND if not standard
 *                    BCM_E_EXISTS if standard
 *                    BCM_E_* otherwise as appropriate
 */
static int
_bcm_fe2000_ete_is_standard(const int unit,
                            const uint32 eteId)
{
    int pIndex;

    PBMP_ALL_ITER(unit, pIndex) {
        if (eteId == SOC_SBX_PORT_ETE(unit, pIndex)) {
            /* standard ETE -- port specific taged */
            return BCM_E_EXISTS;
        }
        if (eteId == SOC_SBX_PORT_UT_ETE(unit, pIndex)) {
            /* standard ETE -- port specific untagged */
            return BCM_E_EXISTS;
        }
    }

    return BCM_E_NOT_FOUND;
}

/*
 *   Function
 *      _bcm_fe2000_vlan_free_ete
 *   Purpose
 *      Allocates the ETE that has the specified ID
 *   Parameters
 *      (in) int unit = target unit for the search
 *      (in) unsigned int eteId = ETE number to free
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* as appropriate otherwise
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_free_ete"
static int
_bcm_fe2000_vlan_free_ete(const int unit,
                          uint32 eteId)
{
    int                       result;   /* this function's result */

    /* hope for the best */
    result = BCM_E_NONE;

    if (BCM_E_EXISTS == _bcm_fe2000_ete_is_standard(unit, eteId)) {
        /* standard ETE -- don't free it */
        return BCM_E_NONE;
    }

    /* not a standard ETE */
    if (!(_vlan_state[unit]->init)) {
        /* not initialising, so ETEs are valid */
        result = _sbx_gu2_resource_free(unit,
                                        SBX_GU2K_USR_RES_ETE_L2,
                                        1,
                                        &eteId,
                                        0);
        if (BCM_E_NONE != result) {
            VLAN_ERR((VLAN_FUNCMSG1("_sbx_gu2_resource_free"
                                    "(%d,SBX_GU2K_USR_RES_ETE_L2,1,&(%08X),0)"
                                    " returned %d (%s)\n"),
                      unit,
                      eteId,
                      result,
                      _SHR_ERRMSG(result)));
        }
    } /* if (!(_vlan_state[unit]->init)) */

    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_translate_set
 *   Purpose
 *      Set translation on ingress from one VID to another (add or delete)
 *   Parameters
 *      (in) int unit = unit number on which the VID is to be translated
 *      (in) int port = port number on which the VID is to be translated
 *      (in) bcm_vlan_t old_vid = original VID
 *      (in) bcm_vlan_t new_vid = translated VID
 *      (in) int prio = new priority (-1 to leave unchanged)
 *      (in) int enable = whether to enable translation
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes unit lock is already taken.
 *      Translation can only be applied to enabled port+VID.
 *      Translation state is lost when port+VID is disabled.
 *      Translation state is lost when port ingress filtering changes.
 *      Apparently, this feature is somehow meant to insert tags rather than
 *      simple 1:1 translation of tags. [maybe not?]
 */



#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_translate_set"
static int
_bcm_fe2000_vlan_translate_set(int unit,
                               int port,
                               bcm_vlan_t old_vid,
                               bcm_vlan_t new_vid,
                               int prio,
                               int enable)
{
#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_pv2e_t      p3pVid2Etc;        /* G2P3 pVid2Etc entry */
#endif /* def BCM_FE2000_P3_SUPPORT */
    int                      result;            /* local result code */

    VLAN_EVERB((VLAN_FUNCMSG("(%d,%d,%04X,%04X,%d,%s) enter\n"),
                unit,
                port,
                old_vid,
                new_vid,
                prio,
                enable?"TRUE":"FALSE"));

    /* Optimistically hope for success from here */
    result = BCM_E_NONE;

    /* do preliminary setup (get handles, &c) */
#ifdef BCM_FE2000_P3_SUPPORT
    if (SOC_IS_SBX_G2P3(unit)) {
        /* only really need this for negative condition */
    } else
#endif /* def BCM_FE2000_P3_SUPPORT */
    {
        /* no supported device here */
        VLAN_ERR((VLAN_FUNCMSG1("unsupported unit %d type\n"), unit));
        result = VLAN_UNKNOWN_TYPE;
    }

    if (PBMP_MEMBER(_vlan_state[unit]->tag_drop, port)) {
        /* this port is set to reject tagged frames */
        result = BCM_E_CONFIG;
        VLAN_ERR((VLAN_FUNCMSG1("unit %d port %d is rejecting"
                                " tagged frames so it can't"
                                " translate incoming tags\n"),
                  unit,
                  port));
    }

    
    if (-1 != prio) {
        result = BCM_E_PARAM;
        VLAN_ERR((VLAN_FUNCMSG1("unit %d can't currently do VLAN priority"
                                " translation\n"),
                  unit));
    }

    if (BCM_E_NONE == result) {
#ifdef BCM_FE2000_P3_SUPPORT
        if (SOC_IS_SBX_G2P3(unit)) {
            result = SOC_SBX_G2P3_PV2E_GET(unit,
                                           port,
                                           old_vid,
                                           &p3pVid2Etc);
            if (BCM_E_NONE == result) {
                if (enable) {
                    p3pVid2Etc.vlan = new_vid;
                } else { /* if (enable) */
                    p3pVid2Etc.vlan = old_vid;
                } /* if (enable) */
                result = _i_soc_sbx_g2p3_pv2e_set(unit,
                                                  port,
                                                  old_vid,
                                                  &p3pVid2Etc);
                if (BCM_E_NONE != result) {
                    VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:pv2e"
                                            "[%04X,%d]: %d (%s)\n"),
                              unit,
                              old_vid,
                              port,
                              result,
                              _SHR_ERRMSG(result)));
                }
            } else { /* if (BCM_E_NONE == result) */
                VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:pv2e[%04X,%d]:"
                                        " %d (%s)\n"),
                          unit,
                          old_vid,
                          port,
                          result,
                          _SHR_ERRMSG(result)));
            } /* if (BCM_E_NONE == result) */
        } /* if (SOC_IS_SBX_G2P3(unit)) */
#endif /* def BCM_FE2000_P3_SUPPORT */
    } /* if (BCM_E_NONE == result) */

    VLAN_EVERB((VLAN_FUNCMSG("(%d,%d,%04X,%04X,%d,%s) return %d (%s)\n"),
                unit,
                port,
                old_vid,
                new_vid,
                prio,
                enable?"TRUE":"FALSE",
                result,
                _SHR_ERRMSG(result)));

    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_translate_egress_set
 *   Purpose
 *      Set translation on egress from one VID to another (add or delete)
 *   Parameters
 *      (in) int unit = unit number on which the VID is to be translated
 *      (in) int port = port number on which the VID is to be translated
 *      (in) bcm_vlan_t old_vid = original VID
 *      (in) bcm_vlan_t new_vid = translated VID
 *      (in) int prio = new priority (-1 to leave unchanged)
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes unit lock is already taken.
 *      Translation can only be applied to enabled port+VID.
 *      Translation state is lost when port+VID is disabled.
 */

#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_translate_egress_set"
static int
_bcm_fe2000_vlan_translate_egress_set(int unit,
                                      int port,
                                      bcm_vlan_t old_vid,
                                      bcm_vlan_t new_vid,
                                      int prio)
{
#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_evp2e_t  p3egrVlanPort2Etc;     /* G2P3 egrVlanPort2Etc ent */
#endif /* def BCM_FE2000_P3_SUPPORT */
    _bcm_sbx_v_e_t_state_t flags = 0;            /* flags for translation */
    int                   untagged = FALSE;      /* whether port is untagged */
    unsigned int          eteNum = ~0;           /* working ETE number */
    unsigned int          eteOld = ~0;           /* previous ETE number */
    int                   result;                /* local result code */
    int                   auxres;                /* spare result code */

    VLAN_EVERB((VLAN_FUNCMSG("(%d,%d,%04X,%04X,%d) enter\n"),
                unit,
                port,
                old_vid,
                new_vid,
                prio));

    /* be optimistic about results */
    result = BCM_E_NONE;

    untagged = FALSE; 

    /* figure out the flags for the new translation */
    if (old_vid != new_vid) {
        /* changing the VID */
        flags |= VLAN_TRANS_CHANGE_VID;
    }
    if (-1 != prio) {
        /* changing the priority */
        flags |= VLAN_TRANS_CHANGE_PRI;
        
        VLAN_ERR((VLAN_FUNCMSG1("egress priority translation not supported"
                                " on unit %d\n"),
                  unit));
        return BCM_E_PARAM;
    }

    /* do preliminary setup (get handles, &c) */
#ifdef BCM_FE2000_P3_SUPPORT
    if (SOC_IS_SBX_G2P3(unit)) {
        /* only really need this for negative condition */
    } else
#endif /* def BCM_FE2000_P3_SUPPORT */
    {
        /* no supported device here */
        VLAN_ERR((VLAN_FUNCMSG1("unsupported unit %d type\n"), unit));
        result = VLAN_UNKNOWN_TYPE;
    }

    /* get the old translation on this port+vid */
    if (BCM_E_NONE == result) {
#ifdef BCM_FE2000_P3_SUPPORT
        if (SOC_IS_SBX_G2P3(unit)) {
            result = soc_sbx_g2p3_evp2e_get(unit,
                                            old_vid,
                                            port,
                                            &p3egrVlanPort2Etc);
            if (BCM_E_NONE == result) {
                eteOld = p3egrVlanPort2Etc.eteptr;
                result = _bcm_fe2000_vlan_g2p3_untagged_get(unit, port,
                                                            old_vid,
                                                            &untagged);
            } else { /* if (BCM_E_NONE == result) */
                VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:evp2e[%d,%04X]:"
                                        " %d (%s)\n"),
                          unit,
                          port,
                          old_vid,
                          result,
                          _SHR_ERRMSG(result)));
            } /* if (BCM_E_NONE == result) */
        } /* if (SOC_IS_SBX_G2P3(unit)) */
#endif /* def BCM_FE2000_P3_SUPPORT */
    } /* if (BCM_E_NONE == result) */

    /* allocate a new translation ETE */
    if (BCM_E_NONE == result) {
        result = _bcm_fe2000_vlan_alloc_ete(unit,
                                            port,
                                            new_vid,
                                            prio,
                                            flags,
                                            untagged,
                                            &eteNum);
        if (BCM_E_NONE != result) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to allocate new translation"
                                    " ETE on unit %d\n"),
                      unit));
        }
    }

    /* set to use the new translation ETE */
    if (BCM_E_NONE == result) {
#ifdef BCM_FE2000_P3_SUPPORT
        if (SOC_IS_SBX_G2P3(unit)) {
            result = _bcm_fe2000_vlan_g2p3_untagged_set(unit, port,
                                                        new_vid, untagged);
            if (BCM_E_NONE == result) {
                p3egrVlanPort2Etc.eteptr = eteNum;
                result = soc_sbx_g2p3_evp2e_set(unit,
                                                old_vid,
                                                port,
                                                &p3egrVlanPort2Etc);
                if (BCM_E_NONE != result) {
                    VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:"
                                            "evp2e[%d,%04X]: %d (%s)\n"),
                              unit,
                              port,
                              old_vid,
                              result,
                              _SHR_ERRMSG(result)));
                }
            }
        } /* if (SOC_IS_SBX_G2P3(unit)) */
#endif /* def BCM_FE2000_P3_SUPPORT */
        if (BCM_E_NONE == result) {
            /* using new ETE; get rid of the old one */
            result = _bcm_fe2000_vlan_free_ete(unit, eteOld);
            if (BCM_E_NONE != result) {
                /* something went wrong freeing old translation ETE */
                VLAN_ERR((VLAN_FUNCMSG1("unable to free old %d:eteL2[%08X]:"
                                        " %d (%s)\n"),
                          unit,
                          eteOld,
                          result,
                          _SHR_ERRMSG(result)));
            }
        } else { /* if (BCM_E_NONE == result) */
            /* not using new ETE; get rid of new ETE */
            auxres = _bcm_fe2000_vlan_free_ete(unit, eteNum);
            if (BCM_E_NONE != result) {
                /* but discard this error, since something else caused it */
                VLAN_ERR((VLAN_FUNCMSG1("unable to free new %d:eteL2[%08X]:"
                                        " %d (%s)\n"),
                          unit,
                          eteNum,
                          result,
                          _SHR_ERRMSG(result)));
            }
        } /* if (BCM_E_NONE == result) */
    } /* if (BCM_E_NONE == result) */

    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_port_add
 *   Purpose
 *      Add a port to a VID in hardware, with specified tagging mode
 *   Parameters
 *      (in) int unit = unit number which is to be affected
 *      (in) bcm_vlan_t vid = vid which is to be affected
 *      (in) bcm_port_t port = port number to add
 *      (in) int untagged = TRUE if untagged
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      Assumes unit lock is already taken.
 *      Assumes all parameters are already verified.
 *      Does not update explicit default VID membership bitmap.
 *      Preserves both translations if port is member unless initialising
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_port_add"
static int
_bcm_fe2000_vlan_port_add(const int unit,
                          const bcm_vlan_t vid,
                          const bcm_port_t port,
                          const unsigned int untagged)
{
#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_pv2e_t   p3pVid2Etc;            /* G2P3 pVid2Etc entry */
    soc_sbx_g2p3_epv2e_t  p3egrPVid2Etc;         /* G2P3 egrPVid2Etc entry */
    soc_sbx_g2p3_evp2e_t  p3egrVlanPort2Etc;     /* G2P3 egrVlanPort2Etc ent */
    soc_sbx_g2p3_pv2appdata_t   p3pv2appdata;    /* G2P3 pv2appdata entry */
#endif /* def BCM_FE2000_P3_SUPPORT */
    int                   result;                /* local result code */
    int                   isMember = FALSE;      /* indicates membership */
    unsigned int          eteOld = ~0;           /* current ET entry index */
    unsigned int          eteNew;                /* new ET entry index */
    unsigned int          vidOld = ~0;           /* current VID number */
    unsigned int          vidNew;                /* new VID number */
    int                   pvv2eCommit = 0;

    VLAN_EVERB((VLAN_FUNCMSG("(%d,%04X,%d,%s) enter\n"),
                unit,
                vid,
                port,
                untagged?"TRUE":"FALSE"));

    /* optimistically assume we'll be successful */
    result = BCM_E_NONE;

    /* make sure the port is valid */
    if (port >= SBX_MAX_PORTS) {
        VLAN_ERR((VLAN_FUNCMSG1("invalid port %d for SBX unit %d\n"),
                  port,
                  unit));
        result = BCM_E_PARAM;
    }

    /* find out if the port is already a member of this VLAN */
    if (BCM_E_NONE == result) {
        if (SOC_IS_SBX_G2P3(unit)) {
#ifdef BCM_FE2000_P3_SUPPORT

            if (_vlan_state[unit]->init) {
                /* initialising; start with clear entries */
                soc_sbx_g2p3_pv2e_t_init(&p3pVid2Etc);
                soc_sbx_g2p3_pv2appdata_t_init(&p3pv2appdata);
                result = BCM_E_NONE;
            } else {

                result = SOC_SBX_G2P3_PV2E_GET(unit,
                                               port,
                                               vid,
                                               &p3pVid2Etc);
                if (BCM_E_NONE == result) {
                    result = SOC_SBX_G2P3_PV2APPDATA_GET(unit,
                                                         port,
                                                         vid,
                                                         &p3pv2appdata);
                    if (BCM_E_NONE != result) {
                        VLAN_ERR((VLAN_FUNCMSG1("unable to read"
                                                " %d:pv2appdata[%d,%03X]:"
                                                " %d (%s)\n"),
                                  unit,
                                  port,
                                  vid,
                                  result,
                                  _SHR_ERRMSG(result)));
                    }
                } else {
                    VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:pv2e[%d,%03X]:"
                                            " %d (%s)\n"),
                              unit,
                              port,
                              vid,
                              result,
                              _SHR_ERRMSG(result)));
                }
            }

            if (BCM_E_NONE == result) {
                /* is this port a member of some VLAN? */
                vidOld = p3pVid2Etc.vlan;
                isMember = (0 != p3pv2appdata.member);
            }
#endif /* def BCM_FE2000_P3_SUPPORT */
        } else {
            /* unknown or disabled unit type */
            VLAN_ERR((VLAN_FUNCMSG1("unsupported unit %d type\n"),
                      unit));
            result = VLAN_UNKNOWN_TYPE;
        }
    }

    /* set the port not to drop this VLAN on egress */
    if (BCM_E_NONE == result) {
        if (SOC_IS_SBX_G2P3(unit)) {
#ifdef BCM_FE2000_P3_SUPPORT
            if (_vlan_state[unit]->init) {
                soc_sbx_g2p3_epv2e_t_init(&p3egrPVid2Etc);
                result = BCM_E_NONE;
            } else {
                result = SOC_SBX_G2P3_EPV2E_GET(unit,
                                                port,
                                                vid,
                                                &p3egrPVid2Etc);
            }
            if (BCM_E_NONE == result) {
                p3egrPVid2Etc.drop = FALSE;
                p3egrPVid2Etc.strip = untagged;
                result = SOC_SBX_G2P3_EPV2E_SET(unit,
                                                port,
                                                vid,
                                                &p3egrPVid2Etc);
            }

            if (BCM_E_NONE != result) {
                VLAN_ERR((VLAN_FUNCMSG1("unable to access %d:epv2e[%d,%03X]:"
                                        " %d (%s)\n"),
                          unit,
                          port,
                          vid,
                          result,
                          _SHR_ERRMSG(result)));
            }
#endif /* def BCM_FE2000_P3_SUPPORT */
        }
    } /* if (BCM_E_NONE == result) */

    /* get the current ET entry index for this VLAN on this port */
    if (BCM_E_NONE == result) {
        if (SOC_IS_SBX_G2P3(unit)) {
#ifdef BCM_FE2000_P3_SUPPORT

            if (_vlan_state[unit]->init) {
                soc_sbx_g2p3_evp2e_t_init(&p3egrVlanPort2Etc);
                result = BCM_E_NONE;
            } else {
                result = soc_sbx_g2p3_evp2e_get(unit,
                                                vid,
                                                port,
                                                &p3egrVlanPort2Etc);
            }

            if (BCM_E_NONE == result) {
                eteOld = p3egrVlanPort2Etc.eteptr;
            } else {
                VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:evp2e[%d,%03X]:"
                                        " %d (%s)\n"),
                          unit,
                          vid,
                          port,
                          result,
                          _SHR_ERRMSG(result)));
            }
#endif /* def BCM_FE2000_P3_SUPPORT */
        }
    } /* if (BCM_E_NONE == result) */

    /* set the new ET entry index */
    if (BCM_E_NONE == result) {
        if (_vlan_state[unit]->init ||
            (!isMember) ||
            (BCM_E_EXISTS == _bcm_fe2000_ete_is_standard(unit, eteOld))) {
            /*
             *  The system is initialising VLAN, the port is not yet marked
             *  as a member of the VLAN, or the port is using one of the
             *  standard ETEs already.  In this case, it is safe to set the
             *  ETE for this port,VLAN to one of the standard ETEs.
             */
            eteNew = untagged?SOC_SBX_PORT_UT_ETE(unit, port):
                              SOC_SBX_PORT_ETE(unit, port);
            if (SOC_IS_SBX_G2P3(unit)) {
#ifdef BCM_FE2000_P3_SUPPORT
                result = _bcm_fe2000_vlan_g2p3_untagged_set(unit, port,
                                                            vid, untagged);

                if (BCM_E_NONE == result) {
                    p3egrVlanPort2Etc.eteptr = eteNew;
                    result = soc_sbx_g2p3_evp2e_set(unit,
                                                    vid,
                                                    port,
                                                    &p3egrVlanPort2Etc);
                    if (BCM_E_NONE != result) {
                        VLAN_ERR((VLAN_FUNCMSG1("unable to update"
                                                " %d:epv2e[%d,%03X]:"
                                                " %d (%s)\n"),
                                  unit,
                                  vid,
                                  port,
                                  result,
                                  _SHR_ERRMSG(result)));
                    }
                }

#endif /* def BCM_FE2000_P3_SUPPORT */
            }

            if ((BCM_E_NONE == result) && (!_vlan_state[unit]->init)) {
                /* not init, and no error so far; dispose of old ET entry */
                result = _bcm_fe2000_vlan_free_ete(unit, eteOld);
                if (BCM_E_NONE != result) {
                    VLAN_ERR((VLAN_FUNCMSG1("unable to free unit %d ET entry"
                                            " %08X: %d (%s)\n"),
                              unit,
                              eteOld,
                              result,
                              _SHR_ERRMSG(result)));
                }
            } /* if not init and okay so far */

        } /* if initing or not member or using standard ET entries */
    } /* if (BCM_E_NONE == result) */

    /* set explicit membership flag, and some init-time-only fields */
    if (BCM_E_NONE == result) {
        if (SOC_IS_SBX_G2P3(unit)) {
#ifdef BCM_FE2000_P3_SUPPORT
            p3pv2appdata.member = TRUE;
#endif /* def BCM_FE2000_P3_SUPPORT */
        } else {
            result = VLAN_UNKNOWN_TYPE;
        }
    } /* if (BCM_E_NONE == result) */

    /* set up for ingress tagged frames (whether to allow or not) to the VID */
    if (BCM_PBMP_MEMBER(_vlan_state[unit]->tag_drop, port)) {
        /* in drop tagged state; don't allow it to be tagged */
        vidNew = 0;
    } else {
        /* not in drop tagged state; map incoming tag as expected */
        if ((!isMember) || _vlan_state[unit]->init) {
            /* not a member yet, or still initialising, so enable no trans */
            vidNew = SBX_VSI_FROM_VID(vid);
        } else {
            /* is already a member and not initiailising, keep translation */
            vidNew = vidOld;
        }
    }

    /* write ingress tag target and maybe other misc data during init */
    if (BCM_E_NONE == result) {
        if (SOC_IS_SBX_G2P3(unit)) {
#ifdef BCM_FE2000_P3_SUPPORT
            p3pVid2Etc.vlan = vidNew;

            /* DO NOT allocate a logical port per port,vid for TB! -
             * Use the port's default logical port for all VIDS.  There are
             * a small number of LPs and one per port,vid will run out quickly
             */
            _vlan_state[unit]->batch = 1;
            _vlan_state[unit]->batchDirty = 0;
            pvv2eCommit = 1;
            result = _i_soc_sbx_g2p3_pv2e_set(unit,
                                              port,
                                              vid,
                                              &p3pVid2Etc);
            _vlan_state[unit]->batch = 0;

            if (BCM_E_NONE != result) {
                VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:pv2e[%d,%03X]:"
                                        " %d (%s)\n"),
                          unit,
                          port,
                          vid,
                          result,
                          _SHR_ERRMSG(result)));
            }
            if (BCM_E_NONE == result) {
                result = SOC_SBX_G2P3_PV2APPDATA_SET(unit,
                                                     port,
                                                     vid,
                                                     &p3pv2appdata);
                if (BCM_E_NONE != result) {
                    VLAN_ERR((VLAN_FUNCMSG1("unable to write"
                                            " %d:pv2appdata[%d,%03X]:"
                                            " %d (%s)\n"),
                              unit,
                              port,
                              vid,
                              result,
                              _SHR_ERRMSG(result)));
                }
            }
#endif /* def BCM_FE2000_P3_SUPPORT */
        }
    } /* if (BCM_E_NONE == result) */

    /* tell STG code we've added this port to this VLAN */
    if (BCM_E_NONE == result) {
        _vlan_state[unit]->batch = 1;
        result = _bcm_fe2000_stg_vlan_port_add(unit,
                                               SBX_VSI_FROM_VID(vid),
                                               port);
        _vlan_state[unit]->batch = 0;

        if ((BCM_E_NOT_FOUND == result) && _vlan_state[unit]->init) {
            /* ignore BCM_E_NOT_FOUND during init */
            result = BCM_E_NONE;
        }
        if (BCM_E_NONE != result) {
            VLAN_ERR((VLAN_FUNCMSG1("_bcm_fe2000_stg_vlan_port_add(%d,%04X,%d)"
                                    " returned %d (%s)\n"),
                      unit,
                      SBX_VSI_FROM_VID(vid),
                      port,
                      result,
                      _SHR_ERRMSG(result)));
        }
    } /* if (BCM_E_NONE == result) */

    /* Commit ONLY if there is something to commit, otherwise the
     * entire pvv2e table is walked looking for dirty entries
     */
    if (pvv2eCommit && _vlan_state[unit]->batchDirty) {
        _vlan_state[unit]->batchDirty = 0;
        result = soc_sbx_g2p3_pvv2e_commit (unit, 0xffffffff);
        if (BCM_FAILURE(result)) {
            VLAN_ERR((VLAN_FUNCMSG1("failed to commit pvv2e changes on unit %d"
                                    ": %d (%s)\n"),
                      unit, result, _SHR_ERRMSG(result)));
        }
    }

    VLAN_EVERB((VLAN_FUNCMSG("(%d,%04X,%d,%s) return %d (%s)\n"),
                unit,
                vid,
                port,
                untagged?"TRUE":"FALSE",
                result,
                _SHR_ERRMSG(result)));

    /* all done; return the result */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *  Function
 *    _bcm_fe2000_map_vlan_gport_targets
 *  Purpose
 *    Get local module ID and target module ID / port ID from a gport that is
 *    supported by VLAN for inclusion in bcm_vlan_port_t.
 *  Parameters
 *    (in) int unit = unit number
 *    (in) bcm_gport_t gport = target port object
 *    (out) bcm_module_t *locMod = where to put local module ID
 *    (out) bcm_module_t *tgtMod = where to put target module ID
 *    (out) bcm_port_t *tgtPort = where to put target port ID
 *    (out) uint32 *portFte = FT entry index for the port
 *    (out) uint32 *portQueue = where to put the port queue ID
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* as appropriate if not successful
 *  Notes
 *    No parameter checking
 *    Must call only when the VLAN lock is owned
 *    TgtMod is assumed local if it is set to -1 on entry.
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_map_vlan_gport_targets"
static int
_bcm_fe2000_map_vlan_gport_targets(int unit,
                                   bcm_gport_t gport,
                                   bcm_module_t *locMod,
                                   bcm_module_t *tgtMod,
                                   bcm_port_t *tgtPort,
                                   uint32 *portFte,
                                   uint32 *portQueue)
{
    int                       result;               /* local result code */
    int                       gpType;               /* port type */
    int                       doFtLookup = TRUE;    /* need FT/Queue lookup */
    uint32                    logicalPort;          /* logical port ID */
    uint32                    gportId, gu2_res, fti;
    _bcm_fe2k_vlan_gport_t    *vlanGPData = NULL;   /* VLAN GPORT  data */

    VLAN_VERB((VLAN_FUNCMSG("(%d,%08X,*,*,*,*,*) enter\n"), unit, gport));

    /* figure out my module information */
    result = bcm_stk_my_modid_get(unit, locMod);

    /* figure out the target module and port information */
    if (BCM_E_NONE == result) {
        /* figure out the target port type */
        gpType = (gport >> _SHR_GPORT_TYPE_SHIFT) & _SHR_GPORT_TYPE_MASK;
        /* parse target port as appropriate for its type */
        switch (gpType) {
        case BCM_GPORT_TYPE_NONE:
            /* assumed to be local port on (maybe) local module */
            if (-1 == *tgtMod) {
                *tgtMod = *locMod;
            }
            *tgtPort = gport;
            break;
        case BCM_GPORT_TYPE_LOCAL:
            /* explicitly local port, set module information */
            *tgtMod = *locMod;
            *tgtPort = BCM_GPORT_LOCAL_GET(gport);
            break;
        case BCM_GPORT_TYPE_TRUNK:
            if (portFte) {
                *portFte = SOC_SBX_TRUNK_FTE(unit, BCM_GPORT_TRUNK_GET(gport));
            }
            doFtLookup = FALSE;
            break;
        case BCM_GPORT_TYPE_MODPORT:
            /* explicit module and port specification */
            *tgtMod = BCM_GPORT_MODPORT_MODID_GET(gport);
            *tgtPort = BCM_GPORT_MODPORT_PORT_GET(gport);
            break;
        case BCM_GPORT_VLAN_PORT:
            /* VLAN GPORT */
            /* get the gport information */
            gportId = BCM_GPORT_VLAN_PORT_ID_GET(gport);
            fti = VLAN_VGPORT_ID_TO_FT_INDEX(unit, gportId);

            gu2_res = SBX_GU2K_USR_RES_FTE_LOCAL_GPORT;
            result = _sbx_gu2_resource_test(unit, gu2_res, fti);

            if (BCM_E_EXISTS == result) {
                logicalPort = _vlan_state[unit]->gportInfo.lpid[gportId];
                result = BCM_E_NONE;
                vlanGPData = (_bcm_fe2k_vlan_gport_t*)
                             (SBX_LPORT_DATAPTR(unit, logicalPort));
                if (!vlanGPData) {
                    VLAN_ERR((VLAN_FUNCMSG1("gport %08X is not valid on unit %d\n"),
                              gport,
                              unit));
                    result = BCM_E_NOT_FOUND;
                }
                if (BCM_GPORT_VLAN_PORT != SBX_LPORT_TYPE(unit, logicalPort)) {
                    VLAN_ERR((VLAN_FUNCMSG1("unit %d gport %08X disagrees with"
                                            " stored type %02X\n"),
                              unit,
                              gport,
                              SBX_LPORT_TYPE(unit, logicalPort)));
                    result = BCM_E_CONFIG;
                }
            } else {
                VLAN_ERR((VLAN_FUNCMSG1("unable to access unit %d GPORT %08X:"
                                        " %d (%s)\n"),
                          unit,
                          gport,
                          result,
                          _SHR_ERRMSG(result)));
            }

            /* now we have this VLAN GPORT's target GPORT; decode THAT... */
            if (BCM_E_NONE == result) {
                result = _bcm_fe2000_map_vlan_gport_targets(unit,
                                                            vlanGPData->p.port,
                                                            locMod, tgtMod,
                                                            tgtPort, portFte,
                                                            portQueue);

                if (portFte) {
                    /* Not traditional bridging; replace FTEID/PID */
                    *portFte = fti;
                }
            }
            doFtLookup = FALSE;
            break;
        default:
            /* don't support anything else at this time */
            result = BCM_E_CONFIG;
            VLAN_ERR((VLAN_FUNCMSG1("gport %08X is unsupported type %d\n"),
                      gport,
                      gpType));
        }
    } else {
        VLAN_ERR((VLAN_FUNCMSG1("unable to get local module ID: %d (%s)\n"),
                  result,
                  _SHR_ERRMSG(result)));
    }

    /* get the queue for the chosen target port */
    if ((BCM_E_NONE == result) && (portFte || portQueue) && doFtLookup) {
        result = _bcm_fe2000_modPort_to_ftEntry(unit,
                                                *tgtMod,
                                                *tgtPort,
                                                portFte,
                                                portQueue);
        if (BCM_E_NONE != result) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to get module %d port %d queue:"
                                    " %d (%s)\n"),
                      *tgtMod,
                      *tgtPort,
                      result,
                      _SHR_ERRMSG(result)));
        }
    }

    VLAN_VERB((VLAN_FUNCMSG("(%d,%08X,&(%d),&(%d),&(%d),&(%08X),&(%08X))"
                            " return %d (%s)\n"),
               unit,
               gport,
               *locMod,
               *tgtMod,
               *tgtPort,
               portFte?(*portFte):0xFFFFFFFF,
               portQueue?(*portQueue):0xFFFFFFFF,
               result,
               _SHR_ERRMSG(result)));

    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *  Function
 *    _bcm_fe2000_map_vlan_gport_target
 *  Purpose
 *    Get local module ID and target module ID / port ID from a gport that is
 *    supported by VLAN for inclusion in bcm_vlan_port_t.
 *  Parameters
 *    (in) int unit = unit number
 *    (in) bcm_gport_t gport = target port object
 *    (out) bcm_module_t *locMod = where to put local module ID
 *    (out) bcm_module_t *tgtMod = where to put target module ID
 *    (out) bcm_port_t *tgtPort = where to put target port ID
 *    (out) uint32 *portFte = FT entry index for the port
 *    (out) uint32 *portQueue = where to put the port queue ID
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* as appropriate if not successful
 *  Notes
 *    No parameter checking
 *    Must call this one from outside VLAN context
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_map_vlan_gport_target"
int
_bcm_fe2000_map_vlan_gport_target(int unit,
                                  bcm_gport_t gport,
                                  bcm_module_t *locMod,
                                  bcm_module_t *tgtMod,
                                  bcm_port_t *tgtPort,
                                  uint32 *portFte,
                                  uint32 *portQueue)
{
    int                       result;               /* local result code */

    /* get the vlan lock */
    VLAN_LOCK_TAKE;

    result = _bcm_fe2000_map_vlan_gport_targets(unit,
                                                gport,
                                                locMod,
                                                tgtMod,
                                                tgtPort,
                                                portFte,
                                                portQueue);

    /* release the lock */
    VLAN_LOCK_RELEASE;

    return result;
}
#undef _VLAN_FUNC_NAME


#ifdef BCM_FE2000_P3_SUPPORT
#define _VLAN_FUNC_NAME "_bcm_fe2000_g2p3_vlan_port_vlan_vector_update"
STATIC int
_bcm_fe2000_g2p3_vlan_port_vlan_vector_update(int               unit,
                                              bcm_vlan_port_t  *vlan_port,
                                              bcm_port_t        phy_port,
                                              int               create)
{
    int                  result = BCM_E_NONE;
    soc_sbx_g2p3_epv2e_t p3epv2e;
    bcm_vlan_t           vid;
    bcm_vlan_vector_t    vlan_vec;

    if (vlan_port->criteria != BCM_VLAN_PORT_MATCH_PORT_VLAN) {
        return BCM_E_NONE;
    }

    soc_sbx_g2p3_epv2e_t_init(&p3epv2e);
    result = soc_sbx_g2p3_epv2e_get(unit,
                                    vlan_port->match_vlan,
                                    phy_port,
                                    &p3epv2e);
    if (result == BCM_E_NONE) {
        if (create) {
            p3epv2e.strip =
               (vlan_port->flags & BCM_VLAN_PORT_EGRESS_UNTAGGED ? 1 : 0);
        } else {
            p3epv2e.strip = FALSE;
        }
        result = soc_sbx_g2p3_epv2e_set(unit,
                                        vlan_port->match_vlan,
                                        phy_port,
                                        &p3epv2e);
    }

    BCM_VLAN_VEC_ZERO(vlan_vec);
    result = _bcm_fe2000_vlan_port_vlan_vector_get(unit,
                                                   vlan_port->vlan_port_id,
                                                   vlan_vec);
    if (result != BCM_E_NONE) {
        /* ignore if there is no vector */
        return BCM_E_NONE;
    }

    /* there may be more than one c-vid registered */
    for (vid = BCM_VLAN_MIN + 1; vid < BCM_VLAN_MAX; vid++) {
         if (!BCM_VLAN_VEC_GET(vlan_vec, vid)) {
             continue;
         }

         soc_sbx_g2p3_epv2e_t_init(&p3epv2e);
         result = soc_sbx_g2p3_epv2e_get(unit, vid, phy_port, &p3epv2e);
         if (result == BCM_E_NONE) {
             if (create) {
                 p3epv2e.strip =
                   (vlan_port->flags & BCM_VLAN_PORT_EGRESS_UNTAGGED ? 1 : 0);
             } else {
                 p3epv2e.strip = FALSE;
             }
             result = soc_sbx_g2p3_epv2e_set(unit, vid, phy_port, &p3epv2e);
         }

         if (result != BCM_E_NONE) {
             VLAN_ERR((VLAN_FUNCMSG1("unable to read/write epv2e %d: port %d vid %d:"
                                    " %d (%s)\n"),
                      unit,
                      phy_port,
                      vid,
                      result,
                      _SHR_ERRMSG(result)));
         }
    }

    return BCM_E_NONE;
}
#undef _VLAN_FUNC_NAME
#endif /* BCM_FE2000_P3_SUPPORT */


/*
 *   Function
 *      _bcm_fe2000_vp_oi_resolve
 *   Purpose
 *      Reserve or allocate an OHI for a vlan gport
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vp_oi_resolve"
static int
_bcm_fe2000_vp_oi_resolve(int unit, _bcm_fe2k_vlan_gport_t *vlan_port,
                          soc_sbx_g2p3_oi2e_t *oi2e, uint32 *ohi, int *allocOhi)
{
    int      rv = BCM_E_NONE;
    int      allocFlags = 0;

    if ((vlan_port->e.flags & _BCM_FE2K_VLAN_GP_EXTERN_OHI) ||
        (vlan_port->p.flags & BCM_VLAN_PORT_REPLACE)        ||
        (vlan_port->p.flags & BCM_VLAN_PORT_ENCAP_WITH_ID))
    {
        /* Externally managed encap, or replacing, determine type and decode.
         */
        if (SOC_SBX_IS_VALID_L2_ENCAP_ID(vlan_port->p.encap_id)) {
            *ohi = SOC_SBX_OHI_FROM_L2_ENCAP_ID(vlan_port->p.encap_id);
        } else if (SOC_SBX_IS_VALID_ENCAP_ID(vlan_port->p.encap_id)) {
            *ohi = SOC_SBX_OHI_FROM_ENCAP_ID(vlan_port->p.encap_id);
        } else {
            VLAN_ERR((VLAN_FUNCMSG1("unexpected encap_id 0x%08X\n"),
                      vlan_port->p.encap_id));
            return BCM_E_INTERNAL;
        }

        allocFlags = _SBX_GU2_RES_FLAGS_RESERVE;
    }
    
    *allocOhi = FALSE;

    if (!((vlan_port->e.flags & _BCM_FE2K_VLAN_GP_EXTERN_OHI) ||
          (vlan_port->p.flags & BCM_VLAN_PORT_REPLACE))) 
    {        
        rv = _sbx_gu2_resource_alloc(unit, SBX_GU2K_USR_RES_OHI, 1,
                                     ohi, allocFlags);
        
        if (rv == BCM_E_RESOURCE) {
            VLAN_VERB((VLAN_FUNCMSG1("EncapId 0x%08x found in "
                                     " reserved range\n"),
                       vlan_port->p.encap_id));
            rv = BCM_E_NONE;
        }
        
        if (BCM_SUCCESS(rv)) {
            *allocOhi = TRUE;
        } else {
            VLAN_ERR((VLAN_FUNCMSG1("unable to allocate OH entry on"
                                    " unit %d: %d (%s)\n"),
                      unit, rv, _SHR_ERRMSG(rv)));
            return rv;
        }
    } 

    /* set the descriptor's encap_id value */
    vlan_port->p.encap_id = SOC_SBX_ENCAP_ID_FROM_OHI(*ohi);

    if (*allocOhi) {
        soc_sbx_g2p3_oi2e_t_init(oi2e);
    } else {
        rv = soc_sbx_g2p3_oi2e_get(unit, *ohi - SBX_RAW_OHI_BASE, oi2e);
    
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:oi2e[0x%08X]:"
                                    " %d (%s)\n"),
                      unit, *ohi, rv, _SHR_ERRMSG(rv)));
            return rv;
        }
    }

    VLAN_VERB((VLAN_FUNCMSG1("%s encapId 0x%08x/OHI 0x%0x\n"),
               (*allocOhi) ? "Allocated" : "Using", vlan_port->p.encap_id, 
               SOC_SBX_OHI_FROM_ENCAP_ID(vlan_port->p.encap_id)));
    
    return rv;
}
#undef _VLAN_FUNC_NAME


/*
 *   Function
 *      _bcm_fe2000_vp_etes_alloc
 *   Purpose
 *      Reserve or allocate Ete Encap & Ete L2 for a vlan gport
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vp_etes_alloc"
static int
_bcm_fe2000_vp_etes_alloc(int unit, _bcm_fe2k_vlan_gport_t *vlan_port,
                          soc_sbx_g2p3_eteencap_t *eteEncap,
                          soc_sbx_g2p3_etel2_t *eteL2,
                          uint32 *eteEncapIndex, uint32 *eteL2Index,
                          int *allocEteEncap, int *allocEteL2)
{
    int            rv;

    *allocEteEncap = *allocEteL2 = FALSE;

    if ((vlan_port->e.flags & _BCM_FE2K_VLAN_GP_EXTERN_OHI) ||
        (vlan_port->p.flags & BCM_VLAN_PORT_REPLACE)) {
        /* replacing; use existing ET entries */
        rv = soc_sbx_g2p3_eteencap_get(unit, *eteEncapIndex, eteEncap);
        
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:eteencap[0x%08X]:"
                                    " %d (%s)\n"),
                      unit, *eteEncapIndex, rv, bcm_errmsg(rv)));
            return rv;
        }

        *eteL2Index = eteEncap->l2ete;
        rv = soc_sbx_g2p3_etel2_get(unit, *eteL2Index, eteL2);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:etel2[0x%08X]:"
                                    " %d (%s)\n"),
                      unit, *eteL2Index, rv, bcm_errmsg(rv)));
            return rv;
        }

    } else {

        /* not replacing; get new ET entries */
        rv = _sbx_gu2_resource_alloc(unit, SBX_GU2K_USR_RES_ETE_ENCAP,
                                     1, eteEncapIndex, 0);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to allocate ETE Encap entry on"
                                    " unit %d: %d (%s)\n"),
                      unit, rv, bcm_errmsg(rv)));
            return rv;
        }
        *allocEteEncap = TRUE;

        rv = _sbx_gu2_resource_alloc(unit, SBX_GU2K_USR_RES_ETE_L2,
                                     1, eteL2Index, 0);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to allocate ETE L2 entry on"
                                    " unit %d: %d (%s)\n"),
                      unit, rv, bcm_errmsg(rv)));
            return rv;
        }
        *allocEteL2 = TRUE;

        soc_sbx_g2p3_etel2_t_init(eteL2);
        soc_sbx_g2p3_eteencap_t_init(eteEncap);

    } /* if (vlan_port->p.flags & BCM_VLAN_PORT_REPLACE) */
    
    VLAN_VERB((VLAN_FUNCMSG1("%s ETEEncap 0x%08x ETE L2 0x%08x\n"),
               (*allocEteEncap) ? "Allocated" : "Using", 
               *eteEncapIndex, *eteL2Index));
    
    return rv;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vp_egress_config
 *   Purpose
 *      Configure and commit to hardware the egress path 
 *      (ohi, ete encap, etel2), for a vlan gport
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vp_egress_config"
static int
_bcm_fe2000_vp_egress_config(int unit, _bcm_fe2k_vlan_gport_t *vlan_port,
                             soc_sbx_g2p3_oi2e_t *oi2e,
                             soc_sbx_g2p3_eteencap_t *eteEncap,
                             soc_sbx_g2p3_etel2_t *eteL2,
                             uint32 ohi, 
                             uint32 eteEncapIndex, 
                             uint32 eteL2Index,
                             uint32 pid)
{
    int                   rv = BCM_E_NONE;
    bcm_vlan_port_match_t egressMode;
    soc_sbx_g2p3_tpid_t   tpid;

    eteEncap->pid          = pid;
    eteEncap->etepid       = TRUE;
    eteEncap->l2ete        = eteL2Index;
    eteEncap->dmacset      = FALSE;
    eteEncap->smacset      = FALSE;
    eteEncap->ipttldec     = FALSE;
    eteEncap->ttlcheck     = FALSE;
    eteEncap->nosplitcheck = FALSE;
    
    
    eteL2->mtu = SBX_DEFAULT_MTU_SIZE;
    eteL2->nostrip = 
        !!(vlan_port->p.flags & BCM_VLAN_PORT_INNER_VLAN_PRESERVE);
    eteL2->nosplitcheck = FALSE;

    /* figure out the egress mode */
    if (vlan_port->p.flags & BCM_VLAN_PORT_EGRESS_UNTAGGED ||
        ((vlan_port->p.flags & BCM_VLAN_PORT_INNER_VLAN_PRESERVE) &&
         !(vlan_port->p.flags & BCM_VLAN_PORT_INNER_VLAN_ADD) &&
         (vlan_port->p.criteria != BCM_VLAN_PORT_MATCH_PORT_VLAN_STACKED))) {
        /* if UNTAGGED selected, force egress as untagged
         * if no inner vlan is added to untagged packets when ctags are
         * preserved, also egress untagged
         */
        egressMode = BCM_VLAN_PORT_MATCH_PORT;
    } else {
        /* otherwise, guess based upon the input match mode */
        egressMode = vlan_port->p.criteria;
    }

    /* configure egress path for the egress mode */
    switch (egressMode) {
    case BCM_VLAN_PORT_MATCH_PORT:
        /* egress without adding tags */
        eteL2->usevid = TRUE;
        eteL2->vid = _BCM_VLAN_G2P3_UNTAGGED_VID;
        break;

    case BCM_VLAN_PORT_MATCH_PORT_VLAN:
        /* egress with one tag */
        eteL2->usevid = TRUE;
        eteL2->vid = vlan_port->p.egress_vlan;
        break;

    case BCM_VLAN_PORT_MATCH_PORT_VLAN_STACKED:
        /* egress with two tags */
        eteL2->usevid = TRUE;
        eteL2->vid = vlan_port->p.egress_vlan;
        rv = soc_sbx_g2p3_tpid_get(unit, SB_G2P3_FE_CTPID_INDEX, &tpid);

        eteEncap->encaplen = 4;
        eteEncap->tpid = tpid.tpid;
        eteEncap->vid = vlan_port->p.egress_inner_vlan;
        break;

    default:
        rv = BCM_E_CONFIG;
        VLAN_ERR((VLAN_FUNCMSG1("unsupported match mode %d\n"), egressMode));
    } /* switch (egressMode) */

    if (BCM_FAILURE(rv)) {
        return rv;
    }

    rv = soc_sbx_g2p3_etel2_set(unit, eteL2Index, eteL2);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:etel2[0x%08X]: %d (%s)\n"),
                  unit, eteL2Index, rv, bcm_errmsg(rv)));
        return rv;
    }

    rv = soc_sbx_g2p3_eteencap_set(unit, eteEncapIndex, eteEncap);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:eteEncap[0x%08X]: "
                                "%d (%s)\n"),
                  unit, eteEncapIndex, rv, bcm_errmsg(rv)));
        return rv;
    }

    if (!(vlan_port->e.flags & _BCM_FE2K_VLAN_GP_EXTERN_OHI)) {
        oi2e->eteptr = eteEncapIndex;
        rv = soc_sbx_g2p3_oi2e_set(unit, ohi - SBX_RAW_OHI_BASE, oi2e);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:oi2e[0x%08X]:"
                                    " %d (%s)\n"),
                      unit, ohi, rv, bcm_errmsg(rv)));
            return rv;
        }
    }

    VLAN_VERB((VLAN_FUNCMSG1("Updated OI[0x%08x] ETEEncap[0x%08x] "
                             "ETE L2 [0x%08x]\n"),
               ohi, eteEncapIndex, eteL2Index));
    
    return rv;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vp_fte_config
 *   Purpose
 *      Configure and commit to hardware the FTE for a vlan gport
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vp_fte_config"
static int
_bcm_fe2000_vp_fte_config(int unit, _bcm_fe2k_vlan_gport_t *vlan_port,
                          uint32 fti, uint32 foverFti,
                          uint32 tgtQueue, uint32 foverTgtQueue,
                          uint32 ohi, 
                          uint32 lagbase, uint32 lagsize)
{
    int                    rv;
    soc_sbx_g2p3_ft_t      fte, foverFte;
    int                    foverPrimary;

    foverPrimary = fti < foverFti;
    if (vlan_port->p.flags & BCM_VLAN_PORT_REPLACE) {
        rv = soc_sbx_g2p3_ft_get(unit, fti, &fte);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to read existing %d:FT[0x%08X]:"
                                    " %d (%s)\n"),
                      unit, fti, rv, bcm_errmsg(rv)));
            return rv;
        }
    } else {
        soc_sbx_g2p3_ft_t_init(&fte);
    }

    fte.excidx  = 0;
    fte.oi      = ohi;
    fte.qid     = tgtQueue;  /* will be zero for Trunked gports */
    fte.lag     = BCM_GPORT_IS_TRUNK(vlan_port->p.port);
    fte.lagbase = lagbase;
    fte.lagsize = lagsize;
    
    if (foverPrimary) {
        rv = soc_sbx_g2p3_ft_get(unit, foverFti, &foverFte);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to read failover %d:fte[0x%08X]:"
                                    " %d (%s)\n"),
                      unit, foverFti, rv, bcm_errmsg(rv)));
            return rv;
        }

        fte.rridx    = vlan_port->p.failover_id;
        fte.oib      = foverFte.oi;
        fte.qidb     = foverTgtQueue;
        fte.lagbaseb = foverFte.lagbase;
        fte.lagsizeb = foverFte.lagsize;
        fte.lagb     = foverFte.lag;
    }

    rv = soc_sbx_g2p3_ft_set(unit, fti, &fte);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:ft[0x%08X]: %d (%s)\n"),
                  unit, fti, rv, bcm_errmsg(rv)));
        return rv;
    }
    
    return rv;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vp_failover_egress_config
 *   Purpose
 *      Configure and commit to hardware the failover FTE for a vlan gport
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vp_failover_egress_config"
static int
_bcm_fe2000_vp_failover_egress_config(int unit, 
                                      _bcm_fe2k_vlan_gport_t *vlan_port, 
                                      uint32 pid)
{

    int                       rv;
    uint32                    foverGportId, lpi, ohi;
    uint32                    eteEncapIdx;
    _bcm_fe2k_vlan_gport_t   *foverGpPtr;
    soc_sbx_g2p3_oi2e_t       oi2e;
    soc_sbx_g2p3_eteencap_t   eteEncap;
    soc_sbx_g2p3_lp_t         lp;

    foverGportId =
        BCM_GPORT_VLAN_PORT_ID_GET(vlan_port->p.failover_port_id);
    lpi = _vlan_state[unit]->gportInfo.lpid[foverGportId];

    foverGpPtr = (_bcm_fe2k_vlan_gport_t *)(SBX_LPORT_DATAPTR(unit, lpi));

    if (SOC_SBX_IS_VALID_L2_ENCAP_ID(foverGpPtr->p.encap_id)) {
        ohi = SOC_SBX_OHI_FROM_L2_ENCAP_ID(foverGpPtr->p.encap_id);
    } else if (SOC_SBX_IS_VALID_ENCAP_ID(foverGpPtr->p.encap_id)) {
        ohi = SOC_SBX_OHI_FROM_ENCAP_ID(foverGpPtr->p.encap_id);
    } else {
        VLAN_ERR((VLAN_FUNCMSG1("unexpected encap_id %08X\n"),
                  foverGpPtr->p.encap_id));
        return BCM_E_INTERNAL;
    }

    /* Get the failover oi2e (for the encap ETE pointer) */
    rv = soc_sbx_g2p3_oi2e_get(unit, ohi - SBX_RAW_OHI_BASE, &oi2e);

    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to read failover %d:oi2e[0x%08X]:"
                                " %d (%s)\n"),
                  unit, ohi, rv, bcm_errmsg(rv)));
        return rv;
    }

    /* Get the failover encap ETE pointer */
    eteEncapIdx = oi2e.eteptr;
    rv = soc_sbx_g2p3_eteencap_get(unit, eteEncapIdx, &eteEncap);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to read failover %d:eteEncap[0x%08X]:"
                                " %d (%s)\n"),
                  unit, eteEncapIdx, rv, bcm_errmsg(rv)));
        return rv;
    }
        
    /* Update the pid in the failover encap ETE pointer */
    eteEncap.pid = pid;
    rv = soc_sbx_g2p3_eteencap_set(unit, eteEncapIdx, &eteEncap);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to write failover %d:eteEncap[0x%08X]"
                                ": %d (%s)\n"),
                  unit, eteEncapIdx, rv, bcm_errmsg(rv)));
        return rv;
    }

    /* Get the failover lp entry */
    rv = soc_sbx_g2p3_lp_get(unit, lpi, &lp);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to get failover lp %d:lp[0x%08X]"
                                ": %d (%s)\n"),
                  unit, lpi, rv, bcm_errmsg(rv)));
        return rv;
    }

    /* Update the pid in the failover lp entry */
    lp.pid = pid;
    rv = soc_sbx_g2p3_lp_set(unit, lpi, &lp);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to set failover lp %d:"
                                "lp[0x%08X]: %d (%s)\n"),
                  unit, lpi, rv, bcm_errmsg(rv)));
        return rv;
    }
    
    return rv;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vp_lp_config
 *   Purpose
 *      Configure and commit to hardware the logical port for a vlan gport
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vp_lp_config"
static int
_bcm_fe2000_vp_lp_config(int unit, _bcm_fe2k_vlan_gport_t *vlan_port,
                         uint32 lpi, uint32 pid)
{
    int                  rv;
    soc_sbx_g2p3_lp_t    lp;
    uint32               qosMap = 0;

    /* verify the ingress QoS map, if we've been given one */
    if (vlan_port->p.qos_map_id) {
        rv = _bcm_fe2000_qos_map_validate(unit, vlan_port->p.qos_map_id,
                                          &qosMap);
    
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("invalid qos map: %d (%s)\n"), 
                      rv, bcm_errmsg(rv)));
            return rv;
        }
    }

    /* re-read what was configured on replace
     * so we don't lose lp.qos!
     */
    if (vlan_port->p.flags & BCM_VLAN_PORT_REPLACE) {

        rv = soc_sbx_g2p3_lp_get(unit, lpi, &lp);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("failed to read lp[0x%08x]: %d %s\n"),
                      lpi, rv, bcm_errmsg(rv)));
            return rv;
        }
    } else {
        soc_sbx_g2p3_lp_t_init(&lp);
    }
    
    rv = _bcm_fe2000_g2p3_policer_lp_program(unit, 
                                             vlan_port->p.policer_id, &lp);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to program lp %d:lp[0x%08X]: %d "
                                "(%s)\n"), unit, lpi, rv, bcm_errmsg(rv)));
        return rv;
    }

    if (vlan_port->p.qos_map_id) {
        /* caller provided a specifc QoS map; use it for ingress */
        lp.qos = qosMap;
    }

    lp.pid = pid;
    rv = soc_sbx_g2p3_lp_set(unit, lpi, &lp);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:lp[0x%08X]: %d (%s)\n"),
                  unit, lpi, rv, bcm_errmsg(rv)));
        return rv;
    }

    return rv;
}
#undef _VLAN_FUNC_NAME

#ifdef BCM_FE2000_P3_SUPPORT
/*
 *   Function
 *      _bcm_fe2000_g2p3_vlan_port_create
 *   Purpose
 *      Create a logical port and commit to hardware for g2p3
 *   Parameters
 *      (in)  unit        = BCM device number
 *      (in)  flags       = flags to affect port creation
 *      (in)  vlan_port   = description of port to be created
 *      (in)  logicalPort = logical port ID to use
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *
 *  Notes:
 *     Assumes all params are valid and proper locking has occured
 *     Uses an Encap ETE to reconcile the pid fields.  If this were exposed in
 *     the standard L2 ETE, would not require the Encap ETE at all.
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_g2p3_vlan_port_create"
static int
_bcm_fe2000_g2p3_vlan_port_create(int unit,
                                  _bcm_fe2k_vlan_gport_t *vlan_port,
                                  uint32 logicalPort)
{
    int                       rv;
    bcm_port_t                ignorePort;
    bcm_module_t              ignoreMod = ~0;
    uint32                    tgtQueue, foverTgtQueue;
    uint32                    trunkFti, foverFti, fti;
    uint32                    eteEncapIdx = ~0, eteL2Idx = ~0;
    uint32                    ohi = 0;
    soc_sbx_g2p3_ft_t         trunkFte;
    soc_sbx_g2p3_oi2e_t       oi2e;
    soc_sbx_g2p3_eteencap_t   eteEncap;
    soc_sbx_g2p3_etel2_t      eteL2;
    int                       allocOhi, allocEteEncap, allocEteL2;
    int                       foverPrimary = FALSE;

    soc_sbx_g2p3_ft_t_init(&trunkFte);
    tgtQueue = 0;

    rv = _bcm_fe2000_map_vlan_gport_targets(unit, vlan_port->p.port,
                                            &ignoreMod, &ignoreMod,
                                            &ignorePort, &trunkFti, &tgtQueue);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to map gport 0x%08X targets: "
                                "%d (%s)\n"),
                  vlan_port->p.port, rv, bcm_errmsg(rv)));
        return rv;
    }
    
    if (BCM_GPORT_IS_TRUNK(vlan_port->p.port)) {
        rv = soc_sbx_g2p3_ft_get(unit, trunkFti, &trunkFte);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("Failed to get trunk ft[0x%x]: %s\n"),
                      trunkFti, bcm_errmsg(rv)));
            return rv;
        }
    }

    foverFti = 0;
    if (vlan_port->p.failover_id) {
        rv = _bcm_fe2000_map_vlan_gport_targets(unit, 
                                                vlan_port->p.failover_port_id,
                                                &ignoreMod, &ignoreMod,
                                                &ignorePort, &foverFti,
                                                &foverTgtQueue);

        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to map failover gport 0x%08X"
                                    " targets: %d (%s)\n"),
                      vlan_port->p.failover_port_id, rv, bcm_errmsg(rv)));
            return rv;
        }
    }

    allocOhi = allocEteEncap = allocEteL2 = 0;

    /* Allocate/gather and configure the egress path. */
    fti = BCM_GPORT_VLAN_PORT_ID_GET(vlan_port->p.vlan_port_id);
    fti = VLAN_VGPORT_ID_TO_FT_INDEX(unit, fti);

    /*
     * Failover API protocol requires the primary GPORT is allocated
     * before the backup GPORT (i.e. primary fte idx < backup fte idx)
     * g2p2 required these ftes to be contiguous as well.  g2p3 doesn't
     * require this, but the FTE ordering is a side-effect of the
     * requirement that IS required here (needed to determine which
     * GPORT PID is used for the pair, and also which GPORT is used
     * when rr[rridx] == 0)
     */
    foverPrimary = fti < foverFti;

    if (BCM_SUCCESS(rv)) {
        rv = _bcm_fe2000_vp_oi_resolve(unit, vlan_port,
                                       &oi2e, &ohi, &allocOhi);
    }

    /* Egress path (fte, ohi, etes) is ALWAYS allocated and configured to
     * simplify Gports over LAG when port membership changes.
     */
    if (BCM_SUCCESS(rv)) {
        /* eteEncapIdx is used on replace, otherwise ignored */
        eteEncapIdx = oi2e.eteptr;
        rv = _bcm_fe2000_vp_etes_alloc(unit, vlan_port,
                                       &eteEncap,      &eteL2,
                                       &eteEncapIdx,   &eteL2Idx,
                                       &allocEteEncap, &allocEteL2);
    }
    
    if (BCM_SUCCESS(rv)) {
        /* Configure the OI and ETEs; using the FTI as the PID */
        rv = _bcm_fe2000_vp_egress_config(unit, vlan_port, 
                                          &oi2e, &eteEncap, &eteL2,
                                          ohi, eteEncapIdx, eteL2Idx,
                                          fti);
    }
    
    VLAN_VERB((VLAN_FUNCMSG1("LP=0x%08X FTE=0x%08X OI=0x%08X ETEEC=0x%08X"
                             " ETEL2=0x%08X u=%d\n"),
               logicalPort, fti, ohi, eteEncapIdx, eteL2Idx, unit));
    
    /* configure failover path, if active */
    if (BCM_SUCCESS(rv) && foverPrimary) {
        rv = _bcm_fe2000_vp_failover_egress_config(unit, vlan_port, fti);            
    }

    if (BCM_SUCCESS(rv)) {
        /* the caller allocated the FTE, configure it here */
        rv = _bcm_fe2000_vp_fte_config(unit, vlan_port, fti, foverFti,
                                       tgtQueue, foverTgtQueue, ohi,
                                       trunkFte.lagbase, trunkFte.lagsize);
    }

    if (BCM_SUCCESS(rv)) {
        rv = _bcm_fe2000_vp_lp_config(unit, vlan_port, logicalPort, fti);
    }

    if (BCM_FAILURE(rv)) {
        if (allocOhi) {
            _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_OHI, 
                                   1, &ohi, 0);
        }

        if (allocEteL2) {
            _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_ETE_L2, 
                                   1, &eteL2Idx, 0);
        }

        if (allocEteEncap) {
            _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_ETE_ENCAP,
                                   1, &eteEncapIdx, 0);
        }
    }

    return rv;

}
#undef _VLAN_FUNC_NAME
#endif /* BCM_FE2000_P3_SUPPORT */

/*
 *   Function
 *      _bcm_fe2000_vgp_resolve_internal
 *   Purpose
 *      Obtain a vlan gport's logical port index and internal state data
 *   Parameters
 *      (in)  unit        = BCM device number
 *      (in)  gport       = vlan gport id to resolve
 *      (out) plpi        = storage location for logical port index
 *      (out) gp          = storage locaiton for internal state pointer
 *   Returns
 *       BCM_E_* 
 *
 *  Notes:
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vgp_resolve_internal"
static int 
_bcm_fe2000_vgp_resolve_internal(int unit, bcm_gport_t gport, 
                                 uint32 *plpi, 
                                 _bcm_fe2k_vlan_gport_t **gp)
{
    int       lpi;
    uint32    gportId;
                                          
    gportId = BCM_GPORT_VLAN_PORT_ID_GET(gport);
    lpi = _vlan_state[unit]->gportInfo.lpid[gportId];
    if (SBX_LPORT_TYPE(unit, lpi) == BCM_GPORT_VLAN_PORT) {
        if (SBX_LPORT_DATAPTR(unit, lpi) == NULL) {
            return BCM_E_INTERNAL;
        }
    } else {
        VLAN_ERR((VLAN_FUNCMSG1("Gport 0x%08x not a configured vlan gport"
                                ", type=%d\n"),
                  gport, SBX_LPORT_TYPE(unit, lpi)));
        return  BCM_E_PARAM;
    }
    
    if (gp) {
        (*gp) = (_bcm_fe2k_vlan_gport_t*)(SBX_LPORT_DATAPTR(unit, lpi));
    }
    if (plpi) {
        *plpi = lpi;
    }
    
    return BCM_E_NONE;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *     _bcm_fe2000_vgp_get_lp_encap
 *   Purpose
 *      Obtain the logical port and out header index for the given vlan gport
 *   Parameters
 *      (in)  unit        = BCM device number
 *      (in)  gport       = vlan gport to query
 *      (out) lpi         = storage location for logical port index
 *      (out) ohi         = storage locaiton for out header index
 *   Returns
 *       BCM_E_* 
 *
 *  Notes:
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vgp_get_lp_encap"
static int
_bcm_fe2000_vgp_get_lp_encap(int unit, bcm_gport_t gport,
                             uint32 *lpi, uint32* ohi)
{
    int rv;
    int gportId;
    _bcm_fe2k_vlan_gport_t *gp;
    uint32                  fti;
    soc_sbx_g2p3_ft_t       fte;

    rv = _bcm_fe2000_vgp_resolve_internal(unit, gport, lpi, &gp);
    if (BCM_FAILURE(rv)) {
        return rv;
    }
    
    gportId = BCM_GPORT_VLAN_PORT_ID_GET(gport);
    fti = VLAN_VGPORT_ID_TO_FT_INDEX(unit, gportId);
        
    rv = soc_sbx_g2p3_ft_get(unit, fti, &fte);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("Failed to read FT[0x%08x]: %d %s\n"),
                  fti, rv, bcm_errmsg(rv)));
        return rv;
    }
    *ohi = fte.oi;

    return rv;
}
#undef _VLAN_FUNC_NAME

#define _VLAN_FUNC_NAME "bcm_fe2k_vlan_vgp_frame_max_access"
int
bcm_fe2k_vlan_vgp_frame_max_access(int unit, bcm_gport_t gport, 
                                   int *size, int set)
{
    int                     rv = BCM_E_NONE;
    soc_sbx_g2p3_ft_t       ft;
    soc_sbx_g2p3_oi2e_t     oh;
    soc_sbx_g2p3_eteencap_t eteencap;
    soc_sbx_g2p3_etel2_t    etel2;
    uint32                  fti, etei;

    VLAN_VERB((VLAN_FUNCMSG1("(0x%08x, 0x%08x, %d)\n"), gport, *size, set));
    
    if (VLAN_GPORT_VALID(unit, gport) == FALSE) {
        return BCM_E_PARAM;
    }

    fti = BCM_GPORT_VLAN_PORT_ID_GET(gport);
    fti = VLAN_VGPORT_ID_TO_FT_INDEX(unit, fti);

    VLAN_LOCK_TAKE;

    rv = bcm_fe2k_egr_path_get(unit, fti, &ft, &oh, &eteencap, &etel2, &etei);

    if (BCM_FAILURE(rv)) {
        VLAN_LOCK_RELEASE;
        return rv;
    }

    if (set) {
        etel2.mtu = *size - 4;
        rv = soc_sbx_g2p3_etel2_set(unit, etei, &etel2);
    } else {
        *size = etel2.mtu + 4;
    }

    VLAN_LOCK_RELEASE;
    return rv;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *     bcm_fe2000_vgp_policer_set
 *   Purpose
 *      Set a policer on a vlan gport
 *   Parameters
 *      (in)  unit        = BCM device number
 *      (in)  gport       = vlan gport to update
 *      (in)  pol_id      = policer id to assign
 *   Returns
 *       BCM_E_* 
 *
 *  Notes:
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vgp_policer_set"
int
bcm_fe2000_vgp_policer_set(int unit, bcm_gport_t gport, bcm_policer_t pol_id)
{
    int                      rv;
    uint32                   lpi;
    soc_sbx_g2p3_lp_t        lp;

    lpi = ~0;

    VLAN_VERB((VLAN_FUNCMSG1("(0x%08x, 0x%08x)\n"), gport, pol_id));

    VLAN_LOCK_TAKE;
    rv = _bcm_fe2000_vgp_resolve_internal(unit, gport, &lpi, NULL);
    if (BCM_FAILURE(rv)) {
        VLAN_LOCK_RELEASE;
        return rv;
    }

    if (BCM_SUCCESS(rv)) {
        rv = soc_sbx_g2p3_lp_get(unit, lpi, &lp);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("Failed to read LP[0x%x]: %s\n"),
                      lpi, bcm_errmsg(rv)));
        }
    }

    if (BCM_SUCCESS(rv)) {
        if (pol_id && lp.policer) {
            VLAN_ERR((VLAN_FUNCMSG1("LP[0x%x] already has a policer 0x%04x\n"),
                      lpi, lp.policer));
            rv = BCM_E_PARAM;
        } else if (pol_id==0 && lp.policer==0){
	    VLAN_ERR((VLAN_FUNCMSG1("LP[0x%x] has no policer to remove\n"),
		     lpi));
	    rv = BCM_E_PARAM;
	}
    }

    if (BCM_SUCCESS(rv)) {
        rv = _bcm_fe2000_g2p3_policer_lp_program(unit, pol_id, &lp);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("Failed to confiure LP[0x%x]: %s\n"),
                      lpi, bcm_errmsg(rv)));
        }
    }

    if (BCM_SUCCESS(rv)) {
        rv = soc_sbx_g2p3_lp_set(unit, lpi, &lp);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("Failed to write LP[0x%x]: %s\n"),
                      lpi, bcm_errmsg(rv)));
        }
    }

    VLAN_LOCK_RELEASE;
    return rv;
}
#undef _VLAN_FUNC_NAME


/*
 *   Function
 *     bcm_fe2000_vgp_stat_enable_get
 *   Purpose
 *      Get the state of statistics on a vlan gport
 *   Parameters
 *      (in)  unit        = BCM device number
 *      (in)  gport       = vlan gport to query
 *      (out) ena         = storage location for stats state
 *   Returns
 *       BCM_E_* 
 *
 *  Notes:
 */
#define _VLAN_FUNC_NAME  "bcm_fe2000_vgp_stat_enable_get"
int
bcm_fe2000_vgp_stat_enable_get(int unit, bcm_gport_t gport, int *ena)
{
    int    rv;
    uint32 lpi, ohi;

    VLAN_LOCK_TAKE;
    rv = _bcm_fe2000_vgp_get_lp_encap(unit, gport, &lpi, &ohi);

    if (BCM_SUCCESS(rv)) {
        rv = _bcm_fe2000_g2p3_logical_interface_ingress_stats_enable_update(unit, lpi, TRUE, ena);
    }
                                                                            
    VLAN_LOCK_RELEASE;

    return rv;
}
#undef _VLAN_FUNC_NAME


/*
 *   Function
 *     bcm_fe2000_vgp_stat_enable_set
 *   Purpose
 *      Set the state of statistics on a vlan gport
 *   Parameters
 *      (in)  unit        = BCM device number
 *      (in)  gport       = vlan gport to query
 *      (in)  ena         = stats state to set
 *   Returns
 *       BCM_E_* 
 *
 *  Notes:
 */
#define _VLAN_FUNC_NAME  "bcm_fe2000_vgp_stat_enable_set"
int
bcm_fe2000_vgp_stat_enable_set(int unit, bcm_gport_t gport, int ena)
{
    int    rv;
    int    enable;
    uint32 lpi, ohi;

    VLAN_LOCK_TAKE;
    rv = _bcm_fe2000_vgp_get_lp_encap(unit, gport, &lpi, &ohi);

    if (BCM_SUCCESS(rv)) {
        enable = ena;
        rv = _bcm_fe2000_g2p3_logical_interface_ingress_stats_enable_update(unit, lpi, FALSE, &enable);
    }

    if (BCM_SUCCESS(rv)) {
        rv = _bcm_fe2000_g2p3_logical_interface_egress_stats_enable_update(unit, ohi, FALSE, &enable);

    }
    
    VLAN_LOCK_RELEASE;
    return rv;
}
#undef _VLAN_FUNC_NAME


/*
 *   Function
 *     _bcm_fe2000_vlan_pv2e_set
 *   Purpose
 *      Set a <port,vid> entry, for any ucode version
 *   Parameters
 *      (in)  unit        = BCM device number
 *      (in)  vsi         = vsi to set
 *      (in) logicalPort  = logicalPort to set
 *      (in) vlanPortData = additional data for the vlan port
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes lock is already taken
 */

#define _VLAN_FUNC_NAME  "_bcm_fe2000_vlan_pv2e_set"
static int
_bcm_fe2000_vlan_pv2e_set(int unit,
                          bcm_vlan_t vsi,
                          uint32_t logicalPort,
                          _bcm_fe2k_vlan_gport_t *vlanPortData,
                          bcm_port_t tgtPort)
{
    int   rv;

#ifdef BCM_FE2000_P3_SUPPORT
    if (SOC_IS_SBX_G2P3(unit)) {
        soc_sbx_g2p3_pv2e_t  sbxPortVid;            /* G2P3 pVid2Etc entry */

        /* update pVid2Etc entry */
        soc_sbx_g2p3_pv2e_t_init(&sbxPortVid);
        sbxPortVid.lpi  = logicalPort;
        sbxPortVid.vlan = vsi;
        rv = _i_soc_sbx_g2p3_pv2e_set(unit,
                                      tgtPort,
                                      vlanPortData->p.match_vlan,
                                      &sbxPortVid);
        if (BCM_E_NONE != rv) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:pv2e[%d,%04X]:"
                                    " %d (%s)\n"),
                      unit,
                      tgtPort,
                      vlanPortData->p.match_vlan,
                      rv,
                      _SHR_ERRMSG(rv)));
        }
    } else
#endif /* def BCM_FE2000_P3_SUPPORT */
    {
        /* if we got to here, current unit is not supported */
        rv = BCM_E_UNAVAIL;
        VLAN_ERR((VLAN_FUNCMSG1("unit %d is not supported type\n"), unit));
    }

    return rv;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *     _bcm_fe2000_vlan_pvv2e_set
 *   Purpose
 *      Set a <port,vid,vid> entry, for any ucode version
 *   Parameters
 *      (in)  unit        = BCM device number
 *      (in)  vsi         = vsi to set
 *      (in) logicalPort  = logicalPort to set
 *      (in) vlanPortData = additional data for the vlan port
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
#define _VLAN_FUNC_NAME  "_bcm_fe2000_vlan_pvv2e_set"
static int
_bcm_fe2000_vlan_pvv2e_set(int  unit,
                           bcm_vlan_t vsi,
                           uint32_t logicalPort,
                           _bcm_fe2k_vlan_gport_t *vlanPortData,
                           bcm_port_t tgtPort)
{
    int   rv;

#ifdef BCM_FE2000_P3_SUPPORT
    if (SOC_IS_SBX_G2P3(unit)) {
        soc_sbx_g2p3_pvv2e_t sbxPortVidVid;
        soc_sbx_g2p3_pvv2e_t_init(&sbxPortVidVid);
        sbxPortVidVid.lpi  = logicalPort;
        sbxPortVidVid.vlan = vsi;
        sbxPortVidVid.keeporstrip = !(vlanPortData->p.flags &
                                      BCM_VLAN_PORT_INNER_VLAN_PRESERVE);

        if (0 == vsi) {
            rv = soc_sbx_g2p3_pvv2e_delete(unit,
                                           vlanPortData->p.match_inner_vlan,
                                           vlanPortData->p.match_vlan,
                                           tgtPort);
            /* The user may have cleared the match vlan already by clearing all
             * port/vids in the vlan vector which will also remove the pvv2e
             * entry for the match vlan, causing an entry not found error
             */
            if (rv == BCM_E_NOT_FOUND) {
                rv = BCM_E_NONE;
            }
        } else {
            rv = soc_sbx_g2p3_pvv2e_set(unit,
                                        vlanPortData->p.match_inner_vlan,
                                        vlanPortData->p.match_vlan,
                                        tgtPort,
                                        &sbxPortVidVid);
        }
    } else
#endif /* BCM_FE2000_P3_SUPPORT */
    {
        /* if we got to here, unit isn't supported */
        rv = BCM_E_UNAVAIL;
        VLAN_ERR((VLAN_FUNCMSG1("unit %d is not supported type\n"), unit));
    }

    return rv;
}
#undef _VLAN_FUNC_NAME


/*
 *   Function
 *      _bcm_fe2000_vp_match_configure
 *   Purpose
 *      Configure and commit to hardware pv2e's for a vlan gport based
 *      on the match criteria
 */
#define _VLAN_FUNC_NAME  "_bcm_fe2000_vp_match_configure"
static int
_bcm_fe2000_vp_match_configure(int unit, _bcm_fe2k_vlan_gport_t *vlanPortData,
                               bcm_port_t tgtPort, bcm_vlan_t vsi, uint32 lpi)
{
    int                   rv;
    soc_sbx_g2p3_p2e_t    p2e;
    int                   isProviderPort = FALSE;
    int                   setFlags, vid, innerPreserve;
    soc_sbx_g2p3_pvv2e_t  pvv2e;
    soc_sbx_g2p3_pv2e_t   pv2e;

    VLAN_VERB((VLAN_FUNCMSG("(%d,*,%d,%04x,%08X) enter\n"),
               unit, tgtPort, vsi, lpi));

    if (SOC_IS_SBX_G2P3(unit)) {
        rv = soc_sbx_g2p3_p2e_get(unit, tgtPort, &p2e);

        if (BCM_E_NONE != rv) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to get port data %d:p2e[%d]:"
                                    " %d (%s)\n"),
                      unit, tgtPort, rv, _SHR_ERRMSG(rv)));
            return rv;
        }
        isProviderPort = p2e.provider;
    }

    /* configure match parameters */
    switch (vlanPortData->p.criteria) {
            
    case BCM_VLAN_PORT_MATCH_PORT:

        innerPreserve = 
            !!(vlanPortData->p.flags & BCM_VLAN_PORT_INNER_VLAN_PRESERVE);

        soc_sbx_g2p3_pv2e_t_init(&pv2e);
        pv2e.lpi  = lpi;
        pv2e.vlan = vsi ? vsi : _vlan_state[unit]->nvid[tgtPort];

        soc_sbx_g2p3_pvv2e_t_init(&pvv2e);
        pvv2e.lpi            = lpi;
        pvv2e.vlan           = pv2e.vlan;
        pvv2e.keeporstrip    = 1;
        pvv2e.untagged_strip = 1;

        for (vid=0; vid < BCM_VLAN_MAX; vid++) {

            rv = SOC_SBX_G2P3_PV2E_SET(unit, tgtPort, vid, &pv2e);

            /* vsi == 0 means the gport is being removed,
             * set the pv2e's back to TB mode.
             * Setup the VSI for the NEXT pv2e, vid=0 is already
             * setup to be the default/ native vid above.
             */
            if (vsi == 0) {                    
                pv2e.vlan = vid + 1;
            }

            if (BCM_FAILURE(rv)) {
                VLAN_ERR((VLAN_FUNCMSG1("failed to write %d:pv2e[%d,0x%x]:"
                                        " %d (%s)\n"),
                          unit, tgtPort, vid, rv, _SHR_ERRMSG(rv)));
                break;
            }
                
            /* add a pvv2e if the ctag is to be removed */
            if (vsi == 0 || vid == 0) {
                rv = soc_sbx_g2p3_pvv2e_remove(unit, 0xfff, vid,
                                               tgtPort);
                if (rv == BCM_E_NOT_FOUND) {
                    rv = BCM_E_NONE;
                }
            } else if (innerPreserve == 0) {
                rv = soc_sbx_g2p3_pvv2e_add(unit, 0xFFF, vid,
                                            tgtPort, &pvv2e);
                if (rv == BCM_E_EXISTS) {
                    rv = soc_sbx_g2p3_pvv2e_renew(unit, 0xFFF, vid,
                                                  tgtPort, &pvv2e);
                }
            }

            if (BCM_FAILURE(rv)) {
                VLAN_ERR((VLAN_FUNCMSG1("failed to %s %d:"
                                        "pvv2e[%d,0x%x,0xFFF]: %d (%s)\n"),
                          ((vid==0 || vsi==0) ? "remove" : "add"), unit,
                          tgtPort, vid, rv, _SHR_ERRMSG(rv)));
                break;
            }
        }

        if (BCM_SUCCESS(rv)) {

            rv = soc_sbx_g2p3_pvv2e_commit(unit, 0xFFFFFFFF);

            if (BCM_FAILURE(rv)) {
                VLAN_ERR((VLAN_FUNCMSG1("pvv2e commit failed:  %d (%s)\n"),
                          rv, _SHR_ERRMSG(rv)));
            }
        }

        break;

    case BCM_VLAN_PORT_MATCH_PORT_VLAN:

        if (BCM_PBMP_MEMBER(_vlan_state[unit]->tag_drop, tgtPort) &&
            (vlanPortData->p.match_vlan == _vlan_state[unit]->nvid[tgtPort]))
        {

            /* when in tag_drop mode, the match_vlan should/must be
             * the same as the current native vid.  In this case
             * the pvv2e flags must be set to override the regular
             * vlan port's version to set the proper vsi, and optionally
             * set the untagged_strip vbit
             */
            _vlan_state[unit]->nvflag[tgtPort] =
                (BCM_FE2K_NVID_OVERRIDE_PVV2E |
                 BCM_FE2K_NVID_USE_PVV2E );

            if (!(vlanPortData->p.flags & BCM_VLAN_PORT_INNER_VLAN_PRESERVE)) {
                _vlan_state[unit]->nvflag[tgtPort] |=
                    BCM_FE2K_NVID_SET_UNTAGGEDSTRIP;
            } else {
                _vlan_state[unit]->nvflag[tgtPort] |=
                    BCM_FE2K_NVID_SET_REPLACE;
            }

            setFlags = 1;
        } else {
            setFlags = 0;
        }

        /* Mod/port/trunk + outer VLAN. */
        rv = _bcm_fe2000_vlan_pv2e_set(unit, vsi, lpi, vlanPortData, tgtPort);
        if (!setFlags &&
            (isProviderPort ||
             !(vlanPortData->p.flags & BCM_VLAN_PORT_INNER_VLAN_PRESERVE))
            && rv == BCM_E_NONE)
        {
            /*
             * If adding a port/vid match on a provider port, must
             * also add a port/vid/vid lookup with inner_vid=0xfff for
             * STAGed only packets to get correct behavior from ucode
             */
            vlanPortData->p.match_inner_vlan = 0xFFF;
            rv = _bcm_fe2000_vlan_pvv2e_set(unit, vsi, lpi, vlanPortData,
                                            tgtPort);
        }

        break;
    case BCM_VLAN_PORT_MATCH_PORT_VLAN_STACKED:
        /* Mod/port/trunk + outer/inner VLAN. */
        rv = _bcm_fe2000_vlan_pvv2e_set(unit, vsi, lpi, vlanPortData,
                                        tgtPort);
        break;
/*        case BCM_VLAN_PORT_MATCH_NONE: */
        /* No source match criteria. */
        /* fallthrough intentional */
/*        case BCM_VLAN_PORT_MATCH_INVALID: */
        /* curious to have an explicitly invalid type...? */
        /* fallthrough intentional */
    default:
        rv = BCM_E_PARAM;
        VLAN_ERR((VLAN_FUNCMSG1("unsupported match criteria %d for"
                                " gport 0x%08X on unit %d\n"),
                  vlanPortData->p.criteria, vlanPortData->p.vlan_port_id, 
                  unit));
        break;
    }
    if (BCM_E_NONE != rv) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to set match criteria %d for"
                                " gport 0x%08X on unit %d: %d (%s)\n"),
                  vlanPortData->p.criteria, vlanPortData->p.vlan_port_id,
                  unit, rv, _SHR_ERRMSG(rv)));
    }


    return rv;
}

#undef _VLAN_FUNC_NAME


/*
 *  Function
 *    _bcm_fe2000_vlan_vsi_port_program
 *  Purpose
 *    Populate tables based upon vlan_port match criteria
 *  Parameters
 *    (in) unit    - BCM device number
 *    (in) vsi     - vsi port is to be added
 *    (in) gport   - gport to add
 *  Returns
 *    BCM_E_NONE if successfull
 *    BCM_E_* on error
 *  Notes
 *    Assumes all params are valid and locks have been taken
 */
#define _VLAN_FUNC_NAME  "_bcm_fe2000_vlan_vsi_port_program"
static int
_bcm_fe2000_vlan_vsi_port_program(int unit,
                                  bcm_vlan_t vsi,
                                  uint32 logicalPort,
                                  bcm_gport_t gport)
{
    int                       rv = BCM_E_NONE;       /* local result code */
    uint32                    gportId;               /* internal GPORT ID */
    uint32_t                  lp = 0;                /* logical port ID */
    _bcm_fe2k_vlan_gport_t    *vlanPortData = NULL;  /* ptr to descriptor */
    bcm_port_t                tgtPort = ~0;          /* target port */
    bcm_module_t              tgtModule = -1;        /* target module */
    bcm_module_t              myModule = ~0;         /* local module */
    bcm_trunk_add_info_t      trunkInfo;
    bcm_trunk_t               trunkId;
    int                       numPorts, portIdx;
    uint32_t                  ohIndex = 0;
    soc_sbx_g2p3_oi2e_t       oi2e;
    soc_sbx_g2p3_eteencap_t   eteencap;

    VLAN_VERB((VLAN_FUNCMSG("(%d,0x%04X,0x%08X,0x%08X) enter\n"),
               unit,
               vsi,
               logicalPort,
               gport));

    gportId = BCM_GPORT_VLAN_PORT_ID_GET(gport);
    lp = _vlan_state[unit]->gportInfo.lpid[gportId];
    vlanPortData = (_bcm_fe2k_vlan_gport_t*)(SBX_LPORT_DATAPTR(unit, lp));


    bcm_trunk_add_info_t_init(&trunkInfo);

    /* parse target port information */
    if (BCM_GPORT_IS_TRUNK(vlanPortData->p.port)) {

        rv = bcm_stk_my_modid_get(unit, &myModule);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("Failed to get local modid: %s\n"),
                      bcm_errmsg(rv)));
            return rv;
        }

        trunkId = BCM_GPORT_TRUNK_GET(vlanPortData->p.port);
        rv = bcm_trunk_get(unit, trunkId, &trunkInfo);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("Failed to get trunk %d info: %s\n"),
                      trunkId, bcm_errmsg(rv)));
            return rv;
        }

        numPorts = trunkInfo.num_ports;

    } else {

        rv = _bcm_fe2000_map_vlan_gport_targets(unit, vlanPortData->p.port,
                                                &myModule, &tgtModule,
                                                &tgtPort, NULL, NULL);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to map gport 0x%08X targets: "
                                    "%d (%s)\n"),
                      vlanPortData->p.port, rv, bcm_errmsg(rv)));
            return rv;
        }

        numPorts = 1;
        trunkInfo.tp[0] = tgtPort;
        trunkInfo.tm[0] = tgtModule;
    }

    /* update all ports in trunk, or the single port for non-trunked ports */
    for (portIdx=0; portIdx < numPorts; portIdx++) {
        tgtPort   = trunkInfo.tp[portIdx];
        tgtModule = trunkInfo.tm[portIdx];

        if (tgtModule == myModule) {
            
            /* For vlan_gports, we always keep the tag on the port.  
             * To remove it, pvv2e entries are added for each vid in the vector
             * and the keeporstrip bit is set to remove it.  This allows gport
             * with _INNER_VLAN_PRESEVE=1 and =0 to coexist on the same physical
             * port.
             */
            rv = _bcm_fe2000_port_strip_tag(unit, tgtPort, 0);
            
            if (BCM_FAILURE(rv)) {
                VLAN_ERR((VLAN_FUNCMSG1("Failed to update port strip "
                                        "on target port\n")));
                break;
            }
            
            rv = _bcm_fe2000_vp_match_configure(unit, vlanPortData, tgtPort,
                                                vsi, logicalPort);
            if (BCM_FAILURE(rv)) {
                break;
            }
            
            rv = _bcm_fe2000_g2p3_vlan_port_vlan_vector_update(unit, 
                                                               &vlanPortData->p,
                                                               tgtPort, TRUE);
            if (BCM_FAILURE(rv)) {
                VLAN_ERR((VLAN_FUNCMSG1("vlan vector update failed %d:"
                                        " %d (%s)\n"),
                          unit, rv, bcm_errmsg(rv)));
                break;
            }
        }
    }

    /* add VSI information to egress path */
    if (SOC_SBX_IS_VALID_L2_ENCAP_ID(vlanPortData->p.encap_id)) {
        ohIndex = SOC_SBX_OHI_FROM_L2_ENCAP_ID(vlanPortData->p.encap_id);
    } else if (SOC_SBX_IS_VALID_ENCAP_ID(vlanPortData->p.encap_id)) {
        ohIndex = SOC_SBX_OHI_FROM_ENCAP_ID(vlanPortData->p.encap_id);
    } else {
        VLAN_ERR((VLAN_FUNCMSG1("unexpected encap_id 0x%08X\n"),
                  vlanPortData->p.encap_id));
        rv = BCM_E_INTERNAL;
    }
    
    if (BCM_SUCCESS(rv)) {
        ohIndex -= SBX_RAW_OHI_BASE;
        rv = soc_sbx_g2p3_oi2e_get(unit, ohIndex, &oi2e);
        if (BCM_E_NONE != rv) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to get data %d:oi2e[0x%x]:"
                                    " %d (%s)\n"),
                      unit, vlanPortData->p.encap_id, rv, 
                      _SHR_ERRMSG(rv)));
        }

        rv = soc_sbx_g2p3_eteencap_get(unit, oi2e.eteptr, &eteencap);
        if (BCM_E_NONE != rv) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to get data %d:eteencap[0x%x]:"
                                    " %d (%s)\n"),
                      unit, oi2e.eteptr, rv, _SHR_ERRMSG(rv)));
        }

        eteencap.vlan = vsi;
        rv = soc_sbx_g2p3_eteencap_set(unit, oi2e.eteptr, &eteencap);
        if (BCM_E_NONE != rv) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to set data %d:eteencap[0x%x]:"
                                    " %d (%s)\n"),
                      unit, oi2e.eteptr, rv, _SHR_ERRMSG(rv)));
        }
    }

    VLAN_VERB((VLAN_FUNCMSG("(%d,0x%04X,0x%08X,0x%08X) return %d (%s)\n"),
               unit, vsi, logicalPort, gport,
               rv, _SHR_ERRMSG(rv)));
    return rv;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_port_create
 *   Purpose
 *      Create a logical port and commit to hardware
 *   Parameters
 *      (in)  unit         = BCM device number
 *      (in/out) vlan_port = description of port to be created
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_port_create"
int
bcm_fe2000_vlan_port_create(int unit,
                            bcm_vlan_port_t *vlan_port)
{
    int     result;                             /* local result code */
    uint32  logicalPort = SBX_LPORT_END + 1;    /* logical port # */
    uint32  foverLogicalPort = SBX_LPORT_END + 1;/* failover logical port # */
    _bcm_fe2k_vlan_gport_t *gpPtr = NULL;       /* local GPORT data ptr */
    _bcm_fe2k_vlan_gport_t gpData;              /* local GPORT working data */
    uint32  gportId;                            /* PORT ID */
    uint32  foverGportId;                       /* failover GPORT ID */
    uint32  fti = ~0;                           /* ft index */
    uint32  gu2_res, gu2_flags;                 /* convenience vars */
    int     allocFte= FALSE;                    /* allocated FTE */
    int     allocLp = FALSE;                    /* allocated LPort */
    int     updated = FALSE;                    /* indicates update started */
    int     pulled  = FALSE;                    /* original entry pulled */
    bcm_vlan_t oldVsi = 0;                      /* original VSI for REPLACE */
    int     tid = SBX_MAX_TRUNKS;
    bcm_trunk_add_info_t trunk_info;

    gportId = VLAN_MAX_VLAN_GPORTS;

    VLAN_VERB((VLAN_FUNCMSG("(%d,*) enter\n"), unit));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* this argument must not be NULL */
    if (!vlan_port) {
        VLAN_ERR((VLAN_FUNCMSG1("NULL pointer for vlan_port argument\n")));
        return BCM_E_PARAM;
    }
    /* make some other sanity checks */
    if (vlan_port->flags & ~VLAN_SUPPORTED_VLAN_PORT_FLAGS) {
        VLAN_ERR((VLAN_FUNCMSG1("unsupported flags are set: %08X(%08X)\n"),
                  vlan_port->flags & (~VLAN_SUPPORTED_VLAN_PORT_FLAGS),
                  VLAN_SUPPORTED_VLAN_PORT_FLAGS));
        return BCM_E_PARAM;
    }

    /* get the vlan lock */
    VLAN_LOCK_TAKE;

    /* figure out the GPORT ID */
    if (vlan_port->flags & BCM_VLAN_PORT_WITH_ID) {
        /* caller provided an ID */
        if (BCM_GPORT_IS_VLAN_PORT(vlan_port->vlan_port_id)) {
            /* caller provided a VLAN GPORT ID */
            gportId = BCM_GPORT_VLAN_PORT_ID_GET(vlan_port->vlan_port_id);
            if (vlan_port->flags & BCM_VLAN_PORT_REPLACE) {

                /* caller wants to replace an 'existing' one */
                fti = VLAN_VGPORT_ID_TO_FT_INDEX(unit, gportId);
                gu2_res = SBX_GU2K_USR_RES_FTE_LOCAL_GPORT;
                result = _sbx_gu2_resource_test(unit, gu2_res, fti);
            
                if (BCM_E_EXISTS == result) {
                    /* 'existing' one really exists */
                    logicalPort = _vlan_state[unit]->gportInfo.lpid[gportId];
                    if (BCM_GPORT_VLAN_PORT == SBX_LPORT_TYPE(unit,
                                                              logicalPort)) {
                        /* existing one is correct type */
                        gpPtr = (_bcm_fe2k_vlan_gport_t*)
                                (SBX_LPORT_DATAPTR(unit, logicalPort));
                        if (gpPtr) {
                            /* existing port is valid */
                            oldVsi = gpPtr->p.vsi;
                            if (oldVsi &&
                                ((gpPtr->p.port != vlan_port->port) ||
                                 (gpPtr->p.match_vlan != vlan_port->match_vlan) ||
                                 (gpPtr->p.match_inner_vlan !=
                                  vlan_port->match_inner_vlan) ||
                                 ((gpPtr->p.flags &
                                   VLAN_ING_SIGNIFICANT_VLAN_PORT_FLAGS) !=
                                  (vlan_port->flags &
                                   VLAN_ING_SIGNIFICANT_VLAN_PORT_FLAGS)))) {
                                /*
                                 *  The gport has differences in the ingress
                                 *  side and will need to be pulled so it can
                                 *  be reprogrammed properly.
                                 */
                                pulled = TRUE;
                                VLAN_VVERB((VLAN_FUNCMSG1("replacing VSI"
                                                          " member; remove"
                                                          " ingress data\n")));
                                result = _bcm_fe2000_vlan_vsi_port_program(unit,
                                                                           0,
                                                                           0,
                                                                           vlan_port->vlan_port_id);
                                if (BCM_E_NONE != result) {
                                    VLAN_ERR((VLAN_FUNCMSG1("unable to remove"
                                                            " ingress data"
                                                            " replacing unit %d"
                                                            " VLAN GPORT %08X:"
                                                            " %d (%s)\n"),
                                              unit,
                                              vlan_port->vlan_port_id,
                                              result,
                                              _SHR_ERRMSG(result)));
                                }
                            } else {
                                if (oldVsi) {
                                    VLAN_VVERB((VLAN_FUNCMSG1("replacing VSI"
                                                              " member, but no"
                                                              " ingress"
                                                              " changes\n")));
                                }
                                result = BCM_E_NONE;
                            }
                        } else { /* if (LPORT data pointer is valid) */
                            /* existing port is not valid */
                            result = BCM_E_INTERNAL;
                            VLAN_ERR((VLAN_FUNCMSG1("expected logical port %08X"
                                                    " for unit %d GPORT %08X is"
                                                    " missing GPORT info\n"),
                                      logicalPort,
                                      unit,
                                      vlan_port->vlan_port_id));
                        } /* if (LPORT data pointer is valid) */
                    } else { /* if (LPORT for GPORT is proper type) */
                        /* existing one is not correct type */
                        result = BCM_E_INTERNAL;
                        VLAN_ERR((VLAN_FUNCMSG1("expected logical port %08X for"
                                                " unit %d GPORT %08X is wrong"
                                                " type (%d, should be %d)\n"),
                                  logicalPort,
                                  unit,
                                  vlan_port->vlan_port_id,
                                  SBX_LPORT_TYPE(unit, logicalPort),
                                  BCM_GPORT_VLAN_PORT));
                    } /* if (LPORT for GPORT is proper type) */
                } else { /* if (BCM_E_EXISTS == result) */
                    /* error accessing 'existing' VLAN GPORT */
                    VLAN_ERR((VLAN_FUNCMSG1("unable to access unit %d GPORT"
                                            " %08X: %d (%s)\n"),
                              unit,
                              vlan_port->vlan_port_id,
                              result,
                              _SHR_ERRMSG(result)));
                } /* if (BCM_E_EXISTS == result) */

            } else { /* if(vlan_port->flags & BCM_VLAN_PORT_REPLACE) */

                /* caller wants to create a new one */
                fti = VLAN_VGPORT_ID_TO_FT_INDEX(unit, gportId);

                gu2_res = SBX_GU2K_USR_RES_FTE_LOCAL_GPORT;
                gu2_flags = _SBX_GU2_RES_FLAGS_RESERVE;
                result = _sbx_gu2_resource_alloc(unit, gu2_res, 1, 
                                                 &fti, gu2_flags);

                /* Convert a busy error to an exists error, they're the same
                 * thing when reserving an entry
                 */
                if (result == BCM_E_BUSY) {
                    result = BCM_E_EXISTS;
                }

                /* if this GPORT is in the reserved range, convert the error 
                 * code to NONE 
                 */
                if (result == BCM_E_RESOURCE) {
                    VLAN_VERB((VLAN_FUNCMSG1("unit %d gport 0x%08x found to be"
                                             " in the reserved range, OK.\n"), 
                               vlan_port->vlan_port_id));
                    result = BCM_E_NONE;
                }

                if (BCM_FAILURE(result)) {
                    /* looks like it already exists */
                    VLAN_ERR((VLAN_FUNCMSG1("unit %d GPORT %08X already exists: %s\n"),
                              unit, vlan_port->vlan_port_id, bcm_errmsg(result)));
                } else {
                    VLAN_ERR((VLAN_FUNCMSG1("unable to reserve unit %d GPORT"
                                            " %08X: %d (%s)\n"),
                              unit,
                              vlan_port->vlan_port_id,
                              result,
                              _SHR_ERRMSG(result)));
                }
            } /* else - if(vlan_port->flags & BCM_VLAN_PORT_REPLACE) */

        } else { /* if (BCM_GPORT_IS_VLAN_PORT(vlan_port->vlan_port_id)) */
            /* caller provided invalid ID */
            result = BCM_E_PARAM;
            VLAN_ERR((VLAN_FUNCMSG1("invalid GPORT ID %08X\n"),
                      vlan_port->vlan_port_id));
        } /* if (BCM_GPORT_IS_VLAN_PORT(vlan_port->vlan_port_id)) */
    } else { /* if (vlan_port->flags & BCM_VLAN_PORT_WITH_ID) */
        if (vlan_port->flags & BCM_VLAN_PORT_REPLACE) {
            /* caller wants to replace one at random; bogus */
            result = BCM_E_PARAM;
            VLAN_ERR((VLAN_FUNCMSG1("must specify WITH_ID when specifying"
                                    " REPLACE\n")));
        } else { /* if (vlan_port->flags & BCM_VLAN_PORT_REPLACE) */
            gu2_res   = SBX_GU2K_USR_RES_FTE_LOCAL_GPORT;
            gu2_flags = 0;
            result = _sbx_gu2_resource_alloc(unit, gu2_res, 1, 
                                             &fti, gu2_flags);

            VLAN_VERB((VLAN_FUNCMSG1("Alloc fte 0x%05x, gportId 0x%04x: %s\n"),
                       fti, VLAN_FT_INDEX_TO_VGPORT_ID(unit, fti), 
                       bcm_errmsg(result)));

            if (BCM_SUCCESS(result)) {
                gportId = VLAN_FT_INDEX_TO_VGPORT_ID(unit, fti);
                allocFte = TRUE;
            } else {
                VLAN_ERR((VLAN_FUNCMSG1("unable to allocate FT entry on"
                                        " unit %d: %d (%s)\n"),
                          unit,
                          result,
                          _SHR_ERRMSG(result)));
            }


        } /* if (vlan_port->flags & BCM_VLAN_PORT_REPLACE) */
    } /* if (vlan_port->flags & BCM_VLAN_PORT_WITH_ID) */


    /* From this point on gportId is a 0-based id that is mapped 1:1 to a
     * GPORT FTE
     */
    if (BCM_SUCCESS(result) && gportId >= VLAN_MAX_VLAN_GPORTS) {
        VLAN_ERR((VLAN_FUNCMSG1("Invalid internal gport id found: 0x%x\n"), 
                  gportId));
        result = BCM_E_INTERNAL;
    }

    /* if not REPLACE, need to set up logical port and descriptor */
    /* we already looked up logicalPort if in REPLACE mode */
    if (!(vlan_port->flags & BCM_VLAN_PORT_REPLACE)) {
        /* allocate a logical port */
        if (BCM_E_NONE == result) {
            gu2_res   = SBX_GU2K_USR_RES_LPORT;
            gu2_flags = 0;
            result = _sbx_gu2_resource_alloc(unit, gu2_res, 1, &logicalPort,
                                             gu2_flags);

            if (BCM_E_NONE == result) {
                /* associate logical port with this VLAN GPORT */
                _vlan_state[unit]->gportInfo.lpid[gportId] = logicalPort;
                allocLp = TRUE;
            } else {
                /* unable to allocate a logical port */
                VLAN_ERR((VLAN_FUNCMSG1("unable to allocate logicalPort on"
                                        " unit %d: %d (%s)\n"),
                          unit,
                          result,
                          _SHR_ERRMSG(result)));
            }
        }

        /* allocate and initialise the port descriptor */
        if (BCM_E_NONE == result) {
            gpPtr = sal_alloc(sizeof(_bcm_fe2k_vlan_gport_t), "vlan_port_data");
            if (gpPtr) {
                sal_memset(gpPtr, 0, sizeof(*gpPtr));
            } else {
                result = BCM_E_MEMORY;
                VLAN_ERR((VLAN_FUNCMSG1("unable to allocate %d bytes for unit"
                                        " %d vlan_port information\n"),
                          sizeof(*vlan_port),
                          unit));
            }
        } /* if (BCM_E_NONE == result) */
    } /* if (!(vlan_port->flags & BCM_VLAN_PORT_REPLACE)) */

    /* Check failover gport */
    if (BCM_E_NONE == result && vlan_port->failover_id) {
        foverGportId = BCM_GPORT_VLAN_PORT_ID_GET(vlan_port->failover_port_id);
        fti = VLAN_VGPORT_ID_TO_FT_INDEX(unit, foverGportId);

        gu2_res = SBX_GU2K_USR_RES_FTE_LOCAL_GPORT;
        result = _sbx_gu2_resource_test(unit, gu2_res, fti);

        if (result == BCM_E_EXISTS) {
            result = BCM_E_NONE;
            foverLogicalPort = _vlan_state[unit]->gportInfo.lpid[foverGportId];
            if (BCM_GPORT_VLAN_PORT
                != SBX_LPORT_TYPE(unit, foverLogicalPort)) {
                /* existing one is wrong type */
                result = BCM_E_PARAM;
                VLAN_ERR((VLAN_FUNCMSG1("expected logical port %08X for"
                                        " unit %d GPORT %08X is wrong"
                                        " type (%d, should be %d)\n"),
                          foverLogicalPort,
                          unit,
                          vlan_port->failover_port_id,
                          SBX_LPORT_TYPE(unit, foverLogicalPort),
                          BCM_GPORT_VLAN_PORT));
            }
        } else { /* if failover gport exists */
            result = BCM_E_PARAM;
            VLAN_ERR((VLAN_FUNCMSG1("failover port %08X"
                                    " for unit %d bcm_vlan_port_t"
                                    " doesn't exist\n"),
                      foverGportId, unit));
        }
    }

    if (BCM_E_NONE == result) {
        /* now we have port descriptor; copy to working copy */
        sal_memcpy(&gpData, gpPtr, sizeof(gpData));
    }

    /* set up the port descriptor */
    if (BCM_E_NONE == result) {
        if ((vlan_port->flags & BCM_VLAN_PORT_REPLACE)) {
            /* replace; only replace some private data */
            gpData.p.criteria = vlan_port->criteria;
            gpData.p.egress_inner_vlan = vlan_port->egress_inner_vlan;
            gpData.p.egress_vlan = vlan_port->egress_vlan;
            gpData.p.flags = vlan_port->flags;
            gpData.p.match_inner_vlan = vlan_port->match_inner_vlan;
            gpData.p.match_vlan = vlan_port->match_vlan;
            gpData.p.port = vlan_port->port;
            gpData.p.policer_id = vlan_port->policer_id;
            gpData.p.failover_id = vlan_port->failover_id;
            gpData.p.failover_port_id = vlan_port->failover_port_id;
            if (gpData.e.flags & _BCM_FE2K_VLAN_GP_EXTERN_OHI) {
                if (!SOC_SBX_IS_VALID_L2_ENCAP_ID(vlan_port->encap_id) &&
                    !SOC_SBX_IS_VALID_ENCAP_ID(vlan_port->encap_id)) {
                    VLAN_ERR((VLAN_FUNCMSG1("invalid encap_id %08X setting"
                                            " unit %d VLAN GPORT %08X\n"),
                              vlan_port->encap_id,
                              unit,
                              gportId));
                    result = BCM_E_PARAM;
                } else {
                    gpData.p.encap_id = vlan_port->encap_id;
                }
            }
        } else { /* if ((vlan_port->flags & BCM_VLAN_PORT_REPLACE)) */
            /* not replace; set up all private data */
            sal_memcpy(&(gpData.p), vlan_port, sizeof(gpData.p));
            gpData.p.vsi = 0; /* never initially a member of a VSI */

            if (0 == gpData.p.encap_id ||
                (gpData.p.flags & BCM_VLAN_PORT_ENCAP_WITH_ID)) {
                gpData.e.flags &= (~_BCM_FE2K_VLAN_GP_EXTERN_OHI);
            } else if (SOC_SBX_IS_VALID_L2_ENCAP_ID(gpData.p.encap_id) ||
                       SOC_SBX_IS_VALID_ENCAP_ID(gpData.p.encap_id)) {
                gpData.e.flags |= _BCM_FE2K_VLAN_GP_EXTERN_OHI;
            } else {
                VLAN_ERR((VLAN_FUNCMSG1("invalid encap_id %08X setting"
                                        " unit %d VLAN GPORT %08X\n"),
                          gpData.p.encap_id,
                          unit,
                          gportId));
                result = BCM_E_PARAM;
            }
        } /* if ((vlan_port->flags & BCM_VLAN_PORT_REPLACE)) */
        /* make sure gport ID is set */
        BCM_GPORT_VLAN_PORT_ID_SET(gpData.p.vlan_port_id, gportId);
    } /* if (BCM_E_NONE == result) */


    /* program the hardware appropriately */
    if (BCM_E_NONE == result) {
#ifdef BCM_FE2000_P3_SUPPORT
        if (SOC_IS_SBX_G2P3(unit)) {
            result = _bcm_fe2000_g2p3_vlan_port_create(unit,
                                                       &gpData,
                                                       logicalPort);
            updated = TRUE;
        } else /* if (SOC_IS_SBX_G2P3(unit)) */
#endif /* BCM_FE2000_P3_SUPPORT */
        {
            VLAN_ERR((VLAN_FUNCMSG1("unsupported unit %d\n"), unit));
            result = BCM_E_UNIT;
        }
    } /* if (BCM_E_NONE == result) */

    
    if (BCM_SUCCESS(result) && BCM_GPORT_IS_TRUNK(vlan_port->port)) {
        tid  = BCM_GPORT_TRUNK_GET(vlan_port->port);        
        result = bcm_trunk_get(unit, tid, &trunk_info);
        if (BCM_FAILURE(result)) {
            VLAN_ERR((VLAN_FUNCMSG1("Failed to get trunk %d info: %s\n"),
                      tid, bcm_errmsg(result)));
        }
    }

    /* clean up */
    if (BCM_E_NONE == result) {
        /* keep the data around */
        sal_memcpy(gpPtr, &gpData, sizeof(*gpPtr));
        SBX_LPORT_DATAPTR(unit, logicalPort) = (void*)gpPtr;
        SBX_LPORT_TYPE(unit, logicalPort) = BCM_GPORT_VLAN_PORT;
        /* return the newly allocated gport ID */
        vlan_port->vlan_port_id = gpData.p.vlan_port_id;
        /* make sure the egress ID is in place */
        vlan_port->encap_id = gpData.p.encap_id;
        if ((vlan_port->flags & BCM_VLAN_PORT_REPLACE) && pulled) {
            /*
             *  Replace the data for a gport that was a member of a VSI and
             *  that we updated.  Note this only adjusts the ingress data and
             *  is therefore only used when the create function (above) is not
             *  enough to do the job (it covers egress data and ingress path
             *  after the p or pv or pvv decision).
             */
            /*
             *  WARNING: This does not directly affect vectors; it only affects
             *  the GPORT itself.  However, if the vectors are used as in some
             *  models and only replicate FT and LP references with pv2 e
             *  adjustment, it will implicitly include them in the update!
             */
            VLAN_VVERB((VLAN_FUNCMSG1("replacing VSI member;"
                                      " update ingress data\n")));
            result = _bcm_fe2000_vlan_vsi_port_program(unit,
                                                       oldVsi,
                                                       logicalPort,
                                                       vlan_port->vlan_port_id);
            if (BCM_E_NONE != result) {
                VLAN_ERR((VLAN_FUNCMSG1("unable to update ingress data"
                                        " replacing unit %d VLAN GPORT %08X:"
                                        " %d (%s)\n"),
                          unit,
                          vlan_port->vlan_port_id,
                          result,
                          _SHR_ERRMSG(result)));
            }
        }


        /* for gports on trunks, track it locally to manage trunk membership 
         * changes later when notified from the trunk module
         */
        DQ_INIT(&gpPtr->nt);
        if (BCM_GPORT_IS_TRUNK(gpPtr->p.port) &&
            tid < SBX_MAX_TRUNKS) 
        {
            _bcm_fe2k_trunk_state_t *ts = &_vlan_state[unit]->trunk[tid];
            
            if (DQ_EMPTY(&ts->vlan_port_head)) {
                sal_memcpy(&ts->tinfo, &trunk_info, sizeof(ts->tinfo));
            } else {
                /* Ensure the trunks are as expected */
	        bcm_trunk_add_info_t tmp_trunk_info;
                sal_memcpy(&tmp_trunk_info, &trunk_info, sizeof(ts->tinfo));
		tmp_trunk_info.dlf_index = ts->tinfo.dlf_index;
		tmp_trunk_info.ipmc_index = ts->tinfo.ipmc_index;
		tmp_trunk_info.mc_index = ts->tinfo.mc_index;
                if (sal_memcmp(&ts->tinfo, &tmp_trunk_info, sizeof(ts->tinfo))
                    != 0) 
                {
                    VLAN_ERR((VLAN_FUNCMSG1("Multpile gports on same trunk "
                                            "%d; trunk data mismatch\n"), 
                              tid));
                    result = BCM_E_INTERNAL;
                }
            }
            
            if (BCM_SUCCESS(result)) {
                DQ_INSERT_HEAD(&ts->vlan_port_head, &gpPtr->nt);
            }
        }
        
        /* also return the VSI, just in case it helps here */
        vlan_port->vsi = gpData.p.vsi;
    } else {
        /* something went wrong; unwrap allocated resources */
        if (allocFte) {
            gu2_res = SBX_GU2K_USR_RES_FTE_LOCAL_GPORT;
            _sbx_gu2_resource_free(unit, gu2_res, 1, &fti, 0);
        }

        if (allocLp) {
            if (SBX_LPORT_DATAPTR(unit, logicalPort)) {
                SBX_LPORT_TYPE(unit, logicalPort) = BCM_GPORT_INVALID;
                SBX_LPORT_DATAPTR(unit, logicalPort) = NULL;
            }
            if (gpPtr) {
                sal_free(gpPtr);
            }
            if (logicalPort < SBX_LPORT_END + 1) {
                gu2_res = SBX_GU2K_USR_RES_LPORT;
                _sbx_gu2_resource_free(unit, gu2_res, 1, &logicalPort, 0);
            }
        }

        if (((vlan_port->flags & (BCM_VLAN_PORT_REPLACE |
                                  BCM_VLAN_PORT_WITH_ID)) ==
             (BCM_VLAN_PORT_REPLACE | BCM_VLAN_PORT_WITH_ID)) &&
            updated) {
            VLAN_ERR((VLAN_FUNCMSG1("partial update may have occurred for"
                                    " unit %d GPORT %08X due to error during"
                                    " WITH_ID+REPLACE create operation\n"),
                      unit,
                      vlan_port->vlan_port_id));
        }
    }

    /* release the lock */
    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,*) return %d (%s)\n"),
               unit,
               result,
               _SHR_ERRMSG(result)));

    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_vsi_port_add
 *   Purpose
 *      Populate necessary tables based on vlan_port match criteria
 *   Parameters
 *      (in) unit    - BCM device number
 *      (in) vsi     - vsi port is to be added
 *      (in) gport   - gport to add
 *   Returns
 *      BCM_E_NONE if successfull
 *      BCM_E_* on error
 *  Notes -
 */
#define _VLAN_FUNC_NAME  "_bcm_fe2000_vlan_vsi_gport_add"
int
_bcm_fe2000_vlan_vsi_gport_add(int unit, bcm_vlan_t vsi, bcm_gport_t gport)
{
    int               rv;        /* local result code */
    uint32            gportId, fti, gu2_res;
    uint32_t          logicalPort = 0;
    _bcm_fe2k_vlan_gport_t *vlanPortData = NULL;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,%08X) enter\n"), unit, vsi, gport));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* make sure parameters are valid */
    if (!BCM_GPORT_IS_VLAN_PORT(gport)) {
        VLAN_ERR((VLAN_FUNCMSG1("gport %08X is not a valid VLAN port on"
                                " unit %d\n"),
                  gport,
                  unit));
        return BCM_E_PARAM;
    }
    if (((vsi < SBX_DYNAMIC_VSI_BASE(unit)) && (vsi > BCM_VLAN_MAX))
        || (vsi > SBX_DYNAMIC_VSI_END(unit))
        || (0 == vsi)
       ) {
        VLAN_ERR((VLAN_FUNCMSG1("VSI %04X is not valid on unit %d\n"),
                  vsi,
                  unit));
        return BCM_E_PARAM;
    }

    VLAN_LOCK_TAKE;

    /* get the gport information */
    gportId = BCM_GPORT_VLAN_PORT_ID_GET(gport);
    fti = VLAN_VGPORT_ID_TO_FT_INDEX(unit, gportId);

    gu2_res = SBX_GU2K_USR_RES_FTE_LOCAL_GPORT;
    rv = _sbx_gu2_resource_test(unit, gu2_res, fti);

    if (BCM_E_EXISTS == rv) {
        logicalPort = _vlan_state[unit]->gportInfo.lpid[gportId];
        rv = BCM_E_NONE;
        vlanPortData = (_bcm_fe2k_vlan_gport_t*)(SBX_LPORT_DATAPTR(unit,
                                                                   logicalPort));
        if (!vlanPortData) {
            VLAN_ERR((VLAN_FUNCMSG1("gport %08X is not valid on unit %d\n"),
                      gport,
                      unit));
            rv = BCM_E_NOT_FOUND;
        }
        if (BCM_GPORT_VLAN_PORT != SBX_LPORT_TYPE(unit, logicalPort)) {
            VLAN_ERR((VLAN_FUNCMSG1("unit %d gport %08X disagrees with"
                                    " stored type %02X\n"),
                      unit,
                      gport,
                      SBX_LPORT_TYPE(unit, logicalPort)));
            rv = BCM_E_CONFIG;
        }
    } else {
        VLAN_ERR((VLAN_FUNCMSG1("unable to access unit %d GPORT %08X:"
                                " %d (%s)\n"),
                  unit,
                  gport,
                  rv,
                  _SHR_ERRMSG(rv)));
    }

    /* make sure it's not already in some other VSI */
    if (BCM_E_NONE == rv) {
        if (vlanPortData->p.vsi) {
            VLAN_ERR((VLAN_FUNCMSG1("gport %08X already belongs to VSI %04X"
                                    " on unit %d\n"),
                      gport,
                      vlanPortData->p.vsi,
                      unit));
            rv = BCM_E_CONFIG;
        }
    }

    if (BCM_E_NONE == rv) {
        /* set the VSI membershp before programming the gport conifg */
        vlanPortData->p.vsi = vsi;

        /* everything looks good; commit the add */
        rv = _bcm_fe2000_vlan_vsi_port_program(unit, vsi, logicalPort, gport);

        /* clear the VSI membership on failure */
        if (BCM_FAILURE(rv)) {
            vlanPortData->p.vsi = 0;
        }
    }

    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,%08X) return %d (%s)\n"),
               unit,
               vsi,
               gport,
               rv,
               _SHR_ERRMSG(rv)));

    return rv;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_vsi_port_delete
 *   Purpose
 *      Clear necessary tables based on vlan_port match criteria
 *   Parameters
 *      (in) unit    - BCM device number
 *      (in) vsi     - vsi port is to be delete
 *      (in) gport   - gport to delete
 *   Returns
 *      BCM_E_NONE if successfull
 *      BCM_E_* on error
 */
#define _VLAN_FUNC_NAME  "_bcm_fe2000_vlan_vsi_gport_delete"
int
_bcm_fe2000_vlan_vsi_gport_delete(int unit, bcm_vlan_t vsi, bcm_gport_t gport)
{
    int                rv = BCM_E_NONE;        /* local result code */
    uint32             gportId, fti, gu2_res;
    uint32_t           logicalPort = 0;
    _bcm_fe2k_vlan_gport_t *vlanPortData = NULL;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,%08X) enter\n"), unit, vsi, gport));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* make sure parameters are valid */
    if (!BCM_GPORT_IS_VLAN_PORT(gport)) {
        VLAN_ERR((VLAN_FUNCMSG1("gport %08X is not a valid VLAN port on"
                                " unit %d\n"),
                  gport,
                  unit));
        return BCM_E_PARAM;
    }
    if (((vsi < SBX_DYNAMIC_VSI_BASE(unit)) && (vsi > BCM_VLAN_MAX))
        || (vsi > SBX_DYNAMIC_VSI_END(unit))
        || (0 == vsi)
       ) {
        VLAN_ERR((VLAN_FUNCMSG1("VSI %04X is not valid on unit %d\n"),
                  vsi,
                  unit));
        return BCM_E_PARAM;
    }

    VLAN_LOCK_TAKE;

    /* get the gport information */
    gportId = BCM_GPORT_VLAN_PORT_ID_GET(gport);
    fti = VLAN_VGPORT_ID_TO_FT_INDEX(unit, gportId);

    gu2_res = SBX_GU2K_USR_RES_FTE_LOCAL_GPORT;
    rv = _sbx_gu2_resource_test(unit, gu2_res, fti);

    if (BCM_E_EXISTS == rv) {
        logicalPort = _vlan_state[unit]->gportInfo.lpid[gportId];
        rv = BCM_E_NONE;
        vlanPortData = (_bcm_fe2k_vlan_gport_t*)(SBX_LPORT_DATAPTR(unit,
                                                                   logicalPort));
        if (!vlanPortData) {
            VLAN_ERR((VLAN_FUNCMSG1("gport %08X is not valid on unit %d\n"),
                      gport,
                      unit));
            rv = BCM_E_NOT_FOUND;
        }
        if (BCM_GPORT_VLAN_PORT != SBX_LPORT_TYPE(unit, logicalPort)) {
            VLAN_ERR((VLAN_FUNCMSG1("unit %d gport %08X disagrees with"
                                    " stored type %02X\n"),
                      unit,
                      gport,
                      SBX_LPORT_TYPE(unit, logicalPort)));
            rv = BCM_E_CONFIG;
        }
    } else {
        VLAN_ERR((VLAN_FUNCMSG1("unable to access unit %d GPORT %08X:"
                                " %d (%s)\n"),
                  unit,
                  gport,
                  rv,
                  _SHR_ERRMSG(rv)));
    }

    /* make sure it belongs to this VSI */
    if (BCM_E_NONE == rv) {
        if (vsi != vlanPortData->p.vsi) {
            VLAN_ERR((VLAN_FUNCMSG1("gport %08X belongs to VSI %04X on"
                                    " unit %d; can't remove from VSI %04X\n"),
                      gport,
                      vlanPortData->p.vsi,
                      unit,
                      vsi));
            rv = BCM_E_CONFIG;
        }
    }

    /* tear down match parameters */
    if (BCM_E_NONE == rv) {
        rv = _bcm_fe2000_vlan_vsi_port_program(unit, 0, 0, gport);
    }

    /* clear the VSI membership */
    if (BCM_E_NONE == rv) {
        vlanPortData->p.vsi = 0;
    }

    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,%08X) return %d (%s)\n"),
               unit,
               vsi,
               gport,
               rv,
               _SHR_ERRMSG(rv)));

    return rv;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_vsi_port_get
 *   Purpose
 *      Get VSI for the given gport
 *   Parameters
 *      (in) unit    - BCM device number
 *      (in) gport   - GPORT to check
 *      (out) vsi    - where to put the VSI to which the GPORT belongs
 *   Returns
 *      BCM_E_NONE if successfull
 *      BCM_E_* on error
 */
#define _VLAN_FUNC_NAME  "_bcm_fe2000_vlan_vsi_gport_get"
int
_bcm_fe2000_vlan_vsi_gport_get(int unit,
                               bcm_gport_t gport,
                               bcm_vlan_t *vsi)
{
    int              rv = BCM_E_NONE;                /* local result code */
    uint32_t         logicalPort = 0;
    uint32           gportId, fti, gu2_res;
    _bcm_fe2k_vlan_gport_t *vlanPortData = NULL;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%08X,*) enter\n"), unit, gport));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* make sure parameters are valid */
    if (!BCM_GPORT_IS_VLAN_PORT(gport)) {
        VLAN_ERR((VLAN_FUNCMSG1("gport %08X is not a valid VLAN port on"
                                " unit %d\n"),
                  gport,
                  unit));
        return BCM_E_PARAM;
    }
    if (!vsi) {
        VLAN_ERR((VLAN_FUNCMSG1("NULL pointer to outbound argument\n")));
        return BCM_E_PARAM;
    }

    VLAN_LOCK_TAKE;

    /* get the gport information */
    gportId = BCM_GPORT_VLAN_PORT_ID_GET(gport);
    fti = VLAN_VGPORT_ID_TO_FT_INDEX(unit, gportId);

    gu2_res = SBX_GU2K_USR_RES_FTE_LOCAL_GPORT;
    rv = _sbx_gu2_resource_test(unit, gu2_res, fti);

    if (BCM_E_EXISTS == rv) {
        logicalPort = _vlan_state[unit]->gportInfo.lpid[gportId];
        rv = BCM_E_NONE;
        vlanPortData = (_bcm_fe2k_vlan_gport_t*)(SBX_LPORT_DATAPTR(unit,
                                                                   logicalPort));
        if (!vlanPortData) {
            VLAN_ERR((VLAN_FUNCMSG1("gport %08X is not valid on unit %d\n"),
                      gport,
                      unit));
            rv = BCM_E_NOT_FOUND;
        }
        if (BCM_GPORT_VLAN_PORT != SBX_LPORT_TYPE(unit, logicalPort)) {
            VLAN_ERR((VLAN_FUNCMSG1("unit %d gport %08X disagrees with"
                                    " stored type %02X\n"),
                      unit,
                      gport,
                      SBX_LPORT_TYPE(unit, logicalPort)));
            rv = BCM_E_CONFIG;
        }
    } else {
        VLAN_ERR((VLAN_FUNCMSG1("unable to access unit %d GPORT %08X:"
                                " %d (%s)\n"),
                  unit,
                  gport,
                  rv,
                  _SHR_ERRMSG(rv)));
    }

    if (BCM_E_NONE == rv) {
        *vsi = vlanPortData->p.vsi;
    }

    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%08X,&(%04X)) return %d (%s)\n"),
               unit,
               gport,
               *vsi,
               rv,
               _SHR_ERRMSG(rv)));

    return rv;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_fte_gport_get
 *   Purpose
 *      Get FT entry index for the given gport
 *   Parameters
 *      (in) unit    - BCM device number
 *      (in) gport   - GPORT to check
 *      (in) ftei    - where to put the VSI to which the GPORT belongs
 *   Returns
 *      BCM_E_NONE if successfull
 *      BCM_E_* on error
 */
#define _VLAN_FUNC_NAME  "_bcm_fe2000_vlan_fte_gport_get"
int
_bcm_fe2000_vlan_fte_gport_get(int unit,
                               bcm_gport_t gport,
                               uint32 *ftei)
{
    int              rv = BCM_E_NONE;                /* local result code */
    bcm_module_t     locMod = ~0;
    bcm_module_t     tgtMod = -1;
    bcm_port_t       tgtPort = ~0;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%08X,*) enter\n"), unit, gport));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* make sure parameters are valid */
    if (!BCM_GPORT_IS_VLAN_PORT(gport)) {
        VLAN_ERR((VLAN_FUNCMSG1("gport %08X is not a valid VLAN port on"
                                " unit %d\n"),
                  gport,
                  unit));
        return BCM_E_PARAM;
    }
    if (!ftei) {
        VLAN_ERR((VLAN_FUNCMSG1("NULL pointer to outbound argument\n")));
        return BCM_E_PARAM;
    }

    VLAN_LOCK_TAKE;

    rv = _bcm_fe2000_map_vlan_gport_targets(unit,
                                            gport,
                                            &locMod,
                                            &tgtMod,
                                            &tgtPort,
                                            ftei,
                                            NULL);

    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%08X,&(%08X)) return %d (%s)\n"),
               unit,
               gport,
               *ftei,
               rv,
               _SHR_ERRMSG(rv)));

    return rv;
}
#undef _VLAN_FUNC_NAME


#ifdef BCM_FE2000_P3_SUPPORT
/*
 *   Function
 *      _bcm_fe2000_g2p3_vlan_port_destroy
 *   Purpose
 *      Remove a logical port from the p3 hardware
 *   Parameters
 *      (in) unit        = BCM device number
 *      (in) logicalPort = logical port to destroy
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *  Notes:
 *    Assumes all parameters have be validated an locks taken
 */
#define _VLAN_FUNC_NAME  "_bcm_fe2000_g2p3_vlan_port_destroy"
static int
_bcm_fe2000_g2p3_vlan_port_destroy(int unit,
                                   _bcm_fe2k_vlan_gport_t *vlan_port,
                                   uint32 logicalPort)
{
    int                   result;                /* local result code */
    soc_sbx_g2p3_lp_t     p3LogicalPort;         /* hw logical port  */
    soc_sbx_g2p3_ft_t     p3fte;                 /* G2P3 FT entry */
    soc_sbx_g2p3_oi2e_t   p3outHdrIndex2Etc;     /* G2P3 OHI2Etc */
    soc_sbx_g2p3_etel2_t  p3eteL2;               /* G2P3 L2 ETE */
    soc_sbx_g2p3_eteencap_t p3eteEncap;          /* G2P3 Encap ETE */
    uint32                eteL2Index = 0;        /* ETE L2 index */
    uint32                eteEncapIndex = 0;     /* ETE encap index */
    uint32                ohIndex = 0;           /* OH index */
    uint32                fteIndex, gu2_res;

    /* destroy the FT entry */
    soc_sbx_g2p3_ft_t_init(&p3fte);
    p3fte.excidx = VLAN_INV_FTE_EXC;
    
    fteIndex = BCM_GPORT_VLAN_PORT_ID_GET(vlan_port->p.vlan_port_id);
    fteIndex = VLAN_VGPORT_ID_TO_FT_INDEX(unit, fteIndex);
    
    result = soc_sbx_g2p3_ft_set(unit, fteIndex, &p3fte);
    
    gu2_res = SBX_GU2K_USR_RES_FTE_LOCAL_GPORT;
    _sbx_gu2_resource_free(unit, gu2_res, 1, &fteIndex,0);
    
    if (BCM_E_NONE != result) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:ft[0x%08X]:"
                                " %d (%s)\n"),
                  unit, fteIndex,
                  result, _SHR_ERRMSG(result)));
    }

    /* figure out the OH */
    if (SOC_SBX_IS_VALID_L2_ENCAP_ID(vlan_port->p.encap_id)) {
        ohIndex = SOC_SBX_OHI_FROM_L2_ENCAP_ID(vlan_port->p.encap_id);
    } else if (SOC_SBX_IS_VALID_ENCAP_ID(vlan_port->p.encap_id)) {
        ohIndex = SOC_SBX_OHI_FROM_ENCAP_ID(vlan_port->p.encap_id);
    } else {
        VLAN_ERR((VLAN_FUNCMSG1("unexpected encap_id %08X\n"),
                  vlan_port->p.encap_id));
        result = BCM_E_INTERNAL;
    }

    /* get Encap ETE index  */
    if (BCM_SUCCESS(result) &&
        !(vlan_port->e.flags & _BCM_FE2K_VLAN_GP_EXTERN_OHI))
    {
        /* okay and not an externally managed encap_id, or none at all */
        result = soc_sbx_g2p3_oi2e_get(unit,
                                       ohIndex - SBX_RAW_OHI_BASE,
                                       &p3outHdrIndex2Etc);
        if (BCM_E_NONE == result) {
            eteEncapIndex = p3outHdrIndex2Etc.eteptr;
        } else {
            VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:oi2e[%08X]:"
                                    " %d (%s)\n"),
                      unit, ohIndex, result, _SHR_ERRMSG(result)));
        }
    }

    /* get L2 ETE  */
    if (BCM_SUCCESS(result) &&
        !(vlan_port->e.flags & _BCM_FE2K_VLAN_GP_EXTERN_OHI))
    {
        /* okay and not an externally managed encap_id, or none at all */
        result = soc_sbx_g2p3_eteencap_get(unit, eteEncapIndex, &p3eteEncap);
        if (BCM_E_NONE == result) {
            eteL2Index = p3eteEncap.l2ete;
        } else {
            VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:ETE[0x%08X]Encap:"
                                    " %d (%s)\n"),
                      unit, eteEncapIndex, result, _SHR_ERRMSG(result)));
        }
    }

    /* destroy the outHdrIdx2Etc */
    if (BCM_SUCCESS(result) &&
        !(vlan_port->e.flags & _BCM_FE2K_VLAN_GP_EXTERN_OHI))
    {
        /* okay and not an externally managed encap_id, or none at all */
        soc_sbx_g2p3_oi2e_t_init(&p3outHdrIndex2Etc);
        result = soc_sbx_g2p3_oi2e_set(unit,
                                       ohIndex - SBX_RAW_OHI_BASE,
                                       &p3outHdrIndex2Etc);

        _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_OHI,
                               1, &ohIndex, 0);
        if (BCM_E_NONE != result) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:oi2e[0x%08X]:"
                                    " %d (%s)\n"),
                      unit, ohIndex, result, _SHR_ERRMSG(result)));
        }
    }

    /* destroy the Encap ETE */
    if (BCM_SUCCESS(result) &&
        !(vlan_port->e.flags & _BCM_FE2K_VLAN_GP_EXTERN_OHI))
    {
        /* okay and not caller provided egress path */
        soc_sbx_g2p3_eteencap_t_init(&p3eteEncap);
        result = soc_sbx_g2p3_eteencap_set(unit, eteEncapIndex, &p3eteEncap);

        _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_ETE_ENCAP,
                               1, &eteEncapIndex, 0);
        if (BCM_E_NONE != result) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:ete[0x%08X]Encap:"
                                    " %d (%s)\n"),
                      unit, eteL2Index, result, _SHR_ERRMSG(result)));
        }
    }

    /* destroy the ETE */
    if (BCM_SUCCESS(result) &&
        !(vlan_port->e.flags & _BCM_FE2K_VLAN_GP_EXTERN_OHI))
    {
        /* okay and local module and not caller provided egress path */
        soc_sbx_g2p3_etel2_t_init(&p3eteL2);
        result = soc_sbx_g2p3_etel2_set(unit, eteL2Index, &p3eteL2);

        _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_ETE_L2,
                               1, &eteL2Index, 0);
        if (BCM_E_NONE != result) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:ete[0x%08X]L2:"
                                    " %d (%s)\n"),
                      unit, eteL2Index, result, _SHR_ERRMSG(result)));
        }
    }

    /* destroy the LPort */
    if (BCM_SUCCESS(result)) {
        soc_sbx_g2p3_lp_t_init(&p3LogicalPort);
        result = soc_sbx_g2p3_lp_set(unit, logicalPort, &p3LogicalPort);
        if (BCM_E_NONE != result) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:lp[0x%08X]: "
                                    "%d (%s)\n"),
                      unit, logicalPort, result, _SHR_ERRMSG(result)));
        }
    }

    return result;
}
#undef _VLAN_FUNC_NAME
#endif /* BCM_FE2000_P3_SUPPORT */

/*
 *   Function
 *      bcm_fe2000_vlan_port_destroy
 *   Purpose
 *      Destroy a logical port and free all associated resources
 *   Parameters
 *      (in) unit      = BCM device number
 *      (in) gport     = logical port in gport form to destroy
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_port_destroy"
int
bcm_fe2000_vlan_port_destroy(int unit,
                             bcm_gport_t gport)
{
    int     result = BCM_E_NONE;                     /* local result code */
    uint32  logicalPort = 0;                        /* logical port number */
    uint32  gportId, fti, gu2_res;
    _bcm_fe2k_vlan_gport_t *vlanPortData = NULL;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%08X) enter\n"), unit, gport));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* check valid parameters */
    if (!BCM_GPORT_IS_VLAN_PORT(gport)) {
        VLAN_ERR((VLAN_FUNCMSG1("invalid gport %08X for vlan port\n"), gport));
        return BCM_E_PARAM;
    }

    /* get the vlan lock */
    VLAN_LOCK_TAKE;

    /* get the gport information */
    gportId = BCM_GPORT_VLAN_PORT_ID_GET(gport);
    fti = VLAN_VGPORT_ID_TO_FT_INDEX(unit, gportId);

    gu2_res = SBX_GU2K_USR_RES_FTE_LOCAL_GPORT;
    result = _sbx_gu2_resource_test(unit, gu2_res, fti);

    if (BCM_E_EXISTS == result) {
        logicalPort = _vlan_state[unit]->gportInfo.lpid[gportId];
        result = BCM_E_NONE;
        vlanPortData = (_bcm_fe2k_vlan_gport_t*)(SBX_LPORT_DATAPTR(unit,
                                                                   logicalPort));
        if (!vlanPortData) {
            VLAN_ERR((VLAN_FUNCMSG1("gport %08X is not valid on unit %d\n"),
                      gport,
                      unit));
            result = BCM_E_NOT_FOUND;
        }
        if (BCM_GPORT_VLAN_PORT != SBX_LPORT_TYPE(unit, logicalPort)) {
            VLAN_ERR((VLAN_FUNCMSG1("unit %d gport %08X disagrees with"
                                    " stored type %02X\n"),
                      unit,
                      gport,
                      SBX_LPORT_TYPE(unit, logicalPort)));
            result = BCM_E_CONFIG;
        }
    } else {
        VLAN_ERR((VLAN_FUNCMSG1("unable to access unit %d GPORT %08X:"
                                " %d (%s)\n"),
                  unit,
                  gport,
                  result,
                  _SHR_ERRMSG(result)));
    }

    /* make sure it's not a member of any VSIs */
    if (BCM_E_NONE == result) {
        if (vlanPortData->p.vsi) {
            VLAN_ERR((VLAN_FUNCMSG1("gport %08X on unit %d belongs to VSI %04X\n"),
                      gport,
                      unit,
                      vlanPortData->p.vsi));
            result = BCM_E_BUSY;
        }
    }

    /* destroy the vlan port */
    if (BCM_E_NONE == result) {
#ifdef BCM_FE2000_P3_SUPPORT
        if (SOC_IS_SBX_G2P3(unit)) {
            result = _bcm_fe2000_g2p3_vlan_port_destroy(unit,
                                                        vlanPortData,
                                                        logicalPort);
        } else
#endif /* BCM_FE2000_P3_SUPPORT */
        {
            VLAN_ERR((VLAN_FUNCMSG1("unsupported unit %d\n"), unit));
            result = BCM_E_UNIT;
        }
        if (BCM_E_NONE != result) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to destroy VLAN GPORT ID %08X"
                                    " on unit %d: %d (%s)\n"),
                      gport,
                      unit,
                      result,
                      _SHR_ERRMSG(result)));
        }
    }

    /* indicate the logical port is now free */
    if (BCM_E_NONE == result) {
        if (BCM_GPORT_IS_TRUNK(vlanPortData->p.port)) {
            DQ_REMOVE(&vlanPortData->nt);
        }

        SBX_LPORT_DATAPTR(unit, logicalPort) = NULL;
        SBX_LPORT_TYPE(unit, logicalPort) = BCM_GPORT_INVALID;
        result = _sbx_gu2_resource_free(unit,
                                        SBX_GU2K_USR_RES_LPORT,
                                        1,
                                        &logicalPort,
                                        0);
        sal_free(vlanPortData);
        vlanPortData = NULL;

        if (BCM_E_NONE != result) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to release logical port ID %08X"
                                    " on unit %d: %d (%s)\n"),
                      logicalPort,
                      unit,
                      result,
                      _SHR_ERRMSG(result)));
        }
    }

    if (BCM_E_NONE == result) {
        if (BCM_E_NONE == result) {
            _vlan_state[unit]->gportInfo.lpid[gportId] = 0;
        } else {
            VLAN_ERR((VLAN_FUNCMSG1("unable to release VLAN GPORT ID %08X"
                                    " on unit %d: %d (%s)\n"),
                      gport,
                      unit,
                      result,
                      _SHR_ERRMSG(result)));
        }
    }

    /* release the lock */
    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%08X) return %d (%s)\n"),
               unit,
               gport,
               result,
               _SHR_ERRMSG(result)));

    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_port_get_lpid
 *   Purpose
 *      Find a logical port based upon the provided VLAN GPORT ID
 *   Parameters
 *      (in) unit      = BCM device number
 *      (in) gport     = VLAN GPORT to be found
 *      (out) lpid     = LP ID for the GPORT
 *      (out) pport    = physical port for the GPORT
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_port_get_lpid"
int
bcm_fe2000_vlan_port_get_lpid(int unit,
                              bcm_gport_t gport,
                              uint32 *lpid,
                              bcm_port_t *pport)
{
    bcm_port_t             tgtPort = ~0;         /* target port */
    bcm_module_t           tgtModule = -1;       /* target module */
    uint32                 tgtQueue = ~0;        /* target queue */
    bcm_module_t           myModule = ~0;        /* local module */
    uint32                 logicalPort;          /* logical port number */
    uint32                 gportId, fti, gu2_res;
    _bcm_fe2k_vlan_gport_t *vlanPortData = NULL;
    int                    result;

    VLAN_EVERB((VLAN_FUNCMSG("(%d,%08X,*) enter\n"),
                unit,
                gport));

    if ((!lpid) || (!pport)) {
        VLAN_ERR((VLAN_FUNCMSG1("NULL not acceptable for out argument\n")));
        return BCM_E_PARAM;
    }
    if (!BCM_GPORT_IS_VLAN_PORT(gport)) {
        VLAN_ERR((VLAN_FUNCMSG1("GPORT %08X is not VLAN GPORT\n"), gport));
        return BCM_E_PARAM;
    }

    /* get the vlan lock */
    VLAN_LOCK_TAKE;

    gportId = BCM_GPORT_VLAN_PORT_ID_GET(gport);
    fti = VLAN_VGPORT_ID_TO_FT_INDEX(unit, gportId);

    gu2_res = SBX_GU2K_USR_RES_FTE_LOCAL_GPORT;
    result = _sbx_gu2_resource_test(unit, gu2_res, fti);

    if (BCM_E_EXISTS == result) {
        logicalPort = _vlan_state[unit]->gportInfo.lpid[gportId];
        result = BCM_E_NONE;
        vlanPortData = (_bcm_fe2k_vlan_gport_t*)(SBX_LPORT_DATAPTR(unit,
                                                                   logicalPort));
        if (!vlanPortData) {
            VLAN_ERR((VLAN_FUNCMSG1("gport %08X is not valid on unit %d\n"),
                      gport,
                      unit));
            result = BCM_E_NOT_FOUND;
        }
        if (BCM_GPORT_VLAN_PORT != SBX_LPORT_TYPE(unit, logicalPort)) {
            VLAN_ERR((VLAN_FUNCMSG1("unit %d gport %08X disagrees with"
                                    " stored type %02X\n"),
                      unit,
                      gport,
                      SBX_LPORT_TYPE(unit, logicalPort)));
            result = BCM_E_CONFIG;
        }
        if (BCM_E_NONE == result) {
            *lpid = logicalPort;
            result = _bcm_fe2000_map_vlan_gport_targets(unit,
                                                        vlanPortData->p.port,
                                                        &myModule,
                                                        &tgtModule,
                                                        &tgtPort,
                                                        NULL,
                                                        &tgtQueue);
            if ((BCM_E_NONE == result) && (tgtModule == myModule)) {
                *pport = tgtPort;
            } else {
                *pport = -1;
            }
        }
    } else if (BCM_E_NONE == result) {
        VLAN_ERR((VLAN_FUNCMSG1("invalid condition accessing GPORT %08X"
                                " on unit %d\n"),
                  gport,
                  unit));
        result = BCM_E_INTERNAL;
    } else {
        VLAN_ERR((VLAN_FUNCMSG1("unable to access GPORT %08X on unit %d:"
                                " %d (%s)\n"),
                  gport,
                  unit,
                  result,
                  _SHR_ERRMSG(result)));
    }

    /* release the lock */
    VLAN_LOCK_RELEASE;

    VLAN_EVERB((VLAN_FUNCMSG("(%d,%08X,*) return %d (%s)\n"),
                unit,
                gport,
                result,
                _SHR_ERRMSG(result)));

    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_port_lookup
 *   Purpose
 *      Find a logical port by reading the caller's mind and the hardware
 *   Parameters
 *      (in) unit      = BCM device number
 *      (in/out) gport = logical port in gport form to look up
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Computers aren't particularly good mindreaders, but at least they can
 *      read tables well.
 *      This reads p2e (to determine port mode, mostly, but also checks the
 *      logicalPort there), pvv2e if the port is in a mode that allows use of
 *      that table, and pv2e, and returns what seems to be the best match.  If
 *      the 'criteria' field implies any particular table, it only checks that
 *      table.
 *      Considers trying to search on the wrong module to be BCM_E_FAIL.
 *      Assumes all locks are taken; does little parameter checking.
 *      Completely overwrites the lookup key with the result if found.
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_port_lookup"
static int
_bcm_fe2000_vlan_port_lookup(int unit, bcm_vlan_port_t *vlanPort)
{
    int result;                                 /* working result */
    uint32 p2elp = ~0;                          /* p2e's LP */
    uint32 pv2elp = ~0;                         /* pv2e's LP */
    uint32 pvv2elp = ~0;                        /* pvv2e's LP */
    uint32 lp = ~0;                             /* chosen LP ID */
    _bcm_fe2k_vlan_gport_t *lpDesc = NULL;      /* located LP description */
    bcm_vlan_port_t *gpDesc = NULL;             /* located GPORT description */
    bcm_port_t            tgtPort = ~0;         /* target port */
    bcm_module_t          tgtModule = -1;       /* target module */
    uint32                tgtQueue = ~0;        /* target queue */
    bcm_module_t          myModule = ~0;        /* local module */
#ifdef BCM_FE2000_P3_SUPPORT
#if 0 
    soc_sbx_g2p3_p2e_t p3p2e;                   /* G2P3 port2etc */
#endif 
    soc_sbx_g2p3_pv2e_t p3pv2e;                 /* G2P3 pVid2Etc */
    soc_sbx_g2p3_pvv2e_t p3pvv2e;               /* G2P3 pVidVid2Etc */
#endif /* BCM_FE2000_P3_SUPPORT */

    VLAN_VVERB((VLAN_FUNCMSG("(%d,*)\n"), unit));

    /* parse target port information */
    result = _bcm_fe2000_map_vlan_gport_targets(unit,
                                                vlanPort->port,
                                                &myModule,
                                                &tgtModule,
                                                &tgtPort,
                                                NULL,
                                                &tgtQueue);
    if (BCM_E_NONE == result) {
        if (myModule != tgtModule) {
            VLAN_ERR((VLAN_FUNCMSG1("can't do direct lookup of VLAN GPORT on"
                                    " this module (%d); its home is on"
                                    " module %d\n"),
                      myModule,
                      tgtModule));
            return BCM_E_FAIL;
        }
    } else {
        VLAN_ERR((VLAN_FUNCMSG1("unable to determine target data: %d (%s)\n"),
                  result,
                  _SHR_ERRMSG(result)));
        return result;
    }

    /* read the tables and see if we find a VLAN GPORT in the path */
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
#if 0 
        result = soc_sbx_g2p3_p2e_get(unit, tgtPort, &p3p2e);
        if (BCM_E_NONE != result) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:p2e[%d]: %d (%s)\n"),
                      unit,
                      tgtPort,
                      result,
                      _SHR_ERRMSG(result)));
            return result;
        }
#endif 
        p2elp = tgtPort; /* implicit */
        result = SOC_SBX_G2P3_PV2E_GET(unit,
                                       tgtPort,
                                       vlanPort->match_vlan,
                                       &p3pv2e);
        if (BCM_E_NONE != result) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:pv2e[%d,%03X]:"
                                    " %d (%s)\n"),
                      unit,
                      tgtPort,
                      vlanPort->match_vlan,
                      result,
                      _SHR_ERRMSG(result)));
            return result;
        }
        pv2elp = p3pv2e.lpi;
        
        result = soc_sbx_g2p3_pvv2e_get(unit,
                                        vlanPort->match_inner_vlan,
                                        vlanPort->match_vlan,
                                        tgtPort,
                                        &p3pvv2e);
        if (BCM_E_NONE == result) {
            /* only set pvv2elp if found... */
            pvv2elp = p3pvv2e.lpi;
        } else if (BCM_E_NOT_FOUND == result) {
            /* ...but don't balk if pvv2e not found... */
            result = BCM_E_NONE;
        } else {
            /* ...unless it's because of some unexpected problem  */
            VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:pvv2e[%d,%03X,%03X]:"
                                    " %d (%s)\n"),
                      unit,
                      tgtPort,
                      vlanPort->match_vlan,
                      vlanPort->match_inner_vlan,
                      result,
                      _SHR_ERRMSG(result)));
            return result;
        }
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        VLAN_ERR((VLAN_FUNCMSG1("unknown/unsupported microcode on unit %d\n"),
                  unit));
        return BCM_E_FAIL;
    } /* switch (SOC_SBX_CONTROL(unit)->ucodetype) */

    /* choose which logical port to use */
    switch (vlanPort->criteria) {
    case BCM_VLAN_PORT_MATCH_PORT:
        lp = p2elp;
        break;
    case BCM_VLAN_PORT_MATCH_PORT_VLAN:
        lp = pv2elp;
        break;
    case BCM_VLAN_PORT_MATCH_PORT_VLAN_STACKED:
        lp = pvv2elp;
        break;
    default:
        /* not specified/unknown; just guess based upon best hit */
        lp = p2elp;
        if (((~0) != pv2elp) && (0 != pv2elp)) {
            lp = pv2elp;
        }
        if (((~0) != pvv2elp) && (0 != pvv2elp)) {
            lp = pvv2elp;
        }
    } /* switch (vlanPort->criteria) */

    /* look up the logical port and see if we have a VLAN GPORT */
    if ((SBX_LPORT_BASE <= lp) && (SBX_LPORT_END >= lp)) {
        if (BCM_GPORT_VLAN_PORT == SBX_LPORT_TYPE(unit, lp)) {
            /* looks like a VLAN GPORT; get its information */
            lpDesc = (_bcm_fe2k_vlan_gport_t*)SBX_LPORT_DATAPTR(unit, lp);
            if (lpDesc) {
                gpDesc = &(lpDesc->p);
            }
            if (gpDesc) {
                /* found one; seems valid; copy it out */
                VLAN_VVERB((VLAN_FUNCMSG("(%d,*) found GPORT %08X at"
                                         " %d:lp[%08X]\n"),
                            unit,
                            gpDesc->vlan_port_id,
                            unit,
                            lp));
                sal_memcpy(vlanPort, gpDesc, sizeof(*vlanPort));
            } else {
                VLAN_ERR((VLAN_FUNCMSG1("apparently %d:lp[%08X] is not a"
                                        " VLAN GPORT's logical port\n"),
                          unit,
                          lp));
                return BCM_E_FAIL;
            }
        } else { /* if (BCM_GPORT_VLAN_PORT == SBX_LPORT_TYPE(unit, lp)) */
            VLAN_ERR((VLAN_FUNCMSG1("%d:lp[%08X] is not a VLAN PORT's"
                                    " logical port\n"),
                      unit,
                      lp));
            return BCM_E_FAIL;
        }
    } else {
        VLAN_ERR((VLAN_FUNCMSG1("%d:lp[%08X] does not exist\n"),
                  unit,
                  lp));
        return BCM_E_FAIL;
    }

    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_port_find
 *   Purpose
 *      Find a logical port by reading the caller's mind
 *   Parameters
 *      (in) unit      = BCM device number
 *      (in) gport     = logical port in gport form to find
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Computers aren't particularly good mindreaders.
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_port_find"
int
bcm_fe2000_vlan_port_find(int unit, bcm_vlan_port_t *vlan_port)
{
    int     result = BCM_E_NONE;                     /* local result code */
    uint32  logicalPort;                             /* logical port number */
    uint32  gportId, fti, gu2_res;
    _bcm_fe2k_vlan_gport_t *vlanPortData = NULL;
#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_ft_t p3fte;                         /* G2P3 FTE */
    soc_sbx_g2p3_oi2e_t p3oi2e;                      /* G2P3 oi2e entry */
    soc_sbx_g2p3_eteencap_t p3eteEncap;              /* G2P3 encap ETE */
    soc_sbx_g2p3_etel2_t p3eteL2;                    /* G2P3 L2 ETE */
#endif /* def BCM_FE2000_P3_SUPPORT */
    bcm_port_t tgtPort = ~0;                         /* target port */
    bcm_module_t tgtModule = -1;                     /* target module */
    bcm_module_t myModule = ~0;                      /* local module */

    VLAN_VERB((VLAN_FUNCMSG("(%d,*) enter\n"), unit));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* make sure arguments are valid */
    if (!vlan_port) {
        VLAN_ERR((VLAN_FUNCMSG1("can't look for a GPORT on unit %d with a"
                                " NULL description\n"),
                  unit));
        return BCM_E_PARAM;
    }

    /* get the vlan lock */
    VLAN_LOCK_TAKE;

    /* try to guess what the caller wanted and match on it */
    if (BCM_GPORT_IS_VLAN_PORT(vlan_port->vlan_port_id) &&
        ((BCM_VLAN_PORT_MATCH_INVALID == vlan_port->criteria) ||
         (BCM_VLAN_PORT_MATCH_NONE == vlan_port->criteria))) {
        /* caller provided a gport and no match; look up by gport */
        VLAN_VVERB((VLAN_FUNCMSG1("find unit %d gport %08X by ID\n"),
                    unit,
                    vlan_port->vlan_port_id));
        /* get the gport information */
        gportId = BCM_GPORT_VLAN_PORT_ID_GET(vlan_port->vlan_port_id);
        fti = VLAN_VGPORT_ID_TO_FT_INDEX(unit, gportId);

        gu2_res = SBX_GU2K_USR_RES_FTE_LOCAL_GPORT;
        result = _sbx_gu2_resource_test(unit, gu2_res, fti);

        if (BCM_E_EXISTS == result) {
            logicalPort = _vlan_state[unit]->gportInfo.lpid[gportId];
            result = BCM_E_NONE;
            vlanPortData = (_bcm_fe2k_vlan_gport_t*)(SBX_LPORT_DATAPTR(unit,
                                                                       logicalPort));
            if (!vlanPortData) {
                VLAN_ERR((VLAN_FUNCMSG1("gport %08X is not valid on unit %d\n"),
                          vlan_port->vlan_port_id,
                          unit));
                result = BCM_E_NOT_FOUND;
            }
            if (BCM_GPORT_VLAN_PORT != SBX_LPORT_TYPE(unit, logicalPort)) {
                VLAN_ERR((VLAN_FUNCMSG1("unit %d gport %08X disagrees with"
                                        " stored type %02X\n"),
                          unit,
                          vlan_port->vlan_port_id,
                          SBX_LPORT_TYPE(unit, logicalPort)));
                result = BCM_E_CONFIG;
            }
            if (BCM_E_NONE == result) {
                /* gport is valid and correct type; copy it out */
                sal_memcpy(vlan_port, &(vlanPortData->p), sizeof(*vlan_port));
            }
        } else {
            VLAN_ERR((VLAN_FUNCMSG1("unable to access unit %d GPORT %08X:"
                                    " %d (%s)\n"),
                      unit,
                      vlan_port->vlan_port_id,
                      result,
                      _SHR_ERRMSG(result)));
        }
    } else if ((0 == vlan_port->vlan_port_id) &&
               ((BCM_VLAN_PORT_MATCH_PORT == vlan_port->criteria) ||
                (BCM_VLAN_PORT_MATCH_PORT_VLAN == vlan_port->criteria) ||
                (BCM_VLAN_PORT_MATCH_PORT_VLAN_STACKED == vlan_port->criteria))) {
        /* caller provided no gport but valid criteria */
        /* try lookup to see if the gport is live somewhere */
        result = _bcm_fe2000_vlan_port_lookup(unit, vlan_port);
        if (BCM_E_NONE == result) {
            /* found it but need some pointers */
            gportId = BCM_GPORT_VLAN_PORT_ID_GET(vlan_port->vlan_port_id);
            logicalPort = _vlan_state[unit]->gportInfo.lpid[gportId];
            vlanPortData = (_bcm_fe2k_vlan_gport_t*)(SBX_LPORT_DATAPTR(unit,
                                                                       logicalPort));
        } else if (BCM_E_FAIL == result && vlan_port->vsi == 0) {
            /* not found in live lookup; search tables instead */
            VLAN_VVERB((VLAN_FUNCMSG1("checking GPORT list\n")));
            result = BCM_E_NOT_FOUND;
            /* look for a matching logical port */
            for (logicalPort = SBX_LPORT_BASE;
                 logicalPort <= SBX_LPORT_END;
                 logicalPort ++) {
                if (BCM_GPORT_VLAN_PORT == SBX_LPORT_TYPE(unit, logicalPort)) {
                    /* get the cached data for this lport */
                    vlanPortData = (_bcm_fe2k_vlan_gport_t*)
                                   (SBX_LPORT_DATAPTR(unit, logicalPort));
                    /* verify the matching information */
                    VLAN_EVERB((VLAN_FUNCMSG1("check logical port %08X\n"),
                                logicalPort));
                    switch (vlan_port->criteria) {
                    case BCM_VLAN_PORT_MATCH_PORT_VLAN_STACKED:
                        if (vlan_port->match_inner_vlan !=
                            vlanPortData->p.match_inner_vlan) {
                            VLAN_EVERB((VLAN_FUNCMSG1("inner VID %03X != %03X\n"),
                                        vlan_port->match_inner_vlan,
                                        vlanPortData->p.match_inner_vlan));
                            vlanPortData = NULL;
                            break;
                        }
                        /* yes, the break belongs within the if */
                    case BCM_VLAN_PORT_MATCH_PORT_VLAN:
                        if (vlan_port->match_vlan != vlanPortData->p.match_vlan) {
                            VLAN_EVERB((VLAN_FUNCMSG1("VID %03X != %03X\n"),
                                        vlan_port->match_vlan,
                                        vlanPortData->p.match_vlan));
                            vlanPortData = NULL;
                            break;
                        }
                        /* yes, the break belongs within the if */
                    case BCM_VLAN_PORT_MATCH_PORT:
                        if (vlan_port->port != vlanPortData->p.port) {
                            VLAN_EVERB((VLAN_FUNCMSG1("Port %d != %d\n"),
                                        vlan_port->port,
                                        vlanPortData->p.port));
                            vlanPortData = NULL;
                            break;
                        }
                        /* yes, the break belongs within the if */
                    default:
                        if (vlan_port->criteria != vlanPortData->p.criteria) {
                            VLAN_EVERB((VLAN_FUNCMSG1("criteria %d != %d\n"),
                                        vlan_port->criteria,
                                        vlanPortData->p.criteria));
                            vlanPortData = NULL;
                        }
                    } /* switch (vlan_port->criteria) */
                } else { /* if (BCM_GPORT_VLAN_PORT == this LPORT's type */
                    vlanPortData = NULL;
                } /* if (BCM_GPORT_VLAN_PORT == this LPORT's type */
                if (vlanPortData) {
                    VLAN_VVERB((VLAN_FUNCMSG1("looks like unit %d lport %08X"
                                              " (VLAN_GPORT %08X) matches\n"),
                                unit,
                                logicalPort,
                                vlanPortData->p.vlan_port_id));
                    result = BCM_E_NONE;
                    sal_memcpy(vlan_port,
                               &(vlanPortData->p),
                               sizeof(*vlan_port));
                    break;
                }
            } /* for (all logical ports) */
        } /* if (BCM_E_FAIL == result) */
    } else {
        /* I really don't know what the caller wanted! */
        result = BCM_E_PARAM;
    }

    if (BCM_E_NONE == result) {
        /* find out if we're on the home module */
        result = _bcm_fe2000_map_vlan_gport_targets(unit,
                                                    vlan_port->port,
                                                    &myModule,
                                                    &tgtModule,
                                                    &tgtPort,
                                                    NULL,
                                                    NULL);
    }

    if ((BCM_E_NONE == result) && (myModule == tgtModule)) {
        /*
         *  We found a port, but it's rather likely that some other module has
         *  mucked about with it, so the cache is unreliable.  Would be nice if
         *  the other modules would reflect the updates to us, but having them
         *  play with our tables is a Bad Thing, so just pretend to be nice...
         *
         *  This method could be problematic since we can not reconstruct all
         *  of the egress path settings from just reading the egress path;
         *  we're only going to resync those settings that can be rebuilt.
         */
        switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:

            fti = BCM_GPORT_VLAN_PORT_ID_GET(vlanPortData->p.vlan_port_id);
            fti = VLAN_VGPORT_ID_TO_FT_INDEX(unit, fti);

            result = soc_sbx_g2p3_ft_get(unit, fti, &p3fte);
            if (BCM_E_NONE != result) {
                VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:ft[%08X]: %d (%s)\n"),
                          unit, fti, result, _SHR_ERRMSG(result)));
            }

            if (BCM_E_NONE == result) {
                result = soc_sbx_g2p3_oi2e_get(unit,
                                               p3fte.oi - SBX_RAW_OHI_BASE,
                                               &p3oi2e);
                if (BCM_E_NONE != result) {
                    VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:oi2e[%08X]:"
                                            " %d (%s)\n"),
                              unit,
                              p3fte.oi - SBX_RAW_OHI_BASE,
                              result,
                              _SHR_ERRMSG(result)));
                }
            }
            if (BCM_E_NONE == result) {
                result = soc_sbx_g2p3_eteencap_get(unit,
                                                   p3oi2e.eteptr,
                                                   &p3eteEncap);
                if (BCM_E_NONE != result) {
                    VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:et[%08X]Encap:"
                                            " %d (%s)\n"),
                              unit,
                              p3oi2e.eteptr,
                              result,
                              _SHR_ERRMSG(result)));
                }
            }
            if (BCM_E_NONE == result) {
                result = soc_sbx_g2p3_etel2_get(unit,
                                                p3eteEncap.l2ete,
                                                &p3eteL2);
                if (BCM_E_NONE != result) {
                    VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:et[%08X]L2:"
                                            " %d (%s)\n"),
                              unit,
                              p3eteEncap.l2ete,
                              result,
                              _SHR_ERRMSG(result)));
                }
            }
            if (BCM_E_NONE == result) {
                vlanPortData->p.egress_vlan = p3eteL2.vid;
                vlanPortData->p.egress_inner_vlan = p3eteEncap.vid;
                vlanPortData->p.encap_id = ((0xFF000000 & vlanPortData->p.encap_id) |
                                            p3fte.oi);
            }
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            /* should never see this */
            result = BCM_E_INTERNAL;
        } /* switch (SOC_SBX_CONTROL(unit)->ucodetype) */
        if (BCM_E_NONE == result) {
            /* still okay, so update the user copy */
            sal_memcpy(vlan_port, &(vlanPortData->p), sizeof(*vlan_port));
        }
    } /* if (BCM_E_NONE == result) */

    /* release the lock */
    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,*) return %d (%s)\n"),
               unit,
               result,
               _SHR_ERRMSG(result)));

    return result;
}
#undef _VLAN_FUNC_NAME

#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_port_vlan_vector_internal"
STATIC int
_bcm_fe2000_vlan_port_vlan_vector_internal(int              unit,
                                           bcm_vlan_port_t *gport,
                                           int             *exit_modid,
                                           bcm_port_t      *phy_port,
                                           bcm_vlan_t      *vpn)
{
    int my_modid, result = BCM_E_NONE;
    int tid, idx;
    _bcm_fe2k_trunk_state_t *ts;

    *exit_modid = -1;

    BCM_IF_ERROR_RETURN(bcm_stk_my_modid_get(unit, &my_modid));
    BCM_IF_ERROR_RETURN(bcm_fe2000_vlan_port_find(unit, gport));

    if (!(gport->criteria == BCM_VLAN_PORT_MATCH_PORT ||
          gport->criteria == BCM_VLAN_PORT_MATCH_PORT_VLAN)) {
        VLAN_ERR(("unit %d: [%s] vlan gport 0x%x cannot be updated\n",
                  unit, FUNCTION_NAME(), gport->vlan_port_id));
        return BCM_E_PARAM;
    }

    if (BCM_GPORT_IS_TRUNK(gport->port)) {

        tid = BCM_GPORT_TRUNK_GET(gport->port);
        ts = &_vlan_state[unit]->trunk[tid];
        /* Assume faliure until a matching mod is found but ignore if trunk is empty */
	if (ts->tinfo.num_ports > 0){
	  result = BCM_E_PARAM;
	}

        for (idx=0; idx < ts->tinfo.num_ports; idx++) {
            if (ts->tinfo.tm[idx] == my_modid) {
                result = BCM_E_NONE;
                *phy_port = ts->tinfo.tp[idx];
                break;
            }
        }
        
    } else if (BCM_GPORT_IS_MODPORT(gport->port)) {
        *exit_modid = BCM_GPORT_MODPORT_MODID_GET(gport->port);
        *phy_port = BCM_GPORT_MODPORT_PORT_GET(gport->port);
    } else if (BCM_GPORT_IS_LOCAL(gport->port)) {
        *phy_port = BCM_GPORT_LOCAL_GET(gport->port);
    } else if (SOC_PORT_VALID(unit, gport->port)) {
        *phy_port = gport->port;
    } else {
        result = BCM_E_PORT;
    }

    if (BCM_FAILURE(result)) {
        VLAN_VERB((VLAN_FUNCMSG("(%d,*) return %d (%s)\n"),
               unit,
               result,
               _SHR_ERRMSG(result)));
        return result;
    }

    if (result == BCM_E_NONE) {
        *vpn = gport->vsi;
    }

    return result;
}
#undef _VLAN_FUNC_NAME


/*
 *   Function
 *      _bcm_fe2000_g2p3_vlan_port_vector_set
 *   Purpose
 *      Establish a logical port vector on g2p3
 *   Parameters
 *      (in) unit      = BCM device number
 *      (in) vlan_vec  = vid vector for logical port
 *      (in) vlanPort  = fully populated vlan port
 *      (in) phyPort   = physical port the vlanPort resides
 *      (in) vpn       = vpn/vsi for which the vlanPort is a member
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      vlan lock assumed to be taken; this routine allocates
 *    memory, beware of returning mid-routine
 */

#ifdef BCM_FE2000_P3_SUPPORT
#define _VLAN_FUNC_NAME FUNCTION_NAME()
int
_bcm_fe2000_g2p3_vlan_port_vlan_vector_set(int unit,
                                           bcm_vlan_vector_t vlan_vec,
                                           int my_modid,
                                           bcm_vlan_port_t *vlanPort,
                                           bcm_port_t phyPort, bcm_vlan_t vpn)
{
    bcm_vlan_t      vid;
    bcm_vlan_t      minVid = BCM_VLAN_MAX + 1;
    int             status = BCM_E_NONE;
    int             matchVlanStpState = 0;
    int             ingFilter = FALSE;     /* ingress filter enable */
    int             egrFilter = FALSE;     /* egress filter enable */
    int             matchVlanRemoved = FALSE;
    int             commit;
    soc_sbx_g2p3_pv2e_t          p3pv2e;
    soc_sbx_g2p3_pvv2e_t         p3pvv2e;
    soc_sbx_g2p3_p2appdata_t     p3p2appdata;     /* G2P3 p2appdata entry */
    uint32                       logicalPort = ~0;
    int                          pvv2eCommit = 0;
    uint32_t                     *fastLpi;
    uint32_t                     *fastVlan;
    uint32_t                     *fastStpstate;
    uint32_t                     *fastUntaggedStrip;
    uint32_t                     *fastStrip;
    int                          *fastStripSets;

    fastLpi = sal_alloc(sizeof(uint32_t) * (BCM_VLAN_MAX + 1),
                        "fastLpi");
    fastVlan = sal_alloc(sizeof(uint32_t) * (BCM_VLAN_MAX + 1),
                        "fastVlan");
    fastStpstate = sal_alloc(sizeof(uint32_t) * (BCM_VLAN_MAX + 1),
                        "fastStpstate");
    fastUntaggedStrip = sal_alloc(sizeof(uint32_t) * (BCM_VLAN_MAX + 1),
                        "fastUntaggedStrip");
    fastStrip = sal_alloc(sizeof(uint32_t) * (BCM_VLAN_MAX + 1),
                          "fastStrip");
    fastStripSets = sal_alloc(sizeof(uint32_t) * (BCM_VLAN_MAX + 1),
                              "fastStripSets");

    if (fastLpi == NULL || fastVlan == NULL ||
        fastStpstate == NULL || fastUntaggedStrip == NULL ||
        fastStrip == NULL || fastStripSets == NULL)
    {
        VLAN_ERR((VLAN_FUNCMSG1("unable to alloc %d bytes for"
                                " unit %d vlan temp data\n"),
                  sizeof(uint32_t) * (BCM_VLAN_MAX + 1),unit));
        if (fastLpi) {
            sal_free(fastLpi);
        }
        if (fastVlan) {
            sal_free(fastVlan);
        }
        if (fastStpstate) {
            sal_free (fastStpstate);
        }
        if (fastUntaggedStrip) {
            sal_free(fastUntaggedStrip);
        }
        if (fastStrip) {
            sal_free(fastStrip);
        }
        if (fastStripSets) {
            sal_free(fastStripSets);
        }
        return BCM_E_MEMORY;
    }
    sal_memset(fastStripSets, 0x00, sizeof(uint32_t) * (BCM_VLAN_MAX + 1));

    soc_sbx_g2p3_pv2e_t_init(&p3pv2e);
    status = SOC_SBX_G2P3_PV2E_GET(unit, phyPort, vlanPort->match_vlan,
                                   &p3pv2e);
    matchVlanStpState = p3pv2e.stpstate;

    if (BCM_FAILURE(status)) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:pv2e[%d,%03X]:"
                                " %d (%s)\n"),
                  unit, phyPort, vlanPort->match_vlan,
                  status, _SHR_ERRMSG(status)));
    }

    if (BCM_GPORT_VLAN_PORT_ID_GET(vlanPort->vlan_port_id) < 0) {
        status = BCM_E_PARAM;
    } else {
        /* we have to set the logical port for g2p3 */
        logicalPort = _vlan_state[unit]->gportInfo.lpid[
            BCM_GPORT_VLAN_PORT_ID_GET(vlanPort->vlan_port_id)];
    }

    /* find out if port is ingress/egress filtered */
    if (BCM_E_NONE == status) {
        status = soc_sbx_g2p3_p2appdata_get(unit,
                                            phyPort,
                                            &p3p2appdata);
        if (BCM_E_NONE == status) {
            egrFilter = (0 != p3p2appdata.efilteren);
            ingFilter = (0 != p3p2appdata.ifilteren);
        } else { /* if (SB_OK == rv) */
            VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:p2appdata[%d]:"
                                    " %d (%s)\n"),
                      unit,
                      phyPort,
                      status,
                      _SHR_ERRMSG(status)));
        } /* if (SB_OK == rv) */

        /*
         *  Now, if the port is ingress filtered, removal from the vector must
         *  set the target VSI to zero, otherwise, if the port is in the
         *  drop_tagged state, removal from the vector must set the target VSI
         *  to zero, otherwise, it must set the target VSI to the VID's VSI.
         *  Since we're already going to drop if ingress filtering is on for
         *  this port, it does not matter if we override that to drop in the
         *  case of drop_tagged mode.  It also does not matter if we do this
         *  after an error; it will be ignored later in that case.
         */
        if (BCM_PBMP_MEMBER(_vlan_state[unit]->tag_drop, phyPort)) {
            ingFilter = TRUE;
        }
    } /* if (BCM_E_NONE == result) */

    /* Always need to read the entry */
    if (BCM_SUCCESS(status)) {
        status = soc_sbx_g2p3_pv2e_lpi_fast_get(unit, 0, phyPort,
                                                0xFFF, phyPort,
                                                fastLpi, BCM_VLAN_MAX + 1);
        if (BCM_FAILURE(status)) {
            VLAN_ERR((VLAN_FUNCMSG1("failed to get pv2e[%d,*] lpi: %d %s\n"),
                      phyPort, status, bcm_errmsg(status)));
        }
    }

    if (BCM_SUCCESS(status)) {
        status = soc_sbx_g2p3_pv2e_vlan_fast_get(unit, 0, phyPort,
                                                 0xFFF, phyPort,
                                                 fastVlan, BCM_VLAN_MAX + 1);
        if (BCM_FAILURE(status)) {
            VLAN_ERR((VLAN_FUNCMSG1("failed to get pv2e[%d,*] vlan: %d %s\n"),
                      phyPort, status, bcm_errmsg(status)));
        }
    }

    if (BCM_SUCCESS(status)) {
        status = soc_sbx_g2p3_pv2e_stpstate_fast_get(unit, 0, phyPort,
                                                     0xFFF, phyPort,
                                                     fastStpstate,
                                                     BCM_VLAN_MAX + 1);
        if (BCM_FAILURE(status)) {
            VLAN_ERR((VLAN_FUNCMSG1("failed to get pv2e[%d,*] stp state: %d %s\n"),
                      phyPort, status, bcm_errmsg(status)));
        }
    }

    if (BCM_SUCCESS(status)) {
        status = soc_sbx_g2p3_pv2e_untagged_strip_fast_get(unit, 0, phyPort,
                                                           0xFFF, phyPort,
                                                           fastUntaggedStrip,
                                                           BCM_VLAN_MAX + 1);
        if (BCM_FAILURE(status)) {
            VLAN_ERR((VLAN_FUNCMSG1("failed to get pv2e[%d,*] untagged strip:"
                                    " %d %s\n"),
                      phyPort, status, bcm_errmsg(status)));
        }
    }

    /* update the VIDs on the port */
    for (vid = BCM_VLAN_MIN + 1;
         (vid < BCM_VLAN_MAX) && (BCM_E_NONE == status);
         vid++) {

        p3pv2e.vlan = fastVlan[vid];
        p3pv2e.lpi = fastLpi[vid];
        p3pv2e.stpstate = fastStpstate[vid];
        p3pv2e.untagged_strip = fastUntaggedStrip[vid];

        if (BCM_VLAN_VEC_GET(vlan_vec, vid)) {
            /* this VID is a member of the vector, set it up */
            if (vid < minVid) {
                minVid = vid;
            }
            VLAN_VVERB((VLAN_FUNCMSG1(" Adding vid 0x%x to gport\n"), vid));

            p3pv2e.vlan = vpn;
            p3pv2e.lpi = logicalPort;
            p3pv2e.stpstate = matchVlanStpState;

            status = _i_soc_sbx_g2p3_pv2e_set(unit, phyPort, vid, &p3pv2e);

            /* set epv2e[port,vid].strip based on egress untagged flag */
            fastStripSets[vid] = 1;
            fastStrip[vid] =
                !!(vlanPort->flags & BCM_VLAN_PORT_EGRESS_UNTAGGED);

            if (status == BCM_E_NONE) {
                int stripping =  !(vlanPort->flags &
                                   BCM_VLAN_PORT_INNER_VLAN_PRESERVE);

                /* set up the pvv2e entry to remove the tag this port is
                 * set to not preserve; ie strip... */
                if (stripping) {
                    soc_sbx_g2p3_pvv2e_t_init(&p3pvv2e);
                    p3pvv2e.lpi            = p3pv2e.lpi;
                    p3pvv2e.vlan           = p3pv2e.vlan;
                    p3pvv2e.stpstate       = p3pv2e.stpstate;
                    p3pvv2e.untagged_strip = p3pv2e.untagged_strip;
                    p3pvv2e.keeporstrip    = 1;

                    /* batch update */
                    status = soc_sbx_g2p3_pvv2e_renew (unit, 0xfff, vid,
                                                       phyPort, &p3pvv2e);

                    if (status == SOC_E_NOT_FOUND) {
                        /* batch add */
                        status = soc_sbx_g2p3_pvv2e_add (unit, 0xfff, vid,
                                                         phyPort, &p3pvv2e);
                        if (status == SOC_E_NONE) {
                            pvv2eCommit = 1;
                        }
                    } else if (BCM_FAILURE(status)) {
                        VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:"
                                                "pvv2e[%d,%03X,0xFFF]:"
                                                " %d (%s)\n"),
                                  unit, phyPort, vid, status,
                                      _SHR_ERRMSG(status)));
                    }
                } else {
                    /* no longer needed, delete it - batch remove */
                    status =  soc_sbx_g2p3_pvv2e_remove(unit, 0xfff,
                                                        vid, phyPort);
                    if (status == SOC_E_NONE) {
                        pvv2eCommit = 1;
                    }
                    status = BCM_E_NONE;

                }
            }

        } else { /* if (BCM_VLAN_VEC_GET(vlan_vec, vid)) */

            /* this VID IS not a member of the vector, but WAS it a member? */
            commit = (p3pv2e.vlan == vpn) && (p3pv2e.lpi == logicalPort);

            if (commit) {
                int ignore_status;

                VLAN_VVERB((VLAN_FUNCMSG1(" Removing vid 0x%x to gport\n"),
                            vid));

                if (vid == vlanPort->match_vlan) {
                    matchVlanRemoved = TRUE;
                }

                /* it was a member; remove it */
                if (ingFilter) {
                    p3pv2e.vlan = 0;
                } else {
                    p3pv2e.vlan = SBX_VSI_FROM_VID(vid);
                }

                p3pv2e.lpi = 0;
                status = _i_soc_sbx_g2p3_pv2e_set(unit, phyPort, vid,
                                                  &p3pv2e);

               /* clear epv2e[port,vid].strip */
                fastStripSets[vid] = 1;
                fastStrip[vid] = FALSE;

                /* unconditionally delete the pvv2e  */
                ignore_status =
                    soc_sbx_g2p3_pvv2e_remove(unit, 0xfff, vid, phyPort);
                if (ignore_status == SOC_E_NONE) {
                    pvv2eCommit = 1;
                }

                if (BCM_E_NONE != status) {
                    VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:pv2e[%d,%03X]:"
                                            " %d (%s)\n"),
                              unit,
                              phyPort,
                              vid,
                              status,
                              _SHR_ERRMSG(status)));
                    break;
                }

            } else {
                /* it was not a member; leave it alone */
                /*
                 *  This should not make any changes to a port,vid that neither
                 *  is nor was in the vector.  Safe to successfully do nothing.
                 */
                status = BCM_E_NONE;
            }
        } /* if (BCM_VLAN_VEC_GET(vlan_vec, vid)) */

        if (BCM_E_NONE != status) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to update unit %d port %d"
                                    " membership to VID %03X: %d (%s)\n"),
                      unit, phyPort, vid, status,
                      _SHR_ERRMSG(status)));
            break;
        }
    } /* for (vid = BCM_VLAN_MIN; vid < BCM_VLAN_MAX; vid++) */

    if (pvv2eCommit) {
        status = soc_sbx_g2p3_pvv2e_commit (unit, 0xffffffff);
        if (BCM_FAILURE(status)) {
            VLAN_ERR((VLAN_FUNCMSG1("failed to commit pvv2e changes on unit %d"
                                    ": %d (%s)\n"),
                      unit, status, _SHR_ERRMSG(status)));
        }
    }

    if (BCM_SUCCESS(status)) {
        status = soc_sbx_g2p3_epv2e_strip_fast_set(unit, 0, phyPort,
                                                   0xFFF, phyPort,
                                                   fastStripSets,
                                                   fastStrip,
                                                   BCM_VLAN_MAX + 1);
    }

    /* need to touch the untagged vlan on the port to update pvv2e based
     * on any new state for the vector
     */
    if (BCM_SUCCESS(status)) {
        bcm_vlan_t vid = _vlan_state[unit]->nvid[phyPort];
        status =
            _bcm_fe2000_port_untagged_vlan_touch(unit, phyPort, vid);
        if (BCM_FAILURE(status)) {
            VLAN_ERR((VLAN_FUNCMSG1(" update native vid on port %d, vid 0x%03x"
                                    " unit %d: %d (%s)\n"),
                      phyPort, vid, unit, status, _SHR_ERRMSG(status)));
        }
    }

    if (BCM_SUCCESS(status)) {
        if (matchVlanRemoved && minVid < BCM_VLAN_MAX) {
            _bcm_fe2k_vlan_gport_t* vlanPortData;
            vlanPortData =
                (_bcm_fe2k_vlan_gport_t*)(SBX_LPORT_DATAPTR(unit, logicalPort));
            VLAN_VERB((VLAN_FUNCMSG(" gport 0x%08x match_vlan 0x%03x removed. "
                                    " replaced with min vid 0x%03x\n"),
                       vlanPort->vlan_port_id, vlanPortData->p.match_vlan,
                       minVid));
            vlanPortData->p.match_vlan = minVid;

        }
    }

    if (fastLpi) {
        sal_free(fastLpi);
    }
    if (fastVlan) {
        sal_free(fastVlan);
    }
    if (fastStpstate) {
        sal_free (fastStpstate);
    }
    if (fastUntaggedStrip) {
        sal_free(fastUntaggedStrip);
    }
    if (fastStrip) {
        sal_free(fastStrip);
    }
    if (fastStripSets) {
        sal_free(fastStripSets);
    }

    return status;
}
#undef _VLAN_FUNC_NAME
#endif /* BCM_FE2000_P3_SUPPORT */


/*
 *   Function
 *      _bcm_fe2000_vlan_port_vector_set
 *   Purpose
 *      Establish a logical port vector on g2p3
 *   Parameters
 *      (in) unit      = BCM device number
 *      (in) gport     = gport to receive vector settings
 *      (in) vlan_vec  = vid vector for logical port
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */

#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_port_vlan_vector_set"
int
_bcm_fe2000_vlan_port_vlan_vector_set(int unit,
                                      bcm_gport_t gport,
                                      bcm_vlan_vector_t vlan_vec)
{

    int                       rv = 0, idx, trunk_id;
    bcm_vlan_port_t           vlan_port;
    int                       my_modid, exit_modid;
    bcm_port_t                phy_port;
    bcm_vlan_t                vpn;
    bcm_trunk_add_info_t      trunk_info;

    rv = BCM_E_NONE;
    sal_memset(&vlan_port, 0, sizeof(vlan_port));
    vlan_port.vlan_port_id = gport;
    vlan_port.criteria = BCM_VLAN_PORT_MATCH_NONE;

    BCM_IF_ERROR_RETURN(bcm_stk_my_modid_get(unit, &my_modid));

    BCM_IF_ERROR_RETURN
       (_bcm_fe2000_vlan_port_vlan_vector_internal(unit, &vlan_port,
                                                   &exit_modid, &phy_port,
                                                   &vpn));

    if ((exit_modid != -1) && (my_modid != exit_modid)) {
        VLAN_ERR(("unit %d: [%s] vlan gport 0x%x of a remote module id %d"
                  " cannot be updated\n",
                  unit, FUNCTION_NAME(), gport, exit_modid));
        return BCM_E_PARAM;
    }

    bcm_trunk_add_info_t_init(&trunk_info);

    if (BCM_GPORT_IS_TRUNK(vlan_port.port)) {

        trunk_id = BCM_GPORT_TRUNK_GET(vlan_port.port);
        rv = bcm_trunk_get(unit, trunk_id, &trunk_info);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("Failed to get trunk %d info: %s\n"),
                      trunk_id, bcm_errmsg(rv)));
            return rv;
        }

    } else {
        trunk_info.num_ports = 1;
        trunk_info.tp[0] = phy_port;
        trunk_info.tm[0] = my_modid;
    }

    VLAN_LOCK_TAKE;
    for (idx=0; idx < trunk_info.num_ports; idx++) {
        rv = _bcm_fe2000_g2p3_vlan_port_vlan_vector_set(unit, vlan_vec,
                                                        my_modid, &vlan_port,
                                                        trunk_info.tp[idx],
                                                        vpn);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("Failed to to set vlan vector on "
                                    "gport 0x%08x physical port %d: %s\n"),
                      gport, trunk_info.tp[idx], bcm_errmsg(rv)));
            break;
        }
    }
    VLAN_LOCK_RELEASE;

    return rv;
}
#undef _VLAN_FUNC_NAME

#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_port_vlan_vector_get"
int
_bcm_fe2000_vlan_port_vlan_vector_get(int unit,
                                      bcm_gport_t gport,
                                      bcm_vlan_vector_t vlan_vec)
{
    bcm_vlan_port_t vlan_port;
    bcm_vlan_t      vid, vpn;
    int             status = BCM_E_NONE;
    bcm_port_t      phy_port;
    int             exit_modid, my_modid;
#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_pv2e_t          p3pv2e;
    uint32                       logicalPort = ~0;
#endif /* BCM_FE2000_P3_SUPPORT */
    int member = FALSE;

    sal_memset(&vlan_port, 0, sizeof(vlan_port));
    vlan_port.vlan_port_id = gport;
    vlan_port.criteria = BCM_VLAN_PORT_MATCH_NONE;

    BCM_IF_ERROR_RETURN(bcm_stk_my_modid_get(unit, &my_modid));

    BCM_IF_ERROR_RETURN
       (_bcm_fe2000_vlan_port_vlan_vector_internal(unit,
                                                   &vlan_port,
                                                   &exit_modid,
                                                   &phy_port,
                                                   &vpn));

    if (exit_modid != -1 && my_modid != exit_modid) {
        VLAN_ERR(("unit %d: [%s] vlan vector for gport 0x%x of a"
                  " remote module id %d cannot be retrieved\n",
                  unit, FUNCTION_NAME(), gport, exit_modid));
        return BCM_E_PARAM;
    }

    BCM_VLAN_VEC_ZERO(vlan_vec);

    VLAN_LOCK_TAKE;

#ifdef BCM_FE2000_P3_SUPPORT
    if (SOC_IS_SBX_G2P3(unit)) {
        /* we have to check the logical port for g2p3 */
        logicalPort = _vlan_state[unit]->gportInfo.lpid[BCM_GPORT_VLAN_PORT_ID_GET(gport)];
    }
#endif /* def BCM_FE2000_P3_SUPPORT */

    for (vid = BCM_VLAN_MIN + 1; vid < BCM_VLAN_MAX; vid++) {
        /* get the current state of this VID */
        switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            soc_sbx_g2p3_pv2e_t_init(&p3pv2e);
            status = SOC_SBX_G2P3_PV2E_GET(unit, phy_port, vid, &p3pv2e);
            member = (p3pv2e.vlan == vpn) && (p3pv2e.lpi == logicalPort);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            status = BCM_E_UNAVAIL;
        } /* switch (microcode) */
        if (BCM_E_NONE != status) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:pv2e[%d,%03X]:"
                                    " %d (%s)\n"),
                      unit,
                      phy_port,
                      vid,
                      status,
                      _SHR_ERRMSG(status)));
            break;
        }
        /* if the VID is a member, mark it so */
        if (member) {
            BCM_VLAN_VEC_SET(vlan_vec, vid);
        }
    }

    VLAN_LOCK_RELEASE;

    return status;
}
#undef _VLAN_FUNC_NAME

#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_port_stp_set_get"
int
_bcm_fe2000_vlan_port_stp_set_get(int unit,
                                  bcm_vlan_t vid,
                                  bcm_gport_t gport,
                                  int *stp_state,
                                  int set_or_get)
{
    bcm_vlan_port_t vlan_port;
    bcm_port_t      phy_port;
    bcm_vlan_t      vsi;
    bcm_module_t    myModId = ~0;
    bcm_module_t    tgtModId = -1;

    sal_memset(&vlan_port, 0, sizeof(vlan_port));
    vlan_port.vlan_port_id = gport;
    vlan_port.criteria = BCM_VLAN_PORT_MATCH_NONE;
    BCM_IF_ERROR_RETURN(bcm_fe2000_vlan_port_find(unit, &vlan_port));

    
    BCM_IF_ERROR_RETURN(_bcm_fe2000_map_vlan_gport_targets(unit,
                                                           gport,
                                                           &myModId,
                                                           &tgtModId,
                                                           &phy_port,
                                                           NULL,
                                                           NULL));
    if (myModId != tgtModId) {
        VLAN_ERR((VLAN_FUNCMSG1("VLAN GPORT %08X home module %d is not local"
                                " module %d; must manipulate STP state on the"
                                " home module\n"),
                  gport,
                  tgtModId,
                  myModId));
        return BCM_E_PARAM;
    }

    switch(vlan_port.criteria) {
    case BCM_VLAN_PORT_MATCH_PORT:
        BCM_IF_ERROR_RETURN(bcm_port_untagged_vlan_get(unit, phy_port, &vsi));

        if (SOC_IS_SBX_G2P3(unit)) {
#ifdef BCM_FE2000_P3_SUPPORT
            if (set_or_get) { /* True = SET */
                BCM_IF_ERROR_RETURN
                   (_bcm_fe2000_g2p3_stg_vid_stp_set(unit, vsi,
                                                 phy_port, *stp_state));
            } else { /* False = GET */
                BCM_IF_ERROR_RETURN
                   (_bcm_fe2000_g2p3_stg_vid_stp_get(unit, vsi,
                                                 phy_port, stp_state));
            }
#endif /* BCM_FE2000_P3_SUPPORT */
        }
        break;
    case BCM_VLAN_PORT_MATCH_PORT_VLAN:
    {
        bcm_vlan_t vecVid;
        bcm_vlan_vector_t vlanVector;

        BCM_VLAN_VEC_ZERO(vlanVector);
        _bcm_fe2000_vlan_port_vlan_vector_get(unit,
                                              vlan_port.vlan_port_id,
                                              vlanVector);

        for (vecVid = BCM_VLAN_MIN; vecVid < BCM_VLAN_MAX; vecVid++) {
            if (!BCM_VLAN_VEC_GET(vlanVector, vecVid)) {
                continue;
            }

            if (SOC_IS_SBX_G2P3(unit)) {
#ifdef BCM_FE2000_P3_SUPPORT
                if (set_or_get) { /* True = SET */
                    BCM_IF_ERROR_RETURN
                        (_bcm_fe2000_g2p3_stg_vid_stp_set(unit, vecVid,
                                                          phy_port,
                                                          *stp_state));
                } else { /* False = GET */
                    BCM_IF_ERROR_RETURN
                        (_bcm_fe2000_g2p3_stg_vid_stp_get(unit, vecVid,
                                                          phy_port,
                                                          stp_state));
                }
#endif /* BCM_FE2000_P3_SUPPORT */
            }
        }

        break;
    }
    case BCM_VLAN_PORT_MATCH_PORT_VLAN_STACKED:
        if (SOC_IS_SBX_G2P3(unit)) {
#ifdef BCM_FE2000_P3_SUPPORT
            BCM_IF_ERROR_RETURN
               (_bcm_fe2000_g2p3_stg_stacked_vid_stp_set_get(unit,
                                                 vlan_port.match_vlan,
                                                 vlan_port.match_inner_vlan,
                                                 phy_port,
                                                 stp_state,
                                                 set_or_get));
#endif /* BCM_FE2000_P3_SUPPORT */
        }
        break;
    default:
        return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_port_remove
 *   Purpose
 *      Remove a port from a VID in hardware
 *   Parameters
 *      (in) int unit = unit number which is to be affected
 *      (in) bcm_vlan_t vid = vid which is to be affected
 *      (in) bcm_port_t port = port number to remove
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      Assumes unit lock is already taken.
 *      Assumes all parameters are already verified.
 *      Informs the trunk code of the remove if the remove is successful.
 *      Loses any translation on this port+VID (both ingress and egress).
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_port_remove"
static int
_bcm_fe2000_vlan_port_remove(const int unit,
                             const bcm_vlan_t vid,
                             const bcm_port_t port)
{
#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_pv2e_t   p3pVid2Etc;            /* G2P3 pVid2Etc entry */
    soc_sbx_g2p3_epv2e_t  p3egrPVid2Etc;         /* G2P3 egrPVid2Etc entry */
    soc_sbx_g2p3_evp2e_t  p3egrVlanPort2Etc;     /* G2P3 egrVlanPort2Etc ent */
    soc_sbx_g2p3_pv2appdata_t  p3pv2appdata;     /* G2P3 pv2appdata entry */
    soc_sbx_g2p3_p2appdata_t  p3p2appdata;     /* G2P3 p2appdata entry */
    soc_sbx_g2p3_oi2e_t   p3oi2e;                /* G2P3 oi2e entry */
#endif /* def BCM_FE2000_P3_SUPPORT */
    int                   result;                /* local result code */
    int                   ingFilter = FALSE;     /* ingress filter enable */
    int                   egrFilter = FALSE;     /* egress filter enable */
    unsigned int          vidNew;                /* new VID number */
    unsigned int          eteOld = ~0;           /* original ET entry index */
#if 0 
    bcm_vlan_t            vidOld;                /* original untagged VID */
    int                   auxres;                /* spare result code */
#endif 

    if (_vlan_state[unit] && (!(_vlan_state[unit]->init))) {
        /* This is too verbose during init */
        VLAN_EVERB((VLAN_FUNCMSG("(%d,%04X,%d) enter\n"),
                    unit,
                    vid,
                    port));
    }

    /* optimistically assume we'll be successful */
    result = BCM_E_NONE;

    if (!SOC_SBX_PORT_ADDRESSABLE(unit, port)) {
        VLAN_ERR((VLAN_FUNCMSG1("port %d is not valid on unit %d\n"),
                  port,
                  unit));
        result = BCM_E_PARAM;
    }

    /* find out if port is ingress/egress filtered */
    if (BCM_E_NONE == result) {
#ifdef BCM_FE2000_P3_SUPPORT
        if (SOC_IS_SBX_G2P3(unit)) {
            result = soc_sbx_g2p3_p2appdata_get(unit, port, &p3p2appdata);
            if (BCM_E_NONE == result) {
                egrFilter = (0 != p3p2appdata.efilteren);
                ingFilter = (0 != p3p2appdata.ifilteren);
            } else { /* if (SB_OK == rv) */
                VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:p2appdata[%d]:"
                                        " %d (%s)\n"),
                          unit,
                          port,
                          result,
                          _SHR_ERRMSG(result)));
            } /* if (SB_OK == rv) */
        } else
#endif /* def BCM_FE2000_P3_SUPPORT */
        {
            /* unknown or disabled unit type */
            VLAN_ERR((VLAN_FUNCMSG1("unsupported unit %d type\n"),
                      unit));
            result = VLAN_UNKNOWN_TYPE;
        }
    } /* if (BCM_E_NONE == result) */

    /* set up for ingress filtering on the to-be-removed VLAN for this port */
    if (BCM_E_NONE == result) {
        if (ingFilter) {
            /* ingress filtering enabled; toss unwanted tag */
            vidNew = 0;
        } else {
            /* ingress filtering disabled; check drop tagged state */
            if (BCM_PBMP_MEMBER(_vlan_state[unit]->tag_drop, port)) {
                /* don't allow tagged ingress in tag drop state */
                vidNew = 0;
            } else {
                /* not in drop tagged; don't filter unexpected tag */
                vidNew = SBX_VSI_FROM_VID(vid);
            }
        }
#ifdef BCM_FE2000_P3_SUPPORT
        if (SOC_IS_SBX_G2P3(unit)) {
            /* update pv2e for this vlan,port */
            if (_vlan_state[unit]->init) {
                /* initialising; start with clear entries */
                soc_sbx_g2p3_pv2e_t_init(&p3pVid2Etc);
                soc_sbx_g2p3_pv2appdata_t_init(&p3pv2appdata);
                result = BCM_E_NONE;
            } else {
                /* get pv2e for this vlan,port */
                result = SOC_SBX_G2P3_PV2E_GET(unit,
                                               port,
                                               vid,
                                               &p3pVid2Etc);
                if (BCM_E_NONE == result) {
                    /* get pv2appdata for this vlan,port */
                    result = SOC_SBX_G2P3_PV2APPDATA_GET(unit,
                                                         port,
                                                         vid,
                                                         &p3pv2appdata);
                    if (BCM_E_NONE != result) {
                        VLAN_ERR((VLAN_FUNCMSG1("unable to read"
                                                " %d:pv2appdata[%d,%03X]:"
                                                " %d (%s)\n"),
                                  unit,
                                  port,
                                  vid,
                                  result,
                                  _SHR_ERRMSG(result)));
                    }
                } else {
                    VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:pv2e[%d,%03X]:"
                                            " %d (%s)\n"),
                              unit,
                              port,
                              vid,
                              result,
                              _SHR_ERRMSG(result)));
                }
            }
            /* set new values */
            if (BCM_E_NONE == result) {
                p3pVid2Etc.vlan = vidNew;
                p3pv2appdata.member = FALSE;
                /*
                 * Free the logical port if it is dynamically
                 * allocated, and we are not initializing
                 */
                if (p3pVid2Etc.lpi != 0 &&
                    p3pVid2Etc.lpi > SBX_MAX_PORTS &&
                    !_vlan_state[unit]->init) {

                    result = _sbx_gu2_resource_free(unit,
                                        SBX_GU2K_USR_RES_LPORT,
                                        1,
                                        &p3pVid2Etc.lpi,
                                        0);
                    if (result == BCM_E_NONE) {
                        p3pVid2Etc.lpi = 0;
                    }
                }

                if (BCM_E_NONE == result) {
                    result = _i_soc_sbx_g2p3_pv2e_set(unit,
                                                      port,
                                                      vid,
                                                      &p3pVid2Etc);
                }

                if (BCM_E_NONE == result) {
                    result = SOC_SBX_G2P3_PV2APPDATA_SET(unit,
                                                         port,
                                                         vid,
                                                         &p3pv2appdata);
                    if (BCM_E_NONE != result) {
                        VLAN_ERR((VLAN_FUNCMSG1("unable to write"
                                                " %d:pv2appdata[%d,%03X]:"
                                                " %d (%s)\n"),
                                  unit,
                                  port,
                                  vid,
                                  result,
                                  _SHR_ERRMSG(result)));
                    }
                } else {
                    VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:pv2e[%d,%03X]:"
                                            " %d (%s)\n"),
                              unit,
                              port,
                              vid,
                              result,
                              _SHR_ERRMSG(result)));
                }
            }
        } /* if (SOC_IS_SBX_G2P3(unit)) */
#endif /* def BCM_FE2000_P3_SUPPORT */
    } /* if (BCM_E_NONE == result) */

    /* set the ET entry index for this VLAN on this port for tagging */
    if (BCM_E_NONE == result) {
#ifdef BCM_FE2000_P3_SUPPORT
        if (SOC_IS_SBX_G2P3(unit)) {
            if (_vlan_state[unit]->init) {
                soc_sbx_g2p3_evp2e_t_init(&p3egrVlanPort2Etc);
                result = BCM_E_NONE;
            } else {
                result = soc_sbx_g2p3_evp2e_get(unit,
                                                vid,
                                                port,
                                                &p3egrVlanPort2Etc);
            }
            if (BCM_E_NONE == result) {
                eteOld = p3egrVlanPort2Etc.eteptr;
                /* point vid=0 entries to the RAW ETE */
                if ( vid != 0) {
                    p3egrVlanPort2Etc.eteptr = SOC_SBX_PORT_ETE(unit, port);
                }else{
                    result = soc_sbx_g2p3_oi2e_get(unit,
                                                   0, /* SBX_RAW_OHI_BASE - SBX_RAW_OHI_BASE */
                                                   &p3oi2e);
                    if (BCM_E_NONE != result) {
                        VLAN_ERR((VLAN_FUNCMSG1("unable to access %d:oie[%d]:"
                                        " %d (%s)\n"),
                                unit,
                                SBX_RAW_OHI_BASE,
                                result,
                                _SHR_ERRMSG(result)));

                    }
                    p3egrVlanPort2Etc.eteptr = p3oi2e.eteptr;
                }
                if (BCM_E_NONE == result) {
                    result = soc_sbx_g2p3_evp2e_set(unit,
                                                    vid,
                                                    port,
                                                    &p3egrVlanPort2Etc);
                }
            }
            if (BCM_E_NONE != result) {
                VLAN_ERR((VLAN_FUNCMSG1("unable to access %d:evp2e[%d,%03X]:"
                                        " %d (%s)\n"),
                          unit,
                          port,
                          vid,
                          result,
                          _SHR_ERRMSG(result)));
            }
        } /* if (SOC_IS_SBX_G2P3(unit)) */
#endif /* def BCM_FE2000_P3_SUPPORT */
    } /* if (BCM_E_NONE == result) */

    /* get rid of any unwanted ET entries */
    if (BCM_E_NONE == result) {
        result = _bcm_fe2000_vlan_free_ete(unit, eteOld);
        if (BCM_E_NONE != result) {
            VLAN_ERR((VLAN_FUNCMSG1("_bcm_fe2000_vlan_free_ete(%d,%08X)"
                                    " returned %d (%s)\n"),
                      unit,
                      eteOld,
                      result,
                      _SHR_ERRMSG(result)));
        } /* if (BCM_E_NONE != result) */
    } /* if (BCM_E_NONE == result) */

    /* set the port to drop this VLAN on egress if filtering egress */
    if (BCM_E_NONE == result) {
#ifdef BCM_FE2000_P3_SUPPORT
        if (SOC_IS_SBX_G2P3(unit)) {
            if (_vlan_state[unit]->init) {
                soc_sbx_g2p3_epv2e_t_init(&p3egrPVid2Etc);
                result = BCM_E_NONE;
            } else {
                result = SOC_SBX_G2P3_EPV2E_GET(unit,
                                                port,
                                                vid,
                                                &p3egrPVid2Etc);
            }
            if (BCM_E_NONE == result) {
                p3egrPVid2Etc.drop  = egrFilter;
                p3egrPVid2Etc.strip = 0;
                result = SOC_SBX_G2P3_EPV2E_SET(unit,
                                                port,
                                                vid,
                                                &p3egrPVid2Etc);
                if (BCM_E_NONE != result) {
                    VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:epv2e[%d,%03X]:"
                                            " %d (%s)\n"),
                              unit,
                              port,
                              vid,
                              result,
                              _SHR_ERRMSG(result)));
                }
            } else {
                VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:epv2e[%d,%03X]:"
                                        " %d (%s)\n"),
                          unit,
                          port,
                          vid,
                          result,
                          _SHR_ERRMSG(result)));
            }
        }
#endif /* def BCM_FE2000_P3_SUPPORT */
    } /* if (BCM_E_NONE == result) */

#if 0 
    /* If this was port's untagged ingress VID, make that default instead */
    if (BCM_E_NONE == result) {
        /* check untagged VID */
        auxres = bcm_fe2000_port_untagged_vlan_get(unit, port, &vidOld);
        if ((BCM_E_NONE == auxres) && (vidOld == vid)) {
            /* untagged VID is this one; make it default */
            auxres = bcm_fe2000_port_untagged_vlan_set(unit,
                                                       port,
                                                       _vlan_state[unit]->
                                                       default_vid);
            if (BCM_E_NONE != auxres) {
                /*
                 *  Although we don't care when completely unable to touch the
                 *  table, this table has no business being read-only, so
                 *  complain if we can read but not write.
                 */
                result = auxres;
                VLAN_ERR((VLAN_FUNCMSG1("unable to set unit %d port %d default"
                                        " VID from %04X to %04X: %d (%s)\n"),
                          unit,
                          port,
                          vidOld,
                          vid,
                          result,
                          _SHR_ERRMSG(result)));
            }
        }
        /*
         *  Actually, we're going to ignore errors fetching the default VID,
         *  other than to not reset it.  There are inconsistencies in the ports
         *  that show up in the port2Etc and egrPort2Etc tables between
         *  microcode versions, and it's too much bother to handle them here,
         *  in a function that gets called as much as this one does.
         */
    } /* if (BCM_E_NONE == result) */
#endif 

#if 0 

    /* inform STP code that we're removing this port */
    if (BCM_E_NONE == result) {
        /* the port was removed; tell the STP code */
        result = _bcm_fe2000_stg_vlan_port_remove(unit, vid, port);
        if ((BCM_E_NONE != result) && (BCM_E_INIT != result)) {
            VLAN_ERR(("%s: _bcm_fe2000_stg_vlan_port_remove(%d,%03X,%d) "
                      "returned %d (%s)\n",
                      FUNCTION_NAME(),
                      unit,
                      vid,
                      port,
                      result,
                      _SHR_ERRMSG(result)));
        } else if (BCM_E_INIT == result) {
            /* ignore BCM_E_INIT from STG here */
            result = BCM_E_NONE;
        }
    } /* if (BCM_E_NONE == result) */
#endif 

    if (_vlan_state[unit] && (!(_vlan_state[unit]->init))) {
        /* This is too verbose during init */
        VLAN_EVERB((VLAN_FUNCMSG("(%d,%04X,%d) return %d (%s)\n"),
                    unit,
                    vid,
                    port,
                    result,
                    _SHR_ERRMSG(result)));
    }

    /* all done; return the result */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_port_change
 *   Purpose
 *      Change whether a port is a member or not of a VLAN
 *   Parameters
 *      (in) int unit = unit number which is to be affected
 *      (in) bcm_vlan_t vid = vid which is to be affected
 *      (in) bcm_port_t port = port number to add
 *      (in) unsigned int untagged = TRUE if untagged
 *      (in) unsigned int remove = TRUE to remove port from VLAN
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      Assumes all parameters are already verified.
 *      Assumes the VLAN exists.
 *      Does not update explicit default VID membership bitmap.
 *      Loses translation on port,VID if removing port; otherwise preserves
 *      (but this means that it can't changed tagged state if the port has
 *      egress translation).
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_port_change"
int
_bcm_fe2000_vlan_port_change(const int unit,
                             const bcm_vlan_t vid,
                             const bcm_port_t port,
                             const unsigned int untagged,
                             const unsigned int remove)
{
    int                   result;           /* working result */

    VLAN_VVERB((VLAN_FUNCMSG("(%d,%04X,%d,%s,%s) enter\n"),
                unit,
                vid,
                port,
                untagged?"TRUE":"FALSE",
                remove?"TRUE":"FALSE"));

    /* Check for proper initialisation */
    if ((!(_primary_lock)) || (!(_vlan_state[unit]))) {
        VLAN_ERR((VLAN_FUNCMSG1("unit %d not initialised\n"), unit));
        return BCM_E_INIT;
    }

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    if (remove) {
        result = _bcm_fe2000_vlan_port_remove(unit, vid, port);
    } else {
        result = _bcm_fe2000_vlan_port_add(unit, vid, port, untagged);
    }

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_VVERB((VLAN_FUNCMSG("(%d,%04X,%d,%s,%s) return %d (%s)\n"),
                unit,
                vid,
                port,
                untagged?"TRUE":"FALSE",
                remove?"TRUE":"FALSE",
                result,
                _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_add
 *   Purpose
 *      Create appropriate entries for provided VID.
 *   Parameters
 *      (in) int unit = unit number on which to create the VID
 *      (in) bcm_vlan_t = VID to create
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      This will change any ports whose untagged ingress VID is the destroyed
 *      VID so their untagged ingress VID is the default VID.
 *      This will not destroy the default VID.
 *      This disables frame forwarding on the specified VID, at the forwarding
 *      table and at the port,VID table and the MVT.
 *      Assumes input parameters have been checked for validity.
 *      Assumes the lock has already been taken (DO NOT CALL EXTERNALLY).
 *      Makes external calls; these should not call back into this module.
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_add"
static int
_bcm_fe2000_vlan_add(int unit,
                     bcm_vlan_t vid)
{
#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_v2e_t     p3vlan2Etc;         /* G2P3 vlan2Etc entry */
#endif /* def BCM_FE2000_P3_SUPPORT */
    _bcm_sbx_vlan_ft_t     ft;                 /* working FT (flood) entry */
    int                    result;             /* local result code */
    int                    tmpres;             /* called function's result */
    bcm_port_t             pIndex;             /* working port index */
    bcm_stg_t              stg;                /* working STG */
    int                    vlanValid = FALSE;  /* whether VID already exists */
    int                    igmpSnoop = FALSE;  /* default IGMP snoop state */

    VLAN_EVERB((VLAN_FUNCMSG("(%d,%04X) enter\n"),
                unit,
                vid));

    /* be optimistic */
    result = BCM_E_NONE;

    /* make sure the VID does not already exist */
    result = _bcm_fe2000_vlan_check(unit, vid, &vlanValid, &ft);
    if (BCM_E_NONE == result) {
        if ((SOC_WARM_BOOT(unit) == FALSE) && vlanValid) {
            result = BCM_E_EXISTS;
            VLAN_ERR((VLAN_FUNCMSG1("VLAN %04X already exists on unit %d\n"),
                      vid,
                      unit));
        }
    } /* if (BCM_E_NONE == result) */

    /* new VLANs inherit IGMP snoop setting from default's current setting */
    if (BCM_E_NONE == result) {
#ifdef BCM_FE2000_P3_SUPPORT
        if (SOC_IS_SBX_G2P3(unit)) {
            result = soc_sbx_g2p3_v2e_get(unit,
                                          SBX_VSI_FROM_VID(_vlan_state[unit]->
                                                           default_vid),
                                          &p3vlan2Etc);
            if (BCM_E_NONE == result) {
                igmpSnoop = (0 != p3vlan2Etc.igmp);
            } else {
                VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:v2e[%04X]:"
                                        " %d (%s)\n"),
                          unit,
                          SBX_VSI_FROM_VID(_vlan_state[unit]->default_vid),
                          result,
                          _SHR_ERRMSG(result)));
            }
        } /* if (SOC_IS_SBX_G2P3(unit)) */
#endif /* def BCM_FE2000_P3_SUPPORT */
    } /* if (BCM_E_NONE == result) */

    /* enable the VID */
    if (BCM_E_NONE == result) {
#ifdef BCM_FE2000_P3_SUPPORT
        if (SOC_IS_SBX_G2P3(unit)) {
            uint32 ftidx = 0, unknown_mc_ft_offset = 0;

            ft.p3ft.excidx = 0;
            ft.p3ft.qid = SBX_MC_QID_BASE;
            ft.p3ft.mc = 1;
            
            ft.p3ft.rridx = 0; 
            BCM_IF_ERROR_RETURN(soc_sbx_g2p3_vlan_ft_base_get(unit, &ftidx));
            ftidx += SBX_VSI_FROM_VID(vid);
            p3vlan2Etc.dontlearn = FALSE;
            p3vlan2Etc.igmp = igmpSnoop;
            p3vlan2Etc.v4route = TRUE;
            p3vlan2Etc.v6route = TRUE;
            result = soc_sbx_g2p3_ft_set(unit,
                                         ftidx,
                                         &(ft.p3ft));

            /* populate the unknown mc group for this VID/VSI */
            if (BCM_E_NONE == result) {
                result =  soc_sbx_g2p3_mc_ft_offset_get(unit, &unknown_mc_ft_offset);
                if (BCM_E_NONE == result) {
                ftidx += unknown_mc_ft_offset;
                result = soc_sbx_g2p3_ft_set(unit,
                                             ftidx,
                                             &(ft.p3ft));
            }
            }

            if (BCM_E_NONE == result) {
                result = soc_sbx_g2p3_v2e_set(unit,
                                              SBX_VSI_FROM_VID(vid),
                                              &p3vlan2Etc);
                if (BCM_E_NONE != result) {
                    VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:v2e[%04X]:"
                                            " %d (%s)\n"),
                              unit,
                              SBX_VSI_FROM_VID(vid),
                              result,
                              _SHR_ERRMSG(result)));
                }
            } else {
                VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:ft[%08X]:"
                                        " %d (%s)\n"),
                          unit,
                          SBX_VSI_FROM_VID(vid),
                          result,
                          _SHR_ERRMSG(result)));
            }
        } /* if (SOC_IS_SBX_G2P3(unit)) */
#endif /* def BCM_FE2000_P3_SUPPORT */
    } /* if (BCM_E_NONE == result) */

    /* assign the VLAN to the default STG */
    if (BCM_E_NONE == result) {
        /* Additional work if the VID was inserted correctly */
        /* get the default STG */
        result = bcm_fe2000_stg_default_get(unit, &stg);
        if (BCM_E_NONE == result) {
            /* We got the default STG; force this VID into it */
            result = bcm_fe2000_stg_vlan_add(unit, stg, vid);
            if (BCM_E_NONE != result) {
                VLAN_ERR((VLAN_FUNCMSG1("bcm_fe2000_stg_vlan_add(%d,%d,%04X)"
                                        " returned %d (%s)\n"),
                          unit,
                          stg,
                          BCM_VLAN_DEFAULT,
                          result,
                          _SHR_ERRMSG(result)));
            }
        } else { /* if (BCM_E_NONE == result) */
            VLAN_ERR((VLAN_FUNCMSG1("bcm_fe2000_stg_default_get(%d,*)"
                                    " returned %d (%s)\n"),
                      unit,
                      result,
                      _SHR_ERRMSG(result)));
        } /* if (BCM_E_NONE == result) */
    } /* if (BCM_E_NONE == result) */

    /* set up MVT entry and make sure only HiGig ports are members of VLAN */
    if (BCM_E_NONE == result) {
        PBMP_ALL_ITER(unit, pIndex) {
            /* handle all ports; don't break (but do keep last error) */
            /* make sure no other ports included */
            tmpres = _bcm_fe2000_vlan_port_remove(unit,
                                                  vid,
                                                  pIndex);
            if (BCM_E_NONE != tmpres) {
                VLAN_ERR((VLAN_FUNCMSG1("_bcm_fe2000_vlan_port_remove/add"
                                        "(%d,%04X,%d[,FALSE])"
                                        " returned %d (%s)\n"),
                          unit,
                          vid,
                          pIndex,
                          tmpres,
                          _SHR_ERRMSG(tmpres)));
                result = tmpres;
            }
        } /* PBMP_ALL_ITER(unit, pIndex) */
    } /* if (BCM_E_NONE == result) */

    VLAN_EVERB((VLAN_FUNCMSG("(%d,%04X) return %d (%s)\n"),
                unit,
                vid,
                result,
                _SHR_ERRMSG(result)));

    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_remove
 *   Purpose
 *      Destroy appropriate entries for provided VID.
 *   Parameters
 *      (in) int unit = unit number from which to remove the VID
 *      (in) bcm_vlan_t = VID to remove
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      This will change any ports whose untagged ingress VID is the destroyed
 *      VID so their untagged ingress VID is the default VID.
 *      This will not destroy the default VID.
 *      This disables frame forwarding on the specified VID, at the forwarding
 *      table and at the port,VID table and the MVT.
 *      Assumes input parameters have been checked for validity.
 *      Assumes the lock has already been taken (DO NOT CALL EXTERNALLY).
 *      Makes external calls; these should not call back into this module.
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_remove"
static int
_bcm_fe2000_vlan_remove(const int unit,
                        const bcm_vlan_t vid)
{
#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_v2e_t    p3vlan2Etc;          /* G2P3 vlan2Etc entry */
#endif /* def BCM_FE2000_P3_SUPPORT */
    _bcm_sbx_vlan_ft_t    ft;                  /* working FT (flood) entry */
    bcm_port_t            pIndex;              /* working port index */
    int                   result = BCM_E_NONE; /* local result code */
    int                   tempRes;             /* called function's result */
    bcm_stg_t             curstg;              /* working STG */
    int                   vlanValid = FALSE;   /* indicates VLAN exists */
#if 0 
    bcm_vlan_t            curvid;              /* working untagged VID */
#endif 

    VLAN_EVERB((VLAN_FUNCMSG("(%d,%04X) enter\n"),
                unit,
                vid));

    /* Check for default VID and abort if so. */
    if (_vlan_state[unit]->default_vid == vid) {
        /* It's the default VID; we are not meant to destroy that one! */
        VLAN_ERR((VLAN_FUNCMSG1("tried to destroy default VID %04X"
                                " on unit %d\n"),
                  vid,
                  unit));
        result = BCM_E_PARAM;
    }
    if (_sbx_vlan_max_vid_value < vid) {
        /* the specified VID is not valid */
        VLAN_ERR((VLAN_FUNCMSG1("tried to destroy invalid VID %04X"
                                " on unit %d\n"),
                  vid,
                  unit));
        result = BCM_E_PARAM;
    }

    /* find out if the VLAN exists */
    if (BCM_E_NONE == result) {
        result = _bcm_fe2000_vlan_check(unit, vid, &vlanValid, &ft);
        if (BCM_E_NONE == result) {
            if (!vlanValid) {
                result = BCM_E_NOT_FOUND;
                VLAN_ERR((VLAN_FUNCMSG1("VLAN %04X does not exist"
                                        " on unit %d\n"),
                          vid,
                          unit));
            }
        } /* if (BCM_E_NONE == result) */
    } /* if (BCM_E_NONE == result) */


    /* remove the VID from its STG */
    if (BCM_E_NONE == result) {
        result = _bcm_fe2000_stg_map_get(unit, vid, &curstg);
        if (BCM_E_NONE == result) {
            /* Got current STG; remove VID from that STG */
            result = _bcm_fe2000_stg_vlan_remove(unit, curstg, vid, TRUE);
            if (BCM_E_NONE != result) {
                VLAN_ERR((VLAN_FUNCMSG1("_bcm_fe2000_stg_vlan_remove"
                                        "(%d,%d,%d,TRUE) returned %d (%s)\n"),
                          unit,
                          curstg,
                          vid,
                          result,
                          _SHR_ERRMSG(result)));
            }
        } else { /* if (BCM_E_NONE == result) */
            /* Failed to read STG for this VID */
            result = BCM_E_FAIL; 
            VLAN_ERR((VLAN_FUNCMSG("_bcm_fe2000_stg_map_get(%d,%03X,*)"
                                   " returned %d (%s)\n"),
                      unit,
                      vid,
                      result,
                      _SHR_ERRMSG(result)));
        } /* if (BCM_E_NONE == result) */
    } /* if (BCM_E_NONE == result) */

    /* tear down the VID */
    if (BCM_E_NONE == result) {
#ifdef BCM_FE2000_P3_SUPPORT
        if (SOC_IS_SBX_G2P3(unit)) {
            uint32 ftidx = 0, unknown_mc_ft_offset = 0;
            /* set exception that indicates invalid FT entry */
            ft.p3ft.excidx = VLAN_INV_FTE_EXC;
            
            ft.p3ft.rridx = 0; 

            BCM_IF_ERROR_RETURN
               (soc_sbx_g2p3_vlan_ft_base_get(unit, &ftidx));
            ftidx += SBX_VSI_FROM_VID(vid);
            result = soc_sbx_g2p3_ft_set(unit,
                                         ftidx,
                                         &(ft.p3ft));

            /* invalidate the unknown mc ft as well */
            if (BCM_E_NONE == result) {
                result = soc_sbx_g2p3_mc_ft_offset_get(unit, &unknown_mc_ft_offset);
                if (BCM_E_NONE == result) {
                ftidx += unknown_mc_ft_offset;
                result = soc_sbx_g2p3_ft_set(unit,
                                         ftidx,
                                         &(ft.p3ft));
            }
            }

            if (BCM_E_NONE != result) {
                VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:ft[%08X]:"
                                        " %d (%s)\n"),
                          unit,
                          SBX_VSI_FROM_VID(vid),
                          result,
                          _SHR_ERRMSG(result)));
            } else {
                result = soc_sbx_g2p3_v2e_get(unit,
                                              SBX_VSI_FROM_VID(vid),
                                              &p3vlan2Etc);
                if (BCM_E_NONE == result) {
                    p3vlan2Etc.dontlearn = TRUE;
                    result = soc_sbx_g2p3_v2e_set(unit,
                                                  SBX_VSI_FROM_VID(vid),
                                                  &p3vlan2Etc);
                }
                if (BCM_E_NONE != result) {
                    VLAN_ERR((VLAN_FUNCMSG1("unable to access %d:v2e[%04X]:"
                                            " %d (%s)\n"),
                              unit,
                              vid,
                              result,
                              _SHR_ERRMSG(result)));
                }
            }
        } /* if (SOC_IS_SBX_G2P3(unit)) */
#endif /* def BCM_FE2000_P3_SUPPORT */
    } /* if (BCM_E_NONE == result) */

    /* take all ports out of this VID */
    if (BCM_E_NONE == result) {
        PBMP_ALL_ITER(unit,pIndex) {
            /* remove this port from this VID */
            tempRes = _bcm_fe2000_vlan_port_remove(unit,
                                                   vid,
                                                   pIndex);
            if (BCM_E_NONE != tempRes) {
                result = tempRes;
                VLAN_ERR((VLAN_FUNCMSG1("_bcm_fe2000_vlan_port_remove"
                                        "(%d,%04X,%d) returned %d (%s)\n"),
                          unit,
                          vid,
                          pIndex,
                          result,
                          _SHR_ERRMSG(result)));
            } /* if (BCM_E_NONE != tempRes) */
#if 0 
            tempRes = bcm_fe2000_port_untagged_vlan_get(unit, pIndex, &curvid);
            if ((BCM_E_NONE == tempRes) && (curvid == vid)) {
                /* Port is on this VID, so move it to default */
                result = bcm_fe2000_port_untagged_vlan_set(unit,
                                                           pIndex,
                                                           _vlan_state[unit]->
                                                                  default_vid);
                if (BCM_E_NONE != result) {
                    /* error setting port's VID */
                    VLAN_ERR((VLAN_FUNCMSG1("bcm_fe2000_port_untagged_vlan_set"
                                            "(%d,%d,%04X) returned %d (%s)\n"),
                              unit,
                              pIndex,
                              _vlan_state[unit]->default_vid,
                              result,
                              _SHR_ERRMSG(result)));
                }
            } else if (BCM_E_NONE != tempRes) {
                /* error getting port's VID */
                result = tempRes;
                VLAN_ERR((VLAN_FUNCMSG1("bcm_fe2000_port_untagged_vlan_get"
                                        "(%d,%d,*) returned %d (%s)\n"),
                          unit,
                          pIndex,
                          result,
                          _SHR_ERRMSG(result)));
            }
#endif 
        } /* PBMP_ALL_ITER(unit,pIndex) */
    } /* if (BCM_E_NONE == result) */

    VLAN_EVERB((VLAN_FUNCMSG("(%d,%04X) return %d (%s)\n"),
                unit,
                vid,
                result,
                _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_port_is_member
 *   Purpose
 *      Test whether a single port is a member of the specified VID.
 *   Parameters
 *      (in) const int unit = unit number
 *      (in) const bcm_vlan_t vid = VID whose port is to be queried
 *      (in) const bcm_port_t port = port whose status is to be queried
 *      (out) int *member = where to put member flag
 *      (out) int *untagged = where to put untagged flag
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately for other conditions
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      Does not check parameter validity.
 *      Does not lock/unlock, so can't call from outside VLAN lock context
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_port_is_member"
static int
_bcm_fe2000_vlan_port_is_member(const int unit,
                                const bcm_vlan_t vid,
                                const bcm_port_t port,
                                int* member,
                                int* untagged)
{
#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_evp2e_t  p3egrVlanPort2Etc;    /* G2P3 egrVlanPort2Etc ent */
#endif /* def BCM_FE2000_P3_SUPPORT */
    soc_sbx_g2p3_pv2appdata_t p3pv2appdata;     /* G2P3 pv2appdata entry */
    int                   isMember = FALSE;     /* whether port is a member */
    int                   isUntagged = FALSE;   /* whether port is untagged */
    uint32                eteNum = ~0;          /* ET entry number */
    int                   result;               /* local result */

    /* optimistically hope for success */
    result = BCM_E_NONE;

#ifdef BCM_FE2000_P3_SUPPORT
    if (SOC_IS_SBX_G2P3(unit)) {
        /* nothing to do here, but need the negative of this condition */
    } else
#endif /* def BCM_FE2000_P3_SUPPORT */
    {
        VLAN_ERR((VLAN_FUNCMSG1("invalid unit %d type\n"), unit));
        result = VLAN_UNKNOWN_TYPE;
    }

    /* check whether port is a member of the VLAN */
    if (BCM_E_NONE == result) {
#ifdef BCM_FE2000_P3_SUPPORT
        if (SOC_IS_SBX_G2P3(unit)) {
            result = SOC_SBX_G2P3_PV2APPDATA_GET(unit,
                                                 port,
                                                 vid,
                                                 &p3pv2appdata);
            if (BCM_E_NONE == result) {
                isMember = (0 != p3pv2appdata.member);
            } else {
                VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:pv2appdata[%d,%03X]:"
                                        " %d (%s)\n"),
                          unit,
                          port,
                          vid,
                          result,
                          _SHR_ERRMSG(result)));
            }
        } /* if (SOC_IS_SBX_G2P3(unit)) */
#endif /* def BCM_FE2000_P3_SUPPORT */
    } /* if (BCM_E_NONE == result) */

    /* if port is a member, find out if port is tagged or untagged */
    if (isMember && (BCM_E_NONE == result)) {
#ifdef BCM_FE2000_P3_SUPPORT
        if (SOC_IS_SBX_G2P3(unit)) {
            result = soc_sbx_g2p3_evp2e_get(unit,
                                            vid,
                                            port,
                                            &p3egrVlanPort2Etc);
            if (BCM_E_NONE == result) {
                eteNum = p3egrVlanPort2Etc.eteptr;
            } else {
                VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:evp2e[%d,%03X]:"
                                        " %d (%s)\n"),
                          unit,
                          vid,
                          port,
                          result,
                          _SHR_ERRMSG(result)));
            }
        } /* if (SOC_IS_SBX_G2P3(unit)) */
#endif /* def BCM_FE2000_P3_SUPPORT */
        
        if (eteNum == SOC_SBX_PORT_UT_ETE(unit,  port)) {
            /* using standard strip ETE for this port, so untagged */
            isUntagged = TRUE;
        }
    }

    /* okay, return the results */
    if (member) {
        *member = isMember;
    }
    if (untagged) {
        *untagged = isUntagged;
    }
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_port_tagged_ingress_drop_get
 *   Purpose
 *      Get the 'drop tagged frames' setting for the specified port.
 *   Parameters
 *      (in) const int unit = unit whose port is to be checked
 *      (in) const bcm_port_t = port whose filter state is to be set
 *      (out) bcm_vlan_t *vlan = where to put native VLAN if dropping tagged
 *      (out) int *drop = where to put the port's drop tagged state
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_port_tagged_ingress_drop_get"
int
_bcm_fe2000_vlan_port_tagged_ingress_drop_get(const int unit,
                                              const bcm_port_t port,
                                              bcm_vlan_t *vlan,
                                              int *drop)
{
    int                   result;               /* local result */

    VLAN_EVERB((VLAN_FUNCMSG("(%d,%d,*,*) enter\n"),
                unit,
                port));

    if ((!drop) || (!vlan)) {
        /* null isn't allowed here */
        VLAN_ERR((VLAN_FUNCMSG1("NULL pointer for output argument\n")));
        return BCM_E_PARAM;
    }

    /* Check for proper initialisation */
    if ((!(_primary_lock)) || (!(_vlan_state[unit]))) {
        VLAN_ERR((VLAN_FUNCMSG1("unit %d does not have VLAN initialised\n"),
                  unit));
        return BCM_E_INIT;
    }

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* from this point, hope for the best */
    result = BCM_E_NONE;

    /* make sure port is valid */
    if ((BCM_E_NONE == result) && (!PBMP_MEMBER(PBMP_ALL(unit), port))) {
        /* port is not valid; set error now */
        result = BCM_E_PARAM;
        VLAN_ERR((VLAN_FUNCMSG1("unit %d port %d is not valid\n"),
                  unit,
                  port));
    } /* if (no error yet and port is not valid) */

    /* set the output argument */
    if (BCM_E_NONE == result) {
        *drop = BCM_PBMP_MEMBER(_vlan_state[unit]->tag_drop, port);
        *vlan = _vlan_state[unit]->nvid[port];
    } /* if (BCM_E_NONE == result) */

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_EVERB((VLAN_FUNCMSG("(%d,%d,&(%04X),&(%s)) return %d (%s)\n"),
                unit,
                port,
                *vlan,
                (*drop)?"TRUE":"FALSE",
                result,
                _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_port_untagged_ingress_drop_get
 *   Purpose
 *      Get the 'drop untagged frames' setting for the specified port.
 *   Parameters
 *      (in) const int unit = unit whose port is to be checked
 *      (in) const bcm_port_t = port whose filter state is to be set
 *      (out) int *drop = where to put the port's drop untagged state
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_port_untagged_ingress_drop_get"
int
_bcm_fe2000_vlan_port_untagged_ingress_drop_get(const int unit,
                                                const bcm_port_t port,
                                                int *drop)
{
    int                   result;               /* local result */

    VLAN_EVERB((VLAN_FUNCMSG("(%d,%d,*) enter\n"),
                unit,
                port));

    if (!drop) {
        /* null isn't allowed here */
        VLAN_ERR((VLAN_FUNCMSG1("NULL pointer for output argument\n")));
        return BCM_E_PARAM;
    }

    /* Check for proper initialisation */
    if ((!(_primary_lock)) || (!(_vlan_state[unit]))) {
        VLAN_ERR((VLAN_FUNCMSG1("unit %d does not have VLAN initialised\n"),
                  unit));
        return BCM_E_INIT;
    }

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* from this point, hope for the best */
    result = BCM_E_NONE;

    /* make sure port is valid */
    if ((BCM_E_NONE == result) && (!PBMP_MEMBER(PBMP_ALL(unit), port))) {
        /* port is not valid; set error now */
        result = BCM_E_PARAM;
        VLAN_ERR((VLAN_FUNCMSG1("unit %d port %d is not valid\n"),
                  unit,
                  port));
    } /* if (no error yet and port is not valid) */

    /* set the output argument */
    if (BCM_E_NONE == result) {
        *drop = BCM_PBMP_MEMBER(_vlan_state[unit]->untag_drop, port);
    } /* if (BCM_E_NONE == result) */

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_EVERB((VLAN_FUNCMSG("(%d,%d,&(%s)) return %d (%s)\n"),
                unit,
                port,
                (*drop)?"TRUE":"FALSE",
                result,
                _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_port_untagged_ingress_drop_set
 *   Purpose
 *      Set the 'drop untagged frames' setting for the specified port.
 *   Parameters
 *      (in) const int unit = unit whose port is to be checked
 *      (in) const bcm_port_t = port whose filter state is to be set
 *      (in) int drop = the port's drop untagged state
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      Destroys translation that applies to untagged/pritagged frames.
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_port_untagged_ingress_drop_set"
int
_bcm_fe2000_vlan_port_untagged_ingress_drop_set(const int unit,
                                                const bcm_port_t port,
                                                int drop)
{
    int                     result;             /* local result */

    VLAN_EVERB((VLAN_FUNCMSG("(%d,%d,%s) enter\n"),
                unit,
                port,
                drop?"TRUE":"FALSE"));

    /* Check for proper initialisation */
    if ((!(_primary_lock)) || (!(_vlan_state[unit]))) {
        VLAN_ERR((VLAN_FUNCMSG1("unit %d does not have VLAN initialised\n"),
                  unit));
        return BCM_E_INIT;
    }

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* from this point, hope for the best */
    result = BCM_E_NONE;

    /* make sure port is valid */
    if (!PBMP_MEMBER(PBMP_ALL(unit), port)) {
        /* port is not valid; set error now */
        result = BCM_E_PARAM;
        VLAN_ERR((VLAN_FUNCMSG1("unit %d port %d is not valid\n"),
                  unit,
                  port));
    } /* if (no error yet and port is not valid) */

    /* set the new state flag */
    if (BCM_E_NONE == result) {
        if (drop) {
            BCM_PBMP_PORT_ADD(_vlan_state[unit]->untag_drop, port);
        } else {
            BCM_PBMP_PORT_REMOVE(_vlan_state[unit]->untag_drop, port);
        }
    } /* if (BCM_E_NONE == result) */

#ifdef BCM_FE2000_P3_SUPPORT
    if (SOC_IS_SBX_G2P3(unit)) {
        result = _bcm_fe2000_g2p3_vlan_port_nativevid_touch(unit, port);
    } else
#endif /* def BCM_FE2000_P3_SUPPORT */
    {
        result = VLAN_UNKNOWN_TYPE;
    }

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_EVERB((VLAN_FUNCMSG("(%d,%d,%s) return %d (%s)\n"),
                unit,
                port,
                drop?"TRUE":"FALSE",
                result,
                _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_port_native_vlan_get
 *   Purpose
 *      Get the native VID for the specified port
 *   Parameters
 *      (in) const int unit = unit whose port is to be checked
 *      (in) const bcm_port_t = port whose filter state is to be set
 *      (out) bcm_vlan_t *vlan = where to put native VLAN if dropping tagged
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      Only sets vlan if dropping of tagged frames is enabled, else it is left
 *      untouched.
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_port_native_vlan_get"
int
_bcm_fe2000_vlan_port_native_vlan_get(const int unit,
                                      const bcm_port_t port,
                                      bcm_vlan_t *vlan)
{
    int                   result;               /* local result */

    VLAN_EVERB((VLAN_FUNCMSG("(%d,%d,*) enter\n"),
                unit,
                port));

    if (!vlan) {
        /* null isn't allowed here */
        VLAN_ERR((VLAN_FUNCMSG1("NULL pointer for output argument\n")));
        return BCM_E_PARAM;
    }

    /* Check for proper initialisation */
    if ((!(_primary_lock)) || (!(_vlan_state[unit]))) {
        VLAN_ERR((VLAN_FUNCMSG1("unit %d does not have VLAN initialised\n"),
                  unit));
        return BCM_E_INIT;
    }

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* from this point, hope for the best */
    result = BCM_E_NONE;

    /* make sure port is valid */
    if ((BCM_E_NONE == result) && (!PBMP_MEMBER(PBMP_ALL(unit), port))) {
        /* port is not valid; set error now */
        result = BCM_E_PARAM;
        VLAN_ERR((VLAN_FUNCMSG1("unit %d port %d is not valid\n"),
                  unit,
                  port));
    } /* if (no error yet and port is not valid) */

    /* set the output argument */
    if (BCM_E_NONE == result) {
        *vlan = _vlan_state[unit]->nvid[port];
    } /* if (BCM_E_NONE == result) */

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_EVERB((VLAN_FUNCMSG("(%d,%d,&(%03X)) return %d (%s)\n"),
                unit,
                port,
                *vlan,
                result,
                _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_port_native_vlan_set
 *   Purpose
 *      Set the native VID for a port.
 *   Parameters
 *      (in) const int unit = unit whose port is to be checked
 *      (in) const bcm_port_t port = port whose filter state is to be set
 *      (in) const bcm_vlan_t vlan = new native VID for the port
 *      (in) const int keeporstrip = whether to set keeporstrip
 *      (in) const _bcm_fe2k_nvid_action_t action = detailed action
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      This mirrors the pvid2etc entry for the native VID to the zero VID, and
 *      sets up so that any further writes will also be mirrored.
 *      The reflect flags control pvv2e behaviour when reflecting changes to
 *      the native VID properties (including this call).
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_port_native_vlan_set"
int
_bcm_fe2000_vlan_port_native_vlan_set(const int unit,
                                      const bcm_port_t port,
                                      const bcm_vlan_t vlan,
                                      const _bcm_fe2k_nvid_pvv2e_control_flags_t flags,
                                      const _bcm_fe2k_nvid_action_t action)
{
#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_pv2e_t   p3pv2e;               /* G2P3 zero VID pv2e */
    soc_sbx_g2p3_lp_t     p3lp;                 /* G2P3 zero VID LP */
#endif /* def BCM_FE2000_P3_SUPPORT */
    int                   result;               /* local result */

    VLAN_VVERB((VLAN_FUNCMSG("(%d,%d,%03X,%08X) enter\n"),
                unit,
                port,
                vlan,
                flags));

    /* Check for proper initialisation */
    if ((!(_primary_lock)) || (!(_vlan_state[unit]))) {
        VLAN_ERR((VLAN_FUNCMSG1("unit %d does not have VLAN initialised\n"),
                  unit));
        return BCM_E_INIT;
    }
    if (~BCM_FE2K_NVID_FLAGS_MASK & flags) {
        VLAN_ERR((VLAN_FUNCMSG1("invalid flags are set: %08X\n"),
                  ~BCM_FE2K_NVID_FLAGS_MASK & flags));
        return BCM_E_PARAM;
    }

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* from this point, hope for the best */
    result = BCM_E_NONE;

#ifdef BCM_FE2000_P3_SUPPORT
    if (SOC_IS_SBX_G2P3(unit)) {
        /* nothing to do here, but need the negative of this condition */
    } else
#endif /* def BCM_FE2000_P3_SUPPORT */
    {
        VLAN_ERR((VLAN_FUNCMSG1("invalid unit %d type\n"), unit));
        result = VLAN_UNKNOWN_TYPE;
    }

    /* make sure port is valid */
    if ((BCM_E_NONE == result) && (!PBMP_MEMBER(PBMP_ALL(unit), port))) {
        /* port is not valid; set error now */
        result = BCM_E_PARAM;
        VLAN_ERR((VLAN_FUNCMSG1("unit %d port %d is not valid\n"),
                  unit,
                  port));
    } /* if (no error yet and port is not valid) */

    /* set the native VLAN for untagged frames */
    if (BCM_E_NONE == result) {
        /* track the native VIDs per port */
        _vlan_state[unit]->nvid[port] = vlan;
#ifdef BCM_FE2000_P3_SUPPORT
        _vlan_state[unit]->nvflag[port] = flags;
#endif /* def BCM_FE2000_P3_SUPPORT */
    
#ifdef BCM_FE2000_P3_SUPPORT
        /* this must be done AFTER local nvid and keeporstrip are updated */
        if (SOC_IS_SBX_G2P3(unit)) {
            if ((action == BCM_FE2K_NVID_ACTION_SET) &&
                (_vlan_state[unit]->divergence[port] & _FE2K_VLAN_DIVERGE_ALL)) {
                VLAN_VVERB((VLAN_FUNCMSG1("reconverge unit %d port %d"
                                          " vid %03X\n"),
                            unit,
                            port,
                            0));
                /* get pv2e for zero VID */
                result = SOC_SBX_G2P3_PV2E_GET(unit,
                                               port,
                                               0,
                                               &p3pv2e);
                if (BCM_E_NONE != result) {
                    VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:pv2e[%d,%03X]:"
                                            " %d (%s)\n"),
                              unit,
                              port,
                              0,
                              result,
                              _SHR_ERRMSG(result)));
                }
                /* read its logical port */
                if (BCM_E_NONE == result) {
                    result = soc_sbx_g2p3_lp_get(unit,
                                                 p3pv2e.lpi?p3pv2e.lpi:port,
                                                 &p3lp);
                    if (BCM_E_NONE != result) {
                        VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:lp[%08X]:"
                                                " %d (%s)\n"),
                                  unit,
                                  p3pv2e.lpi?p3pv2e.lpi:port,
                                  result,
                                  _SHR_ERRMSG(result)));
                    }
                }

                /* reset the qos profile to the default if diverged */
                if ((BCM_E_NONE == result) &&
                    (_vlan_state[unit]->divergence[port] &
                     _FE2K_VLAN_DIVERGE_QOS) &&
                    p3lp.qos &&
                    p3pv2e.lpi) {

                    result =
                        _bcm_fe2000_port_default_qos_profile_set(unit,
                                                                 port, 0);

                    if (BCM_E_NONE != result) {
                        VLAN_ERR((VLAN_FUNCMSG1(" unable to set qos profile "
                                                "back to default.  %d:port=%d "
                                                " qos[%02X]: %d (%s)\n"),
                                  unit, port, p3lp.qos, result,
                                  _SHR_ERRMSG(result)));
                    }

                }
                
                /* free the logical port */
                if ((BCM_E_NONE == result) && p3pv2e.lpi) {
                    result = _sbx_gu2_resource_free(unit,
                                                    SBX_GU2K_USR_RES_LPORT,
                                                    1,
                                                    &(p3pv2e.lpi),
                                                    0);
                    if (BCM_E_NONE != result) {
                        VLAN_ERR((VLAN_FUNCMSG1("unable to free %d:lp[%08X]:"
                                                " %d (%s)\n"),
                                  unit,
                                  p3pv2e.lpi,
                                  result,
                                  _SHR_ERRMSG(result)));
                    }
                }
                _vlan_state[unit]->divergence[port] = 0;
            } /* if (diverged) */
            VLAN_VVERB((VLAN_FUNCMSG1("replicate VID %03X settings to"
                                      " VID 000 settings for unit %d"
                                      " port %d\n"),
                        vlan,
                        unit,
                        port));
            result = _bcm_fe2000_g2p3_vlan_port_nativevid_touch(unit, port);
        } /* if (SOC_IS_SBX_G2P3(unit)) */
#endif /* def BCM_FE2000_P3_SUPPORT */
    } /* if (BCM_E_NONE == result) */

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_VVERB((VLAN_FUNCMSG("(%d,%d,%03X) return %d (%s)\n"),
                unit,
                port,
                vlan,
                result,
                _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME



/*
 *   Function
 *      _bcm_fe2000_g2p3vlan_drop_tagged_hw_update
 *   Purpose
 *      Set the 'drop tagged frames' setting for the specified port for g2p3
 *   Parameters
 *      (in) const int unit = unit whose port is to be checked
 *      (in) const bcm_port_t = port whose filter state is to be set
 *      (in) const bcm_vlan_t = port's native VLAN ID
 *      (in) const int drop = new drop tagged state for the port
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      This will destroy ingress translations on the port.
 */
#ifdef BCM_FE2000_P3_SUPPORT
#define _VLAN_FUNC_NAME "_bcm_fe2000_g2p3vlan_drop_tagged_hw_update"
int
_bcm_fe2000_g2p3_vlan_drop_tagged_hw_update(const int unit,
                                            const bcm_port_t port,
                                            const bcm_vlan_t vlan,
                                            const int drop)
{
    int                   rv;
    bcm_vlan_t            vid;
    soc_sbx_g2p3_pv2e_t   npVid2Etc;            /* G2P3 native pVid2Etc entry */
    soc_sbx_g2p3_p2appdata_t p2appdata;         /* G2P3 p2appdata entry */
    uint32_t             *fastVlan;             /* pv2e.vlan */
    uint32_t             *fastMember;           /* pv2appdata.member */
    int                   ingFilter = FALSE;    /* ingress filtering enable */
    int                   egrFilter = FALSE;    /* egress filtering enable */

    rv = soc_sbx_g2p3_p2appdata_get(unit, port, &p2appdata);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:p2appdatat[%d]:"
                                " %d (%s)\n"),
                  unit, port, rv, bcm_errmsg(rv)));
        return rv;
    }

    ingFilter = (0 != p2appdata.ifilteren);
    egrFilter = (0 != p2appdata.efilteren);

    /* get the native pVid2Etc entry for this port */
    if (vlan != BCM_VLAN_MAX) {
        /* not 'no VLAN'; read information */
        rv = SOC_SBX_G2P3_PV2E_GET(unit, port, vlan, &npVid2Etc);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:pv2e[%d,%03X]:"
                                    " %d (%s)\n"),
                      unit, port, vlan, rv, bcm_errmsg(rv)));
            return rv;
        }
    } else {
        /* 'no VLAN'; just assume native is blank */
        soc_sbx_g2p3_pv2e_t_init(&npVid2Etc);
    }

    fastVlan = sal_alloc(sizeof(uint32_t) * (BCM_VLAN_MAX + 1),
                         "fastVlan");
    fastMember = sal_alloc(sizeof(uint32_t) * (BCM_VLAN_MAX + 1),
                           "fastMember");

    if (fastVlan == NULL || fastMember == NULL) {
        if (fastVlan) {
            sal_free(fastVlan);
        }
        if (fastMember) {
            sal_free(fastMember);
        }
        VLAN_ERR((VLAN_FUNCMSG1("failed to allocate work buffer\n")));
        return BCM_E_MEMORY;
    }

    if (BCM_SUCCESS(rv)) {
        rv = soc_sbx_g2p3_pv2e_vlan_fast_get(unit, 0, port, 0xFFF, port,
                                             fastVlan, BCM_VLAN_MAX + 1);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("failed to get pv2e[%d,*] vlan: %d %s\n"),
                      port, rv, bcm_errmsg(rv)));
        }
    }

    if (BCM_SUCCESS(rv)) {
        rv = soc_sbx_g2p3_pv2appdata_member_fast_get(unit, 0, port,
                                                     0xFFF, port,
                                                     fastMember,
                                                     BCM_VLAN_MAX + 1);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("failed to get pv2appdata[%d,*] member: "
                                    "%d %s\n"),
                      port, rv, bcm_errmsg(rv)));
        }
    }

    /*
     *  If switching from dropTagged off to on, we need to take the
     *  current nativeVid configuration, put it into pVid2Etc[*,0]
     *  for the port, and set the others to use VLAN 0.
     *
     *  If switching from dropTagged on to off, we need to take the
     *  current pVid2Etc[*,0] for the port, put it back into the
     *  nativeVid entry, and set any others to either their VID (if
     *  member or if ingress filtering disabled) or zero (ingress
     *  filtering enabled and not member).
     *
     *  If switching from dropTagged on to on, we can essentially
     *  follow the off to on method (specifically since we already have the
     *  native VID entry based upon the current mode).
     *
     *  If switching from dropTagged off to off, we can essentially
     *  follow the on to off method (specifically since we already have the
     *  native VID entry based upon the current mode).
     */

    /* editing untagged/pritagged VID entry */
    if (PBMP_MEMBER(_vlan_state[unit]->untag_drop, port)) {
        /* dropping untagged frames; VSI = 0 */
        fastVlan[0] = 0;
    } else {
        /* not dropping untagged; VSI = default */
        fastVlan[0] = npVid2Etc.vlan ? npVid2Etc.vlan : SBX_VSI_FROM_VID(vlan);
    }
    VLAN_VVERB((VLAN_FUNCMSG1("port=%d untag_drop=$d vlan[0]=0x%x "
                              "nvlan=0x%x\n"),
                port, PBMP_MEMBER(_vlan_state[unit]->untag_drop, port),
                fastVlan[0], npVid2Etc.vlan));
    

    for (vid = 1; vid <= _sbx_vlan_max_vid_value; vid++) {
        /* for each VLAN, this port will probably need to be adjusted */
        /* make changes per mode to the VSI */
        if (drop) {
            /* dropping tagged; zero target VSI */
            fastVlan[vid] = 0;
        } else if ((fastMember[vid]) || (!ingFilter)) {
            /* member or not ingress filtering; set VSI */
            if (fastVlan[vid] == 0) {
                fastVlan[vid] = SBX_VSI_FROM_VID(vid);
            }
        } else {
            /* ingress filtering and not member; VSI = 0 */
            fastVlan[vid] = 0;
        }

        /* bail out if something went wrong */
        if (BCM_E_NONE != rv) {
            break;
        }
    } /* for (vid = 0; vid <= _sbx_vlan_max_vid_value; vid++) */

    if (BCM_SUCCESS(rv)) {
        rv = soc_sbx_g2p3_pv2e_vlan_fast_set(unit, 0, port, 0xFFF, port,
                                             NULL, /* set them all */
                                             fastVlan, BCM_VLAN_MAX + 1);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("failed to set pv2e[%d,*] vlan: %d %s\n"),
                      port, rv, bcm_errmsg(rv)));
        }
    }

    /* Update the pvv2e entry, if it exists */
    if (BCM_SUCCESS(rv)) {
        soc_sbx_g2p3_pvv2e_t pvv2e;

        rv = soc_sbx_g2p3_pvv2e_get(unit, 0xFFF, 0x000, port, &pvv2e);
        if (BCM_SUCCESS(rv)) {
            pvv2e.vlan = fastVlan[0];
            rv = soc_sbx_g2p3_pvv2e_set(unit, 0xFFF, 0x000, port, &pvv2e);
        } else {
            /* the entry may not exist, and that is OK */
            rv = BCM_E_NONE;
        }
    }

    if (fastVlan) {
        sal_free(fastVlan);
    }
    if (fastMember) {
        sal_free(fastMember);
    }

    return rv;
}
#undef  _VLAN_FUNC_NAME
#endif  /* BCM_FE2000_P3_SUPPORT */


/*
 *   Function
 *      _bcm_fe2000_vlan_port_tagged_ingress_drop_set
 *   Purpose
 *      Set the 'drop tagged frames' setting for the specified port.
 *   Parameters
 *      (in) const int unit = unit whose port is to be checked
 *      (in) const bcm_port_t = port whose filter state is to be set
 *      (in) const bcm_vlan_t = port's native VLAN ID
 *      (in) const int drop = new drop tagged state for the port
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      This will destroy ingress translations on the port.
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_port_tagged_ingress_drop_set"
int
_bcm_fe2000_vlan_port_tagged_ingress_drop_set(const int unit,
                                              const bcm_port_t port,
                                              const bcm_vlan_t vlan,
                                              const int drop)
{
    int                   result;               /* local result code */

    VLAN_VVERB((VLAN_FUNCMSG("(%d,%d,%03X,%s) enter\n"),
                unit,
                port,
                vlan,
                drop?"TRUE":"FALSE"));

    /* Check for proper initialisation */
    if ((!(_primary_lock)) || (!(_vlan_state[unit]))) {
        VLAN_ERR((VLAN_FUNCMSG1("unit %d does not have VLAN initialised\n"),
                  unit));
        return BCM_E_INIT;
    }

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* from this point, hope for the best */
    result = BCM_E_NONE;

#ifdef BCM_FE2000_P3_SUPPORT
    if (SOC_IS_SBX_G2P3(unit)) {
        /* only want this for negative condition */
    } else
#endif /* def BCM_FE2000_P3_SUPPORT */
    {
        VLAN_ERR((VLAN_FUNCMSG1("invalid unit %d type\n"), unit));
        result = VLAN_UNKNOWN_TYPE;
    }

    /* make sure port is valid */
    if ((BCM_E_NONE == result) && (!PBMP_MEMBER(PBMP_ALL(unit), port))) {
        /* port is not valid; set error now */
        result = BCM_E_PARAM;
        VLAN_ERR((VLAN_FUNCMSG1("unit %d port %d is not valid\n"),
                  unit,
                  port));
    } /* if (no error yet and port is not valid) */

    /* preserve the current native VLAN */
    if (BCM_E_NONE == result) {
        _vlan_state[unit]->nvid[port] = vlan;
    }

    /* now set all pVid2Etc entries for this port appropriately */
    if (BCM_E_NONE == result) {
       switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = _bcm_fe2000_g2p3_vlan_drop_tagged_hw_update(unit, port,
                                                                 vlan, drop);
            break;
#endif
        default:
            result = BCM_E_CONFIG;
            break;
        }
    } /* if (BCM_E_NONE == result) */

    /* update the port's drop tag state */
    if (BCM_E_NONE == result) {
        if (drop) {
            /* mark this port as dropping tagged traffic */
            BCM_PBMP_PORT_ADD(_vlan_state[unit]->tag_drop, port);
        } else {
            /* mark this port as not dropping tagged traffic */
            BCM_PBMP_PORT_REMOVE(_vlan_state[unit]->tag_drop, port);
        }
    }

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_VVERB((VLAN_FUNCMSG("(%d,%d,%03X,%s) return %d (%s)\n"),
                unit,
                port,
                vlan,
                drop?"TRUE":"FALSE",
                result,
                _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_port_filter_get
 *   Purpose
 *      Get the filter setting for the specified port.
 *   Parameters
 *      (in) const int unit = unit whose port is to be checked
 *      (in) const bcm_port_t = port whose filter state is to be checked
 *      (out) int *ingress = where to put ingress filtering state (TRUE/FALSE)
 *      (out) int *egress = where to put egress filtering state (TRUE/FALSE)
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      *ingress and *egress will be set TRUE (filtering enabled) or FALSE
 *      (filtering disabled) on success (and not touched on most failures)
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_port_filter_get"
int
_bcm_fe2000_vlan_port_filter_get(const int unit,
                                 const bcm_port_t port,
                                 int *ingress,
                                 int *egress)
{
#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_p2appdata_t p3p2appdata;       /* G2P3 p2appdata entry */
#endif /* def BCM_FE2000_P3_SUPPORT */
    int                   result;               /* local result code */

    VLAN_VVERB((VLAN_FUNCMSG("(%d,%d,*,*) enter\n"),
                unit,
                port));

    /* check for valid parameters */
    if (!ingress || !egress) {
        VLAN_ERR((VLAN_FUNCMSG1("NULL pointer for output arguments\n")));
        return BCM_E_PARAM;
    }

    /* Check for proper initialisation */
    if ((!(_primary_lock)) || (!(_vlan_state[unit]))) {
        VLAN_ERR((VLAN_FUNCMSG1("unit %d does not have VLAN initialised\n"),
                  unit));
        return BCM_E_INIT;
    }

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* from this point, hope for the best */
    result = BCM_E_NONE;

#ifdef BCM_FE2000_P3_SUPPORT
    if (SOC_IS_SBX_G2P3(unit)) {
        /* nothing to do here, but need the negative of this condition */
    } else
#endif /* def BCM_FE2000_P3_SUPPORT */
    {
        VLAN_ERR((VLAN_FUNCMSG1("invalid unit %d type\n"), unit));
        result = VLAN_UNKNOWN_TYPE;
    }

    /* make sure port is valid */
    if ((BCM_E_NONE == result) && (!PBMP_MEMBER(PBMP_ALL(unit), port))) {
        /* port is not valid; set error now */
        result = BCM_E_PARAM;
        VLAN_ERR((VLAN_FUNCMSG1("unit %d port %d is not valid\n"),
                  unit,
                  port));
    } /* if (no error yet and port is not valid) */

    /* get the port filtering settings */
    if (BCM_E_NONE == result) {
#ifdef BCM_FE2000_P3_SUPPORT
        if (SOC_IS_SBX_G2P3(unit)) {
            result = soc_sbx_g2p3_p2appdata_get(unit, port, &p3p2appdata);
            if (BCM_E_NONE == result) {
                *ingress = (0 != p3p2appdata.ifilteren);
                *egress = (0 != p3p2appdata.efilteren);
            } else {
                VLAN_ERR((VLAN_FUNCMSG1("unable to access %d:p2appdata[%d]:"
                                        " %d (%s)\n"),
                          unit,
                          port,
                          result,
                          _SHR_ERRMSG(result)));
            }
        } /* if (SOC_IS_SBX_G2P3(unit)) */
#endif /* def BCM_FE2000_P3_SUPPORT */
    } /* if (BCM_E_NONE == result) */

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_VVERB((VLAN_FUNCMSG("(%d,%d,&(%s),&(%s)) return %d (%s)\n"),
                unit,
                port,
                (*ingress)?"TRUE":"FALSE",
                (*egress)?"TRUE":"FALSE",
                result,
                _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME



/*
 *   Function
 *      _bcm_fe2000_g2p3_vlan_port_ingress_filter_set
 *   Purpose
 *      Set the ingress filter setting for the specified port.
 *   Parameters
 *      (in) const int unit = unit whose port is to be checked
 *      (in) const bcm_port_t = port whose filter state is to be set
 *      (in) const int filter = new filter state for the port
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      Uses _fast accessors
 */
#ifdef BCM_FE2000_P3_SUPPORT
#define _VLAN_FUNC_NAME    "_bcm_fe2000_g2p3_vlan_port_ingress_filter_set"
int
_bcm_fe2000_g2p3_vlan_port_ingress_filter_set(const int unit,
                                              const bcm_port_t port,
                                              const int filter)
{
    soc_sbx_g2p3_p2appdata_t  p2appdata;
    int                       member;
    uint32_t                 *fastVlan;    /* pv2e.vlan */
    uint32_t                 *fastMember;  /* pv2appdata.member */
    bcm_vlan_t                vid;
    int                       rv = BCM_E_NONE;

    fastVlan = sal_alloc(sizeof(uint32_t) * (BCM_VLAN_MAX + 1),
                         "fastVlan");

    fastMember = sal_alloc(sizeof(uint32_t) * (BCM_VLAN_MAX + 1),
                           "fastMember");

    if (fastVlan == NULL || fastMember == NULL) {
        if (fastVlan) {
            sal_free(fastVlan);
        }
        if (fastMember) {
            sal_free(fastMember);
        }
        VLAN_ERR((VLAN_FUNCMSG1("failed to allocate work buffer\n")));
        return BCM_E_MEMORY;
    }

    if (BCM_SUCCESS(rv)) {
        rv = soc_sbx_g2p3_pv2e_vlan_fast_get(unit, 0, port, 0xFFF, port,
                                             fastVlan, BCM_VLAN_MAX + 1);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("failed to get pv2e[%d,*] vlan: %d %s\n"),
                      port, rv, bcm_errmsg(rv)));
        }
    }

    if (BCM_SUCCESS(rv)) {
        rv = soc_sbx_g2p3_pv2appdata_member_fast_get(unit, 0, port,
                                                     0xFFF, port,
                                                     fastMember,
                                                     BCM_VLAN_MAX + 1);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("failed to get pv2appdata[%d,*] member: "
                                    "%d %s\n"),
                      port, rv, bcm_errmsg(rv)));
        }
    }

    /* set all pVid2Etc entries for this port appropriately */
    if (BCM_SUCCESS(rv)) {
        for (vid = 1; vid <= _sbx_vlan_max_vid_value; vid++) {
            /* for each VLAN, this port may need to be adjusted */
            /*
             *  Figure out whether the port is a member of this VID.  Not
             *  as easy as it should be due to translation and other
             *  somewhat esoteric features.  For now:
             *
             *  If p3pv2appdata.member is true, then the port is a member
             *  (this case is TB without translation or translation set up
             *  after the port was added).
             *
             *  If p3pVid2Etc.vlan is outside of the TB range, then the
             *  port is a member (of something to do with this VID) (this
             *  has to do with VSWITCH, MPLS, IPv4, &c).
             *
             *  If p3pVid2Etc.vlan is inside the TB range, but is *not*
             *  equal to the VSI for the VID, then the port is a member
             *  (this has to do with translation and is not fully
             *  implemented in the code elsewhere but is here to avoid
             *  trampling it accidentally).
             *
             *  If there was some error already, the port is not a member
             *  (this is to avoid stupid compiler warnings about maybe
             *  using possibly uninitialised variables despite boolean
             *  short ciruiting that is part of the language).
             */
            member = ((0 != fastMember[vid]) ||
                      (fastVlan[vid] > BCM_VLAN_MAX) ||
                      ((fastVlan[vid] <= BCM_VLAN_MAX) &&
                       (fastVlan[vid] > 0) &&
                       (fastVlan[vid] != SBX_VSI_FROM_VID(vid))));

            /* adjust the entry appropriately, if appropriate */
            if (!member) {
                /*
                 *  The port is not a member of this VID.  This means that
                 *  we need to disable receipt of frames for this VLAN on
                 *  this port if filtering is enabled.  If port filtering
                 *  is being disabled, we set whether we'll accept tagged
                 *  frames based upon the port's dropTagged setting.
                 *
                 *  If the port is a member of this VID, changing the
                 *  filter does not have any effect upon it (and has the
                 *  convenient advantage of preserving any translation, so
                 *  we don't do anything in that case.
                 *
                 *  Also, we don't have any effect on high VSIs because
                 *  those are set up by other APIs and we don't want to
                 *  change them, so they're left alone.
                 */
                if (filter ||
                    BCM_PBMP_MEMBER(_vlan_state[unit]->tag_drop, port)) {
                    /*
                     *  Either filtering enabled or in drop tagged state;
                     *  We need to drop tagged frames.
                     */
                    fastVlan[vid]  = 0;
                } else { /* if (filter || tag_drop) */
                    /*
                     *  Both filtering and drop tagged are disabled; set
                     *  the VSI back to the expected one.
                     */
                    fastVlan[vid] = SBX_VSI_FROM_VID(vid);
                } /* if (filter) */
            } /* if (port is not member of this VLAN) */
        } /* for (vid = 1; vid <= _sbx_vlan_max_vid_value; vid++) */
    }

    if (BCM_SUCCESS(rv)) {
        rv = soc_sbx_g2p3_pv2e_vlan_fast_set(unit, 0, port, 0xFFF, port,
                                             NULL, /* set them all */
                                             fastVlan, BCM_VLAN_MAX + 1);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("failed to set pv2e[%d,*] vlan: %d %s\n"),
                      port, rv, bcm_errmsg(rv)));
        }
    }

    if (BCM_SUCCESS(rv)) {
        rv = soc_sbx_g2p3_p2appdata_get(unit, port, &p2appdata);
        if (BCM_SUCCESS(rv)) {
            /* okay; update state information to reflect new state */
            p2appdata.ifilteren = (0 != filter);
            rv = soc_sbx_g2p3_p2appdata_set(unit, port, &p2appdata);
        }

        if (BCM_FAILURE(rv)) {
            /* failed to read/write port state */
            VLAN_ERR((VLAN_FUNCMSG1("unable to update %d:p2appdata[%d]:"
                                    " %d (%s)\n"),
                      unit, port, rv, _SHR_ERRMSG(rv)));
        }
    }

    if (fastVlan) {
        sal_free(fastVlan);
    }
    if (fastMember) {
        sal_free(fastMember);
    }

    return rv;
}
#undef _VLAN_FUNC_NAME
#endif /* def BCM_FE2000_P3_SUPPORT */


/*
 *   Function
 *      _bcm_fe2000_vlan_port_ingress_filter_set
 *   Purpose
 *      Set the ingress filter setting for the specified port.
 *   Parameters
 *      (in) const int unit = unit whose port is to be checked
 *      (in) const bcm_port_t = port whose filter state is to be set
 *      (in) const int filter = new filter state for the port
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_port_ingress_filter_set"
int
_bcm_fe2000_vlan_port_ingress_filter_set(const int unit,
                                         const bcm_port_t port,
                                         const int filter)
{
    int rv;

    VLAN_VVERB((VLAN_FUNCMSG("(%d,%d,%s) enter\n"),
                unit, port, filter?"TRUE":"FALSE"));

    /* Check for proper initialisation */
    if ((!(_primary_lock)) || (!(_vlan_state[unit]))) {
        VLAN_ERR((VLAN_FUNCMSG1("unit %d does not have VLAN initialised\n"),
                  unit));
        return BCM_E_INIT;
    }

    /* make sure port is valid */
    if (!PBMP_MEMBER(PBMP_ALL(unit), port)) {
        /* port is not valid; set error now */
        VLAN_ERR((VLAN_FUNCMSG1("unit %d port %d is not valid\n"),
                  unit, port));
        return BCM_E_PARAM;
    } /* if (no error yet and port is not valid) */

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_vlan_port_ingress_filter_set(unit, port, filter);
        break;
#endif /* def BCM_FE2000_P3_SUPPORT */
    default:
        VLAN_ERR((VLAN_FUNCMSG1("unsupported ucode type: %d\n"),
                  SOC_SBX_CONTROL(unit)->ucodetype));
        rv = BCM_E_UNAVAIL;
    } /* switch (microcode) */

    VLAN_LOCK_RELEASE;

    VLAN_VVERB((VLAN_FUNCMSG("(%d,%d,%s) return %d (%s)\n"),
                unit, port, filter?"TRUE":"FALSE",
                rv, _SHR_ERRMSG(rv)));

    return rv;
}
#undef _VLAN_FUNC_NAME


/*
 *   Function
 *      _bcm_fe2000_g2p3_vlan_port_egress_filter_set
 *   Purpose
 *      Set the egress filter setting for the specified port.
 *   Parameters
 *      (in) const int unit = unit whose port is to be checked
 *      (in) const bcm_port_t = port whose filter state is to be set
 *      (in) const int filter = new filter state for the port
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 */
#ifdef BCM_FE2000_P3_SUPPORT
#define _VLAN_FUNC_NAME "_bcm_fe2000_g2p3_vlan_port_egress_filter_set"
int
_bcm_fe2000_g2p3_vlan_port_egress_filter_set(const int unit,
                                             const bcm_port_t port,
                                             const int filter)
{
    soc_sbx_g2p3_p2appdata_t  p2appdata;
    uint32_t                 *fastDrop;    /* epv2e.drop */
    int                      *fastDropSets;
    uint32_t                 *fastMember;  /* pv2appdata.member */
    bcm_vlan_t                vid;
    int                       rv = BCM_E_NONE;

    fastDrop = sal_alloc(sizeof(uint32_t) * (BCM_VLAN_MAX + 1),
                         "fastDrop");

    fastDropSets = sal_alloc(sizeof(int) * (BCM_VLAN_MAX + 1),
                         "fastDropSets");

    fastMember = sal_alloc(sizeof(uint32_t) * (BCM_VLAN_MAX + 1),
                           "fastMember");

    if (fastDrop == NULL || fastDropSets == NULL || fastMember == NULL) {
        if (fastDrop) {
            sal_free(fastDrop);
        }
        if (fastDropSets) {
            sal_free(fastDropSets);
        }
        if (fastMember) {
            sal_free(fastMember);
        }
        VLAN_ERR((VLAN_FUNCMSG1("failed to allocate work buffer\n")));
        return BCM_E_MEMORY;
    }
    sal_memset(fastDropSets, 0x00, sizeof(int) * (BCM_VLAN_MAX + 1));

    if (BCM_SUCCESS(rv)) {
        rv = soc_sbx_g2p3_pv2appdata_member_fast_get(unit, 0, port,
                                                     0xFFF, port,
                                                     fastMember,
                                                     BCM_VLAN_MAX + 1);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("failed to get pv2appdata[%d,*] member: "
                                    "%d %s\n"),
                      port, rv, bcm_errmsg(rv)));
        }
    }

    /* now set all egrPVid2Etc entries for this port appropriately */
    if (BCM_SUCCESS(rv)) {
        for (vid = 1; vid <= _sbx_vlan_max_vid_value; vid++) {
            /* for each VLAN, this port may need to be adjusted */

            if (0 == fastMember[vid]) {
                /* port isn't a member of this VLAN */
                /* need to adjust egrPVid2Etc for this port,VLAN */
                
                fastDrop[vid] = (0 != filter);
                fastDropSets[vid] = 1;
            }
        } /* for (vlan = 1; vlan < _vlan_max_vid_value; vlan++) */
    } /* if (BCM_E_NONE == result) */

    if (BCM_SUCCESS(rv)) {
        rv = soc_sbx_g2p3_epv2e_drop_fast_set(unit, 0, port, 0xFFF, port,
                                             fastDropSets, fastDrop,
                                              BCM_VLAN_MAX + 1);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("failed to get epv2e[%d,*] vlan: %d %s\n"),
                      port, rv, bcm_errmsg(rv)));
        }
    }

    /* if all that went well, then set the new state */
    if (BCM_SUCCESS(rv)) {
        rv = soc_sbx_g2p3_p2appdata_get(unit, port, &p2appdata);
        if (BCM_SUCCESS(rv)) {
            p2appdata.efilteren = (0 != filter);
            rv = soc_sbx_g2p3_p2appdata_set(unit, port, &p2appdata);
        }

        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to update %d:p2appdatat[%d]:"
                                    " %d (%s)\n"),
                      unit, port, rv, _SHR_ERRMSG(rv)));
        }
    }

    if (fastDrop) {
        sal_free(fastDrop);
    }
    if (fastDropSets) {
        sal_free(fastDropSets);
    }
    if (fastMember) {
        sal_free(fastMember);
    }

    return rv;
}
#undef _VLAN_FUNC_NAME
#endif /* def BCM_FE2000_P3_SUPPORT */


/*
 *   Function
 *      _bcm_fe2000_vlan_port_egress_filter_set
 *   Purpose
 *      Set the egress filter setting for the specified port.
 *   Parameters
 *      (in) const int unit = unit whose port is to be checked
 *      (in) const bcm_port_t = port whose filter state is to be set
 *      (in) const int filter = new filter state for the port
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_port_egress_filter_set"
int
_bcm_fe2000_vlan_port_egress_filter_set(const int unit,
                                        const bcm_port_t port,
                                        const int filter)
{
    int rv = BCM_E_NONE;

    VLAN_VVERB((VLAN_FUNCMSG("(%d,%d,%s) enter\n"),
                unit, port, filter?"TRUE":"FALSE"));

    /* Check for proper initialisation */
    if ((!(_primary_lock)) || (!(_vlan_state[unit]))) {
        VLAN_ERR((VLAN_FUNCMSG1("unit %d does not have VLAN initialised\n"),
                  unit));
        return BCM_E_INIT;
    }

    /* make sure port is valid */
    if (!PBMP_MEMBER(PBMP_ALL(unit), port)) {
        /* port is not valid; set error now */
        VLAN_ERR((VLAN_FUNCMSG1("unit %d port %d is not valid\n"),
                  unit, port));
        return BCM_E_PARAM;
    }

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_vlan_port_egress_filter_set(unit, port, filter);
        break;
#endif /* def BCM_FE2000_P3_SUPPORT */
    default:
        VLAN_ERR((VLAN_FUNCMSG1("unsupported ucode type: %d\n"),
                  SOC_SBX_CONTROL(unit)->ucodetype));
        rv = BCM_E_UNAVAIL;
    } /* switch (microcode) */

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_VVERB((VLAN_FUNCMSG("(%d,%d,%s) return %d (%s)\n"),
                unit, port, filter?"TRUE":"FALSE",
                rv, _SHR_ERRMSG(rv)));

    return rv;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_port_fetch
 *   Purpose
 *      Get the list of member ports for the specified VID.  The pbmp is the
 *      union of tagged and untagged ports in the VID; the ubmp is the
 *      untagged ports only.
 *   Parameters
 *      (in) sbG2Fe_t *pFe = FE handle
 *      (in) int unit = unit number whose VID is to have the ports list fetched
 *      (in) bcm_vlan_t = VID to which the ports are to ba added
 *      (out) pbmp_t pbmp = ptr to bitmap of ports
 *      (out) pbmp_t *ubmp = ptr to bitmap of untagged ports
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      ubmp will be a subset of pbmp.
 *      ubmp and pbmp are indeterminate on most errors.
 *      Does not check parameter validity
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_port_fetch"
int
_bcm_fe2000_vlan_port_fetch(int unit,
                            bcm_vlan_t vid,
                            pbmp_t* pbmp,
                            pbmp_t* ubmp)
{
    _bcm_sbx_vlan_ft_t    ft;                   /* working FT (flood) entry */
    bcm_port_t            pIndex;               /* working port number */
    int                   result;               /* local result code */
    int                   member;               /* whether port is a member */
    int                   untagged;             /* whether port is untagged */
    int                   vlanValid;            /* whether VLAN exists */

#if 0 
    if (_vlan_state[unit] && (!(_vlan_state[unit]->init))) {
        /* this is too verbose during init */
        VLAN_EVERB((VLAN_FUNCMSG("(%d,%04X,*,*) enter\n"),
                    unit,
                    vid));
    }
#endif 

    /* Check for proper initialisation */
    if ((!(_primary_lock)) || (!(_vlan_state[unit]))) {
        VLAN_ERR((VLAN_FUNCMSG1("unit %d does not have VLAN initialised\n"),
                  unit));
        return BCM_E_INIT;
    }


    /* make sure the arguments are valid */
    if ((!pbmp) || (!ubmp)) {
        /* NULL output pointers */
        VLAN_ERR((VLAN_FUNCMSG1("NULL pointers for output arguments\n")));
        return BCM_E_PARAM;
    }

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* from this point, hope for the best */
    result = _bcm_fe2000_vlan_check(unit, vid, &vlanValid, &ft);
    if (BCM_E_NONE == result) {
        if (!vlanValid) {
            result = BCM_E_NOT_FOUND;
            /*
             *  This message is just obnoxious.  Since this function is called
             *  not only to determine port membership but also to determine the
             *  existence of a VLAN, it's really annoying for things such as
             *  the VLAN SHOW command to dump over four thousand of these.
             */
#if 0 
            VLAN_ERR((VLAN_FUNCMSG1("VLAN %04X does not exist"
                                    " on unit %d\n"),
                      vid,
                      unit));
#endif 
        }
    } /* if (BCM_E_NONE == result) */

    /* go find the ports participating in this VLAN */
    if (BCM_E_NONE == result) {
        /* Clear provided pbmps */
        SOC_PBMP_CLEAR(*pbmp);
        SOC_PBMP_CLEAR(*ubmp);
        /* initialise these variables (though only used if written) */
        member = FALSE;
        untagged = FALSE;
        /* Scan for members and fill them in */
        PBMP_ALL_ITER(unit, pIndex) {
            /* now get the port's membership/untagged status on this VLAN */
            result = _bcm_fe2000_vlan_port_is_member(unit,
                                                     vid,
                                                     pIndex,
                                                     &member,
                                                     &untagged);
            if (BCM_E_NONE == result) {
                /* got the status; use it */
                if (member) {
                    /* port is a member */
                    SOC_PBMP_PORT_ADD(*pbmp, pIndex);
                    /* only check untagged if a member */
                    if (untagged) {
                        /* port is untagged */
                        SOC_PBMP_PORT_ADD(*ubmp, pIndex);
                    }
                } /* if (member) */
            } else { /* if (BCM_E_NONE == result) */
                VLAN_ERR((VLAN_FUNCMSG1("unable to query unit %d port %d"
                                        " membership in VLAN %04X: %d (%s)\n"),
                          unit,
                          pIndex,
                          vid,
                          result,
                          _SHR_ERRMSG(result)));
            } /* if (BCM_E_NONE == result) */
        } /* PBMP_ALL_ITER(unit,pIndex) */
    } /* if (BCM_E_NONE == result) */

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

#if 0 
    if (_vlan_state[unit] && (!(_vlan_state[unit]->init))) {
        /* this is too verbose during init */
        VLAN_EVERB((VLAN_FUNCMSG("(%d,%04X,&("
                                 VLAN_PBMP_FORMAT
                                 "),&("
                                 VLAN_PBMP_FORMAT
                                 ")) return %d (%s)\n"),
                    unit,
                    vid,
                    VLAN_PBMP_SHOW(*pbmp),
                    VLAN_PBMP_SHOW(*ubmp),
                    result,
                    _SHR_ERRMSG(result)));
    }
#endif 

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_list_fetch
 *   Purpose
 *      Get a list of the VIDs that exist on this unit, that maybe include any
 *      of the specified ports (or perhaps that merely exist).
 *   Parameters
 *      (in) int unit = unit number whose VID list is to be fetched
 *      (in) pbmp_t pbmp = ports to consider
 *      (out) bcm_vlan_data_t **listp = pointer to variable for address of list
 *      (out) int *countp = pointer to variable for number of list elements
 *      (in) int chkport = nonzero to check ports list; zero to skip ports list
 *   untagged ports Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      Only locks the table while processing a VID (allows others access).
 *      Allocates maximum storage to avoid saving state internally
 *      but returns correct number of entries.  The list destroy function can
 *      still free the list correctly.
 *      Always sets list pointer and count to sensible values -- in case of
 *      error or early bailout, they will be NULL and zero respectively.
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_list_fetch"
int
_bcm_fe2000_vlan_list_fetch(int unit,
                            pbmp_t pbmp,
                            bcm_vlan_data_t **listp,
                            int *countp,
                            int chkPort)
{
    bcm_vlan_data_t       *wList;           /* working list pointer */
    pbmp_t                wBmp;             /* working port bitmap */
    unsigned int          vIndex;           /* VID index */
    unsigned int          vCount;           /* VID count in array */
    unsigned int          vSize;            /* VID info array size */
    int                   result;           /* local result code */
    int                   rv;               /* called function result code */

    VLAN_VERB((VLAN_FUNCMSG("(%d,"
                            VLAN_PBMP_FORMAT
                            ",*,*,%s) enter\n"),
               unit,
               VLAN_PBMP_SHOW(pbmp),
               chkPort?"TRUE":"FALSE"));

    /* check for valid parameter values */
    if ((!(listp)) || (!(countp))) {
        VLAN_ERR((VLAN_FUNCMSG1("NULL pointers for outbound arguments\n")));
        return BCM_E_PARAM;
    }

    /* Make sure these values are reasonable in case of early bailout */
    *listp = NULL;
    *countp = 0;

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* Allocate memory for the list */
    vSize = BCM_VLAN_COUNT;
    wList = sal_alloc(sizeof(bcm_vlan_data_t) * (vSize + 1),
                      "bcm_fe2000_vlan_data_t_array");
    if (!wList) {
        /* Allocation failed */
        VLAN_ERR((VLAN_FUNCMSG1("unable to alloc %d bytes for"
                                " unit %d vlan list\n"),
                  sizeof(bcm_vlan_data_t) * (vSize + 1),
                  unit));
        return BCM_E_MEMORY;
    }

    /* Optimistically assume success from here */
    result = BCM_E_NONE;

    /* Collect the VID information */
    for (vIndex = 1, vCount = 0;
         (_sbx_vlan_max_vid_value >= vIndex) && (BCM_E_NONE == result);
         vIndex++) {

        /* We still have room in the array; keep going */
        /* Get the VID information (including ports) */
        rv = _bcm_fe2000_vlan_port_fetch(unit,
                                         vIndex,
                                         &(wList[vCount].port_bitmap),
                                         &(wList[vCount].ut_port_bitmap));
        switch (rv) {
        case BCM_E_NONE:
            /* Check this entry's ports list, if needed */
            if (chkPort) {
                /* Need to check for selected port(s) on this VID */
                BCM_PBMP_ASSIGN(wBmp,pbmp);
                BCM_PBMP_AND(wBmp,wList[vCount].port_bitmap);
                if (BCM_PBMP_IS_NULL(wBmp)) {
                    /* None of the requested ports in this VID */
                    /* Go to the next one without commit */
                    break;
                }
            }
            /* Commit this VID to the list */
            wList[vCount].vlan_tag = vIndex;
            /* Advance to next list member */
            vCount++;
            break;
        case BCM_E_NOT_FOUND:
            /* This VID was not found; move to the next one */
            break;
        default:
            /* Something unexpected happened... */
            result = rv;
            VLAN_ERR((VLAN_FUNCMSG1("_bcm_fe2000_vlan_port_fetch"
                                    "(%d,%04X,*,*) returned %d (%s)\n"),
                      unit,
                      vIndex,
                      rv,
                      _SHR_ERRMSG(result)));
        } /* switch (rv) */
    } /* for (vIndex = 0, vCount = 0; ... ; vIndex++) */

    /* Clean up in case or error, or tidy up when no error */
    if (BCM_E_NONE != result) {
        /* Error occurred; dump the array and set count to zero */
        sal_free(wList);
        *countp = 0;
        *listp = NULL;
    } else {
        /* No error; pass the proper data back */
        sal_memset(&(wList[vCount]),
                   0,
                   sizeof(bcm_vlan_data_t) * (vSize - vCount + 1));
        *countp = vCount;
        *listp = wList;
    }

    VLAN_VERB((VLAN_FUNCMSG("(%d,"
                            VLAN_PBMP_FORMAT
                            ",*,&(%d),%s) return %d (%s)\n"),
               unit,
               VLAN_PBMP_SHOW(pbmp),
               *countp,
               chkPort?"TRUE":"FALSE",
               result,
               _SHR_ERRMSG(result)));

    return result;
}
#undef _VLAN_FUNC_NAME

#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_detach"
int
bcm_fe2000_vlan_detach(int unit)
{

    if (sal_mutex_take(_primary_lock, sal_mutex_FOREVER)) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to take primary lock\n")));
        return BCM_E_INTERNAL;
    }

    if (_vlan_state[unit] != NULL) {
        sal_mutex_destroy(_vlan_state[unit]->lock);
        _vlan_state[unit]->lock = NULL;
        sal_free(_vlan_state[unit]);
        _vlan_state[unit] = NULL;
    }

    if (sal_mutex_give(_primary_lock)) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to release primary lock\n")));
        return BCM_E_INTERNAL;
    }

    return BCM_E_NONE;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_gport_targets_recover
 *   Purpose
 *      Recover the switch port target information from a recovered vlan gport.
 *   Parameters
 *      (in)  unit - bcm device number
 *   Returns
 *     BCM_E_*
 *   Notes
 *     Must be called after the application has established the fabric mapping.
 *     This mapping is used to convert the fabric node/port to a switch port
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_gport_targets_recover"
STATIC int
_bcm_fe2000_vlan_gport_targets_recover(int unit)
{
    int                      rv = BCM_E_NONE;
    _bcm_fe2k_vlan_gport_t  *gp_ptr;
    bcm_gport_t              switch_gport, fabric_gport;
    int                      lpi;

    for (lpi = SBX_MAX_PORTS; lpi <= SBX_LPORT_END; lpi++) {

        if (SBX_LPORT_TYPE(unit, lpi) == BCM_GPORT_VLAN_PORT) {
            gp_ptr = (_bcm_fe2k_vlan_gport_t*)SBX_LPORT_DATAPTR(unit, lpi);
            if (gp_ptr == NULL) {
                return BCM_E_INTERNAL;
            }

            VLAN_VERB((VLAN_FUNCMSG1("Second pass recovery of vlan gport: "
                                     "0x%08x\n"),
                       gp_ptr->p.vlan_port_id));
            
            /* Convert the fabric node/port found in the first pass to
             * a front panel mod/port
             */
            fabric_gport = gp_ptr->p.port;
            rv = bcm_sbx_stk_fabric_map_get_switch_port(unit, fabric_gport,
                                                        &switch_gport);
            if (BCM_FAILURE(rv)) {
                VLAN_ERR((VLAN_FUNCMSG1("Failed to convert fabric gport "
                                        "%d:%d: %s\n"),
                          BCM_GPORT_MODPORT_MODID_GET(gp_ptr->p.port),
                          BCM_GPORT_MODPORT_PORT_GET(gp_ptr->p.port),
                          bcm_errmsg(rv)));
                return rv;
            }

            gp_ptr->p.port = switch_gport;

            VLAN_VERB((VLAN_FUNCMSG1("Mapped fabric port %d:%d to "
                                     "switch port %d:%d\n"),
                       BCM_GPORT_MODPORT_MODID_GET(fabric_gport),
                       BCM_GPORT_MODPORT_PORT_GET(fabric_gport),
                       BCM_GPORT_MODPORT_MODID_GET(switch_gport),
                       BCM_GPORT_MODPORT_PORT_GET(switch_gport)));
        }
    }

    return rv;
}
#undef _VLAN_FUNC_NAME


/*
 *   Function
 *      _bcm_fe2000_vlan_gport_recover
 *   Purpose
 *      Recover a single vlan gport
 *   Parameters
 *      (in)  unit - bcm device number
 *      (in)  port - physical port of vlan gport
 *      (in)  vid  - match vid of vlan gport
 *      (in)  lpi  - logical port index
 *      (in)  vsi  - vsi
 *      (in)  lp   - recovered logical port
 *   Returns
 *     BCM_E_*
 *     BCM_E_EXSITS - if GPORT has been previously reserved.
 *   Notes
 *     Currently only installed local vlan gports are supported.  Installed 
 *   meaning the gport has been added to a VSI on the local unit.
 */
#ifdef BCM_WARM_BOOT_SUPPORT
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_gport_recover"
STATIC int
_bcm_fe2000_vlan_gport_recover(int unit, bcm_port_t port, bcm_vlan_t vid,
                               uint32 lpi, uint32 vsi, soc_sbx_g2p3_lp_t *lp)
{
    _bcm_fe2k_vlan_gport_t *gp_ptr;
    int                     rv;
    soc_sbx_g2p3_ft_t       fte;
    soc_sbx_g2p3_oi2e_t     oi;
    soc_sbx_g2p3_eteencap_t etee;
    soc_sbx_g2p3_etel2_t    etel;
    int                     fab_node, fab_port;
    uint32                  gportId, gu2_res, gu2_flags;

    gu2_res = SBX_GU2K_USR_RES_FTE_LOCAL_GPORT;
    gu2_flags = _SBX_GU2_RES_FLAGS_RESERVE;
    
    
    VLAN_VERB((VLAN_FUNCMSG1("Recovering VLAN GPORT on port,"
                             "vid %d, 0x%04x; ft 0x%05x "
                             "lpi 0x%05x\n"),
               port, vid, lp->pid, lpi));
    
    rv = _sbx_gu2_resource_alloc(unit, gu2_res, 1, &lp->pid, gu2_flags);

    if (rv == BCM_E_EXISTS || rv == BCM_E_BUSY) {
        /* multple port vids may be represented by the same
         * gport.  Not an error, continue to next port,vid
         */
        VLAN_VERB((VLAN_FUNCMSG1("Repeat, skipping...\n")));
        return BCM_E_EXISTS;
    }

    /* Don't need to reserve the LP; the recovery module took
     * care of lp reservation at warm boot init
     */
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("Failed to reserve gport "
                                "0x%05x: %s\n"), 
                  lp->pid, bcm_errmsg(rv)));
        return rv;
    }

    /* Read the egress path from hardware, reconstruct the soft state
     */
    rv = soc_sbx_g2p3_ft_get(unit, lp->pid, &fte);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("Failed to read ft[0x%06x]: %s\n"),
                  lp->pid, bcm_errmsg(rv)));
        return rv;
    }

    rv = soc_sbx_g2p3_oi2e_get(unit, fte.oi - SBX_RAW_OHI_BASE, &oi);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("Failed to read oi[0x%06x]: %s\n"),
                  fte.oi - SBX_RAW_OHI_BASE, bcm_errmsg(rv)));
        return rv;
    }

    rv = soc_sbx_g2p3_eteencap_get(unit, oi.eteptr, &etee);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("Failed to read eteencap[0x%06x]: %s\n"),
                  oi.eteptr, bcm_errmsg(rv)));
        return rv;
    }
    
    rv = soc_sbx_g2p3_etel2_get(unit, etee.l2ete, &etel);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("Failed to read etel2[0x%06x]: %s\n"),
                  etee.l2ete, bcm_errmsg(rv)));
        return rv;
    }

    gp_ptr = sal_alloc(sizeof(*gp_ptr), "vlan_port_data");
    if (gp_ptr == NULL) {
        VLAN_ERR((VLAN_FUNCMSG1("Failed to to allocate memory for gport\n")));
        return BCM_E_MEMORY;
    }

    SOC_SBX_NODE_PORT_FROM_QID(unit, fte.qid, fab_node, fab_port, NUM_COS(unit));
    fab_node += SBX_QE_BASE_MODID;

    /* Store the FABRIC node and FABRIC port during the recovery process.
     * The expectation is the user **MUST** call vlan_init again *after*
     * the system's modmap is established (forcemodmap; or
     * bcm_sbx_stk_fabric_map) with warmboot still enabled; the secondary
     * pass can determine the target FE module and FE port.
     */
    BCM_GPORT_MODPORT_SET(gp_ptr->p.port, fab_node, fab_port);

    BCM_GPORT_VLAN_PORT_ID_SET(gp_ptr->p.vlan_port_id,
                               VLAN_FT_INDEX_TO_VGPORT_ID(unit, lp->pid));

    
    gp_ptr->p.criteria   = BCM_VLAN_PORT_MATCH_PORT_VLAN;
    gp_ptr->p.encap_id   = SOC_SBX_ENCAP_ID_FROM_OHI(fte.oi);
    gp_ptr->p.vsi        = vsi;
    gp_ptr->p.match_vlan = vid;
    gp_ptr->p.policer_id = lp->policer;
    gp_ptr->p.qos_map_id = lp->qos;
    gp_ptr->p.egress_vlan = etel.usevid ? etel.vid : 0;

    VLAN_VERB((VLAN_FUNCMSG1("Recovering vlan gport 0x%08x:\n"), 
               gp_ptr->p.vlan_port_id));
    VLAN_VERB((VLAN_FUNCMSG1("ohi=0x%04x encap=0x%08x fabNode=%d fabPort=%d\n"),
               fte.oi, SOC_SBX_ENCAP_ID_FROM_OHI(fte.oi),
               fab_node, fab_port));
    VLAN_VERB((VLAN_FUNCMSG1("vsi=0x%03x, policer=0x%04x qos_map=0x%x\n"),
               vsi, lp->policer, lp->qos));

    gportId = VLAN_FT_INDEX_TO_VGPORT_ID(unit, lp->pid);
    _vlan_state[unit]->gportInfo.lpid[gportId] = lpi;

    SBX_LPORT_DATAPTR(unit, lpi) = (void*)gp_ptr;
    SBX_LPORT_TYPE(unit, lpi)    = BCM_GPORT_VLAN_PORT;

    return rv;
}
#undef _VLAN_FUNC_NAME
#endif /* BCM_WARM_BOOT_SUPPORT */

/*
 *   Function
 *      _bcm_fe2000_vlan_recover
 *   Purpose
 *      warm boot vlan module
 *   Parameters
 *      (in) unit = bcm unit
 *   Returns
 *     BCM_E_*
 *   Notes
 *     Recover internal state from harware 
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_recover"
STATIC int
_bcm_fe2000_vlan_recover(int unit) 
{
#ifdef BCM_WARM_BOOT_SUPPORT
#ifdef BROADCOM_DEBUG
    sal_usecs_t    t0, t1;
#endif
    int                   rv;
    uint32               *fast_vlan, *fast_lpi;
    int                   vid, port;
    uint32                tmp;
    int                   discard_tag, discard_untag;
    int                   base_idx, fast_idx;
    soc_sbx_g2p3_pvv2e_t  pvv2e;
    soc_sbx_g2p3_lp_t     lp;
    _bcm_fe2k_nvid_pvv2e_control_flags_t flags;

    VLAN_VERB((VLAN_FUNCMSG("(%d) enter\n"), unit));

#ifdef BROADCOM_DEBUG
    t0 = sal_time_usecs();
#endif
    
    rv = _bcm_fe2000_recovery_var_get(unit, fe2k_rcvr_default_vid, &tmp);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("failed to recover default vid: %s\n"),
                  bcm_errmsg(rv)));
        return rv;
    }

    _vlan_state[unit]->default_vid = (bcm_vlan_t)tmp;

    fast_vlan = _bcm_fe2000_recovery_pv2e_vlan_get(unit);
    fast_lpi  = _bcm_fe2000_recovery_pv2e_lpi_get(unit);

    PBMP_ALL_ITER(unit, port) {
        base_idx = port * BCM_VLAN_COUNT;

        /* Check some vids to determine port discard mode. */
        discard_untag = (fast_vlan[base_idx] == 0);
        discard_tag = 0;

        for (vid = 1; vid < BCM_VLAN_MAX; vid++) {
            fast_idx = base_idx + vid;
            if (fast_vlan[fast_idx] == 0) {
                discard_tag++;
            }

            if (fast_vlan[fast_idx] > BCM_VLAN_MAX) {
                /* Reduce the number of passes over the port,vid table - store
                 * unhandled VSIs with the recovery module, of course this assumes
                 * the vlan module recovery is called before any other module
                 * interested in VSIs.  Do we need to verify this is the case?? 
                 */
                _bcm_fe2000_recovery_non_tb_vsi_insert(unit, port, vid);

            } else if (fast_lpi[fast_idx]) {

                /* Found a Traditionally bridged VSI, with a logical port; 
                 * check to see if it is a vlan gport by examining the PID
                 */
                rv = soc_sbx_g2p3_lp_get(unit, fast_lpi[fast_idx], &lp);
                if (BCM_FAILURE(rv)) {
                    VLAN_ERR((VLAN_FUNCMSG1("Failed to read LP 0x%04x: %s\n"),
                              fast_lpi[fast_idx], bcm_errmsg(rv)));
                    return rv;
                }
                
                /* vlan gports are found in the local gport space - 
                 * Only local vlan gports will be found here; a level 2
                 * recovery is required for remote gports or application
                 * reconfiguration
                 */
                if (lp.pid >= SBX_LOCAL_GPORT_FTE_BASE(unit) && 
                    lp.pid <= SBX_LOCAL_GPORT_FTE_END(unit)) {

                    /* allocate gport and gather all the data neccessary to 
                     * reconstruct a  vlan port, and commit to internal state
                     */
                    rv = _bcm_fe2000_vlan_gport_recover(unit, port, vid, 
                                                        fast_lpi[fast_idx],
                                                        fast_vlan[fast_idx],
                                                        &lp);

                    if (rv == BCM_E_EXISTS) {
                        /* multiple port,vids may map to the same gport; 
                         * not an error
                         */
                        continue;
                    }
                    if (BCM_FAILURE(rv)) {
                        return rv;
                    }
                }
            }
        }

        /* Set the proper tag drop mode bitmaps */
        if (discard_tag) {
            if (discard_tag < VLAN_WB_NUM_VID_SCAN) {
                /* This condition *may* be okay if there are gports present
                 * on the port.  Gports are not cleared when the port mode is
                 * set to drop_tag
                 */
                VLAN_WARN((VLAN_FUNCMSG("Recovering port %d as "
                                        "'BCM_PORT_DISCARD_TAG', but not all "
                                        "VIDs support the finding. (dt=%d)\n"),
                           port, discard_tag));
            }
            BCM_PBMP_PORT_ADD(_vlan_state[unit]->tag_drop, port);
        }

        /* If the port is found to be in discard untag mode, the native vid
         * really has no meaning anyway.  Before or after a recovery, the
         * the application *should* define the untagged vlan when switching
         * port discard modes from drop_untag to !drop_untag
         */
        if (discard_untag) {
            BCM_PBMP_PORT_ADD(_vlan_state[unit]->untag_drop, port);
        } else {
            _vlan_state[unit]->nvid[port] = fast_vlan[base_idx];
        }

        /* recover nvflag attributes */
        rv = soc_sbx_g2p3_pvv2e_get(unit, 0xFFF, 0x000, port, &pvv2e);
        if (BCM_SUCCESS(rv)) {
            flags = 0;
            flags |= pvv2e.vid;

            if (pvv2e.vid != _vlan_state[unit]->nvid[port]) {
                flags |= BCM_FE2K_NVID_USE_NVID_OVR;
            }

            flags |= BCM_FE2K_NVID_USE_PVV2E;
            flags |= BCM_FE2K_NVID_OVERRIDE_PVV2E;
            flags |= (pvv2e.replace) ? BCM_FE2K_NVID_SET_REPLACE : 0;
            flags |= (pvv2e.keeporstrip) ? BCM_FE2K_NVID_SET_KEEPORSTRIP : 0;
            flags |= (pvv2e.untagged_strip) ? 
                BCM_FE2K_NVID_SET_UNTAGGEDSTRIP : 0;
            _vlan_state[unit]->nvflag[port] = flags;

            VLAN_VERB((VLAN_FUNCMSG1("Found a valid pvv2e entry on [%d,0,0xfff]\n"),
                       port));

        } else if ((rv != BCM_E_NOT_FOUND) && BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("pvv2e_get failed: %s\n"),
                      bcm_errmsg(rv)));
            break;
        }
        rv = BCM_E_NONE;
        
    }
    
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    /* done initializing */
    _vlan_state[unit]->init = FALSE;

    /* TODO: _bcm_fe2k_vgp_info_t      gportInfo; */


#ifdef BROADCOM_DEBUG
    t1 = sal_time_usecs();
    t0 = SAL_USECS_SUB(t1, t0);
    VLAN_OUT(0, (VLAN_FUNCMSG1("Recovery Data time = %d uS (%d S, %d uS)\n"),
                 t0, (int)(t0 / 1e6), (int)(t0 - (int)((int)(t0 / 1e6)*1e6))));

    if (BCM_DEBUG_CHECK (BCM_DBG_VLAN)) {
        _bcm_fe2000_vlan_dump_state(unit);
    }
#endif
    
    return rv;
#else  /* BCM_WARM_BOOT_SUPPORT */
    return BCM_E_CONFIG;
#endif /* BCM_WARM_BOOT_SUPPORT */
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_default_set
 *   Purpose
 *      Set the default vid
 *   Parameters
 *      unit - bcm device number
 *      vid  - default vid
 *   Returns
 *      BCM_E_*
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_default_set"

static int
_bcm_fe2000_vlan_default_set(int unit, bcm_vlan_t vid)
{
    int rv = BCM_E_NONE;
#ifdef BCM_WARM_BOOT_SUPPORT
    rv = _bcm_fe2000_recovery_var_set(unit, fe2k_rcvr_default_vid, vid);
#endif
    _vlan_state[unit]->default_vid = vid;
    return rv;
}
#undef _VLAN_FUNC_NAME


/*
 *   Function
 *      bcm_fe2000_vlan_init
 *   Purpose
 *      Initialise the VLAN subsystem on the specified unit.
 *   Parameters
 *      (in) int unit = unit number whose VLAN suport is to be initialised
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This sets up the system with all of the basic VLAN initial status.
 *      This includes: default VID = INIT_DEFAULT_VID (compile time, above);
 *      unknown multicast flooding is set based upon BCM_MCAST_FLOOD_DEFAULT
 *      (compile time flag).  For now, we ignore BCM_VLAN_NO_AUTO_STACK.
 *      Assumes VLAN:VID mapping is 1:1.  Does not return an error if it is
 *      called multiple times against the same unit; will perform full init
 *      each time (except that it only allocates the resources once).
 */

#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_init"
int
bcm_fe2000_vlan_init(int unit)
{
#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_ft_t        p3ft;              /* G2P3 FT (flooding) ent */
    soc_sbx_g2p3_v2e_t       p3vlan2Etc;        /* G2P3 vlan2Etc entry */
    soc_sbx_g2p3_p2appdata_t p3p2appdata;       /* G2P3 p2appdata entry */
#endif /* def BCM_FE2000_P3_SUPPORT */
    sal_mutex_t              tempLock;          /* temporary lock */
    int                      result;            /* local result code */
    int                      auxres;            /* spare result code */
    bcm_pbmp_t               vlanPbmp;          /* ports on the unit */
    bcm_pbmp_t               ep2ePbmp;          /* egrPort2Etc init ports */
    bcm_pbmp_t               p2ePbmp;           /* port2Etc init port */
    bcm_stg_t                stg;               /* work spanning tree group*/
    unsigned int             vIndex;            /* work VID index */
    bcm_port_t               pIndex;            /* work port index */
    bcm_pbmp_t               default_pbmp;      /* Implicitly in default VID */
    uint32                   vlan_base_ft;      /* flooding FTE base */
    int                      idx;

    VLAN_VERB((VLAN_FUNCMSG("(%d) enter\n"), unit));

    if (SAL_BOOT_BCMSIM) {
        VLAN_VERB(("running on simulator; performing additional setup\n"));

        /* a number of things that may be helpful to do on the simulator... */
        _sbx_vlan_max_vid_value = VLAN_MAX_VID_VALUE_SIM; /* limit max VID */

        

        
        running_simulator = TRUE; 

        /* get QE 'node', but assumes FE unit is 1 above its QE unit */
        result = bcm_stk_my_modid_get(unit - 1, (int*)&vIndex);

        if (BCM_E_NONE != result) {
            /* another hack!?!? */
            vIndex = 10000; /* just assume 10000 */
            VLAN_ERR((VLAN_FUNCMSG1("init unit %d: bcm_stk_my_modid_get(%d,*)"
                                    " returned %d (%s); assuming QE=%d\n"),
                      unit,
                      unit - 1,
                      result,
                      _SHR_ERRMSG(result),
                      vIndex));
            result = BCM_E_NONE;
        }
        if (BCM_E_NONE == result) {
            /* get FE 'node' (or is that not same as module?) */
            result = bcm_stk_my_modid_get(unit, &auxres);
            if (BCM_E_NONE == result) {
                VLAN_VERB((VLAN_FUNCMSG1("assuming FE unit %d attached to"
                                         " QE unit %d\n"),
                           unit,
                           unit - 1));
                VLAN_VERB((VLAN_FUNCMSG1("assuming FE unit %d is module %d\n"),
                           unit,
                           auxres));
                VLAN_VERB((VLAN_FUNCMSG1("assuming QE unit %d is module %d\n"),
                           unit - 1,
                           vIndex));
                PBMP_ALL_ITER(unit, pIndex) {
                    ((soc_sbx_control_t *)(soc_control[unit]->drv))->
                        fabric_units[pIndex] = (sbhandle)(unit - 1);
                    ((soc_sbx_control_t *)(soc_control[unit]->drv))->
                        modport[auxres][pIndex] = (vIndex << 16) | pIndex;
                    VLAN_VVERB((VLAN_FUNCMSG1("assuming FE unit %d mod %d port"
                                              " %d is attached to QE unit %d"
                                              " mod %d port %d modport %d\n"),
                                unit,
                                auxres,
                                pIndex,
                                (unsigned int)(((soc_sbx_control_t *)
                                                (soc_control[unit]->drv))->
                                               fabric_units[pIndex]),
                                vIndex,
                                pIndex,
                                ((soc_sbx_control_t *)(soc_control[unit]->drv))
                                    ->modport[auxres][pIndex]));
                }
            } else {
                VLAN_ERR((VLAN_FUNCMSG1("unable to get FE (unit %d) module ID:"
                                        " %d (%s)\n"),
                          unit,
                          result,
                          _SHR_ERRMSG(result)));
            }
        } else {
            VLAN_ERR((VLAN_FUNCMSG1("unable to get FE unit %d assumed QE"
                                    " (unit %d) module ID: %d (%s)\n"),
                      unit,
                      unit - 1,
                      result,
                      _SHR_ERRMSG(result)));
        }
        if (BCM_E_NONE != result) {
            VLAN_ERR((VLAN_FUNCMSG1("abandoning unit %d initialisation due to"
                                    " inability to set up MVT entry data.\n"),
                      unit));
            return result;
        }
    }

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for (maybe perform) primary lock initialisation */
    tempLock = NULL;

    if (_vlan_state[unit] != NULL) {
        /* Must perform a two-pass init on vlan warmboot to determine
         * vlan gport targetting information
         */
        if (SOC_WARM_BOOT(unit)) {
            VLAN_LOCK_TAKE;
            result = _bcm_fe2000_vlan_gport_targets_recover(unit);
            VLAN_LOCK_RELEASE;
            VLAN_VERB((VLAN_FUNCMSG("(%d) return %d (%s)\n"),
                       unit, result, _SHR_ERRMSG(result)));
            return result;
        }

        result = bcm_fe2000_vlan_detach(unit);
        if (BCM_FAILURE(result)) {
            VLAN_ERR((VLAN_FUNCMSG1("vlan detach failed on unit %d with %d\n"),
                      unit, result));
            return result;
        }
    }

    if (!(_primary_lock)) {
        /* Primary lock not initialised; set it up */
        tempLock = sal_mutex_create("_bcm_fe2000_vlan_primary_lock");
        /* Return an error if that failed */
        if (!(tempLock)) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to create global lock during"
                                    " unit %d init\n"),
                     unit));
            return BCM_E_RESOURCE;
        }
        /* now try to claim it */
        if (sal_mutex_take(tempLock, sal_mutex_FOREVER)) {
            /* could not obtain the temporary lock */
            VLAN_ERR((VLAN_FUNCMSG1("unable to take temporary global lock"
                                    " during unit %d init\n"),
                      unit));
            sal_mutex_destroy(tempLock);
            return BCM_E_INTERNAL;
        }
        /* write the lock value to the global */
        _primary_lock = tempLock;
        /* This must be zeroed on initialisation */
        sal_memset(&(_vlan_state[0]),0,sizeof(_vlan_state));
        /* finally release the temporary lock */
        if (sal_mutex_give(tempLock)) {
            /* could not release temporary lock */
            VLAN_ERR((VLAN_FUNCMSG1("unable to release temporary global lock"
                                    " during unit %d init\n"),
                      unit));
            sal_mutex_destroy(tempLock);
            return BCM_E_INTERNAL;
        }
    }

    /* Optimistically hope all will go well */
    result = BCM_E_NONE;

    /* Check for (maybe perform) unit data structure initialisation */
    if (sal_mutex_take(_primary_lock, sal_mutex_FOREVER)) {
        /* Could not obtain primary lock */
        VLAN_ERR((VLAN_FUNCMSG1("unable to take global lock during"
                                " unit %d init\n"),
                  unit));
        return BCM_E_INTERNAL;
    }
    if (tempLock && (_primary_lock != tempLock)) {
        /* somebody else collided during init; free our to-be-leaked lock */
        VLAN_WARN((VLAN_FUNCMSG1("init collision; destroying temporary global"
                                 " lock created by unit %d init\n"),
                   unit));
        sal_mutex_destroy(tempLock);
    }
    if (!(_vlan_state[unit])) {
        /* Allocate the space for this unit's state information */
        VLAN_VVERB((VLAN_FUNCMSG1("allocate %d bytes for unit %d state\n"),
                    sizeof(_bcm_sbx_fe2000_vlan_state_t),
                    unit));
        _vlan_state[unit] = sal_alloc(sizeof(_bcm_sbx_fe2000_vlan_state_t),
                                      "_bcm_fe2000_vlan_state");
        if (_vlan_state[unit]) {
            /* Alloc success; try to fill in the space */
            sal_memset(_vlan_state[unit],
                       0,
                       sizeof(_bcm_sbx_fe2000_vlan_state_t));
            _vlan_state[unit]->lock = sal_mutex_create("_bcm_vlan_state_lock");
            if (_vlan_state[unit]->lock == NULL) {
                /* failed to create lock; throw it out & return error */
                result = BCM_E_RESOURCE;
                sal_free(_vlan_state[unit]);
                _vlan_state[unit] = NULL;
            }
        } else {
            /* Alloc fail; return an appropriate result (later) */
            VLAN_ERR((VLAN_FUNCMSG1("unable to allocate %d bytes for unit %d"
                                    "state information\n"),
                      (sizeof(_bcm_sbx_fe2000_vlan_state_t) +
                       (sizeof(bcm_port_t) * (MAX_PORT(unit)+1))),
                      unit));
            result = BCM_E_MEMORY;
        }
    }
    if (sal_mutex_give(_primary_lock)) {
        /* Could not release primary lock */
        VLAN_ERR((VLAN_FUNCMSG1("unable to release global lock during unit"
                                " %d init\n"),
                  unit));
        return BCM_E_INTERNAL;
    }
    if (BCM_E_NONE != result) {
        /* Something went wrong above; return the result */
        VLAN_VERB((VLAN_FUNCMSG("(%d) early return %d (%s)\n"),
                   unit,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* indicate initialisation pending */
    VLAN_VVERB((VLAN_FUNCMSG1("begin unit %d initialisation\n"), unit));
    _vlan_state[unit]->init = TRUE;

    /*
     *  Figure out, from the ports that exist, and the restrictions placed
     *  at compile time, which ports are implicitly in the default VID.
     *  This assumes we want nothing in the default VLAN if we're told to
     *  exclude ETHER and CPU, and includes only ETHER and CPU conditional
     *  to not being told to exclude them.
     */
    SOC_PBMP_CLEAR(default_pbmp);
#ifndef BCM_VLAN_NO_DEFAULT_ETHER
    /* We don't want to disable ethernet ports; include them */
    SOC_PBMP_OR(default_pbmp, PBMP_E_ALL(unit));
#endif /* ndef BCM_VLAN_NO_DEFAULT_ETHER */
#ifndef BCM_VLAN_NO_DEFAULT_CPU
    /* We don't want to disable CPU ports; include them */
    SOC_PBMP_OR(default_pbmp, PBMP_CMIC(unit));
#endif /* ndef BCM_VLAN_NO_DEFAULT_CPU */
#ifndef BCM_VLAN_NO_DEFAULT_SPI_SUBPORT
    /* We don't want to disable SPI subports; include them */
    SOC_PBMP_OR(default_pbmp, PBMP_SPI_SUBPORT_ALL(unit));
#endif /* ndef BCM_VLAN_NO_DEFAULT_SPI_SUBPORT */

    /* HiGig ports have membership in default vlan */
    SOC_PBMP_OR(default_pbmp, PBMP_HG_ALL(unit));

    /* do preliminary setup (get handles, &c) */
    if (SOC_IS_SBX_G2P3(unit)) {
#ifdef BCM_FE2000_P3_SUPPORT
        /* only really need this for negative condition */
#endif /* def BCM_FE2000_P3_SUPPORT */
    } else {
        /* no supported device here */
        VLAN_ERR((VLAN_FUNCMSG1("unsupported unit %d type\n"), unit));
        result = VLAN_UNKNOWN_TYPE;
    }

    /* register a callback with trunk to be notified of 
     * trunk membership changes
     */
    result = 
        bcm_fe2000_trunk_change_register(unit, 
                                         _bcm_fe2000_vlan_trunk_change_handler,
                                         NULL);
    if (BCM_FAILURE(result)) {
        VLAN_ERR((VLAN_FUNCMSG1("Failed to register callback with trunk: %s\n"),
                  bcm_errmsg(result)));
    }

    for(idx=0; idx < SBX_MAX_TRUNKS; idx++) {
        DQ_INIT(&_vlan_state[unit]->trunk[idx].vlan_port_head);
    }

    /* Warmboot recovers all state from the device */
    if (BCM_SUCCESS(result) && SOC_WARM_BOOT(unit)) {
        result = _bcm_fe2000_vlan_recover(unit);
        VLAN_LOCK_RELEASE;
        VLAN_VERB((VLAN_FUNCMSG("(%d) return %d (%s)\n"),
                   unit, result, _SHR_ERRMSG(result)));
        return result;
    }

    if (BCM_SUCCESS(result)) {
        /* Set up the default value for default VID */
        result = _bcm_fe2000_vlan_default_set(unit, BCM_VLAN_DEFAULT);
    }

    /* initialise some structures that will be used for initialisation */
    if (BCM_E_NONE == result) {
#ifdef BCM_FE2000_P3_SUPPORT
        if (SOC_IS_SBX_G2P3(unit)) {
            /* set up which ports need egrPort2Etc written */
            BCM_PBMP_ASSIGN(p2ePbmp, PBMP_PORT_ALL(unit));
            BCM_PBMP_OR(p2ePbmp, PBMP_CMIC(unit));
            BCM_PBMP_ASSIGN(ep2ePbmp, PBMP_ALL(unit));
            /* set up FT entry for NULL/default VLAN configuration */
            soc_sbx_g2p3_ft_t_init(&p3ft);
            p3ft.excidx = VLAN_INV_FTE_EXC;
            
            p3ft.rridx = 0; 
            /* set up vlan2Etc entry for NULL/default VLAN configuration */
            soc_sbx_g2p3_v2e_t_init(&p3vlan2Etc);
            p3vlan2Etc.dontlearn = TRUE;
        } /* if (SOC_IS_SBX_G2P3(unit)) */
#endif /* def BCM_FE2000_P3_SUPPORT */
    } /* if (BCM_E_NONE == result) */

    /* set up other tables if we have all the needed resources */
    if (BCM_E_NONE == result) {
        BCM_PBMP_ASSIGN(vlanPbmp, PBMP_PORT_ALL(unit));
        BCM_PBMP_OR(vlanPbmp, PBMP_CMIC(unit));


        result = soc_sbx_g2p3_vlan_ft_base_get(unit, &vlan_base_ft);
        if (BCM_E_NONE != result) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to get ft base %d:"
                                    " %d (%s)\n"),
                      unit,
                      result,
                      _SHR_ERRMSG(result)));
        }
        
        /* initialise all tables keyed only on VSI/VLAN/VID */
        for (vIndex=0; vIndex<= _sbx_vlan_max_vid_value; vIndex++) {
            /* set up and write up the vlan2Etc table entry */
            uint32 ftidx = 0, mc_ft_offset = 0;

            /* set up and write up the vlan2Etc table entry */
            auxres = soc_sbx_g2p3_v2e_set(unit,
                                          SBX_VSI_FROM_VID(vIndex),
                                          &p3vlan2Etc);
            if (BCM_E_NONE != auxres) {
                result = auxres;
                VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:v2e[%04X]:"
                                        " %d (%s)\n"),
                          unit,
                          SBX_VSI_FROM_VID(vIndex),
                          result,
                          _SHR_ERRMSG(result)));
            }
            /* set up and write up the FT entry */
            
            result = soc_sbx_g2p3_vlan_ft_base_get(unit, &ftidx);
            if (BCM_E_NONE != result) {
                VLAN_ERR((VLAN_FUNCMSG1("unable to get ft base %d:"
                                        " %d (%s)\n"),
                          unit,
                          result,
                          _SHR_ERRMSG(result)));
            }
            
            ftidx = SBX_VSI_FROM_VID(vIndex) + vlan_base_ft;
            auxres = soc_sbx_g2p3_ft_set(unit,
                                         ftidx,
                                         &p3ft);
            if (BCM_E_NONE != auxres) {
                result = auxres;
                VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:ft[%08X]:"
                                        " %d (%s)\n"),
                          unit,
                          SBX_VSI_FROM_VID(vIndex),
                          result,
                          _SHR_ERRMSG(result)));
            }
            
            /* Initialize FT entry for unknown mcast */
            if (BCM_E_NONE == auxres) {
                auxres =  soc_sbx_g2p3_mc_ft_offset_get(unit, &mc_ft_offset);
                if (BCM_E_NONE == auxres) {
                    ftidx += mc_ft_offset;
                    auxres = soc_sbx_g2p3_ft_set(unit,
                                                 ftidx,
                                                 &p3ft);
                }
            }
        } /* for (vIndex = 0; vIndex <= MAX_VID_VALUE; vIndex++) */

        /* initialise all tables that are keyed by port number */
        PBMP_ALL_ITER(unit, pIndex) { /* MUST be PBMP_ALL_ITER */

            if (0) {
#ifdef BCM_FE2000_P3_SUPPORT
            } else if (SOC_IS_SBX_G2P3(unit)) {
                /* set default filter modes */
                auxres = soc_sbx_g2p3_p2appdata_get(unit,
                                                    pIndex,
                                                    &p3p2appdata);
                if (SOC_E_NONE == auxres) {
                    p3p2appdata.efilteren = VLAN_FILTER_DEFAULT;
                    p3p2appdata.ifilteren = VLAN_FILTER_DEFAULT;
                    auxres = soc_sbx_g2p3_p2appdata_set(unit,
                                                        pIndex,
                                                        &p3p2appdata);
                }
                if (SOC_E_NONE != auxres) {
                    result = auxres;
                    VLAN_ERR((VLAN_FUNCMSG1("unable to set default filter"
                                            " state for %d:port[%d]:"
                                            " %d (%s)\n"),
                              unit,
                              pIndex,
                              result,
                              _SHR_ERRMSG(result)));
                }
                /* Set untagged VID as being untagged */
                auxres = _bcm_fe2000_vlan_g2p3_untagged_set(unit,
                                                            pIndex,
                                               _BCM_VLAN_G2P3_UNTAGGED_VID,
                                                            1);
                if (SOC_E_NONE != auxres) {
                    result = auxres;
                    VLAN_ERR((VLAN_FUNCMSG1("unable to untag %d:%d:"
                                            " %d (%s)\n"),
                              unit,
                              pIndex,
                              result,
                              _SHR_ERRMSG(result)));
                }
#endif /* def BCM_FE2000_P3_SUPPORT */
            }

            /* there are some tables keyed on port+VID; init them now */
            for (vIndex = 0; vIndex <= _sbx_vlan_max_vid_value; vIndex++) {
                /* make sure the port is not included */
                auxres = _bcm_fe2000_vlan_port_remove(unit, vIndex, pIndex);
                if (BCM_E_NONE != auxres) {
                    result = auxres;
                    VLAN_ERR((VLAN_FUNCMSG1("unable to remove unit %d port %d"
                                            " from VLAN %04X: %d (%s)\n"),
                              unit,
                              pIndex,
                              vIndex,
                              result,
                              _SHR_ERRMSG(result)));
                }
            } /* for (all VLANs including drop and invalid) */
        } /* BCM_PBMP_ITER(vlanPbmp, pIndex) */

        /* create the default VLAN, insert ports, set STP information */
        auxres = bcm_fe2000_stg_default_get(unit, &stg);
        if (BCM_E_NONE != auxres) {
            /* no default STG (yet?); assume one */
            stg = BCM_STG_DEFAULT;
        }
        auxres = _bcm_fe2000_vlan_add(unit, _vlan_state[unit]->default_vid);
        if (BCM_E_NONE == auxres) {
            BCM_PBMP_ITER(default_pbmp, pIndex) {
                if (!IS_HG_PORT(unit, pIndex)) {
                    /* ADD only non-HiGig ports here (create does them) */
                    auxres = _bcm_fe2000_vlan_port_add(unit,
                                                       _vlan_state[unit]->
                                                       default_vid,
                                                       pIndex,
                                                       TRUE);
                } else {
                    /* but other functions do apply to HiGig */
                    auxres = BCM_E_NONE;
                }
                if (BCM_E_NONE != auxres) {
                    result = auxres;
                    VLAN_ERR((VLAN_FUNCMSG1("unable to add port %d to default"
                                            " VLAN %04X on unit %d: %d (%s)\n"),
                              pIndex,
                              _vlan_state[unit]->default_vid,
                              unit,
                              result,
                              _SHR_ERRMSG(result)));
                }
            } /* BCM_PBMP_ITER(vlanPbmp, pIndex) */
        } else { /* if (BCM_E_NONE == auxres) */
            result = auxres;
            VLAN_ERR((VLAN_FUNCMSG1("unable to create default VLAN %04X on"
                                    " unit %d: %d (%s)\n"),
                      _vlan_state[unit]->default_vid,
                      unit,
                      result,
                      _SHR_ERRMSG(result)));
        } /* if (BCM_E_NONE == auxres) */

        /* make sure all ports have native VID set to default VID */
        /* note that this also copies default VID data to ports' pv2e[p,0] */
        PBMP_ALL_ITER(unit, pIndex) { /* MUST be PBMP_ALL_ITER */

            /* if this port is not in the default vlan, do nothing! */
            if (!SOC_PBMP_MEMBER(default_pbmp, pIndex)) {
                continue;
            }

            auxres = _bcm_fe2000_vlan_port_native_vlan_set(unit,
                                                           pIndex,
                                                           _vlan_state[unit]->default_vid,
                                                           0, BCM_FE2K_NVID_ACTION_SET);
            if (BCM_E_NONE != auxres) {
                result = auxres;
                VLAN_ERR((VLAN_FUNCMSG1("unable to set default VID for unit %d"
                                        " port %d to %03X: %d (%s)\n"),
                          unit,
                          pIndex,
                          _vlan_state[unit]->default_vid,
                          result,
                          _SHR_ERRMSG(result)));
            }

            /*
             *  Set STP DISABLE state for all 'front panel' ports.  Explicitly
             *  avoid CPU ports, but there seem to be other 'front panel' ports
             *  in G2 that are not really 'front panel'...
             */
            if (IS_PORT(unit,pIndex) && (!IS_CPU_PORT(unit,pIndex))) {
                auxres = bcm_fe2000_stg_stp_set(unit,
                                                stg,
                                                pIndex,
                                                BCM_STG_STP_DISABLE);
                if (BCM_E_NONE != auxres) {
                    if (BCM_E_PORT != auxres) {
                        /*
                         *  IS_PORT should only apply to front panel ports; it
                         *  seems this is not true in G2; submit a debug
                         *  message but don't set the return value when port
                         *  isn't accepted.
                         */
                        result = auxres;
                    }
                    VLAN_ERR((VLAN_FUNCMSG1("unable to set STP disable"
                                            " state on unit %d port %d"
                                            " after creating default"
                                            " VLAN %04X: %d (%s)\n"),
                              unit,
                              pIndex,
                              _vlan_state[unit]->default_vid,
                              auxres,
                              _SHR_ERRMSG(auxres)));
                }
            } /* (!BCM_PBMP_MEMBER(PBMP_CMIC(unit),pIndex)) */
        } /* PBMP_ALL_ITER(unit, pIndex) */

        /* indicate initialisation no longer pending */
        VLAN_VVERB((VLAN_FUNCMSG1("complete unit %d initialisation\n"),
                    unit));
        /*
         *  THE _vlan_state[unit]->init MUST BE SET TO FALSE ONCE THE
         *  INITIALISATION HAS BEEN COMPLETED, ELSE ETEs WILL BE PROFUSELY
         *  LEAKED BY THE EGRESS VLAN TRANSLATION CODE.
         */
        _vlan_state[unit]->init = FALSE;
    } /* if (BCM_E_NONE == result) */

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d) return %d (%s)\n"),
               unit,
               result,
               _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_create
 *   Purpose
 *      Create appropriate entries for provided VID.
 *   Parameters
 *      (in) int unit = unit number on which to create the VID
 *      (in) bcm_vlan_t = VID to create
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This uses the existing entries filled in by init, above, and changes
 *      them as appropriate to enble the specified VID.  It will return errors
 *      for conditions such as the VID having already been enabled.
 *      Assumes VLAN:VID mapping is 1:1.
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_create"
int
bcm_fe2000_vlan_create(int unit,
                       bcm_vlan_t vid)
{
    int                    result;             /* local result code */

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X) enter\n"),
               unit,
               vid));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* check for valid VID value */
    if ((0 == vid) || (vid > _sbx_vlan_max_vid_value)) {
        VLAN_ERR((VLAN_FUNCMSG1("vid %04X out of range on unit %d\n"),
                  vid,
                  unit));
        return BCM_E_PARAM;
    }

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* create the new VLAN */
    result = _bcm_fe2000_vlan_add(unit, vid);

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X) return %d (%s)\n"),
               unit,
               vid,
               result,
               _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_destroy
 *   Purpose
 *      Destroy appropriate entries for provided VID.
 *   Parameters
 *      (in) int unit = unit number from which to remove the VID
 *      (in) bcm_vlan_t = VID to remove
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      This will change any ports whose untagged ingress VID is the destroyed
 *      VID so their untagged ingress VID is the default VID.
 *      This will not destroy the default VID.
 *      This disables frame forwarding on the specified VID, both at the
 *      forwarding table and at the port,VID table.
 *      Makes external calls; these should not call back into this module.
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_destroy"
int
bcm_fe2000_vlan_destroy(int unit,
                        bcm_vlan_t vid)
{
    int                   result;           /* local result code */

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X) enter\n"),
               unit,
               vid));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* check for valid VID value */
    if ((0 == vid) || (vid > _sbx_vlan_max_vid_value)) {
        VLAN_ERR((VLAN_FUNCMSG1("vid %04X out of range on unit %d\n"),
                  vid,
                  unit));
        return BCM_E_PARAM;
    }

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* Destroy the specified VID */
    result = _bcm_fe2000_vlan_remove(unit, vid);
    if (BCM_E_NONE != result) {
        VLAN_ERR((VLAN_FUNCMSG1("_bcm_fe2000_vlan_remove(%d,%04X)"
                                " returned %d (%s)\n"),
                 unit,
                 vid,
                 result,
                 _SHR_ERRMSG(result)));
    }

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X) return %d (%s)\n"),
               unit,
               vid,
               result,
               _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_destroy_all
 *   Purpose
 *      Destroy all VIDs except the default one.
 *   Parameters
 *      (in) int unit = unit number from which to remove the VID
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      This will change any ports whose untagged ingress VID is the destroyed
 *      VIDs so their untagged ingress VID is the default VID.
 *      This will not destroy the default VID.
 *      This disables frame forwarding on the destroyed VIDs, both at the
 *      forwarding table and at the port,VID table.
 *      Makes external calls; these should not call back into this module.
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_destroy_all"
int
bcm_fe2000_vlan_destroy_all(int unit)
{
    unsigned int          vIndex;           /* working VID */
    int                   curres;           /* working result code */
    int                   result;           /* local result code */

    VLAN_VERB((VLAN_FUNCMSG("(%d) enter\n"),
               unit));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* Optimistically expect success */
    result = BCM_E_NONE;

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* Destroy all VIDs that are not the default */
    for (vIndex=1; vIndex<= _sbx_vlan_max_vid_value; vIndex++) {
        if (_vlan_state[unit]->default_vid != vIndex) {
            /* This VID is not the default; get rid of it */
            curres = _bcm_fe2000_vlan_remove(unit, vIndex);
            if ((BCM_E_NONE != curres) && (BCM_E_NOT_FOUND != curres)) {
                /* An error occurred; adjust result code */
                result = curres;
                VLAN_ERR((VLAN_FUNCMSG1("_bcm_fe2000_vlan_remove(*,%d,%03X)"
                                        " returned %d (%s)\n"),
                          unit,
                          vIndex,
                          result,
                          _SHR_ERRMSG(result)));
            }
        }
    } /* for (vIndex = 0; vIndex < _vlan_max_vid_value; vIndex++) */

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d) return %d (%s)\n"),
               unit,
               result,
               _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_port_add
 *   Purpose
 *      Set the list of member ports for the specified VID to include the
 *      provided list of member ports.  The pbmp is the union of tagged and
 *      untagged ports in the VID; the ubmp is the untagged ports only.
 *   Parameters
 *      (in) int unit = unit number whose VID is to be queried
 *      (in) bcm_vlan_t = VID whose ports are to be queried
 *      (in) pbmp_t pbmp = bitmap of ports on this VID
 *      (in) pbmp_t ubmp = bitmap of those ports that are untagged
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This uses the existing entries filled in by init, above, and changes
 *      them as appropriate to enable the specified ports on the specified
 *      VID.
 *      Assumes VLAN:VID mapping is 1:1.
 *      ubmp must be a subset of pbmp (but need not be proper).
 *      Adding ports already in the VID does not seem to be an error.
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_port_add"
int
bcm_fe2000_vlan_port_add(int unit,
                         bcm_vlan_t vid,
                         pbmp_t pbmp,
                         pbmp_t ubmp)
{
    _bcm_sbx_vlan_ft_t     ft;               /* working FT (flooding) entry */
    int                    valid = FALSE;    /* does VLAN exist */
    unsigned int           pIndex;           /* working port index */
    int                    result;           /* local result code */
    int                    tempRes;          /* called func's result code */
    bcm_pbmp_t             tbmp;             /* working port bitmap */

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,"
                            VLAN_PBMP_FORMAT
                            ","
                            VLAN_PBMP_FORMAT
                            ") enter\n"),
               unit,
               vid,
               VLAN_PBMP_SHOW(pbmp),
               VLAN_PBMP_SHOW(ubmp)));


    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* check for valid VID value */
    if ((0 == vid) || (vid > _sbx_vlan_max_vid_value)) {
        VLAN_ERR((VLAN_FUNCMSG1("VLAN %04X is not valid on unit %d\n"),
                  vid,
                  unit));
        return BCM_E_PARAM;
    }

    /* make sure all the ports are valid */
    BCM_PBMP_ASSIGN(tbmp, pbmp);
    BCM_PBMP_AND(tbmp, PBMP_ALL(unit));
    if (!BCM_PBMP_EQ(tbmp, pbmp)) {
        /* unavailable ports are included in the pbmp */
        VLAN_ERR((VLAN_FUNCMSG1("add unavailable ports ("
                                VLAN_PBMP_FORMAT
                                " chosen; "
                                VLAN_PBMP_FORMAT
                                " available for VLAN %04X on unit %d)\n"),
                  VLAN_PBMP_SHOW(pbmp),
                  VLAN_PBMP_SHOW(PBMP_ALL(unit)),
                  vid,
                  unit));
        return BCM_E_PARAM;
    }

    /* make sure the untagged flags are valid */
    BCM_PBMP_ASSIGN(tbmp, ubmp);
    BCM_PBMP_AND(tbmp, pbmp);
    if (!BCM_PBMP_EQ(tbmp, ubmp)) {
        /* ports are in untagged map but not in ports map */
        VLAN_ERR((VLAN_FUNCMSG1("untag ports ("
                                VLAN_PBMP_FORMAT
                                ") is not subset of add ports ("
                                VLAN_PBMP_FORMAT
                                " on VLAN %04X of unit %d\n"),
                  VLAN_PBMP_SHOW(ubmp),
                  VLAN_PBMP_SHOW(pbmp),
                  vid,
                  unit));
        return BCM_E_PARAM;
    }

    /* Optimistically assume success from here */
    result = BCM_E_NONE;

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* Add the ports to the VID if it exists */
    result = _bcm_fe2000_vlan_check(unit, vid, &valid, &ft);
    if (BCM_E_NONE == result) {
        /* read of VID info successful */
        if (valid) {
            /* VID has been created  */
            /* Add each port to this VID & set tagged/untagged egress */
            SOC_PBMP_ITER(pbmp, pIndex) {
                /* Add each port to the VID */
                tempRes = _bcm_fe2000_vlan_port_add(unit,
                                                    vid,
                                                    pIndex,
                                                    SOC_PBMP_MEMBER(ubmp,
                                                                    pIndex));
                if (BCM_E_NONE != tempRes) {
                    /* something went wrong; keep the result for return */
                    result = tempRes;
                    VLAN_ERR((VLAN_FUNCMSG1("_bcm_fe2000_vlan_port_add"
                                            "(%d,%04X,%d,%s)"
                                            " returned %d (%s)\n"),
                              unit,
                              vid,
                              pIndex,
                              SOC_PBMP_MEMBER(ubmp, pIndex)?"TRUE":"FALSE",
                              result,
                              _SHR_ERRMSG(result)));
                }
            } /* SOC_PBMP_ITER(pbmp,pIndex) */
        } else { /* if (valid) */
            /* VID is not created yet */
            VLAN_ERR((VLAN_FUNCMSG1("VLAN %04X does not exist on unit %d\n"),
                      vid,
                      unit));
            result = BCM_E_NOT_FOUND;
        } /* if (valid) */
    } else {/* if (BCM_E_NONE == result) */
        /* Something went wrong; translate to proper result code */
        VLAN_ERR((VLAN_FUNCMSG1("unable to check unit %d VLAN %04X:"
                                " %d (%s)\n"),
                  unit,
                  vid,
                  result,
                  _SHR_ERRMSG(result)));
    } /* if (BCM_E_NONE == result) */

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,"
                            VLAN_PBMP_FORMAT
                            ","
                            VLAN_PBMP_FORMAT
                            ") return %d (%s)\n"),
               unit,
               vid,
               VLAN_PBMP_SHOW(pbmp),
               VLAN_PBMP_SHOW(ubmp),
               result,
               _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

int
bcm_fe2000_vlan_port_default_action_get(int unit, bcm_port_t port,
                                        bcm_vlan_action_set_t *a)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_port_default_action_set(int unit, bcm_port_t port,
                                        bcm_vlan_action_set_t *a)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_port_egress_default_action_get(int unit, bcm_port_t port,
                                               bcm_vlan_action_set_t *a)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_port_egress_default_action_set(int unit, bcm_port_t port,
                                               bcm_vlan_action_set_t *a)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_port_protocol_action_add(int unit, bcm_port_t port,
                                         bcm_port_frametype_t f,
                                         bcm_port_ethertype_t ethertype,
                                         bcm_vlan_action_set_t *a)
{
    return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_fe2000_vlan_port_remove
 *   Purpose
 *      Remove the specified ports from the list of member ports for the
 *      specified VID.
 *   Parameters
 *      (in) int unit = unit number whose VID is to have ports removed
 *      (in) bcm_vlan_t = VID to which the ports are to ba removed
 *      (in) pbmp_t pbmp = bitmap of ports to remove from this VID
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This uses the existing entries filled in by init, above, and changes
 *      them as appropriate to disable the specified ports on the specified
 *      VID.
 *      Assumes VLAN:VID mapping is 1:1.
 *      Removing ports not already in the VID does not seem to be an error.
 */

#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_port_remove"
int
bcm_fe2000_vlan_port_remove(int unit,
                            bcm_vlan_t vid,
                            pbmp_t pbmp)
{
    _bcm_sbx_vlan_ft_t     ft;               /* working FT (flooding) entry */
    int                    valid;            /* does VLAN exist */
    unsigned int           pIndex;           /* working port index */
    int                    result;           /* local result code */
    int                    tempRes;          /* called func's result code */
    bcm_pbmp_t             tbmp;             /* working port bitmap */

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,"
                            VLAN_PBMP_FORMAT
                            ") enter\n"),
               unit,
               vid,
               VLAN_PBMP_SHOW(pbmp)));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* check for valid VID value */
    if ((0 == vid) || (vid > _sbx_vlan_max_vid_value)) {
        VLAN_ERR((VLAN_FUNCMSG1("VLAN %04X is not valid on unit %d\n"),
                  vid,
                  unit));
        return BCM_E_PARAM;
    }

    /* make sure all the ports are valid */
    BCM_PBMP_ASSIGN(tbmp, pbmp);
    BCM_PBMP_AND(tbmp, PBMP_ALL(unit));
    if (!BCM_PBMP_EQ(tbmp, pbmp)) {
        /* unavailable ports are included in the pbmp */
        VLAN_ERR((VLAN_FUNCMSG1("remove unavailable ports ("
                                VLAN_PBMP_FORMAT
                                " chosen; "
                                VLAN_PBMP_FORMAT
                                " available for VLAN %04X on unit %d)\n"),
                  VLAN_PBMP_SHOW(pbmp),
                  VLAN_PBMP_SHOW(PBMP_ALL(unit)),
                  vid,
                  unit));
        return BCM_E_PARAM;
    }

    /* Optimistically assume success from here */
    result = BCM_E_NONE;

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* make sure the VID exists */
    result = _bcm_fe2000_vlan_check(unit, vid, &valid, &ft);
    if (BCM_E_NONE == result) {
        /* read of VID info successful */
        if (valid) {
            /* VID has been created */
            /* Remove the ports.  */
            SOC_PBMP_ITER(pbmp,pIndex) {
                tempRes = _bcm_fe2000_vlan_port_remove(unit,
                                                       vid,
                                                       pIndex);                
                if (BCM_E_NONE != tempRes) {
                    /* something went wrong; keep the result for return */
                    result = tempRes;
                    VLAN_ERR((VLAN_FUNCMSG1("_bcm_fe2000_vlan_port_remove"
                                            "(%d,%04X,%d)"
                                            " returned %d (%s)\n"),
                              unit,
                              vid,
                              pIndex,
                              result,
                              _SHR_ERRMSG(result)));
                }
            } /* SOC_PBMP_ITER(pbmp,pIndex) */
        } else { /* if (valid) */
            /* VID is not created yet */
            result = BCM_E_NOT_FOUND;
            VLAN_ERR((VLAN_FUNCMSG1("unit %d has no VLAN %04X\n"),
                      unit,
                      vid));
        } /* if (valid) */
    } else {/* if (BCM_E_NONE == result) */
        /* Something went wrong; translate to proper result code */
        VLAN_ERR((VLAN_FUNCMSG1("unable to check unit %d VLAN %04X:"
                                " %d (%s)\n"),
                  unit,
                  vid,
                  result,
                  _SHR_ERRMSG(result)));
    } /* if (BCM_E_NONE == result) */


    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,"
                            VLAN_PBMP_FORMAT
                            ") return %d (%s)\n"),
               unit,
               vid,
               VLAN_PBMP_SHOW(pbmp),
               result,
               _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_port_get
 *   Purpose
 *      Get the list of member ports for the specified VID.  The pbmp is the
 *      union of tagged and untagged ports in the VID; the ubmp is the
 *      untagged ports only.
 *   Parameters
 *      (in) int unit = unit number whose VID is to be examined for ports
 *      (in) bcm_vlan_t = VID to which the ports are to ba added
 *      (out) pbmp_t pbmp = ptr to bitmap of ports
 *      (out) pbmp_t *ubmp = ptr to bitmap of untagged ports
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      ubmp will be a subset of pbmp.
 *      ubmp and pbmp are indeterminate on most errors.
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_port_get"
int
bcm_fe2000_vlan_port_get(int unit,
                         bcm_vlan_t vid,
                         pbmp_t *pbmp,
                         pbmp_t *ubmp)
{
    int result;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,*,*) enter\n"),
               unit,
               vid));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* check for valid parameter values */
    if ((_sbx_vlan_max_vid_value < vid) ||
        (0 == vid)) {
        VLAN_ERR((VLAN_FUNCMSG1("VLAN %04X is not valid on unit %d\n"),
                  vid,
                  unit));
        return BCM_E_PARAM;
    }

    /* check for valid parameter values */
    if ((!(pbmp)) ||
        (!(ubmp))) {
        VLAN_ERR((VLAN_FUNCMSG1("NULL pointers for returned data\n")));
        return BCM_E_PARAM;
    }

    /* get the ports in this VID */
    result = _bcm_fe2000_vlan_port_fetch(unit, vid, pbmp, ubmp);

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,&("
                            VLAN_PBMP_FORMAT
                            "),&("
                            VLAN_PBMP_FORMAT
                            ")) return %d (%s)\n"),
               unit,
               vid,
               VLAN_PBMP_SHOW(*pbmp),
               VLAN_PBMP_SHOW(*ubmp),
               result,
               _SHR_ERRMSG(result)));

    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_list_by_pbmp
 *   Purpose
 *      Get a list of the VIDs that exist on this unit, that include any of the
 *      specified ports.
 *   Parameters
 *      (in) int unit = unit number whose VLAN list is to be built
 *      (in) pbmp_t pbmp = ports to consider
 *      (out) bcm_vlan_data_t **listp = pointer to variable for address of list
 *      (out) int *countp = pointer to variable for number of list elements
 *   untagged ports Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      This implementation allocates one entry too many in the list to avoid
 *      memcpy operations, but returns correct number of entries. The list
 *      destroy function can still free the list correctly.
 *      Always sets list pointer and count to sensible values -- in case of
 *      error or early bailout, they will be NULL and zero respectively.
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_list_by_pbmp"
int
bcm_fe2000_vlan_list_by_pbmp(int unit,
                             pbmp_t pbmp,
                             bcm_vlan_data_t **listp,
                             int *countp)
{
    VLAN_VERB((VLAN_FUNCMSG("(%d,"
                            VLAN_PBMP_FORMAT
                            ",*,*) enter\n"),
               unit,
               VLAN_PBMP_SHOW(pbmp)));

    /* Get the list of VIDs who have at least one port in the bitmap */
    return _bcm_fe2000_vlan_list_fetch(unit, pbmp, listp, countp, TRUE);
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_list
 *   Purpose
 *      Get a list of the VIDs that exist on this unit.
 *   Parameters
 *      (in) int unit = unit number whose VLAN list is to be built
 *      (out) bcm_vlan_data_t **listp = pointer to variable for address of list
 *      (out) int *countp = pointer to variable for number of list elements
 *   untagged ports Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      This implementation allocates one entry too many in the list to avoid
 *      memcpy operations, but returns correct number of entries. The list
 *      destroy function can still free the list correctly.
 *      Always sets list pointer and count to sensible values -- in case of
 *      error or early bailout, they will be NULL and zero respectively.
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_list"
int
bcm_fe2000_vlan_list(int unit,
                     bcm_vlan_data_t **listp,
                     int *countp)
{
    VLAN_VERB((VLAN_FUNCMSG("(%d,*,*) enter\n"),
               unit));

    /* Get the complete VID list */
    return _bcm_fe2000_vlan_list_fetch(unit,
                                       PBMP_ALL(unit),
                                       listp,
                                       countp,
                                       FALSE);
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_list_destroy
 *   Purpose
 *      Dispose of a vlan list obtained from _vlan_list or _vlan_list_by_pbmp.
 *   Parameters
 *      (in) int unit = unit number from which this list was taken
 *      (out) bcm_vlan_data_t **listp = pointer to variable for address of list
 *      (out) int countp = number of list elements
 *   untagged ports Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      This does not verify that the memory being freed was created by the
 *      above functions; if the caller is using the lists and other dynamic
 *      memory cells correctly it does not matter.
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_list_destroy"
int
bcm_fe2000_vlan_list_destroy(int unit,
                             bcm_vlan_data_t *list,
                             int count)
{
    VLAN_VERB((VLAN_FUNCMSG("(%d,*,%d) enter\n"),
               unit,
               count));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Free the list if needed */
    if (list) {
        /* List pointer was not NULL; free the list */
        sal_free(list);
    }

    VLAN_VERB((VLAN_FUNCMSG("(%d,*,%d) return %d (%s)\n"),
               unit,
               count,
               BCM_E_NONE,
               _SHR_ERRMSG(BCM_E_NONE)));

    /* Since free doesn't return a result, assume it's successful */
    return BCM_E_NONE;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_default_get
 *   Purpose
 *      Get the current default VID
 *   Parameters
 *      (in) int unit = unit number whose default VID is to be retrieved
 *      (out) bcm_vlan_t *vid_ptr = where to put default VID number
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This just reads the local state for the particular unit.  No hardware
 *      work is committed.
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_default_get"
int
bcm_fe2000_vlan_default_get(int unit,
                            bcm_vlan_t *vid_ptr)
{
    VLAN_VERB((VLAN_FUNCMSG("(%d,*) enter\n"),
               unit));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* make sure the arguments are valid */
    if (!vid_ptr) {
        VLAN_ERR((VLAN_FUNCMSG1("NULL pointer for outbound argument\n")));
        return BCM_E_PARAM;
    }

#if VLAN_LOCK_ATOMIC_READ
    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;
#endif /* VLAN_LOCK_ATOMIC_READ */

    /* Put the default VID value where we were told to put it */
    *vid_ptr = _vlan_state[unit]->default_vid;

#if VLAN_LOCK_ATOMIC_READ
    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;
#endif /* VLAN_LOCK_ATOMIC_READ */

    VLAN_VERB((VLAN_FUNCMSG("(%d,&(%04X)) return %d (%s)\n"),
               unit,
               *vid_ptr,
               BCM_E_NONE,
               _SHR_ERRMSG(BCM_E_NONE)));

    /* Return successfully */
    return BCM_E_NONE;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_default_set
 *   Purpose
 *      Set the default VID
 *   Parameters
 *      (in) int unit = unit number whose default VID is to be set
 *      (in) bcm_vlan_t vid = new default VID
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      It seems that this function is somewhat complicated here.  All ports
 *      are implicitly members of the default VID, which means that we must
 *      ensure that all ports become members of the new default VID if they're
 *      not already so (we must remember those for which this is the case) and
 *      any port not an explicit member of the current default VID must be
 *      removed from the current default when it is added to the new default
 *      (explicit members of the old default VID retain their membership in
 *      that VID).  Additionally, the new default VID must exist before this
 *      function is called, and the old one default VID continues to exist but
 *      no longer as the default after this function returns.  In addition to
 *      all that, the default VID can be disabled by setting it to
 *      BCM_VLAN_VID_DISABLE.
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_default_set"
int
bcm_fe2000_vlan_default_set(int unit,
                            bcm_vlan_t vid)
{
    int                   valid;            /* does VLAN exist */
    int                   result;           /* local result code */
    _bcm_sbx_vlan_ft_t    ft;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X) enter\n"), unit, vid));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* check for valid parameter values */
    if ((BCM_VLAN_VID_DISABLE != vid) &&
        ((_sbx_vlan_max_vid_value < vid) || (0 == vid))) {
        VLAN_ERR((VLAN_FUNCMSG1("%04X is an invalid default VLAN"
                                " for unit %d\n"),
                  vid,
                  unit));
        return BCM_E_PARAM;
    }

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* Make sure the target VID already exists and is valid */
    if (BCM_VLAN_VID_DISABLE != vid) {
        VLAN_EVERB((VLAN_FUNCMSG1("verify new VID exists\n")));
        /* Target VID is not DISABLE; verify it exists */
        result = _bcm_fe2000_vlan_check(unit, vid, &valid, &ft);
        if (BCM_E_NONE == result) {
            /* read of VID info successful */
            if (!valid) {
                /* But the VID itself does not exist */
                result = BCM_E_NOT_FOUND;
                VLAN_ERR((VLAN_FUNCMSG1("VID %04X does not exist on unit %d\n"),
                          vid,
                          unit));
            }
        } else { /* if (BCM_E_NONE == result) */
            /* could not read new VID information */
            VLAN_ERR((VLAN_FUNCMSG1("unable to check new default VLAN %04X"
                                    " on unit %d\n"),
                     vid,
                     unit));
        } /* if (BCM_E_NONE == result) */
    } /* if (BCM_VLAN_VID_DISABLE != vid) */

    result = _bcm_fe2000_vlan_default_set(unit, vid);

    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X) return %d (%s)\n"),
               unit,
               vid,
               result,
               _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_stg_get
 *   Purpose
 *      Get the spanning tree group for the VID
 *   Parameters
 *      (in) int unit = unit number whose VID is to be checked
 *      (in) bcm_vlan_t vid = VID whose STG is to be read
 *      (out) bcm_stg_t *stg_ptr = place for the STG to be stored
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This just calls the STG functions to do its job.
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_stg_get"
int
bcm_fe2000_vlan_stg_get(int unit,
                        bcm_vlan_t vid,
                        bcm_stg_t *stg_ptr)
{
    _bcm_sbx_vlan_ft_t     ft;               /* working FT (flooding) entry */
    int                    valid;            /* does VLAN exist */
    int                    result;           /* local result code */

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,*) enter\n"),
               unit,
               vid));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* check for valid parameter values */
    
    if ((BCM_VLAN_VID_DISABLE != vid) &&
         ((_sbx_vlan_max_vid_value < vid) || (0 == vid))) {
        VLAN_ERR((VLAN_FUNCMSG1("invalid VID %04X on unit %d\n"),
                  vid,
                  unit));
        return BCM_E_PARAM;
    }
    if (!stg_ptr) {
        VLAN_ERR((VLAN_FUNCMSG1("NULL pointer for outbound argument\n")));
        return BCM_E_PARAM;
    }

    /* Hope for the best... */
    result = BCM_E_NONE;

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    result = _bcm_fe2000_vlan_check(unit, vid, &valid, &ft);

    /* Don't hold the semaphore during a cross-module call! */
    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    if (BCM_E_NONE == result) {
        /* read of VID info successful */
        if (valid) {
            /* Get the current STG for this VID */
            result = _bcm_fe2000_stg_map_get(unit, vid, stg_ptr);
            if (BCM_E_NONE != result) {
                /* failed to read STG for this VID */
                VLAN_ERR((VLAN_FUNCMSG1("_bcm_fe2000_stg_map_get"
                                        "(%d,%04X,*) returned %d (%s)\n"),
                          unit,
                          vid,
                          result,
                          _SHR_ERRMSG(result)));
            }
        } else { /* if (valid) */
            /* VID is not created yet */
            result = BCM_E_NOT_FOUND;
            VLAN_ERR((VLAN_FUNCMSG1("unit %d does not have VLAN %04X\n"),
                      unit,
                      vid));
        } /* if (valid) */
    } /* if (BCM_E_NONE == result) */

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,&(%d)) return %d (%s)\n"),
               unit,
               vid,
               *stg_ptr,
               result,
               _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_stg_set
 *   Purpose
 *      Set the spanning tree group for the VID
 *   Parameters
 *      (in) int unit = unit number whose VID is to be checked
 *      (in) bcm_vlan_t vid = VID whose STG is to be read
 *      (in) bcm_stg_t stg = the new STG for the VID
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This just calls the STG functions to do its job.
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_stg_set"
int
bcm_fe2000_vlan_stg_set(int unit,
                        bcm_vlan_t vid,
                        bcm_stg_t stg)
{
    _bcm_sbx_vlan_ft_t     ft;               /* working FT (flooding) entry */
    int                    valid;            /* does VLAN exist */
    int                    result;           /* local result code */

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,*) enter\n"),
               unit,
               vid));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* check for valid parameter values */
    
    if ((BCM_VLAN_VID_DISABLE != vid) &&
         ((_sbx_vlan_max_vid_value < vid) || (0 == vid))) {
        VLAN_ERR((VLAN_FUNCMSG1("invalid VID %04X on unit %d\n"),
                  vid,
                  unit));
        return BCM_E_PARAM;
    }

    /* Hope for the best... */
    result = BCM_E_NONE;

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    result = _bcm_fe2000_vlan_check(unit, vid, &valid, &ft);

    /* Don't hold the semaphore during a cross-module call! */
    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    if (BCM_E_NONE == result) {
        /* read of VID info successful */
        if (valid) {
            /* Add this VID to the specified STG */
            result = bcm_fe2000_stg_vlan_add(unit, stg, vid);
            if (BCM_E_NONE != result) {
                VLAN_ERR((VLAN_FUNCMSG1("_bcm_fe2000_stg_vlan_add"
                                        "(%d,%d,%04X)"
                                        " returned %d (%s)\n"),
                          unit,
                          stg,
                          vid,
                          result,
                          _SHR_ERRMSG(result)));
            }
        } else { /* if (valid) */
            /* VID is not created yet */
            result = BCM_E_NOT_FOUND;
            VLAN_ERR((VLAN_FUNCMSG1("unit %d does not have VLAN %04X\n"),
                      unit,
                      vid));
        } /* if (valid) */
    } /* if (BCM_E_NONE == result) */

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,%d) return %d (%s)\n"),
               unit,
               vid,
               stg,
               result,
               _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME




/*
 *   Function
 *      _bcm_fe2000_vlan_independent_stp_get
 *   Purpose
 *      Set the spanning tree state for the port on the VID, irrespective
 *      STG, vlan, and port membership.
 *   Parameters
 *      (in)  unit = unit number whose VID is to be checked
 *      (in)  vid = VID whose STP state is to be changed
 *      (in)  port = port whose STP state is to be changed
 *      (in)  stp_state = new stp state for the port on the VID
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_independent_stp_get"
static int
_bcm_fe2000_vlan_independent_stp_get(int unit, bcm_port_t port,
                                     bcm_vlan_t vid, int *stp_state)
{
    int          rv = BCM_E_NONE;
    bcm_port_t   phyPort = port;
    bcm_vlan_t   tgtVid = vid;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,*) enter\n"), unit, vid));

    if (BCM_GPORT_IS_MPLS_PORT(port)) {
        rv = BCM_E_UNAVAIL;

    } else if (BCM_GPORT_IS_VLAN_PORT(port)) {
        switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
        {
            bcm_vlan_port_t vlanPort;
            bcm_module_t myModId, tgtModId = -1;

            /* get the vlan_port data struct represented by the gport */
            sal_memset(&vlanPort, 0, sizeof(vlanPort));
            vlanPort.vlan_port_id = port;
            vlanPort.criteria = BCM_VLAN_PORT_MATCH_NONE;
            rv = bcm_fe2000_vlan_port_find(unit, &vlanPort);

            if (BCM_FAILURE(rv)) {
                VLAN_ERR((VLAN_FUNCMSG1("failed to get vlan_port for "
                                        "gport 0x%08x: %d %s\n"),
                          port, rv, bcm_errmsg(rv)));
            }

            if (BCM_SUCCESS(rv)) {
                rv = _bcm_fe2000_map_vlan_gport_targets(unit, port, &myModId,
                                                        &tgtModId, &phyPort,
                                                        NULL, NULL);

                if (BCM_FAILURE(rv)) {
                    VLAN_ERR((VLAN_FUNCMSG1("failed to get vlan_gort targets for "
                                            "gport 0x%08x: %d %s\n"),
                              port, rv, bcm_errmsg(rv)));
                }
            }

            if (BCM_SUCCESS(rv) && myModId != tgtModId) {
                VLAN_ERR((VLAN_FUNCMSG1("VLAN GPORT %08X home module %d is not"
                                        " local module %d; must manipulate STP"
                                        " state on the home module\n"),
                          port, tgtModId, myModId));
                rv = BCM_E_PARAM;
            }

            tgtVid = vlanPort.match_vlan;
        }
        break;
#endif
        default:
            VLAN_ERR((VLAN_FUNCMSG1("unknown ucode type=%d\n"),
                      SOC_SBX_CONTROL(unit)->ucodetype));
            rv = BCM_E_CONFIG;
        }
    }

    if (BCM_SUCCESS(rv)) {

        switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            rv = _bcm_fe2000_g2p3_stg_vid_stp_get(unit, tgtVid, phyPort,
                                                  stp_state);
            break;
#endif
        default:
            rv = BCM_E_CONFIG;
        }

        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("failed to get stpState on port %d: "
                                    " %d %s\n"),
                      phyPort, rv, bcm_errmsg(rv)));
        }
    }

    return rv;
}
#undef _VLAN_FUNC_NAME


/*
 *   Function
 *      _bcm_fe2000_vlan_independent_stp_set
 *   Purpose
 *      Set the spanning tree state for the port on the VID, irrespective
 *      STG, vlan, and port membership.
 *   Parameters
 *      (in)  unit = unit number whose VID is to be checked
 *      (in)  vid = VID whose STP state is to be changed
 *      (in)  port = port whose STP state is to be changed
 *      (in)  stp_state = new stp state for the port on the VID
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_independent_stp_set"
static int
_bcm_fe2000_vlan_independent_stp_set(int unit, bcm_port_t port,
                                     bcm_vlan_t vid, int stp_state)
{
    int          rv = BCM_E_NONE;
    int         *fastSets;
    bcm_port_t   phyPort = port;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,*) enter\n"), unit, vid));

    /* check for valid parameter values */
    if (vid > BCM_VLAN_MAX) {
        VLAN_ERR((VLAN_FUNCMSG1("invalid VID %04X on unit %d\n"), vid, unit));
        return BCM_E_PARAM;
    }

    fastSets = sal_alloc(BCM_VLAN_COUNT * sizeof(int), "vlan faststp");
    if (fastSets == NULL) {
        return BCM_E_MEMORY;
    }

    sal_memset(fastSets, 0, BCM_VLAN_COUNT * sizeof(int));

    if (BCM_GPORT_IS_MPLS_PORT(port)) {
        rv = BCM_E_UNAVAIL;

    } else if (BCM_GPORT_IS_VLAN_PORT(port)) {
        switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
        {
            bcm_vlan_port_t vlanPort;
            bcm_module_t myModId, tgtModId = -1;
            bcm_vlan_vector_t vlanVec;
            bcm_vlan_t idx;

            /* get the vlan_port data struct represented by the gport, then
             * find the vlan_vector to update the stp state for all vids in
             * the vector
             */
            sal_memset(&vlanPort, 0, sizeof(vlanPort));
            vlanPort.vlan_port_id = port;
            vlanPort.criteria = BCM_VLAN_PORT_MATCH_NONE;
            rv = bcm_fe2000_vlan_port_find(unit, &vlanPort);

            if (BCM_FAILURE(rv)) {
                VLAN_ERR((VLAN_FUNCMSG1("failed to get vlan_port for "
                                        "gport 0x%08x: %d %s\n"),
                          port, rv, bcm_errmsg(rv)));
            }

            if (BCM_SUCCESS(rv)) {
                rv = _bcm_fe2000_map_vlan_gport_targets(unit, port, &myModId,
                                                        &tgtModId, &phyPort,
                                                        NULL, NULL);

                if (BCM_FAILURE(rv)) {
                    VLAN_ERR((VLAN_FUNCMSG1("failed to get vlan_gort targets for "
                                            "gport 0x%08x: %d %s\n"),
                              port, rv, bcm_errmsg(rv)));
                }
            }

            if (BCM_SUCCESS(rv) && myModId != tgtModId) {
                VLAN_ERR((VLAN_FUNCMSG1("VLAN GPORT %08X home module %d is not"
                                        " local module %d; must manipulate STP"
                                        " state on the home module\n"),
                          port, tgtModId, myModId));
                rv = BCM_E_PARAM;
            }

            if (BCM_SUCCESS(rv)) {
                /* build a vid list, based on the vector */
                rv = _bcm_fe2000_vlan_port_vlan_vector_get(unit,
                                                           vlanPort.vlan_port_id,
                                                           vlanVec);
                if (BCM_FAILURE(rv)) {
                    VLAN_ERR((VLAN_FUNCMSG1("failed to get vlan vector for "
                                            "gport 0x%08x: %d %s\n"),
                              port, rv, bcm_errmsg(rv)));
                }
            }

            if (BCM_SUCCESS(rv)) {
                for (idx = BCM_VLAN_MIN + 1; idx < BCM_VLAN_COUNT; idx++) {
                    fastSets[idx] = !!BCM_VLAN_VEC_GET(vlanVec, idx);
                }

                /* if the port's native vid is in the vector, set the untagged
                 * vlan's stp state as well.
                 */
                if (BCM_VLAN_VEC_GET(vlanVec,
                                     _vlan_state[unit]->nvid[phyPort]))
                {
                    fastSets[_BCM_VLAN_G2P3_UNTAGGED_VID] = 1;
                }

            }

            fastSets[vlanPort.match_vlan] = 1;

        }
        break;
#endif
        default:
            VLAN_ERR((VLAN_FUNCMSG1("unknown ucode type=%d\n"),
                      SOC_SBX_CONTROL(unit)->ucodetype));
            rv = BCM_E_CONFIG;
        }
    }  else {
        int idx;

        /* Specail cases for physical port,vids -
         * vid == 0, set all 4K vids
         * vid >= BCM_VLAN_MAX, set all non-gport vids
         */
        if (vid == 0) {
            for (idx=1; idx<BCM_VLAN_COUNT; idx++) {
                fastSets[idx] = 1;
            }
        } else if (vid >= BCM_VLAN_MAX) {

            for (idx=1; idx<BCM_VLAN_COUNT && BCM_SUCCESS(rv); idx++) {
                switch(SOC_SBX_CONTROL(unit)->ucodetype) {

#ifdef BCM_FE2000_P3_SUPPORT
                case SOC_SBX_UCODE_TYPE_G2P3:
                {
                    soc_sbx_g2p3_pv2e_t pv2e;
                    rv = SOC_SBX_G2P3_PV2E_GET(unit, port, idx, &pv2e);
                    fastSets[idx] = (pv2e.vlan < BCM_VLAN_COUNT);
                }
                break;
#endif
                default:
                    VLAN_ERR((VLAN_FUNCMSG1("unknown ucode type=%d\n"),
                              SOC_SBX_CONTROL(unit)->ucodetype));
                    rv = BCM_E_CONFIG;
                    break;
                }  /* switch(SOC_SBX_CONTROL(unit)->ucodetype) */
            }

        } else {
            /* simple case - set exactly the vid given */
            fastSets[vid] = 1;
        }
    }


    if (BCM_SUCCESS(rv)) {

        /* set the ingress and egress stp state */
        rv = _bcm_fe2000_g2p3_stp_fast_set(unit, phyPort, stp_state, fastSets);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("failed to get set vecro  gport 0x%08x: "
                                    " %d %s\n"),
                      port, rv, bcm_errmsg(rv)));
        }
    }

    if (fastSets) {
        sal_free(fastSets);
    }
    return rv;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_stp_set
 *   Purpose
 *      Set the spanning tree state for the port on the VID
 *   Parameters
 *      (in) int unit = unit number whose VID is to be checked
 *      (in) bcm_vlan_t vid = VID whose STP state is to be changed
 *      (in) bcm_port_t port = port whose STP state is to be changed
 *      (in) int stp_state = new stp state for the port on the VID
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This just calls the STG functions to do its job.  The API definition
 *      says this affects this port on ALL VIDs of the STG, despite the fact
 *      that a specific VID is provided.
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_stp_set"
int
bcm_fe2000_vlan_stp_set(int unit,
                        bcm_vlan_t vid,
                        bcm_port_t port,
                        int stp_state)
{
    int                   result;           /* local result code */
    bcm_stg_t             stg;              /* working spanning tree group */
    bcm_pbmp_t            pbmp;             /* working port bitmap */
    bcm_pbmp_t            ubmp;             /* working untagged port bitmap */
    uint32                vsi;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,%d,%d) enter\n"),
               unit,
               vid,
               port,
               stp_state));

    /* Check unit valid; return unit error if not */
    /* ignore vsi check for mim gport */
    if ((!BCM_GPORT_IS_MIM_PORT(port)) &&
        (!BCM_GPORT_IS_MPLS_PORT(port))) {
        /* Check unit valid; return unit error if not */
        VLAN_UNIT_VALID_CHECK;
        
        /* Check for proper initialisation */
        VLAN_UNIT_INIT_CHECK;
    }

    if (BCM_GPORT_IS_MIM_PORT(port)) {
         return (_bcm_fe2000_mim_stp_update(unit, port, &stp_state, TRUE));
    } else if (BCM_GPORT_IS_MPLS_PORT(port)) {
        return (_bcm_fe2000_mpls_vpn_stp_set(unit, vid, port, stp_state));
    }

    /* check for alternate stp set implemenation - */
    if (VLAN_CTL_GET(unit, bcmVlanIndependentStp)) {
        return _bcm_fe2000_vlan_independent_stp_set(unit, port, vid,
                                                    stp_state);
    }

    /* check for valid parameter values */
    if ((BCM_VLAN_VID_DISABLE != vid) && (vid <= 0)) {
        VLAN_ERR((VLAN_FUNCMSG1("invalid VID %04X on unit %d\n"),
                  vid,
                  unit));
         return BCM_E_PARAM;
    }

    /* check the port type, regardless of the vid value */
    if (BCM_GPORT_IS_VLAN_PORT(port)) {
        return (_bcm_fe2000_vlan_port_stp_set_get(unit, vid,
                                                  port, &stp_state, TRUE));
     }

    /* if we get here, and the port wasn't an gport, validate the vid */
    if (vid > BCM_VLAN_MAX) {
        return BCM_E_PARAM;
    }

    vsi = 0;
    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
    {
        soc_sbx_g2p3_pv2e_t pv2e;
        result = SOC_SBX_G2P3_PV2E_GET(unit, port, vid, &pv2e);
        vsi = pv2e.vlan;
    }
    break;
#endif

    default:
        VLAN_ERR((VLAN_FUNCMSG1("unknown ucode type=%d\n"),
                  SOC_SBX_CONTROL(unit)->ucodetype));
        return BCM_E_CONFIG;
        break;
    }  /* switch(SOC_SBX_CONTROL(unit)->ucodetype) */

    if (BCM_FAILURE(result)) {
        VLAN_ERR((VLAN_FUNCMSG1("Error %d (%s) failed to read pv2e\n"),
                  result, bcm_errmsg(result)));
        return  result;
    }

    /* read the port information for this VID  */
    result = _bcm_fe2000_vlan_port_fetch(unit, SBX_VSI_TO_VID(vsi),
                                         &pbmp, &ubmp);

    /* Update the port's STP state on this VID if all is well */
    if (BCM_E_NONE == result) {
        /* All of the data were read successfully */
        if (BCM_PBMP_MEMBER(pbmp,port)) {
            /* Port is on this VID */

            result = _bcm_fe2000_stg_map_get(unit, SBX_VSI_TO_VID(vsi), &stg);
            if (BCM_E_NONE == result) {
                result = bcm_fe2000_stg_stp_set(unit,
                                                stg,
                                                port,
                                                stp_state);
                if (BCM_E_NONE != result) {
                    /* something went wrong; report it */
                    VLAN_ERR((VLAN_FUNCMSG1("bcm_fe2000_stg_stp_set"
                                            "(%d,%d,%04X,%d)"
                                            " returned %d (%s)\n"),
                              unit,
                              stg,
                              port,
                              stp_state,
                              result,
                              _SHR_ERRMSG(result)));
                }
            } else { /* if (BCM_E_NONE == result) */
                /* Failed to read the STG information */
                VLAN_ERR((VLAN_FUNCMSG1("_bcm_fe2000_stg_map_get(%d,%03X,*)"
                                        " returned %d (%s)\n"),
                          unit,
                          SBX_VSI_TO_VID(vsi),
                          result,
                          _SHR_ERRMSG(result)));
            } /* if (BCM_E_NONE == result) */
        } else { /* if (BCM_PBMP_MEMBER(pbmp,port)) */
            /* not a member, report error */
            result = BCM_E_NOT_FOUND;
            VLAN_ERR((VLAN_FUNCMSG1("unit %d port %d is not a member of"
                                    " VLAN %04X\n"),
                      unit,
                      port,
                      vid));
        } /* if (BCM_PBMP_MEMBER(pbmp,port)) */
    } /* if (BCM_E_NONE == result) */

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,%d,%d) return %d (%s)\n"),
               unit,
               vid,
               port,
               stp_state,
               result,
               _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_vector_stp_set
 *   Purpose
 *      Set the spanning tree state for the port on the vlan vector
 *   Parameters
 *      (in) int unit = unit number whose VID is to be checked
 *      (in) bcm_vlan_vector_t vlan_vector = vlan_vector whose STP state is to be changed
 *      (in) bcm_port_t port = port whose STP state is to be changed
 *      (in) int stp_state = new stp state for the port on the vlan vector
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_vector_stp_set"
int
bcm_fe2000_vlan_vector_stp_set(int unit,
                               bcm_vlan_vector_t vlan_vector,
                               bcm_port_t port,
                               int stp_state)
{
    int                   result;           /* local result code */
    bcm_vlan_t            vid;
    int                   *fastStpSets;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,%d,%d) enter\n"),
               unit,
               vlan_vector,
               port,
               stp_state));

    if (BCM_GPORT_IS_MIM_PORT(port)) {
         return BCM_E_PARAM;
    }

    if (BCM_GPORT_IS_VLAN_PORT(port)) {
         return BCM_E_PARAM;
    }

    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
    {
        fastStpSets = sal_alloc(sizeof(uint32_t) * (BCM_VLAN_MAX + 1),
                                "fastStpSets");

        if (fastStpSets == NULL)
        {
            VLAN_ERR((VLAN_FUNCMSG1("unable to alloc %d bytes for"
                                    " unit %d vlan temp data\n"),
                      sizeof(uint32_t) * (BCM_VLAN_MAX + 1),unit));
            sal_free(fastStpSets);
            return BCM_E_MEMORY;
        } else {
            sal_memset(fastStpSets, 0x00, sizeof(uint32_t) * (BCM_VLAN_MAX + 1));
        }

        for (vid = BCM_VLAN_MIN + 1; vid < BCM_VLAN_MAX; vid++) {

           if (BCM_VLAN_VEC_GET(vlan_vector, vid)) {
              fastStpSets[vid] = 1;
           }
        }

        result = _bcm_fe2000_g2p3_stp_fast_set(unit,
                                               port,
                                               stp_state,
                                               fastStpSets);
        if (BCM_FAILURE(result)) {
            VLAN_ERR((VLAN_FUNCMSG1("failed to set pv2e[*] stpstate: %d %s\n"),
                      result, bcm_errmsg(result)));
        }

        sal_free(fastStpSets);
    }
    break;
#endif
    default:
        VLAN_ERR((VLAN_FUNCMSG1("unknown ucode type=%d\n"),
                  SOC_SBX_CONTROL(unit)->ucodetype));
        return BCM_E_CONFIG;
        break; 
    }  /* switch(SOC_SBX_CONTROL(unit)->ucodetype) */
    
    if (BCM_FAILURE(result)) {
        VLAN_ERR((VLAN_FUNCMSG1("Error %d (%s) failed to read pv2e\n"),
                  result, bcm_errmsg(result)));
        return  result;
    }

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,%d,%d) return %d (%s)\n"),
               unit,
               vlan_vector,
               port,
               stp_state,
               result,
               _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_stp_get
 *   Purpose
 *      Get the spanning tree state for the port on the VID
 *   Parameters
 *      (in) int unit = unit number whose VID is to be checked
 *      (in) bcm_vlan_t vid = VID whose STP state is to be fetched
 *      (in) bcm_port_t port = port whose STP state is to be fetched
 *      (out) int *stp_state = place to put STP state of port on VID
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This just calls the STG functions to do its job.  It only returns the
 *      STG state for the port in the specified VID, even if there are other
 *      VIDs in the STG and they may have different states.
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_stp_get"
int
bcm_fe2000_vlan_stp_get(int unit,
                        bcm_vlan_t vid,
                        bcm_port_t port,
                        int *stp_state)
{
    int                   result;           /* local result code */
    bcm_stg_t             stg;              /* working spanning tree group */
    bcm_pbmp_t            pbmp;             /* working port bitmap */
    bcm_pbmp_t            ubmp;             /* working untagged port bitmap */

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,%d,*) enter\n"),
               unit,
               vid,
               port));

    /* Check unit valid; return unit error if not */
    /* ignore vsi check for mim/mpls gport */
    if ((!BCM_GPORT_IS_MIM_PORT(port)) &&
        (!BCM_GPORT_IS_MPLS_PORT(port))){
        /* Check unit valid; return unit error if not */
        VLAN_UNIT_VALID_CHECK;
        
        /* Check for proper initialisation */
        VLAN_UNIT_INIT_CHECK;
    }

    if (BCM_GPORT_IS_MIM_PORT(port)) {
        return (_bcm_fe2000_mim_stp_update(unit, port, stp_state, FALSE));
    } else if (BCM_GPORT_IS_MPLS_PORT(port)) {
        return (_bcm_fe2000_mpls_vpn_stp_get(unit, vid, port, stp_state));
    }

    /* check for valid parameter values */
    if ((BCM_VLAN_VID_DISABLE != vid) && (vid <= 0)) {
        VLAN_ERR((VLAN_FUNCMSG1("invalid VID %04X on unit %d\n"),
                  vid,
                  unit));
        return BCM_E_PARAM;
    }

    if (!stp_state) {
        VLAN_ERR((VLAN_FUNCMSG1("NULL pointer for outbound argument\n")));
        return BCM_E_PARAM;
    }

    /* check for alternate stp set implemenation - */
    if (VLAN_CTL_GET(unit, bcmVlanIndependentStp)) {
        return _bcm_fe2000_vlan_independent_stp_get(unit, port, vid,
                                                    stp_state);
    }

    /* check the port type, regardless of the vid value */
    if (BCM_GPORT_IS_VLAN_PORT(port)) {
        return (_bcm_fe2000_vlan_port_stp_set_get(unit, vid,
                                                  port, stp_state, FALSE));
    }

    /* if we get here, and the port wasn't an gport, validate the vid */
    if (vid > BCM_VLAN_MAX) {
        return BCM_E_PARAM;
    }

    /* read the port information for this vid */
    result = _bcm_fe2000_vlan_port_fetch(unit, vid, &pbmp, &ubmp);

    /* Get the port's STP state on this VID if all is well */
    if (BCM_E_NONE == result) {
        /* All of the data were read successfully */
        if (BCM_PBMP_MEMBER(pbmp,port)) {
            /* Port is on this VID */
            result =_bcm_fe2000_stg_map_get(unit,vid,&stg);
            if (BCM_E_NONE == result) {
                /* Successful read of STG; get the current state */
                result = bcm_fe2000_stg_stp_get(unit, stg, port, stp_state);
                if (BCM_E_NONE != result) {
                    VLAN_ERR((VLAN_FUNCMSG1("bcm_fe2000_stg_stp_get"
                                            "(%d,%d,%d,*)"
                                            " returned %d (%s)\n"),
                              unit,
                              stg,
                              port,
                              result,
                              _SHR_ERRMSG(result)));
                }
            } else { /* if (BCM_E_NONE == result) */
                /* Failed to read the STG information */
                VLAN_ERR((VLAN_FUNCMSG1("_bcm_fe2000_stg_map_get(%d,%03X,*)"
                                        " returned %d (%s)\n"),
                          unit,
                          vid,
                          result,
                          _SHR_ERRMSG(result)));
            } /* if (BCM_E_NONE == result) */
        } else { /* if (BCM_PBMP_MEMBER(pbmp,port)) */
            /* not a member, report error */
            result = BCM_E_NOT_FOUND;
            VLAN_ERR((VLAN_FUNCMSG1("unit %d port %d is not a member of"
                                    " VLAN %04X\n"),
                      unit,
                      port,
                      vid));
        } /* if (BCM_PBMP_MEMBER(pbmp,port)) */
    } /* if (BCM_E_NONE == result) */

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,%d,&(%d)) return %d (%s)\n"),
               unit,
               vid,
               port,
               *stp_state,
               result,
               _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

int
bcm_fe2000_vlan_mac_add(int unit,
                        bcm_mac_t mac,
                        bcm_vlan_t vid,
                        int prio)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_mac_delete(int unit,
                           bcm_mac_t mac)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_vlan_mac_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_fe2000_vlan_translate_add
 *   Purpose
 *      Add translation on ingress from one VID to another
 *   Parameters
 *      (in) int unit = unit number on which the VID is to be translated
 *      (in) int port = port number on which the VID is to be translated
 *      (in) bcm_vlan_t old_vid = original VID
 *      (in) bcm_vlan_t new_vid = translated VID
 *      (in) int prio = new priority (-1 to leave unchanged)
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Translation can only be applied to enabled port+VID.
 *      Translation state is lost when port+VID is disabled.
 *      Apparently, this feature is somehow meant to insert tags rather than
 *      simple 1:1 translation of tags. [maybe not?]
 */




#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_translate_add"
int
bcm_fe2000_vlan_translate_add(int unit,
                              int port,
                              bcm_vlan_t old_vid,
                              bcm_vlan_t new_vid,
                              int prio)
{
    int                   result;           /* local result code */

    VLAN_VERB((VLAN_FUNCMSG("(%d,%d,%04X,%04X,%d) enter\n"),
               unit,
               port,
               old_vid,
               new_vid,
               prio));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* check for valid parameter values */
    if ((_sbx_vlan_max_vid_value < old_vid) || (0 == old_vid)) {
        VLAN_ERR((VLAN_FUNCMSG1("invalid original VLAN %04X\n"),
                  old_vid));
        return BCM_E_PARAM;
    }
    if ((_sbx_vlan_max_vid_value < new_vid) || (0 == new_vid)) {
        VLAN_ERR((VLAN_FUNCMSG1("invalid new VLAN %04X\n"),
                  new_vid));
        return BCM_E_PARAM;
    }

    /* bail if the port isn't allowed */
    if (SBX_MAX_PORTS <= port) {
        /* not in the ports array */
        return BCM_E_PARAM;
    }

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* set translation for this port so old_vid -> new_vid */
    result = _bcm_fe2000_vlan_translate_set(unit,
                                            port,
                                            old_vid,
                                            new_vid,
                                            prio,
                                            TRUE);

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%d,%04X,%04X,%d) return %d (%s)\n"),
               unit,
               port,
               old_vid,
               new_vid,
               prio,
               result,
               _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_translate_delete
 *   Purpose
 *      Delete translation on ingress from one VID to another
 *   Parameters
 *      (in) int unit = unit number on which the VID is to not be translated
 *      (in) int port = port number on which the VID is to not be translated
 *      (in) bcm_vlan_t old_vid = original VID
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Translation can only be applied to enabled port+VID.
 *      Translation state is lost when port+VID is disabled.
 *      Apparently, this feature is somehow meant to insert tags rather than
 *      simple 1:1 translation of tags. [maybe not?]
 */



#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_translate_delete"
int
bcm_fe2000_vlan_translate_delete(int unit,
                                 int port,
                                 bcm_vlan_t old_vid)
{
    int                   result;           /* local result code */
    int                   tempres;          /* called functin result code */

    VLAN_VERB((VLAN_FUNCMSG("(%d,%d,%04X) enter\n"),
               unit,
               port,
               old_vid));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* check for valid parameter values */
    if (((_sbx_vlan_max_vid_value < old_vid) || (0 == old_vid)) &&
        (BCM_VLAN_NONE != old_vid)) {
        VLAN_ERR((VLAN_FUNCMSG1("invalid original VLAN %04X\n"),
                  old_vid));
        return BCM_E_PARAM;
    }

    /* bail if the port isn't allowed */
    if (SBX_MAX_PORTS <= port) {
        /* not in the ports array */
        return BCM_E_PARAM;
    }

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* hope for success */
    result = BCM_E_NONE;

    /* adjust translation as requested by caller */
    if (-1 == port) {
        /* port wildcarded; remove translation for all ports on the VID */
        if (BCM_VLAN_NONE != old_vid) {
            /* old vid is not wildcarded; go ahead */
            PBMP_ALL_ITER(unit,port) {
                /* set translation for all ports so old_vid -> old_vid */
                tempres = _bcm_fe2000_vlan_translate_set(unit,
                                                         port,
                                                         old_vid,
                                                         old_vid,
                                                         -1 /* no prio chg */,
                                                         FALSE);
                if (BCM_E_NONE != tempres) {
                    /* something went wrong; report it */
                    result = tempres;
                    VLAN_ERR((VLAN_FUNCMSG1("unable to clear translation for"
                                            " unit %d port %d VLAN %04X:"
                                            " %d (%s)\n"),
                              unit,
                              port,
                              old_vid,
                              result,
                              _SHR_ERRMSG(result)));
                }
            } /* PBMP_ALL_ITER(unit,port) */
        } else {
            /* can't wildcard both here (use bcm_vlan_translate_delete_all) */
            result = BCM_E_PARAM;
            VLAN_ERR((VLAN_FUNCMSG1("wildcarding both port and VLAN not"
                                    " supported on unit %d\n"),
                      unit));
        }
    } else if (BCM_VLAN_NONE == old_vid) {
        /* VID wildcarded; remove translation for all VIDs on the port */
        for (old_vid = 1; old_vid <= _sbx_vlan_max_vid_value; old_vid++) {
            /* set translation for this port so each old_vid -> itself */
            tempres = _bcm_fe2000_vlan_translate_set(unit,
                                                     port,
                                                     old_vid,
                                                     old_vid,
                                                     -1 /* no priority chg*/,
                                                     FALSE);
            if (BCM_E_NONE != tempres) {
                /* something went wrong; report it */
                result = tempres;
                VLAN_ERR((VLAN_FUNCMSG1("unable to clear translation for"
                                        " unit %d port %d VLAN %04X:"
                                        " %d (%s)\n"),
                          unit,
                          port,
                          old_vid,
                          result,
                          _SHR_ERRMSG(result)));
            }
        } /* for (old_vid = 1; old_vid < _vlan_max_vid_value; old_vid++) */
    } else {
        /* set translation for this port so old_vid -> old_vid */
        result = _bcm_fe2000_vlan_translate_set(unit,
                                                port,
                                                old_vid,
                                                old_vid,
                                                -1 /* no priority change */,
                                                FALSE);
    } /* adjust translation as requested by caller */

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%d,%04X) return %d (%s)\n"),
               unit,
               port,
               old_vid,
               result,
               _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_translate_delete_all
 *   Purpose
 *      Delete all translation on ingress from one VID to another
 *   Parameters
 *      (in) int unit = unit number where ingress VID translation is to purged
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The only fatal error here is semtake/semgive failure.  Other errors
 *      will be ignored while we clear the translations, and the last one to
 *      occur will be reported to the caller.
 *      Takes and releases the unit semaphore per port so other tasks can have
 *      a chance to operate (really a lot of work has to be done).
 *      Apparently, this feature is somehow meant to insert tags rather than
 *      simple 1:1 translation of tags. [maybe not?]
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_translate_delete_all"
int
bcm_fe2000_vlan_translate_delete_all(int unit)
{
    int                   result;           /* local result code */
    int                   rv;               /* called function result code */
    unsigned int          vIndex;           /* working VID index */
    bcm_port_t            pIndex;           /* working port index */

    VLAN_VERB((VLAN_FUNCMSG("(%d) enter\n"),
               unit));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* Optimistically assume success from here */
    result = BCM_E_NONE;

    PBMP_ALL_ITER(unit,pIndex) {
        /* Claim the lock for VID work on this unit */
        if (sal_mutex_take(_vlan_state[unit]->lock, sal_mutex_FOREVER)) {
            /* Cound not obtain unit lock  */
            result =  BCM_E_INTERNAL;
            VLAN_ERR((VLAN_FUNCMSG1("unable to obtain VLAN lock on unit %d\n"),
                      unit));
            break;
        }
        /* reset translation on all VIDs for this port */
        for (vIndex = 1; _sbx_vlan_max_vid_value >= vIndex; vIndex++) {
            /* set translation for this port so old_vid -> new_vid */
            rv = _bcm_fe2000_vlan_translate_set(unit,
                                                pIndex,
                                                vIndex,
                                                vIndex,
                                                -1 /* no priority chng */,
                                                FALSE);
            if (BCM_E_NONE != rv) {
                /* an error occurred; keep track of the last one */
                result = rv;
                VLAN_ERR((VLAN_FUNCMSG1("unable to clear translation for"
                                        " unit %d port %d VLAN %04X:"
                                        " %d (%s)\n"),
                          unit,
                          pIndex,
                          vIndex,
                          result,
                          _SHR_ERRMSG(result)));
            }
        } /* for (vIndex = 1; _vlan_max_vid_value > vIndex; vIndex++) */
        /* Release the VID lock for this unit */
        if (sal_mutex_give(_vlan_state[unit]->lock)) {
            /* Could not release unit lock */
            VLAN_ERR((VLAN_FUNCMSG1("unable to release VLAN lock on unit %d\n"),
                      unit));
            result = BCM_E_INTERNAL;
            break;
        }
    } /* PBMP_ALL_ITER(unit,pIndex) */

    VLAN_VERB((VLAN_FUNCMSG("(%d) return %d (%s)\n"),
               unit,
               result,
               _SHR_ERRMSG(result)));

    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_translate_range_add
 *   Purpose
 *      Add translation on ingress from one VID range to another
 *   Parameters
 *      (in) int unit = unit number on which the VID is to be translated
 *      (in) int port = port number on which the VID is to be translated
 *      (in) bcm_vlan_t old_vid_low = original VID low
 *      (in) bcm_vlan_t old_vid_high = original VID high
 *      (in) bcm_vlan_t new_vid = translated VID
 *      (in) int prio = new priority (-1 to leave unchanged)
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Target VID range is single -- so old_vid_low -> new_vid; old_vid_low +
 *      1 -> new_vid, and so on through old_vid_high -> new_vid.
 *      Apparently, this feature is somehow meant to insert tags rather than
 *      simple 1:1 translation of tags. [maybe not?]
 */




#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_translate_range_add"
int
bcm_fe2000_vlan_translate_range_add(int unit,
                                    int port,
                                    bcm_vlan_t old_vid_low,
                                    bcm_vlan_t old_vid_high,
                                    bcm_vlan_t new_vid,
                                    int prio)
{
    unsigned int          vIndex;           /* working VID index */
    int                   rv;               /* called result code */
    int                   result;           /* local result code */

    VLAN_VERB((VLAN_FUNCMSG("(%d,%d,%04X,%04X,%04X,%d) enter\n"),
               unit,
               port,
               old_vid_low,
               old_vid_high,
               new_vid,
               prio));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* check for valid parameter values */
    if ((_sbx_vlan_max_vid_value < old_vid_low) || (0 == old_vid_low)) {
        VLAN_ERR((VLAN_FUNCMSG1("invalid low original VLAN %04X\n"),
                  old_vid_low));
        return BCM_E_PARAM;
    }
    if ((_sbx_vlan_max_vid_value < old_vid_high) || (0 == old_vid_high)) {
        VLAN_ERR((VLAN_FUNCMSG1("invalid high original VLAN %04X\n"),
                  old_vid_high));
        return BCM_E_PARAM;
    }
    if ((_sbx_vlan_max_vid_value < new_vid) || (0 == new_vid)) {
        VLAN_ERR((VLAN_FUNCMSG1("invalid new VLAN %04X\n"),
                  new_vid));
        return BCM_E_PARAM;
    }
    if ((old_vid_low > old_vid_high)) {
        VLAN_ERR((VLAN_FUNCMSG1("low original VID %04X is > high original"
                                " VID %04X\n"),
                  old_vid_low,
                  old_vid_high));
        return BCM_E_PARAM;
    }

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* Optimistically assume success from here */
    result = BCM_E_NONE;

    /* map old_vid_low to new_vid, old_vid_low + 1 to new_vid + 1, &c */
    for (vIndex = old_vid_low; old_vid_high >= vIndex; vIndex++) {
        /* set translation for this port so old_vid -> new_vid */
        rv = _bcm_fe2000_vlan_translate_set(unit,
                                            port,
                                            vIndex,
                                            new_vid,
                                            prio,
                                            TRUE);
        if (BCM_E_NONE != rv) {
            /* an error occurred; keep track of the last one */
            result = rv;
            VLAN_ERR((VLAN_FUNCMSG1("unable to set translation for unit %d"
                                    " port %d VLAN %04X: %d (%s)\n"),
                      unit,
                      port,
                      vIndex,
                      result,
                      _SHR_ERRMSG(result)));
        }
        
        /* new_vid++; */
    } /* for (vIndex = old_vid_low; old_vid_high >= vIndex; vIndex++) */

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%d,%04X,%04X,%04X,%d) return %d (%s)\n"),
               unit,
               port,
               old_vid_low,
               old_vid_high,
               new_vid,
               prio,
               result,
               _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_translate_range_delete
 *   Purpose
 *      Remove translation on ingress from one VID range to another
 *   Parameters
 *      (in) int unit = unit number on which the VID is to be translated
 *      (in) int port = port number on which the VID is to be translated
 *      (in) bcm_vlan_t old_vid_low = original VID low
 *      (in) bcm_vlan_t old_vid_high = original VID high
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Target VID range is single -- so old_vid_low -> new_vid; old_vid_low +
 *      1 -> new_vid, and so on through old_vid_high -> new_vid.
 *      Apparently, this feature is somehow meant to insert tags rather than
 *      simple 1:1 translation of tags. [maybe not?]
 */




#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_translate_range_delete"
int
bcm_fe2000_vlan_translate_range_delete(int unit,
                                       int port,
                                       bcm_vlan_t old_vid_low,
                                       bcm_vlan_t old_vid_high)
{
    unsigned int          vIndex;           /* working VID index */
    int                   rv;               /* called result code */
    int                   result;           /* local result code */

    VLAN_VERB((VLAN_FUNCMSG("(%d,%d,%04X,%04X) enter\n"),
               unit,
               port,
               old_vid_low,
               old_vid_high));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* check for valid parameter values */
    if ((_sbx_vlan_max_vid_value < old_vid_low) || (0 == old_vid_low)) {
        VLAN_ERR((VLAN_FUNCMSG1("invalid low original VLAN %04X\n"),
                  old_vid_low));
        return BCM_E_PARAM;
    }
    if ((_sbx_vlan_max_vid_value < old_vid_high) || (0 == old_vid_high)) {
        VLAN_ERR((VLAN_FUNCMSG1("invalid high original VLAN %04X\n"),
                  old_vid_high));
        return BCM_E_PARAM;
    }
    if ((old_vid_low > old_vid_high)) {
        VLAN_ERR((VLAN_FUNCMSG1("low original VID %04X is > high original"
                                " VID %04X\n"),
                  old_vid_low,
                  old_vid_high));
        return BCM_E_PARAM;
    }

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* Optimistically assume success from here */
    result = BCM_E_NONE;

    /* map the vid range members to themselves */
    for (vIndex = old_vid_low; old_vid_high >= vIndex; vIndex++) {
        /* set translation for this port so old_vid -> new_vid */
        rv = _bcm_fe2000_vlan_translate_set(unit,
                                            port,
                                            vIndex,
                                            vIndex,
                                            -1,
                                            FALSE);
        if (BCM_E_NONE != rv) {
            /* an error occurred; keep track of the last one */
            result = rv;
            VLAN_ERR((VLAN_FUNCMSG1("unable to clear translation for unit %d"
                                    " port %d VLAN %04X: %d (%s)\n"),
                      unit,
                      port,
                      vIndex,
                      result,
                      _SHR_ERRMSG(result)));
        }
    } /* for (vIndex = old_vid_low; old_vid_high >= vIndex; vIndex++) */

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%d,%04X,%04X) return %d (%s)\n"),
               unit,
               port,
               old_vid_low,
               old_vid_high,
               result,
               _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *  Since we don't strictly support the VLAN 'range' concept, the translation
 *  could vary between elements of the specified 'range'.  I suppose we could
 *  just return the first one (assuming it's translated at all, but there is
 *  no way to indicate that it is not with this function)...
 *
 *  Probably better to avoid the confusion and refuse to do it.
 */
int bcm_fe2000_vlan_translate_range_get (int unit, bcm_port_t port,
                                      bcm_vlan_t old_vlan_low,
                                      bcm_vlan_t old_vlan_high,
                                      bcm_vlan_t *new_vid, int *prio)
{
    return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_fe2000_vlan_translate_range_delete_all
 *   Purpose
 *      Delete all translation ranges on ingress from one VID to another
 *   Parameters
 *      (in) int unit = unit number where ingress VID translation is to purged
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Since we implement ranges as a set of individual entries, this merely
 *      calls bcm_fe2000_vlan_translate_delete_all and returns its result.
 *      Apparently, this feature is somehow meant to insert tags rather than
 *      simple 1:1 translation of tags. [maybe not?]
 */
int
bcm_fe2000_vlan_translate_range_delete_all(int unit)
{
    return bcm_fe2000_vlan_translate_delete_all(unit);
}

/*
 *   Function
 *      bcm_fe2000_vlan_translate_egress_get
 *   Purpose
 *      Get the VLAN egress translation for the specified port and source VLAN ID.
 *   Parameters
 *      (in) int unit = unit number on which the VID is translated
 *      (in) int port = port number on which the VID is translated
 *      (in) bcm_vlan_t old_vid = original VID
 *      (out) bcm_vlan_t new_vid = translated VID
 *      (out) int prio = priority
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *     
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_translate_egress_get"
int
bcm_fe2000_vlan_translate_egress_get(int unit,
                                     int port,
                                     bcm_vlan_t old_vid,
                                     bcm_vlan_t * new_vid,
                                     int * prio)
{
    int rv;
    uint32 eteEncapIndex, eteL2Index;
    soc_sbx_g2p3_evp2e_t    p3egrVlanPort2Etc;
    soc_sbx_g2p3_etel2_t    p3eteL2;
    soc_sbx_g2p3_eteencap_t p3eteEncap;


    VLAN_VERB((VLAN_FUNCMSG("(%d,%d,%d) enter\n"), unit, port, old_vid));

    VLAN_UNIT_VALID_CHECK;
    VLAN_UNIT_INIT_CHECK;

    if ((old_vid <= 0) || (old_vid > BCM_VLAN_MAX) ||
        (!SOC_PORT_VALID(unit, port))) {
        return BCM_E_PARAM;
    }

    VLAN_LOCK_TAKE;

    /* perform the operation */
    *prio = 0;  /* not supported */

    rv = soc_sbx_g2p3_evp2e_get(unit,
                                old_vid,
                                port,
                                &p3egrVlanPort2Etc);

    if (rv == BCM_E_NONE) {
        eteEncapIndex = p3egrVlanPort2Etc.eteptr;
        rv = soc_sbx_g2p3_eteencap_get(unit,
                                       eteEncapIndex,
                                       &p3eteEncap);
    }

    if (rv == BCM_E_NONE) {
        if (p3eteEncap.l2ete) {
            eteL2Index = p3eteEncap.l2ete;
        } else {
            eteL2Index = eteEncapIndex;
        }
        rv = soc_sbx_g2p3_etel2_get(unit,
                                    eteL2Index,
                                    &p3eteL2);
    }

    if (rv == BCM_E_NONE) {
        if (p3eteL2.usevid) {
            *new_vid = p3eteL2.vid;
        } else {
            *new_vid = old_vid;
        }
    }

    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,*) return %d (%s)\n"),
               unit, rv, _SHR_ERRMSG(rv)));

    return rv;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_translate_get
 *   Purpose
 *      Get the VLAN translation for a given port and source VLAN ID.
 *   Parameters
 *      (in) int unit = unit number on which the VID is translated
 *      (in) int port = port number on which the VID is translated
 *      (in) bcm_vlan_t old_vid = original VID
 *      (out) bcm_vlan_t new_vid = translated VID
 *      (out) int prio = priority
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_translate_get"
int
bcm_fe2000_vlan_translate_get(int unit,
                              int port,
                              bcm_vlan_t old_vid,
                              bcm_vlan_t * new_vid,
                              int * prio)
{
    int rv;
    soc_sbx_g2p3_pv2e_t      p3pVid2Etc;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%d,%d) enter\n"), unit, port, old_vid));
  
    VLAN_UNIT_VALID_CHECK;
    VLAN_UNIT_INIT_CHECK;

    if ((old_vid <= 0) || (old_vid > BCM_VLAN_MAX) ||
        (!SOC_PORT_VALID(unit, port))) {
        return BCM_E_PARAM;
    }

    VLAN_LOCK_TAKE;

    /* perform the operation */
    *prio = 0;  /* not supported */

    rv = SOC_SBX_G2P3_PV2E_GET(unit,
                               port,
                               old_vid,
                               &p3pVid2Etc);
    if (rv == BCM_E_NONE) {
        *new_vid = p3pVid2Etc.vlan;
    }

    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,*) return %d (%s)\n"),
               unit, rv, _SHR_ERRMSG(rv)));

    return rv;
}
#undef _VLAN_FUNC_NAME


int
bcm_fe2000_vlan_translate_action_range_add(int unit,
                                           bcm_gport_t port,
                                           bcm_vlan_t outer_vlan_low,
                                           bcm_vlan_t outer_vlan_high,
                                           bcm_vlan_t inner_vlan_low,
                                           bcm_vlan_t inner_vlan_high,
                                           bcm_vlan_action_set_t *action)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_translate_action_range_delete(int unit,
                                              bcm_gport_t port,
                                              bcm_vlan_t outer_vlan_low,
                                              bcm_vlan_t outer_vlan_high,
                                              bcm_vlan_t inner_vlan_low,
                                              bcm_vlan_t inner_vlan_high)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_translate_action_range_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_vlan_dtag_get(int unit,
                         int port,
                         bcm_vlan_t old_vid,
                         bcm_vlan_t * new_vid,
                         int * prio)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_vlan_dtag_traverse(int unit,
                  bcm_vlan_dtag_traverse_cb cb,
                  void *user_data)
{
    return BCM_E_UNAVAIL;
}



/*
 *   Function
 *      bcm_fe2000_vlan_translate_egress_add
 *   Purpose
 *      Add translation on egress from one VID to another
 *   Parameters
 *      (in) int unit = unit number on which the VID is to be translated
 *      (in) int port = port number on which the VID is to be translated
 *      (in) bcm_vlan_t old_vid = original VID
 *      (in) bcm_vlan_t new_vid = translated VID
 *      (in) int prio = new priority (-1 to leave unchanged)
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Translation can only be applied to enabled port+VID.
 *      Translation state is lost when port+VID is disabled.
 */




#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_translate_egress_add"
int
bcm_fe2000_vlan_translate_egress_add(int unit,
                                     int port,
                                     bcm_vlan_t old_vid,
                                     bcm_vlan_t new_vid,
                                     int prio)
{
    int                     result;         /* local result code */

    VLAN_VERB((VLAN_FUNCMSG("(%d,%d,%04X,%04X,%d) enter\n"),
               unit,
               port,
               old_vid,
               new_vid,
               prio));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* check for valid parameter values */
    if ((_sbx_vlan_max_vid_value < old_vid) || (0 == old_vid)) {
        VLAN_ERR((VLAN_FUNCMSG1("old_vid %04X is out of range on unit %d\n"),
                  old_vid,
                  unit));
        return BCM_E_PARAM;
    }
    if ((_sbx_vlan_max_vid_value < new_vid) ) {
        VLAN_ERR((VLAN_FUNCMSG1("new_vid %04X is out of range on unit %d\n"),
                  new_vid,
                  unit));
        return BCM_E_PARAM;
    }

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* set the new egress translation */
    result = _bcm_fe2000_vlan_translate_egress_set(unit,
                                                   port,
                                                   old_vid,
                                                   new_vid,
                                                   prio);

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%d,%04X,%04X,%d) return %d (%s)\n"),
               unit,
               port,
               old_vid,
               new_vid,
               prio,
               result,
               _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_translate_egress_delete
 *   Purpose
 *      Remove translation on egress from one VID to another
 *   Parameters
 *      (in) int unit = unit number on which the VID is to be translated
 *      (in) int port = port number on which the VID is to be translated
 *      (in) bcm_vlan_t old_vid = original VID
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Translation can only be applied to enabled port+VID.
 *      Translation state is lost when port+VID is disabled.
 */




#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_translate_egress_delete"
int
bcm_fe2000_vlan_translate_egress_delete(int unit,
                                        int port,
                                        bcm_vlan_t old_vid)
{
    int                     result;         /* local result code */

    VLAN_VERB((VLAN_FUNCMSG("(%d,%d,%04X) enter\n"),
               unit,
               port,
               old_vid));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* check for valid parameter values */
    if ((_sbx_vlan_max_vid_value < old_vid) || (0 == old_vid)) {
        VLAN_ERR((VLAN_FUNCMSG1("old_vid %04X is out of range on unit %d\n"),
                  old_vid,
                  unit));
        return BCM_E_PARAM;
    }

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* set the new egress translation */
    result = _bcm_fe2000_vlan_translate_egress_set(unit,
                                                   port,
                                                   old_vid,
                                                   old_vid,
                                                   -1);

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%d,%04X) return %d (%s)\n"),
               unit,
               port,
               old_vid,
               result,
               _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_translate_egress_delete_all
 *   Purpose
 *      Delete all translation on egress from one VID to another
 *   Parameters
 *      (in) int unit = unit number where egress VID translation is to purged
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The only fatal error here is semtake/semgive failure.  Other errors
 *      will be ignored while we clear the translations, and the last one to
 *      occur will he reported to the caller.
 *      Takes and releases the unit semaphore per port so other tasks can have
 *      a chance to operate.
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_translate_egress_delete"
int
bcm_fe2000_vlan_translate_egress_delete_all(int unit)
{
    int                   rv;               /* called result code */
    int                   result;           /* local result code */
    unsigned int          vIndex;           /* working VID index */
    bcm_port_t            pIndex;           /* working port index */

    VLAN_VERB((VLAN_FUNCMSG("(%d) enter\n"), unit));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* Optimistically assume success from here */
    result = BCM_E_NONE;

    PBMP_ALL_ITER(unit,pIndex) {
        /* Claim the lock for VID work on this unit */
        if (sal_mutex_take(_vlan_state[unit]->lock, sal_mutex_FOREVER)) {
            /* Cound not obtain unit lock  */
            result =  BCM_E_INTERNAL;
            VLAN_ERR((VLAN_FUNCMSG1("unable to obtain VLAN lock on unit %d\n"),
                      unit));
            break;
        }
        /* reset translation on all VIDs for this port */
        for (vIndex = 1; _sbx_vlan_max_vid_value >= vIndex; vIndex++) {
            /* set translation for this port so old_vid == new_vid */
            rv = _bcm_fe2000_vlan_translate_egress_set(unit,
                                                       pIndex,
                                                       vIndex,
                                                       vIndex,
                                                       -1 /* no pri chng */);
            if (BCM_E_NONE != rv) {
                /* an error occurred; keep track of the last one */
                result = rv;
                VLAN_ERR((VLAN_FUNCMSG1("unable to clear egress translation"
                                        "for unit %d port %d VLAN %04X:"
                                        " %d (%s)\n"),
                          unit,
                          pIndex,
                          vIndex,
                          result,
                          _SHR_ERRMSG(result)));
            }
        } /* for (vIndex = 1; _vlan_max_vid_value >= vIndex; vIndex++) */
        /* Release the VID lock for this unit */
        if (sal_mutex_give(_vlan_state[unit]->lock)) {
            /* Could not release unit lock */
            VLAN_ERR((VLAN_FUNCMSG1("unable to release VLAN lock on unit %d\n"),
                      unit));
            result = BCM_E_INTERNAL;
            break;
        }
    } /* PBMP_ALL_ITER(unit,pIndex) */

    VLAN_VERB((VLAN_FUNCMSG("(%d) return %d (%s)\n"),
               unit,
               result,
               _SHR_ERRMSG(result)));

    return result;
}
#undef _VLAN_FUNC_NAME


int
bcm_fe2000_vlan_translate_egress_traverse(int unit,
                      bcm_vlan_translate_egress_traverse_cb cb,
                      void *user_data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_dtag_add(int unit,
                         int port,
                         bcm_vlan_t inner_vid,
                         bcm_vlan_t outer_vid,
                         int prio)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_dtag_delete(int unit,
                            int port,
                            bcm_vlan_t vid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_dtag_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_dtag_range_add(int unit,
                               int port,
                               bcm_vlan_t old_vid_low,
                               bcm_vlan_t old_vid_high,
                               bcm_vlan_t new_vid,
                               int prio)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_dtag_range_delete(int unit,
                                  int port,
                                  bcm_vlan_t old_vid_low,
                                  bcm_vlan_t old_vid_high)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_dtag_range_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_dtag_range_get (int unit, bcm_port_t port,
                                bcm_vlan_t old_vid_low,
                                bcm_vlan_t old_vid_high,
                                bcm_vlan_t *new_vid,
                                int *prio)
{
    return BCM_E_UNAVAIL;
}


/*
 * Vlan selection based on IPv4 addresses
 */

int
bcm_fe2000_vlan_ip4_add(int unit,
                        bcm_ip_t ipaddr,
                        bcm_ip_t netmask,
                        bcm_vlan_t vid,
                        int prio)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_ip4_delete(int unit,
                           bcm_ip_t ipaddr,
                           bcm_ip_t netmask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_ip4_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

/*
 * Vlan selection based on unified IPv4/IPv6 information structure.
 */

int
bcm_fe2000_vlan_ip_add(int unit,
                       bcm_vlan_ip_t *vlan_ip)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_ip_delete(int unit,
                          bcm_vlan_ip_t *vlan_ip)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_ip_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_fe2000_vlan_control_set
 *   Purpose
 *      Set some other parameter for all VIDs on this unit
 *   Parameters
 *      (in) int unit = unit number whose VIDs are to be changed
 *      (in) bcm_vlan_control_t type = type of control to set
 *      (in) int arg = value for the control being set
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_control_set"
int
bcm_fe2000_vlan_control_set(int unit,
                            bcm_vlan_control_t type,
                            int arg)
{
    int rv = BCM_E_NONE;;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%d,%d) enter\n"),
               unit,
               type,
               arg));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    VLAN_LOCK_TAKE;

    if (type == bcmVlanIndependentStp) {
        VLAN_CTL_SET(unit, bcmVlanIndependentStp, !!arg);
    } else {
        rv = BCM_E_PARAM;
    }

    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%d,%d) return %d (%s)\n"),
               unit, type, arg, rv, _SHR_ERRMSG(rv)));

    return rv;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_control_get
 *   Purpose
 *      Get some other parameter for all VIDs on this unit
 *   Parameters
 *      (in) int unit = unit number whose VIDs are to be queried
 *      (in) bcm_vlan_control_t type = type of control to set
 *      (in) int *arg = pointer to buffer for the value of the control
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */

#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_control_get"
int
bcm_fe2000_vlan_control_get(int unit,
                            bcm_vlan_control_t type,
                            int *arg)
{
    int rv = BCM_E_NONE;
    VLAN_VERB((VLAN_FUNCMSG("(%d,%d,*) enter\n"),
               unit,
               type));

    if (!arg) {
        VLAN_ERR((VLAN_FUNCMSG1("NULL pointer for outbound argument\n")));
        return BCM_E_PARAM;
    }

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

     VLAN_LOCK_TAKE;

     if (type == bcmVlanIndependentStp) {
         *arg = VLAN_CTL_GET(unit, bcmVlanIndependentStp);
     } else {
         rv = BCM_E_PARAM;
     }

    VLAN_LOCK_RELEASE;


    VLAN_VERB((VLAN_FUNCMSG("(%d,%d,&(%d)) return %d (%s)\n"),
               unit, type, *arg, rv, _SHR_ERRMSG(rv)));

    return rv;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_control_port_set
 *   Purpose
 *      Set parameter for all a VID and port combination
 *   Parameters
 *      (in) int unit = unit number whose port is to be set
 *      (in) bcm_port_t port = port to set
 *      (in) bcm_vlan_control_port_t type = what to set
 *      (in) int arg = the new value
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */

#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_control_port_set"
int
bcm_fe2000_vlan_control_port_set(int unit,
                                 bcm_port_t port,
                                 bcm_vlan_control_port_t type,
                                 int arg)
{
    int                   result;           /* local result code */

    VLAN_VERB((VLAN_FUNCMSG("(%d,%d,%d,%d) enter\n"),
               unit,
               port,
               type,
               arg));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* check for valid parameter values */
    if ((!BCM_PBMP_MEMBER(PBMP_ALL(unit),port))) {
        VLAN_ERR((VLAN_FUNCMSG1("port %d is not valid on unit %d\n"),
                  port,
                  unit));
        return BCM_E_PARAM;
    }

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* Optimistically assume success from here */
    result = BCM_E_NONE;

    

    /* Handle the request appropriately */
    switch (type) {
    case bcmVlanTranslateIngressEnable:
    case bcmVlanPortPreferIP4:
    case bcmVlanPortPreferMAC:
    case bcmVlanTranslateIngressMissDrop:
    case bcmVlanTranslateEgressEnable:
    case bcmVlanTranslateEgressMissDrop:
    case bcmVlanTranslateEgressMissUntaggedDrop:
    case bcmVlanLookupMACEnable:
    case bcmVlanLookupIPEnable:
    case bcmVlanPortUseInnerPri:
    case bcmVlanPortVerifyOuterTpid:
    case bcmVlanPortOuterTpidSelect:
    case bcmVlanPortTranslateKeyFirst:
    case bcmVlanPortTranslateKeySecond:
    case bcmVlanTranslateEgressMissUntag:
        result = BCM_E_UNAVAIL;
        break;
    default:
        result = BCM_E_PARAM;
    }

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%d,%d,%d) return %d (%s)\n"),
               unit,
               port,
               type,
               arg,
               result,
               _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_control_port_get
 *   Purpose
 *      Get parameter for all a VID and port combination
 *   Parameters
 *      (in) int unit = unit number whose VIDs are to be queried
 *      (in) bcm_port_t port = port from which to get
 *      (in) bcm_vlan_control_port_t type = what to get
 *      (out) int *arg = where to put it
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */

#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_control_port_get"
int
bcm_fe2000_vlan_control_port_get(int unit,
                                 bcm_port_t port,
                                 bcm_vlan_control_port_t type,
                                 int *arg)
{
    int                   result;           /* local result code */

    VLAN_VERB((VLAN_FUNCMSG("(%d,%d,%d,*) enter\n"),
               unit,
               port,
               type));

    if (!arg) {
        VLAN_ERR((VLAN_FUNCMSG1("NULL pointer for outbound argument\n")));
        return BCM_E_PARAM;
    }

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* check for valid parameter values */
    if (!BCM_PBMP_MEMBER(PBMP_ALL(unit),port)) {
        VLAN_ERR((VLAN_FUNCMSG1("port %d is not valid on unit %d\n"),
                  port,
                  unit));
        return BCM_E_PARAM;
    }

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* Optimistically assume success from here */
    result = BCM_E_NONE;

    

    /* Handle the request appropriately */
    switch (type) {
    case bcmVlanTranslateIngressEnable:
    case bcmVlanPortPreferIP4:
    case bcmVlanPortPreferMAC:
    case bcmVlanTranslateIngressMissDrop:
    case bcmVlanTranslateEgressEnable:
    case bcmVlanTranslateEgressMissDrop:
    case bcmVlanTranslateEgressMissUntaggedDrop:
    case bcmVlanLookupMACEnable:
    case bcmVlanLookupIPEnable:
    case bcmVlanPortUseInnerPri:
    case bcmVlanPortVerifyOuterTpid:
    case bcmVlanPortOuterTpidSelect:
    case bcmVlanPortTranslateKeyFirst:
    case bcmVlanPortTranslateKeySecond:
    case bcmVlanTranslateEgressMissUntag:
        result = BCM_E_UNAVAIL;
        break;
    default:
        result = BCM_E_PARAM;
    }

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%d,%d,%d) return %d (%s)\n"),
               unit,
               port,
               type,
               *arg,
               result,
               _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_mcast_flood_set
 *   Purpose
 *      Set the multicast flooding mode for the VLAN.
 *   Parameters
 *      (in) int unit = unit number whose VID flooding mode is to be set
 *      (in) bcm_vlan_t vlan = VID whose flooding mode is to be set
 *      (in) bcm_vlan_mcast_flood_t mode = new flooding mode
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */

#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_mcast_flood_set"
int
bcm_fe2000_vlan_mcast_flood_set(int unit,
                                bcm_vlan_t vlan,
                                bcm_vlan_mcast_flood_t mode)
{
    _bcm_sbx_vlan_ft_t     ft;               /* working FT (flooding) entry */
    int                    valid;            /* does VLAN exist */
    int                    result;           /* local result code */

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,%d) enter\n"),
               unit,
               vlan,
               mode));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* check for valid parameter values */
    if ((_sbx_vlan_max_vid_value < vlan) || (0 == vlan)) {
        VLAN_ERR((VLAN_FUNCMSG1("invalid VID %04X on unit %d\n"),
                  vlan,
                  unit));
        return BCM_E_PARAM;
    }
    if (BCM_VLAN_MCAST_FLOOD_UNKNOWN != mode) {
        VLAN_ERR((VLAN_FUNCMSG1("unsupported mcast flood mode %d on unit %d"
                                " VLAN %04X\n"),
                  mode,
                  unit,
                  vlan));
        return BCM_E_PARAM;
    }

    /* Optimistically assume success from here */
    result = BCM_E_NONE;

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* Update the VID if it exists */
    result = _bcm_fe2000_vlan_check(unit, vlan, &valid, &ft);
    if (BCM_E_NONE == result) {
        /* read of VID info successful */
        if (valid) {
            /* Entry is valid; get then update the state of the VID */
            
        } else { /* if (valid) */
            /* VID is not created yet */
            VLAN_ERR((VLAN_FUNCMSG1("VLAN %04X does not exist on unit %d\n"),
                      vlan,
                      unit));
            result = BCM_E_NOT_FOUND;
        } /* if (valid) */
    } else {/* if (BCM_E_NONE == result) */
        /* Something went wrong; translate to proper result code */
        VLAN_ERR((VLAN_FUNCMSG1("unable to check unit %d VLAN %04X:"
                                " %d (%s)\n"),
                  unit,
                  vlan,
                  result,
                  _SHR_ERRMSG(result)));
    } /* if (BCM_E_NONE == result) */

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,%d) return %d (%s)\n"),
               unit,
               vlan,
               mode,
               result,
               _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_mcast_flood_get
 *   Purpose
 *      Get the multicast flooding mode for the VLAN.
 *   Parameters
 *      (in) int unit = unit number whose VID flooding mode is to be queried
 *      (in) bcm_vlan_t vlan = VID whose flooding mode is to be queried
 *      (out) bcm_vlan_mcast_flood_t *mode = where to put current flooding mode
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The SBX hardware only appears to support NONE and UNKNOWN flooding
 *      modes; other modes will not be returned.
 */


#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_mcast_flood_get"
int
bcm_fe2000_vlan_mcast_flood_get(int unit,
                                bcm_vlan_t vlan,
                                bcm_vlan_mcast_flood_t *mode)
{
    _bcm_sbx_vlan_ft_t    ft;                /* working FT (flood) entry */
    int                   vlanValid = FALSE; /* whether VID exists */
    int                   result;            /* local result code */

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,*) enter\n"),
               unit,
               vlan));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* check for valid parameter values */
    if ((_sbx_vlan_max_vid_value < vlan) || (0 == vlan)) {
        VLAN_ERR((VLAN_FUNCMSG1("invalid VID %04X for unit %d\n"),
                  vlan,
                  unit));
        return BCM_E_PARAM;
    }

    /* check for valid parameter pointers */
    if (!mode) {
        VLAN_ERR((VLAN_FUNCMSG1("NULL outbound argument\n")));
        return BCM_E_PARAM;
    }

    /* Optimistically assume success from here */
    result = BCM_E_NONE;

    /* this is our default action; assume it if no contradictory evidence */
    *mode = BCM_VLAN_MCAST_FLOOD_UNKNOWN;

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    /* Make sure the VLAN exists */
    result = _bcm_fe2000_vlan_check(unit, vlan, &vlanValid, &ft);
    if (BCM_E_NONE == result) {
        if (vlanValid) {
            /* Entry is valid; get the state of the VID */
            
        } else { /* if (vlanValid) */
            /* Entry not valid; return not found error */
            VLAN_ERR((VLAN_FUNCMSG1("VLAN %04X does not exist on unit %d\n"),
                      vlan,
                      unit));
            result = BCM_E_NOT_FOUND;
        } /* if (vlanValid) */
    } else { /* if (BCM_E_NONE == result) */
        VLAN_ERR((VLAN_FUNCMSG1("unable to read information on unit %d for"
                                " VLAN %04X: %d (%s)\n"),
                  unit,
                  vlan,
                  result,
                  _SHR_ERRMSG(result)));
    } /* if (BCM_E_NONE == result) */

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,(&%d)) return %d (%s)\n"),
               unit,
               vlan,
               *mode,
               result,
               _SHR_ERRMSG(result)));

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_control_vlan_set
 *   Purpose
 *      Set a bunch of VID specific parameters for a VID on this unit
 *   Parameters
 *      (in) int unit = unit number whose VID values are to be read
 *      (in) bcm_vlan_t vid = VID whose values are to be read
 *      (in) bcm_vlan_control_vlan_t control = new values
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */


#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_control_vlan_set"
int
bcm_fe2000_vlan_control_vlan_set(int unit,
                                 bcm_vlan_t vid,
                                 bcm_vlan_control_vlan_t control)
{
    int                      result;              /* local result code */


    VLAN_VERB(("VLAN %d: control vlan set: vid %d\n", unit, vid));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* check for valid parameter values */
    if (/* (_sbx_vlan_max_vid_value < vid) || */ (0 == vid)) {
        return BCM_E_PARAM;
    }
    if ((control.flags & BCM_VLAN_USE_FABRIC_DISTRIBUTION) &&
        (control.distribution_class >= SOC_SBX_CFG(unit)->num_ds_ids)) {
        /* using distribution class, but it's an invalid distribution class */
        return BCM_E_PARAM;
    }

    /* Optimistically assume success from here */
    result = BCM_E_NONE;

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

#ifdef BCM_FE2000_P3_SUPPORT
    if (SOC_IS_SBX_G2P3(unit)) {
        soc_sbx_g2p3_v2e_t v2e;
        uint32 ftidx = 0, unknown_mc_ft_offset = 0;
        soc_sbx_g2p3_ft_t p3ft;

        soc_sbx_g2p3_v2e_t_init(&v2e);
        result =  soc_sbx_g2p3_v2e_get(unit, SBX_VSI_FROM_VID(vid), &v2e);
        if (result != BCM_E_NONE) {
            VLAN_LOCK_RELEASE;
            return result;
        }

        v2e.dontlearn = (0 !=
                         (control.flags & BCM_VLAN_LEARN_DISABLE));
        v2e.igmp = (0 ==
                     (control.flags & BCM_VLAN_IGMP_SNOOP_DISABLE));
        v2e.pim = (0 ==
                    (control.flags & BCM_VLAN_PIM_SNOOP_DISABLE));
        v2e.forceflood = (0 !=
                         (control.flags & BCM_VLAN_L2_LOOKUP_DISABLE));
        v2e.v6mcmode = (0 !=
                     (control.flags & BCM_VLAN_IP6_MCAST_L2_DISABLE));
        v2e.v4mcmode = (0 !=
                     (control.flags & BCM_VLAN_IP4_MCAST_L2_DISABLE));
        v2e.v4route = (0 ==
                     (control.flags & BCM_VLAN_IP4_DISABLE));
        v2e.v6route = (0 ==
                     (control.flags & BCM_VLAN_IP6_DISABLE));
        v2e.laghash = control.trunk_index;
        v2e.dropunksmac = (0 !=
                           (control.flags & BCM_VLAN_UNKNOWN_UCAST_DROP));


        result = soc_sbx_g2p3_v2e_set(unit, SBX_VSI_FROM_VID(vid), &v2e);

        /* update the unknown mc fte for this VID/VSI */
        result = soc_sbx_g2p3_vlan_ft_base_get(unit, &ftidx);
        if (result != BCM_E_NONE) {
            VLAN_LOCK_RELEASE;
            return result;
        }

        ftidx += SBX_VSI_FROM_VID(vid);
        soc_sbx_g2p3_ft_t_init(&p3ft);
        result = soc_sbx_g2p3_ft_get(unit, ftidx, &p3ft);
        if ((result == BCM_E_NONE) &&
            (control.flags & BCM_VLAN_USE_FABRIC_DISTRIBUTION)) {
            /* FTE modifications */
            /* using fabric distribution option */
            p3ft.qid = SBX_MC_QID_BASE +
                            (control.distribution_class * SBX_MAX_COS);
            result = soc_sbx_g2p3_ft_set(unit, ftidx, &p3ft);
        }

        if (result == BCM_E_NONE) {
            result = soc_sbx_g2p3_mc_ft_offset_get(unit, &unknown_mc_ft_offset);
            if (result != BCM_E_NONE) {
                VLAN_LOCK_RELEASE;
                return result;
            }

            ftidx += unknown_mc_ft_offset;

            soc_sbx_g2p3_ft_t_init(&p3ft);
            result = soc_sbx_g2p3_ft_get(unit, ftidx, &p3ft);
            if (result == BCM_E_NONE) {
                if (control.unknown_multicast_group == BCM_MULTICAST_INVALID) {
                    p3ft.oi = SBX_VSI_FROM_VID(vid);
                } else {
                    p3ft.oi = control.unknown_multicast_group;
                }
                p3ft.mc = 1;

                /* FTE modifications */
                if (control.flags & BCM_VLAN_USE_FABRIC_DISTRIBUTION) {
                    /* using fabric distribution option */
                    p3ft.qid = SBX_MC_QID_BASE +
                            (control.distribution_class * SBX_MAX_COS);
                }
                result = soc_sbx_g2p3_ft_set(unit, ftidx, &p3ft);
            }
        }

        VLAN_LOCK_RELEASE;

        return result;
    }
#endif /* BCM_FE2000_P3_SUPPORT */


    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_control_vlan_get
 *   Purpose
 *      Get a bunch of VID specific parameters for a VID on this unit
 *   Parameters
 *      (in) int unit = unit number whose VID values are to be read
 *      (in) bcm_vlan_t vid = VID whose values are to be read
 *      (out) bcm_vlan_control_vlan_t *control = ptr to buffer for current vals
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */

#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_control_vlan_get"
int
bcm_fe2000_vlan_control_vlan_get(int unit,
                                 bcm_vlan_t vid,
                                 bcm_vlan_control_vlan_t *control)
{
    int                      result;              /* local result code */

    VLAN_VERB(("VLAN %d: control vlan get: vid %d\n", unit, vid));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* check for valid parameter values */
    if (/* (_sbx_vlan_max_vid_value < vid) || */ (0 == vid) || (!control)) {
        return BCM_E_PARAM;
    }

    /* clear the return buffer */
    bcm_vlan_control_vlan_t_init(control);

    /* Optimistically assume success from here */
    result = BCM_E_NONE;

    /* Claim the lock for VID work on this unit */
    VLAN_LOCK_TAKE;

    if (SOC_IS_SBX_G2P3(unit)) {
#ifdef BCM_FE2000_P3_SUPPORT
        soc_sbx_g2p3_ft_t  fte;
        soc_sbx_g2p3_v2e_t v2e;
        uint32 ftidx, unknown_mc_ft_offset = 0;

        /* make sure the VID exists */
        result = soc_sbx_g2p3_vlan_ft_base_get(unit, &ftidx);

        if (result == BCM_E_NONE) {
            ftidx += SBX_VSI_FROM_VID(vid);

            soc_sbx_g2p3_ft_t_init(&fte);
            result = soc_sbx_g2p3_ft_get(unit, ftidx, &fte);
            if (result == BCM_E_NONE) {
                soc_sbx_g2p3_v2e_t_init(&v2e);
                result = soc_sbx_g2p3_v2e_get(unit,
                                              SBX_VSI_FROM_VID(vid),
                                              &v2e);
                if (result == BCM_E_NONE) {
                    control->forwarding_vlan = vid;
                    control->l2_mcast_flood_mode = BCM_VLAN_MCAST_FLOOD_UNKNOWN;
                    control->trunk_index = v2e.laghash;
                    if (v2e.dontlearn) {
                        control->flags |= BCM_VLAN_LEARN_DISABLE;
                    }
                    if (!v2e.igmp) {
                        control->flags |= BCM_VLAN_IGMP_SNOOP_DISABLE;
                    }
                    if (!v2e.pim) {
                        control->flags |= BCM_VLAN_PIM_SNOOP_DISABLE;
                    }
                    if (v2e.forceflood) {
                        control->flags |= BCM_VLAN_L2_LOOKUP_DISABLE;
                    }
                    if (!v2e.forceflood) {
                        control->ip4_mcast_flood_mode = BCM_VLAN_MCAST_FLOOD_NONE;
                        control->ip6_mcast_flood_mode = BCM_VLAN_MCAST_FLOOD_NONE;
                    } else {
                        control->ip4_mcast_flood_mode
                                 = BCM_VLAN_MCAST_FLOOD_UNKNOWN;
                        control->ip6_mcast_flood_mode
                                 = BCM_VLAN_MCAST_FLOOD_UNKNOWN;
                    }
                }

                /* retrieve the unknown mc fte for this VID/VSI */
                result = soc_sbx_g2p3_mc_ft_offset_get(unit, &unknown_mc_ft_offset);
                if (result == BCM_E_NONE) {
                    ftidx += unknown_mc_ft_offset;

                    soc_sbx_g2p3_ft_t_init(&fte);
                    result = soc_sbx_g2p3_ft_get(unit, ftidx, &fte);
                    if (result == BCM_E_NONE) {
                        if (fte.oi == 0) {
                            control->unknown_multicast_group =
                                          SBX_VSI_FROM_VID(vid);
                        } else {
                            control->unknown_multicast_group = fte.oi;
                        }

                        if (fte.qid != SBX_MC_QID_BASE) {
                             control->distribution_class =
                              (fte.qid - SBX_MC_QID_BASE) / SBX_MAX_COS;
                        }
                    }
                }
            }
        }
#endif /* BCM_FE2000_P3_SUPPORT */
    }

    /* Release the VID lock for this unit */
    VLAN_LOCK_RELEASE;

    /* Report the result to the caller */
    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_vector_flags_set
 *   Purpose
 *      Set a one or more VLAN control vlan flags on a vlan_vector on this unit
 *   Parameters
 *      (in) int unit = unit number
 *      (in) bcm_vlan_vector_t vlan_vector = Vlan vector for values to be set
 *      (in) uint32 flags_mask
 *      (in) uint32 flags_value
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_vector_flags_set"
int
bcm_fe2000_vlan_vector_flags_set(int unit,
                              bcm_vlan_vector_t vlan_vector,
                              uint32 flags_mask,
                              uint32 flags_value)
{
    int rv;

    bcm_vlan_t              vid;
    bcm_vlan_control_vlan_t control;
    uint32_t                *fastFlood;
    int                     *fastFloodSets;

    VLAN_VERB(("VLAN %d: vector flags set: flags %x\n", unit, flags_mask));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* non-supported vector flags */
    if (flags_mask & (BCM_VLAN_USE_FABRIC_DISTRIBUTION | BCM_VLAN_COSQ_ENABLE)) {
        return BCM_E_PARAM;
    }

    /* Optimistically assume success from here */
    rv = BCM_E_NONE;

#ifdef BCM_FE2000_P3_SUPPORT
    /* fast vector flag support */ 
    if (flags_mask == BCM_VLAN_L2_LOOKUP_DISABLE) { 

        fastFlood = sal_alloc(sizeof(uint32_t) * (BCM_VLAN_MAX + 1),
                              "fastFlood");
        fastFloodSets = sal_alloc(sizeof(uint32_t) * (BCM_VLAN_MAX + 1),
                                  "fastFloodSets");

        if (fastFlood == NULL || fastFloodSets == NULL)
        {
            VLAN_ERR((VLAN_FUNCMSG1("unable to alloc %d bytes for"
                                    " unit %d vlan temp data\n"),
                      sizeof(uint32_t) * (BCM_VLAN_MAX + 1),unit));
            if (fastFlood) {
                sal_free(fastFlood);
            }
            if (fastFloodSets) {
                sal_free(fastFloodSets);
            }
            return BCM_E_MEMORY;
        } else {
            sal_memset(fastFlood, 0x00, sizeof(uint32_t) * (BCM_VLAN_MAX + 1));
            sal_memset(fastFloodSets, 0x00, sizeof(uint32_t) * (BCM_VLAN_MAX + 1));
        }

#if 0
        /* There is not a fast get.  Assume vlans not is vector have forceflood=0 */
        rv = soc_sbx_g2p3_v2e_forceflood_fast_get(unit, 
                                                  0, 
                                                  0xFFF, 
                                                  fastFlood, 
                                                  BCM_VLAN_MAX + 1); 
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("failed to get v2e[*] forceflood: %d %s\n"),
                      rv, bcm_errmsg(rv)));
        }
#endif

        for (vid = BCM_VLAN_MIN + 1; vid < BCM_VLAN_MAX; vid++) {

           if (BCM_VLAN_VEC_GET(vlan_vector, vid)) {
              fastFloodSets[vid] = 1;
              fastFlood[vid] = !!flags_value;
           }
        }

        if (BCM_SUCCESS(rv)) {
            rv = soc_sbx_g2p3_v2e_forceflood_fast_set(unit, 
                                                      0, 
                                                      0xFFF, 
                                                      fastFloodSets,
                                                      fastFlood,
                                                      BCM_VLAN_MAX + 1);
            if (BCM_FAILURE(rv)) {
                VLAN_ERR((VLAN_FUNCMSG1("failed to set v2e[*] forceflood: %d %s\n"),
                          rv, bcm_errmsg(rv)));
            }
        }

        sal_free(fastFlood);
        sal_free(fastFloodSets);

        return (rv);
    }
#endif

    for (vid = BCM_VLAN_MIN + 1; vid < BCM_VLAN_MAX; vid++) {

       if (BCM_VLAN_VEC_GET(vlan_vector, vid)) {

          if (BCM_SUCCESS(rv)) {
              rv = bcm_fe2000_vlan_control_vlan_get(unit, vid, &control);
              if (BCM_FAILURE(rv)) {
                  VLAN_ERR((VLAN_FUNCMSG1("failed to get vlan_control_vlan[%d]: %d %s\n"),
                            vid, rv, bcm_errmsg(rv)));
              }
          }

          control.flags = (~flags_mask & control.flags) | (flags_mask & flags_value);

          if (BCM_SUCCESS(rv)) {
              rv = bcm_fe2000_vlan_control_vlan_set(unit, vid, control);
              if (BCM_FAILURE(rv)) {
                  VLAN_ERR((VLAN_FUNCMSG1("failed to set vlan_control_vlan[%d]: %d %s\n"),
                            vid, rv, bcm_errmsg(rv)));
              }
          }
       }
    }

    return (rv);
}
#undef _VLAN_FUNC_NAME

int
bcm_fe2000_vlan_cross_connect_add(int unit,
                                  bcm_vlan_t outer_vlan,
                                  bcm_vlan_t inner_vlan,
                                  bcm_gport_t port_1,
                                  bcm_gport_t port_2)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_cross_connect_delete(int unit,
                                     bcm_vlan_t outer_vlan,
                                     bcm_vlan_t inner_vlan)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_cross_connect_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_cross_connect_traverse(int unit,
                                       bcm_vlan_cross_connect_traverse_cb cb,
                                       void *user_data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_mac_action_add(int unit,
                               bcm_mac_t mac,
                               bcm_vlan_action_set_t *action)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_mac_action_get(int unit, bcm_mac_t mac, bcm_vlan_action_set_t *action)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_mac_action_delete(int unit, bcm_mac_t mac)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_mac_action_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_mac_action_traverse(int unit,
                                 bcm_vlan_mac_action_traverse_cb cb,
                                 void *user_data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_translate_action_add(int unit,
                                     bcm_port_t port,
                                     bcm_vlan_translate_key_t key_type,
                                     bcm_vlan_t outer_vlan,
                                     bcm_vlan_t inner_vlan,
                                     bcm_vlan_action_set_t *action)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_translate_action_get(int unit,
                                     bcm_gport_t port,
                                     bcm_vlan_translate_key_t key_type,
                                     bcm_vlan_t outer_vlan,
                                     bcm_vlan_t inner_vlan,
                                     bcm_vlan_action_set_t *action)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_translate_action_delete(int unit,
                                        bcm_port_t port,
                                        bcm_vlan_translate_key_t key_type,
                                        bcm_vlan_t outer_vlan,
                                        bcm_vlan_t inner_vlan)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_translate_action_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_translate_action_range_get(int unit,
                                           bcm_gport_t port,
                                           bcm_vlan_translate_key_t key_type,
                                           bcm_vlan_t outer_vlan,
                                           bcm_vlan_t inner_vlan,
                                           bcm_vlan_action_set_t *action)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_vlan_translate_action_range_traverse(int unit,
                   bcm_vlan_translate_action_range_traverse_cb cb,
                                             void *user_data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_translate_range_traverse(int unit,
                   bcm_vlan_translate_action_range_traverse_cb cb,
                                      void *user_data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_dtag_range_traverse(int unit,
                                bcm_vlan_translate_action_range_traverse_cb cb,
                                 void *user_data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_translate_action_traverse(int unit,
                      bcm_vlan_translate_action_traverse_cb cb,
                      void *user_data)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_vlan_translate_egress_action_add(int unit, int port_class,
                                            bcm_vlan_t outer_vlan,
                                            bcm_vlan_t inner_vlan,
                                            bcm_vlan_action_set_t *action)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_translate_egress_action_get(int unit,
                                            int port_class,
                                            bcm_vlan_t outer_vlan,
                                            bcm_vlan_t inner_vlan,
                                            bcm_vlan_action_set_t *action)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_translate_egress_action_delete(int unit,
                                               int port_class,
                                               bcm_vlan_t outer_vlan,
                                               bcm_vlan_t inner_vlan)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_translate_egress_action_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_translate_egress_action_traverse(int unit,
                         bcm_vlan_translate_egress_action_traverse_cb cb,
                         void *user_data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_vlan_translate_traverse(int unit,
                   bcm_vlan_translate_traverse_cb cb,
                   void *user_data)
{
    return BCM_E_UNAVAIL;
}

/* Create a VLAN queue map entry. */
int
bcm_fe2000_vlan_queue_map_create(int unit,
                                 uint32 flags,
                                 int *qmid)
{
    return BCM_E_UNAVAIL;
}

/* Delete a VLAN queue map entry. */
int
bcm_fe2000_vlan_queue_map_destroy(int unit,
                                  int qmid)
{
    return BCM_E_UNAVAIL;
}

/* Delete all VLAN queue map entries. */
int
bcm_fe2000_vlan_queue_map_destroy_all(int unit)
{
    return BCM_E_UNAVAIL;
}

/* Set a VLAN queue map entry. */
int
bcm_fe2000_vlan_queue_map_set(int unit,
                              int qmid,
                              int pkt_pri,
                              int cfi,
                              int queue,
                              int color)
{
    return BCM_E_UNAVAIL;
}

/* Get a VLAN queue map entry. */
int
bcm_fe2000_vlan_queue_map_get(int unit,
                              int qmid,
                              int pkt_pri,
                              int cfi,
                              int *queue,
                              int *color)
{
    return BCM_E_UNAVAIL;
}

/* Attach a queue map object to a VLAN or VFI. */
int
bcm_fe2000_vlan_queue_map_attach(int unit,
                                 bcm_vlan_t vlan,
                                 int qmid)
{
    return BCM_E_UNAVAIL;
}

/* Get the queue map object which is attached to a VLAN or VFI. */
int
bcm_fe2000_vlan_queue_map_attach_get(int unit,
                                     bcm_vlan_t vlan,
                                     int *qmid)
{
    return BCM_E_UNAVAIL;
}

/* Detach a queue map object from a VLAN or VFI. */
int
bcm_fe2000_vlan_queue_map_detach(int unit,
                                 bcm_vlan_t vlan)
{
    return BCM_E_UNAVAIL;
}

/* Detach queue map objects from all VLAN or VFI. */
int
bcm_fe2000_vlan_queue_map_detach_all(int unit)
{
    return BCM_E_UNAVAIL;
}


/*
 *  Function:
 *   _bcm_fe2000_vlan_port_ingress_stats_enable_update
 *  Purpose:
 *    Enables Egress statistics on given vlan + port
 *    If get is enabled, returns status of whether statistics is enabled
 *  Parameters:
 *    (IN)  unit      - bcm device number
 *    (IN)  vlan      - vlan id
 *    (IN)  port      - port number
 *    (IN)  numCounter - number of counters to allocate - based on policer mode
 *    (IN)  get       - Enquire is statistics is enabled or disabled 
 *    (IN/OUT)
 *  (IN) statsEnabled - (1) return >1 if statistics is enabled  (0) if statistics is disabled
 *  (OUT)             - (1)enable or (0) disable statistics -  frees back counter resource 
 *  Returns:
 *    BCM_E_*
 *  Notes:
 */
/* Assumes Arguments are validate by caller for better performance */
#define _VLAN_FUNC_NAME "_bcm_fe2000_g2p3_vlan_port_ingress_stats_update"
static int _bcm_fe2000_g2p3_vlan_port_ingress_stats_enable_update (int unit, 
                                                                   bcm_vlan_t vlan,
                                                                   bcm_port_t port,
                                                                   int get,
                                                                   int *statsEnabled)
{
    int status = BCM_E_NONE, rv=0;
    int numCounter = 0;

    if(!statsEnabled) {
        status = BCM_E_PARAM;
    } else {
        soc_sbx_g2p3_lp_t   lp;
        soc_sbx_g2p3_pv2e_t pv2e;
        uint32 logicalPort = 0;
        uint32 counterId = 0;
        uint8 alloc_lp = FALSE;
        uint32 refcount = 0;

        status = SOC_SBX_G2P3_PV2E_GET(unit, port, vlan, &pv2e);
        if (BCM_FAILURE(status)) {
            VLAN_ERR((VLAN_FUNCMSG1("failed to get %d:pv2e[%d,0x%03X] : %d %s\n"),
                      unit, port, vlan, status, bcm_errmsg(status)));
        } else {
            logicalPort = pv2e.lpi;
            if(logicalPort == 0) { 
                logicalPort = port; 
            }

            status = soc_sbx_g2p3_lp_get(unit, logicalPort, &lp);
            if(BCM_FAILURE(status)) {
                VLAN_ERR((VLAN_FUNCMSG1("failed to get %d: LP[%d] : %d %s\n"),
                          unit, port, status, bcm_errmsg(status)));
            } else {
                if(get) {
                    /* Valid statistics configuration must have policer enabled and
                     * statistics enabled for counting */
                    *statsEnabled = (lp.counter > 0 && lp.policer) ? 1:0;
                } else {

                    if(*statsEnabled > 0) {
                        /* Verify if policer is associated with the logical port */
                        /* If not policer exists counting cannot be performed */
                        /* Also verify if statistics is enabled on the lp */
                        if(0 == lp.policer || lp.counter > 0) {
                            if(lp.policer) {
                                VLAN_ERR((VLAN_FUNCMSG1("Statistics already enabled on LP[%d]" 
                                                        " on vlan[%d] port[%d]\n"),
                                          logicalPort, vlan, port));                                 
                            } else {
                                VLAN_ERR((VLAN_FUNCMSG1("No Policer Associated to the vlan[%d] port[%d]"
                                                        "to enable counting\n"),
                                          vlan, port));
                            }
                            status = BCM_E_PARAM;
                        } else {
                            /* Determine Number of Counter Required Based on policer id */
                            status = _bcm_fe2000_g2p3_num_policer_counters_get(unit,
                                                                               lp.policer,
                                                                               &numCounter);
                            if(BCM_FAILURE(status)) {
                                VLAN_ERR((VLAN_FUNCMSG1("Failed to Get number of counters associated with"
                                                        "policer[%d] : %d %s\n"),
                                          lp.policer,status, bcm_errmsg(status)));
                            } else {

                                VLAN_VERB((VLAN_FUNCMSG1("Number of counters associated with Policer[%d] : %d"),
                                                         lp.policer, numCounter));                            

                                if(logicalPort == port) {
                                    status = _sbx_gu2_resource_alloc(unit, SBX_GU2K_USR_RES_LPORT,
                                                                     1, &logicalPort, 0);
                                    if (BCM_FAILURE(status)) {
                                        
                                        VLAN_ERR((VLAN_FUNCMSG1("failed to allocate LP: %d %s\n"),
                                                  status, bcm_errmsg(status)));
                                    } else {
                                        VLAN_VERB((VLAN_FUNCMSG1("New LP needed to support statistics:"
                                                                 "allocated lp 0x%x\n"), logicalPort));
                                        alloc_lp = TRUE;
                                    }
                                } 
                        
                                /* Allocate counters, associate to lp */
                                if(BCM_SUCCESS(status)) {
                                    status = _sbx_gu2_resource_alloc_counters(unit, 
                                                                              &counterId,
                                                                              SOC_SBX_G2P3_INGCTR_ID,
                                                                              numCounter);                                     
                                    if(BCM_FAILURE(status)) {
                                        VLAN_ERR((VLAN_FUNCMSG1("Failed to allocate Ingress counters: %d %s\n"),
                                                  status, bcm_errmsg(status)));
                                    } else {
                                        soc_sbx_g2p3_counter_value_t soc_val;
                                        int idx;
                                        /* clear allocated counters */
                                        for (idx=0; ((idx < numCounter) && (rv == BCM_E_NONE)); idx++) {
                                            rv = soc_sbx_g2p3_ingctr_get(unit, TRUE, 
                                                                         counterId + idx, 
                                                                         &soc_val);
                                        }

                                        if(BCM_FAILURE(rv)) {
                                            VLAN_ERR((VLAN_FUNCMSG1("Failed to clear Ingress counters: %d %s\n"),
                                                      rv, bcm_errmsg(rv)));
                                            status = BCM_E_INTERNAL;
                                        } else {

                                            lp.counter = counterId;
                                            status = soc_sbx_g2p3_lp_set(unit, logicalPort, &lp);
                                            if(BCM_FAILURE(status)) {
                                                VLAN_ERR((VLAN_FUNCMSG1("Failed to Set  counter[%d]"
                                                                        " on LP[%]: %d %s\n"),
                                                          counterId, logicalPort, status, bcm_errmsg(status)));
                                            } else {
                                                if (BCM_SUCCESS(status)) {
                                                    /* Allocate lport data */
                                                    status = _sbx_gu2_reference_non_gport_lport_type(unit, 
                                                                                                     logicalPort,
                                                                                                     alloc_lp,
                                                                                                     _LP_OWNER_STATS);
                                                    if (BCM_FAILURE(status)) {
                                                        VLAN_ERR((VLAN_FUNCMSG1("unable to allocate"
                                                                                " logical port data: %d %s\n"),
                                                                  status, bcm_errmsg(status)));
                                                    } else {
                                                        VLAN_VERB((VLAN_FUNCMSG1("Referenced lport[%d]\n"),
                                                                   logicalPort));
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                        
                                /* if any operation failed, free back resources consumed */
                                if(BCM_FAILURE(status)) {
                                    if(counterId) {
                                        /* free back the counters allocated */
                                        rv = _sbx_gu2_resource_free_counters(unit, 
                                                                             counterId,
                                                                             SOC_SBX_G2P3_INGCTR_ID);
                                        if(BCM_FAILURE(rv)) {
                                            VLAN_ERR((VLAN_FUNCMSG1("[%d] failed to free counter id[%d]"
                                                                    " number[%d] : %d %s\n"), 
                                                                    unit, counterId, numCounter, rv, bcm_errmsg(rv)));
                                        }
                                    }
                                    
                                    /* if lp not allocated and just referenced, the last point of failure
                                       is referencing. So it is not required to de-reference it */
                                    if(alloc_lp) {
                                        _sbx_gu2_dereference_non_gport_lport_type(unit,
                                                                                  logicalPort,
                                                                                  &refcount);
                                        
                                        rv = _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_LPORT, 1,
                                                                    &logicalPort, 0);
                                        VLAN_ERR((VLAN_FUNCMSG1("[%d] failed to free LP[%d] : %d %s\n"), 
                                                                unit, logicalPort, rv, bcm_errmsg(rv)));
                                    }
                                }
                            }
                        }
                    } /* *statsEnable > 0 */ 
                    else {
                        counterId = lp.counter;

                        if(logicalPort == port || counterId == 0) {
                            VLAN_ERR((VLAN_FUNCMSG1("Statistics diabled No logical port or counter associated" 
                                                    " with on vlan[%d] port[%d]\n"),
                                      vlan, port)); 
                            status = BCM_E_PARAM;
                        } else {
                            /* Free back counters */
                            rv  = _sbx_gu2_resource_free_counters(unit, 
                                                                  counterId,
                                                                  SOC_SBX_G2P3_INGCTR_ID);
                            if(BCM_FAILURE(rv)) {
                                VLAN_ERR((VLAN_FUNCMSG1("[%d] failed to free counter id[%d] : %d %s\n"), 
                                                        unit, counterId, rv, bcm_errmsg(rv)));
                                status = rv;
                            }
                            /* De-reference + free lp */
                            rv = _sbx_gu2_dereference_non_gport_lport_type(unit,
                                                                               logicalPort,
                                                                               &refcount);
                            if (BCM_FAILURE(rv)) {
                                VLAN_ERR((VLAN_FUNCMSG1("Failed to dereference lp[%d]: %d %s\n"),
                                          logicalPort, rv, bcm_errmsg(rv)));
                                status = rv;
                            } else {
                                /* clear counters on lp */
                                lp.counter = 0;
                                status = soc_sbx_g2p3_lp_set(unit, logicalPort, &lp);
                                if(BCM_FAILURE(status)) {
                                    VLAN_ERR((VLAN_FUNCMSG1("Failed to Clear Counter  on LP[%d]: %d %s\n"),
                                              logicalPort, status, bcm_errmsg(status)));
                                } else {                                
                                    VLAN_ERR((VLAN_FUNCMSG1("Cleared Counter  on LP[%d]\n"),
                                              logicalPort));                                
                                    if(refcount == 0) {
                                        VLAN_VERB((VLAN_FUNCMSG1("LPs  free lp 0x%x\n"),
                                                   logicalPort));
                                        
                                        rv = _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_LPORT, 1,
                                                                    &logicalPort, 0);
                                        if (BCM_FAILURE(rv)) {
                                            VLAN_ERR((VLAN_FUNCMSG1("Failed to Free lp[%d]: %d %s\n"),
                                                      logicalPort, rv, bcm_errmsg(rv)));
                                            status = rv;
                                        }
                                        
                                        pv2e.lpi = 0;
                                        
                                        /* update pv2e if lp index is changed */
                                        rv = SOC_SBX_G2P3_PV2E_SET(unit, port, vlan, &pv2e);
                                        if (BCM_FAILURE(rv)) {
                                            VLAN_ERR((VLAN_FUNCMSG1("Failed to write pv2e[%d,0x%03X]: "
                                                                    "%d %s\n"),
                                                      port, vlan, rv, bcm_errmsg(rv)));
                                            status = rv;
                                        }
                                    }       
                                }   
                            }
                        }
                    }
                }   
            }
        }
    }
    return status;

}

#undef _VLAN_FUNC_NAME
#define _VLAN_FUNC_NAME "_bcm_fe2000_g2p3_vlan_port_egress_stats_update"
/*
 *  Function:
 *   _bcm_fe2000_vlan_port_egress_stats_enable_update
 *  Purpose:
 *    Enables Egress statistics on given vlan + port
 *    If get is enabled, returns status of whether statistics is enabled
 *  Parameters:
 *    (IN)  unit      - bcm device number
 *    (IN)  vlan      - vlan id
 *    (IN)  port      - port number
 *    (IN)  enable    - (1)enable or (0) disable statistics -  frees back counter resource 
 *    (IN)  get       - Enquire is statistics is enabled or disabled 
 *    (IN/OUT)
 *  (IN) statsEnabled - (1) return >1 if statistics is enabled  (0) if statistics is disabled
 *  (OUT)             - (1)enable or (0) disable statistics -  frees back counter resource 
 *  Returns:
 *    BCM_E_*
 *  Notes:
 */
/* Assumes Arguments are validate by caller for better performance */

static int _bcm_fe2000_g2p3_vlan_port_egress_stats_enable_update (int unit, 
                                                                  bcm_vlan_t vlan,
                                                                  bcm_port_t port,
                                                                  int get,
                                                                  int *statsEnabled)
{
    int status = BCM_E_NONE, rv=0;
    uint32 counterId = 0;
    soc_sbx_g2p3_evp2e_t  p3egrVlanPort2Etc;

    if(!statsEnabled) {
        status = BCM_E_PARAM;
    } else {
        status = soc_sbx_g2p3_evp2e_get(unit,
                                        vlan,
                                        port,
                                        &p3egrVlanPort2Etc);
        if(BCM_FAILURE(status)) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:evp2e[%d,%04X]:"
                                    " %d (%s)\n"),
                      unit,
                      port,
                      vlan,
                      status,
                      _SHR_ERRMSG(status)));
        } else {
            if(get) {
                *statsEnabled = (p3egrVlanPort2Etc.counter > 0) ? 1:0;
            } else {
                if(*statsEnabled > 0) {
                    if(p3egrVlanPort2Etc.counter > 0) {
                        VLAN_ERR((VLAN_FUNCMSG1("unit[%d] Counter Already Associated with port[%d] vlan[%d]\n"),
                                  unit, port, vlan));              
                        status = BCM_E_PARAM;
                        
                    } else {
                        /* Allocate Egress counters */
                        status = _sbx_gu2_resource_alloc_counters(unit, &counterId, SOC_SBX_G2P3_EGRCTR_ID, 1);
                        
                        if(BCM_SUCCESS(status)) {
                            soc_sbx_g2p3_counter_value_t soc_val;
                            
                            VLAN_VERB((VLAN_FUNCMSG1("Allocated Egress counters[%d]\n"), counterId));
                            
                            /* clear allocated counters */
                            rv = soc_sbx_g2p3_egrctr_get(unit, TRUE, 
                                                         counterId, 
                                                         &soc_val);
                            
                            if(BCM_FAILURE(rv)) {
                                VLAN_ERR((VLAN_FUNCMSG1("Failed to clear Egress counters: %d %s\n"),
                                          rv, bcm_errmsg(rv)));
                                status = BCM_E_INTERNAL;
                            }
                            
                            p3egrVlanPort2Etc.counter = counterId;
                            status = soc_sbx_g2p3_evp2e_set(unit,
                                                            vlan,
                                                            port,
                                                            &p3egrVlanPort2Etc);            
                            if(BCM_FAILURE(status)) {
                                VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:evp2e[%d,%04X]:"
                                                        " %d (%s)\n"),
                                          unit,
                                          port,
                                          vlan,
                                          status,
                                          _SHR_ERRMSG(status)));
                            }
                            
                        } else {
                            VLAN_ERR((VLAN_FUNCMSG1("Failed to Allocate Egress counters\n")));
                        }

                        if(BCM_FAILURE(status)) {
                            if(BCM_FAILURE(_sbx_gu2_resource_free_counters(unit, counterId, SOC_SBX_G2P3_EGRCTR_ID))) {
                                VLAN_ERR((VLAN_FUNCMSG1("Failed to Free back Egress counters - leak\n"))); 
                            }
                        }
                    }
                } else { /* else enable */
                    if(p3egrVlanPort2Etc.counter == 0) {
                        VLAN_ERR((VLAN_FUNCMSG1("unit[%d] No Counter Associated with port[%d] vlan[%d]\n"),
                                  unit, port, vlan));              
                        status = BCM_E_PARAM;
                        
                    } else {
                        counterId = p3egrVlanPort2Etc.counter;
                        status = _sbx_gu2_resource_free_counters(unit, counterId, SOC_SBX_G2P3_EGRCTR_ID);
                        if(BCM_FAILURE(status)) {
                            VLAN_ERR((VLAN_FUNCMSG1("Failed to Free back Egress counters - leak\n")));
                        } else {
                            VLAN_VERB((VLAN_FUNCMSG1("Freed back Egress counters[%d]\n"), counterId));
                        }

                        p3egrVlanPort2Etc.counter = 0;
                        status = soc_sbx_g2p3_evp2e_set(unit,
                                                        vlan,
                                                        port,
                                                        &p3egrVlanPort2Etc);                 
                        if(BCM_FAILURE(status)) {
                            VLAN_ERR((VLAN_FUNCMSG1("Failed to zero Egress counters on port[%d] vlan[%d]\n"),
                                      port, vlan));
                        } else {
                            VLAN_VERB((VLAN_FUNCMSG1("Zero'd Egress counters on port[%d] vlan[%d]\n"),
                                      port, vlan));
                        }    
                    }
                }
            }
        }
    }

    return status;
}
#undef _VLAN_FUNC_NAME


/*
 *  Function:
 *   _bcm_fe2000_g2p3_logical_interface_ingress_stats_enable_update
 *  Purpose:
 *    Enables Ingress statistics on given Logical interface
 *    If get is enabled, returns status of whether statistics is enabled
 *  Parameters:
 *    (IN)  unit      - bcm device number
 *    (IN)  lp        - logical port
 *    (IN)  get       - Enquire is statistics is enabled or disabled 
 *    (IN/OUT) statsEnabled - (1) return >1 if statistics is enabled  (0) if statistics is disabled
 *  Returns:
 *    BCM_E_*
 *  Notes:
 */
/* Assumes Arguments are validate by caller for better performance */
#define _VLAN_FUNC_NAME "_bcm_fe2000_g2p3_logical_interface_ingress_stats_enable_update"
int 
_bcm_fe2000_g2p3_logical_interface_ingress_stats_enable_update (int unit, 
                                                                uint32 lpi,
                                                                int get,
                                                                int *statsEnabled)
{   
    int status = BCM_E_NONE, rv=0;

    if(!statsEnabled) {
        status = BCM_E_PARAM;
    } else {
        soc_sbx_g2p3_lp_t   lp;

        status = soc_sbx_g2p3_lp_get(unit, lpi, &lp);
        if(BCM_FAILURE(status)) {
            VLAN_ERR((VLAN_FUNCMSG1("failed to get %d: LP[%d] : %d %s\n"),
                      unit, lpi, status, bcm_errmsg(status)));
        } else {        
            if(get) {
                *statsEnabled = (lp.counter > 0 && lp.policer > 0) ? 1:0;
                
                VLAN_ERR((VLAN_FUNCMSG1("Ingress Statistics Enabled[%d]\n"), *statsEnabled));
            } else {
                if( *statsEnabled > 0) {
                    if(lp.counter > 0) {
                        /* Warn */
                        VLAN_ERR((VLAN_FUNCMSG1("unit[%d] Counter Already Associated with LP[%d]\n"),
                                  unit, lpi));                        
                        status= BCM_E_PARAM;
                    } else {
                        /* Allocate counters for ingress */
                        if(lp.policer == 0) {
                            status = BCM_E_PARAM;
                            VLAN_ERR((VLAN_FUNCMSG1("unit[%d] failed to get Policer Associated with LP[%d]\n"),
                                      unit, lpi));
                        } else {
                            uint8 alloc_lp_counter = FALSE;
                            int numCounter = 0;
                            uint32 counterId = 0;

                            /* Determine Number of Counter Required Based on policer id */
                            status = _bcm_fe2000_g2p3_num_policer_counters_get(unit,
                                                                               lp.policer,
                                                                               &numCounter);
                            if(BCM_FAILURE(status)) {
                                VLAN_ERR((VLAN_FUNCMSG1("Failed to Get number of counters associated with"
                                                        "policer[%d] : %d %s\n"),
                                          lp.policer,status, bcm_errmsg(status)));
                            } else {
                                
                                VLAN_VERB((VLAN_FUNCMSG1("Number of counters associated with Policer[%d] : %d\n"),
                                           lp.policer, numCounter));
                                
                                /* Allocate counters for ingress counting */
                                status = _sbx_gu2_resource_alloc_counters(unit, 
                                                                          &counterId,
                                                                          SOC_SBX_G2P3_INGCTR_ID,
                                                                          numCounter);                                     
                                if(BCM_FAILURE(status)) {
                                    VLAN_ERR((VLAN_FUNCMSG1("Failed to allocate Ingress counters: %d %s\n"),
                                              status, bcm_errmsg(status)));
                                } else {
                                    VLAN_VERB((VLAN_FUNCMSG1("Counter ID[%d]: Number[%d] Allocated Successfully\n"),
                                               counterId, numCounter));        
                                    
                                    alloc_lp_counter = TRUE;
                                    lp.counter = counterId;
                                    
                                    status = soc_sbx_g2p3_lp_set(unit, lpi, &lp);
                                    if(BCM_FAILURE(status)) {
                                        VLAN_ERR((VLAN_FUNCMSG1("Failed to Set  counter[%d on LP[%]: %d %s\n"),
                                                  counterId, lpi, status, bcm_errmsg(status)));
                                    } 
                                }   
                            }                     
                            
                            /* Clean up if required */
                            if(BCM_FAILURE(status)) {
                                if(alloc_lp_counter) {
                                    /* free back the counters allocated */
                                    rv = _sbx_gu2_resource_free_counters(unit, 
                                                                         counterId,
                                                                         SOC_SBX_G2P3_INGCTR_ID);
                                    if(BCM_FAILURE(rv)) {
                                        VLAN_ERR((VLAN_FUNCMSG1("[%d] failed to free counter id[%d]"
                                                                " number[%d] : %d %s\n"), 
                                                  unit, counterId, numCounter, rv, bcm_errmsg(rv)));
                                    }

                                    lp.counter = 0;
                                    rv = soc_sbx_g2p3_lp_set(unit, lpi, &lp);
                                    if (BCM_FAILURE(rv)) {
                                        VLAN_ERR((VLAN_FUNCMSG1("[%d] failed to write lp[0x%08x]"
                                                                " : %d %s\n"), 
                                                  unit, lpi, rv, bcm_errmsg(rv)));
                                    }

                                }
                            }
                        }
                    }
                } /* if (enable) */ else {
                    if(lp.counter == 0) {
                        VLAN_ERR((VLAN_FUNCMSG1("unit[%d] Counter Not Associated to disable\n"),
                                  unit));                        
                        status= BCM_E_PARAM;
                    } else {   
                        /* Disable Ingress counters & clear it on lp */
                        /* Free back counters */
                        rv  = _sbx_gu2_resource_free_counters(unit, 
                                                              lp.counter,
                                                              SOC_SBX_G2P3_INGCTR_ID); 
                        if(BCM_FAILURE(rv)) {
                            VLAN_ERR((VLAN_FUNCMSG1("[%d] failed to free counter id[%d] : %d %s\n"), 
                                      unit, lp.counter, rv, bcm_errmsg(rv)));
                            status = rv;
                        }    
                        
                        /* clear counters on lp */
                        lp.counter = 0;
                        status = soc_sbx_g2p3_lp_set(unit, lpi, &lp);
                        if(BCM_FAILURE(status)) {
                            VLAN_ERR((VLAN_FUNCMSG1("Failed to Clear Counter  on LP[%d]: %d %s\n"),
                                      lpi, status, bcm_errmsg(status)));
                        }       
                    }           
                } /* disable stats */
            } /* else (get) */
        }            
    } 

    return status;

}
#undef _VLAN_FUNC_NAME



/*
 *  Function:
 *   _bcm_fe2000_g2p3_logical_interface_egress_stats_enable_update
 *  Purpose:
 *    Enables Egress statistics on given Logical interface
 *    If get is enabled, returns status of whether statistics is enabled
 *  Parameters:
 *    (IN)  unit      - bcm device number
 *    (IN)  ohi       - encap id
 *    (IN)  get       - Enquire is statistics is enabled or disabled 
 *    (IN/OUT) statsEnabled - (1) return >1 if statistics is enabled  (0) if statistics is disabled
 *  Returns:
 *    BCM_E_*
 *  Notes:
 */
/* Assumes Arguments are validate by caller for better performance */
#define _VLAN_FUNC_NAME "_bcm_fe2000_g2p3_logical_interface_egress_stats_enable_update"
int _bcm_fe2000_g2p3_logical_interface_egress_stats_enable_update (int unit, 
                                                            uint32 ohi,
                                                            int get,
                                                            int *statsEnabled)
{
    int status = BCM_E_NONE, rv=0;

    if(!statsEnabled) {
        status = BCM_E_PARAM;
    } else {
        soc_sbx_g2p3_oi2e_t oi2e;

        status =  soc_sbx_g2p3_oi2e_get(unit, 
                                        ohi - SBX_RAW_OHI_BASE, 
                                        &oi2e);
        if(BCM_FAILURE(status)) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to read %d: ohi[%d]:"
                                    " %d (%s)\n"),
                      unit,
                      ohi,
                      status,
                      _SHR_ERRMSG(status)));
        } else {        
            if(get) {
                *statsEnabled = (oi2e.counter > 0) ? 1: 0;
                
                VLAN_ERR((VLAN_FUNCMSG1( "Egress Statistics Enabled[%d] \n"),
                          *statsEnabled));
            } else {
                
                if(*statsEnabled) {
                    uint8 alloc_oi_counter = FALSE;
                    uint32 counterId = 0;
                    
                    if(oi2e.counter > 0) {
                        VLAN_ERR((VLAN_FUNCMSG1("unit[%d] Counter Already Associated with OI[%d]\n"),
                                  unit, ohi));              
                            status = BCM_E_PARAM;
                    } else {
                        
                        /* Allocate Egress counters */
                        status = _sbx_gu2_resource_alloc_counters(unit, &counterId,
                                                                  SOC_SBX_G2P3_EGRCTR_ID, 1);
                        
                        if(BCM_SUCCESS(status)) {
                            VLAN_VERB((VLAN_FUNCMSG1("Allocated Egress counters[%d]\n"), counterId));
                                
                            alloc_oi_counter = TRUE;
                            oi2e.counter = counterId;
                                
                            status = soc_sbx_g2p3_oi2e_set(unit, 
                                                           ohi - SBX_RAW_OHI_BASE,
                                                           &oi2e);
                            if(BCM_FAILURE(status)) {
                                VLAN_ERR((VLAN_FUNCMSG1("unable to set %d: ohi[%d]:"
                                                            " %d (%s)\n"),
                                          unit,
                                          ohi,
                                          status,
                                          _SHR_ERRMSG(status)));
                            } else {
                                VLAN_VERB((VLAN_FUNCMSG1("unit[%d] Counter ID[%d]: Set on OHI[%d] \n"),
                                           unit, counterId, ohi)); 
                            }
                            /* All set to count */
                            /* Clean up if required */
                            if(BCM_FAILURE(status)) {
                                if(alloc_oi_counter) {
                                    rv = _sbx_gu2_resource_free_counters(unit, 
                                                                         counterId,
                                                                         SOC_SBX_G2P3_EGRCTR_ID);
                                    if(BCM_FAILURE(rv)) {
                                        VLAN_ERR((VLAN_FUNCMSG1("[%d] Failed to Free back Egress counter[%d]"
                                                                "-leak:%d %s\n"),
                                                  unit, counterId, rv, bcm_errmsg(rv))); 
                                    }

                                    oi2e.counter = 0;
                                    rv = soc_sbx_g2p3_oi2e_set(unit, 
                                                               ohi - SBX_RAW_OHI_BASE,
                                                               &oi2e);
                                    if(BCM_FAILURE(rv)) {
                                        VLAN_ERR((VLAN_FUNCMSG1("[%d] Failed to write oi2e[0x%08x] "
                                                                ":%d %s\n"),
                                                  unit, ohi, rv, bcm_errmsg(rv))); 
                                    }
                                }
                            }
                        } else {
                            VLAN_ERR((VLAN_FUNCMSG1("unit[%d] Failed to Allocate"
                                                    " Egress counters\n"), unit));
                        } 
                    }  
                } /* if (enable) */ else {
                    if(oi2e.counter == 0) {
                        VLAN_ERR((VLAN_FUNCMSG1("unit[%d] Counter Not Associated with OI[%d]\n"),
                                  unit, ohi));              
                        status = BCM_E_PARAM;
                    } else {
                        /* Disable Egress counters & clear it on oi*/
                        status = _sbx_gu2_resource_free_counters(unit, oi2e.counter, SOC_SBX_G2P3_EGRCTR_ID);
                        if(BCM_FAILURE(status)) {
                            VLAN_ERR((VLAN_FUNCMSG1("Failed to Free back Egress counters - leak\n")));
                        } else {
                            VLAN_VERB((VLAN_FUNCMSG1("Freed back Egress counters[%d]\n"), oi2e.counter));
                        }
                        
                        oi2e.counter = 0;
                        status = soc_sbx_g2p3_oi2e_set(unit,
                                                       ohi - SBX_RAW_OHI_BASE,
                                                       &oi2e);
                        if(BCM_FAILURE(status)) {
                            VLAN_ERR((VLAN_FUNCMSG1("Failed to zero Egress counters on ohi[%d]\n"),
                                      ohi));
                        } else {
                            VLAN_VERB((VLAN_FUNCMSG1("Zero'd Egress counters on ohi[%d]\n"),
                                       ohi));
                        }  
                    }
                }
            }            
        }
    }

    return status;
}
#undef _VLAN_FUNC_NAME

#define BCM_FE2000_VALIDATE_STATS_ARGS(unit,vlan,port)                     \
  do                                                                       \
  {                                                                        \
    if (!SOC_IS_SBX_G2P3(unit)) {                                          \
        VLAN_ERR((VLAN_FUNCMSG1("not supported on this microcode\n")));    \
        return BCM_E_UNAVAIL;                                              \
    }                                                                      \
    if(!BCM_GPORT_IS_SET(port)) {                                          \
    VLAN_UNIT_VALID_CHECK;                                                 \
    VLAN_UNIT_INIT_CHECK;                                                  \
    if (vlan == 0 || vlan > VLAN_MAX_VID_VALUE) {                          \
        VLAN_ERR((VLAN_FUNCMSG1("Bad VLAN %d Argument\n"),vlan));          \
        return BCM_E_PARAM;                                                \
    }                                                                      \
                                                                           \
    if(port < 0 || port > SBX_MAX_PORTS) {                                 \
        VLAN_ERR((VLAN_FUNCMSG1("Bad Port %d Argument\n"),port));          \
        return BCM_E_PARAM;                                                \
    }                                                                      \
    }                                                                      \
  } while(0)              


#define _VLAN_FUNC_NAME "bcm_fe2000_gport_stat_enable_update"
int bcm_fe2000_gport_stat_enable_update(int unit,
                                        bcm_port_t port,
                                        int get,
                                        int *statsEnabled)
{
    int status = BCM_E_NONE;
    int gportType;
    char *gportStr = "<na>";

    if (!statsEnabled) {
        return  BCM_E_PARAM;
    } 
    
    /* GPORT statistics */
    if (BCM_GPORT_IS_SET(port)) {
        gportType = (port >> _SHR_GPORT_TYPE_SHIFT) & _SHR_GPORT_TYPE_MASK;

        switch (gportType) 
        {
#ifdef MIM_SUPPORT
        case _SHR_GPORT_TYPE_MIM_PORT:
            gportStr = "MiM";
            if (get) {
                status = 
                    bcm_fe2000_mim_stat_enable_get(unit, port, statsEnabled);
            } else {
                status = 
                    bcm_fe2000_mim_stat_enable_set(unit, port, *statsEnabled);
            }
            break;
#endif

        case _SHR_GPORT_TYPE_VLAN_PORT:
            gportStr = "VLAN";
            if (get) {
                status = 
                    bcm_fe2000_vgp_stat_enable_get(unit, port, statsEnabled);
            } else {
                status = 
                    bcm_fe2000_vgp_stat_enable_set(unit, port, *statsEnabled);
            }
            break;

        default:
            VLAN_ERR((VLAN_FUNCMSG1("GPORT[%d] Not supported\n"), port));
            return BCM_E_PARAM;
        }
        
        VLAN_ERR((VLAN_FUNCMSG1("%s Statistics for %s GPORT[0x%08x] - %s\n"),
                  ((get) ? "Get" : ((*statsEnabled) ? "Enable" : "Disable")),
                  gportStr, port, bcm_errmsg(status)));
    }

    return status;
}
#undef _VLAN_FUNC_NAME


/*
 *   Function
 *      bcm_fe2000_vlan_port_stat_enable_set
 *   Purpose
 *      Enable Statistics for a Vlan + port flow
 *   Parameters
 *      (in)  int unit   = BCM device number
 *      (in)  port       = physical port
 *      (in)  vlan       = vlan
 *      (in)  enable     = 1[enables] 0[disables]
 *   Returns
 *      BCM_E_*
 *   Note:
 *    Statistics mode cannot be used along with Policer Statistics enabled.
 *    Do not enable policer statistics with statistics decoupled from policers
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_port_stat_enable_set"
int bcm_fe2000_vlan_port_stat_enable_set (int unit,
                                          bcm_vlan_t vlan,
                                          bcm_port_t port,
                                          int enable)
{
    int status = BCM_E_NONE, rv=0;

    /* validate args */
    BCM_FE2000_VALIDATE_STATS_ARGS(unit,vlan,port);

    if (BCM_GPORT_IS_SET(port)) {
        status = bcm_fe2000_gport_stat_enable_update(unit, port,
                                                     0, &enable);
        if(BCM_FAILURE(status)) {
            VLAN_ERR((VLAN_FUNCMSG1("Error %s GPORT Statistics\n"),\
                      enable?"Enabling":"Disabling"));
        } else {
            VLAN_VERB((VLAN_FUNCMSG1("Succefully: %s GPORT Statistics\n"),\
                      enable?"Enabled":"Disabled"));
        }
    } else {
        /* Allocate Ingress counters based on Policer Mode supported */
        /* Associate it to the Logical port */
        status = _bcm_fe2000_g2p3_vlan_port_ingress_stats_enable_update(unit, vlan, 
                                                                        port, 0, &enable);
        if(BCM_FAILURE(status)) {
            VLAN_ERR((VLAN_FUNCMSG1("Error %s Ingress Statistics\n"),\
                      enable?"Enabling":"Disabling"));
        }
    
        if (enable > 0 && BCM_FAILURE(status)) {
            /* skip egress stats allocation */
            /* is disable try to disable egress stats even when ingress failed */
        } else {
            /* Allocate Egress counters */
            rv = _bcm_fe2000_g2p3_vlan_port_egress_stats_enable_update(unit, vlan, 
                                                                       port, 0, &enable);
            if (BCM_FAILURE(rv)) {
                VLAN_ERR((VLAN_FUNCMSG1("Error %s Egress Statistics\n"),\
                          enable?"Enabling":"Disabling"));
                status = rv;
            }
        }
    }
    return status;
}
#undef _VLAN_FUNC_NAME


#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_port_stat_enable_get"
int bcm_fe2000_vlan_port_stat_enable_get (int unit,
                                          bcm_vlan_t vlan,
                                          bcm_port_t port,
                                          int *enable)
{
    int status = BCM_E_NONE;
    int isIngStatsEnabled = 0, isEgrStatsEnabled = 0;

    /* validate args */
    BCM_FE2000_VALIDATE_STATS_ARGS(unit,vlan,port);

    if( BCM_GPORT_IS_SET(port)) {
        status = bcm_fe2000_gport_stat_enable_update(unit, port,
                                                     1, enable);
        if(BCM_FAILURE(status)) {
            VLAN_ERR((VLAN_FUNCMSG1("Failed to get GPORT Statistics status\n")));
        } else {
            VLAN_VERB((VLAN_FUNCMSG1("Succefully: got GPORT Statistics status [%d]\n"),\
                      *enable));
        }
    } else {
        status = _bcm_fe2000_g2p3_vlan_port_ingress_stats_enable_update(unit, 
                                                                        vlan, port, 
                                                                        1, &isIngStatsEnabled);
        if(BCM_FAILURE(status)) {
            VLAN_ERR((VLAN_FUNCMSG1("Error Reading Ingress Statistics status\n")));
        } else {
            VLAN_VERB((VLAN_FUNCMSG1("Ingress Statistics status %s\n"),
                       (isIngStatsEnabled > 0)?"Enabled":"Disabled"));

            status = _bcm_fe2000_g2p3_vlan_port_egress_stats_enable_update(unit,
                                                                           vlan, port,
                                                                           1, &isEgrStatsEnabled);
            if(BCM_FAILURE(status)) {
                VLAN_ERR((VLAN_FUNCMSG1("Error Reading Egress Statistics status\n")));
            } else {
                VLAN_VERB((VLAN_FUNCMSG1("Egress Statistics status %s\n"),
                           (isEgrStatsEnabled > 0)?"Enabled":"Disabled"));

                if(isEgrStatsEnabled != isIngStatsEnabled) {
                    VLAN_ERR((VLAN_FUNCMSG1("Error Ingress Egress Statistics Mismatch\n")));
                    status = BCM_E_INTERNAL;
                } else {
                    *enable = isIngStatsEnabled;
                    VLAN_VERB((VLAN_FUNCMSG1("Vlan[%d] Port[%d] Statistics status %s\n"),
                               (*enable > 0)?"Enabled":"Disabled"));
                }
            }
        }
    }
    return status;
}
#undef _VLAN_FUNC_NAME


/*
 *  Function:
 *   _bcm_fe2000_g2p3_policer_mode_ingress_statistics
 *  Purpose:
 *    Gets/clears the policer mapped stats for the specified vlan stat
 *  Parameters:
 *    (IN)  unit      - bcm device number
 *    (IN)  clear     - TRUE to clear stats; FALSE to read
 *    (IN)  coutnerId - g2p3 counter ID to gather statistics
 *    (IN)  groupMode - policer group mode - used to infer count offsets
 *    (IN)  statCos   - cos of stat, is applicible
 *    (IN)  stat      - stat to get/clear
 *    (OUT) val       - returned stat value
 *  Returns:
 *    BCM_E_*
 *  Notes:
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_g2p3_policer_mode_ingress_statistics"
static int
_bcm_fe2000_g2p3_policer_mode_ingress_statistics(int unit, 
                                                 int get,
                                                 uint32 counterId,
                                                 bcm_policer_group_mode_t groupMode,
                                                 bcm_cos_t statCos, bcm_vlan_stat_t stat,
                                                 uint64 *val)
{
#define BCM_FE2K_G2P3_MAX_STAT_CONV   24
    int                      numStats, statIdx;
    bcm_policer_stat_t       polStats[BCM_FE2K_G2P3_MAX_STAT_CONV];
    int                      rv = BCM_E_NONE;
    uint64                   uuVal;
    int                      allCos = FALSE, ctr_offset=0, pkts = 0;
    soc_sbx_g2p3_counter_value_t    soc_val;

    numStats = 0;

    switch (groupMode) {
    case bcmPolicerGroupModeSingle:

        switch (stat) {
        case bcmVlanStatIngressPackets:
            polStats[numStats++] = bcmPolicerStatPackets;
            polStats[numStats++] = bcmPolicerStatDropPackets;
            break;
        case bcmVlanStatIngressBytes:
            polStats[numStats++] = bcmPolicerStatBytes;
            polStats[numStats++] = bcmPolicerStatDropBytes;
            break;
        case bcmVlanStatForwardedPackets:
            polStats[numStats++] = bcmPolicerStatPackets;
            break;
        case bcmVlanStatForwardedBytes:
            polStats[numStats++] = bcmPolicerStatBytes;
            break;
        case bcmVlanStatDropPackets:
            polStats[numStats++] = bcmPolicerStatDropPackets;
            break;
        case bcmVlanStatDropBytes:
            polStats[numStats++] = bcmPolicerStatDropBytes;
            break;
        default:
            break;
        }
        break;
    case bcmPolicerGroupModeTyped:

        switch (stat) {
        case bcmVlanStatUnicastPackets:
            polStats[numStats++] = bcmPolicerStatUnicastPackets;
            break;
        case bcmVlanStatIngressPackets:
            polStats[numStats++] = bcmPolicerStatUnicastPackets;
            polStats[numStats++] = bcmPolicerStatMulticastPackets;
            polStats[numStats++] = bcmPolicerStatUnknownUnicastPackets;
            polStats[numStats++] = bcmPolicerStatBroadcastPackets;
            polStats[numStats++] = bcmPolicerStatDropUnicastPackets;
            polStats[numStats++] = bcmPolicerStatDropMulticastPackets;
            polStats[numStats++] = bcmPolicerStatDropUnknownUnicastPackets;
            polStats[numStats++] = bcmPolicerStatDropBroadcastPackets;
            break;
        case bcmVlanStatUnicastDropPackets:
            polStats[numStats++] = bcmPolicerStatDropUnicastPackets;
            break;
        case bcmVlanStatUnicastDropBytes:
            polStats[numStats++] = bcmPolicerStatDropUnicastBytes;
            break;
        case bcmVlanStatUnicastBytes:
            polStats[numStats++] = bcmPolicerStatUnicastBytes;
            break;
        case bcmVlanStatIngressBytes:
            polStats[numStats++] = bcmPolicerStatUnicastBytes;
            polStats[numStats++] = bcmPolicerStatMulticastBytes;
            polStats[numStats++] = bcmPolicerStatUnknownUnicastBytes;
            polStats[numStats++] = bcmPolicerStatBroadcastBytes;
            polStats[numStats++] = bcmPolicerStatDropUnicastBytes;
            polStats[numStats++] = bcmPolicerStatDropMulticastBytes;
            polStats[numStats++] = bcmPolicerStatDropUnknownUnicastBytes;
            polStats[numStats++] = bcmPolicerStatDropBroadcastBytes;
            break;
        case bcmVlanStatNonUnicastPackets:
            polStats[numStats++] = bcmPolicerStatMulticastPackets;
            break;
        case bcmVlanStatNonUnicastBytes:
            polStats[numStats++] = bcmPolicerStatMulticastBytes;
            break;
        case bcmVlanStatNonUnicastDropPackets:
            polStats[numStats++] = bcmPolicerStatDropMulticastPackets;
            break;
        case bcmVlanStatNonUnicastDropBytes:
            polStats[numStats++] = bcmPolicerStatDropMulticastBytes;
            break;
        case bcmVlanStatFloodDropPackets:
            polStats[numStats++] = bcmPolicerStatDropUnknownUnicastPackets;
            break;
        case bcmVlanStatFloodDropBytes:
            polStats[numStats++] = bcmPolicerStatDropUnknownUnicastBytes;
            break;
        case bcmVlanStatFloodPackets:
            polStats[numStats++] = bcmPolicerStatUnknownUnicastPackets;
            break;
        case bcmVlanStatFloodBytes:
            polStats[numStats++] = bcmPolicerStatUnknownUnicastBytes;
            break;
        case bcmVlanStatForwardedPackets:
            polStats[numStats++] = bcmPolicerStatUnicastPackets;
            polStats[numStats++] = bcmPolicerStatMulticastPackets;
            polStats[numStats++] = bcmPolicerStatUnknownUnicastPackets;
            polStats[numStats++] = bcmPolicerStatBroadcastPackets;
            break;
        case bcmVlanStatForwardedBytes:
            polStats[numStats++] = bcmPolicerStatUnicastBytes;
            polStats[numStats++] = bcmPolicerStatMulticastBytes;
            polStats[numStats++] = bcmPolicerStatUnknownUnicastBytes;
            polStats[numStats++] = bcmPolicerStatBroadcastBytes;
            break;
        case bcmVlanStatDropPackets:
            polStats[numStats++] = bcmPolicerStatDropUnicastPackets;
            polStats[numStats++] = bcmPolicerStatDropMulticastPackets;
            polStats[numStats++] = bcmPolicerStatDropUnknownUnicastPackets;
            polStats[numStats++] = bcmPolicerStatDropBroadcastPackets;
            break;
        case bcmVlanStatDropBytes:
            polStats[numStats++] = bcmPolicerStatDropUnicastBytes;
            polStats[numStats++] = bcmPolicerStatDropMulticastBytes;
            polStats[numStats++] = bcmPolicerStatDropUnknownUnicastBytes;
            polStats[numStats++] = bcmPolicerStatDropBroadcastBytes;
            break;
        default:
            break;
        }
        break;
    case bcmPolicerGroupModeTypedIntPri: /* fall thru intentional */
    case bcmPolicerGroupModeTypedAll:
        switch (stat) {
        case bcmVlanStatIngressPackets:
            polStats[numStats++] = bcmPolicerStatGreenPackets;
            polStats[numStats++] = bcmPolicerStatYellowPackets;
            polStats[numStats++] = bcmPolicerStatRedPackets;
            allCos = (groupMode == bcmPolicerGroupModeTypedIntPri);
            break;
        case bcmVlanStatIngressBytes:
            polStats[numStats++] = bcmPolicerStatGreenBytes;
            polStats[numStats++] = bcmPolicerStatYellowBytes;
            polStats[numStats++] = bcmPolicerStatRedBytes;
            allCos = (groupMode == bcmPolicerGroupModeTypedIntPri);
            break;
        case bcmVlanStatDropPackets:
            polStats[numStats++] = bcmPolicerStatDropUnknownUnicastPackets;
            polStats[numStats++] = bcmPolicerStatDropUnicastPackets;
            polStats[numStats++] = bcmPolicerStatDropMulticastPackets;
            polStats[numStats++] = bcmPolicerStatDropBroadcastPackets;
            break;
        case bcmVlanStatDropBytes:
            polStats[numStats++] = bcmPolicerStatDropUnknownUnicastBytes;
            polStats[numStats++] = bcmPolicerStatDropUnicastBytes;
            polStats[numStats++] = bcmPolicerStatDropMulticastBytes;
            polStats[numStats++] = bcmPolicerStatDropBroadcastBytes;
            break;
        case bcmVlanStatUnicastDropPackets:
            polStats[numStats++] = bcmPolicerStatDropUnicastPackets;
            break;
        case bcmVlanStatUnicastDropBytes:
            polStats[numStats++] = bcmPolicerStatDropUnicastBytes;
            break;
        case bcmVlanStatNonUnicastDropPackets:
            polStats[numStats++] = bcmPolicerStatDropMulticastPackets;
            polStats[numStats++] = bcmPolicerStatDropBroadcastPackets;
            break;
        case bcmVlanStatNonUnicastDropBytes:
            polStats[numStats++] = bcmPolicerStatDropMulticastBytes;
            polStats[numStats++] = bcmPolicerStatDropBroadcastBytes;
            break;
        case bcmVlanStatFloodDropPackets:
            polStats[numStats++] = bcmPolicerStatDropUnknownUnicastPackets;
            break;
        case bcmVlanStatFloodDropBytes:
            polStats[numStats++] = bcmPolicerStatDropUnknownUnicastBytes;
            break;
        case bcmVlanStatGreenPackets:
            polStats[numStats++] = bcmPolicerStatGreenPackets;
            break;
        case bcmVlanStatGreenBytes:
            polStats[numStats++] = bcmPolicerStatGreenBytes;
            break;
        case bcmVlanStatYellowPackets:
            polStats[numStats++] = bcmPolicerStatYellowPackets;
            break;
        case bcmVlanStatYellowBytes:
            polStats[numStats++] = bcmPolicerStatYellowBytes;
            break;
        case bcmVlanStatRedPackets:
            polStats[numStats++] = bcmPolicerStatRedPackets;
            break;
        case bcmVlanStatRedBytes:
            polStats[numStats++] = bcmPolicerStatRedBytes;
            break;
        default:
            break;
        }
        break;

    default:
        VLAN_ERR((VLAN_FUNCMSG1("Unsupported groupMode: %d %s\n"),
                  groupMode,
                  _bcm_fe2000_policer_group_mode_to_str(groupMode)));
        return BCM_E_CONFIG;
        break;
    }

    assert (numStats < BCM_FE2K_G2P3_MAX_STAT_CONV);

    if (numStats <= 0 ) {
        VLAN_ERR((VLAN_FUNCMSG1("VlanStatType %d not supported by "
                                "policer group mode %d\n"),
                  stat, groupMode));
        return BCM_E_PARAM;
    }


    COMPILER_64_ZERO(uuVal);
    for (statIdx = 0; statIdx < numStats; statIdx++) {
        uint64 uuTmp = 0;
        int cos, cosStart, cosEnd;


        if (allCos) {
            cosStart = 0;
            cosEnd = NUM_COS(unit);
        } else {
            cosStart = statCos;
            cosEnd = statCos + 1;
        }

        for (cos = cosStart; cos < cosEnd; cos++) {
            pkts = 0;
            rv = _bcm_fe2000_g2p3_policer_stat_mem_get(unit, groupMode,
                                                       polStats[statIdx], cos, 
                                                       &ctr_offset,
                                                       &pkts); 
            if (BCM_FAILURE(rv)) {
                VLAN_ERR((VLAN_FUNCMSG1("Failed to get vlanStat=%d; "
                                        ": %d %s\n"),
                          stat, rv, bcm_errmsg(rv)));
                break;
            }

            rv = soc_sbx_g2p3_ingctr_get(unit, 
                                         (get) ? 0:1, 
                                         (counterId + ctr_offset),
                                         &soc_val);
            if (BCM_FAILURE(rv)) {
                VLAN_ERR((VLAN_FUNCMSG1("Failed to get vlanStat=%d; "
                                        ": %d %s\n"),
                          stat, rv, bcm_errmsg(rv)));
                return rv;
            }

            if (pkts) {
                COMPILER_64_SET(uuTmp, COMPILER_64_HI(soc_val.packets),
                                COMPILER_64_LO(soc_val.packets));
            } else {
                COMPILER_64_SET(uuTmp, COMPILER_64_HI(soc_val.bytes),
                                COMPILER_64_LO(soc_val.bytes));
            }
            COMPILER_64_ADD_64(uuVal, uuTmp);
        }
    }
    
    if (get) {
        *val = uuVal;
    }

    return rv;
}
#undef _VLAN_FUNC_NAME

/*
 *  Function:
 *   _bcm_fe2000_g2p3_ingress_statistics
 *  Purpose:
 *    Read/Write ingress statistc 
 *  Parameters:
 *   (IN)  unit      - bcm device number
 *   (IN)  get      - TRUE to get stats; FALSE to clear or set statistics
 *   (IN)  policerId - PolicerId to infer mode useful to infer counter offsets
 *   (IN)  counterId - Base statistics index 
 *   (IN)  cos       - cos of stat, is applicible
 *   (IN)  type      - stat to get/clear
 *   (IN) val        - value to read to or write from
 *  Returns:
 *    BCM_E_*
 *  Notes:
 *   This function acquires policer group mode. And reads statistics
 *   based on policer mode and counter ID.
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_g2p3_ingress_statistics"
int
_bcm_fe2000_g2p3_ingress_statistics(int unit,
                                    int get,
                                    bcm_policer_t policerId, 
                                    uint32 counterId,
                                    bcm_cos_t cos,
                                    bcm_vlan_stat_t type,
                                    uint64 *val)
{
    int rv = BCM_E_NONE;
    bcm_policer_group_mode_t groupMode;
    uint64                   uuVal = 0;

    if (!val) {
        return BCM_E_PARAM;
    }

    rv = _bcm_fe2000_policer_group_mode_get(unit, policerId, &groupMode);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("Failed to policer group mode "
                                "policer=0x%04x: %d %s\n"),
                  policerId, rv, bcm_errmsg(rv)));
        return rv;
    }
            
    VLAN_VERB((VLAN_FUNCMSG1("policer=0x%04x,groupMode=%d(%s)\n"),
               policerId, groupMode,
               _bcm_fe2000_policer_group_mode_to_str(groupMode)));
    
    rv = _bcm_fe2000_g2p3_policer_mode_ingress_statistics(unit, get, 
                                                          counterId, groupMode, 
                                                          cos, type,
                                                          &uuVal);
            
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("Failed to %s vlan policer stat: %d %s\n"),
                  get ? "get" : "set", rv, bcm_errmsg(rv)));
        return rv;
    } 
    VLAN_VERB((VLAN_FUNCMSG1("count=%lld\n"),uuVal));
    COMPILER_64_ADD_64(*val, uuVal);

    return rv;
}
#undef _VLAN_FUNC_NAME


#define _VLAN_FUNC_NAME "_bcm_fe2000_g2p3_lp_stats_info_get"
static int
_bcm_fe2000_g2p3_lp_stats_info_get(int unit, 
                                   uint32 lpIdx,
                                   uint32 *policerId,
                                   uint32 *counterId)
{
    int                 rv = BCM_E_NONE;
    soc_sbx_g2p3_lp_t   lp;

    rv = soc_sbx_g2p3_lp_get(unit, lpIdx, &lp);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("Failed to get LP 0x%x: %d %s\n"),
                  lpIdx, rv, bcm_errmsg(rv)));
        return rv;
    }

    if (lp.counter == 0 || lp.policer == 0) {
        return BCM_E_NOT_FOUND;
    } 
    
    *policerId = lp.policer;
    *counterId = lp.counter;

    return BCM_E_NONE;
}
#undef _VLAN_FUNC_NAME


/*
 *  Function:
 *   _bcm_fe2000_g2p3_vlan_pv_stats_info_get
 *  Purpose:
 *    Read policer Id and counter Id associated with given port ,vlan 
 *  Parameters:
 *   (IN)  unit      - bcm device number
 *   (IN)  vlan      - vlan id
 *   (IN)  port      - port number
 *   (OUT) counterId - Base statistics index 
 *   (OUT) policerId - policer Id
 *  Returns:
 *    BCM_E_*
 *  Notes:
 *   This function acquires policer group mode. And reads statistics
 *   based on policer mode and counter ID.
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_g2p3_vlan_pv_stats_info_get"
static int
_bcm_fe2000_g2p3_vlan_pv_stats_info_get(int unit, 
                                        bcm_vlan_t vlan,
                                        bcm_port_t port,
                                        uint32 *policerId,
                                        uint32 *counterId)
{
    int                 rv;
    soc_sbx_g2p3_pv2e_t pv2e;
    int                 lpIdx;

    rv = SOC_SBX_G2P3_PV2E_GET(unit, port, vlan, &pv2e);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("failed to get %d:pv2e[%d,0x%03X] : %d %s\n"),
                  unit, port, vlan, rv, bcm_errmsg(rv)));
        return rv;
    }

    if (pv2e.lpi == 0) {
        lpIdx = port;
    } else {
        lpIdx = pv2e.lpi;
    }

    rv = _bcm_fe2000_g2p3_lp_stats_info_get(unit, lpIdx, policerId, counterId);
    return rv;
}
#undef _VLAN_FUNC_NAME


/*
 *  Function:
 *   _bcm_fe2000_g2p3_vlan_port_ingress_stat_rw
 *  Purpose:
 *    Read/Write Ingress statistics for a port on vlan
 *  Parameters:
 *   (IN)  unit      - bcm device number
 *   (IN)  clear     - TRUE to clear stats; FALSE to read
 *   (IN)  vlan      - vid to gather stats
 *   (IN)  port      - port number
 *   (IN)  cos       - cos of stat, is applicible
 *   (IN)  type      - stat to get/clear
 *   (IN/OUT) val        - value to read to or write from
 *  Returns:
 *    BCM_E_*
 *  Notes:
 */

#define _VLAN_FUNC_NAME "_bcm_fe2000_g2p3_vlan_port_ingress_stat_rw"
static int
_bcm_fe2000_g2p3_vlan_port_ingress_stat_rw(int unit,
                                           int get,
                                           bcm_vlan_t vlan,
                                           bcm_port_t port,
                                           bcm_cos_t cos,
                                           bcm_vlan_stat_t type,
                                           uint64 *val,
                                           uint8 membershipCheck)
{
    int rv = BCM_E_NONE;
    uint32  policerId = 0;
    uint32  counterId = 0;

    if (!val) {
        return BCM_E_PARAM;
    }


    COMPILER_64_ZERO(*val);

    if(vlan <=  BCM_VLAN_MAX) {
        int  member = FALSE;    /* whether port is a member */
        int  untagged = FALSE;  /* whether port is untagged */

        if(membershipCheck) {
            rv = _bcm_fe2000_vlan_port_is_member(unit, vlan, port, &member, &untagged);
        } else {
            member = TRUE;
        }

        if(BCM_SUCCESS(rv)) {
            if(member) {
                rv = _bcm_fe2000_g2p3_vlan_pv_stats_info_get(unit, vlan, port,
                                                             &policerId, &counterId);
                if (BCM_FAILURE(rv)) {
                    if (rv == BCM_E_NOT_FOUND) {
                        VLAN_ERR((VLAN_FUNCMSG1("No policer/counter found on port,vid="
                                                "%d,0x%03x\n"),
                                  port, vlan));
                        rv = BCM_E_PARAM;
                    } else {
                        VLAN_ERR((VLAN_FUNCMSG1("Failed to retrieve policer/counter for"
                                                " port,vid=%d,0x%03x: %d %s\n"),
                                  port, vlan, rv, bcm_errmsg(rv)));
                    }
                } else {
                    rv = _bcm_fe2000_g2p3_ingress_statistics(unit, get, 
                                                             policerId, counterId,
                                                             cos, type, val);
                    if(BCM_FAILURE(rv)) {
                        VLAN_ERR((VLAN_FUNCMSG1("Failed to Get vlan[%d] Port[%d] Statistics !!!\n"),vlan,port));
                    }                        
                }
            } else {
                VLAN_ERR((VLAN_FUNCMSG1("Port(%d) no member of vlan(%d)\n"),
                          port, vlan));
            }
        } else {
            VLAN_ERR((VLAN_FUNCMSG1("Failed to validate port membership: %d %s\n"),
                      rv, bcm_errmsg(rv)));
                
        }

    } else {
        VLAN_ERR((VLAN_FUNCMSG1("Non TB Vsi(%d) not supported\n"),vlan));
    }
        
    VLAN_VERB((VLAN_FUNCMSG1("returned value %lld\n"), *val));
    
    return rv;
}
#undef _VLAN_FUNC_NAME

/*
 *  Function:
 *   _bcm_fe2000_g2p3_egress_stat_rw
 *  Purpose:
 *    Read/Write Ingress statistics 
 *  Parameters:
 *   (IN)  unit      - bcm device number
 *   (IN)  clear     - TRUE to clear stats; FALSE to read
 *   (IN)  counterId - base counter id
 *   (IN)  cos       - cos of stat, is applicible
 *   (IN)  type      - stat to get/clear
 *   (IN/OUT) value  - value to read to or write from
 *  Returns:
 *    BCM_E_*
 *  Notes:
 */
/* Assumes Arguments are validate by caller for better performance */

#define _VLAN_FUNC_NAME "_bcm_fe2000_g2p3_egress_stat_rw"
int
_bcm_fe2000_g2p3_egress_stat_rw (int unit,
                                 uint32 counterId,
                                 bcm_cos_t cos,
                                 bcm_vlan_stat_t stat,
                                 int get, /* 1 - gets statistics, 0 - sets it */
                                 uint64 *value)
{
    int status = BCM_E_PARAM;
    soc_sbx_g2p3_counter_value_t    soc_val;
    uint64  uuVal;
    
    if (counterId == 0 || !value) {
        return BCM_E_PARAM;
    } 
    
    /* Read or Set (clear on read) the stats */
    status = soc_sbx_g2p3_egrctr_get(unit, (get)?0:1,
                                     counterId, &soc_val);
    if(BCM_FAILURE(status)) {
        VLAN_ERR((VLAN_FUNCMSG1("Egress Statistics %s failed %d:%s \n"),
                  get?"GET":"SET",
                  status,
                  _SHR_ERRMSG(status)));
        return status;
    } 


    COMPILER_64_ZERO(uuVal);
    if (get) {
        if (bcmVlanStatEgressPackets == stat) {
            COMPILER_64_SET(uuVal, COMPILER_64_HI(soc_val.packets),
                            COMPILER_64_LO(soc_val.packets));
        } else {
            COMPILER_64_SET(uuVal, COMPILER_64_HI(soc_val.bytes),
                            COMPILER_64_LO(soc_val.bytes));
        }
        VLAN_VERB((VLAN_FUNCMSG1("Egress Stats[%d] = [%lld]\n"), stat, uuVal));
    }


    if (get) {
        *value = uuVal;
    }

    return status;
}
#undef _VLAN_FUNC_NAME

/*
 *  Function:
 *   _bcm_fe2000_g2p3_ohi_egress_stat_rw
 *  Purpose:
 *    Read/Write Ingress statistics for an logical interface 
 &    using encap id or ohi
 *  Parameters:
 *   (IN)  unit      - bcm device number
 *   (IN)  clear     - TRUE to clear stats; FALSE to read
 *   (IN)  ohi       - out header index
 *   (IN)  cos       - cos of stat, is applicible
 *   (IN)  type      - stat to get/clear
 *   (IN/OUT) val    - value to read to or write from
 *  Returns:
 *    BCM_E_*
 *  Notes:
 */
/* Assumes Arguments are validate by caller for better performance */
#define _VLAN_FUNC_NAME "_bcm_fe2000_g2p3_ohi_egress_stat_rw"
int
_bcm_fe2000_g2p3_ohi_egress_stat_rw (int unit,
                                     uint32 ohi,
                                     bcm_cos_t cos,
                                     bcm_vlan_stat_t stat,
                                     int get, /* 1 - gets statistics, 0 - sets it */
                                     uint64 *value)
{
    int status = BCM_E_NONE;
    uint32 counterId = 0;

    if (!value || (!get && *value != 0)) {
        return BCM_E_PARAM;
    }

    switch(stat) {
    case bcmVlanStatEgressPackets:
    case bcmVlanStatEgressBytes:
    {
        /* only supported egress counters */
        soc_sbx_g2p3_oi2e_t oi2e;

        status = soc_sbx_g2p3_oi2e_get(unit,
                                       ohi - SBX_RAW_OHI_BASE,
                                       &oi2e);
        if(BCM_FAILURE(status)) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to read %d: ohi[%d]:"
                                    " %d (%s)\n"),
                      unit,
                      ohi,
                      status,
                      _SHR_ERRMSG(status)));
        } else {
            counterId = oi2e.counter;
            if(counterId == 0) {
                VLAN_ERR((VLAN_FUNCMSG1("Egress Statistics not enabled on ohi[%d]\n"),
                          ohi));
                status = BCM_E_PARAM;
            } else {
                status = _bcm_fe2000_g2p3_egress_stat_rw(unit, counterId,
                                                         cos, stat,
                                                         get, value);
            }
        }
    }
    break;

    default:
        status = BCM_E_PARAM;
        break;
    }

    return status;
}
#undef _VLAN_FUNC_NAME

/*
 *  Function:
 *   _bcm_fe2000_g2p3_vlan_port_egress_stat_rw
 *  Purpose:
 *    Read/Write Ingress statistics for a port on vlan
 *  Parameters:
 *   (IN)  unit      - bcm device number
 *   (IN)  clear     - TRUE to clear stats; FALSE to read
 *   (IN)  vlan      - vid to gather stats
 *   (IN)  port      - port number
 *   (IN)  cos       - cos of stat, is applicible
 *   (IN)  type      - stat to get/clear
 *   (IN/OUT) val    - value to read to or write from
 *  Returns:
 *    BCM_E_*
 *  Notes:
 */
/* Assumes Arguments are validate by caller for better performance */
#define _VLAN_FUNC_NAME "_bcm_fe2000_g2p3_vlan_port_egress_stat_rw"
int
_bcm_fe2000_g2p3_vlan_port_egress_stat_rw (int unit,
                                           bcm_vlan_t vlan, 
                                           int port,
                                           bcm_cos_t cos,
                                           bcm_vlan_stat_t stat,
                                           int get, /* 1 - gets statistics, 0 - sets it */
                                           uint64 *value)
{
   int status = BCM_E_NONE;
    uint32 counterId = 0;

    if (!value) {
        return BCM_E_PARAM;
    }

    switch(stat) {
    case bcmVlanStatEgressPackets:
    case bcmVlanStatEgressBytes:
    {
        /* only supported egress counters */
        soc_sbx_g2p3_evp2e_t  p3egrVlanPort2Etc;
        status = soc_sbx_g2p3_evp2e_get(unit,
                                        vlan,
                                        port,
                                        &p3egrVlanPort2Etc);
        if(BCM_FAILURE(status)) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:evp2e[%d,%04X]:"
                                    " %d (%s)\n"),
                      unit,
                      port,
                      vlan,
                      status,
                      _SHR_ERRMSG(status)));
        } else {
            counterId = p3egrVlanPort2Etc.counter;
            if(counterId == 0) {
                VLAN_ERR((VLAN_FUNCMSG1("Egress Statistics not enabled on vlan[%d] port[%d]\n"),
                          vlan,
                          port));
                status = BCM_E_PARAM;
            } else {
                status = _bcm_fe2000_g2p3_egress_stat_rw(unit, counterId,
                                                         cos, stat,
                                                         get, value);
            }
        }
    }
    break;

    default:
        status = BCM_E_PARAM;
        break;
    }

    return status;
}
#undef _VLAN_FUNC_NAME


/*
 *  Function:
 *   bcm_fe2000_logical_interface_stat_rw
 *  Purpose:
 *    Get Statistics associated with bcm_vlan_stat_t type for a port on vlan
 *  A policer must have been created and associated with port on vlan. Statistics
 *  on ingress are based on policer group mode
 *  Parameters:
 *   (IN)  unit      - bcm device number
 *   (IN)  vlan      - vid to gather stats
 *   (IN)  cos       - cos of stat, is applicible
 *   (IN)  stat      - stat to get/clear
 *   (IN)  val       - value to read to
 *  Returns:
 *    BCM_E_*
 *  Notes:
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_logical_interface_stat_rw"
int
bcm_fe2000_logical_interface_stat_rw(int unit,
                                     uint32 lp,
                                     uint32 ohi,
                                     bcm_cos_t cos,
                                     bcm_vlan_stat_t stat,
                                     int get,
                                     uint64 *val)
{
    int status = BCM_E_NONE;
    uint32 policerId=0, counterId=0;

    if (!val || (get==0 && *val!=0)) {
        return BCM_E_PARAM;
    }

    COMPILER_64_ZERO(*val);

    switch(stat) {
    case bcmVlanStatEgressPackets:
    case bcmVlanStatEgressBytes:

        status = _bcm_fe2000_g2p3_ohi_egress_stat_rw(unit, ohi,
                                                     cos, stat, 
                                                     get, val);
        break;

    default:
        /* all ingress counters */
        status = _bcm_fe2000_g2p3_lp_stats_info_get(unit, lp, 
                                                    &policerId, &counterId);
        if(BCM_FAILURE(status)) {
            VLAN_ERR((VLAN_FUNCMSG1("Failed to read counter/policer from unit %d: LP[%d]:"
                                    " %d (%s)\n"),
                      unit,
                      lp,
                      status,
                      _SHR_ERRMSG(status)));
        } else {
            status = _bcm_fe2000_g2p3_ingress_statistics(unit, get,
                                                         policerId, counterId,
                                                         cos, stat, val);
            if(BCM_FAILURE(status)) {
                VLAN_ERR((VLAN_FUNCMSG1("Failed to read statistics from unit %d:"
                                        "policerid[%d] statsid[%d]  %d (%s)\n"),
                          unit,
                          policerId, counterId,
                          status,
                          _SHR_ERRMSG(status)));
            }
        }
        break;
    }        

    return status;
}
#undef _VLAN_FUNC_NAME

/* Assumes Arguments are validate by caller for better performance */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_port_stats_update"
static int
_bcm_fe2000_vlan_port_stats_update (int unit,
                                    bcm_vlan_t vlan, 
                                    int port,
                                    bcm_cos_t cos,
                                    bcm_vlan_stat_t stat,
                                    int get, /* 1 - gets statistics, 0 - sets it */
                                    uint64 *value,
                                    uint8 membershipCheck)
{
    int    status = BCM_E_NONE;
    int    gportType;
    uint32 lpi=0, ohi=0;
    char *gportStr = "<na>";

    if (!value) {
        return BCM_E_PARAM;
    } 
    
    /* GPORT statistics */
    if (BCM_GPORT_IS_SET(port)) {

        gportType = (port >> _SHR_GPORT_TYPE_SHIFT) & _SHR_GPORT_TYPE_MASK;
        switch (gportType) 
        {
#ifdef MIM_SUPPORT
        case _SHR_GPORT_TYPE_MIM_PORT:
            gportStr = "MiM";
            status = _bcm_fe2000_mim_get_lp_encap(unit, port, &lpi, &ohi);
            break;
#endif
        case _SHR_GPORT_TYPE_VLAN_PORT:
            gportStr = "VLAN";
            status = _bcm_fe2000_vgp_get_lp_encap(unit, port, &lpi, &ohi);
            break;
        default:
            VLAN_ERR((VLAN_FUNCMSG1("GPORT[0x%08x] Not supported\n"),port));
            return BCM_E_PARAM;
        }

        if (BCM_FAILURE(status)) {
            VLAN_ERR((VLAN_FUNCMSG1("Failed to Get LP & Ohi for %s "
                                    "GPORT[0x%08x]: %d %s\n"),
                      gportStr, port, status, bcm_errmsg(status)));
            return status;
        }

        VLAN_VERB((VLAN_FUNCMSG1("LP[0x%x] Ohi[0x%x] for %s GPORT[0x%08x]\n"),
                   lpi, ohi, gportStr, port));

        status = bcm_fe2000_logical_interface_stat_rw(unit, lpi, ohi, 
                                                      cos, stat, get, value);
        if (BCM_FAILURE(status)) {
            VLAN_ERR((VLAN_FUNCMSG1("Failed to Get statistics "
                                    "GPORT[0x%08x]: %d %s\n"),
                      port, status, bcm_errmsg(status)));
        }

        return status;
    } 
    
    /* vlan port statistics */
    switch(stat) {
    case bcmVlanStatEgressPackets:
    case bcmVlanStatEgressBytes:
        status = _bcm_fe2000_g2p3_vlan_port_egress_stat_rw(unit,
                                                           vlan, port,
                                                           cos, stat, 
                                                           get, value);
        break;
        
    default:
        status = _bcm_fe2000_g2p3_vlan_port_ingress_stat_rw(unit, get, 
                                                            vlan, port,
                                                            cos, stat, 
                                                            value, membershipCheck);
        break;
    }

    return status;
}
#undef _VLAN_FUNC_NAME


/*
 *  Function:
 *   bcm_fe2000_vlan_port_stat_get
 *  Purpose:
 *    Get Statistics associated with bcm_vlan_stat_t type for a port on vlan
 *  A policer must have been created and associated with port on vlan. Statistics
 *  on ingress are based on policer group mode
 *  Parameters:
 *   (IN)  unit      - bcm device number
 *   (IN)  vlan      - vid to gather stats
 *   (IN)  cos       - cos of stat, is applicible
 *   (IN)  stat      - stat to get/clear
 *   (IN)  val       - value to read to 
 *  Returns:
 *    BCM_E_*
 *  Notes:
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_port_stat_get"
int
bcm_fe2000_vlan_port_stat_get(int unit,
                              bcm_vlan_t vlan,
                              bcm_port_t port,
                              bcm_cos_t cos,
                              bcm_vlan_stat_t stat,
                              uint64 *val)
{
    int status = BCM_E_NONE;

    /* validate args */
    BCM_FE2000_VALIDATE_STATS_ARGS(unit,vlan,port);

    BCM_STAT_VVERB(("%s(%d, %d, %d, %d, %d, 0x%08X %08X) - Enter\n",
                    FUNCTION_NAME(),
                    unit,
                    vlan,
                    port,
                    cos,
                    stat,
                    u64_H(val),
                    u64_L(val)));

    status = _bcm_fe2000_vlan_port_stats_update(unit, vlan, port, cos, stat, 1, val, TRUE);
    return status;
}
#undef _VLAN_FUNC_NAME


/*
 *  Function:
 *   bcm_fe2000_vlan_port_stat_set
 *  Purpose:
 *    Clear Statistics associated with bcm_vlan_stat_t type for a port on vlan
 *  A policer must have been created and associated with port on vlan. Statistics
 *  on ingress are based on policer group
 *  Parameters:
 *   (IN)  unit      - bcm device number
 *   (IN)  vlan      - vid to gather stats
 *   (IN)  cos       - cos of stat, is applicible
 *   (IN)  stat      - stat to get/clear
 *   (IN)  val       - Only value 0 is supported
 *  Returns:
 *    BCM_E_*
 *  Notes:
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_port_stat_set"
int
bcm_fe2000_vlan_port_stat_set(int unit,
                              bcm_vlan_t vlan,
                              bcm_port_t port,
                              bcm_cos_t cos,
                              bcm_vlan_stat_t stat,
                              uint64 val)
{
    int status = BCM_E_NONE;

    /* validate args */
    BCM_FE2000_VALIDATE_STATS_ARGS(unit,vlan,port);

    STAT_CHECK_STAT_VALUE(val);     /* Only allowed to set to zero */

    BCM_STAT_VVERB(("%s(%d, %d, %d, %d, %d, 0x%08X %08X) - Enter\n",
                    FUNCTION_NAME(),
                    unit,
                    vlan,
                    port,
                    cos,
                    stat,
                    u64_H(val),
                    u64_L(val)));

    status = _bcm_fe2000_vlan_port_stats_update(unit, vlan, port, cos, stat, 0, &val, TRUE);
    return status;
}
#undef _VLAN_FUNC_NAME

/* 
 * Get 32 bit specified statistic to the indicated value for the
 * specified VLAN and port
 *                 or GPORT
 */
int bcm_fe2000_vlan_port_stat_get32(
    int unit, 
    bcm_vlan_t vlan, 
    bcm_port_t port, 
    bcm_cos_t cos, 
    bcm_vlan_stat_t stat, 
    uint32 *val)
{
    return BCM_E_UNAVAIL;
}

/* 
 * Set 32bit specified statistic to the indicated value for the specified
 * VLAN and port
 *                 or GPORT
 */
int bcm_fe2000_vlan_port_stat_set32(
    int unit, 
    bcm_vlan_t vlan, 
    bcm_port_t port, 
    bcm_cos_t cos, 
    bcm_vlan_stat_t stat, 
    uint32 *val)
{
    return BCM_E_UNAVAIL;
}

/*
 *  Function:
 *    _bcm_fe2000_g2p3_vlan_policer_stat_rw
 *  Purpose:
 *    Gets/clears the policer mapped stats for the specified vlan stat
 *  Parameters:
 *    (IN)  unit      - bcm device number
 *    (IN)  clear     - TRUE to clear stats; FALSE to read
 *    (IN)  policerId - g2p3 policer ID to gather stats
 *    (IN)  groupMode - policer group mode
 *    (IN)  statCos   - cos of stat, is applicible
 *    (IN)  stat      - stat to get/clear
 *    (OUT) val       - returned stat value
 *  Returns:
 *    BCM_E_*
 *  Notes:
 */
#ifdef BCM_FE2000_P3_SUPPORT
#define _VLAN_FUNC_NAME "_bcm_fe2000_g2p3_vlan_policer_stat_rw"
static int
_bcm_fe2000_g2p3_vlan_policer_stat_rw(int unit, int clear, uint32 policerId,
                                      bcm_policer_group_mode_t groupMode,
                                      bcm_cos_t statCos, bcm_vlan_stat_t stat,
                                      uint64 *val)
{
#define BCM_FE2K_G2P3_MAX_STAT_CONV   24
    int                      numStats, statIdx;
    bcm_policer_stat_t       polStats[BCM_FE2K_G2P3_MAX_STAT_CONV];
    int                      rv = BCM_E_NONE;
    uint64                   uuVal;
    int                      allCos = FALSE;

    numStats = 0;

    switch (groupMode) {
    case bcmPolicerGroupModeSingle:

        switch (stat) {
        case bcmVlanStatIngressPackets:
            polStats[numStats++] = bcmPolicerStatPackets;
            polStats[numStats++] = bcmPolicerStatDropPackets;
            break;
        case bcmVlanStatIngressBytes:
            polStats[numStats++] = bcmPolicerStatBytes;
            polStats[numStats++] = bcmPolicerStatDropBytes;
            break;
        case bcmVlanStatForwardedPackets:
            polStats[numStats++] = bcmPolicerStatPackets;
            break;
        case bcmVlanStatForwardedBytes:
            polStats[numStats++] = bcmPolicerStatBytes;
            break;
        case bcmVlanStatDropPackets:
            polStats[numStats++] = bcmPolicerStatDropPackets;
            break;
        case bcmVlanStatDropBytes:
            polStats[numStats++] = bcmPolicerStatDropBytes;
            break;
        default:
            break;
        }
        break;
    case bcmPolicerGroupModeTyped:

        switch (stat) {
        case bcmVlanStatUnicastPackets:
            polStats[numStats++] = bcmPolicerStatUnicastPackets;
            break;
        case bcmVlanStatIngressPackets:
            polStats[numStats++] = bcmPolicerStatUnicastPackets;
            polStats[numStats++] = bcmPolicerStatMulticastPackets;
            polStats[numStats++] = bcmPolicerStatUnknownUnicastPackets;
            polStats[numStats++] = bcmPolicerStatBroadcastPackets;
            polStats[numStats++] = bcmPolicerStatDropUnicastPackets;
            polStats[numStats++] = bcmPolicerStatDropMulticastPackets;
            polStats[numStats++] = bcmPolicerStatDropUnknownUnicastPackets;
            polStats[numStats++] = bcmPolicerStatDropBroadcastPackets;
            break;
        case bcmVlanStatUnicastDropPackets:
            polStats[numStats++] = bcmPolicerStatDropUnicastPackets;
            break;
        case bcmVlanStatUnicastDropBytes:
            polStats[numStats++] = bcmPolicerStatDropUnicastBytes;
            break;
        case bcmVlanStatUnicastBytes:
            polStats[numStats++] = bcmPolicerStatUnicastBytes;
            break;
        case bcmVlanStatIngressBytes:
            polStats[numStats++] = bcmPolicerStatUnicastBytes;
            polStats[numStats++] = bcmPolicerStatMulticastBytes;
            polStats[numStats++] = bcmPolicerStatUnknownUnicastBytes;
            polStats[numStats++] = bcmPolicerStatBroadcastBytes;
            polStats[numStats++] = bcmPolicerStatDropUnicastBytes;
            polStats[numStats++] = bcmPolicerStatDropMulticastBytes;
            polStats[numStats++] = bcmPolicerStatDropUnknownUnicastBytes;
            polStats[numStats++] = bcmPolicerStatDropBroadcastBytes;
            break;
        case bcmVlanStatNonUnicastPackets:
            polStats[numStats++] = bcmPolicerStatMulticastPackets;
            break;
        case bcmVlanStatNonUnicastBytes:
            polStats[numStats++] = bcmPolicerStatMulticastBytes;
            break;
        case bcmVlanStatNonUnicastDropPackets:
            polStats[numStats++] = bcmPolicerStatDropMulticastPackets;
            break;
        case bcmVlanStatNonUnicastDropBytes:
            polStats[numStats++] = bcmPolicerStatDropMulticastBytes;
            break;
        case bcmVlanStatFloodDropPackets:
            polStats[numStats++] = bcmPolicerStatDropUnknownUnicastPackets;
            break;
        case bcmVlanStatFloodDropBytes:
            polStats[numStats++] = bcmPolicerStatDropUnknownUnicastBytes;
            break;
        case bcmVlanStatFloodPackets:
            polStats[numStats++] = bcmPolicerStatUnknownUnicastPackets;
            break;
        case bcmVlanStatFloodBytes:
            polStats[numStats++] = bcmPolicerStatUnknownUnicastBytes;
            break;
        case bcmVlanStatForwardedPackets:
            polStats[numStats++] = bcmPolicerStatUnicastPackets;
            polStats[numStats++] = bcmPolicerStatMulticastPackets;
            polStats[numStats++] = bcmPolicerStatUnknownUnicastPackets;
            polStats[numStats++] = bcmPolicerStatBroadcastPackets;
            break;
        case bcmVlanStatForwardedBytes:
            polStats[numStats++] = bcmPolicerStatUnicastBytes;
            polStats[numStats++] = bcmPolicerStatMulticastBytes;
            polStats[numStats++] = bcmPolicerStatUnknownUnicastBytes;
            polStats[numStats++] = bcmPolicerStatBroadcastBytes;
            break;
        case bcmVlanStatDropPackets:
            polStats[numStats++] = bcmPolicerStatDropUnicastPackets;
            polStats[numStats++] = bcmPolicerStatDropMulticastPackets;
            polStats[numStats++] = bcmPolicerStatDropUnknownUnicastPackets;
            polStats[numStats++] = bcmPolicerStatDropBroadcastPackets;
            break;
        case bcmVlanStatDropBytes:
            polStats[numStats++] = bcmPolicerStatDropUnicastBytes;
            polStats[numStats++] = bcmPolicerStatDropMulticastBytes;
            polStats[numStats++] = bcmPolicerStatDropUnknownUnicastBytes;
            polStats[numStats++] = bcmPolicerStatDropBroadcastBytes;
            break;
        default:
            break;
        }
        break;
    case bcmPolicerGroupModeTypedIntPri: /* fall thru intentional */
    case bcmPolicerGroupModeTypedAll:
        switch (stat) {
        case bcmVlanStatIngressPackets:
            polStats[numStats++] = bcmPolicerStatGreenPackets;
            polStats[numStats++] = bcmPolicerStatYellowPackets;
            polStats[numStats++] = bcmPolicerStatRedPackets;
            allCos = (groupMode == bcmPolicerGroupModeTypedIntPri);
            break;
        case bcmVlanStatIngressBytes:
            polStats[numStats++] = bcmPolicerStatGreenBytes;
            polStats[numStats++] = bcmPolicerStatYellowBytes;
            polStats[numStats++] = bcmPolicerStatRedBytes;
            allCos = (groupMode == bcmPolicerGroupModeTypedIntPri);
            break;
        case bcmVlanStatDropPackets:
            polStats[numStats++] = bcmPolicerStatDropUnknownUnicastPackets;
            polStats[numStats++] = bcmPolicerStatDropUnicastPackets;
            polStats[numStats++] = bcmPolicerStatDropMulticastPackets;
            polStats[numStats++] = bcmPolicerStatDropBroadcastPackets;
            break;
        case bcmVlanStatDropBytes:
            polStats[numStats++] = bcmPolicerStatDropUnknownUnicastBytes;
            polStats[numStats++] = bcmPolicerStatDropUnicastBytes;
            polStats[numStats++] = bcmPolicerStatDropMulticastBytes;
            polStats[numStats++] = bcmPolicerStatDropBroadcastBytes;
            break;
        case bcmVlanStatUnicastDropPackets:
            polStats[numStats++] = bcmPolicerStatDropUnicastPackets;
            break;
        case bcmVlanStatUnicastDropBytes:
            polStats[numStats++] = bcmPolicerStatDropUnicastBytes;
            break;
        case bcmVlanStatNonUnicastDropPackets:
            polStats[numStats++] = bcmPolicerStatDropMulticastPackets;
            polStats[numStats++] = bcmPolicerStatDropBroadcastPackets;
            break;
        case bcmVlanStatNonUnicastDropBytes:
            polStats[numStats++] = bcmPolicerStatDropMulticastBytes;
            polStats[numStats++] = bcmPolicerStatDropBroadcastBytes;
            break;
        case bcmVlanStatFloodDropPackets:
            polStats[numStats++] = bcmPolicerStatDropUnknownUnicastPackets;
            break;
        case bcmVlanStatFloodDropBytes:
            polStats[numStats++] = bcmPolicerStatDropUnknownUnicastBytes;
            break;
        case bcmVlanStatGreenPackets:
            polStats[numStats++] = bcmPolicerStatGreenPackets;
            break;
        case bcmVlanStatGreenBytes:
            polStats[numStats++] = bcmPolicerStatGreenBytes;
            break;
        case bcmVlanStatYellowPackets:
            polStats[numStats++] = bcmPolicerStatYellowPackets;
            break;
        case bcmVlanStatYellowBytes:
            polStats[numStats++] = bcmPolicerStatYellowBytes;
            break;
        case bcmVlanStatRedPackets:
            polStats[numStats++] = bcmPolicerStatRedPackets;
            break;
        case bcmVlanStatRedBytes:
            polStats[numStats++] = bcmPolicerStatRedBytes;
            break;
        default:
            break;
        }
        break;

    default:
        VLAN_ERR((VLAN_FUNCMSG1("Unsupported groupMode: %d %s\n"),
                  groupMode,
                  _bcm_fe2000_policer_group_mode_to_str(groupMode)));
        rv = BCM_E_CONFIG;
        break;
    }

    assert (numStats < BCM_FE2K_G2P3_MAX_STAT_CONV);

    if (numStats <= 0 ) {
        VLAN_ERR((VLAN_FUNCMSG1("VlanStatType %d not supported by "
                                "policer group mode %d\n"),
                  stat, groupMode));
        return BCM_E_PARAM;
    }

    COMPILER_64_ZERO(uuVal);
    for (statIdx = 0; statIdx < numStats; statIdx++) {
        uint64 uuTmp = 0;
        int cos, cosStart, cosEnd;

        if (allCos) {
            cosStart = 0;
            cosEnd = NUM_COS(unit);
        } else {
            cosStart = statCos;
            cosEnd = statCos + 1;
        }

        for (cos = cosStart; cos < cosEnd; cos++) {
            if (clear) {
                rv = bcm_fe2000_policer_stat_set(unit, policerId, cos,
                                                 polStats[statIdx], 0);
            } else {
                rv = bcm_fe2000_policer_stat_get(unit, policerId, cos,
                                                 polStats[statIdx], &uuTmp);
            }
            if (BCM_FAILURE(rv)) {
                VLAN_ERR((VLAN_FUNCMSG1("Failed to get vlanStat=%d; "
                                        "policerStat=%d: %d %s\n"),
                          stat, polStats[statIdx], rv, bcm_errmsg(rv)));
                return rv;
            }
            COMPILER_64_ADD_64(uuVal, uuTmp);
        }
    }

    if (!clear) {
        *val = uuVal;
    }

    return rv;
}
#undef _VLAN_FUNC_NAME
#endif /* #ifdef BCM_FE2000_P3_SUPPORT */


/*
 *  Function:
 *    _bcm_fe2000_g2p3_vlan_pv_policer_get
 *  Purpose:
 *    Gets active policer id of specifed port, vid
 *  Parameters:
 *        (IN)  unit      - bcm device number
 *        (IN)  port      - port to access
 *        (IN)  vlan      - vid to access
 *        (OUT) policerId - g2p3 policer ID
 *  Returns:
 *    BCM_E_*
 *  Notes:
 */
#ifdef BCM_FE2000_P3_SUPPORT
#define _VLAN_FUNC_NAME "_bcm_fe2000_g2p3_vlan_pv_policer_get"
static int
_bcm_fe2000_g2p3_vlan_pv_policer_get(int unit, bcm_port_t port,
                                     bcm_vlan_t vlan, uint32 *policerId)
{
    int                 rv;
    soc_sbx_g2p3_lp_t   lp;
    soc_sbx_g2p3_pv2e_t pv2e;
    int                 lpIdx;

    rv = SOC_SBX_G2P3_PV2E_GET(unit, port, vlan, &pv2e);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("failed to get %d:pv2e[%d,0x%03X] : %d %s\n"),
                  unit, port, vlan, rv, bcm_errmsg(rv)));
        return rv;
    }

    if (pv2e.lpi == 0) {
        lpIdx = port;
    } else {
        lpIdx = pv2e.lpi;
    }

    rv = soc_sbx_g2p3_lp_get(unit, lpIdx, &lp);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("Failed to get LP 0x%x: %d %s\n"),
                  lpIdx, rv, bcm_errmsg(rv)));
        return rv;
    }

    if (lp.counter == 0 || lp.policer == 0) {
        return BCM_E_NOT_FOUND;
    }
    *policerId = lp.policer;

    return rv;
}
#undef _VLAN_FUNC_NAME
#endif /* #ifdef BCM_FE2000_P3_SUPPORT */

/*
 *  Function:
 *    _bcm_fe2000_g2p3_vlan_stat_rw
 *  Purpose:
 *    accumulate/clear stats associated with all member ports of the given vlan
 *  Parameters:
 *   (IN)  unit      - bcm device number
 *   (IN)  clear     - TRUE to clear stats; FALSE to read
 *   (IN)  vlan      - vid to gather stats
 *   (IN)  cos       - cos of stat, is applicible
 *   (IN)  type      - stat to get/clear
 *   (OUT) val       - returned stat value
 *  Returns:
 *    BCM_E_*
 *  Notes:
 */
#ifdef BCM_FE2000_P3_SUPPORT
#define _VLAN_FUNC_NAME "_bcm_fe2000_g2p3_vlan_stat_rw"
int
_bcm_fe2000_g2p3_vlan_stat_rw(int unit,
                              int clear,
                              bcm_vlan_t vlan,
                              bcm_cos_t cos,
                              bcm_vlan_stat_t type,
                              uint64 *val)
{
    int                 rv = BCM_E_NONE;
    bcm_pbmp_t          pbmp, ubmp;
    int                 port = -1;
    bcm_policer_group_mode_t groupMode;
    uint32              policerId = ~0;
    uint64              uuVal = 0;

    if(!val) {
        rv = BCM_E_PARAM;
    } else {

        COMPILER_64_ZERO(*val);

        if(vlan <=  BCM_VLAN_MAX) {

            rv = _bcm_fe2000_vlan_port_fetch(unit, vlan, &pbmp, &ubmp);
            if (BCM_FAILURE(rv)) {
                VLAN_ERR((VLAN_FUNCMSG1("Failed vlan 0x%03x port fetch: %d %s\n"),
                          vlan, rv, bcm_errmsg(rv)));
            } else {

                /* For each vlan member port, accumulate the given stat */
                BCM_PBMP_ITER(pbmp, port) {
                    
                    rv = _bcm_fe2000_g2p3_vlan_pv_policer_get(unit, port, vlan,
                                                              &policerId);
                    if (BCM_FAILURE(rv)) {
                        if (rv == BCM_E_NOT_FOUND) {
                            VLAN_ERR((VLAN_FUNCMSG1("No policer/counter found on port,vid="
                                                    "%d,0x%03x\n"),
                                      port, vlan));
                                rv = BCM_E_PARAM;
                        } else {
                            VLAN_ERR((VLAN_FUNCMSG1("Failed to retrieve policer/counter for"
                                                    " port,vid=%d,0x%03x: %d %s\n"),
                                      port, vlan, rv, bcm_errmsg(rv)));
                        }
                    } else {
                        
                        rv = _bcm_fe2000_policer_group_mode_get(unit, policerId, &groupMode);
                        if (BCM_FAILURE(rv)) {
                            VLAN_ERR((VLAN_FUNCMSG1("Failed to policer group mode "
                                                    "policer=0x%04x: %d %s\n"),
                                      policerId, rv, bcm_errmsg(rv)));
                        } else {
                        
                            VLAN_VERB((VLAN_FUNCMSG1("port,vid=%d,0x%03x policer=0x%04x, "
                                                     "groupMode=%d(%s)\n"),
                                       port, vlan, policerId, groupMode,
                                       _bcm_fe2000_policer_group_mode_to_str(groupMode)));
                            
                            rv = _bcm_fe2000_g2p3_vlan_policer_stat_rw(unit, clear, policerId,
                                                                       groupMode, cos, type,
                                                                       &uuVal);
                            if (BCM_FAILURE(rv)) {
                                VLAN_ERR((VLAN_FUNCMSG1("Failed to %s vlan policer stat: %d %s\n"),
                                          clear ? "clear" : "get", rv, bcm_errmsg(rv)));
                            } else {
                            
                                VLAN_VERB((VLAN_FUNCMSG1("Port,Vid=%d,0x%03x count=%lld\n"),
                                           port, vlan, uuVal));
                                COMPILER_64_ADD_64(*val, uuVal);
                            }
                        }
                    }
                }    
            }
        } else {
            /* Statistics for non TB VSI */
            uint8_t             vsitype;
            bcm_policer_t       pol_id;

            rv = _sbx_gu2_get_vsi_type(unit, vlan, &vsitype);    
            if(BCM_SUCCESS(rv)) {

                switch(vsitype) {
                    case BCM_GPORT_MIM_PORT:
                        rv = _bcm_fe2000_mim_vpn_policer_get(unit, vlan, &pol_id);
                        break;

                    default:
                        rv = BCM_E_PARAM;
                        VLAN_ERR((VLAN_FUNCMSG1("Policer not supported on this "
                                                "vlan=%d,0x%03x : %d %s\n"),
                                  vlan, rv, bcm_errmsg(rv))); 
                        break;
                }

                if(BCM_SUCCESS(rv)) {

                    rv = _bcm_fe2000_policer_group_mode_get(unit, pol_id, &groupMode);
                    if (BCM_FAILURE(rv)) {
                        VLAN_ERR((VLAN_FUNCMSG1("Failed to policer group mode "
                                                "policer=0x%04x: %d %s\n"),
                                  policerId, rv, bcm_errmsg(rv)));
                    } else {
                        
                        VLAN_VERB((VLAN_FUNCMSG1("vid=0x%03x policer=0x%04x, "
                                                 "groupMode=%d(%s)\n"),
                                   vlan, policerId, groupMode,
                                   _bcm_fe2000_policer_group_mode_to_str(groupMode)));
                        
                        rv = _bcm_fe2000_g2p3_vlan_policer_stat_rw(unit, clear, (uint32)pol_id,
                                                                   groupMode, cos, type,
                                                                   &uuVal);
                        if (BCM_FAILURE(rv)) {
                            VLAN_ERR((VLAN_FUNCMSG1("Failed to %s vlan policer stat: %d %s\n"),
                                      clear ? "clear" : "get", rv, bcm_errmsg(rv)));
                        } else {
                            
                            VLAN_VERB((VLAN_FUNCMSG1("Vid=%d,0x%03x count=%lld\n"),
                                       vlan, uuVal));
                            COMPILER_64_ADD_64(*val, uuVal);
                        }
                    }                    
                } else {
                    VLAN_ERR((VLAN_FUNCMSG1("Failed to retrieve Policer associated "
                                            "with vlan=%d,0x%03x : %d %s\n"),
                              vlan, rv, bcm_errmsg(rv)));                
                    rv = BCM_E_PARAM;
                }
            } else {
                VLAN_ERR((VLAN_FUNCMSG1("Failed to retrieve VLAN Service Type for "
                                        "vlan=0x%03x : %d %s\n"),
                          vlan, rv, bcm_errmsg(rv)));                
                rv = BCM_E_PARAM;
            }
        }
        
        VLAN_VERB((VLAN_FUNCMSG1("returned value %lld\n"), *val));
    }    
    return rv;
}
#undef _VLAN_FUNC_NAME
#endif /* #ifdef BCM_FE2000_P3_SUPPORT */




/*
 *  Function:
 *    _bcm_fe2000_vlan_stat_rw
 *  Purpose:
 *    Access statistics for a particular VLAN
 *  Parameters:
 *    in int unit - unit to manipulate
 *    in int clear - zero to read, nonzero to clear
 *    in bcm_vlan_t vlan - VLAN whose stats are to be accessed
 *    in bcm_cos_t cos - COS level for the stats to be accessed
 *    in bcm_vlan_stat_t type - which statistic is to be accesssed
 *    out uint64 *val - where to put the value (if reading)
 *  Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 *  Notes:
 *    Limited parameter checking is performed here.
 *    There is no write support on SBX; only read and clear.
 *    Some stats are aggregates of other stats.  Clearing such stats will clear
 *    all of the contributors, just as reading them will sum the contributors.
 *    BCM_COS_INVALID as COS level indicates to collect stats for all COS
 *    levels.  In some cases, this implies aggregation, but aggregation of COS
 *    levels can be disabled at compile time; in other cases, the statistic
 *    already has all COS levels aggregated, so BCM_COS_INVALID is the only way
 *    to access the statistic, even when COS aware counters are in use.
 */

#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_stat_rw"
int
_bcm_fe2000_vlan_stat_rw(int unit,
                         int clear,
                         bcm_vlan_t vlan,
                         bcm_cos_t cos,
                         bcm_vlan_stat_t type,
                         uint64 *val)
{
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        return _bcm_fe2000_g2p3_vlan_stat_rw(unit, clear, vlan, cos, type,
                                             val);
        break;
#endif
    default:
        return BCM_E_CONFIG;
    }
}
#undef _VLAN_FUNC_NAME


/*
 *  Function:
 *    bcm_fe2000_vlan_stat_get
 *  Description:
 *    Get the specified vlan statistic from the chip
 *  Parameters:
 *    in int unit - the unit to manipulate
 *    in bcm_vlan_t vlan - the VLAN whose stats are to be accessed
 *    in bcm_cos_t cos - the COS level for the stats to access
 *    in bcm_vlan_stat_t stat - the statistic to read
 *    out uint64 *val - where to put the retrieved value
 *  Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 */

#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_stat_get"
int
bcm_fe2000_vlan_stat_get(int unit,
                         bcm_vlan_t vlan,
                         bcm_cos_t cos,
                         bcm_vlan_stat_t stat,
                         uint64 *val)
{
    int         result;

    BCM_STAT_VVERB(("%s(%d, %d, %d, %d, *) - Enter\n",
                    FUNCTION_NAME(), unit, cos, vlan, stat));

    /* make sure the VLAN is valid */
    /* STAT_CHECK_VLAN(unit, vlan); */
    if (vlan == 0) {
        return BCM_E_PARAM;
    }

    result = _bcm_fe2000_vlan_stat_rw(unit, FALSE, vlan, cos, stat, val);

    BCM_STAT_VVERB(("%s(%d, %d, %d, %d, &(0x%08X %08X)) - Exit %d (%s)\n",
                    FUNCTION_NAME(), unit, vlan, cos, stat,
                    u64_H(*val), u64_L(*val), result,
                    bcm_errmsg(result)));

    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *  Function:
 *    bcm_fe2000_vlan_stat_get32
 *  Description:
 *    Get the specified vlan statistic from the chip, but only 32b wide
 *  Parameters:
 *    in int unit - the unit to manipulate
 *    in bcm_vlan_t vlan - the VLAN whose stats are to be accessed
 *    in bcm_cos_t cos - the COS level for the stats to access
 *    in bcm_vlan_stat_t stat - the statistic to read
 *    out uint32 *val - where to put the retrieved value
 *  Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 *  Notes:
 *    Same as bcm_fe2000_vlan_stat_get, except converts result to 32-bit.
 */

#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_stat_get32"
int
bcm_fe2000_vlan_stat_get32(int unit,
                           bcm_vlan_t vlan,
                           bcm_cos_t cos,
                           bcm_vlan_stat_t stat,
                           uint32 *val)
{
    int         result;
    uint64      value64;

    BCM_STAT_VVERB(("%s(%d, %d, %d, %d, *) - Enter\n",
                    FUNCTION_NAME(), unit, vlan, cos, stat));

    /* make sure the VLAN is valid */
    /* STAT_CHECK_VLAN(unit, vlan); */
    if (vlan == 0) {
        return BCM_E_PARAM;
    }

    result = _bcm_fe2000_vlan_stat_rw(unit, FALSE, vlan, cos, stat, &value64);

    if (BCM_E_NONE == result) {
        if (value64 > 0x00000000FFFFFFFFull) {
            /* the value is too large */
            *val = 0xFFFFFFFF;
        } else {
            /* the value will fit */
            *val = u64_L(value64);
        }
    }

    BCM_STAT_VVERB(("%s(%d, %d, %d, %d, &(%08X)) - Exit %d (%s)\n",
                    FUNCTION_NAME(), unit, vlan,  cos, stat, *val,
                    result, bcm_errmsg(result)));

    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *  Function:
 *    bcm_fe2000_vlan_stat_set
 *  Description:
 *    Set the specified vlan statistic on the chip to a value
 *  Parameters:
 *    in int unit - the unit to manipulate
 *    in bcm_vlan_t vlan - the VLAN whose stats are to be accessed
 *    in bcm_cos_t cos - the COS level for the stats to access
 *    in bcm_vlan_stat_t stat - the statistic to write
 *    in uint64 val - the value to set
 *  Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 *  Notes:
 *    SBX only supports clearing, so val must be zero (0).
 */

#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_stat_set"
int
bcm_fe2000_vlan_stat_set(int unit,
                         bcm_vlan_t vlan,
                         bcm_cos_t cos,
                         bcm_vlan_stat_t stat,
                         uint64 val)
{
    int         result;

    BCM_STAT_VVERB(("%s(%d, %d, %d, %d, 0x%08X %08X) - Enter\n",
                    FUNCTION_NAME(),
                    unit,
                    vlan,
                    cos,
                    stat,
                    u64_H(val),
                    u64_L(val)));

    /* make sure the VLAN is valid */
    /* STAT_CHECK_VLAN(unit, vlan); */
    if (vlan == 0) {
        return BCM_E_PARAM;
    }

    STAT_CHECK_STAT_VALUE(val);     /* Only allowed to set to zero */

    result = _bcm_fe2000_vlan_stat_rw(unit, TRUE, vlan, cos, stat, &val);

    BCM_STAT_VVERB(("%s(%d, %d, %d, %d, 0x%08X %08X) - Exit %d (%s)\n",
                    FUNCTION_NAME(),
                    unit,
                    vlan,
                    cos,
                    stat,
                    u64_H(val),
                    u64_L(val),
                    result,
                    bcm_errmsg(result)));

    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *  Function:
 *    bcm_fe2000_vlan_stat_set32
 *  Description:
 *    Set the specified vlan statistic on the chip to a 32b value
 *  Parameters:
 *    in int unit - the unit to manipulate
 *    in bcm_vlan_t vlan - the VLAN whose stats are to be accessed
 *    in bcm_cos_t cos - the COS level for the stats to access
 *    in bcm_vlan_stat_t stat - the statistic to write
 *    in uint32 val - the value to set
 *  Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 *  Notes:
 *    SBX only supports clearing, so val must be zero (0).
 */

#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_stat_set32"
int
bcm_fe2000_vlan_stat_set32(int unit,
                           bcm_vlan_t vlan,
                           bcm_cos_t cos,
                           bcm_vlan_stat_t stat,
                           uint32 val)
{
    int         result;
    uint64      value64;

    BCM_STAT_VVERB(("%s(%d, %d, %d, %d, %08X) - Enter\n",
                    FUNCTION_NAME(),
                    unit,
                    vlan,
                    cos,
                    stat,
                    val));

    /* make sure the VLAN is valid */
    /* STAT_CHECK_VLAN(unit, vlan); */
    if (vlan == 0) {
        return BCM_E_PARAM;
    }

    STAT_CHECK_STAT_VALUE(val);     /* Only allowed to set to zero */

    COMPILER_64_SET(value64, 0, val);
    result = _bcm_fe2000_vlan_stat_rw(unit, TRUE, vlan, cos, stat, &value64);

    BCM_STAT_VVERB(("%s(%d, %d, %d, %d, %08X) - Exit %d (%s)\n",
                    FUNCTION_NAME(),
                    unit,
                    vlan,
                    cos,
                    stat,
                    val,
                    result,
                    bcm_errmsg(result)));

    return result;
}
#undef _VLAN_FUNC_NAME

/*
 *  Function:
 *    _bcm_fe2000_g2p3_vlan_stat_enable_set
 *  Description:
 *    Enable/Disable statistics on the indicated vlan
 *  Parameters:
 *    in int unit - the unit to manipulate
 *    in bcm_vlan_t vlan - the VLAN whose stats are to be controlled
 *    in int enable - zero to disable stats, nonzero to enable stats
 *  to set Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 */

#ifdef BCM_FE2000_P3_SUPPORT
#define _VLAN_FUNC_NAME "_bcm_fe2000_g2p3_vlan_stat_enable_set"
int
_bcm_fe2000_g2p3_vlan_stat_enable_set(int unit,
                                      bcm_vlan_t vlan,
                                      int enable)
{
    int                 rv = BCM_E_NONE;
    bcm_pbmp_t          pbmp, ubmp;
    int                 port;
    uint32_t            policerId;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%04X,%s) enter\n"),
               unit, vlan, enable?"TRUE":"FALSE"));

    if(enable) {
        /* Verify if policer is associated with all members of vlan ports to support statistics */
        if(vlan <=  BCM_VLAN_MAX) {

            rv = _bcm_fe2000_vlan_port_fetch(unit, vlan, &pbmp, &ubmp);
            if (BCM_FAILURE(rv)) {
                VLAN_ERR((VLAN_FUNCMSG1("Failed to find port members of vlan 0x%03x"
                                        ": %d %s\n"),
                          vlan, rv, bcm_errmsg(rv)));
                return rv;
            }

            /* Current implmentation expects caller to create policers and enable
             * counter collectoin on all member ports.  validate policers & counters
             * are present on all member ports.
             */
            BCM_PBMP_ITER(pbmp, port) {
                rv = _bcm_fe2000_g2p3_vlan_pv_policer_get(unit, port, vlan,
                                                          &policerId);
                if (rv == BCM_E_NOT_FOUND) {
                    VLAN_ERR((VLAN_FUNCMSG1("Failed to find policer/counter on "
                                           "port,vid=%d,0x%03x\n"),
                             port, vlan));

                    return BCM_E_PARAM;
                }

                if (BCM_FAILURE(rv)) {
                    VLAN_ERR((VLAN_FUNCMSG1("Failed to retrieve policer/counter for "
                                            "port,vid=%d,0x%03x : %d %s\n"),
                              port, vlan, rv, bcm_errmsg(rv)));
                    return rv;
                }

                VLAN_VVERB((VLAN_FUNCMSG1("port,vid=%d,0x%03x policer=0x%04x\n"),
                            port, vlan, policerId));
            }
        } else {
            uint8_t vsitype;
            uint8_t verify=1;

            rv = _sbx_gu2_get_vsi_type(unit, vlan, &vsitype);    
            if(BCM_SUCCESS(rv)) {
                switch(vsitype) {
                case BCM_GPORT_MIM_PORT:
                    rv = _bcm_fe2000_mim_vpn_policer_set(unit, vlan, 0, verify);  
                    break;
                    
                default:
                    rv = BCM_E_UNAVAIL;
                    break;
                }    
            } else {
                VLAN_ERR((VLAN_FUNCMSG1("Failed to retrieve VLAN Service Type for "
                                        "vlan=%d,0x%03x : %d %s\n"),
                          vlan, rv, bcm_errmsg(rv)));                
                rv = BCM_E_INTERNAL;
            }
        }    
    } else {
        VLAN_WARN((VLAN_FUNCMSG1("Stat Disable is not supported\n"),
                   unit));
    }

    return rv;
}
#undef _VLAN_FUNC_NAME
#endif /* #ifdef BCM_FE2000_P3_SUPPORT */

/*
 *  Function:
 *    _bcm_fe2000_vlan_stat_enable_set
 *  Purpose:
 *    Enables/verifies ability of per vlan statistics
 *  Parameters:
 *    (IN)  unit   - bcm device number
 *    (IN)  vlan   - vlan to enable statistics
 *    (IN)  enable - enable/disable
 *  Returns:
 *    BCM_E_*
 *  Notes:
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_stat_enable_set"
int
_bcm_fe2000_vlan_stat_enable_set(int unit,
                                bcm_vlan_t vlan,
                                int enable)
{
    /* make sure the VLAN is valid */
    /* STAT_CHECK_VLAN(unit, vlan); */
    if (vlan == 0) {
        return BCM_E_PARAM;
    }

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        return _bcm_fe2000_g2p3_vlan_stat_enable_set(unit, vlan, enable);
        break;
#endif
    default:
        return BCM_E_CONFIG;
    }
}
#undef _VLAN_FUNC_NAME

/*
 *  Function:
 *    bcm_fe2000_vlan_stat_enable_set
 *  Description:
 *    Enable/Disable statistics on the indicated vlan
 *  Parameters:
 *    in int unit - the unit to manipulate
 *    in bcm_vlan_t vlan - the VLAN whose stats are to be controlled
 *    in int enable - zero to disable stats, nonzero to enable stats
 *  to set Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 */

#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_stat_enable_set"
int
bcm_fe2000_vlan_stat_enable_set(int unit,
                                bcm_vlan_t vlan,
                                int enable)
{
    /* make sure the VLAN is valid */
    /* STAT_CHECK_VLAN(unit, vlan); */
    if (vlan == 0) {
        return BCM_E_PARAM;
    }
    return _bcm_fe2000_vlan_stat_enable_set(unit, vlan, enable);
}
#undef _VLAN_FUNC_NAME



/*
 * Function:
 *      bcm_fe2000_vlan_policer_get
 * Description:
 *      Retrieve the policer associated with this vlan
 * Parameters:
 *      unit   - device unit number.
 *      vid    - Vlan ID
 *      pol_id - (OUT) policer id associated with the Vlan
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_*         - Appropriately
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_policer_get"
int
bcm_fe2000_vlan_policer_get(int unit,
                            bcm_vlan_t vlan,
                            bcm_policer_t *pol_id)
{
    int                         result = BCM_E_FAIL;

    VLAN_VERB(("%s(%d, %d, 0x%08X) called \n",
               FUNCTION_NAME(),
               unit,
               vlan,
               (unsigned int)pol_id));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* check for valid Vlan value */
    if((0 == vlan) || (!pol_id)) {
        return BCM_E_PARAM;
    }

    if (SOC_IS_SBX_G2P3(unit)) {
#ifdef BCM_FE2000_P3_SUPPORT
        /* If > 4K, get VSI type. Issue policer set 
         * based on VSI-GPORT */
        if(vlan > BCM_VLAN_MAX) {
            uint8_t vsitype;

            result = _sbx_gu2_get_vsi_type(unit, vlan, &vsitype);
            if(BCM_SUCCESS(result)) {            
                switch(vsitype) {
                case BCM_GPORT_MIM_PORT:
                    result = _bcm_fe2000_mim_vpn_policer_get(unit, vlan, pol_id);  
                    break;
                    
                default:
                    result = BCM_E_UNAVAIL;
                    break;
                }
            } else {
                VLAN_ERR((VLAN_FUNCMSG1("Failed to retrieve VLAN Service Type for "
                                        "vlan=%d,0x%03x : %d %s\n"),
                          vlan, result, bcm_errmsg(result)));                
                result = BCM_E_INTERNAL;
            }
        }
        else {
            bcm_pbmp_t pbmp, ubmp;
            bcm_port_t port;
            
            result = _bcm_fe2000_vlan_port_fetch(unit, vlan, &pbmp, &ubmp);
            if (result == BCM_E_NONE) {
                soc_sbx_g2p3_pv2e_t pv2e;
                soc_sbx_g2p3_lp_t   lp;
                
                BCM_PBMP_ITER(pbmp, port) {
                    soc_sbx_g2p3_pv2e_t_init(&pv2e);
                    
                    result = SOC_SBX_G2P3_PV2E_GET(unit, port, vlan, &pv2e);
                    if (BCM_FAILURE(result)) {
                        continue;
                    }
                    
                    if (pv2e.lpi) {
                        soc_sbx_g2p3_lp_t_init(&lp);
                        
                        result = soc_sbx_g2p3_lp_get(unit, pv2e.lpi, &lp);
                        if (lp.policer) {
                            *pol_id = lp.policer;
                            return BCM_E_NONE;
                        }
                    }
                }
                
                result = BCM_E_NOT_FOUND;
            }
        }
#endif /* BCM_FE2000_P3_SUPPORT */
    }

    return result;
}
#undef _VLAN_FUNC_NAME



/*
 * Function:
 *      _bcm_fe2000_g2p3_vlan_port_policer_set(
 * Description:
 *      Set the policer associated with this port,vlan
 * Parameters:
 *      unit   - device unit number.
 *      vlan   - Vlan ID
 *      port   - port to set policer
 *      pol_id - policer id to associate with the Vlan
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_*         - Appropriately
 */
#ifdef BCM_FE2000_P3_SUPPORT
#define _VLAN_FUNC_NAME  FUNCTION_NAME()

int
_bcm_fe2000_g2p3_vlan_port_policer_set(int unit, bcm_vlan_t vlan,
                                       bcm_port_t port, bcm_policer_t pol_id)
{
    int rv = BCM_E_NONE;
    soc_sbx_g2p3_pv2e_t pv2e;
    soc_sbx_g2p3_lp_t   lp;
    uint32  logicalPort;

    rv = SOC_SBX_G2P3_PV2E_GET(unit, port, vlan, &pv2e);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("failed to get %d:pv2e[%d,0x%03X] : %d %s\n"),
                  unit, port, vlan, rv, bcm_errmsg(rv)));
        return rv;
    }

    /* When 0 is passed as a policer id, it is consider removed.
     *
     * Free logical port when policer is removed if
     *   all other lp fields match the physical port's LP
     *   and set pv2e.lpi back to port's LP
     *
     * Policer is valid - allocate a new LP, if one does not already exist
     */

    /* always need the current Lp */
    if (pv2e.lpi == 0) {
        logicalPort = port;
    } else {
        logicalPort = pv2e.lpi;
    }

    rv = soc_sbx_g2p3_lp_get(unit, logicalPort, &lp);
    if (BCM_FAILURE(rv)) {

        VLAN_ERR((VLAN_FUNCMSG1("Failed to get LP 0x%x: %d %s\n"),
                  logicalPort, rv, bcm_errmsg(rv)));
        return rv;
    }

    if (pol_id) {

        /* valid policer id, add to logical port, allocate if necessary */
        if (logicalPort == port) {

            /* pv2e.lpi points to physical port's logical port,
             * allocate a new logical port
             */
            rv = _sbx_gu2_resource_alloc(unit, SBX_GU2K_USR_RES_LPORT,
                                         1, &logicalPort, 0);
            if (BCM_FAILURE(rv)) {
                VLAN_ERR((VLAN_FUNCMSG1("failed to allocate LP: %d %s\n"),
                          rv, bcm_errmsg(rv)));
                return rv;
            }

            VLAN_VERB((VLAN_FUNCMSG1("New LP needed to support policer; "
                                     "allocated lp 0x%x\n"), logicalPort));

            pv2e.lpi = logicalPort;
            /* inherit port's lp settings; don't init lp */
        }

        rv = _bcm_fe2000_g2p3_policer_lp_program(unit, pol_id, &lp);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("Failed to program lp with policerId=0x%x"
                                    ": %d %s\n"),
                      pol_id, rv, bcm_errmsg(rv)));
        }

        if (BCM_SUCCESS(rv)) {
            rv = soc_sbx_g2p3_lp_set(unit, pv2e.lpi, &lp);
            if (BCM_FAILURE(rv)) {
                VLAN_ERR((VLAN_FUNCMSG1("Failed to set LP 0x%x: %d %s\n"),
                          pv2e.lpi, rv, bcm_errmsg(rv)));
            }
        }

        if (BCM_SUCCESS(rv)) {
            rv = SOC_SBX_G2P3_PV2E_SET(unit, port, vlan, &pv2e);
            if (BCM_FAILURE(rv)) {
                VLAN_ERR((VLAN_FUNCMSG1("Failed to write pv2e[%d,0x%03X]: "
                                        "%d %s\n"),
                          port, vlan, rv, bcm_errmsg(rv)));
            }
        }

        if (BCM_FAILURE(rv)) {
            _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_LPORT, 1,
                                   &logicalPort, 0);
            VLAN_VERB((VLAN_FUNCMSG1("Freeing LP 0x%x due to error above.\n"),
                       logicalPort));
            return rv;
        }

    } else {
        soc_sbx_g2p3_lp_t physLp;
        /* polier id is 0, consider removed, free resources if applicible */

        if (logicalPort == port) {
            VLAN_VERB((VLAN_FUNCMSG1("Attemted to remove policer from physical"
                                     " port's logical port - ignored\n")));
            return BCM_E_NONE;
        }

        rv = soc_sbx_g2p3_lp_get(unit, port, &physLp);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("Failed to get lp 0x%x: %d %s\n"),
                      port, rv, bcm_errmsg(rv)));
            return rv;
        }

        rv = _bcm_fe2000_g2p3_policer_lp_program(unit, pol_id, &lp);
        if (BCM_FAILURE(rv)) {
            VLAN_ERR((VLAN_FUNCMSG1("Failed to program policerId 0x%x: "
                                    "%d %s\n"),
                      pol_id, rv, bcm_errmsg(rv)));
            return rv;
        }

        /* if, after policer is removed, if logical ports are equal,
         * free the resource and re-assign the port,vid's lp to the
         * physical port's lp
         */
        if (sal_memcmp(&lp, &physLp, sizeof(lp)) == 0) {
            VLAN_VERB((VLAN_FUNCMSG1("LPs are equivalent, free lp 0x%x\n"),
                       logicalPort));
            _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_LPORT, 1,
                                   &logicalPort, 0);
            pv2e.lpi = 0;

            rv = SOC_SBX_G2P3_PV2E_SET(unit, port, vlan, &pv2e);
            if (BCM_FAILURE(rv)) {
                VLAN_ERR((VLAN_FUNCMSG1("Failed to write pv2e[%d,0x%03X]: "
                                        "%d %s\n"),
                          port, vlan, rv, bcm_errmsg(rv)));
                return rv;
            }
        } else {
            /* LPs are different after policer removed */
            VLAN_VERB((VLAN_FUNCMSG1("LPs found to be different; keeping"
                                     " lp 0x%x\n"), logicalPort));
            rv = soc_sbx_g2p3_lp_set(unit, logicalPort, &lp);
            if (BCM_FAILURE(rv)) {
                VLAN_ERR((VLAN_FUNCMSG1("Failed to write LP 0x%x: %d %s\n"),
                          logicalPort, rv, bcm_errmsg(rv)));
                return rv;
            }

        }
    }

    return BCM_E_NONE;
}
#undef _VLAN_FUNC_NAME
#endif /* #if BCM_FE2000_P3_SUPPORT */


/*
 * Function:
 *      bcm_fe2000_vlan_policer_set
 * Description:
 *      Set the policer associated with this vlan
 * Parameters:
 *      unit   - device unit number.
 *      vid    - Vlan ID
 *      pol_id - policer id to associate with the Vlan
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_*         - Appropriately
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_policer_set"
int
bcm_fe2000_vlan_policer_set(int unit,
                            bcm_vlan_t vlan,
                            bcm_policer_t pol_id)
{
    int result = BCM_E_FAIL;

    VLAN_VERB(("%s(%d, %d, %d) called \n", FUNCTION_NAME(), unit, vlan, pol_id));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    /* check for valid Vlan value */
    if(0 == vlan || pol_id < 0) {
        VLAN_ERR(("Bad Vlanid=%d or Policerid=%d \n", vlan, pol_id));
        return BCM_E_BADID;
    }

    if (pol_id) {
        bcm_policer_group_mode_t    group_mode;
        result = _bcm_fe2000_policer_group_mode_get(unit, pol_id, &group_mode);
        if (BCM_FAILURE(result)) {
            VLAN_ERR(("Failed to get policer_id=%d group mode\n", pol_id));
            return result;
        }
    }

    result = BCM_E_UNAVAIL;

    if (SOC_IS_SBX_G2P3(unit)) {
#ifdef BCM_FE2000_P3_SUPPORT    
        /* If > 4K, get VSI type. Issue policer set 
         * based on VSI-GPORT */
        if(vlan > BCM_VLAN_MAX) {
            uint8_t vsitype;

            result = _sbx_gu2_get_vsi_type(unit, vlan, &vsitype);
            if(BCM_SUCCESS(result)) { 
                switch(vsitype) {
                case BCM_GPORT_MIM_PORT:
                    result = _bcm_fe2000_mim_vpn_policer_set(unit, vlan, pol_id, 0);  
                    break;
                    
                default:
                    result = BCM_E_UNAVAIL;
                    break;
                }
            } else {
                VLAN_ERR((VLAN_FUNCMSG1("Failed to retrieve VLAN Service Type for "
                                        "vlan=%d,0x%03x : %d %s\n"),
                          vlan, result, bcm_errmsg(result)));                
                result = BCM_E_INTERNAL;
            }
        } else {
            /* Traditional Bridging VLAN Policer Set */
            bcm_pbmp_t          pbmp, ubmp;
            bcm_port_t          port;
            
            result = _bcm_fe2000_vlan_port_fetch(unit, vlan, &pbmp, &ubmp);
            if (result == BCM_E_NONE) {
                BCM_PBMP_ITER(pbmp, port) {
                    result =
                        _bcm_fe2000_g2p3_vlan_port_policer_set(unit, vlan,
                                                               port, pol_id);
                }
            }
        }
#endif /* BCM_FE2000_P3_SUPPORT */
    }

    if(BCM_SUCCESS(result)) {
        VLAN_VERB(("Set policer_id=%d on vlan=%d \n", pol_id, vlan));
    } else {
        VLAN_ERR(("Failed to set policer_id=%d on vlan=%d"
                  " result=%d %s\n", pol_id, vlan,result,bcm_errmsg(result)));
    }
    
    return result;
}
#undef _VLAN_FUNC_NAME

#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_port_policer_get"
int
bcm_fe2000_vlan_port_policer_get(int unit, bcm_vlan_t vlan,
                                 bcm_port_t port, bcm_policer_t *pol_id)
{
    int rv = BCM_E_NONE;
#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_pv2e_t pv2e;
    soc_sbx_g2p3_lp_t   lp;
#endif

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    if ((!pol_id) || (vlan <= 0) || (vlan > BCM_VLAN_MAX) ||
        (!SOC_PORT_VALID(unit, port))) {
        return BCM_E_PARAM;
    }

    if (SOC_IS_SBX_G2P3(unit)) {
#ifdef BCM_FE2000_P3_SUPPORT
        soc_sbx_g2p3_pv2e_t_init(&pv2e);
        rv = SOC_SBX_G2P3_PV2E_GET(unit, port, vlan, &pv2e);
        if (pv2e.lpi) {
            soc_sbx_g2p3_lp_t_init(&lp);
            rv = soc_sbx_g2p3_lp_get(unit, pv2e.lpi, &lp);
            if (rv == BCM_E_NONE) {
                *pol_id = lp.policer;
            }
        } else {
            /* vlan,port in invalid state. A logical port entry should exist
            if port is a member of vlan */
            rv = BCM_E_CONFIG;
        }
#endif
    }

    return rv;
}
#undef _VLAN_FUNC_NAME

#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_port_policer_set"
int
bcm_fe2000_vlan_port_policer_set(int unit, bcm_vlan_t vlan,
                                 bcm_port_t port, bcm_policer_t pol_id)
{
    int rv = BCM_E_NONE;

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    if ((pol_id < 0) || (pol_id > _bcm_policer_max_id_get(unit)) || (vlan <= 0)
        || (vlan > BCM_VLAN_MAX) || (!SOC_PORT_VALID(unit, port))) {
        return BCM_E_PARAM;
    }

    /* get the vlan lock */
    VLAN_LOCK_TAKE;

    if (SOC_IS_SBX_G2P3(unit)) {
#ifdef BCM_FE2000_P3_SUPPORT
        rv = _bcm_fe2000_g2p3_vlan_port_policer_set(unit, vlan, port, pol_id);
#endif /* BCM_FE2000_P3_SUPPORT */
    }

    VLAN_LOCK_RELEASE;

    return rv;
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      bcm_fe2000_vlan_port_qosmap_set
 *   Purpose
 *      Set QoS mapping behaviour on a VLAN GPORT
 *   Parameters
 *      (in) int unit          = BCM device number
 *      (in) bcm_gport_t gport = VLAN GPORT
 *      (in) int ingrMap     = ingress map
 *      (in) int egrMap      = egress map
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *     Little parameter checking is done here.
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_port_qosmap_set"
int
bcm_fe2000_vlan_port_qosmap_set(int unit,
                                bcm_gport_t gport,
                                int ingrMap,
                                int egrMap,
                                uint32 ingFlags, 
                                uint32 egrFlags)
{
#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_lp_t p3lp;
    soc_sbx_g2p3_ft_t p3ft;
    soc_sbx_g2p3_oi2e_t p3oi2e;
    soc_sbx_g2p3_eteencap_t p3eteEncap;
    soc_sbx_g2p3_etel2_t p3eteL2;
    int result = BCM_E_NONE;
    uint32 logicalPort = ~0;
    uint32 gportId, fti, gu2_res;
    uint32 queueId;
    _bcm_fe2k_vlan_gport_t *vlanPortData = NULL;
    bcm_module_t locMod;
    bcm_module_t tgtMod;
    bcm_port_t tgtPort;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%08X,%d,%d) enter\n"),
               unit,
               gport,
               ingrMap,
               egrMap));

    /* Check unit valid; return unit error if not */
    VLAN_UNIT_VALID_CHECK;

    /* Check for proper initialisation */
    VLAN_UNIT_INIT_CHECK;

    if (!SOC_IS_SBX_G2P3(unit)) {
        VLAN_ERR((VLAN_FUNCMSG1("not supported on this microcode\n")));
        return BCM_E_UNAVAIL;
    }

    if (!BCM_GPORT_IS_VLAN_PORT(gport)) {
        VLAN_ERR((VLAN_FUNCMSG1("%08X is not a valid VLAN GPORT\n"),
                  gport));
        return BCM_E_PARAM;
    }

    /* get the vlan lock */
    VLAN_LOCK_TAKE;

    /* get the gport information */
    gportId = BCM_GPORT_VLAN_PORT_ID_GET(gport);
    fti = VLAN_VGPORT_ID_TO_FT_INDEX(unit, gportId);

    gu2_res = SBX_GU2K_USR_RES_FTE_LOCAL_GPORT;
    result = _sbx_gu2_resource_test(unit, gu2_res, fti);

    if (BCM_E_EXISTS == result) {
        logicalPort = _vlan_state[unit]->gportInfo.lpid[gportId];
        result = BCM_E_NONE;
        vlanPortData = (_bcm_fe2k_vlan_gport_t*)(SBX_LPORT_DATAPTR(unit,
                                                                   logicalPort));
        if (!vlanPortData) {
            VLAN_ERR((VLAN_FUNCMSG1("gport %08X is not valid on unit %d\n"),
                      gport,
                      unit));
            result = BCM_E_NOT_FOUND;
        }
        if (BCM_GPORT_VLAN_PORT != SBX_LPORT_TYPE(unit, logicalPort)) {
            VLAN_ERR((VLAN_FUNCMSG1("unit %d gport %08X disagrees with"
                                    " stored type %02X\n"),
                      unit,
                      gport,
                      SBX_LPORT_TYPE(unit, logicalPort)));
            result = BCM_E_CONFIG;
        }
    } else {
        VLAN_ERR((VLAN_FUNCMSG1("unable to access unit %d GPORT %08X:"
                                " %d (%s)\n"),
                  unit,
                  gport,
                  result,
                  _SHR_ERRMSG(result)));
    }

    if ((egrMap >= 0) && (BCM_E_NONE == result)) {
        /* gather some additional information */
        locMod = -1;
        tgtMod = -1;
        tgtPort = -1;
        result = _bcm_fe2000_map_vlan_gport_targets(unit,
                                                    gport,
                                                    &locMod,
                                                    &tgtMod,
                                                    &tgtPort,
                                                    NULL,
                                                    &queueId);

        /* find the egress path starting point */
        if (BCM_E_NONE == result) {
            result = soc_sbx_g2p3_ft_get(unit, fti, &p3ft);
            if (BCM_E_NONE != result) {
                VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:ft[%08X]: "
                                        "%d (%s)\n"),
                          unit, fti, result,
                          _SHR_ERRMSG(result)));
            }
        } /* if (BCM_E_NONE == result) */

        /* follow and update the egress path if it's on the local module */
        if ((BCM_E_NONE == result) && (locMod == tgtMod)) {
            result = soc_sbx_g2p3_oi2e_get(unit,
                                           p3ft.oi - SBX_RAW_OHI_BASE,
                                           &p3oi2e);
            if (BCM_E_NONE == result) {
                if (p3oi2e.eteptr) {
                    /* has egress path; follow it */
                    result = soc_sbx_g2p3_eteencap_get(unit,
                                                       p3oi2e.eteptr,
                                                       &p3eteEncap);
                    if (BCM_E_NONE == result) {
                        result = soc_sbx_g2p3_etel2_get(unit,
                                                        p3eteEncap.l2ete,
                                                        &p3eteL2);
                        if (BCM_E_NONE != result) {
                            VLAN_ERR((VLAN_FUNCMSG1(
                                          "unable to read %d:ete[%08X].l2:"
                                          " %d (%s)\n"),
                                      unit,
                                      p3eteEncap.l2ete,
                                      result,
                                      _SHR_ERRMSG(result)));
                        }
                    } else { /* if (BCM_E_NONE == result) */
                        VLAN_ERR((VLAN_FUNCMSG1(
                                      "unable to read %d:ete[%08X].encap:"
                                      " %d (%s)\n"),
                                  unit,
                                  p3oi2e.eteptr,
                                  result,
                                  _SHR_ERRMSG(result)));
                    } /* if (BCM_E_NONE == result) */
                } else {
                    /* no egress path -- hasn't been added yet? */
                    VLAN_WARN((VLAN_FUNCMSG1(
                                   "unable to follow egress path for"
                                   " unit %d gport %08X because it has"
                                   " no egress path; not added yet?\n"),
                               unit,
                               gport));
                }
            } else { /* if (BCM_E_NONE == result) */
                VLAN_ERR((VLAN_FUNCMSG1(
                              "unable to read %d:oi2e[%08X]: %d (%s)\n"),
                          unit,
                          p3ft.oi,
                          result,
                          _SHR_ERRMSG(result)));
            } /* if (BCM_E_NONE == result) */

            if ((BCM_E_NONE == result) && p3oi2e.eteptr) {
                /* update if successfully followed egress path */
                p3eteL2.remark = egrMap;
                p3eteL2.dscpremark = (egrFlags & BCM_QOS_MAP_L3)?1:0;
                result = soc_sbx_g2p3_etel2_set(unit, p3eteEncap.l2ete,
                                                &p3eteL2);
                if (BCM_E_NONE != result) {
                    VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:ete[%08X].l2:"
                                            " %d (%s)\n"),
                              unit,
                              p3eteEncap.l2ete,
                              result,
                              _SHR_ERRMSG(result)));
                }
            } /* if (BCM_E_NONE == result) */
        } /* if (result okay and local target) */
    } /* if setting egrMap */

    /* update the ingress path */
    if ((BCM_E_NONE == result) && (ingrMap >= 0)) {
        /* get the logical port (ingress map is here) */
        result = soc_sbx_g2p3_lp_get(unit, logicalPort, &p3lp);
        if (BCM_E_NONE != result) {
            VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:lp[%08X]: "
                                    "%d (%s)\n"),
                      unit,
                      logicalPort,
                      result,
                      _SHR_ERRMSG(result)));
        }

        if (BCM_E_NONE == result) {
            p3lp.qos = ingrMap;
            p3lp.usedscp = (ingFlags & BCM_QOS_MAP_L3)?1:0;
            p3lp.useexp = (ingFlags & BCM_QOS_MAP_MPLS)?1:0;
            result = soc_sbx_g2p3_lp_set(unit, logicalPort, &p3lp);
            if (BCM_E_NONE != result) {
                VLAN_ERR((VLAN_FUNCMSG1("unable to write %d:lp[%08X]:"
                                        " %d (%s)\n"),
                          unit,
                          logicalPort,
                          result,
                          _SHR_ERRMSG(result)));
            }
        } /* if (BCM_E_NONE == result && ingrMap >= 0 */
    }

    /* release the lock */
    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,*) return %d (%s)\n"),
               unit,
               result,
               _SHR_ERRMSG(result)));

    return result;
#else /* def BCM_FE2000_P3_SUPPORT */
    VLAN_VERB((VLAN_FUNCMSG("(%d,%08X,%d,%d) enter\n"),
               unit,
               gport,
               ingrMap,
               egrMap));
    return BCM_E_UNAVAIL;
#endif /* def BCM_FE2000_P3_SUPPORT */
}
#undef _VLAN_FUNC_NAME

/*
 *   Function
 *      _bcm_fe2000_vlan_port_qosmap_get
 *   Purpose
 *      Get the configured QOS mappings of a VLAN GPORT
 *   Parameters
 *      (in)  int unit          = BCM device number
 *      (in)  bcm_gport_t gport = VLAN GPORT
 *      (out) int ing_idx       = qos profile
 *      (out) int egr_idx       = remark index
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *     Little parameter checking is done here.  Lock must be taken by caller
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_vlan_port_qosmap_get"

static int
_bcm_fe2000_vlan_port_qosmap_get(int unit, bcm_gport_t gport,
                                 int *ing_idx, int *egr_idx,
                                 uint32 *ing_flags, uint32 *egr_flags)
{
    soc_sbx_g2p3_lp_t        lp;
    soc_sbx_g2p3_ft_t        fte;
    soc_sbx_g2p3_oi2e_t      oi;
    soc_sbx_g2p3_eteencap_t  ete_encap;
    soc_sbx_g2p3_etel2_t     etel2;
    bcm_module_t             mod, tgt_mod;
    bcm_port_t               tgt_port;
    int                      rv;
    _bcm_fe2k_vlan_gport_t  *data = NULL;
    uint32                   junk, gportId, lpi, fti, gu2_res;

    /* get the gport information */
    gportId = BCM_GPORT_VLAN_PORT_ID_GET(gport);
    fti = VLAN_VGPORT_ID_TO_FT_INDEX(unit, gportId);

    gu2_res = SBX_GU2K_USR_RES_FTE_LOCAL_GPORT;
    rv = _sbx_gu2_resource_test(unit, gu2_res, fti);

    if (rv == BCM_E_EXISTS) {

        rv  = BCM_E_NONE;

        lpi = _vlan_state[unit]->gportInfo.lpid[gportId];
        data = (_bcm_fe2k_vlan_gport_t*)(SBX_LPORT_DATAPTR(unit, lpi));
        
        if (data == NULL) {
            VLAN_ERR((VLAN_FUNCMSG1("gport 0x%08X is not valid on unit %d\n"),
                      gport, unit));
            rv = BCM_E_NOT_FOUND;
        }

        if (SBX_LPORT_TYPE(unit, lpi) != BCM_GPORT_VLAN_PORT) {
            VLAN_ERR((VLAN_FUNCMSG1("unit %d gport 0x%08X disagrees with"
                                    " stored type 0x%02X\n"),
                      unit, gport, SBX_LPORT_TYPE(unit, lpi)));
            rv = BCM_E_CONFIG;
        }
    } else {
        VLAN_ERR((VLAN_FUNCMSG1("unable to access unit %d GPORT 0x%08X:"
                                " %d (%s)\n"),
                  unit, gport, rv, _SHR_ERRMSG(rv)));
        return rv;
    }

    mod = tgt_mod = tgt_port = -1;
    rv = _bcm_fe2000_map_vlan_gport_targets(unit, gport, &mod,
                                            &tgt_mod, &tgt_port,
                                            NULL, &junk);
    
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("Failed to get gport 0x%08x targets: %s\n"),
                  gport, _SHR_ERRMSG(rv)));
        return rv;
    }
    
    if (tgt_mod != mod) {
        VLAN_ERR((VLAN_FUNCMSG1("Gport 0x%08x does not reside on "
                                "this module\n"), gport));
        return BCM_E_PARAM;
    }

    /* get ingress map 
     */    
    rv = soc_sbx_g2p3_lp_get(unit, lpi, &lp);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("failed to read %d:lp[0x%08X]: "
                                "%d (%s)\n"),
                  unit, lpi, rv, _SHR_ERRMSG(rv)));
        return rv;
    }

    *ing_idx    = lp.qos;
    *ing_flags  = BCM_QOS_MAP_INGRESS | BCM_QOS_MAP_L2;
    if (lp.usedscp) {
        *ing_flags |= BCM_QOS_MAP_L3;
    }
    if (lp.useexp) {
        *ing_flags |=  BCM_QOS_MAP_MPLS;   
    }


    /* Get the Egress Map
     */
    rv = soc_sbx_g2p3_ft_get(unit, fti, &fte);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("unable to read %d:ft[0x%08X]: %d (%s)\n"),
                  unit, fti, rv, _SHR_ERRMSG(rv)));
        return rv;
    }

    rv = soc_sbx_g2p3_oi2e_get(unit, fte.oi - SBX_RAW_OHI_BASE, &oi);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("Failed to read %d:oi2e[0x%08X]: %d (%s)\n"),
                  unit, fte.oi, rv, _SHR_ERRMSG(rv)));  
        return rv;
    }
        
    if (oi.eteptr == 0) {
        VLAN_ERR((VLAN_FUNCMSG1("invalid oi found on gport 0x%x\n"), gport));
        return BCM_E_PARAM;
    }

    rv = soc_sbx_g2p3_eteencap_get(unit, oi.eteptr, &ete_encap);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("failed to read %d:eteencap[0x%08X]:"
                                " %d (%s)\n"),
                  unit, oi.eteptr, rv, _SHR_ERRMSG(rv)));
         return rv;
    }

    rv = soc_sbx_g2p3_etel2_get(unit, ete_encap.l2ete, &etel2);
    if (BCM_FAILURE(rv)) {
        VLAN_ERR((VLAN_FUNCMSG1("failed to read %d:etel2[0x%08X]:"
                                " %d (%s)\n"),
                  unit, ete_encap.l2ete, rv, _SHR_ERRMSG(rv)));
        return rv;
    }

    *egr_idx = etel2.remark;
    *egr_flags = BCM_QOS_MAP_EGRESS | BCM_QOS_MAP_L2;
    if (etel2.dscpremark) {
        *egr_flags |= BCM_QOS_MAP_L3;
    }

    return rv;
}
#undef _VLAN_FUNC_NAME


/*
 *   Function
 *      bcm_fe2000_vlan_port_qosmap_get
 *   Purpose
 *      Get the configured QOS mappings of a VLAN GPORT
 *   Parameters
 *      (in)  int unit          = BCM device number
 *      (in)  bcm_gport_t gport = VLAN GPORT
 *      (out) int ing_idx        = qos profile
 *      (out) int egr_idx        = remark index
 *      (out) int ing_flags
 *      (out) int egr_flags
 *   Returns
 *      BCM_E_*
 */
#define _VLAN_FUNC_NAME "bcm_fe2000_vlan_port_qosmap_get"
int
bcm_fe2000_vlan_port_qosmap_get(int unit, bcm_gport_t gport,
                                int *ing_idx, int *egr_idx,
                                uint32 *ing_flags, uint32 *egr_flags)
{
#ifdef BCM_FE2000_P3_SUPPORT
    int rv;

    VLAN_VERB((VLAN_FUNCMSG("(%d,%08X,%d,%d) enter\n"), unit, gport));
    
    VLAN_UNIT_VALID_CHECK;
    VLAN_UNIT_INIT_CHECK;
    
    if (!SOC_IS_SBX_G2P3(unit)) {
        VLAN_ERR((VLAN_FUNCMSG1("not supported on this microcode\n")));
        return BCM_E_UNAVAIL;
    }

    if (!BCM_GPORT_IS_VLAN_PORT(gport)) {
        VLAN_ERR((VLAN_FUNCMSG1("%08X is not a valid VLAN GPORT\n"),
                  gport));
        return BCM_E_PARAM;
    }

    VLAN_LOCK_TAKE;
    rv = _bcm_fe2000_vlan_port_qosmap_get(unit, gport, ing_idx, egr_idx,
                                          ing_flags, egr_flags);
    VLAN_LOCK_RELEASE;

    VLAN_VERB((VLAN_FUNCMSG("(%d,*) return %d (%s)\n"),
               unit, rv, _SHR_ERRMSG(rv)));

    return rv;
#else /* def BCM_FE2000_P3_SUPPORT */
    VLAN_VERB((VLAN_FUNCMSG("(%d,%08X,%d,%d) enter\n"),
               unit,
               gport,
               ingrMap,
               egrMap));
    return BCM_E_UNAVAIL;
#endif /* def BCM_FE2000_P3_SUPPORT */
}
#undef _VLAN_FUNC_NAME


/*
 *  Function
 *      _bcm_fe2000_g2p3_vlan_nvid_pvv2e_flags_set
 *  Purpose
 *      Sets the vlan nvflags for pvv2e 
 *  Parameters
 *      (in) int unit  - BCM device number
 *      (in) int iport - Port number
 *      (in) _bcm_fe2k_nvid_pvv2e_control_flags_t flag
 */
#define _VLAN_FUNC_NAME "_bcm_fe2000_g2p3_vlan_nvid_pvv2e_flags_set"
int
_bcm_fe2000_g2p3_vlan_nvid_pvv2e_flags_set(int unit,
                                    int iport,
                                    _bcm_fe2k_nvid_pvv2e_control_flags_t flags)
{
    if (((iport) < 0) || ((iport) >= SBX_MAX_PORTS)) {
        VLAN_ERR((VLAN_FUNCMSG1("port %d invalid on unit %d\n"),
                  iport,
                  unit));
        return BCM_E_PARAM;
    }
    _vlan_state[unit]->nvflag[iport] = flags;
    return BCM_E_NONE;
}
#undef _VLAN_FUNC_NAME
                                          
