/* -*- mode:c++; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=2 cinoptions=\:0l1(0t0g0: */
/*
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
 *$Id: sbFe2000Pm.h 1.9.60.1 Broadcom SDK $
 *-----------------------------------------------------------------------------*/
#ifndef _SB_FE_2000_POLICER_H_
#define _SB_FE_2000_POLICER_H_

#include <sal/types.h>
#include <soc/sbx/sbDq.h>
#include <soc/sbx/fe2k_common/sbFe2000DmaMgr.h>
#include <soc/sbx/fe2k_common/sbFe2000Common.h>
#include "sbZfFe2000PmGroupConfig.hx"
#include "sbZfFe2000PmGroupConfigConsole.hx"
#include "sbZfFe2000PmOamTimerConfig.hx"
#include "sbZfFe2000PmOamTimerConfigConsole.hx"
#include "sbZfFe2000PmOamTimerRawFormat.hx"
#include "sbZfFe2000PmOamTimerRawFormatConsole.hx"
#include "sbZfFe2000PmPolicerConfig.hx"
#include "sbZfFe2000PmPolicerConfigConsole.hx"
#include "sbZfFe2000PmPolicerRawFormat.hx"
#include "sbZfFe2000PmPolicerRawFormatConsole.hx"
#include "sbZfFe2000PmProfileMemory.hx"
#include "sbZfFe2000PmProfileMemoryConsole.hx"
#include "sbZfC2PmProfileMemory.hx"
#include "sbZfC2PmProfileMemoryConsole.hx"
#include "sbZfFe2000PmProfilePolicerMemory.hx"
#include "sbZfFe2000PmProfilePolicerMemoryConsole.hx"
#include "sbZfC2PmProfilePolicerMemory.hx"
#include "sbZfC2PmProfilePolicerMemoryConsole.hx"
#include "sbZfFe2000PmProfileSeqGenMemory.hx"
#include "sbZfFe2000PmProfileSeqGenMemoryConsole.hx"
#include "sbZfFe2000PmProfileTimerMemory.hx"
#include "sbZfFe2000PmProfileTimerMemoryConsole.hx"
#include "sbZfC2PmProfileTimerMemory.hx"
#include "sbZfC2PmProfileTimerMemoryConsole.hx"
#include "sbZfFe2000PmOamSeqGenRawFormat.hx"
#include "sbZfFe2000PmOamSeqGenRawFormatConsole.hx"

#define SB_FE_2000_PM_NUM_OF_GROUPS     9
#define SB_FE_2000_PM_OAM_TIMERS_GROUP  2
#define SB_FE_2000_PM_SEQNUM_GROUP      8

#define SB_FE_2000_MAX_POLICER_IDX 65536

typedef struct sbFe2000Pm_s sbFe2000Pm_t;
/**
 * @enum sbG2FePmPolicerRate
 * BPS bits per seconds which is the 
 * standard way of describing profiles.
 */
typedef enum sbFe2000PmPolicerRate_s
{
  SB_FE_2000_PM_PRATE_1KBPS = 0,
  SB_FE_2000_PM_PRATE_10KBPS,
  SB_FE_2000_PM_PRATE_100KBPS,
  SB_FE_2000_PM_PRATE_250KBPS,
  SB_FE_2000_PM_PRATE_500KBPS,
  SB_FE_2000_PM_PRATE_1MBPS,
  SB_FE_2000_PM_PRATE_10MBPS,
  SB_FE_2000_PM_PRATE_100MBPS,
  SB_FE_2000_PM_PRATE_1GBPS,
  SB_FE_2000_PM_PRATE_10GBPS,
} sbFe2000PmPolicerRate_t;

/**
 * @struct 
 */
typedef struct sbFe2000PmConfig_s
{
  void                    *regSet;
  uint32_t                base;
  uint32_t                bank;
  void                    *token;
  sbFe2000DmaMgr_t        *dmaMgr;
  uint32_t                uNumberOfPolicers;
  uint32_t                uNumberOfPolicersPerGroup[SB_FE_2000_PM_NUM_OF_GROUPS];
  sbFe2000PmPolicerRate_t   rates[SB_FE_2000_PM_NUM_OF_GROUPS];
} sbFe2000PmConfig_t;

typedef struct sbFe2000PmGroupConfig_s
{
  sbFe2000PmPolicerRate_t      uRate;
  sbZfFe2000PmGroupConfig_t  groupConfig;
} sbFe2000PmGroupConfig_t;

/*
 * Policer  Database Internal data structures.
 */
typedef struct sbFe2000PmPolicer_s  {
  dq_t listElem;
  uint32_t uPolicerId;
  uint32_t uProfileId;
  uint32_t uGroupId;
  uint32_t uMemChunkId;
} sbFe2000PmPolicer_t;

typedef struct sbFe2000PmProfile_s  {
  dq_t listPolicers;
  sbZfFe2000PmProfileMemory_t profile;
  sbZfC2PmProfileMemory_t C2Profile;
  uint32_t uRefCount;
} sbFe2000PmProfile_t;

#define SB_FE_2000_PM_MAX_NUM_PROFILES  (2048)
#define MAX_PM_MEM_CHUNKS             (1024)

struct sbFe2000Pm_s {
  void *regSet;
  sbFe2000PmProfile_t profiledb[SB_FE_2000_PM_MAX_NUM_PROFILES];
  void            *aMemChunks[MAX_PM_MEM_CHUNKS];
  uint32_t        uChunkCount;
  uint32_t        uPolicersPerChunk;        
  uint32_t        uHppFreq;        
  dq_t            listFreePolicers;
  sbFe2000PmConfig_t PolicerConfig;
  sbFe2000DmaOp_t dmaOp;
};

/**
 * This function converts a kbit/s rate to a rate bucket
 */
int
sbFe2000PmConvertRate(int kbps);

/**
 * This function configures a policer and returns
 * the index of the created  policer id.
 * 
 * @param pPm             Pointer to the sbFe2000Pm_t struct
 * @param uPolicerId      Policer Index
 * @param *pConfig        Pointer to the Policer Configuration.
 * @param *pIndex         pointer to the policer index value
 * @return                Return status of the type sbStatus_t
 */
sbStatus_t 
sbFe2000PmPolicerCreate(sbFe2000Pm_t *pPm,
                      uint32_t ulPolicerId,
                      sbZfFe2000PmPolicerConfig_t *pConfig,
                      uint32_t *pIndex);


/**
 * This function deletes the previously configured
 * policer
 * 
 * @param pPm             Pointer to the sbFe2000Pm_t struct
 * @param uPolicerIndex   Pointer to the policer index value
 * @return                Return status of the type sbStatus_t
 */
sbStatus_t 
sbFe2000PmPolicerDelete(sbFe2000Pm_t *pPm,
                      uint32_t uPolicerIndex);

/*
 * This function modified the base rate of the Policer Group.
 * 
 * @param pPm             Pointer to the sbFe2000Pm_t struct
 * @param uGroupId        Policer Group, base rate of which needs to be modified.
 * @param uRateId         Rate ID - sbFe2000PmPolicerRate
 */
sbStatus_t
sbFe2000PmPolicerGroupRateChange( sbFe2000Pm_t *pPm,
                              uint32_t uGroupId,
                              uint32_t uRateId);

/**
 * This function gets policer details.
 * 
 * @param pPm             Pointer to the sbFe2000Pm_t struct
 * @param uPolicerId      Policer Index
 * @param *pConfig        Pointer to the Policer Configuration.
 * @return                Return status of the type sbStatus_t
 */
sbStatus_t
sbFe2000PmPolicerRead(sbFe2000Pm_t *pPm,
                      uint32_t ulPolicerId,
                      sbZfFe2000PmPolicerConfig_t *pConfig);

sbStatus_t
sbFe2000PmPolicerReadExt(sbFe2000Pm_t *pPm,
                         uint32_t ulPolicerId,
                         sbZfFe2000PmPolicerConfig_t *pConfig,
                         uint32_t *pProfileId);


/*
 * This function gets the Policer Group Information.
 * 
 * @param pPm             Pointer to the sbFe2000Pm_t struct
 * @param uGroupId        Policer Group
 * @param *pConfig        Pointer to the Policer Group Configuration.
 */
sbStatus_t
sbFe2000PmPolicerGroupRead( sbFe2000Pm_t *pPm,
                          uint32_t uGroupId,
                          sbFe2000PmGroupConfig_t *pConfig);



/************************** OAM ENTITIES ******************************/


/**
 * This function configures a policer and returns
 * the index of the created  policer id.
 * 
 * @param pPm             Pointer to the sbFe2000Pm_t struct
 * @param uPolicerId      Timer Index
 * @param *pConfig        Pointer to the Timer Configuration.
 * @param *pIndex         pointer to the policer index value
 * @return                Return status of the type sbStatus_t
 */
sbStatus_t
sbFe2000PmOamTimerCreate( sbFe2000Pm_t     *pPm,
                        uint32_t      ulTimerId,
                        sbZfFe2000PmOamTimerConfig_t *pConfig,
                        uint32_t *pIndex);

/**
 * This function deletes a timer
 * 
 * @param pPm             Pointer to the sbFe2000Pm_t struct
 * @param uPolicerId      Timer Index
 * @return                Return status of the type sbStatus_t
 */
sbStatus_t
sbFe2000PmOamTimerDelete( sbFe2000Pm_t     *pPm,
                        uint32_t      uTimerId );

/**
 * This function gets timer details.
 * 
 * @param pPm             Pointer to the sbFe2000Pm_t struct
 * @param uPolicerId      Timer Index
 * @param *pConfig        Pointer to the Timer Configuration.
 * @return                Return status of the type sbStatus_t
 */
sbStatus_t
sbFe2000PmOamTimerRead(sbFe2000Pm_t *pPm,
                     uint32_t ulTimerId,
                     sbZfFe2000PmOamTimerConfig_t *pConfig);

/*
 * This function gets the Timer Group Information.
 * 
 * @param pPm             Pointer to the sbFe2000Pm_t struct
 * @param uGroupId        Timer Group
 * @param *pConfig        Pointer to the Timer Group Configuration.
 */
sbStatus_t
sbFe2000PmOamTimerGroupRead( sbFe2000Pm_t *pPm,
                        uint32_t uGroupId,
                        sbFe2000PmGroupConfig_t *pConfig);


/**
 * This function configures a Sequence Generator and returns
 * the index of the created  id.
 * 
 * @param pPm             Pointer to the sbFe2000Pm_t struct
 * @param uPolicerId      Policer Index
 * @param *pConfig        Pointer to the Sequence Generator Configuration.
 * @param *pIndex         pointer to the policer index value
 * @return                Return status of the type sbStatus_t
 */
sbStatus_t
sbFe2000PmOamSeqGenCreate( sbFe2000Pm_t     *pPm,
                         uint32_t      uSeqGenId,
                         uint32_t     *pIndex);

/**
 * This function deletes a Sequence Generator and returns
 * 
 * @param pPm             Pointer to the sbFe2000Pm_t struct
 * @param uPolicerId      Policer Index
 * @return                Return status of the type sbStatus_t
 */
sbStatus_t
sbFe2000PmOamSeqGenDelete( sbFe2000Pm_t     *pPm,
                         uint32_t      uSeqGenId);

/*
 * This function gets the Sequence Generator Group Information.
 * 
 * @param pPm             Pointer to the sbFe2000Pm_t struct
 * @param uGroupId        Timer Group
 * @param *pConfig        Pointer to the Timer Group Configuration.
 */
sbStatus_t
sbFe2000PmOamSeqGenGroupRead(sbFe2000Pm_t *pPm,
                        uint32_t uGroupId,
                        sbFe2000PmGroupConfig_t *pConfig);



sbStatus_t
sbFe2000PmOamTimerRawGet(sbFe2000Pm_t *pPm,
                      uint32_t ulIndex,
                      sbZfFe2000PmOamTimerRawFormat_t *pPrm);
sbStatus_t
sbFe2000PmOamTimerRawSet(sbFe2000Pm_t *pPm,
                        uint32_t ulIndex,
                        sbZfFe2000PmOamTimerRawFormat_t *pPrm);
/**
 * Initialize the Policer Management Unit
 *
 * @param pPm         Pointer to sbFe2000Pm_t struct
 * @return       Return Staus of type sbStatus_t
 */

sbStatus_t 
sbFe2000PmInit(uint32_t uHppFreq, sbFe2000Pm_t **pPm, sbFe2000PmConfig_t *pConfig);

/**
 * UnInitialize the Policer Management Unit
 *
 * @param pPm         Pointer to sbFe2000Pm_t struct
 * @return       Return Staus of type sbStatus_t
 */

sbStatus_t 
sbFe2000PmUnInit(sbFe2000Pm_t *pPm);


sbStatus_t
sbFe2000PmRecoverPolicer(sbFe2000Pm_t *pPm, uint32_t policerId,
                         uint32_t profileId);

void
sbFe2000PmSwDump(sbFe2000Pm_t *pPm);

#endif
