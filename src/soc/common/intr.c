/*
 * $Id: intr.c 1.16 Broadcom SDK $
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
 * SOC Interrupt Handlers
 *
 * NOTE: These handlers are called from an interrupt context, so their
 *       actions are restricted accordingly.
 */

#include <sal/core/libc.h>
#include <shared/alloc.h>
#include <sal/core/spl.h>
#include <sal/core/sync.h>
#include <sal/core/dpc.h>

#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/dma.h>
#include <soc/i2c.h>

#ifdef BCM_LYNX_SUPPORT
#include <soc/lynx.h>
#endif

#ifdef BCM_TUCANA_SUPPORT
#include <soc/tucana.h>
#endif

#ifdef BCM_FIREBOLT_SUPPORT
#include <soc/firebolt.h>
#endif /* BCM_FIREBOLT_SUPPORT */

#ifdef BCM_EASYRIDER_SUPPORT
#include <soc/easyrider.h>
#endif /* BCM_EASYRIDER_SUPPORT */

#ifdef BCM_BRADLEY_SUPPORT
#include <soc/bradley.h>
#endif /* BCM_BRADLEY_SUPPORT */

#ifdef BCM_TRIUMPH_SUPPORT
#include <soc/triumph.h>
#endif /* BCM_TRIUMPH_SUPPORT */

#ifdef BCM_TRIUMPH2_SUPPORT
#include <soc/triumph2.h>
#endif /* BCM_TRIUMPH2_SUPPORT */

#ifdef BCM_TRIDENT_SUPPORT
#include <soc/trident.h>
#endif /* BCM_TRIDENT_SUPPORT */

#ifdef BCM_ENDURO_SUPPORT
#include <soc/enduro.h>
#endif /* BCM_ENDURO_SUPPORT */

#ifdef BCM_HURRICANE_SUPPORT
#include <soc/hurricane.h>
#endif /* BCM_HURRICANE_SUPPORT */

#ifdef BCM_SIRIUS_SUPPORT
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sirius.h>
#endif

#ifdef INCLUDE_KNET
#include <soc/knet.h>
#define IRQ_MASK_SET(_u,_a,_m) soc_knet_irq_mask_set(_u,_a,_m)
#else
#define IRQ_MASK_SET(_u,_a,_m) soc_pci_write(_u,_a,_m)
#endif

#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)

/* Declare static functions for interrupt handler array */
STATIC void soc_intr_schan_done(int unit, uint32 ignored);
STATIC void soc_intr_pci_parity(int unit, uint32 ignored);
STATIC void soc_intr_pci_fatal(int unit, uint32 ignored);
STATIC void soc_intr_link_stat(int unit, uint32 ignored);
STATIC void soc_intr_gbp_full(int unit, uint32 ignored);
STATIC void soc_intr_arl_xfer(int unit, uint32 ignored);
STATIC void soc_intr_arl_cnt0(int unit, uint32 ignored);
STATIC void soc_intr_arl_drop(int unit, uint32 ignored);
STATIC void soc_intr_arl_mbuf(int unit, uint32 ignored);
STATIC void soc_intr_schan_error(int unit, uint32 ignored);
STATIC void soc_intr_i2c(int unit, uint32 ignored);
STATIC void soc_intr_miim_op(int unit, uint32 ignored);
STATIC void soc_intr_stat_dma(int unit, uint32 ignored);
STATIC void soc_intr_bit21(int unit, uint32 ignored);
STATIC void soc_intr_bit22(int unit, uint32 ignored);
STATIC void soc_intr_bit23(int unit, uint32 ignored);
#ifdef BCM_HERCULES_SUPPORT
STATIC void soc_intr_mmu_stat(int unit, uint32 ignored);
#endif
#if defined(BCM_XGS12_SWITCH_SUPPORT)
STATIC void soc_intr_arl_error(int unit, uint32 ignored);
#endif
STATIC void soc_intr_lpm_lo_parity(int unit, uint32 ignored);
STATIC void soc_intr_bit25(int unit, uint32 ignored);
STATIC void soc_intr_bit26(int unit, uint32 ignored);
STATIC void soc_intr_bit27(int unit, uint32 ignored);
STATIC void soc_intr_bit28(int unit, uint32 ignored);
STATIC void soc_intr_bit31(int unit, uint32 ignored);
STATIC void soc_intr_tdma_done(int unit, uint32 ignored);
STATIC void soc_intr_tslam_done(int unit, uint32 ignored);
STATIC void soc_intr_block(int unit, uint32 block);
#ifdef BCM_CMICM_SUPPORT
extern void soc_cmicm_intr(int unit);
#endif

/*
 * SOC Interrupt Table
 *
 * The table is stored in priority order:  Interrupts that are listed
 * first have their handlers called first.
 *
 * A handler can clear more than one interrupt bit to prevent a
 * subsequent handler from being called.  E.g., if the DMA CHAIN_DONE
 * handler clears both CHAIN_DONE and DESC_DONE, the DESC_DONE handler
 * will not be called.
 */

typedef void (*ifn_t)(int unit, uint32 data);

typedef struct {
    uint32	mask;
    ifn_t	intr_fn;
    uint32	intr_data;
    char	*intr_name;
} intr_handler_t;

STATIC intr_handler_t soc_intr_handlers[] = {

 /* Errors (Highest priority) [0..3] */

 { IRQ_PCI_PARITY_ERR,	soc_intr_pci_parity,	0, "PCI_PARITY_ERR"	},
 { IRQ_PCI_FATAL_ERR,	soc_intr_pci_fatal,	0, "PCI_FATAL_ERR"	},
 { IRQ_SCHAN_ERR,	soc_intr_schan_error,	0, "SCHAN_ERR"		},
 { IRQ_GBP_FULL,	soc_intr_gbp_full,	0, "GBP_FULL"		},

 /* S-Channel [4] */

 { IRQ_SCH_MSG_DONE,	soc_intr_schan_done,	0, "SCH_MSG_DONE"	},

 /* MII [5-6] */

 { IRQ_MIIM_OP_DONE,	soc_intr_miim_op,	0, "MIIM_OP_DONE"	},
 { IRQ_LINK_STAT_MOD,	soc_intr_link_stat,	0, "LINK_STAT_MOD"	},

 /* ARL messages [7-10] */

 { IRQ_ARL_MBUF,	soc_intr_arl_mbuf,	0, "ARL_MBUF"		},
 { IRQ_ARL_MBUF_DROP,	soc_intr_arl_drop,	0, "ARL_MBUF_DROP"	},
 { IRQ_ARL_DMA_CNT0,	soc_intr_arl_cnt0,	0, "ARL_DMA_CNT0"	},
 { IRQ_ARL_DMA_XFER,	soc_intr_arl_xfer,	0, "ARL_DMA_XFER"	},

 /* TDMA/TSLAM [11-12] */
 { IRQ_TDMA_DONE,	soc_intr_tdma_done,	0, "TDMA_DONE"	        },
 { IRQ_TSLAM_DONE,	soc_intr_tslam_done,	0, "TSLAM_DONE"	        },

 /* Packet DMA [13-20] */

 { IRQ_CHAIN_DONE(0),	soc_dma_done_chain,	0, "CH0_CHAIN_DONE"	},
 { IRQ_CHAIN_DONE(1),	soc_dma_done_chain,	1, "CH1_CHAIN_DONE"	},
 { IRQ_CHAIN_DONE(2),	soc_dma_done_chain,	2, "CH2_CHAIN_DONE"	},
 { IRQ_CHAIN_DONE(3),	soc_dma_done_chain,	3, "CH3_CHAIN_DONE"	},

 { IRQ_DESC_DONE(0),	soc_dma_done_desc,	0, "CH0_DESC_DONE"	},
 { IRQ_DESC_DONE(1),	soc_dma_done_desc,	1, "CH1_DESC_DONE"	},
 { IRQ_DESC_DONE(2),	soc_dma_done_desc,	2, "CH2_DESC_DONE"	},
 { IRQ_DESC_DONE(3),	soc_dma_done_desc,	3, "CH3_DESC_DONE"	},

 /* Other (lowest priority) [21-28] */

 { IRQ_BIT21,		soc_intr_bit21,		0, "MMU_IRQ_STAT"	},
 { IRQ_BIT22,		soc_intr_bit22,		0, "IRQ_FIFO_CH1_DMA"	},
 { IRQ_BIT23,		soc_intr_bit23,		0, "IRQ_FIFO_CH2_DMA"	},
 { IRQ_STAT_ITER_DONE,	soc_intr_stat_dma,	0, "STAT_ITER_DONE"	},
 { IRQ_I2C_INTR,	soc_intr_i2c,		0, "I2C_INTR"		},
 { IRQ_ARL_LPM_LO_PAR,	soc_intr_lpm_lo_parity,	0, "LPM_LO_PARITY"	},
 { IRQ_BIT25,           soc_intr_bit25,	        0, "LPM_HI_PARITY/BSE"	},
 { IRQ_BIT26,	        soc_intr_bit26,	        0, "L3_PARITY/CSE"      },
 { IRQ_BIT27,	        soc_intr_bit27,	        0, "L2_PARITY/HSE"      },
 { IRQ_BIT28,           soc_intr_bit28,         0, "VLAN_PARITY/MEMFAIL"},
 { IRQ_BROADSYNC_INTR,  soc_intr_bit31,         0, "BSAFE_OP_DONE/BROADSYNC_INTR"},

};

#define INTR_HANDLERS_COUNT	COUNTOF(soc_intr_handlers)

/*
 * define some short cuts to start processing interrupts quickly
 * start2: skip to packet processing
 * start1: skip low probability errors
 * else start at 0
 */
#define	INTR_START1_MASK	(IRQ_PCI_PARITY_ERR | \
				IRQ_PCI_FATAL_ERR | \
				IRQ_SCHAN_ERR | \
				IRQ_GBP_FULL)
#define	INTR_START1_POS		4
#define	INTR_START2_MASK	(INTR_START1_MASK | \
				IRQ_SCH_MSG_DONE | \
				IRQ_MIIM_OP_DONE | \
				IRQ_LINK_STAT_MOD | \
				IRQ_ARL_MBUF | \
				IRQ_ARL_MBUF_DROP | \
				IRQ_ARL_DMA_CNT0 | \
				IRQ_ARL_DMA_XFER)
#define	INTR_START2_POS		11

STATIC intr_handler_t soc_intr_block_lo_handlers[] = {
 { IRQ_LO_BLOCK_0,	soc_intr_block,	0,  "BLOCK_0_ERR"	},
 { IRQ_LO_BLOCK_1,	soc_intr_block,	1,  "BLOCK_1_ERR"	},
 { IRQ_LO_BLOCK_2,	soc_intr_block,	2,  "BLOCK_2_ERR"	},
 { IRQ_LO_BLOCK_3,	soc_intr_block,	3,  "BLOCK_3_ERR"	},
 { IRQ_LO_BLOCK_4,	soc_intr_block,	4,  "BLOCK_4_ERR"	},
 { IRQ_LO_BLOCK_5,	soc_intr_block,	5,  "BLOCK_5_ERR"	},
 { IRQ_LO_BLOCK_6,	soc_intr_block,	6,  "BLOCK_6_ERR"	},
 { IRQ_LO_BLOCK_7,	soc_intr_block,	7,  "BLOCK_7_ERR"	},
 { IRQ_LO_BLOCK_8,	soc_intr_block,	8,  "BLOCK_8_ERR"	},
 { IRQ_LO_BLOCK_9,	soc_intr_block,	9,  "BLOCK_9_ERR"	},
 { IRQ_LO_BLOCK_10,	soc_intr_block,	10, "BLOCK_10_ERR"	},
 { IRQ_LO_BLOCK_11,	soc_intr_block,	11, "BLOCK_11_ERR"	},
 { IRQ_LO_BLOCK_12,	soc_intr_block,	12, "BLOCK_12_ERR"	},
 { IRQ_LO_BLOCK_13,	soc_intr_block,	13, "BLOCK_13_ERR"	},
 { IRQ_LO_BLOCK_14,	soc_intr_block,	14, "BLOCK_14_ERR"	},
 { IRQ_LO_BLOCK_15,	soc_intr_block,	15, "BLOCK_15_ERR"	},
 { IRQ_LO_BLOCK_16,	soc_intr_block,	16, "BLOCK_16_ERR"	},
 { IRQ_LO_BLOCK_17,	soc_intr_block,	17, "BLOCK_17_ERR"	},
 { IRQ_LO_BLOCK_18,	soc_intr_block,	18, "BLOCK_18_ERR"	},
 { IRQ_LO_BLOCK_19,	soc_intr_block,	19, "BLOCK_19_ERR"	},
 { IRQ_LO_BLOCK_20,	soc_intr_block,	20, "BLOCK_20_ERR"	},
 { IRQ_LO_BLOCK_21,	soc_intr_block,	21, "BLOCK_21_ERR"	},
 { IRQ_LO_BLOCK_22,	soc_intr_block,	22, "BLOCK_22_ERR"	},
 { IRQ_LO_BLOCK_23,	soc_intr_block,	23, "BLOCK_23_ERR"	},
 { IRQ_LO_BLOCK_24,	soc_intr_block,	24, "BLOCK_24_ERR"	},
 { IRQ_LO_BLOCK_25,	soc_intr_block,	25, "BLOCK_25_ERR"	},
 { IRQ_LO_BLOCK_26,	soc_intr_block,	26, "BLOCK_26_ERR"	},
 { IRQ_LO_BLOCK_27,	soc_intr_block,	27, "BLOCK_27_ERR"	},
 { IRQ_LO_BLOCK_28,	soc_intr_block,	28, "BLOCK_28_ERR"	},
 { IRQ_LO_BLOCK_29,	soc_intr_block,	29, "BLOCK_29_ERR"	},
 { IRQ_LO_BLOCK_30,	soc_intr_block,	30, "BLOCK_30_ERR"	},
 { IRQ_LO_BLOCK_31,	soc_intr_block,	31, "BLOCK_31_ERR"	},
};
STATIC intr_handler_t soc_intr_block_hi_handlers[] = {
 { IRQ_HI_BLOCK_32,	soc_intr_block,	32, "BLOCK_32_ERR"	},
 { IRQ_HI_BLOCK_33,	soc_intr_block,	33, "BLOCK_33_ERR"	},
 { IRQ_HI_BLOCK_34,	soc_intr_block,	34, "BLOCK_34_ERR"	},
 { IRQ_HI_BLOCK_35,	soc_intr_block,	35, "BLOCK_35_ERR"	},
 { IRQ_HI_BLOCK_36,	soc_intr_block,	36, "BLOCK_36_ERR"	},
 { IRQ_HI_BLOCK_37,	soc_intr_block,	37, "BLOCK_37_ERR"	},
 { IRQ_HI_BLOCK_38,	soc_intr_block,	38, "BLOCK_38_ERR"	},
 { IRQ_HI_BLOCK_39,	soc_intr_block,	39, "BLOCK_39_ERR"	},
 { IRQ_HI_BLOCK_40,	soc_intr_block,	40, "BLOCK_40_ERR"	},
 { IRQ_HI_BLOCK_41,	soc_intr_block,	41, "BLOCK_41_ERR"	},
 { IRQ_HI_BLOCK_42,	soc_intr_block,	42, "BLOCK_42_ERR"	},
 { IRQ_HI_BLOCK_43,	soc_intr_block,	43, "BLOCK_43_ERR"	},
 { IRQ_HI_BLOCK_44,	soc_intr_block,	44, "BLOCK_44_ERR"	},
 { IRQ_HI_BLOCK_45,	soc_intr_block,	45, "BLOCK_45_ERR"	},
 { IRQ_HI_BLOCK_46,	soc_intr_block,	46, "BLOCK_46_ERR"	},
 { IRQ_HI_BLOCK_47,	soc_intr_block,	47, "BLOCK_47_ERR"	},
 { IRQ_HI_BLOCK_48,	soc_intr_block,	48, "BLOCK_48_ERR"	},
 { IRQ_HI_BLOCK_49,	soc_intr_block,	49, "BLOCK_49_ERR"	},
 { IRQ_HI_BLOCK_50,	soc_intr_block,	50, "BLOCK_50_ERR"	},
 { IRQ_HI_BLOCK_51,	soc_intr_block,	51, "BLOCK_51_ERR"	},
 { IRQ_HI_BLOCK_52,	soc_intr_block,	52, "BLOCK_52_ERR"	},
 { IRQ_HI_BLOCK_53,	soc_intr_block,	53, "BLOCK_53_ERR"	},
 { IRQ_HI_BLOCK_54,	soc_intr_block,	54, "BLOCK_54_ERR"	},
 { IRQ_HI_BLOCK_55,	soc_intr_block,	55, "BLOCK_55_ERR"	},
 { IRQ_HI_BLOCK_56,	soc_intr_block,	56, "BLOCK_56_ERR"	},
 { IRQ_HI_BLOCK_57,	soc_intr_block,	57, "BLOCK_57_ERR"	},
 { IRQ_HI_BLOCK_58,	soc_intr_block,	58, "BLOCK_58_ERR"	},
 { IRQ_HI_BLOCK_59,	soc_intr_block,	59, "BLOCK_59_ERR"	},
 { IRQ_HI_BLOCK_60,	soc_intr_block,	60, "BLOCK_60_ERR"	},
 { IRQ_HI_BLOCK_61,	soc_intr_block,	61, "BLOCK_61_ERR"	},
 { IRQ_HI_BLOCK_62,	soc_intr_block,	62, "BLOCK_62_ERR"	},
 { IRQ_HI_BLOCK_63,	soc_intr_block,	63, "BLOCK_63_ERR"	},
};
#define INTR_BLOCK_LO_HANDLERS_COUNT	COUNTOF(soc_intr_block_lo_handlers)
#define INTR_BLOCK_HI_HANDLERS_COUNT	COUNTOF(soc_intr_block_hi_handlers)


/*
 * Interrupt handler functions
 */

STATIC void
soc_intr_schan_done(int unit, uint32 ignored)
{
    soc_control_t	*soc = SOC_CONTROL(unit);

    COMPILER_REFERENCE(ignored);

    /* Record the schan control regsiter */
    soc->schan_result = soc_pci_read(unit, CMIC_SCHAN_CTRL);

    soc->stat.intr_sc++;

    soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MSG_DONE_CLR);

    if (soc->schanIntr) {
	sal_sem_give(soc->schanIntr);
    }
}

STATIC soc_schan_err_t
soc_schan_error_type(int unit, int err_code)
{
    int			bitcount = 0;
    soc_schan_err_t	err = SOC_SCERR_INVALID;

    switch (SOC_CHIP_GROUP(unit)) {
    case SOC_CHIP_BCM5673:
    case SOC_CHIP_BCM5674:
        if (err_code & 0x10) {
            err = SOC_SCERR_MMU_NPKT_CELLS;
            ++bitcount;
        }
	if (err_code & 0x20) {
            err = SOC_SCERR_MEMORY_PARITY;
            ++bitcount;
        }
        /* Fall through */
    case SOC_CHIP_BCM5690:
    case SOC_CHIP_BCM5695:
        if (err_code & 0x1) {
            err = SOC_SCERR_CFAP_OVER_UNDER;
            ++bitcount;
        }
	if (err_code & 0x2) {
            err = SOC_SCERR_MMU_SOFT_RST;
            ++bitcount;
        }
	if (err_code & 0x4) {
            err = SOC_SCERR_CBP_CELL_CRC;
            ++bitcount;
        }
	if (err_code & 0x8) {
            err = SOC_SCERR_CBP_HEADER_PARITY;
            ++bitcount;
        }
        break;
    case SOC_CHIP_BCM5665:
    case SOC_CHIP_BCM5650:
        if (err_code & 0x1) {
            err = SOC_SCERR_CELL_PTR_CRC;
            ++bitcount;
        }
	if (err_code & 0x2) {
            err = SOC_SCERR_CELL_DATA_CRC;
            ++bitcount;
        }
	if (err_code & 0x4) {
            err = SOC_SCERR_FRAME_DATA_CRC;
            ++bitcount;
        }
	if (err_code & 0x8) {
            err = SOC_SCERR_CELL_PTR_BLOCK_CRC;
            ++bitcount;
        }
	if (err_code & 0x10) {
            err = SOC_SCERR_MEMORY_PARITY;
            ++bitcount;
        }
	if (err_code & 0x20) {
            err = SOC_SCERR_PLL_DLL_LOCK_LOSS;
            ++bitcount;
        }
        break;
    default:
        break;
    }

    if (bitcount > 1) {
        err = SOC_SCERR_MULTIPLE_ERR;
    }

    return err;
}

STATIC void
_soc_intr_mem_parity_analyze(void *p_unit, void *p_src, void *p_mem, 
                             void *p_mf, void *p5)
{
    int		blk;
    int         unit = PTR_TO_INT(p_unit);
    int         mem = PTR_TO_INT(p_mem);
    uint32      mf = PTR_TO_INT(p_mf);
    int         value = PTR_TO_INT(p_src);

    COMPILER_REFERENCE(p5);
    COMPILER_REFERENCE(mem);
    COMPILER_REFERENCE(mf);

    for (blk = 0; SOC_BLOCK_INFO(unit, blk).type >= 0; blk++) {
	if (value == SOC_BLOCK_INFO(unit, blk).schan) {
	    break;
	}
    }

#ifdef BCM_LYNX_SUPPORT
    if (SOC_IS_LYNX(unit)) {
        switch (SOC_BLOCK_INFO(unit, blk).type) {
        case SOC_BLK_ARL:
            soc_lynx_arl_parity_error(unit, blk);
            break;
        case SOC_BLK_MMU:
            soc_lynx_mmu_parity_error(unit, blk);
            break;
        case SOC_BLK_XPIC:
            soc_lynx_xpic_parity_error(unit, blk);
            break;
        default:
            /* some other interrupt or src not found */
            break;
        }
  } else 
#endif
#ifdef BCM_TUCANA_SUPPORT
         if (SOC_IS_TUCANA(unit)) {
        switch (SOC_BLOCK_INFO(unit, blk).type) {
        case SOC_BLK_ARL:
            soc_tucana_arl_parity_error(unit, blk, mem);
            break;
        case SOC_BLK_MMU:
            soc_tucana_mmu_parity_error(unit, blk, mf);
            break;
        default:
            /* some other interrupt or src not found */
            break;
        }
    } else 
#endif
           {
        /* Unknown chip */
        soc_cm_debug(DK_ERR,
                     "UNIT %d SCHAN ERROR: Memory parity error\n",
                     unit);
    }
}

#ifdef BCM_TUCANA_SUPPORT

STATIC void
_soc_intr_mem_crc_analyze(void *p_unit, void *p2, void *p3, 
                          void *p_mf, void *p5)
{
    int         unit = PTR_TO_INT(p_unit);
    uint32      mf = PTR_TO_INT(p_mf);

    COMPILER_REFERENCE(p2);
    COMPILER_REFERENCE(p3);
    COMPILER_REFERENCE(p5);
    COMPILER_REFERENCE(mf);

#ifdef BCM_TUCANA_SUPPORT
    if (SOC_IS_TUCANA(unit)) {
        soc_tucana_mmu_crc_error(unit, mf);
    } else 
#endif
           {
        /* Unknown chip */
        soc_cm_debug(DK_ERR,
                     "UNIT %d SCHAN ERROR: Memory CRC error\n",
                     unit);
    }
}

#endif /* BCM_TUCANA_SUPPORT */

STATIC void
_soc_intr_mem_pdll_analyze(void *p_unit, void *p2, void *p3, 
                             void *p_mf, void *p5)
{
    int         unit = PTR_TO_INT(p_unit);
    uint32      mf = PTR_TO_INT(p_mf);

    COMPILER_REFERENCE(p2);
    COMPILER_REFERENCE(p3);
    COMPILER_REFERENCE(p5);
    COMPILER_REFERENCE(mf);

#ifdef BCM_TUCANA_SUPPORT
    if (SOC_IS_TUCANA(unit)) {
        soc_tucana_pdll_lock_loss(unit, mf);
    } else 
#endif
           {
        /* Unknown chip */
        soc_cm_debug(DK_ERR,
                     "UNIT %d SCHAN ERROR: PLL/DLL lock loss error\n",
                     unit);
    }
}

STATIC void
_soc_sch_error_unblock(void *p_unit, void *p2, void *p3, void *p4, void *p5)
{
    COMPILER_REFERENCE(p2);
    COMPILER_REFERENCE(p3);
    COMPILER_REFERENCE(p4);
    COMPILER_REFERENCE(p5);

    soc_intr_enable(PTR_TO_INT(p_unit), IRQ_SCHAN_ERR);
}

STATIC void
soc_intr_schan_error(int unit, uint32 ignored)
{
    soc_control_t	*soc = SOC_CONTROL(unit);
    uint32		scerr, slot;
    int			vld, src, dst, opc, err;

    COMPILER_REFERENCE(ignored);

    /*
     * Read the beginning of the S-chan message so its contents are
     * visible when a PCI bus analyzer is connected.
     */

    soc_pci_analyzer_trigger(unit);

    if (soc_cm_debug_check(DK_INTR)) {
        slot = soc_pci_read(unit, 0);
        slot = soc_pci_read(unit, 4);
        slot = soc_pci_read(unit, 8);
        slot = soc_pci_read(unit, 0xC);
    }

    scerr = soc_pci_read(unit, CMIC_SCHAN_ERR);	/* Clears intr */
    soc_pci_write(unit, CMIC_SCHAN_ERR, 0); /* Clears intr in Lynx*/

    soc->stat.intr_sce++;

    /*
     * If the valid bit is not set, it's probably because the error
     * occurred at the same time the software was starting an unrelated
     * S-channel operation.  There is no way to prevent this conflict.
     * We'll indicate that that the valid bit was not set and continue,
     * since the error is probably still latched.
     */

    vld = soc_reg_field_get(unit, CMIC_SCHAN_ERRr, scerr,
           (SOC_IS_TUCANA(unit) || SOC_IS_XGS3_SWITCH(unit) || SOC_IS_SIRIUS(unit)) ? ERRBITf : VALIDf);
    src = soc_reg_field_get(unit, CMIC_SCHAN_ERRr, scerr, SRC_PORTf);
    dst = soc_reg_field_get(unit, CMIC_SCHAN_ERRr, scerr, DST_PORTf);
    opc = soc_reg_field_get(unit, CMIC_SCHAN_ERRr, scerr, OP_CODEf);
    err = soc_reg_field_get(unit, CMIC_SCHAN_ERRr, scerr, ERR_CODEf);

    /*
     * Output message in two pieces because on vxWorks, it goes through
     * logMsg which only supports up to 6 arguments (sigh).
     */

    if ((!soc->mmu_error_block) || (opc != MEMORY_FAIL_NOTIFY)) {
        soc_cm_debug(DK_ERR,
                     "UNIT %d SCHAN ERROR: V/E=%d SRC=%d DST=%d ",
                     unit, vld, src, dst);

        soc_cm_debug(DK_ERR,
                     "OPCODE=%d(%s) ERRCODE=0x%x\n",
                     opc, soc_schan_op_name(opc), err);
    }

#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        switch(err) {
        case 3:
            soc_cm_debug(DK_ERR,
                    "UNIT %d SCHAN ERROR: Egress block\n",
                    unit);
            break;
        default:
            soc_cm_debug(DK_ERR,
                    "UNIT %d SCHAN ERROR: Unknown reason\n",
                    unit);
            break;
        }
    } else
#endif
#ifdef BCM_XGS3_SWITCH_SUPPORT
        if (SOC_IS_XGS3_SWITCH(unit)) {
            soc_cm_debug(DK_ERR,
                    "UNIT %d SCHAN ERROR: Unknown reason\n",
                    unit);
    } else
#endif
#ifdef BCM_SIRIUS_SUPPORT
        if (SOC_IS_SIRIUS(unit)) {
            soc_cm_debug(DK_ERR,
                    "UNIT %d SCHAN ERROR: Unknown reason\n",
                    unit);
    } else
#endif
#ifdef BCM_TUCANA_SUPPORT
        if ((SOC_IS_TUCANA(unit)) && (opc == READ_MEMORY_ACK_MSG)) {
            sal_dpc(_soc_intr_mem_parity_analyze, INT_TO_PTR(unit), 
                    INT_TO_PTR(src),
                    INT_TO_PTR(-1), 0, 0);
    } else
#endif
        if (opc == MEMORY_FAIL_NOTIFY) {
	switch (soc_schan_error_type(unit, err)) {
	case SOC_SCERR_CFAP_OVER_UNDER:
	    /*
	     * The CFAP is empty but a request for a cell pointer came
	     * in, or the CFAP is full but a request to return a cell
	     * pointer came in.
	     */
	    soc->stat.err_cfap++;
	    soc_cm_debug(DK_ERR, 
			 "UNIT %d SCHAN ERROR: CFAP oversubscribed\n",
			 unit);
	    break;
	case SOC_SCERR_SDRAM_CHKSUM:
	    /*
	     * Checksum error occurred when fetching a slot from SDRAM.
	     */
	    soc->stat.err_sdram++;
	    slot = soc_pci_read(unit, CMIC_MEM_FAIL);
	    soc_cm_debug(DK_ERR, 
			 "UNIT %d SCHAN ERROR: SDRAM checksum error, "
			 "slot=0x%x (GBP index 0x%x)\n",
			 unit, slot, slot * 0x40);
	    break;
	case SOC_SCERR_UNEXP_FIRST_CELL:
	    /*
	     * Unexpected first cell
	     */
	    soc->stat.err_fcell++;
	    soc_cm_debug(DK_ERR, 
			 "UNIT %d SCHAN ERROR: Unexpected first cell\n",
			 unit);
	    break;
	case SOC_SCERR_MMU_SOFT_RST:
	    /*
	     * MMU soft reset: received a second start cell without
	     * receiving and end cell for the previous packet.
	     */
	    soc->stat.err_sr++;
	    soc_cm_debug(DK_ERR,
			 "UNIT %d SCHAN ERROR: MMU soft reset\n",
			 unit);
	    break;
	case SOC_SCERR_CBP_CELL_CRC:
	    soc->stat.err_cellcrc++;
	    soc_cm_debug(DK_ERR,
			 "UNIT %d SCHAN ERROR: CBP Cell CRC error\n",
			 unit);
	    break;
	case SOC_SCERR_CBP_HEADER_PARITY:
	    soc->stat.err_cbphp++;
	    soc_cm_debug(DK_ERR,
			 "UNIT %d SCHAN ERROR: CBP Header parity error\n",
			 unit);
	    break;
	case SOC_SCERR_MMU_NPKT_CELLS:
	    soc->stat.err_npcell++;
	    soc_cm_debug(DK_ERR,
			 "UNIT %d SCHAN ERROR: "
			 "MMU sent cells not in packet\n",
			 unit);
	    break;
	case SOC_SCERR_MEMORY_PARITY:
	    soc->stat.err_mp++;
	    slot = soc_pci_read(unit, CMIC_MEM_FAIL);
            sal_dpc(_soc_intr_mem_parity_analyze, INT_TO_PTR(unit), 
                    INT_TO_PTR(src), 0, INT_TO_PTR(slot), 0);
	    break;
	case SOC_SCERR_CELL_PTR_CRC:
	    soc->stat.err_cpcrc++;
#ifdef BCM_TUCANA_SUPPORT
            if (SOC_IS_TUCANA(unit)) {
                slot = soc_pci_read(unit, CMIC_MEM_FAIL);
                sal_dpc(_soc_intr_mem_crc_analyze, INT_TO_PTR(unit), 
                        0, 0, INT_TO_PTR(slot), 0);
            } else {
#endif
                soc_cm_debug(DK_ERR,
                             "UNIT %d SCHAN ERROR: Cell data CRC error\n",
                             unit);
#ifdef BCM_TUCANA_SUPPORT
            }
#endif
	    break;
	case SOC_SCERR_CELL_DATA_CRC:
	    soc->stat.err_cdcrc++;
#ifdef BCM_TUCANA_SUPPORT
            if (SOC_IS_TUCANA(unit)) {
                slot = soc_pci_read(unit, CMIC_MEM_FAIL);
                sal_dpc(_soc_intr_mem_crc_analyze, INT_TO_PTR(unit), 
                        0, 0, INT_TO_PTR(slot), 0);
            } else {
#endif
                soc_cm_debug(DK_ERR,
                             "UNIT %d SCHAN ERROR: Cell data CRC error\n",
                             unit);
#ifdef BCM_TUCANA_SUPPORT
            }
#endif
	    break;
	case SOC_SCERR_FRAME_DATA_CRC:
	    soc->stat.err_fdcrc++;
#ifdef BCM_TUCANA_SUPPORT
            if (SOC_IS_TUCANA(unit)) {
                slot = soc_pci_read(unit, CMIC_MEM_FAIL);
                sal_dpc(_soc_intr_mem_crc_analyze, INT_TO_PTR(unit), 
                        0, 0, INT_TO_PTR(slot), 0);
            } else {
#endif
                soc_cm_debug(DK_ERR,
                             "UNIT %d SCHAN ERROR: Frame data CRC error\n",
                             unit);
#ifdef BCM_TUCANA_SUPPORT
            }
#endif
	    break;
	case SOC_SCERR_CELL_PTR_BLOCK_CRC:
	    soc->stat.err_cpbcrc++;
#ifdef BCM_TUCANA_SUPPORT
            if (SOC_IS_TUCANA(unit)) {
                slot = soc_pci_read(unit, CMIC_MEM_FAIL);
                sal_dpc(_soc_intr_mem_crc_analyze, INT_TO_PTR(unit), 
                        0, 0, INT_TO_PTR(slot), 0);
            } else {
#endif
                soc_cm_debug(DK_ERR,
                             "UNIT %d SCHAN ERROR: "
                             "Cell pointer block CRC error\n",
                             unit);
#ifdef BCM_TUCANA_SUPPORT
            }
#endif
	    break;
	case SOC_SCERR_PLL_DLL_LOCK_LOSS:
	    soc->stat.err_pdlock++;
	    slot = soc_pci_read(unit, CMIC_MEM_FAIL);
            sal_dpc(_soc_intr_mem_pdll_analyze, INT_TO_PTR(unit), 
                    0, 0, INT_TO_PTR(slot), 0);
	    break;
	case SOC_SCERR_MULTIPLE_ERR:
	    soc->stat.err_multi++;
	    soc_cm_debug(DK_ERR,
			 "UNIT %d SCHAN ERROR: Multiple errors: 0x%x\n",
			 unit, err);
	    break;
	case SOC_SCERR_INVALID:
	    soc->stat.err_invalid++;
	    soc_cm_debug(DK_ERR,
			 "UNIT %d SCHAN ERROR: Unknown memory error\n",
			 unit);
	    break;
        default:
            assert(0);
            break;
	}
    }

    if (soc->schanIntrBlk != 0) {
	soc_intr_disable(unit, IRQ_SCHAN_ERR);

	sal_dpc_time(soc->schanIntrBlk, _soc_sch_error_unblock,
		     INT_TO_PTR(unit), 0, 0, 0, 0);
    }
}

STATIC void
soc_intr_arl_mbuf(int unit, uint32 ignored)
{
    soc_control_t *soc = SOC_CONTROL(unit);

    COMPILER_REFERENCE(ignored);

#if defined(BCM_TRX_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_TRX(unit) || SOC_IS_SIRIUS(unit)) {
        /* IRQ_CHIP_FUNC_0 */
        soc_intr_disable(unit, IRQ_CHIP_FUNC_0);
        soc->stat.intr_chip_func[0]++;
        return;
    }
#endif

    /*
     * Disable the interrupt; it is re-enabled by the ARL thread after
     * it processes the messages.
     */

    soc_intr_disable(unit, IRQ_ARL_MBUF);

    soc->stat.intr_arl_m++;

    if (soc->arl_notify) {
        soc->arl_mbuf_done = 1;
	if (!soc->arl_notified) {
	    soc->arl_notified = 1;
	    sal_sem_give(soc->arl_notify);
	}
    }
}

STATIC void
soc_intr_arl_drop(int unit, uint32 ignored)
{
    soc_control_t *soc = SOC_CONTROL(unit);

    COMPILER_REFERENCE(ignored);

#if defined(BCM_TRX_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_TRX(unit) || SOC_IS_SIRIUS(unit)) {
        /* IRQ_CHIP_FUNC_1 */
        soc_intr_disable(unit, IRQ_CHIP_FUNC_1);
        soc->stat.intr_chip_func[1]++;
        return;
    }
#endif

    soc_pci_analyzer_trigger(unit);

    soc_intr_disable(unit, IRQ_ARL_MBUF_DROP);

    soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_ARL_MSG_DROPPED_CLR);

    soc->stat.intr_arl_d++;

    if (soc->arl_notify) {
        soc->arl_msg_drop = 1;
	if (!soc->arl_notified) {
	    soc->arl_notified = 1;
	    sal_sem_give(soc->arl_notify);
	}
    }
}

STATIC void
soc_intr_arl_cnt0(int unit, uint32 ignored)
{
    soc_control_t *soc = SOC_CONTROL(unit);

    COMPILER_REFERENCE(ignored);

#if defined(BCM_TRX_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_TRX(unit) || SOC_IS_SIRIUS(unit)) {
        /* IRQ_CHIP_FUNC_4 */
        soc_intr_disable(unit, IRQ_CHIP_FUNC_4);
#if defined(BCM_TRIUMPH_SUPPORT)
        if (SOC_IS_TRIUMPH(unit)) {
            sal_dpc(soc_triumph_esm_intr_status, INT_TO_PTR(unit),
                    0, 0, 0, 0);
        }
#endif /* BCM_TRIUMPH_SUPPORT */
#if defined(BCM_TRIUMPH2_SUPPORT)
        if (SOC_IS_TRIUMPH2(unit)) {
            sal_dpc(soc_triumph2_esm_intr_status, INT_TO_PTR(unit),
                    0, 0, 0, 0);
        }
#endif /* BCM_TRIUMPH2_SUPPORT */
        soc->stat.intr_chip_func[4]++;
        return;
    }
#endif

    soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_ARL_DMA_EN_CLR);
    soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_ARL_DMA_DONE_CLR);

    soc->stat.intr_arl_0++;

    if (soc->arl_notify) {
	soc->arl_dma_cnt0 = 1;
	if (!soc->arl_notified) {
	    soc->arl_notified = 1;
	    sal_sem_give(soc->arl_notify);
	}
    }
}

STATIC void
soc_intr_arl_xfer(int unit, uint32 ignored)
{
    soc_control_t *soc = SOC_CONTROL(unit);

    COMPILER_REFERENCE(ignored);

#if defined(BCM_TRX_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_TRX(unit) || SOC_IS_SIRIUS(unit)) {
        /* IRQ_CHIP_FUNC_3 */
        soc_intr_disable(unit, IRQ_CHIP_FUNC_3);
#if defined(BCM_TRIUMPH_SUPPORT)
        if (SOC_IS_TRIUMPH(unit)) {
            sal_dpc(soc_triumph_esm_intr_status, INT_TO_PTR(unit),
                    0, 0, 0, 0);
        }
#endif /* BCM_TRIUMPH_SUPPORT */
#if defined(BCM_TRIUMPH2_SUPPORT)
        if (SOC_IS_TRIUMPH2(unit)) {
            sal_dpc(soc_triumph2_esm_intr_status, INT_TO_PTR(unit),
                    0, 0, 0, 0);
        }
#endif /* BCM_TRIUMPH2_SUPPORT */
        soc->stat.intr_chip_func[3]++;
        return;
    }
#endif

    soc_intr_disable(unit, IRQ_ARL_DMA_XFER);

    soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_ARL_DMA_XFER_DONE_CLR);

    soc->stat.intr_arl_x++;

    if (soc->arl_notify) {
	soc->arl_dma_xfer = 1;
	if (!soc->arl_notified) {
	    soc->arl_notified = 1;
	    sal_sem_give(soc->arl_notify);
	}
    }
}

STATIC void
soc_intr_tdma_done(int unit, uint32 ignored)
{
    soc_control_t *soc = SOC_CONTROL(unit);

    COMPILER_REFERENCE(ignored);

    soc_intr_disable(unit, IRQ_TDMA_DONE);

    soc->stat.intr_tdma++;

    if (soc->tableDmaIntr) {
        sal_sem_give(soc->tableDmaIntr);
    }
}

STATIC void
soc_intr_tslam_done(int unit, uint32 ignored)
{
    soc_control_t *soc = SOC_CONTROL(unit);

    COMPILER_REFERENCE(ignored);

    soc_intr_disable(unit, IRQ_TSLAM_DONE);

    soc->stat.intr_tslam++;

    if (soc->tslamDmaIntr) {
        sal_sem_give(soc->tslamDmaIntr);
    }
}

STATIC void
soc_intr_gbp_full(int unit, uint32 ignored)
{
    soc_control_t	*soc = SOC_CONTROL(unit);

    COMPILER_REFERENCE(ignored);

#if defined(BCM_TRX_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_TRX(unit) || SOC_IS_SIRIUS(unit)) {
        /* IRQ_CHIP_FUNC_2 */
        soc_intr_disable(unit, IRQ_CHIP_FUNC_2);
        soc->stat.intr_chip_func[2]++;
        return;
    }
#endif

    soc->stat.intr_gbp++;

    soc_pci_analyzer_trigger(unit);

    /*
     * It doesn't make sense to "clear" this interrupt, so we disable
     * the interrupt in the mask register and re-enable it some time
     * later using a deferred procedure call.
     */

    soc_intr_disable(unit, IRQ_GBP_FULL);

#ifdef BCM_GBP_SUPPORT
    sal_dpc(_soc_gbp_full_block, INT_TO_PTR(unit), 0, 0, 0, 0);
#endif
}

STATIC void
soc_intr_link_stat(int unit, uint32 ignored)
{
    soc_control_t	*soc = SOC_CONTROL(unit);

    COMPILER_REFERENCE(ignored);

    soc_pci_analyzer_trigger(unit);

    soc->stat.intr_ls++;

    /* Clear interrupt */

    soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_LINK_STAT_MSG_CLR);

    /* Perform user callout, if one is registered */

    if (soc->soc_link_callout != NULL) {
	(*soc->soc_link_callout)(unit);
    }
}

/*
 * PCI Parity and Fatal Error Reporting
 *
 *	If the interrupt routine prints a message on each error,
 *	the console can be frozen or VxWorks workq overflow can occur.
 *
 *	For this reason errors are counted for a period of time and
 *	reported together at a maximum rate.
 */

#define PCI_REPORT_TYPE_PARITY		1
#define PCI_REPORT_TYPE_FATAL		2
#define PCI_REPORT_PERIOD		(SECOND_USEC / 4)

STATIC char *_soc_pci_dma_types[] = {
    "DMA CH0",
    "DMA CH1",
    "DMA CH2",
    "DMA CH3"
};

STATIC char *_soc_pci_extended_dma_types[] = {
    "Status write for TX and RX DMA CH0",     /* 0 */
    "Table DMA",                              /* 1 */
    "Memory write for RX DMA CH0",            /* 2 */
    "Stats DMA",                              /* 3 */
    "Status write for TX and RX DMA CH1",     /* 4 */
    "Unknown",                                /* 5 */
    "Memory write for RX DMA CH1",            /* 6 */
    "Unknown",                                /* 7 */
    "Status write for TX and RX DMA CH2",     /* 8 */
    "Unknown",                                /* 9 */
    "Memory write for RX DMA CH2",            /* 10 */
    "Unknown",                                /* 11 */
    "Status write for TX and RX DMA CH3",     /* 12 */
    "Unknown",                                /* 13 */
    "Memory write for RX DMA CH3",            /* 14 */
    "Unknown",                                /* 15 */
    "Descriptor read for TX and RX DMA CH0",  /* 16 */
    "SLAM DMA",                               /* 17 */
    "Memory read for TX DMA CH0",             /* 18 */
    "Unknown",                                /* 19 */
    "Descriptor read for TX and RX DMA CH1",  /* 20 */
    "Unknown",                                /* 21 */
    "Memory read for TX DMA CH1",             /* 22 */
    "Unknown",                                /* 23 */
    "Descriptor read for TX and RX DMA CH2",  /* 24 */
    "Unknown",                                /* 25 */
    "Memory read for TX DMA CH2",             /* 26 */
    "Unknown",                                /* 27 */
    "Descriptor read for TX and RX DMA CH3",  /* 28 */
    "Unknown",                                /* 29 */
    "Memory read for TX DMA CH3",             /* 30 */
    "Unknown"                                 /* 31 */
};

STATIC char *_soc_pci_extended_trx_dma_types[] = {
    "Table DMA",                                /* 0 */
    "Stats DMA",                                /* 1 */
    "Memory write for RX DMA CH0",              /* 2 */
    "Memory write for RX DMA CH1",              /* 3 */
    "Memory write for RX DMA CH2",              /* 4 */
    "Memory write for RX DMA CH3",              /* 5 */
    "Status write for TX and RX DMA CH0",       /* 6 */
    "Status write for TX and RX DMA CH1",       /* 7 */
    "Status write for TX and RX DMA CH2",       /* 8 */
    "Status write for TX and RX DMA CH3",       /* 9 */
    "SLAM DMA",                                 /* 10 */
    "Memory read for TX DMA CH0",               /* 11 */
    "Memory read for TX DMA CH1",               /* 12 */
    "Memory read for TX DMA CH2",               /* 13 */
    "Memory read for TX DMA CH3",               /* 14 */
    "Descriptor read for TX and RX DMA CH0",    /* 15 */
    "Descriptor read for TX and RX DMA CH1",    /* 16 */
    "Descriptor read for TX and RX DMA CH2",    /* 17 */
    "Descriptor read for TX and RX DMA CH3",    /* 18 */
    "FIFO DMA CH0",                             /* 19 */
    "FIFO DMA CH1",                             /* 20 */
    "FIFO DMA CH2",                             /* 21 */
    "FIFO DMA CH3",                             /* 22 */
    "Unknown",                                  /* 23 */
    "Unknown",                                  /* 24 */
    "Unknown",                                  /* 25 */
    "Unknown",                                  /* 26 */
    "Unknown",                                  /* 27 */
    "Unknown",                                  /* 28 */
    "Unknown",                                  /* 29 */
    "Unknown",                                  /* 30 */
    "Unknown"                                   /* 31 */
};

STATIC void
_soc_pci_report_error(void *p_unit, void *stat, void *type,
		      void *errcnt_dpc, void *p5)
{
    int         	unit = PTR_TO_INT(p_unit);
    soc_control_t	*soc = SOC_CONTROL(unit);
    uint32		errcnt_cur = 0, dmatype_code = 0;
    char		*errtype = NULL, *dmatype = NULL;

    COMPILER_REFERENCE(p5);

    switch (PTR_TO_INT(type)) {
    case PCI_REPORT_TYPE_PARITY:
	soc->pciParityDPC = 0;
	errcnt_cur = soc->stat.intr_pci_pe;
	errtype = "Parity";
        if (soc_feature(unit, soc_feature_extended_pci_error)) {
            dmatype_code = DS_EXT_PCI_PARITY_ERR(PTR_TO_INT(stat));
        } else {
            dmatype_code = DS_PCI_PARITY_ERR(PTR_TO_INT(stat));
        }
	break;
    case PCI_REPORT_TYPE_FATAL:
	soc->pciFatalDPC = 0;
	errcnt_cur = soc->stat.intr_pci_fe;
	errtype = "Fatal";
        if (soc_feature(unit, soc_feature_extended_pci_error)) {
            dmatype_code = DS_EXT_PCI_FATAL_ERR(PTR_TO_INT(stat));
        } else {
            dmatype_code = DS_PCI_FATAL_ERR(PTR_TO_INT(stat));
        }
	break;
    }

    if (soc_feature(unit, soc_feature_extended_pci_error)) {
        if (SOC_IS_TRX(unit)) {
            dmatype =
                _soc_pci_extended_trx_dma_types[dmatype_code];
        } else {
            dmatype = _soc_pci_extended_dma_types[dmatype_code];
        }
    } else {
        dmatype = _soc_pci_dma_types[dmatype_code];
    }

    if (errcnt_cur == PTR_TO_INT(errcnt_dpc) + 1) {
	soc_cm_debug(DK_ERR,
		     "UNIT %d ERROR interrupt: "
                     "CMIC_DMA_STAT = 0x%08x "
		     "PCI %s Error on %s\n",
		     unit,
                     PTR_TO_INT(stat),
		     errtype, dmatype);
    } else {
	soc_cm_debug(DK_ERR,
		     "UNIT %d ERROR interrupt: "
		     "%d PCI %s Errors on %s\n",
		     unit, errcnt_cur - PTR_TO_INT(errcnt_dpc),
		     errtype, dmatype);
    }
}

STATIC void
soc_intr_pci_parity(int unit, uint32 ignored)
{
    soc_control_t	*soc = SOC_CONTROL(unit);
    uint32		stat;
    int			errcnt;

    COMPILER_REFERENCE(ignored);

    soc_pci_analyzer_trigger(unit);

    stat = soc_pci_read(unit, CMIC_DMA_STAT);

    soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_PCI_PARITY_ERR_CLR);

    errcnt = soc->stat.intr_pci_pe++;

    if (!soc->pciParityDPC) {
	soc->pciParityDPC = 1;
	sal_dpc_time(PCI_REPORT_PERIOD, _soc_pci_report_error,
		     INT_TO_PTR(unit), INT_TO_PTR(stat), 
		     INT_TO_PTR(PCI_REPORT_TYPE_PARITY), 
		     INT_TO_PTR(errcnt), 0);
    }
}

STATIC void
soc_intr_pci_fatal(int unit, uint32 ignored)
{
    soc_control_t	*soc = SOC_CONTROL(unit);
    uint32		stat;
    int			errcnt;

    COMPILER_REFERENCE(ignored);

    soc_pci_analyzer_trigger(unit);

    stat = soc_pci_read(unit, CMIC_DMA_STAT);

    soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_PCI_FATAL_ERR_CLR);

    errcnt = soc->stat.intr_pci_fe++;

    if (!soc->pciFatalDPC) {
	soc->pciFatalDPC = 1;
	sal_dpc_time(PCI_REPORT_PERIOD, _soc_pci_report_error,
		     INT_TO_PTR(unit), INT_TO_PTR(stat), 
		     INT_TO_PTR(PCI_REPORT_TYPE_FATAL), 
		     INT_TO_PTR(errcnt), 0);
    }
}

STATIC void
soc_intr_i2c(int unit, uint32 ignored)
{
    soc_control_t *soc = SOC_CONTROL(unit);

    COMPILER_REFERENCE(ignored);
    
    soc->stat.intr_i2c++;

#ifdef INCLUDE_I2C
    soc_i2c_intr(unit);
#else
    soc_intr_disable(unit, IRQ_I2C_INTR);
#endif
}

STATIC void
soc_intr_miim_op(int unit, uint32 ignored)
{
    soc_control_t *soc = SOC_CONTROL(unit);

    COMPILER_REFERENCE(ignored);

    soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MIIM_OP_DONE_CLR);

    soc->stat.intr_mii++;

    if (soc->miimIntr) {
	sal_sem_give(soc->miimIntr);
    }
}
   
STATIC void
soc_intr_stat_dma(int unit, uint32 ignored)
{
    soc_control_t *soc = SOC_CONTROL(unit);

    COMPILER_REFERENCE(ignored);

    soc_pci_write(unit, CMIC_DMA_STAT, DS_STAT_DMA_ITER_DONE_CLR);

    soc->stat.intr_stats++;

    if (soc->counter_intr) {
	sal_sem_give(soc->counter_intr);
    }
}

#ifdef BCM_HERCULES_SUPPORT

STATIC void
_soc_intr_mmu_analyze(void *p_unit, void *p2, void *p3, void *p4, void *p5)
{
    int unit = PTR_TO_INT(p_unit);
 
    COMPILER_REFERENCE(p2);
    COMPILER_REFERENCE(p3);
    COMPILER_REFERENCE(p4);
    COMPILER_REFERENCE(p5);
 
    if (soc_mmu_error_all(unit) < 0) {
        soc_cm_debug(DK_ERR, 
                     "MMU error analysis failed, MMU interrupt disabled\n");
    } else {
        soc_intr_enable(unit, IRQ_MMU_IRQ_STAT);
    }
}
 
STATIC void
soc_intr_mmu_stat(int unit, uint32 ignored)
{
    uint32 		src, mask;
    soc_control_t 	*soc = SOC_CONTROL(unit);

    COMPILER_REFERENCE(ignored);

    src = soc_pci_read(unit, CMIC_MMUIRQ_STAT);
    mask = soc_pci_read(unit, CMIC_MMUIRQ_MASK);

    mask &= ~src;

    /* We know about the port(s), don't interrupt again until serviced */
    soc_pci_write(unit, CMIC_MMUIRQ_MASK, mask);    

    soc->stat.intr_mmu++;

    /* We'll turn this back on if we succeed in the analysis */
    soc_intr_disable(unit, IRQ_MMU_IRQ_STAT);
    sal_dpc(_soc_intr_mmu_analyze, INT_TO_PTR(unit), 0, 0, 0, 0);
}

#endif /* BCM_HERCULES_SUPPORT */


#if defined(BCM_XGS12_SWITCH_SUPPORT)
STATIC void
soc_intr_arl_error(int unit, uint32 ignored)
{
    soc_control_t	*soc;

    COMPILER_REFERENCE(ignored);

    soc = SOC_CONTROL(unit);
    soc->stat.intr_mmu++;	/* should use separate counter */
    soc_intr_disable(unit, IRQ_ARL_ERROR);
    
    
    soc_cm_debug(DK_ERR,
		 "UNIT %d ARL ERROR (bucket overflow or parity error\n",
		 unit);
}

#endif /* BCM_XGS12_SWITCH_SUPPORT */

STATIC void
soc_intr_bit21(int unit, uint32 ignored)
{
#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        soc_control_t *soc;
        soc = SOC_CONTROL(unit);

        /* IRQ_FIFO_CH0_DMA */
        soc_intr_disable(unit, IRQ_FIFO_CH0_DMA);
        soc->stat.intr_fifo_dma[0]++;

	/* Clear FIFO_CH0_DMA_HOSTMEM_TIMEOUT bit */
	WRITE_CMIC_FIFO_RD_DMA_DEBUGr(unit, 1);

        if (soc->ipfixIntr) {
            /* Ingress IPFIX */
	    sal_sem_give(soc->ipfixIntr);
        }
        return;
    }
#endif
#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
	soc_control_t *soc;
	soc_sbx_sirius_config_t *sir;
	soc = SOC_CONTROL(unit);
	sir = SOC_SBX_CFG_SIRIUS(unit);
	
	soc_intr_disable(unit, IRQ_FIFO_CH0_DMA);

	soc->stat.intr_fifo_dma[0]++;
	soc_cm_debug(DK_INTR,"irq_fifo_ch0_dma unit %d\n", unit);

        /* IRQ_FIFO_CH0_DMA */
	if (soc_sbx_sirius_process_cs_dma_fifo(unit) != SOC_E_NONE) {
	    soc_cm_debug(DK_COUNTER, "irq_fifo_ch0_dma: unit=%d CS FIFO busy\n", unit);
	}

	/* Clear interrupt by clearing FIFO_CH0_DMA_HOSTMEM_OVERFLOW bit */
	WRITE_CMIC_FIFO_RD_DMA_DEBUGr(unit, 0);

	/* Clear interrupt by clearing FIFO_CH0_DMA_HOSTMEM_TIMEOUT bit */
	WRITE_CMIC_FIFO_RD_DMA_DEBUGr(unit, 1);

	soc_intr_enable(unit, IRQ_FIFO_CH0_DMA);
        return;
    }
#endif
#ifdef BCM_HERCULES_SUPPORT
    if (SOC_IS_HERCULES(unit)) {
	soc_intr_mmu_stat(unit, ignored);
    }
#endif /* BCM_HERCULES_SUPPORT */

#if defined(BCM_XGS12_SWITCH_SUPPORT)
    if (SOC_IS_XGS12_SWITCH(unit)) {
	soc_intr_arl_error(unit, ignored);
    }
#endif /* BCM_XGS_SWITCH_SUPPORT */
}

STATIC void
soc_intr_bit22(int unit, uint32 ignored)
{
#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        soc_control_t *soc;
        soc = SOC_CONTROL(unit);

        /* IRQ_FIFO_CH1_DMA */
        soc_intr_disable(unit, IRQ_FIFO_CH1_DMA);
        soc->stat.intr_fifo_dma[1]++;

	/* Clear FIFO_CH1_DMA_HOSTMEM_TIMEOUT bit */
	WRITE_CMIC_FIFO_RD_DMA_DEBUGr(unit, 3);

        if (soc->arl_notify) {
            /* Internal L2_MOD_FIFO */
	    sal_sem_give(soc->arl_notify);
        }
        return;
    }
#endif
}

STATIC void
soc_intr_bit23(int unit, uint32 ignored)
{
#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        soc_control_t *soc;
        soc = SOC_CONTROL(unit);

        /* IRQ_FIFO_CH2_DMA */
        soc_intr_disable(unit, IRQ_FIFO_CH2_DMA);
        soc->stat.intr_fifo_dma[2]++;

	/* Clear FIFO_CH2_DMA_HOSTMEM_TIMEOUT bit */
	WRITE_CMIC_FIFO_RD_DMA_DEBUGr(unit, 5);

        if (soc->arl_notify) {
            /* External EXT_L2_MOD_FIFO */
	    sal_sem_give(soc->arl_notify);
        }
        return;
    }
#endif
}

#define TUCANA_ARL_BLOCK        0
#define EASYRIDER_BSE_BLOCK        0

STATIC void
soc_intr_lpm_lo_parity(int unit, uint32 ignored)
{
#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        soc_control_t *soc;
        soc = SOC_CONTROL(unit);

        /* IRQ_FIFO_CH3_DMA */
        soc_intr_disable(unit, IRQ_FIFO_CH3_DMA);
        soc->stat.intr_fifo_dma[3]++;

	/* Clear FIFO_CH3_DMA_HOSTMEM_TIMEOUT bit */
	WRITE_CMIC_FIFO_RD_DMA_DEBUGr(unit, 7);

        if (soc->ipfixIntr) {
            /* Egress IPFIX */
	    sal_sem_give(soc->ipfixIntr);
        }
        return;
    }
#endif
#ifdef BCM_TUCANA_SUPPORT
    if (SOC_IS_TUCANA(unit)) {
        /* We'll turn this back on if we succeed in the analysis */
        soc_intr_disable(unit, IRQ_ARL_LPM_LO_PAR);
        sal_dpc(_soc_intr_mem_parity_analyze, INT_TO_PTR(unit), 
                INT_TO_PTR(SOC_BLOCK_INFO(unit, TUCANA_ARL_BLOCK).schan),
                INT_TO_PTR(DEFIP_LOm), 0, 0);
    }
#endif /* BCM_TUCANA_SUPPORT */
}

STATIC void
soc_intr_bit25(int unit, uint32 ignored)
{
#ifdef BCM_TUCANA_SUPPORT
    if (SOC_IS_TUCANA(unit)) {
        /* "LPM_HI_PARITY" */
        /* We'll turn this back on if we succeed in the analysis */
        soc_intr_disable(unit, IRQ_ARL_LPM_HI_PAR);
        sal_dpc(_soc_intr_mem_parity_analyze, INT_TO_PTR(unit), 
                INT_TO_PTR(SOC_BLOCK_INFO(unit, TUCANA_ARL_BLOCK).schan),
                INT_TO_PTR(DEFIP_HIm), 0, 0);
    }
#endif /* BCM_TUCANA_SUPPORT */
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        /* "BSE_CMDMEM_DONE" */
        soc_control_t *soc = SOC_CONTROL(unit);
        soc_intr_disable(unit, IRQ_BSE_CMDMEM_DONE);
        soc->stat.intr_mem_cmd[0]++;

        if (soc->schanIntr) {
            sal_sem_give(soc->memCmdIntr[0]);
        }
    }
#endif /* BCM_TUCANA_SUPPORT */
}

STATIC void
soc_intr_bit27(int unit, uint32 ignored)
{
#ifdef BCM_TUCANA_SUPPORT
    if (SOC_IS_TUCANA(unit)) {
        /* "L2_PARITY" */
        /* We'll turn this back on if we succeed in the analysis */
        soc_intr_disable(unit, IRQ_ARL_L2_PAR);
        sal_dpc(_soc_intr_mem_parity_analyze, INT_TO_PTR(unit), 
                INT_TO_PTR(SOC_BLOCK_INFO(unit, TUCANA_ARL_BLOCK).schan),
                INT_TO_PTR(L2Xm), 0, 0);
    }
#endif /* BCM_TUCANA_SUPPORT */
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        /* "HSE_CMDMEM_DONE" */
        soc_control_t *soc = SOC_CONTROL(unit);
        soc_intr_disable(unit, IRQ_HSE_CMDMEM_DONE);
        soc->stat.intr_mem_cmd[2]++;

        if (soc->schanIntr) {
            sal_sem_give(soc->memCmdIntr[2]);
        }
    }
#endif /* BCM_TUCANA_SUPPORT */
}

STATIC void
soc_intr_bit26(int unit, uint32 ignored)
{
#ifdef BCM_TUCANA_SUPPORT
    if (SOC_IS_TUCANA(unit)) {
        /* "L3_PARITY" */
        /* We'll turn this back on if we succeed in the analysis */
        soc_intr_disable(unit, IRQ_ARL_L3_PAR);
        sal_dpc(_soc_intr_mem_parity_analyze, INT_TO_PTR(unit), 
                INT_TO_PTR(SOC_BLOCK_INFO(unit, TUCANA_ARL_BLOCK).schan),
                INT_TO_PTR(L3Xm), 0, 0);
    }
#endif /* BCM_TUCANA_SUPPORT */
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        /* "CSE_CMDMEM_DONE" */
        soc_control_t *soc = SOC_CONTROL(unit);
        soc_intr_disable(unit, IRQ_CSE_CMDMEM_DONE);
        soc->stat.intr_mem_cmd[1]++;

        if (soc->schanIntr) {
            sal_sem_give(soc->memCmdIntr[1]);
        }
    }
#endif /* BCM_TUCANA_SUPPORT */
}

STATIC void
soc_intr_bit28(int unit, uint32 ignored)
{
    soc_pci_analyzer_trigger(unit);
#ifdef BCM_TUCANA_SUPPORT
    if (SOC_IS_TUCANA(unit)) {
        /* We'll turn this back on if we succeed in the analysis */
        soc_intr_disable(unit, IRQ_ARL_VLAN_PAR);
        sal_dpc(_soc_intr_mem_parity_analyze, INT_TO_PTR(unit), 
                INT_TO_PTR(SOC_BLOCK_INFO(unit, TUCANA_ARL_BLOCK).schan),
                INT_TO_PTR(VLAN_TABm), 0, 0);
    }
#endif /* BCM_TUCANA_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FB_FX_HX(unit)) {
        soc_intr_disable(unit, IRQ_MEM_FAIL);
        sal_dpc(soc_fb_mmu_parity_error, INT_TO_PTR(unit), 0, 0, 0, 0);
    }
#endif /* BCM_FIREBOLT_SUPPORT */
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        soc_intr_disable(unit, IRQ_MEM_FAIL);
        sal_dpc(soc_er_parity_error, INT_TO_PTR(unit), 0, 0, 0, 0);
    }
#endif /* BCM_EASYRIDER_SUPPORT */
#ifdef BCM_BRADLEY_SUPPORT
    if (SOC_IS_HB_GW(unit)) {
        soc_intr_disable(unit, IRQ_MEM_FAIL);
        sal_dpc(soc_bradley_mmu_parity_error, INT_TO_PTR(unit), 0, 0, 0, 0);
    }
#endif /* BCM_BRADLEY_SUPPORT */
#ifdef BCM_SCORPION_SUPPORT
    if (SOC_IS_SC_CQ(unit)) {
        soc_intr_disable(unit, IRQ_MEM_FAIL);
        sal_dpc(soc_scorpion_parity_error, INT_TO_PTR(unit), 0, 0, 0, 0);
    }
#endif /* BCM_SCORPION_SUPPORT */
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TRIUMPH(unit) || SOC_IS_VALKYRIE(unit)) {
        soc_intr_disable(unit, IRQ_MEM_FAIL);
        sal_dpc(soc_triumph_parity_error, INT_TO_PTR(unit), 0, 0, 0, 0);
    }
#endif /* BCM_TRIUMPH_SUPPORT */
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
        SOC_IS_VALKYRIE2(unit)) {
        soc_intr_disable(unit, IRQ_MEM_FAIL);
        sal_dpc(soc_triumph2_parity_error, INT_TO_PTR(unit), 0, 0, 0, 0);
    }
#endif /* BCM_TRIUMPH2_SUPPORT */
#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        soc_intr_disable(unit, IRQ_MEM_FAIL);
        sal_dpc(soc_trident_parity_error, INT_TO_PTR(unit), 0, 0, 0, 0);
    }
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_ENDURO_SUPPORT
    if (SOC_IS_ENDURO(unit)) {
        soc_intr_disable(unit, IRQ_MEM_FAIL);
        sal_dpc(soc_enduro_parity_error, INT_TO_PTR(unit), 0, 0, 0, 0);
    }
#endif /* BCM_ENDURO_SUPPORT */
#ifdef BCM_HURRICANE_SUPPORT
    if (SOC_IS_HURRICANE(unit)) {
        soc_intr_disable(unit, IRQ_MEM_FAIL);
        sal_dpc(soc_hurricane_parity_error, INT_TO_PTR(unit), 0, 0, 0, 0);
    }
#endif /* BCM_ENDURO_SUPPORT */
}



STATIC void
soc_intr_bit31(int unit, uint32 ignored)
{
#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_time_support)) {
        soc_control_t	*soc = SOC_CONTROL(unit);
        soc_intr_disable(unit, IRQ_BROADSYNC_INTR);

        /* Perform user callout, if one is registered */
        if (soc->soc_time_callout != NULL) {
            (*soc->soc_time_callout)(unit);
        }
        soc_intr_enable(unit, IRQ_BROADSYNC_INTR);
    }
#endif /* BCM_TRIUMPH2_SUPPORT */
}


STATIC void
soc_intr_block(int unit, uint32 block)
{
#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
	if (block < 32) {
	    soc_intr_block_lo_disable(unit, (1<<block));
	} else {
	    soc_intr_block_hi_disable(unit, (1<<(block-32)));
	}
        sal_dpc(soc_sirius_block_error, INT_TO_PTR(unit),
		INT_TO_PTR(block), 0, 0, 0);
    }
#endif /* BCM_SIRIUS_SUPPORT */
}

/*
 * Enable (unmask) or disable (mask) a set of CMIC interrupts.  These
 * routines should be used instead of manipulating CMIC_IRQ_MASK
 * directly, since a read-modify-write is required.  The return value is
 * the previous mask (can pass mask of 0 to just get the current mask).
 */

uint32
soc_intr_enable(int unit, uint32 mask)
{
    uint32 oldMask;
    uint32 newMask;
    int s;

    s = sal_splhi();
    oldMask = SOC_IRQ_MASK(unit);
    SOC_IRQ_MASK(unit) |= mask;
    newMask = SOC_IRQ_MASK(unit);
    /* In polled mode, the hardware IRQ mask is always zero */
    if (SOC_CONTROL(unit)->soc_flags & SOC_F_POLLED) {
        newMask = 0;
    }
    IRQ_MASK_SET(unit, CMIC_IRQ_MASK, newMask);
    sal_spl(s);

    return oldMask;
}

uint32
soc_intr_disable(int unit, uint32 mask)
{
    uint32 oldMask;
    uint32 newMask;
    int s;

    s = sal_splhi();
    oldMask = SOC_IRQ_MASK(unit);
    SOC_IRQ_MASK(unit) &= ~mask;
    newMask = SOC_IRQ_MASK(unit);
    /* In polled mode, the hardware IRQ mask is always zero */
    if (SOC_CONTROL(unit)->soc_flags & SOC_F_POLLED) {
        newMask = 0;
    }
    IRQ_MASK_SET(unit, CMIC_IRQ_MASK, newMask);
    sal_spl(s);

    return oldMask;
}


/*
 * Enable (unmask) or disable (mask) a set of CMIC block specific interrupts. 
 * soc_intr_block_lo_enable/disable handle block 0-31, while
 * soc_intr_block_hi_enable/disable handle block 32-63. 
 * These routines should be used instead of manipulating CMIC_IRQ_MASK_1/CMIC_IRQ_MASK_2
 * directly, since a read-modify-write is required.  The return value is
 * the previous mask (can pass mask of 0 to just get the current mask).
 * For now, these routines apply to SIRIUS only, leave in the common directory
 * in case later devices start to use the new set of IRQ registers
 */

uint32
soc_intr_block_lo_enable(int unit, uint32 mask)
{
    uint32 oldMask = 0;

#ifdef BCM_SIRIUS_SUPPORT
    uint32 newMask;
    int s;

    if (SOC_IS_SIRIUS(unit)) {
	s = sal_splhi();
	oldMask = SOC_IRQ1_MASK(unit);
	SOC_IRQ1_MASK(unit) |= mask;
	newMask = SOC_IRQ1_MASK(unit);
	/* In polled mode, the hardware IRQ mask is always zero */
	if (SOC_CONTROL(unit)->soc_flags & SOC_F_POLLED) {
	    newMask = 0;
	}
	soc_pci_write(unit, CMIC_IRQ_MASK_1, newMask);
	sal_spl(s);
    }
#endif /* BCM_SIRIUS_SUPPORT */

    return oldMask;
}

uint32
soc_intr_block_lo_disable(int unit, uint32 mask)
{
    uint32 oldMask = 0;

#ifdef BCM_SIRIUS_SUPPORT
    uint32 newMask;
    int s;

    if (SOC_IS_SIRIUS(unit)) {
	s = sal_splhi();
	oldMask = SOC_IRQ1_MASK(unit);
	SOC_IRQ1_MASK(unit) &= ~mask;
	newMask = SOC_IRQ1_MASK(unit);
	/* In polled mode, the hardware IRQ mask is always zero */
	if (SOC_CONTROL(unit)->soc_flags & SOC_F_POLLED) {
	    newMask = 0;
	}
	soc_pci_write(unit, CMIC_IRQ_MASK_1, newMask);
	sal_spl(s);
    }
#endif /* BCM_SIRIUS_SUPPORT */

    return oldMask;
}

uint32
soc_intr_block_hi_enable(int unit, uint32 mask)
{
    uint32 oldMask = 0;

#ifdef BCM_SIRIUS_SUPPORT
    uint32 newMask;
    int s;

    if (SOC_IS_SIRIUS(unit)) {
	s = sal_splhi();
	oldMask = SOC_IRQ2_MASK(unit);
	SOC_IRQ2_MASK(unit) |= mask;
	newMask = SOC_IRQ2_MASK(unit);
	/* In polled mode, the hardware IRQ mask is always zero */
	if (SOC_CONTROL(unit)->soc_flags & SOC_F_POLLED) {
	    newMask = 0;
	}
	soc_pci_write(unit, CMIC_IRQ_MASK_2, newMask);
	sal_spl(s);
    }
#endif /* BCM_SIRIUS_SUPPORT */

    return oldMask;
}

uint32
soc_intr_block_hi_disable(int unit, uint32 mask)
{
    uint32 oldMask = 0;

#ifdef BCM_SIRIUS_SUPPORT
    uint32 newMask;
    int s;

    if (SOC_IS_SIRIUS(unit)) {
	s = sal_splhi();
	oldMask = SOC_IRQ2_MASK(unit);
	SOC_IRQ2_MASK(unit) &= ~mask;
	newMask = SOC_IRQ2_MASK(unit);
	/* In polled mode, the hardware IRQ mask is always zero */
	if (SOC_CONTROL(unit)->soc_flags & SOC_F_POLLED) {
	    newMask = 0;
	}
	soc_pci_write(unit, CMIC_IRQ_MASK_2, newMask);
	sal_spl(s);
    }
#endif /* BCM_SIRIUS_SUPPORT */

    return oldMask;
}

/*
 * SOC Interrupt Service Routine
 *
 *   In PLI simulation, the intr thread can call this routine at any
 *   time.  The connection is protected at the level of pli_{set/get}reg.
 */

#define POLL_LIMIT 100000

void
soc_intr(void *_unit)
{
    uint32 		irqStat, irqMask;
    soc_control_t	*soc;
    int 		i = 0;
    int 		poll_limit = POLL_LIMIT;
    int                 unit = PTR_TO_INT(_unit);
#ifdef SAL_SPL_LOCK_ON_IRQ
    int s;

    s = sal_splhi();
#endif

    soc = SOC_CONTROL(unit);

    /*
     * Our handler is permanently registered in soc_probe().  If our
     * unit is not attached yet, it could not have generated this
     * interrupt.  The interrupt line must be shared by multiple PCI
     * cards.  Simply ignore the interrupt and let another handler
     * process it.
     */
    if (soc == NULL || (soc->soc_flags & SOC_F_BUSY) ||
        !(soc->soc_flags & SOC_F_ATTACHED)) {
#ifdef SAL_SPL_LOCK_ON_IRQ
        sal_spl(s);
#endif
	return;
    }

    soc->stat.intr++;		/* Update count */


#ifdef BCM_CMICM_SUPPORT
    if(soc_feature(unit, soc_feature_cmicm)) {
        soc_cmicm_intr(unit);
#ifdef SAL_SPL_LOCK_ON_IRQ
        sal_spl(s);
#endif
        return;
    }
#endif

    /*
     * Read IRQ Status and IRQ Mask and AND to determine active ints.
     * These are re-read each time since either can be changed by ISRs.
     *
     * Since interrupts are edge-driven, it's necessary to continue
     * processing them until the IRQ_STAT register reads zero.  If we
     * return without doing that, we may never see another interrupt!
     */
    for (;;) {
	irqStat = soc_pci_read(unit, CMIC_IRQ_STAT);
	if (irqStat == 0) {
	    break;
	}
	irqMask = SOC_IRQ_MASK(unit);
	irqStat &= irqMask;
	if (irqStat == 0) {
	    break;
	}

	/*
	 * find starting point for handler search
	 * skip over blocks of high-priority but unlikely entries
	 */
	if ((irqStat & INTR_START2_MASK) == 0) {
	    i = INTR_START2_POS;
	} else if ((irqStat & INTR_START1_MASK) == 0) {
	    i = INTR_START1_POS;
	} else {
	    i = 0;
	}

        /*
         * We may have received an interrupt before all data has been
         * posted from the device or intermediate bridge. 
         * The PCI specification requires that we read a device register
         * to make sure pending data is flushed. 
         * Some bridges (we have determined through testing) require more
         * than one read.
         */
        soc_pci_read(unit, CMIC_SCHAN_CTRL); 
        soc_pci_read(unit, CMIC_IRQ_MASK); 

	for (; i < INTR_HANDLERS_COUNT; i++) {
	    if (irqStat & soc_intr_handlers[i].mask) {

		/*
		 * Bit found, dispatch interrupt
		 */

		soc_cm_debug(DK_INTR,
			     "soc_intr unit %d: dispatch %s\n",
			     unit, soc_intr_handlers[i].intr_name);

		(*soc_intr_handlers[i].intr_fn)
		    (unit, soc_intr_handlers[i].intr_data);

		/*
		 * Prevent infinite loop in interrupt handler by
		 * disabling the offending interrupt(s).
		 */

		if (--poll_limit == 0) {
		    soc_cm_debug(DK_ERR,
				 "soc_intr unit %d: "
				 "ERROR can't clear interrupt(s): "
				 "IRQ=0x%x (disabling 0x%x)\n",
				 unit, irqStat, soc_intr_handlers[i].mask);
		    soc_intr_disable(unit, soc_intr_handlers[i].mask);
		    poll_limit = POLL_LIMIT;
		}

		/*
		 * Go back and re-read IRQ status.  Start processing
		 * from scratch since handler may clear more than one
		 * bit. We don't leave the ISR until all of the bits
		 * have been cleared and their handlers called.
		 */
		break;
	    }
	}
    }

    if (soc_feature(unit, soc_feature_extended_cmic_error)) {
	/* process block specific interrupts for block 0 - 31 */
	for (;;) {
	    irqStat = soc_pci_read(unit, CMIC_IRQ_STAT_1);
	    if (irqStat == 0) {
		break;
	    }
	    irqMask = SOC_IRQ1_MASK(unit);
	    irqStat &= irqMask;
	    if (irqStat == 0) {
		break;
	    }
	    
	    /*
	     * We may have received an interrupt before all data has been
	     * posted from the device or intermediate bridge. 
	     * The PCI specification requires that we read a device register
	     * to make sure pending data is flushed. 
	     * Some bridges (we have determined through testing) require more
	     * than one read.
	     */
	    soc_pci_read(unit, CMIC_SCHAN_CTRL); 
	    soc_pci_read(unit, CMIC_IRQ_MASK_1); 
	    
	    for (; i < INTR_BLOCK_LO_HANDLERS_COUNT; i++) {
		if (irqStat & soc_intr_handlers[i].mask) {
		    
		    /*
		     * Bit found, dispatch interrupt
		     */
		    
		    soc_cm_debug(DK_INTR,
				 "soc_intr unit %d: dispatch %s\n",
				 unit, soc_intr_block_lo_handlers[i].intr_name);
		    
		    (*soc_intr_block_lo_handlers[i].intr_fn)
			(unit, soc_intr_block_lo_handlers[i].intr_data);
		    
		    /*
		     * Prevent infinite loop in interrupt handler by
		     * disabling the offending interrupt(s).
		     */
		    
		    if (--poll_limit == 0) {
			soc_cm_debug(DK_ERR,
				     "soc_intr unit %d: "
				     "ERROR can't clear interrupt(s): "
				     "IRQ=0x%x (disabling 0x%x)\n",
				     unit, irqStat, soc_intr_block_lo_handlers[i].mask);
			soc_intr_block_lo_disable(unit, soc_intr_block_lo_handlers[i].mask);
			poll_limit = POLL_LIMIT;
		    }
		    
		    /*
		     * Go back and re-read IRQ status.  Start processing
		     * from scratch since handler may clear more than one
		     * bit. We don't leave the ISR until all of the bits
		     * have been cleared and their handlers called.
		     */
		    break;
		}
	    }
	}
	
	/* process block specific interrupts for block 32 - 63 */
	for (;;) {
	    irqStat = soc_pci_read(unit, CMIC_IRQ_STAT_2);
	    if (irqStat == 0) {
		break;
	    }
	    irqMask = SOC_IRQ2_MASK(unit);
	    irqStat &= irqMask;
	    if (irqStat == 0) {
		break;
	    }
	    
	    /*
	     * We may have received an interrupt before all data has been
	     * posted from the device or intermediate bridge. 
	     * The PCI specification requires that we read a device register
	     * to make sure pending data is flushed. 
	     * Some bridges (we have determined through testing) require more
	     * than one read.
	     */
	    soc_pci_read(unit, CMIC_SCHAN_CTRL); 
	    soc_pci_read(unit, CMIC_IRQ_MASK_2); 
	    
	    for (; i < INTR_BLOCK_HI_HANDLERS_COUNT; i++) {
		if (irqStat & soc_intr_handlers[i].mask) {
		    
		    /*
		     * Bit found, dispatch interrupt
		     */
		    
		    soc_cm_debug(DK_INTR,
				 "soc_intr unit %d: dispatch %s\n",
				 unit, soc_intr_block_hi_handlers[i].intr_name);
		    
		    (*soc_intr_block_hi_handlers[i].intr_fn)
			(unit, soc_intr_block_hi_handlers[i].intr_data);
		    
		    /*
		     * Prevent infinite loop in interrupt handler by
		     * disabling the offending interrupt(s).
		     */
		    
		    if (--poll_limit == 0) {
			soc_cm_debug(DK_ERR,
				     "soc_intr unit %d: "
				     "ERROR can't clear interrupt(s): "
				     "IRQ=0x%x (disabling 0x%x)\n",
				     unit, irqStat, soc_intr_block_hi_handlers[i].mask);
			soc_intr_block_hi_disable(unit, soc_intr_block_hi_handlers[i].mask);
			poll_limit = POLL_LIMIT;
		    }
		    
		    /*
		     * Go back and re-read IRQ status.  Start processing
		     * from scratch since handler may clear more than one
		     * bit. We don't leave the ISR until all of the bits
		     * have been cleared and their handlers called.
		     */
		    break;
		}
	    }
	}
    }

    /* In polled mode, the hardware IRQ mask is always zero */
    if (SOC_CONTROL(unit)->soc_flags & SOC_F_POLLED) {
#ifdef SAL_SPL_LOCK_ON_IRQ
        sal_spl(s);
#endif
        return;
    }
    /*
     * If the interrupt handler is not run in interrupt context, but 
     * rather as a thread or a signal handler, the interrupt handler 
     * must reenable interrupts on the switch controller. Currently
     * we don't distinguish between the two modes of operation, so 
     * we always reenable interrupts here.
     */
    IRQ_MASK_SET(unit, CMIC_IRQ_MASK, SOC_IRQ_MASK(unit));
    if (soc_feature(unit, soc_feature_extended_cmic_error)) {
	soc_pci_write(unit, CMIC_IRQ_MASK_1, SOC_IRQ1_MASK(unit));
	soc_pci_write(unit, CMIC_IRQ_MASK_2, SOC_IRQ2_MASK(unit));
    }
#ifdef SAL_SPL_LOCK_ON_IRQ
    sal_spl(s);
#endif
}

#endif /* defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) */
