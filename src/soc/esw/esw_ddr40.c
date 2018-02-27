/*
 * $Id: esw_ddr40.c 1.9.2.11 Broadcom SDK $
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

#include <sal/core/boot.h>
#include <sal/core/libc.h>
#include <shared/alloc.h>

#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/cm.h>
#ifdef BCM_CMICM_SUPPORT
#include <soc/cmicm.h>
#endif

#ifdef BCM_DDR3_SUPPORT
#include <soc/esw_ddr40.h>
#include <soc/phy/ddr40.h>

#define DDR_TIMEOUT_10mS 1000
#define NUM_DATA 8
#define SET_UNOVR_STEP(v) ( 0x20000 | ( (v) & 0x3F ) ) /* OVR_FORCE = OVR_EN = 1, OVR_STEP = v */
#define SET_OVR_STEP(v) ( 0x30000 | ( (v) & 0x3F ) ) /* OVR_FORCE = OVR_EN = 1, OVR_STEP = v */


/***********************************************************************
* Functions to Read / Write / Modify DDR Phy Registers
***********************************************************************/
int
soc_ddr40_phy_reg_ci_read(int unit, int ci, uint32 reg_addr, uint32 *reg_data)
{
    uint32 rval;
    soc_timeout_t to;

    rval = 0;
    soc_reg_field_set(unit, CI_DDR_PHY_REG_CTRLr, &rval, PHY_REG_ACKf, 1); /* write 1 to clear */
    soc_reg_field_set(unit, CI_DDR_PHY_REG_CTRLr, &rval, PHY_REG_ERR_ACKf, 1); /* write 1 to clear */
    soc_reg_field_set(unit, CI_DDR_PHY_REG_CTRLr, &rval, PHY_REG_ADDRf, reg_addr);
    soc_reg_field_set(unit, CI_DDR_PHY_REG_CTRLr, &rval, PHY_REG_RD_WR_Nf, 1);
    soc_reg_field_set(unit, CI_DDR_PHY_REG_CTRLr, &rval, PHY_REG_REQf, 1);
    SOC_IF_ERROR_RETURN(WRITE_CI_DDR_PHY_REG_CTRLr(unit, ci, rval));

    if (SAL_BOOT_SIMULATION) {
        soc_timeout_init(&to, 10000000, 0); /* 10 Sec */
    } else {
        soc_timeout_init(&to, 50000, 0); /* 50 mS */
    }
    do {
        SOC_IF_ERROR_RETURN(READ_CI_DDR_PHY_REG_CTRLr(unit, ci, &rval));
        if (soc_reg_field_get(unit, CI_DDR_PHY_REG_CTRLr, rval, PHY_REG_ACKf)) {
            break;
        }
        if (soc_reg_field_get(unit, CI_DDR_PHY_REG_CTRLr, rval, PHY_REG_ERR_ACKf)) {
            return SOC_E_FAIL;
        }
        if (soc_timeout_check(&to)) {
            return SOC_E_TIMEOUT;
        }
    } while (TRUE);
    SOC_IF_ERROR_RETURN(READ_CI_DDR_PHY_REG_DATAr(unit, ci, reg_data));
    return SOC_E_NONE;
}

/* DDR regs Indirect Write */
int
soc_ddr40_phy_reg_ci_write(int unit, int ci, uint32 reg_addr, uint32 reg_data)
{
    uint32 rval;
    soc_timeout_t to;

    SOC_IF_ERROR_RETURN(WRITE_CI_DDR_PHY_REG_DATAr(unit, ci, reg_data));

    rval = 0;
    soc_reg_field_set(unit, CI_DDR_PHY_REG_CTRLr, &rval, PHY_REG_ACKf, 1); /* write 1 to clear */
    soc_reg_field_set(unit, CI_DDR_PHY_REG_CTRLr, &rval, PHY_REG_ERR_ACKf, 1); /* write 1 to clear */
    soc_reg_field_set(unit, CI_DDR_PHY_REG_CTRLr, &rval, PHY_REG_ADDRf, reg_addr);
    soc_reg_field_set(unit, CI_DDR_PHY_REG_CTRLr, &rval, PHY_REG_RD_WR_Nf, 0);
    soc_reg_field_set(unit, CI_DDR_PHY_REG_CTRLr, &rval, PHY_REG_REQf, 1);
    SOC_IF_ERROR_RETURN(WRITE_CI_DDR_PHY_REG_CTRLr(unit, ci, rval));

    if (SAL_BOOT_SIMULATION) {
        soc_timeout_init(&to, 10000000, 0); /* 10 Sec */
    } else {
        soc_timeout_init(&to, 50000, 0); /* 50 mS */
    }
    do {
        SOC_IF_ERROR_RETURN(READ_CI_DDR_PHY_REG_CTRLr(unit, ci, &rval));
        if (soc_reg_field_get(unit, CI_DDR_PHY_REG_CTRLr, rval, PHY_REG_ACKf)) {
            break;
        }
        if (soc_reg_field_get(unit, CI_DDR_PHY_REG_CTRLr, rval, PHY_REG_ERR_ACKf)) {
            return SOC_E_FAIL;
        }
        if (soc_timeout_check(&to)) {
            return SOC_E_TIMEOUT;
        }
    } while (TRUE);
    return SOC_E_NONE;
}

int
soc_ddr40_phy_reg_ci_modify(int unit, uint32 ci, uint32 reg_addr, uint32 data, uint32 mask)
{
    int rv = SOC_E_NONE;
    uint32 tmp, otmp;

    data = data & mask;

    SOC_IF_ERROR_RETURN(soc_ddr40_phy_reg_ci_read(unit, ci, reg_addr, &tmp));
    otmp = tmp;
    tmp &= ~(mask);
    tmp |= data;

    if (otmp != tmp) {
        SOC_IF_ERROR_RETURN
            (soc_ddr40_phy_reg_ci_write(unit, ci, reg_addr, tmp));
    }

    return rv;
}

/***********************************************************************
* Functions to Read / Write / Modify DDR Memory
***********************************************************************/
int DDRWrRdComplete(int unit,
                            int ci,
                            uint32 uTimeout)
{
  uint32 i;
  uint32 data = 0;
  uint32 uAck = 0;

  /* wait for the ACK to indicate rd/wr op is finished */
  for(i = 0; i < uTimeout; i++ ) {
    SOC_IF_ERROR_RETURN(READ_CI_MEM_ACC_CTRLr(unit,ci,&data));  
    uAck = soc_reg_field_get(unit,CI_MEM_ACC_CTRLr,data,MEM_ACC_ACKf);
    if (uAck) {
      return SOC_E_NONE;
    }
    sal_udelay(10);
  }
  
  /* timed out */
  return ( SOC_E_TIMEOUT);
}

int soc_ddr40_read(int unit, int ci, uint32 addr, uint32 *pData0,
                uint32 *pData1, uint32 *pData2, uint32 *pData3,
                uint32 *pData4, uint32 *pData5, uint32 *pData6,
                uint32 *pData7) {
    uint32 uCmd;

    uCmd = 0;
    soc_reg_field_set(unit,CI_MEM_ACC_CTRLr,&uCmd,MEM_ACC_ACKf,1);
    soc_reg_field_set(unit,CI_MEM_ACC_CTRLr,&uCmd,MEM_ACC_REQf,1);
    soc_reg_field_set(unit,CI_MEM_ACC_CTRLr,&uCmd,MEM_ACC_RD_WR_Nf,1);
    soc_reg_field_set(unit,CI_MEM_ACC_CTRLr,&uCmd,MEM_ACC_ADDRf,addr);
    SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_CTRLr(unit,ci,uCmd));

    if (SOC_E_NONE != DDRWrRdComplete(unit, ci, DDR_TIMEOUT_10mS)) {
        soc_cm_debug(DK_ERR, "Error timeout reading from CI:%d addr:0x%x\n",ci,addr);
        return SOC_E_TIMEOUT;
    }

    SOC_IF_ERROR_RETURN(READ_CI_MEM_ACC_DATA0r(unit,ci,pData0));
    SOC_IF_ERROR_RETURN(READ_CI_MEM_ACC_DATA1r(unit,ci,pData1));
    SOC_IF_ERROR_RETURN(READ_CI_MEM_ACC_DATA2r(unit,ci,pData2));
    SOC_IF_ERROR_RETURN(READ_CI_MEM_ACC_DATA3r(unit,ci,pData3));
    SOC_IF_ERROR_RETURN(READ_CI_MEM_ACC_DATA4r(unit,ci,pData4));
    SOC_IF_ERROR_RETURN(READ_CI_MEM_ACC_DATA5r(unit,ci,pData5));
    SOC_IF_ERROR_RETURN(READ_CI_MEM_ACC_DATA6r(unit,ci,pData6));
    SOC_IF_ERROR_RETURN(READ_CI_MEM_ACC_DATA7r(unit,ci,pData7));
    return SOC_E_NONE;
}

int soc_ddr40_write(int unit, int ci, uint32 addr, uint32 uData0,
                uint32 uData1, uint32 uData2, uint32 uData3,
                uint32 uData4, uint32 uData5, uint32 uData6,
                uint32 uData7) {
    uint32 uCmd;

    /* setup the data */
    SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA0r(unit,ci,uData0));
    SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA1r(unit,ci,uData1));
    SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA2r(unit,ci,uData2));
    SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA3r(unit,ci,uData3));
    SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA4r(unit,ci,uData4));
    SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA5r(unit,ci,uData5));
    SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA6r(unit,ci,uData6));
    SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA7r(unit,ci,uData7));

    uCmd = 0;
    soc_reg_field_set(unit, CI_MEM_ACC_CTRLr, &uCmd, MEM_ACC_ACKf, 1);
    soc_reg_field_set(unit, CI_MEM_ACC_CTRLr, &uCmd, MEM_ACC_REQf, 1);
    soc_reg_field_set(unit, CI_MEM_ACC_CTRLr, &uCmd, MEM_ACC_RD_WR_Nf, 0);
    soc_reg_field_set(unit, CI_MEM_ACC_CTRLr, &uCmd, MEM_ACC_ADDRf, addr);
    SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_CTRLr(unit, ci, uCmd));

    if (SOC_E_NONE != DDRWrRdComplete(unit, ci, DDR_TIMEOUT_10mS)) {
        soc_cm_debug(DK_ERR, "Error timeout writing to CI:%d addr:0x%x\n",ci,addr);
        return SOC_E_TIMEOUT;
    }
    return SOC_E_NONE;
}

/***********************************************************************
* Shmoo Functions
***********************************************************************/

STATIC uint32
_ddr40_rand(void)
{
    return sal_time_usecs();
}

/* Generate NUM_DATA uint32 data for memory accesses */
STATIC void
_fill_in_data(uint32 *data)
{
    int32 i;

    for ( i = 0; i < NUM_DATA; i++ ) {
        if ( i & 0x1 )
            data[i] = 0xa5a5a500 | i;
        else
            data[i] = 0x5a5a5a00 | i;
    }
    return;
}

STATIC void
_test_mem_write(int unit, int ci, int addr, uint32 * uDataWR) {
    
    int32 i;
    for ( i = 0; i < NUM_DATA/8; i++ ) {
        soc_ddr40_write(unit, ci, addr+i, uDataWR[i*8],
                        uDataWR[(i*8)+1], uDataWR[(i*8)+2], uDataWR[(i*8)+3],
                        uDataWR[(i*8)+4], uDataWR[(i*8)+5], uDataWR[(i*8)+6],
                        uDataWR[(i*8)+7]);
    }
}

STATIC uint32
_test_rd_data_fifo_status(int unit, int ci, int wl, int loop)
{
    int i;
    uint32 addr;
    uint32 uDataRD[NUM_DATA];
    uint32 rval = 0;
    uint32 result_fail = 1;
  
    addr = (_ddr40_rand() & 0xFFFFFF);
    for ( i = 0; i < NUM_DATA/8; i++ ) {
      soc_ddr40_read(unit,(ci+wl),addr+i,&uDataRD[(i*8)],&uDataRD[(i*8)+1],&uDataRD[(i*8)+2],
                  &uDataRD[(i*8)+3],&uDataRD[(i*8)+4],&uDataRD[(i*8)+5],&uDataRD[(i*8)+6],&uDataRD[(i*8)+7]); 
    }

    rval = 0;
    if (wl==0) { 
        READ_DDR40_PHY_WORD_LANE_0_READ_FIFO_STATUSr(unit, ci, &rval);
    } else {
        READ_DDR40_PHY_WORD_LANE_1_READ_FIFO_STATUSr(unit, ci, &rval);
    }

    result_fail = (rval>0) ? 0xffff : 0;

    return result_fail;
}

static int
soc_test_func_self_test(int unit, int ci, int wl, int loop, int use_addr,uint32 dq_addr, int dq_burst, int dq_iter, int use_dq, uint32 dq_data0, uint32 dq_data0_alt)
{
    uint32 i;
    int j, port;
    uint32 rval = 0;
    uint32 result_fail = 0;
  
    /* Alternate data test */
    port = ci + wl;
    j = 0;
    while (j<loop) {
        rval = 0;
        SOC_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit, port, &rval)); 
        soc_reg_field_set(unit, CI_DDR_TESTr, &rval, MODEf, 0); 
        soc_reg_field_set(unit, CI_DDR_TESTr, &rval, RAM_TESTf, 0); 
        soc_reg_field_set(unit, CI_DDR_TESTr, &rval, RAM_DONEf, 1); /* write 1 to clear */
        soc_reg_field_set(unit, CI_DDR_TESTr, &rval, RAM_TEST_FAILf, 1); /* write 1 to clear */
        SOC_IF_ERROR_RETURN
            (WRITE_CI_DDR_TESTr(unit, port, rval));
        SOC_IF_ERROR_RETURN
            (WRITE_CI_DDR_BURSTr(unit, port, (dq_burst > 0) ? dq_burst : 0x1ff0));
        SOC_IF_ERROR_RETURN
            (WRITE_CI_DDR_STEPr(unit, port, 1));
        SOC_IF_ERROR_RETURN
            (WRITE_CI_DDR_ITERr(unit, port, (dq_iter > 0) ? dq_iter : 2));
        if (use_addr > 0) { 
           SOC_IF_ERROR_RETURN
               (WRITE_CI_DDR_STARTr(unit, port, (dq_addr & 0xffffff)));
        } else {
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_STARTr(unit, port, (_ddr40_rand() & 0xffffff)));
        }
        if (use_dq > 0) { 
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_DATA0r(unit, port, dq_data0));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_DATA1r(unit, port, dq_data0));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_DATA2r(unit, port, dq_data0));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_DATA3r(unit, port, dq_data0));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_DATA4r(unit, port, dq_data0));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_DATA5r(unit, port, dq_data0));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_DATA6r(unit, port, dq_data0));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_DATA7r(unit, port, dq_data0));

            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_ALT_DATA0r(unit, port, dq_data0_alt));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_ALT_DATA1r(unit, port, dq_data0_alt));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_ALT_DATA2r(unit, port, dq_data0_alt));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_ALT_DATA3r(unit, port, dq_data0_alt));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_ALT_DATA4r(unit, port, dq_data0_alt));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_ALT_DATA5r(unit, port, dq_data0_alt));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_ALT_DATA6r(unit, port, dq_data0_alt));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_ALT_DATA7r(unit, port, dq_data0_alt));
        } else {
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_DATA0r(unit, port, _ddr40_rand()));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_DATA1r(unit, port, _ddr40_rand()));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_DATA2r(unit, port, _ddr40_rand()));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_DATA3r(unit, port, _ddr40_rand()));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_DATA4r(unit, port, _ddr40_rand()));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_DATA5r(unit, port, _ddr40_rand()));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_DATA6r(unit, port, _ddr40_rand()));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_DATA7r(unit, port, _ddr40_rand()));

            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_ALT_DATA0r(unit, port, _ddr40_rand()));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_ALT_DATA1r(unit, port, _ddr40_rand()));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_ALT_DATA2r(unit, port, _ddr40_rand()));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_ALT_DATA3r(unit, port, _ddr40_rand()));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_ALT_DATA4r(unit, port, _ddr40_rand()));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_ALT_DATA5r(unit, port, _ddr40_rand()));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_ALT_DATA6r(unit, port, _ddr40_rand()));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TEST_ALT_DATA7r(unit, port, _ddr40_rand()));
        }

        rval = 0;
        SOC_IF_ERROR_RETURN
            (READ_CI_DDR_TESTr(unit, port, &rval)); 
        soc_reg_field_set(unit, CI_DDR_TESTr, &rval, RAM_TESTf, 1); 
        SOC_IF_ERROR_RETURN
            (WRITE_CI_DDR_TESTr(unit, port, rval));

        i=0;
        do {
            sal_usleep(20);
            SOC_IF_ERROR_RETURN
                (READ_CI_DDR_TESTr(unit, port, &rval)); 
            if (soc_reg_field_get(unit, CI_DDR_TESTr, rval, RAM_DONEf)) {
                if (soc_reg_field_get(unit, CI_DDR_TESTr, rval, RAM_TEST_FAILf)) {
                    result_fail = 1;
                }
                break;
            }
            if (i > 10000) {
                soc_cm_debug(DK_ERR, "Functional Self-Test timeout (1) \n");
                result_fail = 0xffff;
                return SOC_E_FAIL;
            }
            i++;
        } while (TRUE);
        j++;
    }

    if (result_fail != 0) {
        result_fail = 0xffff;
    }

    return result_fail;
}

static int
soc_test_two_func_self_test(int unit, int ci, int wl, int loop, int use_addr,uint32 dq_addr, int dq_burst, int dq_iter, int use_dq, uint32 dq_data0, uint32 dq_data0_alt)
{
    int i, j, lane, port;
    uint32 rval = 0;
    uint32 result_fail = 0;
    uint32 max_num_lane ;

    if (ci < 2) {
        max_num_lane=2;
    } else {
        max_num_lane=1;
    }
  
    /* Alternate data test */
    port = ci + wl;
    for (lane = 0; lane < max_num_lane; lane++) {
        j = 0;
        while (j < loop) {
            rval = 0;
            SOC_IF_ERROR_RETURN
                (READ_CI_DDR_TESTr(unit, (ci+lane), &rval)); 
            soc_reg_field_set(unit, CI_DDR_TESTr, &rval, MODEf, 0); 
            soc_reg_field_set(unit, CI_DDR_TESTr, &rval, RAM_TESTf, 0); 
            soc_reg_field_set(unit, CI_DDR_TESTr, &rval, RAM_DONEf, 1); /* write 1 to clear */
            soc_reg_field_set(unit, CI_DDR_TESTr, &rval, RAM_TEST_FAILf, 1); /* write 1 to clear */
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TESTr(unit, (ci+lane), rval));
            SOC_IF_ERROR_RETURN
                (READ_CI_DDR_TESTr(unit, (ci+lane), &rval)); 

            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_BURSTr(unit, port, (dq_burst>0) ? dq_burst : 0x1ff0));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_STEPr(unit, (ci+lane), 1));
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_ITERr(unit, port, (dq_iter>0) ? dq_iter : 2));

            if (use_addr>0) { 
                SOC_IF_ERROR_RETURN(WRITE_CI_DDR_STARTr(unit, port, (dq_addr & 0xffffff)));
            } else {
                SOC_IF_ERROR_RETURN(WRITE_CI_DDR_STARTr(unit, port, (_ddr40_rand() & 0xffffff)));
            }
            if (use_dq>0) { 
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_DATA0r(unit, port, dq_data0));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_DATA1r(unit, port, dq_data0));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_DATA2r(unit, port, dq_data0));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_DATA3r(unit, port, dq_data0));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_DATA4r(unit, port, dq_data0));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_DATA5r(unit, port, dq_data0));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_DATA6r(unit, port, dq_data0));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_DATA7r(unit, port, dq_data0));

                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_ALT_DATA0r(unit, port, dq_data0_alt));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_ALT_DATA1r(unit, port, dq_data0_alt));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_ALT_DATA2r(unit, port, dq_data0_alt));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_ALT_DATA3r(unit, port, dq_data0_alt));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_ALT_DATA4r(unit, port, dq_data0_alt));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_ALT_DATA5r(unit, port, dq_data0_alt));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_ALT_DATA6r(unit, port, dq_data0_alt));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_ALT_DATA7r(unit, port, dq_data0_alt));
            } else {
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_DATA0r(unit, port, _ddr40_rand()));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_DATA1r(unit, port, _ddr40_rand()));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_DATA2r(unit, port, _ddr40_rand()));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_DATA3r(unit, port, _ddr40_rand()));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_DATA4r(unit, port, _ddr40_rand()));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_DATA5r(unit, port, _ddr40_rand()));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_DATA6r(unit, port, _ddr40_rand()));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_DATA7r(unit, port, _ddr40_rand()));

                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_ALT_DATA0r(unit, port, _ddr40_rand()));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_ALT_DATA1r(unit, port, _ddr40_rand()));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_ALT_DATA2r(unit, port, _ddr40_rand()));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_ALT_DATA3r(unit, port, _ddr40_rand()));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_ALT_DATA4r(unit, port, _ddr40_rand()));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_ALT_DATA5r(unit, port, _ddr40_rand()));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_ALT_DATA6r(unit, port, _ddr40_rand()));
                SOC_IF_ERROR_RETURN
                    (WRITE_CI_DDR_TEST_ALT_DATA7r(unit, port, _ddr40_rand()));
            }
        
            rval = 0;
            soc_reg_field_set(unit, CI_DDR_TESTr, &rval, RAM_TESTf, 1); 
            SOC_IF_ERROR_RETURN
                (WRITE_CI_DDR_TESTr(unit, (ci+lane), rval));

            i = 0;
            do {
                sal_usleep(20);
                SOC_IF_ERROR_RETURN
                    (READ_CI_DDR_TESTr(unit, (ci+lane), &rval)); 
                if (soc_reg_field_get(unit, CI_DDR_TESTr, rval, RAM_DONEf)) {
                    if (soc_reg_field_get(unit, CI_DDR_TESTr, rval, RAM_TEST_FAILf)) {
                        result_fail = result_fail + 1;
                    }
                    break;
                }
                if (i > 100000) {
                    soc_cm_debug(DK_ERR, "Functional Self-Test timeout (1) \n");
                    return SOC_E_FAIL;
                }
                i++;
            } while (TRUE);
            j++;
        }
    }

    if (result_fail != 0) {
        result_fail = 0xffff;
    }

    return result_fail;
}

STATIC uint32
_test_mem_read(int unit, int ci, uint32 addr, uint32 * data_exp)
{
    int i, j;
    uint32 uDataRD[NUM_DATA];
    uint32 result = 0;
  
    for ( i = 0; i < NUM_DATA/8; i++ ) {
        soc_ddr40_read(unit,ci,addr+i,&uDataRD[(i*8)],&uDataRD[(i*8)+1],
                       &uDataRD[(i*8)+2], &uDataRD[(i*8)+3],&uDataRD[(i*8)+4],
                       &uDataRD[(i*8)+5],&uDataRD[(i*8)+6],&uDataRD[(i*8)+7]); 

        for ( j = 0; j < 8; j++ ) {
            if (data_exp[(i*8)+j] != uDataRD[(i*8)+j]) {
                result |= data_exp[(i*8)+j] ^ uDataRD[(i*8)+j];
            }
        }
        result |= ( result >> 16 );
        result &= 0xFFFF;
    }
    return result;
}

STATIC uint32
_test_mem_wr_rd(int unit, int ci, uint32 addr)
{
    uint32 uDataWR[NUM_DATA];
    uint32 result = 0;

    _fill_in_data(uDataWR);
    _test_mem_write(unit, ci, addr, uDataWR);

    result = _test_mem_read(unit, ci, addr, uDataWR);

    return result;
}

/* Prepare for shmoo-calibration */
STATIC int
_soc_ddr_shmoo_prepare_for_shmoo(int unit, int ci)
{
    int wl;
    uint32 data;

    wl=0;
    /* I want to display all STATUS value here and report */
    READ_DDR40_PHY_CONTROL_REGS_PLL_STATUSr(unit, ci, &data);
    soc_cm_print( "(WL=%d) data      = 0x%x \n", wl, data );
    soc_cm_print( "(WL=%d) PLL_STATUS : LOCK_LOST = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, PLL_STATUS, LOCK_LOST) );
    soc_cm_print( "(WL=%d) PLL_STATUS : LOCK      = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, PLL_STATUS, LOCK     ) );
    READ_DDR40_PHY_CONTROL_REGS_ZQ_PVT_COMP_CTLr(unit, ci, &data);
    soc_cm_print( "(WL=%d) data      = 0x%x \n", wl, data );
    soc_cm_print( "(WL=%d) ZQ_PVT_COMP_CTL : PD_COMP          = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, ZQ_PVT_COMP_CTL , PD_COMP ) );
    soc_cm_print( "(WL=%d) ZQ_PVT_COMP_CTL : ND_COMP          = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, ZQ_PVT_COMP_CTL , ND_COMP ) );
    if (wl==0) {
      READ_DDR40_PHY_WORD_LANE_0_READ_CONTROLr(unit, ci, &data);
      soc_cm_print( "(WL=%d) data      = 0x%x \n", wl, data );
      soc_cm_print( "(WL=%d) PHY_WORD_LANE_READ_CONTROL : DQ_ODT_ENABLE = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_WORD_LANE_0 , READ_CONTROL , DQ_ODT_ENABLE ) );
      soc_cm_print( "(WL=%d) PHY_WORD_LANE_READ_CONTROL : DQ_ODT_LE_ADJ = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_WORD_LANE_0 , READ_CONTROL , DQ_ODT_LE_ADJ ) );
      soc_cm_print( "(WL=%d) PHY_WORD_LANE_READ_CONTROL : DQ_ODT_TE_ADJ = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_WORD_LANE_0 , READ_CONTROL , DQ_ODT_TE_ADJ ) );
    } else { /* if (wl==0) */
      READ_DDR40_PHY_WORD_LANE_1_READ_CONTROLr(unit, ci, &data);
      soc_cm_print( "(WL=%d) data      = 0x%x \n", wl, data );
      soc_cm_print( "(WL=%d) PHY_WORD_LANE_READ_CONTROL : DQ_ODT_ENABLE = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_WORD_LANE_1 , READ_CONTROL , DQ_ODT_ENABLE ) );
      soc_cm_print( "(WL=%d) PHY_WORD_LANE_READ_CONTROL : DQ_ODT_LE_ADJ = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_WORD_LANE_1 , READ_CONTROL , DQ_ODT_LE_ADJ ) );
      soc_cm_print( "(WL=%d) PHY_WORD_LANE_READ_CONTROL : DQ_ODT_TE_ADJ = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_WORD_LANE_1 , READ_CONTROL , DQ_ODT_TE_ADJ ) );
    } /* if (wl==0) */
    READ_DDR40_PHY_CONTROL_REGS_VDL_CALIBRATEr(unit, ci, &data);
    soc_cm_print( "(WL=%d) data      = 0x%x \n", wl, data );
    soc_cm_print( "(WL=%d) VDL_CALIBRATE : CALIB_FAST       = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIBRATE, CALIB_FAST      ) );
    soc_cm_print( "(WL=%d) VDL_CALIBRATE : CALIB_ONCE       = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIBRATE, CALIB_ONCE      ) );
    soc_cm_print( "(WL=%d) VDL_CALIBRATE : CALIB_ALWAYS     = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIBRATE, CALIB_ALWAYS    ) );
    soc_cm_print( "(WL=%d) VDL_CALIBRATE : CALIB_TEST       = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIBRATE, CALIB_TEST      ) );
    soc_cm_print( "(WL=%d) VDL_CALIBRATE : CALIB_CLOCKS     = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIBRATE, CALIB_CLOCKS    ) );
    soc_cm_print( "(WL=%d) VDL_CALIBRATE : CALIB_BYTE       = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIBRATE, CALIB_BYTE      ) );
    soc_cm_print( "(WL=%d) VDL_CALIBRATE : CALIB_PHYBIST    = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIBRATE, CALIB_PHYBIST   ) );
    soc_cm_print( "(WL=%d) VDL_CALIBRATE : CALIB_FTM        = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIBRATE, CALIB_FTM       ) );
    soc_cm_print( "(WL=%d) VDL_CALIBRATE : CALIB_AUTO       = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIBRATE, CALIB_AUTO      ) );
    soc_cm_print( "(WL=%d) VDL_CALIBRATE : CALIB_STEPS      = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIBRATE, CALIB_STEPS     ) );
    soc_cm_print( "(WL=%d) VDL_CALIBRATE : CALIB_DQS_PAIR   = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIBRATE, CALIB_DQS_PAIR  ) );
    soc_cm_print( "(WL=%d) VDL_CALIBRATE : CALIB_DQS_CLOCKS = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIBRATE, CALIB_DQS_CLOCKS) );
    soc_cm_print( "(WL=%d) VDL_CALIBRATE : CALIB_BIT_OFFSET = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIBRATE, CALIB_BIT_OFFSET) );
    soc_cm_print( "(WL=%d) VDL_CALIBRATE : RD_EN_CAL        = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIBRATE, RD_EN_CAL       ) );
    soc_cm_print( "(WL=%d) VDL_CALIBRATE : BIT_CAL          = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIBRATE, BIT_CAL         ) );
    soc_cm_print( "(WL=%d) VDL_CALIBRATE : SET_MR_MPR       = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIBRATE, SET_MR_MPR      ) );
    soc_cm_print( "(WL=%d) VDL_CALIBRATE : DQ0_ONLY         = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIBRATE, DQ0_ONLY        ) );
    soc_cm_print( "(WL=%d) VDL_CALIBRATE : SET_WR_DQ        = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIBRATE, SET_WR_DQ       ) );
    soc_cm_print( "(WL=%d) VDL_CALIBRATE : BIT_REFRESH      = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIBRATE, BIT_REFRESH     ) );
    soc_cm_print( "(WL=%d) VDL_CALIBRATE : RD_DLY_CAL       = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIBRATE, RD_DLY_CAL      ) );
    soc_cm_print( "(WL=%d) VDL_CALIBRATE : EXIT_IN_SR       = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIBRATE, EXIT_IN_SR      ) );
    soc_cm_print( "(WL=%d) VDL_CALIBRATE : SKIP_RST         = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIBRATE, SKIP_RST        ) );
    soc_cm_print( "(WL=%d) VDL_CALIBRATE : AUTO_INIT        = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIBRATE, AUTO_INIT       ) );
    soc_cm_print( "(WL=%d) VDL_CALIBRATE : USE_STRAPS       = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIBRATE, USE_STRAPS      ) );
    READ_DDR40_PHY_CONTROL_REGS_VDL_CALIB_STATUSr(unit, ci, &data);
    soc_cm_print( "(WL=%d) data      = 0x%x \n", wl, data );
    soc_cm_print( "(WL=%d) VDL_CALIB_STATUS : CALIB_LOCK                           = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIB_STATUS, CALIB_LOCK   ) );
    soc_cm_print( "(WL=%d) VDL_CALIB_STATUS : CALIB_BYTE_SEL                       = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIB_STATUS, CALIB_BYTE_SEL  ) );
    soc_cm_print( "(WL=%d) VDL_CALIB_STATUS : CALIB_BIT_OFFSET  set if byte mode   = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIB_STATUS, CALIB_BIT_OFFSET  ) );
    soc_cm_print( "(WL=%d) VDL_CALIB_STATUS : CALIB_TOTAL  ADDR (steps)            = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIB_STATUS, CALIB_TOTAL )>>4 );
    READ_DDR40_PHY_CONTROL_REGS_VDL_DQ_CALIB_STATUSr(unit, ci, &data);
    soc_cm_print( "(WL=%d) data      = 0x%x \n", wl, data );
    soc_cm_print( "(WL=%d) VDL_DQ_CALIB_STATUS : DQ_CALIB_LOCK                     = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_DQ_CALIB_STATUS, DQ_CALIB_LOCK   ) );
    soc_cm_print( "(WL=%d) VDL_DQ_CALIB_STATUS : DQS_CALIB_LOCK                    = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_DQ_CALIB_STATUS, DQS_CALIB_LOCK  ) );
    soc_cm_print( "(WL=%d) VDL_DQ_CALIB_STATUS : DQS_CALIB_MODE    DQS(1=pair)     = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_DQ_CALIB_STATUS, DQS_CALIB_MODE  ) );
    soc_cm_print( "(WL=%d) VDL_DQ_CALIB_STATUS : DQS_CALIB_CLOCKS  DQS(0=half bit) = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_DQ_CALIB_STATUS, DQS_CALIB_CLOCKS) );
    soc_cm_print( "(WL=%d) VDL_DQ_CALIB_STATUS : DQ_CALIB_TOTAL    DQ (steps)      = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_DQ_CALIB_STATUS, DQ_CALIB_TOTAL  )>>4 );
    soc_cm_print( "(WL=%d) VDL_DQ_CALIB_STATUS : DQS_CALIB_TOTAL   DQS (steps)     = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_DQ_CALIB_STATUS, DQS_CALIB_TOTAL )>>4 );
    READ_DDR40_PHY_CONTROL_REGS_VDL_WR_CHAN_CALIB_STATUSr(unit, ci, &data);
    soc_cm_print( "(WL=%d) data      = 0x%x \n", wl, data );
    soc_cm_print( "(WL=%d) VDL_WR_CHAN_CALIB_STATUS : WR_CHAN_CALIB_LOCK                                          = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_WR_CHAN_CALIB_STATUS, WR_CHAN_CALIB_LOCK      ) );
    soc_cm_print( "(WL=%d) VDL_WR_CHAN_CALIB_STATUS : WR_CHAN_CALIB_BYTE_SEL   (1=byte)                           = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_WR_CHAN_CALIB_STATUS, WR_CHAN_CALIB_BYTE_SEL  ) );
    soc_cm_print( "(WL=%d) VDL_WR_CHAN_CALIB_STATUS : WR_CHAN_CALIB_CLOCKS     (0=1/2bit)                         = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_WR_CHAN_CALIB_STATUS, WR_CHAN_CALIB_CLOCKS    ) );
    soc_cm_print( "(WL=%d) VDL_WR_CHAN_CALIB_STATUS : WR_CHAN_CALIB_TOTAL      (steps)                            = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_WR_CHAN_CALIB_STATUS, WR_CHAN_CALIB_TOTAL     )>>4 );
    soc_cm_print( "(WL=%d) VDL_WR_CHAN_CALIB_STATUS : WR_CHAN_CALIB_BIT_OFFSET (in byte mode, setting for bit vdl)= 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_WR_CHAN_CALIB_STATUS, WR_CHAN_CALIB_BIT_OFFSET) );
    READ_DDR40_PHY_CONTROL_REGS_VDL_RD_EN_CALIB_STATUSr(unit, ci, &data);
    soc_cm_print( "(WL=%d) data      = 0x%x \n", wl, data );
    soc_cm_print( "(WL=%d) VDL_RD_EN_CALIB_STATUS : RD_EN_CALIB_LOCK                                          = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_RD_EN_CALIB_STATUS, RD_EN_CALIB_LOCK      ) );
    soc_cm_print( "(WL=%d) VDL_RD_EN_CALIB_STATUS : RD_EN_CALIB_BYTE_SEL   (1=byte)                           = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_RD_EN_CALIB_STATUS, RD_EN_CALIB_BYTE_SEL  ) );
    soc_cm_print( "(WL=%d) VDL_RD_EN_CALIB_STATUS : RD_EN_CALIB_CLOCKS     (0=1/2bit)                         = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_RD_EN_CALIB_STATUS, RD_EN_CALIB_CLOCKS    ) );
    soc_cm_print( "(WL=%d) VDL_RD_EN_CALIB_STATUS : RD_EN_CALIB_TOTAL      (steps)                            = 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_RD_EN_CALIB_STATUS, RD_EN_CALIB_TOTAL     )>>4 );
    soc_cm_print( "(WL=%d) VDL_RD_EN_CALIB_STATUS : RD_EN_CALIB_BIT_OFFSET (in byte mode, setting for bit vdl)= 0x%x \n", wl, DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_RD_EN_CALIB_STATUS, RD_EN_CALIB_BIT_OFFSET) );

    return SOC_E_NONE;
} /* _soc_ddr_shmoo_prepare_for_shmoo */

/* Initialize VDL 90' result from VDL Calibration */
int
_shmoo_init_vdl_result(int unit, int ci, int wl, uint32 * result, int preset, int dqs_steps, int self_test, int use_addr,uint32 dq_addr, int dq_burst, int dq_iter, int use_dq, uint32 dq_data0, uint32 dq_data0_alt)
{
    int i, byte, bit;
    uint32 data_expected[NUM_DATA];
    uint32 data, setting, setting_steps, setting_byte_sel;
    int32 steps_calib_total;

    _fill_in_data(data_expected);
    if (self_test==0) {
        _test_mem_write(unit, (ci+wl), 0, data_expected); 
    }

    /* RD_EN */
    READ_DDR40_PHY_CONTROL_REGS_VDL_RD_EN_CALIB_STATUSr(unit, ci, &data);
    setting_byte_sel = DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS,
                             VDL_RD_EN_CALIB_STATUS, RD_EN_CALIB_BYTE_SEL);
    setting_steps = DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS,
                          VDL_RD_EN_CALIB_STATUS, RD_EN_CALIB_TOTAL) >> 4;
    setting = setting_steps; 
    if (wl==0) {
        data = SET_OVR_STEP( 0 );
        WRITE_DDR40_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE_RD_ENr(unit,ci,data);
        data = SET_OVR_STEP(setting);
        if (preset) {
            data = (ci==0) ? SET_OVR_STEP( SHMOO_CI0_WL0_PRETUNE_RD_EN ) :
                             SET_OVR_STEP( SHMOO_CI2_WL0_PRETUNE_RD_EN );
        }
        WRITE_DDR40_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_BIT_RD_ENr(unit,ci,data);
        WRITE_DDR40_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_BIT_RD_ENr(unit,ci,data);
    } else { 
        data = SET_OVR_STEP( 0 );
        WRITE_DDR40_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE_RD_ENr(unit,ci,data);
        data = SET_OVR_STEP(setting + SHMOO_CI0_WL1_OFFSET_RD_EN);
        if (preset) {
            data = SET_OVR_STEP( SHMOO_CI0_WL1_PRETUNE_RD_EN ) ;
        }
        WRITE_DDR40_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE0_BIT_RD_ENr(unit,ci,data);
        WRITE_DDR40_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE1_BIT_RD_ENr(unit,ci,data);
    }

    /* RD_DQ */
    READ_DDR40_PHY_CONTROL_REGS_VDL_DQ_CALIB_STATUSr(unit, ci, &data);
    steps_calib_total = DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS,
                              VDL_DQ_CALIB_STATUS, DQ_CALIB_TOTAL) >> 4;
    setting = steps_calib_total ;
    data = (wl==0) ? SET_OVR_STEP(setting) : 
                     SET_OVR_STEP(setting+ SHMOO_CI0_WL1_OFFSET_RD_DQ);
    if (preset) {
        data = (wl==0) ? SET_OVR_STEP( SHMOO_CI0_WL0_PRETUNE_RD_DQ ) :
                         SET_OVR_STEP( SHMOO_CI0_WL1_PRETUNE_RD_DQ ); 
    }
    for ( byte = 0; byte < 2; byte++ ) {
        for ( bit = 0; bit < 8; bit++ ) { 
            WRITE_DDR40_PHY_WORD_LANE_x_VDL_OVRIDE_BYTEy_BITz_R_Pr(wl,byte,bit,unit,ci,data);
            WRITE_DDR40_PHY_WORD_LANE_x_VDL_OVRIDE_BYTEy_BITz_R_Nr(wl,byte,bit,unit,ci,data);
        }
    }

    /* RD_DQS */
    READ_DDR40_PHY_CONTROL_REGS_VDL_DQ_CALIB_STATUSr(unit, ci, &data);
    setting_byte_sel = DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS,
                             VDL_DQ_CALIB_STATUS, DQS_CALIB_MODE);
    setting_steps = DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS,
                          VDL_DQ_CALIB_STATUS, DQS_CALIB_TOTAL) >> 4;
    setting = (setting_steps*(1+setting_byte_sel));
    data = (wl==0) ? SET_OVR_STEP(setting) :
                     SET_OVR_STEP(setting+SHMOO_CI0_WL1_OFFSET_RD_DQS);
    if (preset) {
        data = (wl==0) ? SET_OVR_STEP( SHMOO_CI0_WL0_PRETUNE_RD_DQS ) :
                         SET_OVR_STEP( SHMOO_CI0_WL1_PRETUNE_RD_DQS ); 
    }
    if (dqs_steps>0) {
        data = SET_OVR_STEP( dqs_steps ) ;
    }
    if (wl == 0) {
        WRITE_DDR40_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_R_Pr(unit,ci,data);
        WRITE_DDR40_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_R_Nr(unit,ci,data);
        WRITE_DDR40_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_R_Pr(unit,ci,data); 
        WRITE_DDR40_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_R_Nr(unit,ci,data);
    } else {
        WRITE_DDR40_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE0_R_Pr(unit,ci,data);
        WRITE_DDR40_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE0_R_Nr(unit,ci,data);
        WRITE_DDR40_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE1_R_Pr(unit,ci,data);
        WRITE_DDR40_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE1_R_Nr(unit,ci,data);
    }

    /* WR_DQ  */
    /* bit */
    READ_DDR40_PHY_CONTROL_REGS_VDL_WR_CHAN_CALIB_STATUSr(unit, ci, &data);
    setting_byte_sel = DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS,
                            VDL_WR_CHAN_CALIB_STATUS, WR_CHAN_CALIB_BYTE_SEL);
    setting_steps = DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS,
                          VDL_WR_CHAN_CALIB_STATUS, WR_CHAN_CALIB_TOTAL) >> 4;
    setting = (setting_steps*(1+setting_byte_sel));
    data = (wl==0) ? SET_OVR_STEP( setting ) :
                     SET_OVR_STEP( setting+SHMOO_CI0_WL1_OFFSET_WR_DQ );
    if (preset) {
        data = (wl==0) ? SET_OVR_STEP( SHMOO_CI0_WL0_PRETUNE_WR_DQ ) :
                         SET_OVR_STEP( SHMOO_CI0_WL1_PRETUNE_WR_DQ );
    }
    for (byte = 0; byte < 2; byte++) {
        for (bit = 0; bit < 8; bit++) {
            WRITE_DDR40_PHY_WORD_LANE_x_VDL_OVRIDE_BYTEy_BITz_Wr(wl,byte,bit,unit,ci,data);
        }
    }
    /* byte */
    data = SET_OVR_STEP(0);
    if ( wl == 0 ) {
        WRITE_DDR40_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_Wr(unit, ci, data);
        WRITE_DDR40_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_Wr(unit, ci, data);
    } else {
        WRITE_DDR40_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE0_Wr(unit, ci, data);
        WRITE_DDR40_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE1_Wr(unit, ci, data);
    }

    /* ADDR   */
    if ( wl == 0 ) {
        /* Hardware adjust ADDR 90' prior VDL
         * Because we cannot trust ADDR Calibration result as it reports 0 steps
         * We then will use WR_DQ 90' calibration result to adjust ADDR (by 90') to become 180' for the address
         * Excluded below and use WR_DQ instead
         * setting_byte_sel = DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIB_STATUS, CALIB_BYTE_SEL  ) ;
         * setting_steps = DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIB_STATUS, CALIB_TOTAL )>>4 ;
         * setting = (setting_steps*(1+setting_byte_sel));
         */ 
        READ_DDR40_PHY_CONTROL_REGS_VDL_WR_CHAN_CALIB_STATUSr(unit, ci, &data);
        setting_byte_sel = DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS,
                            VDL_WR_CHAN_CALIB_STATUS, WR_CHAN_CALIB_BYTE_SEL);
        setting_steps = DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS,
                           VDL_WR_CHAN_CALIB_STATUS, WR_CHAN_CALIB_TOTAL) >> 4;
        setting = (setting_steps*(1+setting_byte_sel));
        data = SET_OVR_STEP(setting);
        if (preset) {
            data = SET_OVR_STEP( SHMOO_CI0_WL0_PRETUNE_ADDR ); 
        }
        WRITE_DDR40_PHY_CONTROL_REGS_VDL_OVRIDE_BIT_CTLr(unit, ci, data); 
    }

    /* Shmoo  */
    for ( i = 0; i < 64; ++i ) {
        /* Clear the FIFO error state */
        WRITE_DDR40_PHY_WORD_LANE_0_READ_FIFO_CLEARr(unit, ci, 0); 
        WRITE_DDR40_PHY_WORD_LANE_1_READ_FIFO_CLEARr(unit, ci, 0); 
        sal_usleep(9);
        if (self_test==0) {
            result[i] = _test_mem_read(unit, (ci+wl), 0, data_expected); 
        } else if (self_test==2) {
            result[i] = _test_rd_data_fifo_status(unit, ci, wl, 1); 
        } else {
            result[i] = soc_test_func_self_test(unit, ci, wl, 1, use_addr,dq_addr, dq_burst, dq_iter, use_dq, dq_data0, dq_data0_alt); 
        }
    }
    return SOC_E_NONE;
}

int
_shmoo_rd_en_rd_data_dly(int unit, int ci, int wl, uint32 * result, int self_test, int use_addr,uint32 dq_addr, int dq_burst, int dq_iter, int use_dq, uint32 dq_data0, uint32 dq_data0_alt)
{
    int i, j;
    uint32 data;
    uint32 data_expected[NUM_DATA];
    int see_pass, see_fail, rd_en_pass, rd_en_fail, rd_en_size, rd_data_dly_window_size;
    int rd_data_dly_best_avg;
    int rd_data_dly_best_min;
    int rd_data_dly_best_max;

    _fill_in_data(data_expected);
    if (self_test==0) {
        _test_mem_write(unit, (ci+wl), 0, data_expected); 
    }

    /* Clear the FIFO error state */
    WRITE_DDR40_PHY_WORD_LANE_0_READ_FIFO_CLEARr(unit, ci, 0); 
    WRITE_DDR40_PHY_WORD_LANE_1_READ_FIFO_CLEARr(unit, ci, 0); 
    WRITE_DDR40_PHY_WORD_LANE_0_READ_FIFO_CLEARr(unit, ci, 1); 
    WRITE_DDR40_PHY_WORD_LANE_1_READ_FIFO_CLEARr(unit, ci, 1); 
    /* Clear any error status from previous run */
    WRITE_DDR40_PHY_WORD_LANE_0_READ_FIFO_STATUSr(unit, ci, 1);
    WRITE_DDR40_PHY_WORD_LANE_0_READ_FIFO_STATUSr(unit, ci, 0);
    WRITE_DDR40_PHY_WORD_LANE_1_READ_FIFO_STATUSr(unit, ci, 1);
    WRITE_DDR40_PHY_WORD_LANE_1_READ_FIFO_STATUSr(unit, ci, 0);

    /* RD_EN should be good by this point. We now find the best RD_DATA_DLY (for fast speed, we start from 3 and up) */
    rd_en_size = 0;
    rd_data_dly_best_max = 3;
    rd_data_dly_best_min = 3;
    rd_data_dly_window_size = 0;

    /* Find Best RD_DATA_DLY  */
    for ( j = 1; j < 8; ++j ) {
        /* For every single RD_DATA_DLY, we shmoo RD_EN to find the largest window */
        data = j;
        if (wl == 0) { 
            WRITE_DDR40_PHY_WORD_LANE_0_READ_DATA_DLYr(unit, ci, data); 
        } else {
            WRITE_DDR40_PHY_WORD_LANE_1_READ_DATA_DLYr(unit, ci, data);  
        }
        see_pass = 0;
        see_fail = 0;
        rd_en_pass = 0;
        rd_en_fail = 0;
        rd_en_size = 0;
        for ( i = 0; i < 64; ++i ) {
            data = SET_OVR_STEP( i );

            /* RD_EN */
            if (wl==0) { 
                WRITE_DDR40_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_BIT_RD_ENr(unit,ci,data);
                WRITE_DDR40_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_BIT_RD_ENr(unit,ci,data);
            } else { 
                WRITE_DDR40_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE0_BIT_RD_ENr(unit,ci,data);
                WRITE_DDR40_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE1_BIT_RD_ENr(unit,ci,data);
            }

            /* Clear the FIFO error state */
            WRITE_DDR40_PHY_WORD_LANE_0_READ_FIFO_CLEARr(unit, ci, 0); 
            WRITE_DDR40_PHY_WORD_LANE_1_READ_FIFO_CLEARr(unit, ci, 0); 
            sal_usleep(9);
            if (self_test==0) {
                result[i] = _test_mem_read(unit, (ci+wl), 0, data_expected);
            } else if (self_test==2) {
                result[i] = _test_rd_data_fifo_status(unit, ci, wl, 1); 
            } else {
                result[i] = soc_test_func_self_test(unit, ci, wl, 1, use_addr,dq_addr, dq_burst, dq_iter, use_dq, dq_data0, dq_data0_alt); 
            }

            if (result[i]==0) { /* PASS */
                if (see_pass==0) {
                    rd_en_pass = i;
                    see_pass = 1;
                }
            } else {
                if (see_pass==1 && see_fail==0) {
                    rd_en_fail = i;
                    see_fail = 1;
                }
            }
        }

        if (see_pass==1 && see_fail==1) { 
            rd_en_size = rd_en_fail - rd_en_pass;
            if (rd_en_size > rd_data_dly_window_size) {
                rd_data_dly_window_size=rd_en_size;
                rd_data_dly_best_min=j;
            }
            if (rd_en_size == rd_data_dly_window_size) {
                rd_data_dly_best_max=j;
            }
        }
        rd_data_dly_best_avg = rd_data_dly_best_min + ((rd_data_dly_best_max-rd_data_dly_best_min)/2);
    }

    /* set RD_EN and RD_DATA_DLY */
    /* For every single RD_DATA_DLY, we shmoo RD_EN to find the largest window */
    data = rd_data_dly_best_avg;
    if (wl == 0) { 
        WRITE_DDR40_PHY_WORD_LANE_0_READ_DATA_DLYr(unit, ci, data); 
    } else {
        WRITE_DDR40_PHY_WORD_LANE_1_READ_DATA_DLYr(unit, ci, data);  
    }

    for ( i = 0; i < 64; ++i ) {
        data = SET_OVR_STEP( i );

        /* RD_EN */
        if (wl==0) { 
            WRITE_DDR40_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_BIT_RD_ENr(unit,ci,data);
            WRITE_DDR40_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_BIT_RD_ENr(unit,ci,data);
        } else { 
            WRITE_DDR40_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE0_BIT_RD_ENr(unit,ci,data);
            WRITE_DDR40_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE1_BIT_RD_ENr(unit,ci,data);
        }

        /* Clear the FIFO error state */
        WRITE_DDR40_PHY_WORD_LANE_0_READ_FIFO_CLEARr(unit, ci, 0); 
        WRITE_DDR40_PHY_WORD_LANE_1_READ_FIFO_CLEARr(unit, ci, 0); 
        sal_usleep(9);
        if (self_test==0) {
            result[i] = _test_mem_read(unit, (ci+wl), 0, data_expected); 
        } else if (self_test==2) {
            result[i] = _test_rd_data_fifo_status(unit, ci, wl, 1); 
        } else {
            result[i] = soc_test_func_self_test(unit, ci, wl, 1, use_addr,dq_addr, dq_burst, dq_iter, use_dq, dq_data0, dq_data0_alt); 
        }
    }

    return SOC_E_NONE;
}


STATIC int
_shmoo_rd_dq(int unit, int ci, int wl, uint32 * result, int self_test, int use_addr,uint32 dq_addr, int dq_burst, int dq_iter, int use_dq, uint32 dq_data0, uint32 dq_data0_alt)
{
    int32 i ;
    int32 byte, bit ;
    uint32 data;
    uint32 data_expected[NUM_DATA];

    _fill_in_data(data_expected);
    if (self_test==0) {
        _test_mem_write(unit, (ci+wl), 0, data_expected); 
    }

    for ( i = 0; i < 64; ++i ) {
        data = SET_OVR_STEP( i );

        /* RD_DQ */
        for (byte = 0; byte < 2; byte++) {
            for (bit = 0; bit < 8; bit++) {
                WRITE_DDR40_PHY_WORD_LANE_x_VDL_OVRIDE_BYTEy_BITz_R_Pr(wl,byte,bit,unit,ci,data);
                WRITE_DDR40_PHY_WORD_LANE_x_VDL_OVRIDE_BYTEy_BITz_R_Nr(wl,byte,bit,unit,ci,data);
            }
        }

        /* Clear the FIFO error state */
        WRITE_DDR40_PHY_WORD_LANE_0_READ_FIFO_CLEARr(unit, ci, 0); 
        WRITE_DDR40_PHY_WORD_LANE_1_READ_FIFO_CLEARr(unit, ci, 0); 
        sal_usleep(9);
        if (self_test==0) {
            result[i] = _test_mem_read(unit, (ci+wl), 0, data_expected); 
        } else if (self_test==2) {
            result[i] = _test_rd_data_fifo_status(unit, ci, wl, 1); 
        } else {
            result[i] = soc_test_func_self_test(unit, ci, wl, 1, use_addr,dq_addr, dq_burst, dq_iter, use_dq, dq_data0, dq_data0_alt); 
        }
    }

    /* Clear the FIFO error state */
    WRITE_DDR40_PHY_WORD_LANE_0_READ_FIFO_CLEARr(unit, ci, 0); 
    WRITE_DDR40_PHY_WORD_LANE_1_READ_FIFO_CLEARr(unit, ci, 0); 
    return SOC_E_NONE;

}
                       
/* Make a shmoo with the write dq vdl values */
STATIC int
_shmoo_wr_dq(int unit, int ci, int wl, uint32 * result, int self_test, int use_addr,uint32 dq_addr, int dq_burst, int dq_iter, int use_dq, uint32 dq_data0, uint32 dq_data0_alt)
{
    int32 i ;
    int32 byte, bit ;
    uint32 data;
    uint32 data_expected[NUM_DATA];

    _fill_in_data(data_expected);
    if (self_test==0) {
        _test_mem_write(unit, (ci+wl), 0, data_expected); 
    }

    for ( i = 0; i < 64; ++i ) {
        data = SET_OVR_STEP( i );

        /* WR_DQ */
        for (byte = 0; byte < 2; byte++) {
            for (bit = 0; bit < 8; bit++) {
                WRITE_DDR40_PHY_WORD_LANE_x_VDL_OVRIDE_BYTEy_BITz_Wr(wl,byte,bit,unit,ci,data);
            }
        }

        /* Clear the FIFO error state */
        WRITE_DDR40_PHY_WORD_LANE_0_READ_FIFO_CLEARr(unit, ci, 0); 
        WRITE_DDR40_PHY_WORD_LANE_1_READ_FIFO_CLEARr(unit, ci, 0); 
        sal_usleep(9);
        if (self_test==0) {
            result[i] = _test_mem_read(unit, (ci+wl), 0, data_expected); 
        } else if (self_test==2) {
            result[i] = _test_rd_data_fifo_status(unit, ci, wl, 1); 
        } else {
            result[i] = soc_test_func_self_test(unit, ci, wl, 1, use_addr,dq_addr, dq_burst, dq_iter, use_dq, dq_data0, dq_data0_alt); 
        }
    }

    /* Clear the FIFO error state */
    WRITE_DDR40_PHY_WORD_LANE_0_READ_FIFO_CLEARr(unit, ci, 0); 
    WRITE_DDR40_PHY_WORD_LANE_1_READ_FIFO_CLEARr(unit, ci, 0); 
    return SOC_E_NONE;
}

/* Make a shmoo with the Address */
STATIC int
_shmoo_addr(int unit, int ci, int wl, uint32 * result, int self_test, int use_addr,uint32 dq_addr, int dq_burst, int dq_iter, int use_dq, uint32 dq_data0, uint32 dq_data0_alt)
{
    uint32 data , timeout_cnt;
    uint32 setting;
    uint32 see_pass, see_fail_after_pass;
    uint32 start_vdl_val, pass_start, pass_end, num_lane; 
    int i;

    if (wl==1) return SOC_E_NONE;

    /* Find Starting VDL step        */
    READ_DDR40_PHY_CONTROL_REGS_VDL_WR_CHAN_CALIB_STATUSr(unit, ci, &data);
    start_vdl_val = DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_WR_CHAN_CALIB_STATUS, WR_CHAN_CALIB_TOTAL) >> 4;
    pass_start = 64; /* 64 means "have not found pass yet" */
    pass_end = 64; /* 64 means "have not found pass yet" */

    /* Find pass_end of window       */
    see_pass = 0;
    see_fail_after_pass = 0;

    for (setting = start_vdl_val; setting < 64; setting++) {
        data = SET_OVR_STEP( setting );
        DDR40_SET_FIELD(data,DDR40_PHY_CONTROL_REGS,VDL_OVRIDE_BIT_CTL,BYTE_SEL,1);
        WRITE_DDR40_PHY_CONTROL_REGS_VDL_OVRIDE_BIT_CTLr(unit, ci, data); 
     /* DDR40_SET_FIELD(data,DDR40_PHY_CONTROL_REGS,VDL_OVRIDE_BYTE_CTL,BYTE_SEL,1);
        WRITE_DDR40_PHY_CONTROL_REGS_VDL_OVRIDE_BYTE_CTLr(unit, ci, data); */

        /* Clear the FIFO error state */
        WRITE_DDR40_PHY_WORD_LANE_0_READ_FIFO_CLEARr(unit, ci, 0); 
        WRITE_DDR40_PHY_WORD_LANE_1_READ_FIFO_CLEARr(unit, ci, 0); 
        READ_DDR40_PHY_CONTROL_REGS_VDL_OVRIDE_BIT_CTLr(unit, ci, &data); 
        sal_usleep(9);

        if (self_test==0) {
            result[setting] = _test_mem_wr_rd(unit, (ci+wl), 0 );
        } else {
            if (ci==0) {
                result[setting] = soc_test_two_func_self_test(unit, ci, wl, 1, use_addr,dq_addr, dq_burst, dq_iter, use_dq, dq_data0, dq_data0_alt); 
            } else {
                result[setting] = soc_test_func_self_test(unit, ci, wl, 1, use_addr,dq_addr, dq_burst, dq_iter, use_dq, dq_data0, dq_data0_alt); 
            }
        }

        if (see_pass==0 && see_fail_after_pass==0) {
            if (result[setting]==0) {
                see_pass=1;
                pass_start = setting;
            }
        } else {
            if (see_fail_after_pass==0) {
                if (result[setting]>0) {
                    see_fail_after_pass=1;
                    pass_end = setting-1;
                } else {
                    pass_end = setting;
                }
            }
        }
    }

    /* REWRITE Memory MODE Registers */
    num_lane = (ci==0) ? 2 : 1;
    for (i = 0; i < num_lane; i++) {
        SOC_IF_ERROR_RETURN(READ_CI_DDR_AUTOINITr(unit,ci+i,&data));
        soc_reg_field_set(unit, CI_DDR_AUTOINITr, &data, DONEf, 1);
        soc_reg_field_set(unit, CI_DDR_AUTOINITr, &data, STARTf, 1);
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_AUTOINITr(unit,ci+i,data));
    }

    for (i = 0; i < num_lane; i++) {
        timeout_cnt = 0;
        do {
            sal_usleep(10);
            SOC_IF_ERROR_RETURN(READ_CI_DDR_AUTOINITr(unit,ci+i,&data));
            if (soc_reg_field_get(unit, CI_DDR_AUTOINITr, data, DONEf)) {
                break;
            }
            if (timeout_cnt>20000) {
                soc_cm_print( "REWRITE Memory Register Mode failed\n" );
                break;
            }
            timeout_cnt += 1;
        } while (TRUE);
    }

    /* Find pass_start of window       */
    see_pass = 0;
    see_fail_after_pass = 0;

    for (setting = start_vdl_val; setting > 0; setting--) {
        data = SET_OVR_STEP( setting );
        DDR40_SET_FIELD(data,DDR40_PHY_CONTROL_REGS,VDL_OVRIDE_BIT_CTL,BYTE_SEL,1);
        WRITE_DDR40_PHY_CONTROL_REGS_VDL_OVRIDE_BIT_CTLr(unit, ci, data); 
     /* DDR40_SET_FIELD(data,DDR40_PHY_CONTROL_REGS,VDL_OVRIDE_BYTE_CTL,BYTE_SEL,1);
        WRITE_DDR40_PHY_CONTROL_REGS_VDL_OVRIDE_BYTE_CTLr(unit, ci, data); */

        /* Clear the FIFO error state */
        WRITE_DDR40_PHY_WORD_LANE_0_READ_FIFO_CLEARr(unit, ci, 0); 
        WRITE_DDR40_PHY_WORD_LANE_1_READ_FIFO_CLEARr(unit, ci, 0); 
        sal_usleep(9);
        if (self_test==0) {
            result[setting] = _test_mem_wr_rd(unit, (ci+wl), 0 );
        } else {
            result[setting] = soc_test_two_func_self_test(unit, ci, wl, 1, use_addr,dq_addr, dq_burst, dq_iter, use_dq, dq_data0, dq_data0_alt); 
        }

        if (see_pass==0 && see_fail_after_pass==0) {
            if (result[setting]==0) {
                see_pass=1;
                pass_start = setting;
            }
        } else {
            if (see_fail_after_pass==0) {
                if (result[setting]>0) {
                    see_fail_after_pass=1;
                    pass_start = setting-1;
                } else {
                    pass_start = setting;
                }
            }
        }
    }

    if (self_test==0) {
        result[setting] = _test_mem_wr_rd(unit, (ci+wl), 0 );
    } else {
        result[setting] = soc_test_two_func_self_test(unit, ci, wl, 1, use_addr,dq_addr, dq_burst, dq_iter, use_dq, dq_data0, dq_data0_alt); 
    }

    /* REWRITE Memory MODE Registers */
    for (i = 0; i < num_lane; i++) {
        SOC_IF_ERROR_RETURN(READ_CI_DDR_AUTOINITr(unit,ci+i,&data));
        soc_reg_field_set(unit, CI_DDR_AUTOINITr, &data, DONEf, 1);
        soc_reg_field_set(unit, CI_DDR_AUTOINITr, &data, STARTf, 1);
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_AUTOINITr(unit,ci+i,data));
    }

    for (i = 0; i < num_lane; i++) {
        timeout_cnt = 0;
        do {
            sal_usleep(10);
            SOC_IF_ERROR_RETURN(READ_CI_DDR_AUTOINITr(unit,ci+i,&data));
            if (soc_reg_field_get(unit, CI_DDR_AUTOINITr, data, DONEf)) {
                break;
            }
            if (timeout_cnt>20000) {
                soc_cm_print( "REWRITE Memory Register Mode failed\n" );
                break;
            }
            timeout_cnt += 1;
        } while (TRUE);
    }

    /* Clear FIFO                    */
    WRITE_DDR40_PHY_WORD_LANE_0_READ_FIFO_CLEARr(unit, ci, 0); 
    WRITE_DDR40_PHY_WORD_LANE_1_READ_FIFO_CLEARr(unit, ci, 0); 
    return SOC_E_NONE;
}

/* Make a shmoo with the write data mask vdl values */
STATIC int
_shmoo_wr_dm(int unit, int ci, int wl, uint32 * result)
{
    soc_cm_print( "   ------------------------------------------------------\n" );
    soc_cm_print( "   ------------------------------------------------------\n" );
    soc_cm_print( "     THIS SHMOO WR_DM feature is NOT SUPPORTED ........  \n" );
    soc_cm_print( "   ------------------------------------------------------\n" );
    soc_cm_print( "   ------------------------------------------------------\n" );
    return SOC_E_NONE;
}

/* Calibrate the read enable vdl values */
STATIC int
_calib_rd_en(uint32 * result, uint32 * init_step, uint32 * new_step)
{
    int32 i, b;
    int32 bit_masked_result;
    int32 next_bit_masked_result;
    int32 bit;
    int32 pass_bit_window[16];
    int32 start;
    int32 finish;
    int32 this_step;
    int32 pass_start[16];
    int32 fail_start[16];
    int32 pass_window_sum[2];
    int32 avg_pass_window[2];
    int32 adjustment[2];

    /* determine pass window by finding start of passes and end of passes */
    /*
             0                              63
             ------++++++++++++++++++--------
                   ^                 ^
                   |                 |
               pass_start        fail_start
     */

    /* spliting the visable pass window */
    for (bit = 0; bit < 16; ++bit) {
        pass_bit_window[bit] = 0;
        i = 0;
        start = 1;
        finish = 1;
        /* cycle thru bit value until 1st pass is hit */
        while ( start == 1 ) {
            bit_masked_result = result[i] & (1 << bit);
            next_bit_masked_result = result[i+1] & (1 << bit);
            if ((bit_masked_result == 0) && (next_bit_masked_result == 0)) {
                pass_start[bit] = i;
                start = 0;
            }
            i = i + 1;
            if ( i >= 63 ) {
                pass_start[bit] = ( result[i] & ( 1 << bit ) )? 64 : 63;
                i++;
                break;
            }
        }
        if ( i > 63 ) {
            fail_start[bit] = 64;
            finish = 0;
        }
        while ( finish == 1 ) {
            bit_masked_result = result[i] & (1 << bit);
            next_bit_masked_result = result[i+1] & (1 << bit);
            if (((bit_masked_result != 0) && (next_bit_masked_result != 0)) || (i == 63)) {
                fail_start[bit] = i;
                finish = 0;
            }
            i = i + 1;
            if (i >= 63) {
                fail_start[bit] = 64;
                break;
            }
        }
        pass_bit_window[bit] = fail_start[bit] - pass_start[bit];
    }

    for ( b = 0; b < 2; b++ ) {
        pass_window_sum[b] = 0;
        for ( bit = b*8; bit < (b+1)*8; bit++ ) {
            pass_window_sum[b] += pass_bit_window[bit];
        }
        avg_pass_window[b] = pass_window_sum[b] >> 3;
        adjustment[b] = avg_pass_window[b] >> 1; 
        this_step = fail_start[b*8] - adjustment[b];
        if (this_step < 0) {
            new_step[b] = 0;
        } else if (this_step > 63) {
            new_step[b] = 63;
        } else {
            new_step[b] = this_step;
        }
    }
    return SOC_E_NONE;
}

/* Calibrate the read dq vdl values */
STATIC int
_calib_rd_dq( uint32 * result, uint32 * init_step, uint32 * new_step )
{
    int32 i;
    int32 bit_masked_result;
    int32 next_bit_masked_result;
    int32 next2_bit_masked_result;
    int32 next3_bit_masked_result;
    int32 next4_bit_masked_result;
    int32 next5_bit_masked_result;
    int32 bit;
    int32 pass_bit_window[16];
    int32 start;
    int32 finish;
    int32 this_step;
    int32 pass_start[16];
    int32 fail_start[16];

    /* determine pass window by finding start of passes and end of passes */

    /* spliting the visable pass window */
    for (bit = 0; bit < 16; ++bit) {
        pass_bit_window[bit] = 0;
        i = 0;
        start = 1;
        finish = 1;

        /* cycle thru bit value until 1st pass is hit */
        while (start == 1) {
            bit_masked_result = result[i] & (1 << bit);
            next_bit_masked_result = result[i+1] & (1 << bit);
            next2_bit_masked_result = result[i+2] & (1 << bit);
            next3_bit_masked_result = result[i+3] & (1 << bit);
            next4_bit_masked_result = result[i+4] & (1 << bit);
            next5_bit_masked_result = result[i+5] & (1 << bit);
            if ((bit_masked_result == 0) && 
                (next_bit_masked_result == 0) && 
                (next2_bit_masked_result == 0) && 
                (next3_bit_masked_result == 0) && 
                (next4_bit_masked_result == 0) && 
                (next5_bit_masked_result == 0)) {
                pass_start[bit] = i;
                start = 0;
            }
            i = i + 1;
            if (i >= 63) {
                pass_start[bit] = 64;
                i++;
            break;
            }
        }
        if (i > 63) {
            fail_start[bit] = 64;
            finish = 0;
        }

        while (finish == 1) {
            bit_masked_result = result[i] & (1 << bit);
            next_bit_masked_result = result[i+1] & (1 << bit);
            if (((bit_masked_result != 0) && (next_bit_masked_result != 0)) || (i == 63)) {
                fail_start[bit] = i;
                finish = 0;
            }
            i = i + 1;
            if (i >= 63) {
                fail_start[bit] = 64;
                break;
            }
        }

        pass_bit_window[bit] = fail_start[bit] - pass_start[bit];
    }

    for (bit = 0; bit < 16; ++bit) {
        this_step = pass_bit_window[bit] >> 1;
        this_step += pass_start[bit];
        if (this_step < 0) {
            new_step[bit] = 0;
        } else if (this_step > 63) {
            new_step[bit] = 63;
        } else {
            new_step[bit] = this_step;
        }
    }
    return SOC_E_NONE;
}

/* Calibrate the write data mask vdl values */
STATIC int
_calib_wr_dm(int unit, int ci, uint32 * result, uint32 * init_step, uint32 * new_step )
{
    uint32 data;
    int32 i;
    int32 more_than;
    int32 bit_masked_result;
    int32 next_bit_masked_result;
    int32 byte;
    int32 fail_bit_window[4];
    int32 start;
    int32 pass;
    int32 hit_fail;
    int32 how_many;
    int32 this_step;
    int32 fail_start[4];
    int32 adjustment=0;
    int32 ticks_per_half_clock;
    int32 ticks_per_clock;

    more_than = 3;

    /* determine pass window by counting fails & subtracting from bit time */

    for (byte = 0; byte < 2; ++byte) {
        fail_bit_window[byte] = 0;
        i = 0;
        start = 1;
        pass = 0;
        hit_fail = 1;
        how_many = 0;

        /* cycle thru bit value until 1st pass is hit */
        while (pass == 0) {
            bit_masked_result = result[i] & (1 << (byte * 8));
            next_bit_masked_result = result[i+1] & (1 << (byte * 8));
            if ((bit_masked_result == 0) && (next_bit_masked_result == 0)) {
                pass = 1;
            }
            i = i + 1;
            if ( i >= 63 ) {
                i++;
                break;
            }
        }
        if (i >= 64)
            continue;

        /* continue cycling thru bit value until 1st failure in a window is hit */
        while (start == 1) {
            bit_masked_result = result[i] & (1 << (byte * 8));
            next_bit_masked_result = result[i+1] & (1 << (byte * 8));
            if ((bit_masked_result != 0) && (next_bit_masked_result != 0)) {
                fail_bit_window[byte] = fail_bit_window[byte] + 1;
                fail_start[byte] = i - 1;
                how_many = how_many + 1;
                start = 0;
            }
            i = i + 1;
            if ( i >= 63 ) {
                i++;
                break;
            }
        }

        if (i >= 64)
            continue;

        /* count how wide the failure window */
        while (pass == 1) {
            if (i > 63)
                pass = 0;
            bit_masked_result = result[i] & (1 << (byte * 8));
            if (bit_masked_result != 0) {
                hit_fail = 0;
                fail_bit_window[byte] = fail_bit_window[byte] + 1;
                how_many = how_many + 1;
            } else {
                hit_fail = 1;
            }
            if (hit_fail == 1) {
                if (how_many > more_than) pass = 0;
                else how_many = 0;
            }
            i = i + 1;
        }
    }

    for (byte = 0; byte < 2; ++byte) {
        if (fail_bit_window[byte] == 0) {
            fail_start[byte] = 63;
        }
    }

    /* get ticks_per_clock from PHY CR VDL_CALIB_STATUS reg */
    READ_DDR40_PHY_CONTROL_REGS_VDL_CALIB_STATUSr(unit, ci, &data);
    ticks_per_clock = DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_CALIB_STATUS, CALIB_TOTAL);

    /* division by 2 using looping subtraction */
    i = ticks_per_clock;
    ticks_per_half_clock = 0;;
    while (i >= 2) {
        i = i - 2;
        ticks_per_half_clock = ticks_per_half_clock + 1;
    }

    for (byte = 0; byte < 2; ++byte) {
#ifdef WR_33PER_CALIB
        /* division by 3 using looping subtraction */
        i = ticks_per_half_clock - fail_bit_window[byte];
        adjustment = 0;;
        while (i >= 3) {
            i = i - 3;
            adjustment = adjustment + 1;
        }
        adjustment = adjustment * 2;
#endif  

#ifdef WR_50PER_CALIB
        /* division by 2 using looping subtraction */
        i = ticks_per_half_clock - fail_bit_window[byte];
        adjustment = 0;;
        while (i >= 2) {
            i = i - 2;
            adjustment = adjustment + 1;
        }
#endif  

        this_step = fail_start[byte] - adjustment;
        if (this_step < 0) {
            new_step[byte] = 0;
        } else if (this_step > 63) {
            new_step[byte] = 63;
        } else {
            new_step[byte] = this_step;
        }
    }
    return SOC_E_NONE;
}

/* Get Init steps                                                                                                           */
int
soc_ddr40_shmoo_get_init_step(int unit, int ci, int wl, int type, uint32 * init_step)
{
    uint32 data;
    uint32 calib_total;
    int byte, bit, en, n;

    switch (type) {
        case SHMOO_RD_EN:
            READ_DDR40_PHY_CONTROL_REGS_VDL_RD_EN_CALIB_STATUSr(unit, ci, &data);
            calib_total = DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_RD_EN_CALIB_STATUS, RD_EN_CALIB_TOTAL) >> 4;
            if(wl == 0) {
                READ_DDR40_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_BIT_RD_ENr(unit, ci, &data); /* Byte 0 */
                init_step[0] = (DDR40_GET_FIELD(data, DDR40_PHY_WORD_LANE_0, VDL_OVRIDE_BYTE0_BIT_RD_EN, OVR_EN)) ?
                            DDR40_GET_FIELD(data, DDR40_PHY_WORD_LANE_0, VDL_OVRIDE_BYTE0_BIT_RD_EN, OVR_STEP) :
                            calib_total;
                READ_DDR40_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_BIT_RD_ENr(unit, ci, &data); /* Byte 1 */
                init_step[1] = (DDR40_GET_FIELD(data, DDR40_PHY_WORD_LANE_0, VDL_OVRIDE_BYTE1_BIT_RD_EN, OVR_EN)) ?
                            DDR40_GET_FIELD(data, DDR40_PHY_WORD_LANE_0, VDL_OVRIDE_BYTE1_BIT_RD_EN, OVR_STEP) :
                            calib_total;
            } else {
                READ_DDR40_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE0_BIT_RD_ENr(unit, ci, &data); /* Byte 0 */
                init_step[0] = (DDR40_GET_FIELD(data, DDR40_PHY_WORD_LANE_1, VDL_OVRIDE_BYTE0_BIT_RD_EN, OVR_EN)) ?
                            DDR40_GET_FIELD(data, DDR40_PHY_WORD_LANE_1, VDL_OVRIDE_BYTE0_BIT_RD_EN, OVR_STEP) :
                            calib_total;
                READ_DDR40_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE1_BIT_RD_ENr(unit, ci, &data); /* Byte 1 */
                init_step[1] = (DDR40_GET_FIELD(data, DDR40_PHY_WORD_LANE_1, VDL_OVRIDE_BYTE1_BIT_RD_EN, OVR_EN)) ?
                            DDR40_GET_FIELD(data, DDR40_PHY_WORD_LANE_1, VDL_OVRIDE_BYTE1_BIT_RD_EN, OVR_STEP) :
                            calib_total;
            }
            if(wl == 0) {
                READ_DDR40_PHY_WORD_LANE_0_READ_DATA_DLYr(unit, ci, &data); 
                init_step[2] = data;
            } else {
                READ_DDR40_PHY_WORD_LANE_1_READ_DATA_DLYr(unit, ci, &data); 
                init_step[2] = data;
            }
            break;
        case SHMOO_RD_DQ:
            READ_DDR40_PHY_CONTROL_REGS_VDL_DQ_CALIB_STATUSr(unit, ci, &data); 
            calib_total = DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_DQ_CALIB_STATUS, DQ_CALIB_TOTAL) >> 4;
            n = 0;
            for ( byte = 0; byte < 2; byte++ ) {
                for (bit = 0; bit < 8; bit++ ) {
                    READ_DDR40_PHY_WORD_LANE_x_VDL_OVRIDE_BYTEy_BITz_R_Pr(wl,byte,bit,unit,ci,&data);
                    en = DDR40_GET_FIELD(data, DDR40_PHY_WORD_LANE_0, VDL_OVRIDE_BYTE0_BIT0_R_P, OVR_EN);
                    init_step[n] = (en)? GET_FIELD(data, DDR40_PHY_WORD_LANE_0, VDL_OVRIDE_BYTE0_BIT0_R_P, OVR_STEP) : calib_total;
                    n++;
                }
            }
            break;
        case SHMOO_WR_DQ:
            READ_DDR40_PHY_CONTROL_REGS_VDL_WR_CHAN_CALIB_STATUSr(unit, ci, &data);
            calib_total = DDR40_GET_FIELD( data, DDR40_PHY_CONTROL_REGS, VDL_WR_CHAN_CALIB_STATUS, WR_CHAN_CALIB_TOTAL ) >> 4;
            n = 0;
            for (byte = 0; byte < 2; byte++ ) {
                for (bit = 0; bit < 8; bit++ ) {
                    READ_DDR40_PHY_WORD_LANE_x_VDL_OVRIDE_BYTEy_BITz_Wr(wl, byte, bit, unit,ci,&data);
                    en = DDR40_GET_FIELD(data, DDR40_PHY_WORD_LANE_0, VDL_OVRIDE_BYTE0_BIT0_W, OVR_EN);
                    init_step[n] = ( en )? DDR40_GET_FIELD( data, DDR40_PHY_WORD_LANE_0, VDL_OVRIDE_BYTE0_BIT0_W, OVR_STEP) : calib_total;
                    n++;
                }
            }
            break;
        case SHMOO_ADDRC:
            break;
        case SHMOO_WR_DM:
            READ_DDR40_PHY_CONTROL_REGS_VDL_WR_CHAN_CALIB_STATUSr(unit, ci, &data);
            calib_total = DDR40_GET_FIELD(data, DDR40_PHY_CONTROL_REGS, VDL_WR_CHAN_CALIB_STATUS, WR_CHAN_CALIB_TOTAL) >> 4;
            if(wl == 0) {
                READ_DDR40_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_DM_Wr(unit, ci, &data);
                init_step[0] = (DDR40_GET_FIELD(data, DDR40_PHY_WORD_LANE_0, VDL_OVRIDE_BYTE0_DM_W, OVR_EN))?
                            DDR40_GET_FIELD(data, DDR40_PHY_WORD_LANE_0, VDL_OVRIDE_BYTE0_DM_W, OVR_STEP) :
                            calib_total;
                READ_DDR40_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_DM_Wr(unit, ci, &data);
                init_step[1] = (DDR40_GET_FIELD(data, DDR40_PHY_WORD_LANE_0, VDL_OVRIDE_BYTE1_DM_W, OVR_EN))?
                            DDR40_GET_FIELD(data, DDR40_PHY_WORD_LANE_0, VDL_OVRIDE_BYTE1_DM_W, OVR_STEP) :
                            calib_total;
            } else {
                READ_DDR40_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE0_DM_Wr(unit, ci, &data);
                init_step[0] = (DDR40_GET_FIELD(data, DDR40_PHY_WORD_LANE_1, VDL_OVRIDE_BYTE0_DM_W, OVR_EN))?
                            DDR40_GET_FIELD(data, DDR40_PHY_WORD_LANE_1, VDL_OVRIDE_BYTE0_DM_W, OVR_STEP) :
                            calib_total;
                READ_DDR40_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE1_DM_Wr(unit, ci, &data);
                init_step[1] = (DDR40_GET_FIELD(data, DDR40_PHY_WORD_LANE_1, VDL_OVRIDE_BYTE1_DM_W, OVR_EN))?
                            DDR40_GET_FIELD(data, DDR40_PHY_WORD_LANE_1, VDL_OVRIDE_BYTE1_DM_W, OVR_STEP) :
                            calib_total;
            }
            break;
        default:
            break;
    }
    return SOC_E_NONE;
}


/* Shmoo call step                                                                                                          */
int
soc_ddr40_shmoo_do(int unit, int ci, int wl, int type, int preset, int dqs_steps,  uint32 * result, uint32 * init_step,int use_addr,uint32 dq_addr, int dq_burst, int dq_iter, int use_dq, uint32 dq_data0, uint32 dq_data0_alt)
{
    switch (type) {
        case SHMOO_INIT_VDL_RESULT:
            return _shmoo_init_vdl_result(unit,ci,wl,result,preset, dqs_steps, 1,use_addr,dq_addr, dq_burst,dq_iter, use_dq, dq_data0, dq_data0_alt);
        case SHMOO_RD_EN:
            return _shmoo_rd_en_rd_data_dly(unit,ci,wl,result,1,use_addr,dq_addr, dq_burst,dq_iter, use_dq, dq_data0, dq_data0_alt); 
        case SHMOO_RD_DQ:
            return _shmoo_rd_dq(unit,ci,wl,result,1,use_addr,dq_addr, dq_burst,dq_iter, use_dq, dq_data0, dq_data0_alt);
        case SHMOO_WR_DQ:
            return _shmoo_wr_dq(unit,ci,wl,result,1,use_addr,dq_addr, dq_burst,dq_iter, use_dq, dq_data0, dq_data0_alt);
        case SHMOO_ADDRC:
            return _shmoo_addr(unit,ci,wl,result,1, use_addr,dq_addr, dq_burst,dq_iter, use_dq, dq_data0, dq_data0_alt);
        case SHMOO_WR_DM:
            return _shmoo_wr_dm(unit,ci,wl,result);
    }
    return SOC_E_PARAM;
}

/* Calibration step                                                                                                         */
int
soc_ddr40_shmoo_calib(int unit, int ci, int type, uint32 * result, uint32 * init_step, uint32 * new_step)
{
    switch (type) {
        case SHMOO_INIT_VDL_RESULT:
            return SOC_E_NONE;
        case SHMOO_RD_EN:
            return _calib_rd_en(result, init_step, new_step);
        case SHMOO_RD_DQ:
            /* Reuse */
            return _calib_rd_dq(result, init_step, new_step);
        case SHMOO_WR_DQ:
            /* Reuse */
            return _calib_rd_dq(result, init_step, new_step);
        case SHMOO_ADDRC:
            /* Reuse */
            return _calib_rd_dq(result, init_step, new_step);
        case SHMOO_WR_DM:
            return _calib_wr_dm(unit, ci, result, init_step, new_step);
    }
    return SOC_E_PARAM;
}

/* Plot shmoo with the pass/fail result values and the new steps */
int 
soc_ddr40_shmoo_plot(int type, int wl, uint32 * result, uint32 * step)
{
    int i, x;
    int bit, bit_count;
    int byte_plot;
    char str[65];

    str[64] = 0;

    soc_cm_print( "\n" );

    switch (type) {
        case SHMOO_INIT_VDL_RESULT:
            return SOC_E_NONE; 
            break;
        case SHMOO_RD_EN:
            byte_plot = 1; 
            soc_cm_print( "**** RD_EN  Shmoo (WL=%d)\n", wl );
            break;
        case SHMOO_RD_DQ:
            byte_plot = 1; 
            soc_cm_print( "**** RD_DQ  Shmoo (WL=%d)\n", wl );
            break;
        case SHMOO_WR_DQ:
            byte_plot = 1; 
            soc_cm_print( "**** WR_DQ  Shmoo (WL=%d)\n", wl );
            break;
        case SHMOO_ADDRC:
            byte_plot = 0; 
            if (wl==1) return SOC_E_NONE; 
            soc_cm_print( "**** ADDRC  Shmoo (WL=%d)\n", wl );
            break;
        case SHMOO_WR_DM:
            byte_plot = 1; 
            soc_cm_print( "**** WR_DM  Shmoo (WL=%d)\n", wl );
            break;
        default:
            byte_plot = 0; 
            soc_cm_print( "**** ?????  Shmoo (WL=%d)\n", wl );
            break;
    }
    soc_cm_print( "\n" );

    for (i = 0; i < 64; i++) {
        str[i] = '0' + (i/10);
    }
    soc_cm_print( "    %s\n", str );

    for ( i = 0; i < 64; i++ ) {
        str[i] = '0' + (i%10);
    }
    soc_cm_print( "    %s\n", str );

    bit_count = (type == SHMOO_ADDRC)? 1 : 16;

    for (bit = 0; bit < bit_count; bit++) {
        for ( i = 0; i < 64; i++ ) {
            x = (byte_plot)? (bit/8) : bit;
            if (i == step[x]) {
                str[i] = 'X';
            } else {
                str[i] = (result[i] & (1 << bit))? '-' : '+';
            }
        }
        soc_cm_print( " %02d %s\n", bit, str );
    }
    soc_cm_print( "\n" );
    return SOC_E_NONE;
}

int
soc_ddr40_shmoo_set_new_step(int unit, int ci, int wl, int type, uint32 * new_step) {
    uint32 data;
    int byte, bit, n;

    switch (type) {
        case SHMOO_RD_EN:
            if(wl == 0) {
                n = (new_step[0]<25) ? new_step[0] : 15 ;
                data = SET_OVR_STEP(n);
                WRITE_DDR40_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_BIT_RD_ENr(unit, ci, data);
                n = (new_step[1]<25) ? new_step[1] : 15 ;
                data = SET_OVR_STEP(n);
                WRITE_DDR40_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_BIT_RD_ENr(unit, ci, data);
            } else {
                n = (new_step[0]<40) ? new_step[0] : 25 ;
                data = SET_OVR_STEP(n);
                WRITE_DDR40_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE0_BIT_RD_ENr(unit, ci, data);
                n = (new_step[1]<43) ? new_step[1] : 25 ;
                data = SET_OVR_STEP(n);
                WRITE_DDR40_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE1_BIT_RD_ENr(unit, ci, data);
            }
            break;
        case SHMOO_RD_DQ:
            n = 0;
            for (byte = 0; byte < 2; byte++ ) {
                for (bit = 0; bit < 8; bit++ ) {
                    data = SET_OVR_STEP(new_step[n]);
                    WRITE_DDR40_PHY_WORD_LANE_x_VDL_OVRIDE_BYTEy_BITz_R_Pr(wl,byte,bit,unit,ci,data);
                    WRITE_DDR40_PHY_WORD_LANE_x_VDL_OVRIDE_BYTEy_BITz_R_Nr(wl,byte,bit,unit,ci,data);
                    n++;
                }
            }
            break;
        case SHMOO_WR_DQ:
            n = 0;
            for (byte = 0; byte < 2; byte++ ) {
                for(bit = 0; bit < 8; bit++ ) {
                    data = SET_OVR_STEP(new_step[n]);
                    WRITE_DDR40_PHY_WORD_LANE_x_VDL_OVRIDE_BYTEy_BITz_Wr(wl, byte, bit, unit,ci,data);
                    n++;
                }
            }
            break;
        case SHMOO_ADDRC:
            n = 0;
            if (wl==0) {
                data = SET_OVR_STEP(new_step[n]);
                DDR40_SET_FIELD(data,DDR40_PHY_CONTROL_REGS,VDL_OVRIDE_BIT_CTL,BYTE_SEL,1);
                WRITE_DDR40_PHY_CONTROL_REGS_VDL_OVRIDE_BIT_CTLr(unit, ci, data); 
            }
            break;
        case SHMOO_WR_DM:
            if(wl == 0) {
                data = SET_OVR_STEP(new_step[0]);
                WRITE_DDR40_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_DM_Wr(unit, ci, data);
                data = SET_OVR_STEP(new_step[1]);
                WRITE_DDR40_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_DM_Wr(unit, ci, data);
            } else {
                data = SET_OVR_STEP(new_step[0]);
                WRITE_DDR40_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE0_DM_Wr(unit, ci, data);
                data = SET_OVR_STEP(new_step[1]);
                WRITE_DDR40_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE1_DM_Wr(unit, ci, data);
            }
            break;
        default:
            break;
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *	soc_ddr_test
 * Purpose:
 *	Perform DDR test related functions
 * Parameters:
 *	unit - StrataSwitch unit #
 *	test_no - test function to perform
 *	tp - test parameters
 * Returns:
 *	SOC_E_XXX
 * Notes:
 *	Provides a single entrance point for application level code.
 */
int
soc_ddr_test(int unit, int test_no, soc_ddr_test_param_t *tp)
{
    switch (test_no) {
    case SOC_DDR_TEST_SELF_1:
        return soc_test_func_self_test(unit, tp->ci, tp->wl, tp->loop,
                                       tp->use_addr, tp->dq_addr, tp->dq_burst,
                                       tp->dq_iter, tp->use_dq,
                                       tp->dq_data0, tp->dq_data0_alt);
    case SOC_DDR_TEST_SELF_2:
        return soc_test_two_func_self_test(unit, tp->ci, tp->wl, tp->loop,
                                           tp->use_addr, tp->dq_addr, tp->dq_burst,
                                           tp->dq_iter, tp->use_dq,
                                           tp->dq_data0, tp->dq_data0_alt);
    case SOC_DDR_TEST_SHMOO_PREP:
        return _soc_ddr_shmoo_prepare_for_shmoo(unit, tp->ci);
    default:
        break;
    }
    return SOC_E_PARAM;
}
#endif /* DDR3 Support */
