/*
 * $Id: multicast.c 1.78 Broadcom SDK $
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
 * Sirius Multicast API
 */

#include <soc/debug.h>
#include <soc/mem.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sirius.h>
#include <soc/mcm/memregs.h>
#include <soc/mcm/memacc.h>
#include <bcm/debug.h>
#include <bcm/error.h>
#include <bcm/mcast.h>
#include <bcm/multicast.h>
#include <bcm/trunk.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/trunk.h>
#include <bcm_int/sbx/sirius.h>
#include <bcm_int/sbx/sirius/multicast.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/port.h>

#include <shared/bitop.h>
#include <shared/idxres_fl.h>
#include <shared/idxres_mdb.h>

/*****************************************************************************
 *
 *  Implementation
 */

#define _SBX_SIRIUS_MC_EXCESS_VERBOSITY TRUE
#define _SBX_SIRIUS_MC_BG_SLEEP_WAKE_MSG FALSE
#define _SBX_SIRIUS_MC_BG_OTHER_MSG FALSE
#if 0 
#undef BCM_DEBUG
#define BCM_DEBUG(flags, stuff)   soc_cm_print stuff
#endif 
#define MC_WARN(stuff)       BCM_DEBUG(BCM_DBG_WARN     | BCM_DBG_MCAST, stuff)
#define MC_ERR(stuff)        BCM_DEBUG(BCM_DBG_ERR      | BCM_DBG_MCAST, stuff)
#define MC_VERB(stuff)       BCM_DEBUG(BCM_DBG_VERBOSE  | BCM_DBG_MCAST, stuff)
#define MC_VVERB(stuff)      BCM_DEBUG(BCM_DBG_VVERBOSE | BCM_DBG_MCAST, stuff)
#if _SBX_SIRIUS_MC_EXCESS_VERBOSITY
#define MC_EVERB(stuff)      BCM_DEBUG(BCM_DBG_VVERBOSE | BCM_DBG_MCAST, stuff)
#else /* _SBX_SIRIUS_MC_EXCESS_VERBOSITY */
#define MC_EVERB(stuff)
#endif /* _SBX_SIRIUS_MC_EXCESS_VERBOSITY */
#if defined(__GNUC__) && !defined(__PEDANTIC__)
/* PLEASE DO NOT CHANGE __FUNCTION__ IN THE TWO LINES BELOW!!! */
#define MC_MSG(string) "%s[%d]%s" string, __FILE__, __LINE__, __FUNCTION__
#define MC_MSG1(string) "%s[%d]%s: " string, __FILE__, __LINE__, __FUNCTION__
/* PLEASE DO NOT CHANGE __FUNCTION__ IN THE TWO LINES ABOVE!!! */
#else /* defined(__GNUC__) && !defined(__PEDANTIC__) */
#define MC_MSG(string) "%s[%d]" string, __FILE__, __LINE__
#define MC_MSG1(string) "%s[%d]: " string, __FILE__, __LINE__
#endif /* defined(__GNUC__) && !defined(__PEDANTIC__) */



/*
 *  Since port bitmaps are limited to fewer ports than we need, here's a
 *  bitmap, using the shared bitmap library, that is big enough for our use.
 */
typedef SHR_BITDCLNAME(_mc_tbmp,_SIRIUS_MC_MAX_TARGETS);

#define _MC_TBMP_FORMAT "%01X%08X%08X%08X%08X"
#define _MC_TBMP_DISPLAY(tbmp) ((tbmp)[4]),((tbmp)[3]),((tbmp)[2]),((tbmp)[1]),((tbmp)[0])

/*
 *  We have to cache the GPORT and its respective OI, for every GPORT,OI tuple
 *  on every multicast group.  This is so the 'get' function works as expected,
 *  even with features such as implied aggregates.
 *
 *  The cache layout is linear; basically it's a straight copy of the data that
 *  were provided when adding individual GPORT,OI tuples, or when setting the
 *  lot of them for a group.  When adding, they're just tacked onto the end.
 *
 *  tbmp is the translated target bitmap for the element.
 *
 *  port is the original GPORT for the target.
 *
 *  encap_id is the original encap_id for the target.
 *
 *  extras is split into multiple fields.  The low byte is 0xFF to indicate
 *  that the original target was an aggregate, or some other value that is the
 *  original target ID (before the 'hidden higig aggregation' translation). The
 *  low-mid byte is the number of targets indicated by tbmp.  The high-mid byte
 *  contains higig aggregate information, and the high byte is somehow still
 *  reserved at this time.
 */
#define _MC_CACHE_EXTRA_GPORT_MASK 0xFF
#define _MC_CACHE_EXTRA_COUNT_MASK 0xFF
#define _MC_CACHE_EXTRA_COUNT_SHIFT 8
#define _MC_CACHE_EXTRA_HGA_MASK 0xFF
#define _MC_CACHE_EXTRA_HGA_SHIFT 16
#define _MC_CACHE_EXTRA_FORMAT "%06X"
typedef struct _gpoiCacheData_s {
    _mc_tbmp tbmp;
    bcm_gport_t port;
    bcm_if_t encap_id;
    bcm_gport_t queue_id;
    int extras;
} _gpoiCacheData_t;
typedef struct _gpoiCachePtr_s {
    unsigned int elements;
    _gpoiCacheData_t *data;
} _gpoiCachePtr_t;

/*
 *  This describes the internal intermediate view of a single element of an RR.
 *  There can be many of these (number of targets in MVR dictates number of
 *  elements in RR).
 *
 *  _MC_MAX_REPLICANTS is the largest number of replicants that is permitted on
 *  a single target.  The Sirius hardware docs do not make it clear whether a
 *  single target can have 4096 replicants; I choose to err on the side of
 *  caution and disallow more than 4095 replicants.
 *
 *  base is the OI base address (inside the OITT range if OI translation is to
 *  be used, or outside of it if not)
 *
 *  replicants is the number of replicants for this target
 *
 *  oittPtr points to an array containing the translated OI values if
 *  translation is enabled, or NULL if not
 *
 *  changed is TRUE if the OI data have changed, FALSE if not; this is used
 *  when writing to minimise commit churn
 *
 *  noTrans is TRUE if OI translation is disabled for this target, FALSE if OI
 *  translation is not disabled for this target
 */
#define _MC_MAX_REPLICANTS 4095
typedef struct _mc_rr_elem_internal_s {
    uint32 base;                                 /* OI base */
    uint32 replicants;                           /* number of replicants */
    uint32 *oittPtr;                             /* ptr to OITT buffer */
    uint32 *queuePtr;                            /* ptr to queue buffer */
    int changed;                                 /* OITT data changed */
    int noTrans;                                 /* OI translation disabled */
} _mc_rr_elem_internal_t;

/*
 *  This describes the internal intermediate view of an MVR.  It covers all of
 *  the data needed to build either BCM or hardware views of the MVR.
 *
 *  _MC_MAX_SPARSE is number of targets that can go into a sparse MVR.
 *
 *  _MC_SPARSE_MDB_MVR is largest possible sparse MVR, in MDB elements.
 *
 *  _MC_DENSE_MDB_MVR is the size of a dense MVR, in MDB elements.  It is
 *  actually possible to configure the hardware to a different value, but that
 *  is not currently supported by this module.
 *
 *  _MC_INVALID_BASE is a 'base' value that is not allowed.
 *
 *  targets is the number of active targets in this MVR.
 *
 *  elems is the number of MDB elements taken by the MVR, for informational and
 *  diagnostic purposes only (it is filled in by the read function and ignored
 *  by the write function)
 *
 *  mvr is the base MDB address of the MVR; it is used for tracking whether an
 *  MVR existed before, among other things, and must not be changed between
 *  read and write (though both will update it)
 *
 *  logical is TRUE if this MVR uses logical replication.
 *
 *  sourceKnockout is TRUE if this MVR uses source knockout.
 *
 *  base if the OI base (logical is FALSE) or the base MDB address of the RR
 *  (logical is TRUE)
 *
 *  tsBitmap is the bitmap of the active targets (this is used for dense mode
 *  programming the sparse mode programming information is refreshed from here)
 *
 *  target[] is the sparse mode target list (this is used for sparse mode
 *  programming, but is generated from the dense mode information above)
 *
 *  ptr is a union of the child pointers:
 *    rr is the pointer to the RR descriptor (logical is TRUE) or ignored
 *    (logical is FALSE)
 *
 *    oitt is the pointer to the OITT descriptor (logical is FALSE) or ignored
 *    (logical is TRUE)
 *
 *  ptrChanged is TRUE on writing to indicate something in the child has
 *  changed and the MVR will need to be replaced to point to the new child
 *
 *  changed is TRUE on writing to indicate that something in the MVR has
 *  changed and the MVR needs to be replaced
 *
 *  noTrans is TRUE for PHYSICAL mode if the OITT is disabled for the group.
 *  It is never TRUE for LOGICAL mode (there is an equivalent field for each RR
 *  element in LOGICAL mode that is used instead).
 */
#define _MC_MAX_SPARSE 13
#define _MC_INVALID_BASE 0x0007FFFF
#define _MC_UNICAST_MVR_MASK 0x0003FF00
typedef struct _mc_mvr_internal_s {
    unsigned int targets;                        /* active targets */
    unsigned int elems;                          /* MDB elems (only read) */
    uint32 mvr;                                  /* actual MVR address */
    int logical;                                 /* logical mode */
    int sourceKnockout;                          /* source knockout enable */
    uint32 base;                                 /* OI or RR base */
    _mc_tbmp tsBitmap;                           /* dense target bitmap */
    uint8 target[_MC_MAX_SPARSE];                /* sparse target list */
    union {
        _mc_rr_elem_internal_t *rr;              /* ptr to RR buff (LOG) */
        uint32 *oitt;                            /* ptr to OITT buff (PHY) */
    } ptr;
    uint32 *queuePtr;                            /* ptr to queue buff (PHY) */
    int ptrChanged;                              /* ptr tgt data changed */
    int changed;                                 /* MVR data changed */
    int noTrans;                                 /* OI translation disabled */
} _mc_mvr_internal_t;

/*
 *  Some hardware constants...
 *
 *  _MC_UNICAST_BASE is the offset into MDB space that begins the 'unicast'
 *  sysport space.  This space is used for implied replication of unicast such
 *  as when doing the oversubscribed subaggregate distribution groups.
 *
 *  _MC_UNICAST_SIZE is the number of MDB elements that exist for 'unicast'.
 */
#define _MC_UNICAST_BASE 65536
#define _MC_UNICAST_SIZE 4096

/*
 *  Number of lists used for tracking MDB and OITT blocks in limbo.  For
 *  sanity, this should be at least one greater than the number of seconds in
 *  the frame age time; for resource reclaiming reasons, it should be at most
 *  one greater than the number of seconds in the frame age time.
 *
 *  Once a resource is put on a limbo list, it will be at least this number
 *  minus one seconds before it is actually freed for reuse (this, and the
 *  limbo list advancement, is done by the background thread for all locally
 *  attached units).
 *
 *  Note that resources are not overwritten when they are placed in limbo, nor
 *  are they overwritten when eventually freed thence -- they are only
 *  overwritten when finally reallocated for future use.  This means that all
 *  resources that do not appear to be allocated during a warm restart must be
 *  considered to be in limbo (the alternative to waiting this many seconds
 *  after warm restart before we can do any updates is to overwrite things as
 *  they fall out of limbo and become freed, which will significantly increase
 *  processor loading by the background thread).
 */
#define _MC_LIMBO_LISTS 9

/*
 *  This is the maximum number of free lists this module will use for the MDB
 *  and OITT resource management.  Ideally, this would be exactly the number of
 *  free lists, but it is not that important because this only affects stack
 *  use during initialisation (the actual number of free lists used by the
 *  MDB and OITT resource management affects the memory use during run, rather
 *  than this maximum).  Those must be adjusted in the init sequence code.
 */
#define _MC_MAX_FREELISTS 16

/*
 *  When reloading, we should place all of the free MDB and OITT blocks into
 *  limbo rather than leaving them free.  This will ensure that the we do not
 *  overwrite data that are still being used by frames that are in buffers.
 *
 *  If, for some reason (maybe we're guaranteed enough delay after the reload
 *  that the old frames have been sent or aged out of the buffers), we don't
 *  want to wait for these blocks to fall out of limbo back onto the free
 *  lists, we can disable this by setting _MC_LIMBO_FREE_ON_RELOAD to FALSE.
 */
#define _MC_LIMBO_FREE_ON_RELOAD FALSE

/*
 *  Some backgrounder configuration
 *
 *  Right now, both of these are waits in whatever units are actually used by
 *  sal_sem_take, probably microseconds (but not most OSes or platforms have
 *  peculiar granularities applied to these delays, such as x86/32 OSes often
 *  using roughly 976.5 microsecond grains (actually, it's based on 1024Hz)),
 *  though some x86/32 OSes (vxWorks comes to mind) stull prefer to use the old
 *  DOS timer frequency of 65536/hr (just over 18.2Hz).
 *
 *  If the global lock can not be taken in the wait timeframe, the background
 *  thread will go back to sleep and try again next time it wakes.
 *
 *  If a specific unit lock can not be taken in the wait timeframe, the
 *  background thread will skip that unit (but will try any higher-numbered
 *  units) and try that unit again in sequence the next time it wakes.
 *
 *  NOTE: in a system where there is exactly one Sirius device per line card,
 *  and each line card is locally controlled by a separate processor, both of
 *  these values can be set to sal_sem_FOREVER, since the background task will
 *  not be neglecting other units in such conditions.  This might also be an
 *  acceptable setting on a system where there is high enough activity to have
 *  a high value for 'background skip *' in the diagnostic output.
 */
#define _MC_BACKGROUND_GLOBAL_LOCK_WAIT 100000
#define _MC_BACKGROUND_UNIT_LOCK_WAIT 50000

/*
 *  Some defragmentation tweaks...
 *
 *  _MC_DEFRAG_MAX_ATTEMPT is the number of groups that will be checked during
 *  a single defrag step, whether any of them are successfully shifted or not.
 *
 *  _MC_DEFRAG_MAX_TOUCH is the maximum number of times groups that will be
 *  entirely read or written during a single defrag step.  This number must be
 *  at least twice _MC_DEFRAG_MAX_MOVE, as both read and write are considered
 *  touch.  If configured properly, this should only come into play if there is
 *  some kind of error that prevents groups from being moved.
 *
 *  _MC_DEFRAG_MAX_MOVE is the maximum number of groups that will be actually
 *  moved during a single defrag step.
 *
 *  Since reading a group means a whole bunch of I/O, as does writing a group,
 *  but checking whether a group exists does not, it seems reasonable to weight
 *  this heavily in favour of only moving a few groups at a time, and giving up
 *  after a reasonable number of errors (thus, the attempt can be fairly large,
 *  touch should be twice move, and move should be reasonably smallish).  This
 *  limits the amount of work that will be done per second by the defragger
 *  (which is called by the backgroudn thread, which itself wakes up once per
 *  second).
 */
#define _MC_DEFRAG_MAX_ATTEMPT 4096
#define _MC_DEFRAG_MAX_TOUCH 64
#define _MC_DEFRAG_MAX_MOVE 16

/*
 *  Unit internal flags.
 *
 *  _MC_UNIT_DEFRAG_START is set either by the background process when it
 *  decides defragmentation may be a good idea, or by the user process when
 *  there is a condition that triggers BCM_E_RESOURCE (such as trying to
 *  allocate a block that is larger than the largest available).  The
 *  background process polls this each iteration after the aged list purge, and
 *  will begin the defragmentation process if it is set.
 *
 *  _MC_UNIT_DEFRAG_HALT is currently never set, but the defragmentation
 *  process polls this each iteration and will terminate (and lose its present
 *  state) if this is set.
 *
 *  _MC_UNIT_DEFRAG_RUNNING indicates the defrag process is active (that there
 *  is a defragmentation state and that the 'start' bit will be ignored).  This
 *  bit must not be set except by the defragmentation code.
 *
 *  _MC_UNIT_DEFRAG_UP indicates the defrag process is in its second stage
 *  (first stage is moving all groups to the bottom of MDB and OITT, second
 *  stage is moving all groups to the top of MDB and OITT).  This bit must
 *  not be set except by the defragmentation code.
 *
 *  _MC_UNIT_DEFRAG_ERROR indicates that the group at which the defragger last
 *  stopped caused it to stop due to an error trying to manipulate that group.
 *  On the next defrag step, it will start with the troublesome group, and just
 *  skip it if it still fails.
 *
 *  _MC_UNIT_DEFRAG_BY_* indicates the source of the latest defragmentation
 *  request, except for ...MASK, which masks the bits in the field.  The bits
 *  under the mask should never be zero if there is a defragmentation request,
 *  but this case is treated as if triggered by heuristic.
 *
 *  _MC_UNIT_LIMBO_PEAK_INH indicates that the limbo peak value updates are
 *  inhibited currently.  This only happens during startup if the value for
 *  _MC_LIMBO_FREE_ON_RELOAD is TRUE.  For the first pass through all of the
 *  limbo lists in such configuration, there will be a bogus number of elements
 *  in limbo: since the old state for non-used elements is not known, they are
 *  assumed to be in limbo, so all unused elements will be in limbo initially.
 *  Once the first pass through the limbo lists has finished, the number of
 *  elements in limbo will accurately reflect disuse during the session, and
 *  this flag will clear, allowing updates to the peak values.
 *
 *  _MC_UNIT_DUAL_LOOKUP indicates the unit is in 'dual-lookup' mode.  In this
 *  mode, it splits its part of the ep_oi2qb_map memory into halves (roughly --
 *  if there is any remainder, it is not used), and tracks two data for each
 *  multicast replicant.  The nominal API names for these data are encapId
 *  (which it tracks in 'single-lookup' mode, and queueId, which is additional
 *  to 'dual-lookup' mode, though the application can use them as it requires.
 *  Note that in dual-lookup mode, the maximum number of replicants is
 *  effectively halved, since we have to manage both sides of the split in
 *  parallel (both lookups use replicant ID as key) and have no way of knowing
 *  which replicant will need which lookup or both lookups.  The original
 *  intent of this mode is to do requeue cases, where multicast must provide
 *  both an encapId to the front panel device and a requeueId to the local
 *  Sirius ingress for an additional policing/shaping pass.
 *
 *  _MC_UNIT_NO_OITT indicates the unit is running in non-OITT mode (OI
 *  translation space is not allocated).  This mode should only be changed when
 *  there are no MC groups (normal BCM multicast APIs will not track OI when
 *  this is set, but there can be more than 64K replicants in this state; with
 *  it clear, normal multicast APIs will track OIs and manage OI translation,
 *  so there can only be 64K total replicants including multicast and
 *  aggregation replicants).  In addition, performance (especially for groups
 *  with large replicant count) is improved by running in non-OITT mode.
 *  Gratuitous changes to this setting can result in groups that have OITT
 *  entries associated with some replicants but not others.
 */
typedef enum _mc_unit_flags_e {
    _MC_UNIT_DEFRAG_START     = 0x00000001,      /* start defrag next tic */
    _MC_UNIT_DEFRAG_HALT      = 0x00000002,      /* halt defrag next tic */
    _MC_UNIT_DEFRAG_RUNNING   = 0x00000004,      /* defragmentation running */
    _MC_UNIT_DEFRAG_UP        = 0x00000008,      /* defrag toward top */
    _MC_UNIT_DEFRAG_ERROR     = 0x00000010,      /* defrag hit an error */
    _MC_UNIT_DEFRAG_BY_HEUR   = 0x00000020,      /* defrag due to heuristic */
    _MC_UNIT_DEFRAG_BY_RES    = 0x00000040,      /* defrag due to no resource */
    _MC_UNIT_DEFRAG_BY_USER   = 0x00000060,      /* defrag due to user */
    _MC_UNIT_DEFRAG_BY_MASK   = 0x00000060,      /* defrag cause mask */
    _MC_UNIT_LIMBO_PEAK_INH   = 0x20000000,      /* limbo peak inhibit */
    _MC_UNIT_DUAL_LOOKUP      = 0x40000000,      /* dual lookup configuration */
    _MC_UNIT_NO_OITT          = 0x80000000       /* disable OITT use on unit */
} _mc_unit_flags_t;

/*
 *  Unit local information structure.  This keeps track of various data that
 *  are necessary for the working state.
 *
 *  unit (should be pretty clear) is the unit number.
 *
 *  lock was the unit lock, but the unit lock needed to be shared between this
 *  and the aggregate (er, 'trunk') APIs, so it was moved to the unit globals
 *  in the SOC configuration structure.
 *
 *  flags is described above (_mc_unit_flags_t).
 *
 *  bgSkipGlob is the number of times the background thread has failed to
 *  service any units, as of the last time it successfully serviced this unit.
 *
 *  bgSkipUnit is the number of times the background thread has missed this
 *  unit so far due to it being busy (or, more specifically, the number of
 *  times the background thread had missed it up to the last time that it was
 *  not busy and the background thread serviced it).
 *
 *  cells is the number of additional 'persistent dynamic' memory cells
 *  currently held.  This refers to alloc requests that are persistent across
 *  calls but are allocated and freed dynamically during runtime, as opposed to
 *  the base amount that is set up during initialisation, or the working
 *  buffers that are allocated and freed during a single operation.
 *
 *  defragPosition is the defrag function's current position in the groups if
 *  defragmentation is running, or has all bits set otherwise.
 *
 *  purge is the user list ID in the mdbList and oittList that will have its
 *  members freed during the next timer interval.
 *
 *  limbo is the user list ID in the mdbList and oittList that will have blocks
 *  that are being disused added to it.
 *
 *  gmtGroupMax is the highest 'group ID' that counts as multicast (above this
 *  point is considered as belonging to aggregates instead of multicast) and so
 *  will not be verified against multicast internal structures (caller must
 *  ensure it is valid).  gmtGroupMin is the lowest.
 *
 *  oittMax is the highest OITT entry that multicast will use for encapId, and
 *  oittMin is the lowest.  The same physical memory is shared with other
 *  functionality; also, replicants with OITT disabled will use OI values
 *  outside of this range.
 *
 *  queueMax is the highest OITT entry that will be used for queueId, and
 *  queueMin is the lowest.  The same physical memory is shared with other
 *  functionality.  Replicants with OITT disabled will implicitly have this
 *  lookup disabled as well.  This is part of the dual lookup functionality,
 *  and can be disabled at startup.
 *
 *  denseMvrSize indicates the number of MDB elements that the hardware is
 *  configured to use for dense MVRs, and is also implicitly a limiting factor
 *  on the number of elements the software will use for sparse MVRs (but note
 *  that sparse MVRs are also hardware limited to 4 or fewer elements).
 *
 *  maxTargets is the number of targets that can be achieved with the setting
 *  for denseMvrSize.  Targets numbered this value or higher can not be used.
 *
 *  groupCount and groupCountMax are used for limiting groups on devices where
 *  there is a limit to the number of active groups.
 *
 *  mdbBlocks and oittBlocks indicate the number of active MDB and OITT blocks,
 *  respecively.  This particular information is not tracked by the mdb list
 *  handler, so we track it here instead.
 *
 *  limboMdbMax and limboMdbMaxBlk indicate the number of MDB elements and
 *  blocks (respectively) that were in limbo at the time when the largest
 *  number of MDB elements was in limbo.
 *
 *  limboOittMax and limboOittMaxBlk indicate the number of OITT elements and
 *  blocks (respectively) that were in limbo at the time when the largest
 *  number of OITT elements was in limbo.
 *
 *  defragHeur, defragRes, defratUser indicate the number of attempts to invoke
 *  the defragmentation code that occurred due to heuristic analysis of
 *  available resources, due to a resource shortage, or due to user request,
 *  respectively.  These numbers may be higher than the actual number of times
 *  the defrag code ran, since the defrag code ignores requests to begin if it
 *  is already running.
 *
 *  gmtList is the free list used for tracking GMT space (these are multicast
 *  object IDs to this module).  Note that it only covers the valid GMT space
 *  that is below or equal to gmtGroupMax; the rest of the space (if any) is
 *  assumed to belong to other modules.
 *
 *  mdbList is the set of lists, free and user, that is used for tracking
 *  available, used, free, and pending free space in the MDB.  These are
 *  strictly an internal resource.
 *
 *  oittList is the free list used for tracking OHI translation table space,
 *  which is used to map the internal OHID to an actual OHIndex as expected by
 *  the downstream device, due to limitations in this chip's mapping method.
 *
 *  cache points to an array, each element of which corresponds to an active
 *  multicast group or unicast distribution group, and points to a cached copy
 *  of the contents of that group.
 *
 *  unicast points to a bitmap that indicates which unicast 'sysports' contain
 *  unicast distribution groups instead of pure unicast targets.
 */
#define _MC_MAX_HIGIG_AGGREGATES SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS
typedef struct _mc_unit_s {
    int unit;                                    /* unit ID */
    _mc_unit_flags_t flags;                      /* unit flags */
    unsigned int bgSkipGlob;                     /* global background skip */
    unsigned int bgSkipUnit;                     /* unit background skip */
    unsigned int cells;                          /* dynamic memory cells */
    uint32 defragPosition;                       /* group being moved */
    unsigned int purge;                          /* free from this list */
    unsigned int limbo;                          /* 'free' to this list */
    unsigned int gmtGroupMin;                    /* min GMT entry we use */
    unsigned int gmtGroupMax;                    /* max GMT for groups */
    unsigned int oittMin;                        /* min OITT entry we use */
    unsigned int oittMax;                        /* max OITT entry we use */
    unsigned int queueMin;                       /* min queue entry we use */
    unsigned int queueMax;                       /* max queue entry we use */
    unsigned int denseMvrSize;                   /* MDB elems in dense MVR */
    unsigned int maxTargets;                     /* dense MVR limits tgt cnt */
    unsigned int groupCount;                     /* active group count */
    unsigned int groupCountMax;                  /* maximum group count */
    unsigned int mdbBlocks;                      /* current MDB used blocks */
    unsigned int oittBlocks;                     /* current OITT used blocks */
    unsigned int limboMdbMax;                    /* peak MDB limbo elems */
    unsigned int limboMdbMaxBlk;                 /* peak MDB limbo blocks */
    unsigned int limboOittMax;                   /* peak OITT limbo elems */
    unsigned int limboOittMaxBlk;                /* peak OITT limbo blocks */
    unsigned int defragAttHeur;                  /* defrag tries: heuristic */
    unsigned int defragAttRes;                   /* defrag tries: no resource */
    unsigned int defragAttUser;                  /* defrag tries: user */
    unsigned int defragHeur;                     /* defrag count: heuristic */
    unsigned int defragRes;                      /* defrag count: no resource */
    unsigned int defragUser;                     /* defrag count: user */
    shr_idxres_list_handle_t gmtList;            /* MVR management lists */
    shr_mdb_list_handle_t mdbList;               /* MDB management lists */
    shr_mdb_list_handle_t oittList;              /* OITT management lists */
    _gpoiCachePtr_t *cache;                      /* pointer to GP/OI cache */
    SHR_BITDCL *unicast;                         /* unicast space in use */
} _mc_unit_t;
#define _MC_OITT_HIGH_SHIFT 18
#define _MC_OITT_HW_MASK ((1 << _MC_OITT_HIGH_SHIFT) - 1)

/*
 *  Operating modes for _bcm_sirius_multicast_group_flush
 */
typedef enum _mc_group_flush_mode_e {
    _MC_GROUP_FLUSH_KEEP = 0,                    /* keep group */
    _MC_GROUP_FLUSH_DESTROY,                     /* destroy group */
    _MC_GROUP_FLUSH_REPLACE                      /* replace content with new */
} _mc_group_flush_mode_t;

/*
 *  Pointers to each possible unit's local information
 */
static _mc_unit_t *_mc_unit[SOC_MAX_NUM_DEVICES];

/*
 *  Backgrounded processing thread ID (this thread handles the deferred free
 *  and defragmentation processes).
 *
 *  Note that this is a *single* thread for all locally connected units.  Under
 *  normal conditions, it is anticipated that: such a thread would not normally
 *  need to run for long periods per unit; having more than one such thread
 *  would cause gratuitous resource collisions; having more than one such
 *  thread would waste threading resources and thread switch time.
 */
static volatile sal_thread_t _mc_background = NULL;

/*
 *  Global lock for use when creating/destroying units (or when the background
 *  process needs to ensure units don't appear/disapper while it is running).
 *  The unit locks are sufficient when only manipulating an existing unit.
 *
 *  Note that having this locked does not prevent manipulation of units; it
 *  only prevents their initialisation or detachment.  A unit's lock must also
 *  be taken if that unit is to be manipulated (look at the background thread
 *  code to see how this works).  It is therefore perfectly reasonable for the
 *  background thread to hold this for the relatively prolonged time it takes
 *  to handle background tasks on several units, as holding this alone will not
 *  interfere with the normal operation of any unit.
 */
static volatile sal_mutex_t _mc_lock = NULL;

/*
 *  Some basic macros...
 */
#define MC_UNIT_CHECK(__unit) \
    if (SOC_MAX_NUM_DEVICES <= (__unit)) { \
        MC_ERR((MC_MSG1("invalid unit ID %d\n"), __unit)); \
        return BCM_E_UNIT; \
    }
#define MC_INIT_CHECK(__unit, __dataPtr) \
    (__dataPtr) = _mc_unit[unit]; \
    if ((!_mc_lock) || \
        (!(__dataPtr)) || \
        (!SOC_CONTROL(unit)) || \
        (!SOC_SBX_CONTROL(unit)) || \
        (!SOC_SBX_CFG(unit)) || \
        (!SOC_SBX_CFG_SIRIUS(unit)) || \
        (!(SOC_SBX_CFG_SIRIUS(unit)->lMcAggrLock))) { \
        MC_ERR((MC_MSG1("unit %d is not initialised\n"), __unit)); \
        return BCM_E_INIT; \
    }
#define MC_LOCK_TAKE(__dataPtr) \
    if (sal_mutex_take(SOC_SBX_CFG_SIRIUS((__dataPtr)->unit)->lMcAggrLock, \
                       sal_mutex_FOREVER)) { \
        MC_ERR((MC_MSG1("unable to take unit %d lock\n"), \
                (__dataPtr)->unit)); \
        return BCM_E_INTERNAL; \
    }
#define MC_LOCK_GIVE(__dataPtr) \
    if (sal_mutex_give(SOC_SBX_CFG_SIRIUS((__dataPtr)->unit)->lMcAggrLock)) { \
        MC_ERR((MC_MSG1("unable to release unit %d lock\n"), \
                (__dataPtr)->unit)); \
        return BCM_E_INTERNAL; \
    }

/*****************************************************************************
 *
 *  Internal accounting and management (except initialisation)
 */

/*
 *  Function
 *    _bcm_sirius_multicast_data_destroy
 *  Purpose
 *    Destroy the unit multicast management information.  This does not
 *    initialise the hardware or write the hardware.
 *  Arguments
 *    (in) int unit = the unit to prepare
 *    (out) _mc_unit_t **unitData = where to put the unit data pointer
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    none
 */
static int
_bcm_sirius_multicast_data_destroy(_mc_unit_t *unitData)
{
    int result = BCM_E_NONE;
    int tmpRes;
    unsigned int index;

    if (!unitData) {
        /* there's nothing to destroy -- that was easy! */
        return result;
    }
    MC_VERB((MC_MSG("(%08X) enter\n"), (uint32)unitData));

    /* get rid of GMT allocator */
    tmpRes = shr_idxres_list_destroy(unitData->gmtList);
    if (BCM_E_NONE != tmpRes) {
        result = tmpRes;
        MC_ERR((MC_MSG1("unable to destroy unit %d gmtList: %d (%s)\n"),
                unitData->unit,
                result,
                _SHR_ERRMSG(result)));
    }

    /* get rid of MDB allocator */
    tmpRes = shr_mdb_destroy(unitData->mdbList);
    if (BCM_E_NONE != tmpRes) {
        result = tmpRes;
        MC_ERR((MC_MSG1("unable to destroy unit %d mdbList: %d (%s)\n"),
                unitData->unit,
                result,
                _SHR_ERRMSG(result)));
    }

    /* get rid of OITT allocator */
    tmpRes = shr_mdb_destroy(unitData->oittList);
    if (BCM_E_NONE != tmpRes) {
        result = tmpRes;
        MC_ERR((MC_MSG1("unable to destroy unit %d oittList: %d (%s)\n"),
                unitData->unit,
                result,
                _SHR_ERRMSG(result)));
    }

    /* get rid of group caches */
    for (index = SOC_MEM_INFO(unitData->unit, EG_FD_GMTm).index_min;
         index < SOC_MEM_INFO(unitData->unit, EG_FD_GMTm).index_max;
         index++) {
        if (unitData->cache[index - (SOC_MEM_INFO(unitData->unit, EG_FD_GMTm).index_min)].data) {
            sal_free(unitData->cache[index - (SOC_MEM_INFO(unitData->unit, EG_FD_GMTm).index_min)].data);
            unitData->cells--;
        }
    }
    if (unitData->cells) {
        /* but there should not be any persistent cells left now! */
        MC_ERR((MC_MSG1("unit %d still had %d persistent heap cells after"
                        " destruction; these cells have been leaked\n"),
                unitData->unit,
                unitData->cells));
        result = BCM_E_INTERNAL;
    }

    MC_VERB((MC_MSG("(%08X) return %d (%s)\n"),
             (uint32)unitData,
             result,
             _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    _bcm_sirius_multicast_group_check
 *  Purpose
 *    Verify that a group is valid and exists.
 *  Arguments
 *    (in) _mc_unit_t *unitData = pointer to unit information
 *    (in) bcm_multicast_t group = group to check
 *  Return
 *    BCM_E_NONE if group exists and is valid
 *    BCM_E_NOT_FOUND if group does not exist or is not valid
 *  Notes
 *    none
 */
static int
_bcm_sirius_multicast_group_check(_mc_unit_t *unitData,
                                  bcm_multicast_t group)
{
    int result;

    if ((group >= unitData->gmtGroupMin) &&
        (group <= unitData->gmtGroupMax)) {
        /* group is in multicast space */
        /* make sure the group exists */
        result = shr_idxres_list_elem_state(unitData->gmtList, group);
        switch (result) {
        case BCM_E_EXISTS:
            /* we want it to exist, so that's not an error */
            result = BCM_E_NONE;
            break;
        case BCM_E_PARAM:
            /* for some reason, BCM API thinks bogus ID should be NOT_FOUND */
            result = BCM_E_NOT_FOUND;
            MC_ERR((MC_MSG1("multicast group %08X invalid on unit %d\n"),
                    group,
                    unitData->unit));
            break;
        case BCM_E_NONE:
            /* this should never happen */
            result = BCM_E_INTERNAL;
            MC_ERR((MC_MSG1("unexpected result checking unit %d group %08X\n"),
                    unitData->unit,
                    group));
            break;
        default:
            /* something else went wrong */
            MC_ERR((MC_MSG1("unable to access unit %d group %08X:"
                            " %d (%s)\n"),
                    unitData->unit,
                    group,
                    result,
                    _SHR_ERRMSG(result)));
        } /* switch (result) */
    } else if ((group > unitData->gmtGroupMax) &&
               (group <= SOC_MEM_INFO(unitData->unit, EG_FD_GMTm).index_max)) {
        /* 'group' is in unicast/aggregate space */
        /* this should only occur for unicast aggregation distrib groups */
        if (SHR_BITGET(unitData->unicast,
                       (group - (unitData->gmtGroupMax + 1)))) {
            result = BCM_E_NONE;
        } else {
            result = BCM_E_NOT_FOUND;
        }
    } else {
        /* 'group' has completely bogus ID */
        MC_ERR((MC_MSG1("invalid group ID %08X\n"), group));
        result = BCM_E_PARAM;
    }
    return result;
}

/*
 *  Function
 *    _bcm_sirius_multicast_gport_translate_bitmap
 *  Purpose
 *    Translate the provided GPORT into a local target bitmap
 *  Arguments
 *    (in) _mc_unit_t *unitData = unit data pointer
 *    (in) bcm_module_t myModule = unit's module ID
 *    (in) bcm_gport_t gport = GPORT
 *    (in) int allowXgs = TRUE if XGS mode parsing is allowed
 *    (out) _mc_tbmp *targets = where to put local target bitmap
 *    (out) unsigned int *tgtCnt = where to put actual local target count
 *    (out) int *target = where to put original target ID
 *    (out) unsigned int *hga = where to put higig aggregate ID
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    This is where we translate a GPORT into the bitmap that describes its
 *    targets.  Once this is done, we only use the targets internally (even
 *    caching the targets with the GPORT,OI data for table consistency).
 *
 *    For 'child' and 'egress child' GPORTs, this maps them directly to their
 *    associated target (using bcm_sirius_gport_fifo_get).
 *
 *    For 'modport' and 'egress modport' GPORTs, this maps them directly to the
 *    associated higig internal multicast ports.
 *
 *    For the case of oversubscribed mode, unicast distribution members are
 *    automatically added as aggregates if any of the aggregate members is
 *    added.  This is only done because everything else about these things is
 *    handled automatically.  In all other cases, all members of a given
 *    aggregate must be added explicitly by the application.
 *
 *    Non-local targets are not accepted.  Unlike the aggregate case, multicast
 *    does not need to be aware of membership on other units.  It is simpler to
 *    keep it unaware of such by not accepting remote membership.
 */
static int
_bcm_sirius_multicast_gport_translate_bitmap(_mc_unit_t *unitData,
                                             bcm_module_t myModule,
                                             bcm_gport_t gport,
                                             int allowXgs,
                                             _mc_tbmp *targets,
                                             unsigned int *tgtCnt,
                                             int *target,
                                             unsigned int *hga)
{
    bcm_module_t module = -1;               /* destination module ID */
    bcm_trunk_add_info_t trunkInfo;         /* higig aggregate information */
    bcm_trunk_t trunkId;                    /* higig aggregate ID */
    _mc_tbmp tgtWork;                       /* working bitmap */
    unsigned int hgaId = 0;                 /* which higig aggregate (0=none) */
    unsigned int tgtId = ~1;                /* working target ID */
    unsigned int tCount = 0;                /* working target count */
    unsigned int index;
    unsigned int count = 0;
    int result = BCM_E_NONE;

    MC_EVERB((MC_MSG("(%08X,%d,%08X,%s,*,*,*,*) enter\n"),
              (uint32)unitData,
              myModule,
              gport,
              allowXgs?"TRUE":"FALSE"));
    sal_memset(tgtWork, 0x00, sizeof(tgtWork));

    /*
     *  We don't support non-GPORT specifiers here.  It's because the multicast
     *  descriptors don't have an explicit 'module' -- only a GPORT and an
     *  encap ID.  It's not safe to assume 'local' for this device.
     */
    if (0 == ((gport >> _SHR_GPORT_TYPE_SHIFT) & _SHR_GPORT_TYPE_MASK)) {
        MC_ERR((MC_MSG1("non-GPORT port %d is not supported as a member of a"
                        " multicast group by unit %d\n"),
                gport,
                unitData->unit));
        return BCM_E_PARAM;
    }

    /* Get basic information about the provided port */
    if (BCM_GPORT_IS_MODPORT(gport)) {
        module = BCM_GPORT_MODPORT_MODID_GET(gport);
    } else if (BCM_GPORT_IS_EGRESS_MODPORT(gport)) {
        module = BCM_GPORT_EGRESS_MODPORT_MODID_GET(gport);
    } else if (BCM_GPORT_IS_CHILD(gport)) {
        module = BCM_GPORT_CHILD_MODID_GET(gport);
    } else if (BCM_GPORT_IS_EGRESS_CHILD(gport)) {
        module = BCM_GPORT_EGRESS_CHILD_MODID_GET(gport);
    } else if (BCM_GPORT_IS_EGRESS_GROUP(gport)) {
        module = BCM_GPORT_EGRESS_GROUP_MODID_GET(gport);
    } else {
        MC_ERR((MC_MSG1("unexpected GPORT type %d\n"),
                (gport >> _SHR_GPORT_TYPE_SHIFT) & _SHR_GPORT_TYPE_MASK));
        return BCM_E_PARAM;
    }
    result = bcm_sirius_aggregate_gport_translate(unitData->unit,
                                                  BCM_SIRIUS_AGGREGATE_GPORT_TRANSLATE_INHIBIT_MODPORTMAP,
                                                  myModule,
                                                  module,
                                                  gport,
                                                  &tgtId,
                                                  &tCount,
                                                  NULL);
    if (BCM_E_NONE != result) {
        MC_ERR((MC_MSG1("unable get unit %d GPORT %08X target: %d (%s)\n"),
                unitData->unit,
                gport,
                result,
                _SHR_ERRMSG(result)));
        return result;
    }

    /* We don't support nonlocal multicast membership */
    if (module != myModule) {
        MC_ERR((MC_MSG1("unit %d does not support non-local GPORT %08X"
                        " in local multicast groups\n"),
                unitData->unit,
                gport));
        return BCM_E_PARAM;
    }

    /*
     *  There is a possibility of running with reduced dense MVR size.  In this
     *  mode, there is a limit on the number of targets that can be addressed
     *  via multicast membership.
     */
    if ((tgtId + tCount) >= unitData->maxTargets) {
        /* can't represent this one in a dense MVR, so invalid */
        MC_ERR((MC_MSG1("unit %d unable to represent target %d in dense mode"
                        " (max %d)\n"),
                unitData->unit,
                tgtId,
                unitData->maxTargets - 1));
        return BCM_E_CONFIG;
    }

    /* Include the specified targets */
    count = tCount;
    while (tCount > 0) {
        tCount--;
        SHR_BITSET(tgtWork, tgtId + tCount);
    }

    /*
     *  Now, we need to do aggregate management for the case of oversubscribed
     *  unicast distribution groups if the original GPORT is not of type
     *  MODPORT or EGRESS_MODPORT, and the target is involved in an aggregate,
     *  and that aggregate is a fabric aggregate.
     */
    
    if ((!BCM_GPORT_IS_MODPORT(gport)) &&
        (!BCM_GPORT_IS_EGRESS_MODPORT(gport))) {
        /* it's not MODPORT or EGRESS modport; check aggregate membership */
        result = bcm_sirius_trunk_find_and_get(unitData->unit,
                                               myModule,
                                               gport,
                                               &trunkId,
                                               &trunkInfo);
        switch (result) {
        case BCM_E_NONE:
            /* It't an aggregate member */
            if (trunkId >= _SIRIUS_FAB_LAG_MIN) {
                /* it's fabric; need to include other targets as replicants */
                for (index = 0; index < trunkInfo.num_ports; index++) {
                    result = bcm_sirius_aggregate_gport_translate(unitData->unit,
                                                                  BCM_SIRIUS_AGGREGATE_GPORT_TRANSLATE_INHIBIT_MODPORTMAP,
                                                                  myModule,
                                                                  trunkInfo.tm[index],
                                                                  trunkInfo.tp[index],
                                                                  &tgtId,
                                                                  &tCount,
                                                                  NULL);
                    if (BCM_E_NONE != result) {
                        MC_ERR((MC_MSG1("unable to get target data for unit %d"
                                        " aggregate %08X index %d module %d"
                                        " port %08X: %d (%s)\n"),
                                unitData->unit,
                                trunkId,
                                index,
                                trunkInfo.tm[index],
                                trunkInfo.tp[index],
                                result,
                                _SHR_ERRMSG(result)));
                        return result;
                    }
                    if (tgtId >= unitData->maxTargets) {
                        /* can't represent this one in dense MVR, so invalid */
                        MC_ERR((MC_MSG1("unit %d unable to represent target %d"
                                        " in dense mode (max %d)\n"),
                                unitData->unit,
                                tgtId,
                                unitData->maxTargets - 1));
                        return BCM_E_CONFIG;
                    }
                    if (!SHR_BITGET(tgtWork, tgtId)) {
                        count++;
                        SHR_BITSET(tgtWork, tgtId);
                    }
                } /* for (index = 0; index < trunkInfo.num_ports; index++) */
                hgaId = (trunkId - _SIRIUS_FAB_LAG_MIN) + 1;
                tgtId = _SIRIUS_MC_MAX_TARGETS;
            } /* if (trunkId >= _SIRIUS_FAB_LAG_MIN) */
            break;
        case BCM_E_INIT:
            /* Not initialised guarantees it's not an aggregate member. */
        case BCM_E_NOT_FOUND:
            /* Not an aggregate member; nothing more to do here. */
            result = BCM_E_NONE;
            break;
        default:
            MC_ERR((MC_MSG1("unexpected error checking unit %d gport"
                            " %08X aggregate membership: %d (%s)\n"),
                    unitData->unit,
                    gport,
                    result,
                    _SHR_ERRMSG(result)));
            return result;
        } /* switch (result) */
    } /* if (not MODPORT and not EGRESS_MODPORT) */
    

    /* return the information that was requested by the caller */
    if (BCM_E_NONE == result) {
        if (targets) {
            /* asked for the targets; return them */
            sal_memcpy(targets, &tgtWork, sizeof(*targets));
        }
        if (tgtCnt) {
            /* asked for target count; return it */
            *tgtCnt = count;
        }
        if (target) {
            /* asked for target ID; return it */
            *target = tgtId;
        }
        if (hga) {
            /* asked for higig aggregate ID; return it */
            *hga = hgaId;
        }
    }
    MC_EVERB((MC_MSG("(%08X,%d,%08X,%s,*,*,*,*) return %d (%s)\n"),
              (uint32)unitData,
              myModule,
              gport,
              allowXgs?"TRUE":"FALSE",
              result,
              _SHR_ERRMSG(result)));
    return result;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *  Hardware manipulation and associated descriptor work
 */

/*
 *  Function
 *    _bcm_sirius_multicast_hwinfo_dump
 *  Purpose
 *    Dump the contents of a hardware descriptor that has been read (or
 *    possibly edited since read, or maybe about to be written).
 *  Arguments
 *    (in) char *prefix = string to prefix to each line
 *    (in) _mc_mvr_internal_t *mvrPtr = poitner to MVR information
 *  Return
 *    (none)
 *  Notes
 *    Lock does not matter here since it uses the provided buffer rather than
 *    reading the hardware.
 */
static void
_bcm_sirius_multicast_hwinfo_dump(_mc_unit_t *unitData,
                                  const char *prefix,
                                  const _mc_mvr_internal_t *mvrPtr)
{
    unsigned int rrIndex;
    unsigned int oittIndex;
    unsigned int qsIndex;

    soc_cm_print("%sMVR base   =    %05X\n", prefix, mvrPtr->mvr);
    soc_cm_print("%s%s base%s =    %05X\n",
                 prefix,
                 (mvrPtr->logical)?"RR":(mvrPtr->noTrans)?"EI":"RepId",
                 (mvrPtr->logical)?"   ":(mvrPtr->noTrans)?"   ":"",
                 mvrPtr->base);
    soc_cm_print("%sTargets    = "
                 _MC_TBMP_FORMAT
                 "\n",
                 prefix,
                 _MC_TBMP_DISPLAY(mvrPtr->tsBitmap));
    if (mvrPtr->logical) {
        /* logical mode */
        rrIndex = 0;
        for (qsIndex = 0;
             qsIndex < _SIRIUS_MC_MAX_TARGETS;
             qsIndex++) {
            if (SHR_BITGET(mvrPtr->tsBitmap, qsIndex)) {
                /* this target is included */
                soc_cm_print("%s  target %3d: RR %05X ->"
                             " %s base %05X, %d replicant%s\n",
                             prefix,
                             qsIndex,
                             mvrPtr->base + rrIndex,
                             mvrPtr->ptr.rr[rrIndex].noTrans?"EI   ":"RepId",
                             mvrPtr->ptr.rr[rrIndex].base,
                             mvrPtr->ptr.rr[rrIndex].replicants,
                             (1 == mvrPtr->ptr.rr[rrIndex].replicants)?"":"s");
                if (!(mvrPtr->ptr.rr[rrIndex].noTrans)) {
                    for (oittIndex = 0;
                         oittIndex < mvrPtr->ptr.rr[rrIndex].replicants;
                         oittIndex++) {
                        if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
                            soc_cm_print("%s    RepId %05X -> EI %05X, queue %05X\n",
                                         prefix,
                                         mvrPtr->ptr.rr[rrIndex].base + oittIndex,
                                         mvrPtr->ptr.rr[rrIndex].oittPtr[oittIndex],
                                         mvrPtr->ptr.rr[rrIndex].queuePtr[oittIndex]);
                        } else {
                            soc_cm_print("%s    RepId %05X -> EI %05X\n",
                                         prefix,
                                         mvrPtr->ptr.rr[rrIndex].base + oittIndex,
                                         mvrPtr->ptr.rr[rrIndex].oittPtr[oittIndex]);
                        }
                    } /* for (each replicant) */
                }
                /* use next RR for next included target */
                rrIndex++;
            } /* if (SHR_BITGET(mvrData->tsBitmap, qsIndex)) */
        } /* for (qsIndex = 0; qsIndex < _MC_MAX_QSETS; qsIndex++) */
    } else { /* if (mvrPtr->logical) */
        /* physical mode */
        oittIndex = 0;
        for (qsIndex = 0;
             qsIndex < _SIRIUS_MC_MAX_TARGETS;
             qsIndex++) {
            if (SHR_BITGET(mvrPtr->tsBitmap, qsIndex)) {
                /* dump this target's information */
                if (mvrPtr->noTrans) {
                    soc_cm_print("%s  target %3d: EI %05X\n",
                                 prefix,
                                 qsIndex,
                                 mvrPtr->base + oittIndex);
                } else {
                    if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
                        soc_cm_print("%s  target %3d: RepId %05X -> EI %05X, queue %05X\n",
                                     prefix,
                                     qsIndex,
                                     mvrPtr->base + oittIndex,
                                     mvrPtr->ptr.oitt[oittIndex],
                                     mvrPtr->queuePtr[oittIndex]);
                    } else {
                        soc_cm_print("%s  target %3d: RepId %05X -> EI %05X\n",
                                     prefix,
                                     qsIndex,
                                     mvrPtr->base + oittIndex,
                                     mvrPtr->ptr.oitt[oittIndex]);
                    }
                }
                /* use next OI for next included target */
                oittIndex++;
            } /* if (SHR_BITGET(mvrData->tsBitmap, qsIndex)) */
        } /* for (qsIndex = 0; qsIndex < _MC_MAX_QSETS; qsIndex++) */
    } /* if (mvrPtr->logical) */
}

/*
 *  Function
 *    _bcm_sirius_multicast_mvrdata_free
 *  Purpose
 *    Free an _mc_mvr_internal_t heap cell and its associated heap cells.
 *  Arguments
 *    (in) _mc_mvr_internal_t *mvrData = pointer to MVR data
 *  Return
 *    none
 *  Notes
 *    The pointer must not be used after return from this function.
 */
static void
_bcm_sirius_multicast_mvrdata_free(_mc_mvr_internal_t *mvrData)
{
    unsigned int index;

    if (mvrData) {
        if (mvrData->logical) {
            /* logical mode */
            if (mvrData->ptr.rr) {
                for (index = 0; index < mvrData->targets; index++) {
                    if (mvrData->ptr.rr[index].oittPtr) {
                        sal_free(mvrData->ptr.rr[index].oittPtr);
                        mvrData->ptr.rr[index].oittPtr = NULL;
                    }
                    if (mvrData->ptr.rr[index].queuePtr) {
                        sal_free(mvrData->ptr.rr[index].queuePtr);
                        mvrData->ptr.rr[index].queuePtr = NULL;
                    }
                }
                sal_free(mvrData->ptr.rr);
                mvrData->ptr.rr = NULL;
            }
        } else {
            /* physical mode */
            if (mvrData->ptr.oitt) {
                sal_free(mvrData->ptr.oitt);
                mvrData->ptr.oitt = NULL;
            }
            if (mvrData->queuePtr) {
                sal_free(mvrData->queuePtr);
                mvrData->queuePtr = NULL;
            }
        }
        sal_free(mvrData);
    }
}

/*
 *  Function
 *    _bcm_sirius_multicast_sparse_update
 *  Purpose
 *    Make sure the target count and sparse target list are both in agreement
 *    with the target bitmap.
 *  Arguments
 *    (in/out) _mc_mvr_internal_t *mvrData = pointer to MVR data
 *  Return
 *    none
 *  Notes
 *    Adjusts both target count and sparse target list.
 */
static void
_bcm_sirius_multicast_sparse_update(_mc_mvr_internal_t *mvrData)
{
    unsigned int tsIndex;

    mvrData->targets = 0;
    for (tsIndex = 0; tsIndex < _SIRIUS_MC_MAX_TARGETS; tsIndex++) {
        if (SHR_BITGET(mvrData->tsBitmap, tsIndex)) {
            MC_EVERB((MC_MSG1("mvrData %08X: include target %08X\n"),
                      (uint32)mvrData,
                      tsIndex));
            if (mvrData->targets < _MC_MAX_SPARSE) {
                mvrData->target[mvrData->targets] = tsIndex;
            }
            mvrData->targets++;
        } /* if (SHR_BITGET(mvrData->tsBitmap, qsIndex)) */
    } /* for (qsIndex = 0; qsIndex < _MC_MAX_QSETS; qsIndex++) */
    MC_EVERB((MC_MSG1("mvrData %08X: %d targets included\n"),
              (uint32)mvrData,
              mvrData->targets));
}

/*
 *  Function
 *    _bcm_sirius_multicast_mvr_read
 *  Purpose
 *    Read and parse an MVR.
 *  Arguments
 *    (in) _mc_unit_t *unitData = unit data pointer
 *    (in) uint32 mvrAddr = MVR address in MDB
 *    (out) _mc_mvr_internal_t *mvrParsed = where to put parsed MVR data
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Assumes all arguments are valid.
 *    Some parts of mvrParsed may be updated even if error.
 */
static int
_bcm_sirius_multicast_mvr_read(_mc_unit_t *unitData,
                               uint32 mvrAddr,
                               _mc_mvr_internal_t *mvrParsed)
{
    eg_fd_mdb_entry_t mvrTemp;
    eg_fd_mdb_entry_t *mvrRaw;
    uint32 mvrData0;
    uint32 mvrData1;
    uint32 mvrData2;
    uint32 mvrData3;
    uint32 mvrData4;
    unsigned int count;
    unsigned int offset;
    unsigned int index;
    int result;

    MC_VVERB((MC_MSG("(%08X,%08X,*) enter\n"),
              (uint32)unitData,
              mvrAddr));

    /* verify addresses */
    if ((mvrAddr > SOC_MEM_INFO(unitData->unit, EG_FD_MDBm).index_max) ||
        (mvrAddr <= SOC_MEM_INFO(unitData->unit, EG_FD_MDBm).index_min)) {
        /* yes, <= was intended -- zeroth element indicates disabled group */
        MC_ERR((MC_MSG1("unable to read %d:mdb[%08X]:"
                        " starts out of bounds\n"),
                unitData->unit,
                mvrAddr));
        return BCM_E_PARAM;
    }
    /* get the initial element of the MVR */
    result = soc_mem_read(unitData->unit,
                          EG_FD_MDBm,
                          MEM_BLOCK_ANY,
                          mvrAddr,
                          &mvrTemp);
    if (BCM_E_NONE != result) {
        MC_ERR((MC_MSG1("unable to read %d:mdb[%08X]: %d (%s)\n"),
                unitData->unit,
                mvrAddr,
                result,
                _SHR_ERRMSG(result)));
        return result;
    }
    /* parse the initial element of the MVR */
    mvrData0 = soc_mem_field32_get(unitData->unit,
                                   EG_FD_MDBm,
                                   (uint32*)(&mvrTemp),
                                   MVR_ENTRYf);
    mvrParsed->targets = 0;
    mvrParsed->logical = !(!(mvrData0 & 0x00000002));
    mvrParsed->sourceKnockout = !(!(mvrData0 & 0x00000004));
    mvrParsed->mvr = mvrAddr;
    sal_memset(&(mvrParsed->target), 0xFF, sizeof(mvrParsed->target));
    sal_memset(&(mvrParsed->tsBitmap), 0x00, sizeof(mvrParsed->tsBitmap));
    if (mvrData0 & 0x00000001) {
        /* sparse mode */
        mvrParsed->base = (mvrData0 >> 3) & 0x7FFFF;
        count = (mvrData0 >> 22) & 0x3;
        MC_EVERB((MC_MSG1("MVR is sparse mode (%d elements)\n"), count + 1));
        mvrParsed->target[0] = (mvrData0 >> 24) & 0xFF;
        if (mvrParsed->target[0] != 0xFF) {
            SHR_BITSET(mvrParsed->tsBitmap, mvrParsed->target[0]);
            mvrParsed->targets++;
        }
    } else { /* if (mvrData0 & 0x00000001) */
        /* dense mode */
        mvrParsed->base = (mvrData0 >> 3) & 0xFFFFF;
        count = unitData->denseMvrSize - 1;
        MC_EVERB((MC_MSG1("MVR is dense mode (%d elements)\n"), count + 1));
    } /* if (mvrData0 & 0x00000001) */
    mvrParsed->elems = count + 1;
    /* allocate space for the MVR */
    mvrRaw = soc_cm_salloc(unitData->unit,
                            sizeof(eg_fd_mdb_entry_t) * (mvrParsed->elems),
                            "raw MVR data");
    if (!mvrRaw) {
        MC_ERR((MC_MSG1("unable to allocate %d element buffer"
                        " for MVR at %d:mdb[%08X]\n"),
                count + 1,
                unitData->unit,
                mvrAddr));
        return BCM_E_MEMORY;
    }
    /* load other elements */
    if (count) {
        /* verify addresses */
        if ((mvrAddr + count) >
            SOC_MEM_INFO(unitData->unit, EG_FD_MDBm).index_max) {
            MC_ERR((MC_MSG1("unable to read %d:mdb[%08X]:"
                            " falls out of bounds\n"),
                    unitData->unit,
                    mvrAddr));
            return BCM_E_PARAM;
        }
        /* there are other elements; fetch them */
        result = soc_cm_sinval(unitData->unit,
                               &(mvrRaw[1]),
                               sizeof(*mvrRaw)*(count + 1));
        if (BCM_E_NONE == result) {
            result = soc_mem_read_range(unitData->unit,
                                        EG_FD_MDBm,
                                        MEM_BLOCK_ANY,
                                        mvrAddr + 1,
                                        mvrAddr + count,
                                        &(mvrRaw[1]));
            if (BCM_E_NONE != result) {
                MC_ERR((MC_MSG1("unable to read %d:mdb[%08X..%08X]: %d (%s)\n"),
                        unitData->unit,
                        mvrAddr + 1,
                        mvrAddr + count,
                        result,
                        _SHR_ERRMSG(result)));
            }
        } else { /* if (BCM_E_NONE == result) */
            MC_ERR((MC_MSG1("unable to invalidate cache for"
                            " %d:mdb[%08X..%08X]: %d (%s)\n"),
                    unitData->unit,
                    mvrAddr + 1,
                    mvrAddr + count,
                    result,
                    _SHR_ERRMSG(result)));
        } /* if (BCM_E_NONE == result) */
    } /* if (count) */
    /* copy the initial element into the allocated space */
    if (BCM_E_NONE == result) {
        sal_memcpy(&(mvrRaw[0]), &mvrTemp, sizeof(mvrRaw[0]));
        offset = 1;
        /* build the rest of the target bitmap and target list */
        if (mvrData0 & 0x00000001) {
            /* sparse mode; read list of target numbers */
            while (offset <= count) {
                mvrData1 = soc_mem_field32_get(unitData->unit,
                                               EG_FD_MDBm,
                                               (uint32*)(&(mvrRaw[offset])),
                                               MVR_ENTRYf);
                for (index = 0; index < 4; index++) {
                    mvrParsed->target[mvrParsed->targets] = ((mvrData1 >>
                                                           (8 * index)) &
                                                          0xFF);
                    if (mvrParsed->target[mvrParsed->targets] != 0xFF) {
                        SHR_BITSET(mvrParsed->tsBitmap,
                                   mvrParsed->target[mvrParsed->targets]);
                        mvrParsed->targets++;
                    }
                }
                offset++;
            } /* while (offset <= count) */
        } else { /* if (mvrData0 & 0x00000001) */
            /* dense mode; read target bitmap */
            if (2 <= mvrParsed->elems) {
                mvrData1 = soc_mem_field32_get(unitData->unit,
                                               EG_FD_MDBm,
                                               (uint32*)(&(mvrRaw[1])),
                                               MVR_ENTRYf);
            } else {
                mvrData1 = 0;
            }
            if (3 <= mvrParsed->elems) {
                mvrData2 = soc_mem_field32_get(unitData->unit,
                                               EG_FD_MDBm,
                                               (uint32*)(&(mvrRaw[2])),
                                               MVR_ENTRYf);
            } else {
                mvrData2 = 0;
            }
            if (4 <= mvrParsed->elems) {
                mvrData3 = soc_mem_field32_get(unitData->unit,
                                               EG_FD_MDBm,
                                               (uint32*)(&(mvrRaw[3])),
                                               MVR_ENTRYf);
            } else {
                mvrData3 = 0;
            }
            if (5 <= mvrParsed->elems) {
                mvrData4 = soc_mem_field32_get(unitData->unit,
                                               EG_FD_MDBm,
                                               (uint32*)(&(mvrRaw[4])),
                                               MVR_ENTRYf);
            } else {
                mvrData4 = 0;
            }
#if (32 == SHR_BITWID)
#if (132 == _SIRIUS_MC_MAX_TARGETS)
            mvrParsed->tsBitmap[0] = (mvrData0 >> 23) & 0x000001FF;
            if (2 <= mvrParsed->elems) {
                mvrParsed->tsBitmap[0] |= (mvrData1 << 9);
                mvrParsed->tsBitmap[1] = (mvrData1 >> 23) & 0x000001FF;
            }
            if (3 <= mvrParsed->elems) {
                mvrParsed->tsBitmap[1] |= (mvrData2 << 9);
                mvrParsed->tsBitmap[2] = (mvrData2 >> 23) & 0x000001FF;
            }
            if (4 <= mvrParsed->elems) {
                mvrParsed->tsBitmap[2] |= (mvrData3 << 9);
                mvrParsed->tsBitmap[3] = (mvrData3 >> 23) & 0x000001FF;
            }
            if (5 <= mvrParsed->elems) {
                mvrParsed->tsBitmap[3] |= (mvrData4 << 9);
                mvrParsed->tsBitmap[4] = (mvrData4 >> 23) & 0x0000000F;
            }
#else /* (132 == _SIRIUS_MC_MAX_TARGETS) */
#error "_SIRIUS_MC_MAX_TARGETS has changed without updating mvr read code."
#endif /* (132 == _SIRIUS_MC_MAX_TARGETS) */
            /* parse the target bitmap into the list */
            _bcm_sirius_multicast_sparse_update(mvrParsed);
#else /* (32 == SHR_BITWID) */
#error "Unsupported SHR_BITWID value."
#endif /* (32 == SHR_BITWID) */
        } /* if (mvrData0 & 0x00000001) */
    } /* (BCM_E_NONE == result) */
    /* don't leave the alloc cell lying about */
    soc_cm_sfree(unitData->unit, mvrRaw);

    MC_VVERB((MC_MSG("(%08X,%08X,*) return %d (%s)\n"),
              (uint32)unitData,
              mvrAddr,
              result,
              _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    _bcm_sirius_multicast_mvr_write
 *  Purpose
 *    Allocate an MVR in the MDB and write it.
 *  Arguments
 *    (in) _mc_unit_t *unitData = unit data pointer
 *    (in) uint32 *mvrAddr = where to put the MVR's address
 *    (in) _mc_mvr_internal_t *mvrParsed = ptr to parsed MVR data
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Assumes all arguments are valid.
 *    Switches to dense mode whenever sparse mode is not possible (due to the
 *    OI or the number of targets being too large for sparse mode), and also
 *    will use dense mode if the sparse MVR would be larger than the dense mode
 *    MVR (equal size still prefers sparse mode unless not possible).
 */
static int
_bcm_sirius_multicast_mvr_write(_mc_unit_t *unitData,
                                uint32 *mvrAddr,
                                _mc_mvr_internal_t *mvrParsed)
{
    eg_fd_mdb_entry_t *mvrRaw;
    shr_mdb_elem_index_t mvr;
    uint32 mvrData0;
    uint32 mvrData1;
    uint32 mvrData2;
    uint32 mvrData3;
    uint32 mvrData4;
    unsigned int count;
    unsigned int offset;
    unsigned int index;
    unsigned int mvrSize;
    int result;

    MC_VVERB((MC_MSG("(%08X,*,*) enter\n"),
              (uint32)unitData));

    /* figure out how much space to allocate */
    if ((mvrParsed->targets <=
         (((unitData->denseMvrSize - 1) * 4) + 1)) &&
        (mvrParsed->targets <= _MC_MAX_SPARSE) &&
        (mvrParsed->base < 0x80000)) {
        /* few enough targets and low enough OI to use sparse mode */
        mvrSize = (mvrParsed->targets + 6) >> 2;
        mvrData0 = 0x00000001;
        MC_EVERB((MC_MSG1("MVR is sparse mode (%d elements)\n"), mvrSize));
    } else {
        /*
         *  OI is too high, there are too many targets for sparse mode (largest
         *  sparse mode MVR only holds 13 targets), or the sparse mode MVR
         *  would be larger than the dense mode MVR.  In any of these cases, we
         *  use the dense mode instead.
         */
        mvrSize = unitData->denseMvrSize;
        mvrData0 = 0x00000000;
        MC_EVERB((MC_MSG1("MVR is dense mode (%d elements)\n"), mvrSize));
    }
    /* need to adjust this after the above check */
    mvrParsed->elems = mvrSize;
    /* allocate working buffer space */
    mvrRaw = soc_cm_salloc(unitData->unit,
                           sizeof(*mvrRaw) * mvrSize,
                           "raw MVR data");
    if (!mvrRaw) {
        MC_ERR((MC_MSG1("unable to allocate raw MVR workspace\n")));
        return BCM_E_MEMORY;
    }
    /* allocate the MVR space in the MDB */
    result = shr_mdb_alloc(unitData->mdbList, &mvr, mvrSize);
    if (BCM_E_NONE != result) {
        MC_ERR((MC_MSG1("unable to allocate %d elements in %d:mdb: %d (%s)\n"),
                mvrSize,
                unitData->unit,
                result,
                _SHR_ERRMSG(result)));
        soc_cm_sfree(unitData->unit, mvrRaw);
        return result;
    }
    unitData->mdbBlocks++;
    /* build the MVR */
    if (mvrParsed->logical) {
        mvrData0 |= 0x00000002;
    }
    if (mvrParsed->sourceKnockout) {
        mvrData0 |= 0x00000004;
    }
    if (mvrData0 & 0x00000001) {
        /* sparse mode */
        mvrData0 |= (((mvrParsed->base & 0x7FFFF) << 3) |
                     (((mvrSize - 1) & 0x3) << 22));
        for (count = 0, offset = 0, index = 3;
             count < mvrParsed->targets;
             count++) {
            mvrData0 |= ((mvrParsed->target[count] & 0xFF) << (index * 8));
            if (index < 3) {
                index++;
            } else {
                soc_mem_field32_set(unitData->unit,
                                    EG_FD_MDBm,
                                    (uint32*)(&(mvrRaw[offset])),
                                    MVR_ENTRYf,
                                    mvrData0);
                index = 0;
                mvrData0 = 0;
                offset++;
            }
        }
        while (index) {
            mvrData0 |= (0xFF << (index * 8));
            if (index < 3) {
                index++;
            } else {
                soc_mem_field32_set(unitData->unit,
                                    EG_FD_MDBm,
                                    (uint32*)(&(mvrRaw[offset])),
                                    MVR_ENTRYf,
                                    mvrData0);
                index = 0;
                mvrData0 = 0;
                offset++;
            }
        }
    } else { /* if (mvrData0 & 0x00000001) */
        /* dense mode */
        mvrData0 |= ((mvrParsed->base & 0xFFFFF) << 3);
#if (32 == SHR_BITWID)
#if (132 == _SIRIUS_MC_MAX_TARGETS)
        mvrData0 |= (mvrParsed->tsBitmap[0] << 23);
        if (2 <= mvrSize) {
            mvrData1 = ((mvrParsed->tsBitmap[0] >> 9) & 0x007FFFFF) |
                       (mvrParsed->tsBitmap[1] << 23);
        } else {
            mvrData1 = 0;
        }
        if (3 <= mvrSize) {
            mvrData2 = ((mvrParsed->tsBitmap[1] >> 9) & 0x007FFFFF) |
                       (mvrParsed->tsBitmap[2] << 23);
        } else {
            mvrData2 = 0;
        }
        if (4 <= mvrSize) {
            mvrData3 = ((mvrParsed->tsBitmap[2] >> 9) & 0x007FFFFF) |
                       (mvrParsed->tsBitmap[3] << 23);
        } else {
            mvrData3 = 0;
        }
        if (5 <= mvrSize) {
            mvrData4 = ((mvrParsed->tsBitmap[3] >> 9) & 0x007FFFFF) |
                       ((mvrParsed->tsBitmap[4] << 23) & 0x07800000);
        } else {
            mvrData4 = 0;
        }
#else /* (132 == _SIRIUS_MC_MAX_TARGETS) */
#error "_SIRIUS_MC_MAX_TARGETS has changed without updating mvr read code."
#endif /* (132 == _SIRIUS_MC_MAX_TARGETS) */
#else /* (SHR_BITWID == 32) */
#error "Unsupported SHR_BITWID value."
#endif /* (SHR_BITWID == 32) */
        soc_mem_field32_set(unitData->unit,
                            EG_FD_MDBm,
                            (uint32*)(&(mvrRaw[0])),
                            MVR_ENTRYf,
                            mvrData0);
        if (2 <= mvrSize) {
            soc_mem_field32_set(unitData->unit,
                                EG_FD_MDBm,
                                (uint32*)(&(mvrRaw[1])),
                                MVR_ENTRYf,
                                mvrData1);
        }
        if (3 <= mvrSize) {
            soc_mem_field32_set(unitData->unit,
                                EG_FD_MDBm,
                                (uint32*)(&(mvrRaw[2])),
                                MVR_ENTRYf,
                                mvrData2);
        }
        if (4 <= mvrSize) {
            soc_mem_field32_set(unitData->unit,
                                EG_FD_MDBm,
                                (uint32*)(&(mvrRaw[3])),
                                MVR_ENTRYf,
                                mvrData3);
        }
        if (5 <= mvrSize) {
            soc_mem_field32_set(unitData->unit,
                                EG_FD_MDBm,
                                (uint32*)(&(mvrRaw[4])),
                                MVR_ENTRYf,
                                mvrData4);
        }
    } /* if (mvrData0 & 0x00000001) */
    /* commit the write */
    result = soc_cm_sflush(unitData->unit,
                           mvrRaw,
                           sizeof(eg_fd_mdb_entry_t) * mvrSize);
    if (BCM_E_NONE == result) {
        result = soc_mem_write_range(unitData->unit,
                                     EG_FD_MDBm,
                                     MEM_BLOCK_ALL,
                                     mvr,
                                     mvr + mvrSize - 1,
                                     mvrRaw);
        if (BCM_E_NONE == result) {
            *mvrAddr = mvr;
            mvrParsed->mvr = mvr;
        } else {
            MC_ERR((MC_MSG1("unable to write %d:mdb[%08X..%08X]: %d (%s)\n"),
                    unitData->unit,
                    mvr,
                    mvr + mvrSize - 1,
                    result,
                    _SHR_ERRMSG(result)));
        }
    } else {
        MC_ERR((MC_MSG1("unable to flush cache for %d:mdb[%08X..%08X]:"
                        " %d (%s)\n"),
                unitData->unit,
                mvr,
                mvr + mvrSize - 1,
                result,
                _SHR_ERRMSG(result)));
    }
    /* don't leave the alloc cell lying about */
    soc_cm_sfree(unitData->unit, mvrRaw);
    if (BCM_E_NONE != result) {
        /* don't keep MVR block if we never use it */
        MC_ERR((MC_MSG1("freeing unused MVR %d:mdb[%08X..%08X]: %d (%s)\n"),
                unitData->unit,
                mvr,
                mvr + mvrSize - 1,
                result,
                _SHR_ERRMSG(result)));
        shr_mdb_free(unitData->mdbList, mvr);
        unitData->mdbBlocks--;
    }

    MC_VVERB((MC_MSG("(%08X,&(%08X),*) return %d (%s)\n"),
              (uint32)unitData,
              *mvrAddr,
              result,
              _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    _bcm_sirius_multicast_rr_read
 *  Purpose
 *    Read and parse an RR.
 *  Arguments
 *    (in) _mc_unit_t *unitData = unit data pointer
 *    (in) uint32 rrAddr = RR address in MDB
 *    (in) uint32 rrSize = RR size (elements)
 *    (out) _mc_rr_internal_t *rrParsed = where to put parsed RR data
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Assumes all arguments are valid.
 */
static int
_bcm_sirius_multicast_rr_read(_mc_unit_t *unitData,
                              uint32 rrAddr,
                              unsigned int rrSize,
                              _mc_rr_elem_internal_t *rrParsed)
{
    unsigned int index;
    eg_fd_mdb_entry_t *rrRaw;
    uint32 rrData;
    int result;

    MC_VVERB((MC_MSG("(%08X,%08X,%d,*) enter\n"),
              (uint32)unitData,
              rrAddr,
              rrSize));

    /* verify addresses */
    if ((rrAddr > SOC_MEM_INFO(unitData->unit, EG_FD_MDBm).index_max) ||
        (rrAddr < SOC_MEM_INFO(unitData->unit, EG_FD_MDBm).index_min) ||
        ((rrAddr + rrSize - 1) >
         SOC_MEM_INFO(unitData->unit, EG_FD_MDBm).index_max)) {
        MC_ERR((MC_MSG1("unable to read %d:mdb[%08X..%08X]: out of bounds\n"),
                unitData->unit,
                rrAddr,
                rrAddr + rrSize - 1));
        return BCM_E_PARAM;
    }
    /* allocate working buffer space */
    rrRaw = soc_cm_salloc(unitData->unit,
                          sizeof(*rrRaw) * rrSize,
                          "raw RR data");
    if (!rrRaw) {
        MC_ERR((MC_MSG1("unable to allocate buffer for %d:mdb[%08X..%08X]\n"),
                unitData->unit,
                rrAddr,
                rrAddr + rrSize - 1));
        return BCM_E_MEMORY;
    }
    /* read the RR elements into the buffer */
    result = soc_cm_sinval(unitData->unit,
                           rrRaw,
                           sizeof(*rrRaw) * rrSize);
    if (BCM_E_NONE == result) {
        result = soc_mem_read_range(unitData->unit,
                                    EG_FD_MDBm,
                                    MEM_BLOCK_ANY,
                                    rrAddr,
                                    rrAddr + rrSize - 1,
                                    rrRaw);
        if (BCM_E_NONE != result) {
            MC_ERR((MC_MSG1("unable to read %d:mdb[%08X..%08X]: %d (%s)\n"),
                    unitData->unit,
                    rrAddr,
                    rrAddr + rrSize - 1,
                    result,
                    _SHR_ERRMSG(result)));
        }
    } else {
        MC_ERR((MC_MSG1("unable to invalidate cache for"
                        " %d:mdb[%08X..%08X]: %d (%s)\n"),
                unitData->unit,
                rrAddr,
                rrAddr + rrSize - 1,
                result,
                _SHR_ERRMSG(result)));
    }
    /* parse the RR elements into useful data */
    if (BCM_E_NONE == result) {
        for (index = 0; index < rrSize; index++) {
            soc_mem_field_get(unitData->unit,
                              EG_FD_MDBm,
                              (uint32*)(&(rrRaw[index])),
                              MVR_ENTRYf,
                              &rrData);
           rrParsed[index].base = (rrData >> 12) & 0x000FFFFF;
           rrParsed[index].replicants = (rrData & 0x00000FFF) + 1;
        }
    }
    /* dispose of the alloc cell */
    soc_cm_sfree(unitData->unit, rrRaw);

    MC_VVERB((MC_MSG("(%08X,%08X,%d,*) return %d (%s)\n"),
              (uint32)unitData,
              rrAddr,
              rrSize,
              result,
              _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    _bcm_sirius_multicast_rr_write
 *  Purpose
 *    Allocate an RR in the MDB and write it.
 *  Arguments
 *    (in) _mc_unit_t *unitData = unit data pointer
 *    (in) uint32 *rrAddr = where to put the RR address in MDB
 *    (in) uint32 rrSize = RR size (elements)
 *    (in) _mc_rr_internal_t *rrParsed = ptr to parsed RR data
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Assumes all arguments are valid.
 */
static int
_bcm_sirius_multicast_rr_write(_mc_unit_t *unitData,
                               uint32 *rrAddr,
                               unsigned int rrSize,
                               _mc_rr_elem_internal_t *rrParsed)
{
    unsigned int index;
    eg_fd_mdb_entry_t *rrRaw;
    uint32 rrData;
    shr_mdb_elem_index_t rr;
    int result;

    MC_VVERB((MC_MSG("(%08X,*,%d,*) enter\n"),
              (uint32)unitData,
              rrSize));

    /* allocate working buffer space */
    rrRaw = soc_cm_salloc(unitData->unit,
                          sizeof(*rrRaw) * rrSize,
                          "raw RR data");
    if (!rrRaw) {
        MC_ERR((MC_MSG1("unable to allocate buffer for %d:mdb\n"),
                unitData->unit));
        return BCM_E_MEMORY;
    }
    /* allocate the RR space in the MDB */
    result = shr_mdb_alloc(unitData->mdbList, &rr, rrSize);
    if (BCM_E_NONE != result) {
        MC_ERR((MC_MSG1("unable to allocate %d elements in %d:mdb: %d (%s)\n"),
                rrSize,
                unitData->unit,
                result,
                _SHR_ERRMSG(result)));
        soc_cm_sfree(unitData->unit, rrRaw);
        return result;
    }
    unitData->mdbBlocks++;
    /* fill the buffer with the RR data */
    for (index = 0; index < rrSize; index++) {
        rrData = (((rrParsed[index].base << 12) & 0xFFFFF000) |
                  ((rrParsed[index].replicants - 1) & 0x00000FFF));
        soc_mem_field_set(unitData->unit,
                          EG_FD_MDBm,
                          (uint32*)(&(rrRaw[index])),
                          MVR_ENTRYf,
                          &rrData);
    }
    /* write the RR elements from the buffer */
    result = soc_cm_sflush(unitData->unit,
                           rrRaw,
                           sizeof(*rrRaw) * rrSize);
    if (BCM_E_NONE == result) {
        result = soc_mem_write_range(unitData->unit,
                                     EG_FD_MDBm,
                                     MEM_BLOCK_ALL,
                                     rr,
                                     rr + rrSize - 1,
                                     rrRaw);
        if (BCM_E_NONE == result) {
            *rrAddr = rr;
        } else {
            MC_ERR((MC_MSG1("unable to write %d:mdb[%08X..%08X]: %d (%s)\n"),
                    unitData->unit,
                    rr,
                    rr + rrSize - 1,
                    result,
                    _SHR_ERRMSG(result)));
        }
    } else {
        MC_ERR((MC_MSG1("unable to flush cache for"
                        " %d:mdb[%08X..%08X]: %d (%s)\n"),
                unitData->unit,
                rr,
                rr + rrSize - 1,
                result,
                _SHR_ERRMSG(result)));
    }

    /* dispose of the alloc cell */
    soc_cm_sfree(unitData->unit, rrRaw);
    if (BCM_E_NONE != result) {
        /* don't keep RR block if we never use it */
        MC_ERR((MC_MSG1("freeing unused RR %d:mdb[%08X..%08X]: %d (%s)\n"),
                unitData->unit,
                rr,
                rr + rrSize - 1,
                result,
                _SHR_ERRMSG(result)));
        shr_mdb_free(unitData->mdbList, rr);
        unitData->mdbBlocks--;
    }

    MC_VVERB((MC_MSG("(%08X,&(%08X),%d,*) return %d (%s)\n"),
              (uint32)unitData,
              *rrAddr,
              rrSize,
              result,
              _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    _bcm_sirius_multicast_oitt_read
 *  Purpose
 *    Read and parse a set of OITT entries.
 *  Arguments
 *    (in) _mc_unit_t *unitData = unit data pointer
 *    (in) uint32 oittAddr = OITT address
 *    (in) uint32 oittSize = OITT size (elements)
 *    (out) uint32 *oittParsed = where to put parsed OITT data
 *    (out) uint32 *queueParsed = where to put parsed queue data
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Only basic sanity checks performed here.
 *
 *    Creates and returns cache block for new GPORT values.
 *
 *    The OITT cache is flat and statically allocated (the OITT space is flat,
 *    and does not change shape during runs either).
 *
 *    The GPORT cache orgaisation is explained in the unit global information.
 *
 *    The queueParsed argument is not used unless dual lookup is enabled.
 */
static int
_bcm_sirius_multicast_oitt_read(_mc_unit_t *unitData,
                                const uint32 oittAddr,
                                const unsigned int oittSize,
                                uint32 *oittParsed,
                                uint32 *queueParsed)
{
    ep_oi2qb_map_entry_t *oittRaw;
    int result = BCM_E_NONE;
    unsigned int replIndex;
    uint32 baseAddr;

    MC_VVERB((MC_MSG("(%08X,%08X,%d,*,*) enter\n"),
              (uint32)unitData,
              oittAddr,
              oittSize));

    if (!oittSize) {
        MC_ERR((MC_MSG1("can't have zero size OITT entry\n")));
        return BCM_E_PARAM;
    } /* if (!oittSize) */

    /* get the OITT data */
    if ((oittAddr <= unitData->oittMax) &&
        (oittAddr >= unitData->oittMin) &&
        ((oittAddr + oittSize - 1) <= unitData->oittMax)) {
        /* OITT address is valid; fetch from the hardware */
        oittRaw = soc_cm_salloc(unitData->unit,
                                sizeof(*oittRaw) * oittSize,
                                "raw OITT data");
        if (!oittRaw) {
            MC_ERR((MC_MSG1("unable to allocate buffer for %d:oi2qb\n"),
                    unitData->unit));
            return BCM_E_MEMORY;
        }
        result = soc_cm_sinval(unitData->unit,
                               oittRaw,
                               sizeof(*oittRaw) * oittSize);
        if (BCM_E_NONE == result) {
            result = soc_mem_read_range(unitData->unit,
                                        EP_OI2QB_MAPm,
                                        MEM_BLOCK_ANY,
                                        oittAddr,
                                        oittAddr + oittSize - 1,
                                        oittRaw);
            if (BCM_E_NONE == result) {
                for (replIndex = 0; replIndex < oittSize; replIndex++) {
                    oittParsed[replIndex] = soc_mem_field32_get(unitData->unit,
                                                                EP_OI2QB_MAPm,
                                                                &(oittRaw[replIndex]),
                                                                QUEUE_BASEf);
                }
            } else { /* if (BCM_E_NONE == result) */
                MC_ERR((MC_MSG1("unable to read %d:oi2qb[%08X..%08X]:"
                                " %d (%s)\n"),
                        unitData->unit,
                        oittAddr,
                        oittAddr + oittSize - 1,
                        result,
                        _SHR_ERRMSG(result)));
            } /* if (BCM_E_NONE == result) */
        } else { /* if (BCM_E_NONE == result) */
            MC_ERR((MC_MSG1("unable to invalidate cache for"
                            " %d:oi2qb[%08X..%08X]: %d (%s)\n"),
                    unitData->unit,
                    oittAddr,
                    oittAddr + oittSize - 1,
                    result,
                    _SHR_ERRMSG(result)));
        } /* if (BCM_E_NONE == result) */
        if ((BCM_E_NONE == result) &&
            unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
            baseAddr = oittAddr - unitData->oittMin + unitData->queueMin;
            result = soc_cm_sinval(unitData->unit,
                                   oittRaw,
                                   sizeof(*oittRaw) * oittSize);
            if (BCM_E_NONE == result) {
                result = soc_mem_read_range(unitData->unit,
                                            EP_OI2QB_MAPm,
                                            MEM_BLOCK_ANY,
                                            baseAddr,
                                            baseAddr + oittSize - 1,
                                            oittRaw);
                if (BCM_E_NONE == result) {
                    for (replIndex = 0; replIndex < oittSize; replIndex++) {
                        queueParsed[replIndex] = soc_mem_field32_get(unitData->unit,
                                                                     EP_OI2QB_MAPm,
                                                                     &(oittRaw[replIndex]),
                                                                     QUEUE_BASEf);
                    }
                } else { /* if (BCM_E_NONE == result) */
                    MC_ERR((MC_MSG1("unable to read %d:oi2qb[%08X..%08X]:"
                                    " %d (%s)\n"),
                            unitData->unit,
                            baseAddr,
                            baseAddr + oittSize - 1,
                            result,
                            _SHR_ERRMSG(result)));
                } /* if (BCM_E_NONE == result) */
            } else { /* if (BCM_E_NONE == result) */
                MC_ERR((MC_MSG1("unable to invalidate cache for"
                                " %d:oi2qb[%08X..%08X]: %d (%s)\n"),
                        unitData->unit,
                        baseAddr,
                        baseAddr + oittSize - 1,
                        result,
                        _SHR_ERRMSG(result)));
            } /* if (BCM_E_NONE == result) */
        } /* if (no error and dual lookup mode) */
        /* dispose of the alloc cell */
        soc_cm_sfree(unitData->unit, oittRaw);
    } else { /* if (valid OITT address and range) */
        MC_ERR((MC_MSG1("unit %d invalid OITT address %08X..%08X\n"),
                unitData->unit,
                oittAddr,
                oittAddr + oittSize - 1));
        /*
         *  This might be an error if we didn't have the ability to 'turn off'
         *  the translation, or if we didn't stuff some out-of-range value for
         *  that case, but we do and we do, so it's not guaranteed to be bad.
         */
        for (replIndex = 0; replIndex < oittSize; replIndex++) {
            oittParsed[replIndex] = ~0;
        }
        if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
            for (replIndex = 0; replIndex < oittSize; replIndex++) {
                queueParsed[replIndex] = ~0;
            }
        }
    } /* if (valid OITT address and range) */

    MC_VVERB((MC_MSG("(%08X,%08X,%d,*,*) return %d (%s)\n"),
              (uint32)unitData,
              oittAddr,
              oittSize,
              result,
              _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    _bcm_sirius_multicast_oitt_write
 *  Purpose
 *    Write a set of OITT entries.
 *  Arguments
 *    (in) _mc_unit_t *unitData = unit data pointer
 *    (out) uint32 *oittAddr = where to put OITT address
 *    (in) uint32 oittSize = OITT size (elements)
 *    (in) uint32 *oittParsed = parsed OITT data
 *    (in) uint32 *queueParsed = parsed queue data
 *    (in) int noTrans = TRUE to disable OI translation
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Only basic sanity checks performed here.
 *
 *    Creates and returns cache block for new GPORT values.
 *
 *    The OITT cache is flat and statically allocated (the OITT space is flat,
 *    and does not change shape during runs either).
 *
 *    The GPORT cache orgaisation is explained in the unit global information.
 *
 *    The queueParsed argument is not used unless dual lookup is enabled.
 */
static int
_bcm_sirius_multicast_oitt_write(_mc_unit_t *unitData,
                                 uint32 *oittAddr,
                                 const unsigned int oittSize,
                                 const uint32 *oittParsed,
                                 const uint32 *queueParsed,
                                 const int noTrans)
{
    shr_mdb_elem_index_t oitt;
    ep_oi2qb_map_entry_t *oittRaw;
    int result = BCM_E_NONE;
    unsigned int replIndex;
    uint32 baseAddr;

    MC_VVERB((MC_MSG("(%08X,*,%d,*,%s) enter\n"),
              (uint32)unitData,
              oittSize,
              noTrans?"TRUE":"FALSE"));

    if (!oittSize) {
        MC_ERR((MC_MSG1("can't have zero size OITT entry\n")));
        return BCM_E_PARAM;
    } /* if (!oittSize) */

    if (noTrans) {
        /* always return 'out of range' translation, and write nothing */
        oitt = SOC_MEM_INFO(unitData->unit, EP_OI2QB_MAPm).index_max + 1;
    } else { /* if (noTrans) */
        /* allocate working buffer space */
        oittRaw = soc_cm_salloc(unitData->unit,
                                sizeof(*oittRaw) * oittSize,
                                "raw OITT data");
        if (!oittRaw) {
            MC_ERR((MC_MSG1("unable to allocate buffer for %d:oi2qb\n"),
                    unitData->unit));
            return BCM_E_MEMORY;
        }

        /* allocate space in OITT */
        result = shr_mdb_alloc(unitData->oittList, &oitt, oittSize);
        if (BCM_E_NONE != result) {
            MC_ERR((MC_MSG1("unable to allocate %d elements in %d:oi2qb: %d (%s)\n"),
                    oittSize,
                    unitData->unit,
                    result,
                    _SHR_ERRMSG(result)));
            soc_cm_sfree(unitData->unit, oittRaw);
            return result;
        }
        unitData->oittBlocks++;

        /* fill in the buffer */
        for (replIndex = 0; replIndex < oittSize; replIndex++) {
            /* write OI translation data to the buffer */
            soc_mem_field32_set(unitData->unit,
                                EP_OI2QB_MAPm,
                                &(oittRaw[replIndex]),
                                QUEUE_BASEf,
                                oittParsed[replIndex] & _MC_OITT_HW_MASK);
        }

        /* commit the buffer */
        result = soc_cm_sflush(unitData->unit,
                               oittRaw,
                               sizeof(*oittRaw) * oittSize);
        if (BCM_E_NONE == result) {
            result = soc_mem_write_range(unitData->unit,
                                         EP_OI2QB_MAPm,
                                         MEM_BLOCK_ALL,
                                         oitt,
                                         oitt + oittSize - 1,
                                         oittRaw);
            if (BCM_E_NONE != result) {
                MC_ERR((MC_MSG1("unable to write %d:oi2qb[%08X..%08X]: %d (%s)\n"),
                        unitData->unit,
                        oitt,
                        oitt + oittSize - 1,
                        result,
                        _SHR_ERRMSG(result)));
            } /* if (BCM_E_NONE == result) */
        } else { /* if (BCM_E_NONE == result) */
            MC_ERR((MC_MSG1("unable to flush cache for"
                            " %d:oi2qb[%08X..%08X]: %d (%s)\n"),
                    unitData->unit,
                    oitt,
                    oitt + oittSize - 1,
                    result,
                    _SHR_ERRMSG(result)));
        } /* if (BCM_E_NONE == result) */

        if ((BCM_E_NONE == result) &&
            unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
            baseAddr = oitt - unitData->oittMin + unitData->queueMin;
            /* fill in the buffer */
            for (replIndex = 0; replIndex < oittSize; replIndex++) {
                /* write OI translation data to the buffer */
                soc_mem_field32_set(unitData->unit,
                                    EP_OI2QB_MAPm,
                                    &(oittRaw[replIndex]),
                                    QUEUE_BASEf,
                                    queueParsed[replIndex] & _MC_OITT_HW_MASK);
            }

            /* commit the buffer */
            result = soc_cm_sflush(unitData->unit,
                                   oittRaw,
                                   sizeof(*oittRaw) * oittSize);
            if (BCM_E_NONE == result) {
                result = soc_mem_write_range(unitData->unit,
                                             EP_OI2QB_MAPm,
                                             MEM_BLOCK_ALL,
                                             baseAddr,
                                             baseAddr + oittSize - 1,
                                             oittRaw);
                if (BCM_E_NONE != result) {
                    MC_ERR((MC_MSG1("unable to write %d:oi2qb[%08X..%08X]: %d (%s)\n"),
                            unitData->unit,
                            baseAddr,
                            baseAddr + oittSize - 1,
                            result,
                            _SHR_ERRMSG(result)));
                } /* if (BCM_E_NONE == result) */
            } else { /* if (BCM_E_NONE == result) */
                MC_ERR((MC_MSG1("unable to flush cache for"
                                " %d:oi2qb[%08X..%08X]: %d (%s)\n"),
                        unitData->unit,
                        baseAddr,
                        baseAddr + oittSize - 1,
                        result,
                        _SHR_ERRMSG(result)));
            } /* if (BCM_E_NONE == result) */
        } /* if (no error and dual lookup mode) */

        /* dispose of the alloc cell */
        soc_cm_sfree(unitData->unit, oittRaw);
    } /* if (noTrans) */

    /* clean up if there was an error */
    if (BCM_E_NONE == result) {
        *oittAddr = oitt;
    } else {
        /* free scratch OITT space */
        MC_ERR((MC_MSG1("freeing unused OITT %d:oi2qb[%08X..%08X]: %d (%s)\n"),
                unitData->unit,
                oitt,
                oitt + oittSize - 1,
                result,
                _SHR_ERRMSG(result)));
        if (!noTrans) {
            shr_mdb_free(unitData->oittList, oitt);
            unitData->oittBlocks--;
        }
    }

    MC_VVERB((MC_MSG("(%08X,&(%08X),%d,*,%s) return %d (%s)\n"),
              (uint32)unitData,
              *oittAddr,
              oittSize,
              noTrans?"TRUE":"FALSE",
              result,
              _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    _bcm_sirius_multicast_read_partial
 *  Purpose
 *    Fetch all but the OITT elements for a multicast group
 *  Arguments
 *    (in) _mc_unit_t *unitData = unit data pointer
 *    (in) bcm_multicast_t mcGroup = MC group whose path data are to be read
 *    (out) _mc_mvr_internal_t **mvrPtr = where to put the pointer to the data
 *    (out) uint32 *mvrAddr = where to put current MVR address
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Assumes all arguments are valid.
 *    Caller owns all alloc memory after successful return.
 *    Disposes of all alloc memory if error.
 *    Only fetches the MVR and (if applicable) the RR.
 *    Useful when removing replicants or destroying a group.
 */
static int
_bcm_sirius_multicast_read_partial(_mc_unit_t *unitData,
                                   const bcm_multicast_t mcGroup,
                                   _mc_mvr_internal_t** mvrPtr,
                                   uint32 *mvrAddr)
{
    _mc_mvr_internal_t *mvrData;
    eg_fd_gmt_entry_t gmtData;
    uint32 mvr;
    int result;

    MC_VVERB((MC_MSG("(%08X,%08X,**) enter\n"),
              (uint32)unitData,
              mcGroup));

    /* fetch and parse the GMT entry */
    result = soc_mem_read(unitData->unit,
                          EG_FD_GMTm,
                          MEM_BLOCK_ANY,
                          mcGroup,
                          &gmtData);
    if (BCM_E_NONE == result) {
        /* parse the entry */
        mvr = soc_EG_FD_GMTm_field32_get(unitData->unit,
                                         &(gmtData),
                                         MVRPf);
    } else {
        MC_ERR((MC_MSG1("unable to read %d:gmt[%08X]: %d (%s)\n"),
                unitData->unit,
                mcGroup,
                result,
                _SHR_ERRMSG(result)));
        return result;
    }
    if (!mvr) {
        /* got the entry but it is 'disabled' */
        MC_ERR((MC_MSG1("%d:gmt[%08X] indicates group disabled (0 == MVRp)\n"),
                unitData->unit,
                mcGroup));
        return BCM_E_NOT_FOUND;
    }
    if (_MC_UNICAST_MVR_MASK == (_MC_UNICAST_MVR_MASK & mvr)) {
        /* this entry looks more like unicast */
        MC_ERR((MC_MSG1("%d:gmt[%08X] indicates a unicast target\n"),
                unitData->unit,
                mcGroup));
        
        return BCM_E_NOT_FOUND;
    }

    /* allocate space for an MVR descriptor */
    mvrData = sal_alloc(sizeof(*mvrData), "MVR workspace");
    if (!mvrData) {
        MC_ERR((MC_MSG1("unable to allocate workspace for %d:mvr[%08X]\n"),
                unitData->unit,
                mvr));
        return BCM_E_MEMORY;
    }
    sal_memset(mvrData, 0x00, sizeof(*mvrData));
    /* fill in the MVR descriptor */
    result = _bcm_sirius_multicast_mvr_read(unitData, mvr, mvrData);
    if ((BCM_E_NONE == result) && (mvrData->targets)) {
        /* MVR descriptor fetch good and it has targets enabled */
        if (mvrData->logical) {
            /* need to treat MVR as logical, so also fetch RR */
            mvrData->ptr.rr = sal_alloc(sizeof(*(mvrData->ptr.rr)) *
                                        mvrData->targets,
                                        "RR workspace");
            if (mvrData->ptr.rr) {
                sal_memset(mvrData->ptr.rr,
                           0x00,
                           sizeof(*(mvrData->ptr.rr)) * mvrData->targets);
            } else {
                MC_ERR((MC_MSG1("unable to allocate workspace for %d RRs"
                                " for %d:mvr[%08X]\n"),
                        unitData->unit,
                        mvrData->targets,
                        mvr));
                result = BCM_E_MEMORY;
            }
            if (BCM_E_NONE == result) {
                result = _bcm_sirius_multicast_rr_read(unitData,
                                                       mvrData->base,
                                                       mvrData->targets,
                                                       mvrData->ptr.rr);
            }
        } /* if (mvrData->logical) */
    } /* if ((BCM_E_NONE == result) && (mvrData->targets)) */

    if (BCM_E_NONE == result) {
        /* all okay; update caller's data */
        *mvrPtr = mvrData;
        *mvrAddr = mvr;
    } else { /* if (BCM_E_NONE == result) */
        /* something went wrong; release resources */
        _bcm_sirius_multicast_mvrdata_free(mvrData);
    } /* if (BCM_E_NONE == result) */

    MC_VVERB((MC_MSG("(%08X,%08X,&(%08X)) return %d (%s)\n"),
              (uint32)unitData,
              mcGroup,
              (uint32)(*mvrPtr),
              result,
              _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    _bcm_sirius_multicast_read
 *  Purpose
 *    Fetch an entire multicast path set.
 *  Arguments
 *    (in) _mc_unit_t *unitData = unit data pointer
 *    (in) bcm_multicast_t mcGroup = MC group whose path data are to be read
 *    (out) _mc_mvr_internal_t **mvrPtr = where to put the pointer to the data
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Assumes all arguments are valid.
 *    Caller owns all alloc memory after successful return.
 *    Disposes of all alloc memory if error.
 */
static int
_bcm_sirius_multicast_read(_mc_unit_t *unitData,
                           const bcm_multicast_t mcGroup,
                           _mc_mvr_internal_t **mvrPtr)
{
    _mc_mvr_internal_t *mvrData = NULL;
    uint32 mvrAddr;
    unsigned int index;
    int result;

    MC_VVERB((MC_MSG("(%08X,%08X,**) enter\n"),
              (uint32)unitData,
              mcGroup));

    /* get the MVR (and RR if applicable) first */
    result = _bcm_sirius_multicast_read_partial(unitData,
                                                mcGroup,
                                                &mvrData,
                                                &mvrAddr);
    if (BCM_E_NONE != result) {
        /* called function already displayed error */
        return result;
    }

    if (mvrData->targets) {
        /* MVR descriptor fetch good and it has targets enabled */
        if (mvrData->logical) {
            /* need to treat MVR as logical */
            for (index = 0;
                 (BCM_E_NONE == result) && (index < mvrData->targets);
                 index++) {
                mvrData->ptr.rr[index].oittPtr = sal_alloc(sizeof(*(mvrData->ptr.rr[index].oittPtr)) *
                                                           mvrData->ptr.rr[index].replicants,
                                                           "OITT workspace");
                if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
                    mvrData->ptr.rr[index].queuePtr = sal_alloc(sizeof(*(mvrData->ptr.rr[index].queuePtr)) *
                                                                mvrData->ptr.rr[index].replicants,
                                                                "queue workspace");
                }
                if (mvrData->ptr.rr[index].oittPtr &&
                    (!(unitData->flags & _MC_UNIT_DUAL_LOOKUP) ||
                    mvrData->ptr.rr[index].queuePtr)) {
                    sal_memset(mvrData->ptr.rr[index].oittPtr,
                               0x00,
                               sizeof(*(mvrData->ptr.rr[index].oittPtr)) *
                               mvrData->ptr.rr[index].replicants);
                    if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
                        sal_memset(mvrData->ptr.rr[index].queuePtr,
                                   0x00,
                                   sizeof(*(mvrData->ptr.rr[index].queuePtr)) *
                                   mvrData->ptr.rr[index].replicants);
                    }
                    result = _bcm_sirius_multicast_oitt_read(unitData,
                                                             mvrData->ptr.rr[index].base,
                                                             mvrData->ptr.rr[index].replicants,
                                                             mvrData->ptr.rr[index].oittPtr,
                                                             mvrData->ptr.rr[index].queuePtr);
                    if (BCM_E_NONE == result) {
                        mvrData->ptr.rr[index].noTrans = (unitData->oittMax <
                                                          mvrData->ptr.rr[index].base);
                    }
                } else {
                    MC_ERR((MC_MSG1("unable to allocate workspace for"
                                    " %d OITTs for unit %d group %08X"
                                    " rr[%08X]\n"),
                            mvrData->ptr.rr[index].replicants,
                            unitData->unit,
                            mcGroup,
                            mvrData->ptr.rr[index].base));
                    result = BCM_E_MEMORY;
                    break;
                }
            } /* for all RR elements */
        } else { /* if (mvrData->logical) */
            /* need to treat MVR as physical */
            mvrData->ptr.oitt = sal_alloc(sizeof(*(mvrData->ptr.oitt)) *
                                          mvrData->targets,
                                          "OITT workspace");
            if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
                mvrData->queuePtr = sal_alloc(sizeof(*(mvrData->queuePtr)) *
                                              mvrData->targets,
                                              "queue workspace");
            }
            if (mvrData->ptr.oitt &&
                (!(unitData->flags & _MC_UNIT_DUAL_LOOKUP) ||
                 mvrData->queuePtr)) {
                sal_memset(mvrData->ptr.oitt,
                           0x00,
                           sizeof(*(mvrData->ptr.oitt)) * mvrData->targets);
                if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
                    sal_memset(mvrData->queuePtr,
                               0x00,
                               sizeof(*(mvrData->queuePtr)) * mvrData->targets);
                }
                result = _bcm_sirius_multicast_oitt_read(unitData,
                                                         mvrData->base,
                                                         mvrData->targets,
                                                         mvrData->ptr.oitt,
                                                         mvrData->queuePtr);
                if (BCM_E_NONE == result) {
                    mvrData->noTrans = (unitData->oittMax < mvrData->base);
                }
            } else {
                MC_ERR((MC_MSG1("unable to allocate workspace for %d OITTs"
                                " for unit %d group %08X\n"),
                        mvrData->targets,
                        unitData->unit,
                        mcGroup));
                result = BCM_E_MEMORY;
            }
        } /* if (mvrData->logical) */
    } /* if (mvrData->targets) */

    if (BCM_E_NONE == result) {
        /* all okay; update caller's pointer */
        *mvrPtr = mvrData;
    } else { /* if (BCM_E_NONE == result) */
        /* something went wrong; release resources */
        _bcm_sirius_multicast_mvrdata_free(mvrData);
    } /* if (BCM_E_NONE == result) */

    MC_VVERB((MC_MSG("(%08X,%08X,&(%08X)) return %d (%s)\n"),
              (uint32)unitData,
              mcGroup,
              (uint32)(*mvrPtr),
              result,
              _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    _bcm_sirius_multicast_write
 *  Purpose
 *    Write the parts of a path set that have changed, handling the limbo
 *    requirement and other related subtleties.  Since some data in the
 *    descriptors can change, this will update the caller's data on success.
 *  Arguments
 *    (in) _mc_unit_t *unitData = unit data pointer
 *    (in) bcm_multicast_t mcGroup = MC group whose path data are to be written
 *    (out) _gpCachePtr_t gportCache = where to put the gport cache info
 *    (in/out) _mc_mvr_internal_t **mvrPtr = where is pointer to MVR data
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Assumes all arguments are valid.
 *
 *    This is intended to be an 'update' or 'add' type function.  It has only
 *    partial provisions for remove (the caller must handle some of it).
 *
 *    Does not direct overwrite; allocates new MVR, RR, OITT as appropriate;
 *    provides a copy of the existing MVR (and RR if appropriate) data modified
 *    to have the new locations in place.
 *
 *    Caller must set appropriate 'changed' fields before calling to indicate
 *    the changed or added parts.  If adding a new target (for example a new
 *    target), the applicable 'base' field must be set to _MC_INVALID_BASE.  If
 *    adding a new replicant, the target to which the replicant belongs must be
 *    marked as changed.
 *
 *    'Changed' parts will be allocated new res & old will be put in limbo.
 *
 *    'Changed' indicator fields will be reset after successful completion.
 *
 *    This will place any resources it delinks for substitution into the limbo
 *    list, but the caller must also do the same for any resources that are
 *    being removed instead of updated.
 *
 *    The gport cache block will be created, and filled in for physical mode,
 *    but logical mode will only have lines for the updated targets (old lines
 *    for updated targets or delete targets will have to be freed, and lines
 *    for non-updated targets copied by the caller before committing it.
 */
static int
_bcm_sirius_multicast_write(_mc_unit_t *unitData,
                            const bcm_multicast_t mcGroup,
                            _mc_mvr_internal_t **mvrPtr)
{
    _mc_mvr_internal_t *mvrData;
    eg_fd_gmt_entry_t gmtData;
    uint32 mvr = _MC_INVALID_BASE;
    uint32 oldMvr;
    unsigned int index;
    int result = BCM_E_NONE;

    MC_VVERB((MC_MSG("(%08X,%08X,*,*) enter\n"),
              (uint32)unitData,
              mcGroup));

    /* fetch and parse the GMT entry */
    result = soc_mem_read(unitData->unit,
                          EG_FD_GMTm,
                          MEM_BLOCK_ANY,
                          mcGroup,
                          &gmtData);
    if (BCM_E_NONE == result) {
        /* parse the entry */
        oldMvr = soc_EG_FD_GMTm_field32_get(unitData->unit,
                                            &(gmtData),
                                            MVRPf);
        if ((0 == oldMvr) ||
            (0x2FFFF < oldMvr)) {
            MC_EVERB((MC_MSG1("original unit %d group %08X MVR was %08X (%s);"
                              " considering group to be new\n"),
                      unitData->unit,
                      mcGroup,
                      oldMvr,
                      (0 == oldMvr)?"cleared":
                      (0x3FFFF == oldMvr)?"power-on reset":
                      ((0x3FF00 <= oldMvr) && (0x3FF83 >= oldMvr))?"unicast":
                      ((0x3FFFE <= oldMvr) && (0x3FFFF >= oldMvr))?"hwAggregate":
                      "invalid"));
            oldMvr = 0;
        } else {
            MC_EVERB((MC_MSG1("original MVR for unit %d group %08X was %08X\n"),
                      unitData->unit,
                      mcGroup,
                      oldMvr));
        }
    } else {
        MC_ERR((MC_MSG1("unable to read %d:gmt[%08X]: %d (%s)\n"),
                unitData->unit,
                mcGroup,
                result,
                _SHR_ERRMSG(result)));
        return result;
    }

    /* make a copy of the passed-in MVR (and RR if appropriate) */
    /* no need to copy OITTs; not changed so can leave these in place */
    mvrData = sal_alloc(sizeof(*mvrData), "MVR write workspace");
    if (mvrData) {
        /* alloc good; copy MVR descriptor */
        sal_memcpy(mvrData, *mvrPtr, sizeof(*mvrData));
        if (mvrData->targets) {
            /* it has targets */
            if (mvrData->logical) {
                /* MVR is logical; has an RR descriptor */
                mvrData->ptr.rr = sal_alloc((sizeof(*(mvrData->ptr.rr)) *
                                             mvrData->targets),
                                            "RR write workspace");
                if (mvrData->ptr.rr) {
                    /* alloc good; copy this RR descriptor */
                    sal_memcpy(mvrData->ptr.rr,
                               (*mvrPtr)->ptr.rr,
                               (sizeof(*(mvrData->ptr.rr)) * mvrData->targets));
                    for (index = 0; index < mvrData->targets; index++) {
                        /* don't copy addresses of changed OITT blocks */
                        if (mvrData->ptr.rr[index].changed) {
                            mvrData->ptr.rr[index].base = _MC_INVALID_BASE;
                            /* must propagate change upward */
                            mvrData->ptrChanged = TRUE;
                        }
                    }
                } else {
                    MC_ERR((MC_MSG1("unable to allocate %d bytes"
                                    " for RR workspace\n"),
                            (sizeof(*(mvrData->ptr.rr)) * mvrData->targets)));
                    result = BCM_E_MEMORY;
                }
            } /* if (mvrData->logical) */
        } /* if (mvrData->targets) */
        if (mvrData->ptrChanged) {
            /* don't keep address of changed OITT/RR */
            mvrData->base = _MC_INVALID_BASE;
            /* must propagate change upward */
            mvrData->changed = TRUE;
        }
    } else { /* if (mvrData) */
        MC_ERR((MC_MSG1("unable to allocate %d bytes for MVR workspace\n"),
                sizeof(*mvrData)));
        result = BCM_E_MEMORY;
    } /* if (mvrData) */

    /* write the data (OITT up) */
    if ((BCM_E_NONE == result) && mvrData->targets) {
        /* we do have targets */
        if (mvrData->logical) {
            /* logical mode */
            for (index = 0; index < mvrData->targets; index++) {
                if (mvrData->ptr.rr[index].changed) {
                    /* need to write OITT data */
                    result = _bcm_sirius_multicast_oitt_write(unitData,
                                                              &(mvrData->ptr.rr[index].base),
                                                              mvrData->ptr.rr[index].replicants,
                                                              mvrData->ptr.rr[index].oittPtr,
                                                              mvrData->ptr.rr[index].queuePtr,
                                                              mvrData->ptr.rr[index].noTrans);
                    /* called function already displayed any errors */
                    if (BCM_E_NONE != result) {
                        /* if out of resources, trigger a defrag... */
                        if (BCM_E_RESOURCE == result) {
                            unitData->flags = (unitData->flags &
                                               (~_MC_UNIT_DEFRAG_BY_MASK)) |
                                              (_MC_UNIT_DEFRAG_START |
                                               _MC_UNIT_DEFRAG_BY_RES);
                            unitData->defragAttRes++;
                        }
                        /* ...and give up for now no matter which error */
                        break;
                    } /* if (BCM_E_NONE != result) */
                } /* if (mvrData->ptr.rr[index].changed) */
            } /* for (index = 0; index < mvrData->targets; index++) */
            /* write RR */
            if (BCM_E_NONE == result) {
                if (mvrData->ptrChanged) {
                    result = _bcm_sirius_multicast_rr_write(unitData,
                                                            &(mvrData->base),
                                                            mvrData->targets,
                                                            mvrData->ptr.rr);
                    /* called function already displayed any errors */
                    /* if out of resources, trigger a defrag */
                    if (BCM_E_RESOURCE == result) {
                        unitData->flags = (unitData->flags &
                                           (~_MC_UNIT_DEFRAG_BY_MASK)) |
                                          (_MC_UNIT_DEFRAG_START |
                                           _MC_UNIT_DEFRAG_BY_RES);
                        unitData->defragAttRes++;
                    }
                }
            }
        } else { /* if (mvrData->logical) */
            /* physical mode; only OITT */
            if (mvrData->ptrChanged) {
                result = _bcm_sirius_multicast_oitt_write(unitData,
                                                          &(mvrData->base),
                                                          mvrData->targets,
                                                          mvrData->ptr.oitt,
                                                          mvrData->queuePtr,
                                                          mvrData->noTrans);
                /* called function already displayed any errors */
                /* if out of resources, trigger a defrag */
                if (BCM_E_RESOURCE == result) {
                    unitData->flags = (unitData->flags &
                                       (~_MC_UNIT_DEFRAG_BY_MASK)) |
                                      (_MC_UNIT_DEFRAG_START |
                                       _MC_UNIT_DEFRAG_BY_RES);
                    unitData->defragAttRes++;
                }
            } /* if (mvrData->ptrChanged) */
        } /* if (mvrData->logical) */
    } /* if ((BCM_E_NONE == result) && mvrData->targets) */
    if (BCM_E_NONE == result) {
        /* write the MVR */
        if (mvrData->changed) {
            result = _bcm_sirius_multicast_mvr_write(unitData,
                                                     &mvr,
                                                     mvrData);
            /* called function already displayed any errors */
            /* if out of resources, trigger a defrag */
            if (BCM_E_RESOURCE == result) {
                unitData->flags = (unitData->flags &
                                   (~_MC_UNIT_DEFRAG_BY_MASK)) |
                                  (_MC_UNIT_DEFRAG_START |
                                   _MC_UNIT_DEFRAG_BY_RES);
                unitData->defragAttRes++;
            }
            if (BCM_E_NONE == result) {
                /* write the GMT entry */
                soc_EG_FD_GMTm_field32_set(unitData->unit,
                                           &(gmtData),
                                           MVRPf,
                                           mvr);
                result = soc_mem_write(unitData->unit,
                                       EG_FD_GMTm,
                                       MEM_BLOCK_ALL,
                                       mcGroup,
                                       &gmtData);
                if (BCM_E_NONE != result) {
                    MC_ERR((MC_MSG1("unable to write %d:gmt[%08X]: %d (%s)\n"),
                            unitData->unit,
                            mcGroup,
                            result,
                            _SHR_ERRMSG(result)));
                }
            } /* if (BEM_E_NONE == result) */
        } /* if (mvrData->changed) */
    } /* if (BEM_E_NONE == result) */

    if (BCM_E_NONE == result) {
        /* go back through old version and limbo the disused bits */
        if ((*mvrPtr)->logical) {
            /* logical mode */
            /* stuff old OITT blocks into limbo */
            for (index = 0; index < (*mvrPtr)->targets; index++) {
                if ((*mvrPtr)->ptr.rr[index].changed) {
                    /* we updated this OITT set */
                    if ((unitData->oittMax >= (*mvrPtr)->ptr.rr[index].base) &&
                        (unitData->oittMin <= (*mvrPtr)->ptr.rr[index].base)) {
                        /* it had a valid OITT block */
                        shr_mdb_list_insert(unitData->oittList,
                                            unitData->limbo,
                                            (*mvrPtr)->ptr.rr[index].base);
                        unitData->oittBlocks--;
                    }
                    /* caller frees disused GPORT+OI sets */
                }
                /* clear this target's changed indicator */
                mvrData->ptr.rr[index].changed = FALSE;
            } /* for (index = 0; index < (*mvrPtr)->targets; index++) */
            /* stuff old RR into limbo; free heap memory */
            if (mvrData->ptrChanged) {
                /* we replaced the RR */
                if (_MC_INVALID_BASE != (*mvrPtr)->base) {
                    /* it had a valid MDB block */
                    shr_mdb_list_insert(unitData->mdbList,
                                        unitData->limbo,
                                        (*mvrPtr)->base);
                    unitData->mdbBlocks--;
                    /* it must always have had a valid heap block */
                    sal_free((*mvrPtr)->ptr.rr);
                }
            }
        } else { /* if ((*mvrPtr)->logical) */
            /* physical mode */
            /* stuff old OITT into limbo */
            if (mvrData->ptrChanged &&
                (unitData->oittMax >= (*mvrPtr)->base) &&
                (unitData->oittMin <= (*mvrPtr)->base)) {
                /* we updated it, and it had a valid OITT block */
                shr_mdb_list_insert(unitData->oittList,
                                    unitData->limbo,
                                    (*mvrPtr)->base);
                unitData->oittBlocks--;
            }
        } /* if ((*mvrPtr)->logical) */
        /* stuff old MVR into limbo */
        if (mvrData->changed && oldMvr) {
            /* changed MVR and there was an old one */
            shr_mdb_list_insert(unitData->mdbList,
                                unitData->limbo,
                                oldMvr);
            unitData->mdbBlocks--;
        }
        /* clear 'changed' indicators for the whole group */
        mvrData->ptrChanged = FALSE;
        mvrData->changed = FALSE;
        /* update caller pointer so it references the new version */
        sal_free(*mvrPtr);
        *mvrPtr = mvrData;
    } else { /* if (BCM_E_NONE == result) */
        /* something went wrong; clean up */
        /*
         *  If there was a problem, we can immediately free any resources
         *  allocated during this function.  This is because, in error case,
         *  none of these resources are connected to the forwarding path during
         *  the course of this function, and resources must be aged away only
         *  once they have been connected to the forwarding path.
         */
        if (mvrData) {
            /* we have a new MVR descriptor copy */
            if (mvrData->logical) {
                /* logical mode */
                /* get rid of new RR space */
                if (mvrData->ptrChanged &&
                    (_MC_INVALID_BASE != mvrData->base)) {
                    /* we have allocated RR space */
                    shr_mdb_free(unitData->mdbList, mvrData->base);
                    unitData->mdbBlocks--;
                }
                /* get rid of new RR descriptor */
                if (mvrData->ptr.rr) {
                    /* we have RR descriptor */
                    for (index = 0; index < mvrData->targets; index++) {
                        if (mvrData->ptr.rr[index].changed &&
                            mvrData->ptr.rr[index].oittPtr &&
                            (_MC_INVALID_BASE != mvrData->ptr.rr[index].base)) {
                            /* we have allocated OITT space; get rid of it */
                            shr_mdb_free(unitData->oittList,
                                         mvrData->ptr.rr[index].base);
                            unitData->oittBlocks--;
                        }
                    }
                    sal_free(mvrData->ptr.rr);
                } /* if (mvrData->ptr.rr) */
            } else { /* if (mvrData->logical) */
                /* physical mode */
                /* get rid of OITT space */
                if (mvrData->ptrChanged &&
                    mvrData->ptr.oitt &&
                    (_MC_INVALID_BASE != mvrData->base)) {
                    /* we have allocated OITT space */
                    shr_mdb_free(unitData->oittList, mvrData->base);
                    unitData->oittBlocks--;
                }
            } /* if (mvrData->logical) */
            if (mvrData->changed && (_MC_INVALID_BASE != mvr)) {
                /* get rid of new MVR space */
                shr_mdb_free(unitData->mdbList, mvr);
                unitData->mdbBlocks--;
            }
            /* get rid of new MVR descriptor */
            sal_free(mvrData);
        } /* if (mvrData) */
    } /* if (BCM_E_NONE == result) */

    MC_VVERB((MC_MSG("(%08X,%08X,*,*) return %d (%s)\n"),
              (uint32)unitData,
              mcGroup,
              result,
              _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    _bcm_sirius_multicast_group_flush
 *  Purpose
 *    Remove all replicants from a group, and maybe destroy the group.
 *  Arguments
 *    (in) _mc_unit_t *unitData = unit information
 *    (in) bcm_multicast_t group = group
 *    (in) _mc_group_flush_mode_t mode = what to do afterward
 *    (in) uint32 newMvr = new MVR to use for replace mode
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Must not REPLACE with newMvr zero; this will destroy the group.
 *    This will not dispose of replacement MVR on error (only old on success).
 */
static int
_bcm_sirius_multicast_group_flush(_mc_unit_t* unitData,
                                  bcm_multicast_t group,
                                  _mc_group_flush_mode_t mode,
                                  uint32 newMvr)
{
    _mc_mvr_internal_t *mvrData;
    _mc_mvr_internal_t mvrTemp;
    eg_fd_gmt_entry_t gmtData;
    uint32 oldMvr;
    unsigned int index = 0;
    int result;

    MC_EVERB((MC_MSG("(%08X,%08X,%s(%d)) enter\n"),
              (uint32)unitData,
              group,
              (_MC_GROUP_FLUSH_DESTROY == mode)?"DESTROY":
              (_MC_GROUP_FLUSH_REPLACE == mode)?"REPLACE":
              (_MC_GROUP_FLUSH_KEEP == mode)?"KEEP":
              "UNKNOWN",
              mode));

    /* make sure the group exists */
    result = _bcm_sirius_multicast_group_check(unitData, group);
    if (BCM_E_NONE != result) {
        /* called function displayed diagnostic */
        return result;
    }
    /* read the existing entry, but skip the OITT read */
    result = _bcm_sirius_multicast_read_partial(unitData,
                                                group,
                                                &mvrData,
                                                &oldMvr);
    if (BCM_E_NONE != result) {
        /* called function already displayed error diagnostic */
        return result;
    }

    /* get the GMT entry for this group (may write it later) */
    result = soc_mem_read(unitData->unit,
                          EG_FD_GMTm,
                          MEM_BLOCK_ANY,
                          group,
                          &gmtData);
    if (BCM_E_NONE != result) {
        MC_ERR((MC_MSG1("unable to read %d:gmt[%08X]: %d (%s)\n"),
                unitData->unit,
                group,
                result,
                _SHR_ERRMSG(result)));
    }

    /* get new MVRptr based upon destroying the group or not */
    if (BCM_E_NONE == result) {
        switch (mode) {
        case _MC_GROUP_FLUSH_DESTROY:
            /* unlink the group (ignore passed-in MVR) */
            newMvr = 0;
            /* fallthrough intentional (for now) */
        case _MC_GROUP_FLUSH_REPLACE:
            /* unlink the old MVR and use caller provided MVR */
#if defined(BCM_EASY_RELOAD_SUPPORT)
            if (SOC_IS_RELOADING(unitData->unit)) {
                MC_ERR((MC_MSG1("unit %d can not DESTROY or REPLACE groups"
                                " while reloading\n"),
                        unitData->unit));
                result = BCM_E_CONFIG;
            }
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */
            break;
        case _MC_GROUP_FLUSH_KEEP:
            /* keep the group but with no replicants (ignore passed-in MVR) */
#if defined(BCM_EASY_RELOAD_SUPPORT)
            /* handle reload case */
            if (!SOC_IS_RELOADING(unitData->unit)) {
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */
                mvrTemp = *mvrData;
                sal_memset(&(mvrTemp.tsBitmap), 0x00, sizeof(mvrTemp.tsBitmap));
                mvrTemp.base = _MC_INVALID_BASE;
                mvrTemp.changed = TRUE;
                mvrTemp.targets = 0;
                if (mvrTemp.logical) {
                    mvrTemp.ptr.rr = NULL;
                } else {
                    mvrTemp.ptr.oitt = NULL;
                }
                result = _bcm_sirius_multicast_mvr_write(unitData, &newMvr, &mvrTemp);
                /* called function will display diagnostics */
#if defined(BCM_EASY_RELOAD_SUPPORT)
            } /* if (!SOC_IS_RELOADING(unitData->unit)) */
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */
            break;
        default:
            MC_ERR((MC_MSG1("invalid mode %d\n"), mode));
            result = BCM_E_PARAM;
        } /* switch (mode) */
    } /* if (BCM_E_NONE == result) */

#if defined(BCM_EASY_RELOAD_SUPPORT)
    /* handle reload case */
    if (!SOC_IS_RELOADING(unitData->unit)) {
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */
        /* update the GMT entry for this group */
        if (BCM_E_NONE == result) {
            soc_mem_field32_set(unitData->unit,
                                EG_FD_GMTm,
                                &(gmtData),
                                MVRPf,
                                newMvr);
            result = soc_mem_write(unitData->unit,
                                   EG_FD_GMTm,
                                   MEM_BLOCK_ALL,
                                   group,
                                   &gmtData);
            if (BCM_E_NONE != result) {
                MC_ERR((MC_MSG1("unable to write %d:gmt[%08X]: %d (%s)\n"),
                        unitData->unit,
                        group,
                        result,
                        _SHR_ERRMSG(result)));
            }
        } /* if (BCM_E_NONE == result) */
#if defined(BCM_EASY_RELOAD_SUPPORT)
    } /* if (!SOC_IS_RELOADING(unitData->unit)) */
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */

    /* toss old OITT and RR resources if applicable into limbo */
    if ((BCM_E_NONE == result) && (oldMvr != newMvr)) {
#if defined(BCM_EASY_RELOAD_SUPPORT)
        /* handle reload case */
        if (!SOC_IS_RELOADING(unitData->unit)) {
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */
            /* okay so far and the MVR changed */
            if (mvrData->targets) {
                /* has targets, so must have OITT and maybe RR */
                if (mvrData->logical) {
                    /* logical mode */
                    /* toss OITTs into limbo */
                    for (index = 0; index < mvrData->targets; index++) {
                        if (mvrData->ptr.rr[index].base <= unitData->oittMax) {
                            shr_mdb_list_insert(unitData->oittList,
                                                unitData->limbo,
                                                mvrData->ptr.rr[index].base);
                            unitData->oittBlocks--;
                        }
                    }
                    /* toss RR into limbo */
                    shr_mdb_list_insert(unitData->mdbList,
                                        unitData->limbo,
                                        mvrData->base);
                    unitData->mdbBlocks--;
                } else { /* if (mvrData->logical) */
                    /* physical mode */
                    /* toss OITT into limbo */
                    if (mvrData->base <= unitData->oittMax) {
                        shr_mdb_list_insert(unitData->oittList,
                                            unitData->limbo,
                                            mvrData->base);
                        unitData->oittBlocks--;
                    }
                } /* if (mvrData->logical) */
            } /* if (mvrData->targets) */
            /* toss the old MVR into limbo */
            shr_mdb_list_insert(unitData->mdbList,
                                unitData->limbo,
                                oldMvr);
            unitData->mdbBlocks--;
            /* free the group ID if destroying the group */
            if (0 == newMvr) {
                if (group <= unitData->gmtGroupMax) {
                    /* new MVR is zero, group is multicast; free group */
                    shr_idxres_list_free(unitData->gmtList, group);
                } else {
                    /* new MVR is zero, group is unicast; free group */
                    SHR_BITCLR(unitData->unicast, (group - (unitData->gmtGroupMax + 1)));
                }
            }
#if defined(BCM_EASY_RELOAD_SUPPORT)
        } /* if (!SOC_IS_RELOADING(unitData->unit)) */
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */
    } /* if ((BCM_E_NONE == result) && (oldMvr != newMvr)) */

    /* dispose of working heap space */
    _bcm_sirius_multicast_mvrdata_free(mvrData);

    MC_EVERB((MC_MSG("(%08X,%08X,%s(%d)) return %d (%s)\n"),
              (uint32)unitData,
              group,
              (_MC_GROUP_FLUSH_DESTROY == mode)?"DESTROY":
              (_MC_GROUP_FLUSH_REPLACE == mode)?"REPLACE":
              (_MC_GROUP_FLUSH_KEEP == mode)?"KEEP":
              "UNKNOWN",
              mode,
              result,
              _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    _bcm_sirius_multicast_group_get
 *  Purpose
 *    Get certain properties of a group (type, knockout, &c).  This is modelled
 *    so it will return something that can be passed out to the caller for the
 *    bcm_multicast_group_get implementation.
 *  Arguments
 *    (in) int unit = the unit
 *    (in) bcm_multicast_t group = group ID
 *    (out) uint32 *flags = where to put the flags
 *  Return
 *    bcm_error_t cast as int
 *    BCM_E_NONE if successful
 *    BCM_E_* otherwise as appropriate
 *  Notes
 *    Assumes all arguments are valid.
 */
int
_bcm_sirius_multicast_group_get(_mc_unit_t *unitData,
                                bcm_multicast_t group,
                                uint32 *flags)
{
    _mc_mvr_internal_t mvrData;
    eg_fd_gmt_entry_t gmtData;
    uint32 mvr = ~0;
    uint32 working = BCM_MULTICAST_WITH_ID;
    int result;

#if 0 
    MC_EVERB((MC_MSG("(%08X,%08X,*) enter\n"),
              (uint32)unitData,
              group));
#endif 

    /* make sure the group exists */
    result = _bcm_sirius_multicast_group_check(unitData, group);

    /* get the GMT entry */
    if (BCM_E_NONE == result) {
        result = soc_mem_read(unitData->unit,
                              EG_FD_GMTm,
                              MEM_BLOCK_ANY,
                              group,
                              &gmtData);
        if (BCM_E_NONE == result) {
            /* parse the entry */
            mvr = soc_EG_FD_GMTm_field32_get(unitData->unit,
                                             &(gmtData),
                                             MVRPf);
        } else { /* if (BCM_E_NONE == result) */
            MC_ERR((MC_MSG1("unable to read %d:gmt[%08X]: %d (%s)\n"),
                    unitData->unit,
                    group,
                    result,
                    _SHR_ERRMSG(result)));
        } /* if (BCM_E_NONE == result) */
    } /* if (BCM_E_NONE == result) */
    if (BCM_E_NONE == result) {
        result = _bcm_sirius_multicast_mvr_read(unitData, mvr, &mvrData);
        if (BCM_E_NONE == result) {
            if (!(mvrData.sourceKnockout)) {
                /* source knockout is disabled */
                working |= BCM_MULTICAST_DISABLE_SRC_KNOCKOUT;
            }
            
            if (mvrData.logical) {
                /* logical, claim it's L3 */
                working |= BCM_MULTICAST_TYPE_L3;
            } else {
                /* physical, claim it's L2 */
                working |= BCM_MULTICAST_TYPE_L2;
            }
            *flags = working;
        } else { /* if (BCM_E_NONE == result) */
            MC_ERR((MC_MSG1("unable to read %d:mdb[%08X].mvr: %d (%s)\n"),
                    unitData->unit,
                    mvr,
                    result,
                    _SHR_ERRMSG(result)));
        } /* if (BCM_E_NONE == result) */
    } /* if (BCM_E_NONE == result) */

#if 0 
    MC_EVERB((MC_MSG("(%08X,%08X,&(%08X)) return %d (%s)\n"),
              (uint32)unitData,
              group,
              *flags,
              result,
              _SHR_ERRMSG(result)));
#endif 
    return result;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *  Manipulation of hardware based upon BCM view
 */

/*
 *  Function
 *    _bcm_sirius_multicast_target_scan
 *  Purpose
 *    Scan for the index of a particular target within a group.
 *  Arguments
 *    (in) _mc_mvr_internal_t *mvrData = pointer to MVR data
 *    (in) unsigned int targetSet = target to find
 *    (out) unsigned int *index = where to put index for the target
 *  Return
 *    BCM_E_NONE if found the specified target
 *    BCM_E_NOT_FOUND if the specified target was not in the bitmap
 *  Notes
 *    Sets value at index even if not found; in the not found case, the value
 *    is where the target *should* be.
 */
static int
_bcm_sirius_multicast_target_scan(_mc_mvr_internal_t* mvrData,
                                  unsigned int targetSet,
                                  unsigned int *index)
{
    unsigned int tsIndex;
    unsigned int tNumber = 0;
    int result = BCM_E_NOT_FOUND;

    for (tsIndex = 0; tsIndex < _SIRIUS_MC_MAX_TARGETS; tsIndex++) {
        if (SHR_BITGET(mvrData->tsBitmap, tsIndex)) {
            if (tsIndex >= targetSet) {
                if (tsIndex == targetSet) {
                    /* we found the desired target */
                    result = BCM_E_NONE;
                }
                /* we found it or are where it should be; stop now */
                break;
            }
            tNumber++;
        } /* if (SHR_BITGET(mvrData->tsBitmap, qsIndex)) */
    } /* for (qsIndex = 0; qsIndex < _MC_MAX_QSETS; qsIndex++) */
    *index = tNumber;
    return result;
}

/*
 *  Function
 *    _bcm_sirius_multicast_create_id
 *  Purpose
 *    Create specific multicast group.  Basically treats group as physical or
 *    logical replication, which is decided based upon some of the flags (L2 is
 *    physical, everything else is logical; physical only allows one replicant
 *    per interface, while logical allows up to 4096).
 *  Arguments
 *    (in) int unit = the unit
 *    (in) uint32 flags = the flags for this group
 *    (in) bcm_multicast_t group = group ID
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Does not take lock or check group ID.  Ignores 'WITH_ID' bit and always
 *    tries to create the specified ID, assuming that the ID has already been
 *    claimed appropriately.
 *
 *    We allocate a zero-target MVR to store the logical/physical and source
 *    knockout modes.  It'll be replaced later as needed.
 */
static int
_bcm_sirius_multicast_create_id(_mc_unit_t *unitData,
                                uint32 flags,
                                bcm_multicast_t group)
{
    _mc_mvr_internal_t *mvrData = NULL;
    int result = BCM_E_NONE;

    MC_VVERB((MC_MSG("(%08X(%d),%08X,%08X) enter\n"),
              (uint32)unitData,
              unitData->unit,
              flags,
              group));

#if defined(BCM_EASY_RELOAD_SUPPORT)
    /* handle reload case */
    if (SOC_IS_RELOADING(unitData->unit)) {
        uint32 mvrAddr;
        /* get the current values */
        result = _bcm_sirius_multicast_read_partial(unitData,
                                                    group,
                                                    &mvrData,
                                                    &mvrAddr);
        if (BCM_E_NONE == result) {
            /* check the current values */
            if (!(flags & BCM_MULTICAST_DISABLE_SRC_KNOCKOUT) ==
                !(mvrData->sourceKnockout)) {
                MC_ERR((MC_MSG1("inconsistent source knockout disable during"
                                " reload unit %d group %08X: was %s now %s\n"),
                        unitData->unit,
                        group,
                        !(mvrData->sourceKnockout)?"TRUE":"FALSE",
                        !(!(flags & BCM_MULTICAST_DISABLE_SRC_KNOCKOUT))?"TRUE":"FALSE"));
                result = BCM_E_CONFIG;
            }
            if (((flags & BCM_MULTICAST_TYPE_MASK) &&
                 (BCM_MULTICAST_TYPE_L2 != (flags & BCM_MULTICAST_TYPE_MASK))) ==
                !(mvrData->logical)) {
                MC_ERR((MC_MSG1("inconsistent group type during reload"
                                " unit %d group %08X: was %s now %s\n"),
                        unitData->unit,
                        group,
                        (mvrData->logical)?"logical":"physical",
                        ((flags & BCM_MULTICAST_TYPE_MASK) && (BCM_MULTICAST_TYPE_L2 != (flags & BCM_MULTICAST_TYPE_MASK)))?"logical":"physical"));
                result = BCM_E_CONFIG;
            }
        } /* if (BCM_E_NONE == result) */
    } else { /* if (SOC_IS_RELOADING(unitData->unit)) */
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */
        mvrData = sal_alloc(sizeof(*mvrData), "MVR workspace");
        if (!mvrData) {
            MC_ERR((MC_MSG1("unable to allocate %d bytes for unit %d"
                            " MVR workspace\n"),
                    sizeof(*mvrData),
                    unitData->unit));
            return BCM_E_MEMORY;
        }

        /* create a multicast group with no targets */
        if (BCM_E_NONE == result) {
            sal_memset(mvrData, 0x00, sizeof(*mvrData));
            mvrData->base = _MC_INVALID_BASE;
            if (!(flags & BCM_MULTICAST_DISABLE_SRC_KNOCKOUT)) {
                /* disable source knockout not requested; don't disable it */
                mvrData->sourceKnockout = TRUE;
            }
            if ((flags & BCM_MULTICAST_TYPE_MASK) &&
                (BCM_MULTICAST_TYPE_L2 != (flags & BCM_MULTICAST_TYPE_MASK))) {
                /* not L2 (specifically something else); set logical mode */
                mvrData->logical = TRUE;
            }
            mvrData->changed = TRUE;
            result = _bcm_sirius_multicast_write(unitData,
                                                 group,
                                                 &mvrData);
        } /* if (BCM_E_NONE == result) */
#if defined(BCM_EASY_RELOAD_SUPPORT)
    } /* if (SOC_IS_RELOADING(unitData->unit)) */
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */

    /* get rid of temporary stuff */
    if (mvrData) {
        _bcm_sirius_multicast_mvrdata_free(mvrData);
    }

    MC_VVERB((MC_MSG("(%08X(%d),%08X,%08X) return %d (%s)\n"),
              (uint32)unitData,
              unitData->unit,
              flags,
              group,
              result,
              _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    _bcm_sirius_multicast_egress_add_override_oi
 *  Purpose
 *    Add a GPORT,OI pair to an existing multicast group, using the specified
 *    OI translation mode instead of the current OI translation mode.
 *  Arguments
 *    (in) _mc_unit_t *unitData = pointer to the unit information
 *    (in) bcm_multicast_t group = group ID
 *    (in) bcm_gport_t port = gport to add
 *    (in) bcm_if_t encap_id = OI for the gport
 *    (in) bcm_gport_t queue_id = queue for the gport
 *    (in) int use_oi = TRUE if using OI translation, FALSE if not
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Does not claim lock or check unit validity.
 *
 *    Will not allow multiple references to the same target in physical mode.
 *
 *    Does not care about multiple references to the same target in logical
 *    mode, even if the OIs are identical (allows all).
 *
 *    OI translation in hardware is sticky per target -- if a target already
 *    exists, then its OI translation setting overrides the provided one.  If
 *    not, the provided translation setting is used.
 *
 *    Does not sort the added GPORT,OI tuples in the cache.
 *
 *    Automatically disables OITT for XGS mode higig targets in logical groups;
 *    maintains user statement for OITT mode in all other cases.
 */
static int
_bcm_sirius_multicast_egress_add_override_oi(_mc_unit_t *unitData,
                                             bcm_multicast_t group,
                                             bcm_gport_t port,
                                             bcm_if_t encap_id,
                                             bcm_gport_t queue_id,
                                             int use_oi)
{
#if defined(BCM_EASY_RELOAD_SUPPORT)
    _mc_tbmp cacheTargets;                  /* cache targets */
    _mc_tbmp tempTargets;                   /* for building cache targets */
    unsigned int index;                     /* for merging cache targets */
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */
    _mc_tbmp targets;                       /* targets for the GPORT */
    _mc_mvr_internal_t *mvrData = NULL;     /* working MVR data pointer */
    _mc_rr_elem_internal_t *rr;             /* working RR data pointer */
    _gpoiCacheData_t *ncache;               /* temporary new cache entry */
    uint32 *oiBuff;                         /* temporary OITT work buffer */
    uint32 *queueBuff = NULL;               /* temporary queue work buffer */
    int result;                             /* working result */
    unsigned int target;                    /* target number (0..131) */
    unsigned int tgtIndex;                  /* target index in group */
    int orgTgt;                             /* original target */
    unsigned int tgtCount;                  /* number of targets */
    unsigned int hga;                       /* higig aggregate ID */
    unsigned int hgaIndex = ~0;             /* index of cache higig aggregate */
    bcm_module_t myModule;                  /* local module ID */
    int loc_oi;                             /* local OITT use */

    MC_VVERB((MC_MSG("(%08X(%d),%08X,%08X,%08X,%08X,%s) enter\n"),
              (uint32)unitData,
              unitData->unit,
              group,
              port,
              encap_id,
              queue_id,
              use_oi?"TRUE":"FALSE"));

    /* make sure the group exists */
    result = _bcm_sirius_multicast_group_check(unitData, group);
    if (BCM_E_NONE != result) {
        /* called function displayed error diagnostic */
        return result;
    }

    /* get the local module ID */
    result = bcm_stk_my_modid_get(unitData->unit, &myModule);
    if (BCM_E_NONE != result) {
        MC_ERR((MC_MSG1("unable to get unit %d module ID: %d (%s)\n"),
                unitData->unit,
                result,
                _SHR_ERRMSG(result)));
        return result;
    }

    /* get the target information */
    result = _bcm_sirius_multicast_gport_translate_bitmap(unitData,
                                                          myModule,
                                                          port,
                                                          group <= unitData->gmtGroupMax,
                                                          &targets,
                                                          &tgtCount,
                                                          &orgTgt,
                                                          &hga);
    if (BCM_E_NONE != result) {
        /* called function displayed error diagnostic */
        return result;
    }
    tgtCount &= _MC_CACHE_EXTRA_COUNT_MASK;
    orgTgt &= _MC_CACHE_EXTRA_GPORT_MASK;
    hga &= _MC_CACHE_EXTRA_HGA_MASK;
    if (hga) {
        /* don't bother using OITT for XGS higig ports or aggregates */
        loc_oi = FALSE;
    } else {
        /* use OITT according to user mode/statement for other stuff */
        loc_oi = use_oi;
    }
    if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
        MC_EVERB((MC_MSG1("unit %d group %08X add GPORT %08X EI %08X Q %08X "
                          _MC_TBMP_FORMAT
                          " %02X%02X%02X\n"),
                  unitData->unit,
                  group,
                  port,
                  encap_id,
                  queue_id,
                  _MC_TBMP_DISPLAY(targets),
                  hga,
                  tgtCount,
                  orgTgt));
    } else {
        MC_EVERB((MC_MSG1("unit %d group %08X add GPORT %08X EI %08X "
                          _MC_TBMP_FORMAT
                          " %02X%02X%02X\n"),
                  unitData->unit,
                  group,
                  port,
                  encap_id,
                  _MC_TBMP_DISPLAY(targets),
                  hga,
                  tgtCount,
                  orgTgt));
    }

    /* build a new copy of the cache with the new gport,OI tuple added */
    ncache = sal_alloc(sizeof(*ncache) * (unitData->cache[group].elements + 1),
                       "MC group GPORT+OI member cache");
    if (ncache) {
        if (unitData->cache[group].elements) {
            /* not initial element; copy old data */
            sal_memcpy(ncache,
                       unitData->cache[group].data,
                       sizeof(*ncache) * unitData->cache[group].elements);
        }
        ncache[unitData->cache[group].elements].encap_id = encap_id;
        ncache[unitData->cache[group].elements].queue_id = queue_id;
        ncache[unitData->cache[group].elements].port = port;
        ncache[unitData->cache[group].elements].extras = (orgTgt |
                                                          (tgtCount << _MC_CACHE_EXTRA_COUNT_SHIFT) |
                                                          (hga << _MC_CACHE_EXTRA_HGA_SHIFT));
        sal_memcpy(&(ncache[unitData->cache[group].elements].tbmp),
                   &targets,
                   sizeof(ncache[unitData->cache[group].elements].tbmp));
    } else {
        MC_ERR((MC_MSG1("unable to allocate %d bytes for new group"
                        " membership cache entry\n"),
                sizeof(*ncache) * (unitData->cache[group].elements + 1)));
        return BCM_E_MEMORY;
    }

    /* read the current state of the group */
    result = _bcm_sirius_multicast_read(unitData, group, &mvrData);

#if defined(BCM_EASY_RELOAD_SUPPORT)
    if ((BCM_E_NONE == result) && (SOC_IS_RELOADING(unitData->unit))) {
        sal_memset(&cacheTargets, 0x00, sizeof(cacheTargets));
        /*
         *  We now have group state, but the later checks will cause problems
         *  if trying to add a target that is already in the hardware group,
         *  but not in the cache.  This condition is possible during reload.
         *  While in reload, we build the target set from the cache.
         */
        for (tgtIndex = 0;
             (BCM_E_NONE == result) && (tgtIndex < unitData->cache[group].elements);
             tgtIndex++) {
            result = _bcm_sirius_multicast_gport_translate_bitmap(unitData,
                                                                  myModule,
                                                                  unitData->cache[group].data[tgtIndex].port,
                                                                  group <= unitData->gmtGroupMax,
                                                                  &tempTargets,
                                                                  NULL,
                                                                  NULL,
                                                                  NULL);
            if (BCM_E_NONE == result) {
                /* Got the bitmap for this target; merge to cache. */
                for (index = 0;
                     index < (sizeof(_mc_tbmp) / sizeof(SHR_BITDCL));
                     index++) {
                    cacheTargets[index] |= tempTargets[index];
                } /* for (all SHR_BITDCL in cacheTargets) */
            } /* if (BCM_E_NONE == result) */
        } /* for (all elements in cache) */
        /* Also make sure that the new target vector is in the hardware copy */
        for (index = 0;
             (BCM_E_NONE == result) && (index < (sizeof(_mc_tbmp) / sizeof(SHR_BITDCL)));
             index++) {
            if (targets[index] !=
                (targets[index] & mvrData->tsBitmap[index])) {
                MC_ERR((MC_MSG1("new element contains targets not in hardware"
                                " during reload\n")));
                result = BCM_E_CONFIG;
            } /* if (targets has any bits not set in targets) */
            if (cacheTargets[index] !=
                (cacheTargets[index] & mvrData->tsBitmap[index])) {
                MC_ERR((MC_MSG1("cache contains targets not in hardware"
                                " during reload\n")));
                result = BCM_E_CONFIG;
            } /* if (cacheTargets has any bits not set in targets) */
        } /* for (all SHR_BITDCL in targets as long as okay) */
        if (BCM_E_NONE == result) {
            /*
             *  Now use the error checking below as it was meant to be used, by
             *  providing the *cache* targets rather than the hardware copy.
             */
            MC_EVERB((MC_MSG1("hdwr  = " _MC_TBMP_FORMAT "\n"),
                      _MC_TBMP_DISPLAY(mvrData->tsBitmap)));
            MC_EVERB((MC_MSG1("cache = " _MC_TBMP_FORMAT "\n"),
                      _MC_TBMP_DISPLAY(cacheTargets)));
            MC_EVERB((MC_MSG1("new   = " _MC_TBMP_FORMAT "\n"),
                      _MC_TBMP_DISPLAY(targets)));
            sal_memcpy(&(mvrData->tsBitmap),
                       &cacheTargets,
                       sizeof(mvrData->tsBitmap));
            MC_EVERB((MC_MSG1("using = " _MC_TBMP_FORMAT "\n"),
                      _MC_TBMP_DISPLAY(mvrData->tsBitmap)));
        } else {
            MC_ERR((MC_MSG1("hdwr  = " _MC_TBMP_FORMAT "\n"),
                    _MC_TBMP_DISPLAY(mvrData->tsBitmap)));
            MC_ERR((MC_MSG1("cache = " _MC_TBMP_FORMAT "\n"),
                    _MC_TBMP_DISPLAY(cacheTargets)));
            MC_ERR((MC_MSG1("new   = " _MC_TBMP_FORMAT "\n"),
                    _MC_TBMP_DISPLAY(targets)));
        }
    } /* if ((BCM_E_NONE == result) && (SOC_IS_RELOADING(unitData->unit))) */
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */

    /*
     *  The new <gport,OI> must not duplicate one that is already in the group.
     *  This much seems to be prohibited by the API spec, and applies to both
     *  logical and physical groups.
     *
     *  For logical groups, the new <target,OI> must not duplicate one that is
     *  already in the group.  The API definition sort of implies this, a
     *  little, in some places.
     *
     *  For GPORTs that refer to 'front panel' targets that have been remapped
     *  to XGS mode 'higig' target groups, they must only be considered based
     *  upon the <gport,OI> for membership, but only the first instance of such
     *  a GPORT for a particular 'higig' target group can appear in the
     *  replicants list (even if multiple OIs).  This is due to the fact that
     *  this configuration only applies to XGS devices, which will perform
     *  replication downstream from a single replicant sent by the Sirius.
     *
     *  Also important is that in the case of 'front panel' targets that have
     *  been remapped to XGS mode 'higig' target groups, all of such GPORTs for
     *  a particular 'higig' target group MUST have the same targets in the
     *  cache (so remove can deal with it properly) and so once such a GPORT is
     *  added, any other GPORTs referring to that higig target group will use
     *  the same targets even if the mapping is changed.  The 'set' function
     *  would override this limitation, as it recreates the entire group from
     *  scratch and will therefore use the current map, even if the group had
     *  an old map before.
     */
    if (BCM_E_NONE == result) {
        for (target = 0;
             target < unitData->cache[group].elements;
             target++) {
            if ((unitData->cache[group].data[target].port == port) &&
                (unitData->cache[group].data[target].encap_id == encap_id)) {
                /* same <GPORT,OI> multiple times is always prohibited */
                result = BCM_E_EXISTS;
                MC_ERR((MC_MSG1("unit %d group %08X already has identical"
                                " member gport %08X EI %08X\n"),
                        unitData->unit,
                        group,
                        port,
                        encap_id));
                break;
            }
            if ((0 == hga) &&
                ((unitData->cache[group].data[target].extras & _MC_CACHE_EXTRA_GPORT_MASK) == orgTgt) &&
                ((unitData->cache[group].data[target].encap_id == encap_id) ||
                 (!(mvrData->logical)))) {
                /*
                 *  For logical groups:
                 *
                 *  In SBX mode, same <target,OI> multiple times is prohibited.
                 *
                 *  In XGS mode, same <target,OI> is allowed as long as not
                 *  same <gport,OI>, which was checked above.  We will
                 *  eliminate unwanted duplicate replicants below.
                 *
                 *
                 *  For physical groups:
                 *
                 *  In SBX mode, same <target> multiple times is prohibited.
                 *
                 *  In XGS mode, same <target> is allowed on the assumption
                 *  that the downstream device will balk at any actual
                 *  misconfiguration.  We will eliminate unwanted duplicate
                 *  replicants below.
                 *
                 *
                 *  In neither case do we really care about remote targets.
                 */
                result = BCM_E_EXISTS;
                MC_ERR((MC_MSG1("unit %d group %08X already has member gport"
                                " %08X EI %08X on target %d so gport %08X"
                                " EI %08X is a duplicate\n"),
                        unitData->unit,
                        group,
                        unitData->cache[group].data[target].port,
                        unitData->cache[group].data[target].encap_id,
                        orgTgt,
                        port,
                        encap_id));
                break;
            }
            if ((hgaIndex > unitData->cache[group].elements) &&
                (hga) &&
                (((unitData->cache[group].data[target].extras >> _MC_CACHE_EXTRA_HGA_SHIFT) & _MC_CACHE_EXTRA_HGA_MASK) == hga) &&
                ((_SIRIUS_FAB_LAG_COUNT + 1) != hga)) {
                /*
                 *  Found first instance of this entry's higig aggregate; must
                 *  sync this entry to use the existing aggregate targets.
                 *  Also, since another entry that already exists uses this
                 *  higig target, we track that fact so we don't actually make
                 *  hardware changes (only cache).
                 */
                sal_memcpy(&(targets),
                           &(unitData->cache[group].data[target].tbmp),
                           sizeof(targets));
                sal_memcpy(&(ncache[unitData->cache[group].elements].tbmp),
                           &(unitData->cache[group].data[target].tbmp),
                           sizeof(ncache[unitData->cache[group].elements].tbmp));
                hgaIndex = target;
                /* don't break -- keep looking for duplicates */
            }
            if ((hgaIndex > unitData->cache[group].elements) &&
                ((_SIRIUS_FAB_LAG_COUNT + 1) == hga) &&
                ((unitData->cache[group].data[target].extras & _MC_CACHE_EXTRA_GPORT_MASK) == orgTgt) &&
                (((unitData->cache[group].data[target].extras >> _MC_CACHE_EXTRA_HGA_SHIFT) & _MC_CACHE_EXTRA_HGA_MASK) == hga)) {
                /*
                 *  Found first instance of this non-aggregated higig; must not
                 *  make hardware changes (only cache).
                 */
                hgaIndex = target;
                /* don't break -- keep looking for duplicates */
            }
        } /* for (all cached members in this group) */
    } /* if (BCM_E_NONE == result) */

    /*
     *  If we are not adding an existing higig target group to the multicast
     *  group, then we need to update the hardware, by adding each of the
     *  targets indicated by the provided GPORT.
     */
    if (hgaIndex >= unitData->cache[group].elements) {
        for (target = 0;
             (target < _SIRIUS_MC_MAX_TARGETS) && (BCM_E_NONE == result);
             target++) {
            if (SHR_BITGET(targets,target)) {
                /* this target is included in the new entry */
                result = _bcm_sirius_multicast_target_scan(mvrData,
                                                           target,
                                                           &tgtIndex);
                if ((BCM_E_NONE != result) && (BCM_E_NOT_FOUND != result)) {
                    /*
                     *  Had some unexpected error looking for the target in the
                     *  group (that is, something went wrong other than merely
                     *  not finding it).  The target scan displayed the error
                     *  diagnostic in this case.
                     */
                    break;
                }
                /* add the target or OI appropriately */
                if (mvrData->logical) {
                    /* logical mode */
                    if (BCM_E_NOT_FOUND == result) {
                        /* target is not in group; not an error since adding */
                        result = BCM_E_NONE;
                        /* add target to group */
                        SHR_BITSET(mvrData->tsBitmap, target);
                        /* account for newly added targets */
                        _bcm_sirius_multicast_sparse_update(mvrData);
                        /* build a new RR block */
                        rr = sal_alloc(sizeof(*rr) * mvrData->targets,
                                       "RR workspace");
                        if (rr) {
                            /* build OITT block for this target */
                            oiBuff = sal_alloc(sizeof(*oiBuff),
                                               "OITT workspace");
                            if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
                                queueBuff = sal_alloc(sizeof(*queueBuff),
                                                      "queue workspace");
                            }
                            if (oiBuff &&
                                (!(unitData->flags & _MC_UNIT_DUAL_LOOKUP) ||
                                 queueBuff)) {
                                /* add new oitt block */
                                oiBuff[0] = encap_id;
                                if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
                                    queueBuff[0] = queue_id;
                                }
                                /* adjust RR block for this target */
                                if (tgtIndex > 0) {
                                    /* copy lower part of old RR block */
                                    sal_memcpy(&(rr[0]),
                                               &(mvrData->ptr.rr[0]),
                                               sizeof(*rr) * tgtIndex);
                                }
                                rr[tgtIndex].base = _MC_INVALID_BASE;
                                rr[tgtIndex].changed = TRUE;
                                rr[tgtIndex].replicants = 1;
                                rr[tgtIndex].oittPtr = oiBuff;
                                rr[tgtIndex].queuePtr = queueBuff;
                                rr[tgtIndex].noTrans = !loc_oi;
                                if (tgtIndex < (mvrData->targets - 1)) {
                                    /* copy upper part of old block */
                                    sal_memcpy(&(rr[tgtIndex + 1]),
                                               &(mvrData->ptr.rr[tgtIndex]),
                                               sizeof(*rr) * (mvrData->targets -
                                                              tgtIndex -
                                                              1));
                                }
                                /* switch to new RR block */
                                mvrData->ptrChanged = TRUE;
                                if (mvrData->ptr.rr) {
                                    sal_free(mvrData->ptr.rr);
                                }
                                mvrData->ptr.rr = rr;
                            } else { /* if (oiBuff) */
                                /* could not get OITT space; free RR space */
                                sal_free(rr);
                                MC_ERR((MC_MSG1("unable to allocate %d bytes"
                                                " OITT workspace\n"),
                                        sizeof(*oiBuff)));
                                result = BCM_E_MEMORY;
                            } /* if (oiBuff) */
                        } else { /* if (rr) */
                            MC_ERR((MC_MSG1("unable to allocate %d bytes"
                                            " RR workspace\n"),
                                    sizeof(*rr) * mvrData->targets));
                            result = BCM_E_MEMORY;
                        } /* if (rr) */
                    } else { /* if (BCM_E_NOT_FOUND == result) */
                        /* target is already in group */
                        if (_MC_MAX_REPLICANTS <=
                            mvrData->ptr.rr[tgtIndex].replicants) {
                            /* there are too many replicants to add another */
                            MC_ERR((MC_MSG1("unit %d group %08X target %d"
                                            " already has %d replicants;"
                                            " can not add gport %08X EI"
                                            " %08X\n"),
                                    unitData->unit,
                                    group,
                                    target,
                                    mvrData->ptr.rr[tgtIndex].replicants,
                                    port,
                                    encap_id));
                            result = BCM_E_CONFIG;
                        } else { /* if (too many replicants to add another) */
                            /* need to add new OI */
                            oiBuff = sal_alloc(sizeof(*oiBuff) * (mvrData->ptr.rr[tgtIndex].replicants + 1),
                                               "OITT workspace");
                            if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
                                queueBuff = sal_alloc(sizeof(*queueBuff) * (mvrData->ptr.rr[tgtIndex].replicants + 1),
                                                   "queue workspace");
                            }
                            if (oiBuff &&
                                (!(unitData->flags & _MC_UNIT_DUAL_LOOKUP) ||
                                 queueBuff)) {
                                /* copy lower part of OITT block */
                                sal_memcpy(oiBuff,
                                           mvrData->ptr.rr[tgtIndex].oittPtr,
                                           sizeof(*oiBuff) * mvrData->ptr.rr[tgtIndex].replicants);
                                /* place new replicant in OITT; copy remainder */
                                oiBuff[mvrData->ptr.rr[tgtIndex].replicants] = encap_id;
                                /* get rid of the old OITT data */
                                sal_free(mvrData->ptr.rr[tgtIndex].oittPtr);
                                /* point to updated OITT data */
                                mvrData->ptr.rr[tgtIndex].oittPtr = oiBuff;
                                if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
                                    /* copy lower part of queue block */
                                    sal_memcpy(queueBuff,
                                               mvrData->ptr.rr[tgtIndex].queuePtr,
                                               sizeof(*queueBuff) * mvrData->ptr.rr[tgtIndex].replicants);
                                    /* place new replicant in queue; copy remainder */
                                    queueBuff[mvrData->ptr.rr[tgtIndex].replicants] = queue_id;
                                    /* get rid of the old queue data */
                                    sal_free(mvrData->ptr.rr[tgtIndex].queuePtr);
                                    /* point to update queue data */
                                    mvrData->ptr.rr[tgtIndex].queuePtr = queueBuff;
                                }
                                /* update replicant count for target in RR block */
                                mvrData->ptr.rr[tgtIndex].replicants++;
                                /* mark target replicant list changed */
                                mvrData->ptr.rr[tgtIndex].changed = TRUE;
                            } else { /* if (oitt) */
                                MC_ERR((MC_MSG1("unable to allocate %d bytes"
                                                " OITT workspace\n"),
                                        sizeof(*oiBuff) *
                                        (mvrData->ptr.rr[tgtIndex].replicants + 1)));
                                result = BCM_E_MEMORY;
                            } /* if (oitt) */
                        } /* if (too many replicants to add another) */
                    } /* if (BCM_E_NOT_FOUND == result) */
                } else { /* if (mvrData->logical) */
                    /* physical mode */
                    if (BCM_E_NOT_FOUND == result) {
                        /* target is not in group, but not an error since adding */
                        result = BCM_E_NONE;
                        /* add target to group */
                        SHR_BITSET(mvrData->tsBitmap, target);
                        /* account for newly added target */
                        _bcm_sirius_multicast_sparse_update(mvrData);
                        /* build new OITT block for this group */
                        oiBuff = sal_alloc(sizeof(*oiBuff) * mvrData->targets,
                                           "OITT workspace");
                        if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
                            queueBuff = sal_alloc(sizeof(*queueBuff) * mvrData->targets,
                                                  "queue workspace");
                        }
                        if (oiBuff &&
                            (!(unitData->flags & _MC_UNIT_DUAL_LOOKUP) ||
                             queueBuff)) {
                            if (tgtIndex > 0) {
                                /* copy lower part of old OITT block */
                                sal_memcpy(&(oiBuff[0]),
                                           &(mvrData->ptr.oitt[0]),
                                           sizeof(*oiBuff) * tgtIndex);
                            }
                            oiBuff[tgtIndex] = encap_id;
                            if (tgtIndex < (mvrData->targets - 1)) {
                                /* copy upper part of old block */
                                sal_memcpy(&(oiBuff[tgtIndex + 1]),
                                           &(mvrData->ptr.oitt[tgtIndex]),
                                           sizeof(*oiBuff) * (mvrData->targets -
                                                              tgtIndex -
                                                              1));
                            }
                            if (1 == mvrData->targets) {
                                /* set use OI feature if this is first target */
                                mvrData->noTrans = !use_oi;
                            }
                            /* get rid of old OITT block */
                            if (mvrData->ptr.oitt) {
                                sal_free(mvrData->ptr.oitt);
                            }
                            /* switch to the new OITT block */
                            mvrData->ptr.oitt = oiBuff;
                            if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
                                if (tgtIndex > 0) {
                                    /* copy lower part of old queue block */
                                    sal_memcpy(&(queueBuff[0]),
                                               &(mvrData->queuePtr[0]),
                                               sizeof(*queueBuff) * tgtIndex);
                                }
                                queueBuff[tgtIndex] = queue_id;
                                if (tgtIndex < (mvrData->targets - 1)) {
                                    /* copy upper part of old block */
                                    sal_memcpy(&(queueBuff[tgtIndex + 1]),
                                               &(mvrData->queuePtr[tgtIndex]),
                                               sizeof(*queueBuff) * (mvrData->targets -
                                                                     tgtIndex -
                                                                     1));
                                }
                                /* get rid of old queue block */
                                if (mvrData->queuePtr) {
                                    sal_free(mvrData->queuePtr);
                                }
                                /* switch to the new OITT block */
                                mvrData->queuePtr = queueBuff;
                            }
                            mvrData->ptrChanged = TRUE;
                        } else { /* if (oitt) */
                            MC_ERR((MC_MSG1("unable to allocate %d bytes"
                                            " OITT workspace\n"),
                                    sizeof(*oiBuff) * mvrData->targets));
                            result = BCM_E_MEMORY;
                        } /* if (oitt) */
                    } else { /* if (BCM_E_NOT_FOUND == result) */
                        /* target is already in group; error in physical mode */
                        MC_ERR((MC_MSG1("port %08X already has target %d in"
                                        " unit %d group %08X\n"),
                                port,
                                target,
                                unitData->unit,
                                group));
                        result = BCM_E_EXISTS;
                    } /* if (BCM_E_NOT_FOUND == result) */
                } /* if (mvrData->logical) */
            } /* if (SHR_BITGET(targets,target)) */
        } /* for (all targets or until error) */

#if defined(BCM_EASY_RELOAD_SUPPORT)
        /* handle reload case */
        if (!SOC_IS_RELOADING(unitData->unit)) {
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */
            /* write the new group values, if hardware change is warranted */
            if (BCM_E_NONE == result) {
                result = _bcm_sirius_multicast_write(unitData,
                                                     group,
                                                     &mvrData);
            }
#if defined(BCM_EASY_RELOAD_SUPPORT)
        } /* if (!SOC_IS_RELOADING(unitData->unit)) */
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */
    } /* if (hgaIndex > unitData->cache[group].elements) */

    /* update the cache */
    if (BCM_E_NONE == result) {
        unitData->cache[group].elements++;
        if (unitData->cache[group].data) {
            sal_free(unitData->cache[group].data);
        } else {
            unitData->cells++;
        }
        unitData->cache[group].data = ncache;
    } else { /* if (BCM_E_NONE == result) */
        if (ncache) {
            sal_free(ncache);
        }
    } /* if (BCM_E_NONE == result) */

    /* clean up resources */
    _bcm_sirius_multicast_mvrdata_free(mvrData);

    MC_VVERB((MC_MSG("(%08X(%d),%08X,%08X,%08X,%08X,%s) return %d (%s)\n"),
              (uint32)unitData,
              unitData->unit,
              group,
              port,
              encap_id,
              queue_id,
              use_oi?"TRUE":"FALSE",
              result,
              _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    _bcm_sirius_multicast_egress_set_override_oi
 *  Purpose
 *    Set an existing multicast group to contain exactly the specified
 *    elements.
 *  Arguments
 *    (in) _mc_unit_t *unitData = the unit information
 *    (in) bcm_multicast_t group = group ID
 *    (in) int port_count = number of ports (why can this be negative?)
 *    (in) bcm_gport_t *port_array = pointer to array of gport IDs
 *    (in) bcm_if_t *encap_id_array = pointer to array of encap_ids
 *    (in) bcm_gport_t *queue_array = pointer to array of queue IDs
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Does not take lock or verify unit.
 *
 *    port_array and encap_id_array elements are paired 1:1.
 *
 *    Replaces entire existing group contents with new contents, effectively
 *    deleting anything not in the new set.  The API doc doesn't actually *say*
 *    this, but it should.
 *
 *    Somebody should explain why this doesn't take an *unsigned* count.  I
 *    don't see any information about what to do with negative count, so this
 *    implementation will call it BCM_E_PARAM.  Zero count is valid (it just
 *    means there are no replicants in the group).  Yes, you can provide NULL
 *    pointers to the arrays in the case of zero elements.  However, it's
 *    probably more efficient to just call the delete_all function instead.
 *
 *    Wants a BIG pile of stack space (_SHR_PBMP_PORT_MAX * (sizeof(bcm_port_t)
 *    + sizeof(_mc_oitt_elem_internal_t)) so it does not have to do heap
 *    thrashing for resizing data.
 *
 *    Will not allow multiple replicants on same target for physical mode.
 *
 *    The OI translation override can be useful in setting up aggregates in
 *    systems that do use OI translation for multicast but do not need it for
 *    unicast.  OI translation should always be disabled for the entire unit
 *    (improves performance and removes a particular resource limitation) on
 *    systems that do not use OIs for multicast or unicast.  Not sure if
 *    there's any good reason to override in the TRUE direction...
 *
 *    Does not sort the added GPORT,OI tuples in the cache.
 *
 *    Automatically disables OITT for XGS mode higig targets in logical groups;
 *    maintains user statement for OITT mode in all other cases.
 */
static int
_bcm_sirius_multicast_egress_set_override_oi(_mc_unit_t *unitData,
                                             bcm_multicast_t group,
                                             int port_count,
                                             bcm_gport_t *port_array,
                                             bcm_if_t *encap_id_array,
                                             bcm_gport_t *queue_id_array,
                                             int use_oi)
{
    unsigned int replicants[_SIRIUS_MC_MAX_TARGETS]; /* reps per target */
    unsigned int replicant[_SIRIUS_MC_MAX_TARGETS];  /* curr rep per target */
    uint32 *oittPtr[_SIRIUS_MC_MAX_TARGETS];         /* OITT data per target */
    uint32 *queuePtr[_SIRIUS_MC_MAX_TARGETS];        /* queue data per target */
    _mc_mvr_internal_t *mvrData = NULL;              /* working MVR data */
    _mc_mvr_internal_t *mvrOrig = NULL;              /* original MVR data */
    _gpoiCacheData_t *ncache = NULL;                 /* new cache entry */
    uint32 mvrAddr = _MC_INVALID_BASE;               /* MVR address */
    unsigned int index0;                             /* working index */
    unsigned int index1;                             /* working index */
    unsigned int tIndex;                             /* target index */
    unsigned int target;                             /* target number */
    bcm_module_t myModule;                           /* local module ID */
    int result;                                      /* working result */
    int orgTgt;                                      /* original target */
    unsigned int tgtCount;                           /* number of targets */
    unsigned int hga;                                /* higig aggregate ID */
    uint8 hgaFound[_SIRIUS_FAB_LAG_COUNT];           /* higig aggregates hit */
    uint8 hgFound[_SIRIUS_FAB_LAG_COUNT];            /* higig XGS singles hit */

    MC_VVERB((MC_MSG("(%08X(%d),%08X,%d,*,*,%s) enter\n"),
              (uint32)unitData,
              unitData->unit,
              group,
              port_count,
              use_oi?"TRUE":"FALSE"));
    /* some quick validation */
    if (((!port_array) || (!encap_id_array)) && (0 < port_count)) {
        MC_ERR((MC_MSG1("NULL pointer to required inbound argument\n")));
        return BCM_E_PARAM;
    }
    if (0 > port_count) {
        MC_ERR((MC_MSG1("I refuse to work with a negative count\n")));
        return BCM_E_PARAM;
    }
    if ((unitData->flags & _MC_UNIT_DUAL_LOOKUP) && (!queue_id_array)) {
        MC_WARN((MC_MSG1("unit %d dual-lookup mode but set without queues\n"),
                 unitData->unit));
    }
    if ((!(unitData->flags & _MC_UNIT_DUAL_LOOKUP) && queue_id_array)) {
        MC_WARN((MC_MSG1("unit %d single-lookup mode but set with queues\n"),
                 unitData->unit));
    }

    /* make sure the group exists */
    result = _bcm_sirius_multicast_group_check(unitData, group);
    if (BCM_E_NONE != result) {
        /* called function displayed error diagnostic */
        return result;
    }

    /* get the local module ID */
    result = bcm_stk_my_modid_get(unitData->unit, &myModule);
    if (BCM_E_NONE != result) {
        MC_ERR((MC_MSG1("unable to get unit %d module ID: %d (%s)\n"),
                unitData->unit,
                result,
                _SHR_ERRMSG(result)));
        return result;
    }

    /* get the current group MVR+RR (for later) */
    result = _bcm_sirius_multicast_read_partial(unitData,
                                                group,
                                                &mvrOrig,
                                                &mvrAddr);
    if (BCM_E_NONE != result) {
        /* called function displayed error diagnostic */
        return result;
    }

    /* copy current MVR for type & mode, but no targets & no replicants */
    mvrData = sal_alloc(sizeof(*mvrData), "MVR workspace");
    if (mvrData) {
        sal_memcpy(mvrData, mvrOrig, sizeof(*mvrData));
        mvrData->base = _MC_INVALID_BASE;
        mvrData->changed = TRUE;
        if (mvrData->logical) {
            mvrData->ptr.rr = NULL;
        } else {
            mvrData->ptr.oitt = NULL;
        }
        mvrData->ptrChanged = TRUE;
        sal_memset(&(mvrData->tsBitmap),
                   0x00,
                   sizeof(mvrData->tsBitmap));
        mvrData->targets = 0;
        /* will update sparse target list and target count later */
    } else { /* if (mvrData) */
        MC_ERR((MC_MSG1("unable to allocate %d bytes MVR workspace\n"),
                sizeof(*mvrData)));
        result = BCM_E_MEMORY;
    } /* if (mvrData) */

    /* need the counts and OITT pointers to be clear initially */
    sal_memset(&(replicants[0]),
               0x00,
               sizeof(replicants[0]) * _SIRIUS_MC_MAX_TARGETS);
    sal_memset(&(replicant[0]),
               0x00,
               sizeof(replicant[0]) * _SIRIUS_MC_MAX_TARGETS);
    sal_memset(&(oittPtr[0]),
               0x00,
               sizeof(oittPtr[0]) * _SIRIUS_MC_MAX_TARGETS);
    sal_memset(&(queuePtr[0]),
               0x00,
               sizeof(queuePtr[0]) * _SIRIUS_MC_MAX_TARGETS);

    /* have not yet found any XGS higig ports or aggregates */
    sal_memset(&(hgFound[0]),
               0x00,
               sizeof(hgFound[0]) * _SIRIUS_FAB_LAG_COUNT);
    sal_memset(&(hgaFound[0]),
               0x00,
               sizeof(hgaFound[0]) * _SIRIUS_FAB_LAG_COUNT);

    /*
     *  Set up a new cache block that will contain all of the specified
     *  GPORT,OI tuples.
     */
    if (BCM_E_NONE == result) {
        if (port_count) {
            ncache = sal_alloc(sizeof(*ncache) * port_count,
                               "MC group GPORT+OI member cache");
            if (!ncache) {
                MC_ERR((MC_MSG1("unable to allocate %d bytes for group"
                                " membership cache\n"),
                        sizeof(*ncache) * port_count));
                result = BCM_E_MEMORY;
            }
        } else { /* if (port_count) */
            ncache = NULL;
        } /* if (port_count) */
    } /* if (BCM_E_NONE == result) */

    /*
     *  Fill in the new cache block, with all of the specified GPORT,OI,queue
     *  tuples, and make sure there is no overlap where there should be none.
     *
     *  At the same time, since we're already having to do some work with the
     *  specified GPORTs, this sets up the target replicant counts so we can
     *  avoid lots of tedious heap work.
     */
    if (BCM_E_NONE == result) {
        for (index0 = 0;
             (index0 < port_count) && (BCM_E_NONE == result);
             index0++) {
            /* Build the cache entry for this GPORT,OI */
            result = _bcm_sirius_multicast_gport_translate_bitmap(unitData,
                                                                  myModule,
                                                                  port_array[index0],
                                                                  group <= unitData->gmtGroupMax,
                                                                  &(ncache[index0].tbmp),
                                                                  &tgtCount,
                                                                  &orgTgt,
                                                                  &hga);
            if (BCM_E_NONE != result) {
                /* called function displayed error diagnostic */
                break;
            }
            tgtCount &= _MC_CACHE_EXTRA_COUNT_MASK;
            orgTgt &= _MC_CACHE_EXTRA_GPORT_MASK;
            hga &= _MC_CACHE_EXTRA_HGA_MASK;
            MC_EVERB((MC_MSG1("unit %d group %08X add GPORT %08X OI %08X "
                              _MC_TBMP_FORMAT
                              " %02X%02X%02X\n"),
                      unitData->unit,
                      group,
                      port_array[index0],
                      encap_id_array[index0],
                      _MC_TBMP_DISPLAY(ncache[index0].tbmp),
                      hga,
                      tgtCount,
                      orgTgt));
            ncache[index0].port = port_array[index0];
            ncache[index0].encap_id = encap_id_array[index0];
            if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
                if (queue_id_array) {
                    ncache[index0].queue_id = queue_id_array[index0];
                } else {
                    ncache[index0].queue_id = 0;
                }
            }
            ncache[index0].extras = (orgTgt |
                                     (tgtCount << _MC_CACHE_EXTRA_COUNT_SHIFT) |
                                     (hga << _MC_CACHE_EXTRA_HGA_SHIFT));
            /* scan the returned targets for the port */
            for (target = 0;
                 (target < _SIRIUS_MC_MAX_TARGETS) &&
                 (BCM_E_NONE == result);
                 target++) {
                if (SHR_BITGET(ncache[index0].tbmp, target)) {
                    /* this target is included by this port */
                    if ((!hga) || (!SHR_BITGET(mvrData->tsBitmap, target))) {
                        /*
                         *  There will be one replicant for a target if that
                         *  target is an XGS higig; otherwise, there can be
                         *  many replicants per target.  Only increment the
                         *  replicant count for an XGS higig if it was not
                         *  already included, but increment the replicant count
                         *  for anything else whenever it gets a new replicant.
                         */
                        replicants[target]++;
                        if (!hga) {
                            /* take user OI statement for non-higig targets */
                            replicant[target] = use_oi;
                        }
                    }
                    /* make sure the target is included in the group */
                    SHR_BITSET(mvrData->tsBitmap, target);
                } /* if (SHR_BITGET(ncache[index0]tbmp, target) */
            } /* for (all possible targets unless error) */
            /*
             *  The new <gport,OI> must not duplicate one that is already in
             *  the group.  This much seems to be prohibited by the API spec,
             *  and applies to both logical and physical groups.
             *
             *  For physical groups, the new <target> must not duplicate one
             *  that is already in the group.  This is prohibited by hardware.
             *  For logical groups, we also prohibit the same <target,OI> tuple
             *  appearing multiple times (though the <gport,OI> should have
             *  already caught that).
             */
            
            
            for (index1 = 0; index1 < index0; index1++) {
                if ((ncache[index1].port == port_array[index0]) &&
                    (ncache[index1].encap_id == encap_id_array[index0])) {
                    /* same <GPORT,OI> multiple times is always prohibited */
                    result = BCM_E_EXISTS;
                    MC_ERR((MC_MSG1("unit %d group %08X already has identical"
                                    " member gport %08X EI %08X at %d\n"),
                            unitData->unit,
                            group,
                            port_array[index0],
                            encap_id_array[index0],
                            index1));
                    break;
                }
                if ((0 == hga) &&
                    ((ncache[index1].extras & _MC_CACHE_EXTRA_GPORT_MASK) == orgTgt) &&
                    ((ncache[index1].encap_id == encap_id_array[index0]) ||
                     (!(mvrData->logical)))) {
                    /*
                     *  For logical groups:
                     *
                     *  In SBX mode, same <target,OI> multiple times is
                     *  prohibited.
                     *
                     *  In XGS mode, same <target,OI> is allowed as long as not
                     *  same <gport,OI>, which was checked above.  We will
                     *  eliminate unwanted duplicate replicants below.
                     *
                     *
                     *  For physical groups:
                     *
                     *  In SBX mode, same <target> multiple times is
                     *  prohibited.
                     *
                     *  In XGS mode, same <target> is allowed on the assumption
                     *  that the downstream device will balk at any actual
                     *  misconfiguration.  We will eliminate unwanted duplicate
                     *  replicants below.
                     */
                    result = BCM_E_EXISTS;
                    MC_ERR((MC_MSG1("unit %d group %08X already has member"
                                    " gport %08X EI %08X on target %d so gport"
                                    " %08X EI %08X is a duplicate\n"),
                            unitData->unit,
                            group,
                            ncache[index1].port,
                            ncache[index1].encap_id,
                            orgTgt,
                            port_array[index0],
                            encap_id_array[index0]));
                    break;
                }
            } /* for (all cached members in this group so far) */
        } /* for (all members in the new set) */
        if (BCM_E_NONE == result) {
            /* account for newly added targets */
            _bcm_sirius_multicast_sparse_update(mvrData);
        } /* if (BCM_E_NONE == result) */
    } /* if (BCM_E_NONE == result) */

    /* allocate RR and OITT workspace as appropriate */
    if ((BCM_E_NONE == result) && mvrData->targets) {
        if (mvrData->logical) {
            /* logical mode */
            mvrData->ptr.rr = sal_alloc(sizeof(*(mvrData->ptr.rr)) *
                                        mvrData->targets,
                                        "RR workspace");
            if (mvrData->ptr.rr) {
                sal_memset(mvrData->ptr.rr,
                           0x00,
                           sizeof(*(mvrData->ptr.rr)) * mvrData->targets);
                index0 = 0;
                for (tIndex = 0; tIndex < _SIRIUS_MC_MAX_TARGETS; tIndex++) {
                    if (SHR_BITGET(mvrData->tsBitmap, tIndex)) {
                        /* set up OI information for each target */
                        /*
                         *  We also shortcut the index issue here by setting up
                         *  all of the applicable OITT information, and then
                         *  keeping a pointer to the OITT space for each of the
                         *  active targets, indexed by target number rather
                         *  than by the 'active target index' as in hardware.
                         */
                        mvrData->ptr.rr[index0].base = _MC_INVALID_BASE;
                        mvrData->ptr.rr[index0].changed = TRUE;
                        mvrData->ptr.rr[index0].replicants = replicants[tIndex];
                        mvrData->ptr.rr[index0].noTrans = !(replicant[tIndex]);
                        mvrData->ptr.rr[index0].oittPtr = sal_alloc(sizeof(*(mvrData->ptr.rr[index0].oittPtr)) *
                                                                    replicants[tIndex],
                                                                    "OITT workspace");
                        replicant[tIndex] = 0;
                        if (mvrData->ptr.rr[index0].oittPtr) {
                            sal_memset(mvrData->ptr.rr[index0].oittPtr,
                                       0xFF,
                                       sizeof(*(mvrData->ptr.rr[index0].oittPtr)) *
                                       replicants[tIndex]);
                            oittPtr[tIndex] = mvrData->ptr.rr[index0].oittPtr;
                        } else { /* if (mvrData->ptr.rr[index0].oittPtr) */
                            MC_ERR((MC_MSG1("unable to allocate %d bytes"
                                            " OITT workspace\n"),
                                    sizeof(*(mvrData->ptr.rr[index0].oittPtr)) *
                                    replicants[tIndex]));
                            result = BCM_E_MEMORY;
                        } /* if (mvrData->ptr.rr[index0].oittPtr) */
                        if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
                            mvrData->ptr.rr[index0].queuePtr = sal_alloc(sizeof(*(mvrData->ptr.rr[index0].queuePtr)) *
                                                                         replicants[tIndex],
                                                                         "queue workspace");
                            if (mvrData->ptr.rr[index0].queuePtr) {
                                sal_memset(mvrData->ptr.rr[index0].queuePtr,
                                           0xFF,
                                           sizeof(*(mvrData->ptr.rr[index0].queuePtr)) *
                                           replicants[tIndex]);
                                queuePtr[tIndex] = mvrData->ptr.rr[index0].queuePtr;
                            } else { /* if (mvrData->ptr.rr[index0].oittPtr) */
                                MC_ERR((MC_MSG1("unable to allocate %d bytes"
                                                " queue workspace\n"),
                                        sizeof(*(mvrData->ptr.rr[index0].queuePtr)) *
                                        replicants[tIndex]));
                                result = BCM_E_MEMORY;
                            } /* if (mvrData->ptr.rr[index0].oittPtr) */
                        } /* if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) */
                        index0++;
                    } /* if (SHR_BITGET(mvrData->tsBitmap, qsIndex)) */
                } /* for (qsIndex = 0; qsIndex < _MC_MAX_QSETS; qsIndex++) */
            } else { /* if (mvrData->ptr.rr) */
                MC_ERR((MC_MSG1("unable to allocate %d bytes RR workspace\n"),
                        sizeof(*(mvrData->ptr.rr)) * mvrData->targets));
                result = BCM_E_MEMORY;
            } /* if (mvrData->ptr.rr) */
        } else { /* if (mvrData->logical) */
            /* physical mode */
            if (use_oi) {
                mvrData->ptr.oitt = sal_alloc(sizeof(*(mvrData->ptr.oitt)) * mvrData->targets,
                                              "OITT workspace");
                if (mvrData->ptr.oitt) {
                    sal_memset(mvrData->ptr.oitt,
                               0xFF,
                               sizeof(*(mvrData->ptr.oitt)) * mvrData->targets);
                    index0 = 0;
                    for (tIndex = 0; tIndex < _SIRIUS_MC_MAX_TARGETS; tIndex++) {
                        if (SHR_BITGET(mvrData->tsBitmap, tIndex)) {
                            /*
                             *  This is really taking a shortcut -- we make one
                             *  pass through the active target list and stick
                             *  pointers to the proper place in the OITT for
                             *  each; this way, we don't have to count offsets
                             *  for each element in the user's list (which may
                             *  not be in strict order).
                             */
                            oittPtr[tIndex] = &(mvrData->ptr.oitt[index0]);
                            replicant[tIndex] = 0;
                            index0++;
                        } /* if (SHR_BITGET(mvrData->tsBitmap, qsIndex)) */
                    } /* for (all targets) */
                } else { /* if (mvrData->ptr.oitt) */
                    MC_ERR((MC_MSG1("unable to allocate %d bytes OITT"
                                    " workspace\n"),
                            sizeof(*(mvrData->ptr.oitt)) * mvrData->targets));
                    result = BCM_E_MEMORY;
                } /* if (mvrData->ptr.oitt) */
                if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
                    mvrData->queuePtr = sal_alloc(sizeof(*(mvrData->queuePtr)) * mvrData->targets,
                                                  "queue workspace");
                    if (mvrData->ptr.oitt) {
                        sal_memset(mvrData->queuePtr,
                                   0xFF,
                                   sizeof(*(mvrData->queuePtr)) * mvrData->targets);
                        index0 = 0;
                        for (tIndex = 0; tIndex < _SIRIUS_MC_MAX_TARGETS; tIndex++) {
                            if (SHR_BITGET(mvrData->tsBitmap, tIndex)) {
                                /*
                                 *  This is really taking a shortcut -- we make
                                 *  one pass through the active target list and
                                 *  stick pointers to the proper place in the
                                 *  OITT for each; this way, we don't have to
                                 *  count offsets for each element in the
                                 *  user's list (which may not be in strict
                                 *  order).
                                 */
                                queuePtr[tIndex] = &(mvrData->queuePtr[index0]);
                                replicant[tIndex] = 0;
                                index0++;
                            } /* if (SHR_BITGET(mvrData->tsBitmap, qsIndex)) */
                        } /* for (all targets) */
                    } else { /* if (mvrData->queuePtr) */
                        MC_ERR((MC_MSG1("unable to allocate %d bytes queue"
                                        " workspace\n"),
                                sizeof(*(mvrData->queuePtr)) *
                                mvrData->targets));
                        result = BCM_E_MEMORY;
                    } /* if (mvrData->queuePtr) */
                } /* if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) */
            } /* if (use_oi) */
            mvrData->noTrans = !use_oi;
        } /* if (mvrData->logical) */
    } /* if ((BCM_E_NONE == result) && mvrData->targets) */

    /* fill in the OITT data */
    if ((BCM_E_NONE == result) && mvrData->targets) {
        /* fill in the OITT space */
        for (index0 = 0;
             (index0 < port_count) && (BCM_E_NONE == result);
             index0++) {
            hga = ((ncache[index0].extras >> _MC_CACHE_EXTRA_HGA_SHIFT) &
                   _MC_CACHE_EXTRA_HGA_MASK);
            orgTgt = (ncache[index0].extras & _MC_CACHE_EXTRA_GPORT_MASK);
            if ((0 == hga) ||
                (((_SIRIUS_FAB_LAG_COUNT + 1) == hga) && (!hgFound[orgTgt])) ||
                ((_SIRIUS_FAB_LAG_COUNT >= hga) && (!hgaFound[hga - 1]))) {
                /*
                 *  Include all replicants that are not 'XGS higig' or 'XGS
                 *  higig aggregate'.
                 *
                 *  Include only the first replicant on each 'XGS higig' port.
                 *
                 *  Include only the first replicant on each 'XGS higig'
                 *  aggregate'.
                 */
                for (target = 0;
                     (target < _SIRIUS_MC_MAX_TARGETS) &&
                     (BCM_E_NONE == result);
                     target++) {
                    if (SHR_BITGET(ncache[index0].tbmp, target)) {
                        if (oittPtr[target]) {
                            /* append the appropriate OI to the target's list */
                            oittPtr[target][replicant[target]] = ncache[index0].encap_id;
                            if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
                                queuePtr[target][replicant[target]] = ncache[index0].queue_id;
                            }
                        }
                        /* update replicants on target */
                        replicant[target]++;
                    } /* if (SHR_BITGET(targets[index0], target)) */
                } /* for (all targets unless error) */
                if (hga) {
                    /* mark 'XGS higig' or 'XGS higig aggregate' as hit */
                    if (_SIRIUS_FAB_LAG_COUNT < hga) {
                        hgFound[orgTgt] = TRUE;
                    } else {
                        hgaFound[hga - 1] = TRUE;
                    }
                }
            } /* if (0 == (hgaFound & (1 << hga))) */
        } /* for (all provided ports unless error) */
    } /* if ((BCM_E_NONE == result) && mvrData->targets) */

#if defined(BCM_EASY_RELOAD_SUPPORT)
    /* handle reload case */
    if (!SOC_IS_RELOADING(unitData->unit)) {
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */
        /* write the new group */
        if (BCM_E_NONE == result) {
            /*
             *  Note that the group is written here with 'everything' flagged
             *  as 'changed' but with all of the MDB and OITT addresses set
             *  invalid. This basically means that the write function will
             *  consider everything as new, only writing the new data and not
             *  purging any of the old data.  It's just simpler this way than
             *  trying to calculate the difference and remove it later.
             */
            result = _bcm_sirius_multicast_write(unitData,
                                                 group,
                                                 &mvrData);
        } /* if (BCM_E_NONE == result) */

        /* get rid of the old group */
        if (BCM_E_NONE == result) {
            /*
             *  Since we claimed that the entire group was new to the write
             *  function, now we just get rid of the entire old instance.
             *  Simple.
             */
            if (mvrOrig->targets) {
                /* has targets, so OITT (maybe RR) */
                if (mvrOrig->logical) {
                    /* logical mode, so has RR and OITT */
                    for (index0 = 0; index0 < mvrOrig->targets; index0++) {
                        if (mvrData->ptr.rr[index0].base <= unitData->oittMax) {
                            shr_mdb_list_insert(unitData->oittList,
                                                unitData->limbo,
                                                mvrOrig->ptr.rr[index0].base);
                            unitData->oittBlocks--;
                        }
                    }
                    shr_mdb_list_insert(unitData->mdbList,
                                        unitData->limbo,
                                        mvrOrig->base);
                    unitData->mdbBlocks--;
                } else { /* if (mvrOrig->logical) */
                    /* physical mode, so only has OITT */
                    if (mvrData->base <= unitData->oittMax) {
                        shr_mdb_list_insert(unitData->oittList,
                                            unitData->limbo,
                                            mvrOrig->base);
                        unitData->oittBlocks--;
                    }
                } /* if (mvrOrig->logical) */
            } /* if (mvrOrig->targets) */
            shr_mdb_list_insert(unitData->mdbList,
                                unitData->limbo,
                                mvrAddr);
            unitData->mdbBlocks--;
        } /* if (BCM_E_NONE == result) */
#if defined(BCM_EASY_RELOAD_SUPPORT)
    } /* if (!SOC_IS_RELOADING(unitData->unit)) */
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */

    if (BCM_E_NONE == result) {
        /*
         *  Also just as easy to manage the cache update -- just dispose of
         *  the old cache and put in the new.
         */
        if (unitData->cache[group].data) {
            /* had a cache block already */
            sal_free(unitData->cache[group].data);
        } else {
            /* no cache block so we're adding one */
            unitData->cells++;
        }
        if (ncache) {
            /* have a new cache block */
            unitData->cache[group].data = ncache;
            unitData->cache[group].elements = port_count;
        } else {
            /* no new cache block */
            unitData->cache[group].data = NULL;
            unitData->cache[group].elements = 0;
            unitData->cells--;
        }
    } else { /* if (BCM_E_NONE == result) */
        if (ncache) {
            /* didn't use the new cache block, so dispose of it */
            sal_free(ncache);
        }
    }

    /* clean up resources */
    _bcm_sirius_multicast_mvrdata_free(mvrData);
    _bcm_sirius_multicast_mvrdata_free(mvrOrig);

    MC_VERB((MC_MSG("((%08X)%d,%08X,%d,*,*,%s) return %d (%s)\n"),
             (uint32)unitData,
             unitData->unit,
             group,
             port_count,
             use_oi?"TRUE":"FALSE",
             result,
             _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    _bcm_sirius_multicast_egress_delete
 *  Purpose
 *    Delete a particular GPORT,OI from an existing multicast group.
 *  Arguments
 *    (in) _mc_unit_t *unitData = the unit information
 *    (in) bcm_multicast_t group = group ID
 *    (in) int port_count = number of ports (why can this be negative?)
 *    (in) bcm_gport_t *port_array = pointer to array of gport IDs
 *    (in) bcm_if_t *encap_id_array = pointer to array of encap_ids
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Does not take lock or verify unit.
 *
 *    The particular GPORT,OI must exist within the group.
 *
 *    In logical mode, this has to track the OITT block address and OITT
 *    descriptor pointer for all targets removed due to the specified GPORT,OI
 *    being the last replicant on a target, plus it must track the actual index
 *    into each target's replicant list for the current GPORT,OI.  This means
 *    that it requires a fair bit of stack space.
 *
 *    Tries to reduce memory usage by rearranging things 'in place'.
 *
 *    Note that, unlike add and set, this relies upon the cached data for the
 *    GPORT, so that it will perform the correct deletion even if the GPORT
 *    data have changed for some reason.  This allows it to be less sensitive
 *    to other subsystems having concurrent changes.
 */

static int
_bcm_sirius_multicast_egress_delete(_mc_unit_t *unitData,
                                    bcm_multicast_t group,
                                    bcm_gport_t gport,
                                    bcm_if_t encap_id,
                                    bcm_gport_t queue_id,
                                    int checkQueue)
{
    unsigned int replicant[_SIRIUS_MC_MAX_TARGETS];  /* curr rep per target */
    uint32 *oittPtr[_SIRIUS_MC_MAX_TARGETS];         /* OITT ptrs to delete */
    uint32 oittAddr[_SIRIUS_MC_MAX_TARGETS];         /* OITT addrs to delete */
    _mc_mvr_internal_t *mvrOrig = NULL;              /* original MVR data */
    _mc_rr_elem_internal_t *rrPtr = NULL;            /* RR ptr to delete */
    _gpoiCacheData_t *cache = NULL;                  /* replacement cache */
    uint32 rrAddr = _MC_INVALID_BASE;                /* RR addr to delete */
    unsigned int pIndex;                             /* gport cache index */
    unsigned int tIndex;                             /* target index */
    unsigned int index;                              /* other working index */
    int result;                                      /* working result */
    unsigned int hga = 0;                            /* higig aggregate ID */
    unsigned int hgaIndex = ~0;                      /* higig aggregate last */
    int orgTgt;                                      /* original target */
    uint8 hgaFound[_SIRIUS_FAB_LAG_COUNT];           /* higig aggregates hit */
    uint8 hgFound[_SIRIUS_FAB_LAG_COUNT];            /* higig XGS singles hit */

    MC_VVERB((MC_MSG("(%08X(%d),%08X,%08X,%08X) enter\n"),
              (uint32)unitData,
              unitData->unit,
              group,
              gport,
              encap_id));

    /* make sure the group exists */
    result = _bcm_sirius_multicast_group_check(unitData, group);
    if (BCM_E_NONE != result) {
        /* called function displayed error diagnostic */
        return result;
    }

    /* get the current group data */
    result = _bcm_sirius_multicast_read(unitData,
                                        group,
                                        &mvrOrig);
    if (BCM_E_NONE != result) {
        /* called function displayed error diagnostic */
        return result;
    }

    /* allocate replacement GPORT,OI cache */
    if (unitData->cache[group].elements > 1) {
        cache = sal_alloc(sizeof(*cache) * (unitData->cache[group].elements - 1),
                          "MC group GPORT+OI member cache");
        if (!cache) {
            MC_ERR((MC_MSG1("unable to allocate %d bytes for new group"
                            " membership cache entry\n"),
                    sizeof(*cache) * (unitData->cache[group].elements - 1)));
            result = BCM_E_MEMORY;
        }
    }

    /* mark no higig lags or higig ports as found */
    sal_memset(&(hgaFound[0]), 0x00, sizeof(hgaFound[0]) * _SIRIUS_FAB_LAG_COUNT);
    sal_memset(&(hgFound[0]), 0x00, sizeof(hgFound[0]) * _SIRIUS_FAB_LAG_COUNT);

    /* search for the first instance of the given GPORT,OI tuple */
    if (BCM_E_NONE == result) {
        MC_EVERB((MC_MSG1("prep and search unit %d group %08X for"
                          " GPORT %08X EI %08X\n"),
                  unitData->unit,
                  group,
                  gport,
                  encap_id));
        /*
         *  Logical mode is complicated here, since we need to find not only
         *  the GPORT,OI tuple for which we are searching, but the offset of
         *  that tuple within *all* of the replicant lists for each target.
         *
         *  Physical mode is much simpler -- just search the list until we find
         *  the particular GPORT,OI that we want.
         */
        if (mvrOrig->logical) {
            /* logical mode uses some buffers */
            sal_memset(&(replicant[0]),
                       0x00,
                       sizeof(replicant[0]) * _SIRIUS_MC_MAX_TARGETS);
        } /* if (mvrOrig->logical) */
        sal_memset(&(oittPtr[0]),
                   0x00,
                   sizeof(oittPtr[0]) * _SIRIUS_MC_MAX_TARGETS);
        for (tIndex = 0; tIndex < _SIRIUS_MC_MAX_TARGETS; tIndex++) {
            oittAddr[tIndex] = unitData->oittMax + 1;
        }

        /* search through the target replicants until find match */
        for (pIndex = 0;
             (pIndex < unitData->cache[group].elements) &&
             ((unitData->cache[group].data[pIndex].port != gport) ||
              (unitData->cache[group].data[pIndex].encap_id != encap_id));
             pIndex++) {
            if (checkQueue &&
                (unitData->cache[group].data[pIndex].queue_id != queue_id)) {
                /* need to check queue ID and it's not correct; skip this */
                continue;
            }
            if (mvrOrig->logical) {
                for (tIndex = 0; tIndex < _SIRIUS_MC_MAX_TARGETS; tIndex++) {
                    if (SHR_BITGET(unitData->cache[group].data[pIndex].tbmp, tIndex)) {
                        replicant[tIndex]++;
                    }
                } /* for (tIndex = 0; tIndex < _SIRIUS_MC_MAX_TARGETS; tIndex++) */
            } /* if (mvrOrig->logical) */
            hga = ((unitData->cache[group].data[pIndex].extras >>
                    _MC_CACHE_EXTRA_HGA_SHIFT) &
                   _MC_CACHE_EXTRA_HGA_MASK);
            if (hga) {
                if ((_SIRIUS_FAB_LAG_COUNT + 1) == hga) {
                    /* found an XGS mode higig */
                    orgTgt = (unitData->cache[group].data[pIndex].extras &
                              _MC_CACHE_EXTRA_GPORT_MASK);
                    hgFound[orgTgt] = TRUE;
                } else {
                    /* found an XGS mode higig aggregate */
                    hgaFound[hga - 1] = TRUE;
                }
            }
        } /* for (all gport,oi tuples in the group's cache up to match) */
        if (pIndex >= unitData->cache[group].elements) {
            /* went to end of cache without a match */
            if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
                MC_ERR((MC_MSG1("unit %d group %08X does not contain GPORT %08X"
                                " EI %08X queue %08X\n"),
                        unitData->unit,
                        group,
                        gport,
                        encap_id,
                        queue_id));
            } else {
                MC_ERR((MC_MSG1("unit %d group %08X does not contain GPORT %08X"
                                " EI %08X\n"),
                        unitData->unit,
                        group,
                        gport,
                        encap_id));
            }
            result = BCM_E_NOT_FOUND;
        } else { /* if (pIndex >= unitData->cache[group].elements) */
            hga = ((unitData->cache[group].data[pIndex].extras >>
                    _MC_CACHE_EXTRA_HGA_SHIFT) &
                   _MC_CACHE_EXTRA_HGA_MASK);
            orgTgt = (unitData->cache[group].data[pIndex].extras &
                      _MC_CACHE_EXTRA_GPORT_MASK);
            if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
                MC_EVERB((MC_MSG1("found GPORT %08X EI %08X queue %08X in unit"
                                  " %d group %08X at %d: %08X,%08X,%08X "
                                  _MC_TBMP_FORMAT
                                  " "
                                  _MC_CACHE_EXTRA_FORMAT
                                  " (%d:%d:%s)\n"),
                          gport,
                          encap_id,
                          queue_id,
                          unitData->unit,
                          group,
                          pIndex,
                          unitData->cache[group].data[pIndex].port,
                          unitData->cache[group].data[pIndex].encap_id,
                          unitData->cache[group].data[pIndex].queue_id,
                          _MC_TBMP_DISPLAY(unitData->cache[group].data[pIndex].tbmp),
                          unitData->cache[group].data[pIndex].extras,
                          hga,
                          orgTgt,
                          hga?((((_SIRIUS_FAB_LAG_COUNT + 1) == hga)?hgFound[orgTgt]:hgaFound[hga - 1])?"TRUE":"FALSE"):"N/A"));
            } else {
                MC_EVERB((MC_MSG1("found GPORT %08X EI %08X in unit %d group"
                                  " %08X at %d: %08X,%08X "
                                  _MC_TBMP_FORMAT
                                  " "
                                  _MC_CACHE_EXTRA_FORMAT
                                  " (%d:%d:%s)\n"),
                          gport,
                          encap_id,
                          unitData->unit,
                          group,
                          pIndex,
                          unitData->cache[group].data[pIndex].port,
                          unitData->cache[group].data[pIndex].encap_id,
                          _MC_TBMP_DISPLAY(unitData->cache[group].data[pIndex].tbmp),
                          unitData->cache[group].data[pIndex].extras,
                          hga,
                          orgTgt,
                          hga?((((_SIRIUS_FAB_LAG_COUNT + 1) == hga)?hgFound[orgTgt]:hgaFound[hga - 1])?"TRUE":"FALSE"):"N/A"));
            }
            if (hga) {
                /* this GPORT uses a XGS mode higig */
                result = BCM_E_NOT_FOUND;
                if ((_SIRIUS_FAB_LAG_COUNT + 1) == hga) {
                    /* this port uses a single XGS mode higig... */
                    if (hgFound[orgTgt]) {
                        /* ...and we already hit another port using the same */
                        result = BCM_E_NONE;
                    }
                } else {
                    /* this port uses a XGS mode higig aggregate... */
                    if (hgaFound[hga - 1]) {
                        /* ...and we already hit another port using the same */
                        result = BCM_E_NONE;
                    }
                }
                if (BCM_E_NONE == result) {
                    /*
                     *  We already hit another member of this higig or higig
                     *  aggregate, so we don't need to move things around
                     *  specially; just remove this <gport,oi> from the cache
                     *  and leave the hardware state alone.
                     */
                    MC_EVERB((MC_MSG1("already have another member of hg%s[%d]"
                                      " so removing this one from the cache\n"),
                              ((_SIRIUS_FAB_LAG_COUNT + 1) == hga)?"":"a",
                              ((_SIRIUS_FAB_LAG_COUNT + 1) == hga)?orgTgt:hga - 1));
                    hgaIndex = pIndex;
                    hga = 0;
                } else { /* if (hgaPrev & (1 << hga)) */
                    /*
                     *  There is no prior hit of this higig aggregate in the
                     *  group.  If there is another replicant using this higig
                     *  aggregate later in the cache, bring the final one down
                     *  to here. If there is no other replicant using this
                     *  higig aggregate later in the cache, we need to remove
                     *  this one per normal.
                     */
                    result = BCM_E_NONE;
                    for (tIndex = pIndex + 1;
                         tIndex < unitData->cache[group].elements;
                         tIndex++) {
                        if ((((unitData->cache[group].data[tIndex].extras >>
                               _MC_CACHE_EXTRA_HGA_SHIFT) &
                              _MC_CACHE_EXTRA_HGA_MASK) == hga) &&
                            ((unitData->cache[group].data[tIndex].extras &
                              _MC_CACHE_EXTRA_GPORT_MASK) == orgTgt)) {
                            hgaIndex = tIndex;
                        }
                    }
                    if (hgaIndex < unitData->cache[group].elements) {
                        MC_EVERB((MC_MSG1("found another member of hga[%d] at"
                                          " %d; replacing with that one\n"),
                                  hga,
                                  hgaIndex));
                        /* there is a later hit; move the latest to fill */
                        /* copy cache before the replacement */
                        sal_memcpy(&(cache[0]),
                                   &(unitData->cache[group].data[0]),
                                   sizeof(*cache) * hgaIndex);
                        /* copy the replacement entry over this entry */
                        sal_memcpy(&(cache[pIndex]),
                                   &(unitData->cache[group].data[hgaIndex]),
                                   sizeof(*cache));
                        if (hgaIndex < (unitData->cache[group].elements - 1)) {
                            /* copy cache after the replacement */
                            sal_memcpy(&(cache[hgaIndex]),
                                       &(unitData->cache[group].data[hgaIndex + 1]),
                                       sizeof(*cache) * ((unitData->cache[group].elements - hgaIndex) - 1));
                        }
                    } else { /* if (hgaIndex < unitData->cache[group].elements) */
                        /* there are no other hits; remove as per normal */
                        MC_EVERB((MC_MSG1("no other member of hga[%d];"
                                          " removing as per normal entry\n"),
                                  hga));
                        hga = 0;
                    }
                } /* if (hgaPrev & (1 << hga)) */
            } /* if (hga) */
            if (!hga) {
                /*
                 *  This <gport,oi> needs to be removed from the cache and any
                 *  elements above it need to be shifted in to fill.
                 */
                if (pIndex) {
                    /* copy cache before this entry */
                    sal_memcpy(&(cache[0]),
                               &(unitData->cache[group].data[0]),
                               sizeof(*cache) * pIndex);
                }
                if (pIndex < (unitData->cache[group].elements - 1)) {
                    /* copy cache after this entry */
                    sal_memcpy(&(cache[pIndex]),
                               &(unitData->cache[group].data[pIndex + 1]),
                               sizeof(*cache) * ((unitData->cache[group].elements - pIndex) - 1));
                }
            } /* if (!hga) */
        } /* if (pIndex >= unitData->cache[group].elements) */
    } /* if (BCM_E_NONE == result) */

    /* get rid of the found GPORT,OI tuple */
    if ((BCM_E_NONE == result) &&
        (hgaIndex >= unitData->cache[group].elements)) {
        /*
         *  More logical mode complexity -- for each target, we must remove
         *  only the located GPORT, but if this was the only replicant on the
         *  target, we must remove the target completely.
         *
         *  Physical mode is a little simpler -- we must remove the targets,
         *  but that means figuring out the offset in the OI list.
         */
        if (mvrOrig->logical) {
            /* logical mode */
            index = 0;
            for (tIndex = 0; tIndex < _SIRIUS_MC_MAX_TARGETS; tIndex++) {
                if (SHR_BITGET(mvrOrig->tsBitmap, tIndex)) {
                    /* target is included in group */
                    if (SHR_BITGET(unitData->cache[group].data[pIndex].tbmp,
                                   tIndex)) {
                        /* target is in GPORT to be removed */
                        if (mvrOrig->ptr.rr[index].replicants > 1) {
                            /* not only replicant; modify in place */
                            MC_EVERB((MC_MSG1("remove target %d (offset %d)"
                                              " replicant %d (of %d)"
                                              " from unit %d group %08X\n"),
                                      tIndex,
                                      index,
                                      replicant[tIndex],
                                      mvrOrig->ptr.rr[index].replicants,
                                      unitData->unit,
                                      group));
                            
                            memmove(&(mvrOrig->ptr.rr[index].oittPtr[replicant[tIndex]]),
                                    &(mvrOrig->ptr.rr[index].oittPtr[replicant[tIndex] + 1]),
                                    sizeof(mvrOrig->ptr.rr[index].oittPtr[replicant[tIndex]]) * ((mvrOrig->ptr.rr[index].replicants - replicant[tIndex]) - 1));
                            mvrOrig->ptr.rr[index].replicants--;
                            mvrOrig->ptr.rr[index].changed = TRUE;
                        } else { /* if (not the only replicant on target) */
                            /* only replicant; remove target */
                            MC_EVERB((MC_MSG1("remove target %d (offset %d)"
                                              " from unit %d group %08X:"
                                              " no replicants\n"),
                                      tIndex,
                                      index,
                                      unitData->unit,
                                      group));
                            oittPtr[tIndex] = mvrOrig->ptr.rr[index].oittPtr;
                            oittAddr[tIndex] = mvrOrig->ptr.rr[index].base;
                            
                            memmove(&(mvrOrig->ptr.rr[index]),
                                    &(mvrOrig->ptr.rr[index + 1]),
                                    sizeof(mvrOrig->ptr.rr[index]) * ((mvrOrig->targets - index) - 1));
                            SHR_BITCLR(mvrOrig->tsBitmap, tIndex);
                            mvrOrig->ptrChanged = TRUE;
                            index--;
                        } /* if (not the only replicant on target) */
                    } /* if (target exists in GPORT,OI to be removed */
                    index++;
                } /* if (target exists in group) */
            } /* for (all targets) */
        } else { /* if (mvrOrig->logical) */
            /* physical mode */
            index = 0;
            for (tIndex = 0; tIndex < _SIRIUS_MC_MAX_TARGETS; tIndex++) {
                if (SHR_BITGET(mvrOrig->tsBitmap, tIndex)) {
                    /* target is included in group */
                    if (SHR_BITGET(unitData->cache[group].data[pIndex].tbmp,
                                   tIndex)) {
                        /* target is in GPORT to be removed */
                        MC_EVERB((MC_MSG1("removing target %d from unit %d"
                                          " group %08X\n"),
                                  tIndex,
                                  unitData->unit,
                                  group));
                        
                        memmove(&(mvrOrig->ptr.oitt[index]),
                                &(mvrOrig->ptr.oitt[index + 1]),
                                sizeof(*mvrOrig->ptr.oitt) * ((mvrOrig->targets - index) - 1));
                        if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
                            memmove(&(mvrOrig->queuePtr[index]),
                                    &(mvrOrig->queuePtr[index + 1]),
                                    sizeof(*mvrOrig->queuePtr) * ((mvrOrig->targets - index) - 1));
                        }
                        SHR_BITCLR(mvrOrig->tsBitmap, tIndex);
                        mvrOrig->ptrChanged = TRUE;
                    } else {
                        /* target is not to be removed */
                        index++;
                    } /* if (target exists in GPORT,OI to be removed */
                } /* if (target exists in group) */
            } /* for (all targets) */
        } /* if (mvrOrig->logical) */
        if (mvrOrig->ptrChanged) {
            /* adjust for removed targets */
            MC_EVERB((MC_MSG1("unit %d group %08X: targets changed\n"),
                      unitData->unit,
                      group));
            _bcm_sirius_multicast_sparse_update(mvrOrig);
            mvrOrig->changed = TRUE;
            if (0 == mvrOrig->targets) {
                /* zero targets left */
                MC_EVERB((MC_MSG1("unit %d group %08X now has no targets\n"),
                          unitData->unit,
                          group));
                if (mvrOrig->logical) {
                    /* logical; get rid of RR information */
                    rrPtr = mvrOrig->ptr.rr;
                    rrAddr = mvrOrig->base;
                    mvrOrig->ptr.rr = NULL;
                    mvrOrig->base = _MC_INVALID_BASE;
                } else {
                    /* physical; get rid of OI information */
                    oittPtr[0] = mvrOrig->ptr.oitt;
                    oittAddr[0] = mvrOrig->base;
                    oittPtr[1] = mvrOrig->queuePtr;
                    mvrOrig->ptr.oitt = NULL;
                    mvrOrig->queuePtr = NULL;
                    mvrOrig->base = unitData->oittMax + 1;
                }
            } /* if (0 == mvrOrig->targets) */
        } /* if (mvrOrig->ptrChanged) */

#if defined(BCM_EASY_RELOAD_SUPPORT)
        /* handle reload case */
        if (!SOC_IS_RELOADING(unitData->unit)) {
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */
            /* now try to commit the changes */
            result = _bcm_sirius_multicast_write(unitData,
                                                 group,
                                                 &mvrOrig);
#if defined(BCM_EASY_RELOAD_SUPPORT)
        } /* if (!SOC_IS_RELOADING(unitData->unit)) */
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */
    } /* if (result good and need to remove the replicant) */

    /* clean up removed target data */
    if (BCM_E_NONE == result) {
#if defined(BCM_EASY_RELOAD_SUPPORT)
        /* handle reload case */
        if (!SOC_IS_RELOADING(unitData->unit)) {
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */
            MC_EVERB((MC_MSG1("clean up discards from unit %d group %08X\n"),
                      unitData->unit,
                      group));
            if (rrPtr) {
                sal_free(rrPtr);
            }
            if (rrAddr < _MC_INVALID_BASE) {
                shr_mdb_list_insert(unitData->mdbList,
                                    unitData->limbo,
                                    rrAddr);
                unitData->mdbBlocks--;
            }
            for (tIndex = 0; tIndex < _SIRIUS_MC_MAX_TARGETS; tIndex++) {
                if (oittPtr[tIndex]) {
                    sal_free(oittPtr[tIndex]);
                }
                if (oittAddr[tIndex] <= unitData->oittMax) {
                    shr_mdb_list_insert(unitData->oittList,
                                        unitData->limbo,
                                        oittAddr[tIndex]);
                    unitData->oittBlocks--;
                }
            }
#if defined(BCM_EASY_RELOAD_SUPPORT)
        } /* if (!SOC_IS_RELOADING(unitData->unit)) */
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */
        MC_EVERB((MC_MSG1("update cache for unit %d group %08X\n"),
                  unitData->unit,
                  group));
        sal_free(unitData->cache[group].data);
        unitData->cache[group].data = cache;
        unitData->cache[group].elements--;
    } else { /* if (BCM_E_NONE == result) */
        if (cache) {
            sal_free(cache);
        }
    } /* if (BCM_E_NONE == result) */

    /* clean up resources */
    _bcm_sirius_multicast_mvrdata_free(mvrOrig);

    MC_VVERB((MC_MSG("(%08X(%d),%08X,%08X,%08X) return %d (%s)\n"),
              (uint32)unitData,
              unitData->unit,
              group,
              gport,
              encap_id,
              result,
              _SHR_ERRMSG(result)));
    return result;
}


/*****************************************************************************
 *
 *  Background work
 */

/*
 *  Function
 *    _bcm_sirius_defrag_step
 *  Purpose
 *    Run a single step of the defrag process.  Overall, the defragmentation
 *    process is to move all of the MDB and OITT entries first to the low side
 *    of their respective spaces, and then to the high side.  Due to the way in
 *    which the mdb allocator mechanism works, this should tend to pack the
 *    entries reasonably tightly.  For reference, a single step is moving a
 *    small number of groups, in one direction only.  We leave the allocator
 *    set up with the directional bias even when not defragmenting, since this
 *    encourages new or updated groups to move to the proper end; once the
 *    defrag process is complete, we restore normal alloc behaviour.
 *  Arguments
 *    (in) _mc_unit_t *unitData = pointer to the unit's state
 *  Return
 *    (none)
 *  Notes
 *    Manipulates the alloc method (slower when not default).
 *    Manipulates the flags.
 *    Will abort the defrag process if the halt flag is set.
 *    Will clear the start flag.
 *    Will set the running flag, and clear it when done.
 *    Will clear the up flag initially, then set it at the appropriate point.
 */
static void
_bcm_sirius_defrag_step(_mc_unit_t *unitData)
{
    _mc_mvr_internal_t *mvrPtr;
    unsigned int tried;
    unsigned int touched;
    unsigned int moved;
    unsigned int errors;
    unsigned int index;
    int abort = FALSE;
    uint32 lowest;
    uint32 highest;
    int result;

#if _SBX_SIRIUS_MC_BG_OTHER_MSG
    MC_VVERB((MC_MSG("(%08X) enter\n"), (uint32)unitData));
#endif /* _SBX_SIRIUS_MC_BG_OTHER_MSG */

    /* handle start/stop requests */
    if (unitData->flags & _MC_UNIT_DEFRAG_RUNNING) {
        /* already running a defragmentation */
        if (unitData->flags & _MC_UNIT_DEFRAG_HALT) {
            /* stop defragmentation process */
#if _SBX_SIRIUS_MC_BG_OTHER_MSG
            MC_VVERB((MC_MSG1("ending defrag process for unit %d at %08X,%s\n"),
                      unitData->unit,
                      unitData->defragPosition,
                      (unitData->flags & _MC_UNIT_DEFRAG_UP)?"up":"down"));
#endif /* _SBX_SIRIUS_MC_BG_OTHER_MSG */
            unitData->flags &= (~(_MC_UNIT_DEFRAG_START |
                                  _MC_UNIT_DEFRAG_HALT |
                                  _MC_UNIT_DEFRAG_RUNNING |
                                  _MC_UNIT_DEFRAG_BY_MASK));
            shr_mdb_allocmode_set(unitData->mdbList, (shr_mdb_alloc_bank_first |
                                                      shr_mdb_alloc_block_high |
                                                      shr_mdb_free_block_high |
                                                      shr_mdb_join_alloc_and_free |
                                                      shr_mdb_join_high_and_low));
            shr_mdb_allocmode_set(unitData->oittList, (shr_mdb_alloc_bank_first |
                                                       shr_mdb_alloc_block_high |
                                                       shr_mdb_free_block_high |
                                                       shr_mdb_join_alloc_and_free |
                                                       shr_mdb_join_high_and_low));
        }
    } else { /* if (unitData->flags & _MC_UNIT_DEFRAG_RUNNING) */
        /* not currently running a defragmentation */
        if (unitData->flags & _MC_UNIT_DEFRAG_START) {
            /* start defragmentation process */
#if _SBX_SIRIUS_MC_BG_OTHER_MSG
            MC_VVERB((MC_MSG1("starting defrag process for unit %d\n"),
                      unitData->unit));
#endif /* _SBX_SIRIUS_MC_BG_OTHER_MSG */
            switch (unitData->flags & _MC_UNIT_DEFRAG_BY_MASK) {
            case _MC_UNIT_DEFRAG_BY_RES:
                unitData->defragRes++;
                break;
            case _MC_UNIT_DEFRAG_BY_USER:
                unitData->defragUser++;
                break;
            case _MC_UNIT_DEFRAG_BY_HEUR:
            default:
                unitData->defragHeur++;
                break;
            }
            unitData->flags = (unitData->flags &
                               ~(_MC_UNIT_DEFRAG_START |
                                 _MC_UNIT_DEFRAG_HALT |
                                 _MC_UNIT_DEFRAG_UP |
                                 _MC_UNIT_DEFRAG_ERROR |
                                 _MC_UNIT_DEFRAG_BY_MASK)) |
                              _MC_UNIT_DEFRAG_RUNNING;
            unitData->defragPosition = ~0;
            shr_mdb_allocmode_set(unitData->mdbList, (shr_mdb_alloc_bank_low |
                                                      shr_mdb_alloc_block_low |
                                                      shr_mdb_free_block_low |
                                                      shr_mdb_join_alloc_and_free |
                                                      shr_mdb_join_high_and_low));
            shr_mdb_allocmode_set(unitData->oittList, (shr_mdb_alloc_bank_low |
                                                       shr_mdb_alloc_block_low |
                                                       shr_mdb_free_block_low |
                                                       shr_mdb_join_alloc_and_free |
                                                       shr_mdb_join_high_and_low));
#if _SBX_SIRIUS_MC_BG_OTHER_MSG
        } else {
            MC_WARN((MC_MSG1("called on unit %d with no active defrag and"
                             " no requested defrag\n"),
                     unitData->unit));
#endif /* _SBX_SIRIUS_MC_BG_OTHER_MSG */
        }
    } /* if (unitData->flags & _MC_UNIT_DEFRAG_RUNNING) */
    /* bail if not doing anything */
    if (!(unitData->flags & _MC_UNIT_DEFRAG_RUNNING)) {
        return;
    }
    /* get group limits for this unit */
    result = shr_idxres_list_state(unitData->gmtList,
                                   &lowest,
                                   &highest,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL);
    if (BCM_E_NONE != result) {
        /* problem fetching group limits */
#if _SBX_SIRIUS_MC_BG_OTHER_MSG
        MC_ERR((MC_MSG1("unable to get unit %d group limits: %d (%s)\n"),
                unitData->unit,
                result,
                _SHR_ERRMSG(result)));
#endif /* _SBX_SIRIUS_MC_BG_OTHER_MSG */
        return;
    }
    highest = SOC_MEM_INFO(unitData->unit, EG_FD_GMTm).index_max;
    if (~0 == unitData->defragPosition) {
        /* need to set initial position */
        unitData->defragPosition = lowest;
    }
    /* try to move the expected number of groups */
    for (tried = 0, touched = 0, moved = 0, errors = 0;
         (_MC_DEFRAG_MAX_ATTEMPT > tried) &&
         (_MC_DEFRAG_MAX_TOUCH > touched) &&
         (_MC_DEFRAG_MAX_MOVE > moved) &&
         (!abort) &&
         (highest >= unitData->defragPosition);
         unitData->defragPosition++) {
        if (unitData->defragPosition <= unitData->gmtGroupMax) {
            result = shr_idxres_list_elem_state(unitData->gmtList,
                                                unitData->defragPosition);
        } else {
            if (SHR_BITGET(unitData->unicast,
                           (unitData->defragPosition -
                            (unitData->gmtGroupMax + 1)))) {
                result = BCM_E_EXISTS;
            } else {
                result = BCM_E_NOT_FOUND;
            }
        }
        tried++;
        switch (result) {
        case BCM_E_EXISTS:
            /* this group exists; read it */
            result = _bcm_sirius_multicast_read(unitData,
                                                unitData->defragPosition,
                                                &(mvrPtr));
            touched++;
            if (BCM_E_NONE == result) {
                /* read successful */
                /* mark everything as needing to be rewritten */
                mvrPtr->changed = TRUE;
                if (mvrPtr->targets) {
                    mvrPtr->ptrChanged = TRUE;
                    if (mvrPtr->logical) {
                        for (index = 0; index < mvrPtr->targets; index++) {
                            mvrPtr->ptr.rr[index].changed = TRUE;
                        }
                    }
                }
                /* write it back */
                result = _bcm_sirius_multicast_write(unitData,
                                                     unitData->defragPosition,
                                                     &mvrPtr);
                touched++;
                if (BCM_E_NONE == result) {
                    moved++;
#if _SBX_SIRIUS_MC_BG_OTHER_MSG
                } else {
                    MC_ERR((MC_MSG1("unable to rewrite unit %d group %08X:"
                                    " %d (%s)\n"),
                            unitData->unit,
                            unitData->defragPosition,
                            result,
                            _SHR_ERRMSG(result)));
#endif /* _SBX_SIRIUS_MC_BG_OTHER_MSG */
                }
                _bcm_sirius_multicast_mvrdata_free(mvrPtr);
#if _SBX_SIRIUS_MC_BG_OTHER_MSG
            } else {
                MC_ERR((MC_MSG1("unable to read unit %d group %08X: %d (%s)\n"),
                        unitData->unit,
                        unitData->defragPosition,
                        result,
                        _SHR_ERRMSG(result)));
#endif /* _SBX_SIRIUS_MC_BG_OTHER_MSG */
            }
            break;
        case BCM_E_NONE:
            /* this is unexpected, but it's not an error; skip it */
            /* fallthrough intentional */
        case BCM_E_NOT_FOUND:
            /* this group does not exist; skip it */
            result = BCM_E_NONE;
            break;
        default:
#if _SBX_SIRIUS_MC_BG_OTHER_MSG
            MC_ERR((MC_MSG1("unexpected error examining unit %d group %08X:"
                            " %d (%s)\n"),
                    unitData->unit,
                    unitData->defragPosition,
                    result,
                    _SHR_ERRMSG(result)));
#endif /* _SBX_SIRIUS_MC_BG_OTHER_MSG */
            break;
        } /* switch (result) */
        if (BCM_E_NONE != result) {
            /*
             *  An error occurred.  If this is the first time we see an error
             *  for this group during this pass, we'll defer the rest of this
             *  step, to come back during the next background execution and
             *  start a new step with this group.  If we've already done that,
             *  give up on this group for now and go on.
             */
            errors++;
            if (unitData->flags & _MC_UNIT_DEFRAG_ERROR) {
                /* resuming from an error, but still it fails; just go on */
                unitData->flags &= (~_MC_UNIT_DEFRAG_ERROR);
            } else {
                /* first time to hit an error here; come back later */
                unitData->flags |= _MC_UNIT_DEFRAG_ERROR;
                abort = TRUE;
            }
        } else {
            /* no erorr for resuming */
            unitData->flags &= (~_MC_UNIT_DEFRAG_ERROR);
        }
    } /* for (until we run out of groups, move a bunch, or check a bunch) */
#if _SBX_SIRIUS_MC_BG_OTHER_MSG
    MC_EVERB((MC_MSG1("unit %d defrag step to %08X: %d tries, %d accesses, %d moves, %d errors\n"),
              unitData->unit,
              unitData->defragPosition,
              tried,
              touched,
              moved,
              errors));
#endif /* _SBX_SIRIUS_MC_BG_OTHER_MSG */
    /* check for state transition */
    if (highest <= unitData->defragPosition) {
        /* finished a pass! */
        if (unitData->flags & _MC_UNIT_DEFRAG_UP) {
            /* finished a complete defragmentation run */
#if _SBX_SIRIUS_MC_BG_OTHER_MSG
            MC_VVERB((MC_MSG1("completed defragmentation on unit %d\n"),
                      unitData->unit));
#endif /* _SBX_SIRIUS_MC_BG_OTHER_MSG */
            unitData->flags = unitData->flags &
                              ~(_MC_UNIT_DEFRAG_START |
                                _MC_UNIT_DEFRAG_HALT |
                                _MC_UNIT_DEFRAG_RUNNING |
                                _MC_UNIT_DEFRAG_UP |
                                _MC_UNIT_DEFRAG_ERROR |
                                _MC_UNIT_DEFRAG_BY_MASK);
            unitData->defragPosition = ~0;
            shr_mdb_allocmode_set(unitData->mdbList, (shr_mdb_alloc_bank_first |
                                                      shr_mdb_alloc_block_high |
                                                      shr_mdb_free_block_high |
                                                      shr_mdb_join_alloc_and_free |
                                                      shr_mdb_join_high_and_low));
            shr_mdb_allocmode_set(unitData->oittList, (shr_mdb_alloc_bank_first |
                                                       shr_mdb_alloc_block_high |
                                                       shr_mdb_free_block_high |
                                                       shr_mdb_join_alloc_and_free |
                                                       shr_mdb_join_high_and_low));
        } else {
            /* finished the 'down' pass; switch to 'up' pass */
#if _SBX_SIRIUS_MC_BG_OTHER_MSG
            MC_VVERB((MC_MSG1("unit %d defragmentation first pass done\n"),
                      unitData->unit));
#endif /* _SBX_SIRIUS_MC_BG_OTHER_MSG */
            unitData->flags |= _MC_UNIT_DEFRAG_UP;
            unitData->defragPosition = ~0;
            shr_mdb_allocmode_set(unitData->mdbList, (shr_mdb_alloc_bank_high |
                                                      shr_mdb_alloc_block_high |
                                                      shr_mdb_free_block_high |
                                                      shr_mdb_join_alloc_and_free |
                                                      shr_mdb_join_high_and_low));
            shr_mdb_allocmode_set(unitData->oittList, (shr_mdb_alloc_bank_high |
                                                       shr_mdb_alloc_block_high |
                                                       shr_mdb_free_block_high |
                                                       shr_mdb_join_alloc_and_free |
                                                       shr_mdb_join_high_and_low));
        }
    } /* if (highest <= unitData->defragPosition) */

#if _SBX_SIRIUS_MC_BG_OTHER_MSG
    MC_VVERB((MC_MSG("(%08X) leave\n"), (uint32)unitData));
#endif /* _SBX_SIRIUS_MC_BG_OTHER_MSG */
}

/*
 *  Function
 *    _bcm_sirius_mc_bg
 *  Purpose
 *    Run (about) once per second, during which time it will, for each
 *    initialised Sirius multicast unit:
 *      * free the purge lists
 *      * advance the limbo lists
 *      * advance the purge lists
 *      * check for obvious conditions that should trigger defragmentation
 *  Arguments
 *    (ignored) void *arg = (don't care)
 *  Return
 *    Not likely.  Will remove itself if there are no initialised Sirius
 *    multicast units when it wakes up to run.
 *  Notes
 */
static void
_bcm_sirius_mc_bg(void *arg)
{
    int unit;
    int running = TRUE;
    int result;
    shr_mdb_info_t mdbInfo;
    shr_mdb_elem_index_t upperCount;
    shr_mdb_elem_index_t lowerCount;
    shr_mdb_list_info_t mdbListInfo;
    unsigned int mdbLimboElements;
    unsigned int mdbLimboBlocks;
    unsigned int index;
    static unsigned int skipGlob;
    static unsigned int skipUnit[SOC_MAX_NUM_DEVICES];
#if _SBX_SIRIUS_MC_BG_SLEEP_WAKE_MSG
    sal_usecs_t microsec = sal_time_usecs();
    sal_usecs_t lastmicrosec = microsec;
#endif /* _SBX_SIRIUS_MC_BG_SLEEP_WAKE_MSG */
    _mc_unit_t *thisUnit;

    MC_VERB((MC_MSG1("background process starting\n")));

    skipGlob = 0;
    sal_memset(&(skipUnit[0]), 0x00, sizeof(skipUnit[0]) * SOC_MAX_NUM_DEVICES);

    /* as long as there is a reason, do the job */
    while (running) {
        /*
         *  Wait a second before doing anything this iteration.
         *
         *  We don't really care about precision here; it's more a question of
         *  whether we wait long enough or not.  Since the backgrounder only
         *  frees limbo blocks, makes adjustments to the limbo/free lists, and
         *  defragments the free space, we don't really care if it runs each
         *  1000ms or each 1050ms or longer, as long as it still happens fairly
         *  periodically.  It is also for this reason that we do not bother to
         *  compute the time taken and reduce the sleep time by that amount,
         *  nor use fast spins as usleep is often implemented.
         *
         *  If it is later decided that greater precision and accuracy are
         *  required, it might be easiest to use a select or similar construct.
         */
#if _SBX_SIRIUS_MC_BG_SLEEP_WAKE_MSG
        microsec = sal_time_usecs();
        MC_VERB((MC_MSG1("background process sleeping (%d)\n"),
                 microsec - lastmicrosec));
#endif /* _SBX_SIRIUS_MC_BG_SLEEP_WAKE_MSG */
        sal_sleep(1);
#if _SBX_SIRIUS_MC_BG_SLEEP_WAKE_MSG
        MC_VERB((MC_MSG1("background process waking\n")));
        lastmicrosec = sal_time_usecs();
#endif /* _SBX_SIRIUS_MC_BG_SLEEP_WAKE_MSG */
        /*
         *  We don't want units coming and going while we're running, as that
         *  would cause problems with the code that shuts this down when it
         *  sees no active units during a scan.
         */
        if (sal_mutex_take(_mc_lock, _MC_BACKGROUND_GLOBAL_LOCK_WAIT)) {
            /* could not take the global lock quickly enough; try later */
#if _SBX_SIRIUS_MC_BG_OTHER_MSG
            MC_ERR((MC_MSG1("global lock timeout (%d); will retry later\n"),
                    skipGlob));
#endif /* _SBX_SIRIUS_MC_BG_OTHER_MSG */
            skipGlob++;
            continue;
        }
        /* expect no initialised units */
        running = FALSE;
        /* look for initialised units */
        for (unit = 0; unit < SOC_MAX_NUM_DEVICES; unit++) {
            thisUnit = _mc_unit[unit];
            if (thisUnit &&
                SOC_CONTROL(unit) &&
                SOC_SBX_CONTROL(unit) &&
                SOC_SBX_CFG(unit) &&
                SOC_SBX_CFG_SIRIUS(unit) &&
                (SOC_SBX_CFG_SIRIUS(unit)->lMcAggrLock)) {
                /* this unit is initialised */
                running = TRUE;
                /* take unit lock */
                if (sal_mutex_take(SOC_SBX_CFG_SIRIUS(unit)->lMcAggrLock,
                                   _MC_BACKGROUND_UNIT_LOCK_WAIT)) {
                    /* count not take unit lock quickly enough */
#if _SBX_SIRIUS_MC_BG_OTHER_MSG
                    MC_ERR((MC_MSG1("unit %d lock timeout (%d);"
                                    " will retry later\n"),
                            unit,
                            skipUnit[unit]));
#endif /* _SBX_SIRIUS_MC_BG_OTHER_MSG */
                    skipUnit[unit]++;
                    continue;
                }
                thisUnit->bgSkipGlob = skipGlob;
                thisUnit->bgSkipUnit = skipUnit[unit];
                if (0 == (thisUnit->flags & _MC_UNIT_LIMBO_PEAK_INH)) {
                    /* not inhibiting peak updates anymore; update peaks */
                    mdbLimboElements=0;
                    mdbLimboBlocks=0;
                    for (index = 0; index < _MC_LIMBO_LISTS; index++) {
                        result = shr_mdb_list_info(thisUnit->mdbList,
                                                   index,
                                                   FALSE,
                                                   &mdbListInfo);
                        if (BCM_E_NONE == result) {
                            mdbLimboElements += mdbListInfo.elements;
                            mdbLimboBlocks += mdbListInfo.blocks;
                        }
                    }
                    if (mdbLimboElements > thisUnit->limboMdbMax) {
                        thisUnit->limboMdbMax = mdbLimboElements;
                        thisUnit->limboMdbMaxBlk = mdbLimboBlocks;
                    }
                    mdbLimboElements=0;
                    mdbLimboBlocks=0;
                    for (index = 0; index < _MC_LIMBO_LISTS; index++) {
                        result = shr_mdb_list_info(thisUnit->oittList,
                                                   index,
                                                   FALSE,
                                                   &mdbListInfo);
                        if (BCM_E_NONE == result) {
                            mdbLimboElements += mdbListInfo.elements;
                            mdbLimboBlocks += mdbListInfo.blocks;
                        }
                    }
                    if (mdbLimboElements > thisUnit->limboOittMax) {
                        thisUnit->limboOittMax = mdbLimboElements;
                        thisUnit->limboOittMaxBlk = mdbLimboBlocks;
                    }
                }
                /* free the purge lists */
                result = shr_mdb_list_purge(thisUnit->mdbList,
                                            thisUnit->purge);
#if _SBX_SIRIUS_MC_BG_OTHER_MSG
                if (BCM_E_NONE != result) {
                    MC_ERR((MC_MSG1("unable to purge aged MDB elements on"
                                    " unit %d: %d (%s)\n"),
                            unit,
                            result,
                            _SHR_ERRMSG(result)));
                }
#endif /* _SBX_SIRIUS_MC_BG_OTHER_MSG */
                result = shr_mdb_list_purge(thisUnit->oittList,
                                            thisUnit->purge);
#if _SBX_SIRIUS_MC_BG_OTHER_MSG
                if (BCM_E_NONE != result) {
                    MC_ERR((MC_MSG1("unable to purge aged OITT elements on"
                                    " unit %d: %d (%s)\n"),
                            unit,
                            result,
                            _SHR_ERRMSG(result)));
                }
#endif /* _SBX_SIRIUS_MC_BG_OTHER_MSG */
                /* advance the purge list */
                thisUnit->purge = (thisUnit->purge + 1) % _MC_LIMBO_LISTS;
                /* advance the limbo list */
                thisUnit->limbo = (thisUnit->limbo + 1) % _MC_LIMBO_LISTS;
                /* release peak update inhibit if appropriate */
                if (0 == thisUnit->limbo) {
                    /* have purged all of the limbo lists at least once */
                    thisUnit->flags &= ~_MC_UNIT_LIMBO_PEAK_INH;
                }
                /*
                 *  See if the unit should be defragmented.  Right now, this
                 *  uses a crude heuristic that assumes if there are more
                 *  elements available small blocks than in large blocks,
                 *  defragmentation might be able to help (particularly since
                 *  we want the largest free blocks possible for more efficient
                 *  allocations).  While this may be true, it may possibly
                 *  trigger gratuitous defragmentation runs, and (somewhat more
                 *  probably) may miss other conditions that could be helped by
                 *  a defrag run.
                 *
                 *  Of course, this is not the only thing that can trigger an
                 *  automatic defrag run; it can also be triggered by certain
                 *  resource deficiencies.  Further, there are hooks to allow
                 *  an external trigger (such as a user request).
                 */
                /* check the free information for the MDB space */
                result = shr_mdb_info(thisUnit->mdbList, &mdbInfo);
                if (BCM_E_NONE == result) {
                    upperCount = 0;
                    lowerCount = 0;
                    for (index = 0; index < mdbInfo.free_lists; index++) {
                        result = shr_mdb_list_info(thisUnit->mdbList,
                                                   index,
                                                   TRUE,
                                                   &mdbListInfo);
                        if (BCM_E_NONE == result) {
                            if (_SIRIUS_MC_MAX_TARGETS <=
                                mdbListInfo.block_size) {
                                /* blocks are max alloc size or bigger */
                                upperCount += mdbListInfo.elements;
                            } else {
                                /* blocks are smaller than max alloc size */
                                lowerCount += mdbListInfo.elements;
                            }
                        } else {
#if _SBX_SIRIUS_MC_BG_OTHER_MSG
                            MC_ERR((MC_MSG1("unable to get unit %d MDB "
                                            "freelist[%d] information\n"),
                                    thisUnit->unit,
                                    index));
#endif /* _SBX_SIRIUS_MC_BG_OTHER_MSG */
                            break;
                        }
                    }
                    if (BCM_E_NONE == result) {
                        /* scanned the MDB space properly; check it */
                        if (upperCount < lowerCount) {
#if _SBX_SIRIUS_MC_BG_OTHER_MSG
                            MC_VVERB((MC_MSG1("more free elements are in small"
                                              " blocks than large blocks for"
                                              " unit %d MDB; invoking"
                                              " defrag\n"),
                                      thisUnit->unit));
#endif /* _SBX_SIRIUS_MC_BG_OTHER_MSG */
                            thisUnit->flags = (thisUnit->flags &
                                               (~_MC_UNIT_DEFRAG_BY_MASK)) |
                                              (_MC_UNIT_DEFRAG_START |
                                               _MC_UNIT_DEFRAG_BY_HEUR);
                            thisUnit->defragAttHeur++;
                        }
                    }
#if _SBX_SIRIUS_MC_BG_OTHER_MSG
                } else {
                    MC_ERR((MC_MSG1("unable to get unit %d MDB information\n"),
                            thisUnit->unit));
#endif /* _SBX_SIRIUS_MC_BG_OTHER_MSG */
                }
                /* check the free information for the OITT space */
                result = shr_mdb_info(thisUnit->oittList, &mdbInfo);
                if (BCM_E_NONE == result) {
                    upperCount = 0;
                    lowerCount = 0;
                    for (index = 0; index < mdbInfo.free_lists; index++) {
                        result = shr_mdb_list_info(thisUnit->oittList,
                                                   index,
                                                   TRUE,
                                                   &mdbListInfo);
                        if (BCM_E_NONE == result) {
                            if (_MC_MAX_REPLICANTS <= mdbListInfo.block_size) {
                                /* blocks are max alloc size or bigger */
                                upperCount += mdbListInfo.elements;
                            } else {
                                /* blocks are smaller than max alloc size */
                                lowerCount += mdbListInfo.elements;
                            }
                        } else {
#if _SBX_SIRIUS_MC_BG_OTHER_MSG
                            MC_ERR((MC_MSG1("unable to get unit %d OITT "
                                            "freelist[%d] information\n"),
                                    thisUnit->unit,
                                    index));
#endif /* _SBX_SIRIUS_MC_BG_OTHER_MSG */
                            break;
                        }
                    }
                    if (BCM_E_NONE == result) {
                        /* scanned the OITT space properly; check it */
                        if (upperCount < lowerCount) {
#if _SBX_SIRIUS_MC_BG_OTHER_MSG
                            MC_VVERB((MC_MSG1("more free elements are in small"
                                              " blocks than large blocks for"
                                              " unit %d OITT; invoking"
                                              " defrag\n"),
                                      thisUnit->unit));
#endif /* _SBX_SIRIUS_MC_BG_OTHER_MSG */
                            thisUnit->flags = (thisUnit->flags &
                                               (~_MC_UNIT_DEFRAG_BY_MASK)) |
                                              (_MC_UNIT_DEFRAG_START |
                                               _MC_UNIT_DEFRAG_BY_HEUR);
                            thisUnit->defragAttHeur++;
                        }
                    }
#if _SBX_SIRIUS_MC_BG_OTHER_MSG
                } else {
                    MC_ERR((MC_MSG1("unable to get unit %d OITT information\n"),
                            thisUnit->unit));
#endif /* _SBX_SIRIUS_MC_BG_OTHER_MSG */
                }
                /* call the defragger if appropriate */
                if (thisUnit->flags & (_MC_UNIT_DEFRAG_START |
                                       _MC_UNIT_DEFRAG_RUNNING)) {
                    _bcm_sirius_defrag_step(thisUnit);
                }
                /* release the unit lock */
                if (sal_mutex_give(SOC_SBX_CFG_SIRIUS(unit)->lMcAggrLock)) {
                    /* error releasing unit lock */
                    MC_ERR((MC_MSG1("unit %d lock release error!\n"), unit));
                }
            } /* if (thisUnit && thisUnit->lock) */
        } /* for (unit = 0; unit < SOC_MAX_NUM_DEVICES; unit++) */
        if (sal_mutex_give(_mc_lock)) {
            /* error releasing global lock */
            MC_ERR((MC_MSG1("unable to release global lock!\n")));
        }
    } /* while (running) */
    /* did not see any initialised units */
    MC_VERB((MC_MSG1("background process stopping:"
                     " no initialised units\n")));
    _mc_background = NULL;
}


/*****************************************************************************
 *
 *  Internal accounting and management (initialisation)
 */

/*
 *  Function
 *    _bcm_sirius_multicast_group_preload
 *  Purpose
 *    Scan hardware and allocate resources for a specific group during init.
 *    This is used so that the resources that are allocated to groups that
 *    should be added during 'easy reload' or similar are already in place,
 *    avoiding the bizarre timing requirements for reloading correctly.
 *  Arguments
 *    (in) _mc_unit_t *unitData = the working unit data pointer
 *    (in) bcm_multicast_t group = the group that is being scanned
 *    (in) uint32 mvr = MVR pointer in MDB for the group
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    The unit must be locked by caller.
 */
static int
_bcm_sirius_multicast_group_preload(_mc_unit_t *unitData,
                                    bcm_multicast_t group,
                                    uint32 mvrPtr)
{
    _mc_mvr_internal_t mvrData;
    _mc_rr_elem_internal_t rrData[_SIRIUS_MC_MAX_TARGETS];
    uint32 index;
    int result = BCM_E_NONE;
    mvrData.ptr.rr = &(rrData[0]);

    if (mvrPtr &&
        (_MC_UNICAST_MVR_MASK != (_MC_UNICAST_MVR_MASK & mvrPtr))) {
        /* MVR address is nonzero and not unicast special value */
        MC_VERB((MC_MSG1("unit %d multicast group %08X in use (MVRPtr ="
                         " %08X)\n"),
                 unitData->unit,
                 group,
                 mvrPtr));
        result = _bcm_sirius_multicast_mvr_read(unitData,
                                                mvrPtr,
                                                &mvrData);
        if (BCM_E_NONE != result) {
            MC_ERR((MC_MSG1("unable to read unit %d group %08X MVT"
                            " information at MDB %08X: %d (%s)\n"),
                    unitData->unit,
                    group,
                    mvrPtr,
                    result,
                    _SHR_ERRMSG(result)));
        } else { /* if (BCM_E_NONE != result) */
            if ((mvrData.logical) && mvrData.targets) {
                result = _bcm_sirius_multicast_rr_read(unitData,
                                                       mvrData.base,
                                                       mvrData.targets,
                                                       &(rrData[0]));
                if (BCM_E_NONE != result) {
                    MC_ERR((MC_MSG1("unable to read unit %d group %08X RR"
                                    " information at MDB %08X: %d (%s)\n"),
                            unitData->unit,
                            group,
                            mvrData.base,
                            result,
                            _SHR_ERRMSG(result)));
                }
            } /* if (mvrData->logical) */
        } /* if (BCM_E_NONE != result) */
        if (BCM_E_NONE == result) {
            /* we now have a group; reserve its space */
            result = shr_mdb_alloc_id(unitData->mdbList,
                                      mvrPtr,
                                      mvrData.elems);
            if (BCM_E_NONE == result) {
                unitData->mdbBlocks++;
                MC_EVERB((MC_MSG1("reserved %d:mdb[%08X..%08X] for"
                                  " unit %d group %08X MVT\n"),
                          unitData->unit,
                          mvrPtr,
                          mvrPtr + mvrData.elems - 1,
                          unitData->unit,
                          group));
            } else {
                MC_ERR((MC_MSG1("unable to reserve %d:mdb[%08X..%08X] for"
                                " unit %d group %08X MVT: %d (%s)\n"),
                        unitData->unit,
                        mvrPtr,
                        mvrPtr + mvrData.elems,
                        unitData->unit,
                        group,
                        result,
                        _SHR_ERRMSG(result)));
            }
        } /* if (BCM_E_NONE == result) */
        if ((BCM_E_NONE == result) && mvrData.targets) {
            /* valid so far and nonzero target count */
            if (mvrData.logical) {
                /* logical, always allocate RR space */
                result = shr_mdb_alloc_id(unitData->mdbList,
                                          mvrData.base,
                                          mvrData.targets);
            } else { /* if (mvrData->logical) */
                /* physical, maybe allocate OITT space */
                if ((mvrData.base >= unitData->oittMin) &&
                    ((mvrData.base + mvrData.targets - 1) <= unitData->oittMax)) {
                    result = shr_mdb_alloc_id(unitData->oittList,
                                              mvrData.base,
                                              mvrData.targets);
                    if (BCM_E_NONE == result) {
                        unitData->oittBlocks++;
                    }
                } /* if (OITT that would be used is valid) */
            } /* if (mvrData->logical) */
            if (BCM_E_NONE == result) {
                unitData->mdbBlocks++;
                if (mvrData.logical ||
                    ((mvrData.base >= unitData->oittMin) &&
                     ((mvrData.base + mvrData.targets - 1) <= unitData->oittMax))) {
                    MC_EVERB((MC_MSG1("reserved %d:%s[%08X..%08X] for"
                                      " unit %d group %08X %s\n"),
                              unitData->unit,
                              mvrData.logical?"mdb":"oitt",
                              mvrData.base,
                              mvrData.base + mvrData.targets - 1,
                              unitData->unit,
                              group,
                              mvrData.logical?"RR":"OITT"));
                } /* if (OITT that would be used is valid) */
            } else { /* if (BCM_E_NONE == result) */
                MC_ERR((MC_MSG1("unable to reserve %d:%s[%08X..%08X] for"
                                " unit %d group %08X: %d (%s)\n"),
                        unitData->unit,
                        mvrData.logical?"mdb":"oitt",
                        mvrData.base,
                        mvrData.base + mvrData.targets - 1,
                        unitData->unit,
                        group,
                        result,
                        _SHR_ERRMSG(result)));
            } /* if (BCM_E_NONE == result) */
        } /* if (BCM_E_NONE == result) */
        if (mvrData.logical) {
            /* logical distribution */
            for (index = 0;
                 (BCM_E_NONE == result) && (index < mvrData.targets);
                 index++) {
                if ((rrData[index].base >= unitData->oittMin) &&
                    ((rrData[index].base + rrData[index].replicants - 1) <= unitData->oittMax)) {
                    result = shr_mdb_alloc_id(unitData->oittList,
                                              rrData[index].base,
                                              rrData[index].replicants);
                    if (BCM_E_NONE == result) {
                        unitData->oittBlocks++;
                        MC_EVERB((MC_MSG1("reserved %d:oitt[%08X..%08X]"
                                          " for unit %d group %08X RR[%d].OITT\n"),
                                  unitData->unit,
                                  rrData[index].base,
                                  rrData[index].base + rrData[index].replicants - 1,
                                  unitData->unit,
                                  group,
                                  index));
                    } else { /* if (BCM_E_NONE == result) */
                        MC_ERR((MC_MSG1("unable to reserve %d:oitt[%08X..%08X]"
                                        " for unit %d group %08X RR[%d].OITT:"
                                        " %d (%s)\n"),
                                unitData->unit,
                                rrData[index].base,
                                rrData[index].base + rrData[index].replicants - 1,
                                unitData->unit,
                                group,
                                index,
                                result,
                                _SHR_ERRMSG(result)));
                    } /* if (BCM_E_NONE == result) */
                } /* if (OITT that would be used is valid) */
            } /* for (all elements in RR and result is okay) */
        } /* if (BCM_E_NONE == result) */
#if 0 
    } else { /* if (MVR pointer is not NULL and not unicast */
        MC_EVERB((MC_MSG1("unit %d group %08X is %s (MVRPtr = %08X)\n"),
                  unitData->unit,
                  group,
                  mvrPtr?"unicast":"disabled",
                  mvrPtr));
#endif 
    } /* if (MVR pointer is not NULL and not unicast */
    return result;
}

/*
 *  Function
 *    _bcm_sirius_multicast_data_create
 *  Purpose
 *    Create the unit multicast management information.  This does not
 *    initialise the hardware or read the hardware -- it only builds the basic
 *    management information based upon the assumption that the hardware is
 *    blank.
 *  Arguments
 *    (in) int unit = the unit to prepare
 *    (out) _mc_unit_t **unitData = where to put the unit data pointer
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    The unit will be locked on creation.
 */
static int
_bcm_sirius_multicast_data_create(int unit,
                                  _mc_unit_t **unitData)
{
    volatile sal_mutex_t *lockPtr;
    sal_mutex_t lock = NULL;
    int result;
    _mc_unit_t *thisUnit;
    shr_mdb_elem_bank_index_t freeSizes[_MC_MAX_FREELISTS];
#if defined(BCM_EASY_RELOAD_SUPPORT)
#if _MC_LIMBO_FREE_ON_RELOAD
    shr_mdb_info_t mdbInfo;
#endif /* _MC_LIMBO_FREE_ON_RELOAD */
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */
    eg_fd_gmt_entry_t *gmtWork;
    uint32 group;
    uint32 groups;
    uint32 mvr;
    ep_oi2qb_map_entry_t oittRaw;       /* working OITT entry for HGX opcode */
    eg_fd_mdb_entry_t mdbRaw;           /* working MDB entry for fill */
    uint32 minGroup;                    /* minimum group ID */
    uint32 maxGroup;                    /* maximum group ID */
    uint32 minOitt;                     /* minimum OI ID */
    uint32 maxOitt;                     /* maximum OI ID */
    uint32 minQueue = 0;                /* minimum queue */
    uint32 maxQueue = 0;                /* maximum queue */
    uint32 min = 0;                     /* all other minima */
    uint32 max = 0;                     /* all other maxima */
    uint32 size;                        /* total base cell size */
    uint32 regval;
    uint16 devType;                     /* Device type ID */
    uint8 devRev;                       /* Device revision ID */

    MC_VERB((MC_MSG("(%d,*) enter\n"), unit));

    /* get the device type information */
    /* In theory, this should never fail. */
    result = soc_cm_get_id(unit, &devType, &devRev);

    /* get the current MVR size setting */
    if (BCM_E_NONE == result) {
        /* In theory, this should never fail. */
        result = READ_FD_CONFIGr(unit, &regval);
    }

    if ((!SOC_CONTROL(unit)) ||
        (!SOC_SBX_CONTROL(unit)) ||
        (!SOC_SBX_CFG(unit)) ||
        (!SOC_SBX_CFG_SIRIUS(unit))) {
        MC_ERR((MC_MSG1("unable to access unit %d config struct\n"), unit));
        return BCM_E_INIT;
    }
    lockPtr = &(SOC_SBX_CFG_SIRIUS(unit)->lMcAggrLock);

    if (!(*lockPtr)) {
        /* need to create shared lock */
        
        *lockPtr = lock = sal_mutex_create("unit aggr/mc lock");
        if (!lock) {
            MC_ERR((MC_MSG1("unable to create unit %d lock\n"), unit));
            return BCM_E_RESOURCE;
        }
    }

    /*
     *  Need to know which multicast groups we will support.
     */
    minGroup = SOC_MEM_INFO(unit, EG_FD_GMTm).index_min;
    maxGroup = SOC_MEM_INFO(unit, EG_FD_GMTm).index_max;
    if ((maxGroup - minGroup) > (_MC_UNICAST_BASE - 1)) {
        /* don't include the unicast space */
        maxGroup = minGroup + (_MC_UNICAST_BASE - 1);
    }

    /*
     *  Need to know how many OITTs we're going to be using.
     */
    minOitt = SOC_MEM_INFO(unit, EP_OI2QB_MAPm).index_min;
    maxOitt = ((SOC_SBX_CFG_SIRIUS(unit)->requeueMinPage) << 9) -1;
    if (SOC_SBX_CFG_SIRIUS(unit)->dualLookup) {
        /* we are in dual-lookup mode; must split ep_oi2qb_map space */
        SOC_SBX_CFG_SIRIUS(unit)->queueOffset = SOC_SBX_CFG_SIRIUS(unit)->requeueMinPage >> 1;
        maxOitt = ((SOC_SBX_CFG_SIRIUS(unit)->queueOffset) << 9) -1;
        minQueue = maxOitt + 1;
        maxQueue = ((maxOitt + 1) << 1) - 1;
        MC_VVERB((MC_MSG1("unit %d running dual-lookup mode\n"), unit));
        MC_VVERB((MC_MSG1("unit %d using OITT space [%08X..%08X] for queue\n"),
                  unit,
                  minQueue,
                  maxQueue));
    } else {
        MC_VVERB((MC_MSG1("unit %d running single-lookup mode\n"), unit));
    }
    MC_VVERB((MC_MSG1("unit %d using OITT space [%08X..%08X] for encapId\n"),
              unit,
              minOitt,
              maxOitt));
    if (minOitt > maxOitt) {
        MC_ERR((MC_MSG1("must have some OITT space for multicast but none"
                        " has been allocated to multicast\n")));
        return BCM_E_CONFIG;
    }

    /*
     *  Compute the size of the unit information.  This is made as a single
     *  large alloc cell, to reduce memory overhead and management complexity.
     */
    size = sizeof(_mc_unit_t);
    /*
     *  Add space for an array of cache element arrays.  Each group has a list
     *  of these cache elements that contain the GPORT and OI by which a target
     *  replicant was added to the group.  These lists are flat, and are only
     *  used by the 'get' function, so it can return the actual data that were
     *  used in constructing a group.
     *
     *  This also sets up space for unicast replication cache that will be used
     *  when handling XGS oversubscribed aggregated higig connections.  Note
     *  that this is merely a bitmap rather than a pointer set.
     */
    size += (sizeof(*(thisUnit->cache)) *
             (1 + SOC_MEM_INFO(unit, EG_FD_GMTm).index_max - minGroup));
    size += SHR_BITALLOCSIZE(SOC_MEM_INFO(unit, EG_FD_GMTm).index_max - maxGroup);

    /* Allocate the unit information... */
    MC_EVERB((MC_MSG1("allocate %d bytes for unit info\n"), size));
    thisUnit = sal_alloc(size, "multicast unit info");
    if (!thisUnit) {
        MC_ERR((MC_MSG1("unable to allocate unit %d info (%d bytes)\n"),
                unit,
                sizeof(_mc_unit_t)));
        result = BCM_E_MEMORY;
    }
    /* ...and fill it in. */
    if (BCM_E_NONE == result) {
        sal_memset(thisUnit, 0x00, size);
        thisUnit->unit = unit;
        thisUnit->limbo = 0;
        thisUnit->purge = 1;
        thisUnit->defragPosition = ~0;
        thisUnit->gmtGroupMin = minGroup;
        thisUnit->gmtGroupMax = maxGroup;
        thisUnit->oittMin = minOitt;
        thisUnit->oittMax = maxOitt;
        thisUnit->queueMin = minQueue;
        thisUnit->queueMax = maxQueue;
        if ((BCM56939_DEVICE_ID == devType) ||
            (BCM56931_DEVICE_ID == devType) ||
            (BCM56936_DEVICE_ID == devType) ||
            (BCM56930_DEVICE_ID == devType) ||
            (BCM56935_DEVICE_ID == devType)) {
            /* Sportster class device supports 8192 groups */
            thisUnit->groupCountMax = 8192;
        } else {
            /* Sirius class device supports 65536 groups */
            thisUnit->groupCountMax = 65536;
        }
        MC_EVERB((MC_MSG1("this device supports up to %d active groups\n"),
                  thisUnit->groupCountMax));
        thisUnit->denseMvrSize = soc_reg_field_get(unit,
                                                   FD_CONFIGr,
                                                   regval,
                                                   FD_MAX_MVRf) + 1;
        thisUnit->maxTargets = (thisUnit->denseMvrSize < 5)?
                                ((thisUnit->denseMvrSize - 1) * 32) + 9:
                                132;
        MC_EVERB((MC_MSG1("using dense MVR size of %d MDB elements (%d"
                          " max targets)\n"),
                  thisUnit->denseMvrSize,
                  thisUnit->maxTargets));
        if (SOC_SBX_CFG_SIRIUS(unit)->dualLookup) {
            thisUnit->flags |= _MC_UNIT_DUAL_LOOKUP;
        }

        /* place the cache array after the rest of the unit data */
        thisUnit->cache = (_gpoiCachePtr_t*)(&(thisUnit[1]));
        /* place the unicast use cache after cache array */
        thisUnit->unicast = (SHR_BITDCL*)(&(thisUnit->cache[1 + SOC_MEM_INFO(unit, EG_FD_GMTm).index_max - minGroup]));
    } /* if (BCM_E_NONE == result) */

    /* set up the multicast group management */
    if (BCM_E_NONE == result) {
        thisUnit->gmtGroupMax = maxGroup;
        result = shr_idxres_list_create(&(thisUnit->gmtList),
                                        minGroup,
                                        maxGroup,
                                        minGroup,
                                        maxGroup,
                                        "GMT list");
        if (BCM_E_NONE != result) {
            MC_ERR((MC_MSG1("unable to create unit %d GMT info: %d (%s)\n"),
                    unit,
                    result,
                    _SHR_ERRMSG(result)));
        }
    } /* if (BCM_E_NONE == result) */

    /* set up the MDB management */
    if (BCM_E_NONE == result) {
        /*
         *  These free block sizes are based upon the configurations suggested
         *  by the device specifications document (for the RR sizes) and the
         *  possible sizes for the MVR sizes based upon both sparse and dense
         *  modes.  Note that size '1' is required and implied in the mdb
         *  definition. The free block selections can be tweaked to be more
         *  optimal for a given application.
         *
         *  In addition, we need several 'user' lists so we can track with
         *  fairly tight granularity the elements that need to be freed later,
         *  due to overwrite or free, but which may still have frames that are
         *  lingering in the device.
         *
         *  We don't ask for the mdb manager to create a lock; this will only
         *  be accessed under this unit's multicast lock, so it's not needed.
         */
        
        freeSizes[0] = 2;
        freeSizes[1] = 3;
        freeSizes[2] = 4;
        freeSizes[3] = 5;
        freeSizes[4] = 9;
        freeSizes[5] = 18;
        freeSizes[6] = 54;
        freeSizes[7] = 100;
        freeSizes[8] = 132;
        freeSizes[9] = 256;
        freeSizes[10] = 512;
        freeSizes[11] = 1024;
        freeSizes[12] = 2048;
        min = SOC_MEM_INFO(unit, EG_FD_MDBm).index_min;
        max = SOC_MEM_INFO(unit, EG_FD_MDBm).index_max;
        MC_EVERB((MC_MSG1("creating %d:MDB resource data: %08X..%08X\n"),
                  thisUnit->unit,
                  min,
                  max));
        result = shr_mdb_create(&(thisUnit->mdbList),
                                4096,
                                13,
                                &(freeSizes[0]),
                                _MC_LIMBO_LISTS,
                                min,
                                max,
                                FALSE);
#if defined(BCM_EASY_RELOAD_SUPPORT)
        if (!SOC_IS_RELOADING(unit)) {
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */
            /* prepare a 'safe' single MDB sized MVT entry (as padding) */
            sal_memset(&mdbRaw, 0xFF, sizeof(mdbRaw));
            soc_mem_field32_set(unit, EG_FD_MDBm, &mdbRaw, MVR_ENTRYf, 0xFF3FFFFD);
#if defined(BCM_EASY_RELOAD_SUPPORT)
        }
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */
        if (BCM_E_NONE != result) {
            MC_ERR((MC_MSG1("unable to create unit %d MDB info: %d (%s)\n"),
                    unit,
                    result,
                    _SHR_ERRMSG(result)));
        } else { /* if (BCM_E_NONE != result) */
            /*
             *  Just throw away element zero; it's not a valid element for the
             *  case of an MVRp in the GMT space, and we don't want to deal
             *  with the special case that allows it for part of the MDB use
             *  (RRs) but not for the other part (MVRs).
             */
            MC_EVERB((MC_MSG1("reserve MDB[%08X..%08X] since MVRP=0 invalid\n"),
                      0,
                      0));
            result = shr_mdb_reserve(thisUnit->mdbList, 0, 0);
#if defined(BCM_EASY_RELOAD_SUPPORT)
            if (!SOC_IS_RELOADING(unit)) {
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */
                if (BCM_E_NONE == result) {
                    result = soc_mem_write(unit,
                                           EG_FD_MDBm,
                                           MEM_BLOCK_ALL,
                                           0,
                                           &mdbRaw);
                }
#if defined(BCM_EASY_RELOAD_SUPPORT)
            } /* if (!SOC_IS_RELOADING(unit)) */
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */
        } /* if (BCM_E_NONE != result) */
        if (BCM_E_NONE == result) {
            /*
             *  Hardware manual suggests to avoid MVRs that are within 5 elements
             *  of the top of the MDB.  Best way to do that is to simply reserve
             *  the top 4 elements.  Hardware team confirms that it 'should be
             *  safe' if the MVR in question does not actually wrap due to its
             *  size, but the read might wrap, and sometimes paranoia is better.
             */
            min = max - 3;
            MC_EVERB((MC_MSG1("reserve MDB[%08X..%08X] to avoid MVR wrap\n"),
                      min,
                      max));
            result = shr_mdb_reserve(thisUnit->mdbList, min, max);
            thisUnit->mdbBlocks++;
#if defined(BCM_EASY_RELOAD_SUPPORT)
            if (!SOC_IS_RELOADING(unit)) {
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */
                while ((BCM_E_NONE == result) && (min <= max)) {
                    result = soc_mem_write(unit,
                                           EG_FD_MDBm,
                                           MEM_BLOCK_ALL,
                                           min,
                                           &mdbRaw);
                    min++;
                }
#if defined(BCM_EASY_RELOAD_SUPPORT)
            } /* if (!SOC_IS_RELOADING(unit)) */
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */
        } /* if (BCM_E_NONE == result) */
    } /* if (BCM_E_NONE == result) */

    /* Set up the OI translation table management */
    if (BCM_E_NONE == result) {
        /*
         *  Here we need to work with the OITT.  While sliglty less bizarre, it
         *  has the same persistence requirement, plus it's much smaller and
         *  has significantly larger maximum alloc size.
         *
         *  We don't ask for the mdb manager to create a lock; this will only
         *  be accessed under this unit's multicast lock, so it's not needed.
         *
         *  We will use the lower half of EP_OI2QB_MAP for the OITT space.
         *
         *  This assumes the EP_OI2QB_MAP is 64K entries or larger.
         */
        
        freeSizes[0] = 2;
        freeSizes[1] = 4;
        freeSizes[2] = 8;
        freeSizes[3] = 16;
        freeSizes[4] = 32;
        freeSizes[5] = 64;
        freeSizes[6] = 128;
        freeSizes[7] = 256;
        freeSizes[8] = 512;
        freeSizes[9] = 1024;
        freeSizes[10] = 2048;
        freeSizes[11] = 4096;
        freeSizes[12] = 8192;
        MC_EVERB((MC_MSG1("creating %d:OITT resource data: [%d] %08X..%08X\n"),
                  thisUnit->unit,
                  (7 + maxOitt - minOitt) / 8,
                  minOitt,
                  maxOitt));
        result = shr_mdb_create(&(thisUnit->oittList),
                                (7 + maxOitt - minOitt) / 8,
                                13,
                                &(freeSizes[0]),
                                _MC_LIMBO_LISTS,
                                minOitt,
                                maxOitt,
                                FALSE);
        if (BCM_E_NONE != result) {
            MC_ERR((MC_MSG1("unable to create unit %d OITT info: %d (%s)\n"),
                    unit,
                    result,
                    _SHR_ERRMSG(result)));
        } else { /* if (BCM_E_NONE != result) */
            /*
             *  Reserve ep_oi2qb_map[0] for use by XGS egress parsers in
             *  replacing the HGX opcode field to convert frames to unicast
             *  when replicating on Sirius.  It is also used in replacing
             *  the destination for invalid frames.
             */
            MC_EVERB((MC_MSG1("reserve OITT[%08X..%08X] for HGX opcode & bad dest\n"),
                      0,
                      0));
            result = shr_mdb_reserve(thisUnit->oittList, 0, 0);
            thisUnit->oittBlocks++;
#if defined(BCM_EASY_RELOAD_SUPPORT)
            if (!SOC_IS_RELOADING(unit)) {
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */
                if (BCM_E_NONE == result) {
                    sal_memset(&oittRaw, 0x00, sizeof(oittRaw));
                    soc_mem_field32_set(unit,
                                        EP_OI2QB_MAPm,
                                        &oittRaw,
                                        QUEUE_BASEf,
                                        0x0FF01);
                    result = soc_mem_write(unit,
                                           EP_OI2QB_MAPm,
                                           COPYNO_ALL,
                                           0,
                                           &oittRaw);
                } /* if (BCM_E_NONE == result) */
#if defined(BCM_EASY_RELOAD_SUPPORT)
            } /* if (!SOC_IS_RELOADING(unit)) */
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */
        } /* if (BCM_E_NONE != result) */
    } /* if (BCM_E_NONE == result) */

    groups = maxGroup - minGroup + 1;
    if (_MC_UNICAST_SIZE > groups) {
        groups = _MC_UNICAST_SIZE;
    }
    gmtWork = soc_cm_salloc(unit,
                            sizeof(*gmtWork) * groups,
                            "GMT workspace");
    if (!gmtWork) {
        MC_ERR((MC_MSG1("unable to allocate unit %d GMT workspace\n"),
                unit));
        result = BCM_E_MEMORY;
    }
#if defined(BCM_EASY_RELOAD_SUPPORT)
    /* handle reload case */
    if (SOC_IS_RELOADING(unit)) {
        /*
         *  When using 'easy reload', the caller must set things up exactly as
         *  they were before.  However, due to the alloc behaviour, timing and
         *  direct-overwrite-avoidance, it is highly unlikely that such would
         *  provide an identical allocation of resources.  In order to avoid
         *  this being a problem, we calculate the allocation state from the
         *  current hardware state.
         *
         *  In order to save time, we'll DMA the whole GMT.  We'll use the
         *  hardware probe functions to collect the rest of this stuff.  If we
         *  don't DMA it, just probing for groups will take several seconds on
         *  hardware, and it may still take some time if there are many groups
         *  that actually exist.
         */
        if (BCM_E_NONE == result) {
            result = soc_cm_sinval(unit,
                                   gmtWork,
                                   sizeof(*gmtWork) * groups);
            if (BCM_E_NONE != result) {
                MC_ERR((MC_MSG1("unable to invalidate unit %d GMT reload"
                                " buffer: %d (%s)\n"),
                        unit,
                        result,
                        _SHR_ERRMSG(result)));
            }
        }
        if (BCM_E_NONE == result) {
            result = soc_mem_read_range(unit,
                                        EG_FD_GMTm,
                                        MEM_BLOCK_ANY,
                                        minGroup,
                                        maxGroup,
                                        gmtWork);
            if (BCM_E_NONE != result) {
                MC_ERR((MC_MSG1("unable to read %d:GMT[%08X..%08X] for reload:"
                                " %d (%s)\n"),
                        unit,
                        minGroup,
                        maxGroup,
                        result,
                        _SHR_ERRMSG(result)));
            }
        } /* if (BCM_E_NONE == result) */
        if (BCM_E_NONE == result) {
            /* scan multicast group space */
            for (group = minGroup;
                 (BCM_E_NONE == result) && (group <= maxGroup);
                 group++) {
                /* get this entry's MVR address */
                mvr = soc_EG_FD_GMTm_field32_get(unit,
                                                 &(gmtWork[group - minGroup]),
                                                 MVRPf);
                /* now preallocate its resources */
                result = _bcm_sirius_multicast_group_preload(thisUnit,
                                                             group,
                                                             mvr);
            } /* for (all groups and result is okay) */
        } /* if (BCM_E_NONE == result) */
        if (BCM_E_NONE == result) {
            result = soc_cm_sinval(unit,
                                   gmtWork,
                                   sizeof(*gmtWork) * groups);
            if (BCM_E_NONE != result) {
                MC_ERR((MC_MSG1("unable to invalidate unit %d GMT reload"
                                " buffer: %d (%s)\n"),
                        unit,
                        result,
                        _SHR_ERRMSG(result)));
            }
        } /* if (BCM_E_NONE == result) */
        if (BCM_E_NONE == result) {
            result = soc_mem_read_range(unit,
                                        EG_FD_GMTm,
                                        MEM_BLOCK_ANY,
                                        _MC_UNICAST_BASE,
                                        _MC_UNICAST_BASE + _MC_UNICAST_SIZE - 1,
                                        gmtWork);
            if (BCM_E_NONE != result) {
                MC_ERR((MC_MSG1("unable to read %d:GMT[%08X..%08X] for reload:"
                                " %d (%s)\n"),
                        unit,
                        _MC_UNICAST_BASE,
                        _MC_UNICAST_BASE + _MC_UNICAST_SIZE - 1,
                        result,
                        _SHR_ERRMSG(result)));
            }
        } /* if (BCM_E_NONE == result) */
        if (BCM_E_NONE == result) {
            
            for (group = 0; group < 4096; group++) {
                /* get this entry's MVR address */
                mvr = soc_EG_FD_GMTm_field32_get(unit,
                                                 &(gmtWork[group]),
                                                 MVRPf);
                /* now preallocate its resources */
                result = _bcm_sirius_multicast_group_preload(thisUnit,
                                                             group + _MC_UNICAST_BASE,
                                                             mvr);
            } /* for (all unicast and result okay) */
        } /* if (BCM_E_NONE == result) */
        if (BCM_E_NONE == result) {
            MC_VERB((MC_MSG1("unit %d will not be ready for active changes for"
                             " at least %d seconds while old frames age out\n"),
                     unit,
                     _MC_LIMBO_LISTS - 1));
        } else { /* if (BCM_E_NONE == result) */
            MC_ERR((MC_MSG1("unable to prepare unit %d for easy reload:"
                            " %d (%s)\n"),
                    unit,
                    result,
                    _SHR_ERRMSG(result)));
        } /* if (BCM_E_NONE == result) */
#if _MC_LIMBO_FREE_ON_RELOAD
        /*
         *  Now, the other side of the easy-reload problem is that frames must
         *  age out before we can assume any of the MDB or OITT elements are
         *  actually free (since there might have been changes right up to the
         *  point where the reload occurred, and since frames will continue to
         *  use the original pointers).
         *
         *  We need to reserve all of the remaining free elements of MDB and
         *  OITT, and place them into limbo, before we're really done here.
         */
        thisUnit->flags |= _MC_UNIT_LIMBO_PEAK_INH;
        if (BCM_E_NONE == result) {
            result = shr_mdb_all_free_to_user_list(thisUnit->mdbList,
                                                   thisUnit->limbo);
        }
        if (BCM_E_NONE == result) {
            result = shr_mdb_all_free_to_user_list(thisUnit->oittList,
                                                   thisUnit->limbo);
        }
#endif /* _MC_LIMBO_FREE_ON_RELOAD */
    } else { /* if (SOC_IS_RELOADING(unit)) */
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */
        /*
         *  For non-easy-reload case, life should be easiser, since we just
         *  want to clear the GMT.  We can do this for the multicast side of
         *  the GMT, but must not do so for the unicast side.  This also means
         *  that we need to honour whatever is left in the unicast side as far
         *  as group configuration.  Not pretty.
         */
        if (BCM_E_NONE == result) {
            MC_EVERB((MC_MSG1("clear out unit %d MC GMT space\n"), unit));
            sal_memset(gmtWork, 0x00, sizeof(*gmtWork) * groups);
            result = soc_cm_sflush(unit,
                                   gmtWork,
                                   sizeof(*gmtWork) * groups);
            if (BCM_E_NONE != result) {
                MC_ERR((MC_MSG1("unable to flush unit %d GMT work"
                                " buffer: %d (%s)\n"),
                        unit,
                        result,
                        _SHR_ERRMSG(result)));
            }
        } /* if (BCM_E_NONE == result) */
        if (BCM_E_NONE == result) {
            result = soc_mem_write_range(unit,
                                         EG_FD_GMTm,
                                         MEM_BLOCK_ALL,
                                         minGroup,
                                         maxGroup,
                                         gmtWork);
            if (BCM_E_NONE != result) {
                MC_ERR((MC_MSG1("unable to clear %d:GMT[%08X..%08X]:"
                                " %d (%s)\n"),
                        unit,
                        minGroup,
                        maxGroup,
                        result,
                        _SHR_ERRMSG(result)));
            }
        } /* if (BCM_E_NONE == result) */
        if (BCM_E_NONE == result) {
            result = soc_cm_sinval(unit,
                                   gmtWork,
                                   sizeof(*gmtWork) * groups);
            if (BCM_E_NONE != result) {
                MC_ERR((MC_MSG1("unable to invalidate unit %d GMT reload"
                                " buffer: %d (%s)\n"),
                        unit,
                        result,
                        _SHR_ERRMSG(result)));
            }
        } /* if (BCM_E_NONE == result) */
        if (BCM_E_NONE == result) {
            result = soc_mem_read_range(unit,
                                        EG_FD_GMTm,
                                        MEM_BLOCK_ANY,
                                        _MC_UNICAST_BASE,
                                        _MC_UNICAST_BASE + _MC_UNICAST_SIZE - 1,
                                        gmtWork);
            if (BCM_E_NONE != result) {
                MC_ERR((MC_MSG1("unable to read %d:GMT[%08X..%08X] for reload:"
                                " %d (%s)\n"),
                        unit,
                        _MC_UNICAST_BASE,
                        _MC_UNICAST_BASE + _MC_UNICAST_SIZE - 1,
                        result,
                        _SHR_ERRMSG(result)));
            }
        } /* if (BCM_E_NONE == result) */
        if (BCM_E_NONE == result) {
            
            for (group = 0; group < 4096; group++) {
                /* get this entry's MVR address */
                mvr = soc_EG_FD_GMTm_field32_get(unit,
                                                 &(gmtWork[group]),
                                                 MVRPf);
                /* now preallocate its resources */
                result = _bcm_sirius_multicast_group_preload(thisUnit,
                                                             group + _MC_UNICAST_BASE,
                                                             mvr);
            } /* for (all unicast and result okay) */
        } /* if (BCM_E_NONE == result) */
#if defined(BCM_EASY_RELOAD_SUPPORT)
    }
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */
    if (gmtWork) {
        soc_cm_sfree(unit, gmtWork);
    }

    /* pass out new unit information or clean up */
    if (BCM_E_NONE == result) {
        /* all went well */
        *unitData = thisUnit;
    } else { /* if (BCM_E_NONE == result) */
        /* something went wrong; clean up */
        if (thisUnit) {
            if (thisUnit->gmtList) {
                shr_idxres_list_destroy(thisUnit->gmtList);
            }
            if (thisUnit->mdbList) {
                shr_mdb_destroy(thisUnit->mdbList);
            }
            if (thisUnit->oittList) {
                shr_mdb_destroy(thisUnit->oittList);
            }
            sal_free(thisUnit);
        }
    } /* if (BCM_E_NONE == result) */

    /* try to avoid 'obvious' race condition */
    if (lock && (*lockPtr != lock)) {
        MC_WARN((MC_MSG1("detected race condition setting up multicast"
                         " data for unit %d; trying to compensate\n"),
                  unit));
        sal_mutex_destroy(lock);
    }

    MC_VERB((MC_MSG("(%d,&(%08X)) return %d (%s)\n"),
             unit,
             (uint32)(*unitData),
             result,
             _SHR_ERRMSG(result)));
    return result;
}


/*****************************************************************************
 *
 *  Interface -- intermodule API
 */

/*
 *  Function
 *    bcm_sirius_multicast_aggregate_create_id
 *  Purpose
 *    Create a specific aggregate group.  This sets up the multicast support
 *    for an aggregate; the caller is still responsible for the squelch table.
 *    Note that aggregates should be configured with override operations below
 *    and have their OI translation disabled.
 *  Arguments
 *    (in) int unit = the unit
 *    (in) uint32 flags = the flags for this group
 *    (in) bcm_multicast_t group = group ID
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    We allocate a zero-target MVR to store the logical/physical and source
 *    knockout modes.  It'll be replaced later as needed.
 *
 *    This will balk at creating something in the multicast groups space, and
 *    does not itself manage the system port space, in which it expects to
 *    place aggregates.
 */
int
bcm_sirius_multicast_aggregate_create_id(int unit,
                                         uint32 flags,
                                         bcm_multicast_t group)
{
    _mc_unit_t *unitData;
    int result = BCM_E_NONE;
    eg_fd_gmt_entry_t gmtData;
    uint32 mvr;

    MC_VERB((MC_MSG("(%d,%08X,%08X) enter\n"), unit, flags, group));

    /* get unit information and lock unit */
    MC_UNIT_CHECK(unit);
    MC_INIT_CHECK(unit, unitData);
    MC_LOCK_TAKE(unitData);

    /* verify arguments */
    if ((group <= unitData->gmtGroupMax) ||
        (group > SOC_MEM_INFO(unitData->unit, EG_FD_GMTm).index_max)) {
        MC_ERR((MC_MSG1("can't create unit %d aggregate %08X; invalid ID\n"),
                unit,
                group));
        /* I still think bogus ID should be PARAM instead of NOT_FOUND */
        result = BCM_E_NOT_FOUND;
    }

    /* make sure the GMT entry indicates something not multicast */
    if (BCM_E_NONE == result) {
#if defined(BCM_EASY_RELOAD_SUPPORT)
        /* handle reload case */
        /*
         *  In this case, the check is only wanted when *not* reloading, since
         *  the create call below verifies that the group exists during reload.
         */
        if (!SOC_IS_RELOADING(unit)) {
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */
            result = soc_mem_read(unitData->unit,
                                  EG_FD_GMTm,
                                  MEM_BLOCK_ANY,
                                  group,
                                  &gmtData);
            if (BCM_E_NONE == result) {
                /* parse the entry */
                mvr = soc_EG_FD_GMTm_field32_get(unitData->unit,
                                                 &(gmtData),
                                                 MVRPf);
                if ((SOC_MEM_INFO(unit, EG_FD_MDBm).index_min < mvr) &&
                    (SOC_MEM_INFO(unit, EG_FD_MDBm).index_max >= mvr)) {
                    /* the current GMT entry isn't unicast and isn't disabled */
                    result = BCM_E_EXISTS;
                    MC_ERR((MC_MSG1("%d:gmt[%08X] is multicast/aggregate"
                                    " (%08X < %08X <= %08X\n"),
                            unitData->unit,
                            group,
                            SOC_MEM_INFO(unit, EG_FD_MDBm).index_min + 1,
                            mvr,
                            SOC_MEM_INFO(unit, EG_FD_MDBm).index_max));
                }
            } else { /* if (BCM_E_NONE == result) */
                MC_ERR((MC_MSG1("unable to read %d:gmt[%08X]: %d (%s)\n"),
                        unitData->unit,
                        group,
                        result,
                        _SHR_ERRMSG(result)));
            } /* if (BCM_E_NONE == result) */
#if defined(BCM_EASY_RELOAD_SUPPORT)
        } /* if (!SOC_IS_RELOADING(unit)) */
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */
    } /* if (BCM_E_NONE == result) */

    /* create a multicast group with no targets */
    if (BCM_E_NONE == result) {
        result = _bcm_sirius_multicast_create_id(unitData, flags, group);
        /* called function displays any diagnostic */
        if (BCM_E_NONE == result) {
            /* mark it as used in the unicast use cache */
            SHR_BITSET(unitData->unicast, (group - (unitData->gmtGroupMax + 1)));
        }
    }

    /* unlock unit */
    MC_LOCK_GIVE(unitData);

    MC_VERB((MC_MSG("(%d,%08X,%08X) return %d (%s)\n"),
             unit,
             flags,
             group,
             result,
             _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_multicast_egress_add_override_oi
 *  Purpose
 *    Add an egress element to an existing multicast group, using the specified
 *    OI translation mode instead of the current OI translation mode.
 *  Arguments
 *    (in) _mc_unit_t *unitData = pointer to the unit information
 *    (in) bcm_multicast_t group = group ID
 *    (in) bcm_gport_t port = gport to add
 *    (in) bcm_if_t encap_id = OI for the gport
 *    (in) int use_oi = TRUE if using OI translation, FALSE if not
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Will not allow multiple references to the same target in physical mode.
 *
 *    Does not care about multiple references to the same target in logical
 *    mode, even if the OIs are identical (allows all).
 *
 *    OI translation in hardware is sticky per target -- if a target already
 *    exists, then its OI translation setting overrides the provided one.  If
 *    not, the provided translation setting is used.
 *
 *    Does not sort the added GPORT,OI tuples in the cache.
 */
int
bcm_sirius_multicast_egress_add_override_oi(int unit,
                                            bcm_multicast_t group,
                                            bcm_gport_t port,
                                            bcm_if_t encap_id,
                                            int use_oi)
{
    _mc_unit_t *unitData;
    int result;

    MC_VERB((MC_MSG("(%d,%08X,%08X,%08X,%s) enter\n"),
             unit,
             group,
             port,
             encap_id,
             use_oi?"TRUE":"FALSE"));

    /* get unit information and lock unit */
    MC_UNIT_CHECK(unit);
    MC_INIT_CHECK(unit, unitData);
    MC_LOCK_TAKE(unitData);

    if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
        MC_WARN((MC_MSG1("unit %d dual-lookup mode but add without queue\n"),
                 unit));
    }

    /* add to the group, overriding the OI translation setting */
    result = _bcm_sirius_multicast_egress_add_override_oi(unitData,
                                                          group,
                                                          port,
                                                          encap_id,
                                                          0 /* no queue */,
                                                          use_oi);

    /* unlock unit */
    MC_LOCK_GIVE(unitData);

    MC_VERB((MC_MSG("(%d,%08X,%08X,%08X,%s) return %d (%s)\n"),
             unit,
             group,
             port,
             encap_id,
             use_oi?"TRUE":"FALSE",
             result,
             _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_multicast_egress_subscriber_add_override_oi
 *  Purpose
 *    Add an egress element to an existing multicast group, using the specified
 *    OI translation mode instead of the current OI translation mode.
 *  Arguments
 *    (in) _mc_unit_t *unitData = pointer to the unit information
 *    (in) bcm_multicast_t group = group ID
 *    (in) bcm_gport_t port = gport to add
 *    (in) bcm_if_t encap_id = OI for the gport
 *    (in) bcm_gport_t queue_id = subscriber queue ID
 *    (in) int use_oi = TRUE if using OI translation, FALSE if not
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Will not allow multiple references to the same target in physical mode.
 *
 *    Does not care about multiple references to the same target in logical
 *    mode, even if the OIs are identical (allows all).
 *
 *    OI translation in hardware is sticky per target -- if a target already
 *    exists, then its OI translation setting overrides the provided one.  If
 *    not, the provided translation setting is used.
 *
 *    Does not sort the added GPORT,OI tuples in the cache.
 */
int
bcm_sirius_multicast_egress_subscriber_add_override_oi(int unit,
                                                       bcm_multicast_t group,
                                                       bcm_gport_t port,
                                                       bcm_if_t encap_id,
                                                       bcm_gport_t queue_id,
                                                       int use_oi)
{
    _mc_unit_t *unitData;
    int result;

    MC_VERB((MC_MSG("(%d,%08X,%08X,%08X,%08X,%s) enter\n"),
             unit,
             group,
             port,
             encap_id,
             queue_id,
             use_oi?"TRUE":"FALSE"));

    /* get unit information and lock unit */
    MC_UNIT_CHECK(unit);
    MC_INIT_CHECK(unit, unitData);
    MC_LOCK_TAKE(unitData);

    if (!(unitData->flags & _MC_UNIT_DUAL_LOOKUP)) {
        MC_WARN((MC_MSG1("unit %d single-lookup mode but add with queue\n"),
                 unit));
    }

    /* add to the group, overriding the OI translation setting */
    result = _bcm_sirius_multicast_egress_add_override_oi(unitData,
                                                          group,
                                                          port,
                                                          encap_id,
                                                          queue_id,
                                                          use_oi);

    /* unlock unit */
    MC_LOCK_GIVE(unitData);

    MC_VERB((MC_MSG("(%d,%08X,%08X,%08X,%08X,%s) return %d (%s)\n"),
             unit,
             group,
             port,
             encap_id,
             queue_id,
             use_oi?"TRUE":"FALSE",
             result,
             _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_multicast_egress_set_override_oi
 *  Purpose
 *    Set a group to contain exactly the specified elements, using the
 *    specified OI translation mode instead of the current OI translation mode.
 *  Arguments
 *    (in) int unit = the unit
 *    (in) bcm_multicast_t group = group ID
 *    (in) int port_count = number of ports (why can this be negative?)
 *    (in) bcm_gport_t *port_array = pointer to array of gport IDs
 *    (in) bcm_if_t *encap_id_array = pointer to array of encap_ids
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    port_array and encap_id_array elements are paired 1:1.
 *
 *    Replaces entire existing group contents with new contents, effectively
 *    deleting anything not in the new set.  The API doc doesn't actually *say*
 *    this, but it should.
 *
 *    Somebody should explain why this doesn't take an *unsigned* count.  I
 *    don't see any information about what to do with negative count, so this
 *    implementation will call it BCM_E_PARAM.  Zero count is valid (it just
 *    means there are no replicants in the group).  Yes, you can provide NULL
 *    pointers to the arrays in the case of zero elements.  However, it's
 *    probably more efficient to just call the delete_all function instead.
 *
 *    Wants a BIG pile of stack space (_SHR_PBMP_PORT_MAX * (sizeof(bcm_port_t)
 *    + sizeof(_mc_oitt_elem_internal_t)) so it does not have to do heap
 *    thrashing for resizing data.
 *
 *    Will not allow multiple replicants on same target for physical mode.
 *
 *    The OI translation override can be useful in setting up aggregates in
 *    systems that do use OI translation for multicast but do not need it for
 *    unicast.  OI translation should always be disabled for the entire unit
 *    (improves performance and removes a particular resource limitation) on
 *    systems that do not use OIs for multicast or unicast.  Not sure if
 *    there's any good reason to override in the TRUE direction...
 *
 *    Does not sort the added GPORT,OI tuples in the cache.
 *
 *    Automatically disables OITT for XGS mode higig targets in logical groups;
 *    maintains user statement for OITT mode in all other cases.
 */
int
bcm_sirius_multicast_egress_set_override_oi(int unit,
                                            bcm_multicast_t group,
                                            int port_count,
                                            bcm_gport_t *port_array,
                                            bcm_if_t *encap_id_array,
                                            int use_oi)
{
    _mc_unit_t *unitData;
    int result;

    MC_VERB((MC_MSG("(%d,%08X,%d,*,*,%s) enter\n"),
             unit,
             group,
             port_count,
             use_oi?"TRUE":"FALSE"));

    /* get unit information and lock unit */
    MC_UNIT_CHECK(unit);
    MC_INIT_CHECK(unit, unitData);
    MC_LOCK_TAKE(unitData);

    /* set the group to the specified targets and OIs */
    result = _bcm_sirius_multicast_egress_set_override_oi(unitData,
                                                          group,
                                                          port_count,
                                                          port_array,
                                                          encap_id_array,
                                                          NULL,
                                                          use_oi);

    /* unlock unit */
    MC_LOCK_GIVE(unitData);

    MC_VERB((MC_MSG("(%d,%08X,%d,*,*,%s) return %d (%s)\n"),
             unit,
             group,
             port_count,
             use_oi?"TRUE":"FALSE",
             result,
             _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_multicast_egress_subscriber_set_override_oi
 *  Purpose
 *    Set a group to contain exactly the specified elements, using the
 *    specified OI translation mode instead of the current OI translation mode.
 *  Arguments
 *    (in) int unit = the unit
 *    (in) bcm_multicast_t group = group ID
 *    (in) int port_count = number of ports (why can this be negative?)
 *    (in) bcm_gport_t *port_array = pointer to array of gport IDs
 *    (in) bcm_if_t *encap_id_array = pointer to array of encap_ids
 *    (in) bcm_gport_t *queue_id_array = pointer to array of subscr. queue IDs
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    port_array and encap_id_array elements are paired 1:1.
 *
 *    Replaces entire existing group contents with new contents, effectively
 *    deleting anything not in the new set.  The API doc doesn't actually *say*
 *    this, but it should.
 *
 *    Somebody should explain why this doesn't take an *unsigned* count.  I
 *    don't see any information about what to do with negative count, so this
 *    implementation will call it BCM_E_PARAM.  Zero count is valid (it just
 *    means there are no replicants in the group).  Yes, you can provide NULL
 *    pointers to the arrays in the case of zero elements.  However, it's
 *    probably more efficient to just call the delete_all function instead.
 *
 *    Wants a BIG pile of stack space (_SHR_PBMP_PORT_MAX * (sizeof(bcm_port_t)
 *    + sizeof(_mc_oitt_elem_internal_t)) so it does not have to do heap
 *    thrashing for resizing data.
 *
 *    Will not allow multiple replicants on same target for physical mode.
 *
 *    The OI translation override can be useful in setting up aggregates in
 *    systems that do use OI translation for multicast but do not need it for
 *    unicast.  OI translation should always be disabled for the entire unit
 *    (improves performance and removes a particular resource limitation) on
 *    systems that do not use OIs for multicast or unicast.  Not sure if
 *    there's any good reason to override in the TRUE direction...
 *
 *    Does not sort the added GPORT,OI tuples in the cache.
 *
 *    Automatically disables OITT for XGS mode higig targets in logical groups;
 *    maintains user statement for OITT mode in all other cases.
 */
int
bcm_sirius_multicast_egress_subscriber_set_override_oi(int unit,
                                                       bcm_multicast_t group,
                                                       int port_count,
                                                       bcm_gport_t *port_array,
                                                       bcm_if_t *encap_id_array,
                                                       bcm_gport_t *queue_id_array,
                                                       int use_oi)
{
    _mc_unit_t *unitData;
    int result;

    MC_VERB((MC_MSG("(%d,%08X,%d,*,*,*,%s) enter\n"),
             unit,
             group,
             port_count,
             use_oi?"TRUE":"FALSE"));

    /* get unit information and lock unit */
    MC_UNIT_CHECK(unit);
    MC_INIT_CHECK(unit, unitData);
    MC_LOCK_TAKE(unitData);

    /* set the group to the specified targets and OIs */
    result = _bcm_sirius_multicast_egress_set_override_oi(unitData,
                                                          group,
                                                          port_count,
                                                          port_array,
                                                          encap_id_array,
                                                          queue_id_array,
                                                          use_oi);

    /* unlock unit */
    MC_LOCK_GIVE(unitData);

    MC_VERB((MC_MSG("(%d,%08X,%d,*,*,*,%s) return %d (%s)\n"),
             unit,
             group,
             port_count,
             use_oi?"TRUE":"FALSE",
             result,
             _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_aggregate_to_multicast_id
 *  Purpose
 *    Translate the BCM layer aggregate ('trunk') ID to something used within
 *    multicast to specify the GMT entry that applies to the aggregate.
 *  Arguments
 *    (in) int unit = the unit
 *    (in) bcm_trunk_t aggregate = aggregate ID to use
 *    (out) bcm_multicast_t *group = where to put the aggregate's group ID
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 */

int
bcm_sirius_aggregate_to_multicast_id(int unit,
                                     bcm_trunk_t aggregate,
                                     bcm_multicast_t *group)
{
    if (!group) {
        MC_ERR((MC_MSG1("NULL pointer to out argument unacceptable\n")));
        return BCM_E_PARAM;
    }
    if ((0 > aggregate) || (_MC_UNICAST_SIZE <= aggregate)) {
        MC_ERR((MC_MSG1("invalid aggregate ID %08X\n"), aggregate));
        return BCM_E_PARAM;
    }
    *group = aggregate + _MC_UNICAST_BASE;
    return BCM_E_NONE;
}


/*****************************************************************************
 *
 *  Interface -- standard API
 */

/*
 *  Function
 *    bcm_sirius_multicast_init
 *  Purpose
 *    Initialise the specified unit, creating the management data and clearing
 *    the hardware.
 *  Arguments
 *    (in) int unit = the unit to prepare
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Will blindly obliterate any current state.
 */
int
bcm_sirius_multicast_init(int unit)
{
    int result = BCM_E_NONE;
    sal_mutex_t tempLock = NULL;
    _mc_unit_t *tempUnit = NULL;

    MC_VERB((MC_MSG("(%d) enter\n"), unit));

    if (!SAL_BOOT_BCMSIM && soc_property_get(unit, spn_DIAG_EMULATOR_PARTIAL_INIT, 0)) {
        /* Certain blocks are not fully inited in this case, can not write to memory in   
         * those blocks, skip BCM init. This is here for emulator and bringup   
         * could be cleaned up after sirius passes bringup stage.   
         */
        return BCM_E_NONE;
    }

    /* see if this module has been initialised anywhere locally */
    if (!_mc_lock) {
        /* not initialised yet; set up global information */
        tempLock = sal_mutex_create("multicast lock");
        if (!tempLock) {
            MC_ERR((MC_MSG1("unable to create global lock\n")));
            return BCM_E_RESOURCE;
        }
        if (sal_mutex_take(tempLock, sal_mutex_FOREVER)) {
            MC_ERR((MC_MSG1("unable to claim new global lock\n")));
            sal_mutex_destroy(tempLock);
            return BCM_E_INTERNAL;
        }
        _mc_lock = tempLock;
        memset(&_mc_unit, 0, sizeof(_mc_unit));
        if (sal_mutex_give(tempLock)) {
            MC_ERR((MC_MSG1("unable to release new global lock\n")));
            return BCM_E_INTERNAL;
        }
    }
    /* claim global lock */
    if (sal_mutex_take(_mc_lock, sal_mutex_FOREVER)) {
        MC_ERR((MC_MSG1("unable to claim global lock\n")));
        result = BCM_E_INTERNAL;
    }
    /* try to recover from first create race condition */
    if (tempLock && (tempLock != _mc_lock)) {
        /* looks like somebody clobbered our lock; toss it */
        sal_mutex_destroy(tempLock);
    }
    /* make sure there is a background process */
    if ((BCM_E_NONE == result) && (!_mc_background)) {
        /* everything good so far, but no background process */
        _mc_background = sal_thread_create("sirius_mc_bg",
                                           SAL_THREAD_STKSZ,
                                           50,
                                           _bcm_sirius_mc_bg,
                                           NULL);
        if (SAL_THREAD_ERROR == _mc_background) {
            MC_ERR((MC_MSG1("unable to create background thread\n")));
            _mc_background = NULL;
            result = BCM_E_INTERNAL;
        }
    }
    if (BCM_E_NONE != result) {
        /* at this point, we can safely exit if error */
        return result;
    }
    /* detach first if already initialised */
    if (_mc_unit[unit]) {
        /* this unit is already initialised; detach and then reinit */
        MC_VERB((MC_MSG1("unit %d already inited; detaching then reiniting\n"),
                 unit));
        result = _bcm_sirius_multicast_data_destroy(_mc_unit[unit]);
        if (BCM_E_NONE == result) {
            /* dispose of old unit information */
            sal_free(_mc_unit[unit]);
            /* ensure correct behaviour if reinit fails */
            _mc_unit[unit] = NULL;
        }
    }
    /* create management structures */
    if (BCM_E_NONE == result) {
        result = _bcm_sirius_multicast_data_create(unit,&tempUnit);
    }

    if (BCM_E_NONE == result) {
        /* success; commit unit init and unlock it */
        _mc_unit[unit] = tempUnit;
    }
    /* release global lock */
    if (sal_mutex_give(_mc_lock)) {
        MC_ERR((MC_MSG1("unable to release global lock\n")));
        result = BCM_E_INTERNAL;
    }

    MC_VERB((MC_MSG("(%d) return %d (%s)\n"),
             unit,
             result,
             _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_multicast_detach
 *  Purpose
 *    Detach the specified unit, destroying the management data and clearing
 *    the hardware.
 *  Arguments
 *    (in) int unit = the unit to detach
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Will blindly obliterate any current state.
 */
int
bcm_sirius_multicast_detach(int unit)
{
    int result = BCM_E_NONE;
    _mc_unit_t *tempUnit = NULL;

    MC_VERB((MC_MSG("(%d) enter\n"), unit));

    /* see if this module has been initialised anywhere locally */
    if (!_mc_lock) {
        MC_ERR((MC_MSG1("no initialised modules\n")));
        return BCM_E_INIT;
    }
    /* claim global lock */
    if (sal_mutex_take(_mc_lock, sal_mutex_FOREVER)) {
        MC_ERR((MC_MSG1("unable to claim global lock\n")));
        return BCM_E_INTERNAL;
    }
    /*
     *  The background process will die quietly of boredom if there are no
     *  units to service, so we don't do anything about that here.
     */
    /* detach and destroy state */
    tempUnit = _mc_unit[unit];
    _mc_unit[unit] = NULL;
    if (tempUnit) {
        /* this unit is already initialised; detach and then reinit */
        result = _bcm_sirius_multicast_data_destroy(tempUnit);
    } else {
        MC_ERR((MC_MSG1("unit %d is not initialised yet\n"),
                unit));
        result = BCM_E_INIT;
    }
    if (BCM_E_NONE == result) {
        /* successfully detached unit; free resources */
        sal_free(tempUnit);
    } else {
        /* failed to detach unit; restore resources */
        _mc_unit[unit] = tempUnit;
    }
    /* release global lock */
    if (sal_mutex_give(_mc_lock)) {
        MC_ERR((MC_MSG1("unable to release global lock\n")));
        return BCM_E_INTERNAL;
    }

    MC_VERB((MC_MSG("(%d) return %d (%s)\n"),
             unit,
             result,
             _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_multicast_create
 *  Purpose
 *    Create a (maybe specific) multicast group.  Basically treats group as
 *    physical or logical replication, which is decided based upon some of the
 *    flags (L2 is physical, everything else is logical; physical only allows
 *    one replicant per interface, while logical allows up to 4096).
 *  Arguments
 *    (in) int unit = the unit
 *    (in) uint32 flags = the flags for this group
 *    (in/out) bcm_multicast_t *group = pointer to group ID location
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    The group argument is 'in' only if WITH_ID is set, and is 'out' only if
 *    WITH_ID is not set.
 *
 *    We allocate a zero-target MVR to store the logical/physical and source
 *    knockout modes.  It'll be replaced later as needed.
 */
int
bcm_sirius_multicast_create(int unit,
                            uint32 flags,
                            bcm_multicast_t *group)
{
    shr_idxres_element_t gmtEntry;
    _mc_unit_t *unitData;
    int result = BCM_E_NONE;
    int gmtAlloc = FALSE;

    MC_VERB((MC_MSG("(%d,%08X,*) enter\n"), unit, flags));

    /* verify arguments */
    if (!group) {
        MC_ERR((MC_MSG1("NULL pointer not acceptable for group\n")));
        return BCM_E_PARAM;
    }

    /* get unit information and lock unit */
    MC_UNIT_CHECK(unit);
    MC_INIT_CHECK(unit, unitData);
    MC_LOCK_TAKE(unitData);

    /* make sure this unit supports enough groups for the new one */
    if (unitData->groupCount >= unitData->groupCountMax) {
        MC_ERR((MC_MSG1("unit %d already has its maximum of %d groups\n"),
                unitData->unit,
                unitData->groupCountMax));
        result = BCM_E_RESOURCE;
    }

    if (BCM_E_NONE == result) {
        /* allocate the GMT entry */
        if (flags & BCM_MULTICAST_WITH_ID) {
            /* allocate requested GMT entry */
            gmtEntry = *group;
            result = shr_idxres_list_reserve(unitData->gmtList,
                                             gmtEntry,
                                             gmtEntry);
        } else {
#if defined(BCM_EASY_RELOAD_SUPPORT)
            /* handle reload case */
            if (!SOC_IS_RELOADING(unit)) {
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */
                /* allocate any available GMT entry */
                result = shr_idxres_list_alloc(unitData->gmtList, &gmtEntry);
#if defined(BCM_EASY_RELOAD_SUPPORT)
            } else { /* if (!SOC_IS_RELOADING(unit)) */
                MC_ERR((MC_MSG1("must create WITH_ID on unit %d during"
                                " reload\n"),
                        unit));
                result = BCM_E_CONFIG;
            } /* if (!SOC_IS_RELOADING(unit)) */
#endif /* defined (BCM_EASY_RELOAD_SUPPORT) */
        }
        if (BCM_E_NONE == result) {
            gmtAlloc = TRUE;
        } else {
            MC_ERR((MC_MSG1("unable to allocate unit %d GMT entry: %d (%s)\n"),
                    unit,
                    result,
                    _SHR_ERRMSG(result)));
            if (BCM_E_PARAM == result) {
                /* bad param from idxres == out of range; BCM says NOT_FOUND */
                result = BCM_E_NOT_FOUND;
            }
        }
    } /* if (BCM_E_NONE == result) */

    /* create a multicast group with no targets */
    if (BCM_E_NONE == result) {
        result = _bcm_sirius_multicast_create_id(unitData, flags, gmtEntry);
    }
    if (BCM_E_NONE == result) {
        /* looks good; return group ID */
        *group = gmtEntry;
        unitData->groupCount++;
    } else { /* if (BCM_E_NONE == result) */
        /* something went wrong */
        if (gmtAlloc) {
            shr_idxres_list_free(unitData->gmtList, gmtEntry);
        }
    } /* if (BCM_E_NONE == result) */

    /* unlock unit */
    MC_LOCK_GIVE(unitData);

    MC_VERB((MC_MSG("(%d,%08X,&(%08X)) return %d (%s)\n"),
             unit,
             flags,
             *group,
             result,
             _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_multicast_destroy
 *  Purpose
 *    Destroy a specific multicast group.
 *  Arguments
 *    (in) int unit = the unit
 *    (in) bcm_multicast_t group = group ID
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 */
int
bcm_sirius_multicast_destroy(int unit,
                             bcm_multicast_t group)
{
    _mc_unit_t *unitData;
    int result;

    MC_VERB((MC_MSG("(%d,%08X) enter\n"), unit, group));

    /* get unit information */
    MC_UNIT_CHECK(unit);
    MC_INIT_CHECK(unit, unitData);

#if defined(BCM_EASY_RELOAD_SUPPORT)
    /* handle reload case */
    if (SOC_IS_RELOADING(unit)) {
        MC_ERR((MC_MSG1("may not destroy groups on unit %d during reload\n"),
                unit));
        return BCM_E_CONFIG;
    } /* if (SOC_IS_RELOADING(unit)) */
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */

    /* lock unit */
    MC_LOCK_TAKE(unitData);

    /* destroy the group */
    result = _bcm_sirius_multicast_group_flush(unitData,
                                               group,
                                               _MC_GROUP_FLUSH_DESTROY,
                                               0);
    if (BCM_E_NONE == result) {
        unitData->cache[group].elements = 0;
        if (unitData->cache[group].data) {
            sal_free(unitData->cache[group].data);
            unitData->cache[group].data = NULL;
            unitData->cells--;
        }
        unitData->groupCount--;
    }

    /* unlock unit */
    MC_LOCK_GIVE(unitData);

    MC_VERB((MC_MSG("(%d,%08X) return %d (%s)\n"),
             unit,
             group,
             result,
             _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_multicast_group_get
 *  Purpose
 *    Return the flags that would be needed to recreate the group, perhaps
 *    on another unit, or perhaps after restart.
 *  Arguments
 *    (in) int unit = the unit
 *    (in) bcm_multicast_t group = group ID
 *    (out) uint32 *flags = where to put the flags
 *  Return
 *    bcm_error_t cast as int
 *    BCM_E_NONE if successful
 *    BCM_E_* otherwise as appropriate
 *  Notes
 */
int
bcm_sirius_multicast_group_get(int unit,
                               bcm_multicast_t group,
                               uint32 *flags)
{
    _mc_unit_t *unitData;
    int result;

    MC_VERB((MC_MSG("(%d,%08X,*) enter\n"), unit, group));

    if (!flags) {
        MC_ERR((MC_MSG1("obligatory out argument is NULL\n")));
        return BCM_E_PARAM;
    }

    /* get unit information */
    MC_UNIT_CHECK(unit);
    MC_INIT_CHECK(unit, unitData);

    /* lock unit */
    MC_LOCK_TAKE(unitData);

    result = _bcm_sirius_multicast_group_get(unitData, group, flags);

    /* unlock unit */
    MC_LOCK_GIVE(unitData);

    MC_VERB((MC_MSG("(%d,%08X,&(%08X)) return %d (%s)\n"),
             unit,
             group,
             *flags,
             result,
             _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_multicast_group_traverse
 *  Purpose
 *    Return the flags that would be needed to recreate the group, perhaps
 *    on another unit, or perhaps after restart.
 *  Arguments
 *    (in) int unit = the unit
 *    (in) bcm_multicast_group_traverse_cb_t cb = pointer to callback function
 *    (in) uint32 flags = comparison limiter (TYPE flags only)
 *    (in) void *user_data = pointer to user data (opaque)
 *  Return
 *    bcm_error_t cast as int
 *    BCM_E_NONE if successful
 *    BCM_E_* otherwise as appropriate
 *  Notes
 *    Only makes callback for groups whose type fits within flags provided (any
 *    bits set in the flags permit groups of that type; bits outside of the
 *    BCM_MULTICAST_TYPE_MASK are ignored).
 *
 *    This does not traverse the unicast space.  That space is currently hidden
 *    from the application, and so should not be traversed here.  If it gets
 *    exposed in the future, it probably should be included here somehow.
 */
int
bcm_sirius_multicast_group_traverse(int unit,
                                    bcm_multicast_group_traverse_cb_t cb,
                                    uint32 flags,
                                    void *user_data)
{
    bcm_multicast_t gid;
    _mc_unit_t *unitData;
    uint32 tempFlags;
    int result = BCM_E_NONE;

    MC_VERB((MC_MSG("(%d,*,%08X,*) enter\n"), unit, flags));

    if (!cb) {
        MC_ERR((MC_MSG1("callback handler must not be NULL\n")));
        return BCM_E_PARAM;
    }

    /* get unit information */
    MC_UNIT_CHECK(unit);
    MC_INIT_CHECK(unit, unitData);

    /* lock unit */
    MC_LOCK_TAKE(unitData);

    for (gid = unitData->gmtGroupMin;
         (BCM_E_NONE == result) && (gid <= unitData->gmtGroupMax);
         gid++) {
        result = _bcm_sirius_multicast_group_get(unitData, gid, &tempFlags);
        if (BCM_E_NONE == result) {
            if ((flags & BCM_MULTICAST_TYPE_MASK) & tempFlags) {
                /* at least one of the requested 'type' bits is set */
                result = cb(unit, gid, tempFlags, user_data);
                if (BCM_E_NONE != result) {
                    MC_WARN((MC_MSG1("early abort at unit %d group %d due to"
                                     " error reported by callback function:"
                                     " %d (%s)\n"),
                             unit,
                             gid,
                             result,
                             _SHR_ERRMSG(result)));
                } /* if (BCM_E_NONE != result) */
            } /* if ((flags & BCM_MULTICAST_TYPE_MASK) & tempFlags) */
        } else if (BCM_E_NOT_FOUND == result) {
            /* no group here; just go to next one */
            result = BCM_E_NONE;
        } else {
            /* something went wrong */
            MC_ERR((MC_MSG1("unexpected error getting unit %d group %d:"
                            " %d (%s)\n"),
                    unit,
                    gid,
                    result,
                    _SHR_ERRMSG(result)));
        }
    } /* for (all possible groups as long as everything goes well) */

    /* unlock unit */
    MC_LOCK_GIVE(unitData);

    MC_VERB((MC_MSG("(%d,*,%08X,*) return %d (%s)\n"),
             unit,
             flags,
             result,
             _SHR_ERRMSG(result)));

    return result;
}

/*
 *  Function
 *    bcm_sirius_multicast_egress_add
 *  Purpose
 *    Add an egress element to an existing multicast group.
 *  Arguments
 *    (in) int unit = the unit
 *    (in) bcm_multicast_t group = group ID
 *    (in) bcm_gport_t port = gport to add
 *    (in) bcm_if_t encap_id = OI for the gport
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Will not allow multiple references to the same target in physical mode.
 *
 *    Does not care about multiple references to the same target in logical
 *    mode, even if the OIs are identical (allows all).
 *
 *    OI translation in hardware is sticky per target -- if a target already
 *    exists, then its OI translation setting overrides the provided one.  If
 *    not, the provided translation setting is used.
 *
 *    Does not sort the added GPORT,OI tuples in the cache.
 */
int
bcm_sirius_multicast_egress_add(int unit,
                                bcm_multicast_t group,
                                bcm_gport_t port,
                                bcm_if_t encap_id)
{
    _mc_unit_t *unitData;
    int result;

    MC_VERB((MC_MSG("(%d,%08X,%08X,%08X) enter\n"),
             unit,
             group,
             port,
             encap_id));

    /* get unit information and lock unit */
    MC_UNIT_CHECK(unit);
    MC_INIT_CHECK(unit, unitData);
    MC_LOCK_TAKE(unitData);

    if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
        MC_WARN((MC_MSG1("unit %d dual-lookup mode but add without queue\n"),
                 unit));
    }

    /* add to the group, using current OI translation setting */
    result = _bcm_sirius_multicast_egress_add_override_oi(unitData,
                                                          group,
                                                          port,
                                                          encap_id,
                                                          0 /* no queue */,
                                                          !(unitData->flags &
                                                            _MC_UNIT_NO_OITT));

    /* unlock unit */
    MC_LOCK_GIVE(unitData);

    MC_VERB((MC_MSG("(%d,%08X,%08X,%08X) return %d (%s)\n"),
             unit,
             group,
             port,
             encap_id,
             result,
             _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_multicast_egress_subscriber_add
 *  Purpose
 *    Add an egress element to an existing multicast group.
 *  Arguments
 *    (in) int unit = the unit
 *    (in) bcm_multicast_t group = group ID
 *    (in) bcm_gport_t port = gport to add
 *    (in) bcm_if_t encap_id = OI for the gport
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Will not allow multiple references to the same target in physical mode.
 *
 *    Does not care about multiple references to the same target in logical
 *    mode, even if the OIs are identical (allows all).
 *
 *    OI translation in hardware is sticky per target -- if a target already
 *    exists, then its OI translation setting overrides the provided one.  If
 *    not, the provided translation setting is used.
 *
 *    Does not sort the added GPORT,OI tuples in the cache.
 */
int
bcm_sirius_multicast_egress_subscriber_add(int unit,
                                           bcm_multicast_t group,
                                           bcm_gport_t port,
                                           bcm_if_t encap_id,
                                           bcm_gport_t queue_id)
{
    _mc_unit_t *unitData;
    int result;

    MC_VERB((MC_MSG("(%d,%08X,%08X,%08X,%08X) enter\n"),
             unit,
             group,
             port,
             encap_id,
             queue_id));

    /* get unit information and lock unit */
    MC_UNIT_CHECK(unit);
    MC_INIT_CHECK(unit, unitData);
    MC_LOCK_TAKE(unitData);

    if (!(unitData->flags & _MC_UNIT_DUAL_LOOKUP)) {
        MC_WARN((MC_MSG1("unit %d single-lookup mode but add with queue\n"),
                 unit));
    }

    /* add to the group, using current OI translation setting */
    result = _bcm_sirius_multicast_egress_add_override_oi(unitData,
                                                          group,
                                                          port,
                                                          encap_id,
                                                          queue_id,
                                                          !(unitData->flags &
                                                            _MC_UNIT_NO_OITT));

    /* unlock unit */
    MC_LOCK_GIVE(unitData);

    MC_VERB((MC_MSG("(%d,%08X,%08X,%08X,%08X) return %d (%s)\n"),
             unit,
             group,
             port,
             encap_id,
             queue_id,
             result,
             _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_multicast_egress_delete
 *  Purpose
 *    Remove an egress element from an existing multicast group.
 *  Arguments
 *    (in) int unit = the unit
 *    (in) bcm_multicast_t group = group ID
 *    (in) bcm_gport_t port = gport to remove
 *    (in) bcm_if_t encap_id = OI for the gport
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Does not take lock or verify unit.
 *
 *    The particular GPORT,OI must exist within the group.
 *
 *    In logical mode, this has to track the OITT block address and OITT
 *    descriptor pointer for all targets removed due to the specified GPORT,OI
 *    being the last replicant on a target, plus it must track the actual index
 *    into each target's replicant list for the current GPORT,OI.  This means
 *    that it requires a fair bit of stack space.
 *
 *    Tries to reduce memory usage by rearranging things 'in place'.
 */
int
bcm_sirius_multicast_egress_delete(int unit,
                                   bcm_multicast_t group,
                                   bcm_gport_t port,
                                   bcm_if_t encap_id)
{
    _mc_unit_t *unitData;
    int result;

    MC_VERB((MC_MSG("(%d,%08X,%08X,%08X) enter\n"),
             unit,
             group,
             port,
             encap_id));

    /* get unit information and lock unit */
    MC_UNIT_CHECK(unit);
    MC_INIT_CHECK(unit, unitData);
    MC_LOCK_TAKE(unitData);

    if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
        MC_WARN((MC_MSG1("unit %d dual-lookup mode but delete without queue\n"),
                 unit));
    }

    /* remove from to the group, using current OI translation setting */
    result = _bcm_sirius_multicast_egress_delete(unitData,
                                                 group,
                                                 port,
                                                 encap_id,
                                                 0,
                                                 FALSE);

    /* unlock unit */
    MC_LOCK_GIVE(unitData);

    MC_VERB((MC_MSG("(%d,%08X,%08X,%08X) return %d (%s)\n"),
             unit,
             group,
             port,
             encap_id,
             result,
             _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_multicast_egress_subscriber_delete
 *  Purpose
 *    Remove an egress element from an existing multicast group.
 *  Arguments
 *    (in) int unit = the unit
 *    (in) bcm_multicast_t group = group ID
 *    (in) bcm_gport_t port = gport to remove
 *    (in) bcm_if_t encap_id = OI for the gport
 *    (in) bcm_gport_t queue_id = queue ID for the gport
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Does not take lock or verify unit.
 *
 *    The particular GPORT,OI must exist within the group.
 *
 *    In logical mode, this has to track the OITT block address and OITT
 *    descriptor pointer for all targets removed due to the specified GPORT,OI
 *    being the last replicant on a target, plus it must track the actual index
 *    into each target's replicant list for the current GPORT,OI.  This means
 *    that it requires a fair bit of stack space.
 *
 *    Tries to reduce memory usage by rearranging things 'in place'.
 */
int
bcm_sirius_multicast_egress_subscriber_delete(int unit,
                                              bcm_multicast_t group,
                                              bcm_gport_t port,
                                              bcm_if_t encap_id,
                                              bcm_gport_t queue_id)
{
    _mc_unit_t *unitData;
    int result;

    MC_VERB((MC_MSG("(%d,%08X,%08X,%08X,%08X) enter\n"),
             unit,
             group,
             port,
             encap_id,
             queue_id));

    /* get unit information and lock unit */
    MC_UNIT_CHECK(unit);
    MC_INIT_CHECK(unit, unitData);
    MC_LOCK_TAKE(unitData);

    if (!(unitData->flags & _MC_UNIT_DUAL_LOOKUP)) {
        MC_WARN((MC_MSG1("unit %d single-lookup mode but delete with queue\n"),
                 unit));
    }

    /* remove from to the group, using current OI translation setting */
    result = _bcm_sirius_multicast_egress_delete(unitData,
                                                 group,
                                                 port,
                                                 encap_id,
                                                 queue_id,
                                                 TRUE);

    /* unlock unit */
    MC_LOCK_GIVE(unitData);

    MC_VERB((MC_MSG("(%d,%08X,%08X,%08X,%08X) return %d (%s)\n"),
             unit,
             group,
             port,
             encap_id,
             queue_id,
             result,
             _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_multicast_egress_delete_all
 *  Purpose
 *    Remove all replicants on all targets from a multicast group
 *  Arguments
 *    (in) int unit = the unit
 *    (in) bcm_multicast_t group = group
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 */
int
bcm_sirius_multicast_egress_delete_all(int unit,
                                       bcm_multicast_t group)
{
    _mc_unit_t *unitData;
    int result;

    MC_VERB((MC_MSG("(%d,%08X) enter\n"), unit, group));

    /* get unit information and lock unit */
    MC_UNIT_CHECK(unit);
    MC_INIT_CHECK(unit, unitData);
    MC_LOCK_TAKE(unitData);

    /* dispose of all replicants in the group */
    result = _bcm_sirius_multicast_group_flush(unitData,
                                               group,
                                               _MC_GROUP_FLUSH_KEEP,
                                               0);
    if (BCM_E_NONE == result) {
        unitData->cache[group].elements = 0;
        if (unitData->cache[group].data) {
            sal_free(unitData->cache[group].data);
            unitData->cache[group].data = NULL;
            unitData->cells--;
        }
    }

    /* unlock unit */
    MC_LOCK_GIVE(unitData);

    MC_VERB((MC_MSG("(%d,%08X) return %d (%s)\n"),
             unit,
             group,
             result,
             _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_multicast_egress_set
 *  Purpose
 *    Set a group to contain exactly the specified elements.
 *  Arguments
 *    (in) int unit = the unit
 *    (in) bcm_multicast_t group = group ID
 *    (in) int port_count = number of ports (why can this be negative?)
 *    (in) bcm_gport_t *port_array = pointer to array of gport IDs
 *    (in) bcm_if_t *encap_id_array = pointer to array of encap_ids
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    port_array and encap_id_array elements are paired 1:1.
 *
 *    Replaces entire existing group contents with new contents, effectively
 *    deleting anything not in the new set.  The API doc doesn't actually *say*
 *    this, but it should.
 *
 *    Somebody should explain why this doesn't take an *unsigned* count.  I
 *    don't see any information about what to do with negative count, so this
 *    implementation will call it BCM_E_PARAM.  Zero count is valid (it just
 *    means there are no replicants in the group).  Yes, you can provide NULL
 *    pointers to the arrays in the case of zero elements.  However, it's
 *    probably more efficient to just call the delete_all function instead.
 *
 *    Wants a BIG pile of stack space (_SHR_PBMP_PORT_MAX * (sizeof(bcm_port_t)
 *    + sizeof(_mc_oitt_elem_internal_t)) so it does not have to do heap
 *    thrashing for resizing data.
 *
 *    Performs insertion sort of encap_ids on each target while building the
 *    new mutlicast group, same for targets in the set.  This means that it
 *    will be a bit faster if the list is already sorted by the underlying
 *    target of the gport, and then by the encap_id within the target.  Note
 *    that gport per se is of no internal consequence (it uses the underlying
 *    target for all operations).
 */
int
bcm_sirius_multicast_egress_set(int unit,
                                bcm_multicast_t group,
                                int port_count,
                                bcm_gport_t *port_array,
                                bcm_if_t *encap_id_array)
{
    _mc_unit_t *unitData;
    int result;

    MC_VERB((MC_MSG("(%d,%08X,%d,*,*) enter\n"), unit, group, port_count));

    /* get unit information and lock unit */
    MC_UNIT_CHECK(unit);
    MC_INIT_CHECK(unit, unitData);
    MC_LOCK_TAKE(unitData);

    /* set the group to the specified targets and OIs */
    result = _bcm_sirius_multicast_egress_set_override_oi(unitData,
                                                          group,
                                                          port_count,
                                                          port_array,
                                                          encap_id_array,
                                                          NULL,
                                                          !(unitData->flags &
                                                            _MC_UNIT_NO_OITT));

    /* unlock unit */
    MC_LOCK_GIVE(unitData);

    MC_VERB((MC_MSG("(%d,%08X,%d,*,*) return %d (%s)\n"),
             unit,
             group,
             port_count,
             result,
             _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_multicast_egress_subscriber_set
 *  Purpose
 *    Set a group to contain exactly the specified elements.
 *  Arguments
 *    (in) int unit = the unit
 *    (in) bcm_multicast_t group = group ID
 *    (in) int port_count = number of ports (why can this be negative?)
 *    (in) bcm_gport_t *port_array = pointer to array of gport IDs
 *    (in) bcm_if_t *encap_id_array = pointer to array of encap_ids
 *    (in) bcm_gport_t *queue_id_array = pointer to array of queue IDs
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    port_array and encap_id_array elements are paired 1:1.
 *
 *    Replaces entire existing group contents with new contents, effectively
 *    deleting anything not in the new set.  The API doc doesn't actually *say*
 *    this, but it should.
 *
 *    Somebody should explain why this doesn't take an *unsigned* count.  I
 *    don't see any information about what to do with negative count, so this
 *    implementation will call it BCM_E_PARAM.  Zero count is valid (it just
 *    means there are no replicants in the group).  Yes, you can provide NULL
 *    pointers to the arrays in the case of zero elements.  However, it's
 *    probably more efficient to just call the delete_all function instead.
 *
 *    Wants a BIG pile of stack space (_SHR_PBMP_PORT_MAX * (sizeof(bcm_port_t)
 *    + sizeof(_mc_oitt_elem_internal_t)) so it does not have to do heap
 *    thrashing for resizing data.
 *
 *    Performs insertion sort of encap_ids on each target while building the
 *    new mutlicast group, same for targets in the set.  This means that it
 *    will be a bit faster if the list is already sorted by the underlying
 *    target of the gport, and then by the encap_id within the target.  Note
 *    that gport per se is of no internal consequence (it uses the underlying
 *    target for all operations).
 */
int
bcm_sirius_multicast_egress_subscriber_set(int unit,
                                           bcm_multicast_t group,
                                           int port_count,
                                           bcm_gport_t *port_array,
                                           bcm_if_t *encap_id_array,
                                           bcm_gport_t *queue_id_array)
{
    _mc_unit_t *unitData;
    int result;

    MC_VERB((MC_MSG("(%d,%08X,%d,*,*,*) enter\n"), unit, group, port_count));

    /* get unit information and lock unit */
    MC_UNIT_CHECK(unit);
    MC_INIT_CHECK(unit, unitData);
    MC_LOCK_TAKE(unitData);

    /* set the group to the specified targets and OIs */
    result = _bcm_sirius_multicast_egress_set_override_oi(unitData,
                                                          group,
                                                          port_count,
                                                          port_array,
                                                          encap_id_array,
                                                          queue_id_array,
                                                          !(unitData->flags &
                                                            _MC_UNIT_NO_OITT));

    /* unlock unit */
    MC_LOCK_GIVE(unitData);

    MC_VERB((MC_MSG("(%d,%08X,%d,*,*,*) return %d (%s)\n"),
             unit,
             group,
             port_count,
             result,
             _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_multicast_egress_get
 *  Purpose
 *    Get a bunch of the replicants from a group.
 *  Arguments
 *    (in) int unit = the unit
 *    (in) bcm_multicast_t group = group ID
 *    (in) int port_max = maximum number of ports allowed in arrays
 *    (out) bcm_gport_t *port_array = where to put the gport array
 *    (out) bcm_if_t *encap_id_array = where to put the encap_id array
 *    (out) int *port_count = where to put the filled-in port count
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    port_array and encap_id_array elements are paired 1:1.
 *
 *    Somebody should explain why this doesn't take an *unsigned* maximum.  I
 *    don't see any information about what to do with negative maximum, so this
 *    implementation will call it BCM_E_PARAM.
 *
 *    Special case: if port_max is zero, and *port_array is NULL and
 *    encap_id_array is null, then the actual group size is returned and no
 *    other values are populated.  Since all of this is not possible from some
 *    of the test environments, we'll allow merely port_max zero to suffice.
 *
 *    A fairly annoying API design bug here provides no way to find out how
 *    large a group is besides being prepared for an absurdly large number of
 *    replicants (for example, on Sirius, the maximum theoretical replicant
 *    count for a *single group* is 540672, which would require 4.125MB in
 *    those arrays, just to be sure).  Practically (and for this implementation
 *    particularly), the limit is much smaller, as we can only have 65536
 *    replicants total, but that's still asking for a lot.  It would be nice to
 *    have a way to indicate the total replicant count in one call so the
 *    caller could provide an adequate buffer...
 *
 *    The list is not read from the hardware; it is merely the cached data.
 */
int
bcm_sirius_multicast_egress_get(int unit,
                                bcm_multicast_t group,
                                int port_max,
                                bcm_gport_t *port_array,
                                bcm_if_t *encap_id_array,
                                int *port_count)
{
    _mc_unit_t *unitData;
    unsigned int index;
    int result;

    MC_VERB((MC_MSG("(%d,%08X,%d,*,*,*) enter\n"), unit, group, port_max));

    /* some quick validation */
    if ((((!port_array) || (!encap_id_array)) && (0 < port_max)) ||
        (!port_count)) {
        MC_ERR((MC_MSG1("NULL pointer to required outbound argument\n")));
        return BCM_E_PARAM;
    }
    if (0 > port_max) {
        MC_ERR((MC_MSG1("I refuse to work with a negative count\n")));
        return BCM_E_PARAM;
    }

#if defined(BCM_EASY_RELOAD_SUPPORT)
    /* handle reload case */
    if (SOC_IS_RELOADING(unit)) {
        MC_WARN((MC_MSG1("unit %d state might be out of sync during reload\n"),
                 unit));
    } /* if (SOC_IS_RELOADING(unit)) */
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */

    /* get unit information and lock unit */
    MC_UNIT_CHECK(unit);
    MC_INIT_CHECK(unit, unitData);
    MC_LOCK_TAKE(unitData);

    if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
        MC_WARN((MC_MSG1("unit %d dual-lookup mode but get without queues\n"),
                 unit));
    }

    /* make sure the group exists */
    result = _bcm_sirius_multicast_group_check(unitData, group);

    /* put the group cache information into the caller's buffer */
    if (BCM_E_NONE == result) {
        if (port_max > 0) {
            for (index = 0;
                 (index < unitData->cache[group].elements) &&
                 (index < port_max);
                 index++) {
                port_array[index] = unitData->cache[group].data[index].port;
                encap_id_array[index] = unitData->cache[group].data[index].encap_id;
            }
            *port_count = index;
            if (port_max < unitData->cache[group].elements) {
                MC_WARN((MC_MSG("not enough room to read entire unit %d"
                                " group %08X: buffer is %d but needed %d\n"),
                         unit,
                         group,
                         port_max,
                         unitData->cache[group].elements));
            }
        } else { /* if (port_max > 0) */
            MC_EVERB((MC_MSG1("unit %d group %08X size = %d replicants\n"),
                      unit,
                      group,
                      unitData->cache[group].elements));
            *port_count = unitData->cache[group].elements;
        } /* if (port_max > 0) */
    } /* if (BCM_E_NONE == result) */

    /* unlock unit */
    MC_LOCK_GIVE(unitData);

    MC_VERB((MC_MSG("(%d,%08X,%d,*,*,&(%d)) return %d (%s)\n"),
             unit,
             group,
             port_max,
             *port_count,
             result,
             _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_multicast_egress_subscriber_get
 *  Purpose
 *    Get a bunch of the replicants from a group.
 *  Arguments
 *    (in) int unit = the unit
 *    (in) bcm_multicast_t group = group ID
 *    (in) int port_max = maximum number of ports allowed in arrays
 *    (out) bcm_gport_t *port_array = where to put the gport array
 *    (out) bcm_if_t *encap_id_array = where to put the encap_id array
 *    (out) bcm_gport_t *queue_id_array = where to put the queue_id array
 *    (out) int *port_count = where to put the filled-in port count
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    port_array and encap_id_array and queue_id_array elements are paired 1:1.
 *
 *    Somebody should explain why this doesn't take an *unsigned* maximum.  I
 *    don't see any information about what to do with negative maximum, so this
 *    implementation will call it BCM_E_PARAM.
 *
 *    Special case: if port_max is zero, and *port_array is NULL and
 *    encap_id_array is null, then the actual group size is returned and no
 *    other values are populated.  Since all of this is not possible from some
 *    of the test environments, we'll allow merely port_max zero to suffice.
 *
 *    A fairly annoying API design bug here provides no way to find out how
 *    large a group is besides being prepared for an absurdly large number of
 *    replicants (for example, on Sirius, the maximum theoretical replicant
 *    count for a *single group* is 540672, which would require 4.125MB in
 *    those arrays, just to be sure).  Practically (and for this implementation
 *    particularly), the limit is much smaller, as we can only have 65536
 *    replicants total, but that's still asking for a lot.  It would be nice to
 *    have a way to indicate the total replicant count in one call so the
 *    caller could provide an adequate buffer...
 *
 *    The list is not read from the hardware; it is merely the cached data.
 */
int
bcm_sirius_multicast_egress_subscriber_get(int unit,
                                           bcm_multicast_t group,
                                           int port_max,
                                           bcm_gport_t *port_array,
                                           bcm_if_t *encap_id_array,
                                           bcm_gport_t *queue_id_array,
                                           int *port_count)
{
    _mc_unit_t *unitData;
    unsigned int index;
    int result;

    MC_VERB((MC_MSG("(%d,%08X,%d,*,*,*,*) enter\n"), unit, group, port_max));

    /* some quick validation */
    if ((((!port_array) || (!encap_id_array) || (!queue_id_array)) &&
         (0 < port_max)) ||
        (!port_count)) {
        MC_ERR((MC_MSG1("NULL pointer to required outbound argument\n")));
        return BCM_E_PARAM;
    }
    if (0 > port_max) {
        MC_ERR((MC_MSG1("I refuse to work with a negative count\n")));
        return BCM_E_PARAM;
    }

#if defined(BCM_EASY_RELOAD_SUPPORT)
    /* handle reload case */
    if (SOC_IS_RELOADING(unit)) {
        MC_WARN((MC_MSG1("unit %d state might be out of sync during reload\n"),
                 unit));
    } /* if (SOC_IS_RELOADING(unit)) */
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */

    /* get unit information and lock unit */
    MC_UNIT_CHECK(unit);
    MC_INIT_CHECK(unit, unitData);
    MC_LOCK_TAKE(unitData);

    if (!(unitData->flags & _MC_UNIT_DUAL_LOOKUP)) {
        MC_WARN((MC_MSG1("unit %d single-lookup mode but get with queues\n"),
                 unit));
    }

    /* make sure the group exists */
    result = _bcm_sirius_multicast_group_check(unitData, group);

    /* put the group cache information into the caller's buffer */
    if (BCM_E_NONE == result) {
        if (port_max > 0) {
            for (index = 0;
                 (index < unitData->cache[group].elements) &&
                 (index < port_max);
                 index++) {
                port_array[index] = unitData->cache[group].data[index].port;
                encap_id_array[index] = unitData->cache[group].data[index].encap_id;
                queue_id_array[index] = unitData->cache[group].data[index].queue_id;
            }
            *port_count = index;
            if (port_max < unitData->cache[group].elements) {
                MC_WARN((MC_MSG("not enough room to read entire unit %d"
                                " group %08X: buffer is %d but needed %d\n"),
                         unit,
                         group,
                         port_max,
                         unitData->cache[group].elements));
            }
        } else { /* if (port_max > 0) */
            MC_EVERB((MC_MSG1("unit %d group %08X size = %d replicants\n"),
                      unit,
                      group,
                      unitData->cache[group].elements));
            *port_count = unitData->cache[group].elements;
        } /* if (port_max > 0) */
    } /* if (BCM_E_NONE == result) */

    /* unlock unit */
    MC_LOCK_GIVE(unitData);

    MC_VERB((MC_MSG("(%d,%08X,%d,*,*,*,&(%d)) return %d (%s)\n"),
             unit,
             group,
             port_max,
             *port_count,
             result,
             _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_multicast_source_knockout_get
 *  Purpose
 *    Get source knockout setting for a multicast group
 *  Arguments
 *    (in) int unit = the unit
 *    (in) bcm_multicast_t group = group ID location
 *    (out) int *source_knockout = where to put source knockout state
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    source_knockout is updated on success only and will be merely TRUE or
 *    FALSE.
 */
int
bcm_sirius_multicast_source_knockout_get(int unit,
                                         bcm_multicast_t group,
                                         int *source_knockout)
{
    _mc_unit_t *unitData;
    uint32 flags;
    int result;

    MC_VERB((MC_MSG("(%d,%08X,*) enter\n"), unit, group));

    /* get unit information and lock unit */
    MC_UNIT_CHECK(unit);
    MC_INIT_CHECK(unit, unitData);
    MC_LOCK_TAKE(unitData);

    result = _bcm_sirius_multicast_group_get(unitData, group, &flags);
    if (BCM_E_NONE == result) {
        if (flags & BCM_MULTICAST_DISABLE_SRC_KNOCKOUT) {
            *source_knockout = FALSE;
        } else {
            *source_knockout = TRUE;
        }
    }

    MC_LOCK_GIVE(unitData);

    MC_VERB((MC_MSG("(%d,%08X,&(%s)) return %d (%s)\n"),
             unit,
             group,
             (*source_knockout)?"TRUE":"FALSE",
             result,
             _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_multicast_source_knockout_set
 *  Purpose
 *    Set source knockout setting for a multicast group
 *  Arguments
 *    (in) int unit = the unit
 *    (in) bcm_multicast_t group = group ID location
 *    (in) int source_knockout = new source knockout state
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    source_knockout should be TRUE or FALSE.
 */
int
bcm_sirius_multicast_source_knockout_set(int unit,
                                         bcm_multicast_t group,
                                         int source_knockout)
{
    _mc_mvr_internal_t mvrData;
    _mc_unit_t *unitData;
    eg_fd_gmt_entry_t gmtData;
    uint32 mvrOld = ~0;
    uint32 mvrNew;
    int result;

    MC_VERB((MC_MSG("(%d,%08X,*) enter\n"), unit, group));

    /* get unit information */
    MC_UNIT_CHECK(unit);
    MC_INIT_CHECK(unit, unitData);

#if defined(BCM_EASY_RELOAD_SUPPORT)
    /* handle reload case */
    if (SOC_IS_RELOADING(unit)) {
        MC_ERR((MC_MSG1("may not change source knockout on unit %d during"
                        " reload\n"),
                unit));
        return BCM_E_CONFIG;
    } /* if (SOC_IS_RELOADING(unit)) */
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */

    /* lock unit */
    MC_LOCK_TAKE(unitData);

    /* make sure the group exists */
    result = _bcm_sirius_multicast_group_check(unitData, group);

    /* get the GMT entry */
    if (BCM_E_NONE == result) {
        result = soc_mem_read(unit,
                              EG_FD_GMTm,
                              MEM_BLOCK_ANY,
                              group,
                              &gmtData);
        if (BCM_E_NONE == result) {
            /* parse the entry */
            mvrOld = soc_EG_FD_GMTm_field32_get(unit,
                                                &(gmtData),
                                                MVRPf);
        } else { /* if (BCM_E_NONE == result) */
            MC_ERR((MC_MSG1("unable to read %d:gmt[%08X]: %d (%s)\n"),
                    unit,
                    group,
                    result,
                    _SHR_ERRMSG(result)));
        } /* if (BCM_E_NONE == result) */
    } /* if (BCM_E_NONE == result) */
    if (BCM_E_NONE == result) {
        /* get the MVR */
        result = _bcm_sirius_multicast_mvr_read(unitData, mvrOld, &mvrData);
        if (BCM_E_NONE == result) {
            /* edit the MVR */
            mvrData.sourceKnockout = !(!(source_knockout));
            /* write the MVR back */
            result = _bcm_sirius_multicast_mvr_write(unitData,
                                                     &mvrNew,
                                                     &mvrData);
            if (BCM_E_NONE == result) {
                /* update the GMT entry */
                soc_EG_FD_GMTm_field32_set(unit,
                                           &gmtData,
                                           MVRPf,
                                           mvrNew);
                /* write the GMT entry back */
                result = soc_mem_write(unit,
                                       EG_FD_GMTm,
                                       MEM_BLOCK_ALL,
                                       group,
                                       &gmtData);
                if (BCM_E_NONE == result) {
                    /* dispose of the old MVR */
                    shr_mdb_list_insert(unitData->mdbList,
                                        unitData->limbo,
                                        mvrOld);
                    unitData->mdbBlocks--;
                } else { /* if (BCM_E_NONE == result) */
                    MC_ERR((MC_MSG1("unable to write %d:gmt[%08X]: %d (%s)\n"),
                            unit,
                            group,
                            result,
                            _SHR_ERRMSG(result)));
                } /* if (BCM_E_NONE == result) */
            } /* if (BCM_E_NONE == result) */
        } /* if (BCM_E_NONE == result) */
    } /* if (BCM_E_NONE == result) */

    MC_LOCK_GIVE(unitData);

    MC_VERB((MC_MSG("(%d,%08X,&(%s)) return %d (%s)\n"),
             unit,
             group,
             source_knockout?"TRUE":"FALSE",
             result,
             _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    bcm_sirius_multicast_fabric_distribution_set
 *  Purpose
 *    To update the queue map table based upon the
 *  Arguments
 *    unit
 *    group - multicast group
 *    ds_id - distribution group ID
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 */

int
bcm_sirius_multicast_fabric_distribution_set(int unit,
                                             bcm_multicast_t            group,
                                             bcm_fabric_distribution_t  ds_id)
{
    int rv = BCM_E_NONE;
    int32 base_new;
    int32 base_old;
    bcm_sbx_cosq_queue_state_t *p_qstate = NULL;
    bcm_sbx_cosq_queue_state_t *pOldState = NULL;
    bcm_sbx_cosq_queue_state_t *pNewState = NULL;
    bcm_sbx_mgid_list_t *pRoot = NULL;
    bcm_sbx_mgid_list_t *pOld;
    bcm_sbx_mgid_list_t *pNew = NULL;
    dq_p_t pList;
    int index;
    bcm_multicast_t grpTemp;

    if (SOC_SBX_IF_PROTOCOL_SBX == SOC_SBX_CFG(unit)->uInterfaceProtocol) {
        /* do nothing for SBX mode */
        return BCM_E_NONE;
    }
    /* get information about the distribution_set */
    bcm_sbx_cosq_multicast_queue_group_get(unit, ds_id, &base_new);
    if (-1 == base_new) {
        MC_WARN((MC_MSG1("no queue group assigned with unit %d ds %d: this"
                         " will disable multicast group %d,%d,%d,%d ingress\n"),
                 unit,
                 ds_id,
                 group & 0x3FFF,
                 (group & 0x3FFF) | 0x4000,
                 (group & 0x3FFF) | 0x8000,
                 (group & 0x3FFF) | 0xC000));
    }

    /* get memory for root node just in case */
    pRoot = sal_alloc(sizeof(bcm_sbx_mgid_list_t), "Dq root");
    if (!pRoot) {
        MC_ERR((MC_MSG1("unable to allocate dq root\n")));
        rv = BCM_E_RESOURCE;
    }
    /* get memory for new node and fill it in */
    if (BCM_SUCCESS(rv)) {
        pNew = sal_alloc(sizeof(bcm_sbx_mgid_list_t), "Dq Elem");
        if (!pNew) {
            MC_ERR((MC_MSG1("unable to allocate dq element\n")));
            rv = BCM_E_RESOURCE;
        } else {
            DQ_INIT(&(pNew->node));
            pNew->data = group;
        }
    }
    /* get current queue base for this group */
    if (BCM_SUCCESS(rv)) {
        rv = soc_sirius_rb_higig2_header_multicast_queue_map_read(unit,
                                                                  group,
                                                                  &base_old);
        if (BCM_FAILURE(rv)) {
            MC_ERR((MC_MSG1("read multicast qsel unit %d group %d"
                            " failed: %d (%s)\n"),
                    unit,
                    group,
                    rv,
                    _SHR_ERRMSG(rv)));
        }
    }
    /* set new queue base for this group */
    if (BCM_SUCCESS(rv)) {
        if (-1 == base_new) {
            /* removing relationship by using unconfigured ds_id */
            rv = soc_sirius_rb_higig2_header_multicast_queue_map_config(unit,
                                                                        group,
                                                                        0xFFFF);
        } else {
            /* shifting/adding relationship to configured ds_id */
            rv = soc_sirius_rb_higig2_header_multicast_queue_map_config(unit,
                                                                        group,
                                                                        base_new);
        }
        if (BCM_FAILURE(rv)) {
            MC_ERR((MC_MSG1("write multicast qsel unit %d group %d"
                            " failed: %d (%s)\n"),
                    unit,
                    group,
                    rv,
                    _SHR_ERRMSG(rv)));
        }
    }
    /* get the state information for this queue set */
    if (BCM_SUCCESS(rv)) {
        p_qstate = (bcm_sbx_cosq_queue_state_t *)SOC_SBX_STATE(unit)->queue_state;
        pNewState = &(p_qstate[base_new]);
        pOldState = &(p_qstate[base_old]);
        if (pNewState->mgid_list) {
            /* already have root node */
            sal_free(pRoot);
            pRoot = NULL;
        } else {
            /* don't yet have root node, set up new root node */
            MC_EVERB((MC_MSG1("create root node for multicast groups in unit"
                              " %d queue group %d\n"),
                      unit,
                      base_new));
            DQ_INIT(&(pRoot->node));
            pRoot->data = 0;
            pNewState->mgid_list = pRoot;
            pRoot = NULL;
        }
    }
    /* Update any existing associations */
    if ((BCM_SUCCESS(rv)) && (0xFFFF != base_old)) {
        /*
         *  Find all aliases of this group that exist in the old queue set's
         *  list.  Remove those aliases, and, if the new queue set is valid,
         *  reinsert them into the new queue set's list.  If we actually move
         *  the specified group here, adjust some things so we do not add it to
         *  the new queue set's list later.
         */
        for (index = 0; index < 4; index++) {
            grpTemp = (group & 0x3FFF) | (index << 15);
            DQ_TRAVERSE(&(pOldState->mgid_list->node), pList)
                pOld = (bcm_sbx_mgid_list_t*)pList;
                if (pOld->data == grpTemp) {
                    /* found one of this group's aliases; remove it */
                    MC_EVERB((MC_MSG1("remove unit %d multicast group %d from"
                                      " queue group %d membership\n"),
                              unit,
                              grpTemp,
                              base_old));
                    DQ_REMOVE(pList);
                    pOldState->mgid_list->data--;
                    if (-1 != base_new) {
                        /* need to move it to the new list */
                        MC_EVERB((MC_MSG1("add unit %d multicast group %d to"
                                          " queue group %d membership\n"),
                                  unit,
                                  grpTemp,
                                  base_new));
                        DQ_INIT(pList);
                        DQ_INSERT_TAIL(&(pNewState->mgid_list->node), pList);
                        pNewState->mgid_list->data++;
                        if (grpTemp == group) {
                            /*
                             *  We found the specified group and moved it
                             *  already, so we must not insert it later.
                             */
                            sal_free(pNew);
                            pNew = NULL;
                        }
                    } else {
                        /* move to invalid queue set; discard association */
                        sal_free(pList);
                    }
                    break; /* found; skip to next alias now */
                }
            DQ_TRAVERSE_END(&(pOldState->mgid_list->node), pList);
        }
    }
    /* Add new assocaition if it was not moved already */
    if ((BCM_SUCCESS(rv)) && pNew && (-1 != base_new)) {
        MC_EVERB((MC_MSG1("add unit %d multicast group %d to queue group"
                          " %d membership (new)\n"),
                  unit,
                  group,
                  base_new));
        DQ_INSERT_TAIL(&(pNewState->mgid_list->node), pNew);
        pNewState->mgid_list->data++;
        pNew = NULL;
    }
    /* dispose of any leftover bits */
    if (pRoot) {
        sal_free(pRoot);
    }
    if (pNew) {
        sal_free(pNew);
    }
    return rv;
}

/*
 *  Function
 *    bcm_sirius_multicast_fabric_distribution_get
 *  Purpose
 *    To read the queue map table based upon the
 *  Arguments
 *    unit
 *    group - multicast group
 *    *ds_id - where to put the distribution group ID
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 */
int
bcm_sirius_multicast_fabric_distribution_get(int unit,
                                             bcm_multicast_t            group,
                                             bcm_fabric_distribution_t *ds_id)
{
    int rv = BCM_E_NONE;
    int32 base_queue;
    bcm_fabric_distribution_t dsid;

    /* get the base queue number */
    rv = soc_sirius_rb_higig2_header_multicast_queue_map_read(unit, group, &base_queue);
    if (rv) {
        BCM_ERR(("Unable to read from RB queue map table failed for multicast"
                 " group(%d): %d (%s)\n",
                 group,
                 rv,
                 _SHR_ERRMSG(rv)));
        return rv;
    }

    /* search for that base queue in the distribution base queue information */
    rv = bcm_sbx_cosq_multicast_ds_id_from_queue_group(unit, base_queue, &dsid);
    if (BCM_E_NONE == rv) {
        *ds_id = dsid;
    } else {
        BCM_ERR(("ERROR: unable to fetch DS_ID from queue group for multicast"
                 " group (%d) base queue (%d): %d (%s)\n",
                 group,
                 base_queue,
                 rv,
                 _SHR_ERRMSG(rv)));
    }

    return rv;
}

/*
 *  Function
 *    bcm_sirius_multicast_state_get
 *  Purpose
 *    ???
 *  Arguments
 *    (in) int unit = the unit
 *    (??) char* pbuf = ???
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Not intending to implement this: it's a crash asking to happen if it is
 *    meant to work the way I imagine it to be intended.
 */
int
bcm_sirius_multicast_state_get(int unit, char *pbuf)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}


/*****************************************************************************
 *
 *  Interface -- special API
 */


/*
 *  Function
 *    _bcm_sirius_multicast_dump
 *  Purpose
 *    Dump the multicast state to the debugging console
 *  Arguments
 *    (in) int unit = the unit
 *    (in) unsigned int level = debugging flags
 *    (in) unsigned int first = first group to display/probe
 *    (in) unsigned int last = last group to display/probe
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Debugging flags is a bitmap...
 *      b00 = include unit level data
 *      b01 = include GMT allocator information
 *      b02 = include MDB allocator information
 *      b03 = include OITT allocator information
 *      b08 = include group information...
 *      b09 =  ...even for groups that do not exist
 *      b0A =  ...by probing all groups in hardware
 *      b0B =  ...with full software details
 *      b0C =  ...with full hardware details
 *      other bits reserved (and do nothing now, but should be left zero)
 */
int
_bcm_sirius_multicast_dump(int unit,
                           unsigned int level,
                           unsigned int first,
                           unsigned int last)
{
    _mc_unit_t *unitData;
    _mc_mvr_internal_t *mvrPtr;
    int result, state;
    bcm_multicast_t group;
    unsigned int rrIndex, oittIndex, min, max;
    shr_idxres_element_t ifirst;
    shr_idxres_element_t ilast;
    shr_idxres_element_t ivalid_low;
    shr_idxres_element_t ivalid_high;
    shr_idxres_element_t ifree_count;
    shr_idxres_element_t ialloc_count;
    shr_mdb_info_t mdbInfo;
    shr_mdb_list_info_t mdbListInfo;
    shr_mdb_elem_bank_index_t list, lists;
    shr_mdb_alloc_pref_t allocmode;
    unsigned int qsIndex;
    unsigned int mdbLimboElements;
    unsigned int mdbLimboBlocks;
    int free;
    int displayedTitle;

    MC_VERB((MC_MSG("(%d,%08X) enter\n"), unit, level));

    /* get unit information and lock unit */
    MC_UNIT_CHECK(unit);
    MC_INIT_CHECK(unit, unitData);
    MC_LOCK_TAKE(unitData);

    /* be optimistic */
    result = BCM_E_NONE;

    soc_cm_print("unit %d multicast data (%08X)\n", unit, (uint32)unitData);
    if (level & 0x00000001) {
        /* showing unit level data */
        soc_cm_print("  active groups           = %8d of %8d\n", unitData->groupCount, unitData->groupCountMax);
        soc_cm_print("  dense MVR (MDB elems)   = %8d\n", unitData->denseMvrSize);
        soc_cm_print("  maximum target number   = %8d\n", unitData->maxTargets - 1);
        soc_cm_print("  current limbo list      = %8d\n", unitData->limbo);
        soc_cm_print("  current purge list      = %8d\n", unitData->purge);
        soc_cm_print("  unit status flags       = %08X\n", unitData->flags);
        soc_cm_print("  background skip global  = %8d\n", unitData->bgSkipGlob);
        soc_cm_print("  background skip unit    = %8d\n", unitData->bgSkipUnit);
        soc_cm_print("  pd alloc count          = %8d\n", unitData->cells);
        if (unitData->flags & _MC_UNIT_DEFRAG_RUNNING) {
            soc_cm_print("  defragmenter position   = %08X (%s)\n",
                         unitData->defragPosition,
                         (unitData->flags & _MC_UNIT_DEFRAG_UP)?"Up":"Down");
        } else {
            soc_cm_print("  defragmenter position   =   (idle)\n");
        }
        soc_cm_print("  defrag by heuristics    = %8d requests; %8d runs\n", unitData->defragAttHeur, unitData->defragHeur);
        soc_cm_print("  defrag by res shortage  = %8d requests; %8d runs\n", unitData->defragAttRes, unitData->defragRes);
        soc_cm_print("  defrag by user request  = %8d requests; %8d runs\n", unitData->defragAttUser, unitData->defragUser);
    } /* if (level & 0x00000001) */
    if (level & 0x00000003) {
        /* showing unit level data or GMT data */
        soc_cm_print("  GMT allocator           = %08X (idxres)\n",
                     (uint32)(unitData->gmtList));
    } /* if (level & 0x00000003) */
    if (level & 0x00000002) {
        /* showing GMT data */
        result = shr_idxres_list_state(unitData->gmtList,
                                       &ifirst,
                                       &ilast,
                                       &ivalid_low,
                                       &ivalid_high,
                                       &ifree_count,
                                       &ialloc_count);
        if (BCM_E_NONE == result) {
            soc_cm_print("    first element         = %8d\n", ifirst);
            soc_cm_print("    last element          = %8d\n", ilast);
            soc_cm_print("    low valid element     = %8d\n", ivalid_low);
            soc_cm_print("    high valid element    = %8d\n", ivalid_high);
            soc_cm_print("    used elements         = %8d\n", ialloc_count);
            soc_cm_print("    free elements         = %8d\n", ifree_count);
        } else {
            soc_cm_print("    unable to fetch GMT allocator information:"
                         " %d (%s)\n",
                         result,
                         _SHR_ERRMSG(result));
        }
    } /* if (level & 0x00000002) */
    if (level & 0x00000005) {
        /* showing unit level data or MDB data */
        soc_cm_print("  MDB allocator           = %08X (mdb)\n",
                     (uint32)(unitData->mdbList));
    }
    if (level & 0x00000004) {
        /* showing MDB data */
        result = shr_mdb_info(unitData->mdbList, &mdbInfo);
        if (BCM_E_NONE == result) {
            result = shr_mdb_allocmode_get(unitData->mdbList, &allocmode);
            soc_cm_print("    first element         = %8d\n", mdbInfo.first);
            soc_cm_print("    last element          = %8d\n", mdbInfo.last);
            soc_cm_print("    bank size             = %8d\n",
                         mdbInfo.bank_size);
            soc_cm_print("    free lists            = %8d\n",
                         mdbInfo.free_lists);
            soc_cm_print("    user lists            = %8d\n",
                         mdbInfo.user_lists);
            if (BCM_E_NONE == result) {
                soc_cm_print("    allocation mode       = %08X\n", allocmode);
            }
            mdbLimboElements=0;
            mdbLimboBlocks=0;
            for (list = 0; list < _MC_LIMBO_LISTS; list++) {
                result = shr_mdb_list_info(unitData->mdbList,
                                           list,
                                           FALSE,
                                           &mdbListInfo);
                if (BCM_E_NONE == result) {
                    mdbLimboElements += mdbListInfo.elements;
                    mdbLimboBlocks += mdbListInfo.blocks;
                }
            }
            soc_cm_print("    used elements         = %8d; blocks = %8d\n",
                         ((mdbInfo.last - mdbInfo.first + 1) -
                          mdbInfo.free_elems) - mdbLimboElements,
                         unitData->mdbBlocks);
            soc_cm_print("    limbo elements        = %8d; blocks = %8d\n",
                         mdbLimboElements,
                         mdbLimboBlocks);
            soc_cm_print("    free elements         = %8d; blocks = %8d\n",
                         mdbInfo.free_elems,
                         mdbInfo.free_blocks);
            soc_cm_print("    peak limbo elements   = %8d; blocks = %8d\n",
                         unitData->limboMdbMax,
                         unitData->limboMdbMaxBlk);
            free = 1;
            do { /* while (TRUE) */
                /* for free and user list sets */
                lists = free?mdbInfo.free_lists:mdbInfo.user_lists;
                for (list = 0; list < lists; list++) {
                    /* for each list in the set */
                    result = shr_mdb_list_info(unitData->mdbList,
                                               list,
                                               free,
                                               &mdbListInfo);
                    if (BCM_E_NONE == result) {
                        if (free) {
                            /* free lists have fixed block size */
                            soc_cm_print("    free list %2d elements = %8d;"
                                         " blocks = %8d; elem/blk = %5d\n",
                                         list,
                                         mdbListInfo.elements,
                                         mdbListInfo.blocks,
                                         mdbListInfo.block_size);
                        } else {
                            /* user lists do not have fixed block size */
                            soc_cm_print("    user list %2d elements = %8d;"
                                         " blocks = %8d\n",
                                         list,
                                         mdbListInfo.elements,
                                         mdbListInfo.blocks);
                        }
                    } else { /* if (BCM_E_NONE == result) */
                        soc_cm_print("    %s list %2d access error %d (%s)\n",
                                     free?"free":"user",
                                     list,
                                     result,
                                     _SHR_ERRMSG(result));
                        break;
                    } /* if (BCM_E_NONE == result) */
                } /* for (list = 0; list < lists; list++) */
                if ((!free) || (BCM_E_NONE != result)) {
                    break;
                }
                free = 0;
            } while (TRUE);
        } else { /* if (BCM_E_NONE == result) */
            soc_cm_print("    unable to fetch MDB allocator information:"
                         " %d (%s)\n",
                         result,
                         _SHR_ERRMSG(result));
        } /* if (BCM_E_NONE == result) */
    } /* if (level & 0x00000004) */
    if (level & 0x00000009) {
        /* showing unit level data or OITT data */
        soc_cm_print("  OITT allocator          = %08X (mdb)\n",
                     (uint32)(unitData->oittList));
    }
    if (level & 0x00000008) {
        /* showing OITT data */
        result = shr_mdb_info(unitData->oittList, &mdbInfo);
        if (BCM_E_NONE == result) {
            result = shr_mdb_allocmode_get(unitData->mdbList, &allocmode);
            soc_cm_print("    first element         = %8d\n", mdbInfo.first);
            soc_cm_print("    last element          = %8d\n", mdbInfo.last);
            soc_cm_print("    bank size             = %8d\n",
                         mdbInfo.bank_size);
            soc_cm_print("    free lists            = %8d\n",
                         mdbInfo.free_lists);
            soc_cm_print("    user lists            = %8d\n",
                         mdbInfo.user_lists);
            if (BCM_E_NONE == result) {
                soc_cm_print("    allocation mode       = %08X\n", allocmode);
            }
            mdbLimboElements=0;
            mdbLimboBlocks=0;
            for (list = 0; list < _MC_LIMBO_LISTS; list++) {
                result = shr_mdb_list_info(unitData->oittList,
                                           list,
                                           FALSE,
                                           &mdbListInfo);
                if (BCM_E_NONE == result) {
                    mdbLimboElements += mdbListInfo.elements;
                    mdbLimboBlocks += mdbListInfo.blocks;
                }
            }
            soc_cm_print("    used elements         = %8d; blocks = %8d\n",
                         ((mdbInfo.last - mdbInfo.first + 1) -
                          mdbInfo.free_elems) - mdbLimboElements,
                         unitData->oittBlocks);
            soc_cm_print("    limbo elements        = %8d; blocks = %8d\n",
                         mdbLimboElements,
                         mdbLimboBlocks);
            soc_cm_print("    free elements         = %8d; blocks = %8d\n",
                         mdbInfo.free_elems,
                         mdbInfo.free_blocks);
            soc_cm_print("    peak limbo elements   = %8d; blocks = %8d\n",
                         unitData->limboOittMax,
                         unitData->limboOittMaxBlk);
            free = 1;
            do { /* while (TRUE) */
                /* for free and user list sets */
                lists = free?mdbInfo.free_lists:mdbInfo.user_lists;
                for (list = 0; list < lists; list++) {
                    /* for each list in the set */
                    result = shr_mdb_list_info(unitData->oittList,
                                               list,
                                               free,
                                               &mdbListInfo);
                    if (BCM_E_NONE == result) {
                        if (free) {
                            /* free lists have fixed block size */
                            soc_cm_print("    free list %2d elements = %8d;"
                                         " blocks = %8d; elem/blk = %5d\n",
                                         list,
                                         mdbListInfo.elements,
                                         mdbListInfo.blocks,
                                         mdbListInfo.block_size);
                        } else {
                            /* user lists do not have fixed block size */
                            soc_cm_print("    user list %2d elements = %8d;"
                                         " blocks = %8d\n",
                                         list,
                                         mdbListInfo.elements,
                                         mdbListInfo.blocks);
                        }
                    } else { /* if (BCM_E_NONE == result) */
                        soc_cm_print("    %s list %2d access error %d (%s)\n",
                                     free?"free":"user",
                                     list,
                                     result,
                                     _SHR_ERRMSG(result));
                        break;
                    } /* if (BCM_E_NONE == result) */
                } /* for (list = 0; list < lists; list++) */
                if ((!free) || (BCM_E_NONE != result)) {
                    break;
                }
                free = 0;
            } while (TRUE);
        } else { /* if (BCM_E_NONE == result) */
            soc_cm_print("    unable to fetch MDB allocator information:"
                         " %d (%s)\n",
                         result,
                         _SHR_ERRMSG(result));
        } /* if (BCM_E_NONE == result) */
    } /* if (level & 0x00000008) */

    if (level & 0x00000100) {
        /* show active groups */
        min = SOC_MEM_INFO(unit, EG_FD_GMTm).index_min;
        if (min < first) {
            min = first;
        }
        max = SOC_MEM_INFO(unit, EG_FD_GMTm).index_max;
        if (max > last) {
            max = last;
        }
        for (group = min; group <= max; group++) {
            /* for each group */
            if (group <= unitData->gmtGroupMax) {
                /* in multicast managed space */
                state = shr_idxres_list_elem_state(unitData->gmtList, group);
            } else {
                /* in unicast managed space */
                if (SHR_BITGET(unitData->unicast,
                               (group - (unitData->gmtGroupMax + 1)))) {
                    state = BCM_E_EXISTS;
                } else {
                    state = BCM_E_NOT_FOUND;
                }
            }
            if ((BCM_E_EXISTS == state) || (level & 0x00000200)) {
                if (group > unitData->gmtGroupMax) {
                    soc_cm_print("  Group %08X (unicast sysport %d)\n",
                                 group,
                                 group & 0xFFF);
                } else {
                    soc_cm_print("  Group %08X (multicast)\n", group);
                }
                displayedTitle = TRUE;
            } else {
                displayedTitle = FALSE;
            }
            if (BCM_E_EXISTS == state) {
                if (unitData->cache[group].elements &&
                    (!(unitData->cache[group].data))) {
                    soc_cm_print("    Software layer information: missing\n");
                } else { /* if (cache shows elements but no data) */
                    /* count logical replicants */
                    for (qsIndex = 0, oittIndex = 0;
                         qsIndex < unitData->cache[group].elements;
                         qsIndex++) {
                        if ((unitData->cache[group].data[qsIndex].extras >> _MC_CACHE_EXTRA_HGA_SHIFT) & _MC_CACHE_EXTRA_HGA_MASK) {
                            /*
                             *  We assume these are higig aggregates, so only one
                             *  replicant should ever make it to a single downstream
                             *  unit for local replication.
                             */
                            oittIndex++;
                        } else {
                            /*
                             *  The count for everything else should be accurate.
                             */
                            oittIndex += ((unitData->cache[group].data[qsIndex].extras >> _MC_CACHE_EXTRA_COUNT_SHIFT) & _MC_CACHE_EXTRA_COUNT_MASK);
                        }
                    } /* for all elements in the group's cache */
                    /* dump the group software state */
                    soc_cm_print("    Software layer information: valid\n");
                    soc_cm_print("      GP,EI%s count = %d (%d logical replicants)\n",
                                 (unitData->flags & _MC_UNIT_DUAL_LOOKUP)?",Q":"",
                                 unitData->cache[group].elements,
                                 oittIndex);
                    if (level & 0x00000800) {
                        /* including group details -- logical */
                        if (unitData->cache[group].data) {
                            for (qsIndex = 0;
                                 qsIndex < unitData->cache[group].elements;
                                 qsIndex++) {
                                if (unitData->flags & _MC_UNIT_DUAL_LOOKUP) {
                                    soc_cm_print("      GP,EI,Q %5d = %08X,%08X,%08X "
                                                 _MC_TBMP_FORMAT
                                                 " "
                                                 _MC_CACHE_EXTRA_FORMAT
                                                 "\n",
                                                 qsIndex,
                                                 unitData->cache[group].data[qsIndex].port,
                                                 unitData->cache[group].data[qsIndex].encap_id,
                                                 unitData->cache[group].data[qsIndex].queue_id,
                                                 _MC_TBMP_DISPLAY(unitData->cache[group].data[qsIndex].tbmp),
                                                 unitData->cache[group].data[qsIndex].extras);
                                } else {
                                    soc_cm_print("      GP,EI %5d = %08X,%08X "
                                                 _MC_TBMP_FORMAT
                                                 " "
                                                 _MC_CACHE_EXTRA_FORMAT
                                                 "\n",
                                                 qsIndex,
                                                 unitData->cache[group].data[qsIndex].port,
                                                 unitData->cache[group].data[qsIndex].encap_id,
                                                 _MC_TBMP_DISPLAY(unitData->cache[group].data[qsIndex].tbmp),
                                                 unitData->cache[group].data[qsIndex].extras);
                                }
                            }
                        } /* if (unitData->cache[group].data) */
                    } /* if (level & 0x00000800) */
                } /* if (cache shows elements but no data) */
            } else { /* if (BCM_E_EXISTS == state) */
                if (level & 0x00000200) {
                    /* showing all including non-existent */
                    soc_cm_print("    Software layer information: no group\n");
                }
                if (!(level & 0x00000400)) {
                    /* but don't bother probing if software says no group */
                    continue;
                }
            } /* if (BCM_E_EXISTS == state) */
            /* try to read group from hardware */
            result = _bcm_sirius_multicast_read(unitData, group, &mvrPtr);
            if (!displayedTitle &&
                ((BCM_E_NOT_FOUND != result) || (level & 0x00000200))) {
                if (group > unitData->gmtGroupMax) {
                    soc_cm_print("  Group %08X (unicast sysport %d)\n",
                                 group,
                                 group & 0xFFF);
                } else {
                    soc_cm_print("  Group %08X (multicast)\n", group);
                }
                if (BCM_E_NOT_FOUND == state) {
                    soc_cm_print("    Software layer information: no group\n");
                }
            }
            if (BCM_E_NONE == result) {
                /* figure out how many replicants */
                if (mvrPtr->logical) {
                    /* logical mode, so have to count replicants per target */
                    oittIndex = 0;
                    for (rrIndex = 0; rrIndex < mvrPtr->targets; rrIndex++) {
                        oittIndex += mvrPtr->ptr.rr[rrIndex].replicants;
                    }
                } else {
                    /* physical mode, so one replicant per target */
                    oittIndex = mvrPtr->targets;
                }
                soc_cm_print("    Hardware layer information: valid\n");
                soc_cm_print("      Group type = %8s\n",
                             mvrPtr->logical?"logical":"physical");
                soc_cm_print("      Knockout   = %8s\n",
                             mvrPtr->sourceKnockout?"on":"off");
                soc_cm_print("      Targets    = %8d\n", mvrPtr->targets);
                soc_cm_print("      Replicants = %8d\n", oittIndex);
                if (level & 0x00001000) {
                    /* including group details -- physical */
                    _bcm_sirius_multicast_hwinfo_dump(unitData,
                                                      "      ",
                                                      mvrPtr);
                }
                /* now get rid of this group's descriptor */
                _bcm_sirius_multicast_mvrdata_free(mvrPtr);
                mvrPtr = NULL; 
            } else if (BCM_E_NOT_FOUND == result) {
                /* failed to read group from hardware */
                if (level & 0x00000200) {
                    /* showing all including non-existent */
                    soc_cm_print("    Hardware layer information: no group\n");
                }
            } else {
                /* display error */
                soc_cm_print("    Hardware layer information: error %d (%s)\n",
                             result,
                             _SHR_ERRMSG(result));
            }
            /* don't return gratuitous errors */
            result = BCM_E_NONE;
        } /* for (group = min; group <= max; group++) */
    } /* if (level & 0x00000100) */

    MC_LOCK_GIVE(unitData);

    MC_VERB((MC_MSG("(%d,%08X) return %d (%s)\n"),
             unit,
             level,
             result,
             _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *    _bcm_sirius_multicast_defrag
 *  Purpose
 *    Coerce the defrag state (start or stop defragmentation)
 *  Arguments
 *    (in) int unit = the unit
 *    (in) int newstate = TRUE to start, FALSE to stop
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    If already running, start has no effect.
 *    If not already running, stop has no effect.
 */
int
_bcm_sirius_multicast_defrag(int unit, int newstate)
{
    _mc_unit_t *unitData;

    MC_VERB((MC_MSG("(%d,%s) enter\n"), unit, newstate?"START":"STOP"));

    /* get unit information */
    MC_UNIT_CHECK(unit);
    MC_INIT_CHECK(unit, unitData);

#if defined(BCM_EASY_RELOAD_SUPPORT)
    /* handle reload case */
    if (SOC_IS_RELOADING(unit)) {
        MC_ERR((MC_MSG1("may not invoke defrag on unit %d during reload\n"),
                unit));
        return BCM_E_CONFIG;
    } /* if (SOC_IS_RELOADING(unit)) */
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */

    /* lock unit */
    MC_LOCK_TAKE(unitData);

    /* set begin or abort defragmentation flags */
    if (newstate) {
        /* start defragmentation */
        unitData->flags = (unitData->flags &
                           (~_MC_UNIT_DEFRAG_BY_MASK)) |
                          (_MC_UNIT_DEFRAG_START |
                           _MC_UNIT_DEFRAG_BY_USER);
        unitData->defragAttUser++;
    } else {
        /* stop defragmentation */
        unitData->flags |= _MC_UNIT_DEFRAG_HALT;
    }

    MC_LOCK_GIVE(unitData);

    MC_VERB((MC_MSG("(%d,%s) return %d (%s)\n"),
             unit,
             newstate?"START":"STOP",
             BCM_E_NONE,
             _SHR_ERRMSG(BCM_E_NONE)));
    return BCM_E_NONE;
}

/*
 *  Function
 *    _bcm_sirius_multicast_oitt_enable_set
 *  Purpose
 *    Set whether OI translation is space is managed and used.
 *  Arguments
 *    (in) int unit = the unit
 *    (in) int newstate = TRUE to enable OI translation
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    This does not affect the actual OI translation operation on frames, which
 *    is controlled by the egress processor predicates and frame rewrite rules.
 *
 *    This should only be called when there are no groups; changing this
 *    setting when groups are present and then updating the groups could cause
 *    inconsistent state within the updated groups.  Correct operation is not
 *    guaranteed should this occur.
 *
 *    Disabling OITT (and also removing OI translation from the egress
 *    processing) should be done on platforms where the downstream device
 *    performs egress replication and frame changes for its interfaces; it will
 *    avoid allocating (and manipulating) OITT entries, effectively moving the
 *    64K per unit replicant limit to something rather stratospheric, but OIs
 *    and GPORTs will no longer be tracked for the replicants (so when reading
 *    a replicant with 'get', the GPORT will be the child port designation for
 *    the target, and the OI will be the replicant's index).
 */
int
_bcm_sirius_multicast_oitt_enable_set(int unit, int newstate)
{
    _mc_unit_t *unitData;

    MC_VERB((MC_MSG("(%d,%s) enter\n"), unit, newstate?"ENABLE":"DISABLE"));

    /* get unit information and lock unit */
    MC_UNIT_CHECK(unit);
    MC_INIT_CHECK(unit, unitData);
    MC_LOCK_TAKE(unitData);

    /* set or clear OITT usage disable */
    if (newstate) {
        /* enable OITT usage */
        unitData->flags &= (~_MC_UNIT_NO_OITT);
    } else {
        /* disable OITT usage */
        unitData->flags |= _MC_UNIT_NO_OITT;
    }

    MC_LOCK_GIVE(unitData);

    MC_VERB((MC_MSG("(%d,%s) return %d (%s)\n"),
             unit,
             newstate?"ENABLE":"DISABLE",
             BCM_E_NONE,
             _SHR_ERRMSG(BCM_E_NONE)));
    return BCM_E_NONE;
}

/*
 *  Function
 *    _bcm_sirius_multicast_oitt_enable_get
 *  Purpose
 *    Get whether OI translation space is managed and used.
 *  Arguments
 *    (in) int unit = the unit
 *    (out) int *newstate = where to put current OITT state
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 */
int
_bcm_sirius_multicast_oitt_enable_get(int unit, int *newstate)
{
    _mc_unit_t *unitData;

    MC_VERB((MC_MSG("(%d,*) enter\n"), unit));

    if (!newstate) {
        MC_ERR((MC_MSG1("NULL pointer unacceptable for outbound argument\n")));
        return BCM_E_PARAM;
    }

    /* get unit information and lock unit */
    MC_UNIT_CHECK(unit);
    MC_INIT_CHECK(unit, unitData);
    MC_LOCK_TAKE(unitData);

    *newstate = !(unitData->flags & _MC_UNIT_NO_OITT);

    MC_LOCK_GIVE(unitData);

    MC_VERB((MC_MSG("(%d,%s) return %d (%s)\n"),
             unit,
             (*newstate)?"ENABLE":"DISABLE",
             BCM_E_NONE,
             _SHR_ERRMSG(BCM_E_NONE)));
    return BCM_E_NONE;
}
