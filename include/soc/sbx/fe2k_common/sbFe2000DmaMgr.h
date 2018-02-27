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
 * $Id: sbFe2000DmaMgr.h 1.4.156.1 Broadcom SDK $
 *
 * sbFe2000DmaMgr.h : FE2000 DMA Engine Driver
 *
 *-----------------------------------------------------------------------------*/
#ifndef _SB_FE_2000_DMA_H_
#define _SB_FE_2000_DMA_H_

#include "sbTypes.h"
#include "sbTypesGlue.h"
#include <soc/sbx/sbDq.h>

/* used for defaulting parameters */
#define SB_FE2000_PARAMS_DEFAULT32 0xffffFFFF

/*-----------------------------------------------------------------------------
 * DMA ADDRESS DEFINITIONS
 * We pack the bank and offset for a chip address into
 * a uint32_t word. A portion of this address specifies the target memory bank,
 * and the offset within the bank. The bank is specified by the enumerated type
 * sbFe2000MemorySrcs_e_t, and the offset is in bank addresses (memory words).
 *-----------------------------------------------------------------------------*/
typedef uint32_t sbFe2000DevAddr_t;
typedef uint32_t sbFe2000DmaAddr_t;

/* Addresses are packed with a 5-bit segment, then 23-bit word addr */
#define SB_FE2000_SRAM_BANK_SIZE 5
#define SB_FE2000_SRAM_ADDR_SIZE 23

/* Generate appropriately sized masks for fields */
#define __MSK_N(x) ((0x1 << (x)) - 1)
#define SB_FE2000_DMA_OFFS_MASK __MSK_N(SB_FE2000_SRAM_ADDR_SIZE)
#define SB_FE2000_DMA_BANK_MASK __MSK_N(SB_FE2000_SRAM_BANK_SIZE)

/* Create DMA address from a BANK and OFFSET */
#define SB_FE2000_DMA_MAKE_ADDRESS(bank, off) \
  ((((bank) & SB_FE2000_DMA_BANK_MASK) << SB_FE2000_SRAM_ADDR_SIZE) | \
   ((off) & SB_FE2000_DMA_OFFS_MASK))

/* Parse out BANK from DMA address */
#define SB_FE2000_DMA_ADDRESS_BANK(addr) \
  (((addr) >> SB_FE2000_SRAM_ADDR_SIZE) & SB_FE2000_DMA_BANK_MASK)

/* Parse out OFFSET from DMA address */
#define SB_FE2000_DMA_ADDRESS_OFFSET(addr) \
  ((addr) & SB_FE2000_DMA_OFFS_MASK)

/*-----------------------------------------------------------------------------
 * FUNCTION POINTER DEFINITIONS
 * These data types are used to specify system interface functions that are 
 * required by the DMA for operation. They are used to define references to
 * (function pointer) allowing ease of configuration.
 *-----------------------------------------------------------------------------*/

/*
 * Convert an address to a host bus address for memory which has been
 *   allocated for DMA
 *
 * `words' is to ensure that the buffer being DMAed does not cross
 *    a contiguity boundary on the host bus.
 * This function must be callable from interrupt context
 *
 * mHandle: the handle returned from the DMAable memory allocation function
 * address: pointer to the DMAable memory
 * words: words to be DMAed
 * hbaP: where to return the translated host bus address
 */
typedef sbStatus_t (*sbFe2000DmaGetHostBusAddress_f_t)
  (sbDmaMemoryHandle_t mHandle, uint32_t *address,
   uint32_t words, sbFeDmaHostBusAddress_t *hbaP);

/*
 * Synchronize a memory region before a DMA write operation (host->FE)
 *
 * This function must be callable from interrupt context
 *
 * mHandle: the handle returned from the DMAable memory allocation function
 * address: pointer to the DMAable memory
 * words: words to be DMAed
 */
typedef sbStatus_t (*sbFe2000DmaWriteSync_f_t)
  (sbDmaMemoryHandle_t mHandle, uint32_t *address, uint32_t words);

/*
 * Synchronize a memory region after a DMA read operation (FE->host)
 *
 * This function must be callable from interrupt context
 *
 * mHandle: the handle returned from the DMAable memory allocation function
 * address: pointer to the DMAable memory
 * words: words to be DMAed
 */
typedef sbStatus_t (*sbFe2000DmaReadSync_f_t)
  (sbDmaMemoryHandle_t mHandle, uint32_t *address, uint32_t words);

/*
 * Synchronize a critical section with code that may run
 * in interrupt context
 *
 * This function must be callable from interrupt context
 *
 * On UNIX platforms, this is typically something like splbio(), and the
 * token returned is the current interrupt priority level
 *
 * chipUserToken: chip user-supplied fe->cParams.clientData for the FE
 * Returns:
 *   a value used by the companion unsynchronize
 */
typedef sbSyncToken_t (*sbFe2000IsrSync_f_t)(void *chipUserToken);

/*
 * End a critical section that is synchronized with code that may run
 * in interrupt context
 *
 * This function must be callable from interrupt context
 *
 * On UNIX platforms, this is typically something like splx(token)
 *
 * chipUserToken: chip user-supplied fe->cParams.clientData for the FE
 * syncToken: the sync token returned by the matching IsrSync call
 */
typedef void (*sbFe2000IsrUnsync_f_t)(void *chipUserToken,
                                      sbSyncToken_t syncToken);

/*
 * This enumeration specifies the external memories that are present in
 * the FE-2000. The ordering of these banks matches the encoding used 
 * in the thread-based DMA. DMA addresses use this enumeration to 
 * specify a device memory bank when composing a DMA address.
 */
typedef enum sbFe2000MemorySrcs_e_s
{
  SB_FE2000_MEM_MM0_NARROW_0 = 0,
  SB_FE2000_MEM_MM0_NARROW_1,
  SB_FE2000_MEM_MM0_WIDE,
  SB_FE2000_MEM_MM0_INTERNAL_0,
  SB_FE2000_MEM_MM0_INTERNAL_1,
  SB_FE2000_MEM_MM1_NARROW_0,
  SB_FE2000_MEM_MM1_NARROW_1,
  SB_FE2000_MEM_MM1_WIDE,
  SB_FE2000_MEM_MM1_INTERNAL_0,
  SB_FE2000_MEM_MM1_INTERNAL_1,
  SB_FE2000_MEM_LRP,
  SB_FE2000_MEM_CLS_0,
  SB_FE2000_MEM_CLS_1,
  SB_FE2000_MEM_MEM,
  SB_FE2000_MEM_FIFO,
  /* leave as last */
  SB_FE2000_MEM_MAX_SRCS
} sbFe2000MemorySrcs_e_t;

/*
 * A DMA operation with an FE address of LRP_SYNC ensures that no LRPs
 * are using data from preceeding DMA operations
 * SLAB_END is only used as the FE address in slabs
 */
typedef enum sbFe2000DmaOpcode_e_s
{
  SB_FE2000_DMA_OPCODE_READ,	   /* FE->host */
  SB_FE2000_DMA_OPCODE_READ_CLEAR, /* FE->host */
  SB_FE2000_DMA_OPCODE_WRITE,	   /* host->FE */
  SB_FE2000_DMA_OPCODE_CLEAR,	   /* host->FE */
  SB_FE2000_DMA_OPCODE_SYNC,
  SB_FE2000_DMA_OPCODE_WRITE_READ, /* slabs only, host->FE */
  SB_FE2000_DMA_OPCODE_READ_WORDS, /* slabs only, FE->host */
  SB_FE2000_DMA_OPCODE_END	   /* slabs only */
} sbFe2000DmaOpcode_e_t;

/*
 * Transactions can be issued in one of three manners
 */
typedef enum sbFe200DmaIssueMode_e_s
{
  SB_FE2000_DMA_MODE_PIO_ONLY = 0,
  SB_FE2000_DMA_MODE_DMA_ONLY,
  SB_FE2000_DMA_MODE_PIO_AND_DMA
} sbFe200DmaIssueMode_e_t;

/*
 * Set hostBusAddress to this value to indicate that the client has not
 * computed the host bus address in advance (and the DMA code should
 * compute it).  Note that host bus addresses must be word aligned,
 * so 0x1 is an otherwise illegal value.
 */
#define SB_FE2000_DMA_HBA_COMPUTE 0x1

/* forward declare type to allow self-reference in struct */
typedef struct sbFe2000DmaOp_s sbFe2000DmaOp_t;

/* declare DMA callback function pointer type */
typedef void (*sbFe2000DmaDoneCallback_f_t) (sbFe2000DmaOp_t *op);

/* 
 * This structure defines a DMA operation, and is the element that is used
 * to hand off DMA operations from client software to the DMA software.
 */
struct sbFe2000DmaOp_s 
{
  dq_t links;			          /* must be first */
  void *data;			          /* client data */
  sbStatus_t status;	                  /* only set if completed through callback */
  sbFe2000DmaDoneCallback_f_t cb;         /* may be called in interrupt context */
  sbFe2000DmaOpcode_e_t opcode;           /* dma operation to perform */
  uint32_t *hostAddress;                  /* host DMA buffer address (word aligned) */
  sbDmaMemoryHandle_t dmaHandle;          /* handle for allocated DMA sub-system */
  sbFeDmaHostBusAddress_t hostBusAddress; /* host side Bus-Address, may be calculated */
  sbFe2000DmaAddr_t feAddress;            /* target device address */
  uint32_t words;		          /* transfer length */
  uint8_t bForcePio;                      /* force PIO (no dma) */
};

/* A slab is a sequence of sbFe2000DmaSlab_t */
typedef struct sbFe2000DmaSlab_s {
  sbFe2000DmaOpcode_e_t opcode;
  sbFe2000DmaAddr_t feAddress;
  uint32_t words;		/* chunk length */
  uint32_t data[1];
} sbFe2000DmaSlab_t;

#define SB_FE2000_SLAB_SIZE(nwords) (sizeof(sbFe2000DmaSlab_t) / sizeof(uint32_t) - 1 + (nwords))
#define SB_FE2000_SLAB_NEXT(p) ((sbFe2000DmaSlab_t *) ((uint32_t *)(p) + SB_FE2000_SLAB_SIZE((p)->words)))

/* forward declare for self-reference in struct */
typedef struct sbFe2000DmaSlabOp_s sbFe2000DmaSlabOp_t;

typedef void (*sbFe2000DmaSlabDoneCallback_f_t) (sbFe2000DmaSlabOp_t *op);

struct sbFe2000DmaSlabOp_s 
{
  dq_t links;			/* must be first */
  void *data;			/* client data */
  sbDmaMemoryHandle_t dmaHandle;
  sbFeDmaHostBusAddress_t hostBusAddress;
  sbFe2000DmaSlabDoneCallback_f_t cb;
  sbFe2000DmaSlab_t *slab;
  sbStatus_t status;
  /* internal state */
  int active;
};

#define SB_FE2000_DMA_CHANNELS 4

typedef struct sbFe2000DmaConfig_s 
{
  /* module level parameters */
  uint32_t pciReadsPerEpoch;
  uint32_t pioTimeout;
  uint32_t maxPio;
  uint32_t regSet;
  uint32_t maxPageSize;

  /* internal memory configurations */
  uint32_t mm0i0wide;
  uint32_t mm0i1wide;
  uint32_t mm1i0wide;
  uint32_t mm1i1wide;

  /* external memory masks */
  uint32_t ulMM0MaskNarrow0;
  uint32_t ulMM0MaskNarrow1;
  uint32_t ulMM1MaskNarrow0;
  uint32_t ulMM1MaskNarrow1;
  uint32_t ulMM0MaskWide;
  uint32_t ulMM1MaskWide;
  uint32_t ulClsMask;

  /* system interface functions */
  sbFe2000DmaGetHostBusAddress_f_t getHba;
  sbFe2000DmaWriteSync_f_t writeSync;
  sbFe2000DmaReadSync_f_t readSync;
  sbFe2000IsrSync_f_t isrSync;
  sbFe2000IsrUnsync_f_t isrUnsync;

  /* handle to device and driver */
  sbhandle handle;
  void *clientData;

  /* dma control */
  uint8_t bUseRing;
  uint32_t ulRingEntries;
} sbFe2000DmaConfig_t;

/* forward declare control structure. This is hidden inside the 
 * sbFe2000Dma.c file */
typedef struct sbFe2000DmaMgr_s sbFe2000DmaMgr_t;

/*-----------------------------------------------------------------------------
 * @fn sbFe2000DmaInit()
 *
 * @brief Initializes the DMA software module.
 *
 * @param DMA Control Structure
 * @param Dma Configuration Structure
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000DmaInit(sbFe2000DmaMgr_t **pCtl, sbFe2000DmaConfig_t *pDmaCfg);

/*-----------------------------------------------------------------------------
 * Do an indirect write.
 *
 * non-NULL data is assumed to point to enough words to satisfy an entire write
 * to the bank (e.g. 4 for pktc rule memory)
 * NULL data results in 0s being written
 *
 * The caller of this function MUST externally ensure synchronization with the
 * data mover ISR.  In other words, a call to this function which would be
 * otherwise unsynchronized with the ISR must be surrounded by an ISR
 * sync/unsync pair.
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000DmaWriteIndirect(sbFe2000DmaMgr_t *state, sbFe2000DmaAddr_t feAddress, 
                         uint32_t *data);

sbStatus_t
sbFe2000DmaWriteIndirectMult(sbFe2000DmaMgr_t *state, sbFe2000DmaAddr_t feAddress, 
                             uint32_t *data, uint32_t words);

/*-----------------------------------------------------------------------------
 * Do an indirect read.
 *
 * data is assumed to point to enough words to satisfy an entire read from the
 * bank (e.g. 4 for pktc rule memory)
 *
 * The caller of this function MUST externally ensure synchronization with the
 * data mover ISR.  In other words, a call to this function which would be
 * otherwise unsynchronized with the ISR must be surrounded by an ISR
 * sync/unsync pair.
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000DmaReadIndirect(sbFe2000DmaMgr_t *state, sbFe2000DmaAddr_t feAddress, 
                        uint32_t *data, uint8_t bClearOnRead);

sbStatus_t
sbFe2000DmaReadIndirectMult(sbFe2000DmaMgr_t *state, sbFe2000DmaAddr_t feAddress, 
                            uint32_t *data, uint8_t bClearOnRead, uint32_t words);

/*-----------------------------------------------------------------------------
 * Compute a DMA buffer size considering a chip user-supplied
 * configuration parameter, the maximum contiguous DMA, and
 * an absolute maximum value
 *
 * p: points to the chip user supplied configuration value,
 *    and returns the actual, computed DMA buffer size
 * maxContigWords: the maximum contiguous DMA size, in words
 *    (note that cParams.maximumContiguousDmaMalloc is in
 *     units of bytes!)
 * maxWords: the maximum DMA size that would be `interesting' to
 *    the client code (when the whole table smaller < maxContigWords,
 *    and caveat consumer, maxContigWords could be arbitrarily large,
 *    so maxWords MUST be something sensible).
 *----------------------------------------------------------------------------*/
void
sbFe2000SetDmaSize(uint32_t *p, uint32_t maxContigWords, uint32_t maxWords);

/*-----------------------------------------------------------------------------
 * Perform the next step in laying out a DMA buffer when the buffer is
 * subdivided into several chunks.
 * Pushes the offset of the requested chunk to the beginning of
 * the next `maxContigWords' boundary if the chunk would otherwise
 * cross a boundary (including the case where the buffer is larger
 * than maxContigWords).
 *
 * offset: points to the candidate starting offset,
 *    returns the actual starting offset
 * totalWords: points to the total words in the buffer so far,
 *    returns the new total words in the buffer after adding the
 *    requested chunk
 * words: words in the new chunk
 * maxContigWords: the maximum contiguous DMA size, in words
 *    (note that cParams.maximumContiguousDmaMalloc is in
 *     units of bytes!)
 *----------------------------------------------------------------------------*/
extern void
sbFe2000LayoutDmaBuffer(uint32_t *offset, uint32_t *totalWords,
                        uint32_t words, uint32_t maxContigWords);

/* This must be picked by linker as a client-supplied function */
extern sbStatus_t
sbFe2000DmaRequest(sbFe2000DmaMgr_t *state, void *chipUserToken,
                   sbFe2000DmaOp_t *op);

/* This is the `actual' (ILib-supplied) DmaRequest function */
extern sbStatus_t
sbFe2000DmaRequestI(sbFe2000DmaMgr_t *state, void *chipUserToken,
                    sbFe2000DmaOp_t *op);

/* This must be picked by linker as a client-supplied function */
sbStatus_t
sbFe2000DmaSlabRequest(sbFe2000DmaMgr_t *state, void *chipUserToken,
                       sbFe2000DmaSlabOp_t *op);

/* This is the `actual' (ILib-supplied) DmaSlabRequest function */
sbStatus_t
sbFe2000DmaSlabRequestI(sbFe2000DmaMgr_t *state, void *chipUserToken,
                        sbFe2000DmaSlabOp_t *op);

void
sbFe2000DmaIsr(sbFe2000DmaMgr_t *state);

void
sbFe2000DmaDefaultParams(sbFe2000DmaConfig_t *params);

uint8_t
sbFe2000DmaIsWideBank(uint32_t ulId);

uint32_t
sbFe2000DmaGetMaxPageSize(sbFe2000DmaMgr_t *state);

void
sbFe2000DmaIssueMode(sbFe2000DmaMgr_t *state, sbFe200DmaIssueMode_e_t mode);

#endif /* _SB_FE_2000_DMA_H_ */
