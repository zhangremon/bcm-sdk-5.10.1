/*
 * $Id: lynx.c 1.4 Broadcom SDK $
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
 * File:        lynx.c
 * Purpose:     Driver level functions for 5673 chip.
 * Requires:
 */

#include <sal/core/boot.h>

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/error.h>
#include <soc/debug.h>
#include <soc/lynx.h>

#ifdef BCM_LYNX_SUPPORT

/*
 * Lynx chip driver functions.
 */
soc_functions_t soc_lynx_drv_funs = {
    soc_lynx_misc_init,
    soc_lynx_mmu_init,
    soc_lynx_age_timer_get,
    soc_lynx_age_timer_max_get,
    soc_lynx_age_timer_set,
};

int
soc_lynx_misc_init(int unit)
{
    /* Enable transmit and receive in GPICS */
    uint32		regval, regval2;
    uint64              reg64;
    int			port;

    /* For default of 127 MHz core, 25 MHz reference */
    regval = 0;
    soc_reg_field_set(unit, CMIC_RATE_ADJUSTr, &regval, DIVIDENDf, 25);
    soc_reg_field_set(unit, CMIC_RATE_ADJUSTr, &regval, DIVISORf, 127);
    SOC_IF_ERROR_RETURN(WRITE_CMIC_RATE_ADJUSTr(unit, regval));

    /*
     * Reset the ARL block
     */
    SOC_IF_ERROR_RETURN(READ_ARL_CONTROLr(unit, &regval));
    soc_reg_field_set(unit, ARL_CONTROLr, &regval, ARL_INITf, 1);
    SOC_IF_ERROR_RETURN(WRITE_ARL_CONTROLr(unit, regval));

    /* Init XPIC egress tables also */
    regval2 = 0;
    soc_reg_field_set(unit, PT_DEBUGr, &regval2, DEBUG_EGRESS_TABLES_INITf, 1);
    SOC_IF_ERROR_RETURN(soc_reg_write_all_blocks(unit, PT_DEBUGr, regval2));

    /* For simulation, sleep 1 sec.  Otherwise, sleep 5 ms */
    if (SAL_BOOT_SIMULATION) {
        if (SAL_BOOT_RTLSIM) { /* Or 2 minutes on RTL.  Urk. */
            sal_usleep(120000000);
        } else { /* SAL_BOOT_QUICKTURN || SAL_BOOT_PLISIM */
            sal_usleep(5000000);
        }
    } else {
        sal_usleep(5000);
    }

    SOC_IF_ERROR_RETURN(soc_reg_write_all_blocks(unit, PT_DEBUGr, 0));
    soc_reg_field_set(unit, ARL_CONTROLr, &regval, ARL_INITf, 0);
    SOC_IF_ERROR_RETURN(WRITE_ARL_CONTROLr(unit, regval));

    soc_reg_field_set(unit, ARL_CONTROLr, &regval, L3SH_ENf, 1);
    soc_reg_field_set(unit, ARL_CONTROLr, &regval, L2DH_ENf, 0);
    SOC_IF_ERROR_RETURN(WRITE_ARL_CONTROLr(unit, regval));

    regval = 0;
    soc_reg_field_set(unit, XEGR_ENABLEr, &regval, PRT_ENABLEf, 1);
    PBMP_XE_ITER(unit, port) {
	SOC_IF_ERROR_RETURN(WRITE_XEGR_ENABLEr(unit, port, regval));
    }

    COMPILER_64_ZERO(reg64);
    soc_reg64_field32_set(unit, IEGR_ENABLEr, &reg64, PRT_ENABLEf, 1);
    PBMP_HG_ITER(unit, port) {
	SOC_IF_ERROR_RETURN(WRITE_IEGR_ENABLEr(unit, port, reg64));
    }

    COMPILER_64_ZERO(reg64);
    soc_reg64_field32_set(unit, EPC_LINKr, &reg64, PORT_BITMAPf,
		      SOC_PBMP_WORD_GET(PBMP_CMIC(unit), 0));
    PBMP_XE_ITER(unit, port) {
	SOC_IF_ERROR_RETURN(WRITE_EPC_LINKr(unit, port, reg64));
    }

    COMPILER_64_ZERO(reg64);
    soc_reg64_field32_set(unit, ILINKr, &reg64, PORT_BITMAPf,
			  SOC_PBMP_WORD_GET(PBMP_CMIC(unit), 0));
    PBMP_HG_ITER(unit, port) {
	SOC_IF_ERROR_RETURN(WRITE_ILINKr(unit, port, reg64));
    }

    /*
     * Chip CONFIG register
     */
    SOC_IF_ERROR_RETURN(soc_reg_read_any_block(unit, CONFIGr, &regval));
    soc_reg_field_set(unit, CONFIGr, &regval, PORT_TYPEf, 0);
    SOC_IF_ERROR_RETURN(soc_reg_write_all_blocks(unit, CONFIGr, regval));

    /*
     * Set up the PRTABLE
     */
    COMPILER_64_ZERO(reg64);
    soc_reg64_field32_set(unit, PRTABLE_DEFAULTr, &reg64, VLAN_IDf,
			  VLAN_ID_DEFAULT);
    PBMP_E_ITER(unit, port) {
	SOC_IF_ERROR_RETURN(WRITE_PRTABLE_DEFAULTr(unit, port, reg64));
    }

    /*
     * MMU initialization.
     */
    regval = 0;
    soc_reg_field_set(unit, MMUPORTENABLEr, &regval, PORT_BITMAPf,
		      SOC_PBMP_WORD_GET(PBMP_ALL(unit), 0));
    SOC_IF_ERROR_RETURN(WRITE_MMUPORTENABLEr(unit, regval));

    /* Don't take parity errors from unused tables */
    PBMP_XE_ITER(unit, port) {
	SOC_IF_ERROR_RETURN(WRITE_XPC_PARERRr(unit, port,
					      SOC_LYNX_PARITY_XPIC_CLEAR));
    }

    return SOC_E_NONE;
}

int
soc_lynx_mmu_init(int unit)
{
    return SOC_E_NONE;
}

int
soc_lynx_age_timer_get(int unit, int *age_seconds, int *enabled)
{
    uint32 value;

    SOC_IF_ERROR_RETURN(READ_ARL_AGE_TIMERr(unit, &value));
    *enabled = soc_reg_field_get(unit, ARL_AGE_TIMERr, value, AGE_ENAf);
    *age_seconds = soc_reg_field_get(unit, ARL_AGE_TIMERr, value, AGE_VALf);

    return SOC_E_NONE;
}

int
soc_lynx_age_timer_max_get(int unit, int *max_seconds)
{
    *max_seconds =
        soc_reg_field_get(unit, ARL_AGE_TIMERr, 0xffffffff, AGE_VALf);

    return SOC_E_NONE;
}

int
soc_lynx_age_timer_set(int unit, int age_seconds, int enable)
{
    uint32 value;

    value = 0;
    soc_reg_field_set(unit, ARL_AGE_TIMERr, &value, AGE_ENAf, enable);
    soc_reg_field_set(unit, ARL_AGE_TIMERr, &value, AGE_VALf, age_seconds);
    SOC_IF_ERROR_RETURN(WRITE_ARL_AGE_TIMERr(unit, value));

    return SOC_E_NONE;
}

STATIC void
_soc_lynx_parity(int unit,
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

int
soc_lynx_xpic_parity_error(int unit, int blk)
{
    uint32		xpc_parerr, paraddr;
    int			port;

    port = SOC_BLOCK_PORT(unit, blk);
    SOC_IF_ERROR_RETURN(READ_XPC_PARERRr(unit, port, &xpc_parerr));

    if (xpc_parerr & SOC_LYNX_PARITY_XPIC_EGR_IPMCDATA) {
        char *memname = "IPMC_Data_buffer";
        SOC_IF_ERROR_RETURN(READ_XPC_PARERR_ADDR0r(unit, port, &paraddr));
        _soc_lynx_parity(unit, blk, paraddr, memname, -1);
    }

    if (xpc_parerr & SOC_LYNX_PARITY_XPIC_EGR_SPVLAN) {
        /* Unused */
    }

    if (xpc_parerr & SOC_LYNX_PARITY_XPIC_EGR_IPMCLS) {
        SOC_IF_ERROR_RETURN(READ_XPC_PARERR_ADDR2r(unit, port, &paraddr));
        _soc_lynx_parity(unit, blk, paraddr,
				SOC_MEM_NAME(unit, EGRESS_IPMC_LSm), -1);
    }

    if (xpc_parerr & SOC_LYNX_PARITY_XPIC_EGR_IPMCMS) {
        SOC_IF_ERROR_RETURN(READ_XPC_PARERR_ADDR3r(unit, port, &paraddr));
        _soc_lynx_parity(unit, blk, paraddr,
				SOC_MEM_NAME(unit, EGRESS_IPMC_MSm), -1);
    }

    if (xpc_parerr & SOC_LYNX_PARITY_XPIC_ING_CELLBUF) {
        SOC_IF_ERROR_RETURN(READ_XPC_PARERR_ADDR4r(unit, port, &paraddr));
        _soc_lynx_parity(unit, blk, paraddr,
				SOC_MEM_NAME(unit, CELL_BUFm), -1);
    }

    if (xpc_parerr & SOC_LYNX_PARITY_XPIC_VPLS_TABLE) {
        /* Unused */
    }

    if (xpc_parerr & SOC_LYNX_PARITY_XPIC_FFP_METER1) {
        SOC_IF_ERROR_RETURN(READ_XPC_PARERR_ADDR6r(unit, port, &paraddr));
        _soc_lynx_parity(unit, blk, paraddr,
				SOC_MEM_NAME(unit, XFILTER_METERINGm), 1);
    }

    if (xpc_parerr & SOC_LYNX_PARITY_XPIC_FFP_METER0) {
        SOC_IF_ERROR_RETURN(READ_XPC_PARERR_ADDR7r(unit, port, &paraddr));
        _soc_lynx_parity(unit, blk, paraddr,
				SOC_MEM_NAME(unit, XFILTER_METERINGm), 0);
    }

    if (xpc_parerr & SOC_LYNX_PARITY_XPIC_FFP_IRULE1) {
        SOC_IF_ERROR_RETURN(READ_XPC_PARERR_ADDR8r(unit, port, &paraddr));
        _soc_lynx_parity(unit, blk, paraddr,
				SOC_MEM_NAME(unit, GFILTER_IRULEm), 1);
    }

    if (xpc_parerr & SOC_LYNX_PARITY_XPIC_FFP_IRULE0) {
        SOC_IF_ERROR_RETURN(READ_XPC_PARERR_ADDR9r(unit, port, &paraddr));
        _soc_lynx_parity(unit, blk, paraddr,
				SOC_MEM_NAME(unit, GFILTER_IRULEm), 0);
    }

    if (soc_feature(unit, soc_feature_mstp_mask) &&
       (xpc_parerr & SOC_LYNX_PARITY_XPIC_VLANSTG)) {
        SOC_IF_ERROR_RETURN(READ_XPC_PARERR_ADDR10r(unit, port, &paraddr));
        _soc_lynx_parity(unit, blk, paraddr,
				SOC_MEM_NAME(unit, EGRESS_VLAN_STGm), 0);
    }

    /*
     * Clear errors.  Yes, this is a race condition.
     * Designers judged that this is OK.
     */
    SOC_IF_ERROR_RETURN(WRITE_XPC_PARERRr(unit, port,
                                          SOC_LYNX_PARITY_XPIC_CLEAR));
    return SOC_E_NONE;
}

int
soc_lynx_arl_parity_error(int unit, int blk)
{
    uint32	arl_parerr, paraddr;
    int		bit;

    SOC_IF_ERROR_RETURN(READ_ARL_PARERRr(unit, &arl_parerr));

    if (arl_parerr & SOC_LYNX_PARITY_ARL_L2RAM_MASK) {
        SOC_IF_ERROR_RETURN(READ_ARL_PARADDR_L2r(unit, &paraddr));
        for (bit = 0; bit < 8; bit++) {
            if (arl_parerr & (1 << bit)) {
                _soc_lynx_parity(unit, blk, paraddr,
				 SOC_MEM_NAME(unit, L2X_BASEm), bit);
            }
        }
    }

    if (arl_parerr & SOC_LYNX_PARITY_ARL_L3RAM_MASK) {
        SOC_IF_ERROR_RETURN(READ_ARL_PARADDR_L3r(unit, &paraddr));
        for (bit = 0; bit < 8; bit++) {
            if (arl_parerr & (1 << (bit + SOC_LYNX_PARITY_ARL_L3RAM_SHIFT))) {
                _soc_lynx_parity(unit, blk, paraddr,
				 SOC_MEM_NAME(unit, L3X_BASEm), bit);
            }
        }
    }

    if (arl_parerr & SOC_LYNX_PARITY_ARL_L2_VALID) {
        SOC_IF_ERROR_RETURN(READ_ARL_PARADDR_L2_VALIDr(unit, &paraddr));
        _soc_lynx_parity(unit, blk, paraddr,
			 SOC_MEM_NAME(unit, L2X_VALIDm), -1);
    }

    if (arl_parerr & SOC_LYNX_PARITY_ARL_L2_STATIC) {
        SOC_IF_ERROR_RETURN(READ_ARL_PARADDR_L2_STATICr(unit, &paraddr));
        _soc_lynx_parity(unit, blk, paraddr,
			 SOC_MEM_NAME(unit, L2X_STATICm), -1);
    }

    if (arl_parerr & SOC_LYNX_PARITY_ARL_L3_VALID) {
        SOC_IF_ERROR_RETURN(READ_ARL_PARADDR_L3_VALIDr(unit, &paraddr));
        _soc_lynx_parity(unit, blk, paraddr,
			 SOC_MEM_NAME(unit, L3X_VALIDm), -1);
    }

    if (arl_parerr & SOC_LYNX_PARITY_ARL_QVLAN) {
        SOC_IF_ERROR_RETURN(READ_ARL_PARADDR_QVLANr(unit, &paraddr));
        _soc_lynx_parity(unit, blk, paraddr,
			 SOC_MEM_NAME(unit, VLAN_TABm), -1);
    }

    if (arl_parerr & SOC_LYNX_PARITY_ARL_STG) {
        SOC_IF_ERROR_RETURN(READ_ARL_PARADDR_STGr(unit, &paraddr));
        _soc_lynx_parity(unit, blk, paraddr,
			 SOC_MEM_NAME(unit, STG_TABm), -1);
    }

    /* According to HW, SPF == EGR_MASK */
    if (arl_parerr & SOC_LYNX_PARITY_ARL_SPF) {
        SOC_IF_ERROR_RETURN(READ_ARL_PARADDR_SPFr(unit, &paraddr));
        _soc_lynx_parity(unit, blk, paraddr,
			 SOC_MEM_NAME(unit, EGR_MASKm), -1);
    }

    if (arl_parerr & SOC_LYNX_PARITY_ARL_L2MC) {
        SOC_IF_ERROR_RETURN(READ_ARL_PARADDR_L2MCr(unit, &paraddr));
        _soc_lynx_parity(unit, blk, paraddr,
			 SOC_MEM_NAME(unit, L2X_MCm), -1);
    }

    if (arl_parerr & SOC_LYNX_PARITY_ARL_DEFIP) {
        SOC_IF_ERROR_RETURN(READ_ARL_PARADDR_DEFIPr(unit, &paraddr));
        _soc_lynx_parity(unit, blk, paraddr,
			 SOC_MEM_NAME(unit, DEFIPm), -1);
    }

    if (arl_parerr & SOC_LYNX_PARITY_ARL_L3IF) {
        SOC_IF_ERROR_RETURN(READ_ARL_PARADDR_L3IFr(unit, &paraddr));
        _soc_lynx_parity(unit, blk, paraddr,
			 SOC_MEM_NAME(unit, L3_INTFm), -1);
    }

    if (arl_parerr & SOC_LYNX_PARITY_ARL_IPMC) {
        SOC_IF_ERROR_RETURN(READ_ARL_PARADDR_IPMCr(unit, &paraddr));
        _soc_lynx_parity(unit, blk, paraddr,
			 SOC_MEM_NAME(unit, L3_IPMCm), -1);
    }

    /*
     * Clear errors.  Yes, this is a race condition.
     * Designers judged that this is OK.
     */
    SOC_IF_ERROR_RETURN(WRITE_ARL_PARERRr(unit, 0));
    return SOC_E_NONE;
}

int
soc_lynx_mmu_parity_error(int unit, int blk)
{
    uint32	mmu_parerr, paraddr;

    SOC_IF_ERROR_RETURN(READ_MMU_PARITYERRORr(unit, &mmu_parerr));

    if (mmu_parerr & SOC_LYNX_PARITY_MMU_XQ0) {
        SOC_IF_ERROR_RETURN(READ_MMU_PARITYERROR_XQ0r(unit, &paraddr));
        _soc_lynx_parity(unit, blk, paraddr,
			 SOC_MEM_NAME(unit, MMU_MEMORIES2_XQ0m), -1);
    }

    if (mmu_parerr & SOC_LYNX_PARITY_MMU_XQ1) {
        SOC_IF_ERROR_RETURN(READ_MMU_PARITYERROR_XQ1r(unit, &paraddr));
        _soc_lynx_parity(unit, blk, paraddr,
			 SOC_MEM_NAME(unit, MMU_MEMORIES2_XQ1m), -1);
    }

    if (mmu_parerr & SOC_LYNX_PARITY_MMU_XQ2) {
        SOC_IF_ERROR_RETURN(READ_MMU_PARITYERROR_XQ2r(unit, &paraddr));
        _soc_lynx_parity(unit, blk, paraddr,
			 SOC_MEM_NAME(unit, MMU_MEMORIES2_XQ2m), -1);
    }

    if (mmu_parerr & SOC_LYNX_PARITY_MMU_CFAP) {
        SOC_IF_ERROR_RETURN(READ_MMU_PARITYERROR_CFAPr(unit, &paraddr));
        _soc_lynx_parity(unit, blk, paraddr,
			 SOC_MEM_NAME(unit, MMU_MEMORIES1_CFAPm), -1);
    }

    if (mmu_parerr & SOC_LYNX_PARITY_MMU_CCP) {
        SOC_IF_ERROR_RETURN(READ_MMU_PARITYERROR_CCPr(unit, &paraddr));
        _soc_lynx_parity(unit, blk, paraddr,
			 SOC_MEM_NAME(unit, MMU_MEMORIES1_CCPm), -1);
    }

    /*
     * Clear errors.  Yes, this is a race condition.
     * Designers judged that this is OK.
     */
    SOC_IF_ERROR_RETURN(WRITE_MMU_PARITYERRORr(unit, 0));

    return SOC_E_NONE;
}

#endif	/* BCM_LYNX_SUPPORT */
