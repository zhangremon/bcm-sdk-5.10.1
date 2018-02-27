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
 * $Id: sbFe2000DmaMgr.c 1.7.112.1 Broadcom SDK $
 *
 * sbFe2000DmaMgr.h : FE2000 DMA Engine Driver
 *
 *-----------------------------------------------------------------------------*/

#include "glue.h"
#include "soc/drv.h"
#include <soc/sbx/sbx_drv.h>
#include <soc/debug.h>
#include <sal/appl/io.h>
#include "hal_user.h"
#include "hal_ca_c2.h"
#include <soc/sbx/fe2k_common/sbFe2000DmaMgr.h>
#include "sbWrappers.h"
#include "sbx_drv.h"

#define SB_FE2000_INT_MEM_SIZE (8192 * 2)
#define SB_FE2000XT_INT_MEM_SIZE (16384 * 2)
#define SB_FE2000_RING_ENTRY_SIZE_BYTES (16)

#define IS_POW2(x) ((x) && (((x) & ((x) - 1)) == 0))

/* define fields in control word of ring entry */
#define SB_FE2000_RING_CTL_ST_SHIFT      (22)
#define SB_FE2000_RING_CTL_ST_MASK       (0x1)
#define SB_FE2000_RING_CTL_DMA_SRC_SHIFT (23)
#define SB_FE2000_RING_CTL_DMA_SRC_MASK  (0xf)
#define SB_FE2000_RING_CTL_INT_SHIFT     (27)
#define SB_FE2000_RING_CTL_INT_MASK      (1)
#define SB_FE2000_RING_CTL_COR_SHIFT     (28)
#define SB_FE2000_RING_CTL_COR_MASK      (1)
#define SB_FE2000_RING_CTL_RWN_SHIFT     (29)
#define SB_FE2000_RING_CTL_NWORDS_MASK   (0x3)
#define SB_FE2000_RING_CTL_NWORDS_SHIFT  (28)
#define SB_FE2000_RING_CTL_RWN_MASK      (1)
#define SB_FE2000_RING_CTL_IM_SHIFT      (30)
#define SB_FE2000_RING_CTL_IM_MASK       (1)
#define SB_FE2000_RING_CTL_OWNR_SHIFT    (31)
#define SB_FE2000_RING_CTL_OWNR_MASK     (1)

#define SET_RFLD(name,val) (((val) & SB_FE2000_RING_CTL_##name##_MASK) \
                            << (SB_FE2000_RING_CTL_##name##_SHIFT))


#define _sbFe200DmaMgr_D(string)  "[%s]: " string, FUNCTION_NAME()

/*-----------------------------------------------------------------------------
 * Module-scoped data types
 *-----------------------------------------------------------------------------*/
typedef struct sbFe2000DmaRegs_s
{
  sbFe2000DevAddr_t devBaseAddr; /* chip reference */
  uint32_t devAddress;
  uint32_t hostAddress;
  uint32_t wordCount;
  uint32_t request;
} sbFe2000DmaChanRegs_t;

typedef struct sbFe2000DmaPioRegs_s
{
  sbFe2000DevAddr_t devBaseAddr;
  uint32_t request;
  uint32_t data;
  int words;
  uint32_t addressMask;
  uint32_t bReverseLayout;
} sbFe2000DmaPioRegs_t;

/** Core engine state **/
struct sbFe2000DmaMgr_s {

  /* operations waiting for access to the data mover */
  dq_t waitingOps;

  /* mutex for access to the data mover */
  sal_sem_t sync_sem;

  /* the active ops at any time */
  dq_t activeOps;
  int dmasActive;

  /*
   * The next three members implement a software ring in which each
   * element is one hardware DMA request register set (0 = A, 1 = B).
   * The FE hardware alternates using register set A and B, and this
   * ring makes the data mover behave correspondingly.
   * The producer is the the DMA start function, and the consumer
   * the DMA interrupt service routine.
   */
  int regsProducer;
  int regsConsumer;
  sbFe2000DmaChanRegs_t dmaRegs[SB_FE2000_DMA_CHANNELS];

  /*
   * PIO writes are `posted': completion of the previous write is tested
   * before beginning another PIO or DMA operation to the same memory element.
   * These flags indicate that the last operation to a memory element
   * was a PIO write.
   */
  int writePosted[SB_FE2000_MEM_MAX_SRCS];

  /*
   * This array regularizes PIO access to the various different
   * FE memory elements which are accessible by DMA and PIO.
   */
  sbFe2000DmaPioRegs_t pioRegs[SB_FE2000_MEM_MAX_SRCS];

  /* Slab operation state */
  dq_t slabsWaitingStart;
  /* There is are at most two slabs awaiting a last DMA operation at once */
  dq_t slabsWaitingLast;

  /*
   * There is at most one in-progress (some, but not all corresponding
   * DMA operations starte) slab operation.
   * slabCurrent == NULL if there is no slab operation in-progress (there may
   * still be up to two slab operations awaiting the completion of their
   * final DMAs in slabsWaitingLast).
   * slabCurrentChunk tracks progress through an in-progress slab
   * slabPreviousChunk is initialized to NULL and may subsequently
   * be used to identify the host memory target of a READ_WRITE operation.
   * slabChunkCurrentWord is used to track the current word within a chunk
   * for a READ_WORDS operation.
   */
  sbFe2000DmaSlabOp_t *slabCurrent;
  sbFe2000DmaSlab_t *slabCurrentChunk;
  sbFe2000DmaSlab_t *slabPreviousChunk;
  uint32_t slabChunkCurrentWord;

  /*
   * The slab processing has at most two individual DMA operations in
   * progress.  These DMAs may be associated with either an in-progress slab
   * operation, or a slab waiting for final completion in slabsWaitingLast.
   * The producer to the ring is the chunk start function, and the consumer
   * is the DMA completion function.
   */
  int slabActiveDmas;
  int slabDmaProducer;
  sbFe2000DmaOp_t slabDmas[2];

  /* backpointer reference for callbacks */
  void *clientData;

  /* use ring/thread based dma */
  uint8_t bUseRing;

  /* configuration */
  sbFe2000DmaConfig_t *pCfg;

  /* dma issue mode dma/pio/both */
  sbFe200DmaIssueMode_e_t eDmaMode;

  /* ring related information */
  uint32_t *pRingMem;
  uint32_t *pRingPtr;
  uint32_t ulProducerOffs;
  uint32_t ulConsumerOffs;
  uint32_t ulRingEntries;
  uint32_t ulRingMask;
  uint32_t ulRingSize;
  sbDmaMemoryHandle_t dmaHdl;
};

/*-----------------------------------------------------------------------------
 * Module-scoped function prototypes
 *-----------------------------------------------------------------------------*/
static uint8_t
sbFe2000DmaIsStartable(sbFe2000DmaMgr_t *state, sbFe2000DmaOp_t *op);
static sbStatus_t
sbFe2000DmaCheckOpArguments(sbFe2000DmaMgr_t *state, sbFe2000DmaOp_t *op);
static sbStatus_t
sbFe2000DmaStart(sbFe2000DmaMgr_t *state, sbFe2000DmaOp_t *op);
static sbStatus_t
sbFe2000DmaWaitBankIdle(sbFe2000DmaMgr_t *state, uint32_t feAddress);
static sbStatus_t
sbFe2000DmaWaitIndirect(uint32_t timeout, sbFe2000DmaPioRegs_t *bank);
static void sbFe2000DmaDone(dq_t *op, int a);
static void sbFe2000DmaSlabDmaDone(sbFe2000DmaOp_t *dmaOp);
static void sbFe2000DmaSlabDone(dq_t *op0, int a);
static uint8_t sbFe2000DmaSlabStartChunk(sbFe2000DmaMgr_t *state);
static uint32_t sbFe2000DmaRingNext(sbFe2000DmaMgr_t *state, uint32_t addr);

/*-----------------------------------------------------------------------------
 * Core DMA Engine
 *
 * These functions handle the transfer of data between the driver and the FE.
 * There are several ways to transfer data:
 *
 * 1 - PIO - Indirect read and write of the FE's registers.
 * 2 - DMA - Perform a DMA operation.
 * 3 - Slab - combine multiple DMAs into a single slab operation.
 *
 * The main interface from the rest of the software is through calls to
 * sbFe2000DmaRequest. The function to be performed by the DMA is specified
 * by the opcode parameter sbFe2000DmaOp_p_t->opcode. These are:
 *
 * SB_FE2000_DMA_OPCODE_READ        DMA read from FE
 * SB_FE2000_DMA_OPCODE_READ_CLEAR  DMA read from FE then clear
 * SB_FE2000_DMA_OPCODE_WRITE       DMA write to FE
 * SB_FE2000_DMA_OPCODE_CLEAR       DMA write clear to FE
 * SB_FE2000_DMA_OPCODE_SYNC        Synchronize driver and FE
 *
 * Calling a function to perform a DMA could result in either a PIO or a DMA
 * depending on the size of the data to be transfered. If the data size is less
 * then sbFe2000CommonConfigParams_p_t->macPio then the operation is performed
 * using direct register accesses. For larger data sizes a DMA operation is
 * used.
 *
 * To increase system efficiency the ability to pack multiple DMAs into a
 * single slab operation is provided. This significantly decreases the system
 * overhead. The calling functions is responsible for packing multiple
 * transfers into a slab data structure which uses the DMA primitives to
 * complete the transfer.
 *
 *-----------------------------------------------------------------------------*/

void
sbFe2000DmaDefaultParams(sbFe2000DmaConfig_t *params)
{
  SB_MEMSET(params, 0, sizeof(*params)); /* xxx todo allocate it */

  params->pciReadsPerEpoch = 20;  /* xxx todo -- calculate */
  params->maxPio = 0x80;          /* xxx todo -- tune */
  params->pioTimeout = 1024;      /* xxx todo -- arbitrary */
}

/*-----------------------------------------------------------------------------
 * Initialize the DMA module.
 *---------------------------------------------------------------------------*/
sbStatus_t
sbFe2000DmaInit(sbFe2000DmaMgr_t **pCtl,
                sbFe2000DmaConfig_t *params)
{
  sbFe2000DmaMgr_t *state;
  sbFe2000DmaSlab_t s[2];
  sbDmaMemoryHandle_t dmaHdl;
  sbStatus_t sts;
  void *vp;
  uint32_t rsize, base, offs;
  int i;
  uint32_t intmemsize;

  SB_ASSERT(pCtl);
  SB_ASSERT(params);

  if (!IS_POW2(params->ulRingEntries))
    return SB_BAD_ARGUMENT_ERR_CODE;

  sts = thin_malloc(0, SB_ALLOC_INTERNAL, sizeof(**pCtl), (void**)&vp, 0);
  if (sts != SB_OK)
    return sts;
  if (!vp)
    return SB_MALLOC_FAILED;

  *pCtl = state = vp;

  SB_MEMSET(state, 0, sizeof(*state));

  /* We assume uint32_ts are densely packed in slabs */
  SB_ASSERT(((uint8_t *) &s[0].data[1]) - ((uint8_t *) &s[0].data[0]) == sizeof(uint32_t));
  /* We assume slabs are densely packed in arrays */
  SB_ASSERT(((uint8_t *) &(s[1])) - ((uint8_t *) &(s[0])) == sizeof(sbFe2000DmaSlab_t));

  DQ_INIT(&state->waitingOps);
  DQ_INIT(&state->activeOps);

  /* create mutex */
  state->sync_sem = sal_sem_create("dma_sync_sem", sal_sem_BINARY, 1);
  if (!state->sync_sem){
    return SB_OTHER_ERR_CODE;
  }

  /* inherit configuration */
  state->pCfg = params;

  if (SAND_HAL_IS_FE2KXT(params->regSet)) {
    intmemsize = SB_FE2000XT_INT_MEM_SIZE;
  }else{
    intmemsize = SB_FE2000_INT_MEM_SIZE;
  }
  /* If we are using ring-based (scatter-gather) dma we need to allocate dma accessible
   * memory for the ring. The fe2000 requires that the ring be aligned on a multiple of
   * the ring-size. In the interests of portability, we need to allocate 2x the size to
   * allow us to position ourselves aligned. There are many cleaner (OS specific) ways
   * of doing this under different hosts (Linux/BSD/Unix/vxWorks/pSos/QNX). Unfortunately
   * they are not consistent. */
  if (params->bUseRing) {
    rsize = params->ulRingEntries * SB_FE2000_RING_ENTRY_SIZE_BYTES;

    sts = thin_malloc(params->clientData, SB_ALLOC_OTHER_DMA, 2 * rsize, (void**)&vp, &dmaHdl);
    if (sts != SB_OK)
      return sts;
    if (!vp)
      return SB_MALLOC_FAILED;

    SB_MEMSET(vp, 0, rsize);

    state->bUseRing = params->bUseRing;
    state->dmaHdl.handle = dmaHdl.handle;
    state->ulRingSize = rsize;
    state->pRingMem = vp;

    /* generate a mask of the ring size */
    state->ulRingMask = rsize - 1;

    base = ((uint32_t) state->pRingMem) & ~state->ulRingMask;
    offs = ((uint32_t) state->pRingMem) & state->ulRingMask;

    /* see if we need to align  */
    if (offs) {
      /* round up to next aligned location */
      state->pRingPtr = (uint32_t *) (base + rsize);
    } else {
      /* already aligned on boundary */
      state->pRingPtr = state->pRingMem;
    }

    /* start producer/consumer at bottom of ring */
    state->ulProducerOffs = 0;
    state->ulConsumerOffs = 0;

    /* define ring base and size */
    SAND_HAL_FE2000_WRITE(params->regSet,  PC_DMA_SG_RING_PTR, (uint32_t) state->pRingPtr);
    SAND_HAL_FE2000_WRITE(params->regSet,  PC_DMA_SG_RING_SIZE, rsize);
  }

  state->dmasActive = 0;
  state->regsProducer = 0;
  state->regsConsumer = 0;
  state->clientData = params->handle;
  state->eDmaMode = SB_FE2000_DMA_MODE_PIO_AND_DMA;

  /* dma registers, four channels providing transport to the memories */
  state->dmaRegs[0].devBaseAddr = params->regSet;
  state->dmaRegs[0].request = SAND_HAL_FE2000_REG_OFFSET(params->regSet, PC_DMA_CTRL0);
  state->dmaRegs[0].hostAddress = SAND_HAL_FE2000_REG_OFFSET(params->regSet, PC_DMA_PCI_ADDR0);
  state->dmaRegs[0].devAddress = SAND_HAL_FE2000_REG_OFFSET(params->regSet, PC_DMA_CLI_ADDR0);
  state->dmaRegs[0].wordCount = SAND_HAL_FE2000_REG_OFFSET(params->regSet, PC_DMA_COUNT0);

  state->dmaRegs[1].devBaseAddr = params->regSet;
  state->dmaRegs[1].request = SAND_HAL_FE2000_REG_OFFSET(params->regSet, PC_DMA_CTRL1);
  state->dmaRegs[1].hostAddress = SAND_HAL_FE2000_REG_OFFSET(params->regSet, PC_DMA_PCI_ADDR1);
  state->dmaRegs[1].devAddress = SAND_HAL_FE2000_REG_OFFSET(params->regSet, PC_DMA_CLI_ADDR1);
  state->dmaRegs[1].wordCount = SAND_HAL_FE2000_REG_OFFSET(params->regSet, PC_DMA_COUNT1);

  state->dmaRegs[2].devBaseAddr = params->regSet;
  state->dmaRegs[2].request = SAND_HAL_FE2000_REG_OFFSET(params->regSet, PC_DMA_CTRL2);
  state->dmaRegs[2].hostAddress = SAND_HAL_FE2000_REG_OFFSET(params->regSet, PC_DMA_PCI_ADDR2);
  state->dmaRegs[2].devAddress = SAND_HAL_FE2000_REG_OFFSET(params->regSet, PC_DMA_CLI_ADDR2);
  state->dmaRegs[2].wordCount = SAND_HAL_FE2000_REG_OFFSET(params->regSet, PC_DMA_COUNT2);

  state->dmaRegs[3].devBaseAddr = params->regSet;
  state->dmaRegs[3].request = SAND_HAL_FE2000_REG_OFFSET(params->regSet, PC_DMA_CTRL3);
  state->dmaRegs[3].hostAddress = SAND_HAL_FE2000_REG_OFFSET(params->regSet, PC_DMA_PCI_ADDR3);
  state->dmaRegs[3].devAddress = SAND_HAL_FE2000_REG_OFFSET(params->regSet, PC_DMA_CLI_ADDR3);
  state->dmaRegs[3].wordCount = SAND_HAL_FE2000_REG_OFFSET(params->regSet, PC_DMA_COUNT3);

  /* data registers for 64-bit wide are contiguous */
  SB_ASSERT((SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM0_INT0_MEM_ACC_DATA1) + sizeof(uint32_t))
	    == SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM0_INT0_MEM_ACC_DATA0));
  SB_ASSERT((SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM1_INT0_MEM_ACC_DATA1) + sizeof(uint32_t))
	    == SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM1_INT0_MEM_ACC_DATA0));

  /*
   * 01/02/07 the model doesn't support pio mode for rc.
   * yet. apart from these we need to figure out the dma
   * layout vs indirect memory layout. observe that one is
   * 64 words and the other is 36 words.
   */
  state->pioRegs[SB_FE2000_MEM_CLS_0].devBaseAddr = params->regSet;
  state->pioRegs[SB_FE2000_MEM_CLS_0].request     = SAND_HAL_FE2000_REG_OFFSET(params->regSet, RC0_MEM_ACC_CTRL);
  state->pioRegs[SB_FE2000_MEM_CLS_0].data        = SAND_HAL_FE2000_REG_OFFSET(params->regSet, RC0_MEM_ACC_DATA0);
  state->pioRegs[SB_FE2000_MEM_CLS_0].words       = 64; /* 36 words data, 28 padding */
  state->pioRegs[SB_FE2000_MEM_CLS_0].addressMask = params->ulClsMask;
  state->pioRegs[SB_FE2000_MEM_CLS_0].bReverseLayout = FALSE;

  state->pioRegs[SB_FE2000_MEM_CLS_1].devBaseAddr = params->regSet;
  state->pioRegs[SB_FE2000_MEM_CLS_1].request     = SAND_HAL_FE2000_REG_OFFSET(params->regSet, RC1_MEM_ACC_CTRL);
  state->pioRegs[SB_FE2000_MEM_CLS_1].data        = SAND_HAL_FE2000_REG_OFFSET(params->regSet, RC1_MEM_ACC_DATA0);
  state->pioRegs[SB_FE2000_MEM_CLS_1].words       = 64; /* 36 words data, 28 padding */
  state->pioRegs[SB_FE2000_MEM_CLS_1].addressMask = params->ulClsMask;
  state->pioRegs[SB_FE2000_MEM_CLS_1].bReverseLayout = FALSE;

  /* MMU0 pio registers, one per physical memory bank */
  state->pioRegs[SB_FE2000_MEM_MM0_NARROW_0].devBaseAddr = params->regSet;
  state->pioRegs[SB_FE2000_MEM_MM0_NARROW_0].request = SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM0_NARROW_PORT0_MEM_ACC_CTRL);
  state->pioRegs[SB_FE2000_MEM_MM0_NARROW_0].data = SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM0_NARROW_PORT0_MEM_ACC_DATA0);
  state->pioRegs[SB_FE2000_MEM_MM0_NARROW_0].words = 1;
  state->pioRegs[SB_FE2000_MEM_MM0_NARROW_0].addressMask = params->ulMM0MaskNarrow0;
  /* for the dma srcs for which words = 1 bReverseLayout doesn't really matter */
  state->pioRegs[SB_FE2000_MEM_MM0_NARROW_0].bReverseLayout = TRUE;

  state->pioRegs[SB_FE2000_MEM_MM0_NARROW_1].devBaseAddr = params->regSet;
  state->pioRegs[SB_FE2000_MEM_MM0_NARROW_1].request = SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM0_NARROW_PORT1_MEM_ACC_CTRL);
  state->pioRegs[SB_FE2000_MEM_MM0_NARROW_1].data = SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM0_NARROW_PORT1_MEM_ACC_DATA0);
  state->pioRegs[SB_FE2000_MEM_MM0_NARROW_1].words = 1;
  state->pioRegs[SB_FE2000_MEM_MM0_NARROW_1].addressMask = params->ulMM0MaskNarrow1;
  state->pioRegs[SB_FE2000_MEM_MM0_NARROW_1].bReverseLayout = TRUE;

  state->pioRegs[SB_FE2000_MEM_MM0_WIDE].devBaseAddr = params->regSet;
  state->pioRegs[SB_FE2000_MEM_MM0_WIDE].request = SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM0_WIDE_PORT_MEM_ACC_CTRL);
  state->pioRegs[SB_FE2000_MEM_MM0_WIDE].data = SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM0_WIDE_PORT_MEM_ACC_DATA1);
  state->pioRegs[SB_FE2000_MEM_MM0_WIDE].words = 2;
  state->pioRegs[SB_FE2000_MEM_MM0_WIDE].addressMask = params->ulMM0MaskWide;
  state->pioRegs[SB_FE2000_MEM_MM0_WIDE].bReverseLayout = TRUE;

  state->pioRegs[SB_FE2000_MEM_MM0_INTERNAL_0].devBaseAddr = params->regSet;
  state->pioRegs[SB_FE2000_MEM_MM0_INTERNAL_0].request =
    SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM0_INT0_MEM_ACC_CTRL);
  if (params->mm0i0wide) {
    state->pioRegs[SB_FE2000_MEM_MM0_INTERNAL_0].data =
      SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM0_INT0_MEM_ACC_DATA1);
    state->pioRegs[SB_FE2000_MEM_MM0_INTERNAL_0].words = 2;
    state->pioRegs[SB_FE2000_MEM_MM0_INTERNAL_0].addressMask =
      (intmemsize/ 2) - 1;
  } else {
    state->pioRegs[SB_FE2000_MEM_MM0_INTERNAL_0].data =
      SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM0_INT0_MEM_ACC_DATA0);
    state->pioRegs[SB_FE2000_MEM_MM0_INTERNAL_0].words = 1;
    state->pioRegs[SB_FE2000_MEM_MM0_INTERNAL_0].addressMask =
      intmemsize - 1;
  }
  state->pioRegs[SB_FE2000_MEM_MM0_INTERNAL_0].bReverseLayout = TRUE;

  state->pioRegs[SB_FE2000_MEM_MM0_INTERNAL_1].devBaseAddr = params->regSet;
  state->pioRegs[SB_FE2000_MEM_MM0_INTERNAL_1].request =
    SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM0_INT1_MEM_ACC_CTRL);
  if (params->mm0i1wide) {
    state->pioRegs[SB_FE2000_MEM_MM0_INTERNAL_1].data =
      SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM0_INT1_MEM_ACC_DATA1);
    state->pioRegs[SB_FE2000_MEM_MM0_INTERNAL_1].words = 2;
    state->pioRegs[SB_FE2000_MEM_MM0_INTERNAL_1].addressMask =
      (intmemsize / 2) - 1;
  } else {
    state->pioRegs[SB_FE2000_MEM_MM0_INTERNAL_1].data =
      SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM0_INT1_MEM_ACC_DATA0);
    state->pioRegs[SB_FE2000_MEM_MM0_INTERNAL_1].words = 1;
    state->pioRegs[SB_FE2000_MEM_MM0_INTERNAL_1].addressMask =
      intmemsize - 1;
  }
  state->pioRegs[SB_FE2000_MEM_MM0_INTERNAL_1].bReverseLayout = TRUE;

  /* MMU1 pio registers, one per physical memory bank */
  state->pioRegs[SB_FE2000_MEM_MM1_NARROW_0].devBaseAddr = params->regSet;
  state->pioRegs[SB_FE2000_MEM_MM1_NARROW_0].request = SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM1_NARROW_PORT0_MEM_ACC_CTRL);
  state->pioRegs[SB_FE2000_MEM_MM1_NARROW_0].data = SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM1_NARROW_PORT0_MEM_ACC_DATA0);
  state->pioRegs[SB_FE2000_MEM_MM1_NARROW_0].words = 1;
  state->pioRegs[SB_FE2000_MEM_MM1_NARROW_0].addressMask = params->ulMM1MaskNarrow0;
  state->pioRegs[SB_FE2000_MEM_MM1_NARROW_0].bReverseLayout = TRUE;

  state->pioRegs[SB_FE2000_MEM_MM1_NARROW_1].devBaseAddr = params->regSet;
  state->pioRegs[SB_FE2000_MEM_MM1_NARROW_1].request = SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM1_NARROW_PORT1_MEM_ACC_CTRL);
  state->pioRegs[SB_FE2000_MEM_MM1_NARROW_1].data = SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM1_NARROW_PORT1_MEM_ACC_DATA0);
  state->pioRegs[SB_FE2000_MEM_MM1_NARROW_1].words = 1;
  state->pioRegs[SB_FE2000_MEM_MM1_NARROW_1].addressMask = params->ulMM1MaskNarrow1;
  state->pioRegs[SB_FE2000_MEM_MM1_NARROW_1].bReverseLayout = TRUE;

  state->pioRegs[SB_FE2000_MEM_MM1_WIDE].devBaseAddr = params->regSet;
  state->pioRegs[SB_FE2000_MEM_MM1_WIDE].request = SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM1_WIDE_PORT_MEM_ACC_CTRL);
  state->pioRegs[SB_FE2000_MEM_MM1_WIDE].data = SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM1_WIDE_PORT_MEM_ACC_DATA1);
  state->pioRegs[SB_FE2000_MEM_MM1_WIDE].words = 2;
  state->pioRegs[SB_FE2000_MEM_MM1_WIDE].addressMask = params->ulMM1MaskWide;
  state->pioRegs[SB_FE2000_MEM_MM1_WIDE].bReverseLayout = TRUE;

  state->pioRegs[SB_FE2000_MEM_MM1_INTERNAL_0].devBaseAddr = params->regSet;
  state->pioRegs[SB_FE2000_MEM_MM1_INTERNAL_0].request = SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM1_INT0_MEM_ACC_CTRL);
  if (params->mm1i0wide) {
    state->pioRegs[SB_FE2000_MEM_MM1_INTERNAL_0].data =
      SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM1_INT0_MEM_ACC_DATA1);
    state->pioRegs[SB_FE2000_MEM_MM1_INTERNAL_0].words = 2;
    state->pioRegs[SB_FE2000_MEM_MM1_INTERNAL_0].addressMask =
      (intmemsize / 2) - 1;
  } else {
    state->pioRegs[SB_FE2000_MEM_MM1_INTERNAL_0].data =
      SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM1_INT0_MEM_ACC_DATA0);
    state->pioRegs[SB_FE2000_MEM_MM1_INTERNAL_0].words = 1;
    state->pioRegs[SB_FE2000_MEM_MM1_INTERNAL_0].addressMask =
      intmemsize - 1;
  }
  state->pioRegs[SB_FE2000_MEM_MM1_INTERNAL_0].bReverseLayout = TRUE;

  state->pioRegs[SB_FE2000_MEM_MM1_INTERNAL_1].devBaseAddr = params->regSet;
  state->pioRegs[SB_FE2000_MEM_MM1_INTERNAL_1].request =
    SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM1_INT1_MEM_ACC_CTRL);
  if (params->mm1i1wide) {
    state->pioRegs[SB_FE2000_MEM_MM1_INTERNAL_1].data =
      SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM1_INT1_MEM_ACC_DATA1);
    state->pioRegs[SB_FE2000_MEM_MM1_INTERNAL_1].words = 2;
    state->pioRegs[SB_FE2000_MEM_MM1_INTERNAL_1].addressMask =
      (intmemsize / 2) - 1;
  } else {
    state->pioRegs[SB_FE2000_MEM_MM1_INTERNAL_1].data =
      SAND_HAL_FE2000_REG_OFFSET(params->regSet, MM1_INT1_MEM_ACC_DATA0);
    state->pioRegs[SB_FE2000_MEM_MM1_INTERNAL_1].words = 1;
    state->pioRegs[SB_FE2000_MEM_MM1_INTERNAL_1].addressMask =
      intmemsize - 1;
  }
  state->pioRegs[SB_FE2000_MEM_MM1_INTERNAL_1].bReverseLayout = TRUE;

  DQ_INIT(&(state->slabsWaitingStart));
  DQ_INIT(&(state->slabsWaitingLast));

  state->slabCurrent = NULL;
  state->slabActiveDmas = 0;
  state->slabDmaProducer = 0;

  for (i = 0; i < 2; i++) {
    state->slabDmas[i].data = state;
    state->slabDmas[i].cb = sbFe2000DmaSlabDmaDone;
  }

  for (i = 0; i < SB_FE2000_MEM_MAX_SRCS; i++) {
    state->writePosted[i] = 0;
  }

  return SB_OK;
}


typedef enum {
  _dmaOpDma,
  _dmaOpPio
} _fe2kDmaOp_t;

static sbStatus_t
sbFe2000DmaOperationActionGet(sbFe2000DmaMgr_t *pDmaMgr,
                              sbFe2000DmaOp_t *op,
                              _fe2kDmaOp_t *action) {


  if (op->bForcePio) {
    *action = _dmaOpPio;
    return SB_OK;
  }
  
  /* For CALADAN2 if the request is for internal memory and > 16K, force DMA
  */
  if (SAND_HAL_IS_FE2KXT(pDmaMgr->pCfg->regSet)) {
    if ( ((SB_FE2000_DMA_ADDRESS_BANK(op->feAddress) == SB_FE2000_MEM_MM0_INTERNAL_0) ||
         (SB_FE2000_DMA_ADDRESS_BANK(op->feAddress) == SB_FE2000_MEM_MM0_INTERNAL_1) ||
         (SB_FE2000_DMA_ADDRESS_BANK(op->feAddress) == SB_FE2000_MEM_MM1_INTERNAL_0) ||
         (SB_FE2000_DMA_ADDRESS_BANK(op->feAddress) == SB_FE2000_MEM_MM1_INTERNAL_1)) &&
         (SB_FE2000_DMA_ADDRESS_OFFSET(op->feAddress) > 16383) )
    {
      *action = _dmaOpDma;
      return SB_OK;
    }
  }

  switch (pDmaMgr->eDmaMode) {
  case SB_FE2000_DMA_MODE_PIO_ONLY: *action = _dmaOpPio; break;
  case SB_FE2000_DMA_MODE_DMA_ONLY: *action = _dmaOpDma; break;
  case SB_FE2000_DMA_MODE_PIO_AND_DMA:
  default:
    if (op->words <= pDmaMgr->pCfg->maxPio) {
      *action = _dmaOpPio;
    } else {
      *action = _dmaOpDma;
    }
    break;
  }

  return SB_OK;
}


static void
sbFe2000DmaRequestCallback(sbFe2000DmaOp_t *op)
{
    int unit = (int) op->data;
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);

    sal_sem_give(sbx->dma_sem);
}


/*
 * sbFe2000DmaRequest & sbFe2000DmaSlabRequest are picked up from
 * here by linking
 */
sbStatus_t
sbFe2000DmaRequest(sbFe2000DmaMgr_t *pDmaMgr, void *chipUserToken,
                   sbFe2000DmaOp_t *op)
{
    int unit = (int) chipUserToken;
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    _fe2kDmaOp_t action;
    void *data0 = op->data;
    sbStatus_t status;
    
    op->data = (void *) unit;
    op->cb = sbFe2000DmaRequestCallback;
   
    /* reserve the sync_sem if and only if this operation will cause an actual
     * DMA.  PIO actions to not need to be synchronized because they occur
     * immediately and will never return in-progress.  Furthermore, with the
     * sync_sem, only one DMA will ever be in progress any any given moment
     * and therefore a PIO action will never be queued to be handled by the
     * exception process.
     */
    sbFe2000DmaOperationActionGet(pDmaMgr, op, &action);
    if (action == _dmaOpDma) {
      SOC_DEBUG_PRINT((DK_DMA | DK_VERBOSE, 
                       _sbFe200DmaMgr_D("DMA request.  FeAddr=0x%08x words=%d "
                          "forcePio= %d opcode=%d\n"),
                       op->feAddress, op->words, op->bForcePio, op->opcode));

      sal_sem_take(pDmaMgr->sync_sem, sal_sem_FOREVER);
    }

    status = sbFe2000DmaRequestI(pDmaMgr, chipUserToken, op);

    if (status == SB_IN_PROGRESS) {     
      sal_sem_take(sbx->dma_sem, sal_sem_FOREVER);
      status = op->status;
    }

    if (action == _dmaOpDma) {
      sal_sem_give(pDmaMgr->sync_sem);
    }
    
    op->data = data0;

    return status;
}

/*-----------------------------------------------------------------------------
 * sbFe2000DmaRequest is the interface used by external functions to initiate
 * a DMA operation. It is also used by the slab interface to perform its DMA.
 *
 * The DMA srguments are checked for validity and an error is returned if any
 * arguments are out of range. Next a user supplied isrSync function is caled
 * to prevent asynchronous calls to the ILib.
 *
 * The DMA state is checked and if the DMA engine is capable of starting a
 * transfer the operation is kicked off with a call to sbFe2000DmaStart. If
 * the DMA engine is busy then the current transfer is added to the waitingOps
 * queue for later processing.
 *
 * returns: SB_OK - transfer completed immediately using indirect access
 *          SB_IN_PROGRESS - DMA kicked off OK or DMA busy and request
 *                                 added to queue.
 *          error
 *---------------------------------------------------------------------------*/
sbStatus_t
sbFe2000DmaRequestI(sbFe2000DmaMgr_t *state, void *chipUserToken,
                    sbFe2000DmaOp_t *op)
{
  sbStatus_t status;
  sbSyncToken_t sync;

#ifdef BCM_WARM_BOOT_SUPPORT  
  int wb = SOC_WARM_BOOT((int)state->clientData);
#endif

#ifdef BCM_WARM_BOOT_SUPPORT  
  if (wb) {
    if ((op->opcode == SB_FE2000_DMA_OPCODE_READ) ||
        (op->opcode == SB_FE2000_DMA_OPCODE_SYNC) ||
        (op->opcode == SB_FE2000_DMA_OPCODE_READ_WORDS) ||
        (op->opcode == SB_FE2000_DMA_OPCODE_END))
    {
      SOC_WARM_BOOT_DONE((int)state->clientData);
    } else {
      return SB_OK;
    }
  }
#endif /* BCM_WARM_BOOT_SUPPORT */

  status = sbFe2000DmaCheckOpArguments(state, op);
  if (status == SB_OK) {
    sync = (state->pCfg->isrSync)(state->clientData);

    if (sbFe2000DmaIsStartable(state, op)) {
      status = sbFe2000DmaStart(state, op);
    } else {
      SOC_DEBUG_PRINT((DK_DMA, _sbFe200DmaMgr_D("Queue DMA request to waiting ops.  "
                                  "FeAddr=0x%08x opcode=%d\n"),
                       op->feAddress, op->opcode));

      DQ_INSERT_TAIL(&(state->waitingOps), op);
      status = SB_IN_PROGRESS;
    }

    (state->pCfg->isrUnsync)(state->clientData, (int)sync);
  }

#ifdef BCM_WARM_BOOT_SUPPORT  
  if (wb) {
    SOC_WARM_BOOT_START((int)state->clientData);
  }
#endif /* BCM_WARM_BOOT_SUPPORT */

  return status;
}

/*-----------------------------------------------------------------------------
 * Check the DMA engine to ensure a new DMA operation can be instigated.
 *---------------------------------------------------------------------------*/
static uint8_t
sbFe2000DmaIsStartable(sbFe2000DmaMgr_t *state, sbFe2000DmaOp_t *op)
{
  uint32_t ulMax = state->bUseRing ? state->ulRingEntries : SB_FE2000_DMA_CHANNELS;

  return ((state->dmasActive < ulMax - 1) ||
          ((state->dmasActive == ulMax - 1) &&
           (op->opcode != SB_FE2000_DMA_OPCODE_SYNC)));
}

/*-----------------------------------------------------------------------------
 * Perform a sanity check on the DMA argument.
 *
 * returns: SB_OK - args OK
 *          SB_BAD_ARGUMENT_ERR_CODE
 *---------------------------------------------------------------------------*/
static sbStatus_t
sbFe2000DmaCheckOpArguments(sbFe2000DmaMgr_t *state, sbFe2000DmaOp_t *op)
{
  sbStatus_t status = SB_OK;
  uint32_t bank;
  uint32_t bankWords;
  uint32_t offset;

  /* TODO: range check external SRAMs against sizes in fe->params */
  if (op->opcode == SB_FE2000_DMA_OPCODE_READ
      || op->opcode == SB_FE2000_DMA_OPCODE_READ_CLEAR
      || op->opcode == SB_FE2000_DMA_OPCODE_WRITE
      || op->opcode == SB_FE2000_DMA_OPCODE_CLEAR) {
    if (op->opcode != SB_FE2000_DMA_OPCODE_CLEAR
        && (((uint32_t) op->hostAddress) & 0x3)) {
      status = SB_BAD_ARGUMENT_ERR_CODE;
    }
    bank = SB_FE2000_DMA_ADDRESS_BANK(op->feAddress);
    assert( bank <= 0x1f );
    if (bank >= SB_FE2000_MEM_MAX_SRCS) {
      status = SB_BAD_ARGUMENT_ERR_CODE;
      bank = 0;
    }
    if (op->words == 0) {
      status = SB_BAD_ARGUMENT_ERR_CODE;
    }
    bankWords = state->pioRegs[bank].words;
    offset = SB_FE2000_DMA_ADDRESS_OFFSET(op->feAddress);
#if 0 
    if (op->words % bankWords != 0) {
      status = SB_BAD_ARGUMENT_ERR_CODE;
    }
#endif
    if (offset + op->words / bankWords - 1 >
        state->pioRegs[bank].addressMask) {
      status = SB_BAD_ARGUMENT_ERR_CODE;
    }
  } else if (op->opcode != SB_FE2000_DMA_OPCODE_SYNC) {
    status = SB_BAD_ARGUMENT_ERR_CODE;
  }
  return status;
}

/*-----------------------------------------------------------------------------
 * Start the DMA operation
 * Assumes that the arguments are valid and the DMA engine is idle.
 *
 * The SB_FE2000_DMA_OPCODE_SYNC opcode is used to synchronize the driver to
 * the FE. When the driver writes data to the FE, the FE will not use this data
 * until the end of the current epoch. There is no hardware mechanism for the
 * driver to determine the epoch boundaries. The OPCODE_SYNC simply delays for
 * enough time to ensure the current epoch completes and the FE is using the
 * lastest data written by the driver.
 *
 * The size of the operation is checked and if it is <= maxPIO the transfer is
 * handled with a call to the read/write indirect functions. These use register
 * accesses to perform the data transfer.
 *
 * If a DMA is to proceed then first check that the bank to be accessed is
 * idle. This involves waiting for any indirect accesses to that bank to
 * complete.
 *
 * If the parameter sbFe2000DmaOp_p_t->host is set to
 * SB_FE2000_DMA_HBA_COMPUTE then this function is responsible for
 * calculating the host bus address. This is performed with a call to a user
 * supplied function state->pCfg->pCfg->getHba.
 *
 * If the DMA is a write, then the user supplied state->pCfg->writeSync function
 * is called to ensure the host's memory is consistent.
 *
 * Finally the DMA is performed through writes to the FE's registers.
 *
 * returns: SB_IN_PROGRESS - DMA kicked off sucessfully
 *          SB_OK - data transfer completed
 *          error
 *---------------------------------------------------------------------------*/

uint32_t g_bSbG2FeDmaStartDump = 0;

static sbStatus_t
sbFe2000DmaStart(sbFe2000DmaMgr_t *state, sbFe2000DmaOp_t *op)
{
  sbFe2000DmaOpcode_e_t opcode = op->opcode;
  sbFe2000DmaChanRegs_t *regs = &state->dmaRegs[state->regsProducer];
  uint8_t write = opcode == SB_FE2000_DMA_OPCODE_WRITE;
  uint8_t clear = opcode == SB_FE2000_DMA_OPCODE_CLEAR;
  uint8_t clearOnRead = opcode == SB_FE2000_DMA_OPCODE_READ_CLEAR;
  sbStatus_t status = SB_OK;
  uint32_t bank, reg, offs, wide, bankWords, i, pio, w2, w3, immediate, ctrlReg, *pEntry;
  uint32_t indicesPerBlock;
  _fe2kDmaOp_t action;

  sbFe2000DmaOperationActionGet(state, op, &action);
  pio = (action == _dmaOpPio);

  if (g_bSbG2FeDmaStartDump)
    SB_LOG("DmaStart: %s, opcode: 0x%x, %d words, hba: 0x%x, fea: 0x%x\n",
           pio ? "pio" : "dma", opcode, op->words,
           (unsigned int) op->hostAddress, (unsigned int) op->feAddress);

  if (opcode == SB_FE2000_DMA_OPCODE_SYNC) {
    for (i = 0; i < state->pCfg->pciReadsPerEpoch; i++) {
      /* Just read any old register, for timing purposes */
      SAND_HAL_FE2000_READ_OFFS(state->dmaRegs[0].devBaseAddr, state->dmaRegs[0].request);
    }
  } else {
    SB_ASSERT(opcode == SB_FE2000_DMA_OPCODE_READ
              || opcode == SB_FE2000_DMA_OPCODE_READ_CLEAR
              || opcode == SB_FE2000_DMA_OPCODE_WRITE
              || opcode == SB_FE2000_DMA_OPCODE_CLEAR);
    if (pio) {
      if (!write && !clear) {
        if ((status = sbFe2000DmaWaitBankIdle(state, op->feAddress))
            != SB_OK) {
          return status;
        }
      }

      bank = SB_FE2000_DMA_ADDRESS_BANK(op->feAddress);
      offs = SB_FE2000_DMA_ADDRESS_OFFSET(op->feAddress);
      indicesPerBlock = 1;
      if ((bank == SB_FE2000_MEM_CLS_0) || (bank == SB_FE2000_MEM_CLS_1)) {
        /* For classifier mem, passed in offset is in words. For indirect
           memory access, the offset is instruction offset. And there are 4
           instructions per 64B dma word chunk...so divide by 16 */
        offs /= 16;
        indicesPerBlock = 4;
      }
      bankWords = state->pioRegs[bank].words;
      for (i = 0; i < op->words / bankWords; i++) {
        if (write || clear) {
          uint32_t *data = clear ? NULL : &op->hostAddress[i * bankWords];
          status =
            sbFe2000DmaWriteIndirect(state, 
                      SB_FE2000_DMA_MAKE_ADDRESS(bank, offs)+indicesPerBlock*i,
                      data);
        } else {
          status =
            sbFe2000DmaReadIndirect(state, 
                      SB_FE2000_DMA_MAKE_ADDRESS(bank, offs)+indicesPerBlock*i,
                      &op->hostAddress[i * bankWords], clearOnRead);
        }
        if (status != SB_OK) {
          break;
        }
      }
    } else {
      status = sbFe2000DmaWaitBankIdle(state, op->feAddress);
      if (status != SB_OK) {
        return status;
      }
      if (op->hostBusAddress == SB_FE2000_DMA_HBA_COMPUTE) {
        status =
          (state->pCfg->getHba)(op->dmaHandle, op->hostAddress, op->words,
                               &op->hostBusAddress);
        if (status != SB_OK) {
          return status;
        }
      }
      if (write) {
        /* TODO: client could do a single sync for a group of writes  */
        status =
          (state->pCfg->writeSync)(op->dmaHandle, op->hostAddress, op->words);
        if (status != SB_OK) {
          return status;
        }
      }
      DQ_INSERT_TAIL(&state->activeOps, op);

      bank = SB_FE2000_DMA_ADDRESS_BANK(op->feAddress);
      offs = SB_FE2000_DMA_ADDRESS_OFFSET(op->feAddress);
      wide = sbFe2000DmaIsWideBank(bank);

      if (!state->bUseRing) { /* initiate thread-based dma */

        reg = (SAND_HAL_FE2000_SET_FIELD(regs->devBaseAddr,  PC_DMA_CTRL0, ACK, 1) |
               SAND_HAL_FE2000_SET_FIELD(regs->devBaseAddr,  PC_DMA_CTRL0, REQ, 1) |
               SAND_HAL_FE2000_SET_FIELD(regs->devBaseAddr,  PC_DMA_CTRL0, RD_WR_N, !write) |
               SAND_HAL_FE2000_SET_FIELD(regs->devBaseAddr,  PC_DMA_CTRL0, COR, clearOnRead) |
               SAND_HAL_FE2000_SET_FIELD(regs->devBaseAddr,  PC_DMA_CTRL0, DMA_SRC, bank) |
               SAND_HAL_FE2000_SET_FIELD(regs->devBaseAddr,  PC_DMA_CTRL0, DMA_SRC_TYPE, wide));

        SAND_HAL_FE2000_WRITE_OFFS(regs->devBaseAddr, regs->hostAddress, op->hostBusAddress);
        SAND_HAL_FE2000_WRITE_OFFS(regs->devBaseAddr, regs->devAddress, offs);
        SAND_HAL_FE2000_WRITE_OFFS(regs->devBaseAddr, regs->wordCount, op->words);
        SAND_HAL_FE2000_WRITE_OFFS(regs->devBaseAddr, regs->request, reg);

        state->regsProducer = (state->regsProducer + 1) % SB_FE2000_DMA_CHANNELS;
        state->dmasActive++;
        status = SB_IN_PROGRESS;

      } else { /* initiate ring-based dma */

#if 0 /* xxx hack disable immediate mode. not supported in model */
        /* is this immediate mode entry ? */
        immediate = (op->words <= 2) ? 1 : 0;
#else
        immediate = 0;
#endif

        /* compose shared part of control word in entry */
        ctrlReg = (SET_RFLD(ST, 0) |
                   SET_RFLD(DMA_SRC, bank) |
                   SET_RFLD(INT, 1) |
                   SET_RFLD(IM, immediate) |
                   SET_RFLD(OWNR, 1));

        if (immediate) {
          /* we encode first two words of dma in entry */
          w2 = *(((uint32_t *) op->hostBusAddress) + 0);
          w3 = *(((uint32_t *) op->hostBusAddress) + 1);
          ctrlReg |= SET_RFLD(NWORDS, op->words);
        } else {
          /* we point to the memory and specify num words */
          w2 = op->hostBusAddress;
          w3 = op->words;
          ctrlReg |= SET_RFLD(COR, clearOnRead);
          ctrlReg |= SET_RFLD(RWN, !write);
        }

        /* fill in the ring entry, write control word last */
        pEntry = state->pRingPtr + state->ulProducerOffs / sizeof(uint32_t);

        *(pEntry + 3) = SAND_SWAP_32(w3);
        *(pEntry + 2) = SAND_SWAP_32(w2);
        *(pEntry + 1) = SAND_SWAP_32(offs);
        *(pEntry + 0) = SAND_SWAP_32(ctrlReg);

        /* increment ring pointer modulo ring-size */
        /* write to hardware to notify of producer update */
        state->ulProducerOffs = sbFe2000DmaRingNext(state, state->ulProducerOffs);
        SAND_HAL_FE2000_WRITE(regs->devBaseAddr,  PC_DMA_SG_RING_PRODUCER, state->ulProducerOffs);
      }
    }
  }
  return status;
}

/*-----------------------------------------------------------------------------
 * If the last access to the memory bank containing the feAddress was indirect
 * (via PIO registers) then we must wait for it to complete before issueing
 * another indirect read or write. This function tests for the type of last
 * access to the bank (in state->writePosted[bank]) and calls
 * sbFe2000DmaWaitIndirect if there is a possible wait condition.
 *
 * returns SB_OK - current operation to specified bank is complete
 *         SB_FE_MEM_ACC_TIMEOUT_ERR_CODE - timeout
 *---------------------------------------------------------------------------*/
static sbStatus_t
sbFe2000DmaWaitBankIdle(sbFe2000DmaMgr_t *state, uint32_t feAddress)
{
  sbStatus_t status;
  uint32_t bank;

  bank = SB_FE2000_DMA_ADDRESS_BANK(feAddress);

  if (!state->writePosted[bank]) {
    return SB_OK;
  }
  status = sbFe2000DmaWaitIndirect(state->pCfg->pioTimeout,
                                    &state->pioRegs[bank]);
  state->writePosted[bank] = 0;
  return status;
}

/*-----------------------------------------------------------------------------
 * If the last access to the memory bank containing the feAddress was indirect
 * (via PIO registers) then we must wait for it to complete before issueing
 * another indirect read or write. This function polls the FE register waiting
 * for the current PIO operation to complete.
 *
 * returns SB_OK - current operation to specified bank is complete
 *         SB_FE_MEM_ACC_TIMEOUT_ERR_CODE - timeout
 *
 *---------------------------------------------------------------------------*/
static sbStatus_t
sbFe2000DmaWaitIndirect(uint32_t timeout, sbFe2000DmaPioRegs_t *bank)
{
  uint32_t reg;
  int i;

  for (i = 0; i < timeout; i++) {
    reg = SAND_HAL_FE2000_READ_OFFS(bank->devBaseAddr, bank->request);
    if (reg & SAND_HAL_FE2000_FIELD_MASK(bank->devBaseAddr, MM_INT0, MEM_ACC_CTRL_ACK)) {
      reg &= ~(SAND_HAL_FE2000_FIELD_MASK(bank->devBaseAddr, MM_INT0, MEM_ACC_CTRL_REQ));
      reg |= SAND_HAL_FE2000_FIELD_MASK(bank->devBaseAddr, MM_INT0, MEM_ACC_CTRL_ACK);
      SAND_HAL_FE2000_WRITE_OFFS(bank->devBaseAddr, bank->request, reg);
      return SB_OK;
    }
  }
  return SB_FE_MEM_ACC_TIMEOUT_ERR_CODE;
}

/*-----------------------------------------------------------------------------
 * Perform an indirect write to the FE. This involves writing directly to the
 * FE's registers and not using the DMA functions.
 *
 * First check that there all previous accesses to this bank are complete and
 * then loop through each data word writing it to the FE through the FE
 * registers.
 *
 * returns SB_OK - current operation to specified bank is complete
 *         SB_FE_MEM_ACC_TIMEOUT_ERR_CODE - timeout
 *
 *---------------------------------------------------------------------------*/
sbStatus_t
sbFe2000DmaWriteIndirect(sbFe2000DmaMgr_t *state, sbFe2000DmaAddr_t feAddress,
                         uint32_t *data)
{
  sbFe2000DmaPioRegs_t *pRegs;
  sbStatus_t status;
  uint32_t i, j, bank, offs, reg, bankWords, clsHack;

  if ((status = sbFe2000DmaWaitBankIdle(state, feAddress)) != SB_OK) {
    return status;
  }

  bank = SB_FE2000_DMA_ADDRESS_BANK(feAddress);
  offs = SB_FE2000_DMA_ADDRESS_OFFSET(feAddress);

  /* xxx hack -- Deal with the way the classifier dmas are presented to
   * xxx hack -- pio subsystem. There are 64 words in the dma chunk.
   * xxx hack -- Words 0->3 and 32-63 need to be packed into the 36 word
   * xxx hack -- data registers for the pio transaction. Words 4->31
   * xxx hack -- (28 words) are padding. The pio data registers are
   * xxx hack -- ajacent. We need to transfer the dma data into data regs */
  if ((bank == SB_FE2000_MEM_CLS_0) || (bank == SB_FE2000_MEM_CLS_1))
    clsHack = 1;
  else
    clsHack = 0;

  pRegs = &state->pioRegs[bank];
  bankWords = pRegs->words;

  for (i = 0; i < bankWords; i++) {
    pRegs->bReverseLayout ? (j = bankWords - i - 1) : (j = i);
    if (clsHack) {
      if ((i > 3) && (i < 32))
        continue;   /* skip padding hole */
      if (i >= 32)
        j = j - 28; /* adjust data reg for padding */
    }
    SAND_HAL_FE2000_WRITE_OFFS_RAW(pRegs->devBaseAddr,
                            pRegs->data + j * sizeof(uint32_t),
                            data ? data[i] : 0);
  }

  /* compose request using physical addressing */
  reg = (SAND_HAL_FE2000_SET_FIELD(pRegs->devBaseAddr, MM_INT0_MEM_ACC_CTRL, ACK, 1) |
         SAND_HAL_FE2000_SET_FIELD(pRegs->devBaseAddr, MM_INT0_MEM_ACC_CTRL, REQ, 1) |
         SAND_HAL_FE2000_SET_FIELD(pRegs->devBaseAddr, MM_INT0_MEM_ACC_CTRL, RD_WR_N, 0) |
         SAND_HAL_FE2000_SET_FIELD(pRegs->devBaseAddr, MM_NARROW_PORT0_MEM_ACC_CTRL, ADDRESS, offs));

  SAND_HAL_FE2000_WRITE_OFFS(pRegs->devBaseAddr, pRegs->request, reg);

  state->writePosted[bank] = 1;

  return SB_OK;
}

sbStatus_t
sbFe2000DmaWriteIndirectMult(sbFe2000DmaMgr_t *state, sbFe2000DmaAddr_t feAddress,
                             uint32_t *data, uint32_t words)
{
  uint32_t i, bank, offs, *pdata, bwords, addr;
  sbStatus_t sts;

  /* parse start of transaction */
  bank = SB_FE2000_DMA_ADDRESS_BANK(feAddress);
  offs = SB_FE2000_DMA_ADDRESS_OFFSET(feAddress);

  /* uint32_t words per memory address */
  bwords = state->pioRegs[bank].words;

  for (i = 0; i < words / bwords; i++) {
    addr = SB_FE2000_DMA_MAKE_ADDRESS(bank, offs + i);
    pdata = data + i * bwords;
    sts = sbFe2000DmaWriteIndirect(state, addr, pdata);
    if (sts != SB_OK)
      return sts;
  }

  return SB_OK;
}

/*-----------------------------------------------------------------------------
 * Perform an indirect read to the FE. This involves reading directly from the
 * FE's registers and not using the DMA functions.
 *
 * First check that there all previous accesses to this bank are complete and
 * then loop through each data word writing it to the FE through the FE
 * registers.
 *
 * returns SB_OK - current operation to specified bank is complete
 *         SB_FE_MEM_ACC_TIMEOUT_ERR_CODE - timeout
 *
 *---------------------------------------------------------------------------*/
sbStatus_t
sbFe2000DmaReadIndirect(sbFe2000DmaMgr_t *state, sbFe2000DmaAddr_t feAddress,
                         uint32_t *data, uint8_t clearOnRead)
{
  sbFe2000DmaPioRegs_t *pRegs;
  sbStatus_t status;
  uint32_t i, j, bank, offs, reg, bankWords, clsHack;

  if ((status = sbFe2000DmaWaitBankIdle(state, feAddress)) != SB_OK) {
    return status;
  }

  bank = SB_FE2000_DMA_ADDRESS_BANK(feAddress);
  offs = SB_FE2000_DMA_ADDRESS_OFFSET(feAddress);

  if ((bank == SB_FE2000_MEM_CLS_0) || (bank == SB_FE2000_MEM_CLS_1))
    clsHack = 1;
  else
    clsHack = 0;

  pRegs = &state->pioRegs[bank];
  bankWords = pRegs->words;

  /* compose request using physical addressing, no segments
   * the control registers all share the same layout */
  reg = (SAND_HAL_FE2000_SET_FIELD(pRegs->devBaseAddr, MM_INT0_MEM_ACC_CTRL, ACK, 1) |
         SAND_HAL_FE2000_SET_FIELD(pRegs->devBaseAddr, MM_INT0_MEM_ACC_CTRL, REQ, 1) |
         SAND_HAL_FE2000_SET_FIELD(pRegs->devBaseAddr, MM_INT0_MEM_ACC_CTRL, RD_WR_N, 1) |
         SAND_HAL_FE2000_SET_FIELD(pRegs->devBaseAddr, MM_NARROW_PORT0_MEM_ACC_CTRL, ADDRESS, offs));

  SAND_HAL_FE2000_WRITE_OFFS(pRegs->devBaseAddr, pRegs->request, reg);

  if ((status = sbFe2000DmaWaitIndirect(state->pCfg->pioTimeout, pRegs)) != SB_OK) {
    return status;
  }
  for (i = 0; i < bankWords; i++) {
    pRegs->bReverseLayout ? (j = bankWords - i - 1) : (j = i);
    if (clsHack) {
      if ((i > 3) && (i < 32))
        continue;   /* skip padding hole */
      if (i >= 32)
        j = j - 28; /* adjust data reg for padding */
    }
    data[i] = SAND_HAL_READ_OFFS_RAW(pRegs->devBaseAddr,
                                     pRegs->data + j * sizeof(uint32_t));
  }

  /* clear the ack/req */
  reg &= ~(SAND_HAL_FE2000_SET_FIELD(pRegs->devBaseAddr, MM_INT0_MEM_ACC_CTRL, ACK, 1) |
           SAND_HAL_FE2000_SET_FIELD(pRegs->devBaseAddr, MM_INT0_MEM_ACC_CTRL, REQ, 1));

  SAND_HAL_FE2000_WRITE_OFFS(pRegs->devBaseAddr, pRegs->request, reg);

  return SB_OK;
}

sbStatus_t
sbFe2000DmaReadIndirectMult(sbFe2000DmaMgr_t *state, sbFe2000DmaAddr_t feAddress,
                            uint32_t *data, uint8_t clearOnRead, uint32_t words)
{
  uint32_t i, bank, offs, *pdata, bwords, addr;
  sbStatus_t sts;

  /* parse start of transaction */
  bank = SB_FE2000_DMA_ADDRESS_BANK(feAddress);
  offs = SB_FE2000_DMA_ADDRESS_OFFSET(feAddress);

  /* uint32_t words per memory address */
  bwords = state->pioRegs[bank].words;

  for (i = 0; i < words / bwords; i++) {
    addr = SB_FE2000_DMA_MAKE_ADDRESS(bank, offs + i);
    pdata = data + i * bwords;
    sts = sbFe2000DmaReadIndirect(state, addr, pdata, clearOnRead);
    if (sts != SB_OK)
      return sts;
  }

  return SB_OK;
}

/*-----------------------------------------------------------------------------
 * This function is called when a DMA operation is complete.
 *
 * The basic functionality is to clean up after the DMA operations that have
 * just completed. This also involves calling readSync for any completed read
 * operations. The waitingOps queue is examined and if any DMA operations
 * are in the queue then the top entries are popped and DMA operations are
 * kicked off for them.
 *
 *---------------------------------------------------------------------------*/
void
sbFe2000DmaIsr(sbFe2000DmaMgr_t *state)
{
  uint8_t again = TRUE;
  sbFe2000DmaChanRegs_t *regs;
  sbFe2000DmaOp_t *op;
  dq_t done;
  sbStatus_t status;
  uint32_t ulMax, *pEntry;
  SOC_SBX_WARM_BOOT_DECLARE(int wb);

  SOC_SBX_WARM_BOOT_IGNORE((int)state->clientData, wb);

  ulMax = state->bUseRing ? state->ulRingEntries : SB_FE2000_DMA_CHANNELS;
  SB_ASSERT((state->dmasActive != 0) && (state->dmasActive <= ulMax));

  while (state->dmasActive && again) {
    DQ_INIT(&done);
    while (state->dmasActive != 0) {
      SB_ASSERT(!DQ_EMPTY(&state->activeOps));
      if (!state->bUseRing) {
        /* thread based dma */
        regs = &state->dmaRegs[state->regsConsumer];
        if ((SAND_HAL_FE2000_READ_OFFS(regs->devBaseAddr, regs->request)
             & SAND_HAL_FE2000_FIELD_MASK(regs->devBaseAddr, PC_DMA, CTRL0_ACK)) == 0) {
          again = 0;
          break;
        }
        SAND_HAL_FE2000_WRITE_OFFS(regs->devBaseAddr, regs->request,
                            SAND_HAL_FE2000_FIELD_MASK(regs->devBaseAddr, PC_DMA, CTRL0_ACK));
        state->regsConsumer = (state->regsConsumer + 1) % SB_FE2000_DMA_CHANNELS;
      } else {
        /* ring based dma */
        pEntry = state->pRingPtr + state->ulConsumerOffs / sizeof(uint32_t);
        if (*pEntry & SET_RFLD(OWNR, 1)) {
          again = 0;
          break;
        }
        state->ulConsumerOffs = sbFe2000DmaRingNext(state, state->ulConsumerOffs);
      }
      DQ_REMOVE_HEAD(&state->activeOps, op);
      state->dmasActive--;
      if (op->opcode == SB_FE2000_DMA_OPCODE_READ
          || op->opcode == SB_FE2000_DMA_OPCODE_READ_CLEAR) {
        /* TODO: could do a single sync for all reads if supported */
        status = (state->pCfg->readSync)(op->dmaHandle, op->hostAddress, op->words);
      } else {
        status = SB_OK;
      }
      op->status = status;
      DQ_INSERT_TAIL(&done, op);
    }
    while (!DQ_EMPTY(&state->waitingOps)
           && sbFe2000DmaIsStartable(state, DQ_HEAD(&state->waitingOps,
                                                  sbFe2000DmaOp_t*))) {
      DQ_REMOVE_HEAD(&state->waitingOps, op);
      status = sbFe2000DmaStart(state, op);
      if (status != SB_IN_PROGRESS) {
        op->status = status;
        DQ_INSERT_TAIL(&done, op);
      }
    }
    DQ_MAP(&done, sbFe2000DmaDone, 0);
  }

  SOC_SBX_WARM_BOOT_OBSERVE((int)state->clientData, wb);
}

/*-----------------------------------------------------------------------------
 * Each time a DMA completes it calls sbFe2000DmaIsr. This in turn calls
 * sbFe2000DmaDone for each completion. The purpose of this function is to
 * execute the callback function for the completed DMA operation.
 *---------------------------------------------------------------------------*/
static void
sbFe2000DmaDone(dq_t *op0, int a)
{
  sbFe2000DmaOp_t *op = (sbFe2000DmaOp_t *) op0;

  /* range check the opcode that it isn't garbage */
  SB_ASSERT((op->opcode >= SB_FE2000_DMA_OPCODE_READ) &&
            (op->opcode <= SB_FE2000_DMA_OPCODE_END));

  if (g_bSbG2FeDmaStartDump)
    SB_LOG("DmaDone: opcode: 0x%x, %d words, hba: 0x%x, fea: 0x%x\n",
           op->opcode, op->words,
           (unsigned int) op->hostAddress, (unsigned int) op->feAddress);

  (op->cb)(op);
}

/*****************************************************************
 *****************************************************************
 ***                                                           ***
 ***                      Slab Operations                      ***
 ***                                                           ***
 *****************************************************************
 *****************************************************************
 *
 * Slabs are constructs that sit on top of the DMA functions and provide a
 * more efficient method of transfering data to & from the FE. The basic data
 * structure is the slabOp.
 *
 * slabOp contains a pointer to a multiple contiguous slabs which contains the
 * actual data to be DMAed. The each slab is refered to as a chunk; an
 * overview of the structure is:
 *
 * slabOp->slab -----> chunk1: opcode (READ or WRITE)
 *                             feAddress
 *                             words
 *                             data[words]
 *                     chunk2: opcode (READ or WRITE)
 *                             feAddress
 *                             words
 *                             data[words]
 *                               .....
 *                     chunkN: opcode (SB_FE2000_DMA_OPCODE_END)
 *                             feAddress
 *                             words
 *                             data[words]
 *
 * The interface from the rest of the systems is through a call to
 * sbFe2000DmaSlabRequest. The slab opcodes are a superset of the DMA
 * opcoded are mostly self  expanitory.
 *
 * SB_FE2000_DMA_OPCODE_END does not contain any data, it simply indicates
 * the end of consecutive chunks.
 *
 * SB_FE2000_DMA_OPCODE_WRITE_READ is used in a chunk that follows a READ
 * opcode (either SB_FE2000_DMA_OPCODE_READ or
 * SB_FE2000_DMA_OPCODE_READ_CLEAR) the address of the previous read is used
 * to write the data specified in the WRITE_READ chunk.
 *
 * SB_FE2000_DMA_OPCODE_READ_WORDS is a special case of READ. The data area
 * of the chunk contains FE addresses. The data read from these addresses is
 * copied over the address in the chunk. This allows a single chunk to
 * instigateread multiple single word reads from non-contiguous FE memory.
 *
 *****************************************************************
 *****************************************************************/

static void
sbFe2000DmaSlabRequestCallback(sbFe2000DmaSlabOp_t *slabOp)
{
    int unit = (int) slabOp->data;
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);

    sal_sem_give(sbx->dma_sem);
}

sbStatus_t
sbFe2000DmaSlabRequest(sbFe2000DmaMgr_t *pDmaMgr, void *chipUserToken,
                       sbFe2000DmaSlabOp_t *slabOp)
{
    int unit = (int) chipUserToken;
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    void *data0 = slabOp->data;
    sbStatus_t status;
  
  slabOp->data = (void *) unit;
  slabOp->cb = sbFe2000DmaSlabRequestCallback;
  
  sal_sem_take(pDmaMgr->sync_sem, sal_sem_FOREVER);

    status = sbFe2000DmaSlabRequestI(pDmaMgr, chipUserToken, slabOp);
    if (status == SB_IN_PROGRESS) {
        sal_sem_take(sbx->dma_sem, sal_sem_FOREVER);
        status = slabOp->status;
    }

  sal_sem_give(pDmaMgr->sync_sem);
  
  slabOp->data = data0;
  
  return status;
}



/*-----------------------------------------------------------------------------
 * This function is the main API to the rest of the software. It DMAs the slab
 * data associated with the slabOp parameter.
 *
 * The first part of this function performs a sanity check on the slab
 * structure. Next the hostBusAddress is calculated if it is not specified.
 * The final section kicks off the DMA by calling sbFe2000DmaSlabStartChunk
 * or if the DMA is busy adds the slab onto a waiting queue for later
 * processing
 *
 * returns: SB_IN_PROGRESS - DMA kicked off or queued
 *          SB_OK
 *          SB_BAD_ARGUMENT_ERR_CODE
 *          error
 *---------------------------------------------------------------------------*/
sbStatus_t
sbFe2000DmaSlabRequestI(sbFe2000DmaMgr_t *state, void *chipUserToken,
                        sbFe2000DmaSlabOp_t *slabOp)
{
  sbFe2000DmaSlab_t *chunk;
  sbFe2000DmaSlab_t *prevChunk;
  uint32_t slabSize;
  sbStatus_t status;
  uint8_t writeRead;
  sbFe2000DmaOp_t dmaOp;
  uint8_t noMore;
  sbSyncToken_t sync;
  uint32_t word;

  chunk = slabOp->slab;
  prevChunk = NULL;
  slabSize = 0;

  while (chunk->opcode != SB_FE2000_DMA_OPCODE_END) {
    if (chunk->opcode == SB_FE2000_DMA_OPCODE_READ_WORDS) {
      dmaOp.opcode = SB_FE2000_DMA_OPCODE_READ;
      dmaOp.words = 1;
      dmaOp.hostBusAddress = 0;
      for (word = 0; word < chunk->words; word++) {
        dmaOp.hostAddress = &chunk->data[word];
        dmaOp.feAddress = chunk->data[word];
        status = sbFe2000DmaCheckOpArguments(state, &dmaOp);
        if (status != SB_OK) {
          return status;
        }
      }
      prevChunk = NULL;
      slabSize += SB_FE2000_SLAB_SIZE(chunk->words);
      chunk = SB_FE2000_SLAB_NEXT(chunk);
    } else {
      writeRead = (chunk->opcode == SB_FE2000_DMA_OPCODE_WRITE_READ);
      if (writeRead && !prevChunk) {
        return SB_BAD_ARGUMENT_ERR_CODE;
      }
      dmaOp.opcode = writeRead ? SB_FE2000_DMA_OPCODE_WRITE : chunk->opcode;
      dmaOp.words = chunk->words;
      dmaOp.hostAddress = writeRead ? prevChunk->data : chunk->data;
      dmaOp.hostBusAddress = 0;
      dmaOp.feAddress = chunk->feAddress;
      status = sbFe2000DmaCheckOpArguments(state, &dmaOp);
      if (status != SB_OK) {
        return status;
      }
      prevChunk = ((chunk->opcode == SB_FE2000_DMA_OPCODE_READ
                    || chunk->opcode == SB_FE2000_DMA_OPCODE_READ_CLEAR) ?
                   chunk : NULL);
      slabSize += SB_FE2000_SLAB_SIZE(chunk->words);
      chunk = SB_FE2000_SLAB_NEXT(chunk);
    }
  }

  if (slabOp->hostBusAddress == SB_FE2000_DMA_HBA_COMPUTE) {
    status =
      (state->pCfg->getHba)(slabOp->dmaHandle, (uint32_t *) slabOp->slab,
                           slabSize / sizeof(uint32_t),
                           &slabOp->hostBusAddress);
    if (status != SB_OK) {
      return status;
    }
  }

  slabOp->active = 0;
  slabOp->status = SB_OK;
  status = SB_IN_PROGRESS;
  sync = (state->pCfg->isrSync)(state->clientData);
  if (state->slabCurrent) {
    DQ_INSERT_TAIL(&state->slabsWaitingStart, slabOp);
  } else {
    state->slabCurrent = slabOp;
    state->slabCurrentChunk = slabOp->slab;
    state->slabPreviousChunk = NULL;
    state->slabChunkCurrentWord = 0;
    noMore = 0;
    while (state->slabActiveDmas < 2
           && !(noMore = sbFe2000DmaSlabStartChunk(state)));
    if (noMore) {
      if (slabOp->active) {
        DQ_INSERT_TAIL(&state->slabsWaitingLast, slabOp);
      } else {
        status = slabOp->status;
      }
    }
  }
  (state->pCfg->isrUnsync)(state->clientData, sync);
  return status;
}

/*-----------------------------------------------------------------------------
 * Slab function involked when a slab operation completes.
 *
 * The basic functionality is to clean up after slab chunk DMA operations that
 * have just completed. The current slab is checked for any more chunks and if
 * found the next chunks are DMAed. If the end of the slab is encountered then
 * the slabWaiting queue is examined and if it is not empty the top entry is
 * processed.
 *
 * xxx : higher level callback needed to wrap dma
 *
 *---------------------------------------------------------------------------*/
static void
sbFe2000DmaSlabDmaDone(sbFe2000DmaOp_t *dmaOp)
{
  sbFe2000DmaMgr_t *state = (sbFe2000DmaMgr_t *) dmaOp->data;
  sbFe2000DmaSlabOp_t *slabOp;
  uint8_t currentIsFirst;
  uint8_t noMore;
  dq_t done;
  dq_t *q;

  DQ_INIT(&done);

  SB_ASSERT(state->slabActiveDmas != 0);
  state->slabActiveDmas--;

  currentIsFirst = DQ_EMPTY(&state->slabsWaitingLast);
  if (currentIsFirst) {
    slabOp = state->slabCurrent;
  } else {
    slabOp = DQ_HEAD(&state->slabsWaitingLast, sbFe2000DmaSlabOp_t*);
  }
  SB_ASSERT(slabOp);
  SB_ASSERT(slabOp->active);
  slabOp->active--;

  if (dmaOp->status != SB_OK) {
    slabOp->status = dmaOp->status;
    if (currentIsFirst) {
      state->slabCurrent = NULL;
    }
    q = slabOp->active ? &state->slabsWaitingLast : &done;
    DQ_INSERT_TAIL(q, slabOp);
  }

  while (!DQ_EMPTY(&state->slabsWaitingLast)
         && ((slabOp = DQ_HEAD(&state->slabsWaitingLast,
                               sbFe2000DmaSlabOp_t*))->active == 0)) {
    DQ_REMOVE(slabOp);
    DQ_INSERT_TAIL(&done, slabOp);
  }

  while (state->slabActiveDmas < 2) {
    slabOp = state->slabCurrent;
    if (!slabOp) {
      if (!DQ_EMPTY(&state->slabsWaitingStart)) {
        DQ_REMOVE_HEAD(&state->slabsWaitingStart, slabOp);
        state->slabCurrent = slabOp;
        state->slabCurrentChunk = slabOp->slab;
        state->slabPreviousChunk = NULL;
      } else {
        break;
      }
    }
    noMore = sbFe2000DmaSlabStartChunk(state);
    if (noMore) {
      q = slabOp->active ? &state->slabsWaitingLast : &done;
      DQ_INSERT_TAIL(q, slabOp);
      state->slabCurrent = NULL;
    }
  }
  DQ_MAP(&done, sbFe2000DmaSlabDone, 0);
}

/*-----------------------------------------------------------------------------
 * sbFe2000DmaDone is called upon competion of each slab chunk DMA operation.
 * The purpose of this function is to execute the callback function for the
 * completed DMA operation.
 *---------------------------------------------------------------------------*/
static void
sbFe2000DmaSlabDone(dq_t *op0, int a)
{
  sbFe2000DmaSlabOp_t *op = (sbFe2000DmaSlabOp_t*) op0;

  (op->cb)(op);
}

/*-----------------------------------------------------------------------------
 * Performs the slab chunk specific processing and interfaces to the base DMA
 * function sbFe2000DmaRequest to initiate the DMA transfer.
 *
 * This function translates the WRITE_READ opcode into a DMA WRITE opcode and
 * also deals with the single word transfers associated with the READ_WORDS
 * slab opcode.
 *
 * Returns: true if no more chunks need to be started for this slab
 *---------------------------------------------------------------------------*/
static uint8_t
sbFe2000DmaSlabStartChunk(sbFe2000DmaMgr_t *state)
{
  sbFe2000DmaSlabOp_t *slabOp = state->slabCurrent;
  sbFe2000DmaSlab_t *chunk = state->slabCurrentChunk;
  sbFe2000DmaOp_t *dmaOp = &state->slabDmas[state->slabDmaProducer];
  sbStatus_t status = SB_OK;
  uint8_t readwords = chunk->opcode == SB_FE2000_DMA_OPCODE_READ_WORDS;
  uint8_t noMore;

  SB_ASSERT(state->slabActiveDmas < 2);
  SB_ASSERT(slabOp);
  SB_ASSERT(chunk->opcode != SB_FE2000_DMA_OPCODE_END);
  if (chunk->opcode == SB_FE2000_DMA_OPCODE_WRITE_READ) {
    SB_ASSERT(state->slabPreviousChunk);
    SB_ASSERT(state->slabPreviousChunk->opcode == SB_FE2000_DMA_OPCODE_READ);
    dmaOp->opcode = SB_FE2000_DMA_OPCODE_WRITE;
    dmaOp->hostAddress = state->slabPreviousChunk->data;
    dmaOp->feAddress = chunk->feAddress;
    dmaOp->words = chunk->words;
  } else if (readwords) {
    dmaOp->opcode = SB_FE2000_DMA_OPCODE_READ;
    dmaOp->hostAddress = &(chunk->data[state->slabChunkCurrentWord]);
    dmaOp->feAddress = chunk->data[state->slabChunkCurrentWord];
    dmaOp->words = 1;
    state->slabChunkCurrentWord++;
  } else {

    dmaOp->opcode = chunk->opcode;
    dmaOp->hostAddress = chunk->data;
    dmaOp->feAddress = chunk->feAddress;
    dmaOp->words = chunk->words;
  }

  SB_ASSERT(slabOp->hostBusAddress != SB_FE2000_DMA_HBA_COMPUTE);
  dmaOp->hostBusAddress =
    slabOp->hostBusAddress
    + ((sbFeDmaHostBusAddress_t)
       (((uint8_t *) dmaOp->hostAddress) - ((uint8_t *) slabOp->slab)));
  dmaOp->dmaHandle = slabOp->dmaHandle;

  status = sbFe2000DmaRequestI(state, state->clientData, dmaOp);

  if (!readwords || state->slabChunkCurrentWord >= chunk->words) {
    state->slabPreviousChunk = chunk;
    state->slabCurrentChunk = SB_FE2000_SLAB_NEXT(chunk);
    state->slabChunkCurrentWord = 0;
  }
  if (status == SB_IN_PROGRESS) {
    slabOp->active++;
    state->slabActiveDmas++;
    state->slabDmaProducer ^= 1;
  } else if (status != SB_OK) {
    slabOp->status = status;
  }

  noMore = (status != SB_OK && status != SB_IN_PROGRESS)
    || state->slabCurrentChunk->opcode == SB_FE2000_DMA_OPCODE_END;

  if (noMore) {
    state->slabCurrent = NULL;
  }
  return noMore;
}

/*-------------------------------------------------------------------
 * If the max DMA size for this transfer has not been specifically
 * configured then return the smallest of maxContigWords or maxWords.
 *-------------------------------------------------------------------*/
void
sbFe2000SetDmaSize(uint32_t *p, uint32_t maxContigWords, uint32_t maxWords)
{
  if (*p == SB_FE2000_PARAMS_DEFAULT32) {
    *p = maxContigWords < maxWords ? maxContigWords : maxWords;
  }
}

/*-------------------------------------------------------------------
 *
 * Calculate DMA layout space for fragments in a large buffer.
 *   offset - input as current offset, output as new offset
 *   totalWords - input as words alloced, output as new words alloced
 *   words - input as words to be alloced for this frag
 *   maxContigWords - input as max cont block of DMAable memory
 *
 * Given a number of fragments that need to be DMAed in a single buffer,
 * this function is called for each fragment to build the buffer
 * layout. Each frag is given at least maxContigWords and all frags
 * are on a maxContigWords boundary.
 *
 *-------------------------------------------------------------------*/
void
sbFe2000LayoutDmaBuffer(uint32_t *offset, uint32_t *totalWords,
			 uint32_t words, uint32_t maxContigWords)
{
  uint32_t left = maxContigWords - (*totalWords % maxContigWords);

  if (left != maxContigWords && left < words) {
    *totalWords += left;
  }
  *offset = *totalWords;
  *totalWords += words;
}

uint8_t
sbFe2000DmaIsWideBank(uint32_t ulId)
{
  if (ulId >= SB_FE2000_MEM_MAX_SRCS)
    return 0;

  switch(ulId) {
  case SB_FE2000_MEM_MM0_WIDE:
  case SB_FE2000_MEM_MM1_WIDE:
    return 1;
  default:
    return 0;
  }

  return 0; /* fool gcc */
}

uint32_t
sbFe2000DmaGetMaxPageSize(sbFe2000DmaMgr_t *state)
{
  return state->pCfg->maxPageSize;
}

void
sbFe2000DmaIssueMode(sbFe2000DmaMgr_t *state, sbFe200DmaIssueMode_e_t mode)
{
  state->eDmaMode = mode;
}

uint32_t
sbFe2000DmaRingNext(sbFe2000DmaMgr_t *state, uint32_t ulAddr)
{
  ulAddr += SB_FE2000_RING_ENTRY_SIZE_BYTES;
  ulAddr &= state->ulRingMask;

  return ulAddr;
}
