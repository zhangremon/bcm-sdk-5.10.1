/*
 * $Id: tabledma.c 1.67.2.1 Broadcom SDK $
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
 * Table DMA Test
 */

#include <sal/appl/sal.h>
#include <appl/diag/system.h>
#include <appl/diag/parse.h>
#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/dma.h>
#include <soc/filter.h>
#include <soc/l2x.h>
#include "testlist.h"

#define TD_DEFAULT_ROM_VER_COUNT     10
#define TD_DEFAULT_DUMP_ENTRY_COUNT  10

#if defined BCM_XGS_SWITCH_SUPPORT || defined(BCM_SIRIUS_SUPPORT)

typedef struct tdt_data_s {
    int             init_done;
    char           *mem_str;
    soc_mem_t       mem;
    int             copyno;
    int             index_min;
    int             index_max;
    int             count;
    int             rom_ver_count;

    uint32         *source;
    uint32          srcaddr;
    uint32         *dma_ed;
    uint32          dstaddr;

    soc_mem_cmp_t   comp_fn;
    int             bufwords;
    int             entwords;

    uint32          rnum;
    int             ecc_as_data; /* treat ecc field as regular field */

    uint32         *datamask;
    uint32         *tcammask;
    uint32         *eccmask;
    uint32         accum_tcammask;
    int            ccm;
    int            tslam_en;    /* Table slam Enable */
    int            tslam_dir;   /* Table slam direction */
    int            td_check_data;
} td_test_data_t;


static td_test_data_t td_test_data;

static int td_test_mem_cmp_fn(int, void*, void*);

static int
td_test_data_init(int unit, int alloc)
{
    soc_mem_info_t *meminfo;
    int i;

    if (!td_test_data.init_done) {
#ifdef BCM_EASYRIDER_SUPPORT
        if (SOC_IS_EASYRIDER(unit)) {
            td_test_data.mem = L2_USER_ENTRYm;
            td_test_data.mem_str = sal_strdup("l2_user_entry");
            td_test_data.count = soc_mem_index_count(unit, L2_USER_ENTRYm);
        } else
#endif /* BCM_EASYRIDER_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
         if (SOC_IS_FBX(unit)) {
            td_test_data.mem = L2_ENTRY_ONLYm;
            td_test_data.mem_str = sal_strdup("l2_entry_only");
            td_test_data.count = soc_mem_index_count(unit, L2_ENTRY_ONLYm);
        } else
#endif /* BCM_FIREBOLT_SUPPORT */
#ifdef BCM_SIRIUS_SUPPORT
         if (SOC_IS_SIRIUS(unit)) {
	   /* no default for sirius */
        } else
#endif /* BCM_SIRIUS_SUPPORT */
        if (SOC_IS_XGS_SWITCH(unit)) {
            td_test_data.mem = L2X_BASEm;
            td_test_data.mem_str = sal_strdup("l2x_base");
            td_test_data.count = soc_mem_index_count(unit,L2Xm);
        } else {
            test_error(unit, "Table DMA Error:  Invalid SOC type\n");
            return -1;
        }
        td_test_data.init_done = TRUE;
        td_test_data.tslam_en = FALSE;
        td_test_data.tslam_dir = TRUE; /* SLAM from low to high */
        td_test_data.ccm = FALSE;
        td_test_data.rnum        = sal_rand();
        td_test_data.srcaddr = 0;
        td_test_data.dstaddr = 0;
        td_test_data.td_check_data = TRUE;
    }
#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (!SOC_IS_XGS3_SWITCH(unit)) {
        td_test_data.tslam_en = FALSE;
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
      td_test_data.tslam_en = TRUE;
    }
#endif /* BCM_SIRIUS_SUPPORT */

    /*
     *  Gets reset for every run. 
     */
    td_test_data.rom_ver_count = TD_DEFAULT_ROM_VER_COUNT;

    if (!alloc) {
        return 0;
    }

    meminfo = &(SOC_MEM_INFO(unit, td_test_data.mem));

    td_test_data.entwords    = soc_mem_entry_words(unit, td_test_data.mem);
    td_test_data.bufwords    = td_test_data.entwords * td_test_data.count;
    if (0 == td_test_data.srcaddr) {
        td_test_data.source      = soc_cm_salloc(unit,
                                         WORDS2BYTES(td_test_data.bufwords),
                                         "td_test_data source");
    } else {
        td_test_data.source = INT_TO_PTR(td_test_data.srcaddr);
    }

    if (0 == td_test_data.dstaddr) {
        td_test_data.dma_ed      = soc_cm_salloc(unit,
                                         WORDS2BYTES(td_test_data.bufwords),
                                         "td_test_data dma_ed");
    } else {
        td_test_data.dma_ed = INT_TO_PTR(td_test_data.dstaddr);
    }
    td_test_data.datamask    = sal_alloc(WORDS2BYTES(td_test_data.entwords),
                                         "td_test_data datamask");
    td_test_data.tcammask    = sal_alloc(WORDS2BYTES(td_test_data.entwords),
                                         "td_test_data testmask");
    td_test_data.eccmask     = sal_alloc(WORDS2BYTES(td_test_data.entwords),
                                         "td_test_data eccmask");
    if ((td_test_data.source == NULL) ||
        (td_test_data.dma_ed == NULL) ||
        (td_test_data.datamask == NULL) ||
        (td_test_data.tcammask == NULL) ||
        (td_test_data.eccmask == NULL)) {
        if (td_test_data.source && (0 == td_test_data.srcaddr)) {
            soc_cm_sfree(unit, td_test_data.source);
            td_test_data.source = NULL;
        }
        if (td_test_data.dma_ed && (0 == td_test_data.dstaddr)) {
            soc_cm_sfree(unit, td_test_data.dma_ed);
            td_test_data.dma_ed = NULL;
        }
        if (td_test_data.datamask) {
            sal_free(td_test_data.datamask);
            td_test_data.datamask = NULL;
        }
        if (td_test_data.tcammask) {
            sal_free(td_test_data.tcammask);
            td_test_data.tcammask = NULL;
        }
        if (td_test_data.eccmask) {
            sal_free(td_test_data.eccmask);
            td_test_data.eccmask = NULL;
        }
        test_error(unit, "Table DMA Error:  Failed to allocate DMA memory\n");
        return -1;
    }

    if (0 != td_test_data.rnum) {
        sal_memset(td_test_data.source, 0xff, WORDS2BYTES(td_test_data.bufwords));
        sal_memset(td_test_data.dma_ed, 0xff, WORDS2BYTES(td_test_data.bufwords));
    }
    td_test_data.datamask    = soc_mem_datamask_get(unit, td_test_data.mem,
                                                    td_test_data.datamask);
    td_test_data.tcammask    = soc_mem_tcammask_get(unit, td_test_data.mem,
                                                    td_test_data.tcammask);
    td_test_data.eccmask     = soc_mem_eccmask_get(unit, td_test_data.mem,
                                                   td_test_data.eccmask);
    td_test_data.accum_tcammask = 0;
    for (i = 0; i < td_test_data.entwords; i++) {
        td_test_data.accum_tcammask |= td_test_data.tcammask[i];
    }
    if (!td_test_data.ecc_as_data) {
        for (i = 0; i < td_test_data.entwords; i++) {
            td_test_data.datamask[i] &= ~td_test_data.eccmask[i];
        }
    }
    td_test_data.datamask    =
        soc_mem_datamask_memtest(unit, td_test_data.mem,
                                 td_test_data.datamask);

    td_test_data.comp_fn = td_test_mem_cmp_fn;

    return 0;
}

static void
td_test_dump_data(int unit, int entry_cnt)
{
    int     i, base_index, idx;
    uint32 *s, *d;

    printk("Table DMA test for %s.%s: index_min %d index_max %d\n"
           "    bufwords %d entwords %d rnum %d count %d\n",
           SOC_MEM_UFNAME(unit, td_test_data.mem),
           SOC_BLOCK_NAME(unit, td_test_data.copyno), td_test_data.index_min,
           td_test_data.index_max, td_test_data.bufwords,
           td_test_data.entwords, td_test_data.rnum, td_test_data.count);

    printk("    Datamask: ");
    for (idx = 0; idx < td_test_data.entwords; idx++) {
        printk(" %08x", td_test_data.datamask[idx]);
    }
    printk("\n\n");

    s = td_test_data.source;
    d = td_test_data.dma_ed;

    assert(s && d);

    base_index = td_test_data.index_min;
    s += td_test_data.index_min;

    for (i = td_test_data.index_min;
         i <= td_test_data.index_max && i - base_index < entry_cnt;
         ++i) {
        printk("    Table index %d: "
               "source entry %d, dma_ed entry %d\n",
               i, i - base_index, i - td_test_data.index_min);
        printk("    Source:  ");
        for (idx = 0; idx < td_test_data.entwords; idx++) {
            printk(" %08x", s[idx]);
        }
        printk("\n");
        printk("    DMA_ed:  ");
        for (idx = 0; idx < td_test_data.entwords; idx++) {
            printk(" %08x", d[idx]);
        }
        printk("\n\n");
        d += td_test_data.entwords;
        s += td_test_data.entwords;
    }
}

static void
td_test_data_clear(int unit)
{
    if (td_test_data.source && (0 == td_test_data.srcaddr)) {
        soc_cm_sfree(unit, td_test_data.source);
        td_test_data.source = NULL;
    }
    if (td_test_data.dma_ed && (0 == td_test_data.dstaddr)) {
        soc_cm_sfree(unit, td_test_data.dma_ed);
        td_test_data.dma_ed = NULL;
    }
    if (td_test_data.datamask) {
        sal_free(td_test_data.datamask);
        td_test_data.datamask = NULL;
    }
    if (td_test_data.tcammask) {
        sal_free(td_test_data.tcammask);
        td_test_data.tcammask = NULL;
    }
    if (td_test_data.eccmask) {
        sal_free(td_test_data.eccmask);
        td_test_data.eccmask = NULL;
    }
}

static uint32
td_test_get_curr_rnum(void)
{
    return td_test_data.rnum;
}

static uint32
td_test_rand32(uint32 prev)
{
    return td_test_data.rnum = 1664525L * prev + 1013904223L;
}

static uint32 *
td_test_random_entry(int unit, soc_mem_t mem, uint32 *entbuf)
{
    int i;

    for (i = 0; i < td_test_data.entwords; ++i) {
        uint32 prev = td_test_get_curr_rnum();
        entbuf[i]   = td_test_rand32(prev) & td_test_data.datamask[i];
    }

    return entbuf;
}

static int
td_test_write_entry(int unit, soc_mem_t mem, int copyno,
                 int index, uint32 *entbuf)
{
    int rv = 0;

    if (SOC_MEM_IS_VALID(unit, mem)) {
        rv = soc_mem_write(unit, mem, copyno, index, entbuf);
    }
    if (rv < 0) {
        test_error(unit,
                   "Write entry (copyno %d) failed: %s",
                   copyno, soc_errmsg(rv));
    }
    return rv;
}


static int
td_test_verify(int unit)
{
    int rv         = 0;
    soc_mem_t mem  = td_test_data.mem;
    int base_index = soc_mem_index_min(unit, mem);
    int index_min  = td_test_data.index_min;
    int index_max  = td_test_data.index_max;
    int ent_words  = soc_mem_entry_words(unit, mem);
    int i;
    int idx;
    uint32 *d;
    uint32 *s;

    
    if (
        (SOC_IS_LYNX(unit) &&
         ((mem == XFILTER_METERING_TEST2m) || 
          (mem == XFILTER_METERING_TEST3m))) 
        ||
        (SOC_IS_DRACO15(unit) &&
         (mem == DEFIP_HIm))     
#if defined(BCM_EASYRIDER_SUPPORT)
        ||
        (SOC_IS_EASYRIDER(unit) &&
         ((mem == EGR_VLAN_XLATEm) ||
          (mem == IGR_VLAN_XLATEm) ||
          (mem == VRF_VFI_INTFm)))
#endif /* BCM_EASYRIDER_SUPPORT */
        ) 
    
    {

        int entry, word;
        uint32 *mask = td_test_data.datamask;
        
        d = td_test_data.dma_ed;
        for (entry = 0; entry < (index_max - index_min + 1); entry++) {
            for (word = 0; word < ent_words; word++) {
                d[word] &= mask[word];
            }
            d += ent_words;
        }
    }
    
    /*
     * For readonly mem, compare a few dma-ed entries with
     * entries in hardware from direct reading.
     */
    if (soc_mem_is_readonly(unit, mem)) {
        int error = 0;
        s = td_test_data.source;
        d = td_test_data.dma_ed;
        for (i = 0; i < td_test_data.rom_ver_count; ++i) {
            if (i >= td_test_data.count) {
                break;
            }
            if (soc_mem_read(unit, mem, td_test_data.copyno,
                             i+td_test_data.index_min,
                             s) < 0) {
                printk("Cannot read entry %d\n", i+td_test_data.index_min);
                return -1;
            }
            if (td_test_data.comp_fn(unit, d, s) != 0) {
                error = -1;
                printk("%s.%s diff entry index %d: \n",
                       SOC_MEM_UFNAME(unit, mem),
                       SOC_BLOCK_NAME(unit, td_test_data.copyno),
                       i+td_test_data.index_min);
                printk("Read directly:\n");
                soc_mem_entry_dump(unit, mem, s); printk("\n");
                printk("Read by table dma:\n");
                soc_mem_entry_dump(unit, mem, d); printk("\n");
            }
            d += ent_words;
            s += ent_words;
        }
        return error;
    }

    if (!td_test_data.ecc_as_data) {
        for (i = 0; i <= index_max - index_min; i++) {
            for (idx = 0; idx < td_test_data.entwords; idx++) {
                td_test_data.dma_ed[i * ent_words + idx] &=
                    ~td_test_data.eccmask[idx];
            }
        }
    }

    if (sal_memcmp(td_test_data.dma_ed, td_test_data.source,
                   (index_max - index_min + 1) * ent_words * 4) != 0) {
        printk("Mismatch found\n");
        s = td_test_data.source;
        d = td_test_data.dma_ed;
        for (i = index_min; i <= index_max; ++i) {
            if (td_test_data.comp_fn(unit, d, s) != 0) {
                printk("%s.%s diff entry index %d: "
                       "source entry %d, dma_ed entry %d\n",
                       SOC_MEM_UFNAME(unit, mem),
                       SOC_BLOCK_NAME(unit, td_test_data.copyno),
                       i, i - base_index, i - index_min);
                printk("Source:");
                for (i = 0; i < ent_words; i++) {
                    printk(" 0x%08x", s[i]);
                }
                printk("\nDMA_ed:");
                for (i = 0; i < ent_words; i++) {
                    printk(" 0x%08x", d[i]);
                }
                printk("\n");

                return -1;
            }
            d += ent_words;
            s += ent_words;
        }
        /* If get here, something is wrong, dump for inspection */
        td_test_dump_data(unit, TD_DEFAULT_DUMP_ENTRY_COUNT);
    }

    return rv;
}

static int
td_test_mem_cmp_fn(int unit, void *s, void*d)
{
    soc_mem_t mem  = td_test_data.mem;
    int ent_words  = soc_mem_entry_words(unit, mem);

    assert(s && d);

    return sal_memcmp(s, d, ent_words * 4);
}


static int
td_test_dma_verify_helper(int unit)
{
    int rv = 0;

#ifdef BCM_CMICM_SUPPORT
    if(td_test_data.ccm) {
        if (TRUE == td_test_data.td_check_data) {
            if (sal_memcmp(td_test_data.dma_ed, td_test_data.source,
                           (td_test_data.count * 4)) != 0) {
                test_error(unit, "CCM DMA Compare failed\n");
                td_test_dump_data(unit, TD_DEFAULT_DUMP_ENTRY_COUNT);
                return -1;
            }
        }
    } else
#endif
    {
        if (0 != td_test_data.rnum) {
            sal_memset(td_test_data.dma_ed, 0xff, td_test_data.bufwords * 4);
        }

        if (SOC_IS_XGS_SWITCH(unit) ||
            SOC_IS_SIRIUS(unit)) {
            rv = soc_mem_read_range(unit, td_test_data.mem, td_test_data.copyno,
                                    td_test_data.index_min, td_test_data.index_max,
                                    td_test_data.dma_ed);
        } else {
            test_error(unit, "Chip type not supported for table DMA test\n");
            return -1;
        }        

        if (rv < 0) {
            test_error(unit,
                       "Table DMA failed: %s\n",
                       soc_errmsg(rv));
            td_test_dump_data(unit, TD_DEFAULT_DUMP_ENTRY_COUNT);
            return -1;
        }

        if (TRUE == td_test_data.td_check_data) {
            if ((rv = td_test_verify(unit)) < 0) {
                test_error(unit,
                           "Table DMA failed: inconsistency observed\n");
                td_test_dump_data(unit, TD_DEFAULT_DUMP_ENTRY_COUNT);
                return -1;
            }
        }
    }
    return 0;
}

int
td_test_test(int unit, args_t *a, void *pa)
{
    int        i;
    uint32    *entbuf;

#ifdef BCM_CMICM_SUPPORT
    if(td_test_data.ccm) {
        debugk(DK_TESTS, "Note:  Cross Coupled Memory \n");
    } else 
#endif
    {
        debugk(DK_TESTS,
           "Table DMA %s: copy %d, from entry %d for %d entries SEED = %d\n",
           td_test_data.mem_str,
           td_test_data.copyno,
           td_test_data.index_min,
           td_test_data.count,
           td_test_data.rnum);
        if (!soc_mem_dmaable(unit, td_test_data.mem, td_test_data.copyno)) {
            printk("WARNING: DMA will not be used for memory %s.%d.\n",
                   td_test_data.mem_str, td_test_data.copyno);
        }

        if (soc_mem_cache_get(unit, td_test_data.mem, td_test_data.copyno)) {
            printk("WARNING: All reads from memory %s.%d will come from cache.\n",
                   td_test_data.mem_str, td_test_data.copyno);
        }
    }

    td_test_data_clear(unit);
    if (td_test_data_init(unit, 1) < 0) {
        return -1;
    }

#ifdef BCM_CMICM_SUPPORT
    if(td_test_data.ccm) {
        if (soc_host_ccm_copy(unit, td_test_data.source, td_test_data.dma_ed,
                            td_test_data.count, 0) < 0) {
            test_error(unit, "CCM DMA Error\n");
            return -1;
        }
    } else 
#endif
    /* If not a RO memory, write random data into table and hardware */
    if (!soc_mem_is_readonly(unit, td_test_data.mem)) {
        entbuf = td_test_data.source;
        if (0 != td_test_data.rnum) {
            sal_memset(entbuf, 0xff, WORDS2BYTES(td_test_data.bufwords));
        }

#ifdef BCM_XGS3_SWITCH_SUPPORT
        if (SOC_IS_XGS3_SWITCH(unit) &&
            td_test_data.tslam_en && 
            (soc_mem_slamable(unit, td_test_data.mem, td_test_data.copyno))) {
            for (i = td_test_data.index_min; i <= td_test_data.index_max; ++i) {
                int j;
                td_test_random_entry(unit, td_test_data.mem, entbuf);
                if (td_test_data.accum_tcammask) {
                    /* data read back has dependency on mask */
                    if (soc_feature(unit, soc_feature_xy_tcam)) {
                        for (j = 0; j < td_test_data.entwords; j++) {
                            entbuf[j] |= td_test_data.tcammask[j];
                        }
                    } else if (SOC_BLOCK_TYPE(unit, td_test_data.copyno) ==
                               SOC_BLK_ESM) {
                        for (j = 0; j < td_test_data.entwords; j++) {
                            entbuf[j] &= ~td_test_data.tcammask[j];
                        }
                    }
                } else if (SOC_BLOCK_TYPE(unit, td_test_data.copyno) ==
                           SOC_BLK_ESM) {
                    for (j = 0; j < td_test_data.entwords; j++) {
                        entbuf[j] &= ~td_test_data.tcammask[j];
                    }
                }
                entbuf += td_test_data.entwords;
            }
            soc_cm_sflush(unit, (void *)td_test_data.source,
                          WORDS2BYTES(td_test_data.bufwords));
            if (soc_mem_write_range(unit, td_test_data.mem, td_test_data.copyno,
                                    (td_test_data.tslam_dir ? 
                                    td_test_data.index_min : td_test_data.index_max),
                                    (td_test_data.tslam_dir ? 
                                    td_test_data.index_max : td_test_data.index_min),
                                    td_test_data.source) < 0) {
                test_error(unit,
                   "Table SLAM DMA Error:  Memory %s.%d\n",
                   td_test_data.mem_str, td_test_data.copyno);
                return -1;
            }
        } else 
#endif /* BCM_XGS3_SWITCH_SUPPORT */
#ifdef BCM_SIRIUS_SUPPORT
        if (SOC_IS_SIRIUS(unit) &&
            td_test_data.tslam_en && 
            (soc_mem_slamable(unit, td_test_data.mem, td_test_data.copyno))) {
            for (i = td_test_data.index_min; i <= td_test_data.index_max; ++i) {
                td_test_random_entry(unit, td_test_data.mem, entbuf);
                entbuf += td_test_data.entwords;
            }
            soc_cm_sflush(unit, (void *)td_test_data.source,
                          WORDS2BYTES(td_test_data.bufwords));
            if (soc_mem_write_range(unit, td_test_data.mem, td_test_data.copyno,
                                    (td_test_data.tslam_dir ? 
                                    td_test_data.index_min : td_test_data.index_max),
                                    (td_test_data.tslam_dir ? 
                                    td_test_data.index_max : td_test_data.index_min),
                                    td_test_data.source) < 0) {
                test_error(unit,
                   "Table SLAM DMA Error:  Memory %s.%d\n",
                   td_test_data.mem_str, td_test_data.copyno);
                return -1;
            }
        } else 
#endif /* BCM_SIRIUS_SUPPORT */
        {
            for (i = td_test_data.index_min; i <= td_test_data.index_max; ++i) {
                td_test_random_entry(unit, td_test_data.mem, entbuf);
                if (td_test_data.accum_tcammask) {
                    int j;
                    /* data read back has dependency on mask */
                    if (soc_feature(unit, soc_feature_xy_tcam)) {
                        for (j = 0; j < td_test_data.entwords; j++) {
                            entbuf[j] |= td_test_data.tcammask[j];
                        }
                    } else if (SOC_BLOCK_TYPE(unit, td_test_data.copyno) ==
                               SOC_BLK_ESM) {
                        for (j = 0; j < td_test_data.entwords; j++) {
                            entbuf[j] &= ~td_test_data.tcammask[j];
                        }
                    }
                }
                if (td_test_write_entry(unit, td_test_data.mem,
                                        td_test_data.copyno, i, entbuf) < 0) {
                    return -1;
                }
                entbuf += td_test_data.entwords;
            }
        }
    } else {
        printk("Note:  memory %s is read only, so verifying a few\n",
               td_test_data.mem_str);
        printk("entries directly.\n");
    }

    if (td_test_dma_verify_helper(unit) < 0) {
        return -1;
    }

    return 0;
}

int
td_test_init(int unit, args_t *a, void **pa)
{
    parse_table_t  pt;
    int            index_min, index_max;
    int count_arg;

    if (td_test_data_init(unit, 0) < 0) {
        return -1;
    }

    parse_table_init(unit, &pt);

    parse_table_add(&pt, "Mem", PQ_DFL|PQ_STRING|PQ_STATIC,
                    0, &td_test_data.mem_str, NULL);
    parse_table_add(&pt, "Start", PQ_DFL|PQ_INT,
                    0, &td_test_data.index_min, NULL);
    parse_table_add(&pt, "Count", PQ_DFL|PQ_INT,
                    (void *)(-1), &count_arg, NULL);
    parse_table_add(&pt, "readonlymemVerifyCount", PQ_DFL|PQ_INT,
                    0, &td_test_data.rom_ver_count, NULL);
    parse_table_add(&pt, "SEED",        PQ_INT|PQ_DFL,  0,
                    &td_test_data.rnum,       NULL);
    parse_table_add(&pt, "EccAsData", PQ_BOOL|PQ_DFL,  0,
                    &td_test_data.ecc_as_data, NULL);
#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (SOC_IS_XGS3_SWITCH(unit)) {
        parse_table_add(&pt, "TSlamEnable", PQ_BOOL|PQ_DFL,
                        0, &td_test_data.tslam_en, NULL);
        parse_table_add(&pt, "SlamLowToHigh", PQ_BOOL|PQ_DFL,
                        0, &td_test_data.tslam_dir, NULL);
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */
#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
      parse_table_add(&pt, "SlamLowToHigh", PQ_BOOL|PQ_DFL,
		      0, &td_test_data.tslam_dir,NULL);
    }
#endif /* BCM_SIRIUS_SUPPORT */
#ifdef BCM_CMICM_SUPPORT
    parse_table_add(&pt, "CrossCoupledMemory", PQ_BOOL|PQ_DFL,
                    0, &td_test_data.ccm, NULL);
#endif
#ifdef BCM_XGS3_SWITCH_SUPPORT /* For now implement only for katana */
    if (SOC_IS_KATANA(unit)) {
        parse_table_add(&pt, "SourceAddressOverride", PQ_INT|PQ_DFL,
                        0, &td_test_data.srcaddr, NULL);
        parse_table_add(&pt, "DestAddressOverride", PQ_INT|PQ_DFL,
                        0, &td_test_data.dstaddr, NULL);
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */
    parse_table_add(&pt, "CheckData",   PQ_BOOL|PQ_DFL, 0,
                    &td_test_data.td_check_data, NULL);

    if (parse_arg_eq(a, &pt) < 0 || ARG_CNT(a) != 0) {
        test_error(unit,
                   "%s: Invalid option: %s\n", ARG_CMD(a),
                   ARG_CUR(a) ? ARG_CUR(a) : "*");
        parse_arg_eq_done(&pt);
        return -1;
    }

    if (parse_memory_name(unit, &td_test_data.mem, td_test_data.mem_str,
                          &td_test_data.copyno) < 0) {
        test_error(unit, "Memory \"%s\" is invalid\n",
                   td_test_data.mem_str);
        parse_arg_eq_done(&pt);
        return -1;
    }

    if (count_arg != -1) {
      td_test_data.count = count_arg;
    }

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
      /* if Count is entered use that, otherwise set to zero
       * to allow full range of memory to be tested */
      if (count_arg == -1) {
	td_test_data.count = 0;
      }
    }
#endif /* BCM_SIRIUS_SUPPORT */

    if (td_test_data.srcaddr || td_test_data.dstaddr) {
        test_msg("Overriding buffers can potentially corrupt the system\n");
        test_msg("Don't do it unless you know what you're doing\n");
    }

    if (td_test_data.copyno == COPYNO_ALL) {
        td_test_data.copyno = SOC_MEM_BLOCK_ANY(unit, td_test_data.mem);
    }
    if (!SOC_MEM_BLOCK_VALID(unit, td_test_data.mem, td_test_data.copyno)) {
        test_error(unit,
                   "Invalid copyno %d specified in %s\n",
                   td_test_data.copyno, td_test_data.mem_str);
        parse_arg_eq_done(&pt);
        return -1;
    }

    index_min = soc_mem_index_min(unit, td_test_data.mem);
    index_max = soc_mem_index_max(unit, td_test_data.mem);

    /*
     * Some devices have a last DEFIP entry that is not memory,
     * and that does not have parity
     */
    if (SOC_IS_LYNX(unit) && td_test_data.mem == DEFIPm) {
        index_max--;
    }
    if (SOC_IS_TUCANA(unit) &&
	(td_test_data.mem == DEFIP_HIm || td_test_data.mem == DEFIP_LOm)) {
	index_max--;
    }

    if (td_test_data.index_min <= 0) {
        td_test_data.index_min = index_min;
    }
    if (td_test_data.index_min > index_max) {
        printk("Min index out of range: %d\n",  td_test_data.index_min);
        td_test_data.index_min = index_min;
        printk("Changed to %d\n", td_test_data.index_min);
    }

    if (td_test_data.count <= 0) {
	/*
	 * do not use soc_mem_index_count,index_max may have been,
	 * index_max may have been modified above
	 */
        td_test_data.count = index_max - index_min + 1;
    }

    if (td_test_data.index_min + td_test_data.count - 1 > index_max) {
        td_test_data.count = index_max - td_test_data.index_min + 1;
        printk("Reduced the count to %d\n", td_test_data.count);
    }

    if (td_test_data.count == 0) {
	test_error(unit,
		   "Cannot test memory %s:  No entries.\n",
		   SOC_MEM_UFNAME(unit, td_test_data.mem));
        parse_arg_eq_done(&pt);
        return -1;
    }

    /* for read only memory, number of entries to verify */
    if (td_test_data.rom_ver_count < 0) {
        td_test_data.rom_ver_count = TD_DEFAULT_ROM_VER_COUNT;
    }
    if (td_test_data.rom_ver_count > td_test_data.count) {
        td_test_data.rom_ver_count = td_test_data.count;
    }

    if (soc_mem_parity_control(unit, td_test_data.mem, td_test_data.copyno,
                               FALSE) < 0) {
        test_error(unit, "Could not disable parity warnings on memory %s\n",
                   SOC_MEM_UFNAME(unit, td_test_data.mem));
        parse_arg_eq_done(&pt);
        return -1;
    }

    /*
     * Turn off L2 task to keep it from going crazy if L2 memory is
     * being tested.
     */

#ifdef BCM_XGS_SWITCH_SUPPORT
    if (soc_feature(unit, soc_feature_arl_hashed)) {
        (void)soc_l2x_stop(unit);
    }
#endif

    /*
     * Turn off FFP.
     * Required when testing FFP and METERING memories or tests will fail.
     */

#ifndef BCM_SIRIUS_SUPPORT
    (void)soc_filter_enable_set(unit, 0);
#endif

#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(unit, soc_feature_esm_support)) {
        /* Some tables have dependency on the content of other table */
        switch (td_test_data.mem) {
        case EXT_ACL360_TCAM_DATAm:
        case EXT_ACL360_TCAM_DATA_IPV6_SHORTm:
            if (SOC_FAILURE(soc_mem_clear(unit, EXT_ACL360_TCAM_MASKm,
                MEM_BLOCK_ALL, TRUE)))
            {
                test_error(unit, "Failed to clear EXT_ACL360_TCAM_MASKm\n");
                parse_arg_eq_done(&pt);
                return -1;
            }
            break;
        case EXT_ACL432_TCAM_DATAm:
        case EXT_ACL432_TCAM_DATA_IPV6_LONGm:
        case EXT_ACL432_TCAM_DATA_L2_IPV4m:
        case EXT_ACL432_TCAM_DATA_L2_IPV6m:
            if (SOC_FAILURE(soc_mem_clear(unit, EXT_ACL432_TCAM_MASKm,
                MEM_BLOCK_ALL, TRUE)))
            {
                test_error(unit, "Failed to clear EXT_ACL432_TCAM_MASKm\n");
                parse_arg_eq_done(&pt);
                return -1;
            }
            break;
        default:
            break;
        }
    }
#endif /* BCM_TRIUMPH_SUPPORT */

    td_test_data.index_max = td_test_data.index_min +
        td_test_data.count - 1;

    parse_arg_eq_done(&pt);
    return 0;
}

int
td_test_done(int unit, void *pa)
{
    td_test_data_clear(unit);
#ifdef BCM_CMICM_SUPPORT
    if(td_test_data.ccm) {
        return 0;
    }
#endif

    if (soc_mem_parity_restore(unit, td_test_data.mem,
                               td_test_data.copyno) < 0) {
        test_error(unit, "Could not enable parity warnings on memory %s\n",
                   SOC_MEM_UFNAME(unit, td_test_data.mem));
        return -1;
    }

    return 0;
}

#endif /* BCM_XGS_SWITCH_SUPPORT || BCM_SIRIUS_SUPPORT */
