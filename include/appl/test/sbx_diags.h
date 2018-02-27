 /*
 * $Id: sbx_diags.h 1.5 Broadcom SDK $
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

#ifndef _SBX_DIAGS_H_
#define _SBX_DIAGS_H_

#include <soc/defs.h>
#include <soc/sbx/sbTypesGlue.h>
#ifndef __KERNEL__
#include <string.h>
#endif
#include <appl/diag/sbx/brd_sbx.h>

#define REG_OFFSET(reg)    SAND_HAL_REG_OFFSET(CA, reg)
#define REG_OFFSET_C2(reg) SAND_HAL_REG_OFFSET(C2, reg)
#define BME32K_MIN_CLOCK_DELAY 500
#define SB_BME_MAX_TABLE_K 1
#define SB_BME_MAX_BANK_K 2
#define SB_BME_MAX_OFFSET_K 0x4000
#define ERR_LIST_CNT 29
#define FE2KXT_ERR_LIST_CNT 28
#define AG0_INT 2
#define AG1_INT 3
#define PCI_INT 

#define INTERPACKET_GAP() thin_delay(96)

#define DIAG_IF_ERROR_RETURN(op) \
  do { int __rv__; if ((__rv__ = (op)) < 0) { \
	 printf("line:%d,%s\n",__LINE__,bcm_errmsg(__rv__)); \
	 return(__rv__); } \
} while(0)

#ifdef BCM_SIRIUS_SUPPORT
typedef enum _siriusDiagsMemTests {
  SIRIUS_DDR_STANDARD_TEST = 0,
  SIRIUS_DATA_BUS_WALKING_ONES = 1,
  SIRIUS_DATA_BUS_WALKING_ZEROS = 2,
  SIRIUS_DDR_DATA_EQ_ADDR = 3,
  SIRIUS_DDR_INDIRECT_TEST = 4,
  /* leave as last */
  SIRIUS_MEM_TEST_LAST,
} siriusDiagsMemTests_t;
#endif

/* get the board id */
extern uint8_t sbx_diag_get_board_type(void);
/* get the board revision */
extern uint8_t sbx_diag_get_board_rev(void);

typedef int (*sbBme32kIndirectMemRead_pf)
            (sbregshandle bmeaddr,
	     uint32_t     ulTblId,
	     uint32_t     ulOffset,
	     uint32_t     *pulData);


typedef int (*sbBme32kIndirectMemWrite_pf)
            (sbregshandle bmeaddr,
	     uint32_t     ulTblId,
	     uint32_t     ulOffset,
             uint32_t     ulData);

typedef enum BME32K_STATUS_ET_e {
    BME32K_SUCCESS_E = 0,
    BME32K_BAD_SIZE_E,
    BME32K_MEM_ACC_TIMEOUT_E
} BME32K_STATUS_ET;


typedef enum _sbBme3200DiagsMemTests {
  SB_BME3200_DATA_PATTERN = 0,
  SB_BME3200_AAA_TEST = 1,
  /* leave as last */
  SB_BME3200_MEM_TEST_LAST
} sbBme3200DiagsMemTests_t;

typedef enum _sbQe2000DiagsBistTests {
  SB_QE2000_INTERNAL_BIST = 0,
  SB_QE2000_EXTERNAL_BIST = 1,
  /* leave as last */
  SB_QE2000_BIST_TEST_LAST
} sbQe2000DiagsBistTests_t;

/* FE2000 */
typedef enum sbFe2000DiagsStatus_e {
    SB_FE2000_SUCCESS_E = 0,
    SB_FE2000_BAD_SIZE_E,
    SB_FE2000_MEM_ACC_READ_TIMEOUT_E,
    SB_FE2000_MEM_ACC_WRITE_TIMEOUT_E,
    SB_FE2000_DATA_READ_MISMATCH_E
} sbFe2000DiagsStatus_t;

/* loopback tests */
typedef enum _sbFe2000DiagsLoopbackType {
    SB_FE2000_1G_LOOPBACK = 0,
    SB_FE2000_1G_UNIMAC_LOOPBACK=1,
    SB_FE2000_1G_PHY_LOOPBACK=2,
    SB_FE2000_10G_LOOPBACK=3, 
    SB_FE2000_10G_QE_LOOPBACK=4,
    /* leave as last */
    SB_FE2000_LOOPBACK_TEST_LAST
} sbFe2000DiagsLoopbackType_t;

typedef enum _sbFe2000DiagsMemTests {
  SB_FE2000_MM0_DATA_BUS_WALKING_ONES_NARROW_PORT0 = 0,
  SB_FE2000_MM1_DATA_BUS_WALKING_ONES_NARROW_PORT0 = 1,
  SB_FE2000_MM0_DATA_BUS_WALKING_ZEROS_NARROW_PORT0 = 2,
  SB_FE2000_MM1_DATA_BUS_WALKING_ZEROS_NARROW_PORT0 = 3,
  SB_FE2000_MM0_ADDRESS_BUS_NARROW_PORT0 = 4,
  SB_FE2000_MM1_ADDRESS_BUS_NARROW_PORT0 = 5,
  SB_FE2000_MM0_DATA_BUS_WALKING_ONES_NARROW_PORT1 = 6,
  SB_FE2000_MM1_DATA_BUS_WALKING_ONES_NARROW_PORT1 = 7,
  SB_FE2000_MM0_DATA_BUS_WALKING_ZEROS_NARROW_PORT1 = 8,
  SB_FE2000_MM1_DATA_BUS_WALKING_ZEROS_NARROW_PORT1 = 9,
  SB_FE2000_MM0_ADDRESS_BUS_NARROW_PORT1 = 10,
  SB_FE2000_MM1_ADDRESS_BUS_NARROW_PORT1 = 11,
  SB_FE2000_MM0_DATA_BUS_WALKING_ONES_WIDE_PORT = 12,
  SB_FE2000_MM1_DATA_BUS_WALKING_ONES_WIDE_PORT = 13,
  SB_FE2000_MM0_DATA_BUS_WALKING_ZEROS_WIDE_PORT = 14,
  SB_FE2000_MM1_DATA_BUS_WALKING_ZEROS_WIDE_PORT = 15,
  SB_FE2000_MM0_ADDRESS_BUS_WIDE_PORT = 16,
  SB_FE2000_MM1_ADDRESS_BUS_WIDE_PORT = 17,
  SB_FE2000_MM0_RAND_NARROW_PORT0 = 18,
  SB_FE2000_MM1_RAND_NARROW_PORT0 = 19,
  SB_FE2000_MM0_RAND_NARROW_PORT1 = 20,
  SB_FE2000_MM1_RAND_NARROW_PORT1 = 21,
  SB_FE2000_MM0_RAND_WIDE_PORT = 22,
  SB_FE2000_MM1_RAND_WIDE_PORT = 23,
  /* leave as last */
  SB_FE2000_MEM_TEST_LAST
} sbFe2000DiagsMemTests_t;

typedef enum _sbFe2000DiagsMemType {
  SB_FE2000_NARROW_PORT_0=0,
  SB_FE2000_NARROW_PORT_1,
  SB_FE2000_WIDE_PORT,
  /* leave as last */
  SB_FE2000_MEM_TYPE_LAST
} sbFe2000DiagsMemType_t;


typedef enum _sbFe2000DiagsBistTests {
  SB_FE2000_PP_CAM_ALL,
  SB_FE2000_PP_CAM_0,
  SB_FE2000_PP_CAM_1,
  SB_FE2000_PP_CAM_2,
  SB_FE2000_PP_CAM_3,
  /* leave as last */
  SB_FE2000_BIST_LAST
} sbFe2000DiagsBistTests_t;

typedef struct sbFe2000DiagsWidePortConfig_s {
  uint32_t addr_width;
  uint32_t data_width;
} sbFe2000DiagsWidePortConfig_t;

typedef struct sbFe2000DiagsNarrowPortConfig_s {
  uint32_t addr_width;
  uint32_t data_width;
} sbFe2000DiagsNarrowPortConfig_t;

/* FE2KXT */
typedef enum sbFe2kxtDiagsStatus_e {
    SB_FE2KXT_SUCCESS_E = 0,
    SB_FE2KXT_BAD_SIZE_E,
    SB_FE2KXT_MEM_ACC_READ_TIMEOUT_E,
    SB_FE2KXT_MEM_ACC_WRITE_TIMEOUT_E,
    SB_FE2KXT_DATA_READ_MISMATCH_E
} sbFe2kxtDiagsStatus_t;

/* loopback tests */
typedef enum _sbFe2kxtDiagsLoopbackType {
    SB_FE2KXT_1G_LOOPBACK = 0,
    SB_FE2KXT_1G_UNIMAC_LOOPBACK=1,
    SB_FE2KXT_1G_PHY_LOOPBACK=2,
    SB_FE2KXT_10G_LOOPBACK=3, 
    SB_FE2KXT_10G_QE_LOOPBACK=4,
    /* leave as last */
    SB_FE2KXT_LOOPBACK_TEST_LAST
} sbFe2kxtDiagsLoopbackType_t;

typedef enum _sbFe2kxtDiagsMemTests {
  SB_FE2KXT_MM0_DATA_BUS_WALKING_ONES_NARROW_PORT0 = 0,
  SB_FE2KXT_MM1_DATA_BUS_WALKING_ONES_NARROW_PORT0 = 1,
  SB_FE2KXT_MM0_DATA_BUS_WALKING_ZEROS_NARROW_PORT0 = 2,
  SB_FE2KXT_MM1_DATA_BUS_WALKING_ZEROS_NARROW_PORT0 = 3,
  SB_FE2KXT_MM0_ADDRESS_BUS_NARROW_PORT0 = 4,
  SB_FE2KXT_MM1_ADDRESS_BUS_NARROW_PORT0 = 5,
  SB_FE2KXT_MM0_DATA_BUS_WALKING_ONES_NARROW_PORT1 = 6,
  SB_FE2KXT_MM1_DATA_BUS_WALKING_ONES_NARROW_PORT1 = 7,
  SB_FE2KXT_MM0_DATA_BUS_WALKING_ZEROS_NARROW_PORT1 = 8,
  SB_FE2KXT_MM1_DATA_BUS_WALKING_ZEROS_NARROW_PORT1 = 9,
  SB_FE2KXT_MM0_ADDRESS_BUS_NARROW_PORT1 = 10,
  SB_FE2KXT_MM1_ADDRESS_BUS_NARROW_PORT1 = 11,
  SB_FE2KXT_MM0_DATA_BUS_WALKING_ONES_WIDE_PORT = 12,
  SB_FE2KXT_MM1_DATA_BUS_WALKING_ONES_WIDE_PORT = 13,
  SB_FE2KXT_MM0_DATA_BUS_WALKING_ZEROS_WIDE_PORT = 14,
  SB_FE2KXT_MM1_DATA_BUS_WALKING_ZEROS_WIDE_PORT = 15,
  SB_FE2KXT_MM0_ADDRESS_BUS_WIDE_PORT = 16,
  SB_FE2KXT_MM1_ADDRESS_BUS_WIDE_PORT = 17,
  SB_FE2KXT_MM0_RAND_NARROW_PORT0 = 18,
  SB_FE2KXT_MM1_RAND_NARROW_PORT0 = 19,
  SB_FE2KXT_MM0_RAND_NARROW_PORT1 = 20,
  SB_FE2KXT_MM1_RAND_NARROW_PORT1 = 21,
  SB_FE2KXT_MM0_RAND_WIDE_PORT = 22,
  SB_FE2KXT_MM1_RAND_WIDE_PORT = 23,
  /* leave as last */
  SB_FE2KXT_MEM_TEST_LAST
} sbFe2kxtDiagsMemTests_t;

typedef enum _sbFe2kxtDiagsMemType {
  SB_FE2KXT_NARROW_PORT_0=0,
  SB_FE2KXT_NARROW_PORT_1,
  SB_FE2KXT_WIDE_PORT,
  /* leave as last */
  SB_FE2KXT_MEM_TYPE_LAST
} sbFe2kxtDiagsMemType_t;


typedef enum _sbFe2kxtDiagsBistTests {
  SB_FE2KXT_PP_CAM_ALL,
  SB_FE2KXT_PP_CAM_0,
  SB_FE2KXT_PP_CAM_1,
  SB_FE2KXT_PP_CAM_2,
  SB_FE2KXT_PP_CAM_3,
  /* leave as last */
  SB_FE2KXT_BIST_LAST
} sbFe2kxtDiagsBistTests_t;

typedef struct sbFe2kxtDiagsWidePortConfig_s {
  uint32_t addr_width;
  uint32_t data_width;
} sbFe2kxtDiagsWidePortConfig_t;

typedef struct sbFe2kxtDiagsNarrowPortConfig_s {
  uint32_t addr_width;
  uint32_t data_width;
} sbFe2kxtDiagsNarrowPortConfig_t;

#ifdef BCM_SIRIUS_SUPPORT
typedef struct siriusDiagsParams_s {
  int ci_interface;                               /* CI Interface to run test on */
  int ddr_step_addr;                              /* step address increment */
  int ddr_start_addr;                             /* starting address */
  int ddr_burst;                                  /* specifies number of writes to burst followed by same number of reads */
  int ddr_iter;                                   /* how many iterations to run the test */
  siriusDiagsMemTests_t ddr_test_mode;            /* 0- standard,1-walk ones data,2-walk zeros data */
  int bank;                                       /* Internal DDR bank */
  int max_row;                                    /* max row within bank to test */
  int ddr3_col;                                   /* ddr3 columns configuration */
  int ddr3_row;                                   /* ddr3 rows configuration */
} siriusDiagsParams_t;
#endif

/* todo --- break up into chip specific diag info */
typedef struct _sbxDiagsInfo {
#ifdef BCM_SIRIUS_SUPPORT
  int inited;                                     /* a flag to allow TR50 to run on external memories */
#endif
  uint32_t debug_level;                           /* set the debug level */
  uint8_t bStopLoopingPacket;                     /* for snake test, to stop pkt or not */
  uint8_t bLoopOnError;                           /* for debug, allow the same error condition to repeat so board can be probed */
  sbhandle userDeviceHandle;                      /* handle to sbx chip */
  unsigned long start_addr;                       /* starting address */
  unsigned long end_addr;                         /* ending address */
  uint8_t bEndAddr;                               /* flag that is set when user specified an end_addr */
  unsigned long max_addr;                         /* temp max, will be broken up into np0,np1,wp for flexibility */
  int64_t  pattern;                               /* data pattern to use in random memory tests or bist test*/
  int32_t cam;                                    /* which pp cam to test (-1 tests them all) */
  uint8_t  walkbit;                               /* flag to indicate walk ones(==1) or zeros */
  volatile uint64_t *pDataRd;                     /* data read out of memory */
  sbFe2000DiagsMemTests_t e_mem_test;             /* Indicates the memory test to be performed */
  sbFe2000DiagsMemType_t  e_mem_type;             /* Indicates the type of memory to do the test on */
  sbFe2000DiagsLoopbackType_t e_loopback_type;    /* loopback test type */
  sbBme3200DiagsMemTests_t e_bme_mem_test;        /* Indicates type of memory test to do on BME */
  sbQe2000DiagsBistTests_t e_qe_bist_test;        /* Indicates type of bist to run */
#ifdef BCM_SIRIUS_SUPPORT
  siriusDiagsParams_t   siriusDiagParams;         /* Diag parameters for sirius chip */
#endif
  int seed;                                       /* seed - used in random testing */
  int32_t nLSFR;                                  /* for SPI PRBS test which LSFR to test */
  int32_t nInvert;                                /* for SPI PRBS to test with inverted data */
  int spi_interface;                              /* spi interface to test */
  uint8_t mem_inited[2][SB_FE2000_MEM_TYPE_LAST]; /* flag to indicate memory has been intialized */
  uint8_t mem_override;                           /* override protection scheme flag */
  uint8_t DDRTrained;                             /* flag to indicate DDR has been trained */
  int32_t  nBmeRedundancyOff;                     /* enable redundancy (for BME3200 Memory Test) */
  uint32_t uInstance;                             /* the MMU Instance */
  uint8_t bPolicerRefreshWasOn;                   /* flag to indicate if background refresh was on */
  uint8_t bInternalLpk;                           /* for loopback pkts internally (1g-Unimac,10g-BigMac)*/
  uint8_t b1GPhyLpk;                              /* loopback pkts at the 1g phy (AGM0 and AGM1)*/
  uint8_t b10GPhyLpk;                             /* loopback pkts at the 10G phy (XFP Loopback) */
  uint32_t nInjectPackets;                        /* Number of packets to inject for loopback tests */
  uint32_t nInjectPacketsLen;                     /* Length of packets to inject */
  uint8_t bSnakeLoopOn;                           /* loop the packet forever (snake test ) */
  sbFe2000DiagsWidePortConfig_t wp_config;        /* wide port configuration */
  sbFe2000DiagsNarrowPortConfig_t np0_config;     /* Narrow port0 configuration */
  sbFe2000DiagsNarrowPortConfig_t np1_config;     /* Narrow port1 configuration */
  uint32_t start_port;
  uint32_t end_port;
  uint32_t uSnakeRunTime;                         /* in seconds (approx) */
  uint32_t u10gPort;                              /* (24 Cu, 25 Fiber ) */
  uint32_t unit;                                  /* BCM unit # */
  unsigned long uMemExistsMask;                   /* linerate mem test (pop mems) */
  int dllphase;                                   /* Phase to use in DLL_DEBUG during line rate test */
  uint8_t bForcePRBSError;
  int prbs_direction;                             /* to allow prbs direction QEXX <--> Polaris (0) --> PL, (1) --> QEXX*/
  uint32_t qe_prbs_mask;                          /* enable mask for prbs testing */
  uint32_t qe_prbs_link_mask;                     /* specify links to do prbs testing */
  int init_ports;                                 /* option to re-init ports during loopback testing */
  int reinit;                                     /* at test done, reinit chip */
} sbxDiagsInfo_t;


/* Diag Info for the Qe2000 Device */
typedef struct _sbxQe2000DiagsInfo {
  sbhandle userDeviceHandle;                      /* handle to sbx chip */
  uint32_t unit;                                  /* BCM unit # */
  uint32_t debug_level;                           /* Set the debug level */
  uint32_t uPrintTime;                            /* Enabling Printing of Elapsed Seconds. */
  uint32_t uForceTestPass;                        /* Enable TR test to return immediately with Success. Used for Script Testing. */
  uint32_t uForceTestFail;                        /* Enable TR test to return immediately with Failure. Used for Script Testing. */
  uint32_t uPackets;                              /* Number of Packets to inject for test */
  uint32_t uPacketsLen;                           /* Length of Packets to inject */

  uint32_t uTxPktCnt;                             /* Number of Packets Transmitted from Processor into QE PCI  */
  uint32_t uTxByteCnt;                            /* Number of Bytes Transmitted from Processor into QE PCI */

  uint32_t uRxPktCnt;                             /* Number of Packets received by Processor from QE PCI, unit0  */
  uint32_t uRxByteCnt;                            /* Number of Bytes received by Processor from QE PCI, unit0 */

  uint32_t uRxPktCnt1;                            /* Number of Packets received by Processor from QE PCI, unit1  */
  uint32_t uRxByteCnt1;                           /* Number of Bytes received by Processor from QE PCI, unit1 */

  uint32_t ulRb0Queue;                            /* Packet coming in from RB0 will be enqueued to this Queue */
  uint32_t ulRb1Queue;                            /* Packet coming in from RB1 will be enqueued to this Queue */
  uint32_t ulPciQueue;                            /* Packet coming in from PCI will be enqueued to this Queue */
  uint32_t uRunTime;                              /* Duration for which this test will run, in seconds (approx) */
  uint32_t uTimeOut;                              /* Duration to wait, in seconds(approx), for Rx packets to trickle out. */

  uint32_t uUseFile;                              /* User has specified the packet in a file */
  uint32_t uDualKa;                               /* Test is being run on the KA Benchscreen Board with dual KA's */

  char*    pInFile;                               /* Name of the Input Binary Packet File. User specifies the packet */
                                                  /*   in this binary file. */
  char*    pOutFile;                              /* Name of the Output Binary Packet File. Packets coming out of the PCI */
                                                  /*   end up in this binary file. */
  char*    pOutFile1;                             /* Name of the Output Binary Packet File. Packets coming out of the PCI */
                                                  /*   end up in this binary file. This binary file is used when test is  */
                                                  /*   run simultaneously on two KA devices, such as on the Kamino        */
                                                  /*   Benchscreen Board. */
  char*    pPattern;                              /* User specifies this 4Byte pattern(in hex format) to increment & fill */
                                                  /*   the payload. */
  char*    pNoincr;                               /* User specifies this nByte pattern(in hex format) to fill the payload. The */
                                                  /*   payload is filled repetitively with this pattern upto nInjectPacketsLen. */
  char*    pPayload;                              /* The payload is filled with this pattern and the rest is padded with zeros */
                                                  /*   upto nInjectPacketsLen. */

} sbxQe2000DiagsInfo_t;


/* Prbs Info for the Qe2000 Device */
typedef struct _sbxQe2000PrbsInfo {
  sbhandle userDeviceHandle;                      /* handle to sbx chip */
  uint32_t unit;                                  /* BCM unit # */
  uint32_t debug_level;                           /* Set the debug level, same as nVerbose */

  uint32_t uNumIters;                             /* Number of PRBS Test Iterations */ 
  uint32_t uRunTime;                              /* Duration for which this test will run, in seconds (approx) */
  uint32_t uSleepTime; 
  uint32_t uTimeOut;                              /* Duration to wait, in seconds(approx), for Test to end. */
  uint32_t uPrintTime;                            /* Enabling Printing of Elapsed Seconds. */
  uint32_t uForceTestPass;                        /* Enable TR test to return immediately with Success. Used for Script Testing. */
  uint32_t uForceTestFail;                        /* Enable TR test to return immediately with Failure. Used for Script Testing. */
             
  uint32_t uDualKa;                               /* Test is being run on the KA Benchscreen Board with dual KA's */
  uint32_t uDo8b10b;                              /* Run the 8b/10b Test. */ 
  uint32_t uUsePrbsPoly15;                        /* Use the 15th order PRBS Polynomial */ 
  uint32_t uExitOnError;                          /* If set, Exit test on First Error */ 
  uint32_t uUseSweep;                             /* If set, use the range values instead of TupleList */ 
             
  char*    pTupleList;                            /* The sfx_si_config3 fields are specified as a comma seperated */
                                                  /* list of Tuples, i.e. {lodrv, dtx, deq} e.g. "1_8_15,0_7_14,0_3_5" */

                                                  /* sf1_si_config3: 0x08200000 */             
  uint32_t uDeqLo;                                /*     deq:  0x00000000 4Bits */
  uint32_t uDeqHi;                                
  uint32_t uDtxLo;                                /*     dtx:  0x00000008 4Bits */
  uint32_t uDtxHi;                               
  uint32_t uLoDrvLo;                              /*   lodrv:  0x00000001 1Bit  */
  uint32_t uLoDrvHi;                            
             
  uint32_t uSdLbm;                                /* Serdes Lane Bitmap */

} sbxQe2000PrbsInfo_t;


/*****************************************************************************
 * FUNCTION NAME:   sbBme32kSoftReset
 *
 * OVERVIEW:        Soft Reset the BME and run bist
 *
 * ARGUMENTS:       bmeaddr - sbregs handle to the BME3200
 *
 * RETURNS:         None.
 *
 * DESCRIPTION:     Place the BME3200 into reset, run BIST
 *
 * ASSUMPTIONS:	    BIST takes the BME out of reset
 *
 * SIDE EFFECTS:    BME is reset, BIST is run
 *
 *****************************************************************************/
void sbBme32kSoftReset(sbhandle hdl);

/*****************************************************************************
 * FUNCTION NAME:   sbBme32KBwRedundSet
 *
 * OVERVIEW:        Set the redundancy for the BW banks
 *
 * ARGUMENTS:       bmeaddr - sbregs handle to the BME3200
 *                  onoff - nonzero disables redundancy
 *
 * RETURNS:         0 - Success - Always
 *
 * DESCRIPTION:     Set the redundancy for the BW banks
 *
 * ASSUMPTIONS:	    BIST takes the BME out of reset
 *
 * SIDE EFFECTS:    None.
 *
 *****************************************************************************/
int sbBme32KBwRedundSet(sbxDiagsInfo_t *pDiagsInfo);

/*****************************************************************************
 * FUNCTION NAME:   dgBme32KMBistSet
 *
 * OVERVIEW:        Set the BIST ENABLE for BME3200
 *
 * ARGUMENTS:       bmeaddr - sbregs handle to the BME3200
 *                  onoff - nonzer enables master BIST.
 *
 * RETURNS:         0 - Success - Always
 *
 * DESCRIPTION:     Set the BIST ENABLE for BME3200
 *
 * ASSUMPTIONS:	    BIST takes the BME out of reset
 *
 * SIDE EFFECTS:    None.
 *
 *****************************************************************************/
int sbBme32KMBistSet(sbhandle bmeaddr, int onoff);



/*
 *  BME3200 Diags
 */

int32_t sbBme3200BistStart(sbxDiagsInfo_t *pDiagsInfo);
int bme32kBIST(sbxDiagsInfo_t *pDiagsInfo, uint32_t *stat0_pul,
	       uint32_t *stat1_pul, uint32_t *stat2_pul,
	       uint32_t *stat3_pul);


/* Runs all BME memory tests */
int sbBme3200DiagsSramMemTestAll(sbxDiagsInfo_t *pDiagsInfo);
/* Runs a specified bme mem test */
int sbBme3200DiagsSramMemTest(sbxDiagsInfo_t *pDiagsInfo);


/*****************************************************************************
 * FUNCTION NAME:   sbBme32SramDataPatTest
 *
 * OVERVIEW:        Perform a SRAM data pattern test on both banks of BME3200
 *                  memory
 *
 * ARGUMENTS:       bmeaddr - sbregs handle to the BME3200
 *                  ulDataPattern - data pattern to fill & compare with
 *                  ulBailOnFail - return from the test on the first failure
 *
 * RETURNS:         0 - Success
 *                  -1 - Compare failure
 *                       pulAddress - offset on the first failure
 *                       pulTblId - table id of the failure
 *                       pulBankId - bank of the failure
 *                  -2 - Write/Read Failure (timeout)
 *                       pulAddress - offset of the r/w failure
 *                       pulTblId - table id of the r/w failure
 *                       pulBankId - bank id of the r/w failure
 *                       pulData - 1 = writing at time of failure
 *                                 0 = reading at time of failure
 *
 *
 * DESCRIPTION:     For each bank, fill the memory with the data pattern,
 *                  Then read the bank out and compare with the data pattern.
 *
 * ASSUMPTIONS:	    None.
 *
 * SIDE EFFECTS:    BME is reset, BIST is run, then turned off,
 *                  memory redundancy is turned off
 *
 *****************************************************************************/
int32_t sbBmeDataPat(sbxDiagsInfo_t *pDiagsInfo);
int sbBme32SramDataPatTest(sbxDiagsInfo_t *pDiagsInfo, uint32_t *pulAddress, 
			   uint32_t *pulData, uint32_t *pulTblId, 
			   uint32_t *pulBankId);



/*****************************************************************************
 * FUNCTION NAME:   sbBme32SramAAAMemTest
 *
 * OVERVIEW:        Perform a SRAM Address At Address data pattern test on
 *                  both banks of BME3200 memory
 *
 * ARGUMENTS:       sbxDiagsInfo_t * (others see below)
 *
 * RETURNS:         0 - Success
 *                  -1 - Compare failure
 *                       pulAddress - offset on the first failure
 *                       pulTblId - table id of the failure
 *                       pulBankId - bank of the failure
 *                  -2 - Write/Read Failure (timeout)
 *                       pulAddress - offset of the r/w failure
 *                       pulTblId - table id of the r/w failure
 *                       pulBankId - bank id of the r/w failure
 *                       pulData - 1 = writing at time of failure
 *                                 0 = reading at time of failure
 *
 *
 * DESCRIPTION:     For each bank, fill each offset with its offset,
 *                  Then read the bank out and compare with the offset.
 *
 * ASSUMPTIONS:	    None.
 *
 * SIDE EFFECTS:    BME is reset, BIST is run, then turned off,
 *                  memory redundancy is turned off
 *
 *****************************************************************************/
int sbBme32SramAAAMemTest(sbxDiagsInfo_t *pDiagsInfo,
			  uint32_t *pulAddress, uint32_t *pulData, 
			  uint32_t *pulTblId, uint32_t *pulBankId);

/* starts sbBme32SramAAAMemTest, and reports pass/fail status */
int32_t sbBme32SramAAATest(sbxDiagsInfo_t *pDiagsInfo);




/*****************************************************************************
 * FUNCTION NAME:   sbBme32kBwR0/1 Read
 *
 * OVERVIEW:        Perform an indirect memory read of BW Bank 0
 *
 * ARGUMENTS:       bmeaddr - sbregs handle to the BME3200
 *                  ulTblId - the bank table id
 *                  ulOffset - offset into the bank
 *                  pulData - location to store the read data
 *
 * RETURNS:         0 - Success - read data stored in *pulData
 *                  -1 - Timeout waiting for acknowledge
 *
 * DESCRIPTION:     Perform an indirect memory write to BW Bank 0
 *
 * ASSUMPTIONS:	    BME is out of reset
 *
 * SIDE EFFECTS:    None.
 *
 *****************************************************************************/
int sbBme32kBwR0Read(sbhandle bmeaddr, uint32_t ulTblId, uint32_t ulOffset, uint32_t *pulData);
int sbBme32kBwR1Read(sbhandle bmeaddr, uint32_t ulTblId, uint32_t ulOffset, uint32_t *pulData);

/*****************************************************************************
 * FUNCTION NAME:   sbBme32kBwR0/R1 Write
 *
 * OVERVIEW:        Perform an indirect memory write to BW Bank 1
 *
 * ARGUMENTS:       bmeaddr - sbregs handle to the BME3200
 *                  ulTblId - the bank table id
 *                  ulOffset - offset into the bank
 *                  ulData - pattern to fill the memory location with
 *
 * RETURNS:         0 - Success
 *                  -1 - Timeout waiting for acknowledge
 *
 * DESCRIPTION:     Perform an indirect memory write to BW Bank 1
 *
 * ASSUMPTIONS:	    BME is out of reset
 *
 * SIDE EFFECTS:    None.
 *
 *****************************************************************************/
int sbBme32kBwR1Write(sbhandle bmeaddr, uint32_t ulTblId, uint32_t ulOffset, uint32_t ulData);
int sbBme32kBwR0Write(sbhandle bmeaddr, uint32_t ulTblId, uint32_t ulOffset, uint32_t ulData);


/*
 *  QE2K Diags
 */
void sbQe2000_BringUpPmManually(sbhandle userDeviceHandle);
uint32_t sbQe2000_PmManInitCmd(sbhandle userDeviceHandle, 
			       uint32_t nCmd, 
			       uint32_t nManInit);

uint32_t sbQe2kPmDDRTrain(sbxDiagsInfo_t *pDiagsInfo,
			  int nHalfBus);

int sbQe2kPmDllWrite(sbhandle tKaAddr,
		     uint32_t ulTableId,
		     uint32_t ulAddr,
		     uint32_t ulData);


int sbQe2kDDRIIExtBistMode0(sbxDiagsInfo_t *pDiagsInfo,
			    uint32_t *pulStartAddr,
			    uint32_t *pulEndAddr,
			    uint32_t aulDataPat[8]);
    
int sbQe2kDDRIIExtBistMode1(sbxDiagsInfo_t *pDiagsInfo,
			    uint32_t *pulStartAddr,
			    uint32_t *pulEndAddr,
			    uint32_t aulDataPat[8]);

int sbQe2kDDRIIExtBistMode2(sbxDiagsInfo_t *pDiagsInfo,
			    uint32_t *pulStartAddr,
			    uint32_t *pulEndAddr,
			    uint32_t aulDataPat[8]);

int sbQe2kDDRIIExtBistTest(sbxDiagsInfo_t *pDiagsInfo,
			   uint32_t *pulStartAddr,
			   uint32_t *pulEndAddr,
			   uint32_t aulDataPat[8]);


int sbQe2kDDRIIExtBistErrorsGet(sbhandle regaddr,
                              uint32_t ulTestResults,
                              uint32_t *pulUpperAddr,
                              uint32_t *pulLowerAddr,
				uint32_t aulFailedData[8]);

int sbQe2kPmMemRead(sbhandle tKaAddr,
		    uint32_t ulAddr, 
		    uint32_t *pulData3, 
		    uint32_t *pulData2, 
		    uint32_t *pulData1, 
		    uint32_t *pulData0);

int sbQe2kPmMemWrite(sbhandle tKaAddr,
		     uint32_t ulAddr, 
		     uint32_t ulData3, 
		     uint32_t ulData2,
		     uint32_t ulData1, 
		     uint32_t ulData0);

int sbQe2kExtBist(sbxDiagsInfo_t *pDiagsInfo);
int sbQe2kSoftReset(sbhandle qeregaddr);
int sbQe2kIntBist(sbxDiagsInfo_t *pDiagsInfo);
void sbQe2kIntBistStartAll(sbhandle tKaAddr);
int sbQe2000DiagsBistTestAll(sbxDiagsInfo_t *pDiagsInfo);
int sbQe2000DiagsBistTest(sbxDiagsInfo_t *pDiagsInfo);
int sbQe2kIntBistTest(sbxDiagsInfo_t *pDiagsInfo,
		      uint32_t *pulStatus,
		      uint32_t aulBistStat[15]);

void sbQe2kIntBistCheckAll(sbxDiagsInfo_t *pDiagsInfo, 
			   uint32_t *pulStatus, 
			   uint32_t aulBistStatus[15]);

void sbQe2000BistSrStart(sbhandle tKaAddr, int nStride);
void sbQe2000BistStStart(sbhandle tKaAddr, int nStride);
void sbQe2000BistPcStart(sbhandle tKaAddr);
void sbQe2000BistPmStart(sbhandle tKaAddr);
void sbQe2000BistEbStart(sbhandle tKaAddr);
void sbQe2000BistTxStart(sbhandle tKaAddr);
void sbQe2000BistSvStart(sbhandle tKaAddr);
void sbQe2000BistQmStart(sbhandle tKaAddr);
void sbQe2000BistEgStart(sbhandle tKaAddr);
void sbQe2000BistEiStart(sbhandle tKaAddr);
void sbQe2000BistEpStart(sbhandle tKaAddr);
void sbQe2000BistRbStart(sbhandle tKaAddr);
void sbQe2000BistQsStart(sbhandle tKaAddr);


int sbQe2000BistSrCheck(sbxDiagsInfo_t *pDiagsInfo,
			int nStride, 
			uint32_t *pulStatus0, 
			uint32_t *pulStatus1);
int sbQe2000BistPmCheck(sbxDiagsInfo_t *pDiagsInfo, 
			uint32_t *pulStatus0,
			uint32_t *pulStatus1);

int sbQe2000BistEbCheck(sbxDiagsInfo_t *pDiagsInfo, 
			uint32_t *pulStatus0, 
			uint32_t *pulStatus1);

int sbQe2000BistStCheck(sbxDiagsInfo_t *pDiagsInfo, 
			int nStride, 
			uint32_t *pulStatus0, 
			uint32_t *pulStatus1);

int sbQe2000BistPcCheck(sbxDiagsInfo_t *pDiagsInfo, 
			uint32_t *pulStatus0, 
			uint32_t *pulStatus1);

int sbQe2000BistTxCheck(sbxDiagsInfo_t *pDiagsInfo, 
			uint32_t *pulStatus0, 
			uint32_t *pulStatus1);

int sbQe2000BistSvCheck(sbxDiagsInfo_t *pDiagsInfo, 
			uint32_t *pulStatus0,
			uint32_t *pulStatus1);

int sbQe2000BistQmCheck(sbxDiagsInfo_t *pDiagsInfo,
			uint32_t *pulStatus0, 
			uint32_t *pulStatus1);

int sbQe2000BistEgCheck( sbxDiagsInfo_t *pDiagsInfo,
			 uint32_t *pulStatus0,
			 uint32_t *pulStatus1);

int sbQe2000BistEiCheck(sbxDiagsInfo_t *pDiagsInfo,
			uint32_t *pulStatus0,
			uint32_t *pulStatus1);

int sbQe2000BistEpCheck(sbxDiagsInfo_t *pDiagsInfo,
			uint32_t *pulStatus0,
			uint32_t *pulStatus1);

int sbQe2000BistRbCheck(sbxDiagsInfo_t *pDiagsInfo,
			uint32_t *pulStatus0,
			uint32_t *pulStatus1);

int sbQe2000BistQsCheck(sbxDiagsInfo_t *pDiagsInfo,
			uint32_t *pulStatus0,
			uint32_t *pulStatus1);

void sbQe2kPmDumpChannelStatus(uint8_t uResultsArr[][16]);


#endif /* _SBX_DIAGS_H_ */
