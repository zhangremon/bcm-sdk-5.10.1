/*
 * $Id: draco.c 1.4 Broadcom SDK $
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
 * File:        draco.c
 * Purpose:     Driver level functions for 5690 chip.
 * Requires:
 */

#include <sal/core/boot.h>

#include <soc/draco.h>
#include <soc/drv.h>
#include <soc/error.h>

#ifdef BCM_DRACO_SUPPORT

/*
 * Draco chip driver functions.
 */
soc_functions_t soc_draco_drv_funs = {
    soc_draco_misc_init,
    soc_draco_mmu_init,
    soc_draco_age_timer_get,
    soc_draco_age_timer_max_get,
    soc_draco_age_timer_set,
};

int
soc_draco_misc_init(int unit)
{
    /* Enable transmit and receive in GPICS */
    uint32		regval;
    uint64		reg64;
    int			port;

    /*
     * Reset the ARL block
     */
    SOC_IF_ERROR_RETURN(READ_ARL_CONTROLr(unit, &regval));
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

    soc_reg_field_set(unit, ARL_CONTROLr, &regval, CLK_GRANf, 0x7);
    SOC_IF_ERROR_RETURN(WRITE_ARL_CONTROLr(unit, regval));

    /*
     * Workaround for missed LPM table lookups (GNATS 3159): extends an
     * internal request timeout value in an undocumented location in the
     * ARL block.
     */
    SOC_IF_ERROR_RETURN(soc_reg32_write(unit, 0x00e00010, 0x3f));

    /*
     * Egress Enable
     */
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
    soc_reg64_field32_set(unit, EPC_LINKr, &reg64, PORT_BITMAPf,
			  SOC_PBMP_WORD_GET(PBMP_CMIC(unit), 0));
    PBMP_GE_ITER(unit, port) {
	SOC_IF_ERROR_RETURN(WRITE_EPC_LINKr(unit, port, reg64));
    }

    COMPILER_64_ZERO(reg64);
    soc_reg64_field32_set(unit, ILINKr, &reg64, PORT_BITMAPf,
			  SOC_PBMP_WORD_GET(PBMP_CMIC(unit), 0));
    PBMP_HG_ITER(unit, port) {
        SOC_IF_ERROR_RETURN(WRITE_ILINKr(unit, port, reg64));
    }

    /* GMAC init should be moved to mac */
    SOC_IF_ERROR_RETURN(READ_GMACC1r(unit, SOC_PORT(unit, ge, 0), &reg64));
    soc_reg64_field32_set(unit, GMACC1r, &reg64, TXEN0f, 1);
    soc_reg64_field32_set(unit, GMACC1r, &reg64, RXEN0f, 1);
    PBMP_GE_ITER(unit, port) {
	SOC_IF_ERROR_RETURN(WRITE_GMACC1r(unit, port, reg64));
    }

    /*
     * Chip CONFIG register
     */
    SOC_IF_ERROR_RETURN(soc_reg_read_any_block(unit, CONFIGr, &regval));
    soc_reg_field_set(unit, CONFIGr, &regval, L3SH_ENf, 1);
    soc_reg_field_set(unit, CONFIGr, &regval, L2DH_ENf, 0);
    soc_reg_field_set(unit, CONFIGr, &regval, PORT_TYPEf, 0);
    SOC_IF_ERROR_RETURN(soc_reg_write_all_blocks(unit, CONFIGr, regval));

    /*
     * Set up the PRTABLE
     */
    if (SOC_IS_DRACO1(unit)) {
	COMPILER_64_ZERO(reg64);
	soc_reg64_field32_set(unit, PRTABLE_DEFAULTr, &reg64, VLAN_IDf,
			      VLAN_ID_DEFAULT);
	PBMP_E_ITER(unit, port) {
	    SOC_IF_ERROR_RETURN(WRITE_PRTABLE_DEFAULTr(unit, port, reg64));
	}
    }
#if defined(BCM_5695)
    if (SOC_IS_DRACO15(unit)) {
	regval = 0;
	soc_reg_field_set(unit, VLAN_DEFAULTr, &regval, VLAN_IDf,
			  VLAN_ID_DEFAULT);
	PBMP_E_ITER(unit, port) {
	    SOC_IF_ERROR_RETURN(WRITE_VLAN_DEFAULTr(unit, port, regval));
	}
    }
#endif

    return SOC_E_NONE;
}

#define DRACO_NUM_COS 8

int
soc_draco_mmu_init(int unit)
{
    uint32      end_addr, regval, val30, val74;
    int         i, wgt[DRACO_NUM_COS];

    /* 
     * Spread the XQs across all COSQs as dictated by the weight
     * properties. This will allow use of all COSQs. However, if 
     * some COSQs are later disabled, the XQs allocated here to 
     * those disabled COSQs will be unavailable for use.
     */
    for (i = 0; i < DRACO_NUM_COS; i++) {
        wgt[i] =
            soc_property_suffix_num_get(unit, i, spn_MMU_XQ_WEIGHT,
                         "cos",  1) + ((i > 0) ? wgt[i-1] : 0);
    }

    /*
     * Active cosqs (more or less) share the 16 segments of the
     * XQ memory associated with each port, based on the relative
     * weights specified by the mmu_xq_weight_cos? properties.
     */

    val30 = val74 = 0;

#define	SETENDADDR(_n)	(end_addr = (_n * 16 / wgt[DRACO_NUM_COS - 1]))

    SETENDADDR(wgt[0]);
    soc_reg_field_set(unit, XQCOSRANGE3_0r,
		      &val30, COS0_ENDADDRESSf, end_addr);
    SETENDADDR(wgt[1]);
    soc_reg_field_set(unit, XQCOSRANGE3_0r,
		      &val30, COS1_ENDADDRESSf, end_addr);
    SETENDADDR(wgt[2]);
    soc_reg_field_set(unit, XQCOSRANGE3_0r,
		      &val30, COS2_ENDADDRESSf, end_addr);
    SETENDADDR(wgt[3]);
    soc_reg_field_set(unit, XQCOSRANGE3_0r,
		      &val30, COS3_ENDADDRESSf, end_addr);
    SETENDADDR(wgt[4]);
    soc_reg_field_set(unit, XQCOSRANGE7_4r,
		      &val74, COS4_ENDADDRESSf, end_addr);
    SETENDADDR(wgt[5]);
    soc_reg_field_set(unit, XQCOSRANGE7_4r,
		      &val74, COS5_ENDADDRESSf, end_addr);
    SETENDADDR(wgt[6]);
    soc_reg_field_set(unit, XQCOSRANGE7_4r,
		      &val74, COS6_ENDADDRESSf, end_addr);
    SETENDADDR(wgt[7]);
    soc_reg_field_set(unit, XQCOSRANGE7_4r,
		      &val74, COS7_ENDADDRESSf, end_addr);
#undef	SETENDADDR

    SOC_IF_ERROR_RETURN(WRITE_XQCOSRANGE3_0r(unit, val30));
    SOC_IF_ERROR_RETURN(WRITE_XQCOSRANGE7_4r(unit, val74));

    /*
     * MMU Port Enable.
     */
    regval = 0;
    soc_reg_field_set(unit, MMUPORTENABLEr, &regval, PORT_BITMAPf,
		      SOC_PBMP_WORD_GET(PBMP_ALL(unit), 0));
    SOC_IF_ERROR_RETURN(WRITE_MMUPORTENABLEr(unit, regval));

    return SOC_E_NONE;
}

int
soc_draco_age_timer_get(int unit, int *age_seconds, int *enabled)
{
    uint32 value;

    SOC_IF_ERROR_RETURN(READ_ARL_AGE_TIMERr(unit, &value));
    *enabled = soc_reg_field_get(unit, ARL_AGE_TIMERr, value, AGE_ENAf);
    *age_seconds = soc_reg_field_get(unit, ARL_AGE_TIMERr, value, AGE_VALf);

    return SOC_E_NONE;
}

int
soc_draco_age_timer_max_get(int unit, int *max_seconds)
{
    *max_seconds =
        soc_reg_field_get(unit, ARL_AGE_TIMERr, 0xffffffff, AGE_VALf);

    return SOC_E_NONE;
}

int
soc_draco_age_timer_set(int unit, int age_seconds, int enable)
{
    uint32 value;

    value = 0;
    soc_reg_field_set(unit, ARL_AGE_TIMERr, &value, AGE_ENAf, enable);
    soc_reg_field_set(unit, ARL_AGE_TIMERr, &value, AGE_VALf, age_seconds);
    SOC_IF_ERROR_RETURN(WRITE_ARL_AGE_TIMERr(unit, value));

    return SOC_E_NONE;
}

#endif	/* BCM_DRACO_SUPPORT */
