/*
 * $Id: easyrider.c 1.37 Broadcom SDK $
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
 * File:        easyrider.c
 * Purpose:
 * Requires:    
 */

#include <sal/core/boot.h>
#include <soc/defs.h>
#include <soc/error.h>
#ifdef BCM_EASYRIDER_SUPPORT
#include <soc/easyrider.h>
#include <soc/er_cmdmem.h>
#include <soc/er_tcam.h>
#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/mem.h>
#include <soc/tcam/tcamtype2.h>


/*
 * easyrider chip driver functions.  
 */
soc_functions_t soc_easyrider_drv_funs = {
    soc_easyrider_misc_init,
    soc_easyrider_mmu_init,
    soc_easyrider_age_timer_get,
    soc_easyrider_age_timer_max_get,
    soc_easyrider_age_timer_set,
};


int
soc_easyrider_misc_init(int unit)
{
    uint32		rval;                   
    sop_memcfg_er_t     *mcer;
    int                 seer_init_usec;

    mcer = &(SOC_PERSIST(unit)->er_memcfg);

    /* For simulation, set timeout = 60 sec, otherwise, timeout = 50 ms */
    seer_init_usec =
        soc_property_get(unit, spn_SEER_INIT_TIMEOUT_USEC,
                         SAL_BOOT_QUICKTURN ? 60000000 : 50000);

    /* Memory clear/inits should be fired up here */
    soc_intr_disable(unit, IRQ_MEM_FAIL);

    /* For configs that use FP_METER_TABLE_EXT, set longer s-bus timeout */
    switch (mcer->ext_table_cfg) {
        case ER_SEER_CFG_L4_192_EXT:
        case ER_SEER_CFG_L4_96_EXT:
        case ER_SEER_CFG_LPM_256_L4_128_EXT:
        case ER_SEER_CFG_LPM_384_L4_64_EXT:
        case ER_SEER_CFG_LPM_128_L4_64_EXT:
        case ER_SEER_CFG_LPM_192_L4_32_EXT:
            READ_CMIC_SBUS_TIMEOUTr(unit, &rval);
            soc_reg_field_set(unit, CMIC_SBUS_TIMEOUTr, &rval, 
                              TIMEOUT_VALf, 0x3000);
            WRITE_CMIC_SBUS_TIMEOUTr(unit, rval);
            break;
        default:
            break;
    }

    if (mcer->ext_table_cfg == ER_SEER_CFG_L2_512_EXT) {
        SOC_CONTROL(unit)->l2x_external = 1;
    }

    SOC_CONTROL(unit)->l2x_l2mc_in_l2entry = 
        soc_property_get(unit, spn_L2MC_IN_L2ENTRY, 0);

    SOC_IF_ERROR_RETURN(READ_MVL_ISr(unit, &rval));
    soc_reg_field_set(unit, MVL_ISr, &rval, MEM_CONFIGf,
                      mcer->mvl_hash_table_cfg);
    SOC_IF_ERROR_RETURN(WRITE_MVL_ISr(unit, rval));

    /* BCAM tuning */
    SOC_IF_ERROR_RETURN(READ_TUNNEL_SAMr(unit, &rval));
    soc_reg_field_set(unit, TUNNEL_SAMr, &rval, TUNNEL_SAM_BITSf,
                      soc_property_get(unit, spn_SEER_TUNNEL_SAM,
                      soc_feature(unit, soc_feature_seer_bcam_tune) ? 2 : 0));
    SOC_IF_ERROR_RETURN(WRITE_TUNNEL_SAMr(unit, rval));

    /* MMU parity */
    if (!SOC_IS_RELOADING(unit)) {
        SOC_IF_ERROR_RETURN(READ_MCU_MAIN_STATUSr(unit, &rval));
    } else {
        rval = 0;
    }

    if (soc_reg_field_get(unit, MCU_MAIN_STATUSr,
                          rval, CHN_INIT_DONEf) == 1) {
        soc_cm_debug(DK_WARN,
                     "WARNING:  MMU init incomplete, memfail interrupts disabled\n");
    }

    /*
     * Set reference clock (based on 156MHz core clock)
     * to be 156MHz * (1/31) ~= 5.03MHz
     * so MDC output frequency is 0.5 * 5.03MHz ~= 2.52MHz
     */
    rval = 0;
    soc_reg_field_set(unit, CMIC_RATE_ADJUSTr, &rval, DIVISORf, 31);
    soc_reg_field_set(unit, CMIC_RATE_ADJUSTr, &rval, DIVIDENDf, 1);
    SOC_IF_ERROR_RETURN(WRITE_CMIC_RATE_ADJUSTr(unit, rval));

    rval = 0x01;	/* 125KHz I2C sampling rate */
    SOC_IF_ERROR_RETURN(WRITE_CMIC_I2C_STATr(unit, rval));

    /*
     * If feature is available, disable egress VLAN membership and
     * spanning tree checks for mirrored packets.
     */
    if (SOC_REG_FIELD_VALID(unit, EGR_CONFIGr, MIRROR_CHECKS_DISABLEf)) {
        SOC_IF_ERROR_RETURN(READ_EGR_CONFIGr(unit, &rval));
        soc_reg_field_set(unit, EGR_CONFIGr, &rval, MIRROR_CHECKS_DISABLEf, 1);
        SOC_IF_ERROR_RETURN(WRITE_EGR_CONFIGr(unit, rval));
    }

    /*
     * If feature is available, by default do POP and SEARCH if the
     * "bottom of stack" is not set after the first MPLS label lookup.
     */
    if (SOC_REG_FIELD_VALID(unit, DEF_VLAN_CONTROLr, MPLS_LSR_PEf)) {
        SOC_IF_ERROR_RETURN(READ_DEF_VLAN_CONTROLr(unit, &rval));
        soc_reg_field_set(unit, DEF_VLAN_CONTROLr, &rval, MPLS_LSR_PEf, 1);
        SOC_IF_ERROR_RETURN(WRITE_DEF_VLAN_CONTROLr(unit, rval));
    }

    SOC_IF_ERROR_RETURN(soc_mem_clear(unit, IGR_VLAN_XLATEm, MEM_BLOCK_ALL, 0));
    SOC_IF_ERROR_RETURN(soc_mem_clear(unit, EGR_VLAN_XLATEm, MEM_BLOCK_ALL, 0));

    return SOC_E_NONE;
}

/*
 * Registers to be polled on SEER reconfig:
 *   STATUS_BSE.MEMINIT_DONE
 *   STATUS_HSE.MEMINIT_DONE
 */

STATIC int
_soc_easyrider_seer_init(int unit)
{
    uint32		rval;                   
    uint32		new_rval;
    uint64		reg64;
    soc_port_t          port;
    sop_memcfg_er_t     *mcer;
    soc_timeout_t       to;
    int                 seer_init_usec;

    mcer = &(SOC_PERSIST(unit)->er_memcfg);

    /* For simulation, set timeout = 60 sec, otherwise, timeout = 50 ms */
    seer_init_usec =
        soc_property_get(unit, spn_SEER_INIT_TIMEOUT_USEC,
                         SAL_BOOT_QUICKTURN ? 60000000 : 50000);

    /* Disable purge until any external L2 memory is initialized */
    SOC_IF_ERROR_RETURN(READ_L2_ISr(unit, &rval));
    soc_reg_field_set(unit, L2_ISr, &rval, PRG_ENf, 0);
    SOC_IF_ERROR_RETURN(WRITE_L2_ISr(unit, rval));

    /* External SEER table init */
    SOC_IF_ERROR_RETURN(READ_SEER_CONFIGr(unit, &rval));
    soc_reg_field_set(unit, SEER_CONFIGr, &rval, L3SRCHIT_ENABLEf, 1);
    soc_reg_field_set(unit, SEER_CONFIGr, &rval, L2DHIT_ENABLEf, 0);
    /* The next three should be protected by a property */
    soc_reg_field_set(unit, SEER_CONFIGr, &rval, EXT_TABLE_CONFIGf,
                      mcer->ext_table_cfg);
    soc_reg_field_set(unit, SEER_CONFIGr, &rval, HSE_SEL_EM_LATENCY7f,
                      soc_property_get(unit, spn_SEER_HSE_EM_LATENCY7, 0));
    soc_reg_field_set(unit, SEER_CONFIGr, &rval, CSE_SEL_EM_LATENCY7f,
                      soc_property_get(unit, spn_SEER_CSE_EM_LATENCY7, 0));
    soc_reg_field_set(unit, SEER_CONFIGr, &rval, L2MC_BMP_FROM_L2f,
                      (SOC_CONTROL(unit)->l2x_l2mc_in_l2entry) ? 1 : 0);
    SOC_IF_ERROR_RETURN(WRITE_SEER_CONFIGr(unit, rval));
    
    if (!SAL_BOOT_PLISIM) {
        soc_timeout_init(&to, seer_init_usec, 0);
        /* Wait for SEER BSE memory initialization done. */
        do {
            SOC_IF_ERROR_RETURN(READ_STATUS_BSEr(unit, &rval));
            if (soc_reg_field_get(unit, STATUS_BSEr, rval, MEMINIT_DONEf)) {
                break;
            }
            if (soc_timeout_check(&to)) {
                soc_cm_debug(DK_WARN, "unit %d : STATUS_BSE timeout\n", unit);
                break;
            }
        } while (TRUE);

        /* Wait for SEER CSE memory initialization done. */
        do {
            SOC_IF_ERROR_RETURN(READ_STATUS_CSEr(unit, &rval));
            if (soc_reg_field_get(unit, STATUS_CSEr, rval, MEMINIT_DONEf)) {
                break;
            }
            if (soc_timeout_check(&to)) {
                soc_cm_debug(DK_WARN, "unit %d : STATUS_CSE timeout\n", unit);
                break;
            }
        } while (TRUE);

        /* Wait for SEER HSE memory initialization done. */
        do {
            SOC_IF_ERROR_RETURN(READ_STATUS_HSEr(unit, &rval));
            if (soc_reg_field_get(unit, STATUS_CSEr, rval, MEMINIT_DONEf)) {
                break;
            }
            if (soc_timeout_check(&to)) {
                soc_cm_debug(DK_WARN, "unit %d : STATUS_HSE timeout\n", unit);
                break;
            }
        } while (TRUE);
    }

    /* Order? */
    if (mcer->tcam_select) {
        SOC_IF_ERROR_RETURN(soc_easyrider_qdr_init(unit));
    }
    
    /* Fix up when other configs are settled */
    if (mcer->ext_table_cfg != ER_SEER_CFG_NO_EXT) {
        SOC_IF_ERROR_RETURN(soc_easyrider_external_init(unit));

        /* Only for external table configs */
        SOC_IF_ERROR_RETURN(READ_MCU_ISr(unit, &rval));
        soc_reg_field_set(unit, MCU_ISr, &rval, MCU_ENABLEf, 1);
        SOC_IF_ERROR_RETURN(WRITE_MCU_ISr(unit, rval));
    }
    
    SOC_IF_ERROR_RETURN(READ_L2_ISr(unit, &rval));
    soc_reg_field_set(unit, L2_ISr, &rval, MEM_CONFIGf,
                      mcer->host_hash_table_cfg);
    soc_reg_field_set(unit, L2_ISr, &rval, PRG_ENf, 1);
    if (SAL_BOOT_QUICKTURN) {
        /* Disable static insert delay */
        soc_reg_field_set(unit, L2_ISr, &rval, STATIC_LEARN_TIMER_CTLf, 1);
    }
    if ((mcer->host_hash_table_cfg == ER_SEER_HOST_HASH_CFG_V4_ALL) ||
        (mcer->host_hash_table_cfg == ER_SEER_HOST_HASH_CFG_V4_HALF_V6_HALF) ||
        (mcer->host_hash_table_cfg == ER_SEER_HOST_HASH_CFG_V6_ALL)) {
        /*
         * L2_ENTRY_INTERNAL table is shared with L3_ENTRY_V4/V6 tables.
         * When only using the table for L3_ENTRY_V4/V6, disable the
         * internal L2 table.
         */
        soc_reg_field_set(unit, L2_ISr, &rval, INT_DISf, 1);
    }
    SOC_IF_ERROR_RETURN(WRITE_L2_ISr(unit, rval));

    /* Parity enable after memory init */
    /* Only enable for memories active in this configuration */
    rval = 0;
    if (mcer->tcam_select) {
        rval |= SOC_ER_PARITY_BSE_L3_DEFIP_ALG;
    }
    if (mcer->ext_table_cfg != ER_SEER_CFG_LPM_256_EXT) {
        rval |= SOC_ER_PARITY_BSE_L3_DEFIP_ALG_EXT;
    }
    if (!soc_feature(unit, soc_feature_l3defip_parity)) {
        rval |= SOC_ER_PARITY_BSE_L3_DEFIP_ALG;
        rval |= SOC_ER_PARITY_BSE_VRF_VFI_INTF;
    }
    SOC_IF_ERROR_RETURN(WRITE_PAR_ERR_MASK_BSEr(unit, rval));

    /* If index_max < 0, then the table was disabled during memory config
     * in soc_easyrider_mem_config below */
    rval = 0;
    if (SOC_PERSIST(unit)->memState[FP_TCAM_EXTERNALm].index_max < 0) {
        /* No FP in external TCAM, block all parity from external FP tables */
        rval |= SOC_ER_PARITY_CSE_FP_EXTERNAL_TCAM;
        rval |= SOC_ER_PARITY_CSE_FP_POLICY_TABLE_EXT;
        rval |= SOC_ER_PARITY_CSE_FP_METER_TABLE_EXT;
        rval |= SOC_ER_PARITY_CSE_FP_COUNTER_EXT;
    }
    if (SOC_PERSIST(unit)->memState[NEXT_HOP_INTm].index_max < 0) {
        rval |= SOC_ER_PARITY_CSE_NEXT_HOP_INT;
    } else {
        rval |= SOC_ER_PARITY_CSE_NEXT_HOP_EXT;
    }
    if (SOC_PERSIST(unit)->memState[L3_DEFIP_DATAm].index_max < 0) {
        rval |= SOC_ER_PARITY_CSE_L3_DEFIP_DATA;
    }
    SOC_IF_ERROR_RETURN(WRITE_PAR_ERR_MASK_CSEr(unit, rval));

    rval = 0;
    if (mcer->ext_table_cfg != ER_SEER_CFG_L2_512_EXT) {
        rval |= SOC_ER_PARITY_HSE_L2_ENTRY_EXT;
    }
    if (!soc_feature(unit, soc_feature_ipmc_grp_parity)) {
        rval |= SOC_ER_PARITY_HSE_IPMC_GROUP_V4;
    }
    SOC_IF_ERROR_RETURN(WRITE_PAR_ERR_MASK_HSEr(unit,rval));

    /* Clear all status in SEER (Clear on read) */
    SOC_IF_ERROR_RETURN(READ_PAR_ERR_STATUS_BSEr(unit, &rval));
    SOC_IF_ERROR_RETURN(READ_PAR_ERR_STATUS_CSEr(unit, &rval));
    SOC_IF_ERROR_RETURN(READ_PAR_ERR_STATUS_HSEr(unit, &rval));

    /*
     * Egress Enable
     */
    PBMP_ALL_ITER(unit, port) {
        SOC_IF_ERROR_RETURN(READ_EGR_PORTr(unit, port, &rval));
        soc_reg_field_set(unit, EGR_PORTr, &rval, PORT_ENABLEf, 1);
        SOC_IF_ERROR_RETURN(WRITE_EGR_PORTr(unit, port, rval));
    }

    /* CMIC HG path egress enable */
    SOC_IF_ERROR_RETURN(READ_IEGR_PORTr(unit, CMIC_PORT(unit), &rval));
    soc_reg_field_set(unit, IEGR_PORTr, &rval, PORT_ENABLEf, 1);
    SOC_IF_ERROR_RETURN(WRITE_IEGR_PORTr(unit, CMIC_PORT(unit), rval));
    
    PBMP_HG_ITER(unit, port) {
        SOC_IF_ERROR_RETURN(READ_IGR_PORTr(unit, port, &rval));
        soc_reg_field_set(unit, IGR_PORTr, &rval, HIGIGf, 1);
        SOC_IF_ERROR_RETURN(WRITE_IGR_PORTr(unit, port, rval));
    }

    /* CMIC HG path ingress config */
    SOC_IF_ERROR_RETURN(READ_IGR_IPORTr(unit, CMIC_PORT(unit), &rval));
    soc_reg_field_set(unit, IGR_IPORTr, &rval, HIGIGf, 1);
    SOC_IF_ERROR_RETURN(WRITE_IGR_IPORTr(unit, CMIC_PORT(unit), rval));

    rval = 0;
    soc_reg_field_set(unit, EPC_LINK_BMAPr, &rval, PORT_BITMAPf,
                          SOC_PBMP_WORD_GET(PBMP_CMIC(unit), 0));
    SOC_IF_ERROR_RETURN(WRITE_EPC_LINK_BMAPr(unit, rval));

    if (SOC_PBMP_NOT_NULL(PBMP_GE_ALL(unit))) {
        /* GMAC init should be moved to mac */
        rval = 0;
        soc_reg_field_set(unit, GPORT_CONFIGr, &rval, CLR_CNTf, 1);
        soc_reg_field_set(unit, GPORT_CONFIGr, &rval, GPORT_ENf, 1);
        PBMP_GE_ITER(unit, port) {
            SOC_IF_ERROR_RETURN(WRITE_GPORT_CONFIGr(unit, port, rval));
        }
        soc_reg_field_set(unit, GPORT_CONFIGr, &rval, CLR_CNTf, 0);
        PBMP_GE_ITER(unit, port) {
            SOC_IF_ERROR_RETURN(WRITE_GPORT_CONFIGr(unit, port, rval));
        }

        /* Enable GPORT transmit and receive */
        SOC_IF_ERROR_RETURN(READ_GMACC1r(unit, SOC_PORT(unit, ge, 0), &reg64));
        soc_reg64_field32_set(unit, GMACC1r, &reg64, TXEN0f, 1);
        soc_reg64_field32_set(unit, GMACC1r, &reg64, RXEN0f, 1);
        PBMP_GE_ITER(unit, port) {
            SOC_IF_ERROR_RETURN(WRITE_GMACC1r(unit, port, reg64));
        }
    }

    if (SOC_PBMP_NOT_NULL(PBMP_HG_ALL(unit))) {
        /* XMAC init should be moved to mac */
        rval = 0;
        soc_reg_field_set(unit, XPORT_CONFIGr, &rval, XPORT_ENf, 1);
        soc_reg_field_set(unit, XPORT_CONFIGr, &rval, HIGIG_MODEf, 1);
        PBMP_HG_ITER(unit, port) {
            SOC_IF_ERROR_RETURN(WRITE_XPORT_CONFIGr(unit, port, rval));
        }
    }

    if (SOC_PBMP_NOT_NULL(PBMP_XE_ALL(unit))) {
        rval = 0;
        soc_reg_field_set(unit, XPORT_CONFIGr, &rval, XPORT_ENf, 1);
        soc_reg_field_set(unit, XPORT_CONFIGr, &rval, HIGIG_MODEf, 0);
        PBMP_XE_ITER(unit, port) {
            SOC_IF_ERROR_RETURN(WRITE_XPORT_CONFIGr(unit, port, rval));
        }
    }

    /* Backwards compatible mirroring by default */
    SOC_IF_ERROR_RETURN(READ_MISCCONFIG_2r(unit, &rval));
    soc_reg_field_set(unit, MISCCONFIG_2r, &rval, 
                      DRACO_1_5_MIRRORING_MODE_ENf, 1);
    SOC_IF_ERROR_RETURN(WRITE_MISCCONFIG_2r(unit, rval));
    SOC_IF_ERROR_RETURN(READ_SEER_CONFIGr(unit, &rval));
    soc_reg_field_set(unit, SEER_CONFIGr, &rval, DRACO1_5_MIRRORf, 1);
    SOC_IF_ERROR_RETURN(WRITE_SEER_CONFIGr(unit, rval));
    SOC_IF_ERROR_RETURN(READ_IGR_CONFIGr(unit, &rval));
    soc_reg_field_set(unit, IGR_CONFIGr, &rval, DRACO1_5_MIRRORf, 1);
    SOC_IF_ERROR_RETURN(WRITE_IGR_CONFIGr(unit, rval));
    SOC_IF_ERROR_RETURN(READ_EGR_CONFIGr(unit, &rval));
    soc_reg_field_set(unit, EGR_CONFIGr, &rval, DRACO1_5_MIRRORf, 1);
    soc_reg_field_set(unit, EGR_CONFIGr, &rval, RING_MODEf, 1);
    SOC_IF_ERROR_RETURN(WRITE_EGR_CONFIGr(unit, rval));

    SOC_IF_ERROR_RETURN(READ_EGR_CONFIG2r(unit, &rval));
    if (soc_feature(unit, soc_feature_egr_ts_ctrl)) {
        soc_reg_field_set(unit, EGR_CONFIG2r, &rval, CMIC_TDM_CONTROLf, 3);
    }
    SOC_IF_ERROR_RETURN(WRITE_EGR_CONFIG2r(unit, rval));

    /* Match initial HASH_CONTROL register setting to FB */
    SOC_IF_ERROR_RETURN (READ_HASH_CONTROLr(unit, &rval)); 
    new_rval = rval;                    
    soc_reg_field_set(unit, HASH_CONTROLr, &new_rval, ECMP_HASH_SELf, 2);
    soc_reg_field_set(unit, HASH_CONTROLr, &new_rval,
                      ECMP_HASH_NO_TCP_UDP_PORTSf, 1);
    if (rval != new_rval) { 
        SOC_IF_ERROR_RETURN (WRITE_HASH_CONTROLr(unit, new_rval));
    }

    /* Init tcam if present */
    if (mcer->tcam_select != ER_EXT_TCAM_NONE) {
        SOC_IF_ERROR_RETURN(soc_tcam_init(unit));
    }
    return SOC_E_NONE;
}

STATIC int
_soc_er_mcu_dll_lock_check(int unit, uint32 chn_bmp, int warn)
{
    uint32		rval;
    int                 slice, reps, reps_max;

    reps_max = soc_property_get(unit, spn_MMU_PLL_LOCK_TESTS, 100);

    for (reps = 0; reps < reps_max; reps++) {
        for (slice = 0; slice < 4; slice++) {
            if (chn_bmp & 0x1) {
                SOC_IF_ERROR_RETURN(READ_MCU_CHN0_DDR_REG3r(unit, &rval));
                soc_reg_field_set(unit, MCU_CHN0_DDR_REG3r, &rval,
                                  DLL_MON_SELf, slice);
                SOC_IF_ERROR_RETURN(WRITE_MCU_CHN0_DDR_REG3r(unit, rval));

                SOC_IF_ERROR_RETURN(READ_MCU_CHN0_DDR_STS2r(unit, &rval));
                if (soc_reg_field_get(unit, MCU_CHN0_DDR_STS2r,
                                      rval, QK_DLL90_LOCKEDf) != 1) {
                    if (warn) {
                        soc_cm_debug(DK_WARN,
                                     "WARNING:  MCU channel 0, slice %d,"
                                     " QK DLL90 lock != 1 after %d check(s)\n", 
                                     slice, reps + 1);
                    }
                    return SOC_E_FAIL;
                }
                if (soc_reg_field_get(unit, MCU_CHN0_DDR_STS2r,
                                      rval, QK_DLLDSKW_LOCKEDf) != 1) {
                    if (warn) {
                        soc_cm_debug(DK_WARN,
                                     "WARNING:  MCU channel 0, slice %d,"
                                     " QK DLLDSKW lock != 1 %d check(s)\n", 
                                     slice, reps + 1);
                    }
                    return SOC_E_FAIL;
                }
            }
      
            if (chn_bmp & 0x2) {
                SOC_IF_ERROR_RETURN(READ_MCU_CHN1_DDR_REG3r(unit, &rval));
                soc_reg_field_set(unit, MCU_CHN1_DDR_REG3r, &rval,
                                  DLL_MON_SELf, slice);
                SOC_IF_ERROR_RETURN(WRITE_MCU_CHN1_DDR_REG3r(unit, rval));

                SOC_IF_ERROR_RETURN(READ_MCU_CHN1_DDR_STS2r(unit, &rval));
                if (soc_reg_field_get(unit, MCU_CHN1_DDR_STS2r,
                                      rval, QK_DLL90_LOCKEDf) != 1) {
                    if (warn) {
                        soc_cm_debug(DK_WARN,
                                     "WARNING:  MCU channel 1, slice %d,"
                                     " QK DLL90 lock != 1 %d check(s)\n", 
                                     slice, reps + 1);
                    }
                    return SOC_E_FAIL;
                }
                if (soc_reg_field_get(unit, MCU_CHN1_DDR_STS2r,
                                      rval, QK_DLLDSKW_LOCKEDf) != 1) {
                    if (warn) {
                        soc_cm_debug(DK_WARN,
                                     "WARNING:  MCU channel 1, slice %d,"
                                     " QK DLLDSKW lock != 1 %d check(s)\n", 
                                     slice, reps + 1);
                    }
                    return SOC_E_FAIL;
                }
            }
        }
    }

    soc_cm_debug(DK_VERBOSE, "MCU locks verified %d times\n", reps_max);

    return SOC_E_NONE;
}

STATIC int
_soc_er_ddr_dll_lock_check(int unit, int reg, int field, int slice,
                           char *if_name, char *field_name)
{
    uint32              addr, rval;
    int                 reps, lock_count=0;

    addr = soc_reg_addr(unit, reg, REG_PORT_ANY, 0);
    for (reps = 0; reps < 10; reps++) {
        sal_usleep(1 * MILLISECOND_USEC);
        SOC_IF_ERROR_RETURN(soc_reg32_read(unit, addr, &rval));
        if (soc_reg_field_get(unit, reg, rval, field)) {
            if (++lock_count >= 2) {
                return SOC_E_NONE;
            }
        }
    }
    if (!lock_count) {
        soc_cm_debug(DK_ERR,
                     "unit %d : %s Slice %d %s failed to lock\n",
                     unit, if_name, slice, field_name);
        return SOC_E_INTERNAL;
    } else {
        soc_cm_debug(DK_WARN,
                     "unit %d : %s Slice %d %s locked only one time\n",
                     unit, if_name, slice, field_name);
    }
    return SOC_E_NONE;
}

STATIC int
_soc_easyrider_check_pvt(int unit, int config_reg, int status1_reg,
                         int status2_reg, int check_pn, int thev_75_val, 
                         char *intf_name)
{
    int i, slew, init_usec, ovrd=1;
    uint32 config_addr, status1_addr, status2_addr, rval;

    config_addr = soc_reg_addr(unit, config_reg, REG_PORT_ANY, 0);
    status1_addr = soc_reg_addr(unit, status1_reg, REG_PORT_ANY, 0);
    status2_addr = soc_reg_addr(unit, status2_reg, REG_PORT_ANY, 0);

    init_usec = SAL_BOOT_QUICKTURN ? 1000000 : 100000;
    /* Manually check for PVT compensation convergence 3 times */
    for (i = 0 ; i < 3; i++) {
        SOC_IF_ERROR_RETURN(soc_reg32_read(unit, config_addr, &rval));
        soc_reg_field_set(unit, config_reg, &rval, 
                          OVRD_ODTRES_PVTf, 1);
        soc_reg_field_set(unit, config_reg, &rval, 
                          THEVENIN_75_SELf, thev_75_val);
        soc_reg_field_set(unit, config_reg, &rval, 
                          PVT_ODTRES_VALf, 15);
        SOC_IF_ERROR_RETURN(soc_reg32_write(unit, config_addr, rval));

        sal_usleep(init_usec);

        /* this 1-->0 transition automatically restarts the
         * statemachine and runs auto-compensation routine.
         */
        SOC_IF_ERROR_RETURN(soc_reg32_read(unit, config_addr, &rval));
        soc_reg_field_set(unit, config_reg, &rval, OVRD_ODTRES_PVTf, 0);
        SOC_IF_ERROR_RETURN(soc_reg32_write(unit, config_addr, rval));
        
        sal_usleep(init_usec);

        SOC_IF_ERROR_RETURN(soc_reg32_read(unit, status1_addr, &rval));
        if (soc_reg_field_get(unit, status1_reg, rval, ODT_PVT_DONEf)) {
            ovrd = 0;
            break;
        } 
        /* Failed to converge, try again */
    }

    if (ovrd) {
        soc_cm_debug(DK_WARN,
               "unit %d : %s ODT PVT compensation not done, overriding.\n",
               unit, intf_name);
        SOC_IF_ERROR_RETURN(soc_reg32_read(unit, config_addr, &rval));
        soc_reg_field_set(unit, config_reg, &rval, 
                          OVRD_ODTRES_PVTf, 1);
        soc_reg_field_set(unit, config_reg, &rval, 
                          THEVENIN_75_SELf, thev_75_val);
        soc_reg_field_set(unit, config_reg, &rval, 
                          PVT_ODTRES_VALf, 15);
        SOC_IF_ERROR_RETURN(soc_reg32_write(unit, config_addr, rval));
    } 

    SOC_IF_ERROR_RETURN(soc_reg32_read(unit, status2_addr, &rval));
    slew = soc_reg_field_get(unit, status2_reg, rval, PVT_SLEW_VALf);
    if (slew < 5) {
        soc_cm_debug(DK_WARN,
            "unit %d : %s Overriding PVT slew value from %d to 5.\n",
            unit, intf_name, slew);
        SOC_IF_ERROR_RETURN(soc_reg32_read(unit, config_addr, &rval));
        soc_reg_field_set(unit, config_reg, &rval, OVRD_SLEW_PVTf, 1);
        soc_reg_field_set(unit, config_reg, &rval, PVT_SLEW_VALf, 5);
        SOC_IF_ERROR_RETURN(soc_reg32_write(unit, config_addr, rval));
    }

    if (check_pn) {
        SOC_IF_ERROR_RETURN(soc_reg32_read(unit, status1_addr, &rval));
        if (!soc_reg_field_get(unit, status1_reg, rval, NDRIVER_PVT_DONEf)) {
            soc_cm_debug(DK_WARN,
                         "unit %d : %s NDRIVER PVT compensation not done.\n",
                         unit, intf_name);
        }
        if (!soc_reg_field_get(unit, status1_reg, rval, PDRIVER_PVT_DONEf)) {
            soc_cm_debug(DK_WARN,
                         "unit %d : %s PDRIVER PVT compensation not done.\n",
                         unit, intf_name);
        }
    }

    return SOC_E_NONE;
}

STATIC int
_soc_easyrider_mmu_core_init(int unit, int *has_pointers)
{
    uint32		rval, chn_bmp;
    int                 mcu_init_usec;
    int                 bypass_mcu, slowclk, init_ext, init_mcu;
    soc_port_t          port;
    mmu_ingpktcelllimitibp_entry_t ipclibp;
    int                 module, mod64_mode, mod_mult;
    int                 rv, reinit, mmu_reset_num, mmu_reset_max;

    mcu_init_usec = SAL_BOOT_QUICKTURN ? 1000000 : 5000;
    bypass_mcu = soc_property_get(unit, spn_BYPASS_MCU, 0);
    slowclk = soc_property_get(unit, spn_PLL600_SLOWCLK, 0);
    chn_bmp = soc_property_get(unit, spn_MCU_CHANNEL_BITMAP, 0x3);
    mmu_reset_max = soc_property_get(unit, spn_MMU_RESET_TRIES, 10);
    init_ext = !bypass_mcu && !SAL_BOOT_SIMULATION;
    init_mcu = !bypass_mcu && !SAL_BOOT_PLISIM;

    SOC_IF_ERROR_RETURN(READ_COPYCOUNTCTLr(unit, &rval));
    soc_reg_field_set(unit, COPYCOUNTCTLr, &rval, LIMITEDf, 0);
    SOC_IF_ERROR_RETURN(WRITE_COPYCOUNTCTLr(unit, rval));

    /* MCU initialization sequence */
    SOC_IF_ERROR_RETURN(READ_MEMCONFIGr(unit, &rval));
    soc_reg_field_set(unit, MEMCONFIGr, &rval,
                      PTR_BLOCK_START_ADDRf, 0x1f000);
    SOC_IF_ERROR_RETURN(WRITE_MEMCONFIGr(unit, rval));

    if (init_ext) {
        /* check for PVT done status */
        _soc_easyrider_check_pvt(unit, MCU_CHN0_DDR_REG2r,
                                 MCU_CHN0_DDR_STS1r, MCU_CHN0_DDR_STS2r,
                                 1, 1, "MCU_CHN0");

        /* check for PVT done status */
        _soc_easyrider_check_pvt(unit, MCU_CHN1_DDR_REG2r,
                                 MCU_CHN1_DDR_STS1r, MCU_CHN1_DDR_STS2r,
                                 1, 1, "MCU_CHN1");

        if (slowclk) {
            SOC_IF_ERROR_RETURN(READ_MCU_CHN0_DDR_REG3r(unit, &rval));
            soc_reg_field_set(unit, MCU_CHN0_DDR_REG3r, &rval,
                              PLL600_SLOWCLK_ENf, 1);
            SOC_IF_ERROR_RETURN(WRITE_MCU_CHN0_DDR_REG3r(unit, rval));

            SOC_IF_ERROR_RETURN(READ_MCU_CHN0_AREFr(unit, &rval));
            soc_reg_field_set(unit, MCU_CHN0_AREFr, &rval,
                              HIAREFLIMITf, 0x3d0);
            soc_reg_field_set(unit, MCU_CHN0_AREFr, &rval,
                              LOAREFLIMITf, 0x3a0);
            SOC_IF_ERROR_RETURN(WRITE_MCU_CHN0_AREFr(unit, rval));

            SOC_IF_ERROR_RETURN(READ_MCU_CHN1_AREFr(unit, &rval));
            soc_reg_field_set(unit, MCU_CHN1_AREFr, &rval,
                              HIAREFLIMITf, 0x3d0);
            soc_reg_field_set(unit, MCU_CHN1_AREFr, &rval,
                              LOAREFLIMITf, 0x3a0);
            SOC_IF_ERROR_RETURN(WRITE_MCU_CHN1_AREFr(unit, rval));

            sal_usleep(10 * MILLISECOND_USEC);

            soc_cm_debug(DK_WARN, "5660x PLL set for 500MHz mode\n");
        }

        /* DDR Tuning parameters */
        SOC_IF_ERROR_RETURN(READ_MCU_CHN0_TIMING_32r(unit, &rval));
        soc_reg_field_set(unit, MCU_CHN0_TIMING_32r, &rval, TCRDf,
                          soc_property_suffix_num_get(unit, 0, 
                                                  spn_MCU_TCRD, "ch", 7));
        soc_reg_field_set(unit, MCU_CHN0_TIMING_32r, &rval, TCWDf,
                          soc_property_suffix_num_get(unit, 0, 
                                                  spn_MCU_TCWD, "ch", 8));
        soc_reg_field_set(unit, MCU_CHN0_TIMING_32r, &rval, TWLf,
                          soc_property_suffix_num_get(unit, 0, 
                                                  spn_MCU_TWL, "ch", 6));
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN0_TIMING_32r(unit, rval));

        SOC_IF_ERROR_RETURN(READ_MCU_CHN1_TIMING_32r(unit, &rval));
        soc_reg_field_set(unit, MCU_CHN1_TIMING_32r, &rval, TCRDf,
                          soc_property_suffix_num_get(unit, 1, 
                                                  spn_MCU_TCRD, "ch", 7));
        soc_reg_field_set(unit, MCU_CHN1_TIMING_32r, &rval, TCWDf,
                          soc_property_suffix_num_get(unit, 1, 
                                                  spn_MCU_TCWD, "ch", 8));
        soc_reg_field_set(unit, MCU_CHN1_TIMING_32r, &rval, TWLf,
                          soc_property_suffix_num_get(unit, 1, 
                                                  spn_MCU_TWL, "ch", 6));
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN1_TIMING_32r(unit, rval));

        SOC_IF_ERROR_RETURN(READ_MCU_CHN0_DDR_REG1r(unit, &rval));
        soc_reg_field_set(unit, MCU_CHN0_DDR_REG1r, &rval, DLL90_OFFSET_TXf,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_DLL90_OFFSET_TX, "ch", 3));
        soc_reg_field_set(unit, MCU_CHN0_DDR_REG1r, &rval, DLL90_OFFSET3f,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_DLL90_OFFSET3, "ch", 1));
        soc_reg_field_set(unit, MCU_CHN0_DDR_REG1r, &rval, DLL90_OFFSET2f,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_DLL90_OFFSET2, "ch", 1));
        soc_reg_field_set(unit, MCU_CHN0_DDR_REG1r, &rval, DLL90_OFFSET1f,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_DLL90_OFFSET1, "ch", 1));
        soc_reg_field_set(unit, MCU_CHN0_DDR_REG1r, &rval, DLL90_OFFSET0_QKf,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_DLL90_OFFSET0_QK, "ch", 1));
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN0_DDR_REG1r(unit, rval));
       
        SOC_IF_ERROR_RETURN(READ_MCU_CHN1_DDR_REG1r(unit, &rval));
        soc_reg_field_set(unit, MCU_CHN1_DDR_REG1r, &rval, DLL90_OFFSET_TXf,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_DLL90_OFFSET_TX, "ch", 3));
        soc_reg_field_set(unit, MCU_CHN1_DDR_REG1r, &rval, DLL90_OFFSET3f,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_DLL90_OFFSET3, "ch", 1));
        soc_reg_field_set(unit, MCU_CHN1_DDR_REG1r, &rval, DLL90_OFFSET2f,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_DLL90_OFFSET2, "ch", 1));
        soc_reg_field_set(unit, MCU_CHN1_DDR_REG1r, &rval, DLL90_OFFSET1f,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_DLL90_OFFSET1, "ch", 1));
        soc_reg_field_set(unit, MCU_CHN1_DDR_REG1r, &rval, DLL90_OFFSET0_QKf,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_DLL90_OFFSET0_QK, "ch", 1));
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN1_DDR_REG1r(unit, rval));

        SOC_IF_ERROR_RETURN(READ_MCU_CHN0_DDR_REG2r(unit, &rval));
        soc_reg_field_set(unit, MCU_CHN0_DDR_REG2r, &rval, SEL_EARLY2_3f,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_SEL_EARLY2_3, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN0_DDR_REG2r, &rval, SEL_EARLY1_3f,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_SEL_EARLY1_3, "ch", 1));
        soc_reg_field_set(unit, MCU_CHN0_DDR_REG2r, &rval, SEL_EARLY2_2f,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_SEL_EARLY2_2, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN0_DDR_REG2r, &rval, SEL_EARLY1_2f,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_SEL_EARLY1_2, "ch", 1));
        soc_reg_field_set(unit, MCU_CHN0_DDR_REG2r, &rval, SEL_EARLY2_1f,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_SEL_EARLY2_1, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN0_DDR_REG2r, &rval, SEL_EARLY1_1f,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_SEL_EARLY1_1, "ch", 1));
        soc_reg_field_set(unit, MCU_CHN0_DDR_REG2r, &rval, SEL_EARLY2_0f,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_SEL_EARLY2_0, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN0_DDR_REG2r, &rval, SEL_EARLY1_0f,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_SEL_EARLY1_0, "ch", 1));
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN0_DDR_REG2r(unit, rval));

        SOC_IF_ERROR_RETURN(READ_MCU_CHN1_DDR_REG2r(unit, &rval));
        soc_reg_field_set(unit, MCU_CHN1_DDR_REG2r, &rval, SEL_EARLY2_3f,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_SEL_EARLY2_3, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN1_DDR_REG2r, &rval, SEL_EARLY1_3f,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_SEL_EARLY1_3, "ch", 1));
        soc_reg_field_set(unit, MCU_CHN1_DDR_REG2r, &rval, SEL_EARLY2_2f,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_SEL_EARLY2_2, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN1_DDR_REG2r, &rval, SEL_EARLY1_2f,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_SEL_EARLY1_2, "ch", 1));
        soc_reg_field_set(unit, MCU_CHN1_DDR_REG2r, &rval, SEL_EARLY2_1f,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_SEL_EARLY2_1, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN1_DDR_REG2r, &rval, SEL_EARLY1_1f,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_SEL_EARLY1_1, "ch", 1));
        soc_reg_field_set(unit, MCU_CHN1_DDR_REG2r, &rval, SEL_EARLY2_0f,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_SEL_EARLY2_0, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN1_DDR_REG2r, &rval, SEL_EARLY1_0f,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_SEL_EARLY1_0, "ch", 1));
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN1_DDR_REG2r(unit, rval));

        SOC_IF_ERROR_RETURN(READ_MCU_CHN0_DDR_REG3r(unit, &rval));
        soc_reg_field_set(unit, MCU_CHN0_DDR_REG3r, &rval, DLL90_OFFSET_QKBf,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_DLL90_OFFSET_QKB, "ch", 2));
        soc_reg_field_set(unit, MCU_CHN0_DDR_REG3r, &rval, PHASE_SEL3f,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_PHASE_SEL, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN0_DDR_REG3r, &rval, PHASE_SEL2f,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_PHASE_SEL, "ch",  0));
        soc_reg_field_set(unit, MCU_CHN0_DDR_REG3r, &rval, PHASE_SEL1f,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_PHASE_SEL, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN0_DDR_REG3r, &rval, PHASE_SEL0f,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_PHASE_SEL, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN0_DDR_REG3r, &rval, OVRD_SM3_ENf,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_OVRD_SM_EN, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN0_DDR_REG3r, &rval, OVRD_SM2_ENf,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_OVRD_SM_EN, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN0_DDR_REG3r, &rval, OVRD_SM1_ENf,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_OVRD_SM_EN, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN0_DDR_REG3r, &rval, OVRD_SM0_ENf,
                          soc_property_suffix_num_get(unit, 0, 
                                       spn_MCU_OVRD_SM_EN, "ch", 0));
        if (SOC_REG_FIELD_VALID(unit, MCU_CHN0_DDR_REG3r, 
                                A0_DLLBYP_TXDESKf)) {
            soc_reg_field_set(unit, MCU_CHN0_DDR_REG3r, &rval, 
                              A0_DLLBYP_TXDESKf, 1);
        }
        if (SOC_REG_FIELD_VALID(unit, MCU_CHN0_DDR_REG3r, 
                                USE_468_FROM_PLL468f)) {
            soc_reg_field_set(unit, MCU_CHN0_DDR_REG3r, &rval, 
                              USE_468_FROM_PLL468f, 1);
        }
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN0_DDR_REG3r(unit, rval));

        SOC_IF_ERROR_RETURN(READ_MCU_CHN1_DDR_REG3r(unit, &rval));
        soc_reg_field_set(unit, MCU_CHN1_DDR_REG3r, &rval, DLL90_OFFSET_QKBf,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_DLL90_OFFSET_QKB, "ch",  2));
        soc_reg_field_set(unit, MCU_CHN1_DDR_REG3r, &rval, PHASE_SEL3f,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_PHASE_SEL, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN1_DDR_REG3r, &rval, PHASE_SEL2f,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_PHASE_SEL, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN1_DDR_REG3r, &rval, PHASE_SEL1f,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_PHASE_SEL, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN1_DDR_REG3r, &rval, PHASE_SEL0f,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_PHASE_SEL, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN1_DDR_REG3r, &rval, OVRD_SM3_ENf,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_OVRD_SM_EN, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN1_DDR_REG3r, &rval, OVRD_SM2_ENf,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_OVRD_SM_EN, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN1_DDR_REG3r, &rval, OVRD_SM1_ENf,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_OVRD_SM_EN, "ch", 0));
        soc_reg_field_set(unit, MCU_CHN1_DDR_REG3r, &rval, OVRD_SM0_ENf,
                          soc_property_suffix_num_get(unit, 1, 
                                       spn_MCU_OVRD_SM_EN, "ch", 0));
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN1_DDR_REG3r(unit, rval));

        SOC_IF_ERROR_RETURN(READ_MCU_CHN0_CONFIG_32r(unit, &rval));
        soc_reg_field_set(unit, MCU_CHN0_CONFIG_32r, &rval,
                          INITWAITINTVf, 0x78);
        soc_reg_field_set(unit, MCU_CHN0_CONFIG_32r, &rval,
                          INITAREFINTVf, 0x800);
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN0_CONFIG_32r(unit, rval));

        SOC_IF_ERROR_RETURN(READ_MCU_CHN1_CONFIG_32r(unit, &rval));
        soc_reg_field_set(unit, MCU_CHN1_CONFIG_32r, &rval,
                          INITWAITINTVf, 0x78);
        soc_reg_field_set(unit, MCU_CHN1_CONFIG_32r, &rval,
                          INITAREFINTVf, 0x800);
        SOC_IF_ERROR_RETURN(WRITE_MCU_CHN1_CONFIG_32r(unit, rval));

        if (soc_property_get(unit, spn_MCU_ODT_IMP_ENABLE, 1)) {
            SOC_IF_ERROR_RETURN(READ_MCU_CHN0_MODEREG_RLr(unit, &rval));
            soc_reg_field_set(unit, MCU_CHN0_MODEREG_RLr, &rval,
                              ONDIETERMf, 1);
            soc_reg_field_set(unit, MCU_CHN0_MODEREG_RLr, &rval,
                              IMPMATCHf, 1);
            SOC_IF_ERROR_RETURN(WRITE_MCU_CHN0_MODEREG_RLr(unit, rval));

            SOC_IF_ERROR_RETURN(READ_MCU_CHN1_MODEREG_RLr(unit, &rval));
            soc_reg_field_set(unit, MCU_CHN1_MODEREG_RLr, &rval,
                              ONDIETERMf, 1);
            soc_reg_field_set(unit, MCU_CHN1_MODEREG_RLr, &rval,
                              IMPMATCHf, 1);
            SOC_IF_ERROR_RETURN(WRITE_MCU_CHN1_MODEREG_RLr(unit, rval));
        }

        SOC_IF_ERROR_RETURN(READ_XQ_MISCr(unit, &rval));
        soc_reg_field_set(unit, XQ_MISCr, &rval, EN_XQ_TESTf, 0);
        SOC_IF_ERROR_RETURN(WRITE_XQ_MISCr(unit, rval));
    }

    if (init_mcu && (chn_bmp != 0)) {
        rval = 0;
        soc_reg_field_set(unit, MCU_MAIN_CONTROLr, &rval, ENABLE_CHN0f,
                          (chn_bmp & 0x1) ? 1 : 0);
        soc_reg_field_set(unit, MCU_MAIN_CONTROLr, &rval, ENABLE_CHN1f,
                          (chn_bmp & 0x2) ? 1 : 0);
        SOC_IF_ERROR_RETURN(WRITE_MCU_MAIN_CONTROLr(unit, rval));
    
        sal_usleep(mcu_init_usec);

        soc_reg_field_set(unit, MCU_MAIN_CONTROLr, &rval, INIT_CHN0f,
                          (chn_bmp & 0x1) ? 1 : 0);
        soc_reg_field_set(unit, MCU_MAIN_CONTROLr, &rval, INIT_CHN1f,
                          (chn_bmp & 0x2) ? 1 : 0);
        SOC_IF_ERROR_RETURN(WRITE_MCU_MAIN_CONTROLr(unit, rval));

        sal_usleep(mcu_init_usec);

        SOC_IF_ERROR_RETURN(READ_MCU_MAIN_STATUSr(unit, &rval));
        switch (chn_bmp & 0x3) {
        case 0x1:
            if (soc_reg_field_get(unit, MCU_MAIN_STATUSr,
                                  rval, CHN0_INIT_DONEf) != 1) {
                soc_cm_debug(DK_WARN,
                             "WARNING:  56601 MCU channel 0 init done != 1\n");
            }
            break;
        case 0x2:
            if (soc_reg_field_get(unit, MCU_MAIN_STATUSr,
                                  rval, CHN1_INIT_DONEf) != 1) {
                soc_cm_debug(DK_WARN,
                             "WARNING:  56601 MCU channel 1 init done != 1\n");
            }
            break;
        case 0x3:
            if (soc_reg_field_get(unit, MCU_MAIN_STATUSr,
                                  rval, CHN_INIT_DONEf) != 1) {
                soc_cm_debug(DK_WARN,
                             "WARNING:  56601 MCU channel init done != 1\n");
            }
            break;
        default:
            return SOC_E_INTERNAL;
        }
    }

    SOC_IF_ERROR_RETURN(READ_MISCCONFIGr(unit, &rval));
    /* Turn over parity control to HW */
    soc_reg_field_set(unit, MISCCONFIGr, &rval, PARITY_BIT_WRITE_ACCESSf, 0);
    if (bypass_mcu) {
        soc_reg_field_set(unit, MISCCONFIGr, &rval, SUPPRESS_MEMFULLf, 1);
        soc_reg_field_set(unit, MISCCONFIGr, &rval, SHORTCIRCUITENf, 1);
    } else if (chn_bmp != 0x3) {
        soc_reg_field_set(unit, MISCCONFIGr, &rval, SUPPRESS_MEMFULLf, 1);
    }

    mmu_reset_num = 0;

    do {
        reinit = FALSE;
        soc_reg_field_set(unit, MISCCONFIGr, &rval, MMU_SOFT_RESET_Nf, 0);
        SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, rval));

        sal_usleep(mcu_init_usec);

        soc_reg_field_set(unit, MISCCONFIGr, &rval, MMU_SOFT_RESET_Nf, 1);
        SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, rval));

        if (init_ext) {
            mmu_reset_num++;
            rv = _soc_er_mcu_dll_lock_check(unit, chn_bmp, 
                                            (mmu_reset_num == mmu_reset_max));
            if (rv == SOC_E_FAIL) {
                if (mmu_reset_num < mmu_reset_max) {
                    reinit = TRUE;
                } else {
                    return rv;
                }
            }
        }
    } while (reinit);

    PBMP_ALL_ITER(unit, port) {
        SOC_IF_ERROR_RETURN(READ_MTCREQr(unit, port, &rval));
        soc_reg_field_set(unit, MTCREQr, &rval, CELLREQUESTCOUNTf, 0x40);
        soc_reg_field_set(unit, MTCREQr, &rval, PKTREQUESTCOUNTf, 0x6);
        SOC_IF_ERROR_RETURN(WRITE_MTCREQr(unit, port, rval));
    }

    PBMP_HG_ITER(unit, port) {
        SOC_IF_ERROR_RETURN(READ_MTCREQr(unit, port, &rval));
        soc_reg_field_set(unit, MTCREQr, &rval, CELLREQUESTCOUNTf, 0x180);
        soc_reg_field_set(unit, MTCREQr, &rval, PKTREQUESTCOUNTf, 0x10);
        SOC_IF_ERROR_RETURN(WRITE_MTCREQr(unit, port, rval));
    }
    PBMP_XE_ITER(unit, port) {
        SOC_IF_ERROR_RETURN(READ_MTCREQr(unit, port, &rval));
        soc_reg_field_set(unit, MTCREQr, &rval, CELLREQUESTCOUNTf, 0x180);
        soc_reg_field_set(unit, MTCREQr, &rval, PKTREQUESTCOUNTf, 0x10);
        SOC_IF_ERROR_RETURN(WRITE_MTCREQr(unit, port, rval));
    }

    /* Clear MMU parity status */
    rval = soc_feature(unit, soc_feature_mcu_fifo_suppress) ?
              SOC_ER_PARITY_MMU_MCU_REQ_FIFO_ERR : 0; /* Mask out this error */
    SOC_IF_ERROR_RETURN(WRITE_MMU_ERR_VECTORr(unit, rval));

    /* Prevent CMIC backpressure */
    /* E2E has 8 sets of IBP limits.  We reserve one to be used to prevent
     * CMIC backpressure.  This involves programming the limits for this set,
     * configuring the E2E mode, and pointing the (mod,port) entry at the
     * selected limit set.
     */

    rval = 0;
    soc_reg_field_set(unit, INGLIMITr, &rval, IBPLIMITf, 0x3ffff);
    SOC_IF_ERROR_RETURN
        (WRITE_INGLIMITr(unit, SOC_ER_MMU_LIMITS_PTR_CMIC, rval));

    rval = 0;
    soc_reg_field_set(unit, INGLIMITDISCARDr, &rval, DISCARDLIMITf, 0x3ffff);
    SOC_IF_ERROR_RETURN
        (WRITE_INGLIMITDISCARDr(unit, SOC_ER_MMU_LIMITS_PTR_CMIC, rval));

    mod64_mode = soc_property_get(unit, spn_E2E_64_MODULES, 1);
    SOC_IF_ERROR_RETURN(READ_E2E_MODULE_CONFIGr(unit, &rval));
    soc_reg_field_set(unit, E2E_MODULE_CONFIGr, &rval, MOD_64_MODEf,
                      mod64_mode ? 1 : 0);
    SOC_IF_ERROR_RETURN(WRITE_E2E_MODULE_CONFIGr(unit, rval));

    module = 0;
    mod_mult = mod64_mode ? 16 : 32;
    sal_memset(&ipclibp, 0, sizeof(ipclibp));
    soc_mem_field32_set(unit, MMU_INGPKTCELLLIMITIBPm, &ipclibp, PTRf,
                      SOC_ER_MMU_LIMITS_PTR_CMIC);
    SOC_IF_ERROR_RETURN
        (WRITE_MMU_INGPKTCELLLIMITIBPm(unit, MEM_BLOCK_ALL,
                   CMIC_PORT(unit) + (module * mod_mult), &ipclibp));

    sal_usleep(mcu_init_usec);

    *has_pointers = TRUE;
    if (init_ext) {
        if (chn_bmp & 0x1) {
            SOC_IF_ERROR_RETURN(READ_FREE_CELLPTRS_CH0r(unit, &rval));
            if (soc_reg_field_get(unit, FREE_CELLPTRS_CH0r, 
                                  rval, PTRCOUNTf) == 0) {
                *has_pointers = FALSE;
            }
        }
        if (chn_bmp & 0x2) {
            SOC_IF_ERROR_RETURN(READ_FREE_CELLPTRS_CH1r(unit, &rval));
            if (soc_reg_field_get(unit, FREE_CELLPTRS_CH1r, 
                                  rval, PTRCOUNTf) == 0) {
                *has_pointers = FALSE;
            }
        }
    }

    return SOC_E_NONE;
}

int
soc_easyrider_mmu_init(int unit)
{
    int      has_pointers = FALSE;
    int      rv = SOC_E_NONE;
    int      mcu_init_usec;
    int      mmu_reset_num, mmu_reset_max;
    uint32   rval;

    mmu_reset_max = soc_property_get(unit, spn_MMU_RESET_TRIES, 10);
    mcu_init_usec = SAL_BOOT_QUICKTURN ? 1000000 : 5000;

    for (mmu_reset_num = 0; mmu_reset_num < mmu_reset_max; mmu_reset_num++) {
        /* Kick MCU before init */
        SOC_IF_ERROR_RETURN(READ_MCU_MAIN_CONTROLr(unit, &rval));
        soc_reg_field_set(unit, MCU_MAIN_CONTROLr, &rval, RESETf, 1);
        SOC_IF_ERROR_RETURN(WRITE_MCU_MAIN_CONTROLr(unit, rval));
    
        sal_usleep(mcu_init_usec);

        soc_reg_field_set(unit, MCU_MAIN_CONTROLr, &rval, RESETf, 0);
        SOC_IF_ERROR_RETURN(WRITE_MCU_MAIN_CONTROLr(unit, rval));

        rv = _soc_easyrider_mmu_core_init(unit, &has_pointers);
        if ((rv == SOC_E_NONE) && has_pointers) {
            break;
        }
    }

    if (rv != SOC_E_NONE) {
        soc_cm_debug(DK_ERR,
                     "ERROR:  MMU init failed after %d tries\n",
                     mmu_reset_num);
        return SOC_E_FAIL;
    } else if (!has_pointers) {
        soc_cm_debug(DK_ERR,
                     "ERROR:  MMU pointers failed to resolve after %d tries\n",
                     mmu_reset_num);
        return SOC_E_FAIL;
    }

    rv = _soc_easyrider_seer_init(unit);

    if (rv == SOC_E_NONE) {
        rval = 0;
        SOC_IF_ERROR_RETURN(WRITE_MMU_ERR_VECTORr(unit, rval));
        if (soc_property_get(unit, spn_PARITY_ENABLE, 1)) {
            soc_intr_enable(unit, IRQ_MEM_FAIL);
        }
    }
    return rv;
}

/*
 * ER has 2-bits of age timer status in the L2 entries.  Thus, it may
 * take 4x the age timer setting to age out, rather than 2x as in 1-bit
 * age timer implementations in other devices.  Thus, to get the same
 * behavior, we must covert the provided value to the HW setting.
 */
#define SOC_ER_AGE_TIMER_FACTOR         2

int
soc_easyrider_age_timer_get(int unit, int *age_seconds, int *enabled)
{
    uint32 value;

    value = 0;
    *enabled = 0;
    *age_seconds = 0;

    SOC_IF_ERROR_RETURN(READ_L2_ISr(unit, &value));
    *enabled = soc_reg_field_get(unit, L2_ISr, value, AGE_ENf);

    SOC_IF_ERROR_RETURN(READ_L2_AGE_TIMERr(unit, &value));
    *age_seconds =
        soc_reg_field_get(unit, L2_AGE_TIMERr, value, AGE_VALf)
        * SOC_ER_AGE_TIMER_FACTOR;
    switch (soc_reg_field_get(unit, L2_AGE_TIMERr, value, RES_SELf)) {
    case SOC_ER_AGE_TIMER_RESOLUTION_100MS:
        *age_seconds /= 10;
        break;
    case SOC_ER_AGE_TIMER_RESOLUTION_1MIN:
        *age_seconds *= 60;
        break;
    case SOC_ER_AGE_TIMER_RESOLUTION_5MIN:
        *age_seconds *= 300;
        break;
    case SOC_ER_AGE_TIMER_RESOLUTION_1S:
    default:
        /* No change */
        break;
    }

    return SOC_E_NONE;
}

int
soc_easyrider_age_timer_max_get(int unit, int *max_seconds)
{
    *max_seconds =
        soc_reg_field_get(unit, L2_AGE_TIMERr, 0xffffffff, AGE_VALf)
        * SOC_ER_AGE_TIMER_FACTOR;

    

    return SOC_E_NONE;
}

int
soc_easyrider_age_timer_set(int unit, int age_seconds, int enable)
{
    uint32 value;

    SOC_IF_ERROR_RETURN(READ_L2_ISr(unit, &value));
    soc_reg_field_set(unit, L2_ISr, &value, AGE_ENf, enable);
    SOC_IF_ERROR_RETURN(WRITE_L2_ISr(unit, value));

    SOC_IF_ERROR_RETURN(READ_L2_AGE_TIMERr(unit, &value));
    soc_reg_field_set(unit, L2_AGE_TIMERr, &value, RES_SELf,
                      SOC_ER_AGE_TIMER_RESOLUTION_1S);
    soc_reg_field_set(unit, L2_AGE_TIMERr, &value, AGE_VALf,
                      ((age_seconds + (SOC_ER_AGE_TIMER_FACTOR - 1)) /
                       SOC_ER_AGE_TIMER_FACTOR));
    SOC_IF_ERROR_RETURN(WRITE_L2_AGE_TIMERr(unit, value));

    

    return SOC_E_NONE;
}

int
soc_easyrider_qdr_init(int unit)
{
    uint32		rval, offset;
    int                 master, slave, sharing = 0;
    int                 lpm_count, acl_count;

    master = soc_property_get(unit, spn_EXT_TCAM_SHARING_MASTER, 0);
    slave  = soc_property_get(unit, spn_EXT_TCAM_SHARING_SLAVE,  0);
    if (master || slave) {
        if (!soc_feature(unit, soc_feature_ext_tcam_sharing)) {
            return SOC_E_CONFIG;
        }
        sharing = 1;
    }

    /* This stuff begs for properties */
    SOC_IF_ERROR_RETURN(READ_EXT_TCAM_CONFIG_0r(unit, &rval));
    soc_reg_field_set(unit, EXT_TCAM_CONFIG_0r, &rval, CAM_SIZEf,
                      TCAM_SIZE_18MB);
    soc_reg_field_set(unit, EXT_TCAM_CONFIG_0r, &rval, MAX_CONTEXTf,
                      ACL_LPM_MAX_CONTEXT);
    soc_reg_field_set(unit, EXT_TCAM_CONFIG_0r, &rval, MGMT_CONTEXTf,
                      UD_MACRO_CMD_CONTEXT);
    soc_reg_field_set(unit, EXT_TCAM_CONFIG_0r, &rval,
                      TCAM_SRC_LATENCYf, TCAM_REQ_RSP_LATENCY);
    if (sharing) {
        soc_reg_field_set(unit, EXT_TCAM_CONFIG_0r, &rval,
                          TCAM_OFFSET_OVRDf, 1);
    }
    if (SOC_PERSIST(unit)->er_memcfg.tcam_select == ER_EXT_TCAM_TYPE1) {
        soc_reg_field_set(unit, EXT_TCAM_CONFIG_0r, &rval, TCAM_SELf,
                          ER_EXT_TCAM_TYPE1);
    } else {
        soc_reg_field_set(unit, EXT_TCAM_CONFIG_0r, &rval, TCAM_SELf,
                          ER_EXT_TCAM_TYPE2);
        if (sharing) {
            return SOC_E_CONFIG; /* not yet supported */
        }
    }
    SOC_IF_ERROR_RETURN(WRITE_EXT_TCAM_CONFIG_0r(unit, rval));

    if (SOC_PERSIST(unit)->er_memcfg.tcam_select == ER_EXT_TCAM_TYPE2) {
        /* Flip external TCAM interrupt polarity */
        SOC_IF_ERROR_RETURN(READ_CSE_CONFIGr(unit, &rval));
        soc_reg_field_set(unit, CSE_CONFIGr, &rval, EXT_TCAM_INT_POLf, 1);
        SOC_IF_ERROR_RETURN(WRITE_CSE_CONFIGr(unit, rval));
    }

    /* workaround for misplaced NEXT_HOP_EXT table in mode 5 and 6 */
    switch (SOC_PERSIST(unit)->er_memcfg.ext_table_cfg) {
    case ER_SEER_CFG_LPM_256_L4_128_EXT:
        /* disable metering */
        SOC_IF_ERROR_RETURN(READ_METER_CTLr(unit, &rval));
        soc_reg_field_set(unit, METER_CTLr, &rval, METER_BG_DISf, 1);
        SOC_IF_ERROR_RETURN(WRITE_METER_CTLr(unit, rval));
        break;
    case ER_SEER_CFG_LPM_384_L4_64_EXT:
        /* relocate L3_DEFIP_DATA and FP_POLICY_TABLE_EXTERNAL */
        SOC_IF_ERROR_RETURN(WRITE_ACL_START_OVRDr(unit, 0));
        SOC_IF_ERROR_RETURN(WRITE_ACL_END_OVRDr(unit, 0x1fffe));
        SOC_IF_ERROR_RETURN(WRITE_LPM_START_OVRDr(unit, 0x20000));
        SOC_IF_ERROR_RETURN(WRITE_LPM_END_OVRDr(unit, 0x7ffff));
        SOC_IF_ERROR_RETURN(WRITE_LPM_TOTAL_OVRDr(unit, 0x60000));
        SOC_IF_ERROR_RETURN(WRITE_POL_START_OVRDr(unit, 0x50000));
        SOC_IF_ERROR_RETURN(WRITE_RM_START_OVRDr(unit, 0xb0000));
        SOC_IF_ERROR_RETURN(READ_EXT_TCAM_CONFIG_0r(unit, &rval));
        soc_reg_field_set(unit, EXT_TCAM_CONFIG_0r, &rval, TCAM_OFFSET_OVRDf,
                          1);
        SOC_IF_ERROR_RETURN(WRITE_EXT_TCAM_CONFIG_0r(unit, rval));
        break;
    default:
        break;
    }

    if (sharing) {
        /* Get the number of lpm and acl entries.
         * When sharing is enabled, the values returned by
         * soc_mem_index_count() is 1/2 the actual # of entries.
         */
        lpm_count = soc_mem_index_count(unit, L3_DEFIP_TCAMm);
        acl_count = soc_mem_index_count(unit, FP_TCAM_EXTERNALm);

        /* Calculate sharing offsets. The ADDR values in these
         * registers correspond to 72b CAM addresses. 
         * Each acl entry is 144b and each lpm entry is 72b.
         */
        if (master) {
            /* Use the lower half of ACL CAM */
            offset = 0;
        } else {
            /* Use the upper half of ACL CAM */
            offset = (acl_count * 2);
        }
        SOC_IF_ERROR_RETURN(READ_ACL_START_OVRDr(unit, &rval));
        soc_reg_field_set(unit, ACL_START_OVRDr, &rval, 
                          ACL_START_ADDRf, offset);
        SOC_IF_ERROR_RETURN(WRITE_ACL_START_OVRDr(unit, rval));

        offset += ((acl_count * 2) - 2);
        SOC_IF_ERROR_RETURN(READ_ACL_END_OVRDr(unit, &rval));
        soc_reg_field_set(unit, ACL_END_OVRDr, &rval, 
                          ACL_END_ADDRf, offset);
        SOC_IF_ERROR_RETURN(WRITE_ACL_END_OVRDr(unit, rval));

        if (lpm_count > 0) {
            if (master) {
                /* Use the lower half of LPM CAM (starts after both 
                 * halves of the ACL CAM)
                 */
                offset = (acl_count * 4);
            } else {
                /* Use the upper half of LPM CAM */
                offset = (acl_count * 4) + lpm_count;
            }
            SOC_IF_ERROR_RETURN(READ_LPM_START_OVRDr(unit, &rval));
            soc_reg_field_set(unit, LPM_START_OVRDr, &rval, 
                              LPM_START_ADDRf, offset);
            SOC_IF_ERROR_RETURN(WRITE_LPM_START_OVRDr(unit, rval));

            offset += (lpm_count - 1);
            SOC_IF_ERROR_RETURN(READ_LPM_END_OVRDr(unit, &rval));
            soc_reg_field_set(unit, LPM_END_OVRDr, &rval, 
                              LPM_END_ADDRf, offset);
            SOC_IF_ERROR_RETURN(WRITE_LPM_END_OVRDr(unit, rval));

            SOC_IF_ERROR_RETURN(READ_LPM_TOTAL_OVRDr(unit, &rval));
            soc_reg_field_set(unit, LPM_TOTAL_OVRDr, &rval, 
                              LPM_TOTAL_ADDRf, lpm_count);
            SOC_IF_ERROR_RETURN(WRITE_LPM_TOTAL_OVRDr(unit, rval));
        }

        /* Need to program the SRAM override offsets, even though
         * the values are no different than the non-sharing case.
         */
        offset = 0;
        SOC_IF_ERROR_RETURN(READ_POL_START_OVRDr(unit, &rval));
        soc_reg_field_set(unit, POL_START_OVRDr, &rval, 
                          POL_START_ADDRf, offset);
        SOC_IF_ERROR_RETURN(WRITE_POL_START_OVRDr(unit, rval));

        if (SOC_PERSIST(unit)->er_memcfg.ext_table_cfg == 
            ER_SEER_CFG_LPM_128_L4_64_EXT) {
            offset = ER_SEER_384K;
        } else if (SOC_PERSIST(unit)->er_memcfg.ext_table_cfg == 
            ER_SEER_CFG_LPM_192_L4_32_EXT) {
            offset = ER_SEER_256K;
        } else { 
            offset = 0;
        }
        SOC_IF_ERROR_RETURN(READ_RM_START_OVRDr(unit, &rval));
        soc_reg_field_set(unit, RM_START_OVRDr, &rval, 
                          RM_START_ADDRf, offset);
        SOC_IF_ERROR_RETURN(WRITE_RM_START_OVRDr(unit, rval));
    }

    if (SAL_BOOT_QUICKTURN) {
        /* Variable nature TBD */
        SOC_IF_ERROR_RETURN(READ_QDR36_CONFIG_REG1_ISr(unit, &rval));
        soc_reg_field_set(unit, QDR36_CONFIG_REG1_ISr, &rval, ENABLE_DRRf, 1);
        soc_reg_field_set(unit, QDR36_CONFIG_REG1_ISr, &rval,
                          ENABLE_QK_ODTf, 0);
        soc_reg_field_set(unit, QDR36_CONFIG_REG1_ISr, &rval,
                          ENABLE_DQ_ODTf, 0);
        soc_reg_field_set(unit, QDR36_CONFIG_REG1_ISr, &rval, TCAM_MODEf, 1);
        soc_reg_field_set(unit, QDR36_CONFIG_REG1_ISr, &rval, SEL_AVG_ALGf, 1);
        SOC_IF_ERROR_RETURN(WRITE_QDR36_CONFIG_REG1_ISr(unit, rval));

        /* SOC_IF_ERROR_RETURN(WRITE_QDR36_CONFIG_REG2_ISr(unit, 0x50000)); */
        SOC_IF_ERROR_RETURN(READ_QDR36_CONFIG_REG2_ISr(unit, &rval));
        soc_reg_field_set(unit, QDR36_CONFIG_REG2_ISr, &rval,
                          PVT_ODTRES_VALf, 5);
        SOC_IF_ERROR_RETURN(WRITE_QDR36_CONFIG_REG2_ISr(unit, rval));
    } else {
        SOC_IF_ERROR_RETURN(READ_QDR36_CONFIG_REG1_ISr(unit, &rval));
        soc_reg_field_set(unit, QDR36_CONFIG_REG1_ISr, &rval, TCAM_MODEf, 1);
        SOC_IF_ERROR_RETURN(WRITE_QDR36_CONFIG_REG1_ISr(unit, rval));
    }

    return SOC_E_NONE;
}

void
soc_easyrider_mem_config(int unit)
{
    soc_control_t       *soc;
    soc_persist_t       *sop;
    sop_memcfg_er_t     *mcer;
    int                 prop_val;
    int                 master, slave, sharing = 0;

    soc = SOC_CONTROL(unit);
    sop = SOC_PERSIST(unit);
    mcer = &(sop->er_memcfg);

    /* Properties galore! */
    prop_val = soc_property_get(unit, spn_SEER_EXT_TABLE_CFG,
                                ER_SEER_CFG_NO_EXT);
    mcer->ext_table_cfg = prop_val;
    mcer->tcam_select = soc_property_get(unit, spn_SEER_EXT_TCAM_SELECT,
                                         ER_EXT_TCAM_TYPE1);

    master = soc_property_get(unit, spn_EXT_TCAM_SHARING_MASTER, 0);
    slave  = soc_property_get(unit, spn_EXT_TCAM_SHARING_SLAVE,  0);
    if (master || slave) {
        sharing = 1;
    }

    if (prop_val != ER_SEER_CFG_L2_512_EXT) {
        sop->memState[L2_ENTRY_EXTERNALm].index_max = -1;
    }

    switch (prop_val) {
    case ER_SEER_CFG_NO_EXT:
    case ER_SEER_CFG_L2_512_EXT:
        sop->memState[L3_DEFIP_ALGm].index_max = ER_SEER_16K - 1;
        sop->memState[L3_DEFIP_CAMm].index_max = -1;
        sop->memState[L3_DEFIP_TCAMm].index_max = -1;
        sop->memState[L3_DEFIP_DATAm].index_max = -1;
        sop->memState[NEXT_HOP_EXTm].index_max = -1;
        sop->memState[FP_EXTERNALm].index_max = -1;
        sop->memState[FP_TCAM_EXTERNALm].index_max = -1;
        sop->memState[FP_POLICY_EXTERNALm].index_max = -1;
        sop->memState[FP_METER_TABLE_EXTm].index_max = -1;
        sop->memState[FP_COUNTER_EXTm].index_max = -1;
        mcer->tcam_select = ER_EXT_TCAM_NONE;
        break;

    case ER_SEER_CFG_LPM_256_EXT:
        sop->memState[L3_DEFIP_ALGm].index_max = ER_SEER_512K - 1;
        sop->memState[L3_DEFIP_CAMm].index_max = -1;
        sop->memState[L3_DEFIP_TCAMm].index_max = -1;
        sop->memState[L3_DEFIP_DATAm].index_max = -1;
        sop->memState[NEXT_HOP_INTm].index_max = -1;
        sop->memState[FP_EXTERNALm].index_max = -1;
        sop->memState[FP_TCAM_EXTERNALm].index_max = -1;
        sop->memState[FP_POLICY_EXTERNALm].index_max = -1;
        sop->memState[FP_METER_TABLE_EXTm].index_max = -1;
        sop->memState[FP_COUNTER_EXTm].index_max = -1;
        mcer->tcam_select = ER_EXT_TCAM_NONE;
        break;

    case ER_SEER_CFG_L4_192_EXT:
        sop->memState[L3_DEFIP_ALGm].index_max = ER_SEER_16K - 1;
        sop->memState[L3_DEFIP_CAMm].index_max = -1;
        sop->memState[L3_DEFIP_TCAMm].index_max = -1;
        sop->memState[L3_DEFIP_DATAm].index_max = -1;
        sop->memState[NEXT_HOP_EXTm].index_max = -1;
        sop->memState[FP_EXTERNALm].index_max = ER_SEER_192K - 1;
        sop->memState[FP_TCAM_EXTERNALm].index_max = ER_SEER_192K - 1;
        sop->memState[FP_POLICY_EXTERNALm].index_max = ER_SEER_192K - 1;
        sop->memState[FP_METER_TABLE_EXTm].index_max = ER_SEER_192K - 1;
        sop->memState[FP_COUNTER_EXTm].index_max = ER_SEER_192K - 1;
        break;

    case ER_SEER_CFG_L4_96_EXT:
        if (sharing) {
            sop->memState[FP_EXTERNALm].index_max = ER_SEER_48K - 1;
            sop->memState[FP_TCAM_EXTERNALm].index_max = ER_SEER_48K - 1;
            sop->memState[FP_POLICY_EXTERNALm].index_max = ER_SEER_48K - 1;
            sop->memState[FP_METER_TABLE_EXTm].index_max = ER_SEER_48K - 1;
            sop->memState[FP_COUNTER_EXTm].index_max = ER_SEER_48K - 1;
        } else {
            sop->memState[FP_EXTERNALm].index_max = ER_SEER_96K - 1;
            sop->memState[FP_TCAM_EXTERNALm].index_max = ER_SEER_96K - 1;
            sop->memState[FP_POLICY_EXTERNALm].index_max = ER_SEER_96K - 1;
            sop->memState[FP_METER_TABLE_EXTm].index_max = ER_SEER_96K - 1;
            sop->memState[FP_COUNTER_EXTm].index_max = ER_SEER_96K - 1;
        }
        sop->memState[L3_DEFIP_ALGm].index_max = ER_SEER_16K - 1;
        sop->memState[L3_DEFIP_CAMm].index_max = -1;
        sop->memState[L3_DEFIP_TCAMm].index_max = -1;
        sop->memState[L3_DEFIP_DATAm].index_max = -1;
        sop->memState[NEXT_HOP_EXTm].index_max = -1;
        break;

    case ER_SEER_CFG_LPM_256_L4_128_EXT:
        sop->memState[L3_DEFIP_ALGm].index_max = -1;
        sop->memState[L3_DEFIP_CAMm].index_max = ER_SEER_256K - 1;
        sop->memState[L3_DEFIP_TCAMm].index_max = ER_SEER_256K - 1;
        sop->memState[L3_DEFIP_DATAm].index_max = ER_SEER_256K - 1;
        sop->memState[NEXT_HOP_INTm].index_max = -1;
        sop->memState[FP_EXTERNALm].index_max = ER_SEER_128K - 1;
        sop->memState[FP_TCAM_EXTERNALm].index_max = ER_SEER_128K - 1;
        sop->memState[FP_POLICY_EXTERNALm].index_max = ER_SEER_128K - 1;
        sop->memState[FP_METER_TABLE_EXTm].index_max = ER_SEER_128K - 1;
        sop->memState[FP_COUNTER_EXTm].index_max = ER_SEER_128K - 1;
        break;

    case ER_SEER_CFG_LPM_384_L4_64_EXT:
        sop->memState[L3_DEFIP_ALGm].index_max = -1;
        sop->memState[L3_DEFIP_CAMm].index_max = ER_SEER_384K - 1;
        sop->memState[L3_DEFIP_TCAMm].index_max = ER_SEER_384K - 1;
        sop->memState[L3_DEFIP_DATAm].index_max = ER_SEER_384K - 1;
        sop->memState[NEXT_HOP_INTm].index_max = -1;
        sop->memState[FP_EXTERNALm].index_max = ER_SEER_64K - 1;
        sop->memState[FP_TCAM_EXTERNALm].index_max = ER_SEER_64K - 1;
        sop->memState[FP_POLICY_EXTERNALm].index_max = ER_SEER_64K - 1;
        sop->memState[FP_METER_TABLE_EXTm].index_max = ER_SEER_64K - 1;
        sop->memState[FP_COUNTER_EXTm].index_max = ER_SEER_64K - 1;
        break;

    case ER_SEER_CFG_LPM_128_L4_64_EXT:
        if (sharing) {
            sop->memState[L3_DEFIP_CAMm].index_max = ER_SEER_64K - 1;
            sop->memState[L3_DEFIP_TCAMm].index_max = ER_SEER_64K - 1;
            sop->memState[L3_DEFIP_DATAm].index_max = ER_SEER_64K - 1;
            sop->memState[FP_EXTERNALm].index_max = ER_SEER_32K - 1;
            sop->memState[FP_TCAM_EXTERNALm].index_max = ER_SEER_32K - 1;
            sop->memState[FP_POLICY_EXTERNALm].index_max = ER_SEER_32K - 1;
            sop->memState[FP_METER_TABLE_EXTm].index_max = ER_SEER_32K - 1;
            sop->memState[FP_COUNTER_EXTm].index_max = ER_SEER_32K - 1;
        } else {
            sop->memState[L3_DEFIP_CAMm].index_max = ER_SEER_128K - 1;
            sop->memState[L3_DEFIP_TCAMm].index_max = ER_SEER_128K - 1;
            sop->memState[L3_DEFIP_DATAm].index_max = ER_SEER_128K - 1;
            sop->memState[FP_EXTERNALm].index_max = ER_SEER_64K - 1;
            sop->memState[FP_TCAM_EXTERNALm].index_max = ER_SEER_64K - 1;
            sop->memState[FP_POLICY_EXTERNALm].index_max = ER_SEER_64K - 1;
            sop->memState[FP_METER_TABLE_EXTm].index_max = ER_SEER_64K - 1;
            sop->memState[FP_COUNTER_EXTm].index_max = ER_SEER_64K - 1;
        }
        sop->memState[L3_DEFIP_ALGm].index_max = -1;
        sop->memState[NEXT_HOP_INTm].index_max = -1;
        break;

    case ER_SEER_CFG_LPM_192_L4_32_EXT:
        if (sharing) {
            sop->memState[L3_DEFIP_CAMm].index_max = ER_SEER_96K - 1;
            sop->memState[L3_DEFIP_TCAMm].index_max = ER_SEER_96K - 1;
            sop->memState[L3_DEFIP_DATAm].index_max = ER_SEER_96K - 1;
            sop->memState[FP_EXTERNALm].index_max = ER_SEER_16K - 1;
            sop->memState[FP_TCAM_EXTERNALm].index_max = ER_SEER_16K - 1;
            sop->memState[FP_POLICY_EXTERNALm].index_max = ER_SEER_16K - 1;
            sop->memState[FP_METER_TABLE_EXTm].index_max = ER_SEER_16K - 1;
            sop->memState[FP_COUNTER_EXTm].index_max = ER_SEER_16K - 1;
        } else {
            sop->memState[L3_DEFIP_CAMm].index_max = ER_SEER_192K - 1;
            sop->memState[L3_DEFIP_TCAMm].index_max = ER_SEER_192K - 1;
            sop->memState[L3_DEFIP_DATAm].index_max = ER_SEER_192K - 1;
            sop->memState[FP_EXTERNALm].index_max = ER_SEER_32K - 1;
            sop->memState[FP_TCAM_EXTERNALm].index_max = ER_SEER_32K - 1;
            sop->memState[FP_POLICY_EXTERNALm].index_max = ER_SEER_32K - 1;
            sop->memState[FP_METER_TABLE_EXTm].index_max = ER_SEER_32K - 1;
            sop->memState[FP_COUNTER_EXTm].index_max = ER_SEER_32K - 1;
        }
        sop->memState[L3_DEFIP_ALGm].index_max = -1;
        sop->memState[NEXT_HOP_INTm].index_max = -1;
        break;

    case ER_SEER_CFG_LPM_448_EXT:
        sop->memState[L3_DEFIP_ALGm].index_max = -1;
        sop->memState[L3_DEFIP_CAMm].index_max = ER_SEER_448K - 1;
        sop->memState[L3_DEFIP_TCAMm].index_max = ER_SEER_448K - 1;
        sop->memState[L3_DEFIP_DATAm].index_max = ER_SEER_448K - 1;
        sop->memState[NEXT_HOP_INTm].index_max = -1;
        sop->memState[FP_EXTERNALm].index_max = -1;
        sop->memState[FP_TCAM_EXTERNALm].index_max = -1;
        sop->memState[FP_POLICY_EXTERNALm].index_max = -1;
        sop->memState[FP_METER_TABLE_EXTm].index_max = -1;
        sop->memState[FP_COUNTER_EXTm].index_max = -1;
        break;

    case ER_SEER_CFG_LPM_896_EXT:
        sop->memState[L3_DEFIP_ALGm].index_max = -1;
        sop->memState[L3_DEFIP_CAMm].index_max = ER_SEER_896K - 1;
        sop->memState[L3_DEFIP_TCAMm].index_max = ER_SEER_896K - 1;
        sop->memState[L3_DEFIP_DATAm].index_max = ER_SEER_896K - 1;
        sop->memState[NEXT_HOP_INTm].index_max = -1;
        sop->memState[FP_EXTERNALm].index_max = -1;
        sop->memState[FP_TCAM_EXTERNALm].index_max = -1;
        sop->memState[FP_POLICY_EXTERNALm].index_max = -1;
        sop->memState[FP_METER_TABLE_EXTm].index_max = -1;
        sop->memState[FP_COUNTER_EXTm].index_max = -1;
        break;

    default:
        SOC_ERROR_PRINT((DK_WARN,
                         "soc_easyrider_mem_config:"
                         " unit %d unrecognized external memory config\n",
                         unit));
        mcer->ext_table_cfg = 0;
        mcer->tcam_select = ER_EXT_TCAM_NONE;
    }

    prop_val = soc_property_get(unit, spn_SEER_HOST_HASH_TABLE_CFG,
                            ER_SEER_HOST_HASH_CFG_L2_HALF_V4_QUART_V6_QUART);
    mcer->host_hash_table_cfg = prop_val;
    mcer->l3v4_search_offset = 0;
    mcer->l3v6_search_offset = 0;
    /* Reset hash calculations for reconfig! */
    soc->hash_mask_l2x = 0;
    soc->hash_mask_l2x_ext = 0;
    soc->hash_mask_l3x = 0;
    soc->hash_mask_l3v6 = 0;

    switch (prop_val) {
    case ER_SEER_HOST_HASH_CFG_L2_ALL:
        sop->memState[L2_ENTRY_INTERNALm].index_max = ER_SEER_32K - 1;
        sop->memState[L3_ENTRY_V4m].index_max = -1;
        sop->memState[L3_ENTRY_V6m].index_max = -1;
        break;

    case ER_SEER_HOST_HASH_CFG_L2_HALF_V4_HALF:
        sop->memState[L2_ENTRY_INTERNALm].index_max = ER_SEER_16K - 1;
        sop->memState[L3_ENTRY_V4m].index_max = ER_SEER_16K - 1;
        sop->memState[L3_ENTRY_V6m].index_max = -1;
        mcer->l3v4_search_offset = ER_SEER_16K;
        break;

    case ER_SEER_HOST_HASH_CFG_L2_HALF_V4_QUART_V6_QUART:
        sop->memState[L2_ENTRY_INTERNALm].index_max = ER_SEER_16K - 1;
        sop->memState[L3_ENTRY_V4m].index_max = ER_SEER_8K - 1;
        sop->memState[L3_ENTRY_V6m].index_max = ER_SEER_4K - 1;
        mcer->l3v4_search_offset = ER_SEER_16K;
        mcer->l3v6_search_offset = ER_SEER_12K;
        break;

    case ER_SEER_HOST_HASH_CFG_L2_HALF_V6_HALF:
        sop->memState[L2_ENTRY_INTERNALm].index_max = ER_SEER_16K - 1;
        sop->memState[L3_ENTRY_V4m].index_max = -1;
        sop->memState[L3_ENTRY_V6m].index_max = ER_SEER_8K - 1;
        mcer->l3v6_search_offset = ER_SEER_8K;
        break;

    case ER_SEER_HOST_HASH_CFG_L2_QUART_V4_HALF_V6_QUART:
        sop->memState[L2_ENTRY_INTERNALm].index_max = ER_SEER_8K - 1;
        sop->memState[L3_ENTRY_V4m].index_max = ER_SEER_16K - 1;
        sop->memState[L3_ENTRY_V6m].index_max = ER_SEER_4K - 1;
        mcer->l3v4_search_offset = ER_SEER_8K;
        mcer->l3v6_search_offset = ER_SEER_12K;
        break;

    case ER_SEER_HOST_HASH_CFG_L2_QUART_V4_QUART_V6_HALF:
        sop->memState[L2_ENTRY_INTERNALm].index_max = ER_SEER_8K - 1;
        sop->memState[L3_ENTRY_V4m].index_max = ER_SEER_8K - 1;
        sop->memState[L3_ENTRY_V6m].index_max = ER_SEER_8K - 1;
        mcer->l3v4_search_offset = ER_SEER_8K;
        mcer->l3v6_search_offset = ER_SEER_8K;
        break;

    case ER_SEER_HOST_HASH_CFG_V4_ALL:
        sop->memState[L2_ENTRY_INTERNALm].index_max = -1;
        sop->memState[L3_ENTRY_V4m].index_max = ER_SEER_32K - 1;
        sop->memState[L3_ENTRY_V6m].index_max = -1;
        break;

    case ER_SEER_HOST_HASH_CFG_V4_HALF_V6_HALF:
        sop->memState[L2_ENTRY_INTERNALm].index_max = -1;
        sop->memState[L3_ENTRY_V4m].index_max = ER_SEER_16K - 1;
        sop->memState[L3_ENTRY_V6m].index_max = ER_SEER_8K - 1;
        mcer->l3v6_search_offset = ER_SEER_8K;
        break;

    case ER_SEER_HOST_HASH_CFG_V6_ALL:
        sop->memState[L2_ENTRY_INTERNALm].index_max = -1;
        sop->memState[L3_ENTRY_V4m].index_max = -1;
        sop->memState[L3_ENTRY_V6m].index_max = ER_SEER_16K - 1;
        break;

    default:
        SOC_ERROR_PRINT((DK_WARN,
                         "soc_easyrider_mem_config:"
                         " unit %d unrecognized host hash memory config\n",
                         unit));
        mcer->host_hash_table_cfg = 2;
    }


    prop_val = soc_property_get(unit, spn_SEER_MVL_HASH_TABLE_CFG,
                                ER_SEER_MVL_HASH_CFG_MVL_HALF_MYST_HALF);
    mcer->mvl_hash_table_cfg = prop_val;
    mcer->mystation_search_offset = 0;
    /* Reset hash calculations for reconfig! */
    soc->hash_mask_vlan_mac = 0;
    soc->hash_mask_my_station = 0;

    switch (prop_val) {
    case ER_SEER_MVL_HASH_CFG_MVL_ALL:
        sop->memState[VLAN_MACm].index_max = ER_SEER_4K - 1;
        sop->memState[MY_STATIONm].index_max = -1;
        break;

    case ER_SEER_MVL_HASH_CFG_MVL_HALF_MYST_HALF:
        sop->memState[VLAN_MACm].index_max = ER_SEER_2K - 1;
        sop->memState[MY_STATIONm].index_max = ER_SEER_2K - 1;
        mcer->mystation_search_offset = ER_SEER_2K;
        break;

    case ER_SEER_MVL_HASH_CFG_MYST_ALL:
        sop->memState[VLAN_MACm].index_max = -1;
        sop->memState[MY_STATIONm].index_max = ER_SEER_4K - 1;
        break;

    default:
        SOC_ERROR_PRINT((DK_WARN,
                         "soc_easyrider_mem_config:"
                         " unit %d unrecognized MVL hash memory config\n",
                         unit));
        mcer->mvl_hash_table_cfg = 1;
    }

    
    sop->memState[MMU_IBSm].index_max = ER_SEER_32K - 1;
}

int
_soc_easyrider_ext_sram_tune(int unit)
{
    uint32      rval;
    int         slice, thev_75_val;
    soc_persist_t       *sop;
    sop_memcfg_er_t     *mcer;

    sop = SOC_PERSIST(unit);
    mcer = &(sop->er_memcfg);

    /* check for PVT done status */
    thev_75_val = 0; /* thevenin_75 value is 0 for SRAM */
    _soc_easyrider_check_pvt(unit, DDR72_CONFIG_REG2_ISr,
                             DDR72_STATUS_REG1_ISr, DDR72_STATUS_REG2_ISr,
                             1, thev_75_val, "DDR_72");

    /* Adjust on chip termination */
    SOC_IF_ERROR_RETURN(READ_DDR72_CONFIG_REG1_ISr(unit, &rval));
    soc_reg_field_set(unit, DDR72_CONFIG_REG1_ISr, &rval, DLL90_OFFSET_TXf,
                      soc_property_get(unit, spn_DDR72_DLL90_OFFSET_TX, 4));
    soc_reg_field_set(unit, DDR72_CONFIG_REG1_ISr, &rval, DLL90_OFFSET0_QKf,
                      soc_property_get(unit, spn_DDR72_DLL90_OFFSET0_QK, 4));
    soc_reg_field_set(unit, DDR72_CONFIG_REG1_ISr, &rval, DLL90_OFFSET1f,
                      soc_property_get(unit, spn_DDR72_DLL90_OFFSET1, 4));
    soc_reg_field_set(unit, DDR72_CONFIG_REG1_ISr, &rval, DLL90_OFFSET2f,
                      soc_property_get(unit, spn_DDR72_DLL90_OFFSET2, 4));
    soc_reg_field_set(unit, DDR72_CONFIG_REG1_ISr, &rval, DLL90_OFFSET3f,
                      soc_property_get(unit, spn_DDR72_DLL90_OFFSET3, 4));
    SOC_IF_ERROR_RETURN(WRITE_DDR72_CONFIG_REG1_ISr(unit, rval));

    SOC_IF_ERROR_RETURN(READ_DDR72_CONFIG_REG2_ISr(unit, &rval));
    soc_reg_field_set(unit, DDR72_CONFIG_REG2_ISr, &rval, SEL_EARLY2_3f,
                      soc_property_get(unit, spn_DDR72_SEL_EARLY2_3, 0));
    soc_reg_field_set(unit, DDR72_CONFIG_REG2_ISr, &rval, SEL_EARLY1_3f,
                      soc_property_get(unit, spn_DDR72_SEL_EARLY1_3, 1));
    soc_reg_field_set(unit, DDR72_CONFIG_REG2_ISr, &rval, SEL_EARLY2_2f,
                      soc_property_get(unit, spn_DDR72_SEL_EARLY2_2, 0));
    soc_reg_field_set(unit, DDR72_CONFIG_REG2_ISr, &rval, SEL_EARLY1_2f,
                      soc_property_get(unit, spn_DDR72_SEL_EARLY1_2, 1));
    soc_reg_field_set(unit, DDR72_CONFIG_REG2_ISr, &rval, SEL_EARLY2_1f,
                      soc_property_get(unit, spn_DDR72_SEL_EARLY2_1, 0));
    soc_reg_field_set(unit, DDR72_CONFIG_REG2_ISr, &rval, SEL_EARLY1_1f,
                      soc_property_get(unit, spn_DDR72_SEL_EARLY1_1, 1));
    soc_reg_field_set(unit, DDR72_CONFIG_REG2_ISr, &rval, SEL_EARLY2_0f,
                      soc_property_get(unit, spn_DDR72_SEL_EARLY2_0, 0));
    soc_reg_field_set(unit, DDR72_CONFIG_REG2_ISr, &rval, SEL_EARLY1_0f,
                      soc_property_get(unit, spn_DDR72_SEL_EARLY1_0, 1));
    SOC_IF_ERROR_RETURN(WRITE_DDR72_CONFIG_REG2_ISr(unit, rval));
        
    SOC_IF_ERROR_RETURN(READ_DDR72_CONFIG_REG3_ISr(unit, &rval));
    soc_reg_field_set(unit, DDR72_CONFIG_REG3_ISr, &rval, DLL90_OFFSET_QKBf,
                      soc_property_get(unit, spn_DDR72_DLL90_OFFSET_QKB, 4));
    soc_reg_field_set(unit, DDR72_CONFIG_REG3_ISr, &rval, PHASE_SEL3f,
                      soc_property_get(unit, spn_DDR72_PHASE_SEL, 0));
    soc_reg_field_set(unit, DDR72_CONFIG_REG3_ISr, &rval, PHASE_SEL2f,
                      soc_property_get(unit, spn_DDR72_PHASE_SEL, 0));
    soc_reg_field_set(unit, DDR72_CONFIG_REG3_ISr, &rval, PHASE_SEL1f,
                      soc_property_get(unit, spn_DDR72_PHASE_SEL, 0));
    soc_reg_field_set(unit, DDR72_CONFIG_REG3_ISr, &rval, PHASE_SEL0f,
                      soc_property_get(unit, spn_DDR72_PHASE_SEL, 0));
    soc_reg_field_set(unit, DDR72_CONFIG_REG3_ISr, &rval, OVRD_SM3_ENf,
                      soc_property_get(unit, spn_DDR72_OVRD_SM_EN, 0));
    soc_reg_field_set(unit, DDR72_CONFIG_REG3_ISr, &rval, OVRD_SM2_ENf,
                      soc_property_get(unit, spn_DDR72_OVRD_SM_EN, 0));
    soc_reg_field_set(unit, DDR72_CONFIG_REG3_ISr, &rval, OVRD_SM1_ENf,
                      soc_property_get(unit, spn_DDR72_OVRD_SM_EN, 0));
    soc_reg_field_set(unit, DDR72_CONFIG_REG3_ISr, &rval, OVRD_SM0_ENf,
                      soc_property_get(unit, spn_DDR72_OVRD_SM_EN, 0));
    SOC_IF_ERROR_RETURN(WRITE_DDR72_CONFIG_REG3_ISr(unit, rval));

    /* check for PVT done status */
    if (mcer->ext_table_cfg == ER_SEER_CFG_L2_512_EXT ||
        mcer->ext_table_cfg == ER_SEER_CFG_LPM_256_EXT) {
        thev_75_val = 0; /* thevenin_75 value is 0 for SRAM */
    } else {
        thev_75_val = 1; /* thevenin_75 value is 1 for TCAM */
    }
    _soc_easyrider_check_pvt(unit, QDR36_CONFIG_REG2_ISr,
                             QDR36_STATUS_REG1_ISr, QDR36_STATUS_REG2_ISr,
                             0, thev_75_val, "QDR_36");

    SOC_IF_ERROR_RETURN(READ_QDR36_CONFIG_REG1_ISr(unit, &rval));
    soc_reg_field_set(unit, QDR36_CONFIG_REG1_ISr, &rval,
                      DLL90_OFFSET_TXf,
                      soc_property_get(unit, spn_QDR36_DLL90_OFFSET_TX,
                                       SAL_BOOT_QUICKTURN ? 0 : 4));
    soc_reg_field_set(unit, QDR36_CONFIG_REG1_ISr, &rval,
                      DLL90_OFFSET_QKBf,
                      soc_property_get(unit, spn_QDR36_DLL90_OFFSET_QKB,
                                       SAL_BOOT_QUICKTURN ? 0 : 4));
    soc_reg_field_set(unit, QDR36_CONFIG_REG1_ISr, &rval,
                      DLL90_OFFSET_QKf,
                      soc_property_get(unit, spn_QDR36_DLL90_OFFSET_QK,
                                       SAL_BOOT_QUICKTURN ? 0 : 4));
    SOC_IF_ERROR_RETURN(WRITE_QDR36_CONFIG_REG1_ISr(unit, rval));

    SOC_IF_ERROR_RETURN(READ_QDR36_CONFIG_REG2_ISr(unit, &rval));
    soc_reg_field_set(unit, QDR36_CONFIG_REG2_ISr, &rval, SEL_EARLY2_1f,
                      soc_property_get(unit, spn_QDR36_SEL_EARLY2_1, 0));
    soc_reg_field_set(unit, QDR36_CONFIG_REG2_ISr, &rval, SEL_EARLY1_1f,
                      soc_property_get(unit, spn_QDR36_SEL_EARLY1_1, 1));
    soc_reg_field_set(unit, QDR36_CONFIG_REG2_ISr, &rval, SEL_EARLY2_0f,
                      soc_property_get(unit, spn_QDR36_SEL_EARLY2_0, 0));
    soc_reg_field_set(unit, QDR36_CONFIG_REG2_ISr, &rval, SEL_EARLY1_0f,
                      soc_property_get(unit, spn_QDR36_SEL_EARLY1_0, 1));
    SOC_IF_ERROR_RETURN(WRITE_QDR36_CONFIG_REG2_ISr(unit, rval));

    SOC_IF_ERROR_RETURN(READ_QDR36_CONFIG_REG3_ISr(unit, &rval));
    soc_reg_field_set(unit, QDR36_CONFIG_REG3_ISr, &rval, PHASE_SEL1f,
                      soc_property_get(unit, spn_QDR36_PHASE_SEL, 0));
    soc_reg_field_set(unit, QDR36_CONFIG_REG3_ISr, &rval, PHASE_SEL0f,
                      soc_property_get(unit, spn_QDR36_PHASE_SEL, 0));
    soc_reg_field_set(unit, QDR36_CONFIG_REG3_ISr, &rval, OVRD_SM1_ENf,
                      soc_property_get(unit, spn_QDR36_OVRD_SM_EN, 0));
    soc_reg_field_set(unit, QDR36_CONFIG_REG3_ISr, &rval, OVRD_SM0_ENf,
                      soc_property_get(unit, spn_QDR36_OVRD_SM_EN, 0));
    SOC_IF_ERROR_RETURN(WRITE_QDR36_CONFIG_REG3_ISr(unit, rval));

    /* Force relock */
    SOC_IF_ERROR_RETURN(READ_DDR72_CONFIG_REG1_ISr(unit, &rval));
    soc_reg_field_set(unit, DDR72_CONFIG_REG1_ISr, &rval, RELOCK_DLLf, 1);
    SOC_IF_ERROR_RETURN(WRITE_DDR72_CONFIG_REG1_ISr(unit, rval));
    sal_usleep(1 * MILLISECOND_USEC);
    soc_reg_field_set(unit, DDR72_CONFIG_REG1_ISr, &rval, ENABLE_DDRf, 0);
    SOC_IF_ERROR_RETURN(WRITE_DDR72_CONFIG_REG1_ISr(unit, rval));
    sal_usleep(1 * MILLISECOND_USEC);

    soc_reg_field_set(unit, DDR72_CONFIG_REG1_ISr, &rval, RELOCK_DLLf, 0);
    SOC_IF_ERROR_RETURN(WRITE_DDR72_CONFIG_REG1_ISr(unit, rval));
    sal_usleep(1 * MILLISECOND_USEC);
    soc_reg_field_set(unit, DDR72_CONFIG_REG1_ISr, &rval, ENABLE_DDRf, 1);
    SOC_IF_ERROR_RETURN(WRITE_DDR72_CONFIG_REG1_ISr(unit, rval));

    SOC_IF_ERROR_RETURN(READ_QDR36_CONFIG_REG1_ISr(unit, &rval));
    soc_reg_field_set(unit, QDR36_CONFIG_REG1_ISr, &rval, RELOCK_DLLf, 1);
    SOC_IF_ERROR_RETURN(WRITE_QDR36_CONFIG_REG1_ISr(unit, rval));
    sal_usleep(1 * MILLISECOND_USEC);
    soc_reg_field_set(unit, QDR36_CONFIG_REG1_ISr, &rval, ENABLE_DRRf, 0);
    SOC_IF_ERROR_RETURN(WRITE_QDR36_CONFIG_REG1_ISr(unit, rval));
    sal_usleep(1 * MILLISECOND_USEC);

    soc_reg_field_set(unit, QDR36_CONFIG_REG1_ISr, &rval, RELOCK_DLLf, 0);
    SOC_IF_ERROR_RETURN(WRITE_QDR36_CONFIG_REG1_ISr(unit, rval));
    sal_usleep(1 * MILLISECOND_USEC);
    soc_reg_field_set(unit, QDR36_CONFIG_REG1_ISr, &rval, ENABLE_DRRf, 1);
    SOC_IF_ERROR_RETURN(WRITE_QDR36_CONFIG_REG1_ISr(unit, rval));
    sal_usleep(1 * MILLISECOND_USEC);

    /* Check if DLLs are locked */
    for(slice = 0; slice < 4; slice++) {
        /* Select the DLL lock status for each slice */
        SOC_IF_ERROR_RETURN(READ_DDR72_CONFIG_REG3_ISr(unit, &rval));
        soc_reg_field_set(unit, DDR72_CONFIG_REG3_ISr, &rval,
                          DLL_MON_SELf, slice);
        SOC_IF_ERROR_RETURN(WRITE_DDR72_CONFIG_REG3_ISr(unit, rval));

        /* check for DLL lock */
        _soc_er_ddr_dll_lock_check(unit, DDR72_STATUS_REG2_ISr, 
                                   QK_DLL90_LOCKEDf, slice, 
                                   "DDR72", "QK_DLL90");
        _soc_er_ddr_dll_lock_check(unit, DDR72_STATUS_REG2_ISr, 
                                   QK_DLLDSKW_LOCKEDf, slice, 
                                   "DDR72", "QK_DLLDSKW");
        _soc_er_ddr_dll_lock_check(unit, DDR72_STATUS_REG2_ISr, 
                                   TX_DLL90_LOCKEDf, slice, 
                                   "DDR72", "TX_DLL90");
        if (soc_feature(unit, soc_feature_deskew_dll)) {
            _soc_er_ddr_dll_lock_check(unit, DDR72_STATUS_REG2_ISr, 
                                       TX_DLLDSKW_LOCKEDf, slice, 
                                       "DDR72", "TX_DLLDSKW");
        }
    }

    if (SOC_PERSIST(unit)->er_memcfg.ext_table_cfg != ER_SEER_CFG_NO_EXT) {
        _soc_er_ddr_dll_lock_check(unit, QDR36_STATUS_REG2_ISr, 
                                   QK_DLL90_LOCKEDf, 0, 
                                   "QDR36", "QK_DLL90");
        _soc_er_ddr_dll_lock_check(unit, QDR36_STATUS_REG2_ISr, 
                                   QK_DLLDSKW_LOCKEDf, 0, 
                                   "QDR36", "QK_DLLDSKW");
        _soc_er_ddr_dll_lock_check(unit, QDR36_STATUS_REG2_ISr, 
                                   QKB_DLL90_LOCKEDf, 0, 
                                   "QDR36", "QKB_DLL90");
        _soc_er_ddr_dll_lock_check(unit, QDR36_STATUS_REG2_ISr, 
                                   QKB_DLLDSKW_LOCKEDf, 0, 
                                   "QDR36", "QKB_DLLDSKW");
        _soc_er_ddr_dll_lock_check(unit, QDR36_STATUS_REG2_ISr, 
                                   TX_DLL90_LOCKEDf, 0, 
                                   "QDR36", "TX_DLL90");
        _soc_er_ddr_dll_lock_check(unit, QDR36_STATUS_REG2_ISr, 
                                   TX_DLLDSKW_LOCKEDf, 0, 
                                   "QDR36", "TX_DLLDSKW");
    }

    SOC_IF_ERROR_RETURN(READ_MCU_ISr(unit, &rval));
    soc_reg_field_set(unit, MCU_ISr, &rval, MCU_ENABLEf, 0);
    SOC_IF_ERROR_RETURN(WRITE_MCU_ISr(unit, rval));

    SOC_IF_ERROR_RETURN(READ_MCU_ISr(unit, &rval));
    soc_reg_field_set(unit, MCU_ISr, &rval, MCU_ENABLEf, 1);
    SOC_IF_ERROR_RETURN(WRITE_MCU_ISr(unit, rval));

    return SOC_E_NONE;
}

int
soc_easyrider_external_init(int unit)
{
    uint32		rval;
    soc_timeout_t       to;
    int                 errors, erbmp;
    int                 ext_cfg = SOC_PERSIST(unit)->er_memcfg.ext_table_cfg;
    int                 to_usec;

    if (SOC_IS_RELOADING(unit)) {
        return SOC_E_NONE;
    }

    to_usec =
        soc_property_get(unit, spn_SEER_INIT_TIMEOUT_USEC,
                         SAL_BOOT_QUICKTURN ? 60000000 : 50000);

    if (!SAL_BOOT_SIMULATION) {
        
        SOC_IF_ERROR_RETURN(_soc_easyrider_ext_sram_tune(unit));
    }

    SOC_IF_ERROR_RETURN
        (soc_easyrider_ext_sram_enable_set(unit, BCM_EXT_SRAM_HSE_MODE,
                                           TRUE, TRUE));

    if (ext_cfg != ER_SEER_CFG_LPM_256_EXT) {
        /* HSE external init start */
        rval = 0;
        soc_reg_field_set(unit, HSE_DTU_LTE_ADR0r, &rval, DTU_LTE_ADR0f, 0);
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_ADR0r(unit, rval));

        rval = 0;
        soc_reg_field_set(unit, HSE_DTU_LTE_ADR1r, &rval,
                          DTU_LTE_ADR1f, 0xffffe);
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_ADR1r(unit, rval));

        rval = 0;
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_TMODE1r(unit, rval));

        rval = 0;
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, R2W_NOPSf, 3);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, W2R_NOPSf, 3);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, EM_LATENCY7f,
                          soc_property_get(unit, spn_SEER_HSE_EM_LATENCY7, 0));
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, FOREVERf, 0);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, ADR_MODEf, 2);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, LOOP_MODEf, 3);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, CLR_STATUSf, 0);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval,
                          START_LAB_TESTf, 1);
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_TMODE0r(unit, rval));
    } else {
        /* ext_cfg == ER_SEER_CFG_LPM_256_EXT */
        rval = 0;
        soc_reg_field_set(unit, HSE_DTU_LTE_ADR0r, &rval, DTU_LTE_ADR0f,
                          0x80000);
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_ADR0r(unit, rval));

        rval = 0;
        soc_reg_field_set(unit, HSE_DTU_LTE_ADR1r, &rval,
                          DTU_LTE_ADR1f, 0xffffe);
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_ADR1r(unit, rval));

        rval = 0;
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_TMODE1r(unit, rval));

        rval = 0;
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D0R_0r(unit, rval));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D0R_1r(unit, rval));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D0R_2r(unit, rval));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D0R_3r(unit, rval));

        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D0F_0r(unit, rval));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D0F_1r(unit, rval));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D0F_2r(unit, rval));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D0F_3r(unit, rval));

        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D1R_0r(unit, rval));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D1R_1r(unit, rval));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D1R_2r(unit, rval));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D1R_3r(unit, rval));

        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D1F_0r(unit, rval));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D1F_1r(unit, rval));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D1F_2r(unit, rval));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D1F_3r(unit, rval));

        rval = 0;
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, R2W_NOPSf, 3);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, W2R_NOPSf, 3);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, EM_LATENCY7f,
                          soc_property_get(unit, spn_SEER_HSE_EM_LATENCY7, 0));
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, FOREVERf, 0);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, ADR_MODEf, 2);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, LOOP_MODEf, 3);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, CLR_STATUSf, 0);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval,
                          START_LAB_TESTf, 1);
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_TMODE0r(unit, rval));

        /* Wait for first stage of init. */
        soc_timeout_init(&to, to_usec, 0);
        do {
            SOC_IF_ERROR_RETURN(READ_HSE_DTU_LTE_STS_DONEr(unit, &rval));
            if (soc_reg_field_get(unit, HSE_DTU_LTE_STS_DONEr, rval, DONEf)) {
                break;
            }
            if (soc_timeout_check(&to)) {
                soc_cm_debug(DK_WARN,
                             "unit %d : HSE external init timeout\n", unit);
                rval = 0; /* Don't report errors below */
                break;
            }
        } while (TRUE);

        errors = soc_reg_field_get(unit, HSE_DTU_LTE_STS_DONEr, rval, ERR_CNTf);
        erbmp = soc_reg_field_get(unit, HSE_DTU_LTE_STS_DONEr, rval,
                                  ERR_BITMAPf);

        if ((errors !=0) || (erbmp != 0)) {
            SOC_IF_ERROR_RETURN
                (soc_easyrider_ext_sram_enable_set(unit, BCM_EXT_SRAM_HSE_MODE,
                                                   FALSE, FALSE));
            soc_cm_debug(DK_WARN,
                         "unit %d : HSE external init has %d errors "
                         "with bitmap 0x%02x\n", unit, errors, erbmp);
            return SOC_E_INTERNAL;
        }

        /* Clean up HSE DTU before second stage */
        SOC_IF_ERROR_RETURN
            (soc_easyrider_ext_sram_enable_set(unit, BCM_EXT_SRAM_HSE_MODE,
                                               TRUE, TRUE));

        rval = 0;
        soc_reg_field_set(unit, HSE_DTU_LTE_ADR0r, &rval, DTU_LTE_ADR0f, 0);
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_ADR0r(unit, rval));

        rval = 0;
        soc_reg_field_set(unit, HSE_DTU_LTE_ADR1r, &rval,
                          DTU_LTE_ADR1f, 0x7fffe);
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_ADR1r(unit, rval));

        rval = 0;
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_TMODE1r(unit, rval));

        rval = 0x1feff;
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D0R_0r(unit, rval));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D0R_1r(unit, rval));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D0R_2r(unit, rval));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D0R_3r(unit, rval));

        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D0F_0r(unit, rval));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D0F_1r(unit, rval));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D0F_2r(unit, rval));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D0F_3r(unit, rval));

        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D1R_0r(unit, rval));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D1R_1r(unit, rval));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D1R_2r(unit, rval));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D1R_3r(unit, rval));

        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D1F_0r(unit, rval));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D1F_1r(unit, rval));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D1F_2r(unit, rval));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D1F_3r(unit, rval));

        rval = 0;
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, R2W_NOPSf, 3);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, W2R_NOPSf, 3);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, EM_LATENCY7f,
                          soc_property_get(unit, spn_SEER_HSE_EM_LATENCY7, 0));
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, FOREVERf, 0);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, ADR_MODEf, 2);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, LOOP_MODEf, 3);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, CLR_STATUSf, 0);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval,
                          START_LAB_TESTf, 1);
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_TMODE0r(unit, rval));
        
        if (!SAL_BOOT_PLISIM) {
            defip_alg_entry_t   dae;
            uint32              ent;
            soc_mem_t           mem = L3_DEFIP_ALGm;
            uint32              imin, imax;
 
            imin = soc_mem_index_min(unit, mem);
            imax = soc_mem_index_max(unit, mem);

            sal_memset(&dae, 0xff, sizeof(dae));
            for(ent = imin; ent < (imin + imax) / 2; ent++) {
                SOC_IF_ERROR_RETURN(soc_mem_write(unit, mem, MEM_BLOCK_ANY,
                                                  ent, &dae));
            }
        }
    }

    /* Wait for SEER HSE external memory initialization done. */
    soc_timeout_init(&to, to_usec, 0);
    do {
        SOC_IF_ERROR_RETURN(READ_HSE_DTU_LTE_STS_DONEr(unit, &rval));
        if (soc_reg_field_get(unit, HSE_DTU_LTE_STS_DONEr, rval, DONEf)) {
            break;
        }
        if (soc_timeout_check(&to)) {
            soc_cm_debug(DK_WARN,
                         "unit %d : HSE external init timeout\n", unit);
            rval = 0; /* Don't report errors below */
            break;
        }
    } while (TRUE);

    errors = soc_reg_field_get(unit, HSE_DTU_LTE_STS_DONEr, rval, ERR_CNTf);
    erbmp = soc_reg_field_get(unit, HSE_DTU_LTE_STS_DONEr, rval, ERR_BITMAPf);

    if ((errors !=0) || (erbmp != 0)) {
        SOC_IF_ERROR_RETURN
            (soc_easyrider_ext_sram_enable_set(unit, BCM_EXT_SRAM_HSE_MODE,
                                               FALSE, FALSE));

        soc_cm_debug(DK_WARN,
                     "unit %d : HSE external init has %d errors "
                     "with bitmap 0x%02x\n", unit, errors, erbmp);
        return SOC_E_INTERNAL;
    }

    /* Clean up HSE DTU afterwards */
    SOC_IF_ERROR_RETURN
        (soc_easyrider_ext_sram_enable_set(unit, BCM_EXT_SRAM_HSE_MODE,
                                           FALSE, TRUE));
    
    if (ext_cfg == ER_SEER_CFG_L2_512_EXT) {

        SOC_IF_ERROR_RETURN
            (soc_easyrider_ext_sram_enable_set(unit, BCM_EXT_SRAM_CSE_MODE,
                                                   TRUE, TRUE));

        rval = 0;
        soc_reg_field_set(unit, CSE_DTU_LTE_ADR0r, &rval, DTU_LTE_ADR0f, 0);
        SOC_IF_ERROR_RETURN(WRITE_CSE_DTU_LTE_ADR0r(unit, rval));

        rval = 0;
        soc_reg_field_set(unit, CSE_DTU_LTE_ADR1r, &rval,
                          DTU_LTE_ADR1f, 0xffffe);
        SOC_IF_ERROR_RETURN(WRITE_CSE_DTU_LTE_ADR1r(unit, rval));

        rval = 0;
        SOC_IF_ERROR_RETURN(WRITE_CSE_DTU_LTE_TMODE1r(unit, rval));

        rval = 0;
        soc_reg_field_set(unit, CSE_DTU_LTE_TMODE0r, &rval, R2W_NOPSf, 3);
        soc_reg_field_set(unit, CSE_DTU_LTE_TMODE0r, &rval, W2R_NOPSf, 3);
        soc_reg_field_set(unit, CSE_DTU_LTE_TMODE0r, &rval, EM_LATENCY7f,
                      soc_property_get(unit, spn_SEER_CSE_EM_LATENCY7, 0));
        soc_reg_field_set(unit, CSE_DTU_LTE_TMODE0r, &rval, FOREVERf, 0);
        soc_reg_field_set(unit, CSE_DTU_LTE_TMODE0r, &rval, ADR_MODEf, 2);
        soc_reg_field_set(unit, CSE_DTU_LTE_TMODE0r, &rval, LOOP_MODEf, 3);
        soc_reg_field_set(unit, CSE_DTU_LTE_TMODE0r, &rval, CLR_STATUSf, 0);
        soc_reg_field_set(unit, CSE_DTU_LTE_TMODE0r, &rval,
                          START_LAB_TESTf, 1);
        SOC_IF_ERROR_RETURN(WRITE_CSE_DTU_LTE_TMODE0r(unit, rval));
        
        /* Wait for SEER CSE external memory initialization done. */
        soc_timeout_init(&to, to_usec, 0);
        do {
            SOC_IF_ERROR_RETURN(READ_CSE_DTU_LTE_STS_DONEr(unit, &rval));
            if (soc_reg_field_get(unit, CSE_DTU_LTE_STS_DONEr, rval, DONEf)) {
                break;
            }
            if (soc_timeout_check(&to)) {
                soc_cm_debug(DK_WARN,
                             "unit %d : CSE external init timeout\n", unit);
                rval = 0; /* Don't report errors below */
                break;
            }
        } while (TRUE);

        errors = soc_reg_field_get(unit, CSE_DTU_LTE_STS_DONEr, rval, ERR_CNTf);
        erbmp = soc_reg_field_get(unit, CSE_DTU_LTE_STS_DONEr, rval,
                                  ERR_BITMAPf);

        /* Do not clean up CSE DTU afterwards, for HW diagnostics */
        SOC_IF_ERROR_RETURN
            (soc_easyrider_ext_sram_enable_set(unit, BCM_EXT_SRAM_CSE_MODE,
                                               FALSE, FALSE));

        if ((errors !=0) || (erbmp != 0)) {
            soc_cm_debug(DK_WARN,
                         "unit %d : CSE external init has %d errors "
                         "with bitmap 0x%02x\n", unit, errors, erbmp);
            return SOC_E_INTERNAL;
        }
    }

    return SOC_E_NONE;
}

int
soc_easyrider_ext_sram_enable_set(int unit, int mode, int enable, int clr)
{
    uint32 rval, orval;

    switch (mode) {
    case BCM_EXT_SRAM_CSE_MODE:
        /* Enable/disable  LTE - lab test engine  */
        SOC_IF_ERROR_RETURN(READ_CSE_DTU_MODEr(unit, &rval));
        orval = rval;
        soc_reg_field_set(unit, CSE_DTU_MODEr, &rval, DTU_ENf,
                          enable ? 1 : 0);
        soc_reg_field_set(unit, CSE_DTU_MODEr, &rval, SEL_LTEf,
                          enable ? 1 : 0);
        if (rval != orval) {
            SOC_IF_ERROR_RETURN(WRITE_CSE_DTU_MODEr(unit, rval));
        }
        /* Clear status */
        if (clr) {
            rval = 0;
            soc_reg_field_set(unit, CSE_DTU_LTE_TMODE0r, &rval,
                              CLR_STATUSf, 1);
            SOC_IF_ERROR_RETURN(WRITE_CSE_DTU_LTE_TMODE0r(unit, rval));
        }
        break;
    case BCM_EXT_SRAM_HSE_MODE:
        /* Enable/disable  LTE - lab test engine  */
        SOC_IF_ERROR_RETURN(READ_HSE_DTU_MODEr(unit, &rval));
        orval = rval;
        soc_reg_field_set(unit, HSE_DTU_MODEr, &rval, DTU_ENf,
                          enable ? 1 : 0);
        soc_reg_field_set(unit, HSE_DTU_MODEr, &rval, SEL_LTEf,
                          enable ? 1 : 0);
        if (rval != orval) {
            SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_MODEr(unit, rval));
        }
        /* Clear status */
        if (clr) {
            rval = 0;
            soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval,
                              CLR_STATUSf, 1);
            SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_TMODE0r(unit, rval));
        }
        break;
    default:
        return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

int
soc_easyrider_ext_sram_op(int unit, int mode, er_ext_sram_entry_t *entry)
{
    soc_timeout_t to;
    int errors = 0, erbmp = 0;
    int to_usec;
    uint32 orval, rval = 0;

    if (SAL_BOOT_QUICKTURN) {
        to_usec = 60000000;
    } else {
        to_usec = 50000;
    }

    switch (mode) {
    case BCM_EXT_SRAM_CSE_MODE:
        /* Program data registers */
        SOC_IF_ERROR_RETURN(WRITE_CSE_DTU_LTE_D0R_1r(unit,
                                              entry->data0[3] & 0x3ffff));
        SOC_IF_ERROR_RETURN(WRITE_CSE_DTU_LTE_D0R_0r(unit,
                                              entry->data0[2] & 0x3ffff));
        SOC_IF_ERROR_RETURN(WRITE_CSE_DTU_LTE_D0F_1r(unit,
                                              entry->data0[1] & 0x3ffff));
        SOC_IF_ERROR_RETURN(WRITE_CSE_DTU_LTE_D0F_0r(unit,
                                              entry->data0[0] & 0x3ffff));
        if (entry->addr1 != -1) {
            SOC_IF_ERROR_RETURN(WRITE_CSE_DTU_LTE_D1R_1r(unit,
                                              entry->data1[3] & 0x3ffff));
            SOC_IF_ERROR_RETURN(WRITE_CSE_DTU_LTE_D1R_0r(unit,
                                              entry->data1[2] & 0x3ffff));
            SOC_IF_ERROR_RETURN(WRITE_CSE_DTU_LTE_D1F_1r(unit,
                                              entry->data1[1] & 0x3ffff));
            SOC_IF_ERROR_RETURN(WRITE_CSE_DTU_LTE_D1F_0r(unit,
                                              entry->data1[0] & 0x3ffff));
        }

        /* Program address registers */
        rval = 0;
        soc_reg_field_set(unit, CSE_DTU_LTE_ADR0r, &rval,
                          DTU_LTE_ADR0f, (entry->addr0 & 0xfffff));
        SOC_IF_ERROR_RETURN(WRITE_CSE_DTU_LTE_ADR0r(unit, rval));
        if (entry->addr1 != -1) {
            rval = 0;
            soc_reg_field_set(unit, CSE_DTU_LTE_ADR1r, &rval,
                              DTU_LTE_ADR1f, (entry->addr1 & 0xfffff));
            SOC_IF_ERROR_RETURN(WRITE_CSE_DTU_LTE_ADR1r(unit, rval));
        }

        /* Program TMODE1 register */
        rval = 0;
        soc_reg_field_set(unit, CSE_DTU_LTE_TMODE1r, &rval, WDOEBRf,
                          entry->wdoebr);
        soc_reg_field_set(unit, CSE_DTU_LTE_TMODE1r, &rval, WDOEBFf,
                          entry->wdoebf);
        soc_reg_field_set(unit, CSE_DTU_LTE_TMODE1r, &rval, WDMRf,
                          entry->wdmr);
        soc_reg_field_set(unit, CSE_DTU_LTE_TMODE1r, &rval, WDMFf,
                          entry->wdmf);
        soc_reg_field_set(unit, CSE_DTU_LTE_TMODE1r, &rval, RDMRf,
                          entry->rdmr);
        soc_reg_field_set(unit, CSE_DTU_LTE_TMODE1r, &rval, RDMFf,
                          entry->rdmf);
        SOC_IF_ERROR_RETURN(WRITE_CSE_DTU_LTE_TMODE1r(unit, rval));

        /* Program TMODE0 register */
        SOC_IF_ERROR_RETURN(READ_CSE_DTU_LTE_TMODE0r(unit, &rval));
        orval = rval;
        soc_reg_field_set(unit, CSE_DTU_LTE_TMODE0r, &rval, EM_IF_TYPE_QDR2f,
                          entry->em_if_type);
        soc_reg_field_set(unit, CSE_DTU_LTE_TMODE0r, &rval, CMP_EM_RDAT_FRf,
                          entry->em_fall_rise);
        soc_reg_field_set(unit, CSE_DTU_LTE_TMODE0r, &rval, W2R_NOPSf,
                          entry->w2r_nops);
        soc_reg_field_set(unit, CSE_DTU_LTE_TMODE0r, &rval, R2W_NOPSf,
                          entry->r2w_nops);
        soc_reg_field_set(unit, CSE_DTU_LTE_TMODE0r, &rval, EM_LATENCY7f,
                          entry->latency ? 1 : 0);
        soc_reg_field_set(unit, CSE_DTU_LTE_TMODE0r, &rval, ADR_MODEf,
                          (entry->adr_mode & 3));
        soc_reg_field_set(unit, CSE_DTU_LTE_TMODE0r, &rval, LOOP_MODEf,
                          (entry->test_mode & 3));
        soc_reg_field_set(unit, CSE_DTU_LTE_TMODE0r, &rval, CLR_STATUSf, 0);
        if (rval != orval) {
            SOC_IF_ERROR_RETURN(WRITE_CSE_DTU_LTE_TMODE0r(unit, rval));
        }

        /* Kick off the test */
        SOC_IF_ERROR_RETURN(READ_CSE_DTU_LTE_TMODE0r(unit, &rval));
        soc_reg_field_set(unit, CSE_DTU_LTE_TMODE0r, &rval,
                          START_LAB_TESTf, 1);
        SOC_IF_ERROR_RETURN(WRITE_CSE_DTU_LTE_TMODE0r(unit, rval));

        /* Wait until LTE done */
        soc_timeout_init(&to, to_usec, 0);
        for (;;) {
            SOC_IF_ERROR_RETURN(READ_CSE_DTU_LTE_STS_DONEr(unit, &rval));

            if (soc_reg_field_get(unit,
                                  CSE_DTU_LTE_STS_DONEr, rval, DONEf)) {
                break;
            }

            if (soc_timeout_check(&to)) {
                return SOC_E_TIMEOUT;
            }
        }

        errors = soc_reg_field_get(unit, CSE_DTU_LTE_STS_DONEr, rval, ERR_CNTf);
        erbmp = soc_reg_field_get(unit, CSE_DTU_LTE_STS_DONEr, rval, ERR_BITMAPf);

        if ((errors != 0) || (erbmp != 0)) {
            entry->err_cnt = errors;
            entry->err_bmp = erbmp;
            SOC_IF_ERROR_RETURN(
                                READ_CSE_DTU_LTE_STS_ERR_ADRr(unit, &rval));
            entry->err_addr =
                soc_reg_field_get(unit, CSE_DTU_LTE_STS_ERR_ADRr, rval, ERR_ADRf);
            SOC_IF_ERROR_RETURN(
                                READ_CSE_DTU_LTE_STS_ERR_DR_1r(unit, &rval));
            entry->err_data[3] =
                soc_reg_field_get(unit, CSE_DTU_LTE_STS_ERR_DR_1r, rval, ERR_DR_1f);
            SOC_IF_ERROR_RETURN(
                                READ_CSE_DTU_LTE_STS_ERR_DR_0r(unit, &rval));
            entry->err_data[2] =
                soc_reg_field_get(unit, CSE_DTU_LTE_STS_ERR_DR_0r, rval, ERR_DR_0f);
            SOC_IF_ERROR_RETURN(
                                READ_CSE_DTU_LTE_STS_ERR_DF_1r(unit, &rval));
            entry->err_data[1] =
                soc_reg_field_get(unit, CSE_DTU_LTE_STS_ERR_DF_1r, rval, ERR_DF_1f);
            SOC_IF_ERROR_RETURN(
                                READ_CSE_DTU_LTE_STS_ERR_DF_0r(unit, &rval));
            entry->err_data[0] =
                soc_reg_field_get(unit, CSE_DTU_LTE_STS_ERR_DF_0r, rval, ERR_DF_0f);
        } else {
            entry->err_cnt = 0;
            entry->err_bmp = 0;
        }
        break;
    case BCM_EXT_SRAM_HSE_MODE:
        /* Program data registers */
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D0R_3r(unit,
                                              entry->data0[7] & 0x3ffff));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D0R_2r(unit,
                                              entry->data0[6] & 0x3ffff));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D0R_1r(unit,
                                              entry->data0[5] & 0x3ffff));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D0R_0r(unit,
                                              entry->data0[4] & 0x3ffff));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D0F_3r(unit,
                                              entry->data0[3] & 0x3ffff));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D0F_2r(unit,
                                              entry->data0[2] & 0x3ffff));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D0F_1r(unit,
                                              entry->data0[1] & 0x3ffff));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D0F_0r(unit,
                                              entry->data0[0] & 0x3ffff));
        if (entry->addr1 != -1) {
            SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D1R_3r(unit,
                                              entry->data1[7] & 0x3ffff));
            SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D1R_2r(unit,
                                              entry->data1[6] & 0x3ffff));
            SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D1R_1r(unit,
                                              entry->data1[5] & 0x3ffff));
            SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D1R_0r(unit,
                                              entry->data1[4] & 0x3ffff));
            SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D1F_3r(unit,
                                              entry->data1[3] & 0x3ffff));
            SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D1F_2r(unit,
                                              entry->data1[2] & 0x3ffff));
            SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D1F_1r(unit,
                                              entry->data1[1] & 0x3ffff));
            SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_D1F_0r(unit,
                                              entry->data1[0] & 0x3ffff));
        }

        /* Program address registers */
        rval = 0;
        soc_reg_field_set(unit, HSE_DTU_LTE_ADR0r, &rval,
                          DTU_LTE_ADR0f, (entry->addr0 & 0xfffff));
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_ADR0r(unit, rval));
        if (entry->addr1 != -1) {
            rval = 0;
            soc_reg_field_set(unit, HSE_DTU_LTE_ADR1r, &rval,
                              DTU_LTE_ADR1f, (entry->addr1 & 0xfffff));
            SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_ADR1r(unit, rval));
        }

        /* Program TMODE1 register */
        rval = 0;
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE1r, &rval, WDOEBRf,
                          entry->wdoebr);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE1r, &rval, WDOEBFf,
                          entry->wdoebf);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE1r, &rval, WDMRf,
                          entry->wdmr);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE1r, &rval, WDMFf,
                          entry->wdmf);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE1r, &rval, RDMRf,
                          entry->rdmr);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE1r, &rval, RDMFf,
                          entry->rdmf);
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_TMODE1r(unit, rval));

        /* Program TMODE0 register */
        SOC_IF_ERROR_RETURN(READ_HSE_DTU_LTE_TMODE0r(unit, &rval));
        orval = rval;
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, EM_LATENCY7f,
                          entry->latency ? 1 : 0);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, ADR_MODEf,
                          (entry->adr_mode & 3));
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, LOOP_MODEf,
                          (entry->test_mode & 3));
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, W2R_NOPSf,
                          entry->w2r_nops);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, R2W_NOPSf,
                          entry->r2w_nops);
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval, CLR_STATUSf, 0);
        if (rval != orval) {
            SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_TMODE0r(unit, rval));
        }

        /* Kick off the test */
        SOC_IF_ERROR_RETURN(READ_HSE_DTU_LTE_TMODE0r(unit, &rval));
        soc_reg_field_set(unit, HSE_DTU_LTE_TMODE0r, &rval,
                          START_LAB_TESTf, 1);
        SOC_IF_ERROR_RETURN(WRITE_HSE_DTU_LTE_TMODE0r(unit, rval));

        /* Wait until LTE done */
        soc_timeout_init(&to, to_usec, 0);
        for (;;) {
            SOC_IF_ERROR_RETURN(READ_HSE_DTU_LTE_STS_DONEr(unit, &rval));

            if (soc_reg_field_get(unit,
                                  HSE_DTU_LTE_STS_DONEr, rval, DONEf)) {
                break;
            }

            if (soc_timeout_check(&to)) {
                return SOC_E_TIMEOUT;
            }
        }

        errors = soc_reg_field_get(unit, HSE_DTU_LTE_STS_DONEr,
                                   rval, ERR_CNTf);
        erbmp = soc_reg_field_get(unit, HSE_DTU_LTE_STS_DONEr,
                                  rval, ERR_BITMAPf);

        if ((errors != 0) || (erbmp != 0)) {
            entry->err_cnt = errors;
            entry->err_bmp = erbmp;
            SOC_IF_ERROR_RETURN
                (READ_HSE_DTU_LTE_STS_ERR_ADRr(unit, &rval));
            entry->err_addr =
                soc_reg_field_get(unit, HSE_DTU_LTE_STS_ERR_ADRr,
                                  rval, ERR_ADRf);
            SOC_IF_ERROR_RETURN
                (READ_HSE_DTU_LTE_STS_ERR_DR_3r(unit, &rval));
            entry->err_data[7] =
                soc_reg_field_get(unit, HSE_DTU_LTE_STS_ERR_DR_3r,
                                  rval, ERR_DR_3f);
            SOC_IF_ERROR_RETURN
                (READ_HSE_DTU_LTE_STS_ERR_DR_2r(unit, &rval));
            entry->err_data[6] =
                soc_reg_field_get(unit, HSE_DTU_LTE_STS_ERR_DR_2r,
                                  rval, ERR_DR_2f);
            SOC_IF_ERROR_RETURN
                (READ_HSE_DTU_LTE_STS_ERR_DR_1r(unit, &rval));
            entry->err_data[5] =
                soc_reg_field_get(unit, HSE_DTU_LTE_STS_ERR_DR_1r,
                                  rval, ERR_DR_1f);
            SOC_IF_ERROR_RETURN
                (READ_HSE_DTU_LTE_STS_ERR_DR_0r(unit, &rval));
            entry->err_data[4] =
                soc_reg_field_get(unit, HSE_DTU_LTE_STS_ERR_DR_0r,
                                  rval, ERR_DR_0f);
            SOC_IF_ERROR_RETURN
                (READ_HSE_DTU_LTE_STS_ERR_DF_3r(unit, &rval));
            entry->err_data[3] =
                soc_reg_field_get(unit, HSE_DTU_LTE_STS_ERR_DF_3r,
                                  rval, ERR_DF_3f);
            SOC_IF_ERROR_RETURN
                (READ_HSE_DTU_LTE_STS_ERR_DF_2r(unit, &rval));
            entry->err_data[2] =
                soc_reg_field_get(unit, HSE_DTU_LTE_STS_ERR_DF_2r,
                                  rval, ERR_DF_2f);
            SOC_IF_ERROR_RETURN
                (READ_HSE_DTU_LTE_STS_ERR_DF_1r(unit, &rval));
            entry->err_data[1] =
                soc_reg_field_get(unit, HSE_DTU_LTE_STS_ERR_DF_1r,
                                  rval, ERR_DF_1f);
            SOC_IF_ERROR_RETURN
                (READ_HSE_DTU_LTE_STS_ERR_DF_0r(unit, &rval));
            entry->err_data[0] =
                soc_reg_field_get(unit, HSE_DTU_LTE_STS_ERR_DF_0r,
                                  rval, ERR_DF_0f);
        } else {
            entry->err_cnt = 0;
            entry->err_bmp = 0;
        }
        break;
    default:
        return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

/* Parity error handlers */

int
_soc_er_parity_error(int unit)
{
    uint32	regval, temp_val, paraddr;
    static int  num_ext_l2_errors=0;

    /* SEER parity registers clear on read */
    SOC_IF_ERROR_RETURN(READ_PAR_ERR_STATUS_BSEr(unit, &regval));
    if (regval) {
        if (regval & SOC_ER_PARITY_BSE_L3_DEFIP_ALG) {
            SOC_IF_ERROR_RETURN(READ_PAR_ADR_L3_DEFIP_ALGr(unit, &paraddr));
            soc_cm_debug(DK_ERR,
                         "unit %d memory %s[%d]: parity error\n",
                         unit, SOC_MEM_NAME(unit, L3_DEFIP_ALGm), paraddr);
        }

        if (regval & SOC_ER_PARITY_BSE_L3_DEFIP_ALG_EXT) {
            SOC_IF_ERROR_RETURN
                (READ_PAR_ADR_L3_DEFIP_ALG_EXTr(unit, &paraddr));
            soc_cm_debug(DK_ERR,
                         "unit %d external memory %s[%d]: parity error\n",
                         unit, SOC_MEM_NAME(unit, L3_DEFIP_ALGm), paraddr);
        }

        if (regval & SOC_ER_PARITY_BSE_VRF_VFI_INTF) {
            SOC_IF_ERROR_RETURN(READ_PAR_ADR_VRF_VFI_INTFr(unit, &paraddr));
            soc_cm_debug(DK_ERR,
                         "unit %d memory %s[%d]: parity error\n",
                         unit, SOC_MEM_NAME(unit, VRF_VFI_INTFm), paraddr);
        }

        if (regval & SOC_ER_PARITY_BSE_IGR_VLAN_XLATE) {
            SOC_IF_ERROR_RETURN(READ_PAR_ADR_IGR_VLAN_XLATEr(unit, &paraddr));
            soc_cm_debug(DK_ERR,
                         "unit %d memory %s[%d]: parity error\n",
                         unit, SOC_MEM_NAME(unit, IGR_VLAN_XLATEm), paraddr);
        }

        if (regval & SOC_ER_PARITY_BSE_EGR_VLAN_XLATE) {
            SOC_IF_ERROR_RETURN(READ_PAR_ADR_EGR_VLAN_XLATEr(unit, &paraddr));
            soc_cm_debug(DK_ERR,
                         "unit %d memory %s[%d]: parity error\n",
                         unit, SOC_MEM_NAME(unit, EGR_VLAN_XLATEm), paraddr);
        }
    }

    SOC_IF_ERROR_RETURN(READ_PAR_ERR_STATUS_CSEr(unit, &regval));
    if (regval) {
        if (regval & SOC_ER_PARITY_CSE_NEXT_HOP_INT) {
            SOC_IF_ERROR_RETURN(READ_PAR_ADR_NEXT_HOP_INTr(unit, &paraddr));
            soc_cm_debug(DK_ERR,
                         "unit %d memory %s[%d]: parity error\n",
                         unit, SOC_MEM_NAME(unit, NEXT_HOP_INTm), paraddr);
        }

        if (regval & SOC_ER_PARITY_CSE_NEXT_HOP_EXT) {
            SOC_IF_ERROR_RETURN(READ_PAR_ADR_NEXT_HOP_EXTr(unit, &paraddr));
            soc_cm_debug(DK_ERR,
                         "unit %d memory %s[%d]: parity error\n",
                         unit, SOC_MEM_NAME(unit, NEXT_HOP_EXTm), paraddr);
        }

        if (regval & SOC_ER_PARITY_CSE_L3_INTF_TABLE) {
            SOC_IF_ERROR_RETURN(READ_PAR_ADR_L3_INTF_TABLEr(unit, &paraddr));
            soc_cm_debug(DK_ERR,
                         "unit %d memory %s[%d]: parity error\n",
                         unit, SOC_MEM_NAME(unit, L3INTFm), paraddr);
        }

        if (regval & SOC_ER_PARITY_CSE_FP_EXTERNAL_TCAM) {
            if (SOC_PERSIST(unit)->er_memcfg.tcam_select ==
                ER_EXT_TCAM_TYPE2) {
                SOC_IF_ERROR_RETURN(soc_er_tcam_type2_parity_diagnose(unit));
            } else {
                soc_cm_debug(DK_ERR,
                             "unit %d memory %s: parity error\n",
                             unit, SOC_MEM_NAME(unit, FP_TCAM_EXTERNALm));
            }
        }

        if (regval & SOC_ER_PARITY_CSE_FP_POLICY_TABLE_EXT) {
            soc_cm_debug(DK_ERR,
                         "unit %d memory %s: parity error\n",
                         unit, SOC_MEM_NAME(unit, FP_POLICY_EXTERNALm));
        }
 
        if (regval & SOC_ER_PARITY_CSE_FP_METER_TABLE_EXT) {
            soc_cm_debug(DK_ERR,
                         "unit %d memory %s: parity error\n",
                         unit, SOC_MEM_NAME(unit, FP_METER_TABLE_EXTm));
        }
 
        if (regval & SOC_ER_PARITY_CSE_FP_COUNTER_EXT) {
            soc_cm_debug(DK_ERR,
                         "unit %d memory %s: parity error\n",
                         unit, SOC_MEM_NAME(unit, FP_COUNTER_EXTm));
        }
 
        if (regval & SOC_ER_PARITY_CSE_L3_DEFIP_DATA) {
            soc_cm_debug(DK_ERR,
                         "unit %d memory %s: parity error\n",
                         unit, SOC_MEM_NAME(unit, L3_DEFIP_DATAm));
        }
    }

    SOC_IF_ERROR_RETURN(READ_PAR_ERR_STATUS_HSEr(unit, &regval));
    if (regval) {
        if (regval & SOC_ER_PARITY_HSE_L2_ENTRY) {
            SOC_IF_ERROR_RETURN(READ_PAR_ADR_L2_ENTRYr(unit, &paraddr));
            soc_cm_debug(DK_ERR,
                         "unit %d memory %s[%d]: parity error\n",
                         unit, SOC_MEM_NAME(unit, L2_ENTRY_INTERNALm),
                         paraddr);
        }

        if (regval & SOC_ER_PARITY_HSE_L2_ENTRY_EXT) {
            SOC_IF_ERROR_RETURN(READ_PAR_ADR_L2_ENTRY_EXTr(unit, &paraddr));
            if (++num_ext_l2_errors < 50) {
                soc_cm_debug(DK_ERR,
                             "unit %d memory %s[%d]: parity error\n",
                             unit, SOC_MEM_NAME(unit, L2_ENTRY_EXTERNALm),
                             paraddr);
            } else if (num_ext_l2_errors == 50) {
                soc_cm_debug(DK_ERR,
                             "unit %d memory %s[%d]: Too many parity errors, turning off.\n",
                             unit, SOC_MEM_NAME(unit, L2_ENTRY_EXTERNALm),
                             paraddr);

                /* Turn off L2_ENTRY_EXTERNAL parity errors */
                SOC_IF_ERROR_RETURN(READ_PAR_ERR_MASK_HSEr(unit, &temp_val));
                temp_val |= SOC_ER_PARITY_HSE_L2_ENTRY_EXT;
                SOC_IF_ERROR_RETURN(WRITE_PAR_ERR_MASK_HSEr(unit, temp_val));
            }
        }

        if (regval & SOC_ER_PARITY_HSE_L3_ENTRY_V4) {
            SOC_IF_ERROR_RETURN(READ_PAR_ADR_L3_ENTRY_V4r(unit, &paraddr));
            paraddr -= SOC_PERSIST(unit)->er_memcfg.l3v4_search_offset;
            soc_cm_debug(DK_ERR,
                         "unit %d memory %s[%d]: parity error\n",
                         unit, SOC_MEM_NAME(unit, L3_ENTRY_V4m), paraddr);
        }

        if (regval & SOC_ER_PARITY_HSE_L3_ENTRY_V6) {
            SOC_IF_ERROR_RETURN(READ_PAR_ADR_L3_ENTRY_V6r(unit, &paraddr));
            paraddr -= SOC_PERSIST(unit)->er_memcfg.l3v6_search_offset;
            soc_cm_debug(DK_ERR,
                         "unit %d memory %s[%d]: parity error\n",
                         unit, SOC_MEM_NAME(unit, L3_ENTRY_V6m), paraddr);
        }

        if (regval & SOC_ER_PARITY_HSE_IPMC_GROUP_V4) {
            SOC_IF_ERROR_RETURN(READ_PAR_ADR_IPMC_GROUP_V4r(unit, &paraddr));
            soc_cm_debug(DK_ERR,
                         "unit %d memory %s[%d]: parity error\n",
                         unit, SOC_MEM_NAME(unit, IPMC_GROUP_V4m), paraddr);
        }

        if (regval & SOC_ER_PARITY_HSE_IPMC_GROUP_V6) {
            SOC_IF_ERROR_RETURN(READ_PAR_ADR_IPMC_GROUP_V6r(unit, &paraddr));
            soc_cm_debug(DK_ERR,
                         "unit %d memory %s[%d]: parity error\n",
                         unit, SOC_MEM_NAME(unit, IPMC_GROUP_V6m), paraddr);
        }
    }

    /* Egress parity not supported in HW */

    SOC_IF_ERROR_RETURN(READ_MMU_ERR_VECTORr(unit, &regval));
    if (regval) {
        if (soc_feature(unit, soc_feature_mcu_fifo_suppress)) {
            /* Mask out REQ FIFO error */
            regval &= ~SOC_ER_PARITY_MMU_MCU_REQ_FIFO_ERR;
        }

        if (regval & SOC_ER_PARITY_MMU_ASM_PKTID_ERR) {
            soc_cm_debug(DK_ERR,
                         "unit %d fatal MMU ASM pktid error\n", unit);
        }

        if (regval & SOC_ER_PARITY_MMU_ASM_PARITY_ERR) {
            soc_cm_debug(DK_ERR,
                         "unit %d MMU ASM memory parity error\n", unit);
        }

        if (regval & SOC_ER_PARITY_MMU_XQ_PARITY_ERR) {
            soc_cm_debug(DK_ERR,
                         "unit %d fatal MMU XQ memory parity error\n", unit);
        }

        if (regval & SOC_ER_PARITY_MMU_MSYS_PTR_RELEASE_MGR_ERR) {
            soc_cm_debug(DK_ERR,
                "unit %d MMU MSYS pointer release manager parity error\n",
                         unit);
        }

        if (regval & SOC_ER_PARITY_MMU_MSYS_PTR_CTRL0_ERR) {
            soc_cm_debug(DK_ERR,
                "unit %d MMU MSYS pointer control 0 parity error\n", unit);
        }

        if (regval & SOC_ER_PARITY_MMU_MSYS_PTR_CTRL1_ERR) {
            soc_cm_debug(DK_ERR,
                "unit %d MMU MSYS pointer control 1 parity error\n", unit);
        }

        if (regval & SOC_ER_PARITY_MMU_MSYS_COPYCNT_PTR_ERR) {
            soc_cm_debug(DK_ERR,
                "unit %d MMU MSYS copycount count parity error\n", unit);
        }

        if (regval & SOC_ER_PARITY_MMU_MSYS_COPYCNT_COUNT_ERR) {
            soc_cm_debug(DK_ERR,
                         "unit %d MMU copycount pointer parity error\n", unit);
        }

        if (regval & SOC_ER_PARITY_MMU_MSYS_CELL_0_ERR) {
            soc_cm_debug(DK_ERR,
                         "unit %d MMU channel 0 cell CRC error\n", unit);
        }

        if (regval & SOC_ER_PARITY_MMU_MSYS_PKT_0_ERR) {
            soc_cm_debug(DK_ERR,
                         "unit %d MMU channel 0 packet CRC error\n", unit);
        }

        if (regval & SOC_ER_PARITY_MMU_MSYS_NXTPTR_0_ERR) {
            soc_cm_debug(DK_ERR,
                         "unit %d MMU channel 0 next pointer CRC error\n",
                         unit);
        }

        if (regval & SOC_ER_PARITY_MMU_MSYS_PTR_BLOCK_0_ERR) {
            soc_cm_debug(DK_ERR,
                         "unit %d MMU channel 0 pointer block CRC error\n",
                         unit);
        }

        if (regval & SOC_ER_PARITY_MMU_MSYS_CELL_1_ERR) {
            soc_cm_debug(DK_ERR,
                         "unit %d MMU channel 1 cell CRC error\n", unit);
        }

        if (regval & SOC_ER_PARITY_MMU_MSYS_PKT_1_ERR) {
            soc_cm_debug(DK_ERR,
                         "unit %d MMU channel 1 packet CRC error\n", unit);
        }

        if (regval & SOC_ER_PARITY_MMU_MSYS_NXTPTR_1_ERR) {
            soc_cm_debug(DK_ERR,
                         "unit %d MMU channel 1 next pointer CRC error\n",
                         unit);
        }

        if (regval & SOC_ER_PARITY_MMU_MSYS_PTR_BLOCK_1_ERR) {
            soc_cm_debug(DK_ERR,
                         "unit %d MMU channel 1 pointer block CRC error\n",
                         unit);
        }

        if (regval & SOC_ER_PARITY_MMU_MSYS_INGBUF_OVERFLOW_ERR) {
            soc_cm_debug(DK_ERR,
                         "unit %d MMU ingress buffer overflow\n", unit);
        }

        if (regval & SOC_ER_PARITY_MMU_MSYS_INGBUF_CELL_INCONSISTENCY_ERR) {
            soc_cm_debug(DK_ERR,
                "unit %d MMU MSYS ingress buffer cell inconsistency\n", unit);
        }

        if (regval & SOC_ER_PARITY_MMU_MSYS_IPMC_PTR_ERR) {
            soc_cm_debug(DK_ERR,
                "unit %d MMU IPMC pointer table parity error\n", unit);
        }

        if (regval & SOC_ER_PARITY_MMU_MSYS_IPMC_IF_NO_ERR) {
            soc_cm_debug(DK_ERR,
                         "unit %d MMU IPMC parity error if no table\n", unit);
        }

        if (regval & SOC_ER_PARITY_MMU_MSYS_ING_STAT_ERR) {
            soc_cm_debug(DK_ERR,
                "unit %d MMU ingress statistics parity error\n", unit);
        }

        if (regval & SOC_ER_PARITY_MMU_MCU_REQ_FIFO_ERR) {
            soc_cm_debug(DK_ERR,
                         "unit %d MMU fatal MCU FIFO error\n", unit);
        }

        if (regval & SOC_ER_PARITY_MMU_ASM_FIFO_ERR) {
            soc_cm_debug(DK_ERR,
                         "unit %d MMU fatal ASM FIFO error\n", unit);
        }

        if (regval & SOC_ER_PARITY_MMU_ASM_IGMU_ERR) {
            soc_cm_debug(DK_ERR,
                         "unit %d MMU possibly fatal ASM IGMU error\n", unit);
        }

        /* Clear MMU parity status */
        regval = soc_feature(unit, soc_feature_mcu_fifo_suppress) ?
                  SOC_ER_PARITY_MMU_MCU_REQ_FIFO_ERR : 0; /* Mask out this error */
        SOC_IF_ERROR_RETURN(WRITE_MMU_ERR_VECTORr(unit, regval));
    }

    return SOC_E_NONE;
}

void
soc_er_parity_error(void *p_unit, void *d1, void *d2,
                    void *d3, void *d4 )
{
    int         unit = PTR_TO_INT(p_unit);

    COMPILER_REFERENCE(d1);
    COMPILER_REFERENCE(d2);
    COMPILER_REFERENCE(d3);
    COMPILER_REFERENCE(d4);

    _soc_er_parity_error(unit);
    soc_intr_enable(unit, IRQ_MEM_FAIL);
}


/*
 * Function:
 *      soc_mem_er_search
 * Purpose:
 *      Search an table for a matching entry
 * Parameters:
 *      unit - Strata-XGS unit number
 *      copyno - Actual copy number, can't be "ANY" or "ALL"
 *      index_ptr - (OUT) Gets index of entry in table if found
 *      key_data - Data to search for
 *      entry_data - (OUT) Gets data if found; May be NULL.
 *      lowest_match - For sorted tables only.
 *                     If there are duplicate entries in the
 *                     table, and lowest_match is 1, then the
 *                     matching entry at the lowest index is
 *                     returned.  If lowest_match is 0, then any
 *                     of the matching entries may be picked at
 *                     random, which can be faster.
 * Returns:
 *      SOC_E_XXX;
 */

int
soc_mem_er_search(int unit, soc_mem_t mem, int copyno,
                  int *index_ptr, void *key_data, void *entry_data,
                  int lowest_match)
{
    soc_mem_cmd_t cmd_info;
    uint32        hash_addr;
    int           rv;

    sal_memset(&cmd_info, 0, sizeof(cmd_info));
    switch (mem) {
    case L3_DEFIP_CAMm:
        cmd_info.command = SOC_MEM_CMD_SEARCH_72;
        break;
    default:
        cmd_info.command = SOC_MEM_CMD_SEARCH;
        break;
    }
    cmd_info.entry_data = key_data;
    rv = soc_mem_cmd_op(unit, mem, &cmd_info, FALSE);
    sal_memcpy(entry_data, cmd_info.output_data,
               soc_mem_entry_words(unit, mem) * sizeof (uint32));
    hash_addr = cmd_info.addr0;
    if (soc_mem_is_hashed(unit, mem)) {
        hash_addr &= SOC_MEM_CMD_HASH_ADDR_MASK;
        /* L2/L3 hashes done elsewhere */
     }
    *index_ptr = hash_addr;
    if (rv == SOC_E_NOT_FOUND) {
        /* Debug for emulation */
        soc_pci_analyzer_trigger(unit);
        *index_ptr = -1;
    }

    return rv;
}

STATIC void
_soc_er_defip_alg_to_tcam(int unit, uint32 *alg_data, uint32 *tcam_data,
                          int mask_len)
{
    uint32     field_buf[SOC_MAX_MEM_FIELD_WORDS];
    uint32     prefix, key_type, val;
    uint32     ip_mask[2];
    int        type, subtype;

    sal_memset(tcam_data, 0, sizeof(uint32) *
               BYTES2WORDS(SOC_MEM_INFO(unit, L3_DEFIP_CAMm).bytes));

    soc_mem_field_get(unit, L3_DEFIP_ALGm, alg_data, IP_ADDR_V6f, field_buf);
    soc_mem_field_set(unit, L3_DEFIP_CAMm, tcam_data, IP_ADDR_V6f, field_buf);

    soc_mem_field_get(unit, L3_DEFIP_ALGm, alg_data, DATA_AGRf, field_buf);
    soc_mem_field_set(unit, L3_DEFIP_CAMm, tcam_data, DATA_AGRf, field_buf);

    /* Construct mask in field_buf */
    sal_memset(field_buf, 0, sizeof(field_buf));

    soc_mem_field32_set(unit, L3_DEFIP_CAMm, tcam_data, KEY_VALIDf, 1);
    soc_mem_field32_set(unit, L3_DEFIP_CAMm, field_buf, KEY_VALIDf, 1);
    soc_mem_field32_set(unit, L3_DEFIP_CAMm, field_buf, KEY_V6f, 1);

    if (soc_mem_field32_get(unit, L3_DEFIP_ALGm, alg_data, FFf) == 0xff) {
        key_type = soc_mem_field32_get(unit, L3_DEFIP_ALGm,
                                       alg_data, KEY_TYPEf);
    } else {
        /* IPv6 */
        key_type = 0;
        soc_mem_field32_set(unit, L3_DEFIP_CAMm, tcam_data, KEY_V6f, 1);
    }

    prefix = soc_mem_field32_get(unit, L3_DEFIP_ALGm, alg_data,
                                 PREFIX_LENGTHf);
    if (prefix == 0) {
        if (key_type == SOC_ER_DEFIP_KEY_TYPE_V4) {
            val = 32;
        } else {
            val = 0;
        }
    } else {
        if (key_type > SOC_ER_DEFIP_KEY_TYPE_V4) {
            /* MPLS, need a uniform value, 0 is simplest */
            val = 0;
        } else {
            if ((key_type == SOC_ER_DEFIP_KEY_TYPE_V4) && (prefix > 32)) {
                prefix = 32;
            }
            val = prefix - 1;
        }
    }
    soc_mem_field32_set(unit, L3_DEFIP_CAMm, tcam_data, PREFIX_LENGTHf, val);
    if (prefix || (key_type != 0)) {
        soc_mem_field32_set(unit, L3_DEFIP_CAMm, field_buf,
                            PREFIX_LENGTHf, 0x3f);
    }

    /* force the mask length */
    if (mask_len >= 0 && mask_len <= 64) {
        prefix = mask_len;
    }
    /* Finally, the IP mask */
    if (key_type == SOC_ER_DEFIP_KEY_TYPE_V4) {
        ip_mask[1] = 0xffffffff;
        ip_mask[0] = (prefix == 0) ? 0 : (0xffffffff << (32 - prefix));
    } else if (key_type == 0){
        if (prefix < 32) {
            ip_mask[1] = (prefix == 0) ? 0 : (0xffffffff << (32 - prefix));
            ip_mask[0] = 0;
        } else {
            ip_mask[1] = 0xffffffff;
            ip_mask[0] = (prefix == 32) ? 0 : (0xffffffff << (64 - prefix));
        }
    } else {
        /* MPLS, match all data*/
        ip_mask[1] = 0xffffffff;
        ip_mask[0] = 0xffffffff;
    }

    /* Polarity flip? */
    if (SOC_PERSIST(unit)->er_memcfg.tcam_select == ER_EXT_TCAM_TYPE1) {
        field_buf[0] ^= 0xffffffff;
        ip_mask[1] ^= 0xffffffff;
        ip_mask[0] ^= 0xffffffff;
    }

    soc_mem_field_set(unit, L3_DEFIP_CAMm, field_buf, IP_ADDR_V6f, ip_mask);

    soc_tcam_get_info(unit, &type, &subtype, NULL, NULL);
    if (type == ER_EXT_TCAM_TYPE1 && subtype == 2) {
        val = tcam_data[0];
        tcam_data[0] = val & ~field_buf[0];
        field_buf[0] = ~val & ~field_buf[0];
        val = tcam_data[1];
        tcam_data[1] = val & ~field_buf[1];
        field_buf[1] = ~val & ~field_buf[1];
        val = tcam_data[2];
        tcam_data[2] = (val & ~field_buf[2]) & 0xff;
        field_buf[2] = (~val & ~field_buf[2]) & 0xff;
    }

    soc_mem_field_set(unit, L3_DEFIP_CAMm, tcam_data, MASKf, field_buf);
}

STATIC void
_soc_er_defip_tcam_to_alg(int unit, uint32 *tcam_data, uint32 *alg_data)
{
    uint32              field_buf[SOC_MAX_MEM_FIELD_WORDS];

    /* 
     * As long as we use the calls below, then the ALG form data
     * is stored in the TCAM form data.
     */
    sal_memset(alg_data, 0, sizeof(uint32) *
               BYTES2WORDS(SOC_MEM_INFO(unit, L3_DEFIP_ALGm).bytes));

    soc_mem_field_get(unit, L3_DEFIP_CAMm, tcam_data, IP_ADDR_V6f, field_buf);
    soc_mem_field_set(unit, L3_DEFIP_ALGm, alg_data, IP_ADDR_V6f, field_buf);

    soc_mem_field_get(unit, L3_DEFIP_CAMm, tcam_data, DATA_AGRf, field_buf);
    soc_mem_field_set(unit, L3_DEFIP_ALGm, alg_data, DATA_AGRf, field_buf);
}

int
_soc_er_defip_insert(int unit, defip_alg_entry_t *entry_data, int mask_len)
{
    defip_cam_entry_t tcam_data;
    soc_mem_cmd_t     cmd_info;
    soc_mem_t         defip_mem;
    int               rv;
    int               mpls_entry, change_boundary = 0;
    uint32            mpls_boundary, ipv4_boundary;
    uint32            key_type, rval, tcam_index;

    sal_memset(&cmd_info, 0, sizeof(cmd_info));
    cmd_info.command = SOC_MEM_CMD_LEARN;

    if (soc_mem_index_count(unit, L3_DEFIP_CAMm) > 0) {
        _soc_er_defip_alg_to_tcam(unit, (uint32 *)entry_data,
                                  (uint32 *)&tcam_data, mask_len);
        defip_mem = L3_DEFIP_CAMm;
        cmd_info.entry_data = &tcam_data;

        if (soc_feature(unit, soc_feature_l3defip_bound_adj)) {
            mpls_entry = 0;
            if (soc_mem_field32_get(unit, L3_DEFIP_CAMm,
                                    &tcam_data, FFf) == 0xff) {

                key_type = soc_mem_field32_get(unit, L3_DEFIP_CAMm,
                                               &tcam_data, KEY_TYPEf);
                if ((key_type == SOC_ER_DEFIP_KEY_TYPE_MPLS_1L) ||
                    (key_type == SOC_ER_DEFIP_KEY_TYPE_MPLS_2L)) {

                    mpls_entry = 1;
                }
            }

            if (!mpls_entry) {
                SOC_IF_ERROR_RETURN(READ_BOUNDARY_V4_PREFIXr(unit, 0, &rval));
                ipv4_boundary = soc_reg_field_get(unit, BOUNDARY_V4_PREFIXr,
                                                  rval, V4_BOUNDARYf);
                SOC_IF_ERROR_RETURN(READ_BOUNDARY_MPLSr(unit, &rval));
                mpls_boundary = soc_reg_field_get(unit, BOUNDARY_MPLSr,
                                                  rval, BOUNDARY_MPLS_BITSf);

                if (mpls_boundary > ipv4_boundary) {
                    change_boundary = 1;
                    soc_reg_field_set(unit, BOUNDARY_MPLSr, &rval, 
                                      BOUNDARY_MPLS_BITSf, mpls_boundary - 1);
                    SOC_IF_ERROR_RETURN(WRITE_BOUNDARY_MPLSr(unit, rval));
                }
            }
        }
    } else {
        defip_mem = L3_DEFIP_ALGm;
        cmd_info.entry_data = entry_data;
    }

    rv = soc_mem_cmd_op(unit, defip_mem, &cmd_info, TRUE);

    if (change_boundary) {
        SOC_IF_ERROR_RETURN(READ_BOUNDARY_MPLSr(unit, &rval));
        mpls_boundary = soc_reg_field_get(unit, BOUNDARY_MPLSr,
                                          rval, BOUNDARY_MPLS_BITSf);

        soc_reg_field_set(unit, BOUNDARY_MPLSr, &rval, 
                          BOUNDARY_MPLS_BITSf, mpls_boundary + 1);
        SOC_IF_ERROR_RETURN(WRITE_BOUNDARY_MPLSr(unit, rval));
    }

     
    /* MPLS insertion cleanup: Write one blank entry after insert */
    if ((soc_mem_index_count(unit, L3_DEFIP_CAMm) > 0) &&
        (SOC_PERSIST(unit)->er_memcfg.tcam_select == ER_EXT_TCAM_TYPE1)) {
        if (soc_mem_field32_get(unit, L3_DEFIP_CAMm,
                                &tcam_data, FFf) == 0xff) {

            key_type = soc_mem_field32_get(unit, L3_DEFIP_CAMm,
                                       &tcam_data, KEY_TYPEf);
            if ((key_type == SOC_ER_DEFIP_KEY_TYPE_MPLS_1L) ||
                (key_type == SOC_ER_DEFIP_KEY_TYPE_MPLS_2L)) {
                SOC_IF_ERROR_RETURN(READ_BOUNDARY_MPLSr(unit, &rval));
                tcam_index = soc_reg_field_get(unit, BOUNDARY_MPLSr,
                                               rval, BOUNDARY_MPLS_BITSf);
                /*
                 * The value in BOUNDARY_MPLS is the end of the table +1,
                 * the exact location we want to blank.
                 */
                SOC_IF_ERROR_RETURN
                    (soc_mem_write(unit, defip_mem, COPYNO_ALL, tcam_index,
                                   soc_mem_entry_null(unit, defip_mem)));
            }
        }
    }    

    return rv;
}

int
_soc_er_defip_delete(int unit, defip_alg_entry_t *entry_data, int mask_len)
{
    defip_cam_entry_t tcam_data;
    soc_mem_cmd_t     cmd_info;
    soc_mem_t         defip_mem;
    int               rv;

    sal_memset(&cmd_info, 0, sizeof(cmd_info));
    cmd_info.command = SOC_MEM_CMD_DELETE;

    if (soc_mem_index_count(unit, L3_DEFIP_CAMm) > 0) {
        _soc_er_defip_alg_to_tcam(unit, (uint32 *)entry_data,
                                  (uint32 *)&tcam_data, mask_len);
        defip_mem = L3_DEFIP_CAMm;
        cmd_info.entry_data = &tcam_data;
    } else {
        defip_mem = L3_DEFIP_ALGm;
        cmd_info.entry_data = entry_data;
    }

    rv = soc_mem_cmd_op(unit, defip_mem, &cmd_info, TRUE);

    if (rv == SOC_E_NOT_FOUND) {
        rv = SOC_E_NONE;
    }

    return rv;
}

/*
 * Easyrider L3_DEFIP_ALG tables - a discussion
 *
 * The "L3_DEFIP_ALG" table in Easyrider is a single SBUS addressed table
 * which has two possbile physical interpretations on the chip:
 *
 * 1) An internal table of 16K raw entries
 * 2) An external table of 512K raw entries
 *
 * In either case, the usage of the table is quite complex in the physical
 * implementation.  The original intention was fore command memory access
 * to shield SW from the implementation details, but the need for
 * iteration over valid entries and prefix_length-sensitive searching leads
 * to SW interaction with the detailed table structure.
 *
 * The details of BSETA (Binary Search on Expanded Tree Algorithm) are
 * beyond this dicusssion.  The necessary points to understand are:
 *
 * 1) Each IP route (v4 or v6) requires two raw entries in the table.
 * 2) Packet searches are not aware of the prefix length. They are only
 *    compared to "SEARCH" entries, not all entries in the table.
 * 3) During the insert into the table, the raw entries may be marked
 *    "DUPLICATE" and located in a different portion of the table from the
 *    SEARCH entries.
 *
 * The table is organized into subsections of equal size.  Each subsection
 * may be allocated for SEARCH, DUPLICATE, or unused.  I have called
 * these subsections "regions" in the code below.  SEARCH regions grow from
 * the region with the least table index to regions with greater table
 * indices.  DUPLICATE regions grow from the region with the greatest table
 * index to regions with lesser indices.
 *
 * To add to the challenge, the internal table variant has 8 regions of
 * 2K entries each.  The external table has 16 regions of 32K each.
 *
 * The state of the regions is describe by the LPM_ENTRY_* registers:
 *
 * LPM_ENTRY_VALID is a bitmap of all used regions.
 * LPM_ENTRY_DUP is a bitmap of DUPLICATE regions.
 * LPM_ENTRY_SRCH_AVAIL is the number of SEARCH entries in the most
 *      recently allocated SEARCH region.
 * LPM_ENTRY_DUP_AVAIL is the number of DUPLICATE entries in the most
 *      recently allocated DUPLICATE region.
 *
 * The bitmap of SEARCH regions is calculated below by XOR of the valid and
 * duplicate bitmaps.
 *
 * The parameters structure below is designed to latch the table state
 * at the beginning of an operation so that it does not need to be re-read
 * throughout the operation.  This leads to the requirement that the table
 * lock be held during the operation, noted by the iterate function below.
 *
 */


typedef struct soc_er_defip_alg_parm_s {
    uint32 valid_bmp;
    uint32 dup_bmp;
    uint32 search_remain;
    uint32 last_search_region;
    uint32 last_search_entry;
    uint32 dup_remain;
    uint32 first_dup_region;
    uint32 last_fr_dup_entry;
    uint32 region_size;
    uint32 region_num;
} soc_er_defip_alg_parm_t;

STATIC int
_soc_er_defip_alg_params_get(int unit, soc_er_defip_alg_parm_t *alg_parms)
{
    uint32 region_size, search_bmp;
    int reg_bit;

    SOC_IF_ERROR_RETURN
        (READ_LPM_ENTRY_VLDr(unit, &(alg_parms->valid_bmp)));
    SOC_IF_ERROR_RETURN
        (READ_LPM_ENTRY_DUPr(unit, &(alg_parms->dup_bmp)));
    SOC_IF_ERROR_RETURN
        (READ_LPM_ENTRY_SRCH_AVAILr(unit, &(alg_parms->search_remain)));
    SOC_IF_ERROR_RETURN
        (READ_LPM_ENTRY_DUP_AVAILr(unit, &(alg_parms->dup_remain)));

    /* Our dups are subset of valid, unlike HW */
    alg_parms->dup_bmp &= alg_parms->valid_bmp;
    search_bmp = alg_parms->valid_bmp ^ alg_parms->dup_bmp;

    alg_parms->region_num = ((SOC_PERSIST(unit)->er_memcfg.ext_table_cfg) ==
                             ER_SEER_CFG_LPM_256_EXT) ?
        SOC_ER_DEFIP_ALG_EXT_REGIONS :
        SOC_ER_DEFIP_ALG_INT_REGIONS;

    region_size = soc_mem_index_count(unit, L3_DEFIP_ALGm) /
        alg_parms->region_num;

    for (reg_bit = (alg_parms->region_num - 1);
         reg_bit >= 0; reg_bit--) {
        if ((search_bmp & (1 << reg_bit)) != 0) {
            alg_parms->last_search_region = reg_bit;
            break;
        }
    }
    if (reg_bit < 0) {
        return SOC_E_INTERNAL; /* HW out of sync */
    }

    for (reg_bit = 0; reg_bit < alg_parms->region_num; reg_bit++) {
        if ((alg_parms->dup_bmp & (1 << reg_bit)) != 0) {
            alg_parms->first_dup_region = reg_bit;
            break;
        }
    }
    if (reg_bit >= alg_parms->region_num) {
        return SOC_E_INTERNAL; /* HW out of sync */
    }

    alg_parms->region_size = region_size;
    alg_parms->last_search_entry =
        (region_size * (alg_parms->last_search_region + 1)) -
        alg_parms->search_remain - 1;
    if (alg_parms->dup_remain >= region_size) {
        alg_parms->dup_remain -= region_size;
        alg_parms->first_dup_region++;        
    }
    if (alg_parms->first_dup_region == alg_parms->region_num) {
        alg_parms->last_fr_dup_entry =
            (region_size * (alg_parms->region_num - 1)) - 1;
        alg_parms->first_dup_region--;
    } else {
        alg_parms->last_fr_dup_entry =
            (region_size * (alg_parms->first_dup_region + 1)) -
            alg_parms->dup_remain - 1;
    }

    return SOC_E_NONE;
}

STATIC int
_soc_er_defip_alg_next(int unit, int start_ptr, int dup_only,
                       soc_er_defip_alg_parm_t *lpm_parms)
{
    uint32 region_size, reg_bit, mem_size;
    uint32 next_ptr, region;

    region_size = lpm_parms->region_size;
    next_ptr = start_ptr + 1;
    mem_size = soc_mem_index_count(unit, L3_DEFIP_ALGm);

    while (next_ptr < mem_size) {
        region = (next_ptr / region_size);
        reg_bit = 1 << region;
        if ((lpm_parms->valid_bmp & reg_bit) == 0) {
            /* Invalid region, skip to start of next */
            next_ptr = (region + 1) * region_size;
            continue;
        } else if (((lpm_parms->dup_bmp & reg_bit) == 0) && (dup_only)) {
            /* Invalid dup region, skip to start of next */
            next_ptr = (region + 1) * region_size;
            continue;
        }

        /* Valid region, now check boundaries */
        if (region == lpm_parms->first_dup_region) {
            if (next_ptr > lpm_parms->last_fr_dup_entry) {
                /* Hole at end of dup region, skip to start of next */
                next_ptr = (region + 1) * region_size;
                continue;
            }
        } else if (region == lpm_parms->last_search_region) {
            if (next_ptr > lpm_parms->last_search_entry) {
                /* Hole at end of valid region, skip to start of next */
                next_ptr = (region + 1) * region_size;
                continue;
            } 
        }

        /* Survived to here, OK */
        return next_ptr;
    }

    return -1;
}

/* 
 * Notes:
 *
 * index_ptr is IN/OUT.  It should be the last index returned when IN.
 * It will return the next step when OUT.
 *
 * -1 signals the start and end of the iteration.
 *
 * NB: The calling function must take the L3_DEFIP_ALG table lock before
 * beginning the iterate sequence, and hold it until finished.
 */
STATIC int
soc_er_defip_alg_iterate(int unit, int *index_ptr,
                         defip_alg_entry_t *return_data, int top)
{
    static soc_er_defip_alg_parm_t lpm_parms;
    int    next_ptr;
    uint32 top_bit, ff_bit, key_type;

    if (*index_ptr < 0) {
        SOC_IF_ERROR_RETURN
            (_soc_er_defip_alg_params_get(unit, &lpm_parms));
    }
    
    next_ptr = *index_ptr;
    while (1) {
        next_ptr = _soc_er_defip_alg_next(unit, next_ptr, FALSE, &lpm_parms);
        if (next_ptr < 0) {
            break;
        }
        SOC_IF_ERROR_RETURN
            (READ_L3_DEFIP_ALGm(unit, MEM_BLOCK_ANY, next_ptr, return_data));
        top_bit = soc_mem_field32_get(unit, L3_DEFIP_ALGm,
                                      return_data, TOP_BOTTOMf);
        ff_bit = soc_mem_field32_get(unit, L3_DEFIP_ALGm,
                                      return_data, FFf);
        key_type = soc_mem_field32_get(unit, L3_DEFIP_ALGm,
                                      return_data, KEY_TYPEf);

        if ((ff_bit == 0xff) && (key_type != SOC_ER_DEFIP_KEY_TYPE_V4)) {
            /* MPLS */
            break;
        } else if ((top && top_bit) || (!top && !top_bit)) {
            break;
        }
    }

    *index_ptr = next_ptr;
    return SOC_E_NONE;
}

STATIC int
soc_er_defip_tcam_iterate(int unit, int *index_ptr,
                          defip_alg_entry_t *return_data)
{
    static uint32 num_entries;
    defip_cam_entry_t tcam_return;
    uint32 rval, index;

    if (*index_ptr < 0) {
        SOC_IF_ERROR_RETURN(READ_BOUNDARY_MPLSr(unit, &rval));
        num_entries = soc_reg_field_get(unit, BOUNDARY_MPLSr,
                                        rval, BOUNDARY_MPLS_BITSf);
        index = 0;
    } else {
        index = *index_ptr + 1;
    }

    if (index <= num_entries) {
        SOC_IF_ERROR_RETURN
            (READ_L3_DEFIP_CAMm(unit, MEM_BLOCK_ANY, index, &tcam_return));
        _soc_er_defip_tcam_to_alg(unit, (uint32 *)&tcam_return,
                                  (uint32 *)return_data);
    } else {
        index = -1;
    }
    
    *index_ptr = index;

    return SOC_E_NONE;
}

STATIC void
_soc_er_defip_alg_top_to_bottom(int unit, defip_alg_entry_t *entry_data)
{
    uint32 ipv4, prev_ipv6l;
    uint32 ipu64[SOC_MAX_MEM_FIELD_WORDS];
    int    prefix_length;

    if (soc_mem_field32_get(unit, L3_DEFIP_ALGm, entry_data, FFf) == 0xff) {
        if (soc_mem_field32_get(unit, L3_DEFIP_ALGm, entry_data,
                                KEY_TYPEf) == SOC_ER_DEFIP_KEY_TYPE_V4) {
            ipv4 = soc_mem_field32_get(unit, L3_DEFIP_ALGm, entry_data,
                                       IP_ADDR_V4f);
            prefix_length =  soc_mem_field32_get(unit, L3_DEFIP_ALGm,
                                                 entry_data, PREFIX_LENGTHf);
            if (prefix_length == 0) {
                soc_mem_field32_set(unit, L3_DEFIP_ALGm, entry_data,
                                    IP_ADDR_V4f, 0xffffffff);
            } else {
                ipv4 += 1 << (32 - prefix_length);
                soc_mem_field32_set(unit, L3_DEFIP_ALGm, entry_data,
                                    IP_ADDR_V4f, ipv4);
            }
            soc_mem_field32_set(unit, L3_DEFIP_ALGm, entry_data,
                                TOP_BOTTOMf, 0);
        }
    } else {
        soc_mem_field_get(unit, L3_DEFIP_ALGm, (uint32 *)entry_data,
                          IP_ADDR_V6f, ipu64);
        prefix_length =  soc_mem_field32_get(unit, L3_DEFIP_ALGm,
                                             entry_data, PREFIX_LENGTHf);
        if (prefix_length == 0) {
            ipu64[0] = 0;
            ipu64[1] = 0xff000000;
        } else {
            if (prefix_length <= 32) {
                ipu64[1] += 1 << (32 - prefix_length);
            } else {
                prev_ipv6l = ipu64[0];
                ipu64[0] += 1 << (64 - prefix_length);
                if (prev_ipv6l > ipu64[0]) {
                    /* Carry! */
                    ipu64[1] += 1;
                }
            }
        }
        soc_mem_field_set(unit, L3_DEFIP_ALGm, (uint32 *)entry_data,
                          IP_ADDR_V6f, ipu64);
        soc_mem_field32_set(unit, L3_DEFIP_ALGm, entry_data,
                                TOP_BOTTOMf, 0);
    }
}

STATIC int
soc_er_defip_alg_search(int unit, int *index_ptr,
                        defip_alg_entry_t *entry_data,
                        defip_alg_entry_t *return_data, int top)
{
    soc_er_defip_alg_parm_t lpm_parms;
    defip_alg_entry_t search_data;
    uint32 prlen_in, prlen_out, dup, region;
    int    index, base, count, cmp, ff_bit, key_type, top_out;

    sal_memcpy(&search_data, entry_data, sizeof(defip_alg_entry_t));
    if (!top) {
        /* Searching for a bottom entry, adjust search key */
        _soc_er_defip_alg_top_to_bottom(unit, &search_data);
    }

    SOC_IF_ERROR_RETURN
        (soc_mem_er_search(unit, L3_DEFIP_ALGm, COPYNO_ALL, &index,
                           &search_data, return_data, FALSE));

    ff_bit = soc_mem_field32_get(unit, L3_DEFIP_ALGm,
                                 return_data, FFf);
    key_type = soc_mem_field32_get(unit, L3_DEFIP_ALGm,
                                   return_data, KEY_TYPEf);

    if ((ff_bit == 0xff) && (key_type != SOC_ER_DEFIP_KEY_TYPE_V4)) {
        /* MPLS, take as is */
    } else {
        prlen_in = soc_mem_field32_get(unit, L3_DEFIP_ALGm,
                                       &search_data, PREFIX_LENGTHf);
        prlen_out = soc_mem_field32_get(unit, L3_DEFIP_ALGm,
                                        return_data, PREFIX_LENGTHf);
        top_out = soc_mem_field32_get(unit, L3_DEFIP_ALGm,
                                      return_data, TOP_BOTTOMf);
        dup = soc_mem_field32_get(unit, L3_DEFIP_ALGm,
                                  return_data, DUPf);

        if ((prlen_in != prlen_out) || (top != top_out)) {
            if (!dup) {
                return SOC_E_NOT_FOUND;
            }

            /* We get to find the entry in the dup secions */
            SOC_IF_ERROR_RETURN
                (_soc_er_defip_alg_params_get(unit, &lpm_parms));

            cmp = -1; /* Avoid spurious successes */
            for (region = lpm_parms.first_dup_region;
                 region < lpm_parms.region_num; region++) {
                index = region * lpm_parms.region_size;
                SOC_IF_ERROR_RETURN
                    (READ_L3_DEFIP_ALGm(unit, MEM_BLOCK_ANY,
                                        index, return_data));
                cmp = soc_mem_compare_key(unit, L3_DEFIP_ALGm,
                                          &search_data, return_data);

                if (cmp >= 0) {
                    /* It's in this region */
                    break;
                }
            }

            if (cmp == 0) {
                /* Already done */
            } else if (region < lpm_parms.region_num) {
                base = region * lpm_parms.region_size;
                count = lpm_parms.region_size;

                if (region == lpm_parms.first_dup_region) {
                    count = lpm_parms.last_fr_dup_entry - base + 1;
                }

                while (count > 0) {
                    index = base + count / 2;

                    SOC_IF_ERROR_RETURN
                        (READ_L3_DEFIP_ALGm(unit, MEM_BLOCK_ANY,
                                            index, return_data));

                    cmp = soc_mem_compare_key(unit, L3_DEFIP_ALGm,
                                              &search_data, return_data);

                    if (cmp == 0) {
                        break; /* Done */
                    } else if (cmp < 0) {
                        count = count / 2;
                    } else {
                        base = index + 1;
                        count = (count - 1) / 2;
                    }
                }

                if (count == 0) {
                    /* No luck */
                    return SOC_E_NOT_FOUND;
                }
            } else {
                /* Out of bounds */
                return SOC_E_NOT_FOUND;
            }
        }
    }

    if (index_ptr != NULL) {
        *index_ptr = index;
    }

    return SOC_E_NONE;
}

int
_soc_er_defip_lookup(int unit, int *index_ptr,
                     defip_alg_entry_t *entry_data,
                     defip_alg_entry_t *return_data,
                     int mask_len)
{
    defip_cam_entry_t tcam_data, tcam_return;

    if (soc_mem_index_count(unit, L3_DEFIP_CAMm) > 0) {
        _soc_er_defip_alg_to_tcam(unit, (uint32 *)entry_data,
                                  (uint32 *)&tcam_data, mask_len);
        SOC_IF_ERROR_RETURN
            (soc_mem_er_search(unit, L3_DEFIP_CAMm, COPYNO_ALL, index_ptr,
                               &tcam_data, &tcam_return, FALSE));
        _soc_er_defip_tcam_to_alg(unit, (uint32 *)&tcam_return,
                                  (uint32 *)return_data);
    } else {
        SOC_IF_ERROR_RETURN
            (soc_er_defip_alg_search(unit, index_ptr, entry_data,
                                     return_data, TRUE));
    }

    return SOC_E_NONE;
}

int
soc_er_defip_insert(int unit, defip_alg_entry_t *entry_data)
{
    defip_alg_entry_t return_data;
    uint32 prefix;
    uint32 ip64[2];
    int type, subtype, index, rv0, rv1;

    soc_tcam_get_info(unit, &type, &subtype, NULL, NULL);

    if (soc_mem_index_count(unit, L3_DEFIP_CAMm) <= 0 ||
        type != ER_EXT_TCAM_TYPE1 || subtype != 2 ||
        (prefix = soc_mem_field32_get(unit, L3_DEFIP_ALGm, entry_data,
                                      PREFIX_LENGTHf)) < 63) {
        /* no workaround needed */
        return _soc_er_defip_insert(unit, entry_data, -1);
    }

    /* work around for XY type TCAM on easyrider defip insert */
    if (prefix == 64) {
        /* entry:data/mask length 64, prefix_length field 63 */
        soc_mem_field32_set(unit, L3_DEFIP_ALGm, entry_data,
                            PREFIX_LENGTHf, prefix - 1);
        /* make a mark to indicate this is a substitute /64 entry */
        soc_mem_field32_set(unit, L3_DEFIP_ALGm, entry_data,
                            MPLS_USE_PRIf, 1);
        return _soc_er_defip_insert(unit, entry_data, 64);
    } else if (prefix == 63) {
        /* entry 1:data/mask length 63, prefix_length field 1 */
        soc_mem_field32_set(unit, L3_DEFIP_ALGm, entry_data, PREFIX_LENGTHf,
                            1);
        rv0 =_soc_er_defip_insert(unit, entry_data, 63);
        if (rv0 < 0 && rv0 != SOC_E_NOT_FOUND) {
            return rv0;
        }

        /* entry 2:data/mask length 64, prefix_length field 63 */
        soc_mem_field32_set(unit, L3_DEFIP_ALGm, entry_data, PREFIX_LENGTHf,
                            63);
        rv0 = _soc_er_defip_lookup(unit, &index, entry_data, &return_data, 64);
        if (rv0 == SOC_E_NOT_FOUND) {
            rv0 =_soc_er_defip_insert(unit, entry_data, 64);
        }
        if (rv0 < 0 && rv0 != SOC_E_NOT_FOUND) {
            soc_mem_field32_set(unit, L3_DEFIP_ALGm, entry_data,
                                PREFIX_LENGTHf, 1);
            _soc_er_defip_delete(unit, entry_data, 63);
            return rv0;
        }

        /* entry 3:data/mask length 64, prefix_length field 63 */
        soc_mem_field_get(unit, L3_DEFIP_ALGm, (uint32 *)entry_data,
                          IP_ADDR_V6f, ip64);
        ip64[0] ^= 1; /* toggle the LSB of 64-bit prefix */
        soc_mem_field_set(unit, L3_DEFIP_ALGm, (uint32 *)entry_data,
                          IP_ADDR_V6f, ip64);
        rv1 = _soc_er_defip_lookup(unit, &index, entry_data, &return_data, 64);
        if (rv1 == SOC_E_NOT_FOUND) {
            rv1 =_soc_er_defip_insert(unit, entry_data, 64);
        }
        if (rv1 < 0 && rv1 != SOC_E_NOT_FOUND) {
            if (rv0 == SOC_E_NOT_FOUND) {
                ip64[0] ^= 1;
                soc_mem_field_set(unit, L3_DEFIP_ALGm, (uint32 *)entry_data,
                                   IP_ADDR_V6f, ip64);
                _soc_er_defip_delete(unit, entry_data, 64);
            }
            soc_mem_field32_set(unit, L3_DEFIP_ALGm, entry_data,
                                PREFIX_LENGTHf, 1);
            _soc_er_defip_delete(unit, entry_data, 63);
            return rv1;
        }
        return SOC_E_NONE;
    }

    return SOC_E_INTERNAL;
}

int
soc_er_defip_delete(int unit, defip_alg_entry_t *entry_data)
{
    defip_alg_entry_t return_data;
    uint32 prefix;
    uint32 field_buf[SOC_MAX_MEM_FIELD_WORDS];
    uint32 ip64[2];
    int type, subtype, index, rv;

    soc_tcam_get_info(unit, &type, &subtype, NULL, NULL);

    if (soc_mem_index_count(unit, L3_DEFIP_CAMm) <= 0 ||
        type != ER_EXT_TCAM_TYPE1 || subtype != 2 ||
        (prefix = soc_mem_field32_get(unit, L3_DEFIP_ALGm, entry_data,
                                      PREFIX_LENGTHf)) < 63) {
        /* no workaround needed */
        return _soc_er_defip_delete(unit, entry_data, -1);
    }

    /* work around for XY type TCAM on easyrider defip delete */
    if (prefix == 64) {
        /* entry:data/mask length 64, prefix_length field 63 */
        soc_mem_field32_set(unit, L3_DEFIP_ALGm, entry_data,
                            PREFIX_LENGTHf, prefix - 1);
        SOC_IF_ERROR_RETURN(_soc_er_defip_delete(unit, entry_data, 64));

        /* check if there is a /63 entry covering this /64 */
        soc_mem_field32_set(unit, L3_DEFIP_ALGm, entry_data, PREFIX_LENGTHf,
                            1);
        rv = _soc_er_defip_lookup(unit, &index, entry_data, &return_data, 63);
        if (rv == SOC_E_NONE) {
            soc_mem_field_get(unit, L3_DEFIP_ALGm, (uint32 *)&return_data,
                              DATA_AGRf, field_buf);
            soc_mem_field_set(unit, L3_DEFIP_ALGm, (uint32 *)entry_data,
                              DATA_AGRf, field_buf);
            soc_mem_field32_set(unit, L3_DEFIP_ALGm, entry_data,
                                PREFIX_LENGTHf, prefix - 1);
            /* clear the mark to indicate this is not a substitute /64 entry */
            soc_mem_field32_set(unit, L3_DEFIP_ALGm, entry_data,
                                MPLS_USE_PRIf, 0);
            rv = _soc_er_defip_insert(unit, entry_data, 64);
            if (rv == SOC_E_NOT_FOUND) {
                rv = SOC_E_NONE;
            }
        } else if (rv == SOC_E_NOT_FOUND) {
            rv = SOC_E_NONE;
        }
        return rv;
    } else if (prefix == 63) {
        /* entry 1:data/mask length 63, prefix_length field 1 */
        soc_mem_field32_set(unit, L3_DEFIP_ALGm, entry_data, PREFIX_LENGTHf,
                            1);
        SOC_IF_ERROR_RETURN(_soc_er_defip_delete(unit, entry_data, 63));

        /* entry 2:data/mask length 64, prefix_length field 63 */
        soc_mem_field32_set(unit, L3_DEFIP_ALGm, entry_data, PREFIX_LENGTHf,
                            63);
        rv = _soc_er_defip_lookup(unit, &index, entry_data, &return_data, 64);
        if (rv == SOC_E_NONE) {
           /* check if this is a /64 entry by examining the mark */
           if (soc_mem_field32_get(unit, L3_DEFIP_ALGm, &return_data,
                                    MPLS_USE_PRIf) != 1) {
                SOC_IF_ERROR_RETURN(_soc_er_defip_delete(unit, entry_data,
                                                         64));
            }
        } else if (rv == SOC_E_NOT_FOUND) {
            rv = SOC_E_NONE;
        } else {
            return rv;
        }

        /* entry 3:data/mask length 64, prefix_length field 63 */
        soc_mem_field_get(unit, L3_DEFIP_ALGm, (uint32 *)entry_data,
                          IP_ADDR_V6f, ip64);
        ip64[0] ^= 1; /* toggle the LSB of 64-bit prefix */
        soc_mem_field_set(unit, L3_DEFIP_ALGm, (uint32 *)entry_data,
                          IP_ADDR_V6f, ip64);
        rv = _soc_er_defip_lookup(unit, &index, entry_data, &return_data, 64);
        if (rv == SOC_E_NONE) {
            /* check if this is a /64 entry by examining the mark */
            if (soc_mem_field32_get(unit, L3_DEFIP_ALGm, &return_data,
                                    MPLS_USE_PRIf) != 1) {
                SOC_IF_ERROR_RETURN(_soc_er_defip_delete(unit, entry_data,
                                                         64));
            }
        } else if (rv == SOC_E_NOT_FOUND) {
            rv = SOC_E_NONE;
        } else {
            return rv;
        }
        return rv;
    }

    return SOC_E_INTERNAL;
}

int
soc_er_defip_lookup(int unit, int *index_ptr,
                    defip_alg_entry_t *entry_data,
                    defip_alg_entry_t *return_data)
{
    uint32 prefix;
    int type, subtype, rv;

    soc_tcam_get_info(unit, &type, &subtype, NULL, NULL);

    if (soc_mem_index_count(unit, L3_DEFIP_CAMm) <= 0 ||
        type != ER_EXT_TCAM_TYPE1 || subtype != 2 ||
        (prefix = soc_mem_field32_get(unit, L3_DEFIP_ALGm, entry_data,
                                      PREFIX_LENGTHf)) < 63) {
        /* no workaround needed */
        return _soc_er_defip_lookup(unit, index_ptr, entry_data, return_data,
                                    -1);
    }

    /* work around for XY type TCAM on easyrider defip lookup */
    if (prefix == 64) {
        /* lookup using: data/mask length 64, prefix_length field 63 */
        soc_mem_field32_set(unit, L3_DEFIP_ALGm, entry_data,
                            PREFIX_LENGTHf, prefix - 1);
        rv = _soc_er_defip_lookup(unit, index_ptr, entry_data, return_data,
                                  64);
        if (rv == SOC_E_NONE) {
            /* examine the mark to see if this is a substitute of /64 entry */
            if (soc_mem_field32_get(unit, L3_DEFIP_ALGm, return_data,
                                    MPLS_USE_PRIf) != 1) {
                rv = SOC_E_NOT_FOUND;
            }
        }
        return rv;
    } else if (prefix == 63) {
        /* lookup using: data/mask length 63, prefix_length field 1 */
        soc_mem_field32_set(unit, L3_DEFIP_ALGm, entry_data, PREFIX_LENGTHf,
                            1);
        return _soc_er_defip_lookup(unit, index_ptr, entry_data,
                                    return_data, 63);
    }

    return SOC_E_INTERNAL;
}

/*
 * NB: The calling function must taked the L3_DEFIP_* tables lock before
 * beginning the iterate sequence, and hold it until finished.
 *
 * The first call in the sequence must pass *index_ptr = -1 to initialize
 * static variables used in the iteration.
 */
int
soc_er_defip_iterate(int unit, int *index_ptr,
                     defip_alg_entry_t *return_data)
{
    if (soc_mem_index_count(unit, L3_DEFIP_CAMm) > 0) {
        SOC_IF_ERROR_RETURN
            (soc_er_defip_tcam_iterate(unit, index_ptr, return_data));
    } else {
        SOC_IF_ERROR_RETURN
            (soc_er_defip_alg_iterate(unit, index_ptr, return_data, TRUE));
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_er_defip_traverse
 * Purpose:
 *      Traverse all LPM entries (including MPLS), call back the test
 *      function and save entries which pass.  After a configuration number
 *      of entries are saved, call back the operation function on each.
 *      Repeat from beginning of table after operations are complete.
 *      Continue until the no entries test TRUE.
 * Parameters:
 *      unit      - SOC unit number.
 *      test_cb_fn - the test function to be called for each route found
 *      op_cb_fn  - the operation function to be called for each route
 *                  which tests as TRUE in the test function
 *      cmp_data  - data supplied to test call back function
 *      op_data   - data supplied to operation call back function
 * Notes:
 *      The test function must not perform any changes to the
 *      table.  Also, it must know how to exclude entries which have
 *      already been processed by the operation function.  Otherwise, the
 *      loop may never terminate.
 *      The operation function may update the table.
 *      We require to pass table index so caller can acccess entry hit 
 *      information for traverse & aging functionalities.
 *      Table index is not available during op callback and should be ignored.
 */
int
soc_er_defip_traverse(int unit, soc_er_defip_cb_fn test_cb_fn,
                      soc_er_defip_cb_fn op_cb_fn,
                      void *test_data, void *op_data)
{
    defip_alg_entry_t *search_data_array;
    int table_index, array_index, array_num, op_index, test_result;
    int rv=SOC_E_NONE;

    array_num = soc_property_get(unit, spn_SEER_LPM_TRAVERSE_ENTRIES,
                                 SOC_ER_LPM_TRAVERSE_ENTRIES_DEFAULT);

    if ((search_data_array =
         sal_alloc(array_num * sizeof(defip_alg_entry_t),
                   "ER LPM traverse data array")) == NULL) {
        return SOC_E_MEMORY;
    }

    while (1) {
        array_index = 0;
        table_index = -1;
        
        MEM_LOCK(unit, L3_DEFIP_ALGm);
        MEM_LOCK(unit, L3_DEFIP_CAMm);
        while (array_index < array_num) {
            soc_er_defip_iterate(unit, &table_index,
                                 (search_data_array + array_index));
            if (table_index < 0) {
                break;
            }
            if ((rv = (*test_cb_fn)(unit, (search_data_array + array_index),
                                    table_index, &test_result, test_data)) < 0) {
                MEM_UNLOCK(unit, L3_DEFIP_CAMm);
                MEM_UNLOCK(unit, L3_DEFIP_ALGm);
                goto done;
            }
            if (0 == test_result) {
                array_index++;
            }
        }

        MEM_UNLOCK(unit, L3_DEFIP_CAMm);
        MEM_UNLOCK(unit, L3_DEFIP_ALGm);

        for (op_index = 0; op_index < array_index; op_index++) {
            if ((rv = (*op_cb_fn)(unit, (search_data_array + op_index),
                                  -1, &test_result, op_data)) < 0) {
                goto done;
            }
        }
        if (table_index < 0) {
            break;
        }
    }

done:
    if (search_data_array) {
        sal_free(search_data_array);
    }
    return rv;
}


#define SOC_ER_METER_EXT_WRITE_MAX 200
int
soc_er_fp_meter_table_ext_write(int unit, int index, 
                                fp_meter_table_ext_entry_t *entry_data)
{
    fp_meter_table_ext_entry_t read_data;
    int refresh_count, bucket_size, refresh_mode, rd_val, i;

    refresh_count = soc_FP_METER_TABLE_EXTm_field32_get(unit, entry_data,
                                                        REFRESHCOUNTf);
    bucket_size = soc_FP_METER_TABLE_EXTm_field32_get(unit, entry_data,
                                                      BUCKETSIZEf);
    refresh_mode = soc_FP_METER_TABLE_EXTm_field32_get(unit, entry_data,
                                                       REFRESH_MODEf);

    for (i=0; i < SOC_ER_METER_EXT_WRITE_MAX; i++) {
        SOC_IF_ERROR_RETURN
            (WRITE_FP_METER_TABLE_EXTm(unit, MEM_BLOCK_ALL, index, entry_data));
        SOC_IF_ERROR_RETURN
            (READ_FP_METER_TABLE_EXTm(unit, MEM_BLOCK_ALL, index, &read_data));

        rd_val = soc_FP_METER_TABLE_EXTm_field32_get(unit, &read_data, 
                                                     REFRESHCOUNTf);
        if (rd_val != refresh_count) {
            continue;
        }
        rd_val = soc_FP_METER_TABLE_EXTm_field32_get(unit, &read_data, 
                                                     BUCKETSIZEf);
        if (rd_val != bucket_size) {
            continue;
        }
        rd_val = soc_FP_METER_TABLE_EXTm_field32_get(unit, &read_data, 
                                                     REFRESH_MODEf);
        if (rd_val != refresh_mode) {
            continue;
        }
        /* The write succeeded. */
        return SOC_E_NONE;
    }
    /* The write failed. */
    soc_cm_debug(DK_ERR, 
                 "unit %d : FP_METER_TABLE_EXT write failed after %d tries.\n",
                 unit, i);

    return SOC_E_INTERNAL;
}

STATIC void
_soc_er_fp_ext_normalize_mask(int unit, fp_external_entry_t *entry_data,
                              int read)
{
    uint32     field_buf[SOC_MAX_MEM_FIELD_WORDS];
    uint32     data;
    int        type, subtype, i;

    soc_tcam_get_info(unit, &type, &subtype, NULL, NULL);

    if (type == ER_EXT_TCAM_TYPE1) {
        soc_mem_field_get(unit, FP_EXTERNALm, (uint32 *)entry_data,
                          MASKf, field_buf);

        if (subtype == 2 && read) {
            /* do XY to DM conversion */
            for (i = 0; i < 5; i++) {
                field_buf[i] = ~((uint32 *)entry_data)[i] & ~field_buf[i];
            }
            field_buf[4] &= 0xffff;
        }

        /* Flip TCAM mask polarity Mask is 144 bits in ER */
        field_buf[0] ^= 0xffffffff;
        field_buf[1] ^= 0xffffffff;
        field_buf[2] ^= 0xffffffff;
        field_buf[3] ^= 0xffffffff;
        field_buf[4] ^= 0x0000ffff;

        if (subtype == 2 && !read) {
            /* do DM to XY conversion */
            for (i = 0; i < 5; i++) {
                data = ((uint32 *)entry_data)[i];
                ((uint32 *)entry_data)[i] = data & ~field_buf[i];
                field_buf[i] = ~data & ~field_buf[i];
            }
            field_buf[4] &= 0xffff;
        }

        soc_mem_field_set(unit, FP_EXTERNALm, (uint32 *)entry_data,
                          MASKf, field_buf);
    }
}

int
soc_er_fp_ext_read(int unit, int index, fp_external_entry_t *return_data)
{
    SOC_IF_ERROR_RETURN
        (READ_FP_EXTERNALm(unit, MEM_BLOCK_ANY, index, return_data));
    _soc_er_fp_ext_normalize_mask(unit, return_data, TRUE);

    return SOC_E_NONE;
}

int
soc_er_fp_ext_write(int unit, int index, fp_external_entry_t *entry_data)
{
    fp_external_entry_t normalized_data;

    sal_memcpy(&normalized_data, entry_data, sizeof(fp_external_entry_t));
    _soc_er_fp_ext_normalize_mask(unit, &normalized_data, FALSE);
    SOC_IF_ERROR_RETURN
        (WRITE_FP_EXTERNALm(unit, MEM_BLOCK_ALL, index, &normalized_data));

    return SOC_E_NONE;
}

int
soc_er_fp_ext_lookup(int unit, int *index,
                     fp_external_entry_t *entry_data,
                     fp_external_entry_t *return_data)
{
    fp_external_entry_t normalized_data;

    sal_memcpy(&normalized_data, entry_data, sizeof(fp_external_entry_t));
    _soc_er_fp_ext_normalize_mask(unit, &normalized_data, FALSE);
    SOC_IF_ERROR_RETURN
        (soc_mem_er_search(unit, FP_EXTERNALm, COPYNO_ALL, index,
                           &normalized_data, return_data, FALSE));
    _soc_er_fp_ext_normalize_mask(unit, return_data, TRUE);

    return SOC_E_NONE;
}

STATIC void
_soc_er_fp_tcam_ext_normalize_mask(int unit, fp_tcam_external_entry_t *entry_data, int read)
{
    uint32     field_buf[SOC_MAX_MEM_FIELD_WORDS];
    uint32     data;
    int        type, subtype, i;

    soc_tcam_get_info(unit, &type, &subtype, NULL, NULL);

    if (type == ER_EXT_TCAM_TYPE1) {
        soc_mem_field_get(unit, FP_TCAM_EXTERNALm, (uint32 *)entry_data,
                          MASKf, field_buf);

        if (subtype == 2 && read) {
            /* do XY to DM conversion */
            for (i = 0; i < 5; i++) {
                field_buf[i] = ~((uint32 *)entry_data)[i] & ~field_buf[i];
            }
            field_buf[4] &= 0xffff;
        }

        /* Flip TCAM mask polarity Mask is 144 bits in ER */
        field_buf[0] ^= 0xffffffff;
        field_buf[1] ^= 0xffffffff;
        field_buf[2] ^= 0xffffffff;
        field_buf[3] ^= 0xffffffff;
        field_buf[4] ^= 0x0000ffff;

        if (subtype == 2 && !read) {
            /* do DM to XY conversion */
            for (i = 0; i < 5; i++) {
                data = ((uint32 *)entry_data)[i];
                ((uint32 *)entry_data)[i] = data & ~field_buf[i];
                field_buf[i] = ~data & ~field_buf[i];
            }
            field_buf[4] &= 0xffff;
        }

        soc_mem_field_set(unit, FP_TCAM_EXTERNALm, (uint32 *)entry_data,
                          MASKf, field_buf);
    }
}

int
soc_er_fp_tcam_ext_read(int unit, int index, fp_tcam_external_entry_t *return_data)
{
    SOC_IF_ERROR_RETURN
        (READ_FP_TCAM_EXTERNALm(unit, MEM_BLOCK_ANY, index, return_data));
    _soc_er_fp_tcam_ext_normalize_mask(unit, return_data, TRUE);

    return SOC_E_NONE;
}

int
soc_er_fp_tcam_ext_write(int unit, int index, fp_tcam_external_entry_t *entry_data)
{
    fp_tcam_external_entry_t normalized_data;

    sal_memcpy(&normalized_data, entry_data, sizeof(fp_tcam_external_entry_t));
    _soc_er_fp_tcam_ext_normalize_mask(unit, &normalized_data, FALSE);
    SOC_IF_ERROR_RETURN
        (WRITE_FP_TCAM_EXTERNALm(unit, MEM_BLOCK_ALL, index, &normalized_data));

    return SOC_E_NONE;
}

int
soc_er_celldata_chunk_read(int unit, soc_mem_t mem,
                           int index, soc_er_cd_chunk_t chunk_data)
{
    soc_mem_t          read_mem;
    soc_reg_t          cmd_reg;
    uint32             cmd_data;
    int                chunk;

    if (mem == C0_CELLm) {
        read_mem = C0_CPU_RQm;
        cmd_reg = MCU_CHN0_REQ_CMDr;
    } else if (mem == C1_CELLm) {
        read_mem = C1_CPU_RQm;
        cmd_reg = MCU_CHN1_REQ_CMDr;
    } else {
        return SOC_E_PARAM;
    }

    cmd_data = 0;
    soc_reg_field_set(unit, cmd_reg, &cmd_data, DDR_ADDRf, index);
    /* Read data code */
    soc_reg_field_set(unit, cmd_reg, &cmd_data, COMMANDf, 4);
    SOC_IF_ERROR_RETURN
        (soc_reg32_write(unit, soc_reg_addr(unit, cmd_reg,
                                            REG_PORT_ANY, 0), cmd_data));

    for (chunk = 0; chunk < SOC_ER_CELLDATA_CHUNKS; chunk++) {
        /* Read mem */
        SOC_IF_ERROR_RETURN
            (soc_mem_read(unit, read_mem, MEM_BLOCK_ANY,
                          chunk, chunk_data[chunk]));
    }

    return SOC_E_NONE;
}

void
soc_er_celldata_chunk_to_slice(soc_er_cd_chunk_t chunk_data,
                               soc_er_cd_slice_t slice_data)
{
    int chunk, chunk_word, slice4, slice4_word, slice;
    soc_er_cd_slice4_t slice4_data;

    chunk = 0;
    chunk_word = SOC_ER_CELLDATA_CHUNK_WORDS - 1;
    slice4 = 0;
    slice4_word = SOC_ER_CELLDATA_SLICE4_WORDS - 1;
    /* These transformations are brute force, but comprehensible */
    while (chunk < SOC_ER_CELLDATA_CHUNKS) {
        slice4_data[slice4][slice4_word] = chunk_data[chunk][chunk_word];
        if ((--slice4_word) < 0) {
            slice4_word = SOC_ER_CELLDATA_SLICE4_WORDS - 1;
            slice4++;
        }
        if ((--chunk_word) < 0) {
            chunk_word = SOC_ER_CELLDATA_CHUNK_WORDS - 1;
            chunk++;
        }
    }

    slice4 = 0;
    slice = 0;
    while (slice4 < (SOC_ER_CELLDATA_SLICE4S - 1)) {
        slice_data[slice][2] = (slice4_data[slice4][8] >> 24) & 0xff;
        slice_data[slice][1] = ((slice4_data[slice4][8] << 8) & 0xffffff00) |
            ((slice4_data[slice4][7] >> 24) & 0xff);
        slice_data[slice][0] = ((slice4_data[slice4][7] << 8) & 0xffffff00) |
            ((slice4_data[slice4][6] >> 24) & 0xff);
        slice++;
        
        slice_data[slice][2] = (slice4_data[slice4][6] >> 16) & 0xff;
        slice_data[slice][1] = ((slice4_data[slice4][6] << 16) & 0xffff0000) |
            ((slice4_data[slice4][5] >> 16) & 0xffff);
        slice_data[slice][0] = ((slice4_data[slice4][5] << 16) & 0xffff0000) |
            ((slice4_data[slice4][4] >> 16) & 0xffff);
        slice++;
        
        slice_data[slice][2] = (slice4_data[slice4][4] >> 8) & 0xff;
        slice_data[slice][1] = ((slice4_data[slice4][4] << 24) & 0xff000000) |
            ((slice4_data[slice4][3] >> 8) & 0xffffff);
        slice_data[slice][0] = ((slice4_data[slice4][3] << 24) & 0xff000000) |
            ((slice4_data[slice4][2] >> 8) & 0xffffff);
        slice++;
        
        slice_data[slice][2] = slice4_data[slice4][2] & 0xff;
        slice_data[slice][1] = slice4_data[slice4][1];
        slice_data[slice][0] = slice4_data[slice4][0];
        slice++;
        slice4++;
    }

    /* Last slice */
    slice_data[slice][2] = (slice4_data[slice4][8] >> 24) & 0xff;
    slice_data[slice][1] = ((slice4_data[slice4][8] << 8) & 0xffffff00);
    slice_data[slice][0] = 0;
}

void
soc_er_celldata_slice_to_columns(soc_er_cd_slice_t slice_data,
                                 soc_er_cd_columns_t columns_data)
{
    int                slice;

    for (slice = 0; slice < SOC_ER_CELLDATA_SLICES; slice++) {
        columns_data[slice][0] = slice_data[slice][0] & 0x3ffff;
        columns_data[slice][1] = ((slice_data[slice][0] >> 18) & 0x3fff) |
            ((slice_data[slice][1] & 0xf) << 14);
        columns_data[slice][2] = ((slice_data[slice][1] >> 4) & 0x3ffff);
        columns_data[slice][3] = ((slice_data[slice][1] >> 22) & 0x3ff) |
            ((slice_data[slice][2] & 0xff) << 10);
    }
}

void
soc_er_celldata_columns_to_slice(soc_er_cd_columns_t columns_data,
                                 soc_er_cd_slice_t slice_data)
{
    int                slice;

    for (slice = 0; slice < SOC_ER_CELLDATA_SLICES; slice++) {
        slice_data[slice][0] = (columns_data[slice][0] & 0x3ffff) |
            ((columns_data[slice][1] & 0x3fff) << 18);
        slice_data[slice][1] = ((columns_data[slice][1] >> 14) & 0xf) |
            ((columns_data[slice][2] & 0x3ffff) << 4) |
            ((columns_data[slice][3] & 0x3ff) << 22);
        slice_data[slice][2] = ((columns_data[slice][3] >> 10) & 0xff);
    }
}

void
soc_er_celldata_slice_to_chunk(soc_er_cd_slice_t slice_data,
                               soc_er_cd_chunk_t chunk_data)
{
    int chunk, chunk_word, slice4, slice4_word, slice;
    soc_er_cd_slice4_t slice4_data;

    /* First, flush the write buffers */
    for (slice4 = 0; slice4 < SOC_ER_CELLDATA_SLICE4S; slice4++) {
        sal_memset(slice4_data[slice4], 0,
                   WORDS2BYTES(SOC_ER_CELLDATA_SLICE4_WORDS));
    }
    for (chunk = 0; chunk < SOC_ER_CELLDATA_CHUNKS; chunk++) {
        sal_memset(chunk_data[chunk], 0,
                   WORDS2BYTES(SOC_ER_CELLDATA_CHUNK_WORDS));
    }

    slice4 = 0;
    slice = 0;
    while (slice4 < (SOC_ER_CELLDATA_SLICE4S - 1)) {
        slice4_data[slice4][8] = ((slice_data[slice][2] & 0xff) << 24) | 
            ((slice_data[slice][1] >> 8) & 0xffffff);
        slice4_data[slice4][7] = ((slice_data[slice][1] & 0xff) << 24) | 
            ((slice_data[slice][0] >> 8) & 0xffffff);
        slice4_data[slice4][6] = ((slice_data[slice][0] & 0xff) << 24);
        slice++;

        slice4_data[slice4][6] |= ((slice_data[slice][2] & 0xff) << 16) | 
            ((slice_data[slice][1] >> 16) & 0xffff);
        slice4_data[slice4][5] = ((slice_data[slice][1] & 0xffff) << 16) | 
            ((slice_data[slice][0] >> 16) & 0xffff);
        slice4_data[slice4][4] = ((slice_data[slice][0] & 0xffff) << 16);
        slice++;

        slice4_data[slice4][4] |= ((slice_data[slice][2] & 0xff) << 8) | 
            ((slice_data[slice][1] >> 24) & 0xff);
        slice4_data[slice4][3] = ((slice_data[slice][1] & 0xffffff) << 8) | 
            ((slice_data[slice][0] >> 24) & 0xff);
        slice4_data[slice4][2] = ((slice_data[slice][0] & 0xffffff) << 8);
        slice++;

        slice4_data[slice4][2] |= (slice_data[slice][2] & 0xff);
        slice4_data[slice4][1] = slice_data[slice][1] ;
        slice4_data[slice4][0] = slice_data[slice][0];
        slice++;
        slice4++;
    }

    slice4_data[slice4][8] = ((slice_data[slice][2] & 0xff) << 24) | 
        ((slice_data[slice][1] >> 8) & 0xffffff);

    chunk = 0;
    chunk_word = SOC_ER_CELLDATA_CHUNK_WORDS - 1;
    slice4 = 0;
    slice4_word = SOC_ER_CELLDATA_SLICE4_WORDS - 1;
    while (chunk < SOC_ER_CELLDATA_CHUNKS) {
        chunk_data[chunk][chunk_word] = slice4_data[slice4][slice4_word];
        if ((--slice4_word) < 0) {
            slice4_word = SOC_ER_CELLDATA_SLICE4_WORDS - 1;
            slice4++;
        }
        if ((--chunk_word) < 0) {
            chunk_word = SOC_ER_CELLDATA_CHUNK_WORDS - 1;
            chunk++;
        }
    }
}

int
soc_er_celldata_chunk_write(int unit, soc_mem_t mem,
                            int index, soc_er_cd_chunk_t chunk_data)
{
    soc_mem_t          write_mem;
    soc_reg_t          cmd_reg;
    uint32             cmd_data;
    int                chunk;

    if (mem == C0_CELLm) {
        write_mem = C0_CPU_WQm;
        cmd_reg = MCU_CHN0_REQ_CMDr;
    } else if (mem == C1_CELLm) {
        write_mem = C1_CPU_WQm;
        cmd_reg = MCU_CHN1_REQ_CMDr;
    } else {
        return SOC_E_PARAM;
    }

    /* Load up write buffer once */
    for (chunk = 0; chunk < SOC_ER_CELLDATA_CHUNKS; chunk++) {
        /* Write mem */
        SOC_IF_ERROR_RETURN
            (soc_mem_write(unit, write_mem, MEM_BLOCK_ANY,
                           chunk, chunk_data[chunk]));
    }

    if (index >= 0) {
        cmd_data = 0;
        /* Write data code */
        soc_reg_field_set(unit, cmd_reg, &cmd_data, COMMANDf, 6);
        soc_reg_field_set(unit, cmd_reg, &cmd_data, DDR_ADDRf, index);
        SOC_IF_ERROR_RETURN
            (soc_reg32_write(unit,soc_reg_addr(unit, cmd_reg,
                                               REG_PORT_ANY, 0), cmd_data));
    } /* else, just wanted to load write buffer */

    return SOC_E_NONE;
}

int
soc_er_celldata_columns_test(int unit, soc_mem_t mem, int index,
                             int do_write, int do_read,
                             soc_er_cd_columns_t columns_data,
                             soc_er_cd_columns_t columns_result,
                             soc_er_cd_columns_t columns_diff)
{
    soc_er_cd_chunk_t chunk_data = NULL;
    soc_er_cd_slice_t slice_data = NULL;
    int slice, column, rv;

    rv = soc_er_dual_dimensional_array_alloc(&chunk_data, 
                                             SOC_ER_CELLDATA_CHUNKS,
                                             SOC_MAX_MEM_WORDS);
    if (SOC_FAILURE(rv)) {
        goto done;
    }
    rv = soc_er_dual_dimensional_array_alloc(&slice_data, 
                                             SOC_ER_CELLDATA_SLICES,
                                             SOC_ER_CELLDATA_SLICE_WORDS);
    if (SOC_FAILURE(rv)) {
        goto done;
    }

    if (do_write) {
        soc_er_celldata_columns_to_slice(columns_data, slice_data);
        soc_er_celldata_slice_to_chunk(slice_data, chunk_data);
        rv = soc_er_celldata_chunk_write(unit, mem, index, chunk_data);
        if (SOC_FAILURE(rv)) {
            goto done;
        }
    }

    if (!do_read) {
        for (slice = 0; slice < SOC_ER_CELLDATA_SLICES; slice++) {
            for (column = 0; column < SOC_ER_CELLDATA_SLICE_COLUMNS;
                 column++) {
                columns_diff[slice][column] = 0;
            }
        }
        rv = SOC_E_NONE;
        goto done;
    }

    /* We wrote it, now read it back */
    rv = soc_er_celldata_chunk_read(unit, mem, index, chunk_data);
    if (SOC_FAILURE(rv)) {
        goto done;
    }
    soc_er_celldata_chunk_to_slice(chunk_data, slice_data);
    soc_er_celldata_slice_to_columns(slice_data, columns_result);

    for (slice = 0; slice < SOC_ER_CELLDATA_SLICES; slice++) {
        for (column = 0; column < SOC_ER_CELLDATA_SLICE_COLUMNS; column++) {
            columns_diff[slice][column] =
                columns_result[slice][column] ^ columns_data[slice][column];
        }
    }

    columns_data[SOC_ER_CELLDATA_SLICES - 1][0] = 0;
    columns_data[SOC_ER_CELLDATA_SLICES - 1][1] = 0;
    columns_data[SOC_ER_CELLDATA_SLICES - 1][2] &=
        SOC_ER_CELLDATA_DIFF_S60_C2_MASK;

done:

    if (NULL != chunk_data) {
        soc_er_dual_dimensional_array_free(chunk_data, SOC_ER_CELLDATA_CHUNKS); 
    }
    if (NULL != slice_data) {
        soc_er_dual_dimensional_array_free(slice_data, SOC_ER_CELLDATA_SLICES); 
    }

    return rv;
}

int
soc_er_num_cells(int unit, int *num_cells)
{
    uint32     cellptrs;
    int        fifo_cnt, block_cnt, ptr_cnt;

    if (SAL_BOOT_PLISIM || (soc_property_get(unit, spn_BYPASS_MCU, 0) != 0)) {
        *num_cells = 8192;
        return SOC_E_NONE;
    }

    *num_cells = 0;

    /*
     * Add up free cells from channel 0.
     */
    SOC_IF_ERROR_RETURN
        (READ_FREE_CELLPTRS_CH0r(unit, &cellptrs));

    fifo_cnt = soc_reg_field_get(unit, FREE_CELLPTRS_CH0r, 
                                 cellptrs, FIFOCOUNTf);
    block_cnt = soc_reg_field_get(unit, FREE_CELLPTRS_CH0r, 
                                  cellptrs, BLOCKCOUNTf);
    ptr_cnt = soc_reg_field_get(unit, FREE_CELLPTRS_CH0r, 
                                cellptrs, PTRCOUNTf);

    *num_cells = (4 * ptr_cnt) + (117 * block_cnt) + fifo_cnt;

    /*
     * Add up free cells from channel 1.
     */
    SOC_IF_ERROR_RETURN
        (READ_FREE_CELLPTRS_CH1r(unit, &cellptrs));

    fifo_cnt = soc_reg_field_get(unit, FREE_CELLPTRS_CH1r, 
                                 cellptrs, FIFOCOUNTf);
    block_cnt = soc_reg_field_get(unit, FREE_CELLPTRS_CH1r, 
                                  cellptrs, BLOCKCOUNTf);
    ptr_cnt = soc_reg_field_get(unit, FREE_CELLPTRS_CH1r, 
                                cellptrs, PTRCOUNTf);

    *num_cells += (4 * ptr_cnt) + (117 * block_cnt) + fifo_cnt;

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_easyrider_external_l3_enable
 * Purpose:
 *      For units with external SRAM search memory,
 *      this function is called to dynamically switch from
 *      using the external memory for L2 entries to using the
 *      external memory for L3 DEFIP.
 * Parameters:
 *      unit - XGS unit number
 * Returns:
 *      SOC_E_XXX;
 * Notes:
 *      After this function is called, it is necessary to 
 *      re-initialize the L3 module with bcm_l3_init().
 *      Side effects are that any previous L3 configuration 
 *      will be lost. Also, any L2 entries that were added
 *      in external memory will also be lost.
 */

int
soc_easyrider_external_l3_enable(int unit)
{
    soc_persist_t       *sop;
    sop_memcfg_er_t     *mcer;
    soc_timeout_t       to;
    uint32              rval;
    int                 seer_init_usec;

    sop = SOC_PERSIST(unit);
    mcer = &(sop->er_memcfg);

    /* Verify that we are currently configured for external L2 
     * before switching to external L3 mode.
     */
    if (mcer->ext_table_cfg == ER_SEER_CFG_LPM_256_EXT) {
        /* Already in external L3 mode */
        return SOC_E_NONE;
    } else if (mcer->ext_table_cfg != ER_SEER_CFG_L2_512_EXT) {
        /* Not in L2 external mode */
        return SOC_E_CONFIG;
    }
    mcer->ext_table_cfg = ER_SEER_CFG_LPM_256_EXT;

    /* Disable purge until external memory is re-initialized */
    SOC_IF_ERROR_RETURN(READ_L2_ISr(unit, &rval));
    soc_reg_field_set(unit, L2_ISr, &rval, PRG_ENf, 0);
    SOC_IF_ERROR_RETURN(WRITE_L2_ISr(unit, rval));

    /* Temporarily disable any parity error checks */
    rval = 0xffffffff;
    SOC_IF_ERROR_RETURN(WRITE_PAR_ERR_MASK_BSEr(unit, rval));
    SOC_IF_ERROR_RETURN(WRITE_PAR_ERR_MASK_CSEr(unit, rval));
    SOC_IF_ERROR_RETURN(WRITE_PAR_ERR_MASK_HSEr(unit,rval));

    /* Adjust memory size information */
    sop->memState[L2_ENTRY_EXTERNALm].index_max = -1;
    sop->memState[L3_DEFIP_ALGm].index_max = ER_SEER_512K - 1;
    sop->memState[NEXT_HOP_INTm].index_max = -1;
    sop->memState[NEXT_HOP_EXTm].index_max = ER_SEER_16K - 1;
    SOC_CONTROL(unit)->l2x_external = 0;

    /* External SEER table init */
    SOC_IF_ERROR_RETURN(READ_SEER_CONFIGr(unit, &rval));
    soc_reg_field_set(unit, SEER_CONFIGr, &rval, EXT_TABLE_CONFIGf,
                      mcer->ext_table_cfg);
    SOC_IF_ERROR_RETURN(WRITE_SEER_CONFIGr(unit, rval));

    if (!SAL_BOOT_PLISIM) {
        /* Set timeout = 50 ms */
        seer_init_usec = 
            soc_property_get(unit, spn_SEER_INIT_TIMEOUT_USEC, 50000);

        soc_timeout_init(&to, seer_init_usec, 0);
        /* Wait for SEER BSE memory initialization done. */
        do {
            SOC_IF_ERROR_RETURN(READ_STATUS_BSEr(unit, &rval));
            if (soc_reg_field_get(unit, STATUS_BSEr, rval, MEMINIT_DONEf)) {
                break;
            }
            if (soc_timeout_check(&to)) {
                soc_cm_debug(DK_WARN, "unit %d : STATUS_BSE timeout\n", unit);
                break;
            }
        } while (TRUE);

        /* Wait for SEER CSE memory initialization done. */
        do {
            SOC_IF_ERROR_RETURN(READ_STATUS_CSEr(unit, &rval));
            if (soc_reg_field_get(unit, STATUS_CSEr, rval, MEMINIT_DONEf)) {
                break;
            }
            if (soc_timeout_check(&to)) {
                soc_cm_debug(DK_WARN, "unit %d : STATUS_CSE timeout\n", unit);
                break;
            }
        } while (TRUE);

        /* Wait for SEER HSE memory initialization done. */
        do {
            SOC_IF_ERROR_RETURN(READ_STATUS_HSEr(unit, &rval));
            if (soc_reg_field_get(unit, STATUS_HSEr, rval, MEMINIT_DONEf)) {
                break;
            }
            if (soc_timeout_check(&to)) {
                soc_cm_debug(DK_WARN, "unit %d : STATUS_HSE timeout\n", unit);
                break;
            }
        } while (TRUE);
    }

    /* Re-initialize the external memory interface */
    SOC_IF_ERROR_RETURN(soc_easyrider_external_init(unit));

    /* Re-enable L2 purging */
    SOC_IF_ERROR_RETURN(READ_L2_ISr(unit, &rval));
    soc_reg_field_set(unit, L2_ISr, &rval, PRG_ENf, 1);
    SOC_IF_ERROR_RETURN(WRITE_L2_ISr(unit, rval));

    /* Parity enable after memory init */
    /* Only enable for memories active in this configuration */
    rval = 0;
    if (!soc_feature(unit, soc_feature_l3defip_parity)) {
        rval |= SOC_ER_PARITY_BSE_L3_DEFIP_ALG;
    }
    SOC_IF_ERROR_RETURN(WRITE_PAR_ERR_MASK_BSEr(unit, rval));

    rval = 0;
    /* No FP in external TCAM, block all parity from external FP tables */
    rval |= SOC_ER_PARITY_CSE_FP_EXTERNAL_TCAM;
    rval |= SOC_ER_PARITY_CSE_FP_POLICY_TABLE_EXT;
    rval |= SOC_ER_PARITY_CSE_FP_METER_TABLE_EXT;
    rval |= SOC_ER_PARITY_CSE_FP_COUNTER_EXT;

    rval |= SOC_ER_PARITY_CSE_NEXT_HOP_INT;
    rval |= SOC_ER_PARITY_CSE_L3_DEFIP_DATA;
    SOC_IF_ERROR_RETURN(WRITE_PAR_ERR_MASK_CSEr(unit, rval));

    rval = 0;
    if (!soc_feature(unit, soc_feature_ipmc_grp_parity)) {
        rval |= SOC_ER_PARITY_HSE_IPMC_GROUP_V4;
    }
    SOC_IF_ERROR_RETURN(WRITE_PAR_ERR_MASK_HSEr(unit,rval));

    /* Clear all status in SEER (Clear on read) */
    SOC_IF_ERROR_RETURN(READ_PAR_ERR_STATUS_BSEr(unit, &rval));
    SOC_IF_ERROR_RETURN(READ_PAR_ERR_STATUS_CSEr(unit, &rval));
    SOC_IF_ERROR_RETURN(READ_PAR_ERR_STATUS_HSEr(unit, &rval));

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_easyrider_is_external_l3_enabled
 * Purpose:
 *      For units with external SRAM search memory, this
 *      function is called to determine whether or not the 
 *      external memory is currently used for L3 DEFIP.
 * Parameters:
 *      unit - XGS unit number
 * Returns:
 *      TRUE - if external memory is used for L3 DEFIP
 *      FALSE - if external memory is not used for L3 DEFIP
 */

int
soc_easyrider_is_external_l3_enabled(int unit)
{
    soc_persist_t       *sop;
    sop_memcfg_er_t     *mcer;

    sop = SOC_PERSIST(unit);
    mcer = &(sop->er_memcfg);

    return (mcer->ext_table_cfg == ER_SEER_CFG_LPM_256_EXT);
}


/*
 * Function:
 *      soc_er_celldata_slice_alloc
 * Purpose:
 *      Allocates memory for dual dimensional array 
 * Parameters:
 *      ptr         - pointer to an array to allocate
 *      raw_size    - number of raws to allocate
 *      col_size    - number of columns to allocate
 * Returns:
 *      SOC_E_NONE      - Success
 *      SOC_E_PARAM     - Invalid input    
 *      SOC_E_MEMORY    - not enough memory
 */
int 
soc_er_dual_dimensional_array_alloc(uint32 ***ptr, int raw_size, int col_size)
{
    int mem_size, i;
    uint32  **l_ptr;

    if ((raw_size <= 0) || (col_size <= 0)){
        return SOC_E_PARAM;
    }
    mem_size = sizeof(uint32 *) * raw_size;
    l_ptr = sal_alloc(mem_size, "raws");
    if (NULL == l_ptr) {
        return SOC_E_MEMORY;
    }
    sal_memset(l_ptr, 0, mem_size);
    mem_size = sizeof(uint32) * col_size;
    for (i = 0; i < raw_size; i++) {
        l_ptr[i] = sal_alloc(mem_size, "columns");
        if (NULL == l_ptr[i]) {
            if (i) {
                SOC_IF_ERROR_RETURN(
                    soc_er_dual_dimensional_array_free(l_ptr, i)); 
            } else {
                sal_free(l_ptr);
            }
            return SOC_E_MEMORY;
        }
        sal_memset(l_ptr[i], 0, mem_size);
    }

    *ptr = l_ptr;
    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_er_dual_dimensional_array_free
 * Purpose:
 *      Free memory of dual dimensional array 
 * Parameters:
 *      ptr         - pointer to an allocated array
 *      raw_size    - number of allocated raws
 * Returns:
 *      SOC_E_NONE      - Success
 *      SOC_E_PARAM     - Invalid input    
 */
int 
soc_er_dual_dimensional_array_free(uint32 **ptr, int raw_size)
{
    int i;

    if (raw_size <= 0) {
        return SOC_E_PARAM;
    }
    for (i = raw_size -1; i >= 0; i--) {
        sal_free(ptr[i]);
    }

    sal_free(ptr);

    return SOC_E_NONE;
}

#else   /* !BCM_EASYRIDER_SUPPORT */
int
soc_er_num_cells(int unit, int *num_cells)
{
    *num_cells = 0;
    return SOC_E_NONE;
}

#endif /* BCM_EASYRIDER_SUPPORT */
