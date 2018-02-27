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

  $Id: g2p3_lpm_dma.h 1.3.214.1 Broadcom SDK $

 *****************************************************************************/

#ifndef _G2P3_LPM_DMA_H_
#define _G2P3_LPM_DMA_H_

#include <soc/sbx/sbTypes.h>
#include <soc/sbx/sbStatus.h>
#include <soc/sbx/sbWrappers.h>
#include <soc/sbx/fe2k_common/sbFeISupport.h>
#include <soc/sbx/fe2k_common/sbFe2000DmaMgr.h>
#include "g2p3_lpm_types.h"
#include "g2p3_lpm_host_mem.h"
#include "g2p3_lpm_device_mem.h"

/* DMA MANAGER
 *
 * Accepts, buffers, and executes requests for device memory writes, freeing
 * the client from worrying about slabs and other details of DMA.
 * Writes are flushed when the buffers fill up or on g2p3_lpmDmaFlush().
 *
 * ATOMICITY
 *
 * Consider two write requests, w1 and w2, such that g2p3_lpmDmaWrite()
 * of w1 happened BEFORE g2p3_lpmDmaWrite() of w2;
 * then, if level(w2) > level (w1), w2 is guaranteed to be written AFTER w1,
 * and a SYNC is guaranteed between w1 and w2.
 */

/* abstract DMA manager handle */
typedef struct g2p3_lpmDma_s* g2p3_lpmDmaHandle_t;

/* requires: pDma, hostAlloc(), dmaSlabReq() non-null; fe, hostMem initializd
 * modifies: *pDma; allocates in hostMem and (DMAble memory) using hostAlloc
 * effects:  creates new DMA manager and writes its handle to *pDma
 * returns:  SB_OK on success; error status otherwise
 */
sbStatus_t
g2p3_lpmDmaInit(g2p3_lpmDmaHandle_t *pDma,
                    g2p3_lpmHostMemHandle_t hostMem,
                    sbMalloc_f_t hostAlloc, void *hostAllocToken,
                    sbFree_f_t hostFree, void *hostFreeToken,
                    void *pDmaCtxtP,
                    void *fe, void *dmaReqToken,
                    uint32_t maxReqs, uint32_t slabWords);

/* requires: dma initialized; address and (address + nWords - 1) in SRAM;
 *           nWords > 0
 * modifies: *dma
 * effects:  buffers a new write request: [address, address+nWords) <- datum 
 *           buffers are flushed if they've become full
 *           if asyncCb() non-null, calls asyncCb() with asyncCbUserData
 *           on completion iff flushing was required and asynchronous
 *           see also ATOMICITY above
 * returns:  SB_OK on success
 *           SB_IN_PROGRESS if operation will complete asynchronously
 *           SB_BUSY_ERR_CODE if a flush is already in progress
 */
sbStatus_t
g2p3_lpmDmaWrite(g2p3_lpmDmaHandle_t dma,
                     sbDmaAddress_t address, size_t nWords,
                     uint32_t datum, uint8_t level,
                     g2p3_lpmCallback_f_t asyncCb,
                     void *asyncCbUserData);

/* requires: dma initialized
 * modifies: *dma, device memory
 * effects:  writes current buffers to device memory
 *           calls asyncCb() with asyncCbUserData on completion
 *           if and only if DMA operation was asynchronous
 * returns:  SB_OK on success
 *           SB_IN_PROGRESS if operation will complete asynchronously
 *           SB_BUSY_ERR_CODE if a flush is already in progress
 */
sbStatus_t
g2p3_lpmDmaFlush(g2p3_lpmDmaHandle_t dma,
                     g2p3_lpmCallback_f_t asyncCb,
                     void *asyncCbUserData);

/* requires: *pDma initialized
 * modifies: *pDma; deallocates in hostMem given at creation time
 * effects:  destroys and frees all memory used by manager at *pDma
 *           sets *pDma to null
 * returns:  SB_OK on success; error status otherwise
 */
sbStatus_t
g2p3_lpmDmaUninit(g2p3_lpmDmaHandle_t *pDma);

#endif
