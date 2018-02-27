/*
 * $Id: fe2000_diags.c 1.8.102.1 Broadcom SDK $
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
 * File:        fe2000_diags.c
 * Purpose:     FE-2000-specific diagnostics tests, cleanup pending
 * Requires:
 */

#ifdef BCM_FE2000_SUPPORT

#include <appl/test/fe2000_diags.h>
#include "sbFe2000Util.h"
#include "sbFe2000InitUtils.h"
#include "fe2k-asm2-intd.h"
#include "sbFe2000Init.h"
#include <bcm/port.h>
#include <bcm/l2.h>
#include <soc/sbx/fe2k/sbZfCaDiagUcodeCtl.hx>
#include <soc/sbx/fe2k/sbZfCaDiagUcodeShared.hx>
#include <soc/sbx/fe2k/sbZfFe2000MmInternal0MemoryEntry.hx>
#include <soc/sbx/fe2k/sbZfFe2000MmInternal1MemoryEntry.hx>
#include <appl/diag/sbx/register.h>
#include <soc/sbx/sbx_txrx.h>
#include <soc/sbx/sbx_txrx.h>
#include <appl/diag/sbx/register.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_ka_auto.h>
#ifndef __KERNEL__
#include <signal.h>
#endif
#include <bcm/error.h>
#include <appl/diag/sbx/brd_sbx.h>
#include <appl/diag/test.h>
#include <bcm/vlan.h>


extern cmd_result_t sbx_diag_write_reg(int unit, soc_sbx_reg_info_t *reg_info, uint32_t v);
extern cmd_result_t sbx_diag_read_reg(int unit, soc_sbx_reg_info_t *reg_info, uint32_t *v);
extern int soc_sbx_counter_stop(int unit);
extern int bcm_stk_modid_get(int unit, int *);
/* some g2p3 required */
#include <soc/sbx/g2p3/g2p3.h>

/* default lrp microcode */
fe2kAsm2IntD sbFeG2DefaultUcode;

/* lrp ucode phases */
fe2kAsm2IntD fe2kdiag_phase0;
fe2kAsm2IntD fe2kdiag_phase1;
fe2kAsm2IntD fe2kdiag_phase2;

extern int parse_macaddr(char *str, sal_mac_addr_t macaddr);
extern cmd_result_t cmd_soc_sbx_tx(int unit, args_t *args);
extern cmd_result_t cmd_soc_sbx_rx(int unit, args_t *args);
extern cmd_result_t sbx_clear_all(int unit, uint8_t bErrorsOnly);
extern int bcm_vlan_create(int unit, bcm_vlan_t vid);
extern int bcm_vlan_port_add(int unit,	bcm_vlan_t vid, bcm_pbmp_t pbmp, bcm_pbmp_t ubmp);
extern int bcm_vlan_destroy_all(int unit);
extern int soc_sbx_fe2000_am_read(int unit, int32 block_num, int32 block_offset, uint32 reg, uint32 *data);
static sbFe2000InitParams_t Fe2000InitParams;
extern uint32_t sbFe2000UcodeLoad(sbhandle userDeviceHandle, uint8_t * cParams);
extern int fe2kAsm2IntD__initDefault(fe2kAsm2IntD *a_p);
extern int fe2kAsm2IntD__initDefaultPhase0(fe2kAsm2IntD *a_p);
extern int fe2kAsm2IntD__initDefaultPhase1(fe2kAsm2IntD *a_p);
extern int fe2kAsm2IntD__initDefaultPhase2(fe2kAsm2IntD *a_p);

/* C2 front door provisioning works on C1 also */
extern int sbFe2kxtDiagsSnakeTest_vlan_setup(sbxDiagsInfo_t *pDiagsInfo);
extern int sbFe2kxtDiagsSnakeTest_inject_packets( sbxDiagsInfo_t *pDiagsInfo, int tx_unit, int sp, int ep);
extern int sbFe2kxtDiagsSnakeTest_l2_create(sbxDiagsInfo_t *pDiagsInfo);
extern int sbFe2kxtDiagsSnakeTest_lpi_modify(sbxDiagsInfo_t *pDiagsInfo);

/* set on ctrl-c to stop test */
static uint8_t gStopSnakeTest = 0;
static uint8_t g_bInternalQueuesSetup[4] = {0};
static uint64_t uAGM0_RxCount = 0;
static uint64_t uAGM0_TxCount = 0;
static uint64_t uAGM1_RxCount = 0;
static uint64_t uAGM1_TxCount = 0;
static uint64_t uXG_TxCount = 0;
static uint64_t uXG_RxCount = 0;

/* used in loopback tests */
static uint32_t g_AGTxPorts[SB_FE2000_NUM_AG_INTERFACES][SB_FE2000_MAX_AG_PORTS] ={{0}};
static uint32_t g_AGRxPorts[SB_FE2000_NUM_AG_INTERFACES][SB_FE2000_MAX_AG_PORTS] ={{0}};

/* default patterns used in memory testing */
const uint64_t g_NarrowPortpattern = 0xAAAAAAAAALL;
const uint64_t g_WidePortpattern = 0xAAAAAAAAAAAAAAAALL;
const uint64_t g_NarrowPortantipattern = 0x555555555LL;
const uint64_t g_WidePortantipattern = 0x5555555555555555LL;

soc_sbx_reg_info_list_t *gFe2k_reginfo_l = NULL;
#define XE0_PORT 12
#define XE1_PORT 25
#define IS_ODD(p) (p%2)
#define IS_EVEN(p) (!IS_ODD(p))
#define SNAKE_VLAN_START 2
#define SNAKE_INTERVAL_US 1

/* for debug messages */
const char* sbFe2000DiagsPortInterfaceStr[] = {"SPI0","SPI1","AGM0",
					       "AGM1","XGM0","XGM1",
					       "PCI"};

const char* sbFe2000DiagsMemoryStr[] = {"NARROW_PORT_0",
					"NARROW_PORT_1",
					"WIDE_PORT"};

const char* sbFe2000DiagsMemoryTestStr[] = {"FE2000_MM0_NP0_DATA_BUS_WALKING_ONES",
					    "FE2000_MM1_NP0_DATA_BUS_WALKING_ONES",
					    "FE2000_MM0_NP0_DATA_BUS_WALKING_ZEROS",
					    "FE2000_MM1_NP0_DATA_BUS_WALKING_ZEROS",
					    "FE2000_MM0_NP0_ADDRESS_BUS",
					    "FE2000_MM1_NP0_ADDRESS_BUS",
					    "FE2000_MM0_NP1_DATA_BUS_WALKING_ONES",
					    "FE2000_MM1_NP1_DATA_BUS_WALKING_ONES",
					    "FE2000_MM0_NP1_DATA_BUS_WALKING_ZEROS",
					    "FE2000_MM1_NP1_DATA_BUS_WALKING_ZEROS",
					    "FE2000_MM0_NP1_ADDRESS_BUS",
					    "FE2000_MM1_NP1_ADDRESS_BUS",
					    "FE2000_MM0_WP_DATA_BUS_WALKING_ONES",
					    "FE2000_MM1_WP_DATA_BUS_WALKING_ONES",
					    "FE2000_MM0_WP_DATA_BUS_WALKING_ZEROS",
					    "FE2000_MM1_WP_DATA_BUS_WALKING_ZEROS",
					    "FE2000_MM0_WP_ADDRESS_BUS",
					    "FE2000_MM1_WP_ADDRESS_BUS",
					    "FE2000_MM0_NP0_RANDOM_MEMORY_TEST",
					    "FE2000_MM1_NP0_RANDOM_MEMORY_TEST",
					    "FE2000_MM0_NP1_RANDOM_MEMORY_TEST",
					    "FE2000_MM1_NP1_RANDOM_MEMORY_TEST",
					    "FE2000_MM0_WP_RANDOM_MEMORY_TEST",
					    "FE2000_MM1_WP_RANDOM_MEMORY_TEST"};



/* used in linerate memory test */
static char *MemStr[] =
  {
    "MM0 Narrow 0",
    "MM0 Narrow 1",
    "MM1 Narrow 0",
    "MM1 Narrow 1",
    "MM0 Wide",
    "MM1 Wide"
  };

#define GBITN(x,n) (((x) >> n) & 0x1)
#define ERR_A  0x300
#define ERR_D0 0x400
#define ERR_D1 0x500
#define RAM_SIZE_BASE  17
#define MM0_N0_ADDR_SZ 21
#define MM0_N1_ADDR_SZ 21
#define MM0_W_ADDR_SZ  21
#define MM1_N0_ADDR_SZ 21
#define MM1_N1_ADDR_SZ 21
#define MM1_W_ADDR_SZ  21
#define MAX_PES        48

int sbFe2000DiagsSnakeTest(sbxDiagsInfo_t *pDiagsInfo)
{
  uint32_t nAgm, uDataRx, uDataTx;
  int start_port, end_port, port;
  int dbg,i;
  int rv = 0;
  uint32_t unit;
  int timer_sec = 0;
  int fe_1 = -1; int qe_1 = -1;
  int fe_2 = -1; int qe_2 = -1;
  int qe_unit = -1;
  int port_status = 0;
  uint8_t bPortIsDown = FALSE;

  unit = pDiagsInfo->unit;
  dbg = pDiagsInfo->debug_level;
#ifndef __KERNEL__
  signal(SIGINT,(void *)sigcatcher);
#endif

  start_port = pDiagsInfo->start_port;
  end_port = pDiagsInfo->end_port;

  sbx_clear_all(unit,0);

  /* locate the FE's */
  for(i=0;i<soc_ndev;i++) {
    if (SOC_IS_SBX_FE2000(i)) {
      if (fe_1 == -1) {
	fe_1 = i; /* first fe */
      } else if (fe_2 == -1) {
	fe_2 = i; /* second fe */
      }
    }
  }

  /* locate the QE's */
  for (i=0;i<soc_ndev;i++) {
    if (SOC_IS_SBX_QE2000(i)) {
      if (qe_1 == -1) {
	qe_1 = i; /* first qe */
      } else if (qe_2 == -1) {
	qe_2 = i; /* 2nd qe */
      }
    }
  }

  SB_ASSERT((unit == fe_1) || (unit == fe_2));

  if (unit == fe_1) {
    qe_unit = qe_1;
  } else {
    qe_unit = qe_2;
  }


  if ( qe_unit == -1 ) {
    printk("ERROR Could not locate QE on board for this FE\n");
    return (-1);
  }

  soc_sbx_counter_stop(unit);

  /* initalize the ports */
    printk("Initializing ports..");
    rv = bcm_port_init(unit);
    if (BCM_FAILURE(rv)) {
      printk("ERROR: bcm_port_init failed(%d)(%s)\n",rv,bcm_errmsg(rv));
    }

    thin_delay(3*1E9);
    printk("\n");

  /* Set the error reg list to check for errors */
  sbxDiagsSetErrorRegList(pDiagsInfo,unit);

  /* initialize counters to zero */
  uAGM0_RxCount = 0; uAGM0_TxCount = 0;
  uAGM1_RxCount = 0; uAGM1_TxCount = 0;

  /* check that the ports being tested are up */
  for(port=(int)start_port;port<=(int)end_port;port++) {
    /* check status a few times to avoid issues with reporting
     * link down when it is not. PortStatus is known to not get the
     * correct values the first time it is run */
    for (i=0;i<5;i++) {
      rv = bcm_port_link_status_get(unit,port,&port_status);
      if (BCM_FAILURE(rv)) {
	printk("ERROR: bcm_port_link_status_get(%d)(%s)\n",rv,bcm_errmsg(rv));
	return (-1);
      }
      thin_delay(1000);
    }

    if (port_status == FALSE) {
      printk("ERROR Port:ge%d is down\n",port);
      bPortIsDown = TRUE;
    }
  }

  /* The test will not work if any of the ports between start_port, end_port is down */
  if (bPortIsDown) {
    return (-1);
  }

  /*
   *  Create the vlans, and assign ports to them.
   *  Also assign default vlans for the ports.(pvlan)
   *  Turn learning off for each vlan.
   */

  if (( rv = sbFe2kxtDiagsSnakeTest_vlan_setup(pDiagsInfo)) < 0 ) {
    test_error(unit,
	       "%s failed to setup vlans. \n", SOC_CHIP_STRING(unit));
    return(-1);
  }

  /*
   *  Create L2 entries required for test
   */

  if (( rv = sbFe2kxtDiagsSnakeTest_l2_create(pDiagsInfo)) < 0 ) {
    test_error(unit,
	       "%s: failed to create l2 entries. \n",SOC_CHIP_STRING(unit));
    return(-1);
  }

  /* avoid hairpin exception on ingress */
  if (( rv = sbFe2kxtDiagsSnakeTest_lpi_modify(pDiagsInfo)) < 0 ) {
    test_error(unit,
	       "%s: failed to modify lpi entries. \n",SOC_CHIP_STRING(unit));
    return(-1);
  }  

  /*
   *  Injects packets into QE to go out start port, and end port 
   */  

  for (port = pDiagsInfo->start_port; port <= pDiagsInfo->end_port; port+=2)
    {
      start_port = port;
      end_port = port+1;
      if (( rv = sbFe2kxtDiagsSnakeTest_inject_packets(pDiagsInfo,qe_unit,start_port,end_port)) < 0 ) {
	test_error(unit,
		   "%s: failed packet injection. \n",SOC_CHIP_STRING(unit));
	return(-1);
      }
    }


  /* If we are not looping continously, then the packet should come back to QE PCI */
  if (!pDiagsInfo->bSnakeLoopOn) {
    rv = sbFe2000DiagsReceivePciPacket(pDiagsInfo);
    if (rv != SB_FE2KXT_SUCCESS_E) {
      SB_ERROR("ERROR:%s sbFe2kxtDiagsRecievePciPacket failed with status %d\n",
	       FUNCTION_NAME(),rv);
      return (-1);
    }
  } else {
    printk("Looping packet for roughly %d seconds ..\n",pDiagsInfo->uSnakeRunTime);
    timer_sec = 0;
    while (!gStopSnakeTest && timer_sec < pDiagsInfo->uSnakeRunTime) {
      sal_usleep(SNAKE_INTERVAL_US * 1E6);
      rv = sbxDiagsPollForErrors(pDiagsInfo->start_port,pDiagsInfo->end_port,unit);
      if (rv != 0) {
	break;
      }
      timer_sec += 1;
    }

    /* check again for errors to be sure */
    if (rv == 0 ) {
      rv = sbxDiagsPollForErrors(pDiagsInfo->start_port,pDiagsInfo->end_port,unit);
      if (rv != 0) return (-1);
    }

    /* check that each port start_port to end_port tx/rx packets */
    uDataTx = 0; uDataRx = 0;
    for(port=(int)start_port;port<=(int)end_port;port++) {
      nAgm = (port <= 11) ? 0 : 1;
      if (( rv = soc_sbx_fe2000_am_read(unit,nAgm,port%12,REG_OFFSET(AM_TX_VALID_PKT_CNT),&uDataTx)) < 0 ) {
	SB_LOG("soc_sbx_fe2000_am_read, tx pkts failed (%d) \n",rv);
	/* sal_free(pArgs); */
	return rv;
      }

      if (( rv = soc_sbx_fe2000_am_read(unit,nAgm,port%12,REG_OFFSET(AM_RX_VALID_PKT_CNT),&uDataRx)) < 0 ) {
	SB_LOG("soc_sbx_fe2000_am_read, rx pkts failed (%d) \n",rv);
	/* sal_free(pArgs); */
	return rv;
      }

      if (uDataTx == 0) {
	SB_LOG("ERROR port:ge%d did not transmit any packets\n",port);
	rv = -1;
      }
      if (uDataRx == 0) {
	SB_LOG("ERROR port:ge%d did not recieve any packets\n",port);
	rv = -1;
      }
    }

    if((gStopSnakeTest || (timer_sec >= pDiagsInfo->uSnakeRunTime)) && rv == 0) {
      printk("\nStopping Test. No Errors Found during testing.\n");
    } else if (rv) {
      printk("\nTest stopped with errors, if errors persist try restarting bcm.user \n");
      printk("This test currently can not be run after TR18,19,20,116 or qe bist\n");
    }

    gStopSnakeTest = 0;

    /* need some way to stop the packet in flight */
    if (pDiagsInfo->bStopLoopingPacket) {
      for (port=pDiagsInfo->start_port;port<pDiagsInfo->end_port;port++) {
	bcm_port_enable_set(unit,port,0); 
	thin_delay(100);
	bcm_port_enable_set(unit,port,1); 
      }
    }
  }
  return rv;
}

/*
 *   PRBS (pseudorandom binary sequence) Tests
 *   QE ---> FE prbs test
 */

int sbFe2000DiagsSPIPrbsTest(sbxDiagsInfo_t *pDiagsInfo)

{

  sbhandle fehdl, qehdl;
  uint32_t nLSFR;
  uint32_t nInvert;
  uint32_t uData=0;
  int status=0;
  uint32_t fe;
  int qe = -1;
  int i;
  int qe_1 = -1;
  int qe_2 = -1;
  int fe_1 = -1;
  int fe_2 = -1;
  int nSpi;
  uint32_t start_spi;
  uint32_t end_spi;
  uint32_t uStatus0,uStatus1,uStatus2;

  nLSFR = pDiagsInfo->nLSFR;
  nInvert = pDiagsInfo->nInvert;
  fe = pDiagsInfo->unit;
  nSpi = pDiagsInfo->spi_interface;
  if (nSpi == -1) { /* do all */
    start_spi = 0;
    end_spi = SB_FE2000_NUM_SPI_INTERFACES;
  } else {
    start_spi = nSpi;
    end_spi = nSpi+1;
  }

  /* locate the FE's */
  for(i=0;i<soc_ndev;i++) {
    if (SOC_IS_SBX_FE2000(i)) {
      if (fe_1 == -1) {
	fe_1 = i; /* first fe */
      } else if (fe_2 == -1) {
	fe_2 = i; /* second fe */
      }
    }
  }

  /* locate the QE's */
  for (i=0;i<soc_ndev;i++) {
    if (SOC_IS_SBX_QE2000(i)) {
      if (qe_1 == -1) {
	qe_1 = i; /* first qe */
      } else if (qe_2 == -1) {
	qe_2 = i; /* 2nd qe */
      }
    }
  }

  SB_ASSERT(nLSFR == 0 || nLSFR == 1);
  SB_ASSERT(fe == fe_1 || fe == fe_2);

  /* need to find QE connected to this FE */
  if (fe == fe_1) {
    /* testing on the first fe, pair it with the first qe */
    qe = qe_1;
  }

  if (fe == fe_2) {
    /* 2nd fe, paired with 2nd qe */
    qe = qe_2;
  }

  if ( qe == -1 ) {
    SB_LOG("ERROR Could not QE on board for this FE\n");
    return (-1);
  }

  qehdl = SOC_SBX_CONTROL(qe)->sbhdl;
  fehdl = SOC_SBX_CONTROL(fe)->sbhdl;

  /* take st0, st1 out of reset */
  SAND_HAL_RMW_FIELD(qehdl, KA, PC_CORE_RESET0, ST0_CORE_RESET, 0);
  SAND_HAL_RMW_FIELD(qehdl, KA, ST0_CONFIG1, PC_RESET, 0);
  SAND_HAL_RMW_FIELD(qehdl, KA, ST0_CONFIG1, TSCLK_EDGE, 0);
  SAND_HAL_RMW_FIELD(qehdl, KA, PC_CORE_RESET0, ST1_CORE_RESET, 0);
  SAND_HAL_RMW_FIELD(qehdl, KA, ST1_CONFIG1, PC_RESET, 0);
  SAND_HAL_RMW_FIELD(qehdl, KA, ST1_CONFIG1, TSCLK_EDGE, 0);

  /* enable QE tx, force training */
  SAND_HAL_RMW_FIELD(qehdl,KA,ST0_CONFIG0,TX_ENABLE,1);
  SAND_HAL_RMW_FIELD(qehdl,KA,ST0_CONFIG0,TX_FORCE_TRAINING,1);
  SAND_HAL_RMW_FIELD(qehdl,KA,ST1_CONFIG0,TX_ENABLE,1);
  SAND_HAL_RMW_FIELD(qehdl,KA,ST1_CONFIG0,TX_FORCE_TRAINING,1);

 /* rx out of reset */
  SAND_HAL_RMW_FIELD(fehdl, CA, PC_CORE_RESET, SR0_CORE_RESET, 0);
  SAND_HAL_RMW_FIELD(fehdl, CA, PC_CORE_RESET, SR1_CORE_RESET, 0);
  SAND_HAL_RMW_FIELD(fehdl, CA, SR0_CONFIG0, PC_RESET, 0);
  SAND_HAL_RMW_FIELD(fehdl, CA, SR1_CONFIG0, PC_RESET, 0);

  /* enable rx at the FE side */
  for(nSpi = start_spi; nSpi < end_spi; nSpi++) {
      SAND_HAL_RMW_FIELD_STRIDE(fehdl,CA,SR,nSpi,SR_CONFIG0,RX_ENABLE,1);
      SB_LOG("Testing SPI%d interface QE(UNIT %d) --> FE(UNIT %d) LSRF=%d, invert=%d\n",
	     nSpi,qe,fe,nLSFR,nInvert);
  }

  /* remove force training */
   SAND_HAL_RMW_FIELD(qehdl, KA, ST0_CONFIG0, TX_FORCE_TRAINING, 0);
   SAND_HAL_RMW_FIELD(qehdl, KA, ST1_CONFIG0, TX_FORCE_TRAINING, 0);


  /* check that Rx is in sync */
  uData = SAND_HAL_READ(fehdl,CA,SR0_STATUS);
  if (SAND_HAL_GET_FIELD(CA,SR0_STATUS,RX_INSYNC,uData) != 1) {
      SB_LOG("Error SPI0 Rx is not in sync, sr0_status(%d) \n",uData);
      return (-1);
  }

  uData = SAND_HAL_READ(fehdl,CA,SR1_STATUS);
  if (SAND_HAL_GET_FIELD(CA,SR1_STATUS,RX_INSYNC,uData) != 1) {
      SB_LOG("Error SPI1 Rx is not in sync, sr1_status(%d) \n",uData);
      return (-1);
  }

  /* clear errors */
  SAND_HAL_RMW_FIELD(fehdl, CA, SR0_CONFIG1, PRBS_MONITOR_SYNC, 0);
  SAND_HAL_RMW_FIELD(fehdl, CA, SR1_CONFIG1, PRBS_MONITOR_SYNC, 0);
  uStatus0 = SAND_HAL_READ(fehdl, CA, SR0_PRBS_STATUS0);
  uStatus1 = SAND_HAL_READ(fehdl, CA, SR0_PRBS_STATUS1);
  uStatus2 = SAND_HAL_READ(fehdl, CA, SR0_PRBS_STATUS2);
  SAND_HAL_WRITE(fehdl, CA, SR0_PRBS_STATUS0, uStatus0);
  uStatus0 = SAND_HAL_READ(fehdl, CA, SR1_PRBS_STATUS0);
  uStatus1 = SAND_HAL_READ(fehdl, CA, SR1_PRBS_STATUS1);
  uStatus2 = SAND_HAL_READ(fehdl, CA, SR1_PRBS_STATUS2);
  SAND_HAL_WRITE(fehdl, CA, SR1_PRBS_STATUS0, uStatus0);

  /* settings at FE/QE need to match */
  for(nSpi = start_spi; nSpi < end_spi; nSpi++) {
      SAND_HAL_RMW_FIELD_STRIDE(fehdl,CA,SR,nSpi,SR_CONFIG1,PRBS_POLY_SEL,nLSFR);
  }

  SAND_HAL_RMW_FIELD(qehdl, KA, ST0_CONFIG2, PRBS_POLY_SEL, nLSFR);
  SAND_HAL_RMW_FIELD(qehdl, KA, ST1_CONFIG2, PRBS_POLY_SEL, nLSFR);

  for(nSpi = start_spi; nSpi < end_spi; nSpi++) {
      SAND_HAL_RMW_FIELD_STRIDE(fehdl,CA,SR,nSpi,SR_CONFIG1,PRBS_INVERT,nInvert);
  }

  SAND_HAL_RMW_FIELD(qehdl, KA, ST0_CONFIG2, PRBS_INVERT, nInvert);
  SAND_HAL_RMW_FIELD(qehdl, KA, ST1_CONFIG2, PRBS_INVERT, nInvert);

  /* enable the monitors */
  for(nSpi = start_spi; nSpi < end_spi; nSpi++) {
    SAND_HAL_RMW_FIELD_STRIDE(fehdl,CA,SR,nSpi,SR_CONFIG1,PRBS_MONITOR_SYNC,0);
    SAND_HAL_RMW_FIELD_STRIDE(fehdl,CA,SR,nSpi,SR_CONFIG1,PRBS_MONITOR_ENABLE,1);
  }

  /* enable the generators in the spit at the QE */
 SAND_HAL_RMW_FIELD(qehdl, KA, ST0_CONFIG2, PRBS_GENERATOR_ENABLE, 1);
 SAND_HAL_RMW_FIELD(qehdl, KA, ST1_CONFIG2, PRBS_GENERATOR_ENABLE, 1);

  /* re-synchronize the monitor */
  for(nSpi = start_spi; nSpi < end_spi; nSpi++) {
    SAND_HAL_RMW_FIELD_STRIDE(fehdl,CA,SR,nSpi,SR_CONFIG1,PRBS_MONITOR_SYNC,1);
  }

  thin_delay(10000);

  /* dump the error debug registers to see what bit failed */
  for(nSpi = start_spi; nSpi < end_spi; nSpi++) {
      uData = SAND_HAL_READ_STRIDE(fehdl,CA,SR,nSpi,SR_PRBS_STATUS1);
      if (uData) {
	  SB_LOG("SPI%d ERROR SR_PRBS_STATUS1 Mismatch Errors 0x%08x \n",nSpi,uData);
	  status = -1;
      }
      uData = 0;
      uData = SAND_HAL_READ_STRIDE(fehdl,CA,SR,nSpi,SR_PRBS_STATUS2);
      if (uData) {
	  SB_ERROR("SPI%d: ERROR SR_PRBS_STATUS2 Mismatch Errors 0x%08x\n",nSpi, uData);
	  status = -1;
      }
      uData = 0;
      uData = SAND_HAL_READ_STRIDE(fehdl,CA,SR,nSpi,SR_PRBS_STATUS0);
      if (uData & 0xf) {
	  SB_ERROR("SPI%d ERROR SR_PRBS_STATUS0 Mismatch Errors for SPI Control 0x%08x\n",nSpi, uData&0xf);
	  status = -1;
      }
  }

  if (status != 0) {
      SB_LOG("Failed.\n");
  } else {
      SB_LOG("Passed.\n");
  }


  /* cleanup, disable the monitor,generator any errors are cleared on read above */
  for(nSpi = 0; nSpi < SB_FE2000_NUM_SPI_INTERFACES; nSpi++) {
    SAND_HAL_RMW_FIELD_STRIDE(fehdl,CA,SR,nSpi,SR_CONFIG1,PRBS_MONITOR_ENABLE,0);
    SAND_HAL_RMW_FIELD_STRIDE(fehdl,CA,SR,nSpi,SR_CONFIG1,PRBS_MONITOR_SYNC,0);
  }

  /* disable the generators in the spit at the QE */
  SAND_HAL_RMW_FIELD(qehdl, KA, ST0_CONFIG2, PRBS_GENERATOR_ENABLE, 0);
  SAND_HAL_RMW_FIELD(qehdl, KA, ST1_CONFIG2, PRBS_GENERATOR_ENABLE, 0);

  DEXIT();
  return status;
}

/* run through all the memory tests */
int sbFe2000DiagsSramMemTestAll(sbxDiagsInfo_t *pDiagsInfo) {

  uint32_t mem_test;
  int stat = 0;
  int nMemTests = SB_FE2000_MEM_TEST_LAST-1;
  DENTER();

  SB_ASSERT(pDiagsInfo);
  for(mem_test=0;mem_test<SB_FE2000_MEM_TEST_LAST;mem_test++) {
    pDiagsInfo->e_mem_test = mem_test;
    if (strstr(sbFe2000DiagsMemoryTestStr[mem_test],"MM0") != NULL) {
      pDiagsInfo->uInstance = 0;
    } else {
      pDiagsInfo->uInstance = 1;
    }
    SB_LOG("Running Test[%2d..%2d] %-45s  .. ",mem_test,nMemTests,sbFe2000DiagsMemoryTestStr[mem_test]);
    if(sbFe2000DiagsSramMemTest(pDiagsInfo) != 0) {
      stat = -1;
      if (strstr(sbFe2000DiagsMemoryTestStr[mem_test],"RANDOM") != NULL) {
	SB_LOG("FAILED: To re-run use MemTest=%d Verbose=1 Seed=%d \n", mem_test,pDiagsInfo->seed);
      } else {
	SB_LOG("\nFAILED: To re-run use MemTest=%d Verbose=1 \n",mem_test);
      }
    } else {
      SB_LOG("PASSED\n");
    }
  }
  return stat;
}



/*
 * All SRAM Memory tests start here
 */

int sbFe2000DiagsSramMemTest(sbxDiagsInfo_t *pDiagsInfo)
{

  int status=0;
  char *pTest_str = NULL;
  sbhandle userDeviceHandle;
  uint32_t unit;
  int dbgLevel = pDiagsInfo->debug_level;
  DENTER();

  unit = pDiagsInfo->unit;
  userDeviceHandle = SOC_SBX_CONTROL(unit)->sbhdl;
  pDiagsInfo->userDeviceHandle = userDeviceHandle;

  /* be sure core is out of reset */
  sbFe2000UtilReleaseSoftReset(userDeviceHandle);
  thin_delay(100);

  /* override the memory protection scheme first */
  /* also check if background policer refresh is on we shut it off (test will turn it back on when done)*/
  if (pDiagsInfo->mem_override == 0 ) {
    sbFe2000DiagsOverrideMmParam(pDiagsInfo);
    pDiagsInfo->mem_override = 1;
  }

  switch(pDiagsInfo->e_mem_test) {
  case SB_FE2000_MM0_DATA_BUS_WALKING_ONES_NARROW_PORT0:
  case SB_FE2000_MM1_DATA_BUS_WALKING_ONES_NARROW_PORT0:
    pTest_str = "Walking Ones Data Bus Narrow Port0";
    if (dbgLevel >= 1)
      DISPLAY_TESTSTR(__PRETTY_FUNCTION__,pDiagsInfo->uInstance,pTest_str);
    pDiagsInfo->walkbit=1;
    pDiagsInfo->e_mem_type = SB_FE2000_NARROW_PORT_0;
    status = sbFe2000DiagsSramDataBusNarrowPortTest(pDiagsInfo);
    break;
  case SB_FE2000_MM0_DATA_BUS_WALKING_ZEROS_NARROW_PORT0:
  case SB_FE2000_MM1_DATA_BUS_WALKING_ZEROS_NARROW_PORT0:
    pTest_str = "Walking Zeros Data Bus on Narrow Port0 Memory";
    if (dbgLevel >= 1)
      DISPLAY_TESTSTR(__PRETTY_FUNCTION__,pDiagsInfo->uInstance,pTest_str);
    pDiagsInfo->walkbit=0;
    pDiagsInfo->e_mem_type = SB_FE2000_NARROW_PORT_0;
    status = sbFe2000DiagsSramDataBusNarrowPortTest(pDiagsInfo);
    break;
  case SB_FE2000_MM0_ADDRESS_BUS_NARROW_PORT0:
  case SB_FE2000_MM1_ADDRESS_BUS_NARROW_PORT0:
    pTest_str = "Address Bus on Narrow Port0 Memory";
    if (dbgLevel >= 1)
      DISPLAY_TESTSTR(__PRETTY_FUNCTION__,pDiagsInfo->uInstance,pTest_str);
    pDiagsInfo->e_mem_type = SB_FE2000_NARROW_PORT_0;
    status = sbFe2000DiagsSramAddressBusTest(pDiagsInfo);
    break;
  case SB_FE2000_MM0_DATA_BUS_WALKING_ONES_NARROW_PORT1:
  case SB_FE2000_MM1_DATA_BUS_WALKING_ONES_NARROW_PORT1:
    pTest_str = "Walking Ones Data Bus on Narrow Port1 Memory";
    if (dbgLevel >= 1)
      DISPLAY_TESTSTR(__PRETTY_FUNCTION__,pDiagsInfo->uInstance,pTest_str);
    pDiagsInfo->e_mem_type = SB_FE2000_NARROW_PORT_1;
    pDiagsInfo->walkbit = 1;
    status = sbFe2000DiagsSramDataBusNarrowPortTest(pDiagsInfo);
    break;
  case SB_FE2000_MM0_DATA_BUS_WALKING_ZEROS_NARROW_PORT1:
  case SB_FE2000_MM1_DATA_BUS_WALKING_ZEROS_NARROW_PORT1:
    pTest_str = "Walking Zeros Data Bus on Narrow Port1 Memory";
    if (dbgLevel >= 1)
      DISPLAY_TESTSTR(__PRETTY_FUNCTION__,pDiagsInfo->uInstance,pTest_str);
    pDiagsInfo->e_mem_type = SB_FE2000_NARROW_PORT_1;
    pDiagsInfo->walkbit = 0;
    status = sbFe2000DiagsSramDataBusNarrowPortTest(pDiagsInfo);
    break;
  case SB_FE2000_MM0_ADDRESS_BUS_NARROW_PORT1:
  case SB_FE2000_MM1_ADDRESS_BUS_NARROW_PORT1:
    pTest_str = "Address Bus on Narrow Port1 Memory";
    if (dbgLevel >= 1)
      DISPLAY_TESTSTR(__PRETTY_FUNCTION__,pDiagsInfo->uInstance,pTest_str);
    pDiagsInfo->e_mem_type = SB_FE2000_NARROW_PORT_1;
    status = sbFe2000DiagsSramAddressBusTest(pDiagsInfo);
    break;
  case SB_FE2000_MM0_DATA_BUS_WALKING_ONES_WIDE_PORT:
  case SB_FE2000_MM1_DATA_BUS_WALKING_ONES_WIDE_PORT:
    pTest_str = "Walking Ones Data Bus on Wide Port Memory";
    if (dbgLevel >= 1)
      DISPLAY_TESTSTR(__PRETTY_FUNCTION__,pDiagsInfo->uInstance,pTest_str);
    pDiagsInfo->walkbit=1;
    status = sbFe2000DiagsSramDataBusWidePortTest(pDiagsInfo);
    break;
  case SB_FE2000_MM0_DATA_BUS_WALKING_ZEROS_WIDE_PORT:
  case SB_FE2000_MM1_DATA_BUS_WALKING_ZEROS_WIDE_PORT:
    pTest_str = "Walking Zeros Data Bus on Wide Port Memory";
    if (dbgLevel >= 1)
      DISPLAY_TESTSTR(__PRETTY_FUNCTION__,pDiagsInfo->uInstance,pTest_str);
    pDiagsInfo->walkbit=0;
    status = sbFe2000DiagsSramDataBusWidePortTest(pDiagsInfo);
    break;
  case SB_FE2000_MM0_ADDRESS_BUS_WIDE_PORT:
  case SB_FE2000_MM1_ADDRESS_BUS_WIDE_PORT:
    pTest_str = "Address Bus on Wide Port Memory";
    if (dbgLevel >= 1)
      DISPLAY_TESTSTR(__PRETTY_FUNCTION__,pDiagsInfo->uInstance,pTest_str);
    pDiagsInfo->e_mem_type = SB_FE2000_WIDE_PORT;
    status = sbFe2000DiagsSramAddressBusTest(pDiagsInfo);
    break;
  case SB_FE2000_MM0_RAND_NARROW_PORT0:
  case SB_FE2000_MM1_RAND_NARROW_PORT0:
    pTest_str = "Random data write/read/verify test on narrow port0";
    if (dbgLevel >= 1)
      DISPLAY_TESTSTR(__PRETTY_FUNCTION__,pDiagsInfo->uInstance,pTest_str);
    pDiagsInfo->e_mem_type = SB_FE2000_NARROW_PORT_0;
    status = sbFe2000DiagsRandomDataMemTest(pDiagsInfo);
    break;
  case SB_FE2000_MM0_RAND_NARROW_PORT1:
  case SB_FE2000_MM1_RAND_NARROW_PORT1:
    pTest_str = "Random data write/read/verify test on narrow port1";
    if (dbgLevel >= 1)
      DISPLAY_TESTSTR(__PRETTY_FUNCTION__,pDiagsInfo->uInstance,pTest_str);
    pDiagsInfo->e_mem_type = SB_FE2000_NARROW_PORT_1;
    status = sbFe2000DiagsRandomDataMemTest(pDiagsInfo);
    break;
  case SB_FE2000_MM0_RAND_WIDE_PORT:
  case SB_FE2000_MM1_RAND_WIDE_PORT:
    pTest_str = "Random data write/read/verify test on wide port";
    if (dbgLevel >= 1)
      DISPLAY_TESTSTR(__PRETTY_FUNCTION__,pDiagsInfo->uInstance,pTest_str);
    pDiagsInfo->e_mem_type = SB_FE2000_WIDE_PORT;
    status = sbFe2000DiagsRandomDataMemTest(pDiagsInfo);
    break;
  default:
    SB_ERROR("ERROR:\n%s: Invalid mem test:%d [0-%d supported]\n",__PRETTY_FUNCTION__,
	     pDiagsInfo->e_mem_test,SB_FE2000_MEM_TEST_LAST-1);
    return -1;
  }

  DEXIT();
  return status;
}

int sbFe2000DiagsSramDataBusNarrowPortTest(sbxDiagsInfo_t *pDiagsInfo)

{

  sbhandle userDeviceHandle = pDiagsInfo->userDeviceHandle;
  uint32_t uInstance = pDiagsInfo->uInstance;
  volatile unsigned long address_ul = pDiagsInfo->start_addr;
  uint32_t walkbit = pDiagsInfo->walkbit;
  int status=0;
  int tst_status=0;
  uint32_t i;
  uint64_t ullData_wr=0;
  uint32_t data_bus_width;
  volatile uint64_t ullData_rd=0;
  uint32_t Retry=0;

  DENTER();
  /* sanity check */
  if (sbFe2000DiagsAddressCheck(pDiagsInfo) != 0) {
    return SB_FE2000_BAD_SIZE_E;
  }

  if (pDiagsInfo->e_mem_type == SB_FE2000_NARROW_PORT_0) {
      data_bus_width = pDiagsInfo->np0_config.data_width;
  } else {
      data_bus_width = pDiagsInfo->np1_config.data_width;
  }

  for(i=0;i<data_bus_width;i++) {
    if (walkbit == 1) {
      ullData_wr = (0x1 << i);
    } else {
      ullData_wr = 0x3ffff ^ (0x1 << i);
    }

    /* write the data */
    if (pDiagsInfo->debug_level >=1 ) {
      SB_LOG("%s Writing:0x%llx to address:0x%08lx\n",__PRETTY_FUNCTION__,ullData_wr,address_ul);
    }

  loopOnWriteError:
    status = sbFe2000DiagsMemWrite(userDeviceHandle,uInstance,address_ul,ullData_wr,pDiagsInfo->e_mem_type);
    if (status != SB_FE2000_SUCCESS_E ) {
      return status;
    }

    /* read to check the data */
    thin_delay(1000); /* allow the data bus to settle down to avoid false postives */

  loopOnReadError:
    status = sbFe2000DiagsMemRead(userDeviceHandle,uInstance,address_ul,&ullData_rd,pDiagsInfo->e_mem_type);
    if (status != SB_FE2000_SUCCESS_E ) {
      if (pDiagsInfo->bLoopOnError) {
	SB_LOG("Read Timeout MM%d,MEM_TYPE:%s, retry attempt %d .. -- ctrl-c to stop\r",
	       uInstance,sbFe2000DiagsMemoryStr[pDiagsInfo->e_mem_type],++Retry);
	pDiagsInfo->debug_level = 1;
	goto loopOnReadError;
      }
      return status;
    }

    /* check the data */
    if (ullData_rd != ullData_wr) {
      if (pDiagsInfo->bLoopOnError) {
	SB_ERROR("\r%s: MM%d Data mismatch: write data 0x%llx read data 0x%llx at address: 0x%08lx --- looping on error ctrl-c to stop", __PRETTY_FUNCTION__,uInstance,ullData_wr,ullData_rd,address_ul);
	pDiagsInfo->debug_level = 1;
	goto loopOnWriteError;
      } else {
	SB_ERROR("\nERROR:%s: MM%d Data mismatch: write data 0x%llx read data 0x%llx at address: 0x%08lx", __PRETTY_FUNCTION__,uInstance,ullData_wr,ullData_rd,address_ul);
      }
      tst_status = -1;
    } else {
      if (pDiagsInfo->debug_level >= 1) {
	SB_LOG("%s: GOOD Got:0x%llx Exp:0x%llx\n",__PRETTY_FUNCTION__,ullData_rd,ullData_wr);
      }
    }
  }

  DEXIT();
  return tst_status;
}

int sbFe2000DiagsSramAddressBusTest(sbxDiagsInfo_t *pDiagsInfo)
{

  sbhandle userDeviceHandle = pDiagsInfo->userDeviceHandle;
  uint32_t mmu = pDiagsInfo->uInstance;
  uint32_t mem = pDiagsInfo->e_mem_type;
  int status=0;
  volatile uint64_t ullData_rd=0;
  unsigned long ulWalkAddr;
  volatile unsigned long ulStartAddr;
  volatile unsigned long ulEndAddr;
  unsigned long ulAddrMover;
  int offset;
  int outer;
  uint64_t g_pattern;
  uint64_t g_antipattern;

  DENTER();

  ulStartAddr = pDiagsInfo->start_addr; /* default==0*/
  /* If an ending address was specified, use that otherwise assume maximum */
  if (!pDiagsInfo->bEndAddr) {
      if (pDiagsInfo->e_mem_type == SB_FE2000_NARROW_PORT_0) {
	  ulEndAddr = (1<<pDiagsInfo->np0_config.addr_width);
      } else if (pDiagsInfo->e_mem_type == SB_FE2000_NARROW_PORT_1) {
	  ulEndAddr = (1<<pDiagsInfo->np1_config.addr_width);
      } else {
	  ulEndAddr = (1<<pDiagsInfo->wp_config.addr_width);
      }
  } else {
      ulEndAddr = pDiagsInfo->end_addr;
  }


  /* sanity check */
  if (sbFe2000DiagsAddressCheck(pDiagsInfo) != 0) {
    return SB_FE2000_BAD_SIZE_E;
  }

  if (mem == SB_FE2000_WIDE_PORT) {
    g_pattern = g_WidePortpattern;
    g_antipattern = g_WidePortantipattern;
  } else {
    g_pattern = g_NarrowPortpattern;
    g_antipattern = g_NarrowPortantipattern;
  }

  /* Initialize memory */
  if (pDiagsInfo->mem_inited[mmu][mem] == 0) {
    if(sbFe2000DiagsInitMem(pDiagsInfo) != 0) {
      SB_ERROR("ERROR:%s: mem init:%s mmu%d failed\n",__PRETTY_FUNCTION__,
	       sbFe2000DiagsMemoryStr[mem],mmu);
      return (-1);
    }
    pDiagsInfo->mem_inited[mmu][mem] = 1;
  }


  /* Check for address bits stuck high
   * Write the inverted pattern to the first offset, then check all other power-of-2 addresses to be
   * sure the original pattern exists, that should be there when the memory was intialized
   */

 loopOnError0:
  status = sbFe2000DiagsMemWrite(userDeviceHandle,mmu,ulStartAddr,g_antipattern,mem);
  if (status != SB_FE2000_SUCCESS_E) {
    if (pDiagsInfo->bLoopOnError) {
      SB_LOG("Write timeout, trying again .. ctrl-c to stop\r");
      goto loopOnError0;
    }
    return status;
  }


  /* all the memory should have a data pattern of g_pattern, except the starting address, which
   * was written with the inverted data we check that writing the inverted data to the starting address
   * does not inadvently write that data to another memory address */

  for(ulWalkAddr = ulStartAddr,offset=0;ulWalkAddr<ulEndAddr;offset++) {
  loopOnError1:
    status = sbFe2000DiagsMemRead(userDeviceHandle,mmu,ulWalkAddr,&ullData_rd,mem);
    if (status != SB_FE2000_SUCCESS_E) {
      if (pDiagsInfo->bLoopOnError) {
	SB_LOG("Read timeout, trying again .. ctrl-c to stop\r");
	goto loopOnError1;
      }
      return status;
    }

    if (ulWalkAddr == ulStartAddr) { /* starting address has inverted data */
      if (ullData_rd != g_antipattern) {
	if (pDiagsInfo->bLoopOnError) {
	  SB_ERROR("\r%s:%d MM%d Data mismatch: exp 0x%llx got 0x%llx at address: 0x%lx -- looping on error ctrl-c to stop", __PRETTY_FUNCTION__,__LINE__,
		   mmu,g_antipattern,ullData_rd,ulStartAddr);
	  pDiagsInfo->debug_level=1;
	  goto loopOnError0;
	} else {
	  SB_ERROR("\nERROR:%s:%d MM%d Data mismatch: exp 0x%llx got 0x%llx at address: 0x%lx\n", __PRETTY_FUNCTION__,__LINE__,
		   mmu,g_antipattern,ullData_rd,ulStartAddr);
	  return (-1);
	}
      } else { /* for debug */
	if (pDiagsInfo->debug_level >=1 ) {
	  SB_LOG("%s:%d GOOD MM%d Got:0x%llx Exp:0x%llx at address: 0x%lx\n",__PRETTY_FUNCTION__,__LINE__,
		 mmu,g_antipattern,ullData_rd,ulStartAddr);
	}

      }
    } else {
      if (ullData_rd != g_pattern ) { /* all other memory locations should have g_pattern still */
	if (pDiagsInfo->bLoopOnError) {
	  SB_ERROR("\r%s:%d MM%d Data mismatch: exp 0x%llx got 0x%llx at address: 0x%lx -- looping on error ctrl-c to stop", __PRETTY_FUNCTION__,__LINE__,
		   mmu,g_pattern,ullData_rd,ulWalkAddr);
	  pDiagsInfo->debug_level=1;
	  goto loopOnError1;
	} else {
	  SB_ERROR("\nERROR:%s:%d MM%d Data mismatch: exp 0x%llx got 0x%llx at address: 0x%lx", __PRETTY_FUNCTION__,__LINE__,
		   mmu,g_pattern,ullData_rd,ulWalkAddr);
	  /* return (-1); */
	}
      } else { /* for debug */
	if (pDiagsInfo->debug_level >=1 ) {
	  SB_LOG("%s:%d GOOD MM%d Got:0x%llx Exp:0x%llx at address: 0x%lx\n",__PRETTY_FUNCTION__,__LINE__,
		 mmu,g_pattern,ullData_rd,ulWalkAddr);
	}
      }
    }

    /* go to the next power-of-2 address, this is equivalent of walking ones on the address bus */
    ulWalkAddr = ulStartAddr + (1<<offset);
  }

  /* put the original pattern back at the starting address */
  status = sbFe2000DiagsMemWrite(userDeviceHandle,mmu,ulStartAddr,g_pattern,mem);
  if (status != SB_FE2000_SUCCESS_E) {
    return status;
  }

  /* Check for address bits stuck low or shorted -- see header file for description */
  for(ulAddrMover = ulStartAddr,outer=0;ulAddrMover < ulEndAddr;outer++) {
    loopOnError2:
    status = sbFe2000DiagsMemWrite(userDeviceHandle,mmu,ulAddrMover,g_antipattern,mem);
    if (status != SB_FE2000_SUCCESS_E) {
      return status;
    }

    for(ulWalkAddr = ulStartAddr,offset=0;ulWalkAddr < ulEndAddr;offset++) {
      status = sbFe2000DiagsMemRead(userDeviceHandle,mmu,ulWalkAddr,&ullData_rd,mem);
      if (status != SB_FE2000_SUCCESS_E) {
	return status;
      }
      /* check the data */
      if (ulWalkAddr != ulAddrMover) {
	/* data here should be g_pattern */
	if (ullData_rd != g_pattern) {
	  if (pDiagsInfo->bLoopOnError) {
	    SB_ERROR("\r%s:%d MM%d Data mismatch: exp 0x%llx got 0x%llx at address: 0x%lx --looping on error ctrl-c to stop", __PRETTY_FUNCTION__,__LINE__,
		     mmu,g_pattern,ullData_rd,ulWalkAddr);
	    pDiagsInfo->debug_level=1;
	    goto loopOnError2;
	  } else {
	    SB_ERROR("\nERROR:%s:%d MM%d Data mismatch: exp 0x%llx got 0x%llx at address: 0x%lx", __PRETTY_FUNCTION__,__LINE__,
		     mmu,g_pattern,ullData_rd,ulWalkAddr);
	    return (-1);
	  }
	} else {
	  if (pDiagsInfo->debug_level >=1 ) {
	  SB_LOG("%s:%d GOOD MM%d Got:0x%llx Exp:0x%llx at address: 0x%lx\n",__PRETTY_FUNCTION__,__LINE__,
		 mmu,g_pattern,ullData_rd,ulWalkAddr);
	  }
	}
      } else {
	/* data here should be the inverted data */
	if (ullData_rd != g_antipattern) {
	  if (pDiagsInfo->bLoopOnError) {
	    SB_ERROR("\r%s:%d MM%d Data mismatch: exp 0x%llx got 0x%llx at address: 0x%lx --looping on error ctrl-c to stop", __PRETTY_FUNCTION__,__LINE__,
		     mmu,g_antipattern,ullData_rd,ulWalkAddr);
	    pDiagsInfo->debug_level = 1;
	    goto loopOnError2;
	  } else {
	    SB_ERROR("\nERROR:%s:%d MM%d Data mismatch: exp 0x%llx got 0x%llx at address: 0x%lx", __PRETTY_FUNCTION__,__LINE__,
		     mmu,g_antipattern,ullData_rd,ulWalkAddr);
	    return (-1);
	  }
	} else {
	  if (pDiagsInfo->debug_level >= 1) {
	    SB_LOG("%s:%d GOOD MM%d Got:0x%llx Exp:0x%llx at address: 0x%lx\n",__PRETTY_FUNCTION__,__LINE__,
		   mmu,g_antipattern,ullData_rd,ulWalkAddr);
	  }
	}
      }
      /* walk to the next power-of-2 address */
      ulWalkAddr = ulStartAddr + (1<<offset);
    }

    /* now reset this offset with the original g_pattern */
    status = sbFe2000DiagsMemWrite(userDeviceHandle,mmu,ulAddrMover,g_pattern,mem);
    if (status != SB_FE2000_SUCCESS_E) {
      return status;
    }

    /* move the ulAddrMover to the next address */
    ulAddrMover = ulStartAddr + (1<<outer);
  }

  DEXIT();
  return 0;
}

int sbFe2000DiagsSramDataBusWidePortTest(sbxDiagsInfo_t *pDiagsInfo)
{

  volatile unsigned long address_ul = pDiagsInfo->start_addr;
  uint32_t walk_bit = pDiagsInfo->walkbit;
  uint32_t uInstance = pDiagsInfo->uInstance;
  sbhandle userDeviceHandle = pDiagsInfo->userDeviceHandle;
  int status = 0;
  int tst_status=0;
  uint32_t i;
  uint64_t ullData_wr=0;
  volatile uint64_t ullData_rd=0;
  uint64_t ullPattern=1;

  DENTER();

  if (pDiagsInfo->bLoopOnError) {
    SB_LOG("Loop on error not supported for this test.\n");
    return(-1);
  }

  /* sanity check */
  if (sbFe2000DiagsAddressCheck(pDiagsInfo) != 0) {
    return SB_FE2000_BAD_SIZE_E;
  }

  for(i=0;i<pDiagsInfo->wp_config.data_width;i++) {
      if (walk_bit == 1) {
	  ullData_wr = ullPattern;
      } else {
	  ullData_wr = ~ullPattern & 0xfffffffffLL;
      }

      ullPattern<<=1;

      /* write the data */
      status = sbFe2000DiagsMemWrite(userDeviceHandle,uInstance,address_ul,ullData_wr,SB_FE2000_WIDE_PORT);
      if (status != SB_FE2000_SUCCESS_E ) {
	  return status;
      }

      /* read to check the data */
      thin_delay(1000); /* allow the data bus to settle down to avoid false postives */

      status = sbFe2000DiagsMemRead(userDeviceHandle,uInstance,address_ul,&ullData_rd,SB_FE2000_WIDE_PORT);
      if (status != SB_FE2000_SUCCESS_E ) {
	  return status;
      }

      /* check the data */
      if (ullData_rd != ullData_wr) {
	  SB_ERROR("\nERROR:%s: MM%d Data mismatch: write data 0x%llx read data 0x%llx at address: 0x%08lx", __PRETTY_FUNCTION__,uInstance,ullData_wr,ullData_rd,address_ul);
	  tst_status = -1;
      } else {
	  if (pDiagsInfo->debug_level >= 1) {
	      SB_LOG("%s: GOOD MM%d Got: 0x%llx Exp: 0x%llx at address: 0x%08lx\n", __PRETTY_FUNCTION__,uInstance,ullData_wr,ullData_rd,address_ul);
	  }
      }
  }
  DEXIT();
  return tst_status;
}

/* temp for debug.. */
char *memTestD[] = {"Initializing memory","Read/Verify/Write","Read/Verify"};
int sbFe2000DiagsRandomDataMemTest(sbxDiagsInfo_t *pDiagsInfo)

{
  uint32_t mmu = pDiagsInfo->uInstance;
  unsigned long ulStartAddr;
  unsigned long ulEndAddr;
  const int64_t pattern = pDiagsInfo->pattern;
  int32_t seed = pDiagsInfo->seed;
  sbhandle userDeviceHandle = pDiagsInfo->userDeviceHandle;
  int dbgLevel = pDiagsInfo->debug_level;
  int mem = pDiagsInfo->e_mem_type;
  uint32_t uOffset=0;
  volatile uint64_t uActData;
  uint32_t uPhase;         /* there are three phases to this test */
  uint64_t ullExpData = 0x0ULL;
  uint64_t ullExpDataInv; /* inverted expected data */
  uint64_t ullDataMask;

  if (sbFe2000DiagsAddressCheck(pDiagsInfo) != 0) {
    return SB_FE2000_BAD_SIZE_E;
  }

  if (pDiagsInfo->bLoopOnError) {
    SB_LOG("Loop on error not supported for this test.\n");
    return (-1);
  }

  ulStartAddr = pDiagsInfo->start_addr; /* default==0*/
  /* If an ending address was specified, use that otherwise assume maximum */
  if (!pDiagsInfo->bEndAddr) {
      if (pDiagsInfo->e_mem_type == SB_FE2000_NARROW_PORT_0) {
	  ulEndAddr = (1<<pDiagsInfo->np0_config.addr_width);
      } else if (pDiagsInfo->e_mem_type == SB_FE2000_NARROW_PORT_1) {
	  ulEndAddr = (1<<pDiagsInfo->np1_config.addr_width);
      } else {
	  ulEndAddr = (1<<pDiagsInfo->wp_config.addr_width);
      }
  } else {
      ulEndAddr = pDiagsInfo->end_addr;
  }


  DENTER();

  /* be sure memtype to be tested exists */
  SB_ASSERT(mem < SB_FE2000_MEM_TYPE_LAST);
  if (mem == SB_FE2000_WIDE_PORT) {
    ullDataMask = WIDE_PORT_MASK;
  } else {
    ullDataMask = NARROW_PORT_MASK;
  }

  /* if seed is not specified, use a random seed */
  if (seed == -1) {
#ifndef __KERNEL__
    seed = rand();
#else
    
#endif
  }

  pDiagsInfo->seed = seed;

  if (dbgLevel >= 1)
    SB_LOG("\n\n%s:Random Memory read/write test using seed=%d\n",__PRETTY_FUNCTION__,pDiagsInfo->seed);
#ifndef __KERNEL__
      srand(seed);   /* this will seed the random number generator */
#else
   

#endif 
  if (pattern == -1 ) {
#ifndef __KERNEL__
    ullExpData = rand() & ullDataMask;            /* use random data */
#else
    
#endif

    ullExpDataInv = ~ullExpData & ullDataMask;    /* mask off the inverted data appropriately */
  } else {
    ullExpData = pattern & ullDataMask;           /* use the user specified pattern for the test */
    ullExpDataInv = ~ullExpData & ullDataMask;
  }

  if (pDiagsInfo->debug_level >= 1)
    SB_LOG("\n%s,Testing start_addr=0x%08lx, ulEndAddr=0x%08lx, init data=0x%llx \n",__PRETTY_FUNCTION__,
	 ulStartAddr,ulEndAddr,ullExpData);

  /* Iterate from starting address to ending address for each phase. */
  for(uPhase=0;uPhase<3;uPhase++) {
    if (dbgLevel >=1 )
      SB_LOG("%s, PHASE:%d %s ...\n",__PRETTY_FUNCTION__,uPhase,memTestD[uPhase]);
    for(uOffset = ulStartAddr; uOffset < ulEndAddr; uOffset++) {
      if (uPhase == 0) {
	if (dbgLevel >= 2) {
	  SB_LOG("%s:Writing to offset=0x%08x with data:0x%llx\n",__PRETTY_FUNCTION__,uOffset,ullExpData);
	}
	if (sbFe2000DiagsMemWrite(userDeviceHandle, mmu,uOffset, ullExpData,
				  mem) != SB_FE2000_SUCCESS_E) {
	  return SB_FE2000_MEM_ACC_WRITE_TIMEOUT_E;
	}
      } else if (uPhase == 1) {
	if (sbFe2000DiagsMemRead(userDeviceHandle, mmu,uOffset, &uActData,
				 mem) != SB_FE2000_SUCCESS_E) {
	  return SB_FE2000_MEM_ACC_READ_TIMEOUT_E;
	} else {
	  /* compare actual vs expected */
	  if ( uActData == ullExpData) {
	    if (dbgLevel >= 1) {
	      SB_LOG("%s:GOOD MM%d, MemType:%s Act=0x%llx Exp:0x%llx at Addr:0x%08x\n",__PRETTY_FUNCTION__,
		     mmu,sbFe2000DiagsMemoryStr[mem],uActData,ullExpData,uOffset);
	    }
	    /* data ok write back the inverted value to this location */
	    if (sbFe2000DiagsMemWrite(userDeviceHandle, mmu, uOffset,ullExpDataInv,
				      mem) != SB_FE2000_SUCCESS_E) {
	      return SB_FE2000_MEM_ACC_WRITE_TIMEOUT_E;
	    }
	  } else {
	    SB_ERROR("\nERROR:%s:%d, MM%d, MemType:%s Exp:0x%llx, Got:0x%llx, at address:0x%08x\n",__PRETTY_FUNCTION__,__LINE__,
		     mmu,sbFe2000DiagsMemoryStr[mem],ullExpData,uActData,uOffset);
	    return (-1);
	  }
	}
      } else if (uPhase == 2) {
	if (sbFe2000DiagsMemRead(userDeviceHandle, mmu, uOffset, &uActData,
				 mem) != SB_FE2000_SUCCESS_E) {
	  return SB_FE2000_MEM_ACC_READ_TIMEOUT_E;
	} else {
	  /* compare actual vs expected, actual should be data inverted */
	  if ( uActData != ullExpDataInv) {
	    SB_ERROR("ERROR:%s:%d, MM%d, MemType:%s Exp:0x%llx, Got:0x%llx, at address:0x%08x\n",__PRETTY_FUNCTION__,__LINE__,
		     mmu,sbFe2000DiagsMemoryStr[mem],ullExpDataInv,uActData,uOffset);
	    return (-1);
	  } else if (dbgLevel >=1) {
	    SB_LOG("%s:GOOD MM%d, MemType:%s Act=0x%llx Exp:0x%llx at Addr:0x%08x\n",__PRETTY_FUNCTION__,
		   mmu,sbFe2000DiagsMemoryStr[mem],uActData,ullExpDataInv,uOffset);
	  }
	}
      }
    }
  }
  DEXIT();
  return 0;
}

/*
 *  FE2k bists tests start here
 */

int32_t sbFe2000DiagsBistStart(sbxDiagsInfo_t *pDiagsInfo)
{
  uint32_t status=0;
  int32_t camId = pDiagsInfo->cam;
  DENTER();
  switch( camId ) {
  case -1:
    status = sbFe2000DiagsBistPpCamStartAll(pDiagsInfo);
    break;
  case 0:
  case 1:
  case 2:
  case 3:
    status = sbFe2000DiagsBistPpCamStart(pDiagsInfo);
    break;
  default:
    SB_ERROR("ERROR:%s: Invalid cam switch:%d\n",__PRETTY_FUNCTION__,camId);
    status = -1;
  }
  DEXIT();
  return status;
}



/* bist test done like in system_sim/caladan/units/ca/tb/ca_t023.v */
/* this test will test all 4 cams */
int32_t sbFe2000DiagsBistPpCamStartAll(sbxDiagsInfo_t *pDiagsInfo)
{

  sbhandle userDeviceHandle = pDiagsInfo->userDeviceHandle;
  uint32_t uCam=0;
  uint32_t uData=0;
  uint32_t uDone=0;
  uint32_t uGo=0;
  uint32_t uPass=0;
  int32_t nTimeOut;
  int32_t status = 0;

  DENTER();
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, PP_CORE_RESET, 0x1);
  /* toggle tcam_por */
  SAND_HAL_RMW_FIELD(userDeviceHandle,CA,PP_CONFIG,TCAM_POR,0);
  SAND_HAL_RMW_FIELD(userDeviceHandle,CA,PP_CONFIG,TCAM_POR,1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PP_CONFIG, INIT_DONE, 1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, PP_CORE_RESET, 0x0);
  nTimeOut = CAM_MAX_TIMEOUT;
  while(nTimeOut) {
      uData = SAND_HAL_READ(userDeviceHandle,CA,PP_CONFIG);
      uDone = SAND_HAL_GET_FIELD(CA,PP_CONFIG,INIT_DONE,uData);
      if (uDone == 1)
	  break;
      thin_delay(500);
      nTimeOut--;
  }

  if (uDone != 1) {
      SB_ERROR("ERROR:%s:Timedout waiting for init_done!\n",__PRETTY_FUNCTION__);
      status = -1;
  }

  /* turn off single mode */
  uData = SAND_HAL_READ(userDeviceHandle,CA,PP_CAMBIST_TCAM_DEBUG0);
  uData = SAND_HAL_MOD_FIELD(CA,PP_CAMBIST_TCAM_DEBUG0,BIST_SINGLE_COMP,uData,0);
  SAND_HAL_WRITE(userDeviceHandle,CA,PP_CAMBIST_TCAM_DEBUG0,uData);
  thin_delay(100);

  for(uCam=0;uCam<4;uCam++) {

    uData = SAND_HAL_READ(userDeviceHandle,CA,PP_CAMBIST_CONFIG);
    /* clear */
    uData = SAND_HAL_MOD_FIELD(CA,PP_CAMBIST_CONFIG,BIST_GO,uData,0);
    SAND_HAL_WRITE(userDeviceHandle,CA,PP_CAMBIST_CONFIG,uData);
    /* make sure 'done','pass','go' are deasserted, and then assert 'go' */
    uData = SAND_HAL_READ(userDeviceHandle,CA,PP_CAMBIST_CONFIG);
    uDone = SAND_HAL_GET_FIELD(CA,PP_CAMBIST_CONFIG,BIST_DONE,uData);
    if (uDone != 0 ) {
      SB_ERROR("ERROR:%s:before starting bist_done should be clear!\n",__PRETTY_FUNCTION__);
      status = -1;
    }

    uPass = SAND_HAL_GET_FIELD(CA,PP_CAMBIST_CONFIG,BIST_PASS,uData);
    if (uPass != 0 ) {
      SB_ERROR("ERROR:%s:before starting bist_pass should be clear!\n",__PRETTY_FUNCTION__);
      status = -1;
    }

    uGo = SAND_HAL_GET_FIELD(CA,PP_CAMBIST_CONFIG,BIST_GO,uData);
    if ( uGo != 0) {
      SB_ERROR("ERROR:%s:before starting bist_go should be clear!\n",__PRETTY_FUNCTION__);
      status = -1;
    }

    /* test each cam */
    uData = SAND_HAL_MOD_FIELD(CA,PP_CAMBIST_CONFIG,BIST_GO,uData,1<<uCam);
    SAND_HAL_WRITE(userDeviceHandle,CA,PP_CAMBIST_CONFIG,uData);
    if (pDiagsInfo->debug_level >= 1)
      SB_LOG("\n%s:Running BIST PP CAM%d test ..\n",__PRETTY_FUNCTION__,uCam);
    /* wait for done */
    nTimeOut = CAM_MAX_TIMEOUT;
    while(nTimeOut) {
      uData = SAND_HAL_READ(userDeviceHandle,CA,PP_CAMBIST_CONFIG);
      uDone = SAND_HAL_GET_FIELD(CA,PP_CAMBIST_CONFIG,BIST_DONE,uData);
      if (uDone == (1<<uCam))
	break;
      thin_delay(500);
      nTimeOut--;
    }

    if (uDone != (1<<uCam)) {
      SB_ERROR("ERROR:%s:bist test timed out testing cam%d, never asserted done bit\n",__PRETTY_FUNCTION__,uCam);
      status = -1;
    } else {
      /* test finished, check status */
      uData = SAND_HAL_READ(userDeviceHandle,CA,PP_CAMBIST_CONFIG);
      uPass = SAND_HAL_GET_FIELD(CA,PP_CAMBIST_CONFIG,BIST_PASS,uData);
      if (uPass != 1<<uCam) {
	SB_ERROR("ERROR:%s bist test for cam%d failed.\n",__PRETTY_FUNCTION__,uCam);
	status = -1;
      } else {
	if (pDiagsInfo->debug_level >= 1)
	  SB_LOG("%s:BIST PP CAM%d test passed.\n",__PRETTY_FUNCTION__,uCam);
      }
    }

    /* cleanup */
    uData = SAND_HAL_READ(userDeviceHandle,CA,PP_CAMBIST_CONFIG);
    uData = SAND_HAL_MOD_FIELD(CA,PP_CAMBIST_CONFIG,BIST_GO,uData,0);
    SAND_HAL_WRITE(userDeviceHandle,CA,PP_CAMBIST_CONFIG,uData);

  }
  DEXIT();
  return status;
}

/* specific debug test for a specific Cam with specific pattern */
int32_t sbFe2000DiagsBistPpCamStart(sbxDiagsInfo_t *pDiagsInfo)
{
  sbhandle userDeviceHandle = pDiagsInfo->userDeviceHandle;
  uint32_t uData=0;
  uint32_t uDone=0;
  uint32_t uGo=0;
  uint32_t uPass=0;
  uint32_t nMaskSel;
  uint32_t nDwordSel;
  int32_t nTimeOut;
  int status=0;
  int dbgLevel = pDiagsInfo->debug_level;
  uint32_t uCam = pDiagsInfo->cam;
  uint32_t bistpattern;
  if (pDiagsInfo->pattern == -1) {
#ifndef __KERNEL__
      pDiagsInfo->pattern = rand() & 0xffffffff;
#else
      
#endif
  }
  bistpattern = pDiagsInfo->pattern;
  DENTER();
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, PP_CORE_RESET, 0x1);
  /* toggle tcam_por */
  SAND_HAL_RMW_FIELD(userDeviceHandle,CA,PP_CONFIG,TCAM_POR,0);
  SAND_HAL_RMW_FIELD(userDeviceHandle,CA,PP_CONFIG,TCAM_POR,1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PP_CONFIG, INIT_DONE, 1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, PP_CORE_RESET, 0x0);

  /* turn on single mode */
  uData = SAND_HAL_READ(userDeviceHandle,CA,PP_CAMBIST_TCAM_DEBUG0);
  uData = SAND_HAL_MOD_FIELD(CA,PP_CAMBIST_TCAM_DEBUG0,BIST_SINGLE_COMP,uData,1);
  SAND_HAL_WRITE(userDeviceHandle,CA,PP_CAMBIST_TCAM_DEBUG0,uData);
  thin_delay(100);

  /* data setup */
  uData = SAND_HAL_READ(userDeviceHandle,CA,PP_CAMBIST_TCAM_DEBUG1);
  uData = SAND_HAL_MOD_FIELD(CA,PP_CAMBIST_TCAM_DEBUG1,BIST_DWORD,uData,bistpattern);
  SAND_HAL_WRITE(userDeviceHandle,CA,PP_CAMBIST_TCAM_DEBUG1,uData);

  uData = SAND_HAL_READ(userDeviceHandle,CA,PP_CAMBIST_TCAM_DEBUG2);
  uData = SAND_HAL_MOD_FIELD(CA,PP_CAMBIST_TCAM_DEBUG2,BIST_DWORD,uData,bistpattern<<1);
  SAND_HAL_WRITE(userDeviceHandle,CA,PP_CAMBIST_TCAM_DEBUG2,uData);

  uData = SAND_HAL_READ(userDeviceHandle,CA,PP_CAMBIST_TCAM_DEBUG3);
  uData = SAND_HAL_MOD_FIELD(CA,PP_CAMBIST_TCAM_DEBUG3,BIST_DWORD,uData,bistpattern<<2);
  SAND_HAL_WRITE(userDeviceHandle,CA,PP_CAMBIST_TCAM_DEBUG3,uData);

  uData = SAND_HAL_READ(userDeviceHandle,CA,PP_CAMBIST_TCAM_DEBUG4);
  uData = SAND_HAL_MOD_FIELD(CA,PP_CAMBIST_TCAM_DEBUG4,BIST_DWORD,uData,bistpattern>>1);
  SAND_HAL_WRITE(userDeviceHandle,CA,PP_CAMBIST_TCAM_DEBUG4,uData);

  if (dbgLevel >= 1)
    SB_LOG("\n%s:Running BIST PP CAM%d test with pattern 0x%08x\n",__PRETTY_FUNCTION__,uCam,bistpattern);

  /* select dwordsel */
  for(nMaskSel=0;nMaskSel<=1;nMaskSel++) {
    for(nDwordSel=0;nDwordSel<6;nDwordSel++) { /* 6 32-bit words in 186bit entry */
      if (dbgLevel >=1 )
	SB_LOG("%s Testing with nMaskSel=%d, nDwordSel=%d\n",__PRETTY_FUNCTION__,nMaskSel,nDwordSel);
      /* de-assert go */
      uData = SAND_HAL_READ(userDeviceHandle,CA,PP_CAMBIST_CONFIG);
      uData = SAND_HAL_MOD_FIELD(CA,PP_CAMBIST_CONFIG,BIST_GO,uData,0);
      SAND_HAL_WRITE(userDeviceHandle,CA,PP_CAMBIST_CONFIG,uData);

      /* select DwordSel */
      uData = SAND_HAL_READ(userDeviceHandle,CA,PP_CAMBIST_TCAM_DEBUG0);
      uData = SAND_HAL_MOD_FIELD(CA,PP_CAMBIST_TCAM_DEBUG0,BIST_DWORD_MASK_SEL,uData,nMaskSel<<uCam);

      switch (uCam) {
      case 0:
	uData = SAND_HAL_MOD_FIELD(CA,PP_CAMBIST_TCAM_DEBUG0,BIST_DWORD_SEL_0,uData,nDwordSel);
	break;
      case 1:
	uData = SAND_HAL_MOD_FIELD(CA,PP_CAMBIST_TCAM_DEBUG0,BIST_DWORD_SEL_1,uData,nDwordSel);
	break;
      case 2:
	uData = SAND_HAL_MOD_FIELD(CA,PP_CAMBIST_TCAM_DEBUG0,BIST_DWORD_SEL_2,uData,nDwordSel);
	break;
      case 3:
	uData = SAND_HAL_MOD_FIELD(CA,PP_CAMBIST_TCAM_DEBUG0,BIST_DWORD_SEL_3,uData,nDwordSel);
	break;
      default:
	SB_ERROR("ERROR:%s Invalid cam switch:%d \n",__PRETTY_FUNCTION__,uCam);
	SB_ASSERT( 0 );
      }

      SAND_HAL_WRITE(userDeviceHandle,CA,PP_CAMBIST_TCAM_DEBUG0,uData);

      /* make sure 'done','pass','go' are deasserted, and cam selection was written, and then assert 'go' */
      uData = SAND_HAL_READ(userDeviceHandle,CA,PP_CAMBIST_CONFIG);

      uDone = SAND_HAL_GET_FIELD(CA,PP_CAMBIST_CONFIG,BIST_DONE,uData);
      if (uDone != 0 ) {
	SB_ERROR("ERROR:%s:before starting bist_done should be clear!\n",__PRETTY_FUNCTION__);
	status = -1;
      }

      uPass = SAND_HAL_GET_FIELD(CA,PP_CAMBIST_CONFIG,BIST_PASS,uData);
      if (uPass != 0 ) {
	SB_ERROR("ERROR:%s:before starting bist_pass should be clear!\n",__PRETTY_FUNCTION__);
	status = -1;
      }

      uGo = SAND_HAL_GET_FIELD(CA,PP_CAMBIST_CONFIG,BIST_GO,uData);
      if ( uGo != 0) {
	SB_ERROR("ERROR:%s:before starting bist_go should be clear!\n",__PRETTY_FUNCTION__);
	status = -1;
      }

      /* assert go to start test */
      uData = SAND_HAL_MOD_FIELD(CA,PP_CAMBIST_CONFIG,BIST_GO,uData,1<<uCam);
      SAND_HAL_WRITE(userDeviceHandle,CA,PP_CAMBIST_CONFIG,uData);

      /* wait for done */
      nTimeOut = CAM_MAX_TIMEOUT;
      while(nTimeOut!=0) {
	uData = SAND_HAL_READ(userDeviceHandle,CA,PP_CAMBIST_CONFIG);
	uDone = SAND_HAL_GET_FIELD(CA,PP_CAMBIST_CONFIG,BIST_DONE,uData);
	if (uDone == (1<<uCam))
	  break;
	thin_delay(500);
	nTimeOut--;
      }

      if (uDone != (1<<uCam)) {
	SB_ERROR("ERROR:%s:bist test timed out nMaskSel:%d, nDwordSel:%d, never asserted done bit\n",__PRETTY_FUNCTION__,nMaskSel,nDwordSel);
	status = -1;
      } else {
	/* test finished, check status */
	uData = SAND_HAL_READ(userDeviceHandle,CA,PP_CAMBIST_CONFIG);
	uPass = SAND_HAL_GET_FIELD(CA,PP_CAMBIST_CONFIG,BIST_PASS,uData);
	if (uPass != 1<<uCam) {
	  SB_ERROR("ERROR:\n%s bist test failed for cam%d with dwordsel:%d masksel:%d\n",__PRETTY_FUNCTION__,uCam,nDwordSel,nMaskSel);
	  status = -1;
	} else {
	  if (dbgLevel >=1 )
	    SB_LOG("%s:BIST PP CAM%d test passed.\n",__PRETTY_FUNCTION__,uCam);
	}
      }
    } /* nDwordSel */
  } /* nMaskSel */
  DEXIT();
  return status;
}

int sbFe2000DiagsMemRead(sbhandle userDeviceHandle,
			 uint32_t uInstance,
			 unsigned long addr,
			 volatile uint64_t *pData,
			 sbFe2000DiagsMemType_t e_mem_type) {

  DENTER();
  switch(e_mem_type) {
  case SB_FE2000_NARROW_PORT_0:

    {

      uint32_t offset,ack_mask;
      volatile uint64_t tmp=0;
      /* initiate the read request */
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_CTRL,RD_WR_N,1); /* 1 indicates a read */
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_CTRL,ADDRESS,addr);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_CTRL,ACK,1);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_CTRL,REQ,1);
      /* wait for the read op to complete */
      offset = SAND_HAL_REG_OFFSET_STRIDE(CA,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_CTRL);
      ack_mask = ( uInstance == 1 ) ? SAND_HAL_CA_MM1_NARROW_PORT0_MEM_ACC_CTRL_ACK_MASK : SAND_HAL_CA_MM0_NARROW_PORT0_MEM_ACC_CTRL_ACK_MASK;
      if (sbFe2000DiagsWrRdComplete(userDeviceHandle,
				    offset,
				    ack_mask,
				    SB_FE2000_MEM_TIMEOUT) != TRUE) {
	SB_ERROR("\nERROR:%s: Read timeout, reading from MM%d narrow port0 address:0x%08lx\n",__PRETTY_FUNCTION__,uInstance,addr);
	return (SB_FE2000_MEM_ACC_READ_TIMEOUT_E);
      }
      /* get the first 32bits of 36 bits data */
      *pData = SAND_HAL_READ_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_DATA0);
      /* get the remaining 4 bits of 36 bits of data */
      tmp = SAND_HAL_READ_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_DATA1) & 0xf;
      *pData |= tmp<<32;
    }
    break;

  case SB_FE2000_NARROW_PORT_1:

    {
      uint32_t offset,ack_mask;
      volatile uint64_t tmp=0;
      /* initiate the read request */
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_CTRL,RD_WR_N,1);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_CTRL,ADDRESS,addr);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_CTRL,ACK,1);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_CTRL,REQ,1);
      /* wait for the read op to complete */
      offset = SAND_HAL_REG_OFFSET_STRIDE(CA,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_CTRL);
      ack_mask = ( uInstance == 1 ) ? SAND_HAL_CA_MM1_NARROW_PORT1_MEM_ACC_CTRL_ACK_MASK : SAND_HAL_CA_MM0_NARROW_PORT1_MEM_ACC_CTRL_ACK_MASK;
      if (sbFe2000DiagsWrRdComplete(userDeviceHandle,
				    offset,
				    ack_mask,
				    SB_FE2000_MEM_TIMEOUT) != TRUE) {
	SB_ERROR("\nERROR:%s: Read timeout, reading from MM%d narrow port1 address:0x%08lx\n",__PRETTY_FUNCTION__,uInstance,addr);
	return (SB_FE2000_MEM_ACC_READ_TIMEOUT_E);
      }

      /* get the first 32bits of 36 bits data */
      *pData = SAND_HAL_READ_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_DATA0);
      tmp = SAND_HAL_READ_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_DATA1) & 0xf;
      /* get the remaining 4 bits of 36 bits of data */
      *pData |= tmp<<32;
    }
    break;

  case SB_FE2000_WIDE_PORT:

    {
      uint32_t offset,ack_mask;
      volatile uint64_t tmp=0;
      /* initiate the read request */
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_WIDE_PORT_MEM_ACC_CTRL,RD_WR_N,1);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_WIDE_PORT_MEM_ACC_CTRL,ADDRESS,addr);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_WIDE_PORT_MEM_ACC_CTRL,ACK,1);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_WIDE_PORT_MEM_ACC_CTRL,REQ,1);

      /* wait for the read op to complete */
      offset = SAND_HAL_REG_OFFSET_STRIDE(CA,MM,uInstance,MM_WIDE_PORT_MEM_ACC_CTRL);
      ack_mask = ( uInstance == 1 ) ? SAND_HAL_CA_MM1_WIDE_PORT_MEM_ACC_CTRL_ACK_MASK : SAND_HAL_CA_MM0_WIDE_PORT_MEM_ACC_CTRL_ACK_MASK;
      if (sbFe2000DiagsWrRdComplete(userDeviceHandle,
				    offset,
				    ack_mask,
				    SB_FE2000_MEM_TIMEOUT) != TRUE) {
	SB_ERROR("\nERROR:%s: Read timeout, reading from MM%d wide port address:0x%08lx\n",__PRETTY_FUNCTION__,uInstance,addr);
	return (SB_FE2000_MEM_ACC_READ_TIMEOUT_E);
      }

      /* get the first 32bits of 64 bits of data */
      *pData = SAND_HAL_READ_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_WIDE_PORT_MEM_ACC_DATA0);
      tmp = SAND_HAL_READ_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_WIDE_PORT_MEM_ACC_DATA1);
      /* get the remaining 32 bits of 64 bits of data */
      *pData |= (tmp << 32);
    }
    break;

  default:
    SB_ERROR("ERROR:%s: Bad read request to memory type:%d\n",__PRETTY_FUNCTION__,e_mem_type);
    SB_ASSERT( 0 );
  }
  DEXIT();
  return SB_FE2000_SUCCESS_E;

}

int sbFe2000DiagsMemWrite(sbhandle userDeviceHandle,
			  uint32_t uInstance,
			  unsigned long addr,
			  const uint64_t ullData ,
			  sbFe2000DiagsMemType_t e_mem_type) {

  DENTER();
  switch(e_mem_type) {
  case SB_FE2000_NARROW_PORT_0:

    {
      uint32_t offset,ack_mask;
      SAND_HAL_WRITE_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_DATA0,ullData&MASK_LSW);    /* Least significant 32 bits of 36 bit data to be written */
      SAND_HAL_WRITE_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_DATA1,(ullData>>32)&0xf);   /* Most significant 4 bits of 36 bit data to be written */
      /* initiate the write request */
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_CTRL,RD_WR_N,0);        /* 0 indicates a write */
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_CTRL,ADDRESS,addr);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_CTRL,ACK,1);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_CTRL,REQ,1);
      /* wait for the write op to complete */
      offset = SAND_HAL_REG_OFFSET_STRIDE(CA,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_CTRL);
      ack_mask = ( uInstance == 1 ) ? SAND_HAL_CA_MM1_NARROW_PORT0_MEM_ACC_CTRL_ACK_MASK : SAND_HAL_CA_MM0_NARROW_PORT0_MEM_ACC_CTRL_ACK_MASK;
      if (sbFe2000DiagsWrRdComplete(userDeviceHandle,
				    offset,
				    ack_mask,
				    SB_FE2000_MEM_TIMEOUT) != TRUE) {
	SB_ERROR("\nERROR:%s: Write timeout, writing data=0x%llx, to MM%d narrow port0 memory address:0x%08lx\n",__PRETTY_FUNCTION__,ullData,uInstance,addr);
	return (SB_FE2000_MEM_ACC_WRITE_TIMEOUT_E);
      }
    }
    break;
  case SB_FE2000_NARROW_PORT_1:

    {
      uint32_t offset,ack_mask;
      SAND_HAL_WRITE_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_DATA0,ullData&MASK_LSW);
      SAND_HAL_WRITE_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_DATA1,(ullData>>32)&0xf);
      /* initiate the write request */
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_CTRL,RD_WR_N,0);        /* 0 indicates a write */
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_CTRL,ADDRESS,addr);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_CTRL,ACK,1);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_CTRL,REQ,1);
      /* wait for the write op to complete */
      offset = SAND_HAL_REG_OFFSET_STRIDE(CA,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_CTRL);
      ack_mask = ( uInstance == 1 ) ? SAND_HAL_CA_MM1_NARROW_PORT1_MEM_ACC_CTRL_ACK_MASK : SAND_HAL_CA_MM0_NARROW_PORT1_MEM_ACC_CTRL_ACK_MASK;
      if (sbFe2000DiagsWrRdComplete(userDeviceHandle,
				    offset,
				    ack_mask,
				    SB_FE2000_MEM_TIMEOUT) != TRUE) {
	SB_ERROR("\nERROR:%s: Write timeout, writing data=0x%llx, to MM%d narrow port1 memory address:0x%08lx\n",__PRETTY_FUNCTION__,ullData,uInstance,addr);
	return (SB_FE2000_MEM_ACC_WRITE_TIMEOUT_E);
      }
    }
    break;
  case SB_FE2000_WIDE_PORT:
    {
      uint32_t offset,ack_mask;
      SAND_HAL_WRITE_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_WIDE_PORT_MEM_ACC_DATA0,ullData&MASK_LSW);    /* least significant 32 bits of 64 bit data */
      SAND_HAL_WRITE_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_WIDE_PORT_MEM_ACC_DATA1,(ullData>>32));       /* most significant 32 bits of 64 bit data */
      /* initiate the write request */
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_WIDE_PORT_MEM_ACC_CTRL,RD_WR_N,0);        /* 0 indicates a write */
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_WIDE_PORT_MEM_ACC_CTRL,ADDRESS,addr);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_WIDE_PORT_MEM_ACC_CTRL,ACK,1);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uInstance,MM_WIDE_PORT_MEM_ACC_CTRL,REQ,1);
      /* wait for the write op to complete */
      offset = SAND_HAL_REG_OFFSET_STRIDE(CA,MM,uInstance,MM_WIDE_PORT_MEM_ACC_CTRL);
      ack_mask = ( uInstance == 1 ) ? SAND_HAL_CA_MM1_WIDE_PORT_MEM_ACC_CTRL_ACK_MASK : SAND_HAL_CA_MM0_WIDE_PORT_MEM_ACC_CTRL_ACK_MASK;
      if (sbFe2000DiagsWrRdComplete(userDeviceHandle,
				    offset,
				    ack_mask,
				    SB_FE2000_MEM_TIMEOUT) != TRUE) {
	SB_ERROR("\nERROR:%s: Write timeout, writing data=0x%llx, to MM%d wide port memory address:0x%08lx\n",__PRETTY_FUNCTION__,ullData,uInstance,addr);
	return (SB_FE2000_MEM_ACC_WRITE_TIMEOUT_E);
      }
    }
    break;
  default:
    SB_ERROR("ERROR:%s: Bad write request to memory type:%d\n",__PRETTY_FUNCTION__,e_mem_type);
    SB_ASSERT( 0 );
  }
  DEXIT();
  return SB_FE2000_SUCCESS_E;
}

/* Intialize memory with a default pattern, for diagnostics testing */
int sbFe2000DiagsInitMem(sbxDiagsInfo_t *pDiagsInfo) {

  unsigned long addr;
  int stat;
  int mmu = pDiagsInfo->uInstance;
  sbhandle userDeviceHandle = pDiagsInfo->userDeviceHandle;

  DENTER();
  switch (pDiagsInfo->e_mem_type)
    {

    case SB_FE2000_NARROW_PORT_0:
      if (pDiagsInfo->debug_level) {
	SB_LOG("%s: Initializing MM%d Narrow Port0 Memory..\n",__PRETTY_FUNCTION__,mmu);
      }
    for(addr=0;addr < 1<<pDiagsInfo->np0_config.addr_width;addr++) {
      stat = sbFe2000DiagsMemWrite(userDeviceHandle,mmu,addr,g_NarrowPortpattern,SB_FE2000_NARROW_PORT_0);
      if (stat != SB_FE2000_SUCCESS_E) {
	return stat;
      }
    }
    break;

    case SB_FE2000_NARROW_PORT_1:
      if (pDiagsInfo->debug_level) {
	SB_LOG("%s: Initializing MM%d Narrow Port1 Memory..\n",__PRETTY_FUNCTION__,mmu);
      }
      for(addr=0;addr < 1<<pDiagsInfo->np1_config.addr_width;addr++) {
	stat = sbFe2000DiagsMemWrite(userDeviceHandle,mmu,addr,g_NarrowPortpattern,SB_FE2000_NARROW_PORT_1);
	if (stat != SB_FE2000_SUCCESS_E) {
	  return stat;
	}
      }
      break;

    case SB_FE2000_WIDE_PORT:
      for(mmu=0;mmu<SB_FE2000_NUM_MM_INSTANCES;mmu++) {
	if (mmu!=pDiagsInfo->uInstance) continue;
	if (pDiagsInfo->debug_level) {
	  SB_LOG("%s: Initializing MM%d Wide Port Memory..\n",__PRETTY_FUNCTION__,mmu);
	}
	for(addr=0;addr < 1<<pDiagsInfo->wp_config.addr_width;addr++) {
	  stat = sbFe2000DiagsMemWrite(userDeviceHandle,mmu,addr,g_WidePortpattern,SB_FE2000_WIDE_PORT);
	  if (stat != SB_FE2000_SUCCESS_E) {
	    return stat;
	  }
	}
      }
      break;
    case SB_FE2000_MEM_TYPE_LAST:
      SB_LOG("%s: Invalid mem type %d\n",__PRETTY_FUNCTION__,pDiagsInfo->e_mem_type);
      return(-1);
      break;
    }

  DEXIT();
  return SB_FE2000_SUCCESS_E;

}


int sbFe2000DiagsWrRdComplete(sbhandle userDeviceHandle,
			      uint32_t uCtlReg,
			      uint32_t uAck,
			      uint32_t uTimeOut)
{

  uint32_t uReg;
  uint32_t i;

  /* Wait for the ACK to indicate rd/wr op is finished */
  for(i=0;i < uTimeOut*10 ; i++ ) {
    uReg = SAND_HAL_READ_OFFS(userDeviceHandle,uCtlReg);
    if (uReg & uAck) {
      return ( TRUE ); /* done, GOOD! */
    }
    thin_delay(SB_FE2000_10_USEC_K*10);
  }

  /* timed out op did not complete in time */
  return ( FALSE );

}

/*
 *   FE2K LOOPBACK TESTS
 *
 */

int sbFe2000LoopbackTest(sbxDiagsInfo_t *pDiagsInfo)

{

    int stat=0;
    uint32_t lbk = pDiagsInfo->e_loopback_type;
    uint32_t unit;
    int i,j;
    DENTER();
    unit = pDiagsInfo->unit;

    for (i=0;i<SB_FE2000_NUM_AG_INTERFACES;i++) {
      for(j=0;j<SB_FE2000_MAX_AG_PORTS;j++) {
	g_AGTxPorts[i][j] = 0;
	g_AGRxPorts[i][j] = 0;
      }
    }

    sbx_clear_all(unit,0);
    (void)soc_sbx_counter_stop(unit);

    switch(lbk) {
    case SB_FE2000_1G_LOOPBACK:
    FE2000_STS_CHECK
      (sbFe2000LoopbackSet(unit,BCM_PORT_LOOPBACK_NONE));
	stat = sbFe2000DiagsSerdesExternalLoopback(pDiagsInfo);
	break;
    case SB_FE2000_1G_UNIMAC_LOOPBACK:
    FE2000_STS_CHECK
      (sbFe2000LoopbackSet(unit,BCM_PORT_LOOPBACK_MAC));
	stat = sbFe2000DiagsUnimacLoopback(pDiagsInfo);
	break;
    case SB_FE2000_1G_PHY_LOOPBACK:
    FE2000_STS_CHECK
      (sbFe2000LoopbackSet(unit,BCM_PORT_LOOPBACK_PHY));
	stat = sbFe2000DiagsSerdesInternalLoopback(pDiagsInfo);
	break;
    case SB_FE2000_10G_LOOPBACK:
	stat = sbFe2000Diags10GLoopback(pDiagsInfo);
	break;
    case SB_FE2000_10G_QE_LOOPBACK:
	stat = sbFe2000Diags10GQELoopback(pDiagsInfo);
	break;
    default:
	SB_ERROR("ERROR:%s Invalid loopback test %d\n",__PRETTY_FUNCTION__,lbk);
	return (-1);
    }

    /* do not leave any ports in any loopback state */
    if (lbk == SB_FE2000_1G_UNIMAC_LOOPBACK ||
	lbk == SB_FE2000_1G_PHY_LOOPBACK) {
      FE2000_STS_CHECK
	(sbFe2000LoopbackSet(unit,BCM_PORT_LOOPBACK_NONE));
    } else if (lbk == SB_FE2000_10G_LOOPBACK) {
	FE2000_STS_CHECK
	    (sbFe200010GLoopbackSet(unit,24,BCM_PORT_LOOPBACK_NONE));
	FE2000_STS_CHECK
	    (sbFe200010GLoopbackSet(unit,25,BCM_PORT_LOOPBACK_NONE));
    }

    return stat;
}

void sbFe2000ClearQueues(sbhandle hdl)
{

  DENTER();

  CaQmQueueConfigClear(hdl);
/*   CaPtPortToQueueAg0Clear(hdl); */
/*   CaPtPortToQueueAg1Clear(hdl); */
  CaQmQueueState0Clear(hdl);
  CaQmQueueState1Clear(hdl);

  DEXIT();

}

int
sbFe2000LoopbackSet(uint32_t unit,uint32_t loopback)
{
  int rv = 0;
  uint32_t port;
  for(port=0;port<24;port++) {
    rv = bcm_port_loopback_set(unit,port,loopback);
    if (rv) {
      SB_LOG("ERROR: bcm_port_loopback_set failed(%d)(%s) for port %d\n",rv,bcm_errmsg(rv),port);
      return (-1);
    }
  }
  return SB_OK;
}


/*
 *   Setup Xaui Internal loopback
 *   (comments from verification Test 10.101)
 *   Inject packets into PCI, have the packet loopback external to the xaui
 *   , and exit PCI.  Check packet counter to ascertain that the packet
 *   managed to traverse the sws->bigmac->xaui(external)->bigmac->sws.
 *
 *   Only Xgm1 has and external phy to be able to set into internal loopback.
 *   The other Xgm0 interface goes straight out to a CX4 connector.
 */

int sbFe2000DiagsXauiInternalLoopback(sbxDiagsInfo_t *pDiagsInfo)

{
  uint32_t uData;
  uint32_t uQueue=0;
  uint32_t stat;
  uint32_t uXgmNum;
  uint32_t uPktCnt;
  uint32_t uLinkScanOn;
  uint32_t phy_addr;
  sbhandle userDeviceHandle;
  int dbg;
  userDeviceHandle = pDiagsInfo->userDeviceHandle;
  uPktCnt = pDiagsInfo->nInjectPackets;
  dbg = pDiagsInfo->debug_level;

  DENTER();

  SB_LOG("Running 10G XFP - PHY Loopback Test\n");

  /* get the phy addr for xe1 port (25)*/
  phy_addr = PORT_TO_PHY_ADDR(1,25); /* (u,p)  */
  if (dbg) {
      SB_LOG("Using xe1 phy addr:0x%x\n",phy_addr);
  }

  /* mask link status change since we are using loopback */
  uData = SAND_HAL_READ( userDeviceHandle, CA, PC_INTERRUPT_MASK );
  uData = SAND_HAL_SET_FIELD(CA, PC_INTERRUPT_MASK, LINK_STATUS_CHANGE_DISINT, 1);
  SAND_HAL_WRITE( userDeviceHandle, CA, PC_INTERRUPT_MASK, uData );

  /* disable link scan if it is on */
  uData = SAND_HAL_READ(userDeviceHandle, CA, PC_MIIM_CONTROL0);
  uLinkScanOn = SAND_HAL_GET_FIELD(CA,PC_MIIM_CONTROL0,MIIM_LINK_SCAN_EN,uData);
  if(uLinkScanOn) { /* turn it off */
      SB_LOG("%s Disable link scan to start XGM mdio access\n",__PRETTY_FUNCTION__);
      uData = SAND_HAL_MOD_FIELD(CA, PC_MIIM_CONTROL0, MIIM_LINK_SCAN_EN, uData, 0x0);
      SAND_HAL_WRITE(userDeviceHandle, CA, PC_MIIM_CONTROL0, uData);
  }

  stat = sbFe2000UtilXgmMiimRead( userDeviceHandle,0x0/*bInternalPhy*/,0x1/*bClause45*/,0x1/*uDevAddr*/,
				  phy_addr/*uPhyOrPortAddr=0x18/0x19*/,0/*uRegAddr*/,&uData/* &uData*/);
  if (stat != SB_FE2000_SUCCESS_E) {
    SB_ERROR("ERROR:%s sbFe2000UtilXgmMiimRead failed with status %d\n",__PRETTY_FUNCTION__,stat);
    return(-1);
  }
  if (dbg) {
      SB_LOG("%s XgmMiimRead returned uData=0x%08x\n",__PRETTY_FUNCTION__,uData);
  }

  stat = sbFe2000UtilXgmMiimWrite(userDeviceHandle,0x0/*bInternalPhy*/,0x1/*bClause45*/,0x1/*uDevAddr*/,
				  phy_addr/*uPhyOrPortAddr*/,0/*uRegAddr*/,uData|0x0001/*uData*/);
  if (stat != SB_FE2000_SUCCESS_E) {
    SB_ERROR("ERROR:%s sbFe2000UtilXgmMiimWrite failed with status %d\n",__PRETTY_FUNCTION__,stat);
    return(-1);
  }

  stat = sbFe2000UtilXgmMiimRead( userDeviceHandle,FALSE/*bInternalPhy*/,TRUE/*bClause45*/,0x1/*uDevAddr*/,
				  phy_addr/*uPhyOrPortAddr=0x18/0x19*/,0/*uRegAddr*/,&uData);
  if (stat != SB_FE2000_SUCCESS_E) {
    SB_ERROR("ERROR:%s sbFe2000UtilXgmMiimRead failed with status %d\n",__PRETTY_FUNCTION__,stat);
    return(-1);
  }
  if (dbg) {
      SB_LOG("%s sbFe2000UtilXgmMiimRead returned uData=0x%08x\n",__PRETTY_FUNCTION__,uData);
  }

  stat = sbFe2000UtilXgmMiimWrite(userDeviceHandle,FALSE/*bInternalPhy*/,TRUE/*bClause45*/,0x1/*uDevAddr*/,
				  phy_addr/*uPhyOrPortAddr*/,0/*uRegAddr*/,uData|0x0001/*uData*/);
  if (stat != SB_FE2000_SUCCESS_E) {
    SB_ERROR("ERROR:%s sbFe2000UtilXgmMiimWrite failed with status %d\n",__PRETTY_FUNCTION__,stat);
    return(-1);
  }

  if (uLinkScanOn) { /* turn it back on */
      SB_LOG("%s Re-enable Link Scan\n",__PRETTY_FUNCTION__);
      uData = SAND_HAL_READ(userDeviceHandle, CA, PC_MIIM_CONTROL0);
      uData = SAND_HAL_MOD_FIELD(CA, PC_MIIM_CONTROL0, MIIM_LINK_SCAN_EN, uData, 0x1);
      SAND_HAL_WRITE(userDeviceHandle, CA, PC_MIIM_CONTROL0, uData);
  }

  for(uXgmNum=1;uXgmNum<SB_FE2000_NUM_XG_INTERFACES;uXgmNum++) {
    AddQueue(userDeviceHandle,6/*uInPre*/,0/*uInPort*/,4+uXgmNum/*uOutPte*/,0/*uOutPort*/,uQueue);
    AddQueue(userDeviceHandle,4+uXgmNum/*uInPre*/,0/*uInPort*/,6/*uOutPte*/,0/*uOutPort*/,1);
  }

  stat = sbFe2000DiagsInjectPciPacket(pDiagsInfo);
  if (stat != SB_FE2000_SUCCESS_E) {
    SB_ERROR("ERROR:%s sbFe2000DiagsInjectPciPacket failed with status %d\n",__PRETTY_FUNCTION__,stat);
    return(-1);
  }

  uData = SAND_HAL_READ(userDeviceHandle,CA,PC_TX_PKT_CNT);
  if (uData != uPktCnt) {
    SB_ERROR("ERROR:%s, PCI TxExp:%d TxAct:%d\n",__PRETTY_FUNCTION__,uPktCnt,uData);
    stat = -1;
  } else if (dbg) {
    SB_LOG("GOOD PCI TxExp:%d TxAct:%d\n",uPktCnt,uData);
  }

  /* check packets where accepted by the QM block */
  uData = SAND_HAL_READ(userDeviceHandle,CA,PR_PRE6_PKT_CNT);
  if (uData != uPktCnt) {
    SB_ERROR("ERROR:%s, SWS.PRE6 Exp:%d Act:%d\n",__PRETTY_FUNCTION__,uPktCnt,uData);
    stat = -1;
  } else if (dbg) {
      SB_LOG("GOOD SWS.PRE6 Exp:%d Act:%d\n",uPktCnt,uData);
  }

  for(uXgmNum = 1; uXgmNum < SB_FE2000_NUM_XG_INTERFACES; uXgmNum++) {
      /* check XG1 rx/tx the packets */
      uData = SAND_HAL_READ(userDeviceHandle,CA,PT_PTE5_PKT_CNT);
      if (uData != uPktCnt) {
	  SB_ERROR("ERROR:%s, XGM1 TxExp:%d TxAct:%d\n",__PRETTY_FUNCTION__,uPktCnt,uData);
	  stat = -1;
      } else if (dbg) {
	  SB_LOG("GOOD XGM1 TxExp:%d TxAct:%d\n",uPktCnt,uData);
      }

      uData = SAND_HAL_READ(userDeviceHandle,CA,PR_PRE5_PKT_CNT);
      if (uData != uPktCnt) {
	  SB_ERROR("ERROR:%s, XGM1 RxExp:%d RxAct:%d\n",__PRETTY_FUNCTION__,uPktCnt,uData);
	  stat = -1;
      } else if (dbg) {
	  SB_LOG("GOOD XGM1 RxExp:%d RxAct:%d\n",uPktCnt,uData);
      }
  }

  /* get the packet */
  stat = sbFe2000DiagsReceivePciPacket(pDiagsInfo);
  if (stat != SB_FE2000_SUCCESS_E) {
      SB_ERROR("ERROR:%s sbFe2000DiagsRecievePciPacket failed with status %d\n",__PRETTY_FUNCTION__,stat);
      return (-1);
  }

  /* all nPacketsInjected packets should go back out PCI port */
  uData = SAND_HAL_READ(userDeviceHandle,CA,PT_PTE6_PKT_CNT);
  if (uData != uPktCnt) {
    SB_ERROR("ERROR:%s, SWS.PTE6 TxExp:%d TxAct:%d\n",__PRETTY_FUNCTION__,uPktCnt,uData);
    stat = -1;
  } else if (dbg) {
    SB_LOG("GOOD SWS.PTE6 TxExp:%d TxAct:%d\n",uPktCnt,uData);
  }

  DEXIT();
  return stat;
}

/* Loopback traffic from QE<--->10G */
int sbFe2000Diags10GQELoopback(sbxDiagsInfo_t *pDiagsInfo)
{

  int rv = -1;
  char rh_cmd[40] = {0};
  char pay_cmd[50] = {0};
  uint32_t unit, port_10g;
  uint32_t i;
  args_t *pArgs = NULL;
  int timer_sec = 0;
  int port = 0;
  int port_status;
  char *shim = "shim=0x00c0ffee";
  char len_cmd[5] = {0};
  sal_mac_addr_t l2_macDAaddr;
  sal_mac_addr_t l2_macSAaddr;
  char *l2_macSAaddrPkt1_str = NULL;
  char *l2_macDAaddrPkt1_str = NULL;
  char *l2_macSAaddr_str = NULL;
  char *l2_macDAaddr_str = NULL;
  int dbg,vlan;
  pbmp_t port_pbm;
  pbmp_t port_pbm_untag;
  int port_untag;
  bcm_l2_addr_t  l2addr;
  int qe = -1;
  uint8_t bPortIsDown = FALSE;
  sbhandle qe_hdl,fe_hdl;
  int femodid = 0;
  int qid = 0;
  int tme = 0;
  int num_cos = 0;
  bcm_vlan_control_vlan_t vlan_control;
  soc_sbx_g2p3_p2e_t pv2e;
  soc_sbx_g2p3_p2e_t *p_pv2e = NULL;
  l2_macSAaddrPkt1_str = "00:01:00:00:00:01";
  l2_macDAaddrPkt1_str = "00:01:00:00:00:02";

  unit = pDiagsInfo->unit;
  port_10g = pDiagsInfo->u10gPort;
  dbg = pDiagsInfo->debug_level;
#ifndef __KERNEL__
  signal(SIGINT,(void *)sigcatcher);
#endif
  vlan = 2;
  BCM_PBMP_CLEAR(port_pbm);
  BCM_PBMP_CLEAR(port_pbm_untag);

  sal_memset(&vlan_control,0x0,sizeof(bcm_vlan_control_vlan_t));
  sal_memset(&pv2e,0x0,sizeof(soc_sbx_g2p3_p2e_t));
  p_pv2e = &pv2e;

  tme = soc_property_get(unit,spn_QE_TME_MODE,0);
  num_cos = soc_property_get(unit, spn_BCM_NUM_COS, (tme == 1) ? 16 : 8);

  /* args structure is too big to replicate on local stack */
  if ((pArgs = sal_alloc(sizeof (args_t), "pArgs")) == NULL) {
    printk("(%s) Error: out of memory\n",FUNCTION_NAME());
    return CMD_FAIL;
  }  

  /* Set the error reg list to check for errors */
  sbxDiagsSetErrorRegList(pDiagsInfo,unit);

  for(i=0;i<soc_ndev;i++) {
    if (SOC_IS_SBX_QE2000(i)) {
      if (qe == -1) {
	qe = i;
	break;
      }
    }
  }

  if ( qe == -1 ) {
    printk("ERROR Could not locate QE on board\n");
    sal_free(pArgs);
    return (-1);
  }

  rv = bcm_stk_modid_get(unit,&femodid);
  if (BCM_FAILURE(rv)) {
    test_error(unit,
	       "%s failed when getting modid(%s) .\n",
	       SOC_CHIP_STRING(unit),bcm_errmsg(rv));
    sal_free(pArgs);
    return CMD_FAIL;
  }  


  sbx_clear_all(unit,0);

  BCM_PBMP_PORT_ADD(port_pbm,port_10g);
  BCM_PBMP_PORT_ADD(port_pbm_untag,port_10g);

  qe_hdl = SOC_SBX_CONTROL(qe)->sbhdl;
  fe_hdl = SOC_SBX_CONTROL(unit)->sbhdl;

  /* check that the ports are setup to be 10g ports */
  PBMP_ITER(port_pbm,port) {
    if (!IS_XE_PORT(unit,port)) {
      printk("port %d (%s) is not configured as 10g port\n",port,
	     SOC_PORT_NAME(unit,port));
      sal_free(pArgs);
      return (-1);
    }
  }

  /* check port status -- need to do this a few times to be sure */
  PBMP_ITER(port_pbm,port) {
    rv = bcm_port_link_status_get(unit,port,&port_status);
    if (BCM_FAILURE(rv)) {
      printk("ERROR: bcm_port_link_status_get(%d)(%s)\n",rv,bcm_errmsg(rv));
    }

    if (port_status == FALSE) {
      bPortIsDown = TRUE;
      printk("port %s is down\n",SOC_PORT_NAME(unit,port));
    }
  }

  if (bPortIsDown == TRUE) {
    sal_free(pArgs);
    return (-1);
  }

  rv = bcm_vlan_destroy_all(unit);
  if (BCM_FAILURE(rv)) {
    printk("ERROR: bcm_vlan_destory_all(%d)\n",rv);
    sal_free(pArgs);
    return (-1);
  }

  if (dbg) {
    printk("vlan create %d\n",vlan);
  }


  if (bcm_vlan_create(unit,vlan) < 0) {
    printk("ERROR: can not create vlan %d\n",vlan);
  }

  if (dbg) {
    printk("vlan add %d pbm=%d ubm=%d\n",vlan,port_10g,port_10g);
  }


  bcm_vlan_port_add(unit,vlan,port_pbm,port_pbm_untag);

  /* turn off learning */
  if ((rv = bcm_vlan_control_vlan_get(unit,vlan,&vlan_control)) < 0) {
    printk("ERROR: Could get vlan control for vlan:%d, (%s)\n",vlan,bcm_errmsg(rv));
    sal_free(pArgs);
    return (-1);
  }

  vlan_control.flags |= BCM_VLAN_LEARN_DISABLE;
  vlan_control.forwarding_mode |= bcmVlanForwardBridging;

  if ((rv = bcm_vlan_control_vlan_set(unit,vlan,vlan_control)) < 0) {
    printk("ERROR: Could set vlan control for vlan:%d, (%s)\n",vlan,bcm_errmsg(rv));
    sal_free(pArgs);
    return (-1);
  }  

  PBMP_ITER(port_pbm_untag,port_untag) {
    if ((rv = bcm_port_untagged_vlan_set(unit, port_untag, vlan)) < 0) {
      printk("Error setting port %s default VLAN to %d: %s\n",
	     SOC_PORT_NAME(unit, port_untag), vlan, bcm_errmsg(rv));
      sal_free(pArgs);
      return (-1);
    }
  }

  rv = bcm_l2_clear(unit);
  if (BCM_FAILURE(rv)) {
    printk("ERROR: bcm_l2_clear(%d)\n",rv);
    sal_free(pArgs);
    return (-1);
  }

  /* add l2 entries */
  l2_macSAaddr_str = l2_macSAaddrPkt1_str;
  parse_macaddr(l2_macSAaddr_str,l2_macSAaddr);
  bcm_l2_addr_t_init(&l2addr, l2_macSAaddr, vlan);

  l2addr.modid = femodid;
  l2addr.port = port_10g;
  l2addr.flags = BCM_L2_STATIC;

  if (dbg) {
    printk("l2 add macaddress=%s vlanid=%d module=%d pbm=%d static=true\n",
	   l2_macSAaddr_str,vlan,femodid,port_10g);
  }
  rv = bcm_l2_addr_add(unit,&l2addr);
  if (BCM_FAILURE(rv)) {
    printk("ERROR: bcm_l2_addr_add(%d)(%s)\n",rv,bcm_errmsg(rv));
    sal_free(pArgs);
    return (-1);
  }

  l2_macDAaddr_str = l2_macDAaddrPkt1_str;
  parse_macaddr(l2_macDAaddr_str,l2_macDAaddr);
  bcm_l2_addr_t_init(&l2addr, l2_macDAaddr, vlan);

  l2addr.modid = femodid;
  l2addr.port = port_10g;
  l2addr.flags = BCM_L2_STATIC;

  if (dbg) {
    printk("l2 add macaddress=%s vlanid=%d module=%d pbm=%d static=true\n",
	   l2_macDAaddr_str,vlan,femodid,port_10g);
  }
  rv = bcm_l2_addr_add(unit,&l2addr);
  if (BCM_FAILURE(rv)) {
    printk("ERROR: bcm_l2_addr_add(%d)(%s)\n",rv,bcm_errmsg(rv));
    sal_free(pArgs);
    return (-1);
  }

  /* avoid hairpin exception on ingress */
  if (( rv = sbFe2kxtDiagsSnakeTest_lpi_modify(pDiagsInfo)) < 0 ) {
    test_error(unit,
	       "%s: failed to modify lpi entries. \n",SOC_CHIP_STRING(unit));
    return(-1);
  }  

  /* set the default native vids for this port */
  p_pv2e->nativevid = vlan;
  if (( rv = soc_sbx_g2p3_p2e_set(unit,port_10g,p_pv2e)) < 0) {
    test_error(unit,
	       "%s: failed to set native vid for %s port. \n", SOC_CHIP_STRING(unit),
	       SOC_PORT_NAME(unit,port_10g));	       
  }

  /* get the qids */
  /* the first 10g port is actually port 12 from QE perspective */
  qid = SOC_SBX_NODE_PORT_TO_QID(unit,femodid,port_10g == 24 ? 12 : port_10g ,num_cos);

  /* build ERH for packets going from QE out xe port */
  sprintf(rh_cmd,"rh=0x%03x0%04x0000000000000000",qid/4,
	  pDiagsInfo->nInjectPacketsLen-8);

  parse_macaddr(l2_macDAaddrPkt1_str,l2_macDAaddr);
  parse_macaddr(l2_macSAaddrPkt1_str,l2_macSAaddr);

  sprintf(pay_cmd,"pat=0x%02d%02d%02d%02d%02d%02d%02d%02d%02d%02d%02d%02d9000",
	  l2_macDAaddr[0],l2_macDAaddr[1],l2_macDAaddr[2],l2_macDAaddr[3],l2_macDAaddr[4],l2_macDAaddr[5],
	  l2_macSAaddr[0],l2_macSAaddr[1],l2_macSAaddr[2],l2_macSAaddr[3],l2_macSAaddr[4],l2_macSAaddr[5]);




  printk("Injecting %d packets into %s PCI port..\n",pDiagsInfo->nInjectPackets*2,
	 SOC_CHIP_STRING(qe));

  /* Injected packets */
  for (i = 0; i < pDiagsInfo->nInjectPackets; i++) {
    sal_memset(pArgs,0x0,sizeof(args_t));
    sprintf(len_cmd,"len=%d",pDiagsInfo->nInjectPacketsLen);
    pArgs->a_cmd="SBX_TX";
    pArgs->a_argc = 5;
    pArgs->a_arg=1;
    pArgs->a_argv[0] = "sbx_tx";
    pArgs->a_argv[1] = rh_cmd;
    pArgs->a_argv[2] = shim;
    pArgs->a_argv[3] = pay_cmd;
    pArgs->a_argv[4] = len_cmd;
    if (dbg >= 2)
      printk("%d:sbx_tx %s %s %s %s\n", qe,pArgs->a_argv[1],shim,pArgs->a_argv[3],len_cmd);
    rv = cmd_soc_sbx_tx(qe,pArgs);
    if (rv != CMD_OK) {
      printk("ERROR: sbx_tx returned %d\n",rv);
      sal_free(pArgs);
      return (-1);
    }
  }    

  uXG_TxCount = 0; uXG_RxCount = 0;
  printk("Looping packet for roughly %d seconds ..\n",pDiagsInfo->uSnakeRunTime);
  timer_sec = 0;
  while (!gStopSnakeTest && timer_sec < pDiagsInfo->uSnakeRunTime) {
    sal_usleep(SNAKE_INTERVAL_US * 1E6);
    rv = sbxDiagsPollForErrors(port_10g,port_10g,unit);
    if (rv != 0) {
      break;
    }
    timer_sec += 1;
  }

  /* check again for errors to be sure */
  if (rv == 0 )
    rv = sbxDiagsCheckForErrors(unit);

  if ((gStopSnakeTest || timer_sec >= pDiagsInfo->uSnakeRunTime) && rv == 0) {
    printk("\nStopping Test. No Errors Found.\n");
  } else if (rv) {
    printk("\nStopping test due to errors. If errors persist try restarting bcm.user\n");
  }

  /* stop the packet from looping */
  if (pDiagsInfo->bStopLoopingPacket) {
    bcm_port_enable_set(unit,port_10g,0);
    thin_delay(100);
    bcm_port_enable_set(unit,port_10g,1);
  }

  sal_free(pArgs);
  return rv;
}

int sbFe2000Poll10GCounters(uint32_t unit,uint32_t port_10g)
{

    uint32_t uRetry = 3000;
    uint32_t bXG_Incrementing = 0;
    uint32_t uData = 0;
    uint32_t uXgmNum;
    int rv = 0;
    sbhandle hdl;

    hdl = SOC_SBX_CONTROL(unit)->sbhdl;
    uXgmNum = (port_10g == 24) ? 0 : 1;

    while ( --uRetry && !bXG_Incrementing) {
	if (port_10g == 24) {
	    uData = SAND_HAL_READ(hdl,CA,PT_PTE4_PKT_CNT);
	} else {
	    uData = SAND_HAL_READ(hdl,CA,PT_PTE5_PKT_CNT);
	}
	if (uXG_TxCount+uData <= uXG_TxCount) { /* if this count <= last count, pkt stopped */
	    thin_delay(300); continue;
	}  else {
	    uXG_TxCount += uData;
	    bXG_Incrementing = 1;
	}
    }

    if (uRetry == 0) {
	SB_LOG("XG%d: looping packet stopped unexpectedly\n",uXgmNum);
	rv = -1;
    }

    if (port_10g == 24) {
	uData = SAND_HAL_READ(hdl,CA,PR_PRE4_PKT_CNT);
    } else {
	uData = SAND_HAL_READ(hdl,CA,PR_PRE5_PKT_CNT);
    }

    uXG_RxCount += uData;
    SB_LOG("XG%d Rx:%lld Tx:%lld \r",uXgmNum, uXG_RxCount, uXG_TxCount);
    return rv;
}



/*
 *   Setup 10G loobacks (Internal or External)
 *   (comments from verification Test 10.102)
 *   Inject packets into PCI, have the packet loopback external to the xaui
 *   , and exit PCI.  Check packet counter to ascertain that the packet
 *   managed to traverse the sws->bigmac->xaui(external)->bigmac->sws.
 *   You need external 10G loopback cables that are looped back onto itself.
 */

int sbFe2000Diags10GLoopback(sbxDiagsInfo_t *pDiagsInfo)
{

  uint32_t uPktCnt;
  sbhandle userDeviceHandle;
  uint32_t uXgmNum = 0;
  uint32_t unit;
  int rv = 0;
  uint32_t stat = 0;
  uint32_t loopback;
  uint32_t uData;
  uint32_t dbg;
  char *s = NULL;
  char *portType = NULL;
  uint32_t port_10g = 1;
  uint32_t uQueue = 120;
  uint32_t dst_interface;

  unit = pDiagsInfo->unit;
  userDeviceHandle = SOC_SBX_CONTROL(unit)->sbhdl;
  uPktCnt = pDiagsInfo->nInjectPackets;
  dbg = pDiagsInfo->debug_level;
  port_10g = pDiagsInfo->u10gPort;
  uXgmNum = port_10g - 24;

  SB_LOG("10G port %d\n",port_10g);
  /* By-pass the LRP, this test should not require any microcode */
  uData = SAND_HAL_READ(userDeviceHandle,CA,LR_CONFIG0);
  uData = SAND_HAL_MOD_FIELD(CA,LR_CONFIG0,BYPASS,uData,0x1);
  SAND_HAL_WRITE(userDeviceHandle, CA, LR_CONFIG0, uData);

  if (uXgmNum == 0) {
    portType = "Cu";
  } else {
    portType = "XFP";
  }

  dst_interface = (pDiagsInfo->bSnakeLoopOn) ? uXgmNum+4 : 6;

  if (pDiagsInfo->bInternalLpk) {
    s = "INTERNAL";
    loopback = BCM_PORT_LOOPBACK_MAC;
  } else if (uXgmNum == 1 && pDiagsInfo->b10GPhyLpk) {
    s = "PHY";
    loopback = BCM_PORT_LOOPBACK_PHY;
  } else {
    s = "EXTERNAL";
    loopback = BCM_PORT_LOOPBACK_NONE;
  }

  SB_LOG("10G %s %s Loopback Test\n",portType,s);
  rv = sbFe200010GLoopbackSet(unit,port_10g,loopback);
  if (rv) {
    return (-1);
  }

  /* create the internal SWS connections */

  if (uXgmNum == 0) {
    if (dbg) {
      SB_LOG("Creating SWS connection from %s.p%d to %s.p%d using queue:%d\n",
	     sbFe2000DiagsPortInterfaceStr[6],0,sbFe2000DiagsPortInterfaceStr[4],0,uQueue);
    }
    AddQueue(userDeviceHandle,6/*uInPre*/,0/*uInPort*/,4/*uOutPte*/,0/*uOutPort*/,uQueue);   /* PCI->XGM0 using queue0 */
    uQueue++;
    if (dbg) {
      SB_LOG("Creating SWS connection from %s.p%d to %s.p%d using queue:%d\n",
	     sbFe2000DiagsPortInterfaceStr[4],0,sbFe2000DiagsPortInterfaceStr[dst_interface],0,uQueue);
    }
    AddQueue(userDeviceHandle,4/*uInPre*/,0/**uInPort*/,dst_interface/*uOutPte*/,0/*uOutPort*/,uQueue);  /* XGM0-->PCI  or back onto itself */
  } else {
    if (dbg) {
      SB_LOG("Creating SWS connection from %s.p%d to %s.p%d using queue:%d\n",
	     sbFe2000DiagsPortInterfaceStr[6],0,sbFe2000DiagsPortInterfaceStr[5],0,uQueue);
    }
    AddQueue(userDeviceHandle,6/*uInPre*/,0/*uInPort*/,5/*uOutPte*/,0/*uOutPort*/,uQueue);   /* PCI->XGM1 using queue0 */
    uQueue++;
    if (dbg) {
      SB_LOG("Creating SWS connection from %s.p%d to %s.p%d using queue:%d\n",
	     sbFe2000DiagsPortInterfaceStr[5],0,sbFe2000DiagsPortInterfaceStr[dst_interface],0,uQueue);
    }
    AddQueue(userDeviceHandle,5/*uInPre*/,0/**uInPort*/,dst_interface/*uOutPte*/,0/*uOutPort*/,uQueue);  /* XGM1 -->PCI or back onto itself */
  }

  stat = sbFe2000DiagsInjectPciPacket(pDiagsInfo);
  if (stat != SB_FE2000_SUCCESS_E) {
    SB_ERROR("ERROR:%s sbFe2000DiagsInjectPciPacket failed with status %d\n",__PRETTY_FUNCTION__,stat);
    return(-1);
  }

  uData = SAND_HAL_READ(userDeviceHandle,CA,PC_TX_PKT_CNT);
  if (uData != uPktCnt) {
    SB_ERROR("ERROR:%s, PCI TxExp:%d TxAct:%d\n",__PRETTY_FUNCTION__,uPktCnt,uData);
    stat = -1;
  } else if (dbg) {
    SB_LOG("GOOD:%s, PCI TxExp:%d TxAct:%d\n",__PRETTY_FUNCTION__,uPktCnt,uData);
  }

  /* check packets where accepted by the QM block */
  uData = SAND_HAL_READ(userDeviceHandle,CA,PR_PRE6_PKT_CNT);
  if (uData != uPktCnt) {
    SB_ERROR("ERROR:%s, SWS(PCI) RxExp:%d RxAct:%d\n",__PRETTY_FUNCTION__,uPktCnt,uData);
    stat = -1;
  } else if (dbg) {
    SB_LOG("GOOD:%s, SWS(PCI) RxExp:%d RxAct:%d\n",__PRETTY_FUNCTION__,uPktCnt,uData);
  }

  if (uXgmNum == 0) {
    uData = SAND_HAL_READ(userDeviceHandle,CA,PT_PTE4_PKT_CNT);
  } else {
    uData = SAND_HAL_READ(userDeviceHandle,CA,PT_PTE5_PKT_CNT);
  }
  if (uData != uPktCnt) {
    SB_ERROR("ERROR: XGM%d TxExp:%d TxAct:%d\n",uXgmNum,uPktCnt,uData);
    stat = -1;
  } else if (dbg) {
    SB_LOG("GOOD: XGM%d TxExp:%d TxAct:%d\n",uXgmNum,uPktCnt,uData);
  }
  if (uXgmNum == 0 ) {
    uData = SAND_HAL_READ(userDeviceHandle,CA,PR_PRE4_PKT_CNT);
  } else {
    uData = SAND_HAL_READ(userDeviceHandle,CA,PR_PRE5_PKT_CNT);
  }

  if (uData != uPktCnt) {
    SB_ERROR("ERROR XGM%d RxExp:%d RxAct:%d\n",uXgmNum,uPktCnt,uData);
    stat = -1;
  } else if (dbg) {
    SB_LOG("GOOD XGM%d RxExp:%d RxAct:%d\n",uXgmNum,uPktCnt,uData);
  }

  /* check that the packet came out the PCI port, pull it out of buffer pte6 */
  stat = sbFe2000DiagsReceivePciPacket(pDiagsInfo);
  if (stat != SB_FE2000_SUCCESS_E) {
    SB_ERROR("ERROR:%s sbFe2000DiagsRecievePciPacket failed with status %d\n",__PRETTY_FUNCTION__,stat);
    return (-1);
  }

  /* all nPacketsInjected packets should have gotten sent to pci */
  uData = SAND_HAL_READ(userDeviceHandle,CA,PT_PTE6_PKT_CNT);
  if (uData != uPktCnt) {
    SB_ERROR("ERROR:%s, SWS.PTE6 TxExp:%d TxAct:%d\n",__PRETTY_FUNCTION__,uPktCnt,uData);
    stat = -1;
  } else if (dbg) {
    SB_LOG("GOOD:%s, SWS.PTE6 TxExp:%d TxAct:%d\n",__PRETTY_FUNCTION__,uPktCnt,uData);
  }

  return stat;

}

int
sbFe200010GLoopbackSet(uint32_t unit, uint32_t port_10g,uint32_t loopback)
{

  int rv = 0;
  uint32_t uXgmNum = 0;
  uint32_t uDataLo = 0;
  uint32_t uDataHi = 0;
  uint32_t uLinkScanOn;
  uint32_t xfp_addr;
  sbhandle hdl;
  if (loopback != BCM_PORT_LOOPBACK_NONE) {
    rv = bcm_port_loopback_set(unit,port_10g,loopback);
    if (rv) {
      SB_LOG("ERROR bcm_port_loopback_set failed(%d)\n");
      return (-1);
    }
  } else {
    /* todo find out why above API does not do LOOPBACK_NONE correctly for 10g ports */
    hdl = SOC_SBX_CONTROL(unit)->sbhdl;
    uXgmNum = (port_10g == 24) ? 0 : 1;
    uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_CONFIG_LO, RX_ENABLE,
				 uDataLo, 1);
    uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_CONFIG_LO, TX_ENABLE,
				 uDataLo, 1);

    uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_CONFIG_LO, LOCAL_LOOPBACK_ENABLE,  uDataLo, 0);
    rv = sbFe2000UtilXgmWrite(hdl,uXgmNum,
				SAND_HAL_REG_OFFSET(CA, XM_MAC_CONFIG_LO), uDataHi, uDataLo);
    if (rv != SB_FE2000_SUCCESS_E) {
      SB_ERROR("ERROR:\n%s sbFe2000UtilXgmWrite for Xgm%d failed with status %d\n",__PRETTY_FUNCTION__,uXgmNum,rv);
      return(-1);
    }

    if (uXgmNum == 1) {
      xfp_addr =  PORT_TO_PHY_ADDR(1,port_10g); /* (u,p)  */
      uDataLo = SAND_HAL_READ(hdl, CA, PC_MIIM_CONTROL0);
      uLinkScanOn = SAND_HAL_GET_FIELD(CA,PC_MIIM_CONTROL0,MIIM_LINK_SCAN_EN,uDataLo);
      if(uLinkScanOn) { /* turn it off */
	SB_LOG("%s Disable link scan to start XGM mdio access\n",__PRETTY_FUNCTION__);
	uDataLo = SAND_HAL_MOD_FIELD(CA, PC_MIIM_CONTROL0, MIIM_LINK_SCAN_EN, uDataLo, 0x0);
	SAND_HAL_WRITE(hdl, CA, PC_MIIM_CONTROL0, uDataLo);
      }

      rv = sbFe2000UtilXgmMiimRead( hdl,0x0/*bInternalPhy*/,0x1/*bClause45*/,0x1/*uDevAddr*/,
				      xfp_addr/*uPhyOrPortAddr=0x18/0x19*/,0/*uRegAddr*/,&uDataLo/* &uData*/);

      uDataLo &= ~(1<<0); /* clear internal loopback */
      rv = sbFe2000UtilXgmMiimWrite(hdl,0x0/*bInternalPhy*/,0x1/*bClause45*/,0x1/*uDevAddr*/,
				      xfp_addr/*uPhyOrPortAddr*/,0/*uRegAddr*/,uDataLo/*uData*/);

      if (rv != SB_FE2000_SUCCESS_E) {
	SB_ERROR("ERROR:%s sbFe2000UtilXgmMiimWrite failed with status %d\n",__PRETTY_FUNCTION__,rv);
	return(-1);
      }

      if (uLinkScanOn) { /* turn it back on */
	SB_LOG("%s Re-enable Link Scan\n",__PRETTY_FUNCTION__);
	uDataLo = SAND_HAL_READ(hdl, CA, PC_MIIM_CONTROL0);
	uDataLo = SAND_HAL_MOD_FIELD(CA, PC_MIIM_CONTROL0, MIIM_LINK_SCAN_EN, uDataLo, 0x1);
	SAND_HAL_WRITE(hdl, CA, PC_MIIM_CONTROL0, uDataLo);
      }
    }
  }

  return SB_OK;
}


/*
 *   Setup Unimac loopback
 *   (comments from verification Test 10.103)
 *   Inject packets into PCI, have the packet internally loopback in the
 *   Unimac, and exit PCI.  Check packet counter to ascertain that the packet
 *   managed to traverse the sws->Unimac->sws. This does not loop the packet
 *   at only one port, but snakes the packet through all 1G ports. Similiar
 *   to serdes loopback tests, but this is looped back at the GP, before the
 *   serdes.
 */
int sbFe2000DiagsUnimacLoopback(sbxDiagsInfo_t *pDiagsInfo)
{
  uint32_t nAgm;
  uint32_t uQueue=200;  /* temp starting queue , pick queue not used by gu2k */
  uint32_t nPort;
  uint32_t uData=0;
  int stat=0;
  sbhandle userDeviceHandle;
  uint32_t nPacketsInjected;
  int dbg;
  userDeviceHandle = pDiagsInfo->userDeviceHandle;
  nPacketsInjected = pDiagsInfo->nInjectPackets;
  dbg = pDiagsInfo->debug_level;

  DENTER();
  SB_LOG("Running 1G Internal(Unimac) Loopback Test\n");

  /* check if queues have been setup */
  if (!g_bInternalQueuesSetup[pDiagsInfo->unit & 7] || pDiagsInfo->reinit) {
    /* connect PCI.p0 -> AGM0.p0 */
    if (dbg) {
    SB_LOG("Creating SWS connection from %s.p%d to %s.p%d using queue:%d\n",
	   sbFe2000DiagsPortInterfaceStr[6],0,sbFe2000DiagsPortInterfaceStr[2],0,uQueue);
    }
    AddQueue(userDeviceHandle,6/*uInPre*/,0/*uInPort*/,2/*uOutPte*/,0/*uOutPort*/,uQueue);
    uQueue++;

    for(nAgm=0;nAgm<SB_FE2000_NUM_AG_INTERFACES;nAgm++) {
      for(nPort=1;nPort<12;nPort++) {
	if (dbg) {
	SB_LOG("Creating SWS connection from %s.p%d to %s.p%d using queue:%d\n",
	       sbFe2000DiagsPortInterfaceStr[2+nAgm],nPort-1,sbFe2000DiagsPortInterfaceStr[2+nAgm],nPort,uQueue);
	}
	AddQueue(userDeviceHandle,2+nAgm/*uInPre*/,nPort-1/*uInPort*/,2+nAgm/*uOutPte*/,nPort/*uOutPort*/,uQueue);
	uQueue++;
      }
      if (nAgm == 0) {
	/* now connect AGM0.p11 -> AGM1.p0 */
	if (dbg) {
	SB_LOG("Creating SWS connection from %s.p%d to %s.p%d using queue:%d\n",
	       sbFe2000DiagsPortInterfaceStr[2],11,sbFe2000DiagsPortInterfaceStr[3],0,uQueue);
	}
	AddQueue(userDeviceHandle,2/*uInPre*/,11/*uInPort*/,3/*uOutPte*/,0/*uOutPort*/,uQueue);
	uQueue++;
      } else if (nAgm==1) {
	/* now connect AGM1.p11 -> PCI.p0 */
	if (dbg) {
	SB_LOG("Creating SWS connection from %s.p%d to %s.p%d using queue:%d\n",
	       sbFe2000DiagsPortInterfaceStr[3],11,sbFe2000DiagsPortInterfaceStr[6],0,uQueue);
	}
	AddQueue(userDeviceHandle,3/*uInPre*/,11/*uInPort*/,6/*uOutPte*/,0/*uOutPort*/,uQueue);
	uQueue++;
      }
    }
    g_bInternalQueuesSetup[pDiagsInfo->unit & 7] = 1;
  }

  stat = sbFe2000DiagsInjectPciPacket(pDiagsInfo);
  if (stat != SB_FE2000_SUCCESS_E) {
    SB_ERROR("ERROR:%s sbFe2000DiagsInjectPciPacket failed with status %d\n",__PRETTY_FUNCTION__,stat);
    return(-1);
  }

  thin_delay(100);

  uData = SAND_HAL_READ(userDeviceHandle,CA,PC_TX_PKT_CNT);
  if (uData != nPacketsInjected) {
    SB_ERROR("ERROR:%s, PCI TxExp:%d TxAct:%d\n",__PRETTY_FUNCTION__,nPacketsInjected,uData);
    stat = -1;
  } else if (dbg) {
    SB_LOG("GOOD PCI TxExp:%d TxAct:%d\n",nPacketsInjected,uData);
  }

  /* check packets where accepted by the QM block */
  uData = SAND_HAL_READ(userDeviceHandle,CA,PR_PRE6_PKT_CNT);
  if (uData != nPacketsInjected) {
    SB_ERROR("ERROR:%s, SWS.PCI(PRE6) Exp:%d Act:%d\n",__PRETTY_FUNCTION__,nPacketsInjected,uData);
    stat = -1;
  } else if (dbg) {
    SB_LOG("GOOD SWS.PCI(PRE6) Exp:%d Act:%d\n",nPacketsInjected,uData);
  }

  stat = sbFe2000CheckLoopbackStats(pDiagsInfo,0);
  if (stat != SB_FE2000_SUCCESS_E) {
      SB_ERROR("ERROR:1G Loopback Stats Check failed\n",__PRETTY_FUNCTION__);
      stat = -1;
  }
  stat = sbFe2000DiagsReceivePciPacket(pDiagsInfo);
  if (stat != SB_FE2000_SUCCESS_E) {
    SB_ERROR("ERROR:%s sbFe2000DiagsRecievePciPacket failed with status %d\n",__PRETTY_FUNCTION__,stat);
    return (-1);
  }

  /* all nPacketsInjected packets should go back out PCI port */
  uData = SAND_HAL_READ(userDeviceHandle,CA,PT_PTE6_PKT_CNT);
  if (uData != nPacketsInjected) {
    SB_ERROR("ERROR:%s, SWS.PTE6 TxExp:%d TxAct:%d\n",__PRETTY_FUNCTION__,nPacketsInjected,uData);
    stat = -1;
  } else if (dbg) {
    SB_LOG("GOOD SWS.PTE6 TxExp:%d TxAct:%d\n",nPacketsInjected,uData);
  }


  return stat;

}
/*
 *  Setup Serdes External Loopback Test
 *   (comments from verification Test 10.105)
 *   Inject packets into PCI, have the packet loopback external of the
 *   Serdes, switch/snake the packet through each serdes port, and exit PCI.
 *   Check packet counter to ascertain that the packet managed to traverse
 *   the sws->unimac->serdes(external)->unimac->sws.
 *
 *  We need to create the following connections internally to allow loopbacks to work:
 *  The dashed lines are the internal SWS connections. The external loopbacks are shown
 *  in ++ lines. This is modified from orginal 10.105 verification test.
 *
 *   AG0
 *  ++++ 0 <--- PCI
 *  +
 *  ++++ 1 -----
 *              |
 *  ++++ 2 -----
 *  +
 *  ++++ 3 -----
 *              |
 *  ++++ 4 -----
 *  +
 *  ++++ 5 -----
 *              |
 *  ++++ 6 -----
 *  +
 *  ++++ 7 -----
 *              |
 *  ++++ 8 -----
 *  +
 *  ++++ 9 -----
 *              |
 *  ++++ 10 -----
 *  +
 *  ++++ 11 -----
 *   AG1        |
 *  ++++ 0  -----
 *  +
 *  ++++ 1 -----
 *              |
 *  ++++ 2 -----
 *  +
 *  ++++ 3 -----
 *              |
 *  ++++ 4 -----
 *  +
 *  ++++ 5 -----
 *              |
 *  ++++ 6 -----
 *  +
 *  ++++ 7 -----
 *              |
 *  ++++ 8 -----
 *  +
 *  ++++ 9 -----
 *              |
 *  ++++ 10 -----
 *  +
 *  ++++ 11 -----> PCI
 */

int sbFe2000DiagsSerdesExternalLoopback(sbxDiagsInfo_t *pDiagsInfo)
{

/*   uint32_t nAgm; */
/*   uint32_t nPort; */
  uint32_t uData = 0;
  uint32_t stat=0;
  sbhandle userDeviceHandle;
  uint32_t nPacketsInjected;
  int dbg = pDiagsInfo->debug_level;
  userDeviceHandle = pDiagsInfo->userDeviceHandle;
  nPacketsInjected = pDiagsInfo->nInjectPackets;
  DENTER();

  SB_LOG("Running 1G External Loopback Test\n");

  /* By-pass the LRP, this test should not require any microcode */
  uData = SAND_HAL_READ(userDeviceHandle,CA,LR_CONFIG0);
  uData = SAND_HAL_MOD_FIELD(CA,LR_CONFIG0,BYPASS,uData,0x1);
  SAND_HAL_WRITE(userDeviceHandle, CA, LR_CONFIG0, uData);

  /* create the queues for 1G loopback */
  sbFe2000Create1GLoopbackQueues(pDiagsInfo);

  stat = sbFe2000DiagsInjectPciPacket(pDiagsInfo);
  if (stat != SB_FE2000_SUCCESS_E) {
    SB_ERROR("ERROR:%s sbFe2000DiagsInjectPciPacket failed with status %d\n",__PRETTY_FUNCTION__,stat);
    return stat;
  }

  thin_delay(500);
  uData = SAND_HAL_READ(userDeviceHandle,CA,PC_TX_PKT_CNT);
  if (uData != nPacketsInjected) {
    SB_ERROR("ERROR:%s, PCI->FE2k TxExp:%d TxAct:%d\n",__PRETTY_FUNCTION__,nPacketsInjected,uData);
    /* just return, test is done for */
    return (-1);
  } else {
      if (dbg) {
	  SB_LOG("GOOD PCI->FE2k TxExp:%d TxAct:%d\n",nPacketsInjected,uData);
      }
  }

  stat = sbFe2000CheckLoopbackStats(pDiagsInfo,1);
  if (stat != SB_FE2000_SUCCESS_E) {
      SB_ERROR("ERROR:1G Loopback Stats Check failed\n",__PRETTY_FUNCTION__);
      return (-1);
  }

  /* check that the packet came out the PCI port */
  if (!pDiagsInfo->bSnakeLoopOn) {
      stat = sbFe2000DiagsReceivePciPacket(pDiagsInfo);
    if (stat != SB_FE2000_SUCCESS_E) {
      SB_ERROR("ERROR:%s sbFe2000DiagsRecievePciPacket failed with status %d\n",__PRETTY_FUNCTION__,stat);
      return stat;
    }
  }
  DEXIT();
  return 0;

}

void sbFe2000Create1GLoopbackQueues(sbxDiagsInfo_t *pDiagsInfo)
{

    int dbg;
    uint32_t uQueue=100; /* temp start at queue not used */
    sbhandle hdl;
    uint32_t nPort;
    uint32_t sp;
    uint32_t ep;
    uint32_t sp_interface;
    uint32_t frm_interface;
    uint32_t to_interface;
    uint32_t ep_interface;

    hdl = pDiagsInfo->userDeviceHandle;
    dbg = pDiagsInfo->debug_level;
    sp = pDiagsInfo->start_port;
    ep = pDiagsInfo->end_port;

    sp = sp%12;
    ep = ep%12;

    sp_interface = (pDiagsInfo->start_port < 12) ? SB_FE2000_IF_PTYPE_AGM0 : SB_FE2000_IF_PTYPE_AGM1;
    ep_interface = (pDiagsInfo->end_port < 12) ? SB_FE2000_IF_PTYPE_AGM0 : SB_FE2000_IF_PTYPE_AGM1;

    /* connect PCI.p0 to start port */
    if (dbg) {
    SB_LOG("Creating SWS connection from %s.%d to %s.%d using queue:%d\n",
	   sbFe2000DiagsPortInterfaceStr[SB_FE2000_IF_PTYPE_PCI],0,
	   sbFe2000DiagsPortInterfaceStr[sp_interface],sp,uQueue);
    }
    AddQueue(hdl,SB_FE2000_IF_PTYPE_PCI,0,sp_interface,sp,uQueue);
    uQueue++;

    g_AGTxPorts[sp_interface-2][sp] = 1;

    /* daisy chain the rest of the ports together */
    for(nPort=pDiagsInfo->start_port+1;nPort<pDiagsInfo->end_port;nPort+=2) {
      frm_interface = (nPort < 12) ? SB_FE2000_IF_PTYPE_AGM0 : SB_FE2000_IF_PTYPE_AGM1;
      to_interface = (nPort+1 < 12) ? SB_FE2000_IF_PTYPE_AGM0 : SB_FE2000_IF_PTYPE_AGM1;
      if (dbg) {
      SB_LOG("Creating SWS connection from %s.%d to %s.%d using queue:%d\n",
	     sbFe2000DiagsPortInterfaceStr[frm_interface],nPort%12,
	     sbFe2000DiagsPortInterfaceStr[to_interface],(nPort+1)%12,uQueue);
      }
      g_AGTxPorts[to_interface-2][(nPort+1)%12] = 1;
      g_AGRxPorts[frm_interface-2][nPort%12] = 1;
      AddQueue(hdl,frm_interface,nPort%12,to_interface,(nPort+1)%12,uQueue);
      uQueue++;
    }

    /* for the last connection, back to start port or out PCI (for debug ) */
    if (pDiagsInfo->bSnakeLoopOn) {
      if (dbg) {
      SB_LOG("Creating SWS connection from %s.%d to %s.%d using queue:%d\n",
	     sbFe2000DiagsPortInterfaceStr[ep_interface],ep,
	     sbFe2000DiagsPortInterfaceStr[sp_interface],sp,uQueue);
      }
      AddQueue(hdl,ep_interface,ep%12,sp_interface,sp%12,uQueue);
    } else {
      if (dbg) {
      SB_LOG("Creating SWS connection from %s.%d to %s.%d using queue:%d\n",
	     sbFe2000DiagsPortInterfaceStr[ep_interface],ep,
	     sbFe2000DiagsPortInterfaceStr[SB_FE2000_IF_PTYPE_PCI],0,uQueue);
      }
      g_AGRxPorts[ep_interface-2][ep] = 1;
      AddQueue(hdl,ep_interface,ep,SB_FE2000_IF_PTYPE_PCI,0,uQueue);
    }
}

int sbFe2000CheckLoopbackStats(sbxDiagsInfo_t *pDiagsInfo,
			       uint8_t bExternal)
{

  uint32_t nAgm;
  uint32_t nPort;
  uint32_t nPackets = pDiagsInfo->nInjectPackets;
  uint32_t nDbg = pDiagsInfo->debug_level;
  int unit = pDiagsInfo->unit;
  uint32_t uData = 0;
  int rv = 0;

  if (bExternal) {
    /* check port tx */
    for(nAgm=0;nAgm<SB_FE2000_NUM_AG_INTERFACES;nAgm++) {
      for(nPort=0;nPort<SB_FE2000_MAX_AG_PORTS;nPort++) {
	if (g_AGTxPorts[nAgm][nPort]) {
	  FE2000_STS_CHECK
	      (soc_sbx_fe2000_am_read(unit,nAgm,nPort,REG_OFFSET(AM_TX_VALID_PKT_CNT),&uData));
	  if (uData != nPackets) {
	    SB_ERROR("ERROR:AGM%d.p%d TxExp:%d got TxAct:%d\n",nAgm,nPort,nPackets,uData);
	    rv = -1;
	  } else {
	    if (nDbg) {
	      SB_LOG("GOOD AGM%d.p%d TxExp:%d got TxAct:%d\n",nAgm,nPort,nPackets,uData);
	    }
	  }
	}
	/* check port rx */
	if (g_AGRxPorts[nAgm][nPort]) {
	  FE2000_STS_CHECK
	      (soc_sbx_fe2000_am_read(unit,nAgm,nPort,REG_OFFSET(AM_RX_VALID_PKT_CNT),&uData));
	  if (uData != nPackets) {
	    SB_ERROR("ERROR:AGM%d.p%d RxExp:%d got RxAct:%d\n",nAgm,nPort,nPackets,uData);
	    rv = -1;
	  } else {
	    if (nDbg) {
	      SB_LOG("GOOD AGM%d.p%d RxExp:%d got RxAct:%d\n",nAgm,nPort,nPackets,uData);
	    }
	  }
	}
      }
    }
  } else { /* internal loopbacks are looped back onto itself */
    for(nAgm=0;nAgm<SB_FE2000_NUM_AG_INTERFACES;nAgm++) {
      for(nPort=0;nPort<SB_FE2000_MAX_AG_PORTS;nPort++) {
	  FE2000_STS_CHECK
	      (soc_sbx_fe2000_am_read(unit,nAgm,nPort,REG_OFFSET(AM_TX_VALID_PKT_CNT),&uData));
	if (uData != nPackets) {
	  SB_ERROR("ERROR:AGM%d.p%d TxExp:%d got TxAct:%d\n",nAgm,nPort,nPackets,uData);
	  rv = -1;
	} else {
	  if (nDbg) {
	    SB_LOG("GOOD AGM%d.p%d TxExp:%d got TxAct:%d\n",nAgm,nPort,nPackets,uData);
	  }
	}
	FE2000_STS_CHECK
	    (soc_sbx_fe2000_am_read(unit,nAgm,nPort,REG_OFFSET(AM_RX_VALID_PKT_CNT),&uData));

	if (uData != nPackets) {
	  SB_ERROR("ERROR:AGM%d.p%d RxExp:%d got RxAct:%d\n",nAgm,nPort,nPackets,uData);
	  rv = -1;
	} else {
	  if (nDbg) {
	    SB_LOG("GOOD AGM%d.p%d RxExp:%d got RxAct:%d\n",nAgm,nPort,nPackets,uData);
	  }
	}
      }
    }
  }
  return rv;
}

/*
 *  Setup Serdes Internal Loopback Test (like external loopback, but no cables needed)
 *   (comments from verification Test 10.104)
 *   Inject packets into PCI, have the packet loopback external of the
 *   Serdes, switch/snake the packet through each serdes port, and exit PCI.
 *   Check packet counter to ascertain that the packet managed to traverse
 *   the sws->unimac->serdes(internal)->unimac->sws.
 *
 *   Agm0 instance refers to the RJ45 1G ports. (0-11). PHYS are accessible via MDIO
 *   Agm1 instance refers to the SFPs (0-11). SFPs Internal phys are accessible via MDIO
 */

int sbFe2000DiagsSerdesInternalLoopback(sbxDiagsInfo_t *pDiagsInfo)
{
  uint32_t nAgm;
  uint32_t nPort;
  uint32_t uData;
  uint32_t stat=0;
  sbhandle userDeviceHandle;
  uint32_t nPacketsInjected;
  int dbg;
  uint32_t uQueue=200; /* temp starting queue , pick queue not used by gu2k */

  userDeviceHandle = pDiagsInfo->userDeviceHandle;
  nPacketsInjected = pDiagsInfo->nInjectPackets;
  dbg = pDiagsInfo->debug_level;
  DENTER();

  SB_LOG("Running 1G Internal(PHY) Loopback Test\n");

  /* By-pass the LRP, this test should not require any microcode */
#if 0
  uData = SAND_HAL_READ(userDeviceHandle,CA,LR_CONFIG0);
  uData = SAND_HAL_MOD_FIELD(CA,LR_CONFIG0,BYPASS,uData,0x1);
  SAND_HAL_WRITE(userDeviceHandle, CA, LR_CONFIG0, uData);
#endif

  /* create the queues , just do this once */
  if (!g_bInternalQueuesSetup[pDiagsInfo->unit & 7] || pDiagsInfo->reinit) {
    /* connect PCI.p0 -> AGM0.p0 */
    if (dbg) {
    SB_LOG("Creating SWS connection from %s.p%d to %s.p%d using queue:%d\n",
	   sbFe2000DiagsPortInterfaceStr[6],0,sbFe2000DiagsPortInterfaceStr[2],0,uQueue);
    }
    AddQueue(userDeviceHandle,6/*uInPre*/,0/*uInPort*/,2/*uOutPte*/,0/*uOutPort*/,uQueue);
    uQueue++;

    for(nAgm=0;nAgm<SB_FE2000_NUM_AG_INTERFACES;nAgm++) {
      for(nPort=1;nPort<12;nPort++) {
	if (dbg) {
	SB_LOG("Creating SWS connection from %s.p%d to %s.p%d using queue:%d\n",
	       sbFe2000DiagsPortInterfaceStr[2+nAgm],nPort-1,sbFe2000DiagsPortInterfaceStr[2+nAgm],nPort,uQueue);
	}
	AddQueue(userDeviceHandle,2+nAgm/*uInPre*/,nPort-1/*uInPort*/,2+nAgm/*uOutPte*/,nPort/*uOutPort*/,uQueue);
	uQueue++;
      }
      if (nAgm == 0) {
	/* now connect AGM0.p11 -> AGM1.p0 */
	if (dbg) {
	SB_LOG("Creating SWS connection from %s.p%d to %s.p%d using queue:%d\n",
	       sbFe2000DiagsPortInterfaceStr[2],11,sbFe2000DiagsPortInterfaceStr[3],0,uQueue);
	}
	AddQueue(userDeviceHandle,2/*uInPre*/,11/*uInPort*/,3/*uOutPte*/,0/*uOutPort*/,uQueue);
	uQueue++;
      } else if (nAgm==1) {
	/* now connect AGM1.p11 -> PCI.p0 */
	if (dbg) {
	SB_LOG("Creating SWS connection from %s.p%d to %s.p%d using queue:%d\n",
	       sbFe2000DiagsPortInterfaceStr[3],11,sbFe2000DiagsPortInterfaceStr[6],0,uQueue);
	}
	AddQueue(userDeviceHandle,3/*uInPre*/,11/*uInPort*/,6/*uOutPte*/,0/*uOutPort*/,uQueue);
	uQueue++;
      }
    }
    g_bInternalQueuesSetup[pDiagsInfo->unit & 7] = 1;
  }

  stat = sbFe2000DiagsInjectPciPacket(pDiagsInfo);
  if (stat != SB_FE2000_SUCCESS_E) {
    SB_ERROR("ERROR:%s sbFe2000DiagsInjectPciPacket failed with status %d\n",__PRETTY_FUNCTION__,stat);
    return(-1);
  }

  uData = SAND_HAL_READ(userDeviceHandle,CA,PC_TX_PKT_CNT);
  if (uData != nPacketsInjected) {
    SB_ERROR("ERROR:%s, PCI->FE2k TxExp:%d TxAct:%d\n",__PRETTY_FUNCTION__,nPacketsInjected,uData);
    /* just return, test is done for */
    return (-1);
  } else {
      if (dbg) {
	  SB_LOG("GOOD PCI->FE2k TxExp:%d TxAct:%d\n",nPacketsInjected,uData);
      }
  }
  stat = sbFe2000CheckLoopbackStats(pDiagsInfo,0);
  if (stat != SB_FE2000_SUCCESS_E) {
      SB_ERROR("ERROR:1G Loopback Stats Check failed\n",__PRETTY_FUNCTION__);
      return (stat);
  }

  /* check that the packet came out the PCI port */
  stat = sbFe2000DiagsReceivePciPacket(pDiagsInfo);
  if (stat != SB_FE2000_SUCCESS_E) {
    SB_ERROR("ERROR:%s sbFe2000DiagsRecievePciPacket failed with status %d\n",__PRETTY_FUNCTION__,stat);
    return stat;
  }

  DEXIT();
  return stat;
}

uint8_t sbFe2000DiagsPvtCompensation(sbhandle userDeviceHandle, uint32_t uMmInstance)
{
  return sbFe2000UtilPvtCompensation(userDeviceHandle,uMmInstance);
}

/* This mimics entering the cmds in the BCM shell */
int32_t
sbFe2000DiagsInjectPciPacket(sbxDiagsInfo_t *pDiagsInfo)
{

    cmd_result_t val;
    args_t *pArgs = NULL;
    int pkt;
    char cmd_1[10];
    char cmd_2[10];
    int nPackets = pDiagsInfo->nInjectPackets;
    int len = pDiagsInfo->nInjectPacketsLen;
    int dbg = pDiagsInfo->debug_level;
    int unit = pDiagsInfo->unit;

    if ((pArgs = sal_alloc(sizeof (args_t), "pArgs")) == NULL) {
      printk("(%s) Error: out of memory\n",FUNCTION_NAME());
      return CMD_FAIL;
    }  

    if (dbg) {
	SB_LOG("%s: Injecting %d packets into PCI\n",__PRETTY_FUNCTION__,nPackets);
    }

    sal_memset(pArgs,0x0,sizeof(args_t));
    sprintf(cmd_1,"%d",nPackets);
    sprintf(cmd_2,"len=%d",len);
    if (dbg) {
	SB_LOG("sbx_tx %d len=%d\n",nPackets,len);
    }
    pArgs->a_cmd="SBX_TX";
    pArgs->a_argc=3;
    pArgs->a_arg=1;
    pArgs->a_argv[0] = "sbx_tx";
    pArgs->a_argv[1] = "1";
    pArgs->a_argv[2] = cmd_2;
    /* for now one at a time until sbx_tx fix */
    for(pkt=0;pkt<nPackets;pkt++) {
	val = cmd_soc_sbx_tx(unit,pArgs);
	if (val != CMD_OK) {
	    SB_ERROR("ERROR: sbx_tx returned %d\n",val);
	    sal_free(pArgs);
	    return (-1);
	}
    }
    sal_free(pArgs);
    return(0);
}

int32_t
sbFe2000DiagsReceivePciPacket(sbxDiagsInfo_t *pDiagsInfo)
{


    args_t *pArgs = NULL;
    uint32_t uData=0;
    int i;
    int nPkts;
    sbhandle hdl;
    int dbg;
    char *u_str;
    int unit;

    unit = pDiagsInfo->unit;
    nPkts = pDiagsInfo->nInjectPackets;
    hdl = SOC_SBX_CONTROL(unit)->sbhdl;
    dbg = pDiagsInfo->debug_level;
    u_str="";

    if ((pArgs = sal_alloc(sizeof (args_t), "pArgs")) == NULL) {
      printk("(%s) Error: out of memory\n",FUNCTION_NAME());
      return CMD_FAIL;
    }  

    sal_memset(pArgs,0x0,sizeof(args_t));

    if (SOC_IS_SBX_FE2000(unit)) {
      u_str = "FE2K";
    } else if (SOC_IS_SBX_QE2000(unit)) {
      u_str = "QE2K";
    }

    /* dump all the packets */
    for(i=0;i<nPkts;i++) {
	cmd_soc_sbx_rx(unit,pArgs);
    }
    /* check the pkt count */
    if (SOC_IS_SBX_FE2000(unit)) {
      uData = SAND_HAL_READ(hdl,CA,PC_RX_PKT_CNT);
    } else if (SOC_IS_SBX_QE2000(unit)) {
      uData = SAND_HAL_READ(hdl,KA,PC_RX_PKT_CNT);
    }

    if (uData != nPkts) {
	SB_ERROR("ERROR:%s_%d -> PCI TxExp:%d got TxAct:%d\n",u_str,unit,nPkts,uData);
    } else {
      if (dbg) {
	  SB_LOG("GOOD %s_%d -> PCI TxExp:%d got TxAct:%d\n",u_str,unit,nPkts,uData);
      }
    }
    sal_free(pArgs);
    return 0;
}

int
sbFe2000DiagsDumpExceptionPkt(sbxDiagsInfo_t *pDiagsInfo,
			      uint32_t qe_unit)
{


    args_t *pArgs = NULL;
    int i;

    if ((pArgs = sal_alloc(sizeof (args_t), "pArgs")) == NULL) {
      printk("(%s) Error: out of memory\n",FUNCTION_NAME());
      return CMD_FAIL;
    }  

    sal_memset(pArgs,0x0,sizeof(args_t));

    for(i=0;i<10;i++) {
	cmd_soc_sbx_rx(qe_unit,pArgs);
    }

    sal_free(pArgs);
    return 0;
}



int sbFe2000DiagsOverrideMmParam(sbxDiagsInfo_t *pDiagsInfo)
{

  uint32_t uMmInstance=0;
  uint32_t uMmProtectionScheme0=0;
  uint32_t uMmProtectionScheme1=0;
  sbFe2000InitParamsMm_t mm[SB_FE2000_NUM_MM_INSTANCES];
  uint32_t uPolicerRefreshEnable=0;
  uint32_t uData;
  sbhandle userDeviceHandle = pDiagsInfo->userDeviceHandle;
  int rv;

  sal_memset(&mm[0],0x0,sizeof(sbFe2000InitParamsMm_t));
  sal_memset(&mm[1],0x0,sizeof(sbFe2000InitParamsMm_t));

  for( uMmInstance = 0; uMmInstance < SB_FE2000_NUM_MM_INSTANCES; uMmInstance++ ){
    sbFe2000InitParamsMm_t *pInstance = &(mm[uMmInstance]);
    pInstance->ramprotection.NarrowPort0ProcessorInterface = SB_FE2000_MM_36BITS_NOPROTECTION;
    pInstance->ramprotection.NarrowPort1ProcessorInterface = SB_FE2000_MM_36BITS_NOPROTECTION;
    pInstance->ramprotection.WidePortRamProtectionScheme = SB_FE2000_MM_36BITS_NOPROTECTION;

    uMmProtectionScheme0 = SAND_HAL_READ_STRIDE( userDeviceHandle, CA, MM, uMmInstance, MM_PROT_SCHEME0 );
    uMmProtectionScheme0 = SAND_HAL_MOD_FIELD( CA, MM_PROT_SCHEME0, NP0_PI_SCHEME,  uMmProtectionScheme0,
					       pInstance->ramprotection.NarrowPort0ProcessorInterface );
    uMmProtectionScheme0 = SAND_HAL_MOD_FIELD( CA, MM_PROT_SCHEME0, NP1_PI_SCHEME,  uMmProtectionScheme0,
					       pInstance->ramprotection.NarrowPort1ProcessorInterface );

    SAND_HAL_WRITE_STRIDE( userDeviceHandle, CA, MM, uMmInstance, MM_PROT_SCHEME0, uMmProtectionScheme0 );
    uMmProtectionScheme1 = SAND_HAL_READ_STRIDE( userDeviceHandle, CA, MM, uMmInstance, MM_PROT_SCHEME1 );
    uMmProtectionScheme1 = SAND_HAL_MOD_FIELD( CA, MM_PROT_SCHEME1, WP_SCHEME,   uMmProtectionScheme1,
					       pInstance->ramprotection.WidePortRamProtectionScheme );
    SAND_HAL_WRITE_STRIDE( userDeviceHandle, CA, MM, uMmInstance, MM_PROT_SCHEME1, uMmProtectionScheme1 );
  }

  /* see if policer background refreshes are turned on */
  uData = SAND_HAL_READ( userDeviceHandle,CA,PM_CONFIG0);
  uPolicerRefreshEnable = SAND_HAL_GET_FIELD(CA,PM_CONFIG0,REFRESH_ENABLE,uData);
  if (uPolicerRefreshEnable) {
    uData = SAND_HAL_MOD_FIELD(CA,PM_CONFIG0,REFRESH_ENABLE,uData,0);
    SAND_HAL_WRITE(userDeviceHandle,CA,PM_CONFIG0,uData);
    pDiagsInfo->bPolicerRefreshWasOn = 1;
  }

  /* stop counter thread */
  rv = soc_sbx_counter_stop(pDiagsInfo->unit);
  if (SOC_FAILURE(rv)) {
    printk("Errors stopping counter thread, may interfere with test\n");
  }

  return 0;
}

int sbFe2000DiagsAddressCheck(sbxDiagsInfo_t *pDiagsInfo)
{

  unsigned long ulStartAddr = pDiagsInfo->start_addr;
  unsigned long ulEndAddr = pDiagsInfo->end_addr;
  unsigned long ulMaxAddr;
  uint32_t mem;

  mem = pDiagsInfo->e_mem_type;
  if (mem == SB_FE2000_NARROW_PORT_0) {
      ulMaxAddr = (1<<pDiagsInfo->np0_config.addr_width);
  } else if (mem == SB_FE2000_NARROW_PORT_1) {
      ulMaxAddr = (1<<pDiagsInfo->np1_config.addr_width);
  } else {
      ulMaxAddr = (1<<pDiagsInfo->wp_config.addr_width);
  }

  /* If an EndAddr was specified, use that otherwise assume maximum */
  if (!pDiagsInfo->bEndAddr) {
    ulEndAddr = ulMaxAddr;
  } else {
    ulEndAddr = pDiagsInfo->end_addr;
  }

  /* be sure address range to be tested is valid */
  if ( ulStartAddr > ulMaxAddr ||
       ulEndAddr   > ulMaxAddr ||
       ulEndAddr < ulStartAddr ) {
    SB_ERROR("ERROR:%s: BAD address specified for mem:%s start_addr=0x%08x, end_addr=0x%08x, max allowed=0x%08x\n",
	     __PRETTY_FUNCTION__,sbFe2000DiagsMemoryStr[mem],
	     ulStartAddr, ulEndAddr, ulMaxAddr);
    return (-1);
  }

  return(0);
}

/*
 *  LineRate External Memory Test. (Ported ZTCaladan90d033 test)
 */

int
sbFe2000LineRateMemTestRun(sbxDiagsInfo_t *p)
{
  uint32_t rv = 0;
  uint32_t err = 0;
  uint32_t timeout = 0;
  sbhandle hdl;
  int unit;
  int seed;
  uint32_t MemExists = p->uMemExistsMask;

  if (p->seed == -1) {
    /* seed = rand() & 0x3fff; */
    seed = 0x2aaa;
  } else {
    seed = p->seed;
  }

  seed = seed & 0x3fff;
  p->seed = seed;

  unit = p->unit;
  SB_LOG("%s: Testing with seed=%d\n",__PRETTY_FUNCTION__,seed);
  printk("Testing memories: ");

  if (GBITN(MemExists, 0)) printk("MM0.N0 ");
  if (GBITN(MemExists, 1)) printk("MM0.N1 ");
  if (GBITN(MemExists, 2)) printk("MM0.W ");
  if (GBITN(MemExists, 3)) printk("MM1.N0 ");
  if (GBITN(MemExists, 4)) printk("MM1.N1 ");
  if (GBITN(MemExists, 5))  printk("MM1.W ");
  printk("\n");

  hdl = SOC_SBX_CONTROL(unit)->sbhdl;

  rv = sbFe2000LineRateMemTestSetup(p);
  if (rv != SB_OK) {
    SB_LOG("ERROR sbFe2000LineRateMemTestSetup failed(%d)\n",rv);
    return (-1);
  }

  LrpEnable(hdl);
  LrpDisable(hdl);
  SetupCtlRegs(p);

  /* phase 0 */
  fe2kAsm2IntD__initDefaultPhase0(&fe2kdiag_phase0);

  Fe2000InitParams.lr.uEpochLengthInCycles = 913;
  rv = fe2000_bringup_lrp(unit,&Fe2000InitParams,0);
  if (rv != SB_OK) {
    SB_LOG("ERROR fe2000_bringup_lrp failed(%d)\n",rv);
    return (-1);
  }

  LrpEnable(hdl);
  timeout = WaitForPhase(hdl,1);
  rv = LrpDisable(hdl);
  if (rv != SB_OK) {
    SB_LOG("Error disabling lrp\n");
  }

  if (timeout) {
    SB_LOG("ERROR: Phase 1 Timed out, bailing\n");
    return (-1);
  }

  err += CheckErrors(hdl,seed,1);

  if (err) {
    SB_LOG("MEMORY ERRORS DETECTED = %d\n",err);
    SB_LOG("Skipping second phase, since phase one had errors\n");
    return (-1);
  }

  SetupCtlRegs(p);

  /* phase 1 */
  fe2kAsm2IntD__initDefaultPhase1(&fe2kdiag_phase1);

  /* Fe2000InitParams.lr.uEpochLengthInCycles = 718; */
  Fe2000InitParams.lr.uEpochLengthInCycles = 913;

  rv = fe2000_bringup_lrp(unit,&Fe2000InitParams,1);
  if (rv != 0) {
    SB_LOG("%s:%s ERROR sbFe2000InitLrDiag failed\n",__PRETTY_FUNCTION__,__LINE__);
  }

  LrpEnable(hdl);
  timeout = WaitForPhase(hdl,2);
  LrpDisable(hdl);
  if (timeout) {
    SB_LOG("ERROR: Phase 2 Timed out, bailing\n");
    return (-1);
  }

  err += CheckErrors(hdl,seed,2);

  if (err) {
    SB_LOG("MEMORY ERRORS DETECTED = %d\n",err);
    SB_LOG("Skipping third phase, since phase two had errors\n");
    return (-1);
  }

  SetupCtlRegs(p);

  /* phase 2 */
  fe2kAsm2IntD__initDefaultPhase2(&fe2kdiag_phase2);

  /*   Fe2000InitParams.lr.uEpochLengthInCycles = 480; */
  Fe2000InitParams.lr.uEpochLengthInCycles = 913;

  rv = fe2000_bringup_lrp(unit,&Fe2000InitParams,2);
  if (rv != 0) {
    SB_LOG("%s:%s ERROR sbFe2000InitLrDiag failed\n",__PRETTY_FUNCTION__,__LINE__);
    return (-1);
  }

  LrpEnable(hdl);
  timeout = WaitForPhase(hdl,3);
  LrpDisable(hdl);

  if (timeout) {
    SB_LOG("ERROR: Phase 3 Timed out, bailing\n");
    return (-1);
  }

  err += CheckErrors(hdl,seed,3);
  SB_LOG("MEMORY ERRORS DETECTED = %d\n",err);
  LrpEnable(hdl);

  if (err != 0 ) {
    return (-1);
  } else {
    return 0;
  }
}

int
sbFe2000LineRateMemTestSetup(sbxDiagsInfo_t *pDiagsInfo)
{
  uint32_t nMmu;
  uint32_t nPre, nPb;
  uint32_t ulReg;
  uint32_t nib,rr;
  sbhandle hdl;
  uint8_t bRevA0Chip = FALSE;
  uint32_t uMmRamConfiguration=0;
  uint32_t uMmClientConfig=0;
  uint32_t uMmProtectionScheme1=0;
  uint32_t uData=0;
  int rv;

  hdl = pDiagsInfo->userDeviceHandle;
  sbFe2000InitDefaultParams(&Fe2000InitParams);

  /* bms - Nov 02 2006 - this will set up all of the interfaces for the MM */
  /* bms - Nov 02 2006 - 2x narrow, 1x wide, 2x internal per MMU */
  for( nMmu = 0; nMmu < SB_FE2000_NUM_MM_INSTANCES; nMmu++ ) {
    Fe2000InitParams.mm[nMmu].bBringUp = TRUE;
    Fe2000InitParams.mm[nMmu].uNumberofInternalRam0Segments = 1;
    Fe2000InitParams.mm[nMmu].uNumberofInternalRam1Segments = 1;
    Fe2000InitParams.mm[nMmu].uNumberofNarrowPort0Segments  = 1;
    Fe2000InitParams.mm[nMmu].uNumberofNarrowPort1Segments  = 1;
    Fe2000InitParams.mm[nMmu].uNumberofWidePortSegments     = 1;

    /* bms - Dec 05 2006 - mem configs */
    /*     Fe2000InitParams.mm[nMmu].ramconfiguration.InternalRamConfiguration = SB_FE2000_MM_RAM0_16KBY36_AND_RAM1_16KBY36; */
    /*     Fe2000InitParams.mm[nMmu].ramconfiguration.NarrowPortRam0Configuration = SB_FE2000_MM_ONE_BY_18_RAM_CONNECTED; */
    /*     Fe2000InitParams.mm[nMmu].ramconfiguration.NarrowPortRam1Configuration = SB_FE2000_MM_ONE_BY_18_RAM_CONNECTED; */
    /*     Fe2000InitParams.mm[nMmu].ramconfiguration.WidePortRamConfiguration = SB_FE2000_MM_TWO_BY_18_RAMS_CONNECTED; */

#if 0
    uMmRamConfiguration = SAND_HAL_READ_STRIDE(hdl,CA,MM,nMmu,MM_RAM_CONFIG);
    uMmRamConfiguration = SAND_HAL_MOD_FIELD(CA,MM_RAM_CONFIG,WIDE_PORT_RAM_CONFIG,
					     uMmRamConfiguration,SB_FE2000_MM_TWO_BY_18_RAMS_CONNECTED);
    uMmRamConfiguration = SAND_HAL_MOD_FIELD(CA,MM_RAM_CONFIG,NARROW_PORT1_RAM_CONFIG,
					     uMmRamConfiguration,SB_FE2000_MM_ONE_BY_18_RAM_CONNECTED);
    uMmRamConfiguration = SAND_HAL_MOD_FIELD(CA,MM_RAM_CONFIG,NARROW_PORT0_RAM_CONFIG,
					     uMmRamConfiguration,SB_FE2000_MM_ONE_BY_18_RAM_CONNECTED);
    uMmRamConfiguration = SAND_HAL_MOD_FIELD(CA,MM_RAM_CONFIG,INTERNAL_RAM_CONFIG,
					     uMmRamConfiguration,SB_FE2000_MM_RAM0_16KBY36_AND_RAM1_16KBY36);

    SAND_HAL_WRITE_STRIDE(hdl,CA,MM,nMmu,MM_RAM_CONFIG,uMmRamConfiguration);
#else
    uMmRamConfiguration = uMmRamConfiguration; /* fool gcc */
#endif

    /* bms - Dec 05 2006 - the LRP will be connected to both Internal Rams and both narrow ports (as primary) and the wide ports */
    /*     Fe2000InitParams.mm[nMmu].ramconnections.Lrp0MemoryConnection = SB_FE2000_MM_LRP0_CONNECTED_TO_NARROWPORT_DDRII_PORT0; */
    /*     Fe2000InitParams.mm[nMmu].ramconnections.Lrp1MemoryConnection = SB_FE2000_MM_LRP1_CONNECTED_TO_NARROWPORT_DDRII_PORT1; */
    /*     Fe2000InitParams.mm[nMmu].ramconnections.Lrp2MemoryConnection = SB_FE2000_MM_LRP2_CONNECTED_TO_INTERNAL_SINGLE_RAM0; */
    /*     Fe2000InitParams.mm[nMmu].ramconnections.Lrp4MemoryConnection = SB_FE2000_MM_LRP4_CONNECTED_TO_WIDEPORT_DDRII_PORT; */

    /* bms - Dec 04 2006 - pmu */
    /*     Fe2000InitParams.mm[nMmu].ramconnections.PmuMemoryConnection  = SB_FE2000_MM_PMU_CONNECTED_TO_WIDEPORT_DDRII; */

    uMmClientConfig = SAND_HAL_READ_STRIDE(hdl,CA,MM,nMmu,MM_CLIENT_CONFIG);
    uMmClientConfig = SAND_HAL_MOD_FIELD(CA,MM_CLIENT_CONFIG, LRP0_MEMORY_CONFIG, uMmClientConfig,SB_FE2000_MM_LRP0_CONNECTED_TO_NARROWPORT_DDRII_PORT0);
    uMmClientConfig = SAND_HAL_MOD_FIELD(CA,MM_CLIENT_CONFIG, LRP1_MEMORY_CONFIG, uMmClientConfig,SB_FE2000_MM_LRP1_CONNECTED_TO_NARROWPORT_DDRII_PORT1);
    uMmClientConfig = SAND_HAL_MOD_FIELD(CA,MM_CLIENT_CONFIG, LRP2_MEMORY_CONFIG, uMmClientConfig,SB_FE2000_MM_LRP2_CONNECTED_TO_INTERNAL_SINGLE_RAM0);
    uMmClientConfig = SAND_HAL_MOD_FIELD(CA,MM_CLIENT_CONFIG, LRP4_MEMORY_CONFIG, uMmClientConfig,SB_FE2000_MM_LRP4_CONNECTED_TO_WIDEPORT_DDRII_PORT);
    uMmClientConfig = SAND_HAL_MOD_FIELD(CA,MM_CLIENT_CONFIG, PMU_MEMORY_CONFIG, uMmClientConfig,SB_FE2000_MM_PMU_CONNECTED_TO_WIDEPORT_DDRII);
    SAND_HAL_WRITE_STRIDE(hdl,CA,MM,nMmu, MM_CLIENT_CONFIG,uMmClientConfig);

  }

  /* bms - Jan 12 2007 - only connect MMU0 */
  /*   Fe2000InitParams.mm[0].ramconnections.Lrp3MemoryConnection = SB_FE2000_MM_LRP3_CONNECTED_TO_INTERNAL_SINGLE_RAM1; */
  uMmClientConfig = SAND_HAL_READ_STRIDE(hdl,CA,MM,0,MM_CLIENT_CONFIG);
  uMmClientConfig = SAND_HAL_MOD_FIELD(CA,MM_CLIENT_CONFIG, LRP3_MEMORY_CONFIG, uMmClientConfig,SB_FE2000_MM_LRP3_CONNECTED_TO_INTERNAL_SINGLE_RAM1);

  /* bms - Dec 04 2006 - the cmu */
  /*   Fe2000InitParams.mm[0].ramconnections.Cmu0MemoryConnection = SB_FE2000_MM_CMU0_CONNECTED_TO_NARROWPORT_DDRII_PORT0; */
  /*   Fe2000InitParams.mm[0].ramconnections.Cmu1MemoryConnection = SB_FE2000_MM_CMU1_CONNECTED_TO_NARROWPORT_DDRII_PORT1; */

  uMmClientConfig = SAND_HAL_MOD_FIELD(CA,MM_CLIENT_CONFIG, CMU0_MEMORY_CONFIG, uMmClientConfig,SB_FE2000_MM_CMU0_CONNECTED_TO_NARROWPORT_DDRII_PORT0);
  uMmClientConfig = SAND_HAL_MOD_FIELD(CA,MM_CLIENT_CONFIG, CMU1_MEMORY_CONFIG, uMmClientConfig,SB_FE2000_MM_CMU1_CONNECTED_TO_NARROWPORT_DDRII_PORT1);
  SAND_HAL_WRITE_STRIDE(hdl,CA,MM,0, MM_CLIENT_CONFIG,uMmClientConfig);


  /*   Fe2000InitParams.mm[1].ramconnections.Cmu0MemoryConnection = SB_FE2000_MM_CMU0_CONNECTED_TO_NARROWPORT_DDRII_PORT0; */
  /*   Fe2000InitParams.mm[1].ramconnections.Cmu1MemoryConnection = SB_FE2000_MM_CMU1_CONNECTED_TO_INTERNAL_SINGLE_RAM1; */

  uMmClientConfig = SAND_HAL_READ_STRIDE(hdl,CA,MM,1,MM_CLIENT_CONFIG);
  uMmClientConfig = SAND_HAL_MOD_FIELD(CA,MM_CLIENT_CONFIG, CMU0_MEMORY_CONFIG, uMmClientConfig,SB_FE2000_MM_CMU0_CONNECTED_TO_NARROWPORT_DDRII_PORT0);
  uMmClientConfig = SAND_HAL_MOD_FIELD(CA,MM_CLIENT_CONFIG, CMU1_MEMORY_CONFIG, uMmClientConfig,SB_FE2000_MM_CMU1_CONNECTED_TO_INTERNAL_SINGLE_RAM1);
  SAND_HAL_WRITE_STRIDE(hdl,CA,MM,1, MM_CLIENT_CONFIG,uMmClientConfig);


  /* Fix protection scheme for internal ram.  */
  /*   Fe2000InitParams.mm[1].ramprotection.InternalRam1ProtectionScheme = SB_FE2000_MM_35BITS_1BITPARITY ;   */
  uMmProtectionScheme1 = SAND_HAL_READ_STRIDE( hdl, CA, MM, 1, MM_PROT_SCHEME1 );
  uMmProtectionScheme1 = SAND_HAL_MOD_FIELD(CA,MM_PROT_SCHEME1,INT1_SCHEME,uMmProtectionScheme1,SB_FE2000_MM_35BITS_1BITPARITY);
  SAND_HAL_WRITE_STRIDE(hdl,CA,MM,1,MM_PROT_SCHEME1,uMmProtectionScheme1);

  /*  now that the data is preload into memory load the LRP ucode */
  /*   fe2kAsm2IntD__initDefault(&sbG2FeDefaultUcode); */
  /*   sbFe2000UcodeLoad(hdl,(uint8_t *)&sbG2FeDefaultUcode); */

  /* Set the free page fifo depths to max */
  for ( nPre = 0; nPre < SB_FE2000_PR_NUM_PRE; nPre++ ) {
    for ( nPb = 0; nPb < SB_FE2000_PB_NUM_PACKET_BUFFERS; nPb++ ) {
      Fe2000InitParams.pr.uPbPrefetchPages[nPre][nPb] = 14;
    }
  }

  SAND_HAL_RMW_FIELD(hdl, CA, PR_PRE0_CONFIG, PB1_PREFETCH_PAGES,14);
  SAND_HAL_RMW_FIELD(hdl, CA, PR_PRE0_CONFIG, PB0_PREFETCH_PAGES,14);
  SAND_HAL_RMW_FIELD(hdl, CA, PR_PRE1_CONFIG, PB1_PREFETCH_PAGES,14);
  SAND_HAL_RMW_FIELD(hdl, CA, PR_PRE1_CONFIG, PB0_PREFETCH_PAGES,14);

  /* dwl - Jun 17 2007 - default for dll phase is 0 */
  uData = SAND_HAL_READ(hdl, CA, PC_REVISION);
  bRevA0Chip = SAND_HAL_GET_FIELD(CA, PC_REVISION, REVISION, uData)==0;
  nib = bRevA0Chip ? 0 : 1;

  /* adjust for user input */
  if(pDiagsInfo->dllphase != -1) {
      nib = pDiagsInfo->dllphase & 7;
  }

  rr = (nib << 16) | (nib << 8) | nib;

  SAND_HAL_WRITE(hdl, CA, MM0_DLL_DEBUG, rr);
  SAND_HAL_WRITE(hdl, CA, MM1_DLL_DEBUG, rr);

  ulReg = SAND_HAL_READ(hdl, CA, MM0_RAM_CONFIG);
  ulReg = SAND_HAL_MOD_FIELD(CA, MM_RAM_CONFIG, NARROW_PORT1_USE_DELAYED_DVLD, ulReg, 1);
  ulReg = SAND_HAL_MOD_FIELD(CA, MM_RAM_CONFIG, NARROW_PORT0_USE_DELAYED_DVLD, ulReg, 1);
  ulReg = SAND_HAL_MOD_FIELD(CA, MM_RAM_CONFIG, WIDE_PORT_USE_DELAYED_DVLD, ulReg, 1);
  SAND_HAL_WRITE(hdl, CA, MM0_RAM_CONFIG, ulReg);

  ulReg = SAND_HAL_READ(hdl, CA, MM1_RAM_CONFIG);
  ulReg = SAND_HAL_MOD_FIELD(CA, MM_RAM_CONFIG, NARROW_PORT1_USE_DELAYED_DVLD, ulReg, 1);
  ulReg = SAND_HAL_MOD_FIELD(CA, MM_RAM_CONFIG, NARROW_PORT0_USE_DELAYED_DVLD, ulReg, 1);
  ulReg = SAND_HAL_MOD_FIELD(CA, MM_RAM_CONFIG, WIDE_PORT_USE_DELAYED_DVLD, ulReg, 1);
  SAND_HAL_WRITE(hdl, CA, MM1_RAM_CONFIG, ulReg);

  /* stop counter thread */
  rv = soc_sbx_counter_stop(pDiagsInfo->unit);
  if (SOC_FAILURE(rv)) {
    printk("Errors stopping counter thread, may interfere with test\n");
  }

  /* disable the policer */
  ulReg = SAND_HAL_READ(hdl, CA, PC_CORE_RESET);
  ulReg = SAND_HAL_MOD_FIELD(CA, PC_CORE_RESET, PM_CORE_RESET, ulReg, 1);
  SAND_HAL_WRITE(hdl, CA, PC_CORE_RESET, ulReg);

  return 0;
}

void
SetupCtlRegs(sbxDiagsInfo_t *pDiagsInfo)
{

  sbZfCaDiagUcodeShared_t zfSharedReg;
  sbZfCaDiagUcodeCtl_t zfCtlReg;
  uint32_t ulCtlReg = 0;
  uint8_t ulCtlRegBytes[4] = {0};
  uint32_t uLrp = 0;
  uint32_t ulSharedReg = 0;
  uint8_t ulSharedRegBytes[4] = {0};
  uint32_t ulEMask = pDiagsInfo->uMemExistsMask;
  int32_t seed = pDiagsInfo->seed;
  sbhandle hdl = pDiagsInfo->userDeviceHandle;

  sbZfCaDiagUcodeShared_InitInstance(&zfSharedReg);
  sbZfCaDiagUcodeCtl_InitInstance(&zfCtlReg);

  zfCtlReg.m_ulStatus = 0;
  zfCtlReg.m_ulErrInject0 = 0;
  zfCtlReg.m_ulErrInject1 = 0;
  zfCtlReg.m_ulErrInd = 0;
  zfCtlReg.m_ulUcLoaded = 0;
  zfCtlReg.m_ulLrpState = 0;
  zfCtlReg.m_ulMemExstMm0N0 = GBITN(ulEMask, 0);
  zfCtlReg.m_ulMemExstMm0N1 = GBITN(ulEMask, 1);
  zfCtlReg.m_ulMemExstMm0W  = GBITN(ulEMask, 2);
  zfCtlReg.m_ulMemExstMm1N0 = GBITN(ulEMask, 3);
  zfCtlReg.m_ulMemExstMm1N1 = GBITN(ulEMask, 4);
  zfCtlReg.m_ulMemExstMm1W  = GBITN(ulEMask, 5);

  /* zfCtlReg.CalcBuffer((UCHAR*) &ulCtlReg, 4); */
  /* read data from zframe --> ulCtlRegBytes buffer */
  sbZfCaDiagUcodeCtl_Pack(&zfCtlReg,&ulCtlRegBytes[0],1);
  CaSetDataWord(&ulCtlReg,&ulCtlRegBytes[0]);

  printk("ulCtlReg=0x%x\n",ulCtlReg);
  for(uLrp = 0; uLrp < 48; uLrp++) {
    WrI(hdl,0, uLrp, ulCtlReg);

    /* clear out error information */
    WrI(hdl,0, uLrp + ERR_A, 0);
    WrI(hdl,0, uLrp + ERR_D0, 0);
    WrI(hdl,0, uLrp + ERR_D1, 0);
  }

  /* define ram sizes and seed */
  zfSharedReg.m_ulMemSizeMm0N0 = MemSizeEnc(pDiagsInfo->np0_config.addr_width);
  zfSharedReg.m_ulMemSizeMm0N1 = MemSizeEnc(pDiagsInfo->np1_config.addr_width);
  zfSharedReg.m_ulMemSizeMm0W  = MemSizeEnc(pDiagsInfo->wp_config.addr_width);
  zfSharedReg.m_ulMemSizeMm1N0 = MemSizeEnc(pDiagsInfo->np0_config.addr_width);
  zfSharedReg.m_ulMemSizeMm1N1 = MemSizeEnc(pDiagsInfo->np1_config.addr_width);
  zfSharedReg.m_ulMemSizeMm1W  = MemSizeEnc(pDiagsInfo->wp_config.addr_width);
  zfSharedReg.m_ulRandomSeed   = seed;

  /*   zfSharedReg.CalcBuffer((UCHAR*) &ulSharedReg, 4); */
  /*   ulSharedReg = myswap(ulSharedReg); */
  sbZfCaDiagUcodeShared_Pack(&zfSharedReg,&ulSharedRegBytes[0],1);
  CaSetDataWord(&ulSharedReg,&ulSharedRegBytes[0]);

  /* write out shared config entry (address 0x0 in int mem 1) */
  printk("ulSharedReg = 0x%x\n",ulSharedReg);
  WrI(hdl,1, 0, ulSharedReg);

}

/* write internal */
void
WrI(sbhandle hdl, uint32_t mem, uint32_t offs, uint32_t data)
{

  switch (mem) {
  case 0:
    {
      sbZfFe2000MmInternal0MemoryEntry_t zData;
      sbZfFe2000MmInternal0MemoryEntry_InitInstance(&zData);
      zData.m_uData0 = data;
      CaMm0Internal0MemoryWrite(hdl,offs, &zData);
    }
    break;
  case 1:
    {
      sbZfFe2000MmInternal1MemoryEntry_t zData;
      sbZfFe2000MmInternal1MemoryEntry_InitInstance(&zData);
      zData.m_uData0 = data;
      CaMm0Internal1MemoryWrite(hdl,offs, &zData);
    }
    break;
  case 2:
    {
      sbZfFe2000MmInternal0MemoryEntry_t zData;
      sbZfFe2000MmInternal0MemoryEntry_InitInstance(&zData);
      zData.m_uData0 = data;
      CaMm1Internal0MemoryWrite(hdl,offs, &zData);
    }
    break;
  case 3:
    {
      sbZfFe2000MmInternal1MemoryEntry_t zData;
      sbZfFe2000MmInternal1MemoryEntry_InitInstance(&zData);
      zData.m_uData0 = data;
      CaMm1Internal1MemoryWrite(hdl,offs, &zData);
    }
    break;
  default:
    SB_ASSERT(0);
    break;
  }
}

/* read internal */
uint32_t
RdI(sbhandle hdl,uint32_t mem, uint32_t offs)
{
  switch (mem) {
  case 0:
    {
      sbZfFe2000MmInternal0MemoryEntry_t zData;
      CaMm0Internal0MemoryRead(hdl,offs, &zData);
      return zData.m_uData0;
    }
    break;
  case 1:
    {
      sbZfFe2000MmInternal1MemoryEntry_t zData;
      CaMm0Internal1MemoryRead(hdl,offs, &zData);
      return zData.m_uData0;
    }
    break;
  case 2:
    {
      sbZfFe2000MmInternal0MemoryEntry_t zData;
      CaMm1Internal0MemoryRead(hdl,offs, &zData);
      return zData.m_uData0;
    }
    break;
  case 3:
    {
      sbZfFe2000MmInternal1MemoryEntry_t zData;
      CaMm1Internal1MemoryRead(hdl,offs, &zData);
      return zData.m_uData0;
    }
    break;
  default:
    SB_ASSERT(0);
    break;
  }

  /* unreachable */
  SB_ASSERT(0);
  return 0;
}

/* the memory size is endcoded as 16 - x for ucode
 * so a 22-bit memory would be encoded as 5 */
uint32_t
MemSizeEnc(uint32_t bits)
{
  SB_ASSERT(bits >= RAM_SIZE_BASE);
  SB_ASSERT(bits - RAM_SIZE_BASE <= 0x7);
  return bits - RAM_SIZE_BASE;
}

void
LrpEnable(sbhandle hdl)
{
  uint32_t ulReg;
  ulReg = SAND_HAL_READ(hdl, CA, LR_CONFIG0);
  ulReg = SAND_HAL_MOD_FIELD( CA, LR_CONFIG0, ENABLE, ulReg, 1);
  ulReg = SAND_HAL_MOD_FIELD( CA, LR_CONFIG0, ONLINE, ulReg, 0x3f);
  ulReg = SAND_HAL_MOD_FIELD( CA, LR_CONFIG0, SHUTDOWN, ulReg, 0);
  ulReg = SAND_HAL_MOD_FIELD(CA, LR_CONFIG0, LOAD_ENABLE, ulReg, 0);
  SAND_HAL_WRITE(hdl, CA, LR_CONFIG0, ulReg);
}

uint32_t
LrpDisable(sbhandle hdl)
{

  /* updated way to disable */
  uint32_t timeout;
  uint32_t uData;
  uint32_t uPendingRequests;
  uint32_t uLrpOffline;

  SB_LOG("Disabling LRP\n");

  /* 1. set lr_config2.rq_frames_per_context to zero (this stops LRP from requesting frames) */
  SAND_HAL_RMW_FIELD(hdl,CA,LR_CONFIG2,RQ_FRAMES_PER_CONTEXT,0);
  /* 2. Wait for pending requests to go to zero */
  timeout = 300000;
  while(timeout) {
    uData = SAND_HAL_READ(hdl,CA,LR_STATUS);
    uPendingRequests = SAND_HAL_GET_FIELD(CA,LR_STATUS,PENDING_REQS,uData);
    if (uPendingRequests == 0)
      break;
    thin_delay(800);
    timeout--;
  }

  if (timeout == 0) {
    SB_LOG("WARNING Timed out waiting for lrp pending requests to finish\n");
  }

  /* 3. Shut down context loading */
  SAND_HAL_RMW_FIELD(hdl,CA,LR_CONFIG1,FRAMES_PER_CONTEXT,0);

  /* 4. Request shutdown */
  SAND_HAL_RMW_FIELD(hdl,CA,LR_CONFIG0,SHUTDOWN,1);

  /* 5. Wait for LRP to go offline */
  timeout = 20000000;
  while(timeout) {
    uData = SAND_HAL_READ(hdl,CA,LR_EVENT);
    uLrpOffline = SAND_HAL_GET_FIELD(CA,LR_EVENT,OFFLINE,uData);
    if (uLrpOffline == 1)
      break;
    thin_delay(500);
    timeout--;
  }

  if (timeout == 0) {
    SB_LOG("ERROR Timed out waiting for lrp to go offline\n");
    return (-1);
  }

  /* 6. Clear offline status */
  SAND_HAL_RMW_FIELD(hdl,CA,LR_EVENT,OFFLINE,1);

  /* 7. Reset Control/Config */
  SAND_HAL_WRITE(hdl,CA,LR_EPOCH_CNTRL,0);

  /* restore reset state to lr_config0/1/2 */
  SAND_HAL_WRITE(hdl,CA,LR_CONFIG0,0);

  uData = 0;
  uData = SAND_HAL_MOD_FIELD(CA,LR_CONFIG1,DONE_DELAY,uData,0);
  uData = SAND_HAL_MOD_FIELD(CA,LR_CONFIG1,LOAD_COUNT,uData,1);
  uData = SAND_HAL_MOD_FIELD(CA,LR_CONFIG1,WORDS1,uData,12);
  uData = SAND_HAL_MOD_FIELD(CA,LR_CONFIG1,WORDS0,uData,12);
  uData = SAND_HAL_MOD_FIELD(CA,LR_CONFIG1,PPE_PACE,uData,2);
  uData = SAND_HAL_MOD_FIELD(CA,LR_CONFIG1,FRAMES_PER_CONTEXT,uData,48);
  SAND_HAL_WRITE(hdl,CA,LR_CONFIG1,uData);


  uData = 0;
  uData = SAND_HAL_MOD_FIELD(CA,LR_CONFIG2,RQ_THRESHOLD,uData,85);
  uData = SAND_HAL_MOD_FIELD(CA,LR_CONFIG2,RQ_FRAMES_PER_CONTEXT,uData,48);
  SAND_HAL_WRITE(hdl,CA,LR_CONFIG2,uData);
  return 0;

}

uint32_t
WaitForPhase(sbhandle hdl,uint32_t phase)
{

  uint32_t ulCtlReg, ulTimeout = 0, ulDone, ulOffs, ulStage;
  uint32_t i;

  while (1) {
    ulDone = 1;
    for (i = 0; i < MAX_PES; i++) {
      ulCtlReg = RdI(hdl,0, 0 + i);
      if ((ulCtlReg & 0x1) == 0)
	ulDone = 0;
    }
    ulOffs = 48 * (ulCtlReg >> 13);
    /* SB_LOG("Testing is at offset 0x%08x, ulCtlReg=0x%08x\n",ulOffs,ulCtlReg); */
    if (ulDone) {
      ulStage = (ulCtlReg >> 5) & 0x3;
      SB_LOG( "Finished (%d of 3) stages\n",ulStage);
      if (ulStage != phase)
	SB_LOG("ERROR: Register failed to show correct phase expt %d actual %d\n", phase, ulStage);
      return 0;
    }
    if (ulTimeout++ > 100000) {
      SB_LOG("ERROR: Timed out waiting for test to finish\n");
      return 1;
    }
  }
}

uint32_t
CheckErrors(sbhandle hdl,int32_t seed,uint32_t phase)
{
  uint32_t err = 0;
  uint32_t ulCtlReg, i;
  uint32_t ulReg,ulAddr,ulPort,ulD0,ulD1;
  uint32_t ulE0,ulE1;

  /* check for errors */
  for ( i = 0; i < MAX_PES; i++) {
    ulCtlReg = RdI(hdl,0, 0 + i);
    if (ulCtlReg & 0x8) {
      err = 1;
      /* read {port, addr, data-lo, data-hi} from memory */
      ulReg = RdI(hdl,0, ERR_A + i);
      ulAddr = ulReg & 0xfffFFFF;
      ulPort = ulReg >> 28;
      ulD0 = RdI(hdl,0, ERR_D0 + i);
      ulD1 = RdI(hdl,0, ERR_D1 + i);

      /* calculated expected */
      ulE0 = HashData(seed, ulAddr, 0);
      ulE1 = HashData(seed, ulAddr, 1);

      /* inverted in last stage */
      if (phase > 2) {
	ulE0 = ~ulE0;
	ulE1 = ~ulE1;
      }

      /* 32 bits, upper word is garbage */
      if (ulPort < 4) {
	ulE1 = 0;
	ulD1 = 0;
      }

      SB_LOG("ERROR: PE %d encountered an error\n", i);
      SB_LOG("ERROR: Addr = 0x%08x\n", ulAddr);
      SB_LOG("ERROR: Memory = %s \n", MemStr[ulPort]);
      if (ulPort < 4) {
	SB_LOG("ERROR: read 0x%08x expt 0x%08x xor 0x%08x\n",
	       ulD0, ulE0, ulD0 ^ ulE0);
      } else {
	SB_LOG("ERROR: read 0x%08x%08x expt 0x%08x%08x xor 0x%08x%08x\n",
	       ulD1, ulD0, ulE1, ulE0, ulD1 ^ ulE1, ulD0 ^ ulE0);
      }
    }
  }
  return err;
}

static uint32_t bytetable[] = {
  0x00000000U, 0x04c11db7U, 0x09823b6eU, 0x0d4326d9U,
  0x130476dcU, 0x17c56b6bU, 0x1a864db2U, 0x1e475005U,
  0x2608edb8U, 0x22c9f00fU, 0x2f8ad6d6U, 0x2b4bcb61U,
  0x350c9b64U, 0x31cd86d3U, 0x3c8ea00aU, 0x384fbdbdU,
  0x4c11db70U, 0x48d0c6c7U, 0x4593e01eU, 0x4152fda9U,
  0x5f15adacU, 0x5bd4b01bU, 0x569796c2U, 0x52568b75U,
  0x6a1936c8U, 0x6ed82b7fU, 0x639b0da6U, 0x675a1011U,
  0x791d4014U, 0x7ddc5da3U, 0x709f7b7aU, 0x745e66cdU,
  0x9823b6e0U, 0x9ce2ab57U, 0x91a18d8eU, 0x95609039U,
  0x8b27c03cU, 0x8fe6dd8bU, 0x82a5fb52U, 0x8664e6e5U,
  0xbe2b5b58U, 0xbaea46efU, 0xb7a96036U, 0xb3687d81U,
  0xad2f2d84U, 0xa9ee3033U, 0xa4ad16eaU, 0xa06c0b5dU,
  0xd4326d90U, 0xd0f37027U, 0xddb056feU, 0xd9714b49U,
  0xc7361b4cU, 0xc3f706fbU, 0xceb42022U, 0xca753d95U,
  0xf23a8028U, 0xf6fb9d9fU, 0xfbb8bb46U, 0xff79a6f1U,
  0xe13ef6f4U, 0xe5ffeb43U, 0xe8bccd9aU, 0xec7dd02dU,
  0x34867077U, 0x30476dc0U, 0x3d044b19U, 0x39c556aeU,
  0x278206abU, 0x23431b1cU, 0x2e003dc5U, 0x2ac12072U,
  0x128e9dcfU, 0x164f8078U, 0x1b0ca6a1U, 0x1fcdbb16U,
  0x018aeb13U, 0x054bf6a4U, 0x0808d07dU, 0x0cc9cdcaU,
  0x7897ab07U, 0x7c56b6b0U, 0x71159069U, 0x75d48ddeU,
  0x6b93dddbU, 0x6f52c06cU, 0x6211e6b5U, 0x66d0fb02U,
  0x5e9f46bfU, 0x5a5e5b08U, 0x571d7dd1U, 0x53dc6066U,
  0x4d9b3063U, 0x495a2dd4U, 0x44190b0dU, 0x40d816baU,
  0xaca5c697U, 0xa864db20U, 0xa527fdf9U, 0xa1e6e04eU,
  0xbfa1b04bU, 0xbb60adfcU, 0xb6238b25U, 0xb2e29692U,
  0x8aad2b2fU, 0x8e6c3698U, 0x832f1041U, 0x87ee0df6U,
  0x99a95df3U, 0x9d684044U, 0x902b669dU, 0x94ea7b2aU,
  0xe0b41de7U, 0xe4750050U, 0xe9362689U, 0xedf73b3eU,
  0xf3b06b3bU, 0xf771768cU, 0xfa325055U, 0xfef34de2U,
  0xc6bcf05fU, 0xc27dede8U, 0xcf3ecb31U, 0xcbffd686U,
  0xd5b88683U, 0xd1799b34U, 0xdc3abdedU, 0xd8fba05aU,
  0x690ce0eeU, 0x6dcdfd59U, 0x608edb80U, 0x644fc637U,
  0x7a089632U, 0x7ec98b85U, 0x738aad5cU, 0x774bb0ebU,
  0x4f040d56U, 0x4bc510e1U, 0x46863638U, 0x42472b8fU,
  0x5c007b8aU, 0x58c1663dU, 0x558240e4U, 0x51435d53U,
  0x251d3b9eU, 0x21dc2629U, 0x2c9f00f0U, 0x285e1d47U,
  0x36194d42U, 0x32d850f5U, 0x3f9b762cU, 0x3b5a6b9bU,
  0x0315d626U, 0x07d4cb91U, 0x0a97ed48U, 0x0e56f0ffU,
  0x1011a0faU, 0x14d0bd4dU, 0x19939b94U, 0x1d528623U,
  0xf12f560eU, 0xf5ee4bb9U, 0xf8ad6d60U, 0xfc6c70d7U,
  0xe22b20d2U, 0xe6ea3d65U, 0xeba91bbcU, 0xef68060bU,
  0xd727bbb6U, 0xd3e6a601U, 0xdea580d8U, 0xda649d6fU,
  0xc423cd6aU, 0xc0e2d0ddU, 0xcda1f604U, 0xc960ebb3U,
  0xbd3e8d7eU, 0xb9ff90c9U, 0xb4bcb610U, 0xb07daba7U,
  0xae3afba2U, 0xaafbe615U, 0xa7b8c0ccU, 0xa379dd7bU,
  0x9b3660c6U, 0x9ff77d71U, 0x92b45ba8U, 0x9675461fU,
  0x8832161aU, 0x8cf30badU, 0x81b02d74U, 0x857130c3U,
  0x5d8a9099U, 0x594b8d2eU, 0x5408abf7U, 0x50c9b640U,
  0x4e8ee645U, 0x4a4ffbf2U, 0x470cdd2bU, 0x43cdc09cU,
  0x7b827d21U, 0x7f436096U, 0x7200464fU, 0x76c15bf8U,
  0x68860bfdU, 0x6c47164aU, 0x61043093U, 0x65c52d24U,
  0x119b4be9U, 0x155a565eU, 0x18197087U, 0x1cd86d30U,
  0x029f3d35U, 0x065e2082U, 0x0b1d065bU, 0x0fdc1becU,
  0x3793a651U, 0x3352bbe6U, 0x3e119d3fU, 0x3ad08088U,
  0x2497d08dU, 0x2056cd3aU, 0x2d15ebe3U, 0x29d4f654U,
  0xc5a92679U, 0xc1683bceU, 0xcc2b1d17U, 0xc8ea00a0U,
  0xd6ad50a5U, 0xd26c4d12U, 0xdf2f6bcbU, 0xdbee767cU,
  0xe3a1cbc1U, 0xe760d676U, 0xea23f0afU, 0xeee2ed18U,
  0xf0a5bd1dU, 0xf464a0aaU, 0xf9278673U, 0xfde69bc4U,
  0x89b8fd09U, 0x8d79e0beU, 0x803ac667U, 0x84fbdbd0U,
  0x9abc8bd5U, 0x9e7d9662U, 0x933eb0bbU, 0x97ffad0cU,
  0xafb010b1U, 0xab710d06U, 0xa6322bdfU, 0xa2f33668U,
  0xbcb4666dU, 0xb8757bdaU, 0xb5365d03U, 0xb1f740b4U,
};


/* -------------------------------------------------------------------
 * Perform a hash on a 32bit value and return the result. The hash
 * algorithm is designed to operate identically to the microcode
 * instruction in the LRPs
 * -------------------------------------------------------------------*/

uint32_t
fe2000_crc32(uint32_t x)
{
  uint32_t i;

  for (i = 0; i < 4; i++)
    x = (x << 8) ^ bytetable[x >> 24];
  return x;
}

uint32_t
HashData(int32_t seed, uint32_t ulOffs, uint32_t Inv)
{
  int32_t seedEx;

  SB_ASSERT(seed < (0x1 << 14));

  seedEx = (seed << 28) | (seed << 14) | seed;

  if (Inv)
    seedEx = ~seedEx;

  return fe2000_crc32(ulOffs + seedEx);
}


int
fe2000_bringup_lrp(int unit, const sbFe2000InitParams_t *pInitParams,
		   int phase)
{

  UINT uConfig0;
  UINT uConfig1;
  UINT uStreamIndex;
  UINT uStreamOnline = 0;
  UINT nRequestThreshold;
  sbhandle hdl;
  int rv;

  hdl = SOC_SBX_CONTROL(unit)->sbhdl;

  /* dwl - May 25 2007 - sdk already bringp the LRP. Put all the registers back to the
   *                   - reset state before getting the LR out of reset */
  /* dwl - May 27 2007 - use CZRegSet to iterate all LR registers. Take care of read-on-clear
   *                   - and write 1 to clear fields with extra read & writes.*/

  /* reset LR registers to power-up default values */
  SB_LOG("Resetting LR registers.. \n");
  if (0 != (rv = fe2000_set_lrp_default_regs(unit)))
    return (rv);

  /* set the request threshold */
  nRequestThreshold =(1024 / BCM_SBX_MAX(pInitParams->lr.uWords0,
                                         pInitParams->lr.uWords1));
  SAND_HAL_RMW_FIELD(hdl, CA, LR_CONFIG2, RQ_THRESHOLD, nRequestThreshold);

  SB_LOG("Bringing up LRP\n");
  SAND_HAL_RMW_FIELD(hdl, CA, PC_CORE_RESET, LR_CORE_RESET, 0x0);

  /*   if ( m_bLoadLrUcode == TRUE ) LoadLrUcode(); */
  SB_LOG("Loading Phase %d microcode.. ",phase);
  switch(phase) {
  case 0:
    rv = sbFe2000UcodeLoad(hdl,(uint8_t *)&fe2kdiag_phase0);
    break;
  case 1:
    rv = sbFe2000UcodeLoad(hdl,(uint8_t *)&fe2kdiag_phase1);
    break;
  case 2:
    rv = sbFe2000UcodeLoad(hdl,(uint8_t *)&fe2kdiag_phase2);
    break;
  default:
    return (-1);
  }
  if (rv != SB_OK) {
    SB_LOG("\n ERROR sbFe2000UcodeLoad failed(%d)\n",rv);
    return (-1);
  } else {
    SB_LOG("OK\n");
  }

  uConfig0 = SAND_HAL_READ(hdl, CA, LR_CONFIG0);
  uConfig0 = SAND_HAL_MOD_FIELD( CA, LR_CONFIG0, ENABLE, uConfig0, TRUE );
  /* A quote from Nick as to why the hw adds +11 to the programmed epoch!
     "it takes time to propogate "doneness" from each PE back to the instruction controller and
     then the instruction controller has to recharge its instruction pipeline to get back to the
     beginning of the epoch..." */
  uConfig0 = SAND_HAL_MOD_FIELD( CA, LR_CONFIG0, EPOCH, uConfig0, (pInitParams->lr.uEpochLengthInCycles - 11) );
  uConfig0 = SAND_HAL_MOD_FIELD( CA, LR_CONFIG0, CONTEXTS, uConfig0, pInitParams->lr.uNumberOfContexts );
  uConfig0 = SAND_HAL_MOD_FIELD( CA, LR_CONFIG0, PAIRED, uConfig0, pInitParams->lr.bPairedMode);

  if( pInitParams->lr.bLrpBypass == TRUE ) {
    uConfig0 = SAND_HAL_MOD_FIELD( CA, LR_CONFIG0, BYPASS, uConfig0, 1 );
  } else {
    uConfig0 = SAND_HAL_MOD_FIELD( CA, LR_CONFIG0, BYPASS, uConfig0, 0 );
    if ( pInitParams->lr.bHeaderProcessingMode == TRUE ) {
      uConfig0 = SAND_HAL_MOD_FIELD( CA, LR_CONFIG0, LOAD_ENABLE, uConfig0, 1 );
    }
    for( uStreamIndex = 0; uStreamIndex < SB_FE2000_LR_NUMBER_OF_STREAMS; uStreamIndex++ ) {
      if ( pInitParams->lr.bStreamOnline[uStreamIndex] == TRUE ) {
        uStreamOnline |= (1 << uStreamIndex);
      }
    }
    /* rgf - Aug 09 2006 - This may need to be broken into Step0/Step1 bringup
       rgf - Aug 09 2006 - depending on how long it takes streams to actually
       rgf - Aug 09 2006 - come online. */
    uConfig0 = SAND_HAL_MOD_FIELD( CA, LR_CONFIG0, ONLINE, uConfig0, uStreamOnline );
#ifdef UPDATE
#define _SB_UPDATE_SAVE UPDATE
#undef UPDATE
#endif
    uConfig0 = SAND_HAL_MOD_FIELD( CA, LR_CONFIG0, UPDATE, uConfig0, 1 );
#ifdef _SB_UPDATE_SAVE
#define UPDATE _SB_UPDATE_SAVE
#endif
  }

  /* dgm - Feb 12 2007 - Needed for lrp timer utility (OAM) -- value taken from register description */
  SAND_HAL_WRITE( hdl, CA, LU_CLOCKS_PER_US, 17179869);

  uConfig1 = SAND_HAL_READ(hdl, CA, LR_CONFIG1);
  uConfig1 = SAND_HAL_MOD_FIELD( CA, LR_CONFIG1, PPE_PACE, uConfig1,(pInitParams->lr.uPpeRequestPaceInCycles-1) );
  uConfig1 = SAND_HAL_MOD_FIELD( CA, LR_CONFIG1, FRAMES_PER_CONTEXT, uConfig1, pInitParams->lr.uFramesPerContext );
  uConfig1 = SAND_HAL_MOD_FIELD( CA, LR_CONFIG1, LOAD_COUNT, uConfig1, pInitParams->lr.uLoadCount );
  uConfig1 = SAND_HAL_MOD_FIELD( CA, LR_CONFIG1, WORDS0, uConfig1, pInitParams->lr.uWords0 );
  uConfig1 = SAND_HAL_MOD_FIELD( CA, LR_CONFIG1, WORDS1, uConfig1, pInitParams->lr.uWords1 );

  SAND_HAL_WRITE(hdl, CA, LR_CONFIG1, uConfig1);
  SAND_HAL_WRITE(hdl, CA, LR_CONFIG0, uConfig0);

  return rv;
}

/* set the lrp registers to default values */
int
fe2000_set_lrp_default_regs(int unit)
{

  sbhandle hdl = NULL;
  soc_sbx_chip_info_t *chip_info = NULL;
  soc_sbx_reg_info_t *reg_info = NULL;
  soc_sbx_reg_info_list_t *Fe2k_lrp_regs_l = NULL;
  const char * lr_regs = "lr";
  uint32_t i;
  uint32_t regval = 0;
  uint32_t tmp = 0;
  int idx;
  int j;
  uint32_t rv = 0;

  hdl = SOC_SBX_CONTROL(unit)->sbhdl;
  if (!Fe2k_lrp_regs_l) {
    Fe2k_lrp_regs_l = sal_alloc(sizeof (soc_sbx_reg_info_list_t), "reginfo_l");
    if (!Fe2k_lrp_regs_l) {
      SB_LOG("%s ERROR:  Out of Memory \n",__PRETTY_FUNCTION__);
      return CMD_FAIL;
    }
  }

  Fe2k_lrp_regs_l->count = 0;
  if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info,1) != CMD_OK) {
    printk("ERROR: Register info unknown for unit %d \n", unit);
    sal_free(Fe2k_lrp_regs_l);
    return (-1);
  }

  /* find the lr registers */
  for(idx = 0; idx < chip_info->nregs; idx++) {
    reg_info = chip_info->regs[idx];
    if (strstr(reg_info->name,lr_regs)) {
      Fe2k_lrp_regs_l->idx[Fe2k_lrp_regs_l->count++] = idx;
    }
  }

  /* get and set their default values */
  for(i = 0; i < Fe2k_lrp_regs_l->count; i++) {
    idx = Fe2k_lrp_regs_l->idx[i];
    if (idx >= 0) {
      reg_info = chip_info->regs[idx];
      for(j=0;j<reg_info->nfields;j++) {
	regval |= (reg_info->fields[j]->default_val << reg_info->fields[j]->shift);
      }

      (void)sbx_diag_read_reg(unit,reg_info,&tmp); /* clears any clear-on-read fields */
      (void)sbx_diag_write_reg(unit,reg_info,tmp); /* clears any write 1 to clear fields */
      /* write the default value */
      /* SB_LOG("Setting Reg:%s to 0x%x\n",reg_info->name,regval); */
      rv = sbx_diag_write_reg(unit,reg_info,regval);
      if (rv) {
	SB_LOG("sbx_diag_write_reg failed(%d)\n",rv);
	sal_free(Fe2k_lrp_regs_l);
	return (rv);
      }
      regval = 0;
    }
  }

  sal_free(Fe2k_lrp_regs_l);
  return 0;
}

int sbxDiagsSetErrorRegList(sbxDiagsInfo_t *pDiagsInfo,uint32_t unit) {
  sbhandle hdl;
  int idx = 0;
  int i = 0;
  soc_sbx_chip_info_t *chip_info = NULL;
  soc_sbx_reg_info_t *reg_info = NULL;
  /* these regs should all be zero */
  const char* err_list[ERR_LIST_CNT] = {"fcs_error","jabber","fifo_status","fragmented",
					"runt","undersize","mtu_exceeded","false_carrier",
					"invalid_data","bad_length","aligment_error",
					"unknown_ctrl","enq_drop_bit","pr_pre3_error","pt_pte3_error",
                                        "pr_pre2_error","pt_pte2_error","pr_pre4_error","pt_pte4_error",
                                        "pt_pte5_error","pr_pre5_error","oversize","undersize","underflow",
					"wrong_sop","frame_error","error_pkt","unknown_ctrl","dropped"};

  hdl = SOC_SBX_CONTROL(unit)->sbhdl;
  if (!gFe2k_reginfo_l) {
    gFe2k_reginfo_l = sal_alloc(sizeof (soc_sbx_reg_info_list_t), "reginfo_l");
    if (!gFe2k_reginfo_l) {
      printk("ERROR: sbx_reg_list failed. Out of Memory \n");
      return CMD_FAIL;
    }
  }
  gFe2k_reginfo_l->count = 0;
  if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info,1) != CMD_OK) {
    printk("ERROR: Register info unknown for unit %d \n", unit);
    sal_free(gFe2k_reginfo_l);
    return (-1);
  }

  for(idx = 0; idx < chip_info->nregs; idx++) {
    reg_info = chip_info->regs[idx];
    for(i=0;i<ERR_LIST_CNT;i++) {
      if (strstr(reg_info->name,err_list[i])) {
	gFe2k_reginfo_l->idx[gFe2k_reginfo_l->count++] = idx;
      }
    }
  }
  return 0;
}

int sbxDiagsCheckForErrors(uint32_t unit) {

  int i = 0;
  int idx = 0;
  int rv = 0;
  sbhandle hdl;
  uint32_t regval = 0;
  soc_sbx_reg_info_t *reg_info = NULL;
  soc_sbx_chip_info_t *chip_info = NULL;

  hdl = SOC_SBX_CONTROL(unit)->sbhdl;

  if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info,1) != CMD_OK) {
    printk("ERROR: Register info unknown for unit %d \n", unit);
    return (-1);
  }

  for(i = 0; i < gFe2k_reginfo_l->count; i++) {
    idx = gFe2k_reginfo_l->idx[i];
    if (idx >= 0) {
      reg_info = chip_info->regs[idx];
      rv = sbx_diag_read_reg(unit,reg_info,&regval);
      if (rv != CMD_OK) {
	SB_LOG("ERROR: sbx_diag_read_reg returned(%d)\n");
	return (-1);
      }
      if (regval != 0) {
	/* there is a an error , print it out */
	SB_LOG("\nReg:%s has errors 0x%x\n",reg_info->name,regval);
	rv = sbx_diag_write_reg(unit, reg_info, regval);
	if (rv != CMD_OK) {
	  SB_LOG("ERROR: sbx_diag_reg_write returned(%d)\n");
	  return (-1);
	}
	return (-1);
      }
    }
  }
  return 0;
}



int sbxDiagsPollForErrors(uint32_t sp, uint32_t ep,uint32_t unit) {
  sbhandle hdl;
  int rv = 0;
  uint32_t uData = 0;
  uint32_t uRetry;
  uint8_t bAG0_Incrementing = 0;
  uint8_t bAG1_Incrementing = 0;

  hdl = SOC_SBX_CONTROL(unit)->sbhdl;

  rv = sbxDiagsCheckForErrors(unit);
  if (rv != 0) {
    return rv;
  }

  if (sp == 24 || sp == 25) {
    rv = sbFe2000Poll10GCounters(unit,sp);
    if (rv != 0) {
      return rv;
    }
  } else {
    /* Based on start port and end port we should see AG0 and/or AG1 Tx packet counts always incrementing */
    if (sp < 11) {
      uRetry=600;
      while ( --uRetry && !bAG0_Incrementing) {
	uData = SAND_HAL_READ(hdl,CA,PT_PTE2_PKT_CNT);
	if (uAGM0_TxCount+uData <= uAGM0_TxCount) { /* if this count <= last count, pkt stopped */
	  thin_delay(300); continue;
	}  else {
	  uAGM0_TxCount += uData;
	  bAG0_Incrementing = 1;
	}
      }
      if (uRetry == 0) {
	SB_LOG("\nERROR AG0: looping packet stopped unexpectedly\n");
	rv = -1;
      }
    }

    if (ep > 11) {
      uRetry=600;
      while ( --uRetry && !bAG1_Incrementing) {
	uData = SAND_HAL_READ(hdl,CA,PT_PTE3_PKT_CNT);
	if (uAGM1_TxCount+uData <= uAGM1_TxCount) { /* if this count <= last count, pkt stopped */
	  thin_delay(300); continue;
	}  else {
	  uAGM1_TxCount += uData;
	  bAG1_Incrementing = 1;
	}
      }

      if (uRetry == 0) {
	SB_LOG("\nERROR AG1: looping packet stopped unexpectedly\n");
	rv = -1;
      }

    }

    uData = SAND_HAL_READ(hdl,CA,PR_PRE2_PKT_CNT);
    uAGM0_RxCount += uData;

    uData = SAND_HAL_READ(hdl,CA,PR_PRE3_PKT_CNT);
    uAGM1_RxCount += uData;

    SB_LOG("AGM0 Rx:%lld Tx:%lld, AGM1 Rx:%lld Tx:%lld\r",uAGM0_RxCount,uAGM0_TxCount,
	   uAGM1_RxCount, uAGM1_TxCount);
  }
  return rv;
}

void sigcatcher(void)
{
  SB_LOG("\n");
  gStopSnakeTest = 1;
  return;
}
#else /* BCM_FE2000_SUPPORT */
int appl_test_fe2000_diags_not_empty;
#endif /* BCM_FE2000_SUPPORT */
