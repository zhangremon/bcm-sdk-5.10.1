/*
 * $Id: tucana.c 1.6.26.1 Broadcom SDK $
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
 * File:        tucana.c
 * Purpose:
 * Requires:
 */

#include <sal/core/boot.h>

#include <soc/drv.h>
#include <soc/error.h>
#include <soc/debug.h>
#include <soc/mem.h>
#include <soc/tucana.h>

#ifdef BCM_TUCANA_SUPPORT

/*
 * Tucana chip driver functions.
 */
soc_functions_t soc_tucana_drv_funs = {
    soc_tucana_misc_init,
    soc_tucana_mmu_init,
    soc_tucana_age_timer_get,
    soc_tucana_age_timer_max_get,
    soc_tucana_age_timer_set,
};

int
soc_tucana_misc_init(int unit)
{
    /* Enable transmit and receive in GPICS */
    uint32		regval, memval;
    uint64		reg64;
    soc_port_t          port;
    int                 i;

    /*
     * Reset the ARL block
     */
    SOC_IF_ERROR_RETURN(READ_ARL_CONTROLr(unit, &regval));

    /*
     * Parity memory in ARL requires initialization
     * Writes to parity memory is possible only when parity
     * generation/checking is disabled
     */
    soc_reg_field_set(unit, ARL_CONTROLr, &regval, PARITY_ENf, 0);
    SOC_IF_ERROR_RETURN(WRITE_ARL_CONTROLr(unit, regval));
    memval = 0;
    for (i = 0; i <= SOC_MEM_INFO(unit, L3X_PARITYm).index_max; i++) {
        SOC_IF_ERROR_RETURN
            (WRITE_L3X_PARITYm(unit, COPYNO_ALL, i, &memval));
    }
    soc_reg_field_set(unit, ARL_CONTROLr, &regval, PARITY_ENf, 1);

    soc_reg_field_set(unit, ARL_CONTROLr, &regval, ARL_INITf, 1);
    SOC_IF_ERROR_RETURN(WRITE_ARL_CONTROLr(unit, regval));
    /* For simulation, sleep 1 sec.  Otherwise, sleep 5 ms */
    if (SAL_BOOT_SIMULATION) {
        sal_usleep(1000000);
    } else {
        sal_usleep(5000);
    }
    soc_reg_field_set(unit, ARL_CONTROLr, &regval, ARL_INITf, 0);
    SOC_IF_ERROR_RETURN(WRITE_ARL_CONTROLr(unit, regval));

    if (!SAL_BOOT_SIMULATION) {
        for (i = 0; i <= SOC_MEM_INFO(unit, DEFIP_HIm).index_max; i++) {
            SOC_IF_ERROR_RETURN
                (WRITE_DEFIP_HIm(unit, COPYNO_ALL, i, &memval));
        }

        for (i = 0; i <= SOC_MEM_INFO(unit, DEFIP_LOm).index_max; i++) {
            SOC_IF_ERROR_RETURN
                (WRITE_DEFIP_LOm(unit, COPYNO_ALL, i, &memval));
        }
    }

    /* Enable ARL parity interrupts */
    soc_intr_enable(unit, IRQ_ARL_LPM_LO_PAR | IRQ_ARL_LPM_HI_PAR | 
                    IRQ_ARL_L3_PAR | IRQ_ARL_L2_PAR | IRQ_ARL_VLAN_PAR);

    COMPILER_64_ZERO(reg64);
    soc_reg64_field32_set(unit, EGR_ENABLEr, &reg64, PRT_ENABLEf, 1);
    PBMP_FE_ITER(unit, port) {
	SOC_IF_ERROR_RETURN(WRITE_EGR_ENABLEr(unit, port, reg64));
    }

    COMPILER_64_ZERO(reg64);
    soc_reg64_field32_set(unit, GEGR_ENABLEr, &reg64, PRT_ENABLEf, 1);
    PBMP_GE_ITER(unit, port) {
	SOC_IF_ERROR_RETURN(WRITE_GEGR_ENABLEr(unit, port, reg64));
    }

    COMPILER_64_ZERO(reg64);
    soc_reg64_field32_set(unit, IEGR_ENABLEr, &reg64, PRT_ENABLEf, 1);
    PBMP_HG_ITER(unit, port) {
        SOC_IF_ERROR_RETURN(WRITE_IEGR_ENABLEr(unit, port, reg64));
    }

    COMPILER_64_ZERO(reg64);
    soc_reg64_field32_set(unit, EPC_LINKr, &reg64, PORT_BITMAP_0f,
                          SOC_PBMP_WORD_GET(PBMP_CMIC(unit), 0));
    soc_reg64_field32_set(unit, EPC_LINKr, &reg64, PORT_BITMAP_1f,
                          SOC_PBMP_WORD_GET(PBMP_CMIC(unit), 1));
    PBMP_E_ITER(unit, port) {
	SOC_IF_ERROR_RETURN(WRITE_EPC_LINKr(unit, port, reg64));
    }

    COMPILER_64_ZERO(reg64);
    soc_reg64_field32_set(unit, ILINKr, &reg64, PORT_BITMAP_0f,
                          SOC_PBMP_WORD_GET(PBMP_ALL(unit), 0));
    soc_reg64_field32_set(unit, ILINKr, &reg64, PORT_BITMAP_1f,
                          SOC_PBMP_WORD_GET(PBMP_ALL(unit), 1));
    PBMP_HG_ITER(unit, port) {
	SOC_IF_ERROR_RETURN(WRITE_ILINKr(unit, port, reg64));
    }

    /* GMAC init should be moved to mac */
    PBMP_GE_ITER(unit, port) {
        SOC_IF_ERROR_RETURN(READ_GMACC1r(unit, port, &reg64));
        soc_reg64_field32_set(unit, GMACC1r, &reg64, TXEN0f, 1);
        soc_reg64_field32_set(unit, GMACC1r, &reg64, RXEN0f, 1);
	SOC_IF_ERROR_RETURN(WRITE_GMACC1r(unit, port, reg64));
    }

    /*
     * Set up the PRTABLE
     */
    COMPILER_64_ZERO(reg64);
    soc_reg64_field32_set(unit, PRTABLE_DEFAULTr, &reg64, VLAN_IDf,
			  VLAN_ID_DEFAULT);
    PBMP_E_ITER(unit, port) {
        SOC_IF_ERROR_RETURN(WRITE_PRTABLE_DEFAULTr(unit, port, reg64));
    }

    return SOC_E_NONE;
}

#define TUCANA_NUM_COS  8

/* This must take place during the mmu init sequence. */
STATIC int
_soc_tucana_xqcos_init(int unit)
{
    uint32	val;
    soc_port_t	port;
    uint32      val30, val74;
    int         i, wgt[TUCANA_NUM_COS];

    for (i = 0; i < TUCANA_NUM_COS; i++) {
        wgt[i] =
            soc_property_suffix_num_get(unit, i, spn_MMU_XQ_WEIGHT,
                "cos", 1) + ((i > 0) ? wgt[i - 1] : 0);
    }

    /*
     * Active cosqs (more or less) equally share the 64 segments of the
     * XQ memory associated with each port.
     */
    val30 = val74 = 0;

#define	SETVAL(_n)	(val = (_n * 64 / wgt[TUCANA_NUM_COS - 1]))

    SETVAL(wgt[0]);
    soc_reg_field_set(unit, XQCOSENTRIES0_3r, 
                      &val30, COS0_ENDADDRESSf, val);
    SETVAL(wgt[1]);
    soc_reg_field_set(unit, XQCOSENTRIES0_3r, 
                      &val30, COS1_ENDADDRESSf, val);
    SETVAL(wgt[2]);
    soc_reg_field_set(unit, XQCOSENTRIES0_3r, 
                      &val30, COS2_ENDADDRESSf, val);
    SETVAL(wgt[3]);
    soc_reg_field_set(unit, XQCOSENTRIES0_3r, 
                      &val30, COS3_ENDADDRESSf, val);
    SETVAL(wgt[4]);
    soc_reg_field_set(unit, XQCOSENTRIES4_7r, 
                      &val74, COS4_ENDADDRESSf, val);
    SETVAL(wgt[5]);
    soc_reg_field_set(unit, XQCOSENTRIES4_7r, 
                      &val74, COS5_ENDADDRESSf, val);
    SETVAL(wgt[6]);
    soc_reg_field_set(unit, XQCOSENTRIES4_7r, 
                      &val74, COS6_ENDADDRESSf, val);
    SETVAL(wgt[7]);
    soc_reg_field_set(unit, XQCOSENTRIES4_7r, 
                      &val74, COS7_ENDADDRESSf, val);
#undef	SETVAL
	
    PBMP_ALL_ITER(unit, port) {
        /* Commit calculated values above  for each port */
        SOC_IF_ERROR_RETURN(WRITE_XQCOSENTRIES0_3r(unit, port, val30));
        SOC_IF_ERROR_RETURN(WRITE_XQCOSENTRIES4_7r(unit, port, val74));
    }

    return SOC_E_NONE;
}

int
soc_tucana_mmu_init(int unit)
{
    uint32          val;
    uint64          val64;
    soc_port_t      port;
    int             i, dll_delay, has_ipic;
    int             mmu_reset_num, mmu_reset_max, ddr_16bit;

    has_ipic = (NUM_HG_PORT(unit) != 0);

    mmu_reset_max = soc_property_get(unit, spn_MMU_RESET_TRIES, 10);
    ddr_16bit = soc_property_get(unit, spn_MCU_16BIT_DDR, 0);

    if (SAL_BOOT_SIMULATION) {
        SOC_IF_ERROR_RETURN(READ_MISCCONFIGr(unit, &val));
        soc_reg_field_set(unit, MISCCONFIGr, &val, MMU_SOFT_RESET_Lf, 0);
        soc_reg_field_set(unit, MISCCONFIGr, &val, CTRREADCLEARENf, 1);
        SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, val));

        /***************** Set Max BW commitment *****************/
        SOC_IF_ERROR_RETURN(READ_MAXBWCOMMITMENTr(unit, &val));
        soc_reg_field_set(unit, MAXBWCOMMITMENTr, &val, MAXBWCG0f, 0x37a);
        if (has_ipic) {
            soc_reg_field_set(unit, MAXBWCOMMITMENTr, &val, MAXBWCG1f, 0x37a);
        }
        SOC_IF_ERROR_RETURN(WRITE_MAXBWCOMMITMENTr(unit, val));

        /***************** Bring DLL, etc, out of reset *****************/
        SOC_IF_ERROR_RETURN(READ_MCU_DLL_CONTROLr(unit, &val));
        soc_reg_field_set(unit, MCU_DLL_CONTROLr, &val, RESETf, 0);
        SOC_IF_ERROR_RETURN(WRITE_MCU_DLL_CONTROLr(unit, val));

        SOC_IF_ERROR_RETURN(READ_MCU_PLL_CONTROLr(unit, &val));
        soc_reg_field_set(unit, MCU_PLL_CONTROLr, &val, RESETf, 0);
        SOC_IF_ERROR_RETURN(WRITE_MCU_PLL_CONTROLr(unit, val));

        SOC_IF_ERROR_RETURN(READ_MCU_MAIN_CONTROLr(unit, &val));
        soc_reg_field_set(unit, MCU_MAIN_CONTROLr, &val, RESETf, 0);
        SOC_IF_ERROR_RETURN(WRITE_MCU_MAIN_CONTROLr(unit, val));

        /***************** Set bypass modes, enable channels **************/
        SOC_IF_ERROR_RETURN(READ_MCU_PLL_CONTROLr(unit, &val));
        soc_reg_field_set(unit, MCU_PLL_CONTROLr, &val, BYPASSf, 1);
        SOC_IF_ERROR_RETURN(WRITE_MCU_PLL_CONTROLr(unit, val));

        SOC_IF_ERROR_RETURN(READ_MCU_MAIN_CONTROLr(unit, &val));
        soc_reg_field_set(unit, MCU_MAIN_CONTROLr, &val, ENABLE_CH01f, 1);
        if (has_ipic) {
            soc_reg_field_set(unit, MCU_MAIN_CONTROLr, &val, ENABLE_CH23f, 1);
        }
        soc_reg_field_set(unit, MCU_MAIN_CONTROLr, &val, CHN_INITf, 0);
        SOC_IF_ERROR_RETURN(WRITE_MCU_MAIN_CONTROLr(unit, val));

        /***************** Set up MCU channel config registers ************/
        SOC_IF_ERROR_RETURN(READ_MCU_CHN0_CONFIGr(unit, &val64));
        soc_reg64_field32_set(unit, MCU_CHN0_CONFIGr, &val64, TINITf, 0x10);
        soc_reg64_field32_set(unit, MCU_CHN0_CONFIGr, &val64, TDLLf, 1);
        soc_reg64_field32_set(unit, MCU_CHN0_CONFIGr, &val64, 
                              ROW_ADDR_BITSf, 2);
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN0_CONFIGr(unit, val64));

        SOC_IF_ERROR_RETURN(READ_MCU_CHN1_CONFIGr(unit, &val64));
        soc_reg64_field32_set(unit, MCU_CHN1_CONFIGr, &val64, TINITf, 0x10);
        soc_reg64_field32_set(unit, MCU_CHN1_CONFIGr, &val64, TDLLf, 1);
        soc_reg64_field32_set(unit, MCU_CHN1_CONFIGr, &val64, 
                              ROW_ADDR_BITSf, 2);
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN1_CONFIGr(unit, val64));

        if (has_ipic) {
            SOC_IF_ERROR_RETURN(READ_MCU_CHN2_CONFIGr(unit, &val64));
            soc_reg64_field32_set(unit, MCU_CHN2_CONFIGr, &val64,
                                  TINITf, 0x10);
            soc_reg64_field32_set(unit, MCU_CHN2_CONFIGr, &val64,
                                  TDLLf, 1);
            soc_reg64_field32_set(unit, MCU_CHN2_CONFIGr, &val64,
                                  ROW_ADDR_BITSf, 2);
            SOC_IF_ERROR_RETURN(WRITE_MCU_CHN2_CONFIGr(unit, val64));

            SOC_IF_ERROR_RETURN(READ_MCU_CHN3_CONFIGr(unit, &val64));
            soc_reg64_field32_set(unit, MCU_CHN3_CONFIGr, &val64,
                                  TINITf, 0x10);
            soc_reg64_field32_set(unit, MCU_CHN3_CONFIGr, &val64,
                                  TDLLf, 1);
            soc_reg64_field32_set(unit, MCU_CHN3_CONFIGr, &val64,
                                  ROW_ADDR_BITSf, 2);
            SOC_IF_ERROR_RETURN(WRITE_MCU_CHN3_CONFIGr(unit, val64));
        }

        /***************** MCU timing *****************/
        SOC_IF_ERROR_RETURN(READ_MCU_CHN0_TIMINGr(unit, &val64));
        soc_reg64_field32_set(unit, MCU_CHN0_TIMINGr, &val64, TMRDf, 2);
        soc_reg64_field32_set(unit, MCU_CHN0_TIMINGr, &val64, TRCf, 0xc);
        soc_reg64_field32_set(unit, MCU_CHN0_TIMINGr, &val64, TRFCf, 0xe);
        soc_reg64_field32_set(unit, MCU_CHN0_TIMINGr, &val64, TRCDRf, 4);
        soc_reg64_field32_set(unit, MCU_CHN0_TIMINGr, &val64, TRCDWf, 2);
        soc_reg64_field32_set(unit, MCU_CHN0_TIMINGr, &val64, TRASf, 8);
        soc_reg64_field32_set(unit, MCU_CHN0_TIMINGr, &val64, TRPf, 4);
        soc_reg64_field32_set(unit, MCU_CHN0_TIMINGr, &val64, TRRDf, 2);
        soc_reg64_field32_set(unit, MCU_CHN0_TIMINGr, &val64, TWTRf, 1);
        soc_reg64_field32_set(unit, MCU_CHN0_TIMINGr, &val64, TWRf, 2);
        soc_reg64_field32_set(unit, MCU_CHN0_TIMINGr, &val64, TCWDf, 1);
        soc_reg64_field32_set(unit, MCU_CHN0_TIMINGr, &val64, TCRDf, 3);
        soc_reg64_field32_set(unit, MCU_CHN0_TIMINGr, &val64, TCRDHf, 0);
        soc_reg64_field32_set(unit, MCU_CHN0_TIMINGr, &val64, TW2RIDLEf, 1);
        soc_reg64_field32_set(unit, MCU_CHN0_TIMINGr, &val64, TR2WIDLEf, 1);
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN0_TIMINGr(unit, val64));

        SOC_IF_ERROR_RETURN(READ_MCU_CHN1_TIMINGr(unit, &val64));
        soc_reg64_field32_set(unit, MCU_CHN1_TIMINGr, &val64, TMRDf, 2);
        soc_reg64_field32_set(unit, MCU_CHN1_TIMINGr, &val64, TRCf, 0xc);
        soc_reg64_field32_set(unit, MCU_CHN1_TIMINGr, &val64, TRFCf, 0xe);
        soc_reg64_field32_set(unit, MCU_CHN1_TIMINGr, &val64, TRCDRf, 4);
        soc_reg64_field32_set(unit, MCU_CHN1_TIMINGr, &val64, TRCDWf, 2);
        soc_reg64_field32_set(unit, MCU_CHN1_TIMINGr, &val64, TRASf, 8);
        soc_reg64_field32_set(unit, MCU_CHN1_TIMINGr, &val64, TRPf, 4);
        soc_reg64_field32_set(unit, MCU_CHN1_TIMINGr, &val64, TRRDf, 2);
        soc_reg64_field32_set(unit, MCU_CHN1_TIMINGr, &val64, TWTRf, 1);
        soc_reg64_field32_set(unit, MCU_CHN1_TIMINGr, &val64, TWRf, 2);
        soc_reg64_field32_set(unit, MCU_CHN1_TIMINGr, &val64, TCWDf, 1);
        soc_reg64_field32_set(unit, MCU_CHN1_TIMINGr, &val64, TCRDf, 3);
        soc_reg64_field32_set(unit, MCU_CHN1_TIMINGr, &val64, TCRDHf, 0);
        soc_reg64_field32_set(unit, MCU_CHN1_TIMINGr, &val64, TW2RIDLEf, 1);
        soc_reg64_field32_set(unit, MCU_CHN1_TIMINGr, &val64, TR2WIDLEf, 1);
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN1_TIMINGr(unit, val64));

        if (has_ipic) {
        SOC_IF_ERROR_RETURN(READ_MCU_CHN2_TIMINGr(unit, &val64));
        soc_reg64_field32_set(unit, MCU_CHN2_TIMINGr, &val64, TMRDf, 2);
        soc_reg64_field32_set(unit, MCU_CHN2_TIMINGr, &val64, TRCf, 0xc);
        soc_reg64_field32_set(unit, MCU_CHN2_TIMINGr, &val64, TRFCf, 0xe);
        soc_reg64_field32_set(unit, MCU_CHN2_TIMINGr, &val64, TRCDRf, 4);
        soc_reg64_field32_set(unit, MCU_CHN2_TIMINGr, &val64, TRCDWf, 2);
        soc_reg64_field32_set(unit, MCU_CHN2_TIMINGr, &val64, TRASf, 8);
        soc_reg64_field32_set(unit, MCU_CHN2_TIMINGr, &val64, TRPf, 4);
        soc_reg64_field32_set(unit, MCU_CHN2_TIMINGr, &val64, TRRDf, 2);
        soc_reg64_field32_set(unit, MCU_CHN2_TIMINGr, &val64, TWTRf, 1);
        soc_reg64_field32_set(unit, MCU_CHN2_TIMINGr, &val64, TWRf, 2);
        soc_reg64_field32_set(unit, MCU_CHN2_TIMINGr, &val64, TCWDf, 1);
        soc_reg64_field32_set(unit, MCU_CHN2_TIMINGr, &val64, TCRDf, 3);
        soc_reg64_field32_set(unit, MCU_CHN2_TIMINGr, &val64, TCRDHf, 0);
        soc_reg64_field32_set(unit, MCU_CHN2_TIMINGr, &val64, TW2RIDLEf, 1);
        soc_reg64_field32_set(unit, MCU_CHN2_TIMINGr, &val64, TR2WIDLEf, 1);
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN2_TIMINGr(unit, val64));

        SOC_IF_ERROR_RETURN(READ_MCU_CHN3_TIMINGr(unit, &val64));
        soc_reg64_field32_set(unit, MCU_CHN3_TIMINGr, &val64, TMRDf, 2);
        soc_reg64_field32_set(unit, MCU_CHN3_TIMINGr, &val64, TRCf, 0xc);
        soc_reg64_field32_set(unit, MCU_CHN3_TIMINGr, &val64, TRFCf, 0xe);
        soc_reg64_field32_set(unit, MCU_CHN3_TIMINGr, &val64, TRCDRf, 4);
        soc_reg64_field32_set(unit, MCU_CHN3_TIMINGr, &val64, TRCDWf, 2);
        soc_reg64_field32_set(unit, MCU_CHN3_TIMINGr, &val64, TRASf, 8);
        soc_reg64_field32_set(unit, MCU_CHN3_TIMINGr, &val64, TRPf, 4);
        soc_reg64_field32_set(unit, MCU_CHN3_TIMINGr, &val64, TRRDf, 2);
        soc_reg64_field32_set(unit, MCU_CHN3_TIMINGr, &val64, TWTRf, 1);
        soc_reg64_field32_set(unit, MCU_CHN3_TIMINGr, &val64, TWRf, 2);
        soc_reg64_field32_set(unit, MCU_CHN3_TIMINGr, &val64, TCWDf, 1);
        soc_reg64_field32_set(unit, MCU_CHN3_TIMINGr, &val64, TCRDf, 3);
        soc_reg64_field32_set(unit, MCU_CHN3_TIMINGr, &val64, TCRDHf, 0);
        soc_reg64_field32_set(unit, MCU_CHN3_TIMINGr, &val64, TW2RIDLEf, 1);
        soc_reg64_field32_set(unit, MCU_CHN3_TIMINGr, &val64, TR2WIDLEf, 1);
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN3_TIMINGr(unit, val64));
        }

        /***************** MCU delay control *****************/
        SOC_IF_ERROR_RETURN(READ_MCU_CHN0_DELAY_CTLr(unit, &val));
        soc_reg_field_set(unit, MCU_CHN0_DELAY_CTLr, &val, DQI_ADJ_VALf,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_DELAY_DQI_ADJ_VAL, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN0_DELAY_CTLr, &val, DQI_ADJ_DIRf,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_DELAY_DQI_ADJ_DIR, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN0_DELAY_CTLr, &val, ADDR_ADJ_VALf,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_DELAY_ADDR_ADJ_VAL, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN0_DELAY_CTLr, &val, ADDR_ADJ_DIRf,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_DELAY_ADDR_ADJ_DIR, "ch", 0));
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN0_DELAY_CTLr(unit, val));

        SOC_IF_ERROR_RETURN(READ_MCU_CHN1_DELAY_CTLr(unit, &val));
        soc_reg_field_set(unit, MCU_CHN1_DELAY_CTLr, &val, DQI_ADJ_VALf,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_DELAY_DQI_ADJ_VAL, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN1_DELAY_CTLr, &val, DQI_ADJ_DIRf,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_DELAY_DQI_ADJ_DIR, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN1_DELAY_CTLr, &val, ADDR_ADJ_VALf,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_DELAY_ADDR_ADJ_VAL, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN1_DELAY_CTLr, &val, ADDR_ADJ_DIRf,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_DELAY_ADDR_ADJ_DIR, "ch", 0));
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN1_DELAY_CTLr(unit, val));

        /* Higig optimization (needed for Magnum) */
        SOC_IF_ERROR_RETURN(READ_HGOPT0r(unit, &val));
        soc_reg_field_set(unit, HGOPT0r, &val, DDR_SPEEDf, 0);
        SOC_IF_ERROR_RETURN(WRITE_HGOPT0r(unit, val));

        if (has_ipic) {
            SOC_IF_ERROR_RETURN(READ_MCU_CHN2_DELAY_CTLr(unit, &val));
            soc_reg_field_set(unit, MCU_CHN2_DELAY_CTLr, &val, DQI_ADJ_VALf,
                              soc_property_suffix_num_get(unit, 0, 
                                  spn_MCU_DELAY_DQI_ADJ_VAL, "ch", 0));
            soc_reg_field_set(unit, MCU_CHN2_DELAY_CTLr, &val, DQI_ADJ_DIRf,
                              soc_property_suffix_num_get(unit, 0, 
                                  spn_MCU_DELAY_DQI_ADJ_DIR, "ch", 0));
            soc_reg_field_set(unit, MCU_CHN2_DELAY_CTLr, &val, ADDR_ADJ_VALf,
                              soc_property_suffix_num_get(unit, 0, 
                                  spn_MCU_DELAY_ADDR_ADJ_VAL, "ch", 0));
            soc_reg_field_set(unit, MCU_CHN2_DELAY_CTLr, &val, ADDR_ADJ_DIRf,
                              soc_property_suffix_num_get(unit, 0, 
                                  spn_MCU_DELAY_ADDR_ADJ_DIR, "ch", 0));
           SOC_IF_ERROR_RETURN(WRITE_MCU_CHN2_DELAY_CTLr(unit, val));

           SOC_IF_ERROR_RETURN(READ_MCU_CHN3_DELAY_CTLr(unit, &val));
           soc_reg_field_set(unit, MCU_CHN3_DELAY_CTLr, &val, DQI_ADJ_VALf,
                             soc_property_suffix_num_get(unit, 0, 
                                 spn_MCU_DELAY_DQI_ADJ_VAL, "ch", 0));
           soc_reg_field_set(unit, MCU_CHN3_DELAY_CTLr, &val, DQI_ADJ_DIRf,
                             soc_property_suffix_num_get(unit, 0, 
                                 spn_MCU_DELAY_DQI_ADJ_DIR, "ch", 0));
           soc_reg_field_set(unit, MCU_CHN3_DELAY_CTLr, &val, ADDR_ADJ_VALf,
                             soc_property_suffix_num_get(unit, 0, 
                                 spn_MCU_DELAY_ADDR_ADJ_VAL, "ch", 0));
           soc_reg_field_set(unit, MCU_CHN3_DELAY_CTLr, &val, ADDR_ADJ_DIRf,
                             soc_property_suffix_num_get(unit, 0, 
                                 spn_MCU_DELAY_ADDR_ADJ_DIR, "ch", 0));
            SOC_IF_ERROR_RETURN(WRITE_MCU_CHN3_DELAY_CTLr(unit, val));
         
            /* Higig optimization */
            SOC_IF_ERROR_RETURN(READ_HGOPT0r(unit, &val));
            soc_reg_field_set(unit, HGOPT0r, &val, DDR_SPEEDf, 0);
            soc_reg_field_set(unit, HGOPT0r, &val, MAX_LATENCY_EMPTYf, 0x11);
            soc_reg_field_set(unit, HGOPT0r, &val, MAX_LATENCYf, 0xd);
            soc_reg_field_set(unit, HGOPT0r, &val, REFRESH_LATENCYf, 0xe);
            SOC_IF_ERROR_RETURN(WRITE_HGOPT0r(unit, val));

            SOC_IF_ERROR_RETURN(READ_HGOPT1r(unit, &val));
            soc_reg_field_set(unit, HGOPT1r, &val, DDR_SPEEDf, 0);
            soc_reg_field_set(unit, HGOPT1r, &val, MAX_LATENCY_EMPTYf, 0x11);
            soc_reg_field_set(unit, HGOPT1r, &val, MAX_LATENCYf, 0xd);
            soc_reg_field_set(unit, HGOPT1r, &val, REFRESH_LATENCYf, 0xe);
            SOC_IF_ERROR_RETURN(WRITE_HGOPT1r(unit, val));
        }

        SOC_IF_ERROR_RETURN(READ_SMALLINGBUFFERTHRESr(unit, &val));
        soc_reg_field_set(unit, SMALLINGBUFFERTHRESr, &val, 
                          LOW_WATERMARKf, 0x8);
        soc_reg_field_set(unit, SMALLINGBUFFERTHRESr, &val, 
                          HIGH_WATERMARKf, 0x24);
        SOC_IF_ERROR_RETURN(WRITE_SMALLINGBUFFERTHRESr(unit, val));

        /***************** Re-init main control *****************/
        SOC_IF_ERROR_RETURN(READ_MCU_MAIN_CONTROLr(unit, &val));
        soc_reg_field_set(unit, MCU_MAIN_CONTROLr, &val, CHN_INITf, 1);
        SOC_IF_ERROR_RETURN(WRITE_MCU_MAIN_CONTROLr(unit, val));

        /* Sleep 2 second on QT. */
        sal_usleep(2000000);

        if (SAL_BOOT_QUICKTURN) {
            /* ASSERT:  MCU_MAIN_STATUSr.CHN_INIT_DONEf == 1 */
            SOC_IF_ERROR_RETURN(READ_MCU_MAIN_STATUSr(unit, &val));
            if (soc_reg_field_get(unit, MCU_MAIN_STATUSr, val, CHN_INIT_DONEf)
                != 1) {
                soc_cm_debug(DK_WARN, 
                             "WARNING:  5665 MCU channel init done != 1\n");
            }
        }

        /* Must be done before lifting reset */
        SOC_IF_ERROR_RETURN(_soc_tucana_xqcos_init(unit));

        SOC_IF_ERROR_RETURN(READ_MISCCONFIGr(unit, &val));
        soc_reg_field_set(unit, MISCCONFIGr, &val, MMU_SOFT_RESET_Lf, 1);
        SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, val));
    } else { /* Real HW */
        /********** Disable MMU Pointer initialization **********/
        SOC_IF_ERROR_RETURN(READ_MISCCONFIGr(unit, &val));
        soc_reg_field_set(unit, MISCCONFIGr, &val, MMU_SOFT_RESET_Lf, 0);
        soc_reg_field_set(unit, MISCCONFIGr, &val, DISABLELOCKLOSSMSGSf, 1);
        SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, val));

        SOC_IF_ERROR_RETURN(READ_MCU_MAIN_CONTROLr(unit, &val));
        soc_reg_field_set(unit, MCU_MAIN_CONTROLr, &val, RESETf, 1);
        SOC_IF_ERROR_RETURN(WRITE_MCU_MAIN_CONTROLr(unit, val));

        SOC_IF_ERROR_RETURN(READ_MCU_PLL_CONTROLr(unit, &val));
        soc_reg_field_set(unit, MCU_PLL_CONTROLr, &val, RESETf, 0);
        soc_reg_field_set(unit, MCU_PLL_CONTROLr, &val, MDIVf, 1);
        soc_reg_field_set(unit, MCU_PLL_CONTROLr, &val, N1DIVf, 0x07);
        SOC_IF_ERROR_RETURN(WRITE_MCU_PLL_CONTROLr(unit, val));

        sal_usleep(5000);

        /* Must have MCU_PLL_STATUSr.LOCK_DETf == 1, else abort */
        for (mmu_reset_num = 0; mmu_reset_num < mmu_reset_max;
             mmu_reset_num++) {
            SOC_IF_ERROR_RETURN(READ_MCU_PLL_STATUSr(unit, &val));
            if (soc_reg_field_get(unit, MCU_PLL_STATUSr, val, LOCK_DETf)
                == 1) {
                break;
            }
            sal_usleep(5000);
        }

        if (mmu_reset_num == mmu_reset_max) {
            soc_cm_debug(DK_ERR,
                         "ERROR:  MCU PLL initialization failed\n");
            return SOC_E_FAIL;
        }

        /***************** Set bypass modes, enable channels **************/
        SOC_IF_ERROR_RETURN(READ_MCU_DLL_CONTROLr(unit, &val));
        soc_reg_field_set(unit, MCU_DLL_CONTROLr, &val, RESETf, 0);
        soc_reg_field_set(unit, MCU_DLL_CONTROLr, &val, BYPASSf, 1);
        soc_reg_field_set(unit, MCU_DLL_CONTROLr, &val, TEST_SELECTf, 3);
        SOC_IF_ERROR_RETURN(WRITE_MCU_DLL_CONTROLr(unit, val));

        sal_usleep(5000);

        /* Must have MCU_DLL_STATUSr.LOCK_DETECTf == 1, else abort */
        for (mmu_reset_num = 0; mmu_reset_num < mmu_reset_max;
             mmu_reset_num++) {
            SOC_IF_ERROR_RETURN(READ_MCU_DLL_STATUSr(unit, &val));
            if (soc_reg_field_get(unit, MCU_DLL_STATUSr, val, LOCK_DETECTf)
                == 1) {
                break;
            }
            sal_usleep(5000);
        }

        if (mmu_reset_num == mmu_reset_max) {
            soc_cm_debug(DK_ERR,
                         "ERROR:  MCU DLL initialization failed\n");
            return SOC_E_FAIL;
        }

        /* Bypass DLL with early lock value to prevent drift */
        SOC_IF_ERROR_RETURN(READ_MCU_DLL_STATUSr(unit, &val));
        dll_delay = soc_reg_field_get(unit, MCU_DLL_STATUSr, val, DLL_DELAYf);
        SOC_IF_ERROR_RETURN(READ_MCU_DLL_CONTROLr(unit, &val));
        soc_reg_field_set(unit, MCU_DLL_CONTROLr, &val, BYPASSf, 0);
        soc_reg_field_set(unit, MCU_DLL_CONTROLr, &val, BYPASS_VALf, dll_delay);
        SOC_IF_ERROR_RETURN(WRITE_MCU_DLL_CONTROLr(unit, val));

        SOC_IF_ERROR_RETURN(READ_MCU_MAIN_CONTROLr(unit, &val));
        soc_reg_field_set(unit, MCU_MAIN_CONTROLr, &val, RESETf, 0);
        soc_reg_field_set(unit, MCU_MAIN_CONTROLr, &val, ENABLE_CH01f, 1);
        if (has_ipic) {
            soc_reg_field_set(unit, MCU_MAIN_CONTROLr, &val, ENABLE_CH23f, 1);
        }
        SOC_IF_ERROR_RETURN(WRITE_MCU_MAIN_CONTROLr(unit, val));

        /***************** Set up MCU channel config registers ************/
        SOC_IF_ERROR_RETURN(READ_MCU_CHN0_CONFIGr(unit, &val64));
        soc_reg64_field32_set(unit, MCU_CHN0_CONFIGr, &val64, TREFf, 1428);
        soc_reg64_field32_set(unit, MCU_CHN0_CONFIGr, &val64, TINITf, 3000);
        soc_reg64_field32_set(unit, MCU_CHN0_CONFIGr, &val64, TDLLf, 200);
        soc_reg64_field32_set(unit, MCU_CHN0_CONFIGr, &val64,
                              RDRTN_DLY_ENf, 1);
        soc_reg64_field32_set(unit, MCU_CHN0_CONFIGr, &val64,
                              RDRTN_DLY_CYCLESf, 5);
        if (ddr_16bit) {
            soc_reg64_field32_set(unit, MCU_CHN0_CONFIGr, &val64,
                                  APRE_BITf, 1);
        }
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN0_CONFIGr(unit, val64));

        SOC_IF_ERROR_RETURN(READ_MCU_CHN1_CONFIGr(unit, &val64));
        soc_reg64_field32_set(unit, MCU_CHN1_CONFIGr, &val64, TREFf, 1428);
        soc_reg64_field32_set(unit, MCU_CHN1_CONFIGr, &val64, TINITf, 3000);
        soc_reg64_field32_set(unit, MCU_CHN1_CONFIGr, &val64, TDLLf, 200);
        soc_reg64_field32_set(unit, MCU_CHN1_CONFIGr, &val64,
                              RDRTN_DLY_ENf, 1);
        soc_reg64_field32_set(unit, MCU_CHN1_CONFIGr, &val64,
                              RDRTN_DLY_CYCLESf, 5);
        if (ddr_16bit) {
            soc_reg64_field32_set(unit, MCU_CHN1_CONFIGr, &val64,
                                  APRE_BITf, 1);
        }
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN1_CONFIGr(unit, val64));

        if (has_ipic) {
            SOC_IF_ERROR_RETURN(READ_MCU_CHN2_CONFIGr(unit, &val64));
            soc_reg64_field32_set(unit, MCU_CHN2_CONFIGr, &val64,
                                  TREFf, 1428);
            soc_reg64_field32_set(unit, MCU_CHN2_CONFIGr, &val64,
                                  TINITf, 3000);
            soc_reg64_field32_set(unit, MCU_CHN2_CONFIGr, &val64,
                                  TDLLf, 200);
            soc_reg64_field32_set(unit, MCU_CHN2_CONFIGr, &val64,
                                  RDRTN_DLY_ENf, 1);
            soc_reg64_field32_set(unit, MCU_CHN2_CONFIGr, &val64,
                                  RDRTN_DLY_CYCLESf, 5);
            if (ddr_16bit) {
                soc_reg64_field32_set(unit, MCU_CHN2_CONFIGr, &val64,
                                      APRE_BITf, 1);
            }
            SOC_IF_ERROR_RETURN(WRITE_MCU_CHN2_CONFIGr(unit, val64));

            SOC_IF_ERROR_RETURN(READ_MCU_CHN3_CONFIGr(unit, &val64));
            soc_reg64_field32_set(unit, MCU_CHN3_CONFIGr, &val64,
                                  TREFf, 1428);
            soc_reg64_field32_set(unit, MCU_CHN3_CONFIGr, &val64,
                                  TINITf, 3000);
            soc_reg64_field32_set(unit, MCU_CHN3_CONFIGr, &val64,
                                  TDLLf, 200);
            soc_reg64_field32_set(unit, MCU_CHN3_CONFIGr, &val64,
                                  RDRTN_DLY_ENf, 1);
            soc_reg64_field32_set(unit, MCU_CHN3_CONFIGr, &val64,
                                  RDRTN_DLY_CYCLESf, 5);
            if (ddr_16bit) {
                soc_reg64_field32_set(unit, MCU_CHN3_CONFIGr, &val64,
                                      APRE_BITf, 1);
            }
            SOC_IF_ERROR_RETURN(WRITE_MCU_CHN3_CONFIGr(unit, val64));
        }

        /***************** MCU timing *****************/
        SOC_IF_ERROR_RETURN(READ_MCU_CHN0_TIMINGr(unit, &val64));
        soc_reg64_field32_set(unit, MCU_CHN0_TIMINGr, &val64, TMRDf, 2);
        soc_reg64_field32_set(unit, MCU_CHN0_TIMINGr, &val64, TRFCf, 0xe);
        soc_reg64_field32_set(unit, MCU_CHN0_TIMINGr, &val64, TCRDf, 4);
        if (ddr_16bit) {
            soc_reg64_field32_set(unit, MCU_CHN0_TIMINGr, &val64, TRCDWf, 3);
            soc_reg64_field32_set(unit, MCU_CHN0_TIMINGr, &val64, TWTRf, 2);
            soc_reg64_field32_set(unit, MCU_CHN0_TIMINGr, &val64, TWRf, 3);
        }
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN0_TIMINGr(unit, val64));

        SOC_IF_ERROR_RETURN(READ_MCU_CHN1_TIMINGr(unit, &val64));
        soc_reg64_field32_set(unit, MCU_CHN1_TIMINGr, &val64, TMRDf, 2);
        soc_reg64_field32_set(unit, MCU_CHN1_TIMINGr, &val64, TRFCf, 0xe);
        soc_reg64_field32_set(unit, MCU_CHN1_TIMINGr, &val64, TCRDf, 4);
        if (ddr_16bit) {
            soc_reg64_field32_set(unit, MCU_CHN1_TIMINGr, &val64, TRCDWf, 3);
            soc_reg64_field32_set(unit, MCU_CHN1_TIMINGr, &val64, TWTRf, 2);
            soc_reg64_field32_set(unit, MCU_CHN1_TIMINGr, &val64, TWRf, 3);
        }
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN1_TIMINGr(unit, val64));

        if (has_ipic) {
            SOC_IF_ERROR_RETURN(READ_MCU_CHN2_TIMINGr(unit, &val64));
            soc_reg64_field32_set(unit, MCU_CHN2_TIMINGr, &val64, TMRDf, 2);
            soc_reg64_field32_set(unit, MCU_CHN2_TIMINGr, &val64, TRFCf, 0xe);
            soc_reg64_field32_set(unit, MCU_CHN2_TIMINGr, &val64, TCRDf, 4);
            if (ddr_16bit) {
                soc_reg64_field32_set(unit, MCU_CHN2_TIMINGr, &val64,
                                      TRCDWf, 3);
                soc_reg64_field32_set(unit, MCU_CHN2_TIMINGr, &val64,
                                      TWTRf, 2);
                soc_reg64_field32_set(unit, MCU_CHN2_TIMINGr, &val64,
                                      TWRf, 3);
            }
            SOC_IF_ERROR_RETURN(WRITE_MCU_CHN2_TIMINGr(unit, val64));

            SOC_IF_ERROR_RETURN(READ_MCU_CHN3_TIMINGr(unit, &val64));
            soc_reg64_field32_set(unit, MCU_CHN3_TIMINGr, &val64, TMRDf, 2);
            soc_reg64_field32_set(unit, MCU_CHN3_TIMINGr, &val64, TRFCf, 0xe);
            soc_reg64_field32_set(unit, MCU_CHN3_TIMINGr, &val64, TCRDf, 4);
            if (ddr_16bit) {
                soc_reg64_field32_set(unit, MCU_CHN3_TIMINGr, &val64,
                                      TRCDWf, 3);
                soc_reg64_field32_set(unit, MCU_CHN3_TIMINGr, &val64,
                                      TWTRf, 2);
                soc_reg64_field32_set(unit, MCU_CHN3_TIMINGr, &val64,
                                      TWRf, 3);
            }
            SOC_IF_ERROR_RETURN(WRITE_MCU_CHN3_TIMINGr(unit, val64));
        }

        /************ SET DDR MR & EMR *************/

        SOC_IF_ERROR_RETURN(READ_MCU_CHN0_MODEr(unit, &val));
        soc_reg_field_set(unit, MCU_CHN0_MODEr, &val, DRV_STR_0f, 
                          soc_property_suffix_num_get(unit, 0, 
                                              spn_MCU_DRV_STR0, "ch", 1));
        soc_reg_field_set(unit, MCU_CHN0_MODEr, &val, DRV_STR_1f,
                          soc_property_suffix_num_get(unit, 0, 
                                              spn_MCU_DRV_STR1, "ch", 0));
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN0_MODEr(unit, val));

        SOC_IF_ERROR_RETURN(READ_MCU_CHN1_MODEr(unit, &val));
        soc_reg_field_set(unit, MCU_CHN1_MODEr, &val, DRV_STR_0f,
                          soc_property_suffix_num_get(unit, 1, 
                                              spn_MCU_DRV_STR0, "ch", 1));
        soc_reg_field_set(unit, MCU_CHN1_MODEr, &val, DRV_STR_1f,
                          soc_property_suffix_num_get(unit, 1, 
                                              spn_MCU_DRV_STR1, "ch", 0));
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN1_MODEr(unit, val));

        if (has_ipic) {
            SOC_IF_ERROR_RETURN(READ_MCU_CHN2_MODEr(unit, &val));
            soc_reg_field_set(unit, MCU_CHN2_MODEr, &val, DRV_STR_0f,
                          soc_property_suffix_num_get(unit, 2, 
                                                       spn_MCU_DRV_STR0, "ch", 1));
            soc_reg_field_set(unit, MCU_CHN2_MODEr, &val, DRV_STR_1f,
                          soc_property_suffix_num_get(unit, 2, 
                                                       spn_MCU_DRV_STR1, "ch", 0));
            SOC_IF_ERROR_RETURN(WRITE_MCU_CHN2_MODEr(unit, val));

            SOC_IF_ERROR_RETURN(READ_MCU_CHN3_MODEr(unit, &val));
            soc_reg_field_set(unit, MCU_CHN3_MODEr, &val, DRV_STR_0f,
                          soc_property_suffix_num_get(unit, 3, 
                                                       spn_MCU_DRV_STR0, "ch", 1));
            soc_reg_field_set(unit, MCU_CHN3_MODEr, &val, DRV_STR_1f,
                          soc_property_suffix_num_get(unit, 3, 
                                                       spn_MCU_DRV_STR1, "ch", 0));
            SOC_IF_ERROR_RETURN(WRITE_MCU_CHN3_MODEr(unit, val));
        }

        /************ SET PAD control *************/

        SOC_IF_ERROR_RETURN(READ_MCU_CHN0_PAD_CTLr(unit, &val));
        soc_reg_field_set(unit, MCU_CHN0_PAD_CTLr, &val, DATA_CLASS_2f,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_PAD_DATA_CLASS2, "ch", 1));
        soc_reg_field_set(unit, MCU_CHN0_PAD_CTLr, &val, DATA_PAD_DRIVEf,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_PAD_DATA_DRIVE, "ch", 3));
        soc_reg_field_set(unit, MCU_CHN0_PAD_CTLr, &val, DATA_PAD_SLEWf,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_PAD_DATA_SLEW, "ch", 3));
        soc_reg_field_set(unit, MCU_CHN0_PAD_CTLr, &val, ADDR_CLASS_2f,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_PAD_ADDR_CLASS2, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN0_PAD_CTLr, &val, ADDR_PAD_DRIVEf,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_PAD_ADDR_DRIVE, "ch", 3));
        soc_reg_field_set(unit, MCU_CHN0_PAD_CTLr, &val, ADDR_PAD_SLEWf,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_PAD_ADDR_SLEW, "ch", 3));
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN0_PAD_CTLr(unit, val));
 
        SOC_IF_ERROR_RETURN(READ_MCU_CHN1_PAD_CTLr(unit, &val));
        soc_reg_field_set(unit, MCU_CHN1_PAD_CTLr, &val, DATA_CLASS_2f,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_PAD_DATA_CLASS2, "ch", 1));
        soc_reg_field_set(unit, MCU_CHN1_PAD_CTLr, &val, DATA_PAD_DRIVEf,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_PAD_DATA_DRIVE, "ch", 3));
        soc_reg_field_set(unit, MCU_CHN1_PAD_CTLr, &val, DATA_PAD_SLEWf,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_PAD_DATA_SLEW, "ch", 3));
        soc_reg_field_set(unit, MCU_CHN1_PAD_CTLr, &val, ADDR_CLASS_2f,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_PAD_ADDR_CLASS2, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN1_PAD_CTLr, &val, ADDR_PAD_DRIVEf,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_PAD_ADDR_DRIVE, "ch", 3));
        soc_reg_field_set(unit, MCU_CHN1_PAD_CTLr, &val, ADDR_PAD_SLEWf,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_PAD_ADDR_SLEW, "ch", 3));
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN1_PAD_CTLr(unit, val));
 
        if (has_ipic) {
            SOC_IF_ERROR_RETURN(READ_MCU_CHN2_PAD_CTLr(unit, &val));
            soc_reg_field_set(unit, MCU_CHN2_PAD_CTLr, &val, DATA_CLASS_2f,
                              soc_property_suffix_num_get(unit, 2, 
                                  spn_MCU_PAD_DATA_CLASS2, "ch", 1));
            soc_reg_field_set(unit, MCU_CHN2_PAD_CTLr, &val, DATA_PAD_DRIVEf,
                              soc_property_suffix_num_get(unit, 2, 
                                  spn_MCU_PAD_DATA_DRIVE, "ch", 3));
            soc_reg_field_set(unit, MCU_CHN2_PAD_CTLr, &val, DATA_PAD_SLEWf,
                              soc_property_suffix_num_get(unit, 2, 
                                  spn_MCU_PAD_DATA_SLEW, "ch", 3));
            soc_reg_field_set(unit, MCU_CHN2_PAD_CTLr, &val, ADDR_CLASS_2f,
                              soc_property_suffix_num_get(unit, 2, 
                                  spn_MCU_PAD_ADDR_CLASS2, "ch", 0));
            soc_reg_field_set(unit, MCU_CHN2_PAD_CTLr, &val, ADDR_PAD_DRIVEf,
                              soc_property_suffix_num_get(unit, 2, 
                                  spn_MCU_PAD_ADDR_DRIVE, "ch", 3));
            soc_reg_field_set(unit, MCU_CHN2_PAD_CTLr, &val, ADDR_PAD_SLEWf,
                              soc_property_suffix_num_get(unit, 2, 
                                  spn_MCU_PAD_ADDR_SLEW, "ch", 3));
            SOC_IF_ERROR_RETURN(WRITE_MCU_CHN2_PAD_CTLr(unit, val));
 
            SOC_IF_ERROR_RETURN(READ_MCU_CHN3_PAD_CTLr(unit, &val));
            soc_reg_field_set(unit, MCU_CHN3_PAD_CTLr, &val, DATA_CLASS_2f,
                              soc_property_suffix_num_get(unit, 3, 
                                  spn_MCU_PAD_DATA_CLASS2, "ch", 1));
            soc_reg_field_set(unit, MCU_CHN3_PAD_CTLr, &val, DATA_PAD_DRIVEf,
                              soc_property_suffix_num_get(unit, 3, 
                                  spn_MCU_PAD_DATA_DRIVE, "ch", 3));
            soc_reg_field_set(unit, MCU_CHN3_PAD_CTLr, &val, DATA_PAD_SLEWf,
                              soc_property_suffix_num_get(unit, 3, 
                                  spn_MCU_PAD_DATA_SLEW, "ch", 3));
            soc_reg_field_set(unit, MCU_CHN3_PAD_CTLr, &val, ADDR_CLASS_2f,
                              soc_property_suffix_num_get(unit, 3, 
                                  spn_MCU_PAD_ADDR_CLASS2, "ch", 0));
            soc_reg_field_set(unit, MCU_CHN3_PAD_CTLr, &val, ADDR_PAD_DRIVEf,
                              soc_property_suffix_num_get(unit, 3, 
                                  spn_MCU_PAD_ADDR_DRIVE, "ch", 3));
            soc_reg_field_set(unit, MCU_CHN3_PAD_CTLr, &val, ADDR_PAD_SLEWf,
                              soc_property_suffix_num_get(unit, 3, 
                                  spn_MCU_PAD_ADDR_SLEW, "ch", 3));
            SOC_IF_ERROR_RETURN(WRITE_MCU_CHN3_PAD_CTLr(unit, val));
        }

        /***************** MCU delay control *****************/
        SOC_IF_ERROR_RETURN(READ_MCU_CHN0_DELAY_CTLr(unit, &val));
        soc_reg_field_set(unit, MCU_CHN0_DELAY_CTLr, &val, DQI_ADJ_VALf,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_DELAY_DQI_ADJ_VAL, "ch", 3));
        soc_reg_field_set(unit, MCU_CHN0_DELAY_CTLr, &val, DQI_ADJ_DIRf,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_DELAY_DQI_ADJ_DIR, "ch", 1));
        soc_reg_field_set(unit, MCU_CHN0_DELAY_CTLr, &val, ADDR_ADJ_VALf,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_DELAY_ADDR_ADJ_VAL, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN0_DELAY_CTLr, &val, ADDR_ADJ_DIRf,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_DELAY_ADDR_ADJ_DIR, "ch", 1));
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN0_DELAY_CTLr(unit, val));

        SOC_IF_ERROR_RETURN(READ_MCU_CHN1_DELAY_CTLr(unit, &val));
        soc_reg_field_set(unit, MCU_CHN1_DELAY_CTLr, &val, DQI_ADJ_VALf,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_DELAY_DQI_ADJ_VAL, "ch", 3));
        soc_reg_field_set(unit, MCU_CHN1_DELAY_CTLr, &val, DQI_ADJ_DIRf,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_DELAY_DQI_ADJ_DIR, "ch", 1));
        soc_reg_field_set(unit, MCU_CHN1_DELAY_CTLr, &val, ADDR_ADJ_VALf,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_DELAY_ADDR_ADJ_VAL, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN1_DELAY_CTLr, &val, ADDR_ADJ_DIRf,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_DELAY_ADDR_ADJ_DIR, "ch", 1));
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN1_DELAY_CTLr(unit, val));

        /* Higig optimization (needed for Magnum) */
        SOC_IF_ERROR_RETURN(READ_HGOPT0r(unit, &val));
        soc_reg_field_set(unit, HGOPT0r, &val, DDR_SPEEDf, 0);
        SOC_IF_ERROR_RETURN(WRITE_HGOPT0r(unit, val));

        if (has_ipic) {
            SOC_IF_ERROR_RETURN(READ_MCU_CHN2_DELAY_CTLr(unit, &val));
            soc_reg_field_set(unit, MCU_CHN2_DELAY_CTLr, &val, DQI_ADJ_VALf,
                              soc_property_suffix_num_get(unit, 2, 
                                  spn_MCU_DELAY_DQI_ADJ_VAL, "ch", 3));
            soc_reg_field_set(unit, MCU_CHN2_DELAY_CTLr, &val, DQI_ADJ_DIRf,
                              soc_property_suffix_num_get(unit, 2, 
                                  spn_MCU_DELAY_DQI_ADJ_DIR, "ch", 1));
            soc_reg_field_set(unit, MCU_CHN2_DELAY_CTLr, &val, ADDR_ADJ_VALf,
                              soc_property_suffix_num_get(unit, 2, 
                                  spn_MCU_DELAY_ADDR_ADJ_VAL, "ch", 0));
            soc_reg_field_set(unit, MCU_CHN2_DELAY_CTLr, &val, ADDR_ADJ_DIRf,
                              soc_property_suffix_num_get(unit, 2, 
                                  spn_MCU_DELAY_ADDR_ADJ_DIR, "ch", 1));
            SOC_IF_ERROR_RETURN(WRITE_MCU_CHN2_DELAY_CTLr(unit, val));

            SOC_IF_ERROR_RETURN(READ_MCU_CHN3_DELAY_CTLr(unit, &val));
            soc_reg_field_set(unit, MCU_CHN3_DELAY_CTLr, &val, DQI_ADJ_VALf,
                              soc_property_suffix_num_get(unit, 3, 
                                  spn_MCU_DELAY_DQI_ADJ_VAL, "ch", 3));
            soc_reg_field_set(unit, MCU_CHN3_DELAY_CTLr, &val, DQI_ADJ_DIRf,
                              soc_property_suffix_num_get(unit, 3, 
                                  spn_MCU_DELAY_DQI_ADJ_DIR, "ch", 1));
            soc_reg_field_set(unit, MCU_CHN3_DELAY_CTLr, &val, ADDR_ADJ_VALf,
                              soc_property_suffix_num_get(unit, 3, 
                                  spn_MCU_DELAY_ADDR_ADJ_VAL, "ch", 0));
            soc_reg_field_set(unit, MCU_CHN3_DELAY_CTLr, &val, ADDR_ADJ_DIRf,
                              soc_property_suffix_num_get(unit, 3, 
                                  spn_MCU_DELAY_ADDR_ADJ_DIR, "ch", 1));
            SOC_IF_ERROR_RETURN(WRITE_MCU_CHN3_DELAY_CTLr(unit, val));

            /* Higig optimization */
            SOC_IF_ERROR_RETURN(READ_HGOPT0r(unit, &val));
            soc_reg_field_set(unit, HGOPT0r, &val, DDR_SPEEDf, 0);
            soc_reg_field_set(unit, HGOPT0r, &val, MAX_LATENCY_EMPTYf, 0x11);
            soc_reg_field_set(unit, HGOPT0r, &val, MAX_LATENCYf, 0xd);
            soc_reg_field_set(unit, HGOPT0r, &val, REFRESH_LATENCYf, 0xe);
            SOC_IF_ERROR_RETURN(WRITE_HGOPT0r(unit, val));

            SOC_IF_ERROR_RETURN(READ_HGOPT1r(unit, &val));
            soc_reg_field_set(unit, HGOPT1r, &val, DDR_SPEEDf, 0);
            soc_reg_field_set(unit, HGOPT1r, &val, MAX_LATENCY_EMPTYf, 0x11);
            soc_reg_field_set(unit, HGOPT1r, &val, MAX_LATENCYf, 0xd);
            soc_reg_field_set(unit, HGOPT1r, &val, REFRESH_LATENCYf, 0xe);
            SOC_IF_ERROR_RETURN(WRITE_HGOPT1r(unit, val));
        }

        SOC_IF_ERROR_RETURN(READ_SMALLINGBUFFERTHRESr(unit, &val));
        soc_reg_field_set(unit, SMALLINGBUFFERTHRESr, &val, 
                          LOW_WATERMARKf, 0x8);
        soc_reg_field_set(unit, SMALLINGBUFFERTHRESr, &val, 
                          HIGH_WATERMARKf, 0x24);
        SOC_IF_ERROR_RETURN(WRITE_SMALLINGBUFFERTHRESr(unit, val));

        /***************** Set Max BW commitment *****************/
        SOC_IF_ERROR_RETURN(READ_MAXBWCOMMITMENTr(unit, &val));
        soc_reg_field_set(unit, MAXBWCOMMITMENTr, &val, MAXBWCG0f, 0x348);
        if (has_ipic) {
            soc_reg_field_set(unit, MAXBWCOMMITMENTr, &val, MAXBWCG1f, 0x348);
        }
        SOC_IF_ERROR_RETURN(WRITE_MAXBWCOMMITMENTr(unit, val));

        SOC_IF_ERROR_RETURN(READ_MCU_MAIN_CONTROLr(unit, &val));
        soc_reg_field_set(unit, MCU_MAIN_CONTROLr, &val, CHN_INITf, 1);
        SOC_IF_ERROR_RETURN(WRITE_MCU_MAIN_CONTROLr(unit, val));

        sal_usleep(5000);

        /* Must have MCU_MAIN_STATUSr.CHN_INIT_DONEf == 1, else abort */
        for (mmu_reset_num = 0; mmu_reset_num < mmu_reset_max;
             mmu_reset_num++) {
            SOC_IF_ERROR_RETURN(READ_MCU_MAIN_STATUSr(unit, &val));
            if (soc_reg_field_get(unit, MCU_MAIN_STATUSr, val, CHN_INIT_DONEf)
                == 1) {
                break;
            }
            sal_usleep(5000);
        }

        if (mmu_reset_num == mmu_reset_max) {
            soc_cm_debug(DK_ERR,
                         "ERROR:  MCU channel initialization failed\n");
            return SOC_E_FAIL;
        }

        /* Must be done before lifting reset */
        SOC_IF_ERROR_RETURN(_soc_tucana_xqcos_init(unit));

        SOC_IF_ERROR_RETURN(READ_MISCCONFIGr(unit, &val));
        soc_reg_field_set(unit, MISCCONFIGr, &val, MMU_SOFT_RESET_Lf, 1);
        SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, val));

        /* Init MMU counters that are not affected by reset */
        SOC_PBMP_ITER(PBMP_ALL(unit), port) {
            for (i = 0; i < NUM_COS(unit); i++) {
                SOC_IF_ERROR_RETURN(WRITE_MRPCOSr(unit, port, i, 0));
            }
            SOC_IF_ERROR_RETURN(WRITE_MRCUSE0r(unit, port, 0));
        }
        if (has_ipic) {
            SOC_IF_ERROR_RETURN(WRITE_MRCUSE1r(unit, 0));
        }
    }

    if (soc_property_get(unit, spn_BYPASS_MCU, 0)) {
        SOC_IF_ERROR_RETURN(READ_MISCCONFIGr(unit, &val));
        soc_reg_field_set(unit, MISCCONFIGr, &val, SHORTCIRCUITENf, 1);
        soc_reg_field_set(unit, MISCCONFIGr, &val, DISABLECRCMSGSf, 1);
        SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, val));

        soc_cm_print("Using MCU bypass mode\n");
    }

    return SOC_E_NONE;
}

int
soc_tucana_age_timer_get(int unit, int *age_seconds, int *enabled)
{
    uint32 value;

    SOC_IF_ERROR_RETURN(READ_ARL_AGE_TIMERr(unit, &value));
    *enabled = soc_reg_field_get(unit, ARL_AGE_TIMERr, value, AGE_ENAf);
    *age_seconds = soc_reg_field_get(unit, ARL_AGE_TIMERr, value, AGE_VALf);

    return SOC_E_NONE;
}

int
soc_tucana_age_timer_max_get(int unit, int *max_seconds)
{
    *max_seconds =
        soc_reg_field_get(unit, ARL_AGE_TIMERr, 0xffffffff, AGE_VALf);

    return SOC_E_NONE;
}

int
soc_tucana_age_timer_set(int unit, int age_seconds, int enable)
{
    uint32 value;

    value = 0;
    soc_reg_field_set(unit, ARL_AGE_TIMERr, &value, AGE_ENAf, enable);
    soc_reg_field_set(unit, ARL_AGE_TIMERr, &value, AGE_VALf, age_seconds);
    SOC_IF_ERROR_RETURN(WRITE_ARL_AGE_TIMERr(unit, value));

    return SOC_E_NONE;
}

/* Parity error handlers */

STATIC void
_soc_tucana_parity(int unit,
                   int blk,
                   uint32 index,
                   char *memname,
                   int memindex)
{
    if (memindex < 0) {
	soc_cm_debug(DK_ERR,
		     "unit %d memory %s.%s[%d]: parity error\n",
		     unit, memname, SOC_BLOCK_NAME(unit, blk),
		     index);
    } else {
	soc_cm_debug(DK_ERR,
		     "unit %d memory %s.%s.%d[%d]: parity error\n",
		     unit, memname, SOC_BLOCK_NAME(unit, blk),
		     memindex, index);
    }
}

STATIC void
_soc_tucana_mmu_parity(int unit,
                       char *memname)
{
	soc_cm_debug(DK_ERR,
		     "unit %d MMU parity error in %s\n",
		     unit, memname);
}

int
soc_tucana_arl_parity_error(int unit, int blk, int mem)
{
    uint32	regval, paraddr;
    soc_cm_debug(DK_ERR,
                 "Tucana ARL parity analysis\n");

    /* Brute force our way through the errors */
    if ((mem == VLAN_TABm) || (mem < 0)) {
        SOC_IF_ERROR_RETURN(READ_ARL_QVLAN_PARITY_STATUSr(unit, &regval));
        if (soc_reg_field_get(unit, ARL_QVLAN_PARITY_STATUSr, 
                              regval, PARITY_ERRORf)) {
            paraddr = soc_reg_field_get(unit, ARL_QVLAN_PARITY_STATUSr, 
                                        regval, VLAN_IDf);
            _soc_tucana_parity(unit, blk, paraddr,
                               SOC_MEM_NAME(unit, VLAN_TABm), -1);
            SOC_IF_ERROR_RETURN(WRITE_ARL_QVLAN_PARITY_STATUSr(unit, 0xffff));
            if (mem >= 0) {
                soc_intr_enable(unit, IRQ_ARL_VLAN_PAR);
            }
        }
    }

    if ((mem == L2Xm) || (mem < 0)) {
        SOC_IF_ERROR_RETURN(READ_ARL_L2_PARITY_STATUSr(unit, &regval));
        if (soc_reg_field_get(unit, ARL_L2_PARITY_STATUSr, 
                              regval, PARITY_ERRORf)) {
            paraddr = soc_reg_field_get(unit, ARL_L2_PARITY_STATUSr, 
                                        regval, ADDRf);
            _soc_tucana_parity(unit, blk, paraddr,
                               SOC_MEM_NAME(unit, L2Xm), -1);
            SOC_IF_ERROR_RETURN(WRITE_ARL_L2_PARITY_STATUSr(unit, 0xffff));
            if (mem >= 0) {
                soc_intr_enable(unit, IRQ_ARL_L2_PAR);
            }
        }
    }

    if ((mem == L3Xm) || (mem < 0)) {
        SOC_IF_ERROR_RETURN(READ_ARL_L3_PARITY_STATUSr(unit, &regval));
        if (soc_reg_field_get(unit, ARL_L3_PARITY_STATUSr, 
                              regval, PARITY_ERRORf)) {
            paraddr = soc_reg_field_get(unit, ARL_L3_PARITY_STATUSr, 
                                        regval, ADDRf);
            _soc_tucana_parity(unit, blk, paraddr,
                               SOC_MEM_NAME(unit, L3Xm), -1);
            SOC_IF_ERROR_RETURN(WRITE_ARL_L3_PARITY_STATUSr(unit, 0xffff));
            if (mem >= 0) {
                soc_intr_enable(unit, IRQ_ARL_L3_PAR);
            }
        }
    }

    if ((mem == DEFIP_HIm) || (mem < 0)) {
        SOC_IF_ERROR_RETURN(READ_ARL_DEFIP_HI_PARITY_STATUSr(unit, &regval));
        if (soc_reg_field_get(unit, ARL_DEFIP_HI_PARITY_STATUSr, 
                              regval, PARITY_ERRORf)) {
            paraddr = soc_reg_field_get(unit, ARL_DEFIP_HI_PARITY_STATUSr, 
                                        regval, ADDRf);
            _soc_tucana_parity(unit, blk, paraddr,
                               SOC_MEM_NAME(unit, DEFIP_HIm), -1);
            SOC_IF_ERROR_RETURN(WRITE_ARL_DEFIP_HI_PARITY_STATUSr(unit, 0xffff));
            if (mem >= 0) {
                soc_intr_enable(unit, IRQ_ARL_LPM_HI_PAR);
            }
        }
    }

    if ((mem == DEFIP_LOm) || (mem < 0)) {
        SOC_IF_ERROR_RETURN(READ_ARL_DEFIP_LO_PARITY_STATUSr(unit, &regval));
        if (soc_reg_field_get(unit, ARL_DEFIP_LO_PARITY_STATUSr, 
                              regval, PARITY_ERRORf)) {
            paraddr = soc_reg_field_get(unit, ARL_DEFIP_LO_PARITY_STATUSr, 
                                        regval, VLAN_IDf);
            _soc_tucana_parity(unit, blk, paraddr,
                               SOC_MEM_NAME(unit, DEFIP_LOm), -1);
            SOC_IF_ERROR_RETURN(WRITE_ARL_DEFIP_LO_PARITY_STATUSr(unit, 0xffff));
            if (mem >= 0) {
                soc_intr_enable(unit, IRQ_ARL_LPM_LO_PAR);
            }
        }
    }

    return SOC_E_NONE;
}

int
soc_tucana_mmu_parity_error(int unit, int blk, uint32 info)
{
    if (SOC_CONTROL(unit)->mmu_error_block) {
        /* Internal mask of 1st MMU error after mem testing */
        SOC_CONTROL(unit)->mmu_error_block = FALSE;
        return SOC_E_NONE;
    }

    if (info & SOC_TUCANA_PARITY_XQ) {
        _soc_tucana_mmu_parity(unit, "XQ");
    }

    if (info & SOC_TUCANA_PARITY_EBUF_HG) {
        _soc_tucana_mmu_parity(unit, "EBUF_HG");
    }

    if (info & SOC_TUCANA_PARITY_EBUF) {
        _soc_tucana_mmu_parity(unit, "EBUF");
    }

    if (info & SOC_TUCANA_PARITY_CELLASMB) {
        _soc_tucana_mmu_parity(unit, "Cell Assembly");
    }

    if (info & SOC_TUCANA_PARITY_RELMGR) {
        _soc_tucana_mmu_parity(unit, "Release Ptr");
    }

    if (info & SOC_TUCANA_PARITY_CCC) {
        _soc_tucana_mmu_parity(unit, "Copy Count Pool");
    }

    if (info & SOC_TUCANA_PARITY_CCPTR) {
        _soc_tucana_mmu_parity(unit, "Copy Count Pointers");
    }

    if (info & SOC_TUCANA_PARITY_CG1M1_FIFO) {
        _soc_tucana_mmu_parity(unit, "CFAP.CG1M1");
    }

    if (info & SOC_TUCANA_PARITY_CG1M0_FIFO) {
        _soc_tucana_mmu_parity(unit, "CFAP.CG1M0");
    }

    if (info & SOC_TUCANA_PARITY_CG0M1_FIFO) {
        _soc_tucana_mmu_parity(unit, "CFAP.CG0M1");
    }

    if (info & SOC_TUCANA_PARITY_CG0M0_FIFO) {
        _soc_tucana_mmu_parity(unit, "CFAP.CG0M0");
    }

    return SOC_E_NONE;
}

const char *_soc_tucana_crc_mems[] = {
    "CG0.M0",
    "CG0.M1",
    "CG1.M0",
    "CG1.M1",
    NULL
};

int
soc_tucana_mmu_crc_error(int unit, uint32 info)
{
    uint32 addr;
    uint32 membits;

    addr = info & SOC_TUCANA_CRC_ADDR_MASK;
    membits = (info >> SOC_TUCANA_CRC_MEM_BITS_SHIFT) & 
        SOC_TUCANA_CRC_MEM_BITS_MASK;

    soc_cm_debug(DK_ERR,
                 "Tucana MMU DRAM %s CRC error at 0x%08x\n", 
                 _soc_tucana_crc_mems[membits], addr);
    return SOC_E_NONE;
}

int
soc_tucana_pdll_lock_loss(int unit, uint32 info)
{
    if (info & SOC_TUCANA_MEM_FAIL_DLL) {
	soc_cm_debug(DK_ERR,
		     "unit %d: DLL lock loss\n", unit);
    }

    if (info & SOC_TUCANA_MEM_FAIL_PLL) {
	soc_cm_debug(DK_ERR,
		     "unit %d: PLL lock loss\n", unit);
    }

    return SOC_E_NONE;
}

int
soc_tucana_num_cells(int unit, int *num_cells)
{
    uint32     memconfig;
    int        row_bits, cells_per_row, cells_per_memch;

    SOC_IF_ERROR_RETURN
        (READ_MEMCONFIGr(unit, &memconfig));
    
    row_bits = soc_reg_field_get(unit, MEMCONFIGr, memconfig, ROW_BITSf);
    cells_per_row =
        soc_reg_field_get(unit, MEMCONFIGr, memconfig, NUM_OF_CELLSf);

    /* Convert row_bits to number of rows, multiply by cells/row */
    cells_per_memch = (1 << row_bits) * cells_per_row;

    /* Actual cells/mem channel must be a multiple of the ptr block size */
    cells_per_memch = 
        (cells_per_memch / SOC_TUCANA_MMU_PTRS_PER_PTR_BLOCK) *
        SOC_TUCANA_MMU_PTRS_PER_PTR_BLOCK;

    /* Two mem channels per channel group */
    *num_cells = 2 * cells_per_memch;

    return SOC_E_NONE;
}

#else	/* !BCM_TUCANA_SUPPORT */
int
soc_tucana_num_cells(int unit, int *num_cells)
{
    *num_cells = 0;
    return SOC_E_NONE;
}
#endif	/* !BCM_TUCANA_SUPPORT */
