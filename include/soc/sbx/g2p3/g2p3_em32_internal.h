/* -*- mode:c++; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=4 cinoptions=\:0l1(0t0g0: */

#ifndef _SB_G2P3_EM32_INT_H
#define _SB_G2P3_EM32_INT_H
/******************************************************************************
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
 * g2p3_em32_internal.h : 32-bit exact match compiler, internal defs
 *
 * $Id: g2p3_em32_internal.h 1.11.106.1 Broadcom SDK $
 *
 ******************************************************************************/
#include <soc/sbx/fe2k_common/sbFeISupport.h>
#include "sbFe2000UcodeLoad.h"

/* Max. payload size (our estimate - merely to avoid variable-sized array
 * when adding/updating payloads. This is size in bytes.
 */
#define MAX_PAYLOAD_BYTES 16

/* Type of flush performed - word-at-a-time or max. sized chunks of table */
typedef enum emFlushMode_e {
  emFlushModeSingleEntry, emFlushModeTable,
  emFlushModeCount
} emFlushMode_t;

/* commit type - done for init or regular commit */
typedef enum emCommitType_e {
  emCommitTypeInit, emCommitTypeRegular,
  emCommitTypeCount
} emCommitType_t;

/* Order in which to flush tables to the device */
typedef enum emCommitState_e {
  emCommitStateStart,
  emCommitStateLeft,
  emCommitStateRight,
  emCommitStateSeed,
  emCommitStateCount,
} emCommitState_t;

/* 2 sets of tables in use */
typedef enum {
  emBankStart,
  emBankBottom = emBankStart,
  emBankTop,
  emBankCount
} emBank_t;

/* 2 tables per set */

typedef enum {
  emTableStart,
  emTableLeft = emTableStart, 
  emTableRight,
  emTableCount
} emTable_t;

typedef struct g2p3_em32CompState_s *g2p3_em32CompState_p_t; 

void
g2p3_em32Flush (g2p3_em32CompState_p_t emComp);

void
g2p3_em32HwInitCont(sbFe2000DmaSlabOp_t *slabOp);


void
g2p3_em32CommitCont (sbFe2000DmaSlabOp_t *slabOp);



/* After this many moves (bumps out) a rehash is forced */
#define MAX_MOVES 1000

/* After this many unsuccessful rehashes add returns an error */
#define MAX_REHASH_TRIES 1000

#define lowidxbits(c) (c->lowIdxBits)
#define lowcrcbits(c) (32 - lowidxbits(c))
#define lowidxmask(c) ((1 << lowidxbits(c)) - 1)
#define topcrcmask(c) (~lowidxmask(c))
#define lowcrcmask(c) ((1 << lowcrcbits(c)) - 1)
#define topidxmask(c) (~lowcrcmask(c))
  
#define LOWIDX(c,x) (x & lowidxmask(c))
#define TOPCRC(c,x) ((x & topcrcmask(c)) >> lowidxbits(c))
#define LOWCRC(c,x) (x & lowcrcmask(c))
#define TOPIDX(c,x) ((x & topidxmask(c)) >> lowcrcbits(c))
#define DEVICE_TABLE_ENTRY(c, hcrc, idx) (((hcrc) << lowidxbits(c)) | (idx & lowidxmask(c)))
#define LEFTDEVTABLEENTRY(c,crc,idx)  DEVICE_TABLE_ENTRY(c, ((crc & topcrcmask(c)) >> lowidxbits(c)), idx)
#define RIGHTDEVTABLEENTRY(c,crc,idx) DEVICE_TABLE_ENTRY(c, (crc & lowcrcmask(c)), idx)
#define DEVTABLEIDX(c,x) (x & lowidxmask(c))
#define DEVTABLECRC(c,x) TOPCRC(c,x)
#define LEFTCRC(c,hc,idx)  ((((hc) & lowcrcmask(c))  << lowcrcbits(c)) | ((idx) & lowcrcmask(c)))
#define RIGHTCRC(c,hc,idx) ((((idx) & lowcrcmask(c)) << lowcrcbits(c)) | ((hc) & lowcrcmask(c)))

/* Extract seed from seed/bank word */
#define SEEDMASK 0x7FFFFFFF

/* Extract/set bank bit from seed/bank word */
#define BANKBIT  0x80000000

/* accounting bits for each left/right and payload entry */
typedef struct auxEntry_s { 
  uint8_t              paylSlotUsed;
  uint8_t              paylDirty;

  struct {
    uint8_t    valid;
    uint8_t    dirty[emBankCount];
  } tableState[emTableCount];
} auxEntry_t, *auxEntry_p_t;

/* we keep track of keys and assoc. payload slots */
typedef struct keyIdx_s {
  uint32_t key;
  uint16_t paylIdx;
  uint8_t  valid;
} keyIdx_t, *keyIdx_p_t;

typedef struct em32DmaTableInfo_s {
  struct {
    sbFe2000DmaAddr_t base;
    uint32_t          bank;
  } key, payload;
} em32DmaTableInfo_t;

/* Internal SVid compiler state */
typedef struct g2p3_em32CompState_s {

  em32DmaTableInfo_t   dmaInfo[emTableCount];
  uint32_t             bankOffset[emBankCount];
  em32DmaTableInfo_t   dmaDupInfo[emTableCount];
  uint8_t              useDupBank;

  g2p3_em32KeyValid_f_t keyValid_f;
  g2p3_em32SetCuckooSeed_f_t cuckooSeedSet;
  g2p3_em32GetCuckooSeed_f_t cuckooSeedGet;

  emBank_t           currBank; 
  uint32_t           currSeed;
  uint8_t            seedBankDirty;

  uint32_t           preRehashSeed;
  uint8_t            preRehashSeedBankDirty;

  uint32_t lowIdxBits;

  uint32_t tableSize;
  uint32_t *leftKey;
  uint32_t *rightKey;
  
  auxEntry_t *auxEntries;

  /* software copy of key table; 2B of CRC, 2B of Payload.
   * CRC is in host format; must be converted to device format (BE)
   * payload is already formatted for the device by the caller */
  uint32_t *deviceKeyTable[emTableCount];
  uint32_t *keyTableCopy[emTableCount];
  
  /* software copy of table data; in device format */
  uint8_t  *devicePayloads;
  uint16_t  payloadSize;
  
  /* Key & Payload data, formatted for DMA */
  uint32_t *dmaKeyTable[emTableCount];
  uint32_t *dmaPayloadTable[emTableCount];
  uint32_t *dmaDupKeyTable[emTableCount];


  uint32_t paylFreeSlot;

  uint16_t totEntries;
  uint16_t maxEntries;
  uint16_t dirtyPaylEntries;
  uint16_t cutoffTotalFlush;
  uint16_t dirtyEntries[emTableCount][emBankCount];
  uint8_t  forceFlush[emBankCount]; /* Force flush on top/ or bottom 
                                     * of current bank */

  sbFe2000DmaSlabOp_t *slabOp;

  sbFeInitAsyncCallback_f_t initCb;
  void *initCbArg;

  sbFeAsyncCallback_f_t asyncCb;
  sbFeAsyncCallbackArgument_p_t asyncCbArg;

  uint32_t slabWords;
  uint8_t  busy;
  void *dmaReqToken;
  sbStatus_t status;

  emCommitState_t commitState;
  uint32_t currFlushTablePos;

  emFlushMode_t flushMode;
  emCommitType_t commitType;

  sbCommonConfigParams_p_t pCommParams;

  void *pDmaCtxt;
  soc_sbx_g2p3_state_t *pFe;
  uint32_t async;
} g2p3_em32CompState_t;

#endif
