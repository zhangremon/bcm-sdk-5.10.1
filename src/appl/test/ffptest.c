/*
 * $Id: ffptest.c 1.35.166.1 Broadcom SDK $
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
 * FFP Tests
 */

#include <sal/appl/sal.h>
#include <sal/core/boot.h>
#include <appl/diag/system.h>
#include <appl/diag/parse.h>
#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/dma.h>
#include <bcm/init.h>
#include <bcm/error.h>
#include <bcm/filter.h>
#include <bcm/link.h>
#include <bcm/stack.h>
#include <bcm/stg.h>
#include <bcm/tx.h>
#include <bcm_int/esw/mbcm.h>
#include "testlist.h"

#ifdef BCM_FILTER_SUPPORT

#define SPEED_MAX            0
#define SPEED_10FD           1
#define SPEED_100FD          2
#define SPEED_1000FD         3
#define SPEED_10GFD          4

#define MAX_WIN_COUNT        8
#define MAX_MASK_COUNT       16
#define MAX_WIN_WIDTH        64
#define MAX_FILTERS_PER_MASK 16
#define FILTER_COVERED_SIZE  80

#define TEST_PKT_SIZE        128
#define RX_MAX_PKT           TEST_PKT_SIZE

#define DESTROY_HSW_FILTERS  0
#define DESTROY_SW_FILTERS   1

#define FFP_TEST_PORT_ALL         (-1)

/* structure to hold packet data received by CPU */
typedef struct pkt_rx_s {
    dv_t *      dv;
    void *      buf;   /* buf for one packet */
    sal_sem_t   done;  /* for packet rx complete */
} pkt_rx_t;

typedef struct f_chain_s {
    int                match;  /* should we have a full match? */
    bcm_filterid_t     f;
    struct f_chain_s * next;
} f_chain_t;

typedef struct mask_filter_s {
    uint8   offset[MAX_WIN_COUNT];
    uint8   *mask[MAX_WIN_COUNT];
    uint8   bcm_mask[FILTER_COVERED_SIZE];
    int     win_width;
    int     win_count;
    int     dataoff_max;
    int     pkt_depth;
    int     state;            /* record full match state */

    f_chain_t * f_chain;      /* filters built by above mask data */
} mask_filter_t;

typedef struct ffp_stat_s {
    int scnt;                 /* repeats of pkt sent */
    int mcnt;                 /* count of all Masks used */
    int fcnt;                 /* count of all Filters created */
} ffp_stat_t;

/* hold statistics for reporting */
static ffp_stat_t stat;

/* record randomly created masks and filters */
static mask_filter_t * ffp_mf[MAX_MASK_COUNT];

/* hold packet received by CPU */
static pkt_rx_t pkt_rx;

static uint8 pkt1st_part[FILTER_COVERED_SIZE] = {
    0x0,  0x1,  0x2,  0x3,  0x4,  0x5,  0x6,  0x7,
    0x8,  0x9,  0xa,  0xb,  0x81, 0x0,  0x0,  0x1,
    0x0,  0x80, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f
};

static struct ffptest_params_s {
    int seed;
    pbmp_t pbmp;
    int gen;
    int dow;
    int speed;
    int inited;
} ffpparam;

static int dow_test(int unit, bcm_pkt_t *pkt);

/*
 * Function:
 *    setup_port
 * Purpose:
 *    setup port specific parameters
 * Parameters:
 *    unit - StrataSwitch Unit number
 *    port - port to be set
 *    speed - if SPEED_MAX, sets the maximal speed
 */
static int
setup_port(int unit, soc_port_t port, int speed)
{
    int rv;

    switch (speed) {
    case SPEED_MAX:
	rv = bcm_port_speed_max(unit, port, &speed);
	if (rv < 0) {
	    return rv;
	}
	break;
    case SPEED_10FD:
        speed = 10;
        break;
    case SPEED_100FD:
        speed = 100;
        break;
    case SPEED_1000FD:
        speed = 1000;
        break;
    case SPEED_10GFD:
        speed = 10000;
        break;
    default:
        return BCM_E_PARAM;
    }

    rv = bcm_port_speed_set(unit, port, speed);
    if (rv < 0) {
	return rv;
    }

    rv = bcm_port_duplex_set(unit, port, BCM_PORT_DUPLEX_FULL);

    return rv;
}

/*
 * Function:
 *    destroy_mask_filters
 * Purpose:
 *    destroy mask and all related filters
 * Parameters:
 *    unit - StrataSwitch Unit number
 *    mf -  mask and its related filters
 *    type - DESTROY_HSW_FILTERS, DESTROY_SW_FILTERS
 */
static void
destroy_mask_filters(int unit, mask_filter_t *mf, int type)
{
    f_chain_t * fc;
    int i;

    if (mf == NULL) {
        return;
    }

    assert(type == DESTROY_HSW_FILTERS || type == DESTROY_SW_FILTERS);

    while ((fc = mf->f_chain)!=NULL) {
        /* for quick turn test, do not remove filters from hardware */
        if (type == DESTROY_HSW_FILTERS) {
            bcm_filter_remove(unit, fc->f);
        }
        bcm_filter_destroy(unit, fc->f);

        mf->f_chain = fc->next;
        sal_free(fc);
    }

    for (i = 0; i < mf->win_count; ++i) {
        if (mf->mask[i]) {
            sal_free(mf->mask[i]);

        }
    }

    sal_free(mf);
    return;
}

/*
 * Function:
 *    destroy_filters
 * Purpose:
 *    destroy all filters
 * Parameters:
 *    unit - StrataSwitch Unit number
 */
static int
destroy_filters(int unit, int type)
{
    int i;

    for (i=0; i < MAX_MASK_COUNT; ++i) {
        destroy_mask_filters(unit, ffp_mf[i], type);
        ffp_mf[i] = NULL;
    }

    return 0;
}

/*
 * Function:
 *    create_random_mask_data
 * Purpose:
 *    create random mask data to build filters
 * Parameters:
 *    unit - StrataSwitch Unit number
 */

static mask_filter_t *
create_random_mask_data(int unit)
{
    int i;
    mask_filter_t *mf = sal_alloc(sizeof(mask_filter_t), "mf");

    if (mf == NULL) {
        test_error(unit, "Cannot allocate mask data mf\n");
        return NULL;
    }

    mf->f_chain = NULL;

    if (soc_feature(unit, soc_feature_filter_xgs)) {
        mf->win_count = 8;
        mf->win_width = 4;
        mf->pkt_depth = 80;
        mf->dataoff_max = 7;
    } else if (soc_feature(unit, soc_feature_filter_extended)) {
        mf->win_count = 4;
        mf->win_width = 16;
        mf->pkt_depth = 80;
        mf->dataoff_max = 3;
    } else {
        mf->win_count = 1;
        mf->win_width = 64;
        mf->pkt_depth = 64;
        mf->dataoff_max = 0;
    }

    for (i = 0; i <= mf->dataoff_max; ++i) {
        int lastp1 = mf->pkt_depth - mf->win_width + 1;
        int increment = mf->win_width / 2;
	mf->offset[i] = (sal_rand() % lastp1) / increment * increment;
    }

    memset(mf->bcm_mask, 0, sizeof(mf->bcm_mask));

    for (i = 0; i < mf->win_count; ++i) {
        int j, k;
        if ((mf->mask[i] = sal_alloc(mf->win_width * sizeof(uint8),
                                 "mask")) == NULL) {
            for (j=0; j<i; ++j) {
                sal_free(mf->mask[j]);
            }
            sal_free(mf);
            test_error(unit, "Cannot allocate mask data %d\n", i);
            return NULL;
        }
        k = sal_rand() % 10;
        if (k == 0 && mf->win_count != 1) {
            memset(mf->mask[i], 0, mf->win_width);
        }
        else if (k == 1 && mf->win_count != 1) {
            memset(mf->mask[i], 0xff, mf->win_width);
        }
        else {
            for (j = 0; j < mf->win_width; ++j) {
                mf->mask[i][j] = (sal_rand() % 256);
            }
        }

        for (j = 0; j < mf->win_width; ++j) {
            mf->bcm_mask[mf->offset[i] + j] |= mf->mask[i][j];
        }
    }
    return mf;
}

/*
 * Function:
 *    dump_stat
 * Purpose:
 *    report packet transceived, masks and filters tested
 * Parameters:
 *
 */
static void
dump_stat(int unit, char * test_type, soc_port_t p)
{
    printk("FFP %s: %s xmit %d pkts, use %d masks and %d filters\n",
           test_type, SOC_PORT_NAME(unit, p), stat.scnt, stat.mcnt, stat.fcnt);
}

/*
 * Function:
 *    dump_mask_filter_data
 * Purpose:
 *    dump mask and its filter associates
 * Parameters:
 *    unit - StrataSwitch Unit number
 *    mf   - mask and its related filters
 *    prx  - prefix
 */
static void
dump_mask_filter_data(int unit, mask_filter_t *mf, char *pfx)
{
    int i;
    f_chain_t *fc;

    assert(pfx);

    printk("---- Test data ----\n");
    if (mf == NULL) {
        printk("%s Mask data empty\n", pfx);
        return;
    }

    printk("%s: win_count %d\n", pfx, mf->win_count);
    for (i = 0; i < mf->win_count; ++i) {
        int j;
        printk("\twin %d: width %d, offset %d, mask ",
               i, mf->win_width, mf->offset[i]);
        for (j=0; j<mf->win_width; ++j) {
            printk("%02x", mf->mask[i][j]);
        }
        printk("\n");
    }

    printk("\tmask for filter compression ");
    for (i = 0; i < FILTER_COVERED_SIZE; ++i) {
        printk("%02x", mf->bcm_mask[i]);
    }
    printk("\n");

    fc =  mf->f_chain;
    while (fc) {
#if defined(BROADCOM_DEBUG)
        printk("---- Table data ----\n");
        bcm_filter_dump(unit, fc->f);
#endif  /* BROADCOM_DEBUG */
        fc = fc->next;
    }
    return;
}

/*
 * Function:
 *    get_first_available_mf_index
 * Purpose:
 *    get first available index in array ffp_mf to install filter
 * Parameters:
 */
static int
get_first_available_mf_index(void)
{
    int i;
    for (i=0; i<MAX_MASK_COUNT; ++i) {
        if (ffp_mf[i]==NULL)
            return i;
    }
    return -1;
}


/*
 * Function:
 *    install_one_filter
 * Purpose:
 *    install one filter using mask in mf and val
 * Parameters:
 *    unit - StrataSwitch Unit number
 *    mf    - contains valid mask data, will record filter installed
 *    index - if index < 0, find first index in ffp_mf and install
 *          mask there; otherwise install mf in ffp_mf[index];
 *          the filter installed on hardware will also be recorded
 *          in mf.
 *    val   - use the bits of val and mask in mf to creat a match filter;
 *          random data will be used to create non-match filter
 *    match - specify whether to create a match filter
 * Return:
 *    the index in ffp_mf at which the mask filter is installed
 *    < 0 if error
 */
static int
install_one_filter(int unit, mask_filter_t * mf, int index,
                   uint8 *val, soc_port_t p, int match)
{
    bcm_filterid_t f;
    bcm_pbmp_t	pbm;
    int rv, i;
    f_chain_t * fc;
    uint8 data[FILTER_COVERED_SIZE];

    assert(mf);
    assert(val);

    memset(data, 0, sizeof(data));

    if ((rv = bcm_filter_create(unit, &f)) < 0) {
        test_error(unit,
                   "Could not create filter entry: %s\n",
                   bcm_errmsg(rv));
        return -1;
    }

    BCM_PBMP_CLEAR(pbm);
    BCM_PBMP_PORT_ADD(pbm, p);
    if ((rv = bcm_filter_qualify_ingress(unit, f, pbm)) < 0) {
        test_error(unit,
                   "Could not qualify ingress: %s\n",
                   bcm_errmsg(rv));
        return -1;
    }

    if (index < 0) index = get_first_available_mf_index();

    assert(index >= 0);

    if (soc_feature(unit, soc_feature_filter_extended)) {
        if (soc_feature(unit, soc_feature_filter_pktfmtext)) {
            rv = bcm_filter_qualify_format(unit, f, 
                     SOC_IS_TUCANA(unit) ? 
                    (BCM_FILTER_PKTFMT_INNER_TAG | BCM_FILTER_PKTFMT_802_3): 
                    (BCM_FILTER_PKTFMT_OUTER_TAG | BCM_FILTER_PKTFMT_LLC));
        } else {
            rv = bcm_filter_qualify_format(unit, f, bcmFormatTagged802_3);
        }

        if (rv < 0) {
            test_error(unit,
                       "Could not qualify format: %s\n",
                       bcm_errmsg(rv));
            return -1;
        }

        if ((rv = bcm_filter_action_match(unit, f, bcmActionIncrCounter,
                                          index)) < 0) {
            test_error(unit,
                       "Could not specify action match: %s\n",
                       bcm_errmsg(rv));
            return -1;
        }

        if (SOC_IS_XGS_SWITCH(unit)) {
            int modid = 0;
            if ((rv = bcm_stk_my_modid_get(unit, &modid)) < 0) {
                test_error(unit,
                           "Could not retrieve module id: %s\n",
                           bcm_errmsg(rv));
                return -1;
            }

            if ((rv = bcm_filter_action_match(unit, f, bcmActionSetModule, 
                                              modid)) < 0) {
                test_error(unit,
                           "Could not specify match output module: %s\n",
                           bcm_errmsg(rv));
                return -1;
            }
        }
    }

    if (ffp_mf[index] && ffp_mf[index] != mf) {
        printk("Warning: override filter %d\n", index);
        destroy_mask_filters(unit, ffp_mf[index], DESTROY_HSW_FILTERS);
        ffp_mf[index] = NULL;
    }

    for (i = 0; i < mf->pkt_depth; ++i) {
        if (mf->bcm_mask[i] != 0) {
            data[i] = val[i] & mf->bcm_mask[i];
            if (!match) {
                int t = (sal_rand()%256) & mf->bcm_mask[i];
                if (t != data[i])
                    data[i] = t;
            }
        }
    }

    if ((rv = bcm_filter_qualify_data(unit, f, 0, mf->pkt_depth,
                                 data, mf->bcm_mask)) < 0) {
        test_error(unit,
                   "Could not modify data: %s\n",
                   bcm_errmsg(rv));
        return -1;
    }

    if ((rv = bcm_filter_install(unit, f)) < 0) {
        bcm_filter_destroy(unit, f);
	if (soc_cm_debug_check(DK_TESTS)) {
	    dump_stat(unit, "Gen", p);
	    dump_mask_filter_data(unit, mf, "ffp filter");
	    printk("Could not add new filter entry: %s\n",
                   bcm_errmsg(rv));
	} 
	destroy_mask_filters(unit, mf, DESTROY_HSW_FILTERS);
	ffp_mf[index] = NULL;
        return -1;
    }

    fc = mf->f_chain;
    mf->f_chain = (f_chain_t *)sal_alloc(sizeof(f_chain_t), "f_chain");
    if (mf->f_chain == NULL) {
        test_error(unit, "Could not allocate memory for mask data\n");
        return -1;
    }
    mf->f_chain->f = f;
    mf->f_chain->next = fc;
    mf->f_chain->match = match;
    mf->state = 0;

    ffp_mf[index] = mf;

    return index;
}

/*
 * Function:
 *    install_random_filters
 * Purpose:
 *    install random filters using mask count and prescribed values
 * Parameters:
 *    unit  - StrataSwitch Unit number
 *    val   - use the bits of val filters
 *    mcnt  - distinct masks to be created and used
 * Return:
 *    < 0 for error, 0 success
 */
static int
install_random_filters(int unit, uint8 *val, int mcnt, soc_port_t p)
{
    soc_mem_t	mem;
    int		blk;
    int         attempts;

    assert(mcnt > 0);
    assert(val);

    mem = 0;
    blk = SOC_PORT_BLOCK(unit, p);

    if (SOC_MEM_IS_VALID(unit, GFILTER_IRULEm) && 
        (IS_GE_PORT(unit, p) || IS_XE_PORT(unit, p))) {
        mem = GFILTER_IRULEm;
    }
    if (SOC_MEM_IS_VALID(unit, FILTER_IRULEm) && IS_FE_PORT(unit, p)) {
        mem = FILTER_IRULEm;
    }

    assert(mem != 0);

    attempts = 3 * mcnt; /* 3 attempts per mask */

    while (mcnt--) {
        mask_filter_t * mf;
        int fcnt = (sal_rand() % MAX_FILTERS_PER_MASK) + 1; 
        int random = 1;
        int index = -1;
        int done = 0;

        mf = create_random_mask_data(unit);
        while (fcnt--) {
            /* install at most only one full match filter */
	    int match = random ? (sal_rand() % 2) : 0; 

            if (soc_mem_is_cam(unit, mem)) {
                if (SAL_BOOT_QUICKTURN && (stat.fcnt >= 64)) {
                    /* Exceeding mem size */
                    done = 1;
                    break;
                }
            } else {
                if (soc_mem_index_max(unit, mem) <=
                    soc_mem_index_last(unit, mem, blk)) {
                    if (0 == stat.fcnt) {
                        destroy_mask_filters(unit, mf, DESTROY_SW_FILTERS);
                    }
                    done = 1;
                    break;
                }
            }


            index = install_one_filter(unit, mf, index, val, p, match);
            if (soc_cm_debug_check(DK_TESTS)) {
                printk("install_random_filters: index %d\n", index);
            }
	    if (index < 0) {
	        /* Retry (within reason) until mask installs */
		if ((--attempts) <= 0) {
            destroy_mask_filters(unit, mf, DESTROY_SW_FILTERS);
		    return -1;
		}
		mcnt++;
		break;
	    }
            random -= match;
            assert(random >= 0);
            stat.fcnt++;
        }
    /*    coverity[leaked_storage]    */
        if (done) break;
    }
    return 0;
}

/*
 * Function:
 *    verify_full_match
 * Purpose:
 *    verify the full/non-full matches detected in hardware are as expected;
 *    We detect full matches through ffpcounters
 * Parameters:
 *    unit  - StrataSwitch Unit number
 *    port  - the port to which a packet is send to trigger full matches
 * Return:
 *    < 0 for error, 0 success
 */
static int
verify_full_match(int unit, int port)
{
    int i;
    int vxfc, vgfc;
    int match;
    uint32 state;
    ffpcounters_entry_t entry;
    xffpcounters_entry_t xentry;
    uint32 xbuf[2];
    uint64 xstate;
    f_chain_t * fc;
    int blk;
    int rv;
    int cmp[32];
    int mismatch_cnt;
    soc_mem_t	mem = 0;

    if (soc_cm_debug_check(DK_TESTS)) {
        printk("verify_full_match: port %d\n", port);
    }
    /*    coverity[assignment : FALSE]    */
    blk = SOC_PORT_BLOCK(unit, port);
    vxfc = vgfc = match = mismatch_cnt = 0;
    if (SOC_MEM_IS_VALID(unit, XFILTER_FFPCOUNTERSm) &&
	IS_XE_PORT(unit, port)) {
        vxfc = 1;
    }
    if (SOC_MEM_IS_VALID(unit, GFILTER_FFPCOUNTERSm) &&
	(IS_GE_PORT(unit, port) || 
         (SOC_IS_TUCANA(unit) && IS_FE_PORT(unit, port)))) {
	  vgfc = 1;
	  mem = GFILTER_FFPCOUNTERSm;
    }
    if (SOC_MEM_IS_VALID(unit, GFILTER_FFP_IN_PROFILE_COUNTERSm) &&
	IS_GE_PORT(unit, port) ) {
        /* Draco 1.5 */
        vgfc = 1;
	mem = GFILTER_FFP_IN_PROFILE_COUNTERSm;
    }

    if (!vgfc && !vxfc) {
        printk("Do not verify full match for port %d\n", port);
        return 0;
    }
    /* coverity[dead_error_begin] */
    memset(cmp, 0, sizeof(cmp));
    for (i = 0; i < MAX_MASK_COUNT; ++i) {
        if (ffp_mf[i] == NULL) {
            continue;
        }
        if (vxfc) {
            rv = READ_XFILTER_FFPCOUNTERSm(unit, blk, i, &xentry);
            soc_XFILTER_FFPCOUNTERSm_field_get(unit, &xentry, COUNTf, xbuf);
            COMPILER_64_SET(xstate, xbuf[1], xbuf[0]);
            COMPILER_64_TO_32_LO(state, xstate);
        }
        else if (vgfc) {
            rv = soc_mem_read(unit, mem, blk, i, &entry);
            if (rv < 0) {
                test_error(unit, "Cannot read entry at index %d: %s\n",
                           i, bcm_errmsg(rv));
                return rv;
            }
            state = soc_mem_field32_get(unit, mem,
                                        entry.entry_data, COUNT_VALUEf);
        }
        else {
            assert(0);
            return -1;
        }

        match = (state > ffp_mf[i]->state) ? 1 : 0;
        if (soc_cm_debug_check(DK_TESTS)) {
            printk("----ffpcounters[%d] = %d, match = %d \n", i, state, match);
        }
        ffp_mf[i]->state = state;

        /* consistent with our record? */
        fc = ffp_mf[i]->f_chain;
        
        while (fc != NULL) {
            if (soc_cm_debug_check(DK_TESTS)) {
                printk("index i %d fc %p", i, (void *)fc);
                printk("ffp_mf[%d] %p", i, (void *)ffp_mf[i]);
                printk(" fc->match %d\n", fc->match);
            }
            if (match && fc->match) {
                break;
            }
            else if ((!match) && fc->match) {
                mismatch_cnt++;
                cmp[i] = 1;
            }
            fc = fc->next;
        }

        if (match && fc == NULL) {
            mismatch_cnt++;
            cmp[i] = 1;
        }
    }

    return mismatch_cnt;
}

#ifdef BCM_XGS_SWITCH_SUPPORT
static int
additional_full_match_check(int unit, soc_port_t p)
{
    dcb_t *dcb = pkt_rx.dv->dv_dcb;
    int rindex = SOC_DCB_RX_MATCHRULE_GET(unit, dcb);
    int mindex;
    int blk;
    int rv;
    irule_entry_t rule;
    mask_filter_t *mf;
    f_chain_t *f;

    if (rindex == 0) {
        return 0; /* could be no match */
    }

    blk = SOC_PORT_BLOCK(unit, p);
    assert(SOC_MEM_BLOCK_VALID(unit, GFILTER_IRULEm, blk));
    rv = READ_GFILTER_IRULEm(unit, blk, rindex, (void *)(rule.entry_data));

    mindex = soc_mem_field32_get(unit, GFILTER_IRULEm,
                         rule.entry_data, FSELf);

    assert(soc_mem_index_valid(unit, GFILTER_IMASKm, mindex));
    mf = ffp_mf[mindex];

    if (mf == NULL) {
        return -1;
    }

    for (f = mf->f_chain; f != NULL; f = f->next) {
        if (f->match) {
            return 0;
        }
    }

    return -1;
}
#endif

/*
 * Function:
 *    assign_packet
 * Purpose:
 *    copy rlen bytes from raw to pkt->buf
 * Parameters:
 *    pkt - target packet
 *    raw - the source
 *    rlen - length of the source in bytes
 */
static void
assign_packet(bcm_pkt_t *pkt, uint8 *raw, int rlen)
{
    assert(pkt);
    assert(pkt->pkt_data->data);
    assert(pkt->pkt_data->len >= rlen);
    assert(raw);
    sal_memcpy(pkt->pkt_data->data, raw, rlen);
}

/*
 * Function:
 *    l2_pkt_da_add
 * Purpose:
 *    add DA of pkt into l2 table so no flood to happen
 * Parameters:
 *    unit - StrataSwitch Unit number
 *    pkt  - packet containing the DA
 *    vid  - the vid for the DA
 *    port - the port associated with the DA
 * Return:
 *    0 success, < 0 error
 */
static int
l2_pkt_da_add(int unit, vlan_id_t vid, soc_port_t port, bcm_pkt_t *pkt)
{
    bcm_l2_addr_t l2addr;
    sal_mac_addr_t da;
    int           rv;

    assert(pkt);
    assert(pkt->pkt_data->data);

    sal_memcpy(da, pkt->pkt_data->data, 6);

    bcm_l2_addr_t_init(&l2addr, da, vid);

    l2addr.port = port;
    l2addr.flags |= BCM_L2_STATIC;

    if (SOC_IS_XGS_SWITCH(unit)) {
        if ((rv = bcm_stk_my_modid_get(unit, &l2addr.modid)) < 0) {
            test_error(unit,
                       "Could not retrieve module id: %s\n",
                       bcm_errmsg(rv));
            return -1;
        }
    }

    if ((rv = bcm_l2_addr_add(unit, &l2addr)) < 0) {
        test_error(unit,
                   "Could not add DA into L2 table: %s\n",
                   bcm_errmsg(rv));
        return -1;
    }

    return 0;
}

/*
 * Function:
 *    rx_init
 * Purpose:
 *    allocate resource for RX
 * Parameters:
 *    pkt_rx - to hold packet and dv for RX
 * Return:
 *    0 success, < 0 error
 */
static int
rx_init(int unit, pkt_rx_t * pkt_rx)
{
    assert(pkt_rx);

    if ((pkt_rx->dv = soc_dma_dv_alloc(unit, DV_RX, 1)) == NULL) {
        test_error(unit, "ffp_test: Cannot allocate dv\n");
        return -1;
    }
    if ((pkt_rx->buf = soc_cm_salloc(unit, RX_MAX_PKT, "ffp rx")) == NULL) {
        test_error(unit, "ffp_test: Cannot allocate buf\n");
        return -1;
    }
    if ((pkt_rx->done = sal_sem_create("ffp rx", sal_sem_BINARY, 0)) == NULL) {
        test_error(unit, "ffp_test: Cannot create sem\n");
        return -1;
    }

    return 0;
}

static int
fft_parse_args(int unit, args_t *a)
{
    parse_table_t    pt;

    if (ffpparam.inited == 0) {
	ffpparam.seed = 1;
        ffpparam.pbmp = PBMP_E_ALL(unit);
        ffpparam.gen  = TRUE;
        ffpparam.dow  = TRUE;
        ffpparam.speed  = SPEED_MAX;
        ffpparam.inited = 1;
    }

    parse_table_init(unit, &pt);

    parse_table_add(&pt, "SEED", PQ_DFL|PQ_INT,
            (void *) 0, &ffpparam.seed, NULL);
    parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP,
            (void *) 0, &ffpparam.pbmp, NULL);
    parse_table_add(&pt, "Gen", PQ_DFL|PQ_BOOL,
            (void *) TRUE, &ffpparam.gen, NULL);
    parse_table_add(&pt, "Dow", PQ_DFL|PQ_BOOL,
            (void *) TRUE, &ffpparam.dow, NULL);
    parse_table_add(&pt, "Speed", PQ_DFL|PQ_INT,
            (void *) 0, &ffpparam.speed, NULL);

    if (parse_arg_eq(a, &pt) < 0 || ARG_CNT(a) != 0) {
        test_error(unit,
           "%s: Invalid option: %s\n", ARG_CMD(a),
           ARG_CUR(a) ? ARG_CUR(a) : "*");
	parse_arg_eq_done(&pt);
        return -1;
    }
    parse_arg_eq_done(&pt);

    SOC_PBMP_AND(ffpparam.pbmp, PBMP_E_ALL(unit));

    if (ffpparam.speed < 0 || ffpparam.speed > 3) {
        test_error(unit, "Invalid speed %d (0 -- 3 for max/10m/100m/1g)\n",
                  ffpparam.speed);
        return -1;
    }

    return 0;
}

/*
 * Function:
 *    ffp_test_init
 * Purpose:
 *    init function for ffp test
 *    -- put all ports in loopback mode
 *    -- setup the speed for them
 *    -- prepare rx packet
 * Parameters:
 *    unit - StrataSwitch Unit number
 * Return:
 *    0 success, < 0 error
 */
int
ffp_test_init(int unit, args_t *a, void **pa)
{
    soc_port_t p;
    int        rv;
    pbmp_t     pbmp;

    if (fft_parse_args(unit, a) < 0) {
        return -1;
    }

    BCM_PBMP_ASSIGN(pbmp, ffpparam.pbmp);

    PBMP_ITER(pbmp, p) {
        if ((rv = bcm_linkscan_mode_set(unit, p,
                                        BCM_LINKSCAN_MODE_NONE)) < 0) {
            test_error(unit,
                       "Could not set port %d linkscan mode: %s\n",
                       p, bcm_errmsg(rv));
            return -1;
        }
        if ((rv = bcm_port_stp_set(unit, p, BCM_STG_STP_FORWARD)) < 0) {
            test_error(unit,
                       "Unable to set port %d in forwarding state: %s\n",
                       p, bcm_errmsg(rv));
            return -1;
        }
        if ((rv = bcm_port_loopback_set(unit, p, BCM_PORT_LOOPBACK_MAC)) < 0) {
            test_error(unit,
                       "Port %d: Failed to set MAC loopback: %s\n",
                       p, bcm_errmsg(rv));
            return(-1);
        }
	if ((rv = bcm_port_enable_set(unit, p, TRUE)) < 0) {
            test_error(unit,
                       "Port %d: Failed to enable: %s\n",
                       p, bcm_errmsg(rv));
            return(-1);
	}
        if ((rv = setup_port(unit, p, ffpparam.speed)) < 0) {
            test_error(unit,
                       "Port %d: Port setup failed: %s\n",
                       p, bcm_errmsg(rv));
            return(-1);
        }
    }

    rx_init(unit, &pkt_rx);

    memset(ffp_mf, 0, sizeof(ffp_mf));
    sal_srand(ffpparam.seed);
    stat.scnt = 0;
    stat.mcnt = 0;
    stat.fcnt = 0;

    return 0;
}

/*
 * Function:
 *    get_ffpcounter
 * Purpose:
 *    get counter to store in value
 * Parameters:
 *    unit - StrataSwitch Unit number
 *    p    - the port to which ffpcounters related
 *    index - the counter to get
 *    value - to store the counter value
 * Return:
 *    0 success, < 0 error
 */
#define NO_FFPCOUNTERS -2

static int
get_ffpcounter(int unit, soc_port_t p, int index, uint32 *value)
{
    int                 blk;
    ffpcounters_entry_t entry;
    xffpcounters_entry_t xentry;
    int                 rv;
    soc_mem_t           mem;
    int                 has_counter;
    uint32              loc_value;
    uint64              xvalue;
    uint32              xbuf[2];

    assert(value);

    rv = 0;
    mem = -1;
    has_counter = 0;
    if (SOC_MEM_IS_VALID(unit, XFILTER_FFPCOUNTERSm) && 
        IS_XE_PORT(unit, p)) {
 	blk = SOC_PORT_BLOCK(unit, p);
        mem = XFILTER_FFPCOUNTERSm;
        rv = READ_XFILTER_FFPCOUNTERSm(unit, blk, index, &xentry);
        has_counter = 1;
    }
    if (SOC_MEM_IS_VALID(unit, GFILTER_FFPCOUNTERSm) && 
        IS_GE_PORT(unit, p)) {
 	blk = SOC_PORT_BLOCK(unit, p);
        mem = GFILTER_FFPCOUNTERSm;
        rv = READ_GFILTER_FFPCOUNTERSm(unit, blk, index, &entry);
        has_counter = 1;
    }
    if (SOC_MEM_IS_VALID(unit, GFILTER_FFP_IN_PROFILE_COUNTERSm) && 
	IS_GE_PORT(unit, p)) {
        /* Draco 1.5 */
        blk = SOC_PORT_BLOCK(unit, p);
        mem = GFILTER_FFP_IN_PROFILE_COUNTERSm;
	rv = soc_mem_read(unit, mem, blk, index, &entry);
        has_counter = 1;
    }
    if (SOC_IS_TUCANA(unit) && 
	SOC_MEM_IS_VALID(unit, GFILTER_FFPCOUNTERSm) && 
        IS_FE_PORT(unit, p)) {
 	blk = SOC_PORT_BLOCK(unit, p);
        mem = GFILTER_FFPCOUNTERSm;
        rv = READ_GFILTER_FFPCOUNTERSm(unit, blk, index, &entry);
        has_counter = 1;
    }

    if (!has_counter) {
        return NO_FFPCOUNTERS;
    }
    assert(mem != -1);

    if ( index < soc_mem_index_min(unit, mem) ||
         index > soc_mem_index_max(unit, mem)) {
        test_error(unit,
                   "FFP counters index %d out of range\n",
                   index);
        return -1;
    }

    if (rv < 0) {
        test_error(unit,
                   "Unable to read ffpcounters %d (port %d): %s",
                   index, p, bcm_errmsg(rv));
        return rv;
    }

    if (IS_XE_PORT(unit, p)) {
        soc_XFILTER_FFPCOUNTERSm_field_get(unit, &xentry, COUNTf, xbuf);
        COMPILER_64_SET(xvalue, xbuf[1], xbuf[0]);
        COMPILER_64_TO_32_LO(loc_value, xvalue);
    } else {
        loc_value = soc_mem_field32_get(unit, mem, entry.entry_data,
                                     COUNT_VALUEf);
    }

    *value = loc_value;

    return 0;
}

/*
 * Function:
 *    set_ffpcounter
 * Purpose:
 *    set counter at index to value
 * Parameters:
 *    unit - StrataSwitch Unit number
 *    p    - the port to which ffpcounters related
 *    index - the counter to set
 *    value - to set in the counter
 * Return:
 *    0 success, < 0 error
 */
static int
set_ffpcounter(int unit, soc_port_t p, int index, uint32 value)
{
    int		blk;
    ffpcounters_entry_t entry;
    xffpcounters_entry_t xentry;
    uint32 buf[2];
    int		rv;

    assert(index < 32 && index >= 0);

    rv = 0;
    entry.entry_data[0] = value;
    if (SOC_MEM_IS_VALID(unit, XFILTER_FFPCOUNTERSm) && 
        IS_XE_PORT(unit, p)) {

        buf[0] = value;
        buf[1] = 0;
	blk = SOC_PORT_BLOCK(unit, p);
        memset(&xentry, 0, sizeof(xentry));
        soc_XFILTER_FFPCOUNTERSm_field_set(unit, &xentry, COUNTf, buf);
        rv = WRITE_XFILTER_FFPCOUNTERSm(unit, blk, index, &xentry);
    }
    if (SOC_MEM_IS_VALID(unit, GFILTER_FFPCOUNTERSm) && 
        IS_GE_PORT(unit, p)) {
            blk = SOC_PORT_BLOCK(unit, p);
            soc_mem_field32_set(unit, GFILTER_FFPCOUNTERSm,
                entry.entry_data, COUNT_VALUEf, value);
            rv = WRITE_GFILTER_FFPCOUNTERSm(unit, blk, index, &entry);
    }
    if (SOC_MEM_IS_VALID(unit, GFILTER_FFP_IN_PROFILE_COUNTERSm) && 
        IS_GE_PORT(unit, p)) {
        /* Draco 1.5 */
	blk = SOC_PORT_BLOCK(unit, p);
        soc_mem_field32_set(unit, GFILTER_FFP_IN_PROFILE_COUNTERSm,
                            entry.entry_data, COUNT_VALUEf, value);
        rv = soc_mem_write(unit, GFILTER_FFP_IN_PROFILE_COUNTERSm, 
			   blk, index, &entry);
    }
    if (SOC_IS_TUCANA(unit) && 
	SOC_MEM_IS_VALID(unit, GFILTER_FFPCOUNTERSm) && 
        IS_FE_PORT(unit, p)) {
	blk = SOC_PORT_BLOCK(unit, p);
        soc_mem_field32_set(unit, GFILTER_FFPCOUNTERSm,
                            entry.entry_data, COUNT_VALUEf, value);
        rv = WRITE_GFILTER_FFPCOUNTERSm(unit, blk, index, &entry);
    }

    if (rv < 0) {
        test_error(unit,
                   "Unable to set ffpcounter (port %d): %s",
                   p, bcm_errmsg(rv));
        return rv;
    }
    return 0;
}

/*
 * Function:
 *    clear_ffpcounters
 * Purpose:
 *    reset counters to 0 to observe full matches
 * Parameters:
 *    unit - StrataSwitch Unit number
 *    p    - the port to which ffpcounters related
 * Return:
 *    0 success, < 0 error
 */
static int
clear_ffpcounters(int unit, soc_port_t p)
{
    int rv, i;

    for (i = 0; i < 32; ++i) {
        rv = set_ffpcounter(unit, p, i, 0);
        if (rv < 0) {
            test_error(unit,
                       "Unable to clear ffpcounter (port %d): %s",
                       p, bcm_errmsg(rv));
            return -1;
        }
    }
    return 0;
}

/*
 * Function:
 *    done_chain
 * Purpose:
 *    Release semaphore after packet RX complete
 * Parameters:
 *    unit - StrataSwitch Unit number
 *    dv - dv for DMA operation
 */
static void
done_chain(int unit, dv_t *dv)
{
    sal_sem_give(pkt_rx.done);
}

/*
 * Function:
 *    rx_start
 * Purpose:
 *    Start a DMA request
 * Parameters:
 *    unit - StrataSwitch Unit number
 *    pkt_rx - to hold incoming pkt and dv
 * Return:
 *    0 success, < 0 error
 */
static int
rx_start(int unit, pkt_rx_t * pkt_rx)
{
    dv_t	*dv;
    int		rv;
    pbmp_t	empty;

    assert(pkt_rx);
    dv = pkt_rx->dv;

    SOC_PBMP_CLEAR(empty);
    soc_dma_dv_reset(DV_RX, dv);

    rv = soc_dma_desc_add(dv, (sal_vaddr_t)pkt_rx->buf,
			  TEST_PKT_SIZE, empty, empty, empty, 0, NULL);
    if (rv < 0) {
        test_error(unit, "Descriptor add failed: %s", bcm_errmsg(rv));
        return -1;
    }
    soc_dma_desc_end_packet(dv);

    dv->dv_flags |= DV_F_NOTIFY_CHN;
    dv->dv_done_chain = done_chain;

    if ((rv = soc_dma_start(unit, -1, dv)) < 0) {
        test_error(unit,
                   "DMA request failed: %s",
                   bcm_errmsg(rv));
        return -1;
    }
    return 0;
}

/*
 * Function:
 *    ffp_do_txrx
 * Purpose:
 *    send and receive packet
 * Parameters:
 *    unit - StrataSwitch Unit number
 *    pkt  - hold pkt to be transmitted
 *    p    - port to which the packet is sent
 *    is_first - indicating if the receiving dv has not received pkt yet
 * Return:
 *    0 success, < 0 error
 */
static int
ffp_do_txrx(int unit, soc_port_t p, bcm_pkt_t *pkt, int is_first)
{
    int rv;
    bcm_pbmp_t	pbm, empty_pbm;
    uint32	wait_time;

    assert(pkt);

    if (rx_start(unit, &pkt_rx) < 0) {
        test_error(unit, "ffp_test: Cannot start reveive DMA\n");
        return -1;
    }
    BCM_PBMP_CLEAR(pbm);
    BCM_PBMP_PORT_ADD(pbm, p);
    BCM_PBMP_CLEAR(empty_pbm);

    pkt->unit = unit;
    pkt->tx_pbmp = pbm;
    pkt->tx_upbmp = empty_pbm;
    pkt->tx_upbmp = empty_pbm;

    rv = bcm_tx(unit, pkt, NULL);
    if (rv < 0) {
        test_error(unit, "packet_send failed: %s\n",
                   bcm_errmsg(rv));
    }
    stat.scnt++;

    wait_time = 4000000;
    if (SAL_BOOT_QUICKTURN) {
	wait_time *= 10;
    }
    if (is_first) {
        sal_usleep(wait_time);
    }

    if (sal_sem_take(pkt_rx.done, wait_time) < 0) {
        test_error(unit, "ffp_test: No pkt received in %d usecs\n",
                   wait_time);
        return -1;
    }
    else if (memcmp(pkt->pkt_data->data, pkt_rx.buf, pkt->pkt_data->len - 4) != 0) {
        test_error(unit, "ffp_test: Pkt received in error\n");
        return -1;
    }
    return 0;
}

/*
 * Function:
 *    gen_test
 * Purpose:
 *    using random filters to test rules search algorithm
 * Parameters:
 *    unit - StrataSwitch Unit number
 *    pkt  - hold pkt to be transmitted
 * Return:
 *    0 success, < 0 error
 */
static int
gen_test(int unit, bcm_pkt_t *pkt)
{
    int        rv;
    int        sv = 1;
    int        is_first = 1;
    soc_port_t p;
    pbmp_t     pbmp;

    assert(pkt);

    if ((rv = bcm_filter_init(unit)) < 0) {
        test_error(unit, "Filter initialization failed: %s\n",
                   bcm_errmsg(rv));
        return -1;
    }
    if ((!SOC_MEM_IS_VALID(unit, XFILTER_FFPCOUNTERSm)) &&
        (!SOC_MEM_IS_VALID(unit, GFILTER_FFPCOUNTERSm)) &&
        (!SOC_MEM_IS_VALID(unit, GFILTER_FFP_IN_PROFILE_COUNTERSm))) {
        sv = 0;
    }

    BCM_PBMP_ASSIGN(pbmp, ffpparam.pbmp);

    PBMP_ITER(pbmp, p) {
        int max_mcnt = 0;
        int mcnt;

        stat.scnt = 0;
        stat.mcnt = 0;
        stat.fcnt = 0;

        if (SOC_MEM_IS_VALID(unit, GFILTER_IMASKm) && 
            (IS_GE_PORT(unit, p) || IS_XE_PORT(unit, p))) {
            max_mcnt = soc_mem_index_max(unit, GFILTER_IMASKm) -
                soc_mem_index_min(unit, GFILTER_IMASKm) + 1;
        }
        if (SOC_MEM_IS_VALID(unit, FILTER_IMASKm) && IS_FE_PORT(unit, p)) {
            max_mcnt = soc_mem_index_max(unit, FILTER_IMASKm) -
                soc_mem_index_min(unit, FILTER_IMASKm) + 1;
        }
        assert(max_mcnt > 0);
        mcnt = (sal_rand() % max_mcnt) + 1;
        if (install_random_filters(unit, pkt->pkt_data->data, mcnt, p) < 0) {
	    dump_stat(unit, "Gen", p);
            test_error(unit, "ffp_test: random filter installation failed\n");
	    return -1;
	}
        stat.mcnt += mcnt;

        if ((rv = clear_ffpcounters(unit, p)) < 0) {
            test_error(unit, "Filter initialization failed: %s\n",
                       bcm_errmsg(rv));
            return rv;
        }

        if (ffp_do_txrx(unit, p, pkt, is_first) < 0) {

            test_error(unit, "ffp_test: tx/rx failed\n");
            return -1;
        }
        /* coverity[dead_error_line : FALSE] */
        /* coverity[dead_error_condition : FALSE] */
        else if (sv && verify_full_match(unit, p) != 0) {
            test_error(unit, "ffp_test: Full match missed\n");
            return -1;
        }

#ifdef BCM_XGS_SWITCH_SUPPORT
	/*
         * for 5690, another piece of info on matched rule is in
         * descriptor DMA status offset 7, bits 22:16
	 */
        if (SOC_CHIP_GROUP(unit) == SOC_CHIP_BCM5690) {
            if (additional_full_match_check(unit, p) < 0) {
	        test_error(unit, "ffp_test: full match check failed\n");
		return -1;
	    }
        }
#endif
        is_first = 0;

        if (SAL_BOOT_QUICKTURN) {
            destroy_filters(unit, DESTROY_SW_FILTERS);
        }
        else {
            destroy_filters(unit, DESTROY_HSW_FILTERS);
        }

        dump_stat(unit, "Gen", p);
    }

    return 0;
}


/*
 * Function:
 *    ffp_test
 * Purpose:
 *    ffp test work horse
 * Parameters:
 *    unit - StrataSwitch Unit number
 *    a, pa - not used
 */
int
ffp_test(int unit, args_t *a, void *pa)
{
    bcm_pkt_t     *pkt = NULL;

    bcm_pkt_alloc(unit, TEST_PKT_SIZE, BCM_TX_CRC_REGEN, &pkt);
    if (pkt == NULL) {
        test_error(unit, "Packet create failed\n");
        return -1;
    }
    assign_packet(pkt, pkt1st_part, FILTER_COVERED_SIZE);
    if (l2_pkt_da_add(unit, 1, CMIC_PORT(unit), pkt) < 0) {
        test_error(unit, "L2 addr add failed\n");
        return -1;
    }
    if (ffpparam.dow &&
        dow_test(unit, pkt) < 0) {
        test_error(unit, "FFP dow test (dow for %s) failed\n",
                   SOC_UNIT_GROUP(unit));
        return -1;
    }
    if (ffpparam.gen &&
        gen_test(unit, pkt) < 0) {
        test_error(unit, "FFP gen test failed\n");
        return -1;
    }
    bcm_pkt_free(unit,pkt);

    return 0;
}

/*
 * Function:
 *    ffp_test_done
 * Purpose:
 *    test clean up effort
 *    -- put ports in regular mode
 *    -- release RX resource
 * Parameters:
 *    unit - StrataSwitch Unit number
 *    pa - not used
 */
int
ffp_test_done(int unit, void *pa)
{
    int           rv;

    if ((rv = soc_dma_abort(unit)) < 0) {
        test_error(unit,
                   "Soc DMA abort failed: %s\n",
                   bcm_errmsg(rv));
        return -1;
    }
    soc_dma_dv_free(unit, pkt_rx.dv);
    soc_cm_sfree(unit, pkt_rx.buf);
    sal_sem_destroy(pkt_rx.done);

    return 0;
}


/*************************************************************************
 *
 * 5690-specific features, some detailed hardware knowledge
 *     is used
 *
 ************************************************************************/

/*
 * Functions:
 *    get_pkt_off
 * Purpose:
 *    Map window number and DATAOFFSET value to
 *    offset within packet (according to tables in reference manual).
 * Parameters:
 *    window - for DATAOFFSETx, window is x-1
 *    offnum - value of DATAOFFSETx field
 * Return Values:
 *    Offset in packet data
 */
static int
get_pkt_off(int unit, int window, int offnum)
{
    if (soc_feature(unit, soc_feature_filter_xgs)) {
        return (window * 4 + offnum * (offnum + 1)) % 80;
    }
    if (soc_feature(unit, soc_feature_filter_extended)) {
        return (window * 16 + offnum * 8);
    }
    return 0;
}

/*
 * Functions:
 *    get_window_count
 * Purpose:
 *    Get count of offset fields
 * Parameters:
 * Return Values:
 *    number of DATAOFFSETx fields
 */
static int
get_window_count(int unit)
{
    if (soc_feature(unit, soc_feature_filter_xgs)) {
        return 8;
    }
    if (soc_feature(unit, soc_feature_filter_extended)) {
        return 4;
    }
    return 1;
}

/*
 * Functions:
 *    get_dataoff_max
 * Purpose:
 *    Get max of offset fields
 * Parameters:
 * Return Values:
 *    max of DATAOFFSETx fields
 */
static int
get_dataoff_max(int unit)
{
    if (soc_feature(unit, soc_feature_filter_xgs)) {
        return 7;
    }
    if (soc_feature(unit, soc_feature_filter_extended)) {
        return 2;
    }
    return 0;
}

/*
 * Functions:
 *    get_window_size
 * Purpose:
 *    Get size of each window in bytes
 * Parameters:
 * Return Values:
 *    return size of each window in bytes
 */
static int
get_window_size(int unit)
{
    if (soc_feature(unit, soc_feature_filter_xgs)) {
        return 4;
    }
    if (soc_feature(unit, soc_feature_filter_extended)) {
        return 16;
    }
    return 64;
}

/*
 * Functions:
 *    get_pkt_covered_size
 * Purpose:
 *    Get size of part of pkt covered by window in bytes
 * Parameters:
 * Return Values:
 *    return the size in bytes
 */
static int
get_pkt_covered_size(int unit)
{
    if (soc_feature(unit, soc_feature_filter_xgs)) {
        return 80;
    }
    if (soc_feature(unit, soc_feature_filter_extended)) {
        return 80;
    }
    return 64;
}

static int
get_submask_index(int unit, int window)
{
    if (soc_feature(unit, soc_feature_filter_xgs)) {
        return (7 - window) * 4;
    }
    if (soc_feature(unit, soc_feature_filter_extended)) {
        return (3 - window) * 16;
    }
    return 0;
}

static soc_field_t do_field[] = {
    DATAOFFSET1f, DATAOFFSET2f, DATAOFFSET3f, DATAOFFSET4f,
    DATAOFFSET5f, DATAOFFSET6f, DATAOFFSET7f, DATAOFFSET8f
};

static mask_filter_t *
create_mask_data(int unit, int window, int offnum, uint8 *submask)
{
    mask_filter_t * mf;

    if ((mf = sal_alloc(sizeof(mask_filter_t), "mf")) == NULL) {
        test_error(unit, "Cannot allocate mask data mf\n");
        return NULL;
    }

    mf->f_chain = NULL;
    mf->win_count = get_window_count(unit);
    mf->win_width = get_window_size(unit);
    mf->pkt_depth = get_pkt_covered_size(unit);
    mf->dataoff_max = get_window_count(unit) - 1;

    memset(mf->bcm_mask, 0, sizeof(mf->bcm_mask));
    memset(mf->mask, 0, sizeof(mf->mask));

    if ((mf->mask[window] = sal_alloc(mf->win_width * sizeof(uint8),
                                 "mask")) == NULL) {
        test_error(unit, "Cannot allocate mask data %d\n", window);
        return NULL;
    }

    sal_memcpy(mf->mask[window], submask, mf->win_width * sizeof(uint8));
    sal_memset(mf->offset, 0, sizeof(mf->offset));

    mf->offset[window] = get_pkt_off(unit, window, offnum);
    sal_memcpy(mf->bcm_mask + mf->offset[window], mf->mask[window],
           mf->win_width * sizeof(uint8));

    return mf;
}

static int
modify_mf_helper(int unit, soc_mem_t rmem, soc_mem_t mmem, int copyno,
                 int fsel, uint32 window, uint32 offnum, uint8 *submask,
                 uint8 *subfilter, imask_entry_t * old_mentry)
{
    imask_entry_t mentry;
    irule_entry_t rentry;
    int           rv;
    int           ridx, count;
    uint8         maskbuf[MAX_WIN_WIDTH * MAX_WIN_COUNT],
                  filterbuf[MAX_WIN_WIDTH * MAX_WIN_COUNT];
    int           subidx;
    int           submsize;
    int           rules_128 = soc_feature(unit, soc_feature_filter_128_rules);

    if (!SOC_IS_LYNX(unit)) {
        switch (rmem) {
        case FILTER_IRULEm:
        case GFILTER_IRULEm:
            ridx = SOC_PERSIST(unit)->filter_masks[copyno][fsel].start;
            count = SOC_PERSIST(unit)->filter_masks[copyno][fsel].count;
            break;
        default:
            ridx = count = 0;
            assert(0);
        }
        
        assert(count == 1);
    } else {
        
        if (soc_feature(unit, soc_feature_filter_krules) || rules_128) {
            ridx = 0;
        } else {
            ridx = 1;
        }
    }

    sal_memcpy(mentry.entry_data, old_mentry->entry_data,
               sizeof(mentry.entry_data));

    submsize = get_window_size(unit);

    subidx = get_submask_index(unit, window);

    /* Modify imask entry */
    sal_memset(maskbuf, 0, sizeof(maskbuf));
    sal_memcpy(maskbuf + subidx, submask, submsize);

    soc_mem_field_set(unit, mmem, mentry.entry_data,
                      do_field[window], &offnum);

    soc_mem_field_set(unit, mmem, mentry.entry_data,
                      FMASKf, (uint32*) maskbuf);

    if ((rv = soc_mem_write(unit, mmem, copyno, fsel,
                            mentry.entry_data)) < 0) {
        test_error(unit,
                   "Cannot read imask entry at index %d: %s\n",
                   fsel, bcm_errmsg(rv));
        return rv;
    }

    if ((rv = soc_mem_read(unit, rmem, copyno, ridx, &rentry)) < 0) {
        test_error(unit, "Cannot read irule entry at index %d: %s\n",
                   ridx, bcm_errmsg(rv));
        return rv;
    }

    /* replace filter subfield */
    sal_memset(filterbuf, 0, sizeof(filterbuf));
    sal_memcpy(filterbuf + subidx, subfilter, submsize);
    soc_mem_field_set(unit, rmem, rentry.entry_data,
                      FILTERf, (uint32 *) filterbuf);
    if ((rv = soc_mem_write(unit, rmem, copyno, ridx,
                            rentry.entry_data)) < 0) {
        test_error(unit,
                   "Cannot write imask entry at index %d: %s\n",
                   ridx, bcm_errmsg(rv));
        return rv;
    }

    if (rules_128) {
        while ((++ridx) % 8) {
            if ((rv = soc_mem_write(unit, rmem, copyno, ridx,
                                    rentry.entry_data)) < 0) {
                test_error(unit,
                           "Cannot write imask entry at index %d: %s\n",
                           ridx, bcm_errmsg(rv));
                return rv;
            }
        }
    }

    return 0;
}

/*
 * Function:
 *    modify_imask
 * Purpose:
 *    The function modifies an imask as well as its associated filters
 *    (Now we assume only one filter is associated for simplicity)
 * Parameters:
 *    unit - StrataSwitch Unit number
 *    p    - port with which imask table is associated
 *    fsel - the imask index
 *    window - one of the dataoffset fields in imask table
 *    offnum - content of the field
 *    submask, subfilter - the mask and filter data used to modify
 *    oldentry - data in hardware before modification
 * Return:
 *    0 success, < 0 error
 *
 */
static int
modify_imask(int unit, soc_port_t p, int fsel, uint32 window, uint32 offnum,
             uint8 *submask, uint8 *subfilter, imask_entry_t * oldentry)
{
    int       blk;
    soc_mem_t mmem;
    soc_mem_t rmem;
    int       rv;
    uint8     maskbuf[MAX_WIN_WIDTH];
    int       docnt;

    assert(oldentry);

    mmem = rmem = -1;
    blk = SOC_PORT_BLOCK(unit, p);
    if (SOC_MEM_IS_VALID(unit, GFILTER_IMASKm) && 
        (IS_GE_PORT(unit, p) || IS_XE_PORT(unit, p))) {
        mmem = GFILTER_IMASKm;
        rmem = GFILTER_IRULEm;

    }

    if (SOC_MEM_IS_VALID(unit, FILTER_IMASKm) && IS_FE_PORT(unit, p)) {
        mmem = FILTER_IMASKm;
        rmem = FILTER_IRULEm;
    }

    assert(mmem != -1 && rmem != -1);
    assert(SOC_MEM_BLOCK_VALID(unit, mmem, blk));
    assert(SOC_MEM_BLOCK_VALID(unit, rmem, blk));
    assert(fsel >= soc_mem_index_min(unit, mmem));
    assert(fsel <= soc_mem_index_max(unit, mmem));

    if ((rv = soc_mem_read(unit, mmem, blk, fsel, oldentry)) < 0) {
        test_error(unit, "Cannot read imask entry at fsel %d: %s\n",
                   fsel, bcm_errmsg(rv));
        return rv;
    }

    soc_mem_field_get(unit, mmem, oldentry->entry_data, FMASKf,
                      (uint32 *) maskbuf);

    docnt = get_window_count(unit);
    assert(window < docnt);

    return  modify_mf_helper(unit, rmem, mmem, blk, fsel, window, offnum,
                             submask, subfilter, oldentry);
}

#define REVERSE_WORD_ORDER(w, len) {  \
    uint32 temp, i;                    \
    for ( i = 0; i < (len)/2; ++i) {   \
        temp = (w)[i];                 \
        (w)[i] = (w)[(len) - i - 1];   \
        (w)[(len) - i - 1] = temp;     \
    }                                  \
}

static int
dow_test_one_port(int unit, bcm_pkt_t *pkt, soc_port_t p, int is_first)
{
    uint32 count = 0;
    int    window, offnum;
    int    win_count = get_window_count(unit);
    int    dataoff_max = get_dataoff_max(unit);
    int    rv;

    assert(pkt);

    clear_ffpcounters(unit, p);

    for (window = 0; window < win_count; ++window) {
        for (offnum = 0; offnum <= dataoff_max; offnum++) {
            mask_filter_t *mf;
            uint8          submask[MAX_WIN_WIDTH],
                           subfilter[MAX_WIN_WIDTH];
	    uint32	   newcount;
            int            index,
                           pkt_off,
                           win_size;
            imask_entry_t oldentry;

            sal_memset(submask, 0xff, sizeof(submask));

            /* To do: reduce two memory accesses to one */

            mf = create_mask_data(unit, window, offnum, submask);
            if (mf == NULL) {
                test_error(unit, "Cannot allocate mask data mf\n");
                return 0;
            }

            if ((index = install_one_filter(unit, mf, -1, 
                                            pkt->pkt_data->data, p, 1)) < 0) {
                test_error(unit, "ffp_test: filter install failed\n");
                return -1;
            }
            stat.mcnt++, stat.fcnt++;
            pkt_off = get_pkt_off(unit, window, offnum);
            win_size = get_window_size(unit);
            sal_memcpy(subfilter, pkt->pkt_data->data + pkt_off, win_size);
            REVERSE_WORD_ORDER((uint32 *)subfilter, win_size/4);

            modify_imask(unit, p, index, window, offnum,
                         submask, subfilter, &oldentry);

            if (ffp_do_txrx(unit, p, pkt, is_first) < 0) {
                test_error(unit, "ffp_test: tx/rx failed\n");
                return -1;
            }
            else if ((rv = get_ffpcounter(unit, p, index, &newcount))<0) {
                if (rv == NO_FFPCOUNTERS) {
                    printk("Do not verify full match for port %d\n", p);
                    stat.scnt++;
                    destroy_filters(unit, DESTROY_HSW_FILTERS);
                    return 0;
                }
                test_error(unit, "ffp_test: Cannot read ffpcounter\n");
                return -1;
            }
            else if (newcount != count + 1) {
                test_error(unit, "ffp_test: Full match missed\n");
                return -1;
            }
            is_first = 0;
            stat.scnt++;

            count = newcount;
            destroy_filters(unit, DESTROY_HSW_FILTERS);
        }
    }
    return 0;
}

/*
 * Function:
 *    dow_test
 * Purpose:
 *    test chip specific features
 *    5690: Set dataoffset and filters directly and verify full match.
 *       The simplest case with only one pair of non-trivial dataoffset and
 *       mask is tested.
 * Parameters:
 *    unit - StrataSwitch unit number
 *    pkt  - hold pkt to be transmitted
 * Return:
 *    0 success, < 0 error
 * Note:
 *    First the function installs a (mask0, filter) pair.
 *    Then modify mask0 in hardware so that the resulting
 *       (mask1, filter) pair is functional equivalent.
 */
static int
dow_test(int unit, bcm_pkt_t *pkt)
{
    int        rv;
    soc_port_t p;
    int        is_first = 1;
    pbmp_t     pbmp;

    BCM_PBMP_ASSIGN(pbmp, ffpparam.pbmp);

    if (!soc_feature(unit, soc_feature_filter_extended)) {
        printk("DOW test not available for this unit\n");
        return -1;
    }

    assert(pkt);

    PBMP_ITER(pbmp, p) {

        stat.scnt = 0;
        stat.mcnt = 0;
        stat.fcnt = 0;
        /*
         * Dow test bypasses softcopy of imask/irule tables
         * it's safe to re-init
         */
        if ((rv = bcm_filter_init(unit)) < 0) {
            test_error(unit, "Filter initialization failed: %s\n",
                       bcm_errmsg(rv));
            return -1;
        }

        if (dow_test_one_port(unit, pkt, p, is_first) < 0) {
            test_error(unit, "ffp test: failed dow test\n");
            return -1;
        }
        is_first = 0;
        dump_stat(unit, "Dow", p);
    }

    return 0;
}

#endif /* BCM_FILTER_SUPPORT */
