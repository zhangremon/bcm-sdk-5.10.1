/*
 * $Id: er_tcam_type2.c 1.7 Broadcom SDK $
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
 * File:    er_tcam_type2.c
 * Purpose: Initialization routines for Type 2 A20K and A10K tcam
 */

#include <soc/error.h>
#include <soc/register.h>
#include <soc/debug.h>
#include <soc/drv.h>

#if defined(BCM_EASYRIDER_SUPPORT)
#include <soc/easyrider.h>
#endif /* BCM_EASYRIDER_SUPPORT */
#include <soc/er_cmdmem.h>
#include <soc/er_tcam.h>
#include <soc/tcam/tcamtype2.h>

#include <sal/appl/io.h>

#ifdef BCM_EASYRIDER_SUPPORT

#define TCAM_TYPE2_TIMEOUT_USEC    60000000

static int
tcam_type2_init_regs(int unit)
{
    uint32 regv;

    SOC_IF_ERROR_RETURN(READ_TCAM_TYPE2_T144_CONFIGr(unit, &regv));
    soc_reg_field_set(unit, TCAM_TYPE2_T144_CONFIGr, &regv, ADMIf, ACL_ADM0);
    soc_reg_field_set(unit, TCAM_TYPE2_T144_CONFIGr, &regv, CMPRf, ACL_CMPR0);
    soc_reg_field_set(unit, TCAM_TYPE2_T144_CONFIGr, &regv, GMRf, ACL_GMR0);
    soc_reg_field_set(unit, TCAM_TYPE2_T144_CONFIGr, &regv, SSRf, ACL_SSR0);
    SOC_IF_ERROR_RETURN(WRITE_TCAM_TYPE2_T144_CONFIGr(unit, regv));

    SOC_IF_ERROR_RETURN(READ_TCAM_TYPE2_T72_CONFIGr(unit, &regv));
    soc_reg_field_set(unit, TCAM_TYPE2_T72_CONFIGr, &regv, ADMIf, LPM_ADM1);
    soc_reg_field_set(unit, TCAM_TYPE2_T72_CONFIGr, &regv, CMPRf, LPM_CMPR1);
    soc_reg_field_set(unit, TCAM_TYPE2_T72_CONFIGr, &regv, GMRf, LPM_GMR1);
    soc_reg_field_set(unit, TCAM_TYPE2_T72_CONFIGr, &regv, GMR2f, LPM_GMR2);
    soc_reg_field_set(unit, TCAM_TYPE2_T72_CONFIGr, &regv, SSRf, LPM_SSR1);
    SOC_IF_ERROR_RETURN(WRITE_TCAM_TYPE2_T72_CONFIGr(unit, regv));

    SOC_IF_ERROR_RETURN(READ_TCAM_TYPE2_UD_CONFIGr(unit, &regv));
    soc_reg_field_set(unit, TCAM_TYPE2_UD_CONFIGr, &regv, CHIPIDf,
                      UDQ0_RW_NSE_CHIPID543);
    soc_reg_field_set(unit, TCAM_TYPE2_UD_CONFIGr, &regv, SSRf,
                      UDQ0_RW_NSE_SSR3);
    soc_reg_field_set(unit, TCAM_TYPE2_UD_CONFIGr, &regv, GMRf,
                      UDQ1_W_NSE_GMR3);
    SOC_IF_ERROR_RETURN(WRITE_TCAM_TYPE2_UD_CONFIGr(unit, regv));

    return SOC_E_NONE;
}

static int
tcam_type2_write_reg(int u, uint32 data, uint32 addr)
{
    uint32 tcam_addr_reg = A20K_IF_REG_DAR_ADR;
    uint32 tcam_data_reg = A20K_IF_REG_DWR_ADR;

    SOC_IF_ERROR_RETURN(soc_mem_tcam_op(u, &tcam_data_reg, &data, 1, TRUE));
    SOC_IF_ERROR_RETURN(soc_mem_tcam_op(u, &tcam_addr_reg, &addr, 1, TRUE));
    return SOC_E_NONE;
}

static int
tcam_type2_read_reg(int u, uint32 * data, uint32 addr)
{
    uint32 tcam_addr_reg = A20K_IF_REG_DAR_ADR;
    uint32 tcam_read_reg = A20K_IF_REG_DRR_ADR;
    soc_timeout_t       to;

    addr |= A20K_DAR_READ;
    SOC_IF_ERROR_RETURN(soc_mem_tcam_op(u, &tcam_addr_reg, &addr, 1, TRUE));

    soc_timeout_init(&to, TCAM_TYPE2_TIMEOUT_USEC, 0);
    do {
        SOC_IF_ERROR_RETURN(soc_mem_tcam_op(u, &tcam_addr_reg, data, 1, FALSE));
        if (soc_timeout_check(&to)) {
            soc_cm_debug(DK_WARN, "unit %d :TCAM read timeout\n", u);
            return SOC_E_TIMEOUT;
        }
    } while((*data & A20K_DRR_READY_MASK) != A20K_DRR_READY);

    SOC_IF_ERROR_RETURN(soc_mem_tcam_op(u, &tcam_read_reg, data, 1, FALSE));
    return SOC_E_NONE;
}

static int
tcam_type2_init_1(int unit, int cfg)
{
    uint32 rc;
    uint32 addr[4] = { TT2_NSE_WRITE72, TT2_NSE_WRITE72, TT2_NSE_WRITE72, TT2_NSE_WRITE72 };
    uint32 data[4];
    int nstart_acl = 0;
    int nstart_lpm = 0;
    soc_timeout_t       to;

    /* DCR soft reset */
    SOC_IF_ERROR_RETURN
        (tcam_type2_write_reg(unit, DCR_SOFTREN, A20K_SYS_REG_DCR_ADR));
    SOC_IF_ERROR_RETURN
        (tcam_type2_write_reg(unit, DCR_SOFTR, A20K_SYS_REG_DCR_ADR));

    /* Poll soft reset done */
    soc_timeout_init(&to, TCAM_TYPE2_TIMEOUT_USEC, 0);
    do {
        SOC_IF_ERROR_RETURN
            (tcam_type2_read_reg(unit, &rc, A20K_SYS_REG_DCR_ADR));
        soc_cm_debug(DK_SOCMEM, "Stage 1: %08x\n", rc);
        if (soc_timeout_check(&to)) {
            soc_cm_debug(DK_WARN, "unit %d :TCAM soft reset timeout\n", unit);
            return SOC_E_TIMEOUT;
        }
    } while((rc & DCR_SOFTR) != 0);

    /* DCR init */
    /*
     * Test clock output enabled
     * DQ bus even parity
     * DQ bus parity disabled
     * SRAM disabled
     */
    SOC_IF_ERROR_RETURN
        (tcam_type2_write_reg(unit, 0x00000000, A20K_SYS_REG_DCR_ADR));

    /* sys regs */
    SOC_IF_ERROR_RETURN
        (tcam_type2_write_reg(unit, 0x03, A20K_SYS_REG_HWCR_ADR));
    SOC_IF_ERROR_RETURN
        (tcam_type2_write_reg(unit, 0x05, A20K_SYS_REG_ICTL_ADR));  /* ICTL */
    SOC_IF_ERROR_RETURN
        (tcam_type2_write_reg(unit, 0x238000, A20K_SYS_REG_DLCR_ADR));  /* DLCR (SADREN == 0) */

    /* ADM0 */
    SOC_IF_ERROR_RETURN
        (tcam_type2_write_reg(unit, 0x00, A20K_SYS_REG_ADM_ADR1(0)));
    SOC_IF_ERROR_RETURN
        (tcam_type2_write_reg(unit,
            (1 << ADM_RINFO) | (A20K_NSIZE_144B << ADM_NSIZE) | nstart_acl,
            A20K_SYS_REG_ADM_ADR0(0)));

    /* ADM1 */
    SOC_IF_ERROR_RETURN
        (tcam_type2_write_reg(unit, 0x00, A20K_SYS_REG_ADM_ADR1(1)));
    SOC_IF_ERROR_RETURN
        (tcam_type2_write_reg(unit,
            (1 << ADM_RINFO) | (A20K_NSIZE_72B << ADM_NSIZE) | nstart_lpm,
            A20K_SYS_REG_ADM_ADR0(1)));

    /* nse regs */
    /* NSE COMMAND soft reset */
    data[0] = TT2_NSE_RW_REG_Q0(A20K_BCST_CHIPID, 0, A20K_NSE_REG_COMMAND_ADR);
    data[1] = 0;
    data[2] = NSE_COMMAND_EMODE | NSE_COMMAND_LRN | NSE_COMMAND_EN | NSE_COMMAND_BEN;
    data[3] = NSE_COMMAND_DEVE | NSE_COMMAND_SRST | NSE_COMMAND_TBLSZ_2;
    SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));

    /* Poll NSE soft reset done */
    soc_timeout_init(&to, TCAM_TYPE2_TIMEOUT_USEC, 0);
    do {
        uint32 nse[] = { TT2_NSE_FETCH72, TT2_NSE_FETCH72, TT2_NSE_FETCH72 };

        data[0] = TT2_NSE_RW_REG_Q0(A20K_CHIPID, 0, A20K_NSE_REG_COMMAND_ADR);
        nse[0] = TT2_NSE_READ72;
        SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, nse, data, 1, TRUE));

        nse[0] = TT2_NSE_FETCH72;
        SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, nse, data, 3, FALSE));
        soc_cm_debug(DK_SOCMEM, "%08x %08x %08x\n",
                     data[0], data[1], data[2]);
        if (soc_timeout_check(&to)) {
            soc_cm_debug(DK_WARN, "unit %d :TCAM soft reset timeout\n", unit);
            return SOC_E_TIMEOUT;
        }
    } while(!((data[0] == 0x80000000) && (data[1] == 0) && (data[2] == 0x08)));

    /* NSE COMMAND program */
    data[0] = TT2_NSE_RW_REG_Q0(A20K_CHIPID, 0, A20K_NSE_REG_COMMAND_ADR);
    data[1] = 0;
    data[2] = NSE_COMMAND_EMODE | NSE_COMMAND_LRN | NSE_COMMAND_EN | NSE_COMMAND_BEN;
    data[3] = NSE_COMMAND_DEVE | NSE_COMMAND_TBLSZ_2;

    switch (configs[cfg].tcams) {
        case 1:
            data[3] |= A20K_SET_LRAM_LDEV_MASK;
            SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));
            break;
        case 2:
            SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));

            data[0] = TT2_NSE_RW_REG_Q0(A10K1_CHIPID, 0, A20K_NSE_REG_COMMAND_ADR);
            data[3] |= A20K_SET_LRAM_LDEV_MASK;
            SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));
            break;
        case 4:
            SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));

            data[0] = TT2_NSE_RW_REG_Q0(A10K1_CHIPID, 0, A20K_NSE_REG_COMMAND_ADR);
            SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));

            data[0] = TT2_NSE_RW_REG_Q0(A10K2_CHIPID, 0, A20K_NSE_REG_COMMAND_ADR);
            SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));

            data[0] = TT2_NSE_RW_REG_Q0(A10K3_CHIPID, 0, A20K_NSE_REG_COMMAND_ADR);
            data[3] |= A20K_SET_LRAM_LDEV_MASK;
            SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));
            break;
        default:
            return SOC_E_UNAVAIL;
    }

    SOC_IF_ERROR_RETURN
        (tcam_type2_write_reg(unit, 0x238001, A20K_SYS_REG_DLCR_ADR));
    /* DLCR (SADREN == 0) */

    /* NSE HARDWARE */
    data[0] = TT2_NSE_RW_REG_Q0(A20K_BCST_CHIPID, 0, A20K_NSE_REG_HARDWARE_ADR);
    data[1] = 0x00;
    data[2] = 0x00;
    data[3] = 0x3fc;
    SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));

    return 0;
}

static int
tcam_type2_init_2(int unit, int cfg)
{
    int i;
    uint32 addr[4] = { TT2_NSE_WRITE72, TT2_NSE_WRITE72, TT2_NSE_WRITE72, TT2_NSE_WRITE72 };
    unsigned int data[] = { 0, 0, 0, 0 };

    /* CPR0-15 are identical for all configs:
     * - compare with equal value
     * - no minikey
     * - priority 0
     */
    for(i = 0; i < A20K_NSE_REG_CPR_CNT; i++) {
        data[0] = TT2_NSE_ADDR(A20K_BCST_CHIPID, 0, A20K_NSE_REG_CPR_ADR(i));
        SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));
    }

    /* BPR - same for all configs: all sub-blocks participate, all
     * priorities are 0
     */
    data[2] = 0xf0000000;
    for(i = 0; i < A20K_BLK_CNT; i++) {
        data[0] = TT2_NSE_ADDR(A20K_BCST_CHIPID, i, A20K_NSE_REG_BPRX_ADR);
        SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));
    }

    /* No need to program BPRAiX */

    /* BPAR - same for all configs: enable parity checking */
    data[2] = 0;
    data[3] = 0x80000000;
    for(i = 0; i < A20K_BLK_CNT; i++) {
        data[0] = TT2_NSE_ADDR(A20K_BCST_CHIPID, i, A20K_NSE_REG_BPARX_ADR);
        SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));
    }

    /* GMR0-15 - same for all configs */

    /* GMR0: 144b search in acl table */
    data[1] = 0xff;
    data[2] = 0xffffffff;
    data[3] = 0xffffffff;
        data[0] = TT2_NSE_ADDR(A20K_BCST_CHIPID, 0, A20K_NSE_REG_GMR_07_ADR1(0));
        SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));
        data[0] = TT2_NSE_ADDR(A20K_BCST_CHIPID, 0, A20K_NSE_REG_GMR_07_ADR0(0));
        SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));

    /* GMR1: 64b search in 72b lpm table */
    data[3] = 0xffffff03;
        data[0] = TT2_NSE_ADDR(A20K_BCST_CHIPID, 0, A20K_NSE_REG_GMR_07_ADR1(1));
        SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));
        data[0] = TT2_NSE_ADDR(A20K_BCST_CHIPID, 0, A20K_NSE_REG_GMR_07_ADR0(1));
        SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));

    /* GMR2: 72b search in 72b lpm table */
    data[3] = 0xffffffff;
        data[0] = TT2_NSE_ADDR(A20K_BCST_CHIPID, 0, A20K_NSE_REG_GMR_07_ADR1(2));
        SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));
        data[0] = TT2_NSE_ADDR(A20K_BCST_CHIPID, 0, A20K_NSE_REG_GMR_07_ADR0(2));
        SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));

    /* GMR3: read_writes to data mask array */
        data[0] = TT2_NSE_ADDR(A20K_BCST_CHIPID, 0, A20K_NSE_REG_GMR_07_ADR1(3));
        SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));
        data[0] = TT2_NSE_ADDR(A20K_BCST_CHIPID, 0, A20K_NSE_REG_GMR_07_ADR0(3));
        SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));

    /* All other GMRs (4-7 and 8-15) are disabled */
    data[1] = data[2] = data[3] = 0;
    for (i = 4; i < 8; i++) {
        data[0] = TT2_NSE_ADDR(A20K_BCST_CHIPID, 0, A20K_NSE_REG_GMR_07_ADR1(i));
        SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));
        data[0] = TT2_NSE_ADDR(A20K_BCST_CHIPID, 0, A20K_NSE_REG_GMR_07_ADR0(i));
        SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));
    }
    for(i = 0; i < A20K_NSE_REG_GMR_CNT; i++) {
        data[0] = TT2_NSE_ADDR(A20K_BCST_CHIPID, 0, A20K_NSE_REG_GMR_8F_ADR1(i));
        SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));
        data[0] = TT2_NSE_ADDR(A20K_BCST_CHIPID, 0, A20K_NSE_REG_GMR_8F_ADR0(i));
        SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));
    }
    
    return SOC_E_NONE;
}

static int
tcam_type2_init_config(int unit, int cfg)
{
    int i, acl, lpm;
    uint32 addr[4] = { TT2_NSE_WRITE72, TT2_NSE_WRITE72, TT2_NSE_WRITE72, TT2_NSE_WRITE72 };
    uint32 data[4] = { 0, 0, 0, 0 };

    if ((cfg < 0) || (cfg > ER_SEER_CFG_LPM_896_EXT)) {
        return SOC_E_PARAM;
    }

    if (configs[cfg].tcams != 1) { /* support one tcam only at the time */
        return SOC_E_UNAVAIL;
    }

    acl = configs[cfg].acl_entries * 2 / A20K_MAX_ENTRY_NUM_IN_BLK;
    lpm = configs[cfg].lpm_entries / A20K_MAX_ENTRY_NUM_IN_BLK;

    data[3] = 0x01; /* 144 bit table */
    for (i = 0; i < acl; i++) {
        data[0] = TT2_NSE_ADDR(A20K_CHIPID, i, A20K_NSE_REG_BMRX_ADR);
        SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));
    }

    data[3] = 0x00; /* 72 bit table */
    for (i = acl; i < acl + lpm; i++) {
        data[0] = TT2_NSE_ADDR(A20K_CHIPID, i, A20K_NSE_REG_BMRX_ADR);
        SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));
    }

    return SOC_E_NONE;
}

static int
tcam_type2_init_3(int unit)
{
    int i;
    uint32 data[] = { 0, 0, 0, 0 };
    uint32 addr[] = { TT2_NSE_WRITE72,  TT2_NSE_WRITE72,  TT2_NSE_WRITE72,  TT2_NSE_WRITE72 };

    data[1] = TT2_NSE_WR_ARRAY_Q1(1) | 0x0000;

    for (i = 0; i < A20K_MAX_ENTRY_NUM_IN_BLK; i++) {
        data[0] = TT2_NSE_WR_ARRAY_Q0(A20K_BCST_CHIPID, A20K_TARGET_NSE_DATA_ARRAY, i);
        SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));
    }

    data[1] = TT2_NSE_WR_ARRAY_Q1(1) | 0x00ff;
    data[2] = data[3] = 0xffffffff;

    for (i = 0; i < A20K_MAX_ENTRY_NUM_IN_BLK; i++) {
        data[0] = TT2_NSE_WR_ARRAY_Q0(A20K_BCST_CHIPID, A20K_TARGET_NSE_MASK_ARRAY, i);
        SOC_IF_ERROR_RETURN(soc_mem_tcam_op(unit, addr, data, 4, TRUE));
    }
    return SOC_E_NONE;
}

static int
tcam_type2_init_parity(int unit)
{
    /* Turn on parity reporting */
    SOC_IF_ERROR_RETURN
        (tcam_type2_write_reg(unit, 0x3f, A20K_SYS_REG_IEMR_ADR));
    SOC_IF_ERROR_RETURN
        (tcam_type2_write_reg(unit, 0xfff, A20K_SYS_REG_DEMR_ADR));

    return SOC_E_NONE;
}

int
er_tcam_init_type2(int u, int ext_table_config)
{
    SOC_IF_ERROR_RETURN(tcam_type2_init_regs(u));
    SOC_IF_ERROR_RETURN(tcam_type2_init_1(u, ext_table_config));
    SOC_IF_ERROR_RETURN(tcam_type2_init_2(u, ext_table_config));
    SOC_IF_ERROR_RETURN(tcam_type2_init_config(u, ext_table_config));
    SOC_IF_ERROR_RETURN(tcam_type2_init_3(u));
    SOC_IF_ERROR_RETURN(tcam_type2_init_parity(u));
    return SOC_E_NONE;
}

int
soc_tcam_read_type2_entry_72(int u, int chip, int index, int target,
        uint32 * data)
{
    uint32 addr[3];

    addr[0] = TT2_NSE_READ72;
    data[0] = TT2_NSE_RD_ARRAY_Q0(chip, target, index);
    SOC_IF_ERROR_RETURN(soc_mem_tcam_op(u, addr, data, 1, TRUE));

    addr[0] =
        addr[1] =
        addr[2] = TT2_NSE_FETCH72;
    SOC_IF_ERROR_RETURN(soc_mem_tcam_op(u, addr, &data[1], 3, FALSE));
    data[0] = 0;

    return SOC_E_NONE;
}

int
soc_er_tcam_type2_read_entry(int u, int cfg, int part, int index,
                             uint32 * mask, uint32 * data, int * valid)
{
    uint32 addr;
    int chip;

    switch(part) {
        case TCAM_PARTITION_RAW:
            addr = index;
            chip = addr / A20K_MAX_ENTRY_NUM_IN_BLK;
            addr = addr % A20K_MAX_ENTRY_NUM_IN_BLK;
            SOC_IF_ERROR_RETURN(soc_tcam_read_type2_entry_72(u, chip, addr,
                        A20K_TARGET_NSE_DATA_ARRAY, &data[0]));
            SOC_IF_ERROR_RETURN(soc_tcam_read_type2_entry_72(u, chip, addr,
                        A20K_TARGET_NSE_MASK_ARRAY, &mask[0]));
            *valid = data[1] & 0x80000000 ? 1 : 0;
            data[1] &= 0xff;
            mask[1] &= 0xff;
            break;
        case TCAM_PARTITION_LPM:
            if (index > configs[cfg].lpm_entries) {
                return SOC_E_PARAM;
            }
            addr = configs[cfg].acl_entries * 2 + index;
            chip = addr / A20K_MAX_ENTRY_NUM_IN_BLK;
            addr = addr % A20K_MAX_ENTRY_NUM_IN_BLK;
            SOC_IF_ERROR_RETURN(soc_tcam_read_type2_entry_72(u, chip, addr,
                        A20K_TARGET_NSE_DATA_ARRAY, &data[0]));
            SOC_IF_ERROR_RETURN(soc_tcam_read_type2_entry_72(u, chip, addr,
                        A20K_TARGET_NSE_MASK_ARRAY, &mask[0]));
            *valid = data[1] & 0x80000000 ? 1 : 0;
            data[1] &= 0xff;
            mask[1] &= 0xff;
            break;
        case TCAM_PARTITION_ACL:
            if (index > configs[cfg].acl_entries) {
                return SOC_E_PARAM;
            }
            addr = index * 2;
            chip = addr / A20K_MAX_ENTRY_NUM_IN_BLK;
            addr = addr % A20K_MAX_ENTRY_NUM_IN_BLK;
            SOC_IF_ERROR_RETURN(soc_tcam_read_type2_entry_72(u, chip, addr,
                        A20K_TARGET_NSE_DATA_ARRAY, &data[0]));
            SOC_IF_ERROR_RETURN(soc_tcam_read_type2_entry_72(u, chip, addr + 1,
                        A20K_TARGET_NSE_DATA_ARRAY, &data[4]));
            SOC_IF_ERROR_RETURN(soc_tcam_read_type2_entry_72(u, chip, addr,
                        A20K_TARGET_NSE_MASK_ARRAY, &mask[0]));
            SOC_IF_ERROR_RETURN(soc_tcam_read_type2_entry_72(u, chip, addr + 1,
                        A20K_TARGET_NSE_MASK_ARRAY, &mask[4]));
            *valid = data[1] & data[5] & 0x80000000 ? 1 : 0;
            data[1] &= 0xff;
            data[5] &= 0xff;
            mask[1] &= 0xff;
            mask[5] &= 0xff;
            break;
        default:
            return SOC_E_UNAVAIL;
    }

    return SOC_E_NONE;
}

int
soc_tcam_write_type2_entry_72(int u, int chip, int index, int target,
        uint32 * data)
{
    uint32 addr[] = { TT2_NSE_WRITE72,  TT2_NSE_WRITE72,  TT2_NSE_WRITE72,  TT2_NSE_WRITE72 };
    data[0] = TT2_NSE_WR_ARRAY_Q0(chip, target, index);
    data[1] |= TT2_NSE_WR_ARRAY_Q1(1);
    return soc_mem_tcam_op(u, addr, data, 4, TRUE);
}

int
soc_er_tcam_type2_write_entry(int u, int cfg, int part, int index,
                              uint32 * mask, uint32 * data)
{
    uint32 addr;
    int chip;

    switch(part) {
        case TCAM_PARTITION_RAW:
            addr = index;
            chip = addr / A20K_MAX_ENTRY_NUM_IN_BLK;
            addr = addr % A20K_MAX_ENTRY_NUM_IN_BLK;
            SOC_IF_ERROR_RETURN(soc_tcam_write_type2_entry_72(u, chip, addr,
                        A20K_TARGET_NSE_DATA_ARRAY, &data[0]));
            SOC_IF_ERROR_RETURN(soc_tcam_write_type2_entry_72(u, chip, addr,
                        A20K_TARGET_NSE_MASK_ARRAY, &mask[0]));
            break;

        case TCAM_PARTITION_LPM:
            if (index > configs[cfg].lpm_entries) {
                return SOC_E_PARAM;
            }
            addr = configs[cfg].acl_entries * 2 + index;
            chip = addr / A20K_MAX_ENTRY_NUM_IN_BLK;
            addr = addr % A20K_MAX_ENTRY_NUM_IN_BLK;
            if (data != NULL) {
                SOC_IF_ERROR_RETURN(soc_tcam_write_type2_entry_72(u, chip,
                            addr, A20K_TARGET_NSE_DATA_ARRAY, &data[0]));
            }
            if (mask != NULL) {
                SOC_IF_ERROR_RETURN(soc_tcam_write_type2_entry_72(u, chip,
                            addr, A20K_TARGET_NSE_MASK_ARRAY, &mask[0]));
            }
            break;
        case TCAM_PARTITION_ACL:
            /*
             * There's no native 144-bit write command; it is emulated as
             * two 72-bit writes at sequential addresses. The data are arranged
             * in a 32 byte array contiguously. Therefore, it is necessary to
             * move the top 72 bits into the top 16 bytes. See also
             * soc_er_tcam_type2_read_entry.
             */

            if (index > configs[cfg].acl_entries) {
                return SOC_E_PARAM;
            }
            addr = index * 2;
            chip = addr / A20K_MAX_ENTRY_NUM_IN_BLK;
            addr = addr % A20K_MAX_ENTRY_NUM_IN_BLK;

            if (data != NULL) {
                SOC_IF_ERROR_RETURN(soc_tcam_write_type2_entry_72(u, chip,
                            addr, A20K_TARGET_NSE_DATA_ARRAY, &data[0]));
                SOC_IF_ERROR_RETURN(soc_tcam_write_type2_entry_72(u, chip,
                            addr + 1, A20K_TARGET_NSE_DATA_ARRAY, &data[4]));
            }

            if (mask != NULL) {
                SOC_IF_ERROR_RETURN(soc_tcam_write_type2_entry_72(u, chip,
                            addr, A20K_TARGET_NSE_MASK_ARRAY, &mask[0]));
                SOC_IF_ERROR_RETURN(soc_tcam_write_type2_entry_72(u, chip,
                            addr + 1, A20K_TARGET_NSE_MASK_ARRAY, &mask[4]));
            }
            break;
        default:
            return SOC_E_UNAVAIL;
    }
    return SOC_E_NONE;
}

int
soc_er_tcam_type2_set_valid(int u, int cfg, int part, int index, int valid)
{
#if 1
/* Skip this for now:
 * 1. This code reads type1 tcam for data and mask nothing would come back
 * 2. This code then writes the incorrect data and incrorect mask (and valid
 *    bit) back to device
 * 3. The valid bit in the argument is not used by the write routine except
 *    in the valid case for 72-bit (by typo?).
 */
    return SOC_E_NONE;
#else
    uint32 data[8];
    uint32 mask[8];
    int v;
    SOC_IF_ERROR_RETURN
        (soc_er_tcam_type1_read_entry(u, cfg, part, index,
                mask, data, &v));
    switch(part) {
        case TCAM_PARTITION_RAW:
        case TCAM_PARTITION_LPM:
            if (valid) {
                data[2] |= 0x10000000;
            } else {
                data[3] &= ~0x10000000;
            }
            break;
        case TCAM_PARTITION_ACL:
            if (valid) {
                data[6] |= 0x10000000;
            } else {
                data[6] &= ~0x10000000;
            }
            break;
        default:
            return SOC_E_UNAVAIL;
    }
    return soc_er_tcam_type2_write_entry(u, cfg, part, index, mask, data);
#endif
}

int
soc_er_tcam_type2_parity_diagnose(unit)
{
    uint32 regval, context;

    SOC_IF_ERROR_RETURN
        (tcam_type2_read_reg(unit, &regval, A20K_SYS_REG_IERR_ADR));
    
    if (regval) {
        context = (regval >> SOC_ER_PARITY_TCAM_TYPE2_IERR_CONTEXT_MASK) &
            SOC_ER_PARITY_TCAM_TYPE2_IERR_CONTEXT_SHIFT;
        if (regval & SOC_ER_PARITY_TCAM_TYPE2_IERR_IBPE) {
            soc_cm_debug(DK_ERR,
                         "unit %d External TCAM interface bus parity error\n", unit);
        }
     
        if (regval & SOC_ER_PARITY_TCAM_TYPE2_IERR_CFOERR) {
            soc_cm_debug(DK_ERR,
                         "unit %d External TCAM context %d: command FIFO overflow error\n", unit, context);
        }
     
        if (regval & SOC_ER_PARITY_TCAM_TYPE2_IERR_RFOERR) {
            soc_cm_debug(DK_ERR,
                         "unit %d External TCAM context %d: result FIFO overflow error\n", unit, context);
        }
     
        if (regval & SOC_ER_PARITY_TCAM_TYPE2_IERR_DFOERR) {
            soc_cm_debug(DK_ERR,
                         "unit %d External TCAM context %d: data FIFO overflow error\n", unit, context);
        }
     
        if (regval & SOC_ER_PARITY_TCAM_TYPE2_IERR_CSERR) {
            soc_cm_debug(DK_ERR,
                         "unit %d External TCAM context %d: command sequence error\n", unit, context);
        }
     
        if (regval & SOC_ER_PARITY_TCAM_TYPE2_IERR_ICERR) {
            soc_cm_debug(DK_ERR,
                         "unit %d External TCAM context %d: invalid command error\n", unit, context);
        }

        /* Clear error status by writing 1's to clear the error bits*/
        SOC_IF_ERROR_RETURN
            (tcam_type2_read_reg(unit, &regval, A20K_SYS_REG_IECR_ADR));
    }

     
    SOC_IF_ERROR_RETURN
        (tcam_type2_read_reg(unit, &regval, A20K_SYS_REG_DERR_ADR));
    
    if (regval) {
        if (regval & SOC_ER_PARITY_TCAM_TYPE2_DERR_SRPE) {
            soc_cm_debug(DK_ERR,
                         "unit %d External TCAM SRAM read parity error\n", unit);
        }

        if (regval & SOC_ER_PARITY_TCAM_TYPE2_DERR_NRPE0) {
            soc_cm_debug(DK_ERR,
                         "unit %d External TCAM NSE read parity error 0\n", unit);
        }

        if (regval & SOC_ER_PARITY_TCAM_TYPE2_DERR_NRPE1) {
            soc_cm_debug(DK_ERR,
                         "unit %d External TCAM NSE read parity error 1\n", unit);
        }

        if (regval & SOC_ER_PARITY_TCAM_TYPE2_DERR_NWPE) {
            soc_cm_debug(DK_ERR,
                         "unit %d External TCAM NSE write parity error\n", unit);
        }

        if (regval & SOC_ER_PARITY_TCAM_TYPE2_DERR_NCPE) {
            soc_cm_debug(DK_ERR,
                         "unit %d External TCAM NSE core parity error\n", unit);
        }

        if (regval & SOC_ER_PARITY_TCAM_TYPE2_DERR_NFSE) {
            soc_cm_debug(DK_ERR,
                         "unit %d External TCAM NSE full status\n", unit);
        }

        if (regval & SOC_ER_PARITY_TCAM_TYPE2_DERR_NRTE) {
            soc_cm_debug(DK_ERR,
                         "unit %d External TCAM NSE read timeout error\n", unit);
        }

        if (regval & SOC_ER_PARITY_TCAM_TYPE2_DERR_SCE) {
            soc_cm_debug(DK_ERR,
                         "unit %d External TCAM secondary bus error\n", unit);
        }

        if (regval & SOC_ER_PARITY_TCAM_TYPE2_DERR_S0RPE0) {
            soc_cm_debug(DK_ERR,
                         "unit %d External TCAM secondary cascade 0 read parity error 0\n", unit);
        }

        if (regval & SOC_ER_PARITY_TCAM_TYPE2_DERR_S0RPE1) {
            soc_cm_debug(DK_ERR,
                         "unit %d External TCAM secondary cascade 0 read parity error 1\n", unit);
        }

        if (regval & SOC_ER_PARITY_TCAM_TYPE2_DERR_S1RPE0) {
            soc_cm_debug(DK_ERR,
                         "unit %d External TCAM secondary cascade 1 read parity error 0\n", unit);
        }

        if (regval & SOC_ER_PARITY_TCAM_TYPE2_DERR_S1RPE1) {
            soc_cm_debug(DK_ERR,
                         "unit %d External TCAM secondary cascade 1 read parity error 1\n", unit);
        }

        /* Clear error status by writing 1's to clear the error bits*/
        SOC_IF_ERROR_RETURN
            (tcam_type2_read_reg(unit, &regval, A20K_SYS_REG_DECR_ADR));
    }

    return SOC_E_NONE;
}

#endif
