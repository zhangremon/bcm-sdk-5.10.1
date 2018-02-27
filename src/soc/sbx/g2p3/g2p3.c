
/*
 * $Id: g2p3.c 1.142 Broadcom SDK $
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
 * FE2000 Ingress library Initialization
 */

#include <sal/types.h>
#include <sal/appl/io.h>
#include <soc/debug.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/fe2000.h>
#include <soc/sbx/g2p3/g2p3_int.h>
#include <soc/sbx/g2p3/g2p3_ucode.h>
#include <soc/sbx/g2p3/g2p3_20g_ucode.h>
#include <soc/sbx/g2p3/c2_g2p3_ucode.h>
#include <soc/sbx/g2p3/c2_g2p3_v6_ucode.h>
#include <soc/sbx/g2p3/c2ss_g2p3_ucode.h>
#include <soc/sbx/g2p3/c2ss_g2p3_v6_ucode.h>
#include <soc/sbx/g2p3/c2qess_g2p3_ucode.h>
#include <soc/sbx/g2p3/c2qess_g2p3_v6_ucode.h>
#include <soc/sbx/g2p3/c2_g2p3_mplstp_ucode.h>
#include <bcm_int/sbx/error.h>
#include <soc/sbx/g2p3/g2p3_cc.h>

static int _soc_sbx_g2p3_hw_params_init(int unit);
static int _soc_sbx_g2p3_xt_hw_params_init(int unit);
int _soc_sbx_g2p3_commons_init(int unit);
static int _soc_sbx_g2p3_ucode_update(int unit);

extern int soc_sbx_txrx_uninit_hw_only(int unit);
/*
 * Debug Output Macros
 */
#define INTR_DEBUG(stuff)  SOC_DEBUG(SOC_DBG_INTR, stuff)


/* Use macro to load ucode to avoid typo mistakes with long ucode names that
 * may only differ by a few characters
 *  
 *  Assumes "unit" is defined and availble
 *  Returns  0 for success, !0 for failure.
 */
#define SOC_SBX_LOAD_UCODE(u)  \
 (soc_sbx_fe2000_ucode_load_from_buffer(unit, u, sizeof(u)))


void
_g2p3_isr(void *_unit, uint32 ireg)
{
    int unit = (int) PTR_TO_INT(_unit);
    soc_sbx_control_t *sbx;
    soc_sbx_g2p3_state_t *fe;
    uint32 interrupt;

    sbx = SOC_SBX_CONTROL(unit);
    assert(sbx);

    fe = (soc_sbx_g2p3_state_t *) sbx->drv;

    interrupt = ireg;

    INTR_DEBUG(("\ng2p3_isr invoked with:0x%08x\n", interrupt));

    /* check for DMA completion */
    if ( interrupt & (SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, DMA_0_DONE_DISINT)
                    | SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, DMA_1_DONE_DISINT)
                    | SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, DMA_2_DONE_DISINT)
                    | SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, DMA_3_DONE_DISINT)
                    | SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT, DMA_SG_DONE)) ) {
        /* in SIM, just call ISR */
        INTR_DEBUG(("%d:g2p3_isr: calling sbFe2000DmaIsr\n", unit));
        sbFe2000DmaIsr(fe->pDmaMgr);
    }

    if (SOC_IRQ_MASK(unit) & (1 << SOC_SBX_FE2000_ISR_TYPE_CMU)) {
        INTR_DEBUG(("%d:g2p3_isr: calling sbFe2000CmuRingProcess\n", unit));
        sbFe2000CmuRingProcess(fe->pCmuMgr, 0);
    }

    /* disable unsupported CM errors */
    SAND_HAL_FE2000_WRITE(sbx->sbhdl, CM_SEGMENT_FLUSH_EVENT_MASK, 0xffffffff);
    SAND_HAL_FE2000_WRITE(sbx->sbhdl, CM_ERROR_MASK, 0xffffffff);
    SAND_HAL_FE2000_WRITE(sbx->sbhdl, CM_SEGMENT_RANGE_ERROR_MASK, 0xffffffff);
    SAND_HAL_FE2000_WRITE(sbx->sbhdl, CM_SEGMENT_DISABLE_ERROR_MASK, 0xffffffff);

}

/* For unit tests */
int
soc_sbx_g2p3_ipv4da_singleton_set(int unit, int flag)
{
    void *cstate = soc_sbx_g2p3_ipv4da_complex_state_get(unit);

    return soc_sbx_g2p3_lpm_singleton_set(unit, cstate, flag);
}
#define MAX_CONTIG_MALLOC 0x2000
int
_soc_sbx_g2p3_commons_init(int unit)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    soc_sbx_g2p3_state_t *fe = sbx->drv;
    sbFe2000PmConfig_t pmconfig;
    soc_sbx_g2p3_policer_params_t pparams;
    soc_sbx_g2p3_policer_bank_params_t pbparams;
    int i, rv, base, size;
    soc_sbx_g2p3_counter_bank_params_t cbparams;
    soc_sbx_g2p3_policer_t pol;
    uint32 segment, bank, oi_offset, mc_ft_offset = 0;
    uint32_t uHppFreq;
    uint32_t uIgmpProxyMode;
    int wb = 0;
    void *cls_wb_data = NULL;

    /* DMA parameters */
    fe->DmaCfg.pciReadsPerEpoch = 50;
    fe->DmaCfg.pioTimeout = 10;
    fe->DmaCfg.maxPio = 0x80;
    fe->DmaCfg.regSet = (uint32_t) fe->regSet;
    fe->DmaCfg.maxPageSize = MAX_CONTIG_MALLOC;
    fe->DmaCfg.ulMM0MaskNarrow0 = (1 << 23) - 1;
    fe->DmaCfg.ulMM0MaskNarrow1 = (1 << 23) - 1;
    fe->DmaCfg.ulMM1MaskNarrow0 = (1 << 23) - 1;
    fe->DmaCfg.ulMM1MaskNarrow1 = (1 << 23) - 1;
    fe->DmaCfg.ulMM0MaskWide    = (1 << 22) - 1;
    fe->DmaCfg.ulMM1MaskWide    = (1 << 22) - 1;
    fe->DmaCfg.ulClsMask        = (1 << 23) - 1;
    fe->DmaCfg.writeSync = thin_writeSync;
    fe->DmaCfg.readSync  = thin_readSync;
    fe->DmaCfg.isrSync   = thin_isrSync;
    fe->DmaCfg.isrUnsync = thin_isrUnsync;
    fe->DmaCfg.handle    = fe->regSet;
    fe->DmaCfg.getHba    = thin_getHba;
    fe->DmaCfg.bUseRing = 0;
    fe->DmaCfg.ulRingEntries = 32;

    BCM_SBX_IF_ERROR_RETURN(sbFe2000DmaInit(&fe->pDmaMgr, &fe->DmaCfg));

    if (SOC_WARM_BOOT((int)sbx->sbhdl)) {
        SOC_SBX_WARM_BOOT_IGNORE((int)sbx->sbhdl, wb);
        SOC_IRQ_MASK(unit) &=
          ~(SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, DMA_0_DONE_DISINT)
            | SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, DMA_1_DONE_DISINT)
            | SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, DMA_2_DONE_DISINT)
            | SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, DMA_3_DONE_DISINT));

        /* disable CMU processing in the ISR */
        SOC_IRQ_MASK(unit) &= ~(1 << SOC_SBX_FE2000_ISR_TYPE_CMU);

        SAND_HAL_FE2000_WRITE(sbx->sbhdl, PC_INTERRUPT_MASK, SOC_IRQ_MASK(unit));

        soc_sbx_g2p3_cls_wb_init(fe, &cls_wb_data);

        SOC_IRQ_MASK(unit) |= (1 << SOC_SBX_FE2000_ISR_TYPE_CMU);

        SOC_SBX_WARM_BOOT_OBSERVE((int)sbx->sbhdl, wb);
    }

    BCM_SBX_IF_ERROR_RETURN(sbFe2000CmuSwInit(&fe->pCmuMgr, 4096, 256,
                                              fe->regSet, thin_getHba,
                                              fe->pDmaMgr));

    /* Disable warm boot when reprogramming the CMU's ring pointers
     */
    SOC_SBX_WARM_BOOT_IGNORE((int)sbx->sbhdl, wb);

    rv = sbFe2000CmuHwInit(fe->pCmuMgr, wb);
    if (BCM_FAILURE(rv)) {
        SOC_SBX_WARM_BOOT_OBSERVE((int)sbx->sbhdl, wb);
        return rv;
    }

    if (!wb) {
#ifndef PLISIM
        BCM_SBX_IF_ERROR_RETURN
            (sbFe2000CmuRanceConfig(fe->pCmuMgr,
                                     0, /* legacy prob en */
                                     0, /* turbo byte prob en */
                                     0, /* turbo packet prob en */
                                     0, /* legacy divisor log */
                                     0, /* turbo byte divisor log */
                                     0, /* turbo packet divisor log */
                                     0xffffffff, /* seed 0 */
                                     0xffffffff, /* seed 1 */
                                     0xffffffff, /* seed 2 */
                                     0xffffffff)); /* seed 3 */
#endif
        /* enable CM to PC ejection */
        sbFe2000CmuFifoEnable(fe->pCmuMgr, 1);
    }


    SOC_IRQ_MASK(unit) &=
      ~(SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, DMA_0_DONE_DISINT)
        | SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, DMA_1_DONE_DISINT)
        | SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, DMA_2_DONE_DISINT)
        | SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, DMA_3_DONE_DISINT));
    SAND_HAL_FE2000_WRITE(sbx->sbhdl, PC_INTERRUPT_MASK, SOC_IRQ_MASK(unit));

    SOC_SBX_WARM_BOOT_OBSERVE((int)sbx->sbhdl, wb);

    sal_memset(&pmconfig, 0, sizeof(pmconfig));
    pmconfig.regSet = fe->regSet;
    pmconfig.dmaMgr = fe->pDmaMgr;
    pmconfig.token = INT_TO_PTR(unit);
    base = 0;
    size = 0;
    for (i = 0; i < SOC_SBX_G2P3_POLICER_BANKS_MAX; i++) {
        rv = soc_sbx_g2p3_policer_bank_params_get(unit, i, &pbparams);
        if (rv == SOC_E_NOT_FOUND) {
            continue;
        }
        if (rv) {
            return rv;
        }
        if (!size) {
            base = pbparams.base;
            pmconfig.base = base;
            pmconfig.bank = pbparams.memory;
        } else {
            /* PM code currently assumes policer banks are contiguous */
            if (pbparams.memory != pmconfig.bank
                || pbparams.base != base + size) {
                return SOC_E_PARAM;
            }
        }
        size += pbparams.size;
        pmconfig.rates[i] = sbFe2000PmConvertRate(pbparams.kbps);
        pmconfig.uNumberOfPolicersPerGroup[i] = pbparams.size;
    }
    pmconfig.uNumberOfPolicers = size;

    /* Current microcode actually requires policers be based at 0 */
    if (base) {
        return SOC_E_PARAM;
    }

    /* The no-op policer must be at physical id 0, and therefore
     * must be policer 0 in bank 0
     */
    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_xtpol_policer_params_get(unit, &pparams));
    if (pparams.bank != 0) {
        return SOC_E_PARAM;
    }

    /* PM code requires timers to be a specific group */
    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_oamtimer_policer_params_get(unit, &pparams));
    if (pparams.bank != SB_FE_2000_PM_OAM_TIMERS_GROUP) {
        return SOC_E_PARAM;
    }

    /* Hardware requires sequencers to be a specific group */
    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_oamseq_policer_params_get(unit, &pparams));
    if (pparams.bank != SB_FE_2000_PM_SEQNUM_GROUP) {
        return SOC_E_PARAM;
    }
    
    uHppFreq = soc_property_get(unit, spn_BCM88025_HPP_FREQ,
                                SB_FE2000XT_HPP_FREQ_375000);
    if (uHppFreq < 1000) {
        uHppFreq = uHppFreq * 1000;
    }
    BCM_SBX_IF_ERROR_RETURN(sbFe2000PmInit(uHppFreq, &fe->pm, &pmconfig));

    /*
     * Add an `all permissive' policer 0
     */
    
    soc_sbx_g2p3_policer_t_init(&pol);
    pol.mode = SB_FE_2000_PM_PMODE_RFC_4115;
    pol.cirkbps = 1;
    pol.cbsbytes = 65279;
    pol.eirkbps = 1;
    pol.ebsbytes = 65279;
    pol.nodebit = 1;

    /* The no-op policer must be policerId 0, which is policerId of bank 0
     * Currently, the Exception policers are in bank 0 - fixed at 1-255 by
     * the SDK.
     */
    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_xtpol_set(unit, SOC_SBX_G2P3_NOP_POLICER_ID, &pol));

    BCM_SBX_IF_ERROR_RETURN
        (soc_sbx_g2p3_pp_init(unit, &fe->cParams, &fe->iParams));
    BCM_SBX_IF_ERROR_RETURN
        (soc_sbx_g2p3_pp_hw_init(unit, NULL, fe->regSet));
    BCM_SBX_IF_ERROR_RETURN
        (soc_sbx_g2p3_cls_init(fe, fe->pDmaMgr, &fe->cParams, &fe->iParams, cls_wb_data));

    /* Disable warm boot while configuring the CMU's Segment pointers
     */
    SOC_SBX_WARM_BOOT_IGNORE((int)sbx->sbhdl, wb);

    for (i = 0; i < SOC_SBX_G2P3_COUNTER_BANKS_MAX; i++) {
        rv = soc_sbx_g2p3_counter_bank_params_get(unit, i, &cbparams);
        if (rv == SOC_E_NOT_FOUND) {
            continue;
        } else if (rv != SOC_E_NONE) {
            SOC_SBX_WARM_BOOT_OBSERVE((int)sbx->sbhdl, wb);
            return rv; 
        }
            
        switch (cbparams.memory) {
        case SB_FE2000_MEM_MM0_INTERNAL_0: bank = 0; break;
        case SB_FE2000_MEM_MM0_NARROW_1: bank = 1; break;
        case SB_FE2000_MEM_MM1_NARROW_0: bank = 2; break;
        case SB_FE2000_MEM_MM1_NARROW_1: bank = 3; break;
        default: continue; 
	  /* return SOC_E_PARAM; */
        }

        segment = i;
        rv = sbFe2000CmuCounterGroupRegister(fe->pCmuMgr, &segment,
                                             (uint32) cbparams.size,
                                             bank,
                                             (uint32) cbparams.base,
                                             SB_FE2000_SEG_TBL_CNTR_TYPE_TURBO);
        if (BCM_FAILURE(rv)) {
            SOC_SBX_WARM_BOOT_OBSERVE((int)sbx->sbhdl, wb);
            return rv;
        }

#ifndef PLISIM
        BCM_SBX_IF_ERROR_RETURN(sbFe2000CmuSegmentClear(fe->pCmuMgr, i));
#endif
    }
    
    SOC_SBX_WARM_BOOT_OBSERVE((int)sbx->sbhdl, wb);

    /* clear the ring, twice to ensure no old references to DMA buffers */
    if (SOC_WARM_BOOT((int)sbx->sbhdl)) {
        SOC_SBX_WARM_BOOT_IGNORE((int)sbx->sbhdl, wb);
        sbFe2000CmuRingProcess(fe->pCmuMgr, 1);
        sbFe2000CmuRingProcess(fe->pCmuMgr, 1);
        SOC_SBX_WARM_BOOT_OBSERVE((int)sbx->sbhdl, wb);
    }

    SOC_SBX_WARM_BOOT_IGNORE((int)sbx->sbhdl, wb);
    SOC_IRQ_MASK(unit) &=
      ~(SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, DMA_0_DONE_DISINT)
        | SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, DMA_1_DONE_DISINT)
        | SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, DMA_2_DONE_DISINT)
        | SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, DMA_3_DONE_DISINT)
        | SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, COUNTER_FIFO_DATA_AVAIL_DISINT)
        | SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, UNIT_DISINT));

    SAND_HAL_FE2000_WRITE(sbx->sbhdl, PC_INTERRUPT_MASK, SOC_IRQ_MASK(unit));

    SOC_SBX_WARM_BOOT_OBSERVE((int)sbx->sbhdl, wb);

    rv = soc_sbx_g2p3_ace_init(fe, &fe->cParams, &fe->iParams);
    BCM_SBX_IF_ERROR_RETURN(rv);

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_cuckoo_init(unit));
    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_complex_init(unit));

    if (SOC_SBX_V6_ENABLE(unit)) {
         SOC_IF_ERROR_RETURN(soc_sbx_g2p3_pp_ipv6_str_sel_set(unit, 1));
    } else { 
         /* Invalidate ipv6mc rule, pkts land on bridge */
         SOC_IF_ERROR_RETURN(soc_sbx_g2p3_pp_ipv6mc_str_sel_set(unit, 0));
         SOC_IF_ERROR_RETURN(soc_sbx_g2p3_pp_ipv6_str_sel_set(unit, 0));
    }

    /* set the OI Base offset - 8K
     *  to reclaim TB_VID space
     */
    BCM_IF_ERROR_RETURN(soc_sbx_g2p3_oi2eoff_get(unit, &oi_offset));
    oi_offset -= 8*1024;
    BCM_IF_ERROR_RETURN(soc_sbx_g2p3_oi2eoff_set(unit, oi_offset));

    /* configure egress mirroring */
    SOC_IF_ERROR_RETURN
                    (soc_sbx_g2p3_emirror_init(unit));

    /* set the MC Base FT offset - from where FTs are allocated 
     * for unknown multicast frames
     */
    mc_ft_offset = 32*1024;
    BCM_IF_ERROR_RETURN(soc_sbx_g2p3_mc_ft_offset_set(unit, mc_ft_offset));

    /* configure igmp snooping mode */
    uIgmpProxyMode = soc_property_get(unit, spn_IGMP_PROXY_MODE, 0);
    SOC_IF_ERROR_RETURN
                    (soc_sbx_g2p3_igmp_proxy_mode_set(unit, uIgmpProxyMode));

    BCM_SBX_IF_ERROR_RETURN(sbFe2000BringUpLr(fe->regSet,
                                              (uint8 *) fe->ucode));
    return SOC_E_NONE;
}

/* allow the SDK to override some ucode parameters at startup*/
static int
_soc_sbx_g2p3_sdk_overrides(int unit)
{
    soc_sbx_control_t *sbx;
    soc_sbx_g2p3_state_t *fe;
    soc_sbx_g2p3_table_manager_t *tm;

    sbx = SOC_SBX_CONTROL(unit);
    if (sbx == NULL) {
        return SOC_E_INIT;
    }

    fe = sbx->drv;
    if (fe == NULL) {
        return SOC_E_MEMORY;
    }
    
    tm = fe->tableinfo;
    /* allow the user to define the partitioning of the policer meter state */
    tm->policerbanks[SOC_SBX_G2P3_INGEGRPOL_ID].kbps = 
        soc_property_get(unit, spn_POLICER_BASE_RATE,
                         tm->policerbanks[SOC_SBX_G2P3_INGEGRPOL_ID].kbps);

    if (tm->policerbanks[SOC_SBX_G2P3_XTPOL_ID].kbps ==
        tm->policerbanks[SOC_SBX_G2P3_INGEGRPOL_ID].kbps) {
        soc_cm_print("ERROR: Not supported: exception and application policers"
                     " configured to same base rate: %d kbps\n",
                     tm->policerbanks[SOC_SBX_G2P3_XTPOL_ID].kbps);
        return SOC_E_CONFIG;
    }

    return SOC_E_NONE;
}


static int
_soc_sbx_g2p3_ucode_update(int unit)
{
    soc_sbx_control_t *sbx;
    sbx = SOC_SBX_CONTROL(unit);
    if (sbx == NULL) {
        return SOC_E_INIT;
    }

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_init(unit));

    SOC_IF_ERROR_RETURN(_soc_sbx_g2p3_sdk_overrides(unit));

    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_stpstate_forward_get(unit, &sbx->stpforward));
    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_stpstate_block_get(unit, &sbx->stpblock));
    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_stpstate_learn_get(unit, &sbx->stplearn));

    return SOC_E_NONE;
}

int
soc_sbx_g2p3_drv_hw_init(int unit)
{
    soc_sbx_control_t *sbx;
    soc_sbx_g2p3_state_t *fe;
    char *ucodestr = NULL;

    sbx = SOC_SBX_CONTROL(unit);
    if (!sbx) {
        return SOC_E_INIT;
    }

    sbx->drv = sal_alloc(sizeof(soc_sbx_g2p3_state_t), "g2p3 state");
    if (!sbx->drv) {
        return SOC_E_MEMORY;
    }
    sal_memset(sbx->drv, 0, sizeof(soc_sbx_g2p3_state_t));

    /* load ucode buffer, need for soc_sbx_g2p3_init() */
    sbx->ucodetype = SOC_SBX_UCODE_TYPE_G2P3;

    fe =sbx->drv;
    fe->ucode = &fe->ucodestr;
    sbx->ucode = fe->ucode;

    /* Load ucode for hw parameters available */
    sbx->ucode_update_func = NULL;
    ucodestr = soc_property_get_str(unit, spn_BCM88025_UCODE);
    
    if (SOC_IS_SBX_FE2KXT(unit)) {
        if(ucodestr) {
            if (sal_strcmp(ucodestr, "c2mplstp") == 0) {
                if (SOC_SBX_LOAD_UCODE(soc_sbx_c2_g2p3_mplstp_fe2kAsm2UcodeEc) != 0) {
                    return SOC_E_FAIL;
                }
            } else {
                if (SOC_SBX_LOAD_UCODE(soc_sbx_c2_g2p3_fe2kAsm2UcodeEc) != 0) {
                    return SOC_E_FAIL;
                }
            }
        }  else {
            if (SOC_SBX_LOAD_UCODE(soc_sbx_c2_g2p3_fe2kAsm2UcodeEc) != 0) {
                return SOC_E_FAIL;
            }
        }
    } else {
        if (SOC_SBX_LOAD_UCODE(soc_sbx_g2p3_20g_fe2kAsm2UcodeEc) != 0) {
            return SOC_E_FAIL;
        }
    }

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_init(unit));

    if (SOC_IS_SBX_FE2KXT(unit)) {
        SOC_IF_ERROR_RETURN(_soc_sbx_g2p3_xt_hw_params_init(unit));
    }else{
        SOC_IF_ERROR_RETURN(_soc_sbx_g2p3_hw_params_init(unit));
    }

    return SOC_E_NONE;
}

#define V4_LPM_L1_SIZE     (1 << 18)
#define V6_LPM_L1_SIZE     (1 << 16)
/*
 *  Compute table size based on the following criteria
 *  if no soc param is specified
 *         mac ipv4da (and ipv6da) share the available memory
 *         ipv4sa (and ipv6sa) are carved out of the da memory at 25:75
 *  if any soc param is specified
 *         the remaining are computed. 
 *         ipv4sa (or ipv6sa) param must be < ipv4da (or ipv6da)
 *         if not specified v4sa is carved out of v4da (similar for v6)
 */
int soc_sbx_g2p3_memmap_cal(int unit, uint32 nmsize, int base,
                         int *macl2size, int *ipv4da_l2size,
                         int *ipv4sa_l2size, int *ipv6da_l2size,
                         int *ipv6sa_l2size, int adjustsa) 
{
        int size = 0;
        int block_size = 0;
        int sasize = 0;

        /* based on map for P5
         * Account for lpm L1 table size, currently slice1 is 16bits 
         * There are 2 L1 tables in ipv6 case and 1 in ipv4 case 
         */
        if (SOC_SBX_V6_ENABLE(unit)) {
            block_size = (nmsize - base - V4_LPM_L1_SIZE - V6_LPM_L1_SIZE)/3;
            *macl2size = *ipv6da_l2size = *ipv4da_l2size = block_size;
        } else {
            block_size = (nmsize - base - V4_LPM_L1_SIZE)/2;
            *macl2size = *ipv4da_l2size = block_size;
        }
        *macl2size = soc_property_get(unit, spn_MAC_DEV_MEMSIZE, *macl2size);
        if (*macl2size <= 0) {
            /* Return memory */
            if (SOC_SBX_V6_ENABLE(unit)) {
               *ipv6da_l2size += block_size >> 1;
               *ipv4da_l2size += block_size >> 1;
            } else {
               *ipv4da_l2size += block_size; 
            }
            *macl2size = 0;
            block_size =  *ipv4da_l2size;
        }
 
        /* Reserve space for DA and SA lkup 
         * we go about this as follows:
         *   if sa_l2size soc param set, use that
         *   else set sa:da = 25:75 
         *   if size of da_l2size is also not set,
         *   split mem between v4, v6 and mac, set sa:da=25:75
         */
        if (SOC_SBX_V6_ENABLE(unit)) {
           
           *ipv6da_l2size = soc_property_get(unit, 
                              spn_L3_V6UC_DA_DEV_MEMSIZE, *ipv6da_l2size); 

           if (*ipv6da_l2size > 0) {
               *ipv6sa_l2size = *ipv6da_l2size >> 2;
           }
           *ipv6sa_l2size = soc_property_get(unit, 
                              spn_L3_V6UC_SA_DEV_MEMSIZE, *ipv6sa_l2size); 
           if (*ipv6da_l2size > 0) {
               if (*ipv6sa_l2size > 0) {
                   if (*ipv6sa_l2size > *ipv6da_l2size) {
                     /* Bad value */
                     soc_cm_print("\nParam Error:v6da sram size less than v6sa sram size\n"); 
                     return SOC_E_PARAM;
                   }
                   /* Carve out the SA block */
                   if (adjustsa) 
                      *ipv6da_l2size -= *ipv6sa_l2size;
               } else {
                   /* Nothing was taken to return */
                   *ipv6sa_l2size = 0;
               }
            } else {
              /* Return da and da l1 memory */
              size = block_size ;
              if (*ipv6sa_l2size <= 0) {
                  /* Return da l1 */
                  size += V6_LPM_L1_SIZE;
                  *ipv6sa_l2size = 0;
              } else {
                  /* Carve out SA block */
                  if (adjustsa) 
                      size -= *ipv6sa_l2size;
              }
              
              if (*macl2size > 0) {
                  *ipv4da_l2size += (size>>1);
                  *macl2size += (size>>1);
              } else {
                  *ipv4da_l2size += size; 
              }
              *ipv6da_l2size = 0;
              block_size = *ipv4da_l2size;
           }
        }

        /* Adjust for IPv4 da */
        *ipv4da_l2size = soc_property_get(unit, 
                              spn_L3_V4UC_DA_DEV_MEMSIZE, *ipv4da_l2size); 
        if (*ipv4da_l2size > 0) {
            *ipv4sa_l2size = *ipv4da_l2size >> 2;
        }
        *ipv4sa_l2size = soc_property_get(unit, 
                              spn_L3_V4UC_SA_DEV_MEMSIZE, *ipv4sa_l2size); 
        if (*ipv4da_l2size > 0) {
            if (*ipv4sa_l2size > 0) {
                if (*ipv4sa_l2size > *ipv4da_l2size) {
                     soc_cm_print("\nParam Error:v4da sram size less than v4sa sram size\n"); 
                     /* Bad value */
                     return SOC_E_PARAM;
                }
                /* Carve out the SA block */
                if (adjustsa) 
                   *ipv4da_l2size -= *ipv4sa_l2size;
            } else {
                /* Nothing was taken to return */
                *ipv4sa_l2size = 0;
            }
        } else {
            size = block_size ;
            if (*ipv4sa_l2size <= 0) {
                /* Return da l1 */
                size += (V4_LPM_L1_SIZE);
                *ipv4sa_l2size = 0;
            } else {
                /* Carve out SA block */
                if (adjustsa) 
                   size -= *ipv4sa_l2size;
            }
            if (SOC_SBX_V6_ENABLE(unit) && (*ipv6da_l2size > 0)) {
                /* Reallocate to mac only if mac mem is enabled */
                if (*macl2size > 0) {
                    size >>=1;
                    *macl2size += size;
                    if (*ipv6sa_l2size > 0) {
                       *ipv6sa_l2size += size>>2;
                       if (adjustsa)
                           size -= size>>2;
                    }
                    *ipv6da_l2size += size;
                } else {
                    if (*ipv6sa_l2size > 0) {
                       *ipv6sa_l2size += size>>2;
                       if (adjustsa)
                           size -= size>>2;
                    }
                    *ipv6da_l2size += size;
                }
            } else {
                /* Reallocate to mac only if mac mem is enabled */
                if (*macl2size > 0) {
                    *macl2size +=size;
                }
            }
            *ipv4da_l2size=0;
        }
        /* Check parameters */
        if (SOC_SBX_V6_ENABLE(unit)) {
            if (adjustsa) {
                sasize =  *ipv4sa_l2size +  *ipv6sa_l2size;
            } else {
                sasize = 0;
            }
            if (((base + *ipv4da_l2size + *ipv6da_l2size + sasize +
                *macl2size) > nmsize)) {
                soc_cm_print("\nParam Error: sram param exceeds memory size: ipv4da: %x ipv4sa: %x", *ipv4da_l2size, *ipv4sa_l2size);
                soc_cm_print("\nError: ipv6da 0x%x ipv6sa 0x%x macc 0x%x\n", *ipv6da_l2size, *ipv6sa_l2size, *macl2size);
                return SOC_E_PARAM;
            }
         } else {
            /* No V6 */
            if (adjustsa) {
                sasize =  *ipv4sa_l2size;
            } else {
                sasize = 0;
            }
            if (((base + *ipv4da_l2size + sasize + *macl2size) > nmsize)) {
                soc_cm_print("\nParam Error: sram param exceeds memory size: ipv4da: %x ipv4sa: %x", *ipv4da_l2size, *ipv4sa_l2size);
                soc_cm_print("\nError: ipv6da 0x%x ipv6sa 0x%x macc 0x%x\n", *ipv6da_l2size, *ipv6sa_l2size, *macl2size);
                return SOC_E_PARAM;
            }
        }
        return SOC_E_NONE;
}

int
soc_sbx_g2p3_drv_init(int unit)
{
    soc_sbx_control_t *sbx;
    soc_sbx_g2p3_state_t *fe;
    char *ucodestr = NULL;
    soc_sbx_g2p3_table_bank_params_t tbparams;
    int size = 0;

    sbx = SOC_SBX_CONTROL(unit);
    if (!sbx) {
        return SOC_E_INIT;
    }
    fe = sbx->drv;

    sbx->ucodetype = SOC_SBX_UCODE_TYPE_G2P3;

    fe->ucode = &fe->ucodestr;
    sbx->ucode = fe->ucode;

    sbx->ucode_update_func = _soc_sbx_g2p3_ucode_update;
    
    if (SOC_IS_SBX_FE2KXT(unit)) {
        ucodestr = soc_property_get_str(unit, spn_BCM88025_UCODE);
        /* respect named code if specified */
        if (ucodestr){
            if (sal_strcmp(ucodestr, "c2_g2p3") == 0) {
                if (soc_sbx_g2p3_v6_ena(unit)) {
                    soc_cm_print("soc_sbx_fe2000xt_init() IPV6 Not Supported in ucode\n");
                    return SOC_E_FAIL;
                }
                if (!soc_sbx_g2p3_mim_ena(unit)) {
                    soc_cm_print("soc_sbx_fe2000xt_init() MiM Not Enabled\n");
                }
                if (SOC_SBX_LOAD_UCODE(soc_sbx_c2_g2p3_fe2kAsm2UcodeEc) != 0) {
                    return SOC_E_FAIL;
                }
            } else if (sal_strcmp(ucodestr, "c2_g2p3_v6") == 0) {
                if (soc_sbx_g2p3_mim_ena(unit)) {
                    soc_cm_print("soc_sbx_fe2000xt_init() MiM Not Supported in ucode\n");
                    return SOC_E_FAIL;
                }
                if (!soc_sbx_g2p3_v6_ena(unit)) {
                    soc_cm_print("soc_sbx_fe2000xt_init() IPv6 Not Enabled\n");
                }
                if (SOC_SBX_LOAD_UCODE(soc_sbx_c2_g2p3_v6_fe2kAsm2UcodeEc) != 0) {
                    return SOC_E_FAIL;
                }
            } else if (sal_strcmp(ucodestr, "c2ss_g2p3") == 0){
                if (soc_sbx_g2p3_v6_ena(unit)) {
                    soc_cm_print("soc_sbx_fe2000xt_init() IPV6 Not Supported in ucode\n");
                    return SOC_E_FAIL;
                }
                if (!soc_sbx_g2p3_mim_ena(unit)) {
                    soc_cm_print("soc_sbx_fe2000xt_init() MiM Not Enabled\n");
                }
                if (SOC_SBX_LOAD_UCODE(soc_sbx_c2ss_g2p3_fe2kAsm2UcodeEc) != 0) {
                    return SOC_E_FAIL;
                }
            } else if (sal_strcmp(ucodestr, "c2ss_g2p3_v6") == 0) {
                if (soc_sbx_g2p3_mim_ena(unit)) {
                    soc_cm_print("soc_sbx_fe2000xt_init() MiM Not Supported in ucode\n");
                    return SOC_E_FAIL;
                }
                if (!soc_sbx_g2p3_v6_ena(unit)) {
                    soc_cm_print("soc_sbx_fe2000xt_init() IPv6 Not Enabled\n");
                }
                if (SOC_SBX_LOAD_UCODE(soc_sbx_c2ss_g2p3_v6_fe2kAsm2UcodeEc) != 0) {
                    return SOC_E_FAIL;
                }
            } else if (sal_strcmp(ucodestr, "c2qess_g2p3") == 0){
                if (soc_sbx_g2p3_v6_ena(unit)) {
                    soc_cm_print("soc_sbx_fe2000xt_init() IPV6 Not Supported in ucode\n");
                    return SOC_E_FAIL;
                }
                if (!soc_sbx_g2p3_mim_ena(unit)) {
                    soc_cm_print("soc_sbx_fe2000xt_init() MiM Not Enabled\n");
                }
                if (SOC_SBX_LOAD_UCODE(soc_sbx_c2qess_g2p3_fe2kAsm2UcodeEc) != 0) {
                    return SOC_E_FAIL;
                }
            } else if (sal_strcmp(ucodestr, "c2qess_g2p3_v6") == 0) {
                if (soc_sbx_g2p3_mim_ena(unit)) {
                    soc_cm_print("soc_sbx_fe2000xt_init() MiM Not Supported in ucode\n");
                    return SOC_E_FAIL;
                }
                if (!soc_sbx_g2p3_v6_ena(unit)) {
                    soc_cm_print("soc_sbx_fe2000xt_init() IPv6 Not Enabled\n");
                }
                if (SOC_SBX_LOAD_UCODE(soc_sbx_c2qess_g2p3_v6_fe2kAsm2UcodeEc) != 0) {
                    return SOC_E_FAIL;
                }
            } else if (sal_strcmp(ucodestr, "c2mplstp") == 0) {
                if (SOC_SBX_LOAD_UCODE(soc_sbx_c2_g2p3_mplstp_fe2kAsm2UcodeEc) != 0) {
                    return SOC_E_FAIL;
                }
            } else {
                soc_cm_print("soc_sbx_fe2000xt_init() Unsupported ucode %s detected\n", ucodestr);
                return SOC_E_FAIL;
            }
        } else {
            if (SOC_SBX_LOAD_UCODE(soc_sbx_c2_g2p3_fe2kAsm2UcodeEc) != 0) {
                return SOC_E_FAIL;
            }
        }
    } else {
        ucodestr = soc_property_get_str(unit, spn_BCM88020_UCODE);
        /* respect named code if specified */
        if (ucodestr) {
            /* both g2p3 and g2p3_t20g will load g2p3_20g ucode */
            if (sal_strcmp(ucodestr, "g2p3") == 0 || sal_strcmp(ucodestr, "g2p3_t20g") == 0) {
                if (SOC_SBX_LOAD_UCODE(soc_sbx_g2p3_20g_fe2kAsm2UcodeEc) != 0) {
                    return SOC_E_FAIL;
                }
	    } else if (sal_strcmp(ucodestr, "g2p3_dscp") == 0) {
                if (SOC_SBX_LOAD_UCODE(soc_sbx_g2p3_fe2kAsm2UcodeEc) != 0) {
                    return SOC_E_FAIL;
                }
            } else {
                soc_cm_print("soc_sbx_fe2000_init() Unsupported ucode detected\n");
                return SOC_E_FAIL;
	    }
        } else {
            if (SOC_SBX_LOAD_UCODE(soc_sbx_g2p3_20g_fe2kAsm2UcodeEc) != 0) {
                return SOC_E_FAIL;
            }
        }
    }
   
    /* set version info */
    sal_memcpy(sbx->uver_name, ((fe2kAsm2IntD *) sbx->ucode)->m_uver_name,
               strlen(((fe2kAsm2IntD *) sbx->ucode)->m_uver_name));
    sbx->uver_maj = ((fe2kAsm2IntD *) sbx->ucode)->m_uver_maj;
    sbx->uver_min = ((fe2kAsm2IntD *) sbx->ucode)->m_uver_min;
    sbx->uver_patch = ((fe2kAsm2IntD *) sbx->ucode)->m_uver_patch;

    if (soc_sbx_ucode_versions[sbx->ucodetype]) {
        sal_free(soc_sbx_ucode_versions[sbx->ucodetype]);
    }

    soc_sbx_ucode_versions[sbx->ucodetype] =
        sal_alloc(64, "ucode ver");
    if (!soc_sbx_ucode_versions[sbx->ucodetype]) {
        return SOC_E_MEMORY;
    }

    sal_sprintf(soc_sbx_ucode_versions[sbx->ucodetype], "%s %d.%d.%d",
                sbx->uver_name, sbx->uver_maj,
                sbx->uver_min, sbx->uver_patch);

    /* get header length and type from the ucode */
    if (SOC_IS_SBX_FE2KXT(unit)) {
        if (fe2kAsm2IntD__getNc((fe2kAsm2IntD*)sbx->ucode, "G2P3_ERH_TYPE", &fe->erhtype) > 0){
            return SOC_E_FAIL;
        }
        if (fe2kAsm2IntD__getNc((fe2kAsm2IntD*)sbx->ucode, "G2P3_ERH_LENGTH", &fe->erhlen) > 0){
            return SOC_E_FAIL;
        }
        /* ajust label cuckoo size */
        if (ucodestr && sal_strcmp(ucodestr, "c2mplstp") == 0) {
            SOC_IF_ERROR_RETURN(soc_sbx_g2p3_l2e_table_size_set(unit,0));
        } else {
            SOC_IF_ERROR_RETURN(soc_sbx_g2p3_outerlabelcuckoo_table_size_set(unit,0));
            SOC_IF_ERROR_RETURN(soc_sbx_g2p3_innerlabelcuckoo_table_size_set(unit,0));
        } 
    }else{
        fe->erhlen = 12;
        fe->erhtype = SOC_SBX_G2P3_ERH_DEFAULT;
    }

    if (SOC_SBX_CFG(unit)->v4_ena) {
        /* This code is microcode dependant. Based on layout of v4, macl2
         * table for g2p3 ucode. If layout changes it has to be reflected 
         * in accord.
         */

        int ipv4da_l2size    = 0;
        int ipv4sa_l2size = 0;
        int ipv6da_l2size = 0;
        int ipv6sa_l2size = 0;
        int v6dhosts = 0;
        int v6shosts = 0;
        int macl2size = soc_sbx_g2p3_mac_table_size_get(unit);
        int base1, base;
        int adjustsa = 0;
        uint32 nmsize = 1 << SOC_SBX_CFG_FE2000(unit)->mm1n0awidth;
        soc_sbx_g2p3_table_bank_params_t tbparams;

        if (SOC_SBX_V6_ENABLE(unit)) {
            /* set DA host size size to (64K) */
            /* I do this first because it's at the bottom of the memory */
            /* map, and I'm a coward */
            /* TODO: provide some knobs for this */
            v6dhosts = soc_property_get(unit, spn_BCM88025_V6_DHOSTS,
                                        64 * 1024);
            v6shosts = soc_property_get(unit, spn_BCM88025_V6_SHOSTS,
                                        16 * 1024);

            SOC_IF_ERROR_RETURN
                (soc_sbx_g2p3_ipv6dhost_table_size_set(unit, v6dhosts));
            SOC_IF_ERROR_RETURN
                (soc_sbx_g2p3_ipv6shost_table_size_set(unit, v6shosts));
        } else {
            /* V6 not enabled, reclaim memory */
            SOC_IF_ERROR_RETURN
                (soc_sbx_g2p3_ipv6mcg_table_size_set(unit, 0));
            SOC_IF_ERROR_RETURN
                (soc_sbx_g2p3_ipv6mcsg_table_size_set(unit, 0));
            SOC_IF_ERROR_RETURN
                (soc_sbx_g2p3_ipv6dhost_table_size_set(unit, 0));
            SOC_IF_ERROR_RETURN
                (soc_sbx_g2p3_ipv6shost_table_size_set(unit, 0));
        }
            
        if (nmsize == (2*1024*1024)) {
            /* set oi2e table size to (128K - 8K) */
            SOC_IF_ERROR_RETURN
                (soc_sbx_g2p3_oi2e_table_size_set(unit, (120 * 1024)));

           /* set ft table size to 256K) */
            SOC_IF_ERROR_RETURN
                (soc_sbx_g2p3_ft_table_size_set(unit, (256 * 1024)));
        }

        if (SOC_IS_SBX_FE2KXT(unit)) {
            SOC_IF_ERROR_RETURN
                (soc_sbx_g2p3_v2e1_table_size_set(unit, (16 * 1024)));        
        }

        /* Verify if there is enough memory to fit in v4, mac
         * choose the largest ipv4da base. 
         * If mac is located ahead of ipv4da, we need to 
         * account compile time size of mac table, we do this by reclaiming
         */
        adjustsa = 1;
        SOC_IF_ERROR_RETURN
             (soc_sbx_g2p3_mac_table_size_set(unit, 0));
        SOC_IF_ERROR_RETURN
            (soc_sbx_g2p3_ipv4da_bank_params_get(unit, 1, &tbparams));    
        base1 = base = tbparams.base;
        SOC_IF_ERROR_RETURN
            (soc_sbx_g2p3_ipv4da_bank_params_get(unit, 2, &tbparams)); 
        if(tbparams.base > base) {
            base = tbparams.base;
            adjustsa = 0;
        }
        SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_memmap_cal(unit, nmsize, base, &macl2size, &ipv4da_l2size,
                         &ipv4sa_l2size, &ipv6da_l2size, 
                         &ipv6sa_l2size, adjustsa));
        /* check if we are violating size on P5 if memmap is based on P4 */
        if (adjustsa) {
             if (SOC_SBX_V6_ENABLE(unit)) {
                if ((base - base1) < (ipv6sa_l2size + ipv4sa_l2size)) {
                   soc_cm_print("\nParam Error, p5 memmap out of bound");
                   return SOC_E_PARAM;
                }
             } else {
                if ((base - base1) < ipv4sa_l2size) {
                   soc_cm_print("\nParam Error, p5 memmap out of bound");
                   return SOC_E_PARAM;
                }
            }
        }
        /* Set the mac table to the new size */
        SOC_IF_ERROR_RETURN
            (soc_sbx_g2p3_mac_table_size_set(unit, macl2size));

        /* Set ip v4 da lpm table size */
        SOC_IF_ERROR_RETURN
            (soc_sbx_g2p3_ipv4da_table_size_set(unit, ipv4da_l2size));

        /* Set ip v6 da lpm table size */
        SOC_IF_ERROR_RETURN
            (soc_sbx_g2p3_ipv6da_table_size_set(unit, ipv6da_l2size));

        /* Set ip v4 sa lpm table size */
        SOC_IF_ERROR_RETURN
               (soc_sbx_g2p3_ipv4sa_table_size_set(unit, ipv4sa_l2size));

        /* Set ip v6 sa lpm table size */
        SOC_IF_ERROR_RETURN
               (soc_sbx_g2p3_ipv6sa_table_size_set(unit, ipv6sa_l2size));

    } else {
        SOC_IF_ERROR_RETURN
            (soc_sbx_g2p3_ipv6dhost_table_size_set(unit, 0));
        SOC_IF_ERROR_RETURN
            (soc_sbx_g2p3_ipv6shost_table_size_set(unit, 0));
        SOC_IF_ERROR_RETURN
            (soc_sbx_g2p3_ipv4mcg_table_size_set(unit, 0));
        SOC_IF_ERROR_RETURN
            (soc_sbx_g2p3_ipv4mcsg_table_size_set(unit, 0));
        SOC_IF_ERROR_RETURN
            (soc_sbx_g2p3_ipv6mcg_table_size_set(unit, 0));
        SOC_IF_ERROR_RETURN
            (soc_sbx_g2p3_ipv6mcsg_table_size_set(unit, 0));

        if (SOC_SBX_CFG_FE2000(unit)->mm1n0awidth < 21) {
	    soc_sbx_g2p3_counter_params_t counter_params;

            SOC_IF_ERROR_RETURN
	      (soc_sbx_g2p3_ingctr_counter_params_get(unit,  &counter_params));
	    size = counter_params.size;
            SOC_IF_ERROR_RETURN
                (soc_sbx_g2p3_ingctr_counter_size_set(unit, size*3/4));
            SOC_IF_ERROR_RETURN
                (soc_sbx_g2p3_egrctr_counter_size_set(unit, size/4));

            SOC_IF_ERROR_RETURN
                (soc_sbx_g2p3_mac_ca_table_size_get(unit, (uint*) &size));
            SOC_IF_ERROR_RETURN
                (soc_sbx_g2p3_mac_table_size_set(unit, size));
	}
    }

    size = soc_sbx_g2p3_oi2e_table_size_get(unit);
    if (size < (32 * 1024)) {
        SOC_IF_ERROR_RETURN
            (soc_sbx_g2p3_oi2e_table_size_set(unit, (32 * 1024)));
    }

    /*
     * the v6 host tables are below the ETEs, so resizing moves ETEs
     * Must update the `efficiency hack' globals.  We always do this,
     * to be safer than sorrier (maybe some other table under the ETEs
     * gets resized at some point).
     */
    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_ete_bank_params_get(unit, 0, &tbparams));
    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_ete0plus1_set(unit, tbparams.base + 1));
    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_ete_bank_params_get(unit, 1, &tbparams));
    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_ete1plus1_set(unit, tbparams.base + 1));
            
    /* if C2, then resize the OAM Remap and Multiplier tables. */
    if (SOC_IS_SBX_FE2KXT(unit)) {
        SOC_IF_ERROR_RETURN
            (soc_sbx_g2p3_oamepremap_c2_table_size_get(unit, (uint*) &size));
        SOC_IF_ERROR_RETURN
            (soc_sbx_g2p3_oamepremap_table_size_set(unit, size));

        SOC_IF_ERROR_RETURN
            (soc_sbx_g2p3_oammultiplier_c2_table_size_get(unit, (uint*) &size));
        SOC_IF_ERROR_RETURN
            (soc_sbx_g2p3_oammultiplier_table_size_set(unit, size));
    }

    return SOC_E_NONE;
}

int
soc_sbx_g2p3_ilib_recover(int unit) 
{
    SOC_IF_ERROR_RETURN (soc_sbx_g2p3_recover(unit));
    SOC_IF_ERROR_RETURN (soc_sbx_g2p3_complex_recover(unit));
    return SOC_E_NONE;
}

int
soc_sbx_g2p3_ilib_init(int unit)
{
    soc_sbx_control_t *sbx;
    soc_sbx_g2p3_state_t *fe;

    sbx = SOC_SBX_CONTROL(unit);
    if (sbx == NULL) {
        return SOC_E_INIT;
    }

    if (!SOC_IS_SBX_FE(unit)) {
        soc_cm_print("\nILIB Init allowed for FE only");
        return SOC_E_INTERNAL;
    }

    fe = sbx->drv;
    if (fe == NULL) {
        return SOC_E_MEMORY;
    }
    sbx->lib_isr = _g2p3_isr;

    /* define common config parameters */
    fe->regSet = INT_TO_PTR(unit);
    fe->unit = unit;
    if (SOC_IS_SBX_FE2KXT(unit)) {
        fe->queues = &SOC_SBX_CFG_FE2000(unit)->xt_init_params.queues;
    }else{
        fe->queues = &SOC_SBX_CFG_FE2000(unit)->init_params.queues;
    }

    fe->cParams.clientData = fe->regSet;
    fe->cParams.uHashConfig = 
        SB_FE_HASH_OVER_L2
        | SB_FE_HASH_OVER_VID
        | SB_FE_HASH_OVER_L3
        | SB_FE_HASH_OVER_L4;
    fe->cParams.clientData = fe->regSet;
    fe->cParams.maximumContiguousDmaMalloc = MAX_CONTIG_MALLOC;
    fe->cParams.sbfree = thin_free;
    fe->cParams.sbmalloc = thin_malloc;
    fe->cParams.isrSync = thin_isrSync;
    fe->cParams.isrUnsync = thin_isrUnsync;
    fe->cParams.asyncCallback = NULL;


    SOC_IF_ERROR_RETURN(_soc_sbx_g2p3_commons_init(unit));
    
    return SOC_E_NONE;
}


int
soc_sbx_g2p3_ilib_uninit(int unit)
{
    soc_sbx_control_t *sbx;
    soc_sbx_g2p3_state_t *fe;
    sbStatus_t status;
    int retval = SOC_E_NONE;
    SOC_SBX_WARM_BOOT_DECLARE(int wb);

    sbx = SOC_SBX_CONTROL(unit);
    assert(sbx);
    assert(sbx->drv);

    fe = (soc_sbx_g2p3_state_t *) sbx->drv;

    status = soc_sbx_g2p3_ace_un_init(fe);
    if (status != SB_OK) {
        retval = SOC_E_INTERNAL;
    }

    status = soc_sbx_g2p3_cls_un_init(fe);
    if (status != SB_OK) {
        retval = SOC_E_INTERNAL;
    }

    /* always ignore warm boot when tearing down software state,
     * any chip writes must be allowed to properly clear out and disable
     * chip to host communication configurations
     */
    SOC_SBX_WARM_BOOT_IGNORE(unit, wb);

    sbFe2000CmuHwUninit(fe->pCmuMgr);

    soc_sbx_txrx_uninit_hw_only(unit);

    SOC_SBX_WARM_BOOT_OBSERVE(unit, wb);

    /* teardown UTG data structs */
    soc_sbx_g2p3_uninit(unit);

    sbFe2000CmuSwUninit(&fe->pCmuMgr);

    /*soc_sbx_g2p3_pp_un_init() */
    /*sbFe2000PmUnInit()*/
    /*sbFe2000DmaUnInit()*/
    return retval;
}


soc_sbx_g2p3_state_t *
soc_sbx_g2p3_unit2fe(int unit)
{
    soc_sbx_control_t *sbx;

    sbx = SOC_SBX_CONTROL(unit);
    if (sbx == NULL) {
        return NULL;
    }

    if (!SOC_IS_SBX_FE(unit)) {
        soc_cm_print("\n unit2fe allowed for FE only");
        return NULL;
    }

    return sbx->drv;
}

int soc_sbx_g2p3_is_fe2kxt(int unit)
{
    soc_sbx_control_t *sbx;

    sbx = SOC_SBX_CONTROL(unit);
    return SAND_HAL_IS_FE2KXT(sbx->sbhdl);

}

int soc_sbx_g2p3_v4_ena(int unit)
{
    return (SOC_SBX_V4_ENABLE(unit));
}

int soc_sbx_g2p3_v6_ena(int unit)
{
    return (SOC_SBX_V6_ENABLE(unit));
}

int soc_sbx_g2p3_mim_ena(int unit)
{
    return (SOC_SBX_MIM_ENABLE(unit));
}

int soc_sbx_g2p3_mplstp_ena(int unit)
{
    return (SOC_SBX_MPLSTP_ENABLE(unit));
}

int soc_sbx_g2p3_dscp_ena(int unit)
{
    return (SOC_SBX_DSCP_ENABLE(unit));
}

int
soc_sbx_g2p3_topci_qid_get(int unit, int *qid)
{
    soc_sbx_control_t *sbx;
    soc_sbx_g2p3_state_t *fe;

    sbx = SOC_SBX_CONTROL(unit);
    if (sbx == NULL) {
        return SOC_E_INIT;
    }

    if (!SOC_IS_SBX_FE(unit)) {
        soc_cm_print("\nILIB Init allowed for FE only");
        return SOC_E_INTERNAL;
    }

    fe = sbx->drv;
    if (fe == NULL) {
        return SOC_E_MEMORY;
    }

    *qid = fe->queues->topciqid;

    return SOC_E_NONE;
}
int
soc_sbx_g2p3_frompci_qid_get(int unit, int *qid)
{
    soc_sbx_control_t *sbx;
    soc_sbx_g2p3_state_t *fe;

    sbx = SOC_SBX_CONTROL(unit);
    if (sbx == NULL) {
        return SOC_E_INIT;
    }

    if (!SOC_IS_SBX_FE(unit)) {
        soc_cm_print("\nILIB Init allowed for FE only");
        return SOC_E_INTERNAL;
    }

    fe = sbx->drv;
    if (fe == NULL) {
        return SOC_E_MEMORY;
    }

    *qid = fe->queues->frompciqid;

    return SOC_E_NONE;
}

static int
_soc_sbx_g2p3_hw_params_init(int unit)
{
    int i;
    soc_sbx_g2p3_state_t *fe = SOC_SBX_CONTROL(unit)->drv;
    sbFe2000InitParams_t *ip;
    sbFe2000InitParamsMm_t *mp;
    soc_sbx_g2p3_memory_params_t mparams;

    ip = &SOC_SBX_CFG_FE2000(unit)->init_params;

    for( i = 0; i < SB_FE2000_LR_NUMBER_OF_STREAMS; i++ ) {
        ip->lr.bStreamOnline[i] = !ip->lr.bLrpBypass;
    }

    mp = &(ip->mm[0]); 

    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_memory_params_get(unit, SB_FE2000_MEM_MM0_INTERNAL_0,
                                        &mparams));
    fe->DmaCfg.mm0i0wide = (mparams.width == 64);
    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_memory_params_get(unit, SB_FE2000_MEM_MM0_INTERNAL_1,
                                        &mparams));
    fe->DmaCfg.mm0i1wide = (mparams.width == 64);
    if (!fe->DmaCfg.mm0i0wide && !fe->DmaCfg.mm0i1wide) {
        mp->ramconfiguration.InternalRamConfiguration
            = SB_FE2000_MM_RAM0_16KBY36_AND_RAM1_16KBY36;
    } else if (fe->DmaCfg.mm0i0wide && !fe->DmaCfg.mm0i1wide) {
        mp->ramconfiguration.InternalRamConfiguration
            = SB_FE2000_MM_RAM0_8KBY72_AND_RAM1_16KBY36;
    } else if (!fe->DmaCfg.mm0i0wide && fe->DmaCfg.mm0i1wide) {
        mp->ramconfiguration.InternalRamConfiguration
            = SB_FE2000_MM_RAM0_16KBY36_AND_RAM1_8KBY72;
    } else {
        mp->ramconfiguration.InternalRamConfiguration
            = SB_FE2000_MM_RAM0_8KBY72_AND_RAM1_8KBY72;
    }
    mp->ramconfiguration.NarrowPortRam0Configuration
        = SB_FE2000_MM_ONE_BY_18_RAM_CONNECTED;
    mp->ramconfiguration.NarrowPortRam0Words = 2 * 1024 * 1024;
    mp->ramconfiguration.NarrowPortRam1Configuration
        = SB_FE2000_MM_ONE_BY_18_RAM_CONNECTED;
    mp->ramconfiguration.NarrowPortRam1Words = 2 * 1024 * 1024;
    mp->ramconfiguration.WidePortRamConfiguration
        = SB_FE2000_MM_ONE_BY_36_RAM_CONNECTED;
    mp->ramconfiguration.WidePortRamWords = 1 * 1024 * 1024;

    mp->ramconnections.Lrp0MemoryConnection
        = SB_FE2000_MM_LRP0_CONNECTED_TO_NARROWPORT_DDRII_PORT0;
    mp->ramconnections.Lrp1MemoryConnection
        = SB_FE2000_MM_LRP1_CONNECTED_TO_NARROWPORT_DDRII_PORT1;
    mp->ramconnections.Lrp2MemoryConnection
        = SB_FE2000_MM_LRP2_CONNECTED_TO_NO_RESOURCE;
    mp->ramconnections.Lrp3MemoryConnection
        = SB_FE2000_MM_LRP3_CONNECTED_TO_INTERNAL_SINGLE_RAM1;
    mp->ramconnections.Lrp4MemoryConnection
        = SB_FE2000_MM_LRP4_CONNECTED_TO_WIDEPORT_DDRII_PORT;
    mp->ramconnections.PmuMemoryConnection
        = SB_FE2000_MM_PMU_CONNECTED_TO_WIDEPORT_DDRII;
    mp->ramconnections.Cmu0MemoryConnection
        = SB_FE2000_MM_CMU0_CONNECTED_TO_INTERNAL_SINGLE_RAM0;
    mp->ramconnections.Cmu1MemoryConnection
        = SB_FE2000_MM_CMU1_CONNECTED_TO_NARROWPORT_DDRII_PORT1;
    mp->ramprotection.InternalRam0ProtectionScheme
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.InternalRam1ProtectionScheme
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.WidePortRamProtectionScheme
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.NarrowPort0ProcessorInterface
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.NarrowPort0DmaAccess
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.NarrowPort0SecondaryClient
        = SB_FE2000_MM_35BITS_1BITPARITY;
    mp->ramprotection.NarrowPort0PrimaryClient
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.NarrowPort1ProcessorInterface
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.NarrowPort1DmaAccess
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.NarrowPort1SecondaryClient
        = SB_FE2000_MM_35BITS_1BITPARITY;
    mp->ramprotection.NarrowPort1PrimaryClient
        = SB_FE2000_MM_32BITS_4BITPARITY;

    mp = &(ip->mm[1]); 

    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_memory_params_get(unit, SB_FE2000_MEM_MM1_INTERNAL_0,
                                        &mparams));
    fe->DmaCfg.mm1i0wide = (mparams.width == 64);
    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_memory_params_get(unit, SB_FE2000_MEM_MM1_INTERNAL_1,
                                        &mparams));
    fe->DmaCfg.mm1i1wide = (mparams.width == 64);
    if (!fe->DmaCfg.mm1i0wide && !fe->DmaCfg.mm1i1wide) {
        mp->ramconfiguration.InternalRamConfiguration
            = SB_FE2000_MM_RAM0_16KBY36_AND_RAM1_16KBY36;
    } else if (fe->DmaCfg.mm1i0wide && !fe->DmaCfg.mm1i1wide) {
        mp->ramconfiguration.InternalRamConfiguration
            = SB_FE2000_MM_RAM0_8KBY72_AND_RAM1_16KBY36;
    } else if (!fe->DmaCfg.mm1i0wide && fe->DmaCfg.mm1i1wide) {
        mp->ramconfiguration.InternalRamConfiguration
            = SB_FE2000_MM_RAM0_16KBY36_AND_RAM1_8KBY72;
    } else {
        mp->ramconfiguration.InternalRamConfiguration
            = SB_FE2000_MM_RAM0_8KBY72_AND_RAM1_8KBY72;
    }

    mp->ramconfiguration.NarrowPortRam0Configuration
        = SB_FE2000_MM_ONE_BY_18_RAM_CONNECTED;
    mp->ramconfiguration.NarrowPortRam0Words = 2 * 1024 * 1024;
    mp->ramconfiguration.NarrowPortRam1Configuration
        = SB_FE2000_MM_ONE_BY_18_RAM_CONNECTED;
    mp->ramconfiguration.NarrowPortRam1Words = 2 * 1024 * 1024;
    mp->ramconfiguration.WidePortRamConfiguration
        = SB_FE2000_MM_ONE_BY_36_RAM_CONNECTED;
    mp->ramconfiguration.WidePortRamWords = 1 * 1024 * 1024;

    mp->ramconnections.Lrp0MemoryConnection
        = SB_FE2000_MM_LRP0_CONNECTED_TO_NARROWPORT_DDRII_PORT0;
    mp->ramconnections.Lrp1MemoryConnection
        = SB_FE2000_MM_LRP1_CONNECTED_TO_NARROWPORT_DDRII_PORT1;
    mp->ramconnections.Lrp2MemoryConnection
        = SB_FE2000_MM_LRP2_CONNECTED_TO_INTERNAL_SINGLE_RAM0;
    mp->ramconnections.Lrp3MemoryConnection
        = SB_FE2000_MM_LRP3_CONNECTED_TO_INTERNAL_SINGLE_RAM1;
    mp->ramconnections.Lrp4MemoryConnection
        = SB_FE2000_MM_LRP4_CONNECTED_TO_WIDEPORT_DDRII_PORT;
    mp->ramconnections.PmuMemoryConnection
        = SB_FE2000_MM_PMU_CONNECTED_TO_WIDEPORT_DDRII;
    mp->ramconnections.Cmu0MemoryConnection
        = SB_FE2000_MM_CMU0_CONNECTED_TO_NARROWPORT_DDRII_PORT0;
    mp->ramconnections.Cmu1MemoryConnection
        = SB_FE2000_MM_CMU1_CONNECTED_TO_NARROWPORT_DDRII_PORT1;

    mp->ramprotection.InternalRam0ProtectionScheme
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.InternalRam1ProtectionScheme
        = SB_FE2000_MM_35BITS_1BITPARITY;
    mp->ramprotection.WidePortRamProtectionScheme
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.NarrowPort0ProcessorInterface
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.NarrowPort0DmaAccess
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.NarrowPort0SecondaryClient
        = SB_FE2000_MM_35BITS_1BITPARITY;
    mp->ramprotection.NarrowPort0PrimaryClient
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.NarrowPort1ProcessorInterface
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.NarrowPort1DmaAccess
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.NarrowPort1SecondaryClient
        = SB_FE2000_MM_35BITS_1BITPARITY;
    mp->ramprotection.NarrowPort1PrimaryClient
        = SB_FE2000_MM_32BITS_4BITPARITY;


    BCM_SBX_IF_ERROR_RETURN(sbFe2000SetQueueParams(unit, ip));

    return SOC_E_NONE;
}

static int
_soc_sbx_g2p3_xt_hw_params_init(int unit)
{
    int i;
    soc_sbx_g2p3_state_t *fe = SOC_SBX_CONTROL(unit)->drv;
    sbFe2000XtInitParams_t *ip;
    sbFe2000XtInitParamsMm_t *mp;
    soc_sbx_g2p3_memory_params_t mparams;
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);

    ip = &SOC_SBX_CFG_FE2000(unit)->xt_init_params;

    for( i = 0; i < SB_FE2000XT_LR_NUMBER_OF_STREAMS; i++ ) {
        ip->lr.bStreamOnline[i] = !ip->lr.bLrpBypass;
    }

    mp = &(ip->mm[0]); 

    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_memory_params_get(unit, SB_FE2000_MEM_MM0_INTERNAL_0,
                                        &mparams));
    fe->DmaCfg.mm0i0wide = (mparams.width == 64);
    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_memory_params_get(unit, SB_FE2000_MEM_MM0_INTERNAL_1,
                                        &mparams));
    fe->DmaCfg.mm0i1wide = (mparams.width == 64);
    if (!fe->DmaCfg.mm0i0wide && !fe->DmaCfg.mm0i1wide) {
        mp->ramconfiguration.InternalRamConfiguration
            = SB_FE2000_MM_RAM0_16KBY36_AND_RAM1_16KBY36;
    } else if (fe->DmaCfg.mm0i0wide && !fe->DmaCfg.mm0i1wide) {
        mp->ramconfiguration.InternalRamConfiguration
            = SB_FE2000_MM_RAM0_8KBY72_AND_RAM1_16KBY36;
    } else if (!fe->DmaCfg.mm0i0wide && fe->DmaCfg.mm0i1wide) {
        mp->ramconfiguration.InternalRamConfiguration
            = SB_FE2000_MM_RAM0_16KBY36_AND_RAM1_8KBY72;
    } else {
        mp->ramconfiguration.InternalRamConfiguration
            = SB_FE2000_MM_RAM0_8KBY72_AND_RAM1_8KBY72;
    }

    mp->ramconfiguration.NarrowPortRam0Configuration
        = SB_FE2000_MM_ONE_BY_18_RAM_CONNECTED;
    mp->ramconfiguration.NarrowPortRam0Words = 2 * 1024 * 1024;
    mp->ramconfiguration.NarrowPortRam1Configuration
        = SB_FE2000_MM_ONE_BY_18_RAM_CONNECTED;
    mp->ramconfiguration.NarrowPortRam1Words = 2 * 1024 * 1024;
    mp->ramconfiguration.WidePortRamConfiguration
        = SB_FE2000_MM_ONE_BY_36_RAM_CONNECTED;
    mp->ramconfiguration.WidePortRamWords = 1 * 1024 * 1024;

    mp->ramconnections.Lrp0MemoryConnection
        = SB_FE2000_MM_LRP0_CONNECTED_TO_NARROWPORT_DDRII_PORT0;
    mp->ramconnections.Lrp1MemoryConnection
        = SB_FE2000_MM_LRP1_CONNECTED_TO_NARROWPORT_DDRII_PORT1;
    mp->ramconnections.Lrp2MemoryConnection
        = SB_FE2000_MM_LRP2_CONNECTED_TO_NO_RESOURCE;
    mp->ramconnections.Lrp3MemoryConnection
        = SB_FE2000_MM_LRP3_CONNECTED_TO_INTERNAL_SINGLE_RAM1;
    mp->ramconnections.Lrp4MemoryConnection
        = SB_FE2000_MM_LRP4_CONNECTED_TO_WIDEPORT_DDRII_PORT;
    mp->ramconnections.PmuMemoryConnection
        = SB_FE2000_MM_PMU_CONNECTED_TO_NO_RESOURCE;
    mp->ramconnections.Cmu0MemoryConnection
        = SB_FE2000_MM_CMU0_CONNECTED_TO_INTERNAL_SINGLE_RAM0;
    mp->ramconnections.Cmu1MemoryConnection
        = SB_FE2000_MM_CMU1_CONNECTED_TO_NARROWPORT_DDRII_PORT1;
    mp->ramprotection.InternalRam0ProtectionScheme
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.InternalRam1ProtectionScheme
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.WidePortRamProtectionScheme
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.NarrowPort0ProcessorInterface
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.NarrowPort0DmaAccess
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.NarrowPort0SecondaryClient
        = SB_FE2000_MM_35BITS_1BITPARITY;
    mp->ramprotection.NarrowPort0PrimaryClient
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.NarrowPort1ProcessorInterface
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.NarrowPort1DmaAccess
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.NarrowPort1SecondaryClient
        = SB_FE2000_MM_35BITS_1BITPARITY;
    mp->ramprotection.NarrowPort1PrimaryClient
        = SB_FE2000_MM_32BITS_4BITPARITY;

    mp = &(ip->mm[1]); 

    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_memory_params_get(unit, SB_FE2000_MEM_MM1_INTERNAL_0,
                                        &mparams));
    fe->DmaCfg.mm1i0wide = (mparams.width == 64);
    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_memory_params_get(unit, SB_FE2000_MEM_MM1_INTERNAL_1,
                                        &mparams));
    fe->DmaCfg.mm1i1wide = (mparams.width == 64);
    if (!fe->DmaCfg.mm1i0wide && !fe->DmaCfg.mm1i1wide) {
        mp->ramconfiguration.InternalRamConfiguration
            = SB_FE2000_MM_RAM0_16KBY36_AND_RAM1_16KBY36;
    } else if (fe->DmaCfg.mm1i0wide && !fe->DmaCfg.mm1i1wide) {
        mp->ramconfiguration.InternalRamConfiguration
            = SB_FE2000_MM_RAM0_8KBY72_AND_RAM1_16KBY36;
    } else if (!fe->DmaCfg.mm1i0wide && fe->DmaCfg.mm1i1wide) {
        mp->ramconfiguration.InternalRamConfiguration
            = SB_FE2000_MM_RAM0_16KBY36_AND_RAM1_8KBY72;
    } else {
        mp->ramconfiguration.InternalRamConfiguration
            = SB_FE2000_MM_RAM0_8KBY72_AND_RAM1_8KBY72;
    }

    mp->ramconnections.Lrp0MemoryConnection
        = SB_FE2000_MM_LRP0_CONNECTED_TO_NARROWPORT_DDRII_PORT0;
    mp->ramconnections.Lrp1MemoryConnection
        = SB_FE2000_MM_LRP1_CONNECTED_TO_NARROWPORT_DDRII_PORT1;
    mp->ramconnections.Lrp2MemoryConnection
        = SB_FE2000_MM_LRP2_CONNECTED_TO_INTERNAL_SINGLE_RAM0;
    mp->ramconnections.Lrp3MemoryConnection
        = SB_FE2000_MM_LRP3_CONNECTED_TO_INTERNAL_SINGLE_RAM1;
    mp->ramconnections.Lrp4MemoryConnection
        = SB_FE2000_MM_LRP4_CONNECTED_TO_WIDEPORT_DDRII_PORT;
    mp->ramconnections.PmuMemoryConnection
        = SB_FE2000_MM_PMU_CONNECTED_TO_WIDEPORT_DDRII;
    mp->ramconnections.Cmu0MemoryConnection
        = SB_FE2000_MM_CMU0_CONNECTED_TO_NARROWPORT_DDRII_PORT0;
    mp->ramconnections.Cmu1MemoryConnection
        = SB_FE2000_MM_CMU1_CONNECTED_TO_NARROWPORT_DDRII_PORT1;

    mp->ramprotection.InternalRam0ProtectionScheme
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.InternalRam1ProtectionScheme
        = SB_FE2000_MM_35BITS_1BITPARITY;
    mp->ramprotection.WidePortRamProtectionScheme
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.NarrowPort0ProcessorInterface
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.NarrowPort0DmaAccess
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.NarrowPort0SecondaryClient
        = SB_FE2000_MM_35BITS_1BITPARITY;
    mp->ramprotection.NarrowPort0PrimaryClient
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.NarrowPort1ProcessorInterface
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.NarrowPort1DmaAccess
        = SB_FE2000_MM_32BITS_4BITPARITY;
    mp->ramprotection.NarrowPort1SecondaryClient
        = SB_FE2000_MM_35BITS_1BITPARITY;
    mp->ramprotection.NarrowPort1PrimaryClient
        = SB_FE2000_MM_32BITS_4BITPARITY;

    if (fe2kAsm2IntD__getNc((fe2kAsm2IntD*)sbx->ucode, "elen", &ip->uEpoch) > 0){
        return SOC_E_FAIL;
    }

    BCM_SBX_IF_ERROR_RETURN(sbFe2000XtSetQueueParams(unit, ip));

    return SOC_E_NONE;
}
