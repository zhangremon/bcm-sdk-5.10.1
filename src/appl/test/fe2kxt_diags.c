/*
 * $Id: fe2kxt_diags.c 1.14.100.1 Broadcom SDK $
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
 * File:        fe2kxt_diags.c
 * Purpose:     FE-2KXT-specific diagnostics tests
 * Requires:
 */

#ifdef BCM_FE2000_SUPPORT
#include <appl/test/fe2kxt_diags.h>
#include "sbFe2000XtInitUtils.h"
#include "fe2k-asm2-intd.h"
#include "sbFe2000XtInit.h"
#include <bcm/port.h>
#include <bcm/l2.h>
#include <bcm/stack.h>

#include <soc/sbx/fe2kxt/sbZfC2DiagUcodeCtl.hx>
#include <soc/sbx/fe2kxt/sbZfC2DiagUcodeShared.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmInternal0MemoryEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmInternal1MemoryEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2LrLrpInstructionMemoryBank0Entry.hx>

#include <appl/diag/sbx/register.h>
#include <appl/diag/system.h>
#include <appl/diag/test.h>
#include <soc/sbx/sbx_txrx.h>
#include <appl/diag/sbx/register.h>
#include <soc/sbx/hal_ka_auto.h>
#ifndef __KERNEL__
#include <signal.h>
#endif
#include <bcm/error.h>
#include <bcm/vlan.h>
#include <appl/diag/sbx/brd_sbx.h>
/* some g2p3 required */
#include <soc/sbx/g2p3/g2p3.h>


extern cmd_result_t sbx_diag_write_reg(int unit, soc_sbx_reg_info_t *reg_info, uint32_t v);
extern cmd_result_t sbx_diag_read_reg(int unit, soc_sbx_reg_info_t *reg_info, uint32_t *v);
extern int bcm_stk_modid_get(int unit, int *);
extern cmd_result_t sbx_reg_info_list_get(soc_sbx_chip_info_t *chip_info, soc_sbx_reg_info_list_t *reginfo_l,
					  char *regname, int exactmatch);
extern uint32_t sbFe2000SwapInstructionMemoryBank(sbhandle userDeviceHandle);
extern int soc_sbx_counter_stop(int unit);
extern int bcm_stat_clear(int	unit, bcm_port_t port);
extern void sbx_fe2000_reg_list_prune_reset(int,soc_sbx_chip_info_t *, soc_sbx_reg_info_list_t *);

void XtAddQueue(sbhandle userDeviceHandle, uint32_t uInPre, uint32_t uInPort, uint32_t uOutPte,
		uint32_t uOutPort, uint32_t uQueue);

/* some gu2p3 required for getting queueId for route header */
extern int soc_sbx_g2p3_ep2e_set(int unit,int iport,soc_sbx_g2p3_ep2e_t *e);
extern int soc_sbx_g2p3_ep2e_get(int unit,int iport,soc_sbx_g2p3_ep2e_t *e);

#define SNAKE_VLAN_START 2
#define CAM_BIST_WAIT 125 /* us */
/* default patterns used in memory testing */
const uint64_t g_Fe2kxtNarrowPortpattern = 0xAAAAAAAAALL;
const uint64_t g_Fe2kxtWidePortpattern = 0xAAAAAAAAAAAAAAAALL;
const uint64_t g_Fe2kxtNarrowPortantipattern = 0x555555555LL;
const uint64_t g_Fe2kxtWidePortantipattern = 0x5555555555555555LL;

static uint32_t XgmToPtPr[SB_FE2000XT_NUM_XG_INTERFACES] = { 4, 5, 2, 3 };

/* default lrp microcode */
/* lrp ucode phases */
fe2kAsm2IntD fe2kxtdiag_phase0;
fe2kAsm2IntD fe2kxtdiag_phase1;
fe2kAsm2IntD fe2kxtdiag_phase2;

extern cmd_result_t cmd_soc_sbx_tx(int unit, args_t *args);
extern cmd_result_t cmd_soc_sbx_rx(int unit, args_t *args);
extern cmd_result_t sbx_diag_write_reg(int unit, soc_sbx_reg_info_t *reg_info, uint32_t v);
extern cmd_result_t sbx_diag_read_reg(int unit, soc_sbx_reg_info_t *reg_info, uint32_t *v);
extern int soc_sbx_fe2000_am_read(int unit, int32 block_num, int32 block_offset, uint32 reg, uint32 *data);
extern uint32_t sbFe2000UtilXgmMiimRead(sbhandle userDeviceHandle, uint8_t bInternalPhy, uint8_t bClause45,
					uint32_t uDevAddr, uint32_t uPhyOrPortAddr, uint32_t uRegAddr,
					uint32_t *pData);

extern uint32_t sbFe2000UtilXgmMiimWrite(sbhandle userDeviceHandle,uint8_t bInternalPhy, uint8_t bClause45,
					 uint32_t uDevAddr, uint32_t uPhyOrPortAddr, uint32_t uRegAddr, 
					 uint32_t uData);

extern uint32_t sbFe2000UcodeLoad(sbhandle userDeviceHandle, uint8_t * cParams);
extern uint32_t sbFe2000EnableLr(sbhandle userDeviceHandle, uint8_t *ucode);

extern int fe2kXtAsm2IntD__initDefaultPhase0(fe2kAsm2IntD *a_p);
extern int fe2kXtAsm2IntD__initDefaultPhase1(fe2kAsm2IntD *a_p);
extern int fe2kXtAsm2IntD__initDefaultPhase2(fe2kAsm2IntD *a_p);

/* set on ctrl-c to stop test */
static uint8_t gStopSnakeTest = 0;
static uint64_t uAGM0_RxCount = 0;
static uint64_t uAGM0_TxCount = 0;
static uint64_t uAGM1_RxCount = 0;
static uint64_t uAGM1_TxCount = 0;
static uint64_t uXG_TxCount0 = 0;
static uint64_t uXG_RxCount0 = 0;
static uint64_t uXG_TxCount1 = 0;
static uint64_t uXG_RxCount1 = 0;

/* used in loopback tests */
char *l2_macSAaddrPkt1_str = "00:00:00:02:00:00";
char *l2_macSAaddrPkt2_str = "00:00:00:01:00:00";
char *l2_macDAaddrPkt1_str = "00:00:00:01:00:00";
char *l2_macDAaddrPkt2_str = "00:00:00:02:00:00";

static uint32_t g_AGTxPorts[2 /*SB_FE2000XT_NUM_AG_INTERFACES*/][12/*SB_FE2000XT_MAX_AG_PORTS*/] ={{0}};
static uint32_t g_AGRxPorts[2 /*SB_FE2000XT_NUM_AG_INTERFACES*/][12/*SB_FE2000XT_MAX_AG_PORTS*/] ={{0}};

soc_sbx_reg_info_list_t *gFe2kxt_reginfo_l = NULL;
#define XE0_PORT 24
#define XE1_PORT 25
#define XE2_PORT 26
#define XE3_PORT 27
#define IS_ODD(p) (p%2)
#define IS_EVEN(p) (!IS_ODD(p))
#define SNAKE_INTERVAL_US 1
#define PORT_CHECK 20

/* for debug messages */
const char* sbFe2kxtDiagsPortInterfaceStr[] = {"SPI0","SPI1","AGM0",
					       "AGM1","XGM0","XGM1",
					       "PCI"};

const char* sbFe2kxtDiagsMemoryStr[] = {"NARROW_PORT_0",
					"NARROW_PORT_1",
					"WIDE_PORT"};

const char* sbFe2kxtDiagsMemoryTestStr[] = {"FE2KXT_MM0_NP0_DATA_BUS_WALKING_ONES",
					    "FE2KXT_MM1_NP0_DATA_BUS_WALKING_ONES",
					    "FE2KXT_MM0_NP0_DATA_BUS_WALKING_ZEROS",
					    "FE2KXT_MM1_NP0_DATA_BUS_WALKING_ZEROS",
					    "FE2KXT_MM0_NP0_ADDRESS_BUS",
					    "FE2KXT_MM1_NP0_ADDRESS_BUS",
					    "FE2KXT_MM0_NP1_DATA_BUS_WALKING_ONES",
					    "FE2KXT_MM1_NP1_DATA_BUS_WALKING_ONES",
					    "FE2KXT_MM0_NP1_DATA_BUS_WALKING_ZEROS",
					    "FE2KXT_MM1_NP1_DATA_BUS_WALKING_ZEROS",
					    "FE2KXT_MM0_NP1_ADDRESS_BUS",
					    "FE2KXT_MM1_NP1_ADDRESS_BUS",
					    "FE2KXT_MM0_WP_DATA_BUS_WALKING_ONES",
					    "FE2KXT_MM1_WP_DATA_BUS_WALKING_ONES",
					    "FE2KXT_MM0_WP_DATA_BUS_WALKING_ZEROS",
					    "FE2KXT_MM1_WP_DATA_BUS_WALKING_ZEROS",
					    "FE2KXT_MM0_WP_ADDRESS_BUS",
					    "FE2KXT_MM1_WP_ADDRESS_BUS",
					    "FE2KXT_MM0_NP0_RANDOM_MEMORY_TEST",
					    "FE2KXT_MM1_NP0_RANDOM_MEMORY_TEST",
					    "FE2KXT_MM0_NP1_RANDOM_MEMORY_TEST",
					    "FE2KXT_MM1_NP1_RANDOM_MEMORY_TEST",
					    "FE2KXT_MM0_WP_RANDOM_MEMORY_TEST",
					    "FE2KXT_MM1_WP_RANDOM_MEMORY_TEST"};



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

int sbFe2kxtDiagsSnakeTest(sbxDiagsInfo_t *pDiagsInfo)
{

  uint32_t nAgm, uDataRx, uDataTx;
  int start_port, end_port, port;
  int dbg,i;
  int rv = 0;
  uint32_t unit;
  int timer_sec = 0;
  int fe_1 = -1; int qe_1 = -1;
  int fe_2 = -1; int qe_2 = -1;
  uint32_t qe_unit = -1;
  int ge_stat = 0;

  unit = pDiagsInfo->unit;
  dbg = pDiagsInfo->debug_level;
#ifndef __KERNEL__
  signal(SIGINT,(void *)sigcatcher_fe2kxt);
#endif
  start_port = pDiagsInfo->start_port;
  end_port = pDiagsInfo->end_port;

  /* locate the FE's */
  for(i=0;i<soc_ndev;i++) {
    if (SOC_IS_SBX_FE2KXT(i)) {
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

  /* Test requires LRP be sure it is not in bypass mode */
  SAND_HAL_RMW_FIELD(pDiagsInfo->userDeviceHandle, C2, LR_CONFIG0, BYPASS,0);

  soc_sbx_counter_stop(unit);

  /* Set the error reg list to check for errors */
  if (( rv = Fe2kXtDiagsSetErrorRegList(pDiagsInfo,unit)) < 0 ) {
    test_error(unit,
	       "%s failed to set error list (%s). \n", SOC_CHIP_STRING(unit),
	       bcm_errmsg(rv));
    return(-1);
  }

  printk("Clearing any errors..\n");
  if (( rv = Fe2kXtDiagsClearErrors(pDiagsInfo)) < 0 ) {
    test_error(unit,
	       "%s failed to clear errors (%s). \n", SOC_CHIP_STRING(unit),
	       bcm_errmsg(rv));
    return(-1);
  }

  printk("Clearing counters ..\n");
  if (( rv = Fe2kXtDiagsClearCounters(pDiagsInfo)) < 0 ) {
    test_error(unit,
	       "%s failed to clear counters (%s). \n", SOC_CHIP_STRING(unit),
	       bcm_errmsg(rv));
    return(-1);
  }

  /* initalize the ports */
  if (pDiagsInfo->init_ports) {
    printk("Initializing ports..");
    rv = bcm_port_init(unit);
    if (BCM_FAILURE(rv)) {
      printk("ERROR: bcm_port_init failed(%d)(%s)\n",rv,bcm_errmsg(rv));
    }

    thin_delay(3*1E9);
    printk("\n");
  }

  /* initialize counters to zero */
  uAGM0_RxCount = 0; uAGM0_TxCount = 0;
  uAGM1_RxCount = 0; uAGM1_TxCount = 0;

  /* check the ports being test are up */
  if (( rv = sbFe2kxtDiagsSnakeTest_check_ports(pDiagsInfo)) < 0 ) {
    test_error(unit,
	       "%s failed port check. \n", SOC_CHIP_STRING(unit));
    return(-1);
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

  /*
   *  Set ep2e pid==port to avoid split horizon exception. This is
   *  temporary to get things working. Need to resolve why in tme mode
   *  the pid == 0 for all ep2e tables.
   */

  if (( rv = sbFe2kxtDiagsSnakeTest_ep2e_modify(pDiagsInfo)) < 0 ) {
    test_error(unit,
	       "%s: failed to modify ep2e entries. \n",SOC_CHIP_STRING(unit));
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
    rv = sbFe2kxtDiagsReceivePciPacket(pDiagsInfo);
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
      rv = Fe2kXtDiagsPollForErrors(pDiagsInfo->start_port,pDiagsInfo->end_port,unit);
      if (rv != 0) {
	break;
      }
      timer_sec += 1;
    }

    /* check again for errors to be sure */
    if (rv == 0 ) {
      rv = Fe2kXtDiagsPollForErrors(pDiagsInfo->start_port,pDiagsInfo->end_port,unit);
      if (rv != 0) return (-1);
    }

    /* check that each port start_port to end_port tx/rx packets */
    uDataTx = 0; uDataRx = 0;
    for(port=(int)pDiagsInfo->start_port;port<=(int)pDiagsInfo->end_port;port++) {
      nAgm = (port <= 11) ? 0 : 1;
      if (( ge_stat = soc_sbx_fe2000_am_read(unit,nAgm,port%12,REG_OFFSET_C2(AM_TX_VALID_PKT_CNT),&uDataTx)) == 0) {
	if (uDataTx == 0) {
	  printk("ERROR port:%s did not transmit any packets\n",SOC_PORT_NAME(unit,port));
	  rv = -1;
	}
      } else {
	printk("WARN: Could not read ge%d tx stats (%s)\n",port,bcm_errmsg(ge_stat));
      }
      if (( ge_stat = soc_sbx_fe2000_am_read(unit,nAgm,port%12,REG_OFFSET_C2(AM_RX_VALID_PKT_CNT),&uDataRx)) == 0) {
	if (uDataRx == 0) {
	  printk("ERROR port:%s did not recieve any packets\n",SOC_PORT_NAME(unit,port));
	  rv = -1;
	}
      } else {
	printk("WARN: Could not read ge%d rx stats (%s)\n",port,bcm_errmsg(ge_stat));
      }
    }

    /* check the ports being test are still up */
    if ((sbFe2kxtDiagsSnakeTest_check_ports(pDiagsInfo)) < 0 ) {
      test_error(unit,
		 "Above Port(s) went down during test.\n Cable removed? Re-test.\n");
      return(-1);
    }

    if((gStopSnakeTest || (timer_sec >= pDiagsInfo->uSnakeRunTime)) && rv == 0) {
      printk("\nStopping Test. No Errors Found during testing.\n");
    } else if (rv) {
      printk("\nTest stopped with errors, if errors persist try restarting bcm.user \n");
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

int sbFe2kxtDiagsSnakeTest_check_ports(sbxDiagsInfo_t *pDiagsInfo)
{

  int start_port,end_port;
  int unit;
  int port = 0;
  int port_up = 0;
  int i = 0;
  int stat = 0;
  int rv = 0;

  unit = pDiagsInfo->unit;
  start_port = pDiagsInfo->start_port;
  end_port = pDiagsInfo->end_port;

  for (port = (int)start_port; port <= (int)end_port; port++) {
    /* check status a few times , port status does not always
       get the correct values the first time its called.*/
    for(i = 0; i < PORT_CHECK; i++) {
      rv = bcm_port_link_status_get(unit,port,&port_up);
      if (BCM_FAILURE(rv)) {
	printk("ERROR: bcm_port_link_status_get(%d)(%s)\n",rv,bcm_errmsg(rv));
	return (-1);
      }
      if (port_up == TRUE) break;
      thin_delay(1*1E9);
    }
  
    if (!port_up) {
      printk("ERROR port:%s is down\n",SOC_PORT_NAME(unit,port));
      stat = -1;
    }
  }
  return stat;
}

int sbFe2kxtDiagsSnakeTest_vlan_setup(sbxDiagsInfo_t *pDiagsInfo)

{
  int unit;
  int rv;
  int start_port;
  int end_port;
  int port, port_untag;
  int vlan, end_vlan;
  int dbg;
  pbmp_t port_pbm;
  pbmp_t port_pbm_untag;
  bcm_vlan_control_vlan_t vlan_control;

  unit = pDiagsInfo->unit;
  start_port = pDiagsInfo->start_port;
  end_port = pDiagsInfo->end_port;
  dbg = pDiagsInfo->debug_level;

  /* clear any existing vlans */
  if ((rv = bcm_vlan_destroy_all(unit)) < 0) {
    printk("Could not destroy existing VLANs: %s\n",
	   bcm_errmsg(rv));
    return (-1);
  }

  port = start_port;
  end_vlan = (((end_port - start_port) + 1)/2) + 1;

  for(vlan=SNAKE_VLAN_START;vlan<=end_vlan;vlan++,port+=2) {

    BCM_PBMP_CLEAR(port_pbm);
    BCM_PBMP_CLEAR(port_pbm_untag);

    if (dbg) {
      printk("vlan create %d\n",vlan);
    }

    if (( rv = bcm_vlan_create(unit,vlan)) < 0 ) {
      printk("ERROR: bcm_vlan_create returned(%d) (%s) when creating vlan %d \n",
	     rv,bcm_errmsg(rv),vlan);
      return (-1);

    }

    BCM_PBMP_PORT_ADD(port_pbm, port);
    BCM_PBMP_PORT_ADD(port_pbm, port+1);
    BCM_PBMP_PORT_ADD(port_pbm_untag, port);
    BCM_PBMP_PORT_ADD(port_pbm_untag, port+1);

    if (dbg) {
      printk("vlan add %d pbm=%s,%s ubm=%s,%s\n",vlan,SOC_PORT_NAME(unit,port),
	     SOC_PORT_NAME(unit,port+1),SOC_PORT_NAME(unit,port),
	     SOC_PORT_NAME(unit,port+1));
    }

    if ((rv = bcm_vlan_port_add(unit,vlan,port_pbm,port_pbm_untag)) < 0) {
      printk("ERROR: Could not add vlan:%d to ports %s,%s: (%s)\n",vlan,
	     SOC_PORT_NAME(unit,port),SOC_PORT_NAME(unit,port+1), 
	     bcm_errmsg(rv));
      return (-1);
    }

    if (dbg) {
      printk("pvlan set %s,%s %d\n",SOC_PORT_NAME(unit,port),
	     SOC_PORT_NAME(unit,port+1),vlan);
    }

    /* turn off learning */
    if ((rv = bcm_vlan_control_vlan_get(unit,vlan,&vlan_control)) < 0) {
      printk("ERROR: Could get vlan control for vlan:%d, (%s)\n",vlan,bcm_errmsg(rv));
      return (-1);
    }

    vlan_control.flags |= BCM_VLAN_LEARN_DISABLE;
    if ((rv = bcm_vlan_control_vlan_set(unit,vlan,vlan_control)) < 0) {
      printk("ERROR: Could set vlan control for vlan:%d, (%s)\n",vlan,bcm_errmsg(rv));
      return (-1);
    }


    PBMP_ITER(port_pbm_untag,port_untag) {
      if ((rv = bcm_port_untagged_vlan_set(unit, port_untag, vlan)) < 0) {
	printk("Error setting port %s default VLAN to %d: %s\n",
	       SOC_PORT_NAME(unit, port_untag), vlan, bcm_errmsg(rv));
	return (-1);
      }
    }
  }

  return 0;
}

int sbFe2kxtDiagsSnakeTest_l2_create(sbxDiagsInfo_t *pDiagsInfo)
{

  int rv = 0;
  int unit;
  int vlan;
  int port,start_port,end_port;
  bcm_l2_addr_t  l2addr;
  sal_mac_addr_t l2_macSAaddr;
  char *l2_macSAaddr_str = NULL;
  int dbg;
  int i;
  int femodid = 0;
  
  sal_memset(&l2addr, 0, sizeof(bcm_l2_addr_t));
  unit = pDiagsInfo->unit;
  start_port = pDiagsInfo->start_port;
  end_port = pDiagsInfo->end_port;
  dbg = pDiagsInfo->debug_level;

  rv = bcm_stk_modid_get(unit,&femodid);
  if (BCM_FAILURE(rv)) {
    test_error(unit,
	       "%s failed when getting modid_ (%s) .\n",
	       SOC_CHIP_STRING(unit),bcm_errmsg(rv));
    return CMD_FAIL;
  }

  for (i = 0; i< 2; i++ ) {
    /* clear existing l2 entries */
    if ((rv = bcm_l2_clear(unit)) < 0) {
      printk("ERROR: bcm_l2_clear (%d) (%s)\n",rv,
	     bcm_errmsg(rv));
      return (-1);
    }
  }

  vlan = SNAKE_VLAN_START;
  for(port=start_port;port<=end_port;port++) {
    if (IS_ODD(port)) {
      l2_macSAaddr_str = l2_macSAaddrPkt1_str;
    } else {
      l2_macSAaddr_str = l2_macSAaddrPkt2_str;
    }

    parse_macaddr(l2_macSAaddr_str,l2_macSAaddr);
    bcm_l2_addr_t_init(&l2addr, l2_macSAaddr, vlan);

    l2addr.modid = femodid;
    l2addr.port = port;
    l2addr.flags = BCM_L2_STATIC;

    if (dbg) {
      printk("l2 add mac=%s vlanid=%d module=%d pbm=%s static=true\n",
	     l2_macSAaddr_str,vlan,femodid,SOC_PORT_NAME(unit,port));
    }

    if ((rv = bcm_l2_addr_add(unit,&l2addr)) < 0) {
      printk("ERROR: bcm_l2_addr_add (%d) (%s)\n",rv,
	     bcm_errmsg(rv));
      return (-1);
    }

    /* two adjacent ports are on the same vlan, increment the vlan
     * for the next port pair */
    if (port % 2)
      vlan++;
  }

  return 0;

}
/* avoid split horizon on egress entry from QE-->FE */
int sbFe2kxtDiagsSnakeTest_ep2e_modify(sbxDiagsInfo_t *pDiagsInfo)
{

  bcm_port_t port;
  soc_sbx_g2p3_ep2e_t e0;
  soc_sbx_g2p3_ep2e_t *e = &e0;
  int rv = 0;
  int unit = pDiagsInfo->unit;

  for (port=0;port<=23;port++) {

    rv = soc_sbx_g2p3_ep2e_get(unit,
			       port,
			       e);

    if (rv != SOC_E_NONE && rv != SOC_E_NOT_FOUND) {
      test_error(unit,
		 "%s: ep2e_get for port(%d) failed(%s).\n",
		 SOC_CHIP_STRING(unit),port,bcm_errmsg(rv));
      return rv;
    }

    e->pid = port;
    rv = soc_sbx_g2p3_ep2e_set(unit,
			       port,
			       e);

    if (rv != SOC_E_NONE) {
      test_error(unit,
		 "%s: ep2e_set for port(%d) failed(%s).\n",
		 SOC_CHIP_STRING(unit),port,bcm_errmsg(rv));
      return rv;
    }
  }
  return 0;
}

/* avoid hairpin on ingress FrontPanel-->FE needed for fiber ports 
 * and if looping packet directly back on 10g port (C1/polaris board) */
int sbFe2kxtDiagsSnakeTest_lpi_modify(sbxDiagsInfo_t *pDiagsInfo)
{

  bcm_port_t port;
  int rv = SOC_E_NONE;
  vlan_id_t vid = 0;
  soc_sbx_g2p3_pv2e_t e0;
  soc_sbx_g2p3_pv2e_t *e = &e0;

  soc_sbx_g2p3_lp_t e0_lpi;
  soc_sbx_g2p3_lp_t *e_lpi = &e0_lpi;

  int unit = pDiagsInfo->unit;

  for (port = 12; port <= 25; port++) {

    /* get the vlan this port is on */
    bcm_port_untagged_vlan_get(unit,port,&vid);
    if (vid == 1) continue;

    rv = soc_sbx_g2p3_pv2e_get(unit,
			       vid,
			       port,
			       e);

    if (rv != SOC_E_NONE && rv != SOC_E_NOT_FOUND) {
      test_error(unit,
		 "%s: pv2e_get for port(%d) failed(%s).\n",
		 SOC_CHIP_STRING(unit),port,bcm_errmsg(rv));
      return rv;
    }

    /* use lpi == port, to avoid ports using the same lpi and pid */
    /* can cause a hairpin exception EXC_MAC_HAIRPIN_IDX . Each port */
    /* now will have a unique lpi and pid */
    
    if (port == 25) {
      e->lpi = 13;
    } else {
      e->lpi = port;
    }
    rv = soc_sbx_g2p3_pv2e_set(unit,
			       vid,
			       port,
			       e);

    if (rv != SOC_E_NONE && rv != SOC_E_NOT_FOUND) {
      test_error(unit,
		 "%s: pv2e_set for port(%d) failed(%s).\n",
		 SOC_CHIP_STRING(unit),port,bcm_errmsg(rv));
      return rv;
    }


    /* get the lpi */
    rv = soc_sbx_g2p3_lp_get(unit,
			     e->lpi,
			     e_lpi);

    if (rv != SOC_E_NONE ) {
      test_error(unit,
		 "%s: lp_get for lpi(0x%x failed(%s).\n",
		 SOC_CHIP_STRING(unit),e->lpi,bcm_errmsg(rv));
      return rv;
    }

    /* change the pid */
    if (port == 25) {
      e_lpi->pid = 13;
    } else {
      e_lpi->pid = port;
    }
    rv = soc_sbx_g2p3_lp_set(unit,
			     e->lpi,
			     e_lpi);

    if (rv != SOC_E_NONE ) {
      test_error(unit,
		 "%s: lp_set for lpi(0x%x failed(%s).\n",
		 SOC_CHIP_STRING(unit),e->lpi,bcm_errmsg(rv));
      return rv;
    }

  }

  return rv;
}


/* Need to mimic entering sbx_tx on console */
int sbFe2kxtDiagsSnakeTest_inject_packets( sbxDiagsInfo_t *pDiagsInfo, int tx_unit, int sp, int ep)

{

  char rh_cmd[2][40] =  {{'\0'}};
  char pay_cmd[2][50] = {{'\0'}};
  char len_cmd[5] = {'\0'};
  char *shim_cmd = "shim=0x00c0ffee";
  int start_port, end_port, *pvar;
  int unit;
  int i;
  int rv = 0;
  int dbg;
  sal_mac_addr_t l2_macDAaddr;
  sal_mac_addr_t l2_macSAaddr;
  args_t *pArgs = NULL;
  int qid_start = 0;
  int qid_end = 0;
  int femodid = 0;
  int tme = 0;
  int num_cos = 0;
  bcm_gport_t front_panel_port, fabric_port;

  start_port = sp;
  end_port = ep;
  unit = pDiagsInfo->unit;
  dbg = pDiagsInfo->debug_level;

  if ((pArgs = sal_alloc(sizeof (args_t), "pArgs")) == NULL) {
    printk("(%s) Error: out of memory\n",FUNCTION_NAME());
    return CMD_FAIL;
  }  

  tme = soc_property_get(unit,spn_QE_TME_MODE,0);
  num_cos = soc_property_get(unit, spn_BCM_NUM_COS, (tme == 1) ? 16 : 8);

  rv = bcm_stk_modid_get(unit,&femodid);
  if (BCM_FAILURE(rv)) {
    test_error(unit,
	       "%s failed when getting modid(%s) .\n",
	       SOC_CHIP_STRING(unit),bcm_errmsg(rv));
      sal_free(pArgs);
    return CMD_FAIL;
  }

  /* convert front panel ports into fabric ports  */
  pvar = &start_port;
  while (pvar) {
      BCM_GPORT_MODPORT_SET(front_panel_port, femodid, *pvar);
      rv = bcm_stk_fabric_map_get(unit, front_panel_port, &fabric_port);
      if (BCM_FAILURE(rv)) {
          printk("Failed to convert front panel port %d to fabric port: %s\n",
                 BCM_GPORT_MODPORT_PORT_GET(front_panel_port), bcm_errmsg(rv));
          sal_free(pArgs);
          return CMD_FAIL;
      }

      if (dbg) {
          printk ("Converted front panel port %d to fabric port %d\n",
                  BCM_GPORT_MODPORT_PORT_GET(front_panel_port),
                  BCM_GPORT_MODPORT_PORT_GET(fabric_port));
      }

      *pvar = BCM_GPORT_MODPORT_PORT_GET(fabric_port);
      if (pvar == &end_port) {
          pvar = NULL;
      } else {
          pvar = &end_port;
      }
  }

  /* get the basequeue for the start_port, end_port */
  qid_start = SOC_SBX_NODE_PORT_TO_QID(unit,femodid,start_port,num_cos);
  qid_end = SOC_SBX_NODE_PORT_TO_QID(unit,femodid,end_port,num_cos);

  /* build ERH for packets going from QE out start_port */

  sprintf(rh_cmd[0],"rh=0x%03x0%04x0000000000000000",qid_start/4,
	  pDiagsInfo->nInjectPacketsLen-8);

  parse_macaddr(l2_macDAaddrPkt1_str,l2_macDAaddr);
  parse_macaddr(l2_macSAaddrPkt1_str,l2_macSAaddr);

  sprintf(pay_cmd[0],"pat=0x%02d%02d%02d%02d%02d%02d%02d%02d%02d%02d%02d%02d9000",
	  l2_macDAaddr[0],l2_macDAaddr[1],l2_macDAaddr[2],l2_macDAaddr[3],l2_macDAaddr[4],l2_macDAaddr[5],
	  l2_macSAaddr[0],l2_macSAaddr[1],l2_macSAaddr[2],l2_macSAaddr[3],l2_macSAaddr[4],l2_macSAaddr[5]);


  /* build ERH for packets going from QE out end_port */
  sprintf(rh_cmd[1],"rh=0x%03x0%04x0000000000000000",qid_end/4,
	  pDiagsInfo->nInjectPacketsLen-8);

  parse_macaddr(l2_macDAaddrPkt2_str,l2_macDAaddr);
  parse_macaddr(l2_macSAaddrPkt2_str,l2_macSAaddr);

  sprintf(pay_cmd[1],"pat=0x%02d%02d%02d%02d%02d%02d%02d%02d%02d%02d%02d%02d9000",
	  l2_macDAaddr[0],l2_macDAaddr[1],l2_macDAaddr[2],l2_macDAaddr[3],l2_macDAaddr[4],l2_macDAaddr[5],
	  l2_macSAaddr[0],l2_macSAaddr[1],l2_macSAaddr[2],l2_macSAaddr[3],l2_macSAaddr[4],l2_macSAaddr[5]);

  if (dbg >= 1) {
    printk("Injecting %d packets into %s PCI port..to go out ge%d\n",pDiagsInfo->nInjectPackets,
	   SOC_CHIP_STRING(tx_unit),sp);

    printk("Injecting %d packets into %s PCI port..to go out ge%d\n",pDiagsInfo->nInjectPackets,
	   SOC_CHIP_STRING(tx_unit),ep);
  }

  /* Injected packets alternate between going out start port,
   * and going out end_port. If 100 packets are injected, 100
   * will go out start_port, and another 100 out end_port to
   * allow all ports between start and end to tx and rx packets
   */

  for (i = 0; i < pDiagsInfo->nInjectPackets*2; i++) {
    sal_memset(pArgs,0x0,sizeof(args_t));
    sprintf(len_cmd,"len=%d",pDiagsInfo->nInjectPacketsLen);
    pArgs->a_cmd="SBX_TX";
    pArgs->a_argc = 5;
    pArgs->a_arg=1;
    pArgs->a_argv[0] = "sbx_tx";
    pArgs->a_argv[1] = IS_EVEN(i) ? rh_cmd[1] : rh_cmd[0];
    pArgs->a_argv[2] = shim_cmd;
    pArgs->a_argv[3] = IS_EVEN(i) ? pay_cmd[1] : pay_cmd[0];
    pArgs->a_argv[4] = len_cmd;
    if (dbg >= 2)
      printk("%d:sbx_tx %s %s %s %s\n", tx_unit,pArgs->a_argv[1],shim_cmd,pArgs->a_argv[3],len_cmd);
    rv = cmd_soc_sbx_tx(tx_unit,pArgs);
    INTERPACKET_GAP(); /* spread pkts out */
    if (rv != CMD_OK) {
      printk("ERROR: sbx_tx returned %d\n",rv);
      sal_free(pArgs);
      return (-1);
    }
  }    

  sal_free(pArgs);
  return rv;
}


/*
 *   PRBS (pseudorandom binary sequence) Tests
 *   QE <---> FE prbs test
 */

int sbFe2kxtDiagsSPIPrbsTest(sbxDiagsInfo_t *pDiagsInfo)

{

  sbhandle fehdl, qehdl;
  uint32_t nLSFR;
  int nInvert = 0;
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
  uint32_t uStatus0,uStatus1,uStatus2;
  uint32_t prbs_direction = 0;
  uint32_t uField = 0;
  uint8_t bRxInSync = TRUE;

  #define PRBS_TO_FE 0
  #define PRBS_TO_QE 1

  nLSFR = pDiagsInfo->nLSFR;
  fe = pDiagsInfo->unit;
  nSpi = pDiagsInfo->spi_interface;
  prbs_direction = pDiagsInfo->prbs_direction;

  /* locate the FE's */
  for(i=0;i<soc_ndev;i++) {
    if (SOC_IS_SBX_FE2KXT(i)) {
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
    printk("ERROR Could not QE on board for this FE\n");
    return (-1);
  }

  qehdl = SOC_SBX_CONTROL(qe)->sbhdl;
  fehdl = SOC_SBX_CONTROL(fe)->sbhdl;

  /* restore sr,st to powerup state */
  if (prbs_direction == PRBS_TO_FE) {
    SAND_HAL_RMW_FIELD(qehdl, KA, ST0_CONFIG1,PC_RESET,1);
    SAND_HAL_RMW_FIELD(fehdl, C2, SR0_CONFIG0,PC_RESET,1);
    SAND_HAL_RMW_FIELD(qehdl, KA, ST1_CONFIG1,PC_RESET,1);
    SAND_HAL_RMW_FIELD(fehdl, C2, SR1_CONFIG0,PC_RESET,1);
  } else {
    SAND_HAL_RMW_FIELD(fehdl, C2, ST0_CONFIG1,PC_RESET,1);
    SAND_HAL_RMW_FIELD(qehdl, KA, SR0_CONFIG0,PC_RESET,1);
    SAND_HAL_RMW_FIELD(fehdl, C2, ST1_CONFIG1,PC_RESET,1);
    SAND_HAL_RMW_FIELD(qehdl, KA, SR1_CONFIG0,PC_RESET,1);

  }

  /* take st0, st1 out of reset on tx side */
  if (prbs_direction == PRBS_TO_FE) {
    SAND_HAL_RMW_FIELD(qehdl, KA, PC_CORE_RESET0, ST0_CORE_RESET, 0);
    SAND_HAL_RMW_FIELD(qehdl, KA, ST0_CONFIG1, PC_RESET, 0);
    SAND_HAL_RMW_FIELD(qehdl, KA, ST0_CONFIG1, TSCLK_EDGE, 0);
    SAND_HAL_RMW_FIELD(qehdl, KA, PC_CORE_RESET0, ST1_CORE_RESET, 0);
    SAND_HAL_RMW_FIELD(qehdl, KA, ST1_CONFIG1, PC_RESET, 0);
    SAND_HAL_RMW_FIELD(qehdl, KA, ST1_CONFIG1, TSCLK_EDGE, 0);
  } else {
    SAND_HAL_RMW_FIELD(fehdl, C2, PC_CORE_RESET, ST0_CORE_RESET, 0);
    SAND_HAL_RMW_FIELD(fehdl, C2, ST0_CONFIG1, PC_RESET, 0);
    SAND_HAL_RMW_FIELD(fehdl, C2, ST0_CONFIG1, TSCLK_EDGE, 0);
    SAND_HAL_RMW_FIELD(fehdl, C2, PC_CORE_RESET, ST1_CORE_RESET, 0);
    SAND_HAL_RMW_FIELD(fehdl, C2, ST1_CONFIG1, PC_RESET, 0);
    SAND_HAL_RMW_FIELD(fehdl, C2, ST1_CONFIG1, TSCLK_EDGE, 0);
  }

  /* enable  force training at tx */
  if (prbs_direction == PRBS_TO_FE) {
    SAND_HAL_RMW_FIELD(qehdl,KA,ST0_CONFIG0,TX_ENABLE,1);
    SAND_HAL_RMW_FIELD(qehdl,KA,ST0_CONFIG0,TX_FORCE_TRAINING,1);
    SAND_HAL_RMW_FIELD(qehdl,KA,ST1_CONFIG0,TX_ENABLE,1);
    SAND_HAL_RMW_FIELD(qehdl,KA,ST1_CONFIG0,TX_FORCE_TRAINING,1);
  } else {
    SAND_HAL_RMW_FIELD(fehdl,C2,ST0_CONFIG0,TX_ENABLE,1);
    SAND_HAL_RMW_FIELD(fehdl,C2,ST0_CONFIG0,TX_FORCE_TRAINING,1);
    SAND_HAL_RMW_FIELD(fehdl,C2,ST1_CONFIG0,TX_ENABLE,1);
    SAND_HAL_RMW_FIELD(fehdl,C2,ST1_CONFIG0,TX_FORCE_TRAINING,1);
  }

  /* take rx out of reset */
  if (prbs_direction == PRBS_TO_FE) {
    SAND_HAL_RMW_FIELD(fehdl, C2, PC_CORE_RESET, SR0_CORE_RESET, 0);
    SAND_HAL_RMW_FIELD(fehdl, C2, PC_CORE_RESET, SR1_CORE_RESET, 0);
    SAND_HAL_RMW_FIELD(fehdl, C2, SR0_CONFIG0, PC_RESET, 0);
    SAND_HAL_RMW_FIELD(fehdl, C2, SR1_CONFIG0, PC_RESET, 0);
  } else {
    SAND_HAL_RMW_FIELD(qehdl, KA, PC_CORE_RESET0, SR0_CORE_RESET, 0);
    SAND_HAL_RMW_FIELD(qehdl, KA, PC_CORE_RESET0, SR1_CORE_RESET, 0);
    SAND_HAL_RMW_FIELD(qehdl, KA, SR0_CONFIG0, PC_RESET, 0);
    SAND_HAL_RMW_FIELD(qehdl, KA, SR1_CONFIG0, PC_RESET, 0);
  }

  /* be sure force_error is cleared it is set later as a sanity check */
  SAND_HAL_RMW_FIELD(qehdl,KA,ST0_CONFIG2,FORCE_PRBS_ERROR,0);
  SAND_HAL_RMW_FIELD(qehdl,KA,ST1_CONFIG2,FORCE_PRBS_ERROR,0);
  SAND_HAL_RMW_FIELD(fehdl,C2,ST0_CONFIG2,FORCE_PRBS_ERROR,0);
  SAND_HAL_RMW_FIELD(fehdl,C2,ST1_CONFIG2,FORCE_PRBS_ERROR,0);

  /* enable rx at the reciever */
  if (prbs_direction == PRBS_TO_FE) {
      SAND_HAL_RMW_FIELD_STRIDE(fehdl,C2,SR,nSpi,SR_CONFIG0,RX_ENABLE,1);
      printk("Testing SPI%d interface QE(UNIT %d) --> FE(UNIT %d)\n",
	     nSpi,qe,fe);
  } else {
      printk("Testing SPI%d interface FE(UNIT %d) --> QE(UNIT %d)\n",
	     nSpi,fe,qe);
    if (nSpi == 0) {
      SAND_HAL_RMW_FIELD(qehdl,KA,SR0_CONFIG0,RX_ENABLE,1);
    } else {
      SAND_HAL_RMW_FIELD(qehdl,KA,SR1_CONFIG0,RX_ENABLE,1);
    }
  }

  /* remove force training at transmitter */
  if (prbs_direction == PRBS_TO_FE) {
    SAND_HAL_RMW_FIELD(qehdl, KA, ST0_CONFIG0, TX_FORCE_TRAINING, 0);
    SAND_HAL_RMW_FIELD(qehdl, KA, ST1_CONFIG0, TX_FORCE_TRAINING, 0);
  } else {
    SAND_HAL_RMW_FIELD(fehdl, C2, ST0_CONFIG0, TX_FORCE_TRAINING, 0);
    SAND_HAL_RMW_FIELD(fehdl, C2, ST1_CONFIG0, TX_FORCE_TRAINING, 0);
  }


  /* check that Rx is in sync for spi interface being tested */
  if (prbs_direction == PRBS_TO_FE) {
    if (nSpi == 0) {
      uData = SAND_HAL_READ(fehdl,C2,SR0_STATUS);
      if (SAND_HAL_GET_FIELD(C2,SR0_STATUS,RX_INSYNC,uData) != 1) {
	printk("Error %s SPI0 Rx is not in sync, sr0_status(%d) \n",SOC_CHIP_STRING(fe),uData);
	bRxInSync = FALSE;
      }
    } else {
      uData = SAND_HAL_READ(fehdl,C2,SR1_STATUS);
      if (SAND_HAL_GET_FIELD(C2,SR1_STATUS,RX_INSYNC,uData) != 1) {
	printk("Error %s SPI1 Rx is not in sync, sr1_status(%d) \n",SOC_CHIP_STRING(fe),uData);
	bRxInSync = FALSE;
      }
    }
  } else { /* PRBS to QE */
    if (nSpi == 0) {
      uData = SAND_HAL_READ(qehdl,KA,SR0_STATUS);
      if ((SAND_HAL_GET_FIELD(KA,SR0_STATUS,RX_INSYNC,uData)) != 1) {
	printk("Error %s SPI0 Rx is not in sync, sr0_status(%d)\n",SOC_CHIP_STRING(qe),uData);
	bRxInSync = FALSE;
      }
    } else {
      uData = SAND_HAL_READ(qehdl,KA,SR1_STATUS);
      if ((SAND_HAL_GET_FIELD(KA,SR1_STATUS,RX_INSYNC,uData)) != 1) {
	printk("Error %s SPI1 Rx is not in sync, sr1_status(%d) \n",SOC_CHIP_STRING(qe),uData);
	bRxInSync = FALSE;
      }
    }
  }

  /* if rx is not in sync return */
  if (bRxInSync == FALSE) {
    return(-1);
  }

  /* clear errors */
  uStatus0 = SAND_HAL_READ(fehdl, C2, SR0_PRBS_STATUS0);
  uStatus1 = SAND_HAL_READ(fehdl, C2, SR0_PRBS_STATUS1);
  uStatus2 = SAND_HAL_READ(fehdl, C2, SR0_PRBS_STATUS2);
  uStatus0 = SAND_HAL_READ(fehdl, C2, SR1_PRBS_STATUS0);
  uStatus1 = SAND_HAL_READ(fehdl, C2, SR1_PRBS_STATUS1);
  uStatus2 = SAND_HAL_READ(fehdl, C2, SR1_PRBS_STATUS2);
  uStatus0 = SAND_HAL_READ(qehdl, KA, SR0_PRBS_STATUS0);
  uStatus1 = SAND_HAL_READ(qehdl, KA, SR0_PRBS_STATUS1);
  uStatus2 = SAND_HAL_READ(qehdl, KA, SR0_PRBS_STATUS2);
  uStatus0 = SAND_HAL_READ(qehdl, KA, SR1_PRBS_STATUS0);
  uStatus1 = SAND_HAL_READ(qehdl, KA, SR1_PRBS_STATUS1);
  uStatus2 = SAND_HAL_READ(qehdl, KA, SR1_PRBS_STATUS2);

  /* settings at FE/QE need to match */
  if (prbs_direction == PRBS_TO_FE) {

  SAND_HAL_RMW_FIELD(qehdl, KA, ST0_CONFIG2, PRBS_POLY_SEL, nLSFR);
  SAND_HAL_RMW_FIELD(qehdl, KA, ST0_CONFIG2, PRBS_INVERT, nInvert);
  SAND_HAL_RMW_FIELD(qehdl, KA, ST1_CONFIG2, PRBS_POLY_SEL, nLSFR);
  SAND_HAL_RMW_FIELD(qehdl, KA, ST1_CONFIG2, PRBS_INVERT, nInvert);

  SAND_HAL_RMW_FIELD_STRIDE(fehdl,C2,SR,nSpi,SR_CONFIG1,PRBS_POLY_SEL,nLSFR);
  SAND_HAL_RMW_FIELD_STRIDE(fehdl,C2,SR,nSpi,SR_CONFIG1,PRBS_INVERT,nInvert);

  } else {
    SAND_HAL_RMW_FIELD_STRIDE(fehdl,C2,SR,nSpi,ST0_CONFIG2,PRBS_POLY_SEL,nLSFR);
    SAND_HAL_RMW_FIELD_STRIDE(fehdl,C2,SR,nSpi,ST0_CONFIG2,PRBS_INVERT,nInvert);

    SAND_HAL_RMW_FIELD(qehdl, KA, SR0_CONFIG2, PRBS_POLY_SEL, nLSFR);
    SAND_HAL_RMW_FIELD(qehdl, KA, SR0_CONFIG2, PRBS_INVERT, nInvert);
    SAND_HAL_RMW_FIELD(qehdl, KA, SR1_CONFIG2, PRBS_POLY_SEL, nLSFR);
    SAND_HAL_RMW_FIELD(qehdl, KA, SR1_CONFIG2, PRBS_INVERT, nInvert);
  }

  /* enable the monitors at the reciever */
  if (prbs_direction == PRBS_TO_FE) {
      SAND_HAL_RMW_FIELD_STRIDE(fehdl,C2,SR,nSpi,SR_CONFIG1,PRBS_MONITOR_SYNC,0);
      SAND_HAL_RMW_FIELD_STRIDE(fehdl,C2,SR,nSpi,SR_CONFIG1,PRBS_MONITOR_ENABLE,1);
  } else {
    if (nSpi == 0) {
      SAND_HAL_RMW_FIELD(qehdl,KA,SR0_CONFIG2,PRBS_MONITOR_SYNC,0);
      SAND_HAL_RMW_FIELD(qehdl,KA,SR0_CONFIG2,PRBS_MONITOR_ENABLE,1);
    } else {
      SAND_HAL_RMW_FIELD(qehdl,KA,SR1_CONFIG2,PRBS_MONITOR_SYNC,0);
      SAND_HAL_RMW_FIELD(qehdl,KA,SR1_CONFIG2,PRBS_MONITOR_ENABLE,1);
    }
  }

  /* enable the generators in the spit at the transmitter */
  if (prbs_direction == PRBS_TO_FE) {
    if (nSpi == 0) {
      SAND_HAL_RMW_FIELD(qehdl, KA, ST0_CONFIG2, PRBS_GENERATOR_ENABLE, 1);
    } else {
      SAND_HAL_RMW_FIELD(qehdl, KA, ST1_CONFIG2, PRBS_GENERATOR_ENABLE, 1);
    }
  } else {
    SAND_HAL_RMW_FIELD_STRIDE(fehdl,C2,ST,nSpi,ST_CONFIG2, PRBS_GENERATOR_ENABLE, 1);
  }

  /* re-synchronize the monitor at the reciever */
  if (prbs_direction == PRBS_TO_FE) {
      SAND_HAL_RMW_FIELD_STRIDE(fehdl,C2,SR,nSpi,SR_CONFIG1,PRBS_MONITOR_SYNC,1);
  } else {
    if (nSpi == 0) {
      SAND_HAL_RMW_FIELD(qehdl,KA,SR0_CONFIG2,PRBS_MONITOR_SYNC,1);
    } else {
      SAND_HAL_RMW_FIELD(qehdl,KA,SR1_CONFIG2,PRBS_MONITOR_SYNC,1);
    }
  }

  thin_delay(200000);

  /* dump the error debug registers at the reciever to see what bit failed */
  if (prbs_direction == PRBS_TO_FE) {
    /* check if an error ocurred (hw flagged a spi prbs error) */
    uData = 0;
    uData = SAND_HAL_READ_STRIDE(fehdl,C2,SR,nSpi,SR_PRBS_STATUS0);
    if (nSpi == 0) {
      uField = SAND_HAL_GET_FIELD(C2,SR0_PRBS_STATUS0,PRBS_ERROR_OCCURRED,uData);
    } else {
      uField = SAND_HAL_GET_FIELD(C2,SR1_PRBS_STATUS0,PRBS_ERROR_OCCURRED,uData);
    }
    printk("%s sr%d_prbs_status0 = 0x%x ",SOC_CHIP_STRING(fe),nSpi,uData);
    uField !=0 ? printk(" *prbs_error*\n") : printk("\n");
    /* read the status registers regardless of any error status  */
    uData = SAND_HAL_READ_STRIDE(fehdl,C2,SR,nSpi,SR_PRBS_STATUS1);
      if (uData != 0 && uField != 0) { /* real hw error occurred */
	printk("%s sr%d_prbs_status1 = 0x%x\n",SOC_CHIP_STRING(fe),nSpi,uData);
	status = -1;
	printk("FE SPI%d ERROR SR_PRBS_STATUS1 Mismatch Errors 0x%08x \n",nSpi,uData);
	/* dump the fields for easier debug */
	if (nSpi == 0) {
	  uField = SAND_HAL_GET_FIELD(C2,SR0_PRBS_STATUS1,PRBS_DATA0_ERR_CNT,uData);
	  printk("prbs_data0_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR0_PRBS_STATUS1,PRBS_DATA1_ERR_CNT,uData);
	  printk("prbs_data1_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR0_PRBS_STATUS1,PRBS_DATA2_ERR_CNT,uData);
	  printk("prbs_data2_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR0_PRBS_STATUS1,PRBS_DATA3_ERR_CNT,uData);
	  printk("prbs_data3_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR0_PRBS_STATUS1,PRBS_DATA4_ERR_CNT,uData);
	  printk("prbs_data4_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR0_PRBS_STATUS1,PRBS_DATA5_ERR_CNT,uData);
	  printk("prbs_data5_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR0_PRBS_STATUS1,PRBS_DATA6_ERR_CNT,uData);
	  printk("prbs_data6_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR0_PRBS_STATUS1,PRBS_DATA7_ERR_CNT,uData);
	  printk("prbs_data7_err_cnt=0x%x\n",uField);
	} else {
	  uField = SAND_HAL_GET_FIELD(C2,SR1_PRBS_STATUS1,PRBS_DATA0_ERR_CNT,uData);
	  printk("prbs_data0_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR1_PRBS_STATUS1,PRBS_DATA1_ERR_CNT,uData);
	  printk("prbs_data1_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR1_PRBS_STATUS1,PRBS_DATA2_ERR_CNT,uData);
	  printk("prbs_data2_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR1_PRBS_STATUS1,PRBS_DATA3_ERR_CNT,uData);
	  printk("prbs_data3_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR1_PRBS_STATUS1,PRBS_DATA4_ERR_CNT,uData);
	  printk("prbs_data4_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR1_PRBS_STATUS1,PRBS_DATA5_ERR_CNT,uData);
	  printk("prbs_data5_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR1_PRBS_STATUS1,PRBS_DATA6_ERR_CNT,uData);
	  printk("prbs_data6_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR1_PRBS_STATUS1,PRBS_DATA7_ERR_CNT,uData);
	  printk("prbs_data7_err_cnt=0x%x\n",uField);
	}
      }
      uData = 0;
      uData = SAND_HAL_READ_STRIDE(fehdl,C2,SR,nSpi,SR_PRBS_STATUS2);
      if (uData != 0 && uField != 0) {
	printk("%s sr%d_prbs_status2 = 0x%x\n",SOC_CHIP_STRING(fe),nSpi,uData);
	SB_ERROR("FE SPI%d: ERROR SR_PRBS_STATUS2 Mismatch Errors 0x%08x\n",nSpi, uData);
	status = -1;
	if (nSpi == 0) {
	  uField = SAND_HAL_GET_FIELD(C2,SR0_PRBS_STATUS2,PRBS_DATA8_ERR_CNT,uData);
	  printk("prbs_data8_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR0_PRBS_STATUS2,PRBS_DATA9_ERR_CNT,uData);
	  printk("prbs_data9_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR0_PRBS_STATUS2,PRBS_DATA10_ERR_CNT,uData);
	  printk("prbs_data10_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR0_PRBS_STATUS2,PRBS_DATA11_ERR_CNT,uData);
	  printk("prbs_data11_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR0_PRBS_STATUS2,PRBS_DATA12_ERR_CNT,uData);
	  printk("prbs_data12_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR0_PRBS_STATUS2,PRBS_DATA13_ERR_CNT,uData);
	  printk("prbs_data13_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR0_PRBS_STATUS2,PRBS_DATA14_ERR_CNT,uData);
	  printk("prbs_data14_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR0_PRBS_STATUS2,PRBS_DATA15_ERR_CNT,uData);
	  printk("prbs_data15_err_cnt=0x%x\n",uField);
	} else {
	  uField = SAND_HAL_GET_FIELD(C2,SR1_PRBS_STATUS2,PRBS_DATA8_ERR_CNT,uData);
	  printk("prbs_data8_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR1_PRBS_STATUS2,PRBS_DATA9_ERR_CNT,uData);
	  printk("prbs_data9_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR1_PRBS_STATUS2,PRBS_DATA10_ERR_CNT,uData);
	  printk("prbs_data10_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR1_PRBS_STATUS2,PRBS_DATA11_ERR_CNT,uData);
	  printk("prbs_data11_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR1_PRBS_STATUS2,PRBS_DATA12_ERR_CNT,uData);
	  printk("prbs_data12_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR1_PRBS_STATUS2,PRBS_DATA13_ERR_CNT,uData);
	  printk("prbs_data13_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR1_PRBS_STATUS2,PRBS_DATA14_ERR_CNT,uData);
	  printk("prbs_data14_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(C2,SR1_PRBS_STATUS2,PRBS_DATA15_ERR_CNT,uData);
	  printk("prbs_data15_err_cnt=0x%x\n",uField);
	}
      }
  } else { /* PRBS_TO_QE */
      if (nSpi == 0) {
	uData = SAND_HAL_READ(qehdl,KA,SR0_PRBS_STATUS0);
	uField = SAND_HAL_GET_FIELD(KA,SR0_PRBS_STATUS0,PRBS_ERROR_OCCURRED,uData);
	if (uField != 0) status = -1;
	printk("%s sr%d_prbs_status0 = 0x%x ",SOC_CHIP_STRING(qe),nSpi,uData);
	uField != 0 ? printk(" *prbs_error*\n"): printk("\n");
	uData = SAND_HAL_READ(qehdl,KA,SR0_PRBS_STATUS1);
      } else {
	uData = SAND_HAL_READ(qehdl,KA,SR1_PRBS_STATUS0);
	uField = SAND_HAL_GET_FIELD(KA,SR1_PRBS_STATUS0,PRBS_ERROR_OCCURRED,uData);
	if (uField != 0) status = -1;
	printk("%s sr%d_prbs_status0 = 0x%x ",SOC_CHIP_STRING(qe),nSpi,uData);
	uField != 0 ? printk(" *prbs_error*\n"): printk("\n");
	uData = SAND_HAL_READ(qehdl,KA,SR1_PRBS_STATUS1);
      }
      printk("%s sr%d_prbs_status1 = 0x%x\n",SOC_CHIP_STRING(qe),nSpi,uData);
      if (uData != 0) {
	printk("QE SPI%d ERROR SR_PRBS_STATUS1 Mismatch Errors 0x%08x \n",nSpi,uData);
	status = -1;
	/* dump the fields for easier debug */
	if (nSpi == 0) {
	  uField = SAND_HAL_GET_FIELD(KA,SR0_PRBS_STATUS1,PRBS_DATA0_ERR_CNT,uData);
	  printk("prbs_data0_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR0_PRBS_STATUS1,PRBS_DATA1_ERR_CNT,uData);
	  printk("prbs_data1_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR0_PRBS_STATUS1,PRBS_DATA2_ERR_CNT,uData);
	  printk("prbs_data2_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR0_PRBS_STATUS1,PRBS_DATA3_ERR_CNT,uData);
	  printk("prbs_data3_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR0_PRBS_STATUS1,PRBS_DATA4_ERR_CNT,uData);
	  printk("prbs_data4_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR0_PRBS_STATUS1,PRBS_DATA5_ERR_CNT,uData);
	  printk("prbs_data5_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR0_PRBS_STATUS1,PRBS_DATA6_ERR_CNT,uData);
	  printk("prbs_data6_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR0_PRBS_STATUS1,PRBS_DATA7_ERR_CNT,uData);
	  printk("prbs_data7_err_cnt=0x%x\n",uField);
	} else {
	  uField = SAND_HAL_GET_FIELD(KA,SR1_PRBS_STATUS1,PRBS_DATA0_ERR_CNT,uData);
	  printk("prbs_data0_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR1_PRBS_STATUS1,PRBS_DATA1_ERR_CNT,uData);
	  printk("prbs_data1_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR1_PRBS_STATUS1,PRBS_DATA2_ERR_CNT,uData);
	  printk("prbs_data2_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR1_PRBS_STATUS1,PRBS_DATA3_ERR_CNT,uData);
	  printk("prbs_data3_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR1_PRBS_STATUS1,PRBS_DATA4_ERR_CNT,uData);
	  printk("prbs_data4_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR1_PRBS_STATUS1,PRBS_DATA5_ERR_CNT,uData);
	  printk("prbs_data5_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR1_PRBS_STATUS1,PRBS_DATA6_ERR_CNT,uData);
	  printk("prbs_data6_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR1_PRBS_STATUS1,PRBS_DATA7_ERR_CNT,uData);
	  printk("prbs_data7_err_cnt=0x%x\n",uField);
	}
      }
      uData = 0;
      if (nSpi == 0) {
	uData = SAND_HAL_READ(qehdl,KA,SR0_PRBS_STATUS2);
      } else {
	uData = SAND_HAL_READ(qehdl,KA,SR1_PRBS_STATUS2);
      }
      printk("%s sr%d_prbs_status2 = 0x%x\n",SOC_CHIP_STRING(qe),nSpi,uData);
      if (uData != 0) {
	SB_ERROR("QE SPI%d: ERROR SR_PRBS_STATUS2 Mismatch Errors 0x%08x\n",nSpi, uData);
	status = -1;
	if (nSpi == 0) {
	  uField = SAND_HAL_GET_FIELD(KA,SR0_PRBS_STATUS2,PRBS_DATA8_ERR_CNT,uData);
	  printk("prbs_data8_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR0_PRBS_STATUS2,PRBS_DATA9_ERR_CNT,uData);
	  printk("prbs_data9_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR0_PRBS_STATUS2,PRBS_DATA10_ERR_CNT,uData);
	  printk("prbs_data10_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR0_PRBS_STATUS2,PRBS_DATA11_ERR_CNT,uData);
	  printk("prbs_data11_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR0_PRBS_STATUS2,PRBS_DATA12_ERR_CNT,uData);
	  printk("prbs_data12_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR0_PRBS_STATUS2,PRBS_DATA13_ERR_CNT,uData);
	  printk("prbs_data13_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR0_PRBS_STATUS2,PRBS_DATA14_ERR_CNT,uData);
	  printk("prbs_data14_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR0_PRBS_STATUS2,PRBS_DATA15_ERR_CNT,uData);
	  printk("prbs_data15_err_cnt=0x%x\n",uField);
	} else {
	  uField = SAND_HAL_GET_FIELD(KA,SR1_PRBS_STATUS2,PRBS_DATA8_ERR_CNT,uData);
	  printk("prbs_data8_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR1_PRBS_STATUS2,PRBS_DATA9_ERR_CNT,uData);
	  printk("prbs_data9_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR1_PRBS_STATUS2,PRBS_DATA10_ERR_CNT,uData);
	  printk("prbs_data10_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR1_PRBS_STATUS2,PRBS_DATA11_ERR_CNT,uData);
	  printk("prbs_data11_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR1_PRBS_STATUS2,PRBS_DATA12_ERR_CNT,uData);
	  printk("prbs_data12_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR1_PRBS_STATUS2,PRBS_DATA13_ERR_CNT,uData);
	  printk("prbs_data13_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR1_PRBS_STATUS2,PRBS_DATA14_ERR_CNT,uData);
	  printk("prbs_data14_err_cnt=0x%x\n",uField);
	  uField = SAND_HAL_GET_FIELD(KA,SR1_PRBS_STATUS2,PRBS_DATA15_ERR_CNT,uData);
	  printk("prbs_data15_err_cnt=0x%x\n",uField);
	}
      }
  }

  /* sanity check - attempt for force an error on transmit */
  if (status == 0) {
    if (prbs_direction == PRBS_TO_FE) {
      if (nSpi == 0) { 
	SAND_HAL_RMW_FIELD(qehdl,KA,ST0_CONFIG2,FORCE_PRBS_ERROR,1);
      } else {
	SAND_HAL_RMW_FIELD(qehdl,KA,ST1_CONFIG2,FORCE_PRBS_ERROR,1);
      }

      thin_delay(200000);
      /* check for error at FE */
      if (nSpi == 0) {
	uData = SAND_HAL_READ(fehdl,C2,SR0_PRBS_STATUS0);
	/* soc_cm_print("C2_prbs0_status0=0x%x\n",uData); */
	uField = SAND_HAL_GET_FIELD(C2,SR0_PRBS_STATUS0,PRBS_ERROR_OCCURRED,uData);
	if (uField == 0) {
	  printk("WARN: Could not force error in QE-->FE direction for spi0 interface\n");
	} else {
	  printk("Good force error ok for spi0(%d)\n",uField);
	}
      } else {
	uData = SAND_HAL_READ(fehdl,C2,SR1_PRBS_STATUS0);
	/* soc_cm_print("C2_prbs1_status0=0x%x\n",uData); */
	uField = SAND_HAL_GET_FIELD(C2,SR1_PRBS_STATUS0,PRBS_ERROR_OCCURRED,uData);
	if (uField == 0) {
	  printk("WARN: Could not force error in QE-->FE direction for spi1 interface\n");
	} else {
	  printk("Good force error ok for spi1(%d)\n",uField);
	}
      }
    } else { /* PRBS to QE direction */
      if (nSpi == 0) {
	SAND_HAL_RMW_FIELD(fehdl,C2,ST0_CONFIG2,FORCE_PRBS_ERROR,1);
      } else {
	SAND_HAL_RMW_FIELD(fehdl,C2,ST1_CONFIG2,FORCE_PRBS_ERROR,1);      
      }
      thin_delay(200000);
      /* check for error at QE */
      if (nSpi == 0) {
	uData = SAND_HAL_READ(qehdl,KA,SR0_PRBS_STATUS0);
	uField = SAND_HAL_GET_FIELD(KA,SR0_PRBS_STATUS0,PRBS_ERROR_OCCURRED,uData);
	if (uField == 0) {
	  printk("WARN: Could not force error in FE-->QE direction for spi0 interface\n");
	} else {
	  printk("Good force error ok for spi0(%d)\n",uField);
	}
      } else {
	uData = SAND_HAL_READ(qehdl,KA,SR1_PRBS_STATUS0);
	uField = SAND_HAL_GET_FIELD(KA,SR1_PRBS_STATUS0,PRBS_ERROR_OCCURRED,uData);
	if (uField == 0) {
	  printk("WARN: Could not force error in FE-->QE direction for spi1 interface\n");
	} else {
	  printk("Good force error ok for spi1(%d)\n",uField);
	}
      }
    }
  }

  if (status != 0) {
      printk("FAILED.\n");
  } else {
      printk("PASSED.\n");
  }

  /* cleanup */
  SAND_HAL_RMW_FIELD(fehdl,C2,SR0_CONFIG1,PRBS_MONITOR_ENABLE,0);
  SAND_HAL_RMW_FIELD(fehdl,C2,SR0_CONFIG1,PRBS_MONITOR_SYNC,0);
  SAND_HAL_RMW_FIELD(fehdl,C2,SR1_CONFIG1,PRBS_MONITOR_ENABLE,0);
  SAND_HAL_RMW_FIELD(fehdl,C2,SR1_CONFIG1,PRBS_MONITOR_SYNC,0);

  SAND_HAL_RMW_FIELD(qehdl,KA,SR0_CONFIG2,PRBS_MONITOR_ENABLE,0);
  SAND_HAL_RMW_FIELD(qehdl,KA,SR0_CONFIG2,PRBS_MONITOR_SYNC,0);
  SAND_HAL_RMW_FIELD(qehdl,KA,SR1_CONFIG2,PRBS_MONITOR_ENABLE,0);
  SAND_HAL_RMW_FIELD(qehdl,KA,SR1_CONFIG2,PRBS_MONITOR_SYNC,0);


  SAND_HAL_RMW_FIELD(qehdl, KA, ST0_CONFIG2, PRBS_GENERATOR_ENABLE, 0);
  SAND_HAL_RMW_FIELD(qehdl, KA, ST1_CONFIG2, PRBS_GENERATOR_ENABLE, 0);

  SAND_HAL_RMW_FIELD(fehdl, C2, ST0_CONFIG2, PRBS_GENERATOR_ENABLE, 0);
  SAND_HAL_RMW_FIELD(fehdl, C2, ST1_CONFIG2, PRBS_GENERATOR_ENABLE, 0);

  
  

  DEXIT();
  return status;
}

/* run through all the memory tests */
int sbFe2kxtDiagsSramMemTestAll(sbxDiagsInfo_t *pDiagsInfo) {

  uint32_t mem_test;
  int stat = 0;
  int nMemTests = SB_FE2KXT_MEM_TEST_LAST-1;

  SB_ASSERT(pDiagsInfo);
  for(mem_test=0;mem_test<SB_FE2KXT_MEM_TEST_LAST;mem_test++) {
    pDiagsInfo->e_mem_test = mem_test;
    if (strstr(sbFe2kxtDiagsMemoryTestStr[mem_test],"MM0") != NULL) {
      pDiagsInfo->uInstance = 0;
    } else {
      pDiagsInfo->uInstance = 1;
    }
    printk("Running Test[%2d..%2d] %-45s  .. ",mem_test,nMemTests,sbFe2kxtDiagsMemoryTestStr[mem_test]);
    if(sbFe2kxtDiagsSramMemTest(pDiagsInfo) != 0) {
      stat = -1;
      if (strstr(sbFe2kxtDiagsMemoryTestStr[mem_test],"RANDOM") != NULL) {
	printk("FAILED: To re-run use MemTest=%d Verbose=1 Seed=%d \n", mem_test,pDiagsInfo->seed);
      } else {
	printk("\nFAILED: To re-run use MemTest=%d Verbose=1 \n",mem_test);
      }
    } else {
      printk("PASSED\n");
    }
  }
  return stat;
}



/*
 * All SRAM Memory tests start here
 */

int sbFe2kxtDiagsSramMemTest(sbxDiagsInfo_t *pDiagsInfo)
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
  /* sbFe2kxtUtilReleaseSoftReset(userDeviceHandle); */
  thin_delay(100);

  /* override the memory protection scheme first */
  /* also check if background policer refresh is on we shut it off (test will turn it back on when done)*/
  if (pDiagsInfo->mem_override == 0 ) {
    sbFe2kxtDiagsOverrideMmParam(pDiagsInfo);
    pDiagsInfo->mem_override = 1;
  }

  switch(pDiagsInfo->e_mem_test) {
  case SB_FE2KXT_MM0_DATA_BUS_WALKING_ONES_NARROW_PORT0:
  case SB_FE2KXT_MM1_DATA_BUS_WALKING_ONES_NARROW_PORT0:
    pTest_str = "Walking Ones Data Bus Narrow Port0";
    if (dbgLevel >= 1)
      DISPLAY_TESTSTR(FUNCTION_NAME(),pDiagsInfo->uInstance,pTest_str);
    pDiagsInfo->walkbit=1;
    pDiagsInfo->e_mem_type = SB_FE2KXT_NARROW_PORT_0;
    status = sbFe2kxtDiagsSramDataBusNarrowPortTest(pDiagsInfo);
    break;
  case SB_FE2KXT_MM0_DATA_BUS_WALKING_ZEROS_NARROW_PORT0:
  case SB_FE2KXT_MM1_DATA_BUS_WALKING_ZEROS_NARROW_PORT0:
    pTest_str = "Walking Zeros Data Bus on Narrow Port0 Memory";
    if (dbgLevel >= 1)
      DISPLAY_TESTSTR(FUNCTION_NAME(),pDiagsInfo->uInstance,pTest_str);
    pDiagsInfo->walkbit=0;
    pDiagsInfo->e_mem_type = SB_FE2KXT_NARROW_PORT_0;
    status = sbFe2kxtDiagsSramDataBusNarrowPortTest(pDiagsInfo);
    break;
  case SB_FE2KXT_MM0_ADDRESS_BUS_NARROW_PORT0:
  case SB_FE2KXT_MM1_ADDRESS_BUS_NARROW_PORT0:
    pTest_str = "Address Bus on Narrow Port0 Memory";
    if (dbgLevel >= 1)
      DISPLAY_TESTSTR(FUNCTION_NAME(),pDiagsInfo->uInstance,pTest_str);
    pDiagsInfo->e_mem_type = SB_FE2KXT_NARROW_PORT_0;
    status = sbFe2kxtDiagsSramAddressBusTest(pDiagsInfo);
    break;
  case SB_FE2KXT_MM0_DATA_BUS_WALKING_ONES_NARROW_PORT1:
  case SB_FE2KXT_MM1_DATA_BUS_WALKING_ONES_NARROW_PORT1:
    pTest_str = "Walking Ones Data Bus on Narrow Port1 Memory";
    if (dbgLevel >= 1)
      DISPLAY_TESTSTR(FUNCTION_NAME(),pDiagsInfo->uInstance,pTest_str);
    pDiagsInfo->e_mem_type = SB_FE2KXT_NARROW_PORT_1;
    pDiagsInfo->walkbit = 1;
    status = sbFe2kxtDiagsSramDataBusNarrowPortTest(pDiagsInfo);
    break;
  case SB_FE2KXT_MM0_DATA_BUS_WALKING_ZEROS_NARROW_PORT1:
  case SB_FE2KXT_MM1_DATA_BUS_WALKING_ZEROS_NARROW_PORT1:
    pTest_str = "Walking Zeros Data Bus on Narrow Port1 Memory";
    if (dbgLevel >= 1)
      DISPLAY_TESTSTR(FUNCTION_NAME(),pDiagsInfo->uInstance,pTest_str);
    pDiagsInfo->e_mem_type = SB_FE2KXT_NARROW_PORT_1;
    pDiagsInfo->walkbit = 0;
    status = sbFe2kxtDiagsSramDataBusNarrowPortTest(pDiagsInfo);
    break;
  case SB_FE2KXT_MM0_ADDRESS_BUS_NARROW_PORT1:
  case SB_FE2KXT_MM1_ADDRESS_BUS_NARROW_PORT1:
    pTest_str = "Address Bus on Narrow Port1 Memory";
    if (dbgLevel >= 1)
      DISPLAY_TESTSTR(FUNCTION_NAME(),pDiagsInfo->uInstance,pTest_str);
    pDiagsInfo->e_mem_type = SB_FE2KXT_NARROW_PORT_1;
    status = sbFe2kxtDiagsSramAddressBusTest(pDiagsInfo);
    break;
  case SB_FE2KXT_MM0_DATA_BUS_WALKING_ONES_WIDE_PORT:
  case SB_FE2KXT_MM1_DATA_BUS_WALKING_ONES_WIDE_PORT:
    pTest_str = "Walking Ones Data Bus on Wide Port Memory";
    if (dbgLevel >= 1)
      DISPLAY_TESTSTR(FUNCTION_NAME(),pDiagsInfo->uInstance,pTest_str);
    pDiagsInfo->walkbit=1;
    status = sbFe2kxtDiagsSramDataBusWidePortTest(pDiagsInfo);
    break;
  case SB_FE2KXT_MM0_DATA_BUS_WALKING_ZEROS_WIDE_PORT:
  case SB_FE2KXT_MM1_DATA_BUS_WALKING_ZEROS_WIDE_PORT:
    pTest_str = "Walking Zeros Data Bus on Wide Port Memory";
    if (dbgLevel >= 1)
      DISPLAY_TESTSTR(FUNCTION_NAME(),pDiagsInfo->uInstance,pTest_str);
    pDiagsInfo->walkbit=0;
    status = sbFe2kxtDiagsSramDataBusWidePortTest(pDiagsInfo);
    break;
  case SB_FE2KXT_MM0_ADDRESS_BUS_WIDE_PORT:
  case SB_FE2KXT_MM1_ADDRESS_BUS_WIDE_PORT:
    pTest_str = "Address Bus on Wide Port Memory";
    if (dbgLevel >= 1)
      DISPLAY_TESTSTR(FUNCTION_NAME(),pDiagsInfo->uInstance,pTest_str);
    pDiagsInfo->e_mem_type = SB_FE2KXT_WIDE_PORT;
    status = sbFe2kxtDiagsSramAddressBusTest(pDiagsInfo);
    break;
  case SB_FE2KXT_MM0_RAND_NARROW_PORT0:
  case SB_FE2KXT_MM1_RAND_NARROW_PORT0:
    pTest_str = "Random data write/read/verify test on narrow port0";
    if (dbgLevel >= 1)
      DISPLAY_TESTSTR(FUNCTION_NAME(),pDiagsInfo->uInstance,pTest_str);
    pDiagsInfo->e_mem_type = SB_FE2KXT_NARROW_PORT_0;
    status = sbFe2kxtDiagsRandomDataMemTest(pDiagsInfo);
    break;
  case SB_FE2KXT_MM0_RAND_NARROW_PORT1:
  case SB_FE2KXT_MM1_RAND_NARROW_PORT1:
    pTest_str = "Random data write/read/verify test on narrow port1";
    if (dbgLevel >= 1)
      DISPLAY_TESTSTR(FUNCTION_NAME(),pDiagsInfo->uInstance,pTest_str);
    pDiagsInfo->e_mem_type = SB_FE2KXT_NARROW_PORT_1;
    status = sbFe2kxtDiagsRandomDataMemTest(pDiagsInfo);
    break;
  case SB_FE2KXT_MM0_RAND_WIDE_PORT:
  case SB_FE2KXT_MM1_RAND_WIDE_PORT:
    pTest_str = "Random data write/read/verify test on wide port";
    if (dbgLevel >= 1)
      DISPLAY_TESTSTR(FUNCTION_NAME(),pDiagsInfo->uInstance,pTest_str);
    pDiagsInfo->e_mem_type = SB_FE2KXT_WIDE_PORT;
    status = sbFe2kxtDiagsRandomDataMemTest(pDiagsInfo);
    break;
  default:
    SB_ERROR("ERROR:\n%s: Invalid mem test:%d [0-%d supported]\n",FUNCTION_NAME(),
	     pDiagsInfo->e_mem_test,SB_FE2KXT_MEM_TEST_LAST-1);
    return -1;
  }

  DEXIT();
  return status;
}

int sbFe2kxtDiagsSramDataBusNarrowPortTest(sbxDiagsInfo_t *pDiagsInfo)

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
  if (sbFe2kxtDiagsAddressCheck(pDiagsInfo) != 0) {
    return SB_FE2KXT_BAD_SIZE_E;
  }

  if (pDiagsInfo->e_mem_type == SB_FE2KXT_NARROW_PORT_0) {
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
      printk("%s Writing:0x%llx to address:0x%08lx\n",FUNCTION_NAME(),ullData_wr,address_ul);
    }

  loopOnWriteError:
    status = sbFe2kxtDiagsMemWrite(userDeviceHandle,uInstance,address_ul,ullData_wr,pDiagsInfo->e_mem_type);
    if (status != SB_FE2KXT_SUCCESS_E ) {
      return status;
    }

    /* read to check the data */
    thin_delay(1000); /* allow the data bus to settle down to avoid false postives */

  loopOnReadError:
    status = sbFe2kxtDiagsMemRead(userDeviceHandle,uInstance,address_ul,&ullData_rd,pDiagsInfo->e_mem_type);
    if (status != SB_FE2KXT_SUCCESS_E ) {
      if (pDiagsInfo->bLoopOnError) {
	printk("Read Timeout MM%d,MEM_TYPE:%s, retry attempt %d .. -- ctrl-c to stop\r",
	       uInstance,sbFe2kxtDiagsMemoryStr[pDiagsInfo->e_mem_type],++Retry);
	pDiagsInfo->debug_level = 1;
	goto loopOnReadError;
      }
      return status;
    }

    /* check the data */
    if (ullData_rd != ullData_wr) {
      if (pDiagsInfo->bLoopOnError) {
	SB_ERROR("\r%s: MM%d Data mismatch: write data 0x%llx read data 0x%llx at address: 0x%08lx --- looping on error ctrl-c to stop", FUNCTION_NAME(),uInstance,ullData_wr,ullData_rd,address_ul);
	pDiagsInfo->debug_level = 1;
	goto loopOnWriteError;
      } else {
	SB_ERROR("\nERROR:%s: MM%d Data mismatch: write data 0x%llx read data 0x%llx at address: 0x%08lx", FUNCTION_NAME(),uInstance,ullData_wr,ullData_rd,address_ul);
      }
      tst_status = -1;
    } else {
      if (pDiagsInfo->debug_level >= 1) {
	printk("%s: GOOD Got:0x%llx Exp:0x%llx\n",FUNCTION_NAME(),ullData_rd,ullData_wr);
      }
    }
  }

  DEXIT();
  return tst_status;
}

int sbFe2kxtDiagsSramAddressBusTest(sbxDiagsInfo_t *pDiagsInfo)
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
      if (pDiagsInfo->e_mem_type == SB_FE2KXT_NARROW_PORT_0) {
	  ulEndAddr = (1<<pDiagsInfo->np0_config.addr_width);
      } else if (pDiagsInfo->e_mem_type == SB_FE2KXT_NARROW_PORT_1) {
	  ulEndAddr = (1<<pDiagsInfo->np1_config.addr_width);
      } else {
	  ulEndAddr = (1<<pDiagsInfo->wp_config.addr_width);
      }
  } else {
      ulEndAddr = pDiagsInfo->end_addr;
  }


  /* sanity check */
  if (sbFe2kxtDiagsAddressCheck(pDiagsInfo) != 0) {
    return SB_FE2KXT_BAD_SIZE_E;
  }

  if (mem == SB_FE2KXT_WIDE_PORT) {
    g_pattern = g_Fe2kxtWidePortpattern;
    g_antipattern = g_Fe2kxtWidePortantipattern;
  } else {
    g_pattern = g_Fe2kxtNarrowPortpattern;
    g_antipattern = g_Fe2kxtNarrowPortantipattern;
  }

  /* Initialize memory */
  if (pDiagsInfo->mem_inited[mmu][mem] == 0) {
    if(sbFe2kxtDiagsInitMem(pDiagsInfo) != 0) {
      SB_ERROR("ERROR:%s: mem init:%s mmu%d failed\n",FUNCTION_NAME(),
	       sbFe2kxtDiagsMemoryStr[mem],mmu);
      return (-1);
    }
    pDiagsInfo->mem_inited[mmu][mem] = 1;
  }


  /* Check for address bits stuck high
   * Write the inverted pattern to the first offset, then check all other power-of-2 addresses to be
   * sure the original pattern exists, that should be there when the memory was intialized
   */

 loopOnError0:
  status = sbFe2kxtDiagsMemWrite(userDeviceHandle,mmu,ulStartAddr,g_antipattern,mem);
  if (status != SB_FE2KXT_SUCCESS_E) {
    if (pDiagsInfo->bLoopOnError) {
      printk("Write timeout, trying again .. ctrl-c to stop\r");
      goto loopOnError0;
    }
    return status;
  }


  /* all the memory should have a data pattern of g_pattern, except the starting address, which
   * was written with the inverted data we check that writing the inverted data to the starting address
   * does not inadvently write that data to another memory address */

  for(ulWalkAddr = ulStartAddr,offset=0;ulWalkAddr<ulEndAddr;offset++) {
  loopOnError1:
    status = sbFe2kxtDiagsMemRead(userDeviceHandle,mmu,ulWalkAddr,&ullData_rd,mem);
    if (status != SB_FE2KXT_SUCCESS_E) {
      if (pDiagsInfo->bLoopOnError) {
	printk("Read timeout, trying again .. ctrl-c to stop\r");
	goto loopOnError1;
      }
      return status;
    }

    if (ulWalkAddr == ulStartAddr) { /* starting address has inverted data */
      if (ullData_rd != g_antipattern) {
	if (pDiagsInfo->bLoopOnError) {
	  SB_ERROR("\r%s:%d MM%d Data mismatch: exp 0x%llx got 0x%llx at address: 0x%lx -- looping on error ctrl-c to stop", FUNCTION_NAME(),__LINE__,
		   mmu,g_antipattern,ullData_rd,ulStartAddr);
	  pDiagsInfo->debug_level=1;
	  goto loopOnError0;
	} else {
	  SB_ERROR("\nERROR:%s:%d MM%d Data mismatch: exp 0x%llx got 0x%llx at address: 0x%lx\n", FUNCTION_NAME(),__LINE__,
		   mmu,g_antipattern,ullData_rd,ulStartAddr);
	  return (-1);
	}
      } else { /* for debug */
	if (pDiagsInfo->debug_level >=1 ) {
	  printk("%s:%d GOOD MM%d Got:0x%llx Exp:0x%llx at address: 0x%lx\n",FUNCTION_NAME(),__LINE__,
		 mmu,g_antipattern,ullData_rd,ulStartAddr);
	}

      }
    } else {
      if (ullData_rd != g_pattern ) { /* all other memory locations should have g_pattern still */
	if (pDiagsInfo->bLoopOnError) {
	  SB_ERROR("\r%s:%d MM%d Data mismatch: exp 0x%llx got 0x%llx at address: 0x%lx -- looping on error ctrl-c to stop", FUNCTION_NAME(),__LINE__,
		   mmu,g_pattern,ullData_rd,ulWalkAddr);
	  pDiagsInfo->debug_level=1;
	  goto loopOnError1;
	} else {
	  SB_ERROR("\nERROR:%s:%d MM%d Data mismatch: exp 0x%llx got 0x%llx at address: 0x%lx", FUNCTION_NAME(),__LINE__,
		   mmu,g_pattern,ullData_rd,ulWalkAddr);
	  /* return (-1); */
	}
      } else { /* for debug */
	if (pDiagsInfo->debug_level >=1 ) {
	  printk("%s:%d GOOD MM%d Got:0x%llx Exp:0x%llx at address: 0x%lx\n",FUNCTION_NAME(),__LINE__,
		 mmu,g_pattern,ullData_rd,ulWalkAddr);
	}
      }
    }

    /* go to the next power-of-2 address, this is equivalent of walking ones on the address bus */
    ulWalkAddr = ulStartAddr + (1<<offset);
  }

  /* put the original pattern back at the starting address */
  status = sbFe2kxtDiagsMemWrite(userDeviceHandle,mmu,ulStartAddr,g_pattern,mem);
  if (status != SB_FE2KXT_SUCCESS_E) {
    return status;
  }

  /* Check for address bits stuck low or shorted -- see header file for description */
  for(ulAddrMover = ulStartAddr,outer=0;ulAddrMover < ulEndAddr;outer++) {
    loopOnError2:
    status = sbFe2kxtDiagsMemWrite(userDeviceHandle,mmu,ulAddrMover,g_antipattern,mem);
    if (status != SB_FE2KXT_SUCCESS_E) {
      return status;
    }

    for(ulWalkAddr = ulStartAddr,offset=0;ulWalkAddr < ulEndAddr;offset++) {
      status = sbFe2kxtDiagsMemRead(userDeviceHandle,mmu,ulWalkAddr,&ullData_rd,mem);
      if (status != SB_FE2KXT_SUCCESS_E) {
	return status;
      }
      /* check the data */
      if (ulWalkAddr != ulAddrMover) {
	/* data here should be g_pattern */
	if (ullData_rd != g_pattern) {
	  if (pDiagsInfo->bLoopOnError) {
	    SB_ERROR("\r%s:%d MM%d Data mismatch: exp 0x%llx got 0x%llx at address: 0x%lx --looping on error ctrl-c to stop", FUNCTION_NAME(),__LINE__,
		     mmu,g_pattern,ullData_rd,ulWalkAddr);
	    pDiagsInfo->debug_level=1;
	    goto loopOnError2;
	  } else {
	    SB_ERROR("\nERROR:%s:%d MM%d Data mismatch: exp 0x%llx got 0x%llx at address: 0x%lx", FUNCTION_NAME(),__LINE__,
		     mmu,g_pattern,ullData_rd,ulWalkAddr);
	    return (-1);
	  }
	} else {
	  if (pDiagsInfo->debug_level >=1 ) {
	  printk("%s:%d GOOD MM%d Got:0x%llx Exp:0x%llx at address: 0x%lx\n",FUNCTION_NAME(),__LINE__,
		 mmu,g_pattern,ullData_rd,ulWalkAddr);
	  }
	}
      } else {
	/* data here should be the inverted data */
	if (ullData_rd != g_antipattern) {
	  if (pDiagsInfo->bLoopOnError) {
	    SB_ERROR("\r%s:%d MM%d Data mismatch: exp 0x%llx got 0x%llx at address: 0x%lx --looping on error ctrl-c to stop", FUNCTION_NAME(),__LINE__,
		     mmu,g_antipattern,ullData_rd,ulWalkAddr);
	    pDiagsInfo->debug_level = 1;
	    goto loopOnError2;
	  } else {
	    SB_ERROR("\nERROR:%s:%d MM%d Data mismatch: exp 0x%llx got 0x%llx at address: 0x%lx", FUNCTION_NAME(),__LINE__,
		     mmu,g_antipattern,ullData_rd,ulWalkAddr);
	    return (-1);
	  }
	} else {
	  if (pDiagsInfo->debug_level >= 1) {
	    printk("%s:%d GOOD MM%d Got:0x%llx Exp:0x%llx at address: 0x%lx\n",FUNCTION_NAME(),__LINE__,
		   mmu,g_antipattern,ullData_rd,ulWalkAddr);
	  }
	}
      }
      /* walk to the next power-of-2 address */
      ulWalkAddr = ulStartAddr + (1<<offset);
    }

    /* now reset this offset with the original g_pattern */
    status = sbFe2kxtDiagsMemWrite(userDeviceHandle,mmu,ulAddrMover,g_pattern,mem);
    if (status != SB_FE2KXT_SUCCESS_E) {
      return status;
    }

    /* move the ulAddrMover to the next address */
    ulAddrMover = ulStartAddr + (1<<outer);
  }

  DEXIT();
  return 0;
}

int sbFe2kxtDiagsSramDataBusWidePortTest(sbxDiagsInfo_t *pDiagsInfo)
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
    printk("Loop on error not supported for this test.\n");
    return(-1);
  }

  /* sanity check */
  if (sbFe2kxtDiagsAddressCheck(pDiagsInfo) != 0) {
    return SB_FE2KXT_BAD_SIZE_E;
  }

  for(i=0;i<pDiagsInfo->wp_config.data_width;i++) {
      if (walk_bit == 1) {
	  ullData_wr = ullPattern;
      } else {
	  ullData_wr = ~ullPattern & 0xfffffffffLL;
      }

      ullPattern<<=1;

      /* write the data */
      status = sbFe2kxtDiagsMemWrite(userDeviceHandle,uInstance,address_ul,ullData_wr,SB_FE2KXT_WIDE_PORT);
      if (status != SB_FE2KXT_SUCCESS_E ) {
	  return status;
      }

      /* read to check the data */
      thin_delay(1000); /* allow the data bus to settle down to avoid false postives */

      status = sbFe2kxtDiagsMemRead(userDeviceHandle,uInstance,address_ul,&ullData_rd,SB_FE2KXT_WIDE_PORT);
      if (status != SB_FE2KXT_SUCCESS_E ) {
	  return status;
      }

      /* check the data */
      if (ullData_rd != ullData_wr) {
	  SB_ERROR("\nERROR:%s: MM%d Data mismatch: write data 0x%llx read data 0x%llx at address: 0x%08lx", FUNCTION_NAME(),uInstance,ullData_wr,ullData_rd,address_ul);
	  tst_status = -1;
      } else {
	  if (pDiagsInfo->debug_level >= 1) {
	      printk("%s: GOOD MM%d Got: 0x%llx Exp: 0x%llx at address: 0x%08lx\n", FUNCTION_NAME(),uInstance,ullData_wr,ullData_rd,address_ul);
	  }
      }
  }
  DEXIT();
  return tst_status;
}

char *Fe2kxt_memTestD[] = {"Initializing memory","Read/Verify/Write","Read/Verify"};
int sbFe2kxtDiagsRandomDataMemTest(sbxDiagsInfo_t *pDiagsInfo)

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

  if (sbFe2kxtDiagsAddressCheck(pDiagsInfo) != 0) {
    return SB_FE2KXT_BAD_SIZE_E;
  }

  if (pDiagsInfo->bLoopOnError) {
    printk("Loop on error not supported for this test.\n");
    return (-1);
  }

  ulStartAddr = pDiagsInfo->start_addr; /* default==0*/
  /* If an ending address was specified, use that otherwise assume maximum */
  if (!pDiagsInfo->bEndAddr) {
      if (pDiagsInfo->e_mem_type == SB_FE2KXT_NARROW_PORT_0) {
	  ulEndAddr = (1<<pDiagsInfo->np0_config.addr_width);
      } else if (pDiagsInfo->e_mem_type == SB_FE2KXT_NARROW_PORT_1) {
	  ulEndAddr = (1<<pDiagsInfo->np1_config.addr_width);
      } else {
	  ulEndAddr = (1<<pDiagsInfo->wp_config.addr_width);
      }
  } else {
      ulEndAddr = pDiagsInfo->end_addr;
  }


  DENTER();

  /* be sure memtype to be tested exists */
  SB_ASSERT(mem < SB_FE2KXT_MEM_TYPE_LAST);
  if (mem == SB_FE2KXT_WIDE_PORT) {
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
    printk("\n\n%s:Random Memory read/write test using seed=%d\n",FUNCTION_NAME(),pDiagsInfo->seed);
#ifndef __KERNEL__
  sal_srand(seed);   /* this will seed the random number generator */
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
    printk("\n%s,Testing start_addr=0x%08lx, ulEndAddr=0x%08lx, init data=0x%llx \n",FUNCTION_NAME(),
	 ulStartAddr,ulEndAddr,ullExpData);

  /* Iterate from starting address to ending address for each phase. */
  for(uPhase=0;uPhase<3;uPhase++) {
    if (dbgLevel >=1 )
      printk("%s, PHASE:%d %s ...\n",FUNCTION_NAME(),uPhase,Fe2kxt_memTestD[uPhase]);
    for(uOffset = ulStartAddr; uOffset < ulEndAddr; uOffset++) {
      if (uPhase == 0) {
	if (dbgLevel >= 2) {
	  printk("%s:Writing to offset=0x%08x with data:0x%llx\n",FUNCTION_NAME(),uOffset,ullExpData);
	}
	if (sbFe2kxtDiagsMemWrite(userDeviceHandle, mmu,uOffset, ullExpData,
				  mem) != SB_FE2KXT_SUCCESS_E) {
	  return SB_FE2KXT_MEM_ACC_WRITE_TIMEOUT_E;
	}
      } else if (uPhase == 1) {
	if (sbFe2kxtDiagsMemRead(userDeviceHandle, mmu,uOffset, &uActData,
				 mem) != SB_FE2KXT_SUCCESS_E) {
	  return SB_FE2KXT_MEM_ACC_READ_TIMEOUT_E;
	} else {
	  /* compare actual vs expected */
	  if ( uActData == ullExpData) {
	    if (dbgLevel >= 1) {
	      printk("%s:GOOD MM%d, MemType:%s Act=0x%llx Exp:0x%llx at Addr:0x%08x\n",FUNCTION_NAME(),
		     mmu,sbFe2kxtDiagsMemoryStr[mem],uActData,ullExpData,uOffset);
	    }
	    /* data ok write back the inverted value to this location */
	    if (sbFe2kxtDiagsMemWrite(userDeviceHandle, mmu, uOffset,ullExpDataInv,
				      mem) != SB_FE2KXT_SUCCESS_E) {
	      return SB_FE2KXT_MEM_ACC_WRITE_TIMEOUT_E;
	    }
	  } else {
	    SB_ERROR("\nERROR:%s:%d, MM%d, MemType:%s Exp:0x%llx, Got:0x%llx, at address:0x%08x\n",FUNCTION_NAME(),__LINE__,
		     mmu,sbFe2kxtDiagsMemoryStr[mem],ullExpData,uActData,uOffset);
	    return (-1);
	  }
	}
      } else if (uPhase == 2) {
	if (sbFe2kxtDiagsMemRead(userDeviceHandle, mmu, uOffset, &uActData,
				 mem) != SB_FE2KXT_SUCCESS_E) {
	  return SB_FE2KXT_MEM_ACC_READ_TIMEOUT_E;
	} else {
	  /* compare actual vs expected, actual should be data inverted */
	  if ( uActData != ullExpDataInv) {
	    SB_ERROR("ERROR:%s:%d, MM%d, MemType:%s Exp:0x%llx, Got:0x%llx, at address:0x%08x\n",FUNCTION_NAME(),__LINE__,
		     mmu,sbFe2kxtDiagsMemoryStr[mem],ullExpDataInv,uActData,uOffset);
	    return (-1);
	  } else if (dbgLevel >=1) {
	    printk("%s:GOOD MM%d, MemType:%s Act=0x%llx Exp:0x%llx at Addr:0x%08x\n",FUNCTION_NAME(),
		   mmu,sbFe2kxtDiagsMemoryStr[mem],uActData,ullExpDataInv,uOffset);
	  }
	}
      }
    }
  }
  DEXIT();
  return 0;
}


int sbFe2kxtDiagsMemRead(sbhandle userDeviceHandle,
			 uint32_t uInstance,
			 unsigned long addr,
			 volatile uint64_t *pData,
			 sbFe2kxtDiagsMemType_t e_mem_type) {

  switch(e_mem_type) {
  case SB_FE2KXT_NARROW_PORT_0:

    {

      uint32_t offset,ack_mask;
      volatile uint64_t tmp=0;
      /* initiate the read request */
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_CTRL,RD_WR_N,1); /* 1 indicates a read */
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_CTRL,ADDRESS,addr);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_CTRL,ACK,1);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_CTRL,REQ,1);
      /* wait for the read op to complete */
      offset = SAND_HAL_REG_OFFSET_STRIDE(C2,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_CTRL);
      ack_mask = ( uInstance == 1 ) ? SAND_HAL_C2_MM1_NARROW_PORT0_MEM_ACC_CTRL_ACK_MASK : SAND_HAL_C2_MM0_NARROW_PORT0_MEM_ACC_CTRL_ACK_MASK;
      if (sbFe2kxtDiagsWrRdComplete(userDeviceHandle,
				    offset,
				    ack_mask,
				    SB_FE2KXT_MEM_TIMEOUT) != TRUE) {
	SB_ERROR("\nERROR:%s: Read timeout, reading from MM%d narrow port0 address:0x%08lx\n",FUNCTION_NAME(),uInstance,addr);
	return (SB_FE2KXT_MEM_ACC_READ_TIMEOUT_E);
      }
      /* get the first 32bits of 36 bits data */
      *pData = SAND_HAL_READ_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_DATA0);
      /* get the remaining 4 bits of 36 bits of data */
      tmp = SAND_HAL_READ_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_DATA1) & 0xf;
      *pData |= tmp<<32;
    }
    break;

  case SB_FE2KXT_NARROW_PORT_1:

    {
      uint32_t offset,ack_mask;
      volatile uint64_t tmp=0;
      /* initiate the read request */
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_CTRL,RD_WR_N,1);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_CTRL,ADDRESS,addr);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_CTRL,ACK,1);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_CTRL,REQ,1);
      /* wait for the read op to complete */
      offset = SAND_HAL_REG_OFFSET_STRIDE(C2,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_CTRL);
      ack_mask = ( uInstance == 1 ) ? SAND_HAL_C2_MM1_NARROW_PORT1_MEM_ACC_CTRL_ACK_MASK : SAND_HAL_C2_MM0_NARROW_PORT1_MEM_ACC_CTRL_ACK_MASK;
      if (sbFe2kxtDiagsWrRdComplete(userDeviceHandle,
				    offset,
				    ack_mask,
				    SB_FE2KXT_MEM_TIMEOUT) != TRUE) {
	SB_ERROR("\nERROR:%s: Read timeout, reading from MM%d narrow port1 address:0x%08lx\n",FUNCTION_NAME(),uInstance,addr);
	return (SB_FE2KXT_MEM_ACC_READ_TIMEOUT_E);
      }

      /* get the first 32bits of 36 bits data */
      *pData = SAND_HAL_READ_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_DATA0);
      tmp = SAND_HAL_READ_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_DATA1) & 0xf;
      /* get the remaining 4 bits of 36 bits of data */
      *pData |= tmp<<32;
    }
    break;

  case SB_FE2KXT_WIDE_PORT:

    {
      uint32_t offset,ack_mask;
      volatile uint64_t tmp=0;
      /* initiate the read request */
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_WIDE_PORT_MEM_ACC_CTRL,RD_WR_N,1);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_WIDE_PORT_MEM_ACC_CTRL,ADDRESS,addr);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_WIDE_PORT_MEM_ACC_CTRL,ACK,1);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_WIDE_PORT_MEM_ACC_CTRL,REQ,1);

      /* wait for the read op to complete */
      offset = SAND_HAL_REG_OFFSET_STRIDE(C2,MM,uInstance,MM_WIDE_PORT_MEM_ACC_CTRL);
      ack_mask = ( uInstance == 1 ) ? SAND_HAL_C2_MM1_WIDE_PORT_MEM_ACC_CTRL_ACK_MASK : SAND_HAL_C2_MM0_WIDE_PORT_MEM_ACC_CTRL_ACK_MASK;
      if (sbFe2kxtDiagsWrRdComplete(userDeviceHandle,
				    offset,
				    ack_mask,
				    SB_FE2KXT_MEM_TIMEOUT) != TRUE) {
	SB_ERROR("\nERROR:%s: Read timeout, reading from MM%d wide port address:0x%08lx\n",FUNCTION_NAME(),uInstance,addr);
	return (SB_FE2KXT_MEM_ACC_READ_TIMEOUT_E);
      }

      /* get the first 32bits of 64 bits of data */
      *pData = SAND_HAL_READ_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_WIDE_PORT_MEM_ACC_DATA0);
      tmp = SAND_HAL_READ_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_WIDE_PORT_MEM_ACC_DATA1);
      /* get the remaining 32 bits of 64 bits of data */
      *pData |= (tmp << 32);
    }
    break;

  default:
    SB_ERROR("ERROR:%s: Bad read request to memory type:%d\n",FUNCTION_NAME(),e_mem_type);
    SB_ASSERT( 0 );
  }

  return SB_FE2KXT_SUCCESS_E;

}

int sbFe2kxtDiagsMemWrite(sbhandle userDeviceHandle,
			  uint32_t uInstance,
			  unsigned long addr,
			  const uint64_t ullData ,
			  sbFe2kxtDiagsMemType_t e_mem_type) {

  switch(e_mem_type) {
  case SB_FE2KXT_NARROW_PORT_0:

    {
      uint32_t offset,ack_mask;
      SAND_HAL_WRITE_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_DATA0,ullData&MASK_LSW);    /* Least significant 32 bits of 36 bit data to be written */
      SAND_HAL_WRITE_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_DATA1,(ullData>>32)&0xf);   /* Most significant 4 bits of 36 bit data to be written */
      /* initiate the write request */
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_CTRL,RD_WR_N,0);        /* 0 indicates a write */
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_CTRL,ADDRESS,addr);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_CTRL,ACK,1);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_CTRL,REQ,1);
      /* wait for the write op to complete */
      offset = SAND_HAL_REG_OFFSET_STRIDE(C2,MM,uInstance,MM_NARROW_PORT0_MEM_ACC_CTRL);
      ack_mask = ( uInstance == 1 ) ? SAND_HAL_C2_MM1_NARROW_PORT0_MEM_ACC_CTRL_ACK_MASK : SAND_HAL_C2_MM0_NARROW_PORT0_MEM_ACC_CTRL_ACK_MASK;
      if (sbFe2kxtDiagsWrRdComplete(userDeviceHandle,
				    offset,
				    ack_mask,
				    SB_FE2KXT_MEM_TIMEOUT) != TRUE) {
	SB_ERROR("\nERROR:%s: Write timeout, writing data=0x%llx, to MM%d narrow port0 memory address:0x%08lx\n",FUNCTION_NAME(),ullData,uInstance,addr);
	return (SB_FE2KXT_MEM_ACC_WRITE_TIMEOUT_E);
      }
    }
    break;
  case SB_FE2KXT_NARROW_PORT_1:

    {
      uint32_t offset,ack_mask;
      SAND_HAL_WRITE_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_DATA0,ullData&MASK_LSW);
      SAND_HAL_WRITE_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_DATA1,(ullData>>32)&0xf);
      /* initiate the write request */
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_CTRL,RD_WR_N,0);        /* 0 indicates a write */
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_CTRL,ADDRESS,addr);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_CTRL,ACK,1);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_CTRL,REQ,1);
      /* wait for the write op to complete */
      offset = SAND_HAL_REG_OFFSET_STRIDE(C2,MM,uInstance,MM_NARROW_PORT1_MEM_ACC_CTRL);
      ack_mask = ( uInstance == 1 ) ? SAND_HAL_C2_MM1_NARROW_PORT1_MEM_ACC_CTRL_ACK_MASK : SAND_HAL_C2_MM0_NARROW_PORT1_MEM_ACC_CTRL_ACK_MASK;
      if (sbFe2kxtDiagsWrRdComplete(userDeviceHandle,
				    offset,
				    ack_mask,
				    SB_FE2KXT_MEM_TIMEOUT) != TRUE) {
	SB_ERROR("\nERROR:%s: Write timeout, writing data=0x%llx, to MM%d narrow port1 memory address:0x%08lx\n",FUNCTION_NAME(),ullData,uInstance,addr);
	return (SB_FE2KXT_MEM_ACC_WRITE_TIMEOUT_E);
      }
    }
    break;
  case SB_FE2KXT_WIDE_PORT:
    {
      uint32_t offset,ack_mask;
      SAND_HAL_WRITE_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_WIDE_PORT_MEM_ACC_DATA0,ullData&MASK_LSW);    /* least significant 32 bits of 64 bit data */
      SAND_HAL_WRITE_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_WIDE_PORT_MEM_ACC_DATA1,(ullData>>32));       /* most significant 32 bits of 64 bit data */
      /* initiate the write request */
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_WIDE_PORT_MEM_ACC_CTRL,RD_WR_N,0);        /* 0 indicates a write */
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_WIDE_PORT_MEM_ACC_CTRL,ADDRESS,addr);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_WIDE_PORT_MEM_ACC_CTRL,ACK,1);
      SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_WIDE_PORT_MEM_ACC_CTRL,REQ,1);
      /* wait for the write op to complete */
      offset = SAND_HAL_REG_OFFSET_STRIDE(C2,MM,uInstance,MM_WIDE_PORT_MEM_ACC_CTRL);
      ack_mask = ( uInstance == 1 ) ? SAND_HAL_C2_MM1_WIDE_PORT_MEM_ACC_CTRL_ACK_MASK : SAND_HAL_C2_MM0_WIDE_PORT_MEM_ACC_CTRL_ACK_MASK;
      if (sbFe2kxtDiagsWrRdComplete(userDeviceHandle,
				    offset,
				    ack_mask,
				    SB_FE2KXT_MEM_TIMEOUT) != TRUE) {
	SB_ERROR("\nERROR:%s: Write timeout, writing data=0x%llx, to MM%d wide port memory address:0x%08lx\n",FUNCTION_NAME(),ullData,uInstance,addr);
	return (SB_FE2KXT_MEM_ACC_WRITE_TIMEOUT_E);
      }
    }
    break;
  default:
    SB_ERROR("ERROR:%s: Bad write request to memory type:%d\n",FUNCTION_NAME(),e_mem_type);
    SB_ASSERT( 0 );
  }

  return SB_FE2KXT_SUCCESS_E;
}

/* Intialize memory with a default pattern, for diagnostics testing */
int sbFe2kxtDiagsInitMem(sbxDiagsInfo_t *pDiagsInfo) {

  unsigned long addr;
  int stat;
  int mmu = pDiagsInfo->uInstance;
  sbhandle userDeviceHandle = pDiagsInfo->userDeviceHandle;

  DENTER();
  switch (pDiagsInfo->e_mem_type)
    {

    case SB_FE2KXT_NARROW_PORT_0:
      if (pDiagsInfo->debug_level) {
	printk("%s: Initializing MM%d Narrow Port0 Memory..\n",FUNCTION_NAME(),mmu);
      }
    for(addr=0;addr < 1<<pDiagsInfo->np0_config.addr_width;addr++) {
      stat = sbFe2kxtDiagsMemWrite(userDeviceHandle,mmu,addr,g_Fe2kxtNarrowPortpattern,SB_FE2KXT_NARROW_PORT_0);
      if (stat != SB_FE2KXT_SUCCESS_E) {
	return stat;
      }
    }
    break;

    case SB_FE2KXT_NARROW_PORT_1:
      if (pDiagsInfo->debug_level) {
	printk("%s: Initializing MM%d Narrow Port1 Memory..\n",FUNCTION_NAME(),mmu);
      }
      for(addr=0;addr < 1<<pDiagsInfo->np1_config.addr_width;addr++) {
	stat = sbFe2kxtDiagsMemWrite(userDeviceHandle,mmu,addr,g_Fe2kxtNarrowPortpattern,SB_FE2KXT_NARROW_PORT_1);
	if (stat != SB_FE2KXT_SUCCESS_E) {
	  return stat;
	}
      }
      break;

    case SB_FE2KXT_WIDE_PORT:
      for(mmu=0;mmu< 2/*SB_FE2000XT_NUM_MM_INSTANCES*/;mmu++) {
	if (mmu!=pDiagsInfo->uInstance) continue;
	if (pDiagsInfo->debug_level) {
	  printk("%s: Initializing MM%d Wide Port Memory..\n",FUNCTION_NAME(),mmu);
	}
	for(addr=0;addr < 1<<pDiagsInfo->wp_config.addr_width;addr++) {
	  stat = sbFe2kxtDiagsMemWrite(userDeviceHandle,mmu,addr,g_Fe2kxtWidePortpattern,SB_FE2KXT_WIDE_PORT);
	  if (stat != SB_FE2KXT_SUCCESS_E) {
	    return stat;
	  }
	}
      }
      break;
    case SB_FE2KXT_MEM_TYPE_LAST:
      printk("%s: Invalid mem type %d\n",FUNCTION_NAME(),pDiagsInfo->e_mem_type);
      return(-1);
      break;
    }

  DEXIT();
  return SB_FE2KXT_SUCCESS_E;

}


int sbFe2kxtDiagsWrRdComplete(sbhandle userDeviceHandle,
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
    thin_delay(100000);
  }

  /* timed out op did not complete in time */
  return ( FALSE );

}

/*
 *   FE2KXT LOOPBACK TESTS
 *
 */

int sbFe2kxtLoopbackTest(sbxDiagsInfo_t *pDiagsInfo)

{

    int stat=0;
    int rv = 0;
    uint32_t lbk = pDiagsInfo->e_loopback_type;
    uint32_t unit;
    int i,j;
    int xe = 0;
    sbhandle userDeviceHandle;
    uint32 uData;
    DENTER();
    unit = pDiagsInfo->unit;
    userDeviceHandle = SOC_SBX_CONTROL(unit)->sbhdl;
    
    soc_sbx_counter_stop(unit);

    for (i=0;i<2 /*SB_FE2000XT_NUM_AG_INTERFACES*/;i++) {
      for(j=0;j<12 /*SB_FE2000XT_MAX_AG_PORTS*/;j++) {
	g_AGTxPorts[i][j] = 0;
	g_AGRxPorts[i][j] = 0;
      }
    }

    printk("Clearing counters ..\n");
    if (( rv = Fe2kXtDiagsClearCounters(pDiagsInfo)) < 0 ) {
      test_error(unit,
		 "%s failed to clear counters (%s). \n", SOC_CHIP_STRING(unit),
		 bcm_errmsg(rv));
      return(-1);
    }

    /* turn off congestion controller for ag0/ag1 interfaces */
    uData = SAND_HAL_READ(userDeviceHandle,C2,PR_PRE2_CONFIG);
    uData = SAND_HAL_MOD_FIELD( C2, PR_PRE2_CONFIG, CC_ENABLE,uData, 0);
    SAND_HAL_WRITE(userDeviceHandle,C2,PR_PRE2_CONFIG,uData);
    uData = SAND_HAL_READ(userDeviceHandle,C2,PR_PRE3_CONFIG);
    uData = SAND_HAL_MOD_FIELD( C2, PR_PRE3_CONFIG, CC_ENABLE,uData, 0);
    SAND_HAL_WRITE(userDeviceHandle,C2,PR_PRE3_CONFIG,uData);

    switch(lbk) {
    case SB_FE2KXT_1G_LOOPBACK:
	stat = sbFe2kxtDiagsSerdesExternalLoopback(pDiagsInfo);
	break;
    case SB_FE2KXT_1G_UNIMAC_LOOPBACK:
    FE2KXT_STS_CHECK
      (sbFe2kxt1GLoopbackSet(unit,BCM_PORT_LOOPBACK_MAC));
	stat = sbFe2kxtDiagsUnimacLoopback(pDiagsInfo);
	break;
    case SB_FE2KXT_1G_PHY_LOOPBACK:
    FE2KXT_STS_CHECK
      (sbFe2kxt1GLoopbackSet(unit,BCM_PORT_LOOPBACK_PHY));
	stat = sbFe2kxtDiagsSerdesInternalLoopback(pDiagsInfo);
	break;
    case SB_FE2KXT_10G_LOOPBACK:
	stat = sbFe2kxtDiags10GLoopback(pDiagsInfo);
	break;
    case SB_FE2KXT_10G_QE_LOOPBACK:
	stat = sbFe2kxtDiags10GQELoopback(pDiagsInfo);
	break;
    default:
	SB_ERROR("ERROR:%s Invalid loopback test %d\n",FUNCTION_NAME(),lbk);
	return (-1);
    }

    /* do not leave any ports in any loopback state */
    if (lbk == SB_FE2KXT_1G_UNIMAC_LOOPBACK ||
	lbk == SB_FE2KXT_1G_PHY_LOOPBACK) {
      FE2KXT_STS_CHECK
	(sbFe2kxt1GLoopbackSet(unit,BCM_PORT_LOOPBACK_NONE));
    } else if (lbk == SB_FE2KXT_10G_LOOPBACK) {
      if (pDiagsInfo->bInternalLpk) {
	/* do not leave any 10G ports in internal loopback */
	for(xe=pDiagsInfo->start_port; xe<= pDiagsInfo->end_port; xe++) {
	  if ((rv = bcm_port_loopback_set(unit,xe,BCM_PORT_LOOPBACK_NONE)) < 0) {
	    test_error(unit,
		       "%s failed when setting xe%d to loopback NONE (%s) .\n",
		       SOC_CHIP_STRING(unit),xe,bcm_errmsg(rv));
	    stat = -1;
	  }
	}
      }
    }

    return stat;
}

void sbFe2kxtClearQueues(sbhandle hdl)
{


  C2QmQueueConfigClear(hdl);
/*   C2PtPortToQueueAg0Clear(hdl); */
/*   C2PtPortToQueueAg1Clear(hdl); */
  C2QmQueueState0Clear(hdl);
  C2QmQueueState1Clear(hdl);

}

int
sbFe2kxt1GLoopbackSet(uint32 unit,uint32_t loopback)
{
  int rv = 0;
  uint32_t port;
  for(port=0;port<24;port++) {
    if ((rv = bcm_port_loopback_set(unit,port,loopback)) < 0) {
      test_error(unit,
		 "%s failed when setting loopback on %s port. \n", 
		 SOC_CHIP_STRING(unit),SOC_PORT_NAME(unit,port));
      return(-1);
    }
  }
  return rv;
}


/* Loopback traffic from QE<--->10G */
int sbFe2kxtDiags10GQELoopback(sbxDiagsInfo_t *pDiagsInfo)
{

  int rv = -1;
  char rh_cmd[2][40] =  {{'\0'}};
  char pay_cmd[2][50] = {{'\0'}};
  uint32_t unit;
  uint32_t i;
  uint32_t port_10g_start,port_10g_end;
  args_t *pArgs = NULL;
  int timer_sec = 0;
  int port = 0;
  int port_status;
  char *shim = "shim=0x00c0ffee";
  char len_cmd[5] = {0};
  sal_mac_addr_t l2_macDAaddr;
  sal_mac_addr_t l2_macSAaddr;
  char *l2_macSAaddr_str = NULL;
  int dbg,vlan;
  pbmp_t port_pbm;
  pbmp_t port_pbm_untag;
  int port_untag;
  bcm_l2_addr_t  l2addr;
  int qe = -1;
  uint8_t bPortIsDown = FALSE;
  sbhandle qe_hdl,fe_hdl;
  int femodid = 0;
  int qid_start = 0;
  int qid_end = 0;
  int tme = 0;
  int num_cos = 0;
  bcm_vlan_control_vlan_t vlan_control;
  bcm_gport_t front_panel_port, fabric_port;
  static int provision_done = 0;

  unit = pDiagsInfo->unit;
  port_10g_start = pDiagsInfo->start_port;
  port_10g_end = pDiagsInfo->end_port;
  dbg = pDiagsInfo->debug_level;
#ifndef __KERNEL__
  signal(SIGINT,(void *)sigcatcher_fe2kxt);
#endif
  vlan = 2;
  BCM_PBMP_CLEAR(port_pbm);
  BCM_PBMP_CLEAR(port_pbm_untag);

  tme = soc_property_get(unit,spn_QE_TME_MODE,0);
  num_cos = soc_property_get(unit, spn_BCM_NUM_COS, (tme == 1) ? 16 : 8);

  /* args structure is too big to replicate on local stack */
  if ((pArgs = sal_alloc(sizeof (args_t), "pArgs")) == NULL) {
    printk("(%s) Error: out of memory\n",FUNCTION_NAME());
    return CMD_FAIL;
  }  

  /* Set the error reg list to check for errors */
  if (( rv = Fe2kXtDiagsSetErrorRegList(pDiagsInfo,unit)) < 0 ) {
    test_error(unit,
	       "%s failed to set error reg list(%s). \n", SOC_CHIP_STRING(unit),
	       bcm_errmsg(rv));
    sal_free(pArgs);
    return(-1);
  }    

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


  printk("Clearing errors..\n");
  Fe2kXtDiagsClearErrors(pDiagsInfo);

  BCM_PBMP_PORT_ADD(port_pbm,port_10g_start);
  BCM_PBMP_PORT_ADD(port_pbm,port_10g_end);
  BCM_PBMP_PORT_ADD(port_pbm_untag,port_10g_start);
  BCM_PBMP_PORT_ADD(port_pbm_untag,port_10g_end);

  qe_hdl = SOC_SBX_CONTROL(qe)->sbhdl;
  fe_hdl = SOC_SBX_CONTROL(unit)->sbhdl;

  /* check that the ports are setup to be 10g ports */
  PBMP_ITER(port_pbm,port) {
    if (!(IS_XE_PORT(unit,port) || IS_HG_PORT(unit,port))) {
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
    printk("vlan add %d pbm=%d,%d ubm=%d,%d\n",vlan,
	   port_10g_start,
	   port_10g_end,
	   port_10g_start,
	   port_10g_end);
  }



  bcm_vlan_port_add(unit,vlan,port_pbm,port_pbm_untag);

  /* turn off learning */
  if ((rv = bcm_vlan_control_vlan_get(unit,vlan,&vlan_control)) < 0) {
    printk("ERROR: Could get vlan control for vlan:%d, (%s)\n",vlan,bcm_errmsg(rv));
    sal_free(pArgs);
    return (-1);
  }

  vlan_control.flags |= BCM_VLAN_LEARN_DISABLE;
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

  /* deleting and re-creating l2 entries for 10g ports, causes problems
   * for now skip this if provisioning has been done .. investigating some
   * reason the packets going in xe1-->xe0 direction are not granted correctly from
   * polaris. Polaris grants for port0, and sees mc pkt ? The first time thru works */
  if ( provision_done == 1 ) goto inject_packets;

  rv = bcm_l2_clear(unit);
  if (BCM_FAILURE(rv)) {
    printk("ERROR: bcm_l2_clear(%d)\n",rv);
    sal_free(pArgs);
    return (-1);
  }

  /* add l2 entries for start and end port */
  l2_macSAaddr_str = l2_macSAaddrPkt2_str;
  parse_macaddr(l2_macSAaddr_str,l2_macSAaddr);
  bcm_l2_addr_t_init(&l2addr, l2_macSAaddr, vlan);

  l2addr.modid = femodid;
  l2addr.port = port_10g_start;
  l2addr.flags = BCM_L2_STATIC;

  if (dbg) {
    printk("l2 add macaddress=%s vlanid=%d module=%d pbm=%d static=true\n",
	   l2_macSAaddr_str,vlan,femodid,port_10g_start);
  }
  rv = bcm_l2_addr_add(unit,&l2addr);
  if (BCM_FAILURE(rv)) {
    printk("ERROR: bcm_l2_addr_add(%d)(%s)\n",rv,bcm_errmsg(rv));
    sal_free(pArgs);
    return (-1);
  }

  l2_macSAaddr_str = l2_macSAaddrPkt1_str;
  parse_macaddr(l2_macSAaddr_str,l2_macSAaddr);
  bcm_l2_addr_t_init(&l2addr, l2_macSAaddr, vlan);

  l2addr.modid = femodid;
  l2addr.port = port_10g_end;
  l2addr.flags = BCM_L2_STATIC;

  if (dbg) {
    printk("l2 add macaddress=%s vlanid=%d module=%d pbm=%d static=true\n",
	   l2_macSAaddr_str,vlan,femodid,port_10g_end);
  }
  rv = bcm_l2_addr_add(unit,&l2addr);
  if (BCM_FAILURE(rv)) {
    printk("ERROR: bcm_l2_addr_add(%d)(%s)\n",rv,bcm_errmsg(rv));
    sal_free(pArgs);
    return (-1);
  }

 inject_packets:

  /* get the qids */
  /* Convert Front panel ports to switch/fabric ports */
  BCM_GPORT_MODPORT_SET(front_panel_port,femodid, port_10g_start);
  rv = bcm_stk_fabric_map_get(unit, front_panel_port, &fabric_port);
  if (BCM_FAILURE(rv)) {
      printk("Failed to convert front panel port %d to fabric port: %s\n",
             BCM_GPORT_MODPORT_PORT_GET(front_panel_port), bcm_errmsg(rv));
      sal_free(pArgs);
      return (-1);
  }

  if (dbg) {
      printk ("Converted front panel port %d to fabric port %d\n",
              BCM_GPORT_MODPORT_PORT_GET(front_panel_port),
              BCM_GPORT_MODPORT_PORT_GET(fabric_port));
  }
  qid_start = SOC_SBX_NODE_PORT_TO_QID(unit, femodid, 
                                       BCM_GPORT_MODPORT_PORT_GET(fabric_port),
                                       num_cos);

  BCM_GPORT_MODPORT_SET(front_panel_port, femodid, port_10g_end);
  rv = bcm_stk_fabric_map_get(unit, front_panel_port, &fabric_port);
  if (BCM_FAILURE(rv)) {
      printk("Failed to convert front panel port %d to fabric port: %s\n",
             BCM_GPORT_MODPORT_PORT_GET(front_panel_port), bcm_errmsg(rv));
      sal_free(pArgs);
      return (-1);
  }

  if (dbg) {
      printk ("Converted front panel port %d to fabric port %d\n",
              BCM_GPORT_MODPORT_PORT_GET(front_panel_port),
              BCM_GPORT_MODPORT_PORT_GET(fabric_port));
  }
  qid_end = SOC_SBX_NODE_PORT_TO_QID(unit, femodid,
                                     BCM_GPORT_MODPORT_PORT_GET(fabric_port),
                                     num_cos);


  /* build ERH for packets going from QE out start_port */
  sprintf(rh_cmd[0],"rh=0x%03x0%04x0000000000000000",qid_start/4,
	  pDiagsInfo->nInjectPacketsLen-8);

  parse_macaddr(l2_macDAaddrPkt1_str,l2_macDAaddr);
  parse_macaddr(l2_macSAaddrPkt1_str,l2_macSAaddr);

  sprintf(pay_cmd[0],"pat=0x%02d%02d%02d%02d%02d%02d%02d%02d%02d%02d%02d%02d9000",
	  l2_macDAaddr[0],l2_macDAaddr[1],l2_macDAaddr[2],l2_macDAaddr[3],l2_macDAaddr[4],l2_macDAaddr[5],
	  l2_macSAaddr[0],l2_macSAaddr[1],l2_macSAaddr[2],l2_macSAaddr[3],l2_macSAaddr[4],l2_macSAaddr[5]);


  /* build ERH for packets going from QE out end_port */

  sprintf(rh_cmd[1],"rh=0x%03x0%04x0000000000000000",qid_end/4,
	  pDiagsInfo->nInjectPacketsLen-8);

  parse_macaddr(l2_macDAaddrPkt2_str,l2_macDAaddr);
  parse_macaddr(l2_macSAaddrPkt2_str,l2_macSAaddr);

  sprintf(pay_cmd[1],"pat=0x%02d%02d%02d%02d%02d%02d%02d%02d%02d%02d%02d%02d9000",
	  l2_macDAaddr[0],l2_macDAaddr[1],l2_macDAaddr[2],l2_macDAaddr[3],l2_macDAaddr[4],l2_macDAaddr[5],
	  l2_macSAaddr[0],l2_macSAaddr[1],l2_macSAaddr[2],l2_macSAaddr[3],l2_macSAaddr[4],l2_macSAaddr[5]);


  provision_done = 1;

  printk("Injecting %d packets into %s PCI port..\n",pDiagsInfo->nInjectPackets*2,
	 SOC_CHIP_STRING(qe));

  /* Injected packets alternate between going out start port,
   * and going out end_port. If 100 packets are injected, 100
   * will go out start_port, and another 100 out end_port to
   * allow all ports between start and end to tx and rx packets
   */

  for (i = 0; i < pDiagsInfo->nInjectPackets*2; i++) {
    sal_memset(pArgs,0x0,sizeof(args_t));
    sprintf(len_cmd,"len=%d",pDiagsInfo->nInjectPacketsLen);
    pArgs->a_cmd="SBX_TX";
    pArgs->a_argc = 5;
    pArgs->a_arg=1;
    pArgs->a_argv[0] = "sbx_tx";
    pArgs->a_argv[1] = IS_EVEN(i) ? rh_cmd[1] : rh_cmd[0];
    pArgs->a_argv[2] = shim;
    pArgs->a_argv[3] = IS_EVEN(i) ? pay_cmd[1] : pay_cmd[0];
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

  uXG_TxCount0 = 0; uXG_RxCount0 = 0;  /* start port counters */
  uXG_TxCount1 = 0; uXG_RxCount1 = 0;  /* end port counters */
  printk("Looping packet between %s <---> %s for roughly %d seconds ..\n",SOC_PORT_NAME(unit,port_10g_start),
         SOC_PORT_NAME(unit,port_10g_end), pDiagsInfo->uSnakeRunTime);
  timer_sec = 0;
  while (!gStopSnakeTest && timer_sec < pDiagsInfo->uSnakeRunTime) {
    sal_usleep(SNAKE_INTERVAL_US * 1E6);
    rv = Fe2kXtDiagsPollForErrors(port_10g_start,port_10g_end,unit);
    if (rv != 0) {
      break;
    }
    timer_sec += 1;
  }

  /* check again for errors to be sure */
  if (rv == 0 )
    rv = Fe2kXtDiagsCheckForErrors(unit);

  if ((gStopSnakeTest || timer_sec >= pDiagsInfo->uSnakeRunTime) && rv == 0) {
    printk("\nStopping Test. No Errors Found.\n");
  } else if (rv) {
    printk("\nStopping test due to errors. If errors persist try restarting bcm.user\n");
  }

  /* stop the packet from looping */
  if (pDiagsInfo->bStopLoopingPacket) {
    bcm_port_enable_set(unit,port_10g_start,0);
    thin_delay(100);
    bcm_port_enable_set(unit,port_10g_start,1);
  }

  sal_free(pArgs);
  return rv;
}

int sbFe2kxtPoll10GCounters(uint32_t unit,uint32_t port_10g_start, uint32_t port_10g_end)
{

    uint32_t uRetry = 3000;
    uint32_t bXG_start_Incrementing = 0;
    uint32_t bXG_end_Incrementing = 0;
    uint32_t uDataTx = 0;
    uint32_t uDataRx = 0;
    uint32_t uXgmNum_start;
    uint32_t uXgmNum_end;
    int rv = 0;
    sbhandle userDeviceHandle;

    /* be sure both start_port and end_port are tx pkts */
    userDeviceHandle = SOC_SBX_CONTROL(unit)->sbhdl;
    uXgmNum_start = port_10g_start & 0x3;
    while ( --uRetry && !bXG_start_Incrementing) {
      uDataTx = sbFe2kXtDiagsReadTxPktCount(userDeviceHandle,XgmToPtPr[uXgmNum_start]);
      if (uXG_TxCount0+uDataTx <= uXG_TxCount0) { /* if this count <= last count, pkt stopped */
	thin_delay(300); continue;
      }  else {
	uXG_TxCount0 += uDataTx;
	bXG_start_Incrementing = 1;
      }
    }

    if (uRetry == 0) {
      printk("ERROR: XG%d looping packet stopped unexpectedly\n",uXgmNum_start);
	rv = -1;
    }

    uXgmNum_end = port_10g_end & 0x3;
    uRetry = 3000;
    while ( --uRetry && !bXG_end_Incrementing) {
      uDataTx = sbFe2kXtDiagsReadTxPktCount(userDeviceHandle,XgmToPtPr[uXgmNum_end]);
      if (uXG_TxCount1+uDataTx <= uXG_TxCount1) { /* if this count <= last count, pkt stopped */
	thin_delay(300); continue;
      }  else {
	uXG_TxCount1 += uDataTx;
	bXG_end_Incrementing = 1;
      }
    }

    if (uRetry == 0) {
      printk("ERROR XG%d looping packet stopped unexpectedly\n",uXgmNum_end);
	rv = -1;
    }

    uDataRx = sbFe2kXtDiagsReadRxPktCount(userDeviceHandle,XgmToPtPr[uXgmNum_start]);
    uXG_RxCount0 += uDataRx;

    uDataRx = sbFe2kXtDiagsReadRxPktCount(userDeviceHandle,XgmToPtPr[uXgmNum_end]);
    uXG_RxCount1 += uDataRx;

    printk("XG%d Rx:%lld Tx:%lld XG%d Rx:%lld Tx:%lld\r",uXgmNum_start,
	   uXG_RxCount0, uXG_TxCount0,uXgmNum_end,uXG_RxCount1,uXG_TxCount1);
    return rv;
}



/*
 *   Setup 10G loobacks (Internal or External)
 *   (comments from verification Test 10.102)
 *   Inject packets into PCI, have the packet loopback external to the xaui
 *   , and exit PCI.  Check packet counter to ascertain that the packet
 *   managed to traverse the sws->bigmac->xaui(external)->bigmac->sws.
 *   You need external 10G loopback cables that are looped back onto itself.
 *   or between the starting xe interface and ending xe interface.
 */

int sbFe2kxtDiags10GLoopback(sbxDiagsInfo_t *pDiagsInfo)
{

  uint32_t uPktCnt;
  sbhandle userDeviceHandle;
  uint32_t uXgm_start,uXgm_end = 0;
  uint32_t uXgm = 0;
  uint32_t unit;
  int rv = 0;
  uint32_t stat = 0;
  uint32_t uDataRx,uDataTx = 0;
  uint32_t dbg;
  char *s = NULL;
  uint32_t port_10g = 0;
  uint32_t uQueue = 120;
  uint32_t xe_start_port = 0;
  uint32_t xe_end_port = 0;
  int nPrPt = 0;

  unit = pDiagsInfo->unit;
  userDeviceHandle = SOC_SBX_CONTROL(unit)->sbhdl;
  uPktCnt = pDiagsInfo->nInjectPackets;
  dbg = pDiagsInfo->debug_level;

  /* get the starting/ending xe port */
  xe_start_port = pDiagsInfo->start_port;
  xe_end_port = pDiagsInfo->end_port;

  printk("xe_start=%d, xe_end=%d\n",xe_start_port,xe_end_port);

  if (xe_start_port < XE0_PORT ||
      xe_start_port > XE3_PORT ||
      xe_end_port < XE0_PORT ||
      xe_end_port > XE3_PORT ) {

    sal_printf("Invalid xe port only [%d - %d] is valid\n",XE0_PORT,XE3_PORT);
    return (-1);
  }

  /* get starting/ending Xgm interfaces */
  uXgm_start = xe_start_port - XE0_PORT;
  uXgm_end = xe_end_port - XE0_PORT;
  
  if (pDiagsInfo->bInternalLpk) {
    s = "INTERNAL";
  } else {
    s = "EXTERNAL";
  }

  /* show front panel loopback connections being tested */
  printk("10GE%d - 10GE%d  %s  Loopback Test\n",xe_start_port-23,xe_end_port-23,s);

  /* set loopback - to MAC if running internal loopback test */
  if (pDiagsInfo->bInternalLpk) {
    for (port_10g = xe_start_port; port_10g <= xe_end_port; port_10g++) {
      if ((rv = bcm_port_loopback_set(unit,port_10g,BCM_PORT_LOOPBACK_MAC)) < 0) {
	printk("Error setting loopback for port:%s (%s)\n",SOC_PORT_NAME(unit,port_10g),
	       bcm_errmsg(rv));
	return (-1);
      }
    }
  }

  /* create the internal SWS connections 
   *  
   *     tx     rx
   *  ---- xg0 ] <----------  
   * |                        PCI
   * |   rx     tx
   *   --- xg1 ] -----------> 
   * 
   *   Xe0 PRE4
   *   Xe1 PRE5
   *   Xe2 PRE2
   *   Xe3 PRE3
   *
   *  If start interface = end interface
   *  xg0 ] <----------> PCI   
   *  
   *  For MAC loopback or when IPASS loopback cables are available.
   *
   */

    nPrPt = XgmToPtPr[uXgm_start];
    XtAddQueue(userDeviceHandle,6/*uInPre*/,0/*uInPort*/,nPrPt/*uOutPte*/,0/*uOutPort*/,uQueue); /* PCI --> uXgm_start */
    uQueue++;
    nPrPt = XgmToPtPr[uXgm_end];
    XtAddQueue(userDeviceHandle,nPrPt/*uInPre*/,0/*uInPort*/,6/*uOutPte*/,0/*uOutPort*/,uQueue); /* uXgm_end ---> PCI */
    uQueue++;

    /* if internal loopback create additional SWS connections */
    if (pDiagsInfo->bInternalLpk) {
      for(uXgm = uXgm_start; uXgm < uXgm_end; uXgm++) {
	nPrPt = XgmToPtPr[uXgm & 0x3];
	XtAddQueue(userDeviceHandle,nPrPt/*uInPre*/,0/*uInPort*/,XgmToPtPr[(uXgm+1) & 0x3]/* uOutPte */,0/*uOutPort*/,uQueue);
	uQueue++;
      }
    }
 
    stat = sbFe2kxtDiagsInjectPciPacket(pDiagsInfo);

    if (stat != SB_FE2KXT_SUCCESS_E) {
      test_error(unit,"ERROR: sbFe2kxtDiagsInjectPciPacket failed with status %d\n",stat);
      return(-1);
    }

    uDataTx = SAND_HAL_READ(userDeviceHandle,C2,PC_TX_PKT_CNT);
    if (uDataTx != uPktCnt) {
      SB_ERROR("ERROR: PCI TxExp:%d TxAct:%d\n",uPktCnt,uDataTx);
      stat = -1;
    } else if (dbg) {
      printk("GOOD: PCI TxExp:%d TxAct:%d\n",uPktCnt,uDataTx);
    }

    /* check packets where accepted by the QM block */
    uDataRx = SAND_HAL_READ(userDeviceHandle,C2,PR_PRE6_PKT_CNT);
    if (uDataRx != uPktCnt) {
      SB_ERROR("ERROR:% SWS(PCI) RxExp:%d RxAct:%d\n",uPktCnt,uDataRx);
      stat = -1;
    } else if (dbg) {
      printk("GOOD: SWS(PCI) RxExp:%d RxAct:%d\n",uPktCnt,uDataRx);
    }

    /* check pkt statistics */
    if (pDiagsInfo->bInternalLpk) {
      for (uXgm = uXgm_start; uXgm <= uXgm_end; uXgm++) {
	uDataTx = sbFe2kXtDiagsReadTxPktCount(userDeviceHandle,XgmToPtPr[uXgm&3]);
	uDataRx = sbFe2kXtDiagsReadRxPktCount(userDeviceHandle,XgmToPtPr[uXgm&3]);
	if (uDataTx != uPktCnt) {
	  test_error(unit,"%s XG:%d TxExp:%d TxAct:%d. \n", 
		     SOC_UNIT_NAME(unit),uXgm,uPktCnt,uDataTx);
	  stat = -1;
	}

	if (uDataRx != uPktCnt) {
	  test_error(unit,"%s XG:%d RxExp:%d RxAct:%d. \n", 
		     SOC_UNIT_NAME(unit),uXgm,uPktCnt,uDataRx);
	  stat = -1;
	}
      }
    } else { /* external loopback */
      uDataTx = sbFe2kXtDiagsReadTxPktCount(userDeviceHandle,XgmToPtPr[uXgm_start]);
      if (uDataTx != uPktCnt) {
	  test_error(unit,"%s XG:%d TxExp:%d TxAct:%d. \n", 
		     SOC_UNIT_NAME(unit),uXgm_start,uPktCnt,uDataTx);
	stat = -1;
      } else if (dbg) {
	printk("GOOD: XGM%d TxExp:%d TxAct:%d\n",uXgm_start,uPktCnt,uDataTx);
      }

      uDataRx = sbFe2kXtDiagsReadRxPktCount(userDeviceHandle,XgmToPtPr[uXgm_end]);
      if (uDataRx != uPktCnt) {
	  test_error(unit,"%s XG:%d RxExp:%d RxAct:%d. \n", 
		     SOC_UNIT_NAME(unit),uXgm_end,uPktCnt,uDataRx);
	stat = -1;
      } else if (dbg) {
	printk("GOOD XGM%d RxExp:%d RxAct:%d\n",uXgm_end,uPktCnt,uDataRx);
      }
    }

    /* check that the packet came out the PCI port, pull it out of buffer pte6 */
    stat = sbFe2kxtDiagsReceivePciPacket(pDiagsInfo);
    if (stat != SB_FE2KXT_SUCCESS_E) {
      SB_ERROR("ERROR:%s sbFe2kxtDiagsRecievePciPacket failed with status %d\n",FUNCTION_NAME(),stat);
      return (-1);
    }

    /* all nPacketsInjected packets should have gotten sent to pci */
    uDataTx = SAND_HAL_READ(userDeviceHandle,C2,PT_PTE6_PKT_CNT);
    if (uDataTx != uPktCnt) {
      SB_ERROR("ERROR: SWS.PTE6 TxExp:%d TxAct:%d\n",uPktCnt,uDataTx);
      stat = -1;
    } else if (dbg) {
      printk("GOOD: SWS.PTE6 TxExp:%d TxAct:%d\n",uPktCnt,uDataTx);
    }

    if (stat == 0) {
      printk("Packet looped back to PCI ok. Continuity test passed\n");
    } else {
      printk("Packet was not recieved back to PCI. Test failed\n");
    }

    return stat;
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
int sbFe2kxtDiagsUnimacLoopback(sbxDiagsInfo_t *pDiagsInfo)
{
  uint32_t nAgm;
  uint32_t uQueue=200;  /* temp starting queue , pick queue not used by gu2k */
  uint32_t nPort;
  uint32_t uData=0;
  int stat=0;
  int counter_stat = 0;
  sbhandle userDeviceHandle;
  uint32_t nPacketsInjected;
  int dbg;
  int rv = 0;
  userDeviceHandle = pDiagsInfo->userDeviceHandle;
  nPacketsInjected = pDiagsInfo->nInjectPackets;
  dbg = pDiagsInfo->debug_level;

  DENTER();
  printk("Running 1G Internal(Unimac) Loopback Test\n");

    /* connect PCI.p0 -> AGM0.p0 */
  if (dbg) {
    printk("Creating SWS connection from %s.p%d to %s.p%d using queue:%d\n",
	   sbFe2kxtDiagsPortInterfaceStr[6],0,sbFe2kxtDiagsPortInterfaceStr[2],0,uQueue);
  }
    XtAddQueue(userDeviceHandle,6/*uInPre*/,0/*uInPort*/,2/*uOutPte*/,0/*uOutPort*/,uQueue);
    uQueue++;

    for(nAgm=0;nAgm<2/*SB_FE2000XT_NUM_AG_INTERFACES*/;nAgm++) {
      for(nPort=1;nPort<12;nPort++) {
	if (dbg) {
	printk("Creating SWS connection from %s.p%d to %s.p%d using queue:%d\n",
	       sbFe2kxtDiagsPortInterfaceStr[2+nAgm],nPort-1,sbFe2kxtDiagsPortInterfaceStr[2+nAgm],nPort,uQueue);
	}
	XtAddQueue(userDeviceHandle,2+nAgm/*uInPre*/,nPort-1/*uInPort*/,2+nAgm/*uOutPte*/,nPort/*uOutPort*/,uQueue);
	uQueue++;
      }
      if (nAgm == 0) {
	/* now connect AGM0.p11 -> AGM1.p0 */
	if (dbg) {
	printk("Creating SWS connection from %s.p%d to %s.p%d using queue:%d\n",
	       sbFe2kxtDiagsPortInterfaceStr[2],11,sbFe2kxtDiagsPortInterfaceStr[3],0,uQueue);
	}
	XtAddQueue(userDeviceHandle,2/*uInPre*/,11/*uInPort*/,3/*uOutPte*/,0/*uOutPort*/,uQueue);
	uQueue++;
      } else if (nAgm==1) {
	/* now connect AGM1.p11 -> PCI.p0 */
	if (dbg) {
	printk("Creating SWS connection from %s.p%d to %s.p%d using queue:%d\n",
	       sbFe2kxtDiagsPortInterfaceStr[3],11,sbFe2kxtDiagsPortInterfaceStr[6],0,uQueue);
	}
	XtAddQueue(userDeviceHandle,3/*uInPre*/,11/*uInPort*/,6/*uOutPte*/,0/*uOutPort*/,uQueue);
	uQueue++;
      }
    }

  stat = sbFe2kxtDiagsInjectPciPacket(pDiagsInfo);
  if (stat != SB_FE2KXT_SUCCESS_E) {
    SB_ERROR("ERROR:%s sbFe2kxtDiagsInjectPciPacket failed with status %d\n",FUNCTION_NAME(),stat);
    return(-1);
  }

  thin_delay(500);

  uData = SAND_HAL_READ(userDeviceHandle,C2,PC_TX_PKT_CNT);
  if (uData != nPacketsInjected) {
    SB_ERROR("ERROR:%s, PCI TxExp:%d TxAct:%d\n",FUNCTION_NAME(),nPacketsInjected,uData);
    counter_stat = -1;
  } else if (dbg) {
    printk("GOOD PCI TxExp:%d TxAct:%d\n",nPacketsInjected,uData);
  }

  /* check packets where accepted by the QM block */
  uData = SAND_HAL_READ(userDeviceHandle,C2,PR_PRE6_PKT_CNT);
  if (uData != nPacketsInjected) {
    SB_ERROR("ERROR:%s, SWS.PCI(PRE6) Exp:%d Act:%d\n",FUNCTION_NAME(),nPacketsInjected,uData);
    counter_stat = -1;
  } else if (dbg) {
    printk("GOOD SWS.PCI(PRE6) Exp:%d Act:%d\n",nPacketsInjected,uData);
  }

  stat = sbFe2kxtCheckLoopbackStats(pDiagsInfo,0);
  if (stat != SB_FE2KXT_SUCCESS_E) {
      SB_ERROR("ERROR:1G Loopback Stats Check failed\n",FUNCTION_NAME());
      counter_stat = -1;
  }
  stat = sbFe2kxtDiagsReceivePciPacket(pDiagsInfo);

  if (stat != SB_FE2KXT_SUCCESS_E) {
    SB_ERROR("ERROR:%s sbFe2kxtDiagsRecievePciPacket did not rx expected pkt(s) %d\n",FUNCTION_NAME(),stat);
    return (-1);
  }

  /* all nPacketsInjected packets should go back out PCI port */
  uData = SAND_HAL_READ(userDeviceHandle,C2,PT_PTE6_PKT_CNT);
  if (uData != nPacketsInjected) {
    SB_ERROR("ERROR:%s, SWS.PTE6 TxExp:%d TxAct:%d\n",FUNCTION_NAME(),nPacketsInjected,uData);
    counter_stat = -1;
  } else if (dbg) {
    printk("GOOD SWS.PTE6 TxExp:%d TxAct:%d\n",nPacketsInjected,uData);
  }

  if (counter_stat !=0 ) {
    printk("Counter mismatch error occured. FAILED\n");
    rv = -1;
  } else {
    printk("Packet looped back to PCI ok. PASSED\n");
    rv = 0;
  }

  return rv;

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

int sbFe2kxtDiagsSerdesExternalLoopback(sbxDiagsInfo_t *pDiagsInfo)
{

  uint32_t uData = 0;
  uint32_t stat=0;
  sbhandle userDeviceHandle;
  uint32_t nPacketsInjected;
  int dbg = pDiagsInfo->debug_level;
  userDeviceHandle = pDiagsInfo->userDeviceHandle;
  nPacketsInjected = pDiagsInfo->nInjectPackets;
  DENTER();

  printk("Running 1G External Loopback Test\n");

  /* create the queues for 1G loopback */
  sbFe2kxtCreate1GLoopbackQueues(pDiagsInfo);

  stat = sbFe2kxtDiagsInjectPciPacket(pDiagsInfo);
  if (stat != SB_FE2KXT_SUCCESS_E) {
    SB_ERROR("ERROR:%s sbFe2kxtDiagsInjectPciPacket failed with status %d\n",FUNCTION_NAME(),stat);
    return stat;
  }

  thin_delay(500);
  uData = SAND_HAL_READ(userDeviceHandle,C2,PC_TX_PKT_CNT);
  if (uData != nPacketsInjected) {
    SB_ERROR("ERROR:%s, PCI->FE2k TxExp:%d TxAct:%d\n",FUNCTION_NAME(),nPacketsInjected,uData);
    /* just return, test is done for */
    return (-1);
  } else {
      if (dbg) {
	  printk("GOOD PCI->FE2k TxExp:%d TxAct:%d\n",nPacketsInjected,uData);
      }
  }

  stat = sbFe2kxtCheckLoopbackStats(pDiagsInfo,1);
  if (stat != SB_FE2KXT_SUCCESS_E) {
      SB_ERROR("ERROR:1G Loopback Stats Check failed\n",FUNCTION_NAME());
      return (-1);
  }

  /* check that the packet came out the PCI port */
  stat = sbFe2kxtDiagsReceivePciPacket(pDiagsInfo);
  if (stat != SB_FE2KXT_SUCCESS_E) {
    SB_ERROR("ERROR:%s sbFe2kxtDiagsRecievePciPacket not all pkt(s) recieved to PCI %d\n",FUNCTION_NAME());
    return stat;
  } else {
    printk("Packet looped back to PCI ok. Continuity test passed\n");
  }

  DEXIT();
  return 0;

}

void sbFe2kxtCreate1GLoopbackQueues(sbxDiagsInfo_t *pDiagsInfo)
{

    int dbg;
    uint32_t uQueue=100;
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
    printk("Creating SWS connection from %s.%d to %s.%d using queue:%d\n",
	   sbFe2kxtDiagsPortInterfaceStr[SB_FE2000_IF_PTYPE_PCI],0,
	   sbFe2kxtDiagsPortInterfaceStr[sp_interface],sp,uQueue);
    }
    XtAddQueue(hdl,SB_FE2000_IF_PTYPE_PCI,0,sp_interface,sp,uQueue);
    uQueue++;

    g_AGTxPorts[sp_interface-2][sp] = 1;

    /* daisy chain the rest of the ports together */
    for(nPort=pDiagsInfo->start_port+1;nPort<pDiagsInfo->end_port;nPort+=2) {
      frm_interface = (nPort < 12) ? SB_FE2000_IF_PTYPE_AGM0 : SB_FE2000_IF_PTYPE_AGM1;
      to_interface = (nPort+1 < 12) ? SB_FE2000_IF_PTYPE_AGM0 : SB_FE2000_IF_PTYPE_AGM1;
      if (dbg) {
      printk("Creating SWS connection from %s.%d to %s.%d using queue:%d\n",
	     sbFe2kxtDiagsPortInterfaceStr[frm_interface],nPort%12,
	     sbFe2kxtDiagsPortInterfaceStr[to_interface],(nPort+1)%12,uQueue);
      }
      g_AGTxPorts[to_interface-2][(nPort+1)%12] = 1;
      g_AGRxPorts[frm_interface-2][nPort%12] = 1;
      XtAddQueue(hdl,frm_interface,nPort%12,to_interface,(nPort+1)%12,uQueue);
      uQueue++;
    }

    /* for the last connection, back to start port or out PCI (for debug ) */
    if (pDiagsInfo->bSnakeLoopOn) {
      if (dbg) {
      printk("Creating SWS connection from %s.%d to %s.%d using queue:%d\n",
	     sbFe2kxtDiagsPortInterfaceStr[ep_interface],ep,
	     sbFe2kxtDiagsPortInterfaceStr[sp_interface],sp,uQueue);
      }
      XtAddQueue(hdl,ep_interface,ep%12,sp_interface,sp%12,uQueue);
    } else {
      if (dbg) {
      printk("Creating SWS connection from %s.%d to %s.%d using queue:%d\n",
	     sbFe2kxtDiagsPortInterfaceStr[ep_interface],ep,
	     sbFe2kxtDiagsPortInterfaceStr[SB_FE2000_IF_PTYPE_PCI],0,uQueue);
      }
      g_AGRxPorts[ep_interface-2][ep] = 1;
      XtAddQueue(hdl,ep_interface,ep,SB_FE2000_IF_PTYPE_PCI,0,uQueue);
    }
}

int sbFe2kxtCheckLoopbackStats(sbxDiagsInfo_t *pDiagsInfo,
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
    for(nAgm=0;nAgm< 2/*SB_FE2000XT_NUM_AG_INTERFACES*/;nAgm++) {
      for(nPort=0;nPort<12 /*SB_FE2000XT_MAX_AG_PORTS*/;nPort++) {
	if (g_AGTxPorts[nAgm][nPort]) {
	  FE2KXT_STS_CHECK
	      (soc_sbx_fe2000_am_read(unit,nAgm,nPort,REG_OFFSET_C2(AM_TX_VALID_PKT_CNT),&uData));
	  if (uData != nPackets) {
	    SB_ERROR("ERROR:AGM%d.p%d TxExp:%d got TxAct:%d\n",nAgm,nPort,nPackets,uData);
	    rv = -1;
	  } else {
	    if (nDbg) {
	      printk("GOOD AGM%d.p%d TxExp:%d got TxAct:%d\n",nAgm,nPort,nPackets,uData);
	    }
	  }
	}
	/* check port rx */
	if (g_AGRxPorts[nAgm][nPort]) {
	  FE2KXT_STS_CHECK
	      (soc_sbx_fe2000_am_read(unit,nAgm,nPort,REG_OFFSET_C2(AM_RX_VALID_PKT_CNT),&uData));
	  if (uData != nPackets) {
	    SB_ERROR("ERROR:AGM%d.p%d RxExp:%d got RxAct:%d\n",nAgm,nPort,nPackets,uData);
	    rv = -1;
	  } else {
	    if (nDbg) {
	      printk("GOOD AGM%d.p%d RxExp:%d got RxAct:%d\n",nAgm,nPort,nPackets,uData);
	    }
	  }
	}
      }
    }
  } else { /* internal loopbacks are looped back onto itself */
    for(nAgm=0;nAgm<2/*SB_FE2000XT_MAX_AG_INTERFACES*/;nAgm++) {
      for(nPort=0;nPort<12/*SB_FE2000XT_MAX_AG_PORTS*/;nPort++) {
	  FE2KXT_STS_CHECK
	      (soc_sbx_fe2000_am_read(unit,nAgm,nPort,REG_OFFSET_C2(AM_TX_VALID_PKT_CNT),&uData));
	if (uData != nPackets) {
	  SB_ERROR("ERROR:AGM%d.p%d TxExp:%d got TxAct:%d\n",nAgm,nPort,nPackets,uData);
	  rv = -1;
	} else {
	  if (nDbg) {
	    printk("GOOD AGM%d.p%d TxExp:%d got TxAct:%d\n",nAgm,nPort,nPackets,uData);
	  }
	}
	FE2KXT_STS_CHECK
	    (soc_sbx_fe2000_am_read(unit,nAgm,nPort,REG_OFFSET_C2(AM_RX_VALID_PKT_CNT),&uData));

	if (uData != nPackets) {
	  SB_ERROR("ERROR:AGM%d.p%d RxExp:%d got RxAct:%d\n",nAgm,nPort,nPackets,uData);
	  rv = -1;
	} else {
	  if (nDbg) {
	    printk("GOOD AGM%d.p%d RxExp:%d got RxAct:%d\n",nAgm,nPort,nPackets,uData);
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
 *   Agm1 instance refers to the SFPs (0-11). PHYS are internal accessible via MDIO
 */

int sbFe2kxtDiagsSerdesInternalLoopback(sbxDiagsInfo_t *pDiagsInfo)
{
  uint32_t nAgm;
  uint32_t nPort;
  uint32_t uData;
  uint32_t stat=0;
  sbhandle userDeviceHandle;
  uint32_t nPacketsInjected;
  int dbg;
  uint32_t uQueue=200; /* temp starting queue , pick queue not used by gu2k */
  int rv = 0;

  userDeviceHandle = pDiagsInfo->userDeviceHandle;
  nPacketsInjected = pDiagsInfo->nInjectPackets;
  dbg = pDiagsInfo->debug_level;
  DENTER();

  printk("Running 1G Internal(PHY) Loopback Test\n");

    /* connect PCI.p0 -> AGM0.p0 */
  if (dbg) {
    printk("Creating SWS connection from %s.p%d to %s.p%d using queue:%d\n",
	   sbFe2kxtDiagsPortInterfaceStr[6],0,sbFe2kxtDiagsPortInterfaceStr[2],0,uQueue);
  }
    XtAddQueue(userDeviceHandle,6/*uInPre*/,0/*uInPort*/,2/*uOutPte*/,0/*uOutPort*/,uQueue);
    uQueue++;

    for(nAgm=0;nAgm<2/*SB_FE2000XT_NUM_AG_INTERFACES*/;nAgm++) {
      for(nPort=1;nPort<12;nPort++) {
	if (dbg) {
	printk("Creating SWS connection from %s.p%d to %s.p%d using queue:%d\n",
	       sbFe2kxtDiagsPortInterfaceStr[2+nAgm],nPort-1,sbFe2kxtDiagsPortInterfaceStr[2+nAgm],nPort,uQueue);
	}
	XtAddQueue(userDeviceHandle,2+nAgm/*uInPre*/,nPort-1/*uInPort*/,2+nAgm/*uOutPte*/,nPort/*uOutPort*/,uQueue);
	uQueue++;
      }
      if (nAgm == 0) {
	/* now connect AGM0.p11 -> AGM1.p0 */
	if (dbg) {
	printk("Creating SWS connection from %s.p%d to %s.p%d using queue:%d\n",
	       sbFe2kxtDiagsPortInterfaceStr[2],11,sbFe2kxtDiagsPortInterfaceStr[3],0,uQueue);
	}
	XtAddQueue(userDeviceHandle,2/*uInPre*/,11/*uInPort*/,3/*uOutPte*/,0/*uOutPort*/,uQueue);
	uQueue++;
      } else if (nAgm==1) {
	/* now connect AGM1.p11 -> PCI.p0 */
	if (dbg) {
	printk("Creating SWS connection from %s.p%d to %s.p%d using queue:%d\n",
	       sbFe2kxtDiagsPortInterfaceStr[3],11,sbFe2kxtDiagsPortInterfaceStr[6],0,uQueue);
	}
	XtAddQueue(userDeviceHandle,3/*uInPre*/,11/*uInPort*/,6/*uOutPte*/,0/*uOutPort*/,uQueue);
	uQueue++;
      }
    }

  stat = sbFe2kxtDiagsInjectPciPacket(pDiagsInfo);
  if (stat != SB_FE2KXT_SUCCESS_E) {
    SB_ERROR("ERROR:%s sbFe2kxtDiagsInjectPciPacket failed with status %d\n",FUNCTION_NAME(),stat);
    return(-1);
  }

  uData = SAND_HAL_READ(userDeviceHandle,C2,PC_TX_PKT_CNT);
  if (uData != nPacketsInjected) {
    SB_ERROR("ERROR:%s, PCI->FE2k TxExp:%d TxAct:%d\n",FUNCTION_NAME(),nPacketsInjected,uData);
    /* just return, test is done for */
    return (-1);
  } else {
      if (dbg) {
	  printk("GOOD PCI->FE2k TxExp:%d TxAct:%d\n",nPacketsInjected,uData);
      }
  }
  stat = sbFe2kxtCheckLoopbackStats(pDiagsInfo,0);
  if (stat != SB_FE2KXT_SUCCESS_E) {
      SB_ERROR("ERROR:1G Loopback Stats Check failed\n",FUNCTION_NAME());
      stat = -1;
  }

  /* check that the packet came out the PCI port */
  rv = sbFe2kxtDiagsReceivePciPacket(pDiagsInfo);
  if (rv != SB_FE2KXT_SUCCESS_E) {
    SB_ERROR("ERROR:%s sbFe2kxtDiagsRecievePciPacket failed with status %d\n",FUNCTION_NAME(),stat);
    return rv;
  }

  if (stat == 0) {
    printk("Packet looped back to PCI ok. PASSED\n");
  } else {
    printk("Packet was not recieved back to PCI. FAILED\n");
  }

  return stat;
}

/* This mimics entering the cmds in the BCM shell */
int32_t
sbFe2kxtDiagsInjectPciPacket(sbxDiagsInfo_t *pDiagsInfo)
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
	printk("Injecting %d packets into PCI\n",nPackets);
    }

    sal_memset(pArgs,0x0,sizeof(args_t));
    sprintf(cmd_1,"%d",nPackets);
    sprintf(cmd_2,"len=%d",len);
    if (dbg) {
	printk("sbx_tx %d len=%d\n",nPackets,len);
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
sbFe2kxtDiagsReceivePciPacket(sbxDiagsInfo_t *pDiagsInfo)
{

    args_t *pArgs = NULL;
    uint32_t uData=0;
    int i;
    int nPkts;
    sbhandle hdl;
    int dbg;
    int unit;
    int stat = 0;

    unit = pDiagsInfo->unit;
    nPkts = pDiagsInfo->nInjectPackets;
    hdl = SOC_SBX_CONTROL(unit)->sbhdl;
    dbg = pDiagsInfo->debug_level;

    if ((pArgs = sal_alloc(sizeof (args_t), "pArgs")) == NULL) {
      printk("(%s) Error: out of memory\n",FUNCTION_NAME());
      return CMD_FAIL;
    }  

    sal_memset(pArgs,0x0,sizeof(args_t));

    /* dump all the packets */
    for(i=0;i<nPkts;i++) {
	cmd_soc_sbx_rx(unit,pArgs);
    }
    /* check the pkt count */
    if (SOC_IS_SBX_FE2KXT(unit)) {
      uData = SAND_HAL_READ(hdl,C2,PC_RX_PKT_CNT);
    } else if (SOC_IS_SBX_QE2000(unit)) {
      uData = SAND_HAL_READ(hdl,KA,PC_RX_PKT_CNT);
    }

    if (uData != nPkts) {
      SB_ERROR("ERROR:%s -> PCI TxExp:%d got TxAct:%d\n",SOC_CHIP_STRING(unit),nPkts,uData);
       stat = -1;
    } else {
      if (dbg) {
	printk("GOOD %s_%d -> PCI TxExp:%d got TxAct:%d\n",SOC_CHIP_STRING(unit)
	       ,unit,nPkts,uData);
      }
    }

    sal_free(pArgs);
    return stat;
}

int
sbFe2kxtDiagsDumpExceptionPkt(sbxDiagsInfo_t *pDiagsInfo,
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



int sbFe2kxtDiagsOverrideMmParam(sbxDiagsInfo_t *pDiagsInfo)
{

  uint32_t uMmInstance=0;
  uint32_t uMmProtectionScheme0=0;
  sbFe2000XtInitParamsMm_t mm[2/*SB_FE2000XT_NUM_MM_INSTANCES*/];
  uint32_t uPolicerRefreshEnable=0;
  uint32_t uData;
  sbhandle userDeviceHandle = pDiagsInfo->userDeviceHandle;
  int rv;

  sal_memset(&mm[0],0x0,sizeof(sbFe2000XtInitParamsMm_t));
  sal_memset(&mm[1],0x0,sizeof(sbFe2000XtInitParamsMm_t));

  for( uMmInstance = 0; uMmInstance < 2/*SB_FE2000XT_NUM_MM_INSTANCES*/; uMmInstance++ ){
    sbFe2000XtInitParamsMm_t *pInstance = &(mm[uMmInstance]);
    pInstance->ramprotection.NarrowPort0ProcessorInterface = SB_FE2000XT_MM_36BITS_NOPROTECTION;
    pInstance->ramprotection.NarrowPort1ProcessorInterface = SB_FE2000XT_MM_36BITS_NOPROTECTION;
    pInstance->ramprotection.WidePortRamProtectionScheme = SB_FE2000XT_MM_36BITS_NOPROTECTION;

    uMmProtectionScheme0 = SAND_HAL_READ_STRIDE( userDeviceHandle, C2, MM, uMmInstance, MM_PROT_SCHEME0 );
    uMmProtectionScheme0 = SAND_HAL_MOD_FIELD( C2, MM_PROT_SCHEME0, NP0_PI_SCHEME,  uMmProtectionScheme0,
					       pInstance->ramprotection.NarrowPort0ProcessorInterface );
    uMmProtectionScheme0 = SAND_HAL_MOD_FIELD( C2, MM_PROT_SCHEME0, NP1_PI_SCHEME,  uMmProtectionScheme0,
					       pInstance->ramprotection.NarrowPort1ProcessorInterface );

    SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uMmInstance, MM_PROT_SCHEME0, uMmProtectionScheme0 );
  }

  /* see if policer background refreshes are turned on */
  uData = SAND_HAL_READ( userDeviceHandle,C2,PM_CONFIG0);
  uPolicerRefreshEnable = SAND_HAL_GET_FIELD(C2,PM_CONFIG0,REFRESH_ENABLE,uData);
  if (uPolicerRefreshEnable) {
    uData = SAND_HAL_MOD_FIELD(C2,PM_CONFIG0,REFRESH_ENABLE,uData,0);
    SAND_HAL_WRITE(userDeviceHandle,C2,PM_CONFIG0,uData);
    pDiagsInfo->bPolicerRefreshWasOn = 1;
  }

  /* stop counter thread */
  rv = soc_sbx_counter_stop(pDiagsInfo->unit);
  if (SOC_FAILURE(rv)) {
    printk("Errors stopping counter thread, may interfere with test\n");
  }

  /* put CM in reset, CM updates to SRAM can inter with test */
  /* there does not appear to be a way to disable cm, have looked have asked.. */
  /* SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, CM_CORE_RESET, 1); */

  return 0;
}

int sbFe2kxtDiagsAddressCheck(sbxDiagsInfo_t *pDiagsInfo)
{

  unsigned long ulStartAddr = pDiagsInfo->start_addr;
  unsigned long ulEndAddr = pDiagsInfo->end_addr;
  unsigned long ulMaxAddr;
  uint32_t mem;

  mem = pDiagsInfo->e_mem_type;
  if (mem == SB_FE2KXT_NARROW_PORT_0) {
      ulMaxAddr = (1<<pDiagsInfo->np0_config.addr_width);
  } else if (mem == SB_FE2KXT_NARROW_PORT_1) {
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
	     FUNCTION_NAME(),sbFe2kxtDiagsMemoryStr[mem],
	     ulStartAddr, ulEndAddr, ulMaxAddr);
    return (-1);
  }

  return(0);
}

/*
 *  LineRate External Memory Test. (Ported ZTCaladan90d033 test)
 */

int
sbFe2kxtLineRateMemTestRun(sbxDiagsInfo_t *p)
{
  int rv = 0;
  uint32_t err = 0;
  uint32_t timeout = 0;
  int i = 0;
  sbhandle hdl;
  int unit;
  int seed;
  uint32_t MemExists = p->uMemExistsMask;
  uint32_t status = 0;


  if (p->seed == -1) {
    seed = 0x2aaa;
  } else {
    seed = p->seed;
  }

  seed = seed & 0x3fff;
  p->seed = seed;
  unit = p->unit;

  printk("%s: Testing with seed=0x%x\n",FUNCTION_NAME(),seed);
  printk("Testing memories: ");

  if (GBITN(MemExists, 0)) printk("MM0.N0 ");
  if (GBITN(MemExists, 1)) printk("MM0.N1 ");
  if (GBITN(MemExists, 2)) printk("MM0.W ");
  if (GBITN(MemExists, 3)) printk("MM1.N0 ");
  if (GBITN(MemExists, 4)) printk("MM1.N1 ");
  if (GBITN(MemExists, 5))  printk("MM1.W ");
  printk("\n");

  hdl = SOC_SBX_CONTROL(unit)->sbhdl;

  rv = sbFe2kxtLineRateMemTestSetup(p);
  if (rv != SB_OK) {
    printk("ERROR sbFe2kxtLineRateMemTestSetup failed(%d)\n",rv);
    return (-1);
  }

  /* bring the LRP down, and backup to good state */

  SAND_HAL_RMW_FIELD(hdl, C2, PC_CORE_RESET, LR_CORE_RESET,1);
  printk("Resetting LR registers.. \n");
  if (0 != (rv = fe2kxt_set_lrp_default_regs(unit))) {
    printk("Setting LR registers failed(%d)\n",rv);
    test_error(unit,"%s: failed LRP memory test. \n",SOC_CHIP_STRING(unit));
    return (rv);
  }
  SAND_HAL_RMW_FIELD(hdl, C2, PC_CORE_RESET, LR_CORE_RESET,0);

  /* clear out both banks to remove any g2p3 code */
  if (0 != (rv = fe2kxt_clear_lrp_banks(unit))) {
    printk("Clearing LRP banks failed.\n");
    test_error(unit,"%s failed to run LRP memory test.\n",SOC_CHIP_STRING(unit));
    return (rv);
  }

  fe2kXtAsm2IntD__initDefaultPhase0(&fe2kxtdiag_phase0);
  sbFe2000EnableLr(hdl,(uint8_t*)(&fe2kxtdiag_phase0));

 /*  LRP is not doing any header processing in this test , routine above will
  *  set LOAD_ENABLE so unset it here.
  */

  SAND_HAL_RMW_FIELD(hdl, C2, LR_CONFIG0, LOAD_ENABLE,0);
  /* VxWorks build seems to have defined UPDATE */
#ifdef UPDATE
#define _SB_UPDATE_SAVE UPDATE
#undef UPDATE
#endif
  SAND_HAL_RMW_FIELD(hdl, C2, LR_CONFIG0, UPDATE, 1);
#ifdef _SB_UPDATE_SAVE
#define UPDATE _SB_UPDATE_SAVE
#endif

  /*
   * NOTES:
   * sbFe2000UcodeLoad also does a bank swap, for this reason
   * we need to load the ucode twice for each phase. The first
   * load will put phase0 into the inactive bank and then swap
   * out whatever was in the active bank to the inactive
   * bank. The test needs to modify the first instruction in IMEM
   * for each phase so we need to load the phase again to the inactive
   * bank and then swap. On the second call to UcodeLoad both
   * the inactive and active bank will have the same mcode. Then during
   * EnableProcessing() we remove the first jump from the inactive
   * bank and swap out to run that phase of the test.
   */

  /* phase 0 */
  printk("Loading phase0 microcode.. ");
  for (i = 0 ; i <= 1; i++) {
    status = sbFe2000UcodeLoad(hdl,(uint8_t*)&fe2kxtdiag_phase0);
    if(status != SB_OK) {
      printk("ERROR loading phase0 microcode (%d) %s\n",
	     status,bcm_errmsg(status));
      test_error(unit,"%s: failed LRP memory test. \n",SOC_CHIP_STRING(unit));
      return status;
    }
  }

  printk("OK\n");
  thin_delay(2000); /* for for a few cycles in NOP mode */
  sbFe2000DiagUpdateLrpEpoch(hdl,669);
  Fe2kxt_SetupCtlRegs(p);
  Fe2kxt_EnableProcessing(p); /* clear first jmp instruction, swap banks */

  timeout = Fe2kxt_WaitForPhase(hdl,1);

  if (timeout) {
    printk("ERROR: first phase timed out, bailing\n");
    test_error(unit,"%s: failed LRP memory test. \n",SOC_CHIP_STRING(unit));    
    return (-1);
  }

  err += Fe2kxt_CheckErrors(hdl,seed,1);

  if (err) {
    printk("MEMORY ERRORS DETECTED = %d\n",err);
    printk("Skipping second phase, since phase one had errors\n");
    test_error(unit,"%s: failed LRP memory test. \n",SOC_CHIP_STRING(unit));    
    return (-1);
  }

  /* phase 1 */
  printk("Loading phase1 microcode.. ");
  fe2kXtAsm2IntD__initDefaultPhase1(&fe2kxtdiag_phase1);

  for (i = 0; i <= 1; i++ ) {
    status = sbFe2000UcodeLoad(hdl,(uint8_t*)&fe2kxtdiag_phase1);
    if(status != SB_OK) {
      printk("ERROR loading phase1 microcode (%d) %s \n",
	     status,bcm_errmsg(status));
      test_error(unit,"%s: failed LRP memory test. \n",SOC_CHIP_STRING(unit));
      return status;
    }
  }

  printk("OK\n");
  thin_delay(2000); 
  sbFe2000DiagUpdateLrpEpoch(hdl,613);
  Fe2kxt_SetupCtlRegs(p);
  Fe2kxt_EnableProcessing(p); 

  timeout = Fe2kxt_WaitForPhase(hdl,2);

  if (timeout) {
    printk("ERROR: second phase timed out, bailing\n");
    test_error(unit,"%s: failed LRP memory test. \n",SOC_CHIP_STRING(unit));    
    return (-1);
  }

  err += Fe2kxt_CheckErrors(hdl,seed,2);

  if (err) {
    printk("MEMORY ERRORS DETECTED = %d\n",err);
    printk("Skipping third phase, since phase two had errors\n");
    test_error(unit,"%s: failed LRP memory test. \n",SOC_CHIP_STRING(unit));    
    return (-1);
  }

  /* phase 2 */
  printk("Loading phase2 microcode.. ");
  fe2kXtAsm2IntD__initDefaultPhase2(&fe2kxtdiag_phase2);

  for (i = 0; i <= 1; i++ ) {
    status = sbFe2000UcodeLoad(hdl,(uint8_t*)&fe2kxtdiag_phase2);
    if(status != SB_OK) {
      printk("ERROR loading phase2 microcode (%d) %s\n",
	     status,bcm_errmsg(status));
      test_error(unit,"%s: failed LRP memory test. \n",SOC_CHIP_STRING(unit));
      return status;
    }
  }

  printk("OK\n");
  thin_delay(2000);
  sbFe2000DiagUpdateLrpEpoch(hdl,527);
  Fe2kxt_SetupCtlRegs(p);
  Fe2kxt_EnableProcessing(p);

  timeout = Fe2kxt_WaitForPhase(hdl,3);
  if (timeout) {
    printk("ERROR: Third phase timed out, bailing\n");
    test_error(unit,"%s: failed LRP memory test. \n",SOC_CHIP_STRING(unit));    
    return (-1);
  }

  err += Fe2kxt_CheckErrors(hdl,seed,3);
  printk("MEMORY ERRORS DETECTED = %d\n",err);

  if (err != 0 ) {
    test_error(unit,"%s: failed LRP memory test. \n",SOC_CHIP_STRING(unit));    
    return (-1);
  } else {
    return 0;
  }
}

int
sbFe2kxtLineRateMemTestSetup(sbxDiagsInfo_t *pDiagsInfo)
{
  uint32_t nMmu;
  uint32_t ulReg;
  sbhandle hdl;
  uint32_t uMmClientConfig=0;

  hdl = pDiagsInfo->userDeviceHandle;

  /* bms - Nov 02 2006 - this will set up all of the interfaces for the MM */
  /* bms - Nov 02 2006 - 2x narrow, 1x wide, 2x internal per MMU */
  for( nMmu = 0; nMmu < 2 /*SB_FE2000XT_NUM_MM_INSTANCES*/; nMmu++ ) {
    uMmClientConfig = SAND_HAL_READ_STRIDE(hdl,C2,MM,nMmu,MM_CLIENT_CONFIG);
    uMmClientConfig = SAND_HAL_MOD_FIELD(C2,MM_CLIENT_CONFIG, LRP0_MEMORY_CONFIG, uMmClientConfig,SB_FE2000XT_MM_LRP0_CONNECTED_TO_NARROWPORT_DDRII_PORT0);
    uMmClientConfig = SAND_HAL_MOD_FIELD(C2,MM_CLIENT_CONFIG, LRP1_MEMORY_CONFIG, uMmClientConfig,SB_FE2000XT_MM_LRP1_CONNECTED_TO_NARROWPORT_DDRII_PORT1);
    uMmClientConfig = SAND_HAL_MOD_FIELD(C2,MM_CLIENT_CONFIG, LRP2_MEMORY_CONFIG, uMmClientConfig,SB_FE2000XT_MM_LRP2_CONNECTED_TO_INTERNAL_SINGLE_RAM0);
    uMmClientConfig = SAND_HAL_MOD_FIELD(C2,MM_CLIENT_CONFIG, LRP3_MEMORY_CONFIG, uMmClientConfig,SB_FE2000XT_MM_LRP3_CONNECTED_TO_INTERNAL_SINGLE_RAM1);
    uMmClientConfig = SAND_HAL_MOD_FIELD(C2,MM_CLIENT_CONFIG, LRP4_MEMORY_CONFIG, uMmClientConfig,SB_FE2000XT_MM_LRP4_CONNECTED_TO_WIDEPORT_DDRII_PORT);
    uMmClientConfig = SAND_HAL_MOD_FIELD(C2,MM_CLIENT_CONFIG, PMU_MEMORY_CONFIG, uMmClientConfig,SB_FE2000XT_MM_PMU_CONNECTED_TO_WIDEPORT_DDRII);
    SAND_HAL_WRITE_STRIDE(hdl,C2,MM,nMmu, MM_CLIENT_CONFIG,uMmClientConfig);

  }


  SAND_HAL_RMW_FIELD(hdl, C2, PR_PRE0_CONFIG, PB1_PREFETCH_PAGES,14);
  SAND_HAL_RMW_FIELD(hdl, C2, PR_PRE0_CONFIG, PB0_PREFETCH_PAGES,14);
  SAND_HAL_RMW_FIELD(hdl, C2, PR_PRE1_CONFIG, PB1_PREFETCH_PAGES,14);
  SAND_HAL_RMW_FIELD(hdl, C2, PR_PRE1_CONFIG, PB0_PREFETCH_PAGES,14);

  /* stop counter */
  soc_sbx_counter_stop(pDiagsInfo->unit);

  /* disable the policer */
  ulReg = SAND_HAL_READ(hdl, C2, PC_CORE_RESET);
  ulReg = SAND_HAL_MOD_FIELD(C2, PC_CORE_RESET, PM_CORE_RESET, ulReg, 1);
  SAND_HAL_WRITE(hdl, C2, PC_CORE_RESET, ulReg);

  return 0;
}

void
Fe2kxt_SetupCtlRegs(sbxDiagsInfo_t *pDiagsInfo)
{

  sbZfC2DiagUcodeShared_t zfSharedReg;
  sbZfC2DiagUcodeCtl_t zfCtlReg;
  uint32_t ulCtlReg = 0;
  uint8_t ulCtlRegBytes[4] = {0};
  uint32_t uLrp = 0;
  uint32_t ulSharedReg = 0;
  uint8_t ulSharedRegBytes[4] = {0};
  uint32_t ulEMask = pDiagsInfo->uMemExistsMask;
  int32_t seed = pDiagsInfo->seed;
  sbhandle hdl = pDiagsInfo->userDeviceHandle;

  printk("Setting up ctrl regs using seed=0x%x\n",seed);

  sbZfC2DiagUcodeShared_InitInstance(&zfSharedReg);
  sbZfC2DiagUcodeCtl_InitInstance(&zfCtlReg);

  zfCtlReg.m_ulStatus = 0;
  zfCtlReg.m_ulErrInject0 = 0;
  zfCtlReg.m_ulErrInject1 = 0;
  zfCtlReg.m_ulErrInd = 0;
  zfCtlReg.m_ulUcLoaded = 0;
  zfCtlReg.m_ulLrpState = 0;
  zfCtlReg.m_ulTestOffset = 0;
  zfCtlReg.m_ulMemExstMm0N0 = GBITN(ulEMask, 0);
  zfCtlReg.m_ulMemExstMm0N1 = GBITN(ulEMask, 1);
  zfCtlReg.m_ulMemExstMm0W  = GBITN(ulEMask, 2);
  zfCtlReg.m_ulMemExstMm1N0 = GBITN(ulEMask, 3);
  zfCtlReg.m_ulMemExstMm1N1 = GBITN(ulEMask, 4);
  zfCtlReg.m_ulMemExstMm1W  = GBITN(ulEMask, 5);


  sbZfC2DiagUcodeCtl_Pack(&zfCtlReg,&ulCtlRegBytes[0],1);
  C2SetDataWord(&ulCtlReg,&ulCtlRegBytes[0]);

  for(uLrp = 0; uLrp < 48; uLrp++) {
    /* printk("Writing MM0 Internal0 memory entry at address=0x%08x -- 0x%08x\n",uLrp,ulCtlReg); */
    Fe2kxt_WrI(hdl,0, uLrp, ulCtlReg);

    /* clear out error information */
    Fe2kxt_WrI(hdl,0, uLrp + ERR_A, 0);
    Fe2kxt_WrI(hdl,0, uLrp + ERR_D0, 0);
    Fe2kxt_WrI(hdl,0, uLrp + ERR_D1, 0);
  }

  /* define ram sizes and seed */
  zfSharedReg.m_ulMemSizeMm0N0 = Fe2kxt_MemSizeEnc(pDiagsInfo->np0_config.addr_width);
  zfSharedReg.m_ulMemSizeMm0N1 = Fe2kxt_MemSizeEnc(pDiagsInfo->np1_config.addr_width);
  zfSharedReg.m_ulMemSizeMm0W  = Fe2kxt_MemSizeEnc(pDiagsInfo->wp_config.addr_width);
  zfSharedReg.m_ulMemSizeMm1N0 = Fe2kxt_MemSizeEnc(pDiagsInfo->np0_config.addr_width);
  zfSharedReg.m_ulMemSizeMm1N1 = Fe2kxt_MemSizeEnc(pDiagsInfo->np1_config.addr_width);
  zfSharedReg.m_ulMemSizeMm1W  = Fe2kxt_MemSizeEnc(pDiagsInfo->wp_config.addr_width);
  zfSharedReg.m_ulRandomSeed   = seed;

  sbZfC2DiagUcodeShared_Pack(&zfSharedReg,&ulSharedRegBytes[0],1);
  C2SetDataWord(&ulSharedReg,&ulSharedRegBytes[0]);

  /* write out shared config entry (address 0x0 in int mem 1) */
  /* printk("Writing MM0 Internal1 memory entry at address 0x%08x -- 0x%08x\n",0,ulSharedReg); */
   Fe2kxt_WrI(hdl,1, 0, ulSharedReg);

  Fe2kxt_WrI(hdl,2,0,RAM_SIZE_BASE);

}

/* write internal */
void
Fe2kxt_WrI(sbhandle hdl, uint32_t mem, uint32_t offs, uint32_t data)
{

  switch (mem) {
  case 0:
    {
      sbZfC2MmInternal0MemoryEntry_t zData;
      sbZfC2MmInternal0MemoryEntry_InitInstance(&zData);
      zData.m_uData0 = data;
      C2Mm0Internal0MemoryWrite(hdl,offs, &zData);
    }
    break;
  case 1:
    {
      sbZfC2MmInternal1MemoryEntry_t zData;
      sbZfC2MmInternal1MemoryEntry_InitInstance(&zData);
      zData.m_uData0 = data;
      C2Mm0Internal1MemoryWrite(hdl,offs, &zData);
    }
    break;
  case 2:
    {
      sbZfC2MmInternal0MemoryEntry_t zData;
      sbZfC2MmInternal0MemoryEntry_InitInstance(&zData);
      zData.m_uData0 = data;
      C2Mm1Internal0MemoryWrite(hdl,offs, &zData);
    }
    break;
  case 3:
    {
      sbZfC2MmInternal1MemoryEntry_t zData;
      sbZfC2MmInternal1MemoryEntry_InitInstance(&zData);
      zData.m_uData0 = data;
      C2Mm1Internal1MemoryWrite(hdl,offs, &zData);
    }
    break;
  default:
    SB_ASSERT(0);
    break;
  }
}

/* read internal */
uint32_t
Fe2kxt_RdI(sbhandle hdl,uint32_t mem, uint32_t offs)
{
  switch (mem) {
  case 0:
    {
      sbZfC2MmInternal0MemoryEntry_t zData;
      C2Mm0Internal0MemoryRead(hdl,offs, &zData);
      return zData.m_uData0;
    }
    break;
  case 1:
    {
      sbZfC2MmInternal1MemoryEntry_t zData;
      C2Mm0Internal1MemoryRead(hdl,offs, &zData);
      return zData.m_uData0;
    }
    break;
  case 2:
    {
      sbZfC2MmInternal0MemoryEntry_t zData;
      C2Mm1Internal0MemoryRead(hdl,offs, &zData);
      return zData.m_uData0;
    }
    break;
  case 3:
    {
      sbZfC2MmInternal1MemoryEntry_t zData;
      C2Mm1Internal1MemoryRead(hdl,offs, &zData);
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
Fe2kxt_MemSizeEnc(uint32_t bits)
{
  SB_ASSERT(bits >= RAM_SIZE_BASE);
  SB_ASSERT(bits - RAM_SIZE_BASE <= 0x7);
  return bits - RAM_SIZE_BASE;
}

uint32_t
Fe2kxt_WaitForPhase(sbhandle hdl,uint32_t phase)
{

  uint32_t ulCtlReg, ulTimeout = 0, ulDone, ulOffs, ulStage;
  uint32_t i;
  while (1) {
    ulDone = 1;
    for (i = 0; (i < MAX_PES && ulDone); i++) {
      ulCtlReg = Fe2kxt_RdI(hdl,0, 0 + i);
      if ((ulCtlReg & 0x1) == 0)
	ulDone = 0;
    }
    ulOffs = 48 * (ulCtlReg >> 13);
    /* printk("Phase:%d Testing is at offset 0x%08x, ulCtlReg=0x%08x\n",phase,ulOffs,ulCtlReg); */
    if (ulDone) {
      ulStage = (ulCtlReg >> 5) & 0x3;
      printk( "Finished (%d of 3) stages\n",ulStage);
      if (ulStage != phase)
	printk("ERROR: Register failed to show correct phase expt %d actual %d\n", phase, ulStage);
      return 0;
    }
    if (ulTimeout++ > 100000) {
      printk("ERROR: Timed out waiting for test to finish\n");
      return 1;
    }
  }
}

uint32_t
Fe2kxt_CheckErrors(sbhandle hdl,int32_t seed,uint32_t phase)
{
  uint32_t err = 0;
  uint32_t ulCtlReg, i;
  uint32_t ulReg,ulAddr,ulPort,ulD0,ulD1;
  uint32_t ulE0,ulE1;

  /* check for errors */
  for ( i = 0; i < MAX_PES; i++) {
    ulCtlReg = Fe2kxt_RdI(hdl,0, 0 + i);
    if (ulCtlReg & 0x8) {
      err = 1;
      /* read {port, addr, data-lo, data-hi} from memory */
      ulReg = Fe2kxt_RdI(hdl,0, ERR_A + i);
      ulAddr = ulReg & 0xfffFFFF;
      ulPort = ulReg >> 28;
      ulD0 = Fe2kxt_RdI(hdl,0, ERR_D0 + i);
      ulD1 = Fe2kxt_RdI(hdl,0, ERR_D1 + i);

      /* calculated expected */
      ulE0 = Fe2kxt_HashData(seed, ulAddr, 0);
      ulE1 = Fe2kxt_HashData(seed, ulAddr, 1);

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

      printk("ERROR: PE %d encountered an error\n", i);
      printk("ERROR: Addr = 0x%08x\n", ulAddr);
      printk("ERROR: Memory = %s \n", MemStr[ulPort]);
      if (ulPort < 4) {
	printk("ERROR: read 0x%08x expt 0x%08x xor 0x%08x\n",
	       ulD0, ulE0, ulD0 ^ ulE0);
      } else {
	printk("ERROR: read 0x%08x%08x expt 0x%08x%08x xor 0x%08x%08x\n",
	       ulD1, ulD0, ulE1, ulE0, ulD1 ^ ulE1, ulD0 ^ ulE0);
      }
    }
  }
  return err;
}

uint32_t
Fe2kxt_HashData(int32_t seed, uint32_t ulOffs, uint32_t Inv)
{
  int32_t seedEx;

  SB_ASSERT(seed < (0x1 << 14));

  seedEx = (seed << 28) | (seed << 14) | seed;

  if (Inv)
    seedEx = ~seedEx;

  return Fe2kxt_crc32(ulOffs + seedEx);
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
Fe2kxt_crc32(uint32_t x)
{
  uint32_t i;

  for (i = 0; i < 4; i++)
    x = (x << 8) ^ bytetable[x >> 24];
  return x;
}

/* set the lrp registers to default values */
int
fe2kxt_set_lrp_default_regs(int unit)
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
      printk("%s ERROR:  Out of Memory \n",FUNCTION_NAME());
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

      sbx_diag_read_reg(unit,reg_info,&tmp); /* clears any clear-on-read fields */
      sbx_diag_write_reg(unit,reg_info,tmp); /* clears any write 1 to clear fields */
      /* write the default value */
      /* printk("Setting Reg:%s to 0x%x\n",reg_info->name,regval); */
      rv = sbx_diag_write_reg(unit,reg_info,regval);
      if (rv) {
	printk("sbx_diag_write_reg failed(%d)\n",rv);
	sal_free(Fe2k_lrp_regs_l);
	return (rv);
      }
      regval = 0;
    }
  }

  sal_free(Fe2k_lrp_regs_l);
  return 0;
}

int Fe2kXtDiagsSetErrorRegList(sbxDiagsInfo_t *pDiagsInfo,uint32_t unit) {
  sbhandle hdl;
  int idx = 0;
  int i = 0;
  soc_sbx_chip_info_t *chip_info = NULL;
  soc_sbx_reg_info_t *reg_info = NULL;
  /* these regs should all be zero */
  const char* err_list[FE2KXT_ERR_LIST_CNT] = {"fcs_error","jabber","fifo_status","fragmented",
					       "undersize","mtu_exceeded","false_carrier",
					       "invalid_data","bad_length","aligment_error",
					       "unknown_ctrl","enq_drop_bit","pr_pre3_error","pt_pte3_error",
					       "pr_pre2_error","pt_pte2_error","pr_pre4_error","pt_pte4_error",
					       "pt_pte5_error","pr_pre5_error","oversize","undersize","underflow",
					       "wrong_sop","frame_error","error_pkt","unknown_ctrl","dropped"};

  hdl = SOC_SBX_CONTROL(unit)->sbhdl;
  if (!gFe2kxt_reginfo_l) {
    gFe2kxt_reginfo_l = sal_alloc(sizeof (soc_sbx_reg_info_list_t), "reginfo_l");
    if (!gFe2kxt_reginfo_l) {
      printk("ERROR: sbx_reg_list failed. Out of Memory \n");
      return CMD_FAIL;
    }
  }
  gFe2kxt_reginfo_l->count = 0;
  if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info,1) != CMD_OK) {
    printk("ERROR: Register info unknown for unit %d \n", unit);
    sal_free(gFe2kxt_reginfo_l);
    return (-1);
  }

  for(idx = 0; idx < chip_info->nregs; idx++) {
    reg_info = chip_info->regs[idx];
    for(i=0;i<FE2KXT_ERR_LIST_CNT;i++) {
      if (strstr(reg_info->name,err_list[i])) {
	gFe2kxt_reginfo_l->idx[gFe2kxt_reginfo_l->count++] = idx;
      }
    }
  }
  return 0;
}

int Fe2kXtDiagsClearErrors(sbxDiagsInfo_t *pDiagsInfo) {

  int idx = 0;
  int i = 0;
  soc_sbx_chip_info_t *chip_info = NULL;
  soc_sbx_reg_info_t *reg_info = NULL;
  int rv = 0;
  int unit = pDiagsInfo->unit;
  uint32 regval = 0;

  if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info,1) != CMD_OK) {
    printk("ERROR: Register info unknown for unit %d \n", unit);
    sal_free(gFe2kxt_reginfo_l);
    return (-1);
  }

  sbx_fe2000_reg_list_prune_reset(unit,chip_info,gFe2kxt_reginfo_l);

  for(i=0;i< gFe2kxt_reginfo_l->count;i++) {
    idx = gFe2kxt_reginfo_l->idx[i];
    if (idx >= 0) {
      reg_info = chip_info->regs[idx];
      rv = sbx_diag_read_reg(unit, reg_info, &regval);
      if (rv != CMD_OK) {
	return rv;
      }
      if (regval != 0) {
	/* Clear W1TC fields */
	rv = sbx_diag_write_reg(unit, reg_info, regval);
	if (rv != CMD_OK) {
	  return rv;
	}
	/* Clear other fields (e.g. counters) */
	rv = sbx_diag_write_reg(unit, reg_info, 0);
	if (rv != CMD_OK) {
	  return rv;
	}
      }
    }
  }
  return rv;
}

int Fe2kXtDiagsClearCounters(sbxDiagsInfo_t *pDiagsInfo) {

  int idx = 0;
  int i = 0;
  soc_sbx_chip_info_t *chip_info = NULL;
  soc_sbx_reg_info_t *reg_info = NULL;
  soc_sbx_reg_info_list_t *reginfo_l = NULL;
  int rv = 0;
  int unit = pDiagsInfo->unit;
  uint32 regval = 0;
  pbmp_t pbmp;
  bcm_port_t port;

  if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info,1) != CMD_OK) {
    printk("ERROR: Register info unknown for unit %d \n", unit);
    sal_free(gFe2kxt_reginfo_l);
    return (-1);
  }

  reginfo_l = sal_alloc(sizeof (soc_sbx_reg_info_list_t), "reginfo_l");
  if (!reginfo_l) {
    printk("ERROR: sbx_reg_list failed. Out of Memory \n");
    return CMD_FAIL;
  }

  reginfo_l->count = 0;

  if (sbx_reg_info_list_get(chip_info, reginfo_l, "cnt", 0) != CMD_OK) {
    sal_free(reginfo_l);
    return CMD_FAIL;
  }

  sbx_fe2000_reg_list_prune_reset(unit,chip_info,reginfo_l);

  for(i=0;i< reginfo_l->count;i++) {
    idx = reginfo_l->idx[i];
    if (idx >= 0) {
      reg_info = chip_info->regs[idx];
      rv = sbx_diag_read_reg(unit, reg_info, &regval);
      if (rv != CMD_OK) {
	return rv;
      }
      if (regval != 0) {
	/* Clear W1TC fields */
	rv = sbx_diag_write_reg(unit, reg_info, regval);
	if (rv != CMD_OK) {
	  return rv;
	}
	/* Clear other fields (e.g. counters) */
	rv = sbx_diag_write_reg(unit, reg_info, 0);
	if (rv != CMD_OK) {
	  return rv;
	}
      }
    }
  }

  /* Clear software counter statistics */
  BCM_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
  PBMP_ITER(pbmp, port) {
    bcm_stat_clear(unit, port);
  }

  sal_free(reginfo_l);
  return rv;
}

int Fe2kXtDiagsCheckForErrors(uint32_t unit) {

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

  for(i = 0; i < gFe2kxt_reginfo_l->count; i++) {
    idx = gFe2kxt_reginfo_l->idx[i];
    if (idx >= 0) {
      reg_info = chip_info->regs[idx];
      rv = sbx_diag_read_reg(unit,reg_info,&regval);
      if (rv != CMD_OK) {
	printk("ERROR: sbx_diag_read_reg returned(%d)\n",rv);
	return (-1);
      }
      if (regval != 0) {
	/* there is a an error , print it out */
	printk("\nReg:%s has errors 0x%x\n",reg_info->name,regval);
	rv = sbx_diag_write_reg(unit, reg_info, regval);
	if (rv != CMD_OK) {
	  printk("ERROR: sbx_diag_reg_write returned(%d)\n",rv);
	  return (-1);
	}
	return (-1);
      }
    }
  }
  return 0;
}



int Fe2kXtDiagsPollForErrors(uint32_t sp, uint32_t ep,uint32_t unit) {
  sbhandle hdl;
  int rv = 0;
  uint32_t uData = 0;
  uint32_t uRetry;
  uint8_t bAG0_Incrementing = 0;
  uint8_t bAG1_Incrementing = 0;

  hdl = SOC_SBX_CONTROL(unit)->sbhdl;

  rv = Fe2kXtDiagsCheckForErrors(unit);
  if (rv != 0) {
    return rv;
  }

  if (sp == XE0_PORT ||
      sp == XE1_PORT ||
      sp == XE2_PORT ||
      sp == XE3_PORT) {
    rv = sbFe2kxtPoll10GCounters(unit,sp,ep);
    if (rv != 0) {
      return rv;
    }
  } else {
    /* Based on start port and end port we should see AG0 and/or AG1 Tx packet counts always incrementing */
    if (sp < 11) {
      uRetry=600;
      while ( --uRetry && !bAG0_Incrementing) {
	uData = SAND_HAL_READ(hdl,C2,PT_PTE2_PKT_CNT);
	if (uAGM0_TxCount+uData <= uAGM0_TxCount) { /* if this count <= last count, pkt stopped */
	  thin_delay(300); continue;
	}  else {
	  uAGM0_TxCount += uData;
	  bAG0_Incrementing = 1;
	}
      }
      if (uRetry == 0) {
	printk("\nERROR AG0: looping packet stopped unexpectedly\n");
	rv = -1;
      }
    }

    if (ep > 11) {
      uRetry=600;
      while ( --uRetry && !bAG1_Incrementing) {
	uData = SAND_HAL_READ(hdl,C2,PT_PTE3_PKT_CNT);
	if (uAGM1_TxCount+uData <= uAGM1_TxCount) { /* if this count <= last count, pkt stopped */
	  thin_delay(300); continue;
	}  else {
	  uAGM1_TxCount += uData;
	  bAG1_Incrementing = 1;
	}
      }

      if (uRetry == 0) {
	printk("\nERROR AG1: looping packet stopped unexpectedly\n");
	rv = -1;
      }

    }

    uData = SAND_HAL_READ(hdl,C2,PR_PRE2_PKT_CNT);
    uAGM0_RxCount += uData;

    uData = SAND_HAL_READ(hdl,C2,PR_PRE3_PKT_CNT);
    uAGM1_RxCount += uData;

    printk("AGM0 Rx:%lld Tx:%lld, AGM1 Rx:%lld Tx:%lld\r",uAGM0_RxCount,uAGM0_TxCount,
	   uAGM1_RxCount, uAGM1_TxCount);
  }
  return rv;
}

uint32_t  sbFe2kXtDiagsReadTxPktCount(sbhandle hdl,int pt) {
 uint32_t uData = 0;
 switch(pt) {
 case 2:
   uData = SAND_HAL_READ(hdl,C2,PT_PTE2_PKT_CNT);
   break;
 case 3:
   uData = SAND_HAL_READ(hdl,C2,PT_PTE3_PKT_CNT);
   break;
 case 4:
   uData = SAND_HAL_READ(hdl,C2,PT_PTE4_PKT_CNT);
   break;
 case 5:
   uData = SAND_HAL_READ(hdl,C2,PT_PTE5_PKT_CNT);
 }
 return uData;
}

uint32_t  sbFe2kXtDiagsReadRxPktCount(sbhandle hdl,int pr) {
  uint32_t uData = 0;
  switch(pr) {
  case 2:
    uData = SAND_HAL_READ(hdl,C2,PR_PRE2_PKT_CNT);
    break;
  case 3:
    uData = SAND_HAL_READ(hdl,C2,PR_PRE3_PKT_CNT);
    break;
  case 4:
    uData = SAND_HAL_READ(hdl,C2,PR_PRE4_PKT_CNT);
    break;
  case 5:
    uData = SAND_HAL_READ(hdl,C2,PR_PRE5_PKT_CNT);
  }
  return uData;
}


/* bist test done like in system_sim/caladan2/units/c2/tb/c2_t024.v */
int32_t sbFe2kxtDiagsBistStart(sbxDiagsInfo_t *pDiagsInfo)
{

  int rv = SOC_E_NONE;
  int status = 0;
  sbhandle userDeviceHandle = pDiagsInfo->userDeviceHandle;
  int unit = pDiagsInfo->unit;

  /* take core out of reset */
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, PR_CORE_RESET, 0x0);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, PP_CORE_RESET, 0x0);

    
  if (( rv = sbFe2kxtDiagsCC2Bist(pDiagsInfo)) < 0 ) {
    test_error(unit,
	       "%s: CC2 Bist test failed. \n",SOC_CHIP_STRING(unit));
    status = -1;
  } 

  if (( rv = sbFe2kxtDiagsCC3Bist(pDiagsInfo)) < 0 ) {
    test_error(unit,
	       "%s: CC3 Bist test failed. \n",SOC_CHIP_STRING(unit));
    status = -1;
  }  

  if (( rv = sbFe2kxtDiagsCC4Bist(pDiagsInfo)) < 0 ) {
    test_error(unit,
	       "%s: CC4 Bist test failed. \n",SOC_CHIP_STRING(unit));
    status = -1;
  }  

  if (( rv = sbFe2kxtDiagsCC5Bist(pDiagsInfo)) < 0 ) {
    test_error(unit,
	       "%s: CC5 Bist test failed. \n",SOC_CHIP_STRING(unit));
    status = -1;
  }  

  if (( rv = sbFe2kxtDiagsPPCAM0Bist(pDiagsInfo)) < 0 ) {
    test_error(unit,
	       "%s: PP CAM0 Bist test failed. \n",SOC_CHIP_STRING(unit));
    status = -1;
  }  

  if (( rv = sbFe2kxtDiagsPPCAM1Bist(pDiagsInfo)) < 0 ) {
    test_error(unit,
	       "%s: PP CAM1 Bist test failed. \n",SOC_CHIP_STRING(unit));
    status = -1;
  }  

  if (( rv = sbFe2kxtDiagsPPCAM2Bist(pDiagsInfo)) < 0 ) {
    test_error(unit,
	       "%s: PP CAM2 Bist test failed. \n",SOC_CHIP_STRING(unit));
    status = -1;
  }  

  if (( rv = sbFe2kxtDiagsPPCAM3Bist(pDiagsInfo)) < 0 ) {
    test_error(unit,
	       "%s: PP CAM3 Bist test failed. \n",SOC_CHIP_STRING(unit));
    status = -1;
  }  

  if (status == 0) {
    printk("Bist tests passed.\n");
  }
  return status;
}

/* bist test, CC2,CC3,CC4,CC5 functions */
int32_t sbFe2kxtDiagsCC2Bist(sbxDiagsInfo_t *pDiagsInfo)
{

  uint32_t uData = 0;
  int bistdone = 0;
  int cambist_wait_usec = 0;
  int status = 0;
  soc_timeout_t to;
  sbhandle userDeviceHandle = pDiagsInfo->userDeviceHandle;

  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2,PR_CC2_CAMBIST_CONFIG,BIST_EN,uData,0);
  SAND_HAL_WRITE(userDeviceHandle,C2,PR_CC2_CAMBIST_CONFIG,uData);
  thin_delay(100);

  /* check to see all registers default to 0 with bist off */
  uData = SAND_HAL_READ(userDeviceHandle,C2,PR_CC2_CAMBIST_STATUS);
  if (uData !=0) {
    printk("ERROR: CC2_CAMBIST_STATUS != 0 at start of test\n");
    return (-1);
  }
    
  uData = SAND_HAL_READ(userDeviceHandle,C2,PR_CC2_CAMBIST_CONFIG);
  if (uData !=0) {
    printk("ERROR: CC2_CAMBIST_CONFIG != 0 at start of test\n");
    return (-1);
  }

  uData = SAND_HAL_READ(userDeviceHandle,C2,PR_CC2_CAMBIST_DATA);
  if (uData !=0) {
    printk("ERROR: CC2_CAMBIST_DATA != 0 at start of test\n");
    return (-1);
  }

  /* enable bist testing */
  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2,PR_CC2_CAMBIST_CONFIG,BIST_DBG_DATA_SLICE_OR_STATUS_SEL,uData,0);
  uData = SAND_HAL_MOD_FIELD(C2,PR_CC2_CAMBIST_CONFIG,BIST_EN,uData,1);
  SAND_HAL_WRITE(userDeviceHandle,C2,PR_CC2_CAMBIST_CONFIG,uData);

  /* wait for bist done */
  cambist_wait_usec = CAM_BIST_WAIT;
  bistdone = 0;
  soc_timeout_init(&to,cambist_wait_usec,0);
  while (!soc_timeout_check(&to) && !bistdone) {
    uData = SAND_HAL_READ(userDeviceHandle,C2,PR_CC2_CAMBIST_STATUS);
    bistdone = uData & 1;
  }

  if (bistdone) {
    /* check for pass status */
    printk("CC2 Bist test done in %d usec,  ",soc_timeout_elapsed(&to));
    uData = SAND_HAL_READ(userDeviceHandle,C2,PR_CC2_CAMBIST_STATUS);
    if (uData & 2) {
      printk("PASSED\n");
    } else {
      printk("FAILED\n");
      status = -1;
    }
  } else {
    printk("CC2 bist test timedout\n");
    return status = -1;
  }

  /* cleanup */
  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2,PR_CC2_CAMBIST_CONFIG,BIST_EN,uData,0);
  SAND_HAL_WRITE(userDeviceHandle,C2,PR_CC2_CAMBIST_CONFIG,uData);

  return status;
}

int32_t sbFe2kxtDiagsCC3Bist(sbxDiagsInfo_t *pDiagsInfo)
{

  uint32_t uData = 0;
  int bistdone = 0;
  int cambist_wait_usec = 0;
  int status = 0;
  soc_timeout_t to;
  sbhandle userDeviceHandle = pDiagsInfo->userDeviceHandle;

  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2,PR_CC3_CAMBIST_CONFIG,BIST_EN,uData,0);
  SAND_HAL_WRITE(userDeviceHandle,C2,PR_CC3_CAMBIST_CONFIG,uData);
  thin_delay(100);

  /* check to see all registers default to 0 */
  uData = SAND_HAL_READ(userDeviceHandle,C2,PR_CC3_CAMBIST_STATUS);
  if (uData !=0) {
    printk("ERROR: CC3_CAMBIST_STATUS != 0 at start of test\n");
    return (-1);
  }
    
  uData = SAND_HAL_READ(userDeviceHandle,C2,PR_CC3_CAMBIST_CONFIG);
  if (uData !=0) {
    printk("ERROR: CC3_CAMBIST_CONFIG != 0 at start of test\n");
    return (-1);
  }

  uData = SAND_HAL_READ(userDeviceHandle,C2,PR_CC3_CAMBIST_DATA);
  if (uData !=0) {
    printk("ERROR: CC3_CAMBIST_DATA != 0 at start of test\n");
    return (-1);
  }

  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2,PR_CC3_CAMBIST_CONFIG,BIST_DBG_DATA_SLICE_OR_STATUS_SEL,uData,0);
  uData = SAND_HAL_MOD_FIELD(C2,PR_CC3_CAMBIST_CONFIG,BIST_EN,uData,1);
  SAND_HAL_WRITE(userDeviceHandle,C2,PR_CC3_CAMBIST_CONFIG,uData);

  /* wait for bist done */
  cambist_wait_usec = CAM_BIST_WAIT;
  bistdone = 0;
  soc_timeout_init(&to,cambist_wait_usec,0);
  while (!soc_timeout_check(&to) && !bistdone) {
    uData = SAND_HAL_READ(userDeviceHandle,C2,PR_CC3_CAMBIST_STATUS);
    bistdone = uData & 1;
  }

  if (bistdone) {
    /* check for pass status */
    printk("CC3 Bist test done in %d usec,  ",soc_timeout_elapsed(&to));
    uData = SAND_HAL_READ(userDeviceHandle,C2,PR_CC3_CAMBIST_STATUS);
    if (uData & 2) {
      printk("PASSED\n");
    } else {
      printk("FAILED\n");
      status = -1;
    }
  } else {
    printk("CC3 Bist test timedout\n");
    status = -1;
  }

  /* cleanup */
  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2,PR_CC3_CAMBIST_CONFIG,BIST_EN,uData,0);
  SAND_HAL_WRITE(userDeviceHandle,C2,PR_CC3_CAMBIST_CONFIG,uData);
  return status;
}

int32_t sbFe2kxtDiagsCC4Bist(sbxDiagsInfo_t *pDiagsInfo)
{

  uint32_t uData = 0;
  int bistdone = 0;
  int cambist_wait_usec = 0;
  int status = 0;
  soc_timeout_t to;
  sbhandle userDeviceHandle = pDiagsInfo->userDeviceHandle;

  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2,PR_CC4_CAMBIST_CONFIG,BIST_EN,uData,0);
  SAND_HAL_WRITE(userDeviceHandle,C2,PR_CC4_CAMBIST_CONFIG,uData);

  /* check to see all registers default to 0 */
  uData = SAND_HAL_READ(userDeviceHandle,C2,PR_CC4_CAMBIST_STATUS);
  if (uData !=0) {
    printk("ERROR: CC4_CAMBIST_STATUS != 0 at start of test\n");
    return (-1);
  }
    
  uData = SAND_HAL_READ(userDeviceHandle,C2,PR_CC4_CAMBIST_CONFIG);
  if (uData !=0) {
    printk("ERROR: CC4_CAMBIST_CONFIG != 0 at start of test\n");
    return (-1);
  }

  uData = SAND_HAL_READ(userDeviceHandle,C2,PR_CC4_CAMBIST_DATA);
  if (uData !=0) {
    printk("ERROR: CC4_CAMBIST_DATA != 0 at start of test\n");
    return (-1);
  }

  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2,PR_CC4_CAMBIST_CONFIG,BIST_DBG_DATA_SLICE_OR_STATUS_SEL,uData,0);
  uData = SAND_HAL_MOD_FIELD(C2,PR_CC4_CAMBIST_CONFIG,BIST_EN,uData,1);
  SAND_HAL_WRITE(userDeviceHandle,C2,PR_CC4_CAMBIST_CONFIG,uData);

  /* wait for bist done */
  cambist_wait_usec = CAM_BIST_WAIT;
  bistdone = 0;
  soc_timeout_init(&to,cambist_wait_usec,0);
  while (!soc_timeout_check(&to) && !bistdone) {
    uData = SAND_HAL_READ(userDeviceHandle,C2,PR_CC4_CAMBIST_STATUS);
    bistdone = uData & 1;
  }

  if (bistdone) {
    /* check for pass status */
    printk("CC4 Bist test done in %d usec,  ",soc_timeout_elapsed(&to));
    uData = SAND_HAL_READ(userDeviceHandle,C2,PR_CC4_CAMBIST_STATUS);
    if (uData & 2) {
      printk("PASSED\n");
    } else {
      printk("FAILED\n");
      status = -1;
    }
  } else {
    printk("CC4 Bist test timedout\n");
    status = -1;
  }

  /* cleanup */
  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2,PR_CC4_CAMBIST_CONFIG,BIST_EN,uData,0);
  SAND_HAL_WRITE(userDeviceHandle,C2,PR_CC4_CAMBIST_CONFIG,uData);
  return status;
}

int32_t sbFe2kxtDiagsCC5Bist(sbxDiagsInfo_t *pDiagsInfo)
{

  uint32_t uData = 0;
  int bistdone = 0;
  int cambist_wait_usec = 0;
  int status = 0;
  soc_timeout_t to;
  sbhandle userDeviceHandle = pDiagsInfo->userDeviceHandle;

  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2,PR_CC5_CAMBIST_CONFIG,BIST_EN,uData,0);
  SAND_HAL_WRITE(userDeviceHandle,C2,PR_CC5_CAMBIST_CONFIG,uData);

  /* check to see all registers default to 0 */
  uData = SAND_HAL_READ(userDeviceHandle,C2,PR_CC5_CAMBIST_STATUS);
  if (uData !=0) {
    printk("ERROR: CC5_CAMBIST_STATUS != 0 at start of test\n");
    return (-1);
  }
    
  uData = SAND_HAL_READ(userDeviceHandle,C2,PR_CC5_CAMBIST_CONFIG);
  if (uData !=0) {
    printk("ERROR: CC5_CAMBIST_CONFIG != 0 at start of test\n");
    return (-1);
  }

  uData = SAND_HAL_READ(userDeviceHandle,C2,PR_CC5_CAMBIST_DATA);
  if (uData !=0) {
    printk("ERROR: CC5_CAMBIST_DATA != 0 at start of test\n");
    return (-1);
  }

  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2,PR_CC5_CAMBIST_CONFIG,BIST_DBG_DATA_SLICE_OR_STATUS_SEL,uData,0);
  uData = SAND_HAL_MOD_FIELD(C2,PR_CC5_CAMBIST_CONFIG,BIST_EN,uData,1);
  SAND_HAL_WRITE(userDeviceHandle,C2,PR_CC5_CAMBIST_CONFIG,uData);

  /* wait for bist done */
  cambist_wait_usec = CAM_BIST_WAIT;
  bistdone = 0;
  soc_timeout_init(&to,cambist_wait_usec,0);
  while (!soc_timeout_check(&to) && !bistdone) {
    uData = SAND_HAL_READ(userDeviceHandle,C2,PR_CC5_CAMBIST_STATUS);
    bistdone = uData & 1;
  }

  if (bistdone) {
    /* check for pass status */
    printk("CC5 Bist test done in %d usec,  ",soc_timeout_elapsed(&to));
    uData = SAND_HAL_READ(userDeviceHandle,C2,PR_CC5_CAMBIST_STATUS);
    if (uData & 2) {
      printk("PASSED\n");
    } else {
      printk("FAILED\n");
      status = -1;
    }
  } else {
    printk("CC5 Bist test timedout\n");
    status = -1;
  }

  /* cleanup */
  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2,PR_CC5_CAMBIST_CONFIG,BIST_EN,uData,0);
  SAND_HAL_WRITE(userDeviceHandle,C2,PR_CC5_CAMBIST_CONFIG,uData);

  return status;
}

int32_t sbFe2kxtDiagsPPCAM0Bist(sbxDiagsInfo_t *pDiagsInfo)
{

  uint32_t uData = 0;
  int bistdone = 0;
  int cambist_wait_usec = 0;
  int status = 0;
  soc_timeout_t to;
  sbhandle userDeviceHandle = pDiagsInfo->userDeviceHandle;

  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2,PP_CAMBIST_CONFIG0,BIST_EN,uData,0);
  SAND_HAL_WRITE(userDeviceHandle,C2,PP_CAMBIST_CONFIG0,uData);
  thin_delay(200);

  /* check to see all registers default to 0 */
  uData = SAND_HAL_READ(userDeviceHandle,C2,PP_CAMBIST_STATUS0);
  if (uData !=0) {
    printk("ERROR: PP_CAMBIST_STATUS0 != 0 at start of test\n");
    return (-1);
  }
    
  uData = SAND_HAL_READ(userDeviceHandle,C2,PP_CAMBIST_CONFIG0);
  if (uData !=0) {
    printk("ERROR: PP_CAMBIST_CONFIG0 != 0 at start of test\n");
    return (-1);
  }

  uData = SAND_HAL_READ(userDeviceHandle,C2,PP_CAMBIST_DATA0);
  if (uData !=0) {
    printk("ERROR: PP_CAMBIST_DATA0 != 0 at start of test\n");
    return (-1);
  }

  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2,PP_CAMBIST_CONFIG0,BIST_DATA_SLICE_STATUS_SEL,uData,0);
  uData = SAND_HAL_MOD_FIELD(C2,PP_CAMBIST_CONFIG0,BIST_EN,uData,1);
  SAND_HAL_WRITE(userDeviceHandle,C2,PP_CAMBIST_CONFIG0,uData);

  /* wait for bist done */
  cambist_wait_usec = CAM_BIST_WAIT;
  bistdone = 0;
  soc_timeout_init(&to,cambist_wait_usec,0);
  while (!soc_timeout_check(&to) && !bistdone) {
    uData = SAND_HAL_READ(userDeviceHandle,C2,PP_CAMBIST_STATUS0);
    bistdone = uData & 1;
  }

  if (bistdone) {
    /* check for pass status */
    printk("PP CAM0 Bist test done in %d usec,  ",soc_timeout_elapsed(&to));
    uData = SAND_HAL_READ(userDeviceHandle,C2,PP_CAMBIST_STATUS0);
    if (uData & 2) {
      printk("PASSED\n");
    } else {
      printk("FAILED\n");
      status = -1;
    }
  } else {
    printk("PP CAM0 bist test timedout\n");
    status = -1;
  }

  /* cleanup */
  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2,PP_CAMBIST_CONFIG0,BIST_EN,uData,0);
  SAND_HAL_WRITE(userDeviceHandle,C2,PP_CAMBIST_CONFIG0,uData);

  return status;
}

int32_t sbFe2kxtDiagsPPCAM1Bist(sbxDiagsInfo_t *pDiagsInfo)
{

  uint32_t uData = 0;
  int bistdone = 0;
  int cambist_wait_usec = 0;
  int status = 0;
  soc_timeout_t to;
  sbhandle userDeviceHandle = pDiagsInfo->userDeviceHandle;

  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2,PP_CAMBIST_CONFIG1,BIST_EN,uData,0);
  SAND_HAL_WRITE(userDeviceHandle,C2,PP_CAMBIST_CONFIG1,uData);
  thin_delay(200);

  /* check to see all registers default to 0 */
  uData = SAND_HAL_READ(userDeviceHandle,C2,PP_CAMBIST_STATUS1);
  if (uData !=0) {
    printk("ERROR: PP_CAMBIST_STATUS1 != 0 at start of test\n");
    return (-1);
  }
    
  uData = SAND_HAL_READ(userDeviceHandle,C2,PP_CAMBIST_CONFIG1);
  if (uData !=0) {
    printk("ERROR: PP_CAMBIST_CONFIG1 != 0 at start of test\n");
    return (-1);
  }

  uData = SAND_HAL_READ(userDeviceHandle,C2,PP_CAMBIST_DATA1);
  if (uData !=0) {
    printk("ERROR: PP_CAMBIST_DATA1 != 0 at start of test\n");
    return (-1);
  }

  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2,PP_CAMBIST_CONFIG1,BIST_DATA_SLICE_STATUS_SEL,uData,0);
  uData = SAND_HAL_MOD_FIELD(C2,PP_CAMBIST_CONFIG1,BIST_EN,uData,1);
  SAND_HAL_WRITE(userDeviceHandle,C2,PP_CAMBIST_CONFIG1,uData);

  /* wait for bist done */
  cambist_wait_usec = CAM_BIST_WAIT;
  bistdone = 0;
  soc_timeout_init(&to,cambist_wait_usec,0);
  while (!soc_timeout_check(&to) && !bistdone) {
    uData = SAND_HAL_READ(userDeviceHandle,C2,PP_CAMBIST_STATUS1);
    bistdone = uData & 1;
  }

  if (bistdone) {
    /* check for pass status */
    printk("PP CAM1 Bist test done in %d usec,  ",soc_timeout_elapsed(&to));
    uData = SAND_HAL_READ(userDeviceHandle,C2,PP_CAMBIST_STATUS1);
    if (uData & 2) {
      printk("PASSED\n");
    } else {
      printk("FAILED\n");
      status = -1;
    }
  } else {
    printk("PP CAM1 bist test timedout\n");
    status = -1;
  }

  /* cleanup */
  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2,PP_CAMBIST_CONFIG1,BIST_EN,uData,0);
  SAND_HAL_WRITE(userDeviceHandle,C2,PP_CAMBIST_CONFIG1,uData);

  return status;
}

int32_t sbFe2kxtDiagsPPCAM2Bist(sbxDiagsInfo_t *pDiagsInfo)
{

  uint32_t uData = 0;
  int bistdone = 0;
  int cambist_wait_usec = 0;
  int status = 0;
  soc_timeout_t to;
  sbhandle userDeviceHandle = pDiagsInfo->userDeviceHandle;

  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2,PP_CAMBIST_CONFIG2,BIST_EN,uData,0);
  SAND_HAL_WRITE(userDeviceHandle,C2,PP_CAMBIST_CONFIG2,uData);
  thin_delay(200);

  /* check to see all registers default to 0 */
  uData = SAND_HAL_READ(userDeviceHandle,C2,PP_CAMBIST_STATUS2);
  if (uData !=0) {
    printk("ERROR: PP_CAMBIST_STATUS2 != 0 at start of test\n");
    return (-1);
  }
    
  uData = SAND_HAL_READ(userDeviceHandle,C2,PP_CAMBIST_CONFIG2);
  if (uData !=0) {
    printk("ERROR: PP_CAMBIST_CONFIG2 != 0 at start of test\n");
    return (-1);
  }

  uData = SAND_HAL_READ(userDeviceHandle,C2,PP_CAMBIST_DATA2);
  if (uData !=0) {
    printk("ERROR: PP_CAMBIST_DATA2 != 0 at start of test\n");
    return (-1);
  }

  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2,PP_CAMBIST_CONFIG2,BIST_DATA_SLICE_STATUS_SEL,uData,0);
  uData = SAND_HAL_MOD_FIELD(C2,PP_CAMBIST_CONFIG2,BIST_EN,uData,1);
  SAND_HAL_WRITE(userDeviceHandle,C2,PP_CAMBIST_CONFIG2,uData);

  /* wait for bist done */
  cambist_wait_usec = CAM_BIST_WAIT;
  bistdone = 0;
  soc_timeout_init(&to,cambist_wait_usec,0);
  while (!soc_timeout_check(&to) && !bistdone) {
    uData = SAND_HAL_READ(userDeviceHandle,C2,PP_CAMBIST_STATUS2);
    bistdone = uData & 1;
  }

  if (bistdone) {
    /* check for pass status */
    printk("PP CAM2 Bist test done in %d usec,  ",soc_timeout_elapsed(&to));
    uData = SAND_HAL_READ(userDeviceHandle,C2,PP_CAMBIST_STATUS2);
    if (uData & 2) {
      printk("PASSED\n");
    } else {
      printk("FAILED\n");
      status = -1;
    }
  } else {
    printk("PP CAM2 bist test timedout\n");
    status = -1;
  }

  /* cleanup */
  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2,PP_CAMBIST_CONFIG2,BIST_EN,uData,0);
  SAND_HAL_WRITE(userDeviceHandle,C2,PP_CAMBIST_CONFIG2,uData);

  return status;
}


int32_t sbFe2kxtDiagsPPCAM3Bist(sbxDiagsInfo_t *pDiagsInfo)
{

  uint32_t uData = 0;
  int bistdone = 0;
  int cambist_wait_usec = 0;
  int status = 0;
  soc_timeout_t to;
  sbhandle userDeviceHandle = pDiagsInfo->userDeviceHandle;

  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2,PP_CAMBIST_CONFIG3,BIST_EN,uData,0);
  SAND_HAL_WRITE(userDeviceHandle,C2,PP_CAMBIST_CONFIG3,uData);
  thin_delay(200);

  /* check to see all registers default to 0 */
  uData = SAND_HAL_READ(userDeviceHandle,C2,PP_CAMBIST_STATUS3);
  if (uData !=0) {
    printk("ERROR: PP_CAMBIST_STATUS3 != 0 at start of test\n");
    return (-1);
  }
    
  uData = SAND_HAL_READ(userDeviceHandle,C2,PP_CAMBIST_CONFIG3);
  if (uData !=0) {
    printk("ERROR: PP_CAMBIST_CONFIG3 != 0 at start of test\n");
    return (-1);
  }

  uData = SAND_HAL_READ(userDeviceHandle,C2,PP_CAMBIST_DATA3);
  if (uData !=0) {
    printk("ERROR: PP_CAMBIST_DATA3 != 0 at start of test\n");
    return (-1);
  }

  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2,PP_CAMBIST_CONFIG3,BIST_DATA_SLICE_STATUS_SEL,uData,0);
  uData = SAND_HAL_MOD_FIELD(C2,PP_CAMBIST_CONFIG3,BIST_EN,uData,1);
  SAND_HAL_WRITE(userDeviceHandle,C2,PP_CAMBIST_CONFIG3,uData);

  /* wait for bist done */
  cambist_wait_usec = CAM_BIST_WAIT;
  bistdone = 0;
  soc_timeout_init(&to,cambist_wait_usec,0);
  while (!soc_timeout_check(&to) && !bistdone) {
    uData = SAND_HAL_READ(userDeviceHandle,C2,PP_CAMBIST_STATUS3);
    bistdone = uData & 1;
  }

  if (bistdone) {
    /* check for pass status */
    printk("PP CAM3 Bist test done in %d usec,  ",soc_timeout_elapsed(&to));
    uData = SAND_HAL_READ(userDeviceHandle,C2,PP_CAMBIST_STATUS3);
    if (uData & 2) {
      printk("PASSED\n");
    } else {
      printk("FAILED\n");
      status = -1;
    }
  } else {
    printk("PP CAM3 bist test timedout\n");
    status = -1;
  }

  /* cleanup */
  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2,PP_CAMBIST_CONFIG3,BIST_EN,uData,0);
  SAND_HAL_WRITE(userDeviceHandle,C2,PP_CAMBIST_CONFIG3,uData);

  return status;
}

void Fe2kxt_EnableProcessing(sbxDiagsInfo_t *pDiagsInfo) {
  sbhandle hdl;
  int rv = 0;
  sbZfC2LrLrpInstructionMemoryBank0Entry_t C2LrLrpInstructionMemoryBank0Entry;
  sbZfC2LrLrpInstructionMemoryBank0Entry_InitInstance(&C2LrLrpInstructionMemoryBank0Entry);

  hdl = pDiagsInfo->userDeviceHandle;

  rv = C2LrLrpInstructionMemoryBank0Write(hdl,0,&C2LrLrpInstructionMemoryBank0Entry);
  if (rv != TRUE) {
    printk("LRP Memory Bank0 write failed \n");
  }

  /* do a bank swap */
  rv = sbFe2000SwapInstructionMemoryBank(hdl);
  if (rv != 0) {
    printk("Timeout waiting for bank swap event from LRP\n");
  }
}

void sigcatcher_fe2kxt(void)
{
  printk("\n");
  gStopSnakeTest = 1;
  return;
}

void sbFe2000DiagUpdateLrpEpoch(sbhandle hdl, uint32_t epoch) {
  UINT uTimeOut;
  UINT uUpdateEvent;

  UINT uConfig0 = SAND_HAL_READ(hdl,C2, LR_CONFIG0);
  uConfig0 = SAND_HAL_MOD_FIELD( C2, LR_CONFIG0, EPOCH, uConfig0, (epoch - 11) );
  uConfig0 = SAND_HAL_MOD_FIELD( C2, LR_CONFIG0, UPDATE, uConfig0, 1 );
  SAND_HAL_WRITE(hdl,C2, LR_CONFIG0, uConfig0);

  uUpdateEvent = 0;
  uTimeOut = 1000;
  while ( !uUpdateEvent && uTimeOut ) {
    uUpdateEvent = SAND_HAL_GET_FIELD(C2, LR_EVENT, UPDATE, SAND_HAL_READ(hdl, C2, LR_EVENT));
    --uTimeOut;
  }
  if ( !uTimeOut ) {
    printk("Timeout waiting for UPDATE event from the LRP.\n");
  }

}

int
fe2kxt_clear_lrp_banks(int unit) {

  int status;
  sbZfC2LrLrpInstructionMemoryBank0Entry_t zLrpInstructionMemoryBank0ClearEntry;
  uint32 uIndex;
  sbhandle hdl;

  hdl = SOC_SBX_CONTROL(unit)->sbhdl;

  sbZfC2LrLrpInstructionMemoryBank0Entry_InitInstance(&zLrpInstructionMemoryBank0ClearEntry);
  /* Load "nops" into instruction memory, blow out what was left behind */
  for ( uIndex = 0; uIndex < 1024; ++uIndex ) {
    C2LrLrpInstructionMemoryBank0Write( hdl, uIndex, &zLrpInstructionMemoryBank0ClearEntry);
  }
  status = sbFe2000SwapInstructionMemoryBank(hdl);
  if (status != SB_OK) {
    SOC_ERROR_PRINT((DK_ERR,"%s: swap instruction bank failed\n",FUNCTION_NAME()));
    return status;
  }

  /* Now clear out the other bank */
  for ( uIndex = 0; uIndex < 1024; ++uIndex ) {
    C2LrLrpInstructionMemoryBank0Write( hdl, uIndex, &zLrpInstructionMemoryBank0ClearEntry);
  }

  return 0;
}

#else
int appl_test_fe2kxt_diags_not_empty;
#endif /* BCM_FE2000_SUPPORT */
