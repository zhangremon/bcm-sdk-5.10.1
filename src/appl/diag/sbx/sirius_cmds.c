/*
 * $Id: sirius_cmds.c 1.25.2.3 Broadcom SDK $
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
 * File:        sirius_cmds.c
 * Purpose:     SIRIUS-specific diagnostic shell commands
 * Requires:
 */


#include <sal/core/libc.h>
#include <shared/alloc.h>
#include <soc/defs.h>
#include <soc/cm.h>
#include <soc/phy/ddr23.h>

#ifdef BCM_SIRIUS_SUPPORT
#include <soc/sbx/sirius.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/port.h>
#include <soc/sbx/sirius_ddr23.h>
#include <appl/diag/shell.h>
#include <appl/diag/system.h>
#include <bcm/cosq.h>

static int
sbx_sirius_ddr_reset_read_fifo(int unit, int ci);

static int
sbx_sirius_ddr_check_read_fifo_status(int unit, int ci);

char cmd_sbx_sirius_ddr_mem_write_usage[] = "\n"
" DDRMemWrite ci<n> range=0xstart-[0xend] data=0xdata\n"
" DDRMemWrite ci0,ci1 range=0x0\n"
" DDRMemWrite ci9 range=0x0-0x100"
"\n";


cmd_result_t
cmd_sbx_sirius_ddr_mem_write(int unit, args_t *a)
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

  if (!SOC_IS_SBX_SIRIUS(unit)) {
    printk("only supported on sirius\n");
    return CMD_FAIL;
  }

  if (((c = ARG_GET(a)) == NULL) || (parse_bcm_pbmp(unit, c, &ci_pbm) < 0)) {
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

 /* 
  * Need to check max value allowed for debug leave it at field max.
  */

  if (diag_parse_range(lo,hi,&start_addr,&end_addr,0,1<<21)) {
    printk("Invalid range. Valid range is : 0 - 0x%x\n",(1<<21));
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
      bank = SIRIUS_DDR23_BANK(addr);
      col  = SIRIUS_DDR23_COL(addr);
      row  = SIRIUS_DDR23_ROW(addr);
      /* for debug */
      printk("Writing to ci%d,bank[%d],row[0x%04x],cols[0x%03x - 0x%03x] 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
	     ci,bank,row,col,col+0xf,data_wr[0],data_wr[1],data_wr[2],data_wr[3],data_wr[4],data_wr[5],data_wr[6],data_wr[7]);

      rv = soc_sbx_sirius_ddr23_write(unit,
				      ci,
				      addr,
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

char cmd_sbx_sirius_ddr_mem_read_usage[] = "\n"
" DDRMemRead ci<n> range=0xstart-[0xend]\n"
" DDRMemRead ci0,ci1 range=0x0\n"
" DDRMemRead ci9 range=0x0-0x100"
"\n";

cmd_result_t
cmd_sbx_sirius_ddr_mem_read(int unit, args_t *a)
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

  if (!SOC_IS_SBX_SIRIUS(unit)) {
    printk("only supported on sirius\n");
    return CMD_FAIL;
  }

  if (((c = ARG_GET(a)) == NULL) || (parse_bcm_pbmp(unit, c, &ci_pbm) < 0)) {
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


 /* 
  * Need to check max value allowed for debug leave it at field max.
  */
  
  if (diag_parse_range(lo,hi,&start_addr,&end_addr,0,1<<21)) {
    printk("Invalid range. Valid range is : 0 - 0x%x\n",(1<<21));
    return CMD_FAIL;
  }

  SOC_PBMP_ITER(ci_pbm,ci) {
    printk("Reading ci%d DDR %s ..\n",ci,lo);
    for (addr=start_addr; addr <= end_addr; addr++) {
      rv = soc_sbx_sirius_ddr23_read(unit,
				     ci,
				     addr,
				     &data_rd[0],&data_rd[1],&data_rd[2],
				     &data_rd[3],&data_rd[4],&data_rd[5],
				     &data_rd[6],&data_rd[7]);

      if (rv == BCM_E_NONE) {
	/* convert the dram addr to pla_addr, to show bank,row,col */
	bank = SIRIUS_DDR23_BANK(addr);
	col  = SIRIUS_DDR23_COL(addr);
	row  = SIRIUS_DDR23_ROW(addr);
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


char cmd_sbx_sirius_ddr_phy_read_usage[] = "\n"
" DDRPhyRead ci (all)\n"
" DDRPhyRead ci0     \n"
" DDRPhyRead ci0,ci1 \n";

/* for debug */
cmd_result_t
cmd_sbx_sirius_ddr_phy_read(int unit, args_t *a)
{

  uint32 data[4] = {0};
  int addr = 0;
  int reg = 0;
  int ci = 0;
  char *c = NULL;
  soc_pbmp_t ci_pbm;

  if (!SOC_IS_SBX_SIRIUS(unit)) {
    printk("only supported on sirius\n");
    return CMD_FAIL;
  }

  if (((c = ARG_GET(a)) == NULL) || (parse_bcm_pbmp(unit, c, &ci_pbm) < 0)) {
    return CMD_USAGE;
  }

  SOC_PBMP_ITER(ci_pbm,ci) {
    /* ADDR_CTL */
    printk("CI%d ( DDR23_PHY_ADDR_CTL )\n",ci);
    for (addr=DDR23_PHY_ADDR_CTL_MIN; addr <= DDR23_PHY_ADDR_CTL_MAX;) {
      for (reg=0;reg<4 && addr <= DDR23_PHY_ADDR_CTL_MAX;reg++) {
	if (DDR23_REG_READ(unit,ci,0/*flags*/,addr,&data[reg]) != SOC_E_NONE) {
	  printk("failed to read phy register\n");
	  return CMD_FAIL;
	}
	printk("    0x%04x: 0x%08x ",addr,data[reg]); 
	if (addr == 0x04) addr+= 0x08;
	addr += 0x04;
      }
      printk("\n");
    }

    /* BYTE_LANE0 */
    printk("CI%d ( DDR23_PHY_BYTE_LANE0 )\n",ci);
    for (addr=DDR23_PHY_BYTE_LANE0_ADDR_MIN; addr <= DDR23_PHY_BYTE_LANE0_ADDR_MAX;) {
      for (reg=0;reg<4 && addr <= DDR23_PHY_BYTE_LANE0_ADDR_MAX;reg++) {
	if (DDR23_REG_READ(unit,ci,0/*flags*/,addr,&data[reg]) != SOC_E_NONE) {
	  printk("failed to read phy register\n");
	  return CMD_FAIL;
	}
	printk("    0x%04x: 0x%08x ",addr & 0xff ,data[reg]); 
	if (addr == 0x108) addr+= 0x04;
	addr += 0x04;
      }
      printk("\n");
    }

    /* BYTE_LANE1 */
    printk("CI%d ( DDR23_PHY_BYTE_LANE1 )\n",ci);
    for (addr=DDR23_PHY_BYTE_LANE1_ADDR_MIN; addr <= DDR23_PHY_BYTE_LANE1_ADDR_MAX;) {
      for (reg=0;reg<4 && addr <= DDR23_PHY_BYTE_LANE1_ADDR_MAX;reg++) {
	if (DDR23_REG_READ(unit,ci,0/*flags*/,addr,&data[reg]) != SOC_E_NONE) {
	  printk("failed to read phy register\n");
	  return CMD_FAIL;
	}
	printk("    0x%04x: 0x%08x ",addr & 0x1ff ,data[reg]); 
	if (addr == 0x208) addr+= 0x04;
	addr += 0x04;
      }
      printk("\n");
    }
  }

  return CMD_OK;
}

char cmd_sbx_sirius_ddr_phy_write_usage[] = "\n"
" DDRPhyWrite ci<n> block=<block> offset=<0xoffset> data=<0xdata>\n"
" Examples: Write to all ci's bytelane0\n"
" DDRPhyWrite ci block=1 offset=0x040 data=0x0\n"
" Write to ci0,1 bytelane0\n"
" DDRPhyWrite ci0,ci1 block=1 offset=0x040 data=0x0\n"
" <block> = 0,1,2 (addr_ctrl,bytelane0,bytelane1)\n";

/* for debug */
cmd_result_t
cmd_sbx_sirius_ddr_phy_write(int unit, args_t *a)
{

  uint32 data = 0;
  int addr = 0;
  int block = 0;
  int ci = 0;
  int offset = 0;
  parse_table_t pt;
  int ret_code;
  int rv;
  char *c = NULL;
  soc_pbmp_t ci_pbm;

  if (!SOC_IS_SBX_SIRIUS(unit)) {
    printk("only supported on sirius\n");
    return CMD_FAIL;
  }

  if (((c = ARG_GET(a)) == NULL) || (parse_bcm_pbmp(unit, c, &ci_pbm) < 0)) {
    return CMD_USAGE;
  }  
  
  /* be sure all below args are entered before writing out anything */
  if (ARG_CNT(a) == 3) {
    parse_table_init(0,&pt);
    parse_table_init(0,&pt);
    parse_table_add(&pt,"block",PQ_INT,
		    (void *) (0), &block, NULL);
    parse_table_add(&pt,"offset",PQ_INT,
		    (void *) (0), &offset, NULL);
    parse_table_add(&pt,"data",PQ_INT,
		    (void *) (0), &data, NULL);
    if (!parseEndOk(a,&pt,&ret_code)) {
      return ret_code;
    }

  } else {
    printk("Invalid number of args.\n");
    return CMD_USAGE;
  }

  switch(block) {
  case DDR23_PHY_ADDR_CTL_BLOCK:
    addr = DDR23_PHY_ADDR_CTL_MIN + offset;
    if (addr > DDR23_PHY_ADDR_CTL_MAX) {
      printk("Offset=0x%04x too large for addr_ctl(max=0x%04x)\n",
	     offset,DDR23_PHY_ADDR_CTL_MAX);
      return CMD_FAIL;
    }
    break;
  case DDR23_PHY_BYTE_LANE0_BLOCK:
    addr = DDR23_PHY_BYTE_LANE0_ADDR_MIN + offset;
    if (addr > DDR23_PHY_BYTE_LANE0_ADDR_MAX) {
      printk("Offset=0x%04x too large for bytelane0(max=0x%04x)\n ",
	     offset,DDR23_PHY_BYTE_LANE0_ADDR_MAX & 0xff);
      return CMD_FAIL;
    }
    break;
  case DDR23_PHY_BYTE_LANE1_BLOCK:
    addr = DDR23_PHY_BYTE_LANE1_ADDR_MIN + offset;
    if (addr > DDR23_PHY_BYTE_LANE1_ADDR_MAX) {
      printk("Offset=0x%04x too large for bytelane1(max=0x%04x))\n",
	     offset,DDR23_PHY_BYTE_LANE1_ADDR_MAX & 0x1ff);
      return CMD_FAIL;
    }
    break;
  default:
    printk("Bad block id (%d) (0,1,2 valid) \n",block);
    return CMD_USAGE;
  }

  SOC_PBMP_ITER(ci_pbm,ci) {
    if (( rv = DDR23_REG_WRITE(unit,ci,0x0/*flags*/,addr,data)) < 0) {
      printk("Writing 0x%08x to ci:%d block(%d) addr=0x%08x failed.\n",
	     data,ci,block,addr);
      return CMD_FAIL;
    }
  }

  return CMD_OK;
}

char cmd_sbx_sirius_ddr_phy_tune_usage[] = "\n"
" DDRPhyTune ci (all) tuning_mode=<0-6> read_vdl=<0-63> read_en_vdl=<0-63> addr_vdl=<0-63> write_vdl=<0-63> verify_mode=<0-1>\n"
" DDRPhyTune ci0     \n"
" DDRPhyTune ci0,ci1 \n"
" verify_mode = 1 (default) use indirect memory read/write to verify if setting works\n"
" verify_mode = 0 use ddr functional tests to verify if setting works\n"
" verify_mode = 0 is much slower than verify_mode = 1 and detect more wrong settings\n";

/* for debug */
cmd_result_t
cmd_sbx_sirius_ddr_phy_tune(int unit, args_t *a)
{

  int ci = 0;
  char *c = NULL;
  soc_pbmp_t ci_pbm;
  int tuning_mode;
  int read_vdl=-1, read_en_vdl=-1, addr_vdl=-1, write_vdl=-1;
  int read_vdl_tmp=-1, read_en_vdl_tmp=-1, addr_vdl_tmp=-1, write_vdl_tmp=-1;
  uint32 uRegValue;
  uint32 predefined_addr = 0;
  uint32 vdl_start, vdl_end, vdl_start2, vdl_end2, vdl_start3, vdl_end3;
  uint32 _sirius_init_timeout;
  uint32 uDataWR[8] = {0};
  uint32 uDataRD[8] = {0};
  uint32 i, j, bank;
  int rseed = 0x12345678;
  uint32 uAddr = 0x0;
  uint32 wr_status = SOC_E_NONE;
  uint32 rd_status = SOC_E_NONE;
  uint32 fifo_status = SOC_E_NONE;
  uint32 random_write = FALSE;
  char passed[64][64];
  int rv, verify_mode;

  if (SAL_BOOT_QUICKTURN) {
    _sirius_init_timeout = 2000 * MILLISECOND_USEC;
  } else {
    _sirius_init_timeout = 20 * MILLISECOND_USEC;
  }


  if (!SOC_IS_SBX_SIRIUS(unit)) {
    printk("only supported on sirius\n");
    return CMD_FAIL;
  }

  if (((c = ARG_GET(a)) == NULL) || (parse_bcm_pbmp(unit, c, &ci_pbm) < 0)) {
    return CMD_USAGE;
  }

  uDataWR[0] = 0x01020304;

  if (ARG_CNT(a)) {
    int ret_code;
    parse_table_t pt;
    parse_table_init(0, &pt);
    parse_table_add(&pt, "tuning_mode", PQ_DFL | PQ_INT,
		    (void *)-1, &tuning_mode, NULL);
    parse_table_add(&pt, "read_vdl", PQ_DFL | PQ_INT,
		    (void *)-1, &read_vdl, NULL);
    parse_table_add(&pt, "read_en_vdl", PQ_DFL | PQ_INT,
		    (void *)-1, &read_en_vdl, NULL);
    parse_table_add(&pt, "write_vdl", PQ_DFL | PQ_INT,
		    (void *)-1, &write_vdl, NULL);
    parse_table_add(&pt, "addr_vdl", PQ_DFL | PQ_INT,
		    (void *)-1, &addr_vdl, NULL);
    parse_table_add(&pt, "random_write", PQ_DFL | PQ_INT,
		    (void *)FALSE, &random_write, NULL);
    parse_table_add(&pt, "data_pattern", PQ_DFL | PQ_INT,
		    (void *)-1, &uDataWR[0], NULL);
    parse_table_add(&pt, "data0", PQ_DFL | PQ_INT,
		    (void *)-1, &uDataWR[0], NULL);
    parse_table_add(&pt, "data1", PQ_DFL | PQ_INT,
		    (void *)-1, &uDataWR[1], NULL);
    parse_table_add(&pt, "data2", PQ_DFL | PQ_INT,
		    (void *)-1, &uDataWR[2], NULL);
    parse_table_add(&pt, "data3", PQ_DFL | PQ_INT,
		    (void *)-1, &uDataWR[3], NULL);
    parse_table_add(&pt, "data4", PQ_DFL | PQ_INT,
		    (void *)-1, &uDataWR[4], NULL);
    parse_table_add(&pt, "data5", PQ_DFL | PQ_INT,
		    (void *)-1, &uDataWR[5], NULL);
    parse_table_add(&pt, "data6", PQ_DFL | PQ_INT,
		    (void *)-1, &uDataWR[6], NULL);
    parse_table_add(&pt, "data7", PQ_DFL | PQ_INT,
		    (void *)-1, &uDataWR[7], NULL);
    parse_table_add(&pt, "verify_mode", PQ_DFL | PQ_INT,
		    (void *)1, &verify_mode, NULL);
    if (!parseEndOk(a, &pt, &ret_code)) {
      return ret_code;
    }
  }

  /* auto increment each byte if the data pattern is not specified */
  if (uDataWR[1] == -1) {
    uDataWR[1] = uDataWR[0] + 0x10101010;
  }					
  if (uDataWR[2] == -1) {		
    uDataWR[2] = uDataWR[0] + 0x20202020;
  }					
  if (uDataWR[3] == -1) {		
    uDataWR[3] = uDataWR[0] + 0x30303030;
  }					
  if (uDataWR[4] == -1) {		
    uDataWR[4] = uDataWR[0] + 0x40404040;
  }					
  if (uDataWR[5] == -1) {		
    uDataWR[5] = uDataWR[0] + 0x50505050;
  }					
  if (uDataWR[6] == -1) {		
    uDataWR[6] = uDataWR[0] + 0x60606060;
  }					
  if (uDataWR[7] == -1) {		
    uDataWR[7] = uDataWR[0] + 0x70707070;
  }

  /* init results */
  for (i=0; i<64; i++) {
    for(j=0; j<64; j++) {
      passed[i][j] = FALSE;
    }
  }

  SOC_PBMP_ITER(ci_pbm,ci) {
    /* set the refresh override */
    READ_CI_DEBUGr(unit,ci,&uRegValue);
    soc_reg_field_set(unit,CI_DEBUGr,&uRegValue,REFRESH_OVERRIDEf,1);
    WRITE_CI_DEBUGr(unit,ci,uRegValue);

    if (verify_mode) {
      wr_status = soc_sbx_sirius_ddr23_write(unit, ci, predefined_addr, uDataWR[0],
					     uDataWR[1], uDataWR[2], uDataWR[3],
					     uDataWR[4], uDataWR[5], uDataWR[6], uDataWR[7]);
    }
					   
    switch (tuning_mode) {
    case 0:
      /* Predefined pattern read only, tuning read VDL
      SOC_IF_ERROR_RETURN(READ_CI_DDR_MR3r(unit,ci,&uRegValue));
      uRegValue |= (0x1 << 2);
      SOC_IF_ERROR_RETURN(WRITE_CI_DDR_MR3r(unit,ci,uRegValue));	    
      */

      sal_usleep(200);

      if ((read_vdl >= 0) && (read_vdl <= 63)) {
	vdl_start=read_vdl;
	vdl_end=read_vdl;
      } else {
	vdl_start = 0;
	vdl_end = 63;
      }

      for (read_vdl_tmp = vdl_start; read_vdl_tmp <= vdl_end; read_vdl_tmp++) {
	/* set bytelane 0/1, read DQSN/DQSP to read vdl, fine is always 0 */
	SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 1, 0, read_vdl_tmp, 0));
	SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 1, 1, read_vdl_tmp, 0)); 
	SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 2, 0, read_vdl_tmp, 0));
	SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 2, 1, read_vdl_tmp, 0)); 
	
	sal_usleep(200);

	if (verify_mode) {
	    SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_read(unit, ci, predefined_addr,
							  &uDataRD[0], &uDataRD[1], &uDataRD[2], &uDataRD[3],
							  &uDataRD[4], &uDataRD[5], &uDataRD[6], &uDataRD[7]));
	    
	    fifo_status = sbx_sirius_ddr_check_read_fifo_status(unit, ci);
	    
	    if ((uDataRD[0] != uDataWR[0]) ||
		(uDataRD[1] != uDataWR[1]) ||
		(uDataRD[2] != uDataWR[2]) ||
		(uDataRD[3] != uDataWR[3]) ||
		(uDataRD[4] != uDataWR[4]) ||
		(uDataRD[5] != uDataWR[5]) ||
		(uDataRD[6] != uDataWR[6]) ||
		(uDataRD[7] != uDataWR[7]) ||
		(fifo_status != SOC_E_NONE)) {
		soc_cm_debug(DK_ERR | DK_VERBOSE, "Read VDL %d Wrong read pattern 0x%8x%8x%8x%8x%8x%8x%8x%8x\n",
			     read_vdl_tmp, uDataRD[0], uDataRD[1], uDataRD[2], uDataRD[3], uDataRD[4], uDataRD[5], uDataRD[6], uDataRD[7]);
	    } else {
		soc_cm_debug(DK_PHY | DK_VERBOSE, "Read VDL %d passed\n",
			     read_vdl_tmp);
		passed[0][read_vdl_tmp] = TRUE;
	    }

	    sbx_sirius_ddr_reset_read_fifo(unit, ci);
	} else {
	    /* run DDR3 functional test */
	    rv = soc_sirius_ci_ddr_verify(unit, ci);
	    if (rv == SOC_E_NONE) {
		passed[0][read_vdl_tmp] = TRUE;
	    } else {
		passed[0][read_vdl_tmp] = FALSE;
	    }
	}
      }

      /* print smooth result */
      soc_cm_debug(DK_PHY, "Read VDL ");
      for (read_vdl_tmp = vdl_start; read_vdl_tmp <= vdl_end; read_vdl_tmp++) {
	soc_cm_debug(DK_PHY, " %2d", read_vdl_tmp);
      }
      soc_cm_debug(DK_PHY, "\nResult   ");
      for (read_vdl_tmp = vdl_start; (read_vdl_tmp <= vdl_end) && (read_vdl_tmp < 64); read_vdl_tmp++) {
	if (passed[0][read_vdl_tmp]) {
	  soc_cm_debug(DK_PHY, " * ");
	} else {
	  soc_cm_debug(DK_PHY, " - ");
	}
      }
      soc_cm_debug(DK_PHY, "\n\n");

      break;
    case 1:
      /* Predefined pattern read only, tuning read_en VDL
      SOC_IF_ERROR_RETURN(READ_CI_DDR_MR3r(unit,ci,&uRegValue));
      uRegValue |= (0x1 << 2);
      SOC_IF_ERROR_RETURN(WRITE_CI_DDR_MR3r(unit,ci,uRegValue));	    
      */

      sal_usleep(200);

      if ((read_en_vdl >= 0) && (read_en_vdl <= 63)) {
	vdl_start=read_en_vdl;
	vdl_end=read_en_vdl;
      } else {
	vdl_start = 0;
	vdl_end = 63;
      }

      for (read_en_vdl_tmp = vdl_start; read_en_vdl_tmp <= vdl_end; read_en_vdl_tmp++) {
	/* set bytelane 0/1, read_en vdl, fine is always 0 */
	SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 1, 2, read_en_vdl_tmp, 0));
	SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 2, 2, read_en_vdl_tmp, 0)); 
	
	sal_usleep(200);

	if (verify_mode) {
	    SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_read(unit, ci, predefined_addr,
							  &uDataRD[0], &uDataRD[1], &uDataRD[2], &uDataRD[3],
							  &uDataRD[4], &uDataRD[5], &uDataRD[6], &uDataRD[7]));
	    
	    fifo_status = sbx_sirius_ddr_check_read_fifo_status(unit, ci);
	    
	    if ((uDataRD[0] != uDataWR[0]) ||
		(uDataRD[1] != uDataWR[1]) ||
		(uDataRD[2] != uDataWR[2]) ||
		(uDataRD[3] != uDataWR[3]) ||
		(uDataRD[4] != uDataWR[4]) ||
		(uDataRD[5] != uDataWR[5]) ||
		(uDataRD[6] != uDataWR[6]) ||
		(uDataRD[7] != uDataWR[7]) ||
		(fifo_status != SOC_E_NONE)) {
		soc_cm_debug(DK_ERR | DK_VERBOSE, "Read_en VDL %d Wrong read pattern 0x%8x%8x%8x%8x%8x%8x%8x%8x\n",
			     read_en_vdl_tmp, uDataRD[0], uDataRD[1], uDataRD[2], uDataRD[3], uDataRD[4], uDataRD[5], uDataRD[6], uDataRD[7]);
	    } else {
		soc_cm_debug(DK_PHY | DK_VERBOSE, "Read_en VDL %d passed\n",
			     read_en_vdl_tmp);
		passed[0][read_en_vdl_tmp] = TRUE;
	    }
	    sbx_sirius_ddr_reset_read_fifo(unit, ci);
	} else {
	    /* run DDR3 functional test */
	    rv = soc_sirius_ci_ddr_verify(unit, ci);
	    if (rv == SOC_E_NONE) {
		passed[0][read_en_vdl_tmp] = TRUE;
	    } else {
		/* failed functional tests */
		passed[0][read_en_vdl_tmp] = FALSE;
	    }
	}
      }

      /* print smooth result */
      soc_cm_debug(DK_PHY, "Read En VDL ");
      for (read_en_vdl_tmp = vdl_start; read_en_vdl_tmp <= vdl_end; read_en_vdl_tmp++) {
	soc_cm_debug(DK_PHY, " %2d", read_en_vdl_tmp);
      }
      soc_cm_debug(DK_PHY, "\nResult      ");
      for (read_en_vdl_tmp = vdl_start; read_en_vdl_tmp <= vdl_end; read_en_vdl_tmp++) {
	if ( (read_en_vdl_tmp < 64) && (read_en_vdl_tmp >= 0) ) {
	  if (passed[0][read_en_vdl_tmp]) {
	    soc_cm_debug(DK_PHY, " * ");
	  } else {
	    soc_cm_debug(DK_PHY, " - ");
	  }
	}
      }
      soc_cm_debug(DK_PHY, "\n\n");

      break;
    case 2:
      /* read only, tuning read VDL and read_en VDL
      SOC_IF_ERROR_RETURN(READ_CI_DDR_MR3r(unit,ci,&uRegValue));
      uRegValue |= (0x1 << 2);
      SOC_IF_ERROR_RETURN(WRITE_CI_DDR_MR3r(unit,ci,uRegValue));	    
      */

      sal_usleep(200);

      if ((read_vdl >= 0) && (read_vdl <= 63)) {
	vdl_start=read_vdl;
	vdl_end=read_vdl;
      } else {
	vdl_start = 0;
	vdl_end = 63;
      }

      if ((read_en_vdl >= 0) && (read_en_vdl <= 63)) {
	vdl_start2=read_en_vdl;
	vdl_end2=read_en_vdl;
      } else {
	vdl_start2 = 0;
	vdl_end2 = 63;
      }

      for (read_vdl_tmp = vdl_start; read_vdl_tmp <= vdl_end; read_vdl_tmp++) {
	/* set bytelane 0/1, read DQSN/DQSP to read vdl, fine is always 0 */
	SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 1, 0, read_vdl_tmp, 0));
	SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 1, 1, read_vdl_tmp, 0)); 
	SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 2, 0, read_vdl_tmp, 0));
	SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 2, 1, read_vdl_tmp, 0)); 
	
	sal_usleep(200);

	for (read_en_vdl_tmp = vdl_start2; read_en_vdl_tmp <= vdl_end2; read_en_vdl_tmp++) {
	  /* set bytelane 0/1, read_en vdl, fine is always 0 */
	  SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 1, 2, read_en_vdl_tmp, 0));
	  SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 2, 2, read_en_vdl_tmp, 0)); 
	
	  sal_usleep(200);

	  if (verify_mode) {
	      SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_read(unit, ci, predefined_addr,
							    &uDataRD[0], &uDataRD[1], &uDataRD[2], &uDataRD[3],
							    &uDataRD[4], &uDataRD[5], &uDataRD[6], &uDataRD[7]));
	      
	      fifo_status = sbx_sirius_ddr_check_read_fifo_status(unit, ci);
	      
	      if ((uDataRD[0] != uDataWR[0]) ||
		  (uDataRD[1] != uDataWR[1]) ||
		  (uDataRD[2] != uDataWR[2]) ||
		  (uDataRD[3] != uDataWR[3]) ||
		  (uDataRD[4] != uDataWR[4]) ||
		  (uDataRD[5] != uDataWR[5]) ||
		  (uDataRD[6] != uDataWR[6]) ||
		  (uDataRD[7] != uDataWR[7]) ||
		  (fifo_status != SOC_E_NONE)) {
		  soc_cm_debug(DK_ERR | DK_VERBOSE, "Read VDL/Read_en VDL %d/%d Wrong read pattern 0x%8x%8x%8x%8x%8x%8x%8x%8x\n",
			       read_vdl_tmp, read_en_vdl_tmp, uDataRD[0], uDataRD[1], uDataRD[2], uDataRD[3], uDataRD[4], uDataRD[5], uDataRD[6], uDataRD[7]);
	      } else {
		  soc_cm_debug(DK_PHY | DK_VERBOSE, "Read VDL/Read_en VDL %d/%d passed\n",
			       read_vdl_tmp, read_en_vdl_tmp);
		  passed[read_vdl_tmp][read_en_vdl_tmp] = TRUE;
	      }

	      sbx_sirius_ddr_reset_read_fifo(unit, ci);
	  } else {
	      /* run DDR3 functional test */
	      rv = soc_sirius_ci_ddr_verify(unit, ci);
	      if (rv == SOC_E_NONE) {
		  passed[read_vdl_tmp][read_en_vdl_tmp] = TRUE;
	      } else {
		  /* failed functional tests */
		  passed[read_vdl_tmp][read_en_vdl_tmp] = FALSE;
	      }
	  }
	}
      }

      /* print smooth result */
      soc_cm_debug(DK_PHY, "Read En VDL");
      for (read_en_vdl_tmp = vdl_start2; read_en_vdl_tmp <= vdl_end2; read_en_vdl_tmp++) {
	soc_cm_debug(DK_PHY, " %2d", read_en_vdl_tmp);
      }

      for (read_vdl_tmp = vdl_start; (read_vdl_tmp <= vdl_end) && (read_vdl_tmp < 64); read_vdl_tmp++) {
	soc_cm_debug(DK_PHY, "\nRead VDL %2d", read_vdl_tmp);
	
	for (read_en_vdl_tmp = vdl_start2; (read_en_vdl_tmp <= vdl_end2) && (read_en_vdl_tmp < 64); read_en_vdl_tmp++) {
	  if (passed[read_vdl_tmp][read_en_vdl_tmp]) {
	    soc_cm_debug(DK_PHY, " * ");
	  } else {
	    soc_cm_debug(DK_PHY, " - ");
	  }
	}
      }
      soc_cm_debug(DK_PHY, "\n\n");

      break;
    case 3:
      /* read only, tuning addr/ctrl VDL 
      SOC_IF_ERROR_RETURN(READ_CI_DDR_MR3r(unit,ci,&uRegValue));
      uRegValue |= (0x1 << 2);
      SOC_IF_ERROR_RETURN(WRITE_CI_DDR_MR3r(unit,ci,uRegValue));	    
      */

      sal_usleep(200);

      if ((addr_vdl >= 0) && (addr_vdl <= 63)) {
	vdl_start=addr_vdl;
	vdl_end=addr_vdl;
      } else {
	vdl_start = 0;
	vdl_end = 63;
      }

      for (addr_vdl_tmp = vdl_start; addr_vdl_tmp <= vdl_end; addr_vdl_tmp++) {
	/* set addr vdl, fine is always 0 */
	SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 0, 0, addr_vdl_tmp, 0));
	
	sal_usleep(200);

	SOC_IF_ERROR_RETURN(READ_CI_DDR_AUTOINITr(unit,ci,&uRegValue));
	soc_reg_field_set(unit,CI_DDR_AUTOINITr,&uRegValue,STARTf,1);
	soc_reg_field_set(unit,CI_DDR_AUTOINITr,&uRegValue,DONEf,0);
	SOC_IF_ERROR_RETURN(WRITE_CI_DDR_AUTOINITr(unit,ci,uRegValue));

	if (!SAL_BOOT_BCMSIM) {
	  /* wait for done */
	  if (sirius_reg_done_timeout(unit,
				      CI_DDR_AUTOINITr,
				      ci,
				      0,
				      DONEf,
				      _sirius_init_timeout) ) {
            SOC_ERROR_PRINT((DK_WARN,"Waiting for config done for CI%d timedout\n",ci));
            return SOC_E_TIMEOUT;
	  }
	}

	if (verify_mode) {
	    SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_read(unit, ci, predefined_addr,
							  &uDataRD[0], &uDataRD[1], &uDataRD[2], &uDataRD[3],
							  &uDataRD[4], &uDataRD[5], &uDataRD[6], &uDataRD[7]));
	    
	    fifo_status = sbx_sirius_ddr_check_read_fifo_status(unit, ci);
	    
	    if ((uDataRD[0] != uDataWR[0]) ||
		(uDataRD[1] != uDataWR[1]) ||
		(uDataRD[2] != uDataWR[2]) ||
		(uDataRD[3] != uDataWR[3]) ||
		(uDataRD[4] != uDataWR[4]) ||
		(uDataRD[5] != uDataWR[5]) ||
		(uDataRD[6] != uDataWR[6]) ||
		(uDataRD[7] != uDataWR[7]) ||
		(fifo_status != SOC_E_NONE)) {
		soc_cm_debug(DK_ERR | DK_VERBOSE, "addr VDL %d Wrong read pattern 0x%8x%8x%8x%8x%8x%8x%8x%8x\n",
			     addr_vdl_tmp, uDataRD[0], uDataRD[1], uDataRD[2], uDataRD[3], uDataRD[4], uDataRD[5], uDataRD[6], uDataRD[7]);
	    } else {
		soc_cm_debug(DK_PHY | DK_VERBOSE, "addr VDL %d passed\n",
			     addr_vdl_tmp);
		passed[0][addr_vdl_tmp] = FALSE;
	    }
	    sbx_sirius_ddr_reset_read_fifo(unit, ci);
	} else {
	    /* run DDR3 functional test */
	    rv = soc_sirius_ci_ddr_verify(unit, ci);
	    if (rv == SOC_E_NONE) {
		passed[0][addr_vdl_tmp] = TRUE;
	    } else {
		/* failed functional tests */
		passed[0][addr_vdl_tmp] = FALSE;
	    }
	}
      }

      /* print smooth result */
      soc_cm_debug(DK_PHY, "Addr VDL ");
      for (addr_vdl_tmp = vdl_start; addr_vdl_tmp <= vdl_end; addr_vdl_tmp++) {
	soc_cm_debug(DK_PHY, " %2d", addr_vdl_tmp);
      }
      soc_cm_debug(DK_PHY, "\nResult      ");
      for (addr_vdl_tmp = vdl_start; (addr_vdl_tmp <= vdl_end) && (addr_vdl_tmp < 64); addr_vdl_tmp++) {
	if (passed[0][addr_vdl_tmp]) {
	  soc_cm_debug(DK_PHY, " * ");
	} else {
	  soc_cm_debug(DK_PHY, " - ");
	}
      }
      soc_cm_debug(DK_PHY, "\n\n");

      break;
    case 4:
      /* read only, tuning addr/ctrl VDL and read VDL
      SOC_IF_ERROR_RETURN(READ_CI_DDR_MR3r(unit,ci,&uRegValue));
      uRegValue |= (0x1 << 2);
      SOC_IF_ERROR_RETURN(WRITE_CI_DDR_MR3r(unit,ci,uRegValue));	    
      */

      sal_usleep(200);

      if ((addr_vdl >= 0) && (addr_vdl <= 63)) {
	vdl_start=addr_vdl;
	vdl_end=addr_vdl;
      } else {
	vdl_start = 0;
	vdl_end = 63;
      }

      if ((read_vdl >= 0) && (read_vdl <= 63)) {
	vdl_start2=read_vdl;
	vdl_end2=read_vdl;
      } else {
	vdl_start2 = 0;
	vdl_end2 = 63;
      }

      for (addr_vdl_tmp = vdl_start; addr_vdl_tmp <= vdl_end; addr_vdl_tmp++) {
	/* set addr vdl, fine is always 0 */
	SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 0, 0, addr_vdl_tmp, 0));
	
	sal_usleep(200);

	SOC_IF_ERROR_RETURN(READ_CI_DDR_AUTOINITr(unit,ci,&uRegValue));
	soc_reg_field_set(unit,CI_DDR_AUTOINITr,&uRegValue,STARTf,1);
	soc_reg_field_set(unit,CI_DDR_AUTOINITr,&uRegValue,DONEf,0);
	SOC_IF_ERROR_RETURN(WRITE_CI_DDR_AUTOINITr(unit,ci,uRegValue));

	if (!SAL_BOOT_BCMSIM) {
	  /* wait for done */
	  if (sirius_reg_done_timeout(unit,
				      CI_DDR_AUTOINITr,
				      ci,
				      0,
				      DONEf,
				      _sirius_init_timeout) ) {
            SOC_ERROR_PRINT((DK_WARN,"Waiting for config done for CI%d timedout\n",ci));
            return SOC_E_TIMEOUT;
	  }
	}

	for (read_vdl_tmp = vdl_start2; read_vdl_tmp <= vdl_end2; read_vdl_tmp++) {
	  /* set bytelane 0/1, read DQSN/DQSP to read vdl, fine is always 0 */
	  SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 1, 0, read_vdl_tmp, 0));
	  SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 1, 1, read_vdl_tmp, 0)); 
	  SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 2, 0, read_vdl_tmp, 0));
	  SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 2, 1, read_vdl_tmp, 0)); 
	
	  sal_usleep(200);

	  if (verify_mode) {
	      SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_read(unit, ci, predefined_addr,
							    &uDataRD[0], &uDataRD[1], &uDataRD[2], &uDataRD[3],
							    &uDataRD[4], &uDataRD[5], &uDataRD[6], &uDataRD[7]));
	      
	      fifo_status = sbx_sirius_ddr_check_read_fifo_status(unit, ci);
	      
	      if ((uDataRD[0] != uDataWR[0]) ||
		  (uDataRD[1] != uDataWR[1]) ||
		  (uDataRD[2] != uDataWR[2]) ||
		  (uDataRD[3] != uDataWR[3]) ||
		  (uDataRD[4] != uDataWR[4]) ||
		  (uDataRD[5] != uDataWR[5]) ||
		  (uDataRD[6] != uDataWR[6]) ||
		  (uDataRD[7] != uDataWR[7]) ||
		  (fifo_status != SOC_E_NONE)) {
		  soc_cm_debug(DK_ERR | DK_VERBOSE, "addr VDL/read VDL %d/%d Wrong read pattern 0x%8x%8x%8x%8x%8x%8x%8x%8x\n",
			       addr_vdl_tmp, read_vdl_tmp, uDataRD[0], uDataRD[1], uDataRD[2], uDataRD[3], uDataRD[4], uDataRD[5], uDataRD[6], uDataRD[7]);
	      } else {
		  soc_cm_debug(DK_PHY | DK_VERBOSE, "addr VDL %d passed\n",
			       addr_vdl_tmp);
		  passed[addr_vdl_tmp][read_vdl_tmp] = TRUE;
	      }
	      sbx_sirius_ddr_reset_read_fifo(unit, ci);
	  } else {
	      /* run DDR3 functional test */
	      rv = soc_sirius_ci_ddr_verify(unit, ci);
	      if (rv == SOC_E_NONE) {
		  passed[addr_vdl_tmp][read_vdl_tmp] = TRUE;
	      } else {
		  /* failed functional tests */
		  passed[addr_vdl_tmp][read_vdl_tmp] = FALSE;
	      }
	  }
	}
      }

      /* print smooth result */
      soc_cm_debug(DK_PHY, "Read VDL   ");
      for (read_vdl_tmp = vdl_start2; read_vdl_tmp <= vdl_end2; read_vdl_tmp++) {
	soc_cm_debug(DK_PHY, " %2d", read_vdl_tmp);
      }

      for (addr_vdl_tmp = vdl_start; (addr_vdl_tmp <= vdl_end) && (addr_vdl_tmp < 64); addr_vdl_tmp++) {
	soc_cm_debug(DK_PHY, "\nAddr VDL %2d", addr_vdl_tmp);
	
	for (read_vdl_tmp = vdl_start2; (read_vdl_tmp <= vdl_end2) && (read_vdl_tmp < 64); read_vdl_tmp++) {
	  if (passed[addr_vdl_tmp][read_vdl_tmp]) {
	    soc_cm_debug(DK_PHY, " * ");
	  } else {
	    soc_cm_debug(DK_PHY, " - ");
	  }
	}
      }
      soc_cm_debug(DK_PHY, "\n\n");
      break;
    case 5:
      /* read only, tuning addr/ctrl VDL, read VDL and read-en VDL
      SOC_IF_ERROR_RETURN(READ_CI_DDR_MR3r(unit,ci,&uRegValue));
      uRegValue |= (0x1 << 2);
      SOC_IF_ERROR_RETURN(WRITE_CI_DDR_MR3r(unit,ci,uRegValue));
      */

      sal_usleep(200);

      if ((addr_vdl >= 0) && (addr_vdl <= 63)) {
	vdl_start=addr_vdl;
	vdl_end=addr_vdl;
      } else {
	vdl_start = 0;
	vdl_end = 63;
      }

      if ((read_vdl >= 0) && (read_vdl <= 63)) {
	vdl_start2=read_vdl;
	vdl_end2=read_vdl;
      } else {
	vdl_start2 = 0;
	vdl_end2 = 63;
      }

      if ((read_en_vdl >= 0) && (read_en_vdl <= 63)) {
	vdl_start3=read_en_vdl;
	vdl_end3=read_en_vdl;
      } else {
	vdl_start3 = 0;
	vdl_end3 = 63;
      }

      for (addr_vdl_tmp = vdl_start; addr_vdl_tmp <= vdl_end; addr_vdl_tmp++) {
	/* set addr vdl, fine is always 0 */
	SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 0, 0, addr_vdl_tmp, 0));
	
	sal_usleep(200);

	SOC_IF_ERROR_RETURN(READ_CI_DDR_AUTOINITr(unit,ci,&uRegValue));
	soc_reg_field_set(unit,CI_DDR_AUTOINITr,&uRegValue,STARTf,1);
	soc_reg_field_set(unit,CI_DDR_AUTOINITr,&uRegValue,DONEf,0);
	SOC_IF_ERROR_RETURN(WRITE_CI_DDR_AUTOINITr(unit,ci,uRegValue));

	if (!SAL_BOOT_BCMSIM) {
	  /* wait for done */
	  if (sirius_reg_done_timeout(unit,
				      CI_DDR_AUTOINITr,
				      ci,
				      0,
				      DONEf,
				      _sirius_init_timeout) ) {
            SOC_ERROR_PRINT((DK_WARN,"Waiting for config done for CI%d timedout\n",ci));
            return SOC_E_TIMEOUT;
	  }
	}

	for (read_vdl_tmp = vdl_start2; read_vdl_tmp <= vdl_end2; read_vdl_tmp++) {
	  /* set bytelane 0/1, read DQSN/DQSP to read vdl, fine is always 0 */
	  SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 1, 0, read_vdl_tmp, 0));
	  SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 1, 1, read_vdl_tmp, 0)); 
	  SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 2, 0, read_vdl_tmp, 0));
	  SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 2, 1, read_vdl_tmp, 0)); 
	
	  sal_usleep(200);

	  SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_read(unit, ci, predefined_addr,
							&uDataRD[0], &uDataRD[1], &uDataRD[2], &uDataRD[3],
							&uDataRD[4], &uDataRD[5], &uDataRD[6], &uDataRD[7]));
	  
	  for (read_en_vdl_tmp = vdl_start3; read_en_vdl_tmp <= vdl_end3; read_en_vdl_tmp++) {
	    /* set bytelane 0/1, read_en vdl, fine is always 0 */
	    SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 1, 2, read_en_vdl_tmp, 0));
	    SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 2, 2, read_en_vdl_tmp, 0)); 
	    
	    sal_usleep(200);
	  
	    if (verify_mode) {
		fifo_status = sbx_sirius_ddr_check_read_fifo_status(unit, ci);
		
		if ((uDataRD[0] != uDataWR[0]) ||
		    (uDataRD[1] != uDataWR[1]) ||
		    (uDataRD[2] != uDataWR[2]) ||
		    (uDataRD[3] != uDataWR[3]) ||
		    (uDataRD[4] != uDataWR[4]) ||
		    (uDataRD[5] != uDataWR[5]) ||
		    (uDataRD[6] != uDataWR[6]) ||
		    (uDataRD[7] != uDataWR[7]) ||
		    (fifo_status != SOC_E_NONE)) {
		    soc_cm_debug(DK_ERR | DK_VERBOSE, "addr VDL/read VDL/read_en VDL %d/%d/%d Wrong read pattern 0x%8x%8x%8x%8x%8x%8x%8x%8x\n",
				 addr_vdl_tmp, read_vdl_tmp, read_en_vdl_tmp, uDataRD[0], uDataRD[1], uDataRD[2], uDataRD[3],
				 uDataRD[4], uDataRD[5], uDataRD[6], uDataRD[7]);
		} else {
		    soc_cm_debug(DK_PHY | DK_VERBOSE, "addr VDL/read VDL/read_en VDL %d/%d/%d passed\n",
				 addr_vdl_tmp, read_vdl_tmp, read_en_vdl_tmp);
		    passed[read_vdl_tmp][read_en_vdl_tmp] = TRUE;
		}
		sbx_sirius_ddr_reset_read_fifo(unit, ci);
	    } else {
		/* run DDR3 functional test */
		rv = soc_sirius_ci_ddr_verify(unit, ci);
		if (rv == SOC_E_NONE) {
		    passed[read_vdl_tmp][read_en_vdl_tmp] = TRUE;
		} else {
		    /* failed functional tests */
		    passed[read_vdl_tmp][read_en_vdl_tmp] = FALSE;
		}
	    }
	  }
	}

	/* print smooth result */
	soc_cm_debug(DK_PHY, "Addr VDL %2d ########################################\n\n", addr_vdl_tmp);

	soc_cm_debug(DK_PHY, "Read En VDL");
	for (read_en_vdl_tmp = vdl_start3; read_en_vdl_tmp <= vdl_end3; read_en_vdl_tmp++) {
	  soc_cm_debug(DK_PHY, " %2d", read_en_vdl_tmp);
	}
	
	for (read_vdl_tmp = vdl_start2; (read_vdl_tmp <= vdl_end2) && (read_vdl_tmp < 64); read_vdl_tmp++) {
	  soc_cm_debug(DK_PHY, "\nRead VDL %2d", read_vdl_tmp);
	  
	  for (read_en_vdl_tmp = vdl_start3; (read_en_vdl_tmp <= vdl_end3) && (read_en_vdl_tmp < 64); read_en_vdl_tmp++) {
	    if (passed[read_vdl_tmp][read_en_vdl_tmp]) {
	      soc_cm_debug(DK_PHY, " * ");
	    } else {
	      soc_cm_debug(DK_PHY, " - ");
	    }
	  }
	}
	soc_cm_debug(DK_PHY, "\n\n");

      }
      break;
    default:
      /* read/write, tuning write VDL */
      if ((write_vdl >= 0) && (write_vdl <= 63)) {
	vdl_start=write_vdl;
	vdl_end=write_vdl;
      } else {
	vdl_start = 0;
	vdl_end = 63;
      }

      sal_srand(rseed);
      
      /*
       *  Use random data
       */
      if (random_write) {
	for (i = 0; i < 8; i++ ) {
	  uDataWR[i] = sal_rand() & 0xffffffff;
	}
      }

      for (write_vdl_tmp = vdl_start; write_vdl_tmp <= vdl_end; write_vdl_tmp++) {
	/* set bytelane 0/1, write vdl, fine is always 0 */
	SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 1, 3, write_vdl_tmp, 0));
	SOC_IF_ERROR_RETURN(soc_sbx_sirius_ddr23_vdl_set(unit, ci, 2, 3, write_vdl_tmp, 0));
	
	sal_usleep(200);
	
	passed[0][write_vdl_tmp] = TRUE;

	for (bank = 0; bank < 8; bank++ ) {
	  
	  uAddr = bank; 

	  if (verify_mode) {
	      wr_status = soc_sbx_sirius_ddr23_write(unit, ci,uAddr,
						     uDataWR[0],uDataWR[1],uDataWR[2],
						     uDataWR[3],uDataWR[4],uDataWR[5],
						     uDataWR[6],uDataWR[7]);
	      
	      /*
	       *  Read back and compare
	       */  
	      
	      rd_status = soc_sbx_sirius_ddr23_read(unit,ci,uAddr,&uDataRD[0],
						    &uDataRD[1],&uDataRD[2],&uDataRD[3],
						    &uDataRD[4],&uDataRD[5],&uDataRD[6],
						    &uDataRD[7]);
	      
	      fifo_status = sbx_sirius_ddr_check_read_fifo_status(unit, ci);
	      
	      if (wr_status || 
		  rd_status ||
		  fifo_status ||
		  (uDataWR[0] != uDataRD[0]) ||
		  (uDataWR[1] != uDataRD[1]) ||
		  (uDataWR[2] != uDataRD[2]) ||
		  (uDataWR[3] != uDataRD[3]) ||
		  (uDataWR[4] != uDataRD[4]) ||
		  (uDataWR[5] != uDataRD[5]) ||
		  (uDataWR[6] != uDataRD[6]) ||
		  (uDataWR[7] != uDataRD[7])) {
		  
		  soc_cm_debug(DK_ERR | DK_VERBOSE, "write VDL %d error wrote 0x%8x%8x%8x%8x%8x%8x%8x%8x read 0x%8x%8x%8x%8x%8x%8x%8x%8x\n",
			       write_vdl_tmp, uDataWR[0], uDataWR[1], uDataWR[2], uDataWR[3], uDataWR[4], uDataWR[5], uDataWR[6], uDataWR[7],
			       uDataRD[0], uDataRD[1], uDataRD[2], uDataRD[3], uDataRD[4], uDataRD[5], uDataRD[6], uDataRD[7]);
		  passed[0][write_vdl_tmp] = FALSE;
	      } else {
		  soc_cm_debug(DK_PHY | DK_VERBOSE, "Write VDL %d passed\n",
			       write_vdl_tmp);
	      }
	      sbx_sirius_ddr_reset_read_fifo(unit, ci);
	  } else {
	      /* run DDR3 functional test */
	      rv = soc_sirius_ci_ddr_verify(unit, ci);
	      if (rv == SOC_E_NONE) {
		  passed[0][write_vdl_tmp] = TRUE;
	      } else {
		  /* failed functional tests */
		  passed[0][write_vdl_tmp] = FALSE;
	      }
	  }
	}
      }

      /* print smooth result */
      soc_cm_debug(DK_PHY, "Write VDL ");
      for (write_vdl_tmp = vdl_start; write_vdl_tmp <= vdl_end; write_vdl_tmp++) {
	soc_cm_debug(DK_PHY, " %2d", write_vdl_tmp);
      }
      soc_cm_debug(DK_PHY, "\nResult       ");
      for (write_vdl_tmp = vdl_start; (write_vdl_tmp <= vdl_end) && (write_vdl_tmp < 64); write_vdl_tmp++) {
	if (passed[0][write_vdl_tmp]) {
	  soc_cm_debug(DK_PHY, " * ");
	} else {
	  soc_cm_debug(DK_PHY, " - ");
	}
      }
      soc_cm_debug(DK_PHY, "\n\n");
      
      break;
    }
  }

  return CMD_OK;
}

char cmd_sbx_sirius_ddr_phy_tune_auto_usage[] = "\n"
" DDRPhyTuneAuto ci tread_en=<tread_en> verify_mode=<0-1>\n"
" DDRPhyTuneAuto ci tread_en=13\n"
" Will Tune for tread_en windows for <tread_en> and <tread_en+1> \n"
" optimal soc parameters are printed to screen for each ci interface\n"
" verify_mode = 1 (default) use indirect memory read/write to verify if setting works\n"
" verify_mode = 0 use ddr functional tests to verify if setting works\n"
" verify_mode = 0 is much slower than verify_mode = 1 and detect more wrong settings\n";

cmd_result_t
cmd_sbx_sirius_ddr_phy_tune_auto(int unit, args_t *a)
{
  int ci = 0;
  char *c = NULL;
  /* store results for both windows including 78 width gap in-between */
  uint8 *results[DDR23_READ_EN_VDL_WIDTH];
  soc_pbmp_t ci_pbm;
  uint32 uDataWR[8] = {0};
  uint32 uDataRD[8] = {0};
  uint32 predefined_addr = 0;
  int user_tread_en = -1;
  int tread_en;
  int i,j;
  int read_vdl;
  int read_en_vdl;
  uint32 uRegValue;
  uint32 wr_status = SOC_E_NONE;
  uint32 fifo_status = SOC_E_NONE;
  int offset;
  int offset_start, offset_end;
  int tread_en_start, tread_en_end;
  int rv, verify_mode;

  ci_tune_params_t ci_tune_params[SOC_MAX_NUM_CI_BLKS];
  ci_tune_params_t *pCiTuneParams = NULL;

  if (!SOC_IS_SBX_SIRIUS(unit)) {
    printk("only supported on sirius\n");
    return CMD_FAIL;
  }

  if (((c = ARG_GET(a)) == NULL) || (parse_bcm_pbmp(unit, c, &ci_pbm) < 0)) {
    return CMD_USAGE;
  }

  if (ARG_CNT(a)) {
    int ret_code;
    parse_table_t pt;
    parse_table_init(0, &pt);
    parse_table_add(&pt, "tread_en", PQ_DFL | PQ_INT,
		    (void *)-1, &user_tread_en, NULL);
    parse_table_add(&pt, "verify_mode", PQ_DFL | PQ_INT,
		    (void *)1, &verify_mode, NULL);

    if (!parseEndOk(a, &pt, &ret_code)) {
      return ret_code;
    }
  }

  /* require a starting tread_en */
  if (user_tread_en == -1) {
    printk(" >>  specify starting tread_en\n");
    return CMD_USAGE;
  }

  sal_memset(ci_tune_params,0,sizeof(ci_tune_params_t)*SOC_MAX_NUM_CI_BLKS);

  uDataWR[0] = 0x01020304;

  /* use incr byte pattern by default */
  /*  uDataWR[0] = 0x01020304 */
  /*  uDataWR[1] = 0x05060708 */
  /*  uDataWR[2] = 0x090a0b0c */
  for (i=1;i<8;i++) {
    uDataWR[i] = uDataWR[0] + ((i<<26) | (i<<18) | (i<<10) | (i<<2));
  }

  results[0] = sal_alloc(sizeof(uint8) * DDR23_READ_EN_VDL_WIDTH * (2*DDR23_READ_EN_VDL_WIDTH + TREAD_EN_GAP_WIDTH), "results");
  
  if (results[0] == NULL) {
    return CMD_FAIL;
  } else {
    sal_memset(results[0],0,sizeof(uint8) * DDR23_READ_EN_VDL_WIDTH *(2*DDR23_READ_EN_VDL_WIDTH + TREAD_EN_GAP_WIDTH));
  }

  for (i=0; i<DDR23_READ_EN_VDL_WIDTH; i++) {
    results[i] = results[0] + (i * (2*DDR23_READ_EN_VDL_WIDTH+TREAD_EN_GAP_WIDTH));
  }

  tread_en_start = user_tread_en;
  tread_en_end   = user_tread_en+1;
  
  SOC_PBMP_ITER(ci_pbm,ci) {

    pCiTuneParams = &ci_tune_params[ci];

    /* init results */
    for (i=0; i<DDR23_READ_EN_VDL_WIDTH; i++) {
      for(j=0; j<(2*DDR23_READ_EN_VDL_WIDTH + TREAD_EN_GAP_WIDTH); j++) {
	if (j >=DDR23_READ_EN_VDL_WIDTH && j <= (DDR23_READ_EN_VDL_WIDTH+TREAD_EN_GAP_WIDTH)) {
	  results[i][j] = 1;  /* passing result between two windows */
	} else {
	  results[i][j] = 0;
	}
      }
    }  

    for (tread_en = tread_en_start; tread_en <= tread_en_end; tread_en++) {

      /* configure specified tread_en */
      printk("Tuning CI%d using tread_en=%d\n",ci,tread_en);
      rv = READ_CI_CONFIG2r(unit,ci,&uRegValue);
      if (rv < 0) {
	sal_free(results[0]);
	return CMD_FAIL;
      }
      soc_reg_field_set(unit,CI_CONFIG2r,&uRegValue,TREAD_ENBf,tread_en);
      rv = WRITE_CI_CONFIG2r(unit,ci,uRegValue);
      if (rv < 0) {
	sal_free(results[0]);
	return CMD_FAIL;
      }

      if (tread_en == tread_en_start) {
	offset = 0;
      } else {
	/* next window results start */
	offset = DDR23_READ_EN_VDL_WIDTH + TREAD_EN_GAP_WIDTH;
      }

      /* set the refresh override */
      rv = READ_CI_DEBUGr(unit,ci,&uRegValue);
      if (rv < 0) {
	sal_free(results[0]);
	return CMD_FAIL;
      }
      soc_reg_field_set(unit,CI_DEBUGr,&uRegValue,REFRESH_OVERRIDEf,1);
      rv = WRITE_CI_DEBUGr(unit,ci,uRegValue);
      if (rv < 0) {
	sal_free(results[0]);
	return CMD_FAIL;
      }

      if (verify_mode) {
	wr_status = soc_sbx_sirius_ddr23_write(unit, ci, predefined_addr, uDataWR[0],
					       uDataWR[1], uDataWR[2], uDataWR[3],
					       uDataWR[4], uDataWR[5], uDataWR[6], uDataWR[7]);
      }

      sal_usleep(200);

      /* tune read VDL and read_en VDL */
      for(read_vdl = 0; read_vdl < DDR23_READ_EN_VDL_WIDTH ; read_vdl++) {
	/* set bytelane 0/1, read DQSN/DQSP to read vdl, fine is always 0 */
	rv = soc_sbx_sirius_ddr23_vdl_set(unit, ci, 1, 0, read_vdl, 0);
	if (rv < 0) {
	  sal_free(results[0]);
	  return CMD_FAIL;
	}
	rv = soc_sbx_sirius_ddr23_vdl_set(unit, ci, 1, 1, read_vdl, 0); 
	if (rv < 0) {
	  sal_free(results[0]);
	  return CMD_FAIL;
	}
	rv = soc_sbx_sirius_ddr23_vdl_set(unit, ci, 2, 0, read_vdl, 0);
	if (rv < 0) {
	  sal_free(results[0]);
	  return CMD_FAIL;
	}
	rv = soc_sbx_sirius_ddr23_vdl_set(unit, ci, 2, 1, read_vdl, 0); 
	if (rv < 0) {
	  sal_free(results[0]);
	  return CMD_FAIL;
	}
	
	sal_usleep(200);

	for (read_en_vdl = 0; read_en_vdl < DDR23_READ_EN_VDL_WIDTH; read_en_vdl++) {
	  /* set bytelane 0/1, read_en vdl, fine is always 0 */
	  rv = soc_sbx_sirius_ddr23_vdl_set(unit, ci, 1, 2, read_en_vdl, 0);
	  if (rv < 0) {
	    sal_free(results[0]);
	    return CMD_FAIL;
	  }

	  rv = soc_sbx_sirius_ddr23_vdl_set(unit, ci, 2, 2, read_en_vdl, 0); 
	  if (rv < 0) {
	    sal_free(results[0]);
	    return CMD_FAIL;
	  }
	
	  sal_usleep(200);

	  if (verify_mode) {
	      rv = soc_sbx_sirius_ddr23_read(unit, ci, predefined_addr,
					     &uDataRD[0], &uDataRD[1], &uDataRD[2], &uDataRD[3],
					     &uDataRD[4], &uDataRD[5], &uDataRD[6], &uDataRD[7]);
	      
	      fifo_status = sbx_sirius_ddr_check_read_fifo_status(unit, ci);
	      
	      if ((uDataRD[0] != uDataWR[0]) ||
		  (uDataRD[1] != uDataWR[1]) ||
		  (uDataRD[2] != uDataWR[2]) ||
		  (uDataRD[3] != uDataWR[3]) ||
		  (uDataRD[4] != uDataWR[4]) ||
		  (uDataRD[5] != uDataWR[5]) ||
		  (uDataRD[6] != uDataWR[6]) ||
		  (uDataRD[7] != uDataWR[7]) ||
		  (fifo_status != SOC_E_NONE)) {
		  /* soc_cm_debug(DK_ERR | DK_VERBOSE, "Read VDL/Read_en VDL %d/%d Wrong read pattern 0x%8x%8x%8x%8x%8x%8x%8x%8x\n", */
		  /* 			 read_vdl, read_en_vdl, uDataRD[0], uDataRD[1], uDataRD[2], uDataRD[3], uDataRD[4], uDataRD[5], uDataRD[6], uDataRD[7]); */
		  results[read_vdl][read_en_vdl + offset] = FALSE;
	      } else {
		  /* 	    soc_cm_debug(DK_PHY | DK_VERBOSE, "Read VDL/Read_en VDL %d/%d passed\n", */
		  /* 			 read_vdl, read_en_vdl); */
		  results[read_vdl][read_en_vdl + offset] = TRUE;
	      }
	      sbx_sirius_ddr_reset_read_fifo(unit, ci);
	  } else {
	      rv = soc_sirius_ci_ddr_verify(unit, ci);
	      if (rv == SOC_E_NONE) {
		  results[read_vdl][read_en_vdl + offset] = TRUE;
	      } else {
		  /* failed functional tests */
		  results[read_vdl][read_en_vdl + offset] = FALSE;
	      }
	  }
	}
      }

      /* print results (like in tune) */
      if (tread_en == tread_en_start) {
	offset_start = 0;
	offset_end = DDR23_READ_EN_VDL_WIDTH-1;
      } else {
	offset_start = DDR23_READ_EN_VDL_WIDTH + TREAD_EN_GAP_WIDTH; 
	offset_end   = 2*DDR23_READ_EN_VDL_WIDTH + TREAD_EN_GAP_WIDTH;
      }

      printk("**** TREAD_EN=%d *****\n",tread_en);
      printk( "Read En VDL");
      for (read_en_vdl = 0; read_en_vdl < DDR23_READ_EN_VDL_WIDTH; read_en_vdl++) {
	printk(" %2d",read_en_vdl);
      }

      for (read_vdl = 0; read_vdl < DDR23_READ_EN_VDL_WIDTH ; read_vdl++) {
	printk("\nRead VDL  %2d", read_vdl);
	
	for (read_en_vdl = offset_start; read_en_vdl <= offset_end; read_en_vdl++) {
	  if (results[read_vdl][read_en_vdl]) {
	    printk( " * ");
	  } else {
	    printk( " - ");
	  }
	}
      }
      printk("\n");
    } /* each tread_en */

    /* find optimal results using both tread_en windows */
    soc_sbx_sirius_ddr23_phy_tune_best_setting(tread_en_start,pCiTuneParams,&results[0][0]);

  } /* pbm ci */

  /* dump soc properties to use */
  SOC_PBMP_ITER(ci_pbm,ci) {
    if ((ci < SOC_MAX_NUM_CI_BLKS) && (ci_tune_params[ci].valid == 1)) {
      printk("%s_%d=%d\n",spn_SIRIUS_DDR3_READ_VDL,ci,ci_tune_params[ci].read_vdl);
      printk("%s_%d=%d\n",spn_SIRIUS_DDR3_READ_EN_VDL,ci,ci_tune_params[ci].read_en_vdl);
      printk("%s_%d=%d\n",spn_SIRIUS_DDR3_TREAD_ENB,ci,ci_tune_params[ci].tread_en);
    }
  }

  sal_free(results[0]);
  return CMD_OK;
}

static int
sbx_sirius_ddr_check_read_fifo_status(unit, ci)
{
  int rv = SOC_E_NONE;
  uint uRegValue;
  uint status;

  SOC_IF_ERROR_RETURN(READ_DDR23_PHY_BYTE_LANE0_READ_FIFO_STATUSr(unit,ci,&uRegValue));
  status = DDR23_GET_FIELD(uRegValue, DDR23_PHY_BYTE_LANE0, READ_FIFO_STATUS, STATUS);
  if (status != 0) {
    /* soc_cm_debug(DK_ERR | DK_VERBOSE, "ci %d read fifo error on bytelane 0, status 0x%x\n", ci, status); */
    rv = SOC_E_INTERNAL;
  }

  SOC_IF_ERROR_RETURN(READ_DDR23_PHY_BYTE_LANE1_READ_FIFO_STATUSr(unit,ci,&uRegValue));
  status = DDR23_GET_FIELD(uRegValue, DDR23_PHY_BYTE_LANE1, READ_FIFO_STATUS, STATUS);
  if (status != 0) {
    /* soc_cm_debug(DK_ERR | DK_VERBOSE, "ci %d read fifo status on bytelane 1, status 0x%x\n", ci, status); */
    rv = SOC_E_INTERNAL;
  }

  sal_usleep(200);

  return rv;
}

static int
sbx_sirius_ddr_reset_read_fifo(unit, ci)
{
  int rv = SOC_E_NONE;
  uint uRegValue;
  uint status;

  SOC_IF_ERROR_RETURN(READ_CI_CONFIG3r(unit,ci,&uRegValue));
  soc_reg_field_set(unit,CI_CONFIG3r,&uRegValue,PHY_UPD_VDL_BL0f,0x1);
  soc_reg_field_set(unit,CI_CONFIG3r,&uRegValue,PHY_UPD_VDL_BL1f,0x1);
  /* soc_reg_field_set(unit,CI_CONFIG3r,&uRegValue,PHY_UPD_VDL_ADDRf,0x1); */
  SOC_IF_ERROR_RETURN(WRITE_CI_CONFIG3r(unit,ci,uRegValue));

  sal_usleep(200);

  SOC_IF_ERROR_RETURN(READ_DDR23_PHY_BYTE_LANE0_READ_FIFO_CLEARr(unit,ci,&uRegValue));
  DDR23_SET_FIELD(uRegValue, DDR23_PHY_BYTE_LANE0, READ_FIFO_CLEAR, CLEAR, 1);
  SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE0_READ_FIFO_CLEARr(unit,ci,uRegValue));

  SOC_IF_ERROR_RETURN(READ_DDR23_PHY_BYTE_LANE1_READ_FIFO_CLEARr(unit,ci,&uRegValue));
  DDR23_SET_FIELD(uRegValue, DDR23_PHY_BYTE_LANE1, READ_FIFO_CLEAR, CLEAR, 1);
  SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE1_READ_FIFO_CLEARr(unit,ci,uRegValue));

  sal_usleep(200);

  SOC_IF_ERROR_RETURN(READ_DDR23_PHY_BYTE_LANE0_READ_FIFO_STATUSr(unit,ci,&uRegValue));
  status = DDR23_GET_FIELD(uRegValue, DDR23_PHY_BYTE_LANE0, READ_FIFO_STATUS, STATUS);
  if (status != 0) {
    soc_cm_debug(DK_ERR | DK_VERBOSE, "ci %d failed to clear read fifo status on bytelane 0, status 0x%x\n", ci, status);
    rv = SOC_E_INTERNAL;
  }

  SOC_IF_ERROR_RETURN(READ_DDR23_PHY_BYTE_LANE1_READ_FIFO_STATUSr(unit,ci,&uRegValue));
  status = DDR23_GET_FIELD(uRegValue, DDR23_PHY_BYTE_LANE1, READ_FIFO_STATUS, STATUS);
  if (status != 0) {
    soc_cm_debug(DK_ERR | DK_VERBOSE, "ci %d failed to clear read fifo status on bytelane 1, status 0x%x\n", ci, status);
    rv = SOC_E_INTERNAL;
  }

  sal_usleep(200);

  return rv;
}

char cmd_sbx_sirius_print_queue_info_usage[] = "\n"
" queueinfo <q>\n"
" qi <q>\n"
"\n";

int
cmd_sbx_sirius_print_queue_info(int unit, int queue)
{
    int rv;
    bcm_sbx_cosq_queue_region_type_t qr =  bcm_sbx_cosq_queue_region_global;
    bcm_sbx_cosq_queue_state_t qstate;
    bcm_sbx_cosq_bw_group_state_t bwstate;
    bcm_sbx_subport_info_t sp_info;
    int qmap_idx=0, eg_n = 0, ds_id = 0, fifo = -1, i = 0, fcd=0;
    bcm_sbx_mgid_list_t *mgid_list = NULL;
    dq_p_t mcItem;
    bcm_sbx_mgid_list_t *mcData;

    if ((queue < 0) | (queue >= SB_FAB_DEVICE_SIRIUS_NUM_QUEUES)) {
      return BCM_E_PARAM;
    }

    sal_memset(&qstate, 0, sizeof(bcm_sbx_cosq_queue_state_t));
    sal_memset(&bwstate, 0, sizeof(bcm_sbx_cosq_bw_group_state_t));
    sal_memset(&sp_info, 0, sizeof(bcm_sbx_subport_info_t));
    rv = bcm_sbx_cosq_qinfo_get(unit, queue, &ds_id, &fifo, &qstate, &bwstate, &qr);

    if(rv) {
        printk("Failed to get queue info for Queue=%d status=%d\n",
               queue, rv);
        return CMD_FAIL;
    } else {
	if(qstate.ingress.enabled) {
            if (bwstate.dest_mc) {
                mgid_list = qstate.mgid_list;
                printk("q=%d (base q=%d) is %sABLED Multicast %s mode:\nNode=%d Port=%d cos=%d ds_id=%d\n",
                       queue, bwstate.base_queue, qstate.ingress.enabled ? "EN":"DIS",
                       qr ? "SUBSCRIBER" : "FIC",
                       bwstate.dest_node, bwstate.dest_port, bwstate.num_cos, ds_id);
                qmap_idx = 0;
                if (qstate.mgid_list) {
                    DQ_TRAVERSE(&(qstate.mgid_list->node), mcItem)
                        mcData = (bcm_sbx_mgid_list_t*)mcItem;
                        if (0 == qmap_idx) {
                            printk("mc groups: ");
                        } else if (0 == (qmap_idx % 10)) {
                            printk("           ");
                        }
                        printk("%5d ", mcData->data);
                        qmap_idx++;
                        if (0 == (qmap_idx % 10)) {
                            printk("\n");
                        }
                    DQ_TRAVERSE_END(&(qstate.mgid_list->node), mcItem);
                }
                if (0 == qmap_idx) {
                    printk("no associated mc groups\n");
                } else {
                    if (0 != (qmap_idx % 10)) {
                        printk("\n");
                    }
                }
            } else {
		rv = bcm_sbx_port_qinfo_get(unit, bwstate.gport, &eg_n, &sp_info);
		if(rv) {
		    printk("Failed to get subport info for Gport=0x%x status=%d\n",
			   bwstate.gport, rv);
		    return CMD_FAIL;
		}
		fcd = (qstate.ingress.bw_mode == BCM_COSQ_EF) ? sp_info.egroup[eg_n].ef_fcd : sp_info.egroup[eg_n].nef_fcd;
		for (i=0; i<16; i++)
		    if (qstate.sysport == (sp_info.egroup[eg_n].fcd[i] & 0xfff)) {
			fifo = i;
			fcd = sp_info.egroup[eg_n].fcd[i] >> 12;
			break;
		    }
                qmap_idx = (bwstate.dest_node << 7) | bwstate.dest_port;
                printk("q=%d (base q=%d) is %sABLED Unicast %s mode:\nNode=%d Port=%d cos=%d fifos=%d fifo_idx=%d\n"
                       "qsel[1]idx=0x%x sysport=0x%x fcd=%d\ningress gport=0x%x egress gport=0x%x\n",
                       queue, bwstate.base_queue, qstate.ingress.enabled ? "EN":"DIS",
                       qr ? "SUBSCRIBER" : "FIC",
                       bwstate.dest_node, bwstate.dest_port, bwstate.num_cos,
                       sp_info.egroup[eg_n].num_fifos, (sp_info.egroup[eg_n].es_scheduler_level0_node + fifo),
		       qmap_idx, qstate.sysport, fcd,
		       bwstate.gport, sp_info.egroup[eg_n].egroup_gport);
            }
	} else {
	    printk("q=%d is DISABLED\n",queue);
	}
    }
    return CMD_OK;
}

#endif /* BCM_SIRIUS_SUPPORT */
