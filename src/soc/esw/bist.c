/*
 * $Id: bist.c 1.4 Broadcom SDK $
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
 * CBP, ARL, L3, and FFP Memory Built-In Self-Test
 *
 * Note: Once these routines are called, they can't be called again
 * until the chip is completely reset (via soc_init()).  The chip should
 * always be reset after running BIST to return memory and register
 * contents to normal state before trying to use the chip.
 */

#include <sal/core/libc.h>
#include <sal/core/time.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/cmic.h>
#include <soc/error.h>
#include <soc/drv.h>
#include <soc/enet.h>

#define BIST_POLL_MSEC		10

#define CHK_OP(op)	if ((rv = (op)) < 0) goto op_fail

STATIC soc_mem_t _soc_bist_mems[NUM_SOC_MEM];

#ifdef BCM_DRACO_SUPPORT

/*
 * Function:
 *	soc_bist_bcm5690 (internal)
 * Purpose:
 *	Runs BIST on specified list of memories on the chip.
 * Parameters:
 *	unit - StrataSwitch unit #
 *	mems - mems on which to run BIST
 *	num_mems - Number of memories in the list
 *	timeout_msec - Timemout
 * Returns:
 *	SOC_E_XXX
 * Notes:
 *	Chip should be reset (soc_reset) both before and after BIST.
 */
STATIC int
soc_bist_bcm5690(int unit, soc_mem_t *mems, int num_mems, int timeout_msec)
{
    int		rv;
    int         memno, bit, done, fail = 0;
    soc_mem_t   mem;
    soc_mem_t   ffp_mem=0;
    uint32      mmu_bist = 0, arl_bist = 0, ffp_bist = 0;
    uint32      mmu_bist_done = 0, arl_bist_done = 0, ffp_bist_done = 0;
    uint32      mmu_bist_go = 0, arl_bist_go = 0, ffp_bist_go = 0;
    uint32      val;
    int         blk, port, msec;
    uint64      val64;

    for (memno = 0; memno < num_mems; memno++) {
        mem = mems[memno];
        soc_cm_debug(DK_SOCMEM,
		     "bist_5690: memory %s running (5690 style)\n",
		     SOC_MEM_UFNAME(unit, mem));
        if (!SOC_MEM_IS_VALID(unit, mem)) {
            soc_cm_debug(DK_VERBOSE,
			 "Warning: Memory %s not valid for chip %s\n",
			 SOC_MEM_UFNAME(unit, mem), SOC_UNIT_NAME(unit));
        } else {
            bit = soc_mem_bist_bit(unit, mem);

            if (soc_mem_is_bistepic(unit, mem)) {
                arl_bist |= (1 << bit);
                arl_bist_done |= (1 << ((bit - 7) << 1)); /* MAGIC */
            } else if (soc_mem_is_bistcbp(unit, mem)) {
                mmu_bist |= (1 << bit);
                mmu_bist_done |= (1 << ((bit - 1) << 1)); /* MAGIC */
            } else if (soc_mem_is_bistffp(unit, mem)) {
                ffp_bist |= (1 << bit);
                ffp_bist_done |= (1 << (bit + 1)); /* MAGIC */
                ffp_mem = mem;
            } else {
                soc_cm_debug(DK_ERR,
			     "soc_bist: cannot bist %s\n",
			     SOC_MEM_UFNAME(unit, mem));
                return SOC_E_UNAVAIL;
            }
        }
    }

    if (arl_bist != 0) {
        CHK_OP(READ_ARL_CONTROLr(unit, &val));

        if ((val & arl_bist) != 0) {
        already_ran:
            soc_cm_debug(DK_ERR,
			 "bist_5690: bist can only run once per "
			 "memory per chip reset\n");
            return SOC_E_INTERNAL;
        }

        val |= arl_bist;
        soc_reg_field_set(unit, ARL_CONTROLr, &val, CPU_BIST_ENf, 1);
        SOC_IF_ERROR_RETURN(WRITE_ARL_CONTROLr(unit, val));
        arl_bist_go = (arl_bist_done << 1);
    }

    if (mmu_bist != 0) {
        CHK_OP(READ_MMUMBISTENr(unit, &val));

        if ((val & mmu_bist) != 0) {
            goto already_ran;
        }

        val |= mmu_bist;
        soc_reg_field_set(unit, MMUMBISTENr, &val, MBISTMASTERENf, 1);
        SOC_IF_ERROR_RETURN(WRITE_MMUMBISTENr(unit, val));
        mmu_bist_go = (mmu_bist_done << 1);
    }

    if (ffp_bist != 0) {
        uint64 tmp64;

	SOC_MEM_BLOCK_ITER(unit, ffp_mem, blk) {
	    port = SOC_BLOCK_PORT(unit, blk);
            CHK_OP(READ_PT_DEBUGr(unit, port, &val64));
            COMPILER_64_TO_32_LO(val, val64);

            COMPILER_64_SET(tmp64, 0, ffp_bist);
            COMPILER_64_AND(tmp64, val64);
            if (!COMPILER_64_IS_ZERO(tmp64)) {
                goto already_ran;
            }

            COMPILER_64_SET(tmp64, 0, ffp_bist);
            COMPILER_64_OR(val64, tmp64);
            CHK_OP(WRITE_PT_DEBUGr(unit, port, val64));
        }
        ffp_bist_go = (ffp_bist_done << 1);
    }

    /*
     * Wait for corresponding GO bits to become set and
     * DONE bits to become reset in ARL_MEMBIST_STATUS
     * and MMUMBISTSTATUS registers.
     */
    soc_cm_debug(DK_SOCMEM, "bist_5690: waiting for completion\n");

    for (done = 0, msec = 0;
	 ! done && msec < timeout_msec;
	 msec += BIST_POLL_MSEC) {

        sal_usleep(BIST_POLL_MSEC * MILLISECOND_USEC);

        done = 1;

        if (mmu_bist_done) {
            CHK_OP(READ_MMUMBISTSTATUSr(unit, &val));
            soc_cm_debug(DK_SOCMEM, "bist_5690: MMUMBISTSTATUSr=0x%08x\n",
			 val);

            if ((val & mmu_bist_done) != mmu_bist_done) {
                done = 0;
            }
        }

        if (arl_bist_done) {
            CHK_OP(READ_ARL_MEMBIST_STATUSr(unit, &val));
            soc_cm_debug(DK_SOCMEM, "bist_5690: ARL_MEMBIST_STATUSr=0x%08x\n",
			 val);

            if ((val & arl_bist_done) != arl_bist_done) {
                done = 0;
            }
        }

        if (ffp_bist_done) {
	    SOC_MEM_BLOCK_ITER(unit, ffp_mem, blk) {
		port = SOC_BLOCK_PORT(unit, blk);
                CHK_OP(READ_PT_DEBUGr(unit, port, &val64));
                COMPILER_64_TO_32_LO(val, val64);
                soc_cm_debug(DK_SOCMEM, "bist_5690: PT_DEBUGr.%s=0x%08x\n",
			     SOC_BLOCK_NAME(unit, blk), val);

                if ((val & ffp_bist_done) != ffp_bist_done) {
                    done = 0;
                }
            }
        }
    }

    if (!done) {
        soc_cm_debug(DK_ERR,
		     "bist_5690: operation timed out after %d msec\n",
		     timeout_msec);
        return SOC_E_INTERNAL;
    }

    /*
     * Check the results of the BIST for each memory under test.
     * This is done by checking the GO bit in ARL_MEMBIST_STATUSr
     * and MMUMBISTSTATUSr. 0 - Fail 1 - Pass.
     */

    if (arl_bist_done) {
        CHK_OP(READ_ARL_MEMBIST_STATUSr(unit, &val));
        fail = ((val & arl_bist_go) != arl_bist_go);
        arl_bist_go = val;
    }

    if (mmu_bist_done) {
        CHK_OP(READ_MMUMBISTSTATUSr(unit, &val));
        fail = (fail || ((val & mmu_bist_go) != mmu_bist_go));
        mmu_bist_go = val;
    }

    if (ffp_bist_done) {
	SOC_MEM_BLOCK_ITER(unit, ffp_mem, blk) {
	    port = SOC_BLOCK_PORT(unit, blk);
            CHK_OP(READ_PT_DEBUGr(unit, port, &val64));
            COMPILER_64_TO_32_LO(val, val64);

            if ((val & ffp_bist_go) != ffp_bist_go) {
                ffp_bist_go = val;
                fail = 1;
                break;
            }
        }
    }

    if (fail) {
        for (memno = 0; memno < num_mems; memno++) {
            mem = mems[memno];

            if (!SOC_MEM_IS_VALID(unit, mem)) {
                continue;
            }

            bit = soc_mem_bist_bit(unit, mem);

            if (soc_mem_is_bistepic(unit, mem)) {
		/* MAGIC */
                if ((arl_bist_go & (2 << ((bit - 7) << 1))) == 0) {
                    soc_cm_debug(DK_ERR,
				 "bist_5690: mem=%s addr=0x%x\n",
				 SOC_MEM_UFNAME(unit, mem),
				 soc_mem_addr(unit, mem, 0, 0));
                }
            } else if (soc_mem_is_bistcbp(unit, mem)) {
		/* MAGIC */
                if ((mmu_bist_go & (2 << ((bit - 1) << 1))) == 0) {
                    soc_cm_debug(DK_ERR,
				 "bist_5690: mem=%s addr=0x%x\n",
				 SOC_MEM_UFNAME(unit, mem),
				 soc_mem_addr(unit, mem, 0, 0));
                }
            } else if (soc_mem_is_bistffp(unit, mem)) {
                if ((ffp_bist_go & (1 << (bit + 2))) == 0) {
		    /* MAGIC */
                    soc_cm_debug(DK_ERR,
				 "bist_5690: mem=%s addr=0x%x\n",
				 SOC_MEM_UFNAME(unit, mem),
				 soc_mem_addr(unit, mem, 0, 0));
                }
            }
        }
        return(SOC_E_FAIL);
    }

    return(SOC_E_NONE);

 op_fail:
    soc_cm_debug(DK_ERR, "bist_5690: register access failed\n");
    return rv;
}

#endif /* BCM_DRACO_SUPPORT */

int
soc_bist(int unit, soc_mem_t *mems, int num_mems, int timeout_msec)
{
    int 		rv = SOC_E_NONE;

#ifdef BCM_DRACO_SUPPORT
    if (SOC_IS_DRACO(unit))
    {
        rv = soc_bist_bcm5690(unit, mems, num_mems, timeout_msec);
    }
#endif /* BCM_DRACO_SUPPORT */

    return rv;
}

/*
 * Function:
 *	soc_bist_all
 * Purpose:
 *	Runs BIST on all memories on the chip.
 * Notes:
 *	Chip should be reset (soc_reset) both before and after BIST.
 */

int
soc_bist_all(int unit)
{
    soc_mem_t		mem;
    int			num_mems = 0;

    for (mem = 0; mem < NUM_SOC_MEM; mem++) {
	if (soc_mem_is_valid(unit,mem) &&
	    (soc_mem_is_bistepic(unit, mem) ||
	     soc_mem_is_bistcbp(unit, mem) ||
	     soc_mem_is_bistffp(unit, mem))) {
	    _soc_bist_mems[num_mems++] = mem;
	}
    }

    return soc_bist(unit, _soc_bist_mems, num_mems, SOC_CONTROL(unit)->bistTimeout);
}
