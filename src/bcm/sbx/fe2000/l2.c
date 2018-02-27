/* 
 * $Id: l2.c 1.79.42.1 Broadcom SDK $
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
 * File:        l2.c
 * Purpose:     BCM Layer-2 switch API
 */

#include <sal/core/sync.h>

#include <soc/macipadr.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/fe2000.h>

#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#endif /* BCM_FE2000_P3_SUPPORT */

#include <shared/gport.h>

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/debug.h>
#include <bcm/l2.h>
#include <bcm/stack.h>
#include <bcm/mcast.h>
#include <bcm/vlan.h>
#include <bcm/tunnel.h>
#include <bcm/ipmc.h>
#include <bcm/mpls.h>
#include <bcm/field.h>

#include <shared/idxres_fl.h>
#include <shared/hash_tbl.h>

#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/fe2000/allocator.h>
#include <bcm_int/sbx/l2.h>
#include <bcm_int/sbx/mcast.h>
#include <bcm_int/sbx/stat.h>
#include <bcm_int/sbx/state.h>

#ifdef BCM_FE2000_P3_SUPPORT
#include <bcm_int/sbx/fe2000/g2p3.h>
#endif

#define G2P3_CHECK_AND_WARN(u) \
if (SOC_IS_SBX_G2P3((u))) {        \
   BCM_DEBUG(BCM_DBG_WARN, ("WARNING %s has not been ported for G2P3 (%s:%d)\n", \
               FUNCTION_NAME(), __FILE__, __LINE__));  \

#define NO_G2P3_SUPPORT_CHECK(u)     \
   G2P3_CHECK_AND_WARN(u) \
   return BCM_E_UNAVAIL; \
}

/*
 * Mutex Lock
 */
sal_mutex_t      _l2_mlock[BCM_LOCAL_UNITS_MAX];

/*
 * L2 Age
 *
 * The Age Timer thread is started when aging is enabled.
 *
 * Age-Interval / Age-Cycles
 *   The age time provided by the user indicates the duration of
 *   an age-interval.
 *
 *   Each age-interval can have one or more age-cycles.  This allows
 *   for the microcode aging process to traverse in each age-cycle
 *   blocks of entries, rather than the complete table.  The entire table
 *   should be processed at the end of each age-interval.
 *
 *   The number of entries processed in each age-cycle run is calculated
 *   from the user provided number of cycles for each age-interval as
 *   follows:
 *       entries_per_cycle = MacTableSize / cycles_per_interval
 *
 *   The number of cycles for an age-interval can be specified with
 *   the SOC property "l2_age_cycles".  The default value is '1',
 *   which results in processing ALL the MAC entries in one aging run
 *   (one age-cycle).
 *   
 *   The ager-age (or age timestamp) is incremented in each age-interval.
 *
 *   An entry is aged and removed after one age-interval, up to
 *   just beginning of the second interval, has ocurred since the
 *   last update to the entry.
 */
typedef struct _l2_age_s {
    char                thread_name[16];  /* L2 age timer thread name */
    VOL sal_thread_t    thread_id;        /* L2 age timer thread id */
    sal_sem_t           sem;              /* Semaphore to signal thread */
    uint32              ager;             /* Current ager age */
    int                 num_old_mac;      /* Number of old mac entries */
    int                 table_max;        /* L2 max table size */
    int                 cycles_interval;  /* Cycles per age interval */
    int                 entries_cycle;    /* Entries to process in one cycle */
    VOL int             age_sec;          /* User input age time */
    VOL uint64          cycle_usec;       /* Cycle time */
    int                 sleep_count;      /* For larger timer sleep */
} _l2_age_t;

static _l2_age_t      _l2_age[BCM_LOCAL_UNITS_MAX];
static uint           _l2_sem_usec_max;   /* Max value for sal_sem_take() */

#define L2_AGE_TIMER_THREAD_PRIO        50
#define L2_AGE_TIMER_STOP_WAIT_USEC     1000      /* 1 msec */
#define L2_AGE_TIMER_STOP_RETRIES_MAX   100

#define L2_AGE_TIMER_WAKE(unit)                                        \
    do {                                                               \
        _l2_age[unit].sleep_count = 0;                                 \
        sal_sem_give(_l2_age[unit].sem);                               \
    } while(0)

#define L2_AGE_TIMER_SLEEP(unit, usec)                                 \
    do {                                                               \
        uint32  sleep_time;                                            \
        _l2_age[unit].sleep_count = ((usec) / _l2_sem_usec_max) + 1;   \
        sleep_time = (usec) / _l2_age[unit].sleep_count;               \
        while ((_l2_age[unit].sleep_count-- > 0) &&                    \
               (_l2_age[unit].age_sec != 0)) {                         \
            sal_sem_take(_l2_age[unit].sem, sleep_time);               \
        }                                                              \
    } while(0)


/* Age delta needs to be 1 step more to account for current age elapsed time */
#define L2_AGE_DELTA                    2
                                                 
#define L2_AGE_CYCLE_MIN_USEC           100       /* Minimum between cycles */

#define L2_AGE_INCR(age)    \
    ((age) = ((age) == SB_ZF_G2_FE_SMACPYLD_AGE_MAX) ? 0 : ((age)+1))

#define L2_AGE_TIMER_MAX_SECS           1000000   /* Max age timer value */

#define L2_INVALID_EGRESS_ID         ~0

#define L2_ID_RESERVED(u, id) \
   ( (_l2_egress[u].idList.reservedLow != L2_INVALID_EGRESS_ID) &&   \
     (_l2_egress[u].idList.reservedHigh != L2_INVALID_EGRESS_ID) &&  \
     ((id) >= _l2_egress[u].idList.reservedLow ) &&                  \
     ((id) <= _l2_egress[u].idList.reservedHigh) )


typedef struct _l2_egress_id_list_s {
    shr_idxres_list_handle_t    idMgr;
    uint32                    reservedHigh;
    uint32                    reservedLow;
} _l2_egress_id_list_t;

/*
 * L2 Egress
 */
typedef struct _l2_egress_s {
    uint8                       init_done;
    _l2_egress_id_list_t        idList;
    int                         encap_id_max;
    uint32                      *dest_ports;
#ifdef BCM_WARM_BOOT_SUPPORT
    soc_wb_cache_t              *wb_cache;
    uint32                      scache_size;
#endif
} _l2_egress_t;

static _l2_egress_t   _l2_egress[BCM_LOCAL_UNITS_MAX];

#ifdef BCM_WARM_BOOT_SUPPORT
#define L2_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define L2_WB_CURRENT_VERSION            L2_WB_VERSION_1_0
/* Is Level 2 warmboot configured - */
#define L2_WB_L2_CONFIGURED(unit) (_l2_egress[unit].wb_cache != NULL)

#define L2_WB_L2_HANDLE_SET(_u, _h) \
   SOC_SCACHE_HANDLE_SET(_h, _u, BCM_MODULE_L2, L2_WB_L2);
#define L2_WB_LOCK_CACHE(_u)    \
   {   uint32  __handle__;                      \
       L2_WB_L2_HANDLE_SET(_u, __handle__);     \
       soc_scache_handle_lock(_u, __handle__);  \
   }
#define L2_WB_UNLOCK_CACHE(_u)    \
   {   uint32  __handle__;                        \
       L2_WB_L2_HANDLE_SET(_u, __handle__);       \
       soc_scache_handle_unlock(_u, __handle__);  \
   }

#else  /* BCM_WARM_BOOT_SUPPORT */
#define L2_WB_LOCK_CACHE(_u) 
#define L2_WB_UNLOCK_CACHE(_u) 

#endif /* BCM_WARM_BOOT_SUPPORT */

#define L2_EGR_DEST_PORT_GET(unit, idx, gport) \
    do { \
        if ((_l2_egress[(unit)].dest_ports) && \
            ((idx) <= _l2_egress[(unit)].encap_id_max)) { \
            (gport) = *(((uint32 *)(_l2_egress[(unit)].dest_ports)) + idx); \
        } \
    } while (0)
    
#define L2_EGR_DEST_PORT_SET(unit, idx, gport) \
    do { \
        if ((_l2_egress[(unit)].dest_ports) && \
            ((idx) <= _l2_egress[(unit)].encap_id_max)) { \
             L2_WB_LOCK_CACHE(unit);                       \
            *(((uint32 *)(_l2_egress[(unit)].dest_ports)) + idx) = (gport); \
             L2_WB_UNLOCK_CACHE(unit);                       \
        } \
    } while (0)

#define L2_EGR_DEST_PORT_INVALID 0xffffffff

/*
 * L2 Callback Message Registration
 */
#define L2_CB_MAX             3    /* Max registered callbacks per unit */

typedef struct _l2_cb_entry_s {
    bcm_l2_addr_callback_t  fn;         /* Callback routine */
    void                    *fn_data;   /* User defined data */
} _l2_cb_entry_t;

typedef struct _l2_cb_s {
    _l2_cb_entry_t    entry[L2_CB_MAX];
    int               count;
} _l2_cb_t;

static _l2_cb_t                  _l2_cb[BCM_LOCAL_UNITS_MAX];

#define L2_CB_ENTRY(unit, i)    (_l2_cb[unit].entry[i])
#define L2_CB_COUNT(unit)       (_l2_cb[unit].count)

/* Execute registered callback routines for given unit */
#define L2_CB_RUN(unit, l2addr, insert)    \
    do {                                                              \
        int _i;                                                       \
        for (_i = 0; _i < L2_CB_MAX; _i++) {                          \
            if (L2_CB_ENTRY(unit, _i).fn) {                           \
                L2_CB_ENTRY(unit, _i).fn((unit), (l2addr), (insert),  \
                                   L2_CB_ENTRY((unit), _i).fn_data);  \
            }                                                         \
        }                                                             \
    } while (0)


/*
 * General Utility Macros
 */
#define UNIT_VALID_CHECK(unit) \
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) { return BCM_E_UNIT; }

#define UNIT_INIT_DONE(unit)    (_l2_mlock[unit] != NULL)

#define UNIT_INIT_CHECK(unit) \
    do { \
        UNIT_VALID_CHECK(unit); \
        if (_l2_mlock[unit] == NULL) { return BCM_E_INIT; } \
    } while (0)


#define TRUNK_VALID_CHECK(tid) \
    if (!SBX_TRUNK_VALID(tid)) { return BCM_E_BADID; }

#define VLAN_VALID_CHECK(vid) \
    if (!BCM_VLAN_VALID(vid)) { \
        return BCM_E_PARAM; \
    }


/*
 * BCM L2 flags NOT supported in device
 */
#define L2_FLAGS_UNSUPPORTED \
    ( BCM_L2_COPY_TO_CPU | BCM_L2_HIT       | BCM_L2_REPLACE_DYNAMIC | \
      BCM_L2_SRC_HIT     | BCM_L2_DES_HIT   | BCM_L2_REMOTE_LOOKUP   | \
      BCM_L2_NATIVE      | BCM_L2_MOVE      | BCM_L2_FROM_NATIVE     | \
      BCM_L2_TO_NATIVE   | BCM_L2_MOVE_PORT )


#define L2_EGRESS_VALID_ENCAP_ID(encap_id) \
    ((SOC_SBX_IS_VALID_L2_ENCAP_ID(encap_id)) && \
      (SOC_SBX_OFFSET_FROM_L2_ENCAP_ID((uint32)encap_id) < SBX_MAX_L2_EGRESS_OHI))

#define L2_EGRESS_VALID_ENCAP_ID_CHECK(encap_id) \
    if (!L2_EGRESS_VALID_ENCAP_ID(encap_id)) { return BCM_E_PARAM; }



/* L2 Stat state */
typedef struct _l2_stat_state_s {
    shr_htb_hash_table_t  fieldTable; /* hash(port,mac) -> fieldEntry */

    int                   numStatsInUse;
    int                   groupsValid;
    bcm_field_group_t     smacGroup;  /* count both smac & dmac */
    bcm_field_group_t     dmacGroup;
    
} _l2_stat_state_t;

typedef struct _l2_stat_fields_s {
    bcm_field_entry_t   smac;
    bcm_field_entry_t   dmac;
} _l2_stat_fields_t;

typedef uint8_t _l2_stat_key_t[10];

#define L2_STAT_NUM_QUALIFIERS    3


#define L2_MAX_STAT_ENTRIES       (4*1024)

static _l2_stat_state_t      *_l2_stat_state[BCM_LOCAL_UNITS_MAX];

#define L2_STAT_STATE(u) (_l2_stat_state[(u)])


/* Forward local function declarations */
STATIC int _bcm_fe2000_l2_age_timer_init(int unit);
STATIC int _bcm_fe2000_l2_age_timer_enable(int unit, int age_seconds);
STATIC int _bcm_fe2000_l2_age_timer_disable(int unit);
STATIC int _bcm_fe2000_l2_age_ager_set(int unit, uint32 ager);
STATIC int _bcm_fe2000_l2_egress_init(int unit);
int _bcm_fe2000_l2_addr_update_dest(int unit, bcm_l2_addr_t *l2addr, int qidunion);
extern int bcm_fe2000_l2_cache_init(int unit);
extern int bcm_fe2000_l2_cache_detach(int unit);

/*
 * Function:
 *     _bcm_fe2000_l2_age_ager_get
 * Purpose:
 *     Age accessor for derived subclasses.  Used to keep the 
 *     ager data static to this module
 * Parameters:
 *     unit - Device number
 * Returns:
 *     age of ager for the given unit
 */
int _bcm_fe2000_l2_age_ager_get(int unit) {
    return _l2_age[unit].ager;
}

/*
 * Function:
 *     _bcm_fe2000_l2_hw_init
 * Purpose:
 *     Initialize hardware and ucode L2 layer management tables
 *     for the specified device.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     BCM_E_NONE    - Success
 *     BCM_E_XXX     - Failure
 * Notes:
 *     Assumes lock is held.
 */
STATIC int
_bcm_fe2000_l2_hw_init(int unit)
{

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        return _bcm_fe2000_g2p3_l2_hw_init(unit);
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_UNAVAIL;
    }
}

int
_bcm_fe2000_l2_egress_detach(int unit)
{
    int rv = BCM_E_NONE;

    /* free resources */
    rv = shr_idxres_list_destroy(_l2_egress[unit].idList.idMgr);
    _l2_egress[unit].idList.idMgr = NULL;
    if (_l2_egress[unit].dest_ports) {
#ifdef BCM_WARM_BOOT_SUPPORT
#else
        sal_free(_l2_egress[unit].dest_ports);
#endif
        _l2_egress[unit].dest_ports = NULL;
    }

    return rv;
}


/*
 * Function:
 *     bcm_l2_detach
 * Purpose:
 *     Detach BCM L2 layer from unit.
 *     Deallocate software local resources such as semaphores.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_l2_detach(int unit)
{
    int  rv = BCM_E_NONE;
    int  i;

    UNIT_INIT_CHECK(unit);

    L2_LOCK(unit);

    /*
     * Call chip-dependent finalization ?
     */

    /* Stop L2 age timer and destroy semaphore */
    rv = _bcm_fe2000_l2_age_timer_disable(unit);
    if (BCM_SUCCESS(rv)) {
        if (_l2_age[unit].sem != NULL) {
            sal_sem_destroy(_l2_age[unit].sem);
            _l2_age[unit].sem = NULL;
        }
    }

    /* Clear L2 callback routines */
    for (i = 0; i < L2_CB_MAX; i++) {
        L2_CB_ENTRY(unit, i).fn = NULL;
        L2_CB_ENTRY(unit, i).fn_data = NULL;
    }
    L2_CB_COUNT(unit) = 0;

    if (BCM_FAILURE(rv = _bcm_fe2000_l2_egress_detach(unit))) {
        L2_ERR(("_bcm_fe2000_l2_egress_detach failed rv=%d(%s)", rv,
                bcm_errmsg(rv)));
    }

#if 0
    if (BCM_FAILURE(rv = bcm_fe2000_l2_cache_detach(unit))) {
        L2_ERR(("bcm_fe2000_l2_cache_detach failed rv=%d(%s)", rv,
                bcm_errmsg(rv)));
    }
#endif

    L2_UNLOCK(unit);

    /* Destroy mutex lock */
    sal_mutex_destroy(_l2_mlock[unit]);
    _l2_mlock[unit] = NULL;

    return rv;
}

/*
 * Function:
 *     bcm_l2_init
 * Purpose:
 *     Initialize the L2 interface layer for the specified device.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     BCM_E_NONE    - Success (or already initialized)
 *     BCM_E_MEMORY  - Failed to allocate required memory or lock
 *     BCM_E_XXX     - Failure, other errors
 */
int
bcm_fe2000_l2_init(int unit)
{
    int  rv = BCM_E_NONE;
    int  i;

    /* Check unit */
    UNIT_VALID_CHECK(unit);

    if (_l2_mlock[unit] != NULL) {
        rv = bcm_fe2000_l2_detach(unit);
        BCM_IF_ERROR_RETURN(rv);
        _l2_mlock[unit] = NULL;
    }

    /* Mutex lock */
    if (_l2_mlock[unit] == NULL) {
        if ((_l2_mlock[unit] = sal_mutex_create("bcm_l2_lock")) == NULL) {
            rv = BCM_E_MEMORY;
            L2_ERR((_SBX_D(unit, "failed to create L2 mutex rv=%d(%s)\n"),
                    rv, bcm_errmsg(rv)));
            return rv;
        }
    }

    L2_LOCK(unit);

    /* Initialize L2 age timer */
    if (BCM_FAILURE(rv = _bcm_fe2000_l2_age_timer_init(unit))) {
        L2_UNLOCK(unit);
        sal_mutex_destroy(_l2_mlock[unit]);
        _l2_mlock[unit] = NULL;
        return rv;
    }

    /* Initialize L2 callback routines */
    for (i = 0; i < L2_CB_MAX; i++) {
        L2_CB_ENTRY(unit, i).fn = NULL;
        L2_CB_ENTRY(unit, i).fn_data = NULL;
    }
    L2_CB_COUNT(unit) = 0;

    /* Initialize device hardware, ucode */
    if (BCM_FAILURE(rv = _bcm_fe2000_l2_hw_init(unit))) {
        L2_UNLOCK(unit);
        sal_mutex_destroy(_l2_mlock[unit]);
        _l2_mlock[unit] = NULL;
        return rv;        
    }

    L2_UNLOCK(unit);

    /* Initialize the l2 egress */
    if (BCM_FAILURE(rv = _bcm_fe2000_l2_egress_init(unit))) {
        L2_ERR((_SBX_D(unit, "_bcm_fe2000_l2_egress_init failed rv=%d(%s)"),
                rv, bcm_errmsg(rv)));
    }

    /* Initialize the l2_cache */
    if (BCM_FAILURE(rv = bcm_fe2000_l2_cache_init(unit))) {
        L2_ERR((_SBX_D(unit, "bcm_fe2000_l2_cache_init failed rv=%d(%s)"),
                rv, bcm_errmsg(rv)));
    }

    L2_VERB((_SBX_D(unit, "returning rv=%d\n"), rv));

    /* Disable the L2 API cache  */
    SOC_SBX_STATE(unit)->cache_l2 = FALSE;
    /* Enable l2_age deletion */
    SOC_SBX_STATE(unit)->l2_age_delete = TRUE;

    return rv;
}


/*
 * Function:
 *     bcm_l2_clear
 * Purpose:
 *     Clear the BCM L2 layer for given unit.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_l2_clear(int unit)
{
    UNIT_INIT_CHECK(unit);

    return bcm_fe2000_l2_init(unit);
}


/*
 * Function:
 *     _bcm_fe2000_l2addr_to_mcaddr
 * Purpose:
 *     Translate information from L2 'bcm_l2_addr_t' structure
 *     to MCAST 'bcm_mcast_addr_t' structure.
 * Parameters:
 *     unit      - Device number
 *     l2addr    - L2 address structure to translate
 *     mcaddr    - (OUT) MCAST address structure
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
STATIC int
_bcm_fe2000_l2addr_to_mcaddr(int unit, bcm_l2_addr_t  *l2addr,
                             bcm_mcast_addr_t *mcaddr)
{
    bcm_mcast_addr_t_init(mcaddr, l2addr->mac, l2addr->vid);
    mcaddr->cos_dst    = l2addr->cos_dst;
    mcaddr->l2mc_index = l2addr->l2mc_index;

    /* Adding the L2 MAC address starts without any port members */
    BCM_PBMP_CLEAR(mcaddr->pbmp);
    BCM_PBMP_CLEAR(mcaddr->ubmp);
    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_fe2000_l2_mcast_add
 * Purpose:
 *     Add a MCAST entry to Dmac table with given L2 address information.
 * Parameters:
 *     unit      - Device number
 *     l2addr    - L2 address to add
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
STATIC int
_bcm_fe2000_l2_mcast_add(int unit, bcm_l2_addr_t *l2addr)
{
    uint32            fte;
    uint32            mcgroup;
    bcm_mcast_addr_t  mcaddr;
    
    BCM_IF_ERROR_RETURN(_bcm_fe2000_l2addr_to_mcaddr(unit, l2addr, &mcaddr));

    return _bcm_fe2000_mcast_addr_add(unit, &mcaddr,
                                      l2addr->flags,
                                      BCM_FE2000_MCAST_ADD_SPEC_L2MCIDX,
                                      &fte, &mcgroup);
}


/*
 * Function:
 *     _bcm_fe2000_l2_mcast_get
 * Purpose:
 *     Update MCAST information in L2 address structure
 *     for given MAC address and VLAN ID.
 * Parameters:
 *     unit      - Device number
 *     l2addr    - (IN/OUT) L2 address structure to update
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
_bcm_fe2000_l2_mcast_get(int unit, bcm_l2_addr_t *l2addr)
{
    bcm_mcast_addr_t  mcaddr;

    BCM_IF_ERROR_RETURN(bcm_mcast_port_get(unit, l2addr->mac, l2addr->vid,
                                           &mcaddr));

    l2addr->l2mc_index = mcaddr.l2mc_index;
    l2addr->cos_dst    = mcaddr.cos_dst;
    l2addr->flags     |= BCM_L2_MCAST;

    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_fe2000_multicast_l2_encap_get
 * Purpose:
 *     Given the multicast group, port parameters get
 *     corresponding encap_id
 * Parameters:
 *     unit   - Device number
 *     group  - multicast group
 *     gport  - gport for exiting port
 *     vlan   - vlan for the l2 address
 *     encap_id - encap_id 
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     !!! Called from outside of L2 module (from multicast)
 *     Hence take and release necessary locks
 */

int
_bcm_fe2000_multicast_l2_encap_get(int              unit, 
                                   bcm_multicast_t  group,
                                   bcm_gport_t      gport,
                                   bcm_vlan_t       vlan,
                                   bcm_if_t        *encap_id)
{
    bcm_port_t port = gport;
    
    /* Check params and get device handler */
    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(encap_id);
    /* VLAN_VALID_CHECK(vlan); */

    if (BCM_GPORT_IS_LOCAL(gport)) {
        port = BCM_GPORT_LOCAL_GET(gport);
    } else if (BCM_GPORT_IS_MODPORT(gport)) {
        port  = BCM_GPORT_MODPORT_PORT_GET(gport);
    } else if (BCM_GPORT_IS_VLAN_PORT(gport)) {
        switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            return (_bcm_fe2000_g2p3_multicast_l2_encap_get(unit,
                                                            group,
                                                            gport,
                                                            vlan,
                                                            encap_id));
#endif /* BCM_FE2000_P3_SUPPORT */
        default:
            SBX_UNKNOWN_UCODE_WARN(unit);
            return BCM_E_CONFIG;
        }
    } else {
        return BCM_E_PORT;
    }

    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    *encap_id = vlan;

    return BCM_E_NONE;
}


int
_bcm_fe2000_l2addr_hw_add(int unit, bcm_l2_addr_t *l2addr)
{
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        return _bcm_fe2000_g2p3_l2addr_hw_add(unit, l2addr);
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_CONFIG;
    }

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *     bcm_l2_addr_add
 * Purpose:
 *     Add a MAC address to the Switch Address Resolution Logic (ARL)
 *     port with the given VLAN ID and parameters.
 * Parameters:
 *     unit   - Device number
 *     l2addr - Pointer to bcm_l2_addr_t containing all valid fields
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Use flag of BCM_L2_LOCAL_CPU to associate the entry with the CPU.
 *     Use flag of BCM_L2_COPY_TO_CPU to send a copy to the CPU.
 *     Use flag of BCM_L2_TRUNK_MEMBER to set trunking (TGID must be
 *     passed as well with non-zero trunk group ID)
 */
int
bcm_fe2000_l2_addr_add(int unit, bcm_l2_addr_t *l2addr)
{
    int    rv = BCM_E_NONE;

    /* Check params and get device handler */
    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(l2addr);

    /* Check for valid flags */
    if (l2addr->flags & L2_FLAGS_UNSUPPORTED) {
        return BCM_E_PARAM;
    }

    /* Special handling on a mcast entry */
    if (l2addr->flags & BCM_L2_MCAST) {
        L2_LOCK(unit);
        rv = _bcm_fe2000_l2_mcast_add(unit, l2addr);
        L2_UNLOCK(unit);
        return rv;        
    }

    /* If LOCAL_CPU, get local CPU port and modid */
    if (l2addr->flags & BCM_L2_LOCAL_CPU) {
        l2addr->port = CMIC_PORT(unit);
        BCM_IF_ERROR_RETURN(bcm_stk_modid_get(unit, &l2addr->modid));
    }

    rv = _bcm_fe2000_l2addr_hw_add(unit, l2addr);

    return rv;
}

/*
 * Function:
 *     bcm_l2_addr_get
 * Purpose:
 *     Given a MAC address and VLAN ID, return all associated information
 *     if entry is present in the L2 tables.
 * Parameters:
 *     unit   - Device number
 *     mac    - MAC address to search
 *     vid    - VLAN id to search
 *     l2addr - (OUT) Pointer to bcm_l2_addr_t structure to return L2 entry
 * Returns:
 *     BCM_E_NONE      - Success
 *     BCM_E_NOT_FOUND - L2 address entry (MAC+VLAN) not found
 *     BCM_E_PARAM     - Illegal parameter (NULL pointer)
 *     BCM_E_XXX       - Failure, other
 */
int
bcm_fe2000_l2_addr_get(int unit, sal_mac_addr_t mac, bcm_vlan_t vid,
                       bcm_l2_addr_t *l2addr)
{
    int  rv = BCM_E_UNAVAIL;

    /* Check params */
    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(l2addr);

    L2_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_l2_addr_get(unit, mac, vid, l2addr);
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_CONFIG;
    }

    L2_UNLOCK(unit);

    return rv;
}


/* 
 * Function:
 *     _bcm_fe2000_l2_flush_cache
 * Purpose:
 *     Unconditionally flush the ilib cached transactions to hw
 * Parameters:
 *     unit - BCM Device number
 * Returns:
 *     BCM_E_NONE      - Success
 */
int _bcm_fe2000_l2_flush_cache(int unit)
{
    int  rv = BCM_E_UNAVAIL;

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_l2_flush_cache(unit);
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_CONFIG;
    }

    return rv; 
}


/*
 * Function:
 *     bcm_l2_addr_delete
 * Purpose:
 *     Delete an L2 address (MAC+VLAN) from the device.
 * Parameters:
 *     unit - Device number
 *     mac  - MAC address to delete
 *     vid  - VLAN id 
 * Returns:
 *     BCM_E_NONE      - Success
 *     BCM_E_NOT_FOUND - L2 address entry (MAC+VLAN) not found
 *     BCM_E_XXX       - Failure
 */
int
bcm_fe2000_l2_addr_delete(int unit, bcm_mac_t mac, bcm_vlan_t vid)
{
    int rv = BCM_E_UNAVAIL;

    /* Check params */
    UNIT_INIT_CHECK(unit);

    L2_LOCK(unit);

      switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_l2_addr_delete(unit, mac, vid);
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
      default:
          SBX_UNKNOWN_UCODE_WARN(unit);
          rv = BCM_E_CONFIG;
      }

    L2_UNLOCK(unit);

    return rv;
}


/*
 * Function:
 *     _bcm_fe2000_l2_addr_delete_by
 * Purpose:
 *     Delete MAC entries associated with specified information
 *     from Smac and Dmac tables.
 * Parameters:
 *     unit  - bcm unit number
 *     match - L2 addr information to compare
 *     flags - Indicates what information to compare
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assumes lock is held.
 */
STATIC int
_bcm_fe2000_l2_addr_delete_by(int unit, bcm_l2_addr_t *match, uint32 cmp_flags)
{
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        return _bcm_fe2000_g2p3_l2_addr_delete_by(unit, match, cmp_flags);
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
    }

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *     bcm_l2_addr_delete_by_mac
 * Purpose:
 *     Delete L2 entries associated with a MAC address.
 * Parameters:
 *     unit  - Device number
 *     mac   - MAC address
 *     flags - BCM_L2_DELETE_XXX
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Static entries are removed only if BCM_SBX_DELETE_STATIC flag is used.
 */
int
bcm_fe2000_l2_addr_delete_by_mac(int unit, bcm_mac_t mac, uint32 flags)
{
    int            rv = BCM_E_NONE;
    bcm_l2_addr_t  match;
    
    /* Check params */
    UNIT_INIT_CHECK(unit);

    L2_LOCK(unit);

    bcm_l2_addr_t_init(&match, mac, 0);
    match.flags = flags;
    rv = _bcm_fe2000_l2_addr_delete_by(unit, &match, L2_CMP_MAC);

    L2_UNLOCK(unit);

    L2_VERB((_SBX_D(unit, "Returning %d\n"), rv));

    return rv;
}


/*
 * Function:
 *     bcm_l2_addr_delete_by_vlan
 * Purpose:
 *     Delete L2 entries associated with a VLAN.
 * Parameters:
 *     unit  - Device number
 *     vid   - VLAN id
 *     flags - BCM_L2_DELETE_XXX
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Static entries are removed only if BCM_SBX_DELETE_STATIC flag is used.
 */
int
bcm_fe2000_l2_addr_delete_by_vlan(int unit, bcm_vlan_t vid, uint32 flags)
{
    int            rv = BCM_E_NONE;
    bcm_l2_addr_t  match;

    /* Check params */
    UNIT_INIT_CHECK(unit);
    VLAN_VALID_CHECK(vid);

    L2_LOCK(unit);

    sal_memset(&match, 0, sizeof(match));
    match.vid   = vid;
    match.flags = flags;
    rv = _bcm_fe2000_l2_addr_delete_by(unit, &match, L2_CMP_VLAN);

    L2_UNLOCK(unit);

    L2_VERB((_SBX_D(unit, "Returning %d\n"), rv));

    return rv;
}


/*
 * Function:
 *     bcm_l2_addr_delete_by_port
 * Purpose:
 *     Delete L2 entries associated with a destination module/port.
 * Parameters:
 *     unit  - Device number
 *     mod   - Module id
 *     port  - Port
 *     flags - BCM_L2_DELETE_XXX
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Static entries are removed only if BCM_SBX_DELETE_STATIC flag is used.
 */
int
bcm_fe2000_l2_addr_delete_by_port(int unit, bcm_module_t mod, bcm_port_t port,
                                  uint32 flags)
{
    int            rv = BCM_E_NONE;
    bcm_l2_addr_t  match;
    
    /* Check params */
    /* Need to check for mod/port */
    UNIT_INIT_CHECK(unit);

    L2_LOCK(unit);

    match.modid = mod;
    match.port  = port;
    match.flags = flags;
    rv = _bcm_fe2000_l2_addr_delete_by(unit, &match, L2_CMP_PORT);

    L2_UNLOCK(unit);

    L2_VERB((_SBX_D(unit, "Returning %d\n"), rv));

    return rv;
}


/*
 * Function:
 *     bcm_l2_addr_delete_by_trunk
 * Purpose:
 *     Delete L2 entries associated with a trunk.
 * Parameters:
 *     unit  - Device number
 *     tid   - Trunk id
 *     flags - BCM_L2_DELETE_XXX
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Static entries are removed only if BCM_SBX_DELETE_STATIC flag is used.
 */
int
bcm_fe2000_l2_addr_delete_by_trunk(int unit, bcm_trunk_t tid, uint32 flags)
{
    int            rv = BCM_E_NONE;
    bcm_l2_addr_t  match;
    
    /* Check params */
    UNIT_INIT_CHECK(unit);
    TRUNK_VALID_CHECK(tid);
    
    L2_LOCK(unit);

    match.tgid  = tid;
    match.flags = flags;
    rv = _bcm_fe2000_l2_addr_delete_by(unit, &match, L2_CMP_TRUNK);

    L2_UNLOCK(unit);

    L2_VERB((_SBX_D(unit, "Returning %d\n"), rv));

    return rv;
}


/*
 * Function:
 *     bcm_l2_addr_delete_by_mac_port
 * Purpose:
 *     Delete L2 entries associated with a MAC address and
 *     a destination module/port.
 * Parameters:
 *     unit  - Device number
 *     mac   - MAC address
 *     mod   - Module id
 *     port  - Port
 *     flags - BCM_L2_DELETE_XXX
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Static entries are removed only if BCM_SBX_DELETE_STATIC flag is used.
 */
int
bcm_fe2000_l2_addr_delete_by_mac_port(int unit, bcm_mac_t mac,
                                      bcm_module_t mod, bcm_port_t port,
                                      uint32 flags)
{
    int            rv = BCM_E_NONE;
    bcm_l2_addr_t  match;

    /* Check params */
    /* Need to check mod/port */
    UNIT_INIT_CHECK(unit);

    L2_LOCK(unit);

    bcm_l2_addr_t_init(&match, mac, 0);
    match.modid = mod;
    match.port  = port;
    match.flags = flags;
    rv = _bcm_fe2000_l2_addr_delete_by(unit, &match, L2_CMP_MAC | L2_CMP_PORT);

    L2_UNLOCK(unit);

    L2_VERB((_SBX_D(unit, "Returning %d\n"), rv));

    return rv;
}


/*
 * Function:
 *     bcm_l2_addr_delete_by_vlan_port
 * Purpose:
 *     Delete L2 entries associated with a VLAN and
 *     a destination module/port.
 * Parameters:
 *     unit  - Device number
 *     vid   - VLAN id
 *     mod   - Module id
 *     port  - Port
 *     flags - BCM_L2_DELETE_XXX
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Static entries are removed only if BCM_SBX_DELETE_STATIC flag is used.
 */
int
bcm_fe2000_l2_addr_delete_by_vlan_port(int unit, bcm_vlan_t vid,
                                       bcm_module_t mod, bcm_port_t port,
                                       uint32 flags)
{
    int            rv = BCM_E_NONE;
    bcm_l2_addr_t  match;

    /* Check params */
    /* Need to check mod/port */
    UNIT_INIT_CHECK(unit);
    VLAN_VALID_CHECK(vid);

    L2_LOCK(unit);

    match.vid   = vid;
    match.modid = mod;
    match.port  = port;
    match.flags = flags;
    rv = _bcm_fe2000_l2_addr_delete_by(unit, &match,
                                       L2_CMP_VLAN | L2_CMP_PORT);

    L2_UNLOCK(unit);

    L2_VERB((_SBX_D(unit, "Returning %d\n"), rv));

    return rv;
}


/*
 * Function:
 *     bcm_l2_addr_delete_by_vlan_trunk
 * Purpose:
 *     Delete L2 entries associated with a VLAN and a destination trunk.
 * Parameters:
 *     unit  - Device number
 *     vid   - VLAN id
 *     tid   - Trunk id
 *     flags - BCM_L2_DELETE_XXX
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Static entries are removed only if BCM_SBX_DELETE_STATIC flag is used.
 */
int
bcm_fe2000_l2_addr_delete_by_vlan_trunk(int unit, bcm_vlan_t vid,
                                        bcm_trunk_t tid, uint32 flags)
{
    int            rv = BCM_E_NONE;
    bcm_l2_addr_t  match;

    /* Check params */
    UNIT_INIT_CHECK(unit);
    VLAN_VALID_CHECK(vid);
    TRUNK_VALID_CHECK(tid);

    L2_LOCK(unit);

    match.vid   = vid;
    match.tgid  = tid;
    match.flags = flags;
    rv = _bcm_fe2000_l2_addr_delete_by(unit, &match,
                                       L2_CMP_VLAN | L2_CMP_TRUNK);

    L2_UNLOCK(unit);

    L2_VERB((_SBX_D(unit, "Returning %d\n"), rv));

    return rv;
}

      
/*
 * Function:
 *     bcm_l2_addr_register
 * Purpose:
 *     Register a callback routine that will be called whenever
 *     an entry is inserted into or deleted from the L2 address table
 *     by the BCM layer.
 *     For FE2000 devices, the BCM layer only perform deletes of
 *     old mac entries from L2 address table when aging is enabled.
 * Parameters:
 *     unit    - Device number
 *     fn      - Callback function of type bcm_l2_addr_callback_t
 *     fn_data - Arbitrary value passed to callback along with messages
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     FE2000 does not support L2 learning and aging in hardware.
 *     Software aging provided by the BCM layer when aging is enabled.
 *     The BCM layer does not perform L2 learning.
 *     User must register callback to RX to get notification on new mac.
 */
int
bcm_fe2000_l2_addr_register(int unit, bcm_l2_addr_callback_t fn,
                            void *fn_data)
{
    int  i;

    /* Check params */
    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(fn);

    L2_LOCK(unit);

    /* Check if given function is already registered (with same data) */
    for (i = 0; i < L2_CB_MAX; i++) {
        if ((L2_CB_ENTRY(unit, i).fn == fn) &&
            (L2_CB_ENTRY(unit, i).fn_data == fn_data)) {
            L2_UNLOCK(unit);
            return BCM_E_NONE;
        }
    }

    /* Add to list */
    if (L2_CB_COUNT(unit) >= L2_CB_MAX) {
        L2_UNLOCK(unit);
        return BCM_E_RESOURCE;
    }

    for (i = 0; i < L2_CB_MAX; i++) {
        if (L2_CB_ENTRY(unit, i).fn == NULL) {
            L2_CB_ENTRY(unit, i).fn = fn;
            L2_CB_ENTRY(unit, i).fn_data = fn_data;
            L2_CB_COUNT(unit)++;
            break;
        }
    }

    L2_UNLOCK(unit);

    L2_VERB((_SBX_D(unit, "fn=%p data=%p count=%d\n"),
             fn, fn_data, L2_CB_COUNT(unit)));

    return BCM_E_NONE;
}


/*
 * Function:
 *     bcm_l2_addr_unregister
 * Purpose:
 *     Unregister a previously registered callback routine.
 * Parameters:
 *     unit    - Device number
 *     fn      - Same callback function used to register callback
 *     fn_data - Same arbitrary value used to register callback
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_l2_addr_unregister(int unit, bcm_l2_addr_callback_t fn,
                              void *fn_data)
{
    int  i;

    /* Check params */
    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(fn);

    L2_LOCK(unit);

    if (L2_CB_COUNT(unit) == 0) {
        L2_UNLOCK(unit);
        return BCM_E_NOT_FOUND;
    }        

    for (i = 0; i < L2_CB_MAX; i++) {
        if ((L2_CB_ENTRY(unit, i).fn == fn) &&
            (L2_CB_ENTRY(unit, i).fn_data == fn_data)) {
            L2_CB_ENTRY(unit, i).fn = NULL;
            L2_CB_ENTRY(unit, i).fn_data = NULL;
            L2_CB_COUNT(unit)--;
            break;
        }
    }

    L2_UNLOCK(unit);

    if (i >= L2_CB_MAX) {
        return BCM_E_NOT_FOUND;
    }

    L2_VERB((_SBX_D(unit, "fn=%p data=%p count=%d\n"),
             fn, fn_data, L2_CB_COUNT(unit)));

    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_fe2000_l2_mac_size_get
 * Purpose:
 *     Get the L2 MAC table size.
 * Parameters:
 *     unit       - Device number
 *     table_size - Returns the L2 MAC table size.
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assumes valid params.
 */
STATIC int
_bcm_fe2000_l2_mac_size_get(int unit, int *table_size)
{
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        return _bcm_fe2000_g2p3_l2_mac_size_get(unit, table_size);
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
    }
    
    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *     _bcm_fe2000_l2_age_timer_init
 * Purpose:
 *     Initialize the L2 age timer submodule.
 *     Create semaphore and initialize L2 age timer internal information.
 *     It does NOT start the L2 age timer thread (if thread is running,
 *     this will be stopped).
 * Parameters:
 *     unit - Device number
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure, could not create signaling semaphore, or
 *                  could not stop L2 age timer thread
 * Notes:
 *     Assumes lock is held.
 */
STATIC int
_bcm_fe2000_l2_age_timer_init(int unit)
{
    int   rv = BCM_E_NONE;
    int   table_max;
    int   cycles;
    char  sem_name[20];


    /* Init ager */
    BCM_IF_ERROR_RETURN(_bcm_fe2000_l2_age_ager_set(unit, 0));
    
    /* Get max size for L2 MAC table array */
    BCM_IF_ERROR_RETURN(_bcm_fe2000_l2_mac_size_get(unit, &table_max));

    /* Get number of cycles per age interval */
    cycles =  SOC_SBX_CFG_FE2000(unit)->l2_age_cycles;

    /*
     * Get max positive integer value, needed to determine max
     * value allowed for sal_sem_take().
     */
    _l2_sem_usec_max = (1 << ((sizeof(int) * 8) - 1));
    _l2_sem_usec_max--;

    /* Initialize age interval and cycle information */
    _l2_age[unit].num_old_mac = 0;
    _l2_age[unit].table_max = table_max;
    _l2_age[unit].cycles_interval = cycles;
    _l2_age[unit].entries_cycle = (table_max + (cycles - 1)) / cycles;

    /* Initialize times */
    _l2_age[unit].age_sec = 0;
    _l2_age[unit].cycle_usec = 0;

    /* Initialize L2 age timer thread information, stop thread if running */
    _l2_age[unit].thread_name[0] ='\0';
    _l2_age[unit].sleep_count    = 0;
    if (_l2_age[unit].thread_id != NULL) {
        rv = _bcm_fe2000_l2_age_timer_disable(unit);
    }

    /* Create L2 age timer semaphore */
    if (_l2_age[unit].sem == NULL) {
        sal_snprintf(sem_name, sizeof(sem_name), "bcm_l2_age_SLEEP%d", unit);
        if ((_l2_age[unit].sem = sal_sem_create(sem_name,
                                                sal_sem_BINARY, 0)) == NULL) {
            return BCM_E_MEMORY;
        }
    }

    L2_VERB((_SBX_D(unit, "L2 age timer init table_max=%d "
                    "cycles=%d entries_cycles=%d rv=%d\n"),
             table_max, cycles, _l2_age[unit].entries_cycle, rv));

    return rv;
}


/*
 * Function:
 *     _bcm_fe2000_l2_age_timer_thread_stop
 * Purpose:
 *     Stop L2 age timer thread.  Clears thread information.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure, could not stop L2 age timer thread
 * Notes:
 *     Assumes lock is held.
 */
STATIC int
_bcm_fe2000_l2_age_timer_thread_stop(int unit)
{
    int  retries;

    _l2_age[unit].age_sec = 0;
    _l2_age[unit].cycle_usec = 0;
    _l2_age[unit].thread_name[0] ='\0';

    if (_l2_age[unit].thread_id == NULL) {
        return BCM_E_NONE;
    }

    /* Stop L2 age timer thread */
    L2_AGE_TIMER_WAKE(unit);

    /* Wait for thread to exit */
    for (retries = 0; retries < L2_AGE_TIMER_STOP_RETRIES_MAX; retries++) {
        if (_l2_age[unit].thread_id == NULL) {
            break;
        }
        sal_usleep(L2_AGE_TIMER_STOP_WAIT_USEC);
    }
    if (retries >= L2_AGE_TIMER_STOP_RETRIES_MAX) {
        L2_ERR((_SBX_D(unit, "L2 age timer thread did not stop as requested\n")));
        return BCM_E_INTERNAL;
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_fe2000_l2_age_ager_set
 * Purpose:
 *     Set the L2 ager timestamp (age).
 * Parameters:
 *     unit - Device number
 *     ager - Age ager to set, 0..15
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assumes valid unit/ager values.
 */
STATIC int
_bcm_fe2000_l2_age_ager_set(int unit, uint32 ager)
{
    _l2_age[unit].ager = ager;

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        return _bcm_fe2000_g2p3_l2_age_ager_set(unit, ager);
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
    }
    
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     _bcm_fe2000_l2_age_async_cb
 * Purpose:
 *     L2 age async callback routine to remove given MAC entry from
 *     Smac and Dmac tables.
 *     This routine is called from the microcode aging function
 *     each time there is an aged mac entry (async type
 *     is SB_FE_ASYNC_OLD_SMAC).
 * Parameters:
 *     unit       - Device number
 *     mac_handle - Contains VLAN and MAC of aged entry
 * Returns:
 *     None
 * Notes:
 *     This routine is hooked to the gu2 async processing routine
 *     only when age is enabled (see _bcm_fe2000_l2_age_timer_thread()
 *     routine).
 */
STATIC void
_bcm_fe2000_l2_age_async_cb(int unit, uint8 *mac_handle)
{
    int             rv = BCM_E_UNAVAIL;
    bcm_l2_addr_t   l2addr;
    bcm_l2_addr_t   *payload = NULL;
    
    L2_LOCK(unit);

    /* If callback routines are registered, get payload data */
    if (L2_CB_COUNT(unit)) {
        payload = &l2addr;
    }

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_l2_age_remove(unit, (int*)mac_handle, payload);
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_CONFIG;
    }

    /* Update old entries count, callback if needed */
    if (BCM_SUCCESS(rv)) {
        _l2_age[unit].num_old_mac++;

        /* Callback to registered user routines */
        if (L2_CB_COUNT(unit)) {
            L2_CB_RUN(unit, &l2addr, BCM_L2_CALLBACK_DELETE);
        }
    }

    L2_UNLOCK(unit);

    return;
}

/*
 * Function:
 *     _bcm_fe2000_l2_scan_async_cb
 * Purpose:
 *     L2 scan async callback routine to report given MAC entry from
 *     Smac and Dmac tables.
 *     This routine is called from the microcode aging function
 *     each time there is an aged mac entry (async type
 *     is SB_FE_ASYNC_OLD_SMAC).
 * Parameters:
 *     unit       - Device number
 *     mac_handle - Contains VLAN and MAC of aged entry
 * Returns:
 *     None
 * Notes:
 *     This routine is hooked to the gu2 async processing routine
 *     only when age is enabled (see _bcm_fe2000_l2_age_timer_thread()
 *     routine).  The report only version of _bcm_fe2000_l2_age_async_cb
 */
STATIC void
_bcm_fe2000_l2_scan_async_cb(int unit, uint8 *mac_handle)
{
    int             rv = BCM_E_UNAVAIL;
    bcm_l2_addr_t   l2addr;
    sal_mac_addr_t  mac;
    int             vid;
    int             bmac;
   
    L2_LOCK(unit);

    /* If there is no callback routines registered, just return */
    if (L2_CB_COUNT(unit) == 0) {
        L2_UNLOCK(unit);
        return;
    }        

    rv = soc_sbx_g2p3_mac_index_unpack(unit, (int *) mac_handle, mac,
                                       &vid, &bmac);

    if (BCM_SUCCESS(rv) && !bmac) {
        rv = bcm_fe2000_l2_addr_get(unit, mac, vid, &l2addr);

        /* Update old entries count, callback if needed */
        if (BCM_SUCCESS(rv)) {
            _l2_age[unit].num_old_mac++;
            
            /* Callback to registered user routines */
            if (L2_CB_COUNT(unit)) {
                L2_CB_RUN(unit, &l2addr, BCM_L2_CALLBACK_REPORT);
            }
        }
    }

    L2_UNLOCK(unit);

    return;
}

#ifndef SB_ZF_G2_FE_SMACPYLD_AGE_MAX
#define SB_ZF_G2_FE_SMACPYLD_AGE_MAX 0xF
#endif

/*
 * Function:
 *     _bcm_fe2000_l2_age_timer_thread
 * Purpose:
 *     L2 age timer thread to run the microcode aging process
 *     at given intervals of time.
 *
 *     The BCM L2 age async callback routine _bcm_fe2000_l2_age_async_cb()
 *     is hooked to the main ILIB async callback _gu2_async_callback() to
 *     get notification for aged mac entries.  This is later uninstall
 *     when thread exits.
 *
 *     This routine will perform the 'commit' of mac entries
 *     'removed' by the l2 age async callback routine
 *     _bcm_fe2000_l2_age_async_cb().
 * Parameters:
 *     unit - Device number
 * Returns:
 *     None
 */
STATIC void
_bcm_fe2000_l2_age_timer_thread(int unit)
{
    int          rv = BCM_E_UNAVAIL;
    int          report_only;
    int          cycles_done;
    uint64       cycle_age;
    uint64       cycle_left;
    uint64       cycle_total;
    sal_usecs_t  cycle_start;
    sal_usecs_t  cycle_end;
    uint64       interval_left;
#ifdef BCM_FE2000_P3_SUPPORT
    void *maccstate = NULL;
#endif /* BCM_FE2000_P3_SUPPORT */

    if (SOC_SBX_STATE(unit)->l2_age_delete == FALSE) {
       report_only = 1;
    } else {
       report_only = 0;
    }
    
    /* Get device handler */
    /*
     * Install L2 age async callback routine to be notified of
     * aged mac entries by microcode aging process
     */
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        maccstate = soc_sbx_g2p3_mac_complex_state_get(unit);
        rv = BCM_E_NONE;
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
    }

    if (BCM_FAILURE(rv)) {
        L2_ERR((_SBX_D(unit, "L2 age timer thread, unable to get device "
                       "handler: %d %s\n"), rv, bcm_errmsg(rv)));
        return;
    }

    L2_VERB((_SBX_D(unit, "L2 age timer thread starting\n")));

    _l2_age[unit].thread_id = sal_thread_self();

    /* Main thread loop */
    /* Changes in the age time will take effect on the next age interval */
    while (_l2_age[unit].age_sec != 0) {
        L2_VERB((_SBX_D(unit, "L2 age timer interval start age=%d(seconds)\n"),
                 _l2_age[unit].age_sec));

        cycles_done = 0;
        cycle_total = 0;
        cycle_age   = _l2_age[unit].cycle_usec;

        while (!cycles_done) {

            /* Mark starting cycle time */
            cycle_start = sal_time_usecs();

            /* Run aging */
            switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
            case SOC_SBX_UCODE_TYPE_G2P3:
                rv = soc_sbx_g2p3_eml_run_aging(unit, maccstate,
                                                _l2_age[unit].ager,
                                                L2_AGE_DELTA,
                                                _l2_age[unit].entries_cycle,
                                                report_only
                                                ? _bcm_fe2000_l2_scan_async_cb
                                                : _bcm_fe2000_l2_age_async_cb);
                if (rv == SOC_E_NOT_FOUND) {
                   /* All table has been processed */
                   cycles_done = 1;
                   rv = SOC_E_NONE;
                }
                break;
#endif /* BCM_FE2000_P3_SUPPORT */
            default:
                rv = BCM_E_CONFIG;
            }

            if (BCM_FAILURE(rv)) {
                L2_WARN(("L2 age unit=%d, error during age run rv=%d\n",
                         unit, rv));
                break;
            }

            /* Perform 'commit' if there are any mac entries 'removed' */
            L2_LOCK(unit);
            if (!report_only && _l2_age[unit].num_old_mac) {
                L2_VERB((_SBX_D(unit, "L2 age delete commit count=%d\n"),
                         _l2_age[unit].num_old_mac));

                switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
                case SOC_SBX_UCODE_TYPE_G2P3:
                    soc_sbx_g2p3_mac_commit(unit, SB_COMMIT_COMPLETE);
                    break;
#endif /* BCM_FE2000_P3_SUPPORT */                    
                default:
                    rv = BCM_E_CONFIG;
                }

                _l2_age[unit].num_old_mac = 0;
            }
            L2_UNLOCK(unit);

            /* Mark ending cycle time (including 'commit' time above) */
            cycle_end = sal_time_usecs();


            /* Calculate time left in a cycle */
            /* Set minimum delay if cycle took longer or bad time value */
            /* NOTE:
             * This works fine if we assume that the Ilib aging does not
             * take more than 71 minutes per run.  Otherwise, the calculation
             * of how much time is left would be off.
             *
             * The calculated run aging time is limited to the
             * max value supported by sal_time_usecs(), which
             * is 4294.967296 seconds (~71.5 minutes) before it
             * wraps around (see comments in 'time.h')
             */

            if ((cycle_end < cycle_start) ||
                ((cycle_end - cycle_start) > cycle_age)) {
                cycle_left = L2_AGE_CYCLE_MIN_USEC;
            } else {
                cycle_left = cycle_age;
                COMPILER_64_SUB_32(cycle_left, (cycle_end - cycle_start));
            }
            
            /* Complete (or apply minimum delay) age cycle time */
            L2_AGE_TIMER_SLEEP(unit, cycle_left);

            COMPILER_64_ADD_64(cycle_total, cycle_age);
        }

        /* Check for exit */
        if (_l2_age[unit].age_sec == 0) {
            break;
        }


        /* Complete age interval time, if needed */
        interval_left = ((uint64)_l2_age[unit].age_sec) * SECOND_USEC;
        if (interval_left > cycle_total) {
            COMPILER_64_SUB_64(interval_left, cycle_total);
            L2_AGE_TIMER_SLEEP(unit, interval_left);
        }

        /* Increment ager step */
        L2_AGE_INCR(_l2_age[unit].ager);
        rv = _bcm_fe2000_l2_age_ager_set(unit, _l2_age[unit].ager);
        if (BCM_FAILURE(rv)) {
            L2_ERR((_SBX_D(unit, "L2 age timer thread, unable to update ager: "
                           "%d %s\n"), rv, bcm_errmsg(rv)));
            break;
        }
    }

    /* Uninstall L2 age async callback routine */
    _l2_age[unit].thread_id = NULL;

    L2_VERB((_SBX_D(unit, "L2 age timer thread exiting\n")));

    sal_thread_exit(0);
}


/*
 * Function:
 *     _bcm_fe2000_l2_age_timer_enable
 * Purpose:
 *     Enable L2 age timer:
 *     (1) Enable and set aging in microcode.
 *         The microcode will update the MAC 'age' field
 *         when a HIT occurs.
 *     (2) Start the L2 age timer thread.
 *         The thread will wake up at each intervals of time
 *         and remove old MAC entries.
 * Parameters:
 *     unit        - Device number
 *     age_seconds - Age timer value in seconds
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assumes lock is held.
 */
STATIC int
_bcm_fe2000_l2_age_timer_enable(int unit, int age_seconds)
{
    int       cycles;

    /*
     * Age information
     */
    cycles = _l2_age[unit].cycles_interval;
    _l2_age[unit].age_sec = age_seconds;
    _l2_age[unit].cycle_usec = ((((uint64) age_seconds) * SECOND_USEC)
                                + (cycles - 1)) / cycles;

    /* If thread running, just notify with updated interval and return */
    if (_l2_age[unit].thread_id != NULL) {
        L2_AGE_TIMER_WAKE(unit);
        return BCM_E_NONE;
    }

    /*
     * Enable age updates by microcode
     */
    BCM_IF_ERROR_RETURN(_bcm_fe2000_l2_age_ager_set(unit, 0));

    /* Create L2 age timer thread */
    sal_snprintf(_l2_age[unit].thread_name,
                 sizeof(_l2_age[unit].thread_name),
                 "bcmL2Ager.%d", unit);
    if (sal_thread_create(_l2_age[unit].thread_name,
                          SAL_THREAD_STKSZ,
                          L2_AGE_TIMER_THREAD_PRIO,
                          (void (*)(void*))_bcm_fe2000_l2_age_timer_thread,
                          INT_TO_PTR(unit)) == SAL_THREAD_ERROR) {
        _l2_age[unit].thread_name[0] ='\0';
        _l2_age[unit].age_sec        = 0;

        return BCM_E_MEMORY;
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_fe2000_l2_age_timer_disable
 * Purpose:
 *     Disable L2 age timer:
 *     (1) Stop the L2 age timer thread.
 *     (2) Disable aging in microcode.
 * Parameters:
 *     unit        - Device number
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assumes lock is held.
 */
STATIC int
_bcm_fe2000_l2_age_timer_disable(int unit)
{
    /* Stop L2 age timer thread */
    BCM_IF_ERROR_RETURN(_bcm_fe2000_l2_age_timer_thread_stop(unit));

    /* Reset age stamp */
    BCM_IF_ERROR_RETURN(_bcm_fe2000_l2_age_ager_set(unit, 0));

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_l2_age_timer_get
 * Purpose:
 *     Returns the current age timer value.
 *     The value is 0 if aging is not enabled.
 * Parameters:
 *     unit        - Device number
 *     age_seconds - Place to store returned age timer value in seconds
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_l2_age_timer_get(int unit, int *age_seconds)
{
    int       rv = BCM_E_NONE;

    /* Check params and get device handler */
    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(age_seconds);

    *age_seconds = _l2_age[unit].age_sec;

    L2_VERB((_SBX_D(unit, "age=%d(seconds), age-cycles=%d, rv=%d\n"),
             *age_seconds, _l2_age[unit].cycles_interval, rv));

    return rv;
}


/*
 * Function:
 *     bcm_l2_age_timer_set
 * Purpose:
 *     Set the age timer for all blocks.
 *     Setting the value to 0 disables the age timer.
 * Parameters:
 *     unit        - Device number
 *     age_seconds - Age timer value in seconds
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_l2_age_timer_set(int unit, int age_seconds)
{
    int       rv = BCM_E_NONE;

    /* Check params */
    UNIT_INIT_CHECK(unit);
    if ((age_seconds < 0) || (age_seconds > L2_AGE_TIMER_MAX_SECS)) {
        return BCM_E_PARAM;
    }

    L2_LOCK(unit);
    if (age_seconds == 0) {
        rv = _bcm_fe2000_l2_age_timer_disable(unit);
    } else {
        rv = _bcm_fe2000_l2_age_timer_enable(unit, age_seconds);
    }
    L2_UNLOCK(unit);

    L2_VERB((_SBX_D(unit, "age=%d(seconds), age-cycles=%d, "
                    "cycle_usec=0x%08x%08x rv=%d\n"),
             age_seconds, _l2_age[unit].cycles_interval,
             COMPILER_64_HI(_l2_age[unit].cycle_usec),
             COMPILER_64_LO(_l2_age[unit].cycle_usec),
             rv));

    return rv;
}

int
bcm_fe2000_l2_bpdu_count(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l2_bpdu_get(int unit,
                       int index,
                       sal_mac_addr_t *addr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l2_bpdu_set(int unit,
                       int index,
                       sal_mac_addr_t addr)
{
    return BCM_E_UNAVAIL;
}


/*
 * Displays Smac entries.
 *
 * Note: To start from first entry, set mac and vlan to zeros.
 *       To display to the end of list, set count to <= 0.
 */
STATIC int
_bcm_fe2000_smac_dump(int unit, bcm_mac_t mac, bcm_vlan_t vid, int max_count)
{
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        return _bcm_fe2000_g2p3_smac_dump(unit, mac, vid, max_count);
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_CONFIG;
    }

    return BCM_E_UNAVAIL;
}


STATIC int
_bcm_fe2000_dmac_dump(int unit, bcm_mac_t mac, bcm_vlan_t vid, int max_count)
{

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        return _bcm_fe2000_g2p3_dmac_dump(unit, mac, vid, max_count);
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_CONFIG;
    }

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *     _bcm_fe2000_l2_addr_dump
 * Purpose:
 *     Displays L2 Address Management Smac and Dmac table entries.
 *     If MAC and VLAN id are zeros, routine displays entries starting
 *     at the first entry in table.  Otherwise, routine displays
 *     entries starting at entry matching given MAC and VLAN.
 * Parameters:
 *     unit      - Device number
 *     mac       - Starting MAC address key to display
 *     vid       - Starting VLAN id key to display
 *     max_count - Max number of entries to display per table
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     To start from first entry, set mac and vlan to zeros.
 *     To display to the end of list, set count to <= 0.
 */
int
_bcm_fe2000_l2_addr_dump(int unit, bcm_mac_t mac, bcm_vlan_t vid,
                         int max_count)
{
    UNIT_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(_bcm_fe2000_smac_dump(unit, mac, vid, max_count));
    L2_DUMP("\n");
    BCM_IF_ERROR_RETURN(_bcm_fe2000_dmac_dump(unit, mac, vid, max_count));

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_fe2000_init_key
 * Purpose:
 *     Packs the relevant fields from bcm_l2_addr_t for hashing
 * Parameters:
 *      addr  - src address to pack
 *      key   - storage location for packed key
 * Returns:
 *    n/a
 * Notes:
 */
STATIC void
_bcm_fe2000_init_key(bcm_l2_addr_t *addr, _l2_stat_key_t key)
{
#define L2_KEY_PACK(dest, src, size) \
   sal_memcpy(dest, src, size);       \
   dest += size;

    uint8 *loc = key;

    /* pack the relevant l2 fields into the key */ 
    assert (sizeof(_l2_stat_key_t) >= 
            (sizeof(addr->mac) + sizeof(addr->port)));

    L2_KEY_PACK(loc, addr->mac, sizeof(addr->mac));
    L2_KEY_PACK(loc, &addr->port, sizeof(addr->port));
}


/*
 * Function:
 *     _bcm_fe2000_l2_stat_state_init
 * Purpose:
 *     initialize the statitistics state information
 * Parameters:
 *      unit - bcm device number
 * Returns:
 *    BCM_E_*
 * Notes:
 */
STATIC int
_bcm_fe2000_l2_stat_state_init(int unit)
{
    int rv;
    shr_htb_hash_table_t  fieldTable;

    rv = shr_htb_create(&fieldTable, L2_MAX_STAT_ENTRIES, 
                        sizeof(_l2_stat_key_t), "l2 field entry hash");
    if (BCM_FAILURE(rv)) {
        L2_ERR((_SBX_D(unit, "Failed to create hash table: %d %s\n"), 
                rv, bcm_errmsg(rv)));
        return rv;
    }

    L2_STAT_STATE(unit) = sal_alloc(sizeof(_l2_stat_state_t), "l2_stat_state");
    if (L2_STAT_STATE(unit) == NULL) {
        rv = shr_htb_destroy(&fieldTable, NULL);
        return BCM_E_MEMORY;
    }
    
    sal_memset(L2_STAT_STATE(unit), 0, sizeof(_l2_stat_state_t));

    L2_STAT_STATE(unit)->fieldTable = fieldTable;

    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_fe2000_l2_stat_state_destroy
 * Purpose:
 *     free resources associated with statitistics state information
 * Parameters:
 *      unit - bcm device number
 * Returns:
 *    BCM_E_*
 * Notes:
 */
STATIC int
_bcm_fe2000_l2_stat_state_destroy(int unit) 
{
    int rv;
    
    rv = shr_htb_destroy(&L2_STAT_STATE(unit)->fieldTable, NULL);
    if (BCM_FAILURE(rv)) {
        L2_ERR((_SBX_D(unit, "Failed to destroy hash table: %d %s\n"), 
                rv, bcm_errmsg(rv)));
    }
    
    sal_free(L2_STAT_STATE(unit));
    
    L2_STAT_STATE(unit) = NULL;
    return rv;
}


/*
 * Function:
 *     _bcm_fe2000_l2_stat_group_init
 * Purpose:
 *     Create the two groups necessary for src and destination mac
 *     statitisics
 * Parameters:
 *      unit - bcm device number
 * Returns:
 *    BCM_E_*
 * Notes:
 */
STATIC int
_bcm_fe2000_l2_stat_group_init(int unit)
{
    int rv;
    bcm_field_qset_t qset;
    int i;
    int qualifiers[L2_STAT_NUM_QUALIFIERS] = {
        bcmFieldQualifyStageIngressQoS,
        bcmFieldQualifyInPorts,
        bcmFieldQualifySrcMac
    };

    L2_VERB((_SBX_D(unit, "Enter\n")));

    BCM_FIELD_QSET_INIT(qset);
    for (i=0; i < L2_STAT_NUM_QUALIFIERS; i++) {
        BCM_FIELD_QSET_ADD(qset, qualifiers[i]);
    }

    rv = bcm_field_group_create(unit, qset, BCM_FIELD_GROUP_PRIO_ANY, 
                                &(L2_STAT_STATE(unit)->smacGroup));
    if (BCM_FAILURE(rv)) {
        L2_ERR((_SBX_D(unit, "Failed to create smac group: %d %s\n"), 
                rv, bcm_errmsg(rv)));
        return rv;
    }

    BCM_FIELD_QSET_INIT(qset);
    qualifiers[L2_STAT_NUM_QUALIFIERS - 1] = bcmFieldQualifyDstMac;
    for (i=0; i < L2_STAT_NUM_QUALIFIERS; i++) {
        BCM_FIELD_QSET_ADD(qset, qualifiers[i]);
    }

    rv = bcm_field_group_create(unit, qset, BCM_FIELD_GROUP_PRIO_ANY, 
                                &(L2_STAT_STATE(unit)->dmacGroup));

    if (BCM_FAILURE(rv)) {
        L2_ERR((_SBX_D(unit, "Failed to create dmac group: %d %s\n"),
                rv, bcm_errmsg(rv)));
        return rv;
    }

    L2_STAT_STATE(unit)->groupsValid = TRUE;
    L2_VERB((_SBX_D(unit, "Created smac group=%d,  dmac group=%d\n"),
             L2_STAT_STATE(unit)->smacGroup,
             L2_STAT_STATE(unit)->dmacGroup));

    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_fe2000_l2_stat_group_destroy
 * Purpose:
 *     free resources associated with the field groups for statistics
 * Parameters:
 *      unit - bcm device number
 * Returns:
 *    BCM_E_*
 * Notes:
 */
STATIC void 
_bcm_fe2000_l2_stat_group_destroy(int unit)
{
    int rv;

    L2_VERB((_SBX_D(unit, "Enter - destroying field groups\n")));

    rv = bcm_field_group_destroy(unit, L2_STAT_STATE(unit)->smacGroup);
    if (BCM_FAILURE(rv)) {
        L2_ERR((_SBX_D(unit, "Failed to destroy smac group, keep trying: %d %s\n"), 
                rv, bcm_errmsg(rv)));
    }

    rv = bcm_field_group_destroy(unit, L2_STAT_STATE(unit)->dmacGroup);
    if (BCM_FAILURE(rv)) {
        L2_ERR((_SBX_D(unit, "Failed to destroy dmac group, keep trying: %d %s\n"), 
                rv, bcm_errmsg(rv)));
    }

    L2_STAT_STATE(unit)->smacGroup = L2_STAT_STATE(unit)->dmacGroup = ~0;

    L2_STAT_STATE(unit)->groupsValid = FALSE;
}


/*
 * Function:
 *     _bcm_fe2000_l2_stat_field_entry_create
 * Purpose:
 *     Create the field entries used for statistics gathering
 * Parameters:
 *     unit    - bcm device number
 *     l2_addr - l2 address to track
 *     fields  - fields created for statistics information
 * Returns:
 *    BCM_E_*
 * Notes:
 */
int 
_bcm_fe2000_l2_stat_field_entry_create(int unit, bcm_l2_addr_t *l2_addr,
                                       _l2_stat_fields_t *fields)
{
    int rv;
    int idx;
    bcm_field_group_t *pGroup;
    bcm_field_entry_t *pEntry;
    bcm_pbmp_t pbmp;
    bcm_mac_t macMask;

    L2_VERB((_SBX_D(unit, "Enter port=%d mac=" L2_6B_MAC_FMT "\n"),
             l2_addr->port, L2_6B_MAC_PFMT(l2_addr->mac)));

    macMask[0] = macMask[1] = macMask[2] = 
        macMask[3] = macMask[4] = macMask[5] = 0xFF;

    pGroup = &L2_STAT_STATE(unit)->smacGroup;
    pEntry = &fields->smac;

    for (idx = 0; idx<2; idx++) {
        rv = bcm_field_entry_create(unit, *pGroup, pEntry);
        if (BCM_FAILURE(rv)) {
            L2_ERR((_SBX_D(unit, "Failed to create entry idx=%d group=%d: %d %s"),
                    idx, *pGroup, rv, bcm_errmsg(rv)));
            return rv;
        }

        BCM_PBMP_CLEAR(pbmp);
        BCM_PBMP_PORT_ADD(pbmp, l2_addr->port);
    
        rv = bcm_field_qualify_InPorts(unit, *pEntry, pbmp, PBMP_ALL(unit));
        if (BCM_FAILURE(rv)) {
            L2_ERR((_SBX_D(unit, "Failed to add qualifiers to entry=%d, group=%d idx=%d"
                       ": %d %s\n"),
                    *pEntry, *pGroup, idx, rv, bcm_errmsg(rv)));
            return rv;
        }
        
        if (idx == 0) {
            rv = bcm_field_qualify_SrcMac(unit, *pEntry, l2_addr->mac, 
                                          macMask);
        } else {
            rv = bcm_field_qualify_DstMac(unit, *pEntry, l2_addr->mac, 
                                          macMask);
        }
        
        if (BCM_FAILURE(rv)) {
            L2_ERR((_SBX_D(unit, "Failed to add %s qualifier: %d %s\n"),
                    (idx==0) ? "SrcMac" : "DstMac", rv, bcm_errmsg(rv)));
            return rv;
        }

        rv = bcm_field_entry_install(unit, *pEntry);
        if (BCM_FAILURE(rv)) {
            L2_ERR((_SBX_D(unit, "Failed to install entry %d into group %d: %d %s\n"),
                    *pEntry, *pGroup, rv, bcm_errmsg(rv)));
            return rv;
        }

        rv = bcm_field_counter_create(unit, *pEntry);
        if (BCM_FAILURE(rv)) {
            L2_ERR((_SBX_D(unit, "Failed to create counter on Entry %d, group %d"
                       ": %d %s\n"),
                    *pEntry, *pGroup, rv, bcm_errmsg(rv)));
            return rv;
        }

        rv = bcm_field_action_add(unit, *pEntry, bcmFieldActionUpdateCounter,
                                  BCM_FIELD_COUNTER_MODE_BYTES_PACKETS, 0);
        if (BCM_FAILURE(rv)) {
            L2_ERR((_SBX_D(unit, "Failed to add update counter action to entry %d, "
                       "group %d: %d %s\n"),
                    *pEntry, *pGroup, rv, bcm_errmsg(rv)));
            return rv;
        }

        pGroup = &L2_STAT_STATE(unit)->dmacGroup;
        pEntry = &fields->dmac;
    }

    L2_VERB((_SBX_D(unit, "Created smac entry %d for group %d\n"),
             fields->smac, L2_STAT_STATE(unit)->smacGroup));
    L2_VERB((_SBX_D(unit, "Created dmac entry %d for group %d\n"),
             fields->dmac, L2_STAT_STATE(unit)->dmacGroup));

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_fe2000_l2_stat_field_entry_destroy
 * Purpose:
 *     cleanup state associated with a single l2 statistic
 * Parameters:
 *      unit - bcm device number
 *      fields - fields to destroy
 * Returns:
 *    BCM_E_*
 * Notes:
 */
int
_bcm_fe2000_l2_stat_field_entry_destroy(int unit, _l2_stat_fields_t *fields)
{
    int rv;

    L2_VERB((_SBX_D(unit, "Enter\n")));
    
    rv = bcm_field_entry_remove(unit, fields->smac);
    if (BCM_FAILURE(rv)) {
        L2_ERR((_SBX_D(unit, "Failed to remove SMAC field match, keep trying:  %d %s\n"),
                rv, bcm_errmsg(rv)));
    }
       
    rv = bcm_field_entry_destroy(unit, fields->smac);
    if (BCM_FAILURE(rv)) {
        L2_ERR((_SBX_D(unit, "Failed to destroy SMAC field match, keep trying:  %d %s\n"),
                rv, bcm_errmsg(rv)));
    }

    rv = bcm_field_entry_remove(unit, fields->dmac);
    if (BCM_FAILURE(rv)) {
        L2_ERR((_SBX_D(unit, "Failed to remove DMAC field match, keep trying:  %d %s\n"),
                rv, bcm_errmsg(rv)));
    }

    rv = bcm_field_entry_destroy(unit, fields->dmac);
    if (BCM_FAILURE(rv)) {
        L2_ERR((_SBX_D(unit, "Failed to destroy DMAC field match, keep trying:  %d %s\n"),
                rv, bcm_errmsg(rv)));
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *      bcm_fe2000_l2_stat_get
 * Description:
 *      Get the specified L2 statistic from the chip. MAC entry must have 
 *      previously been enabled for stats collection.
 * Parameters:
 *      unit    - device unit number.
 *      l2_addr - Pointer to L2 address structure.
 *      stat    - L2 Entry statistics type
 *      val     - (OUT) 64-bit counter value.
 * Returns:
 *      BCM_E_UNAVAIL   - Counter/variable is not implemented on this current
 *                        chip.
 */
int bcm_fe2000_l2_stat_get(int unit, bcm_l2_addr_t *l2_addr, bcm_l2_stat_t stat, 
                           uint64 *val)
{
    int rv;
    _l2_stat_fields_t *fields;
    _l2_stat_key_t  key;

    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(l2_addr);

    if (L2_STAT_STATE(unit) == NULL) {
        L2_ERR((_SBX_D(unit, "Must enable a stat before retrieving entry\n")));
        return BCM_E_INIT;
    }

    _bcm_fe2000_init_key(l2_addr, key);
    rv = shr_htb_find(L2_STAT_STATE(unit)->fieldTable, key,
                      (shr_htb_data_t *)&fields,
                      0 /* == don't remove */);

    if (BCM_FAILURE(rv)) {
        L2_ERR((_SBX_D(unit, "Failed to find l2 addr: %d %s\n"), rv, bcm_errmsg(rv)));
        return rv;
    }

    switch (stat) {
    case bcmL2StatSourcePackets:
        rv =  bcm_field_counter_get(unit, fields->smac, 0, val);
        break;
    case bcmL2StatSourceBytes:
        rv =  bcm_field_counter_get(unit, fields->smac, 1, val);
        break;
    case bcmL2StatDestPackets:
        rv =  bcm_field_counter_get(unit, fields->dmac, 0, val);
        break;
    case bcmL2StatDestBytes:
        rv =  bcm_field_counter_get(unit, fields->dmac, 1, val);
        break;
    case bcmL2StatDropPackets: /* fall thru intentional */
    case bcmL2StatDropBytes: /* fall thru intentional */
        L2_ERR((_SBX_D(unit, "stat %d not supported\n"), stat));
        rv = BCM_E_PARAM;
        break;
    default:
        L2_ERR((_SBX_D(unit, "unrecognized stat:%d\n"), stat));
        rv = BCM_E_PARAM;
    }

    return rv;
}

/*
 * Function:
 *      bcm_fe2000_l2_stat_get32
 * Description:
 *      Get the specified L2 statistic from the chip
 * Parameters:
 *      unit    - device unit number.
 *      l2_addr - Pointer to L2 address structure.
 *      stat    - L2 Entry statistics type
 *      val     - (OUT) 32-bit counter value.
 * Returns:
 *      BCM_E_UNAVAIL   - Counter/variable is not implemented on this current
 *                        chip.
 * Notes:
 *      Same as bcm_fe2000_l2_stat_get, except converts result to 32-bit.
 */
int bcm_fe2000_l2_stat_get32(int unit, bcm_l2_addr_t *l2_addr, 
                             bcm_l2_stat_t stat, uint32 *val)
{
    int rv;
    uint64 tmp;

    rv = bcm_fe2000_l2_stat_get(unit, l2_addr, stat, &tmp);
    if (BCM_FAILURE(rv)) {
        L2_ERR((_SBX_D(unit, "Failed to get stat %d: %d %s\n"),
                stat, rv, bcm_errmsg(rv)));
        return rv;
    }
    
    if (tmp > (uint64)0xFFFFFFFF) {
        L2_WARN((_SBX_D(unit, "Data loss when 64bit int cast down to 32 bit for "
                    "stat %d\n"), stat));
    }
    *val = (uint32)tmp;
    
    return rv;
}    

/*
 * Function:
 *      bcm_fe2000_l2_stat_set
 * Description:
 *      Set the specified L2 statistic to the indicated value
 * Parameters:
 *      unit    - device unit number.
 *      l2_addr - Pointer to L2 address structure.
 *      stat    - L2 Entry statistics type
 *      val     - 64-bit value.
 * Returns:
 *      BCM_E_UNAVAIL   - Counter/variable is not implemented on this current
 *                        chip.
 * Notes:
 *      'val' must be zero (0).
 */
int bcm_fe2000_l2_stat_set(int unit, bcm_l2_addr_t *l2_addr, 
                           bcm_l2_stat_t stat, uint64 val)
{
    int rv;
    _l2_stat_fields_t *fields;
    _l2_stat_key_t  key;

    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(l2_addr);

    if (L2_STAT_STATE(unit) == NULL) {
        L2_ERR((_SBX_D(unit, "Must enable a stat before retrieving entry\n")));
        return BCM_E_INIT;
    }

    _bcm_fe2000_init_key(l2_addr, key);
    rv = shr_htb_find(L2_STAT_STATE(unit)->fieldTable, key,
                      (shr_htb_data_t *)&fields,
                      0 /* == don't remove */);

    if (BCM_FAILURE(rv)) {
        L2_ERR((_SBX_D(unit, "Failed to find l2 addr: %d %s\n"), rv, bcm_errmsg(rv)));
        return rv;
    }

    switch (stat) {
    case bcmL2StatSourcePackets:
        rv =  bcm_field_counter_set(unit, fields->smac, 0, val);
        break;
    case bcmL2StatSourceBytes:
        rv =  bcm_field_counter_set(unit, fields->smac, 1, val);
        break;
    case bcmL2StatDestPackets:
        rv =  bcm_field_counter_set(unit, fields->dmac, 0, val);
        break;
    case bcmL2StatDestBytes:
        rv =  bcm_field_counter_set(unit, fields->dmac, 1, val);
        break;
    case bcmL2StatDropPackets: /* fall thru intentional */
    case bcmL2StatDropBytes: /* fall thru intentional */
        L2_ERR((_SBX_D(unit, "stat %d not supported\n"), stat));
        rv = BCM_E_PARAM;
        break;
    default:
        L2_ERR((_SBX_D(unit, "unrecognized stat:%d\n"), stat));
        rv = BCM_E_PARAM;
    }

    return rv;
}

/*
 * Function:
 *      bcm_fe2000_l2_stat_set32
 * Description:
 *      Set the specified L2 statistic to the indicated value
 * Parameters:
 *      unit    - device unit number.
 *      l2_addr - Pointer to L2 address structure.
 *      stat    - L2 Entry statistics type
 *      val     - 32-bit value.
 * Returns:
 *      BCM_E_UNAVAIL   - Counter/variable is not implemented on this current
 *                        chip.
 * Notes:
 *      Same as bcm_fe2000_l2_stat_set, except accepts 32-bit value. 
 *      'val' must be zero (0).
 */
int bcm_fe2000_l2_stat_set32(int unit, bcm_l2_addr_t *l2_addr, 
                             bcm_l2_stat_t stat, uint32 val)
{
    return bcm_fe2000_l2_stat_set(unit, l2_addr, stat, (uint64)val);
}

/*
 * Function:
 *      bcm_fe2000_l2_stat_enable_set
 * Description:
 *      Enable/Disable statistics on the indicated L2 entry.
 * Parameters:
 *      unit    - device unit number.
 *      l2_addr - Pointer to L2 address structure.
 *      enable  - TRUE/FALSE indicator indicating action to enable/disable.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Either the L2 or the stats module is not yet initialized
 *      BCM_E_PARAM     - Illegal parameter.
 *      BCM_E_XXX       - Failure, other
 */
int bcm_fe2000_l2_stat_enable_set(int unit, bcm_l2_addr_t *l2_addr, int enable)
{
    int                rv;
    _l2_stat_fields_t *fields;
    _l2_stat_key_t  key;

    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(l2_addr);

    L2_VERB((_SBX_D(unit, "Enter enable=%d port=%d mac=" L2_6B_MAC_FMT "\n"),
             enable, l2_addr->port, L2_6B_MAC_PFMT(l2_addr->mac)));

    if (L2_STAT_STATE(unit) == NULL) {
        rv = _bcm_fe2000_l2_stat_state_init(unit);
        if (BCM_FAILURE(rv)) {
            L2_ERR((_SBX_D(unit, "Failed to initialize l2 statistics: %d %s\n"),
                    rv, bcm_errmsg(rv)));
            return rv;
        }
    }

    _bcm_fe2000_init_key(l2_addr, key);
    rv = shr_htb_find(L2_STAT_STATE(unit)->fieldTable, key,
                      (shr_htb_data_t *)&fields,
                      0 /* == don't remove */);

    L2_VVERB((_SBX_D(unit, "_find (%d," L2_6B_MAC_FMT ") rv=%d\n"),
              l2_addr->port, L2_6B_MAC_PFMT(l2_addr->mac), rv));

    if (enable) {
        _l2_stat_fields_t tmpFields;

        if (BCM_SUCCESS(rv)) {
            L2_ERR((_SBX_D(unit, "Attempted to enable existing entry\n")));
            return BCM_E_PARAM;
        }
        
        if (rv != BCM_E_NOT_FOUND) {
            L2_ERR((_SBX_D(unit, "Attempted to enable non-existent entry\n")));
            return rv;
        }

        if (!L2_STAT_STATE(unit)->groupsValid) {
            rv = _bcm_fe2000_l2_stat_group_init(unit);
            if (BCM_FAILURE(rv)) {
                L2_ERR((_SBX_D(unit, "Faield to initialize group: %d %s\n"), 
                        rv, bcm_errmsg(rv)));
                return rv;
            }
        }
        
        rv = _bcm_fe2000_l2_stat_field_entry_create(unit, l2_addr, &tmpFields);
        if (BCM_FAILURE(rv)) {
            L2_ERR((_SBX_D(unit, "Failed to create field match entries: %d %s\n"),
                    rv, bcm_errmsg(rv)));
            return rv;
        }
        
        fields = sal_alloc(sizeof(_l2_stat_fields_t), "l2_stats fields");
        if (fields == NULL) {
            L2_ERR((_SBX_D(unit, "Failed to alloc stat field memory\n")));
            return BCM_E_MEMORY;
        }
        
        sal_memcpy(fields, &tmpFields, sizeof(_l2_stat_fields_t));

        rv = shr_htb_insert(L2_STAT_STATE(unit)->fieldTable, key,
                            (shr_htb_data_t)fields);
        if (BCM_FAILURE(rv)) {
            sal_free(fields);
            L2_ERR((_SBX_D(unit, "Failed to insert fields into hash: %d %s\n"),
                    rv, bcm_errmsg(rv)));
            return rv;
        }
        
        L2_STAT_STATE(unit)->numStatsInUse++;
    } else {   /* if (enable) */

        if (rv == BCM_E_NOT_FOUND) {
            L2_ERR((_SBX_D(unit, "Attempted to disable non-existent entry\n")));
            return rv;
        }
        if (BCM_FAILURE(rv)) {
            L2_ERR((_SBX_D(unit, "Unexpected error: %d %s\n"), rv, bcm_errmsg(rv)));
            return rv;
        }

        if (!L2_STAT_STATE(unit)->groupsValid) {
            L2_ERR((_SBX_D(unit, "Invalid state - found fields, but groups not "
                       "valid\n")));
            return BCM_E_INTERNAL;  /* how did this happen? */
        }
        
        rv = _bcm_fe2000_l2_stat_field_entry_destroy(unit, fields);
        if (BCM_FAILURE(rv)) {
            L2_ERR((_SBX_D(unit, "Failed to destroy field entries, keep trying: "
                       "%d %s\n"), rv, bcm_errmsg(rv)));
        }
        
        rv = shr_htb_find(L2_STAT_STATE(unit)->fieldTable, key,
                          (shr_htb_data_t *)&fields,
                          1 /* == remove */);

        sal_free(fields);
        L2_STAT_STATE(unit)->numStatsInUse--;
    }

    if (L2_STAT_STATE(unit)->numStatsInUse == 0 &&
        L2_STAT_STATE(unit)->groupsValid) 
    {
        _bcm_fe2000_l2_stat_group_destroy(unit);
        _bcm_fe2000_l2_stat_state_destroy(unit);
    }

    return rv;
}

/*
 * Function:
 *     bcm_fe2000_l2_traverse
 * Description:
 *     To traverse the L2 table and call provided callback function
 *     with matched entry
 * Parameters:
 *     unit         device number
 *     trav_fn      User specified callback function 
 *     user_data    User specified cookie
 * Return:
 *     BCM_E_XXX
 */
int 
bcm_fe2000_l2_traverse(int unit,
                       bcm_l2_traverse_cb trav_fn,
                       void *user_data)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_fe2000_l2_replace(int unit, uint32 flags, bcm_l2_addr_t *match_addr,
                      bcm_module_t new_module, bcm_port_t new_port, 
                      bcm_trunk_t new_trunk)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      _bcm_fe2000_l2_egress_entry_add
 * Description:
 *      add an l2 egress entry with the given config at specified index
 * Parameters:
 *      unit        - device unit number.
 *      encap_id    - index at which to add the l2 egress entry 
 *      egr         - ptr to l2 egress config
 * Returns:
 *      BCM_E_NONE  - if successfully added entry
 *      BCM_E_XXX   - otherwise
 */
int
_bcm_fe2000_l2_egress_entry_add(int unit, bcm_if_t encap_id, 
                                bcm_l2_egress_t *egr)
{
    int rv = BCM_E_INTERNAL;
    
    L2_EGRESS_VALID_ENCAP_ID_CHECK(encap_id);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_l2_egress_hw_entry_add(unit, encap_id, egr);
        if ((rv == BCM_E_NONE) && (egr->flags & BCM_L2_EGRESS_DEST_PORT)) {
            L2_EGR_DEST_PORT_SET(unit, 
                                 SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(encap_id),
                                 egr->dest_port);
        }
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_CONFIG; 
    }

    return rv;
}

/*
 * Function:
 *      _bcm_fe2000_l2_egress_entry_delete
 * Description:
 *      delete the entry pointed to by the given encap_id
 * Parameters:
 *      unit        - device unit number.
 *      encap_id    - encapsulation index of the entry to delete
 * Returns:
 *      BCM_E_NONE  - if successfully deleted the specified entry
 *      BCM_E_XXX   - otherwise
 */
int
_bcm_fe2000_l2_egress_entry_delete(int unit, bcm_if_t encap_id)
{
    int elem_state;
    int rv = BCM_E_INTERNAL;

    /* perform common list management, and param check */
    L2_EGRESS_VALID_ENCAP_ID_CHECK(encap_id);

    elem_state = shr_idxres_list_elem_state(_l2_egress[unit].idList.idMgr, 
                                            SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(encap_id));
    if (elem_state != BCM_E_EXISTS) {
        return BCM_E_NOT_FOUND;
    }
    
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_l2_egress_hw_entry_delete(unit, encap_id);
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_CONFIG;
    }

    /* finalize with common list managment */
    if (BCM_SUCCESS(rv)) {
        uint32 localId = SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(encap_id);

        /* Free the element only if it is out of the reserved range */
        if (!L2_ID_RESERVED(unit, localId)) {
            rv = shr_idxres_list_free(_l2_egress[unit].idList.idMgr, localId);
        } else {
            L2_VERB((_SBX_D(unit, "encap 0x%08x found in reserved range, not freeing\n"),
                     encap_id));
        }

        L2_EGR_DEST_PORT_SET(unit, localId, L2_EGR_DEST_PORT_INVALID);
    }
    
    return rv;
}

/*
 * Function:
 *      _bcm_fe2000_l2_egress_entry_delete_all
 * Description:
 *      delete all l2 egress entries on a give unit
 * Parameters:
 *      unit        - device unit number.
 * Returns:
 *      BCM_E_NONE  - if successfully deleted all existing entries
 *      BCM_E_XXX   - otherwise
 */
int 
_bcm_fe2000_l2_egress_entry_delete_all(int unit)
{
    int rv = BCM_E_NONE;
    int idx;
    int elem_state;

    for (idx=0; idx <= _l2_egress[unit].encap_id_max; idx++) {
        elem_state = shr_idxres_list_elem_state(_l2_egress[unit].idList.idMgr, idx);
        if (elem_state == BCM_E_EXISTS) {
            rv = _bcm_fe2000_l2_egress_entry_delete(unit, 
                                        SOC_SBX_L2_ENCAP_ID_FROM_OFFSET(idx));
        }
        if (rv != BCM_E_NONE) {
            break;
        }
    }

    return rv;
}

/*
 * Function:
 *      _bcm_fe2000_l2_egress_entry_get
 * Description:
 *      read the l2 egress config give an encap id
 * Parameters:
 *      unit        - device unit number.
 *      encap_id    - encap id of the entry to read config for
 *      egr         - Ptr to entry config into which to read
 * Returns:
 *      BCM_E_NONE  - if entry found and successfully able to read config
 *      BCM_E_XXX   - otherwise
 * Notes:
 *      TODO: to add support for SMAC & vlan priority
 */
int
_bcm_fe2000_l2_egress_entry_get(int unit, bcm_if_t encap_id, 
                                bcm_l2_egress_t *egr)
{
    int                         rv = BCM_E_INTERNAL;
    int                         elem_state;
    uint32                      dest_port;

    L2_EGRESS_VALID_ENCAP_ID_CHECK(encap_id);
    PARAM_NULL_CHECK(egr);
    
    elem_state = shr_idxres_list_elem_state(_l2_egress[unit].idList.idMgr, 
                                   SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(encap_id));
    dest_port = L2_EGR_DEST_PORT_INVALID;

    if (elem_state != BCM_E_EXISTS) {
        return BCM_E_NOT_FOUND;
    }

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_l2_egress_hw_entry_get(unit, encap_id, egr);
        if (rv == BCM_E_NONE) {
            L2_EGR_DEST_PORT_GET(unit, 
                                 SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(encap_id),
                                 dest_port);
            if (dest_port != L2_EGR_DEST_PORT_INVALID) {
                egr->dest_port = dest_port;
                egr->flags |= BCM_L2_EGRESS_DEST_PORT;
            }
        }
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_CONFIG;
    }

    return rv;
}

/*
 * Function:
 *      _bcm_fe2000_l2_egress_entries_compare
 * Description:
 *      Compare if two l2 egress entries are same
 * Parameters:
 *      egr1 - Ptr of first entry config
 *      egr2 - Ptr of second entry config
 * Returns:
 *      0           - if both entries are same
 *      non-zero    - otherwise
 * Notes:
 *      Ignores BCM_L2_EGRESS_WITH_ID & BCM_L2_EGRESS_REPLACE config flags
 *      and encap_ids
 */
int
_bcm_fe2000_l2_egress_entries_compare(bcm_l2_egress_t *pegr1, 
                                      bcm_l2_egress_t *pegr2)
{
    bcm_l2_egress_t egr1, egr2;
    uint32 config_mask = ~(BCM_L2_EGRESS_WITH_ID | BCM_L2_EGRESS_REPLACE);

    if ((!pegr1) || (!pegr2)) {
        return BCM_E_PARAM;
    }

    sal_memcpy(&egr1, pegr1, sizeof(bcm_l2_egress_t));
    sal_memcpy(&egr2, pegr2, sizeof(bcm_l2_egress_t));

    /* ignore WITH_ID & REPLACE flags */
    egr1.flags &= config_mask;
    egr2.flags &= config_mask;

    /* ignore the encap_ids */
    egr1.encap_id = egr2.encap_id = 0;   

    return sal_memcmp(&egr1, &egr2, sizeof(bcm_l2_egress_t));
}

/*
 * Function:
 *      bcm_fe2000_l2_egress_create
 * Description:
 *      Create an encapsulation index with the give l2 egress config
 * Parameters:
 *      unit        - device unit number.
 *      egr         - Ptr of entry config
 * Returns:
 *      BCM_E_NONE      - if entry successfully created
 *      BCM_E_XXX       - Failure, other
 * Notes:
 *      If call is successful, created encapsulation index is stored in 
 *      egr->encap_id
 */
int
bcm_fe2000_l2_egress_create(int unit, bcm_l2_egress_t *egr)
{
    int         rv = BCM_E_NONE;
    int         elem_state;
    bcm_if_t    encap_id = 0;
    uint32    localId;

    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(egr);
    
    /* if BCM_L2_EGRESS_REPLACE is set, then BCM_L2_EGRESS_WITH_ID 
    should also be set */
    if ((egr->flags & BCM_L2_EGRESS_REPLACE) && 
        !(egr->flags & BCM_L2_EGRESS_WITH_ID)) {
        return BCM_E_PARAM; 
    }

    /* setting both is invalid */
    if ((egr->flags & BCM_L2_EGRESS_DEST_MAC_PREFIX5_REPLACE) &&
        (egr->flags & BCM_L2_EGRESS_DEST_MAC_REPLACE)) {
        return BCM_E_PARAM;
    }
    
    localId = SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(egr->encap_id);

    L2_LOCK(unit);
    if (egr->flags & BCM_L2_EGRESS_WITH_ID) {
        if (L2_EGRESS_VALID_ENCAP_ID(egr->encap_id)) {
            elem_state = shr_idxres_list_elem_state(_l2_egress[unit].idList.idMgr,
                                SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(egr->encap_id));
            if (egr->flags & BCM_L2_EGRESS_REPLACE) {
                /* trying to replace, check if it exists */
                if (elem_state != BCM_E_EXISTS) {
                    rv = BCM_E_PARAM;
                } else {
                    /* delete the existing entry */
                    rv = _bcm_fe2000_l2_egress_entry_delete(unit, 
                                                            egr->encap_id);
                    if (rv == BCM_E_NONE) {
                        rv = _bcm_fe2000_l2_egress_entry_add(unit, 
                                                             egr->encap_id, 
                                                             egr);
                    }

                    /* reserve the id only if it's not in the application
                     * reserved range 
                     */
                    if (rv == BCM_E_NONE && !L2_ID_RESERVED(unit, localId)) {
                        rv = shr_idxres_list_reserve(_l2_egress[unit].idList.idMgr,
                                                     localId, localId);
                    } else {
                        L2_VERB((_SBX_D(unit, "encap 0x%08x found in reserved range,"
                                    " not re-allocated, flags=0x%x - OK\n"),
                                 egr->encap_id, egr->flags));
                    }
                    
                }
            } else {

                if (L2_ID_RESERVED(unit, localId)) {
                    L2_VERB((_SBX_D(unit, "found encap 0x%x in reserved range, state=%d"
                                " %s; OK\n"),  
                             egr->encap_id, elem_state, 
                             bcm_errmsg(elem_state)));
                    elem_state = BCM_E_NONE;
                } 

                /* User specified encap id & no replace. error if it exists */
                if (elem_state == BCM_E_EXISTS) {
                    rv = BCM_E_EXISTS;
                } else {
                    rv = _bcm_fe2000_l2_egress_entry_add(unit, egr->encap_id, 
                                                         egr);

                    /* don't reserve it if it's in the application 
                     * reserved space 
                     */
                    if (rv == BCM_E_NONE && !(L2_ID_RESERVED(unit, localId))) {
                        rv = shr_idxres_list_reserve(_l2_egress[unit].idList.idMgr,
                                                     localId, localId);
                    } else {
                        L2_VERB((_SBX_D(unit, "encap 0x%08x found in reserved range,"
                                    " not reallocated - flags=0x%x\n"), 
                                 egr->encap_id, egr->flags));

                    }
                }
            }
        } else {
            rv = BCM_E_PARAM;
        }
    } else {
        /* allocate an encap id and create the entry */
        rv = shr_idxres_list_alloc(_l2_egress[unit].idList.idMgr, 
                                   (uint32 *)&encap_id);
        if (rv == BCM_E_NONE) {
            encap_id = SOC_SBX_L2_ENCAP_ID_FROM_OFFSET(encap_id);
            if (L2_EGRESS_VALID_ENCAP_ID(encap_id)) {
                rv = _bcm_fe2000_l2_egress_entry_add(unit, encap_id, egr);
                if (rv != BCM_E_NONE) {
                    shr_idxres_list_free(_l2_egress[unit].idList.idMgr, 
                                  SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(encap_id));
                }
            } else {
                rv = BCM_E_PARAM;
            }
        }
    }
    L2_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_fe2000_l2_egress_destroy
 * Description:
 *      Find an encapsulation index for a given configuration
 * Parameters:
 *      unit        - device unit number.
 *      encap_id    - encapsulation index of the entry to destroy
 * Returns:
 *      BCM_E_NONE      - if entry successfully deleted
 *      BCM_E_XXX       - Failure, other
 */
int 
bcm_fe2000_l2_egress_destroy(int unit, bcm_if_t encap_id)
{
    int rv = BCM_E_NONE;
    
    UNIT_INIT_CHECK(unit);
    
    L2_LOCK(unit);
    rv = _bcm_fe2000_l2_egress_entry_delete(unit, encap_id);
    L2_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      bcm_fe2000_l2_egress_get
 * Description:
 *      Find an encapsulation index for a given configuration
 * Parameters:
 *      unit        - device unit number.
 *      encap_id    - encapsulation index of the entry to retrieve config for
 *      egr         - Ptr of where to store the l2 egress config
 * Returns:
 *      BCM_E_NONE      - if entry found & able to read the entry successfully
 *      BCM_E_XXX       - Failure, other
 */
int 
bcm_fe2000_l2_egress_get(int unit, bcm_if_t encap_id, bcm_l2_egress_t *egr)
{
    int         rv = BCM_E_NONE;
    int         elem_state;

    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(egr);
    L2_EGRESS_VALID_ENCAP_ID_CHECK(encap_id);

    L2_LOCK(unit);
    if (rv == BCM_E_NONE) {
        elem_state = shr_idxres_list_elem_state(_l2_egress[unit].idList.idMgr,
                                  SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(encap_id));
        if (elem_state == BCM_E_EXISTS) {
            rv = _bcm_fe2000_l2_egress_entry_get(unit, encap_id, egr);
        } else {
            rv = BCM_E_NOT_FOUND;
        }
    }
    L2_UNLOCK(unit);
    
    return rv;
}

/*
 * Function:
 *      bcm_l2_egress_find
 * Description:
 *      Find an encapsulation index for a given configuration
 * Parameters:
 *      unit        - device unit number.
 *      egr         - configuration of l2 egress to look for
 *      pencap_id   - Ptr of where to store the encap_id
 * Returns:
 *      BCM_E_NONE      - if entry found.
 *      BCM_E_XXX       - Failure, other
 */
int 
bcm_fe2000_l2_egress_find(int unit, bcm_l2_egress_t *egr, bcm_if_t *pencap_id)
{
    int rv = BCM_E_NONE;
    int elem_state;
    int idx;
    bcm_l2_egress_t temp_egr;
    
    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(egr);
    PARAM_NULL_CHECK(pencap_id);

    L2_LOCK(unit);
    for (idx = 0; idx <= _l2_egress[unit].encap_id_max; idx++) {
        elem_state = shr_idxres_list_elem_state(_l2_egress[unit].idList.idMgr, idx);
        if (elem_state == BCM_E_EXISTS) {
            rv = _bcm_fe2000_l2_egress_entry_get(unit, 
                                                 SOC_SBX_L2_ENCAP_ID_FROM_OFFSET(idx),
                                                 &temp_egr);
            if (rv != BCM_E_NONE) {
                break;
            }
            if (!_bcm_fe2000_l2_egress_entries_compare(egr, &temp_egr)) {
                /* found the entry */
                *pencap_id = SOC_SBX_L2_ENCAP_ID_FROM_OFFSET(idx);
                break;
            }
        }
    }
    if (idx > _l2_egress[unit].encap_id_max) {
        rv = BCM_E_NOT_FOUND;
    }
    L2_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      bcm_l2_egress_traverse
 * Description:
 *      Traverse the list of existing l2 egress entries and call user provided
 *      callback with user_data
 * Parameters:
 *      unit        - device unit number.
 *      trav_fn     - user provided callback fn
 *      user_data   - user cookie
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_XXX       - Failure, other
 * Notes:
 *      Stops if the callback fn returns anything other than BCM_E_NONE
 */
int bcm_fe2000_l2_egress_traverse(int unit, bcm_l2_egress_traverse_cb trav_fn, 
                           void *user_data)
{
    int                 rv = BCM_E_NONE;
    int                 idx;
    int                 elem_state;
    bcm_l2_egress_t     egr;

    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(trav_fn);

    L2_LOCK(unit);
    for (idx=0; idx <= _l2_egress[unit].encap_id_max; idx++) {
        elem_state = shr_idxres_list_elem_state(_l2_egress[unit].idList.idMgr, idx);
        if (elem_state == BCM_E_EXISTS) {
            rv = _bcm_fe2000_l2_egress_entry_get(unit, 
                                                 SOC_SBX_L2_ENCAP_ID_FROM_OFFSET(idx),
                                                 &egr);
            if (rv != BCM_E_NONE) {
                break;
            }
            rv = (*trav_fn)(unit, &egr, user_data);
            if (rv != BCM_E_NONE) {
                break;
            }
        }
    }
    L2_UNLOCK(unit);

    return rv;
}


/*
 * Function:
 *     _bcm_l2_addr_update_dest
 * Purpose:
 *     Changes FTE of existing DMAC entry by specifying a new qidunion (basequeue).
 *     Code below will allocate new FTE  and point this l2_addr to it.
 * Parameters:
 *     unit   - Device number
 *     l2addr - Pointer to bcm_l2_addr_t containing all valid fields
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     This is an internal function and should not be used. Only works on Unicast.
 *     Assumes that caller will de-allocate the FTE which is allocated here
 */
int
_bcm_fe2000_l2_addr_update_dest(int unit, bcm_l2_addr_t *l2addr, int qidunion)
{
    int rv = BCM_E_UNAVAIL;
    
    /* Check params and get device handler */
    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(l2addr);

    /* Check for valid flags */
    if (l2addr->flags & L2_FLAGS_UNSUPPORTED) {
        return BCM_E_PARAM;
    }

    L2_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_l2_addr_update_dest(unit, l2addr, qidunion);
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_CONFIG;
    }

    L2_UNLOCK(unit);
    return rv;
}

#ifdef BCM_WARM_BOOT_SUPPORT
/*
 * Function:
 *      _bcm_fe2000_l2_egress_recover
 * Description:
 *      Recover the L2 egress for the specified unit
 * Parameters:
 *      unit    - device unit number.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_XXX       - Failure, other
 * Notes:
 */
int
_bcm_fe2000_l2_egress_recover(int unit)
{
    int             rv = BCM_E_NONE;
    int             i;
    uint32_t        localId, encapId=L2_EGR_DEST_PORT_INVALID;

    if (L2_WB_L2_CONFIGURED(unit) == FALSE) {
        return BCM_E_INTERNAL;
    }
    
    /* Reallocated OHIs as needed */
    for (i=0; i < SBX_MAX_L2_EGRESS_OHI; i++) {
        L2_EGR_DEST_PORT_GET(unit, i, encapId);
        if (encapId != L2_EGR_DEST_PORT_INVALID) {
            localId = SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(encapId);
            /* reserve the id only if it's not in the application reserved range */
            if (!L2_ID_RESERVED(unit, localId)) {
                rv = shr_idxres_list_reserve(_l2_egress[unit].idList.idMgr,
                                             localId, localId);
                if (rv != BCM_E_NONE) {
                    /* non-fatal error during warmboot */
                    L2_WARN((_SBX_D(unit, "L2 egress failed to re-allocate encap id: 0x%x, localId: 0x%x\n"), encapId, localId));
                }
            }else{
                L2_VERB((_SBX_D(unit, "encap 0x%08x (locaId 0x%x) found in reserved range,"
                            " not re-allocated - OK\n"),
                         encapId, localId));
            }
        }
    }

    return rv;
}
#endif /* BCM_WARM_BOOT_SUPPORT */
/*
 * Function:
 *      _bcm_fe2000_l2_egress_init
 * Description:
 *      Initialize the L2 egress for the specified unit
 * Parameters:
 *      unit    - device unit number.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_XXX       - Failure, other
 * Notes:
 *      Re-init clears all existing l2 egress entries.
 */

int
_bcm_fe2000_l2_egress_init(unit)
{
    int             rv = BCM_E_NONE;
    int             l2_egr_num;
#ifdef BCM_WARM_BOOT_SUPPORT
    uint32        wb_hdl, size, scache_size;
#endif

    /* number of supported l2 egr objs */
    l2_egr_num = SBX_MAX_L2_EGRESS_OHI;

    UNIT_INIT_CHECK(unit);
    L2_LOCK(unit);

#ifdef BCM_WARM_BOOT_SUPPORT
    L2_WB_L2_HANDLE_SET(unit, wb_hdl);
    /* Is Level 2 recovery even available? */
    rv = soc_stable_size_get(unit, (int*)&size);
    if (BCM_FAILURE(rv)) {
        return rv; 
    }
#endif
    /* if re-init clear all existing entries */
    if ( (_l2_egress[unit].init_done) && (!SOC_WARM_BOOT(unit)) ){
        if ((rv = _bcm_fe2000_l2_egress_entry_delete_all(unit)) != BCM_E_NONE){
            L2_ERR((_SBX_D(unit, "Re-init on failed (%s) \n"), bcm_errmsg(rv)));
        }
    } else if (!SOC_WARM_BOOT(unit)){
        /* first time init */
        _l2_egress[unit].idList.reservedHigh = L2_INVALID_EGRESS_ID;
        _l2_egress[unit].idList.reservedLow  = L2_INVALID_EGRESS_ID;
        
#ifdef BCM_WARM_BOOT_SUPPORT
        /* on cold boot, setup scache */
        if (size > 0) {
            scache_size = l2_egr_num * sizeof(uint32) + SOC_WB_SCACHE_CONTROL_SIZE;
            L2_VVERB(("L2: allocating 0x%x (%d) bytes of scache:",  scache_size, scache_size));
            rv = soc_scache_alloc(unit, wb_hdl, scache_size);
        }
#else
        _l2_egress[unit].dest_ports = sal_alloc((sizeof(uint32) * 
                                                 l2_egr_num),
                                                "L2 egr Dest Port");
        if (_l2_egress[unit].dest_ports == NULL) {
            L2_ERR((_SBX_D(unit, "Allocating L2 Egress dest port storage "
                           "failed\n")));
            rv = BCM_E_MEMORY;
        }

        L2_WB_LOCK_CACHE(unit);
        sal_memset(_l2_egress[unit].dest_ports, 0xff, 
                   sizeof(uint32) * l2_egr_num);
        L2_WB_UNLOCK_CACHE(unit);

#endif
    }

    rv = shr_idxres_list_create(& _l2_egress[unit].idList.idMgr, 0, 
                                (l2_egr_num - 1), 0, (l2_egr_num - 1), 
                                "l2 egress entries");
    if (BCM_FAILURE(rv)) {
        L2_ERR((_SBX_D(unit, "Failed to create id manager:%s\n"),
                bcm_errmsg(rv)));
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    if (BCM_SUCCESS(rv)) {
        /* Get the pointer for the Level 2 cache */
        _l2_egress[unit].wb_cache = NULL;
        _l2_egress[unit].scache_size = 0;
        if (size > 0) {
            rv = soc_scache_ptr_get(unit, wb_hdl,
                                        (uint8**)&_l2_egress[unit].wb_cache, 
                                        &_l2_egress[unit].scache_size);

            /* adjust the scache size for the control word */
            _l2_egress[unit].scache_size = SOC_WB_SCACHE_SIZE(_l2_egress[unit].scache_size);
        }

        /* if supporting warmboot, use scache */
        _l2_egress[unit].dest_ports = (uint32*)_l2_egress[unit].wb_cache->cache;

        if (!BCM_FAILURE(rv)) {
            if (_l2_egress[unit].wb_cache) {
                if (!SOC_WARM_BOOT(unit)) {
                    _l2_egress[unit].wb_cache->version = L2_WB_CURRENT_VERSION;
                }

                L2_VERB((_SBX_D(unit, "Obtained scache pointer=0x%08x, %d bytes, "
                                "version=%d.%d\n"),
                         (int)_l2_egress[unit].wb_cache->cache, _l2_egress[unit].scache_size,
                         SOC_SCACHE_VERSION_MAJOR(_l2_egress[unit].wb_cache->version),
                         SOC_SCACHE_VERSION_MINOR(_l2_egress[unit].wb_cache->version)));

                if (_l2_egress[unit].wb_cache->version > L2_WB_CURRENT_VERSION) {
                    L2_ERR((_SBX_D(unit, "Upgrade scenario not supported.  "
                                   "Current version=%d.%d  found %d.%d\n"),
                            SOC_SCACHE_VERSION_MAJOR(L2_WB_CURRENT_VERSION),
                            SOC_SCACHE_VERSION_MINOR(L2_WB_CURRENT_VERSION),
                            SOC_SCACHE_VERSION_MAJOR(_l2_egress[unit].wb_cache->version),
                            SOC_SCACHE_VERSION_MINOR(_l2_egress[unit].wb_cache->version)));
                    rv = BCM_E_CONFIG;

                } else if (_l2_egress[unit].wb_cache->version < L2_WB_CURRENT_VERSION) {
                    L2_ERR((_SBX_D(unit, "Downgrade scenario not supported.  "
                                   "Current version=%d.%d  found %d.%d\n"),
                            SOC_SCACHE_VERSION_MAJOR(L2_WB_CURRENT_VERSION),
                            SOC_SCACHE_VERSION_MINOR(L2_WB_CURRENT_VERSION),
                            SOC_SCACHE_VERSION_MAJOR(_l2_egress[unit].wb_cache->version),
                            SOC_SCACHE_VERSION_MINOR(_l2_egress[unit].wb_cache->version)));
                    rv = BCM_E_CONFIG;
                }

            }

        }
    }

    /* if warmboot, perform recovery */
    if (SOC_WARM_BOOT(unit)) {
        rv = _bcm_fe2000_l2_egress_recover(unit);
    }
#endif
    
    if (rv == BCM_E_NONE) {
        _l2_egress[unit].init_done = 1;
        _l2_egress[unit].encap_id_max = (l2_egr_num -1);
    } else {

        /* free resources */
        rv = shr_idxres_list_destroy(_l2_egress[unit].idList.idMgr);
#ifdef BCM_WARM_BOOT_SUPPORT
#else
        if (_l2_egress[unit].dest_ports) {
            sal_free(_l2_egress[unit].dest_ports);
        }
#endif
    }

    L2_UNLOCK(unit);
    return rv;
}

/*
 *   Function
 *      bcm_fe2000_l2_egress_range_reserve
 *   Purpose
 *     Reserve a range of L2 egress IDs
 *
 *   Parameters
 *      (IN)  unit   : unit number of the device
 *      (IN) type    : resource to set
 *      (IN) highOrLow  : TRUE - set Upper bounds
 *                      : FALSE - set lower bounds
 *      (IN) val    : inclusive bound to set
 *   Returns
 *       BCM_E_NONE - All required resources are allocated
 *       BCM_E_*    - failure
 *   Notes
 */
int
bcm_fe2000_l2_egress_range_reserve(int unit, int highOrLow, uint32 val)
{

    int rv = BCM_E_NONE;
    int clearIt = 0, reserveIt = 0;
    uint32 first, last;

    UNIT_INIT_CHECK(unit);
    L2_EGRESS_VALID_ENCAP_ID_CHECK(val);


    L2_LOCK(unit);
    first = _l2_egress[unit].idList.reservedLow;
    last = _l2_egress[unit].idList.reservedHigh;

    /* Zero for any value, high or low, will clear the known range */
    if (val == 0) {
        clearIt = 1;
        _l2_egress[unit].idList.reservedLow = L2_INVALID_EGRESS_ID;
        _l2_egress[unit].idList.reservedHigh = L2_INVALID_EGRESS_ID;
    } else {
        val = SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(val);

        if (highOrLow) {
            _l2_egress[unit].idList.reservedHigh = val; 
        } else {
            _l2_egress[unit].idList.reservedLow = val;
        }

        if ((_l2_egress[unit].idList.reservedHigh != L2_INVALID_EGRESS_ID) && 
            (_l2_egress[unit].idList.reservedLow != L2_INVALID_EGRESS_ID)) 
        {
            if (_l2_egress[unit].idList.reservedHigh < 
                _l2_egress[unit].idList.reservedLow) 
            {
                L2_ERR((_SBX_D(unit, "Upper bounds is set less than lower"
                               " bounds: 0x%x < 0x%x\n"),
                        _l2_egress[unit].idList.reservedHigh, 
                        _l2_egress[unit].idList.reservedLow));

                rv = BCM_E_PARAM;
            } else {
                reserveIt = 1;
            }
        }
    }

    if (reserveIt) {
        
        first = _l2_egress[unit].idList.reservedLow;
        last = _l2_egress[unit].idList.reservedHigh;

        rv = shr_idxres_list_reserve(_l2_egress[unit].idList.idMgr, 
                                     first, last);
        
        L2_VERB((_SBX_D(unit, "Reserved egress ids: 0x%08x-0x%08x rv=%d %s\n"),
                 first, last, rv, bcm_errmsg(rv)));

    } else if (clearIt) {

        if (first && last) {
            int elt, ignoreRv;

            for (elt = first; elt <= last; elt++) {
                ignoreRv = shr_idxres_list_free(_l2_egress[unit].idList.idMgr,
                                                elt);

                if (BCM_FAILURE(ignoreRv)) {
                    L2_VERB((_SBX_D(unit, "failed to free element 0x%08x  rv=%d %s "
                                "(ignored)\n"),
                             elt, ignoreRv, bcm_errmsg(ignoreRv)));
                } 
            }
            L2_VERB((_SBX_D(unit, "Freed reserved l2 egress ids: 0x%08x-0x%08x\n"),
                       first, last));
        }
    }

    L2_UNLOCK(unit);

    return rv;

}

/*
 *   Function
 *      bcm_fe2000_l2_egress_range_get
 *   Purpose
 *     Retrieve the range of valid L2 egress IDs
 *
 *   Parameters
 *      (IN)  unit   : unit number of the device
 *      (OUT) first  : first valid ID
 *      (OUT) last   : last valid ID
 *   Returns
 *       BCM_E_NONE - All required resources are allocated
 *       BCM_E_*    - failure
 *   Notes
 */
int bcm_fe2000_l2_egress_range_get(int unit, uint32 *low, uint32 *high)
{
    int rv = BCM_E_NONE;
    uint32 validLow, validHigh, freeCount, allocCount, first, last;

    UNIT_INIT_CHECK(unit);
    L2_LOCK(unit);

    rv = shr_idxres_list_state(_l2_egress[unit].idList.idMgr, &first, &last, 
                               &validLow, &validHigh, &freeCount, &allocCount);

    L2_UNLOCK(unit);

    *low = SOC_SBX_L2_ENCAP_ID_FROM_OFFSET(first);
    *high = SOC_SBX_L2_ENCAP_ID_FROM_OFFSET(last);
    L2_VERB((_SBX_D(unit, "first=0x%x last=0x%x low=0x%08x high=0x%08x allocated=%d rv=%d\n"),
                first, last, *low, *high, allocCount, rv));

    return rv;
}
