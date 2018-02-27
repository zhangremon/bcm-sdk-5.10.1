/* -*- mode:c; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=2 cinoptions=\:0l1(0t0g0: */

#ifndef _G2P3_LPM_H_
#define _G2P3_LPM_H_

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

  $Id: g2p3_lpm.h 1.8.110.1 Broadcom SDK $

 *****************************************************************************/

#include "sbStatus.h"
#include "sbTypes.h"
#include "sbFeISupport.h"
#include "g2p3.h"
#include "g2p3_lpm_types.h"
#include "g2p3_lpm_host_mem.h"
#include "g2p3_lpm_device_mem.h"
#include "g2p3_lpm_dma.h"
#include "g2p3_lpm_hpm.h"
#include "g2p3_lpm_dpm.h"

/* LPM (generic) compiler - internal API used by higher layer (ilib).
 *
 * Manages longest-prefix-match to payload database
 * Accepts updates and writes lookup tables to device SRAM
 */

/* address type */
typedef uint64_t g2p3_lpmAddress_t;

/* LPM walk handle */
typedef struct g2p3_lpmWalkHandle_s *g2p3_lpmWalkHandle_t;

/* abstract LPM compiler handle */
typedef struct g2p3_lpm_s* g2p3_lpmHandle_t;

/* types of payloads: main (route etc), classifier class IDs (A and
 * B), ipv6 'EM separator' (C), drop mask (M).
 */
typedef enum {
    G2P3_LPM_PAYLOAD_MAIN,
    G2P3_LPM_PAYLOAD_CLS_A,
    G2P3_LPM_PAYLOAD_CLS_B,
    G2P3_LPM_PAYLOAD_DROPM_LO,
    G2P3_LPM_PAYLOAD_DROPM_HI,
    G2P3_LPM_PAYLOAD_CLS_C,
    G2P3_LPM_PAYLOAD_NUM_KINDS /* keep as last entry */
} g2p3_lpmPayloadKind_t;

/* requires: pLpm non-null; hostMem, deviceMem, dma initialized;
 *           slices[] contains zero-terminated slice table;
 *           forall s in slices: 0 < s <= 32;
 *           sum slices == address length;
 *           table1Start, table1Start + (1 << slices[0]) in deviceMem;
 *           defaultPayload[0..bytesPerPayload-1] contains payload;
 *           payloadPack() non-null and converts bytesPerPayload contiguous
 *           bytes to wordsPerDevicePayload contiguous words
 *           optSingleton enables singleton-table optimization
 * modifies: *pLpm; allocates in hostMem and deviceMem
 * effects:  creates new LPM compiler and writes its handle to *pLpm
 *           slices[] is copied and does not need to be preserved after init()
 * returns:  SB_OK on success; error status otherwise
 */
sbStatus_t
g2p3_lpmInit(soc_sbx_g2p3_state_t *fe,
              g2p3_lpmHandle_t *pLpm,
              g2p3_lpmHostMemHandle_t hostMem,
              g2p3_lpmDeviceMemHandle_t deviceMemA,
              g2p3_lpmDeviceMemHandle_t deviceMemB,
              g2p3_lpmDmaHandle_t dma, uint32_t dmaBankA, uint32_t dmaBankB,
              uint32_t paylDmaBankA, uint32_t paylDmaBankB, uint32_t paylBanks,
              g2p3_lpmSramAddr table1Start,
              uint8_t slices[],
              void *defaultPayload,
              g2p3_lpmClassifierId_t defaultClsA,
              g2p3_lpmClassifierId_t defaultClsB,
              uint32_t defaultDropMaskLo,
              uint32_t defaultDropMaskHi,
              size_t bytesPerPayload,
              size_t wordsPerDevicePayload,
              g2p3_lpmPayloadPack_f_t payloadPack,
              sbBool_t optSingleton,
              sbBool_t ipv6);

/* requires: *pLpm previously initialized
 * modifies: *pLpm; deallocates in hostMem given at creation time
 * effects:  destroys and frees LPM compiler *pLpm; sets *pLpm to null
 * returns:  SB_OK on success; error status otherwise
 */
sbStatus_t
g2p3_lpmUninit(g2p3_lpmHandle_t *pLpm);

/* requires: lpm previously initialized; address/prefixLen not in lpm
 * modifies: lpm
 * effects:  inserts payload of payloadKind at prefix address/prefixLength
 *           copies bytesPerPayload bytes at payload into internal storage
 * returns:  SB_OK on success
 *           SB_LPM_DUPLICATE_ADDRESS if address/prefixLen in lpm
 *           SB_LPM_OUT_OF_HOST_MEMORY
 *           SB_LPM_OUT_OF_DEVICE_MEMORY
 */
sbStatus_t
g2p3_lpmAdd(g2p3_lpmHandle_t lpm,
             g2p3_lpmAddress_t address,
             uint8_t prefixLength,
             void *payload, 
             g2p3_lpmPayloadKind_t payloadKind,
             uint8_t l3cached);

sbStatus_t
g2p3_lpmAddC(g2p3_lpmHandle_t lpm, 
              g2p3_lpmAddress_t addr,
              uint8_t length, 
              void **payMain, 
              g2p3_lpmClassifierId_t *pClsA,
              g2p3_lpmClassifierId_t *pClsB, 
              uint32_t *pDropMaskLo,
              uint32_t *pDropMaskHi, 
              g2p3_lpmClassifierId_t *pClsC);

sbStatus_t
g2p3_lpmMaskClear(g2p3_lpmHandle_t lpm, 
                   g2p3_lpmAddress_t addr,
                   uint8_t length);


/* requires: lpm previously initialized; address/prefixLen in lpm
 * modifies: lpm
 * effects:  replaces payload of payloadKind at prefix address/prefixLength
 *           with newPayload
 *           copies bytesPerPayload bytes at newPayload into internal storage
 * returns:  SB_OK on success
 *           SB_LPM_ADDRESS_NOT_FOUND if address/prefixLen not in lpm
 *           SB_LPM_OUT_OF_HOST_MEMORY
 *           SB_LPM_OUT_OF_DEVICE_MEMORY
 */
sbStatus_t
g2p3_lpmUpdate(g2p3_lpmHandle_t lpm,
                g2p3_lpmAddress_t address,
                uint8_t prefixLength,
                void *newPayload, 
                g2p3_lpmPayloadKind_t payloadKind,
                uint8_t l3cached);

/* requires: lpm previously initialized; address/prefixLen in lpm
 * modifies: lpm
 * effects:  removes payload of payloadKind at prefix address/prefixLength
 *           copies bytesPerPayload bytes at newPayload into internal storage
 * returns:  SB_OK on success
 *           SB_LPM_ADDRESS_NOT_FOUND if address/prefixLen not in lpm
 */
sbStatus_t
g2p3_lpmRemove(g2p3_lpmHandle_t lpm,
                g2p3_lpmAddress_t address,
                uint8_t prefixLength,
                g2p3_lpmPayloadKind_t payloadKind,
                uint8_t l3cached);

/* requires: lpm previously initialized
 * modifies: lpm, device SRAM
 * effects:  compiles prefix tree in memory into a device representation
 *           and copies it to device SRAM
 *           calls asyncCb() with asyncCbUserData on completion
 *           if and only if commit operation was asynchronous
 * returns:  SB_OK on success
 *           SB_MORE if partial commit completed; call again to continue
 *           SB_IN_PROGRESS if operation will complete asynchronously
 *           SB_BUSY if a commit is already in progress
 */
sbStatus_t
g2p3_lpmCommit(g2p3_lpmHandle_t lpm,
                g2p3_lpmCallback_f_t asyncCb,
                void *asyncCbUserData);

/* requires: lpm previously initialized; address/prefixLen in lpm
 * modifies: *pResult
 * effects:  finds payload of payloadKind at prefix address/prefixLength
 *           stores pointer to internal storage in *pResult
 * returns:  SB_OK on success
 *           SB_LPM_ADDRESS_NOT_FOUND if address/prefixLen not in lpm
 */
sbStatus_t
g2p3_lpmFind(g2p3_lpmHandle_t lpm,
              void **pResult,
              g2p3_lpmAddress_t address,
              uint8_t prefixLength,
              g2p3_lpmPayloadKind_t payloadKind);

extern sbStatus_t
g2p3_lpmWalkReset(g2p3_lpmHandle_t lpm, g2p3_lpmWalkHandle_t *WalkHandle);

sbStatus_t
g2p3_lpmWalk(g2p3_lpmHandle_t lpm,
              g2p3_lpmAddress_t curraddr,
              uint8_t currlenlength,
              g2p3_lpmAddress_t *ncurraddr,
              uint8_t *nlength,
              g2p3_lpmPayloadKind_t *pKind,
              void **payMain,
              uint32_t *pDropMaskLo,
              uint32_t *pDropMaskHi,
              g2p3_lpmClassifierId_t *pClsA,
              g2p3_lpmClassifierId_t *pClsB,
              g2p3_lpmClassifierId_t *pClsC,
              g2p3_lpmWalkHandle_t pDesc);

/* requires: lpm previously initialized
 * modifies: lpm->trie
 * effects:  remove all prefixes (incl. length 0), set lpm->trie to NULL
 * returns:  SB_OK on success
 */
sbStatus_t
g2p3_lpmFlush(g2p3_lpmHandle_t lpm);

/* Testing only
 * Turn on/off singleton optimization
 */
extern void g2p3_lpmSingletonSet(g2p3_lpmHandle_t lpm, int flag);

/* LPM statistics */
typedef enum {
    G2P3_LPM_STATS_TYPE_ADD = 0,  /* Number of Route Adds    */ 
    G2P3_LPM_STATS_TYPE_DEL,      /* Number of Route Deletes */
    G2P3_LPM_STATS_TYPE_UPDATE,   /* Number of Route Updates */
    G2P3_LPM_STATS_TYPE_COMMIT,   /* Number of Route Commits */ 
    G2P3_LPM_STATS_TYPE_MAX 
}lpmStatsType_t;

typedef enum {
    G2P3_LPM_STATS_IDX      = 0,  
    G2P3_LPM_STATS_ROLL_IDX,  
    G2P3_LPM_STATS_MAX_IDX
}lpmStatsIdx_t;

typedef enum {
    G2P3_LPM_STATS_UPD_REQ = 0,  
    G2P3_LPM_STATS_UPD_SUCCESS,
    G2P3_LPM_STATS_UPD_MAX
}lpmStatsUpdateType_t;

typedef struct {
  uint64_t count;
  uint64_t rollcount;
} lpmStats_t;

void 
g2p3_lpm_stats_clear(g2p3_lpmHandle_t lpm);

sbStatus_t 
g2p3_lpm_stats_get(g2p3_lpmHandle_t lpm,
                   lpmStatsType_t   type,
                   lpmStatsUpdateType_t uType,
                   sbBool_t         clear,
                   lpmStats_t       *count);

sbStatus_t
g2p3_lpm_stats_update(g2p3_lpmHandle_t lpm, 
                      lpmStatsType_t   type,
                      lpmStatsUpdateType_t uType);

void 
g2p3_lpm_stats_dump(g2p3_lpmHandle_t lpm,
                         sbBool_t verbose);

void
g2p3_lpmHostMemUsage(g2p3_lpmHostMemHandle_t pHostMem);
#endif
