/*
 * $Id: g2xx.c 1.7 Broadcom SDK $
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
#include <soc/sbx/g2xx/g2xx_int.h>
#include <soc/sbx/g2xx/g2xx_ucode.h>
#include <bcm_int/sbx/error.h>

static int _soc_sbx_g2xx_hw_params_init(int unit);
static int _soc_sbx_g2xx_xt_hw_params_init(int unit);
int _soc_sbx_g2xx_commons_init(int unit);
static int _soc_sbx_g2xx_ucode_update(int unit);

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
_g2xx_isr(void *_unit, uint32 ireg)
{
    int unit = (int) PTR_TO_INT(_unit);
    soc_sbx_control_t *sbx;
    soc_sbx_g2xx_state_t *fe;
    uint32 interrupt;

    sbx = SOC_SBX_CONTROL(unit);
    assert(sbx);

    fe = (soc_sbx_g2xx_state_t *) sbx->drv;

    interrupt = ireg;

    INTR_DEBUG(("\ng2xx_isr invoked with:0x%08x\n", interrupt));

    /* check for DMA completion */
    if ( interrupt & (SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, DMA_0_DONE_DISINT)
                    | SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, DMA_1_DONE_DISINT)
                    | SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, DMA_2_DONE_DISINT)
                    | SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, DMA_3_DONE_DISINT)
                    | SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT, DMA_SG_DONE)) ) {
        /* in SIM, just call ISR */
        INTR_DEBUG(("%d:g2xx_isr: calling sbFe2000DmaIsr\n", unit));
        sbFe2000DmaIsr(fe->pDmaMgr);
    }

    sbFe2000CmuRingProcess(fe->pCmuMgr,0);

    /* disable unsupported CM errors */
    SAND_HAL_FE2000_WRITE(sbx->sbhdl, CM_SEGMENT_FLUSH_EVENT_MASK, 0xffffffff);
    SAND_HAL_FE2000_WRITE(sbx->sbhdl, CM_ERROR_MASK, 0xffffffff);
    SAND_HAL_FE2000_WRITE(sbx->sbhdl, CM_SEGMENT_RANGE_ERROR_MASK, 0xffffffff);
    SAND_HAL_FE2000_WRITE(sbx->sbhdl, CM_SEGMENT_DISABLE_ERROR_MASK, 0xffffffff);

}

#define MAX_CONTIG_MALLOC 0x2000
int
_soc_sbx_g2xx_commons_init(int unit)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    soc_sbx_g2xx_state_t *fe = sbx->drv;
    uint32_t uHppFreq;

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

    BCM_SBX_IF_ERROR_RETURN(sbFe2000CmuSwInit(&fe->pCmuMgr, 4096, 256,
                                              fe->regSet, thin_getHba,
                                              fe->pDmaMgr));
    BCM_SBX_IF_ERROR_RETURN(sbFe2000CmuHwInit(fe->pCmuMgr, 0));
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

    SOC_IRQ_MASK(unit) &=
      ~(SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, DMA_0_DONE_DISINT)
        | SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, DMA_1_DONE_DISINT)
        | SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, DMA_2_DONE_DISINT)
        | SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, DMA_3_DONE_DISINT)
        | SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, COUNTER_FIFO_DATA_AVAIL_DISINT)
        | SAND_HAL_FE2000_FIELD_MASK(sbx->sbhdl, PC_INTERRUPT_MASK, UNIT_DISINT));

    SAND_HAL_FE2000_WRITE(sbx->sbhdl, PC_INTERRUPT_MASK, SOC_IRQ_MASK(unit));

    uHppFreq = soc_property_get(unit, spn_BCM88025_HPP_FREQ,
                                SB_FE2000XT_HPP_FREQ_375000);
    if (uHppFreq < 1000) {
        uHppFreq = uHppFreq * 1000;
    }
#if 0
    BCM_SBX_IF_ERROR_RETURN(sbFe2000PmInit(uHppFreq, &fe->pm, &pmconfig));
#endif

#if 1
    BCM_SBX_IF_ERROR_RETURN
        (soc_sbx_g2xx_pp_init(unit, &fe->cParams, &fe->iParams));
    BCM_SBX_IF_ERROR_RETURN
        (soc_sbx_g2xx_pp_hw_init(unit, NULL, fe->regSet));
#endif

    BCM_SBX_IF_ERROR_RETURN(sbFe2000BringUpLr(fe->regSet,
                                              (uint8 *) fe->ucode));
    return SOC_E_NONE;
}

/* allow the SDK to override some ucode parameters at startup*/
static int
_soc_sbx_g2xx_sdk_overrides(int unit)
{
    soc_sbx_control_t *sbx;
    soc_sbx_g2xx_state_t *fe;
    soc_sbx_g2xx_table_manager_t *tm;

    sbx = SOC_SBX_CONTROL(unit);
    if (sbx == NULL) {
        return SOC_E_INIT;
    }

    fe = sbx->drv;
    if (fe == NULL) {
        return SOC_E_MEMORY;
    }
    
    tm = fe->tableinfo;

    return SOC_E_NONE;
}


static int
_soc_sbx_g2xx_ucode_update(int unit)
{
    soc_sbx_control_t *sbx;
    sbx = SOC_SBX_CONTROL(unit);
    if (sbx == NULL) {
        return SOC_E_INIT;
    }

    SOC_IF_ERROR_RETURN(soc_sbx_g2xx_init(unit));

    SOC_IF_ERROR_RETURN(_soc_sbx_g2xx_sdk_overrides(unit));

    return SOC_E_NONE;
}

int
soc_sbx_g2xx_drv_hw_init(int unit)
{
    soc_sbx_control_t *sbx;
    soc_sbx_g2xx_state_t *fe;

    sbx = SOC_SBX_CONTROL(unit);
    if (!sbx) {
        return SOC_E_INIT;
    }

    sbx->drv = sal_alloc(sizeof(soc_sbx_g2xx_state_t), "g2xx state");
    if (!sbx->drv) {
        return SOC_E_MEMORY;
    }
    sal_memset(sbx->drv, 0, sizeof(soc_sbx_g2xx_state_t));

    /* load ucode buffer, need for soc_sbx_g2xx_init() */
    sbx->ucodetype = SOC_SBX_UCODE_TYPE_G2XX;

    fe =sbx->drv;
    fe->ucode = &fe->ucodestr;
    sbx->ucode = fe->ucode;

    /* Load ucode for hw parameters available */
    sbx->ucode_update_func = NULL;
    
    if (SOC_IS_SBX_FE2KXT(unit)) {
        if (SOC_SBX_LOAD_UCODE(soc_sbx_g2xx_fe2kAsm2UcodeEc) != 0) {
           return SOC_E_FAIL;
        }
    } else {
        return SOC_E_FAIL;
    }

    SOC_IF_ERROR_RETURN(soc_sbx_g2xx_init(unit));

    if (SOC_IS_SBX_FE2KXT(unit)) {
        SOC_IF_ERROR_RETURN(_soc_sbx_g2xx_xt_hw_params_init(unit));
    }else{
        SOC_IF_ERROR_RETURN(_soc_sbx_g2xx_hw_params_init(unit));
    }

    return SOC_E_NONE;
}


int
soc_sbx_g2xx_drv_init(int unit)
{
    soc_sbx_control_t *sbx;
    soc_sbx_g2xx_state_t *fe;
    char *ucodestr = NULL;

    sbx = SOC_SBX_CONTROL(unit);
    if (!sbx) {
        return SOC_E_INIT;
    }
    fe = sbx->drv;

    sbx->ucodetype = SOC_SBX_UCODE_TYPE_G2XX;

    fe->ucode = &fe->ucodestr;
    sbx->ucode = fe->ucode;

    sbx->ucode_update_func = _soc_sbx_g2xx_ucode_update;
    
    if (SOC_IS_SBX_FE2KXT(unit)) {
        ucodestr = soc_property_get_str(unit, spn_BCM88025_UCODE);
        /* respect named code if specified */
        if (ucodestr){
            if ( (sal_strlen(ucodestr) == sal_strlen("c2_g2xx")) && (sal_strcmp(ucodestr, "c2_g2xx") == 0) ) {
                if (SOC_SBX_LOAD_UCODE(soc_sbx_g2xx_fe2kAsm2UcodeEc) != 0) {
                    return SOC_E_FAIL;
                }
            }else{
                soc_cm_print("soc_sbx_fe2000xt_init() Unsupported ucode detected\n");
                return SOC_E_FAIL;
            }
        }
    } else {
        return SOC_E_FAIL;
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

    sbx->ucode_update_func = _soc_sbx_g2xx_ucode_update;

    /* get header length from the ucode */
    if (fe2kAsm2IntD__getNc((fe2kAsm2IntD*)sbx->ucode, "G2XX_ERH_LENGTH", &fe->erhlen) > 0){
        return SOC_E_FAIL;
    }

    return SOC_E_NONE;
}

int
soc_sbx_g2xx_ilib_init(int unit)
{
    soc_sbx_control_t *sbx;
    soc_sbx_g2xx_state_t *fe;

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
    sbx->lib_isr = _g2xx_isr;

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


    SOC_IF_ERROR_RETURN(_soc_sbx_g2xx_commons_init(unit));
    
    return SOC_E_NONE;
}


soc_sbx_g2xx_state_t *
soc_sbx_g2xx_unit2fe(int unit)
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

int soc_sbx_g2xx_is_fe2kxt(int unit)
{
    soc_sbx_control_t *sbx;

    sbx = SOC_SBX_CONTROL(unit);
    return SAND_HAL_IS_FE2KXT(sbx->sbhdl);

}

int
soc_sbx_g2xx_topci_qid_get(int unit, int *qid)
{
    soc_sbx_control_t *sbx;
    soc_sbx_g2xx_state_t *fe;

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
soc_sbx_g2xx_frompci_qid_get(int unit, int *qid)
{
    soc_sbx_control_t *sbx;
    soc_sbx_g2xx_state_t *fe;

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
_soc_sbx_g2xx_hw_params_init(int unit)
{
    int i;
    soc_sbx_g2xx_state_t *fe = SOC_SBX_CONTROL(unit)->drv;
    sbFe2000InitParams_t *ip;
    sbFe2000InitParamsMm_t *mp;
    soc_sbx_g2xx_memory_params_t mparams;

    ip = &SOC_SBX_CFG_FE2000(unit)->init_params;

    for( i = 0; i < SB_FE2000_LR_NUMBER_OF_STREAMS; i++ ) {
        ip->lr.bStreamOnline[i] = !ip->lr.bLrpBypass;
    }

    mp = &(ip->mm[0]); 

    SOC_IF_ERROR_RETURN
        (soc_sbx_g2xx_memory_params_get(unit, SB_FE2000_MEM_MM0_INTERNAL_0,
                                        &mparams));
    fe->DmaCfg.mm0i0wide = (mparams.width == 64);
    SOC_IF_ERROR_RETURN
        (soc_sbx_g2xx_memory_params_get(unit, SB_FE2000_MEM_MM0_INTERNAL_1,
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
        (soc_sbx_g2xx_memory_params_get(unit, SB_FE2000_MEM_MM1_INTERNAL_0,
                                        &mparams));
    fe->DmaCfg.mm1i0wide = (mparams.width == 64);
    SOC_IF_ERROR_RETURN
        (soc_sbx_g2xx_memory_params_get(unit, SB_FE2000_MEM_MM1_INTERNAL_1,
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
_soc_sbx_g2xx_xt_hw_params_init(int unit)
{
    int i;
    soc_sbx_g2xx_state_t *fe = SOC_SBX_CONTROL(unit)->drv;
    sbFe2000XtInitParams_t *ip;
    sbFe2000XtInitParamsMm_t *mp;
    soc_sbx_g2xx_memory_params_t mparams;
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);

    ip = &SOC_SBX_CFG_FE2000(unit)->xt_init_params;

    for( i = 0; i < SB_FE2000XT_LR_NUMBER_OF_STREAMS; i++ ) {
        ip->lr.bStreamOnline[i] = !ip->lr.bLrpBypass;
    }

    mp = &(ip->mm[0]); 

    SOC_IF_ERROR_RETURN
        (soc_sbx_g2xx_memory_params_get(unit, SB_FE2000_MEM_MM0_INTERNAL_0,
                                        &mparams));
    fe->DmaCfg.mm0i0wide = (mparams.width == 64);
    SOC_IF_ERROR_RETURN
        (soc_sbx_g2xx_memory_params_get(unit, SB_FE2000_MEM_MM0_INTERNAL_1,
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
        (soc_sbx_g2xx_memory_params_get(unit, SB_FE2000_MEM_MM1_INTERNAL_0,
                                        &mparams));
    fe->DmaCfg.mm1i0wide = (mparams.width == 64);
    SOC_IF_ERROR_RETURN
        (soc_sbx_g2xx_memory_params_get(unit, SB_FE2000_MEM_MM1_INTERNAL_1,
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
