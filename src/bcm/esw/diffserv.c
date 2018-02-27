/*
 * $Id: diffserv.c 1.66.6.1 Broadcom SDK $
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
 * File:        diffserv.c
 * Purpose:     API for DiffServ applications.
 *              Ties together the FFP/FP, meter and counter functionality.
 */


#include <shared/alloc.h>
#include <sal/core/libc.h>
#include <sal/core/sync.h>

#include <shared/bitop.h>

#include <soc/debug.h>
#include <soc/drv.h>

#include <bcm/error.h>
#include <bcm/diffserv.h>

#include <bcm_int/esw/field.h> /* _bcm_field_setup_post_ethertype_udf() */
#include <bcm_int/esw/diffserv.h>

#include <bcm_int/esw_dispatch.h>

#if defined(BCM_FILTER_SUPPORT) || defined(BCM_FIELD_SUPPORT)

#ifdef BCM_WARM_BOOT_SUPPORT
#include <bcm_int/esw/switch.h>
#endif /* BCM_WARM_BOOT_SUPPORT */

#ifndef INCLUDE_L3
#define bcm_ip_mask_create(_len)                 \
        ((_len) ? (~((1 << (32 - (_len))) - 1)) : 0)
#endif /* !INCLUDE_L3 */

typedef struct _clfr_s {
    int cfe_dpid;
    int cfe_cfid;
    int cfe_installed;          /* classifier has been installed */
#ifdef BCM_FILTER_SUPPORT
    bcm_filterid_t f;           /* filter ID, used if chip has FFP */
    int cfe_l4port_entries;     /* l4port (ie: IPv4) port ranges */
    uint16 *cfe_l4port_val;
    uint16 *cfe_l4port_mask;
    int f_nextra;               /* copies of f, used for l4port ranges */
    bcm_filterid_t *f_extra;    /* range checkers used instead for xgs3 */
    int cfe_meter_id;           /* not used for xgs3 */
    int cfe_ffpcntr_id;         /* not used for xgs3 */
#endif
#ifdef BCM_FIELD_SUPPORT
    bcm_field_qset_t qs;        /* qset, used for xgs3 with Field Proc. */
    bcm_field_group_t g;        /* group ID, used for xgs3 with Field Proc. */
    bcm_field_entry_t e;        /* entry ID, used for xgs3 with Field Proc. */
    bcm_field_range_t cfe_l4port_src_range;   /* xgs3 src port range */
    bcm_field_range_t cfe_l4port_dst_range;   /* xgs3 dst port range */
    bcm_field_qset_t qs_nm;     /* qset, used for xgs3 "no-match" action */
    bcm_field_group_t g_nm;     /* group ID, used for xgs3 "no-match" action */
    bcm_field_entry_t e_nm;     /* entry ID, used for xgs3 "no-match" action */
#endif
    int cfe_precedence;         /* copy from cf_precedence field of cfe_clfr */
    int cfe_ds_port;
    int cfe_l4port_flags;
    bcm_ds_clfr_t cfe_clfr;
    bcm_ds_inprofile_actn_t cfe_inp_actn;
    bcm_ds_outprofile_actn_t cfe_outp_actn;
    bcm_ds_nomatch_actn_t cfe_nm_actn;
    struct _clfr_s *cfe_next;
    struct _clfr_s *cfe_prev;
} _clfr_t;

typedef struct _dp_s {
    bcm_pbmp_t  dp_ports;
    uint32      dp_flags;
    int         dp_installed;
    _clfr_t     *dp_clfr;
    bcm_ds_scheduler_t dp_scheduler;
    int         dp_next_cfid;
    struct _dp_s *next;         /* Linked list per bucket */
    int         dpid;           /* Data path ID associated with this entry */
} _dp_t;

#define INVALID_FFPCOUNTER_ID   -1
#define INVALID_METER_ID         0

/*
 * Data path IDs are hashed to a bucket rather than
 * indexed out a fixed array.  Each bucket is a linked list.
 * This is the number of buckets.
 */

#define DPID_BUCKETS            256

/*
 * CFID_MAX_COUNT and DPID_MAX_COUNT are provided so that when looking
 * for a free ID, we have some limit.  Otherwise, these counts are
 * not reference.
 */
#define DPID_MAX_COUNT          (8 * 1024)
#define CFID_MAX_COUNT          (64 * 1024)
#define DS_MIB_SIZE             256

#define DPID_CHECK_E_BADID(dpid) do { \
    if (((dpid) < 0 ) || ((dpid) >= DPID_MAX_COUNT)) { return (BCM_E_BADID); } } while(0)
#define DPID_CHECK_E_NOT_FOUND(dpid) do { \
    if (((dpid) < 0 ) || ((dpid) >= DPID_MAX_COUNT)) { \
        return (BCM_E_NOT_FOUND); \
    } } while (0)

typedef struct _ds_binary_s {
    void       *ds_mibid;
    int         ds_dpid;
    int         ds_cfid;
} _ds_binary_t;

typedef struct _bcm_ds_s {
    _dp_t               *ds_dp_ptr[DPID_BUCKETS];  /* Bucket head pointer */
    sal_mutex_t         ds_lock;
    _ds_binary_t        *ds_mib;
    int                 ds_mib_count;
    int                 ds_mib_max;
#ifdef BCM_FIELD_SUPPORT
    bcm_field_udf_t     udf_id;     /* UDF used to qualify the 4 bytes */
                                    /* immediately following EtherType */
#endif /* BCM_FIELD_SUPPORT */
} _bcm_ds_t;

static _bcm_ds_t *ds_info[BCM_MAX_NUM_UNITS];
static char ds_string[] = "DIFFSERV";

#define DPID_HASH(_dpid)        (_dpid % DPID_BUCKETS)

#ifdef BCM_WARM_BOOT_SUPPORT
#define DS_SCACHE_SIZE    200    /* Bytes */

static uint32 _ds_scache_size = 0;

STATIC int _bcm_ds_reinit(int unit);

#define BCM_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define BCM_WB_DEFAULT_VERSION            BCM_WB_VERSION_1_0

#endif /* BCM_WARM_BOOT_SUPPORT */

/*
 * Function:
 *      _bcm_dpid_find
 * Purpose:
 *      Find a given DPID
 * Returns:
 *      Pointer to old _dp_t object, or NULL if not found
 * Notes:
 *      Assumes DS_INIT_CHECK and lock held.
 */

STATIC INLINE _dp_t *
_bcm_dpid_find(int unit, int dpid)
{
    _dp_t *dp;

    dp = ds_info[unit]->ds_dp_ptr[DPID_HASH(dpid)];

    while ((dp != NULL) && (dp->dpid != dpid)) {
        dp = dp->next;
    }

    return dp;  /* NULL if not found */
}

/*
 * Function:
 *      _dpid_add
 * Purpose:
 *      Add a DPID entry.
 * Returns:
 *      Pointer to new _dp_t object, or NULL if allocation fails
 * Notes:
 *      Assumes DPID doesn't already exist and that the lock is held
 *      for the unit.  Adds to beginning of bucket's list.
 */

STATIC INLINE _dp_t *
_dpid_add(int unit, int dpid)
{
    int bucket;
    _dp_t *dp;

    dp = sal_alloc(sizeof(_dp_t), "ds_dp");
    if (dp == NULL) {
        return NULL;
    }
    sal_memset(dp, 0, sizeof(_dp_t));

    bucket                           = DPID_HASH(dpid);
    dp->dpid                         = dpid;
    dp->dp_flags                     = BCM_DS_INGRESS;
    dp->next                         = ds_info[unit]->ds_dp_ptr[bucket];
    ds_info[unit]->ds_dp_ptr[bucket] = dp;

    return dp;
}

/*
 * Function:
 *      _dpid_unlink
 * Purpose:
 *      Unlink a DPID entry.
 * Returns:
 *      Pointer to the entry that was unlinked, or NULL if not found
 * Notes:
 *      Assumes the lock is held for the unit.
 *      After the entry is unlinked, the caller may reuse or free it.
 */

STATIC INLINE _dp_t *
_dpid_unlink(int unit, int dpid)
{
    int bucket;
    _dp_t *dp, *found_dp;

    bucket = DPID_HASH(dpid);

    dp = ds_info[unit]->ds_dp_ptr[bucket];
    if (dp == NULL) {
        return NULL;
    }

    if (dp->dpid == dpid) {   /* First entry on list */
        ds_info[unit]->ds_dp_ptr[bucket] = dp->next;
        return dp;
    }

    while (dp->next != NULL && dp->next->dpid != dpid) {
        dp = dp->next;
    }

    if (dp->next == NULL) {  /* Not found */
        return NULL;
    }

    /* Next is non-null, so dp->next is the entry to unlink */
    found_dp = dp->next;
    dp->next = dp->next->next;   /* Unlink dp->next. */

    return found_dp;
}

/*
 * Function:
 *      _ds_init_check
 * Purpose:
 *      Check that the diffserv module has been initialized.
 * Returns:
 *      BCM_E_NONE if initialized
 *      BCM_E_INIT if not initialized
 *      BCM_E_UNAVAIL if diffserv is not supported
 * Notes:
 *      The BCMX diffserv functions rely on the BCM diffserv
 *      functions to return BCM_E_UNAVAIL (and not BCM_E_INIT)
 *      on devices that do not support filter/field processing.
 */

STATIC int
_ds_init_check(int unit)
{
    if (SOC_IS_XGS_FABRIC(unit)) {
        return BCM_E_UNAVAIL;
    }
    if (!soc_feature(unit, soc_feature_filter_metering) &&
        !soc_feature(unit, soc_feature_field)) {
        return BCM_E_UNAVAIL;
    }
    if (ds_info[unit] == NULL) {
        return (BCM_E_INIT);
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _ds_binary_init
 * Purpose:
 *      Initialize binary search array structure
 */

#define DS_INIT_CHECK(unit) \
        BCM_IF_ERROR_RETURN(_ds_init_check(unit))
#define DS_LOCK(unit)     \
        sal_mutex_take(ds_info[unit]->ds_lock, sal_mutex_FOREVER)
#define DS_UNLOCK(unit)   sal_mutex_give(ds_info[unit]->ds_lock)

static int
_ds_binary_init(int unit)
{
    int         i, nbytes;

    nbytes = sizeof(_ds_binary_t) * DS_MIB_SIZE;
    ds_info[unit]->ds_mib = sal_alloc(nbytes, "ds_mib");
    if (ds_info[unit]->ds_mib == NULL) {
        return BCM_E_MEMORY;
    }

    sal_memset(ds_info[unit]->ds_mib, 0, nbytes);
    ds_info[unit]->ds_mib_count = 0;
    ds_info[unit]->ds_mib_max = DS_MIB_SIZE;

    ds_info[unit]->ds_mib[0].ds_mibid = 0;
    ds_info[unit]->ds_mib[0].ds_dpid = 0;
    ds_info[unit]->ds_mib[0].ds_cfid = 0;
    for (i = 1; i < ds_info[unit]->ds_mib_max; i++) {
        ds_info[unit]->ds_mib[i].ds_mibid = (void *)-1;
        ds_info[unit]->ds_mib[i].ds_dpid = -1;
        ds_info[unit]->ds_mib[i].ds_cfid = -1;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _ds_binary_search
 * Purpose:
 *      Search binary search array structure
 */

static int
_ds_binary_search(int unit, _ds_binary_t *entry, int *index)
{
    int low, mid, high, rv;

    if ((entry->ds_mibid == 0) && (entry->ds_dpid == 0)) {
        return BCM_E_PARAM;
    }

    if (ds_info[unit]->ds_mib == NULL) {
        rv = _ds_binary_init(unit);
        if (rv < 0) {
            return rv;
        }
    }

    low = 1;
    high = ds_info[unit]->ds_mib_count;

    while (low <= high) {
        mid = (low + high) / 2;
        rv = sal_memcmp(&ds_info[unit]->ds_mib[mid], entry,
                        sizeof(void *) + sizeof(int));
        if (rv == 0) {
            entry->ds_cfid = ds_info[unit]->ds_mib[mid].ds_cfid;
            return mid;
        } else if (rv < 0)
            low = mid + 1;
        else {
            high = mid - 1;
        }
    }

    if (index != NULL) {
        *index = low;
    }
    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      _ds_binary_insert
 * Purpose:
 *      Insert into binary search array structure
 */

static int
_ds_binary_insert(int unit, _ds_binary_t *entry)
{
    int rv, i, index, ins_index;
    _ds_binary_t ent;

    if ((entry->ds_mibid == 0) && (entry->ds_dpid == 0)) {
        return BCM_E_PARAM;
    }

    if (ds_info[unit]->ds_mib == NULL) {
        rv = _ds_binary_init(unit);
        if (rv < 0) {
            return rv;
        }
    }

    if (ds_info[unit]->ds_mib_count == ds_info[unit]->ds_mib_max) {
        return BCM_E_FULL;
    }

    ent.ds_mibid = entry->ds_mibid;
    ent.ds_dpid = entry->ds_dpid;
    index = _ds_binary_search(unit, &ent, &ins_index);
    if (index >= 0) {
        return index;
    }


    i = ds_info[unit]->ds_mib_count + 1;        /* first unused entry */
    while (i > ins_index) {
        /* Move up one entry */
        ds_info[unit]->ds_mib[i].ds_mibid =
            ds_info[unit]->ds_mib[i-1].ds_mibid;
        ds_info[unit]->ds_mib[i].ds_dpid =
            ds_info[unit]->ds_mib[i-1].ds_dpid;
        ds_info[unit]->ds_mib[i].ds_cfid =
            ds_info[unit]->ds_mib[i-1].ds_cfid;
        i--;
    }

    /* Write entry data at insertion point */

    ds_info[unit]->ds_mib[ins_index].ds_mibid = entry->ds_mibid;
    ds_info[unit]->ds_mib[ins_index].ds_dpid = entry->ds_dpid;
    ds_info[unit]->ds_mib[ins_index].ds_cfid = entry->ds_cfid;
    ds_info[unit]->ds_mib_count++;

    return ins_index;
}

/*
 * Function:
 *      _ds_binary_delete
 * Purpose:
 *      Remove from binary search array structure
 */

static int
_ds_binary_delete(int unit, _ds_binary_t *entry)
{
    int i, index;
    int count;

    index = _ds_binary_search(unit, entry, NULL);
    if (index < 0) {
        return index;
    }

    count = ds_info[unit]->ds_mib_count;
    for (i = index; i < count; i++) {
        ds_info[unit]->ds_mib[i].ds_mibid =
            ds_info[unit]->ds_mib[i+1].ds_mibid;
        ds_info[unit]->ds_mib[i].ds_dpid =
            ds_info[unit]->ds_mib[i+1].ds_dpid;
        ds_info[unit]->ds_mib[i].ds_cfid =
            ds_info[unit]->ds_mib[i+1].ds_cfid;
    }

    ds_info[unit]->ds_mib[count].ds_mibid = (void *)-1;
    ds_info[unit]->ds_mib[count].ds_dpid = -1;
    ds_info[unit]->ds_mib[count].ds_cfid = -1;
    ds_info[unit]->ds_mib_count--;

    return BCM_E_NONE;
}

#ifdef BCM_FILTER_SUPPORT

/*
 * Function:
 *      _ds_clfr_in_profile_action
 * Description:
 *      Set the in profile action.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      cf_ptr - Pointer to classifier.
 *      f - Pointer to filter template to operate on.
 * Returns:
 *      BCM_E_XXX
 */

static int
_ds_clfr_in_profile_action(int unit, _clfr_t *cf_ptr, bcm_filterid_t f)
{
    bcm_ds_inprofile_actn_t *inp_actn;
    uint32 flags;

    soc_cm_debug(DK_VERBOSE,
                 "_ds_clfr_in_profile_action: dpid=%d cfid=%d\n",
                 cf_ptr->cfe_dpid, cf_ptr->cfe_cfid);

    inp_actn = &cf_ptr->cfe_inp_actn;
    flags = inp_actn->ipa_flags;

    if (flags & BCM_DS_ACTN_INSERT_PRIO) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_match(unit, f,
                                         bcmActionInsPrio,
                                         inp_actn->ipa_pri));
    }

    if (flags & BCM_DS_ACTN_SET_PRIO) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_match(unit, f,
                                         bcmActionSetPrio,
                                         inp_actn->ipa_pri));
    }

    if (flags & BCM_DS_ACTN_INSERT_TOSP) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_match(unit, f,
                                         bcmActionInsTOSP,
                                         inp_actn->ipa_tos));
    }

    if (flags & BCM_DS_ACTN_COPY_TO_CPU) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_match(unit, f,
                                         bcmActionCopyToCpu, 0));
    }

    if (flags & BCM_DS_ACTN_DO_NOT_SWITCH) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_match(unit, f,
                                         bcmActionDoNotSwitch, 0));
    }

    if (flags & BCM_DS_ACTN_COPY_TO_MIRROR) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_match(unit, f,
                                         bcmActionCopyToMirror, 0));
    }

    if (flags & BCM_DS_ACTN_INCR_FFPPKT_COUNTER) {
        if (cf_ptr->cfe_ffpcntr_id == INVALID_FFPCOUNTER_ID) {
            BCM_IF_ERROR_RETURN
                (bcm_esw_ffpcounter_create(unit,
                                           cf_ptr->cfe_ds_port,
                                           &cf_ptr->cfe_ffpcntr_id));
        }
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_match(unit, f,
                                         bcmActionIncrCounter,
                                         cf_ptr->cfe_ffpcntr_id));
    }

    if (flags & BCM_DS_ACTN_INSERT_PRIO_FROM_TOSP) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_match(unit, f,
                                         bcmActionInsPrioFromTOSP, 0));
    }

    if (flags & BCM_DS_ACTN_INSERT_TOSP_FROM_PRIO) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_match(unit, f,
                                         bcmActionInsTOSPFromPrio, 0));
    }

    if (flags & BCM_DS_ACTN_INSERT_DSCP) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_match(unit, f,
                                         bcmActionInsDiffServ,
                                         inp_actn->ipa_dscp));
    }

    if (flags & BCM_DS_ACTN_SET_OUT_PORT_UCAST) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_match(unit, f,
                                         bcmActionSetPortUcast,
                                         inp_actn->ipa_port));
    }

    if (flags & BCM_DS_ACTN_SET_OUT_PORT_NON_UCAST) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_match(unit, f,
                                         bcmActionSetPortNonUcast,
                                         inp_actn->ipa_port));
    }

    if (flags & BCM_DS_ACTN_SET_OUT_PORT_ALL) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_match(unit, f,
                                         bcmActionSetPortAll,
                                         inp_actn->ipa_port));
    }

    if (flags & BCM_DS_ACTN_DO_SWITCH) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_match(unit, f,
                                         bcmActionDoSwitch, 0));
    }

    if (flags & BCM_DS_ACTN_DROP_PRECEDENCE) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_match(unit, f,
                                         bcmActionDropPrecedence, 0));
    }

    if (flags & BCM_DS_ACTN_SET_OUT_MODULE) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_match(unit, f,
                                         bcmActionSetModule,
                                         inp_actn->ipa_mod));
    }

    return (BCM_E_NONE);
}

#endif /* BCM_FILTER_SUPPORT */

#ifdef BCM_FIELD_SUPPORT

/*
 * Function:
 *      _ds_clfr_in_profile_action_xgs3
 * Description:
 *      Set the in profile action.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      cf_ptr - Pointer to classifier.
 *      f - Pointer to filter template to operate on.
 * Returns:
 *      BCM_E_XXX
 */

static int
_ds_clfr_in_profile_action_xgs3(int unit, _clfr_t *cf_ptr, bcm_field_entry_t e)
{
    bcm_port_config_t port_config;
    bcm_ds_inprofile_actn_t *inp_actn;
    uint32 flags;

    inp_actn = &cf_ptr->cfe_inp_actn;
    flags = inp_actn->ipa_flags;

    soc_cm_debug(DK_VERBOSE,
                 "_ds_clfr_in_profile_action_xgs3: dpid=%d cfid=%d\n",
                 cf_ptr->cfe_dpid, cf_ptr->cfe_cfid);

    BCM_IF_ERROR_RETURN(bcm_esw_port_config_get(unit, &port_config));

    if (flags & BCM_DS_ACTN_INSERT_PRIO) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_action_add(unit, e,
                                      bcmFieldActionPrioPktAndIntNew,
                                      inp_actn->ipa_pri, 0));
    }

    if (flags & BCM_DS_ACTN_SET_PRIO) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_action_add(unit, e,
                                      bcmFieldActionPrioIntNew,
                                      inp_actn->ipa_pri, 0));
    }

    if (flags & BCM_DS_ACTN_INSERT_TOSP) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_action_add(unit, e,
                                      bcmFieldActionTosNew,
                                      inp_actn->ipa_tos, 0));
    }

    if (flags & BCM_DS_ACTN_COPY_TO_CPU) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_action_add(unit, e,
                                      bcmFieldActionCopyToCpu,
                                      0, 0));
    }

    if (flags & BCM_DS_ACTN_DO_NOT_SWITCH) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_action_add(unit, e,
                                      bcmFieldActionDrop,
                                      0, 0));
    }

    if (flags & BCM_DS_ACTN_COPY_TO_MIRROR) {
        
        bcm_gport_t mirror_dest_id = 0;
        int mirror_dest_count = 0;
        bcm_port_t     port;
        int            rv = BCM_E_INTERNAL;

        BCM_PBMP_ITER(port_config.e, port) {
            rv = bcm_esw_mirror_port_dest_get(unit, port, BCM_MIRROR_PORT_INGRESS,
                                              1, &mirror_dest_id, &mirror_dest_count);

            if (BCM_SUCCESS(rv)) {
                break;
            }
        }

        if (BCM_SUCCESS(rv)) {
            rv = bcm_esw_field_action_add(unit, e, bcmFieldActionMirrorIngress,
                                          mirror_dest_id, 0);
            BCM_IF_ERROR_RETURN(rv);
        }
        else {
            return BCM_E_INTERNAL;
        }
    }

    if (flags & BCM_DS_ACTN_INCR_FFPPKT_COUNTER) {
        /*
         * The upper counter will count Green packets and the lower
         * counter will count Red/Yellow packets (out of profile).
         */
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_counter_create(unit, e));
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_action_add(unit, e,
                                      bcmFieldActionUpdateCounter,
                                      BCM_FIELD_COUNTER_MODE_GREEN_NOTGREEN |
                                      BCM_FIELD_COUNTER_MODE_PACKETS, 0));
    }

    if (flags & BCM_DS_ACTN_INSERT_PRIO_FROM_TOSP) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_action_add(unit, e,
                                      bcmFieldActionPrioPktAndIntTos,
                                      0, 0));
    }

    if (flags & BCM_DS_ACTN_INSERT_TOSP_FROM_PRIO) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_action_add(unit, e,
                                      bcmFieldActionTosCopy,
                                      0, 0));
    }

    if (flags & BCM_DS_ACTN_INSERT_DSCP) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_action_add(unit, e,
                                      bcmFieldActionDscpNew,
                                      inp_actn->ipa_dscp, 0));
    }

    if (flags & BCM_DS_ACTN_DO_SWITCH) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_action_add(unit, e,
                                      bcmFieldActionDropCancel,
                                      0, 0));
    }

    if (flags & BCM_DS_ACTN_DROP_PRECEDENCE) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_action_add(unit, e,
                                      bcmFieldActionDropPrecedence,
                                      BCM_FIELD_COLOR_RED, 0));
    }

    if (flags & (BCM_DS_ACTN_SET_OUT_PORT_UCAST |
                 BCM_DS_ACTN_SET_OUT_PORT_NON_UCAST |
                 BCM_DS_ACTN_SET_OUT_PORT_ALL)) {
        int             modid;

        if (flags & BCM_DS_ACTN_SET_OUT_MODULE) {
            modid = inp_actn->ipa_mod;
        } else {
            BCM_IF_ERROR_RETURN
                (bcm_esw_stk_my_modid_get(unit, &modid));
        }

        BCM_IF_ERROR_RETURN
            (bcm_esw_field_action_add(unit, e,
                                      bcmFieldActionRedirect,
                                      modid, inp_actn->ipa_port));
    }

    return (BCM_E_NONE);
}

#endif /* BCM_FIELD_SUPPORT */

#ifdef BCM_FILTER_SUPPORT

/*
 * Function:
 *      _ds_clfr_meter_set
 * Description:
 *      Set the meter for a classifier.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      cf_ptr - Pointer to classifier.
 * Returns:
 *      BCM_E_XXX
 */

static int
_ds_clfr_meter_set(int unit, _clfr_t *cf_ptr)
{

    if (cf_ptr->cfe_outp_actn.opa_kbits_sec) {
        if (cf_ptr->cfe_meter_id == INVALID_METER_ID) {
            BCM_IF_ERROR_RETURN
                (bcm_esw_meter_create(unit, cf_ptr->cfe_ds_port,
                                      &cf_ptr->cfe_meter_id));
        }

        return bcm_esw_meter_set(unit, cf_ptr->cfe_ds_port,
                                 cf_ptr->cfe_meter_id,
                                 cf_ptr->cfe_outp_actn.opa_kbits_sec,
                                 cf_ptr->cfe_outp_actn.opa_kbits_burst);
    }

    return (BCM_E_NONE);
}

#endif /* BCM_FILTER_SUPPORT */

#ifdef BCM_FIELD_SUPPORT

/*
 * Function:
 *      _ds_clfr_meter_set_xgs3
 * Description:
 *      Set the meter for a classifier.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      cf_ptr - Pointer to classifier.
 * Returns:
 *      BCM_E_XXX
 */

static int
_ds_clfr_meter_set_xgs3(int unit, _clfr_t *cf_ptr)
{
    int                 r = BCM_E_NONE;

    if (cf_ptr->cfe_outp_actn.opa_kbits_sec) {
        r = bcm_esw_field_meter_create(unit, cf_ptr->e);
        if (r == BCM_E_EXISTS) {
            r = BCM_E_NONE;
        }

        if (BCM_SUCCESS(r)) {
            r = (bcm_esw_field_action_add(unit, cf_ptr->e,
                                          bcmFieldActionMeterConfig,
                                          BCM_FIELD_METER_MODE_FLOW,
                                          BCM_FIELD_METER_COMMITTED));
        }

        if (BCM_SUCCESS(r)) {
            r = bcm_esw_field_meter_set(unit, cf_ptr->e,
                                        BCM_FIELD_METER_COMMITTED,
                                        cf_ptr->cfe_outp_actn.opa_kbits_sec,
                                        cf_ptr->cfe_outp_actn.opa_kbits_burst);
        }

        if (BCM_FAILURE(r)) {
            bcm_esw_field_meter_destroy(unit, cf_ptr->e);
        }
    }

    return r;
}

#endif /* BCM_FIELD_SUPPORT */

#ifdef BCM_FILTER_SUPPORT

/*
 * Function:
 *      _ds_clfr_out_profile_action
 * Description:
 *      Set the out profile action.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      cf_ptr - Pointer to classifier.
 *          f - Pointer to filter template to operate on.
 * Returns:
 *      BCM_E_XXX
 */

static int
_ds_clfr_out_profile_action(int unit, _clfr_t *cf_ptr, bcm_filterid_t f)
{
    uint32 flags = cf_ptr->cfe_outp_actn.opa_flags;

    soc_cm_debug(DK_VERBOSE,
                 "_ds_clfr_out_profile_action: dpid=%d cfid=%d\n",
                 cf_ptr->cfe_dpid, cf_ptr->cfe_cfid);

    BCM_IF_ERROR_RETURN
        (_ds_clfr_meter_set(unit, cf_ptr));

    if (flags & BCM_DS_OUT_ACTN_COPY_TO_CPU) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_out_profile(unit, f,
                                               bcmActionCopyToCpu,
                                               0, cf_ptr->cfe_meter_id));
    }

    if (flags & BCM_DS_OUT_ACTN_DO_NOT_SWITCH) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_out_profile(unit, f,
                                               bcmActionDoNotSwitch,
                                               0, cf_ptr->cfe_meter_id));
    }

    if (flags & BCM_DS_OUT_ACTN_INSERT_DSCP) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_out_profile(unit, f,
                                               bcmActionInsDiffServ,
                                               cf_ptr->cfe_outp_actn.opa_dscp,
                                               cf_ptr->cfe_meter_id));
    }

    if (flags & BCM_DS_OUT_ACTN_DO_SWITCH) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_out_profile(unit, f,
                                               bcmActionDoSwitch,
                                               0, cf_ptr->cfe_meter_id));
    }

    if (flags & BCM_DS_OUT_ACTN_DROP_PRECEDENCE) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_out_profile(unit, f,
                                               bcmActionDropPrecedence,
                                               0, cf_ptr->cfe_meter_id));
    }
    return (BCM_E_NONE);
}

#endif /* BCM_FILTER_SUPPORT */

#ifdef BCM_FIELD_SUPPORT

/*
 * Function:
 *      _ds_clfr_out_profile_action_xgs3
 * Description:
 *      Set the out profile action.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      cf_ptr - Pointer to classifier.
 *          f - Pointer to filter template to operate on.
 * Returns:
 *      BCM_E_XXX
 */

static int
_ds_clfr_out_profile_action_xgs3(int unit, _clfr_t *cf_ptr,
                                 bcm_field_entry_t e)
{
    uint32 flags = cf_ptr->cfe_outp_actn.opa_flags;

    soc_cm_debug(DK_VERBOSE,
                 "_ds_clfr_out_profile_action_xgs3: dpid=%d cfid=%d\n",
                 cf_ptr->cfe_dpid, cf_ptr->cfe_cfid);

    BCM_IF_ERROR_RETURN
        (_ds_clfr_meter_set_xgs3(unit, cf_ptr));

    

    if (flags & BCM_DS_OUT_ACTN_COPY_TO_CPU) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_action_add(unit, e,
                                      bcmFieldActionRpCopyToCpu,
                                      0, 0));
    }

    if (flags & BCM_DS_OUT_ACTN_DO_NOT_SWITCH) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_action_add(unit, e,
                                      bcmFieldActionRpDrop,
                                      0, 0));
    }

    if (flags & BCM_DS_OUT_ACTN_INSERT_DSCP) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_action_add(unit, e,
                                      bcmFieldActionRpDscpNew,
                                      cf_ptr->cfe_outp_actn.opa_dscp,
                                      0));
    }

    if (flags & BCM_DS_OUT_ACTN_DO_SWITCH) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_action_add(unit, e,
                                      bcmFieldActionRpDropCancel,
                                      0, 0));
    }

    if (flags & BCM_DS_OUT_ACTN_DROP_PRECEDENCE) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_action_add(unit, e,
                                      bcmFieldActionRpDropPrecedence,
                                      BCM_FIELD_COLOR_RED, 0));
    }

    return (BCM_E_NONE);
}

#endif /* BCM_FIELD_SUPPORT */

#ifdef BCM_FILTER_SUPPORT

/*
 * Function:
 *      _ds_clfr_nomatch_action
 * Description:
 *      Set the no match action.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      cf_ptr - Pointer to classifier.
 * Returns:
 *      BCM_E_XXX
 */

static int
_ds_clfr_nomatch_action(int unit, _clfr_t *cf_ptr)
{
    bcm_ds_nomatch_actn_t *nm_actn;
    uint32 flags;

    soc_cm_debug(DK_VERBOSE,
                 "_ds_clfr_nomatch_action: dpid=%d cfid=%d\n",
                 cf_ptr->cfe_dpid, cf_ptr->cfe_cfid);

    nm_actn = &cf_ptr->cfe_nm_actn;
    flags = nm_actn->nma_flags;

    if (flags & BCM_DS_NM_ACTN_INSERT_PRIO) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_no_match(unit, cf_ptr->f,
                                            bcmActionInsPrio,
                                            nm_actn->nma_pri));
    }

    if (flags & BCM_DS_NM_ACTN_SET_PRIO) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_no_match(unit, cf_ptr->f,
                                            bcmActionInsPrio,
                                            nm_actn->nma_pri));
    }

    if (flags & BCM_DS_NM_ACTN_INSERT_TOSP) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_no_match(unit, cf_ptr->f,
                                            bcmActionInsPrio,
                                            nm_actn->nma_tos));
    }

    if (flags & BCM_DS_NM_ACTN_COPY_TO_CPU) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_no_match(unit, cf_ptr->f,
                                            bcmActionCopyToCpu,
                                            0));
    }

    if (flags & BCM_DS_NM_ACTN_DO_NOT_SWITCH) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_no_match(unit, cf_ptr->f,
                                            bcmActionDoNotSwitch,
                                            0));
    }

    if (flags & BCM_DS_NM_ACTN_COPY_TO_MIRROR) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_no_match(unit, cf_ptr->f,
                                            bcmActionCopyToMirror,
                                            0));
    }

    if (flags & BCM_DS_NM_ACTN_INSERT_PRIO_FROM_TOSP) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_no_match(unit, cf_ptr->f,
                                            bcmActionInsPrioFromTOSP,
                                            0));
    }

    if (flags & BCM_DS_NM_ACTN_INSERT_TOSP_FROM_PRIO) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_no_match(unit, cf_ptr->f,
                                            bcmActionInsTOSPFromPrio,
                                            0));
    }

    if (flags & BCM_DS_NM_ACTN_INSERT_DSCP) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_no_match(unit, cf_ptr->f,
                                            bcmActionInsDiffServ,
                                            nm_actn->nma_dscp));
    }

    if (flags & BCM_DS_NM_ACTN_SET_OUT_PORT_UCAST) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_no_match(unit, cf_ptr->f,
                                            bcmActionSetPortUcast,
                                            nm_actn->nma_port));
    }

    if (flags & BCM_DS_NM_ACTN_SET_OUT_PORT_NON_UCAST) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_no_match(unit, cf_ptr->f,
                                            bcmActionSetPortNonUcast,
                                            nm_actn->nma_port));
    }

    if (flags & BCM_DS_NM_ACTN_SET_OUT_PORT_ALL) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_no_match(unit, cf_ptr->f,
                                            bcmActionSetPortAll,
                                            nm_actn->nma_port));
    }

    if (flags & BCM_DS_NM_ACTN_DROP_PRECEDENCE) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_no_match(unit, cf_ptr->f,
                                            bcmActionDropPrecedence,
                                            nm_actn->nma_port));
    }

    if (flags & BCM_DS_NM_ACTN_SET_OUT_MODULE) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_no_match(unit, cf_ptr->f,
                                            bcmActionSetModule,
                                            nm_actn->nma_mod));
    }

    return (BCM_E_NONE);
}

#endif /* BCM_FILTER_SUPPORT */

#ifdef BCM_FILTER_SUPPORT

/*
 * Function:
 *      _ds_clfr_qualify_l4port
 * Description:
 *      Set the classifier qualification criteria for L4 port
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      cf_ptr - Pointer to classifier.
 *      f - Pointer to filter template.
 *      dest - Qualify on TCP/UDP destination port.
 *      data - Qualified data.
 *      mask - mask.
 * Returns:
 *      BCM_E_XXX
 */

static int
_ds_clfr_qualify_l4port(int unit, _clfr_t *cf_ptr, bcm_filterid_t f,
                        int dest, uint16 data, uint16 mask)
{
    bcm_ds_clfr_t *clfr;
    int ipv4_hlen_adjust;

    clfr = &cf_ptr->cfe_clfr;

    if (clfr->cf_flags & BCM_DS_CLFR_IP_HLEN) {
        ipv4_hlen_adjust = ((clfr->cf_ip_hlen - 5) << 2) + 38;
    } else {
        ipv4_hlen_adjust = 38;
    }

    if (dest) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_data16(unit, f,
                                           ipv4_hlen_adjust + 2,
                                           data, mask));
    } else {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_data16(unit, f,
                                           ipv4_hlen_adjust,
                                           data, mask));
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _ds_clfr_qualify
 * Description:
 *      Set the classifier qualification criteria.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      cf_ptr - Pointer to classifier.
 * Returns:
 *      BCM_E_XXX
 */

static int
_ds_clfr_qualify(int unit, _clfr_t *cf_ptr)
{
    bcm_ds_clfr_t *clfr;
    uint32 flags;
    uint16 mask;
    bcm_mac_t   mac_ones = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    clfr = &cf_ptr->cfe_clfr;
    flags = clfr->cf_flags;

    if (flags & BCM_DS_CLFR_PKT_FORMAT) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_format(unit, cf_ptr->f, clfr->cf_pktfmt));
    }

    if (flags & BCM_DS_CLFR_DST_MAC_ADDR) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_data(unit, cf_ptr->f,
                                         0, sizeof(bcm_mac_t),
                                         clfr->cf_dst_mac, mac_ones));
    }

    if (flags & BCM_DS_CLFR_SRC_MAC_ADDR) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_data(unit, cf_ptr->f,
                                         6, sizeof(bcm_mac_t),
                                         clfr->cf_src_mac, mac_ones));
    }

    if (flags & BCM_DS_CLFR_VLAN_TAG) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_data16(unit, cf_ptr->f,
                                           14, clfr->cf_vid, 0x0fff));
    }

    if (flags & BCM_DS_CLFR_ETHER_TYPE) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_data16(unit, cf_ptr->f,
                                           16, clfr->cf_ethertype, 0xffff));
    }

    if (flags & BCM_DS_CLFR_IP_VERSION) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_data8(unit, cf_ptr->f,
                                          18, clfr->cf_ip_version << 4, 0xf0));
    }

    if (flags & BCM_DS_CLFR_IP_HLEN) {
        /* IP_HLEN field valid for IPv4 frames only */
        if (clfr->cf_ip_version != 4) {
            return BCM_E_PARAM;
        }
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_data8(unit, cf_ptr->f,
                                          18, clfr->cf_ip_hlen, 0x0f));
    }

    if (flags & BCM_DS_CLFR_IPV4_CODEPOINT) {
        uint8 val = clfr->cf_dscp << 2;
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_data8(unit, cf_ptr->f, 19, val, 0xfc));
    }

    if (flags & BCM_DS_CLFR_IPV4_PROTOCOL) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_data8(unit, cf_ptr->f,
                                          27, clfr->cf_protocol, 0xff));
    }

    if (flags & BCM_DS_CLFR_IPV4_SRC_ADDR) {
        bcm_ip_t val = bcm_ip_mask_create(clfr->cf_src_ip_prefix_len);
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_data32(unit, cf_ptr->f,
                                           30, clfr->cf_src_ip_addr, val));
    }

    if (flags & BCM_DS_CLFR_IPV4_DST_ADDR) {
        bcm_ip_t val = bcm_ip_mask_create(clfr->cf_dst_ip_prefix_len);
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_data32(unit, cf_ptr->f,
                                           34, clfr->cf_dst_ip_addr, val));
    }

    if (flags & BCM_DS_CLFR_IPV4_L4_SRC_PORT) {
        if (flags & BCM_DS_CLFR_IPV4_L4_SRC_MASK) {
            mask = clfr->cf_src_mask;
        } else {
            mask = 0xffff;
        }

        BCM_IF_ERROR_RETURN
            (_ds_clfr_qualify_l4port(unit, cf_ptr, cf_ptr->f, 0,
                                     clfr->cf_src_port, mask));
    }

    if (flags & BCM_DS_CLFR_IPV4_L4_DST_PORT) {
        if (flags & BCM_DS_CLFR_IPV4_L4_DST_MASK) {
            mask = clfr->cf_dst_mask;
        } else {
            mask = 0xffff;
        }

        BCM_IF_ERROR_RETURN
            (_ds_clfr_qualify_l4port(unit, cf_ptr, cf_ptr->f, 1,
                                     clfr->cf_dst_port, mask));
    }

    return (BCM_E_NONE);
}

#endif /* BCM_FILTER_SUPPORT */

#ifdef BCM_FIELD_SUPPORT

/*
 * Function:
 *      _ds_clfr_qualify_xgs3
 * Description:
 *      Set the classifier qualification criteria.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      cf_ptr - Pointer to classifier.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Sets the qset value for a qualification, and if the entry has
 *      already been allocated, qualifies the entry.
 */

static int
_ds_clfr_qualify_xgs3(int unit, _dp_t *dp_ptr, _clfr_t *cf_ptr)
{
    bcm_port_config_t port_config;
    bcm_ds_clfr_t *clfr;
    uint32 flags;
    bcm_mac_t mac_ones = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    clfr = &cf_ptr->cfe_clfr;
    flags = clfr->cf_flags;

    BCM_IF_ERROR_RETURN(bcm_esw_port_config_get(unit, &port_config));

    if (!BCM_PBMP_EQ(dp_ptr->dp_ports, port_config.all)) {
        if (dp_ptr->dp_flags & BCM_DS_EGRESS) {
            int         nports;

            BCM_PBMP_COUNT(dp_ptr->dp_ports, nports);

            if (nports != 1) {
                return BCM_E_RESOURCE;
            }

            BCM_FIELD_QSET_ADD(cf_ptr->qs, bcmFieldQualifyDstPortTgid);

            if (cf_ptr->e != 0) {
                int             port;

                BCM_PBMP_ITER(dp_ptr->dp_ports, port) {
                    break;
                }

                BCM_IF_ERROR_RETURN
                    (bcm_esw_field_qualify_DstPortTgid(unit, cf_ptr->e,
                                                       port, 0x3f));
            }
        }
        if (dp_ptr->dp_flags & BCM_DS_INGRESS) {
            BCM_FIELD_QSET_ADD(cf_ptr->qs, bcmFieldQualifyInPorts);

            if (cf_ptr->e != 0) {
                BCM_IF_ERROR_RETURN
                    (bcm_esw_field_qualify_InPorts(unit, cf_ptr->e,
                                                   dp_ptr->dp_ports,
                                                   dp_ptr->dp_ports));
            }
        }
    }

    if (flags & BCM_DS_CLFR_PKT_FORMAT) {
        BCM_FIELD_QSET_ADD(cf_ptr->qs, bcmFieldQualifyPacketFormat);

        if (cf_ptr->e != 0) {

            BCM_IF_ERROR_RETURN
                (bcm_esw_field_qualify_PacketFormat(unit, cf_ptr->e, 
                                                    clfr->cf_pktfmt, 
                                                    BCM_FIELD_EXACT_MATCH_MASK));
        }
    }

    if (flags & BCM_DS_CLFR_DST_MAC_ADDR) {
        BCM_FIELD_QSET_ADD(cf_ptr->qs, bcmFieldQualifyDstMac);

        if (cf_ptr->e != 0) {
            BCM_IF_ERROR_RETURN
                (bcm_esw_field_qualify_DstMac(unit, cf_ptr->e,
                                              clfr->cf_dst_mac, mac_ones));
        }
    }

    if (flags & BCM_DS_CLFR_SRC_MAC_ADDR) {
        BCM_FIELD_QSET_ADD(cf_ptr->qs, bcmFieldQualifySrcMac);

        if (cf_ptr->e != 0) {
            BCM_IF_ERROR_RETURN
                (bcm_esw_field_qualify_SrcMac(unit, cf_ptr->e,
                                              clfr->cf_src_mac, mac_ones));
        }
    }

    if (flags & BCM_DS_CLFR_VLAN_TAG) {
        BCM_FIELD_QSET_ADD(cf_ptr->qs, bcmFieldQualifyOuterVlan);

        if (cf_ptr->e != 0) {
            BCM_IF_ERROR_RETURN
                (bcm_esw_field_qualify_OuterVlan(unit, cf_ptr->e,
                                                 clfr->cf_vid, 0x0fff));
        }
    }

    if (flags & BCM_DS_CLFR_ETHER_TYPE) {
        BCM_FIELD_QSET_ADD(cf_ptr->qs, bcmFieldQualifyEtherType);

        if (cf_ptr->e != 0) {
            BCM_IF_ERROR_RETURN
                (bcm_esw_field_qualify_EtherType(unit, cf_ptr->e,
                                                 clfr->cf_ethertype, 0xffff));
        }
    }

    if (flags & BCM_DS_CLFR_IP_VERSION) {
        if (ds_info[unit]->udf_id == BCM_FIELD_USER_NUM_UDFS) {
            /*
             * Create a virtual UDF for the four bytes immediately
             * following the IPv4 or IPv6 EtherType. For example, in a 
             * single-tagged packet, these are bytes 18-21.
             */
            BCM_IF_ERROR_RETURN
                (_bcm_field_setup_post_ethertype_udf(unit, &ds_info[unit]->udf_id));
        }
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qset_add_udf(unit, &cf_ptr->qs, ds_info[unit]->udf_id));

        if (cf_ptr->e != 0) {
            /* 
             * IP version is in the upper 4 bits of the byte
             * immediately following an IP EtherType.
             */
            uint8 ip_ver[BCM_FIELD_USER_FIELD_SIZE];
            uint8 ip_ver_mask[BCM_FIELD_USER_FIELD_SIZE] = {0xf0, 0, 0, 0};
            ip_ver[0] = clfr->cf_ip_version << 4;
            BCM_IF_ERROR_RETURN
                (bcm_esw_field_qualify_UserDefined(unit, cf_ptr->e, 
                                                   ds_info[unit]->udf_id,
                                                   ip_ver, ip_ver_mask));
        }
    }

    if (flags & BCM_DS_CLFR_IP_HLEN) {
        if (ds_info[unit]->udf_id == BCM_FIELD_USER_NUM_UDFS) {
            /*
             * Create a virtual UDF for the four bytes immediately
             * following the IPv4 EtherType. For example, in a single-tagged 
             * packet, these are bytes 18-21.
             */
            BCM_IF_ERROR_RETURN
                (_bcm_field_setup_post_ethertype_udf(unit, &ds_info[unit]->udf_id));
        }
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qset_add_udf(unit, &cf_ptr->qs, ds_info[unit]->udf_id));

        if (cf_ptr->e != 0) {
            /* 
             * IP header length is in the lower 4 bits of the byte
             * immediately following an IP EtherType.
             */
            uint8 ip_hlen[BCM_FIELD_USER_FIELD_SIZE];
            uint8 ip_hlen_mask[BCM_FIELD_USER_FIELD_SIZE] = {0x0f, 0, 0, 0};
            ip_hlen[0] = clfr->cf_ip_hlen & 0x0f;
            BCM_IF_ERROR_RETURN
                (bcm_esw_field_qualify_UserDefined(unit, cf_ptr->e,
                                                   ds_info[unit]->udf_id,
                                                   ip_hlen, ip_hlen_mask));
        }
    }

    if (flags & BCM_DS_CLFR_IPV4_CODEPOINT) {
        BCM_FIELD_QSET_ADD(cf_ptr->qs, bcmFieldQualifyDSCP);

        if (cf_ptr->e != 0) {
            BCM_IF_ERROR_RETURN
                (bcm_esw_field_qualify_DSCP(unit, cf_ptr->e,
                                            clfr->cf_dscp, 0x3f));
        }
    }

    if (flags & BCM_DS_CLFR_IPV4_PROTOCOL) {
        BCM_FIELD_QSET_ADD(cf_ptr->qs, bcmFieldQualifyIpProtocol);

        if (cf_ptr->e != 0) {
            BCM_IF_ERROR_RETURN
                (bcm_esw_field_qualify_IpProtocol(unit, cf_ptr->e,
                                                  clfr->cf_protocol, 0xff));
        }
    }

    if (flags & BCM_DS_CLFR_IPV4_SRC_ADDR) {
        BCM_FIELD_QSET_ADD(cf_ptr->qs, bcmFieldQualifySrcIp);

        if (cf_ptr->e != 0) {
            bcm_ip_t mask = bcm_ip_mask_create(clfr->cf_src_ip_prefix_len);

            BCM_IF_ERROR_RETURN
                (bcm_esw_field_qualify_SrcIp(unit, cf_ptr->e,
                                             clfr->cf_src_ip_addr, mask));
        }
    }

    if (flags & BCM_DS_CLFR_IPV4_DST_ADDR) {
        BCM_FIELD_QSET_ADD(cf_ptr->qs, bcmFieldQualifyDstIp);

        if (cf_ptr->e != 0) {
            bcm_ip_t mask = bcm_ip_mask_create(clfr->cf_dst_ip_prefix_len);

            BCM_IF_ERROR_RETURN
                (bcm_esw_field_qualify_DstIp(unit, cf_ptr->e,
                                             clfr->cf_dst_ip_addr, mask));
        }
    }

    if (flags & BCM_DS_CLFR_IPV4_L4_SRC_PORT) {
        if (flags & BCM_DS_CLFR_IPV4_L4_SRC_MASK) {
            BCM_FIELD_QSET_ADD(cf_ptr->qs, bcmFieldQualifyL4SrcPort);

            if (cf_ptr->e != 0) {
                BCM_IF_ERROR_RETURN
                    (bcm_esw_field_qualify_L4SrcPort(unit, cf_ptr->e,
                                                     clfr->cf_src_port,
                                                     clfr->cf_src_mask));
            }
        } else {
            BCM_FIELD_QSET_ADD(cf_ptr->qs, bcmFieldQualifyRangeCheck);

            if (cf_ptr->e != 0) {
                BCM_IF_ERROR_RETURN
                    (bcm_esw_field_qualify_RangeCheck(unit, cf_ptr->e,
                                                      cf_ptr->cfe_l4port_src_range,
                                                      0));
            }
        }
    }

    if (flags & BCM_DS_CLFR_IPV4_L4_DST_PORT) {
        if (flags & BCM_DS_CLFR_IPV4_L4_DST_MASK) {
            BCM_FIELD_QSET_ADD(cf_ptr->qs, bcmFieldQualifyL4DstPort);

            if (cf_ptr->e != 0) {
                BCM_IF_ERROR_RETURN
                    (bcm_esw_field_qualify_L4DstPort(unit, cf_ptr->e,
                                                     clfr->cf_dst_port,
                                                     clfr->cf_dst_mask));
            }
        } else {
            BCM_FIELD_QSET_ADD(cf_ptr->qs, bcmFieldQualifyRangeCheck);

            if (cf_ptr->e != 0) {
                BCM_IF_ERROR_RETURN
                    (bcm_esw_field_qualify_RangeCheck(unit, cf_ptr->e,
                                                      cf_ptr->cfe_l4port_dst_range,
                                                      0));
            }
        }
    }

    /* Special support for BCM_DS_(NM_)ACTN_SET_OUT_PORT_xxx */

    if (cf_ptr->cfe_inp_actn.ipa_flags & BCM_DS_ACTN_SET_OUT_PORT_UCAST) {
        BCM_FIELD_QSET_ADD(cf_ptr->qs, bcmFieldQualifyPacketRes);

        if (cf_ptr->e != 0) {
            BCM_IF_ERROR_RETURN
                (bcm_esw_field_qualify_PacketRes(unit, cf_ptr->e,
                                                 BCM_FIELD_PKT_RES_L2UC,
                                                 BCM_FIELD_PKT_RES_L2UC));
        }
    }

    if (cf_ptr->cfe_inp_actn.ipa_flags & BCM_DS_ACTN_SET_OUT_PORT_NON_UCAST) {
        BCM_FIELD_QSET_ADD(cf_ptr->qs, bcmFieldQualifyPacketRes);

        if (cf_ptr->e != 0) {
            BCM_IF_ERROR_RETURN
                (bcm_esw_field_qualify_PacketRes(unit, cf_ptr->e,
                                                 0,
                                                 BCM_FIELD_PKT_RES_L2UC));
        }
    }

    return (BCM_E_NONE);
}

#endif /* BCM_FIELD_SUPPORT */

#ifdef BCM_FILTER_SUPPORT

/*
 * Function:
 *      _ds_classifier_pre_install
 * Description:
 *      Pre-install a classifier. This function will allocate filters and
 *      set up the qualifications and actions associated with a classifier.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      dp_ptr - Point to a datapath.
 *      cf_ptr - Pointer to a classifier area.
 * Returns:
 *      BCM_E_XXX
 */

static int
_ds_classifier_pre_install(int unit, _dp_t *dp_ptr, _clfr_t *cf_ptr)
{
    int i, j, rv = BCM_E_INTERNAL;

    soc_cm_debug(DK_VERBOSE,
                 "_ds_classifier_pre_install: dpid=%d cfid=%d\n",
                 cf_ptr->cfe_dpid, cf_ptr->cfe_cfid);

    /* allocate space for duplicates of main filter */
    cf_ptr->f_nextra = 0;

    if (cf_ptr->cfe_l4port_entries > 1) {
        int b_size;

        cf_ptr->f_nextra = cf_ptr->cfe_l4port_entries - 1;
        b_size = cf_ptr->f_nextra * sizeof(bcm_filterid_t);

        cf_ptr->f_extra = sal_alloc(b_size, ds_string);
        if (cf_ptr->f_extra == NULL) {
            return BCM_E_MEMORY;
        }

        sal_memset(cf_ptr->f_extra, 0, b_size);
    }

    BCM_IF_ERROR_RETURN(bcm_esw_filter_create(unit, &cf_ptr->f));

    bcm_esw_filter_qualify_priority(unit, cf_ptr->f, cf_ptr->cfe_precedence);

    if (dp_ptr->dp_flags & BCM_DS_EGRESS) {
        rv = bcm_esw_filter_qualify_egress(unit, cf_ptr->f, dp_ptr->dp_ports);
    }
    if (dp_ptr->dp_flags & BCM_DS_INGRESS) {
        rv = bcm_esw_filter_qualify_ingress(unit, cf_ptr->f, dp_ptr->dp_ports);
    }

    if (rv >= 0) {
        rv = _ds_clfr_qualify(unit, cf_ptr);
    }

    if (rv < 0) {
        soc_cm_debug(DK_VERBOSE,
                     "_ds_classifier_pre_install: dpid=%d cfid=%d %s\n",
                     cf_ptr->cfe_dpid, cf_ptr->cfe_cfid, bcm_errmsg(rv));
        bcm_esw_filter_destroy(unit, cf_ptr->f);
        cf_ptr->f = 0;
        return rv;
    }

    rv = _ds_clfr_in_profile_action(unit, cf_ptr, cf_ptr->f);
    if (rv < 0) {
        bcm_esw_filter_destroy(unit, cf_ptr->f);
        cf_ptr->f = 0;
        return rv;
    }

    rv = _ds_clfr_nomatch_action(unit, cf_ptr);
    if (rv < 0) {
        bcm_esw_filter_destroy(unit, cf_ptr->f);
        cf_ptr->f = 0;
        return rv;
    }

    rv = _ds_clfr_out_profile_action(unit, cf_ptr, cf_ptr->f);
    if (rv < 0) {
        bcm_esw_filter_destroy(unit, cf_ptr->f);
        cf_ptr->f = 0;
        return rv;
    }

    /*
     * The src/dest port range has been reduced into a series of value/mask
     * pairs.  If only one pair was needed it is already qualified in the
     * filter.  If more than one pair is needed, we duplicate the filter
     * into a series of "extras", and then qualify a value/mask pair into
     * each one.  The initial pair is then qualified into the original filter.
     */
    if (cf_ptr->cfe_l4port_entries > 1) {
        int     isdest;

        isdest = (cf_ptr->cfe_l4port_flags & BCM_DS_CLFR_IPV4_L4_DST_PORT)
            ? 1 : 0;

        /* duplicate the master filter to the extra copies */
        for (i = 0; i < cf_ptr->f_nextra; i++) {
            rv = bcm_esw_filter_copy(unit, cf_ptr->f, &cf_ptr->f_extra[i]);
            if (rv >= 0) {
                rv = _ds_clfr_qualify_l4port(unit, cf_ptr,
                                             cf_ptr->f_extra[i],
                                             isdest,
                                             cf_ptr->cfe_l4port_val[i+1],
                                             cf_ptr->cfe_l4port_mask[i+1]);
            }
            if (rv < 0) {
                for (j = 0; j < i; j++) {
                    bcm_esw_filter_destroy(unit, cf_ptr->f_extra[j]);
                }
                sal_free(cf_ptr->f_extra);
                cf_ptr->f_nextra = 0;
                bcm_esw_filter_destroy(unit, cf_ptr->f);
                cf_ptr->f = 0;
                sal_free(cf_ptr->cfe_l4port_val);
                sal_free(cf_ptr->cfe_l4port_mask);
                cf_ptr->cfe_l4port_val = NULL;
                cf_ptr->cfe_l4port_mask = NULL;
                return rv;
            }
        }
        /* now add the first val/mask pair to the master filter */
        rv = _ds_clfr_qualify_l4port(unit, cf_ptr,
                                     cf_ptr->f,
                                     isdest,
                                     cf_ptr->cfe_l4port_val[0],
                                     cf_ptr->cfe_l4port_mask[0]);
        if (rv < 0) {
            for (j = 0; j < cf_ptr->f_nextra; j++) {
                bcm_esw_filter_destroy(unit, cf_ptr->f_extra[j]);
            }
            sal_free(cf_ptr->f_extra);
            cf_ptr->f_nextra = 0;
            bcm_esw_filter_destroy(unit, cf_ptr->f);
            cf_ptr->f = 0;
            sal_free(cf_ptr->cfe_l4port_val);
            sal_free(cf_ptr->cfe_l4port_mask);
            cf_ptr->cfe_l4port_val = NULL;
            cf_ptr->cfe_l4port_mask = NULL;
            return rv;
        }

    }

    return BCM_E_NONE;
}

#endif /* BCM_FILTER_SUPPORT */

#ifdef BCM_FIELD_SUPPORT

/*
 * Function:
 *      _ds_classifier_pre_install_xgs3
 * Description:
 *      Pre-install a classifier. This function will allocate field
 *      groups and set up the qualifications and actions associated with
 *      a classifier.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      dp_ptr - Point to a datapath.
 *      cf_ptr - Pointer to a classifier area.
 * Returns:
 *      BCM_E_XXX
 */

static int
_ds_classifier_pre_install_xgs3(int unit, _dp_t *dp_ptr, _clfr_t *cf_ptr)
{
    int rv;

    soc_cm_debug(DK_VERBOSE,
                 "_ds_classifier_pre_install_xgs3: dpid=%d cfid=%d\n",
                 cf_ptr->cfe_dpid, cf_ptr->cfe_cfid);

    /* Generate qset */

    assert(cf_ptr->e == 0);
    assert(cf_ptr->g == 0);

    BCM_FIELD_QSET_INIT(cf_ptr->qs);

    rv = _ds_clfr_qualify_xgs3(unit, dp_ptr, cf_ptr);
    if (rv < 0) {
        return rv;
    }

    /* Allocate field group and entry */
    

    rv = bcm_esw_field_group_create(unit,
                                    cf_ptr->qs,
                                    cf_ptr->cfe_precedence,
                                    &cf_ptr->g);
    if (rv < 0) {
        cf_ptr->g = 0;
        return rv;
    }

    rv = bcm_esw_field_entry_create(unit,
                                    cf_ptr->g,
                                    &cf_ptr->e);

    if (rv < 0) {
        (void)bcm_esw_field_group_destroy(unit, cf_ptr->g);
        cf_ptr->g = 0;
        cf_ptr->e = 0;
        return rv;
    }

    /* Add qualifications and actions to entry */

    rv = _ds_clfr_qualify_xgs3(unit, dp_ptr, cf_ptr);
    if (rv < 0) {
        goto fail;
    }

    rv = _ds_clfr_in_profile_action_xgs3(unit, cf_ptr, cf_ptr->e);
    if (rv < 0) {
        goto fail;
    }

    
    if (cf_ptr->cfe_nm_actn.nma_flags != 0) {
        return BCM_E_INTERNAL;
    }

    rv = _ds_clfr_out_profile_action_xgs3(unit, cf_ptr, cf_ptr->e);
    if (rv < 0) {
        goto fail;
    }

    return BCM_E_NONE;

  fail:
    soc_cm_debug(DK_VERBOSE,
                 "_ds_classifier_pre_install_xgs3: dpid=%d cfid=%d %s\n",
                 cf_ptr->cfe_dpid, cf_ptr->cfe_cfid, bcm_errmsg(rv));

    (void)bcm_esw_field_entry_destroy(unit, cf_ptr->e);
    cf_ptr->e = 0;

    (void)bcm_esw_field_group_destroy(unit, cf_ptr->g);
    cf_ptr->g = 0;

    return rv;
}

#endif /* BCM_FIELD_SUPPORT */

#ifdef BCM_FILTER_SUPPORT

/*
 * Function:
 *      _ds_classifier_uninstall
 * Description:
 *      Uninstall a classifier from hardware.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      dp_ptr - Point to a datapath.
 *      cf_ptr - Pointer to a classifier area.
 * Returns:
 *      BCM_E_XXX
 */

static int
_ds_classifier_uninstall(int unit, _dp_t *dp_ptr, _clfr_t *cf_ptr)
{
    int i, rv;

    if (!cf_ptr->cfe_installed) {
        return BCM_E_NONE;
    }

    rv = bcm_esw_filter_remove(unit, cf_ptr->f);

    for (i = 0; i < cf_ptr->f_nextra; i++) {
        bcm_esw_filter_remove(unit, cf_ptr->f_extra[i]);
    }

    cf_ptr->cfe_installed = FALSE;

    return rv;
}

#endif /* BCM_FILTER_SUPPORT */

#ifdef BCM_FIELD_SUPPORT

/*
 * Function:
 *      _ds_classifier_uninstall_xgs3
 * Description:
 *      Uninstall a classifier from hardware.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      dp_ptr - Point to a datapath.
 *      cf_ptr - Pointer to a classifier area.
 * Returns:
 *      BCM_E_XXX
 */

static int
_ds_classifier_uninstall_xgs3(int unit, _dp_t *dp_ptr, _clfr_t *cf_ptr)
{
    int rv;

    if (!cf_ptr->cfe_installed) {
        return BCM_E_NONE;
    }

    rv = bcm_esw_field_entry_remove(unit, cf_ptr->e);

    if (BCM_SUCCESS(rv)) {
        cf_ptr->cfe_installed = FALSE;
    }

    return rv;
}

#endif /* BCM_FIELD_SUPPORT */

#ifdef BCM_FILTER_SUPPORT

/*
 * Function:
 *      _ds_classifier_install
 * Description:
 *      Install a classifier into hardware.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      dp_ptr - Point to a datapath.
 *      cf_ptr - Pointer to a classifier area.
 * Returns:
 *      BCM_E_XXX
 */

static int
_ds_classifier_install(int unit, _dp_t *dp_ptr, _clfr_t *cf_ptr)
{
    int i, j, rv;

    soc_cm_debug(DK_VERBOSE,
                 "_ds_classifier_install: dpid=%d cfid=%d\n",
                 cf_ptr->cfe_dpid, cf_ptr->cfe_cfid);

    BCM_IF_ERROR_RETURN(bcm_esw_filter_install(unit, cf_ptr->f));

    for (i = 0; i < cf_ptr->f_nextra; i++) {
        rv = bcm_esw_filter_install(unit, cf_ptr->f_extra[i]);

        if (rv < 0) {
            for (j = 0; j < i; j++) {
                bcm_esw_filter_remove(unit, cf_ptr->f_extra[j]);
            }

            bcm_esw_filter_remove(unit, cf_ptr->f);

            return rv;
        }
    }

    cf_ptr->cfe_installed = TRUE;

    return BCM_E_NONE;
}

#endif /* BCM_FILTER_SUPPORT */

#ifdef BCM_FIELD_SUPPORT

/*
 * Function:
 *      _ds_classifier_install_xgs3
 * Description:
 *      Install a classifier into hardware.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      dp_ptr - Point to a datapath.
 *      cf_ptr - Pointer to a classifier area.
 * Returns:
 *      BCM_E_XXX
 */

static int
_ds_classifier_install_xgs3(int unit, _dp_t *dp_ptr, _clfr_t *cf_ptr)
{
    soc_cm_debug(DK_VERBOSE,
                 "_ds_classifier_install_xgs3: dpid=%d cfid=%d\n",
                 cf_ptr->cfe_dpid, cf_ptr->cfe_cfid);

    BCM_IF_ERROR_RETURN(bcm_esw_field_entry_install(unit, cf_ptr->e));

    cf_ptr->cfe_installed = TRUE;

    return BCM_E_NONE;
}

#endif /* BCM_FIELD_SUPPORT */

#ifdef BCM_FILTER_SUPPORT

/*
 * Function:
 *      _ds_classifier_reinstall
 * Description:
 *      Install a classifier which is already in hardware.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      flags - Tell which one need to be updated. can be any combination of
 *              BCM_DS_UPDATE_INPROFILE - update in profile action.
 *              BCM_DS_UPDATE_OUTPROFILE - update out profile action.
 *      dp_ptr - Point to a datapath.
 *      cf_ptr - Pointer to a classifier area.
 * Returns:
 *      BCM_E_XXX
 */

static int
_ds_classifier_reinstall(int unit, uint32 flags, _dp_t *dp_ptr,
                         _clfr_t *cf_ptr)
{
    int i, rv;

    soc_cm_debug(DK_VERBOSE,
                 "_ds_classifier_reinstall: dpid=%d cfid=%d\n",
                 cf_ptr->cfe_dpid, cf_ptr->cfe_cfid);

    assert(cf_ptr->f != 0);

    if (flags & BCM_DS_UPDATE_INPROFILE) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_match(unit, cf_ptr->f,
                                         bcmActionCancelAll, 0));
        BCM_IF_ERROR_RETURN
            (_ds_clfr_in_profile_action(unit, cf_ptr, cf_ptr->f));
    }

    if (flags & BCM_DS_UPDATE_OUTPROFILE) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_action_out_profile(unit, cf_ptr->f,
                                               bcmActionCancelAll,
                                               0, INVALID_METER_ID));
        BCM_IF_ERROR_RETURN
            (_ds_clfr_out_profile_action(unit, cf_ptr, cf_ptr->f));
    }

    for (i = 0; i < cf_ptr->f_nextra; i++) {
        if (flags & BCM_DS_UPDATE_INPROFILE) {
            rv = bcm_esw_filter_action_match(unit, cf_ptr->f_extra[i],
                                             bcmActionCancelAll, 0);
            if (rv < 0) {
                return (rv);
            }

            rv = _ds_clfr_in_profile_action(unit, cf_ptr, cf_ptr->f_extra[i]);
            if (rv < 0) {
                return (rv);
            }
        }
        if (flags & BCM_DS_UPDATE_OUTPROFILE) {
            rv = bcm_esw_filter_action_out_profile(unit, cf_ptr->f_extra[i],
                                                   bcmActionCancelAll,
                                                   0, INVALID_METER_ID);
            if (rv < 0) {
                return (rv);
            }

            rv = _ds_clfr_out_profile_action(unit, cf_ptr, cf_ptr->f_extra[i]);
            if (rv < 0) {
                return (rv);
            }
        }
    }

    BCM_IF_ERROR_RETURN
        (bcm_esw_filter_reinstall(unit, cf_ptr->f));

    for (i = 0; i < cf_ptr->f_nextra; i++) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_reinstall(unit, cf_ptr->f_extra[i]));
    }

    return (BCM_E_NONE);
}

#endif /* BCM_FILTER_SUPPORT */

#ifdef BCM_FIELD_SUPPORT

/*
 * Function:
 *      _ds_classifier_reinstall_xgs3
 * Description:
 *      Install a classifier which is already in hardware.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      flags - Tell which one need to be updated. can be any combination of
 *              BCM_DS_UPDATE_INPROFILE - update in profile action.
 *              BCM_DS_UPDATE_OUTPROFILE - update out profile action.
 *      dp_ptr - Point to a datapath.
 *      cf_ptr - Pointer to a classifier area.
 * Returns:
 *      BCM_E_XXX
 */

static int
_ds_classifier_reinstall_xgs3(int unit, uint32 flags, _dp_t *dp_ptr,
                              _clfr_t *cf_ptr)
{
    soc_cm_debug(DK_VERBOSE,
                 "_ds_classifier_reinstall_xgs3: dpid=%d cfid=%d\n",
                 cf_ptr->cfe_dpid, cf_ptr->cfe_cfid);

    assert(cf_ptr->e != 0);

    if (flags & BCM_DS_UPDATE_INPROFILE) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_action_remove_all(unit, cf_ptr->e));
        BCM_IF_ERROR_RETURN
            (_ds_clfr_in_profile_action_xgs3(unit, cf_ptr, cf_ptr->e));
    }

    if (flags & BCM_DS_UPDATE_OUTPROFILE) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_action_remove_all(unit, cf_ptr->e));
        BCM_IF_ERROR_RETURN
            (_ds_clfr_out_profile_action_xgs3(unit, cf_ptr, cf_ptr->e));
    }

    BCM_IF_ERROR_RETURN
        (bcm_esw_field_entry_reinstall(unit, cf_ptr->e));

    return (BCM_E_NONE);
}

#endif /* BCM_FIELD_SUPPORT */

#ifdef BCM_FILTER_SUPPORT

/*
 * Function:
 *      _ds_classifier_remove
 * Description:
 *      Remove a classifier.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      dp_ptr - Point to a datapath.
 *      cf_ptr - Pointer to a classifier area.
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_ds_classifier_remove(int unit, _dp_t *dp_ptr, _clfr_t *cf_ptr)
{
    int i, rv = BCM_E_NONE;

    if (cf_ptr->cfe_installed) {
        rv = bcm_esw_filter_remove(unit, cf_ptr->f);
    }

    if (cf_ptr->f) {
        bcm_esw_filter_destroy(unit, cf_ptr->f);
        cf_ptr->f = 0;
    }

    if (cf_ptr->f_nextra) {
        for (i = 0; i < cf_ptr->f_nextra; i++) {
            bcm_esw_filter_remove(unit, cf_ptr->f_extra[i]);
            bcm_esw_filter_destroy(unit, cf_ptr->f_extra[i]);
        }
        sal_free(cf_ptr->f_extra);
        cf_ptr->f_extra = NULL;
        cf_ptr->f_nextra = 0;
    }

    if (cf_ptr->cfe_l4port_val) {
        sal_free(cf_ptr->cfe_l4port_val);
    }

    if (cf_ptr->cfe_l4port_mask) {
        sal_free(cf_ptr->cfe_l4port_mask);
    }

    cf_ptr->cfe_l4port_entries = 0;

    if (cf_ptr->cfe_ffpcntr_id != INVALID_FFPCOUNTER_ID) {
        bcm_esw_ffpcounter_delete(unit, cf_ptr->cfe_ds_port,
                                  cf_ptr->cfe_ffpcntr_id);
        cf_ptr->cfe_ffpcntr_id = INVALID_FFPCOUNTER_ID;
    }

    if (cf_ptr->cfe_meter_id != INVALID_METER_ID) {
        bcm_esw_meter_delete(unit, cf_ptr->cfe_ds_port, cf_ptr->cfe_meter_id);
        cf_ptr->cfe_meter_id = INVALID_METER_ID;
    }

    cf_ptr->cfe_installed = FALSE;

    return rv;
}

#endif /* BCM_FILTER_SUPPORT */

#ifdef BCM_FIELD_SUPPORT

/*
 * Function:
 *      _ds_classifier_remove_xgs3
 * Description:
 *      Remove a classifier.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      dp_ptr - Point to a datapath.
 *      cf_ptr - Pointer to a classifier area.
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_ds_classifier_remove_xgs3(int unit, _dp_t *dp_ptr, _clfr_t *cf_ptr)
{
    int rv = BCM_E_NONE;

    if (cf_ptr->cfe_installed) {
        rv = bcm_esw_field_entry_remove(unit, cf_ptr->e);
        BCM_IF_ERROR_RETURN(rv);
    }

    if (cf_ptr->e) {
        rv = bcm_esw_field_entry_destroy(unit, cf_ptr->e);
        BCM_IF_ERROR_RETURN(rv);
        cf_ptr->e = 0;
    }

    if (cf_ptr->g) {
        rv = bcm_esw_field_group_destroy(unit, cf_ptr->g);
        BCM_IF_ERROR_RETURN(rv);
        cf_ptr->g = 0;
    }

    if (cf_ptr->cfe_l4port_src_range != 0) {
        rv = bcm_esw_field_range_destroy(unit, cf_ptr->cfe_l4port_src_range);
        BCM_IF_ERROR_RETURN(rv);
        cf_ptr->cfe_l4port_src_range = 0;
    }

    if (cf_ptr->cfe_l4port_dst_range != 0) {
        rv = bcm_esw_field_range_destroy(unit, cf_ptr->cfe_l4port_dst_range);
        BCM_IF_ERROR_RETURN(rv);
        cf_ptr->cfe_l4port_dst_range = 0;
    }

    cf_ptr->cfe_installed = FALSE;

    return rv;
}

#endif /* BCM_FIELD_SUPPORT */

/*
 * Function:
 *      _ds_classifier_insert_installed
 * Description:
 *      Insert a classifier into already installed datapath.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      dp_ptr - Point to a datapath.
 *      cf_ptr - A classifier that we want to installed.
 * Returns:
 *      BCM_E_XXX
 */

static int
_ds_classifier_insert_installed(int unit, _dp_t *dp_ptr, _clfr_t *cf_ptr)
{
    _clfr_t *cur_ptr, *prev_ptr, *temp_ptr;

    if (soc_feature(unit, soc_feature_field)) {
#ifdef BCM_FIELD_SUPPORT
        BCM_IF_ERROR_RETURN
            (_ds_classifier_pre_install_xgs3(unit, dp_ptr, cf_ptr));

        if (dp_ptr->dp_clfr == NULL) {
            return (_ds_classifier_install_xgs3(unit, dp_ptr, cf_ptr));
        }
#endif
    } else {
#ifdef BCM_FILTER_SUPPORT
        BCM_IF_ERROR_RETURN
            (_ds_classifier_pre_install(unit, dp_ptr, cf_ptr));

        if (dp_ptr->dp_clfr == NULL) {
            return (_ds_classifier_install(unit, dp_ptr, cf_ptr));
        }
#endif
    }

    cur_ptr = dp_ptr->dp_clfr;
    prev_ptr = NULL;

    /* Find the last classifier which always has the highest precedence. */

    while (cur_ptr) {
        if (cur_ptr->cfe_precedence > cf_ptr->cfe_precedence) {
            temp_ptr = cur_ptr;
            prev_ptr = cur_ptr;

            /*
             * 1. Remove the installed classifier whose precedence are higher
             *    than the one we want to insert.
             * 2. Install the classifier we want to insert.
             * 3. Install the ones we already just removed from low precedence
             *    to high precedence..
             */

            while (cur_ptr) {
                prev_ptr = cur_ptr;
                cur_ptr = cur_ptr->cfe_next;
            }

            cur_ptr = prev_ptr;

            while (cur_ptr) {
                prev_ptr = cur_ptr->cfe_prev;
                if (soc_feature(unit, soc_feature_field)) {
#ifdef BCM_FIELD_SUPPORT
                    _ds_classifier_uninstall_xgs3(unit, dp_ptr, cur_ptr);
#endif
                } else {
#ifdef BCM_FILTER_SUPPORT
                    _ds_classifier_uninstall(unit, dp_ptr, cur_ptr);
#endif
                }
                cur_ptr = prev_ptr;
            }

            if (soc_feature(unit, soc_feature_field)) {
#ifdef BCM_FIELD_SUPPORT
                BCM_IF_ERROR_RETURN
                    (_ds_classifier_install_xgs3(unit, dp_ptr, cf_ptr));
#endif
            } else {
#ifdef BCM_FILTER_SUPPORT
                BCM_IF_ERROR_RETURN
                    (_ds_classifier_install(unit, dp_ptr, cf_ptr));
#endif
            }

            cur_ptr = temp_ptr;

            while (cur_ptr) {
                if (soc_feature(unit, soc_feature_field)) {
#ifdef BCM_FIELD_SUPPORT
                    BCM_IF_ERROR_RETURN
                        (_ds_classifier_install_xgs3(unit, dp_ptr, cur_ptr));
#endif
                } else {
#ifdef BCM_FILTER_SUPPORT
                    BCM_IF_ERROR_RETURN
                        (_ds_classifier_install(unit, dp_ptr, cur_ptr));
#endif
                }

                cur_ptr = cur_ptr->cfe_next;
            }

            return (BCM_E_NONE);
        } else {
            cur_ptr = cur_ptr->cfe_next;
        }
    }

    /* The new classifier has the highest precedence. */

    if (soc_feature(unit, soc_feature_field)) {
#ifdef BCM_FIELD_SUPPORT
        return _ds_classifier_install_xgs3(unit, dp_ptr, cf_ptr);
#endif
    } else {
#ifdef BCM_FILTER_SUPPORT
        return _ds_classifier_install(unit, dp_ptr, cf_ptr);
#endif
    }
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      _ds_classifier_delete
 * Purpose:
 *      Uninstall and unlink a classier entry.
 * Notes:
 *      Assumes lock held, unit valid, etc.
 */

STATIC int
_ds_classifier_delete(int unit, _dp_t *dp_ptr, _clfr_t *cf_ptr)
{
    _clfr_t *prev_ptr, *next_ptr;

    prev_ptr = cf_ptr->cfe_prev;
    next_ptr = cf_ptr->cfe_next;

    if (soc_feature(unit, soc_feature_field)) {
#ifdef BCM_FIELD_SUPPORT
        _ds_classifier_remove_xgs3(unit, dp_ptr, cf_ptr);
#endif
    } else {
#ifdef BCM_FILTER_SUPPORT
        _ds_classifier_remove(unit, dp_ptr, cf_ptr);
#endif
    }

    sal_free(cf_ptr);

    if (prev_ptr != NULL) {
        prev_ptr->cfe_next = next_ptr;
    } else if (dp_ptr != NULL) {
        dp_ptr->dp_clfr = next_ptr;
    }

    if (next_ptr != NULL) {
        next_ptr->cfe_prev = prev_ptr;
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _ds_classifier_find
 * Description:
 *      Find a a classifier.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      dp_ptr - Point to datapath.
 *      cfid - Classifier ID.
 * Returns:
 *      pointer to classifier or NULL
 */

static _clfr_t *
_ds_classifier_find(int unit, _dp_t *dp_ptr, int cfid)
{
    _clfr_t *cf_ptr;

    cf_ptr = dp_ptr->dp_clfr;

    while (cf_ptr != NULL) {
        if (cf_ptr->cfe_cfid == cfid) {
            return cf_ptr;
        }

        cf_ptr = cf_ptr->cfe_next;
    }

    return NULL;
}

#ifdef BCM_FILTER_SUPPORT

/*
 * Function:
 *      _ds_clfr_l4port_reduce
 * Description:
 *      Given a min and max l4 (ipv4) port sequence, builds
 *      a minimum set of value/mask pairs to check for that
 *      sequence.  Typically called twice, once with NULL
 *      destination pointers in order to get the number of
 *      entries, and then again to fill in the entries after
 *      they have been allocated.
 * Parameters:
 *      min     - low end of port range
 *      max     - hi end of port range
 *      eval    - (OUT) pointer to array of values
 *      emask   - (OUT) pointer to array of masks
 * Returns:
 *      number of eval/emask pairs
 */

static int
_ds_clfr_l4port_reduce(uint16 min, uint16 max, uint16 *eval, uint16 *emask)
{
    int         nentry;
    uint16      t, mask;

    nentry = 0;

    while (min <= max) {
        /* count low order 0 bits in min */
        if (min == 0) {
            mask = 0xffff;
        } else {
            for (t = min, mask = 0; (t & 1) == 0; t >>= 1) {
                mask = (mask << 1) | 1;
            }
        }
        t = (min & ~mask) | mask;
        if (t <= max) {
            ;
        } else {
            while (t > max) {
                mask >>= 1;
                t = (min & ~mask) | mask;
            }
        }
        if (eval) {
            eval[nentry] = min;
        }
        if (emask) {
            emask[nentry] = ~mask & 0xffff;
        }
        nentry += 1;
        if (t >= max) {
            break;
        }
        min = t + 1;
    }

    return nentry;
}

#endif /* BCM_FILTER_SUPPORT */

/*
 * Function:
 *      _ds_clfr_adjust
 * Description:
 *      Make adjustment to the classifier criteria.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      cf_ptr - Point to classifier area.
 * Returns:
 *      BCM_E_XXX
 */

static int
_ds_clfr_adjust(int unit, _clfr_t *cf_ptr)
{
    bcm_ds_clfr_t *clfr;
    uint32 flags;
    bcm_ds_inprofile_actn_t *inp_actn;

    clfr = &cf_ptr->cfe_clfr;

    inp_actn  = &cf_ptr->cfe_inp_actn;

    if ( (inp_actn->ipa_flags & (BCM_DS_ACTN_INSERT_DSCP |
                                 BCM_DS_ACTN_INSERT_TOSP)) &&
         (!(clfr->cf_flags & BCM_DS_CLFR_ETHER_TYPE)) ) {
        clfr->cf_flags |= (BCM_DS_CLFR_ETHER_TYPE | BCM_DS_CLFR_IP_VERSION);
        clfr->cf_ethertype = 0x0800;
        clfr->cf_ip_version = 4;              
    }

    flags = clfr->cf_flags;

    if ((flags & BCM_DS_CLFR_IPV4_SRC_ADDR) && !clfr->cf_src_ip_prefix_len) {
        clfr->cf_flags &= ~(BCM_DS_CLFR_IPV4_SRC_ADDR |
                            BCM_DS_CLFR_IPV4_SRC_PREFIX);
    }

    if ((flags & BCM_DS_CLFR_IPV4_DST_ADDR) && !clfr->cf_dst_ip_prefix_len) {
        clfr->cf_flags &= ~(BCM_DS_CLFR_IPV4_DST_ADDR |
                            BCM_DS_CLFR_IPV4_DST_PREFIX);
    }

    if ((flags & BCM_DS_CLFR_IPV4_L4_SRC_PORT_MIN) &&
        clfr->cf_src_port_min == 0 && clfr->cf_src_port_max == 65535) {
        /* All ports = null operation */
        clfr->cf_flags &= ~(BCM_DS_CLFR_IPV4_L4_SRC_PORT_MIN |
                            BCM_DS_CLFR_IPV4_L4_SRC_PORT_MAX);
    }

    if ((flags & BCM_DS_CLFR_IPV4_L4_DST_PORT_MIN) &&
        clfr->cf_dst_port_min == 0 && clfr->cf_dst_port_max == 65535) {
        /* All ports = null operation */
        clfr->cf_flags &= ~(BCM_DS_CLFR_IPV4_L4_DST_PORT_MIN |
                            BCM_DS_CLFR_IPV4_L4_DST_PORT_MAX);
    }

    if (flags & BCM_DS_CLFR_IPV4_L4_SRC_PORT_MIN) {
        clfr->cf_flags &= ~(BCM_DS_CLFR_IPV4_L4_SRC_PORT_MIN |
                            BCM_DS_CLFR_IPV4_L4_SRC_PORT_MAX);

        if (soc_feature(unit, soc_feature_field)) {
#ifdef BCM_FIELD_SUPPORT
            BCM_IF_ERROR_RETURN
                (bcm_esw_field_range_create(unit,
                                            &cf_ptr->cfe_l4port_src_range,
                                            BCM_FIELD_RANGE_SRCPORT,
                                            clfr->cf_src_port_min,
                                            clfr->cf_src_port_max));
#endif /* BCM_FIELD_SUPPORT */
        } else {
#ifdef BCM_FILTER_SUPPORT
            int nentries;

            nentries = _ds_clfr_l4port_reduce(clfr->cf_src_port_min,
                                              clfr->cf_src_port_max,
                                              (uint16 *)NULL,
                                              (uint16 *)NULL);
            if (nentries == 1) {
                (void) _ds_clfr_l4port_reduce(clfr->cf_src_port_min,
                                              clfr->cf_src_port_max,
                                              &clfr->cf_src_port,
                                              &clfr->cf_src_mask);
                clfr->cf_flags |= (BCM_DS_CLFR_IPV4_L4_SRC_PORT |
                                   BCM_DS_CLFR_IPV4_L4_SRC_MASK);
            } else {
                cf_ptr->cfe_l4port_entries = nentries;
                cf_ptr->cfe_l4port_val = sal_alloc(nentries * sizeof(uint16),
                                                   ds_string);
                if (cf_ptr->cfe_l4port_val == NULL) {
                    return BCM_E_MEMORY;
                }

                cf_ptr->cfe_l4port_mask = sal_alloc(nentries * sizeof(uint16),
                                                    ds_string);
                if (cf_ptr->cfe_l4port_mask == NULL) {
                    sal_free(cf_ptr->cfe_l4port_val);
                    return BCM_E_MEMORY;
                }

                (void) _ds_clfr_l4port_reduce(clfr->cf_src_port_min,
                                              clfr->cf_src_port_max,
                                              cf_ptr->cfe_l4port_val,
                                              cf_ptr->cfe_l4port_mask);
                cf_ptr->cfe_l4port_flags = (BCM_DS_CLFR_IPV4_L4_SRC_PORT |
                                            BCM_DS_CLFR_IPV4_L4_SRC_MASK);
            }
#endif /* BCM_FILTER_SUPPORT */
        }
    }

    if (flags & BCM_DS_CLFR_IPV4_L4_DST_PORT_MIN) {
        clfr->cf_flags &= ~(BCM_DS_CLFR_IPV4_L4_DST_PORT_MIN |
                            BCM_DS_CLFR_IPV4_L4_DST_PORT_MAX);

        if (soc_feature(unit, soc_feature_field)) {
#ifdef BCM_FIELD_SUPPORT
            BCM_IF_ERROR_RETURN
                (bcm_esw_field_range_create(unit,
                                            &cf_ptr->cfe_l4port_dst_range,
                                            BCM_FIELD_RANGE_DSTPORT,
                                            clfr->cf_dst_port_min,
                                            clfr->cf_dst_port_max));
#endif /* BCM_FIELD_SUPPORT */
        } else {
#ifdef BCM_FILTER_SUPPORT
            int nentries;

            nentries = _ds_clfr_l4port_reduce(clfr->cf_dst_port_min,
                                              clfr->cf_dst_port_max,
                                              (uint16 *)NULL,
                                              (uint16 *)NULL);
            if (nentries == 1) {
                (void) _ds_clfr_l4port_reduce(clfr->cf_dst_port_min,
                                              clfr->cf_dst_port_max,
                                              &clfr->cf_dst_port,
                                              &clfr->cf_dst_mask);
                clfr->cf_flags |= (BCM_DS_CLFR_IPV4_L4_DST_PORT |
                                   BCM_DS_CLFR_IPV4_L4_DST_MASK);
            } else {
                cf_ptr->cfe_l4port_entries = nentries;
                cf_ptr->cfe_l4port_val = sal_alloc(nentries * sizeof(uint16),
                                                   ds_string);
                if (cf_ptr->cfe_l4port_val == NULL) {
                    return BCM_E_MEMORY;
                }

                cf_ptr->cfe_l4port_mask = sal_alloc(nentries * sizeof(uint16),
                                                    ds_string);
                if (cf_ptr->cfe_l4port_mask == NULL) {
                    sal_free(cf_ptr->cfe_l4port_val);
                    return BCM_E_MEMORY;
                }

                (void) _ds_clfr_l4port_reduce(clfr->cf_dst_port_min,
                                              clfr->cf_dst_port_max,
                                              cf_ptr->cfe_l4port_val,
                                              cf_ptr->cfe_l4port_mask);
                cf_ptr->cfe_l4port_flags |= (BCM_DS_CLFR_IPV4_L4_DST_PORT |
                                             BCM_DS_CLFR_IPV4_L4_DST_MASK);
            }
#endif /* BCM_FILTER_SUPPORT */
        }
    }

    return BCM_E_NONE;
}

#ifdef BCM_FILTER_SUPPORT

/*
 * Function:
 *      _ds_same_block
 * Description:
 *      Check if the all intended ports on the same PIC.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      pbm - Port bit map.
 *      counter_port - (OUT) Pointer for returned port number.
 * Returns:
 *      BCM_E_XXX
 */

static int
_ds_same_block(int unit, bcm_pbmp_t pbm, int *counter_port)
{
    bcm_port_config_t port_config;
    int p, blk = -1;

    BCM_IF_ERROR_RETURN(bcm_esw_port_config_get(unit, &port_config));

    PBMP_ITER(pbm, p) {
        if (blk != SOC_PORT_BLOCK(unit, p)) {
            if (blk < 0) {
                blk = SOC_PORT_BLOCK(unit, p);
                if (0 == PBMP_MEMBER(port_config.e, p)) {
                    return BCM_E_PARAM;
                } else {
                    *counter_port = p;
                }
            } else {
                return BCM_E_PARAM;
            }
        }
    }

    return BCM_E_NONE;
}

#endif /* BCM_FILTER_SUPPORT */

/*
 * Function:
 *      _ds_clfr_check
 * Description:
 *      Check classifier.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      dp_ptr - Point to a datapath.
 *      cf_ptr - Point to classifier area.
 * Returns:
 *      BCM_E_XXX
 */

static int
_ds_clfr_check(int unit, _dp_t *dp_ptr, bcm_ds_clfr_t *clfr)
{
    uint32 flags;

    flags = clfr->cf_flags;

    if ((flags & BCM_DS_CLFR_IPV4_L4_SRC_PORT_MIN) &&
        !(flags & BCM_DS_CLFR_IPV4_L4_SRC_PORT_MAX)) {
        return BCM_E_PARAM;
    }

    if (!(flags & BCM_DS_CLFR_IPV4_L4_SRC_PORT_MIN) &&
        (flags & BCM_DS_CLFR_IPV4_L4_SRC_PORT_MAX)) {
        return BCM_E_PARAM;
    }

    if ((flags & BCM_DS_CLFR_IPV4_L4_DST_PORT_MIN) &&
        !(flags & BCM_DS_CLFR_IPV4_L4_DST_PORT_MAX)) {
        return BCM_E_PARAM;
    }

    if (!(flags & BCM_DS_CLFR_IPV4_L4_DST_PORT_MIN) &&
        (flags & BCM_DS_CLFR_IPV4_L4_DST_PORT_MAX)) {
        return BCM_E_PARAM;
    }

    if (!(flags & BCM_DS_CLFR_IPV4_L4_SRC_PORT_MIN) &&
        !(flags & BCM_DS_CLFR_IPV4_L4_DST_PORT_MIN)) {
        return BCM_E_NONE;
    }

#ifdef BCM_FILTER_SUPPORT
    if (!soc_feature(unit, soc_feature_field)) {
        int src_entries, dst_entries;

        src_entries = dst_entries = 0;

        if (flags & BCM_DS_CLFR_IPV4_L4_SRC_PORT_MIN) {
            src_entries = _ds_clfr_l4port_reduce(clfr->cf_src_port_min,
                                                 clfr->cf_src_port_max,
                                                 (uint16 *)NULL,
                                                 (uint16 *)NULL);
        }

        if (flags & BCM_DS_CLFR_IPV4_L4_DST_PORT_MIN) {
            dst_entries = _ds_clfr_l4port_reduce(clfr->cf_dst_port_min,
                                                 clfr->cf_dst_port_max,
                                                 (uint16 *)NULL,
                                                 (uint16 *)NULL);
        }

        /*
         * Currently we do not handle port range on both source port and
         * destination port.
         */

        if ((flags & BCM_DS_CLFR_IPV4_L4_SRC_PORT_MIN) &&
            (flags & BCM_DS_CLFR_IPV4_L4_DST_PORT_MIN)) {
            if (src_entries > 1 && dst_entries > 1) {
                return BCM_E_CONFIG;
            }
        }
    }
#endif /* BCM_FILTER_SUPPORT */

    return BCM_E_NONE;
}

/*
 * Function:
 *      _ds_inprofile_action_check
 * Description:
 *      Check to see if the in profile action is correct.
 * Parameters:
 *      inp_actn - Point to in profile action area.
 * Returns:
 *      BCM_E_XXX
 */

static int
_ds_inprofile_action_check(bcm_ds_inprofile_actn_t *inp_actn)
{
    uint32 flags = inp_actn->ipa_flags;

    if ((flags & BCM_DS_ACTN_DO_NOT_SWITCH) &&
        (flags & BCM_DS_ACTN_DO_SWITCH)) {
        return (BCM_E_PARAM);
    }

    if ((flags & BCM_DS_ACTN_INSERT_PRIO) &&
        (flags & BCM_DS_ACTN_INSERT_PRIO_FROM_TOSP)) {
        return (BCM_E_PARAM);
    }

    if ((flags & BCM_DS_ACTN_INSERT_TOSP) &&
        (flags & BCM_DS_ACTN_INSERT_TOSP_FROM_PRIO)) {
        return (BCM_E_PARAM);
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _ds_outprofile_action_check
 * Description:
 *      Check to see if the out profile action is correct.
 * Parameters:
 *      outp_actn - Point to out profile action area.
 * Returns:
 *      BCM_E_XXX
 */

static int
_ds_outprofile_action_check(bcm_ds_outprofile_actn_t *outp_actn)
{
    uint32 flags = outp_actn->opa_flags;

    if ((flags & BCM_DS_OUT_ACTN_DO_NOT_SWITCH) &&
        (flags & BCM_DS_OUT_ACTN_DO_SWITCH)) {
        return (BCM_E_PARAM);
    }

    if (outp_actn->opa_kbits_sec == 0) {
        return (BCM_E_PARAM);
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _ds_nomatch_action_check
 * Description:
 *      Check to see if the no match action is correct.
 * Parameters:
 *      nm_actn - Point to out no match action area.
 * Returns:
 *      BCM_E_XXX
 */

static int
_ds_nomatch_action_check(bcm_ds_nomatch_actn_t *nm_actn)
{
    uint32 flags = nm_actn->nma_flags;

    if ((flags & BCM_DS_NM_ACTN_INSERT_PRIO) &&
        (flags & BCM_DS_NM_ACTN_INSERT_PRIO_FROM_TOSP)) {
        return (BCM_E_PARAM);
    }

    if ((flags & BCM_DS_NM_ACTN_INSERT_TOSP) &&
        (flags & BCM_DS_NM_ACTN_INSERT_TOSP_FROM_PRIO)) {
        return (BCM_E_PARAM);
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _ds_scheduler_set
 * Description:
 *      Set the scheduling algorithm.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      dp_ptr - Point to datapath area.
 *      scheduler - Scheduling parameter.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Should probably just use bcm_cosq_(port_)sched_set directly.
 */

static int
_ds_scheduler_set(int unit, _dp_t *dp_ptr, bcm_ds_scheduler_t *scheduler)
{
    if (!(dp_ptr->dp_flags & BCM_DS_EGRESS)) {
        return (BCM_E_PARAM);
    }

    return bcm_esw_cosq_sched_set(unit, BCM_COSQ_WEIGHTED_ROUND_ROBIN,
                                  scheduler->weight, 0);
}

/*
 * PUBLIC API functions follow
 */

/*
 * Function:
 *      _ds_classifier_create
 * Purpose:
 *      Internals of bcm_ds_classifier_create, below.
 */

STATIC INLINE int
_ds_classifier_create(int unit, _dp_t *dp_ptr,
                      bcm_ds_clfr_t *clfr,
                      bcm_ds_inprofile_actn_t *inp_actn,
                      bcm_ds_outprofile_actn_t *outp_actn,
                      bcm_ds_nomatch_actn_t *nm_actn,
                      int cfid)
{
    
    _clfr_t *cf_ptr;
    _clfr_t *cur_ptr, *prev_ptr;
    int port, rv;

    if (clfr) {
        BCM_IF_ERROR_RETURN(_ds_clfr_check(unit, dp_ptr, clfr));
    }

    if (inp_actn) {
        BCM_IF_ERROR_RETURN(_ds_inprofile_action_check(inp_actn));
    }

    if (outp_actn) {
        BCM_IF_ERROR_RETURN(_ds_outprofile_action_check(outp_actn));
    }

    if (nm_actn) {
        BCM_IF_ERROR_RETURN(_ds_nomatch_action_check(nm_actn));
    }

    /* get first port set in the bitmap */
    PBMP_ITER(dp_ptr->dp_ports, port) {
        break;
    }

    /*
     * For ingress; check to see if all ports are on the same pic if metering
     * or ffpcounter is involved.
     */

#ifdef BCM_FILTER_SUPPORT
    if ((dp_ptr->dp_flags & BCM_DS_INGRESS) &&
        !soc_feature(unit, soc_feature_field)) {
        if (outp_actn ||
            (inp_actn &&
             (inp_actn->ipa_flags & BCM_DS_ACTN_INCR_FFPPKT_COUNTER))) {
            BCM_IF_ERROR_RETURN
                (_ds_same_block(unit, dp_ptr->dp_ports, &port));
        }
    }
#endif /* BCM_FILTER_SUPPORT */

    if ((cf_ptr = sal_alloc(sizeof(_clfr_t), ds_string)) == NULL) {
        return (BCM_E_MEMORY);
    }

    sal_memset(cf_ptr, 0, sizeof(_clfr_t));

#ifdef BCM_FILTER_SUPPORT
    cf_ptr->cfe_ffpcntr_id = INVALID_FFPCOUNTER_ID;
    cf_ptr->cfe_meter_id = INVALID_METER_ID;
#endif /* BCM_FILTER_SUPPORT */
    cf_ptr->cfe_ds_port = port;

    if (inp_actn) {
        sal_memcpy(&cf_ptr->cfe_inp_actn, inp_actn,
                   sizeof(bcm_ds_inprofile_actn_t));
    }

    if (outp_actn) {
        sal_memcpy(&cf_ptr->cfe_outp_actn, outp_actn,
                   sizeof(bcm_ds_outprofile_actn_t));
    }

    if (nm_actn) {
        sal_memcpy(&cf_ptr->cfe_nm_actn, nm_actn,
                   sizeof(bcm_ds_nomatch_actn_t));
    }

    if (clfr) {
        sal_memcpy(&cf_ptr->cfe_clfr, clfr, sizeof(bcm_ds_clfr_t));
        cf_ptr->cfe_precedence = clfr->cf_precedence;
    }

    rv = _ds_clfr_adjust(unit, cf_ptr);

    if (rv < 0) {
        sal_free(cf_ptr);
        return rv;
    }

    cf_ptr->cfe_dpid = dp_ptr->dpid;
    cf_ptr->cfe_cfid = cfid;

    soc_cm_debug(DK_VERBOSE,
                 "ds_classifier_create: dpid=%d cfid=%d\n",
                 dp_ptr->dpid, cf_ptr->cfe_cfid);

    if (dp_ptr->dp_installed) {
        rv = _ds_classifier_insert_installed(unit, dp_ptr, cf_ptr);

        if (rv < 0) {
            sal_free(cf_ptr);
            return (rv);
        }
    }

    /*
     * Insert classifier into a datapath classifier queue sorted by
     * precedence.
     */

    if (dp_ptr->dp_clfr == NULL) {
        dp_ptr->dp_clfr = cf_ptr;
    } else {
        prev_ptr = NULL;
        cur_ptr = dp_ptr->dp_clfr;

        while (cur_ptr != NULL) {
            if (cf_ptr->cfe_precedence < cur_ptr->cfe_precedence) {
                /* insert as the first entry */

                if (cur_ptr == dp_ptr->dp_clfr) {
                    dp_ptr->dp_clfr = cf_ptr;
                    cf_ptr->cfe_next = cur_ptr;
                    cur_ptr->cfe_prev = cf_ptr;
                } else {
                    /* insert into the middle */
                    cf_ptr->cfe_prev = prev_ptr;
                    cf_ptr->cfe_next = cur_ptr;
		    assert(prev_ptr != NULL);
                    prev_ptr->cfe_next = cf_ptr;
                    cur_ptr->cfe_prev = cf_ptr;
                }

                break;
            } else {
                if (cur_ptr->cfe_next == NULL) {
                    /* insert as the last entry */
                    cur_ptr->cfe_next  = cf_ptr;
                    cf_ptr->cfe_prev = cur_ptr;
                    break;
                }

                prev_ptr = cur_ptr;
                cur_ptr = cur_ptr->cfe_next;
            }
        }
    }

    return (BCM_E_NONE);
}

/*
 * Function: bcm_esw_ds_classifier_create
 *
 * Description:
 *      Create a classifier within a datapath.
 *
 * Parameters:
 *      unit            - StrataSwitch PCI device unit number.
 *      dpid            - Datapath ID.
 *      clfr            - Packet selection criteria (or NULL)
 *      inp_actn        - In profile action (or NULL)
 *      outp_actn       - Out of profile action (or NULL)
 *      nm_actn         - No match action (or NULL)
 *      cfid            - (OUT) Classifier id
 *
 * Returns:
 *      BCM_E_NONE     - Success
 *      BCM_E_UNIT     - Invalid unit
 *      BCM_E_UNAVAIL  - No hardware support for Diffserv
 *      BCM_E_INIT     - Diffserv not initialized on this unit
 *      BCM_E_PARAM    - cfid pointer to NULL.
 *      BCM_E_BADID    - Datapath ID out of range.
 *
 * Notes:
 *      The calling application should create all classifiers using this
 *      routine before calling bcm_ds_datapath_install.  If datapath_install
 *      has already been called, then this classifier will be installed into
 *      the hardware immediately.
 *
 *      The clfr parameter can be NULL if no packets should be selected.
 *      This is useful for low priority no match actions.
 *      The three action parameters can be NULL if no action is needed.
 */
int
bcm_esw_ds_classifier_create(int unit, int dpid,
                             bcm_ds_clfr_t *clfr,
                             bcm_ds_inprofile_actn_t *inp_actn,
                             bcm_ds_outprofile_actn_t *outp_actn,
                             bcm_ds_nomatch_actn_t *nm_actn,
                             int *cfid)
{
    int                 rv = BCM_E_RESOURCE;
    _dp_t               *dp_ptr;
    int                 free_cfid;
    int                 idx;

    if (cfid == NULL) {
        return BCM_E_PARAM;
    }

    DS_INIT_CHECK(unit);

    DPID_CHECK_E_BADID(dpid);

    DS_LOCK(unit);

    dp_ptr = _bcm_dpid_find(unit, dpid);

    if (dp_ptr == NULL) {
        DS_UNLOCK(unit);
        return BCM_E_BADID;
    }

    /* Find the next free cfid */

    free_cfid = (dp_ptr->dp_next_cfid)++;

    for (idx = 0; idx < CFID_MAX_COUNT; idx++) {
        if (_ds_classifier_find(unit, dp_ptr, free_cfid) == NULL) {
            rv = _ds_classifier_create(unit, dp_ptr, clfr, inp_actn,
                                       outp_actn, nm_actn, free_cfid);
            *cfid = free_cfid;
            break; /* Found one and created. */
        }

        free_cfid = (dp_ptr->dp_next_cfid)++;
    }

    DS_UNLOCK(unit);

    return rv;
}

/*
 * Function
 *      bcm_esw_ds_classifier_create_id
 * Purpose
 *      Create a classifier within a datapath using the given ID
 * Parameters
 *      unit            - StrataSwitch PCI device unit number.
 *      dpid            - Datapath ID.
 *      clfr            - Packet selection criteria (or NULL)
 *      inp_actn        - In profile action (or NULL)
 *      outp_actn       - Out of profile action (or NULL)
 *      nm_actn         - No match action (or NULL)
 *      cfid            - Classifier id
 * Returns
 *      BCM_E_XXX
 * Notes
 *      See bcm_ds_classifier_create
 */
int
bcm_esw_ds_classifier_create_id(int unit, int dpid,
                                bcm_ds_clfr_t *clfr,
                                bcm_ds_inprofile_actn_t *inp_actn,
                                bcm_ds_outprofile_actn_t *outp_actn,
                                bcm_ds_nomatch_actn_t *nm_actn,
                                int cfid)
{
    int rv;
    _dp_t *dp_ptr;

    DS_INIT_CHECK(unit);

    DPID_CHECK_E_BADID(dpid);

    DS_LOCK(unit);

    dp_ptr = _bcm_dpid_find(unit, dpid);

    if (dp_ptr == NULL) {
        DS_UNLOCK(unit);
        return BCM_E_BADID;
    }

    if (_ds_classifier_find(unit, dp_ptr, cfid) != NULL) {
        DS_UNLOCK(unit);
        return BCM_E_EXISTS;
    }

    rv = _ds_classifier_create(unit, dp_ptr, clfr, inp_actn, outp_actn,
                               nm_actn, cfid);

    DS_UNLOCK(unit);

    return rv;

}

/*
 * Function:
 *      _ds_classifier_update
 * Purpose:
 *      Internals of bcm_ds_classifier_update, below
 */

int
_ds_classifier_update(int unit, int dpid, int cfid, uint32 flags,
                      bcm_ds_inprofile_actn_t *inp_actn,
                      bcm_ds_outprofile_actn_t *outp_actn)
{
    _dp_t *dp_ptr;
    _clfr_t *cf_ptr;

    dp_ptr = _bcm_dpid_find(unit, dpid);
    if (dp_ptr == NULL) {
        return BCM_E_BADID;
    }

    if ((cf_ptr = _ds_classifier_find(unit, dp_ptr, cfid)) == NULL) {
        return BCM_E_BADID;
    }

    if (!(flags & (BCM_DS_UPDATE_INPROFILE | BCM_DS_UPDATE_OUTPROFILE))) {
        return BCM_E_PARAM;
    }

    /*
     * For ingress; check to see if all ports are on the same pic if metering
     * or ffpcounter is involved.
     */

#ifdef BCM_FILTER_SUPPORT
    if ((dp_ptr->dp_flags & BCM_DS_INGRESS) &&
        !soc_feature(unit, soc_feature_field)) {
        if (outp_actn ||
            (inp_actn &&
             (inp_actn->ipa_flags & BCM_DS_ACTN_INCR_FFPPKT_COUNTER))) {
            BCM_IF_ERROR_RETURN
                (_ds_same_block(unit, dp_ptr->dp_ports, &cf_ptr->cfe_ds_port));
        }
    }
#endif /* BCM_FILTER_SUPPORT */

    if (flags & BCM_DS_UPDATE_INPROFILE) {
        if (inp_actn) {
            BCM_IF_ERROR_RETURN
                (_ds_inprofile_action_check(inp_actn));

            sal_memcpy(&cf_ptr->cfe_inp_actn, inp_actn,
                       sizeof(bcm_ds_inprofile_actn_t));
        } else {
            sal_memset(&cf_ptr->cfe_inp_actn, 0,
                       sizeof(bcm_ds_inprofile_actn_t));
        }
    }

    if (flags & BCM_DS_UPDATE_OUTPROFILE) {
        if (outp_actn) {
            BCM_IF_ERROR_RETURN
                (_ds_outprofile_action_check(outp_actn));

            sal_memcpy(&cf_ptr->cfe_outp_actn, outp_actn,
                       sizeof(bcm_ds_outprofile_actn_t));
        } else {
            sal_memset(&cf_ptr->cfe_outp_actn, 0,
                       sizeof(bcm_ds_outprofile_actn_t));
        }
    }

    /* update an classifier which already been installed. */

    if (cf_ptr->cfe_installed) {
        if (soc_feature(unit, soc_feature_field)) {
#ifdef BCM_FIELD_SUPPORT
            return _ds_classifier_reinstall_xgs3(unit, flags, dp_ptr, cf_ptr);
#endif
        } else {
#ifdef BCM_FILTER_SUPPORT
            return _ds_classifier_reinstall(unit, flags, dp_ptr, cf_ptr);
#endif
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_ds_classifier_update
 * Description:
 *      Update an existing classifier.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      dpid - Datapath ID.
 *      cfid - Classifier ID in a datapath.
 *      flags - Tell which one need to be updated. can be any combination of
 *              BCM_DS_UPDATE_INPROFILE - update in profile action.
 *              BCM_DS_UPDATE_OUTPROFILE - update out profile action.
 *      inp_actn - Identify the in profile action.
 *      outp_actn - Identify the out profile action.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Only in profile and out profile actions can be updated for a given
 *      classifier. If the application wants to change the data pattern 
 *      it is classified with, the application has to delete the existing
 *      classifier and create a new one.
 */

int
bcm_esw_ds_classifier_update(int unit, int dpid, int cfid, uint32 flags,
                             bcm_ds_inprofile_actn_t *inp_actn,
                             bcm_ds_outprofile_actn_t *outp_actn)
{
    int rv;

    soc_cm_debug(DK_VERBOSE,
                 "bcm_ds_classifier_update: dpid=%d cfid=%d\n", dpid, cfid);

    DS_INIT_CHECK(unit);

    DPID_CHECK_E_BADID(dpid);

    DS_LOCK(unit);

    rv = _ds_classifier_update(unit, dpid, cfid, flags, inp_actn, outp_actn);

    DS_UNLOCK(unit);

    return rv;
}


/*
 * Function:
 *      bcm_esw_ds_classifier_delete
 * Description:
 *      Delete a classifier from a datapath.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      dpid - Datapath ID.
 *      cfid - Classifier ID in a datapath.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_ds_classifier_delete(int unit, int dpid, int cfid)
{
    _dp_t *dp_ptr;
    _clfr_t *cf_ptr;
    int rv;

    soc_cm_debug(DK_VERBOSE,
                 "bcm_ds_classifier_delete: dpid=%d cfid=%d\n", dpid, cfid);

    DS_INIT_CHECK(unit);

    DPID_CHECK_E_BADID(dpid);

    DS_LOCK(unit);

    dp_ptr = _bcm_dpid_find(unit, dpid);

    if (dp_ptr == NULL) {
        DS_UNLOCK(unit);
        return BCM_E_BADID;
    }

    if ((cf_ptr = _ds_classifier_find(unit, dp_ptr, cfid)) == NULL) {
        DS_UNLOCK(unit);
        return (BCM_E_BADID);
    }

    rv = _ds_classifier_delete(unit, dp_ptr, cf_ptr);

    DS_UNLOCK(unit);

    return rv;
}


/*
 * Function:
 *      bcm_esw_ds_classifier_get
 * Description:
 *      Get a classifier from a datapath.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      dpid - Datapath ID.
 *      cfid - Classifier ID in a datapath.
 *      clfr            - Packet selection criteria (or NULL)
 *      inp_actn        - In profile action (or NULL)
 *      outp_actn       - Out of profile action (or NULL)
 *      nm_actn         - No match action (or NULL)
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_ds_classifier_get(int unit, int dpid, int cfid,
                          bcm_ds_clfr_t *clfr, 
                          bcm_ds_inprofile_actn_t *inp_actn, 
                          bcm_ds_outprofile_actn_t *outp_actn, 
                          bcm_ds_nomatch_actn_t *nm_actn)
{
    _dp_t *dp_ptr;
    _clfr_t *cf_ptr;
    int rv;

    soc_cm_debug(DK_VERBOSE,
                 "bcm_ds_classifier_get: dpid=%d cfid=%d\n", dpid, cfid);

    rv = SOC_E_NONE;

    DS_INIT_CHECK(unit);

    DPID_CHECK_E_BADID(dpid);

    DS_LOCK(unit);

    dp_ptr = _bcm_dpid_find(unit, dpid);

    if (dp_ptr == NULL) {
        DS_UNLOCK(unit);
        return BCM_E_BADID;
    }

    if ((cf_ptr = _ds_classifier_find(unit, dp_ptr, cfid)) == NULL) {
        DS_UNLOCK(unit);
        return (BCM_E_BADID);
    }

    if (NULL != clfr) {
        *clfr = cf_ptr->cfe_clfr;
    }

    if (NULL != inp_actn) {
        *inp_actn = cf_ptr->cfe_inp_actn;
    }

    if (NULL != outp_actn) {
        *outp_actn =  cf_ptr->cfe_outp_actn;
    }

    if (NULL != nm_actn) {
        *nm_actn = cf_ptr->cfe_nm_actn;
    }

    DS_UNLOCK(unit);

    return rv;
}


/*
 * Function:
 *      bcm_esw_ds_classifier_traverse
 * Description:
 *      Traverse datapath classifiers. 
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      dpid - Datapath ID.
 *      cb   - User callback to invoke.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_ds_classifier_traverse(int unit, int dpid, 
                               bcm_ds_classifier_traverse_cb cb, 
                               void *user_data) 
{
    _dp_t   *dp_ptr;
    _clfr_t *cf_ptr;
    _clfr_t cf_info;
    int dpid_min;
    int dpid_max; 
    int dpid_idx;
    int rv = BCM_E_NONE;

    soc_cm_debug(DK_VERBOSE,
                 "bcm_ds_classifier_traverse: dpid=%d\n", dpid);

    if (NULL == cb) {
        return (BCM_E_PARAM);
    }

    DS_INIT_CHECK(unit);

    if (dpid < 0) {
        dpid_min = 0;
        dpid_max = DPID_MAX_COUNT;
    } else {
        DPID_CHECK_E_BADID(dpid);
        dpid_min = dpid;  
        dpid_max = dpid + 1;
    }

    DS_LOCK(unit);
    /* If datapath was not specified - traverse all datapathes. */ 
    for (dpid_idx = dpid_min; dpid_idx< dpid_max; dpid_idx++) {
        dp_ptr = _bcm_dpid_find(unit, dpid_idx);
        /* Skip unused datapathes. */ 
        if (dp_ptr == NULL) {
            continue;
        }

        /*  Datapath classifiers traverse. */ 
        cf_ptr = dp_ptr->dp_clfr;
        while (cf_ptr != NULL) {
            cf_info = *cf_ptr;

            /* Call application notification callback. */
            rv = (*cb)(unit, dpid_idx, cf_info.cfe_cfid, &cf_info.cfe_clfr, 
                       &cf_info.cfe_inp_actn, &cf_info.cfe_outp_actn, 
                       &cf_info.cfe_nm_actn, user_data);
            if (BCM_FAILURE(rv)) {
                DS_UNLOCK(unit);
                return rv;
            }

            cf_ptr = cf_ptr->cfe_next;
        }
    }

    DS_UNLOCK(unit);
    return rv;
}


/*
 * Function:
 *      bcm_esw_ds_scheduler_add
 * Description:
 *      Add a scheduler to a diffserv datapath.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      dpid - Datapath ID.
 *      scheduler - Scheduling parameter.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      The datapath should be in egress for this routine.
 */

int
bcm_esw_ds_scheduler_add(int unit, int dpid, bcm_ds_scheduler_t *scheduler)
{
    _dp_t *dp_ptr;
    int rv = BCM_E_NONE;

    if (!SOC_UNIT_VALID(unit)) {
        return (BCM_E_UNIT);
    }

    soc_cm_debug(DK_VERBOSE,
                 "bcm_ds_scheduler_add: dpid=%d\n", dpid);

    DS_INIT_CHECK(unit);

    DPID_CHECK_E_BADID(dpid);

    DS_LOCK(unit);

    dp_ptr = _bcm_dpid_find(unit, dpid);

    if (dp_ptr == NULL) {
        DS_UNLOCK(unit);
        return BCM_E_BADID;
    }

    if (!scheduler || scheduler->numq > NUM_COS(unit)) {
        DS_UNLOCK(unit);
        return (BCM_E_PARAM);
    }

    if (dp_ptr->dp_flags & BCM_DS_INGRESS) {
        DS_UNLOCK(unit);
        return (BCM_E_PARAM);
    }

    sal_memcpy(&dp_ptr->dp_scheduler, scheduler, sizeof(bcm_ds_scheduler_t));

    if (dp_ptr->dp_installed) {
        rv = _ds_scheduler_set(unit, dp_ptr, scheduler);
    }

    DS_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      bcm_esw_ds_datapath_install
 * Description:
 *      Install the datapath and all associated classifiers onto the
 *      hardware.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      dpid - Datapath ID.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_ds_datapath_install(int unit, int dpid)
{
    _dp_t *dp_ptr;
    _clfr_t *cur_ptr, *rm_ptr;
    int rv;

    soc_cm_debug(DK_VERBOSE, "bcm_ds_datapath_install: dpid=%d\n", dpid);

    DS_INIT_CHECK(unit);

    DPID_CHECK_E_BADID(dpid);

    DS_LOCK(unit);

    dp_ptr = _bcm_dpid_find(unit, dpid);

    if (dp_ptr == NULL) {
        DS_UNLOCK(unit);
        return BCM_E_BADID;
    }

    if (dp_ptr->dp_installed) {
        DS_UNLOCK(unit);
        return (BCM_E_NONE);
    }

    if (!dp_ptr->dp_clfr) {
        dp_ptr->dp_installed = TRUE;
        DS_UNLOCK(unit);
        return (BCM_E_NONE);
    }

    cur_ptr = dp_ptr->dp_clfr;

    while (cur_ptr) {
        rv = BCM_E_INTERNAL;

        /* create filters */

        if (soc_feature(unit, soc_feature_field)) {
#ifdef BCM_FIELD_SUPPORT
            rv = _ds_classifier_pre_install_xgs3(unit, dp_ptr, cur_ptr);

            if (rv >= 0) {
                rv = _ds_classifier_install_xgs3(unit, dp_ptr, cur_ptr);
            }
#endif
        } else {
#ifdef BCM_FILTER_SUPPORT
            rv = _ds_classifier_pre_install(unit, dp_ptr, cur_ptr);

            if (rv >= 0) {
                rv = _ds_classifier_install(unit, dp_ptr, cur_ptr);
            }
#endif
        }

        if (rv < 0) {
            rm_ptr = cur_ptr->cfe_prev;

            while (rm_ptr) {
                if (soc_feature(unit, soc_feature_field)) {
#ifdef BCM_FIELD_SUPPORT
                    _ds_classifier_remove_xgs3(unit, dp_ptr, rm_ptr);
#endif
                } else {
#ifdef BCM_FILTER_SUPPORT
                    _ds_classifier_remove(unit, dp_ptr, rm_ptr);
#endif
                }
                rm_ptr = rm_ptr->cfe_prev;
            }

            DS_UNLOCK(unit);

            return (rv);
        }

        cur_ptr = cur_ptr->cfe_next;

    }

    if (dp_ptr->dp_scheduler.numq) {
        _ds_scheduler_set(unit, dp_ptr, &dp_ptr->dp_scheduler);
    }

    dp_ptr->dp_installed = TRUE;

    DS_UNLOCK(unit);

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _dpid_add_update
 * Purpose:
 *      Utility function for datapath creation
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_MEMORY - failure to allocate new DP ID. 
 */

STATIC INLINE int
_dpid_add_update(int unit, int dpid, uint32 flags, bcm_pbmp_t ports)
{
    _dp_t *dp_ptr = NULL;

    dp_ptr = _dpid_add(unit, dpid);

    if (dp_ptr == NULL) {
        return BCM_E_MEMORY;
    }

    dp_ptr->dp_ports = ports;
    dp_ptr->dp_flags = flags;
    dp_ptr->dp_next_cfid = 1;         /* Classifier ID start with 1. */
    dp_ptr->dp_installed = FALSE;

    return BCM_E_NONE;
}

/*
 * Function: bcm_esw_ds_datapath_create
 *
 * Description:
 *      Create a diffserv datapath.
 *
 * Parameters:
 *      unit  - StrataSwitch PCI device unit number.
 *      flags - BCM_DS_INGRESS  - inbound.
 *              BCM_DS_EGRESS   - outbound.
 *      ports - The port bitmap.
 *      dpid  - (OUT), Pointer to where Diffserv datapath ID stored.
 *
 * Returns:
 *      BCM_E_NONE     - Success
 *      BCM_E_UNIT     - Invalid unit
 *      BCM_E_UNAVAIL  - No hardware support for Diffserv
 *      BCM_E_INIT     - Diffserv not initialized on this unit
 *      BCM_E_RESOURCE - No free Datapath IDs.
 *      BCM_E_MEMORY   - Datapath ID allocation failure.
 *      BCM_E_PARAM    - Invalid ports or dpid is NULL.
 *
 * Notes:
 *      bcm_ds_datapath_install has to be called to install all the classifiers
 *      into hardware.
 */
int
bcm_esw_ds_datapath_create(int unit, uint32 flags, bcm_pbmp_t ports, int *dpid)
{
    bcm_port_config_t   port_config;
    int                 idx;
    bcm_pbmp_t          tpbm;
    int                 rv = BCM_E_RESOURCE;

    DS_INIT_CHECK(unit);

    /* If no flags set, assume default of Ingress. */
    if (! (flags & (BCM_DS_INGRESS | BCM_DS_EGRESS)) ) {
        flags |= BCM_DS_INGRESS;
    }

    BCM_IF_ERROR_RETURN(bcm_esw_port_config_get(unit, &port_config));

    /* Invalid port check */
    if (!soc_feature(unit, soc_feature_field)) {
        /* Using FFP; Ethernet ports only */
        BCM_PBMP_NEGATE(tpbm, port_config.e);
    }
    else {
        /* Using FP; any valid port */
        BCM_PBMP_NEGATE(tpbm, port_config.all);
    }
    BCM_PBMP_AND(tpbm, ports);
    if (BCM_PBMP_NOT_NULL(tpbm)) {
        return (BCM_E_PARAM);
    }

    if (dpid == NULL) {
        return BCM_E_PARAM;
    }

    DS_LOCK(unit);

    for (idx = 0; idx < DPID_MAX_COUNT /* and free ID not found */ ; idx++) {
        if (_bcm_dpid_find(unit, idx) == NULL) {  /* Found a free dpid */
            rv = _dpid_add_update(unit, idx, flags, ports);
            *dpid = idx;
            break;  /* Free ID found */
        }
    }

    DS_UNLOCK(unit);

    if (rv == BCM_E_NONE) {
        soc_cm_debug(DK_VERBOSE, "bcm_ds_datapath_create: dpid=%d\n", *dpid);
    }

    return rv;
}

/*
 * Function: bcm_esw_ds_datapath_create_id
 *
 * Purpose:
 *      Create a diffserv datapath with the given DPID
 *
 * Parameters:
 *      unit     -  StrataSwitch PCI device unit number.
 *      flags    -  BCM_DS_INGRESS  inbound.
 *                  BCM_DS_EGRESS   outbound.
 *      ports    -  The  port bitmap.
 *      dpid     -  Diffserv path ID to use
 *
 * Returns:
 *      BCM_E_NONE     - Success
 *      BCM_E_UNIT     - Invalid unit
 *      BCM_E_UNAVAIL  - No hardware support for Diffserv
 *      BCM_E_INIT     - Diffserv not initialized on this unit
 *      BCM_E_PARAM    - Invalid ports.
 *      BCM_E_EXISTS   - Datapath ID already in use.
 *      BCM_E_MEMORY   - Datapath ID allocation failure.
 *      BCM_E_BADID    - Datapath ID out of range.
 *
 * Notes:
 *      See bcm_ds_datapath_create.
 */
int
bcm_esw_ds_datapath_create_id(int unit, uint32 flags, bcm_pbmp_t ports, int dpid)
{
    bcm_port_config_t   port_config;
    bcm_pbmp_t          tpbm;
    int                 rv;

    DS_INIT_CHECK(unit);

    DPID_CHECK_E_BADID(dpid);

    /* If no flags set, assume default of Ingress. */
    if (! (flags & (BCM_DS_INGRESS | BCM_DS_EGRESS)) ) {
        flags |= BCM_DS_INGRESS;
    }

    BCM_IF_ERROR_RETURN(bcm_esw_port_config_get(unit, &port_config));

    /* Invalid port check */
    if (!soc_feature(unit, soc_feature_field)) {
        /* Using FFP; Ethernet ports only */
        BCM_PBMP_NEGATE(tpbm, port_config.e);
    }
    else {
        /* Using FP; any valid port */
        BCM_PBMP_NEGATE(tpbm, port_config.all);
    }
    BCM_PBMP_AND(tpbm, ports);
    if (BCM_PBMP_NOT_NULL(tpbm)) {
        return (BCM_E_PARAM);
    }

    DS_LOCK(unit);

    if (_bcm_dpid_find(unit, dpid) != NULL) {  /* DPID exists */
        DS_UNLOCK(unit);
        return BCM_E_EXISTS;
    }

    rv = _dpid_add_update(unit, dpid, flags, ports);

    DS_UNLOCK(unit);

    soc_cm_debug(DK_VERBOSE, "bcm_ds_datapath_create_id: dpid=%d\n", dpid);

    return rv;
}

/*
 * Function:
 *      bcm_esw_ds_datapath_delete
 * Description:
 *      Delete a diffserv datapath and all associated classifiers.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      dpid - Diffserv datapath ID to be deleted.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_ds_datapath_delete(int unit, int dpid)
{
    _dp_t *dp_ptr;
    _clfr_t *cur_ptr, *prev_ptr;


    soc_cm_debug(DK_VERBOSE, "bcm_ds_datapath_delete: dpid=%d\n", dpid);

    DS_INIT_CHECK(unit);

    DPID_CHECK_E_NOT_FOUND(dpid);

    DS_LOCK(unit);

    dp_ptr = _dpid_unlink(unit, dpid);

    if (dp_ptr == NULL) {
        DS_UNLOCK(unit);
        return BCM_E_NOT_FOUND;
    }

    cur_ptr = dp_ptr->dp_clfr;

    prev_ptr = NULL;

    /* remove from the highest precedence */

    while (cur_ptr) {
        prev_ptr = cur_ptr;
        cur_ptr = cur_ptr->cfe_next;
    }

    cur_ptr = prev_ptr;

    while (cur_ptr) {
        prev_ptr = cur_ptr->cfe_prev;
        _ds_classifier_delete(unit, dp_ptr, cur_ptr);
        cur_ptr = prev_ptr;
    }

    sal_free(dp_ptr);

    DS_UNLOCK(unit);

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_esw_ds_deinit
 * Description:
 *      Deinitialize the diffserv function.
 * Parameters:
 *      unit - (IN) device unit number.
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_esw_ds_deinit(int unit)
{
    int     bucket, cleared, dpid;

    if ((0 == soc_feature(unit, soc_feature_filter_metering)) &&
        (0 == soc_feature(unit, soc_feature_field))) {
        return BCM_E_NONE;
    }

    if (ds_info[unit] == NULL) {   
        return BCM_E_NONE;
    }

    cleared = 0;
    for (bucket = 0; bucket < DPID_BUCKETS; bucket++) {
        if (ds_info[unit]->ds_dp_ptr[bucket] == NULL) {
            continue;
        }

        dpid = ds_info[unit]->ds_dp_ptr[bucket]->dpid;

        (void)bcm_esw_ds_datapath_delete(unit, dpid);

        cleared += 1;
    }

    if (ds_info[unit]->ds_mib != NULL) {
        sal_free(ds_info[unit]->ds_mib);
        ds_info[unit]->ds_mib = NULL;
    }

    if (ds_info[unit]->ds_lock != NULL) {
        sal_mutex_destroy(ds_info[unit]->ds_lock);
        ds_info[unit]->ds_lock = NULL;
    }

    sal_free(ds_info[unit]);
    ds_info[unit] = NULL;

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_esw_ds_init
 * Description:
 *      Initialize the diffserv function.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_ds_init(int unit)
{
#ifdef BCM_WARM_BOOT_SUPPORT
    int                 rv;
    soc_scache_handle_t scache_handle;
    uint8               *ds_scache_ptr;
#endif /* BCM_WARM_BOOT_SUPPORT */

    if (!soc_feature(unit, soc_feature_filter_metering) &&
        !soc_feature(unit, soc_feature_field)) {
        return BCM_E_UNAVAIL;
    }

    if (ds_info[unit] != NULL) {        /* clear old configuration */
        BCM_IF_ERROR_RETURN(_bcm_esw_ds_deinit(unit));
    } 

    ds_info[unit] = sal_alloc(sizeof(_bcm_ds_t), ds_string);

    if (ds_info[unit] == NULL) {
        return BCM_E_MEMORY;
    }

    sal_memset(ds_info[unit], 0, sizeof(_bcm_ds_t));

    ds_info[unit]->ds_lock = sal_mutex_create("DS_MUTEX");

    if (ds_info[unit]->ds_lock == NULL) {
        sal_free(ds_info[unit]);

        ds_info[unit] = NULL;

        return BCM_E_MEMORY;
    }

#ifdef BCM_FIELD_SUPPORT
    /* Set udf_id to invalid */
    ds_info[unit]->udf_id = BCM_FIELD_USER_NUM_UDFS;
#endif /* BCM_FIELD_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT
    /* Warm boot level 2 cache size */
    if (!SOC_WARM_BOOT_SCACHE_IS_LIMITED(unit)) {
        int     type;
        uint32  flags;
            
        SOC_IF_ERROR_RETURN(soc_stable_get(unit, &type, &flags));

        /* DiffServ Warm Boot is only enabled if the following flag is set */
        if (flags & SOC_STABLE_DIFFSERV) {
            _ds_scache_size = DS_SCACHE_SIZE;
        } else {
            _ds_scache_size = 0;
        }

        if (_ds_scache_size > 0) {
            SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_DIFFSERV, 0);
            rv = _bcm_esw_scache_ptr_get(unit, scache_handle,
                                         (0 == SOC_WARM_BOOT(unit)),
                                         _ds_scache_size, &ds_scache_ptr, 
                                         BCM_WB_DEFAULT_VERSION, NULL);
            if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
                return rv;
            }
        }
    }

    if (SOC_WARM_BOOT(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_ds_reinit(unit));
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_ds_counter_get
 * Description:
 *      Get packet counters for a classifier.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      dpid - Datapath ID.
 *      cfid - Classifier ID.
 *      counter - (OUT) Counter structure
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_ds_counter_get(int unit, int dpid, int cfid, bcm_ds_counters_t *counter)
{
    _dp_t *dp_ptr;
    _clfr_t *cf_ptr;

    DS_INIT_CHECK(unit);

    DPID_CHECK_E_BADID(dpid);

    DS_LOCK(unit);

    dp_ptr = _bcm_dpid_find(unit, dpid);

    if (dp_ptr == NULL) {
        DS_UNLOCK(unit);
        return BCM_E_BADID;
    }

    if ((cf_ptr = _ds_classifier_find(unit, dp_ptr, cfid)) == NULL) {
        DS_UNLOCK(unit);
        return (BCM_E_BADID);
    }

    DS_UNLOCK(unit);

    if (soc_feature(unit, soc_feature_field)) {
#ifdef BCM_FIELD_SUPPORT
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_counter_get(unit,
                                       cf_ptr->e, 0,
                                       &(counter->outp_pkt_cntr)));

        BCM_IF_ERROR_RETURN
            (bcm_esw_field_counter_get(unit,
                                       cf_ptr->e, 1,
                                       &(counter->inp_pkt_cntr)));
#endif /* BCM_FIELD_SUPPORT */
    } else {
#ifdef BCM_FILTER_SUPPORT
        BCM_IF_ERROR_RETURN
            (bcm_esw_ffppacketcounter_get(unit,
                                          cf_ptr->cfe_ds_port,
                                          cf_ptr->cfe_meter_id,
                                          &(counter->outp_pkt_cntr)));

        BCM_IF_ERROR_RETURN
            (bcm_esw_ffpcounter_get(unit,
                                    cf_ptr->cfe_ds_port,
                                    cf_ptr->cfe_ffpcntr_id,
                                    &(counter->inp_pkt_cntr)));
#endif /* BCM_FILTER_SUPPORT */
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_esw_ds_rate_get
 * Description:
 *      Get packet counters for a classifier.
 *      (Internal Use only in bcm_port_rate_ingress_get) 
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      dpid - Datapath ID.
 *      cfid - Classifier ID.
 *      kbits_sec - (OUT) Rate in kilobits (1000 bits) per second, or
 *                        zero if rate limiting is disabled.
 *      kbits_burst - (OUT) Maximum burst size in kilobits (1000 bits).
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_ds_rate_get(int unit, int dpid, int cfid,
                    uint32 *kbits_sec, uint32 *kbits_burst)
{
    _dp_t *dp_ptr;
    _clfr_t *cf_ptr;

    DS_INIT_CHECK(unit);

    DPID_CHECK_E_BADID(dpid);

    DS_LOCK(unit);

    dp_ptr = _bcm_dpid_find(unit, dpid);

    if (dp_ptr == NULL) {
        DS_UNLOCK(unit);
        return BCM_E_BADID;
    }

    if ((cf_ptr = _ds_classifier_find(unit, dp_ptr, cfid)) == NULL) {
        DS_UNLOCK(unit);
        return (BCM_E_BADID);
    }

    DS_UNLOCK(unit);

    if (soc_feature(unit, soc_feature_field)) {
#ifdef BCM_FIELD_SUPPORT
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_meter_get(unit,
                                     cf_ptr->e,
                                     BCM_FIELD_METER_COMMITTED,
                                     kbits_sec,
                                     kbits_burst));
#endif /* BCM_FIELD_SUPPORT */
    } else {
#ifdef BCM_FILTER_SUPPORT
        BCM_IF_ERROR_RETURN
            (bcm_esw_meter_get(unit,
                               cf_ptr->cfe_ds_port,
                               cf_ptr->cfe_meter_id,
                               kbits_sec,
                               kbits_burst));
#endif /* BCM_FILTER_SUPPORT */
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_esw_ds_dpid_cfid_bind
 * Description:
 *      Binding a cookie with datapath ID and classifier ID.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      mibid - Cookie.
 *      dpid - Datapath ID.
 *      cfid - Classifier ID.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_ds_dpid_cfid_bind(int unit, void *mibid, int dpid, int cfid)
{
    _ds_binary_t ent;

    DS_INIT_CHECK(unit);

    DPID_CHECK_E_BADID(dpid);

    ent.ds_mibid = mibid;
    ent.ds_dpid = dpid;
    ent.ds_cfid = cfid;

    return _ds_binary_insert(unit, &ent);
}

/*
 * Function:
 *      bcm_esw_ds_dpid_cfid_unbind
 * Description:
 *      Un-binding a cookie with datapath ID and classifier ID.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      mibid - Cookie.
 *      dpid - Datapath ID.
 *      cfid - Classifier ID.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_ds_dpid_cfid_unbind(int unit, void *mibid, int dpid, int cfid)
{
    _ds_binary_t ent;

    DS_INIT_CHECK(unit);

    DPID_CHECK_E_BADID(dpid);

    ent.ds_mibid = mibid;
    ent.ds_dpid = dpid;

    return _ds_binary_delete(unit, &ent);
}

/*
 * Function:
 *      bcm_esw_ds_dpid_cfid_get
 * Description:
 *      Get classifier ID based on datapath ID and cookie.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      mibid - Cookie.
 *      dpid - Datapath ID.
 *      cfid - (OUT) Classifier ID.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_ds_dpid_cfid_get(int unit, void *mibid, int dpid, int *cfid)
{
    int rv;
    _ds_binary_t ent;

    DS_INIT_CHECK(unit);

    if (cfid == NULL) {
        return BCM_E_PARAM;
    }

    DPID_CHECK_E_BADID(dpid);

    ent.ds_mibid = mibid;
    ent.ds_dpid = dpid;

    rv = _ds_binary_search(unit, &ent, NULL);

    if (rv < 0) {
        return rv;
    }

    *cfid = ent.ds_cfid;

    return BCM_E_NONE;
}

#else  /* !BCM_FILTER_SUPPORT && !BCM_FIELD_SUPPORT */



int
bcm_esw_ds_classifier_create(int unit, int dpid,
                             bcm_ds_clfr_t *clfr,
                             bcm_ds_inprofile_actn_t *inp_actn,
                             bcm_ds_outprofile_actn_t *outp_actn,
                             bcm_ds_nomatch_actn_t *nm_actn,
                             int *cfid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_ds_classifier_create_id(int unit, int dpid,
                                bcm_ds_clfr_t *clfr,
                                bcm_ds_inprofile_actn_t *inp_actn,
                                bcm_ds_outprofile_actn_t *outp_actn,
                                bcm_ds_nomatch_actn_t *nm_actn,
                                int cfid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_ds_classifier_update(int unit, int dpid, int cfid,
                             uint32 flags,
                             bcm_ds_inprofile_actn_t *inp_actn,
                             bcm_ds_outprofile_actn_t *outp_actn)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_ds_classifier_delete(int unit, int dpid, int cfid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_ds_init(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_ds_datapath_create(int unit, uint32 flags, bcm_pbmp_t ports,
                           int *dpid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_ds_datapath_create_id(int unit, uint32 flags, bcm_pbmp_t ports,
                              int dpid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_ds_datapath_delete(int unit, int dpid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_ds_datapath_install(int unit, int dpid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_ds_scheduler_add(int unit, int dpid,
                         bcm_ds_scheduler_t *scheduler)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_ds_counter_get(int unit, int dpid, int cfid,
                       bcm_ds_counters_t *counter)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_ds_rate_get(int unit, int dpid, int cfid,
                    uint32 *kbits_sec, uint32 *kbits_burst)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_ds_dpid_cfid_bind(int unit, void *mibid, int dpid, int cfid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_ds_dpid_cfid_unbind(int unit, void *mibid, int dpid, int cfid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_ds_dpid_cfid_get(int unit, void *mibid, int dpid, int *cfid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_ds_classifier_get(int unit, int dpid, int cfid,
                          bcm_ds_clfr_t *clfr, 
                          bcm_ds_inprofile_actn_t *inp_actn, 
                          bcm_ds_outprofile_actn_t *outp_actn, 
                          bcm_ds_nomatch_actn_t *nm_actn)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_esw_ds_classifier_traverse(int unit, int dpid, 
                               bcm_ds_classifier_traverse_cb cb, 
                               void *user_data) 
{
    return BCM_E_UNAVAIL;
}

#endif /* BCM_FILTER_SUPPORT || BCM_FIELD_SUPPORT */


#ifdef BCM_WARM_BOOT_SUPPORT

#if defined(BCM_FIELD_SUPPORT)
/*
 * Function:
 *      _bcm_ds_dp_sync
 * Purpose:
 *      Record datapath info for Level 2 Warm Boot.
 * Parameters:
 *      ds_scache_ptr - Scache pointer.
 *      dp            - Datapath to store information for.
 *      end_ptr       - End of scache pointer.
 *      rv            - Return value BCM_E_XXX.
 * Returns:
 *      Scache pointer.
 * Notes:
 *      The following is stored for a datapath:
 *      - 1 uint16 : Number of classifiers to follow
 *      - n uint16 : Field Group ID, where 'n' is the number of classifiers
 *
 *      Only installed datapaths/classifiers are stored.
 */
STATIC uint8 *
_bcm_ds_dp_sync(uint8 *ds_scache_ptr, _dp_t *dp, uint8 *end_ptr, int *rv)
{
    int      size;
    uint8    *num_clfr_ptr;
    uint16   num_clfr;
    uint16   group;
    _clfr_t  *clfr;

    /* Need store at least the number of classifiers */
    size = sizeof(num_clfr);
    if ((end_ptr - ds_scache_ptr) < size) {
        *rv = BCM_E_MEMORY;
        return ds_scache_ptr;
    }

    /* Number of classifiers */
    num_clfr_ptr = ds_scache_ptr;
    ds_scache_ptr += sizeof(num_clfr);

    /* Size required for each classifier */
    size = sizeof(group);  /* FP groupd id */

    /* Classifiers */
    num_clfr = 0;
    clfr = dp->dp_clfr;
    while (clfr != NULL){
        if ((end_ptr - ds_scache_ptr) < size) {
            *rv = BCM_E_MEMORY;
            break;
        }

        /* Store installed classifiers */
        if (clfr->cfe_installed) {
            /* FP group id */
            group = clfr->g;
            sal_memcpy(ds_scache_ptr, &group, sizeof(group));
            ds_scache_ptr += sizeof(group);
            num_clfr++;
        }

        clfr = clfr->cfe_next;
    }

    sal_memcpy(num_clfr_ptr, &num_clfr, sizeof(num_clfr));
    
    return ds_scache_ptr;
}
#endif /* BCM_FIELD_SUPPORT */


/*
 * Function:
 *      _bcm_esw_ds_sync
 * Purpose:
 *      Record DiffServ module persisitent info for Level 2 Warm Boot
 * Parameters:
 *      unit - Unit number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Supported only for XGS-3 and newer devices.
 *      Only installed datapaths/classifiers are stored.
 *
 *      Information for DiffServ will be stored as follows:
 *        1 uint16 : num-dp, number of datapaths to follow
 *
 *      If number of datapaths is > 0, then for each datapath:
 *        1 uint16 : num-clfr, number of classifiers to follow
 *
 *      If number of classifiers is > 0, then for each classifier:
 *        1 uint16 : group, field Group ID
 *
 * +--------+---------------------------------+-------------+------------+
 * | num-dp |               dp0               |    .....    |   dp_m     |
 * |        |----------+-------+-----+--------+-------------+------------+
 * |        | num-clfr | clfr0 | ... | clfr_n |                          |
 * |        |          +-------+-----+--------+--------------------------+
 * |        |          | group0| ....| group_n|                          |
 * +--------+----------+-------+-----+--------+--------------------------+
 *
 */
int
_bcm_esw_ds_sync(int unit)
{
    int                 rv = BCM_E_NONE;
#if defined(BCM_FIELD_SUPPORT)
    soc_scache_handle_t scache_handle;
    uint8               *ds_scache_ptr;
    uint8               *end_ptr;
    uint8               *num_dp_ptr;
    int                 i;
    int                 size;
    uint16              num_dp;
    _dp_t               *dp;

    if (!soc_feature(unit, soc_feature_field)) {
        return BCM_E_NONE;
    }

    if (SOC_WARM_BOOT_SCACHE_IS_LIMITED(unit) || (_ds_scache_size == 0)) {
        return BCM_E_NONE;
    }

    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_DIFFSERV, 0);
    BCM_IF_ERROR_RETURN
            (_bcm_esw_scache_ptr_get(unit, scache_handle, FALSE,
                                     0, &ds_scache_ptr, 
                                     BCM_WB_DEFAULT_VERSION, NULL));

    end_ptr = ds_scache_ptr + _ds_scache_size;

    /* Need store at least the number of datapaths */
    size = sizeof(num_dp);
    if ((end_ptr - ds_scache_ptr) < size) {
        return BCM_E_MEMORY;
    }

    /* Number of datapaths */
    num_dp_ptr = ds_scache_ptr;
    ds_scache_ptr += sizeof(num_dp);

    /* Datapaths */
    num_dp = 0;
    for (i = 0; i < DPID_BUCKETS; i++) {
        dp = ds_info[unit]->ds_dp_ptr[i];

        while (dp != NULL) {
            /* Store installed datapaths */
            if (dp->dp_installed) {
                ds_scache_ptr = _bcm_ds_dp_sync(ds_scache_ptr, dp, end_ptr, &rv);
                if (BCM_FAILURE(rv)) {
                    break;
                }
                num_dp++;
            }
            dp = dp->next;
        }
        if (BCM_FAILURE(rv)) {
            break;
        }
    }

    sal_memcpy(num_dp_ptr, &num_dp, sizeof(num_dp));

#endif /* BCM_FIELD_SUPPORT */

    return rv;
}

#if defined(BCM_FIELD_SUPPORT)
/*
 * Function:
 *      _bcm_ds_all_color_action_check
 * Purpose:
 *      Checks that all color-based action is set for given field entry.
 *
 *      A color-based action is set if any of this is true:
 *      - the generic color-based action is set, or
 *      - all of the specific color-based actions are set
 *
 * Parameters:
 *      unit          - Unit number.
 *      entry         - Field entry.
 *      action        - Field action.
 *      color_actions - Array of specific color-based actions.
 *      count         - Number of actions in color_actions.
 *      param0        - Returns param0 for action.
 *      param1        - Returns param1 for action.
 * Returns:
 *      BCM_E_SUCCESS - Color based action is set 
 */
STATIC int
_bcm_ds_all_color_action_check(int unit, bcm_field_entry_t entry,
                               bcm_field_action_t action,
                               bcm_field_action_t *color_actions, int count,
                               uint32 *param0, uint32 *param1)
{
    int  rv;
    int  i;

    rv = bcm_esw_field_action_get(unit, entry, action, param0, param1);
    if (BCM_SUCCESS(rv)) {  /* Nothing else to check */
        return rv;
    }

    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        return rv;
    }

    /* Check that all color-based actions are set */
    for (i = 0; i < count; i++) {
        rv = bcm_esw_field_action_get(unit, entry, color_actions[i],
                                      param0, param1);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_ds_color_action_check
 * Purpose:
 *      Checks that given specific color-based action is set
 *      for specified field entry.
 *
 *      Function returns BCM_E_SUCCESS if both of these is true:
 *      - The specified color-based 'action' is set
 *      - All of the other color-based actions are clear
 *
 * Parameters:
 *      unit          - Unit number.
 *      entry         - Field entry.
 *      action        - A color action.
 *      color_actions - Array of actions to verify that are clear.
 *      count         - Number of actions in actions_color.
 *      param0        - Returns param0 for action.
 *      param1        - Returns param1 for action.
 * Returns:
 *      BCM_E_SUCCESS - Specific color based action is set 
 */
STATIC int
_bcm_ds_color_action_check(int unit, bcm_field_entry_t entry,
                           bcm_field_action_t action,
                           bcm_field_action_t *color_actions, int count,
                           uint32 *param0, uint32 *param1)
{
    int     rv;
    int     i;
    uint32  param0_tmp, param1_tmp;

    rv = bcm_esw_field_action_get(unit, entry, action, param0, param1);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    /* Check that the other color-based actions are not set */
    for (i = 0; i < count; i++) {
        rv = bcm_esw_field_action_get(unit, entry, color_actions[i],
                                      &param0_tmp, &param1_tmp);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        }
        if (BCM_SUCCESS(rv)) {
            return BCM_E_NOT_FOUND;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_ds_in_profile_action_reinit
 * Purpose:
 *      Recover in profile action information for given classifier.
 * Parameters:
 *      unit - Unit number.
 *      clfr - Classifier.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_ds_in_profile_action_reinit(int unit, _clfr_t *clfr)
{
    int                      rv;
    bcm_field_entry_t        entry;
    bcm_ds_inprofile_actn_t  *action;
    bcm_field_action_t       color_actions[3];  /* R, Y, G */
    uint32                   param0, param1;

    entry  = clfr->e;
    action = &clfr->cfe_inp_actn;

    /*
     * In some devices, color-based actions, such as
     * bcmFieldActionPrioPktAndIntNew or bcmFieldActionCopyToCpu,
     * translate into a combination of the three color-based
     * actions (bcmFieldActionRp..., bcmFieldActionYp...,
     * bcmFieldActionGp...)
     *
     * Assume that the user selected BCM_DS_ACTN_xxx when
     * all of the 3 color-based actions are present.
     */

    /* bcmFieldActionPrioPktAndIntNew */
    color_actions[0] = bcmFieldActionRpPrioPktAndIntNew;
    color_actions[1] = bcmFieldActionYpPrioPktAndIntNew;
    color_actions[2] = bcmFieldActionGpPrioPktAndIntNew;
    rv = _bcm_ds_all_color_action_check(unit, entry,
                                        bcmFieldActionPrioPktAndIntNew,
                                        color_actions, 3, &param0, &param1);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        return rv;
    } else if (BCM_SUCCESS(rv)) {
        action->ipa_pri    = param0;
        action->ipa_flags |= BCM_DS_ACTN_INSERT_PRIO;
    }

    /* bcmFieldActionPrioIntNew */
    color_actions[0] = bcmFieldActionRpPrioIntNew;
    color_actions[1] = bcmFieldActionYpPrioIntNew;
    color_actions[2] = bcmFieldActionGpPrioIntNew;
    rv = _bcm_ds_all_color_action_check(unit, entry,
                                        bcmFieldActionPrioIntNew,
                                        color_actions, 3, &param0, &param1);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        return rv;
    } else if (BCM_SUCCESS(rv)) {
        action->ipa_pri    = param0;
        action->ipa_flags |= BCM_DS_ACTN_SET_PRIO;
    }

    /* bcmFieldActionTosNew */
    rv = bcm_esw_field_action_get(unit, entry, bcmFieldActionTosNew,
                                  &param0, &param1);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        return rv;
    } else if (BCM_SUCCESS(rv)) {
        action->ipa_tos    = param0;
        action->ipa_flags |= BCM_DS_ACTN_INSERT_TOSP;
    }

    /* bcmFieldActionCopyToCpu */
    color_actions[0] = bcmFieldActionRpCopyToCpu;
    color_actions[1] = bcmFieldActionYpCopyToCpu;
    color_actions[2] = bcmFieldActionGpCopyToCpu;
    rv = _bcm_ds_all_color_action_check(unit, entry,
                                        bcmFieldActionCopyToCpu,
                                        color_actions, 3, &param0, &param1);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        return rv;
    } else if (BCM_SUCCESS(rv)) {
        action->ipa_flags |= BCM_DS_ACTN_COPY_TO_CPU;
    }

    /* bcmFieldActionDrop */
    color_actions[0] = bcmFieldActionRpDrop;
    color_actions[1] = bcmFieldActionYpDrop;
    color_actions[2] = bcmFieldActionGpDrop;
    rv = _bcm_ds_all_color_action_check(unit, entry,
                                        bcmFieldActionDrop,
                                        color_actions, 3, &param0, &param1);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        return rv;
    } else if (BCM_SUCCESS(rv)) {
        action->ipa_flags |= BCM_DS_ACTN_DO_NOT_SWITCH;
    }

    /* bcmFieldActionMirrorIngress */
    color_actions[0] = bcmFieldActionRpMirrorIngress;
    color_actions[1] = bcmFieldActionGpMirrorIngress;
    rv = _bcm_ds_all_color_action_check(unit, entry,
                                        bcmFieldActionMirrorIngress,
                                        color_actions, 2, &param0, &param1);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        return rv;
    } else if (BCM_SUCCESS(rv)) {
        action->ipa_flags |= BCM_DS_ACTN_COPY_TO_MIRROR;
    }

    /* bcmFieldActionUpdateCounter */
    rv = bcm_esw_field_action_get(unit, entry, bcmFieldActionUpdateCounter,
                                  &param0, &param1);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        return rv;
    } else if (BCM_SUCCESS(rv)) {
        action->ipa_flags |= BCM_DS_ACTN_INCR_FFPPKT_COUNTER;
    } else if (rv == BCM_E_NOT_FOUND) {  /* Try reading stats */
        int               stat_id;
        bcm_field_stat_t  stat_arr[2];

        rv = bcm_esw_field_entry_stat_get(unit, entry, &stat_id);
        if (BCM_SUCCESS(rv)) {
            rv = bcm_esw_field_stat_config_get(unit, stat_id, 2, stat_arr);

            if (BCM_SUCCESS(rv)) {
                if ((stat_arr[0] == bcmFieldStatNotGreenPackets) &&
                    (stat_arr[1] == bcmFieldStatGreenPackets)) {
                    action->ipa_flags |= BCM_DS_ACTN_INCR_FFPPKT_COUNTER;
                }
            }
        }
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND) &&
            (rv != BCM_E_UNAVAIL)) {
            return rv;
        }
    }

    /* bcmFieldActionPrioPktAndIntTos */
    color_actions[0] = bcmFieldActionRpPrioPktAndIntTos;
    color_actions[1] = bcmFieldActionYpPrioPktAndIntTos;
    color_actions[2] = bcmFieldActionGpPrioPktAndIntTos;
    rv = _bcm_ds_all_color_action_check(unit, entry,
                                        bcmFieldActionPrioPktAndIntTos,
                                        color_actions, 3, &param0, &param1);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        return rv;
    } else if (BCM_SUCCESS(rv)) {
        action->ipa_flags |= BCM_DS_ACTN_INSERT_PRIO_FROM_TOSP;
    }

    /* bcmFieldActionTosCopy */
    rv = bcm_esw_field_action_get(unit, entry, bcmFieldActionTosCopy,
                                  &param0, &param1);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        return rv;
    } else if (BCM_SUCCESS(rv)) {
        action->ipa_flags |= BCM_DS_ACTN_INSERT_TOSP_FROM_PRIO;
    }

    /* bcmFieldActionDscpNew */
    color_actions[0] = bcmFieldActionRpDscpNew;
    color_actions[1] = bcmFieldActionYpDscpNew;
    color_actions[2] = bcmFieldActionGpDscpNew;
    rv = _bcm_ds_all_color_action_check(unit, entry,
                                        bcmFieldActionDscpNew,
                                        color_actions, 3, &param0, &param1);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        return rv;
    } else if (BCM_SUCCESS(rv)) {
        action->ipa_dscp   = param0;
        action->ipa_flags |= BCM_DS_ACTN_INSERT_DSCP;
    }

    /* bcmFieldActionDropCancel */
    color_actions[0] = bcmFieldActionRpDropCancel;
    color_actions[1] = bcmFieldActionYpDropCancel;
    color_actions[2] = bcmFieldActionGpDropCancel;
    rv = _bcm_ds_all_color_action_check(unit, entry,
                                        bcmFieldActionDropCancel,
                                        color_actions, 3, &param0, &param1);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        return rv;
    } else if (BCM_SUCCESS(rv)) {
        action->ipa_flags |= BCM_DS_ACTN_DO_SWITCH;
    }

    /* bcmFieldActionDropPrecedence */
    color_actions[0] = bcmFieldActionRpDropPrecedence;
    color_actions[1] = bcmFieldActionYpDropPrecedence;
    color_actions[2] = bcmFieldActionGpDropPrecedence;
    rv = _bcm_ds_all_color_action_check(unit, entry,
                                        bcmFieldActionDropPrecedence,
                                        color_actions, 3, &param0, &param1);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        return rv;
    } else if (BCM_SUCCESS(rv)) {
        action->ipa_flags |= BCM_DS_ACTN_DROP_PRECEDENCE;
    }

    /* bcmFieldActionRedirect */
    color_actions[0] = bcmFieldActionRpRedirectPort;
    color_actions[1] = bcmFieldActionGpRedirectPort;
    rv = _bcm_ds_all_color_action_check(unit, entry,
                                        bcmFieldActionRedirect,
                                        color_actions, 2, &param0, &param1);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        return rv;
    } else if (BCM_SUCCESS(rv)) {
        action->ipa_mod    = param0;
        action->ipa_port   = param1;
        action->ipa_flags |= BCM_DS_ACTN_SET_OUT_MODULE;

        /* Any (or all) of these flags could have been set:
         *   BCM_DS_ACTN_SET_OUT_PORT_UCAST
         *   BCM_DS_ACTN_SET_OUT_PORT_NON_UCAST
         *   BCM_DS_ACTN_SET_OUT_PORT_ALL
         *
         * Set only BCM_DS_ACTN_SET_OUT_PORT_ALL, since the other
         * two will be taken care of during the qualifiers recovery.
         */
        action->ipa_flags |= BCM_DS_ACTN_SET_OUT_PORT_ALL;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_ds_out_profile_action_reinit
 * Purpose:
 *      Recover out profile action information for given classifier.
 * Parameters:
 *      unit - Unit number.
 *      clfr - Classifier.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_ds_out_profile_action_reinit(int unit, _clfr_t *clfr)
{
    int                       rv;
    bcm_field_entry_t         entry;
    bcm_ds_outprofile_actn_t  *action;
    bcm_field_action_t        color_actions[2];
    uint32                    param0, param1;

    entry  = clfr->e;
    action = &clfr->cfe_outp_actn;

    rv = bcm_esw_field_meter_get(unit, entry,
                                 BCM_FIELD_METER_COMMITTED,
                                 &action->opa_kbits_sec,
                                 &action->opa_kbits_burst);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        return rv;
    }

    /*
     * In some devices, color-based actions, such as
     * bcmFieldActionPrioPktAndIntNew or bcmFieldActionCopyToCpu,
     * translate into a combination of the three color-based
     * actions (bcmFieldActionRp..., bcmFieldActionYp...,
     * bcmFieldActionGp...)
     *
     * Assume that the user selected BCM_DS_OUT_ACTN_xxx only
     * when the Rp color-based action is present (Yp/Gp actions are not set).
     */

    /* bcmFieldActionRpCopyToCpu */
    color_actions[0] = bcmFieldActionYpCopyToCpu;
    color_actions[1] = bcmFieldActionGpCopyToCpu;
    rv = _bcm_ds_color_action_check(unit, entry, bcmFieldActionRpCopyToCpu,
                                    color_actions, 2, &param0, &param1);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        return rv;
    } else if (BCM_SUCCESS(rv)) {
        action->opa_flags |= BCM_DS_OUT_ACTN_COPY_TO_CPU;
    }

    /* bcmFieldActionRpDrop */
    color_actions[0] = bcmFieldActionYpDrop;
    color_actions[1] = bcmFieldActionGpDrop;
    rv = _bcm_ds_color_action_check(unit, entry, bcmFieldActionRpDrop,
                                    color_actions, 2, &param0, &param1);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        return rv;
    } else if (BCM_SUCCESS(rv)) {
        action->opa_flags |= BCM_DS_OUT_ACTN_DO_NOT_SWITCH;
    }

    /* bcmFieldActionRpDscpNew */
    color_actions[0] = bcmFieldActionYpDscpNew;
    color_actions[1] = bcmFieldActionGpDscpNew;
    rv = _bcm_ds_color_action_check(unit, entry, bcmFieldActionRpDscpNew,
                                    color_actions, 2, &param0, &param1);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        return rv;
    } else if (BCM_SUCCESS(rv)) {
        action->opa_dscp   = param0;
        action->opa_flags |= BCM_DS_OUT_ACTN_INSERT_DSCP;
    }

    /* bcmFieldActionRpDropCancel */
    color_actions[0] = bcmFieldActionYpDropCancel;
    color_actions[1] = bcmFieldActionGpDropCancel;
    rv = _bcm_ds_color_action_check(unit, entry, bcmFieldActionRpDropCancel,
                                    color_actions, 2, &param0, &param1);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        return rv;
    } else if (BCM_SUCCESS(rv)) {
        action->opa_flags |= BCM_DS_OUT_ACTN_DO_SWITCH;
    }

    /* bcmFieldActionRpDropPrecedence */
    color_actions[0] = bcmFieldActionYpDropPrecedence;
    color_actions[1] = bcmFieldActionGpDropPrecedence;
    rv = _bcm_ds_color_action_check(unit, entry, bcmFieldActionRpDropPrecedence,
                                    color_actions, 2, &param0, &param1);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        return rv;
    } else if (BCM_SUCCESS(rv)) {
        action->opa_flags |= BCM_DS_OUT_ACTN_DROP_PRECEDENCE;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_ds_field_group_process
 * Purpose:
 *      Recover rest of installed datapath and classifier information
 *      from the field module.
 * Parameters:
 *      unit      - Unit number.
 *      group     - FP group.
 *      user_data - User data.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_ds_field_group_process(int unit, bcm_field_group_t group,
                            void *user_data)
{
    int                 rv;
    int                 i;
    int                 match = FALSE;
    _dp_t               *dp = NULL, *dp_cur;
    _clfr_t             *clfr = NULL, *clfr_cur;
    bcm_ds_clfr_t       *cfe_clfr;
    bcm_field_qset_t    qset;
    bcm_field_entry_t   entry;
    int                 count;
    int                 prio;
    int                 port;
    int                 udf_arr[BCM_FIELD_USER_NUM_UDFS];

    /* Check if FP group is part of DiffServ */
    for (i = 0; (i < DPID_BUCKETS) && !match; i++) {
        dp_cur = ds_info[unit]->ds_dp_ptr[i];
        while ((dp_cur != NULL) && !match) {
            clfr_cur = dp_cur->dp_clfr;
            while ((clfr_cur != NULL) && (!match))  {
                if (clfr_cur->g == group) {
                    dp    = dp_cur;
                    clfr  = clfr_cur;
                    match = TRUE;  /* Found */
                    break;
                }
                clfr_cur = clfr_cur->cfe_next;
            }
            dp_cur = dp_cur->next;
        }
    }

    if (!match) {
        return BCM_E_NONE;    /* FP group does not belong to DiffServ */
    }

    /* Get field group qset */
    BCM_IF_ERROR_RETURN(bcm_esw_field_group_get(unit, group, &qset));

    /* A classifier creates only one field entry per group */
    BCM_IF_ERROR_RETURN(bcm_esw_field_entry_multi_get(unit, group, 1,
                                                      &entry, &count));
    if (count != 1) {
        return BCM_E_FAIL;
    }

    /* Fill information */
    clfr->qs = qset;
    clfr->e  = entry;

    cfe_clfr = &clfr->cfe_clfr;
    cfe_clfr->cf_flags = 0x0;

    /* Get precedence */
    if (BCM_SUCCESS(bcm_esw_field_group_priority_get(unit, group, &prio))) {
        clfr->cfe_precedence = prio;
        cfe_clfr->cf_precedence = prio;
    }

    /* Get field qualifiers data */
    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyDstPortTgid)) {
        bcm_port_t data, mask;

        dp->dp_flags |= BCM_DS_EGRESS;
        rv = bcm_esw_field_qualify_DstPortTgid_get(unit, entry, &data, &mask);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        } else if (BCM_SUCCESS(rv)) {
            BCM_PBMP_PORT_ADD(dp->dp_ports, data);
        }
    }

    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyInPorts)) {
        bcm_pbmp_t  mask;

        dp->dp_flags |= BCM_DS_INGRESS;
        rv = bcm_esw_field_qualify_InPorts_get(unit, entry,
                                               &dp->dp_ports, &mask);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        }
    }

    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyPacketFormat) ||
        BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyL2Format) || 
        BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyVlanFormat) ) {
        uint8                 data, mask;
        bcm_field_L2Format_t  l2_fmt;

        /*
         * The data for qualifier PacketFormat is translated into
         * the qualifiers L2Format, VlanFormat, and IpType by the
         * Field module.
         */
        cfe_clfr->cf_flags |= BCM_DS_CLFR_PKT_FORMAT;
        cfe_clfr->cf_pktfmt = 0;

        /* L2Format */
        rv = bcm_esw_field_qualify_L2Format_get(unit, entry, &l2_fmt);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        } else if (BCM_SUCCESS(rv)) {
            switch (l2_fmt) {
            case bcmFieldL2FormatEthII:
            case bcmFieldL2FormatAny:
                cfe_clfr->cf_pktfmt |= BCM_FIELD_PKT_FMT_L2_ETH_II;
                break;
            case bcmFieldL2FormatSnap:
                cfe_clfr->cf_pktfmt |= BCM_FIELD_PKT_FMT_L2_SNAP;
                break;
            case bcmFieldL2FormatLlc:
                cfe_clfr->cf_pktfmt |= BCM_FIELD_PKT_FMT_L2_LLC;
                cfe_clfr->cf_pktfmt |= BCM_FIELD_PKT_FMT_L2_802_3;
                break;
            default:
                break;
            }
        }

        /* VlanFormat */
        rv = bcm_esw_field_qualify_VlanFormat_get(unit, entry, &data, &mask);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        } else if (BCM_SUCCESS(rv)) {
            if (data != 0) {
                cfe_clfr->cf_pktfmt |= ((data << 2) &
                                        BCM_FIELD_PKT_FMT_TAGGED_MASK);
            }
        }

        /* IpType data has too many possibilities, skip */
    }

    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyDstMac)) {
        bcm_mac_t  mask;

        cfe_clfr->cf_flags |= BCM_DS_CLFR_DST_MAC_ADDR;        
        rv = bcm_esw_field_qualify_DstMac_get(unit, entry,
                                              &cfe_clfr->cf_dst_mac, &mask);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        }
    }

    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifySrcMac)) {
        bcm_mac_t  mask;

        cfe_clfr->cf_flags |= BCM_DS_CLFR_SRC_MAC_ADDR;
        rv = bcm_esw_field_qualify_SrcMac_get(unit, entry,
                                              &cfe_clfr->cf_src_mac, &mask);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        }
    }

    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyOuterVlan)) {
        bcm_vlan_t  mask;

        cfe_clfr->cf_flags |= BCM_DS_CLFR_VLAN_TAG;
        rv = bcm_esw_field_qualify_OuterVlan_get(unit, entry,
                                                 &cfe_clfr->cf_vid, &mask);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        }
    }

    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyEtherType)) {
        uint16  mask;

        cfe_clfr->cf_flags |= BCM_DS_CLFR_ETHER_TYPE;
        rv = bcm_esw_field_qualify_EtherType_get(unit, entry,
                                                 &cfe_clfr->cf_ethertype, &mask);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        }
    }

    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyDSCP)) {
        uint8  mask;

        cfe_clfr->cf_flags |= BCM_DS_CLFR_IPV4_CODEPOINT;
        rv = bcm_esw_field_qualify_DSCP_get(unit, entry,
                                            &cfe_clfr->cf_dscp, &mask);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        }
    }

    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyIpProtocol)) {
        uint8  mask;

        cfe_clfr->cf_flags |= BCM_DS_CLFR_IPV4_PROTOCOL;
        rv = bcm_esw_field_qualify_IpProtocol_get(unit, entry,
                                                  &cfe_clfr->cf_protocol, &mask);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        }
    }

    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifySrcIp)) {
        bcm_ip_t  mask;

        cfe_clfr->cf_flags |= BCM_DS_CLFR_IPV4_SRC_ADDR;
        rv = bcm_esw_field_qualify_SrcIp_get(unit, entry,
                                             &cfe_clfr->cf_src_ip_addr, &mask);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        } else if (BCM_SUCCESS(rv)) {
            cfe_clfr->cf_src_ip_prefix_len = bcm_ip_mask_length(mask);
        }
    }

    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyDstIp)) {
        bcm_ip_t  mask;

        cfe_clfr->cf_flags |= BCM_DS_CLFR_IPV4_DST_ADDR;
        rv = bcm_esw_field_qualify_DstIp_get(unit, entry,
                                             &cfe_clfr->cf_dst_ip_addr, &mask);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        } else if (BCM_SUCCESS(rv)) {
            cfe_clfr->cf_dst_ip_prefix_len = bcm_ip_mask_length(mask);
        }
    }

    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyL4SrcPort)) {
        bcm_l4_port_t  data, mask;

        cfe_clfr->cf_flags |= (BCM_DS_CLFR_IPV4_L4_SRC_PORT |
                               BCM_DS_CLFR_IPV4_L4_SRC_MASK);
        rv = bcm_esw_field_qualify_L4SrcPort_get(unit, entry, &data, &mask);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        } else if (BCM_SUCCESS(rv)) {
            cfe_clfr->cf_src_port = (uint16)data;
            cfe_clfr->cf_src_mask = (uint16)mask;
        }

    } else if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyRangeCheck)) {
        int  invert, count;

        cfe_clfr->cf_flags |= BCM_DS_CLFR_IPV4_L4_SRC_PORT;
        rv = bcm_esw_field_qualify_RangeCheck_get(unit, entry, 1,
                                                  &clfr->cfe_l4port_src_range,
                                                  &invert, &count);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        }
    }

    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyL4DstPort)) {
        bcm_l4_port_t  data, mask;

        cfe_clfr->cf_flags |= (BCM_DS_CLFR_IPV4_L4_DST_PORT |
                               BCM_DS_CLFR_IPV4_L4_DST_MASK);
        rv = bcm_esw_field_qualify_L4DstPort_get(unit, entry, &data, &mask);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        } else if (BCM_SUCCESS(rv)) {
            cfe_clfr->cf_dst_port = (uint16)data;
            cfe_clfr->cf_dst_mask = (uint16)mask;
        }

    } else if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyRangeCheck)) {
        int  invert, count;

        cfe_clfr->cf_flags |= BCM_DS_CLFR_IPV4_L4_DST_PORT;
        rv = bcm_esw_field_qualify_RangeCheck_get(unit, entry, 1,
                                                  &clfr->cfe_l4port_dst_range,
                                                  &invert, &count);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        }
    }

    if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyPacketRes)) {
        uint32  data, mask;

        rv = bcm_esw_field_qualify_PacketRes_get(unit, entry, &data, &mask);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        } else if (BCM_SUCCESS(rv)) {
            if (mask == BCM_FIELD_PKT_RES_L2UC) {
                if (data == BCM_FIELD_PKT_RES_L2UC) {
                    clfr->cfe_inp_actn.ipa_flags |=
                        BCM_DS_ACTN_SET_OUT_PORT_UCAST;
                } else if (data == 0) {
                    clfr->cfe_inp_actn.ipa_flags
                        |= BCM_DS_ACTN_SET_OUT_PORT_NON_UCAST;
                }
            }
        }
    }

    /* UserDefined */
    for (i = 0; i < BCM_FIELD_USER_NUM_UDFS; i++) {
        SHR_BITSET(qset.udf_map, i);
    }
    rv = bcm_esw_field_qset_udf_get(unit, &qset, BCM_FIELD_USER_NUM_UDFS,
                                    udf_arr, &count);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        return rv;
    }
    if (count > 0) {
        uint8 data[BCM_FIELD_USER_FIELD_SIZE];
        uint8 mask[BCM_FIELD_USER_FIELD_SIZE];
        bcm_field_udf_t  udf;

        udf = udf_arr[0];
        rv =  bcm_esw_field_qualify_UserDefined_get(unit, entry,
                                                    udf, data, mask);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        } else if (BCM_SUCCESS(rv)) {
            ds_info[unit]->udf_id = udf;
            if (data[0] & 0xf0) {
                cfe_clfr->cf_ip_version = data[0] >> 4;
                cfe_clfr->cf_flags |= BCM_DS_CLFR_IP_VERSION;
            }
            if (data[0] & 0x0f) {
                cfe_clfr->cf_ip_hlen = data[0] & 0x0f;
                cfe_clfr->cf_flags |= BCM_DS_CLFR_IP_HLEN;
            }
            SHR_BITSET(clfr->qs.udf_map, udf);
        }
    }

    /* Get scheduler */
    if ((dp->dp_flags & BCM_DS_EGRESS) && !(dp->dp_flags & BCM_DS_INGRESS)) {
        int mode, delay;
        int weight[BCM_COS_COUNT];

        rv = bcm_esw_cosq_sched_get(unit, &mode, weight, &delay);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        } else if (BCM_SUCCESS(rv)) {
            if (mode == BCM_COSQ_WEIGHTED_ROUND_ROBIN) {
                sal_memcpy(dp->dp_scheduler.weight, weight, sizeof(weight));
                bcm_esw_cosq_config_get(unit, &dp->dp_scheduler.numq);
            }
        }
    }

    PBMP_ITER(dp->dp_ports, port) {
        clfr->cfe_ds_port = port;
    }

    /* Recover in and out profile actions */
    BCM_IF_ERROR_RETURN(_bcm_ds_in_profile_action_reinit(unit, clfr));
    BCM_IF_ERROR_RETURN(_bcm_ds_out_profile_action_reinit(unit, clfr));

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_ds_clfr_reinit
 * Purpose:
 *      Recover and recreate classifier from given scache pointer.
 * Parameters:
 *      unit          - Unit number.
 *      ds_scache_ptr - Scache pointer.
 *      dp            - Datapath for classifier.
 *      rv            - Return value BCM_E_XXX.
 * Returns:
 *      Scache pointer
 * Notes:
 *      Only installed datapath/classifiers will be restored.
 *
 *      Supported only for XGS-3 and newer devices.
 */
STATIC uint8 *
_bcm_ds_clfr_reinit(int unit, uint8 *ds_scache_ptr, _dp_t *dp, int *rv)
{
    _clfr_t   *clfr;
    _clfr_t   *cur_ptr;
    uint16    group;
    int       idx;
    int       free_cfid;

    *rv = BCM_E_NONE;

    /* Recover FP group id */
    sal_memcpy(&group, ds_scache_ptr, sizeof(group));
    ds_scache_ptr += sizeof(group);

    /* Find the next free cfid */
    free_cfid = (dp->dp_next_cfid)++;
    for (idx = 0; idx < CFID_MAX_COUNT; idx++) {
        if (_ds_classifier_find(unit, dp, free_cfid) == NULL) {
            break; /* Found free ID */
        }

        free_cfid = (dp->dp_next_cfid)++;
    }
    if (idx == CFID_MAX_COUNT) {
        *rv = BCM_E_RESOURCE;
        return ds_scache_ptr;
    }

    /* Create classifier */
    if ((clfr = sal_alloc(sizeof(_clfr_t), ds_string)) == NULL) {
        *rv = BCM_E_MEMORY;
        return ds_scache_ptr;
    }
    sal_memset(clfr, 0, sizeof(_clfr_t));

    /*
     * Insert classifier into datapath as last entry.
     * Classifiers were stored in the same order as the link list
     * (sorted by precedence), so there is no need to sort again.
     */
    if (dp->dp_clfr == NULL) {
        dp->dp_clfr = clfr;
    } else {
        cur_ptr = dp->dp_clfr;
        while(cur_ptr->cfe_next != NULL) {
            cur_ptr = cur_ptr->cfe_next;
        }
        cur_ptr->cfe_next  = clfr;
        clfr->cfe_prev = cur_ptr;
    }

    clfr->cfe_dpid = dp->dpid;
    clfr->cfe_cfid = free_cfid;
    clfr->cfe_installed = TRUE;
    clfr->g = group;

    /* Rest of information will be recovered from the field traverse */

    return ds_scache_ptr;
}

/*
 * Function:
 *      _bcm_ds_dp_reinit
 * Purpose:
 *      Recover and recreate datapath from given scache pointer.
 * Parameters:
 *      unit          - Unit number.
 *      ds_scache_ptr - Scache pointer.
 *      rv            - Return value BCM_E_XXX.
 * Returns:
 *      Scache pointer
 * Notes:
 *      Only installed datapath/classifiers will be restored.
 *
 *      Supported only for XGS-3 and newer devices.
 */
STATIC uint8 *
_bcm_ds_dp_reinit(int unit, uint8 *ds_scache_ptr, int *rv)
{
    int         i;
    uint16      num_clfr;
    int         idx;
    uint32      flags;
    bcm_pbmp_t  ports;
    _dp_t       *dp;

    *rv = BCM_E_NONE;

    flags = 0x0;
    BCM_PBMP_CLEAR(ports);

    /* Add datapath */
    for (idx = 0; idx < DPID_MAX_COUNT /* and free ID not found */ ; idx++) {
        if (_bcm_dpid_find(unit, idx) == NULL) {  /* Found a free dpid */
            *rv = _dpid_add_update(unit, idx, flags, ports);
            break;  /* Free ID found */
        }
    }
    if (idx == DPID_MAX_COUNT) {
        *rv = BCM_E_RESOURCE;
    }
    if (BCM_FAILURE(*rv)) {
        return ds_scache_ptr;
    }

    if ((dp = _bcm_dpid_find(unit, idx)) == NULL) {
        return ds_scache_ptr;
    }

    dp->dp_installed = TRUE;

    /* Number of classifiers */
    sal_memcpy(&num_clfr, ds_scache_ptr, sizeof(num_clfr));
    ds_scache_ptr += sizeof(num_clfr);

    /* Classifiers */
    for (i = 0; i < num_clfr; i++) {
        ds_scache_ptr = _bcm_ds_clfr_reinit(unit, ds_scache_ptr, dp, rv);
        if (BCM_FAILURE(*rv)) {
            return ds_scache_ptr;
        }
    }

    /* Rest of information will be recovered from the field traverse */

    return ds_scache_ptr;
}
#endif /* BCM_FIELD_SUPPORT */

/*
 * Function:
 *      _bcm_ds_reinit
 * Purpose:
 *      Recover DiffServ software state.
 * Parameters:
 *      unit - Unit number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Warm Boot level-2 is required in order to find out
 *      the datapath for a given classifier (fp group).
 *
 *      Only installed datapath/classifiers will be restored.
 *
 *      Supported only for XGS-3 and newer devices.
 */
STATIC int
_bcm_ds_reinit(int unit)
{
#if defined(BCM_FIELD_SUPPORT)
    int                 rv = BCM_E_NONE;
    soc_scache_handle_t scache_handle;
    uint8               *ds_scache_ptr;
    int                 i;
    uint16              num_dp;

    if (!soc_feature(unit, soc_feature_field)) {
        return BCM_E_NONE;
    }

    if (SOC_WARM_BOOT_SCACHE_IS_LIMITED(unit) || (_ds_scache_size == 0)) {
        return BCM_E_NONE;
    }

    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_DIFFSERV, 0);
    rv = _bcm_esw_scache_ptr_get(unit, scache_handle, FALSE,
                                 0, &ds_scache_ptr, 
                                 BCM_WB_DEFAULT_VERSION, NULL);

    if (BCM_FAILURE(rv)) {
        if (rv == BCM_E_NOT_FOUND) {
            rv = BCM_E_NONE;
        }
        return rv;
    }

    /* Datapaths */
    sal_memcpy(&num_dp, ds_scache_ptr, sizeof(num_dp));
    ds_scache_ptr += sizeof(num_dp);
    for (i = 0; i < num_dp; i++) {
        ds_scache_ptr = _bcm_ds_dp_reinit(unit, ds_scache_ptr, &rv);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    }

    /* Recover rest of information from installed FP groups and entries */
    BCM_IF_ERROR_RETURN
        (bcm_esw_field_group_traverse(unit, 
                                      _bcm_ds_field_group_process, NULL));

#endif /* BCM_FIELD_SUPPORT */

    return BCM_E_NONE;
}
#endif /* BCM_WARM_BOOT_SUPPORT */


#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP

#if defined(BCM_FIELD_SUPPORT)
#define MAC_FMT         "0x%02x%02x%02x%02x%02x%02x"
#define MAC_DISP(_mac)  _mac[0], _mac[1], _mac[2], _mac[3], _mac[4], _mac[5]

#define IP_FMT         "%d.%d.%d.%d"
#define IP_DISP(_ip)   (_ip >> 24) & 0xff, (_ip >> 16) & 0xff, \
                       (_ip >> 8) & 0xff, _ip & 0xff
/*
 * Function:
 *     _bcm_ds_..._sw_dump
 * Purpose:
 *     Set of helper routines to display DiffServ information
 *     maintained by software.
 */
STATIC void
_bcm_ds_cfe_cflr_sw_dump(bcm_ds_clfr_t *clfr)
{
    soc_cm_print("          Cfe Classifier\n");
    soc_cm_print("              flags            : 0x%x\n", clfr->cf_flags);
    soc_cm_print("              pktfmt           : %d\n", clfr->cf_pktfmt);
    soc_cm_print("              dst_mac          : " MAC_FMT "\n",
                 MAC_DISP(clfr->cf_dst_mac));
    soc_cm_print("              src_mac          : " MAC_FMT "\n",
                 MAC_DISP(clfr->cf_src_mac));
    soc_cm_print("              vid              : %d\n", clfr->cf_vid);
    soc_cm_print("              ethertype        : %d\n", clfr->cf_ethertype);
    soc_cm_print("              ip_version       : %d\n", clfr->cf_ip_version);
    soc_cm_print("              ip_hlen          : %d\n", clfr->cf_ip_hlen);
    soc_cm_print("              dscp             : %d\n", clfr->cf_dscp);
    soc_cm_print("              protocol         : %d\n", clfr->cf_protocol);
    soc_cm_print("              src_ip_addr      : " IP_FMT "\n",
                 IP_DISP(clfr->cf_src_ip_addr));
    soc_cm_print("              src_ip_prefix_len: %d\n",
                 clfr->cf_src_ip_prefix_len);
    soc_cm_print("              dst_ip_addr      : " IP_FMT "\n",
                 IP_DISP(clfr->cf_dst_ip_addr));
    soc_cm_print("              dst_ip_prefix_len: %d\n",
                 clfr->cf_dst_ip_prefix_len);
    soc_cm_print("              src_port_min     : %d\n", clfr->cf_src_port_min);
    soc_cm_print("              src_port_max     : %d\n", clfr->cf_src_port_max);
    soc_cm_print("              dst_port_min     : %d\n", clfr->cf_dst_port_min);
    soc_cm_print("              dst_port_max     : %d\n", clfr->cf_dst_port_max);
    soc_cm_print("              src_port         : %d\n", clfr->cf_src_port);
    soc_cm_print("              src_mask         : 0x%x\n", clfr->cf_src_mask);
    soc_cm_print("              dst_port         : %d\n", clfr->cf_dst_port);
    soc_cm_print("              dst_mask         : 0x%x\n", clfr->cf_dst_mask);
    soc_cm_print("              precedence       : %d\n", clfr->cf_precedence);

    return;
}

STATIC void
_bcm_ds_inprofile_actn_sw_dump(bcm_ds_inprofile_actn_t *actn)
{
    soc_cm_print("          In Profile Action\n");
    soc_cm_print("              flags: 0x%x\n", actn->ipa_flags);
    soc_cm_print("              pri  : %d\n", actn->ipa_pri);
    soc_cm_print("              tos  : %d\n", actn->ipa_tos);
    soc_cm_print("              port : %d\n", actn->ipa_port);
    soc_cm_print("              dscp : %d\n", actn->ipa_dscp);
    soc_cm_print("              mod  : %d\n", actn->ipa_mod);

    return;
}

STATIC void
_bcm_ds_outprofile_actn_sw_dump(bcm_ds_outprofile_actn_t *actn)
{
    soc_cm_print("          Out Profile Action\n");
    soc_cm_print("              flags      : 0x%x\n", actn->opa_flags);
    soc_cm_print("              dscp       : %d\n", actn->opa_dscp);
    soc_cm_print("              kbits_sec  : %d\n", actn->opa_kbits_sec);
    soc_cm_print("              kbits_burst: %d\n", actn->opa_kbits_burst);
    
    return;
}

STATIC void
_bcm_ds_nomatch_actn_sw_dump(bcm_ds_nomatch_actn_t *actn)
{
    soc_cm_print("          No Match Action\n");
    soc_cm_print("              flags: 0x%x\n", actn->nma_flags);
    soc_cm_print("              pri  : %d\n", actn->nma_pri);
    soc_cm_print("              tos  : %d\n", actn->nma_tos);
    soc_cm_print("              port : %d\n", actn->nma_port);
    soc_cm_print("              dscp : %d\n", actn->nma_dscp);
    soc_cm_print("              mod  : %d\n", actn->nma_mod);

    return;
}

STATIC void
_bcm_ds_clfr_sw_dump(_clfr_t *clfr)
{
    int  i;

    soc_cm_print("      Classifier\n");
    soc_cm_print("          dpid     : %d\n", clfr->cfe_dpid);
    soc_cm_print("          cfid     : %d\n", clfr->cfe_cfid);
    soc_cm_print("          installed: %d\n", clfr->cfe_installed);
    soc_cm_print("          qset     :");
    for (i = 0; i < COUNTOF(clfr->qs.w); i++) {
        if ((i > 0) && (!(i % 4)))  {
            soc_cm_print("\n                    ");
        }
        soc_cm_print(" 0x%8.8x", clfr->qs.w[i]);
    }
    soc_cm_print("\n");
    soc_cm_print("          qset udf :");
    for (i = 0; i < COUNTOF(clfr->qs.udf_map); i++) {
        if ((i > 0) && (!(i % 4)))  {
            soc_cm_print("\n                   ");
        }
        soc_cm_print(" 0x%8.8x", clfr->qs.udf_map[i]);
    }
    soc_cm_print("\n");
    soc_cm_print("          group    : %d\n", clfr->g);
    soc_cm_print("          entry    : %d\n", clfr->e);
    soc_cm_print("          l4_port_src_range: %d\n",
                 clfr->cfe_l4port_src_range);
    soc_cm_print("          l4_port_dst_range: %d\n",
                 clfr->cfe_l4port_dst_range);
    soc_cm_print("          qset no-match    :");
    for (i = 0; i < COUNTOF(clfr->qs_nm.w); i++) {
        if ((i > 0) && (!(i % 4)))  {
            soc_cm_print("\n                            ");
        }
        soc_cm_print(" 0x%8.8x", clfr->qs_nm.w[i]);
    }
    soc_cm_print("\n");
    soc_cm_print("          qset no-match udf:");
    for (i = 0; i < COUNTOF(clfr->qs_nm.udf_map); i++) {
        if ((i > 0) && (!(i % 4)))  {
            soc_cm_print("\n                            ");
        }
        soc_cm_print(" 0x%8.8x", clfr->qs_nm.udf_map[i]);
    }
    soc_cm_print("\n");
    soc_cm_print("          no-match group   : %d\n", clfr->g_nm);
    soc_cm_print("          no-match entry   : %d\n", clfr->e_nm);
    soc_cm_print("          precedence   : %d\n", clfr->cfe_precedence);
    soc_cm_print("          ds port      : %d\n", clfr->cfe_ds_port);
    soc_cm_print("          L4 port flags: %d\n", clfr->cfe_l4port_flags);

    _bcm_ds_cfe_cflr_sw_dump(&clfr->cfe_clfr);
    _bcm_ds_inprofile_actn_sw_dump(&clfr->cfe_inp_actn);
    _bcm_ds_outprofile_actn_sw_dump(&clfr->cfe_outp_actn);
    _bcm_ds_nomatch_actn_sw_dump(&clfr->cfe_nm_actn);

    return;
}

STATIC void
_bcm_ds_dp_sw_dump(_dp_t *dp)
{
    int      i;
    char     pfmt[SOC_PBMP_FMT_LEN];
    _clfr_t  *clfr;

    soc_cm_print("\n  Datapath\n");
    soc_cm_print("      dpid     : %d\n", dp->dpid);
    soc_cm_print("      ports    : %s\n", SOC_PBMP_FMT(dp->dp_ports, pfmt));
    soc_cm_print("      flags    : 0x%x\n", dp->dp_flags);
    soc_cm_print("      installed: %d\n", dp->dp_installed);
    soc_cm_print("      next_cfid: %d\n", dp->dp_next_cfid);

    soc_cm_print("      Scheduler\n");
    soc_cm_print("          number COSQs: %d\n",
                 dp->dp_scheduler.numq);
    soc_cm_print("          weight      :");
    for (i = 0; i < COUNTOF(dp->dp_scheduler.weight); i++) {
        soc_cm_print(" %d", dp->dp_scheduler.weight[i]);
    }
    soc_cm_print("\n");

    clfr = dp->dp_clfr;
    while (clfr != NULL) {
        _bcm_ds_clfr_sw_dump(clfr);
        clfr = clfr->cfe_next;
    }
  
    return;
}
#endif /* BCM_FIELD_SUPPORT */

/*
 * Function:
 *     _bcm_ds_sw_dump
 * Purpose:
 *     Displays DiffServ information maintained by software.
 * Parameters:
 *     unit - Device unit number
 * Returns:
 *     None
 */
void
_bcm_ds_sw_dump(int unit)
{
#if defined(BCM_FIELD_SUPPORT)
    int    i;
    _dp_t  *dp;

    soc_cm_print("\nSW Information DiffServ - Unit %d\n\n", unit);

    if (ds_info[unit] != NULL) {
        soc_cm_print("  max   : %d\n", ds_info[unit]->ds_mib_max);
        soc_cm_print("  count : %d\n", ds_info[unit]->ds_mib_count);
        soc_cm_print("  udf_id: %d\n", ds_info[unit]->udf_id);

        soc_cm_print("  mib   - dpid    cfid\n");
        if (ds_info[unit]->ds_mib != NULL) {
            for (i = 0; i < DS_MIB_SIZE; i++) {
                if((ds_info[unit]->ds_mib[i].ds_mibid == (void *)-1) &&
                   (ds_info[unit]->ds_mib[i].ds_dpid == -1) &&
                   (ds_info[unit]->ds_mib[i].ds_cfid == -1)) {
                    continue;
                }

                /* Skip user data 'ds_mibid' */
                soc_cm_print("          %4d  %6d\n",
                             ds_info[unit]->ds_mib[i].ds_dpid,
                             ds_info[unit]->ds_mib[i].ds_cfid);
            }
        }

        /* Datapath */
        for (i = 0; i < DPID_BUCKETS; i++) {
            dp = ds_info[unit]->ds_dp_ptr[i];
            while (dp != NULL) {
                _bcm_ds_dp_sw_dump(dp);
                dp = dp->next;
            }
        }
    }
#endif /* BCM_FIELD_SUPPORT */

    return;
}
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */
