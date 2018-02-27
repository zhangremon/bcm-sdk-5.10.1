/*
 * $Id: memory.c 1.39 Broadcom SDK $
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
 * Memory Tests implemented via S-Channel table read/write.
 * These are especially useful for memories without BIST (e.g. QVLAN).
 */

#include <sal/types.h>

#include <sal/appl/pci.h>

#include <appl/diag/system.h>
#include <appl/diag/parse.h>
#include <appl/diag/progress.h>

#include <soc/mem.h>
#include <soc/filter.h>
#include <soc/l2x.h>

#include <bcm/field.h>

#ifdef BCM_SBX_SUPPORT
#include <soc/sbx/sbx_drv.h>
#endif

#include "testlist.h"

#if defined (BCM_ESW_SUPPORT) || defined (BCM_SIRIUS_SUPPORT)

typedef struct {
    int	                inited;
    int	                was_debug_mode;
    int	                read_count;
    int	                pat_zero, pat_one, pat_five, pat_a;
    int	                pat_checker, pat_invckr, pat_linear, pat_random;
    int	                progress_total;
    int                 pat_hex;
    int                 hex_byte;
    int                 test_by_entry;
    int                 reverify_count;
    int                 reverify_delay;
    int                 continue_on_error;
    int                 error_count;
    int                 error_max;
    soc_mem_test_t      parm;
} mem_testdata_t;

static mem_testdata_t	*mem_work[SOC_MAX_NUM_DEVICES];

/*
 * Memory Test Support Routines
 */

void
mt_status_fn(soc_mem_test_t *parm, char *status_str)
{
    static char buf[120];

    sal_sprintf(buf,
	    "%s on %s.%s",
	    status_str,
	    SOC_MEM_UFNAME(parm->unit, parm->mem),
	    SOC_BLOCK_NAME(parm->unit, parm->copyno));

    progress_status(buf);
}

int
mt_write_fn(soc_mem_test_t *parm,
	    int copyno, int index, uint32 *entry_data)
{
    progress_report(1);

    if (soc_mem_write(parm->unit, parm->mem,
		      copyno, index, entry_data) < 0) {
	printk("Write ERROR: table %s.%s[%d]\n",
	       SOC_MEM_UFNAME(parm->unit, parm->mem),
	       SOC_BLOCK_NAME(parm->unit, copyno), index);
	return -1;
    }

    return 0;
}

#undef TEST_INJECT_READ_ERROR

int
mt_read_fn(soc_mem_test_t *parm,
	   int copyno, int index, uint32 *entry_data)
{
    progress_report(1);

    if (soc_mem_read(parm->unit, parm->mem,
		     copyno, index, entry_data) < 0) {
	printk("Read ERROR: table %s.%s[%d]\n",
	       SOC_MEM_UFNAME(parm->unit, parm->mem),
	       SOC_BLOCK_NAME(parm->unit, copyno), index);
	return -1;
    }

#ifdef TEST_INJECT_READ_ERROR
    if (index == 7)
	entry_data[0] ^= 0x00010000;
#endif

    return 0;
}

int
mt_miscompare_fn(soc_mem_test_t *parm,
		 int copyno, int index,
		 uint32 *read_data, uint32 *wrote_data, uint32 *mask_data)
{
    int			unit = parm->unit;
    int 		i, dw;
    uint32		reread_data[SOC_MAX_MEM_WORDS];

    soc_pci_analyzer_trigger(unit);

    dw = soc_mem_entry_words(unit, parm->mem);

    printk("\n\nCompare ERROR: table %s.%s[%d]\n",
	   SOC_MEM_UFNAME(unit, parm->mem),
	   SOC_BLOCK_NAME(unit, copyno), index);
    printk("  Read  :");
    for (i = 0; i < dw; i++) {
	printk(" 0x%08x", read_data[i] & mask_data[i]);
    }
    printk("\n  Wrote :");
    for (i = 0; i < dw; i++) {
	printk(" 0x%08x", wrote_data[i] & mask_data[i]);
    }
    printk("\n  Diff  :");
    for (i = 0; i < dw; i++) {
	printk(" 0x%08x", (read_data[i] ^ wrote_data[i]) & mask_data[i]);
    }
    if (soc_mem_read(unit, parm->mem,
		     copyno, index, reread_data) >= 0) {
	printk("\n  Reread:");
	for (i = 0; i < dw; i++) {
	    printk(" 0x%08x", reread_data[i] & mask_data[i]);
	}
    }
    test_error(unit, "\n");

    return MT_MISCOMPARE_STOP;
}

int
mem_test_init(int u, args_t *a, void **p)
{
    mem_testdata_t	*mw;
    soc_mem_test_t	*parm;
    char		*mem_name;
    char		*idx_start_str, *idx_end_str;
    int			copies, blk;
    int			rv = -1;
    parse_table_t	pt;

    mw = mem_work[u];
    if (mw == NULL) {
	mw = sal_alloc(sizeof(mem_testdata_t), "memtest");
	if (mw == NULL) {
	    printk("%s: cannot allocate memory test data\n", ARG_CMD(a));
	    return -1;
	}
	sal_memset(mw, 0, sizeof(mem_testdata_t));
	mem_work[u] = mw;
    }
    parm = &mw->parm;

    parse_table_init(u, &pt);
    parse_table_add(&pt,  "Memory",	PQ_STRING, "",
		    &mem_name, NULL);
    parse_table_add(&pt,  "IndexStart",	PQ_STRING, "min",
		    &idx_start_str, NULL);
    parse_table_add(&pt,  "IndexEnd",	PQ_STRING, "max",
		    &idx_end_str, NULL);
    parse_table_add(&pt,  "IndexInc",	PQ_INT, (void *) 1,
		    &parm->index_step, NULL);

    parse_table_add(&pt,  "PatZero",	PQ_BOOL|PQ_DFL,	0,
		    &mw->pat_zero, NULL);
    parse_table_add(&pt,  "PatOne",	PQ_BOOL|PQ_DFL,	0,
		    &mw->pat_one, NULL);
    parse_table_add(&pt,  "PatFive",	PQ_BOOL|PQ_DFL,	0,
		    &mw->pat_five, NULL);
    parse_table_add(&pt,  "PatA",	PQ_BOOL|PQ_DFL,	0,
		    &mw->pat_a, NULL);
    parse_table_add(&pt,  "PatChecker",	PQ_BOOL|PQ_DFL,	0,
		    &mw->pat_checker, NULL);
    parse_table_add(&pt,  "PatInvckr",	PQ_BOOL|PQ_DFL,	0,
		    &mw->pat_invckr, NULL);
    parse_table_add(&pt,  "PatLinear",	PQ_BOOL|PQ_DFL,	0,
		    &mw->pat_linear, NULL);
    parse_table_add(&pt,  "PatRandom",	PQ_BOOL|PQ_DFL,	0,
		    &mw->pat_random, NULL);
    parse_table_add(&pt,  "PatHex",  PQ_BOOL|PQ_DFL, 0,
                    &mw->pat_hex, NULL);
    parse_table_add(&pt,  "HexByte",  PQ_INT|PQ_DFL, 0,
                    &mw->hex_byte, NULL);
    parse_table_add(&pt,  "TestByEntry",  PQ_BOOL|PQ_DFL, 0,
                    &mw->test_by_entry, NULL);
    parse_table_add(&pt,  "ReVerifyCount",  PQ_INT|PQ_DFL, 0,
                    &mw->reverify_count, NULL);
    parse_table_add(&pt,  "ReVerifyDelay",  PQ_INT|PQ_DFL, 0,
                    &mw->reverify_delay, NULL);
    parse_table_add(&pt,  "IGnoreErrors",  PQ_BOOL|PQ_DFL, 0,
                    &mw->continue_on_error, NULL);
    parse_table_add(&pt,  "ErrorCount",  PQ_INT|PQ_DFL, 0,
                    &mw->error_count, NULL);
    parse_table_add(&pt,  "ErrorMax",  PQ_INT|PQ_DFL, (void *) 1,
                    &mw->error_max, NULL);
    parse_table_add(&pt,  "ReadCount",	PQ_INT|PQ_DFL,	(void *) 1,
		    &parm->read_count, NULL);
    parse_table_add(&pt, "EccAsData",   PQ_BOOL|PQ_DFL,  0,
                    &parm->ecc_as_data, NULL);

    mw->was_debug_mode = -1;

    if (!mw->inited) {
	mw->pat_zero = TRUE;
	mw->pat_one = TRUE;
	mw->pat_five = TRUE;
	mw->pat_a = TRUE;
	mw->pat_checker = TRUE;
	mw->pat_invckr = TRUE;
	mw->pat_linear = TRUE;
	mw->pat_random = TRUE;
	parm->unit = u;
	parm->status_cb = mt_status_fn;
	parm->write_cb = mt_write_fn;
	parm->read_cb = mt_read_fn;
	parm->miscompare_cb = mt_miscompare_fn;
	parm->read_count = 1;
        parm->ecc_as_data = 0;
	mw->inited = 1;
    }

    if (parse_arg_eq(a, &pt) < 0) {
	printk("%s: Invalid option: %s\n",
	       ARG_CMD(a), ARG_CUR(a));
	goto done;
    }

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(u)) {
      if (sal_strcasecmp(mem_name,"EXT_DDR") == 0) {
	return (sbx_mem_init(u,a,p));
      }
    }
#endif

    if (ARG_CNT(a) != 0) {
	printk("%s: extra options starting with \"%s\"\n",
	       ARG_CMD(a), ARG_CUR(a));
	goto done;
    }

    if (parse_memory_name(u,
			  &parm->mem,
			  mem_name,
			  &parm->copyno) < 0) {
	test_error(u, "Memory \"%s\" is invalid\n", mem_name);
	goto done;
    }

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(u)) {
        if ( (parm->mem == EG_FD_PER_PORT_DROP_COUNT1m) ||
	     (parm->mem == EG_FD_PER_PORT_DROP_COUNT2m) ||
	     (parm->mem == GLOBAL_STATSm) ||
	     (parm->mem == RT_BKm) ||
	     (parm->mem == TC_FREE_POOLm) ||
	     (parm->mem == TX_SFI_CFIFOm) ||
	     (parm->mem == TX_SFI_DFIFOm) ) {
	    printk("WARNING: TR 50 doesn't apply to memory %s\n",
		   SOC_MEM_UFNAME(u, parm->mem));
	    goto done;
	}
    }
#endif

    if (!soc_mem_is_valid(u, parm->mem) ||
	soc_mem_is_readonly(u, parm->mem)) {
	test_error(u, "Memory %s is invalid or readonly\n",
		   SOC_MEM_UFNAME(u, parm->mem));
	goto done;
    }

    if (parm->copyno != COPYNO_ALL &&
	!SOC_MEM_BLOCK_VALID(u, parm->mem, parm->copyno)) {
	test_error(u,
		   "Copy number %d out of range for memory %s\n",
		   parm->copyno, SOC_MEM_UFNAME(u, parm->mem));
	goto done;
    }

    if (soc_mem_cache_get(u, parm->mem,
			  parm->copyno == COPYNO_ALL ?
			  MEM_BLOCK_ALL : parm->copyno)) {
	printk("WARNING: Caching is enabled on memory %s.%s\n",
	       SOC_MEM_UFNAME(u, parm->mem),
	       SOC_BLOCK_NAME(u, parm->copyno));
    }

    parm->index_start =
	parse_memory_index(parm->unit, parm->mem, idx_start_str);
    parm->index_end =
	parse_memory_index(parm->unit, parm->mem, idx_end_str);

    if (parm->index_start < soc_mem_index_min(parm->unit, parm->mem)) {
	printk("WARNING: Adjust start index from %d to %d for memory %s.%s\n",
               parm->index_start, soc_mem_index_min(parm->unit, parm->mem),
	       SOC_MEM_UFNAME(u, parm->mem),
	       SOC_BLOCK_NAME(u, parm->copyno));
        parm->index_start = soc_mem_index_min(parm->unit, parm->mem);
    }
    if (parm->index_end > soc_mem_index_max(parm->unit, parm->mem)) {
	printk("WARNING: Adjust end index from %d to %d for memory %s.%s\n",
               parm->index_end, soc_mem_index_max(parm->unit, parm->mem),
	       SOC_MEM_UFNAME(u, parm->mem),
	       SOC_BLOCK_NAME(u, parm->copyno));
        parm->index_end = soc_mem_index_max(parm->unit, parm->mem);
    }

    if (SOC_IS_LYNX(parm->unit)) {
        if ((parm->mem == DEFIPm) &&
            (parm->index_end == soc_mem_index_max(parm->unit, DEFIPm))) {
            /* Last DEFIP entry is not memory, and does not have parity */
            parm->index_end--;
        }
    } else if (SOC_IS_TUCANA(parm->unit)) {
        if ((parm->mem == DEFIP_HIm) &&
            (parm->index_end == soc_mem_index_max(parm->unit, DEFIP_HIm))) {
            /* Last DEFIP entry is not memory, and does not have parity */
            parm->index_end--;
        }
        if ((parm->mem == DEFIP_LOm) &&
            (parm->index_end == soc_mem_index_max(parm->unit, DEFIP_LOm))) {
            /* Last DEFIP entry is not memory, and does not have parity */
            parm->index_end--;
        }
    }

    if (parm->index_step == 0 ||
	(parm->index_start < parm->index_end && parm->index_step < 0) ||
	(parm->index_start > parm->index_end && parm->index_step > 0)) {
	test_error(u, "Illegal index step value\n");
	goto done;
    }

    parm->patterns = 0;
    if (mw->pat_zero)
	parm->patterns |= MT_PAT_ZEROES;
    if (mw->pat_one)
	parm->patterns |= MT_PAT_ONES;
    if (mw->pat_five)
	parm->patterns |= MT_PAT_FIVES;
    if (mw->pat_a)
	parm->patterns |= MT_PAT_AS;
    if (mw->pat_checker)
	parm->patterns |= MT_PAT_CHECKER;
    if (mw->pat_invckr)
	parm->patterns |= MT_PAT_ICHECKER;
    if (mw->pat_linear)
	parm->patterns |= MT_PAT_ADDR;
    if (mw->pat_random)
	parm->patterns |= MT_PAT_RANDOM;
    if (mw->pat_hex) {
        parm->patterns |= MT_PAT_HEX;
        parm->hex_byte = mw->hex_byte;
    }
    parm->test_by_entry = mw->test_by_entry;
    if (parm->test_by_entry) {
        /* Not supported unless in fill mode */
        mw->reverify_delay = 0;
        mw->reverify_count = 0;
    }
    parm->reverify_delay = mw->reverify_delay;
    parm->reverify_count = mw->reverify_count;
    parm->continue_on_error = mw->continue_on_error;
    parm->error_count = mw->error_count;
    parm->error_max = mw->error_max;

    /* Place MMU in debug mode if testing an MMU memory */

    if (soc_mem_is_debug(u, parm->mem) &&
	(mw->was_debug_mode = soc_mem_debug_set(u, 0)) < 0) {
	test_error(u, "Could not put MMU in debug mode\n");
	goto done;
    }

    /* Disable any parity control */

    if ((rv = soc_mem_parity_control(u, parm->mem, parm->copyno, FALSE)) < 0) {
	test_error(u, "Could not disable parity warnings on memory %s\n",
		   SOC_MEM_UFNAME(u, parm->mem));
	goto done;
    }

    /*
     * Turn off L2 task to keep it from going crazy if L2 memory is
     * being tested.
     */

#ifdef BCM_XGS_SWITCH_SUPPORT
    if (soc_feature(u, soc_feature_arl_hashed)) {
	(void)soc_l2x_stop(u);
    }
#endif /* BCM_XGS_SWITCH_SUPPORT */
#ifdef BCM_SHADOW_SUPPORT
    if (SOC_IS_SHADOW(u) && (parm->mem == EGR_PERQ_XMT_COUNTERSm)) {
        rv = soc_reg_field32_modify(u, EGR_EDB_HW_CONTROLr, REG_PORT_ANY,
                                    XGS_COUNTER_COMPAT_MODEf, 1);
       if (rv != SOC_E_NONE) {
           goto done;
       }
    }
    if (SOC_IS_SHADOW(u) && SOC_BLOCK_IS_CMP(u,
        SOC_MEM_BLOCK_MIN(u, parm->mem), SOC_BLK_MS_ISEC)) {
        rv = soc_reg_field32_modify(u, ISEC_MASTER_CTRLr, 1,
                                    XGS_COUNTER_COMPAT_MODEf, 1);
        if (rv != SOC_E_NONE) {
            return -1;
       }
        rv = soc_reg_field32_modify(u, ISEC_MASTER_CTRLr, 5,
                                    XGS_COUNTER_COMPAT_MODEf, 1);
       if (rv != SOC_E_NONE) {
           return -1;
       }
    }
    /* Disable WRED */
    if (SOC_IS_SHADOW(u)) {
        uint32              rval;
        SOC_IF_ERROR_RETURN(READ_MISCCONFIGr(u, &rval));
        soc_reg_field_set(u, MISCCONFIGr, &rval,
                      METERING_CLK_ENf, 0);
        soc_reg_field_set(u, MISCCONFIGr, &rval,
                      REFRESH_ENf, 0);
        SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(u, rval));
    }
    if (SOC_IS_SHADOW(u) && SOC_BLOCK_IS_CMP(u,
        SOC_MEM_BLOCK_MIN(u, parm->mem), SOC_BLK_MS_ESEC)) {
        rv = soc_reg_field32_modify(u, ESEC_MASTER_CTRLr, 1,
                                    XGS_COUNTER_COMPAT_MODEf, 1);
        if (rv != SOC_E_NONE) {
            return -1;
        }
        rv = soc_reg_field32_modify(u, ESEC_MASTER_CTRLr, 5,
                                    XGS_COUNTER_COMPAT_MODEf, 1);
        if (rv != SOC_E_NONE) {
           return -1;
        }
    }
#endif

    /*
     * Turn off FP.
     * Required when testing FP and METERING memories or tests will fail.
     * We don't care about the return value as the BCM layer may not have
     * been initialized at all.
     */

#ifndef BCM_SIRIUS_SUPPORT
    (void)soc_filter_enable_set(u, 0);
#endif

    (void)bcm_field_detach(u);

#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(u, soc_feature_esm_support)) {
        /* Some tables have dependency on the content of other table */
        switch (parm->mem) {
        case EXT_ACL360_TCAM_DATAm:
        case EXT_ACL360_TCAM_DATA_IPV6_SHORTm:
            rv = soc_mem_clear(u, EXT_ACL360_TCAM_MASKm, MEM_BLOCK_ALL, TRUE);
            break;
        case EXT_ACL432_TCAM_DATAm:
        case EXT_ACL432_TCAM_DATA_IPV6_LONGm:
        case EXT_ACL432_TCAM_DATA_L2_IPV4m:
        case EXT_ACL432_TCAM_DATA_L2_IPV6m:
            rv = soc_mem_clear(u, EXT_ACL432_TCAM_MASKm, MEM_BLOCK_ALL, TRUE);
            break;
        default:
            break;
        }
	if (rv != SOC_E_NONE) {
	    goto done;
	}
    }
#endif /* BCM_TRIUMPH_SUPPORT */

    if (parm->copyno != COPYNO_ALL) {
	copies = 1;
    } else {
	copies = 0;
	SOC_MEM_BLOCK_ITER(u, parm->mem, blk) {
	    copies += 1;
	}
    }
    mw->progress_total =
	(_shr_popcount(parm->patterns) *
         (1 + (parm->read_count * (1 + parm->reverify_count))) *
	 copies *
	 ((parm->index_end > parm->index_start) ?
	  (parm->index_end - parm->index_start + 1) / parm->index_step :
	  (parm->index_end - parm->index_start - 1) / parm->index_step));

    *p = mw;

    rv = 0;

 done:
    parse_arg_eq_done(&pt);
    return rv;
}

/*
 * Memory Test
 */

int
mem_test(int u, args_t *a, void *p)
{
    mem_testdata_t	*mw = p;
    int			rv;

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(u)) {
      
      if (mw->inited == SBX_EXT_DDR_FLAG) {
	  return (sbx_mem_test(u,a,p));
      }
    }
#endif

    progress_init(mw->progress_total, 3, 0);

    rv = soc_mem_test(&mw->parm);

    progress_done();

    return rv;
}

int
mem_test_done(int unit, void *p)
{
    mem_testdata_t	*mw = p;
#ifdef BCM_SHADOW_SUPPORT
    int rv = SOC_E_NONE;
#endif

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
      
      if (mw->inited == SBX_EXT_DDR_FLAG) {
	return (sbx_test_done(unit,p));
      }
    }
#endif

    if (mw) {
	soc_mem_test_t	*parm = &mw->parm;

	/* Take MMU out of debug mode if testing a CBP memory */

	if (soc_mem_is_debug(parm->unit, parm->mem) &&
	    mw->was_debug_mode >= 0 &&
	    soc_mem_debug_set(unit, mw->was_debug_mode) < 0) {
	    test_error(unit, "Could not restore previous MMU debug state\n");
	    return -1;
	}

	if (soc_mem_parity_restore(unit, parm->mem, parm->copyno) < 0) {
	    test_error(unit, "Could not enable parity warnings on memory %s\n",
		       SOC_MEM_UFNAME(unit, parm->mem));
	    return -1;
	}
#ifdef BCM_SHADOW_SUPPORT
        if (SOC_IS_SHADOW(unit) && (parm->mem == EGR_PERQ_XMT_COUNTERSm)) {
            rv = soc_reg_field32_modify(unit, EGR_EDB_HW_CONTROLr, REG_PORT_ANY,
                                        XGS_COUNTER_COMPAT_MODEf, 0);
           if (rv != SOC_E_NONE) {
               return -1;
           }
        }
        if (SOC_IS_SHADOW(unit)) {
            uint32              rval;
            SOC_IF_ERROR_RETURN(READ_MISCCONFIGr(unit, &rval));
            soc_reg_field_set(unit, MISCCONFIGr, &rval,
                          METERING_CLK_ENf, 1);
            soc_reg_field_set(unit, MISCCONFIGr, &rval,
                          REFRESH_ENf, 1);
            SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, rval));
        }

        if (SOC_IS_SHADOW(unit) && SOC_BLOCK_IS_CMP(unit,
            SOC_MEM_BLOCK_MIN(unit, parm->mem), SOC_BLK_MS_ISEC)) {
            rv = soc_reg_field32_modify(unit, ISEC_MASTER_CTRLr, 1,
                                        XGS_COUNTER_COMPAT_MODEf, 0);
           if (rv != SOC_E_NONE) {
               return -1;
           }
            rv = soc_reg_field32_modify(unit, ISEC_MASTER_CTRLr, 5,
                                        XGS_COUNTER_COMPAT_MODEf, 0);
           if (rv != SOC_E_NONE) {
               return -1;
           }
        }
        if (SOC_IS_SHADOW(unit) && SOC_BLOCK_IS_CMP(unit,
            SOC_MEM_BLOCK_MIN(unit, parm->mem), SOC_BLK_MS_ESEC)) {
            rv = soc_reg_field32_modify(unit, ESEC_MASTER_CTRLr, 1,
                                        XGS_COUNTER_COMPAT_MODEf, 0);
           if (rv != SOC_E_NONE) {
               return -1;
           }
            rv = soc_reg_field32_modify(unit, ESEC_MASTER_CTRLr, 5,
                                        XGS_COUNTER_COMPAT_MODEf, 0);
           if (rv != SOC_E_NONE) {
               return -1;
           }
        }
#endif

    }
    return 0;
}
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT */
