/*
 * $Id: filter.c 1.65.6.1 Broadcom SDK $
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
 * Module: Fast Filter Processor
 *
 * Purpose:
 *     API for filter rules and masks.
 */

#include <soc/cm.h>
#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/cmic.h>
#include <soc/filter.h>

#include <bcm/filter.h>
#include <bcm/error.h>
#include <bcm/debug.h>

#include <bcm_int/esw/meter.h>
#include <bcm_int/esw/filter.h>
#include <bcm_int/esw_dispatch.h>
#include <bcm_int/esw/stack.h>


#ifdef BCM_FILTER_SUPPORT

/*
 * The following values are used for allocations and should be the
 * maximum that could be used for any chip.
 */

#define PACKET_DEPTH_MAX        80
#define FILTER_BYTES_MAX        64
#define WINDOW_MAX              8

/* 
 * Macro: FILTER_DEBUG
 */
#define FILTER_DEBUG(flags, stuff) BCM_DEBUG((flags) | BCM_DBG_FILTER, stuff)
#define FILTER_OUT(stuff)          BCM_DEBUG(BCM_DBG_FILTER, stuff)
#define FILTER_WARN(stuff)         FILTER_DEBUG(BCM_DBG_WARN, stuff)
#define FILTER_ERR(stuff)          FILTER_DEBUG(BCM_DBG_ERR, stuff)
#define FILTER_VERB(stuff)         FILTER_DEBUG(BCM_DBG_VERBOSE, stuff)
#define FILTER_VVERB(stuff)        FILTER_DEBUG(BCM_DBG_VVERBOSE, stuff)

/*
 * Macro: FILTER_[RULE|MASK]_CHK_FIELD_SIZE
 *
 * Purpose:
 *      Check if the value fits in a GFILTER_IRULEm or GFILTER_IMASKm
 *      memory field or not.
 *
 * Parameters:
 *      type - Field type
 *      val  - Field value
 *
 * Notes:
 *      Results in return(BCM_E_PARAM) if fails.
 */
#define FILTER_RULE_CHK_FIELD_SIZE(type, val) do { \
        if (soc_mem_field32_fit(unit, GFILTER_IRULEm, type, val) != SOC_E_NONE) {\
            return SOC_E_PARAM;\
        }\
        } while (0)

#define FILTER_MASK_CHK_FIELD_SIZE(type, val) do { \
        if (soc_mem_field32_fit(unit, GFILTER_IMASKm, type, val) != SOC_E_NONE) {\
            return SOC_E_PARAM;\
        }\
        } while (0)

/*
 * Typedef: filter_mask_entry_t
 *
 * Purpose:
 *      One entry for each filter in each block (each physical filter)
 */
typedef struct filter_mask_entry_s {
    imask_entry_t       fm_mask;        /* Actual mask */
    int                 fm_cnt;         /* Use count, 0 --> unused */
    int                 fm_prio;        /* Priority */
} filter_mask_entry_t;

/*
 * Typedef: filter_blk_imask_entry_t
 *
 * Purpose:
 *      One entry for each block, fpi_mask is an array of fpi_mask_max.
 */
typedef struct filter_blk_s {
    int                 fp_mask_cnt;    /* Maximum number masks in hardware */
    int                 fp_mask_used;   /* Number masks used */
    filter_mask_entry_t fp_mask[1];     /* Array of mask entries (actual
                                           (# of entries = # of masks) */
} filter_blk_t;

#define SIZEOF_FILTER_BLK_T(num_mask)                           \
        (sizeof (filter_blk_t) + ((num_mask) - 1) *             \
                                 sizeof (filter_mask_entry_t))

#define FIELD_MASK(unit, mem, field)    /* Note: only works to 31 bits */ \
        (((uint32) 1 << soc_mem_field_length((unit), (mem), (field))) - 1)

#define FILTERID_BUCKETS        64

/*
 * Typedef: filter_cntl_t
 *
 * Purpose:
 *      One structure for each StrataSwitch Device.
 */
typedef struct filter_cntl_s {
    filter_blk_t        *fc_table[SOC_MAX_NUM_BLKS];

    /* Static details about hardware unit, filled in by bcm_filter_init */

    soc_mem_t           fc_blk2rule[SOC_MAX_NUM_BLKS];
    soc_mem_t           fc_blk2mask[SOC_MAX_NUM_BLKS];
    int                 fc_window_count;
    int                 fc_window_size;
    int                 fc_packet_depth;
    int                 fc_dataoff_max;
    int                 fc_meter_size;
    int                 (*fc_offset_fn)(int window, int dataoff);

    /* filterid to struct filter mapping */
    sal_mutex_t         fc_lock;
    uint32              fc_filterid_next;
    struct filter_s     *fc_fbucket[FILTERID_BUCKETS];
} filter_cntl_t;

static  filter_cntl_t   *_bcm_filter_control[BCM_MAX_NUM_UNITS];

static  soc_field_t     _bcm_filter_win2field[] = {
    DATAOFFSET1f, DATAOFFSET2f, DATAOFFSET3f, DATAOFFSET4f,
    DATAOFFSET5f, DATAOFFSET6f, DATAOFFSET7f, DATAOFFSET8f
};

const bcm_mac_t _bcm_filter_mac_all_ones =
        {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

#define FILTER_LOCK(_u) {               \
    soc_mem_lock(_u, GFILTER_IRULEm);   \
    soc_mem_lock(_u, GFILTER_IMASKm);   \
}

#define FILTER_UNLOCK(_u) {             \
    soc_mem_unlock(_u, GFILTER_IMASKm); \
    soc_mem_unlock(_u, GFILTER_IRULEm); \
}

/*
 * Macro: FILTER_INIT
 *
 * Purpose:
 *      Check the filter functions are initialized for the specified unit.
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *
 * Notes:
 *      Results in return(BCM_E_UNIT), return(BCM_E_UNAVAIL), or
 *      return(BCM_E_INIT) if fails.
 */
#define FILTER_INIT(unit)                                                     \
    if (!SOC_UNIT_VALID(unit))                  { return BCM_E_UNIT; }        \
    if (!soc_feature(unit, soc_feature_filter)) { return BCM_E_UNAVAIL; }     \
    if (_bcm_filter_control[unit] == NULL)      { return BCM_E_INIT; }

/*
 * Macro: BCM_FILTER_[PORT|MODID]_VALID
 *
 * Purpose:
 *      Check the filter parameter used as a port number or modid
 *      is in the valid range for the device.
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      port - Port number
 *      modid - Module ID
 *
 * Notes:
 *      Results in return(BCM_E_PARAM) if fails.
 */
#define BCM_FILTER_PORT_VALID(unit, port) \
    if (!SOC_PORT_ADDRESSABLE(unit, port)) { return BCM_E_PARAM; }

#define BCM_FILTER_MODID_VALID(unit, modid) \
    if (!SOC_MODID_ADDRESSABLE(unit, modid)) { return BCM_E_PARAM; }



/*
 * Typedef: filter_t
 *
 * Purpose:
 *      This is the real filter storage structure.
 *      bcm_filterid_t is an opaque type.
 *
 * To do:
 *      Change rule to a union of irule and girule.
 *      For now irule and girule must have identical entry format.
 */
typedef struct filter_s {
    uint32              filterid;       /* this filter's external name */
    struct filter_s     *next;          /* next filter in fc_fbucket */
    int                 prio;           /* Priority rel. to other filters */
    pbmp_t              ipbm;           /* Ingress port bitmap */
    pbmp_t              epbm;           /* Egress port bitmap */
    int                 eport_special;  /* FILTER_EPORT_xxx */
    uint8               data_val[PACKET_DEPTH_MAX];
    uint8               data_mask[PACKET_DEPTH_MAX];
    irule_entry_t       rule;
    imask_entry_t       mask;
    int                 del_max;        /* # rules to del. on failed ins. */
    int                 modport_set;
#define RMODPORT        1               /* rmodule/rport in use */
#define MMODPORT        2               /* mmodule/mport in use */
    bcm_module_t        rmodule;
    bcm_port_t          rport;
    bcm_module_t        mmodule;
    bcm_port_t          mport;
} filter_t;

#define FID2BUCKET(_fid)        (_fid % FILTERID_BUCKETS)

/*
 * Lookup a filterid in a unit's filters
 */
STATIC filter_t *
_bcm_filter_find(int unit, uint32 fid)
{
    filter_t            *f;
    filter_cntl_t       *fc;
    int                 fbucket;

    fc = _bcm_filter_control[unit];
    if (fc == NULL) {
        return NULL;
    }
    fbucket = FID2BUCKET(fid);

    for (f = fc->fc_fbucket[fbucket]; f != NULL; f = f->next) {
        if (f->filterid == fid) {
            return f;
        }
    }
    return NULL;
}


/***********************************************************************
 *
 * Filter Mask Allocate/Free
 *
 ***********************************************************************/

/*
 * Function:
 *      _bcm_filter_mask_index
 *
 * Purpose:
 *      Returns the current index of the specified mask.
 *
 * Parameters:
 *      unit - StrataSwitch unit #
 *      blk - Block number
 *      imask - mask to locate
 *      prio - priority of mask to locate
 *
 * Returns:
 *      index, or -1 if not found
 *
 * Notes:
 *      Assumes FILTER_LOCK held, reserved fields are 0.
 *
 *      The prio field is passed in because even if two masks are otherwise
 *      identical, if they have different priories, they need to be put in
 *      separate entries.  In a future chip there will be a PRIO field in
 *      the mask itself.
 */
STATIC int
_bcm_filter_mask_index(int unit, int blk, imask_entry_t *imask, int prio)
{
    filter_cntl_t       *fc = _bcm_filter_control[unit];
    filter_blk_t        *fp = fc->fc_table[blk];
    int                 index;

    /* Loop through looking for entry */

    for (index = 0; index < fp->fp_mask_cnt; index++) {
        if (fp->fp_mask[index].fm_cnt > 0 &&
            fp->fp_mask[index].fm_prio == prio &&
            sal_memcmp(&fp->fp_mask[index].fm_mask,
                       imask,
                       sizeof (*imask)) == 0) {
            FILTER_VERB((
                         "_bcm_filter_mask_index: found at %d\n", index));
            return(index);
        }
    }

    FILTER_VERB(("_bcm_filter_mask_index: not found\n"));

    return -1;
}

STATIC int
_bcm_filter_mask_index_to_hw_index(int unit, int mask_index)
{
    int hw_index = mask_index;

    if (soc_feature(unit, soc_feature_filter_128_rules)) {
        /* Mapping mask_index -> hw_index
         *             0            0
         *             1            1
         *             2            4
         *             3            5
         *             4            8
         *             5            9
         *             6            12
         *             7            13
         */
        hw_index = (2 * mask_index) - (mask_index % 2);
    }

    return hw_index;
}

/*
 * Function: _bcm_filter_mask_free
 *
 * Purpose:
 *      Free a filter mask on the specified block.
 * Parameters:
 *      unit - StrataSwitch unit #
 *      blk - Block number
 *      index - index of mask to free.
 *
 * Returns:
 *      Index, or BCM_E_XXX if less than 0.
 *
 * Notes:
 *      Assumes FILTER_LOCK held, reserved fields are 0.
 */
STATIC int
_bcm_filter_mask_free(int unit, int blk, int index)
{
    filter_cntl_t       *fc = _bcm_filter_control[unit];
    filter_blk_t        *fp = fc->fc_table[blk];
    filter_mask_entry_t *fm = &fp->fp_mask[index];

    FILTER_VERB(("_bcm_filter_mask_free: blk=%d ind=%d\n", blk, index));

    assert(fm->fm_cnt > 0);

    if (--fm->fm_cnt == 0) {
        fp->fp_mask_used--;

        sal_memcpy(&fm->fm_mask,
                   soc_mem_entry_null(unit, fc->fc_blk2mask[blk]),
                   sizeof (fm->fm_mask));

        FILTER_VERB(("_bcm_filter_mask_free: clear mask\n"));

        return soc_mem_write(unit,
                             fc->fc_blk2mask[blk],
                             blk,
                       _bcm_filter_mask_index_to_hw_index(unit, index),
                             &fm->fm_mask);
    }

    return(BCM_E_NONE);
}

/*
 * Function: _bcm_filter_mask_alloc
 *
 * Purpose:
 *      Allocate a filter mask on the specified block.
 *
 * Parameters:
 *      unit  - StrataSwitch unit #
 *      blk   - Block number
 *      imask - mask to locate
 *      prio  - priority of mask to locate
 *              if prio is negative then -(prio-1) will be the mask
 *              used or BCM_E_EXISTS is returned.
 *
 * Returns:
 *      Index, or BCM_E_XXX if less than 0.
 *      BCM_E_PARAM  - prio out of range
 *      BCM_E_EXISTS - non-matching mask already exists.
 *      BCM_E_FULL   - no unused masks left
 *
 * Notes:
 *      Assumes FILTER_LOCK held, reserved fields are 0.
 */
STATIC int
_bcm_filter_mask_alloc(int unit, int blk, imask_entry_t *imask, int prio)
{
    filter_cntl_t       *fc = _bcm_filter_control[unit];
    filter_blk_t        *fp = fc->fc_table[blk];
    int                 index, n;

    FILTER_VERB(("_bcm_filter_mask_alloc: blk=%d prio=%d\n", blk, prio));

    /*
     * If prio is negative, then do not do a search, but use a
     * specific mask entry.
     */
    if (prio < 0) {
        index = -prio - 1;

        if (index > fp->fp_mask_cnt) {
            FILTER_ERR(("FFP Error: prio=%d out-of-range\n", prio));
            return BCM_E_PARAM;
        }
        if (fp->fp_mask[index].fm_cnt > 0) {    /* already in use */
            if (fp->fp_mask[index].fm_prio == prio &&
                sal_memcmp(&fp->fp_mask[index].fm_mask,
                           imask,
                           sizeof (*imask)) == 0) {
                FILTER_VERB(("_bcm_filter_mask_alloc: re-use exact index=%d\n",
                             index));
                fp->fp_mask[index].fm_cnt++;
                return index;
            }
            FILTER_ERR(("FFP Error: prio=%d already in use\n", prio));
            return BCM_E_EXISTS;
        }

        FILTER_VERB(("_bcm_filter_mask_alloc: new exact index=%d\n", index));

        SOC_IF_ERROR_RETURN(soc_mem_write(unit,
                                          fc->fc_blk2mask[blk],
                                          blk,
                                          index, imask));

        fp->fp_mask[index].fm_mask = *imask;    /* Structure copy */
        fp->fp_mask[index].fm_cnt = 1;
        fp->fp_mask[index].fm_prio = prio;

        fp->fp_mask_used++;

        return index;
    }

    /* Check if the same mask is already installed */

    if ((index = _bcm_filter_mask_index(unit, blk, imask, prio)) >= 0) {
        /*
         * Found; increment reference count and reuse same mask.
         */

        fp->fp_mask[index].fm_cnt++;

        FILTER_VERB(("_bcm_filter_mask_alloc: re-use index=%d\n", index));

        return index;
    }

    /*
     * Add new mask
     *
     * Currently this uses 'prio' as a hint for which mask index to
     * try to allocate.  To truly support mask priority is too
     * complicated because it would require moving the masks around,
     * which in turn would require moving all the rules around.
     */

    index = (prio % fp->fp_mask_cnt);

    for (n = 0; n < fp->fp_mask_cnt; n++) {
        if (fp->fp_mask[index].fm_cnt == 0) {   /* found free one */
            FILTER_VERB(("_bcm_filter_mask_alloc: new index=%d\n", index));

            SOC_IF_ERROR_RETURN(soc_mem_write(unit,
                                              fc->fc_blk2mask[blk],
                                              blk,
                         _bcm_filter_mask_index_to_hw_index(unit, index),
                                              imask));

            fp->fp_mask[index].fm_mask = *imask;        /* Structure copy */
            fp->fp_mask[index].fm_cnt = 1;
            fp->fp_mask[index].fm_prio = prio;

            fp->fp_mask_used++;

            return(index);
        }

        index = (index + 1) % fp->fp_mask_cnt;
    }

    FILTER_ERR(("FFP Error: no unused masks left\n"));
    return BCM_E_FULL;
}

/***********************************************************************
 *
 * Filter Rule/Mask Generation
 *
 ***********************************************************************/

/*
 * Function: bcm_esw_filter_create
 *
 * Purpose:
 *      Create a blank filter template
 * Parameters:
 *      unit       - StrataSwitch Unit number.
 *      fid_return - (OUT) Pointer to new filter ID.
 *
 * Returns:
 *      BCM_E_NONE    - Success
 *      BCM_E_UNIT    - Invalid unit number
 *      BCM_E_INIT    - Filter not initialized on unit
 *      BCM_E_UNAVAIL - Filter not supported on unit
 *      BCM_E_PARAM   - fid_return points to NULL
 *      BCM_E_MEMORY  - Filter allocation failure
 *
 * Notes:
 *      Filter template should be destroyed when no longer needed.
 */
int
bcm_esw_filter_create(int unit, bcm_filterid_t *fid_return)
{
    filter_cntl_t       *fc;

    FILTER_VVERB(("BEGIN bcm_filter_create(unit=%d, *fid_return=%p\n", unit,
                 fid_return));
    FILTER_INIT(unit);

    if (fid_return == NULL) {
        FILTER_ERR(("FFP Error: fid_return=NULL\n"));
        return BCM_E_PARAM;
    }

    fc = _bcm_filter_control[unit];

    sal_mutex_take(fc->fc_lock, sal_mutex_FOREVER);
    *fid_return = ++fc->fc_filterid_next;
    sal_mutex_give(fc->fc_lock);

    return bcm_esw_filter_create_id(unit, *fid_return);
}

/*
 * Function: bcm_esw_filter_create_id
 *
 * Purpose:
 *      Create a blank filter template
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      fid   - Requested filter id.
 *
 * Returns:
 *      BCM_E_NONE    - Success
 *      BCM_E_UNIT    - Invalid unit number
 *      BCM_E_INIT    - Filter not initialized on unit
 *      BCM_E_UNAVAIL - Filter not supported on unit
 *      BCM_E_EXISTS  - fid points to NULL
 *      BCM_E_MEMORY  - Filter allocation failure
 *
 * Notes:
 *      Filter template should be destroyed when no longer needed.
 */
int
bcm_esw_filter_create_id(int unit, bcm_filterid_t fid)
{
    filter_t            *filter;
    filter_cntl_t       *fc;
    int                 fbucket;
    int rv;

    FILTER_VVERB(("BEGIN bcm_filter_create_id(unit=%d, fid=%d)\n", unit, fid));
    FILTER_INIT(unit);

    filter = _bcm_filter_find(unit, fid);
    if (filter != NULL) {
        FILTER_ERR(("FFP Error: fid=%d already exists.\n", fid));
        return BCM_E_EXISTS;
    }

    if ((filter = sal_alloc(sizeof (*filter), "filter")) == NULL) {
        FILTER_ERR(("FFP Error: filter allocation failure.\n"));
        return BCM_E_MEMORY;
    }

    sal_memset(filter, 0, sizeof (*filter));

    filter->ipbm = PBMP_E_ALL(unit);
    filter->epbm = PBMP_ALL(unit);

    /* default port steering to the local module id */
    if (soc_feature(unit, soc_feature_filter_xgs)) {
        int     modid;

        rv = bcm_esw_stk_my_modid_get(unit, &modid);

        if (BCM_FAILURE(rv))
        {
            sal_free(filter);
            FILTER_ERR(("FFP error:  Unable to get modid.\n"));
            return rv;
        }

        /* My modid is verified as addressable when it is set, so we do not
         * need to recheck it here. If it was set invalid, we would have
         * gotten an error above. */
        soc_GFILTER_IRULEm_field32_set(unit, &filter->rule, OUTPUTMODf, modid);
        soc_GFILTER_IMASKm_field32_set(unit, &filter->mask, OUTPUTMODf, modid);
        filter->rmodule = modid;
        filter->mmodule = modid;
    }

    fc = _bcm_filter_control[unit];

    sal_mutex_take(fc->fc_lock, sal_mutex_FOREVER);
    filter->filterid        = fid;
    fbucket                 = FID2BUCKET(filter->filterid);
    filter->next            = fc->fc_fbucket[fbucket];
    fc->fc_fbucket[fbucket] = filter;
    sal_mutex_give(fc->fc_lock);

    return BCM_E_NONE;
}

/*
 * Function: bcm_esw_filter_destroy
 *
 * Purpose:
 *      Destroy a filter template
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf - Handle to filter template to operate on.
 *
 * Notes:
 *      Filter templates should be destroyed when no longer needed.
 *      A filter template is still needed if it is to be removed later.
 */
int
bcm_esw_filter_destroy(int unit, bcm_filterid_t bf)
{
    filter_t            *f, *fsrch, *fprev;
    filter_cntl_t       *fc;
    int                 fbucket;

    FILTER_VVERB(("BEGIN bcm_filter_destroy(unit=%d, filterid=%d\n", unit, bf));

    FILTER_INIT(unit);

    f = _bcm_filter_find(unit, bf);
    if (f == NULL) {
        FILTER_ERR(("FFP Error: filter id=%d not found\n", bf));
        return BCM_E_NOT_FOUND;
    }

    fc = _bcm_filter_control[unit];
    sal_mutex_take(fc->fc_lock, sal_mutex_FOREVER);
    fbucket = FID2BUCKET(f->filterid);
    fprev = NULL;
    for (fsrch = fc->fc_fbucket[fbucket]; fsrch != NULL; fsrch = fsrch->next) {
        if (fsrch == f) {
            if (fprev == NULL) {
                fc->fc_fbucket[fbucket] = f->next;
            } else {
                fprev->next = f->next;
            }
            break;
        }
        fprev = fsrch;
    }
    sal_mutex_give(fc->fc_lock);

    sal_free(f);

    return BCM_E_NONE;
}

/*
 * Function: bcm_esw_filter_copy
 *
 * Purpose:
 *      Create a copy of an existing filter template.
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      f_src - Pointer to filter to copy.
 *      f_return - (OUT) Pointer to new filter.
 *
 * Notes:
 *      Filter template should be destroyed when no longer needed.
 */
int
bcm_esw_filter_copy(int unit, bcm_filterid_t bf_src, bcm_filterid_t *bf_return)
{
    filter_t            *f_src;
    filter_t            *f;
    filter_cntl_t       *fc;
    int                 fbucket;

    FILTER_VVERB(("BEGIN bcm_filter_copy(unit=%d, src_id=%d\n", unit, bf_src));
    FILTER_INIT(unit);

    f_src = _bcm_filter_find(unit, bf_src);
    if (f_src == NULL) {
        FILTER_ERR(("FFP Error: filter id=%d not found\n", bf_src));
        return BCM_E_NOT_FOUND;
    }

    if ((f = sal_alloc(sizeof (*f), "filter")) == NULL) {
        FILTER_ERR(("FFP Error: allocation failure in bcm_filter_copy()\n"));
        return BCM_E_MEMORY;
    }

    sal_memcpy(f, f_src, sizeof (*f));

    fc = _bcm_filter_control[unit];
    sal_mutex_take(fc->fc_lock, sal_mutex_FOREVER);
    f->filterid = ++fc->fc_filterid_next;
    fbucket = FID2BUCKET(f->filterid);
    f->next = fc->fc_fbucket[fbucket];
    fc->fc_fbucket[fbucket] = f;
    sal_mutex_give(fc->fc_lock);
    *bf_return = f->filterid;

    return BCM_E_NONE;
}

/*
 * Function: bcm_esw_filter_copy_id
 *
 * Purpose:
 *      Create a copy of an existing filter template.
 *
 * Parameters:
 *      unit   - StrataSwitch Unit number.
 *      bf_src - Source filter to copy.
 *      bf_dst - Destination filter for copy.
 *
 * Notes:
 *      Filter template should be destroyed when no longer needed.
 */
int
bcm_esw_filter_copy_id(int unit, bcm_filterid_t bf_src, bcm_filterid_t bf_dst)
{
    filter_t            *f_src;
    filter_t            *f;
    filter_cntl_t       *fc;
    int                 fbucket;

    FILTER_VVERB(("BEGIN bcm_filter_copy_id(unit=%d, src_id=%d, dst_id=%d)\n",
                  unit, bf_src, bf_dst));
    FILTER_INIT(unit);

    f_src = _bcm_filter_find(unit, bf_src);
    if (f_src == NULL) {
        FILTER_ERR(("FFP Error: filter id=%d not found\n", bf_src));
        return BCM_E_NOT_FOUND;
    }
    f = _bcm_filter_find(unit, bf_dst);
    if (f != NULL) {
        FILTER_ERR(("FFP Error: filter id=%d exists\n", bf_dst));
        return BCM_E_EXISTS;
    }

    if ((f = sal_alloc(sizeof (*f), "filter")) == NULL) {
        FILTER_ERR(("FFP Error: allocation failure in bcm_filter_copy_id()\n"));
        return BCM_E_MEMORY;
    }

    sal_memcpy(f, f_src, sizeof (*f));

    fc = _bcm_filter_control[unit];
    sal_mutex_take(fc->fc_lock, sal_mutex_FOREVER);
    f->filterid = bf_dst;
    fbucket = FID2BUCKET(f->filterid);
    f->next = fc->fc_fbucket[fbucket];
    fc->fc_fbucket[fbucket] = f;
    sal_mutex_give(fc->fc_lock);

    return BCM_E_NONE;
}

/*
 * Function: bcm_esw_filter_qualify_priority
 *
 * Purpose:
 *      Set priority of this filter relative to other filters that
 *      match simultaneously.
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf - Handle to filter template to operate on.
 *      prio - Relative priority from 0 (min) to 255 (max).
 *
 * Notes:
 *      On StrataSwitch, this routine internally organizes the mask
 *      entries in an order appropriate to prioritize the filters as
 *      requested (within the limits of the hardware).
 */
int
bcm_esw_filter_qualify_priority(int unit, bcm_filterid_t bf, int prio)
{
    filter_t            *f;

    FILTER_VVERB(("BEGIN bcm_filter_copy_id(unit=%d, fid=%d, prio=%d)\n",
                  unit, bf, prio));
    FILTER_INIT(unit);

    f = _bcm_filter_find(unit, bf);
    if (f == NULL) {
        FILTER_ERR(("FFP Error: filter id=%d not found\n", bf));
        return BCM_E_NOT_FOUND;
    }

    if (prio < BCM_FILTER_QUALIFY_PRIO_MIN ||
        BCM_FILTER_QUALIFY_PRIO_MAX < prio) {
        FILTER_ERR(("FFP Error: %d < (prio=%d) < %d out-of-range\n",
            BCM_FILTER_QUALIFY_PRIO_MIN, prio, BCM_FILTER_QUALIFY_PRIO_MAX));
        return BCM_E_PARAM;
    }

    f->prio = prio;

    return BCM_E_NONE;
}

/*
 * Function: bcm_esw_filter_qualify_ingress
 *
 * Purpose:
 *      Set ingress port(s) that the packet must match to trigger filter.
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf - Handle to filter template to operate on.
 *      pbmp - Bitmap of ingress ports.
 *
 * Notes:
 *      Default is PBMP_E_ALL if not qualified.
 *      Certain port combinations need few hardware resources to
 *      implement while other combinations may require lots of resources;
 *      it depends on the underlying chip features.  For example,
 *      StrataSwitch works efficiently for groups of 8 ports.
 */
int
bcm_esw_filter_qualify_ingress(int unit, bcm_filterid_t bf, pbmp_t pbmp)
{
    filter_t            *f;

    FILTER_VVERB(("BEGIN bcm_filter_qualify_ingress(unit=%d, fid=%d)\n",
                  unit, bf));
    FILTER_INIT(unit);

    f = _bcm_filter_find(unit, bf);
    if (f == NULL) {
        FILTER_ERR(("FFP Error: filter id=%d not found\n", bf));
        return BCM_E_NOT_FOUND;
    }
    f->ipbm = pbmp;

    return BCM_E_NONE;
}

/*
 * Function: bcm_esw_filter_qualify_egress
 *
 * Purpose:
 *      Add egress port(s) that the packet must match to trigger filter.
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf - Handle to filter template to operate on.
 *      pbmp - Bitmap of egress ports.
 *
 * Notes:
 *      Default is PBMP_ALL if not qualified.
 *      Certain port combinations need few hardware resources to
 *      implement while other combinations may require lots of resources;
 *      it depends on the underlying chip features.  For example,
 *      StrataSwitch would require one rule per port unless all ports
 *      are specified.
 */
int
bcm_esw_filter_qualify_egress(int unit, bcm_filterid_t bf, pbmp_t pbmp)
{
    filter_t            *f;

    FILTER_VVERB(("BEGIN bcm_filter_qualify_egress(unit=%d, fid=%d)\n",
                  unit, bf));
    FILTER_INIT(unit);

    f = _bcm_filter_find(unit, bf);
    if (f == NULL) {
        FILTER_ERR(("FFP Error: filter id=%d not found\n", bf));
        return BCM_E_NOT_FOUND;
    }
    if (f->eport_special != 0) {
        return BCM_E_RESOURCE;
    }

    f->epbm = pbmp;

    return BCM_E_NONE;
}

/*
 * Function: bcm_esw_filter_qualify_egress_modid
 *
 * Purpose:
 *      Set a filter to match only a particular egress module ID.
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf - Handle to filter template to operate on.
 *      module_id - Module ID to set.
 *
 * Return Values:
 *      BCM_E_RESOURCE - exceeded ability of hardware filter table
 *
 * Notes:
 *      Used for stacking support.
 */
int
bcm_esw_filter_qualify_egress_modid(int unit, bcm_filterid_t bf, int module_id)
{
    FILTER_VVERB(("BEGIN bcm_filter_qualify_egress_modid(unit=%d, fid=%d,"
                  " mod_id=%d)\n", unit, bf, module_id));
    FILTER_INIT(unit);

    if (! soc_feature(unit, soc_feature_filter_extended)) {
        return BCM_E_UNAVAIL;
    } else {
        filter_t        *f;
        uint32          m;

        f = _bcm_filter_find(unit, bf);
        if (f == NULL) {
            FILTER_ERR(("FFP Error: filter id=%d not found\n", bf));
            return BCM_E_NOT_FOUND;
        }

        m = FIELD_MASK(unit, GFILTER_IMASKm, OUTPUTMODMASKf);
        BCM_FILTER_MODID_VALID(unit, module_id);
        FILTER_MASK_CHK_FIELD_SIZE(OUTPUTMODMASKf, m);
        FILTER_RULE_CHK_FIELD_SIZE(EGRSMODf, module_id);
        soc_GFILTER_IMASKm_field32_set(unit, &f->mask, OUTPUTMODMASKf, m);
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule, EGRSMODf, module_id);
    }

    return BCM_E_NONE;
}

/*
 * Function: bcm_esw_filter_qualify_unknown_ucast
 *
 * Purpose:
 *      Set a filter to match only unknown unicast packets
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf - Handle to filter template to operate on.
 */
int
bcm_esw_filter_qualify_unknown_ucast(int unit, bcm_filterid_t bf)
{
    filter_t            *f;
    int                 eport = -1;

    FILTER_VVERB(("BEGIN bcm_filter_qualify_unknown_ucast(unit=%d, fid=%d)\n",
                  unit, bf));
    FILTER_INIT(unit);

    f = _bcm_filter_find(unit, bf);
    if (f == NULL) {
        FILTER_ERR(("FFP Error: filter id=%d not found\n", bf));
        return BCM_E_NOT_FOUND;
    }

    if (SOC_IS_XGS_SWITCH(unit)) {
        eport = FILTER_EPORT_XGS_UNKNOWN_UC;
    }

    if (eport < 0 ||
        f->eport_special != 0 ||
        SOC_PBMP_NEQ(f->epbm, PBMP_ALL(unit))) {
        return BCM_E_RESOURCE;
    }

    f->eport_special = eport;

    return BCM_E_NONE;
}

/*
 * Function: bcm_esw_filter_qualify_unknown_mcast
 *
 * Purpose:
 *      Set a filter to match only unknown multicast packets
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf - Handle to filter template to operate on.
 */
int
bcm_esw_filter_qualify_unknown_mcast(int unit, bcm_filterid_t bf)
{
    filter_t            *f;

    FILTER_VVERB(("BEGIN bcm_filter_qualify_unknown_mcast(unit=%d, fid=%d)\n",
                  unit, bf));
    FILTER_INIT(unit);

    COMPILER_REFERENCE(f);
    f = _bcm_filter_find(unit, bf);
    if (f == NULL) {
        FILTER_ERR(("FFP Error: filter id=%d not found\n", bf));
        return BCM_E_NOT_FOUND;
    }

    if (SOC_IS_XGS_SWITCH(unit) &&
        f->eport_special == 0 &&
        SOC_PBMP_EQ(f->epbm, PBMP_ALL(unit))) {
        f->eport_special = FILTER_EPORT_XGS_UNKNOWN_MC;
        return BCM_E_NONE;
    }

    return BCM_E_RESOURCE;
}

/*
 * Function: bcm_esw_filter_qualify_known_ucast
 *
 * Purpose:
 *      Set a filter to match only known unicast packets
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf - Handle to filter template to operate on.
 *
 * Notes:
 */

int
bcm_esw_filter_qualify_known_ucast(int unit, bcm_filterid_t bf)
{
    COMPILER_REFERENCE(bf);
    COMPILER_REFERENCE(unit);

    FILTER_VVERB(("BEGIN bcm_filter_qualify_known_ucast(unit=%d, fid=%d)\n",
                  unit, bf));
    FILTER_INIT(unit);

    return BCM_E_RESOURCE;
}

/*
 * Function: bcm_esw_filter_qualify_known_mcast
 *
 * Purpose:
 *      Set a filter to match only known multicast packets
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf - Handle to filter template to operate on.
 */
int
bcm_esw_filter_qualify_known_mcast(int unit, bcm_filterid_t bf)
{
    filter_t            *f;
    int                 eport = -1;

    FILTER_VVERB(("BEGIN bcm_filter_qualify_known_mcast(unit=%d, fid=%d)\n",
                  unit, bf));
    FILTER_INIT(unit);

    f = _bcm_filter_find(unit, bf);
    if (f == NULL) {
        FILTER_ERR(("FFP Error: filter id=%d not found\n", bf));
        return BCM_E_NOT_FOUND;
    }
    if (SOC_IS_XGS_SWITCH(unit)) {
        eport = FILTER_EPORT_XGS_KNOWN_MC;
    }

    if (eport < 0 ||
        f->eport_special != 0 ||
        SOC_PBMP_NEQ(f->epbm, PBMP_ALL(unit))) {
        return BCM_E_RESOURCE;
    }

    f->eport_special = eport;

    return BCM_E_NONE;
}

/*
 * Function: bcm_esw_filter_qualify_broadcast
 *
 * Purpose:
 *      Set a filter to match only broadcast packets
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf - Handle to filter template to operate on.
 */
int
bcm_esw_filter_qualify_broadcast(int unit, bcm_filterid_t bf)
{
    FILTER_VVERB(("BEGIN bcm_filter_qualify_broadcast(unit=%d, fid=%d)\n",
                  unit, bf));
    return bcm_esw_filter_qualify_data(unit, bf,
                                       0, 6,
                                       _soc_mac_all_ones, _soc_mac_all_ones);
}

/*
 * Function: bcm_esw_filter_qualify_stop
 *
 * Purpose:
 *      Set a filter to stop higher mask checks on match.
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf - Handle to filter template to operate on.
 *      partial_match - (TRUE/FALSE) stop only on partial match,
 *              otherwise only on full match.
 */
int
bcm_esw_filter_qualify_stop(int unit, bcm_filterid_t bf, int partial_match)
{
    filter_t            *f;

    FILTER_VVERB(("BEGIN bcm_filter_qualify_stop(unit=%d, fid=%d,"
                  " partial_match=%d)\n", unit, bf, partial_match));
    FILTER_INIT(unit);

    f = _bcm_filter_find(unit, bf);
    if (f == NULL) {
        FILTER_ERR(("FFP Error: filter id=%d not found\n", bf));
        return BCM_E_NOT_FOUND;
    }
    if (!SOC_IS_LYNX(unit)) {
        return BCM_E_UNAVAIL;
    }

    if (partial_match) {
        soc_GFILTER_IMASKm_field32_set(unit, &f->mask, STOPf, 1);
    } else {
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule, STOPf, 1);
    }

    return BCM_E_NONE;
}

/*
 * Function: bcm_esw_filter_qualify_format
 *
 * Purpose:
 *      Set a filter to match only a particular packet format.
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf - Handle to filter template to operate on.
 *      format - Packet format to filter on.
 *
 * Notes:
 *      802.3 packets have type/len < 0x600.
 *      Ethernet II packets have type/len >= 0x600.
 *      Only one format may be filtered on at a time.
 */
int
bcm_esw_filter_qualify_format(int unit, bcm_filterid_t bf,
                          bcm_filter_format_t format)
{
    filter_t            *f;
    uint32              val = 0;
    uint32              chip_mask;
    bcm_filter_format_t parse_mask = 0;

    FILTER_VVERB(("BEGIN bcm_filter_qualify_format(unit=%d, fid=%d,"
                  " format=%d)\n", unit, bf, format));
    FILTER_INIT(unit);

    f = _bcm_filter_find(unit, bf);
    if (f == NULL) {
        FILTER_ERR(("FFP Error: filter id=%d not found\n", bf));
        return BCM_E_NOT_FOUND;
    }
    if (!soc_feature(unit, soc_feature_filter_extended)) {
        return BCM_E_UNAVAIL;
    } else if (soc_feature(unit, soc_feature_filter_pktfmtext)) {
        if (SOC_IS_TUCANA(unit)) {
            parse_mask =
                BCM_FILTER_PKTFMT_NONIP |
                BCM_FILTER_PKTFMT_IPV6 |
                BCM_FILTER_PKTFMT_IPV4 |
                BCM_FILTER_PKTFMT_DOUBLE_TAG |
                BCM_FILTER_PKTFMT_UNTAG |
                BCM_FILTER_PKTFMT_TAG |
                BCM_FILTER_PKTFMT_802_3 |
                BCM_FILTER_PKTFMT_ETH_II;
            chip_mask = FILTER_PKTFORMAT_ALL_TUCANA;
        } else if (SOC_IS_DRACO15(unit)) {
            parse_mask =
                BCM_FILTER_PKTFMT_NONIP |
                BCM_FILTER_PKTFMT_IPV6 |
                BCM_FILTER_PKTFMT_IPV4 |
                BCM_FILTER_PKTFMT_DOUBLE_TAG |
                BCM_FILTER_PKTFMT_UNTAG |
                BCM_FILTER_PKTFMT_TAG |
                BCM_FILTER_PKTFMT_LLC |
                BCM_FILTER_PKTFMT_SNAP |
                BCM_FILTER_PKTFMT_802_3 |
                BCM_FILTER_PKTFMT_ETH_II;
            chip_mask = FILTER_PKTFORMAT_ALL_DRACO15;
        } else if (SOC_IS_LYNX(unit)) {
            parse_mask =
                BCM_FILTER_PKTFMT_DOUBLE_TAG |
                BCM_FILTER_PKTFMT_UNTAG |
                BCM_FILTER_PKTFMT_TAG |
                BCM_FILTER_PKTFMT_LLC |
                BCM_FILTER_PKTFMT_SNAP |
                BCM_FILTER_PKTFMT_802_3 |
                BCM_FILTER_PKTFMT_ETH_II;
            chip_mask = FILTER_PKTFORMAT_ALL_LYNX;
        } else {
            return BCM_E_INTERNAL;
        }

        if ((format & parse_mask) != format) {
            /* Selections not allowed on unit */
            FILTER_ERR(("FFP Error: selection not allowed on unit=%d\n", unit));
            return BCM_E_PARAM;
        }

        /* Packet header type */
        if (format & BCM_FILTER_PKTFMT_ETH_II) {
            val |= SOC_IS_TUCANA(unit) ?
                FILTER_ENHANCED_PKTFORMAT_TUC_ETH_II :
                FILTER_ENHANCED_PKTFORMAT_ETH_II;

            if (format & (BCM_FILTER_PKTFMT_802_3 |
                          BCM_FILTER_PKTFMT_SNAP | BCM_FILTER_PKTFMT_LLC)) {
                FILTER_ERR(("FFP Error: format=%#x not supported\n", format));
                return BCM_E_PARAM;
            }
#ifdef BCM_TUCANA_SUPPORT
        } else if (SOC_IS_TUCANA(unit)) {
            if (format & (BCM_FILTER_PKTFMT_SNAP|BCM_FILTER_PKTFMT_LLC)) {
                FILTER_ERR(("FFP Error: format=%#x not supported\n", format));
                return BCM_E_PARAM;
            }
            if (format & BCM_FILTER_PKTFMT_802_3) {
                /* 802.3 is the default for Tucana, so don't do anything */
            } else {    /* match any packet format */
                chip_mask &= ~FILTER_ENHANCED_PKTFORMAT_TUC_ETH_II;
            }
#endif /*BCM_TUCANA_SUPPORT*/
        } else if (format & BCM_FILTER_PKTFMT_SNAP) {
            val |= FILTER_ENHANCED_PKTFORMAT_SNAP;

            if (format & BCM_FILTER_PKTFMT_LLC) {
                FILTER_ERR(("FFP Error: format=%#x not supported\n", format));
                return BCM_E_PARAM;
            }
        } else if (format & BCM_FILTER_PKTFMT_LLC) {
            val |= FILTER_ENHANCED_PKTFORMAT_LLC;
        } else if (format & BCM_FILTER_PKTFMT_802_3) {
            /* Union of SNAP & LLC */
            val |= FILTER_ENHANCED_PKTFORMAT_SNAP;
            chip_mask &= ~FILTER_ENHANCED_PKTFORMAT_ETH_II;
        } else {        /* match any packet format */
            chip_mask &= ~(FILTER_ENHANCED_PKTFORMAT_ETH_II |
                           FILTER_ENHANCED_PKTFORMAT_SNAP);
        }

        /*
         * Packet tags:
         * UNTAG: match untagged packets
         * TAG: match single tagged packets (outer_tag for all except Tucana)
         * OUTER_TAG, INNER_TAG, DOUBLE_TAG: various double tag matches
         * UNTAG+TAG: ignore tagged state
         * default is match untagged (for backwards compatibility)
         */

        if (format & BCM_FILTER_PKTFMT_TAG) {   /* single tagged */
            val |= SOC_IS_TUCANA(unit) ?
                FILTER_ENHANCED_PKTFORMAT_TUC_802_1Q :
                FILTER_ENHANCED_PKTFORMAT_OUTER_TAGGED;
        }

        if (format & BCM_FILTER_PKTFMT_INNER_TAG) {
            val |= SOC_IS_TUCANA(unit) ?
                FILTER_ENHANCED_PKTFORMAT_TUC_802_1Q :
                FILTER_ENHANCED_PKTFORMAT_INNER_TAGGED;
        }

        if (format & BCM_FILTER_PKTFMT_OUTER_TAG) {
            val |= SOC_IS_TUCANA(unit) ?
                FILTER_ENHANCED_PKTFORMAT_TUC_OUTER_TAG :
                FILTER_ENHANCED_PKTFORMAT_OUTER_TAGGED;
        }

        /* tagged and untagged: ignore tagged state */
        if ((format & BCM_FILTER_PKTFMT_UNTAG) &&
            (format & BCM_FILTER_PKTFMT_TAG)) {
            if (SOC_IS_TUCANA(unit)) {
                chip_mask &= ~FILTER_ENHANCED_PKTFORMAT_TUC_802_1Q;
                val &= ~FILTER_ENHANCED_PKTFORMAT_TUC_802_1Q;
            } else {
                chip_mask &= ~FILTER_ENHANCED_PKTFORMAT_OUTER_TAGGED;
                val &= ~FILTER_ENHANCED_PKTFORMAT_OUTER_TAGGED;
            }
        }

        /* default tag matching is untagged */

        /*
         * IP packet matching:
         * NONIP: match only packets not ip4 or ip6
         * IP4, IP6: match only those IP format packets
         * else: match all ip and non-ip packets (backwards compatible)
         */
        if (format & BCM_FILTER_PKTFMT_NONIP) {
            if (format & (BCM_FILTER_PKTFMT_IPV4|BCM_FILTER_PKTFMT_IPV6)) {
                FILTER_ERR(("FFP Error: format=%#x not supported\n", format));
                return BCM_E_PARAM;
            }
        } else if (format & BCM_FILTER_PKTFMT_IPV4) {
            val |= SOC_IS_TUCANA(unit) ?
                FILTER_ENHANCED_PKTFORMAT_TUC_IPV4 :
                FILTER_ENHANCED_PKTFORMAT_IPV4;

            if (format & BCM_FILTER_PKTFMT_IPV6) {
                FILTER_ERR(("FFP Error: format=%#x not supported\n", format));
                return BCM_E_PARAM;
            }
        } else if (format & BCM_FILTER_PKTFMT_IPV6) {
            val |= SOC_IS_TUCANA(unit) ?
                FILTER_ENHANCED_PKTFORMAT_TUC_IPV6 :
                FILTER_ENHANCED_PKTFORMAT_IPV6;
        } else {        /* IP format bits become do not cares */
            chip_mask &= SOC_IS_TUCANA(unit) ?
                ~(FILTER_ENHANCED_PKTFORMAT_TUC_IPV4 |
                  FILTER_ENHANCED_PKTFORMAT_TUC_IPV6) :
                ~(FILTER_ENHANCED_PKTFORMAT_IPV4 |
                  FILTER_ENHANCED_PKTFORMAT_IPV6);
        }
    } else {
        chip_mask = FILTER_PKTFORMAT_ALL;

        switch (format) {
        case bcmFormatUntaggedEthII:
            val = FILTER_PKTFORMAT_UNTAGGED_ETH_II;
            break;
        case bcmFormatUntagged802_3:
            val = FILTER_PKTFORMAT_UNTAGGED_802_3;
            break;
        case bcmFormatTaggedEthII:
            val = FILTER_PKTFORMAT_TAGGED_ETH_II;
            break;
        case bcmFormatTagged802_3:
            val = FILTER_PKTFORMAT_TAGGED_802_3;
            break;
        case BCM_FILTER_PKTFMT_ETH_II:          /* tagged or untagged */
            chip_mask = FILTER_PKTFORMAT_TAGGED_802_3 |
                FILTER_PKTFORMAT_UNTAGGED_802_3;
            val = 0;
            break;
        case BCM_FILTER_PKTFMT_802_3:           /* tagged or untagged */
            chip_mask = FILTER_PKTFORMAT_TAGGED_ETH_II |
                FILTER_PKTFORMAT_UNTAGGED_ETH_II;
            val = 0;
            break;
        case BCM_FILTER_PKTFMT_TAG:             /* any tagged pkt */
        case BCM_FILTER_PKTFMT_OUTER_TAG:       /* any tagged pkt */
            chip_mask = FILTER_PKTFORMAT_UNTAGGED_ETH_II |
                FILTER_PKTFORMAT_UNTAGGED_802_3;
            val = 0;
            break;
        case BCM_FILTER_PKTFMT_UNTAG:           /* any untagged pkt */
            chip_mask = FILTER_PKTFORMAT_TAGGED_ETH_II |
                FILTER_PKTFORMAT_TAGGED_802_3;
            val = 0;
            break;
        default:
            FILTER_ERR(("FFP Error: format=%#x not supported\n", format));
            return BCM_E_PARAM;
        }
    }

    soc_mem_field32_set(unit, GFILTER_IMASKm, (uint32 *) &f->mask,
                        PKTFORMATMASKf, chip_mask);

    soc_mem_field32_set(unit, GFILTER_IRULEm, (uint32 *) &f->rule,
                        PKTFORMATf, val);

    return BCM_E_NONE;
}

/*
 * Function: bcm_esw_filter_qualify_data
 *
 * Purpose:
 *      Add data field that the packet must match to trigger filter.
 *
 * Parameters:
 *      unit   - StrataSwitch Unit number.
 *      fid    - Handle to filter template to operate on.
 *      offset - Offset of data field in packet.
 *      len    - Length of data field in packet.
 *      val    - Content of data field in packet.
 *      mask   - Binary mask applied to field in packet
 *               before comparing with val.
 *
 * Return Values:
 *      BCM_E_CONFIG   - Qualifying on bits not in mask
 *      BCM_E_RESOURCE - exceeded ability of hardware filter table
 *
 * Notes:
 *      Due to the confusing nature of filter endian entries, this code
 *      is written as plainly as possible at the expense of performance;
 *      it copies the whole field in and out.
 */
int
bcm_esw_filter_qualify_data(int unit, bcm_filterid_t fid,
                        int offset, int len,
                        const uint8 *val, const uint8 *mask)
{
    filter_cntl_t       *fc = _bcm_filter_control[unit];
    filter_t            *filter;
    int                 byte;

    FILTER_VVERB(
        ("BEGIN bcm_filter_qualify_data(unit=%d, fid=%d, offset=%d, len=%d, "
	 "val=%#x, mask=%#x\n", unit, fid, offset, len, *val, *mask));
    FILTER_INIT(unit);

    filter = _bcm_filter_find(unit, fid);
    if (filter == NULL) {
        FILTER_ERR(("FFP Error: fid=%d not found on unit=%d\n", fid, unit));
        return BCM_E_NOT_FOUND;
    }

    if (offset < 0 || len < 0) {
        FILTER_ERR(("FFP Error: offset=%d or len=%d can't be negative\n",
                    offset, len));
        return BCM_E_PARAM;
    }

    if (offset + len > fc->fc_packet_depth) {
        FILTER_ERR(("FFP Error: Can only look %d bytes into packet\n",
                 fc->fc_packet_depth));
        return BCM_E_RESOURCE;
    }

    for (byte = 0; byte < len; byte++) {
        uint8           conflict_mask;

        /*
         * Catch trying to qualify value bits that aren't in the mask.
         */
        if (val[byte] & ~mask[byte]) {
	    FILTER_ERR(("FFP Error: val[%d]=%#x ~mask[%d]=%#x\n", byte,
                       val[byte], byte, ~mask[byte]));
            return BCM_E_CONFIG;
        }

        /*
         * Catch trying to qualify bits that were previously qualified
         * with different values.
         */

        conflict_mask = filter->data_mask[offset + byte] & mask[byte];

        if (((val[byte]) & conflict_mask) != 
            ((filter->data_val[offset + byte]) & conflict_mask)) {
            FILTER_ERR(("FFP Error: requalifying on already qualified bits\n"));
            return BCM_E_CONFIG;
        }

        /*
         * Record mask and value.
         */

        filter->data_mask[offset + byte] |= mask[byte];
        filter->data_val[offset + byte]  |= val[byte];
    }

    return BCM_E_NONE;
}

/*
 * Function: bcm_esw_filter_qualify_data8/16/32
 *
 * Purpose:
 *      Add a byte/half/word that the packet must match to trigger filter.
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf - Handle to filter template to operate on.
 *      offset - Offset of data field in packet.
 *      val - Content of data field in packet.
 *      mask - Binary mask applied to field in packet
 *              before comparing with val.
 *
 * Return Values:
 *      BCM_E_RESOURCE - exceeded ability of hardware filter table
 */
int
bcm_esw_filter_qualify_data8(int unit, bcm_filterid_t bf,
                         int offset, uint8 val, uint8 mask)
{
    return bcm_esw_filter_qualify_data(unit, bf, offset, 1, &val, &mask);
}

int
bcm_esw_filter_qualify_data16(int unit, bcm_filterid_t bf,
                          int offset, uint16 val, uint16 mask)
{
    uint8       val_buf[2], mask_buf[2];

    val_buf[0] = val >> 8;
    val_buf[1] = val >> 0;

    mask_buf[0] = mask >> 8;
    mask_buf[1] = mask >> 0;

    return bcm_esw_filter_qualify_data(unit, bf, offset, 2, val_buf, mask_buf);
}

int
bcm_esw_filter_qualify_data32(int unit, bcm_filterid_t bf,
                          int offset, uint32 val, uint32 mask)
{
    uint8       val_buf[4], mask_buf[4];

    val_buf[0] = val >> 24;
    val_buf[1] = val >> 16;
    val_buf[2] = val >> 8;
    val_buf[3] = val >> 0;

    mask_buf[0] = mask >> 24;
    mask_buf[1] = mask >> 16;
    mask_buf[2] = mask >> 8;
    mask_buf[3] = mask >> 0;

    return bcm_esw_filter_qualify_data(unit, bf, offset, 4, val_buf, mask_buf);
}

#define CHECK_NZ(ext)           if (! (ext)) { return SOC_E_UNAVAIL; }

/*
 * Function: bcm_esw_filter_action_match
 *
 * Purpose:
 *      Add action to be performed when filter rule is matched for
 *      a packet that is in-profile.
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf - Handle to filter template to operate on.
 *      action - Action to perform (bcmActionXXX)
 *      param - Parameter required by some actions (use 0 if not required)
 *
 * Return Values:
 *      BCM_E_UNAVAIL - function not implemented
 *      BCM_E_RESOURCE - exceeded ability of hardware filter table
 *
 * Notes:
 *      On StrataSwitch, bcmActionSetPrio and bcmActionInsPrio are
 *      performed whether the packet is in-profile or out-of-profile.
 */
int
bcm_esw_filter_action_match(int unit, bcm_filterid_t bf,
                        bcm_filter_action_t action, uint32 param)
{
    filter_t            *f;
    uint32              act;
    int                 ext = soc_feature(unit, soc_feature_filter_extended);
    int                 xgs = soc_feature(unit, soc_feature_filter_xgs);
    int                 tuc = soc_feature(unit, soc_feature_filter_tucana);
    int                 d15 = soc_feature(unit, soc_feature_filter_draco15);

    FILTER_VVERB(("BEGIN bcm_filter_action_match(unit=%d, fid=%d, action=%d, "
                  "param=%d(%#x)\n", unit, bf, action, param, param));
    FILTER_INIT(unit);

    f = _bcm_filter_find(unit, bf);
    if (f == NULL) {
        FILTER_ERR(("FFP Error: fid=%d not found on unit=%d\n", bf, unit));
        return BCM_E_NOT_FOUND;
    }

    act = soc_GFILTER_IRULEm_field32_get(unit, &f->rule, ACTIONf);

    switch (action) {
    case bcmActionCancelAll:
        act = 0;
        break;
    case bcmActionDoNotSwitch:
        act &= ~(tuc ?
                 FILTER_TUCANA_ACTION_DO_SWITCH :
                 FILTER_ACTION_DO_SWITCH);
        act |= FILTER_ACTION_DO_NOT_SWITCH;
        break;
    case bcmActionDoSwitch:
        CHECK_NZ(ext);
        act &= ~FILTER_ACTION_DO_NOT_SWITCH;
        act |= tuc ? FILTER_TUCANA_ACTION_DO_SWITCH :
            FILTER_ACTION_DO_SWITCH;
        break;
    case bcmActionCopyToCpu:
        if (tuc) {
            act &= ~FILTER_TUCANA_ACTION_DO_NOT_COPY_TO_CPU;
        }
        act |= FILTER_ACTION_COPY_TO_CPU;
        break;
    case bcmActionCopyToMirror:
        act |= FILTER_ACTION_COPY_TO_MIRROR;
        break;
    case bcmActionIncrCounter:
        CHECK_NZ(ext);
        act |= FILTER_ACTION_INCR_COUNTER;
        FILTER_RULE_CHK_FIELD_SIZE(COUNTERf, param);
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule, COUNTERf, param);
        break;
    case bcmActionSetPrio:
        act |= FILTER_ACTION_SET_COS_QUEUE;
        FILTER_RULE_CHK_FIELD_SIZE(PRIf, param);
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule, PRIf, param);
        break;
    case bcmActionSetPortUcast:
        act &= ~(tuc ?
                 (FILTER_TUCANA_ACTION_SET_OUTPUT_PORT_ALL |
                  FILTER_TUCANA_ACTION_DO_NOT_SET_OUTPUT_PORT) :
                 FILTER_ACTION_SET_OUTPUT_PORT_ALL);
        act |= FILTER_ACTION_SET_OUTPUT_PORT;
        /*
         * COVERITY
         *
         * The port valid macro is designed to operate on bcm_port_t,
         * which is signed.  Here it is used with the input parameter,
         * which is unsigned.  Thus the check for negative value is
         * unnecessary, but kept as part of the base macro. 
         */ 
        /*    coverity[unsigned_compare]    */
        BCM_FILTER_PORT_VALID(unit, param);
        if (SOC_IS_XGS_SWITCH(unit)) {
            FILTER_RULE_CHK_FIELD_SIZE(DST_PORTf, param);
            soc_GFILTER_IRULEm_field32_set(unit, &f->rule, DST_PORTf, param);
        } else {
            FILTER_RULE_CHK_FIELD_SIZE(PORTf, param);
            soc_GFILTER_IRULEm_field32_set(unit, &f->rule, PORTf, param);
        }
        f->modport_set |= RMODPORT;
        f->rport = param;
        break;
    case bcmActionSetPortNonUcast:
        CHECK_NZ(ext);
        if (tuc) {
            act &= ~FILTER_TUCANA_ACTION_DO_NOT_SET_OUTPUT_PORT;
        }
        act &= ~FILTER_ACTION_SET_OUTPUT_PORT;
        act |= (tuc ?
                FILTER_TUCANA_ACTION_SET_OUTPUT_PORT_ALL :
                FILTER_ACTION_SET_OUTPUT_PORT_ALL);
        /*    coverity[unsigned_compare]    */
        BCM_FILTER_PORT_VALID(unit, param);
        if (SOC_IS_XGS_SWITCH(unit)) {
            FILTER_RULE_CHK_FIELD_SIZE(DST_PORTf, param);
            soc_GFILTER_IRULEm_field32_set(unit, &f->rule, DST_PORTf, param);
        } else {
            FILTER_RULE_CHK_FIELD_SIZE(PORTf, param);
            soc_GFILTER_IRULEm_field32_set(unit, &f->rule, PORTf, param);
        }
        f->modport_set |= RMODPORT;
        f->rport = param;
        break;
    case bcmActionSetPortAll:
        CHECK_NZ(ext);
        if (tuc) {
            act &= ~FILTER_TUCANA_ACTION_DO_NOT_SET_OUTPUT_PORT;
        }
        act |= FILTER_ACTION_SET_OUTPUT_PORT;
        act |= (tuc ?
                FILTER_TUCANA_ACTION_SET_OUTPUT_PORT_ALL :
                FILTER_ACTION_SET_OUTPUT_PORT_ALL);
        /*    coverity[unsigned_compare]    */
        BCM_FILTER_PORT_VALID(unit, param);
        if (SOC_IS_XGS_SWITCH(unit)) {
            FILTER_RULE_CHK_FIELD_SIZE(DST_PORTf, param);
            soc_GFILTER_IRULEm_field32_set(unit, &f->rule, DST_PORTf, param);
        } else {
            FILTER_RULE_CHK_FIELD_SIZE(PORTf, param);
            soc_GFILTER_IRULEm_field32_set(unit, &f->rule, PORTf, param);
        }
        f->modport_set |= RMODPORT;
        f->rport = param;
        break;
    case bcmActionSetPortUntag:
        CHECK_NZ(xgs);
        FILTER_RULE_CHK_FIELD_SIZE(UNTAGf, param);
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule, UNTAGf, param);
        break;
    case bcmActionInsPrio:
        act |= FILTER_ACTION_INSERT_PRIO;
        act &= ~FILTER_ACTION_INSERT_PRIO_FROM_TOSP;
        FILTER_RULE_CHK_FIELD_SIZE(PRIf, param);
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule, PRIf, param);
        break;
    case bcmActionInsPrioFromTOSP:
        CHECK_NZ(ext);
        act &= ~FILTER_ACTION_INSERT_PRIO;
        act |= FILTER_ACTION_INSERT_PRIO_FROM_TOSP;
        break;
    case bcmActionInsTOSP:
        act &= ~FILTER_ACTION_INSERT_TOSP_FROM_PRIO;
        act |= FILTER_ACTION_INSERT_TOSP;
        FILTER_RULE_CHK_FIELD_SIZE(tuc ? DSCPf : TOS_Pf, param);
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule,
                    tuc ? DSCPf : TOS_Pf, param);
        break;
    case bcmActionInsTOSPFromPrio:
        CHECK_NZ(ext);
        act &= ~FILTER_ACTION_INSERT_TOSP;
        act |= FILTER_ACTION_INSERT_TOSP_FROM_PRIO;
        break;
    case bcmActionInsDiffServ:
        CHECK_NZ(ext);
        if (tuc && (act & FILTER_TUCANA_ACTION_INSERT_DIFFSERV_IPV6)) {
            return BCM_E_RESOURCE;
        }
        act |= FILTER_ACTION_INSERT_DIFFSERV;
        FILTER_RULE_CHK_FIELD_SIZE(tuc ? DSCPf : DIFFSERVf, param);
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule,
                    tuc ? DSCPf : DIFFSERVf, param);
        break;
    case bcmActionDropPrecedence:
        CHECK_NZ(xgs);
        act |= (tuc ?
                FILTER_TUCANA_ACTION_DROP_PRECEDENCE :
                FILTER_ACTION_DROP_PRECEDENCE);
        break;
    case bcmActionSetClassificnTag:
        CHECK_NZ(xgs);
        if (tuc) {
            if (act & (FILTER_TUCANA_ACTION_RESOURCE_CONFLICTS ^
                       FILTER_TUCANA_ACTION_SET_CLASSIFICN_TAG)) {
                return BCM_E_RESOURCE;
            }
            act |= FILTER_TUCANA_ACTION_SET_CLASSIFICN_TAG;
        } else {
            if (act & FILTER_ACTION_EGRESS_MASK) {
                return BCM_E_RESOURCE;
            }
            if (d15) {
                if (act & FILTER_DRACO15_ACTION_SET_PORT_BITMAP) {
                    return BCM_E_RESOURCE;
                }
            }
            act |= FILTER_ACTION_SET_CLASSIFICN_TAG;
        }
        FILTER_RULE_CHK_FIELD_SIZE(CLASSIFICATION_TAGf, param);
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule,
                                       CLASSIFICATION_TAGf, param);
        break;
    case bcmActionInsVlanId:
        CHECK_NZ(xgs);
        if (tuc) {
            /* VCLABEL is overlaid with VLANID */
            if (act & FILTER_TUCANA_ACTION_SET_VCLABEL) {
                return BCM_E_RESOURCE;
            }
            act |= FILTER_TUCANA_ACTION_INSERT_VLAN_ID ;
        } else {
            act |= FILTER_ACTION_INSERT_VLAN_ID;
        }
        FILTER_RULE_CHK_FIELD_SIZE(VLANIDf, param);
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule, VLANIDf, param);
        break;
    case bcmActionEgressMask:
        CHECK_NZ(xgs);
        if (tuc) {
            if (act & (FILTER_TUCANA_ACTION_RESOURCE_CONFLICTS ^
                       FILTER_TUCANA_ACTION_EGRESS_MASK)) {
                return BCM_E_RESOURCE;
            }
            act |= FILTER_TUCANA_ACTION_EGRESS_MASK;
            /* EGRESS_MASK is overlaid with PORT_BITMAP */
            FILTER_RULE_CHK_FIELD_SIZE(PORT_BITMAP_M0f, param);
            soc_GFILTER_IRULEm_field32_set(unit, &f->rule,
                                           PORT_BITMAP_M0f, param);
        } else {
            if (act & FILTER_ACTION_SET_CLASSIFICN_TAG) {
                return BCM_E_RESOURCE;
            }
            if (d15) {
                if (act & FILTER_DRACO15_ACTION_SET_PORT_BITMAP) {
                    return BCM_E_RESOURCE;
                }
            }
            act |= FILTER_ACTION_EGRESS_MASK;
            /* EGRESS_MASK is overlaid with CLASSIFICATION_TAG */
            FILTER_RULE_CHK_FIELD_SIZE(CLASSIFICATION_TAGf, param);
            soc_GFILTER_IRULEm_field32_set(unit, &f->rule,
                                           CLASSIFICATION_TAGf, param);
        }
        break;
    case bcmActionEgressMaskHi:
        CHECK_NZ(tuc);
        if (act & (FILTER_TUCANA_ACTION_RESOURCE_CONFLICTS ^
                   FILTER_TUCANA_ACTION_EGRESS_MASK)) {
            return BCM_E_RESOURCE;
        }
        act |= FILTER_TUCANA_ACTION_EGRESS_MASK;
        FILTER_RULE_CHK_FIELD_SIZE(PORT_BITMAP_M1f, param);
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule,
                                       PORT_BITMAP_M1f, param);
        break;
    case bcmActionSetModule:
        CHECK_NZ(xgs);
        /* Requires port redirection action to work */
        BCM_FILTER_MODID_VALID(unit, param);
        FILTER_RULE_CHK_FIELD_SIZE(OUTPUTMODf, param);
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule, OUTPUTMODf, param);
        f->rmodule = param;
        break;
    case bcmActionSetECN:
        CHECK_NZ(d15);
        act |= FILTER_DRACO15_ACTION_SET_ECN;
        FILTER_RULE_CHK_FIELD_SIZE(ECNf, param);
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule, ECNf, param);
        break;
    case bcmActionSetPortBitmap:
        if (d15) {
            if (act & (FILTER_ACTION_SET_CLASSIFICN_TAG |
                       FILTER_ACTION_EGRESS_MASK)) {
                return BCM_E_RESOURCE;
            }
            act |= FILTER_DRACO15_ACTION_SET_PORT_BITMAP;
            FILTER_RULE_CHK_FIELD_SIZE(CLASSIFICATION_TAGf, param);
            soc_GFILTER_IRULEm_field32_set(unit, &f->rule,
                                           CLASSIFICATION_TAGf, param);
        } else if (tuc) {
            if (act & (FILTER_TUCANA_ACTION_RESOURCE_CONFLICTS ^
                       FILTER_TUCANA_ACTION_SET_PORT_BITMAP)) {
                 return BCM_E_RESOURCE;
            }
            act |= FILTER_TUCANA_ACTION_SET_PORT_BITMAP;
            FILTER_RULE_CHK_FIELD_SIZE(PORT_BITMAP_M0f, param);
            soc_GFILTER_IRULEm_field32_set(unit, &f->rule,
                                           PORT_BITMAP_M0f, param);
        } else {
            FILTER_ERR(("FFP Error: SetPortBitmap not available on unit=%d\n",
                     unit));
            return BCM_E_UNAVAIL;
        }
        break;
    case bcmActionSetPortBitmapHi:
        CHECK_NZ(tuc);
        if (act & (FILTER_TUCANA_ACTION_RESOURCE_CONFLICTS ^
                   FILTER_TUCANA_ACTION_SET_PORT_BITMAP)) {
            return BCM_E_RESOURCE;
        }
        act |= FILTER_TUCANA_ACTION_SET_PORT_BITMAP;
        FILTER_RULE_CHK_FIELD_SIZE(PORT_BITMAP_M1f, param);
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule,
                                       PORT_BITMAP_M1f, param);
        break;
    case bcmActionInsDiffServIPv6:
        CHECK_NZ(tuc);
        if (act & FILTER_ACTION_INSERT_DIFFSERV) {
            return BCM_E_RESOURCE;
        }
        act |= FILTER_TUCANA_ACTION_INSERT_DIFFSERV_IPV6;
        FILTER_RULE_CHK_FIELD_SIZE(DSCPf, param);
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule, DSCPf, param);
        break;
    case bcmActionDoNotCopyToCpu:
        CHECK_NZ(tuc);
        act &= ~FILTER_ACTION_COPY_TO_CPU;
        act |= FILTER_TUCANA_ACTION_DO_NOT_COPY_TO_CPU;
        break;
    case bcmActionDoNotSetPortAny:
        CHECK_NZ(tuc);
        act &= ~(FILTER_ACTION_SET_OUTPUT_PORT |
                 FILTER_TUCANA_ACTION_SET_OUTPUT_PORT_ALL);
        act |= FILTER_TUCANA_ACTION_DO_NOT_SET_OUTPUT_PORT;
        f->modport_set &= ~RMODPORT;
        break;
    case bcmActionSetDestMacAddrLo:
        CHECK_NZ(tuc);
        if (act & (FILTER_TUCANA_ACTION_RESOURCE_CONFLICTS ^
                   FILTER_TUCANA_ACTION_SET_DEST_MAC)) {
            return BCM_E_RESOURCE;
        }
        act |= FILTER_TUCANA_ACTION_SET_DEST_MAC;
        FILTER_RULE_CHK_FIELD_SIZE(DEST_ADDR_LOf, param);
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule,
                                       DEST_ADDR_LOf, param);
        break;
    case bcmActionSetDestMacAddrHi:
        CHECK_NZ(tuc);
        if (act & (FILTER_TUCANA_ACTION_RESOURCE_CONFLICTS ^
                   FILTER_TUCANA_ACTION_SET_DEST_MAC)) {
            return BCM_E_RESOURCE;
        }
        act |= FILTER_TUCANA_ACTION_SET_DEST_MAC;
        FILTER_RULE_CHK_FIELD_SIZE(DEST_ADDR_HIf, param);
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule,
                                       DEST_ADDR_HIf, param);
        break;
    case bcmActionSetVclabel:
        CHECK_NZ(tuc);
        if (act & FILTER_TUCANA_ACTION_INSERT_VLAN_ID) {
            return BCM_E_RESOURCE;
        }
        act |= FILTER_TUCANA_ACTION_SET_VCLABEL;
        FILTER_RULE_CHK_FIELD_SIZE(VCLABELf, param);
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule, VCLABELf, param);
        break;
    default:
        FILTER_ERR(("FFP Error: action=%d not supported\n", action));
        return BCM_E_PARAM;
    }

    soc_GFILTER_IRULEm_field32_set(unit, &f->rule, ACTIONf, act);

    return BCM_E_NONE;
}

/*
 * Function: bcm_esw_filter_action_no_match
 *
 * Purpose:
 *      Add action to be performed when filter rule is NOT matched.
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf - Handle to filter template to operate on.
 *      action - Action to perform (bcmActionXXX)
 *      param - Parameter required by some actions (use 0 if not required)
 *
 * Return Values:
 *      BCM_E_UNAVAIL - function not implemented
 *      BCM_E_RESOURCE - exceeded ability of hardware filter table
 *
 * Notes:
 *      There are fewer actions supported for No Match.
 */
int
bcm_esw_filter_action_no_match(int unit, bcm_filterid_t bf,
                           bcm_filter_action_t action, uint32 param)
{
    filter_t            *f;
    uint32              act;
    int                 ext = soc_feature(unit, soc_feature_filter_extended);
    int                 xgs = soc_feature(unit, soc_feature_filter_xgs);
    int                 tuc = soc_feature(unit, soc_feature_filter_tucana);

    FILTER_VVERB(("BEGIN bcm_filter_action_no_match(unit=%d, fid=%d, "
                  "action=%d, param=%d)\n", unit, bf, action, param));
    FILTER_INIT(unit);
    CHECK_NZ(ext);

    f = _bcm_filter_find(unit, bf);
    if (f == NULL) {
        FILTER_ERR(("FFP Error: filter id=%d not found\n", bf));
        return BCM_E_NOT_FOUND;
    }
    act = soc_GFILTER_IMASKm_field32_get(unit, &f->mask, NOMATCHACTIONf);

    switch (action) {
    case bcmActionCancelAll:
        act = 0;
        break;
    case bcmActionDoNotSwitch:
        if (xgs) {
            act &= ~(tuc ?
                     FILTER_TUCANA_ACTION_DO_SWITCH :
                     FILTER_ACTION_DO_SWITCH);
        }
        act |= FILTER_ACTION_DO_NOT_SWITCH;
        break;
    case bcmActionDoSwitch:
        CHECK_NZ(xgs);
        act &= ~FILTER_ACTION_DO_NOT_SWITCH;
        act |= tuc ? FILTER_TUCANA_ACTION_DO_SWITCH :
            FILTER_ACTION_DO_SWITCH;
        break;
    case bcmActionCopyToCpu:
        if (tuc) {
            act &= ~FILTER_TUCANA_ACTION_DO_NOT_COPY_TO_CPU;
        }
        act |= FILTER_ACTION_COPY_TO_CPU;
        break;
    case bcmActionCopyToMirror:
        act |= FILTER_ACTION_COPY_TO_MIRROR;
        break;
    case bcmActionSetPrio:
        act |= FILTER_ACTION_SET_COS_QUEUE;
        FILTER_MASK_CHK_FIELD_SIZE(IEEE802DOT1PRIf, param);
        soc_GFILTER_IMASKm_field32_set(unit, &f->mask,
                                       IEEE802DOT1PRIf, param);
        break;
    case bcmActionSetPortUcast:
        act &= ~(tuc ?
                 (FILTER_TUCANA_ACTION_SET_OUTPUT_PORT_ALL |
                  FILTER_TUCANA_ACTION_DO_NOT_SET_OUTPUT_PORT) :
                 FILTER_ACTION_SET_OUTPUT_PORT_ALL);
        act |= FILTER_ACTION_SET_OUTPUT_PORT;
        /*
         * COVERITY
         *
         * The port valid macro is designed to operate on bcm_port_t,
         * which is signed.  Here it is used with the input parameter,
         * which is unsigned.  Thus the check for negative value is
         * unnecessary, but kept as part of the base macro. 
         */ 
        /*    coverity[unsigned_compare]    */
        BCM_FILTER_PORT_VALID(unit, param);
        FILTER_MASK_CHK_FIELD_SIZE(OUTPUTPORTf, param);
        soc_GFILTER_IMASKm_field32_set(unit, &f->mask, OUTPUTPORTf, param);
        f->modport_set |= MMODPORT;
        f->mport = param;
        break;
    case bcmActionSetPortNonUcast:
        if (tuc) {
            act &= ~FILTER_TUCANA_ACTION_DO_NOT_SET_OUTPUT_PORT;
        }
        act &= ~FILTER_ACTION_SET_OUTPUT_PORT;
        act |= (tuc ?
                FILTER_TUCANA_ACTION_SET_OUTPUT_PORT_ALL :
                FILTER_ACTION_SET_OUTPUT_PORT_ALL);
        /*    coverity[unsigned_compare]    */
        BCM_FILTER_PORT_VALID(unit, param);
        FILTER_MASK_CHK_FIELD_SIZE(OUTPUTPORTf, param);
        soc_GFILTER_IMASKm_field32_set(unit, &f->mask, OUTPUTPORTf, param);
        f->modport_set |= MMODPORT;
        f->mport = param;
        break;
    case bcmActionSetPortAll:
        if (tuc) {
            act &= ~FILTER_TUCANA_ACTION_DO_NOT_SET_OUTPUT_PORT;
        }
        act |= FILTER_ACTION_SET_OUTPUT_PORT;
        act |= (tuc ?
                FILTER_TUCANA_ACTION_SET_OUTPUT_PORT_ALL :
                FILTER_ACTION_SET_OUTPUT_PORT_ALL);
        /*    coverity[unsigned_compare]    */
        BCM_FILTER_PORT_VALID(unit, param);
        FILTER_MASK_CHK_FIELD_SIZE(OUTPUTPORTf, param);
        soc_GFILTER_IMASKm_field32_set(unit, &f->mask, OUTPUTPORTf, param);
        f->modport_set |= MMODPORT;
        f->mport = param;
        break;
    case bcmActionSetPortUntag:
        CHECK_NZ(xgs);
        FILTER_MASK_CHK_FIELD_SIZE(UNTAGf, param);
        soc_GFILTER_IMASKm_field32_set(unit, &f->mask, UNTAGf, param);
        break;
    case bcmActionInsPrio:
        act |= FILTER_ACTION_INSERT_PRIO;
        act &= ~FILTER_ACTION_INSERT_PRIO_FROM_TOSP;
        FILTER_MASK_CHK_FIELD_SIZE(IEEE802DOT1PRIf, param);
        soc_GFILTER_IMASKm_field32_set(unit, &f->mask,
                                       IEEE802DOT1PRIf, param);
        break;
    case bcmActionInsPrioFromTOSP:
        act &= ~FILTER_ACTION_INSERT_PRIO;
        act |= FILTER_ACTION_INSERT_PRIO_FROM_TOSP;
        break;
    case bcmActionInsTOSP:
        act &= ~FILTER_ACTION_INSERT_TOSP_FROM_PRIO;
        act |= FILTER_ACTION_INSERT_TOSP;
        FILTER_MASK_CHK_FIELD_SIZE(tuc ? DSCPf : TOS_Pf, param);
        soc_GFILTER_IMASKm_field32_set(unit, &f->mask,
                    tuc ? DSCPf : TOS_Pf, param);
        break;
    case bcmActionInsTOSPFromPrio:
        act &= ~FILTER_ACTION_INSERT_TOSP;
        act |= FILTER_ACTION_INSERT_TOSP_FROM_PRIO;
        break;
    case bcmActionInsDiffServ:
        if (tuc && (act & FILTER_TUCANA_ACTION_INSERT_DIFFSERV_IPV6)) {
            return BCM_E_RESOURCE;
        }
        act |= FILTER_ACTION_INSERT_DIFFSERV;
        FILTER_MASK_CHK_FIELD_SIZE(tuc ? DSCPf : DIFFSERVf, param);
        soc_GFILTER_IMASKm_field32_set(unit, &f->mask,
                    tuc ? DSCPf : DIFFSERVf, param);
        break;
    case bcmActionDropPrecedence:
        CHECK_NZ(xgs);
        act |= (tuc ?
                FILTER_TUCANA_ACTION_DROP_PRECEDENCE :
                FILTER_ACTION_DROP_PRECEDENCE);
        break;
    case bcmActionSetModule:
        CHECK_NZ(xgs);
        /* Requires port redirection action to work */
        BCM_FILTER_MODID_VALID(unit, param);
        FILTER_MASK_CHK_FIELD_SIZE(OUTPUTMODf, param);
        soc_GFILTER_IMASKm_field32_set(unit, &f->mask, OUTPUTMODf, param);
        f->mmodule = param;
        break;
    case bcmActionInsDiffServIPv6:
        CHECK_NZ(tuc);
        if (act & FILTER_ACTION_INSERT_DIFFSERV) {
            return BCM_E_RESOURCE;
        }
        act |= FILTER_TUCANA_ACTION_INSERT_DIFFSERV_IPV6;
        FILTER_MASK_CHK_FIELD_SIZE(DSCPf, param);
        soc_GFILTER_IMASKm_field32_set(unit, &f->mask, DSCPf, param);
        break;
    case bcmActionDoNotCopyToCpu:
        CHECK_NZ(tuc);
        act &= ~FILTER_ACTION_COPY_TO_CPU;
        act |= FILTER_TUCANA_ACTION_DO_NOT_COPY_TO_CPU;
        break;
    case bcmActionDoNotSetPortAny:
        CHECK_NZ(tuc);
        act &= ~(FILTER_ACTION_SET_OUTPUT_PORT |
                 FILTER_TUCANA_ACTION_SET_OUTPUT_PORT_ALL);
        act |= FILTER_TUCANA_ACTION_DO_NOT_SET_OUTPUT_PORT;
        f->modport_set &= ~MMODPORT;
        break;
    default:
        FILTER_ERR(("FFP Error: action=%d not supported\n", action));
        return BCM_E_PARAM;
    }

    soc_GFILTER_IMASKm_field32_set(unit, &f->mask, NOMATCHACTIONf, act);

    return BCM_E_NONE;
}

/*
 * Function: bcm_esw_filter_action_out_profile
 *
 * Purpose:
 *      Add action to be performed when filter rule is matched for
 *      a packet that is out-of-profile.
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf - Handle to filter template to operate on.
 *      action - Action to perform (bcmActionXXX)
 *      param - Parameter required by some actions (use 0 if not required)
 *      meter_id - indicates which Meter and FFP Packet Counter is used
 *              to determine when the packet is out-of-profile.
 *
 * Return Values:
 *      BCM_E_UNAVAIL - function not implemented
 *      BCM_E_RESOURCE - exceeded ability of hardware filter table
 *
 * Notes:
 *      There are fewer actions supported for Out of Profile matches.
 *      On StrataSwitch, bcmActionSetPrio and bcmActionInsPrio are
 *      performed whether the packet is in-profile or out-of-profile.
 */
int
bcm_esw_filter_action_out_profile(int unit, bcm_filterid_t bf,
                              bcm_filter_action_t action, uint32 param,
                              int meter_id)
{
    filter_cntl_t       *fc = _bcm_filter_control[unit];
    filter_t            *f;
    uint32              act;
    int                 ext = soc_feature(unit, soc_feature_filter_extended);
    int                 xgs = soc_feature(unit, soc_feature_filter_xgs);
    int                 tuc = soc_feature(unit, soc_feature_filter_tucana);
    int                 d15 = soc_feature(unit, soc_feature_filter_draco15);

    FILTER_VVERB(("BEGIN bcm_filter_action_out_profile(unit=%d, fid=%d, "
                  "action=%d, param=%d, meter_id)\n",
                  unit, bf, action, param, meter_id));
    FILTER_INIT(unit);
    CHECK_NZ(ext);

    if ((meter_id < 0) || (meter_id >= fc->fc_meter_size)) {
        return BCM_E_PARAM;
    }

    f = _bcm_filter_find(unit, bf);
    if (f == NULL) {
        FILTER_ERR(("FFP Error: filter id=%d not found\n", bf));
        return BCM_E_NOT_FOUND;
    }
    act = soc_GFILTER_IRULEm_field32_get(unit, &f->rule, OUT_ACTIONSf);

    switch (action) {
    case bcmActionCancelAll:
        act = 0;
        break;
    case bcmActionDoNotSwitch:
        act &= ~(tuc ?
                 FILTER_TUCANA_OUT_ACTION_DO_SWITCH :
                 FILTER_OUT_ACTION_DO_SWITCH);
        act |= FILTER_OUT_ACTION_DO_NOT_SWITCH;
        break;
    case bcmActionDoSwitch:
        act &= ~FILTER_OUT_ACTION_DO_NOT_SWITCH;
        act |= (tuc ? FILTER_TUCANA_OUT_ACTION_DO_SWITCH :
            FILTER_OUT_ACTION_DO_SWITCH);
        break;
    case bcmActionCopyToCpu:
        if (tuc) {
            act &= ~FILTER_TUCANA_OUT_ACTION_DO_NOT_COPY_TO_CPU;
        }
        act |= FILTER_OUT_ACTION_COPY_TO_CPU;
        break;
    case bcmActionInsDiffServ:
        if (tuc && (act & FILTER_TUCANA_OUT_ACTION_INSERT_DIFFSERV_IPV6)) {
            return BCM_E_RESOURCE;
        }
        act |= FILTER_OUT_ACTION_INSERT_DIFFSERV;
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule, OUT_DSCPf, param);
        break;
    case bcmActionDropPrecedence:
        CHECK_NZ(xgs);
        act |= (tuc ?
                FILTER_TUCANA_OUT_ACTION_DROP_PRECEDENCE :
                FILTER_OUT_ACTION_DROP_PRECEDENCE);
        break;
    case bcmActionSetECN:
        CHECK_NZ(d15);
        act |= FILTER_DRACO15_OUT_ACTION_SET_ECN;
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule, OUT_ECNf, param);
        break;
    case bcmActionDoNotCopyToCpu:
        CHECK_NZ(tuc);
        act &= ~FILTER_OUT_ACTION_COPY_TO_CPU;
        act |= FILTER_TUCANA_OUT_ACTION_DO_NOT_COPY_TO_CPU;
        break;
    case bcmActionInsDiffServIPv6:
        CHECK_NZ(tuc);
        if (act & FILTER_OUT_ACTION_INSERT_DIFFSERV) {
            return BCM_E_RESOURCE;
        }
        act |= FILTER_TUCANA_OUT_ACTION_INSERT_DIFFSERV_IPV6;
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule, OUT_DSCPf, param);
        break;
    default:
        FILTER_ERR(("FFP Error: action=%d not supported\n", action));
        return BCM_E_PARAM;
    }

    soc_GFILTER_IRULEm_field32_set(unit, &f->rule, OUT_ACTIONSf, act);
    soc_GFILTER_IRULEm_field32_set(unit, &f->rule, METERIDf, meter_id);

    return BCM_E_NONE;
}

/*
 * Function: _bcm_filter_modmap (internal)
 *
 * Purpose:
 *      Map module/port pairs in rule and mask
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf - Handle to filter.
 *
 * Return Values:
 *      BCM_E_NONE
 */
STATIC int
_bcm_filter_modmap(int unit, filter_t *f)
{
    bcm_module_t        mod_in, mod_out;
    bcm_port_t          port_in, port_out;

    

    if (f->modport_set & RMODPORT) {
        mod_in = f->rmodule;
        port_in = f->rport;
        BCM_IF_ERROR_RETURN
            (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                    mod_in, port_in,
                                    &mod_out, &port_out));
        BCM_FILTER_MODID_VALID(unit, mod_out);
        FILTER_RULE_CHK_FIELD_SIZE(OUTPUTMODf, mod_out);
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule, OUTPUTMODf, mod_out);
        BCM_FILTER_PORT_VALID(unit, port_out);
        if (SOC_IS_XGS_SWITCH(unit)) {
            FILTER_RULE_CHK_FIELD_SIZE(DST_PORTf, port_out);
            soc_GFILTER_IRULEm_field32_set(unit, &f->rule, DST_PORTf,
                                           port_out);
        } else {
            FILTER_RULE_CHK_FIELD_SIZE(PORTf, port_out);
            soc_GFILTER_IRULEm_field32_set(unit, &f->rule, PORTf, port_out);
        }
    }
    if (f->modport_set & MMODPORT) {
        mod_in = f->mmodule;
        port_in = f->mport;
        BCM_IF_ERROR_RETURN
            (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                    mod_in, port_in,
                                    &mod_out, &port_out));
        BCM_FILTER_MODID_VALID(unit, mod_out);
        BCM_FILTER_PORT_VALID(unit, port_out);
        FILTER_MASK_CHK_FIELD_SIZE(OUTPUTMODf, mod_out);
        FILTER_MASK_CHK_FIELD_SIZE(OUTPUTPORTf, port_out);
        soc_GFILTER_IMASKm_field32_set(unit, &f->mask, OUTPUTMODf, mod_out);
        soc_GFILTER_IMASKm_field32_set(unit, &f->mask, OUTPUTPORTf, port_out);
    }
    return BCM_E_NONE;
}

/*
 * Functions:
 *      _bcm_fc_off_std (internal)
 *      _bcm_fc_off_extend (internal)
 *      _bcm_fc_off_xgs (internal)
 *
 * Purpose:
 *      Utility routines to map window number and DATAOFFSET value to
 *      offset within packet (according to tables in reference manual).
 *
 * Parameters:
 *      window - for DATAOFFSETx, window is x-1
 *      dataoff - value of DATAOFFSETx field
 *
 * Return Values:
 *      Offset in packet data
 */
STATIC int
_bcm_fc_off_std(int window, int dataoff)
{
    return 0;
}

STATIC int
_bcm_fc_off_extend(int window, int dataoff)
{
    return (window * 16 + dataoff * 8);
}

STATIC int
_bcm_fc_off_xgs(int window, int dataoff)
{
    /*
     *  (window * 4 + dataoff * (dataoff + 1)) % 80;
     */

    static uint8 off[8][8] = {
        {  0,  2,  6, 12, 20, 30, 42, 56 },
        {  4,  6, 10, 16, 24, 34, 46, 60 },
        {  8, 10, 14, 20, 28, 38, 50, 64 },
        { 12, 14, 18, 24, 32, 42, 54, 68 },
        { 16, 18, 22, 28, 36, 46, 58, 72 },
        { 20, 22, 26, 32, 40, 50, 62, 76 },
        { 24, 26, 30, 36, 44, 54, 66,  0 },
        { 28, 30, 34, 40, 48, 58, 70,  4 },
    };

    return off[window][dataoff];
}

/*
 * Function: _bcm_filter_compress_search (internal)
 *
 * Purpose:
 *      Utility routine to search for DATAOFFSET values.
 *
 * Parameters:
 *      unit - unit number
 *      mask - packet mask, raw bytes of length fc_packet_depth.
 *      dataoff_array - array to accumulate results per window
 *      offset - packet offset
 *      depth - search depth, also used as pointer into dataoff_array
 *
 * Return Values:
 *      0 - no solution found
 *      1 - solution found
 *
 * Notes:
 *      Recursive search reminiscent of 8-Queens is guaranteed to find
 *      a solution if one exists.  The search prefers DATAOFFSET values
 *      as small as possible so entries look better when dumped (zero).
 */

STATIC int
_bcm_filter_compress_search(int unit,
                            uint8 *mask,
                            int *dataoff_array,
                            int offset,
                            int depth)
{
    filter_cntl_t       *fc = _bcm_filter_control[unit];
    int                 dataoff, window_off, window, r;

    while (offset < fc->fc_packet_depth && mask[offset] == 0) {
        offset++;
    }

    if (offset == fc->fc_packet_depth) {
        return 1;       /* Success */
    }

    if (depth == fc->fc_window_count) {
        return 0;       /* Failure */
    }

    for (dataoff = 0; dataoff <= fc->fc_dataoff_max; dataoff++) {
        for (window = 0; window < fc->fc_window_count; window++) {

            if (dataoff_array[window] >= 0) {
                continue;           /* Window already used */
            }

            window_off = (fc->fc_offset_fn)(window, dataoff);

            if (offset >= window_off &&
                offset < window_off + fc->fc_window_size) {
                dataoff_array[window] = dataoff;

                r = _bcm_filter_compress_search(unit,
                                                mask,
                                                dataoff_array,
                                                window_off +
                                                   fc->fc_window_size,
                                                depth + 1);

                if (r != 0) {
                    return r;
                }

                dataoff_array[window] = -1;
            }
        }
    }

    return 0;
}

/*
 * Function:
 *      _bcm_filter_ld32 (internal)
 * Purpose:
 *      Load 4 bytes from arbitarily aligned address as big-endian word
 */

static INLINE uint32
_bcm_filter_ld32(void *addr)
{
    uint8       *a = addr;

    return (a[0] << 24 | a[1] << 16 | a[2] << 8 | a[3] << 0);
}

/*
 * Function: _bcm_filter_compress (internal)
 *
 * Purpose:
 *      Utility routine to take raw value/mask data and write it into
 *      the IRULE.FILTER and IMASK.FMASK fields along with DATAOFFSETx.
 *      5690: Compresses 80 bytes into 32.
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf - Handle to filter.
 *      blk - Block to customize entry for.
 *
 * Return Values:
 *      BCM_E_RESOURCE - data can't be compressed sufficiently.
 */

STATIC int
_bcm_filter_compress(int unit, filter_t *f, int blk)
{
    filter_cntl_t       *fc = _bcm_filter_control[unit];
    uint32              field_mask[FILTER_BYTES_MAX / 4];
    uint32              field_val[FILTER_BYTES_MAX / 4];
    int                 window, offset, dataoff, r;
    int                 dataoff_array[WINDOW_MAX];
    int                 have_dataoff, filter_word_max;
    int                 window_words, window_word_start, word;

    have_dataoff = (soc_feature(unit, soc_feature_filter_extended) |
                    soc_feature(unit, soc_feature_filter_xgs));

    sal_memset(field_mask, 0, sizeof (field_mask));
    sal_memset(field_val, 0, sizeof (field_val));

    for (window = 0; window < fc->fc_window_count; window++) {
        dataoff_array[window] = -1;
    }

    r = _bcm_filter_compress_search(unit, f->data_mask, dataoff_array, 0, 0);

    if (r < 0) {
        return r;
    }

    if (r == 0) {
        return BCM_E_RESOURCE;
    }

    filter_word_max = (fc->fc_window_count * fc->fc_window_size) / 4 - 1;
    window_words = fc->fc_window_size / 4;

    for (window = 0; window < fc->fc_window_count; window++) {
        if ((dataoff = dataoff_array[window]) >= 0) {
            offset = (fc->fc_offset_fn)(window, dataoff);

            if (have_dataoff) {
                soc_mem_field32_set(unit, fc->fc_blk2mask[blk], &f->mask,
                                    _bcm_filter_win2field[window], dataoff);
            }

            /*
             * Note that in the mask/value field, words go in reverse
             * and are in native endian.
             */

            window_word_start = window * window_words;

            for (word = 0; word < window_words; word++) {
                field_mask[filter_word_max - (window_word_start + word)] =
                    _bcm_filter_ld32(&f->data_mask[offset + word * 4]);
                field_val [filter_word_max - (window_word_start + word)] =
                    _bcm_filter_ld32(&f->data_val [offset + word * 4]);
            }
        }
    }

    soc_mem_field_set(unit, GFILTER_IMASKm,
                      (uint32 *) &f->mask, FMASKf, field_mask);

    soc_mem_field_set(unit, GFILTER_IRULEm,
                      (uint32 *) &f->rule, FILTERf, field_val);

    if (SOC_BLOCK_IS_CMP(unit, blk, SOC_BLK_XPIC)) {
        soc_mem_field32_set(unit, GFILTER_IMASKm,
                          (uint32 *) &f->mask, VALIDMASKf, 1);
        soc_mem_field32_set(unit, GFILTER_IRULEm,
                          (uint32 *) &f->rule, VALIDf, 1);
    }

    return BCM_E_NONE;
}

/*
 * Function: _bcm_filter_op_install (internal)
 *
 * Purpose:
 *      Utility routine to install a completed entry.
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf - Handle to filter to install.
 *      blk - Block to operate on.
 *
 * Return Values:
 *      BCM_E_FULL - ran out of masks or rules
 *
 * Notes:
 *      Assumes the ingress and egress fields are taken care of.
 */
STATIC int
_bcm_filter_op_install(int unit, filter_t *f, int blk)
{
    filter_cntl_t       *fc = _bcm_filter_control[unit];
    int                 index, rv, lk_idx;
    irule_entry_t       entry_data;

    FILTER_VVERB(("BEGIN _bcm_filter_op_install(unit=%d, f=%#x, blk=%d\n",
                  unit, f, blk));

    /* map module/port if needed */
    if (f->modport_set) {
        BCM_IF_ERROR_RETURN(_bcm_filter_modmap(unit, f));
    }

    BCM_IF_ERROR_RETURN
        (_bcm_filter_compress(unit, f, blk));

    if ((index = _bcm_filter_mask_alloc(unit, blk,
                                        &f->mask, f->prio)) < 0) {
        return index;
    }

    soc_mem_field32_set(unit, fc->fc_blk2rule[blk], &f->rule, FSELf,
                        _bcm_filter_mask_index_to_hw_index(unit, index));

    /* Make sure we don't already have a conflicting rule */

    rv = soc_mem_search(unit,
                        fc->fc_blk2rule[blk],
                        blk,
                        &lk_idx, &f->rule, &entry_data, 0);

    if (rv == SOC_E_NONE) {
        (void)_bcm_filter_mask_free(unit, blk, index);
        FILTER_ERR(("FFP Error: conflicting rule exists\n"));
        return BCM_E_EXISTS;
    }

    if (rv != SOC_E_NOT_FOUND && rv != SOC_MEM_PARTIAL_MATCH) {
        /* Other error */
        (void)_bcm_filter_mask_free(unit, blk, index);
        FILTER_ERR(("FFP Error: not found or partical match.\n"));
        return rv;
    }

    rv = soc_mem_insert(unit, fc->fc_blk2rule[blk],
                        blk, &f->rule);

    if (rv < 0) {
        (void)_bcm_filter_mask_free(unit, blk, index);
        FILTER_ERR(("FFP Error: insertion failure.\n"));
        return rv;
    }

    FILTER_VVERB(("END _bcm_filter_op_install: index=%d\n", rv));

    f->del_max++;

    return BCM_E_NONE;
}

/*
 * Function: _bcm_filter_op_reinstall (internal)
 *
 * Purpose:
 *      Utility routine to re-install a completed entry
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf - Handle to filter to re-install.
 *      blk - Block to operate on.
 *
 * Return Values:
 *      BCM_E_FULL - ran out of masks or rules
 *
 * Notes:
 *      Assumes the ingress and egress fields are taken care of.
 *
 *      Reinstall is permitted only if the required mask is identical,
 *      and filter search key is identical.  In other words, it can only
 *      change action bits (not including no match actions).
 */
STATIC int
_bcm_filter_op_reinstall(int unit, filter_t *f, int blk)
{
    filter_cntl_t       *fc = _bcm_filter_control[unit];
    int                 rule_index, mask_index, rv;
    irule_entry_t       entry_data;
#ifdef BCM_TUCANA_SUPPORT
    uint32              iport_direction;
#endif /*BCM_TUCANA_SUPPORT*/

    FILTER_VERB(("_bcm_filter_op_reinstall: blk=%d\n", blk));

    /* map module/port if needed */
    if (f->modport_set) {
        BCM_IF_ERROR_RETURN(_bcm_filter_modmap(unit, f));
    }

    BCM_IF_ERROR_RETURN
        (_bcm_filter_compress(unit, f, blk));

    /* Sanity check that mask exists */

    if ((mask_index =
         _bcm_filter_mask_index(unit, blk, &f->mask, f->prio)) < 0) {
        FILTER_ERR(("FFP Error: "
            "_bcm_filter_op_reinstall: filter=%d not found\n", f->filterid));
        return BCM_E_NOT_FOUND;
    }

    soc_mem_field32_set(unit, fc->fc_blk2rule[blk], &f->rule, FSELf,
                    _bcm_filter_mask_index_to_hw_index(unit, mask_index));

    /* Find existing rule */

    rv = soc_mem_search(unit, fc->fc_blk2rule[blk], blk,
                        &rule_index, &f->rule, &entry_data, 0);

    SOC_IF_ERROR_RETURN(rv);

#ifdef BCM_TUCANA_SUPPORT
    /* Catch case where software entry is inconsistent with HW
     * IPORT_DIRECTION vector. */
    if (SOC_IS_TUCANA(unit)) {
        /* Get the physical entry's iport direction vector. */
        iport_direction = soc_GFILTER_IRULEm_field32_get(unit, &entry_data,
                                                     IPORT_DIRECTIONf);

        /* Set the software entry's iport direction vector to match physical
         * entry. */
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule, IPORT_DIRECTIONf,
                                       iport_direction);
    }
#endif /*BCM_TUCANA_SUPPORT*/

    FILTER_VERB(("_bcm_filter_op_reinstall: rule=%d mask=%d\n",
                 rule_index, mask_index));

    SOC_IF_ERROR_RETURN
        (soc_mem_write(unit, fc->fc_blk2rule[blk],
                       blk, rule_index, &f->rule));

    return BCM_E_NONE;
}

/*
 * Function: _bcm_filter_op_remove (internal)
 *
 * Purpose:
 *      Utility routine to remove a completed entry
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf - Handle to filter to remove.
 *      blk - Block to operate on.
 *
 * Return Values:
 *      BCM_E_FULL      - ran out of masks or rules
 *      BCM_E_NOT_FOUND - filter not found
 *      BCM_E_NONE      - Success
 *
 * Notes:
 *      Assumes the ingress and egress fields are taken care of.
 */
STATIC int
_bcm_filter_op_remove(int unit, filter_t *f, int blk)
{
    filter_cntl_t       *fc = _bcm_filter_control[unit];
    int                 index;

    FILTER_VERB(("_bcm_filter_op_remove: blk=%d\n", blk));

    if (--f->del_max < 0) {
        return BCM_E_NONE;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_filter_compress(unit, f, blk));

    if ((index = _bcm_filter_mask_index(unit, blk, &f->mask, f->prio)) < 0) {
        FILTER_ERR(("FFP Error: filter not found\n"));
        return BCM_E_NOT_FOUND;
    }

    soc_mem_field32_set(unit, fc->fc_blk2rule[blk], &f->rule, FSELf,
                        _bcm_filter_mask_index_to_hw_index(unit, index));

    SOC_IF_ERROR_RETURN
        (soc_mem_delete(unit,
                        fc->fc_blk2rule[blk],
                        blk,
                        &f->rule));

    BCM_IF_ERROR_RETURN
        (_bcm_filter_mask_free(unit, blk, index));

    return BCM_E_NONE;
}

#define FILTER_OP_INSTALL       0
#define FILTER_OP_REINSTALL     1
#define FILTER_OP_REMOVE        2

/*
 * Function: _bcm_filter_op_eport (internal)
 *
 * Purpose:
 *      Utility routine to install/reinstall/remove for selected
 *      egress port(s).
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf - Handle to filter to install/reinstall/remove.
 *      blk - Block to operate on.
 *      op - FILTER_OP_INSTALL, FILTER_OP_REINSTALL, or FILTER_OP_REMOVE
 *      eport - EPORT field, -1 for All Egresses
 *
 * Return Values:
 *      BCM_E_FULL - ran out of masks or rules
 *
 * Notes:
 *      Fills in the egress field and calls correct execution routine.
 */
STATIC int
_bcm_filter_op_eport(int unit, filter_t *f, int blk, int op, bcm_port_t eport)
{
    int                 rv;

    if (eport < 0) {
        soc_GFILTER_IMASKm_field32_set(unit, &f->mask, EGRMASKf, 0);
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule, EPORTf, 0);
    } else {
        uint32          em;

        em = FIELD_MASK(unit, GFILTER_IMASKm, EGRMASKf);

        BCM_FILTER_PORT_VALID(unit, eport);
        FILTER_MASK_CHK_FIELD_SIZE(EGRMASKf, em);
        FILTER_RULE_CHK_FIELD_SIZE(EPORTf, eport);
        soc_GFILTER_IMASKm_field32_set(unit, &f->mask, EGRMASKf, em);
        soc_GFILTER_IRULEm_field32_set(unit, &f->rule, EPORTf, eport);
    }

    switch (op) {
    case FILTER_OP_INSTALL:
        rv = _bcm_filter_op_install(unit, f, blk);
        break;
    case FILTER_OP_REINSTALL:
        rv = _bcm_filter_op_reinstall(unit, f, blk);
        break;
    case FILTER_OP_REMOVE:
        rv = _bcm_filter_op_remove(unit, f, blk);
        break;
    default:
        assert(0);
        rv = 0;
        break;
    }

    return rv;
}

/*
 * Function: _bcm_filter_op_iport (internal)
 *
 * Purpose:
 *      Utility routine to install/reinstall/remove for selected
 *      ingress port(s).
 *
 * Parameters:
 *      unit  - StrataSwitch Unit number.
 *      f     - Handle to filter to install/reinstall/remove.
 *      blk   - Block to operate on.
 *      op    - FILTER_OP_INSTALL, FILTER_OP_REINSTALL, or FILTER_OP_REMOVE
 *      iport - IPORT field, -1 for All Ingresses
 *
 * Return Values:
 *      BCM_E_FULL - ran out of masks or rules
 *
 * Notes:
 *      Fills in the ingress field and calls egress install.
 */
STATIC int
_bcm_filter_op_iport(int unit, filter_t *f, int blk, int op, bcm_port_t iport)
{
    int                 rv;
#ifdef BCM_TUCANA_SUPPORT
    pbmp_t              pbm, bbm;
    uint32              bitmap = 0;
    int                 bmp_word = 0, bmp_byte = 0;
#endif /*BCM_TUCANA_SUPPORT*/

    /*
     * If all ingresses, leave INGMASK and IPORT both zero
     */

#ifdef BCM_TUCANA_SUPPORT
    if (SOC_IS_TUCANA(unit)) {
        if (SOC_BLOCK_IS_CMP(unit, blk, SOC_BLK_GPIC) ||
            soc_GFILTER_IMASKm_field32_get(unit, &f->mask,
                                           NOMATCHACTIONf) != 0) {
            /* GPIC, or checking partial match: use IPORT_MODE=0 (iport) */
            if (iport < 0) {
                soc_GFILTER_IMASKm_field32_set(unit, &f->mask, INGMASKf, 0);
                soc_GFILTER_IRULEm_field32_set(unit, &f->rule, IPORTf, 0);
            } else {
                BCM_FILTER_PORT_VALID(unit, iport);
                FILTER_RULE_CHK_FIELD_SIZE(IPORTf, SOC_PORT_BINDEX(unit, iport));
                soc_GFILTER_IMASKm_field32_set(unit, &f->mask, INGMASKf, 1);
                soc_GFILTER_IRULEm_field32_set(unit, &f->rule, IPORTf,
                                               SOC_PORT_BINDEX(unit, iport));
            }
            soc_GFILTER_IMASKm_field32_set(unit, &f->mask, IPORT_MODEf, 0);
            soc_GFILTER_IRULEm_field32_set(unit, &f->rule, IPORT_MODEf, 0);
        } else {
            /* EPIC, no partial matches: use IPORT_MODE=1 (bitmap) */
            SOC_PBMP_ASSIGN(pbm, f->ipbm);
            SOC_PBMP_ASSIGN(bbm, SOC_BLOCK_BITMAP(unit, blk));
            SOC_PBMP_AND(pbm, bbm);

            while (!SOC_PBMP_WORD_GET(bbm, bmp_word)) {
                bmp_word++;
                if (bmp_word > SOC_PBMP_WORD_MAX) {
                    FILTER_ERR(("FFP Error: bmp_word=%d out of range\n",
                                bmp_word));
                    return BCM_E_PARAM; /* Or should this assert? */
                }
            }

            bitmap = SOC_PBMP_WORD_GET(bbm, bmp_word);

            while (!(bitmap & 0xff)) {
                bitmap >>= 8;
                bmp_byte++;
                if (bmp_byte > 3) {
                    FILTER_ERR(("FFP Error: bmp_byte=%d out of range\n",
                                bmp_byte));
                    return BCM_E_PARAM; /* Or should this assert? */
                }
            }

            bitmap = SOC_PBMP_WORD_GET(pbm, bmp_word) >> (8 * bmp_byte);

            soc_GFILTER_IMASKm_field32_set(unit, &f->mask, IPORT_MODEf, 1);
            soc_GFILTER_IRULEm_field32_set(unit, &f->rule, IPORT_MODEf, 1);
            soc_GFILTER_IRULEm_field32_set(unit, &f->rule, IPORT_BITMAPf,
                                           bitmap);
        }
    }
#endif /*BCM_TUCANA_SUPPORT*/

    if (f->eport_special != 0) {
        rv = _bcm_filter_op_eport(unit, f, blk, op, f->eport_special);
    } else if (SOC_PBMP_EQ(f->epbm, PBMP_ALL(unit))) {
        rv = _bcm_filter_op_eport(unit, f, blk, op, -1);
    } else {
        bcm_port_t      eport;
        int             r;

        rv = BCM_E_NONE;

        PBMP_ITER(f->epbm, eport) {
            r = _bcm_filter_op_eport(unit, f, blk, op, eport);

            if (r < 0 && rv == BCM_E_NONE) {
                FILTER_ERR(("FFP Error: fid=%d, blk=%#x, op=%d, eport=%d\n",
                            f, blk, op, eport));
                rv = r;
            }
        }
    }

    return rv;
}

/*
 * Function: _bcm_filter_op (internal)
 *
 * Purpose:
 *      Utility routine to install/reinstall/remove
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf - Handle to filter to install/reinstall/remove.
 *      op - FILTER_OP_INSTALL, FILTER_OP_REINSTALL, or FILTER_OP_REMOVE
 *
 * Return Values:
 *      BCM_E_FULL - ran out of masks or rules
 */
STATIC int
_bcm_filter_op(int unit, filter_t *f, int op)
{
    filter_cntl_t       *fc = _bcm_filter_control[unit];
    int                 blk, rv, r;
    pbmp_t              pbm;
    bcm_port_t          iport;

    /*
     * The ingress pbmp determines in which blocks to install the
     * rules/masks.  Only one rule is needed if all ports of an EPIC are
     * specified.  Otherwise, one rule per port is needed for that EPIC.
     */

    rv = BCM_E_NONE;

    SOC_BLOCK_ITER(unit, blk, SOC_BLK_ETHER) {
        if (fc->fc_blk2rule[blk] == 0) {                /* never initialized */
            continue;
        }
        SOC_PBMP_ASSIGN(pbm, f->ipbm);
        SOC_PBMP_AND(pbm, SOC_BLOCK_BITMAP(unit, blk));

        if (SOC_PBMP_IS_NULL(pbm)) {                    /* no ports in blk */
            continue;
        }

#ifdef BCM_TUCANA_SUPPORT
        if (SOC_IS_TUCANA(unit) &&
            (soc_GFILTER_IMASKm_field32_get(unit, &f->mask,
                                            NOMATCHACTIONf) == 0)) {
            /* Can use IPORT_BITMAP mode for EPICs */
            if ((r = _bcm_filter_op_iport(unit, f, blk, op, -1)) < 0 &&
                rv == BCM_E_NONE) {
                rv = r;
            }
            continue;
        }
#endif /*BCM_TUCANA_SUPPORT*/
        /* all ports in blk */
        if (SOC_PBMP_EQ(pbm, SOC_BLOCK_BITMAP(unit, blk))) {
            if ((r = _bcm_filter_op_iport(unit, f, blk, op, -1)) < 0 &&
                rv == BCM_E_NONE) {
                rv = r;
            }
            continue;
        }

        /* selected ports in blk */
        PBMP_ITER(pbm, iport) {
            if ((r = _bcm_filter_op_iport(unit, f, blk, op, iport)) < 0 &&
                rv == BCM_E_NONE) {
                rv = r;
            }
        }
    }
    return rv;
}

/*
 * Function: bcm_esw_filter_install
 *
 * Purpose:
 *      Install a filter into the hardware tables.
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf   - Handle to filter template describing filter to install.
 *
 * Return Values:
 *      BCM_E_RESOURCE  - exceeded ability of hardware filter table
 *      BCM_E_FULL      - ran out of masks or rules
 *      BCM_E_NOT_FOUND - bf not on Unit
 *
 * Notes:
 *      Qualifications and actions should be assigned prior to installing.
 *      The bcm_filterid_t object is not consumed or destroyed.  It can
 *      be used to remove the filter or it can be destroyed.
 */
int
bcm_esw_filter_install(int unit, bcm_filterid_t bf)
{
    filter_t            *filter;
    int                 rv;

#ifdef BROADCOM_DEBUG
    if (soc_cm_debug_check(DK_VERBOSE)) {
        soc_cm_print("bcm_filter_install: ");
        bcm_esw_filter_dump(unit, bf);
    }
#endif   /* BROADCOM_DEBUG */
    FILTER_VVERB(("BEGIN bcm_filter_install(unit=%d, fid=%d\n", unit, bf));
    FILTER_INIT(unit);

    filter = _bcm_filter_find(unit, bf);
    if (filter == NULL) {
        FILTER_ERR(("FFP Error: fid=%d not found on unit=%d\n", bf, unit));
        return BCM_E_NOT_FOUND;
    }
    FILTER_LOCK(unit);

    filter->del_max = 0;

    rv = _bcm_filter_op(unit, filter, FILTER_OP_INSTALL);

    if (BCM_FAILURE(rv)) {
        /* Clean up; only tries to remove up to f->del_max entries */
        (void)_bcm_filter_op(unit, filter, FILTER_OP_REMOVE);
        FILTER_ERR(("FFP Error: _bcm_filter_op(FILTER_OP_INSTALL) failed.\n"));
    }

    FILTER_UNLOCK(unit);

    return rv;
}

/*
 * Function: bcm_esw_filter_reinstall
 *
 * Purpose:
 *      Re-install a filter into the hardware tables.
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      bf - Handle to filter template describing filter to re-install.
 *
 * Return Values:
 *      BCM_E_NOT_FOUND -
 *              could not find previously installed filter that matches
 *
 * Notes:
 *      Reinstall allows actions (only) to be updated without ever
 *      removing the filter.  This call will fail unless a filter is
 *      found with qualifications that match exactly.
 *      This includes Tucana, where the IPORT_BITMAP must exactly match
 *      when in that mode.
 *      The bcm_filterid_t object is not consumed or destroyed.  It can
 *      be used to remove the filter or it can be destroyed.
 */
int
bcm_esw_filter_reinstall(int unit, bcm_filterid_t bf)
{
    filter_t            *f;
    int                 rv;

#ifdef BROADCOM_DEBUG
    if (soc_cm_debug_check(DK_VERBOSE)) {
        soc_cm_print("bcm_filter_reinstall: ");
        bcm_esw_filter_dump(unit, bf);
    }
#endif /* BROADCOM_DEBUG */

    FILTER_VVERB(("BEGIN bcm_filter_reinstall(unit=%d, fid=%d\n", unit, bf));
    FILTER_INIT(unit);
    f = _bcm_filter_find(unit, bf);
    if (f == NULL) {
        FILTER_ERR(("FFP Error: fid=%d not found\n", bf));
        return BCM_E_NOT_FOUND;
    }
    FILTER_LOCK(unit);

    rv = _bcm_filter_op(unit, f, FILTER_OP_REINSTALL);

    FILTER_UNLOCK(unit);

    return rv;
}

/*
 * Function: bcm_esw_filter_remove
 *
 * Purpose:
 *      Remove a filter from the hardware tables.
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *      fid  - Handle to filter template describing filter to remove.
 *
 * Returns:
 *      BCM_E_NONE      - Success
 *      BCM_E_UNIT      - Invalid unit number
 *      BCM_E_INIT      - Filter not initialized on unit
 *      BCM_E_UNAVAIL   - Filter not supported on unit
 *      BCM_E_NOT_FOUND - Filter ID not on Unit
 *
 */
int
bcm_esw_filter_remove(int unit, bcm_filterid_t fid)
{
    filter_t            *filter;
    int                 rv;

#ifdef BROADCOM_DEBUG
    if (soc_cm_debug_check(DK_VERBOSE)) {
        soc_cm_print("bcm_filter_remove: ");
        bcm_esw_filter_dump(unit, fid);
    }
#endif /* BROADCOM_DEBUG */

    FILTER_VVERB(("BEGIN bcm_filter_remove(unit=%d, fid=%d\n", unit, fid));
    FILTER_INIT(unit);
    filter = _bcm_filter_find(unit, fid);
    if (filter == NULL) {
        FILTER_ERR(("FFP Error: fid=%d not found\n", fid));
        return BCM_E_NOT_FOUND;
    }
    FILTER_LOCK(unit);

    filter->del_max = 9999;  /* No limit to # entries deleted */

    rv = _bcm_filter_op(unit, filter, FILTER_OP_REMOVE);

    FILTER_UNLOCK(unit);

    return rv;
}

/*
 * Function: bcm_esw_filter_remove_all
 *
 * Purpose:
 *      Quickly remove all filters from the hardware tables.
 *
 * Parameters:
 *      unit - StrataSwitch Unit number.
 *
 * Notes:
 *      This function quickly clears all filters on a given unit in three
 *      steps:
 *           1) Clears all mask-related fields in the filter_cntl_t structure
 *              and filter_blk_t structures
 *           2) Clears (zeroes out) GIMASK tables on all blocks
 *           3) Clears (zeroes out) GIRULE tables on all blocks
 */
int
bcm_esw_filter_remove_all(int unit)
{
    int                  blk;
    int                  index, rv;
    filter_cntl_t       *fc;
    filter_blk_t        *fp;
    filter_mask_entry_t *fm;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_INIT;
    }

    fc = _bcm_filter_control[unit];
    FILTER_VVERB(("BEGIN bcm_filter_remove_all(unit=%d)\n", unit));
    FILTER_INIT(unit);

    FILTER_LOCK(unit);

    SOC_BLOCK_ITER(unit, blk, SOC_BLK_ETHER) {
        if (fc->fc_blk2rule[blk] == 0 ||
            fc->fc_blk2mask[blk] == 0 ||
            fc->fc_table[blk] == NULL) {
            continue;
        }

        rv = soc_mem_clear(unit, fc->fc_blk2mask[blk], blk,TRUE);
        if (BCM_FAILURE(rv)) {
            FILTER_UNLOCK(unit);
            return rv;
        }

        fp = fc->fc_table[blk];

        for (index = 0; index < fp->fp_mask_cnt; index++) {
            fm = &fp->fp_mask[index];
            fm->fm_cnt  = 0;
            fm->fm_prio = 0;
        }

        fp->fp_mask_used = 0;

        rv = soc_mem_clear(unit, fc->fc_blk2rule[blk], blk,TRUE);
        if (BCM_FAILURE(rv)) {
            FILTER_UNLOCK(unit);
            return rv;
        }

    }

    FILTER_UNLOCK(unit);

    return BCM_E_NONE;
}

/*
 * Function: _bcm_esw_filter_detach (internal)
 *
 * Purpose:
 *      Clear a unit's filter control state.  Typically run when an error
 *      during unit initialization occurs.
 *
 * Parameters:
 *      unit    - device unit number
 *
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_esw_filter_detach(int unit)
{
    filter_cntl_t       *fc;

    fc = _bcm_filter_control[unit];
    if (fc != NULL) {
        int blk;

        SOC_BLOCK_ITER(unit, blk, SOC_BLK_ETHER) {
            if (NULL !=  fc->fc_table[blk]) {
                sal_free (fc->fc_table[blk]);
            }
        }

        if (fc->fc_lock != NULL) {
            sal_mutex_destroy(fc->fc_lock);
        }
        sal_free(fc);
        _bcm_filter_control[unit] = NULL;
    }
    return BCM_E_NONE;
}

/*
 * Function: bcm_esw_filter_init
 *
 * Purpose:
 *      Initialize filter module
 *
 * Parameters:
 *      unit - StrataSwitch unit #.
 *
 * Returns:
 *      BCM_E_UNAVAIL - Filter unavailable on this device
 *      BCM_E_MEMORY  - Memory allocation failure
 *      BCM_E_XXX     - Other errors
 *      BCM_E_NONE    - Success
 */
int
bcm_esw_filter_init(int unit)
{
    filter_cntl_t       *fc;
    int                 blk;
    int                 filter_enable;
    int                 mask_count;
    int                 rv;
#ifdef BCM_TUCANA_SUPPORT
    int                 port;
    uint64              ffp_config;
#endif /*BCM_TUCANA_SUPPORT*/

    FILTER_VVERB(("BEGIN _bcm_filter_init(unit=%d)\n", unit));

    if (!soc_feature(unit, soc_feature_filter)) {
        return BCM_E_UNAVAIL;
    }

    _bcm_esw_filter_detach(unit);

    fc = _bcm_filter_control[unit];
    if (fc == NULL) {
        fc = sal_alloc(sizeof (*fc), "filter_cntl");
        if (fc == NULL) {
            return BCM_E_MEMORY;
        }
        sal_memset(fc, 0, sizeof (*fc));
        fc->fc_lock = sal_mutex_create("filter_cntl");
        if (fc->fc_lock == NULL) {
            sal_free(fc);
            return BCM_E_MEMORY;
        }
        _bcm_filter_control[unit] = fc;
    }

    /*
     * Determine whether filtering will be enabled and set it in the
     * hardware.  If disabled, some software structures will still be
     * initialized but the hardware table entries won't be.
     *
     * Initialization must be done in the order:
     *    (1) soc_filter_enable_set
     *    (2) clear mask table
     *    (3) clear rules table
     */

    filter_enable = soc_property_get(unit, spn_FILTER_ENABLE, 1);
    rv = soc_filter_enable_set(unit, filter_enable);
    if (rv < 0) {
        _bcm_esw_filter_detach(unit);
        return rv;
    }

    /*
     * Per-block initialization
     */

    SOC_BLOCK_ITER(unit, blk, SOC_BLK_ETHER) {
        switch (SOC_BLOCK_INFO(unit, blk).type) {
        case SOC_BLK_EPIC:
            fc->fc_blk2rule[blk] = FILTER_IRULEm;
            fc->fc_blk2mask[blk] = FILTER_IMASKm;
            break;
        case SOC_BLK_GPIC:
            fc->fc_blk2rule[blk] = GFILTER_IRULEm;
            fc->fc_blk2mask[blk] = GFILTER_IMASKm;
            break;
        case SOC_BLK_XPIC:
            fc->fc_blk2rule[blk] = GFILTER_IRULEm;
            fc->fc_blk2mask[blk] = GFILTER_IMASKm;
            break;
        default:
            FILTER_ERR(("unit %d: filtering not supported on %s ports",
                         unit, SOC_BLOCK_NAME(unit, blk)));
            _bcm_esw_filter_detach(unit);
            return BCM_E_UNAVAIL;
        }
    }

    /*
     * Initialize all filter entries.
     *
     * Write zeros to the invalid indexes of the irule table.
     * (Although hardware does not actually look at these.)
     *
     * Write zeroes to the imask table.  The soc_mem_clear routine
     * also clears the soc_fm_t pointers in SOC_PERSIST.
     */

    if (filter_enable) {
        /*
         * Properties are provided to disable the lengthy filter table
         * initialization, for use in simulation environments when
         * filters are not involved.
         */

        if (NUM_FE_PORT(unit) > 0) {
            if (soc_property_get(unit, spn_BCM_FILTER_CLEAR_FE, 1)) {
                rv = soc_mem_clear(unit, FILTER_IMASKm, COPYNO_ALL, TRUE);
                if (rv < 0) {
                    _bcm_esw_filter_detach(unit);
                    return rv;
                }
                rv = soc_mem_clear(unit, FILTER_IRULEm, COPYNO_ALL, TRUE);
                if (rv < 0) {
                    _bcm_esw_filter_detach(unit);
                    return rv;
                }
            }
        }

        if (NUM_GE_PORT(unit) > 0) {
            if (soc_property_get(unit, spn_BCM_FILTER_CLEAR_GE, 1)) {
                rv = soc_mem_clear(unit, GFILTER_IMASKm, COPYNO_ALL, TRUE);
                if (rv < 0) {
                    _bcm_esw_filter_detach(unit);
                    return rv;
                }
                rv = soc_mem_clear(unit, GFILTER_IRULEm, COPYNO_ALL, TRUE);
                if (rv < 0) {
                    _bcm_esw_filter_detach(unit);
                    return rv;
                }
            }
        }

        if (NUM_XE_PORT(unit) > 0) {
            if (soc_property_get(unit, spn_BCM_FILTER_CLEAR_XE, 1)) {
                rv = soc_mem_clear(unit, GFILTER_IMASKm, COPYNO_ALL, TRUE);
                if (rv < 0) {
                    _bcm_esw_filter_detach(unit);
                    return rv;
                }
                rv = soc_mem_clear(unit, GFILTER_IRULEm, COPYNO_ALL, TRUE);
                if (rv < 0) {
                    _bcm_esw_filter_detach(unit);
                    return rv;
                }
            }
        }
    }

    /*
     * Allocate memory to keep a copy of the masks.
     */

    SOC_BLOCK_ITER(unit, blk, SOC_BLK_ETHER) {
        int             alloc_size;

        if (fc->fc_blk2rule[blk] == 0) {
            continue;
        }

        mask_count = soc_mem_index_count(unit, fc->fc_blk2mask[blk]);
        alloc_size = SIZEOF_FILTER_BLK_T(mask_count);

        fc->fc_table[blk] = sal_alloc(alloc_size, "fc_table");
        if (NULL == fc->fc_table[blk]) {
            _bcm_esw_filter_detach(unit);
            return (BCM_E_MEMORY);
        }
        sal_memset(fc->fc_table[blk], 0, alloc_size);
        fc->fc_table[blk]->fp_mask_cnt  = mask_count;
        if (soc_feature(unit, soc_feature_filter_128_rules)) {
            /* Use half of the masks with this feature */
            fc->fc_table[blk]->fp_mask_cnt /= 2;
        }
        fc->fc_table[blk]->fp_mask_used = 0;
    }

    if (soc_feature(unit, soc_feature_filter_xgs)) {
        fc->fc_window_count = 8;
        fc->fc_window_size  = 4;
        fc->fc_packet_depth = 80;
        fc->fc_dataoff_max  = 7;
        fc->fc_offset_fn    = _bcm_fc_off_xgs;
    } else if (soc_feature(unit, soc_feature_filter_extended)) {
        fc->fc_window_count = 4;
        fc->fc_window_size  = 16;
        fc->fc_packet_depth = 80;
        fc->fc_dataoff_max  = 2;
        fc->fc_offset_fn    = _bcm_fc_off_extend;
    } else {
        fc->fc_window_count = 1;
        fc->fc_window_size  = 64;
        fc->fc_packet_depth = 64;
        fc->fc_dataoff_max  = 0;
        fc->fc_offset_fn    = _bcm_fc_off_std;
    }

    if (soc_feature(unit, soc_feature_filter_metering)) {
        rv = bcm_esw_meter_init(unit);
        if (rv < 0) {
            _bcm_esw_filter_detach(unit);
            return rv;
        }
        _bcm_esw_meter_size_get(unit, &(fc->fc_meter_size));
    }

#ifdef BCM_TUCANA_SUPPORT
    if (SOC_IS_TUCANA(unit)) {
        SOC_BLOCK_ITER(unit, blk, SOC_BLK_EPIC) {
            port = SOC_BLOCK_PORT(unit, blk);
            rv = READ_EPC_FFP_CONFIGr(unit, port, &ffp_config);
            if (rv < 0) {
                _bcm_esw_filter_detach(unit);
                return rv;
            }
            soc_reg64_field32_set(unit, EPC_FFP_CONFIGr,
                                  &ffp_config, FFP_MASK_SELECTf,
                                  soc_property_get(unit,
                                                   spn_FILTER_RESIZE, 0));
            rv = WRITE_EPC_FFP_CONFIGr(unit, port, ffp_config);
            if (rv < 0) {
                _bcm_esw_filter_detach(unit);
                return rv;
            }
        }
    }
#endif /*BCM_TUCANA_SUPPORT*/

    return BCM_E_NONE;
}

#if defined(BROADCOM_DEBUG)

/*
 * Function: bcm_esw_filter_show
 *
 * Purpose:
 *      Show current S/W state if compiled in debug mode.
 *
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      pfx - character string to prefix output lines.
 *
 * Returns:
 *      BCM_E_XXX.
 */
int
bcm_esw_filter_show(int unit, const char *pfx)
{
    filter_cntl_t       *fc = _bcm_filter_control[unit];
    filter_blk_t        *fp;
    int                 blk;
    int                 mask;

    if (fc == NULL) {
        soc_cm_print("%sUnit %d: Filter API not initialized\n", pfx, unit);
        return BCM_E_INIT;
    }

    SOC_BLOCK_ITER(unit, blk, SOC_BLK_ETHER) {
        if (fc->fc_blk2rule[blk] == 0) {
            continue;
        }
        fp = fc->fc_table[blk];

        soc_cm_print("%s\tunit %d blk %d (%s%d)\t "
                     "filter masks %d used of %d total\n",
                     pfx, unit, blk,
                     soc_block_name_lookup_ext(SOC_BLOCK_INFO(unit, blk).type, unit),
                     SOC_BLOCK_INFO(unit, blk).number,
                     fp->fp_mask_cnt, fp->fp_mask_used);

        for (mask = 0; mask < fp->fp_mask_cnt; mask++) {
            if (fp->fp_mask[mask].fm_cnt) {
                soc_cm_print("%s\t\tmask %d, reference count %d\n",
                             pfx, mask, fp->fp_mask[mask].fm_cnt);
            }
        }
    }
    return BCM_E_NONE;
}

/*
 * Function: bcm_esw_filter_dump
 *
 * Purpose:
 *      Show contents of a filter template.
 *
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      bf   - Filter to dump.
 *
 * Returns:
 *      Nothing.
 */
int
bcm_esw_filter_dump(int unit, bcm_filterid_t bf)
{
    filter_cntl_t       *fc = _bcm_filter_control[unit];
    filter_t            *filter;
    int                 idx;
    char                pfmti[SOC_PBMP_FMT_LEN],
                        pfmte[SOC_PBMP_FMT_LEN];

    filter = _bcm_filter_find(unit, bf);
    if (filter == NULL) {
        return BCM_E_NOT_FOUND;
    }
    soc_cm_print("Filter %p: id=%u prio=%d, ipbm=%s, epbm=%s\n",
                 (void *)filter, filter->filterid, filter->prio,
                 SOC_PBMP_FMT(filter->ipbm, pfmti),
                 SOC_PBMP_FMT(filter->epbm, pfmte));
    if (fc == NULL) {
        soc_cm_print("unit %d filter not initialized\n", unit);
        return BCM_E_INIT;
    }
    soc_cm_print("  Data value:");
    for (idx = 0; idx < fc->fc_packet_depth; idx++) {
        if (idx % 4 == 0) {
            soc_cm_print(" ");
        }
        soc_cm_print("%02x", filter->data_val[idx]);
    }
    soc_cm_print("\n  Data mask:");
    for (idx = 0; idx < fc->fc_packet_depth; idx++) {
        if (idx % 4 == 0) {
            soc_cm_print(" ");
        }
        soc_cm_print("%02x", filter->data_mask[idx]);
    }
    soc_cm_print("\n  Rule entry: ");
    soc_mem_entry_dump(unit, GFILTER_IRULEm, &filter->rule);
    soc_cm_print("\n  Mask entry: ");
    soc_mem_entry_dump(unit, GFILTER_IMASKm, &filter->mask);
    soc_cm_print("\n");

    return BCM_E_NONE;
}

#endif /* defined(BROADCOM_DEBUG) */

#else  /* !BCM_FILTER_SUPPORT */



int
bcm_esw_filter_init(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_create(int unit, bcm_filterid_t *f_return)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_create_id(int unit, bcm_filterid_t f)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_destroy(int unit, bcm_filterid_t f)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_copy(int unit,
                bcm_filterid_t f_src, bcm_filterid_t *f_return)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_copy_id(int unit,
                   bcm_filterid_t f_src, bcm_filterid_t f_dest)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_qualify_priority(int unit, bcm_filterid_t f, int prio)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_qualify_ingress(int unit, bcm_filterid_t f,
                           bcm_pbmp_t pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_qualify_egress(int unit, bcm_filterid_t f,
                          bcm_pbmp_t pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_qualify_egress_modid(int unit, bcm_filterid_t f,
                                int module_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_qualify_unknown_ucast(int unit, bcm_filterid_t f)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_qualify_unknown_mcast(int unit, bcm_filterid_t f)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_qualify_known_ucast(int unit, bcm_filterid_t f)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_qualify_known_mcast(int unit, bcm_filterid_t f)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_qualify_broadcast(int unit, bcm_filterid_t f)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_qualify_stop(int unit, bcm_filterid_t f,
                        int partial_match)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_qualify_format(int unit, bcm_filterid_t f,
                          bcm_filter_format_t format)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_qualify_data(int unit, bcm_filterid_t f,
                        int offset, int len,
                        const uint8 *data, const uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_qualify_data8(int unit, bcm_filterid_t f,
                         int offset, uint8 val, uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_qualify_data16(int unit, bcm_filterid_t f,
                          int offset, uint16 val, uint16 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_qualify_data32(int unit, bcm_filterid_t f,
                          int offset, uint32 val, uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_action_match(int unit, bcm_filterid_t f,
                        bcm_filter_action_t action,
                        uint32 param)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_action_no_match(int unit, bcm_filterid_t f,
                           bcm_filter_action_t action,
                           uint32 param)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_action_out_profile(int unit, bcm_filterid_t f,
                              bcm_filter_action_t action,
                              uint32 param, int meter_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_install(int unit, bcm_filterid_t f)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_reinstall(int unit, bcm_filterid_t f)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_remove(int unit, bcm_filterid_t f)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_remove_all(int unit)
{
    return BCM_E_UNAVAIL;
}

#ifdef BROADCOM_DEBUG
int
bcm_esw_filter_show(int unit, const char *pfx)
{
    return BCM_E_UNAVAIL;
}

int
bcm_esw_filter_dump(int unit, bcm_filterid_t bf)
{
    return BCM_E_UNAVAIL;
}
#endif /* BROADCOM_DEBUG */

#endif  /* !BCM_FILTER_SUPPORT */
