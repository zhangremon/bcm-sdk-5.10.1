/*
 * $Id: intr_cmicm.c 1.11.2.1 Broadcom SDK $
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
 * SOC CMICm Interrupt Handlers
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
#include <soc/cmicm.h>

#ifdef BCM_CMICM_SUPPORT

#include <soc/shared/mos_intr_common.h>
#include <soc/uc_msg.h>

/* Declare static functions for interrupt handler array */
STATIC void soc_cmicm_intr_schan_done(int unit, uint32 ignored);
STATIC void soc_cmicm_intr_miim_op(int unit, uint32 ignored);
STATIC void soc_cmicm_intr_tdma_done(int unit, uint32 ignored);
STATIC void soc_cmicm_intr_tslam_done(int unit, uint32 ignored);
STATIC void soc_cmicm_intr_stat_dma(int unit, uint32 ignored);
STATIC void soc_cmicm_intr_ccmdma_done(int unit, uint32 ignored);
STATIC void soc_cmicm_fifo_dma_done(int unit, uint32 ch);
STATIC void soc_cmicm_intr_sbusdma_done(int unit, uint32 ch);

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

#define _SOC_CMCIM_MAX_HANDLER_TYPES 19

STATIC intr_handler_t soc_cmicm_intr_handlers[][_SOC_CMCIM_MAX_HANDLER_TYPES] = {
    {
        { IRQ_CMCx_SCH_OP_DONE,    soc_cmicm_intr_schan_done,    0, "SCH_OP_DONE"   },
        { IRQ_CMCx_MIIM_OP_DONE,   soc_cmicm_intr_miim_op,       0, "MIIM_OP_DONE"   },
        { IRQ_CMCx_TDMA_DONE,      soc_cmicm_intr_tdma_done,     0, "TDMA_DONE"   },
        { IRQ_CMCx_TSLAM_DONE,     soc_cmicm_intr_tslam_done,    0, "TSLAM_DONE"   },
        { IRQ_CMCx_CCMDMA_DONE,    soc_cmicm_intr_ccmdma_done,   0, "CCNDMA_DONE"   },
        
        { IRQ_CMCx_CHAIN_DONE(0),  soc_dma_done_chain, 0, "CH0_CHAIN_DONE" },
        { IRQ_CMCx_CHAIN_DONE(1),  soc_dma_done_chain, 1, "CH1_CHAIN_DONE" },
        { IRQ_CMCx_CHAIN_DONE(2),  soc_dma_done_chain, 2, "CH2_CHAIN_DONE" },
        { IRQ_CMCx_CHAIN_DONE(3),  soc_dma_done_chain, 3, "CH3_CHAIN_DONE" },
        
        { IRQ_CMCx_DESC_DONE(0),   soc_dma_done_desc,  0, "CH0_DESC_DONE"  },
        { IRQ_CMCx_DESC_DONE(1),   soc_dma_done_desc,  1, "CH1_DESC_DONE"  },
        { IRQ_CMCx_DESC_DONE(2),   soc_dma_done_desc,  2, "CH2_DESC_DONE"  },
        { IRQ_CMCx_DESC_DONE(3),   soc_dma_done_desc,  3, "CH3_DESC_DONE"  },
        
        { IRQ_CMCx_STAT_ITER_DONE, soc_cmicm_intr_stat_dma,  0, "STAT_ITER_DONE" },
        
        { IRQ_CMCx_SW_INTR(CMICM_SW_INTR_UC0), soc_cmic_sw_intr, CMICM_SW_INTR_UC0, "UC0_SW_INTR" },
        { IRQ_CMCx_SW_INTR(CMICM_SW_INTR_UC1), soc_cmic_sw_intr, CMICM_SW_INTR_UC1, "UC1_SW_INTR" },
        { 0, NULL, 0, "" },
        { 0, NULL, 0, "" },
        { 0, NULL, 0, "" }
    },
    {
        { IRQ_CMCx_SCH_OP_DONE,    soc_cmicm_intr_schan_done,    0, "SCH_OP_DONE"   },
        { IRQ_CMCx_MIIM_OP_DONE,   soc_cmicm_intr_miim_op,       0, "MIIM_OP_DONE"   },

        { IRQ_SBUSDMA_CH0_DONE,    soc_cmicm_intr_sbusdma_done,  0, "SBUS_DMA0_DONE"   },
        { IRQ_SBUSDMA_CH1_DONE,    soc_cmicm_intr_sbusdma_done,  1, "SBUS_DMA1_DONE"   },
        { IRQ_SBUSDMA_CH2_DONE,    soc_cmicm_intr_sbusdma_done,  2, "SBUS_DMA2_DONE"   },

        { IRQ_CMCx_CCMDMA_DONE,    soc_cmicm_intr_ccmdma_done,   0, "CCNDMA_DONE"   },
        
        { IRQ_CMCx_CHAIN_DONE(0),  soc_dma_done_chain, 0, "CH0_CHAIN_DONE" },
        { IRQ_CMCx_CHAIN_DONE(1),  soc_dma_done_chain, 1, "CH1_CHAIN_DONE" },
        { IRQ_CMCx_CHAIN_DONE(2),  soc_dma_done_chain, 2, "CH2_CHAIN_DONE" },
        { IRQ_CMCx_CHAIN_DONE(3),  soc_dma_done_chain, 3, "CH3_CHAIN_DONE" },
        
        { IRQ_CMCx_DESC_DONE(0),   soc_dma_done_desc,  0, "CH0_DESC_DONE"  },
        { IRQ_CMCx_DESC_DONE(1),   soc_dma_done_desc,  1, "CH1_DESC_DONE"  },
        { IRQ_CMCx_DESC_DONE(2),   soc_dma_done_desc,  2, "CH2_DESC_DONE"  },
        { IRQ_CMCx_DESC_DONE(3),   soc_dma_done_desc,  3, "CH3_DESC_DONE"  },
        
        { IRQ_CMCx_STAT_ITER_DONE, soc_cmicm_intr_stat_dma,  0, "STAT_ITER_DONE" },
      
        { IRQ_CMCx_FIFO_CH_DMA(0), soc_cmicm_fifo_dma_done, 0, "CH0_FIFO_DMA_DONE" },
        { IRQ_CMCx_FIFO_CH_DMA(0), soc_cmicm_fifo_dma_done, 1, "CH0_FIFO_DMA_DONE" },
        { IRQ_CMCx_FIFO_CH_DMA(0), soc_cmicm_fifo_dma_done, 2, "CH0_FIFO_DMA_DONE" },
        { IRQ_CMCx_FIFO_CH_DMA(0), soc_cmicm_fifo_dma_done, 3, "CH0_FIFO_DMA_DONE" }
    }
};

/*
 * Interrupt handler functions
 */

STATIC void
soc_cmicm_intr_schan_done(int unit, uint32 ignored)
{
    soc_control_t	*soc = SOC_CONTROL(unit);
    int cmc = SOC_PCI_CMC(unit);

    COMPILER_REFERENCE(ignored);

    /* Record the schan control regsiter */
    soc->schan_result = soc_pci_read(unit, CMIC_CMCx_SCHAN_CTRL_OFFSET(cmc));
    soc_pci_write(unit, CMIC_CMCx_SCHAN_CTRL_OFFSET(cmc),
        soc_pci_read(unit, CMIC_CMCx_SCHAN_CTRL_OFFSET(cmc)) & ~SC_CMCx_MSG_DONE);

    soc->stat.intr_sc++;

    if (soc->schanIntr) {
        sal_sem_give(soc->schanIntr);
    }
}

STATIC void
soc_cmicm_intr_miim_op(int unit, uint32 ignored)
{
    soc_control_t *soc = SOC_CONTROL(unit);
    int cmc = SOC_PCI_CMC(unit);

    COMPILER_REFERENCE(ignored);

    soc_pci_write(unit, CMIC_CMCx_MIIM_CTRL_OFFSET(cmc), 0); /* Clr Read & Write Stat */

    soc->stat.intr_mii++;

    if (soc->miimIntr) {
        sal_sem_give(soc->miimIntr);
    }
}


STATIC void
soc_cmicm_intr_tdma_done(int unit, uint32 ignored)
{
    soc_control_t *soc = SOC_CONTROL(unit);

    COMPILER_REFERENCE(ignored);

    soc_cmicm_intr0_disable(unit, IRQ_CMCx_TDMA_DONE);

    soc->stat.intr_tdma++;

    if (soc->tableDmaIntr) {
        sal_sem_give(soc->tableDmaIntr);
    }
}

STATIC void
soc_cmicm_intr_tslam_done(int unit, uint32 ignored)
{
    soc_control_t *soc = SOC_CONTROL(unit);

    COMPILER_REFERENCE(ignored);

    soc_cmicm_intr0_disable(unit, IRQ_CMCx_TSLAM_DONE);

    soc->stat.intr_tslam++;

    if (soc->tslamDmaIntr) {
        sal_sem_give(soc->tslamDmaIntr);
    }
}

STATIC void
soc_cmicm_intr_stat_dma(int unit, uint32 ignored)
{
    soc_control_t *soc = SOC_CONTROL(unit);
    int cmc = SOC_PCI_CMC(unit);

    COMPILER_REFERENCE(ignored);

    soc_pci_write(unit, CMIC_CMCx_STAT_DMA_CFG_OFFSET(cmc),
        soc_pci_read(unit, CMIC_CMCx_STAT_DMA_CFG_OFFSET(cmc)) | STDMA_ITER_DONE_CLR);

    soc->stat.intr_stats++;

    if (soc->counter_intr) {
        sal_sem_give(soc->counter_intr);
    }
}

STATIC void
soc_cmicm_intr_ccmdma_done(int unit, uint32 ignored)
{
    soc_control_t *soc = SOC_CONTROL(unit);

    COMPILER_REFERENCE(ignored);

    soc_cmicm_intr0_disable(unit, IRQ_CMCx_CCMDMA_DONE);

    soc->stat.intr_ccmdma++;

    if (soc->ccmDmaIntr) {
        sal_sem_give(soc->ccmDmaIntr);
    }
}

#ifdef BCM_SBUSDMA_SUPPORT
STATIC uint32 _soc_irq_cmic_sbusdma_ch[] = {
    IRQ_SBUSDMA_CH0_DONE, 
    IRQ_SBUSDMA_CH1_DONE,
    IRQ_SBUSDMA_CH2_DONE
};
#endif

STATIC void
soc_cmicm_intr_sbusdma_done(int unit, uint32 ch)
{
#ifdef BCM_SBUSDMA_SUPPORT
    soc_control_t *soc = SOC_CONTROL(unit);
    soc_cmicm_intr0_disable(unit, _soc_irq_cmic_sbusdma_ch[ch]);
    if (ch == soc->tdma_ch) {
        if (soc->tableDmaIntrEnb) {
            sal_sem_give(soc->tableDmaIntr);
        }
    } else if (ch == soc->tslam_ch) {
        if (soc->tslamDmaIntrEnb) {
            sal_sem_give(soc->tslamDmaIntr);
        }
    } else if (ch == soc->desc_ch) {
        if (SOC_SBUSDMA_DM_INTRENB(unit)) {
            sal_sem_give(SOC_SBUSDMA_DM_INTR(unit));
        }
    } else {
        soc_cm_debug(DK_INTR, "Received unallocated sbusdma interrupt !!\n");
    }
#else
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(ch);
#endif
}

STATIC void
soc_cmicm_fifo_dma_done(int unit, uint32 ch)
{
    soc_control_t *soc = SOC_CONTROL(unit);
    soc_cmicm_intr0_disable(unit, IRQ_CMCx_FIFO_CH_DMA(ch));
    switch (ch) {
    case SOC_MEM_FIFO_DMA_CHANNEL_1:
        if (soc->l2modDmaIntrEnb) {
            sal_sem_give(soc->arl_notify);
        }
        break;
    default:
        soc_cm_debug(DK_INTR, "Received unallocated fifo dma interrupt !!\n");
    }
}

/*
 * Enable (unmask) or disable (mask) a set of CMIC interrupts.  These
 * routines should be used instead of manipulating CMIC_IRQ_MASK
 * directly, since a read-modify-write is required.  The return value is
 * the previous mask (can pass mask of 0 to just get the current mask).
 * for CMICm use CMIC_CMCx_PCIE_IRQ_MASK0.
 */

uint32
soc_cmicm_intr0_enable(int unit, uint32 mask)
{
    uint32 oldMask;
    uint32 newMask;
    int s;
    int cmc = SOC_PCI_CMC(unit);

    s = sal_splhi();
    oldMask = SOC_CMCx_IRQ0_MASK(unit,cmc);
    SOC_CMCx_IRQ0_MASK(unit,cmc) |= mask;
    newMask = SOC_CMCx_IRQ0_MASK(unit,cmc);
    /* In polled mode, the hardware IRQ mask is always zero */
    if (SOC_CONTROL(unit)->soc_flags & SOC_F_POLLED) {
        newMask = 0;
    }
    soc_cm_debug(DK_INTR,
             "soc_cmicm_intr0_enable unit %d: mask 0x%8x\n",
             unit, mask);

    soc_pci_write(unit, CMIC_CMCx_PCIE_IRQ_MASK0_OFFSET(cmc), newMask);

    sal_spl(s);

    return oldMask;
}

uint32
soc_cmicm_intr0_disable(int unit, uint32 mask)
{
    uint32 oldMask;
    uint32 newMask;
    int s;
    int cmc = SOC_PCI_CMC(unit);

    s = sal_splhi();
    oldMask = SOC_CMCx_IRQ0_MASK(unit,cmc);
    SOC_CMCx_IRQ0_MASK(unit,cmc) &= ~mask;
    newMask = SOC_CMCx_IRQ0_MASK(unit,cmc);

    /* In polled mode, the hardware IRQ mask is always zero */
    if (SOC_CONTROL(unit)->soc_flags & SOC_F_POLLED) {
        newMask = 0;
    }
    soc_cm_debug(DK_INTR,
             "soc_cmicm_intr0_disable unit %d: mask 0x%8x\n",
             unit, mask);
    soc_pci_write(unit, CMIC_CMCx_PCIE_IRQ_MASK0_OFFSET(cmc), newMask);

    sal_spl(s);

    return oldMask;
}


/*
 * Enable (unmask) or disable (mask) a set of CMICM Common / Switch-Specific 
 * interrupts.  These routines should be used instead of manipulating 
 * CMIC_CMCx_PCIE_IRQ_MASK1 directly, since a read-modify-write is required.
 * The return value is the previous mask (can pass mask of 0 to just
 * get the current mask) 
 */

uint32
soc_cmicm_intr1_enable(int unit, uint32 mask)
{
    uint32 oldMask;
    uint32 newMask;
    int s;
    int cmc = SOC_PCI_CMC(unit);

    s = sal_splhi();
    oldMask = SOC_CMCx_IRQ1_MASK(unit,cmc);
    SOC_CMCx_IRQ1_MASK(unit,cmc) |= mask;
    newMask = SOC_CMCx_IRQ1_MASK(unit,cmc);
    /* In polled mode, the hardware IRQ mask is always zero */
    if (SOC_CONTROL(unit)->soc_flags & SOC_F_POLLED) {
        newMask = 0;
    }
    soc_pci_write(unit, CMIC_CMCx_PCIE_IRQ_MASK1_OFFSET(cmc), newMask);
    sal_spl(s);

    return oldMask;
}


uint32
soc_cmicm_intr1_disable(int unit, uint32 mask)
{
    uint32 oldMask;
    uint32 newMask;
    int s;
    int cmc = SOC_PCI_CMC(unit);

    s = sal_splhi();
    oldMask = SOC_CMCx_IRQ1_MASK(unit,cmc);
    SOC_CMCx_IRQ1_MASK(unit,cmc) &= ~mask;
    newMask = SOC_CMCx_IRQ1_MASK(unit,cmc);
    /* In polled mode, the hardware IRQ mask is always zero */
    if (SOC_CONTROL(unit)->soc_flags & SOC_F_POLLED) {
        newMask = 0;
    }
    soc_pci_write(unit, CMIC_CMCx_PCIE_IRQ_MASK1_OFFSET(cmc), newMask);
    sal_spl(s);

    return oldMask;
}


/*
 * SOC CMICm Interrupt Service Routine
 */

#define POLL_LIMIT 100000

void
soc_cmicm_intr(int unit)
{
    uint32  irqStat, irqMask;
    int     i = 0;
    int     poll_limit = POLL_LIMIT;
    int cmc = SOC_PCI_CMC(unit);
    intr_handler_t *intr_handler = soc_cmicm_intr_handlers[0];
    /*
     * Read IRQ Status and IRQ Mask and AND to determine active ints.
     * These are re-read each time since either can be changed by ISRs.
     */
    for (;;) {
        irqStat = soc_pci_read(unit, CMIC_CMCx_IRQ_STAT0_OFFSET(cmc));
        if (irqStat == 0) {
            break;  /* No pending Interrupts */
        }
        irqMask = SOC_CMCx_IRQ0_MASK(unit,cmc);
        irqStat &= irqMask;
        if (irqStat == 0) {
            break;
        }
    
        i = 0;
    
        /*
        * We may have received an interrupt before all data has been
        * posted from the device or intermediate bridge. 
        * The PCI specification requires that we read a device register
        * to make sure pending data is flushed. 
        */
        soc_pci_read(unit, CMIC_CMCx_SCHAN_CTRL_OFFSET(cmc)); 
        soc_pci_read(unit, CMIC_CMCx_PCIE_IRQ_MASK0_OFFSET(cmc));
                
                
    
        for (; i < _SOC_CMCIM_MAX_HANDLER_TYPES; i++) {
            if (irqStat & intr_handler[i].mask) {

             /* dispatch interrupt */
            soc_cm_debug(DK_INTR,
                     "soc_cmicm_intr unit %d: dispatch %s\n",
                     unit, intr_handler[i].intr_name);
    
            (*intr_handler[i].intr_fn)
                (unit, intr_handler[i].intr_data);
    
            /*
             * Prevent infinite loop in interrupt handler by
             * disabling the offending interrupt(s).
             */
    
            if (--poll_limit == 0) {
                soc_cm_debug(DK_ERR,
                     "soc_cmicm_intr unit %d: "
                     "ERROR can't clear interrupt(s): "
                     "IRQ=0x%x (disabling 0x%x)\n",
                     unit, irqStat, intr_handler[i].mask);
                soc_cmicm_intr0_disable(unit, intr_handler[i].mask);
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

#endif /* CMICM Support */
