/*
 * $Id: diffserv.c 1.22 Broadcom SDK $
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
 * File:        bcmx/diffserv.c
 * Purpose:     BCMX Differentiated Services APIs
 *
 * The application must specify the datapath ID and classifier ID on
 * create calls.
 *
 * Create calls only go out to the units with ports in the given
 * port list.  To keep classifier calls reasonable, we locally keep
 * track of the units on a per-id basis.
 */

#include <assert.h>

#include <shared/alloc.h>
#include <sal/core/libc.h>

#include <bcm/types.h>

#include <bcmx/diffserv.h>
#include <bcmx/lport.h>
#include <bcmx/bcmx.h>
#include <bcmx/lplist.h>

#include "bcmx_int.h"

static sal_mutex_t bcmx_ds_mutex;

#define BCMX_DS_LOCK       sal_mutex_take(bcmx_ds_mutex, sal_mutex_FOREVER)
#define BCMX_DS_UNLOCK     sal_mutex_give(bcmx_ds_mutex)

#define BCMX_DS_INIT    do {                                          \
    BCMX_READY_CHECK;                                                 \
    if (bcmx_ds_mutex == NULL &&                                      \
            (bcmx_ds_mutex = sal_mutex_create("bcmx_ds")) == NULL)    \
        return BCM_E_MEMORY;                                          \
    } while (0)


#define BCMX_DS_ERROR_CHECK(_unit, _check, _rv)    \
    BCMX_ERROR_CHECK(_unit, _check, _rv)

#define BCMX_DS_SET_ERROR_CHECK(_unit, _check, _rv)    \
    BCMX_SET_ERROR_CHECK(_unit, _check, _rv)

#define BCMX_DS_DELETE_ERROR_CHECK(_unit, _check, _rv)    \
    BCMX_DELETE_ERROR_CHECK(_unit, _check, _rv)

#define BCMX_DS_GET_IS_VALID(_unit, _rv)    \
    BCMX_ERROR_IS_VALID(_unit, _rv)

/*
 * Data path IDs are hashed to a bucket rather than
 * indexed out a fixed array.  Each bucket is a linked list.
 * This is the number of buckets.
 */

#define DPID_BUCKETS            64
#define DPID_HASH(_dpid)        ((_dpid) % DPID_BUCKETS)

typedef struct _ds_unit_s {
    int dpid;
    int next_cfid;  /* See notes in classifier_create */
    struct _ds_unit_s *next;
    SHR_BITDCLNAME(units, BCM_CONTROL_MAX);
} _ds_unit_t;

#define FOREACH_DS_UNIT(_dsu, _unit, _i)                              \
    BCMX_UNIT_ITER(_unit, _i)                                         \
        if (SHR_BITGET((_dsu)->units, _unit) != 0)                    \

#define DSU_GET(_dpid, _dsu) do {                                     \
        _dsu = _bcmx_dpid_find(_dpid);                                     \
        if (_dsu == NULL) {                                           \
            BCMX_DS_UNLOCK;                                           \
            return BCM_E_NOT_FOUND;                                   \
        }                                                             \
    } while (0)


_ds_unit_t *_ds_unit_buckets[DPID_BUCKETS];

STATIC INLINE _ds_unit_t *
_bcmx_dpid_find(int dpid)
{
    _ds_unit_t *dsu;

    dsu = _ds_unit_buckets[DPID_HASH(dpid)];
    while (dsu != NULL && dsu->dpid != dpid) {
        dsu = dsu->next;
    }

    return dsu;
}

/* If exists, return current one */
STATIC INLINE _ds_unit_t *
_bcmx_dpid_find_add(int dpid)
{
    _ds_unit_t *dsu;
    int hash;

    hash = DPID_HASH(dpid);
    dsu = _bcmx_dpid_find(dpid);
    if (dsu != NULL) {
        return dsu;
    }

    dsu = sal_alloc(sizeof(_ds_unit_t), "bcmx_dpid_add");
    if (dsu == NULL) {
        return NULL;
    }
    sal_memset(dsu, 0, sizeof(*dsu));
    dsu->dpid = dpid;
    dsu->next = _ds_unit_buckets[hash];
    _ds_unit_buckets[hash] = dsu;

    return dsu;
}

#define DPID_SEARCH_MAX 10000 /* To put a bound on loop below */

/* Assumes lock is held; allocates an available DPID. */
STATIC INLINE _ds_unit_t *
_bcmx_dpid_create(void)
{
    int dpid;

    for (dpid = 1; dpid < DPID_SEARCH_MAX; dpid++) {
        if (_bcmx_dpid_find(dpid) == NULL) {
            break;
        }
    }

    if (dpid >= DPID_SEARCH_MAX) {
        return NULL;
    }

    return _bcmx_dpid_find_add(dpid);
}

STATIC INLINE void
_bcmx_dpid_remove(int dpid)
{
    _ds_unit_t *dsu, *tmp_dsu;
    int hash;

    hash = DPID_HASH(dpid);
    dsu = _ds_unit_buckets[hash];
    if (dsu == NULL) {     /* Empty bucket */
        return;
    }

    if (dsu->dpid == dpid) {    /* First in list */
        _ds_unit_buckets[hash] = dsu->next;
        sal_free(dsu);
        return;
    }

    while (dsu->next != NULL && dsu->next->dpid != dpid) {
        dsu = dsu->next;
    }

    if (dsu->next == NULL) {   /* Not in list */
        return;
    }

    assert(dsu->next != NULL && dsu->next->dpid == dpid);
    tmp_dsu = dsu->next;
    dsu->next = tmp_dsu->next;
    sal_free(tmp_dsu);
}

/*
 * Function:
 *      bcmx_ds_init
 */

int
bcmx_ds_init(void)
{
    int rv = BCM_E_UNAVAIL;
    int tmp_rv;
    int i, bcm_unit, cleared, dpid;

    BCMX_READY_CHECK;

    do {
        cleared = 0;
        for (i = 0; i < DPID_BUCKETS; i++) {
            if (_ds_unit_buckets[i] == NULL) {
                continue;
            }
            dpid = _ds_unit_buckets[i]->dpid;
            _bcmx_dpid_remove(dpid);
            cleared += 1;
        }
    } while (cleared > 0);

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_ds_init(bcm_unit);
        BCM_IF_ERROR_RETURN(BCMX_DS_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

STATIC INLINE int
_dp_create(_ds_unit_t *dsu, uint32 flags, bcmx_lplist_t plist)
{
    int dpid;
    int added_unit = FALSE;
    int rv = BCM_E_UNAVAIL;
    int tmp_rv;
    int i, bcm_unit;
    bcm_pbmp_t pbmp;

    dpid = dsu->dpid;
    BCMX_UNIT_ITER(bcm_unit, i) {
        BCMX_LPLIST_TO_PBMP(plist, bcm_unit, pbmp);
        if (BCM_PBMP_NOT_NULL(pbmp)) {
            tmp_rv = bcm_ds_datapath_create_id(bcm_unit, flags, pbmp, dpid);
            if (tmp_rv == BCM_E_NONE) {  /* Indicate successful dp create */
                added_unit = TRUE;
                SHR_BITSET(dsu->units, bcm_unit);
            }
            if (BCM_FAILURE(BCMX_DS_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv))) {
                break;
            }
        }
    }

    if (!added_unit) {
        _bcmx_dpid_remove(dpid);
    }

    return rv;
}


/*
 * Function:
 *      bcmx_ds_datapath_create
 * Notes
 *       Attempt to create for all units.
 *       On error, caller must unwind with datapath_delete if desired.
 *
 *       The DS lock is taken exactly when dsu != NULL
 * Returns
 *      Errors may mean some units were added before the failure.
 */

int
bcmx_ds_datapath_create(int dpid, uint32 flags, bcmx_lplist_t plist)
{
    int rv = BCM_E_NONE;
    _ds_unit_t *dsu = NULL;

    BCMX_DS_INIT;
    BCMX_DS_LOCK;

    /* Check if already exists */
    dsu = _bcmx_dpid_find(dpid);
    if (dsu != NULL) {
        BCMX_DS_UNLOCK;
        return BCM_E_NONE;
    }

    /* Okay, try to create a new entry */
    dsu = _bcmx_dpid_find_add(dpid);
    if (dsu == NULL)  {
        BCMX_DS_UNLOCK;
        return BCM_E_MEMORY;
    }

    rv = _dp_create(dsu, flags, plist);
    BCMX_DS_UNLOCK;

    return rv;
}


/*
 * Function:
 *     bcmx_ds_datapath_alloc_create
 * Notes
 *       Like bcmx_ds_datapath_create, but will allocate a new DPID
 *       return that as an output parameter.
 *
 *       Attempt to create for all units.
 *       On error, caller must unwind with datapath_delete if desired.
 *
 *       The DS lock is taken exactly when dsu != NULL
 * Returns
 *      Errors may mean some units were added before the failure.
 */

int
bcmx_ds_datapath_alloc_create(int *dpid,
                              uint32 flags,
                              bcmx_lplist_t plist)
{
    int rv = BCM_E_NONE;
    _ds_unit_t *dsu = NULL;

    BCMX_DS_INIT;
    BCMX_PARAM_NULL_CHECK(dpid);

    BCMX_DS_LOCK;

    /* Create new dsu */
    dsu = _bcmx_dpid_create();
    if (dsu == NULL) {
        BCMX_DS_UNLOCK;
        return BCM_E_RESOURCE;
    }

    rv = _dp_create(dsu, flags, plist);
    *dpid = dsu->dpid;

    BCMX_DS_UNLOCK;

    return rv;
}

/*
 * Function
 *      bcmx_ds_datapath_unit_count
 * Purpose
 *      Returns the number of units associated with the given DPID,
 *      or -1 if the DPID is not known.
 * Notes
 *      Can be used after a failed create call to determine if any
 *      units had the DP created successsfully.
 */

int
bcmx_ds_datapath_unit_count(int dpid)
{
    int count = 0;
    int unit;
    _ds_unit_t *dsu;
    int i;

    BCMX_READY_CHECK;

    dsu = _bcmx_dpid_find(dpid);
    if (dsu == NULL) {
        return -1;
    }

    /* This could probably be more efficient */
    FOREACH_DS_UNIT(dsu, unit, i) {
        ++count;
    }

    return count;
}

/*
 * Function:
 *      bcmx_ds_datapath_delete
 * Notes
 *      To allow for better clean up, call BCM delete on all known
 *      units, ignoring not found and unavail errors on fabrics.
 */

int
bcmx_ds_datapath_delete(int dpid)
{
    int rv = BCM_E_UNAVAIL;
    int i, bcm_unit;
    int tmp_rv;

    BCMX_DS_INIT;
    BCMX_DS_LOCK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_ds_datapath_delete(bcm_unit, dpid);
        if (BCM_FAILURE(BCMX_DS_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv))) {
            break;
        }
    }

    _bcmx_dpid_remove(dpid);
    BCMX_DS_UNLOCK;
    return rv;
}


/*
 * Function:
 *      bcmx_ds_datapath_install
 */

int
bcmx_ds_datapath_install(int dpid)
{
    int rv = BCM_E_UNAVAIL;
    int i, bcm_unit;
    int tmp_rv;
    _ds_unit_t *dsu;

    BCMX_DS_INIT;
    BCMX_DS_LOCK;

    DSU_GET(dpid, dsu);
    FOREACH_DS_UNIT(dsu, bcm_unit, i) {
        tmp_rv = bcm_ds_datapath_install(bcm_unit, dpid);
        if (BCM_FAILURE(BCMX_DS_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv))) {
            break;
        }
    }

    BCMX_DS_UNLOCK;
    return rv;
}


/*
 * Function:
 *      bcmx_ds_classifier_create
 * Notes:
 *      For a given DPID, only one of classifier_create or
 *      classifier_alloc_create may be called.  This is controlled
 *      by the value of next_cfid.  When the DP is initialized,
 *      next_cfid has a value of 0.  If classifier_create is
 *      called, it is set to -1.  If classifier_alloc_create is
 *      called, it is incremented.
 */

int
bcmx_ds_classifier_create(int dpid,
                          int cfid,
                          bcm_ds_clfr_t *clfr,
                          bcm_ds_inprofile_actn_t *inp_actn,
                          bcm_ds_outprofile_actn_t *outp_actn,
                          bcm_ds_nomatch_actn_t *nm_actn)
{
    int rv = BCM_E_UNAVAIL;
    int i, bcm_unit;
    int tmp_rv;
    _ds_unit_t *dsu;
    bcm_ds_inprofile_actn_t xinp_actn, *pxinp_actn;
    bcm_ds_nomatch_actn_t xnm_actn, *pxnm_actn;
    bcmx_lport_t lport;
    bcm_module_t modid;
    bcm_port_t modport;

    BCMX_DS_INIT;
    BCMX_DS_LOCK;

    DSU_GET(dpid, dsu);
    switch (dsu->next_cfid) {
    case -1:
        break;
    case 0:
        dsu->next_cfid = -1;
        break;
    default:  /* Already had classifier_alloc_create called */
        BCMX_DS_UNLOCK;
        return BCM_E_PARAM;
        break;
    }

    if (inp_actn != NULL) {
        xinp_actn = *inp_actn;
        pxinp_actn = &xinp_actn;
    } else {
        pxinp_actn = NULL;
    }
    if (nm_actn != NULL) {
        xnm_actn = *nm_actn;
        pxnm_actn = &xnm_actn;
    } else {
        pxnm_actn = NULL;
    }

    FOREACH_DS_UNIT(dsu, bcm_unit, i) {
        if (pxinp_actn != NULL &&
            (inp_actn->ipa_flags & (BCM_DS_ACTN_SET_OUT_PORT_UCAST |
                                    BCM_DS_ACTN_SET_OUT_PORT_NON_UCAST |
                                    BCM_DS_ACTN_SET_OUT_PORT_ALL))) {
            lport = inp_actn->ipa_port;
            if (lport == BCMX_LPORT_LOCAL_CPU) {
                lport = BCMX_LPORT_LOCAL_CPU_GET(bcm_unit);
            }
            if (BCM_FAILURE
                (_bcmx_dest_to_modid_port(lport, &modid, &modport,
                                          BCMX_DEST_CONVERT_DEFAULT))) {
                continue;
            }
            xinp_actn.ipa_port = (uint32) modport;
            xinp_actn.ipa_mod = (uint32) modid;
            xinp_actn.ipa_flags |= BCM_DS_ACTN_SET_OUT_MODULE;
        }
        if (pxnm_actn != NULL &&
            (nm_actn->nma_flags & (BCM_DS_NM_ACTN_SET_OUT_PORT_UCAST |
                                   BCM_DS_NM_ACTN_SET_OUT_PORT_NON_UCAST |
                                   BCM_DS_NM_ACTN_SET_OUT_PORT_ALL))) {
            lport = nm_actn->nma_port;
            if (lport == BCMX_LPORT_LOCAL_CPU) {
                lport = BCMX_LPORT_LOCAL_CPU_GET(bcm_unit);
            }
            if (BCM_FAILURE
                (_bcmx_dest_to_modid_port(lport, &modid, &modport,
                                          BCMX_DEST_CONVERT_DEFAULT))) {
                continue;
            }
            xnm_actn.nma_port = (uint32) modport;
            xnm_actn.nma_mod = (uint32) modid;
            xnm_actn.nma_flags |= BCM_DS_NM_ACTN_SET_OUT_MODULE;
        }
        tmp_rv = bcm_ds_classifier_id_create(bcm_unit, dpid, clfr,
                                             pxinp_actn,
                                             outp_actn,
                                             pxnm_actn,
                                             cfid);
        if (BCM_FAILURE(BCMX_DS_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv))) {
            break;
        }
    }

    BCMX_DS_UNLOCK;
    return rv;
}

/*
 * Function:
 *      bcmx_ds_classifier_alloc_create
 * Notes:
 *      Like bcmx_ds_classifier_create, but allocate a CFID.
 *
 *      For a given DPID, only one of classifier_create or
 *      classifier_alloc_create may be called.  This is controlled
 *      by the value of next_cfid.  When the DP is initialized,
 *      next_cfid has a value of 0.  If classifier_create is
 *      called, it is set to -1.  If classifier_alloc_create is
 *      called, it is incremented.
 */

int
bcmx_ds_classifier_alloc_create(int dpid,
                                int *cfid,
                                bcm_ds_clfr_t *clfr,
                                bcm_ds_inprofile_actn_t *inp_actn,
                                bcm_ds_outprofile_actn_t *outp_actn,
                                bcm_ds_nomatch_actn_t *nm_actn)
{
    int rv = BCM_E_UNAVAIL;
    int i, bcm_unit;
    int tmp_rv;
    _ds_unit_t *dsu;
    bcm_ds_inprofile_actn_t xinp_actn, *pxinp_actn;
    bcm_ds_nomatch_actn_t xnm_actn, *pxnm_actn;
    bcmx_lport_t lport;
    bcm_module_t modid;
    bcm_port_t modport;

    BCMX_DS_INIT;
    BCMX_DS_LOCK;

    DSU_GET(dpid, dsu);
    if (dsu->next_cfid == -1) {  /* Already had classifier_create called */
        BCMX_DS_UNLOCK;
        return BCM_E_PARAM;
    }

    *cfid = (dsu->next_cfid)++;

    if (inp_actn != NULL) {
        xinp_actn = *inp_actn;
        pxinp_actn = &xinp_actn;
    } else {
        pxinp_actn = NULL;
    }
    if (nm_actn != NULL) {
        xnm_actn = *nm_actn;
        pxnm_actn = &xnm_actn;
    } else {
        pxnm_actn = NULL;
    }

    FOREACH_DS_UNIT(dsu, bcm_unit, i) {
        if (pxinp_actn != NULL &&
            (inp_actn->ipa_flags & (BCM_DS_ACTN_SET_OUT_PORT_UCAST |
                                    BCM_DS_ACTN_SET_OUT_PORT_NON_UCAST |
                                    BCM_DS_ACTN_SET_OUT_PORT_ALL))) {
            lport = inp_actn->ipa_port;
            if (lport == BCMX_LPORT_LOCAL_CPU) {
                lport = BCMX_LPORT_LOCAL_CPU_GET(bcm_unit);
            }
            if (BCM_FAILURE
                (_bcmx_dest_to_modid_port(lport, &modid, &modport,
                                          BCMX_DEST_CONVERT_DEFAULT))) {
                continue;
            }
            xinp_actn.ipa_port = (uint32) modport;
            xinp_actn.ipa_mod = (uint32) modid;
            xinp_actn.ipa_flags |= BCM_DS_ACTN_SET_OUT_MODULE;
        }
        if (pxnm_actn != NULL &&
            (nm_actn->nma_flags & (BCM_DS_NM_ACTN_SET_OUT_PORT_UCAST |
                                   BCM_DS_NM_ACTN_SET_OUT_PORT_NON_UCAST |
                                   BCM_DS_NM_ACTN_SET_OUT_PORT_ALL))) {
            lport = nm_actn->nma_port;
            if (lport == BCMX_LPORT_LOCAL_CPU) {
                lport = BCMX_LPORT_LOCAL_CPU_GET(bcm_unit);
            }
            if (BCM_FAILURE
                (_bcmx_dest_to_modid_port(lport, &modid, &modport,
                                                BCMX_DEST_CONVERT_DEFAULT))) {
                continue;
            }
            xnm_actn.nma_port = (uint32) modport;
            xnm_actn.nma_mod = (uint32) modid;
            xnm_actn.nma_flags |= BCM_DS_NM_ACTN_SET_OUT_MODULE;
        }
        tmp_rv = bcm_ds_classifier_id_create(bcm_unit, dpid, clfr,
                                             pxinp_actn,
                                             outp_actn,
                                             pxnm_actn,
                                             *cfid);
        if (BCM_FAILURE(BCMX_DS_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv))) {
            break;
        }
    }

    BCMX_DS_UNLOCK;

    return rv;
}


/*
 * Function:
 *      bcmx_ds_classifier_update
 */

int
bcmx_ds_classifier_update(int dpid,
                          int cfid,
                          uint32 flags,
                          bcm_ds_inprofile_actn_t *inp_actn,
                          bcm_ds_outprofile_actn_t *outp_actn)
{
    int rv = BCM_E_UNAVAIL;
    int i, bcm_unit;
    int tmp_rv;
    _ds_unit_t *dsu;
    bcm_ds_inprofile_actn_t xinp_actn, *pxinp_actn;
    bcmx_lport_t lport;
    bcm_module_t modid;
    bcm_port_t modport;

    BCMX_DS_INIT;
    BCMX_DS_LOCK;

    DSU_GET(dpid, dsu);

    if (inp_actn != NULL) {
        xinp_actn = *inp_actn;
        pxinp_actn = &xinp_actn;
    } else {
        pxinp_actn = NULL;
    }

    FOREACH_DS_UNIT(dsu, bcm_unit, i) {
        if (pxinp_actn != NULL &&
            (inp_actn->ipa_flags & (BCM_DS_ACTN_SET_OUT_PORT_UCAST |
                                    BCM_DS_ACTN_SET_OUT_PORT_NON_UCAST |
                                    BCM_DS_ACTN_SET_OUT_PORT_ALL))) {
            lport = inp_actn->ipa_port;
            if (lport == BCMX_LPORT_LOCAL_CPU) {
                lport = BCMX_LPORT_LOCAL_CPU_GET(bcm_unit);
            }
            if (BCM_FAILURE
                (_bcmx_dest_to_modid_port(lport, &modid, &modport,
                                          BCMX_DEST_CONVERT_DEFAULT))) {
                continue;
            }
            xinp_actn.ipa_port = (uint32) modport;
            xinp_actn.ipa_mod = (uint32) modid;
            xinp_actn.ipa_flags |= BCM_DS_ACTN_SET_OUT_MODULE;
        }
        tmp_rv = bcm_ds_classifier_update(bcm_unit, dpid, cfid, flags,
                                          pxinp_actn,
                                          outp_actn);
        if (BCM_FAILURE(BCMX_DS_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv))) {
            break;
        }
    }

    BCMX_DS_UNLOCK;
    return rv;
}


/*
 * Function:
 *      bcmx_ds_classifier_delete
 */

int
bcmx_ds_classifier_delete(int dpid, int cfid)
{
    int rv = BCM_E_UNAVAIL;
    int i, bcm_unit;
    int tmp_rv;
    _ds_unit_t *dsu;

    BCMX_DS_INIT;
    BCMX_DS_LOCK;

    DSU_GET(dpid, dsu);
    FOREACH_DS_UNIT(dsu, bcm_unit, i) {
        tmp_rv = bcm_ds_classifier_delete(bcm_unit, dpid, cfid);
        if (BCM_FAILURE(BCMX_DS_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv))) {
            break;
        }
    }

    BCMX_DS_UNLOCK;
    return rv;
}


/*
 * Function:
 *      bcmx_ds_classifier_get
 */

int
bcmx_ds_classifier_get(int dpid, int cfid, 
                       bcm_ds_clfr_t *clfr, 
                       bcm_ds_inprofile_actn_t *inp_actn, 
                       bcm_ds_outprofile_actn_t *outp_actn, 
                       bcm_ds_nomatch_actn_t *nm_actn)
{
    int rv = BCM_E_UNAVAIL;
    int i, bcm_unit;
    int tmp_rv;
    _ds_unit_t *dsu;
    bcmx_lport_t lport;

    BCMX_DS_INIT;
    BCMX_DS_LOCK;

    DSU_GET(dpid, dsu);
    FOREACH_DS_UNIT(dsu, bcm_unit, i) {
        tmp_rv = bcm_ds_classifier_get(bcm_unit, dpid, cfid,
                                       clfr, inp_actn, outp_actn, nm_actn);
        if (BCMX_DS_GET_IS_VALID(bcm_unit, tmp_rv)) {
            rv = tmp_rv;
            break;
        }
    }

    BCMX_DS_UNLOCK;

    if (BCM_FAILURE(rv)) {
        return rv;
    }

    /* Convert to lport if needed */
    if (inp_actn != NULL) {
        if (inp_actn->ipa_flags & (BCM_DS_ACTN_SET_OUT_PORT_UCAST |
                                   BCM_DS_ACTN_SET_OUT_PORT_NON_UCAST |
                                   BCM_DS_ACTN_SET_OUT_PORT_ALL)) {
            if (!BCM_GPORT_IS_SET(inp_actn->ipa_port)) {
                if (inp_actn->ipa_flags & BCM_DS_ACTN_SET_OUT_MODULE) {
                    BCM_IF_ERROR_RETURN
                        (_bcmx_dest_from_modid_port
                         (&lport,
                          (bcm_module_t)inp_actn->ipa_mod,
                          (bcm_port_t)inp_actn->ipa_port,
                          BCMX_DEST_CONVERT_DEFAULT));
                } else {
                    BCM_IF_ERROR_RETURN
                        (_bcmx_dest_from_unit_port
                         (&lport,
                          bcm_unit,
                          (bcm_port_t)inp_actn->ipa_port,
                          BCMX_DEST_CONVERT_DEFAULT));
                }

                inp_actn->ipa_port = (uint32) lport;
            }
        }
    }

    if (nm_actn != NULL) {
        if (nm_actn->nma_flags & (BCM_DS_NM_ACTN_SET_OUT_PORT_UCAST |
                                  BCM_DS_NM_ACTN_SET_OUT_PORT_NON_UCAST |
                                  BCM_DS_NM_ACTN_SET_OUT_PORT_ALL)) {
            if (!BCM_GPORT_IS_SET(nm_actn->nma_port)) {
                if (nm_actn->nma_flags & BCM_DS_NM_ACTN_SET_OUT_MODULE) {
                    BCM_IF_ERROR_RETURN
                        (_bcmx_dest_from_modid_port
                         (&lport,
                          (bcm_module_t)nm_actn->nma_mod,
                          (bcm_port_t)nm_actn->nma_port,
                          BCMX_DEST_CONVERT_DEFAULT));
                } else {
                    BCM_IF_ERROR_RETURN
                        (_bcmx_dest_from_unit_port
                         (&lport,
                          bcm_unit,
                          (bcm_port_t)nm_actn->nma_port,
                          BCMX_DEST_CONVERT_DEFAULT));
                }

                nm_actn->nma_port = (uint32) lport;
            }
        }
    }

    return rv;
}


/*
 * Function:
 *      bcmx_ds_scheduler_add
 */

int
bcmx_ds_scheduler_add(int dpid, bcm_ds_scheduler_t *scheduler)
{
    int rv = BCM_E_UNAVAIL;
    int i, bcm_unit;
    int tmp_rv;
    _ds_unit_t *dsu;

    BCMX_DS_INIT;
    BCMX_DS_LOCK;

    DSU_GET(dpid, dsu);
    FOREACH_DS_UNIT(dsu, bcm_unit, i) {
        tmp_rv = bcm_ds_scheduler_add(bcm_unit, dpid, scheduler);
        if (BCM_FAILURE(BCMX_DS_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv))) {
            break;
        }
    }

    BCMX_DS_UNLOCK;
    return rv;
}


/*
 * Function:
 *      bcmx_ds_counter_get
 * Notes
 *      Returns sum across all chips controlling ports for this DPID.
 */

int
bcmx_ds_counter_get(int dpid, int cfid, bcm_ds_counters_t *counter)
{
    int rv = BCM_E_UNAVAIL;
    int i, bcm_unit;
    int tmp_rv;
    bcm_ds_counters_t tmp_counters;
    _ds_unit_t *dsu;

    sal_memset(&tmp_counters, 0, sizeof(tmp_counters));
    sal_memset(counter, 0, sizeof(*counter));

    BCMX_DS_INIT;
    BCMX_DS_LOCK;

    DSU_GET(dpid, dsu);
    FOREACH_DS_UNIT(dsu, bcm_unit, i) {
        tmp_rv = bcm_ds_counter_get(bcm_unit, dpid, cfid, &tmp_counters);
        if (BCMX_DS_GET_IS_VALID(bcm_unit, tmp_rv)) {
            rv = tmp_rv;
            if (BCM_SUCCESS(tmp_rv)) {
                COMPILER_64_ADD_64(counter->inp_pkt_cntr,
                                   tmp_counters.inp_pkt_cntr);
                COMPILER_64_ADD_64(counter->outp_pkt_cntr,
                                   tmp_counters.outp_pkt_cntr);
            } else {
                break;
            }
        }
    }

    BCMX_DS_UNLOCK;
    return rv;
}

#if defined(BROADCOM_DEBUG)
int
bcmx_ds_dump(void)
{
    int dpid_count = 0;
    int i, idx;
    int unit;
    _ds_unit_t *dsu;

    BCMX_READY_CHECK;

    BCMX_DEBUG(BCMX_DBG_NORMAL, ("BCMX DS Dump\n"));
    for (i = 0; i < DPID_BUCKETS; i++) {
        dsu = _ds_unit_buckets[i];
        while (dsu != 0) {
            dpid_count++;
            BCMX_DEBUG(BCMX_DBG_NORMAL, ("DPID %d:  ", dsu->dpid));
            FOREACH_DS_UNIT(dsu, unit, idx) {
                BCMX_DEBUG(BCMX_DBG_NORMAL, ("%d ", unit));
            }
            BCMX_DEBUG(BCMX_DBG_NORMAL, ("\n"));
            dsu = dsu->next;
        }
    }

    if (dpid_count == 0) {
        BCMX_DEBUG(BCMX_DBG_NORMAL, ("No DPIDs found\n"));
    }

    return BCM_E_NONE;
}

#endif /* BROADCOM_DEBUG */
