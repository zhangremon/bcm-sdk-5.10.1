/*
 * $Id: sbx_tests.c 1.49.20.1 Broadcom SDK $
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
 */
#include <soc/defs.h>

#ifdef BCM_SBX_SUPPORT

#include <sal/appl/io.h>
#include <bcm/init.h>
#include <sal/appl/sal.h>
#include <appl/diag/test.h>
#include <appl/diag/debug.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_ka_auto.h>
#include <soc/sbx/sbx_drv.h>
#include <appl/test/sbx_diags.h>
#include <appl/test/fe2000_diags.h>
#include <appl/test/fe2kxt_diags.h>
#include <appl/test/bm9600_diags.h>
#include <appl/test/qe2000_diags.h>
#include <appl/diag/shell.h>
#include <appl/diag/system.h>
#include <appl/diag/dport.h>
#ifdef BCM_SIRIUS_SUPPORT
#include <appl/test/sirius_diags.h>
#endif

#ifdef BCM_SIRIUS_SUPPORT
char qe4000_ddr_test_usage[] = 
"Sirius DDR Memory Test Usage:\n"
"MemTest=<0,1,2,3,4> where:\n"
"   0 - Standard DDR burst write/read test\n"
"   1 - Walking ones on data bus\n"
"   2 - Walking zeros on data bus\n"
"   3 - DDR DATA == ADDRESS test \n"
"   4 - Indirect Memory Test\n"
"                            \n"
" Options for Standard DDR test: \n"
" start_addr=<0xvalue>    specifies starting memory address\n"
" addr_step_inc=<0xvalue> specifies address step \n"
" burst_size=<value>      number of burst to write, followed by same number of reads\n"
" pattern=<0xvalue>       memory test pattern \n"
"                                             \n"
" Options for all tests except indirect:      \n"
" iterations=<value>      how many times to run test, defaults to one\n"
"                         note: iterations=0 will run test continuously \n"
" ci=<value>              CI interface to run test on, default is all of them\n"
"                                             \n"
" Indirect Memory Test options:\n"
" pattern=<0xvalue> if not specifed address will be used as data \n"
" bank=<value>      specifies bank within DDR to test (0-7)\n"
" max_row=<value>   specified the maximum row within the bank to test \n"
" ci=<value>        what ci interface to run test on \n"
"                                             \n"
"NOTES:\n"
"If MemTest is not specified all tests (except indirect) are run\n"
  ;
#endif

typedef struct unit_prbs_param_s {
  int unit;            /* Which unit to test on */
  bcm_pbmp_t pbmp;
  int mode;            /* 0 == hc, 1 == si */
  int loopback;        /* loopback at phy or use ext loopbacks */
  int seconds;         /* specify seconds to run */
  int poly;            /* polynomial to use 0..3 */
} unit_prbs_param_t;

static unit_prbs_param_t     *prbs_param[SOC_MAX_NUM_DEVICES];

extern int sbx_qe_prbs_init(int, args_t *, void **);
extern int sbx_qe_prbs_test(int, args_t *, void *);
extern int sbx_qe_prbs_done(int, void *);

/* required for re-init */
extern int diag_rc_load(int);
extern cmd_result_t cmd_sbx_mcinit_config(int unit, args_t *orig_a);
STATIC sbxDiagsInfo_t diagsInfo[1];

#if defined(BCM_FE2000_SUPPORT) || defined(BCM_FE2000XT_SUPPORT)

int
sbx_linerate_mem_init(int unit, args_t *a, void **p)
{

  sbxDiagsInfo_t *pDiagsInfo = &diagsInfo[0];
  int verbose;
  parse_table_t pt;
  int bad_arg=0;
  int rv = -1;
  int seed;
  char *mem_exists_str;
  int dllphase = -1;
  int reinit = 0;
  
  parse_table_init(unit, &pt);

  parse_table_add(&pt,  "Verbose",PQ_INT,(void *) 0,
		  &verbose, NULL);
  parse_table_add(&pt,  "Seed",	PQ_INT, (void *) (-1),
		  &seed, NULL);
  parse_table_add(&pt, "MemExists", PQ_STRING, (void *)"0x3f",
		  &mem_exists_str, NULL); 
  parse_table_add(&pt, "reinit", PQ_INT, (void *)(1),
		  &reinit, NULL); 
  if (!SOC_IS_SBX_FE2KXT(unit)) {
    parse_table_add(&pt, "DLLPhase", PQ_INT, (void *) (-1),
		    &dllphase, NULL);  
  }

  sal_memset(pDiagsInfo,0,sizeof(*pDiagsInfo));
  /* consume valid arguments */
  if (parse_arg_eq(a, &pt) < 0 || ARG_CNT(a) != 0) {
    printk("%s: Invalid option: %s\n",
	   ARG_CMD(a), ARG_CUR(a));
    bad_arg=1;
    goto done;
  }

  if (ARG_CNT(a) != 0) {
    printk("%s: extra options starting with \"%s\" \n",
	   ARG_CMD(a), ARG_CUR(a));
    bad_arg=1;
    goto done;
  }

  pDiagsInfo->userDeviceHandle = SOC_SBX_CONTROL(unit)->sbhdl;
  pDiagsInfo->unit = unit;
  pDiagsInfo->debug_level = verbose;
  pDiagsInfo->seed = seed;
  pDiagsInfo->dllphase = dllphase;
  pDiagsInfo->reinit = reinit;

  sscanf(mem_exists_str,"%lx",&pDiagsInfo->uMemExistsMask);
  if (pDiagsInfo->uMemExistsMask == 0) {
    test_error(unit,"Specify non-zero MemExists Mask\n");
    goto done;
  }

  if (!SOC_SBX_INIT(unit)) {
    if (soc_sbx_init(unit) < 0) {
      test_error(unit,"SOC Layer initialization failed\n");
      goto done;
    }
  }

  /* retrieve params from config.bcm, defaults to fe2k ref platform */
  pDiagsInfo->wp_config.data_width = 
    SOC_SBX_CFG_FE2000(unit)->mm0wdwidth;
  pDiagsInfo->wp_config.addr_width = 
    SOC_SBX_CFG_FE2000(unit)->mm0wawidth;
  pDiagsInfo->np0_config.data_width = 
        SOC_SBX_CFG_FE2000(unit)->mm0n0dwidth;
  pDiagsInfo->np0_config.addr_width = 
    SOC_SBX_CFG_FE2000(unit)->mm0n0awidth;
  pDiagsInfo->np1_config.data_width = 
    SOC_SBX_CFG_FE2000(unit)->mm0n1dwidth;
  pDiagsInfo->np1_config.addr_width = 
    SOC_SBX_CFG_FE2000(unit)->mm0n1awidth;

  *p = pDiagsInfo;
  rv = 0;

done:
  if (bad_arg) {
    printk("valid test args:\n");
    printk("TR 116 MemExists=0x1  will test MM0.N0 only\n");
    printk("TR 116 MemExists=0x2  will test MM0.N1 only\n");
    printk("TR 116 MemExists=0x4  will test MM0.W only\n");
    printk("TR 116 MemExists=0x8  will test MM1.N0 only\n");
    printk("TR 116 MemExists=0x10 will test MM1.N1\n");
    printk("TR 116 MemExists=0x20 will test MM1.W\n");
    printk("Default is to test all memories\n");
    parse_eq_format(&pt);
  }
  parse_arg_eq_done(&pt);
  return rv;

}

int
sbx_linerate_mem_test(int u, args_t *a, void *p)
{

  int rv = 0;
  sbxDiagsInfo_t *pDiagsInfo = p;
  if (!SOC_IS_SBX_FE2KXT(u)) {
    rv = sbFe2000LineRateMemTestRun(pDiagsInfo);
  } else {
    rv = sbFe2kxtLineRateMemTestRun(pDiagsInfo);
  }
  return rv;
}
#endif /* defined(BCM_FE2000_SUPPORT) || defined(BCM_FE2000XT_SUPPORT) */

int
sbx_mem_init(int unit, args_t *a, void **p)
{


  sbxDiagsInfo_t *pDiagsInfo = &diagsInfo[0];
  int mem_test,seed,verbose;
  char *start_addr;
  char *end_addr;
  char *pattern = "";
  int loop_on_error;
  parse_table_t pt;
  int bad_arg=0;
  int rv = -1;
  int help = 0;
#ifdef BCM_SIRIUS_SUPPORT
  int ddr_step_addr;
  int ddr_start_addr;
  int ddr_test_count;
  int ddr_burst_size;
  int ci_interface;
  int bank;
  int max_row;
  siriusDiagsParams_t *pSiriusDiagParams = NULL;
#endif
  help = help; /* fool gcc */
  parse_table_init(unit, &pt);

  if (SOC_IS_SBX_FE2000(unit)) {

    parse_table_add(&pt,  "Seed",	PQ_INT, (void *) (-1),
		    &seed, NULL);
    parse_table_add(&pt, "StartAddr", PQ_STRING, (void *)"0x0",
		    &start_addr,NULL);
    parse_table_add(&pt, "EndAddr", PQ_STRING, (void *)"0x200000",  /* default to fe2k ref platform */
		    &end_addr,NULL);
  }

  parse_table_add(&pt,  "MemTest", PQ_INT, (void *) 0,
		  &mem_test, NULL);
  parse_table_add(&pt,  "Verbose",PQ_INT,(void *) 0,
		  &verbose, NULL);
  parse_table_add(&pt,  "LoopOnError",PQ_INT,(void *) 0,
		  &loop_on_error, NULL);
  parse_table_add(&pt, "Pattern", PQ_STRING, "-1",
		  &pattern, NULL);

#ifdef BCM_SIRIUS_SUPPORT
  if (SOC_IS_SIRIUS(unit)) {
    parse_table_add(&pt,"addr_step_inc",PQ_INT, (void *) 1,
		    &ddr_step_addr, NULL);
    parse_table_add(&pt,"burst_size",PQ_INT, (void *) 256,
		    &ddr_burst_size, NULL);
    parse_table_add(&pt,"iterations",PQ_INT, (void*) 1,
		    &ddr_test_count, NULL);
    parse_table_add(&pt,"start_addr",PQ_INT, (void*) 0,
		    &ddr_start_addr, NULL);
    parse_table_add(&pt,"ci",PQ_INT, (void*) (-1),
		    &ci_interface, NULL);
    parse_table_add(&pt,"bank",PQ_INT, (void*) (-1),
		    &bank, NULL);
    parse_table_add(&pt,"max_row",PQ_INT, (void*) (-1),
		    &max_row, NULL);
    parse_table_add(&pt,"help",PQ_INT, (void*) (0),
		    &help, NULL);
    parse_table_add(&pt,  "Seed",	PQ_INT, (void *) (-1),
		    &seed, NULL);
  }
#endif

  sal_memset(pDiagsInfo,0,sizeof(*pDiagsInfo));
  /* consume valid arguments */
  if (parse_arg_eq(a, &pt) < 0 || ARG_CNT(a) != 0) {
    printk("%s: Invalid option: %s\n",
	   ARG_CMD(a), ARG_CUR(a));
    bad_arg=1;
    goto done;
  }

  if (ARG_CNT(a) != 0) {
    printk("%s: extra options starting with \"%s\" \n",
	   ARG_CMD(a), ARG_CUR(a));
    bad_arg=1;
    goto done;
  }

  pDiagsInfo->userDeviceHandle = SOC_SBX_CONTROL(unit)->sbhdl;
  pDiagsInfo->unit = unit;
  pDiagsInfo->debug_level = verbose;
  pDiagsInfo->bLoopOnError = loop_on_error;
  sscanf(pattern,"%llx",&pDiagsInfo->pattern);

#ifdef BCM_SIRIUS_SUPPORT
  if (SOC_IS_SIRIUS(unit)) {
    pSiriusDiagParams = &(pDiagsInfo->siriusDiagParams); 
    pSiriusDiagParams->ci_interface = ci_interface;
    pSiriusDiagParams->ddr_step_addr = ddr_step_addr;
    pSiriusDiagParams->ddr_burst = ddr_burst_size;
    pSiriusDiagParams->ddr_iter = ddr_test_count;
    pSiriusDiagParams->ddr_test_mode = mem_test;
    pSiriusDiagParams->ddr_start_addr = ddr_start_addr;
    pSiriusDiagParams->ddr3_col = SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns;
    pSiriusDiagParams->ddr3_row = SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows;
    pSiriusDiagParams->bank = bank;
    pSiriusDiagParams->max_row = max_row;
    /* flag for use in mem_test_done for external memories */
    pDiagsInfo->inited = SBX_EXT_DDR_FLAG;
    pDiagsInfo->seed = seed;
  }
#endif

  if (SOC_IS_SBX_BME3200(unit)) {
    pDiagsInfo->e_bme_mem_test = mem_test;
  }

  if (SOC_IS_SBX_FE2000(unit)) {

    pDiagsInfo->seed = seed;
    pDiagsInfo->e_mem_test = mem_test;
    pDiagsInfo->reinit = TRUE;
    if (mem_test%2) { /* odd tests are mmu1 */
      pDiagsInfo->uInstance = 1;
    } else {
      pDiagsInfo->uInstance = 0;
    }
    sscanf(start_addr,"%lx",&pDiagsInfo->start_addr);
    sscanf(end_addr,"%lx",&pDiagsInfo->end_addr);
    
    /* retrieve params from config.bcm, defaults to fe2k ref platform */
    pDiagsInfo->wp_config.data_width = 
        SOC_SBX_CFG_FE2000(unit)->mm0wdwidth;
    pDiagsInfo->wp_config.addr_width = 
        SOC_SBX_CFG_FE2000(unit)->mm0wawidth;
    pDiagsInfo->np0_config.data_width = 
        SOC_SBX_CFG_FE2000(unit)->mm0n0dwidth;
    pDiagsInfo->np0_config.addr_width = 
        SOC_SBX_CFG_FE2000(unit)->mm0n0awidth;
    pDiagsInfo->np1_config.data_width = 
        SOC_SBX_CFG_FE2000(unit)->mm0n1dwidth;
    pDiagsInfo->np1_config.addr_width = 
        SOC_SBX_CFG_FE2000(unit)->mm0n1awidth;

  }

  if (help) {
    if (SOC_IS_SIRIUS(unit)) {
#ifdef BCM_SIRIUS_SUPPORT
      printk("%s\n",qe4000_ddr_test_usage);
      rv = -1; /* do not allow test to run */
      goto done;
#endif      
    }
  }

  if (!SOC_SBX_INIT(unit)) {
    if (soc_sbx_init(unit) < 0) {
      test_error(unit,"SOC Layer initialization failed\n");
      goto done;
    }
  }

  *p = pDiagsInfo;
  rv = 0;

done:
  if (bad_arg) {
    printk("valid test args:\n");
    parse_eq_format(&pt);
  }
  parse_arg_eq_done(&pt);
  return rv;
}

int
sbx_mem_test(int u, args_t *a, void *p)
{

  int rv = 0;
  char *s;
  unsigned char b_cMemTest = 0;
  sbxDiagsInfo_t *pDiagsInfo = p;

  /* if MemTest was specified, just run that one */
  while (( s = ARG_GET(a)) != NULL) {
    if (strstr(s,"MemTest") != 0 ) {
      b_cMemTest = 1;
    }
    /* if EndAddr was specified test only up to EndAddr */
    if (strstr(s,"end_addr") != 0) {
      pDiagsInfo->bEndAddr = 1;
    }
  }

  if (b_cMemTest) {
    if (SOC_IS_SBX_FE2KXT(u)) {
#ifdef BCM_FE2000_SUPPORT
      rv = sbFe2kxtDiagsSramMemTest(pDiagsInfo);
#endif
    } else if (SOC_IS_SBX_FE2000(u)) {
#ifdef BCM_FE2000_SUPPORT
      rv = sbFe2000DiagsSramMemTest(pDiagsInfo);
#endif
    } else if (SOC_IS_SBX_BME3200(u)) {
      rv = sbBme3200DiagsSramMemTest(pDiagsInfo);
    } else if (SOC_IS_SIRIUS(u)) {
#ifdef BCM_SIRIUS_SUPPORT
      rv = siriusDiagsDDRTest(pDiagsInfo);
#endif
    }
  } else { /* run all memtests for unit# */
    if (SOC_INFO(u).chip_type == SOC_INFO_FE2000) {
#ifdef BCM_FE2000_SUPPORT
      rv = sbFe2000DiagsSramMemTestAll(pDiagsInfo);
#endif
    } else if (SOC_IS_SBX_FE2KXT(u)) {
#ifdef BCM_FE2000_SUPPORT
      rv = sbFe2kxtDiagsSramMemTestAll(pDiagsInfo);
#endif
    } else if (SOC_IS_SBX_BME3200(u)) {
      rv = sbBme3200DiagsSramMemTestAll(pDiagsInfo);
    } else if (SOC_IS_SIRIUS(u)) {
#ifdef BCM_SIRIUS_SUPPORT
      rv = siriusDiagsDDRTestAll(pDiagsInfo);
#endif
    }
  }

  return rv;
}

int
sbx_bist_init(int unit, args_t *a, void **p)
{


  sbxDiagsInfo_t *pDiagsInfo = &diagsInfo[0];
  int cam,verbose,qe_bist_test;
  char *pattern = "";
  parse_table_t pt;
  int rv = -1;
  int bad_arg=0;
  int reinit=0;
  parse_table_init(unit, &pt);

  parse_table_add(&pt, "Pattern", PQ_STRING, "-1",
		  &pattern, NULL);

  parse_table_add(&pt,  "Verbose",PQ_INT,(void *) 0,
		  &verbose, NULL);
  parse_table_add(&pt,  "reinit",PQ_INT,(void *) 1,
		  &reinit, NULL);

  if (SOC_IS_SBX_FE2000(unit)) {
    parse_table_add(&pt,  "cam",	PQ_INT, (void *) (-1),
		    &cam, NULL);
  }

  if (SOC_IS_SBX_QE2000(unit)) {
    parse_table_add(&pt,  "BistTest", PQ_INT, (void *) 0,
		    &qe_bist_test, NULL);
  }

  sal_memset(pDiagsInfo,0,sizeof(*pDiagsInfo));

  if (parse_arg_eq(a, &pt) < 0 || ARG_CNT(a) != 0) {
    printk("%s: Invalid option: %s\n",
	   ARG_CMD(a), ARG_CUR(a));
    bad_arg=1;
    goto done;
  }


  pDiagsInfo->userDeviceHandle = SOC_SBX_CONTROL(unit)->sbhdl;
  pDiagsInfo->unit = unit;
  pDiagsInfo->debug_level = verbose;
  pDiagsInfo->reinit = reinit;

  if (SOC_IS_SBX_FE2000(unit)) {
    pDiagsInfo->cam = cam;
  }

  sscanf(pattern,"%llx",&pDiagsInfo->pattern);

  if (SOC_IS_SBX_QE2000(unit)) {
    pDiagsInfo->e_qe_bist_test = qe_bist_test;
  }

  *p = pDiagsInfo;
  rv = 0;

 done:
  if (bad_arg) {
    printk("valid test args:\n");
    parse_eq_format(&pt);
  }
  parse_arg_eq_done(&pt);
  return rv;
}

int
sbx_bist_test(int u, args_t *a, void *p)
{
  unsigned char b_cBistTest = 0;
  char *s;
  int rv=-1;
  sbxDiagsInfo_t *pDiagsInfo = p;

  /* if BistTest was specified, just run that one */
  while (( s = ARG_GET(a)) != NULL) {
    if (strstr(s,"BistTest") != 0) {
      b_cBistTest = 1;
    }
  }

  if (SOC_IS_SBX_FE2KXT(u)) {
#ifdef BCM_FE2000_SUPPORT
    rv = sbFe2kxtDiagsBistStart(pDiagsInfo);
#endif
  } else if (SOC_IS_SBX_FE2000(u)) {
#ifdef BCM_FE2000_SUPPORT
    rv = sbFe2000DiagsBistStart(pDiagsInfo);
#endif
  } else if (SOC_IS_SBX_BME3200(u)) {
    rv = sbBme3200BistStart(pDiagsInfo);
  } else if (SOC_IS_SBX_QE2000(u)) {
#ifdef BCM_QE2000_SUPPORT
    if (!b_cBistTest) {
      rv = sbQe2000DiagsBistTestAll(pDiagsInfo);
    } else {
      rv = sbQe2000DiagsBistTest(pDiagsInfo);
    }
#endif
  }
  return rv;

}

int
sbx_unit_prbs_init(int unit, args_t *a, void **p)
{

  parse_table_t pt;
  bcm_pbmp_t pbmp;
  char *mode_str = NULL;
  char *port_str = NULL;
  unit_prbs_param_t *p_prbs = NULL;

  /* Run a special Serdes PRBS test for QE BenchScreen */
  if ((SOC_IS_SBX_QE2000(unit))) {
    int rv = BCM_E_NONE;

    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: called...\n", FUNCTION_NAME()));

    /* Make the actual call to the QE PRBS Test */
    rv = sbx_qe_prbs_init(unit, a, p);

    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: done...\n", FUNCTION_NAME()));
    return rv;
  }

  p_prbs = sal_alloc(sizeof(unit_prbs_param_t), "prbs param");

  if (p_prbs == NULL) {
    printk("%s: out of memory\n", ARG_CMD(a));
    return(-1);    
  }

  sal_memset(p_prbs, 0, sizeof(*p_prbs));
  BCM_PBMP_CLEAR(pbmp);

  parse_table_init(unit,&pt);
  parse_table_add(&pt, "Mode", PQ_STRING, (void *) 0, &mode_str,NULL);
  parse_table_add(&pt, "Ports", PQ_STRING, (void *) 0, &port_str,NULL);
  parse_table_add(&pt, "LoopBack", PQ_INT,(void *)(1), &p_prbs->loopback, NULL);
  parse_table_add(&pt, "SEConds", PQ_INT, (void *)5, &p_prbs->seconds, NULL);
  parse_table_add(&pt, "Polynomial", PQ_INT, (void *)(0), &p_prbs->poly, NULL);

  /* Parse remaining arguments */
  if (parse_arg_eq(a, &pt) < 0 || ARG_CNT(a) != 0) {
    test_error(unit,
	       "%s: Invalid option: %s\n",
	       ARG_CMD(a),
	       ARG_CUR(a) ? ARG_CUR(a) : "*");
    parse_arg_eq_done(&pt);
    return (-1);
  }
  
  /* check for extra arguments */
  if (ARG_CNT(a) != 0) {
    printk("%s: extra options starting with \"%s\" \n",
	   ARG_CMD(a), ARG_CUR(a));
    parse_arg_eq_done(&pt);
    return (-1);
  }

  if ((mode_str != NULL) && (sal_strcmp(mode_str,"") != 0)) {
    if (sal_strcasecmp(mode_str, "si") == 0) {
      p_prbs->mode = 1;
    } else if (sal_strcasecmp(mode_str, "hc") == 0) {
      p_prbs->mode = 0;
    } else {
      printk("Prbs mode must be si or hc\n");
      return (-1);
    }
  }

  if ((port_str != NULL) && (sal_strcmp(port_str,"") != 0)) {
    if (parse_bcm_pbmp(unit, port_str, &pbmp) < 0) {
      printk("Error: unrecognized port bitmap: %s\n", port_str);
      parse_arg_eq_done(&pt);
      return (-1);
    }
    BCM_PBMP_ASSIGN(p_prbs->pbmp,pbmp);
  } else {
    /* use the unit pbmp to test all ports */
    BCM_PBMP_ASSIGN(p_prbs->pbmp, PBMP_PORT_ALL(unit));
  }

  if (p_prbs->poly < 0 || p_prbs->poly > 3) {
    printk("Error: poly range 0..3 valid\n");
    return (-1);
  }

  printk("Testing mode=%s loopback=%d poly=%d seconds=%04d \n",(p_prbs->mode) ? "si" : "hc",
	 p_prbs->loopback,
	 p_prbs->poly,p_prbs->seconds);

  parse_arg_eq_done(&pt);
  prbs_param[unit] = p_prbs;

  return 0;
}

int
sbx_prbs_init(int unit, args_t *a, void **p)
{

  sbxDiagsInfo_t *pDiagsInfo = &diagsInfo[0];
  int Lsfr, Invert=0;
  int spi = 0;
  int force_error = 0;
  parse_table_t pt;
  int rv = -1;
  int bad_arg=0;
  int direction=0;
  int loop_on_error = 0;
  char* qe_link_mask = NULL;
  char* qe_mask = NULL;


  parse_table_init(unit, &pt);
  parse_table_add(&pt,  "Lsfr",	PQ_INT, (void *) 0,
		  &Lsfr, NULL);

  if (SOC_IS_SBX_FE2000(unit)) {
    parse_table_add(&pt,  "spi", PQ_INT, (void *) -1,
		    &spi, NULL);
  }

  if (!SOC_IS_SBX_FE2KXT(unit)) {
    parse_table_add(&pt,  "Invert", PQ_INT, (void *) -1,
		    &Invert, NULL);
  }

  if (SOC_IS_SBX_FE2KXT(unit)) {
    parse_table_add(&pt,  "direction", PQ_INT, (void *) -1,
		    &direction, NULL);
  }

  if (SOC_IS_SBX_BM9600(unit)) {
    parse_table_add(&pt,  "force_error", PQ_INT, (void *) 0,
		    &force_error, NULL);
    parse_table_add(&pt,  "direction", PQ_INT, (void *) -1,
		    &direction, NULL);
    parse_table_add(&pt,  "qe_mask", PQ_STRING, (void *) "0xffffffff",
		    &qe_mask, NULL);
    parse_table_add(&pt, "qe_link_mask", PQ_STRING, (void *)"0xfffff", 
		    &qe_link_mask,NULL);
    parse_table_add(&pt,  "LoopOnError",PQ_INT,(void *) 0,
		    &loop_on_error, NULL);
  }


  sal_memset(pDiagsInfo,0,sizeof(*pDiagsInfo));

  if (parse_arg_eq(a, &pt) < 0 || ARG_CNT(a) != 0) {
    printk("%s: Invalid option: %s\n",
	   ARG_CMD(a), ARG_CUR(a));
    bad_arg=1;
    goto done;
  }

  if (SOC_IS_SBX_FE2000(unit)) {
    if (spi > 1) {
      printk("Bad SPI Interface\n");
      goto done;
    }
  }


  pDiagsInfo->userDeviceHandle = SOC_SBX_CONTROL(unit)->sbhdl;
  pDiagsInfo->unit = unit;
  pDiagsInfo->nLSFR = Lsfr;
  pDiagsInfo->nInvert = Invert;
  pDiagsInfo->spi_interface = spi;

  if (SOC_IS_SBX_FE2KXT(unit)) {
    pDiagsInfo->prbs_direction = direction;
  }

  if (SOC_IS_SBX_BM9600(unit)) {
    pDiagsInfo->bForcePRBSError = force_error;
    pDiagsInfo->prbs_direction = direction;
    pDiagsInfo->bLoopOnError = loop_on_error;
    sscanf(qe_link_mask,"%x",&pDiagsInfo->qe_prbs_link_mask);
    sscanf(qe_mask,"%x",&pDiagsInfo->qe_prbs_mask);
  }

  *p = pDiagsInfo;
  rv = 0;

  if (SOC_IS_SBX_BM9600(unit)) {
    if (soc_property_get(unit,spn_QE_TME_MODE,0)) {
      soc_cm_print("This test can not be run in tme mode.\n");    
      soc_cm_print("set qe_tme_mode=0 in config.bcm, re-start bcm.user.\n");    
      return (-1);
    }
  }

 done:

  if (bad_arg) {
    printk("valid test args:\n");
    parse_eq_format(&pt);
  }
  parse_arg_eq_done(&pt);
  return rv;
}

int
sbx_unit_prbs_test(int u, args_t *a, void *p)
{
  unit_prbs_param_t *prbs = prbs_param[u];
  bcm_port_t  port, dport;
  int rv = BCM_E_NONE;
  int ret_val = BCM_E_NONE;
  int enable = 0;
  int status;
  int i;

  /* Run a special Serdes PRBS test for QE BenchScreen */
  if ((SOC_IS_SBX_QE2000(u))) {
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: called...\n", FUNCTION_NAME()));

    /* Make the actual call to the QE PRBS Test */
    rv = sbx_qe_prbs_test(u, a, p);

    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: done...\n", FUNCTION_NAME()));
    return rv;
  }

  assert(prbs != NULL);

  DPORT_BCM_PBMP_ITER(u,prbs->pbmp,dport,port) {

    /* skip the requeue ports */
    if (!strncmp(BCM_PORT_NAME(u,port),"req",3)) {
      continue;
    }

    /* enable the port */
    rv = bcm_port_enable_set(u,port,1);
    if (rv != BCM_E_NONE) {
      test_error(u,"Enabling port:%s failed (%s)\n",
		 BCM_PORT_NAME(u,port),bcm_errmsg(rv));
    } 

    /* set polynomial to test */
    rv = bcm_port_control_set(u, port, bcmPortControlPrbsPolynomial,
			      prbs->poly);

    if (rv != BCM_E_NONE) {
      test_error(u,"Setting prbs polynomial(%d) failed for port:%s (%s)\n",
		 prbs->poly,BCM_PORT_NAME(u,port),bcm_errmsg(rv));
    }

    /* set mode to hc=0, si=1 test */
    rv = bcm_port_control_set(u, port, bcmPortControlPrbsMode,
			      prbs->mode);

    if (rv != BCM_E_NONE) {
      test_error(u,"Setting mode(%d) failed for port:%s (%s)\n",
		 prbs->mode,BCM_PORT_NAME(u,port),bcm_errmsg(rv));
    }


    /* clear tx/rx enable first */
    rv = bcm_port_control_set(u,port,
			      bcmPortControlPrbsTxEnable,
			      0);
    if (rv != BCM_E_NONE) {
      test_error(u,"Clearing tx enable failed for port:%s (%s)\n",
		 BCM_PORT_NAME(u,port),bcm_errmsg(rv));
    }

    rv = bcm_port_control_set(u,port,
			      bcmPortControlPrbsRxEnable,
			      0);

    if (rv != BCM_E_NONE) {
      test_error(u,"Clearing rx enable failed for port:%s (%s)\n",
		 BCM_PORT_NAME(u,port),bcm_errmsg(rv));
    }

    /* set rx enable */
    enable = 1;
    rv = bcm_port_control_set(u, port,                       
			      bcmPortControlPrbsRxEnable,       
			      enable);
    if (rv != BCM_E_NONE) {
      test_error(u,"Setting prbs rx enable failed for port:%s (%s)\n",
		 BCM_PORT_NAME(u,port),bcm_errmsg(rv));
      continue;
    }


    /* set tx enable */
    if (prbs->loopback) {
      enable |= 0x8000;
    }

    rv = bcm_port_control_set(u,port,
			      bcmPortControlPrbsTxEnable,
			      enable);

    if (rv != BCM_E_NONE) {
      test_error(u,"Setting prbs tx enable failed for port:%s (%s)\n",
		 BCM_PORT_NAME(u,port),bcm_errmsg(rv));
      continue;
    }

    /* clear any errors before running for specified time */
    for (i = 0; i < 2; i++) {
      /* Read twice to clear errors */
      rv = bcm_port_control_get(u,port,
				bcmPortControlPrbsRxStatus,
				&status);
      if (rv != BCM_E_NONE) {
	test_error(u,"Clearing prbs status failed for port:%s (%s)\n",
		   BCM_PORT_NAME(u,port),bcm_errmsg(rv));

      }
      sal_sleep(1);
    }

    /* now run for specifed time on each link */
    sal_sleep(prbs->seconds);


    /* get results */
    rv = bcm_port_control_get(u,port,
			      bcmPortControlPrbsRxStatus,
			      &status);

    if (rv != BCM_E_NONE) {
      test_error(u,"Getting prbs status failed for port:%s (%s)\n",
		 BCM_PORT_NAME(u,port),bcm_errmsg(rv));
      status = -1;
    }

    switch (status) {
    case 0:
      printk("%s (%2d):  PRBS OK!\n", BCM_PORT_NAME(u, port), port);
      break;
    case -1:
      printk("%s (%2d):  PRBS Failed!\n", BCM_PORT_NAME(u, port), port);
      ret_val = -1;
      break;
    default:
      printk("%s (%2d):  PRBS has %d errors!\n", BCM_PORT_NAME(u, port), 
	     port, status);
      ret_val = -1;
      break;
    }
  }
  return ret_val;
}

int
sbx_prbs_test(int u, args_t *a, void *p)
{

  int lsfr = 0;
  int rv=0;
  int stat=0;
  int dir=0;
  sbxDiagsInfo_t *pDiagsInfo = p;
  uint8_t both_spi = TRUE;

  if (pDiagsInfo->spi_interface != -1) {
    both_spi = FALSE;
  }

  if (SOC_IS_SBX_FE2000(u)) { /* FE or FEXT */
    if (!SOC_IS_SBX_FE2KXT(u)) {
      rv = sbFe2000DiagsSPIPrbsTest(pDiagsInfo);
      if (rv !=0) stat = -1;
    } else {
      if (pDiagsInfo->prbs_direction == -1) {
	for(dir=0;dir<=1;dir++) {
	  pDiagsInfo->prbs_direction = dir;
	  /* run test on each spi by default */
	  if (both_spi) {
	    pDiagsInfo->spi_interface = 0;
	    rv = sbFe2kxtDiagsSPIPrbsTest(pDiagsInfo);
	    if (rv != 0) stat = -1;
	    pDiagsInfo->spi_interface = 1;
	    rv = sbFe2kxtDiagsSPIPrbsTest(pDiagsInfo);
	    if (rv != 0) stat = -1;
	  } else {
	    rv = sbFe2kxtDiagsSPIPrbsTest(pDiagsInfo);
	    if (rv !=0) stat = -1;
	  }
	}
      }
    }
  } else if (SOC_IS_SBX_BM9600(u)) {
    if (pDiagsInfo->nLSFR == -1) {
      for (lsfr=0;lsfr<2;lsfr++) {
	pDiagsInfo->nLSFR = lsfr;
	rv = sbBme9600DiagsPrbsTest(pDiagsInfo);
	if (rv != 0) stat = -1;
      }
    } else {
      rv = sbBme9600DiagsPrbsTest(pDiagsInfo);
      if (rv != 0) stat = -1;
    }
  }
  return stat;
}

int
sbx_10g_loopback_init(int unit, args_t *a, void **p)
{

  sbxDiagsInfo_t *pDiagsInfo = &diagsInfo[0];
  int verbose,nPackets,nPacketLen,nInternal;
  int n10GPhy = 0;
  int nLoop = 0;
  int stop_snake = 0;
  int runtime = 0;
  parse_table_t pt;
  int bad_arg=0;
  int start_port, end_port;
  int rv = -1;

  parse_table_init(unit, &pt);
  parse_table_add(&pt,  "Packets",PQ_INT,(void *) 250,
		  &nPackets, NULL);
  parse_table_add(&pt,  "PacketLen",PQ_INT,(void *) 64,
		  &nPacketLen, NULL);
  parse_table_add(&pt,  "Internal",PQ_INT,(void *) 0,
		  &nInternal, NULL);
  if (!SOC_IS_SBX_FE2KXT(unit)) {
    parse_table_add(&pt,  "10GPhy",PQ_INT,(void *) 0,
		    &n10GPhy, NULL);
  }

  if (SOC_IS_SBX_FE2KXT(unit)) {
    parse_table_add(&pt,  "sp",PQ_INT,(void *) 24,
		    &start_port, NULL);
    parse_table_add(&pt,  "ep",PQ_INT,(void *) 25,
		    &end_port, NULL);
  }

  parse_table_add(&pt,  "Verbose",PQ_INT,(void *) 0,
		  &verbose, NULL);
  parse_table_add(&pt,  "Loop",PQ_INT,(void *) 1,
		  &nLoop, NULL);
  parse_table_add(&pt,  "Stop",PQ_INT,(void *) 1,
		  &stop_snake, NULL);
  parse_table_add(&pt,  "runtime",PQ_INT,(void *) 5,
		  &runtime, NULL);

  sal_memset(pDiagsInfo,0,sizeof(*pDiagsInfo));

  if (parse_arg_eq(a, &pt) < 0 || ARG_CNT(a) != 0) {
    printk("%s: Invalid option: %s\n",
	   ARG_CMD(a), ARG_CUR(a));
    bad_arg=1;
    goto done;
  }


  pDiagsInfo->userDeviceHandle = SOC_SBX_CONTROL(unit)->sbhdl;
  pDiagsInfo->unit = unit;
  pDiagsInfo->debug_level = verbose;
  pDiagsInfo->nInjectPacketsLen = nPacketLen;
  pDiagsInfo->bInternalLpk = nInternal;
  if (!SOC_IS_SBX_FE2KXT(unit)) {
    pDiagsInfo->b10GPhyLpk = n10GPhy;
  }
  pDiagsInfo->bSnakeLoopOn = nLoop;
  pDiagsInfo->uSnakeRunTime = runtime;
  pDiagsInfo->bStopLoopingPacket = stop_snake;

  if (SOC_IS_SBX_FE2KXT(unit)) {
    pDiagsInfo->start_port = start_port;
    pDiagsInfo->end_port = end_port;
  }

  if (nLoop == 1 && nPackets == -1) {
    if (SOC_IS_SBX_FE2KXT(unit)) {
      nPackets = 500; /* temp ,1000 get no_free_page on FE */
    } else {
      nPackets = 1000;
    }
  } 

  if (nLoop == 0 && nPackets == -1) {
    nPackets = 1;
  }
    
  pDiagsInfo->nInjectPackets = nPackets;

  if (!SOC_SBX_INIT(unit)) {
    if (soc_sbx_init(unit) < 0) {
      test_error(unit,"SOC Layer initialization failed\n");
      goto done;
    }
  }

  if (!(bcm_init_check(unit) )) {
    if (bcm_init(unit) < 0) {
      test_error(unit,"BCM Layer initialization failed\n");
      goto done;
    }
  }


  *p = pDiagsInfo;
  rv = 0;

 done:

  if (bad_arg) {
    printk("valid test args:\n");
    parse_eq_format(&pt);
  }
  parse_arg_eq_done(&pt);
  return rv;

}

int
sbx_loopback_init(int unit, args_t *a, void **p)
{

  sbxDiagsInfo_t *pDiagsInfo = &diagsInfo[0];
  int verbose,nPackets,nPacketLen,nInternal,n1GPhy;
  int start_port, end_port;
  int runtime;
  int init_ports;
  parse_table_t pt;
  int bad_arg=0;
  int rv = -1;
  int reinit=0;

  parse_table_init(unit, &pt);
  parse_table_add(&pt,  "Packets",PQ_INT,(void *) 1,
		  &nPackets, NULL);
  parse_table_add(&pt,  "PacketLen",PQ_INT,(void *) 64,
		  &nPacketLen, NULL);
  parse_table_add(&pt,  "Internal",PQ_INT,(void *) 0,
		  &nInternal, NULL);
  parse_table_add(&pt,  "1GPhy",PQ_INT,(void *) 0,
		  &n1GPhy, NULL);
  parse_table_add(&pt,  "Verbose",PQ_INT,(void *) 0,
		  &verbose, NULL);
  parse_table_add(&pt,  "sp",PQ_INT,(void *) 0,
		  &start_port, NULL);
  parse_table_add(&pt,  "ep",PQ_INT,(void *) 23,
		  &end_port, NULL);
  parse_table_add(&pt,  "runtime",PQ_INT,(void *) 5,
		  &runtime, NULL);
  parse_table_add(&pt,  "init_ports",PQ_INT,(void *) 0,
		  &init_ports, NULL);
  parse_table_add(&pt,  "reinit",PQ_INT,(void *) 1,
		  &reinit, NULL);

  sal_memset(pDiagsInfo,0,sizeof(*pDiagsInfo));

  if (parse_arg_eq(a, &pt) < 0 || ARG_CNT(a) != 0) {
    printk("%s: Invalid option: %s\n",
	   ARG_CMD(a), ARG_CUR(a));
    bad_arg=1;
    goto done;
  }

  if (start_port < 0 || start_port > 23) {
    printk("bad starting port %d\n",start_port);
    goto done;
  }
  
  if (end_port < 0 || end_port > 23 ) {
    printk("bad ending port %d\n",end_port);
    goto done;
  }

  pDiagsInfo->userDeviceHandle = SOC_SBX_CONTROL(unit)->sbhdl;
  pDiagsInfo->unit = unit;
  pDiagsInfo->debug_level = verbose;
  pDiagsInfo->nInjectPacketsLen = nPacketLen;
  pDiagsInfo->bInternalLpk = nInternal;
  pDiagsInfo->b1GPhyLpk = n1GPhy;
  pDiagsInfo->start_port = start_port;
  pDiagsInfo->end_port = end_port;
  pDiagsInfo->uSnakeRunTime = runtime;
  pDiagsInfo->init_ports = init_ports;

  pDiagsInfo->nInjectPackets = nPackets;
  /* need to restore default queues after test is run */
  pDiagsInfo->reinit = reinit;

  if (!SOC_SBX_INIT(unit)) {
    if (soc_sbx_init(unit) < 0) {
      test_error(unit,"SOC Layer initialization failed\n");
      goto done;
    }
  }

  if (!(bcm_init_check(unit) )) {
    if (bcm_init(unit) < 0) {
      test_error(unit,"BCM Layer initialization failed\n");
      goto done;
    }
  }

  *p = pDiagsInfo;
  rv = 0;

 done:

  if (bad_arg) {
    printk("valid test args:\n");
    parse_eq_format(&pt);
  }
  parse_arg_eq_done(&pt);
  return rv;

}

#if defined(BCM_FE2000_SUPPORT) || defined(BCM_FE2000XT_SUPPORT)
int
sbx_1G_Ext_loopback_test(int u, args_t *a, void *p)
{
  int rv = 0;
  sbxDiagsInfo_t *pDiagsInfo = p;
  if (SOC_IS_SBX_FE2KXT(u)) {
    pDiagsInfo->e_loopback_type = SB_FE2KXT_1G_LOOPBACK;
    rv = sbFe2kxtLoopbackTest(pDiagsInfo);
  } else {
    pDiagsInfo->e_loopback_type = SB_FE2000_1G_LOOPBACK;
    rv = sbFe2000LoopbackTest(pDiagsInfo);
  }
  return rv;
}

int
sbx_1G_Phy_loopback_test(int u, args_t *a, void *p)
{
  int rv = 0;
  sbxDiagsInfo_t *pDiagsInfo = p;

  if (SOC_IS_SBX_FE2KXT(u)) {
    pDiagsInfo->e_loopback_type = SB_FE2KXT_1G_PHY_LOOPBACK;
    rv = sbFe2kxtLoopbackTest(pDiagsInfo);
  } else {
    pDiagsInfo->e_loopback_type = SB_FE2000_1G_PHY_LOOPBACK;
    rv = sbFe2000LoopbackTest(pDiagsInfo);
  }
  return rv;
}

int
sbx_1G_Int_loopback_test(int u, args_t *a, void *p)
{
  int rv = 0;
  sbxDiagsInfo_t *pDiagsInfo = p;
  if (SOC_IS_SBX_FE2KXT(u)) {
    pDiagsInfo->e_loopback_type = SB_FE2KXT_1G_UNIMAC_LOOPBACK;
    rv = sbFe2kxtLoopbackTest(pDiagsInfo);
  } else {
    pDiagsInfo->e_loopback_type = SB_FE2000_1G_UNIMAC_LOOPBACK;
    rv = sbFe2000LoopbackTest(pDiagsInfo);
  }
  return rv;
}

int
sbx_10G_XE0_loopback_test(int u, args_t *a, void *p)
{

  int rv = 0;
  sbxDiagsInfo_t *pDiagsInfo = p;
  pDiagsInfo->u10gPort = 24;
  if (pDiagsInfo->bSnakeLoopOn) {
    pDiagsInfo->e_loopback_type = SB_FE2000_10G_QE_LOOPBACK;
  } else {
    pDiagsInfo->e_loopback_type = SB_FE2000_10G_LOOPBACK;
  }
  rv = sbFe2000LoopbackTest(pDiagsInfo);
  return rv;

}

int
sbx_10G_XE1_loopback_test(int u, args_t *a, void *p)
{

  int rv = 0;
  sbxDiagsInfo_t *pDiagsInfo = p;
  pDiagsInfo->u10gPort = 25;
  if (pDiagsInfo->bSnakeLoopOn) {
    pDiagsInfo->e_loopback_type = SB_FE2000_10G_QE_LOOPBACK;
  } else {
    pDiagsInfo->e_loopback_type = SB_FE2000_10G_LOOPBACK;
  }
  rv = sbFe2000LoopbackTest(pDiagsInfo);
  return rv;
}

int
sbx_10G_XE_loopback_test(int u, args_t *a, void *p)
{
  int rv = 0;
  sbxDiagsInfo_t *pDiagsInfo = p;
  if (pDiagsInfo->bSnakeLoopOn) {
    pDiagsInfo->e_loopback_type = SB_FE2KXT_10G_QE_LOOPBACK;
  } else {
    pDiagsInfo->e_loopback_type = SB_FE2KXT_10G_LOOPBACK;
  }

  rv = sbFe2kxtLoopbackTest(pDiagsInfo);
  return rv;
}

int
sbx_snake_init(int unit, args_t *a, void **p)
{

  sbxDiagsInfo_t *pDiagsInfo = &diagsInfo[0];
  int verbose,nPackets,nPacketLen;
  int start_port, end_port;
  int nLoop, runtime;
  int stop_snake;
  parse_table_t pt;
  int bad_arg=0;
  int rv = -1;

  parse_table_init(unit, &pt);
  parse_table_add(&pt,  "Packets",PQ_INT,(void *) 1,
		  &nPackets, NULL);
  parse_table_add(&pt,  "PacketLen",PQ_INT,(void *) 64,
		  &nPacketLen, NULL);
  parse_table_add(&pt,  "Loop",PQ_INT,(void *) 1,
		  &nLoop, NULL);
  parse_table_add(&pt,  "sp",PQ_INT,(void *) 0,
		  &start_port, NULL);
  parse_table_add(&pt,  "ep",PQ_INT,(void *) 23,
		  &end_port, NULL);
  parse_table_add(&pt, "runtime",PQ_INT,(void *) 5,
		  &runtime, NULL);
  parse_table_add(&pt,  "Verbose",PQ_INT,(void *) 0,
		  &verbose, NULL);
  parse_table_add(&pt,  "Stop",PQ_INT,(void *) 1,
		  &stop_snake, NULL);

  sal_memset(pDiagsInfo,0,sizeof(*pDiagsInfo));

  if (parse_arg_eq(a, &pt) < 0 || ARG_CNT(a) != 0) {
    printk("%s: Invalid option: %s\n",
	   ARG_CMD(a), ARG_CUR(a));
    bad_arg=1;
    goto done;
  }

  if (start_port%2) {
    printk("start port needs to be even port#\n");
    goto done;
  }

  if (!(end_port%2)) {
    printk("end port needs to be odd port#\n");
    goto done;
  }

  if (end_port <= start_port) {
    printk("use end_port > start_port\n");
    goto done;
  }

  if (start_port > 23) {
    printk("Test only for 1G ports\n");
    goto done;
  }

  if (ARG_CNT(a) != 0) {
    printk("%s: extra options starting with \"%s\" \n",
	   ARG_CMD(a), ARG_CUR(a));
    bad_arg=1;
    goto done;
  }


  pDiagsInfo->userDeviceHandle = SOC_SBX_CONTROL(unit)->sbhdl;
  pDiagsInfo->unit = unit;
  pDiagsInfo->debug_level = verbose;
  pDiagsInfo->nInjectPackets = nPackets;
  pDiagsInfo->nInjectPacketsLen = nPacketLen;
  pDiagsInfo->bSnakeLoopOn = nLoop; /* when set continuously loop packet */
  pDiagsInfo->start_port = start_port;
  pDiagsInfo->end_port = end_port;
  pDiagsInfo->uSnakeRunTime = runtime; /* ~seconds */
  pDiagsInfo->bStopLoopingPacket = stop_snake;

  if (!SOC_SBX_INIT(unit)) {
    if (soc_sbx_init(unit) < 0) {
      test_error(unit,"SOC Layer initialization failed\n");
      goto done;
    }
  }


  if (!(bcm_init_check(unit) )) {
    if (bcm_init(unit) < 0) {
      test_error(unit,"BCM Layer initialization failed\n");
      goto done;
    }
  }

  *p = pDiagsInfo;
  rv = 0;

 done:

  if (bad_arg) {
    printk("valid test args:\n");
    parse_eq_format(&pt);
  }

  parse_arg_eq_done(&pt);
  return rv;

}

#endif /* BCM_FE2000_SUPPORT */

#ifdef BCM_BM9600_SUPPORT

int 
sbx_reg_init(int u, args_t *a, void **p)
{
 
  sbxDiagsInfo_t *pDiagsInfo = &diagsInfo[0];
  int rv = -1;

  sal_memset(pDiagsInfo,0,sizeof(*pDiagsInfo));
  pDiagsInfo->userDeviceHandle = SOC_SBX_CONTROL(u)->sbhdl;
  pDiagsInfo->unit = u;
  *p = pDiagsInfo;
  rv = 0;

  return rv;

}

int 
sbx_reg_test(int u, args_t *a, void *p)
{
  int rv = 0;
  sbxDiagsInfo_t *pDiagsInfo = p;
  if (SOC_IS_SBX_BM9600(u)) {
    rv = sbBme9600DiagsRegTest(pDiagsInfo);
  }
  return rv;
}

#endif /* BCM_BM9600_SUPPORT */

#ifdef BCM_FE2000_SUPPORT

int
sbx_snake_test(int u, args_t *a, void *p)
{

  int rv = 0;
  sbxDiagsInfo_t *pDiagsInfo = p;
  if (SOC_IS_SBX_FE2KXT(u)) {
  rv = sbFe2kxtDiagsSnakeTest(pDiagsInfo);
  } else {
  rv = sbFe2000DiagsSnakeTest(pDiagsInfo);
  }
  return rv;
}

#endif /* BCM_FE2000_SUPPORT */

void
sbx_diag_reinit(int unit) {
  int rv = 0;
  soc_cm_print("Re-initializing %s .. ",SOC_CHIP_STRING(unit));
  /* turn off console to see results of test otherwise scrolls off screen */
  printk_cons_enable(FALSE);
  rv = diag_rc_load(unit);
  if (SOC_IS_SBX_QE2000(unit)) {
    /* also need to call mcinit again */
    (void)cmd_sbx_mcinit_config(unit,0);
  }
  printk_cons_enable(TRUE);
  if (rv != 0) {
    soc_cm_print("Error loading soc file (%d)\n",rv);
  } else {
    soc_cm_print("Done.\n");
  }
  return;
}

int 
sbx_unit_prbs_test_done(int u, void *p)
{
  /* Run a special Serdes PRBS test for QE BenchScreen */
  if ((SOC_IS_SBX_QE2000(u))) {
    int rv = BCM_E_NONE;

    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: called...\n", FUNCTION_NAME()));

    /* Make the actual call to the QE PRBS Test */
    rv = sbx_qe_prbs_done(u, p);

    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: done...\n", FUNCTION_NAME()));
    return rv;
  }

  if (prbs_param[u] != NULL) {
    sal_free(prbs_param[u]);
  }
  return 0;
}

int
sbx_test_done(int u, void *p)
{
  sbxDiagsInfo_t *pDiagsInfo = p;
  uint32_t uData;
#ifdef BCM_SIRIUS_SUPPORT
  int ci;
#endif /* BCM_SIRIUS_SUPPORT */
  if (pDiagsInfo) {
    if (!SOC_IS_SBX_FE2KXT(u)) {
      if (pDiagsInfo->bPolicerRefreshWasOn) {
	/* turn it back on */
	uData = SAND_HAL_READ( pDiagsInfo->userDeviceHandle,CA,PM_CONFIG0);
	uData = SAND_HAL_MOD_FIELD(CA,PM_CONFIG0,REFRESH_ENABLE,uData,1);
	SAND_HAL_WRITE(pDiagsInfo->userDeviceHandle,CA,PM_CONFIG0,uData);
      }
      /* take CM out of reset */
      SAND_HAL_RMW_FIELD(pDiagsInfo->userDeviceHandle, CA, PC_CORE_RESET, CM_CORE_RESET, 0);
    } else if (SOC_IS_SIRIUS(u)) {
#ifdef BCM_SIRIUS_SUPPORT
      /* clear refresh_override_bit */
      for (ci = 0; ci < SOC_MAX_NUM_CI_BLKS; ci++ ) {
	   SOC_IF_ERROR_RETURN(WRITE_CI_DEBUGr(u,ci,0x1));
      }
#endif /* BCM_SIRIUS_SUPPORT */
    }

    /* if this test is destructive reinit , reinit is set during test_init */
    if (pDiagsInfo->reinit) {
      sbx_diag_reinit(u);
    }
  }
  return 0;
}

/* gsrao - 102208 */
int
sbx_qe_traffic_init(int unit, args_t *a, void **p)
{

  sbxQe2000DiagsInfo_t *pDiagsInfo = NULL;  
  parse_table_t pt;
  int nVerbose, nPackets, nPacketLen;
  int nRunTime;
  int nTimeOut;
  int bad_arg=0;
  uint32_t uPrintTime = 0;
  uint32_t uForceTestPass = 0;
  uint32_t uForceTestFail = 0;
  uint32_t uUseFile = 0;
  uint32_t uDualKa = 0;
  uint32_t ulRb0Queue = 0;
  uint32_t ulRb1Queue = 0;
  uint32_t ulPciQueue = 0;
  char* pInFile = NULL;
  char* pOutFile  = NULL; /* This Out File is used when test is run on only 1 KA device */
  char* pOutFile1 = NULL; /* This Out File is used when test is run on 2 KA devices */
  char* pPattern = NULL;
  char* pNoincr =  NULL;
  char* pPayload = NULL;
  char Noincr1[20] = "noincr=0xaa55aa55";
  char *pNoincr1 = Noincr1;

  int rv = -1;

  DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: called...\n", FUNCTION_NAME()));

  if (!(SOC_IS_SBX_QE2000(unit))) {
    printk("ERROR: [%s:%d]: This test can only be run on the QE2000 Device.\n", FUNCTION_NAME(), __LINE__);
    return rv;
  }

  parse_table_init(unit, &pt);
  parse_table_add(&pt, "packets",   PQ_INT, (void *) 10,   &nPackets,   NULL);
  parse_table_add(&pt, "packetlen", PQ_INT, (void *) 100,  &nPacketLen, NULL);
  parse_table_add(&pt, "runtime",   PQ_INT, (void *) 5,    &nRunTime,   NULL);
  parse_table_add(&pt, "timeout",   PQ_INT, (void *) 5,    &nTimeOut,   NULL);
  parse_table_add(&pt, "verbose",   PQ_INT, (void *) 0,    &nVerbose,   NULL);

  parse_table_add(&pt, "printtime",     PQ_INT, (void *) 0,    &uPrintTime,     NULL);
  parse_table_add(&pt, "forcetestpass", PQ_INT, (void *) 0,    &uForceTestPass, NULL);
  parse_table_add(&pt, "forcetestfail", PQ_INT, (void *) 0,    &uForceTestFail, NULL);

  parse_table_add(&pt, "rb0queue",  PQ_HEX, (void *) 0x10,  &ulRb0Queue,   NULL);
  parse_table_add(&pt, "rb1queue",  PQ_HEX, (void *) 0x0,   &ulRb1Queue,   NULL);
  parse_table_add(&pt, "pciqueue",  PQ_HEX, (void *) 0x310, &ulPciQueue,   NULL);

  parse_table_add(&pt, "dualka",    PQ_BOOL | PQ_NO_EQ_OPT, (void *) 0x0,  &uDualKa,    NULL);
  parse_table_add(&pt, "usefile",   PQ_BOOL | PQ_NO_EQ_OPT, (void *) 0x0,  &uUseFile,   NULL);

  parse_table_add(&pt, "infile",    PQ_STRING, (void *) "/root/IN_DATA.bin",    &pInFile,    NULL);
  parse_table_add(&pt, "outfile",   PQ_STRING, (void *) "/root/OUT_DATA.bin",   &pOutFile,   NULL);
  parse_table_add(&pt, "outfile1",  PQ_STRING, (void *) "/root/OUT_DATA_1.bin", &pOutFile1,  NULL);
  parse_table_add(&pt, "pat",       PQ_STRING, (void *) NULL,                   &pPattern,   NULL);
  parse_table_add(&pt, "noincr",    PQ_STRING, (void *) NULL,                   &pNoincr,    NULL);
  parse_table_add(&pt, "pay",       PQ_STRING, (void *) NULL,                   &pPayload,   NULL);

  if (( pDiagsInfo = sal_alloc(sizeof(*pDiagsInfo), "diag-info")) == 0) {
    test_error(unit,"Out of Memory\n");
    goto done;
  }
  sal_memset(pDiagsInfo, 0, sizeof(*pDiagsInfo));

  if (parse_arg_eq(a, &pt) < 0 || ARG_CNT(a) != 0) {
    printk("ERROR: %s: Invalid option: %s\n", ARG_CMD(a), ARG_CUR(a));
    bad_arg=1;
    goto done;
  }

  if (DIAG_DEBUG_CHECK(DIAG_DBG_VERBOSE)) {
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: nPackets   : %d\n", FUNCTION_NAME(), nPackets  ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: nPacketLen : %d\n", FUNCTION_NAME(), nPacketLen));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: nRunTime   : %d\n", FUNCTION_NAME(), nRunTime  ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: nTimeOut   : %d\n", FUNCTION_NAME(), nTimeOut  ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: nVerbose   : %d\n", FUNCTION_NAME(), nVerbose  ));

    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: uPrintTime     : %d\n", FUNCTION_NAME(), uPrintTime    ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: uForceTestPass : %d\n", FUNCTION_NAME(), uForceTestPass));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: uForceTestFail : %d\n", FUNCTION_NAME(), uForceTestFail));

    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: ulRb0Queue : 0x%x\n", FUNCTION_NAME(), ulRb0Queue  ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: ulRb1Queue : 0x%x\n", FUNCTION_NAME(), ulRb1Queue  ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: ulPciQueue : 0x%x\n", FUNCTION_NAME(), ulPciQueue  ));

    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: uUseFile   : %d\n", FUNCTION_NAME(), uUseFile  ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: uDualKa    : %d\n", FUNCTION_NAME(), uDualKa   ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: pInFile    : %s\n", FUNCTION_NAME(), pInFile   ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: pOutFile   : %s\n", FUNCTION_NAME(), pOutFile  ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: pOutFile1  : %s\n", FUNCTION_NAME(), pOutFile1 ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: pPattern   : %s\n", FUNCTION_NAME(), pPattern  ));

    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: pNoincr    : %s\n", FUNCTION_NAME(), pNoincr   ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: pPayload   : %s\n", FUNCTION_NAME(), pPayload  ));

    /* The following will print the arguments */
    parse_eq_format(&pt);
  }
  
  if (ARG_CNT(a) != 0) {
    printk("ERROR: %s: extra options starting with \"%s\" \n", ARG_CMD(a), ARG_CUR(a));
    bad_arg=1;
    goto done;
  }

  pDiagsInfo->userDeviceHandle = SOC_SBX_CONTROL(unit)->sbhdl;
  pDiagsInfo->unit = unit;
  pDiagsInfo->debug_level = nVerbose;
  pDiagsInfo->uPackets = nPackets;
  pDiagsInfo->uPacketsLen = nPacketLen;
  pDiagsInfo->ulRb0Queue = ulRb0Queue;
  pDiagsInfo->ulRb1Queue = ulRb1Queue;
  pDiagsInfo->ulPciQueue = ulPciQueue;
  pDiagsInfo->uRunTime = nRunTime; /* seconds */
  pDiagsInfo->uTimeOut = nTimeOut; /* seconds */
  pDiagsInfo->uUseFile = uUseFile; /* Use File to read Packet Data */
  pDiagsInfo->uDualKa  = uDualKa;  /* Test is being run on KA Benchscreen Board with Dual KA */
  pDiagsInfo->uPrintTime = uPrintTime;
  pDiagsInfo->uForceTestPass = uForceTestPass;
  pDiagsInfo->uForceTestFail = uForceTestFail;

  /* This test can only be run in TME Mode. */
  if (soc_property_get(unit, spn_QE_TME_MODE, 0) == 0) {
    printk("ERROR: [%s:%d]: This test needs to be run in tme mode.\n", FUNCTION_NAME(), __LINE__);
    printk("ERROR: [%s:%d]:   Set qe_tme_mode=1 in config.bcm, re-start bcm.user.\n", FUNCTION_NAME(), __LINE__);
    goto done;
  }

  if (!SOC_SBX_INIT(unit)) {
    test_error(unit,"SOC Layer initialization not done for unit %d. Do \"init soc\" in sbx.soc script\n", unit);
    goto done;
  }

  if (!(bcm_init_check(unit) )) {
    test_error(unit,"BCM Layer initialization not done for unit %d. Do \"init bcm\" in sbx.soc script\n", unit);
    goto done;
  }

  /* If the user has specified dual KA then verify that both the devices have 
   * been attached and initialized.
   */
  if (pDiagsInfo->uDualKa) {
    /* First verify if we have the right kind of board and right kind of devices. */
    if (soc_ndev != 2) {
      printk("ERROR: [%s:%d]: This test for dual KA can only be run on the dual KA Benchscreen board.\n",
              FUNCTION_NAME(), __LINE__);
      goto done;
    }
    /* The Dual KA test can only be run from unit 0 */
    if (unit != 0) {
      printk("ERROR: [%s:%d]: This test for dual KA can only be run on unit 0.\n", FUNCTION_NAME(), __LINE__);
      goto done;
    }
    if (!(soc_attached(unit)) || !(soc_attached(unit +1))) {
      printk("ERROR: [%s:%d]: This test for dual KA can only be run when both KA devices are attached.\n",
              FUNCTION_NAME(), __LINE__);
      printk("ERROR: [%s:%d]:   Type 'attach' at BCM> prompt to see attached devices.\n",
              FUNCTION_NAME(), __LINE__);
      goto done;
    }
    /* The other unit must also be a KA device */
    if (!(SOC_IS_SBX_QE2000(unit +1))) {
      printk("ERROR: [%s:%d]: This test for dual KA can only be run if both devices are KA devices\n",
             FUNCTION_NAME(), __LINE__);
      goto done;
    }

    /* The other unit must also be initialized */
    if (!SOC_SBX_INIT(unit +1)) {
      test_error(unit,"SOC Layer initialization not done for unit %d. Do \"init soc\" in sbx.soc script\n", unit +1);
      goto done;
    }
   
    if (!(bcm_init_check(unit +1) )) {
      test_error(unit,"BCM Layer initialization not done for unit %d. Do \"init bcm\" in sbx.soc script\n", unit +1);
      goto done;
    }
  }

  /* Allocate space for the various string arguments */
  /* OutFile Name is needed regardless of whether the uUseFile
   * option is specified or not
   */
  pDiagsInfo->pOutFile = sal_alloc((sizeof(char) * sal_strlen(pOutFile)+1), "Qe2000Diags Binary Outfile");
  if (!pDiagsInfo->pOutFile) {
    printk("ERROR: [%s:%d] Out of Memory \n", FUNCTION_NAME(), __LINE__);
    goto done;
  }
  sal_strcpy(pDiagsInfo->pOutFile, pOutFile);
  DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: pDiagsInfo->pOutFile: %s\n", FUNCTION_NAME(), pDiagsInfo->pOutFile));

  /* The output for the other KA chip gets put into outfile1 */
  if (pDiagsInfo->uDualKa) {
    pDiagsInfo->pOutFile1 = sal_alloc((sizeof(char) * sal_strlen(pOutFile1)+1), "Qe2000Diags Binary Outfile1");
    if (!pDiagsInfo->pOutFile1) {
      printk("ERROR: [%s:%d] Out of Memory \n", FUNCTION_NAME(), __LINE__);
      goto done;
    }
    sal_strcpy(pDiagsInfo->pOutFile1, pOutFile1);
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: pDiagsInfo->pOutFile1: %s\n", FUNCTION_NAME(), pDiagsInfo->pOutFile1));
  }

  if (pDiagsInfo->uUseFile) {
    /* User has asked to use Binary files for Packet I/O. */
    pDiagsInfo->pInFile = sal_alloc((sizeof(char) * sal_strlen(pInFile)+1), "Qe2000Diags Binary Infile");
    if (!pDiagsInfo->pInFile) {
      printk("ERROR: [%s:%d] Out of Memory \n", FUNCTION_NAME(), __LINE__);
      goto done;
    }
    sal_strcpy(pDiagsInfo->pInFile, pInFile);
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: pDiagsInfo->pInFile: %s\n", FUNCTION_NAME(), pDiagsInfo->pInFile));
  } else {
    /* User has either specified a pattern to use or prefers to use default. */

    if ((pPattern != NULL) && (sal_strcmp(pPattern, "") != 0)) {

      /* pPattern is NULL by default. It is non-NULL if user has specified a pattern to be repeated */
      pDiagsInfo->pPattern = sal_alloc((sizeof(char) * sal_strlen(pPattern)+1), "Qe2000Diags pat Hex Pattern");
      if (!pDiagsInfo->pPattern) {
        printk("ERROR: [%s:%d] Out of Memory \n", FUNCTION_NAME(), __LINE__);
        goto done;
      }
      sal_strcpy(pDiagsInfo->pPattern, pPattern);
      DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: pDiagsInfo->pPattern: %s\n", FUNCTION_NAME(), pDiagsInfo->pPattern));
      pDiagsInfo->pPayload = NULL;
      pDiagsInfo->pNoincr  = NULL;

    } else if ((pPayload != NULL) && (sal_strcmp(pPayload, "") != 0)) {

      /* pPayload is NULL by default. It is non-NULL if user has specified Payload bytes */
      pDiagsInfo->pPayload = sal_alloc((sizeof(char) * sal_strlen(pPayload)+1), "Qe2000Diags pay Hex Pattern");
      if (!pDiagsInfo->pPayload) {
        printk("ERROR: [%s:%d] Out of Memory \n", FUNCTION_NAME(), __LINE__);
        goto done;
      }
      sal_strcpy(pDiagsInfo->pPayload, pPayload);
      DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: pDiagsInfo->pPayload: %s\n", FUNCTION_NAME(), pDiagsInfo->pPayload));
      pDiagsInfo->pPattern = NULL;
      pDiagsInfo->pNoincr  = NULL;

    } else if ((pNoincr != NULL) && (sal_strcmp(pNoincr, "") != 0)) {

      /* pNoincr is NULL by default. It is non-NULL if user has specified Noincr bytes */
      pDiagsInfo->pNoincr = sal_alloc((sizeof(char) * sal_strlen(pNoincr)+1), "Qe2000Diags noincr Hex Pattern");
      if (!pDiagsInfo->pNoincr) {
        printk("ERROR: [%s:%d] Out of Memory \n", FUNCTION_NAME(), __LINE__);
        goto done;
      }
      sal_strcpy(pDiagsInfo->pNoincr, pNoincr);
      DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: pDiagsInfo->pNoincr: %s\n", FUNCTION_NAME(), pDiagsInfo->pNoincr));
      pDiagsInfo->pPayload = NULL;
      pDiagsInfo->pPattern = NULL;

    } else {

      /* If the user did not specify any of the patterns above, then the default is to use Noincr */
      pDiagsInfo->pNoincr = sal_alloc((sizeof(char) * sal_strlen(pNoincr1)+1), "Qe2000Diags Default Hex Pattern");
      if (!pDiagsInfo->pNoincr) {
        printk("ERROR: [%s:%d] Out of Memory \n", FUNCTION_NAME(), __LINE__);
        goto done;
      }
      sal_strcpy(pDiagsInfo->pNoincr, pNoincr1);
      DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: (Default) pDiagsInfo->pNoincr: %s\n", FUNCTION_NAME(), pDiagsInfo->pNoincr));
      pDiagsInfo->pPayload = NULL;
      pDiagsInfo->pPattern = NULL;
    } 
  }

  *p = pDiagsInfo;
  rv = 0;

  done:

  if (bad_arg) {
    printk("valid test args:\n");
    parse_eq_format(&pt);
  }

  parse_arg_eq_done(&pt);
  if (rv < 0 && pDiagsInfo) {
    sal_free(pDiagsInfo);

    if (pDiagsInfo->pInFile) {
      sal_free(pDiagsInfo->pInFile);
    }
    if (pDiagsInfo->pOutFile) {
      sal_free(pDiagsInfo->pOutFile);
    }
    if (pDiagsInfo->uDualKa) {
      if (pDiagsInfo->pOutFile1) {
        sal_free(pDiagsInfo->pOutFile1);
      }
    }
    if (pDiagsInfo->pPattern) {
      sal_free(pDiagsInfo->pPattern);
    }
    if (pDiagsInfo->pPayload) {
      sal_free(pDiagsInfo->pPayload);
    }
    if (pDiagsInfo->pNoincr) {
      sal_free(pDiagsInfo->pNoincr);
    }
  }

  DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: done...\n", FUNCTION_NAME()));
  return rv;
}


/* gsrao - 102208 */
int
sbx_qe_traffic_test(int unit, args_t *a, void *p)
{

  int rv = 0;
  sbxQe2000DiagsInfo_t *pDiagsInfo = p;

  DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: called...\n", FUNCTION_NAME()));

  if (SOC_IS_SBX_QE2000(unit)) {
    rv = sbQe2000DiagsTrafficTest(pDiagsInfo);
  } else {
    printk("ERROR: [%s:%d]: This test can only be run on the QE2000 Device.\n", FUNCTION_NAME(), __LINE__);
    rv = -1;
  }

  DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: done...\n", FUNCTION_NAME()));

  return rv;
}


/* gsrao - 102208 */
int
sbx_qe_traffic_done(int unit, void *p)
{
  sbxQe2000DiagsInfo_t *pDiagsInfo = p;
  sbhandle qehdl = NULL;
  uint32_t ulData = 0;
#if 0
  uint32_t ulPcTxPktCnt  = 0;  /* Pkts Received by the QE */
  uint32_t ulPcTxByteCnt = 0;
  uint32_t ulPcRxPktCnt  = 0;  /* Pkts Received by the Processor */
  uint32_t ulPcRxByteCnt = 0;
  uint32_t ulDiagByteCnt = pDiagsInfo->uPacketsLen * pDiagsInfo->uPackets;
#endif
  
  int rv = 0;

  DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: called...\n", FUNCTION_NAME()));

  qehdl  = SOC_SBX_CONTROL(unit)->sbhdl;

  if (!pDiagsInfo) {
    printk("ERROR: %s Invalid pDiagsInfo\n", FUNCTION_NAME());
    return -1;
  }    
#if 0
  /* Check the sanity of the Packet Counts */
  ulPcTxPktCnt  = SAND_HAL_READ(qehdl, KA, PC_TX_PKT_CNT);
  ulPcTxByteCnt = SAND_HAL_READ(qehdl, KA, PC_TX_BYTE_CNT);
  ulPcRxPktCnt  = SAND_HAL_READ(qehdl, KA, PC_RX_PKT_CNT);
  ulPcRxByteCnt = SAND_HAL_READ(qehdl, KA, PC_RX_BYTE_CNT);

  DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: PC_TX_PKT_CNT:  0x%08x\n", FUNCTION_NAME(), ulPcTxPktCnt));
  DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: PC_TX_BYTE_CNT: 0x%08x\n", FUNCTION_NAME(), ulPcTxByteCnt));
  DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: PC_RX_PKT_CNT:  0x%08x\n", FUNCTION_NAME(), ulPcRxPktCnt));
  DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: PC_RX_BYTE_CNT: 0x%08x\n", FUNCTION_NAME(), ulPcRxByteCnt));
  /* The following are the expected Packet and Byte Counts */
  DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: DIAG_PKT_CNT:   0x%08x\n", FUNCTION_NAME(), pDiagsInfo->uPackets));
  DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: DIAG_BYTE_CNT:  0x%08x\n", FUNCTION_NAME(), ulDiagByteCnt));

  if (ulPcTxPktCnt != ulPcRxPktCnt) {
    printk("ERROR: %s Tx Pkt Count(0x%x, %d) does not match Rx Pkt Count(0x%x, %d)\n", FUNCTION_NAME(),
           ulPcTxPktCnt, ulPcTxPktCnt, ulPcRxPktCnt, ulPcRxPktCnt);
    rv = -1;
  }

  if (ulPcTxByteCnt != ulPcRxByteCnt) {
    printk("ERROR: %s Tx Byte Count(0x%x, %d) does not match Rx Byte Count(0x%x, %d)\n", FUNCTION_NAME(),
           ulPcTxByteCnt, ulPcTxByteCnt, ulPcRxByteCnt, ulPcRxByteCnt);
    rv = -1;
  }

  if (pDiagsInfo->uPackets != ulPcTxPktCnt) {
    printk("ERROR: %s Tx Pkt Count(0x%x, %d) does not match Diag Pkt Count(0x%x, %d)\n", FUNCTION_NAME(),
           ulPcTxPktCnt, ulPcTxPktCnt, pDiagsInfo->uPackets, pDiagsInfo->uPackets);
    rv = -1;
  }

  if (ulDiagByteCnt != ulPcTxByteCnt) {
    printk("ERROR: %s Tx Byte Count(0x%x, %d) does not match Diag Byte Count(0x%x, %d)\n", FUNCTION_NAME(),
           ulPcTxByteCnt, ulPcTxByteCnt, ulDiagByteCnt, ulDiagByteCnt);
    rv = -1;
  }
#endif

  DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: DIAG_TX_PKT_CNT:  0x%08x\n", FUNCTION_NAME(), pDiagsInfo->uTxPktCnt));
  DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: DIAG_TX_BYTE_CNT: 0x%08x\n", FUNCTION_NAME(), pDiagsInfo->uTxByteCnt));
  DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: DIAG_RX_PKT_CNT:  0x%08x\n", FUNCTION_NAME(), pDiagsInfo->uRxPktCnt));
  DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: DIAG_RX_BYTE_CNT: 0x%08x\n", FUNCTION_NAME(), pDiagsInfo->uRxByteCnt));
  if (pDiagsInfo->uDualKa) {
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: DIAG_RX_PKT_CNT:  0x%08x\n", FUNCTION_NAME(), pDiagsInfo->uRxPktCnt1));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: DIAG_RX_BYTE_CNT: 0x%08x\n", FUNCTION_NAME(), pDiagsInfo->uRxByteCnt1));
  }

  if (pDiagsInfo->uTxPktCnt != pDiagsInfo->uRxPktCnt) {
    printk("ERROR: [%s:%d] Tx Pkt Count(0x%x, %d) does not match Rx Pkt Count(0x%x, %d) for unit%d\n", FUNCTION_NAME(), __LINE__,
           pDiagsInfo->uTxPktCnt, pDiagsInfo->uTxPktCnt, pDiagsInfo->uRxPktCnt, pDiagsInfo->uRxPktCnt, unit);
    rv = -1;
  }

  if (pDiagsInfo->uTxByteCnt != pDiagsInfo->uRxByteCnt) {
    printk("ERROR: [%s:%d] Tx Byte Count(0x%x, %d) does not match Rx Byte Count(0x%x, %d) for unit%d\n", FUNCTION_NAME(), __LINE__,
           pDiagsInfo->uTxByteCnt, pDiagsInfo->uTxByteCnt, pDiagsInfo->uRxByteCnt, pDiagsInfo->uRxByteCnt, unit);
    rv = -1;
  }

  /* There are instances when the pc_error1 register is set because of rxbuf_fifo_underflow. If the
   * counts above, do not show a mismatch this error is more of an info than an error. Here is a 
   * note from the QE Specs.
   * #######################################################################################
   * NOTE: The RXBUF_FIFO_UNDERFLOW bit in the PCI_ERROR1 register indicates that all the
   * addresses loaded into the RxBuffer FIFO have been used up. It is not an error; it
   * indicates that the Receive path (QE) cannot transfer out any more data until the processor
   * has loaded in more buffers to the RxBuffer FIFO.
   * #######################################################################################
   * In such a situation, the rxbuf_fifo_underflow field in the Error register should be cleared.
   */
  if (rv == 0) {
    ulData = SAND_HAL_READ(qehdl, KA, PC_ERROR1);
    if (SAND_HAL_GET_FIELD(KA, PC_ERROR1, RXBUF_FIFO_UNDERFLOW, ulData) == 1) {
      /* Write 1 to Clear the field. */
      SAND_HAL_RMW_FIELD(qehdl, KA, PC_ERROR1, RXBUF_FIFO_UNDERFLOW, 1);
      /* printk("INFO: %s Clearing the PC_ERROR1 register\n", FUNCTION_NAME()); */
    }
  }

  if (pDiagsInfo->uDualKa) {
    sbhandle qehdl1; /* Handle to KA1 */

    if (pDiagsInfo->uTxPktCnt != pDiagsInfo->uRxPktCnt) {
      printk("ERROR: [%s:%d] Tx Pkt Count(0x%x, %d) does not match Rx Pkt Count(0x%x, %d) for unit%d\n", FUNCTION_NAME(), __LINE__,
             pDiagsInfo->uTxPktCnt, pDiagsInfo->uTxPktCnt, pDiagsInfo->uRxPktCnt1, pDiagsInfo->uRxPktCnt1, (unit +1));
      rv = -1;
    }

    if (pDiagsInfo->uTxByteCnt != pDiagsInfo->uRxByteCnt) {
      printk("ERROR: [%s:%d] Tx Byte Count(0x%x, %d) does not match Rx Byte Count(0x%x, %d) for unit%d\n", FUNCTION_NAME(), __LINE__,
             pDiagsInfo->uTxByteCnt, pDiagsInfo->uTxByteCnt, pDiagsInfo->uRxByteCnt1, pDiagsInfo->uRxByteCnt1, (unit +1));
      rv = -1;
    }

    qehdl1 = SOC_SBX_CONTROL(unit +1)->sbhdl;
    /* There are instances when the pc_error1 register is set because of rxbuf_fifo_underflow. If the
     * counts above, do not show a mismatch this error is more of an info than an error. Here is a 
     * note from the QE Specs.
     * #######################################################################################
     * NOTE: The RXBUF_FIFO_UNDERFLOW bit in the PCI_ERROR1 register indicates that all the
     * addresses loaded into the RxBuffer FIFO have been used up. It is not an error; it
     * indicates that the Receive path (QE) cannot transfer out any more data until the processor
     * has loaded in more buffers to the RxBuffer FIFO.
     * #######################################################################################
     * In such a situation, the rxbuf_fifo_underflow field in the Error register should be cleared.
     */
    if (rv == 0) {
      ulData = SAND_HAL_READ(qehdl1, KA, PC_ERROR1);
      if (SAND_HAL_GET_FIELD(KA, PC_ERROR1, RXBUF_FIFO_UNDERFLOW, ulData) == 1) {
        /* Write 1 to Clear the field. */
        SAND_HAL_RMW_FIELD(qehdl1, KA, PC_ERROR1, RXBUF_FIFO_UNDERFLOW, 1);
        /* printk("INFO: %s Clearing the PC_ERROR1 register\n", FUNCTION_NAME()); */
      }
    }
  }


  /* gsrao - 102208
   * Should probably put in some code here to list error registers if any.
   */

  /* Clean up the Data-structures */
  if (pDiagsInfo->pInFile) {
    sal_free(pDiagsInfo->pInFile);
  }
  if (pDiagsInfo->pOutFile) {
    sal_free(pDiagsInfo->pOutFile);
  }
  if (pDiagsInfo->pOutFile1) {
    sal_free(pDiagsInfo->pOutFile1);
  }
  if (pDiagsInfo->pPattern) {
    sal_free(pDiagsInfo->pPattern);
  }
  if (pDiagsInfo->pPayload) {
    sal_free(pDiagsInfo->pPayload);
  }
  if (pDiagsInfo->pNoincr) {
    sal_free(pDiagsInfo->pNoincr);
  }
  
  sal_free(p);

  DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: done...\n", FUNCTION_NAME()));

  return rv;
}


/* gsrao - 062110 */
/* If the user has specified the Link BitMap, then ensure that the corresponding
 * Link-Pair is set the same way.  If not then Flag a warning and set it right.
 */
uint32_t
sbx_qe_prbs_process_sd_lbm(uint32_t uSdLbm)
{
  /* Serdes Link BitMap Pairing */
  /* Lane ->                                                 0  1   2   3  4  5  6  7  8   9  10 11 12  13  14  15  16  17 18 19 */
  /* Mapped To ->                                           19  4  11  17  1  7  8  5  6  12  18  2  9  15  16  13  14  3  10  0 */
  uint32_t uaSdLbm[SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS] = {19, 4, 11, 17, 1, 7, 8, 5, 6, 12, 18, 2, 9, 15, 16, 13, 14, 3, 10, 0};
  uint32_t uSdLbm_1 = uSdLbm;
  int nLink = 0;
  int nPair = 0;

  for (nLink=0; nLink < SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; nLink++) {
    nPair = uaSdLbm[nLink];
    if (((0x1 << nLink) & uSdLbm_1) == (0x1 << nLink)) {
      /* Link is enabled, ensure that its pair is enabled too. */
      if (((0x1 << nPair) & uSdLbm_1) == (0x1 << nPair)) {
        continue;
      } else {
        printk("ERROR: Link %2d is disabled while its Pair, %2d, is enabled.\n",
               nPair, nLink);
        /* Disable the Link so it is the same as its Link-Pair. */
        uSdLbm_1 &= ~(0x1 << nLink);
      }
    } else { /* (((0x1 << nLink) & uSdLbm_1) != (0x1 << nLink)) */
      /* Link is disabled, ensure that its pair is disabled too. */
      if (((0x1 << nPair) & uSdLbm_1) != (0x1 << nPair)) {
        continue;
      } else {
        printk("ERROR: Link %2d is disabled while its Pair, %2d, is enabled.\n",
               nLink, nPair);
        /* Disable the Link-Pair so it is the same as the Link in question. */
        uSdLbm_1 &= ~(0x1 << nPair);
      }
    }
  }

  /* Return the Modified Link BitMap.  The calling routine will know if there
   * is a pairing mismatch by comparing this with the original.
   */
  return uSdLbm_1;
}


/* gsrao - 062110 */
int
sbx_qe_prbs_init(int unit, args_t *a, void **p)
{

  sbxQe2000PrbsInfo_t *pPrbsInfo = NULL;  
  parse_table_t pt;

  int nVerbose = 0;
  int bad_arg=0;

  uint32_t uNumIters = 0;
  uint32_t uRunTime = 0;
  uint32_t uSleepTime = 0;
  uint32_t uTimeOut = 0;
  uint32_t uPrintTime = 0;
  uint32_t uForceTestPass = 0;
  uint32_t uForceTestFail = 0;

  uint32_t uDualKa = 0;
  uint32_t uDo8b10b = 0;
  uint32_t uUsePrbsPoly15 = 0;
  uint32_t uExitOnError;                          /* If set, Exit test on First Error */ 
  uint32_t uUseSweep;                             /* If set, use the range values instead of TupleList */ 

  char* pTupleList = NULL;

                                                  /* sf1_si_config3: 0x08200000 */             
  uint32_t uDeqLo;                                /*     deq:  0x00000000 4Bits */
  uint32_t uDeqHi;                                
  uint32_t uDtxLo;                                /*     dtx:  0x00000008 4Bits */
  uint32_t uDtxHi;                               
  uint32_t uLoDrvLo;                              /*   lodrv:  0x00000001 1Bit  */
  uint32_t uLoDrvHi;                            

  uint32_t uSdLbm = 0;
  uint32_t uSdLbm_1 = 0;

  int rv = -1;

  DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: called...\n", FUNCTION_NAME()));

  if (!(SOC_IS_SBX_QE2000(unit))) {
    printk("ERROR: [%s:%d]: This test can only be run on the QE2000 Device.\n", FUNCTION_NAME(), __LINE__);
    return rv;
  }

  parse_table_init(unit, &pt);
  /* parse_table_add(&pt, "numiters",  PQ_INT, (void *) 5,    &uNumIters,  NULL); */
  /* parse_table_add(&pt, "sleeptime", PQ_INT, (void *) 5,    &uSleepTime, NULL); */
  /* parse_table_add(&pt, "timeout",   PQ_INT, (void *) 5,    &uTimeOut,   NULL); */

  parse_table_add(&pt, "runtime",   PQ_INT, (void *) 5,    &uRunTime,   NULL);
  parse_table_add(&pt, "verbose",   PQ_INT, (void *) 0,    &nVerbose,   NULL);

  parse_table_add(&pt, "printtime",     PQ_INT, (void *) 0,    &uPrintTime,     NULL);
  parse_table_add(&pt, "forcetestpass", PQ_INT, (void *) 0,    &uForceTestPass, NULL);
  parse_table_add(&pt, "forcetestfail", PQ_INT, (void *) 0,    &uForceTestFail, NULL);

  parse_table_add(&pt, "dualka",    PQ_BOOL | PQ_NO_EQ_OPT, (void *) 0x0,  &uDualKa,  NULL);
  parse_table_add(&pt, "do8b10b",   PQ_BOOL | PQ_NO_EQ_OPT, (void *) 0x1,  &uDo8b10b, NULL);

  parse_table_add(&pt, "useprbspoly15",   PQ_BOOL | PQ_NO_EQ_OPT, (void *) 0x1,  &uUsePrbsPoly15, NULL);
  parse_table_add(&pt, "exitonerror",     PQ_BOOL | PQ_NO_EQ_OPT, (void *) 0x0,  &uExitOnError,   NULL);
  parse_table_add(&pt, "usesweep",        PQ_BOOL | PQ_NO_EQ_OPT, (void *) 0x0,  &uUseSweep,   NULL);


  /* sf1_si_config3: 0x08200000
   *                                   deq:  0x00000000 4Bits
   *                                   dtx:  0x00000008 4Bits
   *                                 hidrv:  0x00000000 1Bit
   *                                 lodrv:  0x00000001 1Bit
   *                        serdes_pwrdown:  0x00000000 1Bit
   *                          serdes_reset:  0x00000000 1Bit
   */
  /* The TupleList is a comma seperated list of sfx_si_config3 fields values, i.e.
   * {lodrv, dtx, deq} e.g. "1_8_15,0_7_14,0_3_5". These values will be extracted
   * into a list and the PRBS test will be run for each of these values.
   */
  parse_table_add(&pt, "tuplelist", PQ_STRING, (void *) "1_8_15", &pTupleList, NULL);

  parse_table_add(&pt, "lodrvlo",  PQ_HEX, (void *) 0x1,  &uLoDrvLo,   NULL);
  parse_table_add(&pt, "lodrvhi",  PQ_HEX, (void *) 0x1,  &uLoDrvHi,   NULL);
  parse_table_add(&pt, "dtxlo",    PQ_HEX, (void *) 0x8,  &uDtxLo,     NULL);
  parse_table_add(&pt, "dtxhi",    PQ_HEX, (void *) 0x8,  &uDtxHi,     NULL);
  parse_table_add(&pt, "deqlo",    PQ_HEX, (void *) 0x0,  &uDeqLo,     NULL);
  parse_table_add(&pt, "deqhi",    PQ_HEX, (void *) 0xf,  &uDeqHi,     NULL);

  /* This is the SerDes Lane Bit Map. There are 18 SF Lanes and 2 SC Lanes. */
  parse_table_add(&pt, "sd_lbm",   PQ_HEX, (void *) 0xfffff,   &uSdLbm,    NULL);

  if (( pPrbsInfo = sal_alloc(sizeof(*pPrbsInfo), "prbs-info")) == 0) {
    test_error(unit,"Out of Memory\n");
    goto done;
  }
  sal_memset(pPrbsInfo, 0, sizeof(*pPrbsInfo));

  if (parse_arg_eq(a, &pt) < 0) {
    printk("ERROR: %s: Invalid option: %s\n", ARG_CMD(a), ARG_CUR(a));
    bad_arg=1;
    goto done;
  }

#ifdef BROADCOM_DEBUG
  /* User override of debug messages */
  if (nVerbose > 0) {
    diag_debug_level |= (DIAG_DBG_VERBOSE | DIAG_DBG_TESTS);
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: User change: diag_debug_level: 0x%x\n", FUNCTION_NAME(), diag_debug_level));
  }
#endif

  /* If the user has specified certain links to run the test on, ensure that the
   * corresponding link-pairs are setup the same way. Flag an error if this is not
   * so.
   */
  uSdLbm_1 = sbx_qe_prbs_process_sd_lbm(uSdLbm);
  if (uSdLbm_1 != uSdLbm) {
    printk("ERROR: Please fix SERDES Link BitMap before running the test.\n");
    goto done;
  }

  if (DIAG_DEBUG_CHECK(DIAG_DBG_VERBOSE)) {
    /* DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: uNumIters      : %d\n",   FUNCTION_NAME(), uNumIters     )); */
    /* DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: uSleepTime     : %d\n",   FUNCTION_NAME(), uSleepTime    )); */
    /* DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: uTimeOut       : %d\n",   FUNCTION_NAME(), uTimeOut      )); */

    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: uRunTime        : %d\n",   FUNCTION_NAME(), uRunTime       ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: nVerbose        : %d\n",   FUNCTION_NAME(), nVerbose       ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: uPrintTime      : %d\n",   FUNCTION_NAME(), uPrintTime     ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: uForceTestPass  : %d\n",   FUNCTION_NAME(), uForceTestPass ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: uForceTestFail  : %d\n",   FUNCTION_NAME(), uForceTestFail ));
                                                                                                         
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: uDualKa         : %d\n",   FUNCTION_NAME(), uDualKa        ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: uDo8b10b        : %d\n",   FUNCTION_NAME(), uDo8b10b       ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: uUsePrbsPoly15  : %d\n",   FUNCTION_NAME(), uUsePrbsPoly15 ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: uExitOnError    : %d\n",   FUNCTION_NAME(), uExitOnError   ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: uUseSweep       : %d\n",   FUNCTION_NAME(), uUseSweep      ));
                                                                                                         
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: pTupleList      : %s\n",   FUNCTION_NAME(), pTupleList     ));
                                                                                                         
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: uLoDrvLo        : 0x%x\n", FUNCTION_NAME(), uLoDrvLo       ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: uLoDrvHi        : 0x%x\n", FUNCTION_NAME(), uLoDrvHi       ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: uDtxLo          : 0x%x\n", FUNCTION_NAME(), uDtxLo         ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: uDtxHi          : 0x%x\n", FUNCTION_NAME(), uDtxHi         ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: uDeqLo          : 0x%x\n", FUNCTION_NAME(), uDeqLo         ));
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: uDeqHi          : 0x%x\n", FUNCTION_NAME(), uDeqHi         ));
                                                                                                         
    DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: uSdLbm          : 0x%x\n", FUNCTION_NAME(), uSdLbm         ));

    /* The following will print the arguments */
    parse_eq_format(&pt);
  }
  
  if (ARG_CNT(a) != 0) {
    printk("ERROR: %s: extra options starting with \"%s\" \n", ARG_CMD(a), ARG_CUR(a));
    bad_arg=1;
    goto done;
  }

  pPrbsInfo->userDeviceHandle = SOC_SBX_CONTROL(unit)->sbhdl;
  pPrbsInfo->unit = unit;

  /* gsrao 070910
   * The following are not currently used.
   */
  pPrbsInfo->uNumIters = uNumIters;
  pPrbsInfo->uSleepTime = uSleepTime;
  pPrbsInfo->uTimeOut = uTimeOut;
  pPrbsInfo->uPrintTime = uPrintTime;
  pPrbsInfo->uForceTestPass = uForceTestPass;
  pPrbsInfo->uForceTestFail = uForceTestFail;

  pPrbsInfo->uRunTime = uRunTime;
  pPrbsInfo->debug_level = nVerbose;

  pPrbsInfo->uDualKa  = uDualKa;  /* Test is being run on KA Benchscreen Board with Dual KA */
  pPrbsInfo->uDo8b10b = uDo8b10b;
  pPrbsInfo->uUsePrbsPoly15 = uUsePrbsPoly15;
  pPrbsInfo->uExitOnError = uExitOnError;
  pPrbsInfo->uUseSweep = uUseSweep;

  pPrbsInfo->uLoDrvLo = uLoDrvLo;
  pPrbsInfo->uLoDrvHi = uLoDrvHi;
  pPrbsInfo->uDtxLo   = uDtxLo;
  pPrbsInfo->uDtxHi   = uDtxHi;
  pPrbsInfo->uDeqLo   = uDeqLo;
  pPrbsInfo->uDeqHi   = uDeqHi;

  pPrbsInfo->uSdLbm = uSdLbm;       

  if (!SOC_SBX_INIT(unit)) {
    test_error(unit,"SOC Layer initialization not done for unit %d. Do \"init soc\" in sbx.soc script\n", unit);
    goto done;
  }

  if (!(bcm_init_check(unit) )) {
    test_error(unit,"BCM Layer initialization not done for unit %d. Do \"init bcm\" in sbx.soc script\n", unit);
    goto done;
  }

  /* If the user has specified dual KA then verify that both the devices have 
   * been attached and initialized.
   */
  if (pPrbsInfo->uDualKa) {
    /* First verify if we have the right kind of board and right kind of devices. */
    if (soc_ndev != 2) {
      printk("ERROR: [%s:%d]: This test for dual KA can only be run on the dual KA Benchscreen board.\n",
              FUNCTION_NAME(), __LINE__);
      goto done;
    }
    /* The Dual KA test can only be run from unit 0 */
    if (unit != 0) {
      printk("ERROR: [%s:%d]: This test for dual KA can only be run on unit 0.\n", FUNCTION_NAME(), __LINE__);
      goto done;
    }
    if (!(soc_attached(unit)) || !(soc_attached(unit +1))) {
      printk("ERROR: [%s:%d]: This test for dual KA can only be run when both KA devices are attached.\n",
              FUNCTION_NAME(), __LINE__);
      printk("ERROR: [%s:%d]:   Type 'attach' at BCM> prompt to see attached devices.\n",
              FUNCTION_NAME(), __LINE__);
      goto done;
    }
    /* The other unit must also be a KA device */
    if (!(SOC_IS_SBX_QE2000(unit +1))) {
      printk("ERROR: [%s:%d]: This test for dual KA can only be run if both devices are KA devices\n",
             FUNCTION_NAME(), __LINE__);
      goto done;
    }

    /* The other unit must also be initialized */
    if (!SOC_SBX_INIT(unit +1)) {
      test_error(unit,"SOC Layer initialization not done for unit %d. Do \"init soc\" in sbx.soc script\n", unit +1);
      goto done;
    }
   
    if (!(bcm_init_check(unit +1) )) {
      test_error(unit,"BCM Layer initialization not done for unit %d. Do \"init bcm\" in sbx.soc script\n", unit +1);
      goto done;
    }
  }

  /* Allocate space for the various string arguments */
  pPrbsInfo->pTupleList = sal_alloc((sizeof(char) * sal_strlen(pTupleList)+1), "Qe2000Prbs Tuple List");
  if (!pPrbsInfo->pTupleList) {
    printk("ERROR: [%s:%d] Out of Memory \n", FUNCTION_NAME(), __LINE__);
    goto done;
  }
  sal_strcpy(pPrbsInfo->pTupleList, pTupleList);
  DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: pPrbsInfo->pTupleList: %s\n", FUNCTION_NAME(), pPrbsInfo->pTupleList));

  *p = pPrbsInfo;
  rv = 0;

  done:

  if (bad_arg) {
    printk("valid test args:\n");
    parse_eq_format(&pt);
  }

  parse_arg_eq_done(&pt);
  if (rv < 0 && pPrbsInfo) {
    sal_free(pPrbsInfo);
  }

  DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: done...\n", FUNCTION_NAME()));
  return rv;
}


/* gsrao - 062110 */
/* foreach (list{lodrv, dtx, deq}) {
 *   disable PRBS Monitor
 *   change {lodrv, dtx, deq}
 *   enable PRBS Monitor
 *   sleep for y seconds
 *   check for errors
 * }
 */
int
sbx_qe_prbs_test(int unit, args_t *a, void *p)
{

  int rv = 0;
  sbxQe2000PrbsInfo_t *pPrbsInfo = p;

  DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: called...\n", FUNCTION_NAME()));

  if (SOC_IS_SBX_QE2000(unit)) {
    rv = sbQe2000DiagsPrbsTest(pPrbsInfo);
  } else {
    printk("ERROR: [%s:%d]: This test can only be run on the QE2000 Device.\n", FUNCTION_NAME(), __LINE__);
    rv = -1;
  }

  DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: done...\n", FUNCTION_NAME()));

  return rv;
}


/* gsrao - 062110 */
int
sbx_qe_prbs_done(int unit, void *p)
{
  sbxQe2000PrbsInfo_t *pPrbsInfo = p;
  int rv = 0;

  DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: called...\n", FUNCTION_NAME()));

  if (!pPrbsInfo) {
    printk("ERROR: %s Null pPrbsInfo found\n", FUNCTION_NAME());
    return -1;
  }    

  /* Clean up the Data-structures */
  /*
  if (pDiagsInfo->pInFile) {
    sal_free(pDiagsInfo->pInFile);
  }
  
  sal_free(p);
  */

  DIAG_DEBUG(DIAG_DBG_VERBOSE, ("DEBUG: %s: done...\n", FUNCTION_NAME()));

  return rv;
}
#endif /* BCM_SBX_SUPPORT */
