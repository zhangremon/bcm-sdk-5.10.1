/*
* $Id: field.c 1.81.20.1 Broadcom SDK $
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
 * Module: Field Processor APIs
 *
 * Purpose:
 *     Implement 'Field Processor' (FP) API for BCM88200 (SBX FE-2000 +
 *     Guadalupe-2000) G2P3 forwarder microcode and classifier picocode (plus
 *     other undocumented arcana).
 */

#ifdef BCM_FE2000_P3_SUPPORT
#define _SBX_FE2K_FIELD_H_NEEDED_ TRUE

#include <bcm/types.h>
#include <bcm/debug.h>
#include <bcm/error.h>
#include <bcm/field.h>
#include <bcm/vlan.h>
#include <bcm/stack.h>
#include <bcm/mirror.h>

#include <shared/bitop.h>
#include <shared/idxres_fl.h>

#include <soc/sbx/sbTypes.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/g2p3/g2p3.h>

#include <bcm_int/sbx/fe2000/allocator.h>
#include <bcm_int/sbx/fe2000/mirror.h>
#include <bcm_int/sbx/fe2000/field.h>
#include <bcm_int/sbx/fe2000/policer.h>
#include <bcm_int/sbx/fe2000/vlan.h>
#include <bcm_int/sbx/fe2000/l3.h>
#include <bcm_int/sbx/mcast.h>
#ifdef BCM_WARM_BOOT_SUPPORT
#include <bcm/module.h>
#include <bcm/cosq.h>
#include <bcm_int/sbx/stack.h>
#endif

/* we need to know how many stages at compile time */

#define _FIELD_FE2K_G2P3_STAGES SB_G2P3_CLS_MAX_STAGES_SUPPORTED

/*
 *  Only set this TRUE if you really want a lot of output from the field_show
 *  and field_dump functions.  This adds a dump of any associated lists.  It
 *  should probably be left FALSE unless debugging these lists.  Note this AND
 *  _SBX_FE2K_FIELD_DIAGNOSTIC_MODE must be TRUE to get the lists.
 */
#define _FIELD_FE2K_G2P3_DUMP_LISTS FALSE

/*
 *  Set this TRUE if you want all rules that apply to an entry to be dumped;
 *  set it FALSE if you're happy only seeing the first rule that applies to an
 *  entry dumped with the other rules only displaying rule ID and counters.
 */
#define _FIELD_FE2K_G2P3_DUMP_ALL_RULES TRUE

#define _FIELD_FE2K_G2P3_MAX_GROUPS SB_MAX_USER_GROUPS




/*****************************************************************************
 *
 *  Internal implementation
 *
 */

/*
 *  Miserably, somebody decided that there would be separate structs and types
 *  and functions for ingress and egress in the SBX world this time.  This
 *  makes it either seriously redundant to manipulate the rules with separate
 *  sets of functions at this layer or seriously dangerous to manipulate the
 *  rules with a single set of functions at this layer.
 *
 *  Not much can be done about that from up here, though.  It's going to cost
 *  both code and data size, as well as performance, no matter which way, so
 *  let's just go with one set of functions.
 */

/*
 *  This is a union of the ingress and egress rule, so we don't need to have
 *  two separate functions for everything to do with entries.
 */
typedef union _bcm_fe2k_g2p3_hardware_rule_u {
    soc_sbx_g2p3_ifp_t ingress;
    soc_sbx_g2p3_efp_t egress;
    soc_sbx_g2p3_ifp_v6_t ingress_v6;
    soc_sbx_g2p3_efp_v6_t egress_v6;
} _bcm_fe2k_g2p3_hardware_rule_t;

/*
 *  Flags to describe our internal view of an entry.
 */
typedef enum _bcm_fe2k_g2p3_fld_entry_flags_e {
    _FE2K_G2P3_ENTRY_VALID    = 0x00000001, /* entry is in use */
    _FE2K_G2P3_ENTRY_WANT_HW  = 0x00000002, /* entry wants to be in hardware */
    _FE2K_G2P3_ENTRY_IN_HW    = 0x00000004, /* entry is in hardware */
    _FE2K_G2P3_ENTRY_COUNTER  = 0x00000010, /* entry has a counter */
    _FE2K_G2P3_ENTRY_CTR_ENA  = 0x00000020, /* entry counter is enabled */
    _FE2K_G2P3_ENTRY_ACTIONS  = 0x000FFF20, /* entry actions mask */
    _FE2K_G2P3_ENTRY_ACT_RES  = 0x00041000, /*  actions with resources */
    _FE2K_G2P3_ENTRY_ACT_FTVL = 0x00001E00, /*  actions using FT/VLAN ID */
    _FE2K_G2P3_ENTRY_DROP     = 0x00000100, /*  entry has Drop action */
    _FE2K_G2P3_ENTRY_VLANNEW  = 0x00000200, /*  entry has VlanNew action */
    _FE2K_G2P3_ENTRY_L3SWITCH = 0x00000400, /*  entry has L3Switch action */
    _FE2K_G2P3_ENTRY_REDIRECT = 0x00000800, /*  entry has redirect action */
    _FE2K_G2P3_ENTRY_REDIRMC  = 0x00001000, /*  entry has redirectMcast act */
    _FE2K_G2P3_ENTRY_COSQNEW  = 0x00010000, /*  entry has CosQNew action */
    _FE2K_G2P3_ENTRY_DROPPREC = 0x00020000, /*  entry has DropPrecedence act */
    _FE2K_G2P3_ENTRY_MIRROR   = 0x00040000, /*  entry has Mirror* action */
    _FE2K_G2P3_ENTRY_CPYTOCPU = 0x00080000, /*  entry has CopyToCpu action */
    _FE2K_G2P3_ENTRY_POLFLAGS = 0xE0000000, /* policer flags mask */
    _FE2K_G2P3_ENTRY_TYPEPOL  = 0x20000000, /*  policer is typed */
    _FE2K_G2P3_ENTRY_MEFCOS   = 0x40000000, /*  polcier is MEF by CoS */
    _FE2K_G2P3_ENTRY_MEF      = 0x80000000  /*  policer is MEF */
} _bcm_fe2k_g2p3_fld_entry_flags_t;

/*
 *  This structure describes our internal view of an entry.
 *
 *  Entries are collected into a sorted linked list per group, plus a free
 *  list.  Each group's entries are sorted by priority, but multiple entries
 *  are allowed to have the same priority (any inserts of priority x are
 *  performed after any other priority x or higher entries in the list).  The
 *  priority rules are not simple; see the _bcm_fe2000_compare_entry_priority
 *  function for an explanation.  The free list is, like the group free list,
 *  an unsorted stack with the entries blanked before being placed into it.
 *
 *  The SBX description of the 'field' rule is huge.  So much so that including
 *  the rule data for each entry in the module's index table will increase it's
 *  already heavy memory consumption by a factor in excess of four. Because of
 *  this, we store all of the rule data in the SBX API, without replicating it
 *  here except as transient data on the stack.  While this adds quite a bit to
 *  the stack depth and processing time, the argument was that rule updates
 *  should not be happening too quickly and that static and heap data should be
 *  kept minimal.
 *
 *  The next/prev entry (rulebase) fields are to speed searches when doing
 *  inserts to the hardware -- need to find the previous SBX handle for the
 *  rulebase so we can insert 'after' it for ordering; need next so
 *  we can do O(1) insert/delete from this list.
 */
typedef struct _bcm_fe2k_g2p3_field_entry_data_tgt_s {
    bcm_module_t module;                            /* target module */
    bcm_port_t port;                                /* target port */
} _bcm_fe2k_g2p3_field_entry_data_tgt_t;
typedef union _bcm_fe2k_g2p3_field_entry_data_u {
    _bcm_fe2k_g2p3_field_entry_data_tgt_t target;   /* target module/port */
    uint32 ftHandle;                                /* FT entry handle */
    uint32 mcHandle;                                /* MC group handle */
    bcm_vlan_t VSI;                                 /* VSI */
} _bcm_fe2k_g2p3_field_entry_data_t;
#define _FIELD_FE2K_G2P3_RANGES_PER_ENTRY 2
typedef struct _bcm_fe2k_g2p3_field_entry_s {
    void *hwRule;                                   /* hardware rule */
    int16 priority;                                 /* priority */
    _bcm_fe2k_g2p3_field_entry_data_t ftvlData;     /* FT / VLAN data */
    bcm_policer_t policer;                          /* policer */
    bcm_field_range_t range[_FIELD_FE2K_G2P3_RANGES_PER_ENTRY]; /* ranges */
    _bcm_fe2k_g2p3_fld_entry_flags_t entryFlags;    /* internal flags */
    _bcm_fe2k_field_group_index group;              /* group */
    _bcm_fe2k_field_entry_index prevEntry;          /* prev entry (group) */
    _bcm_fe2k_field_entry_index nextEntry;          /* next entry (group) */
    _bcm_fe2k_field_entry_index prevEntryRb;        /* next entry (rulebase) */
    _bcm_fe2k_field_entry_index nextEntryRb;        /* next entry (rulebase) */
    uint16 counterMode;                             /* counter mode */
} _bcm_fe2k_g2p3_field_entry_t;

/*
 *  This structure describes our internal view of a group.  Note this has a lot
 *  of information that is BCM specific state instead of anything useful for
 *  the SBX implementation.
 *
 *  Groups are arranged into lists for each stage, plus a free list.  The stage
 *  lists are strict priority sort (highest numeric priority first and lowest
 *  numeric priority last).  The free list is more of an unsorted stack (but it
 *  doesn't matter because entries in the free list are zeroed first).
 *
 *  Each group can contain a list of entries.
 *
 *  The counters field indicates the *potential* number of counters, and will
 *  be equal to the number of entries unless some entries are sharing counters,
 *  in which case it will be less.
 *
 *  Here, rulebase is the rulebase index from the unit data; if the group is
 *  not valid, the rulebase will be _FIELD_FE2K_INVALID_RULEBASE, unless the
 *  group is in the process of being allocated, then it's ...TEMP_RULEBASE.
 */
typedef struct _bcm_fe2k_g2p3_field_group_s {
    bcm_field_qset_t qset;                        /* qualifier set bitmap */
    int priority;                                 /* priority */
    _bcm_fe2k_field_count entries;                /* number of entries */
    _bcm_fe2k_field_count counters;               /* number of counters */
    _bcm_fe2k_field_entry_index entryHead;        /* first entry ID */
    _bcm_fe2k_field_entry_index entryTail;        /* last entry ID */
    _bcm_fe2k_field_group_index nextGroup;        /* next group ID */
    _bcm_fe2k_field_group_index prevGroup;        /* previous group ID */
    uint8 rulebase;                               /* which rulebase */
} _bcm_fe2k_g2p3_field_group_t;
#define _FIELD_FE2K_INVALID_RULEBASE 0xFF         /* not allocated */
#define _FIELD_FE2K_TEMP_RULEBASE 0xFE            /* allocated but no rb */


/*
 *  Our internal view of a rulebase.
 *
 *  The entryHead field is so we can more quickly find entry information when
 *  searching for placement within the rulebase.
 */
typedef struct _bcm_fe2k_g2p3_field_rulebase_s {
    bcm_pbmp_t ports;                                        /* pbmp */
    bcm_field_qset_t qset;                                   /* qualifiers */
    SHR_BITDCL action[_SHR_BITDCLSIZE(bcmFieldActionCount)]; /* actions */
    _bcm_fe2k_field_count rules;                             /* curr rules */
    _bcm_fe2k_field_count rulesMax;                          /* max rules */
    _bcm_fe2k_field_count entries;                           /* curr entries */
    _bcm_fe2k_field_count entriesMax;                        /* max entries */
    _bcm_fe2k_field_entry_index entryHead;                   /* first entry */
    _bcm_fe2k_field_group_index groupHead;                   /* first group */
    uint8 dbId;                                              /* SBX db ID */
} _bcm_fe2k_g2p3_field_rulebase_t;

/*
 *  This is the global struct per unit; it has local data and pointers to the
 *  arrays that are allocated on init to store the above structures.
 *
 *  We don't use the indexed resource freelist handler here for groups and
 *  entries because they already have to be linked into lists while in use, so
 *  building a linked list of free elements here effectively is cheaper in
 *  terms of both processor and memory than using the indexed resource freelist
 *  handler to implement the list.
 *
 *  We do use the indexed resource freelist handler for the ranges since they
 *  don't have to be linked into lists and therefore the indexed resource
 *  freelist structure is smaller than expanding ranges to be linkable, and
 *  safer than reusing some other field as linkage.
 *
 *  Note that the BCM 'stage' concept maps directly to the SBX 'rulebase'.
 */
typedef struct _bcm_fe2k_g2p3_field_glob_s {
    _bcm_fe2k_g2p3_field_rulebase_t rulebase[_FIELD_FE2K_G2P3_STAGES];/* rbs */
    _bcm_fe2k_g2p3_field_group_t *group;            /* ptr to group array */
    _bcm_fe2k_g2p3_field_entry_t *entry;            /* ptr to entries array */
    _bcm_fe2k_field_range_t *range;                 /* ptr to ranges array */
    shr_idxres_list_handle_t rangeFree;             /* range free list */
    int unit;                                       /* BCM layer unit ID */
    _bcm_fe2k_field_count groupFreeCount;           /* free groups */
    _bcm_fe2k_field_count groupTotal;               /* total groups */
    _bcm_fe2k_field_count entryFreeCount;           /* free entries */
    _bcm_fe2k_field_count entryTotal;               /* total entries */
    _bcm_fe2k_field_group_index groupFreeHead;      /* head of free groups */
    _bcm_fe2k_field_entry_index entryFreeHead;      /* head of free entries */
    bcm_policer_t dropPolicer;                      /* policer for drop actn */
    uint8 uMaxSupportedStages;
} _bcm_fe2k_g2p3_field_glob_t;

/*
 *  These values are used when manipulating policers...
 */
#define _SBX_FE2K_FIELD_POLICER_READ 1
#define _SBX_FE2K_FIELD_POLICER_CLEAR 0
#define _SBX_FE2K_FIELD_POLICER_WRITE -1

/*
 *  These macros deal with the SBX port bitmap of a rule.
 */
#define FIELD_PBMP_SBX_LEN 7
#define FIELD_PBMP_SBX_MAX (FIELD_PBMP_SBX_LEN - 1)
#define FIELD_PBMP_TO_SBX(_sbxPBmp, _bcmPBmp) \
    do { \
        unsigned int _i1; \
        unsigned int _i2; \
        sal_memset(&(_sbxPBmp), 0xFF, sizeof(_sbxPBmp)); \
        for (_i1 = 0, _i2 = 0; \
             (_i1 < _SHR_PBMP_WORD_MAX) && (_i2 < FIELD_PBMP_SBX_LEN); \
             _i2++) { \
            (_sbxPBmp)[FIELD_PBMP_SBX_MAX - _i2] = ((~(_SHR_PBMP_WORD_GET(_bcmPBmp, _i1))) >> ((_i2 & 0x03) * 8)) & 0xFF; \
            if (3 == (_i2 & 3)) { \
                _i1++; \
            } \
        } \
    } while (0)
#define FIELD_PBMP_FROM_SBX(_bcmPBmp, _sbxPBmp, _unit) \
    do { \
        unsigned int _i1; \
        unsigned int _i2; \
        uint32 _pbTmp; \
        sal_memset(&(_bcmPBmp), 0x00, sizeof(_bcmPBmp)); \
        for (_i1 = 0, _i2 = 0, _pbTmp = 0; \
             (_i1 < _SHR_PBMP_WORD_MAX) && (_i2 < FIELD_PBMP_SBX_LEN); \
             _i2++) { \
            _pbTmp |= (((~(_sbxPBmp[FIELD_PBMP_SBX_MAX - _i2])) & 0xFF) << ((_i2 & 3) * 8)); \
            if (3 == (_i2 & 3)) { \
                _SHR_PBMP_WORD_SET(_bcmPBmp, _i1, _pbTmp); \
                _pbTmp = 0; \
                _i1++; \
            } \
        } \
        if ( (3 != ((_i2 - 1) & 3)) && (_i1 < _SHR_PBMP_WORD_MAX) ){ \
            _SHR_PBMP_WORD_SET(_bcmPBmp, _i1, _pbTmp); \
        } \
        BCM_PBMP_AND(_bcmPBmp, PBMP_ALL(_unit)); \
    } while (0)
#define FIELD_PBMP_SBX_FORMAT "%02X %02X %02X %02X %02X %02X %02X"
#define FIELD_PBMP_SBX_SHOW(_pbmp) \
    (_pbmp)[0], \
    (_pbmp)[1], \
    (_pbmp)[2], \
    (_pbmp)[3], \
    (_pbmp)[4], \
    (_pbmp)[5], \
    (_pbmp)[6]

#ifdef BCM_WARM_BOOT_SUPPORT
/* Warmboot support */
typedef struct soc_sbx_g2p3_field_wb_rulebase_s {
   uint16  entryHead;
   uint8 groupHead;
   uint16 entries;
} soc_sbx_g2p3_field_wb_rulebase_t;

typedef struct soc_sbx_g2p3_field_wb_group_s {
    bcm_field_qset_t qset;                        /* qualifier set bitmap */
    int priority;                                 /* priority */
    uint8 rulebase;                               /* which rulebase */
    uint8 nextGroup;                              /* next group id */
    uint8 prevGroup;                              /* previous group id */
} soc_sbx_g2p3_field_wb_group_t;

typedef struct soc_sbx_g2p3_field_wb_entry_s {
    int16 priority;                               /* priority */
    uint16 nextEntryRb;                           /* next entry */
    uint32 entryFlags;                            /* TODO: recover flags rather than store */
    uint8 group;                                  /* group */
    uint8 counterMode;                            /* counter mode */
    bcm_module_t module;                          /* redir target module */
    bcm_port_t port;                              /* redir target port */
    bcm_policer_t policer;                        /* policer */
    uint16 range[_FIELD_FE2K_G2P3_RANGES_PER_ENTRY]; /* ranges */
} soc_sbx_g2p3_field_wb_entry_t;

typedef struct fe2k_field_wb_mem_layout_s {
    soc_sbx_g2p3_field_wb_rulebase_t rulebase[_FIELD_FE2K_G2P3_STAGES];
    soc_sbx_g2p3_field_wb_group_t group[_FIELD_FE2K_G2P3_MAX_GROUPS];
    soc_sbx_g2p3_field_wb_entry_t entry[0];
}fe2k_field_wb_mem_layout_t;

typedef struct soc_sbx_g2p3_field_cache_s {
    uint32                  wb_hdl;
    uint32                  scache_size;
    soc_sbx_g2p3_field_wb_rulebase_t *rulebase;
    soc_sbx_g2p3_field_wb_group_t *group;
    soc_sbx_g2p3_field_wb_entry_t *entry;
} soc_sbx_g2p3_field_cache_t;

soc_sbx_g2p3_field_cache_t _field_cache[BCM_MAX_NUM_UNITS];

#define G2P3_FIELD_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define G2P3_FIELD_WB_CURRENT_VERSION            G2P3_FIELD_WB_VERSION_1_0

/* Is Level 2 warmboot configured - */
#define L2CACHE_WB_L2_CONFIGURED(unit) (_field_cache[unit].scache_size != 0)

static int 
_bcm_fe2000_g2p3_field_scache_store(_bcm_fe2k_g2p3_field_glob_t *thisUnit,
                                    unsigned int rulebaseId);
static int
_bcm_fe2000_g2p3_field_action_wb_recover(void *unitData,
                                 int rulebase,
                                 bcm_field_entry_t entry,
                                 _bcm_fe2k_g2p3_hardware_rule_t *pRule);

#ifdef BROADCOM_DEBUG
static int
_bcm_fe2k_g2p3_field_rulebase_dump(_bcm_fe2k_g2p3_field_glob_t *glob,
                                   const uint8 rulebase,
                                   const char *prefix);
#endif /* BROADCOM_DEBUG */

#endif /* BCM_WARM_BOOT_SUPPORT */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *  Entry management
 *
 */

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_entry_alloc_id
 *  Purpose
 *    Get a specific entry description off the free list
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *glob = pointer to unit globals
 *    (in) _bcm_fe2k_field_entry_index entry = which entry to get
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Presets some fields; others left alone.
 *    Very limited error checking.
 */
static int
_bcm_fe2k_g2p3_field_entry_alloc_id(_bcm_fe2k_g2p3_field_glob_t *glob,
                                    const _bcm_fe2k_field_entry_index entry)
{
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;

    /* check free count */
    if (!(glob->entryFreeCount)) {
        FIELD_ERR((FIELD_MSG1("no available entries on unit %d\n"),
                   glob->unit));
        return BCM_E_RESOURCE;
    }
    /* check whether this entry is available */
    thisEntry = &(glob->entry[entry]);
    if (thisEntry->entryFlags & _FE2K_G2P3_ENTRY_VALID) {
        FIELD_ERR((FIELD_MSG1("unit %d entry ID %08X is already in use\n"),
                   glob->unit,
                   entry));
        return BCM_E_EXISTS;
    }
    /* remove the entry from the list */
    if (thisEntry->nextEntry < glob->entryTotal) {
        /* not end of list */
        glob->entry[thisEntry->nextEntry].prevEntry = thisEntry->prevEntry;
    }
    if (thisEntry->prevEntry < glob->entryTotal) {
        /* not head of list */
        glob->entry[thisEntry->prevEntry].nextEntry = thisEntry->nextEntry;
    } else {
        /* head of list */
        glob->entryFreeHead = thisEntry->nextEntry;
    }
    glob->entryFreeCount--;
    /* initial setup of entry */
    thisEntry->prevEntry = glob->entryTotal;
    thisEntry->nextEntry = glob->entryTotal;
    thisEntry->entryFlags = _FE2K_G2P3_ENTRY_VALID;
    /* done */
    FIELD_EVERB((FIELD_MSG1("unit %d entry %08X allocated from free list\n"),
                 glob->unit,
                 entry));
    return BCM_E_NONE;
}

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_entry_alloc
 *  Purpose
 *    Get an entry description off the free list
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *glob = pointer to unit globals
 *    (out) _bcm_fe2k_field_entry_index *entry = where to put entry ID
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Presets some fields; others left alone.
 *    Very limited error checking.
 *    Destroys current value at entry, even if it fails.
 */
static int
_bcm_fe2k_g2p3_field_entry_alloc(_bcm_fe2k_g2p3_field_glob_t *glob,
                                 _bcm_fe2k_field_entry_index *entry)
{
    /* get head of list */
    *entry = glob->entryFreeHead;
    /* allocate that */
    return _bcm_fe2k_g2p3_field_entry_alloc_id(glob, *entry);
}

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_entry_free
 *  Purpose
 *    Return an entry description to the free list
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *glob = pointer to unit globals
 *    (in) _bcm_fe2k_field_entry_index entry = entry ID to free
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Presets some fields; clears most.
 *    Very limited error checking.
 */
static int
_bcm_fe2k_g2p3_field_entry_free(_bcm_fe2k_g2p3_field_glob_t *glob,
                                const _bcm_fe2k_field_entry_index entry)
{
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;

    /* check valid entry ID */
    if (glob->entryTotal <= entry) {
        FIELD_ERR((FIELD_MSG1("unit %d entry ID %08X invalid\n"),
                   glob->unit,
                   entry));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    /* make sure entry is not already free */
    thisEntry = &(glob->entry[entry]);
    if (!(thisEntry->entryFlags & _FE2K_G2P3_ENTRY_VALID)) {
        /* trying to free already free entry */
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X is already free\n"),
                   glob->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    /* clear the entry */
    sal_memset(thisEntry, 0, sizeof(*thisEntry));
    /* set up some fields specifically */
    thisEntry->prevEntry = glob->entryTotal;
    thisEntry->prevEntryRb = glob->entryTotal;
    thisEntry->nextEntryRb = glob->entryTotal;
    thisEntry->group = glob->groupTotal;
    /* insert the entry as head of free list */
    thisEntry->nextEntry = glob->entryFreeHead;
    glob->entryFreeHead = entry;
    if (thisEntry->nextEntry < glob->entryTotal) {
        /* not only one in list */
        glob->entry[thisEntry->nextEntry].prevEntry = entry;
    }
    glob->entryFreeCount++;
    /* done */
    FIELD_EVERB((FIELD_MSG1("unit %d entry %08X returned to free list\n"),
                 glob->unit,
                 entry));
    return BCM_E_NONE;
}

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_rules_default
 *  Purpose
 *    Set up the rule for an entry in 
 *    the format appropriate for the entry's rulebase (ingress/egress), that
 *    will match everything and do nothing.
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *glob = pointer to unit globals
 *    (in) _bcm_fe2k_field_entry_index entry = which entry to prepare
 *    (out) _bcm_fe2k_g2p3_hardware_rule_t *rule
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Presets some fields; others zeroed.
 *    Very limited error checking.
 */
static int
_bcm_fe2k_g2p3_field_rules_default(_bcm_fe2k_g2p3_field_glob_t *glob,
                                   const _bcm_fe2k_field_entry_index entry,
                                   _bcm_fe2k_g2p3_hardware_rule_t *rule)
{
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;
    bcm_pbmp_t gbPbmp;

    thisEntry = &(glob->entry[entry]);
    /* zero it all */
    sal_memset(rule, 0, sizeof(_bcm_fe2k_g2p3_hardware_rule_t));
    /* only set fields that should be nonzero */
    /* collect ports that apply to this rule */
    BCM_PBMP_ASSIGN(gbPbmp, glob->rulebase[glob->group[thisEntry->group].rulebase].ports);
    /* set nonzero fields according to whether ingress or egress */
    switch (glob->group[thisEntry->group].rulebase) {
    case 0:
        /* entry is ingress */
        /* qualifiers - format */
        /* qualifiers - L1 */
        FIELD_PBMP_TO_SBX(rule->ingress.pbmpn, gbPbmp);
        /* qualifiers - L2 */
        /* qualifiers - L3 */
        /* qualifiers - L4 */
        rule->ingress.dporthi = 0xFFFF;
        rule->ingress.sporthi = 0xFFFF;
        /* actions */
        break;
    case 1:
        /* entry is egress */
        /* qualifiers - format */
        /* qualifiers - L1 */
        FIELD_PBMP_TO_SBX(rule->egress.pbmpn, gbPbmp);
        /* qualifiers - L2 */
        /* qualifiers - L3 */
        /* qualifiers - L4 */
        rule->egress.dporthi = 0xFFFF;
        rule->egress.sporthi = 0xFFFF;
        /* actions */
        break;
    case 2:
        /* entry is ingress */
        /* qualifiers - format */
        /* qualifiers - L1 */
        FIELD_PBMP_TO_SBX(rule->ingress_v6.pbmpn, gbPbmp);
        /* qualifiers - L2 */
        /* qualifiers - L3 */
        /* qualifiers - L4 */
        rule->ingress_v6.dporthi = 0xFFFF;
        rule->ingress_v6.sporthi = 0xFFFF;
        /* actions */
        break;
    case 3:
        /* entry is egress */
        /* qualifiers - format */
        /* qualifiers - L1 */
        FIELD_PBMP_TO_SBX(rule->egress_v6.pbmpn, gbPbmp);
        /* qualifiers - L2 */
        /* qualifiers - L3 */
        /* qualifiers - L4 */
        rule->egress_v6.dporthi = 0xFFFF;
        rule->egress_v6.sporthi = 0xFFFF;
        /* actions */
        break;
    default:
        FIELD_ERR((FIELD_MSG1("invalid rulebase %d for unit %d group %08X"
                              " entry %08X\n"),
                   glob->group[thisEntry->group].rulebase,
                   glob->unit,
                   thisEntry->group,
                   entry));
        return BCM_E_INTERNAL;
    }
    FIELD_EVERB((FIELD_MSG1("set initial defaults for unit %d group %08X"
                            " entry %08X\n"),
                 glob->unit,
                 thisEntry->group,
                 entry));
    return BCM_E_NONE;
}

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_rules_clear_qualifiers
 *  Purpose
 *    Clear all qualifiers from the rule associated with an entry.
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *glob = pointer to unit globals
 *    (in) _bcm_fe2k_field_entry_index entry = which entry to prepare
 *    (out) _bcm_fe2k_g2p3_hardware_rule_t *rule = where to put rule
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Presets some fields; others zeroed.
 *    Very limited error checking.
 */
static int
_bcm_fe2k_g2p3_field_rules_clear_qualifiers(_bcm_fe2k_g2p3_field_glob_t *glob,
                                            const _bcm_fe2k_field_entry_index entry,
                                            _bcm_fe2k_g2p3_hardware_rule_t *rule)
{
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;
    bcm_pbmp_t gbPbmp;

    thisEntry = &(glob->entry[entry]);
    if (thisEntry->hwRule) {
        /* set all fields that are qualifiers to default (match anything) */
        /* collect ports that apply to this rule */
        BCM_PBMP_ASSIGN(gbPbmp, glob->rulebase[glob->group[thisEntry->group].rulebase].ports);
        /* set fields according to whether ingress or egress */
        switch (glob->group[thisEntry->group].rulebase) {
        case 0:
            /* entry is ingress */
            /* qualifiers - format */
            /* qualifiers - L1 */
            FIELD_PBMP_TO_SBX(rule->ingress.pbmpn, gbPbmp);
            /* qualifiers - L2 */
            sal_memset(&(rule->ingress.dmac),
                       0,
                       sizeof(rule->ingress.dmac));
            rule->ingress.dmacwidth = 0;
            sal_memset(&(rule->ingress.smac),
                       0,
                       sizeof(rule->ingress.smac));
            rule->ingress.smacwidth = 0;
            rule->ingress.etype = 0x0000;
            rule->ingress.useetype = FALSE;
            rule->ingress.vid = 0x000; /* outer VID */
            rule->ingress.usevid = FALSE; /* outer VID */
            rule->ingress.pri = 0x0;
            rule->ingress.usepri = FALSE;
            rule->ingress.ecn = FALSE;
            rule->ingress.useecn = FALSE;
            /* qualifiers - L3 */
            rule->ingress.da = 0x00000000; /* IPv4 destination */
            rule->ingress.dawidth = 0; /* IPv4 destination prefix len */
            rule->ingress.sa = 0x00000000; /* IPv4 source */
            rule->ingress.sawidth = 0; /* IPv4 source prefix len */
            rule->ingress.proto = 0x00;
            rule->ingress.useproto = FALSE;
            rule->ingress.dscp = 0x00;
            rule->ingress.usedscp = FALSE;
            rule->ingress.fragment = FALSE;
            rule->ingress.usefragment = FALSE;
            /* qualifiers - L4 */
            rule->ingress.ack = FALSE;
            rule->ingress.useack = FALSE;
            rule->ingress.fin = FALSE;
            rule->ingress.usefin = FALSE;
            rule->ingress.psh = FALSE;
            rule->ingress.usepsh = FALSE;
            rule->ingress.rst = FALSE;
            rule->ingress.userst = FALSE;
            rule->ingress.syn = FALSE;
            rule->ingress.usesyn = FALSE;
            rule->ingress.urg = FALSE;
            rule->ingress.useurg = FALSE;
            rule->ingress.dportlo = 0x0000;
            rule->ingress.dporthi = 0xFFFF;
            rule->ingress.sportlo = 0x0000;
            rule->ingress.sporthi = 0xFFFF;
            /* actions */
            break;
        case 1:
            /* entry is egress */
            /* qualifiers - format */
            /* qualifiers - L1 */
            FIELD_PBMP_TO_SBX(rule->egress.pbmpn, gbPbmp);
            /* qualifiers - L2 */
            sal_memset(&(rule->egress.dmac),
                       0,
                       sizeof(rule->egress.dmac));
            rule->ingress.dmacwidth = 0;
            sal_memset(&(rule->egress.smac),
                       0,
                       sizeof(rule->egress.smac));
            rule->ingress.smacwidth = 0;
            rule->egress.etype = 0x0000;
            rule->egress.useetype = FALSE;
            rule->egress.pri = 0x0;
            rule->egress.usepri = FALSE;
            rule->egress.ecn = FALSE;
            rule->egress.useecn = FALSE;
            /* qualifiers - L3 */
            rule->egress.da = 0x00000000; /* IPv4 destination */
            rule->egress.dawidth = 0; /* IPv4 destination prefix len */
            rule->egress.sa = 0x00000000; /* IPv4 source */
            rule->egress.sawidth = 0; /* IPv4 source prefix len */
            rule->egress.proto = 0x00;
            rule->egress.useproto = FALSE;
            rule->egress.dscp = 0x00;
            rule->egress.usedscp = FALSE;
            rule->egress.fragment = FALSE;
            rule->egress.usefragment = FALSE;
            /* qualifiers - L4 */
            rule->egress.ack = FALSE;
            rule->egress.useack = FALSE;
            rule->egress.fin = FALSE;
            rule->egress.usefin = FALSE;
            rule->egress.psh = FALSE;
            rule->egress.usepsh = FALSE;
            rule->egress.rst = FALSE;
            rule->egress.userst = FALSE;
            rule->egress.syn = FALSE;
            rule->egress.usesyn = FALSE;
            rule->egress.urg = FALSE;
            rule->egress.useurg = FALSE;
            rule->egress.dportlo = 0x0000;
            rule->egress.dporthi = 0xFFFF;
            rule->egress.sportlo = 0x0000;
            rule->egress.sporthi = 0xFFFF;
            /* actions */
            break;
        case 2:
            /* entry is ingress */
            /* qualifiers - format */
            /* qualifiers - L1 */
            FIELD_PBMP_TO_SBX(rule->ingress_v6.pbmpn, gbPbmp);
            rule->ingress_v6.vid = 0x000; /* outer VID */
            rule->ingress_v6.usevid = FALSE; /* outer VID */
            /* qualifiers - L3 */
            sal_memset(&(rule->ingress_v6.da),
                       0,
                       sizeof(rule->ingress_v6.da));  /* IPv6 destination */
            rule->ingress_v6.dawidth = 0; /* IPv6 destination prefix len */
            sal_memset(&(rule->ingress_v6.sa),
                       0,
                       sizeof(rule->ingress_v6.sa));  /* IPv6 source */
            rule->ingress_v6.sawidth = 0; /* IPv6 source prefix len */
            rule->ingress_v6.nextheaderlo = 0x00;
            rule->ingress_v6.nextheaderhi = 0xFF;
            rule->ingress_v6.TC = 0x00;
            rule->ingress_v6.useTC = FALSE;
            /* qualifiers - L4 */
            rule->ingress_v6.ack = FALSE;
            rule->ingress_v6.useack = FALSE;
            rule->ingress_v6.fin = FALSE;
            rule->ingress_v6.usefin = FALSE;
            rule->ingress_v6.psh = FALSE;
            rule->ingress_v6.usepsh = FALSE;
            rule->ingress_v6.rst = FALSE;
            rule->ingress_v6.userst = FALSE;
            rule->ingress_v6.syn = FALSE;
            rule->ingress_v6.usesyn = FALSE;
            rule->ingress_v6.urg = FALSE;
            rule->ingress_v6.useurg = FALSE;
            rule->ingress_v6.dportlo = 0x0000;
            rule->ingress_v6.dporthi = 0xFFFF;
            rule->ingress_v6.sportlo = 0x0000;
            rule->ingress_v6.sporthi = 0xFFFF;
            /* actions */
            break;
        case 3:
            /* entry is egress */
            /* qualifiers - format */
            /* qualifiers - L1 */
            FIELD_PBMP_TO_SBX(rule->egress_v6.pbmpn, gbPbmp);
            /* qualifiers - L2 */
#if 0  /* Not used in egress? */
            rule->egress_v6.vid = 0x000; /* outer VID */
            rule->egress_v6.usevid = FALSE; /* outer VID */
#endif
            /* qualifiers - L3 */
            sal_memset(&(rule->egress_v6.da),
                       0,
                       sizeof(rule->egress_v6.da));  /* IPv6 destination */
            rule->egress_v6.dawidth = 0; /* IPv6 destination prefix len */
            sal_memset(&(rule->egress_v6.sa),
                       0,
                       sizeof(rule->egress_v6.sa));  /* IPv6 source */
            rule->egress_v6.sawidth = 0; /* IPv6 source prefix len */
            rule->egress_v6.nextheaderlo = 0x0;
            rule->egress_v6.nextheaderhi = 0xFF;
            rule->egress_v6.TC = 0x00;
            rule->egress_v6.useTC = FALSE;
            /* qualifiers - L4 */
            rule->egress_v6.ack = FALSE;
            rule->egress_v6.useack = FALSE;
            rule->egress_v6.fin = FALSE;
            rule->egress_v6.usefin = FALSE;
            rule->egress_v6.psh = FALSE;
            rule->egress_v6.usepsh = FALSE;
            rule->egress_v6.rst = FALSE;
            rule->egress_v6.userst = FALSE;
            rule->egress_v6.syn = FALSE;
            rule->egress_v6.usesyn = FALSE;
            rule->egress_v6.urg = FALSE;
            rule->egress_v6.useurg = FALSE;
            rule->egress_v6.dportlo = 0x0000;
            rule->egress_v6.dporthi = 0xFFFF;
            rule->egress_v6.sportlo = 0x0000;
            rule->egress_v6.sporthi = 0xFFFF;
            /* actions */
            break;
        default:
            FIELD_ERR((FIELD_MSG1("invalid rulebase %d for unit %d group %08X"
                                  " entry %08X\n"),
                       glob->group[thisEntry->group].rulebase,
                       glob->unit,
                       thisEntry->group,
                       entry));
            return BCM_E_INTERNAL;
        }
    }
    FIELD_EVERB((FIELD_MSG1("set no qualifiers for unit %d group %08X"
                            " entry %08X\n"),
                 glob->unit,
                 thisEntry->group,
                 entry));
    return BCM_E_NONE;
}

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_rules_clear_actions
 *  Purpose
 *    Clear all actions from the rule associated with an entry.
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *glob = pointer to unit globals
 *    (in) _bcm_fe2k_field_entry_index entry = which entry to prepare
 *    (out) _bcm_fe2k_g2p3_hardware_rule_t *rule = where to put rule
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Presets some fields; others zeroed.
 *    Very limited error checking.
 */
static int
_bcm_fe2k_g2p3_field_rules_clear_actions(_bcm_fe2k_g2p3_field_glob_t *glob,
                                         const _bcm_fe2k_field_entry_index entry,
                                         _bcm_fe2k_g2p3_hardware_rule_t *rule)
{
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;

    thisEntry = &(glob->entry[entry]);
    /* set all fields that are actions to default (do nothing) */
    /* set fields according to whether ingress or egress */
    switch (glob->group[thisEntry->group].rulebase) {
    case 0:
        /* entry is ingress */
        /* qualifiers - format */
        /* qualifiers - L1 */
        /* qualifiers - L2 */
        /* qualifiers - L3 */
        /* qualifiers - L4 */
        /* actions */
        rule->ingress.vlan = 0x0000; /* target VSI */
        rule->ingress.usevlan = FALSE; /* target VSI */
        rule->ingress.mirror = 0x00; /* mirror 00 for none */
        rule->ingress.ftidx = 0x00000; /* target FT ID */
        rule->ingress.useftidx = FALSE; /* target FT ID */
        rule->ingress.copy = FALSE; /* copy to CPU */
        rule->ingress.dp = 0x0; /* global drop precedence */
        rule->ingress.cos = 0x0; /* remarking COS */
        rule->ingress.fcos = 0x0; /* fabric COS */
        rule->ingress.usecos = FALSE; /* don't remark COS */
        /*
         *  Police is not an action (for some reason), but we also use
         *  a special policer to drop, so restore whatever policer (if
         *  any) is associated with the entry to no longer drop.
         */
        rule->ingress.policer = glob->entry[entry].policer;
        rule->ingress.typedpolice = !(!(glob->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_TYPEPOL));
        rule->ingress.mefcos = !(!(glob->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_MEFCOS));
        rule->ingress.mef = !(!(glob->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_MEF));
        break;
    case 1:
        /* entry is egress */
        /* qualifiers - format */
        /* qualifiers - L1 */
        /* qualifiers - L2 */
        /* qualifiers - L3 */
        /* qualifiers - L4 */
        /* actions */
        rule->egress.drop = FALSE;
        rule->egress.mirror = 0x0; /* mirror 0 for none */
        break;
    case 2:
        /* entry is ingress */
        /* qualifiers - format */
        /* qualifiers - L1 */
        /* qualifiers - L2 */
        /* qualifiers - L3 */
        /* qualifiers - L4 */
        /* actions */
        rule->ingress_v6.vlan = 0x0000; /* target VSI */
        rule->ingress_v6.usevlan = FALSE; /* target VSI */
        rule->ingress_v6.mirror = 0x00; /* mirror 00 for none */
        rule->ingress_v6.ftidx = 0x00000; /* target FT ID */
        rule->ingress_v6.useftidx = FALSE; /* target FT ID */
        rule->ingress_v6.copy = FALSE; /* copy to CPU */
        rule->ingress_v6.dp = 0x0; /* global drop precedence */
        rule->ingress_v6.cos = 0x0; /* remarking COS */
        rule->ingress_v6.fcos = 0x0; /* fabric COS */
        rule->ingress_v6.usecos = FALSE; /* don't remark COS */
        /*
         *  Police is not an action (for some reason), but we also use
         *  a special policer to drop, so restore whatever policer (if
         *  any) is associated with the entry to no longer drop.
         */
        rule->ingress_v6.policer = glob->entry[entry].policer;
        rule->ingress_v6.typedpolice = !(!(glob->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_TYPEPOL));
        rule->ingress_v6.mefcos = !(!(glob->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_MEFCOS));
        rule->ingress_v6.mef = !(!(glob->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_MEF));
        break;
    case 3:
        /* entry is egress */
        /* qualifiers - format */
        /* qualifiers - L1 */
        /* qualifiers - L2 */
        /* qualifiers - L3 */
        /* qualifiers - L4 */
        /* actions */
        rule->egress_v6.drop = FALSE;
        rule->egress_v6.mirror = 0x0; /* mirror 0 for none */
        break;
    default:
        FIELD_ERR((FIELD_MSG1("invalid rulebase %d for unit %d group %08X"
                              " entry %08X\n"),
                   glob->group[thisEntry->group].rulebase,
                   glob->unit,
                   thisEntry->group,
                   entry));
        return BCM_E_INTERNAL;
    }
    FIELD_EVERB((FIELD_MSG1("set no actions for unit %d group %08X"
                            " entry %08X\n"),
                 glob->unit,
                 thisEntry->group,
                 entry));
    return BCM_E_NONE;
}

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_rules_read
 *  Purpose
 *    Read the rule for an entry from the hardware.
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *glob = pointer to unit globals
 *    (in) _bcm_fe2k_field_entry_index entry = which entry to fetch
 *    (out) _bcm_fe2k_g2p3_hardware_rule_t *rule = where to put rule
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Very limited error checking.
 */
static int
_bcm_fe2k_g2p3_field_rules_read(_bcm_fe2k_g2p3_field_glob_t *glob,
                                const _bcm_fe2k_field_entry_index entry,
                                _bcm_fe2k_g2p3_hardware_rule_t *rule)
{
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;
    bcm_pbmp_t gbPbmp;
    int result = BCM_E_NONE;

    thisEntry = &(glob->entry[entry]);
    /* fill in defaults */
    /*
     *  Intentionally discard the result from filling in the defaults -- if
     *  this call would generate an error, we will shortly generate the same
     *  error here for the same reason.
     */
    _bcm_fe2k_g2p3_field_rules_default(glob, entry, rule);
    if (thisEntry->hwRule) {
        /* there really is a rule */
        switch (glob->group[thisEntry->group].rulebase) {
        case 0:
            result = soc_sbx_g2p3_ifp_rule_from_handle(glob->unit,
                                                       thisEntry->hwRule,
                                                       &(rule->ingress));
            break;
        case 1:
            result = soc_sbx_g2p3_efp_rule_from_handle(glob->unit,
                                                       thisEntry->hwRule,
                                                       &(rule->egress));
            break;
        case 2:
            result = soc_sbx_g2p3_ifp_v6_rule_from_handle(glob->unit,
                                                       thisEntry->hwRule,
                                                       &(rule->ingress_v6));
            break;
        case 3:
            result = soc_sbx_g2p3_efp_v6_rule_from_handle(glob->unit,
                                                       thisEntry->hwRule,
                                                       &(rule->egress_v6));
            break;
        default:
            FIELD_ERR((FIELD_MSG1("invalid rulebase %d for unit %d"
                                  " entry %08X (group %08X)\n"),
                       glob->group[thisEntry->group].rulebase,
                       glob->unit,
                       entry,
                       thisEntry->group));
            return BCM_E_INTERNAL;
        } /* switch (thisEntry->rulebase) */
        if (BCM_E_NONE != result) {
            FIELD_ERR((FIELD_MSG1("unable to read unit %d group %08X"
                                  " entry %08X rule (rb %02X"
                                  " rule %08X): %d (%s)\n"),
                       glob->unit,
                       thisEntry->group,
                       entry,
                       glob->rulebase[glob->group[thisEntry->group].rulebase].dbId,
                       (uint32)(thisEntry->hwRule),
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
    } else { /* if (thisEntry->hwRule) */
        BCM_PBMP_ASSIGN(gbPbmp, glob->rulebase[glob->group[thisEntry->group].rulebase].ports);
        /* fill in some values */
        switch (glob->group[thisEntry->group].rulebase) {
        case 0:
            /* get all ports allowed */
            FIELD_PBMP_TO_SBX(rule->ingress.pbmpn, gbPbmp);
            break;
        case 1:
            FIELD_PBMP_TO_SBX(rule->egress.pbmpn, gbPbmp);
            break;
        case 2:
            /* get all ports allowed */
            FIELD_PBMP_TO_SBX(rule->ingress_v6.pbmpn, gbPbmp);
            break;
        case 3:
            FIELD_PBMP_TO_SBX(rule->egress_v6.pbmpn, gbPbmp);
            break;
        default:
            FIELD_ERR((FIELD_MSG1("invalid rulebase %d for unit %d"
                                  " entry %08X (group %08X)\n"),
                       glob->group[thisEntry->group].rulebase,
                       glob->unit,
                       entry,
                       thisEntry->group));
            return BCM_E_INTERNAL;
        } /* switch (thisEntry->rulebase) */
    } /* if (thisEntry->hwRule) */
    FIELD_EVERB((FIELD_MSG1("load current rules for unit %d entry %08X\n"),
                 glob->unit,
                 entry));
    return result;
}

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_rules_write
 *  Purpose
 *    Read the rule for an entry from the hardware.
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *glob = pointer to unit globals
 *    (in) _bcm_fe2k_field_entry_index entry = which entry to fetch
 *    (in) _bcm_fe2k_g2p3_hardware_rule_t *rule = where to get rule
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Very limited error checking.
 */

static int
_bcm_fe2k_g2p3_field_rules_write(_bcm_fe2k_g2p3_field_glob_t *glob,
                                 const _bcm_fe2k_field_entry_index entry,
                                 _bcm_fe2k_g2p3_hardware_rule_t *rule)
{
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;
    int result = BCM_E_NONE;

    thisEntry = &(glob->entry[entry]);
    
    /* write this entry's rule */
    if (thisEntry->hwRule) {
        /* there really is a rule for this entry */
        switch (glob->group[thisEntry->group].rulebase) {
        case 0:
            result = soc_sbx_g2p3_ifp_handle_update(glob->unit,
                                                    thisEntry->hwRule,
                                                    &(rule->ingress));
            break;
        case 1:
            result = soc_sbx_g2p3_efp_handle_update(glob->unit,
                                                    thisEntry->hwRule,
                                                    &(rule->egress));
            break;
        case 2:
            result = soc_sbx_g2p3_ifp_v6_handle_update(glob->unit,
                                                    thisEntry->hwRule,
                                                    &(rule->ingress_v6));
            break;
        case 3:
            result = soc_sbx_g2p3_efp_v6_handle_update(glob->unit,
                                                    thisEntry->hwRule,
                                                    &(rule->egress_v6));
            break;
        default:
            FIELD_ERR((FIELD_MSG1("invalid rulebase %d for unit %d"
                                  " entry %08X (group %08X)\n"),
                       glob->group[thisEntry->group].rulebase,
                       glob->unit,
                       entry,
                       thisEntry->group));
            return BCM_E_INTERNAL;
        } /* switch (glob->group[thisEntry->group].rulebase) */
        if (BCM_E_NONE != result) {
            FIELD_ERR((FIELD_MSG1("unable to write unit %d group %08X"
                                  " entry %08X rule (rb %02X"
                                  " rule %08X): %d (%s)\n"),
                       glob->unit,
                       thisEntry->group,
                       entry,
                       glob->rulebase[glob->group[thisEntry->group].rulebase].dbId,
                       (uint32)(thisEntry->hwRule),
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
    } /* if (thisEntry->hwRule) */

    /* if entry was previously in hardware, update its status */
    if (thisEntry->entryFlags & _FE2K_G2P3_ENTRY_IN_HW) {
        thisEntry->entryFlags &= (~_FE2K_G2P3_ENTRY_IN_HW);
        thisEntry->entryFlags |= _FE2K_G2P3_ENTRY_WANT_HW;
    }

    FIELD_EVERB((FIELD_MSG1("write new rules for unit %d entry %08X\n"),
                 glob->unit,
                 entry));
    return result;
}

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_rules_resync
 *  Purpose
 *    Write specified entry data to the entry's rules, adding or removing rules
 *    as necessary to reflect the need for the entry to exist
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *glob = pointer to unit globals
 *    (in) _bcm_fe2k_field_group_index group = which group for add
 *    (in) _bcm_fe2k_field_entry_index entry = which entry to add
 *    (in) _bcm_fe2k_g2p3_hardware_rule_t *rule = where to get rule
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Very limited error checking.
 *    Assumes appropriate data in each of the rule buffers.
 *    May change batch presence for a rule if port qualifiers suggest this.
 */

static int
_bcm_fe2k_g2p3_field_rules_resync(_bcm_fe2k_g2p3_field_glob_t *glob,
                                  const _bcm_fe2k_field_entry_index entry,
                                  _bcm_fe2k_g2p3_hardware_rule_t *rule)
{
    soc_sbx_g2p3_ace_rule_handle_t predRule;
    _bcm_fe2k_g2p3_field_rulebase_t *thisRb;
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;
    _bcm_fe2k_field_entry_index currEntry;
    bcm_pbmp_t targetPorts;
    uint32 needRule = 0;
    uint32 haveRule = 0;
    uint32 newRule = 0;
    int result = BCM_E_NONE;
    uint8 rulebase;

    /* check valid entry */
    if (glob->entryTotal <= entry) {
        FIELD_ERR((FIELD_MSG1("invalid entry %08X on unit %d\n"),
                   entry,
                   glob->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    thisEntry = &(glob->entry[entry]);
    if (!(thisEntry->entryFlags & _FE2K_G2P3_ENTRY_VALID)) {
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X is not in use\n"),
                   glob->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    rulebase = glob->group[thisEntry->group].rulebase;
    thisRb = &(glob->rulebase[rulebase]);
    /* fetch the entry port limitation from the rules */
    BCM_PBMP_CLEAR(targetPorts);
    switch (rulebase) {
    case 0:
        /* entry is ingress */
        FIELD_PBMP_FROM_SBX(targetPorts, rule->ingress.pbmpn, glob->unit);
        break;
    case 1:
        /* entry is egress */
        FIELD_PBMP_FROM_SBX(targetPorts, rule->egress.pbmpn, glob->unit);
        break;
    case 2:
        /* entry is ingress v6 */
        FIELD_PBMP_FROM_SBX(targetPorts, rule->ingress_v6.pbmpn, glob->unit);
        break;
    case 3:
        /* entry is egress v6*/
        FIELD_PBMP_FROM_SBX(targetPorts, rule->egress_v6.pbmpn, glob->unit);
        break;
    default:
        FIELD_ERR((FIELD_MSG1("invalid rulebase %d for unit %d"
                              " entry %08X (group %08X)\n"),
                   rulebase,
                   glob->unit,
                   entry,
                   thisEntry->group));
        return BCM_E_INTERNAL;
    }
    
    /*
     *  Need to figure out which rules we need, but we can also take the
     *  opportunity to free rules we don't need anymore.
     */
    if (thisEntry->hwRule) {
        haveRule = 1;
    }
    /* check target port bitmap */
    if (BCM_PBMP_NOT_NULL(targetPorts)) {
        /* at least one target port exists */
        needRule = 1;
    }
    if (haveRule & (~needRule) ) {
        /* we have but don't need a rule */
        switch (rulebase) {
        case 0:
            result = soc_sbx_g2p3_ifp_handle_remove(glob->unit,
                                                    thisEntry->hwRule);
            break;
        case 1:
            result = soc_sbx_g2p3_efp_handle_remove(glob->unit,
                                                    thisEntry->hwRule);
            break;
        case 2:
            result = soc_sbx_g2p3_ifp_v6_handle_remove(glob->unit,
                                                    thisEntry->hwRule);
            break;
        case 3:
            result = soc_sbx_g2p3_efp_v6_handle_remove(glob->unit,
                                                    thisEntry->hwRule);
            break;
        default:
            FIELD_ERR((FIELD_MSG1("invalid rulebase %d for unit %d"
                                  " entry %08X (group %08X)\n"),
                       rulebase,
                       glob->unit,
                       entry,
                       thisEntry->group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
        if (BCM_E_NONE == result) {
            /* mark the rule as gone so we don't purge it multiply */
            thisEntry->hwRule = NULL;
            /* account for the removal of this rule */
            thisRb->rules--;
        } else {
            FIELD_ERR((FIELD_MSG1("unable to remove unit %d group %08X"
                                  " entry %08X rule (rb %02X"
                                  " rule %08X): %d (%s)\n"),
                       glob->unit,
                       thisEntry->group,
                       entry,
                       glob->rulebase[rulebase].dbId,
                       (uint32)(thisEntry->hwRule),
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
        haveRule = 1;
    } /* if (haveRule & (~needRule) ) */
    FIELD_EVERB((FIELD_MSG1("entry ports " FIELD_PBMP_FORMAT "\n"),
                 FIELD_PBMP_SHOW(targetPorts)));
    FIELD_EVERB((FIELD_MSG1("have rule u02X; need rule %02X\n"),
                 haveRule,
                 needRule));
    /* collect any necessary predecessors for adds */
    sal_memset(&predRule, 0, sizeof(predRule));
    if (haveRule != needRule) {
        /* don't have rule we need; search for predecessor */
        newRule = haveRule;
        for (currEntry = thisEntry->prevEntryRb;
             (newRule != needRule) &&
             (currEntry < glob->entryTotal);
             currEntry = glob->entry[currEntry].prevEntryRb) {
                if ((~haveRule) & needRule ) {
                    /* this entry has a predecessor we need */
                    predRule = glob->entry[currEntry].hwRule;
                    newRule = 1;
                }
        } /* for (each predecessor entry in rulebase until have all) */
    } /* if (haveRule != needRule) */
    /* add or update the hardware if necessary */
    if (needRule == 1) {
        /* need rule */
        if (haveRule == 1) {
            /* already have an entry; update it */
            switch (rulebase) {
            case 0:
                result = soc_sbx_g2p3_ifp_handle_update(glob->unit,
                                                        thisEntry->hwRule,
                                                        &(rule->ingress));
                break;
            case 1:
                result = soc_sbx_g2p3_efp_handle_update(glob->unit,
                                                        thisEntry->hwRule,
                                                        &(rule->egress));
                break;
            case 2:
                result = soc_sbx_g2p3_ifp_v6_handle_update(glob->unit,
                                                        thisEntry->hwRule,
                                                        &(rule->ingress_v6));
                break;
            case 3:
                result = soc_sbx_g2p3_efp_v6_handle_update(glob->unit,
                                                        thisEntry->hwRule,
                                                        &(rule->egress_v6));
                break;
            default:
                FIELD_ERR((FIELD_MSG1("invalid rulebase %d for unit %d"
                                      " entry %08X (group %08X)\n"),
                           rulebase,
                           glob->unit,
                           entry,
                           thisEntry->group));
                return BCM_E_INTERNAL;
            } /* switch (rulebase) */
            if (BCM_E_NONE != result) {
                FIELD_ERR((FIELD_MSG1("unable to update unit %d group"
                                      " %08X entry %08X rule (rb %02X"
                                      " rule %08X):"
                                      " %d (%s)\n"),
                           glob->unit,
                           thisEntry->group,
                           entry,
                           glob->rulebase[rulebase].dbId,
                           (uint32)(thisEntry->hwRule),
                           result,
                           _SHR_ERRMSG(result)));
                return result;
            }
        } else { /* if (haveRule == 1) */
            /* don't have an entry; create it */
            FIELD_EVERB((FIELD_MSG1("Adding new entry after %02X:%08X\n"),
                         thisRb->dbId,
                         (unsigned int)predRule));
            switch (rulebase) {
            case 0:
                result = soc_sbx_g2p3_ifp_handle_after_add(glob->unit,
                                                           thisEntry->group,
                                                           predRule,
                                                           &(rule->ingress),
                                                           &(thisEntry->hwRule));
                break;
            case 1:
                result = soc_sbx_g2p3_efp_handle_after_add(glob->unit,
                                                           thisEntry->group,
                                                           predRule,
                                                           &(rule->egress),
                                                           &(thisEntry->hwRule));
                break;
            case 2:
                result = soc_sbx_g2p3_ifp_v6_handle_after_add(glob->unit,
                                                           thisEntry->group,
                                                           predRule,
                                                           &(rule->ingress_v6),
                                                           &(thisEntry->hwRule));
                break;
            case 3:
                result = soc_sbx_g2p3_efp_v6_handle_after_add(glob->unit,
                                                           thisEntry->group,
                                                           predRule,
                                                           &(rule->egress_v6),
                                                           &(thisEntry->hwRule));
                break;
            default:
                FIELD_ERR((FIELD_MSG1("invalid rulebase %d for unit %d"
                                      " entry %08X (group %08X)\n"),
                           rulebase,
                           glob->unit,
                           entry,
                           thisEntry->group));
                return BCM_E_INTERNAL;
            } /* switch (rulebase) */
            if (BCM_E_NONE == result) {
                /* account for the new rule */
                thisRb->rules++;
            } else { /* if (BCM_E_NONE == result) */
                FIELD_ERR((FIELD_MSG1("unable to create unit %d group"
                                      " %08X entry %08X rule (rb %02X): %d (%s)\n"),
                           glob->unit,
                           thisEntry->group,
                           entry,
                           glob->rulebase[rulebase].dbId,
                           result,
                           _SHR_ERRMSG(result)));
                return result;
            } /* if (BCM_E_NONE == result) */
        } /* if (haveRule == 1) */
    } /* if (needRule) */

    if (SOC_WARM_BOOT(glob->unit) && (haveRule) ) {
        /* account for the new rule */
        thisRb->rules++;
    }

    return result;
}
/*
 *  Function
 *    _bcm_fe2k_g2p3_field_rules_remove
 *  Purpose
 *    Remove the entry's rules from hardware, in preparation to rearrange it
 *    within its group or to destroy the entry completely.
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *glob = pointer to unit globals
 *    (in) _bcm_fe2k_field_entry_index entry = which entry to remove
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Very limited error checking.
 *    Does not preserve rule values; must read the rules first.
 */
static int
_bcm_fe2k_g2p3_field_rules_remove(_bcm_fe2k_g2p3_field_glob_t *glob,
                                  const _bcm_fe2k_field_entry_index entry)
{
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;
    int result = BCM_E_NONE;
    uint8 rulebase;

    /* check valid entry */
    if (glob->entryTotal <= entry) {
        FIELD_ERR((FIELD_MSG1("invalid entry %08X on unit %d\n"),
                   entry,
                   glob->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    thisEntry = &(glob->entry[entry]);
    if (!(thisEntry->entryFlags & _FE2K_G2P3_ENTRY_VALID)) {
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X is not in use\n"),
                   glob->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    rulebase = glob->group[thisEntry->group].rulebase;
    /* get rid of all of the rules for this entry */
    if (thisEntry->hwRule) {
        /* we have a rule in hardware */
        switch (rulebase) {
        case 0:
            result = soc_sbx_g2p3_ifp_handle_remove(glob->unit,
                                                    thisEntry->hwRule);
            break;
        case 1:
            result = soc_sbx_g2p3_efp_handle_remove(glob->unit,
                                                    thisEntry->hwRule);
            break;
        case 2:
            result = soc_sbx_g2p3_ifp_v6_handle_remove(glob->unit,
                                                    thisEntry->hwRule);
            break;
        case 3:
            result = soc_sbx_g2p3_efp_v6_handle_remove(glob->unit,
                                                    thisEntry->hwRule);
            break;
        default:
            FIELD_ERR((FIELD_MSG1("invalid rulebase %d for unit %d"
                                  " entry %08X (group %08X)\n"),
                       rulebase,
                       glob->unit,
                       entry,
                       thisEntry->group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
        if (BCM_E_NONE == result) {
            /* mark the rule as gone so we don't purge it multiply */
            thisEntry->hwRule = NULL;
            /* account for the removal of this rule */
            glob->rulebase[rulebase].rules--;
        } else {
            FIELD_ERR((FIELD_MSG1("unable to remove unit %d group %08X"
                                  " entry %08X rule (rb %02X "
                                  " rule %08X): %d (%s)\n"),
                       glob->unit,
                       thisEntry->group,
                       entry,
                       glob->rulebase[rulebase].dbId,
                       (uint32)(thisEntry->hwRule),
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
    } /* if (thisEntry->hwRule) */
    return result;
}

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_entry_add
 *  Purpose
 *    Add specified entry to speicifed group, priority order (as last entry of
 *    same priority when there are others of this entry's priority.
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *glob = pointer to unit globals
 *    (in) _bcm_fe2k_field_group_index group = which group for add
 *    (in) _bcm_fe2k_field_entry_index entry = which entry to add
 *    (in) _bcm_fe2k_g2p3_hardware_rule_t *rule = where to get rule
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Very limited error checking.
 */

static int
_bcm_fe2k_g2p3_field_entry_add(_bcm_fe2k_g2p3_field_glob_t *glob,
                               const _bcm_fe2k_field_group_index group,
                               const _bcm_fe2k_field_entry_index entry,
                               _bcm_fe2k_g2p3_hardware_rule_t *rule)
{
    _bcm_fe2k_g2p3_field_group_t *thisGroup;
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;
    _bcm_fe2k_field_entry_index currEntry;
    _bcm_fe2k_field_entry_index prevEntry;
    _bcm_fe2k_field_group_index currGroup;

    /* check valid group */
    if (glob->groupTotal <= group) {
        FIELD_ERR((FIELD_MSG1("invalid group %08X on unit %d\n"),
                   group,
                   glob->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    thisGroup = &(glob->group[group]);
    if (glob->uMaxSupportedStages <= thisGroup->rulebase) {
        FIELD_ERR((FIELD_MSG1("unit %d group %08X is not in use\n"),
                   glob->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }
    /* check valid entry */
    if (glob->entryTotal <= entry) {
        FIELD_ERR((FIELD_MSG1("invalid entry %08X on unit %d\n"),
                   entry,
                   glob->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    thisEntry = &(glob->entry[entry]);
    if (!(thisEntry->entryFlags & _FE2K_G2P3_ENTRY_VALID)) {
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X is not in use\n"),
                   glob->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    if (0 > thisEntry->priority) {
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X has invalid priority %d\n"),
                   glob->unit,
                   entry,
                   thisEntry->priority));
        return BCM_E_CONFIG;
    }
    /* find where the entry goes in the group entry list */
    for (prevEntry = glob->entryTotal,
         currEntry = thisGroup->entryHead;
         currEntry < glob->entryTotal;
         prevEntry = currEntry,
         currEntry = glob->entry[currEntry].nextEntry) {
        if (0 < _bcm_fe2000_compare_entry_priority(thisEntry->priority,
                                                   glob->entry[currEntry].priority)) {
            /* this is the first entry whose priority is lower than new one */
            break;
        }
    }
    /* insert the entry within the group after prevEntry */
    if (glob->entryTotal > prevEntry) {
        /* insert within the list */
        thisEntry->nextEntry = glob->entry[prevEntry].nextEntry;
        thisEntry->prevEntry = prevEntry;
        glob->entry[prevEntry].nextEntry = entry;
    } else {
        /* insert at head of list */
        thisEntry->nextEntry = thisGroup->entryHead;
        thisEntry->prevEntry = glob->entryTotal;
        thisGroup->entryHead = entry;
    }
    if (glob->entryTotal > thisEntry->nextEntry) {
        /* not at end of list */
        glob->entry[thisEntry->nextEntry].prevEntry = entry;
    } else {
        /* at end of list */
        thisGroup->entryTail = entry;
    }
    /* find where the entry goes in the rulebase entry list */
    for (currGroup = thisGroup->prevGroup,
         prevEntry = thisEntry->prevEntry;
         (glob->groupTotal > currGroup) &&
         (glob->entryTotal <= prevEntry);
         prevEntry = glob->group[currGroup].entryTail,
         currGroup = glob->group[currGroup].prevGroup) {
        /*
         *  This entry is the first in its group; need to search toward (maybe
         *  all the way to) the beginning of this rulebase and pick the last
         *  entry from a prior group.
         */
    }
    for (currGroup = thisGroup->nextGroup,
         currEntry = thisEntry->nextEntry;
         (glob->groupTotal > currGroup) &&
         (glob->entryTotal <= currEntry);
         currEntry = glob->group[currGroup].entryHead,
         currGroup = glob->group[currGroup].nextGroup) {
        /*
         *  This entry is the last in its group; need to search toward (maybe
         *  all the way to) the end of this rulebase and pick the first entry
         *  from a following group.
         */
    }
    /* we know predecessor and successor in rulebase list, insert it */
    thisEntry->prevEntryRb = prevEntry;
    if (glob->entryTotal > prevEntry) {
        /* not at beginning of rulebase; update prev entry's next pointer */
        glob->entry[prevEntry].nextEntryRb = entry;
    } else {
        /* at beginning of rulebase; update head of list */
        glob->rulebase[thisGroup->rulebase].entryHead = entry;
    }
    thisEntry->nextEntryRb = currEntry;
    if (glob->entryTotal > currEntry) {
        /* not at end of rulebase; update next entry's back pointer */
        glob->entry[currEntry].prevEntryRb = entry;
    }
    /* account for the new entry */
    thisGroup->entries++;
    thisGroup->counters++;
    glob->rulebase[thisGroup->rulebase].entries++;
    /* diagnostics */
    FIELD_EVERB((FIELD_MSG1("added entry %08X to group %08X on unit %d\n"),
                 entry,
                 group,
                 glob->unit));
    /* update the SBX layer rules for this entry */
    return _bcm_fe2k_g2p3_field_rules_resync(glob, entry, rule);
}

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_entry_del
 *  Purpose
 *    Remove specified entry from its group.
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *glob = pointer to unit globals
 *    (in) _bcm_fe2k_field_group_index group = which group for add
 *    (in) _bcm_fe2k_field_entry_index entry = which entry to add
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Very limited error checking.
 *    Does not remove entry from counter sharing, but does adjust group counter
 *    information if the entry is not sharing a counter.
 */
static int
_bcm_fe2k_g2p3_field_entry_del(_bcm_fe2k_g2p3_field_glob_t *glob,
                               const _bcm_fe2k_field_entry_index entry)
{
    _bcm_fe2k_g2p3_field_group_t *thisGroup;
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;
    int result;

    /* check valid entry */
    if (glob->entryTotal <= entry) {
        FIELD_ERR((FIELD_MSG1("invalid entry %08X on unit %d\n"),
                   entry,
                   glob->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    thisEntry = &(glob->entry[entry]);
    if (!(thisEntry->entryFlags & _FE2K_G2P3_ENTRY_VALID)) {
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X is not in use\n"),
                   glob->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    thisGroup = &(glob->group[thisEntry->group]);
    /* pull the entry from the hardware */
    result = _bcm_fe2k_g2p3_field_rules_remove(glob, entry);
    if (BCM_E_NONE != result) {
        FIELD_ERR((FIELD_MSG1("unable to remove unit %d entry %08X rules"
                              " from hardware\n"),
                   glob->unit,
                   entry));
        return result;
    }
    /* remove the entry from the rulebase */
    if (glob->entryTotal > thisEntry->prevEntryRb) {
        /* not the first entry in the rulebase */
        glob->entry[thisEntry->prevEntryRb].nextEntryRb = thisEntry->nextEntryRb;
    } else {
        /* the first entry in the rulebase */
        glob->rulebase[thisGroup->rulebase].entryHead = thisEntry->nextEntryRb;
    }
    if (glob->entryTotal > thisEntry->nextEntryRb) {
        /* not the last entry in the rulebase */
        glob->entry[thisEntry->nextEntryRb].prevEntryRb = thisEntry->prevEntryRb;
    }
    /* remove the entry from the group */
    if (glob->entryTotal > thisEntry->prevEntry) {
        /* not the first entry in the group */
        glob->entry[thisEntry->prevEntry].nextEntry = thisEntry->nextEntry;
    } else {
        /* the first entry in the group */
        thisGroup->entryHead = thisEntry->nextEntry;
    }
    if (glob->entryTotal > thisEntry->nextEntry) {
        /* not the last entry in the group */
        glob->entry[thisEntry->nextEntry].prevEntry = thisEntry->prevEntry;
    } else {
        /* the last entry in the group */
        thisGroup->entryTail = thisEntry->prevEntry;
    }
    glob->rulebase[thisGroup->rulebase].entries--;
    thisGroup->entries--;
    thisGroup->counters--;

    return BCM_E_NONE;
}

/*
 *   Function
 *      _bcm_fe2k_g2p3_field_entry_create
 *   Purpose
 *      Create an empty field entry based upon the specified grup
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_group_t group = the group ID to use
 *      (in/out) bcm_field_entry_t *entry = desired ID if valid
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Annoyingly, this function can only insert the entry into the group
 *      based upon a priority setting of BCM_FIELD_ENTRY_PRIO_DEFAULT, and it
 *      will be moved later if the user actually bothers to set the priority.
 *
 *      Will use the first available entry if an invalid entry ID is given.
 */
static int
_bcm_fe2k_g2p3_field_entry_create(_bcm_fe2k_g2p3_field_glob_t *glob,
                                  const _bcm_fe2k_field_group_index group,
                                  _bcm_fe2k_field_entry_index *entry)
{
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;    /* working entry data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */

    /* check whether group is in use */
    if (glob->uMaxSupportedStages <= glob->group[group].rulebase) {
        /* group is not in use */
        FIELD_ERR((FIELD_MSG1("unit %d group %08X is not in use\n"),
                   glob->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }
    if (glob->rulebase[glob->group[group].rulebase].entries >=
        glob->rulebase[glob->group[group].rulebase].entriesMax) {
        /* there would be too many entries in this rulebase */
        return BCM_E_RESOURCE;
    }
    if (glob->rulebase[glob->group[group].rulebase].rules >=
        glob->rulebase[glob->group[group].rulebase].rulesMax) {
        /* there would be too many rules in this rulebase */
        return BCM_E_RESOURCE;
    }

    /* allocate the new entry */
    if ((glob->entryTotal <= *entry) || (0 > *entry)) {
        /* invalid entry ID; get first available */
        result = _bcm_fe2k_g2p3_field_entry_alloc(glob, entry);
    } else {
        /* valid entry ID; try to allocate it */
        result = _bcm_fe2k_g2p3_field_entry_alloc_id(glob, *entry);
    }
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    thisEntry = &(glob->entry[*entry]);
    /* associate the entry with the group (thence the rulebase) */
    thisEntry->group = group;
    thisEntry->priority = BCM_FIELD_ENTRY_PRIO_DEFAULT;
    /* get a default rule for this rulebase */
    result = _bcm_fe2k_g2p3_field_rules_default(glob, *entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* add the entry to its group */
    result = _bcm_fe2k_g2p3_field_entry_add(glob, group, *entry, &rule);
    if (BCM_E_NONE != result) {
        /* failed to add the entry to the group; free the entry */
        _bcm_fe2k_g2p3_field_entry_free(glob, *entry);
    }
    return result;
}

/*
 *   Function
 *      _bcm_fe2k_g2p3_field_entry_copy
 *   Purpose
 *      Create an copy of a field entry, within the same group
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t orgEntry = original entry ID
 *      (in/out) bcm_field_entry_t *newEntry = desired new ID if valid
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Will use the first available entry if an invalid entry ID is given.
 */
static int
_bcm_fe2k_g2p3_field_entry_copy(_bcm_fe2k_g2p3_field_glob_t *glob,
                                const _bcm_fe2k_field_entry_index orgEntry,
                                _bcm_fe2k_field_entry_index *newEntry)
{
    _bcm_fe2k_g2p3_field_entry_t *thatEntry;    /* working entry data */
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;    /* working entry data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */

    /* check arguments */
    thatEntry = &(glob->entry[orgEntry]);
    if (!(thatEntry->entryFlags & _FE2K_G2P3_ENTRY_VALID)) {
        FIELD_ERR((FIELD_MSG1("unit %d original entry %08X is not in use\n"),
                   glob->unit,
                   orgEntry));
        return BCM_E_NOT_FOUND;
    }
    /* allocate the new entry */
    if ((glob->entryTotal <= *newEntry) || (0 > *newEntry)) {
        /* invalid entry ID; get first available */
        result = _bcm_fe2k_g2p3_field_entry_alloc(glob, newEntry);
    } else {
        /* valid entry ID; try to allocate it */
        result = _bcm_fe2k_g2p3_field_entry_alloc_id(glob, *newEntry);
    }
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    thisEntry = &(glob->entry[*newEntry]);
    /* read the qualifier and action data from the old entry */
    result = _bcm_fe2k_g2p3_field_rules_read(glob, orgEntry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* copy other appropriate data from the old entry */
    thisEntry->entryFlags |= (thatEntry->entryFlags &
                              (~_FE2K_G2P3_ENTRY_VALID));
    thisEntry->priority = thatEntry->priority;
    thisEntry->group = thatEntry->group;
    if (thisEntry->entryFlags & _FE2K_G2P3_ENTRY_COUNTER) {
        /* other entry has a counter so this one will too */
        thisEntry->counterMode = thatEntry->counterMode;
    }
    /* add the new entry to its group */
    return _bcm_fe2k_g2p3_field_entry_add(glob,
                                          thisEntry->group,
                                          *newEntry,
                                          &rule);
}

/*
 *   Function
 *      _bcm_fe2k_g2p3_field_entry_destroy
 *   Purpose
 *      Destroy an existing field entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = entry ID to be destroyed
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
static int
_bcm_fe2k_g2p3_field_entry_destroy(_bcm_fe2k_g2p3_field_glob_t *glob,
                                   const _bcm_fe2k_field_entry_index entry)
{
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;    /* working entry data */
    int result;                                 /* working result */

    /* remove the entry from its group */
    result = _bcm_fe2k_g2p3_field_entry_del(glob, entry);
    if (BCM_E_NONE == result) {
        thisEntry = &(glob->entry[entry]);
        /* now dispose of the entry */
        result = _bcm_fe2k_g2p3_field_entry_free(glob, entry);
    }
    return result;
}

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_entry_check_qset
 *  Purpose
 *    Make sure entry's group has the specified qualifier
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *glob = pointer to unit globals
 *    (in) bcm_field_entry_t entry = entry to validate
 *    (in) bcm_field_qualify_t qual = qualifier to check
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Very limited error checking.
 */
static int
_bcm_fe2k_g2p3_field_entry_check_qset(const _bcm_fe2k_g2p3_field_glob_t *glob,
                                      const bcm_field_entry_t entry,
                                      const bcm_field_qualify_t qual)
{
    if ((glob->entryTotal <= entry) || (0 > entry)) {
        FIELD_ERR((FIELD_MSG1("invalid entry %08X specified on unit %d\n"),
                   entry,
                   glob->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    if (!(glob->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_VALID)) {
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X is not in use\n"),
                   glob->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    if ((0 > qual) || (bcmFieldQualifyCount <= qual)) {
        FIELD_ERR((FIELD_MSG1("unit %d does not support invalid"
                              " qualifier %d\n"),
                   glob->unit,
                   qual));
        return BCM_E_PARAM;
    }
    if (!BCM_FIELD_QSET_TEST(glob->group[glob->entry[entry].group].qset,
                             qual)) {
        FIELD_ERR((FIELD_MSG1("unit %d group %08X qset does not include %s\n"),
                   glob->unit,
                   glob->entry[entry].group,
                   _sbx_fe2000_field_qual_name[qual]));
        /* BCM API claims this should be PARAM, though CONFIG is more sense */
        return BCM_E_PARAM;
    }
    return BCM_E_NONE;
}

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_entry_check_action
 *  Purpose
 *    Make sure entry's group can perform the specified action
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *glob = pointer to unit globals
 *    (in) bcm_field_entry_t entry = entry to validate
 *    (in) bcm_field_qualify_t qual = qualifier to check
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Very limited error checking.
 */
static int
_bcm_fe2k_g2p3_field_entry_check_action(const _bcm_fe2k_g2p3_field_glob_t *glob,
                                        const bcm_field_entry_t entry,
                                        const bcm_field_action_t action)
{
    if ((glob->entryTotal <= entry) || (0 > entry)) {
        FIELD_ERR((FIELD_MSG1("invalid entry %08X specified on unit %d\n"),
                   entry,
                   glob->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    if (!(glob->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_VALID)) {
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X is not in use\n"),
                   glob->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    if ((0 > action) || (bcmFieldActionCount <= action)) {
        FIELD_ERR((FIELD_MSG1("unit %d does not support invalid action %d\n"),
                   glob->unit,
                   action));
        return BCM_E_PARAM;
    }
    if (!SHR_BITGET(glob->rulebase[glob->group[glob->entry[entry].group].rulebase].action,
                    action)) {
        FIELD_ERR((FIELD_MSG1("unit %d rulebase %d (group %08X entry %08X)"
                              " actionset does not include %s\n"),
                   glob->unit,
                   glob->group[glob->entry[entry].group].rulebase,
                   glob->entry[entry].group,
                   entry,
                   _sbx_fe2000_field_action_name[action]));
        return BCM_E_CONFIG;
    }
    return BCM_E_NONE;
}

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_entry_counter_access
 *  Purpose
 *    Make sure entry's group can perform the specified action
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *glob = pointer to unit globals
 *    (in) bcm_field_entry_t entry = entry to access
 *    (in) int clear = TRUE to clear counts during read, FALSE to only read
 *    (out) soc_sbx_g2p3_counter_value_t *counts = where to put counts
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Practically no error checking.
 *    Performs saturation instead of rollover.
 */
static int
_bcm_fe2k_g2p3_field_entry_counter_access(const _bcm_fe2k_g2p3_field_glob_t *glob,
                                          const bcm_field_entry_t entry,
                                          const int clear,
                                          soc_sbx_g2p3_counter_value_t *counts)
{
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;    /* working entry data */
    int result = BCM_E_NONE;                    /* working result */
    soc_sbx_g2p3_counter_value_t tempCounts;    /* working counters */
    soc_sbx_g2p3_counter_value_t currCounts;    /* working counters */

    thisEntry = &(glob->entry[entry]);
    if (0 == (thisEntry->entryFlags & _FE2K_G2P3_ENTRY_IN_HW)) {
        /* don't bother reading counters for entries not in hardware */
        return BCM_E_NONE;
    }
    /* read each available rule's counter pair */
    if (thisEntry->hwRule) {
        FIELD_EVERB((FIELD_MSG1("%s counter for unit %d entry %08X"
                                " (rule %02X:%08X)\n"),
                     clear?"clear":"read",
                     glob->unit,
                     entry,
                     glob->rulebase[glob->group[thisEntry->group].rulebase].dbId,
                     (uint32)(thisEntry->hwRule)));
        currCounts.bytes = 0;
        currCounts.packets = 0;
        switch (glob->group[thisEntry->group].rulebase) {
        case 0:
            result = soc_sbx_g2p3_ifp_counter_read(glob->unit,
                                                   thisEntry->hwRule,
                                                   &tempCounts,
                                                   clear);
            break;
        case 1:
            result = soc_sbx_g2p3_efp_counter_read(glob->unit,
                                                   thisEntry->hwRule,
                                                   &tempCounts,
                                                   clear);
            break;
        case 2:
            result = soc_sbx_g2p3_ifp_v6_counter_read(glob->unit,
                                                   thisEntry->hwRule,
                                                   &tempCounts,
                                                   clear);
            break;
        case 3:
            result = soc_sbx_g2p3_efp_v6_counter_read(glob->unit,
                                                   thisEntry->hwRule,
                                                   &tempCounts,
                                                   clear);
            break;
        default:
            FIELD_ERR((FIELD_MSG1("invalid rulebase %d for unit %d"
                                  " entry %08X (group %08X)\n"),
                       glob->group[thisEntry->group].rulebase,
                       glob->unit,
                       entry,
                       thisEntry->group));
            return BCM_E_INTERNAL;
        } /* switch (glob->group[thisEntry->group].rulebase) */
        if (BCM_E_NONE != result) {
            FIELD_ERR((FIELD_MSG1("unable to read unit %d entry %08X"
                                  " counters: %d (%s)\n"),
                       glob->unit,
                       entry,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
        FIELD_EVERB((FIELD_MSG1("byte counter for unit %d entry %08X"
                                " (rule %02X:%08X)"
                                " = %08X%08X\n"),
                     glob->unit,
                     entry,
                     glob->rulebase[glob->group[thisEntry->group].rulebase].dbId,
                     (uint32)(thisEntry->hwRule),
                     COMPILER_64_HI(tempCounts.bytes),
                     COMPILER_64_LO(tempCounts.bytes)));
        FIELD_EVERB((FIELD_MSG1("frame counter for unit %d entry %08X"
                                " (rule %02X:%08X)"
                                " = %08X%08X\n"),
                     glob->unit,
                     entry,
                     glob->rulebase[glob->group[thisEntry->group].rulebase].dbId,
                     (uint32)(thisEntry->hwRule),
                     COMPILER_64_HI(tempCounts.packets),
                     COMPILER_64_LO(tempCounts.packets)));
        /* accumulate the counts according to the entry's counter mode */
        if ((thisEntry->entryFlags & _FE2K_G2P3_ENTRY_COUNTER) &&
            (thisEntry->entryFlags & _FE2K_G2P3_ENTRY_CTR_ENA)) {
            /* the entry has a counter that is enabled */
            switch (thisEntry->counterMode & 0x000F) {
            case BCM_FIELD_COUNTER_MODE_NO_YES:
                /* entry lower counter accumulates */
                if (thisEntry->counterMode & BCM_FIELD_COUNTER_MODE_BYTES) {
                    /* mode is bytes */
                    currCounts.packets = tempCounts.bytes;
                } else {
                    /* mode is frames */
                    currCounts.packets = tempCounts.packets;
                }
                break;
            case BCM_FIELD_COUNTER_MODE_YES_NO:
                /* entry upper counter accumulates */
                if (thisEntry->counterMode & BCM_FIELD_COUNTER_MODE_BYTES) {
                    /* mode is bytes */
                    currCounts.bytes = tempCounts.bytes;
                } else {
                    /* mode is frames */
                    currCounts.bytes = tempCounts.packets;
                }
                break;
            case BCM_FIELD_COUNTER_MODE_BYTES_PACKETS:
                /* entry upper is bytes, lower is frames */
                currCounts = tempCounts;
                break;
            default:
                /* NO/NO or unknown mode; count nothing */
                break;
            } /* switch (thisEntry->counterMode & 0x000F) */
            /* now we know what to count, accumulate it */
            counts->bytes += currCounts.bytes;
            counts->packets += currCounts.packets;
            if (counts->bytes < currCounts.bytes) {
                counts->bytes = 0xFFFFFFFFFFFFFFFFll;
            }
            if (counts->packets < currCounts.packets) {
                counts->packets = 0xFFFFFFFFFFFFFFFFll;
            }
        } /* if (entry has counter and counter is enabled) */
    } /* if (thisEntry->hwRule) */
    return result;
}

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_entries_counter_access
 *  Purpose
 *    Make sure entry's group can perform the specified action
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *glob = pointer to unit globals
 *    (in) bcm_field_entry_t entry = entry to access
 *    (in) int clear = TRUE to clear counts during read, FALSE to only read
 *    (out) soc_sbx_g2p3_counter_value_t *counts = where to put counts
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Very limited error checking.
 *    Performs saturation instead of rollover.
 */
static int
_bcm_fe2k_g2p3_field_entries_counter_access(const _bcm_fe2k_g2p3_field_glob_t *glob,
                                            const bcm_field_entry_t entry,
                                            const int clear,
                                            soc_sbx_g2p3_counter_value_t *counts)
{
    int result = BCM_E_NONE;

    if ((glob->entryTotal <= entry) || (0 > entry)) {
        FIELD_ERR((FIELD_MSG1("invalid entry %08X specified on unit %d\n"),
                   entry,
                   glob->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    if (!(glob->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_VALID)) {
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X is not in use\n"),
                   glob->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    if (!(glob->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_COUNTER)) {
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X has no counter\n"),
                   glob->unit,
                   entry));
        return BCM_E_EMPTY;
    }
    counts->bytes = 0;
    counts->packets = 0;
    result = _bcm_fe2k_g2p3_field_entry_counter_access(glob,
                                                       entry,
                                                       clear,
                                                       counts);
    if (BCM_E_NONE != result) {
        FIELD_ERR((FIELD_MSG1("unable to read unit %d entry %08X counts:"
                              " %d (%s)\n"),
                   glob->unit,
                   entry,
                   result,
                   _SHR_ERRMSG(result)));
    }
    return result;
}

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_entry_set_install
 *  Purpose
 *    Flag the entry for inclusion on the hardware
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *glob = pointer to unit globals
 *    (in) bcm_field_entry_t entry = entry to access
 *    (in) int install = TRUE for install, FALSE for remove
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Practically no error checking.
 */
static int
_bcm_fe2k_g2p3_field_entry_set_install(_bcm_fe2k_g2p3_field_glob_t *glob,
                                       const bcm_field_entry_t entry,
                                       const int install)
{
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;    /* working entry data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */

    thisEntry = &(glob->entry[entry]);

    /* get the rules for this entry */
    result = _bcm_fe2k_g2p3_field_rules_read(glob, entry, &rule);
    if (BCM_E_NONE != result) {
        /* called function displays reason for failure */
        return result;
    }
    /* set all the rules' enable flags */
    switch (glob->group[thisEntry->group].rulebase) {
    case 0:
        rule.ingress.enable = (0 != install);
        break;
    case 1:
        rule.egress.enable = (0 != install);
        break;
    case 2:
        rule.ingress_v6.enable = (0 != install);
        break;
    case 3:
        rule.egress_v6.enable = (0 != install);
        break;
    default:
        FIELD_ERR((FIELD_MSG1("invalid rulebase %d for unit %d"
                              " entry %08X (group %08X)\n"),
                   glob->group[thisEntry->group].rulebase,
                   glob->unit,
                   entry,
                   thisEntry->group));
        return BCM_E_INTERNAL;
    } /* switch (glob->group[thisEntry->group].rulebase) */
    /* write the rules back */
    result = _bcm_fe2k_g2p3_field_rules_write(glob, entry, &rule);
    if (BCM_E_NONE != result) {
        /* called function displays reason for failure */
        return result;
    }
    /* mark whether the entry wants to be in hardware */
    if (install) {
        thisEntry->entryFlags |= _FE2K_G2P3_ENTRY_WANT_HW;
    } else {
        thisEntry->entryFlags &= (~_FE2K_G2P3_ENTRY_WANT_HW);
    }
    return result;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *  Group management
 *
 */

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_group_alloc_id
 *  Purpose
 *    Get a specific group description off the free list
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *glob = pointer to unit globals
 *    (out) _bcm_fe2k_field_group_index *group = where to put group ID
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Presets some fields; others left alone.
 *    Very limited error checking.
 */
static int
_bcm_fe2k_g2p3_field_group_alloc_id(_bcm_fe2k_g2p3_field_glob_t *glob,
                                    const _bcm_fe2k_field_group_index group)
{
    _bcm_fe2k_g2p3_field_group_t *thisGroup;

    /* check free count */
    if (!(glob->groupFreeCount)) {
        FIELD_ERR((FIELD_MSG1("no available group on unit %d\n"),
                   glob->unit));
        return BCM_E_RESOURCE;
    }
    /* make sure group is free */
    thisGroup = &(glob->group[group]);
    if (_FIELD_FE2K_INVALID_RULEBASE != thisGroup->rulebase) {
        FIELD_ERR((FIELD_MSG1("unit %d group ID %08X is already in use\n"),
                   glob->unit,
                   group));
        return BCM_E_EXISTS;
    }
    /* remove the entry from the list */
    if (thisGroup->nextGroup < glob->groupTotal) {
        /* not end of list */
        glob->group[thisGroup->nextGroup].prevGroup = thisGroup->prevGroup;
    }
    if (thisGroup->prevGroup < glob->groupTotal) {
        /* not head of list */
        glob->group[thisGroup->prevGroup].nextGroup = thisGroup->nextGroup;
    } else {
        /* head of list */
        glob->groupFreeHead = thisGroup->nextGroup;
    }
    glob->groupFreeCount--;
    /* don't leave stale links lying about */
    thisGroup->prevGroup = glob->groupTotal;
    thisGroup->nextGroup = glob->groupTotal;
    /* mark group as neither being in a rulebase nor as being free */
    thisGroup->rulebase = _FIELD_FE2K_TEMP_RULEBASE;
    /* done */
    FIELD_EVERB((FIELD_MSG1("unit %d group %08X allocated from free list\n"),
                 glob->unit,
                 group));
    return BCM_E_NONE;
}

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_group_alloc
 *  Purpose
 *    Get a group description off the free list
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *glob = pointer to unit globals
 *    (out) _bcm_fe2k_field_group_index *group = where to put group ID
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Presets some fields; others left alone.
 *    Very limited error checking.
 *    Destroys current value at group, even if it fails.
 */
static int
_bcm_fe2k_g2p3_field_group_alloc(_bcm_fe2k_g2p3_field_glob_t *glob,
                                 _bcm_fe2k_field_group_index *group)
{
    /* get head of list */
    *group = glob->groupFreeHead;
    /* allocate that */
    return _bcm_fe2k_g2p3_field_group_alloc_id(glob, *group);
}

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_group_free
 *  Purpose
 *    Return an group description to the free list
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *glob = pointer to unit globals
 *    (in) _bcm_fe2k_field_group_index *group = which group ID to free
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Presets some fields; clears most.
 *    Very limited error checking.
 */
static int
_bcm_fe2k_g2p3_field_group_free(_bcm_fe2k_g2p3_field_glob_t *glob,
                                const _bcm_fe2k_field_group_index group)
{
    _bcm_fe2k_g2p3_field_group_t *thisGroup;

    /* check valid group ID */
    if (glob->groupTotal <= group) {
        /* invalid group ID */
        FIELD_ERR((FIELD_MSG1("unit %d group ID %08X invalid\n"),
                   glob->unit,
                   group));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    /* make sure group is not already free and is not in use */
    thisGroup = &(glob->group[group]);
    if (thisGroup->rulebase == _FIELD_FE2K_INVALID_RULEBASE) {
        /* trying to free already free group */
        FIELD_ERR((FIELD_MSG1("unit %d group %08X is already free\n"),
                   glob->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }
    if (thisGroup->entries) {
        /* trying to free a group with entries */
        FIELD_ERR((FIELD_MSG1("unit %d group %08X still has entries\n"),
                   glob->unit,
                   group));
        return BCM_E_BUSY;
    }
    if (thisGroup->rulebase != _FIELD_FE2K_TEMP_RULEBASE) {
        /* trying to free an in-use group */
        FIELD_ERR((FIELD_MSG1("unit %d group %08X is still in rulebase %d\n"),
                   glob->unit,
                   group,
                   thisGroup->rulebase));
        return BCM_E_BUSY;
    }
    /* clear the group */
    sal_memset(thisGroup, 0, sizeof(*thisGroup));
    /* set up some fields specifically */
    thisGroup->prevGroup = glob->groupTotal;
    thisGroup->entryHead = glob->entryTotal;
    thisGroup->entryTail = glob->entryTotal;
    thisGroup->rulebase = _FIELD_FE2K_INVALID_RULEBASE;
    /* insert the group as head of free list */
    thisGroup->nextGroup = glob->groupFreeHead;
    glob->groupFreeHead = group;
    if (thisGroup->nextGroup < glob->groupTotal) {
        /* not only one in list */
        glob->group[thisGroup->nextGroup].prevGroup = group;
    }
    glob->groupFreeCount++;
    /* done */
    FIELD_EVERB((FIELD_MSG1("unit %d group %08X returned to free list\n"),
                 glob->unit,
                 group));
    return BCM_E_NONE;
}

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_group_add
 *  Purpose
 *    Insert a group into the specified rulebase, based upon the priority of
 *    the group and any other groups in that rulebase.  Should be strict
 *    priority, though will follow the 'most recent last' rule if there is a
 *    group with the same priority.
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *glob = pointer to unit globals
 *    (in) unsigned int rulebase = which rulebase (by index)
 *    (in) _bcm_fe2k_field_group_index group = ID of group to be inserted
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Very limited error checking.
 *    Will not insert a group that has entries.
 *    BCM_FIELD_GROUP_PRIO_ANY will always be added last.
 *    Prohibits negative priorities that are not 'special'.
 */
static int
_bcm_fe2k_g2p3_field_group_add(_bcm_fe2k_g2p3_field_glob_t *glob,
                               const unsigned int rulebase,
                               const _bcm_fe2k_field_group_index group)
{
    _bcm_fe2k_g2p3_field_group_t *thisGroup;
    _bcm_fe2k_field_group_index currGroup;
    _bcm_fe2k_field_group_index prevGroup;

    /* make sure everything looks okay */
    if (glob->uMaxSupportedStages <= rulebase) {
        FIELD_ERR((FIELD_MSG1("invalid rulebase %d on unit %d\n"),
                   rulebase,
                   glob->unit));
        return BCM_E_PARAM;
    }
    if (glob->groupTotal <= group) {
        FIELD_ERR((FIELD_MSG1("invalid group %08X on unit %d\n"),
                   group,
                   glob->unit));
        return BCM_E_PARAM;
    }
    thisGroup = &(glob->group[group]);
    if (0 > thisGroup->priority) {
        FIELD_ERR((FIELD_MSG1("unit %d group %08X has invalid priority %d\n"),
                   glob->unit,
                   group,
                   thisGroup->priority));
        return BCM_E_CONFIG;
    }
    if (thisGroup->entries) {
        FIELD_ERR((FIELD_MSG1("unit %d group %08X already has entries\n"),
                   glob->unit,
                   group));
        return BCM_E_CONFIG;
    }
    if (thisGroup->rulebase != _FIELD_FE2K_TEMP_RULEBASE) {
        if (_FIELD_FE2K_INVALID_RULEBASE == thisGroup->rulebase) {
            FIELD_ERR((FIELD_MSG1("unit %d group %08X is not allocated\n"),
                       glob->unit,
                       group));
            return BCM_E_NOT_FOUND;
        }
        if (glob->uMaxSupportedStages > thisGroup->rulebase) {
            FIELD_ERR((FIELD_MSG1("unit %d group %08X is already in"
                                  " rulebase %d\n"),
                       glob->unit,
                       group,
                       thisGroup->rulebase));
            return BCM_E_BUSY;
        }
        FIELD_ERR((FIELD_MSG1("unit %d group %08X is corrupt\n"),
                   glob->unit,
                   group));
        return BCM_E_INTERNAL;
    } /* if (thisGroup->rulebase != _FIELD_FE2K_TEMP_RULEBASE) */
    /* figure out where to put this group */
    currGroup = glob->rulebase[rulebase].groupHead;
    prevGroup = glob->groupTotal;
    while (currGroup < glob->groupTotal) {
        if (glob->group[currGroup].priority < thisGroup->priority) {
            /* this is the first group that is lower priority than inserted */
            break;
        }
        /* go to the next group */
        prevGroup = currGroup;
        currGroup = glob->group[currGroup].nextGroup;
    } /* while (currGroup < glob->groupTotal) */
    /* always insert *after* prevGroup */
    if (prevGroup < glob->groupTotal) {
        /* inserting somewhere after beginning of list */
        thisGroup->nextGroup = glob->group[prevGroup].nextGroup;
        thisGroup->prevGroup = prevGroup;
        glob->group[prevGroup].nextGroup = group;
    } else { /* if (currGroup < glob->groupTotal) */
        /* inserting at the beginning of the list */
        thisGroup->nextGroup = glob->rulebase[rulebase].groupHead;
        thisGroup->prevGroup = glob->groupTotal;
        glob->rulebase[rulebase].groupHead = group;
    } /* if (currGroup < glob->groupTotal) */
    if (thisGroup->nextGroup < glob->groupTotal) {
        /* make sure next element points back correctly */
        glob->group[thisGroup->nextGroup].prevGroup = group;
    }
    /* mark group as being in the new rulebase */
    thisGroup->rulebase = rulebase & 0xFF;
    FIELD_EVERB((FIELD_MSG1("unit %d group %08X added to rulebase %d"
                            " at priority %d\n"),
                 glob->unit,
                 group,
                 rulebase & 0xFF,
                 thisGroup->priority));
    return BCM_E_NONE;
}

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_group_del
 *  Purpose
 *    Remove a group from its rulebase.
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *glob = pointer to unit globals
 *    (in) _bcm_fe2k_field_group_index group = ID of group to be removed
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Very limited error checking.
 *    Will not remove a group that has entries.
 */
static int
_bcm_fe2k_g2p3_field_group_del(_bcm_fe2k_g2p3_field_glob_t *glob,
                               const _bcm_fe2k_field_group_index group)
{
    _bcm_fe2k_g2p3_field_group_t *thisGroup;

    /* make sure everything looks okay */
    if (glob->groupTotal <= group) {
        FIELD_ERR((FIELD_MSG1("invalid group %08X on unit %d\n"),
                   group,
                   glob->unit));
        return BCM_E_PARAM;
    }
    thisGroup = &(glob->group[group]);
    if (thisGroup->entries) {
        FIELD_ERR((FIELD_MSG1("unit %d group %08X still has entries\n"),
                   glob->unit,
                   group));
        return BCM_E_CONFIG;
    }
    if (thisGroup->rulebase >= glob->uMaxSupportedStages) {
        if ((_FIELD_FE2K_TEMP_RULEBASE == thisGroup->rulebase) ||
            (_FIELD_FE2K_INVALID_RULEBASE == thisGroup->rulebase)) {
            FIELD_ERR((FIELD_MSG1("unit %d group %d is not %s\n"),
                       glob->unit,
                       group,
                       ((_FIELD_FE2K_TEMP_RULEBASE == thisGroup->rulebase)?
                        "in a rulebase":
                        "allocated")));
            return ((_FIELD_FE2K_TEMP_RULEBASE == thisGroup->rulebase)?
                    BCM_E_CONFIG:
                    BCM_E_NOT_FOUND);
        }
        FIELD_ERR((FIELD_MSG1("unit %d group %d is corrupt\n"),
                   glob->unit,
                   group));
        return BCM_E_INTERNAL;
    } /* if (thisGroup->rulebase >= glob->uMaxSupportedStages) */
    /* remove this group from the rulebase */
    if (thisGroup->prevGroup < glob->groupTotal) {
        /* not beginning of list */
        glob->group[thisGroup->prevGroup].nextGroup = thisGroup->nextGroup;
    } else {
        /* beginning of list */
        glob->rulebase[thisGroup->rulebase].groupHead = thisGroup->nextGroup;
    }
    if (thisGroup->nextGroup < glob->groupTotal) {
        /* not ending of list */
        glob->group[thisGroup->nextGroup].prevGroup = thisGroup->prevGroup;
    }
    /* don't leave stale links lying about */
    thisGroup->nextGroup = glob->groupTotal;
    thisGroup->prevGroup = glob->groupTotal;
    /* mark group as neither being in a rulebase nor as being free */
    FIELD_EVERB((FIELD_MSG1("unit %d group %08X removed from rulebase %d\n"),
                 glob->unit,
                 group,
                 thisGroup->rulebase));
    soc_sbx_g2p3_ace_group_del(glob->unit,thisGroup->rulebase, (int16_t)group);

    thisGroup->rulebase = _FIELD_FE2K_TEMP_RULEBASE;
    return BCM_E_NONE;
}

/* 
 *   Function
 *       _bcm_fe2000_field_group_qset_translate
 *   Purpose
 *       Translates BCM qset to sbx qset
 *   Parameters
 *       (in) bcm_field_qset_t qset_in = bcm qset
 *       (out) soc_sbx_g2p3_ace_qset_t qset_out = sbx qset
 *   Returns
 *       BCM_E_NONE
 */
int 
_bcm_fe2000_g2p3_field_group_qset_translate(bcm_field_qset_t qset,
                                            soc_sbx_g2p3_ace_qset_t *qset_out)
{
    soc_sbx_g2p3_ace_qset_t temp;
    sal_memset(&temp, 0, sizeof(soc_sbx_g2p3_ace_qset_t));
    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifySrcMac)) {
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifySrcMac); 
    }

    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyDstMac)) {
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifyDstMac); 
    }
    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyDSCP)) {
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifyDSCP); 
    }
    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyOuterVlan)) {
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifyVlanId); 
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifyVlanPri); 
    }
    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyOuterVlanId)) {
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifyVlanId); 
    }
    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyOuterVlanPri)) {
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifyVlanPri); 
    }
    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyIpProtocol)) {
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifyIpProtocol); 
        /* MUST be predicated with EtherType */
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifyEtherType); 
    }
    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifySrcIp)) {
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifySrcIp); 
        /* MUST be predicated with EtherType */
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifyEtherType); 
    }
    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyDstIp)) {
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifyDstIp); 
        /* MUST be predicated with EtherType */
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifyEtherType); 
    }
    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyEtherType)) {
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifyEtherType); 
    }
    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyTcpControl)) {
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifyTcpControl); 
        /* MUST be predicated with EtherType and Protocol */
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifyEtherType); 
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifyIpProtocol); 
    }
    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyInPort)) {
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifyPorts); 
    }
    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyInPorts)) {
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifyPorts); 
    }
    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyRangeCheck)) {
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifyL4SrcPort); 
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifyL4DstPort); 
        /* MUST be qualified with ethertype and protocol */
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifyEtherType); 
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifyIpProtocol); 
    }
    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyOutPort)) {
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifyPorts); 
    }
    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyOutPorts)) {
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifyPorts); 
    }
    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifySrcIp6)) {
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifySrcIp6); 
    }
    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyDstIp6)) {
        BCM_FIELD_QSET_ADD(temp, sbxFieldQualifyDstIp6); 
    }
    memcpy(qset_out, &temp, sizeof(soc_sbx_g2p3_ace_qset_t));

    return BCM_E_NONE;
}

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_group_create
 *  Purpose
 *    Create a new group with the specified ID that has the specified
 *    qualifier set and priority.
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *glob = pointer to unit globals
 *    (in/out) _bcm_fe2k_field_group_index group = desired ID if valid
 *    (in) _bcm_fe2k_g2p3_field_group_t *groupData = group data to use
 *  Returns
 *    int (implied cast from bcm_error_t)
 *      BCM_E_NONE if successful
 *      BCM_E_* appropriately if not
 *  Notes
 *    Very limited error checking.
 *    Allocates first available group ID and fills it into group argument if
 *    provided ID is not valid.
 *    Can not specify a priority already taken by an existing group.
 *    Can not specify a qualifier that another group in the same stage has.
 *    If no stage qualifier, bcmFieldQualifyStageIngressQoS is assumed.
 *    Picks an available priority value if none is given (mods groupData).
 */
static int
_bcm_fe2k_g2p3_field_group_create(_bcm_fe2k_g2p3_field_glob_t *glob,
                                  _bcm_fe2k_field_group_index *group,
                                  _bcm_fe2k_g2p3_field_group_t *groupData)
{
    bcm_field_qset_t qset = groupData->qset;
    int result;
    int priority;
    _bcm_fe2k_field_group_index currGroup;
    _bcm_fe2k_g2p3_field_group_t *thisGroup;
    uint8 tempPri[glob->groupTotal];
    uint8 rulebase;
    soc_sbx_g2p3_ace_qset_t qset_ace;

    /* allocate a group */
    if ((glob->groupTotal <= *group) || (0 > *group)) {
        /* invalid group ID; get first available */
        result = _bcm_fe2k_g2p3_field_group_alloc(glob, group);
    } else {
        /* valid group ID; try to allocate it */
        result = _bcm_fe2k_g2p3_field_group_alloc_id(glob, *group);
    }
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* figure out which stage is appropriate; try ingress first */
    for (rulebase = 0; rulebase < glob->uMaxSupportedStages; rulebase++) {
        result = _bcm_fe2000_qset_subset(glob->rulebase[rulebase].qset,
                                         groupData->qset);
        if ((BCM_E_NONE == result) ||
            ((BCM_E_NONE != result) && (BCM_E_FAIL != result))) {
            /* it fits in this rulebase or something went wrong */
            break;
        }
    } /* for (rulebase = 0; rulebase < glob->uMaxSupportedStages; rulebase++) */
    if (BCM_E_FAIL == result) {
        FIELD_ERR((FIELD_MSG1("specified qset does not match any stage"
                              " on unit %d\n"),
                   glob->unit));
        result = _SBX_FE2K_FIELD_INVALID_QSET_ERR;
    } else if (BCM_E_NONE != result) {
        FIELD_ERR((FIELD_MSG1("unable to compare qsets on unit %d: %d (%s)\n"),
                   glob->unit,
                   result,
                   _SHR_ERRMSG(result)));
    }
    /* check and maybe assign priority */
    if (_SBX_FE2K_FIELD_GROUP_PRIO_ANY == groupData->priority) {
        /* no priority given by caller */
        sal_memset(&(tempPri[0]),
                   0x00,
                   sizeof(tempPri[0]) * glob->groupTotal);
        /* find out which of the first (group count) priorities are in use */
        for (currGroup = glob->rulebase[rulebase].groupHead;
             glob->groupTotal > currGroup;
             currGroup = glob->group[currGroup].nextGroup) {
            if (glob->group[currGroup].priority < glob->groupTotal) {
                tempPri[glob->group[currGroup].priority] = TRUE;
            }
        }
        /* pick the highest available of the first (group count) priorities */
        for (priority = (glob->groupTotal - 1);
             priority > 0;
             priority --) {
            if (!(tempPri[priority])) {
                break;
            }
        }
        groupData->priority = priority;
        FIELD_VVERB((FIELD_MSG1("chose priority %d for this group\n"),
                     priority));
    }
    /* mark stage in qset and scan for same priority in the rulebase */
    if (BCM_E_NONE == result) {
        /* mark the qset for the stage, just in case it's not set */
        BCM_FIELD_QSET_ADD(qset, (rulebase?
                                  bcmFieldQualifyStageEgressSecurity:
                                  bcmFieldQualifyStageIngressQoS));
        /* make sure there are no groups with the same priority or qset */
        for (currGroup = glob->rulebase[rulebase].groupHead;
             glob->groupTotal > currGroup;
             currGroup = glob->group[currGroup].nextGroup) {
            if (glob->group[currGroup].priority == groupData->priority) {
                /* found one with same priority; bail */
                break;
            }
        }
        if (glob->groupTotal > currGroup) {
            /* we found one that matches priority or qset */
            FIELD_ERR((FIELD_MSG1("can not add new group to unit %d that has"
                                  " same priority %d as existing group\n"),
                       glob->unit,
                       groupData->priority));
            /* BCM API regression says this should result in BCM_E_EXISTS */
            result = BCM_E_EXISTS;
        }
    } /* if (BCM_E_NONE == result) */

    /* fill in the allocated group with the provided data & add to rulebase */
    thisGroup = &(glob->group[*group]);
    if (BCM_E_NONE == result) {
        /* fill in the allocated group with the provided data */
        thisGroup->priority = groupData->priority;
        thisGroup->qset = qset;
        /* add the group to the rulebase */
        result = _bcm_fe2k_g2p3_field_group_add(glob, rulebase, *group);
    }

    if (BCM_E_NONE  == result) {
        /* group recovery handle by SOC layer */
        if (!SOC_WARM_BOOT(glob->unit)) {
            _bcm_fe2000_g2p3_field_group_qset_translate(qset, &qset_ace);
            soc_sbx_g2p3_ace_group_create(glob->unit, rulebase, (int16_t) *group,
                                          thisGroup->priority, qset_ace, FALSE);
        }
    }

    /* clean up */
    if (BCM_E_NONE != result) {
        /* something went wrong; free the group */
        /* anything that goes wrong above already provided a diagnostic */
        _bcm_fe2k_g2p3_field_group_free(glob, *group);
    }
    return result;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *  Range management
 *
 */

/*
 *   Function
 *      _bcm_fe2000_g2p3_field_range_create_id
 *   Purpose
 *      Fill in a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) _bcm_fe2k_g2p3_field_glob_t *thisUnit = pointer to unit info
 *      (in) uint32 range = the range ID to use
 *      (in) uint32 flags = flags for the range
 *      (in) bcm_l4_port_t min = low port number for the range
 *      (in) bcm_l4_port_t max = high port number for the range
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range with the specified ID must already be allocated.
 *      No effect on the hardware.
 *      We support Src and Dst port flags together, but not TCP and UDP.  We
 *      also don't support both Src and Dst or both TCP and UDP clear.
 */
static int
_bcm_fe2000_g2p3_field_range_create_id(_bcm_fe2k_g2p3_field_glob_t *glob,
                                       const uint32 range,
                                       const uint32 flags,
                                       const bcm_l4_port_t min,
                                       const bcm_l4_port_t max)
{
    /* verify range parameters */
    if (min > max) {
        FIELD_ERR((FIELD_MSG1("range min %08X must be <= max %08X\n"),
                   min,
                   max));
        return BCM_E_PARAM;
    }
    if (flags & (~(BCM_FIELD_RANGE_TCP |
                   BCM_FIELD_RANGE_UDP |
                   BCM_FIELD_RANGE_SRCPORT |
                   BCM_FIELD_RANGE_DSTPORT))) {
        FIELD_ERR((FIELD_MSG1("invalid range flags specified: %08X\n"),
                   flags & (~(BCM_FIELD_RANGE_TCP |
                              BCM_FIELD_RANGE_UDP |
                              BCM_FIELD_RANGE_SRCPORT |
                              BCM_FIELD_RANGE_DSTPORT))));
        return BCM_E_PARAM;
    }
    if ((BCM_FIELD_RANGE_TCP | BCM_FIELD_RANGE_UDP) ==
         (flags & (BCM_FIELD_RANGE_TCP | BCM_FIELD_RANGE_UDP))) {
        FIELD_ERR((FIELD_MSG1("at most one of TCP or UDP"
                              " may be specified\n")));
        return BCM_E_PARAM;
    }
    if (!(flags & (BCM_FIELD_RANGE_SRCPORT | BCM_FIELD_RANGE_DSTPORT))) {
        FIELD_ERR((FIELD_MSG1("must specify at least one of"
                              " SRCPORT or DSTPORT\n")));
        return BCM_E_PARAM;
    }
    if ((0 > min) ||
        (0 > max) ||
        (0xFFFF < min) ||
        (0xFFFF < max)) {
        FIELD_ERR((FIELD_MSG1("range min %08X or max %08X is not valid TCP"
                              " or UDP port number\n"),
                   min,
                   max));
        return BCM_E_PARAM;
    }
    /* set up the range */
    glob->range[range].flags = flags;
    glob->range[range].min = min;
    glob->range[range].max = max;
    /* done */
    return BCM_E_NONE;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *  Rulebase management
 *
 */

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_rulebase_commit
 *  Purpose
 *    Commit a rulebase and update entry in hardware status
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *glob = pointer to unit globals
 *    (in) unsigned int rulebase = rulebase to commit
 *  Returns
 *    int (implied cast from bcm_error_t)
 *                  BCM_E_NONE if successful
 *                  BCM_E_* appropriately if not
 *  Notes
 *    Little error checking and no locking is done here.
 */
static int
_bcm_fe2k_g2p3_field_rulebase_commit(_bcm_fe2k_g2p3_field_glob_t *glob,
                                     unsigned int rulebase)
{
    _bcm_fe2k_field_entry_index entry;          /* working entry ID */
    _bcm_fe2k_g2p3_fld_entry_flags_t entryFlags;/* working entry flags */
    int result = BCM_E_NONE;                    /* working result */

    /* commit the rulebase */
    FIELD_EVERB((FIELD_MSG1("commit rulebase %d "
                            " (SBX db %02X)\n"),
                 rulebase,
                 glob->rulebase[rulebase].dbId));
    switch (rulebase) {
    case 0:
        result = soc_sbx_g2p3_ifp_handle_commit(glob->unit,
                                                -1);
        break;
    case 1:
        result = soc_sbx_g2p3_efp_handle_commit(glob->unit,
                                                -1);
        break;
    case 2:
        result = soc_sbx_g2p3_ifp_v6_handle_commit(glob->unit,
                                                -1);
        break;
    case 3:
        result = soc_sbx_g2p3_efp_v6_handle_commit(glob->unit,
                                                -1);
        break;
    default:
        FIELD_ERR((FIELD_MSG1("unknown rulebase %d\n"),
                   rulebase));
        return BCM_E_INTERNAL;
    }
    /* update entry in hardware state for affected entries */
    if (BCM_E_NONE == result) {
        FIELD_EVERB((FIELD_MSG1("update 'in hardware' state for entries\n")));
        for (entry = glob->rulebase[rulebase].entryHead;
             entry < glob->entryTotal;
             entry = glob->entry[entry].nextEntryRb) {
            entryFlags = glob->entry[entry].entryFlags;
            if (entryFlags & _FE2K_G2P3_ENTRY_WANT_HW) {
                entryFlags |= _FE2K_G2P3_ENTRY_IN_HW;
            } else {
                entryFlags &= (~_FE2K_G2P3_ENTRY_IN_HW);
            }
            glob->entry[entry].entryFlags = entryFlags;
        }
#ifdef BCM_WARM_BOOT_SUPPORT
        /* update scache */
        result = _bcm_fe2000_g2p3_field_scache_store(glob, rulebase);
        if (BCM_E_NONE  != result) {
            FIELD_ERR((FIELD_MSG1("unable to commit rules to scache for"
                                  " rulebase %d (SBX db %02X): %d (%s)\n"),
                       rulebase,
                       glob->rulebase[rulebase].dbId,
                       result,
                       _SHR_ERRMSG(result)));
        }
#endif
    } else {
        FIELD_ERR((FIELD_MSG1("unable to commit rules to hardware for"
                              " rulebase %d (SBX db %02X): %d (%s)\n"),
                   rulebase,
                   glob->rulebase[rulebase].dbId,
                   result,
                   _SHR_ERRMSG(result)));
    }
    return result;
}

#ifdef BROADCOM_DEBUG
/*****************************************************************************
 *
 *  Debugging support
 *
 */

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_rule_dump
 *  Purpose
 *    Dump information about a single rule to debug output
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *thisUnit = pointer to unit info
 *    (in) _bcm_fe2k_g2p3_field_entry_index entry = the entry ID
 *    (in) char *prefix = a string to display in front of each line
 *    (in) int full = TRUE if displaying full rule, FALSE for ID & counts
 *  Returns
 *    int (implied cast from bcm_error_t)
 *                  BCM_E_NONE if successful
 *                  BCM_E_* appropriately if not
 *  Notes
 *    No error checking or locking is done here.
 */
static int
_bcm_fe2k_g2p3_field_rule_dump(_bcm_fe2k_g2p3_field_glob_t *glob,
                               const _bcm_fe2k_field_entry_index entry,
                               const char *prefix,
                               const int full)
{
    _bcm_fe2k_g2p3_field_entry_t *thisEntry = &(glob->entry[entry]);
    _bcm_fe2k_g2p3_hardware_rule_t thisRule;
    soc_sbx_g2p3_counter_value_t counts;
    bcm_pbmp_t pbmp;
    int displayed;
    int result;
    int tmpRes;
    uint8 tcpCtrlData;
    uint8 tcpCtrlMask;
    bcm_module_t modId;
    bcm_port_t portId;
    bcm_gport_t gportId;
    int enabled = (0 != (glob->entry[entry].entryFlags &
                         _FE2K_G2P3_ENTRY_IN_HW));

    /* display rule information */
    FIELD_PRINT(("%sUnit %d rule %02X:%08X information:\n",
                 prefix,
                 glob->unit,
                 glob->group[thisEntry->group].rulebase,
                 (uint32)(thisEntry->hwRule)));
    if (enabled) {
        /* display rule counts if it should be in hardware */
        switch (glob->group[thisEntry->group].rulebase) {
        case 0:
            result = soc_sbx_g2p3_ifp_counter_read(glob->unit,
                                                   thisEntry->hwRule,
                                                   &counts,
                                                   FALSE);
            break;
        case 1:
            result = soc_sbx_g2p3_efp_counter_read(glob->unit,
                                                   thisEntry->hwRule,
                                                   &counts,
                                                   FALSE);
            break;
        case 2:
            result = soc_sbx_g2p3_ifp_v6_counter_read(glob->unit,
                                                   thisEntry->hwRule,
                                                   &counts,
                                                   FALSE);
            break;
        case 3:
            result = soc_sbx_g2p3_efp_v6_counter_read(glob->unit,
                                                   thisEntry->hwRule,
                                                   &counts,
                                                   FALSE);
            break;
        default:
            FIELD_PRINT(("%s  [Internal error accessing rule]\n", prefix));
            FIELD_ERR((FIELD_MSG1("invalid rulebase %d for unit %d"
                                  " entry %08X (group %08X)\n"),
                       glob->group[thisEntry->group].rulebase,
                       glob->unit,
                       entry,
                       thisEntry->group));
            return BCM_E_INTERNAL;
        } /* switch (glob->group[thisEntry->group].rulebase) */
        if (BCM_E_NONE == result) {
            FIELD_PRINT(("%s  Bytes hit = %08X%08X\n",
                         prefix,
                         COMPILER_64_HI(counts.bytes),
                         COMPILER_64_LO(counts.bytes)));
            FIELD_PRINT(("%s  Frames hit = %08X%08X\n",
                         prefix,
                         COMPILER_64_HI(counts.packets),
                         COMPILER_64_LO(counts.packets)));
        } else {
            FIELD_PRINT(("%s  [Unable to read rule counts: %d (%s)]\n",
                         prefix,
                         result,
                         _SHR_ERRMSG(result)));
            FIELD_ERR((FIELD_MSG("unable to read unit %d rule %02X:%08X"
                                 " counts: %d (%s)\n"),
                       glob->unit,
                       glob->group[thisEntry->group].rulebase,
                       (uint32)(thisEntry->hwRule),
                       result,
                       _SHR_ERRMSG(result)));
        }
    } else { /* if (enabled) */
        result = BCM_E_NONE;
    } /* if (enabled) */
    /* display rule contents */
    if (full) {
        switch (glob->group[thisEntry->group].rulebase) {
        case 0:
            tmpRes = soc_sbx_g2p3_ifp_rule_from_handle(glob->unit,
                                                       thisEntry->hwRule,
                                                       &(thisRule.ingress));
            break;
        case 1:
            tmpRes = soc_sbx_g2p3_efp_rule_from_handle(glob->unit,
                                                       thisEntry->hwRule,
                                                       &(thisRule.egress));
            break;
        case 2:
            tmpRes = soc_sbx_g2p3_ifp_v6_rule_from_handle(glob->unit,
                                                       thisEntry->hwRule,
                                                       &(thisRule.ingress_v6));
            break;
        case 3:
            tmpRes = soc_sbx_g2p3_efp_v6_rule_from_handle(glob->unit,
                                                       thisEntry->hwRule,
                                                       &(thisRule.egress_v6));
            break;
        default:
            /* already bailed out on this condition, but compiler will gripe */
            tmpRes = BCM_E_INTERNAL;
        } /* switch (glob->group[thisEntry->group].rulebase) */
        if (BCM_E_NONE != tmpRes) {
            /* error reading rule information */
            result = tmpRes;
            FIELD_PRINT(("%s  [Unable to read rule contents: %d (%s)]\n",
                         prefix,
                         result,
                         _SHR_ERRMSG(result)));
            FIELD_ERR((FIELD_MSG1("unable to read unit %d rule %02X:%08X:"
                                  " %d (%s)\n"),
                       glob->unit,
                       glob->group[thisEntry->group].rulebase,
                       (uint32)(thisEntry->hwRule),
                       result,
                       _SHR_ERRMSG(result)));
        } else { /* if (BCM_E_NONE != tmpRes) */
            /* successfully read rule information */
            switch (glob->group[thisEntry->group].rulebase) {
            case 0:
                FIELD_PRINT(("%s  Rule enabled = %s\n",
                             prefix,
                             thisRule.ingress.enable?"TRUE":"FALSE"));
                break;
            case 1:
                FIELD_PRINT(("%s  Rule enabled = %s\n",
                             prefix,
                             thisRule.egress.enable?"TRUE":"FALSE"));
                break;
            case 2:
                FIELD_PRINT(("%s  Rule enabled = %s\n",
                             prefix,
                             thisRule.ingress_v6.enable?"TRUE":"FALSE"));
                break;
            case 3:
                FIELD_PRINT(("%s  Rule enabled = %s\n",
                             prefix,
                             thisRule.egress_v6.enable?"TRUE":"FALSE"));
                break;
            default:
                /* already bailed out on this condition, but compiler gripes */
                break;
            }
            displayed = FALSE;
            FIELD_PRINT(("%s  Qualifiers:\n", prefix));
            switch (glob->group[thisEntry->group].rulebase) {
            case 0:
                FIELD_PBMP_FROM_SBX(pbmp, thisRule.ingress.pbmpn, glob->unit);
                if (BCM_PBMP_NEQ(pbmp,
                                 glob->rulebase[glob->group[thisEntry->group].rulebase].ports)) {
                    FIELD_PRINT(("%s    %s = " FIELD_PBMP_FORMAT "\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyInPorts],
                                 FIELD_PBMP_SHOW(pbmp)));
                    FIELD_PRINT(("%s    Native %s = " FIELD_PBMP_SBX_FORMAT "\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyInPorts],
                                 FIELD_PBMP_SBX_SHOW(thisRule.ingress.pbmpn)));
                    displayed = TRUE;
                }
                if (thisRule.ingress.dmacwidth) {
                    FIELD_PRINT(("%s    %s = " FIELD_MACA_FORMAT "/%d\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyDstMac],
                                 FIELD_MACA_SHOW(thisRule.ingress.dmac),
                                 thisRule.ingress.dmacwidth));
                    displayed = TRUE;
                }
                if (thisRule.ingress.smacwidth) {
                    FIELD_PRINT(("%s    %s = " FIELD_MACA_FORMAT "/%d\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifySrcMac],
                                 FIELD_MACA_SHOW(thisRule.ingress.smac),
                                 thisRule.ingress.smacwidth));
                    displayed = TRUE;
                }
                if (thisRule.ingress.usevid) { /* outer VID */
                    FIELD_PRINT(("%s    %s = %03X (%d)\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyOuterVlan],
                                 thisRule.ingress.vid,
                                 thisRule.ingress.vid));
                    displayed = TRUE;
                }
                if (thisRule.ingress.usepri) {
                    FIELD_PRINT(("%s    %s = %01X\n",
                                 prefix,
                                 "(PRI)",
                                 thisRule.ingress.pri));
                    displayed = TRUE;
                }
                if (thisRule.ingress.useecn) {
                    FIELD_PRINT(("%s    %s = %01X\n",
                                 prefix,
                                 "(ECN)",
                                 thisRule.ingress.ecn));
                    displayed = TRUE;
                }
                if (thisRule.ingress.useetype) {
                    FIELD_PRINT(("%s    %s = %04X (%d)\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyEtherType],
                                 thisRule.ingress.etype,
                                 thisRule.ingress.etype));
                    displayed = TRUE;
                }
                if (thisRule.ingress.dawidth) {
                    FIELD_PRINT(("%s    %s = %08X/%08X (%d.%d.%d.%d/%d)\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyDstIp],
                                 thisRule.ingress.da,
                                 thisRule.ingress.dawidth<32?(~0) << (32 - thisRule.ingress.dawidth):~0,
                                 (thisRule.ingress.da >> 24) & 0xFF,
                                 (thisRule.ingress.da >> 16) & 0xFF,
                                 (thisRule.ingress.da >> 8) & 0xFF,
                                 thisRule.ingress.da & 0xFF,
                                 thisRule.ingress.dawidth));
                    displayed = TRUE;
                }
                if (thisRule.ingress.sawidth) {
                    FIELD_PRINT(("%s    %s = %08X/%08X (%d.%d.%d.%d/%d)\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifySrcIp],
                                 thisRule.ingress.sa,
                                 thisRule.ingress.sawidth<32?(~0) << (32 - thisRule.ingress.sawidth):~0,
                                 (thisRule.ingress.sa >> 24) & 0xFF,
                                 (thisRule.ingress.sa >> 16) & 0xFF,
                                 (thisRule.ingress.sa >> 8) & 0xFF,
                                 thisRule.ingress.sa & 0xFF,
                                 thisRule.ingress.sawidth));
                    displayed = TRUE;
                }
                if (thisRule.ingress.useproto) {
                    FIELD_PRINT(("%s    %s = %02X (%d)\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyIpProtocol],
                                 thisRule.ingress.proto,
                                 thisRule.ingress.proto));
                    displayed = TRUE;
                }
                if (thisRule.ingress.usedscp) {
                    FIELD_PRINT(("%s    %s = %02X (%d)\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyDSCP],
                                 thisRule.ingress.dscp,
                                 thisRule.ingress.dscp));
                    displayed = TRUE;
                }
                if (thisRule.ingress.usefragment) {
                    FIELD_PRINT(("%s    %s = %01X\n",
                                 prefix,
                                 "(fragment)",
                                 thisRule.ingress.fragment));
                    displayed = TRUE;
                }
                if ((thisRule.ingress.dportlo != 0x0000) ||
                    (thisRule.ingress.dporthi != 0xFFFF)) {
                    FIELD_PRINT(("%s    %s = %04X..%04X (%d..%d)\n",
                                 prefix,
                                 "(dest port range)",
                                 thisRule.ingress.dportlo,
                                 thisRule.ingress.dporthi,
                                 thisRule.ingress.dportlo,
                                 thisRule.ingress.dporthi));
                    displayed = TRUE;
                }
                if ((thisRule.ingress.sportlo != 0x0000) ||
                    (thisRule.ingress.sporthi != 0xFFFF)) {
                    FIELD_PRINT(("%s    %s = %04X..%04X (%d..%d)\n",
                                 prefix,
                                 "(source port range)",
                                 thisRule.ingress.sportlo,
                                 thisRule.ingress.sporthi,
                                 thisRule.ingress.sportlo,
                                 thisRule.ingress.sporthi));
                    displayed = TRUE;
                }
                tcpCtrlMask = 0;
                tcpCtrlData = 0;
                if (thisRule.ingress.usefin) {
                    tcpCtrlMask |= BCM_FIELD_TCPCONTROL_FIN;
                }
                if (thisRule.ingress.usesyn) {
                    tcpCtrlMask |= BCM_FIELD_TCPCONTROL_SYN;
                }
                if (thisRule.ingress.userst) {
                    tcpCtrlMask |= BCM_FIELD_TCPCONTROL_RST;
                }
                if (thisRule.ingress.usepsh) {
                    tcpCtrlMask |= BCM_FIELD_TCPCONTROL_PSH;
                }
                if (thisRule.ingress.useack) {
                    tcpCtrlMask |= BCM_FIELD_TCPCONTROL_ACK;
                }
                if (thisRule.ingress.useurg) {
                    tcpCtrlMask |= BCM_FIELD_TCPCONTROL_URG;
                }
                if (thisRule.ingress.fin) {
                    tcpCtrlData |= BCM_FIELD_TCPCONTROL_FIN;
                }
                if (thisRule.ingress.syn) {
                    tcpCtrlData |= BCM_FIELD_TCPCONTROL_SYN;
                }
                if (thisRule.ingress.rst) {
                    tcpCtrlData |= BCM_FIELD_TCPCONTROL_RST;
                }
                if (thisRule.ingress.psh) {
                    tcpCtrlData |= BCM_FIELD_TCPCONTROL_PSH;
                }
                if (thisRule.ingress.ack) {
                    tcpCtrlData |= BCM_FIELD_TCPCONTROL_ACK;
                }
                if (thisRule.ingress.urg) {
                    tcpCtrlData |= BCM_FIELD_TCPCONTROL_URG;
                }
                if (tcpCtrlMask) {
                    FIELD_PRINT(("%s    %s = %02X/%02X\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyTcpControl],
                                 tcpCtrlData,
                                 tcpCtrlMask));
                    displayed = TRUE;
                }
                break;
            case 1:
                FIELD_PBMP_FROM_SBX(pbmp, thisRule.egress.pbmpn, glob->unit);
                FIELD_PRINT(("%s    %s = " FIELD_PBMP_FORMAT "\n",
                             prefix,
                             _sbx_fe2000_field_qual_name[bcmFieldQualifyOutPorts],
                             FIELD_PBMP_SHOW(pbmp)));
                FIELD_PRINT(("%s    Native %s = " FIELD_PBMP_SBX_FORMAT "\n",
                             prefix,
                             _sbx_fe2000_field_qual_name[bcmFieldQualifyOutPorts],
                             FIELD_PBMP_SBX_SHOW(thisRule.egress.pbmpn)));
                if (thisRule.egress.dmacwidth) {
                    FIELD_PRINT(("%s    %s = " FIELD_MACA_FORMAT "/%d\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyDstMac],
                                 FIELD_MACA_SHOW(thisRule.egress.dmac),
                                 thisRule.egress.dmacwidth));
                    displayed = TRUE;
                }
                if (thisRule.egress.smacwidth) {
                    FIELD_PRINT(("%s    %s = " FIELD_MACA_FORMAT "/%d\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifySrcMac],
                                 FIELD_MACA_SHOW(thisRule.egress.smac),
                                 thisRule.egress.smacwidth));
                    displayed = TRUE;
                }
                if (thisRule.egress.usepri) {
                    FIELD_PRINT(("%s    %s = %01X\n",
                                 prefix,
                                 "(PRI)",
                                 thisRule.egress.pri));
                    displayed = TRUE;
                }
                if (thisRule.egress.useecn) {
                    FIELD_PRINT(("%s    %s = %01X\n",
                                 prefix,
                                 "(ECN)",
                                 thisRule.egress.ecn));
                    displayed = TRUE;
                }
                if (thisRule.egress.useetype) {
                    FIELD_PRINT(("%s    %s = %04X (%d)\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyEtherType],
                                 thisRule.egress.etype,
                                 thisRule.egress.etype));
                    displayed = TRUE;
                }
                if (thisRule.egress.dawidth) {
                    FIELD_PRINT(("%s    %s = %08X/%08X (%d.%d.%d.%d/%d)\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyDstIp],
                                 thisRule.egress.da,
                                 thisRule.egress.dawidth<32?(~0) << (32 - thisRule.ingress.dawidth):~0,
                                 (thisRule.egress.da >> 24) & 0xFF,
                                 (thisRule.egress.da >> 16) & 0xFF,
                                 (thisRule.egress.da >> 8) & 0xFF,
                                 thisRule.egress.da & 0xFF,
                                 thisRule.egress.dawidth));
                    displayed = TRUE;
                }
                if (thisRule.egress.sawidth) {
                    FIELD_PRINT(("%s    %s = %08X/%08X (%d.%d.%d.%d/%d)\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifySrcIp],
                                 thisRule.egress.sa,
                                 thisRule.egress.sawidth<32?(~0) << (32 - thisRule.ingress.sawidth):~0,
                                 (thisRule.egress.sa >> 24) & 0xFF,
                                 (thisRule.egress.sa >> 16) & 0xFF,
                                 (thisRule.egress.sa >> 8) & 0xFF,
                                 thisRule.egress.sa & 0xFF,
                                 thisRule.egress.sawidth));
                    displayed = TRUE;
                }
                if (thisRule.egress.useproto) {
                    FIELD_PRINT(("%s    %s = %02X (%d)\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyIpProtocol],
                                 thisRule.egress.proto,
                                 thisRule.egress.proto));
                    displayed = TRUE;
                }
                if (thisRule.egress.usedscp) {
                    FIELD_PRINT(("%s    %s = %02X (%d)\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyDSCP],
                                 thisRule.egress.dscp,
                                 thisRule.egress.dscp));
                    displayed = TRUE;
                }
                if (thisRule.egress.usefragment) {
                    FIELD_PRINT(("%s    %s = %01X\n",
                                 prefix,
                                 "(fragment)",
                                 thisRule.egress.fragment));
                    displayed = TRUE;
                }
                if ((thisRule.egress.dportlo != 0x0000) ||
                    (thisRule.egress.dporthi != 0xFFFF)) {
                    FIELD_PRINT(("%s    %s = %04X..%04X (%d..%d)\n",
                                 prefix,
                                 "(dest port range)",
                                 thisRule.egress.dportlo,
                                 thisRule.egress.dporthi,
                                 thisRule.egress.dportlo,
                                 thisRule.egress.dporthi));
                    displayed = TRUE;
                }
                if ((thisRule.egress.sportlo != 0x0000) ||
                    (thisRule.egress.sporthi != 0xFFFF)) {
                    FIELD_PRINT(("%s    %s = %04X..%04X (%d..%d)\n",
                                 prefix,
                                 "(source port range)",
                                 thisRule.egress.sportlo,
                                 thisRule.egress.sporthi,
                                 thisRule.egress.sportlo,
                                 thisRule.egress.sporthi));
                    displayed = TRUE;
                }
                tcpCtrlMask = 0;
                tcpCtrlData = 0;
                if (thisRule.egress.usefin) {
                    tcpCtrlMask |= BCM_FIELD_TCPCONTROL_FIN;
                }
                if (thisRule.egress.usesyn) {
                    tcpCtrlMask |= BCM_FIELD_TCPCONTROL_SYN;
                }
                if (thisRule.egress.userst) {
                    tcpCtrlMask |= BCM_FIELD_TCPCONTROL_RST;
                }
                if (thisRule.egress.usepsh) {
                    tcpCtrlMask |= BCM_FIELD_TCPCONTROL_PSH;
                }
                if (thisRule.egress.useack) {
                    tcpCtrlMask |= BCM_FIELD_TCPCONTROL_ACK;
                }
                if (thisRule.egress.useurg) {
                    tcpCtrlMask |= BCM_FIELD_TCPCONTROL_URG;
                }
                if (thisRule.egress.fin) {
                    tcpCtrlData |= BCM_FIELD_TCPCONTROL_FIN;
                }
                if (thisRule.egress.syn) {
                    tcpCtrlData |= BCM_FIELD_TCPCONTROL_SYN;
                }
                if (thisRule.egress.rst) {
                    tcpCtrlData |= BCM_FIELD_TCPCONTROL_RST;
                }
                if (thisRule.egress.psh) {
                    tcpCtrlData |= BCM_FIELD_TCPCONTROL_PSH;
                }
                if (thisRule.egress.ack) {
                    tcpCtrlData |= BCM_FIELD_TCPCONTROL_ACK;
                }
                if (thisRule.egress.urg) {
                    tcpCtrlData |= BCM_FIELD_TCPCONTROL_URG;
                }
                if (tcpCtrlMask) {
                    FIELD_PRINT(("%s    %s = %02X/%02X\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyTcpControl],
                                 tcpCtrlData,
                                 tcpCtrlMask));
                    displayed = TRUE;
                }
                break;

            case 2:
                FIELD_PBMP_FROM_SBX(pbmp, thisRule.ingress_v6.pbmpn, glob->unit);
                if (BCM_PBMP_NEQ(pbmp,
                                 glob->rulebase[glob->group[thisEntry->group].rulebase].ports)) {
                    FIELD_PRINT(("%s    %s = " FIELD_PBMP_FORMAT "\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyInPorts],
                                 FIELD_PBMP_SHOW(pbmp)));
                    FIELD_PRINT(("%s    Native %s = " FIELD_PBMP_SBX_FORMAT "\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyInPorts],
                                 FIELD_PBMP_SBX_SHOW(thisRule.ingress_v6.pbmpn)));
                    displayed = TRUE;
                }
                if (thisRule.ingress_v6.usevid) { /* outer VID */
                    FIELD_PRINT(("%s    %s = %03X (%d)\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyOuterVlan],
                                 thisRule.ingress_v6.vid,
                                 thisRule.ingress_v6.vid));
                    displayed = TRUE;
                }
                if (thisRule.ingress_v6.dawidth) {
                    FIELD_PRINT(("%s    %s = " FIELD_IPV6A_FORMAT "/%d\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyDstIp6],
                                 FIELD_IPV6A_SHOW(thisRule.ingress_v6.da),
                                 thisRule.ingress_v6.dawidth));
                    displayed = TRUE;
                }
                if (thisRule.ingress_v6.sawidth) {
                    FIELD_PRINT(("%s    %s = " FIELD_IPV6A_FORMAT "/%d\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifySrcIp6],
                                 FIELD_IPV6A_SHOW(thisRule.ingress_v6.sa),
                                 thisRule.ingress_v6.sawidth));
                    displayed = TRUE;
                }              
                if ((thisRule.ingress_v6.sportlo == 0) &&
                    (thisRule.ingress_v6.sporthi == 0)) {
                    FIELD_PRINT(("%s    %s = %02X (%d)\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyIpProtocol],
                                 thisRule.ingress_v6.nextheaderhi,
                                 thisRule.ingress_v6.nextheaderhi));
                    displayed = TRUE;
                }
                if (thisRule.ingress_v6.useTC) {
                    FIELD_PRINT(("%s    %s = %02X (%d)\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyDSCP],
                                 thisRule.ingress_v6.TC,
                                 thisRule.ingress_v6.TC));
                    displayed = TRUE;
                }
                if ((thisRule.ingress_v6.sportlo != 0) ||
                    (thisRule.ingress_v6.sporthi != 0)) {
                    if ((thisRule.ingress_v6.dportlo != 0x0000) ||
                        (thisRule.ingress_v6.dporthi != 0xFFFF)) {
                        FIELD_PRINT(("%s    %s = %04X..%04X (%d..%d)\n",
                                     prefix,
                                     "(dest port range)",
                                     thisRule.ingress_v6.dportlo,
                                     thisRule.ingress_v6.dporthi,
                                     thisRule.ingress_v6.dportlo,
                                     thisRule.ingress_v6.dporthi));
                        displayed = TRUE;
                    }
                    if ((thisRule.ingress_v6.sportlo != 0x0000) ||
                        (thisRule.ingress_v6.sporthi != 0xFFFF)) {
                        FIELD_PRINT(("%s    %s = %04X..%04X (%d..%d)\n",
                                     prefix,
                                     "(source port range)",
                                     thisRule.ingress_v6.sportlo,
                                     thisRule.ingress_v6.sporthi,
                                     thisRule.ingress_v6.sportlo,
                                     thisRule.ingress_v6.sporthi));
                        displayed = TRUE;
                    }
                }
                tcpCtrlMask = 0;
                tcpCtrlData = 0;
                if (thisRule.ingress_v6.usefin) {
                    tcpCtrlMask |= BCM_FIELD_TCPCONTROL_FIN;
                }
                if (thisRule.ingress_v6.usesyn) {
                    tcpCtrlMask |= BCM_FIELD_TCPCONTROL_SYN;
                }
                if (thisRule.ingress_v6.userst) {
                    tcpCtrlMask |= BCM_FIELD_TCPCONTROL_RST;
                }
                if (thisRule.ingress_v6.usepsh) {
                    tcpCtrlMask |= BCM_FIELD_TCPCONTROL_PSH;
                }
                if (thisRule.ingress_v6.useack) {
                    tcpCtrlMask |= BCM_FIELD_TCPCONTROL_ACK;
                }
                if (thisRule.ingress_v6.useurg) {
                    tcpCtrlMask |= BCM_FIELD_TCPCONTROL_URG;
                }
                if (thisRule.ingress_v6.fin) {
                    tcpCtrlData |= BCM_FIELD_TCPCONTROL_FIN;
                }
                if (thisRule.ingress_v6.syn) {
                    tcpCtrlData |= BCM_FIELD_TCPCONTROL_SYN;
                }
                if (thisRule.ingress_v6.rst) {
                    tcpCtrlData |= BCM_FIELD_TCPCONTROL_RST;
                }
                if (thisRule.ingress_v6.psh) {
                    tcpCtrlData |= BCM_FIELD_TCPCONTROL_PSH;
                }
                if (thisRule.ingress_v6.ack) {
                    tcpCtrlData |= BCM_FIELD_TCPCONTROL_ACK;
                }
                if (thisRule.ingress_v6.urg) {
                    tcpCtrlData |= BCM_FIELD_TCPCONTROL_URG;
                }
                if (tcpCtrlMask) {
                    FIELD_PRINT(("%s    %s = %02X/%02X\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyTcpControl],
                                 tcpCtrlData,
                                 tcpCtrlMask));
                    displayed = TRUE;
                }
                break;
            case 3:
                FIELD_PBMP_FROM_SBX(pbmp, thisRule.egress_v6.pbmpn, glob->unit);
                FIELD_PRINT(("%s    %s = " FIELD_PBMP_FORMAT "\n",
                             prefix,
                             _sbx_fe2000_field_qual_name[bcmFieldQualifyOutPorts],
                             FIELD_PBMP_SHOW(pbmp)));
                FIELD_PRINT(("%s    Native %s = " FIELD_PBMP_SBX_FORMAT "\n",
                             prefix,
                             _sbx_fe2000_field_qual_name[bcmFieldQualifyOutPorts],
                             FIELD_PBMP_SBX_SHOW(thisRule.egress_v6.pbmpn)));
                if (thisRule.egress_v6.dawidth) {
                    FIELD_PRINT(("%s    %s = " FIELD_IPV6A_FORMAT "/%d\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyDstIp6],
                                 FIELD_IPV6A_SHOW(thisRule.egress_v6.da),
                                 thisRule.egress_v6.dawidth));
                    displayed = TRUE;
                }
                if (thisRule.egress_v6.sawidth) {
                    FIELD_PRINT(("%s    %s = " FIELD_IPV6A_FORMAT "/%d\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifySrcIp6],
                                 FIELD_IPV6A_SHOW(thisRule.egress_v6.sa),
                                 thisRule.egress_v6.sawidth));
                    displayed = TRUE;
                }
                if ((thisRule.egress_v6.sportlo == 0) &&
                    (thisRule.egress_v6.sporthi == 0)) {
                    FIELD_PRINT(("%s    %s = %02X (%d)\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyIpProtocol],
                                 thisRule.egress_v6.nextheaderhi,
                                 thisRule.egress_v6.nextheaderhi));
                    displayed = TRUE;
                }
                if (thisRule.egress_v6.useTC) {
                    FIELD_PRINT(("%s    %s = %02X (%d)\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyDSCP],
                                 thisRule.egress_v6.TC,
                                 thisRule.egress_v6.TC));
                    displayed = TRUE;
                }
                if ((thisRule.egress_v6.sportlo != 0) ||
                    (thisRule.egress_v6.sporthi != 0)) {
                    if ((thisRule.egress_v6.dportlo != 0x0000) ||
                        (thisRule.egress_v6.dporthi != 0xFFFF)) {
                        FIELD_PRINT(("%s    %s = %04X..%04X (%d..%d)\n",
                                     prefix,
                                    "(dest port range)",
                                     thisRule.egress_v6.dportlo,
                                     thisRule.egress_v6.dporthi,
                                     thisRule.egress_v6.dportlo,
                                     thisRule.egress_v6.dporthi));
                        displayed = TRUE;
                    }
                    if ((thisRule.egress_v6.sportlo != 0x0000) ||
                        (thisRule.egress_v6.sporthi != 0xFFFF)) {
                        FIELD_PRINT(("%s    %s = %04X..%04X (%d..%d)\n",
                                     prefix,
                                     "(source port range)",
                                     thisRule.egress_v6.sportlo,
                                     thisRule.egress_v6.sporthi,
                                     thisRule.egress_v6.sportlo,
                                     thisRule.egress_v6.sporthi));
                        displayed = TRUE;
                    }
                }
                tcpCtrlMask = 0;
                tcpCtrlData = 0;
                if (thisRule.egress_v6.usefin) {
                    tcpCtrlMask |= BCM_FIELD_TCPCONTROL_FIN;
                }
                if (thisRule.egress_v6.usesyn) {
                    tcpCtrlMask |= BCM_FIELD_TCPCONTROL_SYN;
                }
                if (thisRule.egress_v6.userst) {
                    tcpCtrlMask |= BCM_FIELD_TCPCONTROL_RST;
                }
                if (thisRule.egress_v6.usepsh) {
                    tcpCtrlMask |= BCM_FIELD_TCPCONTROL_PSH;
                }
                if (thisRule.egress_v6.useack) {
                    tcpCtrlMask |= BCM_FIELD_TCPCONTROL_ACK;
                }
                if (thisRule.egress_v6.useurg) {
                    tcpCtrlMask |= BCM_FIELD_TCPCONTROL_URG;
                }
                if (thisRule.egress_v6.fin) {
                    tcpCtrlData |= BCM_FIELD_TCPCONTROL_FIN;
                }
                if (thisRule.egress_v6.syn) {
                    tcpCtrlData |= BCM_FIELD_TCPCONTROL_SYN;
                }
                if (thisRule.egress_v6.rst) {
                    tcpCtrlData |= BCM_FIELD_TCPCONTROL_RST;
                }
                if (thisRule.egress_v6.psh) {
                    tcpCtrlData |= BCM_FIELD_TCPCONTROL_PSH;
                }
                if (thisRule.egress_v6.ack) {
                    tcpCtrlData |= BCM_FIELD_TCPCONTROL_ACK;
                }
                if (thisRule.egress_v6.urg) {
                    tcpCtrlData |= BCM_FIELD_TCPCONTROL_URG;
                }
                if (tcpCtrlMask) {
                    FIELD_PRINT(("%s    %s = %02X/%02X\n",
                                 prefix,
                                 _sbx_fe2000_field_qual_name[bcmFieldQualifyTcpControl],
                                 tcpCtrlData,
                                 tcpCtrlMask));
                    displayed = TRUE;
                }
                break;
   
            default:
                /* already bailed out on this condition, but compiler will gripe */
                break;
            } /* switch (glob->group[thisEntry->group].rulebase) */
            if (!displayed) {
                FIELD_PRINT(("%s    (no qualifiers; rule matches all frames)\n",
                             prefix));
            }
            displayed = FALSE;
            FIELD_PRINT(("%s  Actions:\n", prefix));
            switch (glob->group[thisEntry->group].rulebase) {
            case 0:
                if (thisRule.ingress.copy) {
                    FIELD_PRINT(("%s    %s\n",
                                 prefix,
                                 _sbx_fe2000_field_action_name[bcmFieldActionCopyToCpu]));
                    displayed = TRUE;
                }
                if (thisRule.ingress.usevlan) {
                    FIELD_PRINT(("%s    %s = %04X (%d) (VSI, *not* OuterVID)\n",
                                 prefix,
                                 "(VSI)",
                                 thisRule.ingress.vlan,
                                 thisRule.ingress.vlan));
                    displayed = TRUE;
                }
                if (thisRule.ingress.useftidx) {
                    FIELD_PRINT(("%s    %s = %08X\n",
                                 prefix,
                                 "(FTI)",
                                 thisRule.ingress.ftidx));
                }
                if (thisRule.ingress.mirror) {
                    result = _bcm_fe2000_ingr_mirror_get(glob->unit,
                                                         thisRule.ingress.mirror,
                                                         &gportId);
                    if (BCM_E_NONE == result) {
                        if (BCM_GPORT_IS_MODPORT(gportId)) {
                            modId = BCM_GPORT_MODPORT_MODID_GET(gportId);
                            portId = BCM_GPORT_MODPORT_PORT_GET(gportId);
                            FIELD_PRINT(("%s    %s = %02X (%d) (module %d, port %d)\n",
                                     prefix,
                                     _sbx_fe2000_field_action_name[bcmFieldActionMirrorIngress],
                                     thisRule.ingress.mirror,
                                     thisRule.ingress.mirror,
                                     modId,
                                     portId));
			}
                    } else {
                        FIELD_PRINT(("%s    %s = %02X (%d) (%d (%s) reading port/mod)\n",
                                     prefix,
                                     _sbx_fe2000_field_action_name[bcmFieldActionMirrorIngress],
                                     thisRule.ingress.mirror,
                                     thisRule.ingress.mirror,
                                     result,
                                     _SHR_ERRMSG(result)));
                    }
                    displayed = TRUE;
                }
                if (thisRule.ingress.usedp) {
                    FIELD_PRINT(("%s    %s = %01X\n",
                                 prefix,
                                 _sbx_fe2000_field_action_name[bcmFieldActionDropPrecedence],
                                 thisRule.ingress.dp));
                    displayed = TRUE;
                }
                if (thisRule.ingress.usecos) {
                    FIELD_PRINT(("%s    %s = %01X,%01X\n",
                                 prefix,
                                 _sbx_fe2000_field_action_name[bcmFieldActionCosQNew],
                                 thisRule.ingress.cos,
                                 thisRule.ingress.fcos));
                    displayed = TRUE;
                }
                if (thisRule.ingress.policer) {
                    FIELD_PRINT(("%s    %s = %08X"
                                 " (typed = %s, mef = %s (%d))\n",
                                 prefix,
                                 "(police)",
                                 thisRule.ingress.policer,
                                 thisRule.ingress.typedpolice?"TRUE":"FALSE",
                                 thisRule.ingress.mefcos?"TRUE":"FALSE",
                                 thisRule.ingress.mef));
                    displayed = TRUE;
                }
                break;
            case 1:
                if (thisRule.egress.mirror) {
                    result = _bcm_fe2000_egr_mirror_get(glob->unit,
                                                        thisRule.egress.mirror,
                                                        &gportId);
                    if (BCM_E_NONE == result) {
                        if (BCM_GPORT_IS_MODPORT(gportId)) {
                            modId = BCM_GPORT_MODPORT_MODID_GET(gportId);
                            portId = BCM_GPORT_MODPORT_PORT_GET(gportId);
                            FIELD_PRINT(("%s    %s = %02X (%d) (module %d, port %d)\n",
                                     prefix,
                                     _sbx_fe2000_field_action_name[bcmFieldActionMirrorEgress],
                                     thisRule.egress.mirror,
                                     thisRule.egress.mirror,
                                     modId,
                                     portId));
			}
                    } else {
                        FIELD_PRINT(("%s    %s = %02X (%d) (%d (%s) reading port/mod)\n",
                                     prefix,
                                     _sbx_fe2000_field_action_name[bcmFieldActionMirrorEgress],
                                     thisRule.egress.mirror,
                                     thisRule.egress.mirror,
                                     result,
                                     _SHR_ERRMSG(result)));
                    }
                    displayed = TRUE;
                }
                if (thisRule.egress.drop) {
                    FIELD_PRINT(("%s    %s\n",
                                 prefix,
                                 _sbx_fe2000_field_action_name[bcmFieldActionDrop]));
                    displayed = TRUE;
                }
                break;
            case 2:
                if (thisRule.ingress_v6.copy) {
                    FIELD_PRINT(("%s    %s\n",
                                 prefix,
                                 _sbx_fe2000_field_action_name[bcmFieldActionCopyToCpu]));
                    displayed = TRUE;
                }
                if (thisRule.ingress_v6.usevlan) {
                    FIELD_PRINT(("%s    %s = %04X (%d) (VSI, *not* OuterVID)\n",
                                 prefix,
                                 "(VSI)",
                                 thisRule.ingress_v6.vlan,
                                 thisRule.ingress_v6.vlan));
                    displayed = TRUE;
                }
                if (thisRule.ingress_v6.useftidx) {
                    FIELD_PRINT(("%s    %s = %08X\n",
                                 prefix,
                                 "(FTI)",
                                 thisRule.ingress_v6.ftidx));
                }
                if (thisRule.ingress_v6.mirror) {
                    result = _bcm_fe2000_ingr_mirror_get(glob->unit,
                                                         thisRule.ingress_v6.mirror,
                                                         &gportId);
                    if (BCM_E_NONE == result) {
                        if (BCM_GPORT_IS_MODPORT(gportId)) {
                            modId = BCM_GPORT_MODPORT_MODID_GET(gportId);
                            portId = BCM_GPORT_MODPORT_PORT_GET(gportId);
                            FIELD_PRINT(("%s    %s = %02X (%d) (module %d, port %d)\n",
                                     prefix,
                                     _sbx_fe2000_field_action_name[bcmFieldActionMirrorIngress],
                                     thisRule.ingress_v6.mirror,
                                     thisRule.ingress_v6.mirror,
                                     modId,
                                     portId));
                        }
                    } else {
                        FIELD_PRINT(("%s    %s = %02X (%d) (%d (%s) reading port/mod)\n",
                                     prefix,
                                     _sbx_fe2000_field_action_name[bcmFieldActionMirrorIngress],
                                     thisRule.ingress_v6.mirror,
                                     thisRule.ingress_v6.mirror,
                                     result,
                                     _SHR_ERRMSG(result)));
                    }
                    displayed = TRUE;
                }
                if (thisRule.ingress_v6.usedp) {
                    FIELD_PRINT(("%s    %s = %01X\n",
                                 prefix,
                                 _sbx_fe2000_field_action_name[bcmFieldActionDropPrecedence],
                                 thisRule.ingress_v6.dp));
                    displayed = TRUE;
                }
                if (thisRule.ingress_v6.usecos) {
                    FIELD_PRINT(("%s    %s = %01X,%01X\n",
                                 prefix,
                                 _sbx_fe2000_field_action_name[bcmFieldActionCosQNew],
                                 thisRule.ingress_v6.cos,
                                 thisRule.ingress.fcos));
                    displayed = TRUE;
                }
                if (thisRule.ingress_v6.policer) {
                    FIELD_PRINT(("%s    %s = %08X"
                                 " (typed = %s, mef = %s (%d))\n",
                                 prefix,
                                 "(police)",
                                 thisRule.ingress_v6.policer,
                                 thisRule.ingress_v6.typedpolice?"TRUE":"FALSE",
                                 thisRule.ingress_v6.mefcos?"TRUE":"FALSE",
                                 thisRule.ingress_v6.mef));
                    displayed = TRUE;
                }
                break;
            case 3:
                if (thisRule.egress_v6.mirror) {
                    result = _bcm_fe2000_egr_mirror_get(glob->unit,
                                                        thisRule.egress_v6.mirror,
                                                        &gportId);
                    if (BCM_E_NONE == result) {
                        if (BCM_GPORT_IS_MODPORT(gportId)) {
                            modId = BCM_GPORT_MODPORT_MODID_GET(gportId);
                            portId = BCM_GPORT_MODPORT_PORT_GET(gportId);
                            FIELD_PRINT(("%s    %s = %02X (%d) (module %d, port %d)\n",
                                     prefix,
                                     _sbx_fe2000_field_action_name[bcmFieldActionMirrorEgress],
                                     thisRule.egress_v6.mirror,
                                     thisRule.egress_v6.mirror,
                                     modId,
                                     portId));
                        }
                    } else {
                        FIELD_PRINT(("%s    %s = %02X (%d) (%d (%s) reading port/mod)\n",
                                     prefix,
                                     _sbx_fe2000_field_action_name[bcmFieldActionMirrorEgress],
                                     thisRule.egress_v6.mirror,
                                     thisRule.egress_v6.mirror,
                                     result,
                                     _SHR_ERRMSG(result)));
                    }
                    displayed = TRUE;
                }
                if (thisRule.egress_v6.drop) {
                    FIELD_PRINT(("%s    %s\n",
                                 prefix,
                                 _sbx_fe2000_field_action_name[bcmFieldActionDrop]));
                    displayed = TRUE;
                }
                break;
            default:
                /* already bailed out on this condition, but compiler will gripe */
                break;
            } /* switch (glob->group[thisEntry->group].rulebase) */
            if (!displayed) {
                FIELD_PRINT(("%s    (no actions; rule does not affect frames)\n",
                             prefix));
            }
        } /* if (BCM_E_NONE != tmpRes) */
    } /* if (full) */

    return result;
}

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_entry_dump
 *  Purpose
 *    Dump information about the specified entry to debug output
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *thisUnit = pointer to unit info
 *    (in) _bcm_fe2k_field_entry_index entry = the entry ID
 *    (in) char *prefix = a string to display in front of each line
 *  Returns
 *    int (implied cast from bcm_error_t)
 *                  BCM_E_NONE if successful
 *                  BCM_E_* appropriately if not
 *  Notes
 *    No error checking or locking is done here.
 */
static int
_bcm_fe2k_g2p3_field_entry_dump(_bcm_fe2k_g2p3_field_glob_t *glob,
                                const _bcm_fe2k_field_entry_index entry,
                                const char *prefix)
{
    _bcm_fe2k_g2p3_field_entry_t *thisEntry = &(glob->entry[entry]);
    soc_sbx_g2p3_counter_value_t counters;
    unsigned int column;
    char *rulePrefix = NULL;
    int result = BCM_E_NONE;
    int auxRes;
    int full;

    FIELD_PRINT(("%sUnit %d entry " _SBX_FE2K_FIELD_INDEX_FORMAT
                 " (%d) information:\n",
                 prefix,
                 glob->unit,
                 entry,
                 entry));
    FIELD_PRINT(("%s  Priority = %d\n", prefix, thisEntry->priority));
    FIELD_PRINT(("%s  Group = " _SBX_FE2K_FIELD_INDEX_FORMAT " (%d)\n",
                 prefix,
                 thisEntry->group,
                 thisEntry->group));
    FIELD_PRINT(("%s  Entry flags = %08X\n", prefix, thisEntry->entryFlags));
    if (thisEntry->entryFlags & _FE2K_G2P3_ENTRY_COUNTER) {
        if (thisEntry->entryFlags & _FE2K_G2P3_ENTRY_CTR_ENA) {
            FIELD_PRINT(("%s  Counters:\n", prefix));
            FIELD_PRINT(("%s    Counter mode = %04X\n",
                         prefix,
                         thisEntry->counterMode));
            FIELD_PRINT(("%s    Counter mode specifics:\n", prefix));
            FIELD_PRINT(("%s      Counter type = %s\n",
                         prefix,
                         (BCM_FIELD_COUNTER_MODE_NO_NO == (thisEntry->counterMode & 0x000F))?"NO/NO":
                         (BCM_FIELD_COUNTER_MODE_YES_NO == (thisEntry->counterMode & 0x000F))?"YES/NO":
                         (BCM_FIELD_COUNTER_MODE_NO_YES == (thisEntry->counterMode & 0x000F))?"NO/YES":
                         (BCM_FIELD_COUNTER_MODE_BYTES_PACKETS == (thisEntry->counterMode & 0x000F))?"BYTES/PACKETS":
                         "unknown"));
            FIELD_PRINT(("%s      DP being counted = %s\n",
                         prefix,
                         (BCM_FIELD_COUNTER_MODE_ALL == (thisEntry->counterMode & BCM_FIELD_COUNTER_MODE_ALL))?"ALL":
                         "unknown (subset)"));
            FIELD_PRINT(("%s      Counting mode = %s\n",
                         prefix,
                         (BCM_FIELD_COUNTER_MODE_BYTES_PACKETS == (thisEntry->counterMode & 0x000F))?"Bytes and packets":
                         (BCM_FIELD_COUNTER_MODE_BYTES & thisEntry->counterMode)?"Bytes":"Packets"));
            counters.bytes = 0;
            counters.packets = 0;
            result = _bcm_fe2k_g2p3_field_entries_counter_access(glob,
                                                                 entry,
                                                                 FALSE,
                                                                 &counters);
            if (BCM_E_NONE == result) {
                FIELD_PRINT(("%s    Counter 0 = %08X%08X\n",
                             prefix,
                             COMPILER_64_HI(counters.packets),
                             COMPILER_64_LO(counters.packets)));
                FIELD_PRINT(("%s    Counter 1 = %08X%08X\n",
                             prefix,
                             COMPILER_64_HI(counters.bytes),
                             COMPILER_64_LO(counters.bytes)));
            } else {
                FIELD_PRINT(("%s    [Unable to read entry counters: %d (%s)]\n",
                             prefix,
                             result,
                             _SHR_ERRMSG(result)));
                FIELD_ERR((FIELD_MSG1("unable to read unit %d entry %08X"
                                      " counters: %d (%s)\n"),
                           glob->unit,
                           entry,
                           result,
                           _SHR_ERRMSG(result)));
            }
        } else {
            FIELD_PRINT(("%s  Counters inactive\n", prefix));
        }
    } else {
        FIELD_PRINT(("%s  Counters disabled\n", prefix));
    }
    column = 0;
    FIELD_PRINT(("%s  Emulated actions: (rules show direct mapped)\n", prefix));
    if (thisEntry->entryFlags & _FE2K_G2P3_ENTRY_DROP) {
        FIELD_PRINT(("%s    %s\n",
                     prefix,
                     _sbx_fe2000_field_action_name[bcmFieldActionDrop]));
        column = 1;
    }
    if (thisEntry->entryFlags & _FE2K_G2P3_ENTRY_VLANNEW) {
        FIELD_PRINT(("%s    %s %04X (%d)\n",
                     prefix,
                     _sbx_fe2000_field_action_name[bcmFieldActionVlanNew],
                     thisEntry->ftvlData.VSI,
                     thisEntry->ftvlData.VSI));
        column = 1;
    }
    if (thisEntry->entryFlags & _FE2K_G2P3_ENTRY_L3SWITCH) {
        FIELD_PRINT(("%s    %s to %08X\n",
                     prefix,
                     _sbx_fe2000_field_action_name[bcmFieldActionL3Switch],
                     thisEntry->ftvlData.ftHandle));
        column = 1;
    }
    if (thisEntry->entryFlags & _FE2K_G2P3_ENTRY_REDIRECT) {
        FIELD_PRINT(("%s    %s module %d port %d\n",
                     prefix,
                     _sbx_fe2000_field_action_name[bcmFieldActionRedirect],
                     thisEntry->ftvlData.target.module,
                     thisEntry->ftvlData.target.port));
        column = 1;
    }
    if (thisEntry->entryFlags & _FE2K_G2P3_ENTRY_REDIRMC) {
        FIELD_PRINT(("%s    %s l2McIndex %08X\n",
                     prefix,
                     _sbx_fe2000_field_action_name[bcmFieldActionRedirectMcast],
                     thisEntry->ftvlData.mcHandle));
        column = 1;
    }
    if (!column) {
        FIELD_PRINT(("%s    (none; see rules for all actions)\n", prefix));
    }
    FIELD_PRINT(("%s  Prev/Next entry in group = " _SBX_FE2K_FIELD_INDEX_FORMAT
                 " / " _SBX_FE2K_FIELD_INDEX_FORMAT " (%d / %d)\n",
                 prefix,
                 thisEntry->prevEntry,
                 thisEntry->nextEntry,
                 thisEntry->prevEntry,
                 thisEntry->nextEntry));
    FIELD_PRINT(("%s  Prev/Next entry in rulebase = "
                 _SBX_FE2K_FIELD_INDEX_FORMAT " / "
                 _SBX_FE2K_FIELD_INDEX_FORMAT " (%d / %d)\n",
                 prefix,
                 thisEntry->prevEntryRb,
                 thisEntry->nextEntryRb,
                 thisEntry->prevEntryRb,
                 thisEntry->nextEntryRb));
    rulePrefix = sal_alloc(sal_strlen(prefix) + 6,
                            "entry dump working string");
    if (rulePrefix) {
        sal_snprintf(rulePrefix, sal_strlen(prefix) + 5, "%s    ", prefix);
    }
    FIELD_PRINT(("%s  Rules for this entry:\n", prefix));
    full = TRUE;
    column = 0;
    if (rulePrefix) {
        /* can dump the rule properly */
        if (thisEntry->hwRule) {
            auxRes = _bcm_fe2k_g2p3_field_rule_dump(glob,
                                                    entry,
                                                    rulePrefix,
                                                    full);
            if (BCM_E_NONE != auxRes) {
                result = auxRes;
            } else {
                full = _FIELD_FE2K_G2P3_DUMP_ALL_RULES;
            }
            column = 1;
        }
    } else {
        /* no buffer so just dump rule IDs */
        if (thisEntry->hwRule) {
            if (0 == column) {
                FIELD_PRINT(("%s    %02X:%08X",
                             prefix,
                             glob->group[thisEntry->group].rulebase,
                             (uint32)(thisEntry->hwRule)));
                column = sal_strlen(prefix) + 18;
            }
        }
    }
    if (column) {
        if (!rulePrefix) {
            FIELD_PRINT(("\n"));
        }
    } else {
        FIELD_PRINT(("%s    (none)\n", prefix));
    }
    if (rulePrefix) {
        sal_free(rulePrefix);
        rulePrefix = NULL;
    }
    return result;
}

/*
 *  Function
 *    _bcm_fe2k_g2p3_field_group_dump
 *  Purpose
 *    Dump information about the specified group to debug output
 *  Parameters
 *    (in) _bcm_fe2k_g2p3_field_glob_t *thisUnit = pointer to unit info
 *    (in) _bcm_fe2k_field_group_index group = the group ID
 *    (in) char *prefix = a string to display in front of each line
 *  Returns
 *    int (implied cast from bcm_error_t)
 *                  BCM_E_NONE if successful
 *                  BCM_E_* appropriately if not
 *  Notes
 *    No error checking or locking is done here.
 */
static int
_bcm_fe2k_g2p3_field_group_dump(_bcm_fe2k_g2p3_field_glob_t *glob,
                                const _bcm_fe2k_field_group_index group,
                                const char *prefix)
{
    _bcm_fe2k_g2p3_field_group_t *thisGroup = &(glob->group[group]);
    _bcm_fe2k_field_entry_index entry;
    char *entryPrefix = NULL;
    int auxRes;
    int result = BCM_E_NONE;

    FIELD_PRINT(("%sUnit %d group " _SBX_FE2K_FIELD_INDEX_FORMAT
                 " (%d) information:\n",
                 prefix,
                 glob->unit,
                 group,
                 group));
    FIELD_PRINT(("%s  Priority = %d\n", prefix, thisGroup->priority));
    FIELD_PRINT(("%s  Entries = %d (%d counter shares)\n",
                 prefix,
                 thisGroup->entries,
                 thisGroup->entries - thisGroup->counters));
    FIELD_PRINT(("%s  Rulebase = %d\n",
                 prefix,
                 thisGroup->rulebase));
    FIELD_PRINT(("%s  Prev/Next group = " _SBX_FE2K_FIELD_INDEX_FORMAT
                 " / " _SBX_FE2K_FIELD_INDEX_FORMAT " (%d / %d)\n",
                 prefix,
                 thisGroup->prevGroup,
                 thisGroup->nextGroup,
                 thisGroup->prevGroup,
                 thisGroup->nextGroup));
#if _SBX_FE2K_FIELD_DIAGNOSTIC_MODE
    FIELD_PRINT(("%s  First/Last entry = " _SBX_FE2K_FIELD_INDEX_FORMAT
                 " / " _SBX_FE2K_FIELD_INDEX_FORMAT " (%d / %d)\n",
                 prefix,
                 thisGroup->entryHead,
                 thisGroup->entryTail,
                 thisGroup->entryHead,
                 thisGroup->entryTail));
#endif /* _SBX_FE2K_FIELD_DIAGNOSTIC_MODE */
    entryPrefix = sal_alloc(sal_strlen(prefix) + 6,
                            "group dump working string");
    if (entryPrefix) {
        sal_snprintf(entryPrefix, sal_strlen(prefix) + 5, "%s    ", prefix);
        FIELD_PRINT(("%s  Qualifier set:\n", prefix));
        result = _bcm_fe2000_field_qset_dump(thisGroup->qset,
                                             entryPrefix);
        sal_snprintf(entryPrefix, sal_strlen(prefix) + 3, "%s  ", prefix);
        for (entry = thisGroup->entryHead;
             entry < glob->entryTotal;
             entry = glob->entry[entry].nextEntry) {
            auxRes = _bcm_fe2k_g2p3_field_entry_dump(glob, entry, entryPrefix);
            if (BCM_E_NONE != auxRes) {
                /* keep the error but try to proceed despite it */
                result = auxRes;
            }
        }
        sal_free(entryPrefix);
        entryPrefix = NULL;
    } else { /* if (entryPrefix) */
        FIELD_PRINT(("%s  [Unable to allocate working prefix for qset/entry"
                     " display]\n",
                     prefix));
        FIELD_ERR((FIELD_MSG1("unable to allocate working"
                              " prefix for qset and entry dump\n")));
        result = BCM_E_MEMORY;
    } /* if (entryPrefix) */
    return result;
}

/*
 *   Function
 *      _bcm_fe2k_g2p3_field_rulebase_dump
 *   Purpose
 *      Dump information about the specified rulebase to debug output
 *   Parameters
 *      (in) _bcm_fe2k_g2p3_field_glob_t *thisUnit = pointer to unit info
 *      (in) uint8 rulebase = the rulebase ID
 *      (in) char *prefix = a string to display in front of each line
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No error checking or locking is done here.
 */
static int
_bcm_fe2k_g2p3_field_rulebase_dump(_bcm_fe2k_g2p3_field_glob_t *glob,
                                   const uint8 rulebase,
                                   const char *prefix)
{
    _bcm_fe2k_g2p3_field_rulebase_t *thisRb = &(glob->rulebase[rulebase]);
    _bcm_fe2k_field_group_index group;
    bcm_field_action_t action;
    char *groupPrefix = NULL;
    int result = BCM_E_NONE;
    int tempRes;
    unsigned int column = 0;
#if _SBX_FE2K_FIELD_DIAGNOSTIC_MODE
#if _FIELD_FE2K_G2P3_DUMP_LISTS
    _bcm_fe2k_field_entry_index entry;
#endif /* _FIELD_FE2K_G2P3_DUMP_LISTS */
#endif /* _SBX_FE2K_FIELD_DIAGNOSTIC_MODE */

    FIELD_PRINT(("%sUnit %d rulebase %d information:\n",
                 prefix,
                 glob->unit,
                 rulebase));

    switch (thisRb->dbId) {
    case SB_G2P3_CLS_QOS:
        FIELD_PRINT(("%s  SBX database ID = %s\n",
                     prefix, "SB_G2P3_CLS_QOS"));
        break;
    case SB_G2P3_CLS_EGRESS:
        FIELD_PRINT(("%s  SBX database ID = %s\n",
                     prefix, "SB_G2P3_CLS_EGRESS"));
        break;
    case SB_G2P3_CLS_IPV6_QOS:
        FIELD_PRINT(("%s  SBX database ID = %s\n",
                     prefix, "SB_G2P3_CLS_IPV6_QOS"));
        break;
    case SB_G2P3_CLS_IPV6_EGRESS:
        FIELD_PRINT(("%s  SBX database ID = %s\n",
                     prefix, "SB_G2P3_CLS_IPV6_EGRESS"));
        break;
    default:
        FIELD_PRINT(("%s  SBX database ID = %s\n",
                     prefix, "UNKNOWN"));
        break;
    }
    
    FIELD_PRINT(("%s  Rules = %d (%d max)\n",
                 prefix,
                 thisRb->rules,
                 thisRb->rulesMax));
    FIELD_PRINT(("%s  Entries = %d (%d max; first = "
                 _SBX_FE2K_FIELD_INDEX_FORMAT " (%d))\n",
                 prefix,
                 thisRb->entries,
                 thisRb->entriesMax,
                 thisRb->entryHead,
                 thisRb->entryHead));
    FIELD_PRINT(("%s  Ports = " FIELD_PBMP_FORMAT "\n",
                 prefix,
                 FIELD_PBMP_SHOW(thisRb->ports)));
    groupPrefix = sal_alloc(strlen(prefix) + 6,
                            "rulebase dump working string");
    if (groupPrefix) {
        sal_snprintf(groupPrefix, strlen(prefix) + 5, "%s    ", prefix);
        FIELD_PRINT(("%s  Supported qualifiers:\n", prefix));
        result = _bcm_fe2000_field_qset_dump(thisRb->qset,
                                             groupPrefix);
    } else { /* if (groupPrefix) */
        FIELD_PRINT(("%s  [Unable to allocate working prefix for rulebase"
                     " display]\n",
                     prefix));
        FIELD_ERR((FIELD_MSG1("unable to allocate working"
                              " prefix for rulebase dump\n")));
        result = BCM_E_MEMORY;
    } /* if (groupPrefix) */
    FIELD_PRINT(("%s  Supported actions:\n", prefix));
    for (action = 0; action < bcmFieldActionCount; action++) {
        if (SHR_BITGET(thisRb->action, action)) {
            if (0 == column) {
                /* just starting out */
                FIELD_PRINT(("%s    %s",
                             prefix,
                             _sbx_fe2000_field_action_name[action]));
                column = (4 + sal_strlen(prefix) +
                          sal_strlen(_sbx_fe2000_field_action_name[action]));
            } else if ((2 + column +
                       sal_strlen(_sbx_fe2000_field_action_name[action])) >=
                       _SBX_FE2K_FIELD_PAGE_WIDTH) {
                /* this qualifier would wrap */
                FIELD_PRINT(("\n%s    %s",
                             prefix,
                             _sbx_fe2000_field_action_name[action]));
                column = (4 + sal_strlen(prefix) +
                          sal_strlen(_sbx_fe2000_field_action_name[action]));
            } else {
                /* this qualifier fits on the line */
                FIELD_PRINT((", %s",
                             _sbx_fe2000_field_action_name[action]));
                column += (2 +
                           sal_strlen(_sbx_fe2000_field_action_name[action]));
            }
        } /* if (SHR_BITGET(thisRb->action, action)) */
    } /* for (action = 0; action < bcmFieldActionCount; action++) */
    if (column) {
        FIELD_PRINT(("\n"));
    } else {
        FIELD_PRINT(("%s    (none)\n", prefix));
    }
    FIELD_PRINT(("%s  Unit %d rulebase %d information:\n",
                 prefix,
                 glob->unit,
                 rulebase));
    FIELD_PRINT(("%s    Ports = " FIELD_PBMP_FORMAT "\n",
                 prefix,
                 FIELD_PBMP_SHOW(thisRb->ports)));
    FIELD_PRINT(("%s    Rules = %d (%d max)\n",
                 prefix,
                 thisRb->rules,
                 thisRb->rulesMax));
    if (groupPrefix) {
        sal_snprintf(groupPrefix, strlen(prefix) + 3, "%s  ", prefix);
        for (group = thisRb->groupHead;
             group < glob->groupTotal;
             group = glob->group[group].nextGroup) {
            tempRes = _bcm_fe2k_g2p3_field_group_dump(glob,
                                                      group,
                                                      groupPrefix);
            if (BCM_E_NONE != tempRes) {
                /* keep the error but try to proceed despite it */
                result = tempRes;
            }
        } /* for (all groups in this stage) */
        sal_free(groupPrefix);
        groupPrefix = NULL;
    } /* if (groupPrefix) */
#if _SBX_FE2K_FIELD_DIAGNOSTIC_MODE
#if _FIELD_FE2K_G2P3_DUMP_LISTS
    FIELD_PRINT(("%s  Unit %d rulebase %d entries list:\n",
                 prefix,
                 glob->unit,
                 rulebase));
    column = 0;
    for (entry = thisRb->entryHead;
         entry < glob->entryTotal;
         entry = glob->entry[entry].nextEntryRb) {
        if (0 == column) {
            FIELD_PRINT(("%s    " _SBX_FE2K_FIELD_INDEX_FORMAT,
                         prefix,
                         entry));
            column = sal_strlen(prefix) + 4 + _SBX_FE2K_FIELD_INDEX_WIDTH;
        } else if (column + _SBX_FE2K_FIELD_INDEX_WIDTH + 3 >=
                   _SBX_FE2K_FIELD_PAGE_WIDTH) {
            FIELD_PRINT((",\n%s    " _SBX_FE2K_FIELD_INDEX_FORMAT,
                         prefix,
                         entry));
            column = sal_strlen(prefix) + 4 + _SBX_FE2K_FIELD_INDEX_WIDTH;
        } else {
            FIELD_PRINT((", " _SBX_FE2K_FIELD_INDEX_FORMAT, entry));
            column += (2 + _SBX_FE2K_FIELD_INDEX_WIDTH);
        }
    } /* for (all entries in this stage) */
    if (column) {
        FIELD_PRINT(("\n"));
    } else {
        FIELD_PRINT(("%s    (none)\n", prefix));
    }
#endif /* _FIELD_FE2K_G2P3_DUMP_LISTS */
#endif /* _SBX_FE2K_FIELD_DIAGNOSTIC_MODE */
    return result;
}

/*
 *   Function
 *      _bcm_fe2k_g2p3_field_range_dump
 *   Purpose
 *      Dump information about the specified range to debug output
 *   Parameters
 *      (in) _bcm_fe2k_g2p3_field_glob_t *thisUnit = pointer to unit info
 *      (in) bcm_fe2k_field_range_index range = the range to dump
 *      (in) char *prefix = a string to display in front of each line
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No error checking or locking is done here.
 */
static int
_bcm_fe2k_g2p3_field_range_dump(_bcm_fe2k_g2p3_field_glob_t *glob,
                                const _bcm_fe2k_field_range_index range,
                                const char *prefix)
{
    _bcm_fe2k_field_range_t *thisRange = &(glob->range[range]);

    FIELD_PRINT(("%sUnit %d range " _SBX_FE2K_FIELD_INDEX_FORMAT
                 " (%d) information\n",
                 prefix,
                 glob->unit,
                 range + 1,
                 range + 1));
    FIELD_PRINT(("%s  Flags = %08X:\n", prefix, thisRange->flags));
    if (thisRange->flags & BCM_FIELD_RANGE_TCP) {
        FIELD_PRINT(("%s    BCM_FIELD_RANGE_TCP\n", prefix));
    }
    if (thisRange->flags & BCM_FIELD_RANGE_UDP) {
        FIELD_PRINT(("%s    BCM_FIELD_RANGE_UDP\n", prefix));
    }
    if (thisRange->flags & BCM_FIELD_RANGE_SRCPORT) {
        FIELD_PRINT(("%s    BCM_FIELD_RANGE_SRCPORT\n", prefix));
    }
    if (thisRange->flags & BCM_FIELD_RANGE_DSTPORT) {
        FIELD_PRINT(("%s    BCM_FIELD_RANGE_DSTPORT\n", prefix));
    }
    FIELD_PRINT(("%s  Port range = %04X..%04X (%d..%d)\n",
                 prefix,
                 thisRange->min,
                 thisRange->max,
                 thisRange->min,
                 thisRange->max));
    return BCM_E_NONE;
}

/*
 *   Function
 *      _bcm_fe2k_g2p3_field_dump
 *   Purpose
 *      Dump information about the field subsystem
 *   Parameters
 *      (in) _bcm_fe2k_g2p3_field_glob_t *thisUnit = pointer to unit info
 *      (in) char *prefix = a string to display in front of each line
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No error checking or locking is done here.
 */
static int
_bcm_fe2k_g2p3_field_dump(_bcm_fe2k_g2p3_field_glob_t *glob,
                          const char *prefix)
{
    char *rbPrefix = NULL;
    int result = BCM_E_NONE;
    int tempRes;
    uint8 rulebase;
    uint32 rangeStart;
    uint32 rangeEnd;
    uint32 rangeUsed;
    uint32 rangeFree;
    _bcm_fe2k_field_range_index range;
#if _SBX_FE2K_FIELD_DIAGNOSTIC_MODE
#if _FIELD_FE2K_G2P3_DUMP_LISTS
    unsigned int column;
    _bcm_fe2k_field_group_index group;
    _bcm_fe2k_field_entry_index entry;
#endif /* _FIELD_FE2K_G2P3_DUMP_LISTS */
#endif /* _SBX_FE2K_FIELD_DIAGNOSTIC_MODE */

    FIELD_PRINT(("%sUnit %d field subsystem information\n",
                 prefix,
                 glob->unit));
    result = shr_idxres_list_state(glob->rangeFree,
                                   &rangeStart,
                                   &rangeEnd,
                                   NULL,
                                   NULL,
                                   &rangeFree,
                                   &rangeUsed);
    if (BCM_E_NONE == result) {
        FIELD_PRINT(("%s  Ranges = %d (%d max)\n",
                     prefix,
                     rangeUsed,
                     rangeUsed + rangeFree));
    } else { /* if (BCM_E_NONE == result) */
        FIELD_PRINT(("%s  [Unable to get range information: %d (%s)]\n",
                     prefix,
                     result,
                     _SHR_ERRMSG(result)));
        FIELD_ERR((FIELD_MSG1("unable to read unit %d range information:"
                              " %d (%s)\n"),
                   glob->unit,
                   result,
                   _SHR_ERRMSG(result)));
    } /* if (BCM_E_NONE == result) */
    FIELD_PRINT(("%s  Groups = %d (%d max, next alloc "
                 _SBX_FE2K_FIELD_INDEX_FORMAT " (%d))\n",
                 prefix,
                 glob->groupTotal - glob->groupFreeCount,
                 glob->groupTotal,
                 glob->groupFreeHead,
                 glob->groupFreeHead));
    FIELD_PRINT(("%s  Entries = %d (%d max, next alloc "
                 _SBX_FE2K_FIELD_INDEX_FORMAT " (%d))\n",
                 prefix,
                 glob->entryTotal - glob->entryFreeCount,
                 glob->entryTotal,
                 glob->entryFreeHead,
                 glob->entryFreeHead));
    rbPrefix = sal_alloc(strlen(prefix) + 4, "rulebase dump working string");
    if (rbPrefix) {
        sal_snprintf(rbPrefix, strlen(prefix) + 3, "%s  ", prefix);
        for (rulebase = 0; rulebase < glob->uMaxSupportedStages; rulebase++) {
            tempRes = _bcm_fe2k_g2p3_field_rulebase_dump(glob,
                                                         rulebase,
                                                         rbPrefix);
            if (BCM_E_NONE != tempRes) {
                /* keep the error but try to proceed despite it */
                result = tempRes;
            }
        } /* for (all rulebases) */
        for (range = rangeStart; range < rangeEnd; range++) {
            if (BCM_E_EXISTS == shr_idxres_list_elem_state(glob->rangeFree,
                                                           range)) {
                tempRes = _bcm_fe2k_g2p3_field_range_dump(glob,
                                                          range,
                                                          rbPrefix);
                if (BCM_E_NONE != tempRes) {
                    /* keep the error but try to proceed despite it */
                    result = tempRes;
                }
            }
        } /* for (all ranges) */
        sal_free(rbPrefix);
        rbPrefix = NULL;
    } else { /* if (rbPrefix) */
        FIELD_PRINT(("%s  [Unable to allocate working prefix for entry/range"
                     " display]\n",
                     prefix));
        FIELD_ERR((FIELD_MSG1("unable to allocate working"
                              " prefix for entry/range dump\n")));
        result = BCM_E_MEMORY;
    } /* if (rbPrefix) */
#if _SBX_FE2K_FIELD_DIAGNOSTIC_MODE
#if _FIELD_FE2K_G2P3_DUMP_LISTS
    FIELD_PRINT(("%s  Unit %d free groups list:\n", prefix, glob->unit));
    column = 0;
    for (group = glob->groupFreeHead;
         group < glob->groupTotal;
         group = glob->group[group].nextGroup) {
        if (0 == column) {
            FIELD_PRINT(("%s    " _SBX_FE2K_FIELD_INDEX_FORMAT,
                         prefix,
                         group));
            column = sal_strlen(prefix) + 4 + _SBX_FE2K_FIELD_INDEX_WIDTH;
        } else if (column + _SBX_FE2K_FIELD_INDEX_WIDTH + 3 >=
                   _SBX_FE2K_FIELD_PAGE_WIDTH) {
            FIELD_PRINT((",\n%s    " _SBX_FE2K_FIELD_INDEX_FORMAT,
                         prefix,
                         group));
            column = sal_strlen(prefix) + 4 + _SBX_FE2K_FIELD_INDEX_WIDTH;
        } else {
            FIELD_PRINT((", " _SBX_FE2K_FIELD_INDEX_FORMAT, group));
            column += (2 + _SBX_FE2K_FIELD_INDEX_WIDTH);
        }
    } /* for (all free groups) */
    if (column) {
        FIELD_PRINT(("\n"));
    } else {
        FIELD_PRINT(("%s    (none)\n", prefix));
    }
    FIELD_PRINT(("%s  Unit %d free entries list:\n", prefix, glob->unit));
    column = 0;
    for (entry = glob->entryFreeHead;
         entry < glob->entryTotal;
         entry = glob->entry[entry].nextEntry) {
        if (0 == column) {
            FIELD_PRINT(("%s    " _SBX_FE2K_FIELD_INDEX_FORMAT,
                         prefix,
                         entry));
            column = sal_strlen(prefix) + 4 + _SBX_FE2K_FIELD_INDEX_WIDTH;
        } else if (column + _SBX_FE2K_FIELD_INDEX_WIDTH + 3 >=
                   _SBX_FE2K_FIELD_PAGE_WIDTH) {
            FIELD_PRINT((",\n%s    " _SBX_FE2K_FIELD_INDEX_FORMAT,
                         prefix,
                         entry));
            column = sal_strlen(prefix) + 4 + _SBX_FE2K_FIELD_INDEX_WIDTH;
        } else {
            FIELD_PRINT((", " _SBX_FE2K_FIELD_INDEX_FORMAT, entry));
            column += (2 + _SBX_FE2K_FIELD_INDEX_WIDTH);
        }
    } /* for (all free entries) */
    if (column) {
        FIELD_PRINT(("\n"));
    } else {
        FIELD_PRINT(("%s    (none)\n", prefix));
    }
#endif /* _FIELD_FE2K_G2P3_DUMP_LISTS */
#endif /* _SBX_FE2K_FIELD_DIAGNOSTIC_MODE */
    return result;
}

#endif /* def BROADCOM_DEBUG */

#ifdef BCM_WARM_BOOT_SUPPORT
int
_bcm_field_wb_layout_get(int unit, fe2k_field_wb_mem_layout_t **layout)
{
    int rv;
    uint32 size;
    soc_wb_cache_t *wbc;

    *layout = NULL;
    rv = soc_scache_ptr_get(unit, _field_cache[unit].wb_hdl, 
                            (uint8**)&wbc, &size);
    if (BCM_FAILURE(rv)) {
        return rv; 
    }
    *layout = (fe2k_field_wb_mem_layout_t *)wbc->cache;
    return rv;
}

static int 
_bcm_fe2000_g2p3_field_scache_store(_bcm_fe2k_g2p3_field_glob_t *thisUnit, 
                                    unsigned int rulebaseId){
    uint32 i, bStore, count, r;
    fe2k_field_wb_mem_layout_t *layout;
    soc_sbx_g2p3_field_wb_rulebase_t *rulebase;
    soc_sbx_g2p3_field_wb_group_t *group;
    soc_sbx_g2p3_field_wb_entry_t *entry;

    BCM_IF_ERROR_RETURN(
        _bcm_field_wb_layout_get(thisUnit->unit, &layout));

    soc_scache_handle_lock(thisUnit->unit, _field_cache[thisUnit->unit].wb_hdl);

    rulebase = layout->rulebase;
    group = layout->group;
    entry = &(layout->entry[0]);

    /* store rulebase info */
    if (rulebaseId == _FIELD_FE2K_INVALID_RULEBASE) {
        for (i=0; i<_FIELD_FE2K_G2P3_STAGES; i++) {
            rulebase[i].entryHead = thisUnit->rulebase[i].entryHead;
            rulebase[i].groupHead = thisUnit->rulebase[i].groupHead;
            rulebase[i].entries = thisUnit->rulebase[i].entries;
        }
    }else{
        rulebase[rulebaseId].entryHead = thisUnit->rulebase[rulebaseId].entryHead;
        rulebase[rulebaseId].groupHead = thisUnit->rulebase[rulebaseId].groupHead;
        rulebase[rulebaseId].entries = thisUnit->rulebase[rulebaseId].entries;
    }

    /* store group information for this rulebase */
    for (i=0; i<_FIELD_FE2K_G2P3_MAX_GROUPS; i++) {
        if ( (rulebaseId == _FIELD_FE2K_INVALID_RULEBASE) ||
              (thisUnit->group[i].rulebase == rulebaseId) ||
              (thisUnit->group[i].rulebase == _FIELD_FE2K_INVALID_RULEBASE) ) {
            group[i].qset = thisUnit->group[i].qset;
            group[i].priority = thisUnit->group[i].priority;
            group[i].rulebase = thisUnit->group[i].rulebase;
            group[i].nextGroup = thisUnit->group[i].nextGroup;
            group[i].prevGroup = thisUnit->group[i].prevGroup;
        }

    }

    /* store entry information for this rulebase */
    count = 0;
    for (i=0; i<thisUnit->entryTotal; i++) {
        bStore = 0;
        /* Store all entries associated with the targeted rulebase.  During scache
         * init store all invalid entries.
         */
        if (thisUnit->entry[i].group == thisUnit->groupTotal) {
            if (rulebaseId == _FIELD_FE2K_INVALID_RULEBASE) {
                bStore = 1;
            }
        }else if ( (thisUnit->group[thisUnit->entry[i].group].rulebase < thisUnit->uMaxSupportedStages) &&
                   (thisUnit->group[thisUnit->entry[i].group].rulebase == rulebaseId) ) {
            bStore = 1;
        }

        if (bStore) {
            count++;
            entry[i].priority = thisUnit->entry[i].priority;
            for (r=0; r<_FIELD_FE2K_G2P3_RANGES_PER_ENTRY; r++) {
                entry[i].range[r]= thisUnit->entry[i].range[r];
            }
            entry[i].group = thisUnit->entry[i].group;
            entry[i].counterMode = thisUnit->entry[i].counterMode;
            entry[i].nextEntryRb = thisUnit->entry[i].nextEntryRb;
            entry[i].entryFlags = thisUnit->entry[i].entryFlags;
            entry[i].module = thisUnit->entry[i].ftvlData.target.module;
            entry[i].port = thisUnit->entry[i].ftvlData.target.port;
            entry[i].policer = thisUnit->entry[i].policer;
        }
    }

    soc_scache_handle_unlock(thisUnit->unit, _field_cache[thisUnit->unit].wb_hdl);

    return BCM_E_NONE;
}

int
_bcm_fe2000_g2p3_field_scache_recover(_bcm_fe2k_g2p3_field_glob_t *thisUnit){
    uint32 i, e, result, entries, r;
    fe2k_field_wb_mem_layout_t *layout;
    soc_sbx_g2p3_field_wb_rulebase_t *rulebase;
    soc_sbx_g2p3_field_wb_group_t *group;
    soc_sbx_g2p3_field_wb_entry_t *entry;
    void *hwRule;
    uint16_t curEntry;
    _bcm_fe2k_g2p3_hardware_rule_t rule;
    bcm_field_range_t tempRange;
    bcm_l4_port_t srcMin=0, srcMax=0, dstMin=0, dstMax=0;
    uint32 srcFlags=0, dstFlags=0, discards=0;

    BCM_IF_ERROR_RETURN(
        _bcm_field_wb_layout_get(thisUnit->unit, &layout));

    soc_scache_handle_lock(thisUnit->unit, _field_cache[thisUnit->unit].wb_hdl);

    rulebase = layout->rulebase;
    group = layout->group;
    entry = &(layout->entry[0]);

    /* recover group information */
    for (i=0; i<_FIELD_FE2K_G2P3_MAX_GROUPS; i++){
        if (group[i].rulebase < thisUnit->uMaxSupportedStages) {
            /* fill-in: qset, priority, rulebase and call bcm_fe2000_g2p3_field_group_create_id() */
            result = bcm_fe2000_g2p3_field_group_create_id(thisUnit,
                                                       group[i].qset,
                                                       group[i].priority,
                                                       i);
            if (BCM_E_NONE != result) {
                soc_scache_handle_unlock(thisUnit->unit, _field_cache[thisUnit->unit].wb_hdl);
                FIELD_ERR((FIELD_MSG1("unable to recover unit 0x%x group %08x: %d (%s)\n"),
                           thisUnit->unit, i, result, _SHR_ERRMSG(result)));
                return result;
            }
        }
    }

    /*
     * The linkages between groups is determined by the order and priority the groups are added.
     * Since the groups may not have been re-added in identical order, the linkages may not be
     * correct at this point.  Restore linkage from scache info.
     */
    for (i=0; i<_FIELD_FE2K_G2P3_MAX_GROUPS; i++){
        thisUnit->group[i].nextGroup = group[i].nextGroup;
        thisUnit->group[i].prevGroup = group[i].prevGroup;
    }

    /* recover entries for each rulebase */
    for (i=0; i<_FIELD_FE2K_G2P3_STAGES; i++) {
        if (rulebase[i].entries == 0){
            continue;
        }

        curEntry = rulebase[i].entryHead;
        switch (i) {
        case 0:
            result = soc_sbx_g2p3_ifp_handle_head_get(thisUnit->unit,
                                                      &hwRule);
            break;
        case 1:
            result = soc_sbx_g2p3_efp_handle_head_get(thisUnit->unit,
                                                      &hwRule);
            break;

        case 2:
            result = soc_sbx_g2p3_ifp_v6_handle_head_get(thisUnit->unit,
                                                      hwRule);
            break;
        case 3:
            result = soc_sbx_g2p3_efp_v6_handle_head_get(thisUnit->unit,
                                                      hwRule);
            break;
        default:
            FIELD_ERR((FIELD_MSG1("unknown rulebase %d\n"),
                       rulebase));
            soc_scache_handle_unlock(thisUnit->unit, _field_cache[thisUnit->unit].wb_hdl);
            return BCM_E_INTERNAL;

        }
        if (result != BCM_E_NONE) {
            FIELD_ERR((FIELD_MSG1("unable to recover handle unit %d entry %08X:"
                                  " %d (%s); aborting\n"),
                       thisUnit->unit,
                       curEntry,
                       result,
                       _SHR_ERRMSG(result)));
            soc_scache_handle_unlock(thisUnit->unit, _field_cache[thisUnit->unit].wb_hdl);
            return result;

        }
        entries = rulebase[i].entries;
        for (e=0; e<entries; e++) {

            if ( !(curEntry < thisUnit->entryTotal) ) {
            assert(curEntry < thisUnit->entryTotal);
            }
            sal_memset(&rule, 0, sizeof(_bcm_fe2k_g2p3_hardware_rule_t));

            if (entry[curEntry].entryFlags & _FE2K_G2P3_ENTRY_IN_HW) {
                /* recover rule from SOC layer */
                switch (i) {
                case 0:
                    result = soc_sbx_g2p3_ifp_rule_from_handle(thisUnit->unit,
                                                               hwRule,
                                                               &(rule.ingress));
                    rule.ingress.enable = TRUE;
                    srcMin = rule.ingress.sportlo;
                    srcMax = rule.ingress.sporthi;
                    dstMin = rule.ingress.dportlo;
                    dstMax = rule.ingress.dporthi;
                    srcFlags = BCM_FIELD_RANGE_SRCPORT;
                    dstFlags = BCM_FIELD_RANGE_DSTPORT;
                    if ((rule.ingress.proto == 0x06) &&
                        (rule.ingress.useproto == TRUE) ) {
                        srcFlags |= BCM_FIELD_RANGE_TCP;
                        dstFlags |= BCM_FIELD_RANGE_TCP;
                    }
                    if ((rule.ingress.proto == 0x11) &&
                        (rule.ingress.useproto == TRUE) ) {
                        srcFlags |= BCM_FIELD_RANGE_UDP;
                        dstFlags |= BCM_FIELD_RANGE_UDP;
                    }
                    if (!rule.ingress.useport) {
                        FIELD_PBMP_TO_SBX(rule.ingress.pbmpn, PBMP_PORT_ALL(thisUnit->unit));
                    }
                    break;
                case 1:
                    result = soc_sbx_g2p3_efp_rule_from_handle(thisUnit->unit,
                                                               hwRule,
                                                               &(rule.egress));
                    rule.egress.enable = TRUE;
                    srcMin = rule.egress.sportlo;
                    srcMax = rule.egress.sporthi;
                    dstMin = rule.egress.dportlo;
                    dstMax = rule.egress.dporthi;
                    srcFlags = BCM_FIELD_RANGE_SRCPORT;
                    dstFlags = BCM_FIELD_RANGE_DSTPORT;
                    if ((rule.egress.proto == 0x06) &&
                        (rule.egress.useproto == TRUE) ) {
                        srcFlags |= BCM_FIELD_RANGE_TCP;
                        dstFlags |= BCM_FIELD_RANGE_TCP;
                    }
                    if ((rule.egress.proto == 0x11) &&
                        (rule.egress.useproto == TRUE) ) {
                        srcFlags |= BCM_FIELD_RANGE_UDP;
                        dstFlags |= BCM_FIELD_RANGE_UDP;
                    }
                    if (!rule.egress.useport) {
                        FIELD_PBMP_TO_SBX(rule.egress.pbmpn, PBMP_PORT_ALL(thisUnit->unit));
                    }
                    break;
                case 2:
                    result = soc_sbx_g2p3_ifp_v6_rule_from_handle(thisUnit->unit,
                                                               hwRule,
                                                               &(rule.ingress_v6));
                    rule.ingress_v6.enable = TRUE;
                    srcMin = rule.ingress_v6.sportlo;
                    srcMax = rule.ingress_v6.sporthi;
                    dstMin = rule.ingress_v6.dportlo;
                    dstMax = rule.ingress_v6.dporthi;
                    srcFlags = BCM_FIELD_RANGE_SRCPORT;
                    dstFlags = BCM_FIELD_RANGE_DSTPORT;
                    /* TODO: v6 */
                    if (!rule.ingress_v6.useport) {
                        FIELD_PBMP_TO_SBX(rule.ingress_v6.pbmpn, PBMP_PORT_ALL(thisUnit->unit));
                    }
                    break;
                case 3:
                    result = soc_sbx_g2p3_efp_v6_rule_from_handle(thisUnit->unit,
                                                               hwRule,
                                                               &(rule.egress_v6));
                    rule.egress_v6.enable = TRUE;
                    /* TODO: v6 */
                    srcMin = rule.egress_v6.sportlo;
                    srcMax = rule.egress_v6.sporthi;
                    dstMin = rule.egress_v6.dportlo;
                    dstMax = rule.egress_v6.dporthi;
                    srcFlags = BCM_FIELD_RANGE_SRCPORT;
                    dstFlags = BCM_FIELD_RANGE_DSTPORT;
                    if (!rule.egress_v6.useport) {
                        FIELD_PBMP_TO_SBX(rule.egress_v6.pbmpn, PBMP_PORT_ALL(thisUnit->unit));
                    }
                    break;
                default:
                    FIELD_ERR((FIELD_MSG1("invalid rulebase %d for unit %d"
                                          " entry %08X (group %08X)\n"),
                               i,
                               thisUnit->unit,
                               curEntry,
                               entry[curEntry].group));
                    soc_scache_handle_unlock(thisUnit->unit, _field_cache[thisUnit->unit].wb_hdl);
                    return BCM_E_INTERNAL;
                } /* switch (i) */
                if (BCM_E_NONE != result) {
                    FIELD_ERR((FIELD_MSG1("unable to recover handle unit %d group %08X"
                                          " entry %08X rule (rb %02X"
                                          " rule %08X): %d (%s)\n"),
                               thisUnit->unit,
                               entry[curEntry].group,
                               curEntry,
                               thisUnit->rulebase[i].dbId,
                               (uint32)(hwRule),
                               result,
                               _SHR_ERRMSG(result)));
                    soc_scache_handle_unlock(thisUnit->unit, _field_cache[thisUnit->unit].wb_hdl);
                    return result;
                }
                result = _bcm_fe2k_g2p3_field_entry_alloc_id(thisUnit, curEntry);
                if (BCM_E_NONE != result) {
                    FIELD_ERR((FIELD_MSG1("unable to reallocate unit %d entry %08X:"
                                          " %d (%s); aborting\n"),
                               thisUnit->unit,
                               curEntry,
                               result,
                               _SHR_ERRMSG(result)));
                    soc_scache_handle_unlock(thisUnit->unit, _field_cache[thisUnit->unit].wb_hdl);
                    return result;
                }

            }else{
                /* this rule not committed to HW at time of warmboot, unrecoverable */
                discards++;

            }

            /* only recover entries which were committed to hardware */
            if (entry[curEntry].entryFlags & _FE2K_G2P3_ENTRY_IN_HW) {
                /* set entry flags, hwRule, priority */
                thisUnit->entry[curEntry].entryFlags = entry[curEntry].entryFlags;
                thisUnit->entry[curEntry].priority = entry[curEntry].priority;
                thisUnit->entry[curEntry].group = entry[curEntry].group;
                thisUnit->entry[curEntry].ftvlData.target.module = entry[curEntry].module;
                thisUnit->entry[curEntry].ftvlData.target.port = entry[curEntry].port;
                thisUnit->entry[curEntry].policer = entry[curEntry].policer;
                thisUnit->entry[curEntry].hwRule = hwRule;
                for (r=0; r<_FIELD_FE2K_G2P3_RANGES_PER_ENTRY; r++) {
                    thisUnit->entry[curEntry].range[r] = entry[curEntry].range[r];

                    if (thisUnit->entry[curEntry].range[r]) {
                        /* recover range using info from rule */
                        tempRange = thisUnit->entry[curEntry].range[r] - 1;
                        result = shr_idxres_list_elem_state(thisUnit->rangeFree, tempRange);
                        if (result == BCM_E_EXISTS) {
                            /* TODO: WB_CLS verify range contents match expected */
                        }
                        if (result == BCM_E_NOT_FOUND){
                            /* create the range entry */
                            if (r == 0) {
                                result = bcm_fe2000_g2p3_field_range_create_id(thisUnit,
                                                                                tempRange+1,
                                                                                srcFlags,
                                                                                srcMin,
                                                                                srcMax);
                            }else{
                                result = bcm_fe2000_g2p3_field_range_create_id(thisUnit,
                                                                                tempRange+1,
                                                                                dstFlags,
                                                                                dstMin,
                                                                                dstMax);
                            }
                            if (BCM_E_NONE != result) {
                                /* something went wrong; destroy the range */
                                FIELD_ERR((FIELD_MSG1("error recovring unit %d range %08X,"
                                                      " destroying it\n"),
                                           thisUnit->unit,
                                           tempRange));
                                sal_memset(&(thisUnit->range[tempRange]),
                                           0,
                                           sizeof(thisUnit->range[tempRange]));
                                shr_idxres_list_free(thisUnit->rangeFree, tempRange);
                                return result;
                            }
                        }
                    }
                }
                result = _bcm_fe2000_g2p3_field_action_wb_recover(thisUnit,
                                                                 i,
                                                                 curEntry,
                                                                 &rule);
                if (BCM_E_NONE != result) {
                    FIELD_ERR((FIELD_MSG1("unable to recover entry, unit %d entry %08X:"
                                          " %d (%s); aborting\n"),
                               thisUnit->unit,
                               curEntry,
                               result,
                               _SHR_ERRMSG(result)));
                    soc_scache_handle_unlock(thisUnit->unit, _field_cache[thisUnit->unit].wb_hdl);
                    return result;
                }

                result = _bcm_fe2k_g2p3_field_entry_add(thisUnit, entry[curEntry].group, curEntry, &rule);
                if (BCM_E_NONE != result) {
                    FIELD_ERR((FIELD_MSG1("unable to add entry, unit %d entry %08X:"
                                          " %d (%s); aborting\n"),
                               thisUnit->unit,
                               curEntry,
                               result,
                               _SHR_ERRMSG(result)));
                    soc_scache_handle_unlock(thisUnit->unit, _field_cache[thisUnit->unit].wb_hdl);
                    return result;
                }
            }else{
                FIELD_WARN((FIELD_MSG1("unable to recover entry, unit %d entry %08X: not committed to hardware\n"),
                           thisUnit->unit,
                           curEntry));
            }

            if ( entry[curEntry].nextEntryRb < thisUnit->entryTotal) {
                /* next hwRule */
                switch (i) {
                case 0:
                    result = soc_sbx_g2p3_ifp_handle_next_get(thisUnit->unit,
                                                               hwRule,
                                                               &hwRule);
                    break;
                case 1:
                    result = soc_sbx_g2p3_efp_handle_next_get(thisUnit->unit,
                                                               hwRule,
                                                               &hwRule);
                    break;
                case 2:
                    result = soc_sbx_g2p3_ifp_v6_rule_from_handle(thisUnit->unit,
                                                               hwRule,
                                                               &(rule.ingress_v6));
                    break;
                case 3:
                    result = soc_sbx_g2p3_efp_v6_rule_from_handle(thisUnit->unit,
                                                               hwRule,
                                                               &(rule.egress_v6));
                    break;
                default:
                    FIELD_ERR((FIELD_MSG1("invalid rulebase %d for unit %d"
                                          " entry %08X (group %08X)\n"),
                               i,
                               thisUnit->unit,
                               curEntry,
                               entry[curEntry].group));
                    soc_scache_handle_unlock(thisUnit->unit, _field_cache[thisUnit->unit].wb_hdl);
                    return BCM_E_INTERNAL;
                } /* switch (i) */
                if (BCM_E_NONE != result) {
                    FIELD_ERR((FIELD_MSG1("unable to get next handle unit %d group %08X"
                                          " entry %08X rule (rb %02X"
                                          " rule %08X): %d (%s)\n"),
                               thisUnit->unit,
                               entry[curEntry].group,
                               curEntry,
                               thisUnit->rulebase[i].dbId,
                               (uint32)(hwRule),
                               result,
                               _SHR_ERRMSG(result)));
                    soc_scache_handle_unlock(thisUnit->unit, _field_cache[thisUnit->unit].wb_hdl);
                    return result;
                }
            }

            /* next entry */
            curEntry = entry[curEntry].nextEntryRb;
        }
    }

    soc_scache_handle_unlock(thisUnit->unit, _field_cache[thisUnit->unit].wb_hdl);

    return BCM_E_NONE;
}

#endif
/*****************************************************************************
 *
 *  External interface
 *
 */

/*
 *   Function
 *      bcm_fe2000_g2p3_field_init
 *   Purpose
 *      Initialise the field APIs.
 *   Parameters
 *      (in) int unit = the unit number
 *      (out) void** unitData = where to put the pointer to the unit data
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_g2p3_field_init(int unit, void **unitData)
{
    int result = BCM_E_NONE;                  /* working result */
    int tmpRes;                               /* working result */
    unsigned int groupsTotal = _FIELD_FE2K_G2P3_MAX_GROUPS; /* max supported groups */
    unsigned int rangesTotal;                 /* ranges total */
    unsigned int entriesTotal;                /* total entries supported */
    unsigned int entriesIngress;              /* ingress entries supported */
    unsigned int entriesEgress;               /* egress entries supported */
    unsigned int rulesTotal;                  /* total rules supported */
    unsigned int rulesIngress = 0;            /* rules ingress */
    unsigned int rulesEgress = 0;             /* rules egress */
    int i;                                    /* working indices */
    uint32 excId;                             /* exception ID for copyToCpu */
    soc_sbx_g2p3_xt_t copyToCpuExc;           /* copyToCpu exception data */
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */
#ifdef BCM_WARM_BOOT_SUPPORT
    uint32 size, scache_size;
    soc_wb_cache_t *wbc;
#endif

    FIELD_VVERB((FIELD_MSG("(%d,*) enter\n"), unit));
    /*
     *  Find out how many rules are allowed, for ingress and for
     *  egress.  These are the bases for the sizing calculation.
     */
    FIELD_VVERB((FIELD_MSG1("probing unit %d...\n"), unit));
    result = soc_sbx_g2p3_ifp_max_rule_capacity_get(unit, &rulesIngress);
    if (BCM_E_NONE == result) {
        rulesIngress--; /* compensate for sentinel rule */
        FIELD_VVERB((FIELD_MSG1("Unit %d ingress rules = %d\n"),
                     unit,
                     rulesIngress));
    } else { /* if (BCM_E_NONE == result) */
        FIELD_ERR((FIELD_MSG1("unable to get unit %d ingress rules:"
                              " %d (%s)\n"),
                   unit,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    } /* if (BCM_E_NONE == result) */
    result = soc_sbx_g2p3_efp_max_rule_capacity_get(unit, &rulesEgress);
    if (BCM_E_NONE == result) {
        rulesEgress--; /* compensate for sentinel rule */
        FIELD_VVERB((FIELD_MSG1("Unit %d egress rules = %d\n"),
                     unit,
                     rulesEgress));
    } else { /* if (BCM_E_NONE == result) */
        FIELD_ERR((FIELD_MSG1("unable to get unit %d egress rules:"
                              " %d (%s)\n"),
                   unit,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    } /* if (BCM_E_NONE == result) */

    if (SOC_IS_SBX_FE2KXT(unit)) {
        /* No demarkation between ingress and egress rules */
        rulesTotal = rulesIngress;
    } else {
        rulesTotal = rulesEgress + rulesIngress;
    }
    entriesTotal = rulesTotal;
    entriesIngress = rulesIngress;
    entriesEgress = rulesEgress;
    rangesTotal = (rulesTotal + 3) >> 2;

    /* dump the probe results as diagnostics */
    FIELD_VVERB((FIELD_MSG1("unit %d supports %d ingress rules (probed)\n"),
                 unit,
                 rulesIngress));
    FIELD_VVERB((FIELD_MSG1("unit %d supports %d egress rules (probed)\n"),
                 unit,
                 rulesEgress));
    FIELD_VVERB((FIELD_MSG1("unit %d supports %d entries (computed)\n"),
                 unit,
                 entriesTotal));
    FIELD_VVERB((FIELD_MSG1("unit %d supports %d ranges (computed)\n"),
                 unit,
                 rangesTotal));
    FIELD_VVERB((FIELD_MSG1("unit %d supports %d groups (configured)\n"),
                 unit,
                 groupsTotal));

    /* begin real initialisation based upon the probe results */
    FIELD_VVERB((FIELD_MSG1("unit %d probe complete; initialising...\n"),
                 unit));

    /* allocate private description for the unit */
    FIELD_EVERB((FIELD_MSG1("allocate unit description (%d bytes)\n"),
                 sizeof(_bcm_fe2k_g2p3_field_glob_t)));
    thisUnit = sal_alloc(sizeof(_bcm_fe2k_g2p3_field_glob_t),
                         "fe2000_g2p3_unit_field_data");
    if (!thisUnit) {
        FIELD_ERR((FIELD_MSG1("unable to allocate unit %d description\n"),
                   unit));
        return BCM_E_MEMORY;
    }

    /* clear out the structure */
    sal_memset(thisUnit, 0x00, sizeof(_bcm_fe2k_g2p3_field_glob_t));

    /* keep unit number around; we'll be using it later */
    thisUnit->unit = unit;

    /* Set the maximum supported stages */
    thisUnit->uMaxSupportedStages = soc_sbx_g2p3_max_supported_stages(unit);

    /* allocate index space */
    FIELD_EVERB((FIELD_MSG1("allocate unit %d index space (%d bytes)\n"),
                 unit,
                 ((sizeof(_bcm_fe2k_g2p3_field_group_t) * groupsTotal) +
                  (sizeof(_bcm_fe2k_g2p3_field_entry_t) * entriesTotal) +
                  (sizeof(_bcm_fe2k_field_range_t) * rangesTotal))));
    thisUnit->group = sal_alloc(((sizeof(_bcm_fe2k_g2p3_field_group_t) *
                                  groupsTotal) +
                                 (sizeof(_bcm_fe2k_g2p3_field_entry_t) *
                                  entriesTotal) +
                                 (sizeof(_bcm_fe2k_field_range_t) *
                                  rangesTotal)),
                                "_bcm_fe2k_field_unit_indices");
    if (thisUnit->group) {
        /* clear the index space */
        sal_memset(thisUnit->group,
                   0,
                   ((sizeof(_bcm_fe2k_g2p3_field_group_t) * groupsTotal) +
                    (sizeof(_bcm_fe2k_g2p3_field_entry_t) * entriesTotal) +
                    (sizeof(_bcm_fe2k_field_range_t) * rangesTotal)));
        /* split the index space */
        FIELD_EVERB((FIELD_MSG1("split unit %d index space\n"), unit));
        thisUnit->entry = ((_bcm_fe2k_g2p3_field_entry_t *)
                           (&(thisUnit->group[groupsTotal])));
        thisUnit->range = ((_bcm_fe2k_field_range_t *)
                           (&(thisUnit->entry[entriesTotal])));
        FIELD_EVERB((FIELD_MSG1("unit %3d index space: group = %08X\n"),
                     unit,
                     (uint32)(thisUnit->group)));
        FIELD_EVERB((FIELD_MSG1("                      entry = %08X\n"),
                     (uint32)(thisUnit->entry)));
        FIELD_EVERB((FIELD_MSG1("                      range = %08X\n"),
                     (uint32)(thisUnit->range)));
    } else { /* if (thisUnit->group) */
        /* sal_alloc failed if we're here */
        result = BCM_E_MEMORY;
        FIELD_ERR((FIELD_MSG1("failed to allocate unit %d index space\n"),
                   unit));
    } /* if (thisUnit->group) */

    /* initialise entry and group and range data and create free lists */
    if (BCM_E_NONE == result) {
        /* initialise rules */
        FIELD_EVERB((FIELD_MSG1("init unit %d entries into free list\n"),
                     unit));
        for (i = 0; i < entriesTotal; i++) {
            if (0 < i) {
                thisUnit->entry[i].prevEntry = i - 1;
            } else {
                thisUnit->entry[i].prevEntry = entriesTotal;
            }
            if (i < (rulesTotal - 1)) {
                thisUnit->entry[i].nextEntry = i + 1;
            } else {
                thisUnit->entry[i].nextEntry = entriesTotal;
            }
            thisUnit->entry[i].nextEntryRb = entriesTotal;
            thisUnit->entry[i].prevEntryRb = entriesTotal;
            thisUnit->entry[i].group = groupsTotal;
        } /* for (i = 0; i < entriesTotal; i++) */
        thisUnit->entryFreeHead = 0;
        thisUnit->entryFreeCount = entriesTotal;
        thisUnit->entryTotal = entriesTotal;

        FIELD_EVERB((FIELD_MSG1("init unit %d groups into free list\n"),
                     unit));
        for (i = 0; i < groupsTotal; i++) {
            if (0 < i) {
                thisUnit->group[i].prevGroup = i - 1;
            } else {
                thisUnit->group[i].prevGroup = groupsTotal;
            }
            thisUnit->group[i].nextGroup = i + 1;
            thisUnit->group[i].entryHead = entriesTotal;
            thisUnit->group[i].entryTail = entriesTotal;
            thisUnit->group[i].rulebase = _FIELD_FE2K_INVALID_RULEBASE;
        } /* for (i = 0; i < groupsTotal; i++) */
        thisUnit->groupFreeHead = 0;
        thisUnit->groupFreeCount = groupsTotal;
        thisUnit->groupTotal = groupsTotal;

        FIELD_EVERB((FIELD_MSG1("init unit %d ranges\n"),
                     unit));
        for (i = 0; i < rangesTotal; i++) {
            thisUnit->range[i].max = 0xFFFF;
        }
        FIELD_EVERB((FIELD_MSG1("create unit %d range freelist\n"), unit));
        result = shr_idxres_list_create(&(thisUnit->rangeFree),
                                        0,
                                        rangesTotal - 1,
                                        0,
                                        rangesTotal - 1,
                                        "_bcm_fe2k_field_unit_rangeList");
        if ((BCM_E_NONE != result) || (!(thisUnit->rangeFree))) {
            /* failed to create the range freelist */
            FIELD_ERR((FIELD_MSG1("failed to create range freelist\n")));
            thisUnit->rangeFree = NULL;
            if (BCM_E_NONE == result) {
                /* but there should have been an error */
                result = BCM_E_INTERNAL;
            }
        } /* if ((BCM_E_NONE != result) || (!(thisUnit->rangeFree))) */
    } /* if (BCM_E_NONE == result) */

    /*
     *  Set up the rulebase information in the unit description.
     *
     */
    if (BCM_E_NONE == result) {
        FIELD_EVERB((FIELD_MSG1("set up unit %d rulebases)\n"),
                     unit));

        /* ingress (StageQoS) rulebase */
        thisUnit->rulebase[0].dbId = SB_G2P3_CLS_QOS;
        thisUnit->rulebase[0].entriesMax = entriesIngress;
        thisUnit->rulebase[0].rulesMax = rulesIngress;
        thisUnit->rulebase[0].groupHead = groupsTotal;
        BCM_PBMP_ASSIGN(thisUnit->rulebase[0].ports, PBMP_ALL(unit));

        /* egress (StageEgressSecurity) rulebase */
        thisUnit->rulebase[1].dbId = SB_G2P3_CLS_EGRESS;
        thisUnit->rulebase[1].entriesMax = entriesEgress;
        thisUnit->rulebase[1].rulesMax = rulesEgress;
        thisUnit->rulebase[1].groupHead = groupsTotal;
        thisUnit->rulebase[1].ports = PBMP_ALL(unit);

        /* ingress (StageIpv6QoS) rulebase */
        thisUnit->rulebase[2].dbId = SB_G2P3_CLS_IPV6_QOS;
        thisUnit->rulebase[2].entriesMax = entriesIngress;
        thisUnit->rulebase[2].rulesMax = rulesIngress;
        thisUnit->rulebase[2].groupHead = groupsTotal;
        thisUnit->rulebase[2].ports = PBMP_ALL(unit);

        /* egress (StageIPv6EgressSecurity) rulebase */
        thisUnit->rulebase[3].dbId = SB_G2P3_CLS_IPV6_EGRESS;
        thisUnit->rulebase[3].entriesMax = entriesEgress;
        thisUnit->rulebase[3].rulesMax = rulesEgress;
        thisUnit->rulebase[3].groupHead = groupsTotal;
        thisUnit->rulebase[3].ports = PBMP_ALL(unit);

        /*
         *  Set up the allowed qualifiers.  These are assumed for now, but in
         *  the future, it's possible that the rule schema will change and
         *  these will then need to be probed somehow.
         */
        thisUnit->rulebase[0].entryHead = thisUnit->entryTotal;
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyStageIngressQoS);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifySrcMac);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyDstMac);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyDSCP);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyOuterVlan);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyOuterVlanId);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyOuterVlanPri);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyIpProtocol);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifySrcIp);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyDstIp);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyEtherType);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyTcpControl);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyInPort);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyInPorts);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyRangeCheck);
#if 0 
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyIpFrag);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyLlc);
        
#endif 

        /*
         *  Set up the supported actions.  Hopefully this will not change, but
         *  maybe in the future it will.  Again, we'll probably need a way to
         *  probe that in the case that it does.
         */
        SHR_BITSET(thisUnit->rulebase[0].action,
                   bcmFieldActionDrop);
        SHR_BITSET(thisUnit->rulebase[0].action,
                   bcmFieldActionVlanNew);
        SHR_BITSET(thisUnit->rulebase[0].action,
                   bcmFieldActionCosQNew);
        SHR_BITSET(thisUnit->rulebase[0].action,
                   bcmFieldActionDropPrecedence);
        SHR_BITSET(thisUnit->rulebase[0].action,
                   bcmFieldActionL3Switch);
        SHR_BITSET(thisUnit->rulebase[0].action,
                   bcmFieldActionUpdateCounter);
        SHR_BITSET(thisUnit->rulebase[0].action,
                   bcmFieldActionCopyToCpu);
        SHR_BITSET(thisUnit->rulebase[0].action,
                   bcmFieldActionRedirect);
        SHR_BITSET(thisUnit->rulebase[0].action,
                   bcmFieldActionRedirectMcast);
#if 0 
        SHR_BITSET(thisUnit->rulebase[0].action,
                   bcmFieldActionMirrorIngress);
        
#endif 
        

        /*
         *  Set up the allowed qualifiers.  These are assumed for now, but in
         *  the future, it's possible that the rule schema will change and
         *  these will then need to be probed somehow.
         */
        thisUnit->rulebase[1].entryHead = thisUnit->entryTotal;
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                           bcmFieldQualifyStageEgressSecurity);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                           bcmFieldQualifySrcMac);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                           bcmFieldQualifyDstMac);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                           bcmFieldQualifyDSCP);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                           bcmFieldQualifyIpProtocol);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                           bcmFieldQualifySrcIp);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                           bcmFieldQualifyDstIp);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                           bcmFieldQualifyEtherType);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                           bcmFieldQualifyTcpControl);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                           bcmFieldQualifyOutPort);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                           bcmFieldQualifyOutPorts);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                           bcmFieldQualifyRangeCheck);
#if 0 
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                           bcmFieldQualifyIpFrag);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[1].qset,
                           bcmFieldQualifyLlc);
        
#endif 

        /*
         *  Set up the supported actions.  Hopefully this will not change, but
         *  maybe in the future it will.  Again, we'll probably need a way to
         *  probe that in the case that it does.
         */
        SHR_BITSET(thisUnit->rulebase[1].action,
                   bcmFieldActionDrop);
        SHR_BITSET(thisUnit->rulebase[1].action,
                   bcmFieldActionUpdateCounter);
#if 0 
        SHR_BITSET(thisUnit->rulebase[1].action,
                   bcmFieldActionMirrorEgress);
#endif 

        if(thisUnit->uMaxSupportedStages > 2) {

        thisUnit->rulebase[2].entryHead = thisUnit->entryTotal;
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[2].qset,
                           bcmFieldQualifyStageIngressQoS);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[2].qset,
                           bcmFieldQualifyDSCP);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[2].qset,
                           bcmFieldQualifyOuterVlanId);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[2].qset,
                           bcmFieldQualifyIpProtocol);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[2].qset,
                           bcmFieldQualifySrcIp6);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[2].qset,
                           bcmFieldQualifyDstIp6);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[2].qset,
                           bcmFieldQualifyTcpControl);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[2].qset,
                           bcmFieldQualifyInPort);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[2].qset,
                           bcmFieldQualifyInPorts);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[2].qset,
                           bcmFieldQualifyRangeCheck);
#if 0 
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyIpFrag);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[0].qset,
                           bcmFieldQualifyLlc);
        
#endif 

        /*
         *  Set up the supported actions.  Hopefully this will not change, but
         *  maybe in the future it will.  Again, we'll probably need a way to
         *  probe that in the case that it does.
         */
        SHR_BITSET(thisUnit->rulebase[2].action,
                   bcmFieldActionDrop);
        SHR_BITSET(thisUnit->rulebase[2].action,
                   bcmFieldActionVlanNew);
        SHR_BITSET(thisUnit->rulebase[2].action,
                   bcmFieldActionCosQNew);
        SHR_BITSET(thisUnit->rulebase[2].action,
                   bcmFieldActionDropPrecedence);
        SHR_BITSET(thisUnit->rulebase[2].action,
                   bcmFieldActionL3Switch);
        SHR_BITSET(thisUnit->rulebase[2].action,
                   bcmFieldActionUpdateCounter);
        SHR_BITSET(thisUnit->rulebase[2].action,
                   bcmFieldActionCopyToCpu);
        SHR_BITSET(thisUnit->rulebase[2].action,
                   bcmFieldActionRedirect);
        SHR_BITSET(thisUnit->rulebase[2].action,
                   bcmFieldActionRedirectMcast);
#if 0 
        SHR_BITSET(thisUnit->rulebase[0].action,
                   bcmFieldActionMirrorIngress);
        
#endif 
        

        /*
         *  Set up the allowed qualifiers.  These are assumed for now, but in
         *  the future, it's possible that the rule schema will change and
         *  these will then need to be probed somehow.
         */
        thisUnit->rulebase[3].entryHead = thisUnit->entryTotal;
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[3].qset,
                           bcmFieldQualifyStageEgressSecurity);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[3].qset,
                           bcmFieldQualifyDSCP);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[3].qset,
                           bcmFieldQualifyIpProtocol);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[3].qset,
                           bcmFieldQualifySrcIp6);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[3].qset,
                           bcmFieldQualifyDstIp6);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[3].qset,
                           bcmFieldQualifyTcpControl);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[3].qset,
                           bcmFieldQualifyOutPort);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[3].qset,
                           bcmFieldQualifyOutPorts);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[3].qset,
                           bcmFieldQualifyRangeCheck);
#if 0 
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[3].qset,
                           bcmFieldQualifyIpFrag);
        BCM_FIELD_QSET_ADD(thisUnit->rulebase[3].qset,
                           bcmFieldQualifyLlc);
        
#endif 

        /*
         *  Set up the supported actions.  Hopefully this will not change, but
         *  maybe in the future it will.  Again, we'll probably need a way to
         *  probe that in the case that it does.
         */
        SHR_BITSET(thisUnit->rulebase[3].action,
                   bcmFieldActionDrop);
        SHR_BITSET(thisUnit->rulebase[3].action,
                   bcmFieldActionUpdateCounter);
#if 0 
        SHR_BITSET(thisUnit->rulebase[3].action,
                   bcmFieldActionMirrorEgress);
#endif 
     }
    } /* if (BCM_E_NONE == result) */

#ifdef BCM_WARM_BOOT_SUPPORT
    /* on cold boot, setup scache */
    SOC_SCACHE_HANDLE_SET(_field_cache[unit].wb_hdl, unit, BCM_MODULE_FIELD, 0);
    /* Is Level 2 recovery even available? */
    tmpRes = soc_stable_size_get(unit, (int*)&size);
    if (tmpRes == SOC_E_NONE) {
    
        /* Allocate a new chunk of the scache during a cold boot */
        if (!SOC_WARM_BOOT(unit) && (size > 0)) {

            /* calculate memory requirements */
            scache_size = _FIELD_FE2K_G2P3_STAGES * sizeof(soc_sbx_g2p3_field_wb_rulebase_t);
            scache_size += _FIELD_FE2K_G2P3_MAX_GROUPS * sizeof(soc_sbx_g2p3_field_wb_group_t);
            scache_size += rulesTotal * sizeof(soc_sbx_g2p3_field_wb_entry_t);
            scache_size += SOC_WB_SCACHE_CONTROL_SIZE;
            tmpRes = soc_scache_alloc(unit, _field_cache[unit].wb_hdl, scache_size);
        }

        if ( (tmpRes == SOC_E_NONE) || (tmpRes == SOC_E_EXISTS) ){

            /* Get the pointer from scache */
            wbc = NULL;
            _field_cache[unit].scache_size = 0;
            if (size > 0) {
                tmpRes = soc_scache_ptr_get(unit, _field_cache[unit].wb_hdl,
                                            (uint8**)&wbc,
                                            &_field_cache[unit].scache_size);
                if ((tmpRes == SOC_E_NONE) && (wbc != NULL)) {
                    if (!SOC_WARM_BOOT(unit)) {
                        wbc->version = G2P3_FIELD_WB_CURRENT_VERSION;
                    }

                    FIELD_VERB((FIELD_MSG1("Obtained scache pointer=0x%08x, %d bytes, "
                                    "version=%d.%d\n"),
                             (int)wbc->cache, _field_cache[unit].scache_size,
                             SOC_SCACHE_VERSION_MAJOR(wbc->version),
                             SOC_SCACHE_VERSION_MINOR(wbc->version)));

                    if (wbc->version > G2P3_FIELD_WB_CURRENT_VERSION) {
                        FIELD_ERR((FIELD_MSG1("Upgrade scenario not supported.  "
                                       "Current version=%d.%d  found %d.%d\n"),
                                SOC_SCACHE_VERSION_MAJOR(G2P3_FIELD_WB_CURRENT_VERSION),
                                SOC_SCACHE_VERSION_MINOR(G2P3_FIELD_WB_CURRENT_VERSION),
                                SOC_SCACHE_VERSION_MAJOR(wbc->version),
                                SOC_SCACHE_VERSION_MINOR(wbc->version)));
                        tmpRes = SOC_E_CONFIG;

                    } else if (wbc->version < G2P3_FIELD_WB_CURRENT_VERSION) {
                        FIELD_ERR((FIELD_MSG1("Downgrade scenario not supported.  "
                                       "Current version=%d.%d  found %d.%d\n"),
                                SOC_SCACHE_VERSION_MAJOR(G2P3_FIELD_WB_CURRENT_VERSION),
                                SOC_SCACHE_VERSION_MINOR(G2P3_FIELD_WB_CURRENT_VERSION),
                                SOC_SCACHE_VERSION_MAJOR(wbc->version),
                                SOC_SCACHE_VERSION_MINOR(wbc->version)));
                        tmpRes = SOC_E_CONFIG;
                    }

                    if (tmpRes == BCM_E_NONE) {

                        /* if warmboot, perform recovery */
                        if (SOC_WARM_BOOT(unit)) {

                            result = _bcm_fe2000_g2p3_field_scache_recover(thisUnit);

                            if (result != BCM_E_NONE) {
                                FIELD_ERR((FIELD_MSG1("unit %d failed WB recovery: %d (%s)\n"), 
                                                      thisUnit->unit, 
                                                      result, 
                                                      _SHR_ERRMSG(result)));
                            }
                        }else{
                            result = _bcm_fe2000_g2p3_field_scache_store(thisUnit, _FIELD_FE2K_INVALID_RULEBASE);
                            if (result != BCM_E_NONE) {
                                FIELD_ERR((FIELD_MSG1("unit %d to initialize field WB cache: %d (%s)\n"), 
                                                  thisUnit->unit, 
                                                  result, 
                                                  _SHR_ERRMSG(result)));
                            }
                        }
                    } 
                } /* wbc ok? */
            } /* size > 0 */
        }  /* wb_hdl ok? */
    } /* stable_size_get */
#endif

    /* don't do this during warmboot */
    if ((result == BCM_E_NONE) && !SOC_WARM_BOOT(thisUnit->unit)) {
        /* now make sure rulebases are clear (also in hardware) */
        FIELD_EVERB((FIELD_MSG1("clear unit %d rulebases\n"), unit));
        for (i = 0; i < thisUnit->uMaxSupportedStages; i++) {
            FIELD_EVERB((FIELD_MSG1("clear unit %d rulebase %02X \n"),
                         thisUnit->unit,
                         thisUnit->rulebase[i].dbId));
            switch (i) {
            case 0:
                tmpRes = soc_sbx_g2p3_ifp_clear_all(thisUnit->unit);
                break;
            case 1:
                tmpRes = soc_sbx_g2p3_efp_clear_all(thisUnit->unit);
                break;
            case 2:
                tmpRes = soc_sbx_g2p3_ifp_v6_clear_all(thisUnit->unit);
                break;
            case 3:
                tmpRes = soc_sbx_g2p3_efp_v6_clear_all(thisUnit->unit);
                break;
            default:
                FIELD_ERR((FIELD_MSG1("unit %d unexpected stage %d\n"),
                           thisUnit->unit,
                           i));
                tmpRes = BCM_E_INTERNAL;
            }
            if (BCM_E_NONE != tmpRes) {
                result = tmpRes;
                FIELD_ERR((FIELD_MSG1("unable to clear unit %d rulebase %02X: %d (%s)\n"),
                           thisUnit->unit,
                           thisUnit->rulebase[i].dbId,
                           result,
                           _SHR_ERRMSG(result)));
                break;  /* out of loop on error */
            }
            switch (i) {
            case 0:
                tmpRes = soc_sbx_g2p3_ifp_handle_commit(thisUnit->unit, -1);
                break;
            case 1:
                tmpRes = soc_sbx_g2p3_efp_handle_commit(thisUnit->unit, -1);
                break;
            case 2:
                tmpRes = soc_sbx_g2p3_ifp_v6_handle_commit(thisUnit->unit, -1);
                break;
            case 3:
                tmpRes = soc_sbx_g2p3_efp_v6_handle_commit(thisUnit->unit, -1);
                break;
            }
            if (BCM_E_NONE != tmpRes) {
                result = tmpRes;
                FIELD_ERR((FIELD_MSG1("unable to commit unit %d rulebase %02X: %d (%s)\n"),
                           thisUnit->unit,
                           thisUnit->rulebase[i].dbId,
                           result,
                           _SHR_ERRMSG(result)));
                break;  /* out of loop on error */
            }
        }
    }

    if (result == BCM_E_NONE) {

        /* use well-known policer for Drop action */
        thisUnit->dropPolicer = BCM_FE2000_SPEC_POL_DROP_ALL;

        /* turn on the 'copyToCpu' exception */
        tmpRes = soc_sbx_g2p3_exc_rt_copy_idx_get(unit, &excId);
        if (BCM_E_NONE == tmpRes) {

            soc_sbx_g2p3_xt_t_init(&copyToCpuExc);
            tmpRes = soc_sbx_g2p3_xt_get(unit, excId, &copyToCpuExc);
            if (BCM_E_NONE != tmpRes) {
                result = tmpRes;
                FIELD_ERR((FIELD_MSG1("unable to get %d:xt[%08X]: %d (%s)\n"),
                           unit, excId, result, _SHR_ERRMSG(result)));
            }

            FIELD_VVERB((FIELD_MSG1("program unit %d CopyToCpu exception"
                                    " %02X:qid=%08X\n"),
                         unit,
                         excId,
                         copyToCpuExc.qid));

            copyToCpuExc.trunc = FALSE;
            copyToCpuExc.forward = TRUE;
            tmpRes = soc_sbx_g2p3_xt_set(unit, excId, &copyToCpuExc);
            if (BCM_E_NONE != tmpRes) {
                result = tmpRes;
                FIELD_ERR((FIELD_MSG1("unable to set %d:xt[%08X]: %d (%s)\n"),
                           unit,
                           excId,
                           result,
                           _SHR_ERRMSG(result)));
            }
        } else {
            result = tmpRes;
            FIELD_ERR((FIELD_MSG1("unable to get unit %d copyToCpu exception ID:"
                                  " %d (%s)\n"),
                       unit,
                       result,
                       _SHR_ERRMSG(result)));
        }
    }

    /* wrap it up */
    if (BCM_E_NONE == result) {
        /* return the unit information pointer */
        *unitData = (void*)thisUnit;
    } else {/* if (BCM_E_NONE == result) */
        if (thisUnit) {
            if (thisUnit->rangeFree) {
                /* get rid of range freelist */
                FIELD_EVERB((FIELD_MSG1("get rid of unit %d range freelist\n"),
                             unit));
                shr_idxres_list_destroy(thisUnit->rangeFree);
            }
            if (thisUnit->group) {
                /* get rid of unit indices */
                FIELD_EVERB((FIELD_MSG1("get rid of unit %d index space\n"),
                             unit));
                sal_free(thisUnit->group);
            }
            /* get rid of unit private data */
            FIELD_EVERB((FIELD_MSG1("get rid of unit %d private data\n"),
                         unit));
            sal_free(thisUnit);
            thisUnit = NULL;
        } /* if (thisUnit) */
    }/* if (BCM_E_NONE == result) */

    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_detach
 *   Purpose
 *      Initialise the field APIs.
 *   Parameters
 *      (in) void* unitData = the pointer to the unit data
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Returns last error if any occur.
 *      Allows limited resource dangling in order to improve performance.
 */
int
bcm_fe2000_g2p3_field_detach(void *unitData)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;  /* working entry data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;      /* working rule */
    unsigned int mirrorId = 0;                /* working mirror ID */
    uint32 ftIndex = 0;                       /* working FT index */
    int result;                               /* working result */
    int tmpRes;                               /* working result */
    int i;                                    /* working indices */
#ifndef BCM_WARM_BOOT_SUPPORT
    uint32 excId;                             /* exception ID for copyToCpu */
    soc_sbx_g2p3_xt_t copyToCpuExc;           /* copyToCpu exception data */
#endif

    /* map local unit information */
    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;

    /* get rid of the range freelist */
    result = shr_idxres_list_destroy(thisUnit->rangeFree);
    if (BCM_E_NONE != result) {
        FIELD_ERR((FIELD_MSG1("unable to destroy unit %d range freelist:"
                              " %d (%s)\n"),
                   thisUnit->unit,
                   result,
                   _SHR_ERRMSG(result)));
    }
    /* get rid of all rules in all stages */
    for (i = 0; i < thisUnit->entryTotal; i++) {
        /* get rid of rules for this entry */
        thisEntry = &(thisUnit->entry[i]);
        if (!(thisEntry->entryFlags & _FE2K_G2P3_ENTRY_VALID)) {
            /* don't free from non-entries */
            continue;
        }
        
        /* need to read rule so we get resource information */
        result = _bcm_fe2k_g2p3_field_rules_read(thisUnit,
                                                 i,
                                                 &rule);
        if (BCM_E_NONE == result) {
            /* scan entry for resources */
            mirrorId = 0;
            ftIndex = 0;
            switch (thisUnit->group[thisEntry->group].rulebase) {
            case 0:
                if (!mirrorId) {
                    mirrorId = rule.ingress.mirror;
                }
                if ((thisEntry->entryFlags & _FE2K_G2P3_ENTRY_REDIRMC) &&
                    !ftIndex) {
                    ftIndex = rule.ingress.ftidx;
                }
                break;
            case 1:
                if (!mirrorId) {
                    mirrorId = rule.egress.mirror;
                }
                break;
            case 2:
                if (!mirrorId) {
                    mirrorId = rule.ingress_v6.mirror;
                }
                if ((thisEntry->entryFlags & _FE2K_G2P3_ENTRY_REDIRMC) &&
                    !ftIndex) {
                    ftIndex = rule.ingress_v6.ftidx;
                }
                break;
            case 3:
                if (!mirrorId) {
                    mirrorId = rule.egress_v6.mirror;
                }
                break;
            default:
                /* to keep compiler from complaining */
                break;
            } /* switch (rulebase) */
            /* get rid of detected resources */
            if (mirrorId) {
                switch (thisUnit->group[thisEntry->group].rulebase) {
                case 0:
                case 2:
                    result = _bcm_fe2000_ingr_mirror_free(thisUnit->unit, mirrorId);
                    break;
                case 1:
                case 3:
                    result = _bcm_fe2000_egr_mirror_free(thisUnit->unit, mirrorId);
                    break;
                default:
                    /* should never reach this */
                    result = BCM_E_INTERNAL;
                } /* switch (rulebase) */
            } /* if (mirrorId) */
            if (ftIndex) {
                result = _sbx_gu2_resource_free(thisUnit->unit,
                                                SBX_GU2K_USR_RES_FTE_L2MC,
                                                1,
                                                &ftIndex,
                                                0);
            }
        } /* if (BCM_E_NONE == result) */
        /* destory the rules associated with this entry */
        if (thisEntry->hwRule) {
            /* this entry has a rule */
            switch (thisUnit->group[thisEntry->group].rulebase) {
            case 0:
                tmpRes = soc_sbx_g2p3_ifp_handle_remove(thisUnit->unit,
                                                        thisEntry->hwRule);
                break;
            case 1:
                tmpRes = soc_sbx_g2p3_efp_handle_remove(thisUnit->unit,
                                                        thisEntry->hwRule);
                break;
            case 2:
                tmpRes = soc_sbx_g2p3_ifp_v6_handle_remove(thisUnit->unit,
                                                        thisEntry->hwRule);
                break;
            case 3:
                tmpRes = soc_sbx_g2p3_efp_v6_handle_remove(thisUnit->unit,
                                                        thisEntry->hwRule);
                break;
            default:
                tmpRes = BCM_E_INTERNAL;
            } /* switch (thisUnit->group[thisEntry].rulebase) */
            if (BCM_E_NONE != tmpRes) {
                result = tmpRes;
                FIELD_ERR((FIELD_MSG1("unable to remove unit %d entry %08X"
                                      " rule %02X:%08X: %d (%s)\n"),
                           thisUnit->unit,
                           i,
                           thisUnit->rulebase[thisUnit->group[thisEntry->group].rulebase].dbId,
                           (uint32)(thisEntry->hwRule),
                           result,
                           _SHR_ERRMSG(result)));
            }
        } /* if (thisEntry->hwRule) */
        /* get rid of resources owned by this entry */
    } /* for all possible entries */

#ifndef BCM_WARM_BOOT_SUPPORT
    /* now purge anything left */
    for (i = 0; i < thisUnit->uMaxSupportedStages; i++) {
        switch (i) {
        case 0:
            tmpRes = soc_sbx_g2p3_ifp_clear_all(thisUnit->unit);
            break;
        case 1:
            tmpRes = soc_sbx_g2p3_efp_clear_all(thisUnit->unit);
            break;
        case 2:
            tmpRes = soc_sbx_g2p3_ifp_v6_clear_all(thisUnit->unit);
            break;
        case 3:
            tmpRes = soc_sbx_g2p3_efp_v6_clear_all(thisUnit->unit);
            break;
        default:
            tmpRes = BCM_E_INTERNAL;
        }
        if (BCM_E_NONE != tmpRes) {
            result = tmpRes;
            FIELD_ERR((FIELD_MSG1("unable to clear unit %d rulebase %02X: %d (%s)\n"),
                       thisUnit->unit,
                       thisUnit->rulebase[i].dbId,
                       result,
                       _SHR_ERRMSG(result)));
        }
        /* if WARM_BOOT is supported just deallocate memory and resources, do not
         * update hardware
         */
        if (!SOC_WARM_BOOT(thisUnit->unit)) {
            switch (i) {
            case 0:
                tmpRes = soc_sbx_g2p3_ifp_handle_commit(thisUnit->unit, -1);
                break;
            case 1:
                tmpRes = soc_sbx_g2p3_efp_handle_commit(thisUnit->unit, -1);
                break;
            case 2:
                tmpRes = soc_sbx_g2p3_ifp_v6_handle_commit(thisUnit->unit, -1);
                break;
            case 3:
                tmpRes = soc_sbx_g2p3_efp_v6_handle_commit(thisUnit->unit, -1);
                break;
            default:
                FIELD_ERR((FIELD_MSG1("unit %d unexpected stage %d\n"),
                           thisUnit->unit,
                           i));
                tmpRes = BCM_E_INTERNAL;
            }
            if (BCM_E_NONE != tmpRes) {
                result = tmpRes;
                FIELD_ERR((FIELD_MSG1("unable to commit unit %d rulebase %02X: %d (%s)\n"),
                           thisUnit->unit,
                           thisUnit->rulebase[i].dbId,
                           result,
                           _SHR_ERRMSG(result)));
            }
        }
    }

    /* turn off the 'copyToCpu' exception */
    tmpRes = soc_sbx_g2p3_exc_rt_copy_idx_get(thisUnit->unit, &excId);
    if (BCM_E_NONE == tmpRes) {
        soc_sbx_g2p3_xt_t_init(&copyToCpuExc);
        tmpRes = soc_sbx_g2p3_xt_get(thisUnit->unit, excId, &copyToCpuExc);
        if (BCM_E_NONE != tmpRes) {
            result = tmpRes;
            FIELD_ERR((FIELD_MSG1("unable to get %d:xt[%08X]: %d (%s)\n"),
                       thisUnit->unit, excId, result, _SHR_ERRMSG(result)));
        } else {
            copyToCpuExc.forward = FALSE;
            tmpRes = soc_sbx_g2p3_xt_set(thisUnit->unit, excId, &copyToCpuExc);
            if (BCM_E_NONE != tmpRes) {
                result = tmpRes;
                FIELD_ERR((FIELD_MSG1("unable to set %d:xt[%08X]: %d (%s)\n"),
                           thisUnit->unit,
                           excId,
                           result,
                           _SHR_ERRMSG(result)));
            }
        }
    } else {
        result = tmpRes;
        FIELD_ERR((FIELD_MSG1("unable to get unit %d copyToCpu exception ID:"
                              " %d (%s)\n"),
                   thisUnit->unit,
                   result,
                   _SHR_ERRMSG(result)));
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    
    /* get rid of unit indices */
    sal_free(thisUnit->group);
    thisUnit->group = NULL;
    /* get rid of unit data */
    sal_free(thisUnit);
    thisUnit = NULL;
    /* okay, done */
    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_wb_state_sync
 *   Purpose
 *      Synchronize current field state with scache
 *   Parameters
 *      (in) void* unitData = the pointer to the unit data
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Returns last error if any occur.
 */
int
bcm_fe2000_g2p3_field_wb_state_sync(void *unitData, int sync)
{
    int result = BCM_E_NONE;                  /* working result */

#ifdef BCM_WARM_BOOT_SUPPORT
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */
    int i;                                    /* working indices */

    /* map local unit information */
    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;

    if (SOC_WARM_BOOT(thisUnit->unit)) {
        FIELD_ERR((FIELD_MSG1("Cannot write to SCACHE during WarmBoot\n")));
        return BCM_E_INTERNAL;
    }

    /* force a commit to synchronize RAM and hardware state */
    for (i=0; i<_FIELD_FE2K_G2P3_STAGES; i++) {
        result = _bcm_fe2k_g2p3_field_rulebase_commit(thisUnit,
                                                      i);
        if (BCM_E_NONE  != result) {
            FIELD_ERR((FIELD_MSG1("unable to commit rules to scache for"
                                  " rulebase %d (SBX db %02X): %d (%s)\n"),
                       i,
                       thisUnit->rulebase[i].dbId,
                       result,
                       _SHR_ERRMSG(result)));
        }
    }
#endif

    /* okay, done */
    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_status_get
 *   Purpose
 *      Get the status of the field APIs.
 *   Parameters
 *      (in) int unit = the unit number
 *      (out) bcm_field_status_t *status = pointer to where to put status
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_g2p3_field_status_get(void *unitData,
                                 bcm_field_status_t *status)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    status->flags = 0;
    status->group_free = thisUnit->groupFreeCount;
    status->group_total = thisUnit->groupTotal;

    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_group_create
 *   Purpose
 *      Create a new group that has the specified qualifier set and priority.
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_qset_t qset = the qualifier set
 *      (in) int pri = the priority
 *      (out) bcm_field_group_t *group = where to put the group ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Allocates first available group ID.
 *      Can not specify a priority already taken by an existing group (does
 *      this mean globally or within just the single stage?)
 *      Can not specify a qualifier that another group in the same stage has.
 *      If no stage qualifier, it is assumed to be ingress stage.
 */
int
bcm_fe2000_g2p3_field_group_create(void *unitData,
                                   bcm_field_qset_t qset,
                                   int pri,
                                   bcm_field_group_t *group)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */
    _bcm_fe2k_g2p3_field_group_t groupData;   /* working group data */
    _bcm_fe2k_field_group_index groupId;      /* working group ID */
    int result;                               /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* set up the group */
    sal_memset(&groupData, 0, sizeof(groupData));
    groupData.qset = qset;
    if (BCM_FIELD_GROUP_PRIO_ANY == pri) {
        groupData.priority = _SBX_FE2K_FIELD_GROUP_PRIO_ANY;
    }else{
        groupData.priority = pri;
    }
    groupId = thisUnit->groupTotal;
    result = _bcm_fe2k_g2p3_field_group_create(unitData, &groupId, &groupData);
    if (BCM_E_NONE == result) {
        *group = groupId;
    }
    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_group_create_id
 *   Purpose
 *      Create a new group with the specified ID that has the specified
 *      qualifier set and priority.
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_qset_t qset = the qualifier set
 *      (in) int pri = the priority
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Allocates first available group ID.
 *      Can not specify a priority already taken by an existing group.
 *      Can not specify a qualifier that another group in the same stage has.
 *      If no stage qualifier, it is assumed to be bcmFieldQualifyStageIngress.
 */
int
bcm_fe2000_g2p3_field_group_create_id(void *unitData,
                                      bcm_field_qset_t qset,
                                      int pri,
                                      bcm_field_group_t group)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */
    _bcm_fe2k_g2p3_field_group_t groupData;   /* working group data */
    _bcm_fe2k_field_group_index groupId;      /* working group ID */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    if ((thisUnit->groupTotal <= group) || (0 > group)) {
        FIELD_ERR((FIELD_MSG1("can't create invalid group ID %08X"
                              " on unit %d\n"),
                   group,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    /* set up the group */
    sal_memset(&groupData, 0, sizeof(groupData));
    groupData.qset = qset;
    if (BCM_FIELD_GROUP_PRIO_ANY == pri) {
        groupData.priority = _SBX_FE2K_FIELD_GROUP_PRIO_ANY;
    }else{
        groupData.priority = pri;
    }
    groupId = group;
    return _bcm_fe2k_g2p3_field_group_create(unitData, &groupId, &groupData);
}


/*
 *   Function
 *      bcm_fe2000_g2p3_field_group_install
 *   Purpose
 *      Insert all of a group's entries to the hardware.
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This inserts and updates all of the group's entries to the hardware as
 *      appropriate.  No error is asserted for entries already in hardware,
 *      even if the entire group is already in hardware.
 *      Since this calls the SBX commit for the database, entries that are
 *      changed even in other groups will also be committed to the hardware.
 */
int
bcm_fe2000_g2p3_field_group_install(void *unitData,
                                    bcm_field_group_t group)
{
    int result = BCM_E_NONE;                  /* working result */
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */
    _bcm_fe2k_g2p3_field_group_t *thisGroup;  /* working group data */
    _bcm_fe2k_field_entry_index entry;        /* working entry ID */
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;    /* working entry data */
    _bcm_fe2k_g2p3_fld_entry_flags_t entryFlags;/* working entry flags */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    if ((thisUnit->groupTotal <= group) || (0 > group)) {
        FIELD_ERR((FIELD_MSG1("can't install invalid group ID %08X"
                              " on unit %d\n"),
                   group,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    thisGroup = &(thisUnit->group[group]);
    if (thisUnit->uMaxSupportedStages <= thisGroup->rulebase) {
        FIELD_ERR((FIELD_MSG1("unit %d group %08X is not in use\n"),
                   thisUnit->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }

    for (entry = thisGroup->entryHead;
         entry < thisUnit->entryTotal;
         entry = thisUnit->entry[entry].nextEntry) {

        thisEntry = &(thisUnit->entry[entry]);
        entryFlags = thisUnit->entry[entry].entryFlags;
        if (entryFlags & _FE2K_G2P3_ENTRY_IN_HW) {
            continue;
        }

        result = _bcm_fe2k_g2p3_field_entry_set_install(thisUnit,
                                                        entry,
                                                        TRUE);
        if (BCM_E_NONE != result) {
            /* called function will display error */
            /* if some error setting them enabled, bail out */
            break;
        }
    }
    if (BCM_E_NONE == result) {
        result = _bcm_fe2k_g2p3_field_rulebase_commit(thisUnit,
                                                      thisGroup->rulebase);
        /* called function will display error */
    }
    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_group_remove
 *   Purpose
 *      Remove all of a group's entries from the hardware.
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This removes all of the group's entries from the hardware as
 *      appropriate.  No error is asserted for entries not in hardware,
 *      even if the entire group is not in hardware.
 *      Since this calls the SBX commit for the database, entries that are
 *      changed even in other groups will also be committed to the hardware.
 */
int
bcm_fe2000_g2p3_field_group_remove(void *unitData,
                                   bcm_field_group_t group)
{
    int result = BCM_E_NONE;                  /* working result */
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */
    _bcm_fe2k_g2p3_field_group_t *thisGroup;  /* working group data */
    _bcm_fe2k_field_entry_index entry;        /* working entry ID */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    if ((thisUnit->groupTotal <= group) || (0 > group)) {
        FIELD_ERR((FIELD_MSG1("can't install invalid group ID %08X"
                              " on unit %d\n"),
                   group,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    thisGroup = &(thisUnit->group[group]);
    if (thisUnit->uMaxSupportedStages <= thisGroup->rulebase) {
        FIELD_ERR((FIELD_MSG1("unit %d group %08X is not in use\n"),
                   thisUnit->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }
    for (entry = thisGroup->entryHead;
         entry < thisUnit->entryTotal;
         entry = thisUnit->entry[entry].nextEntry) {
        result = _bcm_fe2k_g2p3_field_entry_set_install(thisUnit,
                                                        entry,
                                                        FALSE);
        if (BCM_E_NONE != result) {
            /* called function will display error */
            /* if some error setting them enabled, bail out */
            break;
        }
    }
    if (BCM_E_NONE == result) {
        result = _bcm_fe2k_g2p3_field_rulebase_commit(thisUnit,
                                                      thisGroup->rulebase);
        /* called function will display error */
    }
    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_group_flush
 *   Purpose
 *      Remove all of a group's entries from the hardware, remove the group
 *      from the hardware, remove the group's entries from the software, and
 *      remove the group from the software.
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This will destroy the field group and all its entries.  No mention is
 *      made that it affects ranges, so they aren't destroyed.  This also
 *      destroys the field group and its entries in hardware, so it also
 *      commits the appropriate stage entries to the SBX driver.
 */
int
bcm_fe2000_g2p3_field_group_flush(void *unitData,
                                  bcm_field_group_t group)
{
    int result;                               /* working result */
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */
    _bcm_fe2k_g2p3_field_group_t *thisGroup;  /* working group data */
    _bcm_fe2k_field_entry_index entry;        /* working entry ID */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    if ((thisUnit->groupTotal <= group) || (0 > group)) {
        FIELD_ERR((FIELD_MSG1("can't check qset of invalid group ID %08X"
                              " on unit %d\n"),
                   group,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    thisGroup = &(thisUnit->group[group]);
    if (thisUnit->uMaxSupportedStages <= thisGroup->rulebase) {
        FIELD_ERR((FIELD_MSG1("unit %d group %08X is not in use\n"),
                   thisUnit->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }
    /* get rid of all the entries in the group */
    for (entry = thisGroup->entryHead;
         thisUnit->entryTotal > entry;
         entry = thisGroup->entryHead) {
        result = _bcm_fe2k_g2p3_field_entry_destroy(unitData, entry);
        if (BCM_E_NONE != result) {
            /* called function will display appropriate errors */
            return result;
        }
    }
    /* commit the updates to the driver */
    result = _bcm_fe2k_g2p3_field_rulebase_commit(thisUnit,
                                                  thisGroup->rulebase);
    if (BCM_E_NONE != result) {
        /* called function displays error message */
        return result;
    }
    /* get rid of the group */
    result = _bcm_fe2k_g2p3_field_group_del(unitData, group);
    if ((BCM_E_NONE == result) || (BCM_E_CONFIG == result)) {
        /*
         *  We want to try to free the group if we got BCM_E_CONFIG because it
         *  could indicate bad state (a group that, for example, tried to move
         *  between rulebases and failed, getting caught in limbo).
         */
        result = _bcm_fe2k_g2p3_field_group_free(unitData, group);
    }
    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_group_set
 *   Purpose
 *      This changes the group's qualifier set so it is the specified set.
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_group_t group = which group ID to use
 *      (in) bcm_field_qset_t qset = new qset
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      If there are any entries, all of them must be representable using the
 *      new qualifier set (if not, this fails), plus the new qualifier set can
 *      not change the required pattern type (it will also fail in this case).
 *
 *      Updates are always permitted if there are no entries present.
 *
 *      We don't bother to update any of the rulebase/entry linkages when
 *      moving a group between rulebases because we don't allow any group with
 *      entries to engage in such a move.
 */
int
bcm_fe2000_g2p3_field_group_set(void *unitData,
                                bcm_field_group_t group,
                                bcm_field_qset_t qset)
{
    int result = BCM_E_NONE;                  /* working result */
    int auxRes;                               /* spare result */
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */
    _bcm_fe2k_g2p3_field_group_t *thisGroup;  /* working group data */
    uint8 rulebase;                           /* working rulebase */
    uint8 oldRulebase;                        /* original rulebase */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    if ((thisUnit->groupTotal <= group) || (0 > group)) {
        FIELD_ERR((FIELD_MSG1("can't check qset of invalid group ID %08X"
                              " on unit %d\n"),
                   group,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    thisGroup = &(thisUnit->group[group]);
    if (thisUnit->uMaxSupportedStages <= thisGroup->rulebase) {
        FIELD_ERR((FIELD_MSG1("unit %d group %08X is not in use\n"),
                   thisUnit->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }
    if (thisGroup->entries) {
        /* the group has entries; make sure new qset is superset of current */
        result = _bcm_fe2000_qset_subset(qset, thisGroup->qset);
        if (BCM_E_FAIL == result) {
            /* new qset is not a superset of current */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X has entries and new"
                                  " qset is not superset of current\n"),
                       thisUnit->unit,
                       group));
            return BCM_E_CONFIG;
        } else if (BCM_E_NONE != result) {
            /* some other error comparing qsets */
            FIELD_ERR((FIELD_MSG1("unable to compare new qset to unit %d"
                                  " group %08X: %d (%s)\n"),
                       thisUnit->unit,
                       group,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
        /* now make sure it still fits in the stage */
        result = _bcm_fe2000_qset_subset(thisUnit->rulebase[thisGroup->rulebase].qset,
                                         qset);
        if (BCM_E_FAIL == result) {
            /* new qset does not fit in this stage */
            FIELD_ERR((FIELD_MSG1("new qset does not fit in the group's"
                                  " current stage %d\n"),
                       thisGroup->rulebase));
            return BCM_E_CONFIG;
        } else if (BCM_E_NONE != result) {
            /* some other error comparing qsets */
            FIELD_ERR((FIELD_MSG1("unable to compare new qset to unit %d"
                                  " stage %d qset: %d (%s)\n"),
                       thisUnit->unit,
                       thisGroup->rulebase,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
        /* now make sure the LLC field hasn't toggled */
        if (BCM_FIELD_QSET_TEST(thisGroup->qset, bcmFieldQualifyLlc) !=
            BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyLlc)) {
            /* the LLC mode changed; can't allow that with entries */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset LLC differs from"
                                  " the new qset LLC; this change affects"
                                  " entry format and can not be done with"
                                  " entries present\n"),
                       thisUnit->unit,
                       group));
            return BCM_E_CONFIG;
        }
        /* finally, all looks well, so update the qset */
        thisGroup->qset = qset;
        FIELD_EVERB((FIELD_MSG1("updated unit %d group %08X qset in place\n"),
                     thisUnit->unit,
                     group));
        return BCM_E_NONE;
    } /* if (thisGroup->entries) */
    /* figure out which stage is appropriate; try ingress first */
    for (rulebase = 0; rulebase < thisUnit->uMaxSupportedStages; rulebase++) {
        result = _bcm_fe2000_qset_subset(thisUnit->rulebase[rulebase].qset,
                                         qset);
        if ((BCM_E_NONE == result) ||
            ((BCM_E_NONE != result) && (BCM_E_FAIL != result))) {
            /* it fits in this rulebase or something went wrong */
            break;
        }
    } /* for (rulebase = 0; rulebase < thisUnit->uMaxSupportedStages; rulebase++) */
    if (BCM_E_FAIL == result) {
        FIELD_ERR((FIELD_MSG1("specified qset does not match any stage"
                              " on unit %d\n"),
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_QSET_ERR;
    } else if (BCM_E_NONE != result) {
        FIELD_ERR((FIELD_MSG1("unable to compare qsets on unit %d: %d (%s)\n"),
                   thisUnit->unit,
                   result,
                   _SHR_ERRMSG(result)));
    }
    if (rulebase != thisGroup->rulebase) {
        /* need to change stages */
        FIELD_VVERB((FIELD_MSG1("need to move unit %d group %08X from rulebase"
                                " %d to rulebase %d...\n"),
                     thisUnit->unit,
                     group,
                     thisGroup->rulebase,
                     rulebase));
        oldRulebase = thisGroup->rulebase;
        result = _bcm_fe2k_g2p3_field_group_del(thisUnit, group);
        if (BCM_E_NONE == result) {
            result = _bcm_fe2k_g2p3_field_group_add(thisUnit, rulebase, group);
        }
        if (BCM_E_NONE == result) {
            /* that went well; update the group's qset */
            thisGroup->qset = qset;
        } else { /* if (BCM_E_NONE == result) */
            if (thisGroup->rulebase < thisUnit->uMaxSupportedStages) {
                /* there was an error, but the state is consistent */
                FIELD_ERR((FIELD_MSG1("unable to move unit %d group %08X from"
                                      " rulebase %d to rulebase %d: %d (%s)\n"),
                           thisUnit->unit,
                           group,
                           oldRulebase,
                           rulebase,
                           result,
                           _SHR_ERRMSG(result)));
            } else { /* if (thisGroup->rulebase < thisUnit->uMaxSupportedStages) */
                /* there was an error that left the state inconsistent */
                FIELD_ERR((FIELD_MSG1("it appears that unit %d group %08X is"
                                      " caught in limbo because of an error"
                                      " moving between rulebases: %d (%s)\n"),
                           thisUnit->unit,
                           group,
                           result,
                           _SHR_ERRMSG(result)));
                auxRes = _bcm_fe2k_g2p3_field_group_add(thisUnit,
                                                        oldRulebase,
                                                        group);
                if (BCM_E_NONE != auxRes) {
                    FIELD_ERR((FIELD_MSG1("unable to revert unit %d group %08X"
                                          " to rulebase %d: %d (%s)\n"),
                               thisUnit->unit,
                               group,
                               oldRulebase,
                               auxRes,
                               _SHR_ERRMSG(auxRes)));
                    auxRes = _bcm_fe2k_g2p3_field_group_free(thisUnit,
                                                             group);
                    if (BCM_E_NONE == auxRes) {
                        FIELD_ERR((FIELD_MSG1("destroyed unit %d group %08X"
                                              " to avoid inconsistent state\n"),
                                   thisUnit->unit,
                                   group));
                    } else { /* if (BCM_E_NONE != auxRes) */
                        FIELD_ERR((FIELD_MSG1("unit %d group %08X is in an"
                                              " inconsistent state and can"
                                              " only be destroyed, but doing"
                                              " so failed: %d (%s)\n"),
                                   thisUnit->unit,
                                   group,
                                   auxRes,
                                   _SHR_ERRMSG(auxRes)));
                    } /* if (BCM_E_NONE != auxRes) */
                } /* if (BCM_E_NONE != auxRes) */
            } /* if (thisGroup->rulebase < thisUnit->uMaxSupportedStages) */
        } /* if (BCM_E_NONE == result) */
    } else { /* if (rulebase != thisGroup->rulebase) */
        /* just update the qset */
        thisGroup->qset = qset;
        FIELD_EVERB((FIELD_MSG1("updated unit %d group %08X qset in place\n"),
                     thisUnit->unit,
                     group));
        result = BCM_E_NONE;
    } /* if (rulebase != thisGroup->rulebase) */
    if (BCM_E_NONE == result) {
        /* mark the qset for the stage, just in case it's not set */
        BCM_FIELD_QSET_ADD(thisGroup->qset, (thisGroup->rulebase?
                                             bcmFieldQualifyStageEgressSecurity:
                                             bcmFieldQualifyStageIngressQoS));
    }
    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_group_get
 *   Purpose
 *      Gets the group's qualifier set.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *      (out) bcm_field_qset_t *qset = where to put the current qset
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_g2p3_field_group_get(void *unitData,
                                bcm_field_group_t group,
                                bcm_field_qset_t *qset)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */
    _bcm_fe2k_g2p3_field_group_t *thisGroup;  /* working group data */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    if ((thisUnit->groupTotal <= group) || (0 > group)) {
        FIELD_ERR((FIELD_MSG1("can't get qset of invalid group ID %08X"
                              " on unit %d\n"),
                   group,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    thisGroup = &(thisUnit->group[group]);
    if (thisUnit->uMaxSupportedStages <= thisGroup->rulebase) {
        FIELD_ERR((FIELD_MSG1("unit %d group %08X is not in use\n"),
                   thisUnit->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }
    *qset = thisGroup->qset;

    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_group_destroy
 *   Purpose
 *      Destroys a group.
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      There must be no entries in this group when calling this function.
 */
int
bcm_fe2000_g2p3_field_group_destroy(void *unitData,
                                    bcm_field_group_t group)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */
#ifdef BCM_WARM_BOOT_SUPPORT
    uint rulebase;
#endif

    int result;                               /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    if ((thisUnit->groupTotal <= group) || (0 > group)) {
        FIELD_ERR((FIELD_MSG1("can not destroy invalid group %08X on"
                              " unit %d\n"),
                   group,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
#ifdef BCM_WARM_BOOT_SUPPORT
    rulebase = thisUnit->group[group].rulebase;
#endif
    result = _bcm_fe2k_g2p3_field_group_del(unitData, group);
    if ((BCM_E_NONE == result) || (BCM_E_CONFIG == result)) {
        /*
         *  We want to try to free the group if we got BCM_E_CONFIG because it
         *  could indicate bad state (a group that, for example, tried to move
         *  between rulebases and failed, getting caught in limbo).
         */
        result = _bcm_fe2k_g2p3_field_group_free(unitData, group);
        if (BCM_E_NONE  != result) {
            FIELD_ERR((FIELD_MSG1("unable to free group 0x%x: %d (%s)\n"),
                       group,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
    }

    if (BCM_E_NONE  != result) {
        FIELD_ERR((FIELD_MSG1("unable to delete group 0x%x: %d (%s)\n"),
                   group,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }
#ifdef BCM_WARM_BOOT_SUPPORT
    /* force rulebase commit to update hardware and scache */
    result = _bcm_fe2k_g2p3_field_rulebase_commit(thisUnit,
                                                  rulebase);
    if (BCM_E_NONE  != result) {
        FIELD_ERR((FIELD_MSG1("unable to commit rules to scache for"
                              " rulebase %d (SBX db %02X): %d (%s)\n"),
                   rulebase,
                   thisUnit->rulebase[rulebase].dbId,
                   result,
                   _SHR_ERRMSG(result)));
    }
#endif
    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_group_status_get
 *   Purpose
 *      Gets the group's status.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *      (out) bcm_field_group_status_t *status = where to put the status
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Frankly, the data returned by
 *      this call will look bogus to anybody used to the XGS implementation.
 */
int
bcm_fe2000_g2p3_field_group_status_get(void *unitData,
                                       bcm_field_group_t group,
                                       bcm_field_group_status_t *status)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */
    _bcm_fe2k_g2p3_field_rulebase_t *thisRb;  /* working rulebase data */
    _bcm_fe2k_g2p3_field_group_t *thisGroup;  /* working group data */
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;  /* working entry data */
    _bcm_fe2k_field_entry_index entry;        /* working entry ID */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    if ((thisUnit->groupTotal <= group) || (0 > group)) {
        FIELD_ERR((FIELD_MSG1("can't get qset of invalid group ID %08X"
                              " on unit %d\n"),
                   group,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    thisGroup = &(thisUnit->group[group]);
    if (thisUnit->uMaxSupportedStages <= thisGroup->rulebase) {
        FIELD_ERR((FIELD_MSG1("unit %d group %08X is not in use\n"),
                   thisUnit->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }
    thisRb = &(thisUnit->rulebase[thisGroup->rulebase]);

    /* make sure the buffer is empty */
    bcm_field_group_status_t_init(status);

    /* fill in the known capacities */
    status->entries_total = thisRb->entriesMax;
    status->counters_total = thisRb->entriesMax;
    status->meters_total = 0;

    /* walk the group and fill in things from that path */
    status->counter_count = 0;
    status->entry_count = 0;
    status->meter_count = 0;
    status->prio_min = _SBX_FE2K_FIELD_ENTRY_PRIO_HIGHEST;
    status->prio_max = BCM_FIELD_ENTRY_PRIO_LOWEST;
    for (entry = thisGroup->entryHead;
         entry < thisUnit->entryTotal;
         entry = thisUnit->entry[entry].nextEntry) {
        thisEntry = &(thisUnit->entry[entry]);
        status->counter_count++;
        if (0 < _bcm_fe2000_compare_entry_priority(status->prio_max,
                                                   thisEntry->priority)) {
            /* entry priority is higher than highest so far; keep it */
            status->prio_max = thisEntry->priority;
        }
        if (0 > _bcm_fe2000_compare_entry_priority(status->prio_min,
                                                   thisEntry->priority)) {
            /* entry priority is lower than lowest so far; keep it */
            status->prio_min = thisEntry->priority;
        }
    } /* for (all entries in this group) */

    /* fill in the free numbers */
    status->entries_free = thisRb->entriesMax - thisRb->entries;
    status->counters_free = status->entries_total - status->counter_count;
    status->meters_free = 0;

    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_range_create
 *   Purpose
 *      Create a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (out) bcm_field_range_t *range = where to put the assigned range ID
 *      (in) uint32 flags = flags for the range
 *      (in) bcm_l4_port_t min = low port number for the range
 *      (in) bcm_l4_port_t max = high port number for the range
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range ID is always nonzero.  This is translated here instead of using
 *      the list manager feature because we have ranges in a zero-based array.
 *      This, range_create_id, and range_destroy are the places where we hold
 *      two locks at the same time: one for this unit's field work, and one
 *      that is managed by the list manager.  They are always taken in the same
 *      order and always released in the same order (reverse from the order in
 *      which they are taken), so there should be no deadlock issues here.
 */
int
bcm_fe2000_g2p3_field_range_create(void *unitData,
                                   bcm_field_range_t *range,
                                   uint32 flags,
                                   bcm_l4_port_t min,
                                   bcm_l4_port_t max)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */
    uint32 tempRange;                         /* working range ID */
    int result;                               /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* try to reserve the range */
    result = shr_idxres_list_alloc(thisUnit->rangeFree, &tempRange);
    if (BCM_E_NONE == result) {
        /* the range was reserved; create it */
        FIELD_EVERB((FIELD_MSG1("create unit %d range"
                                " [alloc %08X] (%08X %08X..%08X)\n"),
                     thisUnit->unit,
                     tempRange + 1,
                     flags,
                     min,
                     max));
        result = _bcm_fe2000_g2p3_field_range_create_id(thisUnit,
                                                        tempRange,
                                                        flags,
                                                        min,
                                                        max);
        if (BCM_E_NONE != result) {
            /* something went wrong; destroy the range */
            FIELD_ERR((FIELD_MSG1("error creating unit %d range %08X,"
                                  " destroying it\n"),
                       thisUnit->unit,
                       *range));
            sal_memset(&(thisUnit->range[tempRange]),
                       0,
                       sizeof(thisUnit->range[tempRange]));
            shr_idxres_list_free(thisUnit->rangeFree, tempRange);
        }
    }
    if (BCM_E_NONE == result) {
        *range = tempRange + 1;
    }
    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_range_create_id
 *   Purpose
 *      Create a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_range_t range = the range ID to use
 *      (in) uint32 flags = flags for the range
 *      (in) bcm_l4_port_t min = low port number for the range
 *      (in) bcm_l4_port_t max = high port number for the range
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range ID is always nonzero.  This is translated here instead of using
 *      the list manager feature because we have ranges in a zero-based array.
 *      This, range_create, and range_destroy are the places where we hold two
 *      locks at the same time: one for this unit's field work, and one that is
 *      managed by the list manager.  They are always taken in the same
 *      order and always released in the same order (reverse from the order in
 *      which they are taken), so there should be no deadlock issues here.
 */
int
bcm_fe2000_g2p3_field_range_create_id(void *unitData,
                                      bcm_field_range_t range,
                                      uint32 flags,
                                      bcm_l4_port_t min,
                                      bcm_l4_port_t max)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */
    uint32 tempRange;                         /* working range ID */
    int result;                               /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* try to reserve the range */
    tempRange = range - 1;
    result = shr_idxres_list_reserve(thisUnit->rangeFree,
                                     tempRange,
                                     tempRange);
    if (BCM_E_NONE == result) {
        /* the range was reserved; create it */
        FIELD_EVERB((FIELD_MSG1("create unit %d range"
                                " %08X (%08X %08X..%08X)\n"),
                     thisUnit->unit,
                     range,
                     flags,
                     min,
                     max));
        result = _bcm_fe2000_g2p3_field_range_create_id(thisUnit,
                                                        tempRange,
                                                        flags,
                                                        min,
                                                        max);
        if (BCM_E_NONE != result) {
            /* something went wrong; destroy the range */
            FIELD_ERR((FIELD_MSG1("error creating unit %d range %08X,"
                                  " destroying it\n"),
                       thisUnit->unit,
                       range));
            sal_memset(&(thisUnit->range[tempRange]),
                       0,
                       sizeof(thisUnit->range[tempRange]));
            shr_idxres_list_free(thisUnit->rangeFree, tempRange);
        }
    } else if (BCM_E_RESOURCE == result) {
        /* translate 'resource not available' to 'it already exists' */
        result = BCM_E_EXISTS;
    } else if (BCM_E_PARAM == result) {
        /* translate 'bogus ID' to 'not found' (silly?) */
        result = _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_range_get
 *   Purpose
 *      Create a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_range_t range = the range ID to use
 *      (in) uint32 *flags = where to put the flags for the range
 *      (in) bcm_l4_port_t *min = where to put range's low port number
 *      (in) bcm_l4_port_t *max = where to put range's high port number
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range ID is always nonzero.
 */
int
bcm_fe2000_g2p3_field_range_get(void *unitData,
                                bcm_field_range_t range,
                                uint32 *flags,
                                bcm_l4_port_t *min,
                                bcm_l4_port_t *max)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */
    uint32 tempRange;                         /* working range ID */
    int result;                               /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* try to access the range */
    tempRange = range - 1;
    result = shr_idxres_list_elem_state(thisUnit->rangeFree, tempRange);
    if (BCM_E_EXISTS == result) {
        /* the range is allocated; adjust return code */
        FIELD_EVERB((FIELD_MSG1("get range %08X information\n"), range));
        result = BCM_E_NONE;
        /* fill in the values */
        *flags = thisUnit->range[tempRange].flags;
        *min = thisUnit->range[tempRange].min;
        *max = thisUnit->range[tempRange].max;
    } else if (BCM_E_PARAM == result) {
        /* translate 'bogus ID' to 'not found' (silly?) */
        result = _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_range_destroy
 *   Purpose
 *      Destroy a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_range_t range = the range ID to destroy
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This, range_create, and range_create_id are the places where we hold
 *      two locks at the same time: one for this unit's field work, and one
 *      that is managed by the list manager.  They are always taken in the same
 *      order and always released in the same order (reverse from the order in
 *      which they are taken), so there should be no deadlock issues here.
 *      Destroying a range has absolutely no effect upon the rules that were
 *      set up using that range, at least on this hardware.
 */
int
bcm_fe2000_g2p3_field_range_destroy(void *unitData,
                                    bcm_field_range_t range)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */
    uint32 tempRange;                         /* working range ID */
    int result;                               /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* try to access the range */
    tempRange = range - 1;
    result = shr_idxres_list_elem_state(thisUnit->rangeFree, tempRange);
    if (BCM_E_EXISTS == result) {
        /* the range is allocated; destroy it */
        FIELD_EVERB((FIELD_MSG1("destroy range %08X\n"), range));
        sal_memset(&(thisUnit->range[tempRange]),
                   0,
                   sizeof(thisUnit->range[tempRange]));
        /* free the range */
        result = shr_idxres_list_free(thisUnit->rangeFree, tempRange);
    } else if (BCM_E_PARAM == result) {
        /* translate 'bogus ID' to 'not found' (silly?) */
        result = _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_entry_create
 *   Purpose
 *      Create an empty field entry based upon the specified grup
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_group_t group = the group ID to use
 *      (out) bcm_field_entry_t *entry = where to put the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Annoyingly, this function can only insert the entry into the group
 *      based upon a priority setting of BCM_FIELD_ENTRY_PRIO_DEFAULT, and it
 *      will be moved later if the user actually bothers to set the priority.
 */
int
bcm_fe2000_g2p3_field_entry_create(void *unitData,
                                   bcm_field_group_t group,
                                   bcm_field_entry_t *entry)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */
    _bcm_fe2k_field_entry_index entryId;      /* working entry ID */
    int result;                               /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check argument validity */
    if ((thisUnit->groupTotal <= group) || (0 > group)) {
        /* group ID is invalid */
        FIELD_ERR((FIELD_MSG1("invalid group %08X specified on unit %d\n"),
                   group,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    if (thisUnit->uMaxSupportedStages <= thisUnit->group[group].rulebase) {
        /* group is not in use */
        FIELD_ERR((FIELD_MSG1("unit %d group %08X is not in use\n"),
                   thisUnit->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }
    /* don't care about the entry ID */
    entryId = thisUnit->entryTotal;
    /* create the new entry */
    result = _bcm_fe2k_g2p3_field_entry_create(thisUnit, group, &entryId);
    if (BCM_E_NONE == result) {
        /* success */
        *entry = entryId;
    }
    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_entry_create_id
 *   Purpose
 *      Create an empty field entry based upon the specified grup
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_group_t group = the group ID to use
 *      (in) bcm_field_entry_t entry = the entry ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Note
 *      Annoyingly, this function can only insert the entry into the group
 *      based upon a priority setting of BCM_FIELD_ENTRY_PRIO_DEFAULT, and it
 *      will be moved later if the user actually bothers to set the priority.
 */
int
bcm_fe2000_g2p3_field_entry_create_id(void *unitData,
                                      bcm_field_group_t group,
                                      bcm_field_entry_t entry)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */
    _bcm_fe2k_field_entry_index entryId;      /* working entry ID */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check argument validity */
    if ((thisUnit->groupTotal <= group) || (0 > group)) {
        /* group ID is invalid */
        FIELD_ERR((FIELD_MSG1("invalid group %08X specified on unit %d\n"),
                   group,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        FIELD_ERR((FIELD_MSG1("invalid entry %08X specified on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    if (thisUnit->uMaxSupportedStages <= thisUnit->group[group].rulebase) {
        /* group is not in use */
        FIELD_ERR((FIELD_MSG1("unit %d group %08X is not in use\n"),
                   thisUnit->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }
    /* use the specified entry ID */
    entryId = entry;
    /* create the new entry */
    return _bcm_fe2k_g2p3_field_entry_create(thisUnit, group, &entryId);
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_entry_destroy
 *   Purpose
 *      Destroy a field entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID to destroy
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Does not remove any associated entries from the hardware.[!?]
 */
int
bcm_fe2000_g2p3_field_entry_destroy(void *unitData,
                                    bcm_field_entry_t entry)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check parameters */
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        FIELD_ERR((FIELD_MSG1("invalid entry %08X specified on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }

    return _bcm_fe2k_g2p3_field_entry_destroy(thisUnit, entry);
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_entry_destroy_all
 *   Purpose
 *      Destroy all field entries
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The API spec indicates that this function does not remove any
 *      associated entries from the hardware.  Unfortunately, if we're to avoid
 *      resource leakage, we can't honour this.  We don't *commit* the database
 *      after this call, so the entries are still in the actual hardware, but
 *      they're no longer in the SBX driver, so next commit they *will* die.
 */
int
bcm_fe2000_g2p3_field_entry_destroy_all(void *unitData)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */
    _bcm_fe2k_field_entry_index entry;        /* working entry ID */
    int result = BCM_E_NONE;                  /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* destroy all (active) entries */
    for (entry = 0; entry < thisUnit->entryTotal; entry++) {
        if (thisUnit->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_VALID) {
            result = bcm_fe2000_g2p3_field_entry_destroy(unitData, entry);
        }
        if (BCM_E_NONE != result) {
            FIELD_ERR((FIELD_MSG1("unable to delete unit %d entry %08X:"
                                  " %d (%s); aborting\n"),
                       thisUnit->unit,
                       entry,
                       result,
                       _SHR_ERRMSG(result)));
            break;
        }
    }
    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_entry_copy
 *   Purpose
 *      Copy an existing field entry to another one
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t src_entry = the original entry ID
 *      (out) bcm_field_entry_t *dst_entry = where to put the copy entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This can only copy the entry within its group, and the copy will be
 *      inserted as the last entry of the priority.  If the original entry is
 *      participating in counter sharing, so is the copy; if not, neither is
 *      the copy (but if the original had a counter allocated, so will the
 *      copy, though it will be a different counter if not sharing).
 */
int
bcm_fe2000_g2p3_field_entry_copy(void *unitData,
                                 bcm_field_entry_t src_entry,
                                 bcm_field_entry_t *dst_entry)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */
    _bcm_fe2k_field_entry_index entryId;      /* working entry ID */
    int result;                               /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check argument validity */
    if ((thisUnit->entryTotal <= src_entry) || (0 > src_entry)) {
        /* source entry ID is invalid */
        FIELD_ERR((FIELD_MSG1("invalid source entry %08X specified"
                              " on unit %d\n"),
                   src_entry,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    /* don't care about the new entry ID */
    entryId = thisUnit->entryTotal;
    /* create the new entry */
    result = _bcm_fe2k_g2p3_field_entry_copy(thisUnit, src_entry, &entryId);
    if (BCM_E_NONE == result) {
        /* success */
        *dst_entry = entryId;
    }
    return result;
}


/*
 *   Function
 *      bcm_fe2000_g2p3_field_entry_copy_id
 *   Purpose
 *      Copy an existing field entry to a specific one
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t src_entry = the original entry ID
 *      (in) bcm_field_entry_t dst_entry = the copy entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This can only copy the entry within its group, and the copy will be
 *      inserted as the last entry of the priority.  If the original entry is
 *      participating in counter sharing, so is the copy; if not, neither is
 *      the copy (but if the original had a counter allocated, so will the
 *      copy, though it will be a different counter if not sharing).
 */
int
bcm_fe2000_g2p3_field_entry_copy_id(void *unitData,
                                    bcm_field_entry_t src_entry,
                                    bcm_field_entry_t dst_entry)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */
    _bcm_fe2k_field_entry_index entryId;      /* working entry ID */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check argument validity */
    if ((thisUnit->entryTotal <= src_entry) || (0 > src_entry)) {
        /* source entry ID is invalid */
        FIELD_ERR((FIELD_MSG1("invalid source entry %08X specified"
                              " on unit %d\n"),
                   src_entry,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    if ((thisUnit->entryTotal <= dst_entry) || (0 > dst_entry)) {
        /* destination entry ID is invalid */
        FIELD_ERR((FIELD_MSG1("invalid destination entry %08X specified"
                              " on unit %d\n"),
                   src_entry,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    /* set up for the desired new entry ID */
    entryId = dst_entry;
    /* create the new entry */
    return _bcm_fe2k_g2p3_field_entry_copy(thisUnit, src_entry, &entryId);
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_entry_install
 *   Purpose
 *      Install a field entry to the hardware
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID to destroy
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This API is one of a set that will cause poor performance on SBX.
 *      This causes an error if the entry is already in hardware.
 *      This will commit the appropriate database to the hardware.
 */
int
bcm_fe2000_g2p3_field_entry_install(void *unitData,
                                    bcm_field_entry_t entry)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;  /* working entry data */
    int result;                               /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check argument validity */
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        FIELD_ERR((FIELD_MSG1("invalid entry %08X specified on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    thisEntry = &(thisUnit->entry[entry]);
    if (!(thisEntry->entryFlags & _FE2K_G2P3_ENTRY_VALID)) {
        /* specified entry is not used */
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X is not in use\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    if (thisEntry->entryFlags & _FE2K_G2P3_ENTRY_IN_HW) {
        /* entry is already in hardware */
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X is already installed\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_EXISTS;
    }
    result = _bcm_fe2k_g2p3_field_entry_set_install(thisUnit, entry, TRUE);
    if (BCM_E_NONE == result) {
        result = _bcm_fe2k_g2p3_field_rulebase_commit(thisUnit,
                                                      thisUnit->group[thisEntry->group].rulebase);
    }
    /* both of those functions display any appropriate error */
    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_entry_reinstall
 *   Purpose
 *      Reinstall a field entry to the hardware
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID to destroy
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Apprarently, despite the API doc indicating this can only be used to
 *      refresh an entry that is already in hardware, the regression tests
 *      require that this work to install an entry that is not in hardware.
 */
int
bcm_fe2000_g2p3_field_entry_reinstall(void *unitData,
                                      bcm_field_entry_t entry)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;  /* working entry data */
    int result;                               /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check argument validity */
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        FIELD_ERR((FIELD_MSG1("invalid entry %08X specified on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    thisEntry = &(thisUnit->entry[entry]);
    if (!(thisEntry->entryFlags & _FE2K_G2P3_ENTRY_VALID)) {
        /* specified entry is not used */
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X is not in use\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    result = _bcm_fe2k_g2p3_field_entry_set_install(thisUnit, entry, TRUE);
    if (BCM_E_NONE == result) {
        result = _bcm_fe2k_g2p3_field_rulebase_commit(thisUnit,
                                                      thisUnit->group[thisEntry->group].rulebase);
    }
    /* both of those functions display any appropriate error */
    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_entry_remove
 *   Purpose
 *      Remove a field entry from the hardware
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID to destroy
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The API doc indicates that this can only be used against an entry that
 *      is already in hardware, but the regression tests require that it work
 *      even if the entry isn't in hardware.
 */
int
bcm_fe2000_g2p3_field_entry_remove(void *unitData,
                                   bcm_field_entry_t entry)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;  /* working entry data */
    int result;                               /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check argument validity */
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        FIELD_ERR((FIELD_MSG1("invalid entry %08X specified on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    thisEntry = &(thisUnit->entry[entry]);
    if (!(thisEntry->entryFlags & _FE2K_G2P3_ENTRY_VALID)) {
        /* specified entry is not used */
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X is not in use\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    /*
     *  Should check and return error if not in hardware according to API doc,
     *  but regression tests demand that this call succeed in that case.
     */
    result = _bcm_fe2k_g2p3_field_entry_set_install(thisUnit, entry, FALSE);
    if (BCM_E_NONE == result) {
        result = _bcm_fe2k_g2p3_field_rulebase_commit(thisUnit,
                                                      thisUnit->group[thisEntry->group].rulebase);
    }
    /* both of those functions display any appropriate error */
    return result;
}


/*
 *   Function
 *      bcm_fe2000_g2p3_field_entry_prio_get
 *   Purpose
 *      Get the priority of a specific entry (within its group)
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (out) int *prio = where to put the entry's priority
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The field entry identifier is NOT the priority of the entry in the
 *      group on in the system.
 */
int
bcm_fe2000_g2p3_field_entry_prio_get(void *unitData,
                                     bcm_field_entry_t entry,
                                     int *prio)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        FIELD_ERR((FIELD_MSG1("invalid entry %08X specified"
                              " on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    if (!(thisUnit->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_VALID)) {
        /* entry is not in use */
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X is not in use\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    *prio = thisUnit->entry[entry].priority;
    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_entry_prio_set
 *   Purpose
 *      Set the priority of a specific entry (within its group)
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int prio = the entry's new priority
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The field entry identifier is NOT the priority of the entry in the
 *      group on in the system.
 *      Priority is signed; nonnegative numbers are priority order; negative
 *      numbers have special meanings.
 *      Overall sort is:
 *          highest >= numbered >= dontcare >= lowest
 */
int
bcm_fe2000_g2p3_field_entry_prio_set(void *unitData,
                                     bcm_field_entry_t entry,
                                     int prio)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        FIELD_ERR((FIELD_MSG1("invalid entry %08X specified"
                              " on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    if (0 > prio) {
        FIELD_ERR((FIELD_MSG1("invalid entry priority %d specified for"
                              " unit %d entry %08X\n"),
                   prio,
                   thisUnit->unit,
                   entry));
        return BCM_E_PARAM;
    }
    if (!(thisUnit->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_VALID)) {
        /* entry is not in use */
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X is not in use\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    /* get the entry qualifiers and actions so we can preserve them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* remove the entry from its current position in its group */
    result = _bcm_fe2k_g2p3_field_entry_del(thisUnit, entry);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* change the priority */
    if (prio == BCM_FIELD_ENTRY_PRIO_HIGHEST) {
        thisUnit->entry[entry].priority = _SBX_FE2K_FIELD_ENTRY_PRIO_HIGHEST;
    }else{
        thisUnit->entry[entry].priority = prio;
    }
    /* reinsert the entry to the group according to its new priority */
    return _bcm_fe2k_g2p3_field_entry_add(thisUnit,
                                          thisUnit->entry[entry].group,
                                          entry,
                                          &rule);
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_qualify_clear
 *   Purpose
 *      Clear all qualifiers for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_g2p3_field_qualify_clear(void *unitData,
                                    bcm_field_entry_t entry)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        FIELD_ERR((FIELD_MSG1("invalid entry %08X specified"
                              " on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    if (!(thisUnit->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_VALID)) {
        /* entry is not in use */
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X is not in use\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* remove the qualifiers from this entry's rules */
    result = _bcm_fe2k_g2p3_field_rules_clear_qualifiers(thisUnit,
                                                         entry,
                                                         &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* update the entry with the new rules */
    return _bcm_fe2k_g2p3_field_rules_resync(thisUnit, entry, &rule);
}

/*
 *   Function
 *      _bcm_fe2000_g2p3_field_qualify_Port
 *   Purpose
 *      Set expected port qualifier for an entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_gport_t gport = gport as qualifier
 *      (in) bcm_field_qualify_t qual = which qualifier to consider
 *      (in) int apply = TRUE to apply, FALSE to remove
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the outer VLAN ID to be masked (all are always
 *      significant if any are significant).
 *      Accepts 'gport' for port, and will dissect it to find the hardware port
 *      if applicable.
 *      Clobbers 'ports' qualifiers.
 */
static int
_bcm_fe2000_g2p3_field_qualify_Port(_bcm_fe2k_g2p3_field_glob_t *thisUnit,
                                    const bcm_field_entry_t entry,
                                    const bcm_gport_t gport,
                                    const bcm_field_qualify_t qual,
                                    const int apply)
{
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    bcm_pbmp_t ports;                           /* ports to include */
    bcm_pbmp_t pb;                              /* ports */
    int result;                                 /* working result */
    bcm_module_t locMod = -1;                   /* local module */
    bcm_module_t tgtMod = -1;                   /* target module */
    bcm_port_t tgtPort = -1;                    /* target port */

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   qual);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (apply) {
        /* adding port qualifier; map the target port */
        result = _bcm_fe2000_map_vlan_gport_target(thisUnit->unit,
                                                   gport,
                                                   &locMod,
                                                   &tgtMod,
                                                   &tgtPort,
                                                   NULL,
                                                   NULL);
        if (BCM_E_NONE != result) {
            FIELD_ERR((FIELD_MSG1("unable to get target data for unit %d"
                                  " port %08X: %d (%s)\n"),
                       thisUnit->unit,
                       gport,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
        if (tgtMod != locMod) {
            FIELD_ERR((FIELD_MSG1("unit %d port %08X is not local,"
                                  " so can't set it as a filter\n"),
                       thisUnit->unit,
                       gport));
            return BCM_E_PARAM;
        }
    } else { /* if (apply) */
        tgtPort = 0;
    }

    /* make sure the new port (if it's limited) is allowed */
    if (apply) {
        sal_memset(&ports, 0x00, sizeof(ports));
        BCM_PBMP_PORT_ADD(ports, tgtPort);
    } else {
        /* entry will apply to all ports */
        BCM_PBMP_ASSIGN(ports,
                        thisUnit->rulebase[thisUnit->group[thisUnit->entry[entry].group].rulebase].ports);
    }

    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* now update the entry to use the port */
    BCM_PBMP_ASSIGN(pb, ports);
    BCM_PBMP_AND(pb, thisUnit->rulebase[thisUnit->group[thisUnit->entry[entry].group].rulebase].ports);
    switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
    case 0:
        FIELD_PBMP_TO_SBX(rule.ingress.pbmpn, pb);
        break;
    case 1:
        FIELD_PBMP_TO_SBX(rule.egress.pbmpn, pb);
        break;
    case 2:
        FIELD_PBMP_TO_SBX(rule.ingress_v6.pbmpn, pb);
        break;
    case 3:
        FIELD_PBMP_TO_SBX(rule.egress_v6.pbmpn, pb);
        break;
    default:
        /* should never reach this */
        FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                              " with supported\n"),
                   thisUnit->unit,
                   thisUnit->entry[entry].group));
        return BCM_E_INTERNAL;
    } /* switch (rulebase) */
    FIELD_EVERB((FIELD_MSG1("write Port(%08X) qualifier to"
                            " unit %d entry %08X\n"),
                 gport,
                 thisUnit->unit,
                 entry));
    /* update entry, also creating/destroying rules as needed */
    result = _bcm_fe2k_g2p3_field_rules_resync(thisUnit, entry, &rule);
    /* the above call will display any error messages itself */
    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_qualify_InPort
 *   Purpose
 *      Set expected outer VLAN for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_port_t data = which ingress port
 *      (in) bcm_port_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the outer VLAN ID to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_fe2000_g2p3_field_qualify_InPort(void *unitData,
                                     bcm_field_entry_t entry,
                                     bcm_port_t data,
                                     bcm_port_t mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* make sure the mask is valid (subcall does the rest) */
    if (((~0) != mask) && (0 != mask)) {
        FIELD_ERR((FIELD_MSG1("unit %d only supports all-or-nothing %s mask\n"),
                   thisUnit->unit,
                   _sbx_fe2000_field_qual_name[bcmFieldQualifyInPort]));
        return BCM_E_PARAM;
    }
    /* update the port */
    return _bcm_fe2000_g2p3_field_qualify_Port(thisUnit,
                                               entry,
                                               data,
                                               bcmFieldQualifyInPort,
                                               0 != mask);
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_qualify_OutPort
 *   Purpose
 *      Set expected outer VLAN for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_port_t data = which egress port
 *      (in) bcm_port_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the outer VLAN ID to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_fe2000_g2p3_field_qualify_OutPort(void *unitData,
                                      bcm_field_entry_t entry,
                                      bcm_port_t data,
                                      bcm_port_t mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* make sure the mask is valid (subcall does the rest) */
    if (((~0) != mask) && (0 != mask)) {
        FIELD_ERR((FIELD_MSG1("unit %d only supports all-or-nothing %s mask\n"),
                   thisUnit->unit,
                   _sbx_fe2000_field_qual_name[bcmFieldQualifyOutPort]));
        return BCM_E_PARAM;
    }
    /* update the port */
    return _bcm_fe2000_g2p3_field_qualify_Port(thisUnit,
                                               entry,
                                               data,
                                               bcmFieldQualifyOutPort,
                                               0 != mask);
}

/*
 *   Function
 *      _bcm_fe2000_g2p3_field_qualify_Ports
 *   Purpose
 *      Set expected ports qualifier for an entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_pbmp_t ports = which ports to allow
 *      (in) bcm_field_qualify_t qual = which qualifier to consider
 *      (in) int apply = TRUE to apply, FALSE to remove
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the outer VLAN ID to be masked (all are always
 *      significant if any are significant).
 *      Accepts 'gport' for port, and will dissect it to find the hardware port
 *      if applicable.
 */
static int
_bcm_fe2000_g2p3_field_qualify_Ports(_bcm_fe2k_g2p3_field_glob_t *thisUnit,
                                     const bcm_field_entry_t entry,
                                     const bcm_pbmp_t eports,
                                     const bcm_field_qualify_t qual,
                                     const int apply)
{
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    bcm_pbmp_t ports;                           /* ports to include */
    bcm_pbmp_t pb;                              /* ports */
    int result;                                 /* working result */

    /* check entry valid, exists, and group has proper item in its qset, &c */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   qual);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (apply) {
        BCM_PBMP_ASSIGN(ports, eports);
        BCM_PBMP_AND(ports, thisUnit->rulebase[thisUnit->group[thisUnit->entry[entry].group].rulebase].ports);
    } else {
        BCM_PBMP_ASSIGN(ports, thisUnit->rulebase[thisUnit->group[thisUnit->entry[entry].group].rulebase].ports);
    }
    if (BCM_PBMP_IS_NULL(ports)) {
        FIELD_ERR((FIELD_MSG1("must provide at least one acceptable port\n")));
        return BCM_E_CONFIG;
    }

    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* hope for success */
    result = BCM_E_NONE;

    /* try to update the rules in the entry according to the pbmp */
    BCM_PBMP_ASSIGN(pb, ports);
    BCM_PBMP_AND(pb, thisUnit->rulebase[thisUnit->group[thisUnit->entry[entry].group].rulebase].ports);
    switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
    case 0:
        FIELD_PBMP_TO_SBX(rule.ingress.pbmpn, pb);
        break;
    case 1:
        FIELD_PBMP_TO_SBX(rule.egress.pbmpn, pb);
        break;
    case 2:
        FIELD_PBMP_TO_SBX(rule.ingress_v6.pbmpn, pb);
        break;
    case 3:
        FIELD_PBMP_TO_SBX(rule.egress_v6.pbmpn, pb);
        break;
    default:
        /* should never reach this */
        FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                              " with supported\n"),
                   thisUnit->unit,
                   thisUnit->entry[entry].group));
        return BCM_E_INTERNAL;
    } /* switch (rulebase) */
    FIELD_EVERB((FIELD_MSG1("write Ports(" FIELD_PBMP_FORMAT
                            ") qualifier to unit %d entry %08X\n"),
                 FIELD_PBMP_SHOW(ports),
                 thisUnit->unit,
                 entry));
    /* update entry, also creating/destroying rules as needed */
    result = _bcm_fe2k_g2p3_field_rules_resync(thisUnit, entry, &rule);
    /* the above call will display any error messages itself */
    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_qualify_InPorts
 *   Purpose
 *      Set allowed ingress ports for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_pbmp_t data = allowed ports
 *      (in) bcm_pbmp_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be all ports, PBMP_ALL(unit), or no ports.  Partial masking
 *      is not supported.
 */
int
bcm_fe2000_g2p3_field_qualify_InPorts(void *unitData,
                                      bcm_field_entry_t entry,
                                      bcm_pbmp_t data,
                                      bcm_pbmp_t mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    bcm_pbmp_t pbmp;                            /* working port bitmap */
    bcm_pbmp_t pbmpC;                           /* working port bitmap */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    BCM_PBMP_CLEAR(pbmpC);
    BCM_PBMP_NEGATE(pbmp, pbmpC);
    if (BCM_PBMP_NEQ(pbmp, mask) &&
        BCM_PBMP_NEQ(PBMP_ALL(thisUnit->unit),mask) &&
        BCM_PBMP_NEQ(PBMP_PORT_ALL(thisUnit->unit),mask) &&
        BCM_PBMP_NEQ(pbmpC, mask)) {
        /* mask is not all bits set or all valid port bits set or none set */
        FIELD_ERR((FIELD_MSG1("unit %d only supports all-or-nothing %s mask\n"),
                   thisUnit->unit,
                   _sbx_fe2000_field_qual_name[bcmFieldQualifyInPorts]));
        return BCM_E_PARAM;
    }
    BCM_PBMP_NEGATE(pbmpC, mask);
    BCM_PBMP_ASSIGN(pbmp, data);
    BCM_PBMP_AND(pbmp, pbmpC);
    if (BCM_PBMP_NOT_NULL(pbmp)) {
        FIELD_ERR((FIELD_MSG1("ports " FIELD_PBMP_FORMAT " in data but not"
                              " in mask " FIELD_PBMP_FORMAT "\n"),
                   FIELD_PBMP_SHOW(pbmp),
                   FIELD_PBMP_SHOW(mask)));
        return BCM_E_PARAM;
    }
    /* update the port */
    return _bcm_fe2000_g2p3_field_qualify_Ports(thisUnit,
                                                entry,
                                                data,
                                                bcmFieldQualifyInPorts,
                                                BCM_PBMP_NOT_NULL(mask));
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_qualify_OutPorts
 *   Purpose
 *      Set allowed ingress ports for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_pbmp_t data = allowed ports
 *      (in) bcm_pbmp_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be all ports, PBMP_ALL(unit), or no ports.  Partial masking
 *      is not supported.
 */
int
bcm_fe2000_g2p3_field_qualify_OutPorts(void *unitData,
                                       bcm_field_entry_t entry,
                                       bcm_pbmp_t data,
                                       bcm_pbmp_t mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    bcm_pbmp_t pbmp;                            /* working port bitmap */
    bcm_pbmp_t pbmpC;                           /* working port bitmap */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    BCM_PBMP_CLEAR(pbmpC);
    BCM_PBMP_NEGATE(pbmp, pbmpC);
    if (BCM_PBMP_NEQ(pbmp, mask) &&
        BCM_PBMP_NEQ(PBMP_ALL(thisUnit->unit),mask) &&
        BCM_PBMP_NEQ(pbmpC, mask)) {
        /* mask is not all bits set or all valid port bits set or none set */
        FIELD_ERR((FIELD_MSG1("unit %d only supports all-or-nothing %s mask\n"),
                   thisUnit->unit,
                   _sbx_fe2000_field_qual_name[bcmFieldQualifyOutPorts]));
        return BCM_E_PARAM;
    }
    BCM_PBMP_NEGATE(pbmpC, mask);
    BCM_PBMP_ASSIGN(pbmp, data);
    BCM_PBMP_AND(pbmp, pbmpC);
    if (BCM_PBMP_NOT_NULL(pbmp)) {
        FIELD_ERR((FIELD_MSG1("ports " FIELD_PBMP_FORMAT " in data but not"
                              " in mask " FIELD_PBMP_FORMAT "\n"),
                   FIELD_PBMP_SHOW(pbmp),
                   FIELD_PBMP_SHOW(mask)));
        return BCM_E_PARAM;
    }
    /* update the port */
    return _bcm_fe2000_g2p3_field_qualify_Ports(thisUnit,
                                                entry,
                                                data,
                                                bcmFieldQualifyOutPorts,
                                                BCM_PBMP_NOT_NULL(mask));
}

/*
 *   Function
 *      _bcm_fe2000_g2p3_field_qualify_Ports_get
 *   Purpose
 *      Get expected port(s) qualifier for an entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_field_qualify_t qual = which qualifier to consider
 *      (out) bcm_gport_t *gport = where to put 'port' qualifier data
 *      (out) bcm_gport_t *gpmask = where to put 'port' qualifier mask
 *      (out) bcm_pbmp_t *pbmp = where to put 'ports' qualifier data
 *      (out) bcm_pbmp_t *pbmask = where to put 'ports' qualifier mask
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Will only fill in gport or pbmp, according to the qualifier given.
 *
 *      Will return BCM_E_CONFIG if can't represent appropriately (such as
 *      multiple ports in 'ports' qualifier but asked for 'port' qualifier).
 *
 *      Returned GPORT is ModPort variety, if 'port' mode.
 *
 *      The unwanted return pointer can be NULL.
 *
 *      Will return BCM_E_NOT_FOUND if the entry applies to all ports 
 */
static int
_bcm_fe2000_g2p3_field_qualify_Ports_get(_bcm_fe2k_g2p3_field_glob_t *thisUnit,
                                         const bcm_field_entry_t entry,
                                         const bcm_field_qualify_t qual,
                                         bcm_port_t *gport,
                                         bcm_port_t *gpmask,
                                         bcm_pbmp_t *pbmp,
                                         bcm_pbmp_t *pbmask)
{
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */
    int multiple = FALSE;                       /* found multiple ports */
    bcm_pbmp_t ports;                           /* local ports */
    bcm_pbmp_t mask;                            /* local ports mask */
    bcm_pbmp_t pb;                              /* port bitmap */
    bcm_pbmp_t pm;                              /* port mask */
    bcm_port_t port;                            /* local port */
    bcm_port_t pIndex;                          /* working port index */

    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   qual);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* collect the data for the requested qualifier */
    result = BCM_E_NOT_FOUND; /* but assume no port restriction */
    BCM_PBMP_CLEAR(ports);
    BCM_PBMP_CLEAR(mask);
    /* keep track of affected ports */
    BCM_PBMP_ASSIGN(pm, thisUnit->rulebase[thisUnit->group[thisUnit->entry[entry].group].rulebase].ports);
    BCM_PBMP_OR(mask, pm);
    if (thisUnit->entry[entry].hwRule) {
        /* hardware has a rule */
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            /* ingress */
            FIELD_PBMP_FROM_SBX(pb, rule.ingress.pbmpn, thisUnit->unit);
            break;
        case 1:
            /* egress */
            FIELD_PBMP_FROM_SBX(pb, rule.egress.pbmpn, thisUnit->unit);
            break;
        case 2:
            /* ingress v6 */
            FIELD_PBMP_FROM_SBX(pb, rule.ingress_v6.pbmpn, thisUnit->unit);
            break;
        case 3:
            /* egress v6 */
            FIELD_PBMP_FROM_SBX(pb, rule.egress_v6.pbmpn, thisUnit->unit);
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) */
        BCM_PBMP_OR(ports, pb);
        if (!BCM_PBMP_EQ(pm, pb)) {
            /* ports qualifier mismatch */
            result = BCM_E_NONE;
            multiple = TRUE;
        }
    } else { /* if (thisUnit->entry[entry].hwRule) */
        /* didn't find one; entry has no rules for some reason */
        FIELD_ERR((FIELD_MSG1("unable to locate a rule for unit %d"
                              " entry %08X\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_INTERNAL;
    } /* if (thisUnit->entry[entry]hwRule) */
    /* make sure the current state makes sense */
    if (BCM_E_NOT_FOUND == result) {
        /* no port restriction */
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X has no port qualification\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }

    /* return the parsed data according to which mode we want */
    switch (qual) {
    case bcmFieldQualifyInPorts:
    case bcmFieldQualifyOutPorts:
        /* want port bitmap output */
        sal_memcpy(pbmp, &ports, sizeof(*pbmp));
        sal_memcpy(pbmask, &mask, sizeof(*pbmask));
        break;
    case bcmFieldQualifyInPort:
    case bcmFieldQualifyOutPort:
        /* want only single port output */
        multiple = FALSE;
        port = -1; 
        BCM_PBMP_ITER(ports, pIndex) {
            if (multiple) {
                FIELD_ERR((FIELD_MSG1("a single-port qualifier can not be"
                                      " read on an entry that is qualified"
                                      " against multiple ports\n")));
                return BCM_E_CONFIG;
            } else {
                port = pIndex;
                multiple = TRUE;
            }
        }
        if (multiple) {
            *gport = port;
            *gpmask = ~0;
        } else {
            FIELD_ERR((FIELD_MSG1("could not find any ports to which unit"
                                  " %d entry %08X applies\n"),
                       thisUnit->unit,
                       entry));
            return BCM_E_NOT_FOUND;
        }
        break;
    default:
        /* should never get to here */
        FIELD_ERR((FIELD_MSG1("unknown port class qualifier %d\n"),
                   qual));
        return BCM_E_INTERNAL;
    } /* switch (qual) */
    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_qualify_InPort
 *   Purpose
 *      Get expected outer VLAN for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (out) bcm_vlan_t *data = where to put ingress port
 *      (out) bcm_vlan_t *mask = where to put ingress port mask
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Will return BCM_E_CONFIG if entry is qualified on multiple ports.
 */
int
bcm_fe2000_g2p3_field_qualify_InPort_get(void *unitData,
                                         bcm_field_entry_t entry,
                                         bcm_port_t *data,
                                         bcm_port_t *mask)
{
    /* get the port qualifier */
    return _bcm_fe2000_g2p3_field_qualify_Ports_get((_bcm_fe2k_g2p3_field_glob_t*)unitData,
                                                    entry,
                                                    bcmFieldQualifyInPort,
                                                    data,
                                                    mask,
                                                    NULL,
                                                    NULL);
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_qualify_OutPort_get
 *   Purpose
 *      Get expected outer VLAN for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (out) bcm_vlan_t *data = where to put ingress port
 *      (out) bcm_vlan_t *mask = where to put ingress port mask
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Will return BCM_E_CONFIG if entry is qualified on multiple ports.
 */
int
bcm_fe2000_g2p3_field_qualify_OutPort_get(void *unitData,
                                          bcm_field_entry_t entry,
                                          bcm_port_t *data,
                                          bcm_port_t *mask)
{
    /* get the port qualfier */
    return _bcm_fe2000_g2p3_field_qualify_Ports_get((_bcm_fe2k_g2p3_field_glob_t*)unitData,
                                                    entry,
                                                    bcmFieldQualifyOutPort,
                                                    data,
                                                    mask,
                                                    NULL,
                                                    NULL);
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_qualify_InPorts
 *   Purpose
 *      Set allowed ingress ports for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (out) bcm_pbmp_t *data = where to put ingress ports
 *      (out) bcm_pbmp_t *mask = where to put ingress ports mask
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_g2p3_field_qualify_InPorts_get(void *unitData,
                                          bcm_field_entry_t entry,
                                          bcm_pbmp_t *data,
                                          bcm_pbmp_t *mask)
{
    return _bcm_fe2000_g2p3_field_qualify_Ports_get((_bcm_fe2k_g2p3_field_glob_t*)unitData,
                                                    entry,
                                                    bcmFieldQualifyInPorts,
                                                    NULL,
                                                    NULL,
                                                    data,
                                                    mask);
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_qualify_OutPorts
 *   Purpose
 *      Set allowed ingress ports for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (out) bcm_pbmp_t *data = where to put ingress ports
 *      (out) bcm_pbmp_t *mask = where to put ingress ports mask
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_g2p3_field_qualify_OutPorts_get(void *unitData,
                                           bcm_field_entry_t entry,
                                           bcm_pbmp_t *data,
                                           bcm_pbmp_t *mask)
{
    return _bcm_fe2000_g2p3_field_qualify_Ports_get((_bcm_fe2k_g2p3_field_glob_t*)unitData,
                                                    entry,
                                                    bcmFieldQualifyOutPorts,
                                                    NULL,
                                                    NULL,
                                                    data,
                                                    mask);
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_qualify_OuterVlan
 *   Purpose
 *      Set expected outer VLAN tag for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_vlan_t data = VLAN tag data (16 bits)
 *      (in) bcm_vlan_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be all zeroes or ones for supported subfields.  While we
 *      accept a nonzero mask for CFI, it is ignored with a warning.
 *
 *      BCM API docs neglect to metion that this function should apply to the
 *      entire tag, rather than just the VID.
 */
int
bcm_fe2000_g2p3_field_qualify_OuterVlan(void *unitData,
                                        bcm_field_entry_t entry,
                                        bcm_vlan_t data,
                                        bcm_vlan_t mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyOuterVlan);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if ((0xFFFF != mask) &&
        (0xEFFF != mask) &&
        (0xE000 != mask) &&
        (0x0FFF != mask) &&
        (0x0000 != mask)) {
        /*
         *  We accept FFFF as mask even though we ignore CFI in classifier:
         *  this is legacy behaviour that our customers probably expect.
         *
         *  We do check PRI and VID, so we also accept masks appropriately.
         */
        FIELD_ERR((FIELD_MSG1("unit %d can only mask %s based upon"
                              " entire PRI or entire VID or both.\n"),
                   thisUnit->unit,
                   _sbx_fe2000_field_qual_name[bcmFieldQualifyOuterVlan]));
        return BCM_E_PARAM;
    }
    if (0x8000 & mask) {
        FIELD_WARN((FIELD_MSG1("unit %d does not support CFI matching\n"),
                    thisUnit->unit));
    }
    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (thisUnit->entry[entry].hwRule) {
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            rule.ingress.vid = (mask & 0x0FFF)?data:0x000;
            rule.ingress.usevid = (mask & 0x0FFF)?TRUE:FALSE;
            rule.ingress.pri = (mask & 0x7000)?(data >> 12) & 0x7:0;
            rule.ingress.usepri = (mask & 0x7000)?TRUE:FALSE;
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }
    FIELD_EVERB((FIELD_MSG1("write OuterVlan(%03X,%03X) qualifier to"
                            " unit %d entry %08X\n"),
                 data,
                 mask,
                 thisUnit->unit,
                 entry));
    /* update the entry with the new rules */
    return _bcm_fe2k_g2p3_field_rules_write(thisUnit, entry, &rule);
}
int
bcm_fe2000_g2p3_field_qualify_OuterVlan_get(void *unitData,
                                        bcm_field_entry_t entry,
                                        bcm_vlan_t *data,
                                        bcm_vlan_t *mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result = BCM_E_NONE;                    /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyOuterVlan);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (thisUnit->entry[entry].hwRule) {
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            *mask = rule.ingress.usevid ? 0xFFF : 0;
            *data = rule.ingress.usevid ? rule.ingress.vid : 0;
            *mask |= rule.ingress.usepri ? (0x7000) : 0;
            *data |= rule.ingress.usepri ? ((rule.ingress.pri & 0x7) << 12) : 0;
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }
    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_qualify_OuterVlanId
 *   Purpose
 *      Set expected outer VID for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_vlan_t data = VLAN tag data (12 bits)
 *      (in) bcm_vlan_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the outer VLAN ID to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_fe2000_g2p3_field_qualify_OuterVlanId(void *unitData,
                                          bcm_field_entry_t entry,
                                          bcm_vlan_t data,
                                          bcm_vlan_t mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyOuterVlanId);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if ((0xFFFF != mask) &&
        (0x0FFF != mask) &&
        (0x0000 != mask)) {
        FIELD_ERR((FIELD_MSG1("unit %d only supports all-or-nothing mask"
                              "for %s\n"),
                   thisUnit->unit,
                   _sbx_fe2000_field_qual_name[bcmFieldQualifyOuterVlanId]));
        return BCM_E_PARAM;
    }
    if (0xFFF < data) {
        FIELD_ERR((FIELD_MSG1("VID %04X is not valid\n"),
                   data));
        return BCM_E_PARAM;
    }
    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (thisUnit->entry[entry].hwRule) {
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            rule.ingress.vid = (mask & 0xFFF)?data:0x000;
            rule.ingress.usevid = (mask & 0xFFF)?TRUE:FALSE;
            break;
        case 2:
            rule.ingress_v6.vid = (mask & 0xFFF)?data:0x000;
            rule.ingress_v6.usevid = (mask & 0xFFF)?TRUE:FALSE;
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }
    FIELD_EVERB((FIELD_MSG1("write OuterVlan(%03X,%03X) qualifier to"
                            " unit %d entry %08X\n"),
                 data,
                 mask,
                 thisUnit->unit,
                 entry));
    /* update the entry with the new rules */
    return _bcm_fe2k_g2p3_field_rules_write(thisUnit, entry, &rule);
}
int
bcm_fe2000_g2p3_field_qualify_OuterVlanId_get(void *unitData,
                                          bcm_field_entry_t entry,
                                          bcm_vlan_t *data,
                                          bcm_vlan_t *mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyOuterVlanId);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (thisUnit->entry[entry].hwRule) {
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            *mask = rule.ingress.usevid ? 0xFFF : 0;
            *data = rule.ingress.usevid ? rule.ingress.vid : 0;
            break;
        case 2:
            *mask = rule.ingress_v6.usevid ? 0xFFF : 0;
            *data = rule.ingress_v6.usevid ? rule.ingress.vid : 0;
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }
    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_qualify_OuterVlanPri
 *   Purpose
 *      Set expected outer pri for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint8 data = VLAN pri data (3 bits)
 *      (in) uint8 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the outer VLAN Pri to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_fe2000_g2p3_field_qualify_OuterVlanPri(void *unitData,
                                           bcm_field_entry_t entry,
                                           uint8 data,
                                           uint8 mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyOuterVlanPri);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if ((0xFF != mask) && (0x07 != mask) && (0x00 != mask)) {
        FIELD_ERR((FIELD_MSG1("unit %d only supports all-or-nothing mask"
                              "for %s\n"),
                   thisUnit->unit,
                   _sbx_fe2000_field_qual_name[bcmFieldQualifyOuterVlanPri]));
        return BCM_E_PARAM;
    }
    if (0x07 < data) {
        FIELD_ERR((FIELD_MSG1("Pri %02X is not valid\n"),
                   data));
        return BCM_E_PARAM;
    }
    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (thisUnit->entry[entry].hwRule) {
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            rule.ingress.pri = (mask & 0x07)?data:0;
            rule.ingress.usepri = (mask & 0x07)?TRUE:FALSE;
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }
    FIELD_EVERB((FIELD_MSG1("write OuterVlan(%03X,%03X) qualifier to"
                            " unit %d entry %08X\n"),
                 data,
                 mask,
                 thisUnit->unit,
                 entry));
    /* update the entry with the new rules */
    return _bcm_fe2k_g2p3_field_rules_write(thisUnit, entry, &rule);
}
int
bcm_fe2000_g2p3_field_qualify_OuterVlanPri_get(void *unitData,
                                           bcm_field_entry_t entry,
                                           uint8 *data,
                                           uint8 *mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyOuterVlanPri);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (thisUnit->entry[entry].hwRule) {
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            *mask = rule.ingress.usepri ? 0x7 : 0;
            *data = rule.ingress.usepri ? rule.ingress.pri : 0;
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }

    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_qualify_EtherType
 *   Purpose
 *      Set expected outer VLAN for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_vlan_t data = which VLAN
 *      (in) bcm_vlan_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the EtherType to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_fe2000_g2p3_field_qualify_EtherType(void *unitData,
                                        bcm_field_entry_t entry,
                                        uint16 data,
                                        uint16 mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyEtherType);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if ((0xFFFF != mask) && (0x0000 != mask)) {
        FIELD_ERR((FIELD_MSG1("unit %d only supports all-or-nothing %s mask\n"),
                   thisUnit->unit,
                   _sbx_fe2000_field_qual_name[bcmFieldQualifyEtherType]));
        return BCM_E_PARAM;
    }
    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (thisUnit->entry[entry].hwRule) {
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            rule.ingress.etype = mask?data:0x0000;
            rule.ingress.useetype = mask?TRUE:FALSE;
            break;
        case 1:
            rule.egress.etype = mask?data:0x0000;
            rule.egress.useetype = mask?TRUE:FALSE;
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }

    FIELD_EVERB((FIELD_MSG1("write EtherType(%04X,%04X) qualifier to"
                            " unit %d entry %08X\n"),
                 data,
                 mask,
                 thisUnit->unit,
                 entry));
    /* update the entry with the new rules */
    return _bcm_fe2k_g2p3_field_rules_write(thisUnit, entry, &rule);
}
int
bcm_fe2000_g2p3_field_qualify_EtherType_get(void *unitData,
                                        bcm_field_entry_t entry,
                                        uint16 *data,
                                        uint16 *mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyEtherType);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (thisUnit->entry[entry].hwRule) {
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            *mask = rule.ingress.useetype ? 0xFFFF : 0;
            *data = rule.ingress.useetype ? rule.ingress.etype : 0;
            break;
        case 1:
            *mask = rule.egress.useetype ? 0xFFFF : 0;
            *data = rule.egress.useetype ? rule.egress.etype : 0;
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }

    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_qualify_IpProtocol
 *   Purpose
 *      Set expected IPv4 protocol type type for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint16 data = which ethertype
 *      (in) uint16 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the IP protocol to be masked (all are always significant
 *      if any are significant).
 */
int
bcm_fe2000_g2p3_field_qualify_IpProtocol(void *unitData,
                                         bcm_field_entry_t entry,
                                         uint8 data,
                                         uint8 mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyIpProtocol);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if ((0xFF != mask) && (0x00 != mask)) {
        FIELD_ERR((FIELD_MSG1("unit %d only supports all-or-nothing %s mask\n"),
                   thisUnit->unit,
                   _sbx_fe2000_field_qual_name[bcmFieldQualifyIpProtocol]));
        return BCM_E_PARAM;
    }
    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (thisUnit->entry[entry].hwRule) {
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            rule.ingress.proto = mask?data:0x00;
            rule.ingress.useproto = mask?TRUE:FALSE;
            rule.ingress.etype = 0x0800; 
            rule.ingress.useetype = TRUE;
            break;
        case 1:
            rule.egress.proto = mask?data:0x00;
            rule.egress.useproto = mask?TRUE:FALSE;
            rule.egress.etype = 0x0800; 
            rule.egress.useetype = TRUE;
            break;
        case 2:
            if (data != 0x11 && data != 0x6) {
                rule.ingress_v6.nextheaderhi = mask?data:0x00;
                rule.ingress_v6.nextheaderlo = mask?data:0x00;
                rule.ingress_v6.sporthi = 0;
                rule.ingress_v6.sportlo = 0;
            }
            break;
        case 3:
            if (data != 0x11 && data != 0x6) {
                rule.egress_v6.nextheaderhi = mask?data:0x00;
                rule.egress_v6.nextheaderlo = mask?data:0x00;
                rule.egress_v6.sporthi = 0;
                rule.egress_v6.sportlo = 0;
            }
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }
    FIELD_EVERB((FIELD_MSG1("write IpProtocol(%02X,%02X) qualifier to"
                            " unit %d entry %08X\n"),
                 data,
                 mask,
                 thisUnit->unit,
                 entry));
    /* update the entry with the new rules */
    return _bcm_fe2k_g2p3_field_rules_write(thisUnit, entry, &rule);
}
int
bcm_fe2000_g2p3_field_qualify_IpProtocol_get(void *unitData,
                                         bcm_field_entry_t entry,
                                         uint8 *data,
                                         uint8 *mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyIpProtocol);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (thisUnit->entry[entry].hwRule) {
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            if ( (!rule.ingress.useproto) ||
                 (!rule.ingress.useetype) ||
                 (rule.ingress.etype != 0x800) ) {
                *mask = 0;
                *data = 0;
            }else if ((rule.ingress.usefin) ||
                        (rule.ingress.usesyn) ||
                        (rule.ingress.userst) ||
                        (rule.ingress.usepsh) ||
                        (rule.ingress.useack) ||
                        (rule.ingress.useurg) ) {
                /* TcpControl qualifier */
                *mask = 0;
                *data = 0;
            }else{
                *mask = 0xFF;
                *data = rule.ingress.proto;
            }
            break;
        case 1:
            if ( (!rule.egress.useproto) ||
                 (!rule.egress.useetype) ||
                 (rule.egress.etype != 0x800) ) {
                *mask = 0;
                *data = 0;
            }else if ((rule.egress.usefin) ||
                        (rule.egress.usesyn) ||
                        (rule.egress.userst) ||
                        (rule.egress.usepsh) ||
                        (rule.egress.useack) ||
                        (rule.egress.useurg) ) {
                /* TcpControl qualifier */
                *mask = 0;
                *data = 0;
            }else{
                *mask = 0xFF;
                *data = rule.egress.proto;
            }
            break;
        case 2:
            if ( (rule.ingress_v6.nextheaderhi = rule.ingress_v6.nextheaderlo) &&
                 (rule.ingress_v6.sporthi == 0) &&
                 (rule.ingress_v6.sportlo == 0) ) {
                *mask = rule.ingress_v6.nextheaderhi ? 0xFF : 0;
                *data = (*mask) ? rule.ingress_v6.nextheaderhi : 0;
            }else{
                *mask = 0;
                *data = 0;
            }
            break;
        case 3:
            if ( (rule.egress_v6.nextheaderhi = rule.egress_v6.nextheaderlo) &&
                 (rule.egress_v6.sporthi == 0) &&
                 (rule.egress_v6.sportlo == 0) ) {
                *mask = rule.egress_v6.nextheaderhi ? 0xFF : 0;
                *data = (*mask) ? rule.egress_v6.nextheaderhi : 0;
            }else{
                *mask = 0;
                *data = 0;
            }
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }
    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_qualify_SrcIp
 *   Purpose
 *      Set expected source IPv4 address for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip_t data = which source IPv4 address
 *      (in) bcm_ip_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_fe2000_g2p3_field_qualify_SrcIp(void *unitData,
                                    bcm_field_entry_t entry,
                                    bcm_ip_t data,
                                    bcm_ip_t mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */
    unsigned int prefLen;                       /* working prefix length */
    bcm_ip_t prefBuff;                          /* working prefix buffer */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifySrcIp);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (((~mask) + 1) & (~mask)) {
        /* mask is not prefix form with significant bits set */
        FIELD_ERR((FIELD_MSG1("unit %d only supports prefix %s mask\n"),
                   thisUnit->unit,
                   _sbx_fe2000_field_qual_name[bcmFieldQualifySrcIp]));
        return BCM_E_PARAM;
    }
    /* figure out the mask length by shifting left until it's zero */
    for (prefBuff = mask, prefLen = 0;
         prefBuff && (prefLen < 32);
         prefBuff <<= 1, prefLen++) {
        /* just iterate */
    }
    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (thisUnit->entry[entry].hwRule) {
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            rule.ingress.sa = data;
            rule.ingress.sawidth = prefLen;
            rule.ingress.etype = 0x0800; 
            rule.ingress.useetype = TRUE;
            break;
        case 1:
            rule.egress.sa = data;
            rule.egress.sawidth = prefLen;
            rule.egress.etype = 0x0800; 
            rule.egress.useetype = TRUE;
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }
    FIELD_EVERB((FIELD_MSG1("write SrcIp(%08X,%08X) qualifier to"
                            " unit %d entry %08X\n"),
                 data,
                 mask,
                 thisUnit->unit,
                 entry));
    /* update the entry with the new rules */
    return _bcm_fe2k_g2p3_field_rules_write(thisUnit, entry, &rule);
}
int
bcm_fe2000_g2p3_field_qualify_SrcIp_get(void *unitData,
                                    bcm_field_entry_t entry,
                                    bcm_ip_t *data,
                                    bcm_ip_t *mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */
    unsigned int prefLen;                       /* working prefix length */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifySrcIp);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (thisUnit->entry[entry].hwRule) {
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            prefLen = rule.ingress.sawidth;
            if ( (!prefLen) ||
                 (!rule.ingress.useetype) ||
                 (rule.ingress.etype != 0x0800) ){
                prefLen = 0;
            }
            *data = prefLen ? rule.ingress.sa : 0;
            prefLen = 32 - prefLen;
            *mask = (prefLen==32) ? 0x0: ~(0xFFFFFFFF & ((1 << prefLen) - 1) );
            break;
        case 1:
            prefLen = rule.egress.sawidth;
            if ( (!prefLen) ||
                 (!rule.egress.useetype) ||
                 (rule.egress.etype != 0x0800) ){
                prefLen = 0;
            }
            *data = prefLen ? rule.egress.sa : 0;
            prefLen = 32 - prefLen;
            *mask = (prefLen==32) ? 0x0: ~(0xFFFFFFFF & ((1 << prefLen) - 1) );
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }
    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_qualify_DstIp
 *   Purpose
 *      Set expected source IPv4 address for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip_t data = which source IPv4 address
 *      (in) bcm_ip_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_fe2000_g2p3_field_qualify_DstIp(void *unitData,
                                    bcm_field_entry_t entry,
                                    bcm_ip_t data,
                                    bcm_ip_t mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */
    unsigned int prefLen;                       /* working prefix length */
    bcm_ip_t prefBuff;                          /* working prefix buffer */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyDstIp);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (((~mask) + 1) & (~mask)) {
        /* mask is not prefix form with significant bits set */
        FIELD_ERR((FIELD_MSG1("unit %d only supports prefix %s mask\n"),
                   thisUnit->unit,
                   _sbx_fe2000_field_qual_name[bcmFieldQualifyDstIp]));
        return BCM_E_PARAM;
    }
    /* figure out the mask length by shifting left until it's zero */
    for (prefBuff = mask, prefLen = 0;
         prefBuff && (prefLen < 32);
         prefBuff <<= 1, prefLen++) {
        /* just iterate */
    }
    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (thisUnit->entry[entry].hwRule) {
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            rule.ingress.da = data;
            rule.ingress.dawidth = prefLen;
            rule.ingress.etype = 0x0800; 
            rule.ingress.useetype = TRUE;
            break;
        case 1:
            rule.egress.da = data;
            rule.egress.dawidth = prefLen;
            rule.egress.etype = 0x0800; 
            rule.egress.useetype = TRUE;
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }
    FIELD_EVERB((FIELD_MSG1("write DstIp(%08X,%08X) qualifier to"
                            " unit %d entry %08X\n"),
                 data,
                 mask,
                 thisUnit->unit,
                 entry));
    /* update the entry with the new rules */
    return _bcm_fe2k_g2p3_field_rules_write(thisUnit, entry, &rule);
}
int
bcm_fe2000_g2p3_field_qualify_DstIp_get(void *unitData,
                                    bcm_field_entry_t entry,
                                    bcm_ip_t *data,
                                    bcm_ip_t *mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */
    unsigned int prefLen;                       /* working prefix length */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyDstIp);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (thisUnit->entry[entry].hwRule) {
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            prefLen = rule.ingress.dawidth;
            if ( (!prefLen) ||
                 (!rule.ingress.useetype) ||
                 (rule.ingress.etype != 0x0800) ){
                prefLen = 0;
            }
            *data = prefLen ? rule.ingress.da : 0;
            prefLen = 32 - prefLen;
            *mask = (prefLen==32) ? 0x0: ~(0xFFFFFFFF & ((1 << prefLen) - 1) );
            break;
        case 1:
            prefLen = rule.egress.dawidth;
            if ( (!prefLen) ||
                 (!rule.egress.useetype) ||
                 (rule.egress.etype != 0x0800) ){
                prefLen = 0;
            }
            *data = prefLen ? rule.egress.da : 0;
            prefLen = 32 - prefLen;
            *mask = (prefLen==32) ? 0x0: ~(0xFFFFFFFF & ((1 << prefLen) - 1) );
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }
    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_qualify_DSCP
 *   Purpose
 *      Set expected IPv4 DSCP for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint8 data = which DSCP
 *      (in) uint8 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the IP protocol to be masked (all are always significant
 *      if any are significant).
 */
int
bcm_fe2000_g2p3_field_qualify_DSCP(void *unitData,
                                   bcm_field_entry_t entry,
                                   uint8 data,
                                   uint8 mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyDSCP);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if ((0x3F != mask) && (0x00 != mask)) {
        FIELD_ERR((FIELD_MSG1("unit %d only supports all-or-nothing %s mask\n"),
                   thisUnit->unit,
                   _sbx_fe2000_field_qual_name[bcmFieldQualifyDSCP]));
        return BCM_E_PARAM;
    }
    if (0xC0 & data) {
        FIELD_ERR((FIELD_MSG1("invalid value %02X for unit %d %s\n"),
                   data,
                   thisUnit->unit,
                   _sbx_fe2000_field_qual_name[bcmFieldQualifyDSCP]));
        return BCM_E_PARAM;
    }
    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (thisUnit->entry[entry].hwRule) {
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            rule.ingress.dscp = mask?data:0x00;
            rule.ingress.usedscp = mask?TRUE:FALSE;
            rule.ingress.etype = 0x0800; 
            rule.ingress.useetype = TRUE;
            break;
        case 1:
            rule.egress.dscp = mask?data:0x00;
            rule.egress.usedscp = mask?TRUE:FALSE;
            rule.egress.etype = 0x0800; 
            rule.egress.useetype = TRUE;
            break;
        case 2:
            rule.ingress_v6.TC = mask?data:0x00;
            rule.ingress_v6.useTC = mask?TRUE:FALSE;
            break;
        case 3:
            rule.egress_v6.TC = mask?data:0x00;
            rule.egress_v6.useTC = mask?TRUE:FALSE;
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }
    FIELD_EVERB((FIELD_MSG1("write DSCP(%02X,%02X) qualifier to"
                            " unit %d entry %08X\n"),
                 data,
                 mask,
                 thisUnit->unit,
                 entry));
    /* update the entry with the new rules */
    return _bcm_fe2k_g2p3_field_rules_write(thisUnit, entry, &rule);
}
int
bcm_fe2000_g2p3_field_qualify_DSCP_get(void *unitData,
                                   bcm_field_entry_t entry,
                                   uint8 *data,
                                   uint8 *mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result = BCM_E_NONE;                    /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyDSCP);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (thisUnit->entry[entry].hwRule) {
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            (*mask) = rule.ingress.usedscp ? 0x3F : 0;
            (*data) = rule.ingress.usedscp ? rule.ingress.dscp : 0;
            break;
        case 1:
            (*mask) = rule.egress.usedscp ? 0x3F : 0;
            (*data) = rule.egress.usedscp ? rule.egress.dscp : 0;
            break;
        case 2:
            (*mask) = rule.ingress_v6.useTC ? 0x3F : 0;
            (*data) = rule.ingress_v6.useTC ? rule.ingress_v6.TC : 0;
            break;
        case 3:
            (*mask) = rule.egress_v6.useTC ? 0x3F : 0;
            (*data) = rule.egress_v6.useTC ? rule.ingress_v6.TC : 0;
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }

    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_qualify_TcpControl
 *   Purpose
 *      Set expected TCP control flags for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint8 data = which TCP control bits
 *      (in) uint8 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Also implies TCP as protocol.
 */
int
bcm_fe2000_g2p3_field_qualify_TcpControl(void *unitData,
                                         bcm_field_entry_t entry,
                                         uint8 data,
                                         uint8 mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyTcpControl);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (((~(BCM_FIELD_TCPCONTROL_FIN |
            BCM_FIELD_TCPCONTROL_SYN |
            BCM_FIELD_TCPCONTROL_RST |
            BCM_FIELD_TCPCONTROL_PSH |
            BCM_FIELD_TCPCONTROL_ACK |
            BCM_FIELD_TCPCONTROL_URG)) & mask) ||
        ((~(BCM_FIELD_TCPCONTROL_FIN |
            BCM_FIELD_TCPCONTROL_SYN |
            BCM_FIELD_TCPCONTROL_RST |
            BCM_FIELD_TCPCONTROL_PSH |
            BCM_FIELD_TCPCONTROL_ACK |
            BCM_FIELD_TCPCONTROL_URG)) & data) /* ||
        ((~mask) & data)*/) {
        FIELD_ERR((FIELD_MSG1("invalid combination of TCP control mask %02X"
                              " and data %02X\n"),
                   mask,
                   data));
        return BCM_E_PARAM;
    }
    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* normalise the data to under the mask */
    data &= mask;
    /* update the qualifiers */
    if (thisUnit->entry[entry].hwRule) {
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            rule.ingress.fin = (0 != (data & BCM_FIELD_TCPCONTROL_FIN));
            rule.ingress.usefin = (0 != (mask & BCM_FIELD_TCPCONTROL_FIN));
            rule.ingress.syn = (0 != (data & BCM_FIELD_TCPCONTROL_SYN));
            rule.ingress.usesyn = (0 != (mask & BCM_FIELD_TCPCONTROL_SYN));
            rule.ingress.rst = (0 != (data & BCM_FIELD_TCPCONTROL_RST));
            rule.ingress.userst = (0 != (mask & BCM_FIELD_TCPCONTROL_RST));
            rule.ingress.psh = (0 != (data & BCM_FIELD_TCPCONTROL_PSH));
            rule.ingress.usepsh = (0 != (mask & BCM_FIELD_TCPCONTROL_PSH));
            rule.ingress.ack = (0 != (data & BCM_FIELD_TCPCONTROL_ACK));
            rule.ingress.useack = (0 != (mask & BCM_FIELD_TCPCONTROL_ACK));
            rule.ingress.urg = (0 != (data & BCM_FIELD_TCPCONTROL_URG));
            rule.ingress.useurg = (0 != (mask & BCM_FIELD_TCPCONTROL_URG));
            rule.ingress.proto = 0x06;   
            rule.ingress.useproto = TRUE;
            rule.ingress.etype = 0x0800; 
            rule.ingress.useetype = TRUE;
            break;
        case 1:
            rule.egress.fin = (0 != (data & BCM_FIELD_TCPCONTROL_FIN));
            rule.egress.usefin = (0 != (mask & BCM_FIELD_TCPCONTROL_FIN));
            rule.egress.syn = (0 != (data & BCM_FIELD_TCPCONTROL_SYN));
            rule.egress.usesyn = (0 != (mask & BCM_FIELD_TCPCONTROL_SYN));
            rule.egress.rst = (0 != (data & BCM_FIELD_TCPCONTROL_RST));
            rule.egress.userst = (0 != (mask & BCM_FIELD_TCPCONTROL_RST));
            rule.egress.psh = (0 != (data & BCM_FIELD_TCPCONTROL_PSH));
            rule.egress.usepsh = (0 != (mask & BCM_FIELD_TCPCONTROL_PSH));
            rule.egress.ack = (0 != (data & BCM_FIELD_TCPCONTROL_ACK));
            rule.egress.useack = (0 != (mask & BCM_FIELD_TCPCONTROL_ACK));
            rule.egress.urg = (0 != (data & BCM_FIELD_TCPCONTROL_URG));
            rule.egress.useurg = (0 != (mask & BCM_FIELD_TCPCONTROL_URG));
            rule.egress.proto = 0x06;   
            rule.egress.useproto = TRUE;
            rule.egress.etype = 0x0800; 
            rule.egress.useetype = TRUE;
            break;
        case 2:
            rule.ingress_v6.fin = (0 != (data & BCM_FIELD_TCPCONTROL_FIN));
            rule.ingress_v6.usefin = (0 != (mask & BCM_FIELD_TCPCONTROL_FIN));
            rule.ingress_v6.syn = (0 != (data & BCM_FIELD_TCPCONTROL_SYN));
            rule.ingress_v6.usesyn = (0 != (mask & BCM_FIELD_TCPCONTROL_SYN));
            rule.ingress_v6.rst = (0 != (data & BCM_FIELD_TCPCONTROL_RST));
            rule.ingress_v6.userst = (0 != (mask & BCM_FIELD_TCPCONTROL_RST));
            rule.ingress_v6.psh = (0 != (data & BCM_FIELD_TCPCONTROL_PSH));
            rule.ingress_v6.usepsh = (0 != (mask & BCM_FIELD_TCPCONTROL_PSH));
            rule.ingress_v6.ack = (0 != (data & BCM_FIELD_TCPCONTROL_ACK));
            rule.ingress_v6.useack = (0 != (mask & BCM_FIELD_TCPCONTROL_ACK));
            rule.ingress_v6.urg = (0 != (data & BCM_FIELD_TCPCONTROL_URG));
            rule.ingress_v6.useurg = (0 != (mask & BCM_FIELD_TCPCONTROL_URG));
            break;
        case 3:
            rule.egress_v6.fin = (0 != (data & BCM_FIELD_TCPCONTROL_FIN));
            rule.egress_v6.usefin = (0 != (mask & BCM_FIELD_TCPCONTROL_FIN));
            rule.egress_v6.syn = (0 != (data & BCM_FIELD_TCPCONTROL_SYN));
            rule.egress_v6.usesyn = (0 != (mask & BCM_FIELD_TCPCONTROL_SYN));
            rule.egress_v6.rst = (0 != (data & BCM_FIELD_TCPCONTROL_RST));
            rule.egress_v6.userst = (0 != (mask & BCM_FIELD_TCPCONTROL_RST));
            rule.egress_v6.psh = (0 != (data & BCM_FIELD_TCPCONTROL_PSH));
            rule.egress_v6.usepsh = (0 != (mask & BCM_FIELD_TCPCONTROL_PSH));
            rule.egress_v6.ack = (0 != (data & BCM_FIELD_TCPCONTROL_ACK));
            rule.egress_v6.useack = (0 != (mask & BCM_FIELD_TCPCONTROL_ACK));
            rule.egress_v6.urg = (0 != (data & BCM_FIELD_TCPCONTROL_URG));
            rule.egress_v6.useurg = (0 != (mask & BCM_FIELD_TCPCONTROL_URG));
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }
    FIELD_EVERB((FIELD_MSG1("write TcpControl(%02X,%02X) qualifier to"
                            " unit %d entry %08X\n"),
                 data,
                 mask,
                 thisUnit->unit,
                 entry));
    /* update the entry with the new rules */
    return _bcm_fe2k_g2p3_field_rules_write(thisUnit, entry, &rule);
}
int
bcm_fe2000_g2p3_field_qualify_TcpControl_get(void *unitData,
                                         bcm_field_entry_t entry,
                                         uint8 *data,
                                         uint8 *mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyTcpControl);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* update the qualifiers */
    if (thisUnit->entry[entry].hwRule) {
        *data = 0;
        *mask = 0;
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            *mask |= rule.ingress.usefin ? BCM_FIELD_TCPCONTROL_FIN : 0;
            *data |= rule.ingress.fin ? BCM_FIELD_TCPCONTROL_FIN : 0;
            *mask |= rule.ingress.usesyn ? BCM_FIELD_TCPCONTROL_SYN : 0;
            *data |= rule.ingress.syn ? BCM_FIELD_TCPCONTROL_SYN : 0;
            *mask |= rule.ingress.userst ? BCM_FIELD_TCPCONTROL_RST : 0;
            *data |= rule.ingress.rst ? BCM_FIELD_TCPCONTROL_RST : 0;
            *mask |= rule.ingress.usepsh ? BCM_FIELD_TCPCONTROL_PSH : 0;
            *data |= rule.ingress.psh ? BCM_FIELD_TCPCONTROL_PSH : 0;
            *mask |= rule.ingress.useack ? BCM_FIELD_TCPCONTROL_ACK : 0;
            *data |= rule.ingress.ack ? BCM_FIELD_TCPCONTROL_ACK : 0;
            *mask |= rule.ingress.useurg ? BCM_FIELD_TCPCONTROL_URG : 0;
            *data |= rule.ingress.urg ? BCM_FIELD_TCPCONTROL_URG : 0;
            break;
        case 1:
            *mask |= rule.egress.usefin ? BCM_FIELD_TCPCONTROL_FIN : 0;
            *data |= rule.egress.fin ? BCM_FIELD_TCPCONTROL_FIN : 0;
            *mask |= rule.egress.usesyn ? BCM_FIELD_TCPCONTROL_SYN : 0;
            *data |= rule.egress.syn ? BCM_FIELD_TCPCONTROL_SYN : 0;
            *mask |= rule.egress.userst ? BCM_FIELD_TCPCONTROL_RST : 0;
            *data |= rule.egress.rst ? BCM_FIELD_TCPCONTROL_RST : 0;
            *mask |= rule.egress.usepsh ? BCM_FIELD_TCPCONTROL_PSH : 0;
            *data |= rule.egress.psh ? BCM_FIELD_TCPCONTROL_PSH : 0;
            *mask |= rule.egress.useack ? BCM_FIELD_TCPCONTROL_ACK : 0;
            *data |= rule.egress.ack ? BCM_FIELD_TCPCONTROL_ACK : 0;
            *mask |= rule.egress.useurg ? BCM_FIELD_TCPCONTROL_URG : 0;
            *data |= rule.egress.urg ? BCM_FIELD_TCPCONTROL_URG : 0;
            break;
        case 2:
            *mask |= rule.ingress_v6.usefin ? BCM_FIELD_TCPCONTROL_FIN : 0;
            *data |= rule.ingress_v6.fin ? BCM_FIELD_TCPCONTROL_FIN : 0;
            *mask |= rule.ingress_v6.usesyn ? BCM_FIELD_TCPCONTROL_SYN : 0;
            *data |= rule.ingress_v6.syn ? BCM_FIELD_TCPCONTROL_SYN : 0;
            *mask |= rule.ingress_v6.userst ? BCM_FIELD_TCPCONTROL_RST : 0;
            *data |= rule.ingress_v6.rst ? BCM_FIELD_TCPCONTROL_RST : 0;
            *mask |= rule.ingress_v6.usepsh ? BCM_FIELD_TCPCONTROL_PSH : 0;
            *data |= rule.ingress_v6.psh ? BCM_FIELD_TCPCONTROL_PSH : 0;
            *mask |= rule.ingress_v6.useack ? BCM_FIELD_TCPCONTROL_ACK : 0;
            *data |= rule.ingress_v6.ack ? BCM_FIELD_TCPCONTROL_ACK : 0;
            *mask |= rule.ingress_v6.useurg ? BCM_FIELD_TCPCONTROL_URG : 0;
            *data |= rule.ingress_v6.urg ? BCM_FIELD_TCPCONTROL_URG : 0;
            break;
        case 3:
            *mask |= rule.egress_v6.usefin ? BCM_FIELD_TCPCONTROL_FIN : 0;
            *data |= rule.egress_v6.fin ? BCM_FIELD_TCPCONTROL_FIN : 0;
            *mask |= rule.egress_v6.usesyn ? BCM_FIELD_TCPCONTROL_SYN : 0;
            *data |= rule.egress_v6.syn ? BCM_FIELD_TCPCONTROL_SYN : 0;
            *mask |= rule.egress_v6.userst ? BCM_FIELD_TCPCONTROL_RST : 0;
            *data |= rule.egress_v6.rst ? BCM_FIELD_TCPCONTROL_RST : 0;
            *mask |= rule.egress_v6.usepsh ? BCM_FIELD_TCPCONTROL_PSH : 0;
            *data |= rule.egress_v6.psh ? BCM_FIELD_TCPCONTROL_PSH : 0;
            *mask |= rule.egress_v6.useack ? BCM_FIELD_TCPCONTROL_ACK : 0;
            *data |= rule.egress_v6.ack ? BCM_FIELD_TCPCONTROL_ACK : 0;
            *mask |= rule.egress_v6.useurg ? BCM_FIELD_TCPCONTROL_URG : 0;
            *data |= rule.egress_v6.urg ? BCM_FIELD_TCPCONTROL_URG : 0;
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }
    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_qualify_RangeCheck
 *   Purpose
 *      Set expected TCP/UDP port range for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_field_range_t range = which ethertype
 *      (in) int invert = whether the range match is to be inverted
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The range that is specified is read only during this call; it will not
 *      be read later if that range changes; another call to this function will
 *      be required should the range change and the update need to apply.
 *      The invert flag is not supported.
 *      This can't use the helper functions because it is setting a more
 *      complex set of fields under a more complex set of conditions.
 */
int
bcm_fe2000_g2p3_field_qualify_RangeCheck(void *unitData,
                                         bcm_field_entry_t entry,
                                         bcm_field_range_t range,
                                         int invert)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    bcm_field_range_t tempRange;                /* working range ID */
    _bcm_fe2k_field_range_t *thisRange;         /* working range data */
    int result;                                 /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyRangeCheck);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* we don't support invert */
    if (invert) {
        FIELD_ERR((FIELD_MSG1("invert option is not supported on unit %d\n"),
                   thisUnit->unit));
        return BCM_E_PARAM;
    }
    /* make sure the range is valid and exists */
    tempRange = range - 1;
    result = shr_idxres_list_elem_state(thisUnit->rangeFree, tempRange);
    if (BCM_E_EXISTS != result) {
        FIELD_ERR((FIELD_MSG1("unable to access unit %d range %08X: %d (%s)\n"),
                   thisUnit->unit,
                   range,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }
    /* get a pointer to the range; it's faster */
    thisRange = &(thisUnit->range[tempRange]);
    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (thisUnit->entry[entry].hwRule) {
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            if (thisRange->flags & BCM_FIELD_RANGE_SRCPORT) {
                rule.ingress.sportlo = thisRange->min;
                rule.ingress.sporthi = thisRange->max;
            }
            if (thisRange->flags & BCM_FIELD_RANGE_DSTPORT) {
                rule.ingress.dportlo = thisRange->min;
                rule.ingress.dporthi = thisRange->max;
            }
            if (thisRange->flags & BCM_FIELD_RANGE_TCP) {
                rule.ingress.proto = 0x06;
                rule.ingress.useproto = TRUE;
            } else if (thisRange->flags & BCM_FIELD_RANGE_UDP) {
                rule.ingress.proto = 0x11;
                rule.ingress.useproto = TRUE;
            } else {
                rule.ingress.proto = 0x00;
                rule.ingress.useproto = FALSE;
            }
            rule.ingress.etype = 0x0800; 
            rule.ingress.useetype = TRUE;
            break;
        case 1:
            if (thisRange->flags & BCM_FIELD_RANGE_SRCPORT) {
                rule.egress.sportlo = thisRange->min;
                rule.egress.sporthi = thisRange->max;
            }
            if (thisRange->flags & BCM_FIELD_RANGE_DSTPORT) {
                rule.egress.dportlo = thisRange->min;
                rule.egress.dporthi = thisRange->max;
            }
            if (thisRange->flags & BCM_FIELD_RANGE_TCP) {
                rule.egress.proto = 0x06;
                rule.egress.useproto = TRUE;
            } else if (thisRange->flags & BCM_FIELD_RANGE_UDP) {
                rule.egress.proto = 0x11;
                rule.egress.useproto = TRUE;
            } else {
                rule.egress.proto = 0x00;
                rule.egress.useproto = FALSE;
            }
            rule.egress.etype = 0x0800; 
            rule.egress.useetype = TRUE;
            break;
        case 2:
            if (thisRange->flags & BCM_FIELD_RANGE_SRCPORT) {
                rule.ingress_v6.sportlo = thisRange->min;
                rule.ingress_v6.sporthi = thisRange->max;
            }
            if (thisRange->flags & BCM_FIELD_RANGE_DSTPORT) {
                rule.ingress_v6.dportlo = thisRange->min;
                rule.ingress_v6.dporthi = thisRange->max;
            }
            break;
        case 3:
            if (thisRange->flags & BCM_FIELD_RANGE_SRCPORT) {
                rule.egress_v6.sportlo = thisRange->min;
                rule.egress_v6.sporthi = thisRange->max;
            }
            if (thisRange->flags & BCM_FIELD_RANGE_DSTPORT) {
                rule.egress_v6.dportlo = thisRange->min;
                rule.egress_v6.dporthi = thisRange->max;
            }
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }
    if (thisRange->flags & BCM_FIELD_RANGE_SRCPORT) {
        thisUnit->entry[entry].range[0] = range;
    }
    if (thisRange->flags & BCM_FIELD_RANGE_DSTPORT) {
        thisUnit->entry[entry].range[1] = range;
    }
    FIELD_EVERB((FIELD_MSG1("write RangeCheck(%08X) qualifier to"
                            " unit %d entry %08X\n"),
                 range,
                 thisUnit->unit,
                 entry));
    /* update the entry with the new rules */
    return _bcm_fe2k_g2p3_field_rules_write(thisUnit, entry, &rule);
}
int
bcm_fe2000_g2p3_field_qualify_RangeCheck_get(void *unitData,
                                         bcm_field_entry_t entry,
                                         int max_count,
                                         bcm_field_range_t *range,
                                         int *invert,
                                         int *count)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    int result, i;

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyRangeCheck);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* rangeIds are not reflected in hardware */
    *count = 0;
    for (i=0; i<_FIELD_FE2K_G2P3_RANGES_PER_ENTRY; i++) {
        if ( (*count) == max_count ) {
            break;
        }
        range[*count] = thisUnit->entry[entry].range[i];
        invert[*count] = 0;
        if (range[*count]) {
            (*count)++;
        }
    }

    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_qualify_SrcMac
 *   Purpose
 *      Set expected source MAC address for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_mac_t data = which source MAC address
 *      (in) bcm_mac_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the source MAC address to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_fe2000_g2p3_field_qualify_SrcMac(void *unitData,
                                     bcm_field_entry_t entry,
                                     bcm_mac_t data,
                                     bcm_mac_t mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */
    unsigned int maskLen;                       /* mask length */
    uint8 byte;                                 /* mask working byte */
    uint8 bit;                                  /* mask working bit */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifySrcMac);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* ensure MAC address is prefix masked */
    byte = 0;
    maskLen = 0;
    while ((byte < 6) && (0xFF == mask[byte])) {
        byte++;
        maskLen += 8;
    }
    if (byte < 6) {
        if (((~(mask[byte])) & 0xFF) & (((~(mask[byte])) + 1) & 0xFF)) {
            /* this byte is not prefix masked */
            result = BCM_E_PARAM;
        }
        for (bit = 0x80; bit & (mask[byte]); bit = bit >> 1) {
            /* current byte has this bit set */
            maskLen++;
        }
        byte++;
        while (byte < 6) {
            if (mask[byte]) {
                /* not prefix masked if bits are set down here */
                result = BCM_E_PARAM;
            }
            byte++;
        }
    }

    if (BCM_E_NONE != result) {
        FIELD_ERR((FIELD_MSG1("unit %d only supports prefix %s mask\n"),
                   thisUnit->unit,
                   _sbx_fe2000_field_qual_name[bcmFieldQualifySrcMac]));
        return BCM_E_PARAM;
    }
    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (thisUnit->entry[entry].hwRule) {
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            if (maskLen) {
                sal_memcpy(&(rule.ingress.smac[0]),
                           &(data[0]),
                           sizeof(rule.ingress.smac));
            } else {
                sal_memset(&(rule.ingress.smac[0]),
                           0,
                           sizeof(rule.ingress.smac));
            }
            rule.ingress.smacwidth = maskLen;
            break;
        case 1:
            if (maskLen) {
                sal_memcpy(&(rule.egress.smac[0]),
                           &(data[0]),
                           sizeof(rule.egress.smac));
            } else {
                sal_memset(&(rule.egress.smac[0]),
                           0,
                           sizeof(rule.egress.smac));
            }
            rule.egress.smacwidth = maskLen;
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }
    FIELD_EVERB((FIELD_MSG1("write SrcMac(" FIELD_MACA_FORMAT ","
                            FIELD_MACA_FORMAT ") qualifier to"
                            " unit %d entry %08X\n"),
                 FIELD_MACA_SHOW(data),
                 FIELD_MACA_SHOW(mask),
                 thisUnit->unit,
                 entry));
    /* update the entry with the new rules */
    return _bcm_fe2k_g2p3_field_rules_write(thisUnit, entry, &rule);
}

int
bcm_fe2000_g2p3_field_qualify_SrcMac_get(void *unitData,
                                     bcm_field_entry_t entry,
                                     bcm_mac_t *data,
                                     bcm_mac_t *mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */
    unsigned int maskLen;                       /* mask length */
    uint8 byte;                                 /* working byte */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifySrcMac);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (thisUnit->entry[entry].hwRule){
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            maskLen = rule.ingress.smacwidth;
            if (maskLen) {
                sal_memcpy(&(data[0]),
                           &(rule.ingress.smac[0]),
                           sizeof(rule.ingress.smac));
            } else {
                sal_memset(&(data[0]),
                           0,
                           sizeof(rule.ingress.smac));
            }
            break;
        case 1:
            maskLen = rule.egress.smacwidth;
            if (maskLen) {
                sal_memcpy(&(data[0]),
                           &(rule.egress.smac[0]),
                           sizeof(rule.egress.smac));
            } else {
                sal_memset(&(data[0]),
                           0,
                           sizeof(rule.egress.smac));
            }
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }else{
        return BCM_E_NOT_FOUND;
    }

    for (byte=0; byte<6; byte++){
        (*mask)[byte] = 0;
    }

    /* build mask */
    byte = 0;
    while (maskLen >= 8) {
        (*mask)[byte] = 0xFF;
        byte++;
        maskLen -= 8;
    }
    if (maskLen) {
        (*mask)[byte] = ((1 << maskLen) - 1) << (8 - maskLen);
    }

    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_qualify_DstMac
 *   Purpose
 *      Set expected source MAC address for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_mac_t data = which source MAC address
 *      (in) bcm_mac_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the source MAC address to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_fe2000_g2p3_field_qualify_DstMac(void *unitData,
                                     bcm_field_entry_t entry,
                                     bcm_mac_t data,
                                     bcm_mac_t mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */
    unsigned int maskLen;                       /* mask length */
    uint8 byte;                                /* working byte */
    uint8 bit;                                  /* mask working bit */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyDstMac);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* ensure MAC address is prefix masked */
    byte = 0;
    maskLen = 0;
    while ((byte < 6) && (0xFF == mask[byte])) {
        byte++;
        maskLen += 8;
    }
    if (byte < 6) {
        if (((~(mask[byte])) & 0xFF) & (((~(mask[byte])) + 1) & 0xFF)) {
            /* this byte is not prefix masked */
            result = BCM_E_PARAM;
        }
        for (bit = 0x80; bit & (mask[byte]); bit = bit >> 1) {
            /* current byte has this bit set */
            maskLen++;
        }
        byte++;
        while (byte < 6) {
            if (mask[byte]) {
                /* not prefix masked if bits are set down here */
                result = BCM_E_PARAM;
            }
            byte++;
        }
    }
    if (BCM_E_NONE != result) {
        FIELD_ERR((FIELD_MSG1("unit %d only supports prefix %s mask\n"),
                   thisUnit->unit,
                   _sbx_fe2000_field_qual_name[bcmFieldQualifyDstMac]));
        return BCM_E_PARAM;
    }

    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (thisUnit->entry[entry].hwRule){
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            if (maskLen) {
                sal_memcpy(&(rule.ingress.dmac[0]),
                           &(data[0]),
                           sizeof(rule.ingress.dmac));
            } else {
                sal_memset(&(rule.ingress.dmac[0]),
                           0,
                           sizeof(rule.ingress.dmac));
            }
            rule.ingress.dmacwidth = maskLen;
            break;
        case 1:
            if (maskLen) {
                sal_memcpy(&(rule.egress.dmac[0]),
                           &(data[0]),
                           sizeof(rule.egress.dmac));
            } else {
                sal_memset(&(rule.egress.dmac[0]),
                           0,
                           sizeof(rule.egress.dmac));
            }
            rule.egress.dmacwidth = maskLen;
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }
    FIELD_EVERB((FIELD_MSG1("write DstMac(" FIELD_MACA_FORMAT ","
                            FIELD_MACA_FORMAT ") qualifier to"
                            " unit %d entry %08X\n"),
                 FIELD_MACA_SHOW(data),
                 FIELD_MACA_SHOW(mask),
                 thisUnit->unit,
                 entry));
    /* update the entry with the new rules */
    return _bcm_fe2k_g2p3_field_rules_write(thisUnit, entry, &rule);
}

int
bcm_fe2000_g2p3_field_qualify_DstMac_get(void *unitData,
                                     bcm_field_entry_t entry,
                                     bcm_mac_t *data,
                                     bcm_mac_t *mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */
    unsigned int maskLen;                       /* mask length */
    uint8 byte;                                 /* working byte */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyDstMac);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (thisUnit->entry[entry].hwRule){
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            maskLen = rule.ingress.dmacwidth;
            if (maskLen) {
                sal_memcpy(&(data[0]),
                           &(rule.ingress.dmac[0]),
                           sizeof(rule.ingress.dmac));
            } else {
                sal_memset(&(data[0]),
                           0,
                           sizeof(rule.ingress.dmac));
            }
            break;
        case 1:
            maskLen = rule.egress.dmacwidth;
            if (maskLen) {
                sal_memcpy(&(data[0]),
                           &(rule.egress.dmac[0]),
                           sizeof(rule.egress.dmac));
            } else {
                sal_memset(&(data[0]),
                           0,
                           sizeof(rule.egress.dmac));
            }
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }else{
        return BCM_E_NOT_FOUND;
    }

    for (byte=0; byte<6; byte++){
        (*mask)[byte] = 0;
    }

    /* build mask */
    byte = 0;
    while (maskLen >= 8) {
        (*mask)[byte] = 0xFF;
        byte++;
        maskLen -= 8;
    }
    if (maskLen) {
        (*mask)[byte] = ((1 << maskLen) - 1) << (8 - maskLen);
    }

    return BCM_E_NONE;
}
/*
 *   Function
 *      bcm_fe2000_g2p3_field_qualify_SrcIp6
 *   Purpose
 *      Set expected source IPv6 address for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip6_t data = which source IPv6 address
 *      (in) bcm_ip6_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_fe2000_g2p3_field_qualify_SrcIp6(void *unitData,
                                     bcm_field_entry_t entry,
                                     bcm_ip6_t data,
                                     bcm_ip6_t mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;  /* working rule */
    int result;                                 /* working result */
    unsigned int prefLen;                       /* working prefix length */
    int i,j;
    int isPrefixOver=0;

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifySrcIp6);

    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* figure out the mask length by shifting left until it's zero */
    prefLen = 0;
    for (i = 0; i < 16 ; i++) {
        for(j = 7; j >= 0; j--) {
            if(!(mask[i] & ( 1 << j))) {
                isPrefixOver = 1;
            } else if(isPrefixOver) {
                /* mask is not prefix form with significant bits set */
                FIELD_ERR((FIELD_MSG1("unit %d only supports prefix %s mask\n"),
                       thisUnit->unit,
                       _sbx_fe2000_field_qual_name[bcmFieldQualifySrcIp6]));
                return BCM_E_PARAM;
            }
            
            if(!isPrefixOver) {
                prefLen++;
            }
        }
    }

    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (thisUnit->entry[entry].hwRule) {
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 2:
            sal_memcpy(&(rule.ingress_v6.sa[0]),
                           &(data[0]),
                           sizeof(rule.ingress_v6.sa));
            rule.ingress_v6.sawidth = prefLen;
            break;
        case 3:
            sal_memcpy(&(rule.egress_v6.sa[0]),
                           &(data[0]),
                           sizeof(rule.egress_v6.sa));
            rule.egress_v6.sawidth = prefLen;
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }
    FIELD_EVERB((FIELD_MSG1("write SrcIp6(" FIELD_IPV6A_FORMAT ","
                            FIELD_IPV6A_FORMAT ") qualifier to"
                            " unit %d entry %08X\n"),
                 FIELD_IPV6A_SHOW(data),
                 FIELD_IPV6A_SHOW(mask),
                 thisUnit->unit,
                 entry));

    /* update the entry with the new rules */
    return _bcm_fe2k_g2p3_field_rules_write(thisUnit, entry, &rule);
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_qualify_DstIp6
 *   Purpose
 *      Set expected destination IPv6 address for this entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip6_t data = which destination IPv6 address
 *      (in) bcm_ip6_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_fe2000_g2p3_field_qualify_DstIp6(void *unitData,
                                     bcm_field_entry_t entry,
                                     bcm_ip6_t data,
                                     bcm_ip6_t mask)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    int result;                                 /* working result */
    unsigned int prefLen;                       /* working prefix length */
    int i,j;
    int isPrefixOver=0;

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    /* check entry valid, exists, and group has proper item in its qset */
    result = _bcm_fe2k_g2p3_field_entry_check_qset(thisUnit,
                                                   entry,
                                                   bcmFieldQualifyDstIp6);

    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }

    /* figure out the mask length by shifting left until it's zero */
    prefLen = 0;
    for (i = 0; i < 16 ; i++) {
        for(j = 7; j >= 0; j--) {
            if(!(mask[i] & ( 1 << j))) {
                isPrefixOver = 1;
            } else if(isPrefixOver) {
                /* mask is not prefix form with significant bits set */
                FIELD_ERR((FIELD_MSG1("unit %d only supports prefix %s mask\n"),
                       thisUnit->unit,
                       _sbx_fe2000_field_qual_name[bcmFieldQualifySrcIp6]));
                return BCM_E_PARAM;
            }
            
            if(!isPrefixOver) {
                prefLen++;
            }
        }
    }
    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    if (thisUnit->entry[entry].hwRule) {
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 2:
            sal_memcpy(&(rule.ingress_v6.da[0]),
                           &(data[0]),
                           sizeof(rule.ingress_v6.da));
            rule.ingress_v6.dawidth = prefLen;
            break;
        case 3:
            sal_memcpy(&(rule.egress_v6.da[0]),
                           &(data[0]),
                           sizeof(rule.egress_v6.da));
            rule.egress_v6.dawidth = prefLen;
            break;
        default:
            /* should never reach this */
            FIELD_ERR((FIELD_MSG1("unit %d group %08X qset inconsistent"
                                  " with supported\n"),
                       thisUnit->unit,
                       thisUnit->entry[entry].group));
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }
    FIELD_EVERB((FIELD_MSG1("write DstIp6(" FIELD_IPV6A_FORMAT ","
                            FIELD_IPV6A_FORMAT ") qualifier to"
                            " unit %d entry %08X\n"),
                 FIELD_IPV6A_SHOW(data),
                 FIELD_IPV6A_SHOW(mask),
                 thisUnit->unit,
                 entry));

    /* update the entry with the new rules */
    return _bcm_fe2k_g2p3_field_rules_write(thisUnit, entry, &rule);
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_action_add
 *   Purpose
 *      Add a specific action to a specific entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_field_action_t action = the action to add
 *      (in) uint32 param0 = action parameter 0 (some actions)
 *      (in) uint32 param1 = action parameter 1 (some actions)
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No hardware changes; only software state.
 */
int
bcm_fe2000_g2p3_field_action_add(void *unitData,
                                 bcm_field_entry_t entry,
                                 bcm_field_action_t action,
                                 uint32 param0,
                                 uint32 param1)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    _bcm_fe2k_g2p3_fld_entry_flags_t newFlags;  /* working flags */
    _bcm_fe2k_g2p3_fld_entry_flags_t oldFlags;  /* working flags */
    _bcm_fe2k_g2p3_fld_entry_flags_t chkFlags;  /* working flags */
    int result;                                 /* working result */
    unsigned int mirrorId = 0;                  /* working mirror ID */
    int destNode;                               /* working target node */
    int fabUnit;                                /* working fabric unit */
    int fabPort;                                /* working fabric port */
    uint32 ftIndex = ~0;                        /* working FT index */
    soc_sbx_g2p3_ft_t p3ft;                     /* working FT entry */
    bcm_mirror_destination_t mirror_dest;
    int                                unit;

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    unit = thisUnit->unit;
    result = _bcm_fe2k_g2p3_field_entry_check_action(thisUnit,
                                                     entry,
                                                     action);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* get the entry qualifiers and actions so we can update them */
    oldFlags = newFlags = thisUnit->entry[entry].entryFlags;
    chkFlags = 0;
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /*
     *  Verify the arguments for actions up front, and perform any preparatory
     *  work that would apply to all of the rules associated with the entry.
     *
     *  Also make sure the action doesn't already exist.
     */
    switch (action) {
    case bcmFieldActionDrop:
        if (thisUnit->dropPolicer) {
            chkFlags = _FE2K_G2P3_ENTRY_DROP;
        } else {
            FIELD_ERR((FIELD_MSG1("no blackhole policer available for %s"
                                  " action on unit %d\n"),
                       _sbx_fe2000_field_action_name[action],
                       thisUnit->unit));
            return BCM_E_UNAVAIL;
        }
        break;
    case bcmFieldActionCopyToCpu:
        chkFlags = _FE2K_G2P3_ENTRY_CPYTOCPU;
        break;
    case bcmFieldActionVlanNew:
        if (param0 > SBX_DYNAMIC_VSI_END(thisUnit->unit)) {
            FIELD_ERR((FIELD_MSG1("unit %d only supports VSI %04X..%04X\n"),
                       thisUnit->unit,
                       0,
                       SBX_DYNAMIC_VSI_END(thisUnit->unit) + 1));
            return BCM_E_PARAM;
        }
        chkFlags = _FE2K_G2P3_ENTRY_VLANNEW;
        break;
    case bcmFieldActionCosQNew:
        if (param0 >= SBX_MAX_COS) {
            FIELD_ERR((FIELD_MSG1("unit %d only supports COS queues %d..%d\n"),
                       thisUnit->unit,
                       0,
                       SBX_MAX_COS));
            return BCM_E_PARAM;
        }
        chkFlags = _FE2K_G2P3_ENTRY_COSQNEW;
        break;
    case bcmFieldActionDropPrecedence:
        if (param0 >= 4  ) {
            FIELD_ERR((FIELD_MSG1("unit %d only suports DP values"
                                  " %d..%d\n"),
                       thisUnit->unit,
                       0,
                       4 ));
            return BCM_E_PARAM;
        }
        chkFlags = _FE2K_G2P3_ENTRY_DROPPREC;
        break;
    case bcmFieldActionL3Switch:
        if (!(_FE2K_L3_FTE_VALID(thisUnit->unit, param0))) {
            FIELD_ERR((FIELD_MSG1("unit %d only supports FT values"
                                  " %d..%d\n"),
                       thisUnit->unit,
                       0,
                       0x0003FFFF));
            return BCM_E_PARAM;
        }
        chkFlags = _FE2K_G2P3_ENTRY_L3SWITCH;
        break;
    case bcmFieldActionRedirect:
        if ((param0 >= SBX_MAX_MODIDS) || (param1 >= SBX_MAX_PORTS)) {
            /* port or module invalid; bad parameter */
            FIELD_ERR((FIELD_MSG1("module %d or port %d is invalid\n"),
                       param0,
                       param1));
            return BCM_E_PARAM;
        }
        result = soc_sbx_node_port_get(thisUnit->unit,
                                       param0, /* modid */
                                       param1, /* port */
                                       &fabUnit,
                                       &destNode,
                                       &fabPort);
        if (BCM_E_NONE == result) {
            /* make sure target node is accessible */
            if (!SOC_SBX_NODE_ADDRESSABLE(thisUnit->unit, destNode)) {
                /* inaccessible destination node */
                FIELD_ERR((FIELD_MSG1("node %d (module %d port %d) is"
                                      "inaccessible from unit %d\n"),
                           destNode,
                           param0,
                           param1,
                           thisUnit->unit));
                return BCM_E_BADID;
            }
        } else {
            FIELD_ERR((FIELD_MSG1("unable to locate redirect target for mod"
                                  " %d port %d: %d (%s)\n"),
                       param0,
                       param1,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
        chkFlags = _FE2K_G2P3_ENTRY_REDIRECT;
        break;
    case bcmFieldActionRedirectMcast:
        result = _sbx_gu2_resource_alloc(thisUnit->unit,
                                         SBX_GU2K_USR_RES_FTE_L2MC,
                                         1,
                                         &ftIndex,
                                         0);
        if (BCM_E_NONE == result) {
            soc_sbx_g2p3_ft_t_init(&p3ft);
            p3ft.excidx = 0;
            p3ft.hc = FALSE;
            p3ft.oi = param0
#ifdef MCAST_MVT_BASE
                      + MCAST_MVT_BASE
#endif /* def MCAST_MVT_BASE */
                      ;
            p3ft.qid = MCAST_QID_BASE;
            p3ft.mc = 1;
            result = soc_sbx_g2p3_ft_set(thisUnit->unit, ftIndex, &p3ft);
            if (BCM_E_NONE != result) {
                FIELD_ERR((FIELD_MSG1("unable to write %d:ft[%08X] for"
                                      " %s(%08X) action: %d (%s)\n"),
                           thisUnit->unit,
                           ftIndex,
                           _sbx_fe2000_field_action_name[bcmFieldActionRedirectMcast],
                           param0,
                           result,
                           _SHR_ERRMSG(result)));
                _sbx_gu2_resource_free(thisUnit->unit,
                                       SBX_GU2K_USR_RES_FTE_L2MC,
                                       1,
                                       &ftIndex,
                                       0);
                return result;
            }
        } else {
            ftIndex = 0;
            FIELD_ERR((FIELD_MSG1("unable to allocate FT entry for %s(%08X)"
                                  " action: %d (%s)\n"),
                       _sbx_fe2000_field_action_name[bcmFieldActionRedirectMcast],
                       param0,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
        chkFlags = _FE2K_G2P3_ENTRY_REDIRMC;
        break;
    case bcmFieldActionMirrorIngress:
        /* mirror destination gport */
        if (BCM_GPORT_IS_MIRROR(param1)) {
            result = bcm_mirror_destination_get(thisUnit->unit, param1, &mirror_dest);
            if (BCM_E_NONE != result) {
                return result;
            }
        } else {
            FIELD_ERR((FIELD_MSG1("Invalid Mirror port (%08X)\n"), param1));
            return BCM_E_PARAM;
        }
        result = _bcm_fe2000_ingr_mirror_alloc(thisUnit->unit,
                                               &mirrorId,
                                               mirror_dest.gport);
        if (BCM_E_NONE != result) {
            /* called function should have displayed reason */
            return result;
        }
        chkFlags = _FE2K_G2P3_ENTRY_MIRROR;
        break;
    case bcmFieldActionMirrorEgress:
        /* mirror destination gport */
        if (BCM_GPORT_IS_MIRROR(param1)) {
            result = bcm_mirror_destination_get(thisUnit->unit, param1, &mirror_dest);
            if (BCM_E_NONE != result) {
                return result;
            }
        } else {
            FIELD_ERR((FIELD_MSG1("Invalid Mirror port (%08X)\n"), param1));
            return BCM_E_PARAM;
        }
        result = _bcm_fe2000_egr_mirror_alloc(thisUnit->unit,
                                              &mirrorId,
                                              mirror_dest.gport);
        if (BCM_E_NONE != result) {
            /* called function should have displayed reason */
            return result;
        }
        chkFlags = _FE2K_G2P3_ENTRY_MIRROR;
        break;
    case bcmFieldActionUpdateCounter:
        chkFlags = _FE2K_G2P3_ENTRY_CTR_ENA;
        break;
    default:
        FIELD_ERR((FIELD_MSG1("unexpected action %s\n"),
                   _sbx_fe2000_field_action_name[action]));
        return BCM_E_INTERNAL;
    } /* switch (action) */
    if (oldFlags & chkFlags) {
        /* the action already exists */
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X already has %s action\n"),
                   thisUnit->unit,
                   entry,
                   _sbx_fe2000_field_action_name[action]));
        return  _SBX_FE2K_FIELD_DUPLICATE_ACTION_ERR;
    }
    /* anticipate success here */
    newFlags |= chkFlags;
    /* deal with counter update (special) */
    if ((BCM_E_NONE == result) && (bcmFieldActionUpdateCounter == action)) {
        /*
         *  Updating counters is handled within this module instead of by
         *  passing any kind of data to the lower layer, so counter setup
         *  is all done here and is therefore common to all stages.  Note we
         *  also don't bother writing the rule again when setting counter
         *  mode, since hardware isn't aware of any of this stuff.
         */
        if (!(thisUnit->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_COUNTER)) {
            FIELD_ERR((FIELD_MSG1("unit %d entry %08X does not have an"
                                  " assigned counter\n"),
                       thisUnit->unit,
                       entry));
            return BCM_E_CONFIG;
        }
        if (!(BCM_FIELD_COUNTER_MODE_ALL & param0)) {
            FIELD_ERR((FIELD_MSG1("unit %d assuming request to count NO frame"
                                  " colours means count ALL colours\n"),
                       thisUnit->unit));
            param0 |= BCM_FIELD_COUNTER_MODE_ALL;
        }
        if (BCM_FIELD_COUNTER_MODE_ALL !=
            (BCM_FIELD_COUNTER_MODE_ALL & param0)) {
            FIELD_ERR((FIELD_MSG1("unit %d can only count ALL colours\n"),
                       thisUnit->unit));
            return BCM_E_PARAM;
        }
        if ((~(BCM_FIELD_COUNTER_MODE_ALL | BCM_FIELD_COUNTER_MODE_BYTES |
               0x000F)) & param0) {
            FIELD_ERR((FIELD_MSG1("unit %d does not support bits %08X in"
                                  " counter mode\n"),
                       thisUnit->unit,
                       (~(BCM_FIELD_COUNTER_MODE_ALL |
                          BCM_FIELD_COUNTER_MODE_BYTES |
                          0x000F)) & param0));
            return BCM_E_PARAM;
        }
        switch (param0 & 0x000F) {
        case BCM_FIELD_COUNTER_MODE_NO_NO:
            /* fallthrough intentional */
        case BCM_FIELD_COUNTER_MODE_NO_YES:
            /* fallthrough intentional */
        case BCM_FIELD_COUNTER_MODE_YES_NO:
            /* fallthrough intentional */
        case BCM_FIELD_COUNTER_MODE_BYTES_PACKETS:
            /* caller specified a supported mode */
            thisUnit->entry[entry].counterMode = param0;
            thisUnit->entry[entry].entryFlags = newFlags;
            return BCM_E_NONE;
        default:
            /* caller specified an unsupported mode */
            FIELD_ERR((FIELD_MSG1("unit %d does not support counter mode %d\n"),
                       thisUnit->unit,
                       param0 & 0xF));
            return BCM_E_PARAM;
        }
    }
    if (thisUnit->entry[entry].hwRule) {
        /* apply the action to the entry's rules */
        /*
         *  Make changes to actions that are different per rulebase.  Also
         *  checks that the action is valid, so actions that are common to all
         *  rulebases will appear in here with merely a break statement per
         *  rulebase, so they don't get flagged as a problem.
         */
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            switch (action) {
            case bcmFieldActionDrop:
                rule.ingress.policer = thisUnit->dropPolicer;
                rule.ingress.mef = FALSE;
                rule.ingress.mefcos = FALSE;
                rule.ingress.typedpolice = FALSE;
                break;
            case bcmFieldActionVlanNew:
                if (!(oldFlags & _FE2K_G2P3_ENTRY_ACT_FTVL)) {
                    /* no action collision */
                    rule.ingress.ftidx = 0; 
                    rule.ingress.vlan = param0;
                    rule.ingress.usevlan = TRUE;
                } else {
                    /* vlannew collides with existing action */
                    result = _SBX_FE2K_FIELD_OVERLAP_ACTION_ERR;
                }
                break;
            case bcmFieldActionL3Switch:
                if (!(oldFlags & _FE2K_G2P3_ENTRY_ACT_FTVL)) {
                    /* no action collision */
                    rule.ingress.vlan = 0; 
                    rule.ingress.ftidx = _FE2K_GET_FTE_IDX_FROM_USER_HANDLE(param0);
                    rule.ingress.useftidx = TRUE;
                } else {
                    /* l3switch collides with existing action */
                    result = _SBX_FE2K_FIELD_OVERLAP_ACTION_ERR;
                }
                break;
            case bcmFieldActionRedirect:
                if (!(oldFlags & _FE2K_G2P3_ENTRY_ACT_FTVL)) {
                    /* no action collision */
                    rule.ingress.vlan = 0; 
                    rule.ingress.ftidx = SOC_SBX_PORT_FTE(thisUnit->unit,
                                                                    destNode,
                                                                    fabPort);
                    rule.ingress.useftidx = TRUE;
                } else {
                    /* redirect collides with existing action */
                    result = _SBX_FE2K_FIELD_OVERLAP_ACTION_ERR;
                }
                break;
            case bcmFieldActionRedirectMcast:
                if (!(oldFlags & _FE2K_G2P3_ENTRY_ACT_FTVL)) {
                    /* no action collision */
                    rule.ingress.vlan = 0; 
                    rule.ingress.ftidx = ftIndex;
                    rule.ingress.useftidx = TRUE;
                } else {
                    /* redirect collides with existing action */
                    result = _SBX_FE2K_FIELD_OVERLAP_ACTION_ERR;
                }
                break;
            case bcmFieldActionCosQNew:
                
                rule.ingress.cos = param0;
                rule.ingress.fcos = param0;
                rule.ingress.usecos = TRUE;
                break;
            case bcmFieldActionDropPrecedence:
                rule.ingress.dp = param0;
                rule.ingress.usedp = TRUE;
                break;
            case bcmFieldActionMirrorIngress:
                rule.ingress.mirror = mirrorId;
                break;
            case bcmFieldActionCopyToCpu:
                rule.ingress.copy = TRUE;
                break;
            default:
                /* should never reach this */
                result = BCM_E_INTERNAL;
            } /* switch (action) */
            break;
        case 1:
            switch (action) {
            case bcmFieldActionDrop:
                rule.egress.drop = TRUE;
                break;
            case bcmFieldActionMirrorEgress:
                rule.egress.mirror = mirrorId;
                break;
            default:
                /* should never reach this */
                result = BCM_E_INTERNAL;
            } /* switch (action) */
            break;
        case 2:
            switch (action) {
            case bcmFieldActionDrop:
                rule.ingress_v6.policer = thisUnit->dropPolicer;
                rule.ingress_v6.mef = FALSE;
                rule.ingress_v6.mefcos = FALSE;
                rule.ingress_v6.typedpolice = FALSE;
                break;
            case bcmFieldActionVlanNew:
                if (!(oldFlags & _FE2K_G2P3_ENTRY_ACT_FTVL)) {
                    /* no action collision */
                    rule.ingress_v6.ftidx = 0; 
                    rule.ingress_v6.vlan = param0;
                    rule.ingress_v6.usevlan = TRUE;
                } else {
                    /* vlannew collides with existing action */
                    result = _SBX_FE2K_FIELD_OVERLAP_ACTION_ERR;
                }
                break;
            case bcmFieldActionL3Switch:
                if (!(oldFlags & _FE2K_G2P3_ENTRY_ACT_FTVL)) {
                    /* no action collision */
                    rule.ingress_v6.vlan = 0; 
                    rule.ingress_v6.ftidx = _FE2K_GET_FTE_IDX_FROM_USER_HANDLE(param0);
                    rule.ingress_v6.useftidx = TRUE;
                } else {
                    /* l3switch collides with existing action */
                    result = _SBX_FE2K_FIELD_OVERLAP_ACTION_ERR;
                }
                break;
            case bcmFieldActionRedirect:
                if (!(oldFlags & _FE2K_G2P3_ENTRY_ACT_FTVL)) {
                    /* no action collision */
                    rule.ingress_v6.vlan = 0; 
                    rule.ingress_v6.ftidx = SOC_SBX_PORT_FTE(thisUnit->unit,
                                                                    destNode,
                                                                    fabPort);
                    rule.ingress_v6.useftidx = TRUE;
                } else {
                    /* redirect collides with existing action */
                    result = _SBX_FE2K_FIELD_OVERLAP_ACTION_ERR;
                }
                break;
            case bcmFieldActionRedirectMcast:
                if (!(oldFlags & _FE2K_G2P3_ENTRY_ACT_FTVL)) {
                    /* no action collision */
                    rule.ingress_v6.vlan = 0; 
                    rule.ingress_v6.ftidx = ftIndex;
                    rule.ingress_v6.useftidx = TRUE;
                } else {
                    /* redirect collides with existing action */
                    result = _SBX_FE2K_FIELD_OVERLAP_ACTION_ERR;
                }
                break;
            case bcmFieldActionCosQNew:
                
                rule.ingress_v6.cos = param0;
                rule.ingress_v6.fcos = param0;
                rule.ingress_v6.usecos = TRUE;
                break;
            case bcmFieldActionDropPrecedence:
                rule.ingress_v6.dp = param0;
                rule.ingress_v6.usedp = TRUE;
                break;
            case bcmFieldActionMirrorIngress:
                rule.ingress_v6.mirror = mirrorId;
                break;
            case bcmFieldActionCopyToCpu:
                rule.ingress_v6.copy = TRUE;
                break;
            default:
                /* should never reach this */
                result = BCM_E_INTERNAL;
            } /* switch (action) */
            break;
        case 3:
            switch (action) {
            case bcmFieldActionDrop:
                rule.egress_v6.drop = TRUE;
                break;
            case bcmFieldActionMirrorEgress:
                rule.egress_v6.mirror = mirrorId;
                break;
            default:
                /* should never reach this */
                result = BCM_E_INTERNAL;
            } /* switch (action) */
            break;
        default:
            /* should never reach this */
            result = BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }
    /* make changes to actions that are common to all rulebases */
    /* commit changes and clean up */
    if (BCM_E_NONE == result) {
        /* update the entry rules with the added/updated action */
        FIELD_EVERB((FIELD_MSG1("write %s(%08X,%08X) action to"
                                " unit %d entry %08X\n"),
                     _sbx_fe2000_field_action_name[action],
                     param0,
                     param1,
                     thisUnit->unit,
                     entry));
        result =  _bcm_fe2k_g2p3_field_rules_write(thisUnit, entry, &rule);
        /* called function would have displayed any error message */
    } else {
        switch (result) {
        case _SBX_FE2K_FIELD_OVERLAP_ACTION_ERR:
            FIELD_ERR((FIELD_MSG1("unit %d entry %08X already has an"
                                  " action that collides with %s\n"),
                       thisUnit->unit,
                       entry,
                       _sbx_fe2000_field_action_name[action]));
            break;
        default:
            FIELD_ERR((FIELD_MSG1("unable to add %s action to unit %d"
                                  " entry %08X: %d (%s)\n"),
                       _sbx_fe2000_field_action_name[action],
                       thisUnit->unit,
                       entry,
                       result,
                       _SHR_ERRMSG(result)));
        }
    }
    if (BCM_E_NONE == result) {
        /* commit any changes that are conditional upon success */
        FIELD_EVERB((FIELD_MSG1("update unit %d entry %08X flags"
                                " %08X -> %08X\n"),
                     thisUnit->unit,
                     entry,
                     thisUnit->entry[entry].entryFlags,
                     newFlags));
        thisUnit->entry[entry].entryFlags = newFlags;
        /* update any cached data */
        switch (action) {
        case bcmFieldActionRedirect:
            thisUnit->entry[entry].ftvlData.target.module = param0;
            thisUnit->entry[entry].ftvlData.target.port = param1;
            break;
        case bcmFieldActionL3Switch:
            thisUnit->entry[entry].ftvlData.ftHandle = param0;
            break;
        case bcmFieldActionVlanNew:
            thisUnit->entry[entry].ftvlData.VSI = param0;
            break;
        case bcmFieldActionRedirectMcast:
            thisUnit->entry[entry].ftvlData.mcHandle = param0;
            break;
        default:
            /* to keep compiler from complaining */
            break;
        }
    } else { /* if (BCM_E_NONE == result) */
        FIELD_EVERB((FIELD_MSG1("free any resources allocated for action\n")));
        /* clean up any resources allocated */
        if ((bcmFieldActionMirrorIngress == action) && mirrorId) {
            _bcm_fe2000_ingr_mirror_free(thisUnit->unit, mirrorId);
        }
        if ((bcmFieldActionMirrorEgress == action) && mirrorId) {
            _bcm_fe2000_egr_mirror_free(thisUnit->unit, mirrorId);
        }
        if ((bcmFieldActionRedirectMcast == action) && ftIndex) {
            _sbx_gu2_resource_free(thisUnit->unit,
                                   SBX_GU2K_USR_RES_FTE_L2MC,
                                   1,
                                   &ftIndex,
                                   0);
        }
    } /* if (BCM_E_NONE == result) */
    /* return the result */
    return result;
}
#ifdef BCM_WARM_BOOT_SUPPORT
/*
 *   Function
 *      bcm_fe2000_g2p3_field_action_wb_recover
 *   Purpose
 *      Recover state information during warmboot operation
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No hardware changes; only software state.
 */
static int
_bcm_fe2000_g2p3_field_action_wb_recover(void *unitData,
                                 int rulebase,
                                 bcm_field_entry_t entry,
                                 _bcm_fe2k_g2p3_hardware_rule_t *pRule)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_fld_entry_flags_t flags;     /* working flags */
    int result = BCM_E_NONE;                    /* working result */
    unsigned int ftidx, vlan, mirrorId;
    int unit;

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    unit = thisUnit->unit;

    /* read information from the hardware rule */
    switch (rulebase) {
    case 0:
        ftidx = pRule->ingress.ftidx;
        vlan = pRule->ingress.vlan;
        mirrorId = pRule->ingress.mirror;
        break;
    case 1:
        ftidx = 0;
        vlan = 0;
        mirrorId = pRule->egress.mirror;
        break;
    case 2:
        ftidx = pRule->ingress_v6.ftidx;
        vlan = pRule->ingress_v6.vlan;
        mirrorId = pRule->ingress_v6.mirror;
        break;
    case 3:
        ftidx = 0;
        vlan = 0;
        mirrorId = pRule->egress_v6.mirror;
        break;
    default:
        return BCM_E_INTERNAL;
    }

    flags = thisUnit->entry[entry].entryFlags;

    if (flags & _FE2K_G2P3_ENTRY_DROP) {
        /* bcmFieldActionDrop */
    }
    if (flags & _FE2K_G2P3_ENTRY_CPYTOCPU) {
        /* bcmFieldActionCopyToCpu */
    }
    if (flags & _FE2K_G2P3_ENTRY_VLANNEW) {
        /* bcmFieldActionVlanNew */
        thisUnit->entry[entry].ftvlData.VSI = vlan;
    }
    if (flags & _FE2K_G2P3_ENTRY_COSQNEW) {
        /* bcmFieldActionCosQNew */
    }
    if (flags & _FE2K_G2P3_ENTRY_DROPPREC) {
        /* bcmFieldActionDropPrecedence */
    }
    if (flags & _FE2K_G2P3_ENTRY_L3SWITCH) {
        /* bcmFieldActionL3Switch */
        thisUnit->entry[entry].ftvlData.ftHandle = ftidx;
    }
    if (flags & _FE2K_G2P3_ENTRY_REDIRECT) {
        /* bcmFieldActionRedirect */
#if 0
        
        soc_sbx_g2p3_ft_t           fte;
        bcm_gport_t         fabric_port, switch_port;
        int fab_node, fab_port;

        soc_sbx_g2p3_ft_t_init(&fte);
        BCM_IF_ERROR_RETURN(soc_sbx_g2p3_ft_get(thisUnit->unit, ftidx, &fte));

        SOC_SBX_NODE_PORT_FROM_QID(thisUnit->unit, fte.qid, fab_node, fab_port, NUM_COS(thisUnit->unit));
        SOC_SBX_MODID_FROM_NODE(fab_node, fab_node);
        BCM_GPORT_MODPORT_SET(fabric_port, fab_node, fab_port);
        result = bcm_sbx_stk_fabric_map_get_switch_port(thisUnit->unit, 
                                                    fabric_port, &switch_port);
        if (BCM_FAILURE(result)) {
            FIELD_ERR((_SBX_D(unit, "Failed to convert fabric port to switch"
                           " port: %s\n"), bcm_errmsg(result)));
            return result;
        }

        thisUnit->entry[entry].ftvlData.target.module = BCM_GPORT_MODPORT_MODID_GET(switch_port);
        thisUnit->entry[entry].ftvlData.target.port = BCM_GPORT_MODPORT_PORT_GET(switch_port);
#endif
    }
    if (flags & _FE2K_G2P3_ENTRY_REDIRMC) {
        /* bcmFieldActionRedirectMcast */
        thisUnit->entry[entry].ftvlData.mcHandle = vlan;
        /* reallocate the shared resource */
        result = _sbx_gu2_resource_alloc(thisUnit->unit,
                                         SBX_GU2K_USR_RES_FTE_L2MC,
                                         1,
                                         &vlan,
                                         _SBX_GU2_RES_FLAGS_RESERVE);
        if (BCM_E_NONE != result) {
            FIELD_ERR((FIELD_MSG1("unable to recover FTE resource, unit %d entry %08X"
                                  " FTE 0x%0x: %d (%s); aborting\n"),
                       thisUnit->unit,
                       entry,
                       result,
                       vlan,
                       _SHR_ERRMSG(result)));
        }

    }
    if (flags & _FE2K_G2P3_ENTRY_MIRROR) {
        /* bcmFieldActionMirrorIngress */
    }
    if (flags & _FE2K_G2P3_ENTRY_MIRROR) {
        /* bcmFieldActionMirrorEgress */
        /*
         * No need to reallocate here as mirror IDs are recoverd during
         * warmboot of the mirror module.
         */
    }
    if (flags & _FE2K_G2P3_ENTRY_CTR_ENA) {
        /* bcmFieldActionUpdateCounter */
    }

    return result;
}
#endif /* BCM_WARM_BOOT_SUPPORT */
/*
 *   Function
 *      bcm_fe2000_g2p3_field_action_get
 *   Purpose
 *      Get a specific action from a specific entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_field_action_t action = the action to remove
 *      (out) uint32 param0 = where to put action parameter 0 (some actions)
 *      (out) uint32 param1 = where to put action parameter 1 (some actions)
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No hardware changes; only software state.
 *      If the action does not use one or both of param0 or param1, the unused
 *      will be zeroed.
 */
int
bcm_fe2000_g2p3_field_action_get(void *unitData,
                                 bcm_field_entry_t entry,
                                 bcm_field_action_t action,
                                 uint32 *param0,
                                 uint32 *param1)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;  /* working rule */
    _bcm_fe2k_g2p3_fld_entry_flags_t oldFlags;  /* working flags */
    int result;                                 /* working result */
    bcm_gport_t gportId;                        /* working Gport ID */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    result = _bcm_fe2k_g2p3_field_entry_check_action(thisUnit,
                                                     entry,
                                                     action);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* get the entry qualifiers and actions so we can update them */
    oldFlags = thisUnit->entry[entry].entryFlags;
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* make sure the action exists */
    switch (action) {
    case bcmFieldActionDrop:
        oldFlags &= _FE2K_G2P3_ENTRY_DROP;
        break;
    case bcmFieldActionCopyToCpu:
        oldFlags &= _FE2K_G2P3_ENTRY_CPYTOCPU;
        break;
    case bcmFieldActionVlanNew:
        oldFlags &= _FE2K_G2P3_ENTRY_VLANNEW;
        break;
    case bcmFieldActionCosQNew:
        oldFlags &= _FE2K_G2P3_ENTRY_COSQNEW;
        break;
    case bcmFieldActionDropPrecedence:
        oldFlags &= _FE2K_G2P3_ENTRY_DROPPREC;
        break;
    case bcmFieldActionL3Switch:
        oldFlags &= _FE2K_G2P3_ENTRY_L3SWITCH;
        break;
    case bcmFieldActionRedirect:
        oldFlags &= _FE2K_G2P3_ENTRY_REDIRECT;
        break;
    case bcmFieldActionRedirectMcast:
        oldFlags &= _FE2K_G2P3_ENTRY_REDIRMC;
        break;
    case bcmFieldActionMirrorIngress:
        /* fallthrough intentional; same flag for ingress & egress */
    case bcmFieldActionMirrorEgress:
        oldFlags &= _FE2K_G2P3_ENTRY_MIRROR;
        break;
    case bcmFieldActionUpdateCounter:
        oldFlags &= _FE2K_G2P3_ENTRY_CTR_ENA;
        break;
    default:
        FIELD_ERR((FIELD_MSG1("unexpected action %s\n"),
                   _sbx_fe2000_field_action_name[action]));
        return BCM_E_INTERNAL;
    } /* switch (action) */
    if (!oldFlags) {
        /* the action does not exist */
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X has no %s action\n"),
                   thisUnit->unit,
                   entry,
                   _sbx_fe2000_field_action_name[action]));
        return BCM_E_NOT_FOUND;
    }
    /* deal with cached action types (shortcut) */
    switch (action) {
    case bcmFieldActionUpdateCounter:
        *param0 = thisUnit->entry[entry].counterMode;
        *param1 = 0;
        return BCM_E_NONE;
    case bcmFieldActionRedirect:
        *param0 = thisUnit->entry[entry].ftvlData.target.module;
        *param1 = thisUnit->entry[entry].ftvlData.target.port;
        return BCM_E_NONE;
    case bcmFieldActionL3Switch:
        *param0 = thisUnit->entry[entry].ftvlData.ftHandle;
        *param1 = 0;
        return BCM_E_NONE;
    case bcmFieldActionVlanNew:
        *param0 = thisUnit->entry[entry].ftvlData.VSI;
        *param1 = 0;
        return BCM_E_NONE;
    case bcmFieldActionRedirectMcast:
        *param0 = thisUnit->entry[entry].ftvlData.mcHandle;
        *param1 = 0;
        return BCM_E_NONE;
    case bcmFieldActionCopyToCpu:
        /* no arguments, so fall through to Drop (also no args) */
    case bcmFieldActionDrop:
        *param0 = 0;
        *param1 = 0;
        return BCM_E_NONE;
    default:
        /* so the compiler doesn't complain */
        break;
    }
    if (!thisUnit->entry[entry].hwRule) {
        /* didn't find one; entry has no rules for some reason */
        FIELD_ERR((FIELD_MSG1("unable to locate a rule for unit %d"
                              " entry %08X\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_INTERNAL;
    }
    /* fill in the results based upon the data in the rule */
    FIELD_EVERB((FIELD_MSG1("read %s action from unit %d entry %08X\n"),
                 _sbx_fe2000_field_action_name[action],
                 thisUnit->unit,
                 entry));
    switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
    case 0:
        switch (action) {
        case bcmFieldActionCosQNew:
            *param0 = rule.ingress.cos;
            *param1 = 0;
            break;
        case bcmFieldActionDropPrecedence:
            *param0 = rule.ingress.dp;
            *param1 = 0;
            break;
        case bcmFieldActionMirrorIngress:
            result = _bcm_fe2000_ingr_mirror_get(thisUnit->unit,
                                                 rule.ingress.mirror,
                                                 &gportId);
            /* called function should have displayed any error diagnostic */
            /* todo: provide mirror_gport rather than mod/port */
            if (BCM_E_NONE == result) {
		if (BCM_GPORT_IS_MODPORT(gportId)) {
                    *param0 = BCM_GPORT_MODPORT_MODID_GET(gportId);
                    *param1 = BCM_GPORT_MODPORT_PORT_GET(gportId);
		}
            }
            break;
        default:
            /* should never reach this */
            result = BCM_E_INTERNAL;
        } /* switch (action) */
        break;
    case 1:
        switch (action) {
        case bcmFieldActionMirrorEgress:
            result = _bcm_fe2000_egr_mirror_get(thisUnit->unit,
                                                rule.egress.mirror,
                                                &gportId);
            /* called function should have displayed any error diagnostic */
            /* todo: provide mirror_gport rather than mod/port */
            if (BCM_E_NONE == result) {
		if (BCM_GPORT_IS_MODPORT(gportId)) {
                    *param0 = BCM_GPORT_MODPORT_MODID_GET(gportId);
                    *param1 = BCM_GPORT_MODPORT_PORT_GET(gportId);
		}
            }
            break;
        default:
            /* should never reach this */
            result = BCM_E_INTERNAL;
        } /* switch (action) */
        break;
    case 2:
        switch (action) {
        case bcmFieldActionCosQNew:
            *param0 = rule.ingress_v6.cos;
            *param1 = 0;
            break;
        case bcmFieldActionDropPrecedence:
            *param0 = rule.ingress_v6.dp;
            *param1 = 0;
            break;
        case bcmFieldActionMirrorIngress:
            result = _bcm_fe2000_ingr_mirror_get(thisUnit->unit,
                                                 rule.ingress_v6.mirror,
                                                 &gportId);
            /* called function should have displayed any error diagnostic */
            if (BCM_E_NONE == result) {
		if (BCM_GPORT_IS_MODPORT(gportId)) {
                    *param0 = BCM_GPORT_MODPORT_MODID_GET(gportId);
                    *param1 = BCM_GPORT_MODPORT_PORT_GET(gportId);
		}
            }
            break;
        default:
            /* should never reach this */
            result = BCM_E_INTERNAL;
        } /* switch (action) */
        break;
    case 3:
        switch (action) {
        case bcmFieldActionMirrorEgress:
            result = _bcm_fe2000_egr_mirror_get(thisUnit->unit,
                                                rule.egress_v6.mirror,
                                                &gportId);
            /* called function should have displayed any error diagnostic */
            if (BCM_E_NONE == result) {
		if (BCM_GPORT_IS_MODPORT(gportId)) {
                    *param0 = BCM_GPORT_MODPORT_MODID_GET(gportId);
                    *param1 = BCM_GPORT_MODPORT_PORT_GET(gportId);
		}
            }
            break;
        default:
            /* should never reach this */
            result = BCM_E_INTERNAL;
        } /* switch (action) */
        break;
    default:
        /* should never reach this */
        result = BCM_E_INTERNAL;
    } /* switch (rulebase) */
    if (BCM_E_NONE != result) {
        FIELD_ERR((FIELD_MSG1("unable to read %s action from unit %d"
                              " entry %08X: %d (%s)\n"),
                   _sbx_fe2000_field_action_name[action],
                   thisUnit->unit,
                   entry,
                   result,
                   _SHR_ERRMSG(result)));
    }
    /* return the result */
    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_action_remove
 *   Purpose
 *      Remove a specific action from a specific entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_field_action_t action = the action to remove
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No hardware changes; only software state.
 */
int
bcm_fe2000_g2p3_field_action_remove(void *unitData,
                                    bcm_field_entry_t entry,
                                    bcm_field_action_t action)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;  /* working rule */
    _bcm_fe2k_g2p3_fld_entry_flags_t newFlags;  /* working flags */
    _bcm_fe2k_g2p3_fld_entry_flags_t chkFlags;  /* working flags */
    int result;                                 /* working result */
    unsigned int mirrorId = 0;                  /* working mirror ID */
    uint32 ftIndex = 0;                         /* working FT index */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    result = _bcm_fe2k_g2p3_field_entry_check_action(thisUnit,
                                                     entry,
                                                     action);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* get the entry qualifiers and actions so we can update them */
    newFlags = thisUnit->entry[entry].entryFlags;
    chkFlags = 0;
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* make sure the action exists */
    switch (action) {
    case bcmFieldActionDrop:
        chkFlags = _FE2K_G2P3_ENTRY_DROP;
        break;
    case bcmFieldActionCopyToCpu:
        chkFlags = _FE2K_G2P3_ENTRY_CPYTOCPU;
        break;
    case bcmFieldActionVlanNew:
        chkFlags = _FE2K_G2P3_ENTRY_VLANNEW;
        break;
    case bcmFieldActionCosQNew:
        chkFlags = _FE2K_G2P3_ENTRY_COSQNEW;
        break;
    case bcmFieldActionDropPrecedence:
        chkFlags = _FE2K_G2P3_ENTRY_DROPPREC;
        break;
    case bcmFieldActionL3Switch:
        chkFlags = _FE2K_G2P3_ENTRY_L3SWITCH;
        break;
    case bcmFieldActionRedirect:
        chkFlags = _FE2K_G2P3_ENTRY_REDIRECT;
        break;
    case bcmFieldActionRedirectMcast:
        chkFlags = _FE2K_G2P3_ENTRY_REDIRMC;
        break;
    case bcmFieldActionMirrorIngress:
        /* fallthrough intentional; same flag for ingress & egress */
    case bcmFieldActionMirrorEgress:
        chkFlags = _FE2K_G2P3_ENTRY_MIRROR;
        break;
    case bcmFieldActionUpdateCounter:
        chkFlags = _FE2K_G2P3_ENTRY_CTR_ENA;
        break;
    default:
        FIELD_ERR((FIELD_MSG1("unexpected action %s\n"),
                   _sbx_fe2000_field_action_name[action]));
        return BCM_E_INTERNAL;
    } /* switch (action) */
    if (!(newFlags & chkFlags)) {
        /* the action does not exist */
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X has no %s action\n"),
                   thisUnit->unit,
                   entry,
                   _sbx_fe2000_field_action_name[action]));
        return BCM_E_NOT_FOUND;
    }
    /* anticipate success here */
    newFlags &= (~chkFlags);
    /* deal with counter update (special) */
    if ((BCM_E_NONE == result) && (bcmFieldActionUpdateCounter == action)) {
        /*
         *  Updating counters is handled within this module instead of by
         *  passing any kind of data to the lower layer, so counter setup
         *  is all done here and is therefore common to all stages.  We don't
         *  bother to write the rules since none of that matters for counters.
         */
        thisUnit->entry[entry].entryFlags = newFlags;
        thisUnit->entry[entry].counterMode = 0;
        return BCM_E_NONE;
    }
    /* remove the action from the entry's rule */
    if (thisUnit->entry[entry].hwRule) {
        switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
        case 0:
            switch (action) {
            case bcmFieldActionDrop:
                rule.ingress.policer = thisUnit->entry[entry].policer;
                rule.ingress.typedpolice = !(!(thisUnit->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_TYPEPOL));
                rule.ingress.mefcos = !(!(thisUnit->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_MEFCOS));
                rule.ingress.mef = !(!(thisUnit->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_MEF));
                break;
            case bcmFieldActionVlanNew:
                rule.ingress.ftidx = 0; 
                rule.ingress.vlan = 0;
                rule.ingress.usevlan = FALSE;
                break;
            case bcmFieldActionRedirectMcast:
                ftIndex = rule.ingress.ftidx;
                /* fallthrough intentional; same fields affected */
            case bcmFieldActionL3Switch:
                /* fallthrough intentional; same fields affected */
            case bcmFieldActionRedirect:
                rule.ingress.vlan = 0; 
                rule.ingress.ftidx = 0;
                rule.ingress.useftidx = FALSE;
                break;
            case bcmFieldActionCosQNew:
                rule.ingress.cos = 0;
                rule.ingress.fcos = 0;
                rule.ingress.usecos = FALSE;
                break;
            case bcmFieldActionDropPrecedence:
                rule.ingress.dp = 0;
                rule.ingress.usedp = FALSE;
                break;
            case bcmFieldActionMirrorIngress:
                if (!mirrorId) {
                    mirrorId = rule.ingress.mirror;
                }
                rule.ingress.mirror = 0;
                break;
            case bcmFieldActionCopyToCpu:
                rule.ingress.copy = FALSE;
                break;
            default:
                /* should never reach this */
                result = BCM_E_INTERNAL;
            } /* switch (action) */
            break;
        case 1:
            switch (action) {
            case bcmFieldActionDrop:
                rule.egress.drop = FALSE;
                break;
            case bcmFieldActionMirrorEgress:
                if (!mirrorId) {
                    mirrorId = rule.egress.mirror;
                }
                rule.egress.mirror = 0;
                break;
            default:
                /* should never reach this */
                result = BCM_E_INTERNAL;
            } /* switch (action) */
            break;
        case 2:
            switch (action) {
            case bcmFieldActionDrop:
                rule.ingress_v6.policer = thisUnit->entry[entry].policer;
                rule.ingress_v6.typedpolice = !(!(thisUnit->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_TYPEPOL));
                rule.ingress_v6.mefcos = !(!(thisUnit->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_MEFCOS));
                rule.ingress_v6.mef = !(!(thisUnit->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_MEF));
                break;
            case bcmFieldActionVlanNew:
                rule.ingress_v6.ftidx = 0; 
                rule.ingress_v6.vlan = 0;
                rule.ingress_v6.usevlan = FALSE;
                break;
            case bcmFieldActionRedirectMcast:
                ftIndex = rule.ingress_v6.ftidx;
                /* fallthrough intentional; same fields affected */
            case bcmFieldActionL3Switch:
                /* fallthrough intentional; same fields affected */
            case bcmFieldActionRedirect:
                rule.ingress_v6.vlan = 0; 
                rule.ingress_v6.ftidx = 0;
                rule.ingress_v6.useftidx = FALSE;
                break;
            case bcmFieldActionCosQNew:
                rule.ingress_v6.cos = 0;
                rule.ingress_v6.fcos = 0;
                rule.ingress_v6.usecos = FALSE;
                break;
            case bcmFieldActionDropPrecedence:
                rule.ingress_v6.dp = 0;
                rule.ingress_v6.usedp = FALSE;
                break;
            case bcmFieldActionMirrorIngress:
                if (!mirrorId) {
                    mirrorId = rule.ingress_v6.mirror;
                }
                rule.ingress_v6.mirror = 0;
                break;
            case bcmFieldActionCopyToCpu:
                rule.ingress_v6.copy = FALSE;
                break;
            default:
                /* should never reach this */
                result = BCM_E_INTERNAL;
            } /* switch (action) */
            break;
        case 3:
            switch (action) {
            case bcmFieldActionDrop:
                rule.egress_v6.drop = FALSE;
                break;
            case bcmFieldActionMirrorEgress:
                if (!mirrorId) {
                    mirrorId = rule.egress_v6.mirror;
                }
                rule.egress_v6.mirror = 0;
                break;
            default:
                /* should never reach this */
                result = BCM_E_INTERNAL;
            } /* switch (action) */
            break;
        default:
            /* should never reach this */
            result = BCM_E_INTERNAL;
        } /* switch (rulebase) */
    }
    if (BCM_E_NONE == result) {
        FIELD_EVERB((FIELD_MSG1("remove %s action from unit %d entry %08X\n"),
                     _sbx_fe2000_field_action_name[action],
                     thisUnit->unit,
                     entry));
        /* update the entry rules with the removed action */
        result =  _bcm_fe2k_g2p3_field_rules_write(thisUnit, entry, &rule);
        /* called function would have displayed any error message */
    } else { /* if (BCM_E_NONE == result) */
        switch (result) {
        case BCM_E_NOT_FOUND:
            FIELD_ERR((FIELD_MSG1("unit %d entry %08X does not have any"
                                  " %s action\n"),
                       thisUnit->unit,
                       entry,
                       _sbx_fe2000_field_action_name[action]));
            break;
        default:
            FIELD_ERR((FIELD_MSG1("unable to remove %s action from unit %d"
                                  " entry %08X: %d (%s)\n"),
                       _sbx_fe2000_field_action_name[action],
                       thisUnit->unit,
                       entry,
                       result,
                       _SHR_ERRMSG(result)));
        } /* switch (result) */
    } /* if (BCM_E_NONE == result) */
    if (BCM_E_NONE == result) {
        /* commit any changes that are conditional upon success */
        FIELD_EVERB((FIELD_MSG1("update unit %d entry %08X flags"
                                " %08X -> %08X\n"),
                     thisUnit->unit,
                     entry,
                     thisUnit->entry[entry].entryFlags,
                     newFlags));
        thisUnit->entry[entry].entryFlags = newFlags;
        /* update any cached data */
        switch (action) {
        case bcmFieldActionRedirect:
            thisUnit->entry[entry].ftvlData.target.module = 0;
            thisUnit->entry[entry].ftvlData.target.port = 0;
            break;
        case bcmFieldActionL3Switch:
            thisUnit->entry[entry].ftvlData.ftHandle = 0;
            break;
        case bcmFieldActionVlanNew:
            thisUnit->entry[entry].ftvlData.VSI = 0;
            break;
        case bcmFieldActionRedirectMcast:
            thisUnit->entry[entry].ftvlData.mcHandle = 0;
            break;
        default:
            /* to keep compiler from complaining */
            break;
        }
        /* clean up any resources we need to free now */
        FIELD_EVERB((FIELD_MSG1("release resources used by %s action"
                                " from unit %d entry %08X\n"),
                     _sbx_fe2000_field_action_name[action],
                     thisUnit->unit,
                     entry));
        if ((bcmFieldActionMirrorIngress == action) && mirrorId) {
            _bcm_fe2000_ingr_mirror_free(thisUnit->unit, mirrorId);
        }
        if ((bcmFieldActionMirrorEgress == action) && mirrorId) {
            _bcm_fe2000_egr_mirror_free(thisUnit->unit, mirrorId);
        }
        if ((bcmFieldActionRedirectMcast == action) && ftIndex) {
            _sbx_gu2_resource_free(thisUnit->unit,
                                   SBX_GU2K_USR_RES_FTE_L2MC,
                                   1,
                                   &ftIndex,
                                   0);
        }
    } /* if (BCM_E_NONE == result) */
    /* return the result */
    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_action_remove_all
 *   Purpose
 *      Remove all actions from a specific entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No hardware changes; only software state.
 */
int
bcm_fe2000_g2p3_field_action_remove_all(void *unitData,
                                        bcm_field_entry_t entry)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;  /* working rule */
    unsigned int mirrorId = 0;                  /* working mirror ID */
    uint32 ftIndex = 0;                         /* working FT index */
    int result;                                 /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        FIELD_ERR((FIELD_MSG1("invalid entry %08X specified"
                              " on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    if (!(thisUnit->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_VALID)) {
        /* entry is not in use */
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X is not in use\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    /* get the entry qualifiers and actions so we can update them */
    result = _bcm_fe2k_g2p3_field_rules_read(thisUnit, entry, &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* collect any resources this would implicitly free */
    switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
    case 0:
        if (!mirrorId) {
            mirrorId = rule.ingress.mirror;
        }
        if (thisUnit->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_REDIRMC) {
            if (!ftIndex) {
                ftIndex = rule.ingress.ftidx;
            }
        }
        break;
    case 1:
        if (!mirrorId) {
            mirrorId = rule.egress.mirror;
        }
        break;
    case 2:
        if (!mirrorId) {
            mirrorId = rule.ingress_v6.mirror;
        }
        if (thisUnit->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_REDIRMC) {
            if (!ftIndex) {
                ftIndex = rule.ingress_v6.ftidx;
            }
        }
        break;
    case 3:
        if (!mirrorId) {
            mirrorId = rule.egress_v6.mirror;
        }
        break;
    default:
        /* should never reach this */
        result = BCM_E_INTERNAL;
    } /* switch (rulebase) */
    /* remove the actions from this entry's rules */
    result = _bcm_fe2k_g2p3_field_rules_clear_actions(thisUnit,
                                                      entry,
                                                      &rule);
    if (BCM_E_NONE != result) {
        /* would display error, but called function already did it */
        return result;
    }
    /* update the entry with the new rules */
    FIELD_EVERB((FIELD_MSG1("remove all actions from unit %d entry %08X\n"),
                 thisUnit->unit,
                 entry));
    result = _bcm_fe2k_g2p3_field_rules_write(thisUnit, entry, &rule);
    if (BCM_E_NONE == result) {
        /* commit any changes that are conditional upon success */
        FIELD_EVERB((FIELD_MSG1("update unit %d entry %08X flags"
                                " %08X -> %08X\n"),
                     thisUnit->unit,
                     entry,
                     thisUnit->entry[entry].entryFlags,
                     thisUnit->entry[entry].entryFlags & (~_FE2K_G2P3_ENTRY_ACTIONS)));
        /* make sure no actions are indicated */
        thisUnit->entry[entry].entryFlags &= (~_FE2K_G2P3_ENTRY_ACTIONS);
        thisUnit->entry[entry].counterMode = 0;
        sal_memset(&(thisUnit->entry[entry].ftvlData),
                   0,
                   sizeof(thisUnit->entry[entry].ftvlData));
        /* clean up any resources we need to free now */
        FIELD_EVERB((FIELD_MSG1("release resources used by actions"
                                " from unit %d entry %08X\n"),
                     thisUnit->unit,
                     entry));
        if (mirrorId) {
            switch (thisUnit->group[thisUnit->entry[entry].group].rulebase) {
            case 0:
            case 2:
                result = _bcm_fe2000_ingr_mirror_free(thisUnit->unit, mirrorId);
                break;
            case 1:
            case 3:
                result = _bcm_fe2000_egr_mirror_free(thisUnit->unit, mirrorId);
                break;
            default:
                /* should never reach this */
                result = BCM_E_INTERNAL;
            } /* switch (rulebase) */
        } /* if (mirrorId) */
        if (ftIndex) {
            result = _sbx_gu2_resource_free(thisUnit->unit,
                                            SBX_GU2K_USR_RES_FTE_L2MC,
                                            1,
                                            &ftIndex,
                                            0);
        }
    } /* if (BCM_E_NONE == result) */
    /* return the result */
    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_counter_create
 *   Purpose
 *      'Create' a counter for the specified entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This really doesn't do anything on the SBX hardware, which maintains
 *      counters for all rules.  What it does is update the software state to
 *      indicate that this particular entry has a counter.
 */
int
bcm_fe2000_g2p3_field_counter_create(void *unitData,
                                     bcm_field_entry_t entry)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;    /* working entry data */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        FIELD_ERR((FIELD_MSG1("invalid entry %08X specified"
                              " on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    thisEntry = &(thisUnit->entry[entry]);
    if (!(thisEntry->entryFlags & _FE2K_G2P3_ENTRY_VALID)) {
        /* entry is not in use */
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X is not in use\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    if (thisEntry->entryFlags & _FE2K_G2P3_ENTRY_COUNTER) {
        /* entry already has a counter */
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X already has a counter\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_EXISTS;
    }
    FIELD_EVERB((FIELD_MSG1("enabling counter on unit %d entry %08X\n"),
                 thisUnit->unit,
                 entry));
    thisEntry->entryFlags |= _FE2K_G2P3_ENTRY_COUNTER;
    thisEntry->entryFlags &= (~_FE2K_G2P3_ENTRY_CTR_ENA);
    thisEntry->counterMode = 0;
    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_counter_destroy
 *   Purpose
 *      Remove the entry's counter
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      If an entry is not sharing a counter, this merely turns off the counter
 *      for that entry.  If the entry is sharing a counter, it removes that
 *      entry from the sharing list and then disables the counter for that
 *      entry (so the end result is the specified entry has no counter but any
 *      other entries that shared with it are left alone).
 */
int
bcm_fe2000_g2p3_field_counter_destroy(void *unitData,
                                      bcm_field_entry_t entry)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;    /* working entry data */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        FIELD_ERR((FIELD_MSG1("invalid entry %08X specified"
                              " on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    thisEntry = &(thisUnit->entry[entry]);
    if (!(thisEntry->entryFlags & _FE2K_G2P3_ENTRY_VALID)) {
        /* entry is not in use */
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X is not in use\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    if (!(thisEntry->entryFlags & _FE2K_G2P3_ENTRY_COUNTER)) {
        /* entry already has a counter */
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X has no counter\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_EMPTY;
    }
    FIELD_EVERB((FIELD_MSG1("disabling counter on unit %d entry %08X\n"),
                 thisUnit->unit,
                 entry));
    /* adjust group potential counter count */
    thisUnit->group[thisEntry->group].counters++;
    thisEntry->entryFlags &= (~(_FE2K_G2P3_ENTRY_COUNTER |
                                _FE2K_G2P3_ENTRY_CTR_ENA));
    thisEntry->counterMode = 0;
    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_counter_set
 *   Purpose
 *      Set the specified counter to a value
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int counter_num = which counter (perhaps frame or byte?)
 *      (in) uint64 val = new value for counter
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The only supported value is zero.
 *      We ignore counter_num because we don't have a way to only clear one
 *      side or the other.
 */
int
bcm_fe2000_g2p3_field_counter_set(void *unitData,
                                  bcm_field_entry_t entry,
                                  int counter_num,
                                  uint64 val)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    soc_sbx_g2p3_counter_value_t tempCounts;    /* working counters */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        FIELD_ERR((FIELD_MSG1("invalid entry %08X specified"
                              " on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    if (!(thisUnit->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_VALID)) {
        /* entry is not in use */
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X is not in use\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    if ((0 > counter_num) || (1 < counter_num)) {
        /* counter number is bogus */
        FIELD_ERR((FIELD_MSG1("invalid counter number %d\n"),
                   counter_num));
        return BCM_E_PARAM;
    }
    if (val) {
        /* can't do any writes but zero */
        FIELD_ERR((FIELD_MSG1("unit %d does not support arbitrary counter"
                              " writes -- only reading or clearing\n"),
                   thisUnit->unit));
        return BCM_E_PARAM;
    }

    return _bcm_fe2k_g2p3_field_entries_counter_access(thisUnit,
                                                       entry,
                                                       TRUE,
                                                       &tempCounts);
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_counter_get
 *   Purpose
 *      Set the specified counter to a value
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int counter_num = which counter (perhaps frame or byte?)
 *      (in) uint64 *val = new value for counter
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The only supported value is zero.
 *      We ignore counter_num because we don't have a way to only clear one
 *      side or the other.
 */
int
bcm_fe2000_g2p3_field_counter_get(void *unitData,
                                  bcm_field_entry_t entry,
                                  int counter_num,
                                  uint64 *val)
{
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;      /* working unit data */
    soc_sbx_g2p3_counter_value_t tempCounts;    /* working counters */
    int result;                                 /* working result */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        FIELD_ERR((FIELD_MSG1("invalid entry %08X specified"
                              " on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    if (!(thisUnit->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_VALID)) {
        /* entry is not in use */
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X is not in use\n"),
                   thisUnit->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }

    result = _bcm_fe2k_g2p3_field_entries_counter_access(thisUnit,
                                                         entry,
                                                         FALSE,
                                                         &tempCounts);
    /* return appropriate count if no errors */
    if (BCM_E_NONE == result) {
        if (counter_num) {
            /* caller wants 'upper' counter */
            *val = tempCounts.bytes;
        } else {
            /* caller wants 'lower' counter */
            *val = tempCounts.packets;
        }
    }
    return result;
}

/*
 *   Function
 *      _bcm_fe2000_g2p3_field_policer_manipulate
 *   Purpose
 *      Manipulate (get,set,clear) the policer on a given entry
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in/out) bcm_policer_t *policer_id = pointer to the policer ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No hardware changes; only software state.
 */
static int
_bcm_fe2000_g2p3_field_policer_manipulate(_bcm_fe2k_g2p3_field_glob_t *glob,
                                          const bcm_field_entry_t entry,
                                          bcm_policer_t *policer_id,
                                          int action)
{
    _bcm_fe2k_g2p3_field_entry_t *thisEntry;    /* working entry data */
    _bcm_fe2k_g2p3_hardware_rule_t rule;        /* working rule */
    bcm_policer_t policer = 0;                  /* working policer ID */
    bcm_policer_group_mode_t polMode;           /* working policer mode */
    int result = BCM_E_NONE;                    /* working result */
    int typedPolice = FALSE;                    /* working policer mode flag */
    int mef = FALSE;                            /* working policer mode flag */
    int mefCos = FALSE;                         /* working policer mode flag */

    /* check entry validity */
    if ((glob->entryTotal <= entry) || (0 > entry)) {
        /* entry ID is invalid */
        FIELD_ERR((FIELD_MSG1("invalid entry %08X specified"
                              " on unit %d\n"),
                   entry,
                   glob->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
    thisEntry = &(glob->entry[entry]);
    if (!(thisEntry->entryFlags & _FE2K_G2P3_ENTRY_VALID)) {
        /* entry is not in use */
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X is not in use\n"),
                   glob->unit,
                   entry));
        return BCM_E_NOT_FOUND;
    }
    /* check for and execute read if appropriate */
    if (_SBX_FE2K_FIELD_POLICER_READ == action) {
        /* reading; just get it from the cache (h/w read has special cases) */
        if (thisEntry->policer) {
            /* entry has a policer */
            *policer_id = thisEntry->policer;
            return BCM_E_NONE;
        } else {
            /* entry has no policer */
            
            return BCM_E_NOT_FOUND;
        }
    }
    /* prep for write if appropriate */
    if (_SBX_FE2K_FIELD_POLICER_WRITE == action) {
        if (thisEntry->policer) {
            /* entry already has associated policer; must dissociate first */
            FIELD_ERR((FIELD_MSG1("unit %d entry %08X already has policer"
                                  " %08X; dissociate before replacing\n"),
                       glob->unit,
                       entry,
                       thisEntry->policer));
            return BCM_E_EXISTS;
        }
        /* get information about this policer */
        policer = *policer_id;
        result = _bcm_fe2000_policer_group_mode_get(glob->unit,
                                                    policer,
                                                    &polMode);
        if (BCM_E_NONE != result) {
            /* error accessing the policer; assume it isn't valid */
            FIELD_ERR((FIELD_MSG1("unable to access unit %d policer %08X:"
                                  " %d (%s)\n"),
                       glob->unit,
                       policer,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
        switch (polMode) {
        case bcmPolicerGroupModeSingle:
            break;
        case bcmPolicerGroupModeTyped:
            typedPolice = TRUE;
            break;
        case bcmPolicerGroupModeTypedIntPri:
            mefCos = TRUE;
            /* fallthrough intentional */
        case bcmPolicerGroupModeTypedAll:
            mef = TRUE;
            break;
        default:
            FIELD_ERR((FIELD_MSG1("unexpected policer mode %d on unit %d"
                                  " policer %08X\n"),
                       polMode,
                       glob->unit,
                       policer));
            return BCM_E_CONFIG;
        } /* switch (mode) */
    } /* if (_SBX_FE2K_FIELD_POLICER_WRITE == action) */
    /* prep for clear if appropriate */
    if (_SBX_FE2K_FIELD_POLICER_CLEAR == action) {
        if (!thisEntry->policer) {
            /* entry has no associated policer; can't dissociate nothing */
            FIELD_ERR((FIELD_MSG1("unit %d entry %08X has no policer\n"),
                       glob->unit,
                       entry));
            return BCM_E_NOT_FOUND;
        }
    } /* if (_SBX_FE2K_FIELD_POLICER_CLEAR == action) */
    /* clear or set the policer as appropriate */
    if (!(thisEntry->entryFlags & _FE2K_G2P3_ENTRY_DROP)) {
        /* we're not dropping, so can modify hardware */
        /* read current entry rules */
        result = _bcm_fe2k_g2p3_field_rules_read(glob, entry, &rule);
        if (BCM_E_NONE != result) {
            /* would display error, but called function already did it */
            return result;
        }
        /* update entry rule */
        switch (glob->group[glob->entry[entry].group].rulebase) {
        case 0:
            /* ingress */
            rule.ingress.policer = policer;
            rule.ingress.typedpolice = typedPolice;
            rule.ingress.mef = mef;
            rule.ingress.mefcos = mefCos;
            break;
        case 1:
            /* egress */
            FIELD_ERR((FIELD_MSG1("policing not supported on egress rules"
                                  " for unit %d\n"),
                       glob->unit));
            return BCM_E_UNAVAIL;
            break;
        case 2:
            /* ingress v6 */
            rule.ingress_v6.policer = policer;
            rule.ingress_v6.typedpolice = typedPolice;
            rule.ingress_v6.mef = mef;
            rule.ingress_v6.mefcos = mefCos;
            break;
        case 3:
            /* egress v6 */
            FIELD_ERR((FIELD_MSG1("policing not supported on egress v6 rules"
                                  " for unit %d\n"),
                       glob->unit));
            return BCM_E_UNAVAIL;
            break;
        default:
            /* should never reach this */
            return BCM_E_INTERNAL;
        } /* switch (rulebase) */
        /* update the entry rules */
        result = _bcm_fe2k_g2p3_field_rules_write(glob, entry, &rule);
        if (BCM_E_NONE != result) {
            /* would display error, but called function already did it */
            return result;
        }
    } /* if (!(thisEntry->entryFlags & _FE2K_G2P3_ENTRY_DROP)) */
    /* update the entry information to reflect the policer */
    thisEntry->policer = policer;
    if (mef) {
        thisEntry->entryFlags |= _FE2K_G2P3_ENTRY_MEF;
    } else {
        thisEntry->entryFlags &= (~_FE2K_G2P3_ENTRY_MEF);
    }
    if (mefCos) {
        thisEntry->entryFlags |= _FE2K_G2P3_ENTRY_MEFCOS;
    } else {
        thisEntry->entryFlags &= (~_FE2K_G2P3_ENTRY_MEFCOS);
    }
    if (typedPolice) {
        thisEntry->entryFlags |= _FE2K_G2P3_ENTRY_TYPEPOL;
    } else {
        thisEntry->entryFlags &= (~_FE2K_G2P3_ENTRY_TYPEPOL);
    }
    /* return the result */
    return result;
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_entry_policer_attach
 *   Purpose
 *      Attach a policer to a specified entry, at the given heirarchical level
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int level = level (for heirarchical policing)
 *      (in) bcm_policer_t policer = which policer to attach to the entry
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Does not allow direct replace; must dissociate first.
 *      Assumes policer 0 = no policer and does not allow it to be set.
 *      SBX does not support heirarchical policing, so level != 0 is error.
 *      Policers must be managed by caller.
 */
int
bcm_fe2000_g2p3_field_entry_policer_attach(void *unitData,
                                           bcm_field_entry_t entry_id,
                                           int level,
                                           bcm_policer_t policer_id)
{

    /* we don't support heirarchical policing */
    if (level) {
        /* nonzero level is bad parameter */
        FIELD_ERR((FIELD_MSG1("no support for heirarchical policing\n")));
        return BCM_E_PARAM;
    }
    /* we don't allow policer zero */
    if (!policer_id) {
        /* policer zero isn't valid */
        FIELD_ERR((FIELD_MSG1("policer %08X is not valid\n"), policer_id));
        return BCM_E_PARAM;
    }
    /* okay, so now set the policer */
    return _bcm_fe2000_g2p3_field_policer_manipulate((_bcm_fe2k_g2p3_field_glob_t*)unitData,
                                                     entry_id,
                                                     &policer_id,
                                                     _SBX_FE2K_FIELD_POLICER_WRITE);
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_entry_policer_detach
 *   Purpose
 *      Detach a policer to a specified entry, at the given heirarchical level
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int level = level (for heirarchical policing)
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      SBX does not support heirarchical policing, so level != 0 is error.
 *      Policers must be managed by caller.
 */
int
bcm_fe2000_g2p3_field_entry_policer_detach(void *unitData,
                                           bcm_field_entry_t entry_id,
                                           int level)
{
    bcm_policer_t policer;

    /* we don't support heirarchical policing */
    if (level) {
        /* nonzero level is bad parameter */
        FIELD_ERR((FIELD_MSG1("no support for heirarchical policing\n")));
        return BCM_E_PARAM;
    }
    /* okay, so now clear the policer */
    return _bcm_fe2000_g2p3_field_policer_manipulate((_bcm_fe2k_g2p3_field_glob_t*)unitData,
                                                     entry_id,
                                                     &policer,
                                                     _SBX_FE2K_FIELD_POLICER_CLEAR);
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_entry_policer_detach_all
 *   Purpose
 *      Attach a policer to a specified entry, at the given heirarchical level
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int level = level (for heirarchical policing)
 *      (in) bcm_policer_t policer = which policer to attach to the entry
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_g2p3_field_entry_policer_detach_all(void *unitData,
                                               bcm_field_entry_t entry_id)
{
    /* we don't support heirarchical, so just detach level 0  */
    return bcm_fe2000_g2p3_field_entry_policer_detach(unitData, entry_id, 0);
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_entry_policer_get
 *   Purpose
 *      Attach a policer to a specified entry, at the given heirarchical level
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int level = level (for heirarchical policing)
 *      (in) bcm_policer_t *policer_id = where to put current policer
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Does not allow direct replace; must dissociate first.
 *      Assumes policer 0 = no policer and does not allow it to be set.
 *      SBX does not support heirarchical policing, so level != 0 is error.
 *      Policers must be managed by caller.
 */
int
bcm_fe2000_g2p3_field_entry_policer_get(void *unitData,
                                           bcm_field_entry_t entry_id,
                                           int level,
                                           bcm_policer_t *policer_id)
{
    /* we don't support heirarchical policing */
    if (level) {
        /* nonzero level is bad parameter */
        FIELD_ERR((FIELD_MSG1("no support for heirarchical policing\n")));
        return BCM_E_PARAM;
    }
    /* okay, so now set the policer */
    return _bcm_fe2000_g2p3_field_policer_manipulate((_bcm_fe2k_g2p3_field_glob_t*)unitData,
                                                     entry_id,
                                                     policer_id,
                                                     _SBX_FE2K_FIELD_POLICER_READ);
}


/*
 *   Function
 *      bcm_fe2000_g2p3_field_show
 *   Purpose
 *      Dump all field information for the unit
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_g2p3_field_show(void *unitData,
                           const char *pfx)
{
#ifdef BROADCOM_DEBUG
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    return _bcm_fe2k_g2p3_field_dump(thisUnit, pfx);
#else /* BROADCOM_DEBUG */
    return BCM_E_UNAVAIL;
#endif /* def BROADCOM_DEBUG */
}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_entry_dump
 *   Purpose
 *      Dump information about the specified entry to debug output
 *   Parameters
 *      (in) _bcm_fe2k_g2p2_field_glob_t *thisUnit = pointer to unit info
 *      (in) _field_entry_index entry = the entry ID
 *      (in) char *prefix = a string to display in front of each line
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No error checking or locking is done here.  Under most cases, this will
 *      return BCM_E_NONE unconditionally.  The only time when it would not is
 *      if it is unable to fetch the rule data from the SBX driver.
 */
int
bcm_fe2000_g2p3_field_entry_dump(void *unitData,
                                 bcm_field_entry_t entry)
{
#ifdef BROADCOM_DEBUG
    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    if ((thisUnit->entryTotal <= entry) || (0 > entry)) {
        FIELD_ERR((FIELD_MSG1("can't dump invalid entry ID %08X"
                              " on unit %d\n"),
                   entry,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
#if !_SBX_FE2K_FIELD_DIAGNOSTIC_MODE
    /* only verify in-use if *not* in diagnostic mode */
    if (!(thisUnit->entry[entry].entryFlags & _FE2K_G2P3_ENTRY_VALID)) {
        FIELD_ERR((FIELD_MSG1("unit %d entry %08X is not in use\n"),
                   thisUnit->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }
#endif /* !_SBX_FE2K_FIELD_DIAGNOSTIC_MODE */
    return _bcm_fe2k_g2p3_field_entry_dump(unitData, entry, "");
#else /* BROADCOM_DEBUG */
    return BCM_E_UNAVAIL;
#endif /* def BROADCOM_DEBUG */

}

/*
 *   Function
 *      bcm_fe2000_g2p3_field_group_dump
 *   Purpose
 *      Dump information about the specified group to debug output
 *   Parameters
 *      (in) void* unitData = pointer to the unit's private data
 *      (in) bcm_field_group_t group = the group ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_g2p3_field_group_dump(void *unitData,
                                 bcm_field_group_t group)
{
#ifdef BROADCOM_DEBUG

    _bcm_fe2k_g2p3_field_glob_t *thisUnit;    /* working unit data */

    thisUnit = (_bcm_fe2k_g2p3_field_glob_t*)unitData;
    if ((thisUnit->groupTotal <= group) || (0 > group)) {
        FIELD_ERR((FIELD_MSG1("can't dump invalid group ID %08X"
                              " on unit %d\n"),
                   group,
                   thisUnit->unit));
        return _SBX_FE2K_FIELD_INVALID_ID_ERR;
    }
#if !_SBX_FE2K_FIELD_DIAGNOSTIC_MODE
    /* only verify in-use if *not* in diagnostic mode */
    if (thisUnit->uMaxSupportedStages <= thisUnit->group[group].rulebase) {
        FIELD_ERR((FIELD_MSG1("unit %d group %08X is not in use\n"),
                   thisUnit->unit,
                   group));
        return BCM_E_NOT_FOUND;
    }
#endif /* !_SBX_FE2K_FIELD_DIAGNOSTIC_MODE */
    return _bcm_fe2k_g2p3_field_group_dump(unitData, group, "");
#else /* BROADCOM_DEBUG */
    return BCM_E_UNAVAIL;
#endif /* def BROADCOM_DEBUG */

}


#endif /* def BCM_FE2000_P3_SUPPORT */
