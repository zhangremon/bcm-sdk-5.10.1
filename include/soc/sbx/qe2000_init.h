/*
 * $Id: qe2000_init.h 1.21 Broadcom SDK $
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
 * QE2000 HW Init Header
 */

#ifndef _QE2000_INIT_H
#define _QE2000_INIT_H

#include <soc/sbx/sbTypesGlue.h>
#include <soc/sbx/qe2000.h>

/* status */
#define    HW_QE2000_STS_OK_K                                             (0)
#define    HW_QE2000_STS_SPI4_PORT_OUT_OF_RANGE_K                         (1)
#define    HW_QE2000_STS_INIT_PMC_AUTO_INIT_TIMEOUT_ERR_K                 (2)
#define    HW_QE2000_STS_INIT_PMC_SELFTEST_TIMEOUT_ERR_K                  (3)
#define    HW_QE2000_STS_INIT_PMC_SELFTEST_FAIL_K                         (4)
#define    HW_QE2000_STS_INIT_PMC_DDR_TRAINING_FAIL_K                     (5)
#define    HW_QE2000_STS_INIT_SERDES_POWERUP_DONE_TIMEOUT_ERR_K           (6)
#define    HW_QE2000_STS_INIT_QOS_MEM_TIMEOUT_ERR_K                       (7)
#define    HW_QE2000_STS_INIT_QOS_AUTOINIT_TIMEOUT_ERR_K                  (8)
#define    HW_QE2000_STS_INIT_PM_INTERNAL_MEMBIST_TIMEOUT_ERR_K           (9)
#define    HW_QE2000_STS_INIT_PM_MEM_TIMEOUT_ERR_K                       (10)
#define    HW_QE2000_STS_INIT_RB_INTERNAL_MEMBIST_TIMEOUT_ERR_K          (11)
#define    HW_QE2000_STS_INIT_RB_POL_INIT_TIMEOUT_ERR_K                  (12)
#define    HW_QE2000_STS_INIT_SF_MEM_TIMEOUT_ERR_K                       (13)
#define    HW_QE2000_STS_INIT_QM_MEM_TIMEOUT_ERR_K                       (14)
#define    HW_QE2000_STS_INIT_QM_AUTOINIT_TIMEOUT_ERR_K                  (15)
#define    HW_QE2000_STS_INIT_EG_MEM_TIMEOUT_ERR_K                       (16)
#define    HW_QE2000_STS_INIT_EG_FP_INIT_TIMEOUT_ERR_K                   (17)
#define    HW_QE2000_STS_INIT_EI_MEM_TIMEOUT_ERR_K                       (18)
#define    HW_QE2000_STS_INIT_PM_IO_CALIBRATION_FAILURE_K                (19)
#define    HW_QE2000_STS_INIT_EI_INTERNAL_MEMBIST_TIMEOUT_ERR_K          (20)
#define    HW_QE2000_STS_INIT_EP_INIT_TIMEOUT_ERR_K                      (21)
#define    HW_QE2000_STS_INVALID_QUEUES_PER_SHAPER_INGRESS               (22)
#define    HW_QE2000_STS_WORKAROUND21876_NOT_ENABLED                     (23)
#define    HW_QE2000_STS_PM_REFRESH_COUNT_UPDATE_ERROR                   (24)
#define    HW_QE2000_STS_INIT_QS_INTERNAL_MEMBIST_TIMEOUT_ERR_K          (25)
#define    HW_QE2000_STS_FAILURE_K                                       (26)

/* SPI Clock */
#define HW_QE2000_SFI_TXPLL_M_MAX                                        (16)
#define HW_QE2000_SFI_TXPLL_N_MAX                                        (4)
#define HW_QE2000_SFI_TXPLL_P_MAX                                        (2)


/* time */
#ifdef NEO23_KERNEL
#define HW_QE2000_10_USEC_K       (10)
#define HW_QE2000_100_USEC_K     (100)
#define HW_QE2000_1_MSEC_K      (1000)
#define HW_QE2000_10_MSEC_K    (10000)
#define HW_QE2000_100_MSEC_K  (100000)
#define HW_QE2000_500_MSEC_K  (500000)
#define HW_QE2000_1_SEC_K    (1000000)
#else
#define HW_QE2000_10_USEC_K       (10000)
#define HW_QE2000_100_USEC_K     (100000)
#define HW_QE2000_1_MSEC_K      (1000000)
#define HW_QE2000_10_MSEC_K    (10000000)
#define HW_QE2000_100_MSEC_K  (100000000)
#define HW_QE2000_500_MSEC_K  (500000000)
#define HW_QE2000_1_SEC_K    (1000000000)
#endif

#define HW_QE2000_MAX_QUEUES 16384
#define HW_QE2000_MAX_EGRESS 4096

#define HW_QE2000_QS_MEM_PRI_LUT_TABLE_SIZE (8192) /* 13b addr 8b entries */

/* default burst size when in packet interleaved mode */
#define HW_QE2000_PORT_MODE_PKT_IL_BURST_SIZE      (32)


/* st_config2, data_max_t settings
 * max interval between traing sequences */
enum
{
    HW_QE1000_TRAIN_DISABLE = 0x0, /* 0x0 : no training */
    HW_QE1000_TRAIN_2TO20,         /* 0x1 : 2^20 cycles */
    HW_QE1000_TRAIN_2TO22,         /* 0x2 : 2^22 cycles */
    HW_QE1000_TRAIN_2TO24,         /* 0x3 : 2^24 cycles */
    HW_QE1000_TRAIN_2TO26,         /* 0x4 : 2^26 cycles */
    HW_QE1000_TRAIN_2TO28,         /* 0x5 : 2^28 cycles */
    HW_QE1000_TRAIN_2TO30,         /* 0x6 : 2^30 cycles */
    HW_QE1000_TRAIN_2TO32          /* 0x7 : 2^32 cycles */
};

/*
** QE Initialization Structure
** TODO: need to update new parameters to match old style
**
*/
typedef struct HW_QE2000_INIT_ST_s {
    /* Global configuration */
    sbhandle     userDeviceHandle;
    uint32_t     reset;               /* reset qe device if non-zero */
    sbBool_t     bDmode;              /* Dmode if set Cmode if not */
    sbBool_t     bFabCrc32;           /* 32/16 bit crc packet payload protection */
    uint32_t     uClockSpeedInMHz;    /* clock speed of device 129-250 nominally 250 */
    uint32_t     uEpochSizeInNs;      /* epoch duration */
    uint32_t     uNodeId;             /* Node id number for this node */
    uint32_t     uQe1KLinkEnMask;     /*  bit mask defining which links are used by the qe1k.
				       * A bit being set means that its associated link is
				       * used for QE1K traffic.
				       */
    sbBool_t     bStandAlone;         /* stand alone switch mode */
    sbBool_t     bHybrid;             /* hybrid mode */
    uint32_t     hybridModeQueueDemarcation; /* valid in hybrid mode only */
    uint32_t     uStandAloneTsInNs;   /* timeslot duration in stand alone switch mode */
    sbBool_t     bWorkaround25276Enable; /* Enable EI memory corruption workaround */
    int32_t      nLinesPerTimeslot; /* the number of lines transmitted per timeslot */
    int32_t      nLinesPerTimeslotCongested; /* the number of lines transmitted during congestion */
    int32_t      nLinesPerTimeslotQe1k; /* the number of lines transmitted to a qe1k */

#define HW_QE2000_NUM_SPI_INTERFACES 2
#define HW_QE2000_NUM_SPI_SUBPORTS   49
    uint32_t     uSpiRefClockSpeed[HW_QE2000_NUM_SPI_INTERFACES];
    uint32_t     uSpiClockSpeed[HW_QE2000_NUM_SPI_INTERFACES];
    uint32_t     uNumPhySpiPorts[HW_QE2000_NUM_SPI_INTERFACES]; /* Number of physical ports
         							 * attached to this QE egress per SPI
								 * (regardless enabled or not)
                                                                 */
    uint32_t     uSpiSubportSpeed[HW_QE2000_NUM_SPI_INTERFACES][HW_QE2000_NUM_SPI_SUBPORTS]; /* Speed of SPI subports
											   * in Mbps
											   */											  
    uint64_t     uuRequeuePortsMask[HW_QE2000_NUM_SPI_INTERFACES]; /* ports within phys ports which are
								     mirror - only */
    uint32_t     uInterleaveBurstSize[HW_QE2000_NUM_SPI_INTERFACES]; /* burst size interleaving */
    int32_t      uPacketAdjustFormat;

    /* PM Block */
    sbBool_t     bPmHalfBus;          /* 128/64 bit external DDR2 bus */
    sbBool_t     bPmRunSelfTest;      /* user can kick off self test at this point if they want */
    sbBool_t     bPmDdrTrain;         /* perform DDR training */
    sbBool_t     bRunLongDdrMemoryTest; /* run extensive DDR memory test */

    /* QS Block */
    uint32_t     uQsMaxNodes;         /* maximum number of nodes the system will support */
    int32_t      nQueuesPerShaperIngress;
    sbBool_t     bMixHighAndLowRateFlows; /* set up pri lut table to mix high and low rate (1g and 10g traffic) */
    int32_t      spMode;

    /* RB Block */
    uint32_t     uRbSpi0PortCount;    /* number of physical ports on spi0- (set to 1 in the fe1k case) */

    /* QMGR Block */
    sbBool_t     bQm512MbDdr2;        /*  If true, external DDR2 parts are 512Mbit, else they are 256Mbit */
    uint32_t     uQmMaxArrivalRateMbs; /* maximum arrival rate in Mbps - usually 20Gbps */
    int32_t      nGlobalShapingAdjustInBytes; /* adjustment for packet lengths for accurate shaping */
    int32_t      nDemandScale;        /* demand scale */

    /* SV Block */
    sbBool_t     bSv2_5GbpsLinks;     /*  set true if the data links are running at 2.5Gbps */

    /* EG Block */
    uint32_t     uEgressMcastEfDescFifoSize[SB_FAB_DEVICE_QE2000_MAX_PORT];
    uint32_t     bEgressMcastEfDescFifoInUse[SB_FAB_DEVICE_QE2000_MAX_PORT];
    uint32_t     uEgressMcastNefDescFifoSize[SB_FAB_DEVICE_QE2000_MAX_PORT];
    uint32_t     bEgressMcastNefDescFifoInUse[SB_FAB_DEVICE_QE2000_MAX_PORT];

    uint32_t     uEgMVTSize;          /* 0:12K entries, 1:24K entries, 2:48K entries */

    uint32_t     uEgMcDropOnFull;     /* 'drop on full' for all SPI ports and PCI port */

    uint32_t     uEgMvtFormat;        /* multicast group APIs, "encap_id" parameter */
                                      /* format. This depends on microcode          */
    uint32_t     uEgMcEfTimeout;
    uint32_t     uEgMcNefTimeout;
    sbBool_t     bEgressFifoIndependentFlowControl; /* requires polaris configuration */
    uint16_t     uEgHiMcPri;           /* multicast priority */
    uint16_t     uEgLoMcPri;           /* multicast priority */

    /* EI BLock */
    uint32_t     uEiPortInactiveTimeout; /* Time period for a port to remain idle while data is available
					  * before the port is timed out in units of ~16 milliseconds
					  * (there is error based on the exact clock period-- at 250MHz
					  * core clock, the units are 8.4milliseconds)
					  */
    uint32_t     bEiSpiFullPacketMode[HW_QE2000_NUM_SPI_INTERFACES];
    uint32_t     uEiLines[HW_QE2000_NUM_SPI_INTERFACES];

    /* EP Block */
    sbBool_t     bEpDisable;          /* EP Block is disabled */

    /* SFI Block */
    uint32_t     uSfiDataLinkInitMask;    /* bits 17:0 make up a mask defining which data links are used
					   * (set to 1 to enable a link)
					   */
#define HW_QE2000_NUM_SCI_LINKS 2
#define HW_QE2000_NUM_SFI_LINKS 18
#define HW_QE2000_NUM_XCFG_REMAP_ENTRIES 256
    uint32_t     uSfiXconfig[HW_QE2000_NUM_SFI_LINKS][HW_QE2000_NUM_XCFG_REMAP_ENTRIES];    /* Xconfig mapping */
    uint32_t     uSfiHeaderPad;       /* tx/tx header padding */

    /* Tx Block */
    /* This is an adjustment indicating the number of idle clock cycles after getting an SoT pulse before
     * sending an outbound packet.  By default this should be set to 0x40.  If line lengths are longer or an
     * LCM is used, this time will need to be adjusted by any increased delay.
     */
    /* NOTE: we call this uSfiTimeslotOffsetInClocks to remain similar to qe1000 format, but the control */
    /*       has moved to the TxDMA which drives the SFI */
    uint32_t      uSfiTimeslotOffsetInClocks;

    /* SCI Block */
    uint32_t      uSciLinkEnRemap[HW_QE2000_NUM_SCI_LINKS][HW_QE2000_NUM_SFI_LINKS];
    uint32_t      uSciLinkStatusRemap[HW_QE2000_NUM_SCI_LINKS][HW_QE2000_NUM_SFI_LINKS];
    uint32_t      uSiLsWindow; /* link error window size */
    uint32_t      uSiLsThreshold; /* link error threshold */
    uint32_t      uSciDefaultBmId; /* which BME to listen to */
    uint32_t      uScTxdmaSotDelayInClocks; /* the delay between SOT sent to txdma (traffic going out si after data available) */
    uint32_t      uScGrantoffset; /* The number of clock cycles to wait after receiving a SOT, before generating the original
				     grant pulse. If an entire grant isn't received within this time, an idle grant is generated.*/

    /*
    ** SPI4 0 Parameters
    */
    uint32_t      nodeNum_ul;          /* QE node this device is connected to */
    uint32_t      Spi0RxChans_ul;      /* number SPI0 channels in receive direction (from line) */
    uint32_t      Spi0TxChans_ul;      /* number SPI0 channels in transmit direction (to line)  */
    uint32_t      Spi0MinFrameSz_ul;
    uint32_t      Spi0MaxFrameSz_ul;
    uint32_t      Spi0MaxBurst1_ul;
    uint32_t      Spi0MaxBurst2_ul;
    uint32_t      Spi0Alpha_ul;
    uint32_t      Spi0DataMaxT_ul;
    uint32_t      Spi0AlignDelay_ul;
    uint32_t      Spi0TsInvert_ul;
    uint32_t      Spi0RsInvert_ul;
    uint32_t      Spi0TxIgnStatus_ul;  /* Ignore status from tx interface - no backpressure */
    uint64_t      Spi0JumboMask_ull;
    uint32_t      Spi0RxCalM;
    uint32_t      Spi0TxCalM;

    /*
    ** SPI4 1 Parameters
    */
    uint32_t      Spi1RxChans_ul;      /* number SPI1 channels in receive direction (from line) */
    uint32_t      Spi1TxChans_ul;      /* number SPI1 channels in transmit direction (to line)  */
    uint32_t      Spi1MinFrameSz_ul;
    uint32_t      Spi1MaxFrameSz_ul;
    uint32_t      Spi1MaxBurst1_ul;
    uint32_t      Spi1MaxBurst2_ul;
    uint32_t      Spi1Alpha_ul;
    uint32_t      Spi1DataMaxT_ul;
    uint32_t      Spi1AlignDelay_ul;
    uint32_t      Spi1TsInvert_ul;
    uint32_t      Spi1RsInvert_ul;
    uint32_t      Spi1TxIgnStatus_ul;  /* Ignore status from tx interface - no backpressure */
    uint64_t      Spi1JumboMask_ull;
    uint32_t      Spi1RxCalM;
    uint32_t      Spi1TxCalM;

} HW_QE2000_INIT_ST;

uint32
hwQe2000Init(HW_QE2000_INIT_ST *hwQe2000Init_sp);

uint32  
hwQe2000InitSpi(HW_QE2000_INIT_ST *hwQe2000Init_sp);

void
hwQe2000ConfigMcSrcId(sbhandle userDeviceHandle, 
                      uint32 uNodeId,
                      uint32 uNumSpi0Ports);

#endif /* _INIT_QE2000_H */
