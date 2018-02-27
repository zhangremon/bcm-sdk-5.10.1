/* -*-  Mode:C; c-basic-offset:4 -*- */
/*****************************************************************************
 *
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
 *
 * $Id: sbx_diags.c 1.7.40.1 Broadcom SDK $
 * Contains source code for SBX diagnostics tests
 * ******************************************************************************/
#ifdef BCM_SBX_SUPPORT
#include <appl/diag/shell.h>
#include <bcm/l2.h>
#include <bcm/error.h>
#include "soc/drv.h"
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_pt_auto.h>
#include <soc/sbx/hal_ka_auto.h>
#include <soc/sbx/sbx_drv.h>
#ifdef BCM_QE2000_SUPPORT
#include <soc/sbx/qe2000.h>
#endif
#ifndef __KERNEL__
#include <signal.h>
#endif
#include <appl/test/sbx_diags.h>
#include "sbWrappers.h"

#ifdef BCM_BME3200_SUPPORT
const char* sbBme3200DiagsMemoryTestStr[] = {"BME3200_DATA_PATTERN_TEST",
					     "BME3200_AAA_TEST"};

uint32_t aulDgBmeTable[SB_BME_MAX_BANK_K] = { 0x6, 0xD };
#endif

#ifdef BCM_QE2000_SUPPORT
const char* sbQe2000DiagsBistTestStr[] =    {"QE2000_INTERNAL_BIST_TEST",
					     "QE2000_EXTERNAL_BIST_TEST"};

static const char acQe2kIntBistStr[15][3] =
{
    "SR0",
    "SR1",
    "ST0",
    "ST1",
    "PC",
    "EB",
    "TX",
    "SV",
    "QM",
    "PM",
    "EG",
    "EI",
    "RB",
    "QS"
};
#endif

extern int _mc_fpga_read8(int addr, uint8_t *v);
uint8_t sbx_diag_get_board_type()
{

  int rv;
  uint8_t v = 0;
  /* read the fpga at offset 0x02 == brdtype */
  rv = _mc_fpga_read8(FPGA_BOARD_ID_OFFSET,&v);
  if (rv) {
    printk("FPGA read failed (%d)\n",rv);
    return (-1);
  }
  return v;
}

uint8_t sbx_diag_get_board_rev()
{

  int rv;
  uint8_t v = 0;
  switch(sbx_diag_get_board_type()) {
  case BOARD_TYPE_POLARIS_LC:
  case BOARD_TYPE_FE2KXT_QE2K_POLARIS_LC:
    rv = _mc_fpga_read8(FPGA_BOARD_REV_OFFSET,&v);
    if (rv) {
    printk("FPGA read failed (%d)\n",rv);
    return (-1);
    } else {
      return v;
    }
  default:
    return -1;
  }
}


#ifdef BCM_BME3200_SUPPORT
/*
 **********************************************************************************
 *   BME3200 Tests
 **********************************************************************************
 */

int32_t sbBme3200BistStart(sbxDiagsInfo_t *pDiagsInfo)
{

  int status;
  uint32_t stat0_ul=0;
  uint32_t stat1_ul=0;
  uint32_t stat2_ul=0;
  uint32_t stat3_ul=0;

  status = bme32kBIST(pDiagsInfo,
		      &stat0_ul,
		      &stat1_ul,
		      &stat2_ul,
		      &stat3_ul);
  if(1 == status) {
    SB_LOG( "Memory Repaired, passed\n");
    return 0;
  } else if(0 == status) {
    return 0;
  } else if( -3 == status) {
    SB_LOG( "Failed:  BIST go status:\n");
    SB_LOG( "g0119_96=0x%08X, go95_64=0x%08X, go63_32=0x%8X,   go31_00=0x%08X\n",
	      stat0_ul, stat1_ul, stat2_ul, stat3_ul);
  }
  else if( (-2 == status) || (-1 == status)) {
    SB_LOG( "Bank 0 Mem 0:  0x%08X\n", stat0_ul);
    SB_LOG( "Bank 0 Mem 1:  0x%08X\n", stat1_ul);
    SB_LOG( "Bank 1 Mem 0:  0x%08X\n", stat2_ul);
    SB_LOG( "Bank 1 Mem 1:  0x%08X\n", stat3_ul);
  }

  return status;
}

int bme32kBIST(sbxDiagsInfo_t *pDiagsInfo,
	       uint32_t *stat0_pul,
	       uint32_t *stat1_pul,
	       uint32_t *stat2_pul,
	       uint32_t *stat3_pul)
{

  uint64_t data_ul;
  uint64_t timeout_ul;
  uint64_t status0_ul;
  uint64_t status1_ul;
  uint64_t status2_ul;
  uint64_t status3_ul;
  uint64_t bwmemstatus_aul[4];
  int retcode;
  int j;
  sbhandle bmeaddr;
  int diagVerbose_b;

  bmeaddr = pDiagsInfo->userDeviceHandle;
  diagVerbose_b = pDiagsInfo->debug_level;
  retcode = 0;

  /* Reset the BME32 */
  sbBme32kSoftReset(bmeaddr);
  data_ul  = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_MASTER_CONFIG);
  if (SAND_HAL_GET_FIELD(PT, PI_MBIST_MASTER_CONFIG, MBIST_MASTER_DONE, data_ul)) {
    SB_LOG(  "WARNING: MBist already done once, PI_MBIST_MASTER_CONFIG=0x%lx. MBist is skipped.\n",
	       data_ul);
  }

  SAND_HAL_WRITE(bmeaddr, PT, PI_MBIST_SLAVE_CONFIG0, 0xffffffff);
  SAND_HAL_WRITE(bmeaddr, PT, PI_MBIST_SLAVE_CONFIG1, 0xffffffff);
  SAND_HAL_WRITE(bmeaddr, PT, PI_MBIST_SLAVE_CONFIG2, 0xffffffff);
  SAND_HAL_WRITE(bmeaddr, PT, PI_MBIST_SLAVE_CONFIG3, 0x3fff);

  data_ul = 0;
  /* all phases */
  data_ul = SAND_HAL_MOD_FIELD(PT, PI_MBIST_MASTER_CONFIG, MBIST_MASTER_ALGO_MODE, data_ul, 0x0);
  /* run from PI */
  data_ul = SAND_HAL_MOD_FIELD(PT, PI_MBIST_MASTER_CONFIG, MBIST_MASTER_MODE, data_ul, 0x2);
  SAND_HAL_WRITE(bmeaddr, PT, PI_MBIST_MASTER_CONFIG, data_ul);
  /* start the test */
  data_ul = SAND_HAL_MOD_FIELD(PT, PI_MBIST_MASTER_CONFIG, MBIST_MASTER_ENABLE, data_ul, 0x1);
  SAND_HAL_WRITE(bmeaddr, PT, PI_MBIST_MASTER_CONFIG, data_ul);

  /* check for MBist done */
  timeout_ul = 1000;
  while (timeout_ul)
    {
      data_ul  = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_MASTER_CONFIG);
      if (SAND_HAL_GET_FIELD(PT, PI_MBIST_MASTER_CONFIG, MBIST_MASTER_DONE, data_ul))
        {
	  break;
        }
      thin_delay(5000);
      timeout_ul--;
    }

  if (SAND_HAL_GET_FIELD(PT, PI_MBIST_MASTER_CONFIG, MBIST_MASTER_DONE, data_ul))
    {
      if (SAND_HAL_GET_FIELD(PT, PI_MBIST_MASTER_CONFIG, MBIST_MASTER_GO, data_ul))
        {
	  if(diagVerbose_b) {
	    SB_LOG(  "MBIST done with no error. PI_MBIST_MASTER_CONFIG=0x%lx\n", data_ul);
	  }
	  /* dump individual MBist status */
	  status0_ul = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_DONE_STATUS0);
	  status1_ul = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_DONE_STATUS1);
	  status2_ul = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_DONE_STATUS2);
	  status3_ul = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_DONE_STATUS3);
	  if(diagVerbose_b) {
	    SB_LOG(  "Summary: done119_96=0x%lx, done95_64=0x%lx, done63_32=0x%lx, done31_00=0x%lx\n",
		       status3_ul, status2_ul, status1_ul, status0_ul);
	  }

	  status0_ul = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_GO_STATUS0);
	  status1_ul = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_GO_STATUS1);
	  status2_ul = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_GO_STATUS2);
	  status3_ul = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_GO_STATUS3);
	  if(diagVerbose_b) {
	    SB_LOG(  "           go119_96=0x%lx,   go95_64=0x%lx,   go63_32=0x%lx,   go31_00=0x%lx\n",
		       status3_ul, status2_ul, status1_ul, status0_ul);
	  }
        }
      else
        {
	  SB_LOG(  "MBIST done with error. PI_MBIST_MASTER_CONFIG=0x%lx\n", data_ul);
	  /* dump individual MBist status */
	  status0_ul = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_DONE_STATUS0);
	  status1_ul = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_DONE_STATUS1);
	  status2_ul = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_DONE_STATUS2);
	  status3_ul = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_DONE_STATUS3);
	  SB_LOG(  "Summary: done119_96=0x%lx, done95_64=0x%lx, done63_32=0x%lx, done31_00=0x%lx\n",
		     status3_ul, status2_ul, status1_ul, status0_ul);

	  status0_ul = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_GO_STATUS0);
	  status1_ul = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_GO_STATUS1);
	  status2_ul = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_GO_STATUS2);
	  status3_ul = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_GO_STATUS3);
	  SB_LOG(  "           go119_96=0x%lx,   go95_64=0x%lx,   go63_32=0x%lx,   go31_00=0x%lx\n",
		     status3_ul, status2_ul, status1_ul, status0_ul);
	  *stat0_pul = status0_ul;
	  *stat1_pul = status1_ul;
	  *stat2_pul = status2_ul;
	  *stat3_pul = status3_ul;
	  retcode = -3;
        }
    }
  else
    {
      SB_LOG(  "Time out waiting for all MBIST done. PI_MBIST_MASTER_CONFIG=0x%lx\n", data_ul);
      /* dump individual MBist status */
      status0_ul = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_DONE_STATUS0);
      status1_ul = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_DONE_STATUS1);
      status2_ul = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_DONE_STATUS2);
      status3_ul = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_DONE_STATUS3);
      SB_LOG(  "Summary: done119_96=0x%lx, done95_64=0x%lx, done63_32=0x%lx, done31_00=0x%lx\n",
		 status3_ul, status2_ul, status1_ul, status0_ul);

      status0_ul = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_GO_STATUS0);
      status1_ul = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_GO_STATUS1);
      status2_ul = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_GO_STATUS2);
      status3_ul = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_GO_STATUS3);
      SB_LOG(  "           go119_96=0x%lx,   go95_64=0x%lx,   go63_32=0x%lx,   go31_00=0x%lx\n",
		 status3_ul, status2_ul, status1_ul, status0_ul);
    }

  if ( 0 == retcode)
    {
      data_ul = SAND_HAL_READ(bmeaddr, PT, BW_MEM_STATUS);

      bwmemstatus_aul[0] = SAND_HAL_GET_FIELD(PT, BW_MEM_STATUS, BANK0_MEM0, data_ul);
      bwmemstatus_aul[1] = SAND_HAL_GET_FIELD(PT, BW_MEM_STATUS, BANK0_MEM1, data_ul);
      bwmemstatus_aul[2] = SAND_HAL_GET_FIELD(PT, BW_MEM_STATUS, BANK1_MEM0, data_ul);
      bwmemstatus_aul[3] = SAND_HAL_GET_FIELD(PT, BW_MEM_STATUS, BANK1_MEM1, data_ul);

      *stat0_pul = bwmemstatus_aul[0];
      *stat1_pul = bwmemstatus_aul[1];
      *stat2_pul = bwmemstatus_aul[2];
      *stat3_pul = bwmemstatus_aul[3];

      /* Check for BW mem bank errors */
      for (j = 0; j < 4; j++)
        {
	  if (bwmemstatus_aul[j] == 0x3)
            {
	      SB_LOG(  "Bw Memory Bank%d, Mem%d has unrepairable error, BW_MEM_STATUS=0x%lx\n",
			 (int)bwmemstatus_aul[j]/2, (int)bwmemstatus_aul[j]&1, data_ul);
	      retcode = -1;
	      break;
            }

	  if (bwmemstatus_aul[j] == 0x0)
            {
	      SB_LOG(  "Bw Memory Bank%d, Mem%d did not complete MBIST, BW_MEM_STATUS=0x%lx\n",
			 (int)bwmemstatus_aul[j]/2, (int)bwmemstatus_aul[j]&1, data_ul);
	      retcode = -2;
	      break;
            }

	  if (bwmemstatus_aul[j] == 0x2)
            {
	      SB_LOG(  "WARNING: Bw Memory Bank%d, Mem%d was repaired. BW_MEM_STATUS=0x%lx\n",
			 (int)bwmemstatus_aul[j]/2, (int)bwmemstatus_aul[j]&1, data_ul);
	      /* This is not necessarily bad */
	      retcode = 1;
            }
        }
    }

  /* turn off MBist */
  SAND_HAL_WRITE(bmeaddr, PT, PI_MBIST_SLAVE_CONFIG0, 0);
  SAND_HAL_WRITE(bmeaddr, PT, PI_MBIST_SLAVE_CONFIG1, 0);
  SAND_HAL_WRITE(bmeaddr, PT, PI_MBIST_SLAVE_CONFIG2, 0);
  SAND_HAL_WRITE(bmeaddr, PT, PI_MBIST_SLAVE_CONFIG3, 0);
  SAND_HAL_WRITE(bmeaddr, PT, PI_MBIST_MASTER_CONFIG, 0);

  return (retcode);

}

void sbBme32kSoftReset(sbhandle bmeaddr)
{

    uint32_t ulBmeReg;
    ulBmeReg = SAND_HAL_READ(bmeaddr, PT, PI_RESET);
    ulBmeReg = SAND_HAL_MOD_FIELD(PT, PI_RESET, RESET, ulBmeReg, 0x1);
    SAND_HAL_WRITE(bmeaddr, PT, PI_RESET, ulBmeReg);
    thin_delay(100);
    ulBmeReg = SAND_HAL_MOD_FIELD(PT, PI_RESET, RESET, ulBmeReg, 0x0);
    SAND_HAL_WRITE(bmeaddr, PT, PI_RESET, ulBmeReg);
    thin_delay(100);

}

int sbBme32KBwRedundSet(sbxDiagsInfo_t *pDiagsInfo)
{

    uint32_t ulBmeReg;
    uint32_t memstate;
    int nRedunancyOff = pDiagsInfo->nBmeRedundancyOff;
    sbhandle bmeaddr = pDiagsInfo->userDeviceHandle;
    if(!nRedunancyOff)
    {
	memstate = 3;
    }
    else
    {
	memstate = 0;
    }

    ulBmeReg = SAND_HAL_READ(bmeaddr, PT, BW_MODE);
    ulBmeReg = SAND_HAL_MOD_FIELD(PT, BW_MODE, DISABLE_REDUNDANT_MEM, ulBmeReg, memstate);
    SAND_HAL_WRITE(bmeaddr, PT, BW_MODE, ulBmeReg);

    return (0);

}

int sbBme32KMBistSet(sbhandle bmeaddr, int onoff)
{

  uint32_t ulBmeReg;
  ulBmeReg = SAND_HAL_READ(bmeaddr, PT, PI_MBIST_MASTER_CONFIG);
  ulBmeReg = SAND_HAL_MOD_FIELD(PT, PI_MBIST_MASTER_CONFIG, MBIST_MASTER_ENABLE, ulBmeReg, onoff);
  SAND_HAL_WRITE(bmeaddr, PT, PI_MBIST_MASTER_CONFIG, ulBmeReg);
  return (0);
}

int sbBme3200DiagsSramMemTestAll(sbxDiagsInfo_t *pDiagsInfo)
{

  uint32_t mem_test;
  int stat = 0;
  int nMemTests = SB_BME3200_MEM_TEST_LAST-1;
  if (pDiagsInfo->pattern == -1) {
#ifndef __KERNEL__
    pDiagsInfo->pattern = rand() & 0xffffffff;
#else
    
#endif
  }
  for(mem_test=0;mem_test<SB_BME3200_MEM_TEST_LAST;mem_test++) {
    pDiagsInfo->e_bme_mem_test = mem_test;
    SB_LOG("Running Test[%1d..%1d] %-20s ",mem_test,nMemTests,sbBme3200DiagsMemoryTestStr[mem_test]);
    if (mem_test == SB_BME3200_DATA_PATTERN) {
      SB_LOG(" (0x%llx) ",pDiagsInfo->pattern);
    }
    if (sbBme3200DiagsSramMemTest(pDiagsInfo) !=0 ) {
      stat = -1;
      if (mem_test == SB_BME3200_DATA_PATTERN) {
	SB_LOG("FAILED To re-run use MemTest=%d Verbose=1 Pattern=0x%llx",mem_test,pDiagsInfo->pattern);
      } else {
	SB_LOG("FAILED To re-run use MemTest=%d Verbose=1\n",mem_test);
      }
    }
  }
  return stat;
}

int sbBme3200DiagsSramMemTest(sbxDiagsInfo_t *pDiagsInfo)
{
  int stat=0;
  switch(pDiagsInfo->e_bme_mem_test) {
  case SB_BME3200_DATA_PATTERN:
    stat = sbBmeDataPat(pDiagsInfo);
    break;
  case SB_BME3200_AAA_TEST:
    stat = sbBme32SramAAATest(pDiagsInfo);
    break;
  default:
    SB_ERROR("ERROR:\n%s: Invalid mem test:%d [0-%d supported]\n",__PRETTY_FUNCTION__,
	     pDiagsInfo->e_bme_mem_test,SB_BME3200_MEM_TEST_LAST-1);
    return (-1);
  }
  if (stat != 0) {
    SB_LOG("FAILED\n");
  } else {
    SB_LOG("PASSED\n");
  }
  return stat;
}

int32_t sbBmeDataPat(sbxDiagsInfo_t *pDiagsInfo)
{

    int status;
    uint32_t address_ul;
    uint32_t tableid_ul;
    uint32_t bankid_ul;
    uint32_t data_ul;

    data_ul = address_ul = tableid_ul = bankid_ul = 0;

    status = sbBme32SramDataPatTest(pDiagsInfo,
                                    &address_ul,
                                    &data_ul,
                                    &tableid_ul,
                                    &bankid_ul);
    /*
    ** Check status, return error codes appropriately
    */
    if (0 != status) {
	if(-2 == status) {
	    SB_LOG( "Failed to read bank %d table %d address 0x%08X\n",
		      bankid_ul, tableid_ul, address_ul);
	} else if(-3 == status) {
	    SB_LOG( "Failed to write bank %d table %d address 0x%08X\n",
		      bankid_ul, tableid_ul, address_ul);
	} else if(-1 == status) {
	    SB_LOG( "Failed at Bank %d Table %d Address 0x%08X got 0x%08X should have been 0x%08x\n",
		    bankid_ul, tableid_ul, address_ul, data_ul, pDiagsInfo->pattern);
	} else {
	    SB_LOG( "Unknown failure\n");
	}
	return -1;
    }

    return 0;

}

int sbBme32SramDataPatTest(sbxDiagsInfo_t *pDiagsInfo,
			   uint32_t *pulAddress,
			   uint32_t *pulData,
			   uint32_t *pulTblId,
			   uint32_t *pulBankId)
{
    uint32_t ulTstOffset;
    uint32_t ulRdData, ulBankId;
    int status;
    int preverror;
    sbBme32kIndirectMemRead_pf read_pf;
    sbBme32kIndirectMemWrite_pf write_pf;
    uint32_t ulDataPattern;
    int diagVerbose_b;
    sbhandle bmeaddr;
    ulDataPattern = pDiagsInfo->pattern & 0xffffffff;
    diagVerbose_b = pDiagsInfo->debug_level;
    bmeaddr = pDiagsInfo->userDeviceHandle;
    preverror = 0;

    sbBme32kSoftReset(bmeaddr);
    sbBme32KBwRedundSet(pDiagsInfo);
    /* turn off bist */
    sbBme32KMBistSet(bmeaddr, 0);

    if (diagVerbose_b) {
      SB_LOG("%s: Testing with pattern 0x%08x",__PRETTY_FUNCTION__,ulDataPattern);
    }

    /*
     * Write the test pattern out to sram
     */
    for (ulBankId = 0; ulBankId < SB_BME_MAX_BANK_K; ulBankId++)
    {
	if (0 == ulBankId)
	{
	    read_pf = sbBme32kBwR0Read;
	    write_pf = sbBme32kBwR0Write;
	}
	else
	{
	    read_pf = sbBme32kBwR1Read;
	    write_pf = sbBme32kBwR1Write;
	}

	for (ulTstOffset = 0; ulTstOffset < SB_BME_MAX_OFFSET_K; ulTstOffset++)
	{
	    status = write_pf(bmeaddr, aulDgBmeTable[ulBankId], ulTstOffset, ulDataPattern);

	    if (0 != status)
	    {
		SB_LOG("\n%s: Error during indirect write to Table %d address 0x%08X\n",
			  __FUNCTION__,
			  aulDgBmeTable[ulBankId],
			  ulTstOffset);
		*pulAddress = ulTstOffset;
		*pulTblId =  aulDgBmeTable[ulBankId];
		*pulBankId = ulBankId;
		/* we were writing during the failure */
		*pulData = 1;
		return (-3);
	    }

	    if (diagVerbose_b)
	    {
		if ( (0 == (ulTstOffset % 0x8000)) && (0 != ulTstOffset))
		{
		    SB_LOG(".");
		}
	    }
	} /* end for each offset */


	/*
	 * Read back and compare
	 */
	for (ulTstOffset = 0; ulTstOffset < SB_BME_MAX_OFFSET_K; ulTstOffset++)
	{
	    ulRdData = 0;

	    if ( (0 == (ulTstOffset % 0x1000)) && (0 != ulTstOffset))
	      {
		SB_LOG(".");
	      }

	    /*
	     * Read the address, no clear on read
	     */
	    status = read_pf(bmeaddr, aulDgBmeTable[ulBankId], ulTstOffset, &ulRdData);
	    if (0 != status)
	    {
	      SB_LOG("%s: Error during indirect read of Table %d address 0x%08X\n",
			  __FUNCTION__,
			  aulDgBmeTable[ulBankId],
			  ulTstOffset);
		*pulAddress = ulTstOffset;
		*pulTblId = aulDgBmeTable[ulBankId];
		*pulBankId = ulBankId;
		/* we were reading at the time of the failure */
		*pulData = 0;
		return (-2);
	    }

	    /*
	     * We now have the data, see if it matches
	     */
	    if (ulRdData != ulDataPattern)
	    {
	      SB_LOG("Data compare failure at Table %d address 0x%08X\n",
			  aulDgBmeTable[ulBankId],
			  ulTstOffset);
	      SB_LOG("Expected 0x%08X  got 0x%08X\n",
			  ulDataPattern, ulRdData);
		if (0 == preverror)
		{
		    *pulAddress = ulTstOffset;
		    *pulData = ulRdData;
		    *pulTblId = aulDgBmeTable[ulBankId];
		    *pulBankId = ulBankId;
		    preverror = 1;
		}
	    }
	} /* end for each offset */
    } /* end for each bank */

    if (0 != preverror)
    {
        return (-1);
    }

    *pulAddress = 0;
    *pulData = 0;
    *pulTblId = 0;
    *pulBankId = 0;

    return (0);
} /* end sbBme32SramDataPatTest */

int sbBme32SramAAATest(sbxDiagsInfo_t *pDiagsInfo)
{

    int status;
    uint32_t address_ul;
    uint32_t data_ul;
    uint32_t tableid_ul;
    uint32_t bankid_ul;
    address_ul = data_ul = tableid_ul = bankid_ul = 0;

    status = sbBme32SramAAAMemTest(pDiagsInfo,
                                   &address_ul,
                                   &data_ul,
                                   &tableid_ul,
                                   &bankid_ul);
    /*
    ** Check status, return error codes appropriately
    */
    if (0 != status) {
	if(-3 == status) {
	    SB_LOG("Failed to read bank %d table %d address 0x%08X\n",
		      bankid_ul, tableid_ul, address_ul);
	} else if(-2 == status) {
	    SB_LOG("Failed to write bank %d table %d address 0x%08X\n",
		      bankid_ul, tableid_ul, address_ul);
	} else if(-1 == status) {
	    SB_LOG( "Failed at Bank %d Table %d Address 0x%08X got 0x%08X\n",
		      bankid_ul, tableid_ul, address_ul, data_ul);
	} else {
	    SB_LOG( "Unknown failure\n");
	}
	return -1;
    }

    return 0;
}

int sbBme32SramAAAMemTest(sbxDiagsInfo_t *pDiagsInfo,
			  uint32_t *pulAddress,
			  uint32_t *pulData,
			  uint32_t *pulTblId,
			  uint32_t *pulBankId)
{
    uint32_t ulTstOffset;
    uint32_t ulRdData, ulBankId;
    int status;
    int preverror;
    sbBme32kIndirectMemRead_pf read_pf;
    sbBme32kIndirectMemWrite_pf write_pf;
    int verbose = pDiagsInfo->debug_level;
    sbhandle bmeaddr = pDiagsInfo->userDeviceHandle;

    sbBme32kSoftReset(bmeaddr);
    sbBme32KBwRedundSet(pDiagsInfo);
    /* turn off bist */
    sbBme32KMBistSet(bmeaddr, 0);
    preverror = 0;

    if (verbose) {
      SB_LOG("%s Starting Address at Address Test",__PRETTY_FUNCTION__);
    }

    /*
     * Write the test pattern out to sram
     */
    for (ulBankId = 0; ulBankId < SB_BME_MAX_BANK_K; ulBankId++)
    {
	if (0 == ulBankId)
	{
	    read_pf = sbBme32kBwR0Read;
	    write_pf = sbBme32kBwR0Write;
	}
	else
	{
	    read_pf = sbBme32kBwR1Read;
	    write_pf = sbBme32kBwR1Write;
	}

	for (ulTstOffset = 0; ulTstOffset < SB_BME_MAX_OFFSET_K; ulTstOffset++)
	{
	    status = write_pf(bmeaddr, aulDgBmeTable[ulBankId], ulTstOffset, ulTstOffset);

	    if (0 != status)
	    {
		SB_LOG("\n%s: Error during indirect write to Table %d address 0x%08X\n",
			  __FUNCTION__,
			  aulDgBmeTable[ulBankId],
			  ulTstOffset);
		*pulAddress = ulTstOffset;
		*pulTblId = aulDgBmeTable[ulBankId];
		*pulBankId = ulBankId;
		/* we were writing during the failure */
		*pulData = 1;
		return (-3);
	    }

	    if ( (0 == (ulTstOffset % 0x1000)) && (0 != ulTstOffset))
	      {
		SB_LOG(".");
	      }
	} /* end for each offset */

	/*
	 * Read back and compare
	 */
	for (ulTstOffset = 0; ulTstOffset < SB_BME_MAX_OFFSET_K; ulTstOffset++)
	{
	    ulRdData = 0;

	    if (verbose)
	    {
		if ( (0 == (ulTstOffset % 0x1000)) && (0 != ulTstOffset))
		{
		    SB_LOG(".");
		}
	    }

	    /*
	     * Read the address, no clear on read
	     */
	    status = read_pf(bmeaddr, aulDgBmeTable[ulBankId], ulTstOffset, &ulRdData);
	    if (0 != status)
	    {
		SB_LOG("%s: Error during indirect read of Table %d address 0x%08X\n",
			  __FUNCTION__,
			  aulDgBmeTable[ulBankId],
			  ulTstOffset);
		*pulAddress = ulTstOffset;
		*pulTblId = aulDgBmeTable[ulBankId];
		*pulBankId = ulBankId;
		/* we were reading at the time of the failure */
		*pulData = 0;
		return (-2);
	    }

	    /*
	     * We now have the data, see if it matches
	     */
	    if (ulRdData != ulTstOffset)
	    {
		SB_LOG("Data compare failure at Table %d address 0x%08X\n",
			  aulDgBmeTable[ulBankId],
			  ulTstOffset);
		SB_LOG("Expected 0x%08X  got 0x%08X\n",
			  ulTstOffset, ulRdData);
		if (0 == preverror)
		{
		    *pulAddress = ulTstOffset;
		    *pulData = ulRdData;
		    *pulTblId = aulDgBmeTable[ulBankId];
		    *pulBankId = ulBankId;
		    preverror = 1;
		}
	    }
	} /* end for each offset */

    } /* end each bank */

    if (0 != preverror)
    {
        return (-1);
    }

    *pulAddress = 0;
    *pulData = 0;
    *pulTblId = 0;

    return (0);
} /* end sbBme32SramAAAMemTest */

int sbBme32kBwR0Read(sbhandle bmeaddr, uint32_t ulTblId, uint32_t ulOffset, uint32_t *pulData)
{
    uint32_t ulBmeCtrl;
    uint32_t ulTimeOut;

    ulBmeCtrl = SAND_HAL_READ(bmeaddr, PT, BW_R0_ACC_CTRL);
    /*
     * Clear any pending ACK
     */
    ulBmeCtrl = SAND_HAL_MOD_FIELD(PT, BW_R0_ACC_CTRL, REQ, ulBmeCtrl, 0);
    SAND_HAL_WRITE(bmeaddr, PT, BW_R0_ACC_CTRL, ulBmeCtrl);

    ulBmeCtrl = SAND_HAL_READ(bmeaddr, PT, BW_R0_ACC_CTRL);

    ulBmeCtrl = SAND_HAL_MOD_FIELD(PT, BW_R0_ACC_CTRL, REQ, ulBmeCtrl, 1);
    /* Write OP */
    ulBmeCtrl = SAND_HAL_MOD_FIELD(PT, BW_R0_ACC_CTRL, RD_WR_N, ulBmeCtrl, 1);
    /* Table ID */
    ulBmeCtrl = SAND_HAL_MOD_FIELD(PT, BW_R0_ACC_CTRL, TBL_ID, ulBmeCtrl, ulTblId);
    /* Offset */
    ulBmeCtrl = SAND_HAL_MOD_FIELD(PT, BW_R0_ACC_CTRL, OFS, ulBmeCtrl, ulOffset);

    SAND_HAL_WRITE(bmeaddr, PT, BW_R0_ACC_CTRL, ulBmeCtrl);
    /* Wait for the operation to complete */
    ulTimeOut = 500;
    while (ulTimeOut)
    {
	ulBmeCtrl = SAND_HAL_READ(bmeaddr, PT, BW_R0_ACC_CTRL);
	if ( 1 == SAND_HAL_GET_FIELD(PT, BW_R0_ACC_CTRL, ACK, ulBmeCtrl))
	    break;
	thin_delay(BME32K_MIN_CLOCK_DELAY);
	ulTimeOut--;
    }

    if(0 == ulTimeOut)
    {
	return (-1);
    }

    *pulData = SAND_HAL_READ(bmeaddr, PT, BW_R0_ACC_DAT);

    return (0);

}

int sbBme32kBwR0Write(sbhandle bmeaddr, uint32_t ulTblId, uint32_t ulOffset, uint32_t ulData)
{
    uint32_t ulBmeCtrl;
    uint32_t ulTimeOut;

    ulBmeCtrl = SAND_HAL_READ(bmeaddr, PT, BW_R0_ACC_CTRL);
    /*
     * Clear any pending ACK
     */
    ulBmeCtrl = SAND_HAL_MOD_FIELD(PT, BW_R0_ACC_CTRL, REQ, ulBmeCtrl, 0);
    SAND_HAL_WRITE(bmeaddr, PT, BW_R0_ACC_CTRL, ulBmeCtrl);

    ulBmeCtrl = SAND_HAL_READ(bmeaddr, PT, BW_R0_ACC_CTRL);

    ulBmeCtrl = SAND_HAL_MOD_FIELD(PT, BW_R0_ACC_CTRL, REQ, ulBmeCtrl, 1);
    /* Write OP */
    ulBmeCtrl = SAND_HAL_MOD_FIELD(PT, BW_R0_ACC_CTRL, RD_WR_N, ulBmeCtrl, 0);
    /* Table ID */
    ulBmeCtrl = SAND_HAL_MOD_FIELD(PT, BW_R0_ACC_CTRL, TBL_ID, ulBmeCtrl, ulTblId);
    /* Offset */
    ulBmeCtrl = SAND_HAL_MOD_FIELD(PT, BW_R0_ACC_CTRL, OFS, ulBmeCtrl, ulOffset);

    SAND_HAL_WRITE(bmeaddr, PT, BW_R0_ACC_DAT, ulData);
    SAND_HAL_WRITE(bmeaddr, PT, BW_R0_ACC_CTRL, ulBmeCtrl);
    /* Wait for the operation to complete */
    ulTimeOut = 500;
    while (ulTimeOut)
    {
	ulBmeCtrl = SAND_HAL_READ(bmeaddr, PT, BW_R0_ACC_CTRL);
	if ( 1 == SAND_HAL_GET_FIELD(PT, BW_R0_ACC_CTRL, ACK, ulBmeCtrl))
	    break;
	thin_delay(BME32K_MIN_CLOCK_DELAY);
	ulTimeOut--;
    }

    if(0 == ulTimeOut)
    {
	return (-1);
    }

    return (0);

}

int sbBme32kBwR1Read(sbhandle bmeaddr, uint32_t ulTblId, uint32_t ulOffset, uint32_t *pulData)
{
    uint32_t ulBmeCtrl;
    uint32_t ulTimeOut;

    ulBmeCtrl = SAND_HAL_READ(bmeaddr, PT, BW_R1_ACC_CTRL);
    /*
     * Clear any pending ACK
     */
    ulBmeCtrl = SAND_HAL_MOD_FIELD(PT, BW_R1_ACC_CTRL, REQ, ulBmeCtrl, 0);
    SAND_HAL_WRITE(bmeaddr, PT, BW_R1_ACC_CTRL, ulBmeCtrl);

    ulBmeCtrl = SAND_HAL_READ(bmeaddr, PT, BW_R1_ACC_CTRL);

    ulBmeCtrl = SAND_HAL_MOD_FIELD(PT, BW_R1_ACC_CTRL, REQ, ulBmeCtrl, 1);
    /* Write OP */
    ulBmeCtrl = SAND_HAL_MOD_FIELD(PT, BW_R1_ACC_CTRL, RD_WR_N, ulBmeCtrl, 1);
    /* Table ID */
    ulBmeCtrl = SAND_HAL_MOD_FIELD(PT, BW_R1_ACC_CTRL, TBL_ID, ulBmeCtrl, ulTblId);
    /* Offset */
    ulBmeCtrl = SAND_HAL_MOD_FIELD(PT, BW_R1_ACC_CTRL, OFS, ulBmeCtrl, ulOffset);

    SAND_HAL_WRITE(bmeaddr, PT, BW_R1_ACC_CTRL, ulBmeCtrl);
    /* Wait for the operation to complete */
    ulTimeOut = 500;
    while (ulTimeOut)
    {
	ulBmeCtrl = SAND_HAL_READ(bmeaddr, PT, BW_R1_ACC_CTRL);
	if ( 1 == SAND_HAL_GET_FIELD(PT, BW_R1_ACC_CTRL, ACK, ulBmeCtrl))
	    break;
	thin_delay(BME32K_MIN_CLOCK_DELAY);
	ulTimeOut--;
    }

    if(0 == ulTimeOut)
    {
	return (-1);
    }

    *pulData = SAND_HAL_READ(bmeaddr, PT, BW_R1_ACC_DAT);

    return (0);

}

int sbBme32kBwR1Write(sbregshandle bmeaddr, uint32_t ulTblId, uint32_t ulOffset, uint32_t ulData)
{
    uint32_t ulBmeCtrl;
    uint32_t ulTimeOut;

    ulBmeCtrl = SAND_HAL_READ(bmeaddr, PT, BW_R1_ACC_CTRL);
    /*
     * Clear any pending ACK
     */
    ulBmeCtrl = SAND_HAL_MOD_FIELD(PT, BW_R1_ACC_CTRL, REQ, ulBmeCtrl, 0);
    SAND_HAL_WRITE(bmeaddr, PT, BW_R1_ACC_CTRL, ulBmeCtrl);

    ulBmeCtrl = SAND_HAL_READ(bmeaddr, PT, BW_R1_ACC_CTRL);

    ulBmeCtrl = SAND_HAL_MOD_FIELD(PT, BW_R1_ACC_CTRL, REQ, ulBmeCtrl, 1);
    /* Write OP */
    ulBmeCtrl = SAND_HAL_MOD_FIELD(PT, BW_R1_ACC_CTRL, RD_WR_N, ulBmeCtrl, 0);
    /* Table ID */
    ulBmeCtrl = SAND_HAL_MOD_FIELD(PT, BW_R1_ACC_CTRL, TBL_ID, ulBmeCtrl, ulTblId);
    /* Offset */
    ulBmeCtrl = SAND_HAL_MOD_FIELD(PT, BW_R1_ACC_CTRL, OFS, ulBmeCtrl, ulOffset);

    SAND_HAL_WRITE(bmeaddr, PT, BW_R1_ACC_DAT, ulData);
    SAND_HAL_WRITE(bmeaddr, PT, BW_R1_ACC_CTRL, ulBmeCtrl);
    /* Wait for the operation to complete */
    ulTimeOut = 500;
    while (ulTimeOut)
    {
	ulBmeCtrl = SAND_HAL_READ(bmeaddr, PT, BW_R1_ACC_CTRL);
	if ( 1 == SAND_HAL_GET_FIELD(PT, BW_R1_ACC_CTRL, ACK, ulBmeCtrl))
	    break;
	thin_delay(BME32K_MIN_CLOCK_DELAY);
	ulTimeOut--;
    }

    if(0 == ulTimeOut)
    {
	return (-1);
    }

    return (0);
}
#endif /* BCM_BME3200_SUPPORT */

#ifdef BCM_QE2000_SUPPORT
/*
 **********************************************************************************
 *   QE2K Tests
 **********************************************************************************
 */

int sbQe2000DiagsBistTestAll(sbxDiagsInfo_t *pDiagsInfo)
{
    uint32_t bist_test;
    int stat = 0;
    int unit = pDiagsInfo->unit;
    int nBistTests = SB_QE2000_BIST_TEST_LAST-1;
    if (pDiagsInfo->pattern == -1) {
#ifndef __KERNEL__
	pDiagsInfo->pattern = rand() & 0xffffffff;
	SB_LOG("Using pattern 0x%llx\n",pDiagsInfo->pattern);
#else
         
#endif
    }
    for(bist_test=0;bist_test<SB_QE2000_BIST_TEST_LAST;bist_test++) {
	pDiagsInfo->e_qe_bist_test = bist_test;
	SB_LOG("Running Test on Unit %d [%1d..%1d] %-20s .. ", unit,bist_test,
	       nBistTests,sbQe2000DiagsBistTestStr[bist_test]);
	if (sbQe2000DiagsBistTest(pDiagsInfo) != 0) {
	    stat = -1;
	    if (bist_test == SB_QE2000_EXTERNAL_BIST) {
		SB_LOG("To Re-run use BistTest=%d Verbose=1 Pattern=0x%llx\n",bist_test,pDiagsInfo->pattern);
	    } else {
		SB_LOG("To Re-run use BistTest=%d Verbose=1\n",bist_test);
	    }
	}
    }
    SOC_SBX_CFG(unit)->reset_ul = 1;
    soc_qe2000_init(unit, SOC_SBX_CFG(unit));
    return stat;
}


int sbQe2000DiagsBistTest(sbxDiagsInfo_t *pDiagsInfo)
{
  int stat=0;
  switch(pDiagsInfo->e_qe_bist_test) {
  case SB_QE2000_INTERNAL_BIST:
      stat = sbQe2kIntBist(pDiagsInfo);
    break;
  case SB_QE2000_EXTERNAL_BIST:
      stat = sbQe2kExtBist(pDiagsInfo);
    break;
  default:
    SB_ERROR("ERROR:\n%s: Invalid bist test:%d [0-%d supported]\n",__PRETTY_FUNCTION__,
	     pDiagsInfo->e_qe_bist_test,SB_QE2000_BIST_TEST_LAST-1);
    return (-1);
  }
  if (stat != 0) {
    SB_LOG("FAILED\n");
  } else {
    SB_LOG("PASSED\n");
  }
  return stat;
}

int sbQe2kSoftReset(sbhandle qeregaddr)
{
    /* Set the reset bit. */
    SAND_HAL_RMW_FIELD(qeregaddr, KA, PC_RESET, SOFT_RESET, 1);

    /* Wait some OS ticks to allow state machines to settle. */
    thin_delay(100);

    /* Clear the reset bit to start the state machines. */
    SAND_HAL_RMW_FIELD(qeregaddr, KA, PC_RESET,  SOFT_RESET, 0);
    thin_delay(100);
    return (0);
}

int sbQe2kExtBist(sbxDiagsInfo_t *pDiagsInfo)
{
    int status;
    uint32_t ulTmpStatus;
    int i;
    uint32_t ulFailedStart, ulFailedEnd, aulFailedData[8];

    ulFailedStart = 0;
    ulFailedEnd = 0xFFFFF;

    for(i = 0; i < 4; i++) {
        aulFailedData[i] = pDiagsInfo->pattern;
    }

    status = sbQe2kDDRIIExtBistTest(pDiagsInfo,
				    &ulFailedStart,
				    &ulFailedEnd,
				    aulFailedData);

    if(0 != status) {
        switch(status & 0xF0000000)
        {
            case 0x80000000:
                SB_LOG("Mode 0 Failure:\n");
                break;
            case 0x40000000:
                SB_LOG("Mode 1 Failure:\n");
                break;
            case 0x20000000:
                SB_LOG("Mode 2 failure:\n");
                break;
            default:
                break;
        }

        switch(status)
        {
            case -1:
            case -2:
            case -3:
                break;
            case -4:
                sal_printf("Mode 3 failure:\n");
                break;

            case -5:
                sal_printf("Ram auto init did not complete\n");
                break;

            case -6:
                sal_printf("BIST mode %i timed out\n", ulFailedStart);
                break;

            case -7:
                sal_printf("Auto-calibration did not complete\n");
                break;

            default:
                sal_printf("Unknown failure: %d\n", status);
                break;
        }

        if(status > -5)
        {
            ulTmpStatus = status & 0x3;
            if(ulTmpStatus & 0x1)
            {
                sal_printf("\tFailed upper address:  0x%08X\n", ulFailedStart);
                sal_printf("\tFailed upper data: 0x%08X 0x%08X 0x%08X 0x%08X\n",
                       aulFailedData[0], aulFailedData[1], aulFailedData[2], aulFailedData[3]);
            }

            if(ulTmpStatus & 0x2)
            {
                sal_printf("\tFailed lower address:  0x%08X\n", ulFailedEnd);
                sal_printf("\tFailed lower data: 0x%08X 0x%08X 0x%08X 0x%08X\n",
                       aulFailedData[4], aulFailedData[5], aulFailedData[6], aulFailedData[7]);
            }
        }
        return -1;
    }

    return 0;
}

int sbQe2kIntBist(sbxDiagsInfo_t *pDiagsInfo)
{

    int status, i;
    uint32_t ulStatus, aulFailedData[15];

    for(i = 0; i < 15; i++)
    {
        aulFailedData[i] = 0;
    }
    ulStatus = 0;

    status = sbQe2kIntBistTest(pDiagsInfo,
			       &ulStatus,
			       aulFailedData);

    if(0 != ulStatus) {
	SB_LOG( "\nFailure in internal Block(s), Bit Mask:  0x%08X\n", ulStatus);
	SB_LOG( "%-10s\t%-10s\n", "BLOCK", "Go");
	for( i = 0; i < 15; i++ )
	    {
		if( ulStatus & (1 << i))
                {
                    SB_LOG( "%-10s\t0x%08X\n", acQe2kIntBistStr[i], aulFailedData[i]);
                }
            }
        return status;
    }

    return 0;
}

int sbQe2kIntBistTest(sbxDiagsInfo_t *pDiagsInfo,
                    uint32_t *pulStatus,
                    uint32_t aulBistStat[15])
{

    int diagVerbose_b;
    sbhandle qmehdl;
    diagVerbose_b = pDiagsInfo->debug_level;
    qmehdl = pDiagsInfo->userDeviceHandle;

    if (diagVerbose_b)
    {
        SB_LOG("Running memory BIST.\n");
    }

    sbQe2kSoftReset(qmehdl);
    thin_delay(10);
    /*
     * Kick off all the BISTs in 'parallel'
     */
    sbQe2kIntBistStartAll(qmehdl);

    /*
     * Retrieve the status of all BIST tests
     */
    sbQe2kIntBistCheckAll(pDiagsInfo, pulStatus, aulBistStat);

    if(*pulStatus)
    {
	return( -1 );
    }

    return (0);
}

void sbQe2kIntBistStartAll(sbhandle tKaAddr)
{
    /*
     * Kick off the bist tests
     */
    sbQe2000BistSrStart(tKaAddr, 0);
    sbQe2000BistSrStart(tKaAddr, 1);
    sbQe2000BistStStart(tKaAddr, 0);
    sbQe2000BistStStart(tKaAddr, 1);
    sbQe2000BistPcStart(tKaAddr);
    sbQe2000BistEbStart(tKaAddr);
    sbQe2000BistTxStart(tKaAddr);
    sbQe2000BistSvStart(tKaAddr);
    sbQe2000BistQmStart(tKaAddr);
    sbQe2000BistPmStart(tKaAddr);
    sbQe2000BistEgStart(tKaAddr);
    sbQe2000BistEiStart(tKaAddr);
    sbQe2000BistEpStart(tKaAddr);
    sbQe2000BistRbStart(tKaAddr);
    sbQe2000BistQsStart(tKaAddr);

    return;
}

int sbQe2kDDRIIExtBistTest(sbxDiagsInfo_t *pDiagsInfo,
			   uint32_t *pulStartAddr,
			   uint32_t *pulEndAddr,
			   uint32_t aulDataPat[8])
{
    uint32_t ulIoCal0;
    uint32_t ulIoCal1;
    int status;
    int nDone;
    int nTimeOut;
    int diagVerbose_b;
    sbhandle regaddr;
    regaddr = pDiagsInfo->userDeviceHandle;
    diagVerbose_b = pDiagsInfo->debug_level;

    if (diagVerbose_b)
    {
        SB_LOG("%s Running memory BIST.\n",__PRETTY_FUNCTION__);
    }

    sbQe2kSoftReset(regaddr);
    thin_delay(1000);

    /*
     * Start internal bist
     */
    sbQe2000BistPmStart(regaddr);
    status = sbQe2000BistPmCheck(pDiagsInfo, &ulIoCal0, &ulIoCal1);
    if( 0 != status )
    {
        SB_LOG("%s:  PM Internal bist failed: 0x%08X 0x%08X\n",
                  __PRETTY_FUNCTION__, ulIoCal0, ulIoCal1);
        return( -7 );
    }

    /*
     * Ensure the PM is out of reset
     */
    SAND_HAL_RMW_FIELD(regaddr, KA, PC_CORE_RESET0, PM_CORE_RESET, 0);
    SAND_HAL_RMW_FIELD(regaddr, KA, PC_CORE_RESET0, DR_CORE_RESET, 0x0);
    thin_delay(100);

    /*
     * Setup IO calibration
     */
    if(diagVerbose_b)
        SB_LOG("%s: IOCalibration\n", __PRETTY_FUNCTION__);
    ulIoCal0 = SAND_HAL_SET_FIELD(KA, PM_IOCAL0, IO_GDDRII, 1) |
        SAND_HAL_SET_FIELD(KA, PM_IOCAL0, IO_GDDRI, 1) |
        SAND_HAL_SET_FIELD(KA, PM_IOCAL0, IO_TERM150_120, 1);

    SAND_HAL_WRITE(regaddr, KA, PM_IOCAL0, ulIoCal0);
    SAND_HAL_RMW_FIELD(regaddr, KA, PM_IOCAL0, IO_UPDATE_ENABLE, 0xff);
    SAND_HAL_RMW_FIELD(regaddr, KA, PM_IOCAL0, IO_START_CALIBRATION, 1);

    nTimeOut = 100;
    nDone = 0;
    while(nTimeOut)
    {
        thin_delay(100);
        ulIoCal1 = SAND_HAL_READ(regaddr, KA, PM_IOCAL1);
        if(SAND_HAL_GET_FIELD(KA, PM_IOCAL1, IO_CALIBRATION_DONE, ulIoCal1))
        {
            nDone = 1;
            break;
        }
        nTimeOut--;
    }

    if(0 == nDone)
    {
        SB_LOG("%s:  IO Calibration did not complete\n", __PRETTY_FUNCTION__);
        return( -7 );
    }

    SAND_HAL_RMW_FIELD(regaddr, KA, PM_IOCAL0, IO_UPDATE_ENABLE, 0);
    SAND_HAL_RMW_FIELD(regaddr, KA, PM_IOCAL0, IO_START_CALIBRATION, 0);
    SAND_HAL_RMW_FIELD(regaddr, KA, PM_IOCAL1, IO_CALIBRATION_DONE, 1);

    /* Clock Enable */
    SAND_HAL_WRITE(regaddr, KA, PM_DDR2_CKE_N, 0);

    ulIoCal0 = SAND_HAL_READ(regaddr, KA, PM_DDR_EMR);
    ulIoCal0 &= ~(1 << 6);
    ulIoCal0 |= (1 << 2);
    SAND_HAL_WRITE(regaddr, KA, PM_DDR_EMR, ulIoCal0);

    sbQe2000_BringUpPmManually(regaddr);
    if(diagVerbose_b)
        SB_LOG("%s: Starting DDR Training\n", __PRETTY_FUNCTION__);

    status = sbQe2kPmDDRTrain(pDiagsInfo, 0);
    if( 0 != status)
    {
        SB_LOG("%s:  DDR Training failed\n", __PRETTY_FUNCTION__);
        return( -7 );

    }


    /*
     * Execute Mode 0 BIST
     */
    status = sbQe2kDDRIIExtBistMode0(pDiagsInfo,
                                   pulStartAddr,
                                   pulEndAddr,
                                   aulDataPat);

    if( 0 != status)
    {
        return( status | 0x80000000 );
    }

    /*
     * Execute Mode 1 BIST
     */
    status = sbQe2kDDRIIExtBistMode1(pDiagsInfo,
                                   pulStartAddr,
                                   pulEndAddr,
                                   aulDataPat);
    if( 0 != status)
    {
        return( status | 0x40000000);
    }

    /*
     * Execute Mode 2 BIST
     */
    status = sbQe2kDDRIIExtBistMode2(pDiagsInfo,
                                   pulStartAddr,
                                   pulEndAddr,
                                   aulDataPat);
    if( 0 != status)
    {
        return( status | 0x20000000);
    }


    return (0);
}
/*
 * Mode 0:  Data Pattern Test
 */
int sbQe2kDDRIIExtBistMode0(sbxDiagsInfo_t *pDiagsInfo,
                          uint32_t *pulStartAddr,
                          uint32_t *pulEndAddr,
                          uint32_t aulDataPat[8])
{
    uint32_t ulTestConfig;
    int nTimeOut;
    int nHadAnError;
    int nDone;
    int i;
    sbhandle regaddr;
    int diagVerbose_b;
    regaddr = pDiagsInfo->userDeviceHandle;
    diagVerbose_b = pDiagsInfo->debug_level;

    /*
     * Start Mode 0 Testing
     */
    if(diagVerbose_b)
        SB_LOG("%s:  Starting BIST Mode 0 Test\n", __PRETTY_FUNCTION__);
    ulTestConfig = SAND_HAL_SET_FIELD(KA, PM_DDR_TEST, RAM_DONE, 1) |
        SAND_HAL_SET_FIELD(KA, PM_DDR_TEST, RAM_TEST_FAIL_LOWER, 1) |
        SAND_HAL_SET_FIELD(KA, PM_DDR_TEST, RAM_TEST_FAIL_UPPER, 1);
    SAND_HAL_WRITE(regaddr, KA, PM_DDR_TEST, ulTestConfig);
    thin_delay(10);

    SAND_HAL_WRITE(regaddr, KA, PM_DDR_START, *pulStartAddr);
    SAND_HAL_WRITE(regaddr, KA, PM_DDR_END,   *pulEndAddr);

    /*
     * Write out the data to be used as the test pattern
     */
    for(i = 0; i < 4; i++)
    {
        SAND_HAL_WRITE_INDEX(regaddr, KA, PM_TEST_DATA0, i, aulDataPat[0]);
        SAND_HAL_WRITE_INDEX(regaddr, KA, PM_TEST_ALT_DATA0, i, ~aulDataPat[0]);
    }

    ulTestConfig = SAND_HAL_SET_FIELD(KA, PM_DDR_TEST, RAM_TEST, 1);

    SAND_HAL_WRITE(regaddr, KA, PM_DDR_TEST, ulTestConfig);

    nTimeOut = 10;
    nDone = 0;
    while(nTimeOut)
    {
	thin_delay(1*1E9);
        ulTestConfig = SAND_HAL_READ(regaddr, KA, PM_DDR_TEST);
        nDone = SAND_HAL_GET_FIELD(KA, PM_DDR_TEST, RAM_DONE, ulTestConfig);
        if(1 == nDone)
        {
            break;
        }
        nTimeOut--;
    }

    if(1 != nDone)
    {
        SB_LOG("%s:  Timed out waiting for Mode 0 BIST test to complete\n", __PRETTY_FUNCTION__);
        *pulStartAddr = 0;
        *pulEndAddr = 0;
        for(i = 0; i < 8; i++)
        {
            aulDataPat[i] = 0;
        }
        return( -6 );
    }

    nHadAnError = sbQe2kDDRIIExtBistErrorsGet(regaddr,
                                            ulTestConfig,
                                            pulStartAddr,
                                            pulEndAddr,
                                            aulDataPat);

    if(nHadAnError)
    {
        return( nHadAnError );
    }

    if(diagVerbose_b)
        SB_LOG("%s:  PASSED\n", __PRETTY_FUNCTION__);
    return( 0 );

}

/*
 * Mode 1:  Walking 1s & 0s
 */
int sbQe2kDDRIIExtBistMode1(sbxDiagsInfo_t *pDiagsInfo,
			    uint32_t *pulStartAddr,
			    uint32_t *pulEndAddr,
			    uint32_t aulDataPat[8])
{
    uint32_t ulTestConfig;
    int nTimeOut;
    int nHadAnError;
    int nDone;
    int i;
    sbhandle regaddr;
    int diagVerbose_b;
    regaddr = pDiagsInfo->userDeviceHandle;
    diagVerbose_b = pDiagsInfo->debug_level;

    /*
     * Start Mode 0 Testing
     */
    if(diagVerbose_b)
        SB_LOG("%s:  Starting BIST Mode 1 Test\n", __PRETTY_FUNCTION__);
    ulTestConfig = SAND_HAL_SET_FIELD(KA, PM_DDR_TEST, RAM_DONE, 1) |
        SAND_HAL_SET_FIELD(KA, PM_DDR_TEST, RAM_TEST_FAIL_LOWER, 1) |
        SAND_HAL_SET_FIELD(KA, PM_DDR_TEST, RAM_TEST_FAIL_UPPER, 1);
    SAND_HAL_WRITE(regaddr, KA, PM_DDR_TEST, ulTestConfig);
    thin_delay(10);

    SAND_HAL_WRITE(regaddr, KA, PM_DDR_START, *pulStartAddr);
    SAND_HAL_WRITE(regaddr, KA, PM_DDR_END, *pulEndAddr);

    ulTestConfig = SAND_HAL_SET_FIELD(KA, PM_DDR_TEST, RAM_TEST, 1) |
        SAND_HAL_SET_FIELD(KA, PM_DDR_TEST, MODE, 1);

    SAND_HAL_WRITE(regaddr, KA, PM_DDR_TEST, ulTestConfig);

    nTimeOut = 100;
    nDone = 0;
    while(nTimeOut)
    {
	thin_delay(1000);
        ulTestConfig = SAND_HAL_READ(regaddr, KA, PM_DDR_TEST);
        nDone = SAND_HAL_GET_FIELD(KA, PM_DDR_TEST, RAM_DONE, ulTestConfig);
        if(1 == nDone)
        {
            break;
        }
        nTimeOut--;
    }

    if(1 != nDone)
    {
        SB_LOG( "%s:  Timed out waiting for Mode 1 BIST test to complete\n", __PRETTY_FUNCTION__);
        *pulStartAddr = 1;
        *pulEndAddr = 0;
        for(i = 0; i < 8; i++)
        {
            aulDataPat[i] = 0;
        }
        return( -6 );
    }

    nHadAnError = sbQe2kDDRIIExtBistErrorsGet(regaddr,
                                            ulTestConfig,
                                            pulStartAddr,
                                            pulEndAddr,
                                            aulDataPat);

    if(nHadAnError)
    {
        return( nHadAnError );
    }

    if(diagVerbose_b)
        SB_LOG( "%s:  PASSED\n", __PRETTY_FUNCTION__);
    return( 0 );

}

/*
 * Mode 2:  Data Pattern Test, full speed
 */
int sbQe2kDDRIIExtBistMode2(sbxDiagsInfo_t *pDiagsInfo,
			    uint32_t *pulStartAddr,
			    uint32_t *pulEndAddr,
			    uint32_t aulDataPat[8])
{
    uint32_t ulTestConfig;
    int nTimeOut;
    int nHadAnError;
    int nDone;
    int i;
    sbhandle regaddr;
    int diagVerbose_b;
    regaddr = pDiagsInfo->userDeviceHandle;
    diagVerbose_b = pDiagsInfo->debug_level;

    /*
     * Start Mode 0 Testing
     */
    if(diagVerbose_b)
        SB_LOG("%s:  Starting BIST Mode 2 Test\n", __PRETTY_FUNCTION__);
    ulTestConfig = SAND_HAL_SET_FIELD(KA, PM_DDR_TEST, RAM_DONE, 1) |
        SAND_HAL_SET_FIELD(KA, PM_DDR_TEST, RAM_TEST_FAIL_LOWER, 1) |
        SAND_HAL_SET_FIELD(KA, PM_DDR_TEST, RAM_TEST_FAIL_UPPER, 1);
    SAND_HAL_WRITE(regaddr, KA, PM_DDR_TEST, ulTestConfig);
    thin_delay(10);

    SAND_HAL_WRITE(regaddr, KA, PM_DDR_START, *pulStartAddr);
    SAND_HAL_WRITE(regaddr, KA, PM_DDR_END, *pulEndAddr);

    /*
     * Write out the data to be used as the test pattern
     */
    for(i = 0; i < 4; i++)
    {
        SAND_HAL_WRITE_INDEX(regaddr, KA, PM_TEST_DATA0, i, aulDataPat[0]);
    }

    ulTestConfig = SAND_HAL_SET_FIELD(KA, PM_DDR_TEST, RAM_TEST, 1) |
        SAND_HAL_SET_FIELD(KA, PM_DDR_TEST, MODE, 2);

    SAND_HAL_WRITE(regaddr, KA, PM_DDR_TEST, ulTestConfig);

    nTimeOut = 10;
    nDone = 0;
    while(nTimeOut)
    {
	thin_delay(1*1E9);
        ulTestConfig = SAND_HAL_READ(regaddr, KA, PM_DDR_TEST);
        nDone = SAND_HAL_GET_FIELD(KA, PM_DDR_TEST, RAM_DONE, ulTestConfig);
        if(1 == nDone)
        {
            break;
        }
        nTimeOut--;
    }

    if(1 != nDone)
    {
        SB_LOG( "%s:  Timed out waiting for Mode 2 BIST test to complete\n", __PRETTY_FUNCTION__);
        *pulStartAddr = 2;
        *pulEndAddr = 0;
        for(i = 0; i < 8; i++)
        {
            aulDataPat[i] = 0;
        }
        return( -6 );
    }

    nHadAnError = sbQe2kDDRIIExtBistErrorsGet(regaddr,
					      ulTestConfig,
					      pulStartAddr,
					      pulEndAddr,
					      aulDataPat);

    if(nHadAnError)
    {
        return( nHadAnError );
    }

    if(diagVerbose_b)
        SB_LOG("%s:  PASSED\n", __PRETTY_FUNCTION__);
    return( 0 );

}

int sbQe2kDDRIIExtBistErrorsGet(sbhandle regaddr,
                              uint32_t ulTestResults,
                              uint32_t *pulUpperAddr,
                              uint32_t *pulLowerAddr,
                              uint32_t aulFailedData[8])
{
    int i;
    int nHadAnError;
    uint32_t ulTmp;
    uint32_t ulMemMode;

    ulTmp = SAND_HAL_READ(regaddr, KA, PM_CONFIG0);
    ulMemMode = SAND_HAL_GET_FIELD(KA, PM_CONFIG0, MEM_MODE, ulTmp);


    /*
     * We've had an error, get the offending addresses
     * and the data
     */
    nHadAnError = 0;
    if(1 == SAND_HAL_GET_FIELD(KA, PM_DDR_TEST, RAM_TEST_FAIL_UPPER, ulTestResults))
    {
        *pulUpperAddr = SAND_HAL_READ(regaddr, KA, PM_FAILED_ADDR_UPPER);
        /*
         * Get the upper failed data
         */
        for(i = 0; i < 4; i++)
        {
            aulFailedData[i] = SAND_HAL_READ_INDEX(regaddr, KA, PM_FAILED_DATA_UPPER0, i);
        }
        nHadAnError |= 1;

        SB_LOG("Failed upper addr: 0x%08X\n", *pulUpperAddr);
        if(0 == ulMemMode)
        {
            SB_LOG( "(128bit mode)Physical Address: 0x%08X Bank: 0x%02X Channel: 0x%03X\n",
                      (*pulUpperAddr >> 5),
                      (*pulUpperAddr >> 3) & 0x3,
                      (*pulUpperAddr >> 0) & 0x7);
        }
        else
        {
            ulTmp = (((*pulUpperAddr) & 0x2) << 1) | 0x2 | (*pulUpperAddr & 0x1);
            SB_LOG( "(64bit mode)Physical Address: 0x%08X Bank: 0x%02X Channel: 0x%03X\n",
                      (*pulUpperAddr >> 5),
                      (*pulUpperAddr >> 3) & 0x3,
                      ulTmp);
        }
    }
    else
    {
        for(i = 0; i < 4; i++)
        {
            aulFailedData[i] = 0;
        }
    }

    if(1 == SAND_HAL_GET_FIELD(KA, PM_DDR_TEST, RAM_TEST_FAIL_LOWER, ulTestResults))
    {
        *pulLowerAddr = SAND_HAL_READ(regaddr, KA, PM_FAILED_ADDR_LOWER);
        /*
         * Get the lower failed data
         */
        for(i = 4; i < 8; i++)
        {
            aulFailedData[i] = SAND_HAL_READ_INDEX(regaddr, KA, PM_FAILED_DATA_LOWER0, i);
        }
        nHadAnError |= 2;
        SB_LOG( "Failed lower addr: 0x%08X\n", *pulLowerAddr);
        if(0 == ulMemMode)
        {
            SB_LOG( "(128bit mode)Physical Address: 0x%08X Bank: 0x%02X Channel: 0x%03X\n",
                      (*pulLowerAddr >> 5),
                      (*pulLowerAddr >> 3) & 0x3,
                      (*pulLowerAddr >> 0) & 0x7);
        }
        else
        {
            ulTmp = (((*pulLowerAddr) & 0x2) << 1) | 0x2 | (*pulLowerAddr & 0x1);
            SB_LOG( "(64bit mode)Physical Address: 0x%08X Bank: 0x%02X Channel: 0x%03X\n",
                      (*pulLowerAddr >> 5),
                      (*pulLowerAddr >> 3) & 0x3,
                      ulTmp);
        }
    }
    else
    {
        for(i = 4; i < 8; i++)
        {
            aulFailedData[i] = 0;
        }
    }

    return( nHadAnError );
}

static uint8_t uResultsArr[256][16]; /* index [delay][byte position] */

uint32_t sbQe2kPmDDRTrain(sbxDiagsInfo_t *pDiagsInfo,
			  int nHalfBus)
{
    /*
            - DDR trim delay training
            - Each dram is 16 bit wide, it correspond to a channel
            - Each channel has an upper byte and a lower byte
            - Each byte has its own DLL to trim DQS delay
            - For indirect memory access, channel # = indirect address[2:0]
            - Each DR group is connected to two channels
            - Each DR group therefore controls 4 byte lanes

              addr channel dram dr_group
            - --------------------------
            -  0   0       0    0
            -  1   1       2    1
            -  2   2       4    2
            -  3   3       6    3
            -  4   4       1    0
            -  5   5       3    1
            -  6   6       5    2
            -  7   7       7    3

            - Pseudo code for DDR training

            - (1) Initialize dram for addr[7:0], i.e. for channels[7:0]
            - (2) for each channel
            -       for (delay=0,1,...)
            -         read back the data
            -         for the two byte lanes at that channel
            -         check for data mismatch
            - (3) post process the results above
            -       for each byte lane
            -         find longest span of delay values where there is no error
            -         the mid-point of this span is the optimal delay for this byte lane
    */

    const uint32_t uChannelToDrGroup[8] = {0,1,2,3,0,1,2,3};
    const uint32_t uHalfBusChannelToDrGroup[4] = {2,3,2,3};
    const uint32_t uChannelToReg[8] = {0,0,0,0,2,2,2,2};
    const uint32_t uHalfBusChannelToReg[4] = {0,0,2,2};
    const uint32_t uUpperByteMask = 0xff00ff00;
    const uint32_t uLowerByteMask = ~uUpperByteMask;
    uint32_t uRdData[4];
    uint32_t uWrData[4];
    uint32_t uBytePos;
    uint32_t uDly, uMaxLanes;
    uint32_t uByte;
    uint32_t uW;
    uint32_t uWrAddr;
    uint32_t uChannel;
    uint32_t uByteLane;
    uint32_t uMaxAddr = 8;
    uint32_t uMaxDelay = 256;
    uint32_t uDrGroup;
    uint32_t uWord;
    uint32_t uReg;
    uint32_t uSetUpDrDllStatus;
    sbhandle tKaAddr;
    int diagVerbose_b;
    tKaAddr = pDiagsInfo->userDeviceHandle;
    diagVerbose_b = pDiagsInfo->debug_level;

    if (nHalfBus){
        uMaxLanes = 8;
        uMaxAddr = 4;
    }
    else {
        uMaxLanes = 16;
        uMaxAddr = 8;
    }

    /* this matrix keeps track of error count at a byte position for a
     *  particular trimming delay. Initialize to 0 count.
     */
    for (uDly=0; uDly<256; uDly++) {
	for (uByte=0; uByte<16; uByte++) {
	    uResultsArr[uDly][uByte] = 0;
	}
    }

    /* use a data pattern of 0x0000ffff, at the memory interface, this
     * pattern will be translated to 0x55555555 for a maximum bit transitions.
     */
    for (uW=0; uW<4; uW++) {
	uWrData[uW] = 0x0000ffff;
    }

    /* write initial data to memory once */
    for (uWrAddr=0; uWrAddr<uMaxAddr; uWrAddr++) {

	if (sbQe2kPmMemWrite(tKaAddr,
				   uWrAddr,
				   uWrData[3],
				   uWrData[2],
				   uWrData[1],
				   uWrData[0]) != 0) {

	    return -3;
	}

    }

    for (uChannel=0; uChannel<uMaxAddr; uChannel++) {

        if (nHalfBus)
        {
	    uDrGroup = uHalfBusChannelToDrGroup[uChannel];
	    uReg = uHalfBusChannelToReg[uChannel];
        }
        else
        {
	    uDrGroup = uChannelToDrGroup[uChannel];
	    uReg = uChannelToReg[uChannel];
        }

       	for (uDly = 0; uDly < (uint32_t)uMaxDelay; uDly++) {
            for(uByte = 0; uByte < 2; uByte++) {

                uSetUpDrDllStatus=sbQe2kPmDllWrite(tKaAddr,
						 uDrGroup,
						 uReg + uByte,
						 uDly);
                if (uSetUpDrDllStatus != 0)
	             return uSetUpDrDllStatus;

	         /* Read back data and check for error */
                if (sbQe2kPmMemRead(tKaAddr,
                                    uChannel,
                                    &uRdData[3],
                                    &uRdData[2],
                                    &uRdData[1],
                                    &uRdData[0]) != 0) {

	             return -2;
                }
                if(diagVerbose_b)
                    SB_LOG("Channel 0x%08X:  0x%08X 0x%08X 0x%08X 0x%08X\n",
                              uChannel,
                              uRdData[3],
                              uRdData[2],
                              uRdData[1],
                              uRdData[0]);

                for (uWord=0; uWord<4; uWord++) {
	            if((uRdData[uWord] & uUpperByteMask) != (uWrData[uWord] & uUpperByteMask)) {
	                uBytePos = uChannel*2;
	                uResultsArr[uDly][uBytePos]++;
	            }
	            if ((uRdData[uWord] & uLowerByteMask) != (uWrData[uWord] & uLowerByteMask)) {
	                uBytePos = uChannel*2 + 1;
	                uResultsArr[uDly][uBytePos]++;
	            }
                }
            }
	} /* uDly */
    }

    /* post process the result matrix
       for each byte lane, find the longest span of delays that results no error.
       The mid-point of this span is the optimum trim value to use. */
    for (uByteLane = 0; uByteLane < uMaxLanes; uByteLane++) {
	uint32_t uPos = 0;
	uint32_t uBestPos = 0;
	uint32_t uBestCount = 0;

	  uPos = 0;
	  while (uPos<256) {
	    /* uPos: 0,    1,   2,   ..126, 127, 128,.., 255
	       uDly: 128,  129, 130, ..254, 255, 0,      127
	       =-128, -127          -2   -1  0       127 */
	    if (uPos<128) {
	      uDly = uPos+128;
	    } else {
	      uDly  = (uPos - 128) & 0xff;
	    }
	    if (uResultsArr[uDly][uByteLane] == 0) {
	      /* find out the run length */
	      uint32_t uStartPos = uPos;
	      uint32_t uCount = 0;
	      uint32_t j;
	      for (j=uPos; j<256; j++) {
		uint32_t uDlyTmp;
		uCount++;
		uPos++;
		uDlyTmp = (uPos - 128) & 0xff;
		if (uResultsArr[uDlyTmp][uByteLane] != 0) break;
	      }

	      /* store the best run length + starting position */
	      if (uCount>uBestCount) {
		uBestCount=uCount;
		uBestPos=uStartPos;
	      }

	    } else {
	      uPos++;
	    }
	  }

	  if (uBestCount==0) { /* were no good bit positions for this byte lane */
              SB_LOG("ERROR: No good bit positions for this byte lane DDR training failed\n");
	      sbQe2kPmDumpChannelStatus(uResultsArr); /* show what channel(s) failed training */
              return -1;
	  } else { /* there was at least 1 good bit position */
              uint32_t uBestDly;

              uBestDly      = (uBestPos + (uBestPos + uBestCount))/2; /* mid-point of span */
              uBestDly      = (uBestDly - 128) & 0xff;                /* translated to delay number */
           /*
            * now update registers with best trim values
            */
            uChannel = uByteLane >> 1;
            if (nHalfBus)
            {
                uDrGroup = uHalfBusChannelToDrGroup[uChannel];
                uReg = uHalfBusChannelToReg[uChannel];
            }
            else
            {
                uDrGroup = uChannelToDrGroup[uChannel];
                uReg = uChannelToReg[uChannel];
            }
            if ((uByteLane & 0x1)==0) uReg |= 1;
	    uSetUpDrDllStatus=sbQe2kPmDllWrite(tKaAddr, uDrGroup,uReg,uBestDly);
	    if (uSetUpDrDllStatus != 0)
	      return uSetUpDrDllStatus;
        }
    }

    return 0;

}


int sbQe2kPmMemWrite(sbhandle tKaAddr,
		     uint32_t ulAddr,
		     uint32_t ulData3,
		     uint32_t ulData2,
		     uint32_t ulData1,
		     uint32_t ulData0)
{
    uint32_t ulCtrl;
    int nTimeOut;
    int nAck;

    /*
     * Clear any outstanding requests and acks
     */
    ulCtrl = SAND_HAL_SET_FIELD(KA, PM_MEM_ACC_CTRL, ACK, 1);
    SAND_HAL_WRITE(tKaAddr, KA, PM_MEM_ACC_CTRL, ulCtrl);

    /*
     * Write out the data
     */
    SAND_HAL_WRITE(tKaAddr, KA, PM_MEM_ACC_DATA0, ulData0);
    SAND_HAL_WRITE(tKaAddr, KA, PM_MEM_ACC_DATA1, ulData1);
    SAND_HAL_WRITE(tKaAddr, KA, PM_MEM_ACC_DATA2, ulData2);
    SAND_HAL_WRITE(tKaAddr, KA, PM_MEM_ACC_DATA3, ulData3);

    /*
     * Setup for the write
     */
    ulCtrl = SAND_HAL_SET_FIELD(KA, PM_MEM_ACC_CTRL, REQ, 1) |
        SAND_HAL_SET_FIELD(KA, PM_MEM_ACC_CTRL, RD_WR_N, 0) |
        SAND_HAL_SET_FIELD(KA, PM_MEM_ACC_CTRL, ADDR, ulAddr);

    SAND_HAL_WRITE(tKaAddr, KA, PM_MEM_ACC_CTRL, ulCtrl);

    /*
     * Wait for an ACK or timeout
     */
    nTimeOut = 1000;
    nAck = 0;

    while(nTimeOut--)
    {
        ulCtrl = SAND_HAL_READ(tKaAddr, KA, PM_MEM_ACC_CTRL);
        ulCtrl = SAND_HAL_GET_FIELD(KA, PM_MEM_ACC_CTRL, ACK, ulCtrl);
        if(1 == ulCtrl)
        {
            nAck = 1;
            break;
        }
        thin_delay(100);
    }

    if(0 == nAck)
    {
        return( -1 );
    }

    return( 0 );
}

int sbQe2kPmMemRead(sbhandle tKaAddr,
		    uint32_t ulAddr,
		    uint32_t *pulData3,
		    uint32_t *pulData2,
		    uint32_t *pulData1,
		    uint32_t *pulData0)
{
    uint32_t ulCtrl;
    int nTimeOut;
    int nAck;

    /*
     * Clear any outstanding requests and acks
     */
    ulCtrl = SAND_HAL_SET_FIELD(KA, PM_MEM_ACC_CTRL, ACK, 1);
    SAND_HAL_WRITE(tKaAddr, KA, PM_MEM_ACC_CTRL, ulCtrl);

    /*
     * Setup for the read
     */
    ulCtrl = SAND_HAL_SET_FIELD(KA, PM_MEM_ACC_CTRL, REQ, 1) |
        SAND_HAL_SET_FIELD(KA, PM_MEM_ACC_CTRL, RD_WR_N, 1) |
        SAND_HAL_SET_FIELD(KA, PM_MEM_ACC_CTRL, ADDR, ulAddr);

    SAND_HAL_WRITE(tKaAddr, KA, PM_MEM_ACC_CTRL, ulCtrl);

    nTimeOut = 1000;
    nAck = 0;
    while(nTimeOut--)
    {
        ulCtrl = SAND_HAL_READ(tKaAddr, KA, PM_MEM_ACC_CTRL);
        ulCtrl = SAND_HAL_GET_FIELD(KA, PM_MEM_ACC_CTRL, ACK, ulCtrl);

        if(1 == ulCtrl)
        {
            nAck = 1;
            break;
        }
	thin_delay(100);
    }

    if(0 == nAck)
    {
        return( -1 );
    }

    *pulData0 = SAND_HAL_READ(tKaAddr, KA, PM_MEM_ACC_DATA0);
    *pulData1 = SAND_HAL_READ(tKaAddr, KA, PM_MEM_ACC_DATA1);
    *pulData2 = SAND_HAL_READ(tKaAddr, KA, PM_MEM_ACC_DATA2);
    *pulData3 = SAND_HAL_READ(tKaAddr, KA, PM_MEM_ACC_DATA3);

    return( 0 );

}

int sbQe2kPmDllWrite(sbhandle tKaAddr,
		     uint32_t ulTableId,
		     uint32_t ulAddr,
		     uint32_t ulData)
{
    uint32_t ulCtrl;
    int nTimeOut;
    int nAck;

    /*
     * Clear any outstanding requests and acks
     */
    ulCtrl = SAND_HAL_READ(tKaAddr, KA, PM_DLL_ACC_CTRL);
    ulCtrl = SAND_HAL_MOD_FIELD(KA, PM_DLL_ACC_CTRL, REQ, ulCtrl, 0);
    ulCtrl = SAND_HAL_MOD_FIELD(KA, PM_DLL_ACC_CTRL, ACK, ulCtrl, 1);
    SAND_HAL_WRITE(tKaAddr, KA, PM_DLL_ACC_CTRL, ulCtrl);

    /*
     * Write out the data
     */
    SAND_HAL_WRITE(tKaAddr, KA, PM_DLL_ACC_DATA, ulData);

    /*
     * Setup for the write
     */
    ulCtrl = SAND_HAL_SET_FIELD(KA, PM_DLL_ACC_CTRL, REQ, 1) |
        SAND_HAL_SET_FIELD(KA, PM_DLL_ACC_CTRL, RD_WR_N, 0) |
        SAND_HAL_SET_FIELD(KA, PM_DLL_ACC_CTRL, DR_SEL, ulTableId) |
        SAND_HAL_SET_FIELD(KA, PM_DLL_ACC_CTRL, ADDR, ulAddr + 0x80);

    SAND_HAL_WRITE(tKaAddr, KA, PM_DLL_ACC_CTRL, ulCtrl);

    /*
     * Wait for an ACK or timeout
     */
    nTimeOut = 1000;
    nAck = 0;

    while(nTimeOut--)
    {
        ulCtrl = SAND_HAL_READ(tKaAddr, KA, PM_DLL_ACC_CTRL);
        ulCtrl = SAND_HAL_GET_FIELD(KA, PM_DLL_ACC_CTRL, ACK, ulCtrl);
        if(1 == ulCtrl)
        {
            nAck = 1;
            break;
        }
	thin_delay(100);
    }

    if(0 == nAck)
    {
        return( -1 );
    }

    return( 0 );
}

void sbQe2000_BringUpPmManually(sbhandle userDeviceHandle){
  uint32_t nNopCmd = 0;
  uint32_t nPreCmd = 1;
  uint32_t nMrsCmd = 2;
  uint32_t nRefCmd = 3;
  uint32_t nMrDefault      = 0x643;
  uint32_t nMr_dllReset    = 0x100;
  uint32_t nEMr_ocdDefault = 0x380;
  uint32_t nEMrDefault     = 0x41c;
  int32_t nRefresh=0;
  uint32_t nData;

  /* save it for restore later */
  uint32_t nMrData  = SAND_HAL_READ(userDeviceHandle, KA, PM_DDR_MR);

  uint32_t nManInit = 0;
  nManInit = SAND_HAL_MOD_FIELD(KA, PM_MAN_INIT, INIT_ACTIVE, nManInit, 1);

  nManInit = sbQe2000_PmManInitCmd(userDeviceHandle, nNopCmd, nManInit); /* Nop */
  nManInit = sbQe2000_PmManInitCmd(userDeviceHandle, nPreCmd, nManInit); /* Precharge */

  nData = 0;
  nData = SAND_HAL_MOD_FIELD(KA, PM_DDR_MR, MR_RUN_BA, nData, 2);
  SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_MR, nData);
  nManInit = sbQe2000_PmManInitCmd(userDeviceHandle, nMrsCmd, nManInit); /*mode register ba=2, add=0000 */

  nData = 0;
  nData = SAND_HAL_MOD_FIELD(KA, PM_DDR_MR, MR_RUN_BA, nData, 3);
  SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_MR, nData);
  nManInit = sbQe2000_PmManInitCmd(userDeviceHandle, nMrsCmd, nManInit); /*mode register ba=3, add=0000 */

  nData = 0;
  nData = SAND_HAL_MOD_FIELD(KA, PM_DDR_MR, MR_RUN_BA, nData, 1);
  nData = SAND_HAL_MOD_FIELD(KA, PM_DDR_MR, MR_RUN_ADDR, nData, nEMrDefault);
  SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_MR, nData);
  nManInit = sbQe2000_PmManInitCmd(userDeviceHandle, nMrsCmd, nManInit); /* enable DLL */

  nData = 0;
  nData = SAND_HAL_MOD_FIELD(KA, PM_DDR_MR, MR_RUN_BA, nData, 0);
  nData = SAND_HAL_MOD_FIELD(KA, PM_DDR_MR, MR_RUN_ADDR, nData, nMrDefault | nMr_dllReset);
  SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_MR, nData);
  nManInit = sbQe2000_PmManInitCmd(userDeviceHandle, nMrsCmd, nManInit); /* DLL reset */

  nManInit = sbQe2000_PmManInitCmd(userDeviceHandle, nPreCmd, nManInit); /* Precharge */

  for (nRefresh=0; nRefresh<8; nRefresh++) {
   /* assuming 300ns for one pair of write read cycles, */
    /* each refresh consumes at least 75 clocks */
    /* eight refresh will consumes 600 clocks */
    /* more than enough to allow the DLL to lock (200 clocks) */
    nManInit = sbQe2000_PmManInitCmd(userDeviceHandle, nRefCmd, nManInit); /* Refresh */
  }

  nData = 0;
  nData = SAND_HAL_MOD_FIELD(KA, PM_DDR_MR, MR_RUN_BA, nData, 0);
  nData = SAND_HAL_MOD_FIELD(KA, PM_DDR_MR, MR_RUN_ADDR, nData, nMrDefault);
  SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_MR, nData);
  nManInit = sbQe2000_PmManInitCmd(userDeviceHandle, nMrsCmd, nManInit); /* Initialize without DLL reset */

  nData = 0;
  nData = SAND_HAL_MOD_FIELD(KA, PM_DDR_MR, MR_RUN_BA, nData, 1);
  nData = SAND_HAL_MOD_FIELD(KA, PM_DDR_MR, MR_RUN_ADDR, nData, nEMrDefault | nEMr_ocdDefault);
  SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_MR, nData);
  nManInit = sbQe2000_PmManInitCmd(userDeviceHandle, nMrsCmd, nManInit); /* enable OCD default */

  nData = 0;
  nData = SAND_HAL_MOD_FIELD(KA, PM_DDR_MR, MR_RUN_BA, nData, 1);
  nData = SAND_HAL_MOD_FIELD(KA, PM_DDR_MR, MR_RUN_ADDR, nData, nEMrDefault);
  SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_MR, nData);
  nManInit = sbQe2000_PmManInitCmd(userDeviceHandle, nMrsCmd, nManInit); /* exit OCD setup */

  /* exit manual init mode */
  nManInit = 0;
  nManInit = SAND_HAL_MOD_FIELD(KA, PM_MAN_INIT, INIT_ACTIVE, nManInit, 0);
  nManInit = SAND_HAL_MOD_FIELD(KA, PM_MAN_INIT, INIT_COMMAND, nManInit, 0);
  nManInit = SAND_HAL_MOD_FIELD(KA, PM_MAN_INIT, INIT_COMMAND_REQ, nManInit, 0);
  nManInit = SAND_HAL_MOD_FIELD(KA, PM_MAN_INIT, INIT_COMMAND_DONE, nManInit, 1);
  SAND_HAL_WRITE (userDeviceHandle, KA, PM_MAN_INIT, nManInit);

  /* restore PM_DDR_MR */
  SAND_HAL_WRITE(userDeviceHandle, KA, PM_DDR_MR, nMrData);
}

uint32_t sbQe2000_PmManInitCmd(sbhandle userDeviceHandle,
			       uint32_t nCmd,
			       uint32_t nManInit) {
    uint32_t nData;
    nData = SAND_HAL_MOD_FIELD(KA, PM_MAN_INIT, INIT_COMMAND, nManInit, nCmd);
    nData = SAND_HAL_MOD_FIELD(KA, PM_MAN_INIT, INIT_COMMAND_REQ, nData, 1);
    nData = SAND_HAL_MOD_FIELD(KA, PM_MAN_INIT, INIT_COMMAND_DONE, nData, 1);
    SAND_HAL_WRITE (userDeviceHandle, KA, PM_MAN_INIT, nData);

    /* verify the write happened */
    nData = SAND_HAL_READ(userDeviceHandle, KA, PM_MAN_INIT);

  if (SAND_HAL_GET_FIELD(KA, PM_MAN_INIT, INIT_COMMAND_DONE, nData) != 1) {
      SB_LOG("ERROR: PM manual init Ack not SB_ASSERTed, PM_MAN_INIT=0x%x\n",nData);
  }

  return nData;
}

void sbQe2kIntBistCheckAll(sbxDiagsInfo_t *pDiagsInfo,
			   uint32_t *pulStatus,
			   uint32_t aulBistStatus[15])
{
    uint32_t ulStatus0;
    uint32_t ulStatus1;
    int status;
    int nTestIdx;

    /*
     * Check the status of each test
     */
    nTestIdx = 0;
    status = sbQe2000BistSrCheck(pDiagsInfo, 0, &ulStatus0, &ulStatus1);
    if(0 != status)
    {
        *pulStatus |= 1 << nTestIdx;
        aulBistStatus[nTestIdx] = ulStatus1;
    }
    nTestIdx++;

    status = sbQe2000BistSrCheck(pDiagsInfo, 1, &ulStatus0, &ulStatus1);
    if(0 != status)
    {
        *pulStatus |= 1 << nTestIdx;
        aulBistStatus[nTestIdx] = ulStatus1;
    }
    nTestIdx++;

    status = sbQe2000BistStCheck(pDiagsInfo, 0, &ulStatus0, &ulStatus1);
    if(0 != status)
    {
        *pulStatus |= 1 << nTestIdx;
        aulBistStatus[nTestIdx] = ulStatus1;
    }
    nTestIdx++;

    status = sbQe2000BistStCheck(pDiagsInfo, 1, &ulStatus0, &ulStatus1);
    if(0 != status)
    {
        *pulStatus |= 1 << nTestIdx;
        aulBistStatus[nTestIdx] = ulStatus1;
    }
    nTestIdx++;

    status = sbQe2000BistPcCheck(pDiagsInfo, &ulStatus0, &ulStatus1);
    if(0 != status)
    {
        *pulStatus |= 1 << nTestIdx;
        aulBistStatus[nTestIdx] = ulStatus1;
    }
    nTestIdx++;

    status = sbQe2000BistEbCheck(pDiagsInfo, &ulStatus0, &ulStatus1);
    if(0 != status)
    {
        *pulStatus |= 1 << nTestIdx;
        aulBistStatus[nTestIdx] = ulStatus1;
    }
    nTestIdx++;

    status = sbQe2000BistTxCheck(pDiagsInfo, &ulStatus0, &ulStatus1);
    if(0 != status)
    {
        *pulStatus |= 1 << nTestIdx;
        aulBistStatus[nTestIdx] = ulStatus1;
    }
    nTestIdx++;

    status = sbQe2000BistSvCheck(pDiagsInfo, &ulStatus0, &ulStatus1);
    if(0 != status)
    {
        *pulStatus |= 1 << nTestIdx;
        aulBistStatus[nTestIdx] = ulStatus1;
    }
    nTestIdx++;

    status = sbQe2000BistQmCheck(pDiagsInfo, &ulStatus0, &ulStatus1);
    if(0 != status)
    {
        *pulStatus |= 1 << nTestIdx;
        aulBistStatus[nTestIdx] = ulStatus1;
    }
    nTestIdx++;

    status = sbQe2000BistPmCheck(pDiagsInfo, &ulStatus0, &ulStatus1);
    if(0 != status)
    {
        *pulStatus |= 1 << nTestIdx;
        aulBistStatus[nTestIdx] = ulStatus1;
    }
    nTestIdx++;

    status = sbQe2000BistEgCheck(pDiagsInfo, &ulStatus0, &ulStatus1);
    if(0 != status)
    {
        *pulStatus |= 1 << nTestIdx;
        aulBistStatus[nTestIdx] = ulStatus1;
    }
    nTestIdx++;

    status = sbQe2000BistEiCheck(pDiagsInfo, &ulStatus0, &ulStatus1);
    if(0 != status)
    {
        *pulStatus |= 1 << nTestIdx;
        aulBistStatus[nTestIdx] = ulStatus1;
    }
    nTestIdx++;

    status = sbQe2000BistEpCheck(pDiagsInfo, &ulStatus0, &ulStatus1);
    if(0 != status)
    {
        *pulStatus |= 1 << nTestIdx;
        aulBistStatus[nTestIdx] = ulStatus1;
    }
    nTestIdx++;

    status = sbQe2000BistRbCheck(pDiagsInfo, &ulStatus0, &ulStatus1);
    if(0 != status)
    {
        *pulStatus |= 1 << nTestIdx;
        aulBistStatus[nTestIdx] = ulStatus1;
    }
    nTestIdx++;

    status = sbQe2000BistQsCheck(pDiagsInfo, &ulStatus0, &ulStatus1);
    if(0 != status)
    {
        *pulStatus |= 1 << nTestIdx;
        aulBistStatus[nTestIdx] = ulStatus1;
    }
    nTestIdx++;

    return;
}

/*
 * Per block bist tests
 */
void sbQe2000BistSrStart(sbhandle tKaAddr, int nStride)
{
    uint32_t ulData;

    ulData = SAND_HAL_READ_STRIDE(tKaAddr, KA, SR, nStride, SR0_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, SR0_BIST_CONFIG0, BIST_SETUP, ulData, 2);

    /*
     * BIST devition for SR & ST
     */
    ulData = SAND_HAL_MOD_FIELD(KA, SR0_BIST_CONFIG0, BIST_CLK_ENABLE_SPI, ulData, 0);
    ulData = SAND_HAL_MOD_FIELD(KA, SR0_BIST_CONFIG0, BIST_CLK_ENABLE_CORE, ulData, 1);

    SAND_HAL_WRITE_STRIDE(tKaAddr, KA, SR, nStride, SR0_BIST_CONFIG0, ulData);

    SAND_HAL_WRITE_STRIDE(tKaAddr, KA, SR, nStride, SR0_BIST_CONFIG1, 0xffffffff);

}

void sbQe2000BistStStart(sbhandle tKaAddr, int nStride)
{
    uint32_t ulData;

    ulData = SAND_HAL_READ_STRIDE(tKaAddr, KA, ST, nStride, ST0_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, ST0_BIST_CONFIG0, BIST_SETUP, ulData, 2);

    /*
     * BIST devition for SR & ST
     */
    ulData = SAND_HAL_MOD_FIELD(KA, ST0_BIST_CONFIG0, BIST_CLK_ENABLE_SPI, ulData, 0);
    ulData = SAND_HAL_MOD_FIELD(KA, ST0_BIST_CONFIG0, BIST_CLK_ENABLE_CORE, ulData, 1);

    SAND_HAL_WRITE_STRIDE(tKaAddr, KA, ST, nStride, ST0_BIST_CONFIG0, ulData);

    SAND_HAL_WRITE_STRIDE(tKaAddr, KA, ST, nStride, ST0_BIST_CONFIG1, 0xffffffff);

}

void sbQe2000BistPcStart(sbhandle tKaAddr)
{
    uint32_t ulData;

    ulData = SAND_HAL_READ(tKaAddr, KA, PC_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, PC_BIST_CONFIG0, BIST_SETUP, ulData, 2);
    SAND_HAL_WRITE(tKaAddr, KA, PC_BIST_CONFIG0, ulData);
    SAND_HAL_WRITE(tKaAddr, KA, PC_BIST_CONFIG1, 0xffffffff);

}

void sbQe2000BistEbStart(sbhandle tKaAddr)
{
    uint32_t ulData;

    ulData = SAND_HAL_READ(tKaAddr, KA, EB_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, EB_BIST_CONFIG0, BIST_SETUP, ulData, 2);
    SAND_HAL_WRITE(tKaAddr, KA, EB_BIST_CONFIG0, ulData);
    SAND_HAL_WRITE(tKaAddr, KA, EB_BIST_CONFIG1, 0xffffffff);

}

void sbQe2000BistTxStart(sbhandle tKaAddr)
{
    uint32_t ulData;

    ulData = SAND_HAL_READ(tKaAddr, KA, TX_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, TX_BIST_CONFIG0, BIST_SETUP, ulData, 2);
    SAND_HAL_WRITE(tKaAddr, KA, TX_BIST_CONFIG0, ulData);
    SAND_HAL_WRITE(tKaAddr, KA, TX_BIST_CONFIG1, 0xffffffff);

}

void sbQe2000BistSvStart(sbhandle tKaAddr)
{
    uint32_t ulData;

    ulData = SAND_HAL_READ(tKaAddr, KA, SV_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, SV_BIST_CONFIG0, BIST_SETUP, ulData, 2);
    SAND_HAL_WRITE(tKaAddr, KA, SV_BIST_CONFIG0, ulData);
    SAND_HAL_WRITE(tKaAddr, KA, SV_BIST_CONFIG1, 0xffffffff);

}

void sbQe2000BistQmStart(sbhandle tKaAddr)
{
    uint32_t ulData;

    ulData = SAND_HAL_READ(tKaAddr, KA, QM_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, QM_BIST_CONFIG0, BIST_SETUP, ulData, 2);
    SAND_HAL_WRITE(tKaAddr, KA, QM_BIST_CONFIG0, ulData);
    SAND_HAL_WRITE(tKaAddr, KA, QM_BIST_CONFIG1, 0xffffffff);

}

void sbQe2000BistPmStart(sbhandle tKaAddr)
{
    uint32_t ulData;

    ulData = SAND_HAL_READ(tKaAddr, KA, PM_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, PM_BIST_CONFIG0, BIST_SETUP, ulData, 2);
    SAND_HAL_WRITE(tKaAddr, KA, PM_BIST_CONFIG0, ulData);
    SAND_HAL_WRITE(tKaAddr, KA, PM_BIST_CONFIG1, 0xffffffff);

}

void sbQe2000BistEgStart(sbhandle tKaAddr)
{
    uint32_t ulData;

    ulData = SAND_HAL_READ(tKaAddr, KA, EG_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, EG_BIST_CONFIG0, BIST_SETUP, ulData, 2);
    SAND_HAL_WRITE(tKaAddr, KA, EG_BIST_CONFIG0, ulData);
    SAND_HAL_WRITE(tKaAddr, KA, EG_BIST_CONFIG1, 0xffffffff);

}

void sbQe2000BistEiStart(sbhandle tKaAddr)
{
    uint32_t ulData;

    ulData = SAND_HAL_READ(tKaAddr, KA, EI_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, EI_BIST_CONFIG0, BIST_SETUP, ulData, 2);
    SAND_HAL_WRITE(tKaAddr, KA, EI_BIST_CONFIG0, ulData);
    SAND_HAL_WRITE(tKaAddr, KA, EI_BIST_CONFIG1, 0xffffffff);

}

void sbQe2000BistEpStart(sbhandle tKaAddr)
{
    uint32_t ulData;

    ulData = SAND_HAL_READ(tKaAddr, KA, EP_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, EP_BIST_CONFIG0, BIST_SETUP, ulData, 2);
    SAND_HAL_WRITE(tKaAddr, KA, EP_BIST_CONFIG0, ulData);
    SAND_HAL_WRITE(tKaAddr, KA, EP_BIST_CONFIG1, 0xffffffff);

}

void sbQe2000BistRbStart(sbhandle tKaAddr)
{
    uint32_t ulData;

    ulData = SAND_HAL_READ(tKaAddr, KA, RB_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, RB_BIST_CONFIG0, BIST_SETUP, ulData, 2);
    SAND_HAL_WRITE(tKaAddr, KA, RB_BIST_CONFIG0, ulData);
    SAND_HAL_WRITE(tKaAddr, KA, RB_BIST_CONFIG1, 0xffffffff);

}

void sbQe2000BistQsStart(sbhandle tKaAddr)
{
    uint32_t ulData;

    SAND_HAL_RMW_FIELD(tKaAddr, KA, PC_CORE_RESET0, QS_CORE_RESET, 0x0);
    ulData = SAND_HAL_READ(tKaAddr, KA, QS_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, QS_BIST_CONFIG0, BIST_SETUP, ulData, 2);
    SAND_HAL_WRITE(tKaAddr, KA, QS_BIST_CONFIG0, ulData);
    SAND_HAL_WRITE(tKaAddr, KA, QS_BIST_CONFIG1, 0xffffffff);

}


/*
 * Per block bist checkers
 */

int sbQe2000BistSrCheck(sbxDiagsInfo_t *pDiagsInfo,
			int nStride,
			uint32_t *pulStatus0,
			uint32_t *pulStatus1)
{
    uint32_t ulEnableMask;
    uint32_t ulDoneMask;
    uint32_t ulGoMask;
    uint32_t ulData;
    int nTimeOut;
    int nStatus;
    sbhandle tKaAddr;
    int diagVerbose_b;
    tKaAddr = pDiagsInfo->userDeviceHandle;
    diagVerbose_b = pDiagsInfo->debug_level;

    ulDoneMask = 0;
    ulGoMask = 0;
    nStatus = 0;
    nTimeOut = 1000;

    ulEnableMask = SAND_HAL_READ_STRIDE(tKaAddr, KA, SR, nStride, SR0_BIST_CONFIG1);

    while(nTimeOut)
    {
        ulDoneMask = SAND_HAL_READ_STRIDE(tKaAddr, KA, SR, nStride, SR0_BIST_STATUS0);
        ulGoMask = SAND_HAL_READ_STRIDE(tKaAddr, KA, SR, nStride, SR0_BIST_STATUS1);
        if (ulDoneMask == ulEnableMask)
            break;
        thin_delay(1000);
        nTimeOut--;

    }

    /*
     *  none of the blocks have 32 memories, so a Go mask of 0xffffffff should
     *  be flagged as an error. This happens when the Kamino clock is running really
     * low in the lab. We want to flag this as an error to avoid a false positive indication
     */
    if(diagVerbose_b) {
        if (0xffffffff == ulGoMask)
        {
            SB_LOG( "%s:  False positive indication, bogus masks on read back. nEnableMask=0x%x, nDoneMask=0x%x, nGoMask=0x%x",
                      __PRETTY_FUNCTION__, ulEnableMask, ulDoneMask, ulGoMask);
        } else {
            SB_LOG( "%s:  Enable Mask 0x%08X GoMask 0x%08X\n", __PRETTY_FUNCTION__, ulEnableMask, ulGoMask);
        }
    }

    /*
     * clear the enables
     */
    SAND_HAL_WRITE_STRIDE(tKaAddr, KA, SR, nStride, SR0_BIST_CONFIG1, 0);

    ulData = SAND_HAL_READ_STRIDE(tKaAddr, KA, SR, nStride, SR0_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, SR0_BIST_CONFIG0, BIST_SETUP, ulData, 0);
    ulData = SAND_HAL_MOD_FIELD(KA, SR0_BIST_CONFIG0, BIST_DIAG_EN, ulData, 0);
    ulData = SAND_HAL_MOD_FIELD(KA, SR0_BIST_CONFIG0, BIST_CLK_ENABLE_SPI, ulData, 1);
    ulData = SAND_HAL_MOD_FIELD(KA, SR0_BIST_CONFIG0, BIST_CLK_ENABLE_CORE, ulData, 0);

    SAND_HAL_WRITE_STRIDE(tKaAddr, KA, SR, nStride, SR0_BIST_CONFIG0, ulData);

    if(ulEnableMask != ulGoMask)
    {
        *pulStatus0 = ulEnableMask;
        *pulStatus1 = ulGoMask;
        nStatus = -1;
    }

    return( nStatus );
}

int sbQe2000BistStCheck(sbxDiagsInfo_t *pDiagsInfo,
			int nStride,
			uint32_t *pulStatus0,
			uint32_t *pulStatus1)
{
    uint32_t ulEnableMask;
    uint32_t ulDoneMask;
    uint32_t ulGoMask;
    uint32_t ulData;
    int nTimeOut;
    int nStatus;
    sbhandle tKaAddr;
    int diagVerbose_b;
    tKaAddr = pDiagsInfo->userDeviceHandle;
    diagVerbose_b = pDiagsInfo->debug_level;

    ulDoneMask = 0;
    ulGoMask = 0;
    nStatus = 0;
    nTimeOut = 1000;

    ulEnableMask = SAND_HAL_READ_STRIDE(tKaAddr, KA, ST, nStride, ST0_BIST_CONFIG1);

    while(nTimeOut)
    {
        ulDoneMask = SAND_HAL_READ_STRIDE(tKaAddr, KA, ST, nStride, ST0_BIST_STATUS0);
        ulGoMask = SAND_HAL_READ_STRIDE(tKaAddr, KA, ST, nStride, ST0_BIST_STATUS1);
        if (ulDoneMask == ulEnableMask)
            break;
        thin_delay(1000);
        nTimeOut--;

    }

    /*
     *  none of the blocks have 32 memories, so a Go mask of 0xffffffff should
     *  be flagged as an error. This happens when the Kamino clock is running really
     * low in the lab. We want to flag this as an error to avoid a false positive indication
     */
    if(diagVerbose_b) {
        if (0xffffffff == ulGoMask)
        {
            SB_LOG( "%s:  False positive indication, bogus masks on read back. nEnableMask=0x%x, nDoneMask=0x%x, nGoMask=0x%x",
                      __PRETTY_FUNCTION__, ulEnableMask, ulDoneMask, ulGoMask);
        } else {
            SB_LOG( "%s:  Enable Mask 0x%08X GoMask 0x%08X\n", __PRETTY_FUNCTION__, ulEnableMask, ulGoMask);
        }
    }
    /*
     * clear the enables
     */
    SAND_HAL_WRITE_STRIDE(tKaAddr, KA, ST, nStride, ST0_BIST_CONFIG1, 0);

    ulData = SAND_HAL_READ_STRIDE(tKaAddr, KA, ST, nStride, ST0_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, ST0_BIST_CONFIG0, BIST_SETUP, ulData, 0);
    ulData = SAND_HAL_MOD_FIELD(KA, ST0_BIST_CONFIG0, BIST_DIAG_EN, ulData, 0);
    ulData = SAND_HAL_MOD_FIELD(KA, ST0_BIST_CONFIG0, BIST_CLK_ENABLE_SPI, ulData, 1);
    ulData = SAND_HAL_MOD_FIELD(KA, ST0_BIST_CONFIG0, BIST_CLK_ENABLE_CORE, ulData, 0);

    SAND_HAL_WRITE_STRIDE(tKaAddr, KA, ST, nStride, ST0_BIST_CONFIG0, ulData);

    if(ulEnableMask != ulGoMask)
    {
        *pulStatus0 = ulEnableMask;
        *pulStatus1 = ulGoMask;
        nStatus = -1;
    }

    return( nStatus );
}

int sbQe2000BistPcCheck(sbxDiagsInfo_t *pDiagsInfo,
			uint32_t *pulStatus0,
			uint32_t *pulStatus1)
{
    uint32_t ulEnableMask;
    uint32_t ulDoneMask;
    uint32_t ulGoMask;
    uint32_t ulData;
    int nTimeOut;
    int nStatus;
    sbhandle tKaAddr;
    int diagVerbose_b;
    tKaAddr = pDiagsInfo->userDeviceHandle;
    diagVerbose_b = pDiagsInfo->debug_level;

    ulDoneMask = 0;
    ulGoMask = 0;
    nStatus = 0;
    nTimeOut = 1000;

    ulEnableMask = SAND_HAL_READ(tKaAddr, KA, PC_BIST_CONFIG1);

    while(nTimeOut)
    {
        ulDoneMask = SAND_HAL_READ(tKaAddr, KA, PC_BIST_STATUS0);
        ulGoMask = SAND_HAL_READ(tKaAddr, KA, PC_BIST_STATUS1);
        if (ulDoneMask == ulEnableMask)
            break;
        thin_delay(1000);
        nTimeOut--;

    }

    /*
     *  none of the blocks have 32 memories, so a Go mask of 0xffffffff should
     *  be flagged as an error. This happens when the Kamino clock is running really
     * low in the lab. We want to flag this as an error to avoid a false positive indication
     */
    if(diagVerbose_b) {
        if (0xffffffff == ulGoMask)
        {
            SB_LOG( "%s:  False positive indication, bogus masks on read back. nEnableMask=0x%x, nDoneMask=0x%x, nGoMask=0x%x",
                      __PRETTY_FUNCTION__, ulEnableMask, ulDoneMask, ulGoMask);
        } else {
            SB_LOG( "%s:  Enable Mask 0x%08X GoMask 0x%08X\n", __PRETTY_FUNCTION__, ulEnableMask, ulGoMask);
        }
    }

    /*
     * clear the enables
     */
    SAND_HAL_WRITE(tKaAddr, KA, PC_BIST_CONFIG1, 0);

    ulData = SAND_HAL_READ(tKaAddr, KA, PC_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, PC_BIST_CONFIG0, BIST_SETUP, ulData, 0);
    ulData = SAND_HAL_MOD_FIELD(KA, PC_BIST_CONFIG0, BIST_DIAG_EN, ulData, 0);

    SAND_HAL_WRITE(tKaAddr, KA, PC_BIST_CONFIG0, ulData);

    if(ulEnableMask != ulGoMask)
    {
        *pulStatus0 = ulEnableMask;
        *pulStatus1 = ulGoMask;
        nStatus = -1;
    }

    return( nStatus );
}

int sbQe2000BistEbCheck(sbxDiagsInfo_t *pDiagsInfo,
			uint32_t *pulStatus0,
			uint32_t *pulStatus1)
{
    uint32_t ulEnableMask;
    uint32_t ulDoneMask;
    uint32_t ulGoMask;
    uint32_t ulData;
    int nTimeOut;
    int nStatus;
    sbhandle tKaAddr;
    int diagVerbose_b;
    tKaAddr = pDiagsInfo->userDeviceHandle;
    diagVerbose_b = pDiagsInfo->debug_level;

    ulDoneMask = 0;
    ulGoMask = 0;
    nStatus = 0;
    nTimeOut = 1000;

    ulEnableMask = SAND_HAL_READ(tKaAddr, KA, EB_BIST_CONFIG1);

    while(nTimeOut)
    {
        ulDoneMask = SAND_HAL_READ(tKaAddr, KA, EB_BIST_STATUS0);
        ulGoMask = SAND_HAL_READ(tKaAddr, KA, EB_BIST_STATUS1);
        if (ulDoneMask == ulEnableMask)
            break;
        thin_delay(1000);
        nTimeOut--;

    }

    /*
     *  none of the blocks have 32 memories, so a Go mask of 0xffffffff should
     *  be flagged as an error. This happens when the Kamino clock is running really
     * low in the lab. We want to flag this as an error to avoid a false positive indication
     */
    if(diagVerbose_b) {
        if (0xffffffff == ulGoMask)
        {
            SB_LOG("%s:  False positive indication, bogus masks on read back. nEnableMask=0x%x, nDoneMask=0x%x, nGoMask=0x%x",
                      __PRETTY_FUNCTION__, ulEnableMask, ulDoneMask, ulGoMask);
        } else {
            SB_LOG("%s:  Enable Mask 0x%08X GoMask 0x%08X\n", __PRETTY_FUNCTION__, ulEnableMask, ulGoMask);
        }
    }

    /*
     * clear the enables
     */
    SAND_HAL_WRITE(tKaAddr, KA, EB_BIST_CONFIG1, 0);

    ulData = SAND_HAL_READ(tKaAddr, KA, EB_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, EB_BIST_CONFIG0, BIST_SETUP, ulData, 0);
    ulData = SAND_HAL_MOD_FIELD(KA, EB_BIST_CONFIG0, BIST_DIAG_EN, ulData, 0);

    SAND_HAL_WRITE(tKaAddr, KA, EB_BIST_CONFIG0, ulData);

    if(ulEnableMask != ulGoMask)
    {
        *pulStatus0 = ulEnableMask;
        *pulStatus1 = ulGoMask;
        nStatus = -1;
    }

    return( nStatus );
}

int sbQe2000BistTxCheck(sbxDiagsInfo_t *pDiagsInfo,
			uint32_t *pulStatus0,
			uint32_t *pulStatus1)
{
    uint32_t ulEnableMask;
    uint32_t ulDoneMask;
    uint32_t ulGoMask;
    uint32_t ulData;
    int nTimeOut;
    int nStatus;
    sbhandle tKaAddr;
    int diagVerbose_b;
    tKaAddr = pDiagsInfo->userDeviceHandle;
    diagVerbose_b = pDiagsInfo->debug_level;

    ulDoneMask = 0;
    ulGoMask = 0;
    nStatus = 0;
    nTimeOut = 1000;

    ulEnableMask = SAND_HAL_READ(tKaAddr, KA, TX_BIST_CONFIG1);

    while(nTimeOut)
    {
        ulDoneMask = SAND_HAL_READ(tKaAddr, KA, TX_BIST_STATUS0);
        ulGoMask = SAND_HAL_READ(tKaAddr, KA, TX_BIST_STATUS1);
        if (ulDoneMask == ulEnableMask)
            break;
        thin_delay(1000);
        nTimeOut--;

    }

    /*
     *  none of the blocks have 32 memories, so a Go mask of 0xffffffff should
     *  be flagged as an error. This happens when the Kamino clock is running really
     * low in the lab. We want to flag this as an error to avoid a false positive indication
     */
    if(diagVerbose_b) {
        if (0xffffffff == ulGoMask)
        {
            SB_LOG("%s:  False positive indication, bogus masks on read back. nEnableMask=0x%x, nDoneMask=0x%x, nGoMask=0x%x",
                      __PRETTY_FUNCTION__, ulEnableMask, ulDoneMask, ulGoMask);
        } else {
            SB_LOG("%s:  Enable Mask 0x%08X GoMask 0x%08X\n", __PRETTY_FUNCTION__, ulEnableMask, ulGoMask);
        }
    }

    /*
     * clear the enables
     */
    SAND_HAL_WRITE(tKaAddr, KA, TX_BIST_CONFIG1, 0);

    ulData = SAND_HAL_READ(tKaAddr, KA, TX_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, TX_BIST_CONFIG0, BIST_SETUP, ulData, 0);
    ulData = SAND_HAL_MOD_FIELD(KA, TX_BIST_CONFIG0, BIST_DIAG_EN, ulData, 0);

    SAND_HAL_WRITE(tKaAddr, KA, TX_BIST_CONFIG0, ulData);

    if(ulEnableMask != ulGoMask)
    {
        *pulStatus0 = ulEnableMask;
        *pulStatus1 = ulGoMask;
        nStatus = -1;
    }

    return( nStatus );
}

int sbQe2000BistSvCheck(sbxDiagsInfo_t *pDiagsInfo,
			uint32_t *pulStatus0,
			uint32_t *pulStatus1)
{
    uint32_t ulEnableMask;
    uint32_t ulDoneMask;
    uint32_t ulGoMask;
    uint32_t ulData;
    int nTimeOut;
    int nStatus;
    sbhandle tKaAddr;
    int diagVerbose_b;
    tKaAddr = pDiagsInfo->userDeviceHandle;
    diagVerbose_b = pDiagsInfo->debug_level;

    ulDoneMask = 0;
    ulGoMask = 0;
    nStatus = 0;
    nTimeOut = 1000;

    ulEnableMask = SAND_HAL_READ(tKaAddr, KA, SV_BIST_CONFIG1);

    while(nTimeOut)
    {
        ulDoneMask = SAND_HAL_READ(tKaAddr, KA, SV_BIST_STATUS0);
        ulGoMask = SAND_HAL_READ(tKaAddr, KA, SV_BIST_STATUS1);
        if (ulDoneMask == ulEnableMask)
            break;
        thin_delay(1000);
        nTimeOut--;

    }

    /*
     *  none of the blocks have 32 memories, so a Go mask of 0xffffffff should
     *  be flagged as an error. This happens when the Kamino clock is running really
     * low in the lab. We want to flag this as an error to avoid a false positive indication
     */
    if(diagVerbose_b) {
        if (0xffffffff == ulGoMask)
        {
            SB_LOG( "%s:  False positive indication, bogus masks on read back. nEnableMask=0x%x, nDoneMask=0x%x, nGoMask=0x%x",
                      __PRETTY_FUNCTION__, ulEnableMask, ulDoneMask, ulGoMask);
        } else {
            SB_LOG( "%s:  Enable Mask 0x%08X GoMask 0x%08X\n", __PRETTY_FUNCTION__, ulEnableMask, ulGoMask);
        }
    }

    /*
     * clear the enables
     */
    SAND_HAL_WRITE(tKaAddr, KA, SV_BIST_CONFIG1, 0);

    ulData = SAND_HAL_READ(tKaAddr, KA, SV_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, SV_BIST_CONFIG0, BIST_SETUP, ulData, 0);
    ulData = SAND_HAL_MOD_FIELD(KA, SV_BIST_CONFIG0, BIST_DIAG_EN, ulData, 0);

    SAND_HAL_WRITE(tKaAddr, KA, SV_BIST_CONFIG0, ulData);

    if(ulEnableMask != ulGoMask)
    {
        *pulStatus0 = ulEnableMask;
        *pulStatus1 = ulGoMask;
        nStatus = -1;
    }

    return( nStatus );
}

int sbQe2000BistQmCheck(sbxDiagsInfo_t *pDiagsInfo,
			uint32_t *pulStatus0,
			uint32_t *pulStatus1)
{
    uint32_t ulEnableMask;
    uint32_t ulDoneMask;
    uint32_t ulGoMask;
    uint32_t ulData;
    int nTimeOut;
    int nStatus;
    sbhandle tKaAddr;
    int diagVerbose_b;
    tKaAddr = pDiagsInfo->userDeviceHandle;
    diagVerbose_b = pDiagsInfo->debug_level;

    ulDoneMask = 0;
    ulGoMask = 0;
    nStatus = 0;
    nTimeOut = 1000;

    ulEnableMask = SAND_HAL_READ(tKaAddr, KA, QM_BIST_CONFIG1);

    while(nTimeOut)
    {
        ulDoneMask = SAND_HAL_READ(tKaAddr, KA, QM_BIST_STATUS0);
        ulGoMask = SAND_HAL_READ(tKaAddr, KA, QM_BIST_STATUS1);
        if (ulDoneMask == ulEnableMask)
            break;
        thin_delay(1000);
        nTimeOut--;

    }

    /*
     *  none of the blocks have 32 memories, so a Go mask of 0xffffffff should
     *  be flagged as an error. This happens when the Kamino clock is running really
     * low in the lab. We want to flag this as an error to avoid a false positive indication
     */
    if(diagVerbose_b) {
        if (0xffffffff == ulGoMask)
        {
            SB_LOG("%s:  False positive indication, bogus masks on read back. nEnableMask=0x%x, nDoneMask=0x%x, nGoMask=0x%x",
                      __PRETTY_FUNCTION__, ulEnableMask, ulDoneMask, ulGoMask);
        } else {
            SB_LOG("%s:  Enable Mask 0x%08X GoMask 0x%08X\n", __PRETTY_FUNCTION__, ulEnableMask, ulGoMask);
        }
    }
    /*
     * clear the enables
     */
    SAND_HAL_WRITE(tKaAddr, KA, QM_BIST_CONFIG1, 0);

    ulData = SAND_HAL_READ(tKaAddr, KA, QM_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, QM_BIST_CONFIG0, BIST_SETUP, ulData, 0);
    ulData = SAND_HAL_MOD_FIELD(KA, QM_BIST_CONFIG0, BIST_DIAG_EN, ulData, 0);

    SAND_HAL_WRITE(tKaAddr, KA, QM_BIST_CONFIG0, ulData);

    if(ulEnableMask != ulGoMask)
    {
        *pulStatus0 = ulEnableMask;
        *pulStatus1 = ulGoMask;
        nStatus = -1;
    }

    return( nStatus );
}

int sbQe2000BistPmCheck(sbxDiagsInfo_t *pDiagsInfo,
			uint32_t *pulStatus0,
			uint32_t *pulStatus1)
{
    uint32_t ulEnableMask;
    uint32_t ulDoneMask;
    uint32_t ulGoMask;
    uint32_t ulData;
    int nTimeOut;
    int nStatus;
    sbhandle tKaAddr;
    int diagVerbose_b;
    tKaAddr = pDiagsInfo->userDeviceHandle;
    diagVerbose_b = pDiagsInfo->debug_level;

    ulDoneMask = 0;
    ulGoMask = 0;
    nStatus = 0;
    nTimeOut = 1000;

    ulEnableMask = SAND_HAL_READ(tKaAddr, KA, PM_BIST_CONFIG1);

    while(nTimeOut)
    {
        ulDoneMask = SAND_HAL_READ(tKaAddr, KA, PM_BIST_STATUS0);
        ulGoMask = SAND_HAL_READ(tKaAddr, KA, PM_BIST_STATUS1);
        if (ulDoneMask == ulEnableMask)
            break;
        thin_delay(1000);
        nTimeOut--;

    }

    /*
     *  none of the blocks have 32 memories, so a Go mask of 0xffffffff should
     *  be flagged as an error. This happens when the Kamino clock is running really
     * low in the lab. We want to flag this as an error to avoid a false positive indication
     */
    if(diagVerbose_b) {
        if (0xffffffff == ulGoMask)
        {
            SB_LOG( "%s:  False positive indication, bogus masks on read back. nEnableMask=0x%x, nDoneMask=0x%x, nGoMask=0x%x",
                      __PRETTY_FUNCTION__, ulEnableMask, ulDoneMask, ulGoMask);
        } else {
            SB_LOG( "%s:  Enable Mask 0x%08X GoMask 0x%08X\n", __PRETTY_FUNCTION__, ulEnableMask, ulGoMask);
        }
    }

    /*
     * clear the enables
     */
    SAND_HAL_WRITE(tKaAddr, KA, PM_BIST_CONFIG1, 0);

    ulData = SAND_HAL_READ(tKaAddr, KA, PM_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, PM_BIST_CONFIG0, BIST_SETUP, ulData, 0);
    ulData = SAND_HAL_MOD_FIELD(KA, PM_BIST_CONFIG0, BIST_DIAG_EN, ulData, 0);

    SAND_HAL_WRITE(tKaAddr, KA, PM_BIST_CONFIG0, ulData);

    if(ulEnableMask != ulGoMask)
    {
        *pulStatus0 = ulEnableMask;
        *pulStatus1 = ulGoMask;
        nStatus = -1;
    }

    return( nStatus );
}

int sbQe2000BistEgCheck( sbxDiagsInfo_t *pDiagsInfo,
			 uint32_t *pulStatus0,
			 uint32_t *pulStatus1)
{
    uint32_t ulEnableMask;
    uint32_t ulDoneMask;
    uint32_t ulGoMask;
    uint32_t ulData;
    int nTimeOut;
    int nStatus;
    sbhandle tKaAddr;
    int diagVerbose_b;
    tKaAddr = pDiagsInfo->userDeviceHandle;
    diagVerbose_b = pDiagsInfo->debug_level;

    ulDoneMask = 0;
    ulGoMask = 0;
    nStatus = 0;
    nTimeOut = 1000;

    ulEnableMask = SAND_HAL_READ(tKaAddr, KA, EG_BIST_CONFIG1);

    while(nTimeOut)
    {
        ulDoneMask = SAND_HAL_READ(tKaAddr, KA, EG_BIST_STATUS0);
        ulGoMask = SAND_HAL_READ(tKaAddr, KA, EG_BIST_STATUS1);
        if (ulDoneMask == ulEnableMask)
            break;
        thin_delay(1000);
        nTimeOut--;

    }

    /*
     *  none of the blocks have 32 memories, so a Go mask of 0xffffffff should
     *  be flagged as an error. This happens when the Kamino clock is running really
     * low in the lab. We want to flag this as an error to avoid a false positive indication
     */
    if(diagVerbose_b) {
        if (0xffffffff == ulGoMask)
        {
            SB_LOG( "%s:  False positive indication, bogus masks on read back. nEnableMask=0x%x, nDoneMask=0x%x, nGoMask=0x%x",
                      __PRETTY_FUNCTION__, ulEnableMask, ulDoneMask, ulGoMask);
        } else {
            SB_LOG( "%s:  Enable Mask 0x%08X GoMask 0x%08X\n", __PRETTY_FUNCTION__, ulEnableMask, ulGoMask);
        }
    }
    /*
     * clear the enables
     */
    SAND_HAL_WRITE(tKaAddr, KA, EG_BIST_CONFIG1, 0);

    ulData = SAND_HAL_READ(tKaAddr, KA, EG_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, EG_BIST_CONFIG0, BIST_SETUP, ulData, 0);
    ulData = SAND_HAL_MOD_FIELD(KA, EG_BIST_CONFIG0, BIST_DIAG_EN, ulData, 0);

    SAND_HAL_WRITE(tKaAddr, KA, EG_BIST_CONFIG0, ulData);

    if(ulEnableMask != ulGoMask)
    {
        *pulStatus0 = ulEnableMask;
        *pulStatus1 = ulGoMask;
        nStatus = -1;
    }

    return( nStatus );
}

int sbQe2000BistEiCheck(sbxDiagsInfo_t *pDiagsInfo,
			uint32_t *pulStatus0,
			uint32_t *pulStatus1)
{
    uint32_t ulEnableMask;
    uint32_t ulDoneMask;
    uint32_t ulGoMask;
    uint32_t ulData;
    int nTimeOut;
    int nStatus;
    sbhandle tKaAddr;
    int diagVerbose_b;
    tKaAddr = pDiagsInfo->userDeviceHandle;
    diagVerbose_b = pDiagsInfo->debug_level;

    ulDoneMask = 0;
    ulGoMask = 0;
    nStatus = 0;
    nTimeOut = 1000;

    ulEnableMask = SAND_HAL_READ(tKaAddr, KA, EI_BIST_CONFIG1);

    while(nTimeOut)
    {
        ulDoneMask = SAND_HAL_READ(tKaAddr, KA, EI_BIST_STATUS0);
        ulGoMask = SAND_HAL_READ(tKaAddr, KA, EI_BIST_STATUS1);
        if (ulDoneMask == ulEnableMask)
            break;
        thin_delay(1000);
        nTimeOut--;

    }

    /*
     *  none of the blocks have 32 memories, so a Go mask of 0xffffffff should
     *  be flagged as an error. This happens when the Kamino clock is running really
     * low in the lab. We want to flag this as an error to avoid a false positive indication
     */
    if(diagVerbose_b) {
        if (0xffffffff == ulGoMask)
        {
            SB_LOG( "%s:  False positive indication, bogus masks on read back. nEnableMask=0x%x, nDoneMask=0x%x, nGoMask=0x%x",
                      __PRETTY_FUNCTION__, ulEnableMask, ulDoneMask, ulGoMask);
        } else {
            SB_LOG( "%s:  Enable Mask 0x%08X GoMask 0x%08X\n", __PRETTY_FUNCTION__, ulEnableMask, ulGoMask);
        }
    }
    /*
     * clear the enables
     */
    SAND_HAL_WRITE(tKaAddr, KA, EI_BIST_CONFIG1, 0);

    ulData = SAND_HAL_READ(tKaAddr, KA, EI_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, EI_BIST_CONFIG0, BIST_SETUP, ulData, 0);
    ulData = SAND_HAL_MOD_FIELD(KA, EI_BIST_CONFIG0, BIST_DIAG_EN, ulData, 0);

    SAND_HAL_WRITE(tKaAddr, KA, EI_BIST_CONFIG0, ulData);

    if(ulEnableMask != ulGoMask)
    {
        *pulStatus0 = ulEnableMask;
        *pulStatus1 = ulGoMask;
        nStatus = -1;
    }

    return( nStatus );
}

int sbQe2000BistEpCheck(sbxDiagsInfo_t *pDiagsInfo,
			uint32_t *pulStatus0,
			uint32_t *pulStatus1)
{
    uint32_t ulEnableMask;
    uint32_t ulDoneMask;
    uint32_t ulGoMask;
    uint32_t ulData;
    int nTimeOut;
    int nStatus;
    sbhandle tKaAddr;
    int diagVerbose_b;
    tKaAddr = pDiagsInfo->userDeviceHandle;
    diagVerbose_b = pDiagsInfo->debug_level;

    ulDoneMask = 0;
    ulGoMask = 0;
    nStatus = 0;
    nTimeOut = 1000;

    ulEnableMask = SAND_HAL_READ(tKaAddr, KA, EP_BIST_CONFIG1);

    while(nTimeOut)
    {
        ulDoneMask = SAND_HAL_READ(tKaAddr, KA, EP_BIST_STATUS0);
        ulGoMask = SAND_HAL_READ(tKaAddr, KA, EP_BIST_STATUS1);
        if (ulDoneMask == ulEnableMask)
            break;
        thin_delay(1000);
        nTimeOut--;

    }

    /*
     *  none of the blocks have 32 memories, so a Go mask of 0xffffffff should
     *  be flagged as an error. This happens when the Kamino clock is running really
     * low in the lab. We want to flag this as an error to avoid a false positive indication
     */
    if(diagVerbose_b) {
        if (0xffffffff == ulGoMask)
        {
            SB_LOG( "%s:  False positive indication, bogus masks on read back. nEnableMask=0x%x, nDoneMask=0x%x, nGoMask=0x%x",
                      __PRETTY_FUNCTION__, ulEnableMask, ulDoneMask, ulGoMask);
        } else {
            SB_LOG( "%s:  Enable Mask 0x%08X GoMask 0x%08X\n", __PRETTY_FUNCTION__, ulEnableMask, ulGoMask);
        }
    }

    /*
     * clear the enables
     */
    SAND_HAL_WRITE(tKaAddr, KA, EP_BIST_CONFIG1, 0);

    ulData = SAND_HAL_READ(tKaAddr, KA, EP_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, EP_BIST_CONFIG0, BIST_SETUP, ulData, 0);
    ulData = SAND_HAL_MOD_FIELD(KA, EP_BIST_CONFIG0, BIST_DIAG_EN, ulData, 0);

    SAND_HAL_WRITE(tKaAddr, KA, EP_BIST_CONFIG0, ulData);

    if(ulEnableMask != ulGoMask)
    {
        *pulStatus0 = ulEnableMask;
        *pulStatus1 = ulGoMask;
        nStatus = -1;
    }

    return( nStatus );
}

int sbQe2000BistRbCheck(sbxDiagsInfo_t *pDiagsInfo,
			uint32_t *pulStatus0,
			uint32_t *pulStatus1)
{
    uint32_t ulEnableMask;
    uint32_t ulDoneMask;
    uint32_t ulGoMask;
    uint32_t ulData;
    int nTimeOut;
    int nStatus;
    sbhandle tKaAddr;
    int diagVerbose_b;
    tKaAddr = pDiagsInfo->userDeviceHandle;
    diagVerbose_b = pDiagsInfo->debug_level;

    ulDoneMask = 0;
    ulGoMask = 0;
    nStatus = 0;
    nTimeOut = 1000;

    ulEnableMask = SAND_HAL_READ(tKaAddr, KA, RB_BIST_CONFIG1);

    while(nTimeOut)
    {
        ulDoneMask = SAND_HAL_READ(tKaAddr, KA, RB_BIST_STATUS0);
        ulGoMask = SAND_HAL_READ(tKaAddr, KA, RB_BIST_STATUS1);
        if (ulDoneMask == ulEnableMask)
            break;
        thin_delay(1000);
        nTimeOut--;

    }

    /*
     *  none of the blocks have 32 memories, so a Go mask of 0xffffffff should
     *  be flagged as an error. This happens when the Kamino clock is running really
     * low in the lab. We want to flag this as an error to avoid a false positive indication
     */
    if(diagVerbose_b) {
        if (0xffffffff == ulGoMask)
        {
            SB_LOG( "%s:  False positive indication, bogus masks on read back. nEnableMask=0x%x, nDoneMask=0x%x, nGoMask=0x%x",
                      __PRETTY_FUNCTION__, ulEnableMask, ulDoneMask, ulGoMask);
        } else {
            SB_LOG( "%s:  Enable Mask 0x%08X GoMask 0x%08X\n", __PRETTY_FUNCTION__, ulEnableMask, ulGoMask);
        }
    }

    /*
     * clear the enables
     */
    SAND_HAL_WRITE(tKaAddr, KA, RB_BIST_CONFIG1, 0);

    ulData = SAND_HAL_READ(tKaAddr, KA, RB_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, RB_BIST_CONFIG0, BIST_SETUP, ulData, 0);
    ulData = SAND_HAL_MOD_FIELD(KA, RB_BIST_CONFIG0, BIST_DIAG_EN, ulData, 0);

    SAND_HAL_WRITE(tKaAddr, KA, RB_BIST_CONFIG0, ulData);

    if(ulEnableMask != ulGoMask)
    {
        *pulStatus0 = ulEnableMask;
        *pulStatus1 = ulGoMask;
        nStatus = -1;
    }

    return( nStatus );
}

int sbQe2000BistQsCheck(sbxDiagsInfo_t *pDiagsInfo,
			uint32_t *pulStatus0,
			uint32_t *pulStatus1)
{
    uint32_t ulEnableMask;
    uint32_t ulDoneMask;
    uint32_t ulGoMask;
    uint32_t ulData;
    int nTimeOut;
    int nStatus;
    sbhandle tKaAddr;
    int diagVerbose_b;
    tKaAddr = pDiagsInfo->userDeviceHandle;
    diagVerbose_b = pDiagsInfo->debug_level;

    ulDoneMask = 0;
    ulGoMask = 0;
    nStatus = 0;
    nTimeOut = 1000;

    ulEnableMask = SAND_HAL_READ(tKaAddr, KA, QS_BIST_CONFIG1);

    while(nTimeOut)
    {
        ulDoneMask = SAND_HAL_READ(tKaAddr, KA, QS_BIST_STATUS0);
        ulGoMask = SAND_HAL_READ(tKaAddr, KA, QS_BIST_STATUS1);
        if (ulDoneMask == ulEnableMask)
            break;
        thin_delay(1000);
        nTimeOut--;
    }

    /*
     *  none of the blocks have 32 memories, so a Go mask of 0xffffffff should
     *  be flagged as an error. This happens when the Kamino clock is running really
     * low in the lab. We want to flag this as an error to avoid a false positive indication
     */
    if(diagVerbose_b) {
        if (0xffffffff == ulGoMask)
        {
            SB_LOG( "%s:  False positive indication, bogus masks on read back. nEnableMask=0x%x, nDoneMask=0x%x, nGoMask=0x%x",
                      __PRETTY_FUNCTION__, ulEnableMask, ulDoneMask, ulGoMask);
        } else {
            SB_LOG( "%s:  Enable Mask 0x%08X GoMask 0x%08X\n", __PRETTY_FUNCTION__, ulEnableMask, ulGoMask);
        }
    }
    /*
     * clear the enables
     */
    SAND_HAL_WRITE(tKaAddr, KA, QS_BIST_CONFIG1, 0);

    ulData = SAND_HAL_READ(tKaAddr, KA, QS_BIST_CONFIG0);
    ulData = SAND_HAL_MOD_FIELD(KA, QS_BIST_CONFIG0, BIST_SETUP, ulData, 0);
    ulData = SAND_HAL_MOD_FIELD(KA, QS_BIST_CONFIG0, BIST_DIAG_EN, ulData, 0);

    SAND_HAL_WRITE(tKaAddr, KA, QS_BIST_CONFIG0, ulData);

    if(ulEnableMask != ulGoMask)
    {
        *pulStatus0 = ulEnableMask;
        *pulStatus1 = ulGoMask;
        nStatus = -1;
    }

    return( nStatus );
}

/* added for debug when external BIST test fails */
void sbQe2kPmDumpChannelStatus(uint8_t uResultsArray[][16]) {

  uint32_t uDelay;
  uint32_t channel;
  uint32_t uChannelStatus[8] = {0};

  /* Channel status is stored as follows in the results array
     uResultsArray[uDelay][0] -- Channel 0 Upper Byte
     uResultsArray[uDelay][1] -- Channel 0 Lower Byte
     uResultsArray[uDelay][2] -- Channel 1 Upper Byte
     uResultsArray[uDelay][3] -- Channel 1 Lower Byte
     ....so on
  */


  /* If a channel fails it means no dly tap resulted in a correct
     read for both byte lanes.
  */

  for (uDelay = 0; uDelay < 256; uDelay++ ) {
    if ( uResultsArray[uDelay][0] == 0 &&	 uResultsArray[uDelay][1] == 0 ) {
      uChannelStatus[0] = 1; /* channel 0 passed */
    }
    if ( uResultsArray[uDelay][2] == 0 &&	 uResultsArray[uDelay][3] == 0 ) {
      uChannelStatus[1] = 1;
    }
    if ( uResultsArray[uDelay][4] == 0 &&	 uResultsArray[uDelay][5] == 0 ) {
      uChannelStatus[2] = 1;
    }
    if ( uResultsArray[uDelay][6] == 0 &&	 uResultsArray[uDelay][7] == 0 ) {
      uChannelStatus[3] = 1;
    }
    if ( uResultsArray[uDelay][8] == 0 &&	 uResultsArray[uDelay][9] == 0 ) {
      uChannelStatus[4] = 1;
    }
    if ( uResultsArray[uDelay][10] == 0 &&  uResultsArray[uDelay][11] == 0 ) {
      uChannelStatus[5] = 1;
    }
    if ( uResultsArray[uDelay][12] == 0 &&  uResultsArray[uDelay][13] == 0 ) {
      uChannelStatus[6] = 1;
    }
    if ( uResultsArray[uDelay][14] == 0 &&  uResultsArray[uDelay][15] == 0 ) {
      uChannelStatus[7] = 1;
    }
  }

  /* print out any failing channels */
  for (channel = 0; channel < 8; channel++) {
    if (uChannelStatus[channel] == 0 ) {
      SB_LOG("Channel %d failed training\n",channel);
    }
  }
}

#endif /* BCM_QE2000_SUPPORT */
#else
int appl_test_sbx_diags_not_empty;
#endif /* (BCM_SBX_SUPPORT) */
