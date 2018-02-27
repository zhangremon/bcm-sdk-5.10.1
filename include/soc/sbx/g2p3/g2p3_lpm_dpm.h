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

  $Id: g2p3_lpm_dpm.h 1.3.214.1 Broadcom SDK $

 *****************************************************************************/

#ifndef _G2P3_LPM_DPM_H_
#define _G2P3_LPM_DPM_H_

#include <soc/sbx/sbTypes.h>
#include <soc/sbx/sbStatus.h>
#include "g2p3_lpm_types.h"
#include "g2p3_lpm_host_mem.h"
#include "g2p3_lpm_device_mem.h"
#include "g2p3_lpm_dma.h"
#include "g2p3_lpm_hpm.h"

/* DEVICE PAYLOAD MANAGER
 *
 * Allocates (in device memory) and DMAs device payloads
 * Each device payload is a triple <main,clsA,clsB>, comprising main
 * payload (main) and two class identifiers (clsA, clsB);
 * the DPM calls a user-provided pack() function to convert the triple
 * to a device-representation byte stream
 * Existing payload triples are reused, new ones allocated in device memory;
 * device memory of each triple is deallocated when the ref count reaches 0
 */

/* abstract device payload manager handle */
typedef struct g2p3_lpmDPM_s *g2p3_lpmDPMHandle_t;

/* requires: pDpm, pack() non-null; hostMem, deviceMem, dma initialized
 *           dmaBank is a valid DMA bank; wordsPerDevicePayload > 0
 * modifies: *pDpm, allocates in hostMem
 * effects:  creates new manager for payloads wordsPerDevicePayload each
 *           in deviceMem/dma/dmaBank
 * returns:  SB_OK on success; error status otherwise
 */
sbStatus_t
g2p3_lpmDPMInit(g2p3_lpmDPMHandle_t *pDpm,
                 g2p3_lpmHostMemHandle_t hostMem,
                 g2p3_lpmDeviceMemHandle_t deviceMemA,
                 g2p3_lpmDeviceMemHandle_t deviceMemB,
                 g2p3_lpmDmaHandle_t dma,
                 uint32_t paylDmaBankA, uint32_t paylDmaBankB, uint32_t paylBanks,
                 g2p3_lpmPayloadPack_f_t pack,
                 size_t wordsPerDevicePayload,
                 int unit);

/* requires: dpm initialized; no commit in progress
 * modifies: *dpm; allocates in hostMem and deviceMem given at creation time
 * effects:  references <payload,clsA,clsB> and allocates device memory for it
 *           if it has never been referenced before
 * returns:  SB_OK on success
 *           SB_BUSY_ERR_CODE if commit is in progress
 */
sbStatus_t
g2p3_lpmDPMRef(g2p3_lpmDPMHandle_t dpm,
                   g2p3_lpmPayloadHandle_t payload,
                   g2p3_lpmClassifierId_t clsA,
                   g2p3_lpmClassifierId_t clsB,
                   uint32_t dropMaskLo, uint32_t dropMaskHi);

/* requires: dpm initialized; <payload,clsA,clsB> in dpm; no commit in progress
 * modifies: *pAddress, *pExtraBits
 * effects:  sets *pAddress to device memory address of <payload,clsA,clsB>
 *           sets *pExtraBits to extra bits to store in payload node
 * returns:  SB_OK on success
 *           SB_LPM_ADDRESS_NOT_FOUND if <payload,clsA,clsB> not in dpm
 *           SB_BUSY_ERR_CODE if commit is in progress
 */
sbStatus_t
g2p3_lpmDPMInfo(g2p3_lpmDPMHandle_t dpm,
                    g2p3_lpmSramAddr *pAddress,
                    uint16_t *pExtraBits,
                    g2p3_lpmPayloadHandle_t payload,
                    g2p3_lpmClassifierId_t clsA,
                    g2p3_lpmClassifierId_t clsB,
                    uint32_t dropMaskLo, uint32_t dropMaskHi);

/* requires: dpm initialized; <payload,clsA,clsB> in dpm; no commit in progress
 * modifies: *dpm; deallocates in hostMem and deviceMem given at creation time
 * effects:  dereferences <payload,clsA,clsB> and schedules its device memory
 *           for deallocation if the ref count has reached zero
 * returns:  SB_OK on success
 *           SB_LPM_ADDRESS_NOT_FOUND if <payload,clsA,clsB> not in dpm
 *           SB_BUSY_ERR_CODE if commit is in progress
 */
sbStatus_t
g2p3_lpmDPMDeref(g2p3_lpmDPMHandle_t dpm,
                     g2p3_lpmPayloadHandle_t payload,
                     g2p3_lpmClassifierId_t clsA,
                     g2p3_lpmClassifierId_t clsB,
                     uint32_t dropMaskLo, uint32_t dropMaskHi);

/* requires: dpm initialized; no commit in progress
 * modifies: *dpm; device memory
 * effects:  writes new payloads to device memory
 *           if asyncCb() non-null, calls asyncCb() with asyncCbUserData on completion
 *           if and only if the DMAs were asynchronous
 * returns:  SB_OK on success
 *           SB_IN_PROGRESS if operation will complete asynchronously
 *           SB_BUSY_ERR_CODE if a commit is already in progress
 */
sbStatus_t
g2p3_lpmDPMCommit(g2p3_lpmDPMHandle_t dpm,
                      g2p3_lpmCallback_f_t asyncCb,
                      void *asyncCbUserData);

/* requires: *pDpm initialized
 * modifies: *pDpm; deallocates in hostMem given at creation time
 * effects:  destroys and frees all memory used by manager at *pDpm
 *           sets *pDpm to null
 * returns:  SB_OK on success; error status otherwise
 */
sbStatus_t
g2p3_lpmDPMUninit(g2p3_lpmDPMHandle_t *pDpm);

#endif
