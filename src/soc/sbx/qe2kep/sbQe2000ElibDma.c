/*
 *
 * ==========================================================
 * == sbQe2000Dma.c - ELIB User supplied DMA functionality ==
 * ==========================================================
 *
 * WORKING REVISION: $Id: sbQe2000ElibDma.c 1.3.178.1 Broadcom SDK $
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
 * MODULE NAME:
 *
 *     sbQe2000Dma.c
 *
 * ABSTRACT:
 *
 *     Sample code for the user supplied DMA functionality.
 *     This code allows the Qe2000 Egress Library to be operating
 *     system and glue layer independant.
 *
 *     The user needs to modify this code as appropriate for the
 *     OS and glue layer used.  The samples provided below outline
 *     the usage, and may need to be taylored.
 *
 *
 *
 *      SAMPLE CODE ONLY
 *
 *
 *
 *
 * LANGUAGE:
 *
 *     C
 *
 * AUTHORS:
 *
 *     Travis B. Sawyer
 *
 * CREATION DATE:
 *
 *     15-June-2005
 *
 */
#ifndef __KERNEL__
#include <stdlib.h>
#endif
#include <stdarg.h>
#include "sbTypes.h"
#if !defined(SIM) && defined(SB_LINUX) && !defined(NO_QENIC)
#include <sys/ioctl.h>
#include <linux/sbqeioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <errno.h>
#endif
#include "glue.h"

#include "sbQe2000Elib.h"
#include "sbQe2000ElibDma.h"
#include "sbWrappers.h"

/**
 * sbQe2kNicDma
 *
 * Utilize the Sandburst QE2000 PCI NIC Driver to perform statistics
 * DMAs.
 *
 */
#if !defined(SIM) && defined(SB_LINUX) && !defined(NO_QENIC)
int sbQe2kNicDMA(sbhandle hSbQe, struct sb_qe2000_elib_dma_func_params_s *pDmaArg )
{
    int status;
    SBQE2K_STATS_IO QeStats;
    struct sbqe_ioc_cmd_s qe_ioc_cmd;
    struct ifreq sIfr;
    struct sockaddr_ll	sll;
    int fd;

    SB_ASSERT( pDmaArg );
    SB_ASSERT( pDmaArg->pData );
    SB_ASSERT( pDmaArg->pUserData );


    fd = thin_get_fd(hSbQe);
    SB_MEMSET(&sIfr, 0, sizeof(sIfr));

    /*
     * Bind to the QE NIC driver
     */
    SB_STRNCPY(sIfr.ifr_name, pDmaArg->pUserData, sizeof(sIfr.ifr_name));
    if (-1 == ioctl(fd, SIOCGIFINDEX, &sIfr))
    {
        return( -1 );
    }

    SB_MEMSET(&sll, 0, sizeof(sll));
    sll.sll_family	= AF_PACKET;
    sll.sll_ifindex	= sIfr.ifr_ifindex;
    sll.sll_protocol	= htons(ETH_P_ALL);

    if ( -1 == bind(fd, (struct sockaddr *) &sll, sizeof(sll)))
    {
        return( -1 );
    }

    /*
     * Setup for the ioctl to run the stats dma
     */
    qe_ioc_cmd.data = (void *)&QeStats;
    qe_ioc_cmd.cmd = SBQE_DMA_STATS;
    sIfr.ifr_data = (char *)&qe_ioc_cmd;
    switch(pDmaArg->StatsMem)
    {
        case SB_QE2000_ELIB_STATS_PCT:
            QeStats.req_type = SBQE2K_CNT_STATS_REQ;
            break;

        case SB_QE2000_ELIB_IP_MEM:
            QeStats.req_type = SBQE2K_IP_MEM_REQ;
            break;

        case SB_QE2000_ELIB_STATS_VRT:
            QeStats.req_type = SBQE2K_BF_CNT_STATS_REQ;
            break;

        default:
            return( -1 );
            break;
    }

    QeStats.start_addr = pDmaArg->ulStartAddr;
    QeStats.num_lines =  pDmaArg->ulNumLines;
    QeStats.preserve = pDmaArg->ulPreserve;
    QeStats.addr =  pDmaArg->pData;

    status = ioctl(fd, SIOCDEVPRIVATE, &sIfr);
    if( 0 != status )
    {
        return( status );
    }

    return( 0 );
}
#endif /* SB_LINUX && !NO_QENIC */

/**
 * sbQe2kDma
 *
 * Utilize the Sandburst Thin Glue to perform memory management
 * (allocate DMA buffers) and perform the DMA via SAND_HAL[READ|WRITE].
 *
 * This code assumes that pDmaArg->pData is DMA'able memory
 *
 */
#if defined(SIM) || defined(NO_QENIC)
int sbQe2kDMA(sbhandle hSbQe, struct sb_qe2000_elib_dma_func_params_s *pDmaArg )
{
    uint32_t ulDmaCtl;
    int nTimeOut;
    int nAck;

    SB_ASSERT( pDmaArg );
    SB_ASSERT( pDmaArg->pData );

    SAND_HAL_WRITE(hSbQe, KA, PC_DMA_COUNT, pDmaArg->ulNumLines);
    SAND_HAL_WRITE(hSbQe, KA, PC_DMA_PCI_ADDR, (uint32_t)pDmaArg->pData);
    SAND_HAL_WRITE(hSbQe, KA, PC_DMA_CTRL,
                   SAND_HAL_SET_FIELD(KA, PC_DMA_CTRL, START, pDmaArg->ulStartAddr) |
                   SAND_HAL_SET_FIELD(KA, PC_DMA_CTRL, DMA_SRC, pDmaArg->StatsMem) |
		   SAND_HAL_SET_FIELD(KA, PC_DMA_CTRL, PRESERVE_ON_READ, pDmaArg->ulPreserve) |
                   SAND_HAL_SET_FIELD(KA, PC_DMA_CTRL, REQ, 1));

    nTimeOut = 1000;
    nAck = 0;
    while(nTimeOut--)
    {
        ulDmaCtl = SAND_HAL_READ(hSbQe, KA, PC_DMA_CTRL);
        if(SAND_HAL_GET_FIELD(KA, PC_DMA_CTRL, ACK, ulDmaCtl))
        {
            SAND_HAL_WRITE(hSbQe, KA, PC_DMA_CTRL,
                           SAND_HAL_SET_FIELD(KA, PC_DMA_CTRL, ACK, 1));
            nAck = 1;
            break;
        }
        thin_delay(1000);
    }

    if( 0 == nAck )
    {
        return( -1 );
    }

    return( 0 );

}
#endif

int sbQe2kUserSemCreate(void *pUserSemData)
{
    int nSemId;
    sbStatus_t sbStatus;
    uint32_t ulKey;

    SB_ASSERT( pUserSemData );  /* Sandburst Glue uses a keyed semaphore */

    if(NULL != pUserSemData)
    {
        ulKey = *((uint32_t *)pUserSemData);
    }
    else
    {
        return( -1 );
    }

    sbStatus = thin_sem_open(ulKey, &nSemId);
    if(SB_OK != sbStatus)
    {
        return( -1 );
    }

    /*
     * Make the semaphore available
     */
    sbStatus = thin_sem_put(nSemId);
    if(SB_OK != sbStatus)
    {
        return( -1 );
    }

    return( nSemId );
}

int sbQe2kUserSemGive(int nSemId, void* pUserSemData)
{
    sbStatus_t sbStatus;

    /*
     * We don't need the user data for the sandburst glue
     * sem put
     */
    pUserSemData = pUserSemData;

    sbStatus = thin_sem_put(nSemId);
    if(SB_OK != sbStatus)
    {
        return( -1 );
    }

    return( 0 );
}

int sbQe2kUserSemWaitGet(int nSemId, void* pUserSemData, int nTimeOut)
{
    sbStatus_t sbStatus;

    /*
     * We don't need the user data for the sandburst glue
     * sem put
     */
    pUserSemData = pUserSemData;

    sbStatus = thin_sem_wait_get(nSemId, nTimeOut);
    if(SB_OK != sbStatus)
    {
        return( -1 );
    }

    return( 0 );
}
