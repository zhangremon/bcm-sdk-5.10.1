/* -*- Mode:c++; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
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
 * $Id: sbFe2000CmuMgr.c 1.15.52.1 Broadcom SDK $
 *
 * sbFe2000CmuMgr.c : FE2000 Counter Management Unit
 *
 *-----------------------------------------------------------------------------*/

#include "glue.h"
#include "soc/drv.h"
#include "hal_user.h"
#include "hal_ca_auto.h"
#include "hal_c2_auto.h"
#include "hal_ca_c2.h"
#include <soc/sbx/fe2k_common/sbFe2000CmuMgr.h>
#include <soc/sbx/fe2k_common/sbFe2000Common.h>
#include "sbWrappers.h"
#include "sbFe2000Init.h"
#include "sbx_drv.h"

#define CMU_TIMEOUT 50000

/*-----------------------------------------------------------------------------
 * Module Local Types / Prototypes
 *-----------------------------------------------------------------------------*/

/* For use with TREX-generated mask and shift fields. Right align these fields
 * so that we can use them to perform range checks. The masks are aligned with
 * the fields so they cannot be directly compared with register values. This
 * macro cleans up the code so we don't have to replicate mask and shift */
#define MALIGN(addr, x) \
  ( SAND_HAL_IS_FE2KXT(addr) \
    ? (SAND_HAL_C2_##x##_MASK >> SAND_HAL_C2_##x##_SHIFT) \
    : (SAND_HAL_CA_##x##_MASK >> SAND_HAL_CA_##x##_SHIFT) )

/*-----------------------------------------------------------------------------
 * CMU control structure. This describes all the information needed to manage
 * the counter ejection process. The declaration is intentionally HIDDEN here
 * at file-scope.
 *-----------------------------------------------------------------------------*/
struct sbFe2000CmuMgr_s {
  /* pointer to chip */
  void *dev;
  /* pointer to per-segment host-side counters */
  uint64_t *ullCount[SB_FE2000_SEG_TBL_SEG_MAX];
  void* pSegMem[SB_FE2000_SEG_TBL_SEG_MAX];
  /* hold onto the bank and base */
  uint32_t  ulBank[SB_FE2000_SEG_TBL_SEG_MAX];
  uint32_t  ulBase[SB_FE2000_SEG_TBL_SEG_MAX];
  uint32_t ulSize[SB_FE2000_SEG_TBL_SEG_MAX];
  /* pointer to per-segment host-side counters */
  sbFe2000SegTblType_e_t eType[SB_FE2000_SEG_TBL_SEG_MAX];
  /* segment in use count */
  uint32_t ulUsedSegments;
  /* dma-accessible pci ring */
  uint8_t   bFlushing;
  uint32_t *pRing;
  uint32_t pRingHba;
  uint32_t  ulRingSize;
  uint32_t  ulRingThresh;
  uint32_t *pRingEntry;
  sbDmaMemoryHandle_t hRingHandle;
  sal_mutex_t cmu_flush_lock;
  uint32_t  cmu_watermark;

  sbFe2000DmaMgr_t *pDmaState; /* temporary, while hacks are in */
};

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuSwInit()
 *
 * @brief Perform the counter manager software initialization. This will prepare
 *        the driver that will handle counter updates that are ejected from the
 *        CMU. This initialization does not touch the state of the FE-2000. All
 *        initializations that affec the hardware are performed later by the
 *        hardware initialization.
 *
 * @param pCtl - Referenced pointer to CMU control structure
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuSwInit(sbFe2000CmuMgr_t **ppCtl,
                  uint32_t ulRingSize,
                  uint32_t ulRingThresh,
                  void *hChipHandle,
                  sbFe2000DmaGetHostBusAddress_f_t v2p,
                  sbFe2000DmaMgr_t *pDmaState) /* xxx remove me (temporary) */
{
  sbFe2000CmuMgr_t *pCtl;
  sbStatus_t sts;
  void *vp;

  /* ring size must be a power of two */
  if (!ulRingSize || (ulRingSize & (ulRingSize - 1)))
    return SB_BAD_ARGUMENT_ERR_CODE;

  /* allocate control structure, validate it, zero it */
  vp = thin_host_malloc(sizeof(sbFe2000CmuMgr_t));
  if (!vp)
    return SB_MALLOC_FAILED;
  SB_MEMSET(vp, 0x0, sizeof(sbFe2000CmuMgr_t));
  pCtl = vp;

  pCtl->dev = hChipHandle;
  pCtl->ulRingSize = ulRingSize;
  pCtl->ulRingThresh = ulRingThresh;
  pCtl->ulUsedSegments = 0;
  pCtl->pDmaState = pDmaState;

  /* allocate dma memory for ring handle to live */
  sts = thin_contig_malloc(hChipHandle, SB_ALLOC_OTHER_DMA, 2 * ulRingSize,
                           &vp, &pCtl->hRingHandle);
  if (!vp) {
    thin_host_free(pCtl);
    return SB_MALLOC_FAILED;
  }

  vp =
    (((char *) vp) + (ulRingSize - (((unsigned int) vp) & (ulRingSize - 1))));

  sts = (v2p)(pCtl->hRingHandle,  vp, ulRingSize, &pCtl->pRingHba);

  SB_ASSERT(sts == SB_OK);

  /* clear out the ring */
  SB_MEMSET(vp, 0x0, ulRingSize);

  /* keep a pointer to the process ring */
  pCtl->pRing = vp;
  pCtl->pRingEntry = vp;

  /* create the lock to control the CMU flush */
  pCtl->cmu_flush_lock = sal_mutex_create("CMU flush lock");
  if (!pCtl->cmu_flush_lock) {
      thin_host_free(pCtl);
      return SB_MALLOC_FAILED; /* closest to memory error */
  }

  pCtl->cmu_watermark = CMU_TIMEOUT;

  /* pass back pointer */
  *ppCtl = pCtl;

  return SB_OK;
}

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuHwInit()
 *
 * @brief Perform the counter manager hardware initialization
 *
 * @param pCtl - Pointer to CMU control structure
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuHwInit(sbFe2000CmuMgr_t *pCtl, uint32_t warmboot)
{
  sbFe2000MmuCmuConfig_e_t ulMm0Cmu0Cfg, ulMm0Cmu1Cfg, ulMm1Cmu0Cfg, ulMm1Cmu1Cfg;
  uint32_t ulRingSize = pCtl->ulRingSize - 1; /* power-of-2, expressed as mask */
  uint32_t ulRingThresh = pCtl->ulRingThresh, ulMMClientCfg, reg;

  if ((!pCtl) ||
      (ulRingSize > MALIGN(pCtl->dev, PC_COUNTER_RING_SIZE_COUNTER_RING_SIZE)) ||
      (ulRingThresh > MALIGN(pCtl->dev, PC_COUNTER_RING_CTRL_COUNTER_RING_THRESH)))
    return SB_BAD_ARGUMENT_ERR_CODE;

  if (!warmboot) {
    /* config thresholds */
    SAND_HAL_FE2000_WRITE(pCtl->dev,  CM_CONFIG_AGE_THRESHOLD, 50);
    SAND_HAL_FE2000_WRITE(pCtl->dev,  CM_CONFIG_DMA_FORCE_COMMIT, 0);
  }
  /* zero ring out */
  SB_MEMSET(pCtl->pRing, 0, ulRingSize + 1);

  /* config ring pointer and ring size */
  SAND_HAL_FE2000_WRITE(pCtl->dev,  PC_COUNTER_RING_PTR, pCtl->pRingHba);
  if (!warmboot) {
    SAND_HAL_FE2000_WRITE(pCtl->dev,  PC_COUNTER_RING_SIZE, ulRingSize);
    SAND_HAL_FE2000_WRITE(pCtl->dev,  PC_COUNTER_RING_CONSUMER, 0);
  }else{
    reg = SAND_HAL_FE2000_READ(pCtl->dev, PC_COUNTER_RING_PRODUCER);
    SAND_HAL_FE2000_WRITE(pCtl->dev, PC_COUNTER_RING_CONSUMER, reg);
    SAND_HAL_FE2000_WRITE(pCtl->dev,  PC_COUNTER_RING_SIZE, ulRingSize);
  }

  /* PC_COUNTER_RING_PRODUCER = RO */

  if (!warmboot) {
    /* define the threshold, do not enable it yet */
    SAND_HAL_FE2000_WRITE(pCtl->dev,  PC_COUNTER_RING_CTRL,
    SAND_HAL_FE2000_SET_FIELD(pCtl->dev, PC_COUNTER_RING_CTRL, COUNTER_FIFO_ENABLE, warmboot)|
    SAND_HAL_FE2000_SET_FIELD(pCtl->dev, PC_COUNTER_RING_CTRL, COUNTER_RING_THRESH, ulRingThresh));

    /* xxx - fixme don't hard-code this stuff */
    /* Setup MM0_CMU0 to MM0 EXTERNAL NARROW Memory 0 */
    /* Setup MM0_CMU1 to MM0 EXTERNAL NARROW Memory 1 */
    /* Setup MM1_CMU0 to MM1 EXTERNAL NARROW Memory 0 */
    /* Setup MM1_CMU1 to MM1 INTERNAL Memory 1 */
    ulMm0Cmu0Cfg = SB_FE2000_MM_CMU_CFG_INTERNAL;
    ulMm0Cmu1Cfg = SB_FE2000_MM_CMU_CFG_EXTERNAL;
    ulMm1Cmu0Cfg = SB_FE2000_MM_CMU_CFG_EXTERNAL;
    ulMm1Cmu1Cfg = SB_FE2000_MM_CMU_CFG_EXTERNAL;

    /* Assign Counter Manager configuration MM0 */
    ulMMClientCfg = SAND_HAL_FE2000_READ(pCtl->dev,  MM0_CLIENT_CONFIG);
    ulMMClientCfg = SAND_HAL_FE2000_MOD_FIELD(pCtl->dev, MM0_CLIENT_CONFIG, CMU0_MEMORY_CONFIG, ulMMClientCfg, ulMm0Cmu0Cfg);
    ulMMClientCfg = SAND_HAL_FE2000_MOD_FIELD(pCtl->dev, MM0_CLIENT_CONFIG, CMU1_MEMORY_CONFIG, ulMMClientCfg, ulMm0Cmu1Cfg);
    SAND_HAL_FE2000_WRITE(pCtl->dev,  MM0_CLIENT_CONFIG, ulMMClientCfg);

    /* Assign Counter Manager configuration MM1 */
    ulMMClientCfg = SAND_HAL_FE2000_READ(pCtl->dev,  MM1_CLIENT_CONFIG);
    ulMMClientCfg = SAND_HAL_FE2000_MOD_FIELD(pCtl->dev, MM1_CLIENT_CONFIG, CMU0_MEMORY_CONFIG, ulMMClientCfg, ulMm1Cmu0Cfg);
    ulMMClientCfg = SAND_HAL_FE2000_MOD_FIELD(pCtl->dev, MM1_CLIENT_CONFIG, CMU1_MEMORY_CONFIG, ulMMClientCfg, ulMm1Cmu1Cfg);
    SAND_HAL_FE2000_WRITE(pCtl->dev,  MM1_CLIENT_CONFIG, ulMMClientCfg);

    /* xxx - need to setup ( MM_RAM_CONFIG) */

    /* Setup MM1 Int Memory 1 To be 35x1 protection */
    /* TODO: this needs to be changed if we don't use mm1_int1 for counters */
    /*       should do this in segment config on-demand (needed at all?)    */

    if (!SAND_HAL_IS_FE2KXT(pCtl->dev)) {
      reg = SAND_HAL_READ(pCtl->dev, CA, MM0_PROT_SCHEME1);
      reg = SAND_HAL_MOD_FIELD(CA, MM0_PROT_SCHEME1, INT0_SCHEME, reg,
                               SB_FE2000_MM_PROT_SCH_35D_1P);

      SAND_HAL_WRITE(pCtl->dev, CA, MM0_PROT_SCHEME1, reg);
    }

    /* set force flush rate to non-zero */
    SAND_HAL_FE2000_WRITE(pCtl->dev,  CM_CONFIG_FLUSH_ISSUE_RATE, 0x800);
  }

  return SB_OK;
}


/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuHwUninit()
 *
 * @brief Perform the counter manager hardware uninitialization
 *
 * @param pCtl - Pointer to CMU control structure
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuHwUninit(sbFe2000CmuMgr_t *pCtl)
{
  uint32_t reg;

  /* disable DMA transactions from the device */
  SAND_HAL_FE2000_WRITE(pCtl->dev,  PC_COUNTER_RING_SIZE, 0);
  SAND_HAL_FE2000_WRITE(pCtl->dev,  PC_COUNTER_RING_CONSUMER, 0);
  if (pCtl->ulUsedSegments) {
    sbFe2000CmuForceFlush(pCtl, 0, 0, 1);
  }

  /* and disable CM interrupts */
  reg = SAND_HAL_FE2000_READ(pCtl->dev, PC_UNIT_INTERRUPT_MASK);
  reg |= SAND_HAL_FE2000_FIELD_MASK(pCtl->dev, PC_UNIT_INTERRUPT_MASK, CM_DISINT);
  SAND_HAL_FE2000_WRITE(pCtl->dev, PC_UNIT_INTERRUPT_MASK, reg);
  
  SAND_HAL_FE2000_WRITE(pCtl->dev,  CM_SEGMENT_ENABLE, 0);

  return SB_OK;
}

sbStatus_t
sbFe2000CmuWbRecover(int unit)
{
  uint32_t ulReg;

  /* disable DMA transactions from the device */
  SAND_HAL_FE2000_WRITE(unit,  PC_COUNTER_RING_SIZE, 0);
  SAND_HAL_FE2000_WRITE(unit,  PC_COUNTER_RING_CONSUMER, 0);

  /* issue force flush */
  ulReg = SAND_HAL_FE2000_READ(unit,  CM_CONFIG_FORCED_FLUSH);
  if (!SAND_HAL_FE2000_GET_FIELD(unit, CM_CONFIG_FORCED_FLUSH, IN_PROGRESS, ulReg))
  {
    /* clear the flush event done. Only one pending force flush event allowed. 
       protected by cmu_flush_lock */
    ulReg = SAND_HAL_FE2000_SET_FIELD(unit, CM_FORCE, FLUSH_EVENT_DONE, 1);
    SAND_HAL_FE2000_WRITE(unit, CM_FORCE_FLUSH_EVENT, ulReg);

    /* number of entries to flush (minus 1) */
    SAND_HAL_FE2000_WRITE(unit,  CM_CONFIG_FORCE_FLUSH_SIZE, 0);

    /* compose and issue request */
    ulReg = (SAND_HAL_FE2000_SET_FIELD(unit, CM_CONFIG_FORCED_FLUSH, GO, 1) |
             SAND_HAL_FE2000_SET_FIELD(unit, CM_CONFIG_FORCED_FLUSH, SEGMENT_NUM, 0) |
             SAND_HAL_FE2000_SET_FIELD(unit, CM_CONFIG_FORCED_FLUSH, STARTING_OFFSET, 0));
    SAND_HAL_FE2000_WRITE(unit,  CM_CONFIG_FORCED_FLUSH, ulReg);

  }

  /* and disable CM interrupts */
  ulReg = SAND_HAL_FE2000_READ(unit, PC_UNIT_INTERRUPT_MASK);
  ulReg |= SAND_HAL_FE2000_FIELD_MASK(unit, PC_UNIT_INTERRUPT_MASK, CM_DISINT);
  SAND_HAL_FE2000_WRITE(unit, PC_UNIT_INTERRUPT_MASK, ulReg);
  
  SAND_HAL_FE2000_WRITE(unit,  CM_SEGMENT_ENABLE, 0);

  return SB_OK;
}

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuSwUninit()
 *
 * @brief Perform the counter manager software uninitialization
 *
 * @param pCtl - Referenced pointer to CMU control structure 
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuSwUninit(sbFe2000CmuMgr_t **ppCtl)
{
  sbFe2000CmuMgr_t *pCtl = *ppCtl;

  uint32_t i;
  /* walk used segments and deallocate any associated memory */
  for (i=0; i < pCtl->ulUsedSegments; i++) {
    if (pCtl->pSegMem[i]) {
      thin_host_free( pCtl->pSegMem[i]);
    }
  }

  /* free dma ring memory */
  if (pCtl->pRing) {
    thin_free(pCtl->dev, SB_ALLOC_OTHER_DMA, 2*pCtl->ulRingSize, pCtl->pRing, pCtl->hRingHandle);
    pCtl->pRing=0;
  }

  /* deallocate lock */
  if (pCtl->cmu_flush_lock) {
    sal_mutex_destroy(pCtl->cmu_flush_lock);
    pCtl->cmu_flush_lock = 0;
  }

  /* free CMU control structure */
  thin_host_free(pCtl);
  *ppCtl = 0;

  return SB_OK;
}
/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuFifoEnable()
 *
 * @brief Enable/Disable Counter Manager Unit fifo that feeds the processor
 *
 * @param pCtl - Pointer to CMU control structure
 * @param bEnable - Enable or Disable the Fifo
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuFifoEnable(sbFe2000CmuMgr_t *pCtl,
                      uint8_t bEnable)
{
  uint32_t reg, msk;

  reg = SAND_HAL_FE2000_READ(pCtl->dev,  PC_COUNTER_RING_CTRL);
  msk = SAND_HAL_FE2000_SET_FIELD(pCtl->dev, PC_COUNTER_RING_CTRL, COUNTER_FIFO_ENABLE, 1);
  if (bEnable)
    reg |= msk;
  else
    reg &= ~msk;
  SAND_HAL_FE2000_WRITE(pCtl->dev,  PC_COUNTER_RING_CTRL, reg);

  return SB_OK;
}

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuCounterGroupRegister()
 *
 * @brief Add a segment and it's associated host-side buffer that will cache
 *        the update messages sent by the the CMU. This function should be
 *        called for each unique counter group, implying that for each segment
 *
 * @param pCtl - Pointer to CMU control structure
 * @param ulSegment  - Segment Id (0 -> 31)
 * @param ulBank     - Bank Id (0 -> 3)
 * @param ulBankBase - Base address for this segment in the device memory
 * @param ulTblLimit - Max number of table entries in this segment
 * @param eType      - Counter Type (0=Turbo, 1=Range, 2=Legacy, 3=Chained)
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuCounterGroupRegister(sbFe2000CmuMgr_t *pCtl,
                                uint32_t *ulSegId,
                                uint32_t ulNumCounters,
                                uint32_t ulBank,
                                uint32_t ulBankBase,
                                sbFe2000SegTblType_e_t eType)
{
  sbStatus_t sts;
  uint32_t ulHostWords;
  void  *pBuf, *pBufOrig;

  /* Number of words per table entry. For chained legacy and turbo counters
   * there are should be two 64-bit words in host memory for each entry. The
   * The lower 64-bit word is the byte counter for the address, and the upper
   * 64-bit word is a packet counter. The ring will receive two separate update
   * messages (one for byte, one for packet). The turbo format is to preserve
   * MMU bandwidth, and should appear identical to legacy-chained */
  switch (eType) {
  case SB_FE2000_SEG_TBL_CNTR_TYPE_RANGE:
  case SB_FE2000_SEG_TBL_CNTR_TYPE_LEGACY:
    ulHostWords = ulNumCounters;
    break;
  case SB_FE2000_SEG_TBL_CNTR_TYPE_CHAINED_LEGACY:
  case SB_FE2000_SEG_TBL_CNTR_TYPE_TURBO:
    ulHostWords = ulNumCounters * 2;
    break;
  case SB_FE2000_SEG_TBL_CNTR_TYPE_MAX:
  default:
    ulHostWords = 0; /* satisfy gcc */
    /* illegal encoding */
    SB_ASSERT(0);
    break;
  }

  /* allocate the host-side cache buffer */
  pBuf = thin_host_malloc(sizeof(uint64_t) * ulHostWords + 7);
  if (!pBuf)
    return SB_MALLOC_FAILED;
  
  pBufOrig = pBuf;

  /* Fix for vxworks that had following assertion failure */
  pBuf = (char*)pBuf + 7;
  pBuf = (void*)((uint32_t)pBuf & ~0x7);

  /* Buffer needs to be 64-bit aligned. Most mallocs do this automatically
   * when they are compiled with uint64_t support, even on 32-bit archs.
   * Support can be added by allocating an extra 64-bit word and sliding
   * the buffer reference to be aligned. */
  SB_ASSERT((((uint32_t) pBuf) & 0x7) == 0);

  if(*ulSegId == -1) {
    /* map the segment for use */
    SB_LOGV2("Automatically Allocating Counter Seg %d\n", *ulSegId);
    sts = sbFe2000CmuSegmentSet(pCtl, pCtl->ulUsedSegments, ulBank, ulBankBase,
                                (uint32_t) pBuf, ulNumCounters, eType,
                                SB_FE2000_SEG_TBL_CNTR_EJECT_PCI_RAM);
    if (sts != SB_OK) {
      thin_host_free(pBufOrig);
      return sts;
    }
  } else {
    SB_LOGV2("Allocating Counter Seg %d\n", *ulSegId);
    sts = sbFe2000CmuSegmentSet(pCtl, *ulSegId, ulBank, ulBankBase,
                                (uint32_t) pBuf, ulNumCounters, eType,
                                SB_FE2000_SEG_TBL_CNTR_EJECT_PCI_RAM);
    if (sts != SB_OK) {
      thin_host_free(pBufOrig);
      return sts;
    }
    pCtl->ulUsedSegments = *ulSegId;
  }

  /* pass back the segment identifier, stash the allocated buffer and advance
   * the used-segment count. */
  *ulSegId = pCtl->ulUsedSegments;
  pCtl->ullCount[pCtl->ulUsedSegments] = pBuf;
  pCtl->pSegMem[pCtl->ulUsedSegments] = pBufOrig;
  pCtl->ulBank[pCtl->ulUsedSegments] = ulBank;
  pCtl->ulBase[pCtl->ulUsedSegments] = ulBankBase;
  pCtl->eType[pCtl->ulUsedSegments] = eType;
  pCtl->ulSize[pCtl->ulUsedSegments] = ulNumCounters;

  pCtl->ulUsedSegments++;

  return sts;
}

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuRingProcess()
 *
 * @brief Process entries from the CMU ring counter structure. This will empty
 *        entries from the ring update structure and synchronize their values
 *        with the shadowed 64-bit host-side counters. This needs to be called
 *        periodically, either by interrupt or polling to guarantee the ring
 *        has room for the CMU to operate.
 *
 * @param pCtl - Pointer to CMU control structure
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuRingProcess(sbFe2000CmuMgr_t *pCtl, int flush)
{
  uint32_t ulD0, ulD1;
  uint64_t *pData;
  uint8_t bWrapped = 0;
  uint32_t consumer, producer, flushevent, count=0;

  SB_ASSERT(pCtl->pRing);
  SB_ASSERT(pCtl->pRingEntry);

  producer = SAND_HAL_FE2000_READ(pCtl->dev, PC_COUNTER_RING_PRODUCER);
  consumer = SAND_HAL_FE2000_READ(pCtl->dev, PC_COUNTER_RING_CONSUMER);
  pCtl->pRingEntry = pCtl->pRing + (consumer / 4);


  while (1) {
    /* read next entry */
    ulD0 = SAND_FE_TO_HOST_32(pCtl->pRingEntry[0]);
    ulD1 = SAND_FE_TO_HOST_32(pCtl->pRingEntry[1]);

    /* parse the address out, 8-byte aligned */
    pData = (uint64_t *) (ulD1 & 0xffffFFF8);

    /* done if reached next blank entry */
    if (!pData)
      break;

    count++;

    /* xxx updating 64-bit memory words may not be atomic on some
     * xxx architectures as two 32-bit word writes may be interleaved
     * xxx by a read from another process. many architectures explicitly
     * xxx provide 64-bit locking on writes. */

    if ( !SOC_WARM_BOOT((uint32_t)pCtl->dev) && (!flush) ) {
      /* update cached copy of the counter */
      COMPILER_64_ADD_32(*pData, ulD0);
    }

    /* clear entry so chip can use this entry */
    pCtl->pRingEntry[0] = 0;
    pCtl->pRingEntry[1] = 0;

    /* advance ring pointer modulo ring size. to prevent tight-loop
     * problems check to see if we've already chased the chip around
     * the ring once. if so return, and we'll pick up processing on
     * the next periodic call */
    pCtl->pRingEntry += 2;
    consumer += 8;
    if (pCtl->pRingEntry >= (pCtl->pRing + (pCtl->ulRingSize/4) )) {
      pCtl->pRingEntry = pCtl->pRing;
      consumer=0;
      if (bWrapped)
        break;
      bWrapped = 1;
    }
  }

  /* single completion of flush */
  if (ulD1 & 0x1)
  {
    pCtl->bFlushing = 0;
  }

  /* clear manual flush event */
  flushevent = SAND_HAL_FE2000_READ(pCtl->dev,  CM_FORCE_FLUSH_EVENT);
  if (flushevent & SAND_HAL_FE2000_FIELD_MASK(pCtl->dev, CM_FORCE,FLUSH_EVENT_DONE)) {
    SAND_HAL_FE2000_WRITE(pCtl->dev,  CM_FORCE_FLUSH_EVENT, flushevent);
    pCtl->bFlushing = 0;
  }
  
  if (count) {
    /* update consumer */
    SAND_HAL_FE2000_WRITE(pCtl->dev,  PC_COUNTER_RING_CONSUMER, consumer);
  }else if (producer != consumer) {
    /* force consumer forward */
    SAND_HAL_FE2000_WRITE(pCtl->dev, PC_COUNTER_RING_CONSUMER, producer);
  }

  /* clear FIFO_DATA_AVAIL */
  SAND_HAL_FE2000_WRITE(pCtl->dev,  PC_INTERRUPT, SAND_HAL_FE2000_FIELD_MASK(pCtl->dev,  PC_INTERRUPT, COUNTER_FIFO_DATA_AVAIL));

  return SB_OK;
}

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuCounterRead()
 *
 * @brief Re
 *
 * @param pCtl      - Pointer to CMU control structure
 * @param ulSegment - Memory segment to read from
 * @param ulOffset  - Which entry/entries to read
 * @param ulEntries - Number of entries to read
 * @param ulData    - Pointer to array for return results
 * @param bSync     - Force flush synchronization
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuCounterRead(sbFe2000CmuMgr_t *pCtl,
                       uint32_t ulSegment,
                       uint32_t ulOffset,
                       uint32_t ulEntries,
                       uint64_t *ullData,
                       uint8_t bSync,
                       uint8_t bClear)
#ifndef PLISIM
{
  uint32_t      ulStride = 0;
  uint32_t      ulTimeout = CMU_TIMEOUT;
  uint64_t      *pullCount;
  uint32_t      i, base;
  uint32_t      flush_event, flush_event_done_mask;
  sbStatus_t    sts = SB_OK;
  uint32_t      bMutex = 0;

  if ( (bSync) && (!SOC_WARM_BOOT((uint32_t)pCtl->dev))) {
    sal_mutex_take(pCtl->cmu_flush_lock, sal_mutex_FOREVER);
    pCtl->bFlushing = 1;
    sts = sbFe2000CmuForceFlush(pCtl, ulSegment, ulOffset, ulEntries);
    if (sts != SB_OK) {
      SB_LOG("ERROR: CMU Force Flush already in progress. \n");
      sal_mutex_give(pCtl->cmu_flush_lock);
      return sts;
    }else{
      bMutex = 1;
    }
  }

  /* Number of words per table entry. For chained legacy and turbo counters
   * there are should be two 64-bit words in host memory for each entry. The
   * The lower 64-bit word is the byte counter for the address, and the upper
   * 64-bit word is a packet counter. The ring will receive two separate update
   * messages (one for byte, one for packet). The turbo format is to preserve
   * MMU bandwidth, and should appear identical to legacy-chained */
  switch (pCtl->eType[ulSegment]) {
  case SB_FE2000_SEG_TBL_CNTR_TYPE_RANGE:
  case SB_FE2000_SEG_TBL_CNTR_TYPE_LEGACY:
    ulStride = 1;
    break;
  case SB_FE2000_SEG_TBL_CNTR_TYPE_CHAINED_LEGACY:
  case SB_FE2000_SEG_TBL_CNTR_TYPE_TURBO:
    ulStride = 2;
    break;
  case SB_FE2000_SEG_TBL_CNTR_TYPE_MAX:
  default:
    SB_ASSERT(0); /* illegal encoding */
    ulStride = 0; /* satisfy gcc */
    break;
  }

  /* wait for force flush to finish */
  while (pCtl->bFlushing && ulTimeout && bSync) {
    thin_delay(100);
    ulTimeout--;
  }

  if (pCtl->cmu_watermark > ulTimeout) {
    SB_LOG("CMU force flush watermark = %d \n", ulTimeout);
    pCtl->cmu_watermark = ulTimeout; 
  }

  if (!ulTimeout) {
      /* check if the done bit is set in flush event. */
      flush_event = SAND_HAL_FE2000_READ(pCtl->dev, CM_FORCE_FLUSH_EVENT);
      flush_event_done_mask = SAND_HAL_FE2000_FIELD_MASK(pCtl->dev, 
                                                    CM_FORCE,FLUSH_EVENT_DONE);
      if (flush_event & flush_event_done_mask) {
          /* cmu flush completed but interrupt is not received in time. 
             Return value from last successful flush */
          SB_LOG("WARNING: did not receive CM_FORCE_FLUSH_EVENT interrupt "
                 "but done bit set. \n");
          sts = SB_TIMEOUT_ERR_CODE;
      } else {
          /* cmu flush operation did not complete */
          SB_LOG("CMU flush timed out!\n");
          if (bSync) {
              sal_mutex_give(pCtl->cmu_flush_lock);
          }
          return SB_OTHER_ERR_CODE; /* translates to SOC_E_FAIL */
      }
  }

  /* get ahold of the buffer, and pass back the data */
  pullCount = pCtl->ullCount[ulSegment];
  for (i=0; i<ulEntries; i++)
  {
    base = i * ulStride;
    ullData[base] = pullCount[ulOffset * ulStride + 0];
    if (ulStride == 2)
        ullData[base+1] = pullCount[ulOffset * ulStride + 1];
    if (bClear) {
      COMPILER_64_ZERO(pullCount[ulOffset * ulStride + 0]);
      if (ulStride == 2)
        COMPILER_64_ZERO(pullCount[ulOffset * ulStride + 1]);
    }
    ulOffset++;
  }

  if (bMutex) {
      sal_mutex_give(pCtl->cmu_flush_lock);
  }

  return sts;
}
#else
/* xxx - this is a hack because the model doesn't eject counters
 * xxx - this is for testing only and should never be used in
 * xxx - the real world. */
{
  sbStatus_t sts;
  uint32_t ulBank, ulMemSrc, ulData, ulBytes, ulPackets, ulBase, ulIdx,ulDataClear;
  sbFe2000DmaAddr_t ulAddr;

  /* 
   * The code below is assuming that the counters are formatted internally 
   * as turbo counters. It will crash if that assumption is no longer valid.
   * This segment of code is purely for simulation.
   */
  SB_ASSERT(pCtl->eType[ulSegment] == SB_FE2000_SEG_TBL_CNTR_TYPE_TURBO);
  
  ulBank = pCtl->ulBank[ulSegment];
  ulBase = pCtl->ulBase[ulSegment];

  switch (ulBank) {
  case 0: ulMemSrc = SB_FE2000_MEM_MM0_INTERNAL_0; break;
  case 1: ulMemSrc = SB_FE2000_MEM_MM0_NARROW_1; break;
  case 2: ulMemSrc = SB_FE2000_MEM_MM1_NARROW_0; break;
  case 3: ulMemSrc = SB_FE2000_MEM_MM1_NARROW_1; break;
  default:
    ulMemSrc = 0; /* satisfy gcc */
    SB_ASSERT(0);
    break;
  }

  for (ulIdx = 0; ulIdx < ulEntries; ulIdx++) {
    ulAddr = SB_FE2000_DMA_MAKE_ADDRESS(ulMemSrc, ulBase + ulOffset + ulIdx);

    sts = sbFe2000DmaReadIndirect(pCtl->pDmaState, ulAddr, &ulData, 1);
    if (sts != SB_OK)
      return sts;

    if(bClear) {
      /* don't use this in a real system as it is not write synchronous. */
      ulDataClear = 0;
      sts = sbFe2000DmaWriteIndirect(pCtl->pDmaState, ulAddr, &ulDataClear);
      if (sts != SB_OK)
        return sts;
    }


    /* xxx this is busted, limited to 32 bits */
    ulBytes = ulData & 0x1FFFFF;
    ulPackets = (ulData >> 21) & 0x3FFF;

    ullData[ulIdx * 2 + 0] = ulBytes;
    ullData[ulIdx * 2 + 1] = ulPackets;
  }

  return SB_OK;
}
#endif

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuSegmentSet()
 *
 * @brief Write a set of CMU segment control registers to prepare it for use.
 *
 * @param pCtl       - Pointer to CMU control structure
 * @param ulSegment  - Segment Id (0 -> 31)
 * @param ulBank     - Bank Id (0 -> 3)
 * @param ulBankBase - Base address for this segment in the device memory
 * @param ulPciBase  - Base address for this segment in host PCI memory
 * @param ulTblLimit - Max number of table entries in this segment
 * @param eType      - Counter Type (0=Turbo, 1=Range, 2=Legacy, 3=Chained)
 * @param eEject     - Eject Type (0=External PCI Memory, 1=MMU Int Memory)
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuSegmentSet(sbFe2000CmuMgr_t *pCtl,
                      uint32_t ulSegment,
                      uint32_t ulBank,
                      uint32_t ulBankBase,
                      uint32_t ulPciBase,
                      uint32_t ulTblLimit,
                      sbFe2000SegTblType_e_t eType,
                      sbFe2000SegTblEject_e_t eEject)
{
  uint32_t ulOffset, ulCfgAddr, ulMmuAddr, ulPciAddr, ulLimitAddr;
  uint32_t ulPciBaseDw, ulCfgData, ulMmuData, ulLimitData, ulPciData;
  uint32_t ulSegEnable;

  /* check arguments */
  if ((!pCtl) ||
      (ulSegment >= SB_FE2000_SEG_TBL_SEG_MAX) ||
      (ulBank >= SB_FE2000_SEG_TBL_SEG_BANKS_MAX) ||
      (ulBankBase > MALIGN(pCtl->dev, CM_SEGMENT_TABLE_MMU0_BANK_BASE)) ||
      ((ulPciBase>>3) > MALIGN(pCtl->dev, CM_SEGMENT_TABLE_PCI0_PCI_BASE)) ||
      (ulTblLimit > MALIGN(pCtl->dev, CM_SEGMENT_TABLE_LIMIT0_LIMIT)) ||
      (eType >= SB_FE2000_SEG_TBL_CNTR_TYPE_MAX) ||
      (eEject >= SB_FE2000_SEG_TBL_CNTR_EJECT_MAX))
    return SB_BAD_ARGUMENT_ERR_CODE;

  /* xxx todo validate range in bank base is not beyond end of physical mem */

  /* strided in blocks of four registers, calculate which set we'll
   * be configuring on this call and calculate the address of the registers */
  ulOffset = ulSegment * (4 * sizeof(uint32_t));
  ulCfgAddr = SAND_HAL_FE2000_REG_OFFSET(pCtl->dev, CM_SEGMENT_TABLE_CONFIG0) + ulOffset;
  ulMmuAddr = SAND_HAL_FE2000_REG_OFFSET(pCtl->dev, CM_SEGMENT_TABLE_MMU0) + ulOffset;
  ulPciAddr = SAND_HAL_FE2000_REG_OFFSET(pCtl->dev, CM_SEGMENT_TABLE_PCI0) + ulOffset;
  ulLimitAddr = SAND_HAL_FE2000_REG_OFFSET(pCtl->dev, CM_SEGMENT_TABLE_LIMIT0) + ulOffset;

  /* the pci address field is defined as a double word aligned (64-bit)
   * and is stored in the register field with the 3 LSBs removed. */
  ulPciBaseDw = ulPciBase >> 3;

  /* compose the contents of the registers */
  ulCfgData = (SAND_HAL_FE2000_SET_FIELD(pCtl->dev, CM_SEGMENT_TABLE_CONFIG0, BANK, ulBank)|
               SAND_HAL_FE2000_SET_FIELD(pCtl->dev, CM_SEGMENT_TABLE_CONFIG0, CNTR_TYPE, eType)|
               SAND_HAL_FE2000_SET_FIELD(pCtl->dev, CM_SEGMENT_TABLE_CONFIG0, EJECT, eEject));
  ulMmuData = SAND_HAL_FE2000_SET_FIELD(pCtl->dev, CM_SEGMENT_TABLE_MMU0, BANK_BASE, ulBankBase);
  ulPciData = SAND_HAL_FE2000_SET_FIELD(pCtl->dev, CM_SEGMENT_TABLE_PCI0, PCI_BASE, ulPciBaseDw);
  ulLimitData = SAND_HAL_FE2000_SET_FIELD(pCtl->dev, CM_SEGMENT_TABLE_LIMIT0, LIMIT, ulTblLimit);

  /* write out the segment control registers */
  SAND_HAL_FE2000_WRITE_OFFS(pCtl->dev, ulCfgAddr, ulCfgData);
  SAND_HAL_FE2000_WRITE_OFFS(pCtl->dev, ulMmuAddr, ulMmuData);
  SAND_HAL_FE2000_WRITE_OFFS(pCtl->dev, ulPciAddr, ulPciData);
  SAND_HAL_FE2000_WRITE_OFFS(pCtl->dev, ulLimitAddr, ulLimitData);

  /* enable the segment */
  ulSegEnable = SAND_HAL_FE2000_READ(pCtl->dev,  CM_SEGMENT_ENABLE);
  ulSegEnable = ulSegEnable | ( 0x1 << ulSegment );
  SAND_HAL_FE2000_WRITE(pCtl->dev,  CM_SEGMENT_ENABLE, ulSegEnable);

  return SB_OK;
}

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuSegmentEnable()
 *
 * @brief Enable or disable a segment
 *
 * @param pCtl       - Pointer to CMU control structure
 * @param ulSegment  - Segment Id (0 -> 31)
 * @param bEnable    - Enable = 1, Disable = 0
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuSegmentEnable(sbFe2000CmuMgr_t *pCtl,
                         uint32_t ulSegment,
                         uint8_t bEnable)
{
  uint32_t ulReg, ulMask;

  if ((!pCtl) || (ulSegment > SB_FE2000_SEG_TBL_SEG_MAX))
    return SB_BAD_ARGUMENT_ERR_CODE;

  ulReg = SAND_HAL_FE2000_READ(pCtl->dev,  CM_SEGMENT_ENABLE);
  ulMask = (0x1 << ulSegment);
  if (bEnable)
    ulReg |= ulMask;
  else
    ulReg &= ~ulMask;
  SAND_HAL_FE2000_WRITE(pCtl->dev,  CM_SEGMENT_ENABLE, ulReg);

  return SB_OK;
}

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuRanceConfig()
 *
 * @brief Configure the CMU's RANdomized Counter Ejection (RANCE).
 *
 * @param pCtl             - Pointer to CMU control structure
 * @param Legacy Prob En   - Enable static eject prob for legacy config
 * @param Byte Prob En     - Enable static eject prob for byte part of turbo config
 * @param Packet Prob En   - Enable static eject prob for packet part of turbo config
 * @param LegacyDivLog     - If static_legacy_prob_en is set, the packet accumulation
 *                           amount is shifted to the right by byte_divisor_log bit
 *                           positions in order to obtain the eject probability.
 * @param ByteDivisorLog   - If static_byte_prob_en is set, the packet accumulation
 *                           amount is shifted to the right by byte_divisor_log bit
 *                           positions in order to obtain the eject probability.
 * @param PacketDivisorLog - If static_packet_prob_en is set, the packet accumulation
 *                           amount is shifted to the right by packet_divisor_log bit
 *                           positions in order to obtain the eject probability.
 * @param Seed0            - Seed for Linear Feedback Shift Register 0
 * @param Seed1            - Seed for Linear Feedback Shift Register 1
 * @param Seed2            - Seed for Linear Feedback Shift Register 2
 * @param Seed3            - Seed for Linear Feedback Shift Register 3
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuRanceConfig(sbFe2000CmuMgr_t *pCtl,
                       uint8_t bStaticLegacyProbEn,
                       uint8_t bStaticByteProbEn,
                       uint8_t bStaticPacketProbEn,
                       uint32_t ulLegacyDivisorLog,
                       uint32_t ulByteDivisorLog,
                       uint32_t ulPacketDivisorLog,
                       uint32_t ulSeed0,
                       uint32_t ulSeed1,
                       uint32_t ulSeed2,
                       uint32_t ulSeed3)
{
  uint32_t ulRance;

  /* check arguments */
  if ((!pCtl) ||
      (ulLegacyDivisorLog > MALIGN(pCtl->dev, CM_CONFIG_RANCE_LEGACY_DIVISOR_LOG)) ||
      (ulByteDivisorLog > MALIGN(pCtl->dev, CM_CONFIG_RANCE_BYTE_DIVISOR_LOG)) ||
      (ulPacketDivisorLog > MALIGN(pCtl->dev, CM_CONFIG_RANCE_PACKET_DIVISOR_LOG)))
    return SB_BAD_ARGUMENT_ERR_CODE;

  /* compose range config register */
  ulRance =
    (SAND_HAL_FE2000_SET_FIELD(pCtl->dev, CM_CONFIG_RANCE, STATIC_LEGACY_PROB_EN, bStaticLegacyProbEn) |
     SAND_HAL_FE2000_SET_FIELD(pCtl->dev, CM_CONFIG_RANCE, LEGACY_DIVISOR_LOG, ulLegacyDivisorLog) |
     SAND_HAL_FE2000_SET_FIELD(pCtl->dev, CM_CONFIG_RANCE, STATIC_BYTE_PROB_EN, bStaticByteProbEn) |
     SAND_HAL_FE2000_SET_FIELD(pCtl->dev, CM_CONFIG_RANCE, BYTE_DIVISOR_LOG, ulByteDivisorLog) |
     SAND_HAL_FE2000_SET_FIELD(pCtl->dev, CM_CONFIG_RANCE, STATIC_PACKET_PROB_EN, bStaticPacketProbEn) |
     SAND_HAL_FE2000_SET_FIELD(pCtl->dev, CM_CONFIG_RANCE, PACKET_DIVISOR_LOG, ulPacketDivisorLog));

  /* write out configurations */
  SAND_HAL_FE2000_WRITE(pCtl->dev,  CM_CONFIG_RANCE, ulRance);
  SAND_HAL_FE2000_WRITE(pCtl->dev,  CM_LFSR0_SEED, ulSeed0);
  SAND_HAL_FE2000_WRITE(pCtl->dev,  CM_LFSR1_SEED, ulSeed1);
  SAND_HAL_FE2000_WRITE(pCtl->dev,  CM_LFSR2_SEED, ulSeed2);
  SAND_HAL_FE2000_WRITE(pCtl->dev,  CM_LFSR3_SEED, ulSeed3);

  return SB_OK;
}

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuSegmentFlush()
 *
 * @brief Enable or disable flushing (Forced Counter Ejection) on a specified
 *        segment. When this is enabled, the counter manager will cycle through
 *        a segment and serially eject counters for handling. When it reaches
 *        the end of a segment, it will wrap and continue.
 *
 * @param pCtl       - Pointer to CMU control structure
 * @param ulSegment  - Segment to perform flushing on
 * @param bEnable    - Enable or disable ejection on this segment
 * @param ulRateData - Configure rate of counter ejection.
 *
 * @return error code, SB_OK on success
 *-----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuSegmentFlush(sbFe2000CmuMgr_t *pCtl,
                        uint32_t ulSegment,
                        uint8_t bEnable,
                        uint32_t ulRateData)
{
  uint32_t ulOffset, ulCtrlAddr, ulRateAddr, ulCtrlData;

  if ((!pCtl) || (ulSegment > SB_FE2000_SEG_TBL_SEG_MAX))
    return SB_BAD_ARGUMENT_ERR_CODE;

  /* strided in blocks of two registers, calculate which set we'll be
   * configuring on this call and calculate the address of registers */
  ulOffset = ulSegment * (2 * sizeof(uint32_t));
  ulCtrlAddr = SAND_HAL_FE2000_REG_OFFSET(pCtl->dev, CM_CONFIG_SEGMENT_FLUSH_CTRL0) + ulOffset;
  ulRateAddr = SAND_HAL_FE2000_REG_OFFSET(pCtl->dev, CM_CONFIG_SEGMENT_FLUSH_RATE0) + ulOffset;

  ulCtrlData = SAND_HAL_FE2000_SET_FIELD(pCtl->dev, CM_CONFIG_SEGMENT_FLUSH_CTRL0, ENABLE, bEnable);

  SAND_HAL_FE2000_WRITE_OFFS(pCtl->dev, ulCtrlAddr, ulCtrlData);
  SAND_HAL_FE2000_WRITE_OFFS(pCtl->dev, ulRateAddr, ulRateData);

  return SB_OK;
}

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuSegmentFlushStatus()
 *
 * @brief Return current index in flush progress for a segment. If no flush is
 *        is in progress, return BUSY error code.
 *
 * @param pCtl       - Pointer to CMU control structure
 * @param ulSegment  - Segment performing flushing
 * @param ulCntrId   - Current location in segment space
 *
 * @return error code, SB_OK on success
 *-----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuSegmentFlushStatus(sbFe2000CmuMgr_t *pCtl,
                              uint32_t ulSegment,
                              uint32_t *ulCntrId)
{
  uint32_t ulOffset, ulCtrlAddr, ulReg, ulCntrIdx;
  uint8_t bInProgress;

  if ((!pCtl) || (ulSegment > SB_FE2000_SEG_TBL_SEG_MAX))
    return SB_BAD_ARGUMENT_ERR_CODE;

  /* strided in blocks of two registers, calculate which set we'll be
   * configuring on this call and calculate the address of registers */
  ulOffset = ulSegment * (2 * sizeof(uint32_t));
  ulCtrlAddr = SAND_HAL_FE2000_REG_OFFSET(pCtl->dev, CM_CONFIG_SEGMENT_FLUSH_CTRL0) + ulOffset;

  /* get field and parse it */
  ulReg = SAND_HAL_FE2000_READ_OFFS(pCtl->dev, ulCtrlAddr);
  ulCntrIdx = SAND_HAL_FE2000_GET_FIELD(pCtl->dev, CM_CONFIG_SEGMENT_FLUSH_CTRL0, FLUSH_COUNTER, ulReg);
  bInProgress = SAND_HAL_FE2000_GET_FIELD(pCtl->dev, CM_CONFIG_SEGMENT_FLUSH_CTRL0, IN_PROGRESS, ulReg);

  if (!bInProgress)
    return SB_BUSY_ERR_CODE;

  *ulCntrId = ulCntrIdx;

  return SB_OK;
}

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuForceFlush()
 *
 * @brief Request a manual flush of a specified counter or counters to the host
 *        for handling. Currently, the hardware only supports ranges (n -> 0)
 *        but we should be adding support for individual counters.
 *
 * @param pCtl      - Pointer to CMU control structure
 * @param ulSegment - Segment to perform the forced flush on
 * @param ulOffset  - Offset within that segment
 * @param ulSize    - Number of entries to flush
 *
 * @return error code, SB_OK on success
 *-----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuForceFlush(sbFe2000CmuMgr_t *pCtl,
                      uint32_t ulSegment,
                      uint32_t ulOffset,
                      uint32_t ulSize)
{
  uint32_t ulReg;

  if ((!pCtl) || (ulSegment > SB_FE2000_SEG_TBL_SEG_MAX) ||
      (ulOffset > MALIGN(pCtl->dev, CM_CONFIG_FORCED_FLUSH_STARTING_OFFSET)))
    return SB_BAD_ARGUMENT_ERR_CODE;

  /* check another flush isn't in progress */
  ulReg = SAND_HAL_FE2000_READ(pCtl->dev,  CM_CONFIG_FORCED_FLUSH);
  if (SAND_HAL_FE2000_GET_FIELD(pCtl->dev, CM_CONFIG_FORCED_FLUSH, IN_PROGRESS, ulReg))
    return SB_BUSY_ERR_CODE;

  /* clear the flush event done. Only one pending force flush event allowed. 
     protected by cmu_flush_lock */
  ulReg = SAND_HAL_FE2000_SET_FIELD(pCtl->dev, CM_FORCE, FLUSH_EVENT_DONE, 1);
  SAND_HAL_FE2000_WRITE(pCtl->dev, CM_FORCE_FLUSH_EVENT, ulReg);

  /* number of entries to flush (minus 1) */
  SAND_HAL_FE2000_WRITE(pCtl->dev,  CM_CONFIG_FORCE_FLUSH_SIZE, ulSize - 1);

  /* compose and issue request */
  ulReg = (SAND_HAL_FE2000_SET_FIELD(pCtl->dev, CM_CONFIG_FORCED_FLUSH, GO, 1) |
           SAND_HAL_FE2000_SET_FIELD(pCtl->dev, CM_CONFIG_FORCED_FLUSH, SEGMENT_NUM, ulSegment) |
           SAND_HAL_FE2000_SET_FIELD(pCtl->dev, CM_CONFIG_FORCED_FLUSH, STARTING_OFFSET, ulOffset));
  SAND_HAL_FE2000_WRITE(pCtl->dev,  CM_CONFIG_FORCED_FLUSH, ulReg);

  return SB_OK;
}
/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuSegmentClear()
 *
 * @brief Clear the counter segment memory, respecting the protection scheme of
 *        the CMU.
 *
 * @param pCtl - Referenced pointer to CMU control structure
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuSegmentClear(sbFe2000CmuMgr_t *pCtl, uint32_t ulSegment)
{
    uint32_t uMmProtectionScheme0, uMmProtectionScheme0Stash;
    uint32_t uMmProtectionScheme1, uMmProtectionScheme1Stash;
    uint32_t uInstance;
    uint32_t v, v0, i, ulAddr, ulMemSrc, ulBank, ulBase, ulWords;
    uint32_t interrupt_mask;

    /* update mm0 configuration to respect parity of CMU client */
    uInstance = 0;
    uMmProtectionScheme0Stash = SAND_HAL_FE2000_READ_STRIDE( pCtl->dev,  MM, uInstance, MM_PROT_SCHEME0 );
    uMmProtectionScheme0 = uMmProtectionScheme0Stash;
    /* CMU is secondary client of narrow port 0 and 1 on MMU 0 */
    uMmProtectionScheme0 = SAND_HAL_FE2000_MOD_FIELD(pCtl->dev,  MM_PROT_SCHEME0, NP0_DMA_SCHEME,  uMmProtectionScheme0, SB_FE2000_MM_35BITS_1BITPARITY);
    uMmProtectionScheme0 = SAND_HAL_FE2000_MOD_FIELD(pCtl->dev,  MM_PROT_SCHEME0, NP1_DMA_SCHEME,  uMmProtectionScheme0, SB_FE2000_MM_35BITS_1BITPARITY);
    SAND_HAL_FE2000_WRITE_STRIDE( pCtl->dev,  MM, uInstance, MM_PROT_SCHEME0, uMmProtectionScheme0 );

    uInstance = 1;
    uMmProtectionScheme1Stash = SAND_HAL_FE2000_READ_STRIDE( pCtl->dev,  MM, uInstance, MM_PROT_SCHEME0 );
    uMmProtectionScheme1 = uMmProtectionScheme1Stash;
    /* CMU is secondary client of narrow port 0 and 1 on MMU 0 */
    uMmProtectionScheme1 = SAND_HAL_FE2000_MOD_FIELD(pCtl->dev,  MM_PROT_SCHEME0, NP0_DMA_SCHEME,  uMmProtectionScheme1, SB_FE2000_MM_35BITS_1BITPARITY);
    uMmProtectionScheme1 = SAND_HAL_FE2000_MOD_FIELD(pCtl->dev,  MM_PROT_SCHEME0, NP1_DMA_SCHEME,  uMmProtectionScheme1, SB_FE2000_MM_35BITS_1BITPARITY);
    SAND_HAL_FE2000_WRITE_STRIDE( pCtl->dev,  MM, uInstance, MM_PROT_SCHEME0, uMmProtectionScheme1 );

    interrupt_mask = SOC_IRQ_MASK((uint32_t)pCtl->dev);
    SOC_IRQ_MASK((uint32_t)pCtl->dev) = 0x00001fff;
    SAND_HAL_FE2000_WRITE(pCtl->dev, PC_INTERRUPT_MASK, SOC_IRQ_MASK((uint32_t)pCtl->dev));

    /* clear counter segment memory */
    v0 = 0;
    v0 = SAND_HAL_FE2000_MOD_FIELD(pCtl->dev, PC_DMA_CTRL0, ACK, v0, 1);
    v0 = SAND_HAL_FE2000_MOD_FIELD(pCtl->dev, PC_DMA_CTRL0, REQ, v0, 1);
    v0 = SAND_HAL_FE2000_MOD_FIELD(pCtl->dev, PC_DMA_CTRL0, FIXED_PTRN, v0, 1);
    v0 = SAND_HAL_FE2000_MOD_FIELD(pCtl->dev, PC_DMA_CTRL0, DMA_SRC_TYPE, v0, 1);

    ulBank = pCtl->ulBank[ulSegment];
    ulBase = pCtl->ulBase[ulSegment];
    ulWords = pCtl->ulSize[ulSegment];


    switch (ulBank) {
    case 0: ulMemSrc = SB_FE2000_MEM_MM0_INTERNAL_0; break;
    case 1: ulMemSrc = SB_FE2000_MEM_MM0_NARROW_1; break;
    case 2: ulMemSrc = SB_FE2000_MEM_MM1_NARROW_0; break;
    case 3: ulMemSrc = SB_FE2000_MEM_MM1_NARROW_1; break;
    default:
        ulMemSrc = 0; /* satisfy gcc */
        SB_ASSERT(0);
        break;
    }

    v = SAND_HAL_FE2000_MOD_FIELD(pCtl->dev, PC_DMA_CTRL0, DMA_SRC, v0, ulMemSrc);

    ulAddr = SB_FE2000_DMA_MAKE_ADDRESS(ulMemSrc, ulBase );

    SAND_HAL_FE2000_WRITE(pCtl->dev,  PC_DMA_CLI_ADDR0, ulAddr);
    SAND_HAL_FE2000_WRITE(pCtl->dev,  PC_DMA_PCI_ADDR0, 0x0);
    SAND_HAL_FE2000_WRITE(pCtl->dev,  PC_DMA_COUNT0, ulWords * 2);
    SAND_HAL_FE2000_WRITE(pCtl->dev,  PC_DMA_CTRL0, v);
    for (i = 0; i < 5000; i++) {
        if ( SAND_HAL_FE2000_READ(pCtl->dev,  PC_DMA_CTRL0)
             & SAND_HAL_FE2000_FIELD_MASK(pCtl->dev, PC_DMA, CTRL0_ACK)) {
            break;
        }
        thin_delay(10000);
    }
    SAND_HAL_FE2000_WRITE(pCtl->dev,  PC_DMA_CTRL0,
                   SAND_HAL_FE2000_FIELD_MASK(pCtl->dev, PC_DMA, CTRL0_ACK));
    if (i == 5000) {
      SOC_IRQ_MASK((uint32_t)pCtl->dev) = interrupt_mask;
      SAND_HAL_FE2000_WRITE(pCtl->dev, PC_INTERRUPT_MASK, SOC_IRQ_MASK((uint32_t)pCtl->dev));
      return SB_FE_MEM_ACC_TIMEOUT_ERR_CODE;
    }

    /* restore orig memory config */
    uInstance=0;
    SAND_HAL_FE2000_WRITE_STRIDE( pCtl->dev,  MM, uInstance, MM_PROT_SCHEME0,
                           uMmProtectionScheme0Stash );
    uInstance++;
    SAND_HAL_FE2000_WRITE_STRIDE( pCtl->dev,  MM, uInstance, MM_PROT_SCHEME0,
                           uMmProtectionScheme1Stash );

    SOC_IRQ_MASK((uint32_t)pCtl->dev) = interrupt_mask;
    SAND_HAL_FE2000_WRITE(pCtl->dev, PC_INTERRUPT_MASK, SOC_IRQ_MASK((uint32_t)pCtl->dev));

    /* Clear the host memory region */
    if (pCtl->eType[ulSegment] == SB_FE2000_SEG_TBL_CNTR_TYPE_CHAINED_LEGACY
        || pCtl->eType[ulSegment] == SB_FE2000_SEG_TBL_CNTR_TYPE_TURBO) {
      ulWords *= 2;
    }

    SB_MEMSET(pCtl->ullCount[ulSegment], 0, sizeof(uint64_t) * ulWords);

    return SB_OK;
}
