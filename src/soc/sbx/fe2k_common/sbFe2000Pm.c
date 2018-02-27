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
 * $Id: sbG2FePm.c,v 1.1.4.13 2007/07/10 01:07:43 lbush Exp
 *
 *-----------------------------------------------------------------------------*/
#include "sbTypes.h"
#include "sbTypesGlue.h"
#include "sbWrappers.h"
#include "glue.h"
#include "sbFe2000CommonDriver.h"
#include "hal_ca_auto.h"
#include "sbFe2000Pm.h"
#include "sbZfFe2000PmGroupConfig.hx"
#include "sbZfFe2000PmGroupConfigConsole.hx"
#include "sbZfC2PmProfileSeqGenMemory.hx"
#include "sbZfFe2000PmPolicerRawFormat.hx"
#ifndef __KERNEL__
#include <math.h>
#endif

#include <soc/drv.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_c2_auto.h>
#include <soc/sbx/hal_ca_c2.h>
#include <soc/sbx/sbx_drv.h>

#define PUBLIC  
#define PRIVATE  static


#define SB_FE_2000_PM_MAX_REFRESHES           (64)
#define SB_FE_2000_PM_REFRESH_THRESH          16    /* per-group threshold. Bigger than needed but no harm */
#define SB_FE_2000_PM_TOTAL_REFRESH_THRESH    (3*16)
#define SB_FE_2000_PM_NUM_PROF_GROUPS         (7)
#define SB_FE_2000_PM_MM_SEL0                 (24) /* bit0 is zero */
#define SB_FE_2000_PM_MM_SEL1                 (23) /* bit1 is one */
#define SB_FE_2000_PM_NPU_CLOCK_FREQ          (250.0E6)
#define SB_FE_2000_PM_NPU_CLOCK_PERIOD_NS     (1e9/SB_FE_2000_PM_NPU_CLOCK_FREQ)
/*
#define SB_FE_2000XT_PM_NPU_CLOCK_FREQ        (375.0E6)
#define SB_FE_2000XT_PM_NPU_CLOCK_PERIOD_NS   (1e9/SB_FE_2000XT_PM_NPU_CLOCK_FREQ)
*/
#define SB_FE_2000_PM_NULL_PROFILE_ID         (0)
#define SB_FE_2000_PM_POLICER_BASE            (0)
#define SB_FE_2000_PM_ENABLE_REFRESH          (0)
#define SB_FE_2000_PM_DISABLE_REFRESH         (1)
#define SB_FE_2000_PM_MIN_MANTISSA            (0)
#define SB_FE_2000_PM_MAX_MANTISSA            (127)

#define SB_FE_2000_PM_NUM_TOTAL_GROUPS        (9)
#define SB_FE_2000_PM_MTU                     (1500)
#define SB_FE_2000_PM_OAM_TIMER_TICK_NS       (10e6)

#define SB_FE_2000_PM_PORT_POLICER_GROUP_ID      (0)
#define SB_FE_2000_PM_TYPED_POLICER_GROUP_BASE   (0)
typedef enum
{
  SB_FE_2000_PM_METERTYPE_POLICER     = 0,
  SB_FE_2000_PM_METERTYPE_TIMER       = 1,
  SB_FE_2000_PM_METERTYPE_GENERATOR   = 2,
  SB_FE_2000_PM_METERTYPE_SEMAPHORE   = 3,
} sbFe2000PmMeterType_t;



/*****************************************************
 * Policer Management Local Functions
 ****************************************************/

PRIVATE sbStatus_t
sbFe2000PolicerRemove(sbFe2000Pm_t *pPm, 
		                sbFe2000PmPolicer_t *pPolicer);

PRIVATE sbStatus_t
sbFe2000ProfileCreate(sbFe2000Pm_t *pPm,
                    uint32_t uProfileId, 
                    sbZfFe2000PmProfileMemory_t *pProfile);

PRIVATE sbStatus_t
sbFe2000XtProfileCreate(sbFe2000Pm_t *pPm,
                    uint32_t uProfileId, 
                    sbZfC2PmProfileMemory_t *pProfile);

 sbStatus_t
sbFe2000ProfileDelete(sbFe2000Pm_t *pPm,
                    uint32_t uProfileId);

PRIVATE sbStatus_t
sbFe2000NullProfileCreate(sbFe2000Pm_t *pPm);

PRIVATE sbStatus_t  
sbFe2000PolicerTimerTickGet(sbFe2000Pm_t *pPm, sbFe2000PmPolicerRate_t rate, uint32_t *pTimerTick);

PRIVATE sbStatus_t 
sbFe2000PmLocProfileMemoryWrite(sbFe2000Pm_t *pPm,
                           uint32_t uProfileId, 
                           sbZfFe2000PmProfileMemory_t *pProfile);

PRIVATE sbStatus_t 
sbFe2000XtPmLocProfileMemoryWrite(sbFe2000Pm_t *pPm,
                           uint32_t uProfileId, 
                           sbZfC2PmProfileMemory_t *pProfile);


PRIVATE sbStatus_t
sbFe2000PmLocProfileMemoryRead(sbFe2000Pm_t *pPm, uint32_t uProfileId, sbZfFe2000PmProfileMemory_t *pProfile);

PRIVATE sbStatus_t
sbFe2000XtPmLocProfileMemoryRead(sbFe2000Pm_t *pPm, uint32_t uProfileId, sbZfC2PmProfileMemory_t *pProfile);

PRIVATE sbStatus_t 
sbFe2000PmProfileAlloc(sbFe2000Pm_t *pPm, sbZfFe2000PmProfileMemory_t *pProfile, uint32_t *pIndex);

PRIVATE sbStatus_t 
sbFe2000XtPmProfileAlloc(sbFe2000Pm_t *pPm, sbZfC2PmProfileMemory_t *pProfile, uint32_t *pIndex);

PRIVATE sbStatus_t
sbFe2000PmPolicerBestFitProfileGet(sbZfFe2000PmPolicerConfig_t *pConfig, 
                                 sbZfFe2000PmProfilePolicerMemory_t *pProfile);

PRIVATE sbStatus_t
sbFe2000XtPmPolicerBestFitProfileGet(sbZfFe2000PmPolicerConfig_t *pConfig, 
                                 sbZfC2PmProfilePolicerMemory_t *pProfile);

PRIVATE sbStatus_t 
sbFe2000PmIEEECodingGet(uint32_t uBytes, uint32_t *pEncodedValue);


PRIVATE sbStatus_t
sbFe2000PmGroupGet(sbFe2000Pm_t *pPm, sbFe2000PmPolicerRate_t rate, uint32_t *pGroup);

PRIVATE sbStatus_t
AllocMemChunk(sbFe2000Pm_t *pPmMgr);

PRIVATE sbStatus_t
AllocPolicer(sbFe2000Pm_t *pPmMgr, sbFe2000PmPolicer_t **pPolicer);

PRIVATE sbStatus_t
FreePolicer(sbFe2000Pm_t *pPmMgr, sbFe2000PmPolicer_t *pPolicer);

PRIVATE sbStatus_t
sbFe2000PmPolicerRefreshSet(sbFe2000Pm_t *pPm, uint32_t uPolicerId, uint32_t bEnable);

PRIVATE sbStatus_t
sbFe2000PolicerGroupRateGet(sbFe2000Pm_t *pPm, uint32_t uTimerTick, sbFe2000PmPolicerRate_t *uRate);

PRIVATE sbStatus_t
sbFe2000PmOamTimerBestFitProfileGet(sbZfFe2000PmOamTimerConfig_t *pConfig, 
                                    sbZfFe2000PmProfileTimerMemory_t *pProfile);

PRIVATE sbStatus_t
sbFe2000XtPmOamTimerBestFitProfileGet(sbZfFe2000PmOamTimerConfig_t *pConfig, 
                                      sbZfC2PmProfileTimerMemory_t *pProfile);

PRIVATE sbStatus_t
sbFe2000PmOamSeqGenBestFitProfileGet( sbZfFe2000PmProfileSeqGenMemory_t *pProfile);

PRIVATE sbStatus_t
sbFe2000XtPmOamSeqGenBestFitProfileGet( sbZfC2PmProfileSeqGenMemory_t *pProfile);

PRIVATE sbStatus_t
sbFe2000PmPolicerGroupReadConfig(sbFe2000Pm_t *pPm,  uint32_t uGroupId, 
                           sbZfFe2000PmGroupConfig_t *pGroupConfig);

PRIVATE sbStatus_t
sbFe2000PmPolicerRawSet(sbFe2000Pm_t *pPm,
                        uint32_t ulIndex,
                        sbZfFe2000PmPolicerRawFormat_t *pPrm);
PRIVATE sbStatus_t
sbFe2000PmPolicerRawGet(sbFe2000Pm_t *pPm,
                        uint32_t ulIndex,
                        sbZfFe2000PmPolicerRawFormat_t *pPrm);
PRIVATE sbStatus_t
sbFe2000PmOamSeqGenRawSet(sbFe2000Pm_t *pPm,
                        uint32_t ulIndex,
                        sbZfFe2000PmOamSeqGenRawFormat_t *pPrm);

#define SB_FE_2000_PM_DMA_MEMORY_SIZE  \
    SB_ZF_FE_2000_PM_POLICERRAWFORMAT_SIZE_IN_BYTES

int
sbFe2000PmConvertRate(int kbps)
{
  /* Split points are arbitrary; expect to be called with exact #s */
  if (kbps < 3) {
    return SB_FE_2000_PM_PRATE_1KBPS;
  } else if (kbps < 33) {
    return SB_FE_2000_PM_PRATE_10KBPS;
  } else if (kbps < 150) {
    return SB_FE_2000_PM_PRATE_100KBPS;
  } else if (kbps < 444) {
    return SB_FE_2000_PM_PRATE_250KBPS;
  } else if (kbps < 666) {
    return SB_FE_2000_PM_PRATE_500KBPS;
  } else if (kbps < 3333) {
    return SB_FE_2000_PM_PRATE_1MBPS;
  } else if (kbps < 33333) {
    return SB_FE_2000_PM_PRATE_10MBPS;
  } else if (kbps < 333333) {
    return SB_FE_2000_PM_PRATE_100MBPS;
  } else if (kbps < 3333333) {
    return SB_FE_2000_PM_PRATE_1GBPS;
  } else {
    return SB_FE_2000_PM_PRATE_10GBPS;
  }
}

void
sbFe2000PmSwDump(sbFe2000Pm_t *pPm)
{
  uint32_t idx, count, shown;
  dq_t *pElt;


  soc_cm_print("Profiles:     profile_id:<num_policers, ref_count>\n");
  
  shown = 0;
  for (idx = 0; idx < SB_FE_2000_PM_MAX_NUM_PROFILES; idx++) {

    count = 0;
    DQ_TRAVERSE((dq_t*) &pPm->profiledb[idx].listPolicers, pElt) {
      count++;
    } DQ_TRAVERSE_END(listPolicers, pElt);

    if (count || pPm->profiledb[idx].uRefCount) {
      soc_cm_print("0x%03x:<%5d,%5d> ", idx, count, pPm->profiledb[idx].uRefCount);
      if ((shown++ & 3) == 3) {
        soc_cm_print("\n");
      }
    }
  }
  soc_cm_print("\n");
}

sbStatus_t
sbFe2000PmRecoverPolicer(sbFe2000Pm_t *pPm, uint32_t policer_id,
                         uint32_t profile_id)
{
  uint32_t             group, group_max_id;
  sbFe2000PmPolicer_t *pPolicerRef = NULL;
  
  /* Find the group for this policer */
  group_max_id = 0;
  for (group = 0; group < SB_FE_2000_PM_NUM_OF_GROUPS; group++) {
    group_max_id += pPm->PolicerConfig.uNumberOfPolicersPerGroup[group];
    if (policer_id < group_max_id) {
      break;
    }
  }
  
  if (group == SB_FE_2000_PM_NUM_OF_GROUPS) {
    return SB_BUSY_ERR_CODE;
  }

  pPm->profiledb[profile_id].uRefCount++;

  /* Add the policer configuration to profile ref list */
  AllocPolicer(pPm, &pPolicerRef);
  pPolicerRef->uPolicerId = policer_id;
  pPolicerRef->uProfileId = profile_id;
  pPolicerRef->uGroupId   = group;
  DQ_INSERT_TAIL((dq_p_t) &pPm->profiledb[profile_id].listPolicers, &pPolicerRef->listElem);

  return SB_OK;
}


sbStatus_t
sbFe2000PmRecover(sbFe2000Pm_t *pPm)
{
  uint32_t                              id;
  sbStatus_t                            st = SB_OK;
    
  for (id=0; id < SB_FE_2000_PM_MAX_NUM_PROFILES; id++) {
      
    if (SAND_HAL_IS_FE2KXT(pPm->PolicerConfig.regSet)) {
      st = sbFe2000XtPmLocProfileMemoryRead(pPm, id, &pPm->profiledb[id].C2Profile);
    } else {
      st = sbFe2000PmLocProfileMemoryRead(pPm, id, &pPm->profiledb[id].profile);
    }
      
    if (st != SB_OK) {
      soc_cm_print("Failed to read profile %d on recovery: 0x%08x\n", 
                   id, st);
      return st;
    }      
  }

  /* The remaining profile state is recovered when the bcm layer policer
   * recovers the individual policer via sbFe2000PmRecoverPolicer()
   */

  return SB_OK;
}

/*
 * Policer Management Unit software module initialization
 */
sbStatus_t 
sbFe2000PmInit(uint32_t uHppFreq, sbFe2000Pm_t **pPmMgr, sbFe2000PmConfig_t *pConfig)
{
  sbFe2000Pm_t    *pPm;
  sbStatus_t            err = SB_OK;
  void                  *vp;
  uint32_t uPolicerBase = 0;
  uint32_t uValue = 0, uAddress =0;
  uint32_t uNumPolicerGroups = 0;
  uint32_t i =0;
  sbZfFe2000PmGroupConfig_t zGroupConfig;

  /* allocate memory for the policer manager */
  vp = sal_alloc(sizeof(struct sbFe2000Pm_s), "fe2k pm state");
  if (!vp) {
    return SB_MALLOC_FAILED;
  }
  sal_memset(vp, 0, sizeof(struct sbFe2000Pm_s));

  *pPmMgr = pPm = (sbFe2000Pm_t *)vp;
  pPm->PolicerConfig.regSet = pConfig->regSet;
  pPm->PolicerConfig.dmaMgr = pConfig->dmaMgr;
  pPm->PolicerConfig.base = pConfig->base;
  pPm->PolicerConfig.bank = pConfig->bank;
  pPm->PolicerConfig.token = pConfig->token;
  pPm->PolicerConfig.uNumberOfPolicers = pConfig->uNumberOfPolicers;

  for (i = 0; i < SB_FE_2000_PM_NUM_OF_GROUPS; i++) {
    pPm->PolicerConfig.uNumberOfPolicersPerGroup[i]
      = pConfig->uNumberOfPolicersPerGroup[i];
      pPm->PolicerConfig.rates[i] = pConfig->rates[i];
  }
  
  err = thin_malloc(pPm->PolicerConfig.token, SB_ALLOC_OTHER_DMA,
                    SB_FE_2000_PM_DMA_MEMORY_SIZE,
                    (void **) &pPm->dmaOp.hostAddress,
                    &pPm->dmaOp.dmaHandle);
  if (err) {
      *pPmMgr = NULL;
      sal_free(vp);
      return err;
  }
  pPm->dmaOp.hostBusAddress = SB_FE2000_DMA_HBA_COMPUTE;
  pPm->dmaOp.words = 2;

  DQ_INIT((dq_p_t) &pPm->listFreePolicers);

  /* Initialize the List of Policers per Profile */
  for(i=0; i < SB_FE_2000_PM_MAX_NUM_PROFILES; i++) {
    DQ_INIT((dq_p_t) &pPm->profiledb[i].listPolicers);
    pPm->profiledb[i].uRefCount = 0;

    if (SAND_HAL_IS_FE2KXT (pPm->PolicerConfig.regSet ))
    {
      sbZfC2PmProfileMemory_InitInstance(&pPm->profiledb[i].C2Profile);
    }
    else
    {
      sbZfFe2000PmProfileMemory_InitInstance(&pPm->profiledb[i].profile);
    }
  }

  /* configure the HPP clock frequency */
  pPm->uHppFreq = uHppFreq;

  /* allocate the memory chunks */
  SB_MEMSET(pPm->aMemChunks, 0, sizeof(pPm->aMemChunks));
  pPm->uChunkCount       = 0;
  pPm->uPolicersPerChunk = MAX_PM_MEM_CHUNKS;

  AllocMemChunk(pPm);

  sbZfFe2000PmGroupConfig_InitInstance(&zGroupConfig);

  /* configure the maximum number of in-fligh refreshes */
  SB_LOGV3("%s: Initialize PM_CONFIG1 \n", __FUNCTION__);
  uValue = SAND_HAL_FE2000_MOD_FIELD(pPm->PolicerConfig.regSet, PM_CONFIG1, MAX_REFRESHES, uValue, SB_FE_2000_PM_MAX_REFRESHES);
  SAND_HAL_FE2000_WRITE(pPm->PolicerConfig.regSet, PM_CONFIG1, uValue);

  SB_LOGV3("%s: Initialize PM_CONFIG0 \n", __FUNCTION__);
  /* read the current valueof the PM_CONFIG0 */
  uValue = SAND_HAL_FE2000_READ(pPm->PolicerConfig.regSet,PM_CONFIG0);

  /* make sure the global timer_enable is disabled */
  uValue = SAND_HAL_FE2000_MOD_FIELD(pPm->PolicerConfig.regSet, PM_CONFIG0, REFRESH_ENABLE, uValue, 0);
  uValue = SAND_HAL_FE2000_MOD_FIELD(pPm->PolicerConfig.regSet, PM_CONFIG0, TOTAL_REFRESH_THRESH,uValue, SB_FE_2000_PM_TOTAL_REFRESH_THRESH);

  switch (pPm->PolicerConfig.bank) {
  case SB_FE2000_MEM_MM0_WIDE:
  case SB_FE2000_MEM_MM0_INTERNAL_0:
  case SB_FE2000_MEM_MM0_INTERNAL_1:
    i = SB_FE_2000_PM_MM_SEL0;
    break;
  case SB_FE2000_MEM_MM1_WIDE:
  case SB_FE2000_MEM_MM1_INTERNAL_0:
  case SB_FE2000_MEM_MM1_INTERNAL_1:
    i = SB_FE_2000_PM_MM_SEL1;
    break;
  default:
    SB_LOG("illegal policer bank selection: %d", pPm->PolicerConfig.bank);
    return SB_BAD_ARGUMENT_ERR_CODE;
  }

  uValue = SAND_HAL_FE2000_MOD_FIELD(pPm->PolicerConfig.regSet, PM_CONFIG0, MM_SELECT_BIT, uValue, i);
  /* leaving the arbitration as it is i.e to default values */

  uAddress = SAND_HAL_FE2000_REG_OFFSET(pPm->PolicerConfig.regSet, PM_CONFIG0);
  SAND_HAL_FE2000_WRITE_OFFS(pPm->PolicerConfig.regSet, uAddress, uValue);

  /* configure each of the profile memories */
  uPolicerBase = pPm->PolicerConfig.base;

  uNumPolicerGroups = SB_FE_2000_PM_NUM_TOTAL_GROUPS;
  for(i=0; i < uNumPolicerGroups; i++) {
    sbZfFe2000PmGroupConfig_InitInstance(&zGroupConfig);
    /* Turn Enable ON for Groups with non-zero Number of Policers */
    zGroupConfig.uEnable = 0;
    if(pPm->PolicerConfig.uNumberOfPolicersPerGroup[i])  zGroupConfig.uEnable = 1;
    zGroupConfig.uMinPolicerId = uPolicerBase;
    zGroupConfig.uMaxPolicerId = uPolicerBase+pPm->PolicerConfig.uNumberOfPolicersPerGroup[i]-1;

    err = sbFe2000PmPolicerGroupReadConfig(pPm, i, &zGroupConfig);
    if (err != SB_OK) {
      return err;
    }

    sbFe2000PmGroupConfigSet(pPm->PolicerConfig.regSet,i,&zGroupConfig);

    /* update the new policer base */
    uPolicerBase += pPm->PolicerConfig.uNumberOfPolicersPerGroup[i];
  }
  
  if (SOC_WARM_BOOT((int)pPm->PolicerConfig.regSet)) {

    err = sbFe2000PmRecover(pPm);
    if (err != SB_OK) {
      soc_cm_print("Failed to recover soc_policer: 0x%08x\n", err);
      return err;
    }
    
  } else {
    /* +1: C2 has an extra word and sbFe2000PmProfileMemoryWrite is common for FE2000 and C2 */
    uint32_t id, uProfileWords[SB_ZF_FE_2000_PM_PROFMEMORY_SIZE_IN_WORDS + 1];
    sal_memset(uProfileWords, 0, (SB_ZF_FE_2000_PM_PROFMEMORY_SIZE_IN_WORDS + 1) * sizeof(uProfileWords[0]));

    for (id=0; id < SB_FE_2000_PM_MAX_NUM_PROFILES; id++) {

      err = sbFe2000PmProfileMemoryWrite(pPm->PolicerConfig.regSet, id, uProfileWords);
      if (err != SB_OK) {
        soc_cm_print("Failed to clear profile %d: 0x%08x\n", id, err);
        return err;
      }
    }

    /* create the null profile 
     * The Null profile is needed for warm boot.  By ensuring exactly one
     * null profile exsists, the last valid profile can be found because
     * only uniqueue profiles are ever created
     */
    err = sbFe2000NullProfileCreate(pPm);
    if(err != SB_OK) {
      SB_LOGV1("error: null profile create failed\n");
    }
  }

  /* enable the global refresh */
  /* read the current valueof the PM_CONFIG0 */
  uValue = SAND_HAL_FE2000_READ(pPm->PolicerConfig.regSet,PM_CONFIG0);
  uValue = SAND_HAL_FE2000_MOD_FIELD(pPm->PolicerConfig.regSet, PM_CONFIG0, REFRESH_ENABLE, uValue, 1);
  uAddress = SAND_HAL_FE2000_REG_OFFSET(pPm->PolicerConfig.regSet, PM_CONFIG0);
  SAND_HAL_FE2000_WRITE_OFFS(pPm->PolicerConfig.regSet, uAddress, uValue);

  return SB_OK;
}

/*
 * Policer Management Unit UnInitialization
 */
sbStatus_t 
sbFe2000PmUnInit(sbFe2000Pm_t *pPm)
{
  return SB_OK;
}

/**
 * This function configures a policer and returns
 * the index of the created  policer id.
 * 
 * @param pPm             Pointer to the sbFe2000Pm_t struct
 * @param uPolicerId      PolicerId
 * @param *pConfig        Pointer to the Policer Configuration.
 * @param *pIndex         pointer to the policer index value
 * @return                Return status of the type sbStatus_t
 */
sbStatus_t 
sbFe2000PmPolicerCreate(sbFe2000Pm_t *pPm,
                      uint32_t ulPolicerId,
                      sbZfFe2000PmPolicerConfig_t *pConfig,
                      uint32_t *pIndex)
{
  sbStatus_t uStatus;
  uint32_t grp=0xFF;
  uint32_t uMinPolicerId = 0,uMaxPolicerId = 0;
  uint32_t uAddress = 0, uValue =0;
  uint32_t uPolicerId=0, uProfileId= 0xFFFF;
  uint8_t bFound = 0;
  uint8_t   uPolicerBytes[SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_SIZE_IN_BYTES];
  uint8_t   uXtPolicerBytes[SB_ZF_C2_PM_PROFPOLICERMEMORY_SIZE_IN_BYTES];
  uint32_t  uBktCBits, tempBktCBits, tempBktEBits, uBktEBits, uBktEShift, tempCBS, tempEBS; 
  uint64_t  uBktCMask, uBktEMask;
  uint64_t  ulTimeStamp, ulBktE, ulBktC, ulBktTemp;
  uint32_t  uBktESize; /* configured BktE size */


  sbZfFe2000PmProfilePolicerMemory_t  zProfile;
  sbZfC2PmProfilePolicerMemory_t  zXtProfile;
  sbZfFe2000PmPolicerRawFormat_t      zPolicer;
  sbZfFe2000PmGroupConfig_t         zGroupConfig;
  sbFe2000PmPolicer_t *pPolicerRef = NULL;

  sbZfFe2000PmProfileMemory_t  zCommonProfile;
  sbZfC2PmProfileMemory_t  zXtCommonProfile;

  sbZfFe2000PmProfilePolicerMemory_InitInstance(&zProfile);
  sbZfC2PmProfilePolicerMemory_InitInstance(&zXtProfile);
  sbZfFe2000PmPolicerRawFormat_InitInstance(&zPolicer);
  sbZfFe2000PmProfileMemory_InitInstance(&zCommonProfile);
  sbZfC2PmProfileMemory_InitInstance(&zXtCommonProfile);
  SB_MEMSET(&uPolicerBytes[0], 0, SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_SIZE_IN_BYTES);
  SB_MEMSET(&uXtPolicerBytes[0], 0, SB_ZF_C2_PM_PROFPOLICERMEMORY_SIZE_IN_BYTES);

  sbZfFe2000PmGroupConfig_InitInstance(&zGroupConfig);

  uBktCBits = 0;
  tempBktCBits = 0;
  uBktEBits = 0;
  tempBktEBits = 0;
  uBktEShift = 0; 

  COMPILER_64_ZERO(uBktCMask);
  COMPILER_64_ZERO(uBktEMask);
  COMPILER_64_ZERO(ulTimeStamp);
  COMPILER_64_ZERO(ulBktE);
  COMPILER_64_ZERO(ulBktC);

  COMPILER_64_ZERO(ulBktTemp);
  COMPILER_64_NOT(ulBktTemp);

  tempCBS = 0;
  tempEBS = 0;


  /* 
   * step 1: get the policer group.
   */
  uStatus = sbFe2000PmGroupGet(pPm, (sbFe2000PmPolicerRate_t) pConfig->uRate, &grp);
  if(uStatus != SB_OK) {
    return uStatus;
  }

  /* 
   * step 2: see if this group has any policer left 
   * that can be assigned to this new policer
   */

  /* read this groups min and max policer id configured */
  uAddress = SAND_HAL_FE2000_REG_OFFSET(pPm->PolicerConfig.regSet, PM_GROUP0_CONFIG0) + 16*grp;
  uValue = SAND_HAL_FE2000_READ_OFFS(pPm->PolicerConfig.regSet, uAddress);
  uMinPolicerId = SAND_HAL_FE2000_GET_FIELD(pPm->PolicerConfig.regSet,PM_GROUP0_CONFIG0, POLID_MIN,uValue);
  uValue = SAND_HAL_FE2000_READ_OFFS(pPm->PolicerConfig.regSet, (uAddress+4));
  uMaxPolicerId = SAND_HAL_FE2000_GET_FIELD(pPm->PolicerConfig.regSet,PM_GROUP0_CONFIG1, POLID_MAX,uValue);

  /* Min & Max Policer Id stores the Policer Memory Bases */
  uMinPolicerId -= pPm->PolicerConfig.base;
  uMaxPolicerId -= pPm->PolicerConfig.base;

  uPolicerId = ulPolicerId;
  bFound = 1;

  if (uPolicerId < uMinPolicerId) {
    return SB_INVALID_RANGE;
  }

  if(!bFound || (uPolicerId >uMaxPolicerId) ) {
    return SB_PM_TOO_MANY_POLICERS;
  }

  /* retrieve the group config to use the timestamp */
  uStatus = sbFe2000PmGroupConfigGet(pPm->PolicerConfig.regSet, grp, &zGroupConfig);
  if(uStatus != SB_OK) {
    return uStatus;
  }
  /* Check if the configured Excess burst sizes can be accomodated */
  uBktESize = 0;
  while(pConfig->uEBS >> uBktESize) {
    uBktESize++;
  }

  /* 
   * step 3: find the profile that's a fit for this policer
   * configuration.
   */

  if (SAND_HAL_IS_FE2KXT (pPm->PolicerConfig.regSet )) {
    uStatus = sbFe2000XtPmPolicerBestFitProfileGet(pConfig, &zXtProfile);
    if (uStatus != SB_OK) {
      return uStatus;
    }

    if (uBktESize > (zGroupConfig.uTimestampOffset - zXtProfile.uBktCSize)) {
      SB_LOG("%s: Invalid Excess Burst Size. Max supported - %d Bytes \n",
             FUNCTION_NAME(), ((1<<(zGroupConfig.uTimestampOffset - zXtProfile.uBktCSize)) - 1));
      return SB_INVALID_RANGE;
    }
    
    sbZfC2PmProfilePolicerMemory_Pack( &zXtProfile, &uXtPolicerBytes[0], SB_ZF_C2_PM_PROFPOLICERMEMORY_SIZE_IN_BYTES);
    sbZfC2PmProfileMemory_Unpack( &zXtCommonProfile, &uXtPolicerBytes[0], sizeof(zXtCommonProfile) ) ;
    
    uStatus = sbFe2000XtPmProfileAlloc(pPm, &zXtCommonProfile, &uProfileId);
    if(uStatus != SB_OK) {
      return uStatus;
    }
    
  } else {
    uStatus = sbFe2000PmPolicerBestFitProfileGet(pConfig, &zProfile);
    if (uStatus != SB_OK) {
      return uStatus;
    }

    if (uBktESize > (zGroupConfig.uTimestampOffset - zProfile.uBktCSize)) {
      SB_LOG("%s: Invalid Excess Burst Size. Max supported - %d Bytes \n",
             FUNCTION_NAME(), ((1<<(zGroupConfig.uTimestampOffset - zProfile.uBktCSize)) - 1));
      return SB_INVALID_RANGE;
    }
    
    sbZfFe2000PmProfilePolicerMemory_Pack( &zProfile, &uPolicerBytes[0], SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_SIZE_IN_BYTES);
    sbZfFe2000PmProfileMemory_Unpack( &zCommonProfile, &uPolicerBytes[0], sizeof(zCommonProfile) ) ;
    
    uStatus = sbFe2000PmProfileAlloc(pPm, &zCommonProfile, &uProfileId);
    if(uStatus != SB_OK) {
      return uStatus;
    }
  }

  /* Add the policer configuration to profile ref list */
  AllocPolicer(pPm, &pPolicerRef);
  pPolicerRef->uPolicerId = uPolicerId;
  pPolicerRef->uProfileId = uProfileId;
  pPolicerRef->uGroupId   = grp;
  DQ_INSERT_TAIL((dq_p_t) &pPm->profiledb[uProfileId].listPolicers, &pPolicerRef->listElem);

  /* suppress writes to this policer id before changing it's config */
  sbFe2000PmPolicerRefreshSet(pPm, uPolicerId, SB_FE_2000_PM_DISABLE_REFRESH);

  zPolicer.uProfileId   = uProfileId;

  /* Num BktCBits is indicated by uBktCSize - max of 28 bits */

  if (SAND_HAL_IS_FE2KXT (pPm->PolicerConfig.regSet ))
  {
   
    uBktCBits = zXtProfile.uBktCSize;
    tempBktCBits = zXtProfile.uBktCSize;
    tempBktEBits = zXtProfile.uBktCSize;
    tempEBS = zXtProfile.uEBS;
    tempCBS = zXtProfile.uCBS;
  }
  else
  {
    uBktCBits = zProfile.uBktCSize;
    tempBktCBits = zProfile.uBktCSize;
    tempBktEBits = zProfile.uBktCSize;
    tempEBS = zProfile.uEBS;
    tempCBS = zProfile.uCBS;
  }
  if( uBktCBits > 28 )
  {
    uBktCBits = 28;
  }

  /* 
   * BktEBits = 0 if TimeStampOffset = BktCSize 
   * Otherwise BktEBits = ( TimeStampOffset - BktCSize ) => max of 32 
   */
  if( zGroupConfig.uTimestampOffset == tempBktCBits )
  {
    uBktEBits = 0;
  } else {
    uBktEBits = zGroupConfig.uTimestampOffset - tempBktCBits;
    if((uBktEBits + uBktCBits) >= 53) {
      uBktEBits = 53 - uBktCBits - 1; /* 1 is for the TimeStamp */
    } else if( uBktEBits > 28 ) {
      uBktEBits = 28;
    }
  }

  uBktEShift  = uBktCBits;

  COMPILER_64_SHL(uBktCMask, uBktCBits);
  COMPILER_64_SUB_32(uBktCMask, 1);

  COMPILER_64_SHL(uBktEMask, uBktEBits);
  COMPILER_64_SUB_32(uBktEMask, 1);
  
  COMPILER_64_ZERO(ulTimeStamp);

  COMPILER_64_SET(ulBktE, 0, tempEBS);
  COMPILER_64_AND(ulBktE, uBktEMask);
  COMPILER_64_SHL(ulBktE, uBktEShift);
  COMPILER_64_SHL(ulBktTemp, (0 << (uBktCBits - 1)));
  COMPILER_64_AND(ulBktE, ulBktTemp);

  COMPILER_64_SET(ulBktC, 0, tempCBS);
  COMPILER_64_AND(ulBktC, uBktCMask);

  if( uBktEBits == 0 ) {
    COMPILER_64_ZERO(ulBktE);
  } 

  COMPILER_64_OR(zPolicer.uMeterState, ulTimeStamp);
  COMPILER_64_OR(zPolicer.uMeterState, ulBktE);
  COMPILER_64_OR(zPolicer.uMeterState, ulBktC);

  SB_LOGV3("Policer Raw Set: ulBktE %llx  ulBktC %llx ulTimeStamp %llx\n", 
            ulBktE, ulBktC, ulTimeStamp);
  SB_LOGV3("   MeterState %llx\n", zPolicer.uMeterState);
  SB_LOGV3("   uBktCBits %d   uBktEBits %d\n", uBktCBits, uBktEBits);

  uStatus = sbFe2000PmPolicerRawSet(pPm, uPolicerId, &zPolicer);

  /* let the refreshes go on */
  sbFe2000PmPolicerRefreshSet(pPm, uPolicerId, SB_FE_2000_PM_ENABLE_REFRESH);

  /* return the configured policer id to the user */
  *pIndex = uPolicerId;
  return uStatus;
}


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
                      uint32_t uPolicerId)
{
  sbStatus_t uStatus;
  uint32_t uProfileId=0,uGroupId=0,bFound=0;
  uint32_t uAddress=0 , uValue =0,uMinPolicerId=0;
  sbFe2000PmPolicer_t *pPolicerRef = NULL;
  sbZfFe2000PmPolicerRawFormat_t zPolicer;
  dq_p_t pPolicerElm = NULL;

  uStatus = sbFe2000PmPolicerRawGet(pPm, uPolicerId, &zPolicer);
  if(uStatus != SB_OK) {
    return uStatus;
  }
  /* copy the current profile id */
  uProfileId = zPolicer.uProfileId;

  /* See if the policer is active */
  bFound =0;
  DQ_TRAVERSE((dq_p_t) &pPm->profiledb[uProfileId].listPolicers, pPolicerElm) {
    pPolicerRef  = (sbFe2000PmPolicer_t *) pPolicerElm;
    if( (pPolicerRef->uPolicerId == uPolicerId) &&
        (pPolicerRef->uProfileId == uProfileId)  ) {
      bFound =1;
      break;
    }
  } DQ_TRAVERSE_END(listPolicers, pPolicerElm);
   
  /* if the policer is found free it */
  if(!bFound) {
    /* error */
    return SB_BAD_ARGUMENT_ERR_CODE;
  }

  sbZfFe2000PmPolicerRawFormat_InitInstance(&zPolicer);
  zPolicer.uProfileId  = SB_FE_2000_PM_NULL_PROFILE_ID;
  COMPILER_64_ZERO(zPolicer.uMeterState);
  /* suppress writes to this policer id before changing it's config */
  sbFe2000PmPolicerRefreshSet(pPm, uPolicerId, SB_FE_2000_PM_DISABLE_REFRESH);
  /* now set it to null profile id */
  uStatus = sbFe2000PmPolicerRawSet(pPm, uPolicerId, &zPolicer);
  /* let the refreshes go on with lowest rate assured by slowrate */
  sbFe2000PmPolicerRefreshSet(pPm, uPolicerId, SB_FE_2000_PM_ENABLE_REFRESH);
  if(uStatus != SB_OK) {
    return uStatus;
  }

  sbFe2000PolicerRemove(pPm, pPolicerRef);

  /* free the policer in the group */
  uGroupId = pPolicerRef->uGroupId;
  uAddress = SAND_HAL_FE2000_REG_OFFSET(pPm->PolicerConfig.regSet, PM_GROUP0_CONFIG0) + 16*uGroupId;
  uValue = SAND_HAL_FE2000_READ_OFFS(pPm->PolicerConfig.regSet, uAddress);
  uMinPolicerId = SAND_HAL_FE2000_GET_FIELD(pPm->PolicerConfig.regSet,PM_GROUP0_CONFIG0, POLID_MIN,uValue);
  uMinPolicerId -= pPm->PolicerConfig.base;
  uPolicerId -= uMinPolicerId;

  /* update the database */
  pPm->profiledb[uProfileId].uRefCount--;
  if(pPm->profiledb[uProfileId].uRefCount == 0) {
    sbFe2000ProfileDelete(pPm, uProfileId);
  }

  return SB_OK;
}

/**
 * @fn sbFe2000PolicerRemove()
 *
 * @brief 
 *
 * Remove a Policer from a profile, given a pointer to the Policer.
 *
 * @param pPm         - Driver control structure
 * @param pPolicer    - Reference to the Policer to be removed
 *
 * @return - SB_OK on success, error code otherwise
 **/
sbStatus_t
sbFe2000PolicerRemove(sbFe2000Pm_t *pPm, 
		                sbFe2000PmPolicer_t *pPolicer)
{
  if (DQ_NULL((dq_p_t) &pPolicer->listElem) == FALSE)
    DQ_REMOVE((dq_p_t) &pPolicer->listElem);
  return FreePolicer(pPm, pPolicer);
}

sbStatus_t
sbFe2000PmPolicerGroupRateChange( sbFe2000Pm_t *pPm,
                                uint32_t uGroupId,
                                uint32_t uRateId)
{
  sbStatus_t status = SB_OK;
  uint32_t uPolicerId=0;
  sbZfFe2000PmGroupConfig_t zGroupConfig;
  uint32_t uConfig=0, uValue=0, uAddress=0;
  char     buf[1024];

  /* Invalidate all Policers and Profiles */
  for( uPolicerId = 0; uPolicerId < pPm->PolicerConfig.uNumberOfPolicersPerGroup[uGroupId]; uPolicerId++ )
  {
    sbFe2000PmPolicerDelete( pPm, uPolicerId );
  } 

  /* START : make sure the global timer_enable is disabled */
  uValue = SAND_HAL_FE2000_READ(pPm->PolicerConfig.regSet,PM_CONFIG0);

  uValue = SAND_HAL_FE2000_MOD_FIELD(pPm->PolicerConfig.regSet, PM_CONFIG0, REFRESH_ENABLE, uValue, 0);
  uValue = SAND_HAL_FE2000_MOD_FIELD(pPm->PolicerConfig.regSet, PM_CONFIG0, TOTAL_REFRESH_THRESH,uValue, SB_FE_2000_PM_TOTAL_REFRESH_THRESH);

  /*
   * Re-Configure / Identify the Policer Group Config
   *    a. Find out the Time Tick from Rate - sbFe2000PolicerTimerTickGet
   *    b. Create the null profile.
   */

  pPm->PolicerConfig.rates[uGroupId] = uRateId;
  sbZfFe2000PmGroupConfig_InitInstance(&zGroupConfig);

  status = sbFe2000PmPolicerGroupReadConfig(pPm, uGroupId, &zGroupConfig);

  if(status != SB_OK) {
    return status;
  }

  sbZfFe2000PmGroupConfig_SPrint(&zGroupConfig,&buf[0], 1024);
  SB_LOGV3("PmGroup%d Configuration reinited is %s\n", uGroupId, &buf[0]);

  /* 
   * Commit the information to the registers
   *    a. First disable the group before touching anything 
   *    b. configure the refresh counts and periods
   *    c. configure the timer tick period
   *    ref : sbFe2000PmGroupConfigSet()
   */
  uAddress = SAND_HAL_FE2000_REG_OFFSET(pPm->PolicerConfig.regSet,PM_GROUP0_CONFIG0) + uGroupId*16;
  uConfig = SAND_HAL_FE2000_READ_OFFS(pPm->PolicerConfig.regSet, uAddress);

  /* First disable the group before touching anything */
  uConfig = SAND_HAL_FE2000_MOD_FIELD(pPm->PolicerConfig.regSet, PM_GROUP0_CONFIG0, ENABLE, uConfig, 0);
  SAND_HAL_FE2000_WRITE_OFFS(pPm->PolicerConfig.regSet, uAddress, uConfig);

  uAddress += 4;

  uConfig = SAND_HAL_FE2000_READ_OFFS(pPm->PolicerConfig.regSet, uAddress); /* Max Policer Id */

  uAddress +=4;

  uConfig = SAND_HAL_FE2000_READ_OFFS(pPm->PolicerConfig.regSet, uAddress);
  uConfig = SAND_HAL_FE2000_MOD_FIELD(pPm->PolicerConfig.regSet, PM_GROUP0_CONFIG2,REFRESH_COUNT, uConfig,zGroupConfig.uRefreshCount);
  uConfig = SAND_HAL_FE2000_MOD_FIELD(pPm->PolicerConfig.regSet, PM_GROUP0_CONFIG2,REFRESH_THRESH, uConfig,zGroupConfig.uRefreshThresh);
  uConfig = SAND_HAL_FE2000_MOD_FIELD(pPm->PolicerConfig.regSet, PM_GROUP0_CONFIG2,REFRESH_PERIOD, uConfig,zGroupConfig.uRefreshPeriod);
  SAND_HAL_FE2000_WRITE_OFFS(pPm->PolicerConfig.regSet, uAddress, uConfig);

  uAddress += 4;

  uConfig = SAND_HAL_FE2000_READ_OFFS(pPm->PolicerConfig.regSet, uAddress);
  uConfig = SAND_HAL_FE2000_MOD_FIELD(pPm->PolicerConfig.regSet, PM_GROUP0_CONFIG3, TIMER_ENABLE, uConfig, zGroupConfig.uTimerEnable ? 1 : 0);
  uConfig = SAND_HAL_FE2000_MOD_FIELD(pPm->PolicerConfig.regSet, PM_GROUP0_CONFIG3, TIMESTAMP_OFFSET, uConfig, zGroupConfig.uTimestampOffset);
  uConfig = SAND_HAL_FE2000_MOD_FIELD(pPm->PolicerConfig.regSet, PM_GROUP0_CONFIG3, TIMER_TICK_PERIOD, uConfig, zGroupConfig.uTimerTickPeriod);
  SAND_HAL_FE2000_WRITE_OFFS(pPm->PolicerConfig.regSet, uAddress, uConfig);


  uAddress = SAND_HAL_FE2000_REG_OFFSET(pPm->PolicerConfig.regSet,PM_GROUP0_CONFIG0) + uGroupId*16;
  uConfig = SAND_HAL_FE2000_READ_OFFS(pPm->PolicerConfig.regSet, uAddress);
  uConfig = SAND_HAL_FE2000_MOD_FIELD(pPm->PolicerConfig.regSet, PM_GROUP0_CONFIG0, ENABLE, uConfig, 1);
  SAND_HAL_FE2000_WRITE_OFFS(pPm->PolicerConfig.regSet, uAddress, uConfig);

  /* create the null profile  - NOT SURE IF WE NEED THIS*/
  status = sbFe2000NullProfileCreate(pPm);
  if(status != SB_OK) {
     SB_LOGV1("error: null profile create failed\n");
  }

  /* enable the global refresh */
  uValue = SAND_HAL_FE2000_READ(pPm->PolicerConfig.regSet,PM_CONFIG0);
  uValue = SAND_HAL_FE2000_MOD_FIELD(pPm->PolicerConfig.regSet, PM_CONFIG0, REFRESH_ENABLE, uValue, 1);
  uAddress = SAND_HAL_FE2000_REG_OFFSET(pPm->PolicerConfig.regSet, PM_CONFIG0);
  SAND_HAL_FE2000_WRITE_OFFS(pPm->PolicerConfig.regSet, uAddress, uValue);

  return SB_OK;

}
                             
/********************************************
 * Policer Profile Management 
 ********************************************/

sbStatus_t
sbFe2000ProfileCreate(sbFe2000Pm_t *pPm, uint32_t uProfileId, sbZfFe2000PmProfileMemory_t *pProfile)
{
  sbStatus_t uStatus;

  /* increment the reference count for this profile */

  sbZfFe2000PmProfileMemory_Copy(pProfile, 
                               &pPm->profiledb[uProfileId].profile);

  /* commit the null profile into hardware */
  uStatus = sbFe2000PmLocProfileMemoryWrite(pPm,uProfileId, pProfile);
  return uStatus;
}

sbStatus_t
sbFe2000ProfileDelete(sbFe2000Pm_t *pPm, uint32_t uProfileId)
{
  sbStatus_t uStatus;

  /* decrement the reference count for this profile */
  pPm->profiledb[uProfileId].uRefCount  = 0;

  /* reset the configuration */

  if (SAND_HAL_IS_FE2KXT (pPm->PolicerConfig.regSet ))
  {
    sbZfC2PmProfileMemory_InitInstance(&pPm->profiledb[uProfileId].C2Profile);
    uStatus = sbFe2000XtPmLocProfileMemoryWrite(pPm,uProfileId, &pPm->profiledb[uProfileId].C2Profile);
  }
  else
  {
    sbZfFe2000PmProfileMemory_InitInstance(&pPm->profiledb[uProfileId].profile);
    uStatus = sbFe2000PmLocProfileMemoryWrite(pPm,uProfileId, &pPm->profiledb[uProfileId].profile);
  }

  /* commit the profile into hardware */
  return uStatus;
}

/*
 * The null profile is for all the policers which are non operational.
 * This profile always exists in the system and can never be deleted.
 */
sbStatus_t
sbFe2000NullProfileCreate(sbFe2000Pm_t *pPm)
{
  sbStatus_t uStatus = SB_OK;
  uint32_t uProfileId = SB_FE_2000_PM_NULL_PROFILE_ID;
  sbZfFe2000PmProfileMemory_t zProfile;
  sbZfC2PmProfileMemory_t zXtProfile;

  /* initialize to default */
  sbZfFe2000PmProfileMemory_InitInstance(&zProfile);
  sbZfC2PmProfileMemory_InitInstance(&zXtProfile);

  /* create & update the profile db */

  if (SAND_HAL_IS_FE2KXT (pPm->PolicerConfig.regSet ))
  {
      uStatus = sbFe2000XtProfileCreate(pPm, uProfileId, &zXtProfile);
  }
  else
  {
      uStatus = sbFe2000ProfileCreate(pPm, uProfileId, &zProfile);
  }
  return SB_OK;
}

sbStatus_t 
sbFe2000PmProfileAlloc(sbFe2000Pm_t *pPm, sbZfFe2000PmProfileMemory_t *pProfile, uint32_t *pIndex)
{
  sbZfFe2000PmProfilePolicerMemory_t    zPolicerProfile, zPolicerProfileData;
  sbZfFe2000PmProfileTimerMemory_t      zTimerProfile, zTimerProfileData;
  sbStatus_t                          uStatus = SB_OK;

  uint8_t                             uProfileBytes[SB_ZF_FE_2000_PM_PROFMEMORY_SIZE_IN_BYTES];
  uint32_t                            uId =0, bFound =0;
  

  /* see if there is a matching profile already configured. */
  bFound = 0;
  for(uId = (SB_FE_2000_PM_NULL_PROFILE_ID+1); uId < SB_FE_2000_PM_MAX_NUM_PROFILES;uId++) {
    if(pPm->profiledb[uId].uRefCount ) 
    {
      if( pProfile->uProfType == SB_FE_2000_PM_PROF_TYPE_POLICER )
      {
        sbZfFe2000PmProfilePolicerMemory_InitInstance( &zPolicerProfile );
        
        sbZfFe2000PmProfileMemory_Pack( pProfile, &uProfileBytes[0], SB_ZF_FE_2000_PM_PROFMEMORY_SIZE_IN_BYTES );
        sbZfFe2000PmProfilePolicerMemory_Unpack( &zPolicerProfile, &uProfileBytes[0], sizeof(zPolicerProfile) ) ;

        sbZfFe2000PmProfilePolicerMemory_InitInstance( &zPolicerProfileData );
        SB_MEMSET( &uProfileBytes[0], 0, sizeof(uProfileBytes) );

        sbZfFe2000PmProfileMemory_Pack( (&pPm->profiledb[uId].profile), &uProfileBytes[0], 
                                      SB_ZF_FE_2000_PM_PROFMEMORY_SIZE_IN_BYTES );
        sbZfFe2000PmProfilePolicerMemory_Unpack( &zPolicerProfileData,  &uProfileBytes[0], sizeof(zPolicerProfileData));

        if( sbZfFe2000PmProfilePolicerMemory_Compare(&zPolicerProfile, &zPolicerProfileData) )
        {
          bFound = 1; *pIndex = uId;
          break;
        }
      } else if( pProfile->uProfType == SB_FE_2000_PM_PROF_TYPE_OAM ) {
        if( pProfile->uOamProfType == SB_FE_2000_PM_OAMPROF_TYPE_TIMER ) {
          sbZfFe2000PmProfileTimerMemory_InitInstance( &zTimerProfile );
        
          sbZfFe2000PmProfileMemory_Pack( pProfile, &uProfileBytes[0], SB_ZF_FE_2000_PM_PROFMEMORY_SIZE_IN_BYTES );
          sbZfFe2000PmProfileTimerMemory_Unpack( &zTimerProfile, &uProfileBytes[0], sizeof(zTimerProfile) ) ;

          sbZfFe2000PmProfileTimerMemory_InitInstance( &zTimerProfileData );
          SB_MEMSET( &uProfileBytes[0], 0, sizeof(uProfileBytes) );

          sbZfFe2000PmProfileMemory_Pack( (&pPm->profiledb[uId].profile), &uProfileBytes[0], 
                                        SB_ZF_FE_2000_PM_PROFMEMORY_SIZE_IN_BYTES );
          sbZfFe2000PmProfileTimerMemory_Unpack( &zTimerProfileData,  &uProfileBytes[0], sizeof(zTimerProfileData));

          if( sbZfFe2000PmProfileTimerMemory_Compare(&zTimerProfile, &zTimerProfileData) )
          {
            bFound = 1; *pIndex = uId;
            break;
          }
        } else if( pProfile->uOamProfType == SB_FE_2000_PM_OAMPROF_TYPE_SEQGEN ) {
          if( sbZfFe2000PmProfileSeqGenMemory_Compare((sbZfFe2000PmProfileSeqGenMemory_t *)pProfile, 
                                                    (sbZfFe2000PmProfileSeqGenMemory_t *)&pPm->profiledb[uId].profile) )
          {
            /* Compare always returns 1 */
            bFound = 1; *pIndex = uId;
            break;
          }
        }
      }
    }
  }

  if(!bFound) {
    for(uId = (SB_FE_2000_PM_NULL_PROFILE_ID+1);
        uId < SB_FE_2000_PM_MAX_NUM_PROFILES;
        uId++) {
      if(pPm->profiledb[uId].uRefCount == 0) {
        bFound =1;
        *pIndex = uId;
        break;
      }
    }

    if(!bFound) {
      return SB_PM_TOO_MANY_POLICERS;
    }

    /* configure the new profile to hardware */
    uStatus = sbFe2000ProfileCreate(pPm, *pIndex, pProfile);
  }

  pPm->profiledb[*pIndex].uRefCount++;
  return uStatus;
}

/**
 * @fn AllocMemChunk()
 * 
 * @brief - Allocate a chunk of memory, and split it into policers that
 *          are then added onto the free list. This allocates memory
 *          for batches of policers together.
 *
 * @param pPm - Pointer to Policer Management Unit control structure
 *
 * @return - SB_OK on success, error code otherwise
 **/
PRIVATE sbStatus_t
AllocMemChunk(sbFe2000Pm_t *pPm) 
{
  sbFe2000PmPolicer_t      *pPolicer;
  uint32_t               i;
  void                  *vp;

  SB_ASSERT(pPm);

  if (MAX_PM_MEM_CHUNKS <= pPm->uChunkCount) {
    return SB_MALLOC_FAILED;
  }

  /* allocate chunk of elements */
  vp = sal_alloc(pPm->uPolicersPerChunk * sizeof(sbFe2000PmPolicer_t), "Policers");

  if (!vp) {
    return SB_MALLOC_FAILED;
  }

  /* array of Policers */
  pPolicer = (sbFe2000PmPolicer_t *) vp;
  
  /* go through and them individually to free list */
  for (i = 0; i < pPm->uPolicersPerChunk; i++) {
    /* init element struct */
    DQ_INIT((dq_p_t) &pPolicer[i]);

    /* init rule struct */
    pPolicer[i].uMemChunkId = pPm->uChunkCount;
    pPolicer[i].uPolicerId  = 0; /* fix me */
    pPolicer[i].uProfileId  = 0; /* fix me */

    /* push onto free list */
    DQ_INSERT_TAIL((dq_p_t) &pPm->listFreePolicers, &pPolicer[i].listElem);
  }

  /* keep track of how many chunks have been allocated */
  pPm->aMemChunks[pPm->uChunkCount++] = vp;
  return SB_OK;
}

/**
 * @fn AllocPolicer()
 * 
 * @brief 
 *
 * Get Policer from free list. If we're out of Policers, allocate another memory 
 * chunk to satisfy the request. Add remaining memory to the free-list
 *
 * @param pPm    - Pointer to PMU control structure
 * @param ppPolicer - Policer pointer (passed by reference, returns pointer to rule)
 *
 * @return SB_OK on success, error code otherwise
 **/
PRIVATE sbStatus_t
AllocPolicer(sbFe2000Pm_t *pPm, sbFe2000PmPolicer_t **pPolicer) 
{
  sbFe2000PmPolicer_t *pElem = NULL;
  sbStatus_t status;

  /* get a rule off free list */
  if (DQ_EMPTY((dq_p_t) &pPm->listFreePolicers) == FALSE)
    DQ_REMOVE_HEAD((dq_p_t) &pPm->listFreePolicers, pElem);
  
  /* if successful, return it */
  if (pElem) {
    *pPolicer = (sbFe2000PmPolicer_t *) pElem;
    return SB_OK;
  }
    
  /* no Policer avail free list, so allocate another memory chunk */
  status = AllocMemChunk(pPm);
  if (status != SB_OK) {
    return status;
  }

  /* no try and get a policer again off 'refreshed' free list */
  if (DQ_EMPTY((dq_p_t) &pPm->listFreePolicers) == FALSE)
    DQ_REMOVE_HEAD((dq_p_t) &pPm->listFreePolicers, pElem);
  
  /* if successful, return it */
  if (pElem) {
    *pPolicer = (sbFe2000PmPolicer_t *) pElem;
    return SB_OK;
  }
  
  /* unreachable, should have failed AllocMemChunk() earlier */
  SB_ASSERT(0);
  return SB_MALLOC_FAILED;
}

/**
 * @fn FreePolicer()
 * 
 * @brief - Return a Policer to the free-list. 
 *          This will allow the Policer to be
 *          reused later by later allocations.
 *
 * @param pPm   - Pointer to PMU control structure
 * @param pPolicer - Policer pointer
 *
 * @return SB_OK on success, error code otherwise
 **/
PRIVATE sbStatus_t
FreePolicer(sbFe2000Pm_t *pPm, sbFe2000PmPolicer_t *pPolicer) 
{
  pPolicer->uPolicerId  = 0; /* fix me */
  pPolicer->uProfileId  = 0; /* fix me */
  DQ_INSERT_HEAD(&pPm->listFreePolicers, &pPolicer->listElem);
  return SB_OK;
}

/* 
 * The goal of this function is to get the reference
 * timer tick from the npu clock frequency. Let's say 
 * if we want to generate a timer tick for rate R bps
 * then you want to generate a reference timer tick 
 * at every K=((R*1000)/8) BYTES/sec. i.e 1 BYTE per
 * for every 1/K seconds. so if we give one unit token 
 * at every 1/K seconds measured in terms of clock frequency 
 * we should be ok. So the time measured is , 1/K / 1/CpuFreq 
 * = CpuFreq/K. ( we want to convert into Bytes per sec
 * as the CIR/EIR will be specified in Bytes Per Second.
 */ 

sbStatus_t  
sbFe2000PolicerTimerTickGet(sbFe2000Pm_t *pPm, sbFe2000PmPolicerRate_t rate, uint32_t *pTimerTick)
{
  uint32_t uTimerTick = 0;
  uint32_t kbps, uRBps = 0; /* Bytes Per second */
  uint32_t uNpuClockFreq =0;

  switch(rate) 
  {
  case SB_FE_2000_PM_PRATE_1KBPS:    kbps = 1;     break;
  case SB_FE_2000_PM_PRATE_10KBPS:   kbps = 10;    break;
  case SB_FE_2000_PM_PRATE_100KBPS:  kbps = 100;   break;
  case SB_FE_2000_PM_PRATE_250KBPS:  kbps = 250;   break;
  case SB_FE_2000_PM_PRATE_500KBPS:  kbps = 500;   break;
  case SB_FE_2000_PM_PRATE_1MBPS:    kbps = 1e3;   break;
  case SB_FE_2000_PM_PRATE_10MBPS:   kbps = 10e3;  break;
  case SB_FE_2000_PM_PRATE_100MBPS:  kbps = 100e3; break;
  case SB_FE_2000_PM_PRATE_1GBPS:    kbps = 1e6;   break;
  case SB_FE_2000_PM_PRATE_10GBPS:   kbps = 10e6;  break;
  default: {
    return SB_BAD_ARGUMENT_ERR_CODE;
  }
    break;
  }
  
  if (SAND_HAL_IS_FE2KXT (pPm->PolicerConfig.regSet ))
  {
    /* uNpuClockFreq = SB_FE_2000XT_PM_NPU_CLOCK_FREQ; */
    if        (pPm->uHppFreq == 400000) {
        uNpuClockFreq = 400.0E6;
    } else if (pPm->uHppFreq == 375000) {
        uNpuClockFreq = 375.0E6;
    } else if (pPm->uHppFreq == 300000) {
        uNpuClockFreq = 300.0E6;
    } else if (pPm->uHppFreq == 262500) {
        uNpuClockFreq = 262.500E6;
    } else if (pPm->uHppFreq == 258333) {
        uNpuClockFreq = 258.333E6;
    } else if (pPm->uHppFreq == 250000) {
        uNpuClockFreq = 250.0E6;
    } else {
        SB_ASSERT(0);
    }
  }
  else
  {
    uNpuClockFreq = SB_FE_2000_PM_NPU_CLOCK_FREQ;
  }

  uRBps = (kbps * 1000) / 8;
  uTimerTick = uNpuClockFreq / uRBps;
  *pTimerTick = uTimerTick;
  return SB_OK;
}

sbStatus_t
sbFe2000PmPolicerBestFitProfileGet(sbZfFe2000PmPolicerConfig_t *pConfig, sbZfFe2000PmProfilePolicerMemory_t *pProfile)
{
  uint32_t uTemp;
  sbStatus_t rv;

  sbZfFe2000PmProfilePolicerMemory_InitInstance(pProfile);
  
  pProfile->uType = SB_FE_2000_PM_METERTYPE_POLICER;
  /* 
   * we can enhance this further later to respect these
   * rfc modes.
   */
  pProfile->uEIR = pConfig->uEIR;
  pProfile->uCIR = pConfig->uCIR;

  /* configure the blind mode */
  pProfile->bBlind = pConfig->bBlindMode;

  /* configure the drop on red policy */
  pProfile->bDropOnRed = pConfig->bDropOnRed;

  /* configure the coupling between bktc & bkte carry over */
  pProfile->bCouplingFlag = pConfig->bCoupling;

  /* copy no decrement policies */
  pProfile->bBktCNoDecrement = pConfig->bCBSNoDecrement;
  pProfile->bBktENoDecrement = pConfig->bEBSNoDecrement;

  /* encode CBS & EBS to mantissa and exponent */

  /* 
   * force some parameters as per the rfc mode 
   * Todo: clean the user policer configuration.
   */
  if(pConfig->uRfcMode == SB_FE_2000_PM_PMODE_RFC_2697) {
    pProfile->uEIR = 0;
    pProfile->bCouplingFlag = pConfig->bCoupling;
  } else if(pConfig->uRfcMode == SB_FE_2000_PM_PMODE_RFC_2698) {
    pProfile->bRFC2698Mode = 1;
  } else if(pConfig->uRfcMode == SB_FE_2000_PM_PMODE_RFC_4115) {
  } else if(pConfig->uRfcMode == SB_FE_2000_PM_PMODE_RFC_MEF) {
    pProfile->bCouplingFlag = pConfig->bCoupling;
  } else {
    /* error */
  }

  rv = sbFe2000PmIEEECodingGet(pConfig->uCBS, &pProfile->uCBS);
  if (rv != SB_OK) {
    return rv;
  }

  rv = sbFe2000PmIEEECodingGet(pConfig->uEBS, &pProfile->uEBS);
  if (rv != SB_OK) {
    return rv;
  }

  /* find the number bits required to represent the bktc */
  pProfile->uBktCSize = 0;
  uTemp = pConfig->uCBS;
  while(uTemp) {
    pProfile->uBktCSize++;
    uTemp = uTemp >> 1;
  }
 
  /* for now length shift is always zero 
   * don't trade off accuracy yet.
   */
  /* pProfile->uLengthShift = 0; */
  pProfile->uLengthShift = pConfig->uLenShift;
  return SB_OK;
}

sbStatus_t
sbFe2000PmLocProfileMemoryWrite(sbFe2000Pm_t *pPm, uint32_t uProfileId, sbZfFe2000PmProfileMemory_t *pProfile)
{
  sbStatus_t uStatus;
  uint32_t uProfileWords[SB_ZF_FE_2000_PM_PROFMEMORY_SIZE_IN_WORDS + 1];
  /* +1: C2 has an extra word and sbFe2000PmProfileMemoryWrite is common for FE2000 and C2 */

  /* make wods from the zframe */
  sbZfFe2000PmProfileMemory_Pack32(pProfile, &uProfileWords[0], SB_ZF_FE_2000_PM_PROFMEMORY_SIZE_IN_WORDS);

  /* write to the profile memory */
  uStatus = sbFe2000PmProfileMemoryWrite(pPm->PolicerConfig.regSet,uProfileId,&uProfileWords[0]);
  return uStatus;
}

sbStatus_t
sbFe2000PmLocProfileMemoryRead(sbFe2000Pm_t *pPm, uint32_t uProfileId, sbZfFe2000PmProfileMemory_t *pProfile)
{
  sbStatus_t uStatus;
  uint32_t uProfileWords[SB_ZF_FE_2000_PM_PROFMEMORY_SIZE_IN_WORDS + 1];
  /* +1: C2 has an extra word and sbFe2000PmProfileMemoryRead is common for FE2000 and C2 */

  /* write to the profile memory */
  uStatus = sbFe2000PmProfileMemoryRead(pPm->PolicerConfig.regSet,uProfileId,&uProfileWords[0]);
  if( uStatus != SB_OK )
  {
    return uStatus;
  }

  /* make zframe from the words */
  sbZfFe2000PmProfileMemory_Unpack32( pProfile, &uProfileWords[0], SB_ZF_FE_2000_PM_PROFMEMORY_SIZE_IN_WORDS);

  return SB_OK;
}

sbStatus_t
sbFe2000PmPolicerRefreshSet(sbFe2000Pm_t *pPm, uint32_t uPolicerId, uint32_t bEnable)
{
  uint32_t uValue = 0;

  uValue = SAND_HAL_FE2000_MOD_FIELD(pPm->PolicerConfig.regSet, PM_SUPPRESS_WRITE_CONFIG, SUPPRESS, uValue, bEnable);
  uValue = SAND_HAL_FE2000_MOD_FIELD(pPm->PolicerConfig.regSet, PM_SUPPRESS_WRITE_CONFIG, POLID, uValue, uPolicerId);
  SAND_HAL_FE2000_WRITE(pPm->PolicerConfig.regSet, PM_SUPPRESS_WRITE_CONFIG, uValue);
  return SB_OK;
}

sbStatus_t
sbFe2000PmGroupGet(sbFe2000Pm_t *pPm, sbFe2000PmPolicerRate_t rate, uint32_t *pGroup)
{
  uint32_t uGroup =0xFF;

  *pGroup = uGroup;
  /* 
   * find the group that this one is going to go 
   */

  for(uGroup=SB_FE_2000_PM_TYPED_POLICER_GROUP_BASE;
      uGroup < SB_FE_2000_PM_NUM_OF_GROUPS; uGroup++) {
    if(rate == pPm->PolicerConfig.rates[uGroup]) {
      *pGroup = uGroup;
      break;
    }
  }
  if(*pGroup == 0xFF) {
    /* error : not able to find the base rate */
    return SB_BAD_ARGUMENT_ERR_CODE;
  }
  return SB_OK;
}

/* 
 * Both CBS & EBS are represented in IEEE standard form of
 * notation for numbers which (128+m) * 2 ^(E-8) where m is
 * 7 bits i.e 127 is the maximum value and E is 5 bits which 
 * means 31 is the maximum value it can support. Though hardware
 * discards any fractional generated due to E < 8 and results 0 
 * for E=0. And for E > 28 it consider the number as 0xFFFFFFF
 * see section 3.9.3.1.1 of the FE2000 spec. The goal of this
 * whole function is find the best fit values for m and E.
 *
 */

sbStatus_t
sbFe2000PmIEEECodingGet(uint32_t uValue, uint32_t *pEncodedValue)
{
  uint32_t uExponent = 0;
  uint32_t uBytes = uValue;
  uint32_t uBFMantissa = 0, uBFExponent =0;
  uint32_t bFound =0;
  /* reset the encoded value */
  *pEncodedValue = 0;

  if (uValue == 0) {
    return SB_OK;
  }

  if(uBytes <= 255) {
    bFound =0;
    /* find the exponent find */
    for(uExponent=0; uExponent < 8 ; uExponent++) {
      if( (uBytes >> uExponent) == 1 ) {
        bFound = 1;
        uBFExponent = uExponent + 1;
        break;
      }
    } /* end of exponent find */

    if(!bFound) {
      soc_cm_print("ERROR: No valid conversion found for %d\n", uValue);
      return SB_BAD_ARGUMENT_ERR_CODE;
      /* error */
    }
    /* for the best fit Exponent find the mantissa */
    uBFMantissa = uBytes << (8 - uBFExponent);
    *pEncodedValue  = ( (uBFMantissa & 0x7F) << 5) | (uBFExponent & 0x1F);
    return SB_OK;
  } else {
  /* rest of the code is for uBytes > 255 */
  bFound =0;
  /* find the exponent find */
  for(uExponent=8; uExponent <=31 ; uExponent++) {
    if( (uBytes >> uExponent) == 1 ) {
      bFound = 1;
      uBFExponent = uExponent + 1;
      break;
    }
  } /* end of exponent find */

  if(!bFound) {
    soc_cm_print("ERROR: No valid conversion found for %d\n", uValue);
    return SB_BAD_ARGUMENT_ERR_CODE;
    /* error */
  }

  /* for the best fit Exponent find the mantissa */
  uBFMantissa = uBytes >> (uBFExponent - 8);
  *pEncodedValue  = ( (uBFMantissa & 0x7F) << 5) | (uBFExponent & 0x1F);
  return SB_OK;
  }
}

/* 
 * The goal of this function is to get the reference
 * rate from the timer tick. 
 * inverse of "PolicerTimerTickGet"
 */ 
sbStatus_t  
sbFe2000PolicerGroupRateGet(sbFe2000Pm_t *pPm, uint32_t uTimerTick, sbFe2000PmPolicerRate_t *uRate)
{
  uint32_t uRps = 0; /* Per second */
  uint32_t uNpuClockFreq =0;

  uTimerTick = (uTimerTick * 1000)/8;

  if (SAND_HAL_IS_FE2KXT (pPm->PolicerConfig.regSet ))
  {
    /* uNpuClockFreq = SB_FE_2000XT_PM_NPU_CLOCK_FREQ; */
    if        (pPm->uHppFreq == 400000) {
        uNpuClockFreq = 400.0E6;
    } else if (pPm->uHppFreq == 375000) {
        uNpuClockFreq = 375.0E6;
    } else if (pPm->uHppFreq == 300000) {
        uNpuClockFreq = 300.0E6;
    } else if (pPm->uHppFreq == 262500) {
        uNpuClockFreq = 262.500E6;
    } else if (pPm->uHppFreq == 258333) {
        uNpuClockFreq = 258.333E6;
    } else if (pPm->uHppFreq == 250000) {
        uNpuClockFreq = 250.0E6;
    } else {
        SB_ASSERT(0);
    }
  }
  else
  {
    uNpuClockFreq = SB_FE_2000_PM_NPU_CLOCK_FREQ;
  }


  uRps = uNpuClockFreq/uTimerTick;

  switch(uRps)
  {
  case 1:
    *uRate = SB_FE_2000_PM_PRATE_1KBPS;
    break;
  case 10:
    *uRate = SB_FE_2000_PM_PRATE_10KBPS;
    break;
  case 100:
    *uRate = SB_FE_2000_PM_PRATE_100KBPS;
    break;
  case 250:
    *uRate = SB_FE_2000_PM_PRATE_250KBPS;
    break;
  case 500:
    *uRate = SB_FE_2000_PM_PRATE_500KBPS;
    break;
  case 1000:
    *uRate = SB_FE_2000_PM_PRATE_1MBPS;
    break;
  case 10000:
    *uRate = SB_FE_2000_PM_PRATE_10MBPS;
    break;
  case 100000:
    *uRate = SB_FE_2000_PM_PRATE_100MBPS;
    break;
  case 1000000:
    *uRate = SB_FE_2000_PM_PRATE_1GBPS;
    break;
  default:
      return SB_BAD_ARGUMENT_ERR_CODE;
  }
  return SB_OK;
}

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
                      sbZfFe2000PmPolicerConfig_t *pConfig)
{
  return sbFe2000PmPolicerReadExt(pPm, ulPolicerId, pConfig, NULL);
}


sbStatus_t
sbFe2000PmPolicerReadExt(sbFe2000Pm_t *pPm,
                         uint32_t ulPolicerId,
                         sbZfFe2000PmPolicerConfig_t *pConfig,
                         uint32_t *pProfileId)
{
  sbZfFe2000PmPolicerRawFormat_t        zPolicer;
  sbStatus_t                          uStatus;
  uint32_t                            uProfileId;
  sbZfFe2000PmProfileMemory_t           zProfile;
  sbZfFe2000PmProfilePolicerMemory_t    zPolicerProfile;
  sbZfC2PmProfileMemory_t           zXtProfile;
  sbZfC2PmProfilePolicerMemory_t    zXtPolicerProfile;
  uint8_t                             uPolicerBytes[SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_SIZE_IN_BYTES];
  uint8_t                             uXtPolicerBytes[SB_ZF_C2_PM_PROFPOLICERMEMORY_SIZE_IN_BYTES];

  sbZfFe2000PmProfileMemory_InitInstance(&zProfile);
  sbZfC2PmProfileMemory_InitInstance(&zXtProfile);
  sbZfFe2000PmPolicerRawFormat_InitInstance(&zPolicer);
  sbZfFe2000PmProfilePolicerMemory_InitInstance(&zPolicerProfile);
  sbZfC2PmProfilePolicerMemory_InitInstance(&zXtPolicerProfile);

  /* Verify the Policer Id is in the range specified. */
  if (ulPolicerId > SB_FE2000_PM_MAX_POLICER_ID)
  {
    return SB_BAD_ARGUMENT_ERR_CODE;
  }

  /* Retrieve the Policer Details */
  sbZfFe2000PmPolicerRawFormat_InitInstance(&zPolicer);

  uStatus = sbFe2000PmPolicerRawGet(pPm, ulPolicerId, &zPolicer);
  if(uStatus != SB_OK) {
    return uStatus;
  }
  /* copy the current profile id */
  uProfileId = zPolicer.uProfileId;
  if (pProfileId) {
    *pProfileId = uProfileId;
  }

  /* Copy into database depending on whether it is a Policer or OAM Entity */

  if (SAND_HAL_IS_FE2KXT (pPm->PolicerConfig.regSet ))
  {
    uStatus = sbFe2000XtPmLocProfileMemoryRead( pPm, uProfileId, &zXtProfile);
  }
  else{
    uStatus = sbFe2000PmLocProfileMemoryRead( pPm, uProfileId, &zProfile);
  }

  if( uStatus != SB_OK )
  {
    return uStatus;
  }

  if (SAND_HAL_IS_FE2KXT (pPm->PolicerConfig.regSet ))
  {
      sbZfC2PmProfileMemory_Pack( &zXtProfile, &uXtPolicerBytes[0], sizeof(zXtProfile) );
      sbZfC2PmProfilePolicerMemory_Unpack( &zXtPolicerProfile, &uXtPolicerBytes[0], sizeof(zXtPolicerProfile) ) ;
  }
  else{
      sbZfFe2000PmProfileMemory_Pack( &zProfile, &uPolicerBytes[0], sizeof(zProfile) );
      sbZfFe2000PmProfilePolicerMemory_Unpack( &zPolicerProfile, &uPolicerBytes[0], sizeof(zPolicerProfile) ) ;
  }


  if (SAND_HAL_IS_FE2KXT (pPm->PolicerConfig.regSet ))
  {
      if( zXtPolicerProfile.uType == SB_FE_2000_PM_PROF_TYPE_POLICER )
      {
        pConfig->uLenShift = zXtPolicerProfile.uLengthShift;
        /* pConfig->uRate = pPm->profiledb[uProfileId].profile. */
        pConfig->uCBS = zXtPolicerProfile.uCBS;
        pConfig->uCIR = zXtPolicerProfile.uCIR;
        pConfig->uEBS = zXtPolicerProfile.uEBS;
        pConfig->uEIR = zXtPolicerProfile.uEIR;
        pConfig->uRfcMode = zXtPolicerProfile.bRFC2698Mode;
        pConfig->bBlindMode = zXtPolicerProfile.bBlind;
        pConfig->bDropOnRed = zXtPolicerProfile.bDropOnRed;
        pConfig->bCoupling = zXtPolicerProfile.bCouplingFlag;
        pConfig->bCBSNoDecrement = zXtPolicerProfile.bBktCNoDecrement;
        pConfig->bEBSNoDecrement = zXtPolicerProfile.bBktCNoDecrement;
      }
      else
      {
        return SB_BAD_ARGUMENT_ERR_CODE;
      }
   }
   else
   {
      if( zPolicerProfile.uType == SB_FE_2000_PM_PROF_TYPE_POLICER )
      {
        pConfig->uLenShift = zPolicerProfile.uLengthShift;
        /* pConfig->uRate = pPm->profiledb[uProfileId].profile. */
        pConfig->uCBS = zPolicerProfile.uCBS;
        pConfig->uCIR = zPolicerProfile.uCIR;
        pConfig->uEBS = zPolicerProfile.uEBS;
        pConfig->uEIR = zPolicerProfile.uEIR;
        pConfig->uRfcMode = zPolicerProfile.bRFC2698Mode;
        pConfig->bBlindMode = zPolicerProfile.bBlind;
        pConfig->bDropOnRed = zPolicerProfile.bDropOnRed;
        pConfig->bCoupling = zPolicerProfile.bCouplingFlag;
        pConfig->bCBSNoDecrement = zPolicerProfile.bBktCNoDecrement;
        pConfig->bEBSNoDecrement = zPolicerProfile.bBktENoDecrement;
      }
      else
      {
        return SB_BAD_ARGUMENT_ERR_CODE;
      }

   }
  return SB_OK;
}

/*
 * This function gets the Policer Group Information.
 * 
 * @param pPm             Pointer to the sbFe2000Pm_t struct
 * @param uGroupId        Policer Group, base rate of which needs to be modified.
 * @param *pConfig        Pointer to the Policer Group Configuration.
 */
sbStatus_t
sbFe2000PmPolicerGroupRead( sbFe2000Pm_t *pPm,
                          uint32_t uGroupId,
                          sbFe2000PmGroupConfig_t *pConfig)
{
  sbStatus_t  uStatus;

  /* Verify the Group ID is in expected range */
  if (uGroupId > SB_FE_2000_PM_NUM_TOTAL_GROUPS)
  {
    return SB_BAD_ARGUMENT_ERR_CODE;
  }

  /* Lookup the Group to retrieve the Group Information */
  uStatus = sbFe2000PmGroupConfigGet( pPm->PolicerConfig.regSet, uGroupId, &(pConfig->groupConfig) );
  if( uStatus != SB_OK )
  {
    return uStatus;
  }
  
  /* Populate into the corresponding table and send it out. */
  /* Recalculate the Rate and push it back in */
  uStatus = sbFe2000PolicerGroupRateGet( pPm, pConfig->groupConfig.uTimerTickPeriod, &(pConfig->uRate) );
  if( uStatus != SB_OK )
  {
    return uStatus;
  }
  
  return SB_OK;
}


/************************** OAM ENTITIES ******************************/
sbStatus_t
sbFe2000PmOamTimerBestFitProfileGet(sbZfFe2000PmOamTimerConfig_t *pConfig, 
                                  sbZfFe2000PmProfileTimerMemory_t *pProfile)
{
  sbZfFe2000PmProfileTimerMemory_InitInstance(pProfile);
  
  pProfile->uType = SB_FE_2000_PM_PROF_TYPE_OAM;
  pProfile->uProfMode = SB_FE_2000_PM_OAMPROF_TYPE_TIMER;


  pProfile->uDeadline = pConfig->uDeadline;

  pProfile->bInterrupt = pConfig->bInterrupt;

  pProfile->bReset = pConfig->bReset;

  pProfile->bStrict = pConfig->bStrict;

  pProfile->bStarted = pConfig->bStarted;

  return SB_OK;
}

/*
 * This function configures a policer and returns
 * the index of the created  policer id.
 * 
 * @param pPm             Pointer to the sbFe2000Pm_t struct
 * @param uTimerId      Timer Index
 * @param *pConfig        Pointer to the Timer Configuration.
 * @param *pIndex         pointer to the policer index value
 * @return                Return status of the type sbStatus_t
 */
sbStatus_t
sbFe2000PmOamTimerCreate( sbFe2000Pm_t     *pPm,
                        uint32_t      ulTimerId,
                        sbZfFe2000PmOamTimerConfig_t *pConfig,
                        uint32_t *pIndex)
{
  sbStatus_t  uStatus;
  uint32_t    grp=0xFF;
  uint32_t    uMinPolicerId = 0,uMaxPolicerId = 0;
  uint32_t    uAddress = 0, uValue =0;
  uint32_t    uTimerId=0, uProfileId= 0xFFFF;
  uint8_t     bFound = 0;
  uint8_t     uTimerBytes[SB_ZF_FE_2000_PM_PROFTIMERMEMORY_SIZE_IN_BYTES];
  uint8_t     uXtTimerBytes[SB_ZF_C2_PM_PROFTIMERMEMORY_SIZE_IN_BYTES];

  sbZfFe2000PmProfileTimerMemory_t  zProfile;
  sbZfC2PmProfileTimerMemory_t      zXtProfile;
  sbZfFe2000PmProfileMemory_t       zCommonProfile;
  sbZfC2PmProfileMemory_t           zXtCommonProfile;
  sbZfFe2000PmOamTimerRawFormat_t   zTimer;
  sbZfFe2000PmGroupConfig_t         zGroupConfig;
  sbFe2000PmPolicer_t               *pPolicerRef = NULL;

  sbZfFe2000PmProfileTimerMemory_InitInstance(&zProfile);
  sbZfC2PmProfileTimerMemory_InitInstance(&zXtProfile);
  sbZfFe2000PmOamTimerRawFormat_InitInstance(&zTimer);
  sbZfFe2000PmProfileMemory_InitInstance(&zCommonProfile);
  sbZfC2PmProfileMemory_InitInstance(&zXtCommonProfile);
  SB_MEMSET(&uTimerBytes[0], 0, SB_ZF_FE_2000_PM_PROFTIMERMEMORY_SIZE_IN_BYTES);
  SB_MEMSET(&uXtTimerBytes[0], 0, SB_ZF_C2_PM_PROFTIMERMEMORY_SIZE_IN_BYTES);
  sbZfFe2000PmGroupConfig_InitInstance(&zGroupConfig);

  /* 
   * step 1: get the oam timer group.
   */
  /* Force group to the oam timers.  GroupGet searches based on rate, and
   * will return a policer group instead of the timers group if the policers
   * are configured to the same rate
   */
  grp = SB_FE_2000_PM_OAM_TIMERS_GROUP;

  /* 
   * step 2: see if this group has any policer left 
   * that can be assigned to this new policer
   */
  /* read this groups min and max policer id configured */
  uAddress = SAND_HAL_FE2000_REG_OFFSET(pPm->PolicerConfig.regSet, PM_GROUP0_CONFIG0) + 16*grp;
  uValue = SAND_HAL_FE2000_READ_OFFS(pPm->PolicerConfig.regSet, uAddress);
  uMinPolicerId = SAND_HAL_FE2000_GET_FIELD(pPm->PolicerConfig.regSet,PM_GROUP0_CONFIG0, POLID_MIN,uValue);
  uValue = SAND_HAL_FE2000_READ_OFFS(pPm->PolicerConfig.regSet, (uAddress+4));
  uMaxPolicerId = SAND_HAL_FE2000_GET_FIELD(pPm->PolicerConfig.regSet,PM_GROUP0_CONFIG1, POLID_MAX,uValue);

  /* Min & Max Policer Id stores the Policer Memory Bases */
  uMinPolicerId -= pPm->PolicerConfig.base;
  uMaxPolicerId -= pPm->PolicerConfig.base;

  if( (ulTimerId < uMinPolicerId)  || (ulTimerId > uMaxPolicerId) )
  {
    /* TimerId == 0 special case where Policer Id needs to be allocated */
    if( ulTimerId != 0 )
    {
      return SB_BAD_ARGUMENT_ERR_CODE;
    }
  }

  if( ulTimerId != 0 )
  {
    uTimerId = ulTimerId;
    bFound = 1;
  } else {
    /* caller must supply ID now that g2p3 is used. */
    return SB_BAD_ARGUMENT_ERR_CODE;
  }

  if(!bFound || (uTimerId>uMaxPolicerId) ) {
    return SB_PM_TOO_MANY_TIMERS;
  }

  /* 
   * step 3: find the profile that's a fit for this policer
   * configuration.
   */

  if (SAND_HAL_IS_FE2KXT (pPm->PolicerConfig.regSet ))
  {
      sbFe2000XtPmOamTimerBestFitProfileGet(pConfig, &zXtProfile);
      sbZfC2PmProfileTimerMemory_Pack( &zXtProfile, &uXtTimerBytes[0], SB_ZF_C2_PM_PROFTIMERMEMORY_SIZE_IN_BYTES );
      sbZfC2PmProfileMemory_Unpack( &zXtCommonProfile, &uXtTimerBytes[0], sizeof(zXtCommonProfile) ) ;
  }
  else{
      sbFe2000PmOamTimerBestFitProfileGet(pConfig, &zProfile);
      sbZfFe2000PmProfileTimerMemory_Pack( &zProfile, &uTimerBytes[0], SB_ZF_FE_2000_PM_PROFTIMERMEMORY_SIZE_IN_BYTES );
      sbZfFe2000PmProfileMemory_Unpack( &zCommonProfile, &uTimerBytes[0], sizeof(zCommonProfile) ) ;
  }

  if (SAND_HAL_IS_FE2KXT (pPm->PolicerConfig.regSet ))
  {
      uStatus = sbFe2000XtPmProfileAlloc(pPm, &zXtCommonProfile, &uProfileId);
  }
  else{
      uStatus = sbFe2000PmProfileAlloc(pPm, &zCommonProfile, &uProfileId);
  }
  if(uStatus != SB_OK) {
    return uStatus;
  }

  /* Add the policer configuration to profile ref list */
  AllocPolicer(pPm, &pPolicerRef);
  pPolicerRef->uPolicerId = uTimerId;
  pPolicerRef->uProfileId = uProfileId;
  pPolicerRef->uGroupId   = grp;
  DQ_INSERT_TAIL((dq_p_t) &pPm->profiledb[uProfileId].listPolicers, &pPolicerRef->listElem);

  /* suppress writes to this policer id before changing it's config */
  sbFe2000PmPolicerRefreshSet(pPm, uTimerId, SB_FE_2000_PM_DISABLE_REFRESH);

  zTimer.uProfileId  = uProfileId;
  zTimer.uTime =  0;
  zTimer.uStarted =  1;
  uStatus = sbFe2000PmOamTimerRawSet(pPm, uTimerId, &zTimer);

  /* let the refreshes go on */
  sbFe2000PmPolicerRefreshSet(pPm, uTimerId, SB_FE_2000_PM_ENABLE_REFRESH);

  /* return the configured policer id to the user */
  *pIndex = uTimerId;
  return uStatus;
}



/**
 * This function gets timer details.
 * 
 * @param pPm             Pointer to the sbFe2000Pm_t struct
 * @param uTimerId        Timer-Index
 * @param *pConfig        Pointer to the Timer Configuration.
 * @return                Return status of the type sbStatus_t
 */
sbStatus_t
sbFe2000PmOamTimerRead(sbFe2000Pm_t *pPm,
                     uint32_t ulTimerId,
                     sbZfFe2000PmOamTimerConfig_t *pConfig)
{
  sbZfFe2000PmOamTimerRawFormat_t       zTimer;
  sbStatus_t                            uStatus;
  uint32_t                              uProfileId;
  sbZfFe2000PmProfileMemory_t           zProfile;
  sbZfC2PmProfileMemory_t               zXtProfile;
  sbZfFe2000PmProfileTimerMemory_t      zTimerProfile;
  sbZfC2PmProfileTimerMemory_t          zXtTimerProfile;
  uint8_t                               uTimerBytes[SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_SIZE_IN_BYTES];
  uint8_t                               uXtTimerBytes[SB_ZF_C2_PM_PROFPOLICERMEMORY_SIZE_IN_BYTES];

  sbZfFe2000PmProfileMemory_InitInstance(&zProfile);
  sbZfC2PmProfileMemory_InitInstance(&zXtProfile);
  sbZfFe2000PmOamTimerRawFormat_InitInstance(&zTimer);
  sbZfFe2000PmProfileTimerMemory_InitInstance(&zTimerProfile);
  sbZfC2PmProfileTimerMemory_InitInstance(&zXtTimerProfile);

  /* Verify the Policer Id is in the range specified. */
  if (ulTimerId > SB_FE2000_PM_MAX_POLICER_ID)
  {
    return SB_BAD_ARGUMENT_ERR_CODE;
  }

  /* Retrieve the Timer Details */
  sbZfFe2000PmOamTimerRawFormat_InitInstance(&zTimer);

  uStatus = sbFe2000PmOamTimerRawGet(pPm, ulTimerId, &zTimer);
  if(uStatus != SB_OK) {
    return uStatus;
  }
  /* copy the current profile id */
  uProfileId = zTimer.uProfileId;

  /* Copy into database depending on whether it is a Policer or OAM Entity */

  if (SAND_HAL_IS_FE2KXT (pPm->PolicerConfig.regSet ))
  {
    uStatus = sbFe2000XtPmLocProfileMemoryRead( pPm, uProfileId, &zXtProfile);
  }
  else 
  {
    uStatus = sbFe2000PmLocProfileMemoryRead( pPm, uProfileId, &zProfile);
  }  

  if( uStatus != SB_OK )
  {
    return uStatus;
  }

  if (SAND_HAL_IS_FE2KXT (pPm->PolicerConfig.regSet ))
  {
    sbZfC2PmProfileMemory_Pack( &zXtProfile, &uXtTimerBytes[0], sizeof(zXtProfile) );
    sbZfC2PmProfileTimerMemory_Unpack( &zXtTimerProfile, &uXtTimerBytes[0], sizeof(zXtTimerProfile) );

    if( zXtTimerProfile.uType == SB_FE_2000_PM_PROF_TYPE_OAM )
    {
      pConfig->bInterrupt = zXtTimerProfile.bInterrupt;
      /* pConfig->uRate = pPm->profiledb[uProfileId].profile. */
      pConfig->bReset = zXtTimerProfile.bReset;
      pConfig->bStrict = zXtTimerProfile.bStrict;
      pConfig->bStarted = zXtTimerProfile.bStarted;
      pConfig->uDeadline = zXtTimerProfile.uDeadline;
    }
    else
    {
      return SB_BAD_ARGUMENT_ERR_CODE;
    }
  }
  else
  {
    sbZfFe2000PmProfileMemory_Pack( &zProfile, &uTimerBytes[0], sizeof(zProfile) );
    sbZfFe2000PmProfileTimerMemory_Unpack( &zTimerProfile, &uTimerBytes[0], sizeof(zTimerProfile) );

    if( zTimerProfile.uType == SB_FE_2000_PM_PROF_TYPE_OAM )
    {
      pConfig->bInterrupt = zTimerProfile.bInterrupt;
      /* pConfig->uRate = pPm->profiledb[uProfileId].profile. */
      pConfig->bReset = zTimerProfile.bReset;
      pConfig->bStrict = zTimerProfile.bStrict;
      pConfig->bStarted = zTimerProfile.bStarted;
      pConfig->uDeadline = zTimerProfile.uDeadline;
    }
    else
    {
      return SB_BAD_ARGUMENT_ERR_CODE;
    }
  }

  /* Lookup the Profile Details and Populate the Policer config */

  return SB_OK;
}

/*
 * This function gets the Timer Group Information.
 * 
 * @param pPm             Pointer to the sbFe2000Pm_t struct
 * @param uGroupId        Policer Group, base rate of which needs to be modified.
 * @param *pConfig        Pointer to the Timer Group Configuration.
 */
sbStatus_t
sbFe2000PmOamTimerGroupRead(sbFe2000Pm_t *pPm,
                       uint32_t uGroupId,
                       sbFe2000PmGroupConfig_t *pConfig)
{
  sbStatus_t  uStatus;

  uStatus = sbFe2000PmPolicerGroupRead( pPm, uGroupId, pConfig );

  if( uStatus != SB_OK )
  {
    return uStatus;
  }

  return SB_OK;
}

/**
 * This function deletes the previously configured timer
 * 
 * @param pPm             Pointer to the sbFe2000Pm_t struct
 * @param uTimerIndex     Pointer to the Timer index value
 * @return                Return status of the type sbStatus_t
 */
sbStatus_t 
sbFe2000PmOamTimerDelete(sbFe2000Pm_t *pPm,
                       uint32_t uTimerId)
{

  sbStatus_t uStatus;
  
  uStatus = sbFe2000PmPolicerDelete(pPm, uTimerId);

  if( uStatus != SB_OK )
  {
    return uStatus;
  }
  return SB_OK;
}

sbStatus_t
sbFe2000PmOamSeqGenBestFitProfileGet( sbZfFe2000PmProfileSeqGenMemory_t *pProfile)
{
  sbZfFe2000PmProfileSeqGenMemory_InitInstance(pProfile);
  
  pProfile->uType = SB_FE_2000_PM_PROF_TYPE_OAM;
  pProfile->uProfMode = SB_FE_2000_PM_OAMPROF_TYPE_SEQGEN;

  return SB_OK;
}


sbStatus_t
sbFe2000XtPmOamSeqGenBestFitProfileGet( sbZfC2PmProfileSeqGenMemory_t *pProfile)
{
  sbZfC2PmProfileSeqGenMemory_InitInstance(pProfile);
  
  pProfile->uType = SB_FE_2000_PM_PROF_TYPE_OAM;
  pProfile->uProfMode = SB_FE_2000_PM_OAMPROF_TYPE_SEQGEN;

  return SB_OK;
}



/*
 * This function configures a policer and returns
 * the index of the created  policer id.
 * 
 * @param pPm             Pointer to the sbFe2000Pm_t struct
 * @param uSeqGenId       Sequence Generator Index
 * @param *pConfig        Pointer to the Sequence Generator Configuration.
 * @param *pIndex         pointer to the policer index value
 * @return                Return status of the type sbStatus_t
 */
sbStatus_t
sbFe2000PmOamSeqGenCreate( sbFe2000Pm_t     *pPm,
                         uint32_t      ulSeqGenId,
                         uint32_t     *pIndex)
{
  sbStatus_t  uStatus;
  uint32_t    grp=0xFF;
  uint32_t    uMinPolicerId = 0,uMaxPolicerId = 0;
  uint32_t    uAddress = 0, uValue =0;
  uint32_t    uSeqGenId=0, uProfileId= 0xFFFF;
  uint8_t     bFound = 0;
  uint8_t     uSeqGenBytes[SB_ZF_C2_PM_PROFSEQGENMEMORY_SIZE_IN_BYTES];

  sbZfFe2000PmProfileMemory_t         zCommonProfile;
  sbZfC2PmProfileMemory_t         zXtCommonProfile;
  sbZfFe2000PmOamSeqGenRawFormat_t    zSeqGen;
  sbFe2000PmPolicer_t                 *pPolicerRef = NULL;

  assert(SB_ZF_C2_PM_PROFSEQGENMEMORY_SIZE_IN_BYTES > SB_ZF_FE_2000_PM_PROFSEQGENMEMORY_SIZE_IN_BYTES);

  sbZfFe2000PmOamSeqGenRawFormat_InitInstance(&zSeqGen);
  sbZfFe2000PmProfileMemory_InitInstance(&zCommonProfile);
  sbZfC2PmProfileMemory_InitInstance(&zXtCommonProfile);
  SB_MEMSET(&uSeqGenBytes[0], 0, SB_ZF_C2_PM_PROFSEQGENMEMORY_SIZE_IN_BYTES);
  
  /* 
   * step 1: get the oam sequence generator group.
   */
  grp = 8;

  /* 
   * step 2: see if this group has any policer left 
   * that can be assigned to this new policer
   */
  /* read this groups min and max policer id configured */
  uAddress = SAND_HAL_FE2000_REG_OFFSET(pPm->PolicerConfig.regSet, PM_GROUP0_CONFIG0) + 16*grp;
  uValue = SAND_HAL_FE2000_READ_OFFS(pPm->PolicerConfig.regSet, uAddress);
  uMinPolicerId = SAND_HAL_FE2000_GET_FIELD(pPm->PolicerConfig.regSet,PM_GROUP0_CONFIG0, POLID_MIN,uValue);
  uValue = SAND_HAL_FE2000_READ_OFFS(pPm->PolicerConfig.regSet, (uAddress+4));
  uMaxPolicerId = SAND_HAL_FE2000_GET_FIELD(pPm->PolicerConfig.regSet,PM_GROUP0_CONFIG1, POLID_MAX,uValue);

  /* Min & Max Policer Id stores the Policer Memory Bases */
  uMinPolicerId -= pPm->PolicerConfig.base;
  uMaxPolicerId -= pPm->PolicerConfig.base;

  if( (ulSeqGenId < uMinPolicerId)  || (ulSeqGenId > uMaxPolicerId) )
  {
    /* SeqGenId == 0 special case where Policer Id needs to be allocated */
    if( ulSeqGenId != 0 )
    {
      return SB_BAD_ARGUMENT_ERR_CODE;
    }
  }

  if( ulSeqGenId != 0 )
  {
    uSeqGenId = ulSeqGenId;
    bFound = 1;
  }
  else
  {
    /* caller must supply ID now that g2p3 is used. */
    return SB_BAD_ARGUMENT_ERR_CODE;
  }

  if(!bFound || (uSeqGenId>uMaxPolicerId) ) {
    return SB_PM_TOO_MANY_TIMERS;
  }

  /* 
   * step 3: find the profile that's a fit for this policer
   * configuration.
   */


  if (SAND_HAL_IS_FE2KXT (pPm->PolicerConfig.regSet ))
  {
    sbZfC2PmProfileSeqGenMemory_t   zProfile;
    sbZfC2PmProfileSeqGenMemory_InitInstance(&zProfile);

    sbFe2000XtPmOamSeqGenBestFitProfileGet(&zProfile);
    
    sbZfC2PmProfileSeqGenMemory_Pack( &zProfile, &uSeqGenBytes[0], SB_ZF_C2_PM_PROFSEQGENMEMORY_SIZE_IN_BYTES );
    sbZfC2PmProfileMemory_Unpack( &zXtCommonProfile, &uSeqGenBytes[0], sizeof(zXtCommonProfile) ) ;
    uStatus = sbFe2000XtPmProfileAlloc(pPm, &zXtCommonProfile, &uProfileId);
  }
  else{
    sbZfFe2000PmProfileSeqGenMemory_t   zProfile;
    sbZfFe2000PmProfileSeqGenMemory_InitInstance(&zProfile);

    sbFe2000PmOamSeqGenBestFitProfileGet(&zProfile);

    sbZfFe2000PmProfileSeqGenMemory_Pack( &zProfile, &uSeqGenBytes[0], SB_ZF_FE_2000_PM_PROFSEQGENMEMORY_SIZE_IN_BYTES );
    sbZfFe2000PmProfileMemory_Unpack( &zCommonProfile, &uSeqGenBytes[0], sizeof(zCommonProfile) ) ;
    uStatus = sbFe2000PmProfileAlloc(pPm, &zCommonProfile, &uProfileId);
  }

  if(uStatus != SB_OK) {
    return uStatus;
  }

  /* Add the policer configuration to profile ref list */
  AllocPolicer(pPm, &pPolicerRef);
  pPolicerRef->uPolicerId = uSeqGenId;
  pPolicerRef->uProfileId = uProfileId;
  pPolicerRef->uGroupId   = grp;
  DQ_INSERT_TAIL((dq_p_t) &pPm->profiledb[uProfileId].listPolicers, &pPolicerRef->listElem);

  /* suppress writes to this policer id before changing it's config */
  sbFe2000PmPolicerRefreshSet(pPm, uSeqGenId, SB_FE_2000_PM_DISABLE_REFRESH);

  zSeqGen.uProfileId  = uProfileId;
  /* Sequence Num can be initialized as per requirement */
  zSeqGen.uSequenceNum = 0;
  uStatus = sbFe2000PmOamSeqGenRawSet(pPm, uSeqGenId, &zSeqGen);

  /* let the refreshes go on */
  sbFe2000PmPolicerRefreshSet(pPm, uSeqGenId, SB_FE_2000_PM_ENABLE_REFRESH);

  /* return the configured policer id to the user */
  *pIndex = uSeqGenId;
  return uStatus;
}

/*
 * This function gets the SeqGen Group Information.
 * 
 * @param pPm             Pointer to the sbFe2000Pm_t struct
 * @param uGroupId        Policer Group, base rate of which needs to be modified.
 * @param *pConfig        Pointer to the Timer Group Configuration.
 */
sbStatus_t
sbFe2000PmOamSeqGenGroupRead(sbFe2000Pm_t *pPm,
                        uint32_t uGroupId,
                        sbFe2000PmGroupConfig_t *pConfig)
{
  sbStatus_t  uStatus;

  uStatus = sbFe2000PmPolicerGroupRead( pPm, uGroupId, pConfig );

  if( uStatus != SB_OK )
  {
    return uStatus;
  }
  return SB_OK;
}

/**
 * This function deletes the previously configured SequenceGenerator
 * 
 * @param pPm             Pointer to the sbFe2000Pm_t struct
 * @param uSeqGenIndex    Pointer to the sequence generator index value
 * @return                Return status of the type sbStatus_t
 */
sbStatus_t
sbFe2000PmOamSeqGenDelete(sbFe2000Pm_t *pPm,
                        uint32_t uSeqGenId)
{
  sbStatus_t uStatus;

  uStatus = sbFe2000PmPolicerDelete(pPm, uSeqGenId);

  if( uStatus != SB_OK )
  {
    return uStatus;
  }
  return SB_OK;
}


PRIVATE sbStatus_t
sbFe2000PmPolicerGroupReadConfig(sbFe2000Pm_t *pPm,  uint32_t uGroupId, 
                           sbZfFe2000PmGroupConfig_t *pGroupConfig)
{
  sbStatus_t status;
  uint32_t uTimerTick, uMtuTimeNs, uBits = 0;
  uint32_t uNpuClockPeriod = 0; /* In nanosec */

  if (SAND_HAL_IS_FE2KXT (pPm->PolicerConfig.regSet ))
  {
    /* uNpuClockPeriod = SB_FE_2000XT_PM_NPU_CLOCK_PERIOD_NS; */
    if        (pPm->uHppFreq == 400000) {
        uNpuClockPeriod = 1000.0/400.0;
    } else if (pPm->uHppFreq == 375000) {
        uNpuClockPeriod = 1000.0/375.0;
    } else if (pPm->uHppFreq == 300000) {
        uNpuClockPeriod = 1000.0/300.0;
    } else if (pPm->uHppFreq == 262500) {
        uNpuClockPeriod = 1000.0/262.500;
    } else if (pPm->uHppFreq == 258333) {
        uNpuClockPeriod = 1000.0/258.333;
    } else if (pPm->uHppFreq == 250000) {
        uNpuClockPeriod = 1000.0/250.0;
    } else {
        SB_ASSERT(0);
    }
  }
  else
  {
    uNpuClockPeriod = SB_FE_2000_PM_NPU_CLOCK_PERIOD_NS;
  }


  /* hardcode refresh count to 1 for now.  Since we only have one group of
   * policers, we can optimize for the one group
   */
  pGroupConfig->uRefreshCount  = 1;

  /* pick a number; this defines the refresh queue depth before an error is
   * generated when there's a misconfiguration with respect to the available 
   * bandwith
   */
  pGroupConfig->uRefreshThresh = SB_FE_2000_PM_REFRESH_THRESH;
  
  /* 
   * note that this timer enable is not sufficient. we have to enable the global timer 
   * also.
   */
  pGroupConfig->uTimerEnable   = 1;

  /* 
   * The number of bits required must be equal to timestamp 
   * required for a "ideal" packet arrival of K BYTES at every sec to result
   * R bps . N = LOG ( zGroupConfig.uTimerTickPeriod) /LOG 2 
   * N = N*2; This is all "ideal" case calculations. so needs to be 
   * fixed after the trials on the real hardware.
   */
  /* for the given rate, convert rate to timerTick in cycles/byte */
  status = sbFe2000PolicerTimerTickGet(pPm, pPm->PolicerConfig.rates[uGroupId], 
                                     &uTimerTick);
  if(status != SB_OK) {
    return status;
  }
  pGroupConfig->uTimerTickPeriod = uTimerTick;

#ifndef __KERNEL__  
  uBits = log10(pGroupConfig->uTimerTickPeriod)/log10(2);
#endif
  uBits = uBits+1; /* add one more almost double the time period*/
  pGroupConfig->uTimestampOffset = 52-uBits; 

  /* refresh period is used to define the timer rate and is defined by:
   *   group_size / refresh_count * refresh_period * 4 ns =  ns/tick
   * Ideally, the timer tick should result in a small enough granularity
   * to detect an MTU at the given rate
   */ 
  uMtuTimeNs = (uTimerTick * SB_FE_2000_PM_MTU * 
                uNpuClockPeriod);

  /* special case the timers group; we want a 10ms timer per tick.
   * for the OAM timers group, the timer_tick and timer tick period and 
   * timestamp offeset are don't cares.  All that matters is the 
   * timer refresh rate, which defines the units of a tick.  For OAM, we want
   * a 10ms unit of timer per tick; that is the contract with the SDK.
   */
  if (pPm->PolicerConfig.uNumberOfPolicersPerGroup[uGroupId] == 0) {
    pGroupConfig->uRefreshPeriod = 0;
  }else if (uGroupId == SB_FE_2000_PM_OAM_TIMERS_GROUP) {
      
    pGroupConfig->uRefreshPeriod = SB_FE_2000_PM_OAM_TIMER_TICK_NS / 
      (uNpuClockPeriod * 
       pPm->PolicerConfig.uNumberOfPolicersPerGroup[uGroupId]);

  } else {
    pGroupConfig->uRefreshPeriod = uMtuTimeNs / 
      (pPm->PolicerConfig.uNumberOfPolicersPerGroup[uGroupId] * 
       uNpuClockPeriod);
  }

  SB_LOGV3("group %d: %d policers, Timer: %f ms per tick\n", uGroupId,
           pPm->PolicerConfig.uNumberOfPolicersPerGroup[uGroupId],
           (pPm->PolicerConfig.uNumberOfPolicersPerGroup[uGroupId] * 
            pGroupConfig->uRefreshPeriod *
            uNpuClockPeriod / 1e6));

  return status;
}

/* Assumes synchronous DMA completion */
PRIVATE sbStatus_t
sbFe2000PmPolicerRawSet(sbFe2000Pm_t *pPm,
                      uint32_t ulIndex,
                      sbZfFe2000PmPolicerRawFormat_t *pPrm)
{
  sbStatus_t uStatus;

  if ((ulIndex >= SB_FE_2000_MAX_POLICER_IDX ))
    return SB_BAD_ARGUMENT_ERR_CODE;

  sbZfFe2000PmPolicerRawFormat_Pack(pPrm,
                                    (uint8_t *) pPm->dmaOp.hostAddress, 0);
  
  pPm->dmaOp.feAddress =
    SB_FE2000_DMA_MAKE_ADDRESS(pPm->PolicerConfig.bank,
                               pPm->PolicerConfig.base + ulIndex);
  pPm->dmaOp.opcode = SB_FE2000_DMA_OPCODE_WRITE;
  
  uStatus = sbFe2000DmaRequest(pPm->PolicerConfig.dmaMgr,
                               pPm->PolicerConfig.token,
                               &pPm->dmaOp);

  return uStatus;
}

/* Assumes synchronous DMA completion */
PRIVATE sbStatus_t
sbFe2000PmPolicerRawGet(sbFe2000Pm_t *pPm,
                        uint32_t ulIndex,
                        sbZfFe2000PmPolicerRawFormat_t *pPrm)
{
  sbStatus_t uStatus;

  if ((ulIndex >= SB_FE_2000_MAX_POLICER_IDX ))
    return SB_BAD_ARGUMENT_ERR_CODE;

  pPm->dmaOp.feAddress =
    SB_FE2000_DMA_MAKE_ADDRESS(pPm->PolicerConfig.bank,
                               pPm->PolicerConfig.base + ulIndex);
  pPm->dmaOp.opcode = SB_FE2000_DMA_OPCODE_READ;
  
  uStatus = sbFe2000DmaRequest(pPm->PolicerConfig.dmaMgr,
                               pPm->PolicerConfig.token,
                               &pPm->dmaOp);

  sbZfFe2000PmPolicerRawFormat_Unpack(pPrm,
                                      (uint8_t *) pPm->dmaOp.hostAddress, 0);
  
  return uStatus;
}

/* Assumes synchronous DMA completion */
sbStatus_t
sbFe2000PmOamTimerRawSet(sbFe2000Pm_t *pPm,
                      uint32_t ulIndex,
                      sbZfFe2000PmOamTimerRawFormat_t *pPrm)
{
  sbStatus_t uStatus;

  if ((ulIndex >= SB_FE_2000_MAX_POLICER_IDX ))
    return SB_BAD_ARGUMENT_ERR_CODE;

  sbZfFe2000PmOamTimerRawFormat_Pack(pPrm,
                                     (uint8_t *) pPm->dmaOp.hostAddress, 0);
  
  pPm->dmaOp.feAddress =
    SB_FE2000_DMA_MAKE_ADDRESS(pPm->PolicerConfig.bank,
                               pPm->PolicerConfig.base + ulIndex);
  pPm->dmaOp.opcode = SB_FE2000_DMA_OPCODE_WRITE;
  
  uStatus = sbFe2000DmaRequest(pPm->PolicerConfig.dmaMgr,
                               pPm->PolicerConfig.token,
                               &pPm->dmaOp);

  return uStatus;
}

/* Assumes synchronous DMA completion */
sbStatus_t
sbFe2000PmOamTimerRawGet(sbFe2000Pm_t *pPm,
                        uint32_t ulIndex,
                        sbZfFe2000PmOamTimerRawFormat_t *pPrm)
{
  sbStatus_t uStatus;

  if ((ulIndex >= SB_FE_2000_MAX_POLICER_IDX ))
    return SB_BAD_ARGUMENT_ERR_CODE;

  pPm->dmaOp.feAddress =
    SB_FE2000_DMA_MAKE_ADDRESS(pPm->PolicerConfig.bank,
                               pPm->PolicerConfig.base + ulIndex);
  pPm->dmaOp.opcode = SB_FE2000_DMA_OPCODE_READ;
  
  uStatus = sbFe2000DmaRequest(pPm->PolicerConfig.dmaMgr,
                               pPm->PolicerConfig.token,
                               &pPm->dmaOp);

  sbZfFe2000PmOamTimerRawFormat_Unpack(pPrm,
                                       (uint8_t *) pPm->dmaOp.hostAddress, 0);
  
  return uStatus;
}

/* Assumes synchronous DMA completion */
PRIVATE sbStatus_t
sbFe2000PmOamSeqGenRawSet(sbFe2000Pm_t *pPm,
                      uint32_t ulIndex,
                      sbZfFe2000PmOamSeqGenRawFormat_t *pPrm)
{
  sbStatus_t uStatus;

  if ((ulIndex >= SB_FE_2000_MAX_POLICER_IDX ))
    return SB_BAD_ARGUMENT_ERR_CODE;

  sbZfFe2000PmOamSeqGenRawFormat_Pack(pPrm,
                                     (uint8_t *) pPm->dmaOp.hostAddress, 0);
  
  pPm->dmaOp.feAddress =
    SB_FE2000_DMA_MAKE_ADDRESS(pPm->PolicerConfig.bank,
                               pPm->PolicerConfig.base + ulIndex);
  pPm->dmaOp.opcode = SB_FE2000_DMA_OPCODE_WRITE;
  
  uStatus = sbFe2000DmaRequest(pPm->PolicerConfig.dmaMgr,
                               pPm->PolicerConfig.token,
                               &pPm->dmaOp);

  return uStatus;
}


sbStatus_t
sbFe2000XtPmOamTimerBestFitProfileGet(sbZfFe2000PmOamTimerConfig_t *pConfig, 
                                      sbZfC2PmProfileTimerMemory_t *pProfile)
{
  sbZfC2PmProfileTimerMemory_InitInstance(pProfile);
  
  pProfile->uType = SB_FE_2000_PM_PROF_TYPE_OAM;
  pProfile->uProfMode = SB_FE_2000_PM_OAMPROF_TYPE_TIMER;


  pProfile->uDeadline = pConfig->uDeadline;

  pProfile->bInterrupt = pConfig->bInterrupt;

  pProfile->bReset = pConfig->bReset;

  pProfile->bStrict = pConfig->bStrict;

  pProfile->bStarted = pConfig->bStarted;

  return SB_OK;
}

sbStatus_t
sbFe2000XtPmPolicerBestFitProfileGet(sbZfFe2000PmPolicerConfig_t *pConfig, sbZfC2PmProfilePolicerMemory_t *pProfile)
{
  int32_t uTemp;
  sbStatus_t rv;

  sbZfC2PmProfilePolicerMemory_InitInstance(pProfile);
  
  pProfile->uType = SB_FE_2000_PM_METERTYPE_POLICER;
  /* 
   * we can enhance this further later to respect these
   * rfc modes.
   */
  pProfile->uEIR = pConfig->uEIR;
  pProfile->uCIR = pConfig->uCIR;

  /* configure the blind mode */
  pProfile->bBlind = pConfig->bBlindMode;

  /* configure the drop on red policy */
  pProfile->bDropOnRed = pConfig->bDropOnRed;

  /* configure the coupling between bktc & bkte carry over */
  pProfile->bCouplingFlag = pConfig->bCoupling;

  /* copy no decrement policies */
  pProfile->bBktCNoDecrement = pConfig->bCBSNoDecrement;
  pProfile->bBktENoDecrement = pConfig->bEBSNoDecrement;

  /* encode CBS & EBS to mantissa and exponent */

  /* 
   * force some parameters as per the rfc mode 
   * Todo: clean the user policer configuration.
   */
  if(pConfig->uRfcMode == SB_FE_2000_PM_PMODE_RFC_2697) {
    pProfile->uEIR = 0;
    pProfile->bCouplingFlag = pConfig->bCoupling;
  } else if(pConfig->uRfcMode == SB_FE_2000_PM_PMODE_RFC_2698) {
    pProfile->bRFC2698Mode = 1;
  } else if(pConfig->uRfcMode == SB_FE_2000_PM_PMODE_RFC_4115) {
  } else if(pConfig->uRfcMode == SB_FE_2000_PM_PMODE_RFC_MEF) {
    pProfile->bCouplingFlag = pConfig->bCoupling;
  } else {
    /* error */
  }

  rv = sbFe2000PmIEEECodingGet(pConfig->uCBS, &pProfile->uCBS);
  if (rv != SB_OK) {
    return rv;
  }
  rv = sbFe2000PmIEEECodingGet(pConfig->uEBS, &pProfile->uEBS);
  if (rv != SB_OK) {
    return rv;
  }

  /* find the number bits required to represent the bktc */
  pProfile->uBktCSize = 0;
  uTemp = pConfig->uCBS;
  while(uTemp) {
    pProfile->uBktCSize++;
    uTemp = uTemp >> 1;
  }
 
  /* for now length shift is always zero 
   * don't trade off accuracy yet.
   */
  /* pProfile->uLengthShift = 0; */
  pProfile->uLengthShift = pConfig->uLenShift;
  return SB_OK;
}


sbStatus_t 
sbFe2000XtPmProfileAlloc(sbFe2000Pm_t *pPm, sbZfC2PmProfileMemory_t *pProfile, uint32_t *pIndex)
{
  sbZfC2PmProfilePolicerMemory_t      zPolicerProfile, zPolicerProfileData;
  sbZfC2PmProfileTimerMemory_t        zTimerProfile, zTimerProfileData;
  sbStatus_t                          uStatus = SB_OK;

  uint8_t                             uProfileBytes[SB_ZF_C2_PM_PROFMEMORY_SIZE_IN_BYTES];
  uint32_t                            uId =0, bFound =0;
  

  /* see if there is a matching profile already configured. */
  bFound = 0;
  for(uId = (SB_FE_2000_PM_NULL_PROFILE_ID+1); uId < SB_FE_2000_PM_MAX_NUM_PROFILES;uId++) {
    if(pPm->profiledb[uId].uRefCount ) 
    {
      if( pProfile->uProfType == SB_FE_2000_PM_PROF_TYPE_POLICER )
      {
        sbZfC2PmProfilePolicerMemory_InitInstance( &zPolicerProfile );
        
        sbZfC2PmProfileMemory_Pack( pProfile, &uProfileBytes[0], SB_ZF_C2_PM_PROFMEMORY_SIZE_IN_BYTES );
        sbZfC2PmProfilePolicerMemory_Unpack( &zPolicerProfile, &uProfileBytes[0], sizeof(zPolicerProfile) ) ;

        sbZfC2PmProfilePolicerMemory_InitInstance( &zPolicerProfileData );
        SB_MEMSET( &uProfileBytes[0], 0, sizeof(uProfileBytes) );

        sbZfC2PmProfileMemory_Pack( (&pPm->profiledb[uId].C2Profile), &uProfileBytes[0], 
                                      SB_ZF_C2_PM_PROFMEMORY_SIZE_IN_BYTES );
        sbZfC2PmProfilePolicerMemory_Unpack( &zPolicerProfileData,  &uProfileBytes[0], sizeof(zPolicerProfileData));

        if( sbZfC2PmProfilePolicerMemory_Compare(&zPolicerProfile, &zPolicerProfileData) )
        {
          bFound = 1; *pIndex = uId;
          break;
        }
      } else if( pProfile->uProfType == SB_FE_2000_PM_PROF_TYPE_OAM ) {
        if( pProfile->uOamProfType == SB_FE_2000_PM_OAMPROF_TYPE_TIMER ) {
          sbZfC2PmProfileTimerMemory_InitInstance( &zTimerProfile );
        
          sbZfC2PmProfileMemory_Pack( pProfile, &uProfileBytes[0], SB_ZF_C2_PM_PROFMEMORY_SIZE_IN_BYTES );
          sbZfC2PmProfileTimerMemory_Unpack( &zTimerProfile, &uProfileBytes[0], sizeof(zTimerProfile) ) ;

          sbZfC2PmProfileTimerMemory_InitInstance( &zTimerProfileData );
          SB_MEMSET( &uProfileBytes[0], 0, sizeof(uProfileBytes) );

          sbZfC2PmProfileMemory_Pack( (&pPm->profiledb[uId].C2Profile), &uProfileBytes[0], 
                                        SB_ZF_C2_PM_PROFMEMORY_SIZE_IN_BYTES );
          sbZfC2PmProfileTimerMemory_Unpack( &zTimerProfileData,  &uProfileBytes[0], sizeof(zTimerProfileData));

          if( sbZfC2PmProfileTimerMemory_Compare(&zTimerProfile, &zTimerProfileData) )
          {
            bFound = 1; *pIndex = uId;
            break;
          }
        } else if( pProfile->uOamProfType == SB_FE_2000_PM_OAMPROF_TYPE_SEQGEN ) {
          if( sbZfFe2000PmProfileSeqGenMemory_Compare((sbZfFe2000PmProfileSeqGenMemory_t *)pProfile, 
                                                    (sbZfFe2000PmProfileSeqGenMemory_t *)&pPm->profiledb[uId].C2Profile) )
          {
            /* Compare always returns 1 */
            bFound = 1; *pIndex = uId;
            break;
          }
        }
      }
    }
  }

  if(!bFound) {
    for(uId = (SB_FE_2000_PM_NULL_PROFILE_ID+1);
        uId < SB_FE_2000_PM_MAX_NUM_PROFILES;
        uId++) {
      if(pPm->profiledb[uId].uRefCount == 0) {
        bFound =1;
        *pIndex = uId;
        break;
      }
    }

    if(!bFound) {
      return SB_PM_TOO_MANY_POLICERS;
    }

    /* configure the new profile to hardware */
    uStatus = sbFe2000XtProfileCreate(pPm, *pIndex, pProfile);
  }

  pPm->profiledb[*pIndex].uRefCount++;
  return uStatus;
}

sbStatus_t
sbFe2000XtProfileCreate(sbFe2000Pm_t *pPm, uint32_t uProfileId, sbZfC2PmProfileMemory_t *pProfile)
{
  sbStatus_t uStatus;

  /* increment the reference count for this profile */

  sbZfC2PmProfileMemory_Copy(pProfile, 
                               &pPm->profiledb[uProfileId].C2Profile);

  /* commit the null profile into hardware */
  uStatus = sbFe2000XtPmLocProfileMemoryWrite(pPm,uProfileId, pProfile);
  return uStatus;
}

sbStatus_t
sbFe2000XtPmLocProfileMemoryWrite(sbFe2000Pm_t *pPm, uint32_t uProfileId, sbZfC2PmProfileMemory_t *pProfile)
{
  sbStatus_t uStatus;
  uint32_t uProfileWords[SB_ZF_C2_PM_PROFMEMORY_SIZE_IN_WORDS];

  /* make wods from the zframe */
  sbZfC2PmProfileMemory_Pack32(pProfile, &uProfileWords[0], SB_ZF_C2_PM_PROFMEMORY_SIZE_IN_WORDS);

  /* write to the profile memory */
  uStatus = sbFe2000PmProfileMemoryWrite(pPm->PolicerConfig.regSet,uProfileId,&uProfileWords[0]);
  return uStatus;
}


sbStatus_t
sbFe2000XtPmLocProfileMemoryRead(sbFe2000Pm_t *pPm, uint32_t uProfileId, sbZfC2PmProfileMemory_t *pProfile)
{
  sbStatus_t uStatus;
  uint32_t uProfileWords[SB_ZF_C2_PM_PROFMEMORY_SIZE_IN_WORDS];

  /* write to the profile memory */
  uStatus = sbFe2000PmProfileMemoryRead(pPm->PolicerConfig.regSet,uProfileId,&uProfileWords[0]);
  if( uStatus != SB_OK )
  {
    return uStatus;
  }

  /* make zframe from the words */
  sbZfC2PmProfileMemory_Unpack32( pProfile, &uProfileWords[0], SB_ZF_C2_PM_PROFMEMORY_SIZE_IN_WORDS);

  return SB_OK;
}
