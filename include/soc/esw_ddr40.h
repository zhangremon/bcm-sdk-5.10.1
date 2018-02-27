/*
 * $Id: esw_ddr40.h 1.5.2.4 Broadcom SDK $
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
 * DDR3 Memory support
 */
     
#ifndef _ESW_DDR40_H__
#define _ESW_DDR40_H__

#ifdef BCM_DDR3_SUPPORT

/**
 * ddr40 register access
 * m = memory, c = core, r = register, f = field, d = data.
 */

#define DDR40_REG_READ(_unit, _pc, flags, _reg_addr, _val) \
            soc_ddr40_phy_reg_ci_read((_unit), (_pc), (_reg_addr), (_val))
#define DDR40_REG_WRITE(_unit, _pc, _flags, _reg_addr, _val) \
            soc_ddr40_phy_reg_ci_write((_unit), (_pc), (_reg_addr), (_val))
#define DDR40_REG_MODIFY(_unit, _pc, _flags, _reg_addr, _val, _mask) \
            soc_ddr40_phy_reg_ci_modify((_unit), (_pc), (_reg_addr), (_val), (_mask))
#define DDR40_GET_FIELD(m,c,r,f) \
            GET_FIELD(m,c,r,f)
#define DDR40_SET_FIELD(m,c,r,f,d) \
            SET_FIELD(m,c,r,f,d)


/* Conveniece Macros - recheck when ddr40.h changes */
#define DDR40_PHY_ADDR_CTL_MIN (0x0)
#define DDR40_PHY_ADDR_CTL_MAX (0x00c0)
#define DDR40_PHY_BYTE_LANE0_ADDR_MIN (0x0200)
#define DDR40_PHY_BYTE_LANE0_ADDR_MAX (0x03ac)
#define DDR40_PHY_BYTE_LANE1_ADDR_MIN (0x0400)
#define DDR40_PHY_BYTE_LANE1_ADDR_MAX (0x05ac)

#define READ_DDR40_PHY_WORD_LANE_x_VDL_OVRIDE_BYTEy_BITz_R_Pr(_x, _y, _z, _unit, _pc, _val) \
        DDR40_REG_READ((_unit), (_pc), 0x00, (0x00000234+(_x*0x200)+(_y*0xa0)+(_z*8)), (_val))

#define WRITE_DDR40_PHY_WORD_LANE_x_VDL_OVRIDE_BYTEy_BITz_R_Pr(_x, _y, _z, _unit, _pc, _val) \
        DDR40_REG_WRITE((_unit), (_pc), 0x00, (0x00000234+(_x*0x200)+(_y*0xa0)+(_z*8)), (_val))

#define WRITE_DDR40_PHY_WORD_LANE_x_VDL_OVRIDE_BYTEy_BITz_R_Nr(_x, _y, _z, _unit, _pc, _val) \
        DDR40_REG_WRITE((_unit), (_pc), 0x00, (0x00000238+(_x*0x200)+(_y*0xa0)+(_z*8)), (_val))

#define READ_DDR40_PHY_WORD_LANE_x_VDL_OVRIDE_BYTEy_BITz_Wr(_x, _y, _z, _unit,_pc,_val) \
        DDR40_REG_READ((_unit), (_pc), 0x00, (0x00000210+(_x*0x200)+(_y*0xa0)+(_z*4)), (_val))

#define WRITE_DDR40_PHY_WORD_LANE_x_VDL_OVRIDE_BYTEy_BITz_Wr(_x, _y, _z, _unit,_pc,_val) \
        DDR40_REG_WRITE((_unit), (_pc), 0x00, (0x00000210+(_x*0x200)+(_y*0xa0)+(_z*4)), (_val))


/* DDR Phy Registers Read/Write */
extern int soc_ddr40_phy_reg_ci_read(int unit, int ci, uint32 reg_addr, uint32 *reg_data);
extern int soc_ddr40_phy_reg_ci_write(int unit, int ci, uint32 reg_addr, uint32 reg_data);
extern int soc_ddr40_phy_reg_ci_modify(int unit, uint32 ci, uint32 reg_addr, uint32 data, uint32 mask);

/* DDR Registers Read/Write */
extern int soc_ddr40_read(int unit, int ci, uint32 addr, uint32 *pData0,
                uint32 *pData1, uint32 *pData2, uint32 *pData3,
                uint32 *pData4, uint32 *pData5, uint32 *pData6,
                uint32 *pData7);

extern int soc_ddr40_write(int unit, int ci, uint32 addr, uint32 uData0,
                uint32 uData1, uint32 uData2, uint32 uData3,
                uint32 uData4, uint32 uData5, uint32 uData6,
                uint32 uData7);

/* ------------------- */
/* PRE-TUNE Parameters */
/* ------------------- */
#define SHMOO_USE_PRETUNE 0
#define SHMOO_CI0_WL0_PRETUNE_RD_EN 13
#define SHMOO_CI0_WL0_PRETUNE_RD_DQS 47
#define SHMOO_CI0_WL0_PRETUNE_RD_DQ 37
#define SHMOO_CI0_WL0_PRETUNE_WR_DQ 30
#define SHMOO_CI0_WL1_PRETUNE_RD_EN (SHMOO_CI0_WL0_PRETUNE_RD_EN+8)
#define SHMOO_CI0_WL1_PRETUNE_RD_DQS (SHMOO_CI0_WL0_PRETUNE_RD_DQS-4)
#define SHMOO_CI0_WL1_PRETUNE_RD_DQ (SHMOO_CI0_WL0_PRETUNE_RD_DQ+8)
#define SHMOO_CI0_WL1_PRETUNE_WR_DQ (SHMOO_CI0_WL0_PRETUNE_WR_DQ+16)

#define SHMOO_CI0_WL0_PRETUNE_ADDR  35

#define SHMOO_CI2_WL0_PRETUNE_RD_EN (SHMOO_CI0_WL0_PRETUNE_RD_EN+2)
#define SHMOO_CI2_WL0_PRETUNE_RD_DQS SHMOO_CI0_WL0_PRETUNE_RD_DQS
#define SHMOO_CI2_WL0_PRETUNE_RD_DQ SHMOO_CI0_WL0_PRETUNE_RD_DQ
#define SHMOO_CI2_WL0_PRETUNE_WR_DQ SHMOO_CI0_WL0_PRETUNE_WR_DQ

/* --------------------- */
/* CI1 Offset Parameters */
/* --------------------- */
#define SHMOO_CI1_OFFSET_RD_EN         0
#define SHMOO_CI1_OFFSET_RD_DQS        0
#define SHMOO_CI1_OFFSET_RD_DQ         10+5
#define SHMOO_CI1_OFFSET_WR_DQ         10

/* for the name consistency sake */
#define SHMOO_CI0_WL1_OFFSET_RD_EN     SHMOO_CI1_OFFSET_RD_EN
#define SHMOO_CI0_WL1_OFFSET_RD_DQS    SHMOO_CI1_OFFSET_RD_DQS
#define SHMOO_CI0_WL1_OFFSET_RD_DQ     SHMOO_CI1_OFFSET_RD_DQ
#define SHMOO_CI0_WL1_OFFSET_WR_DQ     SHMOO_CI1_OFFSET_WR_DQ

/* Shmoo Functions */
#define SHMOO_INIT_VDL_RESULT 0
#define SHMOO_RD_EN 1
#define SHMOO_RD_DQ 2
#define SHMOO_WR_DQ 3
#define SHMOO_ADDRC 4
#define SHMOO_WR_DM 5 /* 13->9 */

/* Exported test functions */
#define SOC_DDR_TEST_SELF_1             1
#define SOC_DDR_TEST_SELF_2             2
#define SOC_DDR_TEST_SHMOO_PREP         3

typedef struct soc_ddr_test_param_s {
    int ci;
    int wl;
    int loop;
    int use_addr;
    uint32 dq_addr;
    int dq_burst;
    int dq_iter;
    int use_dq;
    uint32 dq_data0;
    uint32 dq_data0_alt;
} soc_ddr_test_param_t;

extern int
soc_ddr_test(int unit, int test_no, soc_ddr_test_param_t *tp);

extern int soc_ddr40_shmoo_get_init_step(int unit, int ci, int wl, int type, uint32 * init_step);
extern int soc_ddr40_shmoo_do(int unit, int ci, int wl, int type, int preset, int dqs_steps, uint32 * result, uint32 * init_step, int use_addr, uint32 dq_addr, int dq_burst, int dq_iter, int use_dq, uint32 dq_data0, uint32 dq_data_alt0);
extern int soc_ddr40_shmoo_calib(int unit, int ci, int type, uint32 * result, uint32 * init_step, uint32 * new_step);
extern int soc_ddr40_shmoo_plot(int type, int wl, uint32 * result, uint32 * step);
extern int soc_ddr40_shmoo_set_new_step(int unit, int ci, int wl, int type, uint32 * new_step);

#endif /* DDR3 Support */

#endif /* _ESW_DDR40_H__ */
