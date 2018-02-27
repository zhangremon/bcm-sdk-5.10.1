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
 * $Id: sbFe2000XtInit.c 1.116.6.2 Broadcom SDK $
 * ******************************************************************************/

#include "glue.h"
#include "soc/drv.h"
#include "soc/sbx/sbx_drv.h"
#include "hal_user.h"
#include "hal_ca_auto.h"
#include "hal_c2_auto.h"
#include "hal_ca_c2.h"
#include "sbWrappers.h"
#include "sbFe2000XtInit.h"
#include "sbFe2000CommonUtil.h"
#include "sbFe2000UcodeLoad.h"
#include "sbFe2000XtInitUtils.h"
#include "sbFe2000XtImfDriver.h"
#ifndef __KERNEL__
#include "math.h"
#endif
#include <soc/sbx/fe2kxt/sbZfC2Includes.h>
#include <soc/sbx/sbx_txrx.h>
#include <soc/sbx/fe2000_spi.h>
#include <fe2000.h>
#include <fe2kdiag_vcdl_phase0_ucode.h>
#include <fe2kdiag_vcdl_phase1_ucode.h>
#include <fe2kdiag_vcdl_phase2_ucode.h>
#include <fe2kdiag_clearcc_ucode.h>
#include <c2_g2p3_ucode.h>
#include <soc/debug.h>

/* Never use certain defines */
#undef SB_FE2000_NUM_XG_INTERFACES

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

#define FE2K_MAX_PVT_COMPENSATION_ATTEMPTS 20

/* forward declarations */
static uint32_t CalculateLrLoaderEpoch(sbFe2000XtInitParams_t *pInitParams);
static uint32_t sbFe2000XtInitRegDone(sbhandle userDeviceHandle, uint32_t uCtlReg,uint32_t uDone, uint32_t uTimeOut);
static uint32_t sbFe2000XtInitQm(sbhandle userDeviceHandle, sbFe2000XtInitParams_t *pInitParams);
static void sbFe2000XtInitPb(sbhandle userDeviceHandle, sbFe2000XtInitParams_t *pInitParams);
static void sbFe2000XtInitPd(sbhandle userDeviceHandle, sbFe2000XtInitParams_t *pInitParams);
static void sbFe2000XtInitCm(sbhandle userDeviceHandle, sbFe2000XtInitParams_t *pInitParams);
static void sbFe2000XtInitRc(uint32_t uInstance,sbhandle userDeviceHandle, sbFe2000XtInitParams_t *pInitParams);
static uint32_t sbFe2000XtInitMm(uint32_t uInstance,sbhandle userDeviceHandle, sbFe2000XtInitParams_t *pInitParams);
static void sbFe2000XtInitPm(sbhandle userDeviceHandle, sbFe2000XtInitParams_t *pInitParams);
static uint32_t sbFe2000XtInitLr(sbhandle userDeviceHandle, sbFe2000XtInitParams_t *pInitParams);
static uint32_t sbFe2000XtInitCoCounters(sbhandle userDeviceHandle, sbFe2000XtInitParams_t *pInitParams);
static uint32_t sbFe2000XtInitPt(sbhandle userDeviceHandle, sbFe2000XtInitParams_t *pInitParams);
static uint32_t sbFe2000XtInitPr(sbhandle userDeviceHandle, sbFe2000XtInitParams_t *pInitParams);
static uint32_t sbFe2000XtInitPp(sbhandle userDeviceHandle, sbFe2000XtInitParams_t *pInitParams);
static uint32_t sbFe2000XtInitSpi(sbhandle userDeviceHandle, uint16_t nNum, sbFe2000XtInitParams_t *pInitParams);
static uint32_t sbFe2000XtInitSpiTx(sbhandle userDeviceHandle, uint16_t nSpi, sbFe2000XtInitParams_t *pInitParams);
static uint32_t sbFe2000XtInitSpiRx(sbhandle userDeviceHandle, uint16_t nSpi, sbFe2000XtInitParams_t *pInitParams);
static uint32_t sbFe2000XtInitXgm(sbhandle userDeviceHandle, uint16_t nMac, sbFe2000XtInitParams_t *pInitParams);
static uint32_t sbFe2000XtInitAgmPortRx(sbhandle userDeviceHandle, uint16_t nAgmNum, uint16_t nPort, sbFe2000XtInitParams_t *pInitParams);
static uint32_t sbFe2000XtInitAgmPortTx(sbhandle userDeviceHandle, uint16_t nAgmNum, uint16_t nPort, sbFe2000XtInitParams_t *pInitParams);
static int agmMacConfigCalcAddr(sbhandle userDeviceHandle, int nAgmNum, int nPort);
static uint32_t sbFe2000XtInitAgm(sbhandle userDeviceHandle, uint16_t nAgmNum, sbFe2000XtInitParams_t *pInitParams);
static uint32_t sbFe2000XtInitAgms(sbhandle userDeviceHandle, sbFe2000XtInitParams_t *pInitParams);
static uint32_t sbFe2000XtTakeXgXsOutOfReset(sbhandle userDeviceHandle, uint32_t nMac);
static void sbFe2000XtInitCleanUp(sbhandle userDeviceHandle, sbFe2000XtInitParams_t *pInitParams);
static void sbFe2000XtInitCleanUpPt(sbhandle userDeviceHandle);
static void sbFe2000XtInitCleanUpQm(sbhandle userDeviceHandle);
static void sbFe2000XtInitCleanUpPb(sbhandle userDeviceHandle);
static void sbFe2000XtInitCleanUpPr(sbhandle userDeviceHandle);
static void sbFe2000XtInitCleanUpPp(sbhandle userDeviceHandle);
static void sbFe2000XtInitCleanUpPd(sbhandle userDeviceHandle);
static void sbFe2000XtInitCleanUpMm(sbhandle userDeviceHandle, uint16_t nNum);
static void sbFe2000XtInitCleanUpCm(sbhandle userDeviceHandle);
static void sbFe2000XtInitCleanUpRc(sbhandle userDeviceHandle, uint16_t nNum);
static void sbFe2000XtInitCleanUpPm(sbhandle userDeviceHandle);
static void sbFe2000XtInitCleanUpLr(sbhandle userDeviceHandle, sbFe2000XtInitParams_t *pInitParams);
static void sbFe2000XtInitCleanUpAgm(sbhandle userDeviceHandle, uint16_t nNum);
static void sbFe2000XtInitCleanUpXgm(sbhandle userDeviceHandle, uint16_t nNum);
static void sbFe2000XtInitCleanUpSpi4(sbhandle userDeviceHandle, uint16_t nNum);
static void sbFe2000XtInitCleanUpAgmPortRx(uint16_t nNum, uint16_t nPort, sbhandle userDeviceHandle);
static void sbFe2000XtInitCleanUpAgmPortTx(uint16_t nNum, uint16_t nPort, sbhandle userDeviceHandle);
static uint32_t sbFe2000XtInitPLL(sbhandle userDeviceHandle,sbFe2000XtInitParams_t *pInitParams);
static uint32_t sbFe2000XtInitPsyncSlipBuffers(sbhandle userDeviceHandle,sbFe2000XtInitParams_t *pInitParams);

#include <soc/sbx/fe2k/sbZfCaDiagUcodeCtl.hx>
#include <soc/sbx/fe2k/sbZfCaDiagUcodeShared.hx>
#define RAM_SIZE_BASE  17
#define GBITN(x,n) (((x) >> n) & 0x1)

#define UNIT_FROM_HANDLE(_h_) ( (uint32_t)_h_)
extern uint8_t g_bSimRun;
#include <sal/core/libc.h>

/* local functions for parsing VCDL parameters */
/* local_strspn */
size_t (local_strspn)(const char *s1, const char *s2)
{
    const char *sc1;
    for (sc1 = s1; *sc1 != '\0'; sc1++)
        if (strchr(s2, *sc1) == NULL)
            return (sc1 - s1);
    return sc1 - s1;            /* terminating nulls don't match */
}

/* local_strcspn */
size_t (local_strcspn)(const char *s1, const char *s2)
{
    const char *sc1;
    for (sc1 = s1; *sc1 != '\0'; sc1++)
        if (strchr(s2, *sc1) != NULL)
            return (sc1 - s1);
    return sc1 - s1;            /* terminating nulls match */
}
/* local_strtok_r */
char *(local_strtok_r)(char *s, const char *delimiters, char **lasts)
{
    char *sbegin, *send;
    sbegin = s ? s : *lasts;
    sbegin += local_strspn(sbegin, delimiters);
    if (*sbegin == '\0') {
        *lasts = "";
        return NULL;
    }
    send = sbegin + local_strcspn(sbegin, delimiters);
    if (*send != '\0')
        *send++ = '\0';
    *lasts = send;
    return sbegin;
}
/* local_strtok */
char *(local_strtok)(char *s1, const char *delimiters)
{
    static char *ssave = "";
    return local_strtok_r(s1, delimiters, &ssave);
}

#define _GET_NEXT_VALUE(_str, _v)         \
    { char *token = local_strtok(_str, ",");    \
    if (token == NULL) {                  \
        SB_ASSERT(0);                     \
        return SOC_E_PARAM;               \
    }                                     \
    _v = sal_ctoi(token,0);               \
    }

void
sbFe2000XtInitDefaultParams(int unit, sbFe2000XtInitParams_t *pInitParams)
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
    sbFe2000XtInitParamsMm_t *pInstance;
    sbFe2000XtInitParamsMmDdrConfig_t *pDdrConfig;
    int  nSegment;
    uint32_t uBaseAddr;
    int nProfile;
    int nGroup;
    uint32_t uStreamIndex;
    uint32_t uRce, intf;
    int i, j;
    sbFe2000Connection_t *con;

    pInitParams->bBringUp = TRUE;
    pInitParams->pci.bBringUp = TRUE;
    pInitParams->pci.bDoSoftReset = TRUE;
    pInitParams->uDdr2MemorySelect = SB_FE2000XT_DDR2_PLUS_25;
#ifdef BROADCOM_SVK
    pInitParams->bDVTboard = FALSE;
#endif

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

    for ( nXgmii = 0; nXgmii < SB_FE2000XT_NUM_XG_INTERFACES; nXgmii++ ) {
        pInitParams->xg[nXgmii].bBringUp = FALSE;
        pInitParams->xg[nXgmii].bSerdesLocalLoopback = FALSE;
        pInitParams->xg[nXgmii].bLocalLoopback = FALSE;
        pInitParams->xg[nXgmii].bRemoteLoopback = FALSE;
        pInitParams->xg[nXgmii].nXgmiiSpeed = 10;
        pInitParams->xg[nXgmii].nE2eccBitmap = 0xFF; 
        pInitParams->xgt[nXgmii].bE2eccEnable = FALSE; 
        pInitParams->xgt[nXgmii].uE2eccPeriod = 0x100; 
        pInitParams->xgt[nXgmii].uE2eccNumPorts = 40; 
        pInitParams->xgt[nXgmii].uE2eccEtherType = 0x7000; 
        pInitParams->xgt[nXgmii].uE2eccCtlOp = 0x0000; 
        pInitParams->xgt[nXgmii].uE2eccFrameLength = 0x50;
        pInitParams->xgt[nXgmii].uE2eccPortEnable = 0xFFF;
        pInitParams->xgt[nXgmii].uuE2eccMacDa = 0xFFFFFFFFFFFFULL; 
        pInitParams->xgt[nXgmii].uuE2eccMacSa = 0x000000000000ULL;
        pInitParams->xgt[nXgmii].nMaxSize = 16000;
        /* translate from XG index to IF */
        if (nXgmii < 2) {
            intf = SB_FE2000_IF_PTYPE_XGM0 + nXgmii;
        }else{
            intf = SB_FE2000_IF_PTYPE_AGM0 + nXgmii - 2;
        }
        if (SOC_SBX_CFG_FE2000_IF_IS_HIGIG(unit, intf)) {
            pInitParams->xgt[nXgmii].nHeaderMode = 1;
            pInitParams->xgt[nXgmii].bPauseEnable = 0;
        }else {
            pInitParams->xgt[nXgmii].nHeaderMode = 0;
            pInitParams->xgt[nXgmii].bPauseEnable = 1;
        }
        pInitParams->xgt[nXgmii].nCrcMode = SB_FE2000_XG_MAC_CRC_REPLACE;
        pInitParams->xgt[nXgmii].nAvgIpg = 12;
        pInitParams->xgt[nXgmii].nThrotNumer = 0;
        pInitParams->xgt[nXgmii].nThrotDenom = 0;
        pInitParams->xgt[nXgmii].bDiscard = 0;
        pInitParams->xgt[nXgmii].bAnyStart = 0;
        pInitParams->xgt[nXgmii].nHiGig2Mode = 0;
        pInitParams->xgr[nXgmii].nMaxSize = 16000;
        pInitParams->xgr[nXgmii].bStripCrc = TRUE;
        pInitParams->xgr[nXgmii].bIgnoreCrc = 0;
        pInitParams->xgr[nXgmii].bStrictPreamble = 1;
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
            = SB_FE2000XT_MAX_PACKET_SIZE_IN_BYTES;
        pInitParams->ag[nAgm].bTrainingByPass = FALSE;

        for ( nPort = 0; nPort < SB_FE2000_MAX_AG_PORTS; nPort++ ) {
            pInitParams->ag[nAgm].common[nPort].bBringUp = FALSE;
            pInitParams->ag[nAgm].common[nPort].bMacLocalLoopback = FALSE;
            pInitParams->ag[nAgm].common[nPort].bMacRemoteLoopback = FALSE;
            pInitParams->ag[nAgm].common[nPort].bSerdesLocalLoopback = FALSE;
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
            /* pInitParams->ag[nAgm].tx[nPort].uCrcMode = SB_FE2000_AG_MAC_CRC_KEEP; */
            pInitParams->ag[nAgm].tx[nPort].uCrcMode = SB_FE2000_AG_MAC_CRC_APPEND;
        }
    }

    /* enable on HIGIG interfaces only */
    for( uIndex = SB_FE2000_IF_PTYPE_AGM0; uIndex <=SB_FE2000_IF_PTYPE_XGM1; uIndex++ ) {
        if (SOC_SBX_CFG_FE2000_IF_IS_HIGIG(unit, uIndex) ) {
            pInitParams->pr.bCcEnable[uIndex - 2] = TRUE;
        }else{
            pInitParams->pr.bCcEnable[uIndex - 2] = FALSE;
        }
        pInitParams->pr.bCcUsePortSteering[uIndex] = FALSE;
        pInitParams->pr.bCcUsePacketSteering[uIndex] = FALSE;
    }

    pInitParams->pr.bBringUp = TRUE;
    for( uIndex = 0; uIndex < SB_FE2000XT_PR_NUM_PRE; uIndex++ ) {
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
    for( uIndex = 0; uIndex < SB_FE2000XT_PB_NUM_PACKET_BUFFERS; uIndex++ ) {
        pInitParams->qm.uNumberOfFreePages[uIndex] = SB_FE2000XT_PB_PAGES_PER_PACKET_BUFFER;
    }
    /* take off few pages in PB0 for OAM CCM. Important dont take pages from
     * PB1 - there is a bug on it
     * OAM is now taking 2 packet buffers.  we need to use an even numbered buffer. */
    pInitParams->qm.uNumberOfFreePages[0] -= (SB_FE2000XT_QM_OAM_FREEPAGES_DEFAULT+1);

    pInitParams->qm.uBaseDropQueue = SB_FE2000XT_QM_BASE_DROP_QUEUE_DEFAULT;
    pInitParams->qm.uNumberOfDropQueues = SB_FE2000XT_QM_MAX_DROP_QUEUES;
    pInitParams->qm.bLockedQueueEnable = TRUE;
    pInitParams->qm.uLockedQueue = SB_FE2000XT_QM_LOCKED_QUEUE_DEFAULT;

    for( uIndex = 0; uIndex < SB_FE2000XT_PR_NUM_PRE; uIndex++ ) {
        for( uPb = 0; uPb < SB_FE2000XT_PB_NUM_PACKET_BUFFERS; uPb++ ) {
            pInitParams->qm.uGlobalInterfaceFlowControlThresh[uIndex][uPb]
                = SB_FE2000XT_PB_PAGES_PER_PACKET_BUFFER+1;
                pInitParams->qm.uGlobalInterfaceFlowControlThresh2[uIndex][uPb]
                    = SB_FE2000XT_PB_PAGES_PER_PACKET_BUFFER+1;
    }
    }

    pInitParams->pt.bBringUp = TRUE;
    pInitParams->pt.bEnablePbPrefetch[0] = TRUE;
    pInitParams->pt.bEnablePbPrefetch[1] = TRUE;

    pInitParams->pt.bUsePtEnable = FALSE;
    for( uIndex = 0; uIndex < SB_FE2000XT_PT_NUM_PTE; uIndex++ ) {
        pInitParams->pt.bPtEnable[uIndex] = FALSE;
    }
    pInitParams->pt.PipelinePriority = SB_FE2000_PT_PIPELINE_EQUAL_TO_ALL_PTES;
    pInitParams->pt.uExpandedFifoSize = 0x1F;
    for( uIndex = 0; uIndex < SB_FE2000XT_PT_NUM_HDR_WORDS; uIndex++ ) {
        pInitParams->pt.uMirrorHeaderMask[uIndex] = 0;
    }

    pInitParams->pb.bBringUpPb = TRUE;

    sal_memset(&pInitParams->pp,0x0,sizeof(sbFe2000XtInitParamsPp_t));

    pInitParams->pp.bBringUp = TRUE;

    pInitParams->pp.uDefaultNumberofQueuesToConfigure = 0;
    pInitParams->pp.uMaxMplsRouteLabel  = C2_PP_DEFAULT_MAX_MPLS_RTLBL;
    pInitParams->pp.uMaxMplsStatsLabel  = C2_PP_DEFAULT_MAX_MPLS_STLBL;
    pInitParams->pp.uMinMplsStatsLabel  = C2_PP_DEFAULT_MIN_MPLS_STLBL;
    pInitParams->pp.uEthernetTypeExceptionMin  = C2_PP_ETHERNET_TYPE_EXCPT_MIN;
    pInitParams->pp.uEthernetTypeExceptionMax  = C2_PP_ETHERNET_TYPE_EXCPT_MAX;

    pInitParams->pp.uDefaultVlanTypeValue     = C2_PP_DEFAULT_VLAN_TYPE;
    pInitParams->pp.uDefaultVlanTypeMaskValue = C2_PP_DEFAULT_VLAN_TYPE_MASK;
    pInitParams->pp.uDefaultLlcMaxLength      = C2_PP_DEFAULT_LLC_MAX_LEN;
    pInitParams->pp.bHashRequired             = FALSE;

    pInitParams->pp.exceptions.bEnableAllExceptions = TRUE;

    /* configure all the queues intial header type to be ethernet */
    for(uIndex=0; uIndex < SB_FE2000XT_PP_NUM_QUEUES; uIndex++) {
        pInitParams->pp.QueueInitialState[uIndex].uHeaderType
            = C2_PP_DEFAULT_ETH_HDRID;
        pInitParams->pp.QueueInitialState[uIndex].uShift = 0;
        pInitParams->pp.QueueInitialState[uIndex].uState = 0;
        pInitParams->pp.QueueInitialState[uIndex].uVariable = 0;
    }

    uPriorityGeneration = 0;
    uBatchGroupGeneration = 0;
    for(uIndex=0; uIndex < SB_FE2000XT_PP_NUM_QUEUES; uIndex++) {
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
                = C2_PP_DEFAULT_EGRESS_RECORD_SIZE;
        } else {
            pInitParams->pp.uHeaderRecordSize[uIndex]
                = C2_PP_DEFAULT_INGRESS_RECORD_SIZE;
        }
        pInitParams->pp.bAddRouteHeader[uIndex] = FALSE;
        pInitParams->pp.bDeleteFirstHeader[uIndex] = FALSE;
        pInitParams->pp.bSourceBufferInsert[uIndex] = FALSE;
    }
    pInitParams->pp.PpByPassLrpModeConfiguration.uMirrorIndex
        = C2_PP_DEFAULT_DEBUG_PED_HDR_MIRRORINDEX;
    pInitParams->pp.PpByPassLrpModeConfiguration.uCopyCount
        = C2_PP_DEFAULT_DEBUG_PED_HDR_COPYCOUNT;
    pInitParams->pp.PpByPassLrpModeConfiguration.bHeaderCopy
        = C2_PP_DEFAULT_DEBUG_PED_HDR_HDRCOPY;
    pInitParams->pp.PpByPassLrpModeConfiguration.bDrop
        = C2_PP_DEFAULT_DEBUG_PED_HDR_DROP;

    pInitParams->pp.uExceptionStreamEnable = 0;
    pInitParams->pp.uExceptionStream = SB_FE2000XT_LR_NUMBER_OF_STREAMS - 1;


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
    pHeaderConfig[C2_PD_DEFAULT_SBROUTE_HDRID].uBaseLength
        = C2_PD_DEFAULT_SBROUTE_HDRLEN;
    pHeaderConfig[C2_PD_DEFAULT_PPP_HDRID].uBaseLength
        = C2_PD_DEFAULT_PPP_HDRLEN;
    pHeaderConfig[C2_PD_DEFAULT_ETH_HDRID].uBaseLength
        = C2_PD_DEFAULT_ETH_HDRLEN;
    pHeaderConfig[C2_PD_DEFAULT_LLC_HDRID].uBaseLength
        = C2_PD_DEFAULT_LLC_HDRLEN;
    pHeaderConfig[C2_PD_DEFAULT_SNAP_HDRID].uBaseLength
        = C2_PD_DEFAULT_SNAP_HDRLEN;
    pHeaderConfig[C2_PD_DEFAULT_VLAN_HDRID].uBaseLength
        = C2_PD_DEFAULT_VLAN_HDRLEN;
    pHeaderConfig[C2_PD_DEFAULT_MPLS_HDRID].uBaseLength
        = C2_PD_DEFAULT_MPLS_HDRLEN;
    pHeaderConfig[C2_PD_DEFAULT_IPV4_HDRID].uBaseLength
        = C2_PD_DEFAULT_IPV4_HDRLEN;
    pHeaderConfig[C2_PD_DEFAULT_IPV6_HDRID].uBaseLength
        = C2_PD_DEFAULT_IPV6_HDRLEN;
    pHeaderConfig[C2_PD_DEFAULT_GRE_HDRID].uBaseLength
        = C2_PD_DEFAULT_GRE_HDRLEN;
    pHeaderConfig[C2_PD_DEFAULT_TCP_HDRID].uBaseLength
        = C2_PD_DEFAULT_TCP_HDRLEN;
    pHeaderConfig[C2_PD_DEFAULT_UDP_HDRID].uBaseLength
        = C2_PD_DEFAULT_UDP_HDRLEN;
    pHeaderConfig[C2_PD_DEFAULT_HIGIG_HDRID].uBaseLength
        = C2_PD_DEFAULT_HIGIG_HDRLEN;
    pHeaderConfig[C2_PD_DEFAULT_SBROUTE_HDRID].uLengthPosition
        = C2_PD_DEFAULT_LEN_POS;
    pHeaderConfig[C2_PD_DEFAULT_PPP_HDRID].uLengthPosition
        = C2_PD_DEFAULT_LEN_POS;
    pHeaderConfig[C2_PD_DEFAULT_ETH_HDRID].uLengthPosition
        = C2_PD_DEFAULT_LEN_POS;
    pHeaderConfig[C2_PD_DEFAULT_LLC_HDRID].uLengthPosition
        = C2_PD_DEFAULT_LEN_POS;
    pHeaderConfig[C2_PD_DEFAULT_SNAP_HDRID].uLengthPosition
        = C2_PD_DEFAULT_LEN_POS;
    pHeaderConfig[C2_PD_DEFAULT_VLAN_HDRID].uLengthPosition
        = C2_PD_DEFAULT_LEN_POS;
    pHeaderConfig[C2_PD_DEFAULT_MPLS_HDRID].uLengthPosition
        = C2_PD_DEFAULT_LEN_POS;
    pHeaderConfig[C2_PD_DEFAULT_IPV4_HDRID].uLengthPosition
        = C2_PD_DEFAULT_IPV4_LEN_POS;
    pHeaderConfig[C2_PD_DEFAULT_IPV6_HDRID].uLengthPosition
        = C2_PD_DEFAULT_LEN_POS;
    pHeaderConfig[C2_PD_DEFAULT_GRE_HDRID].uLengthPosition
        = C2_PD_DEFAULT_LEN_POS;
    pHeaderConfig[C2_PD_DEFAULT_TCP_HDRID].uLengthPosition
        = C2_PD_DEFAULT_LEN_POS;
    pHeaderConfig[C2_PD_DEFAULT_UDP_HDRID].uLengthPosition
        = C2_PD_DEFAULT_LEN_POS;
    pHeaderConfig[C2_PD_DEFAULT_HIGIG_HDRID].uLengthPosition
        = C2_PD_DEFAULT_LEN_POS;
    pHeaderConfig[C2_PD_DEFAULT_EMBEDDED_LENGTH_HDRID].uLengthPosition
        = C2_PD_DEFAULT_LEN_POS;
    pHeaderConfig[C2_PD_DEFAULT_IPV4_HDRID].uLengthSize
        = C2_PD_DEFAULT_IPV4_LEN_SIZE;
    pHeaderConfig[C2_PD_DEFAULT_IPV4_HDRID].uLengthUnits
        = C2_PD_DEFAULT_IPV4_LEN_UNITS;

    sal_memset(&pInitParams->mm[0],0x0,sizeof(sbFe2000XtInitParamsMm_t));
    sal_memset(&pInitParams->mm[1],0x0,sizeof(sbFe2000XtInitParamsMm_t));
    pInitParams->mm[0].bBringUp =TRUE;
    pInitParams->mm[1].bBringUp =TRUE;
    pInitParams->mm[0].uMemDiagConfigMm = TRUE;
    pInitParams->mm[1].uMemDiagConfigMm = TRUE;

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
        pDdrConfig = &(pInitParams->ddrconfig[uMmInstance]);

        pInstance->uNumberofInternalRam0Segments = 1;
        pInstance->uNumberofInternalRam1Segments = 1;
        pInstance->uNumberofNarrowPort0Segments  = 1;
        pInstance->uNumberofNarrowPort1Segments  = 1;
        pInstance->uNumberofWidePortSegments     = 1;

        pInstance->ramconfiguration.InternalRamConfiguration = SB_FE2000XT_MM_RAM0_8KBY72_AND_RAM1_8KBY72;
        pInstance->ramconfiguration.IntRam0DmaControl = SB_FE2000XT_ONE_32_BIT_DAM_XFERS;
        pInstance->ramconfiguration.IntRam0DmaControl = SB_FE2000XT_ONE_32_BIT_DAM_XFERS;

    pInstance->ramconnections.bPmuPrimaryClientOnWideport = FALSE;
        pInstance->ramconnections.Lrp0MemoryConnection
            = SB_FE2000XT_MM_LRP0_CONNECTED_TO_NO_RESOURCE;
        pInstance->ramconnections.Lrp1MemoryConnection
            = SB_FE2000XT_MM_LRP1_CONNECTED_TO_NO_RESOURCE;
        pInstance->ramconnections.Lrp2MemoryConnection
            = SB_FE2000XT_MM_LRP2_CONNECTED_TO_NO_RESOURCE;
        pInstance->ramconnections.Lrp3MemoryConnection
            = SB_FE2000XT_MM_LRP3_CONNECTED_TO_NO_RESOURCE;
        pInstance->ramconnections.Lrp4MemoryConnection
            = SB_FE2000XT_MM_LRP4_CONNECTED_TO_NO_RESOURCE;
        pInstance->ramconnections.PmuMemoryConnection
            = SB_FE2000XT_MM_PMU_CONNECTED_TO_NO_RESOURCE;
        pInstance->ramconnections.Cmu0MemoryConnection
            = SB_FE2000XT_MM_CMU0_CONNECTED_TO_NO_RESOURCE;
        pInstance->ramconnections.Cmu1MemoryConnection
            = SB_FE2000XT_MM_CMU1_CONNECTED_TO_NO_RESOURCE;

        pInstance->ramprotection.InternalRam0ProtectionScheme
            = SB_FE2000XT_MM_32BITS_4BITPARITY;
        pInstance->ramprotection.InternalRam1ProtectionScheme
            = SB_FE2000XT_MM_32BITS_4BITPARITY;
        pInstance->ramprotection.WidePortRamProtectionScheme
            = SB_FE2000XT_MM_32BITS_4BITPARITY;
        pInstance->ramprotection.NarrowPort0ProcessorInterface
            = SB_FE2000XT_MM_32BITS_4BITPARITY;
        pInstance->ramprotection.NarrowPort0DmaAccess
            = SB_FE2000XT_MM_32BITS_4BITPARITY;
        pInstance->ramprotection.NarrowPort0SecondaryClient
            = SB_FE2000XT_MM_35BITS_1BITPARITY;
        pInstance->ramprotection.NarrowPort0PrimaryClient
            = SB_FE2000XT_MM_32BITS_4BITPARITY;
        pInstance->ramprotection.NarrowPort1ProcessorInterface
            = SB_FE2000XT_MM_32BITS_4BITPARITY;
        pInstance->ramprotection.NarrowPort1DmaAccess
            = SB_FE2000XT_MM_32BITS_4BITPARITY;
        pInstance->ramprotection.NarrowPort1SecondaryClient
            = SB_FE2000XT_MM_35BITS_1BITPARITY;
        pInstance->ramprotection.NarrowPort1PrimaryClient
            = SB_FE2000XT_MM_32BITS_4BITPARITY;

        pDdrConfig->bValid = 0;
        
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
            = SB_FE2000XT_DEFAULT_BKT_SIZE;
        pInitParams->pm.profile[nProfile].uCBS
            = SB_FE2000XT_MAX_PACKET_SIZE_IN_BYTES;
        pInitParams->pm.profile[nProfile].uCIRBytes  = 0;
        pInitParams->pm.profile[nProfile].uEBS
            = SB_FE2000XT_MAX_PACKET_SIZE_IN_BYTES;
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
    for( uStreamIndex = 0; uStreamIndex < SB_FE2000XT_LR_NUMBER_OF_STREAMS;
         uStreamIndex++ ) {
        pInitParams->lr.bStreamOnline[uStreamIndex] = FALSE;
    }
    pInitParams->lr.uPpeRequestPaceInCycles = 2;
    pInitParams->lr.uFramesPerContext = 48;
    pInitParams->lr.bPairedMode  = TRUE;
    pInitParams->lr.uLoadCount = 1;
    pInitParams->lr.uNumberOfContexts = SB_FE2000XT_LR_MAX_NUMBER_OF_CONTEXTS - 1;
    pInitParams->lr.uWords0 =  C2_PP_DEFAULT_INGRESS_RECORD_SIZE/16;
    pInitParams->lr.uWords1 =  C2_PP_DEFAULT_EGRESS_RECORD_SIZE/16;
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
CalculateLrLoaderEpoch(sbFe2000XtInitParams_t *pInitParams)
{
  uint32_t uLoaderEpoch = 0;

  int64_t fIngressLoadTime;
  int64_t fEgressLoadTime;
#if 0
  int64_t fXferTime;
  int64_t fFirstTime;
  int64_t fIntermediate;
#endif
  /* rgf - Aug 29 2007 - The loader happens in the CORE domain but this value is used in the 
   * rgf - Aug 29 2007 - HPP domain so we need to scale it to accommodate.*/

/*uint64_t fClockScale = (SB_FE2000XT_PIPELINE_CLOCK_RATE / SB_FE2000XT_CORE_CLOCK_RATE);*/
  uint64_t fClockScale = 0;

  if (pInitParams->uHppFreq == 0 || pInitParams->uSwsFreq == 0) {
    SB_ASSERT(0);
  }

  fClockScale = (pInitParams->uHppFreq / pInitParams->uSwsFreq);

  /* rgf - Dec 08 2006 - From Nick's explanation in BUG23565
   * rgf - Dec 08 2006 - Context Load Time:
   * rgf - Dec 08 2006 -
   * rgf - Dec 08 2006 - xfer_time = ceiling(frames_per_context / (10 + 2*load_count)) *
   * rgf - Dec 08 2006 -                    (10 + 2*load_count) * ceiling(words0 / 2)
   * rgf - Dec 08 2006 - first_time = load_count + 4 ;; {init & first states}
   * rgf - Aug 29 2007 - MODIFIED FOR C2 first_time = load_count + 10; added latency
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

#if 0
  fIntermediate = (10.0 + 2.0*(int64_t)pInitParams->lr.uLoadCount);
  fXferTime = (uint32_t)(ceil( (int64_t)(pInitParams->lr.uFramesPerContext) / fIntermediate ) *
             fIntermediate * ceil( (int64_t)pInitParams->lr.uWords0 / 2.0 ));

  fFirstTime = (int64_t)pInitParams->lr.uLoadCount + 4.0;
  fIngressLoadTime = fXferTime + fFirstTime + 2.0;

  fXferTime = (uint32_t)(ceil( (int64_t)(pInitParams->lr.uFramesPerContext) / fIntermediate ) *
             fIntermediate * ceil( (int64_t)pInitParams->lr.uWords1 / 2.0 ));
  /* First time remains the same as ingress */
  fEgressLoadTime = fXferTime + fFirstTime + 2.0;

  if( pInitParams->lr.bPairedMode == TRUE ) {
    uLoaderEpoch = (uint32_t)(fIngressLoadTime + fEgressLoadTime);
    /* if( (uLoaderEpoch < SB_FE2000_LR_20G_DUPLEX_EPOCH) && (gZM.GetAttribute("allow_small_epochs",0) == 0) ) { */
    if(uLoaderEpoch < SB_FE2000_LR_20G_DUPLEX_EPOCH ) {
      uLoaderEpoch = SB_FE2000_LR_20G_DUPLEX_EPOCH;
    }
  } else {
    uLoaderEpoch = (uint32_t)fIngressLoadTime;
  }
#endif

  fIngressLoadTime = (uint64_t)pInitParams->lr.uLoadCount + 4 + 
                     ((uint64_t)pInitParams->lr.uFramesPerContext * ((uint64_t)pInitParams->lr.uWords0 >> 1)); 

  fEgressLoadTime =  (uint64_t)pInitParams->lr.uLoadCount + 4 +
                     ((uint64_t)pInitParams->lr.uFramesPerContext * ((uint64_t)pInitParams->lr.uWords1 >> 1));

  if( pInitParams->lr.bPairedMode == TRUE ) {
    uLoaderEpoch = (uint32_t)(COMPILER_64_ADD_64(fIngressLoadTime,fEgressLoadTime) * fClockScale);
    uLoaderEpoch += 14;  /* HPP delay is 14 clocks */
    /* if( (uLoaderEpoch < SB_FE2000XT_LR_24G_DUPLEX_EPOCH) && (gZM.GetAttribute("allow_small_epochs",0) == 0) ) { */
    if(uLoaderEpoch < SB_FE2000XT_LR_24G_DUPLEX_EPOCH ) {
      uLoaderEpoch = SB_FE2000XT_LR_24G_DUPLEX_EPOCH;
    }
  } else {
    /* rgf - Nov 05 2007 - Add a fudge factor for C2 added latency, not yet understood from where.*/
    uLoaderEpoch = (uint32_t)(fIngressLoadTime * fClockScale) + 14;
  }


  SB_LOGV2("LoaderEpoch %d based on Paired %d Words0 %d Words1 %d FramesPerContext %d LoadCount %d",
         uLoaderEpoch, pInitParams->lr.bPairedMode, pInitParams->lr.uWords0,
         pInitParams->lr.uWords1, pInitParams->lr.uFramesPerContext, pInitParams->lr.uLoadCount);

  return uLoaderEpoch;

}

uint32_t
sbFe2000XtInit(sbhandle userDeviceHandle,
             sbFe2000XtInitParams_t *pInitParams)
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
      SAND_HAL_WRITE(userDeviceHandle, C2,  PM_ERROR_MASK, ~0);
      SOC_WARM_BOOT_START((int)userDeviceHandle);

      return SB_OK;
  }

  /* SB_ASSERT(pInitParamsInternal); */

  /*
   * Bringup blocks within FE
   */

  /* check ==> PCI interface should already be initialized via CFE */

  /* dclee - Mar 28 2007 - Initiate PLL bringup procedure */
  status = sbFe2000XtInitPLL(userDeviceHandle,pInitParams);
  if (status != SB_FE2000_STS_INIT_OK_K)
    {
        soc_cm_print("sbFe2000XtInitPLL() failed with error code:0x%x\n", status);
        return status;
    }

  /* configure slip buffers */
  status = sbFe2000XtInitPsyncSlipBuffers(userDeviceHandle, pInitParams);

  /* SWS Queue Manager */
  status = sbFe2000XtInitQm(userDeviceHandle,pInitParams);
  if (status != SB_FE2000_STS_INIT_OK_K)
    {
        soc_cm_print("sbFe2000XtInitQm() failed with error code:0x%x\n", status);
        return status;
    }

  /* SPI */
  status = sbFe2000XtInitSpi(userDeviceHandle,0,pInitParams);
  if (status != SB_FE2000_STS_INIT_OK_K)
    {
        soc_cm_print("sbFe2000XtInitSpi[0] failed with error code:0x%x\n", status);
        return status;
    }

  status = sbFe2000XtInitSpi(userDeviceHandle,1,pInitParams);
  if (status != SB_FE2000_STS_INIT_OK_K)
    {
        soc_cm_print("sbFe2000XtInitSpi[1] failed with error code:0x%x\n", status);
        return status;
    }

  /* Ten Gigabit Ethernet MACs */
  for(i=0;i< SB_FE2000XT_NUM_XG_INTERFACES;i++) {
    if (pInitParams->xg[i].bBringUp) {
        status = sbFe2000XtInitXgm(userDeviceHandle,i,pInitParams);
        if (status != SB_FE2000_STS_INIT_OK_K)
        {
            soc_cm_print("sbFe2000XtInitXgm [interface %d] failed with error code:0x%x\n", i, status);
            return status;
        }
    }
  }

  /* Aggregated Gigabit Ethernet MAC */
  status = sbFe2000XtInitAgms(userDeviceHandle,pInitParams);
  if (status != SB_FE2000_STS_INIT_OK_K)
  {
      soc_cm_print("sbFe2000XtInitAgms failed with error code:0x%x\n", status);
      return status;
  }

  /* SWS Packet Buffer */
  sbFe2000XtInitPb(userDeviceHandle,pInitParams);

  /* SWS Packet Transmitter */
  status = sbFe2000XtInitPt(userDeviceHandle,pInitParams);
  if (status != SB_FE2000_STS_INIT_OK_K)
    {
        soc_cm_print("sbFe2000XtInitPt failed with error code:0x%x\n", status);
        return status;
    }

  /* SWS Packet Receiver */
  status = sbFe2000XtInitPr(userDeviceHandle,pInitParams);
  if (status != SB_FE2000_STS_INIT_OK_K)
    {
        soc_cm_print("sbFe2000XtInitPr failed with error code:0x%x\n", status);
        return status;
    }

  /* Packet Parsing Engine */
  status = sbFe2000XtInitPp(userDeviceHandle,pInitParams);
  if (status != SB_FE2000_STS_INIT_OK_K)
    {
        soc_cm_print("sbFe2000XtInitPp failed with error code:0x%x\n", status);
        return status;
    }

  /* PacketEditor */
  sbFe2000XtInitPd(userDeviceHandle,pInitParams);

  /* Memory Management Unit */
  for(i=0;i<SB_FE2000_NUM_MM_INSTANCES;i++) {
    status = sbFe2000XtInitMm(i,userDeviceHandle,pInitParams);
    if (status != SB_FE2000_STS_INIT_OK_K)
    {
        soc_cm_print(" sbFe2000XtInitMm instance %d failed with error code:0x%x\n",i, status);
        return status;
    }
  }

  /* clear the coherent counters */
  sbFe2000XtInitCoCounters(userDeviceHandle,pInitParams);

  /* Counter Management Unit */
  sbFe2000XtInitCm(userDeviceHandle,pInitParams);

  /* Rule Classification Engine */
  for(i=0;i<SB_FE2000_NUM_RC_INSTANCES;i++) {
    sbFe2000XtInitRc(i,userDeviceHandle,pInitParams);
  }

  /* Policier Management Unit */
  sbFe2000XtInitPm(userDeviceHandle,pInitParams);

  /* Line Rate Processor */
  status = sbFe2000XtInitLr(userDeviceHandle,pInitParams);
  if (status != SB_FE2000_STS_INIT_OK_K) {
      soc_cm_print("sbFe2000XtInitLr failed with error code:0x%x\n", status);
      return status;
  }

  /* clean up all blocks, clear error registers, unmask error registers.. */
  sbFe2000XtInitCleanUp(userDeviceHandle,pInitParams);

  DEXIT();
  return SB_FE2000_STS_INIT_OK_K;
}

static uint32_t
sbFe2000XtInitRegDone(sbhandle userDeviceHandle,
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
sbFe2000XtInitQm(sbhandle userDeviceHandle,
               sbFe2000XtInitParams_t *pInitParams)
{
  sbZfC2QmQueueConfigEntry_t zQueueConfig;
  uint32_t uDropQueue;
  sbZfC2QmQueueState0Entry_t zQueueState;
  /* page is 192byte, there are 192/8 = 24 words for a page*/

  DENTER();

  /* bms - May 01 2006 - setup the number_free_pages_pb0 and number_free_pages_pb1 */
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, QM_FP_CONFIG, NUMBER_FREE_PAGES_PB0, pInitParams->qm.uNumberOfFreePages[0]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, QM_FP_CONFIG, NUMBER_FREE_PAGES_PB1, pInitParams->qm.uNumberOfFreePages[1]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, QM_CONFIG0, TOTAL_MAX_PAGES_AVAILABLE_PB0, pInitParams->qm.uNumberOfFreePages[0]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, QM_CONFIG0, TOTAL_MAX_PAGES_AVAILABLE_PB1, pInitParams->qm.uNumberOfFreePages[1]);

  /* rgf - Jan 18 2007 - Setup per PRE per PB global backpressure thresholds */
  SAND_HAL_WRITE(userDeviceHandle, C2, QM_GLOBAL_INTF0_PB0_THRESHOLD, 
         SAND_HAL_SET_FIELD(C2, QM_GLOBAL_INTF0_PB0_THRESHOLD, FLOW_CTRL_THRESH1, pInitParams->qm.uGlobalInterfaceFlowControlThresh[0][0]) |
         SAND_HAL_SET_FIELD(C2, QM_GLOBAL_INTF0_PB0_THRESHOLD, FLOW_CTRL_THRESH2, pInitParams->qm.uGlobalInterfaceFlowControlThresh2[0][0]));
  SAND_HAL_WRITE(userDeviceHandle, C2, QM_GLOBAL_INTF0_PB1_THRESHOLD, 
         SAND_HAL_SET_FIELD(C2, QM_GLOBAL_INTF0_PB1_THRESHOLD, FLOW_CTRL_THRESH1, pInitParams->qm.uGlobalInterfaceFlowControlThresh[0][1]) |
         SAND_HAL_SET_FIELD(C2, QM_GLOBAL_INTF0_PB1_THRESHOLD, FLOW_CTRL_THRESH2, pInitParams->qm.uGlobalInterfaceFlowControlThresh2[0][1]));

  SAND_HAL_WRITE(userDeviceHandle, C2, QM_GLOBAL_INTF1_PB0_THRESHOLD, 
         SAND_HAL_SET_FIELD(C2, QM_GLOBAL_INTF1_PB0_THRESHOLD, FLOW_CTRL_THRESH1, pInitParams->qm.uGlobalInterfaceFlowControlThresh[1][0]) |
         SAND_HAL_SET_FIELD(C2, QM_GLOBAL_INTF1_PB0_THRESHOLD, FLOW_CTRL_THRESH2, pInitParams->qm.uGlobalInterfaceFlowControlThresh2[1][0]));
  SAND_HAL_WRITE(userDeviceHandle, C2, QM_GLOBAL_INTF1_PB1_THRESHOLD, 
         SAND_HAL_SET_FIELD(C2, QM_GLOBAL_INTF1_PB1_THRESHOLD, FLOW_CTRL_THRESH1, pInitParams->qm.uGlobalInterfaceFlowControlThresh[1][1]) |
         SAND_HAL_SET_FIELD(C2, QM_GLOBAL_INTF1_PB1_THRESHOLD, FLOW_CTRL_THRESH2, pInitParams->qm.uGlobalInterfaceFlowControlThresh2[1][1]));

  SAND_HAL_WRITE(userDeviceHandle, C2, QM_GLOBAL_INTF2_PB0_THRESHOLD, 
         SAND_HAL_SET_FIELD(C2, QM_GLOBAL_INTF2_PB0_THRESHOLD, FLOW_CTRL_THRESH1, pInitParams->qm.uGlobalInterfaceFlowControlThresh[2][0]) |
         SAND_HAL_SET_FIELD(C2, QM_GLOBAL_INTF2_PB0_THRESHOLD, FLOW_CTRL_THRESH2, pInitParams->qm.uGlobalInterfaceFlowControlThresh2[2][0]));
  SAND_HAL_WRITE(userDeviceHandle, C2, QM_GLOBAL_INTF2_PB1_THRESHOLD, 
         SAND_HAL_SET_FIELD(C2, QM_GLOBAL_INTF2_PB1_THRESHOLD, FLOW_CTRL_THRESH1, pInitParams->qm.uGlobalInterfaceFlowControlThresh[2][1]) |
         SAND_HAL_SET_FIELD(C2, QM_GLOBAL_INTF2_PB1_THRESHOLD, FLOW_CTRL_THRESH2, pInitParams->qm.uGlobalInterfaceFlowControlThresh2[2][1]));

  SAND_HAL_WRITE(userDeviceHandle, C2, QM_GLOBAL_INTF3_PB0_THRESHOLD, 
         SAND_HAL_SET_FIELD(C2, QM_GLOBAL_INTF3_PB0_THRESHOLD, FLOW_CTRL_THRESH1, pInitParams->qm.uGlobalInterfaceFlowControlThresh[3][0]) |
         SAND_HAL_SET_FIELD(C2, QM_GLOBAL_INTF3_PB0_THRESHOLD, FLOW_CTRL_THRESH2, pInitParams->qm.uGlobalInterfaceFlowControlThresh2[3][0]));
  SAND_HAL_WRITE(userDeviceHandle, C2, QM_GLOBAL_INTF3_PB1_THRESHOLD, 
         SAND_HAL_SET_FIELD(C2, QM_GLOBAL_INTF3_PB1_THRESHOLD, FLOW_CTRL_THRESH1, pInitParams->qm.uGlobalInterfaceFlowControlThresh[3][1]) |
         SAND_HAL_SET_FIELD(C2, QM_GLOBAL_INTF3_PB1_THRESHOLD, FLOW_CTRL_THRESH2, pInitParams->qm.uGlobalInterfaceFlowControlThresh2[3][1]));

  SAND_HAL_WRITE(userDeviceHandle, C2, QM_GLOBAL_INTF4_PB0_THRESHOLD, 
         SAND_HAL_SET_FIELD(C2, QM_GLOBAL_INTF4_PB0_THRESHOLD, FLOW_CTRL_THRESH1, pInitParams->qm.uGlobalInterfaceFlowControlThresh[4][0]) |
         SAND_HAL_SET_FIELD(C2, QM_GLOBAL_INTF4_PB0_THRESHOLD, FLOW_CTRL_THRESH2, pInitParams->qm.uGlobalInterfaceFlowControlThresh2[4][0]));
  SAND_HAL_WRITE(userDeviceHandle, C2, QM_GLOBAL_INTF4_PB1_THRESHOLD, 
         SAND_HAL_SET_FIELD(C2, QM_GLOBAL_INTF4_PB1_THRESHOLD, FLOW_CTRL_THRESH1, pInitParams->qm.uGlobalInterfaceFlowControlThresh[4][1]) |
         SAND_HAL_SET_FIELD(C2, QM_GLOBAL_INTF4_PB1_THRESHOLD, FLOW_CTRL_THRESH2, pInitParams->qm.uGlobalInterfaceFlowControlThresh2[4][1]));

  SAND_HAL_WRITE(userDeviceHandle, C2, QM_GLOBAL_INTF5_PB0_THRESHOLD, 
         SAND_HAL_SET_FIELD(C2, QM_GLOBAL_INTF5_PB0_THRESHOLD, FLOW_CTRL_THRESH1, pInitParams->qm.uGlobalInterfaceFlowControlThresh[5][0]) |
         SAND_HAL_SET_FIELD(C2, QM_GLOBAL_INTF5_PB0_THRESHOLD, FLOW_CTRL_THRESH2, pInitParams->qm.uGlobalInterfaceFlowControlThresh2[5][0]));
  SAND_HAL_WRITE(userDeviceHandle, C2, QM_GLOBAL_INTF5_PB1_THRESHOLD, 
         SAND_HAL_SET_FIELD(C2, QM_GLOBAL_INTF5_PB1_THRESHOLD, FLOW_CTRL_THRESH1, pInitParams->qm.uGlobalInterfaceFlowControlThresh[5][1]) |
         SAND_HAL_SET_FIELD(C2, QM_GLOBAL_INTF5_PB1_THRESHOLD, FLOW_CTRL_THRESH2, pInitParams->qm.uGlobalInterfaceFlowControlThresh2[5][1]));

  /* rgf - Sep 29 2006 - Setup the drop queues */
  if( (pInitParams->qm.uBaseDropQueue & 0x3) != 0 ) {
    SB_ERROR("Bad configuration, the two LSBs of the base drop queue must be zero.  Specified value 0x%x", pInitParams->qm.uBaseDropQueue);
  }
  if( (pInitParams->qm.uNumberOfDropQueues < 1) || (pInitParams->qm.uNumberOfDropQueues > 4) ) {
    SB_ERROR("Bad Configuration, valid values for Number of Drop Queues are 1 through 4.  Specified value %d", pInitParams->qm.uNumberOfDropQueues);
  }
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, QM_CONFIG1, LOCKED_QUEUE_ENABLE, pInitParams->qm.bLockedQueueEnable);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, QM_CONFIG1, LOCKED_QUEUE, pInitParams->qm.uLockedQueue);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, QM_CONFIG1, DROP_QUEUES, pInitParams->qm.uNumberOfDropQueues);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, QM_CONFIG1, DROP_QUEUES_BASE, pInitParams->qm.uBaseDropQueue);
  /* rgf - Dec 04 2006 - Queue groups are groups of 16 queues, force bottom 4 bits to zero */
  /* rgf - Dec 04 2006 - Mark queue group and queues as allocated */

  /* rgf - May 11 2006 - Take block out of reset */
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, QM_CORE_RESET, 0x0);

  
  /* bms - May 01 2006 - setup the number_free_pages_pb0 and number_free_pages_pb1 */
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, QM_FP_CONFIG, NUMBER_FREE_PAGES_PB0, pInitParams->qm.uNumberOfFreePages[0]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, QM_FP_CONFIG, NUMBER_FREE_PAGES_PB1, pInitParams->qm.uNumberOfFreePages[1]);
  SAND_HAL_WRITE(userDeviceHandle, C2,  QM_QUEUE_CONFIG_MEM_ACC_DATA0, 0);
  SAND_HAL_WRITE(userDeviceHandle, C2,  QM_QUEUE_CONFIG_MEM_ACC_DATA1, 0);
  SAND_HAL_WRITE(userDeviceHandle, C2,  QM_QUEUE_CONFIG_MEM_ACC_DATA2, 0);
  SAND_HAL_WRITE(userDeviceHandle, C2,  QM_QUEUE_STATE0_MEM_ACC_DATA, 0);
  SAND_HAL_WRITE(userDeviceHandle, C2,  QM_QUEUE_STATE1_MEM_ACC_DATA, 0);

  /* bms - May 01 2006 - now set the init bit */
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, QM_CONFIG0, INIT, 1);

  if( sbFe2000XtInitRegDone(userDeviceHandle,
              SAND_HAL_REG_OFFSET(C2, QM_CONFIG0),
              SAND_HAL_FIELD_MASK(C2, QM_CONFIG0, INIT_DONE),
              SB_FE2000_QM_INIT_TIMEOUT) ) {

    SB_ERROR("Waiting for QM init_done timed out..\n");
    return SB_FE2000_STS_INIT_QM_TIMEOUT_ERR_K;
  }

  sbZfC2QmQueueConfigEntry_InitInstance(&zQueueConfig);

  zQueueConfig.m_uEnable = 1;
  zQueueConfig.m_uMaxPages = SB_FE2000XT_PB_PAGES_PER_PACKET_BUFFER;
  zQueueConfig.m_uDropThresh2 = SB_FE2000XT_PB_PAGES_PER_PACKET_BUFFER;
  zQueueConfig.m_uMinPages = 0;
  zQueueConfig.m_uEnforcePoliceMarkings = 0;
  zQueueConfig.m_uFlowControlEnable = 0;

  for( uDropQueue = pInitParams->qm.uBaseDropQueue; uDropQueue < (pInitParams->qm.uBaseDropQueue + pInitParams->qm.uNumberOfDropQueues); uDropQueue++ ) {
    C2QmQueueConfigWrite( userDeviceHandle, uDropQueue, &zQueueConfig);

    /* bms - Nov 08 2006 - Since only QueueState0 is back door and since we are only setting this enable bit before */
    /* bms - Nov 08 2006 - passing any traffic we can safely do a RMW */
    sbZfC2QmQueueState0Entry_InitInstance(&zQueueState);
    C2QmQueueState0Read( userDeviceHandle,uDropQueue, &zQueueState );
    zQueueState.m_uEnable = 1;
    zQueueState.m_uEmpty = 1;
    C2QmQueueState0Write( userDeviceHandle,uDropQueue, &zQueueState );

  }

  /* Init Locked queue for OAM */
  sbZfC2QmQueueConfigEntry_InitInstance(&zQueueConfig);
  zQueueConfig.m_uEnable = 1;
  zQueueConfig.m_uMaxPages = SB_FE2000XT_QM_OAM_FREEPAGES_DEFAULT - 1;
  zQueueConfig.m_uDropThresh2 = SB_FE2000XT_QM_OAM_FREEPAGES_DEFAULT;
  zQueueConfig.m_uMinPages = 0;
  zQueueConfig.m_uEnforcePoliceMarkings = 0;
  zQueueConfig.m_uFlowControlEnable = 0;
  C2QmQueueConfigWrite( userDeviceHandle,pInitParams->qm.uLockedQueue, &zQueueConfig);
  sbZfC2QmQueueState0Entry_InitInstance(&zQueueState);
  C2QmQueueState0Read(userDeviceHandle, pInitParams->qm.uLockedQueue,
                       &zQueueState );
  zQueueState.m_uEnable = 1;
  zQueueState.m_uEmpty = 1;
  C2QmQueueState0Write(userDeviceHandle, pInitParams->qm.uLockedQueue,
                       &zQueueState );

  /* Clear init done and init bits  */
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, QM_CONFIG0, INIT, 0);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, QM_CONFIG0, INIT_DONE, 0);

  DEXIT();
  return SB_FE2000_STS_INIT_OK_K;
}

static uint32_t
sbFe2000XtInitSpiTx(sbhandle userDeviceHandle,
                  uint16_t nSpi,
                  sbFe2000XtInitParams_t *pInitParams) {
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
  uint32_t uStStatus;
  uint32_t bLocked;
  uint32_t uTimeOut;
  uint32_t uMDivSpi;
  uint32_t uNDivSpi;
  uint32_t uVRngSpi;

#define SB_FE2000_CALENDAR_ENTRIES_PER_REGISTER 4
  DENTER();

  nCalendarLength = pInitParams->st[nSpi].nCalendarLength;
  if (nCalendarLength == 0) {
      nCalendarLength = 1;
  }

  /* csk - May 01 2007 - Handle SPI loopback if requested */
  SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle, C2,ST,nSpi,ST_CONFIG2,LOOPBACK_ENABLE,
                pInitParams->st[nSpi].bLoopback);

  uConfig0 = SAND_HAL_READ_STRIDE(userDeviceHandle, C2, ST, nSpi, ST0_CONFIG0);

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

  uConfig0 = SAND_HAL_MOD_FIELD(C2, ST0_CONFIG0, TX_CAL_LEN, uConfig0,
                                nCalendarLength - 1);
  uConfig0 = SAND_HAL_MOD_FIELD(C2, ST0_CONFIG0, TX_ARB_LEN, uConfig0,
                                nCalendarLength - 1);
  uConfig0 = SAND_HAL_MOD_FIELD(C2, ST0_CONFIG0, TX_CAL_M,   uConfig0, (pInitParams->st[nSpi].nCalendarM-1));
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2, ST, nSpi, ST0_CONFIG0, uConfig0);

  uConfig1 = SAND_HAL_READ_STRIDE(userDeviceHandle, C2, ST, nSpi, ST0_CONFIG1);
  /* Control only at SOP/EOP  */
  uConfig1 = SAND_HAL_MOD_FIELD(C2, ST0_CONFIG1, TX_CTRL_MAX_DISABLE, uConfig1, 0x1 );
  uConfig1 = SAND_HAL_MOD_FIELD(C2, ST0_CONFIG1, TX_MAXBURST2, uConfig1, (pInitParams->st[nSpi].nMaxBurst2));
  uConfig1 = SAND_HAL_MOD_FIELD(C2, ST0_CONFIG1, TX_MAXBURST1, uConfig1, (pInitParams->st[nSpi].nMaxBurst1));
  /* Disable interval training  */
  uConfig1 = SAND_HAL_MOD_FIELD(C2, ST0_CONFIG1, DATA_MAX_T,   uConfig1, 0x0);
  /* repeat training only once  */
  uConfig1 = SAND_HAL_MOD_FIELD(C2, ST0_CONFIG1, ALPHA,        uConfig1, 0x2);
  uConfig1 = SAND_HAL_MOD_FIELD(C2, ST0_CONFIG1, TSCLK_EDGE,   uConfig1, (pInitParams->st[nSpi].nTSclkEdge));
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2, ST, nSpi, ST0_CONFIG1, uConfig1);

  /*
   * - Setup the streaming packet fifo such that all enabled
   * - ports are equally sharing the fifo.
   * - give high speed channel larger fifo
   */

  nFifoBottom = 0;
  for ( nPort=0; nPort<nCalendarLength; nPort++ ) {
      if (pInitParams->sr[nSpi].nFifoLines[nPort] < SB_FE2000XT_SPI_MIN_LINES_PER_PORT)
      {
          nFifoTop = SB_FE2000XT_SPI_MIN_LINES_PER_PORT;
      } else {
          nFifoTop = nFifoBottom + pInitParams->sr[nSpi].nFifoLines[nPort] - 1;
      }

      uValue  = SAND_HAL_SET_FIELD(C2, ST_P0_FIFO_CONFIG, FIFO_TOP, nFifoTop);
      uValue |=
          SAND_HAL_SET_FIELD(C2, ST_P0_FIFO_CONFIG, FIFO_BOTTOM, nFifoBottom);
      SAND_HAL_WRITE_INDEX_STRIDE(userDeviceHandle, C2, ST, nSpi,
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

    SAND_HAL_WRITE_INDEX_STRIDE(userDeviceHandle, C2, ST, nSpi,
                ST0_CAL_CONFIG0, (nCalendarIdx/4)-1, uValue);
    SAND_HAL_WRITE_INDEX_STRIDE(userDeviceHandle, C2, ST, nSpi,
                ST0_TX_ARB_CONFIG0, (nCalendarIdx/4)-1,
                                uValue);
  }

  /* rgf - Apr 26 2006 - Take block out of reset and enable  */
  if ( 0 == nSpi ) {
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, ST0_CORE_RESET, 0x0);
  } else {
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, ST1_CORE_RESET, 0x0);
  }

  uConfig1 = SAND_HAL_MOD_FIELD(C2, ST0_CONFIG1, PC_RESET, uConfig1, 0x0);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2, ST, nSpi, ST0_CONFIG1, uConfig1);

  
  thin_delay(300);

  uConfig0 = SAND_HAL_MOD_FIELD(C2, ST0_CONFIG0, TX_ENABLE, uConfig0, 1);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2, ST, nSpi, ST0_CONFIG0, uConfig0);

  /* SPI PLL */
  if (!pInitParams->bSimulation) {

    if ( pInitParams->uSpiFreq == SB_FE2000XT_SPI_FREQ_500000) {
      uMDivSpi = 2;
      uNDivSpi = 40;
      uVRngSpi = 0;
    } else if ( pInitParams->uSpiFreq == SB_FE2000XT_SPI_FREQ_400000) {
      uMDivSpi = 4;
      uNDivSpi = 64;
      uVRngSpi = 0;
    } else {
      uMDivSpi = 0;
      uNDivSpi = 0;
      uVRngSpi = 0;
      SB_ASSERT(0);
    }

    uConfig0 = SAND_HAL_READ_STRIDE(userDeviceHandle, C2, ST, nSpi, ST0_PLL_CONFIG0);
    uConfig0 = SAND_HAL_MOD_FIELD(C2, ST0_PLL_CONFIG0, TXPLL_MDIV, uConfig0, uMDivSpi);
    uConfig0 = SAND_HAL_MOD_FIELD(C2, ST0_PLL_CONFIG0, TXPLL_NDIV, uConfig0, uNDivSpi);
    uConfig0 = SAND_HAL_MOD_FIELD(C2, ST0_PLL_CONFIG0, TXPLL_VCO_RNG, uConfig0, uVRngSpi);
    uConfig0 = SAND_HAL_MOD_FIELD(C2, ST0_PLL_CONFIG0, TXPLL_A_RESET, uConfig0, 1);
    uConfig0 = SAND_HAL_MOD_FIELD(C2, ST0_PLL_CONFIG0, TXPLL_D_RESET, uConfig0, 1);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2, ST, nSpi, ST0_PLL_CONFIG0, uConfig0);
    thin_delay(200);
    uConfig0 = SAND_HAL_MOD_FIELD(C2, ST0_PLL_CONFIG0, TXPLL_A_RESET, uConfig0, 0);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2, ST, nSpi, ST0_PLL_CONFIG0, uConfig0);
    bLocked = 0;
    uTimeOut = 50;
    while ( !bLocked && uTimeOut ) {
      bLocked = SAND_HAL_GET_FIELD(C2, ST0_STATUS, LOCKTX, SAND_HAL_READ_STRIDE(userDeviceHandle, C2, ST, nSpi, ST0_STATUS) );
      --uTimeOut;
      /* dgm - Apr 03 2008 - Successful work-around for PLL model issue that has been fixed. */
      /*
      if ( !(uTimeOut % 25) ) {
        uConfig0 = SAND_HAL_MOD_FIELD(C2, ST0_PLL_CONFIG0, TXPLL_A_RESET, uConfig0, 1);
        SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2, ST, nSpi, ST0_PLL_CONFIG0, uConfig0);
        thin_delay(200);
        uConfig0 = SAND_HAL_MOD_FIELD(C2, ST0_PLL_CONFIG0, TXPLL_A_RESET, uConfig0, 0);
        SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2, ST, nSpi, ST0_PLL_CONFIG0, uConfig0);
      }
      */
    }
    if ( !uTimeOut ) {
      uStStatus = SAND_HAL_READ_STRIDE(userDeviceHandle, C2, ST, nSpi, ST0_STATUS);
      SB_ERROR("ST%d PLL failed to lock!  Status 0x%x", nSpi, uStStatus);
    } else {
      SB_LOGV1("ST%d PLL locked.", nSpi);
    }
    uConfig0 = SAND_HAL_MOD_FIELD(C2, ST0_PLL_CONFIG0, TXPLL_D_RESET, uConfig0, 0);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2, ST, nSpi, ST0_PLL_CONFIG0, uConfig0);
  }

  return SB_FE2000_STS_INIT_OK_K;
}


static uint32_t
sbFe2000XtInitSpiRx(sbhandle userDeviceHandle,
                  uint16_t nSpi,
                  sbFe2000XtInitParams_t *pInitParams) {
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
  SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle, C2,SR,nSpi,SR_CONFIG1,LOOPBACK_ENABLE,
               pInitParams->sr[nSpi].bLoopback);

  /* jts - Apr 06 2006 - ALIGN_DLY is the only field, just write (not RMW) */
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2,SR,nSpi,SR0_CONFIG4,pInitParams->sr[nSpi].nAlignDelay);

  uConfig0 = SAND_HAL_READ_STRIDE(userDeviceHandle, C2,SR,nSpi,SR0_CONFIG5);
  uConfig0 = SAND_HAL_MOD_FIELD(C2,SR0_CONFIG5,MAXBURST2,uConfig0,(pInitParams->sr[nSpi].nMaxBurst2));
  uConfig0 = SAND_HAL_MOD_FIELD(C2,SR0_CONFIG5,MAXBURST1,uConfig0,(pInitParams->sr[nSpi].nMaxBurst1));
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2,SR,nSpi,SR0_CONFIG5,uConfig0);

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

  SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2,SR,nSpi,SR0_CONFIG2,uPortEnable);

  /* jts - Apr 06 2006 - next, operate on ports 32-64  */
  for ( nCalendarIdx=0; nCalendarIdx < nCalendarLength; nCalendarIdx++ ) {
    nCalendarEntry = pInitParams->sr[nSpi].nCalendarEntry[nCalendarIdx];
    if ( nCalendarEntry > 31 && nCalendarEntry < 64 ) {
      uPortEnable |= 1 << (nCalendarEntry-32);
    }
  }

  SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2, SR, nSpi, SR0_CONFIG3,
            uPortEnable);

  /* jts - Apr 06 2006 - Setup the calendar  */
  uConfig0 = SAND_HAL_READ_STRIDE(userDeviceHandle, C2, SR, nSpi, SR0_CONFIG0);

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
  uConfig0 = SAND_HAL_MOD_FIELD(C2, SR0_CONFIG0, RX_CAL_LEN, uConfig0,
                                nCalendarLength-1);
  uConfig0 = SAND_HAL_MOD_FIELD(C2, SR0_CONFIG0, RX_CAL_M,   uConfig0, (pInitParams->sr[nSpi].nCalendarM-1));
  uConfig0 = SAND_HAL_MOD_FIELD(C2, SR0_CONFIG0, RX_DIP4_LOS_THRESH, uConfig0, 2);

  /* SAND_HAL_WRITE(C2_BASE_SR(userDeviceHandle,nSpi), CA, SR0_CONFIG0, uConfig0); */
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2,SR,nSpi,SR0_CONFIG0,uConfig0);

  for ( nCalendarIdx=0; nCalendarIdx < nCalendarLength; ) {
    uValue = 0;

    /* jts - Apr 06 2006 - setup for the 32-bit write (4 entries)  */
    for ( nFieldIdx=0; nFieldIdx < nCalendarEntriesPerRegister; nFieldIdx++ ) {
      nCalendarEntry = pInitParams->sr[nSpi].nCalendarEntry[nCalendarIdx++];
      uValue |= (nCalendarEntry&0xFF)<<(8*nFieldIdx);
    }

    SAND_HAL_WRITE_INDEX_STRIDE(userDeviceHandle, C2, SR, nSpi,
                SR0_CAL_CONFIG0, (nCalendarIdx/4)-1, uValue);
  }

  nFifoBottom = 0;
  for ( nPort=0; nPort<nCalendarLength; nPort++ ) {
      if (pInitParams->st[nSpi].nFifoLines[nPort] < SB_FE2000XT_SPI_MIN_LINES_PER_PORT)
      {
          nFifoTop = SB_FE2000XT_SPI_MIN_LINES_PER_PORT;
      } else {
          nFifoTop = nFifoBottom + pInitParams->st[nSpi].nFifoLines[nPort] - 1;
      }

      uValue  = SAND_HAL_SET_FIELD(C2, SR_P0_FIFO_CONFIG, FIFO_TOP, nFifoTop);
      uValue |=
          SAND_HAL_SET_FIELD(C2, SR_P0_FIFO_CONFIG, FIFO_BOTTOM, nFifoBottom);
      SAND_HAL_WRITE_INDEX_STRIDE(userDeviceHandle, C2, SR, nSpi,
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
    uValue = SAND_HAL_MOD_FIELD(C2, SR0_P0_FRAME_SIZE, MIN_FRAME_SIZE,
                uValue, pInitParams->sr[nSpi].nMinFrameSize[nPort]);
    uValue = SAND_HAL_MOD_FIELD(C2, SR0_P0_FRAME_SIZE, MAX_FRAME_SIZE,
                uValue, pInitParams->sr[nSpi].nMaxFrameSize[nPort]);

    SAND_HAL_WRITE_INDEX_STRIDE(userDeviceHandle, C2, SR, nSpi,
                SR0_P0_FRAME_SIZE, nPort, uValue);
  }


  if ( nSpi == 0 ) {
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, SR0_CORE_RESET, 0x0);
  } else {
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, SR1_CORE_RESET, 0x0);
  }

  SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle, C2, SR, nSpi, SR0_PC_CONFIG,
                RSCLK_EDGE, (pInitParams->sr[nSpi].nRSclkEdge));

  SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle, C2, SR, nSpi, SR0_PC_CONFIG,
                BANDGAP_CTAT_ADJ, 0x3);

  uConfig0 = SAND_HAL_MOD_FIELD(C2, SR0_CONFIG0, PC_RESET, uConfig0, 0x0);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2, SR, nSpi, SR0_CONFIG0, uConfig0);

  uConfig0 = SAND_HAL_MOD_FIELD(C2, SR0_CONFIG0, RX_ENABLE, uConfig0, 1);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2, SR, nSpi, SR0_CONFIG0, uConfig0);

  return SB_FE2000_STS_INIT_OK_K;
}

static uint32_t
sbFe2000XtInitSpi(sbhandle userDeviceHandle,
        uint16_t nNum,
                sbFe2000XtInitParams_t *pInitParams)
{
  uint32_t uStatus;
  DENTER();
  uStatus = sbFe2000XtInitSpiTx(userDeviceHandle,nNum,pInitParams);
  if (uStatus != SB_FE2000_STS_INIT_OK_K) {
    SB_ERROR("%sTx init failed with status 0x%x\n",__PRETTY_FUNCTION__,uStatus);
    return uStatus;
  }
  uStatus = sbFe2000XtInitSpiRx(userDeviceHandle,nNum,pInitParams);
  if (uStatus != SB_FE2000_STS_INIT_OK_K) {
    SB_ERROR("%sRx init failed with status 0x%x\n",__PRETTY_FUNCTION__,uStatus);
    return uStatus;
  }
  DEXIT();
  return SB_FE2000_STS_INIT_OK_K;
}

static uint32_t
sbFe2000XtInitXgm(sbhandle userDeviceHandle, uint16_t nMac,
                sbFe2000XtInitParams_t *pInitParams)
{

  uint32_t uConfigData;
  uint32_t uDataLo;
  uint32_t uDataHi;
  uint32_t uData, uMemAccCtrl;
  uint32_t uStatus;
  uint32_t uScanBusy;
  uint32_t uTimeOut;
  uint32_t uE2EccConfig;
  uint32_t uPhyOrPortAddr;
  uint8_t bClause45 = TRUE;
  uint32_t uXG_debug = 0;
#if 1
  uint8_t bInternalPhy = TRUE;
  uint32_t uDevAddr = 1;
  uint32_t uMdioData;
#endif
  DENTER();

  SB_LOGV1("Initializing Xgm%d .. \n",nMac);

  /* Set mii clock to ~2.5Mhz for MDIO minimum cycle time of 400ns 
   * adjust based on sws frequncy 
   */
  uData = 0;
  uData = SAND_HAL_MOD_FIELD(C2, PC_MIIM_RATE_ADJUST, DIVIDEND, uData, 0x1);
  uData = SAND_HAL_MOD_FIELD(C2, PC_MIIM_RATE_ADJUST, DIVISOR, uData, (pInitParams->uSwsFreq / 5000));
  SAND_HAL_WRITE(userDeviceHandle, C2,  PC_MIIM_RATE_ADJUST, uData);

  /* First disable automatic scan so there will be no collisions with mmi accesses. */
  uMemAccCtrl = SAND_HAL_READ(userDeviceHandle, C2, PC_MIIM_CONTROL0);
  if ( SAND_HAL_GET_FIELD(C2, PC_MIIM_CONTROL0, MIIM_LINK_SCAN_EN, uMemAccCtrl) ) {
    uMemAccCtrl = SAND_HAL_MOD_FIELD(C2, PC_MIIM_CONTROL0, MIIM_LINK_SCAN_EN, uMemAccCtrl, 0x0);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_MIIM_CONTROL0, uMemAccCtrl);
    uTimeOut = 0;
    uScanBusy = 1;
    while ( uScanBusy && (1000 > uTimeOut) ) {
	      uScanBusy = SAND_HAL_GET_FIELD(C2, PC_MIIM_STATUS, MIIM_SCAN_BUSY, SAND_HAL_READ(userDeviceHandle, C2, PC_MIIM_STATUS) );
      uTimeOut++;
    }
    if ( uTimeOut == 1000 ) {
      SB_ERROR("PC_MIIM_STATUS-MIIM_SCAN_BUSY, timeout waiting for scan to cease.\n");
    }
  }

#if 0 
  DOUBLE fClkXgXtorRef = 6.400*1000; 
  CZMPClk* pClk = (CZMPClk*)GetModule("xgm_clk");
  ASSERT(pClk);
  for ( nNum = 0; nNum < SB_FE2000XT_NUM_XG_INTERFACES; ++nNum ) {
    if (pInitParams->xg[nNum].nXgmiiSpeed == 12) {
      fClkXgXtorRef = 5.333*1000; 
    }
    else if (pInitParams->xg[nNum].nXgmiiSpeed == 16) {
      fClkXgXtorRef = 4*1000; 
    }
    else {
      fClkXgXtorRef = 6.400*1000; 
    }
    pClk->SetClock(nNum, (uint32_t)fClkXgXtorRef,  50,  0, 0, 0);
  }
  pClk->ProgramClocks();
#endif

  /* Take the block out of reset.  Also enable indirect access to MAC.  */
  if ( nMac == 0 ) {
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, XG0_CORE_RESET, 0x0);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, XG0_CONFIG, XM_RESET, 0x0);
  } else if ( nMac == 1 ) {
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, XG1_CORE_RESET, 0x0);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, XG1_CONFIG, XM_RESET, 0x0);
  } else if ( nMac == 2 ) {
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, AG0_CORE_RESET, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, XG2_CORE_RESET, 0x0);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, XG2_CONFIG, XM_RESET, 0x0);
  } else if ( nMac == 3 ) {
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, AG1_CORE_RESET, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, XG3_CORE_RESET, 0x0);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, XG3_CONFIG, XM_RESET, 0x0);
  }

  /* jts - Dec 29 2006 - workaround for bug 23963 */
  uXG_debug = SAND_HAL_READ_STRIDE(userDeviceHandle, C2, XG, nMac, XG_DEBUG);
  uXG_debug = SAND_HAL_MOD_FIELD(C2, XG_DEBUG, TI_TX_THRESHOLD, uXG_debug,0x18);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2, XG, nMac, XG_DEBUG, uXG_debug);

  /* Configure E2ecc controls */
  uData = SAND_HAL_MOD_FIELD(C2, XM_MAC_CONFIG_LO, TX_ENABLE, uData, pInitParams->xg[nMac].bBringUp);
  {
    uE2EccConfig = SAND_HAL_READ_STRIDE(userDeviceHandle, C2, XG, nMac, XG_E2ECC_CONFIG0);
    uE2EccConfig = SAND_HAL_MOD_FIELD(C2, XG_E2ECC_CONFIG0, TX_ENABLE, uE2EccConfig, pInitParams->xgt[nMac].bE2eccEnable);
    /* rgf - Nov 16 2007 - CRC mode in TX_MAC_CONFIG needs to be congruent with the CRC mode in the E2ECC CONFIG */
    /* rgf - Nov 16 2007 - Ensure this is the case by driving both from the same init param */
    if( pInitParams->xgt[nMac].nCrcMode == SB_FE2000_XG_MAC_CRC_APPEND ) { 
      uE2EccConfig = SAND_HAL_MOD_FIELD(C2, XG_E2ECC_CONFIG0, CRC_MODE, uE2EccConfig, 0);
    } else {
      uE2EccConfig = SAND_HAL_MOD_FIELD(C2, XG_E2ECC_CONFIG0, CRC_MODE, uE2EccConfig, 2);
    }
    uE2EccConfig = SAND_HAL_MOD_FIELD(C2, XG_E2ECC_CONFIG0, TX_PERIOD, uE2EccConfig, pInitParams->xgt[nMac].uE2eccPeriod);
    uE2EccConfig = SAND_HAL_MOD_FIELD(C2, XG_E2ECC_CONFIG0, TX_PERIOD, uE2EccConfig, pInitParams->xg[nMac].nE2eccBitmap);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2, XG, nMac, XG_E2ECC_CONFIG0, uE2EccConfig);

    uE2EccConfig = SAND_HAL_READ_STRIDE(userDeviceHandle, C2, XG, nMac, XG_E2ECC_CONFIG1);
    uE2EccConfig = SAND_HAL_MOD_FIELD(C2, XG_E2ECC_CONFIG1, FRAME_LENGTH, uE2EccConfig, pInitParams->xgt[nMac].uE2eccFrameLength);
    uE2EccConfig = SAND_HAL_MOD_FIELD(C2, XG_E2ECC_CONFIG1, PORT_ENABLE, uE2EccConfig, pInitParams->xgt[nMac].uE2eccPortEnable);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2, XG, nMac, XG_E2ECC_CONFIG1, uE2EccConfig);
  }
      
  SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle, C2, XG, nMac, XG_E2ECC_FRAME3, CTL_OP, pInitParams->xgt[nMac].uE2eccCtlOp);
  SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle, C2, XG, nMac, XG_E2ECC_FRAME3, ETHERTYPE, pInitParams->xgt[nMac].uE2eccEtherType);
  SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle, C2, XG, nMac, XG_E2ECC_FRAME4, NUM_PORTS, pInitParams->xgt[nMac].uE2eccNumPorts);

  /* Config the XGXS via PCI control registers. */
  uConfigData = 0;
  uConfigData = SAND_HAL_MOD_FIELD(C2, PC_MIIM_XGXS_CONFIG0,
                   PHY_CLAUSE_45, uConfigData, bClause45);
  uConfigData = SAND_HAL_MOD_FIELD(C2, PC_MIIM_XGXS_CONFIG0,
                   MDIO_DEVAD, uConfigData, 1);
  uConfigData = SAND_HAL_MOD_FIELD(C2, PC_MIIM_XGXS_CONFIG0,
                   IEEE_DEVICES_IN_PKG, uConfigData, 0x1e);
  SAND_HAL_WRITE_INDEX(userDeviceHandle, C2, PC_MIIM_XGXS_CONFIG0, nMac, uConfigData);
  uPhyOrPortAddr = SB_FE2000XT_XGXS0_PHY_ADDR + nMac;

  uStatus = sbFe2000XtTakeXgXsOutOfReset(userDeviceHandle, nMac);
  if( SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  /* Config the XGXS via MDIO */
  if (!pInitParams->bSimulation) {
    /* First read is bogus.  Need to have 32 preamble bits generated after the clause45 mode switch above.*/
    sbFe2000UtilXgmMiimRead( userDeviceHandle, bInternalPhy, bClause45, uDevAddr, uPhyOrPortAddr, (uint32_t) 0xffde, &uMdioData);
    /* Clear start sequencer bit.  This commands XGXS_CL73 PLL into a reset state, and forces all of XGXS_CL73 datapaths into a reset state. */
    sbFe2000UtilXgmMiimRead( userDeviceHandle, bInternalPhy, bClause45, uDevAddr, uPhyOrPortAddr, (uint32_t) 0x8000, &uMdioData);
    sbFe2000UtilXgmMiimWrite(userDeviceHandle, bInternalPhy, bClause45, uDevAddr, uPhyOrPortAddr, (uint32_t) 0x8000, (uMdioData & 0xdfff));
    /* Disable auto-neg */
    sbFe2000UtilXgmMiimRead( userDeviceHandle, bInternalPhy, bClause45, uDevAddr, uPhyOrPortAddr, (uint32_t) 0xffe0, &uMdioData);
    sbFe2000UtilXgmMiimWrite(userDeviceHandle, bInternalPhy, bClause45, uDevAddr, uPhyOrPortAddr, (uint32_t) 0xffe0, (uMdioData & 0xefff));
    /* For speeds which exceed 1G set the force_speed bits of the register Misc1 (0x8308). */
    /* Note that bit 4 of this register serves as a speed expansion bit, allowing rates which exceed 1G to be programmed.*/
    /* Bits 3:0 are mapped to each of the various rates supported by XGXS_CL73.  Refer to XGXS_CL73 Register map for a complete description. */
    sbFe2000UtilXgmMiimRead( userDeviceHandle, bInternalPhy, bClause45, uDevAddr, uPhyOrPortAddr, (uint32_t) 0x8308, &uMdioData);
    uMdioData |= ( pInitParams->xg[nMac].nXgmiiSpeed >= 10 ) ? 0x10 : 0x0;
    switch (pInitParams->xg[nMac].nXgmiiSpeed) {
      case 10:
        uMdioData |= 0x4; 
        break;
      case 12:
        uMdioData |= 0x5; 
        break;
      case 16:
        uMdioData |= 0x9; 
        break;
      default:
        break;
    }
    sbFe2000UtilXgmMiimWrite(userDeviceHandle, bInternalPhy, bClause45, uDevAddr, uPhyOrPortAddr, (uint32_t) 0x8308, uMdioData);
    if ( pInitParams->xg[nMac].bSerdesLocalLoopback == TRUE ) {
      sbFe2000UtilXgmMiimWrite(userDeviceHandle, bInternalPhy, bClause45, uDevAddr, uPhyOrPortAddr, (uint32_t) 0xffde, 0x0800);
      sbFe2000UtilXgmMiimRead(userDeviceHandle,  bInternalPhy, bClause45, uDevAddr, uPhyOrPortAddr, (uint32_t) 0x0000, &uMdioData);
      sbFe2000UtilXgmMiimWrite(userDeviceHandle, bInternalPhy, bClause45, uDevAddr, uPhyOrPortAddr, (uint32_t) 0x0000, (uMdioData | 0x0001));
      sbFe2000UtilXgmMiimWrite(userDeviceHandle, bInternalPhy, bClause45, uDevAddr, uPhyOrPortAddr, (uint32_t) 0xffde, 0x0000);
    }
    /* Set start sequencer bit.*/
    sbFe2000UtilXgmMiimRead( userDeviceHandle, bInternalPhy, bClause45, uDevAddr, uPhyOrPortAddr, (uint32_t) 0x8000, &uMdioData);
    sbFe2000UtilXgmMiimWrite(userDeviceHandle, bInternalPhy, bClause45, uDevAddr, uPhyOrPortAddr, (uint32_t) 0x8000, (uMdioData | 0x2000));
  }

  /* Config 10G MAC main control registers.  */
  /* TX  */
  uDataLo = 0;
  uDataHi=0;
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_TX_CONFIG_LO, HDR_MODE,
                   uDataLo, pInitParams->xgt[nMac].nHeaderMode);
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_TX_CONFIG_LO, CRC_MODE,
                   uDataLo, pInitParams->xgt[nMac].nCrcMode);
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_TX_CONFIG_LO, AVG_IPG,
                   uDataLo, pInitParams->xgt[nMac].nAvgIpg);
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_TX_CONFIG_LO, THROTTLE_NUM,
                   uDataLo, pInitParams->xgt[nMac].nThrotNumer);
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_TX_CONFIG_LO, THROTTLE_DENOM,
                   uDataLo, pInitParams->xgt[nMac].nThrotDenom);
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_TX_CONFIG_LO, PAUSE_ENABLE,
                   uDataLo, pInitParams->xgt[nMac].bPauseEnable);
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_TX_CONFIG_LO, DROP_ALL,
                   uDataLo, pInitParams->xgt[nMac].bDiscard);
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_TX_CONFIG_LO, ANY_START,
                   uDataLo, pInitParams->xgt[nMac].bAnyStart);
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_TX_CONFIG_LO, HIGIG2_MODE,
                   uDataLo, pInitParams->xgt[nMac].nHiGig2Mode);
  uStatus = sbFe2000UtilXgmWrite(userDeviceHandle, nMac, SAND_HAL_REG_OFFSET(C2, XM_MAC_TX_CONFIG_LO), uDataHi, uDataLo);
  if( SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  /* RX  */
  uDataLo = 0;
  uDataHi=0;
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_RX_CONFIG_LO, HDR_MODE,
                   uDataLo, pInitParams->xgr[nMac].nHeaderMode);
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_RX_CONFIG_LO, STRIP_CRC,
                   uDataLo, pInitParams->xgr[nMac].bStripCrc);
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_RX_CONFIG_LO, IGNORE_CRC,
                   uDataLo, pInitParams->xgr[nMac].bIgnoreCrc);
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_RX_CONFIG_LO, STRICT_PREAMBLE,
                   uDataLo, pInitParams->xgr[nMac].bStrictPreamble);
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_RX_CONFIG_LO, RX_PAUSE_ENABLE,
                   uDataLo, pInitParams->xgr[nMac].bPauseEnable);
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_RX_CONFIG_LO, RX_PASS_CTRL,
                   uDataLo, pInitParams->xgr[nMac].bPassCtrl);
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_RX_CONFIG_LO, ANY_START,
                   uDataLo, pInitParams->xgr[nMac].bAnyStart);
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_RX_CONFIG_LO, HIGIG2_MODE,
                   uDataLo, pInitParams->xgr[nMac].nHiGig2Mode);
  uStatus = sbFe2000UtilXgmWrite(userDeviceHandle, nMac, SAND_HAL_REG_OFFSET(C2, XM_MAC_RX_CONFIG_LO), uDataHi, uDataLo);
  if( SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  /* Config 10G MAC RX, TX MAX_SIZE */
  uDataHi = 0;
  uStatus = sbFe2000UtilXgmWrite(userDeviceHandle, nMac, SAND_HAL_REG_OFFSET(C2, XM_MAC_RX_CONFIG_LO),
                 uDataHi, pInitParams->xgt[nMac].nMaxSize);
  if( SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  uStatus = sbFe2000UtilXgmWrite(userDeviceHandle, nMac, uDataHi,SAND_HAL_REG_OFFSET(C2, XM_MAC_TX_CONFIG_LO),
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
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_CONFIG_LO, REMOTE_LOOPBACK_ENABLE,
                   uDataLo, pInitParams->xg[nMac].bRemoteLoopback);
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_CONFIG_LO, LOCAL_LOOPBACK_ENABLE,
                   uDataLo, pInitParams->xg[nMac].bLocalLoopback);
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_CONFIG_LO, RX_ENABLE,
                   uDataLo, pInitParams->xg[nMac].bBringUp);
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_CONFIG_LO, TX_ENABLE,
                   uDataLo, pInitParams->xg[nMac].bBringUp);

  uStatus = sbFe2000UtilXgmWrite(userDeviceHandle, nMac, SAND_HAL_REG_OFFSET(C2, XM_MAC_CONFIG_LO),uDataHi, uDataLo);

  if( SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }
#endif

  /* Start automatic link status scanning */

  /* Scan all possible Xg ports */
  uMemAccCtrl = SAND_HAL_READ(userDeviceHandle, C2, PC_MIIM_SCAN_PORTS);
  uData = SAND_HAL_GET_FIELD(C2, PC_MIIM_SCAN_PORTS, MIIM_SCAN_PORTS, uMemAccCtrl);
  uData |= 0x3000000;
  uMemAccCtrl = SAND_HAL_MOD_FIELD(C2, PC_MIIM_SCAN_PORTS, MIIM_SCAN_PORTS, uMemAccCtrl, uData);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PC_MIIM_SCAN_PORTS, uMemAccCtrl);

  /* Scan internal ports */
  uMemAccCtrl = SAND_HAL_READ(userDeviceHandle, C2, PC_MIIM_INT_SEL_MAP);
  uData = SAND_HAL_GET_FIELD(C2, PC_MIIM_INT_SEL_MAP, MIIM_INT_SEL_MAP, uMemAccCtrl);
  uData |= 0x3000000;
  uMemAccCtrl = SAND_HAL_MOD_FIELD(C2, PC_MIIM_INT_SEL_MAP, MIIM_INT_SEL_MAP, uMemAccCtrl, uData);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PC_MIIM_INT_SEL_MAP, uMemAccCtrl);

  /* Enable automatic scan */
  /* Keep off, GNATS 24612 - conflicts with phy probing */
  uMemAccCtrl = SAND_HAL_READ(userDeviceHandle, C2, PC_MIIM_CONTROL0);
  uMemAccCtrl = SAND_HAL_MOD_FIELD(C2, PC_MIIM_CONTROL0, MIIM_LINK_SCAN_EN, uMemAccCtrl, 0x0);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PC_MIIM_CONTROL0, uMemAccCtrl);

  DEXIT();
  return SB_FE2000_STS_INIT_OK_K;
}

sbStatus_t
sbFe2000XtInitXgmRxTx(sbhandle userDeviceHandle, uint16_t nMac,
                 sbFe2000XtInitParams_t *pInitParams)
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
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_CONFIG_LO, REMOTE_LOOPBACK_ENABLE,
                               uDataLo, pInitParams->xg[nMac].bRemoteLoopback);
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_CONFIG_LO, LOCAL_LOOPBACK_ENABLE,
                               uDataLo, pInitParams->xg[nMac].bLocalLoopback);
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_CONFIG_LO, RX_ENABLE,
                               uDataLo, pInitParams->xg[nMac].bBringUp);
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_CONFIG_LO, TX_ENABLE,
                               uDataLo, pInitParams->xg[nMac].bBringUp);

  uStatus = sbFe2000UtilXgmWrite(userDeviceHandle, nMac, SAND_HAL_REG_OFFSET(C2, XM_MAC_CONFIG_LO),uDataHi, uDataLo);

  if( SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  return SB_FE2000_STS_INIT_OK_K;
}

static uint32_t
sbFe2000XtInitAgmPortRx(sbhandle userDeviceHandle, uint16_t nAgmNum, uint16_t nPort,
              sbFe2000XtInitParams_t *pInitParams)
{

  uint32_t uData, uSetting;
  uint32_t uStatus;

  DENTER();

  /* Command Config register setup  */
  uStatus = sbFe2000UtilAgmRead(userDeviceHandle, nAgmNum, nPort,
                SAND_HAL_REG_OFFSET(C2, AM_CONFIG), &uData);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  uData = SAND_HAL_MOD_FIELD(C2, AM_CONFIG, RX_ENABLE, uData, 0x1);

  uSetting = ( pInitParams->ag[nAgmNum].common[nPort].bMacRemoteLoopback == TRUE ) ? 0x1 : 0x0;
  uData = SAND_HAL_MOD_FIELD(C2, AM_CONFIG, LINE_LOOPBACK_ENABLE, uData, uSetting);

  uSetting = ( pInitParams->ag[nAgmNum].common[nPort].bMacLocalLoopback == TRUE ) ? 0x1 : 0x0;
  uData = SAND_HAL_MOD_FIELD(C2, AM_CONFIG, LOOPBACK_ENABLE, uData, uSetting);

  uSetting = ( pInitParams->ag[nAgmNum].common[nPort].bSpeed1000Mbps == TRUE ) ? 0x2 :
    (( pInitParams->ag[nAgmNum].common[nPort].bSpeed100Mbps == TRUE ) ? 0x1 : 0x0);
  uData = SAND_HAL_MOD_FIELD(C2, AM_CONFIG, MAC_SPEED, uData, uSetting);

  uSetting = ( pInitParams->ag[nAgmNum].common[nPort].bFullDuplex == FALSE ) ? 0x1 : 0x0;
  uData = SAND_HAL_MOD_FIELD(C2, AM_CONFIG, HALF_DUPLEX, uData, uSetting);

  uSetting = ( pInitParams->ag[nAgmNum].rx[nPort].bPromiscuousEnable == TRUE ) ? 0x1 : 0x0;
  uData = SAND_HAL_MOD_FIELD(C2, AM_CONFIG, PROMISCUOUS_ENABLE, uData, uSetting);

  uSetting = ( pInitParams->ag[nAgmNum].rx[nPort].bCrcForward == TRUE ) ? 0x1 : 0x0;
  uData = SAND_HAL_MOD_FIELD(C2, AM_CONFIG, FORWARD_CRC, uData, uSetting);

  uSetting = ( pInitParams->ag[nAgmNum].rx[nPort].bPauseForward == TRUE ) ? 0x1 : 0x0;
  uData = SAND_HAL_MOD_FIELD(C2, AM_CONFIG, FORWARD_PAUSE_FRAMES, uData, uSetting);

  uSetting = ( pInitParams->ag[nAgmNum].rx[nPort].bPayloadLengthCheck == FALSE ) ? 0x1 : 0x0;
  uData = SAND_HAL_MOD_FIELD(C2, AM_CONFIG, LENGTH_CHECK_DISABLE, uData, uSetting);

  uSetting = ( pInitParams->ag[nAgmNum].rx[nPort].bMacControlEnable == TRUE ) ? 0x1 : 0x0;
  uData = SAND_HAL_MOD_FIELD(C2, AM_CONFIG, CONTROL_FRAME_ENABLE, uData, uSetting);

  uStatus = sbFe2000UtilAgmWrite(userDeviceHandle, nAgmNum, nPort, SAND_HAL_REG_OFFSET(C2, AM_CONFIG), uData);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }


  /* MAC_0 -- Core MAC address bit 47 to 16  */
  uStatus = sbFe2000UtilAgmWrite(userDeviceHandle, nAgmNum, nPort,
                 SAND_HAL_REG_OFFSET(C2, AM_MAC_ADDRESS0),
                 pInitParams->ag[nAgmNum].common[nPort].uuMacAddr >> 16);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }


  /* MAC_1 -- Core MAC address bit 15 to 0  */
  uStatus = sbFe2000UtilAgmWrite(userDeviceHandle, nAgmNum, nPort,
                 SAND_HAL_REG_OFFSET(C2, AM_MAC_ADDRESS1),
                 pInitParams->ag[nAgmNum].common[nPort].uuMacAddr & 0xffff);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  /* Rx MaxTU  */
  uStatus = sbFe2000UtilAgmWrite(userDeviceHandle, nAgmNum, nPort,
                 SAND_HAL_REG_OFFSET(C2, AM_RX_MAX_PKT_LENGTH),
                 pInitParams->ag[nAgmNum].rx[nPort].uMaxTransferUnit);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  DEXIT();
  return SB_FE2000_STS_INIT_OK_K;
}

static uint32_t
sbFe2000XtInitAgmPortTx(sbhandle userDeviceHandle, uint16_t nAgmNum, uint16_t nPort,
              sbFe2000XtInitParams_t *pInitParams)
{

  uint32_t uData;
  uint32_t uStatus;

  DENTER();
  /* Command Config register setup  */
  uStatus = sbFe2000UtilAgmRead(userDeviceHandle, nAgmNum, nPort, SAND_HAL_REG_OFFSET(C2, AM_CONFIG), &uData);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  uData = SAND_HAL_MOD_FIELD(C2, AM_CONFIG, TX_ENABLE, uData, 0x1);
  if ( pInitParams->ag[nAgmNum].tx[nPort].bPadEnable == TRUE ) {
    uData = SAND_HAL_MOD_FIELD(C2, AM_CONFIG, PAD_TX_FRAMES, uData, 0x1);
  }

  uStatus = sbFe2000UtilAgmWrite(userDeviceHandle, nAgmNum, nPort, SAND_HAL_REG_OFFSET(C2, AM_CONFIG), uData);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  /* Tx IPG Length register setup  */
  uData = 0xc;
  uStatus = sbFe2000UtilAgmWrite(userDeviceHandle, nAgmNum, nPort,
                                 SAND_HAL_REG_OFFSET(C2, AM_TX_IPG_LENGTH),
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
    int portStride = SAND_HAL_REG_OFFSET(C2, AG_MAC1_CONFIG)
        - SAND_HAL_REG_OFFSET(C2, AG_MAC0_CONFIG);

    return SAND_HAL_C2_AG_INSTANCE_ADDR_STRIDE * nAgmNum
        + SAND_HAL_REG_OFFSET(C2, AG_MAC0_CONFIG) + portStride * nPort;
}

static uint32_t
sbFe2000XtTakeXgXsOutOfReset(sbhandle userDeviceHandle, uint32_t nMac) {
  uint32_t uDataLo, uDataHi, uStatus;

  /*
   *  Config the XGXS via 10G MAC control register.
   *  Lo bit ------------------------------------------------------------------- Hi bit
   *  DIGITAL_CLK_SHUTDOWN=0, ANALOG_CLK_SHUTDOWN=0, RESET_N=0, XGXS_MDIO_RESET_N=0
   *  XGXS_PLL_RESET_N=0, BIGMAC_RESET_N=0, TX_FIFO_RESET_N=0, TX_FIFO_AUTO_RESET_ENABLE=1
   *  BYPASS_PLL=0, SELECT_LOCAL_PLL=1, RX_LANE_SWAP=0, TX_LANE_SWAP=0,
   *  BOOTSTRAP_MODE=0001
   *  FORCE_BOOTSTRAP_MODE=1, REMOTE_LOOPBACK_ENABLE=0, RESET_FILTER_BYPASS=0
   */

  uStatus = sbFe2000UtilXgmBigMacRead(userDeviceHandle,nMac,SAND_HAL_REG_OFFSET(C2, XM_MAC_XGXS_CONFIG_LO), &uDataHi, &uDataLo); /* MAC_XGXS_CONFIG Indirect */
  if( SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  /* Power up, force mode */

  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_XGXS_CONFIG_LO, ANALOG_CLK_SHUTDOWN, uDataLo, 0x0);
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_XGXS_CONFIG_LO, DIGITAL_CLK_SHUTDOWN, uDataLo, 0x0);
  uStatus = sbFe2000UtilXgmBigMacWrite(userDeviceHandle,nMac,SAND_HAL_REG_OFFSET(C2, XM_MAC_XGXS_CONFIG_LO), uDataHi, uDataLo); /* MAC_XGXS_CONFIG Indirect */
  if( SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }
  /* Select LCPLL */
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_XGXS_CONFIG_LO, SELECT_LOCAL_PLL, uDataLo, 0x1);
  uStatus = sbFe2000UtilXgmBigMacWrite(userDeviceHandle,nMac, SAND_HAL_REG_OFFSET(C2, XM_MAC_XGXS_CONFIG_LO), uDataHi, uDataLo); /* MAC_XGXS_CONFIG Indirect */
  if( SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  /* Deassert reset */
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_XGXS_CONFIG_LO, RESET_N, uDataLo, 0x1);
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_XGXS_CONFIG_LO, PLL_RESET_N, uDataLo, 0x1);
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_XGXS_CONFIG_LO, MDIOREGS_RESET_N, uDataLo, 0x1);
  uStatus = sbFe2000UtilXgmBigMacWrite(userDeviceHandle,nMac, SAND_HAL_REG_OFFSET(C2, XM_MAC_XGXS_CONFIG_LO), uDataHi, uDataLo); /* MAC_XGXS_CONFIG Indirect */
  if( SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  /* Deassert TX FIFO reset */
  uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_XGXS_CONFIG_LO, TX_FIFO_RESET_N, uDataLo, 0x1);
  uStatus = sbFe2000UtilXgmBigMacWrite(userDeviceHandle,nMac, SAND_HAL_REG_OFFSET(C2, XM_MAC_XGXS_CONFIG_LO), uDataHi, uDataLo); /* MAC_XGXS_CONFIG Indirect */

  return uStatus;

}
static uint32_t
sbFe2000XtInitAgm(sbhandle userDeviceHandle, uint16_t nAgmNum, sbFe2000XtInitParams_t *pInitParams)
{

  int32_t  nPort;
  uint32_t uData;
  uint32_t uStatus;
  uint32_t uPhyOrPortAddr;
  uint8_t bInternalPhy = TRUE;
  uint8_t bClause45 = TRUE;
  uint32_t uDevAddr = 1;
  uint32_t uMdioData;
  uint32_t uMdioDataSave;
  uint32_t uXgXsConfig;

  DENTER();
  SB_LOGV1("Initializing Agm%d .. \n",nAgmNum);

  /* uData = SAND_HAL_READ(C2_BASE_AG(userDeviceHandle,nAgmNum), CA, AG_CONFIG); */
  uData = SAND_HAL_READ_STRIDE(userDeviceHandle, C2,AG,nAgmNum,AG_CONFIG);

  uData = SAND_HAL_MOD_FIELD(C2, AG_CONFIG, SERDES_RESET_N, uData, 0x1);
  uData = SAND_HAL_MOD_FIELD(C2, AG_CONFIG, GPORT_RESET_N, uData, 0x1);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2, AG, nAgmNum, AG_CONFIG, uData);


  if ( pInitParams->ag[nAgmNum].bSwsLoopBackEnable == TRUE ) {
    /* uData = SAND_HAL_READ(C2_BASE_AG(userDeviceHandle,nAgmNum), CA, AG_DEBUG); */
    uData = SAND_HAL_READ_STRIDE(userDeviceHandle, C2,AG,nAgmNum,AG_DEBUG);
    uData = SAND_HAL_MOD_FIELD(C2, AG_DEBUG, SWS_LOOPBACK_ENABLE, uData, 0x1);
    uData = SAND_HAL_MOD_FIELD(C2, AG_DEBUG, SWS_LOOPBACK_CRC_MODE, uData,
                   pInitParams->ag[nAgmNum].uSwsLoopBackCrcMode);
    SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2, AG, nAgmNum, AG_DEBUG, uData);
  }
#if 0 /* verification does this a bit later */
  for ( nPort = 0; nPort < SB_FE2000_MAX_AG_PORTS; nPort++ ) {
    uStatus = sbFe2000XtInitAgmPortRx(userDeviceHandle, nAgmNum, nPort, pInitParams);
    if( SB_FE2000_STS_INIT_OK_K != uStatus ) {
      return uStatus;
    }
    uStatus = sbFe2000XtInitAgmPortTx(userDeviceHandle, nAgmNum, nPort, pInitParams);
    if( SB_FE2000_STS_INIT_OK_K != uStatus ) {
      return uStatus;
    }
  }
#endif
  /* GPORT_CONFIG  */
  uStatus = sbFe2000UtilAgmRead(userDeviceHandle, nAgmNum, SB_FE2000_MAX_AG_PORTS,
                SAND_HAL_REG_OFFSET(C2, GPORT_CONFIG), &uData);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  uData = SAND_HAL_MOD_FIELD(C2, GPORT_CONFIG, CLEAR_MIB_COUNTERS, uData, 0x1);
  uStatus = sbFe2000UtilAgmWrite(userDeviceHandle, nAgmNum, SB_FE2000_MAX_AG_PORTS,
                 SAND_HAL_REG_OFFSET(C2, GPORT_CONFIG), uData);

  uStatus = sbFe2000UtilAgmRead(userDeviceHandle, nAgmNum, SB_FE2000_MAX_AG_PORTS,
                SAND_HAL_REG_OFFSET(C2, GPORT_CONFIG), &uData);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  uData = SAND_HAL_MOD_FIELD(C2, GPORT_CONFIG, CLEAR_MIB_COUNTERS, uData, 0x0);
  uData = SAND_HAL_MOD_FIELD(C2, GPORT_CONFIG, ENABLE, uData, 0x1);
  uStatus = sbFe2000UtilAgmWrite(userDeviceHandle, nAgmNum, SB_FE2000_MAX_AG_PORTS,
                 SAND_HAL_REG_OFFSET(C2, GPORT_CONFIG), uData);

  /* RSV mask register -- Control which RSV bits cause packets to be purged  */
  uStatus = sbFe2000UtilAgmRead(userDeviceHandle, nAgmNum, SB_FE2000_MAX_AG_PORTS,
                SAND_HAL_REG_OFFSET(C2, GPORT_RX_PURGE_CONFIG), &uData);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  uData |= 0x00000040;  /* Drop "frame length out of range" packets  */
  if ( pInitParams->ag[nAgmNum].bCrcCheck == FALSE ) {
    uData &= 0xffffffef;
  }
  uStatus = sbFe2000UtilAgmWrite(userDeviceHandle, nAgmNum, SB_FE2000_MAX_AG_PORTS,
                 SAND_HAL_REG_OFFSET(C2, GPORT_RX_PURGE_CONFIG), uData);

  /*
   * RSV stat mask register --
   * Control which RSV events should cause a statistic counter update
   */

  uStatus = sbFe2000UtilAgmRead(userDeviceHandle, nAgmNum, SB_FE2000_MAX_AG_PORTS,
                SAND_HAL_REG_OFFSET(C2, GPORT_RX_STAT_UPDATE_CONFIG), &uData);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  if ( pInitParams->ag[nAgmNum].bCrcCheck == FALSE ) {
    uData &= 0xffffffef;
  }

  uStatus = sbFe2000UtilAgmWrite(userDeviceHandle, nAgmNum, SB_FE2000_MAX_AG_PORTS,
                 SAND_HAL_REG_OFFSET(C2, GPORT_RX_STAT_UPDATE_CONFIG), uData);
  if(SB_FE2000_STS_INIT_OK_K != uStatus) {
    return uStatus;
  }

  /*
   * GPORT CNTMAXSIZE register -- The max packet size that is used
   * in statistic counter update
   */
  uStatus = sbFe2000UtilAgmWrite(userDeviceHandle, nAgmNum, SB_FE2000_MAX_AG_PORTS, SAND_HAL_REG_OFFSET(C2, GPORT_MAX_PKT_SIZE_STATUS_UPDATE),
                 pInitParams->ag[nAgmNum].uMaxStatCounterUnit);

    if ( (!pInitParams->bSimulation) &&
       (pInitParams->ag[nAgmNum].common[8].bBringUp ||  pInitParams->ag[nAgmNum].common[9].bBringUp ||
        pInitParams->ag[nAgmNum].common[10].bBringUp || pInitParams->ag[nAgmNum].common[11].bBringUp) ) {

      uXgXsConfig = 0;
      uXgXsConfig = SAND_HAL_MOD_FIELD(C2, PC_MIIM_XGXS_CONFIG0, PHY_CLAUSE_45, uXgXsConfig, bClause45);
      uXgXsConfig = SAND_HAL_MOD_FIELD(C2, PC_MIIM_XGXS_CONFIG0, MDIO_DEVAD, uXgXsConfig, uDevAddr);
      uXgXsConfig = SAND_HAL_MOD_FIELD(C2, PC_MIIM_XGXS_CONFIG0, IEEE_DEVICES_IN_PKG, uXgXsConfig, 0x1e);
  
      if ( nAgmNum == 0 ) {
        SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, XG2_CORE_RESET, 0x0);
        SAND_HAL_RMW_FIELD(userDeviceHandle, C2, XG2_CONFIG, XM_RESET, 0x0);
        SAND_HAL_WRITE(userDeviceHandle, C2, PC_MIIM_XGXS_CONFIG2, uXgXsConfig);
      } else {
        SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, XG3_CORE_RESET, 0x0);
        SAND_HAL_RMW_FIELD(userDeviceHandle, C2, XG3_CONFIG, XM_RESET, 0x0);
        SAND_HAL_WRITE(userDeviceHandle, C2, PC_MIIM_XGXS_CONFIG3, uXgXsConfig);
      } 
  
  
      /* Take the appropriate XGXS(Ag0->2, Ag1->3) out of reset.*/
      uStatus = sbFe2000XtTakeXgXsOutOfReset(userDeviceHandle, 2+nAgmNum);
      if( SB_FE2000_STS_INIT_OK_K != uStatus ) {
        return uStatus;
      }
  
      uPhyOrPortAddr = SB_FE2000XT_XGXS2_PHY_ADDR + nAgmNum; 
      /* First read is bogus.  Need to have 32 preamble bits generated after the clause45 mode switch above.*/
      sbFe2000UtilXgmMiimRead( userDeviceHandle, bInternalPhy, bClause45, uDevAddr, uPhyOrPortAddr, (uint32_t) 0xffde, &uMdioData);
      /* Clear start sequencer bit.  This commands XGXS_CL73 PLL into a reset state, and forces all of XGXS_CL73 datapaths into a reset state. */
      /* Set "mode_10g" to independent clock control*/
      sbFe2000UtilXgmMiimRead( userDeviceHandle, bInternalPhy, bClause45, uDevAddr, uPhyOrPortAddr, (uint32_t) 0x8000, &uMdioDataSave);
      uMdioDataSave &= 0xd0ff;
      uMdioDataSave |= 0x0600;
      sbFe2000UtilXgmMiimWrite( userDeviceHandle, bInternalPhy, bClause45, uDevAddr, uPhyOrPortAddr, (uint32_t) 0x8000, uMdioDataSave);
      /* Enable lanes via XGXS BLOCK1 "laneCtrl0" register */
      sbFe2000UtilXgmMiimRead(  userDeviceHandle, bInternalPhy, bClause45, uDevAddr, uPhyOrPortAddr, (uint32_t) 0x8015, &uMdioData);
      if ( pInitParams->ag[nAgmNum].common[8].bBringUp ) uMdioData |= 0x11;
      if ( pInitParams->ag[nAgmNum].common[9].bBringUp ) uMdioData |= 0x22;
      if ( pInitParams->ag[nAgmNum].common[10].bBringUp ) uMdioData |= 0x44;
      if ( pInitParams->ag[nAgmNum].common[11].bBringUp ) uMdioData |= 0x88;
      sbFe2000UtilXgmMiimWrite( userDeviceHandle, bInternalPhy, bClause45, uDevAddr, uPhyOrPortAddr, (uint32_t) 0x8015, uMdioData);
  
    /* verifcation does this here...rather than above */
    for ( nPort = 0; nPort < SB_FE2000_MAX_AG_PORTS; nPort++ ) {
      uStatus = sbFe2000XtInitAgmPortRx(userDeviceHandle, nAgmNum, nPort, pInitParams);
      if( SB_FE2000_STS_INIT_OK_K != uStatus ) {
        return uStatus;
      }
      uStatus = sbFe2000XtInitAgmPortTx(userDeviceHandle, nAgmNum, nPort, pInitParams);
      if( SB_FE2000_STS_INIT_OK_K != uStatus ) {
        return uStatus;
      }
    }

    /* Set start sequencer bit.*/
    sbFe2000UtilXgmMiimWrite(userDeviceHandle, bInternalPhy, bClause45, uDevAddr, uPhyOrPortAddr, (uint32_t) 0x8000, (uMdioDataSave | 0x2000));

    /* XG2 and XG3 must be placed in reset before traffic is enabled.  AG0 and XG2 can NOT both be out of reset.
     *                                                                 AG1 and XG3 can NOT both be out of reset. */

    if ( nAgmNum == 0 ) {
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, XG2_CORE_RESET, 0x1);
    } else {
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, XG3_CORE_RESET, 0x1);
    } 
  }

  DEXIT();
  return uStatus;
}

static uint32_t
sbFe2000XtInitAgms(sbhandle userDeviceHandle,
                 sbFe2000XtInitParams_t *pInitParams)
{
  int32_t nAgmNum;
  uint32_t uData, uMemAccCtrl;
  uint32_t uStatus;

  DENTER();

  if ( pInitParams->ag[0].bBringUp || pInitParams->ag[1].bBringUp ) {
    /* Set mii clock to ~2.5Mhz for MDIO minimum cycle time of 400ns */
    uData = 0;
    uData = SAND_HAL_MOD_FIELD(CA, PC_MIIM_RATE_ADJUST, DIVIDEND, uData, 0x1);
  uData = SAND_HAL_MOD_FIELD(C2, PC_MIIM_RATE_ADJUST, DIVISOR, uData, (pInitParams->uSwsFreq / 5000));
    SAND_HAL_WRITE(userDeviceHandle, CA,  PC_MIIM_RATE_ADJUST, uData);
  }

  for ( nAgmNum = 0; nAgmNum < SB_FE2000_NUM_AG_INTERFACES; ++nAgmNum ) {
    if ( pInitParams->ag[nAgmNum].bBringUp ) {
      /* Take the block out of reset.  Also enable indirect access to MAC.  */
      if ( nAgmNum == 0 ) {
          SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, AG0_CORE_RESET, 0x0);
      } else {
          SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, AG1_CORE_RESET, 0x0);
      }

      uStatus = sbFe2000XtInitAgm(userDeviceHandle, nAgmNum, pInitParams);
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
    uMemAccCtrl = SAND_HAL_MOD_FIELD(C2, PC_MIIM_SCAN_PORTS,
                     MIIM_SCAN_PORTS, uMemAccCtrl, 0xffffff);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_MIIM_SCAN_PORTS, uMemAccCtrl);

    /* Scan internal ports  */
    uMemAccCtrl = 0;
    uMemAccCtrl = SAND_HAL_MOD_FIELD(C2, PC_MIIM_INT_SEL_MAP, MIIM_INT_SEL_MAP, uMemAccCtrl, 0xffffff);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_MIIM_INT_SEL_MAP, uMemAccCtrl);

    /* Set automatic scan enable  */
    /* Keep off, GNATS 24612 - conflicts with phy probing */
    uMemAccCtrl = SAND_HAL_READ(userDeviceHandle, C2, PC_MIIM_CONTROL0);
    uMemAccCtrl = SAND_HAL_MOD_FIELD(C2, PC_MIIM_CONTROL0, MIIM_LINK_SCAN_EN, uMemAccCtrl, 0x0);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_MIIM_CONTROL0, uMemAccCtrl);
  }
                      
  DEXIT();
  return SB_FE2000_STS_INIT_OK_K;
}

static void
sbFe2000XtInitPb(sbhandle userDeviceHandle,
           sbFe2000XtInitParams_t *pInitParams)
{
  /* rgf - May 11 2006 - Take block out of reset */
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, PB_CORE_RESET, 0x0);
}

static void
sbFe2000XtInitPd(sbhandle userDeviceHandle,
           sbFe2000XtInitParams_t *pInitParams)
{

  sbFe2000InitParamsPd_t *pPdInitParams = &(pInitParams->pd);
  sbFe2000InitParamsPdHeaderConfig_t *pHeaderConfig = pPdInitParams->HeaderConfig;
  uint32_t uIndex=0;

  /* bring out of reset */
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, PD_CORE_RESET, 0x0);


  {

    for(uIndex =0; uIndex < SB_FE2000_PD_NUM_HEADER_CONFIGS; uIndex++) {
      sbZfC2PdHeaderConfig_t zHeaderConfig;
      zHeaderConfig.m_uBaseLength = pHeaderConfig[uIndex].uBaseLength;
      zHeaderConfig.m_uLengthPosition = pHeaderConfig[uIndex].uLengthPosition;
      zHeaderConfig.m_uLengthSize = pHeaderConfig[uIndex].uLengthSize;
      zHeaderConfig.m_uLengthUnits = pHeaderConfig[uIndex].uLengthUnits;
      if ( C2PdHeaderConfigWrite(userDeviceHandle, uIndex, &zHeaderConfig) ) {
      } else {
    SB_ERROR("%s Failed on write for HeaderConfig(%d).",__PRETTY_FUNCTION__, uIndex);
      }
    }
  }


  {
    uint32_t uPdConfig = SAND_HAL_READ(userDeviceHandle, C2,PD_CONFIG);
    uPdConfig = SAND_HAL_MOD_FIELD(C2,PD_CONFIG,TRUNC_REMOVE,uPdConfig,pInitParams->pd.uTruncationRemoveValue);
    uPdConfig = SAND_HAL_MOD_FIELD(C2,PD_CONFIG,TRUNC_LENGTH,uPdConfig,pInitParams->pd.uTruncationLengthValue);
    if(pPdInitParams->bRouteHeaderPresent) {
     uPdConfig = SAND_HAL_MOD_FIELD(C2,PD_CONFIG,FRAME_LEN_ADJUST,uPdConfig,0x1);
    }
    if(pPdInitParams->bDoContinueByteAdjust) {
     uPdConfig = SAND_HAL_MOD_FIELD(C2,PD_CONFIG,CONT_BYTE_ADJUST,uPdConfig,0x1);
    } else {
     uPdConfig = SAND_HAL_MOD_FIELD(C2,PD_CONFIG,CONT_BYTE_ADJUST,uPdConfig,0x0);
    }
    if(pPdInitParams->bDoConditionalIpv4CheckSumUpdate) {
     uPdConfig = SAND_HAL_MOD_FIELD(C2,PD_CONFIG,CONDITIONAL_UPDATE,uPdConfig,0x1);
    }
    SAND_HAL_WRITE(userDeviceHandle, C2,PD_CONFIG,uPdConfig);
  }

  {
  uint32_t uPdDebug = SAND_HAL_READ(userDeviceHandle, C2,PD_DEBUG);
  uPdDebug = SAND_HAL_MOD_FIELD(C2,PD_DEBUG,EMBEDDED_LENGTH_TYPE,uPdDebug,CA_PD_DEFAULT_EMBEDDED_LENGTH_HDRID);
  if ( pInitParams->pd.bDoIpv4CheckSumUpdate == TRUE || pInitParams->pd.bDoConditionalIpv4CheckSumUpdate == TRUE ) {
    uPdDebug = SAND_HAL_MOD_FIELD(C2,PD_DEBUG,IPV4_HDR_TYPE,uPdDebug,SB_FE2000_PP_IPV4_HDR_TYPE);
  }
  SAND_HAL_WRITE(userDeviceHandle, C2,PD_DEBUG,uPdDebug);

  /* set len pos for embedded route len header to 0x1ff */
  uPdDebug = SAND_HAL_MOD_FIELD(C2,PD_HDR_CONFIG14,LEN_POSN,uPdDebug,CA_PD_DEFAULT_LEN_POS);
  SAND_HAL_WRITE(userDeviceHandle, C2,PD_HDR_CONFIG14,uPdDebug);
  }
}


static void
sbFe2000XtInitCm(sbhandle userDeviceHandle,
           sbFe2000XtInitParams_t *pInitParams) {

  uint32_t uCmDmaThresh    = pInitParams->cm.uCmDmaThresh;
  uint32_t uTotalRequestedCounters[SB_FE2000_NUM_CMU_MMU_INTERFACES];

  sal_memset(uTotalRequestedCounters, 0x0, sizeof(uTotalRequestedCounters));

  /* gma - Oct 09 2006 - Set CMU config options */
  SAND_HAL_WRITE(userDeviceHandle, C2, CM_CONFIG_BACKPRESSURE, uCmDmaThresh );
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, CM_CORE_RESET, 0x0);

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

      uCmSegFlushStride = SAND_HAL_REG_OFFSET(C2, CM_CONFIG_SEGMENT_FLUSH_CTRL0 ) - SAND_HAL_REG_OFFSET(C2, CM_CONFIG_SEGMENT_FLUSH_CTRL1 );
      uCmSegFlushRateStride = SAND_HAL_REG_OFFSET(C2, CM_CONFIG_SEGMENT_FLUSH_RATE0 ) - SAND_HAL_REG_OFFSET(C2, CM_CONFIG_SEGMENT_FLUSH_RATE1 );

      uFlushCtrlOffset = (nCmuSegment * uCmSegFlushStride) + SAND_HAL_C2_CM_CONFIG_SEGMENT_FLUSH_CTRL0_OFFSET;
      uFlushRateOffset  =(nCmuSegment * uCmSegFlushRateStride) + SAND_HAL_C2_CM_CONFIG_SEGMENT_FLUSH_RATE0_OFFSET;

      /* gma - Dec 01 2006 - Chose 398 here because lrp table lists 403 instructions per epoch @ 20G @ 250MHz */
      SAND_HAL_WRITE_OFFS( (uint32_t)userDeviceHandle, uFlushRateOffset,
              SAND_HAL_SET_FIELD(C2, CM_CONFIG_SEGMENT_FLUSH_RATE0, RATE, pInitParams->cm.segment[nCmuSegment].uAutoFlushRate ));

      SAND_HAL_WRITE_OFFS( (uint32_t)userDeviceHandle, uFlushCtrlOffset,
              SAND_HAL_SET_FIELD(C2, CM_CONFIG_SEGMENT_FLUSH_CTRL0, ENABLE, 0x1 ));

    }
      }
    }
  }

  SAND_HAL_WRITE(userDeviceHandle, C2, CM_CONFIG_AGE_THRESHOLD, SAND_HAL_SET_FIELD(C2, CM_CONFIG_AGE_THRESHOLD, THRESHOLD, 0xff ));
  SAND_HAL_WRITE(userDeviceHandle, C2, CM_CONFIG_DMA_FORCE_COMMIT, SAND_HAL_SET_FIELD(C2, CM_CONFIG_DMA_FORCE_COMMIT, COMMIT_TIMEOUT, 0x0 ));

  DEXIT();
}
static void
sbFe2000XtInitRc(uint32_t uInstance, sbhandle userDeviceHandle,
           sbFe2000XtInitParams_t *pInitParams)
{
  DENTER();
  if (uInstance == 0) {
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, RC0_CORE_RESET, 0x0);
  } else {
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, RC1_CORE_RESET, 0x0);
  }
  DEXIT();
}

static void
sbFe2000XtInitPm(sbhandle userDeviceHandle,
           sbFe2000XtInitParams_t *pInitParams)
{

  uint32_t uGroup;
  DENTER();
  for( uGroup = 0; uGroup < SB_FE2000_PM_TOTAL_NUM_GROUPS; uGroup++ ) {
    uint32_t uBaseRegAddress = SAND_HAL_REG_OFFSET(C2,PM_GROUP0_CONFIG0)+(16*uGroup);
    SAND_HAL_WRITE_OFFS(userDeviceHandle, uBaseRegAddress,
             SAND_HAL_SET_FIELD(C2, PM_GROUP0_CONFIG0, ENABLE, pInitParams->pm.group[uGroup].bEnable ) |
             SAND_HAL_SET_FIELD(C2, PM_GROUP0_CONFIG0, POLID_MIN, pInitParams->pm.group[uGroup].uPolicerIdMinimum ));
    SAND_HAL_WRITE_OFFS(userDeviceHandle, uBaseRegAddress+4,
             SAND_HAL_SET_FIELD(C2, PM_GROUP0_CONFIG1, POLID_MAX, pInitParams->pm.group[uGroup].uPolicerIdMaximum ));
    /* rgf - Dec 01 2006 - Group 8 has only the enable and ID range, no refreshes, no timers */
    if( uGroup < 8 ) {
      SAND_HAL_WRITE_OFFS(userDeviceHandle, uBaseRegAddress+8,
               SAND_HAL_SET_FIELD(C2, PM_GROUP0_CONFIG2, REFRESH_COUNT, pInitParams->pm.group[uGroup].uRefreshCount ) |
               SAND_HAL_SET_FIELD(C2, PM_GROUP0_CONFIG2, REFRESH_THRESH, pInitParams->pm.group[uGroup].uRefreshThreshold ) |
               SAND_HAL_SET_FIELD(C2, PM_GROUP0_CONFIG2, REFRESH_PERIOD, pInitParams->pm.group[uGroup].uRefreshPeriod ));
      SAND_HAL_WRITE_OFFS(userDeviceHandle, uBaseRegAddress+12,
               SAND_HAL_SET_FIELD(C2, PM_GROUP0_CONFIG3, TIMER_ENABLE, pInitParams->pm.group[uGroup].bTimerEnable ) |
               SAND_HAL_SET_FIELD(C2, PM_GROUP0_CONFIG3, TIMESTAMP_OFFSET, pInitParams->pm.group[uGroup].uTimestampOffset ) |
               SAND_HAL_SET_FIELD(C2, PM_GROUP0_CONFIG3, TIMER_TICK_PERIOD, pInitParams->pm.group[uGroup].uTimerPeriod ));
    }

  }

  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, PM_CORE_RESET, 0x0);

  /* kpl - setup a default policer config here? Or done at application level.. */

  {
    uint32_t uPmConfig = SAND_HAL_READ(userDeviceHandle, C2, PM_CONFIG0);
    uPmConfig = SAND_HAL_FE2000_MOD_FIELD( userDeviceHandle, PM_CONFIG0, MM_SELECT_BIT, uPmConfig, pInitParams->pm.uMmuSelectBit );
    uPmConfig = SAND_HAL_FE2000_MOD_FIELD( userDeviceHandle, PM_CONFIG0, REFRESH_ENABLE, uPmConfig, pInitParams->pm.bBackgroundRefreshEnable);
    uPmConfig = SAND_HAL_FE2000_MOD_FIELD( userDeviceHandle, PM_CONFIG0, TOTAL_REFRESH_THRESH, uPmConfig, pInitParams->pm.uTotalRefreshThreshold );
    SAND_HAL_WRITE(userDeviceHandle, C2,  PM_CONFIG0, uPmConfig);
  }

  DEXIT();
}

static uint32_t
sbFe2000XtInitLr(sbhandle userDeviceHandle,
           sbFe2000XtInitParams_t *pInitParams)
{
    uint32_t uConfig0;
    uint32_t uConfig1;
    uint32_t uStreamIndex;
    uint32_t uStreamOnline = 0;
    uint32_t nRequestThreshold;
    int i, j;
    int nLrImemSize = SAND_HAL_C2_LR_IMEM_ACC_CTRL_ADDRESS_MASK + 1;
#ifdef PLISIM
    nLrImemSize = SB_FE2000XT_LR_NUMBER_OF_STREAMS * SB_FE2000XT_LR_NUMBER_OF_INSTRS;
#endif
    int32_t nClocksPerUs = 0;

    DENTER();

    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, LR_CORE_RESET,
                       0x0);

    SAND_HAL_WRITE(userDeviceHandle, C2,  LR_IMEM_ACC_DATA0, 0);
    SAND_HAL_WRITE(userDeviceHandle, C2,  LR_IMEM_ACC_DATA1, 0);
    SAND_HAL_WRITE(userDeviceHandle, C2,  LR_IMEM_ACC_DATA2, 0);
    for (i = 0; i < nLrImemSize; i++) {
        SAND_HAL_WRITE(userDeviceHandle, C2,  LR_IMEM_ACC_CTRL,
                       (SAND_HAL_C2_LR_IMEM_ACC_CTRL_REQ_MASK
                        | SAND_HAL_C2_LR_IMEM_ACC_CTRL_ACK_MASK
                        | i));
        for (j = 0; j < 100; j++) {
            if (SAND_HAL_READ(userDeviceHandle, C2, LR_IMEM_ACC_CTRL)
                & SAND_HAL_C2_LR_IMEM_ACC_CTRL_ACK_MASK) {
                break;
            }
        }
        if (j == 100) {
            return SB_FE_MEM_ACC_TIMEOUT_ERR_CODE;
        }
    }

    nRequestThreshold =(1024/
                        (pInitParams->lr.uWords0 > pInitParams->lr.uWords1
                         ? pInitParams->lr.uWords0 : pInitParams->lr.uWords1));
    
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LU_CONFIG0, RQ_THRESHOLD, nRequestThreshold);

    uConfig0 = SAND_HAL_READ(userDeviceHandle, C2, LR_CONFIG0);

    uConfig0 = SAND_HAL_FE2000_MOD_FIELD( userDeviceHandle, LR_CONFIG0, ENABLE, uConfig0, TRUE );

    /* A quote from Nick as to why the hw adds +10 to the programmed epoch!
     * "it takes time to propogate "doneness" from each PE back to the
     * instruction controller and then the instruction controller has to
     * recharge its instruction pipeline to get back to the beginning of
     * the epoch..."
     */
    uConfig0 = SAND_HAL_FE2000_MOD_FIELD( userDeviceHandle, LR_CONFIG0, EPOCH, uConfig0,
                                   (pInitParams->lr.uEpochLengthInCycles - 11) );
    uConfig0 = SAND_HAL_FE2000_MOD_FIELD( userDeviceHandle, LR_CONFIG0, CONTEXTS, uConfig0,
                                   pInitParams->lr.uNumberOfContexts );
    uConfig0 = SAND_HAL_FE2000_MOD_FIELD( userDeviceHandle, LR_CONFIG0, PAIRED, uConfig0,
                                   pInitParams->lr.bPairedMode);

    if( pInitParams->lr.bLrpBypass == TRUE ) {
        uConfig0 = SAND_HAL_FE2000_MOD_FIELD( userDeviceHandle, LR_CONFIG0, BYPASS, uConfig0, 1 );
    } else {
        uConfig0 = SAND_HAL_FE2000_MOD_FIELD( userDeviceHandle, LR_CONFIG0, BYPASS, uConfig0, 0 );
        if ( pInitParams->lr.bHeaderProcessingMode == TRUE) {
            uConfig0 = SAND_HAL_FE2000_MOD_FIELD( userDeviceHandle, LR_CONFIG0, LOAD_ENABLE,
                                           uConfig0, 1 );
        }
        for( uStreamIndex = 0; uStreamIndex < SB_FE2000XT_LR_NUMBER_OF_STREAMS;
             uStreamIndex++ ) {
            if ( pInitParams->lr.bStreamOnline[uStreamIndex] == TRUE ) {
                uStreamOnline |= (1 << uStreamIndex);
            }
        }
        uConfig0 = SAND_HAL_FE2000_MOD_FIELD( userDeviceHandle, LR_CONFIG0, ONLINE, uConfig0,
                                       uStreamOnline );
#undef UPDATE
        uConfig0 = SAND_HAL_FE2000_MOD_FIELD( userDeviceHandle, LR_CONFIG0, UPDATE, uConfig0, 1 );
    }

#ifndef __KERNEL__
    /* dgm - Apr 23 2008 - Needed for lrp timer utility (OAM) -- Updated for C2 HPP clock */
    /*
    nClocksPerUs = (int32_t) floor( (((uint64_t) 0xffffffff) * SB_FE2000XT_NSEC_PER_PIPELINE_CLOCK) / \
                             ((uint64_t) 1000)  );
    */
    nClocksPerUs = (int32_t) floor( (((uint64_t) 0xffffffff) *    (1000000.0 / pInitParams->uHppFreq)) / \
                             ((uint64_t) 1000)  );
#else
        
#endif

    SAND_HAL_WRITE( userDeviceHandle, C2, LU_CLOCKS_PER_US, nClocksPerUs);

    /* LU_RTC_NS_INC
     *    amount to increment nanosecond counter every clock
     *    tick Computed by taking floor of (hpp clock period-1)
     * 
     * LU_RTC_SUBNS_INC
     *    Fractional Amount of Nanosecond
     *    accumulated per clock tick
     *    The clock_period listed below would be an accurate measured period s
     *    ince this register accounts for fractional clock periods as well as drift 2^31 *
     *    (clock_period - rtc_ns_inc)
     */
    if        ( pInitParams->uHppFreq == SB_FE2000XT_HPP_FREQ_400000) {
        SAND_HAL_WRITE( userDeviceHandle, C2, LU_RTC_NS_INC, 1);
        SAND_HAL_WRITE( userDeviceHandle, C2, LU_RTC_SUB_NS_INC, 0xBFFFFFFF);
    } else if ( pInitParams->uHppFreq == SB_FE2000XT_HPP_FREQ_375000) {
        SAND_HAL_WRITE( userDeviceHandle, C2, LU_RTC_NS_INC, 1);
        SAND_HAL_WRITE( userDeviceHandle, C2, LU_RTC_SUB_NS_INC, 0xD5555555);
    } else if ( pInitParams->uHppFreq == SB_FE2000XT_HPP_FREQ_300000) {
        SAND_HAL_WRITE( userDeviceHandle, C2, LU_RTC_NS_INC, 2);
        SAND_HAL_WRITE( userDeviceHandle, C2, LU_RTC_SUB_NS_INC, 0xAAAAAAAA);
    } else if ( pInitParams->uHppFreq == SB_FE2000XT_HPP_FREQ_262500) {
        SAND_HAL_WRITE( userDeviceHandle, C2, LU_RTC_NS_INC, 3);
        SAND_HAL_WRITE( userDeviceHandle, C2, LU_RTC_SUB_NS_INC, 0x679E79E7);
    } else if ( pInitParams->uHppFreq == SB_FE2000XT_HPP_FREQ_258333) {
        SAND_HAL_WRITE( userDeviceHandle, C2, LU_RTC_NS_INC, 3);
        SAND_HAL_WRITE( userDeviceHandle, C2, LU_RTC_SUB_NS_INC, 0x6F7C08DD);
    } else if ( pInitParams->uHppFreq == SB_FE2000XT_HPP_FREQ_250000) {
        SAND_HAL_WRITE( userDeviceHandle, C2, LU_RTC_NS_INC, 3);
        SAND_HAL_WRITE( userDeviceHandle, C2, LU_RTC_SUB_NS_INC, 0x0);
    } else {
    SB_ASSERT(0);
    }
    
    uConfig1 = SAND_HAL_READ(userDeviceHandle, C2, LR_CONFIG1);
    uConfig1 = SAND_HAL_MOD_FIELD(C2, LR_CONFIG1, PPE_PACE, uConfig1,
                                  (pInitParams->lr.uPpeRequestPaceInCycles-1));
    uConfig1 = SAND_HAL_MOD_FIELD(C2, LR_CONFIG1, FRAMES_PER_CONTEXT,
                                  uConfig1, pInitParams->lr.uFramesPerContext);
    uConfig1 = SAND_HAL_MOD_FIELD(C2, LR_CONFIG1, LOAD_COUNT, uConfig1,
                                  pInitParams->lr.uLoadCount);
    uConfig1 = SAND_HAL_MOD_FIELD(C2, LR_CONFIG1, WORDS0, uConfig1,
                                  pInitParams->lr.uWords0 );
    uConfig1 = SAND_HAL_MOD_FIELD(C2, LR_CONFIG1, WORDS1, uConfig1,
                                  pInitParams->lr.uWords1 );
    if (pInitParams->lr.bBringUp) {
        SAND_HAL_WRITE(userDeviceHandle, C2,  LR_CONFIG1, uConfig1);
        SAND_HAL_WRITE(userDeviceHandle, C2,  LR_CONFIG0, uConfig0);
    }

    DEXIT();

    return SB_FE2000_STS_INIT_OK_K;
}


static int
_MemDiagUpdateLrpEpoch(sbhandle userDeviceHandle, uint32_t epoch)
{
  uint32_t uTimeOut;
  uint32_t uUpdateEvent;

  uint32_t uConfig0 = SAND_HAL_READ(userDeviceHandle, C2, LR_CONFIG0);
  uConfig0 = SAND_HAL_MOD_FIELD( C2, LR_CONFIG0, EPOCH, uConfig0, epoch - 11 );
  uConfig0 = SAND_HAL_MOD_FIELD( C2, LR_CONFIG0, UPDATE, uConfig0, 1 );
  SAND_HAL_WRITE(userDeviceHandle, C2, LR_CONFIG0, uConfig0);

  uUpdateEvent = 0;
  uTimeOut = 100;
  while ( !uUpdateEvent && uTimeOut ) {
    uUpdateEvent = SAND_HAL_GET_FIELD(C2, LR_EVENT, UPDATE, SAND_HAL_READ(userDeviceHandle, C2, LR_EVENT));
    --uTimeOut;
  }

  SAND_HAL_WRITE(userDeviceHandle, C2, LR_EVENT, SAND_HAL_SET_FIELD(C2, LR_EVENT, UPDATE, 0x1));

  if ( !uTimeOut ) {
    SB_ERROR("Timeout waiting for UPDATE event from the LRP.\n");
    return SB_FAILED;
  }
 
  return SB_OK;
}
static
uint32_t _MemDiagWaitForPhase(sbhandle userDeviceHandle, uint32_t phase)
{
  uint32_t uTimeout = 0;
  uint32_t uDone = 0;
  sbZfC2MmInternal0MemoryEntry_t zData;

  while ( !uDone ) {
    C2Mm0Internal0MemoryRead(userDeviceHandle,0, &zData);
    uDone = zData.m_uData0 & 0x1;

    if ( (++uTimeout) > 100000) {
      return 1;
    }
  }

  return 0;
}

static
uint32_t _MemDiagEnableProcessing(sbhandle userDeviceHandle)
{
  sbZfC2LrLrpInstructionMemoryBank0Entry_t zLrpInstructionMemoryBank0ClearEntry;
  uint32_t status;

  sbZfC2LrLrpInstructionMemoryBank0Entry_InitInstance(&zLrpInstructionMemoryBank0ClearEntry);
  /* Remove the unconditional jump at stream0-instruction0 to start processing */
  C2LrLrpInstructionMemoryBank0Write( userDeviceHandle, 0, &zLrpInstructionMemoryBank0ClearEntry);
  status = sbFe2000SwapInstructionMemoryBank(userDeviceHandle);
  if (status != SB_OK)
      return status;

  return SB_OK;
}

static uint32_t 
_MemDiagCheckForErrors(sbhandle userDeviceHandle) {
  uint32_t uError = SAND_HAL_READ(userDeviceHandle, C2, LU_ERROR);
  if (uError) {
   /*   SB_ERROR("MemDiagCheckForErrors LU ERRORS 0x%x \n", uError); */
  }
  return uError;
}
static void 
_MemDiagCleanUp(sbhandle userDeviceHandle, sbFe2000XtInitParams_t *pInitParams)
{
  soc_sbx_control_t *sbx;
  UINT uReg;

  sbx = SOC_SBX_CONTROL(UNIT_FROM_HANDLE(userDeviceHandle));
  if (!sbx) {
      SB_ASSERT(0);
  }

  pInitParams->lr.uEpochLengthInCycles = CalculateLrLoaderEpoch(pInitParams);

  /* return LR config registers to power-on values */
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, LR_CORE_RESET, 0x1);
  uReg = SAND_HAL_READ(userDeviceHandle, C2, LR_CONFIG1) ;
  SAND_HAL_WRITE(userDeviceHandle, C2, LR_CONFIG1, uReg);
  SAND_HAL_WRITE(userDeviceHandle, C2, LR_CONFIG1, 0x1CC230 );
  uReg = SAND_HAL_READ(userDeviceHandle, C2, LR_CONFIG0) ;
  SAND_HAL_WRITE(userDeviceHandle, C2, LR_CONFIG0, uReg);
  SAND_HAL_WRITE(userDeviceHandle, C2, LR_CONFIG0, 0x4000000 );

  SAND_HAL_WRITE(userDeviceHandle, C2, LU_ERROR, 0xffffffff);
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, 0, MM_ERROR,  0xffffffff );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, 1, MM_ERROR,  0xffffffff );


  SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2, MM, 0, MM_CLIENT_CONFIG,pInitParams->mm[0].uMemDiagClientConfigSave);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2, MM, 1, MM_CLIENT_CONFIG,pInitParams->mm[1].uMemDiagClientConfigSave);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2, MM, 0, MM_RAM_CONFIG, pInitParams->mm[0].uMemDiagRamConfigSave);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2, MM, 1, MM_RAM_CONFIG, pInitParams->mm[1].uMemDiagRamConfigSave);

}

static uint32_t
sbFe2000XtInitCoCounters(sbhandle userDeviceHandle,
           sbFe2000XtInitParams_t *pInitParams)
{
  int status = SB_OK;
  soc_sbx_control_t *sbx;
  fe2kAsm2IntD *ucodeBytes;
  uint32_t elen;
  uint32_t uConfig;
  sbZfC2MmInternal0MemoryEntry_t zData;

  uint32_t uConfig0;
  uint32_t uConfig1;
  uint32_t uEvent;
  uint32_t uTimeOut;
  uint32_t uMmClientConfig;
  uint32_t uMmRamConfig;
  uint32_t uInstance;
  uint32_t uIndex;
  sbZfC2LrLrpInstructionMemoryBank0Entry_t zLrpInstructionMemoryBank0ClearEntry;

    SB_LOGV3("start Clearing Coherent Counters\n");

    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, MM1_CORE_RESET, 0x0);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, LR_CORE_RESET, 0x0);

    /* save ram configuration */
    pInitParams->mm[0].uMemDiagRamConfigSave = SAND_HAL_READ_STRIDE(userDeviceHandle, C2, MM, 0, MM_RAM_CONFIG); 
    pInitParams->mm[1].uMemDiagRamConfigSave = SAND_HAL_READ_STRIDE(userDeviceHandle, C2, MM, 1, MM_RAM_CONFIG); 

    /* Load "nops" into instruction memory, blow out what was left behind */
    for ( uIndex = 0; uIndex < 1024; ++uIndex ) {
      C2LrLrpInstructionMemoryBank0Write( userDeviceHandle, uIndex, &zLrpInstructionMemoryBank0ClearEntry);
    }

    status = sbFe2000SwapInstructionMemoryBank(userDeviceHandle);
    if (status != SB_OK)
        return status;

    uConfig0 = SAND_HAL_READ(userDeviceHandle, C2, LR_CONFIG0);
    uConfig0 = SAND_HAL_MOD_FIELD( C2, LR_CONFIG0, ENABLE, uConfig0, TRUE );
    uConfig0 = SAND_HAL_MOD_FIELD( C2, LR_CONFIG0, EPOCH, uConfig0, (pInitParams->lr.uEpochLengthInCycles - 11) );
    uConfig0 = SAND_HAL_MOD_FIELD( C2, LR_CONFIG0, CONTEXTS, uConfig0, pInitParams->lr.uNumberOfContexts );
    uConfig0 = SAND_HAL_MOD_FIELD( C2, LR_CONFIG0, PAIRED, uConfig0, pInitParams->lr.bPairedMode);
    uConfig0 = SAND_HAL_MOD_FIELD( C2, LR_CONFIG0, BYPASS, uConfig0, 0 );
    uConfig0 = SAND_HAL_MOD_FIELD( C2, LR_CONFIG0, LOAD_ENABLE, uConfig0, 0 );
    uConfig0 = SAND_HAL_MOD_FIELD( C2, LR_CONFIG0, ONLINE, uConfig0, 0x01 );
    uConfig0 = SAND_HAL_MOD_FIELD( C2, LR_CONFIG0, UPDATE, uConfig0, 1 );

    uConfig1 = SAND_HAL_READ(userDeviceHandle, C2, LR_CONFIG1);
    uConfig1 = SAND_HAL_MOD_FIELD( C2, LR_CONFIG1, PPE_PACE, uConfig1,(pInitParams->lr.uPpeRequestPaceInCycles-1) );
    uConfig1 = SAND_HAL_MOD_FIELD( C2, LR_CONFIG1, FRAMES_PER_CONTEXT, uConfig1, pInitParams->lr.uFramesPerContext );
    uConfig1 = SAND_HAL_MOD_FIELD( C2, LR_CONFIG1, LOAD_COUNT, uConfig1, pInitParams->lr.uLoadCount );
    uConfig1 = SAND_HAL_MOD_FIELD( C2, LR_CONFIG1, WORDS0, uConfig1, pInitParams->lr.uWords0 );
    uConfig1 = SAND_HAL_MOD_FIELD( C2, LR_CONFIG1, WORDS1, uConfig1, pInitParams->lr.uWords1 );
    SAND_HAL_WRITE(userDeviceHandle, C2, LR_CONFIG1, uConfig1);
    SAND_HAL_WRITE(userDeviceHandle, C2, LR_CONFIG0, uConfig0);

    uEvent = 0;
    uTimeOut = 0;
    while ( !uEvent && (1500 > uTimeOut) ) {
      uEvent=SAND_HAL_GET_FIELD(C2, LR_EVENT, ONLINE0, SAND_HAL_READ(userDeviceHandle, C2, LR_EVENT));
      uTimeOut++;
    }
    if ( uTimeOut == 1500 ) {
      SB_ERROR("Timeout waiting for ONLINE0 event from LRP.\n");
    }

    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LR_GROUP0_GPR0_DEBUG, DISABLE_PARITY_ERROR, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LR_GROUP0_GPR1_DEBUG, DISABLE_PARITY_ERROR, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LR_GROUP1_GPR0_DEBUG, DISABLE_PARITY_ERROR, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LR_GROUP1_GPR1_DEBUG, DISABLE_PARITY_ERROR, 0x1);

    for ( uInstance = 0; uInstance < SB_FE2000_NUM_MM_INSTANCES; ++uInstance ) {
      uMmClientConfig = 0;
      uMmClientConfig = SAND_HAL_MOD_FIELD(C2,MM_CLIENT_CONFIG, LRP0_MEMORY_CONFIG, uMmClientConfig, 1);
      uMmClientConfig = SAND_HAL_MOD_FIELD(C2,MM_CLIENT_CONFIG, LRP1_MEMORY_CONFIG, uMmClientConfig, 1);
      uMmClientConfig = SAND_HAL_MOD_FIELD(C2,MM_CLIENT_CONFIG, LRP2_MEMORY_CONFIG, uMmClientConfig, 1);
      uMmClientConfig = SAND_HAL_MOD_FIELD(C2,MM_CLIENT_CONFIG, LRP3_MEMORY_CONFIG, uMmClientConfig, 1);
      uMmClientConfig = SAND_HAL_MOD_FIELD(C2,MM_CLIENT_CONFIG, LRP4_MEMORY_CONFIG, uMmClientConfig, 1);
      SAND_HAL_WRITE_STRIDE(userDeviceHandle,C2, MM, uInstance, MM_CLIENT_CONFIG,uMmClientConfig);

      uMmRamConfig = SAND_HAL_READ_STRIDE(userDeviceHandle, C2, MM, uInstance, MM_RAM_CONFIG);; 
      uMmRamConfig = SAND_HAL_MOD_FIELD( C2, MM_RAM_CONFIG, INTERNAL_RAM_CONFIG, uMmRamConfig, SB_FE2000XT_MM_RAM0_16KBY36_AND_RAM1_16KBY36 );
      uMmRamConfig = SAND_HAL_MOD_FIELD( C2, MM_RAM_CONFIG, INT_RAM0_DMA_GT32, uMmRamConfig, SB_FE2000XT_ONE_32_BIT_DAM_XFERS );
      uMmRamConfig = SAND_HAL_MOD_FIELD( C2, MM_RAM_CONFIG, INT_RAM1_DMA_GT32, uMmRamConfig, SB_FE2000XT_ONE_32_BIT_DAM_XFERS );
      SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2, MM, uInstance, MM_RAM_CONFIG, uMmRamConfig);
    }

  SAND_HAL_WRITE(userDeviceHandle, C2, LU_ERROR, 0xffffffff);
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, 0, MM_ERROR,  0xffffffff );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, 1, MM_ERROR,  0xffffffff );


    /* configure the coherent counters to clear when written with 0xffffffff */
  uConfig = SAND_HAL_READ(userDeviceHandle, C2, LU_CONFIG1);
  uConfig = SAND_HAL_MOD_FIELD(C2, LU_CONFIG1, OAM_COW, uConfig, 1 );
  SAND_HAL_WRITE(userDeviceHandle, C2, LU_CONFIG1, uConfig);

  sbx = SOC_SBX_CONTROL(UNIT_FROM_HANDLE(userDeviceHandle));
  if (!sbx) {
    SB_ERROR("no handle\n");
    return SOC_E_INIT;
  }

  /* load the ucode program that will clear the Coherent Counters */
  if (soc_sbx_fe2000_ucode_load_from_buffer((int)userDeviceHandle, soc_sbx_fe2kxt_clearcc_fe2kAsm2UcodeEc, sizeof (soc_sbx_fe2kxt_clearcc_fe2kAsm2UcodeEc)) != 0) {
    SB_ERROR("Couldnt load clearCc ucode.\n");
    return SB_FAILED;
  }

  /* set epoch length */
  ucodeBytes = (fe2kAsm2IntD *)sbx->ucode;
  if(fe2kAsm2IntD__getNc(ucodeBytes, "elen", &elen) > 0){
    SB_ERROR("Couldnt read epoch loength from clearCc ucode.\n");
    SB_ASSERT(0);
  }

  pInitParams->lr.uEpochLengthInCycles = elen;

  status = sbFe2000UcodeLoad(userDeviceHandle, sbx->ucode);
  if (status != SB_OK) {
    SB_ERROR("Couldnt write clearCc ucode.\n");
    return status;
  }

  if (_MemDiagUpdateLrpEpoch(userDeviceHandle, pInitParams->lr.uEpochLengthInCycles) != 0) {
    SB_ERROR("Couldnt upate lrp epoch for clearCc ucode.\n");
    return SB_FAILED;
  }

  /* clear the done bit */
  zData.m_uData0 = 0;
  C2Mm0Internal0MemoryWrite(userDeviceHandle, 0, &zData);

  /* start the ucode program */
  if ( _MemDiagEnableProcessing(userDeviceHandle) != SB_OK) {
    SB_ERROR("Couldnt start clearCc ucode.\n");
    return SB_FAILED;
  }

  /* check for done */
  if ( _MemDiagWaitForPhase(userDeviceHandle, 0) ) {
    SB_ERROR("Clear Coherent Counters timeout,  FAIL\n");
  } else if ( _MemDiagCheckForErrors(userDeviceHandle) ) {
    SB_LOGV3("Clear Coherent Counters  ERRORS!\n");
    status = SB_FAILED;
  } else {
    SB_LOGV3("Cleared Coherent Counters\n");
  }

  _MemDiagCleanUp(userDeviceHandle, pInitParams);

  return status;
}

static uint32_t
sbFe2000XtInitMm(uint32_t uInstance, sbhandle userDeviceHandle,
           sbFe2000XtInitParams_t *pInitParams)
{

  sbFe2000XtInitParamsMm_t *pInstance = &(pInitParams->mm[uInstance]);
  sbFe2000XtInitParamsMmDdrConfig_t *pDdrConfig = &(pInitParams->ddrconfig[uInstance]);
  uint32_t uNumInternalMem0;
  uint32_t uNumInternalMem1;
  uint32_t uMmClientConfig;
  uint32_t uMmProtectionScheme0;
  uint32_t uMmRamConfiguration;
  uint32_t uValidDelay, uTurnAroundDelay, uPhyConfig1, uPhyConfig2, uPhyConfig3, uAddressShift;
  uint32_t uMmError;
  uint8_t bPvtFailed;
  uint32_t uPvtCompAttempt;
  char *sPropName = NULL, *sTuningParams = NULL;
  DENTER();

  if( uInstance ){
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, MM1_CORE_RESET, 0x0);
  } else {
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, MM0_CORE_RESET, 0x0);
  }

  bPvtFailed = 1;
  uPvtCompAttempt = 0;
  while ( (uPvtCompAttempt < FE2K_MAX_PVT_COMPENSATION_ATTEMPTS) && (bPvtFailed == 1) ) {
    uPvtCompAttempt++;
    SB_LOGV1("MM%d PVT compensation attempt: %d\n",
	     uInstance,uPvtCompAttempt);
    bPvtFailed = sbFe2000XtUtilPvtCompensation(userDeviceHandle,uInstance);
  }

  if (bPvtFailed) {
    SB_ERROR("MM%d PVT compensation: All %d calls to PvtCompensation ended in failure\n",
	     uInstance,uPvtCompAttempt);
  } else {
    SB_LOGV1("MM%d PVT compensation succeeded after %d calls to PvtCompensation\n",
	     uInstance,uPvtCompAttempt);
  }


  /* step1: configure the memory configuration */
  uMmRamConfiguration = SAND_HAL_READ_STRIDE(userDeviceHandle, C2,MM,uInstance,MM_RAM_CONFIG);
  uMmRamConfiguration = SAND_HAL_MOD_FIELD(C2,MM_RAM_CONFIG,INTERNAL_RAM_CONFIG,uMmRamConfiguration,pInstance->ramconfiguration.InternalRamConfiguration);
  uMmRamConfiguration = SAND_HAL_MOD_FIELD( C2, MM_RAM_CONFIG, INT_RAM0_DMA_GT32, uMmRamConfiguration, pInstance->ramconfiguration.IntRam0DmaControl );
  uMmRamConfiguration = SAND_HAL_MOD_FIELD( C2, MM_RAM_CONFIG, INT_RAM1_DMA_GT32, uMmRamConfiguration, pInstance->ramconfiguration.IntRam1DmaControl );

    uValidDelay = SB_FE2000XT_MIN_VALID_DELAY;   /* Start with initial valid_delay value of 4.  Will be experimentally derived below.*/
    uPhyConfig1 = 0x0088808d;  
  /*
   * [30] = 1'b0; // XMIT VCDL Phase Offset Control, bit 4
   * [23:20] = 4'hC; // XMIT VCDL Phase Offset Control, bits 3:0
   * [29] = 1'b0; // QKB VCDL Phase Offset Control, bit 4
   * [19:16] = 4'hC; // QKB VCDL Phase Offset Control, bits 3:0
   * [28] = 1'b0; // QK VCDL Phase Offset Control, bit 4
   * [15:12] = 4'hC; // QK VCDL Phase Offset Control, bits 3:0
   *
   * [7] = 1'b1; // Select "average" algorithm;
   * [5] = 1'b1; // Half-Cycle Latency, = 1 for 2.5-cycle DDR2+ SRAMs
   * [3] = 1'b1; // DQ ODT Enable
   * [2] = 1'b1; // QK ODT Enable
   * [0] = 1'b1; // Enable DDR PHY
   */
    /* uPhyConfig1 = 0x00000001; */
    uPhyConfig2 = 0x00f00001;  
    /*
     * Initialize DDR PHY configuration register 2. Select "early"
     * registers, to reduce total latency by two cycles
     * [23:20] = 4'hF; // Sel_early
     * [0] = 1'b1; // ODT RT75 Ohm resistor
     */

    uPhyConfig3 = 0x00000001;  
    /*
     * Initialize DDR PHY configuration register 3.
     * [23:20] = 4'h0; // Bias Generator DAC Control Override Value
     * [19] = 1'h0; // Bias Generator DAC Control Override Enable
     * [18] = 1'h0; // Bias Generator DAC Bypass
     * [17] = 1'h0; // Bias Generator DAC Reset
     * [16] = 1'h0; // TX VCDL Bypass
     * [14] = 1'h0; // QK_N VCDL Bypass
     * [13] = 1'h0; // DLL DESKEW Bypass
     * [12] = 1'h0; // QK VCDL Bypass
     * [5:4] = 1'h0; // Override Resample Clock Select Value
     * [0] = 1'h0; // Override Resample Clock Select Enable 
     */

    if ( pInitParams->uDdr2MemorySelect == SB_FE2000XT_DDR2 ) {
      uTurnAroundDelay = 1;
      uPhyConfig1 |= 0x20;
      uAddressShift = 1;
    } else if ( pInitParams->uDdr2MemorySelect == SB_FE2000XT_DDR2_PLUS_20 ) {
      uTurnAroundDelay = 2;
      uAddressShift = 1; /*gZM.GetAttribute("embedded_transport",0);*/
    } else if ( pInitParams->uDdr2MemorySelect == SB_FE2000XT_DDR2_PLUS_25 ){
      uTurnAroundDelay = 2;
      uPhyConfig1 |= 0x20;
      uAddressShift = 1; /*gZM.GetAttribute("embedded_transport",0); */
    } else {
    /* to please compiler */
    uValidDelay = uTurnAroundDelay = uPhyConfig1 = uAddressShift = 0xffffffff;
    SB_LOG("Invalid memory configuration\n");
    SB_ASSERT(0);
  }

  uMmRamConfiguration = SAND_HAL_MOD_FIELD(C2,MM_RAM_CONFIG,NARROW_PORT0_VALID_DELAY,uMmRamConfiguration,uValidDelay);
  uMmRamConfiguration = SAND_HAL_MOD_FIELD(C2,MM_RAM_CONFIG,NARROW_PORT1_VALID_DELAY,uMmRamConfiguration,uValidDelay );
  uMmRamConfiguration = SAND_HAL_MOD_FIELD(C2,MM_RAM_CONFIG,WIDE_PORT_VALID_DELAY,uMmRamConfiguration,uValidDelay );
  uMmRamConfiguration = SAND_HAL_MOD_FIELD(C2,MM_RAM_CONFIG,NARROW_PORT0_TURNAROUND_DELAY,uMmRamConfiguration,uTurnAroundDelay);
  uMmRamConfiguration = SAND_HAL_MOD_FIELD(C2,MM_RAM_CONFIG,NARROW_PORT1_TURNAROUND_DELAY,uMmRamConfiguration,uTurnAroundDelay);
  uMmRamConfiguration = SAND_HAL_MOD_FIELD(C2,MM_RAM_CONFIG,WIDE_PORT_TURNAROUND_DELAY,uMmRamConfiguration,uTurnAroundDelay);
  uMmRamConfiguration = SAND_HAL_MOD_FIELD(C2,MM_RAM_CONFIG,NARROW_PORT0_ADDRESS_SHIFT,uMmRamConfiguration,uAddressShift);
  uMmRamConfiguration = SAND_HAL_MOD_FIELD(C2,MM_RAM_CONFIG,NARROW_PORT1_ADDRESS_SHIFT,uMmRamConfiguration,uAddressShift);
  uMmRamConfiguration = SAND_HAL_MOD_FIELD(C2,MM_RAM_CONFIG,WIDE_PORT_ADDRESS_SHIFT,uMmRamConfiguration,uAddressShift);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle,C2,MM,uInstance, MM_RAM_CONFIG,uMmRamConfiguration);

  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_IOW_CONFIG_REG1,  uPhyConfig1 );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION0_CONFIG_REG1, uPhyConfig1 );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION1_CONFIG_REG1, uPhyConfig1 );

  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_IOW_CONFIG_REG2,  uPhyConfig2 );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION0_CONFIG_REG2, uPhyConfig2 );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION1_CONFIG_REG2, uPhyConfig2 );

  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_IOW_CONFIG_REG3,  uPhyConfig3 );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION0_CONFIG_REG3, uPhyConfig3 );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION1_CONFIG_REG3, uPhyConfig3 );

  {
  /* ssm - Nov 20 2008 - Incorporate DDR interface initialization from Triumph
   * ssm - Nov 20 2008 - (system_sim/sdk/src/soc/esw/triumph.c).  Their init routine
   * ssm - Nov 20 2008 - stops and restarts K clock outputs, then turns on IDDQ and
   * ssm - Nov 20 2008 - disables interface, then turns off IDDQ and enables interface,
   * ssm - Nov 20 2008 - then asserts and clears BIASGEN DLL reset.
   */
  UINT uStopKClock   = 0x80000000;  /* Bit 31 on */
  UINT uAssertIddq   = 0x00000002;  /* Bit 01 on */
  UINT uDisablePhy   = 0xFFFFFFFE;  /* Bit 00 off */
  UINT uBiasGenReset = 0x00020000;  /* Bit 17 on */

  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_IOW_CONFIG_REG1,  uPhyConfig1 | uStopKClock );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION0_CONFIG_REG1, uPhyConfig1 | uStopKClock );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION1_CONFIG_REG1, uPhyConfig1 | uStopKClock );
  thin_delay(20);

  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_IOW_CONFIG_REG1,  uPhyConfig1 );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION0_CONFIG_REG1, uPhyConfig1 );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION1_CONFIG_REG1, uPhyConfig1 );

  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_IOW_CONFIG_REG1,  uPhyConfig1 | uAssertIddq );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION0_CONFIG_REG1, uPhyConfig1 | uAssertIddq );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION1_CONFIG_REG1, uPhyConfig1 | uAssertIddq );
  thin_delay(20);

  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_IOW_CONFIG_REG1,  (uPhyConfig1 | uAssertIddq) & uDisablePhy );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION0_CONFIG_REG1, (uPhyConfig1 | uAssertIddq) & uDisablePhy );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION1_CONFIG_REG1, (uPhyConfig1 | uAssertIddq) & uDisablePhy );
  thin_delay(2000);

  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_IOW_CONFIG_REG1,  uPhyConfig1 & uDisablePhy );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION0_CONFIG_REG1, uPhyConfig1 & uDisablePhy );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION1_CONFIG_REG1, uPhyConfig1 & uDisablePhy );
  thin_delay(20);

  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_IOW_CONFIG_REG1,  uPhyConfig1 );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION0_CONFIG_REG1, uPhyConfig1 );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION1_CONFIG_REG1, uPhyConfig1 );

  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_IOW_CONFIG_REG3,  uPhyConfig3 | uBiasGenReset );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION0_CONFIG_REG3, uPhyConfig3 | uBiasGenReset );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION1_CONFIG_REG3, uPhyConfig3 | uBiasGenReset );
  thin_delay(200);

  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_IOW_CONFIG_REG3,  uPhyConfig3 );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION0_CONFIG_REG3, uPhyConfig3 );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION1_CONFIG_REG3, uPhyConfig3 );

  /* ssm - Nov 20 2008 - End of Triumph initialization code */
  }
  /* gma - Nov 03 2006 - Internal memories can only be allocated to one block.  Check for that here.*/
  uNumInternalMem0 = ( pInstance->ramconnections.Lrp2MemoryConnection == SB_FE2000XT_MM_LRP2_CONNECTED_TO_INTERNAL_SINGLE_RAM0 ) ? 1 : 0;
  uNumInternalMem0 += ( pInstance->ramconnections.Cmu0MemoryConnection == SB_FE2000XT_MM_CMU0_CONNECTED_TO_INTERNAL_SINGLE_RAM0 ) ? 1 : 0;
  uNumInternalMem0 += ( pInstance->ramconnections.PmuMemoryConnection  == SB_FE2000XT_MM_PMU_CONNECTED_TO_INTERNAL_SINGLE_RAM0  ) ? 1 : 0;

  uNumInternalMem1 = ( pInstance->ramconnections.Lrp3MemoryConnection == SB_FE2000XT_MM_LRP3_CONNECTED_TO_INTERNAL_SINGLE_RAM1 ) ? 1 : 0;
  uNumInternalMem1 += ( pInstance->ramconnections.Cmu1MemoryConnection == SB_FE2000XT_MM_CMU1_CONNECTED_TO_INTERNAL_SINGLE_RAM1 ) ? 1 : 0;
  uNumInternalMem1 += ( pInstance->ramconnections.PmuMemoryConnection  == SB_FE2000XT_MM_PMU_CONNECTED_TO_INTERNAL_SINGLE_RAM1  ) ? 1 : 0;

  if( uNumInternalMem0 > 1 || uNumInternalMem1 > 1 ){
    SB_LOG("Conflict on the allocation of Mmu internal memories between"
           " LRP/CMU/PMU blocks.\n");
    SB_ASSERT( 0 );
  }

  /* step 2: configure the client memory connections */
  uMmClientConfig = SAND_HAL_READ_STRIDE(userDeviceHandle, C2,MM,uInstance,MM_CLIENT_CONFIG);

  uMmClientConfig = SAND_HAL_MOD_FIELD(C2,MM_CLIENT_CONFIG, PMU_PRIMARY_CLIENT, uMmClientConfig,pInstance->ramconnections.bPmuPrimaryClientOnWideport);
  uMmClientConfig = SAND_HAL_MOD_FIELD(C2,MM_CLIENT_CONFIG, LRP0_MEMORY_CONFIG, uMmClientConfig,pInstance->ramconnections.Lrp0MemoryConnection);
  uMmClientConfig = SAND_HAL_MOD_FIELD(C2,MM_CLIENT_CONFIG, LRP1_MEMORY_CONFIG, uMmClientConfig,pInstance->ramconnections.Lrp1MemoryConnection);
  uMmClientConfig = SAND_HAL_MOD_FIELD(C2,MM_CLIENT_CONFIG, LRP2_MEMORY_CONFIG, uMmClientConfig,pInstance->ramconnections.Lrp2MemoryConnection);
  uMmClientConfig = SAND_HAL_MOD_FIELD(C2,MM_CLIENT_CONFIG, LRP3_MEMORY_CONFIG, uMmClientConfig,pInstance->ramconnections.Lrp3MemoryConnection);
  uMmClientConfig = SAND_HAL_MOD_FIELD(C2,MM_CLIENT_CONFIG, LRP4_MEMORY_CONFIG, uMmClientConfig,pInstance->ramconnections.Lrp4MemoryConnection);
  uMmClientConfig = SAND_HAL_MOD_FIELD(C2,MM_CLIENT_CONFIG,  PMU_MEMORY_CONFIG, uMmClientConfig,pInstance->ramconnections.PmuMemoryConnection);
  uMmClientConfig = SAND_HAL_MOD_FIELD(C2,MM_CLIENT_CONFIG, CMU0_MEMORY_CONFIG, uMmClientConfig,pInstance->ramconnections.Cmu0MemoryConnection);
  uMmClientConfig = SAND_HAL_MOD_FIELD(C2,MM_CLIENT_CONFIG, CMU1_MEMORY_CONFIG, uMmClientConfig,pInstance->ramconnections.Cmu1MemoryConnection);
  /* jts - Oct 16 2006 - removed reference to LUE client setup (these were removed from .trex2) */

  SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2,MM,uInstance, MM_CLIENT_CONFIG,uMmClientConfig);
  pInitParams->mm[uInstance].uMemDiagClientConfigSave = uMmClientConfig;

  /* csk Feb 12 2008 - Reduce the AF Threshold by 1 for PMU since latencies have increased.*/
  SAND_HAL_RMW_FIELD_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_AFULL_THRESHOLD0, PM_RD_AF_THRESH, 0xC );
  SAND_HAL_RMW_FIELD_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_AFULL_THRESHOLD0, PM_WR_AF_THRESH, 0xC );

  /* If present, use tuning parameters for VCDL */
  sPropName = uInstance ? "bcm88025_mmu1_narrow0_config" : "bcm88025_mmu0_narrow0_config";
  sTuningParams = soc_property_get_str(UNIT_FROM_HANDLE(userDeviceHandle), sPropName);
  /* soc_properties override EEPROM */
  if (sTuningParams) {
      /* if any parameters are present, then all must be present */
      _GET_NEXT_VALUE(sTuningParams, pDdrConfig->uValidDelay[0]);
      _GET_NEXT_VALUE(NULL, pDdrConfig->uPhaseSelect[0]);
      _GET_NEXT_VALUE(NULL, pDdrConfig->uQkMidRange[0]);
      _GET_NEXT_VALUE(NULL, pDdrConfig->uQknMidRange[0]);
      _GET_NEXT_VALUE(NULL, pDdrConfig->uXMidRange[0]);

      sPropName = uInstance ? "bcm88025_mmu1_narrow1_config" : "bcm88025_mmu0_narrow1_config";
      sTuningParams = soc_property_get_str(UNIT_FROM_HANDLE(userDeviceHandle), sPropName);
      if (sTuningParams == NULL){
          SB_ERROR("Tuning parameter: %s missing\n", sPropName);
          SB_ASSERT(0);
          return SOC_E_PARAM;
      }
      _GET_NEXT_VALUE(sTuningParams, pDdrConfig->uValidDelay[1]);
      _GET_NEXT_VALUE(NULL, pDdrConfig->uPhaseSelect[1]);
      _GET_NEXT_VALUE(NULL, pDdrConfig->uQkMidRange[1]);
      _GET_NEXT_VALUE(NULL, pDdrConfig->uQknMidRange[1]);
      _GET_NEXT_VALUE(NULL, pDdrConfig->uXMidRange[1]);

      sPropName = uInstance ? "bcm88025_mmu1_wide_config" : "bcm88025_mmu0_wide_config";
      sTuningParams = soc_property_get_str(UNIT_FROM_HANDLE(userDeviceHandle), sPropName);
      if (sTuningParams == NULL){
          SB_ERROR("Tuning parameter: %s missing\n", sPropName);
          SB_ASSERT(0);
          return SOC_E_PARAM;
      }

      _GET_NEXT_VALUE(sTuningParams, pDdrConfig->uValidDelay[2]);
      _GET_NEXT_VALUE(NULL, pDdrConfig->uPhaseSelect[2]);
      _GET_NEXT_VALUE(NULL, pDdrConfig->uQkMidRange[2]);
      _GET_NEXT_VALUE(NULL, pDdrConfig->uQknMidRange[2]);
      _GET_NEXT_VALUE(NULL, pDdrConfig->uXMidRange[2]);

      pDdrConfig->bValid = 1;
    
  }
  
  if (pDdrConfig->bValid){
      /* narrow 0 */
      uMmRamConfiguration = SAND_HAL_MOD_FIELD(C2,MM_RAM_CONFIG,NARROW_PORT0_VALID_DELAY,uMmRamConfiguration,pDdrConfig->uValidDelay[0]);
      SAND_HAL_WRITE_STRIDE(userDeviceHandle,C2,MM,uInstance, MM_RAM_CONFIG,uMmRamConfiguration);
      uPhyConfig3 = SAND_HAL_READ_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_MM_ION0_CONFIG_REG3); 
      uPhyConfig3 &= 0xffffffcf;
      uPhyConfig3 |= pDdrConfig->uPhaseSelect[0] << 4;
      SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION0_CONFIG_REG3, uPhyConfig3 );

      SB_LOGV1("MMInstance%d-NarrowPort0 VALID_DELAY=%d, PHASE_SELECT=%d,\n", uInstance, pDdrConfig->uValidDelay[0], pDdrConfig->uPhaseSelect[0] );

      uPhyConfig1 = SAND_HAL_READ_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_MM_ION0_CONFIG_REG1);
      uPhyConfig1 = SAND_HAL_MOD_FIELD(C2, MM_MM_ION0_CONFIG_REG1, QK_VCDL_PHASE_MS, uPhyConfig1, (pDdrConfig->uQkMidRange[0] >> 4));
      uPhyConfig1 = SAND_HAL_MOD_FIELD(C2, MM_MM_ION0_CONFIG_REG1, QK_VCDL_PHASE_LS, uPhyConfig1, (pDdrConfig->uQkMidRange[0] & 0xf));

      uPhyConfig1 = SAND_HAL_MOD_FIELD(C2, MM_MM_ION0_CONFIG_REG1, QK_N_VCDL_PHASE_MS, uPhyConfig1, (pDdrConfig->uQknMidRange[0] >> 4));
      uPhyConfig1 = SAND_HAL_MOD_FIELD(C2, MM_MM_ION0_CONFIG_REG1, QK_N_VCDL_PHASE_LS, uPhyConfig1, (pDdrConfig->uQknMidRange[0] & 0xf));

      uPhyConfig1 = SAND_HAL_MOD_FIELD(C2, MM_MM_ION0_CONFIG_REG1, X_VCDL_PHASE_MS, uPhyConfig1, (pDdrConfig->uXMidRange[0] >> 4));
      uPhyConfig1 = SAND_HAL_MOD_FIELD(C2, MM_MM_ION0_CONFIG_REG1, X_VCDL_PHASE_LS, uPhyConfig1, (pDdrConfig->uXMidRange[0] & 0xf));

      SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION0_CONFIG_REG1, uPhyConfig1 );

      /* narrow 1 */
      uMmRamConfiguration = SAND_HAL_MOD_FIELD(C2,MM_RAM_CONFIG,NARROW_PORT1_VALID_DELAY,uMmRamConfiguration, pDdrConfig->uValidDelay[1]);
      SAND_HAL_WRITE_STRIDE(userDeviceHandle,C2,MM,uInstance, MM_RAM_CONFIG,uMmRamConfiguration);

      uPhyConfig3 = SAND_HAL_READ_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_MM_ION1_CONFIG_REG3); 
      uPhyConfig3 &= 0xffffffcf;
      uPhyConfig3 |= pDdrConfig->uPhaseSelect[1] << 4;
      SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION1_CONFIG_REG3, uPhyConfig3 );

      SB_LOGV1("MMInstance%d-NarrowPort1 VALID_DELAY=%d, PHASE_SELECT=%d,\n", uInstance, pDdrConfig->uValidDelay[1], pDdrConfig->uPhaseSelect[1] );

      uPhyConfig1 = SAND_HAL_READ_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_MM_ION1_CONFIG_REG1);
      uPhyConfig1 = SAND_HAL_MOD_FIELD(C2, MM_MM_ION1_CONFIG_REG1, QK_VCDL_PHASE_MS, uPhyConfig1, (pDdrConfig->uQkMidRange[1] >> 4));
      uPhyConfig1 = SAND_HAL_MOD_FIELD(C2, MM_MM_ION1_CONFIG_REG1, QK_VCDL_PHASE_LS, uPhyConfig1, (pDdrConfig->uQkMidRange[1] & 0xf));

      uPhyConfig1 = SAND_HAL_MOD_FIELD(C2, MM_MM_ION1_CONFIG_REG1, QK_N_VCDL_PHASE_MS, uPhyConfig1, (pDdrConfig->uQknMidRange[1] >> 4));
      uPhyConfig1 = SAND_HAL_MOD_FIELD(C2, MM_MM_ION1_CONFIG_REG1, QK_N_VCDL_PHASE_LS, uPhyConfig1, (pDdrConfig->uQknMidRange[1] & 0xf));

      uPhyConfig1 = SAND_HAL_MOD_FIELD(C2, MM_MM_ION1_CONFIG_REG1, X_VCDL_PHASE_MS, uPhyConfig1, (pDdrConfig->uXMidRange[1] >> 4));
      uPhyConfig1 = SAND_HAL_MOD_FIELD(C2, MM_MM_ION1_CONFIG_REG1, X_VCDL_PHASE_LS, uPhyConfig1, (pDdrConfig->uXMidRange[1] & 0xf));
      SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_ION1_CONFIG_REG1, uPhyConfig1 );

      uMmRamConfiguration = SAND_HAL_MOD_FIELD(C2,MM_RAM_CONFIG,WIDE_PORT_VALID_DELAY,uMmRamConfiguration,pDdrConfig->uValidDelay[2]);
      SAND_HAL_WRITE_STRIDE(userDeviceHandle,C2,MM,uInstance, MM_RAM_CONFIG,uMmRamConfiguration);

      uPhyConfig3 = SAND_HAL_READ_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_MM_IOW_CONFIG_REG3); 
      uPhyConfig3 &= 0xffffffcf;
      uPhyConfig3 |= pDdrConfig->uPhaseSelect[2] << 4;
      SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_IOW_CONFIG_REG3, uPhyConfig3 );
      SB_LOGV1("MMInstance%d-Wide VALID_DELAY=%d, PHASE_SELECT=%d,\n", uInstance, pDdrConfig->uValidDelay[2], pDdrConfig->uPhaseSelect[2] );

      uPhyConfig1 = SAND_HAL_READ_STRIDE(userDeviceHandle,C2,MM,uInstance,MM_MM_IOW_CONFIG_REG1);
      uPhyConfig1 = SAND_HAL_MOD_FIELD(C2, MM_MM_IOW_CONFIG_REG1, QK_VCDL_PHASE_MS, uPhyConfig1, (pDdrConfig->uQkMidRange[2] >> 4));
      uPhyConfig1 = SAND_HAL_MOD_FIELD(C2, MM_MM_IOW_CONFIG_REG1, QK_VCDL_PHASE_LS, uPhyConfig1, (pDdrConfig->uQkMidRange[2] & 0xf));

      uPhyConfig1 = SAND_HAL_MOD_FIELD(C2, MM_MM_IOW_CONFIG_REG1, QK_N_VCDL_PHASE_MS, uPhyConfig1, (pDdrConfig->uQknMidRange[2] >> 4));
      uPhyConfig1 = SAND_HAL_MOD_FIELD(C2, MM_MM_IOW_CONFIG_REG1, QK_N_VCDL_PHASE_LS, uPhyConfig1, (pDdrConfig->uQknMidRange[2] & 0xf));

      uPhyConfig1 = SAND_HAL_MOD_FIELD(C2, MM_MM_IOW_CONFIG_REG1, X_VCDL_PHASE_MS, uPhyConfig1, (pDdrConfig->uXMidRange[2] >> 4));
      uPhyConfig1 = SAND_HAL_MOD_FIELD(C2, MM_MM_IOW_CONFIG_REG1, X_VCDL_PHASE_LS, uPhyConfig1, (pDdrConfig->uXMidRange[2] & 0xf));

      SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_MM_IOW_CONFIG_REG1, uPhyConfig1 );

  }else{ /* perform tuning */
      SB_ERROR("%s:DDR Tuning Parameters not found for MMU 0x%x.  Run exttsbx to perform DDR tuning\n", __PRETTY_FUNCTION__, uInstance );
      /* perform coarse tuning to allow soc_init to complete */
  }

  /* ssm - Jun 20 2008 - Wide Port "raw" mode reads (BYPASS_ECC bit in MM_WIDE_PORT_MEM_ACC_CTRL */
  /* ssm - Jun 20 2008 - set) still calculate ECC and set correctable/uncorrectable error bits */
  /* ssm - Jun 20 2008 - in MM_ERROR.  Must clear interrupts after training completes. */
  uMmError = 0x0;
  uMmError = SAND_HAL_MOD_FIELD( C2, MM_ERROR, WIDE_PORT_COR_ERROR, uMmError, 1 );
  uMmError = SAND_HAL_MOD_FIELD( C2, MM_ERROR, WIDE_PORT_UNCOR_ERROR, uMmError, 1 );
  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_ERROR, uMmError );

  /* default value (7) produces poor waveforms on DDR data bus, increase to 15 */
  SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle, C2, MM, uInstance, MM_PVT_CONTROL2, PVT_SCOMP_CONTROL, 0xf);

  /* gma - Nov 03 2006 - step 3: configure the ram protection scheme0*/
  uMmProtectionScheme0 = SAND_HAL_READ_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_PROT_SCHEME0 );

  uMmProtectionScheme0 = SAND_HAL_MOD_FIELD( C2, MM_PROT_SCHEME0, NP0_PRI_SCHEME, uMmProtectionScheme0, pInstance->ramprotection.NarrowPort0PrimaryClient );
  uMmProtectionScheme0 = SAND_HAL_MOD_FIELD( C2, MM_PROT_SCHEME0, NP0_SEC_SCHEME, uMmProtectionScheme0, pInstance->ramprotection.NarrowPort0SecondaryClient );
  uMmProtectionScheme0 = SAND_HAL_MOD_FIELD( C2, MM_PROT_SCHEME0, NP0_DMA_SCHEME, uMmProtectionScheme0, pInstance->ramprotection.NarrowPort0DmaAccess );
  uMmProtectionScheme0 = SAND_HAL_MOD_FIELD( C2, MM_PROT_SCHEME0, NP0_PI_SCHEME,  uMmProtectionScheme0, pInstance->ramprotection.NarrowPort0ProcessorInterface );

  uMmProtectionScheme0 = SAND_HAL_MOD_FIELD( C2, MM_PROT_SCHEME0, NP1_PRI_SCHEME, uMmProtectionScheme0, pInstance->ramprotection.NarrowPort1PrimaryClient );
  uMmProtectionScheme0 = SAND_HAL_MOD_FIELD( C2, MM_PROT_SCHEME0, NP1_SEC_SCHEME, uMmProtectionScheme0, pInstance->ramprotection.NarrowPort1SecondaryClient );
  uMmProtectionScheme0 = SAND_HAL_MOD_FIELD( C2, MM_PROT_SCHEME0, NP1_DMA_SCHEME, uMmProtectionScheme0, pInstance->ramprotection.NarrowPort1DmaAccess );
  uMmProtectionScheme0 = SAND_HAL_MOD_FIELD( C2, MM_PROT_SCHEME0, NP1_PI_SCHEME,  uMmProtectionScheme0, pInstance->ramprotection.NarrowPort1ProcessorInterface );

  SAND_HAL_WRITE_STRIDE( userDeviceHandle, C2, MM, uInstance, MM_PROT_SCHEME0, uMmProtectionScheme0 );

  /* gma - Nov 03 2006 - Need to wait for the dlls to lock prior to enabling the narrow and wide port memories. */


    DEXIT();

    return SB_OK;
}

static uint32_t
sbFe2000XtInitPt(sbhandle userDeviceHandle,
           sbFe2000XtInitParams_t *pInitParams)
{
    int i;
  DENTER();

  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, PT_CORE_RESET, 0x0);

  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_CONFIG0, PTE_PRIORITY,
                     (uint32_t)pInitParams->pt.PipelinePriority);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_CONFIG1, EXPANDED_FIFO_SIZE,
                     pInitParams->pt.uExpandedFifoSize);

  SAND_HAL_WRITE(userDeviceHandle, C2, PT_MIRROR_HEADER_MASK0, pInitParams->pt.uMirrorHeaderMask[0]);
  SAND_HAL_WRITE(userDeviceHandle, C2, PT_MIRROR_HEADER_MASK1, pInitParams->pt.uMirrorHeaderMask[1]);
  SAND_HAL_WRITE(userDeviceHandle, C2, PT_MIRROR_HEADER_MASK2, pInitParams->pt.uMirrorHeaderMask[2]);
  SAND_HAL_WRITE(userDeviceHandle, C2, PT_MIRROR_HEADER_MASK3, pInitParams->pt.uMirrorHeaderMask[3]);

  
  if (pInitParams->sr[0].bBringUp) {
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE0_CONFIG, INIT, 1);
      if( sbFe2000XtInitRegDone(userDeviceHandle,
                              SAND_HAL_REG_OFFSET(C2, PT_PTE0_CONFIG),
                              SAND_HAL_FIELD_MASK(C2, PT_PTE0_CONFIG, INIT_DONE),
                              SB_FE2000XT_PT_INIT_TIMEOUT) ) {

          SB_ERROR("%s:Waiting for PT (SPI0) init_done timed out..\n",
                   __PRETTY_FUNCTION__);
          return SB_FE2000_STS_INIT_PT_SPI0_TIMEOUT_ERR_K;
      }

      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE0_CONFIG, INIT, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE0_CONFIG, INIT_DONE, 0);

      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE0_CONFIG, PTE_ENABLE, 1);
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PT_PTE0_ENABLE_CONFIG0,
                         PORT_ENABLE,0xffffffff);
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PT_PTE0_ENABLE_CONFIG1,
                         PORT_ENABLE,0xffffffff);
  }
  for (i = 0; i < SB_FE2000_MAX_PORTS_PER_SPI; i++) {
      C2PtPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_SPI0, i, 0xff);
  }

  if (pInitParams->sr[1].bBringUp) {
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE1_CONFIG, INIT, 1);
      if( sbFe2000XtInitRegDone(userDeviceHandle,
                              SAND_HAL_REG_OFFSET(C2, PT_PTE1_CONFIG),
                              SAND_HAL_FIELD_MASK(C2, PT_PTE1_CONFIG, INIT_DONE),
                              SB_FE2000XT_PT_INIT_TIMEOUT) ) {

          SB_ERROR("%s:Waiting for PT (SPI1) init_done timed out..\n",
                   __PRETTY_FUNCTION__);
          return SB_FE2000_STS_INIT_PT_SPI1_TIMEOUT_ERR_K;
      }

      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE1_CONFIG, INIT, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE1_CONFIG, INIT_DONE, 0);

      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE1_CONFIG, PTE_ENABLE, 1);
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PT_PTE1_ENABLE_CONFIG0,
                         PORT_ENABLE,0xffffffff);
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PT_PTE1_ENABLE_CONFIG1,
                         PORT_ENABLE,0xffffffff);
  }
  for (i = 0; i < SB_FE2000_MAX_PORTS_PER_SPI; i++) {
      C2PtPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_SPI1, i, 0xff);
  }

  if ( (pInitParams->ag[0].bBringUp) || (pInitParams->xg[2].bBringUp) ){
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE2_CONFIG, INIT, 1);
      if( sbFe2000XtInitRegDone(userDeviceHandle,
                              SAND_HAL_REG_OFFSET(C2, PT_PTE2_CONFIG),
                              SAND_HAL_FIELD_MASK(C2, PT_PTE2_CONFIG, INIT_DONE),
                              SB_FE2000XT_PT_INIT_TIMEOUT) ) {

          SB_ERROR("%s:Waiting for PT (AG0/XG2) init_done timed out..\n",
                   __PRETTY_FUNCTION__);
          return SB_FE2000_STS_INIT_PT_AG0_TIMEOUT_ERR_K;
      }

      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE2_CONFIG, INIT, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE2_CONFIG, INIT_DONE, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE2_CONFIG, PTE_ENABLE, 1);

      SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PT_PTE2_ENABLE_CONFIG,PORT_ENABLE,
                         0xfff);
  }

  if (pInitParams->ag[0].bBringUp) {
      for (i = 0; i < SB_FE2000_MAX_AG_PORTS; i++) {
          C2PtPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_AGM0, i, 0xff);
      }
  }else{
      for (i = 0; i < SB_FE2000_MAX_XG_PORTS; i++) {
          C2PtPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_AGM0, i, 0xff);
      }
  }

  if ( (pInitParams->ag[1].bBringUp) || (pInitParams->xg[3].bBringUp) ){
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE3_CONFIG, INIT, 1);
      if( sbFe2000XtInitRegDone(userDeviceHandle,
                              SAND_HAL_REG_OFFSET(C2, PT_PTE3_CONFIG),
                              SAND_HAL_FIELD_MASK(C2, PT_PTE3_CONFIG, INIT_DONE),
                              SB_FE2000XT_PT_INIT_TIMEOUT) ) {

          SB_ERROR("%s:Waiting for PT (AG1) init_done timed out..\n",
                   __PRETTY_FUNCTION__);
          return SB_FE2000_STS_INIT_PT_AG1_TIMEOUT_ERR_K;
      }

      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE3_CONFIG, INIT, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE3_CONFIG, INIT_DONE, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE3_CONFIG, PTE_ENABLE, 1);

      SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PT_PTE3_ENABLE_CONFIG,PORT_ENABLE,
                         0xfff);
  }
  if (pInitParams->ag[1].bBringUp) {
      for (i = 0; i < SB_FE2000_MAX_AG_PORTS; i++) {
          C2PtPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_AGM1, i, 0xff);
      }
  }else{
      for (i = 0; i < SB_FE2000_MAX_XG_PORTS; i++) {
          C2PtPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_AGM1, i, 0xff);
      }
  }

  if (pInitParams->xg[0].bBringUp) {
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE4_CONFIG, INIT, 1);
      if( sbFe2000XtInitRegDone(userDeviceHandle,
                              SAND_HAL_REG_OFFSET(C2, PT_PTE4_CONFIG),
                              SAND_HAL_FIELD_MASK(C2, PT_PTE4_CONFIG, INIT_DONE),
                              SB_FE2000_PT_INIT_TIMEOUT) ) {

          SB_ERROR("%s:Waiting for PT (XG0) init_done timed out..\n",
                   __PRETTY_FUNCTION__);
          return SB_FE2000_STS_INIT_PT_XG0_TIMEOUT_ERR_K;
      }

      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE4_CONFIG, INIT, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE4_CONFIG, INIT_DONE, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE4_CONFIG, PTE_ENABLE, 1);
  }
  for (i = 0; i < SB_FE2000_MAX_XG_PORTS; i++) {
      C2PtPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_XGM0, i, 0xff);
  }

  if (pInitParams->xg[1].bBringUp) {
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE5_CONFIG, INIT, 1);
      if( sbFe2000XtInitRegDone(userDeviceHandle,
                              SAND_HAL_REG_OFFSET(C2, PT_PTE5_CONFIG),
                              SAND_HAL_FIELD_MASK(C2, PT_PTE5_CONFIG, INIT_DONE),
                              SB_FE2000XT_PT_INIT_TIMEOUT) ) {

          SB_ERROR("%s:Waiting for PT (XG1) init_done timed out..\n",
                   __PRETTY_FUNCTION__);
          return SB_FE2000_STS_INIT_PT_XG1_TIMEOUT_ERR_K;
      }

      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE5_CONFIG, INIT, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE5_CONFIG, INIT_DONE, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE5_CONFIG, PTE_ENABLE, 1);
  }
  for (i = 0; i < SB_FE2000_MAX_XG_PORTS; i++) {
      C2PtPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_XGM1, i, 0xff);
  }

  if (pInitParams->pci.bBringUp) {
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE6_CONFIG, INIT, 1);
      if( sbFe2000XtInitRegDone(userDeviceHandle,
                              SAND_HAL_REG_OFFSET(C2, PT_PTE6_CONFIG),
                              SAND_HAL_FIELD_MASK(C2, PT_PTE6_CONFIG, INIT_DONE),
                              SB_FE2000XT_PT_INIT_TIMEOUT) ) {

          SB_ERROR("%s:Waiting for PT (PCI) init_done timed out..\n",
                   __PRETTY_FUNCTION__);
          return SB_FE2000_STS_INIT_PT_PCI_TIMEOUT_ERR_K;
      }

      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE6_CONFIG, INIT, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE6_CONFIG, INIT_DONE, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE6_CONFIG, PTE_ENABLE, 1);
  }
  C2PtPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_PCI, 0, 0xff);

  if (pInitParams->pp.bBringUp) {
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE7_CONFIG, INIT, 1);
      if( sbFe2000XtInitRegDone(userDeviceHandle,
                              SAND_HAL_REG_OFFSET(C2, PT_PTE7_CONFIG),
                              SAND_HAL_FIELD_MASK(C2, PT_PTE7_CONFIG, INIT_DONE),
                              SB_FE2000XT_PT_INIT_TIMEOUT) ) {

          SB_ERROR("%s:Waiting for PT (PED)  init_done timed out..\n",
                   __PRETTY_FUNCTION__);
          return SB_FE2000_STS_INIT_PT_PED_TIMEOUT_ERR_K;
      }

      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE7_CONFIG, INIT, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE7_CONFIG, INIT_DONE, 0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE7_CONFIG, PTE_ENABLE, 1);

      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_PTE7_CONFIG, PB_ENABLE,
                         sbFe2000XtInitConvertPbEnables(pInitParams, FALSE, 0));
  }

  DEXIT();

  return SB_FE2000_STS_INIT_OK_K;

}

uint32_t
sbFe2000XtEnablePrPorts(sbhandle userDeviceHandle,
                   sbFe2000XtInitParams_t *pInitParams){

  SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE0_ENABLE_CONFIG0,
                     PORT_ENABLE,0xffffffff);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE0_ENABLE_CONFIG1,
                     PORT_ENABLE,0xffffffff);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE1_ENABLE_CONFIG0,
                     PORT_ENABLE,0xffffffff);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE1_ENABLE_CONFIG1,
                     PORT_ENABLE, 0xffffffff);
  if (pInitParams->xg[2].bBringUp) {
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE2_ENABLE_CONFIG, PORT_ENABLE, 0x1);
  }else{
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE2_ENABLE_CONFIG,PORT_ENABLE,
                     0xfff);
  }

  if (pInitParams->xg[3].bBringUp) {
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE3_ENABLE_CONFIG, PORT_ENABLE, 0x1);
  }else{
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE3_ENABLE_CONFIG,PORT_ENABLE,
                     0xfff);
  }

  return SB_FE2000_STS_INIT_OK_K;
}

static uint32_t
sbFe2000XtInitPr(sbhandle userDeviceHandle,
           sbFe2000XtInitParams_t *pInitParams)
{
    int i;
    uint32_t v0, v1;
  DENTER();

  /* rgf - May 11 2006 - Take block out of reset */
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, PR_CORE_RESET, 0x0);

  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_CONFIG, PED_PRIORITY, (uint32_t)pInitParams->pr.PipelinePriority);

  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE0_CONFIG, PRE_ENABLE, 1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE0_CONFIG,
                     PB1_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[0][1]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE0_CONFIG,
                     PB0_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[0][0]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE0_CONFIG, PB_ENABLE,
                     sbFe2000XtInitConvertPbEnables(pInitParams, TRUE, 0));

  SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE0_ENABLE_CONFIG0,
                     PORT_ENABLE,0x0);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE0_ENABLE_CONFIG1,
                     PORT_ENABLE,0x0);
  v0 = 0;
  v1 = 0;
  for (i = 0; i < SB_FE2000_MAX_PORTS_PER_SPI; i++) {
      if (i >= 32) {
          v1 |= (pInitParams->pr.nPre0_PacketBufferSelect[i] << (i % 32));
      } else {
          v0 |= (pInitParams->pr.nPre0_PacketBufferSelect[i] << (i % 32));
      }
      C2PrPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_SPI0, i, 0xff);
  }
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE0_PB_SELECT_CONFIG0,
                     PACKET_BUFFER_SELECT, v0);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE0_PB_SELECT_CONFIG1,
                     PACKET_BUFFER_SELECT, v1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE0_DEBUG1,
                     FP_UNDERFLOW_WAIT_CNT, 16);

  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE1_CONFIG, PRE_ENABLE, 1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE1_CONFIG,
                     PB1_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[1][1]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE1_CONFIG,
                     PB0_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[1][0]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE1_CONFIG, PB_ENABLE,
                     sbFe2000XtInitConvertPbEnables(pInitParams, TRUE, 1));

  SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE1_ENABLE_CONFIG0,
                     PORT_ENABLE,0x0);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE1_ENABLE_CONFIG1,PORT_ENABLE,
                     0x0);
  v0 = 0;
  v1 = 0;
  for (i = 0; i < SB_FE2000_MAX_PORTS_PER_SPI; i++) {
      if (i >= 32) {
          v1 |= (pInitParams->pr.nPre1_PacketBufferSelect[i] << (i % 32));
      } else {
          v0 |= (pInitParams->pr.nPre1_PacketBufferSelect[i] << (i % 32));
      }
      C2PrPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_SPI1, i, 0xff);
  }
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE1_PB_SELECT_CONFIG0,
                     PACKET_BUFFER_SELECT, v0);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE1_PB_SELECT_CONFIG1,
                     PACKET_BUFFER_SELECT, v1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE1_DEBUG1,
                     FP_UNDERFLOW_WAIT_CNT, 16);

  /*  AG0 or XG2*/
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE2_CONFIG, PRE_ENABLE, 1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE2_CONFIG, CC_ENABLE, pInitParams->pr.bCcEnable[0]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE2_CONFIG, PB1_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[2][1]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE2_CONFIG, PB0_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[2][0]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE2_CONFIG, PB_ENABLE,
                     sbFe2000XtInitConvertPbEnables(pInitParams, TRUE, 2));

  SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE2_ENABLE_CONFIG,PORT_ENABLE,
                     0x0);
  v0 = 0;
  for (i = 0; i < SB_FE2000_MAX_AG_PORTS; i++) {
      v0 |= (pInitParams->pr.nPre2_PacketBufferSelect[i] << i);
      C2PrPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_AGM0, i, 0xff);
  }

  SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE2_PB_SELECT_CONFIG,
                     PACKET_BUFFER_SELECT, v0);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE2_DEBUG1,
                     FP_UNDERFLOW_WAIT_CNT, 16);
  if (pInitParams->xg[2].bBringUp) {
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE2_ENABLE_CONFIG, PORT_ENABLE, 0x0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE2_PB_SELECT_CONFIG, PACKET_BUFFER_SELECT, pInitParams->pr.nPre2_PacketBufferSelect[0]);
  }

  /* rgf - Sep 07 2007 - Program always match entries in the final rule of each CC CAM with a drop action */
  /* rgf - Sep 07 2007 - so that if there is "no match" this rule will match and drop the traffic */
  /* rgf - Sep 07 2007 - The symptom will be a PRE drop on enqueue. */
  if( pInitParams->pr.bCcEnable[0] ) {
      uint32_t uCcIndex;
      sbZfC2PrCcCamRamLastEntry_t zLastEntryRam;
      sbZfC2PrCcTcamConfigEntry_t zTcamConfigEntry;
      uint32_t uCfgBuffer[SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_WORDS];
      uint32_t uCfgRamBuffer[SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS];

      sbZfC2PrCcCamRamLastEntry_InitInstance(&zLastEntryRam);
      sbZfC2PrCcTcamConfigEntry_InitInstance(&zTcamConfigEntry);

#ifndef __KERNEL__
      /* rgf - Nov 02 2007 - Initialize ECC check timer to random value */
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_CC2_SCRB_TIMER, SCRB_TIMER, rand());
#else
      
#endif
      /* rgf - Sep 27 2007 - Since there is no HW init for the CC memories, initialize all entries in */
      /* rgf - Sep 27 2007 - the memory to all zero, invalid rule entries.  LAST_VALID_RULE is actually the */
      /* rgf - Sep 27 2007 - last entry in the cam, therefore lowest priority match.  That will be programmed */
      /* rgf - Sep 27 2007 - below to a valid, always match rule. */
      zTcamConfigEntry.m_uValid = SB_FE2000XT_PR_CC_KEY_INVALID;
      sbZfC2PrCcTcamConfigEntry_Pack(&zTcamConfigEntry, (uint8_t*)uCfgBuffer,
                           SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_BYTES);
      sbZfC2PrCcCamRamLastEntry_Pack(&zLastEntryRam, (uint8_t*)uCfgRamBuffer,
                              SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_BYTES);

      for( uCcIndex = 0; uCcIndex < SB_FE2000XT_PR_CC_LAST_VALID_RULE; uCcIndex++ ) {
          if (C2PrCcCamRuleWrite(userDeviceHandle,
                                 2,
                                 uCcIndex,
                                 uCfgBuffer,
                                 uCfgRamBuffer,
                                 0) != TRUE) {
              return SB_FAILED;
          }
      }
      zTcamConfigEntry.m_uValid = SB_FE2000XT_PR_CC_KEY_VALID;

      /* rgf - Sep 07 2007 - Use port default queue. */
      zLastEntryRam.m_uQueueAction = SB_ZF_C2PRCCCAMRAMLASTENTRY_QUEUE_ACT_DEFAULT_ONLY;
      zLastEntryRam.m_uQueue = 0;
      zLastEntryRam.m_uDrop = 0;
      zLastEntryRam.m_uLast = 1;

      sbZfC2PrCcTcamConfigEntry_Pack(&zTcamConfigEntry, (uint8_t*)uCfgBuffer,
                           SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_BYTES);
      sbZfC2PrCcCamRamLastEntry_Pack(&zLastEntryRam, (uint8_t*)uCfgRamBuffer,
                              SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_BYTES);

      if (C2PrCcCamRuleWrite(userDeviceHandle,
                             2,
                             SB_FE2000XT_PR_CC_LAST_VALID_RULE,
                             uCfgBuffer,
                             uCfgRamBuffer,
                             1) != TRUE) {
          return SB_FAILED;
      }
  }

  /* AG1 or XG3 */
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE3_CONFIG, PRE_ENABLE, 1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE3_CONFIG, CC_ENABLE, pInitParams->pr.bCcEnable[1]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE3_CONFIG, PB1_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[3][1]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE3_CONFIG, PB0_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[3][0]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE3_CONFIG, PB_ENABLE,
                     sbFe2000XtInitConvertPbEnables(pInitParams, TRUE, 3));

  SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE3_ENABLE_CONFIG,PORT_ENABLE,
                     0x0);

  v0 = 0;
  for (i = 0; i < SB_FE2000_MAX_AG_PORTS; i++) {
      v0 |= (pInitParams->pr.nPre3_PacketBufferSelect[i] << i);
      C2PrPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_AGM1, i, 0xff);
  }

  SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE3_PB_SELECT_CONFIG,
                     PACKET_BUFFER_SELECT, v0);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE3_DEBUG1,
                     FP_UNDERFLOW_WAIT_CNT, 16);
  if (pInitParams->xg[3].bBringUp) {
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE3_ENABLE_CONFIG, PORT_ENABLE, 0x0);
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE3_PB_SELECT_CONFIG, PACKET_BUFFER_SELECT, pInitParams->pr.nPre3_PacketBufferSelect[0]);
  }

  /* rgf - Sep 07 2007 - Program always match entries in the final rule of each CC CAM with a drop action */
  /* rgf - Sep 07 2007 - so that if there is "no match" this rule will match and drop the traffic */
  /* rgf - Sep 07 2007 - The symptom will be a PRE drop on enqueue. */
  if( pInitParams->pr.bCcEnable[1] ) {
      uint32_t uCcIndex;
      sbZfC2PrCcCamRamLastEntry_t zLastEntryRam;
      sbZfC2PrCcTcamConfigEntry_t zTcamConfigEntry;
      uint32_t uCfgBuffer[SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_WORDS];
      uint32_t uCfgRamBuffer[SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS];

      sbZfC2PrCcCamRamLastEntry_InitInstance(&zLastEntryRam);
      sbZfC2PrCcTcamConfigEntry_InitInstance(&zTcamConfigEntry);

#ifndef __KERNEL__
      /* rgf - Nov 02 2007 - Initialize ECC check timer to random value */
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_CC2_SCRB_TIMER, SCRB_TIMER, rand());
#else
      
#endif
      /* rgf - Sep 27 2007 - Since there is no HW init for the CC memories, initialize all entries in */
      /* rgf - Sep 27 2007 - the memory to all zero, invalid rule entries.  LAST_VALID_RULE is actually the */
      /* rgf - Sep 27 2007 - last entry in the cam, therefore lowest priority match.  That will be programmed */
      /* rgf - Sep 27 2007 - below to a valid, always match rule. */
      zTcamConfigEntry.m_uValid = SB_FE2000XT_PR_CC_KEY_INVALID;
      sbZfC2PrCcTcamConfigEntry_Pack(&zTcamConfigEntry, (uint8_t*)uCfgBuffer,
                           SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_BYTES);
      sbZfC2PrCcCamRamLastEntry_Pack(&zLastEntryRam, (uint8_t*)uCfgRamBuffer,
                              SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_BYTES);

      for( uCcIndex = 0; uCcIndex < SB_FE2000XT_PR_CC_LAST_VALID_RULE; uCcIndex++ ) {
          if (C2PrCcCamRuleWrite(userDeviceHandle,
                                 3,
                                 uCcIndex,
                                 uCfgBuffer,
                                 uCfgRamBuffer,
                                 0) != TRUE) {
              return SB_FAILED;
          }
      }
      zTcamConfigEntry.m_uValid = SB_FE2000XT_PR_CC_KEY_VALID;

      /* rgf - Sep 07 2007 - Use port default queue. */
      zLastEntryRam.m_uQueueAction = SB_ZF_C2PRCCCAMRAMLASTENTRY_QUEUE_ACT_DEFAULT_ONLY;
      zLastEntryRam.m_uQueue = 0;
      zLastEntryRam.m_uDrop = 0;
      zLastEntryRam.m_uLast = 1;

      sbZfC2PrCcTcamConfigEntry_Pack(&zTcamConfigEntry, (uint8_t*)uCfgBuffer,
                           SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_BYTES);
      sbZfC2PrCcCamRamLastEntry_Pack(&zLastEntryRam, (uint8_t*)uCfgRamBuffer,
                              SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_BYTES);

      if (C2PrCcCamRuleWrite(userDeviceHandle,
                             3,
                             SB_FE2000XT_PR_CC_LAST_VALID_RULE,
                             uCfgBuffer,
                             uCfgRamBuffer,
                             1) != TRUE) {
          return SB_FAILED;
      }
  }

  /* XG0 */
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE4_CONFIG, PRE_ENABLE, 1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE4_CONFIG, CC_ENABLE, pInitParams->pr.bCcEnable[2]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE4_CONFIG, PB1_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[4][1]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE4_CONFIG, PB0_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[4][0]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE4_CONFIG, PB_ENABLE,
                     sbFe2000XtInitConvertPbEnables(pInitParams, TRUE, 4));
  for (i = 0; i < SB_FE2000_MAX_XG_PORTS; i++) {
      C2PrPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_XGM0, i, 0xff);
  }
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE4_PB_SELECT_CONFIG,
                     PACKET_BUFFER_SELECT,
                     pInitParams->pr.nPre4_PacketBufferSelect);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE4_DEBUG1,
                     FP_UNDERFLOW_WAIT_CNT, 16);

  /* rgf - Sep 07 2007 - Program always match entries in the final rule of each CC CAM with a drop action */
  /* rgf - Sep 07 2007 - so that if there is "no match" this rule will match and drop the traffic */
  /* rgf - Sep 07 2007 - The symptom will be a PRE drop on enqueue. */
  if( pInitParams->pr.bCcEnable[2] ) {
      uint32_t uCcIndex;
      sbZfC2PrCcCamRamLastEntry_t zLastEntryRam;
      sbZfC2PrCcTcamConfigEntry_t zTcamConfigEntry;
      uint32_t uCfgBuffer[SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_WORDS];
      uint32_t uCfgRamBuffer[SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS];

      sbZfC2PrCcCamRamLastEntry_InitInstance(&zLastEntryRam);
      sbZfC2PrCcTcamConfigEntry_InitInstance(&zTcamConfigEntry);

#ifndef __KERNEL__
      /* rgf - Nov 02 2007 - Initialize ECC check timer to random value */
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_CC2_SCRB_TIMER, SCRB_TIMER, rand());
#else
      
#endif
      /* rgf - Sep 27 2007 - Since there is no HW init for the CC memories, initialize all entries in */
      /* rgf - Sep 27 2007 - the memory to all zero, invalid rule entries.  LAST_VALID_RULE is actually the */
      /* rgf - Sep 27 2007 - last entry in the cam, therefore lowest priority match.  That will be programmed */
      /* rgf - Sep 27 2007 - below to a valid, always match rule. */
      zTcamConfigEntry.m_uValid = SB_FE2000XT_PR_CC_KEY_INVALID;
      sbZfC2PrCcTcamConfigEntry_Pack(&zTcamConfigEntry, (uint8_t*)uCfgBuffer,
                           SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_BYTES);
      sbZfC2PrCcCamRamLastEntry_Pack(&zLastEntryRam, (uint8_t*)uCfgRamBuffer,
                              SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_BYTES);

      for( uCcIndex = 0; uCcIndex < SB_FE2000XT_PR_CC_LAST_VALID_RULE; uCcIndex++ ) {
          if (C2PrCcCamRuleWrite(userDeviceHandle,
                                 4,
                                 uCcIndex,
                                 uCfgBuffer,
                                 uCfgRamBuffer,
                                 0) != TRUE) {
              return SB_FAILED;
          }
      }
      zTcamConfigEntry.m_uValid = SB_FE2000XT_PR_CC_KEY_VALID;

      /* rgf - Sep 07 2007 - Use port default queue. */
      zLastEntryRam.m_uQueueAction = SB_ZF_C2PRCCCAMRAMLASTENTRY_QUEUE_ACT_DEFAULT_ONLY;
      zLastEntryRam.m_uQueue = 0;
      zLastEntryRam.m_uDrop = 0;
      zLastEntryRam.m_uLast = 1;

      sbZfC2PrCcTcamConfigEntry_Pack(&zTcamConfigEntry, (uint8_t*)uCfgBuffer,
                           SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_BYTES);
      sbZfC2PrCcCamRamLastEntry_Pack(&zLastEntryRam, (uint8_t*)uCfgRamBuffer,
                              SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_BYTES);

      if (C2PrCcCamRuleWrite(userDeviceHandle,
                             4,
                             SB_FE2000XT_PR_CC_LAST_VALID_RULE,
                             uCfgBuffer,
                             uCfgRamBuffer,
                             1) != TRUE) {
          return SB_FAILED;
      }
  }

  /*  XG1 */
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE5_CONFIG, PRE_ENABLE, 1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE5_CONFIG, CC_ENABLE, pInitParams->pr.bCcEnable[3]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE5_CONFIG, PB1_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[5][1]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE5_CONFIG, PB0_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[5][0]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE5_CONFIG, PB_ENABLE,
                     sbFe2000XtInitConvertPbEnables(pInitParams, TRUE, 5));
  for (i = 0; i < SB_FE2000_MAX_XG_PORTS; i++) {
      C2PrPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_XGM1, i, 0xff);
  }

  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE5_PB_SELECT_CONFIG,
                     PACKET_BUFFER_SELECT,
                     pInitParams->pr.nPre5_PacketBufferSelect);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE5_DEBUG1,
                     FP_UNDERFLOW_WAIT_CNT, 16);

  /* rgf - Sep 07 2007 - Program always match entries in the final rule of each CC CAM with a drop action */
  /* rgf - Sep 07 2007 - so that if there is "no match" this rule will match and drop the traffic */
  /* rgf - Sep 07 2007 - The symptom will be a PRE drop on enqueue. */
  if( pInitParams->pr.bCcEnable[3] ) {
      uint32_t uCcIndex;
      sbZfC2PrCcCamRamLastEntry_t zLastEntryRam;
      sbZfC2PrCcTcamConfigEntry_t zTcamConfigEntry;
      uint32_t uCfgBuffer[SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_WORDS];
      uint32_t uCfgRamBuffer[SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS];

      sbZfC2PrCcCamRamLastEntry_InitInstance(&zLastEntryRam);
      sbZfC2PrCcTcamConfigEntry_InitInstance(&zTcamConfigEntry);

#ifndef __KERNEL__
      /* rgf - Nov 02 2007 - Initialize ECC check timer to random value */
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_CC2_SCRB_TIMER, SCRB_TIMER, rand());
#else
      
#endif 
      /* rgf - Sep 27 2007 - Since there is no HW init for the CC memories, initialize all entries in */
      /* rgf - Sep 27 2007 - the memory to all zero, invalid rule entries.  LAST_VALID_RULE is actually the */
      /* rgf - Sep 27 2007 - last entry in the cam, therefore lowest priority match.  That will be programmed */
      /* rgf - Sep 27 2007 - below to a valid, always match rule. */
      zTcamConfigEntry.m_uValid = SB_FE2000XT_PR_CC_KEY_INVALID;
      sbZfC2PrCcTcamConfigEntry_Pack(&zTcamConfigEntry, (uint8_t*)uCfgBuffer,
                           SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_BYTES);
      sbZfC2PrCcCamRamLastEntry_Pack(&zLastEntryRam, (uint8_t*)uCfgRamBuffer,
                              SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_BYTES);

      for( uCcIndex = 0; uCcIndex < SB_FE2000XT_PR_CC_LAST_VALID_RULE; uCcIndex++ ) {
          if (C2PrCcCamRuleWrite(userDeviceHandle,
                                 5,
                                 uCcIndex,
                                 uCfgBuffer,
                                 uCfgRamBuffer,
                                 0) != TRUE) {
              return SB_FAILED;
          }
      }
      zTcamConfigEntry.m_uValid = SB_FE2000XT_PR_CC_KEY_VALID;

      /* rgf - Sep 07 2007 - Use port default queue. */
      zLastEntryRam.m_uQueueAction = SB_ZF_C2PRCCCAMRAMLASTENTRY_QUEUE_ACT_DEFAULT_ONLY;
      zLastEntryRam.m_uQueue = 0;
      zLastEntryRam.m_uDrop = 0;
      zLastEntryRam.m_uLast = 1;

      sbZfC2PrCcTcamConfigEntry_Pack(&zTcamConfigEntry, (uint8_t*)uCfgBuffer,
                           SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_BYTES);
      sbZfC2PrCcCamRamLastEntry_Pack(&zLastEntryRam, (uint8_t*)uCfgRamBuffer,
                              SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_BYTES);

      if (C2PrCcCamRuleWrite(userDeviceHandle,
                             5,
                             SB_FE2000XT_PR_CC_LAST_VALID_RULE,
                             uCfgBuffer,
                             uCfgRamBuffer,
                             1) != TRUE) {
          return SB_FAILED;
      }
  }

  /* PCI */
  /* C2 spec, 3.4.5 Packet Receiver:
   * The Packet Receiver Element for the PCI (PRE6) is connected to Packet 
   * Buffer 0 only, for this reason, PR_PRE[6]_PB_SELECT_CONFIG must be left
   * at its reset value of 0. In addition, the PR_PRE6_CONFIG.PB_ENABLE field
   * should be configured to a value of 0x1 only to prevent the prefetch of
   * free pages from Packet Buffer 1.
   */
  if (pInitParams->pr.uPbPrefetchPages[6][1] || 
      pInitParams->pr.nPre6_PacketBufferSelect)
  {
      soc_cm_print("%s: invalid config for PRE6\n", FUNCTION_NAME());
      return SB_FE2000_STS_INIT_BAD_ARGS;
  }

  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE6_CONFIG, PRE_ENABLE, 1);

  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE6_CONFIG, PB1_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[6][1]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE6_CONFIG, PB0_PREFETCH_PAGES,
                     pInitParams->pr.uPbPrefetchPages[6][0]);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE6_CONFIG, PB_ENABLE,
                     sbFe2000XtInitConvertPbEnables(pInitParams, TRUE, 6));
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE6_PB_SELECT_CONFIG,
                     PACKET_BUFFER_SELECT,
                     pInitParams->pr.nPre6_PacketBufferSelect);

  C2PrPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_PCI, 0, 0xff);

  /* PED */
  if (pInitParams->pr.bPrEnable[7]) {
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE7_CONFIG, PRE_ENABLE, 1);
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_PRE7_CONFIG, PB_ENABLE,
                         sbFe2000XtInitConvertPbEnables(pInitParams, TRUE, 7));
  }

  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_CC2_TM, TCAM_TM, 0x10);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_CC3_TM, TCAM_TM, 0x10);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_CC4_TM, TCAM_TM, 0x10);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_CC5_TM, TCAM_TM, 0x10);

  DEXIT();
  return SB_FE2000_STS_INIT_OK_K;

}

static uint32_t
sbFe2000XtInitPp(sbhandle userDeviceHandle,
           sbFe2000XtInitParams_t *pInitParams) {

  uint32_t uIndex;
  uint32_t uQueue;
  sbFe2000XtInitParamsPp_t *pPpInitParams = &(pInitParams->pp);
  uint32_t uEthTypeMaxMin = 0;
  uint32_t uPpHdrTypeConfig0;
  uint32_t uPpSourceBufferBatch;

  DENTER();

  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_CORE_RESET, PP_CORE_RESET, 0x0);

  /* rgf - Aug 10 2007 - Hit the init bit to initialize the memories */
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_CONFIG, INIT, 0x1);

  if( sbFe2000XtInitRegDone(userDeviceHandle,
              SAND_HAL_REG_OFFSET(C2, PP_CONFIG),
              SAND_HAL_FIELD_MASK(C2, PP_CONFIG, INIT_DONE),
              SB_FE2000_PP_INIT_TIMEOUT) ) {

    SB_ERROR("%s Waiting for PP init_done timed out..\n",__PRETTY_FUNCTION__);
    return SB_FE2000_STS_INIT_PP_TIMEOUT_ERR_K;
  }

  /*  rgf - Jun 20 2006 - Clear both the init and the init_done */
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_CONFIG, INIT, 0);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_CONFIG, INIT_DONE, 0);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PP_TCAM_GLOBAL_CONFIG, 0xFFFFFFFF);

  uEthTypeMaxMin = SAND_HAL_MOD_FIELD(C2,PP_ETH_TYPE_RANGE,MIN_TYPE,uEthTypeMaxMin, pPpInitParams->uEthernetTypeExceptionMin);
  uEthTypeMaxMin = SAND_HAL_MOD_FIELD(C2,PP_ETH_TYPE_RANGE,MAX_TYPE,uEthTypeMaxMin, pPpInitParams->uEthernetTypeExceptionMax);
  SAND_HAL_WRITE(userDeviceHandle, C2,PP_ETH_TYPE_RANGE,uEthTypeMaxMin);

  /* configure the queue initial header type */
  for(uQueue=0; uQueue < pPpInitParams->uDefaultNumberofQueuesToConfigure;  uQueue++) {
    sbZfC2PpInitialQueueStateEntry_t zInitialQueueState;
    sbZfC2PpInitialQueueStateEntry_InitInstance(&zInitialQueueState);
    zInitialQueueState.m_uHeaderType =
        pInitParams->pp.QueueInitialState[uQueue].uHeaderType;
    zInitialQueueState.m_uHeaderShift =
        pInitParams->pp.QueueInitialState[uQueue].uShift;
    zInitialQueueState.m_uState =
        pInitParams->pp.QueueInitialState[uQueue].uState;
    zInitialQueueState.m_uVariable =
        pInitParams->pp.QueueInitialState[uQueue].uVariable;

    if ( C2PpInitialQueueStateWrite(userDeviceHandle, uQueue, &zInitialQueueState) ) {
    } else {
      SB_ERROR("%s Failed on write for queue(%d).",__PRETTY_FUNCTION__, uQueue);
    }
  }

  /* CAM and HASH Programming Section */
  /* -- this is done via ilib init (sbG2FePp.c) */

  /* configure the queue default record size */
  for(uIndex=0; uIndex < SB_FE2000_PP_NUM_BATCHGROUPS; uIndex++) {

    sbZfC2PpHeaderRecordSizeEntry_t zQueueRecordSize;
    sbZfC2PpHeaderRecordSizeEntry_InitInstance(&zQueueRecordSize);
    zQueueRecordSize.m_uRecordSize = pPpInitParams->uHeaderRecordSize[uIndex];

    if( C2PpHeaderRecordSizeWrite(userDeviceHandle,uIndex,&zQueueRecordSize) ) {
    } else {
      SB_ERROR("%s Failed on write for queue(%d).",__PRETTY_FUNCTION__, uQueue);
    }
  }

  /* configure the queue batch group and priority */
  for(uQueue=0; uQueue < pPpInitParams->uDefaultNumberofQueuesToConfigure;uQueue++) {

    sbZfC2PpQueuePriorityGroupEntry_t zQueueConfig;
    sbZfC2PpQueuePriorityGroupEntry_InitInstance(&zQueueConfig);
    zQueueConfig.m_uPriority = pPpInitParams->QueueConfiguration[uQueue].uPriority;
    zQueueConfig.m_uBatchGroup = pPpInitParams->QueueConfiguration[uQueue].uBatchGroup;

    if( C2PpQueuePriorityGroupWrite(userDeviceHandle,uQueue,&zQueueConfig) ) {
    } else {
      SB_ERROR("%s Failed on write for queue(%d).",__PRETTY_FUNCTION__, uQueue);
    }
  }

  /* dgm - Dec 28 2006 - Source buffer insert */

  uPpHdrTypeConfig0 = SAND_HAL_READ(userDeviceHandle, C2,PP_HDR_TYPE_CONFIG0);
  uPpSourceBufferBatch = SAND_HAL_GET_FIELD(C2,PP_HDR_TYPE_CONFIG0,
                                            SRC_BUFFER_BATCH,
                                            uPpHdrTypeConfig0);
  for(uIndex=0; uIndex < SB_FE2000_PP_NUM_BATCHGROUPS; uIndex++) {
    if( pPpInitParams->bSourceBufferInsert[uIndex] ) {
      uPpSourceBufferBatch |= (1 << uIndex);
    }
  }

  uPpHdrTypeConfig0 = SAND_HAL_MOD_FIELD(C2,PP_HDR_TYPE_CONFIG0,
                                         SRC_BUFFER_BATCH,uPpHdrTypeConfig0,
                                         uPpSourceBufferBatch);
  /* dgm - Oct 12 2007 - Exception stream hooks */
  uPpHdrTypeConfig0 = SAND_HAL_MOD_FIELD(C2,PP_HDR_TYPE_CONFIG0,STR_VALUE,uPpHdrTypeConfig0,pPpInitParams->uExceptionStream);
  uPpHdrTypeConfig0 = SAND_HAL_MOD_FIELD(C2,PP_HDR_TYPE_CONFIG0,STR_EXCEPTION_EN,uPpHdrTypeConfig0,pPpInitParams->uExceptionStreamEnable);
  SAND_HAL_WRITE(userDeviceHandle, C2,PP_HDR_TYPE_CONFIG0,uPpHdrTypeConfig0);


  {
    /* LRP By Pass Mode configuration */
     uint32_t uDebugPedHeader = SAND_HAL_READ(userDeviceHandle, C2,PP_DEBUG_PED_HDR);
     uint32_t uPpDebugConfig;
     uint32_t uPpMakePedHeader;
     uint32_t uPpAddRouteHeader;
     uint32_t uPpDeleteFirstHeader;

     uDebugPedHeader = SAND_HAL_MOD_FIELD(C2,PP_DEBUG_PED_HDR,MIRROR_INDEX,uDebugPedHeader,pPpInitParams->PpByPassLrpModeConfiguration.uMirrorIndex);
     uDebugPedHeader = SAND_HAL_MOD_FIELD(C2,PP_DEBUG_PED_HDR,COPY_CNT,uDebugPedHeader,pPpInitParams->PpByPassLrpModeConfiguration.uCopyCount);
     uDebugPedHeader = SAND_HAL_MOD_FIELD(C2,PP_DEBUG_PED_HDR,HDR_COPY,uDebugPedHeader,pPpInitParams->PpByPassLrpModeConfiguration.bHeaderCopy);
     uDebugPedHeader = SAND_HAL_MOD_FIELD(C2,PP_DEBUG_PED_HDR,DROP,uDebugPedHeader,pPpInitParams->PpByPassLrpModeConfiguration.bDrop);
     SAND_HAL_WRITE(userDeviceHandle, C2,PP_DEBUG_PED_HDR,uDebugPedHeader);

      uPpDebugConfig = SAND_HAL_READ(userDeviceHandle, C2,PP_DEBUG_CONFIG);
      uPpMakePedHeader = SAND_HAL_GET_FIELD(C2,PP_DEBUG_CONFIG,C_USE_PED_HDR,uPpDebugConfig);

      for(uIndex=0; uIndex < SB_FE2000_PP_NUM_BATCHGROUPS; uIndex++) {
        if( pPpInitParams->PpByPassLrpModeConfiguration.bByPassLrpMode[uIndex] ) {
            uPpMakePedHeader |= (1 << uIndex);
        }
      }

      uPpAddRouteHeader = SAND_HAL_GET_FIELD(C2,PP_DEBUG_CONFIG,C_ADD_ROUTE_HDR,uPpDebugConfig);
      for(uIndex=0; uIndex < SB_FE2000_PP_NUM_BATCHGROUPS; uIndex++) {
        if( pPpInitParams->bAddRouteHeader[uIndex] ) {
            uPpAddRouteHeader |= (1 << uIndex);
        }
      }

      uPpDeleteFirstHeader = SAND_HAL_GET_FIELD(C2,PP_DEBUG_CONFIG,C_DELETE_FIRST_HDR,uPpDebugConfig);
      for(uIndex=0; uIndex < SB_FE2000_PP_NUM_BATCHGROUPS; uIndex++) {
        if( pPpInitParams->bDeleteFirstHeader[uIndex] ) {
            uPpDeleteFirstHeader |= (1 << uIndex);
        }
      }
      uPpDebugConfig = SAND_HAL_MOD_FIELD(C2,PP_DEBUG_CONFIG,C_USE_PED_HDR,uPpDebugConfig,uPpMakePedHeader);
      uPpDebugConfig = SAND_HAL_MOD_FIELD(C2,PP_DEBUG_CONFIG,C_ADD_ROUTE_HDR,uPpDebugConfig,uPpAddRouteHeader);
      uPpDebugConfig = SAND_HAL_MOD_FIELD(C2,PP_DEBUG_CONFIG,C_DELETE_FIRST_HDR,uPpDebugConfig,uPpDeleteFirstHeader);
      SAND_HAL_WRITE(userDeviceHandle, C2,PP_DEBUG_CONFIG,uPpDebugConfig);
  }

  {
    /* exceptions initialization */
    if(pInitParams->pp.exceptions.bEnableAllExceptions == TRUE) {
      SAND_HAL_WRITE(userDeviceHandle, C2,PP_EVENT0_MASK, 0);
      SAND_HAL_WRITE(userDeviceHandle, C2,PP_EVENT1_MASK, 0);
    }
  }

  /* rgf - Nov 02 2007 - Initialize ECC check timer to random value
   * SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_TCAM_SCAN_CONFIG, TCAM_WINDOW_CNT, m_pCcScrubTimerSelect->Random());
   * dgm - Dec 05 2007 - Turning off due to load extend errors */
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_TCAM_SCAN_CONFIG, TCAM_WINDOW_CNT, 0);

  /* Set TCAM test registers */
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_TCAM_TEST1, TM3, 0x10);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_TCAM_TEST0, TM2, 0x10);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_TCAM_TEST0, TM1, 0x10);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_TCAM_TEST0, TM0, 0x10);

  DEXIT();
  return SB_FE2000_STS_INIT_OK_K;
}



/* rgf - Nov 13 2006 - Convert the boolean array for PB enable to a 2 bit mask */
/* rgf - Nov 13 2006 - bPre equals true if called for a PRE, otherwise this call is for PTE7 */
/* rgf - Nov 13 2006 - uPre indicates which PRE if bPre is true */
uint32_t sbFe2000XtInitConvertPbEnables(sbFe2000XtInitParams_t *pInitParams, uint8_t bPre, uint32_t uPre)
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
void sbFe2000XtInitCleanUp(sbhandle userDeviceHandle, sbFe2000XtInitParams_t *pInitParams)
{
  uint16_t nNum;
  DENTER();
  sbFe2000XtInitCleanUpPt(userDeviceHandle);
  sbFe2000XtInitCleanUpQm(userDeviceHandle);
  sbFe2000XtInitCleanUpPb(userDeviceHandle);
  sbFe2000XtInitCleanUpPr(userDeviceHandle);
  sbFe2000XtInitCleanUpPp(userDeviceHandle);
  sbFe2000XtInitCleanUpPd(userDeviceHandle);

  for(nNum=0;nNum<SB_FE2000_NUM_MM_INSTANCES;nNum++) {
    sbFe2000XtInitCleanUpMm(userDeviceHandle,nNum);
  }

  sbFe2000XtInitCleanUpCm(userDeviceHandle);

  for(nNum=0;nNum<SB_FE2000_NUM_RC_INSTANCES;nNum++) {
    sbFe2000XtInitCleanUpRc(userDeviceHandle,nNum);
  }

  sbFe2000XtInitCleanUpPm(userDeviceHandle);
  sbFe2000XtInitCleanUpLr(userDeviceHandle,pInitParams);

  for(nNum=0;nNum<SB_FE2000_NUM_AG_INTERFACES;nNum++) {
    sbFe2000XtInitCleanUpAgm(userDeviceHandle,nNum);
  }
  for(nNum=0;nNum<SB_FE2000XT_NUM_XG_INTERFACES;nNum++) {
    sbFe2000XtInitCleanUpXgm(userDeviceHandle,nNum);
  }

  sbFe2000XtInitCleanUpSpi4(userDeviceHandle,0);
  sbFe2000XtInitCleanUpSpi4(userDeviceHandle,1);

  SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PC_CONFIG, CII_ENABLE, 0x1);

  DEXIT();
}

static
void sbFe2000XtInitCleanUpPt(sbhandle userDeviceHandle)
{
  /* rgf - May 12 2006 - Read and write back the PTE Error registers to clear */
  /* rgf - May 12 2006 - any error bits set and then unmask the error register */
  uint32_t uError = SAND_HAL_READ(userDeviceHandle, C2, PT_PTE0_ERROR0);
  DENTER();
  SAND_HAL_WRITE(userDeviceHandle, C2,  PT_PTE0_ERROR0, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PT_PTE0_ERROR0_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, C2, PT_PTE0_ERROR1);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PT_PTE0_ERROR1, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PT_PTE0_ERROR1_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, C2, PT_PTE1_ERROR0);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PT_PTE1_ERROR0, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PT_PTE1_ERROR0_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, C2, PT_PTE1_ERROR1);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PT_PTE1_ERROR1, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PT_PTE1_ERROR1_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, C2, PT_PTE2_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PT_PTE2_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PT_PTE2_ERROR_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, C2, PT_PTE3_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PT_PTE3_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PT_PTE3_ERROR_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, C2, PT_PTE4_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PT_PTE4_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PT_PTE4_ERROR_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, C2, PT_PTE5_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PT_PTE5_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PT_PTE5_ERROR_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, C2, PT_PTE6_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PT_PTE6_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PT_PTE6_ERROR_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, C2, PT_PTE7_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PT_PTE7_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PT_PTE7_ERROR_MASK, 0);

  DEXIT();
}

static
void sbFe2000XtInitCleanUpQm(sbhandle userDeviceHandle)
{
  /* rgf - May 12 2006 - Clear ERROR0/ERROR1 simply by reading them, all fields are counters */
  uint32_t uError = SAND_HAL_READ(userDeviceHandle, C2, QM_ERROR0);
  DENTER();
  uError = SAND_HAL_READ(userDeviceHandle, C2, QM_ERROR1);
  /* rgf - May 12 2006 - and unmask the error registers */
  SAND_HAL_WRITE(userDeviceHandle, C2,  QM_ERROR0_MASK, 0);
  SAND_HAL_WRITE(userDeviceHandle, C2,  QM_ERROR1_MASK, 0);
  SAND_HAL_WRITE(userDeviceHandle, C2,  QM_ERROR2_MASK, 0);
  SAND_HAL_WRITE(userDeviceHandle, C2,  QM_ERROR3_MASK, 0);

  DEXIT();
}

static
void sbFe2000XtInitCleanUpPb(sbhandle userDeviceHandle)
{
  DENTER();
  DEXIT();
}

static
void sbFe2000XtInitCleanUpPr(sbhandle userDeviceHandle)
{
  /* rgf - May 12 2006 - Read PR_ERROR and write back to clear all errors, then unmask */
  uint32_t uError = SAND_HAL_READ(userDeviceHandle, C2, PR_ERROR);
  DENTER();
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_ERROR_MASK, 0);

  /* rgf - May 12 2006 - Now do the same for all the PRE error registers */
  /* rgf - May 12 2006 - a bit repetitive, could use the stride trick but... */
  uError = SAND_HAL_READ(userDeviceHandle, C2, PR_PRE0_ERROR0);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE0_ERROR0, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE0_ERROR0_MASK, 0);
  uError = SAND_HAL_READ(userDeviceHandle, C2, PR_PRE0_ERROR1);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE0_ERROR1, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE0_ERROR1_MASK, 0);
  uError = SAND_HAL_READ(userDeviceHandle, C2, PR_PRE0_ERROR2);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE0_ERROR2, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE0_ERROR2_MASK, 0);
  uError = SAND_HAL_READ(userDeviceHandle, C2, PR_PRE0_ERROR3);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE0_ERROR3, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE0_ERROR3_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, C2, PR_PRE1_ERROR0);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE1_ERROR0, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE1_ERROR0_MASK, 0);
  uError = SAND_HAL_READ(userDeviceHandle, C2, PR_PRE1_ERROR1);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE1_ERROR1, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE1_ERROR1_MASK, 0);
  uError = SAND_HAL_READ(userDeviceHandle, C2, PR_PRE1_ERROR2);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE1_ERROR2, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE1_ERROR2_MASK, 0);
  uError = SAND_HAL_READ(userDeviceHandle, C2, PR_PRE1_ERROR3);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE1_ERROR3, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE1_ERROR3_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, C2, PR_PRE2_ERROR0);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE2_ERROR0, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE2_ERROR0_MASK, 0);
  uError = SAND_HAL_READ(userDeviceHandle, C2, PR_PRE2_ERROR1);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE2_ERROR1, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE2_ERROR1_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, C2, PR_PRE3_ERROR0);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE3_ERROR0, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE3_ERROR0_MASK, 0);
  uError = SAND_HAL_READ(userDeviceHandle, C2, PR_PRE3_ERROR1);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE3_ERROR1, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE3_ERROR1_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, C2, PR_PRE4_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE4_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE4_ERROR_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, C2, PR_PRE5_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE5_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE5_ERROR_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, C2, PR_PRE6_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE6_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE6_ERROR_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, C2, PR_PRE7_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE7_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PR_PRE7_ERROR_MASK, 0);

  DEXIT();
}

static
void sbFe2000XtInitCleanUpPp(sbhandle userDeviceHandle)
{
  uint32_t uError = SAND_HAL_READ(userDeviceHandle, C2, PP_EVENT0);
  DENTER();
  SAND_HAL_WRITE(userDeviceHandle, C2,  PP_EVENT0, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PP_EVENT0_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, C2, PP_EVENT1);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PP_EVENT1, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  PP_EVENT1_MASK, 0);

  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_SP_RAM_DEBUG4, SP_RAM4_DISABLE_ECC, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_SP_RAM_DEBUG5, SP_RAM5_DISABLE_ECC, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_SP_RAM_DEBUG6, SP_RAM6_DISABLE_ECC, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_SP_RAM_DEBUG7, SP_RAM7_DISABLE_ECC, 0x1);

  /* dgm - Feb 19 2008 - Clear out un-correctable error registers in pp */
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_IQSM_RAM_DEBUG, IQSM_RAM_ECC_ERROR, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_LKUP_RAM_DEBUG0, LKUP_RAM0_ECC_ERROR, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_LKUP_RAM_DEBUG1, LKUP_RAM1_ECC_ERROR, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_LKUP_RAM_DEBUG2, LKUP_RAM2_ECC_ERROR, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_LKUP_RAM_DEBUG3, LKUP_RAM3_ECC_ERROR, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_SP_RAM_DEBUG0, SP_RAM0_ECC_ERROR, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_SP_RAM_DEBUG1, SP_RAM1_ECC_ERROR, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_SP_RAM_DEBUG2, SP_RAM2_ECC_ERROR, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_SP_RAM_DEBUG3, SP_RAM3_ECC_ERROR, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_SP_RAM_DEBUG4, SP_RAM4_ECC_ERROR, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_SP_RAM_DEBUG5, SP_RAM5_ECC_ERROR, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_SP_RAM_DEBUG6, SP_RAM6_ECC_ERROR, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_SP_RAM_DEBUG7, SP_RAM7_ECC_ERROR, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PP_TCAM_SCAN_DEBUG, ECC_ERROR, 0x1);

  DEXIT();
}

static
void sbFe2000XtInitCleanUpPd(sbhandle userDeviceHandle)
{
  DENTER();
  /* rgf - Oct 17 2006 - To some extent, these probably shouldn't even be interrupts but leave them */
  /* rgf - Oct 17 2006 - masked otherwise every test which uses the pipeline will fail wrapup with these */
  /* rgf - Oct 17 2006 - interrupts set. */
  SAND_HAL_WRITE(userDeviceHandle, C2,  PD_COPY_IN_EVENT, SAND_HAL_SET_FIELD(C2, PD_COPY_IN_EVENT, HDR_REC_OVERFLOW, 1));
  SAND_HAL_WRITE(userDeviceHandle, C2,  PD_COPY_IN_EVENT_MASK, 0);

  SAND_HAL_WRITE(userDeviceHandle, C2,  PD_COPY_OUT_EVENT, SAND_HAL_SET_FIELD(C2, PD_COPY_OUT_EVENT, HDR_REC_OVERFLOW, 1));
  SAND_HAL_WRITE(userDeviceHandle, C2,  PD_COPY_OUT_EVENT_MASK, 0);

  DEXIT();
}

static
void sbFe2000XtInitCleanUpMm(sbhandle userDeviceHandle, uint16_t nNum)
{
  uint32_t uError = SAND_HAL_READ_STRIDE(userDeviceHandle, C2, MM, nNum, MM_ERROR);
  DENTER();
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2, MM, nNum, MM_ERROR, uError);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2, MM, nNum, MM_ERROR_MASK, 0);

  if ( nNum==0 ) {
  } else if ( nNum==1 ) {
  } else {
    SB_ASSERT(0);
  }
  DEXIT();
}

static
void sbFe2000XtInitCleanUpCm(sbhandle userDeviceHandle)
{
  uint32_t uError = SAND_HAL_READ(userDeviceHandle, C2, CM_SEGMENT_DISABLE_ERROR);
  uint32_t uCmLfsrEnable = 0;
  DENTER();
  SAND_HAL_WRITE(userDeviceHandle, C2,  CM_SEGMENT_DISABLE_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  CM_SEGMENT_DISABLE_ERROR_MASK, 0xffffffff);

  uError = SAND_HAL_READ(userDeviceHandle, C2, CM_SEGMENT_RANGE_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, C2,  CM_SEGMENT_RANGE_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  CM_SEGMENT_RANGE_ERROR_MASK, 0xffffffff);

  uError = SAND_HAL_READ(userDeviceHandle, C2, CM_ERROR);
  SAND_HAL_WRITE(userDeviceHandle, C2,  CM_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  CM_ERROR_MASK, 0xffffffff);

  uError = SAND_HAL_READ(userDeviceHandle, C2, CM_SEGMENT_FLUSH_EVENT);
  SAND_HAL_WRITE(userDeviceHandle, C2,  CM_SEGMENT_FLUSH_EVENT, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  CM_SEGMENT_FLUSH_EVENT_MASK, 0xffffffff);

  uError = SAND_HAL_READ(userDeviceHandle, C2, CM_FORCE_FLUSH_EVENT);
  SAND_HAL_WRITE(userDeviceHandle, C2,  CM_FORCE_FLUSH_EVENT, uError);
  /*
  SAND_HAL_WRITE(userDeviceHandle, C2,  CM_FORCE_FLUSH_EVENT_MASK, 0xffffffff);
  */

  /* rgf - Oct 16 2006 - Clear on read */
  uError = SAND_HAL_READ(userDeviceHandle, C2, CM_TOTAL_NUM_UPDATES);
  /* jts - Oct 31 2006 - new tag, CM_TOTAL_NUM_UPDATES_MASK removed: */
  /* jts - Oct 31 2006 - SAND_HAL_WRITE(userDeviceHandle, C2,  CM_TOTAL_NUM_UPDATES_MASK, 0); */


  uCmLfsrEnable = SAND_HAL_MOD_FIELD( C2, CM_LFSR_ENABLE, ENABLE3, uCmLfsrEnable, 0x1 );
  uCmLfsrEnable = SAND_HAL_MOD_FIELD( C2, CM_LFSR_ENABLE, ENABLE2, uCmLfsrEnable, 0x1 );
  uCmLfsrEnable = SAND_HAL_MOD_FIELD( C2, CM_LFSR_ENABLE, ENABLE1, uCmLfsrEnable, 0x1 );
  uCmLfsrEnable = SAND_HAL_MOD_FIELD( C2, CM_LFSR_ENABLE, ENABLE0, uCmLfsrEnable, 0x1 );
  SAND_HAL_WRITE(userDeviceHandle, C2, CM_LFSR_ENABLE, uCmLfsrEnable );
  SAND_HAL_WRITE(userDeviceHandle, C2, CM_LFSR_ENABLE, 0x0 );
  DEXIT();
}

static
void sbFe2000XtInitCleanUpRc(sbhandle userDeviceHandle, uint16_t nNum)
{
  uint32_t uError = SAND_HAL_READ(userDeviceHandle, C2, RC_ERROR);
  DENTER();
  SAND_HAL_WRITE(userDeviceHandle, C2,  RC_ERROR, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  RC_ERROR_MASK, 0);

  if ( nNum==0 ) {
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, RC0_ERROR_MASK,
                       FIFO_NOT_AVAIL_DISINT, 0);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, RC0_ERROR_MASK,
                       PROG_NOT_CONFIGURED_DISINT, 0);
  } else if ( nNum==1 ) {
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, RC1_ERROR_MASK,
                       FIFO_NOT_AVAIL_DISINT, 0);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, RC1_ERROR_MASK,
                       PROG_NOT_CONFIGURED_DISINT, 0);
  } else {
    SB_ASSERT(0);
  }
  DEXIT();
}


static
void sbFe2000XtInitCleanUpPm(sbhandle userDeviceHandle)
{
  DENTER();
  /* rgf - Oct 29 2007 - Without initializing all of the MM locations defined by the PM groups */
  /* rgf - Oct 29 2007 - (which even if they were all being used would not happen normally until */
  /* rgf - Oct 29 2007 - provision), the PM refresh will attempt to access profile memory with X */
  /* rgf - Oct 29 2007 - for address generating uncorrectable ECC errors.  For now, keep the PM */
  /* rgf - Oct 29 2007 - profile memory ECC errors masked. */
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PM_ECC_ERROR_MASK, CORRECTED_DISINT, 1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PM_ECC_ERROR_MASK, UNCORRECTABLE_DISINT, 1);

  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_UNIT_INTERRUPT_MASK, PM_DISINT, 0);

  DEXIT();
}

static
void sbFe2000XtInitCleanUpLr(sbhandle userDeviceHandle, sbFe2000XtInitParams_t *pInitParams)
{
  uint32_t uEvent;
  uint32_t uTimeOut;
  uint32_t uError;

  DENTER();

  if ( pInitParams->lr.bStreamOnline[0] && pInitParams->lr.bBringUp) {
    /* Polling stream0 online event for indication that the lrp streams are online */
    uEvent = 0;
    uTimeOut = 0;
    while ( !uEvent && (1500 > uTimeOut) ) {
      uEvent=SAND_HAL_GET_FIELD(C2, LR_EVENT, ONLINE0, SAND_HAL_READ(userDeviceHandle, C2, LR_EVENT));
      uTimeOut++;
    }
    if ( uTimeOut == 1500 ) {
      SB_ERROR("Timeout waiting for ONLINE0 event from LRP.\n");
    }
  }

  /* dgm - Oct 11 2006 - Read and clear any error bits from bringup, then unmask the error registers */
  uError = SAND_HAL_READ(userDeviceHandle, C2, LR_EVENT);
  SAND_HAL_WRITE(userDeviceHandle, C2,  LR_EVENT, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  LR_EVENT_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, C2, LU_UCODE_EVENT0);
  SAND_HAL_WRITE(userDeviceHandle, C2,  LU_UCODE_EVENT0, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  LU_UCODE_EVENT0_MASK, 0);

  uError = SAND_HAL_READ(userDeviceHandle, C2, LU_UCODE_EVENT1);
  SAND_HAL_WRITE(userDeviceHandle, C2,  LU_UCODE_EVENT1, uError);
  SAND_HAL_WRITE(userDeviceHandle, C2,  LU_UCODE_EVENT1_MASK, 0);

  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LR_GROUP0_GPR0_DEBUG, DISABLE_PARITY_ERROR, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LR_GROUP0_GPR1_DEBUG, DISABLE_PARITY_ERROR, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LR_GROUP1_GPR0_DEBUG, DISABLE_PARITY_ERROR, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LR_GROUP1_GPR1_DEBUG, DISABLE_PARITY_ERROR, 0x1);
  /* dgm - Feb 19 2008 - Clear out un-correctable error registers in lrp */
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LR_GROUP0_GPR0_DEBUG, PARITY_ERROR, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LR_GROUP0_GPR1_DEBUG, PARITY_ERROR, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LR_GROUP0_HDR0_DEBUG, PARITY_ERROR, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LR_GROUP0_HDR1_DEBUG, PARITY_ERROR, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LR_GROUP1_GPR0_DEBUG, PARITY_ERROR, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LR_GROUP1_GPR1_DEBUG, PARITY_ERROR, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LR_GROUP1_HDR0_DEBUG, PARITY_ERROR, 0x1);
  SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LR_GROUP1_HDR1_DEBUG, PARITY_ERROR, 0x1);

  DEXIT();
}

static
void sbFe2000XtInitCleanUpAgm(sbhandle userDeviceHandle, uint16_t nNum)
{
  uint16_t nPort;
  uint32_t uData;
  DENTER();

  /* Clear any link status events due to auto-neg */
  uData = SAND_HAL_READ_STRIDE(userDeviceHandle, C2,AG,nNum,AG_LINK_STATUS_EVENT);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2,AG,nNum,AG_LINK_STATUS_EVENT,uData);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2,AG,nNum,AG_LINK_STATUS_EVENT_MASK,0x0);

  /* Clear any signal detect events */
  uData = SAND_HAL_READ_STRIDE(userDeviceHandle, C2,AG,nNum,AG_SIGNAL_DETECT_EVENT);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2,AG,nNum,AG_SIGNAL_DETECT_EVENT,uData);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle, C2,AG,nNum,AG_SIGNAL_DETECT_EVENT_MASK,0x0);

  for ( nPort = 0; nPort < SB_FE2000_MAX_AG_PORTS; nPort++ ) {

    sbFe2000XtInitCleanUpAgmPortRx(nNum, nPort,userDeviceHandle);
    sbFe2000XtInitCleanUpAgmPortTx(nNum, nPort, userDeviceHandle);

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
void sbFe2000XtInitCleanUpAgmPortRx(uint16_t nNum, uint16_t nPort, sbhandle userDeviceHandle)
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
void sbFe2000XtInitCleanUpAgmPortTx(uint16_t nNum, uint16_t nPort, sbhandle userDeviceHandle)
{
  DENTER();
  /* kpl currently this routine appears to do nothing, be need updates for real hw */
#ifdef DEBUG_PRINT
  SB_LOGV1("%s Agm%d, port:%d\n",__PRETTY_FUNCTION__,nNum,nPort);
#endif
  DEXIT();
}

static
void sbFe2000XtInitCleanUpXgm(sbhandle userDeviceHandle, uint16_t nNum)
{
  DENTER();
  if ( nNum == 0 ) {
  } else if ( nNum == 1 ) {
  } else if ( nNum == 2 ) {
  } else if ( nNum == 3 ) {
  } else {
    SB_ASSERT(0);
  }
  DEXIT();
}

static
void sbFe2000XtInitCleanUpSpi4(sbhandle userDeviceHandle, uint16_t nNum)
{
  DENTER();
  DEXIT();
}

static uint32_t
sbFe2000XtInitPsyncSlipBuffers(sbhandle userDeviceHandle,
                sbFe2000XtInitParams_t *pInitParams)
{
    DENTER();

    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_PR_SLIP_BUFFER_DEBUG, FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PC_PT_SLIP_BUFFER_DEBUG, FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, SR0_SLIP_BUFFER_DEBUG_PR, FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, SR1_SLIP_BUFFER_DEBUG_PR, FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, SR0_SLIP_BUFFER_DEBUG_QM, FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, SR1_SLIP_BUFFER_DEBUG_QM, FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, ST0_SLIP_BUFFER_DEBUG_PT, FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, ST1_SLIP_BUFFER_DEBUG_PT, FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_SLIP_BUFFER_DEBUG_SR0, FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_SLIP_BUFFER_DEBUG_SR1, FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PR_SLIP_BUFFER_DEBUG_PC, FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_SLIP_BUFFER_DEBUG_ST0, FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_SLIP_BUFFER_DEBUG_ST1, FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_SLIP_BUFFER_DEBUG_PC, FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PM_LR_SLIP_BUFFER_DEBUG, FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PM_MM0_SLIP_BUFFER_DEBUG, FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PM_MM1_SLIP_BUFFER_DEBUG, FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LU_SLIP_BUFFER_DEBUG_PM, H_FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LU_SLIP_BUFFER_DEBUG_EXT_MM0, EXT_H_FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LU_SLIP_BUFFER_DEBUG_EXT_MM1, EXT_H_FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LU_SLIP_BUFFER_DEBUG_INT_MM0, INT_H_FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LU_SLIP_BUFFER_DEBUG_INT_MM1, INT_H_FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LU_SLIP_BUFFER_DEBUG_CM, H_FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, CM_SLIP_BUFFER_DEBUG_MM0, H_FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, CM_SLIP_BUFFER_DEBUG_MM1, H_FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, CM_SLIP_BUFFER_DEBUG_LR, H_FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, MM0_SLIP_BUFFER_DEBUG_PM, FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, MM1_SLIP_BUFFER_DEBUG_PM, FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, MM0_SLIP_BUFFER_DEBUG_CM, FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, MM1_SLIP_BUFFER_DEBUG_CM, FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, MM0_SLIP_BUFFER_DEBUG_LR_EXT, FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, MM1_SLIP_BUFFER_DEBUG_LR_EXT, FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, MM0_SLIP_BUFFER_DEBUG_LR_INT, FIVE_DEEP_BUFFER, 0x1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, MM1_SLIP_BUFFER_DEBUG_LR_INT, FIVE_DEEP_BUFFER, 0x1);

    DEXIT();

    return SB_FE2000_STS_INIT_OK_K;
}

static uint32_t
sbFe2000XtInitPLL(sbhandle userDeviceHandle,
                sbFe2000XtInitParams_t *pInitParams)
{
    uint32_t uData;
    uint32_t bLocked;
    uint32_t uIndex;
    uint32_t uMDivSws, uMDivHpp;
    uint32_t uNDivSws, uNDivHpp;
    uint32_t uVRngSws, uVRngHpp;

    DENTER();
    
    /* HPP PLL */
    if ( pInitParams->uHppFreq == SB_FE2000XT_HPP_FREQ_400000) {
        uMDivHpp = 4;
        uNDivHpp = 64;
        uVRngHpp = 0;
    } else if ( pInitParams->uHppFreq == SB_FE2000XT_HPP_FREQ_375000) {
        uMDivHpp = 4;
        uNDivHpp = 60;
        uVRngHpp = 0;
    } else if ( pInitParams->uHppFreq == SB_FE2000XT_HPP_FREQ_300000) {
        uMDivHpp = 5;
        uNDivHpp = 60;
        uVRngHpp = 0;
    } else if ( pInitParams->uHppFreq == SB_FE2000XT_HPP_FREQ_262500) {
        uMDivHpp = 6;
        uNDivHpp = 63;
        uVRngHpp = 0;
    } else if ( pInitParams->uHppFreq == SB_FE2000XT_HPP_FREQ_258333) {
        uMDivHpp = 6;
        uNDivHpp = 62;
        uVRngHpp = 0;
    } else if ( pInitParams->uHppFreq == SB_FE2000XT_HPP_FREQ_250000) {
        uMDivHpp = 6;
        uNDivHpp = 60;
        uVRngHpp = 0;
    } else {
        uMDivHpp = 0;
        uNDivHpp = 0;
        uVRngHpp = 0;
        SB_ASSERT(0);
    }
    
    /* SWS PLL */
    if ( pInitParams->uSwsFreq == SB_FE2000XT_SWS_FREQ_360000) {
        uMDivSws = 5;
        uNDivSws = 72;
        uVRngSws = 1;
    } else if ( pInitParams->uSwsFreq == SB_FE2000XT_SWS_FREQ_340000) {
        uMDivSws = 5;
        uNDivSws = 68;
        uVRngSws = 1;
    } else if ( pInitParams->uSwsFreq == SB_FE2000XT_SWS_FREQ_295000) {
        uMDivSws = 5;
        uNDivSws = 59;
        uVRngSws = 0;
    } else if ( pInitParams->uSwsFreq == SB_FE2000XT_SWS_FREQ_275000) {
        uMDivSws = 5;
        uNDivSws = 55;
        uVRngSws = 0;
    } else if ( pInitParams->uSwsFreq == SB_FE2000XT_SWS_FREQ_254167) {
        uMDivSws = 6;
        uNDivSws = 61;
        uVRngSws = 0;
    } else if ( pInitParams->uSwsFreq == SB_FE2000XT_SWS_FREQ_245000) {
        uMDivSws = 5;
        uNDivSws = 49;
        uVRngSws = 0;
    } else {
        uMDivSws = 0;
        uNDivSws = 0;
        uVRngSws = 0;
        SB_ASSERT(0);
    }
    
    /* core PLL */
    uData = SAND_HAL_READ(userDeviceHandle, C2, PC_PLLC_CONFIG0);
    uData = SAND_HAL_MOD_FIELD(C2, PC_PLLC_CONFIG0, A_RESET, uData, 1);
    uData = SAND_HAL_MOD_FIELD(C2, PC_PLLC_CONFIG0, D_RESET, uData, 1);
    uData = SAND_HAL_MOD_FIELD(C2, PC_PLLC_CONFIG0, MDIV, uData, uMDivSws);
    uData = SAND_HAL_MOD_FIELD(C2, PC_PLLC_CONFIG0, NDIV, uData, uNDivSws);
    uData = SAND_HAL_MOD_FIELD(C2, PC_PLLC_CONFIG0, VCO_RNG, uData, uVRngSws);
    SAND_HAL_WRITE(userDeviceHandle, C2, PC_PLLC_CONFIG0, uData);
    thin_delay(2000);
    uData = SAND_HAL_MOD_FIELD(C2, PC_PLLC_CONFIG0, A_RESET, uData, 0);
    SAND_HAL_WRITE(userDeviceHandle, C2, PC_PLLC_CONFIG0, uData);

    if (!pInitParams->bSimulation) {
        bLocked = 0;
        for( uIndex = 0; uIndex < 200; uIndex++ ) {
            thin_delay(100);
            uData = SAND_HAL_READ(userDeviceHandle, C2, PC_PLLC_CONFIG0);
            bLocked = SAND_HAL_GET_FIELD(C2, PC_PLLC_CONFIG0, LOCK, uData);
            if( bLocked ) {
                break;
            }
        }
        uData = SAND_HAL_READ(userDeviceHandle, C2, PC_PLLC_CONFIG0);
        bLocked = SAND_HAL_GET_FIELD(C2, PC_PLLC_CONFIG0, LOCK, uData);
        if( !bLocked ) {
            return -1;
        }
    }

    uData = SAND_HAL_MOD_FIELD(C2, PC_PLLC_CONFIG0, D_RESET, uData, 0);
    SAND_HAL_WRITE(userDeviceHandle, C2, PC_PLLC_CONFIG0, uData);

    /* HPP PLL */
    uData = SAND_HAL_READ(userDeviceHandle, C2, PC_PLLH_CONFIG0);
    uData = SAND_HAL_MOD_FIELD(C2, PC_PLLH_CONFIG0, A_RESET, uData, 1);
    uData = SAND_HAL_MOD_FIELD(C2, PC_PLLH_CONFIG0, D_RESET, uData, 1);
    uData = SAND_HAL_MOD_FIELD(C2, PC_PLLH_CONFIG0, M1DIV, uData, uMDivHpp);
    uData = SAND_HAL_MOD_FIELD(C2, PC_PLLH_CONFIG0, NDIV, uData, uNDivHpp);
    uData = SAND_HAL_MOD_FIELD(C2, PC_PLLH_CONFIG0, VCO_RNG, uData, uVRngHpp);
    SAND_HAL_WRITE(userDeviceHandle, C2, PC_PLLH_CONFIG0, uData);
    thin_delay(2000);
    uData = SAND_HAL_MOD_FIELD(C2, PC_PLLH_CONFIG0, A_RESET, uData, 0);
    SAND_HAL_WRITE(userDeviceHandle, C2, PC_PLLH_CONFIG0, uData);

    if (!pInitParams->bSimulation) {
        bLocked = 0;
        for( uIndex = 0; uIndex < 200; uIndex++ ) {
            thin_delay(100);
            uData = SAND_HAL_READ(userDeviceHandle, C2, PC_PLLH_CONFIG0);
            bLocked = SAND_HAL_GET_FIELD(C2, PC_PLLH_CONFIG0, LOCK, uData);
            if( bLocked ) {
                break;
            }
        }
        uData = SAND_HAL_READ(userDeviceHandle, C2, PC_PLLH_CONFIG0);
        bLocked = SAND_HAL_GET_FIELD(C2, PC_PLLH_CONFIG0, LOCK, uData);
        if( !bLocked ) {
            return -1;
        }
    }

    uData = SAND_HAL_MOD_FIELD(C2, PC_PLLH_CONFIG0, D_RESET, uData, 0);
    SAND_HAL_WRITE(userDeviceHandle, C2, PC_PLLH_CONFIG0, uData);

    /* SPI Frequency and PLL handled in sbFe2000XtInitSpiTx */

    /* Lc PLL */
    /* Reset LCPLL -- Supplies 156MHz reference clock to all 1Gx8 and 16G serdes */
    uData = SAND_HAL_READ(userDeviceHandle, C2, XG0_XGXS_PLL_CONTROL4);
    uData = SAND_HAL_MOD_FIELD(C2, XG0_XGXS_PLL_CONTROL4, RESET, uData, 0x1);
    SAND_HAL_WRITE(userDeviceHandle, C2, XG0_XGXS_PLL_CONTROL4, uData);
    thin_delay(100);
    uData = SAND_HAL_MOD_FIELD(C2, XG0_XGXS_PLL_CONTROL4, RESET, uData, 0x0);
    SAND_HAL_WRITE(userDeviceHandle, C2, XG0_XGXS_PLL_CONTROL4, uData);

    DEXIT();

    return SB_FE2000_STS_INIT_OK_K;
}

sbStatus_t
sbFe2000XtInitClearMems(sbhandle userDeviceHandle,
                      sbFe2000XtInitParams_t *pInitParams)
{
    uint32_t v, v0;
    int i;

    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CLI_ADDR0, 0);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CLI_ADDR1, 0);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CLI_ADDR2, 0);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CLI_ADDR3, 0);

    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_PCI_ADDR0, 0);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_PCI_ADDR1, 0);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_PCI_ADDR2, 0);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_PCI_ADDR3, 0);

    v0 = 0;
    v0 = SAND_HAL_MOD_FIELD(C2, PC_DMA_CTRL0, ACK, v0, 1);
    v0 = SAND_HAL_MOD_FIELD(C2, PC_DMA_CTRL0, REQ, v0, 1);
    v0 = SAND_HAL_MOD_FIELD(C2, PC_DMA_CTRL0, FIXED_PTRN, v0, 1);

    /* Clear the internal RAMs */
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_COUNT0, 32 * 1024);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_COUNT1, 32 * 1024);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_COUNT2, 32 * 1024);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_COUNT3, 32 * 1024);

    v = SAND_HAL_MOD_FIELD(C2, PC_DMA_CTRL0, DMA_SRC, v0,
                           SB_FE2000_PC_DMA_SRC_MM0_INT0);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CTRL0, v);
    v = SAND_HAL_MOD_FIELD(C2, PC_DMA_CTRL0, DMA_SRC, v0,
                           SB_FE2000_PC_DMA_SRC_MM0_INT1);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CTRL1, v);
    v = SAND_HAL_MOD_FIELD(C2, PC_DMA_CTRL0, DMA_SRC, v0,
                           SB_FE2000_PC_DMA_SRC_MM1_INT0);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CTRL2, v);
    v = SAND_HAL_MOD_FIELD(C2, PC_DMA_CTRL0, DMA_SRC, v0,
                           SB_FE2000_PC_DMA_SRC_MM1_INT1);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CTRL3, v);

    for (i = 0; i < 1000; i++) {
        thin_delay(100000);
        if (SAND_HAL_READ(userDeviceHandle, C2, PC_DMA_CTRL0)
            & SAND_HAL_READ(userDeviceHandle, C2, PC_DMA_CTRL1)
            & SAND_HAL_READ(userDeviceHandle, C2, PC_DMA_CTRL2)
            & SAND_HAL_READ(userDeviceHandle, C2, PC_DMA_CTRL3)
            & SAND_HAL_C2_PC_DMA_CTRL0_ACK_MASK) {
            break;
        }
    }
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CTRL0,
                   SAND_HAL_C2_PC_DMA_CTRL0_ACK_MASK);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CTRL1,
                   SAND_HAL_C2_PC_DMA_CTRL0_ACK_MASK);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CTRL2,
                   SAND_HAL_C2_PC_DMA_CTRL0_ACK_MASK);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CTRL3,
                   SAND_HAL_C2_PC_DMA_CTRL0_ACK_MASK);
    if (i == 1000) {
        return SB_FE_MEM_ACC_TIMEOUT_ERR_CODE;
    }

    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_COUNT0,
                   pInitParams->mm[0].ramconfiguration.NarrowPortRam0Words);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_COUNT1,
                   pInitParams->mm[0].ramconfiguration.NarrowPortRam1Words);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_COUNT2,
                   pInitParams->mm[1].ramconfiguration.NarrowPortRam0Words);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_COUNT3,
                   pInitParams->mm[1].ramconfiguration.NarrowPortRam1Words);

    v = SAND_HAL_MOD_FIELD(C2, PC_DMA_CTRL0, DMA_SRC, v0,
                           SB_FE2000_PC_DMA_SRC_MM0_NARROW0);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CTRL0, v);
    v = SAND_HAL_MOD_FIELD(C2, PC_DMA_CTRL0, DMA_SRC, v0,
                           SB_FE2000_PC_DMA_SRC_MM0_NARROW1);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CTRL1, v);
    v = SAND_HAL_MOD_FIELD(C2, PC_DMA_CTRL0, DMA_SRC, v0,
                           SB_FE2000_PC_DMA_SRC_MM1_NARROW0);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CTRL2, v);
    v = SAND_HAL_MOD_FIELD(C2, PC_DMA_CTRL0, DMA_SRC, v0,
                           SB_FE2000_PC_DMA_SRC_MM1_NARROW1);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CTRL3, v);

    for (i = 0; i < 100000; i++) {
        thin_delay(10000);
        if (SAND_HAL_READ(userDeviceHandle, C2, PC_DMA_CTRL0)
            & SAND_HAL_READ(userDeviceHandle, C2, PC_DMA_CTRL1)
            & SAND_HAL_READ(userDeviceHandle, C2, PC_DMA_CTRL2)
            & SAND_HAL_READ(userDeviceHandle, C2, PC_DMA_CTRL3)
            & SAND_HAL_C2_PC_DMA_CTRL0_ACK_MASK) {
            break;
        }
    }
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CTRL0,
                   SAND_HAL_C2_PC_DMA_CTRL0_ACK_MASK);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CTRL1,
                   SAND_HAL_C2_PC_DMA_CTRL0_ACK_MASK);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CTRL2,
                   SAND_HAL_C2_PC_DMA_CTRL0_ACK_MASK);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CTRL3,
                   SAND_HAL_C2_PC_DMA_CTRL0_ACK_MASK);
    if (i == 10000) {
        return SB_FE_MEM_ACC_TIMEOUT_ERR_CODE;
    }

    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_COUNT0,
                   pInitParams->mm[0].ramconfiguration.WidePortRamWords * 2);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_COUNT1,
                   pInitParams->mm[1].ramconfiguration.WidePortRamWords * 2);

    v0 = SAND_HAL_MOD_FIELD(C2, PC_DMA_CTRL0, DMA_SRC_TYPE, v0, 1);
    v = SAND_HAL_MOD_FIELD(C2, PC_DMA_CTRL0, DMA_SRC, v0,
                           SB_FE2000_PC_DMA_SRC_MM0_WIDE);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CTRL0, v);
    v = SAND_HAL_MOD_FIELD(C2, PC_DMA_CTRL0, DMA_SRC, v0,
                           SB_FE2000_PC_DMA_SRC_MM1_WIDE);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CTRL1, v);

    for (i = 0; i < 100000; i++) {
        thin_delay(10000);
        if (SAND_HAL_READ(userDeviceHandle, C2, PC_DMA_CTRL0)
            & SAND_HAL_READ(userDeviceHandle, C2, PC_DMA_CTRL1)
            & SAND_HAL_C2_PC_DMA_CTRL0_ACK_MASK) {
            break;
        }
    }
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CTRL0,
                   SAND_HAL_C2_PC_DMA_CTRL0_ACK_MASK);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CTRL1,
                   SAND_HAL_C2_PC_DMA_CTRL0_ACK_MASK);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CTRL2,
                   SAND_HAL_C2_PC_DMA_CTRL0_ACK_MASK);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PC_DMA_CTRL3,
                   SAND_HAL_C2_PC_DMA_CTRL0_ACK_MASK);

    if (i == 10000) {
        return SB_FE_MEM_ACC_TIMEOUT_ERR_CODE;
    }

    return SB_FE2000_STS_INIT_OK_K;
}


void sbFe2000XtStartBubbleTimer(uint32_t unit, uint32_t startBubble, uint32_t size)
{
  /* Configure the lrp to insert bubbles on the ingress task of stream 0 */
  uint32_t uBubbleStream, uRingData;
  int uAddress, nData;

  /* vn-0909101655 cmodel has bug in the bubble gen implementation 
     and to fix crash of the bcmsim on start up we bail out of this function*/
  /* vn-0909101655+*/ if (SAL_BOOT_BCMSIM) {
  /* vn-0909101655+*/   SOC_DEBUG_PRINT((DK_VERBOSE, "Skipping %s for sbx bcmsim initialization\n", FUNCTION_NAME()));
  /* vn-0909101655+*/   return;
  /* vn-0909101655+*/ }

  SB_ASSERT(SOC_IS_SBX_FE2KXT(unit));

  /* init List manager on service processor - port 3
   * 8K elements, stride =1, enable ring
   * LIST_CONFIG_B* must be configured before LIST_CONFIG_A*
   */

  uRingData = 0;
  uRingData = SAND_HAL_MOD_FIELD( C2, LU_LIST_CONFIG_B1, BASE, uRingData, 0x0 );
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, LU_LIST_CONFIG_B1, uRingData);

  uRingData = 0;
  uRingData = SAND_HAL_MOD_FIELD( C2, LU_LIST_CONFIG_A1, UPDATE, uRingData, 0x1 );
  uRingData = SAND_HAL_MOD_FIELD( C2, LU_LIST_CONFIG_A1, RING,   uRingData, 0x1 );
  uRingData = SAND_HAL_MOD_FIELD( C2, LU_LIST_CONFIG_A1, STRIDE, uRingData, 0x1 );
  uRingData = SAND_HAL_MOD_FIELD( C2, LU_LIST_CONFIG_A1, LENGTH, uRingData, size-1);
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, LU_LIST_CONFIG_A1, uRingData);


  /* clear the bubble packet */
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_DATA0, 0);              
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_DATA1, 0);              
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_DATA2, 0);              
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_DATA3, 0);              
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_DATA4, 0);              
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_DATA5, 0);              
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_DATA6, 0);              
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_DATA7, 0);              
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_DATA8, 0);              
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_DATA9, 0);              
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_DATA10, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_DATA11, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_DATA12, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_DATA13, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_DATA14, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_DATA15, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_DATA16, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_DATA17, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_DATA18, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_DATA19, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_DATA20, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_DATA21, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_DATA22, 0);            
  SAND_HAL_WRITE(SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_DATA23, 0);            
                                 
  uAddress = (SB_FE2000XT_QM_OAM_SRC_BUFFER) * 2; /* (0xFFE) * 2 = 0x1FFC */  
                  
  SAND_HAL_WRITE( SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_CTRL,                       
                 SAND_HAL_SET_FIELD(C2, PB0_MEM_ACC_CTRL, REQ, 1) |           
                 SAND_HAL_SET_FIELD(C2, PB0_MEM_ACC_CTRL, ACK, 1) |           
                 SAND_HAL_SET_FIELD(C2, PB0_MEM_ACC_CTRL, RD_WR_N, 0) |       
                 SAND_HAL_SET_FIELD(C2, PB0_MEM_ACC_CTRL, ADDRESS, uAddress)  
                 );                                                           
                                                                            
  nData = SAND_HAL_READ( SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_CTRL);     
  nData = SAND_HAL_MOD_FIELD(C2, PB0_MEM_ACC_CTRL, REQ, nData, 0);      
  nData = SAND_HAL_MOD_FIELD(C2, PB0_MEM_ACC_CTRL, ACK, nData, 1);      
  SAND_HAL_WRITE( SOC_SBX_SBHANDLE(unit), C2, PB0_MEM_ACC_CTRL, nData);          


  /* init bubble timer on service processor - port 7 */

  /* FE2K-XT */
  /* all bubble types are dropped into stream 6. */
  uBubbleStream = SAND_HAL_SET_FIELD(C2, LU_BUBBLE_STREAM, TYPE0, 6) |
                  SAND_HAL_SET_FIELD(C2, LU_BUBBLE_STREAM, TYPE1, 6) |
                  SAND_HAL_SET_FIELD(C2, LU_BUBBLE_STREAM, TYPE2, 6) |
                  SAND_HAL_SET_FIELD(C2, LU_BUBBLE_STREAM, TYPE3, 6) |
                  SAND_HAL_SET_FIELD(C2, LU_BUBBLE_STREAM, TYPE4, 6) |
                  SAND_HAL_SET_FIELD(C2, LU_BUBBLE_STREAM, TYPE5, 6) |
                  SAND_HAL_SET_FIELD(C2, LU_BUBBLE_STREAM, TYPE6, 6) |
                  SAND_HAL_SET_FIELD(C2, LU_BUBBLE_STREAM, TYPE7, 6);
  SAND_HAL_WRITE( SOC_SBX_SBHANDLE(unit), C2, LU_BUBBLE_STREAM, uBubbleStream);

  /* Bubble message will simulate ppe message with header types/locn f's */
  SAND_HAL_WRITE( SOC_SBX_SBHANDLE(unit), C2, LU_BUBBLE_HEADER0, 0xffffffff);
  SAND_HAL_WRITE( SOC_SBX_SBHANDLE(unit), C2, LU_BUBBLE_HEADER1, 0xffffffff);

  /* start the OAM bubble.  REQ3 is for port 7.  startBubbble is bool: 0=off, 1=on */
  SAND_HAL_RMW_FIELD( SOC_SBX_SBHANDLE(unit), C2, LU_BUBBLE, REQ3, startBubble);


  /* init the Real-Time Clock */
  SAND_HAL_WRITE( SOC_SBX_SBHANDLE(unit), C2, LU_RTC_NS_INC, 1);
  SAND_HAL_WRITE( SOC_SBX_SBHANDLE(unit), C2, LU_RTC_SUB_NS_INC, 0xC0000000);
  SAND_HAL_WRITE( SOC_SBX_SBHANDLE(unit), C2, LU_RTC_NS_TO_S_MARK, 1000000000);

}


sbStatus_t
sbFe2000XtInitQueues(sbhandle userDeviceHandle,
                   sbFe2000Queues_t *queues)
{
    uint32_t fromIface, fromPort, toIface, toPort, fromQid, toQid;
    sbZfC2PtQueueToPortEntry_t q2p;
    sbZfC2QmQueueConfigEntry_t qc;
    sbFe2000Connection_t *c = NULL;
    sbZfC2QmQueueState0EnEntry_t qe;
    uint32_t pb0PagesAvail, pb1PagesAvail, v, qmConfig0;
    int i, iface, many2one, one2many, pbsel, egress, port;
    uint32_t mirrorData, uPtConfig, uPtData;
    uint32_t unit = UNIT_FROM_HANDLE(userDeviceHandle);
    sbFe2000XtInitParams_t *feip;
    int e[SB_FE2000_IF_PTYPE_MAX];

    SB_ASSERT(queues);
    sbZfC2PtQueueToPortEntry_InitInstance(&q2p);
    sbZfC2QmQueueConfigEntry_InitInstance(&qc);
    sbZfC2QmQueueState0EnEntry_InitInstance(&qe);

    qc.m_uEnforcePoliceMarkings = 0;
    qc.m_uDropThresh2 = 0xfff;
    qc.m_uDropThresh1 = 0;
    qc.m_uEnable = 1;

    qe.m_uEnable = 1;

    qmConfig0 = SAND_HAL_READ(userDeviceHandle, C2, QM_CONFIG0);
    pb0PagesAvail = SAND_HAL_GET_FIELD(C2, QM_CONFIG0,
                                       TOTAL_MAX_PAGES_AVAILABLE_PB0,
                                       qmConfig0);
    pb1PagesAvail = SAND_HAL_GET_FIELD(C2, QM_CONFIG0,
                                       TOTAL_MAX_PAGES_AVAILABLE_PB1,
                                       qmConfig0);

    /* Expanded Mode Configuration */
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, PT_CONFIG1, EXPANDED_FIFO_SIZE,
                       SB_FE2000XT_XGM_EXPANDED_FIFO_SIZE);

    for (i = 0; i < SB_FE2000_IF_PTYPE_MAX; i++){
        e[i]=0;
    }

    for (i = 0; i < queues->n; i++) {
        c = &queues->connections[i];
        egress = c->bEgress;
        fromIface = c->from.ulInterfacePortType;
        fromPort = c->from.ulPortId;
        toIface = c->to.ulInterfacePortType;
        toPort = c->to.ulPortId;
        many2one = queues->many2onecon[i] != SB_FE2000_SWS_UNINITIALIZED_CON;
        one2many = queues->one2manycon[i] != SB_FE2000_SWS_UNINITIALIZED_CON;
        pbsel = queues->pbsel[i];

        fromQid = queues->fromqid[i];
        toQid = queues->toqid[i];

        /* set up default queue map in congestion control block */
        if (!egress) {
            switch (fromIface) {
            case SB_FE2000_IF_PTYPE_AGM0:
                if (sbFe2000XtImfDriver_PrCc2QueueRamWrite((uint32_t)userDeviceHandle,
                                                            fromPort,
                                                            fromQid)){
                    return SB_FAILED;
                }
                break;
            case SB_FE2000_IF_PTYPE_AGM1:
                if (sbFe2000XtImfDriver_PrCc3QueueRamWrite((uint32_t)userDeviceHandle,
                                                            fromPort,
                                                            fromQid)){
                    return SB_FAILED;
                }
                break;
            case SB_FE2000_IF_PTYPE_XGM0:
                if (sbFe2000XtImfDriver_PrCc4QueueRamWrite((uint32_t)userDeviceHandle,
                                                            fromPort,
                                                            fromQid)){
                    return SB_FAILED;
                }
                break;
            case SB_FE2000_IF_PTYPE_XGM1:
                if (sbFe2000XtImfDriver_PrCc5QueueRamWrite((uint32_t)userDeviceHandle,
                                                            fromPort,
                                                            fromQid)){
                    return SB_FAILED;
                }
                break;
            }
        }

        if (queues->ifexpmode[fromIface] || queues->ifexpmode[toIface]) {
            uPtConfig = SAND_HAL_READ(userDeviceHandle, C2, PT_CONFIG1);
            uPtData = SAND_HAL_GET_FIELD(C2, PT_CONFIG1, EXPANDED_FIFO_ENABLE,
                                         uPtConfig);
            uPtData |= ((1 << (fromQid / SB_FE2000XT_SWS_QUEUES_PER_GROUP))
                        | (1 << (toQid / SB_FE2000XT_SWS_QUEUES_PER_GROUP)));
            uPtConfig = SAND_HAL_MOD_FIELD(C2, PT_CONFIG1,
                                           EXPANDED_FIFO_ENABLE, uPtConfig,
                                           uPtData);
            SAND_HAL_WRITE(userDeviceHandle, C2,  PT_CONFIG1, uPtConfig);
        }

        /* Configure the *->HPP queue */
        qc.m_uFlowControlTarget = SB_FE2000_QM_FLOW_CONTROL_TARGET_PORT;
        /* For HiGig interfaces, "channelize" backpressure for E2ECC */
        if (SOC_SBX_CFG_FE2000_IF_IS_HIGIG(unit, fromIface)) {
            qc.m_uFlowControlPort = (fromIface << 6) + toPort;
        }else{
            qc.m_uFlowControlPort = (fromIface << 6) + fromPort;
        }
        qc.m_uFlowControlThresh1 = queues->fromflowthresh[i];
        qc.m_uFlowControlThresh2 = queues->fromflowthresh[i];
        qc.m_uFlowControlEnable = !!queues->fromflowthresh[i];
        qc.m_uMinPages = queues->minfrompages[i];
        qc.m_uMaxPages =  queues->maxfrompages[i];
        C2QmQueueConfigWrite(userDeviceHandle, fromQid, &qc);
        C2QmQueueState0EnWrite(userDeviceHandle, fromQid, &qe);
        q2p.m_uPbSel = pbsel;
        q2p.m_uPpeBound = 1;
        q2p.m_uDestInterface = toIface;
        q2p.m_uDestPort = toPort;
        q2p.m_uPpeEnqueue = egress && !many2one;
        q2p.m_uDestQueue = toQid;
        C2PtQueueToPortWrite(userDeviceHandle, fromQid, &q2p);
        C2PrPortToQueueWrite(userDeviceHandle, fromIface, fromPort, fromQid);
        if (pbsel) {
            pb1PagesAvail -= qc.m_uMinPages;
        } else {
            pb0PagesAvail -= qc.m_uMinPages;
        }

        if (!many2one) {
            uint32_t bp_reg_offset, bp_reg;
            /* Configure the HPP->* queue */
            qc.m_uFlowControlTarget = SB_FE2000_QM_FLOW_CONTROL_TARGET_QUEUE;
            qc.m_uFlowControlPort = toQid % (SB_FE2000XT_TO_QUEUE_OFFSET);
            bp_reg_offset = SAND_HAL_REG_OFFSET(C2, PT_BP_CONFIG0);

            /* vn-0909210817 fix of the error message "sal-non aligned address"
	       seems port address calculated wrong and it can not be aligned
	       because of that wrong calculation. For the error message fix
	       correct calculated required as described below:
	       line from the original code replaced*/
	    /* vn-0909210817- bp_reg_offset += qc.m_uFlowControlPort; */
	    /* vn-0909210817+*/bp_reg_offset += (qc.m_uFlowControlPort%4)<<2;

            bp_reg = SAND_HAL_READ_OFFS(userDeviceHandle, bp_reg_offset);
            bp_reg = bp_reg & (~(0xff << (8 * (qc.m_uFlowControlPort % 4))));
            bp_reg = bp_reg | (( 0x80 | fromQid) << (8 * (qc.m_uFlowControlPort % 4)));
            SAND_HAL_WRITE_OFFS(userDeviceHandle, bp_reg_offset, bp_reg);
            qc.m_uMinPages = queues->mintopages[i];
            qc.m_uMaxPages =  queues->maxtopages[i];
            qc.m_uFlowControlThresh1 = queues->toflowthresh[i];
            qc.m_uFlowControlThresh2 = queues->toflowthresh[i];
            qc.m_uFlowControlEnable = !!queues->toflowthresh[i];
            C2QmQueueConfigWrite(userDeviceHandle, toQid, &qc);
            C2QmQueueState0EnWrite(userDeviceHandle, toQid, &qe);
            q2p.m_uPbSel = pbsel;
            q2p.m_uPpeBound = 0;
            q2p.m_uPpeEnqueue = 0;
            q2p.m_uDestQueue = 0;
            C2PtQueueToPortWrite(userDeviceHandle, toQid, &q2p);
            C2PtPortToQueueWrite(userDeviceHandle, toIface, toPort, toQid);

            if (pbsel) {
                pb1PagesAvail -= qc.m_uMinPages;
            } else {
                pb0PagesAvail -= qc.m_uMinPages;
            }
        }
    }

    qmConfig0 = SAND_HAL_MOD_FIELD(C2, QM_CONFIG0,
                                   TOTAL_MAX_PAGES_AVAILABLE_PB0, qmConfig0,
                                   pb0PagesAvail);
    qmConfig0 = SAND_HAL_MOD_FIELD(C2, QM_CONFIG0,
                                   TOTAL_MAX_PAGES_AVAILABLE_PB1, qmConfig0,
                                   pb1PagesAvail);
    SAND_HAL_WRITE(userDeviceHandle, C2,  QM_CONFIG0, qmConfig0);

    /* configure queue mappings for higig interfaces */
    feip = &SOC_SBX_CFG_FE2000(unit)->xt_init_params;
    PBMP_ALL_ITER(unit, port) {
        if (IS_HG_PORT(unit, port)) {
            iface = sbFe2000TranslateBlock(SOC_PORT_BLOCK_TYPE(unit, port),
                                    SOC_PORT_BLOCK_NUMBER(unit, port));
            e[iface] = 0;
            for (i=0; i < feip->queues.n; i++) {
                c = &feip->queues.connections[i];
                if (c->from.ulInterfacePortType == iface){
                    if  (sbFe2000XtCcPort2QidMapSet(userDeviceHandle, iface, e[iface], c->ulUcodePort, feip->queues.fromqid[i])){
                        return SB_FAILED;
                    }
                    e[iface]++;
                }
            }
        }
    }
    for (iface = 0; iface < SB_FE2000_IF_PTYPE_MAX; iface++) {
        if (e[iface] > 0) {
                if  (sbFe2000XtCcPort2QidMapSet(userDeviceHandle, iface, e[iface], -1, -1)){
                    return SB_FAILED;
                }
        }
    }

    /* Miscellaneous PP & PD config */

    v = SAND_HAL_READ(userDeviceHandle, C2, PD_CONFIG);

    v = SAND_HAL_MOD_FIELD(C2,PD_CONFIG, TRUNC_LENGTH, v, 12 + 64);

    v = SAND_HAL_MOD_FIELD(C2,PD_CONFIG, CONDITIONAL_UPDATE, v, 1);
    v = SAND_HAL_MOD_FIELD(C2,PD_CONFIG, FRAME_LEN_ADJUST, v, 1);
    SAND_HAL_WRITE(userDeviceHandle, C2,  PD_CONFIG, v);

    v = SAND_HAL_READ(userDeviceHandle, C2, PD_DEBUG);
    v = SAND_HAL_MOD_FIELD(C2,PD_DEBUG,IPV4_HDR_TYPE, v, 0x7);
    SAND_HAL_WRITE(userDeviceHandle, C2, PD_DEBUG, v);

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
                              SAND_HAL_C2_PT_MIRROR_INDEX_MEM_ACC_CTRL_OFFSET,
                              1, 1, &mirrorData)){
        return SB_FAILED;
    }
    mirrorData = queues->mir1qid ? queues->mir1qid : queues->mir0qid;
    mirrorData |= 0x0300;
    if(sbFe2000UtilWriteIndir(userDeviceHandle, FALSE, TRUE,
                              SAND_HAL_C2_PT_MIRROR_INDEX_MEM_ACC_CTRL_OFFSET,
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
sbFe2000XtSetQueueParams(int unit, sbFe2000XtInitParams_t *ip)
{
    int i, j, k, maxcal, egress, toIface, toPort, fromIface, fromPort;
    int totalports, iface, port, pbsel, owe, con, con0;
    int freequeues, qid, lines, ports, group, queuesused, bwused;
    int portspergroup, groups;
    int portbw = 0;
    int ifbw[SB_FE2000_IF_PTYPE_MAX][2];
    int maxports[SB_FE2000_IF_PTYPE_MAX];
    int pbcnt[SB_FE2000_IF_PTYPE_MAX][2];
    int queuesbygroup[SB_FE2000XT_SWS_INGRESS_QUEUE_GROUPS];
    int bwbygroup[SB_FE2000XT_SWS_INGRESS_QUEUE_GROUPS];
    int spirlines[2], spitlines[2];
    sbFe2000Connection_t *c, *c0;
    sbFe2000Queues_t *queues = &ip->queues;
    pbmp_t xe_pbm;
    int xg, hg;
#if FE2K_SPI_PORTS_TO_MAX_CONTIGOUS
    int maxSr[2], maxSt[2];
#endif

    /* determine whether to bringup AG0,1 or XG2,3 */
    SOC_PBMP_ASSIGN(xe_pbm, PBMP_XE_ALL(unit));
    SOC_PBMP_OR(xe_pbm, PBMP_HG_ALL(unit));

    maxports[SB_FE2000_IF_PTYPE_SPI0] = SB_FE2000_MAX_PORTS_PER_SPI;
    maxports[SB_FE2000_IF_PTYPE_SPI1] = SB_FE2000_MAX_PORTS_PER_SPI;

    xg = SOC_SBX_CFG_FE2000_IF_IS_XG(unit, SB_FE2000_IF_PTYPE_AGM0);
    hg = SOC_SBX_CFG_FE2000_IF_IS_HIGIG(unit, SB_FE2000_IF_PTYPE_AGM0);
    /* port is either xg or hg */
    if (hg && xg) {
        SB_ASSERT(0);
    }
    if (xg) {
        maxports[SB_FE2000_IF_PTYPE_AGM0] = SB_FE2000_MAX_XG_PORTS;
    }else if (hg) {
        maxports[SB_FE2000_IF_PTYPE_AGM0] = SB_FE2000_MAX_HG_SUBPORTS;
    }else{
        maxports[SB_FE2000_IF_PTYPE_AGM0] = SB_FE2000_MAX_AG_PORTS;
    }

    xg = SOC_SBX_CFG_FE2000_IF_IS_XG(unit, SB_FE2000_IF_PTYPE_AGM1);
    hg = SOC_SBX_CFG_FE2000_IF_IS_HIGIG(unit, SB_FE2000_IF_PTYPE_AGM1);
    /* port is either xg or hg */
    if (hg && xg) {
        SB_ASSERT(0);
    }
    if (xg) {
        maxports[SB_FE2000_IF_PTYPE_AGM1] = SB_FE2000_MAX_XG_PORTS;
    }else if (hg) {
        maxports[SB_FE2000_IF_PTYPE_AGM1] = SB_FE2000_MAX_HG_SUBPORTS;
    }else{
        maxports[SB_FE2000_IF_PTYPE_AGM1] = SB_FE2000_MAX_AG_PORTS;
    }

    maxports[SB_FE2000_IF_PTYPE_XGM0] = SB_FE2000_MAX_XG_PORTS;
    maxports[SB_FE2000_IF_PTYPE_XGM1] = SB_FE2000_MAX_XG_PORTS;

    hg = SOC_SBX_CFG_FE2000_IF_IS_HIGIG(unit, SB_FE2000_IF_PTYPE_XGM0);
    maxports[SB_FE2000_IF_PTYPE_XGM0] = (hg) ? SB_FE2000_MAX_HG_SUBPORTS : maxports[SB_FE2000_IF_PTYPE_XGM0];
    hg = SOC_SBX_CFG_FE2000_IF_IS_HIGIG(unit, SB_FE2000_IF_PTYPE_XGM1);
    maxports[SB_FE2000_IF_PTYPE_XGM1] = (hg) ? SB_FE2000_MAX_HG_SUBPORTS : maxports[SB_FE2000_IF_PTYPE_XGM1];
    
    maxports[SB_FE2000_IF_PTYPE_PCI] = 1;

    queues->mir0qid = 0;
    queues->mir1qid = 0;
    queues->topciqid = 0;
    queues->frompciqid = 0;

    /* Initialize per-connection state */
    for (i = 0; i < SB_FE2000_SWS_INIT_MAX_CONNECTIONS; i++) {
        queues->qid2con[i] = SB_FE2000_SWS_UNINITIALIZED_CON;
        queues->many2onecon[i] = SB_FE2000_SWS_UNINITIALIZED_CON;
        queues->one2manycon[i] = SB_FE2000_SWS_UNINITIALIZED_CON;
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
        queues->one2manycon[i] = SB_FE2000_SWS_UNINITIALIZED_CON;
        for (j = 0; j < i; j++) {
            if (queues->many2onecon[i] == SB_FE2000_SWS_UNINITIALIZED_CON
                && toIface == queues->connections[j].to.ulInterfacePortType
                && (toPort == queues->connections[j].to.ulPortId ||
                    SOC_SBX_CFG_FE2000_IF_IS_HIGIG(unit, toIface))) {
                queues->many2onecon[i] = j;
            }

            if (queues->one2manycon[i] == SB_FE2000_SWS_UNINITIALIZED_CON
                && fromIface == queues->connections[j].from.ulInterfacePortType
                && SOC_SBX_CFG_FE2000_IF_IS_HIGIG(unit, fromIface)) {
                queues->one2manycon[i] = j;
            }

            if (c->ulUcodePort == queues->connections[j].ulUcodePort
                && c->bEgress == queues->connections[j].bEgress) {
                SB_LOG("Duplicate uCode flow in SWS connection"
                       " %d & %d (%d/%d:%d.%d)\n",
                       i, j, fromIface, fromPort, c->ulUcodePort, c->bEgress);
                return SB_BAD_ARGUMENT_ERR_CODE;
            }
        }
        if ( egress && (queues->one2manycon[i]==SB_FE2000_SWS_UNINITIALIZED_CON) ) {
            queues->if2con[toIface][toPort][egress] = i;
        } 
        if ( !egress ){
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
        port = egress
            ? c->to.ulPortId
            : c->from.ulPortId;

        xg = SOC_SBX_CFG_FE2000_IF_IS_XG(unit, iface);
        hg = SOC_SBX_CFG_FE2000_IF_IS_HIGIG(unit, iface);

        switch (iface) {
        case SB_FE2000_IF_PTYPE_SPI0:
        case SB_FE2000_IF_PTYPE_SPI1:
            /* SPI front panel ports are low speed */
            /* portbw = 10000 / queues->ifports[iface]; */
            portbw = 100;
            break;
        case SB_FE2000_IF_PTYPE_AGM0:
        case SB_FE2000_IF_PTYPE_AGM1:
            if (xg) {
                portbw = 10000;
            }else if (hg) {
                portbw =  12000;
            }else{
                portbw = 1000;
            }
            break;
        case SB_FE2000_IF_PTYPE_XGM0:
        case SB_FE2000_IF_PTYPE_XGM1:
            if (xg) {
                portbw = 10000;
            }else{
                portbw = 12000;
            }
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

    for (i = 0; i < SB_FE2000XT_SWS_INGRESS_QUEUE_GROUPS; i++) {
        queuesbygroup[i] = SB_FE2000XT_SWS_QUEUES_PER_GROUP;
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
            group = SB_FE2000XT_SWS_INGRESS_QUEUE_GROUPS - 1;

            /* find the least utilized group */
            for (j = SB_FE2000XT_SWS_INGRESS_QUEUE_GROUPS - 2; j >= 0 ; j--) {
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
                for (j = SB_FE2000XT_SWS_INGRESS_QUEUE_GROUPS - 2; j >= 0 ; j--) {
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
            qid = (group + 1) * SB_FE2000XT_SWS_QUEUES_PER_GROUP - queuesbygroup[group]
                + portspergroup - 1;
            for (; portspergroup; i++) {
                con = queues->conbybw[i];
                c = &queues->connections[con];
                if (c->bEgress) {
                    continue;
                }
                queues->fromqid[con] = qid;
                queues->toqid[con] = qid + SB_FE2000XT_TO_QUEUE_OFFSET;
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
                    if (queues->ifexpmode[c->from.ulInterfacePortType]) {
                        SB_LOGV3("    qid: 0x%x set for expanded mode FIFO\n", qid);
                    }
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
            qid = queues->port2iqid[c->ulUcodePort] + SB_FE2000XT_EGRESS_QUEUE_OFFSET;
            queues->port2eqid[c->ulUcodePort] = qid;
            queues->qid2con[qid] = i;
            queues->fromqid[i] = qid;
            queues->toqid[i] = qid + SB_FE2000XT_TO_QUEUE_OFFSET;
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

            /* According to C2 spec, 3.4.5 Packet Receiver, the PCI
             * is hardcoded to PB0 
             */
            if (c->from.ulInterfacePortType == SB_FE2000_IF_PTYPE_PCI) {
                pbsel = 0;
            } else {
                pbsel = (owe < 0);
            }
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
     * Do muxed queue assignment
     */
    for (i = 0; i < queues->n; i++) {
        c = &queues->connections[i];
        if (queues->many2onecon[i] != SB_FE2000_SWS_UNINITIALIZED_CON) {
            queues->toqid[i] = queues->toqid[queues->many2onecon[i]];
        }
        SB_LOGV3("    iface: 0x%x, port: 0x%x --> qid: 0x%x\n", c->from.ulInterfacePortType,
                    c->from.ulPortId, queues->fromqid[i]);
        SB_LOGV3("    toqid: 0x%x --> iface:0x%x, port0x%x\n", queues->toqid[i],
                    c->to.ulInterfacePortType, c->to.ulPortId);
        if ( (c->to.ulInterfacePortType == SB_FE2000_IF_PTYPE_XGM0) &&
                SOC_SBX_CFG_FE2000_IF_IS_HIGIG(unit, SB_FE2000_IF_PTYPE_XGM0) &&
                !queues->mir0qid) {
            queues->mir0qid = queues->toqid[i];
            SB_LOGV3("    mirror 0 qid: 0x%x\n", queues->mir0qid);
        } else if ( (c->to.ulInterfacePortType == SB_FE2000_IF_PTYPE_XGM1) &&
                SOC_SBX_CFG_FE2000_IF_IS_HIGIG(unit, SB_FE2000_IF_PTYPE_XGM1) &&
                !queues->mir1qid) {
            queues->mir1qid = queues->toqid[i];
            SB_LOGV3("    mirror 1 qid: 0x%x\n", queues->mir1qid);
        }
    }

    /*
     * Compute page allocation & flow control thresholds
     */

    /*
     * This will allocate fewer from-queue maximum pages than possible
     * when there are many2one connections (it assumes every connection
     * consumes tomin pages for a to-queue)
     */
    for (i = 0; i < queues->n; i++) {
        sbFe2000QmParams_t mac_to_hpp, hpp_to_spi;
        sbFe2000QmParams_t spi_to_hpp, hpp_to_mac;
        sbFe2000QmParams_t *to;    /*   HPP->*   */
        sbFe2000QmParams_t *from;  /*   *->HPP   */
        uint32_t higig = 0;

        if (queues->connections[i].bEgress &&
            SOC_SBX_CFG_FE2000_IF_IS_HIGIG(unit, queues->connections[i].from.ulInterfacePortType)) {
            higig = 1;
        }
        sbFe2000SwsConfigCompute(ip->uHppFreq / 1000, ip->uEpoch,
                                 SB_FE2000XT_LR_MAX_NUMBER_OF_CONTEXTS,
                                 SB_FE2000XT_MAX_PACKET,
                                 queues->bw[i] * 1000, higig,
                                 &mac_to_hpp, &hpp_to_spi,
                                 &spi_to_hpp, &hpp_to_mac);
        
        if (queues->connections[i].bEgress) {
            from = &spi_to_hpp;
            to   = &hpp_to_mac;
        } else {
            from = &mac_to_hpp;
            to   = &hpp_to_spi;
        }
        
        queues->toflowthresh[i] = to->threshold;
        queues->mintopages[i]   = to->min_pages;
        queues->maxtopages[i]   = to->max_pages;
        
        queues->fromflowthresh[i] = from->threshold;
        queues->maxfrompages[i]   = from->max_pages;
        queues->minfrompages[i]   = from->min_pages;        
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
        xg = SOC_SBX_CFG_FE2000_IF_IS_XG(unit, i);
        hg = SOC_SBX_CFG_FE2000_IF_IS_HIGIG(unit, i);
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
                if (!xg) {
                    ip->pr.uPbPrefetchPages[i][j] =
                        pbcnt[i][j] > 4 ? 14 : pbcnt[i][j] * 2;
                }else{
                    ip->pr.uPbPrefetchPages[i][j] = pbcnt[i][j] ? 14 : 0;
                }
                break;
            case SB_FE2000_IF_PTYPE_AGM1:
                if (!xg) {
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
    spitlines[0] = SB_FE2000XT_SPI_MAX_LINES;
    spitlines[1] = SB_FE2000XT_SPI_MAX_LINES;
    spirlines[0] = SB_FE2000XT_SPI_MAX_LINES;
    spirlines[1] = SB_FE2000XT_SPI_MAX_LINES;

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
            if (lines < SB_FE2000XT_SPI_MIN_LINES_PER_PORT) {
                ip->sr[j].nFifoLines[fromPort] = SB_FE2000XT_SPI_MIN_LINES_PER_PORT;
                spirlines[j] -= SB_FE2000XT_SPI_MIN_LINES_PER_PORT;
                ifbw[fromIface][0] -= portbw;
            }
            }
        switch (toIface) {
        case SB_FE2000_IF_PTYPE_SPI0:
        case SB_FE2000_IF_PTYPE_SPI1:
            j = toIface - SB_FE2000_IF_PTYPE_SPI0;
            lines = spitlines[j] * portbw / ifbw[toIface][1];
            if (lines < SB_FE2000XT_SPI_MIN_LINES_PER_PORT) {
                ip->st[j].nFifoLines[toPort] = SB_FE2000XT_SPI_MIN_LINES_PER_PORT;
                spitlines[j] -= SB_FE2000XT_SPI_MIN_LINES_PER_PORT;
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
                           SB_FE2000XT_SPI_MAX_LINES_PER_PORT);
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
                           SB_FE2000XT_SPI_MAX_LINES_PER_PORT);
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

        xg = SOC_SBX_CFG_FE2000_IF_IS_XG(unit, iface);
        hg = SOC_SBX_CFG_FE2000_IF_IS_HIGIG(unit, iface);

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
        case SB_FE2000_IF_PTYPE_AGM1:
            iface -= SB_FE2000_IF_PTYPE_AGM0;
            if (!xg) {
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
     * and fabric port numbering
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
    


    xg = SOC_SBX_CFG_FE2000_IF_IS_XG(unit, SB_FE2000_IF_PTYPE_XGM0);
    hg = SOC_SBX_CFG_FE2000_IF_IS_HIGIG(unit, SB_FE2000_IF_PTYPE_XGM0);
    ip->xg[0].bBringUp = (xg || hg) ? TRUE : FALSE;

    xg = SOC_SBX_CFG_FE2000_IF_IS_XG(unit, SB_FE2000_IF_PTYPE_XGM1);
    hg = SOC_SBX_CFG_FE2000_IF_IS_HIGIG(unit, SB_FE2000_IF_PTYPE_XGM1);
    ip->xg[1].bBringUp = (xg || hg) ? TRUE : FALSE;

    xg = SOC_SBX_CFG_FE2000_IF_IS_XG(unit, SB_FE2000_IF_PTYPE_AGM0);
    hg = SOC_SBX_CFG_FE2000_IF_IS_HIGIG(unit, SB_FE2000_IF_PTYPE_AGM0);
    ip->ag[0].bBringUp = (xg || hg) ? FALSE : TRUE;
    ip->xg[2].bBringUp = (xg || hg) ? TRUE : FALSE;

    xg = SOC_SBX_CFG_FE2000_IF_IS_XG(unit, SB_FE2000_IF_PTYPE_AGM1);
    hg = SOC_SBX_CFG_FE2000_IF_IS_HIGIG(unit, SB_FE2000_IF_PTYPE_AGM1);
    ip->ag[1].bBringUp = (xg || hg) ? FALSE : TRUE;
    ip->xg[3].bBringUp = (xg || hg) ? TRUE : FALSE;

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

static int 
sbFe2000PpQueueConfig(sbhandle userDeviceHandle,
                      uint32_t uQueue,
                      uint32_t uBatch, 
                      uint32_t uPriority)
{
      sbZfC2PpQueuePriorityGroupEntry_t zQueueConfig;
      sbZfC2PpQueuePriorityGroupEntry_InitInstance(&zQueueConfig);
      zQueueConfig.m_uPriority = uPriority;
      zQueueConfig.m_uBatchGroup = uBatch;

      if( C2PpQueuePriorityGroupWrite(userDeviceHandle, uQueue, &zQueueConfig) ) {
          return 0;
      } else {
        SB_ERROR("%s Failed on write for queue(%d).",__PRETTY_FUNCTION__, uQueue);
          return -1;
      }
}

int 
sbFe2000XtIsBatchSupported(uint32_t unit)
{
    return 0;
}

uint8_t
sbFe2000XtMaxBatch(uint32_t unit)
{
    if(sbFe2000IsBatchSupported(unit)){
        return SOC_SBX_CFG_FE2000(unit)->maxBatch;
    } else {
        return -1;
    }
}

#define DEBUG_PORT_BATCH_ASSGN
sbStatus_t
sbFe2000XtInitBatch(sbhandle userDeviceHandle,
                  uint32_t unit,
                  sbFe2000XtInitParams_t *ip,
                  sbFe2000InitPortBatchMap_t *pBatchMap,
                  uint32_t *pMaxBatch)
{
    sbFe2000Queues_t *pQueues = &ip->queues;
    sbFe2000XtInitParamsPp_t *pp = &ip->pp;
    sbFe2000Connection_t    *pConn;
    pbmp_t                  ge_pbm, xe_pbm;  
    int                     geCount=0, xeCount=0, xGeBlocks=0;
    int                     index, bIndex, fromQid, toQid;
    int                     bandWidth, perBatchBw, deltaBw;
    int                     xeThisBatch, geThisBatch, updatePp;

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
                          pp->QueueConfiguration[fromQid].uBatchGroup = 0;
                          pp->QueueConfiguration[toQid].uBatchGroup = \
                               pp->QueueConfiguration[fromQid].uBatchGroup + 1;   
                          pp->QueueConfiguration[fromQid].uPriority = 0;
                          pp->QueueConfiguration[toQid].uPriority = 0;
                          SOC_PBMP_PORT_SET(pBatchMap[index].pbm, pConn->ulUcodePort);
                          sbFe2000PpQueueConfig(userDeviceHandle, fromQid, 
                                          pp->QueueConfiguration[fromQid].uBatchGroup,
                                          pp->QueueConfiguration[fromQid].uPriority);
                          sbFe2000PpQueueConfig(userDeviceHandle, toQid, 
                                          pp->QueueConfiguration[toQid].uBatchGroup,
                                          pp->QueueConfiguration[toQid].uPriority);
                          updatePp = 0;
                          pQueues->batchAssigned[index] = 1;
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
                    SOC_PBMP_PORT_SET(pBatchMap[index].pbm, pConn->ulUcodePort);
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
                if((geThisBatch==0) && (xeThisBatch==0)){
                    soc_cm_print("\n#### End of batch loop ###\n");
                    break;
                }
            }
        }
    }
    return SB_OK;
}





/*
 * Function:
 *     sbFe2000XtFlushEccOnSpi
 * Purpose:
 *     Flush ecc errors on spi interfaces
 * Parameters:
 *     unit  - Device number
 * Returns:
 *     SOC_E_*   as appropriate
 */
int
sbFe2000XtFlushEccOnSpi(int unit)
{
    int spi, qid, i, spiSupported, rv;
    sbhandle userDeviceHandle;
    sbZfC2PtQueueToPortEntry_t queueToPort;
    sbZfC2QmQueueConfigEntry_t queueConfig;
    sbZfC2QmQueueState0EnEntry_t queueState;
    char *txBuffer, *rxBuffer;
    int rxBufferLen;
    const int bufferLen = 16384;
    uint32_t preConfig, pbPages, pbEnable, prEnable;
    int pciPbSel, spiPbSel[2];

    userDeviceHandle = (sbhandle)unit;
    rv = SOC_E_NONE;

    /* Check to see if the current configuration will support the spi flush */
    /* For each Packet Receiver Element, check to see if it is configured to
     * prefetch pages
     */
    spiSupported = 3;  /* assume both are supported */

    preConfig = SAND_HAL_READ(userDeviceHandle, C2, PR_PRE6_CONFIG);

    pbEnable = SAND_HAL_GET_FIELD(C2, PR_PRE6_CONFIG,
                                  PB_ENABLE, preConfig);
    pciPbSel = 0;
    pbPages  = 0;
    if (pbEnable & 1) {
        pbPages  = SAND_HAL_GET_FIELD(C2, PR_PRE6_CONFIG,
                                      PB0_PREFETCH_PAGES, preConfig);
        pciPbSel = 0;
        
    } else if (pbEnable & 2) {
        pbPages  = SAND_HAL_GET_FIELD(C2, PR_PRE6_CONFIG,
                                      PB1_PREFETCH_PAGES, preConfig);
        pciPbSel = 1;
    }

    prEnable = SAND_HAL_GET_FIELD(C2, PR_PRE6_CONFIG, 
                                  PRE_ENABLE, preConfig);
    if ( (pbPages < 1) || (!pbEnable) || (prEnable == 0)) {
        /* pci PRE is not configured to support the flush */
        return 0;
    }

    /* enable packet receive on the SPI interface port */
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE0_ENABLE_CONFIG0,
                         PORT_ENABLE,0xffffffff);
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE0_ENABLE_CONFIG1,
                         PORT_ENABLE,0xffffffff);
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE1_ENABLE_CONFIG0,
                         PORT_ENABLE,0xffffffff);
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2,PR_PRE1_ENABLE_CONFIG1,
                         PORT_ENABLE, 0xffffffff);

    for (spi = 0; spi < 2; spi++) {
        if (spi) {
            preConfig = SAND_HAL_READ(userDeviceHandle, C2, PR_PRE1_CONFIG);
        } else {
            preConfig = SAND_HAL_READ(userDeviceHandle, C2, PR_PRE0_CONFIG);
        }

        pbEnable = SAND_HAL_GET_FIELD(C2, PR_PRE0_CONFIG,
                                      PB_ENABLE, preConfig);

        spiPbSel[spi] = 0;
        pbPages       = 0;
        if (pbEnable & 1) {
            pbPages  = SAND_HAL_GET_FIELD(C2, PR_PRE0_CONFIG,
                                          PB0_PREFETCH_PAGES, preConfig);
        } else if (pbEnable & 2) {
            pbPages  = SAND_HAL_GET_FIELD(C2, PR_PRE0_CONFIG,
                                          PB1_PREFETCH_PAGES, preConfig);
            spiPbSel[spi] = 1;
        }

        prEnable = SAND_HAL_GET_FIELD(C2, PR_PRE0_CONFIG, 
                                      PRE_ENABLE, preConfig);
        if ( (pbPages < 1) || (!pbEnable) || (prEnable == 0)) {
            spiSupported &= ~(1<<spi);
        }
    }

    /*
     * set spi in local loopback
     * configure SWS queues for PCI->ST[0,1] ; SR[0,1]->PCI
     * send a packet to each interface
     */
    txBuffer = soc_cm_salloc(unit, bufferLen, "ecc flush pkt");
    if (!txBuffer) {
        soc_cm_print("failed to allocate pkt tx buffer\n");
        return SOC_E_MEMORY;
    }

    rxBuffer = soc_cm_salloc(unit, bufferLen, "ecc flush pkt");
    if (!rxBuffer) {
        soc_cm_sfree(unit, txBuffer);
        soc_cm_print("failed to allocate pkt rx buffer\n");
        return SOC_E_MEMORY;
    }

    for (i=0; i<64; i++) {
        txBuffer[i] = i;
    }

    /* configure the bare-minimum to push a packet through the system */
    sbZfC2PtQueueToPortEntry_InitInstance(&queueToPort);
    sbZfC2QmQueueConfigEntry_InitInstance(&queueConfig);
    sbZfC2QmQueueState0EnEntry_InitInstance(&queueState);

    for (spi=0; spi<2 && SOC_SUCCESS(rv); spi++) {
        if (!(spiSupported & (1<<spi))) {
            continue;
        }

        /* enable looback */
        SAND_HAL_RMW_FIELD_STRIDE(unit, C2, ST, spi, ST_CONFIG2,
                                  LOOPBACK_ENABLE, 1);
            
        SAND_HAL_RMW_FIELD_STRIDE(unit, C2, SR, spi, SR_CONFIG1,
                                  LOOPBACK_ENABLE, 1);

        queueConfig.m_uDropThresh2 = 0xfff;
        queueConfig.m_uEnable = 1;
            
        queueState.m_uEnable = 1;
            
        /* Configure the PCI->St[spi] queue */
        qid = 0;
        queueConfig.m_uMaxPages = 10;
        C2QmQueueConfigWrite(userDeviceHandle, qid, &queueConfig);
        C2QmQueueState0EnWrite(userDeviceHandle, qid, &queueState);
            
        queueToPort.m_uPpeBound = 0;
        queueToPort.m_uDestPort = 0;
        queueToPort.m_uPbSel    = pciPbSel;
        queueToPort.m_uDestInterface =  SB_FE2000_IF_PTYPE_SPI0 + spi;
        C2PtQueueToPortWrite(userDeviceHandle, qid, &queueToPort);
        C2PrPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_PCI, 
                             0, qid);
            
        /* Configure the SR[spi]->PCI queue */
        qid++;
        queueConfig.m_uMaxPages = 10;
        C2QmQueueConfigWrite(userDeviceHandle, qid, &queueConfig);
        C2QmQueueState0EnWrite(userDeviceHandle, qid, &queueState);
        queueToPort.m_uPpeBound = 0;
        queueToPort.m_uDestPort = 0;
        queueToPort.m_uPbSel    = spiPbSel[spi];
        queueToPort.m_uDestInterface =  SB_FE2000_IF_PTYPE_PCI;
        C2PtQueueToPortWrite(userDeviceHandle, qid, &queueToPort);
        C2PtPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_SPI0 + spi, 
                             0, qid-1);
        C2PtPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_PCI, 
                             0, qid); 
        C2PrPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_SPI0 + spi, 
                             0, qid);

        /* send a packet to flush the first ECC error in SPIs */
        rv = soc_sbx_txrx_sync_tx(unit, NULL, 0, txBuffer, 64, 1000000);
        if (SOC_FAILURE(rv)) {
            soc_cm_print("Failed to send ecc flush packet: %d %s\n",
                         rv, soc_errmsg(rv));
        }
        
        rxBufferLen = bufferLen;
        rv = soc_sbx_txrx_sync_rx(unit, rxBuffer, &rxBufferLen, 1000000);
        if (SOC_FAILURE(rv)) {
            soc_cm_print("Failed to receive ecc flush packet on spi %d: "
                         "%d %s\n", spi, rv, soc_errmsg(rv));
        }

        /* clean up */
        sbZfC2QmQueueConfigEntry_InitInstance(&queueConfig);
        sbZfC2QmQueueState0EnEntry_InitInstance(&queueState);
        sbZfC2PtQueueToPortEntry_InitInstance(&queueToPort);
        qid = 0;
                
        C2QmQueueConfigWrite(userDeviceHandle, qid, &queueConfig);
        C2QmQueueState0EnWrite(userDeviceHandle, qid, &queueState);
        C2PtQueueToPortWrite(userDeviceHandle, qid, &queueToPort);
        C2PrPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_PCI, 
                             0, 0xFF);

        qid++;
        C2QmQueueConfigWrite(userDeviceHandle, qid, &queueConfig);
        C2QmQueueState0EnWrite(userDeviceHandle, qid, &queueState);
        C2PtQueueToPortWrite(userDeviceHandle, qid, &queueToPort);
        C2PtPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_SPI0 + spi,
                             0, 0xFF);
        C2PtPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_PCI, 0, qid); 
        C2PrPortToQueueWrite(userDeviceHandle, SB_FE2000_IF_PTYPE_SPI0 + spi, 
                             0, 0xFF);

        SAND_HAL_RMW_FIELD_STRIDE(unit, C2, ST, spi, ST_CONFIG2,
                                  LOOPBACK_ENABLE, 0);
                
        SAND_HAL_RMW_FIELD_STRIDE(unit, C2, SR, spi, SR_CONFIG1,
                                  LOOPBACK_ENABLE, 0);
        
    }
    
    soc_cm_sfree(unit, txBuffer);
    soc_cm_sfree(unit, rxBuffer);

    return rv;
}


/*
 * Function:
 *     bFe2000XtEccClear
 * Purpose:
 *     clear out all ecc errors that occur during init
 * Parameters:
 *     unit  - Device number
 * Returns:
 *     SOC_E_*   as appropriate
 */
int
sbFe2000XtEccClear(int unit)
{
    int rv = SOC_E_NONE, i;
    uint32_t uData;

    /*vn-0909210830 fix for the crash on the bcmsim start up
      since fifo implementaiton in the lcmodel is not finished,
      and PC_RXBUF_FIFO_DEBUG mode is not implemented
      we have to bail out of this funciton to prevent crash */
    /*vn-0909210830+*/ if (SAL_BOOT_BCMSIM) {
    /*vn-0909210830+*/     SOC_DEBUG_PRINT((DK_VERBOSE, "Skipping %s for sbx bcmsim initialization\n", FUNCTION_NAME()));
    /*vn-0909210830+*/     return(rv);
    /*vn-0909210830+*/ }

    /* clear out the rx_buf memory*/
	for (i=0; i<256; i++) {
	    SAND_HAL_WRITE(unit, C2, PC_RXBUF_LOAD0, 0);
	}

	for (i=0; i<256; i++) {
	    uData = SAND_HAL_READ(unit, C2, PC_RXBUF_FIFO_DEBUG);
	    uData = SAND_HAL_MOD_FIELD(C2, PC_RXBUF_FIFO_DEBUG, POP_FIFO, uData, 1);
	    SAND_HAL_WRITE(unit, C2, PC_RXBUF_FIFO_DEBUG, uData);
	}


    /* clear out the first ECC error on SPIs by sending a packet 
     * through the interfaces.*/
        rv = sbFe2000XtFlushEccOnSpi(unit);
        if (rv) {
            soc_cm_print("Failed to flush ecc on spi: %d %s\n", 
                         rv, soc_errmsg(rv));
        }

    /* after all errors have been forced; clear them out */
    SAND_HAL_WRITE(unit, C2, SR0_ECC_ERROR, ~0);
    SAND_HAL_WRITE(unit, C2, SR1_ECC_ERROR, ~0);  
    SAND_HAL_WRITE(unit, C2, ST0_ECC_ERROR, ~0);
    SAND_HAL_WRITE(unit, C2, ST1_ECC_ERROR, ~0);
    SAND_HAL_WRITE(unit, C2, XG0_ECC_ERROR, ~0);
    SAND_HAL_WRITE(unit, C2, XG1_ECC_ERROR, ~0);
    SAND_HAL_WRITE(unit, C2, XG2_ECC_ERROR, ~0);
    SAND_HAL_WRITE(unit, C2, XG3_ECC_ERROR, ~0);
    SAND_HAL_WRITE(unit, C2, AG0_ECC_ERROR, ~0);
    SAND_HAL_WRITE(unit, C2, AG1_ECC_ERROR, ~0);
    SAND_HAL_WRITE(unit, C2, PR_ECC_ERROR, ~0);  
    SAND_HAL_WRITE(unit, C2, PT_ECC_ERROR, ~0);
    SAND_HAL_WRITE(unit, C2, PB0_ECC_ERROR, ~0);
    SAND_HAL_WRITE(unit, C2, PB1_ECC_ERROR, ~0);
    SAND_HAL_WRITE(unit, C2, PP_ECC_ERROR, ~0);
    SAND_HAL_WRITE(unit, C2, PD_ECC_ERROR, ~0);
    SAND_HAL_WRITE(unit, C2, LR_ECC_ERROR, ~0);
    SAND_HAL_WRITE(unit, C2, RC0_ECC_ERROR, ~0);
    SAND_HAL_WRITE(unit, C2, RC1_ECC_ERROR, ~0);
    SAND_HAL_WRITE(unit, C2, PM_ECC_ERROR, ~0);
    SAND_HAL_WRITE(unit, C2, QM_ECC_ERROR, ~0);
    SAND_HAL_WRITE(unit, C2, LU_ECC_ERROR, ~0);

    SAND_HAL_WRITE(unit, C2, PC_RXBUF_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, SR0_OVFL_FIFO_0_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, SR1_OVFL_FIFO_0_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, SR0_OVFL_FIFO_1_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, SR1_OVFL_FIFO_1_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, SR0_PORT_FIFO_0_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, SR1_PORT_FIFO_0_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, SR0_PORT_FIFO_1_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, SR1_PORT_FIFO_1_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, SR0_CNT_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, SR1_CNT_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, SR0_RX_FIFO_HI_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, SR1_RX_FIFO_HI_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, SR0_RX_FIFO_LO_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, SR1_RX_FIFO_LO_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, ST0_PORT_FIFO_0_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, ST1_PORT_FIFO_0_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, ST0_PORT_FIFO_1_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, ST1_PORT_FIFO_1_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, ST0_RT_FIFO_0_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, ST1_RT_FIFO_0_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, ST0_RT_FIFO_1_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, ST1_RT_FIFO_1_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, ST0_CNT_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, ST1_CNT_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, XG0_E2ECC_DEBUG0, ~0);
    SAND_HAL_WRITE(unit, C2, XG1_E2ECC_DEBUG0, ~0);
    SAND_HAL_WRITE(unit, C2, XG2_E2ECC_DEBUG0, ~0);
    SAND_HAL_WRITE(unit, C2, XG3_E2ECC_DEBUG0, ~0);
    SAND_HAL_WRITE(unit, C2, XG0_E2ECC_DEBUG1, ~0);
    SAND_HAL_WRITE(unit, C2, XG1_E2ECC_DEBUG1, ~0);
    SAND_HAL_WRITE(unit, C2, XG2_E2ECC_DEBUG1, ~0);
    SAND_HAL_WRITE(unit, C2, XG3_E2ECC_DEBUG1, ~0);
    SAND_HAL_WRITE(unit, C2, PR_CC2_TCAM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, PR_CC3_TCAM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, PR_CC4_TCAM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, PR_CC5_TCAM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, PT_QUEUE_TO_PORT_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, PT_PPE_BURST_FIFO_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, PT_DEQ_RESP_BUFFER_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, PT_PTE0_PORT_STATE_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, PT_PTE1_PORT_STATE_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, PT_PTE0_REMAINDER_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, PT_PTE1_REMAINDER_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, PM_PROFILE_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, PM_LRP_REQ_BUFFER_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, PM_READ_PENDING_BUFFER_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, PM_WB_BUFFER_MEM0_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, PM_WB_BUFFER_MEM1_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, PM_TAGGED_TIMESTAMP_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, PM_TAGGED_BKT_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, PM_COUNT_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_QUEUE_STATE1_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_QUEUE_STATE0_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_QUEUE_CONFIG_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_BUFFER_STATE0_0_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_BUFFER_STATE0_1_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_BUFFER_STATE1_0_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_BUFFER_STATE1_1_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_REPL_STATE_0_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_REPL_STATE_1_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_BUFFER_STATE2_0_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_BUFFER_STATE2_1_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_BUFFER_STATE2_2_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_BUFFER_STATE2_3_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_BUFFER_STATE3_0_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_BUFFER_STATE3_1_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_BUFFER_STATE3_2_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_BUFFER_STATE3_3_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_NEXT_BUFFER_0_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_NEXT_BUFFER_1_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_NEXT_PAGE_0_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_NEXT_PAGE_1_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_NEXT_PAGE_2_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_NEXT_PAGE_3_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_FREE_PAGE_FIFO_0_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_FREE_PAGE_FIFO_1_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_FREE_PAGE_FIFO_2_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_FREE_PAGE_FIFO_3_MEM_ECC_DEBUG, ~0);
    SAND_HAL_WRITE(unit, C2, QM_DEQ_STATE0_MEM_ECC_DEBUG, ~0);

    SAND_HAL_WRITE(unit, C2, PC_ECC_ERROR, ~0);
    SAND_HAL_WRITE(unit, C2, PC_ERROR1, ~0);

    return rv;
}
