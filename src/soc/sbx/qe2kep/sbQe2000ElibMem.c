/*
 *
 * ==================================================
 * ==  sbQe2000ElibMem.c - elib private memory access API  ==
 * ==================================================
 *
 * WORKING REVISION: $Id: sbQe2000ElibMem.c 1.5.112.1 Broadcom SDK $
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
 *     sbQe2000ElibMem.c
 *
 * ABSTRACT:
 *
 *     elib private memory access API
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
 *     21-December-2004
 *
 */
#include "sbWrappers.h"
#include "sbTypes.h"
#include "sbQe2000Elib.h"
#include "sbQe2000ElibMem.h"
#include "sbQe2000ElibContext.h"
#include "soc/drv.h"

#if !defined(SAND_BIG_ENDIAN_HOST)
#define BSWAP32(__X) (__X)
#else
#define BSWAP32(__X) SAND_SWAP_32(__X)
#endif

sbElibStatus_et sbQe2000ElibClMemWrite( sbhandle HalCtxt,
                    uint32_t ulOffset,
                    uint32_t ulData0,
                    uint32_t ulData1 )
{
    int nTimeout;
    int nAck;
    uint32_t ulCtlReg;

    SB_ASSERT( ulOffset < SB_QE2000_ELIB_CL_MEM_MAX_OFFSET );

    /*
     * Clear out any previous acks in the mem ctrl register
     */
    ulCtlReg = (SAND_HAL_SET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, ACK, 1) |
                SAND_HAL_SET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, REQ, 0));
    SAND_HAL_WRITE( HalCtxt, KA, EP_AM_CL_MEM_ACC_CTRL, ulCtlReg );

    /*
     * Build up our command
     */
    ulCtlReg = SAND_HAL_SET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, REQ, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, RD_WR_N, 0 ) |
        SAND_HAL_SET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, ADDR, ulOffset );

    /*
     * Write out our data
     */
    SAND_HAL_WRITE( HalCtxt, KA, EP_AM_CL_MEM_ACC_DATA0, BSWAP32(ulData0) );
    SAND_HAL_WRITE( HalCtxt, KA, EP_AM_CL_MEM_ACC_DATA1, BSWAP32(ulData1) );

    /*
     * Write out our command and wait for the acknowledgement
     */
    SAND_HAL_WRITE( HalCtxt, KA, EP_AM_CL_MEM_ACC_CTRL, ulCtlReg );

    nTimeout = SB_QE2000_ELIB_IND_MEM_TIMEOUT;
    nAck = 0;

    while ( nTimeout-- )
    {
        ulCtlReg = SAND_HAL_READ( HalCtxt, KA, EP_AM_CL_MEM_ACC_CTRL );
        if (1 == SAND_HAL_GET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, ACK, ulCtlReg ) )
        {
            /*
             * Clear the ack
             */
            nAck = 1;
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, ACK, ulCtlReg, 1 );
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, REQ, ulCtlReg, 0 );
            SAND_HAL_WRITE( HalCtxt, KA, EP_AM_CL_MEM_ACC_CTRL, ulCtlReg );
            break;
        }

        thin_delay( 100 );
    }

    if ( 0 == nAck )
    {
        return( SB_ELIB_IND_MEM_TIMEOUT );
    }

    return( SB_ELIB_OK );

}

sbElibStatus_et sbQe2000ElibClMemRead( sbhandle HalCtxt,
                           uint32_t ulOffset,
                           uint32_t ulClrOnRd,
                           uint32_t *pulData0,
                           uint32_t *pulData1 )
{
    int nTimeout;
    int nAck;
    uint32_t ulCtlReg;

    SB_ASSERT( ulOffset < SB_QE2000_ELIB_CL_MEM_MAX_OFFSET );
    SB_ASSERT( pulData0 );
    SB_ASSERT( pulData1 );

    /*
     * Clear out any previous acks in the mem ctrl register & any stale data
     */
    ulCtlReg = (SAND_HAL_SET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, ACK, 1) |
                SAND_HAL_SET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, REQ, 0));
    SAND_HAL_WRITE( HalCtxt, KA, EP_AM_CL_MEM_ACC_CTRL, ulCtlReg );
    SAND_HAL_WRITE( HalCtxt, KA, EP_AM_CL_MEM_ACC_DATA0, 0 );
    SAND_HAL_WRITE( HalCtxt, KA, EP_AM_CL_MEM_ACC_DATA1, 0 );

    /*
     * Build up our command
     */
    ulCtlReg = SAND_HAL_SET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, REQ, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, RD_WR_N, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, CLR_ON_RD, ulClrOnRd ) |
        SAND_HAL_SET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, ADDR, ulOffset );

    /*
     * Write out our command and wait for the acknowledgement
     */
    SAND_HAL_WRITE( HalCtxt, KA, EP_AM_CL_MEM_ACC_CTRL, ulCtlReg );

    nTimeout = SB_QE2000_ELIB_IND_MEM_TIMEOUT;
    nAck = 0;

    while ( nTimeout-- )
    {
        ulCtlReg = SAND_HAL_READ( HalCtxt, KA, EP_AM_CL_MEM_ACC_CTRL );
        if (1 == SAND_HAL_GET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, ACK, ulCtlReg ) )
        {
            /*
             * Grab the data & clear the ack
             */
            nAck = 1;
            *pulData0 = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EP_AM_CL_MEM_ACC_DATA0 ));
            *pulData1 = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EP_AM_CL_MEM_ACC_DATA1 ));
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, ACK, ulCtlReg, 1 );
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, REQ, ulCtlReg, 0 );
            SAND_HAL_WRITE( HalCtxt, KA, EP_AM_CL_MEM_ACC_CTRL, ulCtlReg );
            break;
        }

        thin_delay( 100 );
    }

    if ( 0 == nAck )
    {
        return( SB_ELIB_IND_MEM_TIMEOUT );
    }

    return( SB_ELIB_OK );

}


sbElibStatus_et sbQe2000ElibPriMemWrite( sbhandle HalCtxt,
                     uint32_t ulOffset,
                     uint32_t ulData )
{
    int nTimeout;
    int nAck;
    uint32_t ulCtlReg;

    SB_ASSERT( ulOffset < SB_QE2000_ELIB_PRI_MEM_MAX_OFFSET );

    /*
     * Clear out any previous acks in the mem ctrl register
     */
    ulCtlReg = (SAND_HAL_SET_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, ACK, 1 ) |
                SAND_HAL_SET_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, REQ, 0 ));
    SAND_HAL_WRITE( HalCtxt, KA, EP_BM_BF_MEM_ACC_CTRL, ulCtlReg );

    /*
     * Build up our command
     */
    ulCtlReg = SAND_HAL_SET_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, REQ, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, RD_WR_N, 0 ) |
        SAND_HAL_SET_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, ADDR, ulOffset );

    /*
     * Write out our data
     */
    SAND_HAL_WRITE( HalCtxt, KA, EP_BM_BF_MEM_ACC_DATA, BSWAP32(ulData) );

    /*
     * Write out our command and wait for the acknowledgement
     */
    SAND_HAL_WRITE( HalCtxt, KA, EP_BM_BF_MEM_ACC_CTRL, ulCtlReg );

    nTimeout = SB_QE2000_ELIB_IND_MEM_TIMEOUT;
    nAck = 0;

    while ( nTimeout-- )
    {
        ulCtlReg = SAND_HAL_READ( HalCtxt, KA, EP_BM_BF_MEM_ACC_CTRL );
        if (1 == SAND_HAL_GET_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, ACK, ulCtlReg ) )
        {
            /*
             * Clear the ack
             */
            nAck = 1;
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, ACK, ulCtlReg, 1 );
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, REQ, ulCtlReg, 0 );
            SAND_HAL_WRITE( HalCtxt, KA, EP_BM_BF_MEM_ACC_CTRL, ulCtlReg );
            break;
        }

        thin_delay( 100 );
    }

    if ( 0 == nAck )
    {
        return( SB_ELIB_IND_MEM_TIMEOUT );
    }

    return( SB_ELIB_OK );

}

sbElibStatus_et sbQe2000ElibPriMemRead( sbhandle HalCtxt,
                    uint32_t ulOffset,
                    uint32_t *pulData )
{
    int nTimeout;
    int nAck;
    uint32_t ulCtlReg;

    SB_ASSERT( ulOffset < SB_QE2000_ELIB_PRI_MEM_MAX_OFFSET );
    SB_ASSERT( pulData );

    /*
     * Clear out any previous acks in the mem ctrl register & any stale data
     */
    ulCtlReg = (SAND_HAL_SET_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, ACK, 1 ) |
                SAND_HAL_SET_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, REQ, 0 ));
    SAND_HAL_WRITE( HalCtxt, KA, EP_BM_BF_MEM_ACC_CTRL, ulCtlReg );
    SAND_HAL_WRITE( HalCtxt, KA, EP_BM_BF_MEM_ACC_DATA, 0 );

    /*
     * Build up our command
     */
    ulCtlReg = SAND_HAL_SET_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, REQ, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, RD_WR_N, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, ADDR, ulOffset );

    /*
     * Write out our command and wait for the acknowledgement
     */
    SAND_HAL_WRITE( HalCtxt, KA, EP_BM_BF_MEM_ACC_CTRL, ulCtlReg );

    nTimeout = SB_QE2000_ELIB_IND_MEM_TIMEOUT;
    nAck = 0;

    while ( nTimeout-- )
    {
        ulCtlReg = SAND_HAL_READ( HalCtxt, KA, EP_BM_BF_MEM_ACC_CTRL );
        if (1 == SAND_HAL_GET_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, ACK, ulCtlReg ) )
        {
            /*
             * Grab the data & clear the ack
             */
            nAck = 1;
            *pulData = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EP_BM_BF_MEM_ACC_DATA ));
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, ACK, ulCtlReg, 1 );
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, REQ, ulCtlReg, 0 );
            SAND_HAL_WRITE( HalCtxt, KA, EP_BM_BF_MEM_ACC_CTRL, ulCtlReg );
            break;
        }

        thin_delay( 100 );
    }

    if ( 0 == nAck )
    {
        return( SB_ELIB_IND_MEM_TIMEOUT );
    }

    return( SB_ELIB_OK );

}



sbElibStatus_et sbQe2000ElibBfMemWrite( sbhandle HalCtxt,
                    uint32_t ulOffset,
                    uint32_t ulData0,
                    uint32_t ulData1 )
{
    int nTimeout;
    int nAck;
    uint32_t ulCtlReg;

    SB_ASSERT( ulOffset < SB_QE2000_ELIB_BF_MEM_MAX_OFFSET );

    /*
     * Clear out any previous acks in the mem ctrl register
     */
    ulCtlReg = (SAND_HAL_SET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, ACK, 1 ) |
                SAND_HAL_SET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, REQ, 0 ));
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_BF_MEM_ACC_CTRL, ulCtlReg );

    /*
     * Build up our command
     */
    ulCtlReg = SAND_HAL_SET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, REQ, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, RD_WR_N, 0 ) |
        SAND_HAL_SET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, ADDR, ulOffset );

    /*
     * Write out our data
     */
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_BF_MEM_ACC_DATA0, BSWAP32(ulData0) );
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_BF_MEM_ACC_DATA1, BSWAP32(ulData1) );

    /*
     * Write out our command and wait for the acknowledgement
     */
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_BF_MEM_ACC_CTRL, ulCtlReg );

    nTimeout = SB_QE2000_ELIB_IND_MEM_TIMEOUT;
    nAck = 0;

    while ( nTimeout-- )
    {
        ulCtlReg = SAND_HAL_READ( HalCtxt, KA, EP_MM_BF_MEM_ACC_CTRL );
        if (1 == SAND_HAL_GET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, ACK, ulCtlReg ) )
        {
            /*
             * Clear the ack
             */
            nAck = 1;
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, ACK, ulCtlReg, 1 );
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, REQ, ulCtlReg, 0 );
            SAND_HAL_WRITE( HalCtxt, KA, EP_MM_BF_MEM_ACC_CTRL, ulCtlReg );
            break;
        }

        thin_delay( 100 );
    }

    if ( 0 == nAck )
    {
        return( SB_ELIB_IND_MEM_TIMEOUT );
    }

    return( SB_ELIB_OK );

}

sbElibStatus_et sbQe2000ElibBfMemRead( sbhandle HalCtxt,
                           uint32_t ulOffset,
                           uint32_t ulClrOnRd,
                           uint32_t *pulData0,
                           uint32_t *pulData1 )
{
    int nTimeout;
    int nAck;
    uint32_t ulCtlReg;

    SB_ASSERT( ulOffset < SB_QE2000_ELIB_BF_MEM_MAX_OFFSET );
    SB_ASSERT( pulData0 );
    SB_ASSERT( pulData1 );

    /*
     * Clear out any previous acks in the mem ctrl register & any stale data
     */
    ulCtlReg = (SAND_HAL_SET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, ACK, 1 ) |
                SAND_HAL_SET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, REQ, 0 ));
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_BF_MEM_ACC_CTRL, ulCtlReg );

    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_BF_MEM_ACC_DATA0, 0 );
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_BF_MEM_ACC_DATA1, 0 );

    /*
     * Build up our command
     */
    ulCtlReg = SAND_HAL_SET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, REQ, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, RD_WR_N, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, CLR_ON_RD, ulClrOnRd ) |
        SAND_HAL_SET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, ADDR, ulOffset );

    /*
     * Write out our command and wait for the acknowledgement
     */
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_BF_MEM_ACC_CTRL, ulCtlReg );

    nTimeout = SB_QE2000_ELIB_IND_MEM_TIMEOUT;
    nAck = 0;

    while ( nTimeout-- )
    {
        ulCtlReg = SAND_HAL_READ( HalCtxt, KA, EP_MM_BF_MEM_ACC_CTRL );
        if (1 == SAND_HAL_GET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, ACK, ulCtlReg ) )
        {
            /*
             * Grab the data & clear the ack
             */
            nAck = 1;
            *pulData0 = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EP_MM_BF_MEM_ACC_DATA0 ));
            *pulData1 = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EP_MM_BF_MEM_ACC_DATA1 ));
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, ACK, ulCtlReg, 1 );
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, REQ, ulCtlReg, 0 );
            SAND_HAL_WRITE( HalCtxt, KA, EP_MM_BF_MEM_ACC_CTRL, ulCtlReg );
            break;
        }

        thin_delay( 100 );
    }

    if ( 0 == nAck )
    {
        return( SB_ELIB_IND_MEM_TIMEOUT );
    }

    return( SB_ELIB_OK );

}



sbElibStatus_et sbQe2000ElibIpMemWrite( sbhandle HalCtxt,
                    uint32_t ulOffset,
                    uint32_t ulData0,
                    uint32_t ulData1 )
{
    int nTimeout;
    int nAck;
    uint32_t ulCtlReg;

    /*
      The proper way to bounds-check offset is to read ep_mm_config and compare depending
      on how much memory is allocated to IP. But this seems costly for a low-risk possibility.

    uint32_t ep_mm_config = SAND_HAL_READ( HalCtxt, KA, EP_MM_CONFIG);
    if (SAND_HAL_GET_FIELD( KA, EP_MM_CONFIG, ENABLE,  ep_mm_config) == 1) {
      SB_ASSERT(ulOffset < 0x8000);
    } else if (SAND_HAL_GET_FIELD( KA, EP_MM_CONFIG, ENABLE,  ep_mm_config) == 3) {
          SB_ASSERT( ulOffset < 0x4000);
    } else {
      return SB_ELIB_BAD_ARGS;
    }
    */
    SB_ASSERT( ulOffset < SB_QE2000_ELIB_IP_MEM_MAX_OFFSET );

    /*
     * Clear out any previous acks in the mem ctrl register
     */
    ulCtlReg = (SAND_HAL_SET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, ACK, 1 ) |
                SAND_HAL_SET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, REQ, 0 ));
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_IP_MEM_ACC_CTRL, ulCtlReg );

    /*
     * Build up our command
     */
    ulCtlReg = SAND_HAL_SET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, REQ, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, RD_WR_N, 0 ) |
        SAND_HAL_SET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, ADDR, ulOffset );

    /*
     * Write out our data
     */
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_IP_MEM_ACC_DATA0, BSWAP32(ulData0) );
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_IP_MEM_ACC_DATA1, BSWAP32(ulData1) );

    /*
     * Write out our command and wait for the acknowledgement
     */
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_IP_MEM_ACC_CTRL, ulCtlReg );

    nTimeout = SB_QE2000_ELIB_IND_MEM_TIMEOUT;
    nAck = 0;

    while ( nTimeout-- )
    {
        ulCtlReg = SAND_HAL_READ( HalCtxt, KA, EP_MM_IP_MEM_ACC_CTRL );
        if (1 == SAND_HAL_GET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, ACK, ulCtlReg ) )
        {
            /*
             * Clear the ack
             */
            nAck = 1;
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, ACK, ulCtlReg, 1 );
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, REQ, ulCtlReg, 0 );
            SAND_HAL_WRITE( HalCtxt, KA, EP_MM_IP_MEM_ACC_CTRL, ulCtlReg );
            break;
        }

        thin_delay( 100 );
    }

    if ( 0 == nAck )
    {
        return( SB_ELIB_IND_MEM_TIMEOUT );
    }

    return( SB_ELIB_OK );

}

sbElibStatus_et sbQe2000ElibIpMemRead( sbhandle HalCtxt,
                                       uint32_t ulOffset,
                                       uint32_t ulClrOnRd,
                                       uint32_t *pulData0,
                                       uint32_t *pulData1 )
{
    int nTimeout;
    int nAck;
    uint32_t ulCtlReg;

    SB_ASSERT( ulOffset < SB_QE2000_ELIB_IP_MEM_MAX_OFFSET );
    SB_ASSERT( pulData0 );
    SB_ASSERT( pulData1 );

    /*
     * Clear out any previous acks in the mem ctrl register & any stale data
     */
    ulCtlReg = (SAND_HAL_SET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, ACK, 1 ) |
                SAND_HAL_SET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, REQ, 0 ));
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_IP_MEM_ACC_CTRL, ulCtlReg );
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_IP_MEM_ACC_DATA0, 0 );
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_IP_MEM_ACC_DATA1, 0 );

    /*
     * Build up our command
     */
    ulCtlReg = SAND_HAL_SET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, REQ, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, CLR_ON_RD, ulClrOnRd ) |
        SAND_HAL_SET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, RD_WR_N, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, ADDR, ulOffset );

    /*
     * Write out our command and wait for the acknowledgement
     */
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_IP_MEM_ACC_CTRL, ulCtlReg );

    nTimeout = SB_QE2000_ELIB_IND_MEM_TIMEOUT;
    nAck = 0;

    while ( nTimeout-- )
    {
        ulCtlReg = SAND_HAL_READ( HalCtxt, KA, EP_MM_IP_MEM_ACC_CTRL );
        if (1 == SAND_HAL_GET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, ACK, ulCtlReg ) )
        {
            /*
             * Grab the data & clear the ack
             */
            nAck = 1;
            *pulData0 = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EP_MM_IP_MEM_ACC_DATA0 ));
            *pulData1 = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EP_MM_IP_MEM_ACC_DATA1 ));
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, ACK, ulCtlReg, 1 );
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, REQ, ulCtlReg, 0 );
            SAND_HAL_WRITE( HalCtxt, KA, EP_MM_IP_MEM_ACC_CTRL, ulCtlReg );
            break;
        }

        thin_delay( 100 );
    }

    if ( 0 == nAck )
    {
        return( SB_ELIB_IND_MEM_TIMEOUT );
    }

    return( SB_ELIB_OK );

}




sbElibStatus_et sbQe2000ElibEbMemWrite( sbhandle HalCtxt,
                    uint32_t ulOffset,
                    uint32_t ulData[8])
{
    int nTimeout;
    int nAck;
    uint32_t ulCtlReg;

    SB_ASSERT( ulOffset < (SB_QE2000_ELIB_EB_MEM_MAX_OFFSET + 1));

    /*
     * Clear out any previous acks in the mem ctrl register
     */
    ulCtlReg = (SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, ACK, 1 ) |
                SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, REQ, 0 ));
    SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_CTRL, ulCtlReg );

    /*
     * Build up our command
     */
    /* BUG 21826: Use ENTIRE mem slice section */
    ulCtlReg = SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, REQ, 1 ) |
        SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, RD_WR_N, 0 ) |
        SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, WRITE_MASK, 0xFF) |
        SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, ADDR, ulOffset );

    /*
     * Write out our data
     */
    SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_SLICE0, BSWAP32(ulData[0]) );
    SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_SLICE1, BSWAP32(ulData[1]) );
    SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_SLICE2, BSWAP32(ulData[2]) );
    SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_SLICE3, BSWAP32(ulData[3]) );
    SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_SLICE4, BSWAP32(ulData[4]) );
    SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_SLICE5, BSWAP32(ulData[5]) );
    SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_SLICE6, BSWAP32(ulData[6]) );
    SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_SLICE7, BSWAP32(ulData[7]) );

    /*
     * Write out our command and wait for the acknowledgement
     */
    SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_CTRL, ulCtlReg );

    nTimeout = SB_QE2000_ELIB_IND_MEM_TIMEOUT;
    nAck = 0;

    while ( nTimeout-- )
    {
        ulCtlReg = SAND_HAL_READ( HalCtxt, KA, EB_MEM_ACC_CTRL );
        if (1 == SAND_HAL_GET_FIELD( KA, EB_MEM_ACC_CTRL, ACK, ulCtlReg ) )
        {
            /*
             * Clear the ack
             */
            nAck = 1;
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EB_MEM_ACC_CTRL, ACK, ulCtlReg, 1 );
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EB_MEM_ACC_CTRL, REQ, ulCtlReg, 0 );
            SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_CTRL, ulCtlReg );
            break;
        }

        thin_delay( 100 );
    }

    if ( 0 == nAck )
    {
        return( SB_ELIB_IND_MEM_TIMEOUT );
    }

    return( SB_ELIB_OK );

}

sbElibStatus_et sbQe2000ElibEbMemRead( sbhandle HalCtxt,
                   uint32_t ulOffset,
                   uint32_t pulData[8])
{
    int nTimeout;
    int nAck;
    uint32_t ulCtlReg;

    SB_ASSERT( ulOffset < (SB_QE2000_ELIB_EB_MEM_MAX_OFFSET+1) );

    /*
     * Clear out any previous acks in the mem ctrl register
     */
    ulCtlReg = (SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, ACK, 1 ) |
                SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, REQ, 0 ));
    SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_CTRL, ulCtlReg );

    /*
     * Build up our command
     */
    ulCtlReg = SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, REQ, 1 ) |
        SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, RD_WR_N, 1 ) |
        SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, ADDR, ulOffset );

    /*
     * Write out our command and wait for the acknowledgement
     */
    SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_CTRL, ulCtlReg );

    nTimeout = SB_QE2000_ELIB_IND_MEM_TIMEOUT;
    nAck = 0;

    while ( nTimeout-- )
    {
        ulCtlReg = SAND_HAL_READ( HalCtxt, KA, EB_MEM_ACC_CTRL );
        if (1 == SAND_HAL_GET_FIELD( KA, EB_MEM_ACC_CTRL, ACK, ulCtlReg ) )
        {
            /*
             * Grab the data & clear the ack
             */
            nAck = 1;
            pulData[0] = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EB_MEM_ACC_SLICE0 ));
            pulData[1] = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EB_MEM_ACC_SLICE1 ));
            pulData[2] = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EB_MEM_ACC_SLICE2 ));
            pulData[3] = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EB_MEM_ACC_SLICE3 ));
            pulData[4] = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EB_MEM_ACC_SLICE4 ));
            pulData[5] = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EB_MEM_ACC_SLICE5 ));
            pulData[6] = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EB_MEM_ACC_SLICE6 ));
            pulData[7] = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EB_MEM_ACC_SLICE7 ));

            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EB_MEM_ACC_CTRL, ACK, ulCtlReg, 1 );
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EB_MEM_ACC_CTRL, REQ, ulCtlReg, 0 );
            SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_CTRL, ulCtlReg );
            break;
        }

        thin_delay( 100 );
    }

    if ( 0 == nAck )
    {
        return( SB_ELIB_IND_MEM_TIMEOUT );
    }

    return( SB_ELIB_OK );

}

sbElibStatus_et sbQe2000ElibMvtEntryReadRaw( sbhandle HalCtxt, uint32_t ulIndex, uint32_t pulData[8] )
{
    /* BUG 21826: change to volatile */
    volatile uint32_t nGrpSize;
    volatile uint32_t ulRow;
    volatile uint32_t ulColumn;
    int nStatus;


    nGrpSize = SAND_HAL_READ( HalCtxt, KA, EG_MC_CONFIG0 );
    nGrpSize = SAND_HAL_GET_FIELD( KA, EG_MC_CONFIG0, MCGROUP_SIZE, nGrpSize );
    ulRow = SB_QE2000_ELIB_EB_MVT_ROW_GET(ulIndex, nGrpSize);
    ulColumn = SB_QE2000_ELIB_EB_MVT_COL_GET(ulIndex, nGrpSize);

    SB_ASSERT( ulIndex < SB_QE2000_ELIB_EB_MVT_MIN(nGrpSize));
    SB_ASSERT( ulColumn < 3 );


    nStatus = sbQe2000ElibEbMemRead( HalCtxt, ulRow, pulData );

    return( nStatus );

}

sbElibStatus_et sbQe2000ElibMvtEntryWriteRaw( sbhandle HalCtxt, uint32_t ulIndex, uint32_t pulData[8] )
{
    /* BUG 21826: change to volatile */
    volatile uint32_t nGrpSize;
    volatile uint32_t ulRow;
    volatile uint32_t ulColumn;
    int nStatus;


    nGrpSize = SAND_HAL_READ( HalCtxt, KA, EG_MC_CONFIG0 );
    nGrpSize = SAND_HAL_GET_FIELD( KA, EG_MC_CONFIG0, MCGROUP_SIZE, nGrpSize );
    ulRow = SB_QE2000_ELIB_EB_MVT_ROW_GET(ulIndex, nGrpSize);
    ulColumn = SB_QE2000_ELIB_EB_MVT_COL_GET(ulIndex, nGrpSize);

    SB_ASSERT( ulIndex < SB_QE2000_ELIB_EB_MVT_MIN(nGrpSize));
    SB_ASSERT( ulColumn < 3 );

    nStatus = sbQe2000ElibEbMemWrite( HalCtxt, ulRow, pulData );

    return( nStatus );

}
