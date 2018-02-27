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
 * $Id: sbFe2000Init.c 1.65.6.1 Broadcom SDK $
 * ******************************************************************************/

#include "glue.h"
#include "soc/drv.h"
#include "soc/sbx/sbx_drv.h"
#include "hal_user.h"
#include "hal_ca_auto.h"
#include "hal_c2_auto.h"
#include "hal_ca_c2.h"
#include "sbWrappers.h"
#include "sbFe2000Init.h"
#include "sbFe2000Util.h"
#include "sbFe2000InitUtils.h"
#ifndef __KERNEL__
#include "math.h"
#endif

/* FE2K_SPI_PORTS_TO_MAX_CONTIGOUS -
 * reconfigure the spi calendar to be a sequential list from 0 to max
 * configured subport.  This re-ordering is necessary for user defined
 * ucode ports where ports may not be contigous because the QE is highly
 * dependant on the spi subport numbering scheme for both calendar
 * and fabric port numbering.
 */
#ifndef FE2K_SPI_PORTS_TO_MAX_CONTIGOUS
#define FE2K_SPI_PORTS_TO_MAX_CONTIGOUS    1
#endif


/* forward declarations */
static uint32_t CalculateLrLoaderEpoch(sbFe2000InitParams_t
                                       *pInitParams);
static uint32_t sbFe2000InitRegDone(sbhandle userDeviceHandle, uint32_t uCtlReg,uint32_t uDone, uint32_t uTimeOut);
static uint32_t sbFe2000InitQm(sbhandle userDeviceHandle, sbFe2000InitParams_t *pInitParams);
static void sbFe2000InitPb(sbhandle userDeviceHandle, sbFe2000InitParams_t *pInitParams);
static void sbFe2000InitPd(sbhandle userDeviceHandle, sbFe2000InitParams_t *pInitParams);
static void sbFe2000InitCm(sbhandle userDeviceHandle, sbFe2000InitParams_t *pInitParams);
static void sbFe2000InitRc(uint32_t uInstance,sbhandle userDeviceHandle, sbFe2000InitParams_t *pInitParams);
static void sbFe2000TrainDdr(sbhandle userDeviceHandle, sbFe2000InitParams_t *pInitParams);
static void sbFe2000EnableSramDll(sbhandle userDeviceHandle, sbFe2000InitParams_t *pInitParams);
static void sbFe2000InitMm(uint32_t uInstance,sbhandle userDeviceHandle, sbFe2000InitParams_t *pInitParams);
static void sbFe2000InitPm(sbhandle userDeviceHandle, sbFe2000InitParams_t *pInitParams);
static uint32_t sbFe2000InitLr(sbhandle userDeviceHandle, sbFe2000InitParams_t *pInitParams);
static uint32_t sbFe2000InitPt(sbhandle userDeviceHandle, sbFe2000InitParams_t *pInitParams);
static uint32_t sbFe2000InitPr(sbhandle userDeviceHandle, sbFe2000InitParams_t *pInitParams);
static uint32_t sbFe2000InitPp(sbhandle userDeviceHandle, sbFe2000InitParams_t *pInitParams);
static uint32_t sbFe2000InitSpi(sbhandle userDeviceHandle, uint16_t nNum, sbFe2000InitParams_t *pInitParams);
static uint32_t sbFe2000InitSpiTx(sbhandle userDeviceHandle, uint16_t nSpi, sbFe2000InitParams_t *pInitParams);
static uint32_t sbFe2000InitSpiRx(sbhandle userDeviceHandle, uint16_t nSpi, sbFe2000InitParams_t *pInitParams);
static uint32_t sbFe2000InitXgm(sbhandle userDeviceHandle, uint16_t nMac, sbFe2000InitParams_t *pInitParams);
static uint32_t sbFe2000InitAgmPortRx(sbhandle userDeviceHandle, uint16_t nAgmNum, uint16_t nPort, sbFe2000InitParams_t *pInitParams);
static uint32_t sbFe2000InitAgmPortTx(sbhandle userDeviceHandle, uint16_t nAgmNum, uint16_t nPort, sbFe2000InitParams_t *pInitParams);
static int agmMacConfigCalcAddr(sbhandle userDeviceHandle, int nAgmNum, int nPort);
static uint32_t sbFe2000InitAgm(sbhandle userDeviceHandle, uint16_t nAgmNum, sbFe2000InitParams_t *pInitParams);
static uint32_t sbFe2000InitAgms(sbhandle userDeviceHandle, sbFe2000InitParams_t *pInitParams);
static void sbFe2000InitCleanUp(sbhandle userDeviceHandle, sbFe2000InitParams_t *pInitParams);
static void sbFe2000InitCleanUpPt(sbhandle userDeviceHandle);
static void sbFe2000InitCleanUpQm(sbhandle userDeviceHandle);
static void sbFe2000InitCleanUpPb(sbhandle userDeviceHandle);
static void sbFe2000InitCleanUpPr(sbhandle userDeviceHandle);
static void sbFe2000InitCleanUpPp(sbhandle userDeviceHandle);
static void sbFe2000InitCleanUpPd(sbhandle userDeviceHandle);
static void sbFe2000InitCleanUpMm(sbhandle userDeviceHandle, uint16_t nNum);
static void sbFe2000InitCleanUpCm(sbhandle userDeviceHandle);
static void sbFe2000InitCleanUpRc(sbhandle userDeviceHandle, uint16_t nNum);
static void sbFe2000InitCleanUpPm(sbhandle userDeviceHandle);
static void sbFe2000InitCleanUpLr(sbhandle userDeviceHandle, sbFe2000InitParams_t *pInitParams);
static void sbFe2000InitCleanUpAgm(sbhandle userDeviceHandle, uint16_t nNum);
static void sbFe2000InitCleanUpXgm(sbhandle userDeviceHandle, uint16_t nNum);
static void sbFe2000InitCleanUpSpi4(sbhandle userDeviceHandle, uint16_t nNum);
static void sbFe2000InitCleanUpAgmPortRx(uint16_t nNum, uint16_t nPort, sbhandle userDeviceHandle);
static void sbFe2000InitCleanUpAgmPortTx(uint16_t nNum, uint16_t nPort, sbhandle userDeviceHandle);
static void sbFe2000InitPLL(sbhandle userDeviceHandle,sbFe2000InitParams_t *pInitParams);
static uint32_t sbFe2000InitFuse(sbhandle userDeviceHandle,sbFe2000InitParams_t *pInitParams);

/* kpl 03-14-07 some assumptions are made in simulation like assuming DLL lock, 1g links come up.. */
uint8_t g_bSimRun = FALSE;

void
sbFe2000InitDefaultParams(sbFe2000InitParams_t *pInitParams)
{
    int nSpiRx;
    uint16_t uSpiPort;
    int nSpiTx;
    int nXgmii;
    int nAgm;
    int nPort;
    uint32_t uIndex;
    uint32_t uPb;
    uint32_t uPriorityGeneration;
    uint32_t uBatchGroupGeneration;
    sbFe2000InitParamsPdHeaderConfig_t *pHeaderConfig;
    uint32_t uMmInstance;
    sbFe2000InitParamsMm_t *pInstance;
    int  nSegment;
    uint32_t uBaseAddr;
    int nProfile;
    int nGroup;
    uint32_t uStreamIndex;
    uint32_t uRce;
    int i, j;
    sbFe2000Connection_t *con;

    sal_memset(pInitParams, 0 ,sizeof(*pInitParams));

    pInitParams->bBringUp = TRUE;
    pInitParams->pci.bBringUp = TRUE;
    pInitParams->pci.bDoSoftReset = TRUE;

    for ( nSpiRx=0; nSpiRx<SB_FE2000_NUM_SPI_INTERFACES; nSpiRx++ ) {
        pInitParams->sr[nSpiRx].bBringUp = TRUE;
        pInitParams->sr[nSpiRx].nAlignDelay = 80;
        pInitParams->sr[nSpiRx].nCalendarLength = 1;
        pInitParams->sr[nSpiRx].nCalendarM = 1;
        pInitParams->sr[nSpiRx].nRSclkEdge = 0;

        /* allow for 256B of data to accumulate within
         * OVFL Fifo prior to backpressure being asserted.
         * The thresholds should be set so that the FIFO still has abundant free space
         * when the thresholds are crossed. The OVFL Fifo should never fill up.  In the
         * case where there is congestion, the Fifo should backpressure early to allow
         * enough time for the whole calendar to be processed and each port shut down.

         * nMaxBurst1/2 describes empty space as opposed to full space
         * OVFL Fifo is 3K deep and 16B wide -> 192 16B slots
         * nMaxBurst units is in 16B slots left remaining  therefore
         * 256B / 16 = 16 so value assigned should be 192-16 or 176 (2816B)
         */
        pInitParams->sr[nSpiRx].nMaxBurst1 = ( 3072 / 16 ) - 16;
        pInitParams->sr[nSpiRx].nMaxBurst2 = ( 3072 / 16 ) - 16;

        pInitParams->sr[nSpiRx].bLoopback = FALSE;
        for( uSpiPort = 0; uSpiPort < SB_FE2000_MAX_PORTS_PER_SPI; uSpiPort++ ) {
            pInitParams->sr[nSpiRx].nMinFrameSize[uSpiPort] = 40 + 12;
            pInitParams->sr[nSpiRx].nMaxFrameSize[uSpiPort] = 16000;
        }
    }

    for ( nSpiTx = 0; nSpiTx < SB_FE2000_NUM_SPI_INTERFACES; nSpiTx++ ) {
        pInitParams->st[nSpiTx].bBringUp = TRUE;
        pInitParams->st[nSpiTx].nCalendarLength = 1;
        pInitParams->st[nSpiTx].nCalendarM = 1;
        pInitParams->st[nSpiTx].nMaxBurst1 = 0x55;
        pInitParams->st[nSpiTx].nMaxBurst2 = 0x15;
        pInitParams->st[nSpiTx].nTSclkEdge = 1;

        pInitParams->st[nSpiTx].bLoopback = FALSE;
    }

    for ( nXgmii = 0; nXgmii < SB_FE2000_NUM_XG_INTERFACES; nXgmii++ ) {
        pInitParams->xg[nXgmii].bBringUp = FALSE;
        pInitParams->xg[nXgmii].bLocalLoopback = FALSE;
        pInitParams->xg[nXgmii].bRemoteLoopback = FALSE;
        pInitParams->xgt[nXgmii].nMaxSize = 16000;
        pInitParams->xgt[nXgmii].nHeaderMode = 0;
        pInitParams->xgt[nXgmii].nCrcMode = SB_FE2000_XG_MAC_CRC_APPEND;
        pInitParams->xgt[nXgmii].nAvgIpg = 12;
        pInitParams->xgt[nXgmii].nThrotNumer = 0;
        pInitParams->xgt[nXgmii].nThrotDenom = 0;
        pInitParams->xgt[nXgmii].bPauseEnable = 1;
        pInitParams->xgt[nXgmii].bDiscard = 0;
        pInitParams->xgt[nXgmii].bAnyStart = 0;
        pInitParams->xgt[nXgmii].nHiGig2Mode = 0;
        pInitParams->xgr[nXgmii].nMaxSize = 16000;
        pInitParams->xgr[nXgmii].nHeaderMode = 0;
        pInitParams->xgr[nXgmii].bStripCrc = TRUE;
        pInitParams->xgr[nXgmii].bIgnoreCrc = 0;
        pInitParams->xgr[nXgmii].bStrictPreamble = 1;
        pInitParams->xgr[nXgmii].bPauseEnable = 1;
        pInitParams->xgr[nXgmii].bPassCtrl = 0;
        pInitParams->xgr[nXgmii].bAnyStart = 0;
        pInitParams->xgr[nXgmii].nHiGig2Mode = 0;
    }

    for ( nAgm = 0; nAgm < SB_FE2000_NUM_AG_INTERFACES; nAgm++ ) {
        pInitParams->ag[nAgm].bBringUp = FALSE;
        pInitParams->ag[nAgm].bSwsLoopBackEnable = FALSE;
        pInitParams->ag[nAgm].uSwsLoopBackCrcMode
            = SB_FE2000_AG_MAC_CRC_APPEND;
        pInitParams->ag[nAgm].bCrcCheck = TRUE;
        pInitParams->ag[nAgm].uMaxStatCounterUnit
            = SB_FE2000_MAX_PACKET_SIZE_IN_BYTES;
        pInitParams->ag[nAgm].bTrainingByPass = FALSE;

        for ( nPort = 0; nPort < SB_FE2000_MAX_AG_PORTS; nPort++ ) {
            pInitParams->ag[nAgm].common[nPort].bBringUp = FALSE;
            pInitParams->ag[nAgm].common[nPort].bMacLocalLoopback = FALSE;
            pInitParams->ag[nAgm].common[nPort].bMacRemoteLoopback = FALSE;
            pInitParams->ag[nAgm].common[nPort].bSerGlobalLoopback = FALSE;
            pInitParams->ag[nAgm].common[nPort].bSpeed1000Mbps = TRUE;
            pInitParams->ag[nAgm].common[nPort].bSpeed100Mbps = FALSE;
            pInitParams->ag[nAgm].common[nPort].bFullDuplex = TRUE;
            pInitParams->ag[nAgm].common[nPort].b1000Xmode = TRUE;
            pInitParams->ag[nAgm].common[nPort].bEarlyCrsMode = FALSE;
            pInitParams->ag[nAgm].common[nPort].bSignalDetectMode = FALSE;
            pInitParams->ag[nAgm].common[nPort].uuMacAddr = 0;
            pInitParams->ag[nAgm].common[nPort].bXactorLoopback = FALSE ;
            pInitParams->ag[nAgm].rx[nPort].bPromiscuousEnable = TRUE;
            pInitParams->ag[nAgm].rx[nPort].bCrcForward = FALSE;
            pInitParams->ag[nAgm].rx[nPort].bPauseForward = FALSE;
            pInitParams->ag[nAgm].rx[nPort].bMacControlEnable = TRUE;
            pInitParams->ag[nAgm].rx[nPort].bPayloadLengthCheck = TRUE;
            pInitParams->ag[nAgm].rx[nPort].bRxErrFrameDiscard = FALSE;
            pInitParams->ag[nAgm].rx[nPort].uMaxTransferUnit
                = pInitParams->ag[nAgm].uMaxStatCounterUnit + 4;

            pInitParams->ag[nAgm].tx[nPort].bPadEnable = FALSE;
            pInitParams->ag[nAgm].tx[nPort].bPauseIgnore = FALSE;
            pInitParams->ag[nAgm].tx[nPort].bMacSaInsert = FALSE;
            pInitParams->ag[nAgm].tx[nPort].uCrcMode =
                SB_FE2000_AG_MAC_CRC_APPEND;
        }
    }

    pInitParams->pr.bBringUp = TRUE;
    for( uIndex = 0; uIndex < SB_FE2000_PR_NUM_PRE; uIndex++ ) {
        pInitParams->pr.bPrEnable[uIndex] = TRUE;
        pInitParams->pr.bEnablePbPrefetch[uIndex][0] = TRUE;
        pInitParams->pr.bEnablePbPrefetch[uIndex][1] = TRUE;
        pInitParams->pr.uPbPrefetchPages[uIndex][0] = 8;
        pInitParams->pr.uPbPrefetchPages[uIndex][1] = 8;
    }

    for( uIndex = 0; uIndex < SB_FE2000_MAX_PORTS_PER_SPI; uIndex++ ) {
        pInitParams->pr.nPre0_PacketBufferSelect[uIndex] = 1;
        pInitParams->pr.nPre1_PacketBufferSelect[uIndex] = 1;
    }
    for( uIndex = 0; uIndex < SB_FE2000_MAX_AG_PORTS; uIndex++ ) {
        pInitParams->pr.nPre2_PacketBufferSelect[uIndex] = 0;
        pInitParams->pr.nPre3_PacketBufferSelect[uIndex] = 0;
    }
    pInitParams->pr.nPre4_PacketBufferSelect = 0;
    pInitParams->pr.nPre5_PacketBufferSelect = 0;
    pInitParams->pr.nPre6_PacketBufferSelect = 0;
    pInitParams->pr.PipelinePriority = SB_FE2000_PR_PIPELINE_EQUAL_TO_ALL_PRES;

    pInitParams->qm.bBringUp = TRUE;
    for( uIndex = 0; uIndex < SB_FE2000_PB_NUM_PACKET_BUFFERS; uIndex++ ) {
        pInitParams->qm.uNumberOfFreePages[uIndex]
            = SB_FE2000_QM_FREEPAGES_DEFAULT;
    }
    /* take off few pages in PB0 for OAM CCM. Important dont take pages from
     * PB1 - there is a bug on it */
    pInitParams->qm.uNumberOfFreePages[0] -= SB_FE2000_QM_OAM_FREEPAGES_DEFAULT;

    pInitParams->qm.uBaseDropQueue = SB_FE2000_QM_BASE_DROP_QUEUE_DEFAULT;
    pInitParams->qm.uNumberOfDropQueues = SB_FE2000_QM_MAX_DROP_QUEUES;
    pInitParams->qm.bLockedQueueEnable = TRUE;
    pInitParams->qm.uLockedQueue = SB_FE2000_QM_LOCKED_QUEUE_DEFAULT;

    for( uIndex = 0; uIndex < SB_FE2000_PR_NUM_PRE; uIndex++ ) {
        for( uPb = 0; uPb < SB_FE2000_PB_NUM_PACKET_BUFFERS; uPb++ ) {
            pInitParams->qm.uGlobalInterfaceFlowControlThresh[uIndex][uPb]
                = 0x7FF;
            if( uIndex < SB_FE2000_NUM_SPI_INTERFACES ) {
                pInitParams->qm.uGlobalInterfaceFlowControlThresh2[uIndex][uPb]
                    = 0x7FF;
            }
        }
    }

    pInitParams->pt.bBringUp = TRUE;
    pInitParams->pt.bEnablePbPrefetch[0] = TRUE;
    pInitParams->pt.bEnablePbPrefetch[1] = TRUE;

    pInitParams->pt.bUsePtEnable = FALSE;
    for( uIndex = 0; uIndex < SB_FE2000_PT_NUM_PTE; uIndex++ ) {
        pInitParams->pt.bPtEnable[uIndex] = FALSE;
    }
    pInitParams->pt.PipelinePriority = SB_FE2000_PT_PIPELINE_EQUAL_TO_ALL_PTES;
    pInitParams->pt.uExpandedFifoSize = 0x1F;

    pInitParams->pb.bBringUpPb0 = TRUE;
    pInitParams->pb.bBringUpPb1 = TRUE;

    sal_memset(&pInitParams->pp,0x0,sizeof(sbFe2000InitParamsPp_t));

    pInitParams->pp.bBringUp = TRUE;

    pInitParams->pp.uDefaultNumberofQueuesToConfigure = 0;
    pInitParams->pp.uMaxMplsRouteLabel  = CA_PP_DEFAULT_MAX_MPLS_RTLBL;
    pInitParams->pp.uMaxMplsStatsLabel  = CA_PP_DEFAULT_MAX_MPLS_STLBL;
    pInitParams->pp.uMinMplsStatsLabel  = CA_PP_DEFAULT_MIN_MPLS_STLBL;
    pInitParams->pp.uEthernetTypeExceptionMin  = CA_PP_ETHERNET_TYPE_EXCPT_MIN;
    pInitParams->pp.uEthernetTypeExceptionMax  = CA_PP_ETHERNET_TYPE_EXCPT_MAX;

    pInitParams->pp.uDefaultVlanTypeValue     = CA_PP_DEFAULT_VLAN_TYPE;
    pInitParams->pp.uDefaultVlanTypeMaskValue = CA_PP_DEFAULT_VLAN_TYPE_MASK;
    pInitParams->pp.uDefaultLlcMaxLength      = CA_PP_DEFAULT_LLC_MAX_LEN;
    pInitParams->pp.bHashRequired             = FALSE;

    pInitParams->pp.exceptions.bEnableAllExceptions = TRUE;

    /* configure all the queues intial header type to be ethernet */
    for(uIndex=0; uIndex < SB_FE2000_PP_NUM_QUEUES; uIndex++) {
        pInitParams->pp.QueueInitialState[uIndex].uHeaderType
            = CA_PP_DEFAULT_ETH_HDRID;
        pInitParams->pp.QueueInitialState[uIndex].uShift = 0;
        pInitParams->pp.QueueInitialState[uIndex].uState = 0;
        pInitParams->pp.QueueInitialState[uIndex].uVariable = 0;
    }

    uPriorityGeneration = 0;
    uBatchGroupGeneration = 0;
    for(uIndex=0; uIndex < SB_FE2000_PP_NUM_QUEUES; uIndex++) {
        pInitParams->pp.QueueConfiguration[uIndex].uPriority
            = uPriorityGeneration%4;
        pInitParams->pp.QueueConfiguration[uIndex].uBatchGroup
            = uBatchGroupGeneration%8;
        uPriorityGeneration++;
        uBatchGroupGeneration++;
    }

    for(uIndex=0; uIndex < SB_FE2000_PP_NUM_BATCHGROUPS; uIndex++) {
        pInitParams->pp.PpByPassLrpModeConfiguration.bByPassLrpMode[uIndex]
            = FALSE;
        if( (uIndex & 0x1) == 1 ) {
            pInitParams->pp.uHeaderRecordSize[uIndex]
                = CA_PP_DEFAULT_EGRESS_RECORD_SIZE;
        } else {
            pInitParams->pp.uHeaderRecordSize[uIndex]
                = CA_PP_DEFAULT_INGRESS_RECORD_SIZE;
        }
        pInitParams->pp.bAddRouteHeader[uIndex] = FALSE;
        pInitParams->pp.bDeleteFirstHeader[uIndex] = FALSE;
        pInitParams->pp.bSourceBufferInsert[uIndex] = FALSE;
    }
    pInitParams->pp.PpByPassLrpModeConfiguration.uMirrorIndex
        = CA_PP_DEFAULT_DEBUG_PED_HDR_MIRRORINDEX;
    pInitParams->pp.PpByPassLrpModeConfiguration.uCopyCount
        = CA_PP_DEFAULT_DEBUG_PED_HDR_COPYCOUNT;
    pInitParams->pp.PpByPassLrpModeConfiguration.bHeaderCopy
        = CA_PP_DEFAULT_DEBUG_PED_HDR_HDRCOPY;
    pInitParams->pp.PpByPassLrpModeConfiguration.bDrop
        = CA_PP_DEFAULT_DEBUG_PED_HDR_DROP;

    sal_memset(&pInitParams->pd,0x0,sizeof(sbFe2000InitParamsPd_t));
    pInitParams->pd.bBringUp = TRUE;
    pInitParams->pd.bDebug = FALSE;
    pInitParams->pd.bRouteHeaderPresent = FALSE;
    pInitParams->pd.bDoIpv4CheckSumUpdate = FALSE;
    pInitParams->pd.bDoConditionalIpv4CheckSumUpdate = FALSE;
    pInitParams->pd.bDoContinueByteAdjust = 1;
    pInitParams->pd.uTruncationRemoveValue = 4;  /* mac crc */

    pInitParams->pd.uTruncationLengthValue = 12+ 64; /* erh + min size packet */

    pHeaderConfig = (pInitParams->pd.HeaderConfig);
    pHeaderConfig[CA_PD_DEFAULT_SBROUTE_HDRID].uBaseLength
        = CA_PD_DEFAULT_SBROUTE_HDRLEN;
    pHeaderConfig[CA_PD_DEFAULT_PPP_HDRID].uBaseLength
        = CA_PD_DEFAULT_PPP_HDRLEN;
    pHeaderConfig[CA_PD_DEFAULT_ETH_HDRID].uBaseLength
        = CA_PD_DEFAULT_ETH_HDRLEN;
    pHeaderConfig[CA_PD_DEFAULT_LLC_HDRID].uBaseLength
        = CA_PD_DEFAULT_LLC_HDRLEN;
    pHeaderConfig[CA_PD_DEFAULT_SNAP_HDRID].uBaseLength
        = CA_PD_DEFAULT_SNAP_HDRLEN;
    pHeaderConfig[CA_PD_DEFAULT_VLAN_HDRID].uBaseLength
        = CA_PD_DEFAULT_VLAN_HDRLEN;
    pHeaderConfig[CA_PD_DEFAULT_MPLS_HDRID].uBaseLength
        = CA_PD_DEFAULT_MPLS_HDRLEN;
    pHeaderConfig[CA_PD_DEFAULT_IPV4_HDRID].uBaseLength
        = CA_PD_DEFAULT_IPV4_HDRLEN;
    pHeaderConfig[CA_PD_DEFAULT_IPV6_HDRID].uBaseLength
        = CA_PD_DEFAULT_IPV6_HDRLEN;
    pHeaderConfig[CA_PD_DEFAULT_GRE_HDRID].uBaseLength
        = CA_PD_DEFAULT_GRE_HDRLEN;
    pHeaderConfig[CA_PD_DEFAULT_TCP_HDRID].uBaseLength
        = CA_PD_DEFAULT_TCP_HDRLEN;
    pHeaderConfig[CA_PD_DEFAULT_UDP_HDRID].uBaseLength
        = CA_PD_DEFAULT_UDP_HDRLEN;
    pHeaderConfig[CA_PD_DEFAULT_HIGIG_HDRID].uBaseLength
        = CA_PD_DEFAULT_HIGIG_HDRLEN;
    pHeaderConfig[CA_PD_DEFAULT_SBROUTE_HDRID].uLengthPosition
        = CA_PD_DEFAULT_LEN_POS;
    pHeaderConfig[CA_PD_DEFAULT_PPP_HDRID].uLengthPosition
        = CA_PD_DEFAULT_LEN_POS;
    pHeaderConfig[CA_PD_DEFAULT_ETH_HDRID].uLengthPosition
        = CA_PD_DEFAULT_LEN_POS;
    pHeaderConfig[CA_PD_DEFAULT_LLC_HDRID].uLengthPosition
        = CA_PD_DEFAULT_LEN_POS;
    pHeaderConfig[CA_PD_DEFAULT_SNAP_HDRID].uLengthPosition
        = CA_PD_DEFAULT_LEN_POS;
    pHeaderConfig[CA_PD_DEFAULT_VLAN_HDRID].uLengthPosition
        = CA_PD_DEFAULT_LEN_POS;
    pHeaderConfig[CA_PD_DEFAULT_MPLS_HDRID].uLengthPosition
        = CA_PD_DEFAULT_LEN_POS;
    pHeaderConfig[CA_PD_DEFAULT_IPV4_HDRID].uLengthPosition
        = CA_PD_DEFAULT_IPV4_LEN_POS;
    pHeaderConfig[CA_PD_DEFAULT_IPV6_HDRID].uLengthPosition
        = CA_PD_DEFAULT_LEN_POS;
    pHeaderConfig[CA_PD_DEFAULT_GRE_HDRID].uLengthPosition
        = CA_PD_DEFAULT_LEN_POS;
    pHeaderConfig[CA_PD_DEFAULT_TCP_HDRID].uLengthPosition
        = CA_PD_DEFAULT_LEN_POS;
    pHeaderConfig[CA_PD_DEFAULT_UDP_HDRID].uLengthPosition
        = CA_PD_DEFAULT_LEN_POS;
    pHeaderConfig[CA_PD_DEFAULT_HIGIG_HDRID].uLengthPosition
        = CA_PD_DEFAULT_LEN_POS;
    pHeaderConfig[CA_PD_DEFAULT_EMBEDDED_LENGTH_HDRID].uLengthPosition
        = CA_PD_DEFAULT_LEN_POS;
    pHeaderConfig[CA_PD_DEFAULT_IPV4_HDRID].uLengthSize
        = CA_PD_DEFAULT_IPV4_LEN_SIZE;
    pHeaderConfig[CA_PD_DEFAULT_IPV4_HDRID].uLengthUnits
        = CA_PD_DEFAULT_IPV4_LEN_UNITS;

    sal_memset(&pInitParams->mm[0],0x0,sizeof(sbFe2000InitParamsMm_t));
    sal_memset(&pInitParams->mm[1],0x0,sizeof(sbFe2000InitParamsMm_t));
    pInitParams->mm[0].bBringUp =TRUE;
    pInitParams->mm[1].bBringUp =TRUE;

    /**********************************************************
     * There are two instances of MM and default configuration:
     * Each MM instance has 1. 2 Internal 8kx72
     *                      2. 2 External 36b
     *                      3. 1 External 72b
     *
     * LRP requires atmost two 64b external memories.
     * LRP requires atmost 4x32b internal memories.
     * LRP requires atmost 4x32b external memories
     * PMU requires one 64b external memory
     * CMU requires two 36b external memories.
     * LUE requires two 36b external memories
     *
     * For this example configuration, needs fine tuning later.
     * LRP with 2 Internal 32b & 2 External 32b & 1 External 64b
     * PMU with 1 External 64b memory
     * CMU with 2 external 32b memories.
     * LUE with 2 external 32b memories.
     ********************************************************/

    for( uMmInstance = 0; uMmInstance < SB_FE2000_NUM_MM_INSTANCES;
         uMmInstance++ ){
        pInstance = &(pInitParams->mm[uMmInstance]);

        pInstance->uNumberofInternalRam0Segments = 1;
        pInstance->uNumberofInternalRam1Segments = 1;
        pInstance->uNumberofNarrowPort0Segments  = 1;
        pInstance->uNumberofNarrowPort1Segments  = 1;
        pInstance->uNumberofWidePortSegments     = 1;

        pInstance->ramconfiguration.InternalRamConfiguration
            = SB_FE2000_MM_RAM0_8KBY72_AND_RAM1_8KBY72;
        pInstance->ramconfiguration.NarrowPortRam0Configuration
            = SB_FE2000_MM_ONE_BY_18_RAM_CONNECTED;
        pInstance->ramconfiguration.NarrowPortRam1Configuration
            = SB_FE2000_MM_ONE_BY_18_RAM_CONNECTED;
        pInstance->ramconfiguration.WidePortRamConfiguration
            = SB_FE2000_MM_ONE_BY_36_RAM_CONNECTED;

        pInstance->ramconnections.bPmuPrimaryClientOnWideport = FALSE;
        pInstance->ramconnections.Lrp0MemoryConnection
            = SB_FE2000_MM_LRP0_CONNECTED_TO_NO_RESOURCE;
        pInstance->ramconnections.Lrp1MemoryConnection
            = SB_FE2000_MM_LRP1_CONNECTED_TO_NO_RESOURCE;
        pInstance->ramconnections.Lrp2MemoryConnection
            = SB_FE2000_MM_LRP2_CONNECTED_TO_NO_RESOURCE;
        pInstance->ramconnections.Lrp3MemoryConnection
            = SB_FE2000_MM_LRP3_CONNECTED_TO_NO_RESOURCE;
        pInstance->ramconnections.Lrp4MemoryConnection
            = SB_FE2000_MM_LRP4_CONNECTED_TO_NO_RESOURCE;
        pInstance->ramconnections.PmuMemoryConnection
            = SB_FE2000_MM_PMU_CONNECTED_TO_NO_RESOURCE;
        pInstance->ramconnections.Cmu0MemoryConnection
            = SB_FE2000_MM_CMU0_CONNECTED_TO_NO_RESOURCE;
        pInstance->ramconnections.Cmu1MemoryConnection
            = SB_FE2000_MM_CMU1_CONNECTED_TO_NO_RESOURCE;

        pInstance->ramprotection.InternalRam0ProtectionScheme
            = SB_FE2000_MM_32BITS_4BITPARITY;
        pInstance->ramprotection.InternalRam1ProtectionScheme
            = SB_FE2000_MM_32BITS_4BITPARITY;
        pInstance->ramprotection.WidePortRamProtectionScheme
            = SB_FE2000_MM_32BITS_4BITPARITY;
        pInstance->ramprotection.NarrowPort0ProcessorInterface
            = SB_FE2000_MM_32BITS_4BITPARITY;
        pInstance->ramprotection.NarrowPort0DmaAccess
            = SB_FE2000_MM_32BITS_4BITPARITY;
        pInstance->ramprotection.NarrowPort0SecondaryClient
            = SB_FE2000_MM_35BITS_1BITPARITY;
        pInstance->ramprotection.NarrowPort0PrimaryClient
            = SB_FE2000_MM_32BITS_4BITPARITY;
        pInstance->ramprotection.NarrowPort1ProcessorInterface
            = SB_FE2000_MM_32BITS_4BITPARITY;
        pInstance->ramprotection.NarrowPort1DmaAccess
            = SB_FE2000_MM_32BITS_4BITPARITY;
        pInstance->ramprotection.NarrowPort1SecondaryClient
            = SB_FE2000_MM_35BITS_1BITPARITY;
        pInstance->ramprotection.NarrowPort1PrimaryClient
            = SB_FE2000_MM_32BITS_4BITPARITY;
    }

    pInitParams->cm.bBringUp = FALSE;
    pInitParams->cm.uNumRingElements = 0;
    pInitParams->cm.uCounterRingThresh = 0;
    pInitParams->cm.uCmDmaThresh = 0x3ec;

    uBaseAddr = 0;
    for( nSegment = 0; nSegment < SB_FE2000_MAX_CM_SEGMENTS; nSegment++ ){
        pInitParams->cm.segment[nSegment].bEnable          = FALSE;
        pInitParams->cm.segment[nSegment].bEnableAutoFlush = FALSE;
        pInitParams->cm.segment[nSegment].counterType
            = SB_FE2000_CM_LEGACY_COUNTER;
        pInitParams->cm.segment[nSegment].uBank            = 0;
        pInitParams->cm.segment[nSegment].uEject = SB_FE2000_CM_PCI_EJECT;
        pInitParams->cm.segment[nSegment].uBaseAddr        = uBaseAddr;
        pInitParams->cm.segment[nSegment].uLimit           = 512;
        pInitParams->cm.segment[nSegment].uAutoFlushRate   = 256;
        uBaseAddr += 512;
    }

    pInitParams->pm.bBringUp = FALSE;
    pInitParams->pm.uMmuSelectBit = 0;
    pInitParams->pm.bBackgroundRefreshEnable = TRUE;
    pInitParams->pm.uTotalRefreshThreshold = 1024;
    for( nGroup = 0; nGroup < SB_FE2000_PM_TOTAL_NUM_GROUPS; nGroup++ ){
        pInitParams->pm.group[nGroup].bEnable = FALSE;
        pInitParams->pm.group[nGroup].uPolicerIdMinimum = 0;
        pInitParams->pm.group[nGroup].uPolicerIdMaximum = 0;

        pInitParams->pm.group[nGroup].uRefreshPeriod = 0;
        pInitParams->pm.group[nGroup].uRefreshCount = 0;
        pInitParams->pm.group[nGroup].uRefreshThreshold = 0;

        pInitParams->pm.group[nGroup].bTimerEnable = FALSE;
        pInitParams->pm.group[nGroup].uTimestampOffset = 0;
        pInitParams->pm.group[nGroup].uTimerPeriod = 0;
    }

    for(nProfile = 0; nProfile < SB_FE2000_PM_TOTAL_NUM_PROFILES; nProfile++){
        pInitParams->pm.profile[nProfile].bEnable = FALSE;
        pInitParams->pm.profile[nProfile].profileType = SB_FE2000_PM_POLICER;
        pInitParams->pm.profile[nProfile].bBlind     = FALSE;
        pInitParams->pm.profile[nProfile].bDropOnRed = FALSE;
        pInitParams->pm.profile[nProfile].bCpFlag    = TRUE;
        pInitParams->pm.profile[nProfile].bRFC2698   = FALSE;
        pInitParams->pm.profile[nProfile].bBktCNoDec = FALSE;
        pInitParams->pm.profile[nProfile].bBktENoDec = FALSE;
        pInitParams->pm.profile[nProfile].uBktCSize
            = SB_FE2000_DEFAULT_BKT_SIZE;
        pInitParams->pm.profile[nProfile].uCBS
            = SB_FE2000_MAX_PACKET_SIZE_IN_BYTES;
        pInitParams->pm.profile[nProfile].uCIRBytes  = 0;
        pInitParams->pm.profile[nProfile].uEBS
            = SB_FE2000_MAX_PACKET_SIZE_IN_BYTES;
        pInitParams->pm.profile[nProfile].uEIRBytes  = 0;
        pInitParams->pm.profile[nProfile].uLenShift  = 0;

        pInitParams->pm.profile[nProfile].bInterrupt = FALSE;
        pInitParams->pm.profile[nProfile].bReset     = FALSE;
        pInitParams->pm.profile[nProfile].bStrict    = FALSE;
        pInitParams->pm.profile[nProfile].bMode32    = FALSE;

        pInitParams->pm.profile[nProfile].uDeadline  = 0;
    }

    pInitParams->lr.bBringUp = TRUE;
    pInitParams->lr.bLrpBypass = FALSE;
    pInitParams->lr.bHeaderProcessingMode = TRUE;
    for( uStreamIndex = 0; uStreamIndex < SB_FE2000_LR_NUMBER_OF_STREAMS;
         uStreamIndex++ ) {
        pInitParams->lr.bStreamOnline[uStreamIndex] = FALSE;
    }
    pInitParams->lr.uPpeRequestPaceInCycles = 2;
    pInitParams->lr.uFramesPerContext = 48;
    pInitParams->lr.bPairedMode  = TRUE;
    pInitParams->lr.uLoadCount = 3;
    pInitParams->lr.uNumberOfContexts = 11;
    pInitParams->lr.uWords0 =  8;
    pInitParams->lr.uWords1 =  8;
    pInitParams->lr.uEpochLengthInCycles = CalculateLrLoaderEpoch(pInitParams);

    if(!pInitParams->lr.bLrpBypass) {
        for(uRce=0; uRce < SB_FE2000_NUM_RC_INSTANCES; uRce++) {
            pInitParams->rc[uRce].bBringUp = TRUE;
        }
    }

    pInitParams->queues.n = 0;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 12; j++) {
            con = &pInitParams->queues.connections[pInitParams->queues.n];
            con->from.ulInterfacePortType = SB_FE2000_IF_PTYPE_AGM0 + i;
            con->from.ulPortId = j;
            con->to.ulInterfacePortType = SB_FE2000_IF_PTYPE_SPI0 + i;
            con->to.ulPortId = j;
            con->ulUcodePort = pInitParams->queues.n;
            pInitParams->queues.n++;
        }
    }
}

static
uint32_t
CalculateLrLoaderEpoch(sbFe2000InitParams_t *pInitParams)
{
  uint32_t uLoaderEpoch = 0;

  int64_t fIngressLoadTime = 0x0LL;
  int64_t fEgressLoadTime = 0x0LL;
#ifndef __KERNEL__
  int64_t fXferTime = 0x0LL;
  int64_t fFirstTime;
  int64_t fIntermediate;
#endif

  /* rgf - Dec 08 2006 - From Nick's explanation in BUG23565
   * rgf - Dec 08 2006 - Context Load Time:
   * rgf - Dec 08 2006 -
   * rgf - Dec 08 2006 - xfer_time = ceiling(frames_per_context / (10 + 2*load_count)) *
   * rgf - Dec 08 2006 -                    (10 + 2*load_count) * ceiling(words0 / 2)
   * rgf - Dec 08 2006 - first_time = load_count + 4 ;; {init & first states}
   * rgf - Dec 08 2006 - load_time = xfer_time + first_time + 2 ;; {done & start states}
   * rgf - Dec 08 2006 -  example:
   * rgf - Dec 08 2006 - frames_per_context = 48; load_count = 1; words0 = 8
   * rgf - Dec 08 2006 - => xfer_time  = 192
   * rgf - Dec 08 2006 - => first_time = 5
   * rgf - Dec 08 2006 - => load_time = 192 + 5 + 2 = 199
   * rgf - Dec 08 2006 -
   * rgf - Dec 08 2006 - In paired mode you perform a similar calculation using words1 and add the two.
   * rgf - Dec 08 2006 -
   * rgf - Dec 08 2006 - Let's look at your example:
   * rgf - Dec 08 2006 - frames_per_context = 48; load_count = 3; words0 = 10; words1 = 6
   * rgf - Dec 08 2006 -
   * rgf - Dec 08 2006 - xfer_time0 = 48 * 5 = 240
   * rgf - Dec 08 2006 - xfer_time1 = 48 * 3 = 144
   * rgf - Dec 08 2006 - first_time = 7
   * rgf - Dec 08 2006 - load_time0 = 240 + 7 + 2 = 249
   * rgf - Dec 08 2006 - load_time1 = 144 + 7 + 2 = 153
   * rgf - Dec 08 2006 - load_time = 249 + 153 = 402
   */

#ifndef __KERNEL__
  fIntermediate = (10.0 + 2.0*(int64_t)pInitParams->lr.uLoadCount);
  fXferTime = (uint32_t)(ceil( (int64_t)(pInitParams->lr.uFramesPerContext) / fIntermediate ) *
                     fIntermediate * ceil( (int64_t)pInitParams->lr.uWords0 / 2.0 ));
  fFirstTime = (int64_t)pInitParams->lr.uLoadCount + 4.0;
  fIngressLoadTime = fXferTime + fFirstTime + 2.0;

  fXferTime = (uint32_t)(ceil( (int64_t)(pInitParams->lr.uFramesPerContext) / fIntermediate ) *
                     fIntermediate * ceil( (int64_t)pInitParams->lr.uWords1 / 2.0 ));
  /* First time remains the same as ingress */
  fEgressLoadTime = fXferTime + fFirstTime + 2.0;
#endif

  if( pInitParams->lr.bPairedMode == TRUE ) {
    uLoaderEpoch = (uint32_t)(fIngressLoadTime + fEgressLoadTime);
    /* if( (uLoaderEpoch < SB_FE2000_LR_20G_DUPLEX_EPOCH) && (gZM.GetAttribute("allow_small_epochs",0) == 0) ) { */
    if(uLoaderEpoch < SB_FE2000_LR_20G_DUPLEX_EPOCH ) {
      uLoaderEpoch = SB_FE2000_LR_20G_DUPLEX_EPOCH;
    }
  } else {
    uLoaderEpoch = (uint32_t)fIngressLoadTime;
  }

  SB_LOGV2("LoaderEpoch %d based on Paired %d Words0 %d Words1 %d FramesPerContext %d LoadCount %d",
             uLoaderEpoch, pInitParams->lr.bPairedMode, pInitParams->lr.uWords0,
             pInitParams->lr.uWords1, pInitParams->lr.uFramesPerContext, pInitParams->lr.uLoadCount);
  return uLoaderEpoch;

}

uint32_t
sbFe2000Init(sbhandle userDeviceHandle,
             sbFe2000InitParams_t *pInitParams)
{

  uint32_t status;
  uint32_t i;
  DENTER();
  SB_ASSERT(pInitParams);

  if (pInitParams->bSimulation) {
    g_bSimRun = TRUE;
  }

  /* TODO..
   * Convert the user export hwInitParams to an internal
   * one for bringup..
   */

  /* This function modifies no sw state.  It walks the device components to 
   * bring them out of reset and configure it based on the initiialization 
   * parameters, none of which should be done in a warm boot - skip it.
   */

  if (SOC_WARM_BOOT((int)userDeviceHandle)) {
      SOC_DEBUG_PRINT((DK_VERBOSE, "Skipping %s for warm boot\n", FUNCTION_NAME()));

      SOC_WARM_BOOT_DONE((int)userDeviceHandle);
      SAND_HAL_WRITE(userDeviceHandle, CA,  PM_ERROR_MASK, ~0);
      SOC_WARM_BOOT_START((int)userDeviceHandle);

      return SB_OK;
  }

  /* SB_ASSERT(pInitParamsInternal); */

  /*
   * Bringup blocks within FE
   */

  /* check ==> PCI interface should already be initialized via CFE */

  /* dclee - Mar 28 2007 - Initiate PLL bringup procedure */
  sbFe2000InitPLL(userDeviceHandle,pInitParams);

  /* FUSE block */
  status = sbFe2000InitFuse(userDeviceHandle,pInitParams);
  if (status != SB_FE2000_STS_INIT_OK_K)
    {
      return status;
    }

  /* SWS Queue Manager */
  status = sbFe2000InitQm(userDeviceHandle,pInitParams);
  if (status != SB_FE2000_STS_INIT_OK_K)
    {
      return status;
    }

  /* SPI */
  status = sbFe2000InitSpi(userDeviceHandle,0,pInitParams);
  if (status != SB_FE2000_STS_INIT_OK_K)
    {
      return status;
    }

  status = sbFe2000InitSpi(userDeviceHandle,1,pInitParams);
  if (status != SB_FE2000_STS_INIT_OK_K)
    {
      return status;
    }

  /* Ten Gigabit Ethernet MACs */
  for(i=0;i< SB_FE2000_NUM_XG_INTERFACES;i++) {
    status = sbFe2000InitXgm(userDeviceHandle,i,pInitParams);
    if (status != SB_FE2000_STS_INIT_OK_K)
      {
        return status;
      }
  }

  /* Aggregated Gigabit Ethernet MAC */
  status = sbFe2000InitAgms(userDeviceHandle,pInitParams);
  if (status != SB_FE2000_STS_INIT_OK_K)
    {
      return status;
    }

  /* SWS Packet Buffer */
  sbFe2000InitPb(userDeviceHandle,pInitParams);

  /* SWS Packet Transmitter */
  status = sbFe2000InitPt(userDeviceHandle,pInitParams);
  if (status != SB_FE2000_STS_INIT_OK_K)
    {
      return status;
    }

  /* SWS Packet Receiver */
  status = sbFe2000InitPr(userDeviceHandle,pInitParams);
  if (status != SB_FE2000_STS_INIT_OK_K)
    {
      return status;
    }

  /* Packet Parsing Engine */
  status = sbFe2000InitPp(userDeviceHandle,pInitParams);
  if (status != SB_FE2000_STS_INIT_OK_K)
    {
      return status;
    }

  /* PacketEditor */
  sbFe2000InitPd(userDeviceHandle,pInitParams);

  /* Memory Management Unit */
  sbFe2000EnableSramDll(userDeviceHandle, pInitParams);
  for(i=0;i<SB_FE2000_NUM_MM_INSTANCES;i++) {
    sbFe2000InitMm(i,userDeviceHandle,pInitParams);
  }
  sbFe2000TrainDdr(userDeviceHandle, pInitParams);

  /* Counter Management Unit */
  sbFe2000InitCm(userDeviceHandle,pInitParams);

  /* Rule Classification Engine */
  for(i=0;i<SB_FE2000_NUM_RC_INSTANCES;i++) {
    sbFe2000InitRc(i,userDeviceHandle,pInitParams);
  }

  /* Policier Management Unit */
  sbFe2000InitPm(userDeviceHandle,pInitParams);

  /* Line Rate Processor */
  status = sbFe2000InitLr(userDeviceHandle,pInitParams);
  if (status != SB_FE2000_STS_INIT_OK_K) {
      return status;
  }

  /* clean up all blocks, clear error registers, unmask error registers.. */
  sbFe2000InitCleanUp(userDeviceHandle,pInitParams);

  DEXIT();
  return SB_FE2000_STS_INIT_OK_K;
}

static uint32_t
sbFe2000InitRegDone(sbhandle userDeviceHandle,
                    uint32_t uCtlReg,
                    uint32_t uDone,
                    uint32_t uTimeOut)
{
  uint32_t uReg;
  uint32_t i;

  /* Wait for done to pop */
  for( i = 0; i < (uTimeOut * 10); i++) {
    uReg = SAND_HAL_READ_OFFS(userDeviceHandle, uCtlReg);
    if( uReg & uDone) {
      return( FALSE );
    }

    thin_delay(SB_FE2000_10_USEC_K);
  }

  return( TRUE );
}



static uint32_t
sbFe2000InitQm(sbhandle userDeviceHandle,
               sbFe2000InitParams_t *pInitParams)
{
  sbZfFe2000QmQueueConfigEntry_t zQueueConfig;
  uint32_t uDropQueue;
  sbZfFe2000QmQueueState0Entry_t zQueueState;
  /* page is 192byte, there are 192/8 = 24 words for a page*/

  DENTER();

  SB_ASSERT(!SAND_HAL_IS_FE2KXT((int)userDeviceHandle));
  /* bms - May 01 2006 - setup the number_free_pages_pb0 and number_free_pages_pb1 */
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, QM_FP_CONFIG, NUMBER_FREE_PAGES_PB0, pInitParams->qm.uNumberOfFreePages[0]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, QM_FP_CONFIG, NUMBER_FREE_PAGES_PB1, pInitParams->qm.uNumberOfFreePages[1]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, QM_CONFIG0, TOTAL_MAX_PAGES_AVAILABLE_PB0, pInitParams->qm.uNumberOfFreePages[0]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, QM_CONFIG0, TOTAL_MAX_PAGES_AVAILABLE_PB1, pInitParams->qm.uNumberOfFreePages[1]);

  /* rgf - Jan 18 2007 - Setup per PRE per PB global backpressure thresholds */
  SAND_HAL_WRITE(userDeviceHandle, CA, QM_GLOBAL_INTF0_PB0_THRESHOLD,
                 SAND_HAL_SET_FIELD(CA, QM_GLOBAL_INTF0_PB0_THRESHOLD, FLOW_CTRL_THRESH1, pInitParams->qm.uGlobalInterfaceFlowControlThresh[0][0]) |
                 SAND_HAL_SET_FIELD(CA, QM_GLOBAL_INTF0_PB0_THRESHOLD, FLOW_CTRL_THRESH2, pInitParams->qm.uGlobalInterfaceFlowControlThresh2[0][0]));
  SAND_HAL_WRITE(userDeviceHandle, CA, QM_GLOBAL_INTF0_PB1_THRESHOLD,
                 SAND_HAL_SET_FIELD(CA, QM_GLOBAL_INTF0_PB1_THRESHOLD, FLOW_CTRL_THRESH1, pInitParams->qm.uGlobalInterfaceFlowControlThresh[0][1]) |
                 SAND_HAL_SET_FIELD(CA, QM_GLOBAL_INTF0_PB1_THRESHOLD, FLOW_CTRL_THRESH2, pInitParams->qm.uGlobalInterfaceFlowControlThresh2[0][1]));

  SAND_HAL_WRITE(userDeviceHandle, CA, QM_GLOBAL_INTF1_PB0_THRESHOLD,
                 SAND_HAL_SET_FIELD(CA, QM_GLOBAL_INTF1_PB0_THRESHOLD, FLOW_CTRL_THRESH1, pInitParams->qm.uGlobalInterfaceFlowControlThresh[1][0]) |
                 SAND_HAL_SET_FIELD(CA, QM_GLOBAL_INTF1_PB0_THRESHOLD, FLOW_CTRL_THRESH2, pInitParams->qm.uGlobalInterfaceFlowControlThresh2[1][0]));
  SAND_HAL_WRITE(userDeviceHandle, CA, QM_GLOBAL_INTF1_PB1_THRESHOLD,
                 SAND_HAL_SET_FIELD(CA, QM_GLOBAL_INTF1_PB1_THRESHOLD, FLOW_CTRL_THRESH1, pInitParams->qm.uGlobalInterfaceFlowControlThresh[1][1]) |
                 SAND_HAL_SET_FIELD(CA, QM_GLOBAL_INTF1_PB1_THRESHOLD, FLOW_CTRL_THRESH2, pInitParams->qm.uGlobalInterfaceFlowControlThresh2[1][1]));

  SAND_HAL_WRITE(userDeviceHandle, CA, QM_GLOBAL_INTF2_PB0_THRESHOLD,
                 SAND_HAL_SET_FIELD(CA, QM_GLOBAL_INTF2_PB0_THRESHOLD, FLOW_CTRL_THRESH, pInitParams->qm.uGlobalInterfaceFlowControlThresh[2][0]));
  SAND_HAL_WRITE(userDeviceHandle, CA, QM_GLOBAL_INTF2_PB1_THRESHOLD,
                 SAND_HAL_SET_FIELD(CA, QM_GLOBAL_INTF2_PB1_THRESHOLD, FLOW_CTRL_THRESH, pInitParams->qm.uGlobalInterfaceFlowControlThresh[2][1]));

  SAND_HAL_WRITE(userDeviceHandle, CA, QM_GLOBAL_INTF3_PB0_THRESHOLD,
                 SAND_HAL_SET_FIELD(CA, QM_GLOBAL_INTF3_PB0_THRESHOLD, FLOW_CTRL_THRESH, pInitParams->qm.uGlobalInterfaceFlowControlThresh[3][0]));
  SAND_HAL_WRITE(userDeviceHandle, CA, QM_GLOBAL_INTF3_PB1_THRESHOLD,
                 SAND_HAL_SET_FIELD(CA, QM_GLOBAL_INTF3_PB1_THRESHOLD, FLOW_CTRL_THRESH, pInitParams->qm.uGlobalInterfaceFlowControlThresh[3][1]));

  SAND_HAL_WRITE(userDeviceHandle, CA, QM_GLOBAL_INTF4_PB0_THRESHOLD,
                 SAND_HAL_SET_FIELD(CA, QM_GLOBAL_INTF4_PB0_THRESHOLD, FLOW_CTRL_THRESH, pInitParams->qm.uGlobalInterfaceFlowControlThresh[4][0]));
  SAND_HAL_WRITE(userDeviceHandle, CA, QM_GLOBAL_INTF4_PB1_THRESHOLD,
                 SAND_HAL_SET_FIELD(CA, QM_GLOBAL_INTF4_PB1_THRESHOLD, FLOW_CTRL_THRESH, pInitParams->qm.uGlobalInterfaceFlowControlThresh[4][1]));

  SAND_HAL_WRITE(userDeviceHandle, CA, QM_GLOBAL_INTF5_PB0_THRESHOLD,
                 SAND_HAL_SET_FIELD(CA, QM_GLOBAL_INTF5_PB0_THRESHOLD, FLOW_CTRL_THRESH, pInitParams->qm.uGlobalInterfaceFlowControlThresh[5][0]));
  SAND_HAL_WRITE(userDeviceHandle, CA, QM_GLOBAL_INTF5_PB1_THRESHOLD,
                 SAND_HAL_SET_FIELD(CA, QM_GLOBAL_INTF5_PB1_THRESHOLD, FLOW_CTRL_THRESH, pInitParams->qm.uGlobalInterfaceFlowControlThresh[5][1]));

  /* rgf - Sep 29 2006 - Setup the drop queues */
  if( (pInitParams->qm.uBaseDropQueue & 0x3) != 0 ) {
      SB_ERROR("Bad configuration, the two LSBs of the base drop queue must be zero.  Specified value 0x%x", pInitParams->qm.uBaseDropQueue);
  }
  if( (pInitParams->qm.uNumberOfDropQueues < 1) || (pInitParams->qm.uNumberOfDropQueues > 4) ) {
    SB_ERROR("Bad Configuration, valid values for Number of Drop Queues are 1 through 4.  Specified value %d", pInitParams->qm.uNumberOfDropQueues);
  }
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, QM_CONFIG1, LOCKED_QUEUE_ENABLE, pInitParams->qm.bLockedQueueEnable);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, QM_CONFIG1, LOCKED_QUEUE, pInitParams->qm.uLockedQueue);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, QM_CONFIG1, DROP_QUEUES, pInitParams->qm.uNumberOfDropQueues);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, QM_CONFIG1, DROP_QUEUES_BASE, pInitParams->qm.uBaseDropQueue);
  /* rgf - Dec 04 2006 - Queue groups are groups of 16 queues, force bottom 4 bits to zero */
  /* rgf - Dec 04 2006 - Mark queue group and queues as allocated */
/*   m_uDropQueueGroup = (pInitParams->qm.uBaseDropQueue & SB_FE2000_QM_QUEUE_GROUP_MASK) >> SB_FE2000_QM_QUEUE_GROUP_SHIFT; */
/*   m_bQueueGroupAllocated[m_uDropQueueGroup] = TRUE; */

  /* Take QM block out of reset  */
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, QM_CORE_RESET, 0x0);

  /* bms - May 01 2006 - setup the number_free_pages_pb0 and number_free_pages_pb1 */
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, QM_FP_CONFIG, NUMBER_FREE_PAGES_PB0, pInitParams->qm.uNumberOfFreePages[0]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, QM_FP_CONFIG, NUMBER_FREE_PAGES_PB1, pInitParams->qm.uNumberOfFreePages[1]);
  SAND_HAL_WRITE(userDeviceHandle, CA,  QM_QUEUE_CONFIG_MEM_ACC_DATA0, 0);
  SAND_HAL_WRITE(userDeviceHandle, CA,  QM_QUEUE_CONFIG_MEM_ACC_DATA1, 0);
  SAND_HAL_WRITE(userDeviceHandle, CA,  QM_QUEUE_CONFIG_MEM_ACC_DATA2, 0);
  SAND_HAL_WRITE(userDeviceHandle, CA,  QM_QUEUE_STATE0_MEM_ACC_DATA, 0);
  SAND_HAL_WRITE(userDeviceHandle, CA,  QM_QUEUE_STATE1_MEM_ACC_DATA, 0);


  /* bms - May 01 2006 - now set the init bit */
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, QM_CONFIG0, INIT, 1);

  if( sbFe2000InitRegDone(userDeviceHandle,
			  SAND_HAL_REG_OFFSET(CA, QM_CONFIG0),
			  SAND_HAL_FIELD_MASK(CA, QM_CONFIG0, INIT_DONE),
			  SB_FE2000_QM_INIT_TIMEOUT) ) {
    SB_ERROR("Waiting for QM init_done timed out..\n");
    return SB_FE2000_STS_INIT_QM_TIMEOUT_ERR_K;
  }


  sbZfFe2000QmQueueConfigEntry_InitInstance(&zQueueConfig);

  zQueueConfig.m_uEnable = 1;
  zQueueConfig.m_uMaxPages = SB_FE2000_PB_PAGES_PER_PACKET_BUFFER;
  zQueueConfig.m_uDropThresh2 = SB_FE2000_PB_PAGES_PER_PACKET_BUFFER;
  zQueueConfig.m_uMinPages = 0;
  zQueueConfig.m_uEnforcePoliceMarkings = 0;
  zQueueConfig.m_uFlowControlEnable = 0;

  for( uDropQueue = pInitParams->qm.uBaseDropQueue; uDropQueue < (pInitParams->qm.uBaseDropQueue + pInitParams->qm.uNumberOfDropQueues); uDropQueue++ ) {
    CaQmQueueConfigWrite( userDeviceHandle,uDropQueue, &zQueueConfig);

    /* bms - Nov 08 2006 - Since only QueueState0 is back door and since we are only setting this enable bit before */
    /* bms - Nov 08 2006 - passing any traffic we can safely do a RMW */
    sbZfFe2000QmQueueState0Entry_InitInstance(&zQueueState);
    CaQmQueueState0Read( userDeviceHandle,uDropQueue, &zQueueState );
    zQueueState.m_uEnable = 1;
    zQueueState.m_uEmpty = 1;
    CaQmQueueState0Write( userDeviceHandle,uDropQueue, &zQueueState );
  }

  /* Init Locked queue for OAM */
  sbZfFe2000QmQueueConfigEntry_InitInstance(&zQueueConfig);
  zQueueConfig.m_uEnable = 1;
  zQueueConfig.m_uMaxPages = SB_FE2000_QM_OAM_FREEPAGES_DEFAULT;
  zQueueConfig.m_uDropThresh2 = SB_FE2000_QM_OAM_FREEPAGES_DEFAULT + 1;
  zQueueConfig.m_uMinPages = 0;
  zQueueConfig.m_uEnforcePoliceMarkings = 0;
  zQueueConfig.m_uFlowControlEnable = 0;
  CaQmQueueConfigWrite( userDeviceHandle,pInitParams->qm.uLockedQueue, &zQueueConfig);
  sbZfFe2000QmQueueState0Entry_InitInstance(&zQueueState);
  CaQmQueueState0Read(userDeviceHandle, pInitParams->qm.uLockedQueue,
                       &zQueueState );
  zQueueState.m_uEnable = 1;
  zQueueState.m_uEmpty = 1;
  CaQmQueueState0Write(userDeviceHandle, pInitParams->qm.uLockedQueue,
                       &zQueueState );

  /* Clear init done and init bits  */
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, QM_CONFIG0, INIT, 0);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, QM_CONFIG0, INIT_DONE, 0);

  DEXIT();
  return SB_FE2000_STS_INIT_OK_K;
}

static uint32_t
sbFe2000InitSpiTx(sbhandle userDeviceHandle,
                  uint16_t nSpi,
                  sbFe2000InitParams_t *pInitParams) {
  uint32_t uConfig0;
  uint32_t uConfig1;
  uint32_t uValue;
  int32_t  nCalendarLength;
  int32_t  nPort;
  int32_t  nFifoBottom;
  int32_t  nFifoTop;
  int32_t  nFieldIdx;
  int32_t  nCalendarIdx;
  int32_t  nCalendarEntry;
#define SB_FE2000_CALENDAR_ENTRIES_PER_REGISTER 4
  DENTER();

  nCalendarLength = pInitParams->st[nSpi].nCalendarLength;
  if (nCalendarLength == 0) {
      nCalendarLength = 1;
  }

  /* csk - May 01 2007 - Handle SPI loopback if requested */
  SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,ST,nSpi,ST_CONFIG2,LOOPBACK_ENABLE,
                            pInitParams->st[nSpi].bLoopback);

  uConfig0 = SAND_HAL_READ_STRIDE(userDeviceHandle, CA, ST, nSpi, ST0_CONFIG0);

  /*
   * rgf - Apr 26 2006 - Now setup arb and status calendars.  The following assumptions
   * rgf - Apr 26 2006 - and restrictions currently exist.
   * rgf - Apr 26 2006 -
   * rgf - Apr 26 2006 -    1) All calendar registers are contiguous
   * rgf - Apr 26 2006 -       (i.e. +4 in address gets to next register)
   * rgf - Apr 26 2006 -    2) All calendar config fields are laid out the same
   * rgf - Apr 26 2006 -    3) There are 4 calender entry fields per register
   * rgf - Apr 26 2006 -    4) The arb and status calendars are the same.
   * rgf - Apr 26 2006 -
   * rgf - Apr 26 2006 - Currently, these are true.  If the arb and status calendars end up
   * rgf - Apr 26 2006 - needing to be different then the init params structure will need to
   * rgf - Apr 26 2006 - change to allow representing that to this routine.
   */

  uConfig0 = SAND_HAL_MOD_FIELD(CA, ST0_CONFIG0, TX_CAL_LEN, uConfig0,
                                nCalendarLength - 1);
  uConfig0 = SAND_HAL_MOD_FIELD(CA, ST0_CONFIG0, TX_ARB_LEN, uConfig0,
                                nCalendarLength - 1);
  uConfig0 = SAND_HAL_MOD_FIELD(CA, ST0_CONFIG0, TX_CAL_M,   uConfig0, (pInitParams->st[nSpi].nCalendarM-1));
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA, ST, nSpi, ST0_CONFIG0, uConfig0);

  uConfig1 = SAND_HAL_READ_STRIDE(userDeviceHandle, CA, ST, nSpi, ST0_CONFIG1);
  /* Control only at SOP/EOP  */
  uConfig1 = SAND_HAL_MOD_FIELD(CA, ST0_CONFIG1, TX_CTRL_MAX_DISABLE, uConfig1, 0x1 );
  uConfig1 = SAND_HAL_MOD_FIELD(CA, ST0_CONFIG1, TX_MAXBURST2, uConfig1, (pInitParams->st[nSpi].nMaxBurst2));
  uConfig1 = SAND_HAL_MOD_FIELD(CA, ST0_CONFIG1, TX_MAXBURST1, uConfig1, (pInitParams->st[nSpi].nMaxBurst1));
  /* Disable interval training  */
  uConfig1 = SAND_HAL_MOD_FIELD(CA, ST0_CONFIG1, DATA_MAX_T,   uConfig1, 0x0);
  /* repeat training only once  */
  uConfig1 = SAND_HAL_MOD_FIELD(CA, ST0_CONFIG1, ALPHA,        uConfig1, 0x2);
  uConfig1 = SAND_HAL_MOD_FIELD(CA, ST0_CONFIG1, TSCLK_EDGE,   uConfig1, (pInitParams->st[nSpi].nTSclkEdge));
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA, ST, nSpi, ST0_CONFIG1, uConfig1);

  /*
   * - Setup the streaming packet fifo such that all enabled
   * - ports are equally sharing the fifo.
   * - give high speed channel larger fifo
   */

  nFifoBottom = 0;
  for ( nPort=0; nPort<nCalendarLength; nPort++ ) {
      nFifoTop = nFifoBottom + pInitParams->st[nSpi].nFifoLines[nPort] - 1;

      uValue  = SAND_HAL_SET_FIELD(CA, ST_P0_FIFO_CONFIG, FIFO_TOP, nFifoTop);
      uValue |=
          SAND_HAL_SET_FIELD(CA, ST_P0_FIFO_CONFIG, FIFO_BOTTOM, nFifoBottom);
      SAND_HAL_WRITE_INDEX_STRIDE(userDeviceHandle, CA, ST, nSpi,
                                  ST0_P0_FIFO_CONFIG, nPort, uValue);
      nFifoBottom = nFifoTop + 1;
  }

  for ( nCalendarIdx=0; nCalendarIdx < nCalendarLength; ) {
    uValue = 0;

    /* rgf - Apr 26 2006 - setup for the 32-bit write (4 entries)  */
    for ( nFieldIdx=0;
          nFieldIdx < SB_FE2000_CALENDAR_ENTRIES_PER_REGISTER;
          nFieldIdx++ ) {
      nCalendarEntry = pInitParams->st[nSpi].nCalendarEntry[nCalendarIdx++];
      uValue |= (nCalendarEntry&0xFF)<<(8*nFieldIdx);
    }

    SAND_HAL_WRITE_INDEX_STRIDE(userDeviceHandle, CA, ST, nSpi,
                                ST0_CAL_CONFIG0, (nCalendarIdx/4)-1, uValue);
    SAND_HAL_WRITE_INDEX_STRIDE(userDeviceHandle, CA, ST, nSpi,
                                ST0_TX_ARB_CONFIG0, (nCalendarIdx/4)-1,
                                uValue);
  }

  /* rgf - Apr 26 2006 - Take block out of reset and enable  */
  if ( 0 == nSpi ) {
    SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, ST0_CORE_RESET, 0x0);
  } else {
    SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, ST1_CORE_RESET, 0x0);
  }

  uConfig1 = SAND_HAL_MOD_FIELD(CA, ST0_CONFIG1, PC_RESET, uConfig1, 0x0);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA, ST, nSpi, ST0_CONFIG1, uConfig1);

  
  thin_delay(300);

  uConfig0 = SAND_HAL_MOD_FIELD(CA, ST0_CONFIG0, TX_ENABLE, uConfig0, 1);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA, ST, nSpi, ST0_CONFIG0, uConfig0);

  return SB_FE2000_STS_INIT_OK_K;
}


static uint32_t
sbFe2000InitSpiRx(sbhandle userDeviceHandle,
                  uint16_t nSpi,
                  sbFe2000InitParams_t *pInitParams) {
  uint32_t uConfig0;
  uint32_t uValue;
  uint32_t uPortEnable = 0;
  int32_t  nPort;
  int32_t  nFifoBottom;
  int32_t  nFifoTop;
  uint32_t nCalendarIdx;
  uint32_t nCalendarEntry;
  uint32_t nCalendarLength;
  uint32_t nFieldIdx;
  uint32_t nCalendarEntriesPerRegister = 4;

  DENTER();

  nCalendarLength = pInitParams->sr[nSpi].nCalendarLength;
  if (nCalendarLength == 0) {
      nCalendarLength = 1;
  }

  /* csk - May 01 2007 - Handle SPI loopback if requested */
  SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,SR,nSpi,SR_CONFIG1,LOOPBACK_ENABLE,
                           pInitParams->sr[nSpi].bLoopback);

  /* jts - Apr 06 2006 - ALIGN_DLY is the only field, just write (not RMW) */
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA,SR,nSpi,SR0_CONFIG4,pInitParams->sr[nSpi].nAlignDelay);

  uConfig0 = SAND_HAL_READ_STRIDE(userDeviceHandle, CA,SR,nSpi,SR0_CONFIG5);
  uConfig0 = SAND_HAL_MOD_FIELD(CA,SR0_CONFIG5,MAXBURST2,uConfig0,(pInitParams->sr[nSpi].nMaxBurst2));
  uConfig0 = SAND_HAL_MOD_FIELD(CA,SR0_CONFIG5,MAXBURST1,uConfig0,(pInitParams->sr[nSpi].nMaxBurst1));
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA,SR,nSpi,SR0_CONFIG5,uConfig0);

  /*
   * jts - Apr 06 2006 - Enable SPI Rx ports based on the calendar.  If a port
   * jts - Apr 06 2006 - is listed in the calendar then we'll enable it.
   */

  /* jts - Apr 06 2006 - first, operate on ports 0-31  */
  for ( nCalendarIdx=0; nCalendarIdx < nCalendarLength; nCalendarIdx++ ) {
    nCalendarEntry = pInitParams->sr[nSpi].nCalendarEntry[nCalendarIdx];
    if ( nCalendarEntry < 32 ) {
      uPortEnable |= 1 << nCalendarEntry;
    }
  }

  SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA,SR,nSpi,SR0_CONFIG2,uPortEnable);

  /* jts - Apr 06 2006 - next, operate on ports 32-64  */
  for ( nCalendarIdx=0; nCalendarIdx < nCalendarLength; nCalendarIdx++ ) {
    nCalendarEntry = pInitParams->sr[nSpi].nCalendarEntry[nCalendarIdx];
    if ( nCalendarEntry > 31 && nCalendarEntry < 64 ) {
      uPortEnable |= 1 << (nCalendarEntry-32);
    }
  }

  SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA, SR, nSpi, SR0_CONFIG3,
                        uPortEnable);

  /* jts - Apr 06 2006 - Setup the calendar  */
  uConfig0 = SAND_HAL_READ_STRIDE(userDeviceHandle, CA, SR, nSpi, SR0_CONFIG0);

  /* jts - Apr 06 2006 - Now setup calendar length, calendar M,  and the caldendar entries themselves
   * jts - Apr 06 2006 - due to the indexing of the registers, and naming of the fields
   * jts - Apr 06 2006 - this code is based on the following assumptions:
   * jts - Apr 06 2006 -
   * jts - Apr 06 2006 -    1) All calendar registers are contiguous (i.e. +4 in address gets to next register)
   * jts - Apr 06 2006 -    2) All calendar config fields are laid out the same
   * jts - Apr 06 2006 -    3) There are 4 calender entry fields per register
   * jts - Apr 06 2006 -
   * jts - Apr 06 2006 - Currently, these are true
   */
  uConfig0 = SAND_HAL_MOD_FIELD(CA, SR0_CONFIG0, RX_CAL_LEN, uConfig0,
                                nCalendarLength-1);
  uConfig0 = SAND_HAL_MOD_FIELD(CA, SR0_CONFIG0, RX_CAL_M,   uConfig0, (pInitParams->sr[nSpi].nCalendarM-1));
  uConfig0 = SAND_HAL_MOD_FIELD(CA, SR0_CONFIG0, RX_DIP4_LOS_THRESH, uConfig0, 2);

  /* SAND_HAL_WRITE(CA_BASE_SR(userDeviceHandle,nSpi), CA, SR0_CONFIG0, uConfig0); */
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA,SR,nSpi,SR0_CONFIG0,uConfig0);

  for ( nCalendarIdx=0; nCalendarIdx < nCalendarLength; ) {
    uValue = 0;

    /* jts - Apr 06 2006 - setup for the 32-bit write (4 entries)  */
    for ( nFieldIdx=0; nFieldIdx < nCalendarEntriesPerRegister; nFieldIdx++ ) {
      nCalendarEntry = pInitParams->sr[nSpi].nCalendarEntry[nCalendarIdx++];
      uValue |= (nCalendarEntry&0xFF)<<(8*nFieldIdx);
    }

    SAND_HAL_WRITE_INDEX_STRIDE(userDeviceHandle, CA, SR, nSpi,
                                SR0_CAL_CONFIG0, (nCalendarIdx/4)-1, uValue);
  }

  nFifoBottom = 0;
  for ( nPort=0; nPort<nCalendarLength; nPort++ ) {
      nFifoTop = nFifoBottom + pInitParams->sr[nSpi].nFifoLines[nPort] - 1;

      uValue  = SAND_HAL_SET_FIELD(CA, SR_P0_FIFO_CONFIG, FIFO_TOP, nFifoTop);
      uValue |=
          SAND_HAL_SET_FIELD(CA, SR_P0_FIFO_CONFIG, FIFO_BOTTOM, nFifoBottom);
      SAND_HAL_WRITE_INDEX_STRIDE(userDeviceHandle, CA, SR, nSpi,
                                  SR0_P0_FIFO_CONFIG, nPort, uValue);
      nFifoBottom = nFifoTop + 1;
  }

  /*
   * jts - Apr 06 2006 - Setup the min/max frame size per port
   * jts - Apr 06 2006 - Here we make similar assumptions to the calendar:
   * jts - Apr 06 2006 -
   * jts - Apr 06 2006 -    1) All registers are contiguous
   * jts - Apr 06 2006 -    2) All registers have same field layout
   * jts - Apr 06 2006 -
   * jts - Apr 06 2006 - Currently, assumptions are correct
   *
   * jts - Apr 06 2006 - NOTE: A bit of an optimization, we loop over the calendar to find
   * jts - Apr 06 2006 - ports rather than looping over ALL ports.  This way if the calendar
   * jts - Apr 06 2006 - is short (typical case) we save writing min/max frame sizes for
   * jts - Apr 06 2006 - ports that are not enabled.
   */
  for ( nCalendarIdx=0; nCalendarIdx< nCalendarLength; nCalendarIdx++) {
    nPort  = pInitParams->sr[nSpi].nCalendarEntry[nCalendarIdx];
    uValue = 0;
    uValue = SAND_HAL_MOD_FIELD(CA, SR0_P0_FRAME_SIZE, MIN_FRAME_SIZE,
                                uValue, pInitParams->sr[nSpi].nMinFrameSize[nPort]);
    uValue = SAND_HAL_MOD_FIELD(CA, SR0_P0_FRAME_SIZE, MAX_FRAME_SIZE,
                                uValue, pInitParams->sr[nSpi].nMaxFrameSize[nPort]);

    SAND_HAL_WRITE_INDEX_STRIDE(userDeviceHandle, CA, SR, nSpi,
                                SR0_P0_FRAME_SIZE, nPort, uValue);
  }


  if ( nSpi == 0 ) {
    SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, SR0_CORE_RESET, 0x0);
  } else {
    SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, SR1_CORE_RESET, 0x0);
  }

  SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle, CA, SR, nSpi, SR0_PC_CONFIG,
                            RSCLK_EDGE, (pInitParams->sr[nSpi].nRSclkEdge));
  SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle, CA, SR, nSpi, SR0_PC_CONFIG,
                            BANDGAP_TRIM, 10);

  uConfig0 = SAND_HAL_MOD_FIELD(CA, SR0_CONFIG0, PC_RESET, uConfig0, 0x0);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA, SR, nSpi, SR0_CONFIG0, uConfig0);

  uConfig0 = SAND_HAL_MOD_FIELD(CA, SR0_CONFIG0, RX_ENABLE, uConfig0, 1);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA, SR, nSpi, SR0_CONFIG0, uConfig0);

  return SB_FE2000_STS_INIT_OK_K;
}

static uint32_t
sbFe2000InitSpi(sbhandle userDeviceHandle,
                uint16_t nNum,
                sbFe2000InitParams_t *pInitParams)
{
  uint32_t uStatus;
  DENTER();
  uStatus = sbFe2000InitSpiTx(userDeviceHandle,nNum,pInitParams);
  if (uStatus != SB_FE2000_STS_INIT_OK_K) {
    SB_ERROR("%sTx init failed with status 0x%x\n",__PRETTY_FUNCTION__,uStatus);
    return uStatus;
  }
  uStatus = sbFe2000InitSpiRx(userDeviceHandle,nNum,pInitParams);
  if (uStatus != SB_FE2000_STS_INIT_OK_K) {
    SB_ERROR("%sRx init failed with status 0x%x\n",__PRETTY_FUNCTION__,uStatus);
    return uStatus;
  }
  DEXIT();
  return SB_FE2000_STS_INIT_OK_K;
}

static uint32_t
sbFe2000InitXgm(sbhandle userDeviceHandle, uint16_t nMac,
                sbFe2000InitParams_t *pInitParams)
{

  uint32_t uConfigData;
  uint32_t uDataLo;
  uint32_t uDataHi;
  uint32_t uData, uMemAccCtrl;
  uint32_t bClause45 = TRUE;
  uint32_t uPhyOrPortAddr;
  uint32_t uStatus;
  uint32_t uScanBusy;
  uint32_t uTimeOut;

  DENTER();

  SB_LOGV1("Initializing Xgm%d .. \n",nMac);

  SB_ASSERT(!SAND_HAL_IS_FE2KXT((int)userDeviceHandle));

  /* Set mii clock to ~2.5Mhz for MDIO minimum cycle time of 400ns */
  uData = 0;
  uData = SAND_HAL_MOD_FIELD(CA, PC_MIIM_RATE_ADJUST, DIVIDEND, uData, 0x1);
  uData = SAND_HAL_MOD_FIELD(CA, PC_MIIM_RATE_ADJUST, DIVISOR, uData, 0x44);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PC_MIIM_RATE_ADJUST, uData);

  /* First disable automatic scan so there will be no collisions with mmi accesses. */
  uMemAccCtrl = SAND_HAL_READ(userDeviceHandle, CA, PC_MIIM_CONTROL0);
  if ( SAND_HAL_GET_FIELD(CA, PC_MIIM_CONTROL0, MIIM_LINK_SCAN_EN, uMemAccCtrl) ) {
    uMemAccCtrl = SAND_HAL_MOD_FIELD(CA, PC_MIIM_CONTROL0, MIIM_LINK_SCAN_EN, uMemAccCtrl, 0x0);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_MIIM_CONTROL0, uMemAccCtrl);
    uTimeOut = 0;
    uScanBusy = 1;
    while ( uScanBusy && (1000 > uTimeOut) ) {
	      uScanBusy = SAND_HAL_GET_FIELD(CA, PC_MIIM_STATUS, MIIM_SCAN_BUSY, SAND_HAL_READ(userDeviceHandle, CA, PC_MIIM_STATUS) );
      uTimeOut++;
    }
    if ( uTimeOut == 1000 ) {
      SB_ERROR("PC_MIIM_STATUS-MIIM_SCAN_BUSY, timeout waiting for scan to cease.\n");
    }
  }

  /* Take the block out of reset.  Also enable indirect access to MAC.  */
  if ( nMac == 0 ) {
    SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, XG0_CORE_RESET, 0x0);
    SAND_HAL_RMW_FIELD(userDeviceHandle, CA, XG0_CONFIG, XM_RESET, 0x0);
  } else {
    SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, XG1_CORE_RESET, 0x0);
    SAND_HAL_RMW_FIELD(userDeviceHandle, CA, XG1_CONFIG, XM_RESET, 0x0);
  }

  /* Config the XGXS via PCI control registers. */
  uConfigData = 0;
  uConfigData = SAND_HAL_MOD_FIELD(CA, PC_MIIM_XGXS_CONFIG_0,
                                   PHY_CLAUSE_45, uConfigData, bClause45);
  uConfigData = SAND_HAL_MOD_FIELD(CA, PC_MIIM_XGXS_CONFIG_0,
                                   MDIO_DEVAD, uConfigData, 0x1);
  uConfigData = SAND_HAL_MOD_FIELD(CA, PC_MIIM_XGXS_CONFIG_0,
                                   IEEE_DEVICES_IN_PKG, uConfigData, 0x1e);
  if ( nMac == 0 ) {
    SAND_HAL_WRITE(userDeviceHandle, CA, PC_MIIM_XGXS_CONFIG_0, uConfigData);
    uPhyOrPortAddr = 0x18;
  }
  else {
    SAND_HAL_WRITE(userDeviceHandle, CA, PC_MIIM_XGXS_CONFIG_1, uConfigData);
    uPhyOrPortAddr = 0x19;
  }

  /*
   *  Config the XGXS via 10G MAC control register.
   *  Lo bit ------------------------------------------------------------------- Hi bit
   *  DIGITAL_CLK_SHUTDOWN=0, ANALOG_CLK_SHUTDOWN=0, RESET_N=0, XGXS_MDIO_RESET_N=0
   *  XGXS_PLL_RESET_N=0, BIGMAC_RESET_N=0, TX_FIFO_RESET_N=0, TX_FIFO_AUTO_RESET_ENABLE=1
   *  BYPASS_PLL=0, SELECT_LOCAL_PLL=1, RX_LANE_SWAP=0, TX_LANE_SWAP=0,
   *  BOOTSTRAP_MODE=0001
   *  FORCE_BOOTSTRAP_MODE=1, REMOTE_LOOPBACK_ENABLE=0, RESET_FILTER_BYPASS=0
   */

  uStatus = sbFe2000UtilXgmRead(userDeviceHandle,nMac,SAND_HAL_REG_OFFSET(CA, XM_MAC_XGXS_CONFIG_LO), &uDataHi, &uDataLo); /* MAC_XGXS_CONFIG Indirect */
  if( SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  /* Power up, force mode */
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_XGXS_CONFIG_LO, MODE_FORCE, uDataLo, 0x1);
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_XGXS_CONFIG_LO, ANALOG_CLK_SHUTDOWN, uDataLo, 0x0);
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_XGXS_CONFIG_LO, DIGITAL_CLK_SHUTDOWN, uDataLo, 0x0);
  uStatus = sbFe2000UtilXgmWrite(userDeviceHandle,nMac,SAND_HAL_REG_OFFSET(CA, XM_MAC_XGXS_CONFIG_LO), uDataHi, uDataLo); /* MAC_XGXS_CONFIG Indirect */
  if( SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }
  /* Select LCPLL */
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_XGXS_CONFIG_LO, SELECT_LOCAL_PLL, uDataLo, 0x1);
  uStatus = sbFe2000UtilXgmWrite(userDeviceHandle,nMac, SAND_HAL_REG_OFFSET(CA, XM_MAC_XGXS_CONFIG_LO), uDataHi, uDataLo); /* MAC_XGXS_CONFIG Indirect */
  if( SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }
  /* Deassert reset */
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_XGXS_CONFIG_LO, RESET_N, uDataLo, 0x1);
  uStatus = sbFe2000UtilXgmWrite(userDeviceHandle,nMac, SAND_HAL_REG_OFFSET(CA, XM_MAC_XGXS_CONFIG_LO), uDataHi, uDataLo); /* MAC_XGXS_CONFIG Indirect */
  if( SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  /* Deassert TX FIFO reset */
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_XGXS_CONFIG_LO, TX_FIFO_RESET_N, uDataLo, 0x1);
  uStatus = sbFe2000UtilXgmWrite(userDeviceHandle,nMac, SAND_HAL_REG_OFFSET(CA, XM_MAC_XGXS_CONFIG_LO), uDataHi, uDataLo); /* MAC_XGXS_CONFIG Indirect */
  if( SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  /* Config 10G MAC main control registers.  */
  /* TX  */
  uDataLo = 0;
  uDataHi=0;
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_TX_CONFIG_LO, HDR_MODE,
                               uDataLo, pInitParams->xgt[nMac].nHeaderMode);
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_TX_CONFIG_LO, CRC_MODE,
                               uDataLo, pInitParams->xgt[nMac].nCrcMode);
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_TX_CONFIG_LO, AVG_IPG,
                               uDataLo, pInitParams->xgt[nMac].nAvgIpg);
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_TX_CONFIG_LO, THROTTLE_NUM,
                               uDataLo, pInitParams->xgt[nMac].nThrotNumer);
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_TX_CONFIG_LO, THROTTLE_DENOM,
                               uDataLo, pInitParams->xgt[nMac].nThrotDenom);
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_TX_CONFIG_LO, PAUSE_ENABLE,
                               uDataLo, pInitParams->xgt[nMac].bPauseEnable);
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_TX_CONFIG_LO, DROP_ALL,
                               uDataLo, pInitParams->xgt[nMac].bDiscard);
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_TX_CONFIG_LO, ANY_START,
                               uDataLo, pInitParams->xgt[nMac].bAnyStart);
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_TX_CONFIG_LO, HIGIG2_MODE,
                               uDataLo, pInitParams->xgt[nMac].nHiGig2Mode);
  uStatus = sbFe2000UtilXgmWrite(userDeviceHandle, nMac, SAND_HAL_REG_OFFSET(CA, XM_MAC_TX_CONFIG_LO), uDataHi, uDataLo);
  if( SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  /* RX  */
  uDataLo = 0;
  uDataHi=0;
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_RX_CONFIG_LO, HDR_MODE,
                               uDataLo, pInitParams->xgr[nMac].nHeaderMode);
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_RX_CONFIG_LO, STRIP_CRC,
                               uDataLo, pInitParams->xgr[nMac].bStripCrc);
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_RX_CONFIG_LO, IGNORE_CRC,
                               uDataLo, pInitParams->xgr[nMac].bIgnoreCrc);
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_RX_CONFIG_LO, STRICT_PREAMBLE,
                               uDataLo, pInitParams->xgr[nMac].bStrictPreamble);
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_RX_CONFIG_LO, RX_PAUSE_ENABLE,
                               uDataLo, pInitParams->xgr[nMac].bPauseEnable);
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_RX_CONFIG_LO, RX_PASS_CTRL,
                               uDataLo, pInitParams->xgr[nMac].bPassCtrl);
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_RX_CONFIG_LO, ANY_START,
                               uDataLo, pInitParams->xgr[nMac].bAnyStart);
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_RX_CONFIG_LO, HIGIG2_MODE,
                               uDataLo, pInitParams->xgr[nMac].nHiGig2Mode);
  uStatus = sbFe2000UtilXgmWrite(userDeviceHandle, nMac, SAND_HAL_REG_OFFSET(CA, XM_MAC_RX_CONFIG_LO), uDataHi, uDataLo);
  if( SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  /* Config 10G MAC RX, TX MAX_SIZE */
  uDataHi = 0;
  uStatus = sbFe2000UtilXgmWrite(userDeviceHandle, nMac, SAND_HAL_REG_OFFSET(CA, XM_MAC_RX_CONFIG_LO),
                             uDataHi, pInitParams->xgt[nMac].nMaxSize);
  if( SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  uStatus = sbFe2000UtilXgmWrite(userDeviceHandle, nMac, uDataHi,SAND_HAL_REG_OFFSET(CA, XM_MAC_TX_CONFIG_LO),
                             pInitParams->xgr[nMac].nMaxSize);
  if( SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

#ifdef EARLY_XM_ENABLE
  /*
   * Enable the 10G MAC RX/TX blocks.
   * Enable Rx and Tx and set loopbacks.
   */

  uDataLo = 0;
  uDataHi=0;
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_CONFIG_LO, REMOTE_LOOPBACK_ENABLE,
                               uDataLo, pInitParams->xg[nMac].bRemoteLoopback);
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_CONFIG_LO, LOCAL_LOOPBACK_ENABLE,
                               uDataLo, pInitParams->xg[nMac].bLocalLoopback);
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_CONFIG_LO, RX_ENABLE,
                               uDataLo, pInitParams->xg[nMac].bBringUp);
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_CONFIG_LO, TX_ENABLE,
                               uDataLo, pInitParams->xg[nMac].bBringUp);

  uStatus = sbFe2000UtilXgmWrite(userDeviceHandle, nMac, SAND_HAL_REG_OFFSET(CA, XM_MAC_CONFIG_LO),uDataHi, uDataLo);

  if( SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }
#endif

  /* Start automatic link status scanning */

  /* Scan all possible Xg ports */
  uMemAccCtrl = SAND_HAL_READ(userDeviceHandle, CA, PC_MIIM_SCAN_PORTS);
  uData = SAND_HAL_GET_FIELD(CA, PC_MIIM_SCAN_PORTS, MIIM_SCAN_PORTS, uMemAccCtrl);
  uData |= 0x3000000;
  uMemAccCtrl = SAND_HAL_MOD_FIELD(CA, PC_MIIM_SCAN_PORTS, MIIM_SCAN_PORTS, uMemAccCtrl, uData);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PC_MIIM_SCAN_PORTS, uMemAccCtrl);

  /* Scan internal ports */
  uMemAccCtrl = SAND_HAL_READ(userDeviceHandle, CA, PC_MIIM_INT_SEL_MAP);
  uData = SAND_HAL_GET_FIELD(CA, PC_MIIM_INT_SEL_MAP, MIIM_INT_SEL_MAP, uMemAccCtrl);
  uData |= 0x3000000;
  uMemAccCtrl = SAND_HAL_MOD_FIELD(CA, PC_MIIM_INT_SEL_MAP, MIIM_INT_SEL_MAP, uMemAccCtrl, uData);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PC_MIIM_INT_SEL_MAP, uMemAccCtrl);

  /* Enable automatic scan */
  uMemAccCtrl = SAND_HAL_READ(userDeviceHandle, CA, PC_MIIM_CONTROL0);
  uMemAccCtrl = SAND_HAL_MOD_FIELD(CA, PC_MIIM_CONTROL0, MIIM_LINK_SCAN_EN, uMemAccCtrl, 0x1);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PC_MIIM_CONTROL0, uMemAccCtrl);

  DEXIT();
  return SB_FE2000_STS_INIT_OK_K;
}

sbStatus_t
sbFe2000InitXgmRxTx(sbhandle userDeviceHandle, uint16_t nMac,
                 sbFe2000InitParams_t *pInitParams)
{
  uint32_t uDataLo;
  uint32_t uDataHi;
  uint32_t uStatus;

  /*
   * Enable the 10G MAC RX/TX blocks.
   * Enable Rx and Tx and set loopbacks.
   */

  uDataLo = 0;
  uDataHi=0;
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_CONFIG_LO, REMOTE_LOOPBACK_ENABLE,
                               uDataLo, pInitParams->xg[nMac].bRemoteLoopback);
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_CONFIG_LO, LOCAL_LOOPBACK_ENABLE,
                               uDataLo, pInitParams->xg[nMac].bLocalLoopback);
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_CONFIG_LO, RX_ENABLE,
                               uDataLo, pInitParams->xg[nMac].bBringUp);
  uDataLo = SAND_HAL_MOD_FIELD(CA, XM_MAC_CONFIG_LO, TX_ENABLE,
                               uDataLo, pInitParams->xg[nMac].bBringUp);

  uStatus = sbFe2000UtilXgmWrite(userDeviceHandle, nMac, SAND_HAL_REG_OFFSET(CA, XM_MAC_CONFIG_LO),uDataHi, uDataLo);

  if( SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  return SB_FE2000_STS_INIT_OK_K;
}

static uint32_t
sbFe2000InitAgmPortRx(sbhandle userDeviceHandle, uint16_t nAgmNum, uint16_t nPort,
                      sbFe2000InitParams_t *pInitParams)
{

  uint32_t uData, uSetting;
  uint32_t uStatus;

  DENTER();

  /* Command Config register setup  */
  uStatus = sbFe2000UtilAgmRead(userDeviceHandle, nAgmNum, nPort,
                            SAND_HAL_REG_OFFSET(CA, AM_CONFIG), &uData);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  uData = SAND_HAL_MOD_FIELD(CA, AM_CONFIG, RX_ENABLE, uData, 0x1);

  uSetting = ( pInitParams->ag[nAgmNum].common[nPort].bMacRemoteLoopback == TRUE ) ? 0x1 : 0x0;
  uData = SAND_HAL_MOD_FIELD(CA, AM_CONFIG, LINE_LOOPBACK_ENABLE, uData, uSetting);

  uSetting = ( pInitParams->ag[nAgmNum].common[nPort].bMacLocalLoopback == TRUE ) ? 0x1 : 0x0;
  uData = SAND_HAL_MOD_FIELD(CA, AM_CONFIG, LOOPBACK_ENABLE, uData, uSetting);

  uSetting = ( pInitParams->ag[nAgmNum].common[nPort].bSpeed1000Mbps == TRUE ) ? 0x2 :
    (( pInitParams->ag[nAgmNum].common[nPort].bSpeed100Mbps == TRUE ) ? 0x1 : 0x0);
  uData = SAND_HAL_MOD_FIELD(CA, AM_CONFIG, MAC_SPEED, uData, uSetting);

  uSetting = ( pInitParams->ag[nAgmNum].common[nPort].bFullDuplex == FALSE ) ? 0x1 : 0x0;
  uData = SAND_HAL_MOD_FIELD(CA, AM_CONFIG, HALF_DUPLEX, uData, uSetting);

  uSetting = ( pInitParams->ag[nAgmNum].rx[nPort].bPromiscuousEnable == TRUE ) ? 0x1 : 0x0;
  uData = SAND_HAL_MOD_FIELD(CA, AM_CONFIG, PROMISCUOUS_ENABLE, uData, uSetting);

  uSetting = ( pInitParams->ag[nAgmNum].rx[nPort].bCrcForward == TRUE ) ? 0x1 : 0x0;
  uData = SAND_HAL_MOD_FIELD(CA, AM_CONFIG, FORWARD_CRC, uData, uSetting);

  uSetting = ( pInitParams->ag[nAgmNum].rx[nPort].bPauseForward == TRUE ) ? 0x1 : 0x0;
  uData = SAND_HAL_MOD_FIELD(CA, AM_CONFIG, FORWARD_PAUSE_FRAMES, uData, uSetting);

  uSetting = ( pInitParams->ag[nAgmNum].rx[nPort].bPayloadLengthCheck == FALSE ) ? 0x1 : 0x0;
  uData = SAND_HAL_MOD_FIELD(CA, AM_CONFIG, LENGTH_CHECK_DISABLE, uData, uSetting);

  uSetting = ( pInitParams->ag[nAgmNum].rx[nPort].bMacControlEnable == TRUE ) ? 0x1 : 0x0;
  uData = SAND_HAL_MOD_FIELD(CA, AM_CONFIG, CONTROL_FRAME_ENABLE, uData, uSetting);

  uStatus = sbFe2000UtilAgmWrite(userDeviceHandle, nAgmNum, nPort, SAND_HAL_REG_OFFSET(CA, AM_CONFIG), uData);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }


  /* MAC_0 -- Core MAC address bit 47 to 16  */
  uStatus = sbFe2000UtilAgmWrite(userDeviceHandle, nAgmNum, nPort,
                             SAND_HAL_REG_OFFSET(CA, AM_MAC_ADDRESS0),
                             pInitParams->ag[nAgmNum].common[nPort].uuMacAddr >> 16);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }


  /* MAC_1 -- Core MAC address bit 15 to 0  */
  uStatus = sbFe2000UtilAgmWrite(userDeviceHandle, nAgmNum, nPort,
                             SAND_HAL_REG_OFFSET(CA, AM_MAC_ADDRESS1),
                             pInitParams->ag[nAgmNum].common[nPort].uuMacAddr & 0xffff);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  /* Rx MaxTU  */
  uStatus = sbFe2000UtilAgmWrite(userDeviceHandle, nAgmNum, nPort,
                             SAND_HAL_REG_OFFSET(CA, AM_RX_MAX_PKT_LENGTH),
                             pInitParams->ag[nAgmNum].rx[nPort].uMaxTransferUnit);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  DEXIT();
  return SB_FE2000_STS_INIT_OK_K;
}

static uint32_t
sbFe2000InitAgmPortTx(sbhandle userDeviceHandle, uint16_t nAgmNum, uint16_t nPort,
                      sbFe2000InitParams_t *pInitParams)
{

  uint32_t uData;
  uint32_t uStatus;

  DENTER();
  /* Command Config register setup  */
  uStatus = sbFe2000UtilAgmRead(userDeviceHandle, nAgmNum, nPort, SAND_HAL_REG_OFFSET(CA, AM_CONFIG), &uData);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  uData = SAND_HAL_MOD_FIELD(CA, AM_CONFIG, TX_ENABLE, uData, 0x1);
  if ( pInitParams->ag[nAgmNum].tx[nPort].bPadEnable == TRUE ) {
    uData = SAND_HAL_MOD_FIELD(CA, AM_CONFIG, PAD_TX_FRAMES, uData, 0x1);
  }

  uStatus = sbFe2000UtilAgmWrite(userDeviceHandle, nAgmNum, nPort, SAND_HAL_REG_OFFSET(CA, AM_CONFIG), uData);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  /* Tx IPG Length register setup  */
  uData = 0xc;
  uStatus = sbFe2000UtilAgmWrite(userDeviceHandle, nAgmNum, nPort,
                                 SAND_HAL_REG_OFFSET(CA, AM_TX_IPG_LENGTH),
                                 uData);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }


  SAND_HAL_WRITE_OFFS(userDeviceHandle, agmMacConfigCalcAddr(userDeviceHandle, nAgmNum, nPort),
                      pInitParams->ag[nAgmNum].tx[nPort].uCrcMode);

  DEXIT();
  return SB_FE2000_STS_INIT_OK_K;
}

static int
agmMacConfigCalcAddr(sbhandle userDeviceHandle, int nAgmNum, int nPort)
{
    int portStride = SAND_HAL_REG_OFFSET(CA, AG_MAC1_CONFIG)
        - SAND_HAL_REG_OFFSET(CA, AG_MAC0_CONFIG);

    return SAND_HAL_CA_AG_INSTANCE_ADDR_STRIDE * nAgmNum
        + SAND_HAL_REG_OFFSET(CA, AG_MAC0_CONFIG) + portStride * nPort;
}

static uint32_t
sbFe2000InitAgm(sbhandle userDeviceHandle, uint16_t nAgmNum, sbFe2000InitParams_t *pInitParams)
{

  int32_t  nPort;
  uint32_t uData;
  uint32_t uStatus;


  DENTER();
  SB_LOGV1("Initializing Agm%d .. \n",nAgmNum);

  /* uData = SAND_HAL_READ(CA_BASE_AG(userDeviceHandle,nAgmNum), CA, AG_CONFIG); */
  uData = SAND_HAL_READ_STRIDE(userDeviceHandle, CA,AG,nAgmNum,AG_CONFIG);

  uData = SAND_HAL_MOD_FIELD(CA, AG_CONFIG, SERDES_RESET_N, uData, 0x1);
  uData = SAND_HAL_MOD_FIELD(CA, AG_CONFIG, GPORT_RESET_N, uData, 0x1);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA, AG, nAgmNum, AG_CONFIG, uData);


  if ( pInitParams->ag[nAgmNum].bSwsLoopBackEnable == TRUE ) {
    /* uData = SAND_HAL_READ(CA_BASE_AG(userDeviceHandle,nAgmNum), CA, AG_DEBUG); */
    uData = SAND_HAL_READ_STRIDE(userDeviceHandle, CA,AG,nAgmNum,AG_DEBUG);
    uData = SAND_HAL_MOD_FIELD(CA, AG_DEBUG, SWS_LOOPBACK_ENABLE, uData, 0x1);
    uData = SAND_HAL_MOD_FIELD(CA, AG_DEBUG, SWS_LOOPBACK_CRC_MODE, uData,
                               pInitParams->ag[nAgmNum].uSwsLoopBackCrcMode);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA, AG, nAgmNum, AG_DEBUG, uData);
  }

  for ( nPort = 0; nPort < SB_FE2000_MAX_AG_PORTS; nPort++ ) {
    uStatus = sbFe2000InitAgmPortRx(userDeviceHandle, nAgmNum, nPort, pInitParams);
    if( SB_FE2000_STS_INIT_OK_K != uStatus ) {
      return uStatus;
    }
    uStatus = sbFe2000InitAgmPortTx(userDeviceHandle, nAgmNum, nPort, pInitParams);
    if( SB_FE2000_STS_INIT_OK_K != uStatus ) {
      return uStatus;
    }
  }

  /* GPORT_CONFIG  */
  uStatus = sbFe2000UtilAgmRead(userDeviceHandle, nAgmNum, SB_FE2000_MAX_AG_PORTS,
                            SAND_HAL_REG_OFFSET(CA, GPORT_CONFIG), &uData);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  uData = SAND_HAL_MOD_FIELD(CA, GPORT_CONFIG, CLEAR_MIB_COUNTERS, uData, 0x1);
  uStatus = sbFe2000UtilAgmWrite(userDeviceHandle, nAgmNum, SB_FE2000_MAX_AG_PORTS,
                             SAND_HAL_REG_OFFSET(CA, GPORT_CONFIG), uData);

  uStatus = sbFe2000UtilAgmRead(userDeviceHandle, nAgmNum, SB_FE2000_MAX_AG_PORTS,
                            SAND_HAL_REG_OFFSET(CA, GPORT_CONFIG), &uData);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  uData = SAND_HAL_MOD_FIELD(CA, GPORT_CONFIG, CLEAR_MIB_COUNTERS, uData, 0x0);
  uData = SAND_HAL_MOD_FIELD(CA, GPORT_CONFIG, ENABLE, uData, 0x1);
  uStatus = sbFe2000UtilAgmWrite(userDeviceHandle, nAgmNum, SB_FE2000_MAX_AG_PORTS,
                             SAND_HAL_REG_OFFSET(CA, GPORT_CONFIG), uData);

  /* RSV mask register -- Control which RSV bits cause packets to be purged  */
  uStatus = sbFe2000UtilAgmRead(userDeviceHandle, nAgmNum, SB_FE2000_MAX_AG_PORTS,
                            SAND_HAL_REG_OFFSET(CA, GPORT_RX_PURGE_CONFIG), &uData);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  uData |= 0x00000040;  /* Drop "frame length out of range" packets  */
  if ( pInitParams->ag[nAgmNum].bCrcCheck == FALSE ) {
    uData &= 0xffffffef;
  }
  uStatus = sbFe2000UtilAgmWrite(userDeviceHandle, nAgmNum, SB_FE2000_MAX_AG_PORTS,
                             SAND_HAL_REG_OFFSET(CA, GPORT_RX_PURGE_CONFIG), uData);

  /*
   * RSV stat mask register --
   * Control which RSV events should cause a statistic counter update
   */

  uStatus = sbFe2000UtilAgmRead(userDeviceHandle, nAgmNum, SB_FE2000_MAX_AG_PORTS,
                            SAND_HAL_REG_OFFSET(CA, GPORT_RX_STAT_UPDATE_CONFIG), &uData);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  if ( pInitParams->ag[nAgmNum].bCrcCheck == FALSE ) {
    uData &= 0xffffffef;
  }

  uStatus = sbFe2000UtilAgmWrite(userDeviceHandle, nAgmNum, SB_FE2000_MAX_AG_PORTS,
                             SAND_HAL_REG_OFFSET(CA, GPORT_RX_STAT_UPDATE_CONFIG), uData);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  /*
   * GPORT CNTMAXSIZE register -- The max packet size that is used
   * in statistic counter update
   */
  uStatus = sbFe2000UtilAgmWrite(userDeviceHandle, nAgmNum, SB_FE2000_MAX_AG_PORTS, SAND_HAL_REG_OFFSET(CA, GPORT_MAX_PKT_SIZE_STATUS_UPDATE),
                             pInitParams->ag[nAgmNum].uMaxStatCounterUnit);

  DEXIT();
  return uStatus;
}

static uint32_t
sbFe2000InitAgms(sbhandle userDeviceHandle,
                 sbFe2000InitParams_t *pInitParams)
{
  int32_t nAgmNum;
  uint32_t uData, uMemAccCtrl;
  uint32_t uStatus;

  DENTER();

  if ( pInitParams->ag[0].bBringUp || pInitParams->ag[1].bBringUp ) {
    /* Set mii clock to ~2.5Mhz for MDIO minimum cycle time of 400ns */
    uData = 0;
    uData = SAND_HAL_MOD_FIELD(CA, PC_MIIM_RATE_ADJUST, DIVIDEND, uData, 0x1);
    uData = SAND_HAL_MOD_FIELD(CA, PC_MIIM_RATE_ADJUST, DIVISOR, uData, 0x44);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_MIIM_RATE_ADJUST, uData);
  }

  for ( nAgmNum = 0; nAgmNum < SB_FE2000_NUM_AG_INTERFACES; ++nAgmNum ) {
    if ( pInitParams->ag[nAgmNum].bBringUp ) {
      /* Take the block out of reset.  Also enable indirect access to MAC.  */
      if ( nAgmNum == 0 ) {
        SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, AG0_CORE_RESET, 0x0);
      } else {
        SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, AG1_CORE_RESET, 0x0);
      }

      uStatus = sbFe2000InitAgm(userDeviceHandle, nAgmNum, pInitParams);
      if( SB_FE2000_STS_INIT_OK_K != uStatus ) {
        return uStatus;
      }
    }
  }

  if ( pInitParams->ag[0].bBringUp || pInitParams->ag[1].bBringUp ) {
    /*
     * Start automatic link status scanning
     * Scan all possible ports
     *
     * NOTE:  This must be turned off when accessing to MII regs!
     */
    uMemAccCtrl = 0;
    uMemAccCtrl = SAND_HAL_MOD_FIELD(CA, PC_MIIM_SCAN_PORTS,
                                     MIIM_SCAN_PORTS, uMemAccCtrl, 0xffffff);
    SAND_HAL_WRITE(userDeviceHandle, CA, PC_MIIM_SCAN_PORTS, uMemAccCtrl);

    /* Scan internal ports  */
    uMemAccCtrl = 0;
    uMemAccCtrl = SAND_HAL_MOD_FIELD(CA, PC_MIIM_INT_SEL_MAP, MIIM_INT_SEL_MAP, uMemAccCtrl, 0xffffff);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_MIIM_INT_SEL_MAP, uMemAccCtrl);

    /* Set automatic scan enable  */
    /* This can not be turned on, it is not compatible with phy probing. */
    uMemAccCtrl = SAND_HAL_READ(userDeviceHandle, CA, PC_MIIM_CONTROL0);
    uMemAccCtrl = SAND_HAL_MOD_FIELD(CA, PC_MIIM_CONTROL0, MIIM_LINK_SCAN_EN, uMemAccCtrl, 0x0);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_MIIM_CONTROL0, uMemAccCtrl);
  }

  DEXIT();
  return SB_FE2000_STS_INIT_OK_K;
}

static void
sbFe2000InitPb(sbhandle userDeviceHandle,
               sbFe2000InitParams_t *pInitParams)
{

  SB_ASSERT(!SAND_HAL_IS_FE2KXT((int)userDeviceHandle));

  /* rgf - May 11 2006 - Take block out of reset */
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, PB0_CORE_RESET, 0x0);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, PB1_CORE_RESET, 0x0);
}

static void
sbFe2000InitPd(sbhandle userDeviceHandle,
               sbFe2000InitParams_t *pInitParams)
{

  sbFe2000InitParamsPd_t *pPdInitParams = &(pInitParams->pd);
  sbFe2000InitParamsPdHeaderConfig_t *pHeaderConfig = pPdInitParams->HeaderConfig;
  uint32_t uIndex=0;

  /* bring out of reset */
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, PD_CORE_RESET, 0x0);


  {

    for(uIndex =0; uIndex < SB_FE2000_PD_NUM_HEADER_CONFIGS; uIndex++) {
      sbZfFe2000PdHeaderConfig_t zHeaderConfig;
      zHeaderConfig.m_uBaseLength = pHeaderConfig[uIndex].uBaseLength;
      zHeaderConfig.m_uLengthPosition = pHeaderConfig[uIndex].uLengthPosition;
      zHeaderConfig.m_uLengthSize = pHeaderConfig[uIndex].uLengthSize;
      zHeaderConfig.m_uLengthUnits = pHeaderConfig[uIndex].uLengthUnits;
      if ( CaPdHeaderConfigWrite(userDeviceHandle, uIndex, &zHeaderConfig) ) {
      } else {
        SB_ERROR("%s Failed on write for HeaderConfig(%d).",__PRETTY_FUNCTION__, uIndex);
      }
    }
  }


  {
    uint32_t uPdConfig = SAND_HAL_READ(userDeviceHandle, CA,PD_CONFIG);
    uPdConfig = SAND_HAL_MOD_FIELD(CA,PD_CONFIG,TRUNC_REMOVE,uPdConfig,pInitParams->pd.uTruncationRemoveValue);
    uPdConfig = SAND_HAL_MOD_FIELD(CA,PD_CONFIG,TRUNC_LENGTH,uPdConfig,pInitParams->pd.uTruncationLengthValue);
    if(pPdInitParams->bRouteHeaderPresent) {
     uPdConfig = SAND_HAL_MOD_FIELD(CA,PD_CONFIG,FRAME_LEN_ADJUST,uPdConfig,0x1);
    }
    if(pPdInitParams->bDoContinueByteAdjust) {
     uPdConfig = SAND_HAL_MOD_FIELD(CA,PD_CONFIG,CONT_BYTE_ADJUST,uPdConfig,0x1);
    } else {
     uPdConfig = SAND_HAL_MOD_FIELD(CA,PD_CONFIG,CONT_BYTE_ADJUST,uPdConfig,0x0);
    }
    if(pPdInitParams->bDoConditionalIpv4CheckSumUpdate) {
     uPdConfig = SAND_HAL_MOD_FIELD(CA,PD_CONFIG,CONDITIONAL_UPDATE,uPdConfig,0x1);
    }
    SAND_HAL_WRITE(userDeviceHandle, CA,PD_CONFIG,uPdConfig);
  }

  {
  uint32_t uPdDebug = SAND_HAL_READ(userDeviceHandle, CA,PD_DEBUG);
  uPdDebug = SAND_HAL_MOD_FIELD(CA,PD_DEBUG,EMBEDDED_LENGTH_TYPE,uPdDebug,CA_PD_DEFAULT_EMBEDDED_LENGTH_HDRID);
  if ( pInitParams->pd.bDoIpv4CheckSumUpdate == TRUE || pInitParams->pd.bDoConditionalIpv4CheckSumUpdate == TRUE ) {
    uPdDebug = SAND_HAL_MOD_FIELD(CA,PD_DEBUG,IPV4_HDR_TYPE,uPdDebug,SB_FE2000_PP_IPV4_HDR_TYPE);
  }
  SAND_HAL_WRITE(userDeviceHandle, CA,PD_DEBUG,uPdDebug);

  /* set len pos for embedded route len header to 0x1ff */
  uPdDebug = SAND_HAL_MOD_FIELD(CA,PD_HDR_CONFIG14,LEN_POSN,uPdDebug,CA_PD_DEFAULT_LEN_POS);
  SAND_HAL_WRITE(userDeviceHandle, CA,PD_HDR_CONFIG14,uPdDebug);
  }
}


static void
sbFe2000InitCm(sbhandle userDeviceHandle,
               sbFe2000InitParams_t *pInitParams) {

  uint32_t uCmDmaThresh    = pInitParams->cm.uCmDmaThresh;
  uint32_t uTotalRequestedCounters[SB_FE2000_NUM_CMU_MMU_INTERFACES];

  sal_memset(uTotalRequestedCounters, 0x0, sizeof(uTotalRequestedCounters));

  /* gma - Oct 09 2006 - Set CMU config options */
  SAND_HAL_WRITE(userDeviceHandle, CA, CM_CONFIG_BACKPRESSURE, uCmDmaThresh );
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, CM_CORE_RESET, 0x0);

  /* gma - Oct 30 2006 - Setup the segments */
  {
    uint32_t nCmuSegment = 0;
    uint32_t nConnectionType = 0;
    uint32_t nMmu = 0;

    uint32_t uType        = 0;
    uint32_t uBank        = 0;
    uint32_t uEject       = 0;
    uint32_t uBaseAddr    = 0;
    uint32_t uLimit       = 0;
    uint32_t uConsumedMem = 0;

    for( nCmuSegment = 0; nCmuSegment < SB_FE2000_MAX_CM_SEGMENTS; nCmuSegment++ ){
      if( pInitParams->cm.segment[nCmuSegment].bEnable ){
        uType     = pInitParams->cm.segment[nCmuSegment].counterType;
        uBank     = pInitParams->cm.segment[nCmuSegment].uBank;
        uEject    = pInitParams->cm.segment[nCmuSegment].uEject;
        uBaseAddr = pInitParams->cm.segment[nCmuSegment].uBaseAddr;
        uLimit    = pInitParams->cm.segment[nCmuSegment].uLimit;

        /* gma - Oct 30 2006 - a chained counter requires limit*2 */
        /* gma - Oct 30 2006 - a turbo counter which is ejected back into MMU requires limit*3 */
        if( uType == SB_FE2000_CM_CHAINED_LEGACY_COUNTER ){
          uConsumedMem = uLimit * 2;
        } else if(( uType == SB_FE2000_CM_TURBO_COUNTER ) && ( uEject == SB_FE2000_CM_SRAM_EJECT )){
          uConsumedMem = uLimit * 3;
        } else {
          uConsumedMem = uLimit;
        }

        uTotalRequestedCounters[uBank] += uConsumedMem;

        nMmu = ( uBank & 0x02 ) >> 1;
        if( uBank & 0x01 ){
          nConnectionType = pInitParams->mm[nMmu].ramconnections.Cmu1MemoryConnection;
        } else {
          nConnectionType = pInitParams->mm[nMmu].ramconnections.Cmu0MemoryConnection;
        }

        if( nConnectionType == SB_FE2000_MM_CMU1_CONNECTED_TO_INTERNAL_SINGLE_RAM1 ||
            nConnectionType == SB_FE2000_MM_CMU0_CONNECTED_TO_INTERNAL_SINGLE_RAM0 ){

          /* gma - Oct 30 2006 - Check to ensure we are not over subscribing the memory */
          if( uTotalRequestedCounters[uBank] <= SB_FE2000_CM_MM_INT_MEM_ADDR_LOCATIONS ){
            sbFe2000UtilSetupCmSegment( userDeviceHandle,nCmuSegment, uType, uBank, uEject, uBaseAddr, uLimit, uConsumedMem );

          } else {
            SB_ERROR("CMU: Total requested counters %d for bank %d, MMU_%d_Memory_%d, exceeds the capabilities of the internal memory. Not Setting up the segment.\n",
                     uTotalRequestedCounters[uBank], uBank, nMmu, ( uBank & 0x01 ));
          }

        } else if( nConnectionType == SB_FE2000_MM_CMU0_CONNECTED_TO_NARROWPORT_DDRII_PORT0 ||
                   nConnectionType == SB_FE2000_MM_CMU1_CONNECTED_TO_NARROWPORT_DDRII_PORT1 ){

          /* gma - Oct 30 2006 - Check to ensure we are not over subscribing the memory */
          if( uTotalRequestedCounters[uBank] <=  SB_FE2000_CM_MM_NARROW_MEM_ADDR_LOCATIONS ){
            sbFe2000UtilSetupCmSegment( userDeviceHandle,nCmuSegment, uType, uBank, uEject, uBaseAddr, uLimit, uConsumedMem );

          } else {
            SB_ERROR("CMU: Total requested counters %d for bank %d, MMU_%d_Memory_%d, exceeds the capabilities of the internal memory. Not Setting up the segment.\n",
                     uTotalRequestedCounters[uBank], uBank, nMmu, ( uBank & 0x01 ));
          }

        } else {
          SB_ERROR("CMU: Invalid connection type %d for MMU_%d_Memory_%d. Not setting up segment.\n", nConnectionType, nMmu, ( uBank & 0x01 ));
        }

        if( pInitParams->cm.segment[nCmuSegment].bEnableAutoFlush ){
          uint32_t uFlushCtrlOffset = 0;
          uint32_t uFlushRateOffset = 0;
          uint32_t uCmSegFlushStride;
          uint32_t uCmSegFlushRateStride;

          SB_ERROR("%s:Setting Auto Flush for Segment %d at Rate %d", __PRETTY_FUNCTION__,nCmuSegment, pInitParams->cm.segment[nCmuSegment].uAutoFlushRate );

          uCmSegFlushStride = SAND_HAL_REG_OFFSET( CA, CM_CONFIG_SEGMENT_FLUSH_CTRL0 ) - SAND_HAL_REG_OFFSET( CA, CM_CONFIG_SEGMENT_FLUSH_CTRL1 );
          uCmSegFlushRateStride = SAND_HAL_REG_OFFSET( CA, CM_CONFIG_SEGMENT_FLUSH_RATE0 ) - SAND_HAL_REG_OFFSET( CA, CM_CONFIG_SEGMENT_FLUSH_RATE1 );

          uFlushCtrlOffset = (nCmuSegment * uCmSegFlushStride) + SAND_HAL_CA_CM_CONFIG_SEGMENT_FLUSH_CTRL0_OFFSET;
          uFlushRateOffset  = (nCmuSegment * uCmSegFlushRateStride) + SAND_HAL_CA_CM_CONFIG_SEGMENT_FLUSH_RATE0_OFFSET;

          /* gma - Dec 01 2006 - Chose 398 here because lrp table lists 403 instructions per epoch @ 20G @ 250MHz */
          SAND_HAL_WRITE_OFFS( (uint32_t)userDeviceHandle, uFlushRateOffset,
                          SAND_HAL_SET_FIELD( CA, CM_CONFIG_SEGMENT_FLUSH_RATE0, RATE, pInitParams->cm.segment[nCmuSegment].uAutoFlushRate ));

          SAND_HAL_WRITE_OFFS( (uint32_t)userDeviceHandle, uFlushCtrlOffset,
                          SAND_HAL_SET_FIELD( CA, CM_CONFIG_SEGMENT_FLUSH_CTRL0, ENABLE, 0x1 ));

        }
      }
    }
  }

  SAND_HAL_WRITE(userDeviceHandle, CA, CM_CONFIG_AGE_THRESHOLD, SAND_HAL_SET_FIELD(CA, CM_CONFIG_AGE_THRESHOLD, THRESHOLD, 0xff ));
  SAND_HAL_WRITE(userDeviceHandle, CA, CM_CONFIG_DMA_FORCE_COMMIT, SAND_HAL_SET_FIELD(CA, CM_CONFIG_DMA_FORCE_COMMIT, COMMIT_TIMEOUT, 0x0 ));

  DEXIT();
}
static void
sbFe2000InitRc(uint32_t uInstance, sbhandle userDeviceHandle,
               sbFe2000InitParams_t *pInitParams)
{
  DENTER();
  if (uInstance == 0) {
    SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, RC0_CORE_RESET, 0x0);
  } else {
    SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, RC1_CORE_RESET, 0x0);
  }
  DEXIT();
}

static void
sbFe2000InitPm(sbhandle userDeviceHandle,
               sbFe2000InitParams_t *pInitParams)
{

  uint32_t uGroup;
  DENTER();
  for( uGroup = 0; uGroup < SB_FE2000_PM_TOTAL_NUM_GROUPS; uGroup++ ) {
    uint32_t uBaseRegAddress = SAND_HAL_REG_OFFSET(CA,PM_GROUP0_CONFIG0)+(16*uGroup);
    SAND_HAL_WRITE_OFFS( userDeviceHandle, uBaseRegAddress,
                         SAND_HAL_SET_FIELD( CA, PM_GROUP0_CONFIG0, ENABLE, pInitParams->pm.group[uGroup].bEnable ) |
                         SAND_HAL_SET_FIELD( CA, PM_GROUP0_CONFIG0, POLID_MIN, pInitParams->pm.group[uGroup].uPolicerIdMinimum ));
    SAND_HAL_WRITE_OFFS( userDeviceHandle, uBaseRegAddress+4,
                         SAND_HAL_SET_FIELD( CA, PM_GROUP0_CONFIG1, POLID_MAX, pInitParams->pm.group[uGroup].uPolicerIdMaximum ));
    /* rgf - Dec 01 2006 - Group 8 has only the enable and ID range, no refreshes, no timers */
    if( uGroup < 8 ) {
      SAND_HAL_WRITE_OFFS( userDeviceHandle, uBaseRegAddress+8,
                           SAND_HAL_SET_FIELD( CA, PM_GROUP0_CONFIG2, REFRESH_COUNT, pInitParams->pm.group[uGroup].uRefreshCount ) |
                           SAND_HAL_SET_FIELD( CA, PM_GROUP0_CONFIG2, REFRESH_THRESH, pInitParams->pm.group[uGroup].uRefreshThreshold ) |
                           SAND_HAL_SET_FIELD( CA, PM_GROUP0_CONFIG2, REFRESH_PERIOD, pInitParams->pm.group[uGroup].uRefreshPeriod ));
      SAND_HAL_WRITE_OFFS( userDeviceHandle, uBaseRegAddress+12,
                           SAND_HAL_SET_FIELD( CA, PM_GROUP0_CONFIG3, TIMER_ENABLE, pInitParams->pm.group[uGroup].bTimerEnable ) |
                           SAND_HAL_SET_FIELD( CA, PM_GROUP0_CONFIG3, TIMESTAMP_OFFSET, pInitParams->pm.group[uGroup].uTimestampOffset ) |
                           SAND_HAL_SET_FIELD( CA, PM_GROUP0_CONFIG3, TIMER_TICK_PERIOD, pInitParams->pm.group[uGroup].uTimerPeriod ));
    }

  }

  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, PM_CORE_RESET, 0x0);

  /* kpl - setup a default policer config here? Or done at application level.. */

  {
    uint32_t uPmConfig = SAND_HAL_READ(userDeviceHandle, CA, PM_CONFIG0);
    uPmConfig = SAND_HAL_MOD_FIELD( CA, PM_CONFIG0, MM_SELECT_BIT, uPmConfig, pInitParams->pm.uMmuSelectBit );
    uPmConfig = SAND_HAL_MOD_FIELD( CA, PM_CONFIG0, REFRESH_ENABLE, uPmConfig, pInitParams->pm.bBackgroundRefreshEnable);
    uPmConfig = SAND_HAL_MOD_FIELD( CA, PM_CONFIG0, TOTAL_REFRESH_THRESH, uPmConfig, pInitParams->pm.uTotalRefreshThreshold );
    SAND_HAL_WRITE(userDeviceHandle, CA,  PM_CONFIG0, uPmConfig);
  }

  DEXIT();
}

static uint32_t
sbFe2000InitLr(sbhandle userDeviceHandle,
               sbFe2000InitParams_t *pInitParams)
{
    uint32_t uConfig0;
    uint32_t uConfig1;
    uint32_t uStreamIndex;
    uint32_t uStreamOnline = 0;
    uint32_t nRequestThreshold;
    int i, j;
    int nLrImemSize = SAND_HAL_CA_LR_IMEM_ACC_CTRL_ADDRESS_MASK + 1;
#ifdef PLISIM
    nLrImemSize = SB_FE2000_LR_NUMBER_OF_STREAMS * SB_FE2000_LR_NUMBER_OF_INSTRS;
#endif

    DENTER();

    SB_ASSERT(!SAND_HAL_IS_FE2KXT((int)userDeviceHandle));

    SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, LR_CORE_RESET,
                       0x0);

    SAND_HAL_WRITE(userDeviceHandle, CA,  LR_IMEM_ACC_DATA0, 0);
    SAND_HAL_WRITE(userDeviceHandle, CA,  LR_IMEM_ACC_DATA1, 0);
    SAND_HAL_WRITE(userDeviceHandle, CA,  LR_IMEM_ACC_DATA2, 0);
    for (i = 0; i < nLrImemSize; i++) {
        SAND_HAL_WRITE(userDeviceHandle, CA,  LR_IMEM_ACC_CTRL,
                       (SAND_HAL_CA_LR_IMEM_ACC_CTRL_REQ_MASK
                        | SAND_HAL_CA_LR_IMEM_ACC_CTRL_ACK_MASK
                        | i));
        for (j = 0; j < 100; j++) {
            if (SAND_HAL_READ(userDeviceHandle, CA, LR_IMEM_ACC_CTRL)
                & SAND_HAL_CA_LR_IMEM_ACC_CTRL_ACK_MASK) {
                break;
            }
        }
        if (j == 100) {
            return SB_FE_MEM_ACC_TIMEOUT_ERR_CODE;
        }
    }

    uConfig0 = SAND_HAL_READ(userDeviceHandle, CA, LR_CONFIG0);

    uConfig0 = SAND_HAL_MOD_FIELD( CA, LR_CONFIG0, ENABLE, uConfig0, TRUE );

    /* A quote from Nick as to why the hw adds +10 to the programmed epoch!
     * "it takes time to propogate "doneness" from each PE back to the
     * instruction controller and then the instruction controller has to
     * recharge its instruction pipeline to get back to the beginning of
     * the epoch..."
     */
    uConfig0 = SAND_HAL_MOD_FIELD( CA, LR_CONFIG0, EPOCH, uConfig0,
                                   (pInitParams->lr.uEpochLengthInCycles - 11) );
    uConfig0 = SAND_HAL_MOD_FIELD( CA, LR_CONFIG0, CONTEXTS, uConfig0,
                                   pInitParams->lr.uNumberOfContexts );
    uConfig0 = SAND_HAL_MOD_FIELD( CA, LR_CONFIG0, PAIRED, uConfig0,
                                   pInitParams->lr.bPairedMode);

    if( pInitParams->lr.bLrpBypass == TRUE ) {
        uConfig0 = SAND_HAL_MOD_FIELD( CA, LR_CONFIG0, BYPASS, uConfig0, 1 );
    } else {
        uConfig0 = SAND_HAL_MOD_FIELD( CA, LR_CONFIG0, BYPASS, uConfig0, 0 );
        if ( pInitParams->lr.bHeaderProcessingMode == TRUE) {
            uConfig0 = SAND_HAL_MOD_FIELD( CA, LR_CONFIG0, LOAD_ENABLE,
                                           uConfig0, 1 );
        }
        for( uStreamIndex = 0; uStreamIndex < SB_FE2000_LR_NUMBER_OF_STREAMS;
             uStreamIndex++ ) {
            if ( pInitParams->lr.bStreamOnline[uStreamIndex] == TRUE ) {
                uStreamOnline |= (1 << uStreamIndex);
            }
        }
        uConfig0 = SAND_HAL_MOD_FIELD( CA, LR_CONFIG0, ONLINE, uConfig0,
                                       uStreamOnline );
#undef UPDATE
        uConfig0 = SAND_HAL_MOD_FIELD( CA, LR_CONFIG0, UPDATE, uConfig0, 1 );
    }

    /* dgm - Feb 12 2007 - Needed for lrp timer utility (OAM) --
     * value taken from register description */
    SAND_HAL_WRITE(userDeviceHandle, CA, LU_CLOCKS_PER_US, 17179869);

    uConfig1 = SAND_HAL_READ(userDeviceHandle, CA, LR_CONFIG1);
    uConfig1 = SAND_HAL_MOD_FIELD(CA, LR_CONFIG1, PPE_PACE, uConfig1,
                                  (pInitParams->lr.uPpeRequestPaceInCycles-1));
    uConfig1 = SAND_HAL_MOD_FIELD(CA, LR_CONFIG1, FRAMES_PER_CONTEXT,
                                  uConfig1, pInitParams->lr.uFramesPerContext);
    uConfig1 = SAND_HAL_MOD_FIELD(CA, LR_CONFIG1, LOAD_COUNT, uConfig1,
                                  pInitParams->lr.uLoadCount);
    uConfig1 = SAND_HAL_MOD_FIELD(CA, LR_CONFIG1, WORDS0, uConfig1,
                                  pInitParams->lr.uWords0 );
    uConfig1 = SAND_HAL_MOD_FIELD(CA, LR_CONFIG1, WORDS1, uConfig1,
                                  pInitParams->lr.uWords1 );
    nRequestThreshold =(1024/
                        (pInitParams->lr.uWords0 > pInitParams->lr.uWords1
                         ? pInitParams->lr.uWords0 : pInitParams->lr.uWords1));

    if (pInitParams->lr.bBringUp) {
        SAND_HAL_RMW_FIELD(userDeviceHandle, CA, LR_CONFIG2, RQ_THRESHOLD,
                           nRequestThreshold);
        SAND_HAL_WRITE(userDeviceHandle, CA,  LR_CONFIG1, uConfig1);
        SAND_HAL_WRITE(userDeviceHandle, CA,  LR_CONFIG0, uConfig0);
    }

    DEXIT();

    return SB_FE2000_STS_INIT_OK_K;
}

static void
sbFe2000TrainDdr(sbhandle userDeviceHandle,
                 sbFe2000InitParams_t *pInitParams)
{
    if(pInitParams->trainDdr) {
        pInitParams->trainDdr(userDeviceHandle);
    }
}

static void
sbFe2000EnableSramDll(sbhandle userDeviceHandle,
                      sbFe2000InitParams_t *pInitParams)
{
    int i, j;
    uint32_t uData;
    uint32_t uDllPhase;
    uint8_t bRevA0Chip;

    SB_ASSERT(!SAND_HAL_IS_FE2KXT((int)userDeviceHandle));

    uData = SAND_HAL_READ(userDeviceHandle, CA, PC_REVISION);
    bRevA0Chip = SAND_HAL_GET_FIELD(CA, PC_REVISION, REVISION, uData)==0;
    SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, MM0_CORE_RESET,
                       0x0);
    SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, MM1_CORE_RESET,
                       0x0);

    SB_LOGV1("Set ODT for MMUs to 75 ohms\n");
    uData = 0;
    uData = SAND_HAL_MOD_FIELD(CA,MM_DDR_TERMINATION, WIDE_PORT_RT75, uData, 1);        /* 0:150 ohm, 1: 75 ohm */
    uData = SAND_HAL_MOD_FIELD(CA,MM_DDR_TERMINATION, WIDE_PORT_DATA_REB, uData, 0);    /* 0 ODT on,  1: ODT off */
    uData = SAND_HAL_MOD_FIELD(CA,MM_DDR_TERMINATION, WIDE_PORT_CQ_REB, uData, 0);      /* 0 ODT on,  1: ODT off */
    uData = SAND_HAL_MOD_FIELD(CA,MM_DDR_TERMINATION, NARROW_PORT1_RT75, uData, 1);     /* 0:150 ohm, 1: 75 ohm */
    uData = SAND_HAL_MOD_FIELD(CA,MM_DDR_TERMINATION, NARROW_PORT1_DATA_REB, uData, 0); /* 0 ODT on,  1: ODT off */
    uData = SAND_HAL_MOD_FIELD(CA,MM_DDR_TERMINATION, NARROW_PORT1_CQ_REB, uData, 0);   /* 0 ODT on,  1: ODT off */
    uData = SAND_HAL_MOD_FIELD(CA,MM_DDR_TERMINATION, NARROW_PORT0_RT75, uData, 1);     /* 0:150 ohm, 1: 75 ohm */
    uData = SAND_HAL_MOD_FIELD(CA,MM_DDR_TERMINATION, NARROW_PORT0_DATA_REB, uData, 0); /* 0 ODT on,  1: ODT off */
    uData = SAND_HAL_MOD_FIELD(CA,MM_DDR_TERMINATION, NARROW_PORT0_CQ_REB, uData, 0);   /* 0 ODT on,  1: ODT off */
    SAND_HAL_WRITE(userDeviceHandle, CA, MM0_DDR_TERMINATION, uData);
    SAND_HAL_WRITE(userDeviceHandle, CA, MM1_DDR_TERMINATION, uData);

   uData = 0;
   /* dwl - Jul 26 2007 - For non RevA0 parts, the default value should be TBD
    *                   - Use 1 for now, need more characterization
    *                   - May also need to add training routine
    */
   uDllPhase = bRevA0Chip ? 0 : 1;
   uData = SAND_HAL_MOD_FIELD(CA, MM0_DLL_DEBUG, WIDE_PORT_DLL_PHASE,
                              uData, uDllPhase);
   uData = SAND_HAL_MOD_FIELD(CA, MM0_DLL_DEBUG, NARROW_PORT1_DLL_PHASE,
                              uData, uDllPhase);
   uData = SAND_HAL_MOD_FIELD(CA, MM0_DLL_DEBUG, NARROW_PORT0_DLL_PHASE,
                              uData, uDllPhase);
   SAND_HAL_WRITE(userDeviceHandle, CA, MM0_DLL_DEBUG, uData);
   SAND_HAL_WRITE(userDeviceHandle, CA, MM1_DLL_DEBUG, uData);

   for (i = 0; i < 2; i++) {
       for (j = 0; j < 5; j++) {
           if (!sbFe2000UtilPvtCompensation(userDeviceHandle,i)) break;
           SB_LOG("PVT Compensation iteration#%d finished with errors,"
                  " try again.\n", j);

       }
   }

   if (pInitParams->resetDll && bRevA0Chip) {
       (pInitParams->resetDll)(userDeviceHandle);
   }
}

static void
sbFe2000InitMm(uint32_t uInstance, sbhandle userDeviceHandle,
               sbFe2000InitParams_t *pInitParams)
{

  sbFe2000InitParamsMm_t *pInstance = &(pInitParams->mm[uInstance]);
  uint32_t uNumInternalMem0;
  uint32_t uNumInternalMem1;
  uint32_t uMmClientConfig;
  uint32_t uMmProtectionScheme0;
  uint32_t uMmProtectionScheme1;
  uint32_t uMmStatus = 0;
  uint32_t uWidePortDllLock = 0;
  uint32_t uNarrowPort0DllLock = 0;
  uint32_t uNarrowPort1DllLock = 0;
  uint32_t uDllLockTimeOut = 0;
  uint8_t  bDllAchievedLock = FALSE;
  uint32_t uData = 0;
  uint32_t uMmRamConfiguration;


  DENTER();

  SB_ASSERT(!SAND_HAL_IS_FE2KXT((int)userDeviceHandle));

   /* step1: configure the memory configuration */
  uMmRamConfiguration = SAND_HAL_READ_STRIDE(userDeviceHandle, CA,MM,uInstance,MM_RAM_CONFIG);

  /* step1: configure the memory configuration */
  uMmRamConfiguration = SAND_HAL_READ_STRIDE(userDeviceHandle, CA,MM,uInstance,MM_RAM_CONFIG);
  uMmRamConfiguration = SAND_HAL_MOD_FIELD(CA,MM_RAM_CONFIG,INTERNAL_RAM_CONFIG,uMmRamConfiguration,pInstance->ramconfiguration.InternalRamConfiguration);

  uMmRamConfiguration = SAND_HAL_MOD_FIELD(CA,MM_RAM_CONFIG,NARROW_PORT0_RAM_CONFIG,uMmRamConfiguration,pInstance->ramconfiguration.NarrowPortRam0Configuration);
  uMmRamConfiguration = SAND_HAL_MOD_FIELD(CA,MM_RAM_CONFIG,NARROW_PORT1_RAM_CONFIG,uMmRamConfiguration,pInstance->ramconfiguration.NarrowPortRam1Configuration);
  uMmRamConfiguration = SAND_HAL_MOD_FIELD(CA,MM_RAM_CONFIG,WIDE_PORT_RAM_CONFIG,uMmRamConfiguration,pInstance->ramconfiguration.WidePortRamConfiguration);

  if(pInstance->ramconfiguration.NarrowPortRam0Configuration == SB_FE2000_MM_ONE_BY_18_RAM_CONNECTED) {
    uMmRamConfiguration = SAND_HAL_MOD_FIELD(CA,MM_RAM_CONFIG,NARROW_PORT0_ADDRESS_SHIFT,uMmRamConfiguration,1);
  } else {
    uMmRamConfiguration = SAND_HAL_MOD_FIELD(CA,MM_RAM_CONFIG,NARROW_PORT0_ADDRESS_SHIFT,uMmRamConfiguration,0);
  }

  if(pInstance->ramconfiguration.NarrowPortRam1Configuration == SB_FE2000_MM_ONE_BY_18_RAM_CONNECTED) {
    uMmRamConfiguration = SAND_HAL_MOD_FIELD(CA,MM_RAM_CONFIG,NARROW_PORT1_ADDRESS_SHIFT,uMmRamConfiguration,1);
  } else {
    uMmRamConfiguration = SAND_HAL_MOD_FIELD(CA,MM_RAM_CONFIG,NARROW_PORT1_ADDRESS_SHIFT,uMmRamConfiguration,0);
  }

  if(pInstance->ramconfiguration.WidePortRamConfiguration == SB_FE2000_MM_TWO_BY_9_RAM_AND_TOLOWER_UPPER_DATAPIN_NOCONNECTION) {
    uMmRamConfiguration = SAND_HAL_MOD_FIELD(CA,MM_RAM_CONFIG,WIDE_PORT_ADDRESS_SHIFT,uMmRamConfiguration,0);
  } else {
    uMmRamConfiguration = SAND_HAL_MOD_FIELD(CA,MM_RAM_CONFIG,WIDE_PORT_ADDRESS_SHIFT,uMmRamConfiguration,1);
  }

  /* dwl - May 11 2007 - For lab run, Steve McLafferty recommends to set the XXX_use_delayed_dvld bits */
  /*                   - based on trace length analysis of our boards */

  uMmRamConfiguration = SAND_HAL_MOD_FIELD(CA,MM0_RAM_CONFIG,NARROW_PORT1_USE_DELAYED_DVLD,uMmRamConfiguration,1);
  uMmRamConfiguration = SAND_HAL_MOD_FIELD(CA,MM0_RAM_CONFIG,NARROW_PORT0_USE_DELAYED_DVLD,uMmRamConfiguration,1);
  uMmRamConfiguration = SAND_HAL_MOD_FIELD(CA,MM0_RAM_CONFIG,WIDE_PORT_USE_DELAYED_DVLD,uMmRamConfiguration,1);

  SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA,MM,uInstance, MM_RAM_CONFIG,uMmRamConfiguration);

  /* gma - Nov 03 2006 - Internal memories can only be allocated to one block.  Check for that here. */
  uNumInternalMem0 = ( pInstance->ramconnections.Lrp2MemoryConnection == SB_FE2000_MM_LRP2_CONNECTED_TO_INTERNAL_SINGLE_RAM0 ) ? 1 : 0;
  uNumInternalMem0 += ( pInstance->ramconnections.Cmu0MemoryConnection == SB_FE2000_MM_CMU0_CONNECTED_TO_INTERNAL_SINGLE_RAM0 ) ? 1 : 0;
  uNumInternalMem0 += ( pInstance->ramconnections.PmuMemoryConnection  == SB_FE2000_MM_PMU_CONNECTED_TO_INTERNAL_SINGLE_RAM0  ) ? 1 : 0;

  uNumInternalMem1 = ( pInstance->ramconnections.Lrp3MemoryConnection == SB_FE2000_MM_LRP3_CONNECTED_TO_INTERNAL_SINGLE_RAM1 ) ? 1 : 0;
  uNumInternalMem1 += ( pInstance->ramconnections.Cmu1MemoryConnection == SB_FE2000_MM_CMU1_CONNECTED_TO_INTERNAL_SINGLE_RAM1 ) ? 1 : 0;
  uNumInternalMem1 += ( pInstance->ramconnections.PmuMemoryConnection  == SB_FE2000_MM_PMU_CONNECTED_TO_INTERNAL_SINGLE_RAM1  ) ? 1 : 0;

  if( uNumInternalMem0 > 1 || uNumInternalMem1 > 1 ){
    SB_LOG("Conflict on the allocation of Mmu internal memories between"
           " LRP/CMU/PMU blocks.\n");
    SB_ASSERT( 0 );
  }

  /* step 2: configure the client memory connections */
  uMmClientConfig = SAND_HAL_READ_STRIDE(userDeviceHandle, CA,MM,uInstance,MM_CLIENT_CONFIG);

  uMmClientConfig = SAND_HAL_MOD_FIELD(CA,MM_CLIENT_CONFIG, PMU_PRIMARY_CLIENT, uMmClientConfig,pInstance->ramconnections.bPmuPrimaryClientOnWideport);
  uMmClientConfig = SAND_HAL_MOD_FIELD(CA,MM_CLIENT_CONFIG, LRP0_MEMORY_CONFIG, uMmClientConfig,pInstance->ramconnections.Lrp0MemoryConnection);
  uMmClientConfig = SAND_HAL_MOD_FIELD(CA,MM_CLIENT_CONFIG, LRP1_MEMORY_CONFIG, uMmClientConfig,pInstance->ramconnections.Lrp1MemoryConnection);
  uMmClientConfig = SAND_HAL_MOD_FIELD(CA,MM_CLIENT_CONFIG, LRP2_MEMORY_CONFIG, uMmClientConfig,pInstance->ramconnections.Lrp2MemoryConnection);
  uMmClientConfig = SAND_HAL_MOD_FIELD(CA,MM_CLIENT_CONFIG, LRP3_MEMORY_CONFIG, uMmClientConfig,pInstance->ramconnections.Lrp3MemoryConnection);
  uMmClientConfig = SAND_HAL_MOD_FIELD(CA,MM_CLIENT_CONFIG, LRP4_MEMORY_CONFIG, uMmClientConfig,pInstance->ramconnections.Lrp4MemoryConnection);
  uMmClientConfig = SAND_HAL_MOD_FIELD(CA,MM_CLIENT_CONFIG,  PMU_MEMORY_CONFIG, uMmClientConfig,pInstance->ramconnections.PmuMemoryConnection);
  uMmClientConfig = SAND_HAL_MOD_FIELD(CA,MM_CLIENT_CONFIG, CMU0_MEMORY_CONFIG, uMmClientConfig,pInstance->ramconnections.Cmu0MemoryConnection);
  uMmClientConfig = SAND_HAL_MOD_FIELD(CA,MM_CLIENT_CONFIG, CMU1_MEMORY_CONFIG, uMmClientConfig,pInstance->ramconnections.Cmu1MemoryConnection);
  /* jts - Oct 16 2006 - removed reference to LUE client setup (these were removed from .trex2) */

  SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA,MM,uInstance, MM_CLIENT_CONFIG,uMmClientConfig);

  /* gma - Nov 03 2006 - step 3: configure the ram protection scheme0 */
  uMmProtectionScheme0 = SAND_HAL_READ_STRIDE(userDeviceHandle, CA, MM, uInstance, MM_PROT_SCHEME0 );

  uMmProtectionScheme0 = SAND_HAL_MOD_FIELD( CA, MM_PROT_SCHEME0, NP0_PRI_SCHEME, uMmProtectionScheme0, pInstance->ramprotection.NarrowPort0PrimaryClient );
  uMmProtectionScheme0 = SAND_HAL_MOD_FIELD( CA, MM_PROT_SCHEME0, NP0_SEC_SCHEME, uMmProtectionScheme0, pInstance->ramprotection.NarrowPort0SecondaryClient );
  uMmProtectionScheme0 = SAND_HAL_MOD_FIELD( CA, MM_PROT_SCHEME0, NP0_DMA_SCHEME, uMmProtectionScheme0, pInstance->ramprotection.NarrowPort0DmaAccess );
  uMmProtectionScheme0 = SAND_HAL_MOD_FIELD( CA, MM_PROT_SCHEME0, NP0_PI_SCHEME,  uMmProtectionScheme0, pInstance->ramprotection.NarrowPort0ProcessorInterface );

  uMmProtectionScheme0 = SAND_HAL_MOD_FIELD( CA, MM_PROT_SCHEME0, NP1_PRI_SCHEME, uMmProtectionScheme0, pInstance->ramprotection.NarrowPort1PrimaryClient );
  uMmProtectionScheme0 = SAND_HAL_MOD_FIELD( CA, MM_PROT_SCHEME0, NP1_SEC_SCHEME, uMmProtectionScheme0, pInstance->ramprotection.NarrowPort1SecondaryClient );
  uMmProtectionScheme0 = SAND_HAL_MOD_FIELD( CA, MM_PROT_SCHEME0, NP1_DMA_SCHEME, uMmProtectionScheme0, pInstance->ramprotection.NarrowPort1DmaAccess );
  uMmProtectionScheme0 = SAND_HAL_MOD_FIELD( CA, MM_PROT_SCHEME0, NP1_PI_SCHEME,  uMmProtectionScheme0, pInstance->ramprotection.NarrowPort1ProcessorInterface );

  SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA, MM, uInstance, MM_PROT_SCHEME0, uMmProtectionScheme0 );

  /* gma - Nov 03 2006 - step 3: configure the ram protection scheme1 */
  uMmProtectionScheme1 = SAND_HAL_READ_STRIDE( userDeviceHandle, CA, MM, uInstance, MM_PROT_SCHEME1 );

  uMmProtectionScheme1 = SAND_HAL_MOD_FIELD( CA, MM_PROT_SCHEME1, INT0_SCHEME, uMmProtectionScheme1, pInstance->ramprotection.InternalRam0ProtectionScheme );
  uMmProtectionScheme1 = SAND_HAL_MOD_FIELD( CA, MM_PROT_SCHEME1, INT1_SCHEME, uMmProtectionScheme1, pInstance->ramprotection.InternalRam1ProtectionScheme );
  uMmProtectionScheme1 = SAND_HAL_MOD_FIELD( CA, MM_PROT_SCHEME1, WP_SCHEME,   uMmProtectionScheme1, pInstance->ramprotection.WidePortRamProtectionScheme );

  SAND_HAL_WRITE_STRIDE( userDeviceHandle, CA, MM, uInstance, MM_PROT_SCHEME1, uMmProtectionScheme1 );

  /* gma - Nov 03 2006 - Need to wait for the dlls to lock prior to enabling the narrow and wide port memories. */

  /* kpl forced to TRUE for simulation only */
  if (g_bSimRun) {
    bDllAchievedLock = TRUE;
  }

  /* gma - Nov 08 2006 - If there exists a client which is connected to the Wide Port memory then need */
  /* gma - Nov 08 2006 - to check for dll lock and once lock is achieved need to enable the memory. */
  if( pInstance->ramconnections.PmuMemoryConnection == SB_FE2000_MM_PMU_CONNECTED_TO_WIDEPORT_DDRII ||
      pInstance->ramconnections.Lrp4MemoryConnection == SB_FE2000_MM_LRP4_CONNECTED_TO_WIDEPORT_DDRII_PORT ){

    do{
      uDllLockTimeOut++;
      uMmStatus = SAND_HAL_READ_STRIDE(userDeviceHandle, CA, MM, uInstance, MM_STATUS );
      uWidePortDllLock = SAND_HAL_GET_FIELD(CA, MM_STATUS, WIDE_PORT_DLL_LOCK, uMmStatus );

      if( uWidePortDllLock == SB_FE2000_MM_WIDE_PORT_DLL_LOCK_DOUBLE_RAM ){
        bDllAchievedLock = TRUE;
      }

    }while(( uDllLockTimeOut < 200 ) && ( bDllAchievedLock == FALSE ));

    if( bDllAchievedLock ){
      SB_LOGV3("%s:Enabling the MMU_%d Wide Port Memory.\n", __PRETTY_FUNCTION__,uInstance );
      uData = SAND_HAL_READ_STRIDE(userDeviceHandle, CA,MM,uInstance,MM_RAM_CONFIG);
      uData = SAND_HAL_MOD_FIELD(CA,MM_RAM_CONFIG,ENABLE_WIDE_PORT,uData,0x1);
      SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA,MM,uInstance,MM_RAM_CONFIG,uData);
    } else {
      SB_LOGV1(" ** WARNING ** %s:Enabling the MMU_%d Wide Port Memory with DLL Lock Timeout\n", __PRETTY_FUNCTION__,uInstance );
      uData = SAND_HAL_READ_STRIDE(userDeviceHandle, CA,MM,uInstance,MM_RAM_CONFIG);
      uData = SAND_HAL_MOD_FIELD(CA,MM_RAM_CONFIG,ENABLE_WIDE_PORT,uData,0x1);
      SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA,MM,uInstance,MM_RAM_CONFIG,uData);
    }
  }

  /* gma - Nov 08 2006 - If there exists a client which is connected to the Narrow Port 0 memory then need */
  /* gma - Nov 08 2006 - to check for dll lock and once lock is achieved need to enable the memory. */
  if( pInstance->ramconnections.Cmu0MemoryConnection == SB_FE2000_MM_CMU0_CONNECTED_TO_NARROWPORT_DDRII_PORT0 ||
      pInstance->ramconnections.Lrp0MemoryConnection == SB_FE2000_MM_LRP0_CONNECTED_TO_NARROWPORT_DDRII_PORT0 ){

    uDllLockTimeOut = 0;
    bDllAchievedLock = FALSE;
    /* kpl forced to TRUE for simulation only */
    if (g_bSimRun) {
      bDllAchievedLock = TRUE;
    }

    do{
      uDllLockTimeOut++;
      uMmStatus = SAND_HAL_READ_STRIDE(userDeviceHandle, CA, MM, uInstance, MM_STATUS );
      uNarrowPort0DllLock = SAND_HAL_GET_FIELD(CA, MM_STATUS, NARROW_PORT0_DLL_LOCK, uMmStatus );

      if( uNarrowPort0DllLock == SB_FE2000_MM_NARROW_PORT_DLL_LOCK_DOUBLE_RAM ){
        bDllAchievedLock = TRUE;
      }

    }while(( uDllLockTimeOut < 200 ) && ( bDllAchievedLock == FALSE ));

    if( bDllAchievedLock ){
      SB_LOGV3("%s:Enabling the MMU_%d Narrow Port 0 Memory.\n", __PRETTY_FUNCTION__,uInstance );
      uData = SAND_HAL_READ_STRIDE(userDeviceHandle, CA,MM,uInstance,MM_RAM_CONFIG);
      uData = SAND_HAL_MOD_FIELD(CA,MM_RAM_CONFIG,ENABLE_NARROW_PORT0,uData,0x1);
      SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA,MM,uInstance,MM_RAM_CONFIG,uData);
    } else {
      SB_LOGV1(" ** WARNING ** %s:Enabling the MMU_%d Narrow Port 0 Memory with DLL Lock Timeout\n", __PRETTY_FUNCTION__,uInstance );
      uData = SAND_HAL_READ_STRIDE(userDeviceHandle, CA,MM,uInstance,MM_RAM_CONFIG);
      uData = SAND_HAL_MOD_FIELD(CA,MM_RAM_CONFIG,ENABLE_NARROW_PORT0,uData,0x1);
      SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA,MM,uInstance,MM_RAM_CONFIG,uData);
    }
  }

  /* gma - Nov 08 2006 - If there exists a client which is connected to the Narrow Port 1 memory then need */
  /* gma - Nov 08 2006 - to check for dll lock and once lock is achieved need to enable the memory. */
  if( pInstance->ramconnections.Cmu1MemoryConnection == SB_FE2000_MM_CMU1_CONNECTED_TO_NARROWPORT_DDRII_PORT1 ||
      pInstance->ramconnections.Lrp1MemoryConnection == SB_FE2000_MM_LRP1_CONNECTED_TO_NARROWPORT_DDRII_PORT1 ){

    uDllLockTimeOut = 0;
    bDllAchievedLock = FALSE;
    /* kpl forced to TRUE for simulation only */
    if (g_bSimRun) {
      bDllAchievedLock = TRUE;
    }

    do{
      uDllLockTimeOut++;
      uMmStatus = SAND_HAL_READ_STRIDE(userDeviceHandle, CA, MM, uInstance, MM_STATUS );
      uNarrowPort1DllLock = SAND_HAL_GET_FIELD(CA, MM_STATUS, NARROW_PORT1_DLL_LOCK, uMmStatus );

      if( uNarrowPort1DllLock == SB_FE2000_MM_NARROW_PORT_DLL_LOCK_DOUBLE_RAM ){
        bDllAchievedLock = TRUE;
      }

    }while(( uDllLockTimeOut < 200 ) && ( bDllAchievedLock == FALSE ));
    if( bDllAchievedLock ){
      SB_LOGV3("%s Enabling the MMU_%d Narrow Port 1 Memory.\n", __PRETTY_FUNCTION__, uInstance );
      uData = SAND_HAL_READ_STRIDE(userDeviceHandle, CA,MM,uInstance,MM_RAM_CONFIG);
      uData = SAND_HAL_MOD_FIELD(CA,MM_RAM_CONFIG,ENABLE_NARROW_PORT1,uData,0x1);
      SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA,MM,uInstance,MM_RAM_CONFIG,uData);
    } else {
      SB_LOGV1(" ** WARNING ** %s Enabling the MMU_%d Narrow Port 1 Memory with DLL Lock Timeout\n", __PRETTY_FUNCTION__, uInstance );
      uData = SAND_HAL_READ_STRIDE(userDeviceHandle, CA,MM,uInstance,MM_RAM_CONFIG);
      uData = SAND_HAL_MOD_FIELD(CA,MM_RAM_CONFIG,ENABLE_NARROW_PORT1,uData,0x1);
      SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA,MM,uInstance,MM_RAM_CONFIG,uData);
    }
  }

    DEXIT();
}

static uint32_t
sbFe2000InitPt(sbhandle userDeviceHandle,
               sbFe2000InitParams_t *pInitParams)
{
    int i;
  DENTER();

  SB_ASSERT(!SAND_HAL_IS_FE2KXT((int)userDeviceHandle));

  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, PT_CORE_RESET, 0x0);

  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_CONFIG0, PTE_PRIORITY,
                     (uint32_t)pInitParams->pt.PipelinePriority);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_CONFIG1, EXPANDED_FIFO_SIZE,
                     pInitParams->pt.uExpandedFifoSize);

  if (pInitParams->sr[0].bBringUp) {
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE0_CONFIG, INIT, 1);
      if( sbFe2000InitRegDone(userDeviceHandle,
                              SAND_HAL_REG_OFFSET(CA, PT_PTE0_CONFIG),
                              SAND_HAL_FIELD_MASK(CA, PT_PTE0_CONFIG, INIT_DONE),
                              SB_FE2000_PT_INIT_TIMEOUT) ) {

          SB_ERROR("%s:Waiting for PT (SPI0) init_done timed out..\n",
                   __PRETTY_FUNCTION__);
          return SB_FE2000_STS_INIT_PT_SPI0_TIMEOUT_ERR_K;
      }

      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE0_CONFIG, INIT, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE0_CONFIG, INIT_DONE, 0);

      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE0_CONFIG, PTE_ENABLE, 1);
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PT_PTE0_ENABLE_CONFIG0,
                         PORT_ENABLE,0xffffffff);
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PT_PTE0_ENABLE_CONFIG1,
                         PORT_ENABLE,0xffffffff);
  }
  for (i = 0; i < SB_FE2000_MAX_PORTS_PER_SPI; i++) {
      CaPtPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_SPI0, i, 0xff);
  }

  if (pInitParams->sr[1].bBringUp) {
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE1_CONFIG, INIT, 1);
      if( sbFe2000InitRegDone(userDeviceHandle,
                              SAND_HAL_REG_OFFSET(CA, PT_PTE1_CONFIG),
                              SAND_HAL_FIELD_MASK(CA, PT_PTE1_CONFIG, INIT_DONE),
                              SB_FE2000_PT_INIT_TIMEOUT) ) {

          SB_ERROR("%s:Waiting for PT (SPI1) init_done timed out..\n",
                   __PRETTY_FUNCTION__);
          return SB_FE2000_STS_INIT_PT_SPI1_TIMEOUT_ERR_K;
      }

      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE1_CONFIG, INIT, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE1_CONFIG, INIT_DONE, 0);

      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE1_CONFIG, PTE_ENABLE, 1);
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PT_PTE1_ENABLE_CONFIG0,
                         PORT_ENABLE,0xffffffff);
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PT_PTE1_ENABLE_CONFIG1,
                         PORT_ENABLE,0xffffffff);
  }
  for (i = 0; i < SB_FE2000_MAX_PORTS_PER_SPI; i++) {
      CaPtPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_SPI1, i, 0xff);
  }

  if (pInitParams->ag[0].bBringUp) {
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE2_CONFIG, INIT, 1);
      if( sbFe2000InitRegDone(userDeviceHandle,
                              SAND_HAL_REG_OFFSET(CA, PT_PTE2_CONFIG),
                              SAND_HAL_FIELD_MASK(CA, PT_PTE2_CONFIG, INIT_DONE),
                              SB_FE2000_PT_INIT_TIMEOUT) ) {

          SB_ERROR("%s:Waiting for PT (AG0) init_done timed out..\n",
                   __PRETTY_FUNCTION__);
          return SB_FE2000_STS_INIT_PT_AG0_TIMEOUT_ERR_K;
      }

      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE2_CONFIG, INIT, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE2_CONFIG, INIT_DONE, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE2_CONFIG, PTE_ENABLE, 1);

      SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PT_PTE2_ENABLE_CONFIG,PORT_ENABLE,
                         0xfff);
  }
  for (i = 0; i < SB_FE2000_MAX_AG_PORTS; i++) {
      CaPtPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_AGM0, i, 0xff);
  }

  if (pInitParams->ag[1].bBringUp) {
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE3_CONFIG, INIT, 1);
      if( sbFe2000InitRegDone(userDeviceHandle,
                              SAND_HAL_REG_OFFSET(CA, PT_PTE3_CONFIG),
                              SAND_HAL_FIELD_MASK(CA, PT_PTE3_CONFIG, INIT_DONE),
                              SB_FE2000_PT_INIT_TIMEOUT) ) {

          SB_ERROR("%s:Waiting for PT (AG1) init_done timed out..\n",
                   __PRETTY_FUNCTION__);
          return SB_FE2000_STS_INIT_PT_AG1_TIMEOUT_ERR_K;
      }

      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE3_CONFIG, INIT, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE3_CONFIG, INIT_DONE, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE3_CONFIG, PTE_ENABLE, 1);

      SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PT_PTE3_ENABLE_CONFIG,PORT_ENABLE,
                         0xfff);
  }
  for (i = 0; i < SB_FE2000_MAX_AG_PORTS; i++) {
      CaPtPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_AGM1, i, 0xff);
  }


  if (pInitParams->xg[0].bBringUp) {
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE4_CONFIG, INIT, 1);
      if( sbFe2000InitRegDone(userDeviceHandle,
                              SAND_HAL_REG_OFFSET(CA, PT_PTE4_CONFIG),
                              SAND_HAL_FIELD_MASK(CA, PT_PTE4_CONFIG, INIT_DONE),
                              SB_FE2000_PT_INIT_TIMEOUT) ) {

          SB_ERROR("%s:Waiting for PT (XG0) init_done timed out..\n",
                   __PRETTY_FUNCTION__);
          return SB_FE2000_STS_INIT_PT_XG0_TIMEOUT_ERR_K;
      }

      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE4_CONFIG, INIT, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE4_CONFIG, INIT_DONE, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE4_CONFIG, PTE_ENABLE, 1);
  }
  for (i = 0; i < SB_FE2000_MAX_XG_PORTS; i++) {
      CaPtPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_XGM0, i, 0xff);
  }

  if (pInitParams->xg[1].bBringUp) {
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE5_CONFIG, INIT, 1);
      if( sbFe2000InitRegDone(userDeviceHandle,
                              SAND_HAL_REG_OFFSET(CA, PT_PTE5_CONFIG),
                              SAND_HAL_FIELD_MASK(CA, PT_PTE5_CONFIG, INIT_DONE),
                              SB_FE2000_PT_INIT_TIMEOUT) ) {

          SB_ERROR("%s:Waiting for PT (XG1) init_done timed out..\n",
                   __PRETTY_FUNCTION__);
          return SB_FE2000_STS_INIT_PT_XG1_TIMEOUT_ERR_K;
      }

      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE5_CONFIG, INIT, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE5_CONFIG, INIT_DONE, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE5_CONFIG, PTE_ENABLE, 1);
  }
  for (i = 0; i < SB_FE2000_MAX_XG_PORTS; i++) {
      CaPtPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_XGM1, i, 0xff);
  }

  if (pInitParams->pci.bBringUp) {
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE6_CONFIG, INIT, 1);
      if( sbFe2000InitRegDone(userDeviceHandle,
                              SAND_HAL_REG_OFFSET(CA, PT_PTE6_CONFIG),
                              SAND_HAL_FIELD_MASK(CA, PT_PTE6_CONFIG, INIT_DONE),
                              SB_FE2000_PT_INIT_TIMEOUT) ) {

          SB_ERROR("%s:Waiting for PT (PCI) init_done timed out..\n",
                   __PRETTY_FUNCTION__);
          return SB_FE2000_STS_INIT_PT_PCI_TIMEOUT_ERR_K;
      }

      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE6_CONFIG, INIT, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE6_CONFIG, INIT_DONE, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE6_CONFIG, PTE_ENABLE, 1);
  }
  CaPtPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_PCI, 0, 0xff);

  if (pInitParams->pp.bBringUp) {
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE7_CONFIG, INIT, 1);
      if( sbFe2000InitRegDone(userDeviceHandle,
                              SAND_HAL_REG_OFFSET(CA, PT_PTE7_CONFIG),
                              SAND_HAL_FIELD_MASK(CA, PT_PTE7_CONFIG, INIT_DONE),
                              SB_FE2000_PT_INIT_TIMEOUT) ) {

          SB_ERROR("%s:Waiting for PT (PED)  init_done timed out..\n",
                   __PRETTY_FUNCTION__);
          return SB_FE2000_STS_INIT_PT_PED_TIMEOUT_ERR_K;
      }

      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE7_CONFIG, INIT, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE7_CONFIG, INIT_DONE, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE7_CONFIG, PTE_ENABLE, 1);

      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_PTE7_CONFIG, PB_ENABLE,
                         sbFe2000InitConvertPbEnables(pInitParams, FALSE, 0));
  }

  DEXIT();

  return SB_FE2000_STS_INIT_OK_K;

}

static uint32_t
sbFe2000InitPr(sbhandle userDeviceHandle,
               sbFe2000InitParams_t *pInitParams)
{
    int i;
    uint32_t v0, v1;
  DENTER();

  /* rgf - May 11 2006 - Take block out of reset */
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, PR_CORE_RESET, 0x0);

  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_CONFIG, PED_PRIORITY, (uint32_t)pInitParams->pr.PipelinePriority);

  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE0_CONFIG, PRE_ENABLE, 1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE0_CONFIG,
                     PB1_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[0][1]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE0_CONFIG,
                     PB0_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[0][0]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE0_CONFIG, PB_ENABLE,
                     sbFe2000InitConvertPbEnables(pInitParams, TRUE, 0));

  SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PR_PRE0_ENABLE_CONFIG0,
                     PORT_ENABLE,0xffffffff);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PR_PRE0_ENABLE_CONFIG1,
                     PORT_ENABLE,0xffffffff);
  v0 = 0;
  v1 = 0;
  for (i = 0; i < SB_FE2000_MAX_PORTS_PER_SPI; i++) {
      if (i >= 32) {
          v1 |= (pInitParams->pr.nPre0_PacketBufferSelect[i] << (i % 32));
      } else {
          v0 |= (pInitParams->pr.nPre0_PacketBufferSelect[i] << (i % 32));
      }
      CaPrPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_SPI0, i, 0xff);
  }
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PR_PRE0_PB_SELECT_CONFIG0,
                     PACKET_BUFFER_SELECT, v0);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PR_PRE0_PB_SELECT_CONFIG1,
                     PACKET_BUFFER_SELECT, v1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PR_PRE0_DEBUG1,
                     FP_UNDERFLOW_WAIT_CNT, 16);

  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE1_CONFIG, PRE_ENABLE, 1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE1_CONFIG,
                     PB1_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[1][1]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE1_CONFIG,
                     PB0_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[1][0]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE1_CONFIG, PB_ENABLE,
                     sbFe2000InitConvertPbEnables(pInitParams, TRUE, 1));

  SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PR_PRE1_ENABLE_CONFIG0,
                     PORT_ENABLE,0xffffffff);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PR_PRE1_ENABLE_CONFIG1,PORT_ENABLE,
                     0xffffffff);
  v0 = 0;
  v1 = 0;
  for (i = 0; i < SB_FE2000_MAX_PORTS_PER_SPI; i++) {
      if (i >= 32) {
          v1 |= (pInitParams->pr.nPre1_PacketBufferSelect[i] << (i % 32));
      } else {
          v0 |= (pInitParams->pr.nPre1_PacketBufferSelect[i] << (i % 32));
      }
      CaPrPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_SPI1, i, 0xff);
  }
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PR_PRE1_PB_SELECT_CONFIG0,
                     PACKET_BUFFER_SELECT, v0);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PR_PRE1_PB_SELECT_CONFIG1,
                     PACKET_BUFFER_SELECT, v1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PR_PRE1_DEBUG1,
                     FP_UNDERFLOW_WAIT_CNT, 16);

  /*  AG0 */
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE2_CONFIG, PRE_ENABLE, 1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE2_CONFIG, PB1_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[2][1]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE2_CONFIG, PB0_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[2][0]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE2_CONFIG, PB_ENABLE,
                     sbFe2000InitConvertPbEnables(pInitParams, TRUE, 2));

  SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PR_PRE2_ENABLE_CONFIG,PORT_ENABLE,
                     0xfff);
  v0 = 0;
  for (i = 0; i < SB_FE2000_MAX_AG_PORTS; i++) {
      v0 |= (pInitParams->pr.nPre2_PacketBufferSelect[i] << i);
      CaPrPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_AGM0, i, 0xff);
  }

  SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PR_PRE2_PB_SELECT_CONFIG,
                     PACKET_BUFFER_SELECT, v0);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PR_PRE2_DEBUG1,
                     FP_UNDERFLOW_WAIT_CNT, 16);

  /* AG1 */
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE3_CONFIG, PRE_ENABLE, 1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE3_CONFIG, PB1_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[3][1]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE3_CONFIG, PB0_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[3][0]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE3_CONFIG, PB_ENABLE,
                     sbFe2000InitConvertPbEnables(pInitParams, TRUE, 3));

  SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PR_PRE3_ENABLE_CONFIG,PORT_ENABLE,
                     0xfff);

  v0 = 0;
  for (i = 0; i < SB_FE2000_MAX_AG_PORTS; i++) {
      v0 |= (pInitParams->pr.nPre3_PacketBufferSelect[i] << i);
      CaPrPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_AGM1, i, 0xff);
  }

  SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PR_PRE3_PB_SELECT_CONFIG,
                     PACKET_BUFFER_SELECT, v0);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PR_PRE3_DEBUG1,
                     FP_UNDERFLOW_WAIT_CNT, 16);

  /* XG0 */
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE4_CONFIG, PRE_ENABLE, 1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE4_CONFIG, PB1_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[4][1]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE4_CONFIG, PB0_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[4][0]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE4_CONFIG, PB_ENABLE,
                     sbFe2000InitConvertPbEnables(pInitParams, TRUE, 4));
  for (i = 0; i < SB_FE2000_MAX_XG_PORTS; i++) {
      CaPrPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_XGM0, i, 0xff);
  }
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE4_PB_SELECT_CONFIG,
                     PACKET_BUFFER_SELECT,
                     pInitParams->pr.nPre4_PacketBufferSelect);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PR_PRE4_DEBUG1,
                     FP_UNDERFLOW_WAIT_CNT, 16);

  /*  XG1 */
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE5_CONFIG, PRE_ENABLE, 1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE5_CONFIG, PB1_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[5][1]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE5_CONFIG, PB0_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[5][0]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE5_CONFIG, PB_ENABLE,
                     sbFe2000InitConvertPbEnables(pInitParams, TRUE, 5));
  for (i = 0; i < SB_FE2000_MAX_XG_PORTS; i++) {
      CaPrPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_XGM1, i, 0xff);
  }

  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE5_PB_SELECT_CONFIG,
                     PACKET_BUFFER_SELECT,
                     pInitParams->pr.nPre5_PacketBufferSelect);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PR_PRE5_DEBUG1,
                     FP_UNDERFLOW_WAIT_CNT, 16);

  /* PCI */
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE6_CONFIG, PRE_ENABLE, 1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE6_CONFIG, PB1_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[6][1]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE6_CONFIG, PB0_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[6][0]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE6_CONFIG, PB_ENABLE,
                     sbFe2000InitConvertPbEnables(pInitParams, TRUE, 6));
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE6_PB_SELECT_CONFIG,
                     PACKET_BUFFER_SELECT,
                     pInitParams->pr.nPre6_PacketBufferSelect);
  CaPrPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_PCI, 0, 0xff);

  /* PED */
  if (pInitParams->pr.bPrEnable[7]) {
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE7_CONFIG, PRE_ENABLE, 1);
      SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PR_PRE7_CONFIG, PB_ENABLE,
                         sbFe2000InitConvertPbEnables(pInitParams, TRUE, 7));
  }
  DEXIT();
  return SB_FE2000_STS_INIT_OK_K;

}

static uint32_t
sbFe2000InitPp(sbhandle userDeviceHandle,
               sbFe2000InitParams_t *pInitParams) {

  uint32_t uIndex;
  uint32_t uQueue;
  sbFe2000InitParamsPp_t *pPpInitParams = &(pInitParams->pp);
  uint32_t uEthTypeMaxMin = 0;
  uint32_t uPpHdrTypeConfig0;
  uint32_t uPpSourceBufferBatch;

  DENTER();

  SB_ASSERT(!SAND_HAL_IS_FE2KXT((int)userDeviceHandle));

  if(!g_bSimRun) {
    /* Need a low->high transition to ensure PPE TCAM is properly reset */
    SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PP_CONFIG, TCAM_POR, 0);
    SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PP_CONFIG, TCAM_POR, 1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PP_CONFIG, INIT_DONE, 1);
  }

  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PC_CORE_RESET, PP_CORE_RESET, 0x0);
  /* rgf - Oct 25 2006 - Purposefully not hitting init bit, PPE performs init automatically */
  /* rgf - Oct 25 2006 - coming out of reset */

  if( sbFe2000InitRegDone(userDeviceHandle,
			  SAND_HAL_REG_OFFSET(CA, PP_CONFIG),
			  SAND_HAL_FIELD_MASK(CA, PP_CONFIG, INIT_DONE),
			  SB_FE2000_PP_INIT_TIMEOUT) ) {

    SB_ERROR("%s Waiting for PP init_done timed out..\n",__PRETTY_FUNCTION__);
    return SB_FE2000_STS_INIT_PP_TIMEOUT_ERR_K;
  }

  /*  rgf - Jun 20 2006 - Clear both the init and the init_done */
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PP_CONFIG, INIT, 0);
  SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PP_CONFIG, INIT_DONE, 0);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PP_TCAM_GLOBAL_CONFIG, 0xFFFFFFFF);

  uEthTypeMaxMin = SAND_HAL_MOD_FIELD(CA,PP_ETH_TYPE_RANGE,MIN_TYPE,uEthTypeMaxMin, pPpInitParams->uEthernetTypeExceptionMin);
  uEthTypeMaxMin = SAND_HAL_MOD_FIELD(CA,PP_ETH_TYPE_RANGE,MAX_TYPE,uEthTypeMaxMin, pPpInitParams->uEthernetTypeExceptionMax);
  SAND_HAL_WRITE(userDeviceHandle, CA,PP_ETH_TYPE_RANGE,uEthTypeMaxMin);

  /* configure the queue initial header type */
  for(uQueue=0; uQueue < pPpInitParams->uDefaultNumberofQueuesToConfigure;  uQueue++) {
    sbZfFe2000PpInitialQueueStateEntry_t zInitialQueueState;
    sbZfFe2000PpInitialQueueStateEntry_InitInstance(&zInitialQueueState);
    zInitialQueueState.m_uHeaderType =
        pInitParams->pp.QueueInitialState[uQueue].uHeaderType;
    zInitialQueueState.m_uHeaderShift =
        pInitParams->pp.QueueInitialState[uQueue].uShift;
    zInitialQueueState.m_uState =
        pInitParams->pp.QueueInitialState[uQueue].uState;
    zInitialQueueState.m_uVariable =
        pInitParams->pp.QueueInitialState[uQueue].uVariable;

    if ( CaPpInitialQueueStateWrite(userDeviceHandle, uQueue, &zInitialQueueState) ) {
    } else {
      SB_ERROR("%s Failed on write for queue(%d).",__PRETTY_FUNCTION__, uQueue);
    }
  }

  /* CAM and HASH Programming Section */
  /* -- this is done via ilib init (sbG2FePp.c) */

  /* configure the queue default record size */
  for(uIndex=0; uIndex < SB_FE2000_PP_NUM_BATCHGROUPS; uIndex++) {

    sbZfFe2000PpHeaderRecordSizeEntry_t zQueueRecordSize;
    sbZfFe2000PpHeaderRecordSizeEntry_InitInstance(&zQueueRecordSize);
    zQueueRecordSize.m_uRecordSize = pPpInitParams->uHeaderRecordSize[uIndex];

    if( CaPpHeaderRecordSizeWrite(userDeviceHandle,uIndex,&zQueueRecordSize) ) {
    } else {
      SB_ERROR("%s Failed on write for queue(%d).",__PRETTY_FUNCTION__, uQueue);
    }
  }

  /* configure the queue batch group and priority */
  for(uQueue=0; uQueue < pPpInitParams->uDefaultNumberofQueuesToConfigure;uQueue++) {

    sbZfFe2000PpQueuePriorityGroupEntry_t zQueueConfig;
    sbZfFe2000PpQueuePriorityGroupEntry_InitInstance(&zQueueConfig);
    zQueueConfig.m_uPriority = pPpInitParams->QueueConfiguration[uQueue].uPriority;
    zQueueConfig.m_uBatchGroup = pPpInitParams->QueueConfiguration[uQueue].uBatchGroup;

    if( CaPpQueuePriorityGroupWrite(userDeviceHandle,uQueue,&zQueueConfig) ) {
    } else {
      SB_ERROR("%s Failed on write for queue(%d).",__PRETTY_FUNCTION__, uQueue);
    }
  }

  /* dgm - Dec 28 2006 - Source buffer insert */

  uPpHdrTypeConfig0 = SAND_HAL_READ(userDeviceHandle, CA,PP_HDR_TYPE_CONFIG0);
  uPpSourceBufferBatch = SAND_HAL_GET_FIELD(CA,PP_HDR_TYPE_CONFIG0,
                                            SRC_BUFFER_BATCH,
                                            uPpHdrTypeConfig0);
  for(uIndex=0; uIndex < SB_FE2000_PP_NUM_BATCHGROUPS; uIndex++) {
    if( pPpInitParams->bSourceBufferInsert[uIndex] ) {
      uPpSourceBufferBatch |= (1 << uIndex);
    }
  }

  uPpHdrTypeConfig0 = SAND_HAL_MOD_FIELD(CA,PP_HDR_TYPE_CONFIG0,
                                         SRC_BUFFER_BATCH,uPpHdrTypeConfig0,
                                         uPpSourceBufferBatch);
  SAND_HAL_WRITE(userDeviceHandle, CA,PP_HDR_TYPE_CONFIG0,uPpHdrTypeConfig0);


  {
    /* LRP By Pass Mode configuration */
     uint32_t uDebugPedHeader = SAND_HAL_READ(userDeviceHandle, CA,PP_DEBUG_PED_HDR);
     UINT uPpDebugConfig;
     UINT uPpMakePedHeader;
     UINT uPpAddRouteHeader;
     UINT uPpDeleteFirstHeader;

     uDebugPedHeader = SAND_HAL_MOD_FIELD(CA,PP_DEBUG_PED_HDR,MIRROR_INDEX,uDebugPedHeader,pPpInitParams->PpByPassLrpModeConfiguration.uMirrorIndex);
     uDebugPedHeader = SAND_HAL_MOD_FIELD(CA,PP_DEBUG_PED_HDR,COPY_CNT,uDebugPedHeader,pPpInitParams->PpByPassLrpModeConfiguration.uCopyCount);
     uDebugPedHeader = SAND_HAL_MOD_FIELD(CA,PP_DEBUG_PED_HDR,HDR_COPY,uDebugPedHeader,pPpInitParams->PpByPassLrpModeConfiguration.bHeaderCopy);
     uDebugPedHeader = SAND_HAL_MOD_FIELD(CA,PP_DEBUG_PED_HDR,DROP,uDebugPedHeader,pPpInitParams->PpByPassLrpModeConfiguration.bDrop);
     SAND_HAL_WRITE(userDeviceHandle, CA,PP_DEBUG_PED_HDR,uDebugPedHeader);

      uPpDebugConfig = SAND_HAL_READ(userDeviceHandle, CA,PP_DEBUG_CONFIG);
      uPpMakePedHeader = SAND_HAL_GET_FIELD(CA,PP_DEBUG_CONFIG,C_USE_PED_HDR,uPpDebugConfig);

      for(uIndex=0; uIndex < SB_FE2000_PP_NUM_BATCHGROUPS; uIndex++) {
        if( pPpInitParams->PpByPassLrpModeConfiguration.bByPassLrpMode[uIndex] ) {
            uPpMakePedHeader |= (1 << uIndex);
        }
      }

      uPpAddRouteHeader = SAND_HAL_GET_FIELD(CA,PP_DEBUG_CONFIG,C_ADD_ROUTE_HDR,uPpDebugConfig);
      for(uIndex=0; uIndex < SB_FE2000_PP_NUM_BATCHGROUPS; uIndex++) {
        if( pPpInitParams->bAddRouteHeader[uIndex] ) {
            uPpAddRouteHeader |= (1 << uIndex);
        }
      }

      uPpDeleteFirstHeader = SAND_HAL_GET_FIELD(CA,PP_DEBUG_CONFIG,C_DELETE_FIRST_HDR,uPpDebugConfig);
      for(uIndex=0; uIndex < SB_FE2000_PP_NUM_BATCHGROUPS; uIndex++) {
        if( pPpInitParams->bDeleteFirstHeader[uIndex] ) {
            uPpDeleteFirstHeader |= (1 << uIndex);
        }
      }
      uPpDebugConfig = SAND_HAL_MOD_FIELD(CA,PP_DEBUG_CONFIG,C_USE_PED_HDR,uPpDebugConfig,uPpMakePedHeader);
      uPpDebugConfig = SAND_HAL_MOD_FIELD(CA,PP_DEBUG_CONFIG,C_ADD_ROUTE_HDR,uPpDebugConfig,uPpAddRouteHeader);
      uPpDebugConfig = SAND_HAL_MOD_FIELD(CA,PP_DEBUG_CONFIG,C_DELETE_FIRST_HDR,uPpDebugConfig,uPpDeleteFirstHeader);
      SAND_HAL_WRITE(userDeviceHandle, CA,PP_DEBUG_CONFIG,uPpDebugConfig);
  }

  {
    /* exceptions initialization */
    if(pInitParams->pp.exceptions.bEnableAllExceptions == TRUE) {
      SAND_HAL_WRITE(userDeviceHandle, CA,PP_EVENT0_MASK, 0);
      SAND_HAL_WRITE(userDeviceHandle, CA,PP_EVENT1_MASK, 0);
    }
  }

  DEXIT();
  return SB_FE2000_STS_INIT_OK_K;
}



/* rgf - Nov 13 2006 - Convert the boolean array for PB enable to a 2 bit mask */
/* rgf - Nov 13 2006 - bPre equals true if called for a PRE, otherwise this call is for PTE7 */
/* rgf - Nov 13 2006 - uPre indicates which PRE if bPre is true */
uint32_t sbFe2000InitConvertPbEnables(sbFe2000InitParams_t *pInitParams, uint8_t bPre, uint32_t uPre)
{
  uint32_t uPbEnable = 0;

  if( bPre == TRUE ) {
    if( pInitParams->pr.bEnablePbPrefetch[uPre][1] == TRUE ) {
      uPbEnable |= 0x2;
    }
    if( pInitParams->pr.bEnablePbPrefetch[uPre][0] == TRUE ) {
      uPbEnable |= 0x1;
    }
  } else {
    if( pInitParams->pt.bEnablePbPrefetch[1] == TRUE ) {
      uPbEnable |= 0x2;
    }
    if( pInitParams->pt.bEnablePbPrefetch[0] == TRUE ) {
      uPbEnable |= 0x1;
    }
  }

  return uPbEnable;
}

static
void sbFe2000InitCleanUp(sbhandle userDeviceHandle, sbFe2000InitParams_t *pInitParams)
{
  uint16_t nNum;
  DENTER();
  sbFe2000InitCleanUpPt(userDeviceHandle);
  sbFe2000InitCleanUpQm(userDeviceHandle);
  sbFe2000InitCleanUpPb(userDeviceHandle);
  sbFe2000InitCleanUpPr(userDeviceHandle);
  sbFe2000InitCleanUpPp(userDeviceHandle);
  sbFe2000InitCleanUpPd(userDeviceHandle);

  for(nNum=0;nNum<SB_FE2000_NUM_MM_INSTANCES;nNum++) {
    sbFe2000InitCleanUpMm(userDeviceHandle,nNum);
  }

  sbFe2000InitCleanUpCm(userDeviceHandle);

  for(nNum=0;nNum<SB_FE2000_NUM_RC_INSTANCES;nNum++) {
    sbFe2000InitCleanUpRc(userDeviceHandle,nNum);
  }

  sbFe2000InitCleanUpPm(userDeviceHandle);
  sbFe2000InitCleanUpLr(userDeviceHandle,pInitParams);

  for(nNum=0;nNum<SB_FE2000_NUM_AG_INTERFACES;nNum++) {
    sbFe2000InitCleanUpAgm(userDeviceHandle,nNum);
  }
  for(nNum=0;nNum<SB_FE2000_NUM_XG_INTERFACES;nNum++) {
    sbFe2000InitCleanUpXgm(userDeviceHandle,nNum);
  }

  sbFe2000InitCleanUpSpi4(userDeviceHandle,0);
  sbFe2000InitCleanUpSpi4(userDeviceHandle,1);

  SAND_HAL_RMW_FIELD(userDeviceHandle, CA,PC_CONFIG, CII_ENABLE, 0x1);

  DEXIT();
}

static
void sbFe2000InitCleanUpPt(sbhandle userDeviceHandle)
{
  /* rgf - May 12 2006 - Read and write back the PTE Error registers to clear */
  /* rgf - May 12 2006 - any error bits set and then unmask the error register */
  UINT uError = SAND_HAL_READ(userDeviceHandle, CA, PT_PTE0_ERROR0);
  DENTER();
  SAND_HAL_WRITE(userDeviceHandle, CA,  PT_PTE0_ERROR0, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PT_PTE0_ERROR0_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, CA, PT_PTE0_ERROR1);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PT_PTE0_ERROR1, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PT_PTE0_ERROR1_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, CA, PT_PTE1_ERROR0);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PT_PTE1_ERROR0, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PT_PTE1_ERROR0_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, CA, PT_PTE1_ERROR1);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PT_PTE1_ERROR1, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PT_PTE1_ERROR1_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, CA, PT_PTE2_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PT_PTE2_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PT_PTE2_ERROR_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, CA, PT_PTE3_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PT_PTE3_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PT_PTE3_ERROR_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, CA, PT_PTE4_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PT_PTE4_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PT_PTE4_ERROR_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, CA, PT_PTE5_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PT_PTE5_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PT_PTE5_ERROR_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, CA, PT_PTE6_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PT_PTE6_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PT_PTE6_ERROR_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, CA, PT_PTE7_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PT_PTE7_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PT_PTE7_ERROR_MASK, 0);

  DEXIT();
}

static
void sbFe2000InitCleanUpQm(sbhandle userDeviceHandle)
{
  /* rgf - May 12 2006 - Clear ERROR0/ERROR1 simply by reading them, all fields are counters */
  UINT uError = SAND_HAL_READ(userDeviceHandle, CA, QM_ERROR0);
  DENTER();
  uError = SAND_HAL_READ(userDeviceHandle, CA, QM_ERROR1);
  /* rgf - May 12 2006 - and unmask the error registers */
  SAND_HAL_WRITE(userDeviceHandle, CA,  QM_ERROR0_MASK, 0);
  SAND_HAL_WRITE(userDeviceHandle, CA,  QM_ERROR1_MASK, 0);
  SAND_HAL_WRITE(userDeviceHandle, CA,  QM_ERROR2_MASK, 0);
  SAND_HAL_WRITE(userDeviceHandle, CA,  QM_ERROR3_MASK, 0);

  DEXIT();
}

static
void sbFe2000InitCleanUpPb(sbhandle userDeviceHandle)
{
  DENTER();
  DEXIT();
}

static
void sbFe2000InitCleanUpPr(sbhandle userDeviceHandle)
{
  /* rgf - May 12 2006 - Read PR_ERROR and write back to clear all errors, then unmask */
  UINT uError = SAND_HAL_READ(userDeviceHandle, CA, PR_ERROR);
  DENTER();
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_ERROR_MASK, 0);

  /* rgf - May 12 2006 - Now do the same for all the PRE error registers */
  /* rgf - May 12 2006 - a bit repetitive, could use the stride trick but... */
  uError = SAND_HAL_READ(userDeviceHandle, CA, PR_PRE0_ERROR0);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE0_ERROR0, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE0_ERROR0_MASK, 0);
  uError = SAND_HAL_READ(userDeviceHandle, CA, PR_PRE0_ERROR1);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE0_ERROR1, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE0_ERROR1_MASK, 0);
  uError = SAND_HAL_READ(userDeviceHandle, CA, PR_PRE0_ERROR2);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE0_ERROR2, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE0_ERROR2_MASK, 0);
  uError = SAND_HAL_READ(userDeviceHandle, CA, PR_PRE0_ERROR3);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE0_ERROR3, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE0_ERROR3_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, CA, PR_PRE1_ERROR0);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE1_ERROR0, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE1_ERROR0_MASK, 0);
  uError = SAND_HAL_READ(userDeviceHandle, CA, PR_PRE1_ERROR1);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE1_ERROR1, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE1_ERROR1_MASK, 0);
  uError = SAND_HAL_READ(userDeviceHandle, CA, PR_PRE1_ERROR2);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE1_ERROR2, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE1_ERROR2_MASK, 0);
  uError = SAND_HAL_READ(userDeviceHandle, CA, PR_PRE1_ERROR3);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE1_ERROR3, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE1_ERROR3_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, CA, PR_PRE2_ERROR0);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE2_ERROR0, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE2_ERROR0_MASK, 0);
  uError = SAND_HAL_READ(userDeviceHandle, CA, PR_PRE2_ERROR1);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE2_ERROR1, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE2_ERROR1_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, CA, PR_PRE3_ERROR0);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE3_ERROR0, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE3_ERROR0_MASK, 0);
  uError = SAND_HAL_READ(userDeviceHandle, CA, PR_PRE3_ERROR1);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE3_ERROR1, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE3_ERROR1_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, CA, PR_PRE4_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE4_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE4_ERROR_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, CA, PR_PRE5_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE5_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE5_ERROR_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, CA, PR_PRE6_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE6_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE6_ERROR_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, CA, PR_PRE7_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE7_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PR_PRE7_ERROR_MASK, 0);

  DEXIT();
}

static
void sbFe2000InitCleanUpPp(sbhandle userDeviceHandle)
{
  UINT uError = SAND_HAL_READ(userDeviceHandle, CA, PP_EVENT0);
  DENTER();
  SAND_HAL_WRITE(userDeviceHandle, CA,  PP_EVENT0, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PP_EVENT0_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, CA, PP_EVENT1);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PP_EVENT1, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PP_EVENT1_MASK, 0);

  DEXIT();
}

static
void sbFe2000InitCleanUpPd(sbhandle userDeviceHandle)
{
  DENTER();
  /* rgf - Oct 17 2006 - To some extent, these probably shouldn't even be interrupts but leave them */
  /* rgf - Oct 17 2006 - masked otherwise every test which uses the pipeline will fail wrapup with these */
  /* rgf - Oct 17 2006 - interrupts set. */
  SAND_HAL_WRITE(userDeviceHandle, CA,  PD_COPY_IN_EVENT, SAND_HAL_SET_FIELD(CA, PD_COPY_IN_EVENT, HDR_REC_OVERFLOW, 1));
  SAND_HAL_WRITE(userDeviceHandle, CA,  PD_COPY_IN_EVENT_MASK, 0);

  SAND_HAL_WRITE(userDeviceHandle, CA,  PD_COPY_OUT_EVENT, SAND_HAL_SET_FIELD(CA, PD_COPY_OUT_EVENT, HDR_REC_OVERFLOW, 1));
  SAND_HAL_WRITE(userDeviceHandle, CA,  PD_COPY_OUT_EVENT_MASK, 0);

  DEXIT();
}

static
void sbFe2000InitCleanUpMm(sbhandle userDeviceHandle, uint16_t nNum)
{
  UINT uError = SAND_HAL_READ_STRIDE(userDeviceHandle, CA, MM, nNum, MM_ERROR);
  DENTER();
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA, MM, nNum, MM_ERROR, uError);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA, MM, nNum, MM_ERROR_MASK, 0);

  if ( nNum==0 ) {
  } else if ( nNum==1 ) {
  } else {
    SB_ASSERT(0);
  }
  DEXIT();
}

static
void sbFe2000InitCleanUpCm(sbhandle userDeviceHandle)
{
  UINT uError = SAND_HAL_READ(userDeviceHandle, CA, CM_SEGMENT_DISABLE_ERROR);
  UINT uCmLfsrEnable = 0;
  DENTER();
  SAND_HAL_WRITE(userDeviceHandle, CA,  CM_SEGMENT_DISABLE_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  CM_SEGMENT_DISABLE_ERROR_MASK, 0xffffffff);

  uError = SAND_HAL_READ(userDeviceHandle, CA, CM_SEGMENT_RANGE_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, CA,  CM_SEGMENT_RANGE_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  CM_SEGMENT_RANGE_ERROR_MASK, 0xffffffff);

  uError = SAND_HAL_READ(userDeviceHandle, CA, CM_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, CA,  CM_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  CM_ERROR_MASK, 0xffffffff);

  uError = SAND_HAL_READ(userDeviceHandle, CA, CM_SEGMENT_FLUSH_EVENT);
  SAND_HAL_WRITE(userDeviceHandle, CA,  CM_SEGMENT_FLUSH_EVENT, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  CM_SEGMENT_FLUSH_EVENT_MASK, 0xffffffff);

  uError = SAND_HAL_READ(userDeviceHandle, CA, CM_FORCE_FLUSH_EVENT);
  SAND_HAL_WRITE(userDeviceHandle, CA,  CM_FORCE_FLUSH_EVENT, uError);
  /*
  SAND_HAL_WRITE(userDeviceHandle, CA,  CM_FORCE_FLUSH_EVENT_MASK, 0xffffffff);
  */

  /* rgf - Oct 16 2006 - Clear on read */
  uError = SAND_HAL_READ(userDeviceHandle, CA, CM_TOTAL_NUM_UPDATES);
  /* jts - Oct 31 2006 - new tag, CM_TOTAL_NUM_UPDATES_MASK removed: */
  /* jts - Oct 31 2006 - SAND_HAL_WRITE(userDeviceHandle, CA,  CM_TOTAL_NUM_UPDATES_MASK, 0); */


  uCmLfsrEnable = SAND_HAL_MOD_FIELD( CA, CM_LFSR_ENABLE, ENABLE3, uCmLfsrEnable, 0x1 );
  uCmLfsrEnable = SAND_HAL_MOD_FIELD( CA, CM_LFSR_ENABLE, ENABLE2, uCmLfsrEnable, 0x1 );
  uCmLfsrEnable = SAND_HAL_MOD_FIELD( CA, CM_LFSR_ENABLE, ENABLE1, uCmLfsrEnable, 0x1 );
  uCmLfsrEnable = SAND_HAL_MOD_FIELD( CA, CM_LFSR_ENABLE, ENABLE0, uCmLfsrEnable, 0x1 );
  SAND_HAL_WRITE(userDeviceHandle, CA, CM_LFSR_ENABLE, uCmLfsrEnable );
  SAND_HAL_WRITE(userDeviceHandle, CA, CM_LFSR_ENABLE, 0x0 );
  DEXIT();
}

static
void sbFe2000InitCleanUpRc(sbhandle userDeviceHandle, uint16_t nNum)
{
  UINT uError = SAND_HAL_READ(userDeviceHandle, CA, RC_ERROR);
  DENTER();
  SAND_HAL_WRITE(userDeviceHandle, CA,  RC_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  RC_ERROR_MASK, 0);

  if ( nNum==0 ) {
    SAND_HAL_RMW_FIELD(userDeviceHandle, CA, RC0_ERROR_MASK,
                       FIFO_NOT_AVAIL_DISINT, 0);
    SAND_HAL_RMW_FIELD(userDeviceHandle, CA, RC0_ERROR_MASK,
                       PROG_NOT_CONFIGURED_DISINT, 0);
  } else if ( nNum==1 ) {
    SAND_HAL_RMW_FIELD(userDeviceHandle, CA, RC1_ERROR_MASK,
                       FIFO_NOT_AVAIL_DISINT, 0);
    SAND_HAL_RMW_FIELD(userDeviceHandle, CA, RC1_ERROR_MASK,
                       PROG_NOT_CONFIGURED_DISINT, 0);
  } else {
    SB_ASSERT(0);
  }
  DEXIT();
}


static
void sbFe2000InitCleanUpPm(sbhandle userDeviceHandle)
{
  uint32_t uValue;
  DENTER();
  /* jts - Aug 21 2006 - Read/Write-Back to ensure clean of PM_ERROR */
  uValue = SAND_HAL_READ(userDeviceHandle, CA, PM_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, CA,  PM_ERROR, uValue);

  SAND_HAL_WRITE(userDeviceHandle, CA,  PM_ERROR_MASK, 0);
  DEXIT();
}

static
void sbFe2000InitCleanUpLr(sbhandle userDeviceHandle, sbFe2000InitParams_t *pInitParams)
{
  UINT uEvent;
  UINT uTimeOut;
  UINT uError;

  DENTER();

  SB_ASSERT(!SAND_HAL_IS_FE2KXT((int)userDeviceHandle));

  if ( pInitParams->lr.bStreamOnline[0] && pInitParams->lr.bBringUp) {
    /* Polling stream0 online event for indication that the lrp streams are online */
    uEvent = 0;
    uTimeOut = 0;
    while ( !uEvent && (1500 > uTimeOut) ) {
      uEvent=SAND_HAL_GET_FIELD(CA, LR_EVENT, ONLINE0, SAND_HAL_READ(userDeviceHandle, CA, LR_EVENT));
      uTimeOut++;
    }
    if ( uTimeOut == 1500 ) {
      SB_ERROR("Timeout waiting for ONLINE0 event from LRP.\n");
    }
  }

  /* dgm - Oct 11 2006 - Read and clear any error bits from bringup, then unmask the error registers */

  uError = SAND_HAL_READ(userDeviceHandle, CA, LR_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, CA, LR_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA, LR_ERROR_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, CA, LR_EVENT);
  SAND_HAL_WRITE(userDeviceHandle, CA,  LR_EVENT, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  LR_EVENT_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, CA, LU_UCODE_EVENT0);
  SAND_HAL_WRITE(userDeviceHandle, CA,  LU_UCODE_EVENT0, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  LU_UCODE_EVENT0_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, CA, LU_UCODE_EVENT1);
  SAND_HAL_WRITE(userDeviceHandle, CA,  LU_UCODE_EVENT1, uError);
  SAND_HAL_WRITE(userDeviceHandle, CA,  LU_UCODE_EVENT1_MASK, 0);

  DEXIT();
}

static
void sbFe2000InitCleanUpAgm(sbhandle userDeviceHandle, uint16_t nNum)
{
  uint16_t nPort;
  uint32_t uData;
  DENTER();

  /* Clear any link status events due to auto-neg */
  uData = SAND_HAL_READ_STRIDE(userDeviceHandle, CA,AG,nNum,AG_LINK_STATUS_EVENT);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA,AG,nNum,AG_LINK_STATUS_EVENT,uData);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA,AG,nNum,AG_LINK_STATUS_EVENT_MASK,0x0);

  /* Clear any signal detect events */
  uData = SAND_HAL_READ_STRIDE(userDeviceHandle, CA,AG,nNum,AG_SIGNAL_DETECT_EVENT);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA,AG,nNum,AG_SIGNAL_DETECT_EVENT,uData);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, CA,AG,nNum,AG_SIGNAL_DETECT_EVENT_MASK,0x0);

  for ( nPort = 0; nPort < SB_FE2000_MAX_AG_PORTS; nPort++ ) {

    sbFe2000InitCleanUpAgmPortRx(nNum, nPort,userDeviceHandle);
    sbFe2000InitCleanUpAgmPortTx(nNum, nPort, userDeviceHandle);

  }

  /* Un-mask interrupts */
  if ( nNum == 0 ) {
  } else if ( nNum == 1 ) {
  } else {
    SB_ASSERT(0);
  }
  DEXIT();
}

static
void sbFe2000InitCleanUpAgmPortRx(uint16_t nNum, uint16_t nPort, sbhandle userDeviceHandle)
{
  uint32_t temp;
  DENTER();
#ifdef DEBUG_PRINT
  SB_LOGV1("%s Agm%d, port:%d\n",__PRETTY_FUNCTION__,nNum,nPort);
#endif
  /* SAND_HAL_WRITE_ADDR( sbFe2000AgmMacErrorMaskCalcAddr(nNum, nPort,userDeviceHandle), 0x0); */
  temp = sbFe2000UtilAgmMacErrorMaskCalcAddr(nNum, nPort,userDeviceHandle);
  DEXIT();
}

static
void sbFe2000InitCleanUpAgmPortTx(uint16_t nNum, uint16_t nPort, sbhandle userDeviceHandle)
{
  DENTER();
  /* kpl currently this routine appears to do nothing, be need updates for real hw */
#ifdef DEBUG_PRINT
  SB_LOGV1("%s Agm%d, port:%d\n",__PRETTY_FUNCTION__,nNum,nPort);
#endif
  DEXIT();
}

static
void sbFe2000InitCleanUpXgm(sbhandle userDeviceHandle, uint16_t nNum)
{
  DENTER();
  if ( nNum == 0 ) {
  } else if ( nNum == 1 ) {
  } else {
    SB_ASSERT(0);
  }
  DEXIT();
}

static
void sbFe2000InitCleanUpSpi4(sbhandle userDeviceHandle, uint16_t nNum)
{
  DENTER();
  DEXIT();
}

static void
sbFe2000InitPLL(sbhandle userDeviceHandle,
                sbFe2000InitParams_t *pInitParams)
{
    uint32_t uData0, uData1;

    DENTER();

    /* core PLL */
    SB_ASSERT(!SAND_HAL_IS_FE2KXT((int)userDeviceHandle));

    /*zero register pc_cpr.bypb_autoreg */
    uData0 = SAND_HAL_READ(userDeviceHandle, CA, PC_CPR);
    uData0 = SAND_HAL_MOD_FIELD(CA, PC_CPR, BYPB_AUTOREG, uData0, 0);
    SAND_HAL_WRITE(userDeviceHandle, CA, PC_CPR, uData0);

    /*assert high pc_cpr.reset for 1us */
    uData0 = SAND_HAL_MOD_FIELD(CA, PC_CPR, RESET, uData0, 1);
    SAND_HAL_WRITE(userDeviceHandle, CA, PC_CPR, uData0);
    thin_delay(1000);

    /*deassert pc_cpr.reset and wait 500usec for vco to settle */
    uData0 = SAND_HAL_MOD_FIELD(CA, PC_CPR, RESET, uData0, 0);
    SAND_HAL_WRITE(userDeviceHandle, CA, PC_CPR, uData0);
    thin_delay(500000);

    /* SPI Frequency */
    uData0 = SAND_HAL_READ(userDeviceHandle, CA, ST0_CONFIG3);
    uData1 = SAND_HAL_READ(userDeviceHandle, CA, ST1_CONFIG3);
    uData0 = SAND_HAL_MOD_FIELD(CA, ST0_CONFIG3, TXPLL_N1DIV, uData0, 19);
    uData1 = SAND_HAL_MOD_FIELD(CA, ST1_CONFIG3, TXPLL_N1DIV, uData1, 19);
    SAND_HAL_WRITE(userDeviceHandle, CA, ST0_CONFIG3, uData0);
    SAND_HAL_WRITE(userDeviceHandle, CA, ST1_CONFIG3, uData1);

    /* SPI PLL */
    /* zero register st0/1_config3.txpll_bypb_autoreg */
    uData0 = SAND_HAL_READ(userDeviceHandle, CA, ST0_CONFIG3);
    uData1 = SAND_HAL_READ(userDeviceHandle, CA, ST1_CONFIG3);
    uData0 = SAND_HAL_MOD_FIELD(CA, ST0_CONFIG3, TXPLL_BYPB_AUTOREG, uData0, 0);
    uData1 = SAND_HAL_MOD_FIELD(CA, ST1_CONFIG3, TXPLL_BYPB_AUTOREG, uData1, 0);
    SAND_HAL_WRITE(userDeviceHandle, CA, ST0_CONFIG3, uData0);
    SAND_HAL_WRITE(userDeviceHandle, CA, ST1_CONFIG3, uData1);

    /* assert high st0/1_config3.txpll_reset for 1us */
    uData0 = SAND_HAL_MOD_FIELD(CA, ST0_CONFIG3, TXPLL_RESET, uData0, 1);
    uData1 = SAND_HAL_MOD_FIELD(CA, ST1_CONFIG3, TXPLL_RESET, uData1, 1);
    SAND_HAL_WRITE(userDeviceHandle, CA, ST0_CONFIG3, uData0);
    SAND_HAL_WRITE(userDeviceHandle, CA, ST1_CONFIG3, uData1);
    thin_delay(1000);

    /* deassert st0/1_config3.txpll_reset and wait 500us for vco to settle */
    uData0 = SAND_HAL_MOD_FIELD(CA, ST0_CONFIG3, TXPLL_RESET, uData0, 0);
    uData1 = SAND_HAL_MOD_FIELD(CA, ST1_CONFIG3, TXPLL_RESET, uData1, 0);
    SAND_HAL_WRITE(userDeviceHandle, CA, ST0_CONFIG3, uData0);
    SAND_HAL_WRITE(userDeviceHandle, CA, ST1_CONFIG3, uData1);
    thin_delay(200000); /* the rest of bringup takes 300us before we get to the spi */

    /* Lc PLL */
    /* reset lcpll divider: assert high xg_xgxs_pll_control1.reset for 1us */
    uData0 = SAND_HAL_READ(userDeviceHandle, CA, XG_XGXS_PLL_CONTROL1);
    uData0 = SAND_HAL_MOD_FIELD(CA, XG_XGXS_PLL_CONTROL1, RESET, uData0, 1);
    SAND_HAL_WRITE(userDeviceHandle, CA,  XG_XGXS_PLL_CONTROL1, uData0);

    /* deassert xg_xgxs_pll_control1.reset */
    uData0 = SAND_HAL_MOD_FIELD(CA, XG_XGXS_PLL_CONTROL1, RESET, uData0, 0);
    SAND_HAL_WRITE(userDeviceHandle, CA,  XG_XGXS_PLL_CONTROL1, uData0);

    /* reset lcpll vco: assert high xg_xgxs_pll_control0.reset for 30us */
    uData0 = SAND_HAL_READ(userDeviceHandle, CA, XG_XGXS_PLL_CONTROL0);
    uData0 = SAND_HAL_MOD_FIELD(CA, XG_XGXS_PLL_CONTROL0, RESET, uData0, 1);
    SAND_HAL_WRITE(userDeviceHandle, CA,  XG_XGXS_PLL_CONTROL0, uData0);

    /* deassert xg_xgxs_pll_control0.reset and wait 20msec */
    uData0 = SAND_HAL_MOD_FIELD(CA, XG_XGXS_PLL_CONTROL0, RESET, uData0, 0);
    SAND_HAL_WRITE(userDeviceHandle, CA,  XG_XGXS_PLL_CONTROL0, uData0);
    thin_delay(80);

    DEXIT();
}

static uint32_t
sbFe2000InitFuse(sbhandle userDeviceHandle,sbFe2000InitParams_t *pInitParams)
{
    uint32_t status = 0;
    uint32_t uData;
    DENTER();
    SB_ASSERT(!SAND_HAL_IS_FE2KXT((int)userDeviceHandle));

    /* start fuse state machine to program all memory redundancy info */
    uData = SAND_HAL_READ(userDeviceHandle, CA, PC_CORE_RESET);
    uData = SAND_HAL_MOD_FIELD(CA, PC_CORE_RESET, FS0_CORE_RESET, uData, 0);
    uData = SAND_HAL_MOD_FIELD(CA, PC_CORE_RESET, FS1_CORE_RESET, uData, 0);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_CORE_RESET, uData);

    if (!g_bSimRun) {
        thin_delay(17000);
        uData = SAND_HAL_READ(userDeviceHandle, CA, FS0_STATUS);
        if (!SAND_HAL_GET_FIELD(CA, FS0_STATUS, DONE, uData)) {
            SB_ERROR("Fuse 0 state machine didn't finish");
            status = -1;
        } else {
            SB_LOGV1("Fuse 0 state machine completed");
        };
        uData = SAND_HAL_READ(userDeviceHandle, CA, FS1_STATUS);
        if (!SAND_HAL_GET_FIELD(CA, FS1_STATUS, DONE, uData)) {
            SB_ERROR("Fuse 1 state machine didn't finish");
            status = -1;
        } else {
            SB_LOGV1("Fuse 1 state machine completed");
        };
    }

    DEXIT();
    return status;
}


sbStatus_t
sbFe2000InitClearMems(sbhandle userDeviceHandle,
                      sbFe2000InitParams_t *pInitParams)
{
    uint32_t v, v0;
    int i;

    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CLI_ADDR0, 0);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CLI_ADDR1, 0);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CLI_ADDR2, 0);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CLI_ADDR3, 0);

    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_PCI_ADDR0, 0);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_PCI_ADDR1, 0);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_PCI_ADDR2, 0);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_PCI_ADDR3, 0);

    v0 = 0;
    v0 = SAND_HAL_MOD_FIELD(CA, PC_DMA_CTRL0, ACK, v0, 1);
    v0 = SAND_HAL_MOD_FIELD(CA, PC_DMA_CTRL0, REQ, v0, 1);
    v0 = SAND_HAL_MOD_FIELD(CA, PC_DMA_CTRL0, FIXED_PTRN, v0, 1);

    /* Clear the internal RAMs */
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_COUNT0, 16 * 1024);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_COUNT1, 16 * 1024);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_COUNT2, 16 * 1024);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_COUNT3, 16 * 1024);

    v = SAND_HAL_MOD_FIELD(CA, PC_DMA_CTRL0, DMA_SRC, v0,
                           SB_FE2000_PC_DMA_SRC_MM0_INT0);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CTRL0, v);
    v = SAND_HAL_MOD_FIELD(CA, PC_DMA_CTRL0, DMA_SRC, v0,
                           SB_FE2000_PC_DMA_SRC_MM0_INT1);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CTRL1, v);
    v = SAND_HAL_MOD_FIELD(CA, PC_DMA_CTRL0, DMA_SRC, v0,
                           SB_FE2000_PC_DMA_SRC_MM1_INT0);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CTRL2, v);
    v = SAND_HAL_MOD_FIELD(CA, PC_DMA_CTRL0, DMA_SRC, v0,
                           SB_FE2000_PC_DMA_SRC_MM1_INT1);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CTRL3, v);

    for (i = 0; i < 1000; i++) {
        thin_delay(100000);
        if (SAND_HAL_READ(userDeviceHandle, CA, PC_DMA_CTRL0)
            & SAND_HAL_READ(userDeviceHandle, CA, PC_DMA_CTRL1)
            & SAND_HAL_READ(userDeviceHandle, CA, PC_DMA_CTRL2)
            & SAND_HAL_READ(userDeviceHandle, CA, PC_DMA_CTRL3)
            & SAND_HAL_CA_PC_DMA_CTRL0_ACK_MASK) {
            break;
        }
    }
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CTRL0,
                   SAND_HAL_CA_PC_DMA_CTRL0_ACK_MASK);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CTRL1,
                   SAND_HAL_CA_PC_DMA_CTRL0_ACK_MASK);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CTRL2,
                   SAND_HAL_CA_PC_DMA_CTRL0_ACK_MASK);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CTRL3,
                   SAND_HAL_CA_PC_DMA_CTRL0_ACK_MASK);
    if (i == 1000) {
        return SB_FE_MEM_ACC_TIMEOUT_ERR_CODE;
    }

    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_COUNT0,
                   pInitParams->mm[0].ramconfiguration.NarrowPortRam0Words);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_COUNT1,
                   pInitParams->mm[0].ramconfiguration.NarrowPortRam1Words);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_COUNT2,
                   pInitParams->mm[1].ramconfiguration.NarrowPortRam0Words);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_COUNT3,
                   pInitParams->mm[1].ramconfiguration.NarrowPortRam1Words);

    v = SAND_HAL_MOD_FIELD(CA, PC_DMA_CTRL0, DMA_SRC, v0,
                           SB_FE2000_PC_DMA_SRC_MM0_NARROW0);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CTRL0, v);
    v = SAND_HAL_MOD_FIELD(CA, PC_DMA_CTRL0, DMA_SRC, v0,
                           SB_FE2000_PC_DMA_SRC_MM0_NARROW1);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CTRL1, v);
    v = SAND_HAL_MOD_FIELD(CA, PC_DMA_CTRL0, DMA_SRC, v0,
                           SB_FE2000_PC_DMA_SRC_MM1_NARROW0);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CTRL2, v);
    v = SAND_HAL_MOD_FIELD(CA, PC_DMA_CTRL0, DMA_SRC, v0,
                           SB_FE2000_PC_DMA_SRC_MM1_NARROW1);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CTRL3, v);

    for (i = 0; i < 100000; i++) {
        thin_delay(10000);
        if (SAND_HAL_READ(userDeviceHandle, CA, PC_DMA_CTRL0)
            & SAND_HAL_READ(userDeviceHandle, CA, PC_DMA_CTRL1)
            & SAND_HAL_READ(userDeviceHandle, CA, PC_DMA_CTRL2)
            & SAND_HAL_READ(userDeviceHandle, CA, PC_DMA_CTRL3)
            & SAND_HAL_CA_PC_DMA_CTRL0_ACK_MASK) {
            break;
        }
    }
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CTRL0,
                   SAND_HAL_CA_PC_DMA_CTRL0_ACK_MASK);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CTRL1,
                   SAND_HAL_CA_PC_DMA_CTRL0_ACK_MASK);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CTRL2,
                   SAND_HAL_CA_PC_DMA_CTRL0_ACK_MASK);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CTRL3,
                   SAND_HAL_CA_PC_DMA_CTRL0_ACK_MASK);
    if (i == 10000) {
        return SB_FE_MEM_ACC_TIMEOUT_ERR_CODE;
    }

    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_COUNT0,
                   pInitParams->mm[0].ramconfiguration.WidePortRamWords * 2);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_COUNT1,
                   pInitParams->mm[1].ramconfiguration.WidePortRamWords * 2);

    v0 = SAND_HAL_MOD_FIELD(CA, PC_DMA_CTRL0, DMA_SRC_TYPE, v0, 1);
    v = SAND_HAL_MOD_FIELD(CA, PC_DMA_CTRL0, DMA_SRC, v0,
                           SB_FE2000_PC_DMA_SRC_MM0_WIDE);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CTRL0, v);
    v = SAND_HAL_MOD_FIELD(CA, PC_DMA_CTRL0, DMA_SRC, v0,
                           SB_FE2000_PC_DMA_SRC_MM1_WIDE);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CTRL1, v);

    for (i = 0; i < 100000; i++) {
        thin_delay(10000);
        if (SAND_HAL_READ(userDeviceHandle, CA, PC_DMA_CTRL0)
            & SAND_HAL_READ(userDeviceHandle, CA, PC_DMA_CTRL1)
            & SAND_HAL_CA_PC_DMA_CTRL0_ACK_MASK) {
            break;
        }
    }
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CTRL0,
                   SAND_HAL_CA_PC_DMA_CTRL0_ACK_MASK);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CTRL1,
                   SAND_HAL_CA_PC_DMA_CTRL0_ACK_MASK);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CTRL2,
                   SAND_HAL_CA_PC_DMA_CTRL0_ACK_MASK);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_DMA_CTRL3,
                   SAND_HAL_CA_PC_DMA_CTRL0_ACK_MASK);

    if (i == 10000) {
        return SB_FE_MEM_ACC_TIMEOUT_ERR_CODE;
    }

    return SB_FE2000_STS_INIT_OK_K;
}

void sbFe2000StartBubbleTimer(uint32_t unit, uint32_t startBubble, uint32_t size)
{
  /* Configure the lrp to insert bubbles on the ingress task of stream 0 */
  uint32_t uBubbleStream, uRingData;
  int uAddress, nData;

  SB_ASSERT(!SOC_IS_SBX_FE2KXT(unit));

  /* init List manager on service processor - port 3
   * 8K elements, stride =1, enable ring
   * LIST_CONFIG_B* must be configured before LIST_CONFIG_A*
   */

  uRingData = 0;
  uRingData = SAND_HAL_MOD_FIELD( CA, LU_LIST_CONFIG_B1, BASE, uRingData, 0x0 );
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, LU_LIST_CONFIG_B1, uRingData);

  uRingData = 0;
  uRingData = SAND_HAL_MOD_FIELD( CA, LU_LIST_CONFIG_A1, UPDATE, uRingData, 0x1 );
  uRingData = SAND_HAL_MOD_FIELD( CA, LU_LIST_CONFIG_A1, RING,   uRingData, 0x1 );
  uRingData = SAND_HAL_MOD_FIELD( CA, LU_LIST_CONFIG_A1, STRIDE, uRingData, 0x1 );
  uRingData = SAND_HAL_MOD_FIELD( CA, LU_LIST_CONFIG_A1, LENGTH, uRingData, size-1);
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, LU_LIST_CONFIG_A1, uRingData);


  /* clear the bubble packet */
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_DATA0, 0);              
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_DATA1, 0);              
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_DATA2, 0);              
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_DATA3, 0);              
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_DATA4, 0);              
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_DATA5, 0);              
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_DATA6, 0);              
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_DATA7, 0);              
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_DATA8, 0);              
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_DATA9, 0);              
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_DATA10, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_DATA11, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_DATA12, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_DATA13, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_DATA14, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_DATA15, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_DATA16, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_DATA17, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_DATA18, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_DATA19, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_DATA20, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_DATA21, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_DATA22, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_DATA23, 0);            
                                 
  uAddress = SB_FE2000_QM_OAM_SRC_BUFFER * 2; /* 0x71f * 2 = 0xe3e */  
                                
  SAND_HAL_WRITE( SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_CTRL,                       
                 SAND_HAL_SET_FIELD(CA, PB0_MEM_ACC_CTRL, REQ, 1) |           
                 SAND_HAL_SET_FIELD(CA, PB0_MEM_ACC_CTRL, ACK, 1) |           
                 SAND_HAL_SET_FIELD(CA, PB0_MEM_ACC_CTRL, RD_WR_N, 0) |       
                 SAND_HAL_SET_FIELD(CA, PB0_MEM_ACC_CTRL, ADDRESS, uAddress)  
                 );                                                           
                                                                            
  nData = SAND_HAL_READ( SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_CTRL);     
  nData = SAND_HAL_MOD_FIELD(CA, PB0_MEM_ACC_CTRL, REQ, nData, 0);      
  nData = SAND_HAL_MOD_FIELD(CA, PB0_MEM_ACC_CTRL, ACK, nData, 1);      
  SAND_HAL_WRITE( SOC_SBX_SBHANDLE(unit), CA, PB0_MEM_ACC_CTRL, nData);          

  /* init bubble timer on service processor - port 7 */
  /* FE2K */
  /* all bubble types are dropped into oam-tx stream (5). */
  uBubbleStream = SAND_HAL_SET_FIELD(CA, LR_BUBBLE_STREAM, TYPE0, 5) |
                  SAND_HAL_SET_FIELD(CA, LR_BUBBLE_STREAM, TYPE1, 5) |
                  SAND_HAL_SET_FIELD(CA, LR_BUBBLE_STREAM, TYPE2, 5) |
                  SAND_HAL_SET_FIELD(CA, LR_BUBBLE_STREAM, TYPE3, 5) |
                  SAND_HAL_SET_FIELD(CA, LR_BUBBLE_STREAM, TYPE4, 5) |
                  SAND_HAL_SET_FIELD(CA, LR_BUBBLE_STREAM, TYPE5, 5) |
                  SAND_HAL_SET_FIELD(CA, LR_BUBBLE_STREAM, TYPE6, 5) |
                  SAND_HAL_SET_FIELD(CA, LR_BUBBLE_STREAM, TYPE7, 5);
  SAND_HAL_WRITE( SOC_SBX_SBHANDLE(unit), CA, LR_BUBBLE_STREAM, uBubbleStream);

  /* Bubble message will simulate ppe message with header types/locn f's */
  SAND_HAL_WRITE( SOC_SBX_SBHANDLE(unit), CA, LR_BUBBLE_HEADER0, 0xffffffff);
  SAND_HAL_WRITE( SOC_SBX_SBHANDLE(unit), CA, LR_BUBBLE_HEADER1, 0xffffffff);

  /* start the OAM bubble.  REQ3 is for port 7.  startBubbble is bool: 0=off, 1=on */
  /* SAND_HAL_RMW_FIELD( SOC_SBX_SBHANDLE(unit), CA, LR_BUBBLE, INGRESS, 0xff); */
  SAND_HAL_RMW_FIELD( SOC_SBX_SBHANDLE(unit), CA, LR_BUBBLE, REQ3, startBubble);
}


#define TO_QUEUE_OFFSET 128
#define EGRESS_QUEUE_OFFSET 64
#define XGM_EXPANDED_FIFO_SIZE 63

#define SWS_QUEUES_PER_GROUP 16
#define SWS_INGRESS_QUEUE_GROUPS \
 ((SB_FE2000_SWS_INIT_MAX_CONNECTIONS / 2) / SWS_QUEUES_PER_GROUP)

#define SWS_PAGES_PER_PB 1800
#define SWS_BYTES_PER_PAGE 192
#define SPI_MAX_LINES 768
#define SPI_MAX_LINES_PER_PORT 127
#define SPI_MIN_LINES_PER_PORT 7
#define MAX_PACKET 9000

sbStatus_t
sbFe2000InitQueues(sbhandle userDeviceHandle,
                   sbFe2000Queues_t *queues)
{
    uint32_t fromIface, fromPort, toIface, toPort, fromQid, toQid;
    sbZfFe2000PtQueueToPortEntry_t q2p;
    sbZfFe2000QmQueueConfigEntry_t qc;
    sbFe2000Connection_t *c;
    sbZfFe2000QmQueueState0EnEntry_t qe;
    uint32_t pb0PagesAvail, pb1PagesAvail, v, qmConfig0;
    int i, many2one, pbsel, egress;
    uint32_t mirrorData, uPtConfig, uPtData;

    SB_ASSERT(queues);
    sbZfFe2000PtQueueToPortEntry_InitInstance(&q2p);
    sbZfFe2000QmQueueConfigEntry_InitInstance(&qc);
    sbZfFe2000QmQueueState0EnEntry_InitInstance(&qe);

    qc.m_uEnforcePoliceMarkings = 0;
    qc.m_uDropThresh2 = 0xfff;
    qc.m_uDropThresh1 = 0;
    qc.m_uEnable = 1;

    qe.m_uEnable = 1;

    qmConfig0 = SAND_HAL_READ(userDeviceHandle, CA, QM_CONFIG0);
    pb0PagesAvail = SAND_HAL_GET_FIELD(CA, QM_CONFIG0,
                                       TOTAL_MAX_PAGES_AVAILABLE_PB0,
                                       qmConfig0);
    pb1PagesAvail = SAND_HAL_GET_FIELD(CA, QM_CONFIG0,
                                       TOTAL_MAX_PAGES_AVAILABLE_PB1,
                                       qmConfig0);

    /* Expanded Mode Configuration */
    SAND_HAL_RMW_FIELD(userDeviceHandle, CA, PT_CONFIG1, EXPANDED_FIFO_SIZE,
                       XGM_EXPANDED_FIFO_SIZE);

    for (i = 0; i < queues->n; i++) {
        c = &queues->connections[i];
        egress = c->bEgress;
        fromIface = c->from.ulInterfacePortType;
        fromPort = c->from.ulPortId;
        toIface = c->to.ulInterfacePortType;
        toPort = c->to.ulPortId;
        many2one = queues->many2onecon[i] != SB_FE2000_SWS_UNINITIALIZED_CON;
        pbsel = queues->pbsel[i];

        fromQid = queues->fromqid[i];
        toQid = queues->toqid[i];

        if (queues->ifexpmode[fromIface] || queues->ifexpmode[toIface]) {
            uPtConfig = SAND_HAL_READ(userDeviceHandle, CA, PT_CONFIG1);
            uPtData = SAND_HAL_GET_FIELD(CA, PT_CONFIG1, EXPANDED_FIFO_ENABLE,
                                         uPtConfig);
            uPtData |= ((1 << (fromQid / SWS_QUEUES_PER_GROUP))
                        | (1 << (toQid / SWS_QUEUES_PER_GROUP)));
            uPtConfig = SAND_HAL_MOD_FIELD(CA, PT_CONFIG1,
                                           EXPANDED_FIFO_ENABLE, uPtConfig,
                                           uPtData);
            SAND_HAL_WRITE(userDeviceHandle, CA,  PT_CONFIG1, uPtConfig);
        }

        /* Configure the *->HPP queue */
        qc.m_uFlowControlTarget = SB_FE2000_QM_FLOW_CONTROL_TARGET_PORT;
        qc.m_uFlowControlPort = (fromIface << 6) + fromPort;
        qc.m_uFlowControlThresh1 = queues->fromflowthresh[i];
        qc.m_uFlowControlThresh2 = queues->fromflowthresh[i];
        qc.m_uFlowControlEnable = !!queues->fromflowthresh[i];
        qc.m_uMinPages = queues->minfrompages[i];
        qc.m_uMaxPages =  queues->maxfrompages[i];
        CaQmQueueConfigWrite(userDeviceHandle, fromQid, &qc);
        CaQmQueueState0EnWrite(userDeviceHandle, fromQid, &qe);
        q2p.m_uPbSel = pbsel;
        q2p.m_uPpeBound = 1;
        q2p.m_uDestInterface = toIface;
        q2p.m_uDestPort = toPort;
        q2p.m_uPpeEnqueue = egress && !many2one;
        q2p.m_uDestQueue = toQid;
        CaPtQueueToPortWrite(userDeviceHandle, fromQid, &q2p);
        CaPrPortToQueueWrite(userDeviceHandle, fromIface, fromPort, fromQid);
        if (pbsel) {
            pb1PagesAvail -= qc.m_uMinPages;
        } else {
            pb0PagesAvail -= qc.m_uMinPages;
        }

        if (!many2one) {
            /* Configure the HPP->* queue */
            qc.m_uFlowControlTarget = SB_FE2000_QM_FLOW_CONTROL_TARGET_QUEUE;
            qc.m_uFlowControlPort = fromQid;
            qc.m_uMinPages = queues->mintopages[i];
            qc.m_uMaxPages =  queues->maxtopages[i];
            qc.m_uFlowControlThresh1 = queues->toflowthresh[i];
            qc.m_uFlowControlThresh2 = queues->toflowthresh[i];
            qc.m_uFlowControlEnable = !!queues->toflowthresh[i];
            CaQmQueueConfigWrite(userDeviceHandle, toQid, &qc);
            CaQmQueueState0EnWrite(userDeviceHandle, toQid, &qe);
            q2p.m_uPbSel = pbsel;
            q2p.m_uPpeBound = 0;
            q2p.m_uPpeEnqueue = 0;
            q2p.m_uDestQueue = 0;
            CaPtQueueToPortWrite(userDeviceHandle, toQid, &q2p);
            CaPtPortToQueueWrite(userDeviceHandle, toIface, toPort, toQid);
            if (pbsel) {
                pb1PagesAvail -= qc.m_uMinPages;
            } else {
                pb0PagesAvail -= qc.m_uMinPages;
            }
        }
    }

    qmConfig0 = SAND_HAL_MOD_FIELD(CA, QM_CONFIG0,
                                   TOTAL_MAX_PAGES_AVAILABLE_PB0, qmConfig0,
                                   pb0PagesAvail);
    qmConfig0 = SAND_HAL_MOD_FIELD(CA, QM_CONFIG0,
                                   TOTAL_MAX_PAGES_AVAILABLE_PB1, qmConfig0,
                                   pb1PagesAvail);
    SAND_HAL_WRITE(userDeviceHandle, CA,  QM_CONFIG0, qmConfig0);

    /* Miscellaneous PP & PD config */

    v = SAND_HAL_READ(userDeviceHandle, CA, PD_CONFIG);

    v = SAND_HAL_MOD_FIELD(CA,PD_CONFIG, TRUNC_LENGTH, v, 12 + 64);

    v = SAND_HAL_MOD_FIELD(CA,PD_CONFIG, CONDITIONAL_UPDATE, v, 1);
    v = SAND_HAL_MOD_FIELD(CA,PD_CONFIG, FRAME_LEN_ADJUST, v, 1);
    SAND_HAL_WRITE(userDeviceHandle, CA,  PD_CONFIG, v);

    v = SAND_HAL_READ(userDeviceHandle, CA, PD_DEBUG);
    v = SAND_HAL_MOD_FIELD(CA,PD_DEBUG,IPV4_HDR_TYPE, v, 0x7);
    SAND_HAL_WRITE(userDeviceHandle, CA, PD_DEBUG, v);

    /*
     * Setup Egress Exception - Mirror Table on PT
     * Setup Mirror Index 1/2 - which is used for mapping
     * egress exception packets based on destination (or)
     * source queue to either SPI0/SPI1 queue
     *
     * If there is only one mirror qid (e.g. because only 1 SPI is
     * system-facing), both mirrors go to the same queue
     */
    mirrorData = queues->mir0qid ? queues->mir0qid : queues->mir1qid;
    mirrorData |= 0x0300;
    if(sbFe2000UtilWriteIndir(userDeviceHandle, FALSE, TRUE,
                              SAND_HAL_CA_PT_MIRROR_INDEX_MEM_ACC_CTRL_OFFSET,
                              1, 1, &mirrorData)){
        return SB_FAILED;
    }
    mirrorData = queues->mir1qid ? queues->mir1qid : queues->mir0qid;
    mirrorData |= 0x0300;
    if(sbFe2000UtilWriteIndir(userDeviceHandle, FALSE, TRUE,
                              SAND_HAL_CA_PT_MIRROR_INDEX_MEM_ACC_CTRL_OFFSET,
                              2, 1, &mirrorData)){
        return SB_FAILED;
    }


    return SB_FE2000_STS_INIT_OK_K;
}

static int sb_max(int a, int b)
{
    return a > b ? a : b;
}

static int sb_min(int a, int b)
{
    return a < b ? a : b;
}

/*
 * Configure queue & related parameters based on params->queues
 *
 * line->HPP->system (i.e. ingress) queues use packet buffer 0
 * system->HPP->line (i.e. egress) queues use packet buffer 1
 */
sbStatus_t
sbFe2000SetQueueParams(int unit, sbFe2000InitParams_t *ip)
{
    int i, j, k, maxcal, egress, toIface, toPort, fromIface, fromPort;
    int totalports, iface, port, pbsel, owe, con, con0, totalpages;
    int maxpacket, freequeues, qid, lines, ports, group, queuesused, bwused;
    int portspergroup, groups;
    int portbw = 0;
    int ifbw[SB_FE2000_IF_PTYPE_MAX][2];
    int maxports[SB_FE2000_IF_PTYPE_MAX];
    int pbcnt[SB_FE2000_IF_PTYPE_MAX][2];
    int queuesbygroup[SWS_INGRESS_QUEUE_GROUPS];
    int bwbygroup[SWS_INGRESS_QUEUE_GROUPS];
    int spirlines[2], spitlines[2];
    sbFe2000Connection_t *c, *c0;
    sbFe2000Queues_t *queues = &ip->queues;
    int xe_count = 2;
#if FE2K_SPI_PORTS_TO_MAX_CONTIGOUS
    int maxSr[2], maxSt[2];
#endif

    maxports[SB_FE2000_IF_PTYPE_SPI0] = SB_FE2000_MAX_PORTS_PER_SPI;
    maxports[SB_FE2000_IF_PTYPE_SPI1] = SB_FE2000_MAX_PORTS_PER_SPI;
    maxports[SB_FE2000_IF_PTYPE_AGM0] = (xe_count <=2) ? SB_FE2000_MAX_AG_PORTS : SB_FE2000_MAX_XG_PORTS;
    /*    coverity[new_values]    */
    maxports[SB_FE2000_IF_PTYPE_AGM1] = (xe_count <=3) ? SB_FE2000_MAX_AG_PORTS : SB_FE2000_MAX_XG_PORTS;
    maxports[SB_FE2000_IF_PTYPE_XGM0] = SB_FE2000_MAX_XG_PORTS;
    maxports[SB_FE2000_IF_PTYPE_XGM1] = SB_FE2000_MAX_XG_PORTS;
    maxports[SB_FE2000_IF_PTYPE_PCI] = 1;
 
    queues->mir0qid = 0;
    queues->mir1qid = 0;
    queues->topciqid = 0;
    queues->frompciqid = 0;

    /* Initialize per-connection state */
    for (i = 0; i < SB_FE2000_SWS_INIT_MAX_CONNECTIONS; i++) {
        queues->qid2con[i] = SB_FE2000_SWS_UNINITIALIZED_CON;
        queues->many2onecon[i] = SB_FE2000_SWS_UNINITIALIZED_CON;
        queues->conbybw[i] = i;
        queues->bw[i] = 0;
    }

    /* Initialize the Port/Qid mappings */
    for (i =0; i < SB_FE2000_SWS_INIT_MAX_CONNECTIONS/2; i++) {
        queues->port2iqid[i] = SB_FE2000_SWS_UNINITIALIZED_QID;
        queues->port2eqid[i] = SB_FE2000_SWS_UNINITIALIZED_QID;
    }

    for (i = 0; i < SB_FE2000_IF_PTYPE_MAX; i++) {
        queues->ifports[i] = 0;
        queues->ifexpmode[i] = 0;
        for (j = 0; j < 2; j++) {
            pbcnt[i][j] = 0;
            ifbw[i][j] = 0;
        }
        for (j = 0; j < SB_FE2000_MAX_PORTS_PER_SPI; j++) {
            for (k = 0; k < 2; k++) {
                queues->if2con[i][j][k] = SB_FE2000_SWS_UNINITIALIZED_CON;
            }
        }
    }

    /*
     * Sanity check queue configuration
     * count ports per interface
     * construct interface to connection map
     */
    totalports = 0;
    for (i = 0; i < queues->n; i++) {
        c = &queues->connections[i];
        fromIface = c->from.ulInterfacePortType;
        fromPort = c->from.ulPortId;
        toIface = c->to.ulInterfacePortType;
        toPort = c->to.ulPortId;
        egress = c->bEgress;
        if (fromPort >= maxports[fromIface]) {
            SB_LOG("from port too large %d (%d/%d)\n", i, fromIface, fromPort);
            return SB_BAD_ARGUMENT_ERR_CODE;
        }
        if (toPort >= maxports[toIface]) {
            SB_LOG("to port too large %d (%d/%d)\n", i, toIface, toPort);
            return SB_BAD_ARGUMENT_ERR_CODE;
        }
        queues->many2onecon[i] = SB_FE2000_SWS_UNINITIALIZED_CON;
        for (j = 0; j < i; j++) {
            if (queues->many2onecon[i] == SB_FE2000_SWS_UNINITIALIZED_CON
                && toIface == queues->connections[j].to.ulInterfacePortType
                && toPort == queues->connections[j].to.ulPortId) {
                queues->many2onecon[i] = j;
            }
            if (fromIface == queues->connections[j].from.ulInterfacePortType
                && fromPort == queues->connections[j].from.ulPortId) {
                SB_LOG("Duplicate SWS connection %d & %d (%d/%d:%d.%d)\n",
                       i, j, fromIface, fromPort, c->ulUcodePort, c->bEgress);
                return SB_BAD_ARGUMENT_ERR_CODE;
            }
            if (c->ulUcodePort == queues->connections[j].ulUcodePort
                && c->bEgress == queues->connections[j].bEgress) {
                SB_LOG("Duplicate uCode flow in SWS connection"
                       " %d & %d (%d/%d:%d.%d)\n",
                       i, j, fromIface, fromPort, c->ulUcodePort, c->bEgress);
                return SB_BAD_ARGUMENT_ERR_CODE;
            }
        }
        if (egress) {
            queues->if2con[toIface][toPort][egress] = i;
        } else {
            queues->if2con[fromIface][fromPort][egress] = i;
            queues->ifports[fromIface]++;
            totalports++;
        }
    }

    /*
     * Compute (estimate, really) bandwidth for
     *   + each connection
     *   + each interface
     */
    for (i = 0; i < queues->n; i++) {
        c = &queues->connections[i];
        egress = c->bEgress;
        iface = egress
            ? c->to.ulInterfacePortType
            : c->from.ulInterfacePortType;
        switch (iface) {
        case SB_FE2000_IF_PTYPE_SPI0:
        case SB_FE2000_IF_PTYPE_SPI1:
            /* SPI front panel ports are low speed */
            /* portbw = 10000 / queues->ifports[iface]; */
            portbw = 100;
            break;
        case SB_FE2000_IF_PTYPE_AGM0:
            portbw = (xe_count <=2) ? 1000 : 10000;
            break;
        case SB_FE2000_IF_PTYPE_AGM1:
            portbw = (xe_count <=3) ? 1000 : 10000;
            break;
            break;
        case SB_FE2000_IF_PTYPE_XGM0:
        case SB_FE2000_IF_PTYPE_XGM1:
            portbw = 10000;
            break;
        case SB_FE2000_IF_PTYPE_PCI:
            portbw = 100;
        }
        queues->bw[i] += portbw;
        if (100 < portbw && portbw < 1000) {
            portbw = 1000;
        }
        ifbw[c->from.ulInterfacePortType][0] += portbw;
        ifbw[c->to.ulInterfacePortType][1] += portbw;
    }

    /*
     * Get a sorted list of connections by bandwidth, direction & ucode port
     */
    for (i = 0; i < queues->n; i++) {
        for (j = i + 1; j < queues->n; j++) {
            con = queues->conbybw[i];
            c = &queues->connections[con];
            con0 = queues->conbybw[j];
            c0 = &queues->connections[con0];
            if (queues->bw[con] < queues->bw[con0]
                || (queues->bw[con] == queues->bw[con0]
                    && c->bEgress > c0->bEgress)
                || (queues->bw[con] == queues->bw[con0]
                    && c->bEgress == c0->bEgress
                    && c->ulUcodePort < c0->ulUcodePort)) {
                queues->conbybw[j] = con;
                queues->conbybw[i] = con0;
            }
        }
    }

    freequeues = SB_FE2000_SWS_INIT_MAX_CONNECTIONS / 2 - totalports;

    for (i = 0; i < SWS_INGRESS_QUEUE_GROUPS; i++) {
        queuesbygroup[i] = SWS_QUEUES_PER_GROUP;
        bwbygroup[i] = 10000;
    }

    /* iterate though all queues */
    for (i = 0; i < queues->n; i++) {
        con = queues->conbybw[i];
        c = &queues->connections[con];
        if (c->bEgress) {
            continue;
        }
        /* count ports with similar bandwidth requirements */
        ports = 0;
        for (j = i; j < queues->n; j++) {
            con0 = queues->conbybw[j];
            c0 = &queues->connections[con0];
            if (!c0->bEgress) {
                if (queues->bw[con] != queues->bw[con0]) {
                    break;
                }
                ports++;
            }
        }
        SB_LOGV3("processing 0x%x ports at BW=0x%x\n", ports, queues->bw[con]);

        while (ports) {
            groups = sb_max(((ports * queues->bw[con]) + 5000) / 10000, 1);
            SB_LOGV3("    group required = 0x%x\n", groups);
            portspergroup = (ports + (groups - 1)) / groups;
            SB_LOGV3("    ports per group = 0x%x\n", portspergroup);
            group = SWS_INGRESS_QUEUE_GROUPS - 1;

            /* find the least utilized group */
            for (j = SWS_INGRESS_QUEUE_GROUPS - 2; j >= 0 ; j--) {
                /* group 3 has 5 reserved queues (4 drop, 1 locked) */
                if (group == 3 && (queuesbygroup[group] - portspergroup < 5)) {
                    group = j;
                } else if (queuesbygroup[j] > queuesbygroup[group]) {
                    /* check for available bw */
                    if ( (bwbygroup[j] - (portspergroup * queues->bw[con]) >= 0) ) {
                        group = j;
                    }
                }
            }
            SB_LOGV3("    least utilized group=0x%x with queues=0x%x, bw=0x%x\n", group, queuesbygroup[group], bwbygroup[group]);
            
            if ((bwbygroup[group] <= 0) || (!queuesbygroup[group]) ||
                ((bwbygroup[group] - (portspergroup * queues->bw[con]) <= 0) )) {
                /* no group with both queues and BW - oversubscription
                 * allocate based on queues only
                 */
                SB_LOGV3("    oversubscribed, looking for group based on queues alone\n");
                for (j = SWS_INGRESS_QUEUE_GROUPS - 2; j >= 0 ; j--) {
                    /* group 3 has 5 reserved queues (4 drop, 1 locked) */
                    if ((group == 3 && queuesbygroup[group] - portspergroup < 5)
                        || queuesbygroup[j] > queuesbygroup[group]) {
                        group = j;
                    }
                }
                SB_LOGV3("    least oversubscribed group=0x%x with queues=0x%x, bw=0x%x\n", group, queuesbygroup[group], bwbygroup[group]);
            }

            if (!queuesbygroup[group]) {
                return SB_BAD_ARGUMENT_ERR_CODE;
            }

            /* fit as many ports as possible into this group */
            portspergroup = sb_min(portspergroup, queuesbygroup[group]);
            qid = (group + 1) * SWS_QUEUES_PER_GROUP - queuesbygroup[group]
                + portspergroup - 1;
            for (; portspergroup; i++) {
                con = queues->conbybw[i];
                c = &queues->connections[con];
                if (c->bEgress) {
                    continue;
                }
                queues->fromqid[con] = qid;
                queues->toqid[con] = qid + TO_QUEUE_OFFSET;
                queues->qid2con[qid] = con;
                queues->port2iqid[c->ulUcodePort] = qid;

                SB_LOGV3("    iface: 0x%x, port: 0x%x --> qid: 0x%x\n", c->from.ulInterfacePortType,
                             c->from.ulPortId, queues->fromqid[con]);
                SB_LOGV3("    toqid: 0x%x --> iface:0x%x, port0x%x\n", queues->toqid[con],
                        c->to.ulInterfacePortType, c->to.ulPortId);
                if (c->to.ulInterfacePortType == SB_FE2000_IF_PTYPE_SPI0
                    && !queues->mir0qid) {
                    queues->mir0qid = queues->toqid[con];
                } else if (c->to.ulInterfacePortType == SB_FE2000_IF_PTYPE_SPI1
                           && !queues->mir1qid) {
                    queues->mir1qid = queues->toqid[con];
                }
                if (c->from.ulInterfacePortType == SB_FE2000_IF_PTYPE_PCI) {
                    queues->frompciqid = queues->fromqid[con];
                    SB_LOGV3("FROM FE PCI SWS qid: 0x%x\n",queues->frompciqid);
                }

                if (queues->bw[con] >= 10000) {
                    /* expanded mode FIFO uses only 8 queues. Remaining queues
                     * may be used when oversubscribed.
                     */
                    queuesused = sb_min(8, freequeues + 1);
                    bwused = 10000;
                    queues->ifexpmode[c->from.ulInterfacePortType]
                        = ( (queuesused == 8) && (! (qid & 0xf) ) );
                } else {
                    queuesused = 1;
                    bwused = queues->bw[con];
                }
                freequeues -= queuesused - 1;
                queuesbygroup[group] -= queuesused;
                bwbygroup[group] -= bwused;
                ports--;
                portspergroup--;
                qid--;
            }
        }
    }

    /*
     * Do egress queue assignment
     */
    for (i = 0; i < queues->n; i++) {
        c = &queues->connections[i];
        if (!c->bEgress) {
            continue;
        }
        if (queues->port2iqid[c->ulUcodePort]
            != SB_FE2000_SWS_UNINITIALIZED_QID) {
            /*
             * We currently only handle the case where every egress connection
             * has a corresponding ingress connection
             */
            qid = queues->port2iqid[c->ulUcodePort] + EGRESS_QUEUE_OFFSET;
            queues->port2eqid[c->ulUcodePort] = qid;
            queues->qid2con[qid] = i;
            queues->fromqid[i] = qid;
            queues->toqid[i] = qid + TO_QUEUE_OFFSET;
            if (c->to.ulInterfacePortType == SB_FE2000_IF_PTYPE_PCI) {
                queues->topciqid = queues->toqid[i];
                SB_LOGV3("To FE PCI SWS qid: 0x%x\n",queues->topciqid);
            }
        }
    }

    /*
     * Do packet buffer assignment
     */
    owe = 0;
    for (i = 0; i < queues->n; i++) {
        pbsel = queues->pbsel[i];
        if (pbsel >= 0) {
            owe += (pbsel ? 1 : -1) * queues->bw[i];
        }
    }

    for (i = 0; i < queues->n; i++) {
        con = queues->conbybw[i];
        c = &queues->connections[con];
        if (!c->bEgress && queues->pbsel[con] < 0) {
            pbsel = (owe < 0);
            queues->pbsel[con] = pbsel;
            owe += (pbsel ? 1 : -1) * queues->bw[con];
        }
    }

    /* Egress packet buffer assignment is opposite ingress */
    for (i = 0; i < queues->n; i++) {
        c = &queues->connections[i];
        if (c->bEgress) {
            toIface = c->to.ulInterfacePortType;
            toPort = c->to.ulPortId;
            con = queues->if2con[toIface][toPort][0];
            queues->pbsel[i] = !queues->pbsel[con];
        }
    }
    
    /*
     * Compute page allocation & flow control thresholds
     */

    totalpages = SWS_PAGES_PER_PB;
    maxpacket = (MAX_PACKET + SWS_BYTES_PER_PAGE - 1) / SWS_BYTES_PER_PAGE;

    /*
     * This will allocate fewer from-queue maximum pages than possible
     * when there are many2one connections (it assumes every connection
     * consumes tomin pages for a to-queue)
     */
    for (i = 0; i < queues->n; i++) {
        c = &queues->connections[i];
        egress = c->bEgress;
        iface = egress
            ? c->to.ulInterfacePortType
            : c->from.ulInterfacePortType;
        /* We assume the configuration is not oversubscribed on ingress */
        queues->minfrompages[i] = 0;
        queues->mintopages[i] = 0;
        queues->maxfrompages[i] = totalpages;
        queues->maxtopages[i] = totalpages;
        if (egress) {
            queues->toflowthresh[i] = queues->bw[i] * 3 / 100;
            queues->fromflowthresh[i] = maxpacket + queues->toflowthresh[i];
        } else {
            queues->fromflowthresh[i] = queues->maxfrompages[i];
            queues->toflowthresh[i] = queues->maxtopages[i];
        }
    }

    /*
     * Compute prefetch allocation
     */

    for (i = 0; i < queues->n; i++) {
        c = &queues->connections[i];
        iface = c->from.ulInterfacePortType;
        pbsel = queues->pbsel[i];
        pbcnt[iface][pbsel]++;
    }

    for (i = 0; i < SB_FE2000_IF_PTYPE_MAX; i++) {
        for (j = 0; j < 2; j++) {
            ip->pr.bEnablePbPrefetch[i][j] = !!pbcnt[i][j];
            switch (i) {
            case SB_FE2000_IF_PTYPE_SPI0:
            case SB_FE2000_IF_PTYPE_SPI1:
            case SB_FE2000_IF_PTYPE_XGM0:
            case SB_FE2000_IF_PTYPE_XGM1:
                ip->pr.uPbPrefetchPages[i][j] = pbcnt[i][j] ? 14 : 0;
                break;
            case SB_FE2000_IF_PTYPE_AGM0:
                if (xe_count <= 2) {
                    ip->pr.uPbPrefetchPages[i][j] =
                        pbcnt[i][j] > 4 ? 14 : pbcnt[i][j] * 2;
                }else{
                    ip->pr.uPbPrefetchPages[i][j] = pbcnt[i][j] ? 14 : 0;
                }
                break;
            case SB_FE2000_IF_PTYPE_AGM1:
                if (xe_count <= 3) {
                    ip->pr.uPbPrefetchPages[i][j] =
                        pbcnt[i][j] > 4 ? 14 : pbcnt[i][j] * 2;
                }else{
                    ip->pr.uPbPrefetchPages[i][j] = pbcnt[i][j] ? 14 : 0;
                }
                break;
            case SB_FE2000_IF_PTYPE_PCI:
                ip->pr.uPbPrefetchPages[i][j] = pbcnt[i][j];
            }
        }
    }


    /*
     * Allocate SPI FIFOs
     */
    spitlines[0] = SPI_MAX_LINES;
    spitlines[1] = SPI_MAX_LINES;
    spirlines[0] = SPI_MAX_LINES;
    spirlines[1] = SPI_MAX_LINES;

    /*
     * First, make sure all ports have at least 6 lines
     */
    for (i = 0; i < queues->n; i++) {
        con = queues->conbybw[i];
        c = &queues->connections[con];
        egress = c->bEgress;
        fromIface = c->from.ulInterfacePortType;
        fromPort = c->from.ulPortId;
        toIface = c->to.ulInterfacePortType;
        toPort = c->to.ulPortId;
        portbw = queues->bw[con];
        if (100 < portbw && portbw < 1000) {
            portbw = 1000;
        }
        switch (fromIface) {
        case SB_FE2000_IF_PTYPE_SPI0:
        case SB_FE2000_IF_PTYPE_SPI1:
            j = fromIface - SB_FE2000_IF_PTYPE_SPI0;
            lines = spirlines[j] * portbw / ifbw[fromIface][0];
            if (lines < SPI_MIN_LINES_PER_PORT) {
                ip->sr[j].nFifoLines[fromPort] = SPI_MIN_LINES_PER_PORT;
                spirlines[j] -= SPI_MIN_LINES_PER_PORT;
                ifbw[fromIface][0] -= portbw;
            }
        }

        switch (toIface) {
        case SB_FE2000_IF_PTYPE_SPI0:
        case SB_FE2000_IF_PTYPE_SPI1:
            j = toIface - SB_FE2000_IF_PTYPE_SPI0;
            lines = spitlines[j] * portbw / ifbw[toIface][1];
            if (lines < SPI_MIN_LINES_PER_PORT) {
                ip->st[j].nFifoLines[toPort] = SPI_MIN_LINES_PER_PORT;
                spitlines[j] -= SPI_MIN_LINES_PER_PORT;
                ifbw[toIface][1] -= portbw;
            }
        }
    }

    /*
     * Next, share out the remainder of the FIFO by bandwidth
     */
    for (i = 0; i < queues->n; i++) {
        con = queues->conbybw[i];
        c = &queues->connections[con];
        egress = c->bEgress;
        fromIface = c->from.ulInterfacePortType;
        fromPort = c->from.ulPortId;
        toIface = c->to.ulInterfacePortType;
        toPort = c->to.ulPortId;
        portbw = queues->bw[con];
        if (100 < portbw && portbw < 1000) {
            portbw = 1000;
        }
        switch (fromIface) {
        case SB_FE2000_IF_PTYPE_SPI0:
        case SB_FE2000_IF_PTYPE_SPI1:
            j = fromIface - SB_FE2000_IF_PTYPE_SPI0;
            if (ip->sr[j].nFifoLines[fromPort] == 0) {
                lines = 
                    sb_min(spirlines[j] * portbw / ifbw[fromIface][0],
                           SPI_MAX_LINES_PER_PORT);
                ip->sr[j].nFifoLines[fromPort] = lines;
                spirlines[j] -= lines;
                ifbw[fromIface][0] -= portbw;
            }
        }
        switch (toIface) {
        case SB_FE2000_IF_PTYPE_SPI0:
        case SB_FE2000_IF_PTYPE_SPI1:
            j = toIface - SB_FE2000_IF_PTYPE_SPI0;
            if (ip->st[j].nFifoLines[toPort] == 0) {
                lines = 
                    sb_min(spitlines[j] * portbw / ifbw[toIface][1],
                           SPI_MAX_LINES_PER_PORT);
                ip->st[j].nFifoLines[toPort] = lines;
                spitlines[j] -= lines;
                ifbw[toIface][1] -= portbw;
            }
        }
    }

    /*
     * Compute SPI calendars
     * Configure PB assignments
     */
    ip->sr[0].nCalendarLength = 0;
    ip->sr[1].nCalendarLength = 0;
    ip->st[0].nCalendarLength = 0;
    ip->st[1].nCalendarLength = 0;
#if FE2K_SPI_PORTS_TO_MAX_CONTIGOUS
    maxSr[0] = maxSr[1] = 0;
    maxSt[0] = maxSt[1] = 0;
#endif

    for (i = 0; i < queues->n; i++) {
        c = &queues->connections[i];
        egress = c->bEgress;
        iface = c->from.ulInterfacePortType;
        port = c->from.ulPortId;
        pbsel = queues->pbsel[i];
        switch (iface) {
        case SB_FE2000_IF_PTYPE_SPI0:
        case SB_FE2000_IF_PTYPE_SPI1:
            iface -= SB_FE2000_IF_PTYPE_SPI0;
            if (iface) {
                ip->pr.nPre1_PacketBufferSelect[port] = pbsel;
            } else {
                ip->pr.nPre0_PacketBufferSelect[port] = pbsel;
            }
            j = ip->sr[iface].nCalendarLength;
            ip->sr[iface].nCalendarEntry[j] = port;
            ip->sr[iface].nCalendarLength = j + 1;
#if FE2K_SPI_PORTS_TO_MAX_CONTIGOUS
            if (port > maxSr[iface]) {
                maxSr[iface] = port;
            }
#endif
            break;
        case SB_FE2000_IF_PTYPE_AGM0:
            iface -= SB_FE2000_IF_PTYPE_AGM0;
            if (xe_count <= 2) {
                if (iface) {
                    ip->pr.nPre3_PacketBufferSelect[port] = pbsel;
                } else {
                    ip->pr.nPre2_PacketBufferSelect[port] = pbsel;
                }
                ip->ag[iface].common[port].bBringUp = TRUE;
            }else{
                if (iface) {
                    ip->pr.nPre3_PacketBufferSelect[port] = pbsel;
                } else {
                    ip->pr.nPre2_PacketBufferSelect[port] = pbsel;
                }
            }
            break;
        case SB_FE2000_IF_PTYPE_AGM1:
            iface -= SB_FE2000_IF_PTYPE_AGM0;
            if (xe_count <= 3) {
                if (iface) {
                    ip->pr.nPre3_PacketBufferSelect[port] = pbsel;
                } else {
                    ip->pr.nPre2_PacketBufferSelect[port] = pbsel;
                }
                ip->ag[iface].common[port].bBringUp = TRUE;
            }else{
                if (iface) {
                    ip->pr.nPre3_PacketBufferSelect[port] = pbsel;
                } else {
                    ip->pr.nPre2_PacketBufferSelect[port] = pbsel;
                }
            }
            break;
        case SB_FE2000_IF_PTYPE_XGM0:
        case SB_FE2000_IF_PTYPE_XGM1:
            iface -= SB_FE2000_IF_PTYPE_XGM0;
            if (iface) {
                ip->pr.nPre5_PacketBufferSelect = pbsel;
            } else {
                ip->pr.nPre4_PacketBufferSelect = pbsel;
            }
            break;
        case SB_FE2000_IF_PTYPE_PCI:
            break;
        }
        iface = c->to.ulInterfacePortType;
        port = c->to.ulPortId;
        switch (iface) {
        case SB_FE2000_IF_PTYPE_SPI0:
        case SB_FE2000_IF_PTYPE_SPI1:
            iface -= SB_FE2000_IF_PTYPE_SPI0;
            j = ip->st[iface].nCalendarLength;
            ip->st[iface].nCalendarEntry[j] = port;
            ip->st[iface].nCalendarLength = j + 1;   
#if FE2K_SPI_PORTS_TO_MAX_CONTIGOUS
            if (port > maxSt[iface]) {
                maxSt[iface] = port;
            }
#endif

         break;
        }
    }


#if FE2K_SPI_PORTS_TO_MAX_CONTIGOUS
    /* reconfigure the spi calendar to be a sequential list from 0 to max
     * configured subport.  This re-ordering is necessary for user defined
     * ucode ports where ports may not be contigous because the QE is highly
     * dependant on the spi subport numbering scheme for both calendar
     * and fabric port numbering.
     */
    if (soc_property_get(unit, spn_UCODE_NUM_PORTS, 0)) {
        for (iface=0; iface < 2; iface++) {
            for (i=0; i <= maxSt[iface]; i++) {
                ip->st[iface].nCalendarEntry[i] = i;
            }
            ip->st[iface].nCalendarLength =  maxSt[iface] + 1;
        }
        
        for (iface=0; iface < 2; iface++) {
            for (i=0; i <= maxSr[iface]; i++) {
                ip->sr[iface].nCalendarEntry[i] = i;
            }
            ip->sr[iface].nCalendarLength = maxSr[iface] + 1;
        }
    }
#endif /* FE2K_SPI_PORTS_TO_MAX_CONTIGOUS */
    
    ip->xg[0].bBringUp = TRUE;
    ip->xg[1].bBringUp = TRUE;
    ip->ag[0].bBringUp = (xe_count <= 2) ? TRUE : FALSE;
    ip->ag[1].bBringUp = (xe_count <= 3) ? TRUE : FALSE;
    ip->xg[2].bBringUp = (xe_count <= 2) ? FALSE : TRUE;
    ip->xg[3].bBringUp = (xe_count <= 3) ? FALSE : TRUE;
    
    /* Make RX & TX calendars equal in length (for QE). */
    /* TODO: remove this by fixing QE impl (properties for asymmetric cal?) */
    for (i = 0; i < 2; i++) {
        if (ip->sr[i].nCalendarLength < ip->st[i].nCalendarLength) {
            maxcal = ip->st[i].nCalendarLength
                - ip->sr[i].nCalendarLength;
            for (j = 0; j < maxcal; j++) {
                ip->sr[i]
                    .nCalendarEntry[ip->sr[i].nCalendarLength + j] =
                    ip->sr[i]
                    .nCalendarEntry[j % ip->sr[i].nCalendarLength];
            }
            ip->sr[i].nCalendarLength = ip->st[i].nCalendarLength;
        } else {
            maxcal = ip->sr[i].nCalendarLength
                - ip->st[i].nCalendarLength;
            for (j = 0; j < maxcal; j++) {
                ip->st[i]
                    .nCalendarEntry[ip->st[i].nCalendarLength + j] =
                    ip->st[i]
                    .nCalendarEntry[j % ip->st[i].nCalendarLength];
            }
            ip->st[i].nCalendarLength = ip->sr[i].nCalendarLength;
        }
    }

    /* Configure PPE batch groups */
    ip->pp.bHashRequired = FALSE;
    ip->pd.bRouteHeaderPresent = FALSE;

    ip->pp.PpByPassLrpModeConfiguration.bByPassLrpMode[0]
        = ip->lr.bLrpBypass;
    ip->pp.bSourceBufferInsert[0] = FALSE;
    ip->pp.bAddRouteHeader[0] = FALSE;
    ip->pp.bDeleteFirstHeader[0] = FALSE;

    ip->pp.PpByPassLrpModeConfiguration.bByPassLrpMode[1]
        = ip->lr.bLrpBypass;
    ip->pp.bSourceBufferInsert[1] = FALSE;
    ip->pp.bAddRouteHeader[1] = FALSE;
    ip->pp.bDeleteFirstHeader[1] = FALSE;

    ip->pp.uDefaultNumberofQueuesToConfigure = 128;
    for (i = 0; i < ip->pp.uDefaultNumberofQueuesToConfigure; i++) {
        egress = (64 <= i && i < 128);
    ip->pp.QueueConfiguration[i].uPriority = 0;
    ip->pp.QueueConfiguration[i].uBatchGroup = egress;
    ip->pp.QueueInitialState[i].uHeaderType = egress ? 0 : 2;
    }

    return SB_OK;
}

sbStatus_t
sbFe2000SetPortPages(sbhandle userDeviceHandle,
                     sbFe2000InitParams_t *ip,
                     int port, int mbps, int mtu)
{
    sbFe2000Queues_t *queues = &ip->queues;
    int mtupages = (mtu + SWS_BYTES_PER_PAGE - 1) / SWS_BYTES_PER_PAGE;
    int ratepages = mbps * 3 / 100;
    int eqid = queues->port2eqid[port];
    int etoqid = eqid + TO_QUEUE_OFFSET;
    int con;
    sbZfFe2000QmQueueConfigEntry_t qc;

    if (eqid == SB_FE2000_SWS_UNINITIALIZED_QID) {
        return SB_BAD_ARGUMENT_ERR_CODE;
    }

    con = queues->qid2con[eqid];
    queues->fromflowthresh[con] = ratepages + mtupages;
    queues->toflowthresh[con] = ratepages;

    CaQmQueueConfigRead(userDeviceHandle, etoqid, &qc);
    qc.m_uFlowControlThresh1 = queues->toflowthresh[con];
    qc.m_uFlowControlThresh2 = qc.m_uFlowControlThresh1;
    CaQmQueueConfigWrite(userDeviceHandle, etoqid, &qc);

    CaQmQueueConfigRead(userDeviceHandle, eqid, &qc);
    qc.m_uFlowControlThresh1 = queues->fromflowthresh[con];
    qc.m_uFlowControlThresh2 = qc.m_uFlowControlThresh1;
    CaQmQueueConfigWrite(userDeviceHandle, eqid, &qc);

    return SB_OK;
}

static int
sbFe2000PpQueueConfig(sbhandle userDeviceHandle,
                      uint32_t uQueue,
                      uint32_t uBatch,
                      uint32_t uPriority)
{
      sbZfFe2000PpQueuePriorityGroupEntry_t zQueueConfig;
      sbZfFe2000PpQueuePriorityGroupEntry_InitInstance(&zQueueConfig);
      zQueueConfig.m_uPriority = uPriority;
      zQueueConfig.m_uBatchGroup = uBatch;

      if( CaPpQueuePriorityGroupWrite(userDeviceHandle, uQueue, &zQueueConfig) ) {
          return 0;
      } else {
        SB_ERROR("%s Failed on write for queue(%d).",__PRETTY_FUNCTION__, uQueue);
          return -1;
      }
}

int
sbFe2000IsBatchSupported(uint32_t unit)
{
    return 0;
}

uint8_t
sbFe2000MaxBatch(uint32_t unit)
{
    if(sbFe2000IsBatchSupported(unit)){
        return SOC_SBX_CFG_FE2000(unit)->maxBatch;
    } else {
        return -1;
    }
}

#define DEBUG_PORT_BATCH_ASSGN
sbStatus_t
sbFe2000InitBatch(sbhandle userDeviceHandle,
                  uint32_t unit,
                  sbFe2000InitParams_t *ip,
                  sbFe2000InitPortBatchMap_t *pBatchMap,
                  uint32_t *pMaxBatch)
{
    sbFe2000Queues_t *pQueues = &ip->queues;
    sbFe2000InitParamsPp_t *pp = &ip->pp;
    sbFe2000Connection_t    *pConn;
    pbmp_t                  ge_pbm, xe_pbm;
    int                     geCount=0, xeCount=0, xGeBlocks=0;
    int                     index, bIndex, fromQid, toQid;
    int                     bandWidth, perBatchBw, deltaBw;
    int                     xeThisBatch, geThisBatch, updatePp, pciThisBatch;

    *pMaxBatch = 0;

    /* invalidate all Batch groups */
    for(index=0; index < SB_G2P3_MAX_BATCH_SUPPORTED; index++){
        SOC_PBMP_CLEAR(pBatchMap[index].pbm);
        pBatchMap[index].uValid = 0;
        pBatchMap[index].uBatchNumber = index;
    }

    for(index=0; index < SB_FE2000_SWS_INIT_MAX_CONNECTIONS; index++){
        pQueues->batchAssigned[index] = 0;
    }

    SOC_PBMP_ASSIGN(ge_pbm, PBMP_PORT_ALL(unit));

    /* Balance Bandwidth between available GE/XE port */
    SOC_PBMP_ASSIGN(ge_pbm, PBMP_GE_ALL(unit));
    SOC_PBMP_COUNT(ge_pbm, geCount);
    SOC_PBMP_ASSIGN(xe_pbm, PBMP_XE_ALL(unit));
    SOC_PBMP_COUNT(xe_pbm, xeCount);

    /* The below logic groups bandwith based on xe or ge
     * Only XE, GE are considered. HG is not considered
     * (1) intially total bandwidth available is considered and maximum batches
     *     it can be segregated is computed
     *
     * (2) 10G ports are divided to individual batches. 1G total BW is divided into
     *     10G slots. The delta bandwidth is tried to be evenly shuffled between total
     *     number of XE, GE 10G blocks.
     *
     * (3) Step 2 is repeated until its feasible to evenly distribute bandwith. If not
     *     only 1 batch is used
     */

    /* step 1 */
    bandWidth = 10 * xeCount + geCount;
    deltaBw   = 0;

    for(index=SB_G2P3_MAX_BATCH_SUPPORTED; index > 0; index--){
        if(bandWidth % index == 0){
            *pMaxBatch = index;
            break;
        }
    }

    /* step 2,3 */
    if(xeCount){
        deltaBw = geCount%10;

        for(index = *pMaxBatch; index > 1; index--){

            if(deltaBw % index){
                if((geCount - deltaBw) >= 10){
                  deltaBw += 10;
                }
            } else {
                break;
            }
        }
        *pMaxBatch = index;
    }

    /* Number of aggregated 1G blocks apart from XG blocks */
    xGeBlocks = (geCount - deltaBw)/10;
    perBatchBw = bandWidth/(*pMaxBatch);

    /* Assign geCount/maxBatch & xeCount/maxBatch into one Batch */
    /* Even for ingress, odd for egress */
    for(bIndex=0; bIndex < *pMaxBatch; bIndex++){

        /* Based on Per Batch BW determine how many XE +/ Gig ports
         * must be a member of this batch.
         * Divide XE / GE ports such a way that maximum number of ports
         * get accomodated on each Batch. Doing this way fairly increases
         * overall rules distributed for each ports as a system
         */
        xeThisBatch = 0;
        geThisBatch = 0;
        pciThisBatch = 0;
        deltaBw = perBatchBw;

        while(deltaBw/10){
            if(xeCount && (xeCount - xeThisBatch)){
                xeThisBatch++;
                xeCount--;
            } else if (xGeBlocks && (xGeBlocks - geThisBatch)){
                geThisBatch += 10;
                xGeBlocks--;
            } else {
                break;
            }
            deltaBw -= 10;
        }

        geThisBatch += deltaBw;

        pBatchMap[bIndex].uValid = 1;
        pBatchMap[bIndex].uBatchNumber = bIndex;
        /* always default PCI to batch 0 */
        if(bIndex == 0){
            pciThisBatch = 1;
        }

#ifdef DEBUG_PORT_BATCH_ASSGN
        soc_cm_print("\n### Batch[%d] thisxe[%d] thisge[%d]  xe[%d]###",\
                      bIndex, xeThisBatch, geThisBatch, xeCount);
#endif

        /* Walk through  Connections and Map HPP facing queues
         * to ingress and egress batches */
        for (index = 0; index < pQueues->n; index++) {
            pConn  = &pQueues->connections[index];
            fromQid = pQueues->port2iqid[pConn->ulUcodePort];
            toQid   = pQueues->port2eqid[pConn->ulUcodePort];
            updatePp = 0;

#ifdef DEBUG_PORT_BATCH_ASSGN
            soc_cm_print("\n Port[%d] IfType[%d] Egress[%d] Assigned[%d] ",
                   pConn->ulUcodePort, pConn->from.ulInterfacePortType,
                   pConn->bEgress, pQueues->batchAssigned[index]);
#endif

            if((!pConn->bEgress) && (!pQueues->batchAssigned[index])){
                switch(pConn->from.ulInterfacePortType){
                    case SB_FE2000_IF_PTYPE_XGM0:
                    case SB_FE2000_IF_PTYPE_XGM1:
                        if(xeThisBatch){
                          xeThisBatch--;
                          updatePp = 1;
                        }
                        break;

                    case SB_FE2000_IF_PTYPE_AGM0:
                    case SB_FE2000_IF_PTYPE_AGM1:
                        if(geThisBatch){
                          geThisBatch--;
                          updatePp = 1;
                        }
                        break;

                    
                    case SB_FE2000_IF_PTYPE_PCI:
                          pp->QueueConfiguration[fromQid].uBatchGroup = (bIndex << 1);
                          pp->QueueConfiguration[toQid].uBatchGroup = \
                               pp->QueueConfiguration[fromQid].uBatchGroup + 1;
                          pp->QueueConfiguration[fromQid].uPriority = 0;
                          pp->QueueConfiguration[toQid].uPriority = 0;
                          SOC_PBMP_PORT_ADD(pBatchMap[bIndex].pbm, pConn->ulUcodePort);
                          sbFe2000PpQueueConfig(userDeviceHandle, fromQid,
                                          pp->QueueConfiguration[fromQid].uBatchGroup,
                                          pp->QueueConfiguration[fromQid].uPriority);
                          sbFe2000PpQueueConfig(userDeviceHandle, toQid,
                                          pp->QueueConfiguration[toQid].uBatchGroup,
                                          pp->QueueConfiguration[toQid].uPriority);
                          updatePp = 0;
                          pQueues->batchAssigned[index] = 1;
                          pciThisBatch = 0;
                        break;

                    default:
                        updatePp = 0;
                        break;
                }

                if(updatePp){
                    pp->QueueConfiguration[fromQid].uBatchGroup = \
                                                          bIndex * 2;
                    pp->QueueConfiguration[toQid].uBatchGroup = \
                        pp->QueueConfiguration[fromQid].uBatchGroup + 1;
                    pp->QueueConfiguration[fromQid].uPriority = 0;
                    pp->QueueConfiguration[toQid].uPriority = 0;
                    SOC_PBMP_PORT_ADD(pBatchMap[bIndex].pbm, pConn->ulUcodePort);
                    sbFe2000PpQueueConfig(userDeviceHandle, fromQid,
                                          pp->QueueConfiguration[fromQid].uBatchGroup,
                                          pp->QueueConfiguration[fromQid].uPriority);
                    sbFe2000PpQueueConfig(userDeviceHandle, toQid,
                                          pp->QueueConfiguration[toQid].uBatchGroup,
                                          pp->QueueConfiguration[toQid].uPriority);
                    updatePp = 0;
                    pQueues->batchAssigned[index] = 1;
#ifdef DEBUG_PORT_BATCH_ASSGN
                    soc_cm_print("\n *** Assigning: Port[%d] IfType[%d] xe[%d] ge[%d] ***", \
                               pConn->ulUcodePort, \
                               pConn->from.ulInterfacePortType,
                               xeThisBatch, geThisBatch);
                    soc_cm_print("\n PPE QUEUE NO FROM[%d] TO[%d] ", fromQid, toQid);

#endif
                }
                if((geThisBatch==0) && (xeThisBatch==0) && (pciThisBatch == 0)){
                    soc_cm_print("\n#### End of batch loop ###\n");
                    break;
                }
            }
        }
    }
    return SB_OK;
}
