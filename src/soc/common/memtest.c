/*
 * $Id: memtest.c 1.13 Broadcom SDK $
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
 * Memory Test Kernel
 *
 * Streamlined module designed for inclusion in the SOC driver for
 * performing power-on memory tests.
 *
 * This module is also used by the main SOC diagnostics memory tests,
 * fronted by user interface code.
 */

#include <sal/core/libc.h>

#include <soc/cm.h>
#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/drv.h>
#ifdef BCM_EASYRIDER_SUPPORT
#include <soc/easyrider.h>
#endif
#ifdef BCM_BRADLEY_SUPPORT
#include <soc/bradley.h>
#endif
#if defined(BCM_TRIUMPH2_SUPPORT)
#include <soc/triumph2.h>
#endif /* BCM_TRIUMPH2_SUPPORT */
#if defined(BCM_TRIDENT_SUPPORT)
#include <soc/trident.h>
#endif /* BCM_TRIDENT_SUPPORT */
#if defined(BCM_ENDURO_SUPPORT)
#include <soc/enduro.h>
#endif /* BCM_ENDURO_SUPPORT */
#if defined(BCM_HURRICANE_SUPPORT)
#include <soc/hurricane.h>
#endif /* BCM_HURRICANE_SUPPORT */

/*
 * Table memory test
 */

#define PAT_PLAIN	0	/* Keep writing same word */
#define PAT_XOR		1	/* XOR 0xffffffff between entries */
#define PAT_RANDOM	2	/* Add 0xdeadbeef between words */
#define PAT_INCR	3	/* Add 1 between words */

STATIC void
fillpat(uint32 *seed, uint32 *mask, uint32 *buf, int pat, int dw)
{
    int			i;

    switch (pat) {
    case PAT_PLAIN:
    default:
	for (i = 0; i < dw; i++) {
	    buf[i] = (*seed) & mask[i];
	}
	break;
    case PAT_XOR:
	for (i = 0; i < dw; i++) {
	    buf[i] = (*seed) & mask[i];
	}
	(*seed) ^= 0xffffffff;
	break;
    case PAT_RANDOM:
	for (i = 0; i < dw; i++) {
	    buf[i] = (*seed) & mask[i];
	    (*seed) += 0xdeadbeef;
	}
	break;
    case PAT_INCR:
	for (i = 0; i < dw; i++) {
	    buf[i] = (*seed) & mask[i];
	}
	(*seed) += 1;
	break;
    }
}

/*
 * Function:     soc_mem_datamask_memtest
 * Purpose:      Patch a bit mask for a memory entry under test
 * Returns:      SOC_E_xxx
 */
uint32 *
soc_mem_datamask_memtest(int unit, soc_mem_t mem, uint32 *buf)
{
#ifdef BCM_SCORPION_SUPPORT
    if (SOC_IS_SC_CQ(unit)) {
        switch (mem) {
        case FP_TCAM_Xm:
            soc_mem_field32_set(unit, FP_TCAM_Xm, buf, IPBMf, 0x1fff);
            soc_mem_field32_set(unit, FP_TCAM_Xm, buf, IPBM_MASKf, 0x1fff);
            break;
        case FP_TCAM_Ym:
            soc_mem_field32_set(unit, FP_TCAM_Ym, buf, IPBMf, 0x1fffe000);
            soc_mem_field32_set(unit, FP_TCAM_Ym, buf, IPBM_MASKf, 0x1fffe000);
            break;
        default:
            break;
        }
    }
#endif
    return buf;
}

/*
 * memtest_fill
 *
 *   parm:         Test parameters
 *   mem:	   soc_mem_t of memory to test
 *   copyno:       which copy of memory to test
 *   seed:         test pattern start value
 *   pat:          test pattern function
 *
 *   Returns 0 on success, -1 on error.
 */

#define FOREACH_INDEX \
  for (index = index_start; \
    index_start <= index_end ? index <= index_end : index >= index_end; \
    index += index_step)

STATIC int
memtest_fill(int unit, soc_mem_test_t *parm, int copyno,
                 uint32 *seed, int pat)
{
    uint32               buf[SOC_MAX_MEM_WORDS];
    uint32               mask[SOC_MAX_MEM_WORDS];
    uint32               tcammask[SOC_MAX_MEM_WORDS];
    uint32               eccmask[SOC_MAX_MEM_WORDS];
    uint32               accum_tcammask;
    soc_mem_t            mem = parm->mem;
    int                  index, dw, rv, i;
    int                  index_start = parm->index_start;
    int                  index_end = parm->index_end;
    int                  index_step = parm->index_step;

    dw = soc_mem_entry_words(unit, mem);
    soc_mem_datamask_get(unit, mem, mask);
    soc_mem_tcammask_get(unit, mem, tcammask);
    soc_mem_eccmask_get(unit, mem, eccmask);
    accum_tcammask = 0;
    for (i = 0; i < dw; i++) {
        accum_tcammask |= tcammask[i];
    }
    if (!parm->ecc_as_data) {
        for (i = 0; i < dw; i++) {
            mask[i] &= ~eccmask[i];
        }
    }
    soc_mem_datamask_memtest(unit, mem, mask);

    soc_cm_debug(DK_VERBOSE,
		 "  FILL %s.%s[%d-%d]\n",
		 SOC_MEM_UFNAME(unit, mem),
		 SOC_BLOCK_NAME(unit, copyno),
		 index_start, index_end);

    if (soc_cm_debug_check(DK_SOCMEM)) {
	soc_cm_print("   MASK");
	for (i = 0; i < dw; i++) {
	    soc_cm_print(" 0x%08x", mask[i]);
	}
	soc_cm_print("\n");
        if (accum_tcammask) {
            soc_cm_print("   TCAM MASK");
            for (i = 0; i < dw; i++) {
                soc_cm_print(" 0x%08x", tcammask[i]);
            }
            soc_cm_print("\n");
        }
    }

    FOREACH_INDEX {
	fillpat(seed, mask, buf, pat, dw);
        if (accum_tcammask) {
            /* data read back has dependency on mask */
            if (soc_feature(unit, soc_feature_xy_tcam)) {
                for (i = 0; i < dw; i++) {
                    buf[i] |= tcammask[i];
                }
            } else if (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_ESM) {
                for (i = 0; i < dw; i++) {
                    buf[i] &= ~tcammask[i];
                }
            }
        }
	if ((rv = (*parm->write_cb)(parm, copyno, index, buf)) < 0) {
	    return rv;
	}
    }

    return SOC_E_NONE;
}

/*
 * memtest_verify
 *
 *   parm:         Test parameters
 *   mem:	   soc_mem_t of memory to test
 *   copyno:       which copy of memory to test
 *   seed:         test pattern start value
 *   incr:         test pattern increment value
 *
 *   Returns 0 on success, -1 on error.
 */

STATIC int
memtest_verify(int unit, soc_mem_test_t *parm, int copyno,
               uint32 *seed, int pat)
{
    uint32		buf[SOC_MAX_MEM_WORDS];
    uint32		mask[SOC_MAX_MEM_WORDS];
    uint32		tcammask[SOC_MAX_MEM_WORDS];
    uint32		eccmask[SOC_MAX_MEM_WORDS];
    uint32              accum_tcammask;
    uint32		cmp[SOC_MAX_MEM_WORDS];
    soc_mem_t		mem = parm->mem;
    int			index, dw, rv, i;
    int			index_start = parm->index_start;
    int			index_end = parm->index_end;
    int			index_step = parm->index_step;
    int			read_cnt;

    dw = soc_mem_entry_words(unit, mem);
    soc_mem_datamask_get(unit, mem, mask);
    soc_mem_tcammask_get(unit, mem, tcammask);
    soc_mem_eccmask_get(unit, mem, eccmask);
    accum_tcammask = 0;
    for (i = 0; i < dw; i++) {
        accum_tcammask |= tcammask[i];
    }
    if (!parm->ecc_as_data) {
        for (i = 0; i < dw; i++) {
            mask[i] &= ~eccmask[i];
        }
    }
    soc_mem_datamask_memtest(unit, mem, mask);

    soc_cm_debug(DK_VERBOSE,
		 "  VERIFY %s.%s[%d-%d] Reading %d times\n",
		 SOC_MEM_UFNAME(unit, mem),
		 SOC_BLOCK_NAME(unit, copyno),
		 index_start, index_end,
		 parm->read_count);

    FOREACH_INDEX {
	fillpat(seed, mask, cmp, pat, dw);
        if (accum_tcammask) {
            /* data read back has dependency on mask */
            if (soc_feature(unit, soc_feature_xy_tcam)) {
                for (i = 0; i < dw; i++) {
                    cmp[i] |= tcammask[i];
                }
            } else if (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_ESM) {
                for (i = 0; i < dw; i++) {
                    cmp[i] &= ~tcammask[i];
                }
            }
        }

	for (read_cnt = 0; read_cnt < parm->read_count; read_cnt++) {
	    if ((rv = (*parm->read_cb)(parm, copyno, index, buf)) < 0) {
		return rv;
	    }

	    for (i = 0; i < dw; i++) {
		if ((buf[i] ^ cmp[i]) & mask[i]) {
		    break;
		}
	    }

	    if (i < dw) {
		parm->err_count++;
		if ((*parm->miscompare_cb)(parm, copyno, index, buf,
				   cmp, mask) == MT_MISCOMPARE_STOP) {
                    parm->error_count++;
                    if (!parm->continue_on_error &&
                               (parm->error_count >= parm->error_max)) {
                        return SOC_E_FAIL;
                    }
		}
	    }
	}
    }

    return SOC_E_NONE;
}

/*
 * memtest_test_by_entry_pattern
 *
 *   Test memories one entry at a time.
 */
STATIC int
memtest_test_by_entry_pattern(int unit, soc_mem_test_t *parm, uint32 seed0,
                          int pat, char *desc)
{
    int                 copyno;
    uint32              buf[SOC_MAX_MEM_WORDS];
    uint32              mask[SOC_MAX_MEM_WORDS];
    uint32              tcammask[SOC_MAX_MEM_WORDS];
    uint32              eccmask[SOC_MAX_MEM_WORDS];
    uint32              cmp[SOC_MAX_MEM_WORDS];
    uint32              accum_tcammask;
    soc_mem_t           mem = parm->mem;
    int                 index, dw, i;
    int                 index_start = parm->index_start;
    int                 index_end = parm->index_end;
    int                 index_step = parm->index_step;
    int                 read_cnt;

    dw = soc_mem_entry_words(unit, mem);
    soc_mem_datamask_get(unit, mem, mask);
    soc_mem_tcammask_get(unit, mem, tcammask);
    soc_mem_eccmask_get(unit, mem, eccmask);
    accum_tcammask = 0;
    for (i = 0; i < dw; i++) {
        accum_tcammask |= tcammask[i];
    }
    if (!parm->ecc_as_data) {
        for (i = 0; i < dw; i++) {
            mask[i] &= ~eccmask[i];
        }
    }
    soc_mem_datamask_memtest(unit, mem, mask);

    SOC_MEM_BLOCK_ITER(unit, mem, copyno) {
        if (parm->copyno != COPYNO_ALL && parm->copyno != copyno) {
            continue;
        }
        FOREACH_INDEX {
            soc_cm_debug(DK_VERBOSE,
                         "  WRITE/READ %s.%s[%d]\n",
                         SOC_MEM_UFNAME(unit, mem),
                         SOC_BLOCK_NAME(unit, copyno),
                         index);
            if (soc_cm_debug_check(DK_SOCMEM)) {
                soc_cm_print("   MASK");
                for (i = 0; i < dw; i++) {
                    soc_cm_print(" 0x%08x", mask[i]);
                }
                soc_cm_print("\n");
                if (accum_tcammask) {
                    soc_cm_print("   TCAM MASK");
                    for (i = 0; i < dw; i++) {
                        soc_cm_print(" 0x%08x", tcammask[i]);
                    }
                }
                soc_cm_print("\n");
            }
            /* First, write the data */
            fillpat(&seed0, mask, buf, pat, dw);
            if (accum_tcammask) {
                /* data read back has dependency on mask */
                if (soc_feature(unit, soc_feature_xy_tcam)) {
                    for (i = 0; i < dw; i++) {
                        buf[i] |= tcammask[i];
                    }
                } else if (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_ESM) {
                    for (i = 0; i < dw; i++) {
                        buf[i] &= ~tcammask[i];
                    }
                }
            }
            for (i = 0; i < dw; i++) {
                cmp[i] = buf[i];
            }
            if ((*parm->write_cb)(parm, copyno, index, buf) < 0) {
                return -1;
            }
            /* Then, read the data */
            for (read_cnt = 0; read_cnt < parm->read_count; read_cnt++) {
                if ((*parm->read_cb)(parm, copyno, index, buf) < 0) {
                    return -1;
                }
                for (i = 0; i < dw; i++) {
                    if ((buf[i] ^ cmp[i]) & mask[i]) {
                        break;
                    }
                }
                if (i < dw) {
                    parm->err_count++;
                    if ((*parm->miscompare_cb)(parm, copyno, index, buf,
                                               cmp, mask) == MT_MISCOMPARE_STOP) {
                        parm->error_count++;
                        if (!parm->continue_on_error &&
                                   (parm->error_count >= parm->error_max)) {
                            return SOC_E_FAIL;
                        }
                    }
                }
            }
        }
    }

    return SOC_E_NONE;
}


/*
 * memtest_pattern
 *
 *   Calls memtest_fill and memtest_verify on copies of the table.
 */
STATIC int
memtest_pattern(int unit, soc_mem_test_t *parm, uint32 seed0,
		int pat, char *desc)
{
    int			copyno;
    uint32		seed;
    uint32              seed_save = 0;
    int			verify, rv, i;
    static char		msg[80];

    if (parm->test_by_entry) {
        return memtest_test_by_entry_pattern(unit, parm, seed0, pat, desc);
    }

    /* Two passes: fill, then verify */

    for (verify = 0; verify < 2; verify++) {
	seed = seed0;

	SOC_MEM_BLOCK_ITER(unit, parm->mem, copyno) {
	    if (parm->copyno != COPYNO_ALL && parm->copyno != copyno) {
		continue;
	    }

	    if (verify) {
                seed_save = seed;
                for (i = 0; i < parm->reverify_count + 1; i++) {
                    if (parm->reverify_delay > 0) {
                        sal_sleep(parm->reverify_delay);
                    }
                    if (parm->status_cb) {
                        sal_sprintf(msg, "Verifying %s", desc);
                        (*parm->status_cb)(parm, msg);
                    }

                    seed = seed_save;
                    if (memtest_verify(unit, parm, copyno, &seed, pat) != 0) {
                        if (!parm->continue_on_error &&
                            (parm->error_count >= parm->error_max)) {
                            return SOC_E_FAIL;
                        }
                    }
                }
	    } else {
		if (parm->status_cb) {
		    sal_sprintf(msg, "Filling %s", desc);
		    (*parm->status_cb)(parm, msg);
		}

		if ((rv = memtest_fill(unit, parm, copyno,
				       &seed, pat)) < 0) {
		    return rv;
		}
	    }
	}
    }

    return SOC_E_NONE;
}

/*
 * Memory test main entry point
 */

int
soc_mem_test(soc_mem_test_t *parm)
{
    int			rv = SOC_E_NONE;
    int			unit = parm->unit;

    parm->err_count = 0;
    
    if (parm->patterns & MT_PAT_ZEROES) {
	/* Detect bits stuck at 1 */
	if ((rv = memtest_pattern(unit, parm, 0x00000000,
				  PAT_PLAIN, "0x00000000")) < 0) {
	    goto done;
	}
    }

    if (parm->patterns & MT_PAT_ONES) {
	/* Detect bits stuck at 0 */
	if ((rv = memtest_pattern(unit, parm, 0xffffffff,
				  PAT_PLAIN, "0xffffffff")) < 0) {
	    goto done;
	}
    }

    if (parm->patterns & MT_PAT_FIVES) {
	if ((rv = memtest_pattern(unit, parm, 0x55555555,
				  PAT_PLAIN, "0x55555555")) < 0) {
	    goto done;
	}
    }

    if (parm->patterns & MT_PAT_AS) {
	if ((rv = memtest_pattern(unit, parm, 0xaaaaaaaa,
				  PAT_PLAIN, "0xaaaaaaaa")) < 0) {
	    goto done;
	}
    }

    if (parm->patterns & MT_PAT_CHECKER) {
	/*
	 * Checkerboard alternates 5's and a's to generate a pattern shown
	 * below.  This is useful to detecting horizontally or vertically
	 * shorted data lines (in cases where the RAM layout is plain).
	 *
	 *     1 0 1 0 1 0 1 0 1 0 1
	 *     0 1 0 1 0 1 0 1 0 1 0
	 *     1 0 1 0 1 0 1 0 1 0 1
	 *     0 1 0 1 0 1 0 1 0 1 0
	 *     1 0 1 0 1 0 1 0 1 0 1
	 */
	if ((rv = memtest_pattern(unit, parm, 0x55555555,
				  PAT_XOR, "checker-board")) < 0) {
	    goto done;
	}
    }

    if (parm->patterns & MT_PAT_ICHECKER) {
	/* Same as checker-board but inverted */
	if ((rv = memtest_pattern(unit, parm, 0xaaaaaaaa,
				  PAT_XOR, "inverted checker-board")) < 0) {
	    goto done;
	}
    }

    if (parm->patterns & MT_PAT_ADDR) {
	/*
	 * Write a unique value in every location to make sure all
	 * locations are distinct (detect shorted address bits).
	 * The unique value used is the address so dumping the failed
	 * memory may indicate where the bad data came from.
	 */
	if ((rv = memtest_pattern(unit, parm, 0,
				  PAT_INCR, "linear increment")) < 0) {
	    goto done;
	}
    }

    if (parm->patterns & MT_PAT_RANDOM) {
	/*
	 * Write a unique value in every location using pseudo-random data.
	 */
	if ((rv = memtest_pattern(unit, parm, 0xd246fe4b,
				  PAT_RANDOM, "pseudo-random")) < 0) {
	    goto done;
	}
    }

    if (parm->patterns & MT_PAT_HEX) {
        /*
         * Write a value specified by the user.
         */
        int i, val = 0;
        for (i = 0; i < 4; i ++) {
            val |= (parm->hex_byte & 0xff) << i*8;
        }
        if ((rv = memtest_pattern(unit, parm, val,
                                  PAT_PLAIN, "hex_byte")) < 0) {
          goto done;
        }
    }

    if (parm->err_count > 0) {
	rv = SOC_E_FAIL;
    }

 done:

    return rv;
}

#ifdef BCM_FIREBOLT_SUPPORT
STATIC int
_soc_fb_mem_parity_control(int unit, soc_mem_t mem, int copyno, int enable)
{
    uint32 imask, oimask, misc_cfg, omisc_cfg;
    soc_field_t parity_f = PARITY_CHECK_ENf;

    COMPILER_REFERENCE(copyno);

    switch (mem) {
        case L2Xm:
        case L2_ENTRY_ONLYm:
        if (soc_feature(unit, soc_feature_l2x_parity)) {
            SOC_IF_ERROR_RETURN(READ_L2_ENTRY_PARITY_CONTROLr(unit, &imask));
            soc_reg_field_set(unit, L2_ENTRY_PARITY_CONTROLr, &imask,
                              PARITY_ENf, (enable ? 1 : 0));
            soc_reg_field_set(unit, L2_ENTRY_PARITY_CONTROLr, &imask,
                              PARITY_IRQ_ENf, (enable ? 1 : 0));
            SOC_IF_ERROR_RETURN(WRITE_L2_ENTRY_PARITY_CONTROLr(unit, imask));
        }
        return SOC_E_NONE;

        case L3_ENTRY_IPV4_MULTICASTm:
        case L3_ENTRY_IPV4_UNICASTm:
        case L3_ENTRY_IPV6_MULTICASTm:
        case L3_ENTRY_IPV6_UNICASTm:
        case L3_ENTRY_ONLYm:
        case L3_ENTRY_VALID_ONLYm:
        if (soc_feature(unit, soc_feature_l3x_parity)) {
            SOC_IF_ERROR_RETURN(READ_L3_ENTRY_PARITY_CONTROLr(unit, &imask));
            soc_reg_field_set(unit, L3_ENTRY_PARITY_CONTROLr, &imask,
                              PARITY_ENf, (enable ? 1 : 0));
            soc_reg_field_set(unit, L3_ENTRY_PARITY_CONTROLr, &imask,
                              PARITY_IRQ_ENf, (enable ? 1 : 0));
            SOC_IF_ERROR_RETURN(WRITE_L3_ENTRY_PARITY_CONTROLr(unit, imask));
        }
        return SOC_E_NONE;

        case L3_DEFIPm:
        case L3_DEFIP_DATA_ONLYm:
        if (soc_feature(unit, soc_feature_l3defip_parity)) {
            SOC_IF_ERROR_RETURN(READ_L3_DEFIP_PARITY_CONTROLr(unit, &imask));
            soc_reg_field_set(unit, L3_DEFIP_PARITY_CONTROLr, &imask,
                              PARITY_ENf, (enable ? 1 : 0));
            soc_reg_field_set(unit, L3_DEFIP_PARITY_CONTROLr, &imask,
                              PARITY_IRQ_ENf, (enable ? 1 : 0));
            SOC_IF_ERROR_RETURN(WRITE_L3_DEFIP_PARITY_CONTROLr(unit, imask));
        }
        return SOC_E_NONE;

#ifdef BCM_RAVEN_SUPPORT
        case DSCP_TABLEm:
        if (soc_feature(unit, soc_feature_ip_ep_mem_parity)) {
            SOC_IF_ERROR_RETURN(READ_DSCP_TABLE_PARITY_CONTROLr(unit, &imask));
            soc_reg_field_set(unit, DSCP_TABLE_PARITY_CONTROLr, &imask,
                              PARITY_ENf, (enable ? 1 : 0));
            soc_reg_field_set(unit, DSCP_TABLE_PARITY_CONTROLr, &imask,
                              PARITY_IRQ_ENf, (enable ? 1 : 0));
            SOC_IF_ERROR_RETURN(WRITE_DSCP_TABLE_PARITY_CONTROLr(unit, imask));
        }
        return SOC_E_NONE;
     
        case EGR_L3_INTFm:
        if (!SOC_IS_HAWKEYE(unit)) {
            if (soc_feature(unit, soc_feature_ip_ep_mem_parity)) {
                SOC_IF_ERROR_RETURN(READ_EGR_L3_INTF_PARITY_CONTROLr(unit, &imask));
                soc_reg_field_set(unit, EGR_L3_INTF_PARITY_CONTROLr, &imask,
                                  PARITY_ENf, (enable ? 1 : 0));
                soc_reg_field_set(unit, EGR_L3_INTF_PARITY_CONTROLr, &imask,
                                  PARITY_IRQ_ENf, (enable ? 1 : 0));
                SOC_IF_ERROR_RETURN(WRITE_EGR_L3_INTF_PARITY_CONTROLr(unit, imask));
            }
        }
        return SOC_E_NONE;

        case EGR_MASKm:
        if (!SOC_IS_HAWKEYE(unit)) {
            if (soc_feature(unit, soc_feature_ip_ep_mem_parity)) {
                SOC_IF_ERROR_RETURN(READ_EGR_MASK_PARITY_CONTROLr(unit, &imask));
                soc_reg_field_set(unit, EGR_MASK_PARITY_CONTROLr, &imask,
                                  PARITY_ENf, (enable ? 1 : 0));
                soc_reg_field_set(unit, EGR_MASK_PARITY_CONTROLr, &imask,
                                  PARITY_IRQ_ENf, (enable ? 1 : 0));
                SOC_IF_ERROR_RETURN(WRITE_EGR_MASK_PARITY_CONTROLr(unit, imask));
            }
        }
        return SOC_E_NONE;

        case EGR_L3_NEXT_HOPm:
        if (!SOC_IS_HAWKEYE(unit)) {
            if (soc_feature(unit, soc_feature_ip_ep_mem_parity)) {
                SOC_IF_ERROR_RETURN(READ_EGR_NEXT_HOP_PARITY_CONTROLr(unit, &imask));
                soc_reg_field_set(unit, EGR_NEXT_HOP_PARITY_CONTROLr, &imask,
                                  PARITY_ENf, (enable ? 1 : 0));
                soc_reg_field_set(unit, EGR_NEXT_HOP_PARITY_CONTROLr, &imask,
                                  PARITY_IRQ_ENf, (enable ? 1 : 0));
                SOC_IF_ERROR_RETURN(WRITE_EGR_NEXT_HOP_PARITY_CONTROLr(unit, imask));
            }
        }
        return SOC_E_NONE;

        case EGR_VLANm:
        if (soc_feature(unit, soc_feature_ip_ep_mem_parity)) {
            SOC_IF_ERROR_RETURN(READ_EGR_VLAN_TABLE_PARITY_CONTROLr(unit, &imask));
            soc_reg_field_set(unit, EGR_VLAN_TABLE_PARITY_CONTROLr, &imask,
                              PARITY_ENf, (enable ? 1 : 0));
            soc_reg_field_set(unit, EGR_VLAN_TABLE_PARITY_CONTROLr, &imask,
                              PARITY_IRQ_ENf, (enable ? 1 : 0));
            SOC_IF_ERROR_RETURN(WRITE_EGR_VLAN_TABLE_PARITY_CONTROLr(unit, imask));
        }
        return SOC_E_NONE;

        case FP_POLICY_TABLEm:
        if (soc_feature(unit, soc_feature_ip_ep_mem_parity)) {
            SOC_IF_ERROR_RETURN(READ_FP_POLICY_PARITY_CONTROLr(unit, &imask));
            soc_reg_field_set(unit, FP_POLICY_PARITY_CONTROLr, &imask,
                              PARITY_ENf, (enable ? 1 : 0));
            soc_reg_field_set(unit, FP_POLICY_PARITY_CONTROLr, &imask,
                              PARITY_IRQ_ENf, (enable ? 1 : 0));
            SOC_IF_ERROR_RETURN(WRITE_FP_POLICY_PARITY_CONTROLr(unit, imask));
        }
        return SOC_E_NONE;

        case ING_L3_NEXT_HOPm:
        if (!SOC_IS_HAWKEYE(unit)) {
            if (soc_feature(unit, soc_feature_ip_ep_mem_parity)) {
                SOC_IF_ERROR_RETURN(READ_ING_L3_NEXT_HOP_PARITY_CONTROLr(unit, &imask));
                soc_reg_field_set(unit, ING_L3_NEXT_HOP_PARITY_CONTROLr, &imask,
                                  PARITY_ENf, (enable ? 1 : 0));
                soc_reg_field_set(unit, ING_L3_NEXT_HOP_PARITY_CONTROLr, &imask,
                                  PARITY_IRQ_ENf, (enable ? 1 : 0));
                SOC_IF_ERROR_RETURN(WRITE_ING_L3_NEXT_HOP_PARITY_CONTROLr(unit, imask));
            }
        }
        return SOC_E_NONE;

        case L2MCm:
        if (soc_feature(unit, soc_feature_ip_ep_mem_parity)) {
            SOC_IF_ERROR_RETURN(READ_L2MC_PARITY_CONTROLr(unit, &imask));
            soc_reg_field_set(unit, L2MC_PARITY_CONTROLr, &imask,
                              PARITY_ENf, (enable ? 1 : 0));
            soc_reg_field_set(unit, L2MC_PARITY_CONTROLr, &imask,
                              PARITY_IRQ_ENf, (enable ? 1 : 0));
            SOC_IF_ERROR_RETURN(WRITE_L2MC_PARITY_CONTROLr(unit, imask));
        }
        return SOC_E_NONE;

        case L3_IPMCm:
        if (!SOC_IS_HAWKEYE(unit)) {
            if (soc_feature(unit, soc_feature_ip_ep_mem_parity)) {
                SOC_IF_ERROR_RETURN(READ_L3_IPMC_PARITY_CONTROLr(unit, &imask));
                soc_reg_field_set(unit, L3_IPMC_PARITY_CONTROLr, &imask,
                                  PARITY_ENf, (enable ? 1 : 0));
                soc_reg_field_set(unit, L3_IPMC_PARITY_CONTROLr, &imask,
                                  PARITY_IRQ_ENf, (enable ? 1 : 0));
                SOC_IF_ERROR_RETURN(WRITE_L3_IPMC_PARITY_CONTROLr(unit, imask));
            }
        }
        return SOC_E_NONE;

        case VLAN_TABm:
        if (soc_feature(unit, soc_feature_ip_ep_mem_parity)) {
            SOC_IF_ERROR_RETURN(READ_VLAN_PARITY_CONTROLr(unit, &imask));
            soc_reg_field_set(unit, VLAN_PARITY_CONTROLr, &imask,
                              PARITY_ENf, (enable ? 1 : 0));
            soc_reg_field_set(unit, VLAN_PARITY_CONTROLr, &imask,
                              PARITY_IRQ_ENf, (enable ? 1 : 0));
            SOC_IF_ERROR_RETURN(WRITE_VLAN_PARITY_CONTROLr(unit, imask));
        }
        return SOC_E_NONE;
#endif /* BCM_RAVEN_SUPPORT */

        default:
            break;	/* Look at MMU memories below */
    }

    SOC_IF_ERROR_RETURN(READ_MEMFAILINTMASKr(unit, &imask));
    oimask = imask;
    /* MISCCONFIGr has PARITY_CHECK_EN & CELLCRCCHECKEN control */
    SOC_IF_ERROR_RETURN(READ_MISCCONFIGr(unit, &misc_cfg));
    omisc_cfg = misc_cfg;

    switch (mem) {
        /* Covered by parity */
        case MMU_CFAPm:
            soc_reg_field_set(unit, MEMFAILINTMASKr, &imask,
                              CFAPPARITYERRORINTMASKf, (enable ? 1 : 0));
            break;

        case MMU_CCPm:
            soc_reg_field_set(unit, MEMFAILINTMASKr, &imask,
                              CCPPARITYERRORINTMASKf, (enable ? 1 : 0));
            break;

        case MMU_CBPPKTHEADER0m:
        case MMU_CBPPKTHEADER1m:
            soc_reg_field_set(unit, MEMFAILINTMASKr, &imask,
                              CBPPKTHDRPARITYERRORINTMASKf, (enable ? 1 : 0));
            break;

        case MMU_CBPCELLHEADERm:
            soc_reg_field_set(unit, MEMFAILINTMASKr, &imask,
                              CBPCELLHDRPARITYERRORINTMASKf, (enable ? 1 : 0));
            break;

        case MMU_XQ0m:
        case MMU_XQ1m:
        case MMU_XQ2m:
        case MMU_XQ3m:
        case MMU_XQ4m:
        case MMU_XQ5m:
        case MMU_XQ6m:
        case MMU_XQ7m:
        case MMU_XQ8m:
        case MMU_XQ9m:
        case MMU_XQ10m:
        case MMU_XQ11m:
        case MMU_XQ12m:
        case MMU_XQ13m:
        case MMU_XQ14m:
        case MMU_XQ15m:
        case MMU_XQ16m:
        case MMU_XQ17m:
        case MMU_XQ18m:
        case MMU_XQ19m:
        case MMU_XQ20m:
        case MMU_XQ21m:
        case MMU_XQ22m:
        case MMU_XQ23m:
        case MMU_XQ24m:
        case MMU_XQ25m:
        case MMU_XQ26m:
        case MMU_XQ27m:
        case MMU_XQ28m:
#if defined(BCM_RAPTOR_SUPPORT)
        case MMU_XQ29m:
        case MMU_XQ30m:
        case MMU_XQ31m:
        case MMU_XQ32m:
        case MMU_XQ33m:
        case MMU_XQ34m:
        case MMU_XQ35m:
        case MMU_XQ36m:
        case MMU_XQ37m:
        case MMU_XQ38m:
        case MMU_XQ39m:
        case MMU_XQ40m:
        case MMU_XQ41m:
        case MMU_XQ42m:
        case MMU_XQ43m:
        case MMU_XQ44m:
        case MMU_XQ45m:
        case MMU_XQ46m:
        case MMU_XQ47m:
        case MMU_XQ48m:
        case MMU_XQ49m:
        case MMU_XQ50m:
        case MMU_XQ51m:
        case MMU_XQ52m:
        case MMU_XQ53m:
#endif /* BCM_RAPTOR_SUPPORT */
            soc_reg_field_set(unit, MEMFAILINTMASKr, &imask,
                              XQPARITYERRORINTMASKf, (enable ? 1 : 0));
            break;

        /* Covered by CRC */
        case MMU_CBPDATA0m:
        case MMU_CBPDATA1m:
        case MMU_CBPDATA2m:
        case MMU_CBPDATA3m:
        case MMU_CBPDATA4m:
        case MMU_CBPDATA5m:
        case MMU_CBPDATA6m:
        case MMU_CBPDATA7m:
        case MMU_CBPDATA8m:
        case MMU_CBPDATA9m:
        case MMU_CBPDATA10m:
        case MMU_CBPDATA11m:
        case MMU_CBPDATA12m:
        case MMU_CBPDATA13m:
        case MMU_CBPDATA14m:
        case MMU_CBPDATA15m:
            soc_reg_field_set(unit, MEMFAILINTMASKr, &imask,
                              CRCERRORINTMASKf, (enable ? 1 : 0));
            parity_f = CELLCRCCHECKENf;
            break;
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_HAWKEYE_SUPPORT)
        case MMU_MIN_BUCKET_GPORTm:
        case MMU_MAX_BUCKET_GPORTm:
            soc_reg_field_set(unit, MISCCONFIGr, &misc_cfg,
                              METERING_CLK_ENf, (enable ? 1 : 0));
            break;
#endif /* BCM_FIREBOLT2_SUPPORT */
        default:
            return SOC_E_NONE;	/* do nothing */
    }
    if (oimask != imask) {
        SOC_IF_ERROR_RETURN(WRITE_MEMFAILINTMASKr(unit, imask));
    }
    soc_reg_field_set(unit, MISCCONFIGr, &misc_cfg,
                      parity_f, (enable ? 1 : 0));
    if (omisc_cfg != misc_cfg) {
        SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, misc_cfg));
    }
    return SOC_E_NONE;
}
#endif /* BCM_FIREBOLT_SUPPORT */

#ifdef BCM_SHADOW_SUPPORT
STATIC int
_soc_shadow_mem_parity_control(int unit, soc_mem_t mem, int copyno, int enable)
{

    COMPILER_REFERENCE(copyno);

    switch (mem) {
        case ING_VLAN_RANGEm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IPARS_DBGCTRLr, REG_PORT_ANY, 
                                        VLR0_ENABLE_ECCf, (enable ? 1 : 0))); 
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IPARS_DBGCTRLr, REG_PORT_ANY, 
                                        VLR1_ENABLE_ECCf, (enable ? 1 : 0))); 
            break;
        case SOURCE_TRUNK_MAP_TABLEm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IPARS_DBGCTRLr, REG_PORT_ANY, 
                                        STM_ENABLE_ECCf, (enable ? 1 : 0))); 
            break;
        case UDF_OFFSETm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IPARS_DBGCTRLr, REG_PORT_ANY, 
                                        UDF_ENABLE_ECCf, (enable ? 1 : 0))); 
            break;
        case VLAN_TABm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IVLAN_DBGCTRLr, REG_PORT_ANY, 
                                        VLAN_ENABLE_ECCf, (enable ? 1 : 0))); 
            break;
        case STG_TABm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IVLAN_DBGCTRLr, REG_PORT_ANY, 
                                        VLAN_STG_ENABLE_ECCf, (enable ? 1 : 0))); 
            break;
        case VLAN_SUBNETm:
        case VLAN_SUBNET_ONLYm:
        case VLAN_SUBNET_DATAm:
        case VLAN_SUBNET_DATA_ONLYm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit,  VXLT_DEBUG_CONTROL_0r, 
                                        REG_PORT_ANY, VLAN_SUBNET_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit,  VXLT_DEBUG_CONTROL_0r, 
                                        REG_PORT_ANY, VLAN_SUBNET_DATA_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        case VLAN_XLATEm:
        case VLAN_MACm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, VXLT_DEBUG_CONTROL_1r, 
                                        REG_PORT_ANY, VLAN_XLATE_MEM_0_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, VXLT_DEBUG_CONTROL_1r, 
                                        REG_PORT_ANY, VLAN_XLATE_MEM_1_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, VXLT_DEBUG_CONTROL_1r, 
                                        REG_PORT_ANY, VLAN_XLATE_MEM_2_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, VXLT_DEBUG_CONTROL_1r, 
                                        REG_PORT_ANY, VLAN_XLATE_MEM_3_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, VXLT_DEBUG_CONTROL_1r, 
                                        REG_PORT_ANY, VLAN_XLATE_MEM_4_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, VXLT_DEBUG_CONTROL_1r, 
                                        REG_PORT_ANY, VLAN_XLATE_MEM_5_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, VXLT_DEBUG_CONTROL_1r, 
                                        REG_PORT_ANY, VLAN_XLATE_MEM_6_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, VXLT_DEBUG_CONTROL_1r, 
                                        REG_PORT_ANY, VLAN_XLATE_MEM_7_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        case VLAN_PROTOCOL_DATAm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, VXLT_DEBUG_CONTROL_1r, 
                                        REG_PORT_ANY, VLAN_PROTOCOL_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        case DSCP_TABLEm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ISW1_ECC_DEBUGr, REG_PORT_ANY, 
                                        DSCP_TABLE_ENABLE_ECCf, (enable ? 1 : 0))); 
            break;
        case UFLOW_Bm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ISW1_ECC_DEBUGr, REG_PORT_ANY, 
                                        UFLOW_B_0_ENABLE_ECCf, (enable ? 1 : 0))); 
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ISW1_ECC_DEBUGr, REG_PORT_ANY, 
                                        UFLOW_B_1_ENABLE_ECCf, (enable ? 1 : 0))); 
            break;
        case UFLOW_Am:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ISW1_ECC_DEBUGr, REG_PORT_ANY, 
                                        UFLOW_A_0_ENABLE_ECCf, (enable ? 1 : 0))); 
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ISW1_ECC_DEBUGr, REG_PORT_ANY, 
                                        UFLOW_A_1_ENABLE_ECCf, (enable ? 1 : 0))); 
            break;
        case FP_TCAMm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, FP_DEBUG_CONTROLr, REG_PORT_ANY, 
                                        TCAM_ENABLE_ECCf, (enable ? 1 : 0))); 
            break;
        case FP_METER_TABLEm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, FP_DEBUG_CONTROLr, REG_PORT_ANY, 
                                        REFRESH_ENABLEf, (enable ? 1 : 0))); 
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, FP_RAM_CONTROL_1r, REG_PORT_ANY, 
                                        METER_EVEN_SLICE_0_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, FP_RAM_CONTROL_1r, REG_PORT_ANY, 
                                        METER_EVEN_SLICE_1_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, FP_RAM_CONTROL_1r, REG_PORT_ANY, 
                                        METER_EVEN_SLICE_2_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, FP_RAM_CONTROL_1r, REG_PORT_ANY, 
                                        METER_EVEN_SLICE_3_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, FP_RAM_CONTROL_1r, REG_PORT_ANY, 
                                        METER_ODD_SLICE_0_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, FP_RAM_CONTROL_1r, REG_PORT_ANY, 
                                        METER_ODD_SLICE_1_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, FP_RAM_CONTROL_1r, REG_PORT_ANY, 
                                        METER_ODD_SLICE_2_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, FP_RAM_CONTROL_1r, REG_PORT_ANY, 
                                        METER_ODD_SLICE_3_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        case FP_COUNTER_TABLEm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, FP_RAM_CONTROL_64r, REG_PORT_ANY, 
                                        COUNTER_SLICE_0_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, FP_RAM_CONTROL_64r, REG_PORT_ANY, 
                                        COUNTER_SLICE_1_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, FP_RAM_CONTROL_64r, REG_PORT_ANY, 
                                        COUNTER_SLICE_2_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, FP_RAM_CONTROL_64r, REG_PORT_ANY, 
                                        COUNTER_SLICE_3_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        case FP_STORM_CONTROL_METERSm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, FP_RAM_CONTROL_64r, REG_PORT_ANY, 
                                        STORM_ENABLE_ECCf, (enable ? 1 : 0))); 
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, FP_DEBUG_CONTROLr, REG_PORT_ANY, 
                                        REFRESH_ENABLEf, (enable ? 1 : 0))); 
            break;
        case FP_POLICY_TABLEm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, FP_RAM_CONTROL_64r, REG_PORT_ANY, 
                                        POLICY_SLICE_0_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, FP_RAM_CONTROL_64r, REG_PORT_ANY, 
                                        POLICY_SLICE_1_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, FP_RAM_CONTROL_64r, REG_PORT_ANY, 
                                        POLICY_SLICE_2_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, FP_RAM_CONTROL_64r, REG_PORT_ANY, 
                                        POLICY_SLICE_3_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        case CELL_BUFFER3m:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ISW2_DEBUG0r, REG_PORT_ANY, 
                                        CELL_BUFFER3_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case CELL_BUFFER2m:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ISW2_DEBUG0r, REG_PORT_ANY, 
                                        CELL_BUFFER2_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case CELL_BUFFER1m:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ISW2_DEBUG0r, REG_PORT_ANY, 
                                        CELL_BUFFER1_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case CELL_BUFFER0m:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ISW2_DEBUG0r, REG_PORT_ANY, 
                                        CELL_BUFFER0_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case DEBUG_CAPTUREm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ISW2_DEBUG0r, REG_PORT_ANY, 
                                        DEBUG_CAPTURE_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case VLAN_PROFILE_2m:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ISW2_DEBUG0r, REG_PORT_ANY, 
                                        VLAN_PROFILE_2_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case EGR_MASKm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ISW2_DEBUG0r, REG_PORT_ANY, 
                                        EGR_MASK_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case TRUNK_BITMAPm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ISW2_DEBUG0r, REG_PORT_ANY, 
                                        TRUNK_BITMAP_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case E2E_HOL_STATUSm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ISW2_DEBUG1r, REG_PORT_ANY, 
                                        E2E_HOL_STATUS3_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ISW2_DEBUG1r, REG_PORT_ANY, 
                                        E2E_HOL_STATUS2_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ISW2_DEBUG1r, REG_PORT_ANY, 
                                        E2E_HOL_STATUS1_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ISW2_DEBUG1r, REG_PORT_ANY, 
                                        E2E_HOL_STATUS0_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case EGR_VLANm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, EGR_EVLAN_ECC_DEBUGr, REG_PORT_ANY, 
                                        EP_VLAN_ENABLE_ECCf, (enable ? 1 : 0))); 
            break;
        case EGR_VLAN_STGm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, EGR_EVLAN_ECC_DEBUGr, 
                                        REG_PORT_ANY, EP_VLAN_STG_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case EGR_VLAN_XLATEm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, EGR_EVXLT_ECC_DEBUGr, REG_PORT_ANY, 
                                        EP_VLAN_XLATE_U_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, EGR_EVXLT_ECC_DEBUGr, REG_PORT_ANY, 
                                        EP_VLAN_XLATE_L_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case EGR_PRI_CNG_MAPm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, EGR_EVXLT_ECC_DEBUGr, REG_PORT_ANY, 
                                        EP_PRI_CNG_MAP_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case EGR_DSCP_TABLEm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, EGR_EVXLT_ECC_DEBUGr, REG_PORT_ANY, 
                                        EP_DSCP_ENABLE_ECCf, (enable ? 1 : 0))); 
            break;
        case EGR_SCI_TABLEm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, EGR_EHCPM_ECC_DEBUGr, REG_PORT_ANY, 
                                        EP_SCI_TABLE_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case EGR_PERQ_XMT_COUNTERSm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, EGR_EDB_ECC_DEBUGr, REG_PORT_ANY, 
                                        EP_PERQ_XMT_COUNTER_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case MMU_CFAP_MEMm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG0r, REG_PORT_ANY, 
                                        CFAP_ENABLE_ECCf, (enable ? 1 : 0))); 
            break;
        case MMU_PKTLINK0m:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG0r, REG_PORT_ANY, 
                                        PKTLINK_ENABLE_ECCf, (enable ? 1 : 0))); 
            break;
        case MMU_CELLLINKm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG0r, REG_PORT_ANY, 
                                        CELLLINK_ENABLE_ECCf, (enable ? 1 : 0))); 
            break;
        case MMU_CBPDATA0m:
        case MMU_CBPDATA1m:
        case MMU_CBPDATA2m:
        case MMU_CBPDATA3m:
        case MMU_CBPDATA4m:
        case MMU_CBPDATA5m:
        case MMU_CBPDATA6m:
        case MMU_CBPDATA7m:
        case MMU_CBPDATA8m:
        case MMU_CBPDATA9m:
        case MMU_CBPDATA10m:
        case MMU_CBPDATA11m:
        case MMU_CBPDATA12m:
        case MMU_CBPDATA13m:
        case MMU_CBPDATA14m:
        case MMU_CBPDATA15m:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG0r, REG_PORT_ANY, 
                                        CELLDATA_ENABLE_ECCf, (enable ? 1 : 0))); 
            break;
        case MMU_CBPPKTLENGTHm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG0r, REG_PORT_ANY, 
                                        PKTLENGTH_ENABLE_ECCf, (enable ? 1 : 0))); 
            break;
        case MMU_CBPPKTHEADER0_MEM0m:
        case MMU_CBPPKTHEADER0_MEM1m:
        case MMU_CBPPKTHEADER0_MEM2m:
        case MMU_CBPPKTHEADER0_MEM3m:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG0r, REG_PORT_ANY, 
                                        PKTHDR0_ENABLE_ECCf, (enable ? 1 : 0))); 
            break;
        case MMU_AGING_EXPm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, PKTAGINGTIMERr, REG_PORT_ANY, 
                                        DURATIONSELECTf, (enable ? 0x1000 : 0))); 
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG0r, REG_PORT_ANY, 
                                        AGING_EXP_ENABLE_ECCf, (enable ? 1 : 0))); 
            break;
        case MMU_AGING_CTRm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, PKTAGINGTIMERr, REG_PORT_ANY, 
                                        DURATIONSELECTf, (enable ? 0x1000 : 0))); 
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG0r, REG_PORT_ANY, 
                                        AGING_CTR_ENABLE_ECCf, (enable ? 1 : 0))); 
            break;
        case ES_ARB_TDM_TABLEm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG0r, REG_PORT_ANY, 
                                        ES_ARB_TDM_TABLE_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case CTR_MEMm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY, 
                                        CTR_MEM_ENABLE_ECCf, (enable ? 1 : 0))); 
            break;
        case MMU_WRED_PORT_THD_0_CELLm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY, 
                                        WRED_PORT_THD_0_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case MMU_WRED_PORT_THD_1_CELLm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY, 
                                        WRED_PORT_THD_1_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case MMU_WRED_THD_0_CELLm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY, 
                                        WRED_THD_0_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case MMU_WRED_THD_1_CELLm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY, 
                                        WRED_THD_1_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case MMU_WRED_PORT_CFG_CELLm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY, 
                                        WRED_PORT_CFG_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case MMU_WRED_CFG_CELLm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY, 
                                        WRED_CFG_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
/*        case MMU_MTRO_CPU_PKT_SHAPING_WRAPPERm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY, 
                                        MTRO_CPU_PKT_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case MMU_MTRO_SHAPE_G0_BUCKET_WRAPPERm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY, 
                                        MTRO_SHAPE_G0_BUCKET_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case MMU_MTRO_SHAPE_G0_CONFIG_WRAPPERm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY, 
                                        MTRO_SHAPE_G0_CONFIG_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case MMU_MTRO_SHAPE_G1_BUCKET_WRAPPERm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY, 
                                        MTRO_SHAPE_G1_BUCKET_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case MMU_MTRO_SHAPE_G1_CONFIG_WRAPPERm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY, 
                                        MTRO_SHAPE_G1_CONFIG_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case MMU_MTRO_SHAPE_G2_BUCKET_WRAPPERm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY, 
                                        MTRO_SHAPE_G2_BUCKET_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case MMU_MTRO_SHAPE_G2_CONFIG_WRAPPERm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY, 
                                        MTRO_SHAPE_G2_CONFIG_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case MMU_MTRO_SHAPE_G3_BUCKET_WRAPPERm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY, 
                                        MTRO_SHAPE_G3_BUCKET_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;
        case MMU_MTRO_SHAPE_G3_CONFIG_WRAPPERm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY, 
                                        MTRO_SHAPE_G3_CONFIG_ENABLE_ECCf, 
                                        (enable ? 1 : 0))); 
            break;*/
        case ESEC_PKT_HEADER_CAPTURE_BUFFERm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ESEC_ECC_DEBUGr, 1, 
                                        DEBUGFF_ENABLE_ECCf, (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ESEC_ECC_DEBUGr, 5, 
                                        DEBUGFF_ENABLE_ECCf, (enable ? 1 : 0)));
            break;
        case QL_TABLE0m:
        case QL_TABLE1m:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, MISCCONFIGr, REG_PORT_ANY, 
                                        REFRESH_ENf, (enable ? 1 : 0)));
            break;
        case ESEC_SC_TABLEm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ESEC_ECC_DEBUGr, 1, 
                                        SCDB_ENABLE_ECCf, (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ESEC_ECC_DEBUGr, 5, 
                                        SCDB_ENABLE_ECCf, (enable ? 1 : 0)));
            break;
        case ESEC_SA_TABLEm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ESEC_ECC_DEBUGr, 1, 
                                        SADB_ENABLE_ECCf, (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ESEC_ECC_DEBUGr, 5, 
                                        SADB_ENABLE_ECCf, (enable ? 1 : 0)));
            break;
        case ESEC_SA_KEY_TABLEm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ESEC_ECC_DEBUGr, 1, 
                                        SAKEY_ENABLE_ECCf, (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ESEC_ECC_DEBUGr, 5, 
                                        SAKEY_ENABLE_ECCf, (enable ? 1 : 0)));
            break;
        case OUTUNCTRLUCASTPKTSm:
        case OUTUNCTRLMCASTPKTSm:
        case OUTUNCTRLBCASTPKTSm:
        case OUTCTRLMCASTPKTSm:	
        case OUTCTRLUCASTPKTSm:
        case OUTCTRLBCASTPKTSm:		
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ESEC_ECC_DEBUGr, 1, 
                                        MIB1_ENABLE_ECCf, (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ESEC_ECC_DEBUGr, 5, 
                                        MIB1_ENABLE_ECCf, (enable ? 1 : 0)));
            break;
        case TXSAPRTCPKTSm:
        case TXSACRPTPKTSm:		
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ESEC_ECC_DEBUGr, 1, 
                                        MIB2_ENABLE_ECCf, (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ESEC_ECC_DEBUGr, 5, 
                                        MIB2_ENABLE_ECCf, (enable ? 1 : 0)));
            break;
        case TXSCPRTCPKTSm:
        case TXSCCRPTPKTSm:		
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ESEC_ECC_DEBUGr, 1, 
                                        MIB3_ENABLE_ECCf, (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ESEC_ECC_DEBUGr, 5, 
                                        MIB3_ENABLE_ECCf, (enable ? 1 : 0)));
            break;
        case TXSCPRTCBYTm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ESEC_ECC_DEBUGr, 1, 
                                        MIB4_ENABLE_ECCf, (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ESEC_ECC_DEBUGr, 5, 
                                        MIB4_ENABLE_ECCf, (enable ? 1 : 0)));
            break;
        case TXSCCRPTBYTm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ESEC_ECC_DEBUGr, 1, 
                                        MIB5_ENABLE_ECCf, (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, ESEC_ECC_DEBUGr, 5, 
                                        MIB5_ENABLE_ECCf, (enable ? 1 : 0)));
            break;
        case IL_STAT_MEM_0m:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IL_ECC_DEBUG0r, 9, 
                                        IL_RX_STAT0_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IL_ECC_DEBUG0r, 13, 
                                        IL_RX_STAT0_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IL_GLOBAL_CONFIGr, 9, 
                                        XGS_COUNTER_COMPAT_MODEf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IL_GLOBAL_CONFIGr, 13, 
                                        XGS_COUNTER_COMPAT_MODEf, 
                                        (enable ? 1 : 0)));
            break;
        case IL_STAT_MEM_1m:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IL_ECC_DEBUG0r, 9, 
                                        IL_RX_STAT1_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IL_ECC_DEBUG0r, 13, 
                                        IL_RX_STAT1_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IL_GLOBAL_CONFIGr, 9, 
                                        XGS_COUNTER_COMPAT_MODEf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IL_GLOBAL_CONFIGr, 13, 
                                        XGS_COUNTER_COMPAT_MODEf, 
                                        (enable ? 1 : 0)));
            break;
        case IL_STAT_MEM_2m:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IL_ECC_DEBUG0r, 9, 
                                        IL_RX_STAT2_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IL_ECC_DEBUG0r, 13, 
                                        IL_RX_STAT2_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IL_GLOBAL_CONFIGr, 9, 
                                        XGS_COUNTER_COMPAT_MODEf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IL_GLOBAL_CONFIGr, 13, 
                                        XGS_COUNTER_COMPAT_MODEf, 
                                        (enable ? 1 : 0)));
            break;
        case IL_STAT_MEM_3m:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IL_ECC_DEBUG0r, 9, 
                                        IL_TX_STAT0_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IL_ECC_DEBUG0r, 13, 
                                        IL_TX_STAT0_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IL_GLOBAL_CONFIGr, 9, 
                                        XGS_COUNTER_COMPAT_MODEf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IL_GLOBAL_CONFIGr, 13, 
                                        XGS_COUNTER_COMPAT_MODEf, 
                                        (enable ? 1 : 0)));
            break;
        case IL_STAT_MEM_4m:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IL_ECC_DEBUG0r, 9, 
                                        IL_TX_STAT1_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IL_ECC_DEBUG0r, 13, 
                                        IL_TX_STAT1_ENABLE_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IL_GLOBAL_CONFIGr, 9, 
                                        XGS_COUNTER_COMPAT_MODEf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, IL_GLOBAL_CONFIGr, 13, 
                                        XGS_COUNTER_COMPAT_MODEf, 
                                        (enable ? 1 : 0)));
            break;
        case ISEC_SA_TABLEm:
        case ISEC_SA_KEY_TABLEm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 1, SA1_EN_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 5, SA1_EN_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 1, SA2_EN_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 5, SA2_EN_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        case RXSAUNUSEDSAPKTSm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 1, 
                                        RXSASTATSUNUSEDSAPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 5, 
                                        RXSASTATSUNUSEDSAPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        case RXSANOTUSINGSAPKTSm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 1, 
                                        RXSASTATSNOTUSINGSAPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 5, 
                                        RXSASTATSNOTUSINGSAPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        case RXSANOTVLDPKTSm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 1, 
                                        RXSASTATSNOTVALIDPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 5, 
                                        RXSASTATSNOTVALIDPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        case RXSAINVLDPKTSm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 1, 
                                        RXSASTATSINVALIDPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 5, 
                                        RXSASTATSINVALIDPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        case RXSAOKPKTSm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 1, 
                                        RXSASTATSOKPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 5, 
                                        RXSASTATSOKPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        case INCTRLUCASTPKTSm:
        case INCTRLMCASTPKTSm:
        case INCTRLBCASTPKTSm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 1, 
                                        RXPACKETTYPE_EN_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 5, 
                                        RXPACKETTYPE_EN_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        case INCTRLDISCPKTSm:
        case INCTRLERRPKTSm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 1, 
                                        RXERRDSCRDSPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 5, 
                                        RXERRDSCRDSPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            break; 
        case RXUNTAGPKTSm:
        case RXNOTAGPKTSm:
        case RXBADTAGPKTSm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 1, 
                                        RXTAGUNTAGNONEBAD_EN_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 5, 
                                        RXTAGUNTAGNONEBAD_EN_ECCf, 
                                        (enable ? 1 : 0)));
            break;  
        case RXUNKNOWNSCIPKTSm:
        case RXNOSCIPKTSm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 1, 
                                        RXSCIUNKNOWNNONEPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 5, 
                                        RXSCIUNKNOWNNONEPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            break; 
        case RXSCUNUSEDSAPKTSm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 1,
                                        RXSCSTATSUNUSEDSAPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 5, 
                                        RXSCSTATSUNUSEDSAPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        case RXSCNOTUSINGSAPKTSm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 1, 
                                        RXSCSTATSNOTUSINGSAPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 5, 
                                        RXSCSTATSNOTUSINGSAPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        case RXSCLATEPKTSm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 1, 
                                        RXSCSTATSLATEPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 5, 
                                        RXSCSTATSLATEPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        case RXSCNOTVLDPKTSm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 1, 
                                        RXSCSTATSNOTVALIDPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 5, 
                                        RXSCSTATSNOTVALIDPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        case RXSCINVLDPKTSm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 1, 
                                        RXSCSTATSINVALIDPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 5, 
                                        RXSCSTATSINVALIDPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        case RXSCDLYPKTSm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 1, 
                                        RXSCSTATSDELAYEDPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 5, 
                                        RXSCSTATSDELAYEDPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        case RXSCUNCHKPKTSm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 1, 
                                        RXSCSTATSUNCHECKEDPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 5, 
                                        RXSCSTATSUNCHECKEDPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        case RXSCOKPKTSm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 1, 
                                        RXSCSTATSOKPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 5, 
                                        RXSCSTATSOKPKTS_EN_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        case RXSCVLDTBYTm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 1, 
                                        RXSCSTATSOCTETSVALIDATED_EN_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 5, 
                                        RXSCSTATSOCTETSVALIDATED_EN_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        case RXSCDCRPTBYTm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 1, 
                                        RXSCSTATSOCTETSDECRYPTED_EN_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 5, 
                                        RXSCSTATSOCTETSDECRYPTED_EN_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        case FILTERMATCHCOUNTm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 1, 
                                        FILTERMATCHCOUNT_EN_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 5, 
                                        FILTERMATCHCOUNT_EN_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        case ISEC_DEBUG_RAMm:
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 1, DEBUGRAM_EN_ECCf, 
                                        (enable ? 1 : 0)));
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, DEBUG20r, 5, DEBUGRAM_EN_ECCf, 
                                        (enable ? 1 : 0)));
            break;
        default:
            return SOC_E_NONE;	/* do nothing */
    }
    return SOC_E_NONE;
}
#endif

#ifdef BCM_BRADLEY_SUPPORT
STATIC int
_soc_hb_mem_parity_control(int unit, soc_mem_t mem, int copyno, int enable)
{
    uint32 rval, orval, imask, oimask,
           misc_cfg, sbs_ctrl;
    soc_reg_t mctlreg = INVALIDr, sbsreg = INVALIDr;
    soc_field_t imask_f[4] = { INVALIDf, INVALIDf, INVALIDf, INVALIDf };
    int en_val = enable ? 1 : 0;
    int i;

    COMPILER_REFERENCE(copyno);

    switch (mem) {
    case VLAN_TABm:
        if (!SOC_IS_SHADOW(unit)) {
            mctlreg = VLAN_PARITY_CONTROLr;
            imask_f[0] = TOQ0_VLAN_TBL_PAR_ERR_ENf;
            imask_f[1] = TOQ1_VLAN_TBL_PAR_ERR_ENf;
            imask_f[2] = MEM1_VLAN_TBL_PAR_ERR_ENf;
            sbsreg = SBS_CONTROLr;
        }
        break;
    case L2Xm:
    case L2_ENTRY_ONLYm:
#ifdef BCM_SCORPION_SUPPORT
    case L2_ENTRY_SCRATCHm:
#endif
        mctlreg = L2_ENTRY_PARITY_CONTROLr;
        break;
    case L2MCm:
        mctlreg = L2MC_PARITY_CONTROLr;
        sbsreg = SBS_CONTROLr;
        break;
    case L3_ENTRY_ONLYm:
    case L3_ENTRY_VALID_ONLYm:
    case L3_ENTRY_IPV4_UNICASTm:
    case L3_ENTRY_IPV4_MULTICASTm:
    case L3_ENTRY_IPV6_UNICASTm:
    case L3_ENTRY_IPV6_MULTICASTm:
        mctlreg = L3_ENTRY_PARITY_CONTROLr;
        break;
    case EGR_L3_INTFm:
        mctlreg = EGR_L3_INTF_PARITY_CONTROLr;
        sbsreg = EGR_SBS_CONTROLr;
        break;
    case EGR_L3_NEXT_HOPm:
        mctlreg = EGR_L3_NEXT_HOP_PARITY_CONTROLr;
        sbsreg = EGR_SBS_CONTROLr;
        break;
    case EGR_VLANm:
        if (!SOC_IS_SHADOW(unit)) {
            mctlreg = EGR_VLAN_PARITY_CONTROLr;
            sbsreg = EGR_SBS_CONTROLr;
        }
        break;
#ifdef BCM_SCORPION_SUPPORT
    case SOURCE_TRUNK_MAP_TABLEm:
        if (SOC_IS_SC_CQ(unit) && !SOC_IS_SHADOW(unit)) {
            mctlreg = SRC_TRUNK_PARITY_CONTROLr;
            sbsreg = SBS_CONTROLr;
        } /* No parity in Bradley */
        break;
    case VLAN_MACm:
    case VLAN_XLATEm:
        if (SOC_IS_SC_CQ(unit) && !SOC_IS_SHADOW(unit)) {
            mctlreg = VLAN_MAC_OR_XLATE_PARITY_CONTROLr;
            sbsreg = SBS_CONTROLr;
        } /* No parity in Bradley */
        break;
    case VFP_POLICY_TABLEm:
        if (SOC_IS_SC_CQ(unit)) {
            mctlreg = VFP_POLICY_TABLE_PARITY_CONTROLr;
            sbsreg = SBS_CONTROLr;
        } /* No parity in Bradley */
        break;
    case L3_DEFIPm:
    case L3_DEFIP_DATA_ONLYm:
        if (SOC_IS_SC_CQ(unit)) {
            mctlreg =  L3_DEFIP_PARITY_CONTROLr;
            sbsreg = SBS_CONTROLr;
        } /* No parity in Bradley */
        break;
    case INITIAL_ING_L3_NEXT_HOPm:
        if (SOC_IS_SC_CQ(unit)) {
            mctlreg = INITIAL_ING_L3_NEXT_HOP_PARITY_CONTROLr;
            sbsreg = SBS_CONTROLr;
        } /* No parity in Bradley */
        break;
    case FP_POLICY_TABLEm:
        if (SOC_IS_SC_CQ(unit) && !SOC_IS_SHADOW(unit)) {
            mctlreg = IFP_POLICY_TABLE_PARITY_CONTROLr;
            sbsreg = SBS_CONTROLr;
        } /* No parity in Bradley */
        break;
    case ING_L3_NEXT_HOPm:
        if (SOC_IS_SC_CQ(unit)) {
            mctlreg = ING_L3_NEXT_HOP_PARITY_CONTROLr;
            sbsreg = SBS_CONTROLr;
        } /* No parity in Bradley */
        break;
    case EGR_MASKm:
        if (SOC_IS_SC_CQ(unit) && !SOC_IS_SHADOW(unit)) {
            mctlreg = EGR_MASK_PARITY_CONTROLr;
            sbsreg = SBS_CONTROLr;
        } /* No parity in Bradley */
        break;
    case SRC_MODID_BLOCKm:
        if (SOC_IS_SC_CQ(unit)) {
            mctlreg = SRC_MODID_BLOCK_PARITY_CONTROLr;
            sbsreg = SBS_CONTROLr;
        } /* No parity in Bradley */
        break;
    case MODPORT_MAPm:
    case MODPORT_MAP_SWm:
        if (SOC_IS_SC_CQ(unit)) {
            mctlreg = MODPORT_MAP_SW_PARITY_CONTROLr;
            sbsreg = SBS_CONTROLr;
        } /* No parity in Bradley */
        break;
    case MODPORT_MAP_IMm:
        if (SOC_IS_SC_CQ(unit)) {
            mctlreg = MODPORT_MAP_IM_PARITY_CONTROLr;
            sbsreg = SBS_CONTROLr;
        } /* No parity in Bradley */
        break;
    case MODPORT_MAP_EMm:
        if (SOC_IS_SC_CQ(unit)) {
            mctlreg = MODPORT_MAP_EM_PARITY_CONTROLr;
            sbsreg = SBS_CONTROLr;
        } /* No parity in Bradley */
        break;
    case EGR_VLAN_XLATEm:
        if (SOC_IS_SC_CQ(unit) && !SOC_IS_SHADOW(unit)) {
            mctlreg = EGR_VLAN_XLATE_PARITY_CONTROLr;
            sbsreg = EGR_SBS_CONTROLr;
        }
        break;
    case EGR_IP_TUNNELm:
    case EGR_IP_TUNNEL_IPV6m:
        if (SOC_IS_SC_CQ(unit)) {
            mctlreg = EGR_IP_TUNNEL_PARITY_CONTROLr;
            sbsreg = EGR_SBS_CONTROLr;
        } /* No parity in Bradley */
        break;
#endif
    case MMU_AGING_CTRm:
        imask_f[0] = AGING_CTR_PAR_ERR_ENf;
        break;
    case MMU_AGING_EXPm:
        imask_f[0] = AGING_EXP_PAR_ERR_ENf;
        break;
    case L3_IPMCm:
#ifdef BCM_SCORPION_SUPPORT
        if (SOC_IS_SC_CQ(unit)) {
            mctlreg = L3MC_PARITY_CONTROLr;
            sbsreg = SBS_CONTROLr;
        }
        /* Fall through */
#endif
    case MMU_IPMC_VLAN_TBLm:
    case MMU_IPMC_VLAN_TBL_MEM0m:
    case MMU_IPMC_VLAN_TBL_MEM1m:
    case MMU_IPMC_GROUP_TBL0m:
    case MMU_IPMC_GROUP_TBL1m:
    case MMU_IPMC_GROUP_TBL2m:
    case MMU_IPMC_GROUP_TBL3m:
#ifdef BCM_SCORPION_SUPPORT
    case MMU_IPMC_GROUP_TBL4m:
    case MMU_IPMC_GROUP_TBL5m:
    case MMU_IPMC_GROUP_TBL6m:
#endif
        imask_f[0] = TOQ0_IPMC_TBL_PAR_ERR_ENf;
        imask_f[1] = TOQ1_IPMC_TBL_PAR_ERR_ENf;
        imask_f[2] = MEM1_IPMC_TBL_PAR_ERR_ENf;
        imask_f[3] = ENQ_IPMC_TBL_PAR_ERR_ENf;
        break;
    case MMU_CFAP_MEMm :
        imask_f[0] = CFAP_PAR_ERR_ENf;
        break;
    case MMU_CCP_MEMm :
        imask_f[0] = CCP_PAR_ERR_ENf;
        break;
    case MMU_CELLLINKm:
        imask_f[0] = TOQ0_CELLLINK_PAR_ERR_ENf;
        imask_f[1] = TOQ1_CELLLINK_PAR_ERR_ENf;
        break;
    case MMU_PKTLINK0m:
    case MMU_PKTLINK1m:
    case MMU_PKTLINK2m:
    case MMU_PKTLINK3m:
    case MMU_PKTLINK4m:
    case MMU_PKTLINK5m:
    case MMU_PKTLINK6m:
    case MMU_PKTLINK7m:
    case MMU_PKTLINK8m:
    case MMU_PKTLINK9m:
    case MMU_PKTLINK10m:
    case MMU_PKTLINK11m:
    case MMU_PKTLINK12m:
    case MMU_PKTLINK13m:
    case MMU_PKTLINK14m:
    case MMU_PKTLINK15m:
    case MMU_PKTLINK16m:
    case MMU_PKTLINK17m:
    case MMU_PKTLINK18m:
    case MMU_PKTLINK19m:
    case MMU_PKTLINK20m:
#ifdef BCM_TRX_SUPPORT
    case MMU_PKTLINK21m:
    case MMU_PKTLINK22m:
    case MMU_PKTLINK23m:
    case MMU_PKTLINK24m:
    case MMU_PKTLINK25m:
    case MMU_PKTLINK26m:
    case MMU_PKTLINK27m:
    case MMU_PKTLINK28m:
#endif
        imask_f[0] = TOQ0_PKTLINK_PAR_ERR_ENf;
        imask_f[1] = TOQ1_PKTLINK_PAR_ERR_ENf;
        break;
    case MMU_CBPPKTHEADER0_MEM0m:
    case MMU_CBPPKTHEADER0_MEM1m:
    case MMU_CBPPKTHEADER0_MEM2m:
    case MMU_CBPPKTHEADER1_MEM0m:
    case MMU_CBPPKTHEADER1_MEM1m:
    case MMU_CBPPKTHEADERCPUm:
#ifdef BCM_SCORPION_SUPPORT
    case MMU_CBPPKTHEADER0_MEM3m:
#endif
        imask_f[0] = TOQ0_PKTHDR1_PAR_ERR_ENf;
        imask_f[1] = TOQ1_PKTHDR1_PAR_ERR_ENf;
        break;
    case MMU_CBPCELLHEADERm:
        imask_f[0] = TOQ0_CELLHDR_PAR_ERR_ENf;
        imask_f[1] = TOQ1_CELLHDR_PAR_ERR_ENf;
        break;
    case MMU_CBPDATA0m:
    case MMU_CBPDATA1m:
    case MMU_CBPDATA2m:
    case MMU_CBPDATA3m:
    case MMU_CBPDATA4m:
    case MMU_CBPDATA5m:
    case MMU_CBPDATA6m:
    case MMU_CBPDATA7m:
    case MMU_CBPDATA8m:
    case MMU_CBPDATA9m:
    case MMU_CBPDATA10m:
    case MMU_CBPDATA11m:
    case MMU_CBPDATA12m:
    case MMU_CBPDATA13m:
    case MMU_CBPDATA14m:
    case MMU_CBPDATA15m:
        /* Covered by CRC */
        imask_f[0] = DEQ0_CELLCRC_ERR_ENf;
        imask_f[1] = DEQ1_CELLCRC_ERR_ENf;
        break;
    case MMU_CBPPKTLENGTHm:
        /* Covered by CRC */
        imask_f[0] = DEQ0_LENGTH_PAR_ERR_ENf;
        imask_f[1] = DEQ1_LENGTH_PAR_ERR_ENf;
        break;

    case MMU_WRED_CFG_CELLm:
    case MMU_WRED_PORT_CFG_CELLm:
    case MMU_WRED_PORT_THD_0_CELLm:
    case MMU_WRED_PORT_THD_1_CELLm:
    case MMU_WRED_THD_0_CELLm:
    case MMU_WRED_THD_1_CELLm:
        imask_f[0] = WRED_PAR_ERR_ENf;
        break;
    default:
        /* Do nothing */
        return SOC_E_NONE;
    }

    /*
     * Turn off parity
     */
    if (mctlreg != INVALIDr) {
        /* Memory has a separate parity control */
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, mctlreg, REG_PORT_ANY, 0, &rval));
        orval = rval;
        soc_reg_field_set(unit, mctlreg, &rval, PARITY_ENf, en_val);
        soc_reg_field_set(unit, mctlreg, &rval, PARITY_IRQ_ENf, en_val);
        if (orval != rval) {
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, mctlreg, REG_PORT_ANY, 0, rval));
        }

        if ((sbsreg != INVALIDr) && (!SOC_IS_SHADOW(unit))) { /* Dual pipe controls needed */
            sbs_ctrl = 0;
            soc_reg_field_set(unit, sbsreg, &sbs_ctrl, PIPE_SELECTf,
                              SOC_PIPE_SELECT_Y);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, sbsreg, REG_PORT_ANY, 0, sbs_ctrl));
            
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, mctlreg, REG_PORT_ANY, 0, &rval));
            orval = rval;
            soc_reg_field_set(unit, mctlreg, &rval, PARITY_ENf, en_val);
            soc_reg_field_set(unit, mctlreg, &rval, PARITY_IRQ_ENf, en_val);
            if (orval != rval) {
                SOC_IF_ERROR_RETURN(soc_reg32_set(unit, mctlreg, REG_PORT_ANY, 0, rval));
            }

            soc_reg_field_set(unit, sbsreg, &sbs_ctrl, PIPE_SELECTf,
                              SOC_PIPE_SELECT_X);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, sbsreg, REG_PORT_ANY, 0, sbs_ctrl));
        }
    } else {
        /* Memory does not have a separate parity control */
        SOC_IF_ERROR_RETURN(READ_MISCCONFIGr(unit, &misc_cfg));
        soc_reg_field_set(unit, MISCCONFIGr, &misc_cfg, PARITY_STAT_CLEARf, 1);
        SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, misc_cfg));

        soc_reg_field_set(unit, MISCCONFIGr, &misc_cfg, PARITY_STAT_CLEARf, 0);
        if (soc_reg_field_valid(unit, MISCCONFIGr, PARITY_GEN_ENf)) {
            soc_reg_field_set(unit, MISCCONFIGr, &misc_cfg, PARITY_GEN_ENf, en_val);
            soc_reg_field_set(unit, MISCCONFIGr, &misc_cfg, PARITY_CHK_ENf, en_val);
        }
        soc_reg_field_set(unit, MISCCONFIGr, &misc_cfg, REFRESH_ENf, en_val);
        SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, misc_cfg));
    }

    /*
     * Disable parity and CRC interrupts
     */
    if ((imask_f[0] != INVALIDr) && SOC_REG_IS_VALID(unit, MEM_FAIL_INT_ENr)) {
        SOC_IF_ERROR_RETURN(READ_MEM_FAIL_INT_ENr(unit, &imask));
        oimask = imask;
        for (i = 0; i < COUNTOF(imask_f); i++) {
            if (imask_f[i] != INVALIDr) {
                soc_reg_field_set(unit, MEM_FAIL_INT_ENr, &imask, 
                                  imask_f[i], en_val);
            }
        }
        if (oimask != imask) {
            SOC_IF_ERROR_RETURN(WRITE_MEM_FAIL_INT_ENr(unit, imask));
        }
    }

    return SOC_E_NONE;
}
#endif /* BCM_BRADLEY_SUPPORT */

/*
 * Toggle parity checks during memory tests
 */
int
soc_mem_parity_control(int unit, soc_mem_t mem, int copyno, int enable)
{

    soc_cm_debug(DK_VERBOSE,
		 "soc_mem_parity_control: unit %d memory %s.%s %sable\n",
		 unit,  SOC_MEM_UFNAME(unit, mem),
		 SOC_BLOCK_NAME(unit, copyno),
		 enable ? "en" : "dis");
#ifdef BCM_SHADOW_SUPPORT
    if (SOC_IS_SHADOW(unit)) {
        return _soc_shadow_mem_parity_control(unit, mem, copyno, enable);
    }
#endif /* BCM_SHADOW_SUPPORT */
#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        return _soc_trident_mem_parity_control(unit, mem, copyno, enable);
    }
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit)) {
        return _soc_triumph2_mem_parity_control(unit, mem, copyno, enable);
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

#ifdef BCM_ENDURO_SUPPORT
    if (SOC_IS_ENDURO(unit)) {
        return _soc_enduro_mem_parity_control(unit, mem, copyno, enable);
    }
#endif /* BCM_ENDURO_SUPPORT */

#ifdef BCM_HURRICANE_SUPPORT
    if (SOC_IS_HURRICANE(unit)) {
        return _soc_hurricane_mem_parity_control(unit, mem, copyno, enable);
    }
#endif /* BCM_ENDURO_SUPPORT */

#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FB_FX_HX(unit)) {
        return _soc_fb_mem_parity_control(unit, mem, copyno, enable);
    }
#endif /* BCM_FIREBOLT_SUPPORT */

#ifdef BCM_BRADLEY_SUPPORT
    if (SOC_IS_HBX(unit)) {
        return _soc_hb_mem_parity_control(unit, mem, copyno, enable);
    }
#endif /* BCM_BRADLEY_SUPPORT */

#ifdef	BCM_HERCULES_SUPPORT
    if (SOC_IS_HERCULES1(unit)) {
	int copy, port;
        uint32 interrupt, regval;

        /* Shut off parity interrupts during memory test */
	switch (mem) {
        case MEM_XQm:
            interrupt = SOC_ERR_HAND_MMU_XQ_PARITY;
	    break;
        case MEM_LLAm:
            interrupt = SOC_ERR_HAND_MMU_LLA_PARITY;
	    break;
	default:
	    return SOC_E_NONE;	/* nothing to do */
        }

	SOC_MEM_BLOCK_ITER(unit, mem, copy) {
	    if (copyno != COPYNO_ALL && copyno != copy) {
		continue;
	    }

	    /* find the ports matching this copy/blk */
	    PBMP_ITER(SOC_BLOCK_BITMAP(unit, copy), port) {
		SOC_IF_ERROR_RETURN(READ_MMU_INTCNTLr(unit, port, &regval));
		if (enable) {
		    regval |= interrupt;
		} else {
		    regval &= ~interrupt;
		}
		if (enable) {
		    SOC_IF_ERROR_RETURN
			(WRITE_MMU_INTCLRr(unit, port, interrupt));
		}
		SOC_IF_ERROR_RETURN(WRITE_MMU_INTCNTLr(unit, port, regval));
	    }
        }
    }
#endif	/* BCM_HERCULES_SUPPORT */
#ifdef	BCM_HERCULES15_SUPPORT
    if (SOC_IS_HERCULES15(unit)) {
        int     copy;
        int     port;
        uint32  interrupt = 0;
        uint32  regval;

        /* Shut off parity interrupts during memory test */
        switch (mem) {
            case MEM_XQm:
                soc_reg_field_set
                (unit, MMU_INTCTRLr, &interrupt, XQ_PE_ENf, 1);
                break;
            case MEM_PPm:
                soc_reg_field_set
                (unit, MMU_INTCTRLr, &interrupt, PP_DBE_ENf, 1);
                soc_reg_field_set
                (unit, MMU_INTCTRLr, &interrupt, PP_SBE_ENf, 1);
                break;
            case MEM_LLAm:
                soc_reg_field_set
                (unit, MMU_INTCTRLr, &interrupt, LLA_PE_ENf, 1);
                break;
            case MEM_INGBUFm:
            case MEM_MCm:
            case MEM_IPMCm:
            case MEM_UCm:
            case MEM_VIDm:
            case MEM_ING_SRCMODBLKm:
            case MEM_ING_MODMAPm:
            case MEM_EGR_MODMAPm:
                soc_reg_field_set
                (unit, MMU_INTCTRLr, &interrupt, ING_PE_ENf, 1);
                break;
                break;
            default:
                return SOC_E_NONE;	/* nothing to do */
        }
        SOC_MEM_BLOCK_ITER(unit, mem, copy) {
            if (copyno != COPYNO_ALL && copyno != copy) {
                continue;
            }

            /* find the ports matching this copy/blk */
            PBMP_ITER(SOC_BLOCK_BITMAP(unit, copy), port) {
                SOC_IF_ERROR_RETURN
                (READ_MMU_INTCTRLr(unit, port, &regval));
                if (enable) {
                    regval |= interrupt;
                } else {
                    regval &= ~interrupt;
                }
                if (enable) {
                    SOC_IF_ERROR_RETURN
                    (WRITE_MMU_INTCLRr(unit, port, interrupt));
                }
                SOC_IF_ERROR_RETURN
                (WRITE_MMU_INTCTRLr(unit, port, (port == 0) ? 0:regval));
                SOC_IF_ERROR_RETURN(READ_MMU_CFGr(unit, port, &regval));
                soc_reg_field_set(unit, MMU_CFGr, &regval,
                                PARITY_DIAGf, enable ? 0 : 1);
                SOC_IF_ERROR_RETURN(WRITE_MMU_CFGr(unit, port, regval));
            }
        }
    }
#endif	/* BCM_HERCULES15_SUPPORT */

#ifdef	BCM_LYNX_SUPPORT
    if (SOC_IS_LYNX(unit)) {
        uint32 regval;

        switch (mem) {
        case L2X_BASEm:
        case L2X_VALIDm:
        case L2X_STATICm:
        case L3X_BASEm:
        case L3X_VALIDm:
        case VLAN_TABm:
        case STG_TABm:
        case EGR_MASKm:
        case L2X_MCm:
        case DEFIPm:
        case L3_INTFm:
        case L3_IPMCm:
            /* 
             * These next two do not have parity, but due to a HW issue,
             * they will trigger parity errors in the base memory if they
             * are present.
             */
        case L2X_HITm:
        case L3X_HITm:
            /* ARL memories */
            SOC_IF_ERROR_RETURN
		(soc_reg_read_any_block(unit, ARL_CONTROLr, &regval));
            soc_reg_field_set(unit, ARL_CONTROLr, &regval,
			      PARITY_DIAGNOSIS_ENf, enable ? 0 : 1);
            SOC_IF_ERROR_RETURN
		(soc_reg_write_all_blocks(unit, ARL_CONTROLr, regval));
            break;
            
        case XFILTER_METERINGm:
        case XFILTER_METERING_TEST0m:
        case XFILTER_METERING_TEST1m:
        case GFILTER_IRULEm:
        case GFILTER_IRULE_TEST0m:
        case GFILTER_IRULE_TEST1m:
        case VPLSTABLEm:
        case CELL_BUFm:
        case EGRESS_IPMC_MSm:
        case EGRESS_IPMC_LSm:
        case EGRESS_SPVLAN_IDm:
        case EGRESS_VLAN_STGm:
            /* XPIC memories */
            SOC_IF_ERROR_RETURN
		(soc_reg_write_all_blocks(unit, XPC_PARITY_DIAGr,
					  enable ? 0 : 1));
            break;

        case MMU_MEMORIES2_XQ0m:
        case MMU_MEMORIES2_XQ1m:
        case MMU_MEMORIES2_XQ2m:
        case MMU_MEMORIES1_CFAPm:
        case MMU_MEMORIES1_CCPm:
            /* MMU memories */
            SOC_IF_ERROR_RETURN
		(soc_reg_write_all_blocks(unit, MMUECCOVERRIDEr,
					  enable ? 0 : 1));
            break;

        default:
            break;
        }
    }
#endif	/* BCM_LYNX_SUPPORT */

#ifdef	BCM_TUCANA_SUPPORT
    if (SOC_IS_TUCANA(unit)) {
	int blk;
        uint32 regval;

        blk = SOC_MEM_BLOCK_ANY(unit, mem);
        if (blk == MMU_BLOCK(unit)) {
            if (enable && (mem == MMU_CCPm)) {
                SOC_CONTROL(unit)->mmu_error_block = TRUE;
            }
            SOC_IF_ERROR_RETURN(READ_MISCCONFIGr(unit, &regval));
            soc_reg_field_set(unit, MISCCONFIGr, &regval, DISABLEPARITYMSGSf,
                              (enable ? 0 : 1));
            SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, regval));
            if (enable && (mem == MMU_CCPm)) {
                sal_sleep(1);
                SOC_CONTROL(unit)->mmu_error_block = FALSE;
            }
        }
        if (blk == ARL_BLOCK(unit)) {
            SOC_IF_ERROR_RETURN(READ_ARL_CONTROLr(unit, &regval));
            soc_reg_field_set(unit, ARL_CONTROLr, &regval, PARITY_ENf,
                              (enable ? 1 : 0));
            SOC_IF_ERROR_RETURN(WRITE_ARL_CONTROLr(unit, regval));
        }
    }

    /* TO DO:  Possibly enable/disable interrupts:
     *     ARL_<arl-mem>_PARITY_ERROR in CMIC_IRQ_MASK
    */
#endif	/* BCM_TUCANA_SUPPORT */

#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
	int blk;
        uint32 regval;

        blk = SOC_MEM_BLOCK_ANY(unit, mem);
        if ((blk == HSE_BLOCK(unit)) || (blk == CSE_BLOCK(unit)) ||
            (blk == BSE_BLOCK(unit))) {
            SOC_IF_ERROR_RETURN(READ_SEER_CONFIGr(unit, &regval));
            soc_reg_field_set(unit, SEER_CONFIGr, &regval,
                              PARITY_DIAG_ENABLEf, (enable ? 0 : 1));
            SOC_IF_ERROR_RETURN(WRITE_SEER_CONFIGr(unit, regval));

            /* Special mem process switches */
            switch (mem) {
            case L2_ENTRY_EXTERNALm:
            case L2_ENTRY_INTERNALm:
            case L2_ENTRY_OVERFLOWm:
                SOC_IF_ERROR_RETURN(READ_L2_ISr(unit, &regval));
                soc_reg_field_set(unit, L2_ISr, &regval, AGE_ENf, 0);
                soc_reg_field_set(unit, L2_ISr, &regval,
                                  PRG_ENf, (enable ? 1 : 0));
                SOC_IF_ERROR_RETURN(WRITE_L2_ISr(unit, regval));
                /* Wait for purge task to complete on external (worst-case) */
                sal_usleep(200000);
                break;
            case L2_MOD_FIFOm:
                SOC_IF_ERROR_RETURN(READ_L2_ISr(unit, &regval));
                soc_reg_field_set(unit, L2_ISr, &regval,
                                  L2_MOD_FIFO_LOCKf, (enable ? 0 : 1));
                SOC_IF_ERROR_RETURN(WRITE_L2_ISr(unit, regval));
                break;
            case FP_METER_TABLE_INTm:
            case FP_METER_TABLE_EXTm:
                SOC_IF_ERROR_RETURN(READ_METER_CTLr(unit, &regval));
                soc_reg_field_set(unit, METER_CTLr, &regval,
                                  METER_BG_DISf, (enable ? 0 : 1));
                SOC_IF_ERROR_RETURN(WRITE_METER_CTLr(unit, regval));
                break;
            case IPMC_GROUP_V4m:
                regval = 0;
                soc_reg_field_set(unit, IPMC_ENTRY_VLDr, &regval,
                                  IPMC_ENTRY_VLD_BITSf,
                                  SOC_ER_IPMC_GROUP_TBL_VALID_MASK);
                SOC_IF_ERROR_RETURN(WRITE_IPMC_ENTRY_VLDr(unit, regval));
                SOC_IF_ERROR_RETURN(WRITE_IPMC_ENTRY_V6r(unit, 0));
                SOC_IF_ERROR_RETURN(WRITE_IPMC_ENTRY_V4_AVAILr(unit, 0));
                regval = 0;
                soc_reg_field_set(unit, IPMC_ENTRY_V4_BLKCNTr, &regval,
                                  IPMC_ENTRY_V4_BLKCNT_BITSf,
                                  SOC_ER_IPMC_GROUP_TBL_BLK_NUM);
                SOC_IF_ERROR_RETURN(WRITE_IPMC_ENTRY_V4_BLKCNTr(unit, regval));
                SOC_IF_ERROR_RETURN(WRITE_IPMC_V4_MAPPING_0r(unit,
                                    SOC_ER_IPMC_GROUP_V4_TEST_MAPPING));
                break;
            case IPMC_GROUP_V6m:
                regval = 0;
                soc_reg_field_set(unit, IPMC_ENTRY_VLDr, &regval,
                                  IPMC_ENTRY_VLD_BITSf,
                                  SOC_ER_IPMC_GROUP_TBL_VALID_MASK);
                SOC_IF_ERROR_RETURN(WRITE_IPMC_ENTRY_VLDr(unit, regval));
                SOC_IF_ERROR_RETURN(WRITE_IPMC_ENTRY_V6r(unit, regval));
                SOC_IF_ERROR_RETURN(WRITE_IPMC_ENTRY_V6_AVAILr(unit, 0));
                regval = 0;
                soc_reg_field_set(unit, IPMC_ENTRY_V6_BLKCNTr, &regval,
                                  IPMC_ENTRY_V6_BLKCNT_BITSf,
                                  SOC_ER_IPMC_GROUP_TBL_BLK_NUM);
                SOC_IF_ERROR_RETURN(WRITE_IPMC_ENTRY_V6_BLKCNTr(unit, regval));
                SOC_IF_ERROR_RETURN(WRITE_IPMC_V6_MAPPING_0r(unit,
                                    SOC_ER_IPMC_GROUP_V6_TEST_MAPPING));
                break;
            default:
                /* No special processing */
                break;
            }
        } else if (blk == EGR_BLOCK(unit)) {
            SOC_IF_ERROR_RETURN(READ_EGR_CONFIG3r(unit, &regval));
            soc_reg_field_set(unit, EGR_CONFIG3r, &regval,
                              PARITY_DIAG_ENABLEf, (enable ? 0 : 1));
            SOC_IF_ERROR_RETURN(WRITE_EGR_CONFIG3r(unit, regval));
        } else if (blk == MMU_BLOCK(unit)) {
            SOC_IF_ERROR_RETURN(READ_MISCCONFIGr(unit, &regval));
            soc_reg_field_set(unit, MISCCONFIGr, &regval,
                              PARITY_BIT_WRITE_ACCESSf, (enable ? 0 : 1));
            SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, regval));
        }

        if (enable) {
            /* Flush the pending errors */
            SOC_IF_ERROR_RETURN(READ_PAR_ERR_STATUS_BSEr(unit, &regval));
            SOC_IF_ERROR_RETURN(READ_PAR_ERR_STATUS_CSEr(unit, &regval));
            SOC_IF_ERROR_RETURN(READ_PAR_ERR_STATUS_HSEr(unit, &regval));
            SOC_IF_ERROR_RETURN(WRITE_MMU_ERR_VECTORr(unit, 0));

            if (soc_property_get(unit, spn_PARITY_ENABLE, 1)) {
                soc_intr_enable(unit, IRQ_MEM_FAIL);
            }
        } else {
            soc_intr_disable(unit, IRQ_MEM_FAIL);
        }
    }
#endif /* BCM_EASYRIDER_SUPPORT */

    return SOC_E_NONE;
}

/*
 * Clean up parity bits after memtest to avoid later parity errors
 */
int
soc_mem_parity_clean(int unit, soc_mem_t mem, int copyno)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(mem);
    COMPILER_REFERENCE(copyno);
    /* To avoid later parity errors */
#ifdef	BCM_LYNX_SUPPORT
    if (SOC_IS_LYNX(unit)) {
        switch (mem) {
        case L2X_BASEm:
        case L2X_VALIDm:
        case L2X_STATICm:
        case L2X_HITm:
            if (soc_mem_clear(unit, mem, copyno, TRUE) < 0) {
                return -1;
            }
            break;
        default:
            /* Do nothing */
            break;
        }
    }
#endif	/* BCM_LYNX_SUPPORT */

#if defined(BCM_FIREBOLT_SUPPORT)
    switch (mem) {
        case L2Xm:
        case L2_ENTRY_ONLYm:
        if (soc_feature(unit, soc_feature_l2x_parity)) {
            if (soc_mem_clear(unit, mem, copyno, TRUE) < 0) {
                return -1;
            }
        }
        break;

        case L3_ENTRY_IPV4_MULTICASTm:
        case L3_ENTRY_IPV4_UNICASTm:
        case L3_ENTRY_IPV6_MULTICASTm:
        case L3_ENTRY_IPV6_UNICASTm:
        case L3_ENTRY_ONLYm:
        case L3_ENTRY_VALID_ONLYm:
        if (soc_feature(unit, soc_feature_l3x_parity)) {
            if (soc_mem_clear(unit, mem, copyno, TRUE) < 0) {
                return -1;
            }
        }
        break;

        case L3_DEFIPm:
        case L3_DEFIP_DATA_ONLYm:
        if (soc_feature(unit, soc_feature_l3defip_parity)) {
            if (soc_mem_clear(unit, mem, copyno, TRUE) < 0) {
                return -1;
            }
        }
        break;

#ifdef BCM_RAPTOR_SUPPORT
        case DSCP_TABLEm:
        case EGR_L3_INTFm:
        case EGR_MASKm:
        case EGR_L3_NEXT_HOPm:
        case EGR_VLANm:
        case FP_POLICY_TABLEm:
        case ING_L3_NEXT_HOPm:
        case L2MCm:
        case L3_IPMCm:
        case VLAN_TABm:
        if (soc_feature(unit, soc_feature_ip_ep_mem_parity)) {
            if (soc_mem_clear(unit, mem, copyno, TRUE) < 0) {
                return -1;
            }
        }
        break;
#endif /* BCM_RAPTOR_SUPPORT */

#ifdef BCM_TRX_SUPPORT
        case MMU_WRED_CFG_CELLm:
        case MMU_WRED_THD_0_CELLm:
        case MMU_WRED_THD_1_CELLm:
        case MMU_WRED_PORT_CFG_CELLm:
        case MMU_WRED_PORT_THD_0_CELLm:
        case MMU_WRED_PORT_THD_1_CELLm:
#ifdef BCM_TRIUMPH_SUPPORT
        case MMU_WRED_CFG_PACKETm:
        case MMU_WRED_THD_0_PACKETm:
        case MMU_WRED_THD_1_PACKETm:
        case MMU_WRED_PORT_CFG_PACKETm:
        case MMU_WRED_PORT_THD_0_PACKETm:
        case MMU_WRED_PORT_THD_1_PACKETm:
#endif /* BCM_TRIUMPH_SUPPORT */
#ifdef BCM_ENDURO_SUPPORT
        case LMEPm:
#endif /* BCM_ENDURO_SUPPORT */
            if (soc_mem_clear(unit, mem, copyno, TRUE) < 0) {
                return -1;
            }
            break;
#endif /* BCM_TRX_SUPPORT */

        default:
            break; /* Do nothing */
    }
#endif

#ifdef	BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        uint32   read_data[SOC_MAX_MEM_WORDS];
        void *null_entry = soc_mem_entry_null(unit, mem);

        switch (mem) {
        case L3_ENTRY_V4m:
        case L3_ENTRY_V6m:
        case L2_ENTRY_INTERNALm:
            if (SAL_BOOT_QUICKTURN) {
                break;
            }
        case IPMC_GROUP_V4m:
        case IPMC_GROUP_V6m:
        case L2_ENTRY_EXTERNALm:
        case MMU_INGPKTCELLUSEm:
            if (soc_mem_clear(unit, mem, copyno, TRUE) < 0) {
                return -1;
            }
            break;
        case FP_EXTERNALm:
        case FP_TCAM_EXTERNALm:
        case L3_DEFIP_CAMm:
        case L3_DEFIP_TCAMm:
            if (SOC_PERSIST(unit)->er_memcfg.tcam_select ==
                ER_EXT_TCAM_TYPE1) {
                /* Use 0's for type1 TCAM */
                null_entry = soc_mem_entry_zeroes(unit, mem);
            }
            /* Fall through */
        case FP_POLICY_EXTERNALm:
        case L3_DEFIP_DATAm:
            /* Write 0's and read to clear data buffer */
            if (soc_mem_write(unit, mem, MEM_BLOCK_ALL, 0,
                              null_entry) < 0) {
                return -1;
            }
            if (soc_mem_read(unit, mem, MEM_BLOCK_ANY, 0, read_data) < 0) {
                return -1;
            }
        default:
            /* Do nothing */
            break;
        }
    }
#endif	/* BCM_EASYRIDER_SUPPORT */
     return SOC_E_NONE;
}

/* Reenable parity after testing */
int
soc_mem_parity_restore(int unit, soc_mem_t mem, int copyno)
{
    /* 
     * Some devices need memories cleared before reenabling because
     * of background HW processes which scan the tables and throw errors
     */
    if (SOC_IS_EASYRIDER(unit) || SOC_IS_TRX(unit)) {
        SOC_IF_ERROR_RETURN
            (soc_mem_parity_clean(unit, mem, copyno));
    }

    SOC_IF_ERROR_RETURN
        (soc_mem_parity_control(unit, mem, copyno, TRUE));

    /* 
     * Other devices need memories cleared after parity is reenabled
     * so the HW generation of parity bits is correct.
     */

    if (!(SOC_IS_EASYRIDER(unit) || SOC_IS_TRX(unit))) {
        SOC_IF_ERROR_RETURN
            (soc_mem_parity_clean(unit, mem, copyno));
    }

    return 0;
}
