
/*
 * $Id: qe2000_intr.c 1.3.62.2 Broadcom SDK $
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
 * QE2000 Interrupt handling
 */

#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbx_txrx.h>
#include <soc/sbx/hal_ka_auto.h>
#include <soc/error.h>
#include <soc/sbx/qe2000_intr.h>

typedef void (*ifn_t)(int unit, uint32 data);

typedef struct {
    uint32	mask;
    ifn_t	intr_fn;
    uint32	intr_data;
    char	*intr_name;
} intr_handler_t;

STATIC intr_handler_t soc_qe2000_intr_handlers[] = {
    {SAND_HAL_KA_PC_INTERRUPT_MASK_PCI_ERROR0_DISINT_MASK,
        soc_qe2000_pci_error0, 0, "SBQE_PCI_ERROR0"},
    {SAND_HAL_KA_PC_INTERRUPT_MASK_PCI_ERROR1_DISINT_MASK,
        soc_qe2000_pci_error1, 0, "SBQE_PCI_ERROR1"},
    {SAND_HAL_KA_PC_INTERRUPT_MASK_PCI_COMPLETION_DISINT_MASK, 
        soc_sbx_txrx_intr, 0, "QEPKT_INTERRUPT"},
    {SAND_HAL_KA_PC_INTERRUPT_MASK_DMA_DONE_DISINT_MASK, 
     soc_qe2000_dma_done_intr, 0, "DMA_DONE_INTERRUPT%"},
    {SAND_HAL_KA_PC_INTERRUPT_MASK_QMGR_DISINT_MASK, 
     soc_qe2000_qmgr_intr, 0, "QMGR_INTERRUPT%"}
};

#define QE2000_INTR_HANDLERS_COUNT	COUNTOF(soc_qe2000_intr_handlers)

void
soc_qe2000_pci_error0(int unit, uint32 ignored)
{
    uint32 intrs, status;

    COMPILER_REFERENCE(ignored);

    intrs = SAND_HAL_READ_OFFS(unit, SAND_HAL_KA_PC_INTERRUPT_OFFSET);

    if (intrs & SAND_HAL_KA_PC_INTERRUPT_PCI_ERROR0_MASK) {
        soc_cm_debug(DK_ERR, "PCI Error 0 Interrupt\n");
        status = SAND_HAL_READ_OFFS(unit, SAND_HAL_KA_PC_ERROR0_OFFSET);
        soc_cm_debug(DK_ERR, "PCI Error0 status 0x%08x\n", status);
        SAND_HAL_WRITE_OFFS(unit, SAND_HAL_KA_PC_ERROR0_OFFSET, status);
    }

    return;
}

void
soc_qe2000_pci_error1(int unit, uint32 ignored)
{
    uint32 intrs, status;

    COMPILER_REFERENCE(ignored);

    intrs = SAND_HAL_READ_OFFS(unit, SAND_HAL_KA_PC_INTERRUPT_OFFSET);

    if (intrs & SAND_HAL_KA_PC_INTERRUPT_PCI_ERROR1_MASK) {
        status = SAND_HAL_READ_OFFS(unit, SAND_HAL_KA_PC_ERROR1_OFFSET);
        soc_cm_debug(DK_ERR, "PCI Error1 status 0x%08x\n", status);
    
#if 0
        /* TODO: these shouldn't happen */
        if ((SAND_HAL_KA_PC_ERROR1_TX_RING_UNDERFLOW_MASK & status )
            || (SAND_HAL_KA_PC_ERROR1_RXBUF_FIFO_OVERFLOW_MASK & status)) {
        }
        soc_qe2000_txrx_error(unit, QE2000_LG_DROP, 
			      SBQE_PCI_ERR1_GET_LARGE_DROP(status));
        soc_qe2000_txrx_error(unit, QE2000_EOP_MISS, 
			      SBQE_PCI_ERR1_GET_EOP_MISSING(status));
        soc_qe2000_txrx_error(unit, QE2000_EXTRA_SOP,
			      SBQE_PCI_ERR1_GET_EXTRA_SOP(status));
        soc_qe2000_txrx_error(unit, QE2000_RXPKT_ABRT,
			      SBQE_PCI_ERR1_GET_RXPKT_ABRT(status));
#endif
        
        SAND_HAL_WRITE_OFFS(unit, SAND_HAL_KA_PC_ERROR1_OFFSET, status);
    }
}

void
soc_qe2000_dma_done_intr(int unit, uint32 ignored)
{
    uint32 intrs = 0;

    COMPILER_REFERENCE(ignored);

    intrs = SAND_HAL_READ_OFFS(unit, SAND_HAL_KA_PC_INTERRUPT_OFFSET);

    if (intrs & SAND_HAL_KA_PC_INTERRUPT_MASK_DMA_DONE_DISINT_MASK) {
	SAND_HAL_RMW_FIELD(unit, KA, PC_DMA_CTRL, PRESERVE_ON_READ, 1);
	SAND_HAL_RMW_FIELD(unit, KA, PC_DMA_CTRL, ACK, 1);
    }

    return;
}

void
soc_qe2000_qmgr_intr(int unit, uint32 ignored)
{
    uint32 pc_interrupt = 0;
    uint32 qmgr;
    uint32 qmgr_error2 = 0;
    uint32 early_demand_req = 0;

    COMPILER_REFERENCE(ignored);

    pc_interrupt = SAND_HAL_READ(unit, KA, PC_INTERRUPT);
    qmgr = SAND_HAL_GET_FIELD(KA, PC_INTERRUPT, QMGR, pc_interrupt);
	
    if (qmgr) {
	soc_cm_debug(DK_ERR, "QMGR error event\n");
	qmgr_error2 = SAND_HAL_READ(unit, KA, QM_ERROR2);
	early_demand_req = SAND_HAL_GET_FIELD(KA, QM_ERROR2, EARLY_DEMAND_REQ, qmgr_error2);
	if (early_demand_req) {
	    soc_cm_debug(DK_ERR, "Early demand request error - fatal error - bandwidth allocation unreliable\n");

	    soc_event_generate(unit,
			       SOC_SWITCH_EVENT_TUNE_ERROR, 
			       SOC_SWITCH_EVENT_TUNE_ERROR_QM_BLOCK, qmgr_error2, 0);

	    SAND_HAL_RMW_FIELD(unit, KA, QM_ERROR2, EARLY_DEMAND_REQ, early_demand_req);
	}
	SAND_HAL_RMW_FIELD(unit, KA, PC_INTERRUPT, QMGR, qmgr);
   }

    return;
}
/* SDK-24645 early demand request error - enabled during bcm init */
/* User should register a callback function.  If error occurs, reboot of Kamino is required */
void
soc_qe2000_qmgr_intr_enable(int unit)
{

    /* unmask - enable interrupt */
    SAND_HAL_RMW_FIELD(unit, KA, PC_INTERRUPT_MASK, QMGR_DISINT, 0);
    SAND_HAL_RMW_FIELD(unit, KA, QM_ERROR2_MASK, EARLY_DEMAND_REQ_DISINT, 0);

}

void
soc_qe2000_isr(void *_unit)
{
    int                 i, unit = PTR_TO_INT(_unit);
    uint32 		intrs, intrmask, handled;
    soc_control_t       *soc = SOC_CONTROL(unit);
    soc_sbx_control_t   *sbx = SOC_SBX_CONTROL(unit);
    int                 serviced = 0;

    /*
     * Our handler is permanently registered.  If our
     * unit is not attached yet, it could not have generated this
     * interrupt.  The interrupt line must be shared by multiple PCI
     * cards.  Simply ignore the interrupt and let another handler
     * process it.
     */
    if (soc == NULL || !(soc->soc_flags & SOC_F_ATTACHED)) {
	    return;
    }

    soc->stat.intr++;		/* Update count */

    intrs = SAND_HAL_READ_OFFS(unit, SAND_HAL_KA_PC_INTERRUPT_OFFSET);
    intrmask = SAND_HAL_READ_OFFS(unit, SAND_HAL_KA_PC_INTERRUPT_MASK_OFFSET);

    handled = 0;
    i = 0; /* Start from first registered handler */
    for (; i < QE2000_INTR_HANDLERS_COUNT; i++) {
        handled |= soc_qe2000_intr_handlers[i].mask;
        if (intrs & soc_qe2000_intr_handlers[i].mask) {
            /*
             * Bit found, dispatch interrupt
             */
            soc_cm_debug(DK_INTR, "soc_qe2000_isr unit %d: dispatch %s\n",
                         unit, soc_qe2000_intr_handlers[i].intr_name);
    
            (*soc_qe2000_intr_handlers[i].intr_fn) 
                (unit, soc_qe2000_intr_handlers[i].intr_data);
            if (soc_qe2000_intr_handlers[i].mask
                == SAND_HAL_KA_PC_INTERRUPT_PCI_COMPLETION_MASK) {
                /* Clear the completion interrupt if necessary */
                SAND_HAL_WRITE_OFFS(unit, SAND_HAL_KA_PC_INTERRUPT_OFFSET,
                                    SAND_HAL_KA_PC_INTERRUPT_PCI_COMPLETION_MASK);
            }
            serviced = 1;
        }
    }

    if ((serviced == 0) && (intrs & ~intrmask)) {
        soc_cm_debug(DK_ERR, "No interrupts serviced on Unit %d \n", unit);
        
        if(0 != intrs) {
            if(SAND_HAL_KA_PC_INTERRUPT_SFI_INTERRUPT_MASK & intrs) {
                SAND_HAL_WRITE_OFFS(unit, 
                    SAND_HAL_KA_PC_SFI_INTERRUPT_SUMMARY_MASK_OFFSET, 
                    0x3FFFF);
            }
            
            soc_cm_debug(DK_ERR, "Unit %d: unhandled interrupt: "
                         "0x%08X mask: 0x%08X\n", unit, intrs, intrmask);
            /*
             * mask off any interrupts we don't handle
             */
            intrmask = ~handled;
            SAND_HAL_WRITE_OFFS(unit, SAND_HAL_KA_PC_INTERRUPT_MASK_OFFSET, 
                                intrmask);
        }
    }

    SAND_HAL_WRITE(sbx->sbhdl, KA, PC_INTERRUPT_MASK, SOC_IRQ_MASK(unit));
}
