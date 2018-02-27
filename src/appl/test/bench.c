/*
 * $Id: bench.c 1.85.20.3 Broadcom SDK $
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
 * Benchmark tests
 */

#include <sal/core/libc.h>
#include <stdarg.h>

#include <sal/types.h>
#include <soc/mem.h>
#include <soc/cm.h>

#include <sal/appl/pci.h>
#include <sal/appl/sal.h>
#include <sal/core/boot.h>
#include <sal/core/time.h>

#include <soc/l2x.h>
#include <soc/l3x.h>
#include <soc/phyctrl.h>
#include <soc/phy.h>

#if defined(BCM_EASYRIDER_SUPPORT)
#include <soc/easyrider.h> /* For L3_DEFIP tests */
#include <soc/er_cmdmem.h>
#endif /* BCM_EASYRIDER_SUPPORT */

#include <bcm/error.h>
#include <bcm/link.h>
#ifdef BCM_ROBO_SUPPORT
#include <bcm/l2.h>
#endif /* BCM_ROBO_SUPPORT */

#if defined(BCM_ESW_SUPPORT)
#include <bcm_int/esw/mbcm.h>
#endif

#include <appl/diag/system.h>
#include <appl/diag/parse.h>
#include <appl/diag/test.h>
#include <appl/diag/progress.h>

#include "testlist.h"

#ifdef  BCM_SBX_SUPPORT
#include <soc/sbx/sbx_drv.h>
#endif

/* Decimal point formatting */
#define INT_FRAC_2PT(x) (x) / 100, (x) % 100

typedef struct benchmark_s {
    uint32	testMask;
    int		testNum;
    int		sizeMem;
    char	*bufMem;
    int		sizeDMA;
    char	*bufDMA;
    char	*testName;
    sal_usecs_t	stime, etime;
    uint32	dataSize;
    char	*dataUnit;
} benchmark_t;

#define BENCH_BUFMEM_SIZE	0x100000
#define BENCH_BUFDMA_SIZE	0x200000

int
benchmark_done(int u, void *pa)
{
    benchmark_t		*b = (benchmark_t *) pa;

    if (b) {
	if (b->bufMem)
	    sal_free(b->bufMem);
	if (b->bufDMA)
	    soc_cm_sfree(u, b->bufDMA);

	sal_free(b);
    }

    return 0;
}

int
benchmark_init(int u, args_t *a, void **pa)
{
    benchmark_t		*b = 0;
    int			rv = -1;
    parse_table_t       pt;

    *pa = NULL;

    if ((b = sal_alloc(sizeof (benchmark_t), "benchmark")) == 0)
	goto done;

    memset(b, 0, sizeof (*b));

    *pa = (void *) b;

    b->testMask = ~0;
    b->testNum = 0;

    b->sizeMem = BENCH_BUFMEM_SIZE;

    if ((b->bufMem = sal_alloc(b->sizeMem, "benchmark")) == 0) {
	printk("Not enough host memory\n");
	goto done;
    }

    b->sizeDMA = BENCH_BUFDMA_SIZE;

    if ((b->bufDMA = soc_cm_salloc(u, b->sizeDMA, "benchmark")) == 0) {
	printk("Not enough DMA memory\n");
	goto done;
    }

    parse_table_init(u, &pt);
    parse_table_add(&pt, "TestMask", PQ_HEX|PQ_DFL, 0,
                    &b->testMask, NULL);

    if (parse_arg_eq(a, &pt) < 0 || ARG_CNT(a) != 0) {
        test_error(u,
                   "%s: Invalid option: %s\n",
                   ARG_CMD(a),
                   ARG_CUR(a) ? ARG_CUR(a) : "*");
        parse_arg_eq_done(&pt);
	goto done;
    }

    parse_arg_eq_done(&pt);

    rv = 0;

 done:
    if (rv < 0)
	benchmark_done(u, *pa);

    return rv;
}

static int
benchmark_begin(benchmark_t *b,
		char *testName,
		char *dataUnit,
		uint32 dataSize)
{
    assert(b->testName == 0);

    if ((b->testMask & (1U << b->testNum)) == 0) {
	return 0;
    }

    b->testName = testName;
    b->dataSize = dataSize;
    b->dataUnit = dataUnit;
    b->stime = sal_time_usecs();

    return 1;
}

static void
benchmark_end(benchmark_t *b)
{
    if (b->testName) {
	int		per_sec, usec_per;
        sal_usecs_t     td;
	char            plus = ' ', lessthan = ' ';

        b->etime = sal_time_usecs();
        td = SAL_USECS_SUB(b->etime, b->stime);
	if (td == 0) {
	    td = 1; /* Non-zero value */
	    plus = '+';
	    lessthan = '<';
	}

        per_sec = _shr_div_exp10(b->dataSize, td, 8);
        usec_per = b->dataSize ? _shr_div_exp10(td, b->dataSize, 2) : 0;

	printk("%2d) %-28s%7d.%02d%c %5s/sec  ; %c%7d.%02d usec/%s\n",
	       b->testNum,
	       b->testName,
	       INT_FRAC_2PT(per_sec),
	       plus,
	       b->dataUnit,
	       lessthan,
	       INT_FRAC_2PT(usec_per),
	       b->dataUnit);

	b->testName = 0;
    }

    b->testNum++;
}

#if defined BCM_TRIUMPH_SUPPORT || defined BCM_SCORPION_SUPPORT
static int
_gen_l2x_insert(int unit, l2x_entry_t *entry)
{
    return soc_mem_insert(unit, L2Xm, MEM_BLOCK_ANY, (void *)entry);
}

static int
_gen_l2x_delete(int unit, l2x_entry_t *entry)
{
    return soc_mem_delete(unit, L2Xm, MEM_BLOCK_ANY, (void *)entry);
}

static int
_gen_l2x_lookup(int unit, l2x_entry_t *key,
                l2x_entry_t *result, int *index_ptr)
{
    return soc_mem_search(unit, L2Xm, MEM_BLOCK_ANY, index_ptr, 
                          (void *)key, (void *)result, 0);
}
#endif /* BCM_TRIUMPH_SUPPORT */

int
benchmark_test(int unit, args_t *a, void *pa)
{
    benchmark_t		*b = (benchmark_t *) pa;
    int                 rv = 0;
    uint16		phy_data;
    int			iter_mem, iter_cmic, iter_miim, iter_dma, iter_pci;
    int			quickturn, i;

#if defined(BCM_ESW_SUPPORT) || defined(BCM_ROBO_SUPPORT)
    uint32		reg = 0;
    uint32		entry[SOC_MAX_MEM_WORDS];
    soc_reg_t   test_register;
#endif

#if defined(BCM_XGS_SWITCH_SUPPORT) || defined(BCM_ROBO_SUPPORT)
    sal_mac_addr_t mac = { 0x00, 0x01, 0x10, 0x55, 0x66, 0x77 };
    sal_mac_addr_t maclkup = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
#endif /* BCM_XGS_SWITCH_SUPPORT || BCM_ROBO_SUPPORT */

#ifdef BCM_ESW_SUPPORT
    int			blk = 0, port = 0, phy_port = 0;
#endif /* BCM_ESW_SUPPORT */

#ifdef BCM_ROBO_SUPPORT
    uint32	        reg_len, reg_addr;
    int			index, index_min, index_max, entry_size;
    bcm_l2_addr_t l2addr;
#endif /* BCM_ROBO_SUPPORT */

    quickturn = (SAL_BOOT_QUICKTURN != 0);

    if (quickturn) {
    	iter_mem = 100;
    	iter_cmic = 200;
    	iter_miim = 200;
    	iter_dma = 1;
    	iter_pci = 2000;
    } else if (NULL != SOC_CONTROL(unit)->soc_rcpu_schan_op){
        iter_mem = 10;
        iter_cmic = 5000; 
        iter_miim = 0;
        iter_dma = 0;
        iter_pci = 0; 
    } else {
    	iter_mem = 10;
    	iter_cmic = 5000;
    	iter_miim = 5000;
    	iter_dma = 100;
    	iter_pci = 50000;
    }

    if (benchmark_begin(b, "memset sys mem", "MB",
			iter_mem * (b->sizeMem / 0x100000))) {
	for (i = 0; i < iter_mem; i++) {
	    memset(b->bufMem, 0, b->sizeMem);
        }
    }
    benchmark_end(b);

    if (benchmark_begin(b, "memcpy sys mem", "MB",
            iter_mem / 2 * (b->sizeMem / 0x100000))) {
        for (i = 0; i < iter_mem; i++) {
            memcpy(b->bufMem + b->sizeMem / 2, b->bufMem, b->sizeMem / 2);
        }
    }
    benchmark_end(b);


    if (benchmark_begin(b, "memset dma mem", "MB",
			iter_mem * (b->sizeDMA / 0x100000))) {
	for (i = 0; i < iter_mem; i++) {
	    memset(b->bufDMA, 0, b->sizeDMA);
        }
    }
    benchmark_end(b);

    if (benchmark_begin(b, "memcpy dma mem", "MB",
            iter_mem  / 2 * (b->sizeDMA / 0x100000 ))) {
        for (i = 0; i < iter_mem; i++) {
            memcpy(b->bufDMA + b->sizeDMA / 2, b->bufDMA, b->sizeDMA / 2);
        }
    }
    benchmark_end(b);

#ifdef BCM_ESW_SUPPORT
    if (SOC_IS_ESW(unit)) {
        test_register = SOC_REG_IS_VALID(unit, CMIC_DEV_REV_IDr) ? \
            CMIC_DEV_REV_IDr : CMIC_I2C_DATAr;
        if (benchmark_begin(b, "read pci reg", "read", iter_pci)) {
            for (i = 0; i < iter_pci; i++) {
                soc_pci_getreg(unit, soc_reg_addr(unit, test_register, 
                                              REG_PORT_ANY, 0), &reg);
            }
        }
   
        benchmark_end(b);

        if (benchmark_begin(b, "write pci reg", "write", iter_pci)) {
            for (i = 0; i < iter_pci; i++) {
                soc_pci_write(unit, soc_reg_addr(unit, test_register, 
                                             REG_PORT_ANY, 0), reg);
            }
        }
        benchmark_end(b);
    }
#endif /* BCM_ESW_SUPPORT */

#ifdef BCM_ESW_SUPPORT
    if (SOC_IS_ESW(unit)) {
        /* find an appropriate block to read and write */
        if (SOC_PORT_NUM(unit, fe) > 0) {
	    port = SOC_PORT(unit, fe, 0);
        } else if (SOC_PORT_NUM(unit, ge) > 0) {
	    port = SOC_PORT(unit, ge, 0);
        } else if (SOC_PORT_NUM(unit, xe) > 0) {
	    port = SOC_PORT(unit, xe, 0);
        } else {
	    port = SOC_PORT(unit, hg, 0);
        }
        if (soc_feature(unit, soc_feature_logical_port_num)) {
            phy_port = SOC_INFO(unit).port_l2p_mapping[port];
        } else {
            phy_port = port;
        }
        blk = SOC_PORT_BLOCK(unit, phy_port);
    }
#endif /* BCM_ESW_SUPPORT */

    if (benchmark_begin(b, "read soc reg", "read",
			iter_cmic)) {
        if (SOC_IS_ESW(unit)) {
#ifdef BCM_ESW_SUPPORT
            if (SOC_IS_HERCULES(unit)) {
                for (i = 0; i < iter_cmic; i++) {
                    READ_ING_COS_MAPr(unit, port, &reg);
                }
            } else if (SOC_IS_TUCANA(unit)) {
                for (i = 0; i < iter_cmic; i++) {
                    READ_RESETLIMITSr(unit, port, &reg);
                }
#ifdef BCM_TRX_SUPPORT
            } else if (SOC_IS_TRX(unit)) {
                for (i = 0; i < iter_cmic; i++) {
                    READ_EGR_CONFIGr(unit, &reg);
                }
#endif /* BCM_TRX_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
            } else if (SOC_IS_FBX(unit)) {
                for (i = 0; i < iter_cmic; i++) {
                    READ_EMIRROR_CONTROLr(unit, port, &reg);
                }
#endif /* BCM_FIREBOLT_SUPPORT */
#ifdef BCM_EASYRIDER_SUPPORT
            } else if (SOC_IS_EASYRIDER(unit)) {
                for (i = 0; i < iter_cmic; i++) {
                    READ_IGR_CONFIGr(unit, &reg);
                }
#endif /* BCM_EASYRIDER_SUPPORT */
            } else {
                for (i = 0; i < iter_cmic; i++) {
                    READ_CONFIGr(unit, port, &reg);
                }
            }
#endif /* BCM_ESW_SUPPORT */
        } else {
#ifdef BCM_ROBO_SUPPORT
            for (test_register = 0; test_register < NUM_SOC_ROBO_REG; test_register++) {
                if (&SOC_REG_INFO(unit, test_register) == NULL) {            
                    continue;
                }
                if (!SOC_REG_IS_VALID(unit, test_register)) {
                    continue;
                }
                /* Select the first found valid soc register */
                if ((SOC_REG_INFO(unit, test_register).regtype == soc_genreg) ||
                    (SOC_REG_INFO(unit, test_register).regtype == soc_portreg) ||
                    (SOC_REG_INFO(unit, test_register).regtype == soc_cosreg)) {
                    break;
                }
            }
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, test_register);
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, test_register, 0, 0);
            for (i = 0; i < iter_cmic; i++) {
                (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, &reg, reg_len);
            }
#endif /* BCM_ROBO_SUPPORT */
       }
    }
    benchmark_end(b);
    if (benchmark_begin(b, "write soc reg", "write",
			iter_cmic)) {
        if (SOC_IS_ESW(unit)) {
#ifdef BCM_ESW_SUPPORT
            if (SOC_IS_HERCULES(unit)) {
                for (i = 0; i < iter_cmic; i++) {
                    WRITE_ING_COS_MAPr(unit, port, reg);
                }
            } else if (SOC_IS_TUCANA(unit)) {
                for (i = 0; i < iter_cmic; i++) {
                    WRITE_RESETLIMITSr(unit, port, reg);
                }
#ifdef BCM_TRX_SUPPORT
            } else if (SOC_IS_TRX(unit)) {
                for (i = 0; i < iter_cmic; i++) {
                    WRITE_EGR_CONFIGr(unit, reg);
                }
#endif /* BCM_TRX_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
            } else if (SOC_IS_FBX(unit)) {
                for (i = 0; i < iter_cmic; i++) {
                    WRITE_EMIRROR_CONTROLr(unit, port, reg);
                }
#endif /* BCM_FIREBOLT_SUPPORT */
#ifdef BCM_EASYRIDER_SUPPORT
            } else if (SOC_IS_EASYRIDER(unit)) {
                for (i = 0; i < iter_cmic; i++) {
                    WRITE_IGR_CONFIGr(unit, reg);
                }
#endif /* BCM_EASYRIDER_SUPPORT */
            } else {
                for (i = 0; i < iter_cmic; i++) {
                    WRITE_CONFIGr(unit, port, reg);
                }
	     }
#endif /* BCM_ESW_SUPPORT */
        } else {
#ifdef BCM_ROBO_SUPPORT
            for (test_register = 0; test_register < NUM_SOC_ROBO_REG; test_register++) {
                if (&SOC_REG_INFO(unit, test_register) == NULL) {            
                    continue;
                }
                if (!SOC_REG_IS_VALID(unit, test_register)) {
                    continue;
                }
                /* Select the first found valid soc register */
                if (SOC_REG_INFO(unit, test_register).regtype == soc_genreg ||
                    SOC_REG_INFO(unit, test_register).regtype == soc_portreg ||
                    SOC_REG_INFO(unit, test_register).regtype == soc_cosreg) {
                    break;
                }
            }
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, test_register);
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, test_register, 0, 0);
            for (i = 0; i < iter_cmic; i++) {
                (DRV_SERVICES(unit)->reg_write)
                    (unit, reg_addr, &reg, reg_len);
            }
#endif /* BCM_ROBO_SUPPORT */
        }
    }
    benchmark_end(b);

    if (SOC_IS_ESW(unit)) {
#ifdef BCM_ESW_SUPPORT
        if (SOC_IS_HERCULES(unit)) {
            if (benchmark_begin(b, "read mc table entry", "read",
                iter_cmic)) {
                for (i = 0; i < iter_cmic; i++)
                    READ_MEM_MCm(unit, blk, 10, entry);
            }
            benchmark_end(b);
            
            if (benchmark_begin(b, "write mc table entry", "write",
                iter_cmic)) {
                for (i = 0; i < iter_cmic; i++)
                    WRITE_MEM_MCm(unit, blk, 10, entry);
            }
            benchmark_end(b);
#ifdef BCM_BRADLEY_SUPPORT
    } else if (SOC_IS_XGS3_FABRIC(unit)) {
        blk = SOC_MEM_BLOCK_ANY(unit, L2MCm);
        if (benchmark_begin(b, "read l2mc table entry", "read",
            iter_cmic)) {
            for (i = 0; i < iter_cmic; i++)
                READ_L2MCm(unit, blk, 10, entry);
        }
        benchmark_end(b);
        
        if (benchmark_begin(b, "write l2mc table entry", "write",
            iter_cmic)) {
            for (i = 0; i < iter_cmic; i++)
                WRITE_L2MCm(unit, blk, 10, entry);
        }
        benchmark_end(b);
#endif /* BCM_BRADLEY_SUPPORT */
#ifdef BCM_RAPTOR_SUPPORT
    } else if (SOC_IS_RAPTOR(unit) || SOC_IS_RAVEN(unit) || SOC_IS_HAWKEYE(unit)) {
        blk = SOC_MEM_BLOCK_ANY(unit, FP_POLICY_TABLEm);
        if (benchmark_begin(b, "read FP Policy table entry", "read",
            iter_cmic)) {
            for (i = 0; i < iter_cmic; i++)
                READ_FP_POLICY_TABLEm(unit, blk, 10, entry);
        }
        benchmark_end(b);
        
        if (benchmark_begin(b, "write FP Policy table entry", "write",
            iter_cmic)) {
            for (i = 0; i < iter_cmic; i++)
                WRITE_FP_POLICY_TABLEm(unit, blk, 10, entry);
        }
        benchmark_end(b);
#endif /* BCM_RAPTOR_SUPPORT */
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT)
    } else if (SOC_IS_FIREBOLT2(unit) || SOC_IS_TRX(unit)) {
        blk = SOC_MEM_BLOCK_ANY(unit, FP_POLICY_TABLEm);
        if (benchmark_begin(b, "read FP Policy table entry", "read",
            iter_cmic)) {
            for (i = 0; i < iter_cmic; i++)
                READ_FP_POLICY_TABLEm(unit, blk, 10, entry);
        }
        benchmark_end(b);
        
        
        /* No further testing for RCPU_ONLY units */
        if(SOC_IS_RCPU_ONLY(unit)) {
            return rv; 
        }
        
        if (benchmark_begin(b, "Clear L2X table", "clear",
            iter_mem)) {
            for (i = 0; i < iter_mem; i++)
    /*    coverity[unchecked_value]    */
                soc_mem_clear(unit, L2Xm, MEM_BLOCK_ALL, TRUE);
        }
        benchmark_end(b);
        
        if (benchmark_begin(b, "write FP Policy table entry", "write",
            iter_cmic)) {
            for (i = 0; i < iter_cmic; i++)
                WRITE_FP_POLICY_TABLEm(unit, blk, 10, entry);
        }
        benchmark_end(b);
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
    } else if (SOC_IS_FBX(unit)) {
        blk = SOC_MEM_BLOCK_ANY(unit, FP_TCAM_PLUS_POLICYm);
        if (benchmark_begin(b, "read FP Policy table entry", "read",
            iter_cmic)) {
            for (i = 0; i < iter_cmic; i++)
                READ_FP_TCAM_PLUS_POLICYm(unit, blk, 10, entry);
        }
        benchmark_end(b);
        
        if (benchmark_begin(b, "write FP Policy table entry", "write",
            iter_cmic)) {
            for (i = 0; i < iter_cmic; i++)
                WRITE_FP_TCAM_PLUS_POLICYm(unit, blk, 10, entry);
        }
        benchmark_end(b);
#endif /* BCM_FIREBOLT_SUPPORT */
#ifdef BCM_EASYRIDER_SUPPORT
    } else if (SOC_IS_EASYRIDER(unit)) {
        blk = SOC_MEM_BLOCK_ANY(unit, FP_POLICY_INTERNALm);
        if (benchmark_begin(b, "read FP Policy table entry", "read",
            iter_cmic)) {
            for (i = 0; i < iter_cmic; i++)
                READ_FP_POLICY_INTERNALm(unit, blk, 10, entry);
        }
        benchmark_end(b);
        
        if (benchmark_begin(b, "write FP Policy table entry", "write",
            iter_cmic)) {
            for (i = 0; i < iter_cmic; i++)
                WRITE_FP_POLICY_INTERNALm(unit, blk, 10, entry);
        }
        benchmark_end(b);
#endif /* BCM_EASYRIDER_SUPPORT */
    } else {
        blk = SOC_MEM_BLOCK_ANY(unit, GFILTER_IRULEm);
        if (benchmark_begin(b, "read girule table entry", "read",
            iter_cmic)) {
            for (i = 0; i < iter_cmic; i++)
                READ_GFILTER_IRULEm(unit, blk, 10, entry);
        }
        benchmark_end(b);
        
        if (benchmark_begin(b, "write girule table entry", "write",
            iter_cmic)) {
            for (i = 0; i < iter_cmic; i++)
                WRITE_GFILTER_IRULEm(unit, blk, 10, entry);
        }
        benchmark_end(b);
    }
#endif /* BCM_ESW_SUPPORT */
    } else {
#ifdef BCM_ROBO_SUPPORT
        if (benchmark_begin(b, "read ARL table entry", "read",
                        iter_cmic)) {
            for (i = 0; i < iter_cmic; i++) {
                (DRV_SERVICES(unit)->mem_read)(
                    unit, DRV_MEM_ARL, 10, 1, entry);
            }
        }
        benchmark_end(b);

        if (benchmark_begin(b, "write ARL table entry", "write",
                        iter_cmic)) {
            for (i = 0; i < iter_cmic; i++) {
                (DRV_SERVICES(unit)->mem_write)(
                unit, DRV_MEM_ARL, 10, 1, entry);
            }
        }
        benchmark_end(b);
#endif /* BCM_ROBO_SUPPORT */
    }

    if (! quickturn) {
	if (benchmark_begin(b, "read phy reg", "read",
			    iter_miim)) {
	    for (i = 0; i < iter_miim; i++)
		soc_miim_read(unit, PORT_TO_PHY_ADDR(unit, 2),
			      MII_PHY_ID0_REG, &phy_data);
	}
	benchmark_end(b);

	if (benchmark_begin(b, "write phy reg", "write",
			    iter_miim)) {
	    for (i = 0; i < iter_miim; i++)
		soc_miim_write(unit, PORT_TO_PHY_ADDR(unit, 2),
			       MII_PHY_ID0_REG, phy_data);
	}
	benchmark_end(b);
    }

#ifdef BCM_ESW_SUPPORT
#if defined(BCM_EASYRIDER_SUPPORT)
    if (SOC_IS_EASYRIDER(unit)) {
        l2_entry_internal_entry_t l2ent;
#if defined(INCLUDE_L3)
        char                      *defip_text;
	soc_mem_t                 defip_mem, tmem;
	defip_alg_entry_t         defip_data[3], defip_out;
	int                       defip_size, num_ops;
	int                       size, ext_cfg;
	uint32                    *ip_ptr[2];
	uint8                     null_ip6_addr[BCM_IP6_ADDRLEN];
	uint8                     ip6_addr[BCM_IP6_ADDRLEN] = {
	  0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 
	  0, 0, 0, 0, 0, 0, 0, 0};
        static char               buf[80];
#endif /* INCLUDE_L3 */

        memset(&l2ent, 0, sizeof(l2ent));
        soc_L2_ENTRY_INTERNALm_mac_addr_set(unit, &l2ent, MAC_ADDRf, mac);
        soc_L2_ENTRY_INTERNALm_field32_set(unit, &l2ent, VLAN_IDf, 1);

        if (benchmark_begin(b, "L2 insert/delete", "i+d",
                            iter_cmic)) {
            for (i = 0; i < iter_cmic; i++) {
    /*    coverity[unchecked_value]    */
                soc_er_l2x_insert(unit, &l2ent);
    /*    coverity[unchecked_value]    */
                soc_er_l2x_delete(unit, &l2ent);
            }
        }
        benchmark_end(b);

        memset(&l2ent, 0, sizeof(l2ent));
        soc_L2_ENTRY_INTERNALm_mac_addr_set(unit, &l2ent, MAC_ADDRf, maclkup);
        soc_L2_ENTRY_INTERNALm_field32_set(unit, &l2ent, VLAN_IDf, 1);

        if (benchmark_begin(b, "L2 lookup", "lkup",
                            iter_cmic)) {
            for (i = 0; i < iter_cmic; i++) {
                l2_entry_internal_entry_t l2xresult;
                soc_mem_t       mem;
                int		index;
    /*    coverity[unchecked_value]    */
                soc_er_l2x_lookup(unit, &l2ent, &l2xresult, &index, &mem);
            }
        }
        benchmark_end(b);

	if (benchmark_begin(b, "DMA L2 table", "xfer",
			    iter_dma)) {
	    for (i = 0; i < iter_dma; i++)
		if (soc_mem_read_range(unit, L2_ENTRY_INTERNALm, MEM_BLOCK_ANY,
				   soc_mem_index_min(unit, L2_ENTRY_INTERNALm),
				   soc_mem_index_max(unit, L2_ENTRY_INTERNALm),
				   b->bufDMA) < 0) {
                    rv = -1;
                    break;
                }

	}
	benchmark_end(b);

#if defined(INCLUDE_L3)
	/* Easyrider L3_DEFIP tests */

	if (soc_mem_index_count(unit, L3_DEFIP_CAMm) > 0) {
	    tmem = L3_DEFIP_CAMm;
	    defip_text = "TCAM";
	    defip_size = soc_mem_index_count(unit, L3_DEFIP_CAMm);
	} else {
	    tmem = L3_DEFIP_ALGm;
	    defip_size = 2048; /* limit the number of entries to speed up test */
            ext_cfg = SOC_PERSIST(unit)->er_memcfg.ext_table_cfg;
	    if ((ext_cfg != ER_SEER_CFG_NO_EXT) &&
	        (ext_cfg != ER_SEER_CFG_L2_512_EXT)) {
	        defip_text = "External ALG";
	    }
	    else {
	        defip_text = "Internal ALG";
	    }
	}

	printk("Running L3_DEFIP tests (%s[%d])", defip_text, defip_size);
	if (tmem == L3_DEFIP_CAMm) {
	    printk("...\n");
	}
	else {
	    printk("; this may take a while...\n");
	}

	num_ops = defip_size / 2;
        defip_mem = L3_DEFIP_ALGm;

	/* Setup base IPv4 entry */
	sal_memset(&defip_data[0], 0, sizeof(defip_data[0]));
	soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[0], 
			    FFf, 0xff);
	soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[0], 
			    KEY_TYPEf, SOC_ER_DEFIP_KEY_TYPE_V4);
	soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[0], 
			    PREFIX_LENGTHf, 16);
	soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[0], 
			    VALIDf, 1);

	/* Setup base IPv6 entry */
	sal_memset(null_ip6_addr, 0, sizeof(null_ip6_addr));
	sal_memset(&defip_data[1], 0, sizeof(defip_data[1]));
	soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[1], 
			    PREFIX_LENGTHf, 16);
	soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[1], 
			    VALIDf, 1);

	/* Setup base MPLS entry */
	sal_memset(&defip_data[2], 0, sizeof(defip_data[2]));
	soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[2], 
			    FFf, 0xff);
	soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[2], 
			    KEY_TYPEf, SOC_ER_DEFIP_KEY_TYPE_MPLS_2L);
	soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[2], 
			    VALIDf, 1);

	/* Fixed prefix, incrementing IP addresses (mix of IPv4 and IPv6) */
	soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[0], 
			    IP_ADDR_V4f, 0);
	soc_mem_ip6_addr_set(unit, L3_DEFIP_ALGm, &defip_data[1], 
			     IP_ADDR_V6f, null_ip6_addr, 
			     SOC_MEM_IP6_UPPER_ONLY);

        progress_init(num_ops, 3, 0);
        sal_sprintf(buf, "DEFIP insert, inc IP (%d operations)", num_ops);
        progress_status(buf);
	ip_ptr[0] = (uint32 *)(&defip_data[0]); /* Points to IPv4 addr */
	/* ip_ptr[1] points to IPv6 addr[127:96] */
	ip_ptr[1] = (uint32 *)(&defip_data[1]) + 1; 
        if (benchmark_begin(b, "DEFIP insert, inc IP", "ins", num_ops)) {
            for (i = 0; i < num_ops; i++) {
    /*    coverity[unchecked_value]    */
		soc_mem_insert(unit, defip_mem, COPYNO_ALL, &defip_data[i&1]);
		*ip_ptr[i&1]+=0x00010000;
                progress_report(1);
            }
        }
        progress_done();
        benchmark_end(b);

        progress_init(num_ops, 3, 0);
        sal_sprintf(buf, "DEFIP delete, inc IP (%d operations)", num_ops);
        progress_status(buf);
	*ip_ptr[0] = *ip_ptr[1] = 0; /* Delete in insert order */
        if (benchmark_begin(b, "DEFIP delete, inc IP", "del", num_ops)) {
            for (i = 0; i < num_ops; i++) {
    /*    coverity[unchecked_value]    */
		soc_mem_delete(unit, defip_mem, COPYNO_ALL, &defip_data[i&1]);
		*ip_ptr[i&1]+=0x00010000; 
                progress_report(1);
            }
        }
        progress_done();
        benchmark_end(b);

        progress_init(num_ops, 3, 0);
        sal_sprintf(buf, "DEFIP insert, dec IP (%d operations)", num_ops);
        progress_status(buf);
	/* Fixed prefix, decrementing IP address (mix of IPv4 and IPv6) */
	*ip_ptr[0] = *ip_ptr[1] = 0xc0a80000;
        if (benchmark_begin(b, "DEFIP insert, dec IP", "ins", num_ops)) {
            for (i = 0; i < num_ops; i++) {
    /*    coverity[unchecked_value]    */
		soc_mem_insert(unit, defip_mem, COPYNO_ALL, &defip_data[i&1]);
		*ip_ptr[i&1] -= 0x00010000;
                progress_report(1);
            }
        }
        progress_done();
        benchmark_end(b);

        progress_init(num_ops, 3, 0);
        sal_sprintf(buf, "DEFIP delete, dec IP (%d operations)", num_ops);
        progress_status(buf);
	*ip_ptr[0] = *ip_ptr[1] = 0xc0a80000; /* Delete in insert order */
        if (benchmark_begin(b, "DEFIP delete, dec IP", "del", num_ops)) {
            for (i = 0; i < num_ops; i++) {
    /*    coverity[unchecked_value]    */
		soc_mem_delete(unit, defip_mem, COPYNO_ALL, &defip_data[i&1]);
		*ip_ptr[i&1] -= 0x00010000; 
                progress_report(1);
            }
        }
        progress_done();
        benchmark_end(b);

	/* Fixed IP, increasing mask size (mix of IPv4 and IPv6) */

	soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[0], 
			    IP_ADDR_V4f, 0xc0a81234);
	soc_mem_ip6_addr_set(unit, L3_DEFIP_ALGm, &defip_data[1], 
			     IP_ADDR_V6f, ip6_addr, 
			     SOC_MEM_IP6_UPPER_ONLY);

        if (benchmark_begin(b, "DEFIP insert, inc mask", "ins", num_ops)) {
	    b->dataSize = 0; /* Only count the actual inserts */
            for (size=1; size <= 64; size++) {
	        if (size < 32) {
		    soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[0], 
					PREFIX_LENGTHf, size+1);
		    b->dataSize++;
    /*    coverity[unchecked_value]    */
		    soc_mem_insert(unit, defip_mem, COPYNO_ALL, 
				   &defip_data[0]); 
		}
		soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[1], 
				    PREFIX_LENGTHf, size);
		b->dataSize++;
    /*    coverity[unchecked_value]    */
		soc_mem_insert(unit, defip_mem, COPYNO_ALL, 
			       &defip_data[1]); 
            }
        }
        benchmark_end(b);

        if (benchmark_begin(b, "DEFIP delete, inc mask", "del", num_ops)) {
	    b->dataSize = 0; /* Only count the actual deletes */
            for (size=1; size <= 64; size++) {
	        if (size < 32) {
		    soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[0], 
					PREFIX_LENGTHf, size+1);
		    b->dataSize++;
    /*    coverity[unchecked_value]    */
		    soc_mem_delete(unit, defip_mem, COPYNO_ALL, 
				   &defip_data[0]); 
		}
		soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[1], 
				    PREFIX_LENGTHf, size);
		b->dataSize++;
    /*    coverity[unchecked_value]    */
		soc_mem_delete(unit, defip_mem, COPYNO_ALL, 
			       &defip_data[1]); 
            }
        }
        benchmark_end(b);

	/* Fixed IP, decreasing mask size (mix of IPv4 and IPv6) */

        if (benchmark_begin(b, "DEFIP insert, dec mask", "ins", num_ops)) {
	    b->dataSize = 0; /* Only count the actual inserts */
            for (size=64; size >= 1; size--) {
	        if (size < 32) {
		    soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[0], 
					PREFIX_LENGTHf, size+1);
		    b->dataSize++;
    /*    coverity[unchecked_value]    */
		    soc_mem_insert(unit, defip_mem, COPYNO_ALL, 
				   &defip_data[0]); 
		}
		soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[1], 
				    PREFIX_LENGTHf, size);
		b->dataSize++;
    /*    coverity[unchecked_value]    */
		soc_mem_insert(unit, defip_mem, COPYNO_ALL, 
			       &defip_data[1]); 
            }
        }
        benchmark_end(b);

        if (benchmark_begin(b, "DEFIP delete, dec mask", "del", num_ops)) {
	    b->dataSize = 0; /* Only count the actual deletes */
            for (size=64; size >= 1; size--) {
	        if (size < 32) {
		    soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[0], 
					PREFIX_LENGTHf, size+1);
		    b->dataSize++;
    /*    coverity[unchecked_value]    */
		    soc_mem_delete(unit, defip_mem, COPYNO_ALL, 
				   &defip_data[0]); 
		}
		soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[1], 
				    PREFIX_LENGTHf, size);
		b->dataSize++;
    /*    coverity[unchecked_value]    */
		soc_mem_delete(unit, defip_mem, COPYNO_ALL, 
			       &defip_data[1]); 
            }
        }
        benchmark_end(b);

	/* Initialize DEFIP to a "search testable" state */
	/* Memory half filled with a mix of IPv4, IPv6 and MPLS entries */

	for (i=0; i<num_ops; i++) {
	    if (!(i%3)) {
	        /* IPv4 entry */
	        size = (i%31) + 2;
		soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[0], 
				    PREFIX_LENGTHf, size);
		soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[0], 
				    IP_ADDR_V4f, i << (33-size));
    /*    coverity[unchecked_value]    */
		soc_mem_insert(unit, defip_mem, COPYNO_ALL, 
			       &defip_data[0]); 
	    }
	    else if ((i%3) == 1) {
	        /* IPv6 entry */
	        size = (i & 0x3f) + 1;
		soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[1], 
				    PREFIX_LENGTHf, size);
		*ip_ptr[1] = i << (64-size);
    /*    coverity[unchecked_value]    */
		soc_mem_insert(unit, defip_mem, COPYNO_ALL, 
			       &defip_data[1]); 
	    }
	    else {
	        /* MPLS entry */
		soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[2], 
				    MPLS_LABEL1f, i & 0x1f);
		soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[2], 
				    MPLS_LABEL2f, (i >> 5) & 0x1f);
    /*    coverity[unchecked_value]    */
		soc_mem_insert(unit, defip_mem, COPYNO_ALL, 
			       &defip_data[2]); 
	    }
	}

        if (benchmark_begin(b, "DEFIP lookup (hits)", "lkup", num_ops)) {
	    int defip_idx;
	    int index;
            for (i = 0; i < num_ops; i++) {
	        index = i%3;
	        if (!index) {
		    /* IPv4 entry */
		    size = (i%31) + 2;
		    soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[0], 
					PREFIX_LENGTHf, size);
		    soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[0], 
					IP_ADDR_V4f, i << (33-size));
    /*    coverity[unchecked_value]    */
		    soc_mem_search(unit, defip_mem, COPYNO_ALL, &defip_idx,
				   &defip_data[0], &defip_out, FALSE);
		}
		else if (index == 1) {
		    /* IPv6 entry */
		    size = (i & 0x3f) + 1;
		    soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[1], 
					PREFIX_LENGTHf, size);
    /*    coverity[unchecked_value]    */
		    *ip_ptr[1] = i << (64-size);
    /** coverity[unchecked_value]    **/
		    soc_mem_search(unit, defip_mem, COPYNO_ALL, &defip_idx,
				   &defip_data[1], &defip_out, FALSE);
		}
		else {
		    /* MPLS entry */
		    soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[2], 
    /*    coverity[unchecked_value]    */
					MPLS_LABEL1f, i & 0x1f);
    /** coverity[unchecked_value]    **/
		    soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[2], 
					MPLS_LABEL2f, (i >> 5) & 0x1f);
    /** coverity[unchecked_value]    **/
		    soc_mem_search(unit, defip_mem, COPYNO_ALL, &defip_idx,
				   &defip_data[2], &defip_out, FALSE);
		}
            }
        }
        benchmark_end(b);

        if (benchmark_begin(b, "DEFIP lookup (misses)", "lkup", num_ops)) {
	    int defip_idx;
	    int index;
            for (i = 0; i < num_ops; i++) {
	        index = i%3;
	        if (!index) {
    /*    coverity[unchecked_value]    */
		    /* IPv4 entry */
    /** coverity[unchecked_value]    **/
		    size = (i%31) + 2;
		    soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[0], 
    /** coverity[unchecked_value]    **/
					PREFIX_LENGTHf, size);
		    soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[0], 
					IP_ADDR_V4f, (i+1) << (33-size));
    /*    coverity[unchecked_value]    */
    /** coverity[unchecked_value]    **/
		    soc_mem_search(unit, defip_mem, COPYNO_ALL, &defip_idx,
				   &defip_data[0], &defip_out, FALSE);
    /** coverity[unchecked_value]    **/
		}
		else if (index == 1) {
		    /* IPv6 entry */
    /** coverity[unchecked_value]    **/
    /*    coverity[unchecked_value]    */
		    size = (i & 0x3f) + 1;
		    soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[1], 
    /** coverity[unchecked_value]    **/
					PREFIX_LENGTHf, size);
		    *ip_ptr[1] = (i+1) << (64-size);
    /** coverity[unchecked_value]    **/
    /** coverity[unchecked_value]    **/
		    soc_mem_search(unit, defip_mem, COPYNO_ALL, &defip_idx,
				   &defip_data[1], &defip_out, FALSE);
		}
		else {
    /** coverity[unchecked_value]    **/
		    /* MPLS entry */
		    soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[2], 
					MPLS_LABEL1f, (i+1) & 0x1f);
		    soc_mem_field32_set(unit, L3_DEFIP_ALGm, &defip_data[2], 
					MPLS_LABEL2f, (i >> 5) & 0x1f);
    /** coverity[unchecked_value]    **/
		    soc_mem_search(unit, defip_mem, COPYNO_ALL, &defip_idx,
				   &defip_data[2], &defip_out, FALSE);
		}
            }
        }
        benchmark_end(b);
#endif /* INCLUDE_L3 */

        return rv; /* END OF ER BENCHMARKS */
    }
#endif /* BCM_EASYRIDER_SUPPORT */

#if defined(BCM_XGS12_SWITCH_SUPPORT) || defined(BCM_FIREBOLT_SUPPORT)
    if (SOC_IS_XGS_SWITCH(unit)) {
        l2x_entry_t l2xent;
#if defined(BCM_XGS3_SWITCH_SUPPORT) && defined(INCLUDE_L3)
        char *l3_dma_buf;
        int  alloc_size;
#endif /* BCM_XGS3_SWITCH_SUPPORT && INCLUDE_L3 */
        int (*l2x_ins)(int unit, l2x_entry_t *entry);
        int (*l2x_del)(int unit, l2x_entry_t *entry);
        int (*l2x_lkup)(int unit, l2x_entry_t *key, l2x_entry_t *result,
                       int *index_ptr);

#ifdef BCM_TRX_SUPPORT
        if (soc_feature(unit, soc_feature_generic_table_ops)) {
            l2x_ins = _gen_l2x_insert;
            l2x_del = _gen_l2x_delete;
            l2x_lkup = _gen_l2x_lookup;
        } else 
#endif /* BCM_TRX_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
        if (SOC_IS_FBX(unit)) {
            l2x_ins = soc_fb_l2x_insert;
            l2x_del = soc_fb_l2x_delete;
            l2x_lkup = soc_fb_l2x_lookup;
        } else 
#endif /* BCM_FIREBOLT_SUPPORT */
#if defined(BCM_XGS12_SWITCH_SUPPORT)
        {
            l2x_ins = soc_l2x_insert;
            l2x_del = soc_l2x_delete;
            l2x_lkup = soc_l2x_lookup;
        }
#else /* !BCM_XGS12_SWITCH_SUPPORT */
        {
            l2x_ins = NULL;
            l2x_del = NULL;
            l2x_lkup = NULL;
        }
#endif /* !BCM_XGS12_SWITCH_SUPPORT */

        memset(&l2xent, 0, sizeof(l2xent));
        soc_L2Xm_mac_addr_set(unit, &l2xent, MAC_ADDRf, mac);
        soc_L2Xm_field32_set(unit, &l2xent, VLAN_IDf, 1);

        if (benchmark_begin(b, "L2 insert/delete", "i+d",
                            iter_cmic)) {
            for (i = 0; i < iter_cmic; i++) {
                l2x_ins(unit, &l2xent);
                l2x_del(unit, &l2xent);
            }
        }
        benchmark_end(b);

        memset(&l2xent, 0, sizeof(l2xent));
        soc_L2Xm_mac_addr_set(unit, &l2xent, MAC_ADDRf, maclkup);
        soc_L2Xm_field32_set(unit, &l2xent, VLAN_IDf, 1);

        if (benchmark_begin(b, "L2 lookup", "lkup",
                            iter_cmic)) {
            for (i = 0; i < iter_cmic; i++) {
                l2x_entry_t	l2xresult;
                int		index;
                l2x_lkup(unit, &l2xent, &l2xresult, &index);
            }
        }
        benchmark_end(b);

	if (benchmark_begin(b, "DMA L2 table", "xfer",
			    iter_dma)) {
	    for (i = 0; i < iter_dma; i++)
		if (soc_mem_read_range(unit, L2Xm, MEM_BLOCK_ANY,
				   soc_mem_index_min(unit, L2Xm),
				   soc_mem_index_max(unit, L2Xm),
				   b->bufDMA) < 0) {
                   rv = -1;
                   break;
                }
	}
	benchmark_end(b);

#if defined(BCM_XGS3_SWITCH_SUPPORT) && defined(INCLUDE_L3)
        if (SOC_IS_FBX(unit) &&
            !SOC_IS_HAWKEYE(unit) &&
            SOC_MEM_IS_VALID(unit, L3_ENTRY_ONLYm) &&
            (soc_mem_index_count(unit,L3_ENTRY_ONLYm) > 0)) {

            if (benchmark_begin(b, "Read L3_ENTRY_V4 table ", "xfer",
                                iter_cmic)) {
                for (i = 0; i < iter_cmic; i++) {
                    READ_L3_ENTRY_ONLYm(unit, blk, 10, entry);
                }
            }
            benchmark_end(b);

            alloc_size = (SOC_L3X_BUCKET_SIZE(unit) / 2) *
                WORDS2BYTES(soc_mem_entry_words(unit, L3_ENTRY_ONLYm));
            l3_dma_buf =  soc_cm_salloc(unit, alloc_size, "L3 bucket dma");
            if (NULL != l3_dma_buf) {
                sal_memset(l3_dma_buf, 0, alloc_size);

                if (benchmark_begin(b, "Dma read L3_ENTRY bucket", "xfer",
                                    iter_cmic)) {
                    for (i = 0; i < iter_cmic; i++) {
                        if (soc_mem_read_range(unit, L3_ENTRY_ONLYm, MEM_BLOCK_ANY,
                                           10 * SOC_L3X_BUCKET_SIZE(unit), 
                                           10 * SOC_L3X_BUCKET_SIZE(unit) + 
                                           (SOC_L3X_BUCKET_SIZE(unit) / 2)  - 1,
                                           l3_dma_buf) < 0) {
                            rv = -1;
                            break;
                       }
                    }
                }
                benchmark_end(b);

                soc_cm_sfree(unit, l3_dma_buf);
            } else {
                    rv = -1;
            }
        }
#endif /* BCM_XGS3_SWITCH_SUPPORT && INCLUDE_L3 */  
    }
#endif /* BCM_XGS12_SWITCH_SUPPORT || BCM_FIREBOLT_SUPPORT */
#endif /* BCM_ESW_SUPPORT */

#ifdef BCM_ROBO_SUPPORT
        if (SOC_IS_ROBO(unit)) {
       
        bcm_l2_addr_init(&l2addr, mac, 1);

        if (benchmark_begin(b, "L2 insert/delete", "i+d",
                            iter_cmic)) {
            for (i = 0; i < iter_cmic; i++) {
                bcm_l2_addr_add(unit, &l2addr);
                bcm_l2_addr_delete(unit, mac, 1);
            }
        }
        benchmark_end(b);

        if (benchmark_begin(b, "L2 lookup", "lkup",
                            iter_cmic)) {
            for (i = 0; i < iter_cmic; i++) {
                bcm_l2_addr_get(unit, maclkup, 1, &l2addr);
            }
        }
        benchmark_end(b);

	if (benchmark_begin(b, "DMA L2 table", "xfer",
			    iter_dma)) {
           index_min = soc_robo_mem_index_min(unit, L2_ARLm);
           index_max = soc_robo_mem_index_max(unit, L2_ARLm);
           entry_size = soc_mem_entry_bytes(unit, L2_ARLm);
	    for (i = 0; i < iter_dma; i++)
              for (index = index_min; index <= index_max; index++) {
                  (DRV_SERVICES(unit)->mem_read)(
                      unit, DRV_MEM_ARL, index, 1, entry);
                  index += (entry_size - 1);
              }
	}
	benchmark_end(b);
    } /* SOC_IS_ROBO() */
#endif /* BCM_ROBO_SUPPORT */

    {
         sal_mutex_t	m;

         m = sal_mutex_create("bench");

         if (benchmark_begin(b, "mutex lock/unlock", "lock",
                             iter_cmic)) {
             for (i = 0; i < iter_miim; i++) {
                 sal_mutex_take(m, sal_mutex_FOREVER);
                 sal_mutex_give(m);
             }
         }
         benchmark_end(b);
         sal_mutex_destroy(m);
    }

    return rv;
}
