/*
 * $Id: fe2000_diags.h 1.2 Broadcom SDK $
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
 * == fe2000_diags.h - FE2000 Diagnostics      ==
 */

#ifndef _FE2000_DIAGS_H_
#define _FE2000_DIAGS_H_

#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/fe2k/sbFe2000Init.h>
#include "sbWrappers.h"
#include "sbx_diags.h"

#define MASK_LSW (0xffffffff)
#define SB_FE2000_MEM_TIMEOUT 200
#define CAM_MAX_TIMEOUT 1000
#define PRBS_TEST_RUN_ITERATIONS 2000

#define WIDE_PORT_MASK 0xffffffffffffffffLL
/* look at 36 bits worth */
#define NARROW_PORT_MASK 0xfffffffffLL

#define DISPLAY_TESTSTR(func,mmu,str) SB_LOG("\n%s: Starting MM%d %s test..\n",func,mmu,str)
/* from fe2000unimac.c */
#define FE2000_STS_CHECK(status)    \
    if ((status) != SB_FE2000_STS_INIT_OK_K) { return SOC_E_INTERNAL; }

void ConfigCmuHardMapping(char *cntrType,uint32_t base_address,uint32_t segment_limit);
void WrI(sbhandle hdl, uint32_t mem, uint32_t offs, uint32_t data);
uint32_t RdI(sbhandle hdl, uint32_t mem, uint32_t offs);
uint32_t MemSizeEnc(uint32_t bits);
void LrpEnable(sbhandle hdl);
uint32_t LrpDisable(sbhandle hdl);
uint32_t WaitForPhase(sbhandle hdl, uint32_t phase);
uint32_t CheckErrors(sbhandle hdl,int32_t seed,uint32_t phase);
uint32_t HashData(int32_t seed, uint32_t ulOffs, uint32_t Inv);
uint32_t crc32(uint32_t x);
void SetupCtlRegs(sbxDiagsInfo_t *pDiagsInfo);

int sbxDiagsSetErrorRegList(sbxDiagsInfo_t *pDiagsInfo,uint32_t unit);
int sbxDiagsPollForErrors(uint32_t sp, uint32_t ep, uint32_t unit);
int sbxDiagsCheckForErrors(uint32_t unit);

int sbFe2000LineRateMemTestRun(sbxDiagsInfo_t *pDiagsInfo);
int sbFe2000LineRateMemTestSetup(sbxDiagsInfo_t *pDiagsInfo);

int fe2000_set_lrp_default_regs(int unit);
int fe2000_bringup_lrp(int unit, const sbFe2000InitParams_t *pInitParams,int phase);

/* foward declarations */
void sigcatcher(void); /* handler for ctrl-c */
int sbFe2000DiagsSnakeTest(sbxDiagsInfo_t *pDiagsInfo);
int sbFe2000DiagsDumpExceptionPkt(sbxDiagsInfo_t *pDiagsInfo, uint32_t qe_unit);
int32_t sbFe2000DiagsInjectPciPacket(sbxDiagsInfo_t *pDiagsInfo);
int32_t sbFe2000DiagsReceivePciPacket(sbxDiagsInfo_t *pDiagsInfo);
uint8_t sbFe2000DiagsPvtCompensation(sbhandle userDeviceHandle, uint32_t uMmInstance);

/* dispatcher to run all the fe2k memory tests */
int sbFe2000DiagsSramMemTestAll(sbxDiagsInfo_t *pDiagsInfo);

/* override the default parity protection on memories */
int sbFe2000DiagsOverrideMmParam(sbxDiagsInfo_t *pDiagsInfo);
int sbFe2000CheckLoopbackStats(sbxDiagsInfo_t *pDiagsInfo, uint8_t bExternal);

/* clear the queues */
void sbFe2000ClearQueues(sbhandle userDeviceHandle);
int sbFe2000LoopbackSet(uint32_t unit,uint32_t loopback);
/* start a loopback test */
int sbFe2000LoopbackTest(sbxDiagsInfo_t *pDiagsInfo);

/*
 * Traffic is loopback internally through all XMG<n>. Packets come
 * into MAC from SWS get turned around back into SWS. Traffic source is PCI.
 * @param userDeviceHandle Glue handle to the FE2000
 * @param uInstance        MMU Instance
 * @param nPacketsInjected nPacketsToInject from PCI
 *
 * @return                 Status, SB_FE2000_SUCCESS_E or failure code.
 */


/* 10G loopback test */
int sbFe2000Diags10GLoopback(sbxDiagsInfo_t *pDiagsInfo);
int sbFe2000Diags10GQELoopback(sbxDiagsInfo_t *pDiagsInfo);
int sbFe2000Poll10GCounters(uint32_t unit, uint32_t port_10g);
int sbFe200010GLoopbackSet(uint32_t unit, uint32_t port_10g, uint32_t loopback);

/*
 * Traffic is loopback internally through all AGM0/1 Mac ports. Packets come
 * into MAC from SWS get turned around back into SWS. Traffic source is PCI.
 * @param pDiagsInfo Glue handle to the FE2000 + params
 *
 * @return                 Status, SB_FE2000_SUCCESS_E or failure code.
 */
int sbFe2000DiagsUnimacLoopback(sbxDiagsInfo_t *pDiagsInfo);

/*
 * Setup External loopback on all the 1G ports. Requires loopack cables.
 * @param userDeviceHandle  Glue handle to the FE2000
 * @param nPacketsInjected  Number of packets to inject via PCI
 *
 * @return                 Status, SB_FE2000_SUCCESS_E or failure code.
 *
 *  External loopback cables should be connected as follows:
 *  Connect each 1G port to adjacent port :
 *
 *   AGM0, 0--1,1--2,3--4,5--6,7--8,9--10,10--11
 *   AGM1, 0--1,1--2,3--4,5--6,7--8,9--10,10--11
 *
 *  The intent is that the packet traverses like the following thru SWS
 *  PCI-->[AGM0,p0]->[AGM0,p1]->[AGM0,p2]->...[AGM0.p11]-->
 *     -->[AGM1,p0]->[AGM1,p1]->[AGM1,p2]->...[AGM1.p11]-->PCI
 */

int sbFe2000DiagsSerdesExternalLoopback(sbxDiagsInfo_t *pDiagsInfo);

/*
 * Same as external loopback, but does not require external cables. 
 * Loops back traffic at the PHY.
 * @param userDeviceHandle Glue handle to the FE2000
 *
 * @return                 Status, SB_FE2000_SUCCESS_E or failure code.
 */
int sbFe2000DiagsSerdesInternalLoopback(sbxDiagsInfo_t *pDiagsInfo);

/*
 * Sets up Packet Replication within FE2k
 *
 * @param sbFe2000DiagsInfo Pointer to Fe2k Diags info
 * @param nPacketRepCount   How many times to replicate the packet, default is 0xfff (4095)
 * @param uIngressClient    Ingress Client (source of the orginal packet, default is PCI )
 * @param uEgressInterface  Destination interface for the replicated packets. 
 * @param uEgressPort       Destination port within the uEgressInterface to send pkts out.
 *
 * uEgressInterface, uEgressPorts:
 * 0 - SPI0, [0-63]
 * 1 - SPI1, [0-63]
 * 2 - AGM0, [0-11]
 * 3 - AGM1, [0-11]
 * 4 - XGM0, [0]
 * 5 - XGM1, [0]
 * 6 - PCI,  [0]
 * 
 * 
 * @return                 Status, SB_FE2000_SUCCESS_E or failure code.
 */

/* creates queues to snake packets from PCI->AGM0(p0-->p11)-->AGM1(p0--p11)-->PCI */
void sbFe2000Create1GLoopbackQueues(sbxDiagsInfo_t *pDiagsInfo);

int sbFe2000DiagsPacketReplication(sbxDiagsInfo_t *pDiagsInfo,
				   int32_t nPacketRepCount,
				   uint32_t uIngressClient,
				   uint32_t uEgressInterface,
				   uint32_t uEgressPort);

/*
 * Main entry for all the SRAM memory tests
 *
 * @param sbxDiagsInfo_t Pointer to sbFe2000DiagsInfo
 *
 * @return                 Status, SB_FE2000_SUCCESS_E or failure code.
 */

int sbFe2000DiagsSramMemTest(sbxDiagsInfo_t *pDiagsInfo);

/*
 * Data bus tests
 * Test will ensure each of the data pins can be set to 0 and 1 without affecting any of the others.
 * @param sbFe2000DiagsInfo Glue handle to the FE2000 + diag parameters
 * @return                 Status, SB_FE2000_SUCCESS_E or failure code.
 */

int sbFe2000DiagsSramDataBusNarrowPortTest(sbxDiagsInfo_t *pDiagsInfo);


int sbFe2000DiagsSramDataBusWidePortTest(sbxDiagsInfo_t *pDiagsInfo);

/*
 * Address bus tests
 * Test will ensure each of the address pins can be set to 0 and 1 without affecting any of the others.
 * @param sbFe2000DiagsInfo Glue handle to the FE2000 + diag parameters
 *
 * @return                 Status, SB_FE2000_SUCCESS_E or failure code.
 *
 * Code in part is based on Michael Barr, Copyright(c) 1998, Programming for embedded systems.
 * 
 *  After memory is intialized a default pattern (g_pattern) is written. The diagrams below assume the
 *  starting address for the test is 0x0, and there is 32M per port.
 * 
 *  After memInit           Address             Check for Address bits stuck high
 *  ----------------------                         ----------------------                 
 *  *    g_pattern       *    0000000h             *    ~g_pattern      *    0000000h   1) Write ~g_pattern to first offset
 *  *---------------------                         *---------------------
 *  *                    *                         *                    *                        
 *  *                    *                         *                    *    0000001h   2) Check each power-of-2 addr =? g_pattern
 *  *                    *                         *                    *
 *  *                    *                         *                    *    0000002h
 *  *                    *                         *                    *    
 *  *                    *                         *                    *    0000004h
 *  *                    *                         *                    *    
 *  *                    *                         *                    *    0000008h  .. so on
 *  *                    *                         *                    *
 *  *                    *                         *                    *
 *  *                    *                         *                    *
 *  *                    *                         *                    *
 *  *                    *                         *                    *
 *  *                    *                         *                    *
 *  *                    *                         *                    *
 *  /                    /                         /                    /
 *  *---------------------                         *---------------------
 *  *   g_pattern        *   2000000h (32M)        *   g_pattern        *   2000000h (32M)
 *  *---------------------                         *---------------------
 *
 *
 *  check for address bits stuck low or shorted
 *  
 *  Step 1) Write ~g_pattern to starting address.
 *  Step 2) Walk all other power-of-2 address still contain g_pattern (walking ones on address bus)
 *  Step 3) Restore starting address with g_pattern
 *  Step 4) Move to next power-of-2 address and repeat step 1).
 *
 *  In each case the ulWalkAddr always traverses from top to bottom.
 *
 *  Step 1)                    Address              Step 2)
 *  ----------------------    <--ulWalkAddr        ----------------------                 
 *  *                         <--ulAddrMover       *                    *
 *  *    ~g_pattern      *    0000000h             *    ~g_pattern      *    0000000h   <-- ulAddrMover, <-- ulWalkAddr, skip
 *  *---------------------                         *---------------------
 *  *                    *                         *                    *                        
 *  *                    *                         *                    *    0000001h   <-- *ulWalkAddr =? g_pattern
 *  *                    *                         *                    *
 *  *                    *                         *                    *    0000002h   <-- *ulWalkAddr =? g_pattern
 *  *                    *                         *                    *    
 *  *                    *                         *                    *    0000004h   <-- *ulWalkAddr =? g_pattern
 *  *                    *                         *                    *    
 *  *                    *                         *                    *    0000008h  .. so on
 *  *                    *                         *                    *
 *  *                    *                         *                    *
 *  *                    *                         *                    *
 *  *                    *                         *                    *
 *  *                    *                         *                    *
 *  *                    *                         *                    *
 *  *                    *                         *                    *
 *  /                    /                         /                    /
 *  *---------------------                         *---------------------
 *  *   g_pattern        *   2000000h (32M)        *   g_pattern        *   2000000h (32M)
 *  *---------------------                         *---------------------
 *
 *  Step 3)                    Address              Step 4)
 *  ----------------------    <--ulWalkAddr        ----------------------                 
 *  *                         <--ulAddrMover       *                    *
 *  *    g_pattern      *     0000000h             *    g_pattern       *    0000000h   <-- *ulWalkAddr =? g_pattern
 *  *---------------------                         *---------------------
 *  *                    *                         *    ~g_pattern      *    0000001h   <-- ulAddrMover, <-- ulWalkAddr, skip
 *  *                    *                         *---------------------                        
 *  *                    *                         *                    *    
 *  *                    *                         *                    *
 *  *                    *                         *                    *    0000002h   <-- *ulWalkAddr =? g_pattern
 *  *                    *                         *                    *    
 *  *                    *                         *                    *    0000004h   <-- *ulWalkAddr =? g_pattern
 *  *                    *                         *                    *    
 *  *                    *                         *                    *    0000008h  .. so on
 *  *                    *                         *                    *
 *  *                    *                         *                    *
 *  *                    *                         *                    *
 *  *                    *                         *                    *
 *  *                    *                         *                    *
 *  *                    *                         *                    *
 *  *                    *                         *                    *
 *  /                    /                         /                    /
 *  *---------------------                         *---------------------
 *  *   g_pattern        *   2000000h (32M)        *   g_pattern        *   2000000h (32M)
 *  *---------------------                         *---------------------
 */

int sbFe2000DiagsSramAddressBusTest(sbxDiagsInfo_t *pDiagsInfo);


/*
 * Random memory write/read/verify test
 * Works in three phases as follows:
 * PHASE0: Init memory with random data generated from seeded value
 * PHASE1: Reads back memory to verify data written is correct, then read
 *         back data is then inverted and written back.
 * PHASE2: Reads back entire memory range again to be sure data read
 *         back is the inverted data written in PHASE1.
 *
 * @param sbFe2000DiagsInfo Glue handle to the FE2000 + diag parameters
 * @return                 Status, SB_FE2000_SUCCESS_E or failure code.
 */

int sbFe2000DiagsRandomDataMemTest(sbxDiagsInfo_t *pDiagsInfo);

/*
 * Random memory write/read/verify test for narrow port 0
 * 
 * @param sbFe2000DiagsInfo Glue handle to the FE2000 + diag parameters
 * @return                 Status, SB_FE2000_SUCCESS_E or failure code.
 */

int sbFe2000DiagsRandomDataNarrowPort0Test(sbxDiagsInfo_t *pDiagsInfo);
int sbFe2000DiagsRandomDataNarrowPort1Test(sbxDiagsInfo_t *pDiagsInfo);
int sbFe2000DiagsRandomDataWidePortTest(sbxDiagsInfo_t *pDiagsInfo);


/*
 * Prbs Test for SPI
 * @param sbFe2000DiagsInfo Glue handle to the FE2000 + Diag parameters
 * @param p->nLSFR             Flag to indicate which LSFR function to select (0,1)
 * @param p->bInvert           Flag to indicate if inverted generated data is to be used.
 *
 * @return                 status ok, or error
 */
int sbFe2000DiagsSPIPrbsTest(sbxDiagsInfo_t *pDiagsInfo);

/*
 * Main Entry of all FE2000 BIST tests
 * @param sbxDiagsInfo_t     Glue handle to the chip + diag parameters
 *
 * @return                  status
 */
int32_t  sbFe2000DiagsBistStart(sbxDiagsInfo_t *pDiagsInfo);

/*
 * Main Entry for BME32000 BIST tests
 * @param sbxDiagsInfo_t     Glue handle to chip + diag parameters
 *
 * @return                  status
 */

/*
 * Bist tests all 4 PP CAMS at once
 * @param userDeviceHandle Glue handle to the FE2000 + diag parameters
 *
 * @return    status
 */

int32_t sbFe2000DiagsBistPpCamStartAll(sbxDiagsInfo_t *pDiagsInfo);

/*
 * Specific CAM test
 * @param userDeviceHandle Glue handle to the FE2000
 * @param p->cam       Indicates what cam to test
 * @param p->pattern   pattern to write during bist testing
 *
 * @return                 status
 */

int32_t sbFe2000DiagsBistPpCamStart(sbxDiagsInfo_t *pDiagsInfo);


/*
 * Helper routine to check address range is valid for memtype to be tested.
 * @param userDeviceHandle Glue handle to the FE2000+params
 *
 *
 * @return                 Status: (-1) for bad address
 *                                 (0) valid address
 */ 
int sbFe2000DiagsAddressCheck(sbxDiagsInfo_t *pDiagsInfo);


/*
 * Helper routine to intialize memory. 
 * @param userDeviceHandle Glue handle to the FE2000
 *
 * By default all the FE2k SRAMs are written.
 *
 * @return                 Status: SB_FE2000_SUCCESS_E for successful init
 *                                 SB_FE2000_MEM_ACC_WRITE_TIMEOUT_E for timeout
 */ 

int sbFe2000DiagsInitMem(sbxDiagsInfo_t *pDiagsInfo);
			

/*
 * Helper routine to read FE2K SRAM memory
 * @param userDeviceHandle Glue handle to the FE2000
 * @param uInstance        MMU Instance
 * @param addr             Address to read from
 * @param pData            Pointer used to return data to caller.
 * @param e_mem_type       Type of memory to read from
 *
 * @return                 Status: SB_FE2000_SUCCESS_E for successful read
 *                                 SB_FE2000_MEM_ACC_READ_TIMEOUT_E for timeout
 */ 

int sbFe2000DiagsMemRead(sbhandle userDeviceHandle, 
			 uint32_t uInstance,
			 unsigned long addr, 
			 volatile uint64_t *pData, 
			 sbFe2000DiagsMemType_t e_mem_type);

/*
 * Helper routine to write FE2k SRAM memory
 * @param userDeviceHandle Glue handle to the FE2000
 * @param uInstance        MMU Instance
 * @param addr             Address to write to
 * @param data             data to be written
 * @param e_mem_type       Type of memory to write to.
 *
 * @return                 Status: SB_FE2000_SUCCESS_E for successful write
 *                                 SB_FE2000_MEM_ACC_WRITE_TIMEOUT_E for timeout
 */ 

int sbFe2000DiagsMemWrite(sbhandle userDeviceHandle, 
			  uint32_t uInstance,
			  unsigned long addr, 
			  const uint64_t data, 
			  sbFe2000DiagsMemType_t e_mem_type);


/*
 * Helper routine to check that memory rd/wr op finished 
 * @param userDeviceHandle Glue handle to the FE2000
 * @param uCtlReg          Offset for ctl reg to read
 * @param uAck             The ACK bit
 * @param uTimeout         How long to wait, currently SB_FE2000_MEM_TIMEOUT
 *
 * @return                 Status, TRUE finished in time, FALSE -  timed out.
 */
int sbFe2000DiagsWrRdComplete(sbhandle userDeviceHandle, 
			      uint32_t uCtlReg, 
			      uint32_t uAck,
			      uint32_t uTimeOut);



#endif /* _FE2000_DIAGS_H_ */
