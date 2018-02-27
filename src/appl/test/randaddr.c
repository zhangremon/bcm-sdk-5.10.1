/*
 * $Id: randaddr.c 1.42.20.1 Broadcom SDK $
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
 * Random index memory test implemented via S-Channel table read/write.
 *
 * This test function is a variation on memrand for random memory
 * testing.  It was requested by Govind to do the following:
 *
 * Write every word in the memory each pass.
 * Write all then read all
 * Vary the pattern each write, for example, from all 1s to all 0s.
 * For different seeds, to order of writes to the memory should vary.
 *
 * Randomizing algorithm:
 *   Inputs: current value, original seed.
 *
 */

#include <sal/types.h>

#include <sal/appl/pci.h>

#include <appl/diag/system.h>
#include <appl/diag/parse.h>
#include <appl/diag/progress.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/filter.h>
#include <soc/l2x.h>

#include "testlist.h"

#if defined (BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
/*
 * Return true if a and b are relatively prime
 * Bad O(n) algorithm
 */
static uint32
relprime(uint32 a, uint32 b)
{
    uint32 i;
    uint32 min = a < b ? a : b;
    uint32 max = a < b ? b : a;

    if (max % min == 0) {
        return 0;
    }

    if ((a % 2 == 0) && (b % 2 == 0)) {
        return 0;
    }

    for (i = 3; i <= min / 2; i += 2) {
        if ((a % i == 0) && (b % i == 0)) {
            return 0;
        }
    }

    return 1;
}

/*
 * Return the first value greater than or equal to start that is
 * relatively prime to val.
 */
static uint32
get_rel_prime(uint32 val, uint32 start)
{
    while (!relprime(val, start)) {
        start++;
    }

    return start;
}


/*
 * Memory test status structure
 */

typedef struct rand_work_s {
    int was_debug_mode;

    int iters;                          /* Total iterations to perform */

    soc_mem_t mem;                      /* Memory to test */

    int copyno;                         /* copy to test (or COPYNO_ALL) */
    int copyno_total;                   /* Number of copies */

    int index_min;                      /* Lowest index in range to test */
    int index_max;                      /* Last index in range to test */
    int index_total;                    /* Size of index range (max-min+1) */

    uint32 data_start;                  /* initial data value */
    uint32 data;                        /* current data value */
    uint32 seed;                        /* seed */
    int ecc_as_data;                    /* treat ecc field as regular field */
    uint32 addr_incr;                   /* for internal random */
    uint32 first_addr;
    uint32 cur_addr;
} rand_work_t;

static rand_work_t      *rand_work[SOC_MAX_NUM_DEVICES];

int
addr_rand_init(int unit, args_t *a, void **p)
{
    rand_work_t         *rw;
    char                *mem_name;
    char                *idx_start_str, *idx_end_str;
    int                 blk;
    int                 rv = -1;
    parse_table_t       pt;

    rw = rand_work[unit];
    if (rw == NULL) {
        rw = sal_alloc(sizeof(rand_work_t), "randaddr");
        if (rw == NULL) {
            printk("%s: cannot allocate memory test data\n", ARG_CMD(a));
            return -1;
        }
        sal_memset(rw, 0, sizeof(rand_work_t));
        rand_work[unit] = rw;
    }

    parse_table_init(unit, &pt);
    parse_table_add(&pt,  "Memory",     PQ_STRING, "",
                    &mem_name, NULL);
    parse_table_add(&pt,  "IndexStart", PQ_STRING, (void *) "min",
                    &idx_start_str, NULL);
    parse_table_add(&pt,  "IndexEnd",   PQ_STRING, (void *) "max",
                    &idx_end_str, NULL);
    parse_table_add(&pt,  "ITERations", PQ_INT, (void *) 10,
                    &rw->iters, NULL);
    parse_table_add(&pt,  "SEED",       PQ_INT, (void *) 0xdecade,
                    &rw->seed, NULL );
    parse_table_add(&pt,  "EccAsData",  PQ_BOOL, 0,
                    &rw->ecc_as_data, NULL );
    parse_table_add(&pt,  "InitialData", PQ_INT, (void *) 0xffffffff,
                    &rw->data_start, NULL );

    if (parse_arg_eq(a, &pt) < 0) {
        printk("%s: Invalid option: %s\n",
               ARG_CMD(a), ARG_CUR(a));
        goto done;
    }

    if (ARG_CNT(a) != 0) {
        printk("%s: extra options starting with \"%s\"\n",
               ARG_CMD(a), ARG_CUR(a));
        goto done;
    }

    if (mem_name == 0 || *mem_name == 0 ||
        parse_memory_name(unit,
                          &rw->mem,
                          mem_name,
                          &rw->copyno) < 0) {
        test_error(unit,
                   "Missing or unknown memory name "
                   "(use listmem for list)\n");
        goto done;
    }

    if (!soc_mem_is_valid(unit, rw->mem) ||
        soc_mem_is_readonly(unit, rw->mem) ||
        soc_mem_index_max(unit, rw->mem) < 3) {
        test_error(unit,
                   "Cannot test memory %s with this command\n",
                   SOC_MEM_UFNAME(unit, rw->mem));
        goto done;
    }

    if (rw->copyno == COPYNO_ALL) {
        rw->copyno_total = 0;
        SOC_MEM_BLOCK_ITER(unit, rw->mem, blk) {
            rw->copyno_total += 1;
        }
    } else {
        rw->copyno_total = 1;
        if (!SOC_MEM_BLOCK_VALID(unit, rw->mem, rw->copyno)) {
            test_error(unit,
                       "Copy number out of range for memory %s\n",
                       SOC_MEM_UFNAME(unit, rw->mem));
            goto done;
        }
    }

    if (soc_mem_cache_get(unit, rw->mem,
                          rw->copyno == COPYNO_ALL ?
                          MEM_BLOCK_ALL : rw->copyno)) {
        printk("WARNING: Caching is enabled on memory %s.%s\n",
               SOC_MEM_UFNAME(unit, rw->mem),
               SOC_BLOCK_NAME(unit, rw->copyno));
    }

    rw->index_min = parse_memory_index(unit, rw->mem, idx_start_str);
    rw->index_max = parse_memory_index(unit, rw->mem, idx_end_str);
    if (SOC_IS_LYNX(unit)) {
        if ((rw->mem == DEFIPm) &&
            (rw->index_max == soc_mem_index_max(unit, DEFIPm))) {
            /* Last DEFIP entry is not memory, and does not have parity */
            rw->index_max--;
        }
    } else if (SOC_IS_TUCANA(unit) &&
                ((rw->mem == DEFIP_HIm) || (rw->mem == DEFIP_LOm))) {
        /* Last DEFIP entry is not memory, and does not have parity */
        rw->index_max--;
    }
#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FB_FX_HX(unit)) {
        if (rw->index_min < soc_mem_index_min(unit, rw->mem)) {
            rw->index_min = soc_mem_index_min(unit, rw->mem);
        }
    }
#endif /* BCM_FIREBOLT_SUPPORT */
    rw->index_total = rw->index_max - rw->index_min + 1;

    if (rw->index_total < 1 || rw->copyno_total < 1) {
        test_error(unit,
                   "Min copyno/index must be less than max copyno/index\n");
        goto done;
    }

    /* Place MMU in debug mode if testing a CBP memory */

    if (soc_mem_is_debug(unit, rw->mem) &&
        (rw->was_debug_mode = soc_mem_debug_set(unit, 0)) < 0) {
        test_error(unit, "Could not put MMU in debug mode\n");
        goto done;
    }

    /* Disable any parity control (herc and lynx) */

    if (soc_mem_parity_control(unit, rw->mem, rw->copyno, FALSE) < 0) {
        test_error(unit, "Could not disable parity warnings on memory %s\n",
                   SOC_MEM_UFNAME(unit, rw->mem));
        goto done;
    }

    /*
     * Turn off L2 task to keep it from going crazy if L2 memory is
     * being tested.
     */

#ifdef BCM_XGS_SWITCH_SUPPORT
    if (soc_feature(unit, soc_feature_arl_hashed)) {
        (void)soc_l2x_stop(unit);
    }
#endif /* BCM_XGS_SWITCH_SUPPORT */
#ifdef BCM_SHADOW_SUPPORT
    if (SOC_IS_SHADOW(unit) && (rw->mem == EGR_PERQ_XMT_COUNTERSm)) {
        rv = soc_reg_field32_modify(unit, EGR_EDB_HW_CONTROLr, REG_PORT_ANY,
                                    XGS_COUNTER_COMPAT_MODEf, 1);
       if (rv != SOC_E_NONE) {
           goto done;
       }
    }
    if (SOC_IS_SHADOW(unit) && SOC_BLOCK_IS_CMP(unit,
        SOC_MEM_BLOCK_MIN(unit, rw->mem), SOC_BLK_MS_ISEC)) {
        rv = soc_reg_field32_modify(unit, ISEC_MASTER_CTRLr, 1,
                                    XGS_COUNTER_COMPAT_MODEf, 1);
        if (rv != SOC_E_NONE) {
            return -1;
       }
        rv = soc_reg_field32_modify(unit, ISEC_MASTER_CTRLr, 5,
                                    XGS_COUNTER_COMPAT_MODEf, 1);
      if (rv != SOC_E_NONE) {
           return -1;
       }
    }
    if (SOC_IS_SHADOW(unit) && SOC_BLOCK_IS_CMP(unit,
        SOC_MEM_BLOCK_MIN(unit, rw->mem), SOC_BLK_MS_ESEC)) {
        rv = soc_reg_field32_modify(unit, ESEC_MASTER_CTRLr, 1,
                                    XGS_COUNTER_COMPAT_MODEf, 1);
        if (rv != SOC_E_NONE) {
            return -1;
        }
        rv = soc_reg_field32_modify(unit, ESEC_MASTER_CTRLr, 5,
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
    (void)soc_filter_enable_set(unit, 0);
#endif

    (void)bcm_field_detach(unit);

#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(unit, soc_feature_esm_support)) {
        /* Some tables have dependency on the content of other table */
        switch (rw->mem) {
        case EXT_ACL360_TCAM_DATAm:
        case EXT_ACL360_TCAM_DATA_IPV6_SHORTm:
            rv = soc_mem_clear(unit, EXT_ACL360_TCAM_MASKm, 
                               MEM_BLOCK_ALL, TRUE);
            if (rv < 0) {
                test_error(unit, "Could not clear EXT_ACL360_TCAM_MASK\n");
                goto done;
            }
            break;
        case EXT_ACL432_TCAM_DATAm:
        case EXT_ACL432_TCAM_DATA_IPV6_LONGm:
        case EXT_ACL432_TCAM_DATA_L2_IPV4m:
        case EXT_ACL432_TCAM_DATA_L2_IPV6m:
            rv = soc_mem_clear(unit, EXT_ACL432_TCAM_MASKm, 
                               MEM_BLOCK_ALL, TRUE);
            if (rv < 0) {
                test_error(unit, "Could not clear EXT_ACL432_TCAM_MASK\n");
                goto done;
            }
            break;
        default:
            break;
        }
    }
#endif /* BCM_TRIUMPH_SUPPORT */

    /* Choose a value for increment that is
       relatively prime to total index. */
    rw->addr_incr = get_rel_prime(rw->index_total, rw->seed *
                                  rw->index_total);
    rw->first_addr = (rw->addr_incr % rw->index_total) + rw->index_min;
    debugk(DK_TESTS, "Running with simple seed.  Incr: %d. First 0x%x.\n",
           rw->addr_incr, rw->first_addr);

    *p = rw;

    rv = 0;

 done:
    parse_arg_eq_done(&pt);
    return rv;
}

#define NEXT_ADDR(cur) \
  ((((cur) - rw->index_min + rw->addr_incr) % rw->index_total) + rw->index_min)


int
addr_rand(int unit, args_t *a, void *p)
{
    rand_work_t         *rw = p;
    uint32              mask[SOC_MAX_MEM_WORDS];
    uint32              tcammask[SOC_MAX_MEM_WORDS];
    uint32              eccmask[SOC_MAX_MEM_WORDS];
    uint32              accum_tcammask;
    int                 iter, i, dw, word, copyno;
    uint32              data[SOC_MAX_MEM_WORDS];
    char                status[160];
    int                 count;
    uint32              miscomp;

    COMPILER_REFERENCE(a);

    dw = soc_mem_entry_words(unit, rw->mem);
    soc_mem_datamask_get(unit, rw->mem, mask);
    soc_mem_tcammask_get(unit, rw->mem, tcammask);
    soc_mem_eccmask_get(unit, rw->mem, eccmask);
    accum_tcammask = 0;
    for (i = 0; i < dw; i++) {
        accum_tcammask |= tcammask[i];
    }
    if (!rw->ecc_as_data) {
        for (i = 0; i < dw; i++) {
            mask[i] &= ~eccmask[i];
        }
    }
    soc_mem_datamask_memtest(unit, rw->mem, mask);

    progress_init(rw->copyno_total * rw->iters * rw->index_total * 2, 3, 0);

    SOC_MEM_BLOCK_ITER(unit, rw->mem, copyno) {
        if (rw->copyno != COPYNO_ALL && rw->copyno != copyno) {
            continue;
        }

        sal_sprintf(status,
                "Running %d iterations on %s.%s[%d-%d]",
                rw->iters,
                SOC_MEM_UFNAME(unit, rw->mem),
                SOC_BLOCK_NAME(unit, copyno),
                rw->index_min, rw->index_max);

        progress_status(status);

        for (iter = 0; iter < rw->iters; iter++) {
            /* Set up addr and data and write to all memory addresses */

            rw->cur_addr = rw->first_addr;
            rw->data = rw->data_start;

            count = 0;

            do {
                count++;

                if (soc_mem_entry_words(unit, rw->mem) % 2 == 0) {
                    rw->data = ~rw->data;
                }
                for (word = 0; word < dw; word++) {
                    data[word] = rw->data & mask[word];
                    rw->data = ~rw->data;
                }
                if (accum_tcammask) {
                    /* data read back has dependency on mask */
                    if (soc_feature(unit, soc_feature_xy_tcam)) {
                        for (word = 0; word < dw; word++) {
                            data[word] |= tcammask[word];
                        }
                    } else if (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_ESM) {
                        for (word = 0; word < dw; word++) {
                            data[word] &= ~tcammask[word];
                        }
                    }
                }

                if (soc_mem_write(unit, rw->mem, copyno,
                                  rw->cur_addr, data) < 0) {
                    printk("Write ERROR: table %s.%s[%d] iteration %d\n",
                           SOC_MEM_UFNAME(unit, rw->mem),
                           SOC_BLOCK_NAME(unit, copyno),
                           rw->cur_addr, iter);
                    goto break_all;
                }

                rw->cur_addr = NEXT_ADDR(rw->cur_addr);

                progress_report(1);
            } while (rw->cur_addr != rw->first_addr);

            /* Reset addr and data and Verify values written */

            rw->cur_addr = rw->first_addr;
            rw->data = rw->data_start;

            miscomp = 0;

            if (count != rw->index_total) {
                printk("WARNING:  number of writes != index total\n");
            }

            do {
                if (soc_mem_entry_words(unit, rw->mem) % 2 == 0) {
                    rw->data = ~rw->data;
                }
                if (soc_mem_read(unit, rw->mem, copyno,
                                 rw->cur_addr, data) < 0) {
                    printk("Read ERROR: table %s.%s[%d] iteration %d\n",
                           SOC_MEM_UFNAME(unit, rw->mem),
                           SOC_BLOCK_NAME(unit, copyno),
                           rw->cur_addr, iter);
                    goto break_all;
                }

                for (word = 0; word < dw; word++) {
                    uint32 comp_word = rw->data & mask[word];
                    if (accum_tcammask) {
                        /* data read back has dependency on mask */
                        if (soc_feature(unit, soc_feature_xy_tcam)) {
                            comp_word |= tcammask[word];
                        } else if (SOC_BLOCK_TYPE(unit, copyno) ==
                                   SOC_BLK_ESM) {
                            comp_word &= ~tcammask[word];
                        }
                    }
                    if ((data[word] ^ comp_word) & mask[word]) {
                        soc_pci_analyzer_trigger(unit);

                        printk("Compare ERROR: table %s.%s iteration %d\n",
                               SOC_MEM_UFNAME(unit, rw->mem),
                               SOC_BLOCK_NAME(unit, copyno),
                               iter+1);
                        printk("Wrote word %d: 0x%08x.\n", word, comp_word);
                        printk("Read word %d:  0x%08x.\n", word, data[word]);
                        printk("Difference:    0x%08x.\n",
                               comp_word ^ data[word]);
                        miscomp = 1;
                    }
                    rw->data = ~rw->data;
                }

                rw->cur_addr = NEXT_ADDR(rw->cur_addr);

                progress_report(1);
            } while ((rw->cur_addr != rw->first_addr) && (!miscomp));

            if (miscomp) {
                printk("Cycle aborted due to error\n");
                test_error(unit, "\n");
            }
        }
    }

 break_all:
    progress_done();

    return 0;
}

int
addr_rand_done(int unit, void *p)
{
    rand_work_t         *rw = p;
#ifdef BCM_SHADOW_SUPPORT
    int rv = SOC_E_NONE;
#endif

    if (rw) {
        /* Take MMU out of debug mode if testing a CBP memory */

        if (soc_mem_is_debug(unit,rw->mem) &&
            rw->was_debug_mode >= 0 &&
            soc_mem_debug_set(unit, rw->was_debug_mode) < 0) {
            test_error(unit, "Could not restore previous MMU debug state\n");
            return -1;
        }

	if (soc_mem_parity_restore(unit, rw->mem, rw->copyno) < 0) {
	    test_error(unit, "Could not enable parity warnings on memory %s\n",
		       SOC_MEM_UFNAME(unit, rw->mem));
	    return -1;
	}
#ifdef BCM_SHADOW_SUPPORT
        if (SOC_IS_SHADOW(unit) && (rw->mem == EGR_PERQ_XMT_COUNTERSm)) {
            rv = soc_reg_field32_modify(unit, EGR_EDB_HW_CONTROLr, REG_PORT_ANY,
                                        XGS_COUNTER_COMPAT_MODEf, 0);
           if (rv != SOC_E_NONE) {
               return -1;
           }
        }
        if (SOC_IS_SHADOW(unit) && SOC_BLOCK_IS_CMP(unit,
            SOC_MEM_BLOCK_MIN(unit, rw->mem), SOC_BLK_MS_ISEC)) {
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
            SOC_MEM_BLOCK_MIN(unit, rw->mem), SOC_BLK_MS_ESEC)) {
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
