/*
 * $Id: easyrider.h 1.41 Broadcom SDK $
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
 * File:        easyrider.h
 */

#ifndef _SOC_EASYRIDER_H_
#define _SOC_EASYRIDER_H_

#include <soc/drv.h>
#include <shared/sram.h>

/* External DDR2_SRAM interface */
#define BCM_EXT_SRAM_CSE_MODE     _SHR_EXT_SRAM_CSE_MODE 
#define BCM_EXT_SRAM_HSE_MODE     _SHR_EXT_SRAM_HSE_MODE 

typedef _shr_ext_sram_entry_t er_ext_sram_entry_t;

extern int soc_easyrider_misc_init(int);
extern int soc_easyrider_mmu_init(int);
extern int soc_easyrider_age_timer_get(int, int *, int *);
extern int soc_easyrider_age_timer_max_get(int, int *);
extern int soc_easyrider_age_timer_set(int, int, int);
extern int soc_easyrider_stat_get(int, soc_port_t, int, uint64*);

extern void soc_er_parity_error(void *unit, void *d1, void *d2,
                                void *d3, void *d4 );

extern soc_functions_t soc_easyrider_drv_funs;

extern int soc_easyrider_external_init(int unit);
extern int soc_easyrider_external_l3_enable(int unit);
extern int soc_easyrider_is_external_l3_enabled(int unit);

extern int soc_easyrider_ext_sram_enable_set(int unit, int mode, 
                                             int enable, int clr);
extern int soc_easyrider_ext_sram_op(int unit, int mode, 
                                     er_ext_sram_entry_t *entry);
extern int soc_easyrider_qdr_init(int unit);
extern void soc_easyrider_mem_config(int unit);

extern int soc_er_defip_insert(int unit, defip_alg_entry_t *entry_data);
extern int soc_er_defip_delete(int unit, defip_alg_entry_t *entry_data);
extern int soc_er_defip_lookup(int unit, int *index_ptr,
                               defip_alg_entry_t *entry_data,
                               defip_alg_entry_t *return_data);
extern int soc_er_defip_iterate(int unit, int *index_ptr,
                                defip_alg_entry_t *return_data);

extern int soc_er_fp_meter_table_ext_write(int unit, int index,      
                          fp_meter_table_ext_entry_t *entry_data);
extern int soc_er_fp_ext_read(int unit, int index,
                              fp_external_entry_t *return_data);
extern int soc_er_fp_ext_write(int unit, int index,
                               fp_external_entry_t *entry_data);
extern int soc_er_fp_ext_lookup(int unit, int *index,
                                fp_external_entry_t *entry_data,
                                fp_external_entry_t *return_data);

extern int soc_er_fp_tcam_ext_read(int unit, int index,
                                   fp_tcam_external_entry_t *return_data);
extern int soc_er_fp_tcam_ext_write(int unit, int index,
                                    fp_tcam_external_entry_t *entry_data);

extern int soc_mem_er_search(int unit, soc_mem_t mem, int copyno,
                             int *index_ptr, void *key_data,
                             void *entry_data, int lowest_match);
extern int soc_er_num_cells(int unit, int *num_cells);

typedef int (*soc_er_defip_cb_fn)(int unit, defip_alg_entry_t *lpm_entry, 
                                  int ent_idx, int *test, void *data);
extern int
soc_er_defip_traverse(int unit, soc_er_defip_cb_fn test_cb_fn,
                      soc_er_defip_cb_fn op_cb_fn,
                      void *test_data, void *op_data);

#define SOC_ER_LPM_TRAVERSE_ENTRIES_DEFAULT      100

#define SOC_ER_CELLDATA_CHUNKS        17
#define SOC_ER_CELLDATA_CHUNK_WORDS    8
#define SOC_ER_CELLDATA_SLICES        61
#define SOC_ER_CELLDATA_SLICE_WORDS    3
#define SOC_ER_CELLDATA_SLICE_COLUMNS  4

#define SOC_ER_CELLDATA_SLICE4S        16
#define SOC_ER_CELLDATA_SLICE4_WORDS    9

#define SOC_ER_CELLDATA_CBMP_MASK     0xf

#define SOC_ER_CELLDATA_DIFF_COLUMN_MASK 0x3ffff
#define SOC_ER_CELLDATA_DIFF_S60_C2_MASK 0x3fff0

#define SOC_ER_CELLDATA_DIFF_COLUMN_55   0x15555
#define SOC_ER_CELLDATA_DIFF_COLUMN_AA   0x2aaaa

typedef uint32 ** soc_er_cd_chunk_t;
typedef uint32 ** soc_er_cd_slice_t;
typedef uint32 ** soc_er_cd_columns_t;
typedef uint32 soc_er_cd_slice4_t[SOC_ER_CELLDATA_SLICE4S][SOC_MAX_MEM_WORDS];

extern int soc_er_dual_dimensional_array_alloc(uint32 ***ptr, int raw_size, 
                                               int col_size);
extern int soc_er_dual_dimensional_array_free(uint32 **ptr, int raw_size); 

extern int soc_er_celldata_chunk_read(int unit, soc_mem_t mem, int index,
                                      soc_er_cd_chunk_t chunk_data);
extern void soc_er_celldata_chunk_to_slice(soc_er_cd_chunk_t chunk_data,
                                           soc_er_cd_slice_t slice_data);
extern void soc_er_celldata_slice_to_columns(soc_er_cd_slice_t slice_data,
                                           soc_er_cd_columns_t columns_data);

extern void soc_er_celldata_columns_to_slice(soc_er_cd_columns_t columns_data,
                                             soc_er_cd_slice_t slice_data);
extern void soc_er_celldata_slice_to_chunk(soc_er_cd_slice_t slice_data,
                                           soc_er_cd_chunk_t chunk_data);
extern int soc_er_celldata_chunk_write(int unit, soc_mem_t mem, int index,
                                       soc_er_cd_chunk_t chunk_data);

extern int soc_er_celldata_columns_test(int unit, soc_mem_t mem, int index,
                                      int do_write, int do_read,
                                      soc_er_cd_columns_t columns_data,
                                      soc_er_cd_columns_t columns_result,
                                      soc_er_cd_columns_t columns_diff);

/* CMIC backpressure blocking */
#define SOC_ER_MMU_LIMITS_PTR_LOCAL      0       /* Reserved for local ports */
#define SOC_ER_MMU_LIMITS_PTR_REMOTE     1       /* Reserved for remote modules */
#define SOC_ER_MMU_LIMITS_PTR_CMIC       7       /* Reserved for CMIC use */

/* External memory */
#define ER_SEER_CFG_NO_EXT                              0
#define ER_SEER_CFG_L2_512_EXT                          1
#define ER_SEER_CFG_LPM_256_EXT                         2
#define ER_SEER_CFG_L4_192_EXT                          3
#define ER_SEER_CFG_L4_96_EXT                           4
#define ER_SEER_CFG_LPM_256_L4_128_EXT                  5
#define ER_SEER_CFG_LPM_384_L4_64_EXT                   6
#define ER_SEER_CFG_LPM_128_L4_64_EXT                   7
#define ER_SEER_CFG_LPM_192_L4_32_EXT                   8
#define ER_SEER_CFG_LPM_448_EXT                         9
#define ER_SEER_CFG_LPM_896_EXT                         10

#define ER_SEER_2K        (2 * 1024)
#define ER_SEER_4K        (4 * 1024)
#define ER_SEER_8K        (8 * 1024)
#define ER_SEER_12K       (12 * 1024)
#define ER_SEER_16K       (16 * 1024)
#define ER_SEER_32K       (32 * 1024)
#define ER_SEER_48K       (48 * 1024)
#define ER_SEER_64K       (64 * 1024)
#define ER_SEER_96K       (96 * 1024)
#define ER_SEER_128K      (128 * 1024)
#define ER_SEER_192K      (192 * 1024)
#define ER_SEER_256K      (256 * 1024)
#define ER_SEER_384K      (384 * 1024)
#define ER_SEER_448K      (498 * 1024)
#define ER_SEER_512K      (512 * 1024)
#define ER_SEER_896K      (896 * 1024)

/* L2 internal & L3 memory */
#define ER_SEER_HOST_HASH_CFG_L2_ALL                    0
#define ER_SEER_HOST_HASH_CFG_L2_HALF_V4_HALF           1
#define ER_SEER_HOST_HASH_CFG_L2_HALF_V4_QUART_V6_QUART 2
#define ER_SEER_HOST_HASH_CFG_L2_HALF_V6_HALF           3
#define ER_SEER_HOST_HASH_CFG_L2_QUART_V4_HALF_V6_QUART 4
#define ER_SEER_HOST_HASH_CFG_L2_QUART_V4_QUART_V6_HALF 5
#define ER_SEER_HOST_HASH_CFG_V4_ALL                    6
#define ER_SEER_HOST_HASH_CFG_V4_HALF_V6_HALF           7
#define ER_SEER_HOST_HASH_CFG_V6_ALL                    8

/* MAC VLAN and My Station memory */
#define ER_SEER_MVL_HASH_CFG_MVL_ALL                    0
#define ER_SEER_MVL_HASH_CFG_MVL_HALF_MYST_HALF         1
#define ER_SEER_MVL_HASH_CFG_MYST_ALL                   2

/* TCAM selectors */
#define ER_EXT_TCAM_NONE    0
#define ER_EXT_TCAM_TYPE1   1
#define ER_EXT_TCAM_TYPE2   2

/* DEFIP overlay types */
#define SOC_ER_DEFIP_FF_FIELD           0xff
#define SOC_ER_DEFIP_KEY_TYPE_V4        4
#define SOC_ER_DEFIP_KEY_TYPE_MPLS_1L   6
#define SOC_ER_DEFIP_KEY_TYPE_MPLS_2L   7

#define SOC_ER_DEFIP_ALG_INT_REGIONS    8
#define SOC_ER_DEFIP_ALG_EXT_REGIONS    16

/* Parity error decoding */
#define SOC_ER_PARITY_BSE_L3_DEFIP_ALG           0x001
#define SOC_ER_PARITY_BSE_L3_DEFIP_ALG_EXT       0x002
#define SOC_ER_PARITY_BSE_VRF_VFI_INTF           0x004
#define SOC_ER_PARITY_BSE_IGR_VLAN_XLATE         0x008
#define SOC_ER_PARITY_BSE_EGR_VLAN_XLATE         0x010

#define SOC_ER_PARITY_CSE_NEXT_HOP_INT           0x001
#define SOC_ER_PARITY_CSE_NEXT_HOP_EXT           0x002
#define SOC_ER_PARITY_CSE_L3_INTF_TABLE          0x004
#define SOC_ER_PARITY_CSE_FP_EXTERNAL_TCAM       0x008
#define SOC_ER_PARITY_CSE_FP_POLICY_TABLE_EXT    0x010
#define SOC_ER_PARITY_CSE_FP_METER_TABLE_EXT     0x020
#define SOC_ER_PARITY_CSE_FP_COUNTER_EXT         0x040
#define SOC_ER_PARITY_CSE_L3_DEFIP_DATA          0x080

#define SOC_ER_PARITY_HSE_L2_ENTRY        0x001
#define SOC_ER_PARITY_HSE_L2_ENTRY_EXT    0x002
#define SOC_ER_PARITY_HSE_L3_ENTRY_V4     0x004
#define SOC_ER_PARITY_HSE_L3_ENTRY_V6     0x008
#define SOC_ER_PARITY_HSE_L3_LPM_HITBIT   0x010
#define SOC_ER_PARITY_HSE_IPMC_GROUP_V4   0x020
#define SOC_ER_PARITY_HSE_IPMC_GROUP_V6   0x040

#define SOC_ER_PARITY_MMU_ASM_PKTID_ERR                         0x0000002
#define SOC_ER_PARITY_MMU_ASM_PARITY_ERR                        0x0000004
#define SOC_ER_PARITY_MMU_XQ_PARITY_ERR                         0x0000008
#define SOC_ER_PARITY_MMU_MSYS_PTR_RELEASE_MGR_ERR              0x0000010
#define SOC_ER_PARITY_MMU_MSYS_PTR_CTRL0_ERR                    0x0000020
#define SOC_ER_PARITY_MMU_MSYS_PTR_CTRL1_ERR                    0x0000040
#define SOC_ER_PARITY_MMU_MSYS_COPYCNT_PTR_ERR                  0x0000080
#define SOC_ER_PARITY_MMU_MSYS_COPYCNT_COUNT_ERR                0x0000100
#define SOC_ER_PARITY_MMU_MSYS_CELL_0_ERR                       0x0000200
#define SOC_ER_PARITY_MMU_MSYS_PKT_0_ERR                        0x0000400
#define SOC_ER_PARITY_MMU_MSYS_NXTPTR_0_ERR                     0x0000800
#define SOC_ER_PARITY_MMU_MSYS_PTR_BLOCK_0_ERR                  0x0001000
#define SOC_ER_PARITY_MMU_MSYS_CELL_1_ERR                       0x0002000
#define SOC_ER_PARITY_MMU_MSYS_PKT_1_ERR                        0x0004000
#define SOC_ER_PARITY_MMU_MSYS_NXTPTR_1_ERR                     0x0008000
#define SOC_ER_PARITY_MMU_MSYS_PTR_BLOCK_1_ERR                  0x0010000
#define SOC_ER_PARITY_MMU_MSYS_INGBUF_OVERFLOW_ERR              0x0020000
#define SOC_ER_PARITY_MMU_MSYS_INGBUF_CELL_INCONSISTENCY_ERR    0x0040000
#define SOC_ER_PARITY_MMU_MSYS_IPMC_PTR_ERR                     0x0080000
#define SOC_ER_PARITY_MMU_MSYS_IPMC_IF_NO_ERR                   0x0100000
#define SOC_ER_PARITY_MMU_MSYS_ING_STAT_ERR                     0x0200000
#define SOC_ER_PARITY_MMU_MCU_REQ_FIFO_ERR                      0x0400000
#define SOC_ER_PARITY_MMU_ASM_FIFO_ERR                          0x0800000
#define SOC_ER_PARITY_MMU_ASM_IGMU_ERR                          0x1000000


/* Age timer definitions */
#define SOC_ER_AGE_TIMER_RESOLUTION_1S          0
#define SOC_ER_AGE_TIMER_RESOLUTION_100MS       1
#define SOC_ER_AGE_TIMER_RESOLUTION_1MIN        2
#define SOC_ER_AGE_TIMER_RESOLUTION_5MIN        3

#define SOC_ER_IPMC_GROUP_TBL_BLK_NUM           8
#define SOC_ER_IPMC_GROUP_TBL_VALID_MASK        0xff
#define SOC_ER_IPMC_GROUP_V4_TEST_MAPPING       0x76543210
#define SOC_ER_IPMC_GROUP_V6_TEST_MAPPING       0x01234567

#endif	/* !_SOC_EASYRIDER_H_ */
