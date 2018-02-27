#ifndef __SBQE2000ELIBCONTEXT_H__
#define __SBQE2000ELIBCONTEXT_H__
/**
 * <pre>
 * ======================================================================
 * ==  sbQe2000ElibContext.h - Private declaration of the elib context struct  ==
 * ======================================================================
 *
 * WORKING REVISION: $Id: sbQe2000ElibContext.h 1.3.158.1 Broadcom SDK $
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
 *
 * MODULE NAME:
 *
 *     sbQe2000ElibContext.h
 *
 * ABSTRACT:
 *
 *     This is the private declaration of the elib context struct.  The user
 *     gets an opaque handle to this structure in the form of an SB_EP_HANDLE
 *     that is defined in sbQe2000Elib.h.  The user always uses the opaque handle to
 *     access the elib API.  This file defines the internal structure of the
 *     elib context, and is private to the elib.
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
 *     25-August-2004
 *
 * </pre>
 */
#include "glue.h"
#include "sbQe2000Elib.h"
#include "sbQe2000ElibZf.h"
#include "sbWrappers.h"
#include <shared/avl.h>

#if defined(DENTER)
#undef DENTER
#endif

#if defined(DEXIT)
#undef DEXIT
#endif

#if defined (DEBUG_PRINT)
#define DENTER() SB_LOG("%s: enter\n", __PRETTY_FUNCTION__);
#define DEXIT()  SB_LOG("%s: line: %d:  exit\n", __PRETTY_FUNCTION__, __LINE__);
#else
#define DENTER()
#define DEXIT()
#endif


#define SBQE2000_ELIB_SCIDX_MAX_K     (4)          /* Up to MAX # of scidx's attached to a vrt */
#define SBQE2000_ELIB_VRT_MAP_MAX_K   (4095)       /* Max # of VRT Map Entries, 0xFFF is used by PCI */
#define SBQE2000_ELIB_SCIDX_VALID_M   (0x8000)     /* Marks the scidx as valid */
#define SBQE2000_ELIB_SCIDX_M         (0x0FFF)




/**
 * Switch Context Record Memory Management
 */
typedef struct sbqe2000bfmemavl_s
{
    int nNumLines;                     /**< Number of lines in this block */
    uint32_t ulAddr;                   /**< BF Mem Address of this block */
    struct sbqe2000bfmemavl_s *pNext;  /**< Next element in the DLL */
    struct sbqe2000bfmemavl_s *pPrev;  /**< Previous element in the DLL */
} sbQe2000ElibBfMemAvl_st, *sbQe2000ElibBfMemAvl_pst;

/**
 * VLAN Indirection Table (VIT) OP field encoding
 */
typedef enum sb_qe2000_elib_vit_op_t {
    SB_QE2000_ELIB_VIT_OP_DROP_NOTIFY = 0,   /**< Drop frame and generate notification. */
    SB_QE2000_ELIB_VIT_OP_DROP_NO_NOTIFY,    /**< Drop frame and do not notification.   */
    SB_QE2000_ELIB_VIT_OP_FWD_NO_TAG,        /**< Forward frame without tag.            */
    SB_QE2000_ELIB_VIT_OP_FWD_TAG,           /**< Forward frame with tag.               */
    SB_QE2000_ELIB_VIT_OP_MAX
} SB_QE2000_ELIB_VIT_OP_T, *SB_QE2000_ELIB_VIT_OP_PT;


typedef uint32_t VrtPtr_t;

/**
 * Switch Context Entry Information Structure
 */
typedef struct sb_qe2000_elib_vrt_map_entry_s {
    VrtPtr_t tVrtPtr;                              /**< Address of the SCR */
    sbZfSbQe2000ElibVRT_t sZfVrt;          /**< ZFrame representation of the PCW */
    uint16_t auwScIdx[SBQE2000_ELIB_SCIDX_MAX_K];  /**< Switch Context Indices associated with this SCR */
    uint32_t ulNumPorts;                           /**< Number of ports that are enabled for this SCR */
    uint32_t ulVID;                                /**< VLAN ID for this SCR */
    uint64_t ullPortEnable;                        /**< Port Enable Mask */
    int      nNumLines;                            /**< Number of BF Mem lines for this SCR */
    bool_t   bDummyLine;                           /**< Dummy line within the SCR? */
    SB_QE2000_ELIB_VIT_CMAP_T tCmap;               /**< Counter map for the SCR */
    SB_QE2000_ELIB_COUNTER_PST pCntrShadow;        /**< Acuumulated counters */
} SB_QE2000_ELIB_VRT_MAP_ENTRY_ST, *SB_QE2000_ELIB_VRT_MAP_ENTRY_PST;



/*
 * elib context struct
 */
typedef struct sb_ep_context_s {
    void                             *pHalCtx;       /**< Handle the the glue layer */
    int                              nPorts;         /**< Number of ports within the system */
    int                              nOnePcwEnable;  /**< Number of Port Control Words in BF VRT Entry */
    SB_QE2000_ELIB_USER_DMA_PF       pfUserDmaFunc;  /**< Pointer to the user supplied DMA function */
    void*                            pUserData;      /**< Pointer to the user supplied DMA function data */
    SB_QE2000_ELIB_PORT_CFG_ST       asPortConfig[SB_QE2000_ELIB_NUM_PORTS_K];      /**< Saved port configurations */
    sbZfSbQe2000ElibPCT_t    asPCT[SB_QE2000_ELIB_NUM_PORTS_K];             /**< Per port class counters */
    sbZfSbQe2000ElibCRT_t    asCRT[SB_QE2000_ELIB_NUM_CRT_ENTRIES_K / SB_QE2000_ELIB_NUM_CLASSES_K];      /**< Saved Class Resolution Table */
    bool_t                           abCRTDirty[SB_QE2000_ELIB_NUM_CRT_ENTRIES_K / SB_QE2000_ELIB_NUM_CLASSES_K]; /**< Class Resolution Table Dirty Indicator */
    sbZfSbQe2000ElibCIT_t    asCIT[SB_QE2000_ELIB_NUM_CLASSES_K];           /**< Saved Class Instruction Table */
    uint64_t                         aullClassPktCnt[SB_QE2000_ELIB_NUM_CLASSES_K]; /**< Saved Per Class Packet Count */
    int                              nCMapClassCnt[SB_QE2000_ELIB_NUM_CMAPS_K];     /**< Number of counters in CMAP 0 */
    SB_QE2000_ELIB_CMAP_ST           sCmap[SB_QE2000_ELIB_NUM_CMAPS_K];             /**< Copy of the counter maps */
    SB_QE2000_ELIB_VRT_MAP_ENTRY_PST apVrtMap[SBQE2000_ELIB_VRT_MAP_MAX_K];         /**< Array of pointers to VRTMap Entries */
    int                              nLastScIdxAccum;   /**< Last scidx counts were accumulated for */
    SB_QE2000_ELIB_SEM_CREATE_PF     pfUserSemCreate;   /**< User supplied sem create function */
    SB_QE2000_ELIB_SEM_TRYWAIT_PF    pfUserSemTryWait;  /**< User supplied sem try & wait function */
    SB_QE2000_ELIB_SEM_GIVE_PF       pfUserSemGive;     /**< User supplied sem give function */
    SB_QE2000_ELIB_SEM_DESTROY_PF    pfUserSemDestroy;  /**< User supplied sem destroy function */
    void*                            pUserSemData;      /**< User supplied sem data passed to semaphore functions */
    int                              nStatsSemId;       /**< Statistics Semaphore ID */
    void*                            pMVTUserSemData;   /**< User supplied semaphore void pointer for use with the MVT */
    int                              nMVTSemId;         /**< Semaphore ID supplied by the user to negogiate access to the MVT */
    int                              nMVTTimeOut;       /**< Time out value for the semaphore try & wait for use with the MVT semaphore */
    shr_avl_t                        *sBfMem;           /**< AVL Tree for VLAN Memory Management */
    sbQe2000ElibBfMemAvl_pst pBfMemList;                /**< DLL Root for VLAN Memory Management */

} SB_QE2000_ELIB_CONTEXT_ST, *SB_QE2000_ELIB_CONTEXT_PST;


/*
 * Private Internal Functions
 */
sbElibStatus_et sbQe2000ElibSemGet( SB_QE2000_ELIB_CONTEXT_PST pEp, uint32_t nTimeOut );
sbElibStatus_et sbQe2000ElibSemGive( SB_QE2000_ELIB_CONTEXT_PST pEp );
sbElibStatus_et sbQe2000ElibVITPciSet( SB_QE2000_ELIB_HANDLE Handle );

sbElibStatus_et sbQe2000ElibVlanMemInit(SB_QE2000_ELIB_CONTEXT_PST pEp);
sbElibStatus_et sbQe2000ElibVlanMemUninit(SB_QE2000_ELIB_CONTEXT_PST pEp);
sbElibStatus_et sbQe2000ElibVlanMalloc(SB_QE2000_ELIB_CONTEXT_PST pEp, int nNumLines, VrtPtr_t *ptVrtPtr);
sbElibStatus_et sbQe2000ElibVlanRealloc(SB_QE2000_ELIB_CONTEXT_PST pEp, int nNumLines, VrtPtr_t *ptVrtPtr);

sbElibStatus_et sbQe2000ElibVlanFree(SB_QE2000_ELIB_CONTEXT_PST pEp, int nNumLines, VrtPtr_t *ptVrtPtr);

sbElibStatus_et sbQe2000ElibVlanMemset(SB_QE2000_ELIB_CONTEXT_PST pEp, VrtPtr_t *ptVrtPtr,
                           uint32_t ulData0, uint32_t ulData1, int nNumLines);

sbElibStatus_et sbQe2000ElibVlanCountReset( SB_QE2000_ELIB_CONTEXT_PST pEp );
#endif
