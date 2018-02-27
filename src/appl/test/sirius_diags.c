/*
 * $Id: sirius_diags.c 1.7 Broadcom SDK $
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
 * File:        sirius_diags.c
 * Purpose:     Sirius-specific diagnostics tests
 * Requires:
 */

#include <soc/defs.h> 

#ifdef BCM_SIRIUS_SUPPORT
#include <appl/test/sirius_diags.h>
#include <soc/cm.h>
#include <bcm/debug.h>
#include <soc/drv.h>
#include <soc/sbx/glue.h>
#ifndef __KERNEL__
#include <signal.h>
#endif
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sirius.h>
#include <appl/diag/system.h>
#include <bcm/error.h>

#define printf soc_cm_print

#define CI_DDR_MASK 0x3fffff /* [21:0] bit field for writes */

#define SIRIUS_MEM_TIMEOUT (1000)
#define VT100_CLEAR_TO_EOL	"\033\133\113"  /* clear to end of line */

#define MAX_ITERATIONS 100
const char* siriusDiagsMemoryTestStr[] = {"DDR Standard burst",
					  "DDR walking ones on data bus",
					  "DDR walking zeros on data bus",
					  "DDR Data == Address",
                                          "DDR Indirect"};

/* used when DDR test is run continuously, stopped on ctrl-c */
static uint8_t gStopDDRTest = FALSE;

/* run through all the Functional DDR tests */
int siriusDiagsDDRTestAll(sbxDiagsInfo_t *pDiagsInfo) {
  uint32_t mem_test;
  siriusDiagsParams_t *pSiriusDiagParams = &(pDiagsInfo->siriusDiagParams); 
  int stat = 0;
  for(mem_test=0;mem_test<SIRIUS_MEM_TEST_LAST;mem_test++) {
    pSiriusDiagParams->ddr_test_mode = mem_test;
    /* skip the indirect test -- too long */
    if (mem_test == SIRIUS_DDR_INDIRECT_TEST) {
      continue;
    }
    if (siriusDiagsDDRTest(pDiagsInfo) != 0) {
      stat = -1;
    }
  }
  return stat;
}


int siriusDiagsDDRTest(sbxDiagsInfo_t *pDiagsInfo) {

  int rv = BCM_E_NONE;
  int ddr_test;
  siriusDiagsParams_t *pSiriusDiagParams = &(pDiagsInfo->siriusDiagParams);

  if (!(pSiriusDiagParams->ci_interface < 0) &&
      pSiriusDiagParams->ci_interface > SOC_MAX_NUM_CI_BLKS) {
    printf("Bad CI interface specified(%d) 0..9 supported\n",pSiriusDiagParams->ci_interface);
    return (-1);
  }

  ddr_test = pSiriusDiagParams->ddr_test_mode;
  switch(ddr_test) {
  case SIRIUS_DDR_STANDARD_TEST:                       /* burst write/read test */
  case SIRIUS_DATA_BUS_WALKING_ONES:
  case SIRIUS_DATA_BUS_WALKING_ZEROS:
  case SIRIUS_DDR_DATA_EQ_ADDR:
    rv = siriusDiagsDDRFunctionalTest(pDiagsInfo);     /* at speed tests */
    break;
  case SIRIUS_DDR_INDIRECT_TEST:                       /* indirect mem access test */
    rv = siriusDiagsDDRIndirectPat(pDiagsInfo);
    break;
  default:
    printf("Unknown DDR Test\n");
    return (-1);
  }

  if (rv == 0) {
    printf("Test %s passed.\n",siriusDiagsMemoryTestStr[ddr_test]);
  } else {
    printf("Test %s failed \n",siriusDiagsMemoryTestStr[ddr_test]);
  } 

  return rv;
}

int siriusDiagsDDRFunctionalTest(sbxDiagsInfo_t *pDiagsInfo) {

  int rv = BCM_E_NONE;
  int stat = 0;
  uint32 uData = 0;
  int unit;
  uint32 pattern = 0;
  int ci;
  int mode = 0;
  int ci_start, ci_end;
  uint64_t uFailedCount[SOC_MAX_NUM_CI_BLKS] = {0};
  uint64_t uPassedCount[SOC_MAX_NUM_CI_BLKS] = {0};
  uint64_t uTimedOutCount[SOC_MAX_NUM_CI_BLKS] = {0};
  uint64_t iter_count = 0;
  uint32_t uDDRIter;
  uint32_t uStartAddr;
  uint32_t uStepSize;
  uint32_t uDDRBurstSize;

  siriusDiagsParams_t *pSiriusDiagParams = &(pDiagsInfo->siriusDiagParams);
#ifndef __KERNEL__
  signal(SIGINT,(void *)sigcatcher_sirius);
#endif
  unit = pDiagsInfo->unit;
  gStopDDRTest = FALSE;

  if (pSiriusDiagParams->ci_interface < 0) {
    /* test all of them */
    ci_start = 0; ci_end = SOC_MAX_NUM_CI_BLKS;
  } else {
    ci_start = pSiriusDiagParams->ci_interface;
    ci_end = ci_start+1;
    printf("Testing only CI%d interface\n",ci_start);
  }

  mode = pSiriusDiagParams->ddr_test_mode;
  uDDRIter = pSiriusDiagParams->ddr_iter & CI_DDR_MASK;

  printf("Running %s test\n",siriusDiagsMemoryTestStr[mode]);

  if (pDiagsInfo->pattern != -1 ) {
    pattern = pDiagsInfo->pattern & 0xffffffff;
    if (mode != SIRIUS_DDR_STANDARD_TEST) {
      printf("NOTE: Test data is not used during data bus test\n");
    }
  } else {
    /* no pattern specified use alternating 0101.. pattern */
    pattern = 0x55555555;
  }

  uStartAddr = pSiriusDiagParams->ddr_start_addr & CI_DDR_MASK;
  uStepSize = pSiriusDiagParams->ddr_step_addr & CI_DDR_MASK;
  uDDRBurstSize = pSiriusDiagParams->ddr_burst & CI_DDR_MASK;

  if (mode == SIRIUS_DDR_STANDARD_TEST ||
      mode == SIRIUS_DDR_DATA_EQ_ADDR) {
    printf("start_addr=0x%x,addr_step_inc=0x%x,burst_size=%d\n",uStartAddr,uStepSize,uDDRBurstSize);
    if (mode == SIRIUS_DDR_STANDARD_TEST) {
      printf("using pattern=0x%8x, and alt_pattern=0x%8x\n",pattern,~pattern);
    }
  }

  for (ci = ci_start; ci < ci_end; ci++) {
    /* set test specific attributes */
    /* DIAG_IF_ERROR_RETURN(WRITE_CI_DEBUGr(unit,ci,0x1)); ? inject auto refresh */
    DIAG_IF_ERROR_RETURN(WRITE_CI_DDR_STARTr(unit,ci,uStartAddr));
    DIAG_IF_ERROR_RETURN(WRITE_CI_DDR_STEPr(unit,ci,uStepSize));
    DIAG_IF_ERROR_RETURN(WRITE_CI_DDR_BURSTr(unit,ci,uDDRBurstSize));
    /* only STANDARD_DDR test uses pattern data */
    if ( mode == SIRIUS_DDR_STANDARD_TEST) {
      DIAG_IF_ERROR_RETURN(WRITE_CI_TEST_DATA0r(unit,ci,pattern));
      DIAG_IF_ERROR_RETURN(WRITE_CI_TEST_DATA1r(unit,ci,pattern));
      DIAG_IF_ERROR_RETURN(WRITE_CI_TEST_DATA2r(unit,ci,pattern));
      DIAG_IF_ERROR_RETURN(WRITE_CI_TEST_DATA3r(unit,ci,pattern));
      DIAG_IF_ERROR_RETURN(WRITE_CI_TEST_DATA4r(unit,ci,pattern));
      DIAG_IF_ERROR_RETURN(WRITE_CI_TEST_DATA5r(unit,ci,pattern));
      DIAG_IF_ERROR_RETURN(WRITE_CI_TEST_DATA6r(unit,ci,pattern));
      DIAG_IF_ERROR_RETURN(WRITE_CI_TEST_DATA7r(unit,ci,pattern));
      /* set the alt_data */
      DIAG_IF_ERROR_RETURN(WRITE_CI_TEST_ALT_DATA0r(unit,ci,~pattern));
      DIAG_IF_ERROR_RETURN(WRITE_CI_TEST_ALT_DATA1r(unit,ci,~pattern));
      DIAG_IF_ERROR_RETURN(WRITE_CI_TEST_ALT_DATA2r(unit,ci,~pattern));
      DIAG_IF_ERROR_RETURN(WRITE_CI_TEST_ALT_DATA3r(unit,ci,~pattern));
      DIAG_IF_ERROR_RETURN(WRITE_CI_TEST_ALT_DATA4r(unit,ci,~pattern));
      DIAG_IF_ERROR_RETURN(WRITE_CI_TEST_ALT_DATA5r(unit,ci,~pattern));
      DIAG_IF_ERROR_RETURN(WRITE_CI_TEST_ALT_DATA6r(unit,ci,~pattern));
      DIAG_IF_ERROR_RETURN(WRITE_CI_TEST_ALT_DATA7r(unit,ci,~pattern));
      DIAG_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit,ci,&uData));
    }
  }

  /* Run the particular test uDDRIter times */
  for(iter_count = 0; iter_count < uDDRIter; iter_count++) {

    /* start the test for this iteration */
    if (( rv = siriusDiagsStartDDRFunctionalTest(pDiagsInfo,
						 ci_start,
						 ci_end,
						 mode)) < 0 ) {
	printf("%s failed to start %s test.\n",SOC_CHIP_STRING(unit),
	       siriusDiagsMemoryTestStr[mode]);
      return rv;
    }


    /* wait long enough for all ci's being tested to be done */
    sal_sleep(1);

    /* check the test results for this iteration */
    if (( rv = siriusDiagsResultsDDRFunctionalTest(pDiagsInfo,
						   ci_start,
						   ci_end,
						   uFailedCount,
						   uTimedOutCount,
						   uPassedCount)) < 0 ) {
      
	printf("%s failed %s test.\n",SOC_CHIP_STRING(unit),
	       siriusDiagsMemoryTestStr[mode]);

      /* one or more CI's failed
       * dump register debug info here
       */
      stat = -1;

    }

    /* display the results for this iteration */
    if (( rv = siriusDiagsDDRDumpCIResults(unit,
					   ci_start,
					   ci_end,
					   iter_count,
					   uFailedCount,
					   uTimedOutCount,
					   uPassedCount)) < 0) {
      return rv;
    }
  }

  /* special case if ddr_iter == 0, run test continuously */
  if (uDDRIter == 0) {
    /* start the test to run continuously */
    if (( rv = siriusDiagsStartDDRFunctionalTest(pDiagsInfo,
						 ci_start,
						 ci_end,
						 mode)) < 0 ) {
	printf("%s failed to start %s test.\n",SOC_CHIP_STRING(unit),
	       siriusDiagsMemoryTestStr[mode]);
      return rv;
    }
    printf("Running test continuously CTRL-C to STOP.. \n");
    while(1) {
      if (gStopDDRTest == TRUE) {
	/* stop the test by turning off ram_test */
	printf("Stopping Test.\n");
	for (ci = ci_start; ci < ci_end; ci++) {
	  DIAG_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit,ci,&uData));
	  soc_reg_field_set(unit,CI_DDR_TESTr,&uData,RAM_TESTf,0);
	  DIAG_IF_ERROR_RETURN(WRITE_CI_DDR_TESTr(unit,ci,uData));
	}
	break;
      }

      /* check results at some interval */
      sal_sleep(2);
      if (( rv = siriusDiagsResultsDDRFunctionalTest(pDiagsInfo,
						     ci_start,
						     ci_end,
						     uFailedCount,
						     uTimedOutCount,
						     uPassedCount)) < 0 ) {

	  printf("%s failed %s test.\n",SOC_CHIP_STRING(unit),
		 siriusDiagsMemoryTestStr[mode]);

	/* one or more CI's failed
         * todo dump register debug info here
	 */
	stat = -1;
      }

      /* display results while test is running */
      if (( rv = siriusDiagsDDRDumpCIResults(unit,
					     ci_start,
					     ci_end,
					     -1,
					     uFailedCount,
					     uTimedOutCount,
					     uPassedCount)) < 0) {
	return rv;

      }
    }
  }

  /* clean up after test run */
  for (ci = ci_start; ci < ci_end; ci++) {
    DIAG_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit,ci,&uData));
    soc_reg_field_set(unit,CI_DDR_TESTr,&uData,RAM_DONEf,1);      /* W1TC */
    soc_reg_field_set(unit,CI_DDR_TESTr,&uData,RAM_TESTf,0);      /* clear */
    soc_reg_field_set(unit,CI_DDR_TESTr,&uData,RAM_TEST_FAILf,1); /* W1TC */
    DIAG_IF_ERROR_RETURN(WRITE_CI_DDR_TESTr(unit,ci,uData));
  }

  return stat;
}

int siriusDiagsStartDDRFunctionalTest(sbxDiagsInfo_t *pDiagsInfo,
				      int ci_start,
				      int ci_end,
				      int mode) {

  uint32_t uData;
  int unit = pDiagsInfo->unit;
  int ci;
  uint32_t ddr_iter;

  siriusDiagsParams_t *pSiriusDiagParams = &(pDiagsInfo->siriusDiagParams);
  ddr_iter = pSiriusDiagParams->ddr_iter & CI_DDR_MASK;

  /*
   * Be able to Show test results for each iteration, unless ddr_iter == 0
   * then results are displayed after some interval
   */

  if (ddr_iter != 0) ddr_iter = 1;

  /*
   * Set the test mode as follows:
   * 0 - Standard writing burst of data and alt_data, followed by reads
   * 1 - Walking ones test on the data lines (test data not used).
   * 2 - Walking zeros on the test data lines (test data not used). 
   * 3 - Used Data == Address (test data not used)
   * - clear ram_done
   * - clear ram_test
   * - clear ram_test_fail
   * - set ram_test to initiate test
   */

  uData = 0;
  for (ci = ci_start; ci < ci_end; ci++) {
    DIAG_IF_ERROR_RETURN(WRITE_CI_DDR_ITERr(unit,ci,ddr_iter));
    DIAG_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit,ci,&uData));
    soc_reg_field_set(unit,CI_DDR_TESTr,&uData,MODEf,mode);
    soc_reg_field_set(unit,CI_DDR_TESTr,&uData,RAM_DONEf,1);      /* W1TC */
    soc_reg_field_set(unit,CI_DDR_TESTr,&uData,RAM_TESTf,0);      /* clear */
    soc_reg_field_set(unit,CI_DDR_TESTr,&uData,RAM_TEST_FAILf,1); /* W1TC */
    DIAG_IF_ERROR_RETURN(WRITE_CI_DDR_TESTr(unit,ci,uData));

    /* set ram_test - to start the test */
    DIAG_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit,ci,&uData));
    soc_reg_field_set(unit,CI_DDR_TESTr,&uData,RAM_TESTf,1);
    DIAG_IF_ERROR_RETURN(WRITE_CI_DDR_TESTr(unit,ci,uData));
    
  }

  return BCM_E_NONE;
}

 int siriusDiagsResultsDDRFunctionalTest(sbxDiagsInfo_t *pDiagsInfo,
					 int ci_start,
					 int ci_end,
					 uint64_t *pFailedCount,
					 uint64_t *pTimedOutCount,
					 uint64_t *pPassedCount) {

   int rv = BCM_E_NONE;
   int ci = 0;
   uint32_t uData = 0;
   uint8 bFailed = 0;
   uint8 bDone = 0;
   int unit = 0;
   uint32_t ddr_iter = 0;
   siriusDiagsParams_t *pSiriusDiagParams = &(pDiagsInfo->siriusDiagParams);

   unit = pDiagsInfo->unit;
   ddr_iter = pSiriusDiagParams->ddr_iter & CI_DDR_MASK;

   for (ci = ci_start; ci < ci_end; ci++) {
     DIAG_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit,ci,&uData));
     bFailed = soc_reg_field_get(unit,CI_DDR_TESTr,uData,RAM_TEST_FAILf);
     bDone = soc_reg_field_get(unit,CI_DDR_TESTr,uData,RAM_DONEf);
     if (bFailed) {
       pFailedCount[ci]++; rv = -1;
     } else if (bDone == 0 && ddr_iter != 0) {
       pTimedOutCount[ci]++; rv = -1;
     } else {
       pPassedCount[ci]++;
     }
   }

   return rv;
 }

/* show results for each iteration */
int siriusDiagsDDRDumpCIResults(int unit,
				int ci_start,
				int ci_end,
				uint64_t iter_count,
				uint64_t *pFailedCount,
				uint64_t *pTimedOutCount,
				uint64_t *pPassedCount) {

  int ci = 0;
  int i = 0;
  uint32_t uFailedAddr=0;
  uint32_t uFailedData[8] = {0};
  uint32_t uData = 0;
  uint8 bFailed = 0;

  /* print results for each iteration */
  for(ci=ci_start;ci<ci_end;ci++) {
    if (iter_count != -1 ) {
      printf("Iteration:%lld CI%d Fail:%lld Pass:%lld Timedout:%lld\n",iter_count,ci,pFailedCount[ci],pPassedCount[ci],pTimedOutCount[ci]);
    } else { /* running continuously */
      printf("CI%d Fail:%lld Pass:%lld \n",ci,pFailedCount[ci],pPassedCount[ci]);
    }
    /* if this CI interface failed, print the failing addr and data */
    DIAG_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit,ci,&uData));
    bFailed = soc_reg_field_get(unit,CI_DDR_TESTr,uData,RAM_TEST_FAILf);    
    if (bFailed) {
      DIAG_IF_ERROR_RETURN(READ_CI_FAILED_ADDRr(unit,ci,&uFailedAddr));
      DIAG_IF_ERROR_RETURN(READ_CI_FAILED_DATA0r(unit,ci,&uFailedData[0]));
      DIAG_IF_ERROR_RETURN(READ_CI_FAILED_DATA1r(unit,ci,&uFailedData[1]));
      DIAG_IF_ERROR_RETURN(READ_CI_FAILED_DATA2r(unit,ci,&uFailedData[2]));
      DIAG_IF_ERROR_RETURN(READ_CI_FAILED_DATA3r(unit,ci,&uFailedData[3]));
      DIAG_IF_ERROR_RETURN(READ_CI_FAILED_DATA4r(unit,ci,&uFailedData[4]));
      DIAG_IF_ERROR_RETURN(READ_CI_FAILED_DATA5r(unit,ci,&uFailedData[5]));
      DIAG_IF_ERROR_RETURN(READ_CI_FAILED_DATA6r(unit,ci,&uFailedData[6]));
      DIAG_IF_ERROR_RETURN(READ_CI_FAILED_DATA7r(unit,ci,&uFailedData[7]));
      printf("CI%d Failing address = (0x%08x)\n",ci,uFailedAddr);
      for (i=0;i<8;i++) {
	if ( i == 4) printf("\n");
	printf("failing_data_%d = (0x%08x)  ",i,uFailedData[i]);
      }
      printf("\n");

      /* clear the failing status bit for this ci */
      DIAG_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit,ci,&uData));
      soc_reg_field_set(unit,CI_DDR_TESTr,&uData,RAM_TEST_FAILf,1); /* W1TC */
      DIAG_IF_ERROR_RETURN(WRITE_CI_DDR_TESTr(unit,ci,uData));
    }
  }
  return BCM_E_NONE;
}

/* see header file for description */
int siriusDiagsDDRIndirectPat(sbxDiagsInfo_t *pDiagsInfo) {
  int status = BCM_E_NONE;
  uint32_t ci;
  uint32_t bank;
  uint32_t row;
  uint32_t col;
  uint32_t data;

  status = siriusDiagsIndirectPatTest(pDiagsInfo,
				      &ci,
				      &bank,
				      &row,
				      &col,
				      &data);


  /*
   *  Check status, return error codes appropriately
   */

  if (status != 0) {
    if (status == -1) {
      return status; /* some bad configuration */
    } else if (status == CI_WRITE_TIMEOUT) {
      printf("Write timeout. CI%d, bank:%d, row:0x%x, start col:0x%x\n",
	     ci,bank,row,col);
      return status;
    } else if (status == CI_READ_TIMEOUT) {
      printf("Read timeout. CI%d, bank:%d, row:0x%x, start col:0x%x\n",
	     ci,bank,row,col);
      return status;
    } else if (status == CI_DATA_MISMATCH_ERROR) {
      printf("Data Mismatch Error(s) found. \n");
      return status;
    } else {
      printf("Unknown failure\n");
      return (-5);
    }
  }
  return status;
}

int siriusDiagsIndirectPatTest(sbxDiagsInfo_t *pDiagsInfo,
			       uint32_t *pCi,
			       uint32_t *pBank,
			       uint32_t *pRow,
			       uint32_t *pCol,
			       uint32_t *pData)

{

  uint32_t bank;
  uint32_t ci;
  uint32_t row, max_row;
  uint32_t col, max_col;
  uint32_t pla_addr;
  int i;
  siriusDiagsParams_t *pSiriusDiagParams = &(pDiagsInfo->siriusDiagParams); 
  uint32_t start_bank,end_bank;
  uint32_t ci_start,ci_end;
  int unit;
  uint32_t status = 0;
  uint32_t uDataWR[8] = {0};
  uint32_t uDataRD[8] = {0};
  uint8_t bUseAddrAsData = FALSE;
  uint32_t pat = 0;
  int data_mismatch = 0;
  int user_max_row = 0;

  unit = pDiagsInfo->unit;
  if (pSiriusDiagParams->bank == -1) {
    start_bank = 0;
    end_bank = 8;
  } else {
    start_bank = pSiriusDiagParams->bank & 0x7;
    end_bank = start_bank+1;
  }

  if (pSiriusDiagParams->ci_interface < 0) {
    ci_start = 0; ci_end = SOC_MAX_NUM_CI_BLKS;
  } else {
    ci_start = pSiriusDiagParams->ci_interface;
    ci_end = ci_start+1;
    printf("Testing only CI%d interface\n",ci_start);
  }
  
  /* if no pattern specified use the address as data */
  if (pDiagsInfo->pattern == -1) {
    bUseAddrAsData = TRUE;
  } else {
    pat = pDiagsInfo->pattern & 0xffffffff;
  }

  /* determine maximum row and col for the DDR */
  max_row = pSiriusDiagParams->ddr3_row;
  max_col = pSiriusDiagParams->ddr3_col;

  if (((max_row == 8192) && (max_col == 1024)) ||
      ((max_row == 16384) && (max_col == 1024)) ||
      ((max_row == 8192) && (max_col == 2048))) {
    /* supported memory config */
  } else {
    printf("Bad memory config\n");
    return (-1);
  }

  /* If max_row was specified use that */
  if (pSiriusDiagParams->max_row != -1) {
    user_max_row = pSiriusDiagParams->max_row;
    if (user_max_row > max_row ) {
      printf("Memory bank only supports (%d) rows\n",max_row);
      return (-1);
    } else {
      max_row = user_max_row;
    }
  }

  if (max_row == 0) {
    printf("Need max_row !=0 specify how many rows to test\n");
    return (-1);
  }

  /*
   * Write the pattern out to DDR
   */
  
  for(ci=ci_start;ci<ci_end;ci++) {
    for (bank=start_bank;bank<end_bank;bank++) {
      for(row=0;row<max_row;row++) {
	for (col=0;col<max_col;col++) {
	  /* build the pla_addr */
	  pla_addr = bank;
	  pla_addr |= col << 3;
	  pla_addr |= row << 10;
	  if (((max_col == 1024) && (pla_addr & (1<<9))) ||
	      ((max_col == 2048) && (pla_addr & (1<<10)))) {
	    break; /* went to the end of the col for this row */
	  }
	  for (i = 0; i < 8; i++ ) { 
	    if (bUseAddrAsData) {
	      uDataWR[i] = pla_addr;
	    } else {
	      uDataWR[i] = pat;
	    }
	  }
	  printf("Filling ci%d bank[%d],row[0x%x],cols[0x%x-0x%x] \n",
		 ci,bank,row,col<<4,(col<<4)+0xf);
	  status = siriusDDRWrite(unit,ci,pla_addr,uDataWR[0],uDataWR[1],uDataWR[2],
				  uDataWR[3],uDataWR[4],uDataWR[5],uDataWR[6],
				  uDataWR[7]); 
	  if (status != 0) {
	    *pCi = ci;
	    *pBank = bank;
	    *pRow = row;
	    *pCol = col;
	    *pData = pla_addr;
	    return status;
	  }
	} 
      }
    } 

  /*
   * Read back and compare, verify each CI's DDR Memory
   */

    for (bank=start_bank;bank<end_bank;bank++) {
      for(row=0;row<max_row;row++) {
	for (col=0;col<max_col;col++) {
	  /* build the pla_addr */
	  pla_addr = bank;
	  pla_addr |= col << 3;
	  pla_addr |= row << 10;
	  if (((max_col == 1024) && (pla_addr & (1<<9))) ||
	      ((max_col == 2048) && (pla_addr & (1<<10)))) {
	    break; /* went to the end of the col for this row */
	  }
	  printf("Verifing ci%d bank[%d],row[0x%x],cols[0x%x-0x%x] \n",
		 ci,bank,row,col<<4,(col<<4)+0xf);
	  status = siriusDDRRead(unit,ci,pla_addr,&uDataRD[0],&uDataRD[1],&uDataRD[2],
				 &uDataRD[3],&uDataRD[4],&uDataRD[5],&uDataRD[6],
				 &uDataRD[7]); 
	  if (status != 0) {
	    *pCi = ci;
	    *pBank = bank;
	    *pRow = row;
	    *pCol = col;
	    return status;
	  }
	  
	  /* check the data */
	  for (i = 0; i < 8; i++ ) {
	    if (bUseAddrAsData) {
	      if (uDataRD[i] != pla_addr) {
		printf("Data compare failure at pla_addr:(0x%x)\n",pla_addr);
		printf("Expected (0x%x) got (0x%x)\n",
		       pla_addr,uDataRD[i]);
		data_mismatch = 1;
	      }
	    } else {
	      if (uDataRD[i] != pat) {
		printf("Data compare failure at pla_addr:(0x%x)\n",pla_addr);
		printf("Expected (0x%x) got (0x%x)\n",
		       pat,uDataRD[i]);
		data_mismatch = 1;
	      }
	    }

	    if (data_mismatch != 0 ) {
	      *pCi = ci;
	      *pBank = bank;
	      *pRow = row;
	      *pCol = col;
	      status = CI_DATA_MISMATCH_ERROR;
	    }
	  }
	} 
      }
    } 
    
    /* clear all memory so next CI DDR we write is the only DDR
     * that contains data */
#if 0
    status = soc_sbx_sirius_ddr23_clear(unit);
    if (status !=0 ) {
      return status;
    }
#endif

  } /* ci */

  return status;
}

int siriusDDRRead(int unit, int ci, uint32_t addr, uint32_t *pData0,
		  uint32_t *pData1, uint32_t *pData2, uint32_t *pData3,
		  uint32_t *pData4, uint32_t *pData5, uint32_t *pData6,
		  uint32_t *pData7) {

  uint32_t uStatus = BCM_E_NONE;
  uint32 uCmd;

  uCmd = 0;
  soc_reg_field_set(unit,CI_MEM_ACC_CTRLr,&uCmd,MEM_ACC_ACKf,1);
  soc_reg_field_set(unit,CI_MEM_ACC_CTRLr,&uCmd,MEM_ACC_REQf,1);
  soc_reg_field_set(unit,CI_MEM_ACC_CTRLr,&uCmd,MEM_ACC_RD_WR_Nf,1);
  soc_reg_field_set(unit,CI_MEM_ACC_CTRLr,&uCmd,MEM_ACC_ADDRf,addr);

  DIAG_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_CTRLr(unit,ci,uCmd));

  if (siriusDiagsWrRdComplete(unit,
			      ci,
			      SIRIUS_MEM_TIMEOUT) != TRUE) {
    printf("Error timeout reading from CI:%d addr:0x%x\n",ci,addr);
    return CI_READ_TIMEOUT;
  }

  DIAG_IF_ERROR_RETURN(READ_CI_MEM_ACC_DATA0r(unit,ci,pData0));
  DIAG_IF_ERROR_RETURN(READ_CI_MEM_ACC_DATA1r(unit,ci,pData1));
  DIAG_IF_ERROR_RETURN(READ_CI_MEM_ACC_DATA2r(unit,ci,pData2));
  DIAG_IF_ERROR_RETURN(READ_CI_MEM_ACC_DATA3r(unit,ci,pData3));
  DIAG_IF_ERROR_RETURN(READ_CI_MEM_ACC_DATA4r(unit,ci,pData4));
  DIAG_IF_ERROR_RETURN(READ_CI_MEM_ACC_DATA5r(unit,ci,pData5));
  DIAG_IF_ERROR_RETURN(READ_CI_MEM_ACC_DATA6r(unit,ci,pData6));
  DIAG_IF_ERROR_RETURN(READ_CI_MEM_ACC_DATA7r(unit,ci,pData7));

  return uStatus;
}

int siriusDDRWrite(int unit, int ci, uint32_t addr, uint32_t uData0,
		   uint32_t uData1, uint32_t uData2, uint32_t uData3,
		   uint32_t uData4, uint32_t uData5, uint32_t uData6,
		   uint32_t uData7) {

  uint32_t uStatus = BCM_E_NONE;
  uint32 uCmd;

  /* setup the data */
  DIAG_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA0r(unit,ci,uData0));
  DIAG_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA1r(unit,ci,uData1));
  DIAG_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA2r(unit,ci,uData2));
  DIAG_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA3r(unit,ci,uData3));
  DIAG_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA4r(unit,ci,uData4));
  DIAG_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA5r(unit,ci,uData5));
  DIAG_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA6r(unit,ci,uData6));
  DIAG_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA7r(unit,ci,uData7));

  uCmd = 0;
  soc_reg_field_set(unit,CI_MEM_ACC_CTRLr,&uCmd,MEM_ACC_ACKf,1);
  soc_reg_field_set(unit,CI_MEM_ACC_CTRLr,&uCmd,MEM_ACC_REQf,1);
  soc_reg_field_set(unit,CI_MEM_ACC_CTRLr,&uCmd,MEM_ACC_RD_WR_Nf,0);
  soc_reg_field_set(unit,CI_MEM_ACC_CTRLr,&uCmd,MEM_ACC_ADDRf,addr);

  DIAG_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_CTRLr(unit,ci,uCmd));

  if (siriusDiagsWrRdComplete(unit,
			      ci,
			      SIRIUS_MEM_TIMEOUT) != TRUE) {
    printf("Error timeout writing to CI:%d addr:0x%x\n",ci,addr);
    return CI_WRITE_TIMEOUT;
  }
  return uStatus;
}

sbBool_t siriusDiagsWrRdComplete(int unit,
				 int ci,
				 uint32 uTimeout)
{
  uint32 i;
  uint32 data = 0;
  uint32 uAck = 0;

  /* wait for the ACK to indicate rd/wr op is finished */
  for(i = 0; i < uTimeout; i++ ) {
    DIAG_IF_ERROR_RETURN(READ_CI_MEM_ACC_CTRLr(unit,ci,&data));  
    uAck = soc_reg_field_get(unit,CI_MEM_ACC_CTRLr,data,MEM_ACC_ACKf);
    if (uAck) {
      return TRUE;
    }
    sal_udelay(10);
  }
  
  /* timed out */
  return ( FALSE );
}
			    

void sigcatcher_sirius(void)
{
  printf("\n");
  gStopDDRTest = TRUE;
  return;
}

#endif /* BCM_SIRIUS_SUPPORT */
