/*
 * $Id: qe2000_util.c 1.4 Broadcom SDK $
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
 * ============================================================
 * == qe2000_util.c - QE util routines                       ==
 * ============================================================
 *
 */

#include "sbWrappers.h"
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ka_auto.h>
#include <soc/sbx/sandHalFrameHwQe2000FMVT.h>
#include <soc/sbx/fabric/sbZfHwQe2000QsPriLutEntry.hx>

#ifdef BCM_QE2000_SUPPORT

#include <soc/sbx/qe2000.h>
#include <soc/sbx/qe2000_util.h>

#include <assert.h>

/*****************************************************************************/
int hwQe2000MvtEntryReadRaw( sbhandle userDeviceHandle, uint32_t uIndex, uint32_t puData[8] );
int hwQe2000MvtEntryWriteRaw( sbhandle userDeviceHandle, uint32_t uIndex, uint32_t puData[8] );
/*****************************************************************************/


/*****************************************************************************
 * FUNCTION NAME:  hwQe2000MvtEntryReadRaw
 *
 * OVERVIEW:       Write QE2000 EB Memory.
 *
 * ARGUMENTS:      userDeviceHandle - handle to access the device.
 *                 uOffset - offset into the memory.
 *                 puData - pointer to the data to write.
 *
 * RETURNS:        0 on success
 *
 *
 * DESCRIPTION:   Write a location in EB memory.
 *
 *****************************************************************************/
int hwQe2000MvtEntryReadRaw( sbhandle userDeviceHandle, uint32_t uIndex, uint32_t puData[8] )
{
    int nGrpSize;
    int nStatus;
    uint32_t ulRow;
    uint32_t ulColumn;

    /*SB_ASSERT( userDeviceHandle ); */

    nGrpSize = SAND_HAL_READ( userDeviceHandle, KA, EG_MC_CONFIG0 );
    nGrpSize = SAND_HAL_GET_FIELD( KA, EG_MC_CONFIG0, MCGROUP_SIZE, nGrpSize );
    ulRow = HW_QE2000_EB_MVT_ROW_GET(uIndex, nGrpSize);
    ulColumn = HW_QE2000_EB_MVT_COL_GET(uIndex, nGrpSize);

    SB_ASSERT( uIndex < HW_QE2000_EB_MVT_MIN(nGrpSize));
    SB_ASSERT( ulColumn < 3 );


    nStatus = soc_qe2000_eb_mem_read( (int)userDeviceHandle, ulRow, puData );

    return( nStatus );

}
/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:       arg1 -
 *	            arg2 -
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

int hwQe2000MvtEntryWriteRaw( sbhandle userDeviceHandle, uint32_t uIndex, uint32_t puData[8] )
{
    int nGrpSize;
    int nStatus;
    uint32_t ulRow;
    uint32_t ulColumn;

    /* SB_ASSERT( userDeviceHandle ); */

    nGrpSize = SAND_HAL_READ( userDeviceHandle, KA, EG_MC_CONFIG0 );
    nGrpSize = SAND_HAL_GET_FIELD( KA, EG_MC_CONFIG0, MCGROUP_SIZE, nGrpSize );
    ulRow = HW_QE2000_EB_MVT_ROW_GET(uIndex, nGrpSize);
    ulColumn = HW_QE2000_EB_MVT_COL_GET(uIndex, nGrpSize);

    SB_ASSERT( uIndex < HW_QE2000_EB_MVT_MIN(nGrpSize));
    SB_ASSERT( ulColumn < 3 );

    nStatus = soc_qe2000_eb_mem_write( (int)userDeviceHandle, ulRow, puData );

    return( nStatus );

}

/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:       arg1 -
 *	            arg2 -
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

int hwQe2000MVTSet( sbhandle                 pHalCtx,
		    HW_QE2000_MVT_ENTRY_ST   Entry,
		    uint32_t                 uIndex,
		    HW_QE2000_SEM_TRYWAIT_PF pfSemTryWait,
		    HW_QE2000_SEM_GIVE_PF    pfSemGive,
		    int                      nSemId,
		    uint32_t                 ulTimeOut,
		    void                    *pUserData)
{
    sandHalFrameHwQe2000FMVT_t sZfFMVT;
    uint64_t ullPortMask;
    int nPort;
    int nStatus;
    uint32_t aulMvtTuple[SAND_HAL_FRAME_HW_QE2000_FMVT_ENTRY_SIZE_IN_BYTES / sizeof(uint32_t)];
    int nGrpSize;
    int nColumn;

    /* SB_ASSERT(pHalCtx); */

    if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
    {
        nStatus = (*pfSemTryWait)(nSemId, pUserData, ulTimeOut);
        if( 0 != nStatus)
        {
            return( nStatus );
        }
    }

    sal_memset( &sZfFMVT, 0, sizeof(sZfFMVT) );
    /*
     * Get the column info
     */
    nGrpSize = SAND_HAL_READ( pHalCtx, KA, EG_MC_CONFIG0 );
    nGrpSize = SAND_HAL_GET_FIELD( KA, EG_MC_CONFIG0, MCGROUP_SIZE, nGrpSize );
    nColumn = HW_QE2000_EB_MVT_COL_GET(uIndex, nGrpSize);
    SB_ASSERT( nColumn < 3 );
    /*
     * Get the MVT row
     */
    nStatus = hwQe2000MvtEntryReadRaw( pHalCtx, uIndex, aulMvtTuple );
    if( 0 != nStatus )
    {
        if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
        {
            (*pfSemGive)(nSemId, pUserData);
        }
        return( -1 );
    }

    sandHalFrameHwQe2000FMVT_Unpack(&sZfFMVT,
				    (uint8_t *)aulMvtTuple,
				    SAND_HAL_FRAME_HW_QE2000_FMVT_ENTRY_SIZE_IN_BYTES);

    ullPortMask = 0;
    for( nPort = 0; nPort < HW_QE2000_MAX_NUM_PORTS_K; nPort++ )
    {
        if( TRUE == Entry.bPortEnable[nPort] )
        {
            ullPortMask |= (1ULL << nPort);
        }
    }

    switch( nColumn )
    {
        case 0:
            sZfFMVT.m_nnPortMap0 = ullPortMask;
            sZfFMVT.m_nMvtda0 = Entry.ulMvtdA;
            sZfFMVT.m_nMvtdb0 = Entry.ulMvtdB;
            sZfFMVT.m_nNext0 = Entry.ulNext;
            if( TRUE == Entry.bSourceKnockout )
            {
                sZfFMVT.m_nKnockout0 = 1;
            } else {
                sZfFMVT.m_nKnockout0 = 0;
            }
            break;

        case 1:
            sZfFMVT.m_nnPortMap1 = ullPortMask;
            sZfFMVT.m_nMvtda1 = Entry.ulMvtdA;
            sZfFMVT.m_nMvtdb1 = Entry.ulMvtdB;
            sZfFMVT.m_nNext1 = Entry.ulNext;
            if( TRUE == Entry.bSourceKnockout )
            {
                sZfFMVT.m_nKnockout1 = 1;
            } else {
                sZfFMVT.m_nKnockout1 = 0;
            }
            break;

        case 2:
            sZfFMVT.m_nnPortMap2 = ullPortMask;
            sZfFMVT.m_nMvtda2 = Entry.ulMvtdA;
            sZfFMVT.m_nMvtdb2 = Entry.ulMvtdB;
            sZfFMVT.m_nNext2 = Entry.ulNext;

            if( TRUE == Entry.bSourceKnockout )
            {
                sZfFMVT.m_nKnockout2 = 1;
            } else {
                sZfFMVT.m_nKnockout2 = 0;
            }
            break;

        default:
            if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
            {
                (*pfSemGive)(nSemId, pUserData);
            }
            return( -2 ); /* should have SB_ASSERTed above */
    }

    (void)sandHalFrameHwQe2000FMVT_Pack( &sZfFMVT,
					 (uint8_t *)aulMvtTuple,
					 0 );

    nStatus = hwQe2000MvtEntryWriteRaw( pHalCtx, uIndex, aulMvtTuple );
    if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
    {
        (*pfSemGive)(nSemId, pUserData);
    }

    if( 0 != nStatus )
    {
        return( -1 );
    }

    return( 0 );
}

/*****************************************************************************
 * FUNCTION NAME:
 *
 * OVERVIEW:
 *
 * ARGUMENTS:       arg1 -
 *	            arg2 -
 *
 * RETURNS:
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * ASSUMPTIONS:
 *
 * SIDE EFFECTS:
 *
 *****************************************************************************/

int hwQe2000MVTGet( sbhandle pHalCtx,
		    HW_QE2000_MVT_ENTRY_PST pEntry,
		    uint32_t uIndex,
		    HW_QE2000_SEM_TRYWAIT_PF pfSemTryWait,
		    HW_QE2000_SEM_GIVE_PF pfSemGive,
		    int nSemId,
		    uint32_t ulTimeOut,
		    void *pUserData)
{
    sandHalFrameHwQe2000FMVT_t sZfFMVT;
    int nPort;
    int nStatus;
    uint32_t aulMvtTuple[SAND_HAL_FRAME_HW_QE2000_FMVT_ENTRY_SIZE_IN_BYTES / sizeof(uint32_t)];
    uint64_t ullPortMask;
    int nGrpSize;
    int nColumn;

    /* SB_ASSERT( pHalCtx ); */
    SB_ASSERT( pEntry );

    if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
    {
        nStatus = (*pfSemTryWait)(nSemId, pUserData, ulTimeOut);
        if( 0 != nStatus)
        {
            return( nStatus );
        }
    }

    sal_memset( &sZfFMVT, 0, sizeof(sZfFMVT) );
    sal_memset( pEntry, 0, sizeof(*pEntry) );

    nGrpSize = SAND_HAL_READ( pHalCtx, KA, EG_MC_CONFIG0 );
    nGrpSize = SAND_HAL_GET_FIELD( KA, EG_MC_CONFIG0, MCGROUP_SIZE, nGrpSize );
    nColumn = HW_QE2000_EB_MVT_COL_GET(uIndex, nGrpSize);
    SB_ASSERT( nColumn < 3 );
    /*
     * Get the MVT row
     */
    nStatus = hwQe2000MvtEntryReadRaw( pHalCtx, uIndex, aulMvtTuple );
    if( 0 != nStatus )
    {
        if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
        {
            (*pfSemGive)(nSemId, pUserData);
        }
        return( -1 );
    }

    sandHalFrameHwQe2000FMVT_Unpack(&sZfFMVT,
                                        (uint8_t *)aulMvtTuple,
                                        SAND_HAL_FRAME_HW_QE2000_FMVT_ENTRY_SIZE_IN_BYTES);

    switch( nColumn )
    {
        case 0:
            ullPortMask = sZfFMVT.m_nnPortMap0;
            pEntry->ulMvtdA = sZfFMVT.m_nMvtda0;
            pEntry->ulMvtdB = sZfFMVT.m_nMvtdb0;
            pEntry->ulNext  = sZfFMVT.m_nNext0;
            if( TRUE == sZfFMVT.m_nKnockout0 )
            {
                pEntry->bSourceKnockout = 1;
            }
            break;

        case 1:
            ullPortMask = sZfFMVT.m_nnPortMap1;
            pEntry->ulMvtdA = sZfFMVT.m_nMvtda1;
            pEntry->ulMvtdB = sZfFMVT.m_nMvtdb1;
            pEntry->ulNext  = sZfFMVT.m_nNext1;
            if( TRUE == sZfFMVT.m_nKnockout1 )
            {
                pEntry->bSourceKnockout = 1;
            }
            break;

        case 2:
            ullPortMask = sZfFMVT.m_nnPortMap2;
            pEntry->ulMvtdA = sZfFMVT.m_nMvtda2;
            pEntry->ulMvtdB = sZfFMVT.m_nMvtdb2;
            pEntry->ulNext  = sZfFMVT.m_nNext2;
            if( TRUE == sZfFMVT.m_nKnockout2 )
            {
                pEntry->bSourceKnockout = 1;
            }
            break;

        default:
            if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
            {
                (*pfSemGive)(nSemId, pUserData);
            }
            return( -2 ); /* should have SB_ASSERTed above */
    }

    for( nPort = 0; nPort < HW_QE2000_MAX_NUM_PORTS_K; nPort++ )
    {
        if(ullPortMask & ((uint64_t)1 << nPort) )
        {
            pEntry->bPortEnable[nPort] = TRUE;
        }
        else
        {
            pEntry->bPortEnable[nPort] = FALSE;
        }
    }

    if( (NULL != pfSemTryWait) && (NULL != pfSemGive) && (0 != nSemId) )
    {
        (*pfSemGive)(nSemId, pUserData);
    }

    return( 0 );
}

#endif /* BCM_QE2000_SUPPORT */
