/* -*- mode:c; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=2 cinoptions=\:0l1(0t0g0: */

/******************************************************************************

  $Copyright: Copyright 2011 Broadcom Corporation.
  This program is the proprietary software of Broadcom Corporation
  and/or its licensors, and may only be used, duplicated, modified
  or distributed pursuant to the terms and conditions of a separate,
  written license agreement executed between you and Broadcom
  (an "Authorized License").  Except as set forth in an Authorized
  License, Broadcom grants no license (express or implied), right
  to use, or waiver of any kind with respect to the Software, and
  Broadcom expressly reserves all rights in and to the Software
  and all intellectual property rights therein.  IF YOU HAVE
  NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
  IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
  ALL USE OF THE SOFTWARE.  
   
  Except as expressly set forth in the Authorized License,
   
  1.     This program, including its structure, sequence and organization,
  constitutes the valuable trade secrets of Broadcom, and you shall use
  all reasonable efforts to protect the confidentiality thereof,
  and to use this information only in connection with your use of
  Broadcom integrated circuit products.
   
  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
  PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
  REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
  OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
  DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
  NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
  ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
  CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
  OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
  
  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
  BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
  INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
  ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
  TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
  THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
  ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$

  $Id: g2p3_lpm_dma.c 1.2.112.1 Broadcom SDK $

 *****************************************************************************/

#ifndef LPM_DMA_DEBUG
#define LPM_DMA_DEBUG 0
#endif
#include "sbWrappers.h"
#include "g2p3_lpm_dma_internal.h"

/* see g2p3_lpmDma.h for contract */
sbStatus_t
g2p3_lpmDmaInit(g2p3_lpmDmaHandle_t *pHandle,
                    /* for allocating internal structs */
                    g2p3_lpmHostMemHandle_t hostMem,
                    /* for allocating/freeing slab data */
                    sbMalloc_f_t hostAlloc, void *hostAllocToken,
                    sbFree_f_t hostFree, void *hostFreeToken,
                    /* DMA context */
                    void *pDmaCtxtP,
                    /* passed to DMA requests */
                    void *fe, void *dmaReqToken,
                    /* operating parameters */
                    uint32_t maxReqs, uint32_t slabWords)
{
    sbStatus_t status;
    g2p3_lpmDmaHandle_t dma;
    void *vp;
    uint8_t *p;
    uint32_t hostAllocBytes;

    SB_ASSERT(pHandle);
    SB_ASSERT(hostMem);
    SB_ASSERT(hostAlloc);
    SB_ASSERT(pDmaCtxtP);
    SB_ASSERT(maxReqs > 0);
    SB_ASSERT(slabWords >=
           (SB_FE2000_SLAB_SIZE(1)
            + SB_FE2000_SLAB_SIZE(0))); /* for OPCODE_END */

    hostAllocBytes = (sizeof(g2p3_lpmDma_t)
                     + maxReqs * sizeof(writeRequest_t)
                     + sizeof(sbFe2000DmaSlabOp_t));
    status = g2p3_lpmHostMemAlloc(hostMem, &vp, hostAllocBytes, "lpm dma mgr");
    if (status != SB_OK)
        return status;
    p = vp;
    dma = (g2p3_lpmDmaHandle_t) p;
    p += sizeof(g2p3_lpmDma_t);
    dma->maxReqs = maxReqs;
    dma->nReqs = 0;
    dma->lastLevel = 0;
    dma->busy = FALSE;
    dma->async = FALSE;
    dma->pDmaCtxt = pDmaCtxtP;
    dma->fe = fe;
    dma->dmaReqToken = dmaReqToken;
    dma->slabOp = (sbFe2000DmaSlabOp_t *) p;
    p += sizeof(sbFe2000DmaSlabOp_t);
    dma->reqs = (writeRequest_p_t) p;
    dma->slabOp->data = (void *) dma;
    /* XXX: init rest of slabOp? */

    status = hostAlloc(hostAllocToken, SB_ALLOC_LPM_DMA,
                       slabWords * sizeof(uint32_t),
                       &vp, &dma->slabOp->dmaHandle);
    if (status != SB_OK) {
        g2p3_lpmHostMemFree(hostMem, (void *) dma, hostAllocBytes);
        return status;
    }
    SB_MEMSET(vp, 0xaa, slabWords * sizeof(uint32_t));
    dma->slabOp->slab = (sbFe2000DmaSlab_t *) vp;
    dma->slabWords = slabWords;
    dma->hostMem = hostMem;
    dma->hostFree = hostFree;
    dma->hostFreeToken = hostFreeToken;
    dma->hostAllocBytes = hostAllocBytes;

    *pHandle = dma;
    return SB_OK;
}

/* see g2p3_lpmDma.h for contract */
sbStatus_t
g2p3_lpmDmaWrite(g2p3_lpmDmaHandle_t dma,
                     sbDmaAddress_t address, size_t nWords,
                     uint32_t datum, uint8_t level,
                     g2p3_lpmCallback_f_t cb, void *cbUserData)
{
    writeRequest_p_t req;

    SB_ASSERT(dma);
    SB_ASSERT(dma->reqs);
    SB_ASSERT(dma->maxReqs > 0);
    SB_ASSERT(dma->nReqs < dma->maxReqs);

    if (dma->busy) return SB_BUSY_ERR_CODE;
    if (nWords == 0) return SB_OK;

    req = dma->reqs + dma->nReqs;
    req->address = address;
    req->nWords = nWords;
    req->datum = datum;
    req->level = level;
    ++dma->nReqs;

    if (dma->nReqs == dma->maxReqs) {
#if LPM_DMA_DEBUG
    SB_LOG("g2p3_lpmDmaWrite: calling Flush\n");
#endif
        return g2p3_lpmDmaFlush(dma, cb, cbUserData);
    } else {
        return SB_OK;
    }
}

/* requires: dma initialized; dma->nReqs > 0
 * modifies: *dma
 * effects:  none
 * returns:  earliest-address lowest-level request in the request buffer
 *           (sorted by level first, address second)
 */
static
writeRequest_p_t
nextReq(g2p3_lpmDmaHandle_t dma)
{
    writeRequest_p_t req;
    sbDmaAddress_t address;
    uint8_t level;
    uint32_t i;

    SB_ASSERT(dma);
    SB_ASSERT(dma->nReqs != 0);

    req = NULL;
    address = 0; level = 0; /* fool neurotic gcc 3.3 warning detector */
    for (i = 0; i < dma->nReqs; ++i) {
        if (dma->reqs[i].nWords != 0
            && (req == NULL
                || dma->reqs[i].level < level
                || (dma->reqs[i].level == level
                    && dma->reqs[i].address < address))) {
            req = dma->reqs + i;
            level = req->level;
            address = req->address;
        }
    }
    return req;
}

/* requires: dma initialized, dma->busy
 * modifies: *dma
 * effects:  resets nReqs and busy; calls user callback if required
 * returns:  nothing
 */
static
void
flushDone(g2p3_lpmDmaHandle_t dma)
{
    SB_ASSERT(dma);
    SB_ASSERT(dma->busy);
#if LPM_DMA_DEBUG
    SB_LOG("flushDone: nReqs => 0\n");
#endif
    dma->nReqs = 0;
    dma->busy = FALSE;
    dma->lastLevel = 0;
    if (dma->cb && dma->async)
        dma->cb(dma->cbUserData, dma->slabOp->status);
}

/* requires: slabOp, slapOp->data is an initialized dma, dma->busy
 * modifies: dma at slabOp->data
 * effects:  resets nReqs and busy; calls user callback if provided
 * returns:  nothing
 */
static
void
flushDoneCB(sbFe2000DmaSlabOp_t *slabOp)
{
    g2p3_lpmDmaHandle_t dma;

    SB_ASSERT(slabOp);
    dma = (g2p3_lpmDmaHandle_t) slabOp->data;
    SB_ASSERT(dma);
    dma->async = TRUE;
    flushDone(dma);
}

/* requires: slabOp not null; slabOp->data is an initialized dma object
 * modifies: see contract for flush()
 * effects:  calls flush() on the dma object in slabOp->data
 *           see also contract for flush()
 * returns:  nothing
 */
static
void
flushCB(sbFe2000DmaSlabOp_t *slabOp);

/* requires: dma initialized, dma->reqs not empty,
 * modifies: *dma, *slabOp, device memory
 * effects:  commits the request buffer to device memory, respecting atomicity
 * returns:  nothing
 */
/* XXX: coalesce adjacent writes into the same slab */
static
void
flush(g2p3_lpmDmaHandle_t dma)
{
    sbFe2000DmaSlabOp_t *slabOp;
    writeRequest_p_t req;
    sbFe2000DmaSlab_t *slab;
    uint32_t slabWords, i;

    SB_ASSERT(dma);
    SB_ASSERT(dma->reqs);
    slabOp = dma->slabOp;
    SB_ASSERT(slabOp);
    slab = slabOp->slab;
    SB_ASSERT(slab);
    slabWords = dma->slabWords - SB_FE2000_SLAB_SIZE(0); /* for OPCODE_END */
    while ((req = nextReq(dma))) {
        if (req->level > dma->lastLevel
            && slabWords >= SB_FE2000_SLAB_SIZE(0)) {
            slab->opcode = SB_FE2000_DMA_OPCODE_SYNC;
            slab->feAddress = 0;
            slab->words = 0;
            slab = SB_FE2000_SLAB_NEXT(slab);
            SB_ASSERT(slab);
            slabWords -= SB_FE2000_SLAB_SIZE(0);
        }
        dma->lastLevel = req->level;
        if (slabWords >= SB_FE2000_SLAB_SIZE(1)) {
            slab->opcode = SB_FE2000_DMA_OPCODE_WRITE;
            slab->feAddress = req->address;
            slab->words =
                req->nWords <  slabWords - SB_FE2000_SLAB_SIZE(0)
                ? req->nWords : slabWords - SB_FE2000_SLAB_SIZE(0);
            for (i = 0; i < slab->words; ++i) {
                slab->data[i] = req->datum;
            }
            slabWords -= SB_FE2000_SLAB_SIZE(slab->words);
#if LPM_DMA_DEBUG
            SB_LOG("dma flush: slab opcode write, @0x%06x : %3d x 0x%08x\n",
                   slab->feAddress, slab->words, req->datum);
#endif
            req->nWords -= slab->words;
            req->address += slab->words;
            slab = SB_FE2000_SLAB_NEXT(slab);
        } else {
            slab->opcode = SB_FE2000_DMA_OPCODE_END;
            slab->feAddress = 0;
            slab->words = 0;
            slabOp->hostBusAddress = SB_FE2000_DMA_HBA_COMPUTE;
            slabOp->cb = flushCB;
            dma->status = sbFe2000DmaSlabRequest(dma->pDmaCtxt, 
                            dma->dmaReqToken, slabOp);

            if (dma->status == SB_IN_PROGRESS) {
                slabOp->status = dma->status;
                return;
            }
            if (dma->status != SB_OK) {
                slabOp->status = dma->status;
                flushDone(dma);
                return;
            }
            slab = slabOp->slab;
            slabWords = dma->slabWords - SB_FE2000_SLAB_SIZE(0);
        }
    }
    if (slab != slabOp->slab) { /* finish off last slab */
        slab->opcode = SB_FE2000_DMA_OPCODE_END;
        slab->feAddress = 0;
        slab->words = 0;
        slabOp->hostBusAddress = SB_FE2000_DMA_HBA_COMPUTE;
        slabOp->cb = flushDoneCB;
        dma->status = sbFe2000DmaSlabRequest(dma->pDmaCtxt, dma->dmaReqToken, 
                        slabOp);
    }

    if (dma->status != SB_IN_PROGRESS) {
        if (dma->busy == TRUE)
            flushDone(dma);
    }
}

/* see prototype for contract */
static
void
flushCB(sbFe2000DmaSlabOp_t *slabOp)
{
    g2p3_lpmDmaHandle_t dma;

    SB_ASSERT(slabOp);
    dma = (g2p3_lpmDmaHandle_t) slabOp->data;
    SB_ASSERT(dma);
    dma->async = TRUE;
    flush(dma);
}

/* see g2p3_lpmDma.h for contract */
sbStatus_t
g2p3_lpmDmaFlush(g2p3_lpmDmaHandle_t dma,
                     g2p3_lpmCallback_f_t cb,
                     void *cbUserData)
{
    SB_ASSERT(dma);
    SB_ASSERT(dma->slabOp);
    SB_ASSERT(dma->slabOp->data == (void *) dma);

#if LPM_DMA_DEBUG
    SB_LOG("g2p3_lpmDmaFlush: nReqs == %d\n", dma->nReqs);
#endif
    if (dma->busy)
        return SB_BUSY_ERR_CODE;
    if (dma->nReqs == 0)
        return SB_OK;
    dma->busy = TRUE;
    dma->async = FALSE;
    dma->cb = cb;
    dma->cbUserData = cbUserData;
    flush(dma);
    return dma->status;
}

/* see g2p3_lpmDma.h for contract */
sbStatus_t
g2p3_lpmDmaUninit(g2p3_lpmDmaHandle_t *pDma)
{
    sbStatus_t status1, status2;
    g2p3_lpmDmaHandle_t dma;

    SB_ASSERT(pDma);
    dma = *pDma;
    SB_ASSERT(dma);
    status1 = SB_OK;
    if (dma->hostFree) {
        status1 = dma->hostFree(dma->hostFreeToken, SB_ALLOC_LPM_DMA,
                                dma->slabWords * sizeof(uint32_t),
                                dma->slabOp->slab, dma->slabOp->dmaHandle);
    }
    status2 = g2p3_lpmHostMemFree(dma->hostMem, dma, dma->hostAllocBytes);
    *pDma = NULL;

    return (status1 != SB_OK ? status1 : status2);
}
