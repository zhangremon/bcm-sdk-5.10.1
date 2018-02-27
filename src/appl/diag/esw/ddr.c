/*
 * $Id: ddr.c 1.2.2.15 Broadcom SDK $
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
 */

#include <appl/diag/system.h>
#include <appl/diag/parse.h>
#include <bcm/error.h>
#include <sal/appl/sal.h>
#include <sal/appl/config.h>

#ifdef BCM_DDR3_SUPPORT
#include <soc/esw_ddr40.h>
#include <soc/phy/ddr40.h>

char cmd_ddr_mem_write_usage[] = "\n"
" DDRMemWrite ci<n> range=0xstart-[0xend] data=0xdata\n"
" DDRMemWrite ci0,ci1 range=0x0\n"
" DDRMemWrite ci2 range=0x0-0x100"
"\n";

cmd_result_t
cmd_ddr_mem_write(int unit, args_t *a)
{
    uint32 data_wr[8] = {0};
    uint32 data = 0;
    int ci = 0;
    int addr = 0;
    int bank = 0;
    int row = 0;
    int col = 0;
    int rv_stat = CMD_OK;
    int ret_code;
    char *c = NULL;
    char *range = NULL;
    char *lo = NULL;
    char *hi = NULL;
    soc_pbmp_t ci_pbm;
    parse_table_t pt;
    int start_addr;
    int end_addr;
    int i;
    int rv;
    int inc;

    if (((c = ARG_GET(a)) == NULL) || (parse_pbmp(unit, c, &ci_pbm) < 0)) {
        return CMD_USAGE;
    }  

    if((lo = range = ARG_GET(a)) != NULL) {
        if((hi = strchr(range, '-')) != NULL) {
            hi++;
        } else {
            hi = lo;
        }
    } else {
        return CMD_USAGE;
    }

    if (ARG_CNT(a)) {
        parse_table_init(0,&pt);
        parse_table_add(&pt,"data",PQ_INT,
                        0, &data, NULL);
        parse_table_add(&pt,"inc",PQ_INT,
                        0, &inc, NULL);
        if (!parseEndOk(a,&pt,&ret_code)) {
            return ret_code;
        }
    } else {
        return CMD_USAGE;
    }

    if (diag_parse_range(lo,hi,&start_addr,&end_addr,0,1<<22)) {
        printk("Invalid range. Valid range is : 0 - 0x%x\n",(1<<22));
        return CMD_FAIL;
    }

    /* for now 32B of data will be same */
    for(i=0;i<8;i++) {
        if (inc) {
            data_wr[i] = data+i;
        } else {
            data_wr[i] = data;
        }
    }

    SOC_PBMP_ITER(ci_pbm,ci) {
        printk("Writing ci%d DDR %s ..\n",ci,lo);
        for (addr=start_addr; addr <= end_addr; addr++) {
            bank = (addr & 0x7);
            col  = (addr >> 3) & 0x3f;
            row  = (addr >> 9) & 0x7fff;
            /* for debug */
            printk("Writing to ci%d,bank[%d],row[0x%04x],cols[0x%03x - 0x%03x] 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
                   ci,bank,row,col,col+0xf,data_wr[0],data_wr[1],data_wr[2],data_wr[3],data_wr[4],data_wr[5],data_wr[6],data_wr[7]);

            rv = soc_ddr40_write(unit, ci, addr,
                                 data_wr[0],data_wr[1],data_wr[2],
                                 data_wr[3],data_wr[4],data_wr[5],
                                 data_wr[6],data_wr[7]);
            if (rv != BCM_E_NONE) {
                rv_stat = CMD_FAIL;
            }
        }
    }

    return rv_stat;
}

char cmd_ddr_mem_read_usage[] = "\n"
" DDRMemRead ci<n> range=0xstart-[0xend]\n"
" DDRMemRead ci0,ci1 range=0x0\n"
" DDRMemRead ci2 range=0x0-0x100"
"\n";

cmd_result_t
cmd_ddr_mem_read(int unit, args_t *a)
{
    uint32 data_rd[8] = {0};
    int ci = 0;
    int addr = 0;
    int bank = 0;
    int row = 0;
    int col = 0;
    int rv_stat = CMD_OK;
    char *c = NULL;
    char *range = NULL;
    char *lo = NULL;
    char *hi = NULL;
    soc_pbmp_t ci_pbm;
    int start_addr;
    int end_addr;
    int rv;

    if (((c = ARG_GET(a)) == NULL) || (parse_pbmp(unit, c, &ci_pbm) < 0)) {
        return CMD_USAGE;
    }  

    if((lo = range = ARG_GET(a)) != NULL) {
        if((hi = strchr(range, '-')) != NULL) {
            hi++;
        } else {
            hi = lo;
        }
    } else {
        return CMD_USAGE;
    }

    if (diag_parse_range(lo,hi,&start_addr,&end_addr,0,1<<22)) {
        printk("Invalid range. Valid range is : 0 - 0x%x\n",(1<<22));
        return CMD_FAIL;
    }

    SOC_PBMP_ITER(ci_pbm,ci) {
        printk("Reading ci%d DDR %s ..\n",ci,lo);
        for (addr=start_addr; addr <= end_addr; addr++) {
            rv = soc_ddr40_read(unit, ci, addr,
                                &data_rd[0],&data_rd[1],&data_rd[2],
                                &data_rd[3],&data_rd[4],&data_rd[5],
                                &data_rd[6],&data_rd[7]);

            if (rv == BCM_E_NONE) {
                /* convert the dram addr to pla_addr, to show bank,row,col */
                bank = (addr & 0x7);
                col  = (addr >> 3) & 0x3f;
                row  = (addr >> 9) & 0x7fff;
                printk("ci%d,bank[%d],row[%d],col[0x%03x - 0x%03x] = 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
                       ci,bank,row,col,col+0xf,data_rd[0],data_rd[1],data_rd[2],data_rd[3],
                       data_rd[4],data_rd[5],data_rd[6],data_rd[7]);
            } else {
                rv_stat = CMD_FAIL;
            }
        }
    }
    return rv_stat;
}

char cmd_ddr_phy_read_usage[] = "\n"
" DDRPhyRead ci<n> [addr=<0xaddr>]\n"
" eg. DDRPhyRead ci0\n"
"     DDRPhyRead ci0,ci1 addr=0x0\n";


cmd_result_t
cmd_ddr_phy_read(int unit, args_t *a)
{
    uint32 data[4] = {0};
    int addr,address = -1;
    int reg = 0;
    int ci = 0;
    char *c = NULL;
    soc_pbmp_t ci_pbm;
    parse_table_t pt;
    int rv;

    if (((c = ARG_GET(a)) == NULL) || (parse_pbmp(unit, c, &ci_pbm) < 0)) {
        return CMD_USAGE;
    }

    if (ARG_CNT(a) > 0) {
        parse_table_init(0,&pt);
        parse_table_add(&pt,"addr",PQ_INT,
                        (void *) (0), &address, NULL);
        if (!parseEndOk(a,&pt,&rv)) {
            return rv;
        }
    }

    SOC_PBMP_ITER(ci_pbm,ci) {
        addr = address;
        if (addr == -1) {
            /* ADDR_CTL */
            printk("CI%d ( DDR40_PHY_ADDR_CTL )\n",ci);
            for (addr=DDR40_PHY_ADDR_CTL_MIN; addr <= DDR40_PHY_ADDR_CTL_MAX;) {
                for (reg=0;reg<4 && addr <= DDR40_PHY_ADDR_CTL_MAX;reg++) {
                    if(soc_ddr40_phy_reg_ci_read(unit, ci, addr, &data[reg])) {
                        printk("failed to read phy register 0x%04x\n", addr);
                    }
                    printk("    0x%04x: 0x%08x ",addr,data[reg]); 
                    addr += 4;
                }
                printk("\n");
            }

            /* BYTE_LANE0 */
            printk("CI%d ( DDR40_PHY_BYTE_LANE0 )\n",ci);
            for (addr=DDR40_PHY_BYTE_LANE0_ADDR_MIN; addr <= DDR40_PHY_BYTE_LANE0_ADDR_MAX;) {
                for (reg=0;reg<4 && addr <= DDR40_PHY_BYTE_LANE0_ADDR_MAX;reg++) {
                    if(soc_ddr40_phy_reg_ci_read(unit, ci, addr, &data[reg])) {
                        printk("failed to read phy register 0x%04x\n", addr);
                    }
                    printk("    0x%04x: 0x%08x ",addr,data[reg]); 
                    addr += 4;
                }
                printk("\n");
            }

            /* BYTE_LANE1 */
            printk("CI%d ( DDR40_PHY_BYTE_LANE1 )\n",ci);
            for (addr=DDR40_PHY_BYTE_LANE1_ADDR_MIN; addr <= DDR40_PHY_BYTE_LANE1_ADDR_MAX;) {
                for (reg=0;reg<4 && addr <= DDR40_PHY_BYTE_LANE1_ADDR_MAX;reg++) {
                    if(soc_ddr40_phy_reg_ci_read(unit, ci, addr, &data[reg])) {
                        printk("failed to read phy register 0x%04x\n", addr);
                    }
                    printk("    0x%04x: 0x%08x ",addr,data[reg]); 
                    addr += 4;
                }
                printk("\n");
            }
        } else {
            if(soc_ddr40_phy_reg_ci_read(unit, ci, addr, &data[0])) {
                printk("failed to read phy register 0x%04x\n", addr);
            }
            printk("    0x%04x: 0x%08x\n",addr,data[0]); 
        }
    }
    return CMD_OK;
}

char cmd_ddr_phy_write_usage[] = "\n"
" DDRPhyWrite ci<n> addr=<0xaddr> data=<0xdata>\n"
" eg. DDRPhyWrite ci addr=0x0 data=0x55\n"
"     DDRPhyWrite ci0,ci2 addr=0x240 data=0xaa\n";


cmd_result_t
cmd_ddr_phy_write(int unit, args_t *a)
{
    uint32 data = 0;
    int ci = 0;
    int addr = 0;
    parse_table_t pt;
    int rv;
    char *c = NULL;
    soc_pbmp_t ci_pbm;

    if (((c = ARG_GET(a)) == NULL) || (parse_pbmp(unit, c, &ci_pbm) < 0)) {
        return CMD_USAGE;
    }  

    if (ARG_CNT(a) == 2) {
        parse_table_init(0,&pt);
        parse_table_add(&pt,"addr",PQ_INT,
                        (void *) (0), &addr, NULL);
        parse_table_add(&pt,"data",PQ_INT,
                        (void *) (0), &data, NULL);
        if (!parseEndOk(a,&pt,&rv)) {
            return rv;
        }
    } else {
        printk("Invalid number of args.\n");
        return CMD_USAGE;
    }

    SOC_PBMP_ITER(ci_pbm,ci) {
        if(soc_ddr40_phy_reg_ci_write(unit, ci, addr, data)) {
            printk("Writing 0x%08x to ci:%d addr=0x%08x failed.\n",
                   data,ci,addr);
            return CMD_FAIL;
        }
    }
    return CMD_OK;
}

char cmd_ddr_phy_tune_usage[] = "\n"
" DDRPhyTune ci<n> [Type=n] [Plot=true/false] [SaveConfig=true/false] \n"
#ifndef COMPILER_STRING_CONST_LIMIT
"       type : 0=SHMOO_INIT_VDL_RESULT 1=SHMOO_RD_DATA_DLY 2=SHMOO_RD_EN 3=SHMOO_RD_DQS 4=SHMOO_RD_DQ 5=SHMOO_WR_DQ 6=SHMOO_ADDR \n"
"              -1=All \n"
"   Defaults : Type-All , Plot=true , SaveConfig=false\n"
#endif
;

cmd_result_t
_ddr_phy_tune(int unit, int ci, int type, int preset, int dqs_steps, int only_wl1, int use_addr,uint32 dq_addr,int dq_burst, int dq_iter, int use_dq,uint32 dq_data0,uint32 dq_data0_alt, int isplot, int issave)
{
    int wl;     /* Word Lane */
    uint32 result[64];
    uint32 init_step[16];
    uint32 new_step[16];
    /* For Saving  */
    int i;
    uint32 val_rd_en=0, val_addrc=0;
    uint32 val_rd_dqs[8] = {0,0,0,0,0,0,0,0};
    uint32 val_wr_dq[8] = {0,0,0,0,0,0,0,0};
    soc_ddr_test_param_t tp;

    soc_cm_debug(DK_VERBOSE, "Tuning ci:%d Type %d %s Saving\n", ci, type, (issave) ? "With" : "Without");

    if (type == SHMOO_INIT_VDL_RESULT) {
        sal_memset(&tp, 0, sizeof(tp));
        tp.ci = ci;
        SOC_IF_ERROR_RETURN
            (soc_ddr_test(unit, SOC_DDR_TEST_SHMOO_PREP, &tp));
    }

    for ( wl = 0; wl < 2; wl++ ) {
        if (SOC_IS_KATANA(unit)) {
            if ((ci == 2) && (wl == 1)) {
                continue;
            }
            if ((ci == 0) && (wl == 0) && only_wl1>0) {
                continue;
            }
        }
        soc_ddr40_shmoo_get_init_step(unit, ci, wl, type, init_step);
        soc_ddr40_shmoo_do(unit,ci,wl,type,preset,dqs_steps, result,init_step, use_addr,dq_addr,dq_burst,dq_iter,use_dq,dq_data0,dq_data0_alt);
        soc_ddr40_shmoo_calib(unit,ci,type,result,init_step,new_step);
        soc_ddr40_shmoo_set_new_step(unit, ci, wl, type, new_step);
        if (isplot) {
            soc_ddr40_shmoo_plot(type, wl, result, new_step);
        }

        /* To Save Config */
        switch (type) {
        case SHMOO_RD_EN:
            /* DDR40_PHY_WORD_LANE_x_VDL_OVRIDE_BYTEy_BIT_RD_ENr (4 regs) */
            if (wl == 0) {
                val_rd_en |= ((new_step[0] & 0xff) << 0);
                val_rd_en |= ((new_step[1] & 0xff) << 8);
            } else {
                val_rd_en |= ((new_step[0] & 0xff) << 16);
                val_rd_en |= ((new_step[1] & 0xff) << 24);
            }
            break;
        case SHMOO_RD_DQ:
            /* DDR40_PHY_WORD_LANE_x_VDL_OVRIDE_BYTEy_BITz_R_P/Nr (32 regs) */
            for (i=0; i<4; i++) {
                val_rd_dqs[i+(wl*4)] |= ((new_step[(i*4)+0] & 0xff) << 0);
                val_rd_dqs[i+(wl*4)] |= ((new_step[(i*4)+1] & 0xff) << 8);
                val_rd_dqs[i+(wl*4)] |= ((new_step[(i*4)+2] & 0xff) << 16);
                val_rd_dqs[i+(wl*4)] |= ((new_step[(i*4)+3] & 0xff) << 24);
            }
            break;
        case SHMOO_WR_DQ:
            /* DDR40_PHY_WORD_LANE_x_VDL_OVRIDE_BYTEy_BITz_Wr (32 regs) */
            for (i=0; i<4; i++) {
                val_wr_dq[i+(wl*4)] |= ((new_step[(i*4)+0] & 0xff) << 0);
                val_wr_dq[i+(wl*4)] |= ((new_step[(i*4)+1] & 0xff) << 8);
                val_wr_dq[i+(wl*4)] |= ((new_step[(i*4)+2] & 0xff) << 16);
                val_wr_dq[i+(wl*4)] |= ((new_step[(i*4)+3] & 0xff) << 24);
            }
            break;
        case SHMOO_ADDRC:
            /* DDR40_PHY_CONTROL_REGS_VDL_OVRIDE_BIT_CTL  (1 reg) */
            if(wl == 0) {
                val_addrc |= ((new_step[0] & 0xff) << 0);
            } else {
                val_addrc |= ((new_step[0] & 0xff) << 8);
            }
            break;
        }
    }

#ifndef NO_SAL_APPL
    if (issave) {
        char name_str[32], val_str[12];
        switch (type) {
        case SHMOO_RD_EN:
            sal_sprintf(name_str, "%s_ci%d", spn_DDR3_TUNE_RD_EN, ci);
            sal_sprintf(val_str, "0x%08x", val_rd_en);
            if (sal_config_set(name_str, val_str) < 0) {
                return SOC_E_MEMORY;
            }
            break;
        case SHMOO_RD_DQ:
            sal_sprintf(name_str, "%s_ci%d", spn_DDR3_TUNE_WL0_RD_DQS_B0_0_3, ci);
            sal_sprintf(val_str, "0x%08x", val_rd_dqs[0]);
            if (sal_config_set(name_str, val_str) < 0) {
                return SOC_E_MEMORY;
            }
            sal_sprintf(name_str, "%s_ci%d", spn_DDR3_TUNE_WL0_RD_DQS_B0_4_7, ci);
            sal_sprintf(val_str, "0x%08x", val_rd_dqs[1]);
            if (sal_config_set(name_str, val_str) < 0) {
                return SOC_E_MEMORY;
            }
            sal_sprintf(name_str, "%s_ci%d", spn_DDR3_TUNE_WL0_RD_DQS_B1_0_3, ci);
            sal_sprintf(val_str, "0x%08x", val_rd_dqs[2]);
            if (sal_config_set(name_str, val_str) < 0) {
                return SOC_E_MEMORY;
            }
            sal_sprintf(name_str, "%s_ci%d", spn_DDR3_TUNE_WL0_RD_DQS_B1_4_7, ci);
            sal_sprintf(val_str, "0x%08x", val_rd_dqs[3]);
            if (sal_config_set(name_str, val_str) < 0) {
                return SOC_E_MEMORY;
            }
            sal_sprintf(name_str, "%s_ci%d", spn_DDR3_TUNE_WL1_RD_DQS_B0_0_3, ci);
            sal_sprintf(val_str, "0x%08x", val_rd_dqs[4]);
            if (sal_config_set(name_str, val_str) < 0) {
                return SOC_E_MEMORY;
            }
            sal_sprintf(name_str, "%s_ci%d", spn_DDR3_TUNE_WL1_RD_DQS_B0_4_7, ci);
            sal_sprintf(val_str, "0x%08x", val_rd_dqs[5]);
            if (sal_config_set(name_str, val_str) < 0) {
                return SOC_E_MEMORY;
            }
            sal_sprintf(name_str, "%s_ci%d", spn_DDR3_TUNE_WL1_RD_DQS_B1_0_3, ci);
            sal_sprintf(val_str, "0x%08x", val_rd_dqs[6]);
            if (sal_config_set(name_str, val_str) < 0) {
                return SOC_E_MEMORY;
            }
            sal_sprintf(name_str, "%s_ci%d", spn_DDR3_TUNE_WL1_RD_DQS_B1_4_7, ci);
            sal_sprintf(val_str, "0x%08x", val_rd_dqs[7]);
            if (sal_config_set(name_str, val_str) < 0) {
                return SOC_E_MEMORY;
            }
            break;
        case SHMOO_WR_DQ:
            sal_sprintf(name_str, "%s_ci%d", spn_DDR3_TUNE_WL0_WR_DQ_B0_0_3, ci);
            sal_sprintf(val_str, "0x%08x", val_wr_dq[0]);
            if (sal_config_set(name_str, val_str) < 0) {
                return SOC_E_MEMORY;
            }
            sal_sprintf(name_str, "%s_ci%d", spn_DDR3_TUNE_WL0_WR_DQ_B0_4_7, ci);
            sal_sprintf(val_str, "0x%08x", val_wr_dq[1]);
            if (sal_config_set(name_str, val_str) < 0) {
                return SOC_E_MEMORY;
            }
            sal_sprintf(name_str, "%s_ci%d", spn_DDR3_TUNE_WL0_WR_DQ_B1_0_3, ci);
            sal_sprintf(val_str, "0x%08x", val_wr_dq[2]);
            if (sal_config_set(name_str, val_str) < 0) {
                return SOC_E_MEMORY;
            }
            sal_sprintf(name_str, "%s_ci%d", spn_DDR3_TUNE_WL0_WR_DQ_B1_4_7, ci);
            sal_sprintf(val_str, "0x%08x", val_wr_dq[3]);
            if (sal_config_set(name_str, val_str) < 0) {
                return SOC_E_MEMORY;
            }
            sal_sprintf(name_str, "%s_ci%d", spn_DDR3_TUNE_WL1_WR_DQ_B0_0_3, ci);
            sal_sprintf(val_str, "0x%08x", val_wr_dq[4]);
            if (sal_config_set(name_str, val_str) < 0) {
                return SOC_E_MEMORY;
            }
            sal_sprintf(name_str, "%s_ci%d", spn_DDR3_TUNE_WL1_WR_DQ_B0_4_7, ci);
            sal_sprintf(val_str, "0x%08x", val_wr_dq[5]);
            if (sal_config_set(name_str, val_str) < 0) {
                return SOC_E_MEMORY;
            }
            sal_sprintf(name_str, "%s_ci%d", spn_DDR3_TUNE_WL1_WR_DQ_B1_0_3, ci);
            sal_sprintf(val_str, "0x%08x", val_wr_dq[6]);
            if (sal_config_set(name_str, val_str) < 0) {
                return SOC_E_MEMORY;
            }
            sal_sprintf(name_str, "%s_ci%d", spn_DDR3_TUNE_WL1_WR_DQ_B1_4_7, ci);
            sal_sprintf(val_str, "0x%08x", val_wr_dq[7]);
            if (sal_config_set(name_str, val_str) < 0) {
                return SOC_E_MEMORY;
            }
            break;
        case SHMOO_ADDRC:
            sal_sprintf(name_str, "%s_ci%d", spn_DDR3_TUNE_ADDRC, ci);
            sal_sprintf(val_str, "0x%08x", val_addrc);
            if (sal_config_set(name_str, val_str) < 0) {
                return SOC_E_MEMORY;
            }
            break;
        }

    }
#endif

    return CMD_OK;
}


cmd_result_t
cmd_ddr_phy_tune(int unit, args_t *a)
{
    int ci = 0;
    parse_table_t pt;
    int plot=1;
    int savecfg=0;
    int type=-1;
    int preset=0;
    int dqs_steps=0;
    int re_shmoo=0;
    int only_wl1=0;
    int self_test_only=0;
    int use_dq=0;
    int use_addr=0;
    int dq_burst=0;
    int dq_iter=0;
    uint32 dq_addr;
    uint32 dq_data0;
    uint32 dq_data0_alt;
    int rv, i, self_test_ci, self_test_wl;
    uint32 rval;
    char *c = NULL;
    soc_pbmp_t ci_pbm;
    soc_ddr_test_param_t tp;
    char *rstr;

    if (((c = ARG_GET(a)) == NULL) || (parse_pbmp(unit, c, &ci_pbm) < 0)) {
        return CMD_USAGE;
    }  

    if (ARG_CNT(a) > 0) {
        parse_table_init(0,&pt);
        parse_table_add(&pt, "Type", PQ_DFL|PQ_INT,
                        (void *)(-1), &type, NULL);
        parse_table_add(&pt, "Plot", PQ_BOOL|PQ_DFL,
                        0, &plot, NULL);
        parse_table_add(&pt, "SaveConfig", PQ_BOOL|PQ_DFL,
                        0, &savecfg, NULL);
        parse_table_add(&pt, "preset", PQ_DFL|PQ_INT,
                        (void *)(0), &preset, NULL);
        parse_table_add(&pt, "dqs_steps", PQ_DFL|PQ_INT,
                        (void *)(0), &dqs_steps, NULL);
        parse_table_add(&pt, "re_shmoo", PQ_DFL|PQ_INT,
                        (void *)(0), &re_shmoo, NULL);
        parse_table_add(&pt, "only_wl1", PQ_DFL|PQ_INT,
                        (void *)(0), &only_wl1, NULL);
        parse_table_add(&pt, "self_test_only", PQ_DFL|PQ_INT,
                        (void *)(0), &self_test_only, NULL);
        parse_table_add(&pt, "use_dq", PQ_DFL|PQ_INT,
                        (void *)(0), &use_dq, NULL);
        parse_table_add(&pt, "use_addr", PQ_DFL|PQ_INT,
                        (void *)(0), &use_addr, NULL);
        parse_table_add(&pt, "dq_addr", PQ_DFL|PQ_INT,
                        (void *)(0), &dq_addr, NULL);
        parse_table_add(&pt, "dq_burst", PQ_DFL|PQ_INT,
                        (void *)(0), &dq_burst, NULL);
        parse_table_add(&pt, "dq_iter", PQ_DFL|PQ_INT,
                        (void *)(0), &dq_iter, NULL);
        parse_table_add(&pt, "dq_data0", PQ_DFL|PQ_INT,
                        (void *)(0), &dq_data0, NULL);
        parse_table_add(&pt, "dq_data0_alt", PQ_DFL|PQ_INT,
                        (void *)(0), &dq_data0_alt, NULL);
        if (!parseEndOk(a,&pt,&rv)) {
            return rv;
        }
    }
    if(type >= SHMOO_WR_DM) { 
        printk("Invalid Type\n");
        return CMD_USAGE;
    }

    if (self_test_only) {
        /* Initialize fixed parameters */
        sal_memset(&tp, 0, sizeof(tp));
        tp.loop = 1;
        tp.use_addr = use_addr;
        tp.dq_addr = dq_addr;
        tp.dq_burst = dq_burst;
        tp.dq_iter = dq_iter;
        tp.use_dq = use_dq;
        tp.dq_data0 = dq_data0;
        tp.dq_data0_alt = dq_data0_alt;

        if (self_test_only > 1) {
            SOC_PBMP_ITER(ci_pbm, ci) {
                self_test_wl = (ci % 2) > 0;
                self_test_ci = ci - self_test_wl;
                tp.ci = self_test_ci;
                tp.wl = self_test_wl;
                rv = soc_ddr_test(unit, SOC_DDR_TEST_SELF_2, &tp);
                rstr = "PASSED";
                if (SOC_FAILURE(rv)) {
                    rstr = "FAILED";
                }
                soc_cm_print("RESULT of PAIR SELF-TEST on ci%0d(ci=%0d,wl=%0d) : %s \n",
                             ci, self_test_ci, self_test_wl, rstr);
                SOC_IF_ERROR_RETURN
                    (READ_CI_DDR_TESTr(unit, self_test_ci, &rval)); 
                soc_cm_print("ci%0d MODE=%0d RAM_TEST=%0d RAM_DONE=%0d RAM_TEST_FAIL=%0d\n",
                             self_test_ci,
                             soc_reg_field_get(unit, CI_DDR_TESTr, rval, MODEf),
                             soc_reg_field_get(unit, CI_DDR_TESTr, rval, RAM_TESTf),
                             soc_reg_field_get(unit, CI_DDR_TESTr, rval, RAM_DONEf),
                             soc_reg_field_get(unit, CI_DDR_TESTr, rval, RAM_TEST_FAILf)); 
                SOC_IF_ERROR_RETURN
                    (READ_CI_DDR_TESTr(unit, self_test_ci+1, &rval)); 
                soc_cm_print("ci%0d MODE=%0d RAM_TEST=%0d RAM_DONE=%0d RAM_TEST_FAIL=%0d\n",
                             self_test_ci+1,
                             soc_reg_field_get(unit, CI_DDR_TESTr, rval, MODEf),
                             soc_reg_field_get(unit, CI_DDR_TESTr, rval, RAM_TESTf),
                             soc_reg_field_get(unit, CI_DDR_TESTr, rval, RAM_DONEf),
                             soc_reg_field_get(unit, CI_DDR_TESTr, rval, RAM_TEST_FAILf)); 
            }
        } else {
            SOC_PBMP_ITER(ci_pbm,ci) {
                self_test_wl = (ci % 2) > 0;
                self_test_ci = ci - self_test_wl;
                tp.ci = self_test_ci;
                tp.wl = self_test_wl;
                rv = soc_ddr_test(unit, SOC_DDR_TEST_SELF_2, &tp);
                rstr = "PASSED";
                if (SOC_FAILURE(rv)) {
                    rstr = "FAILED";
                }
                soc_cm_print( "RESULT of SELF-TEST on ci%0d(ci=%0d,wl=%0d) : PASSED%s \n",
                              ci, self_test_ci, self_test_wl, rstr);
                SOC_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit, ci, &rval)); 
                soc_cm_print( "MODE=%0d RAM_TEST=%0d RAM_DONE=%0d RAM_TEST_FAIL=%0d\n"
                              ,soc_reg_field_get(unit, CI_DDR_TESTr, rval, MODEf)
                              ,soc_reg_field_get(unit, CI_DDR_TESTr, rval, RAM_TESTf)
                              ,soc_reg_field_get(unit, CI_DDR_TESTr, rval, RAM_DONEf)
                              ,soc_reg_field_get(unit, CI_DDR_TESTr, rval, RAM_TEST_FAILf)
                    ); 
            }
        }
    } else {
        /* self_test_only = 0 */
        SOC_PBMP_ITER(ci_pbm,ci) {
            if(type == -1) {
                for(i=re_shmoo; i<SHMOO_WR_DM; i++) {
                    _ddr_phy_tune(unit,ci,i,preset,dqs_steps,only_wl1, use_addr,dq_addr,dq_burst,dq_iter,use_dq,dq_data0,dq_data0_alt,plot,savecfg);
                    if (i==SHMOO_ADDRC) break;
                }
            } else {
                _ddr_phy_tune(unit,ci,type,preset,dqs_steps,only_wl1, use_addr,dq_addr,dq_burst,dq_iter,use_dq,dq_data0,dq_data0_alt,plot,savecfg);
            }
        }
    }
    return CMD_OK;
}

#endif
