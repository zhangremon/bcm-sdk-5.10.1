/*
 * $Id: ddrtest.c 1.11.2.2 Broadcom SDK $
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
#include <sal/types.h>

#include <sal/core/libc.h>
#include <sal/core/boot.h>
#include <sal/appl/sal.h>
#include <sal/appl/io.h>
#include <sal/appl/pci.h>
#include <sal/appl/config.h>

#include <appl/diag/system.h>
#include <appl/diag/parse.h>
#include <appl/diag/progress.h>
#ifndef __KERNEL__
#include <signal.h>
#endif
#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/filter.h>
#include <soc/l2x.h>
#include <soc/esw_ddr40.h>

#include "testlist.h"

#ifdef BCM_DDR3_SUPPORT

#define CI_DDR_MASK 0x3fffff /* [21:0] bit field for writes */

typedef enum _DDRMemTests {
  DDR_ALL_TESTS = 0,
  DDR_STANDARD_TEST = 1,
  DATA_BUS_WALKING_ONES = 2,
  DATA_BUS_WALKING_ZEROS = 3,
  DDR_DATA_EQ_ADDR = 4,
  DDR_INDIRECT_TEST = 5,
  /* leave as last */
  MEM_TEST_LAST
} DDRMemTests_t;

const char* DDRMemTestStr[] = {"All DDR tests",
                        "DDR Standard burst",
                        "DDR walking ones on data bus",
                        "DDR walking zeros on data bus",
                        "DDR Data == Address",
                        "DDR Indirect pattern"};

typedef struct ddr_test_params_s {
    int unit;
    DDRMemTests_t ddr_test_mode;
    int32 pattern;
    int ci_interface;
    int ddr_start_addr;
    int ddr_step_addr;
    int ddr_burst;
    int ddr_iter;
    int bank;
    int max_row;
    int max_col;
} ddr_test_params_t;

static ddr_test_params_t      *ddr_test_params[SOC_MAX_NUM_DEVICES];
static uint8 stopDDRTest = FALSE;

char ddr_test_usage[] = 
"DDR Memory Test Usage:\n"
#ifdef COMPILER_STRING_CONST_LIMIT
"MemTest=<0,1,2,3,4> <args>\n"
#else
"MemTest=<0,1,2,3,4> where:\n"
"   0 - All Standard tests\n"
"   1 - Standard DDR burst write/read test\n"
"   2 - Walking ones on data bus\n"
"   3 - Walking zeros on data bus\n"
"   4 - DDR DATA == ADDRESS test \n"
"   5 - Indirect Memory Test\n"
"                            \n"
" Options for Standard DDR test: \n"
"  StartAddr=<0xvalue>     specifies starting memory address\n"
"  AddrStepInc=<0xvalue>   specifies address step \n"
"  BurstSize=<value>       number of burst to write, followed by same number of reads\n"
"  Pattern=<0xvalue>       memory test pattern \n"
"                                             \n"
" Options for all tests except indirect:      \n"
"  Iterations=<value>      how many times to run test, defaults to one\n"
"                         note: iterations=0 will run test continuously \n"
"  CI=<value>              CI interface to run test on, default is all of them\n"
"                                             \n"
" Indirect Memory Test options:\n"
"  Pattern=<0xvalue> if not specifed address will be used as data \n"
"  Bank=<value>      specifies bank within DDR to test (0-7)\n"
"  MaxRow=<value>    specified the maximum row within the bank to test \n"
"  MaxCol=<value>    specified the maximum column within the bank to test \n"
"  CI=<value>        what ci interface to run test on \n"
"                                             \n"
"NOTE: If MemTest is not specified or is set to a value of 0,\n"
"      then all tests (except indirect) are run\n"
#endif
;


/* Handle Ctrl-C to stop continuous test */
void sigcatcher_ddr(int sig)
{
    printk("\n");
    stopDDRTest = TRUE;
    return;
}

int
ddr_test_init(int unit, args_t *a, void **p)
{
    ddr_test_params_t *dt;
    int mem_test;
    parse_table_t pt;
    int rv = -1;
    int ddr_step_addr;
    int ddr_start_addr;
    int ddr_test_count;
    int ddr_burst_size;
    int ci_interface;
    int bank, max_row, max_col;
#ifndef NO_SAL_APPL
    char *pattern = "";
#endif

    dt = ddr_test_params[unit];
    if (dt == NULL) {
        dt = sal_alloc(sizeof(ddr_test_params_t), "ddrtest");
        if (dt == NULL) {
            debugk(DK_ERR, "%s: cannot allocate memory test data\n", ARG_CMD(a));
            return -1;
        }
        sal_memset(dt, 0, sizeof(ddr_test_params_t));
        ddr_test_params[unit] = dt;
    }

    parse_table_init(unit, &pt);

    parse_table_add(&pt, "MemTest", PQ_INT, (void *) 0,
          &mem_test, NULL);
    parse_table_add(&pt, "StartAddr", PQ_INT, (void*) 0,
            &ddr_start_addr, NULL);
    parse_table_add(&pt, "AddrStepInc", PQ_INT, (void *) 1,
            &ddr_step_addr, NULL);
    parse_table_add(&pt, "Burst_Size", PQ_INT, (void *) 256,
            &ddr_burst_size, NULL);
#ifndef NO_SAL_APPL
    parse_table_add(&pt, "Pattern", PQ_STRING, "-1",
          &pattern, NULL);
#endif
    parse_table_add(&pt, "Iterations", PQ_INT, (void *) 1,
            &ddr_test_count, NULL);
    parse_table_add(&pt, "CI", PQ_INT, (void *) (-1),
            &ci_interface, NULL);
    parse_table_add(&pt, "Bank", PQ_INT, (void *) (-1),
            &bank, NULL);
    parse_table_add(&pt, "MaxRow", PQ_INT, INT_TO_PTR(SOC_DDR3_NUM_ROWS(unit)),
            &max_row, NULL);
    parse_table_add(&pt, "MaxCol", PQ_INT, INT_TO_PTR(SOC_DDR3_NUM_COLUMNS(unit)),
            &max_col, NULL);

    if (parse_arg_eq(a, &pt) < 0) {
        printk("%s: Invalid option: %s\n",
               ARG_CMD(a), ARG_CUR(a));
        printk("%s\n",ddr_test_usage);
        goto done;
    }

    if (ARG_CNT(a) != 0) {
        printk("%s: extra options starting with \"%s\"\n",
               ARG_CMD(a), ARG_CUR(a));
        printk("%s\n",ddr_test_usage);
        goto done;
    }

    dt->unit = unit;
#ifndef NO_SAL_APPL
    sscanf(pattern,"%d", &dt->pattern);
#else
    dt->pattern = -1;
#endif
    dt->ci_interface = ci_interface;
    dt->ddr_start_addr = ddr_start_addr;
    dt->ddr_step_addr = ddr_step_addr;
    dt->ddr_burst = ddr_burst_size;
    dt->ddr_iter = ddr_test_count;
    dt->ddr_test_mode = mem_test;
    dt->bank = bank;
    dt->max_row = max_row;
    dt->max_col = max_col;
    *p = dt;
    rv = 0;

    done:
       parse_arg_eq_done(&pt);
       return rv;
}



int StartDDRFunctionalTest(ddr_test_params_t *testparams,
                            int ci_start,
                            int ci_end,
                            DDRMemTests_t mode) {

    uint32 uData, ddr_iter;
    int ci, unit;
    ddr_test_params_t *dt = testparams;

    ddr_iter = dt->ddr_iter & CI_DDR_MASK;
    unit = dt->unit;

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
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_ITERr(unit, ci, ddr_iter));
        SOC_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit, ci, &uData));
        soc_reg_field_set(unit, CI_DDR_TESTr, &uData, MODEf,(mode-1));
        soc_reg_field_set(unit, CI_DDR_TESTr, &uData, RAM_DONEf, 1);      /* W1TC */
        soc_reg_field_set(unit, CI_DDR_TESTr, &uData, RAM_TESTf, 0);      /* clear */
        soc_reg_field_set(unit, CI_DDR_TESTr, &uData, RAM_TEST_FAILf, 1); /* W1TC */
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TESTr(unit, ci, uData));

        /* set ram_test - to start the test */
        SOC_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit, ci, &uData));
        soc_reg_field_set(unit, CI_DDR_TESTr, &uData, RAM_TESTf, 1);
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TESTr(unit, ci, uData));
    }

  return BCM_E_NONE;
}


int CheckDDRFunctionalTest(ddr_test_params_t *testparams,
                            int ci_start, int ci_end,
                            uint64 *pFailedCount,
                            uint64 *pTimedOutCount,
                            uint64 *pPassedCount) {
    int unit, ci, rv = BCM_E_NONE;
    uint32 ddr_iter, uData = 0;
    uint8 bFailed = 0;
    uint8 bDone = 0;
    ddr_test_params_t *dt = testparams;

    unit = dt->unit;
    ddr_iter = dt->ddr_iter & CI_DDR_MASK;

    for (ci = ci_start; ci < ci_end; ci++) {
        SOC_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit, ci, &uData));
        bFailed = soc_reg_field_get(unit,CI_DDR_TESTr, uData, RAM_TEST_FAILf);
        bDone = soc_reg_field_get(unit, CI_DDR_TESTr, uData, RAM_DONEf);
        if (bFailed) {
          COMPILER_64_ADD_32(pFailedCount[ci], 1); rv = -1;
        } else if (bDone == 0 && ddr_iter != 0) {
          COMPILER_64_ADD_32(pTimedOutCount[ci], 1); rv = -1;
        } else {
          COMPILER_64_ADD_32(pPassedCount[ci], 1);
        }
    }
    return rv;
}


/* show results for each iteration */
int DumpDDRResults(int unit,
                    int ci_start, int ci_end,
                    int iter_count,
                    uint64 *pFailedCount,
                    uint64 *pTimedOutCount,
                    uint64 *pPassedCount) {
    int i, ci;
    uint32 uFailedAddr=0;
    uint32 uFailedData[8] = {0};
    uint32 uData = 0;
    uint8 bFailed = 0;

    /* print results for each iteration */
    for(ci = ci_start; ci < ci_end; ci++) {
        if (iter_count != -1 ) {
            debugk(DK_TESTS, "Iteration:%d CI%d Fail:0x%08x%08x Pass:0x%08x%08x Timedout:0x%08x%08x\n",
                            iter_count,ci,
                            COMPILER_64_HI(pFailedCount[ci]),COMPILER_64_LO(pFailedCount[ci]),
                            COMPILER_64_HI(pPassedCount[ci]),COMPILER_64_LO(pPassedCount[ci]),
                            COMPILER_64_HI(pTimedOutCount[ci]),COMPILER_64_LO(pTimedOutCount[ci]));
        } else { /* running continuously */
            debugk(DK_TESTS, "CI%d Fail:0x%08x%08x Pass:0x%08x%08x \n",ci,
                            COMPILER_64_HI(pFailedCount[ci]),COMPILER_64_LO(pFailedCount[ci]),
                            COMPILER_64_HI(pPassedCount[ci]),COMPILER_64_LO(pPassedCount[ci]));
        }
        /* if this CI interface failed, print the failing addr and data */
        SOC_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit, ci, &uData));
        bFailed = soc_reg_field_get(unit, CI_DDR_TESTr, uData, RAM_TEST_FAILf);
        if (bFailed) {
            SOC_IF_ERROR_RETURN(READ_CI_FAILED_ADDRr(unit,ci,&uFailedAddr));
            SOC_IF_ERROR_RETURN(READ_CI_DDR_TEST_FAILED_DATA0r(unit,ci,&uFailedData[0]));
            SOC_IF_ERROR_RETURN(READ_CI_DDR_TEST_FAILED_DATA1r(unit,ci,&uFailedData[1]));
            SOC_IF_ERROR_RETURN(READ_CI_DDR_TEST_FAILED_DATA2r(unit,ci,&uFailedData[2]));
            SOC_IF_ERROR_RETURN(READ_CI_DDR_TEST_FAILED_DATA3r(unit,ci,&uFailedData[3]));
            SOC_IF_ERROR_RETURN(READ_CI_DDR_TEST_FAILED_DATA4r(unit,ci,&uFailedData[4]));
            SOC_IF_ERROR_RETURN(READ_CI_DDR_TEST_FAILED_DATA5r(unit,ci,&uFailedData[5]));
            SOC_IF_ERROR_RETURN(READ_CI_DDR_TEST_FAILED_DATA6r(unit,ci,&uFailedData[6]));
            SOC_IF_ERROR_RETURN(READ_CI_DDR_TEST_FAILED_DATA7r(unit,ci,&uFailedData[7]));
            printk("CI%d Failing address = (0x%08x)\n",ci,uFailedAddr);
            for (i = 0; i < 8; i++) {
                if ( i == 4) printk("\n");
                printk("failing_data_%d = (0x%08x)  ", i, uFailedData[i]);
            }
            printk("\n");

            /* clear the failing status bit for this ci */
            SOC_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit, ci, &uData));
            soc_reg_field_set(unit, CI_DDR_TESTr, &uData, RAM_TEST_FAILf, 1); /* W1TC */
            SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TESTr(unit, ci, uData));
        }
    }
    return BCM_E_NONE;
}


/* run single DDR test */
int DDRTest(ddr_test_params_t *testparams) {
    ddr_test_params_t *dt = testparams;
    DDRMemTests_t ddr_test;
    int rv, unit, iter_count, ci, ci_start, ci_end, stat=0;
    uint32 uData=0, pattern, uDDRIter, uStartAddr, uStepSize, uDDRBurstSize;
    uint64 uFailedCount[SOC_MAX_NUM_CI_BLKS];
    uint64 uPassedCount[SOC_MAX_NUM_CI_BLKS];
    uint64 uTimedOutCount[SOC_MAX_NUM_CI_BLKS];

#ifndef __KERNEL__
    signal(SIGINT, sigcatcher_ddr);
#endif
    stopDDRTest = FALSE;

    for (ci = 0; ci < SOC_MAX_NUM_CI_BLKS; ci++) {
        COMPILER_64_ZERO(uFailedCount[ci]);
        COMPILER_64_ZERO(uPassedCount[ci]);
        COMPILER_64_ZERO(uTimedOutCount[ci]);
    }

    if (dt->ci_interface < 0) {
        /* test all of them */
        ci_start = 0; ci_end = 3; 
    } else {
        ci_start = dt->ci_interface;
        ci_end = ci_start+1;
        debugk(DK_WARN, "Testing only CI%d interface\n",ci_start);
    }
    unit = dt->unit;
    ddr_test = dt->ddr_test_mode;
    uDDRIter = dt->ddr_iter & CI_DDR_MASK;

    debugk(DK_TESTS, "Running Test %s\n", DDRMemTestStr[ddr_test]);

    if (dt->pattern != -1 ) {
        pattern = dt->pattern & 0xffffffff;
        if (DDR_STANDARD_TEST != ddr_test) {
            debugk(DK_WARN, "NOTE: Test data is not used during data bus test\n");
        }
    } else {
        /* no pattern specified use alternating 0101.. pattern */
        pattern = 0x55555555;
    }

    uStartAddr = dt->ddr_start_addr & CI_DDR_MASK;
    uStepSize = dt->ddr_step_addr & CI_DDR_MASK;
    uDDRBurstSize = dt->ddr_burst & CI_DDR_MASK;

    if (DDR_STANDARD_TEST == ddr_test ||
        DDR_DATA_EQ_ADDR == ddr_test) {
        debugk(DK_TESTS, "start_addr=0x%x,addr_step_inc=0x%x,burst_size=%d\n",uStartAddr,uStepSize,uDDRBurstSize);
        if (DDR_STANDARD_TEST == ddr_test) {
            debugk(DK_TESTS, "using pattern=0x%8x, and alt_pattern=0x%8x\n",pattern,~pattern);
        }
    }

    for (ci = ci_start; ci < ci_end; ci++) {
      /* set test specific attributes */
      /* SOC_IF_ERROR_RETURN(WRITE_CI_DEBUGr(unit,ci,0x1)); ? inject auto refresh */
      SOC_IF_ERROR_RETURN(WRITE_CI_DDR_STARTr(unit,ci,uStartAddr));
      SOC_IF_ERROR_RETURN(WRITE_CI_DDR_STEPr(unit,ci,uStepSize));
      SOC_IF_ERROR_RETURN(WRITE_CI_DDR_BURSTr(unit,ci,uDDRBurstSize));
      /* only STANDARD_DDR test uses pattern data */
      if (DDR_STANDARD_TEST == ddr_test) {
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_DATA0r(unit,ci,pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_DATA1r(unit,ci,pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_DATA2r(unit,ci,pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_DATA3r(unit,ci,pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_DATA4r(unit,ci,pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_DATA5r(unit,ci,pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_DATA6r(unit,ci,pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_DATA7r(unit,ci,pattern));
        /* set the alt_data */
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_ALT_DATA0r(unit,ci,~pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_ALT_DATA1r(unit,ci,~pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_ALT_DATA2r(unit,ci,~pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_ALT_DATA3r(unit,ci,~pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_ALT_DATA4r(unit,ci,~pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_ALT_DATA5r(unit,ci,~pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_ALT_DATA6r(unit,ci,~pattern));
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TEST_ALT_DATA7r(unit,ci,~pattern));
        SOC_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit,ci,&uData));
      }
    }

    /* Run the particular test uDDRIter times */
    for(iter_count = 0; iter_count < uDDRIter; iter_count++) {
        /* start the test for this iteration */
        if (( rv = StartDDRFunctionalTest(dt, ci_start,
                                          ci_end, ddr_test)) < 0 ) {
            debugk(DK_ERR, "%s failed to start %s test.\n",SOC_CHIP_STRING(unit),
                  DDRMemTestStr[ddr_test]);
            return rv;
        }
        /* wait long enough for all ci's being tested to be done */
        sal_sleep(1);

        /* check the test results for this iteration */
        if (( rv = CheckDDRFunctionalTest(dt,
                                            ci_start, ci_end,
                                            uFailedCount,
                                            uTimedOutCount,
                                            uPassedCount)) < 0 ) {
          
            debugk(DK_ERR, "%s failed %s test.\n",SOC_CHIP_STRING(unit),
                    DDRMemTestStr[ddr_test]);

            /* one or more CI's failed
            * dump register debug info here
            */
            stat = -1;
        }

        /* display the results for this iteration */
        if (( rv = DumpDDRResults(unit,
                                    ci_start, ci_end,
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
        if (( rv = StartDDRFunctionalTest(dt, ci_start,
                                          ci_end, ddr_test)) < 0 ) {
            debugk(DK_ERR, "%s failed to start %s test.\n",SOC_CHIP_STRING(unit),
                  DDRMemTestStr[ddr_test]);
            return rv;
        }
        printk("Running test continuously CTRL-C to STOP.. \n");
        while(1) {
            if (stopDDRTest == TRUE) {
                /* stop the test by turning off ram_test */
                printk("Stopping Test.\n");
                for (ci = ci_start; ci < ci_end; ci++) {
                    SOC_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit, ci, &uData));
                    soc_reg_field_set(unit, CI_DDR_TESTr, &uData, RAM_TESTf, 0);
                    SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TESTr(unit, ci, uData));
                }
                break;
            }

            /* check results at some interval */
            sal_sleep(2);

            if (( rv = CheckDDRFunctionalTest(dt,
                                                ci_start, ci_end,
                                                uFailedCount,
                                                uTimedOutCount,
                                                uPassedCount)) < 0 ) {
              
                debugk(DK_ERR, "%s failed %s test.\n",SOC_CHIP_STRING(unit),
                        DDRMemTestStr[ddr_test]);
            
                /* one or more CI's failed
                * dump register debug info here
                */
                stat = -1;
            }

            /* display results while test is running */
            if (( rv = DumpDDRResults(unit,
                                        ci_start, ci_end,
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
        SOC_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit,ci,&uData));
        soc_reg_field_set(unit,CI_DDR_TESTr,&uData,RAM_DONEf,1);      /* W1TC */
        soc_reg_field_set(unit,CI_DDR_TESTr,&uData,RAM_TESTf,0);      /* clear */
        soc_reg_field_set(unit,CI_DDR_TESTr,&uData,RAM_TEST_FAILf,1); /* W1TC */
        SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TESTr(unit,ci,uData));
    }
    
    return stat;
}

/* run DDR Indirect Pattern test */
int DDRIndirectTest(ddr_test_params_t *testparams) {
    ddr_test_params_t *dt = testparams;
    uint8 bUseAddrAsData = FALSE;
    uint32 uDataWR[8] = {0};
    uint32 uDataRD[8] = {0};
    uint32 pla_addr, comp_data, pattern=0;
    uint32 row, max_row, col, max_col;
    int i, unit, bank, start_bank, end_bank, ci, ci_start, ci_end, stat;

    unit = dt->unit;

    if (dt->bank == -1) {
      start_bank = 0;
      end_bank = 8;
    } else {
      start_bank = dt->bank & 0x7;
      end_bank = start_bank+1;
    }
    
    if (dt->ci_interface < 0) {
      ci_start = 0; ci_end = SOC_DDR3_NUM_MEMORIES(unit);
    } else {
      ci_start = dt->ci_interface;
      ci_end = ci_start+1;
      debugk(DK_WARN, "Testing only CI%d interface\n",ci_start);
    }
    
    /* if no pattern specified use the address as data */
    if (dt->pattern == -1) {
      bUseAddrAsData = TRUE;
    } else {
      pattern = dt->pattern & 0xffffffff;
    }

    max_row = (dt->max_row <= SOC_DDR3_NUM_ROWS(unit)) ?
                            dt->max_row : SOC_DDR3_NUM_ROWS(unit);
    max_col = (dt->max_col <= SOC_DDR3_NUM_COLUMNS(unit)) ?
                            dt->max_col : SOC_DDR3_NUM_COLUMNS(unit);

    for(ci=ci_start;ci<ci_end;ci++) {
        debugk(DK_TESTS, "Filling ci%d\n", ci);
        for (bank=start_bank;bank<end_bank;bank++) {
            for(row=0;row<max_row;row++) {
                for (col=0;col<max_col;col++) {
                    pla_addr = bank;
                    pla_addr |= col << 3;
                    pla_addr |= row << 9;
                    for (i = 0; i < 8; i++ ) { 
                        if (bUseAddrAsData) {
                            uDataWR[i] = pla_addr;
                        } else {
                            uDataWR[i] = pattern;
                        }
                    }
                    debugk(DK_TESTS, "Filling ci%d bank[%d],row[0x%x],cols[0x%x-0x%x] \n",
                            ci,bank,row,col<<4,(col<<4)+0xf);

                    stat = soc_ddr40_write(unit,ci,pla_addr,uDataWR[0],uDataWR[1],uDataWR[2],
                             uDataWR[3],uDataWR[4],uDataWR[5],uDataWR[6], uDataWR[7]);
                    if (0 != stat) {
                        /* Write Error */
                        return stat;
                    }
                }
            }
        }
    }

    /* Read back and compare, verify each CI's DDR Memory */
    for(ci=ci_start;ci<ci_end;ci++) {
        debugk(DK_TESTS, "Verifying ci%d\n", ci);
        for (bank=start_bank;bank<end_bank;bank++) {
            for(row=0;row<max_row;row++) {
                for (col=0;col<max_col;col++) {
                    pla_addr = bank;
                    pla_addr |= col << 3;
                    pla_addr |= row << 9;
                    debugk(DK_TESTS, "Verifing ci%d bank[%d],row[0x%x],cols[0x%x-0x%x] \n",
                            ci,bank,row,col<<4,(col<<4)+0xf);
                    stat = soc_ddr40_read(unit,ci,pla_addr,&uDataRD[0],&uDataRD[1],&uDataRD[2],
                             &uDataRD[3],&uDataRD[4],&uDataRD[5],&uDataRD[6],&uDataRD[7]); 
                    if (0 != stat) {
                        /* Read Error */
                        return stat;
                    }
                    /* check the data */
                    for (i = 0; i < 8; i++ ) {
                        comp_data = bUseAddrAsData ? pla_addr : pattern;
                        if (uDataRD[i] != comp_data) {
                            debugk(DK_ERR, "Data compare failure at pla_addr:(0x%x)\n",pla_addr);
                            debugk(DK_ERR, "Expected (0x%x) got (0x%x)\n",
                                       comp_data,uDataRD[i]);
                            /* Compare Error */
                            return -1;
                        }
                    }
                }
            }
        }
    }

    return 0;
}

int
ddr_test(int unit, args_t *a, void *p)
{
    ddr_test_params_t *dt = p;
    DDRMemTests_t ddr_test = dt->ddr_test_mode;
    int mem_test, rv = 0;

    if (DDR_ALL_TESTS == ddr_test) {
        /* All Tests */
        for(mem_test = 1; mem_test < MEM_TEST_LAST; mem_test++) {
            if (DDR_INDIRECT_TEST == mem_test) {
                /* skip the indirect test -- takes too long */
                continue;
            }
            dt->ddr_test_mode = mem_test;
            if (DDRTest(dt) != 0) {
                rv = -1;
            }
        }
        return rv;
    } else if (DDR_INDIRECT_TEST == ddr_test) {
        return DDRIndirectTest(dt);
    } else {
        return DDRTest(dt);
    }
}

int
ddr_test_done(int unit, void *p)
{
    debugk(DK_TESTS, "DDR Mem Tests Done\n");
    return 0;
}

#endif /* BCM_DDR3_SUPPORT */
