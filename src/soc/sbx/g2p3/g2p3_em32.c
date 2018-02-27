/* -*- mode:c++; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=4 cinoptions=\:0l1(0t0g0: */
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
 * g2p3_em32.c: 32-bit key exact match compiler (using
 *              cuckoo hashing scheme).
 *
 * $Id: g2p3_em32.c 1.27.60.1 Broadcom SDK $
 *
 *****************************************************************************/

#include <soc/sbx/g2p3/g2p3_int.h>

#include <soc/sbx/g2p3/g2p3_em32.h>
#include <soc/sbx/g2p3/g2p3_em32_internal.h>

#include <soc/sbx/fe2k_common/sbCrc32.h>

#include <soc/debug.h>
#define INTR_DEBUG(stuff)  SOC_DEBUG(SOC_DBG_INTR, stuff)
#define EM_DEBUG(stuff)  SOC_DEBUG(SOC_DBG_MEM, stuff)

/* Check top and bottom banks within the table for the given index,
 * mark as dirty if not already.
 */
#define TABLE_DIRTY_CHECK(em_, table_, idx_) \
do { \
  emBank_t bank_ = emBankStart;                                               \
  for (; bank_ < emBankCount; bank_++) {                                      \
      if ((em_)->auxEntries[(idx_)].tableState[(table_)].dirty[bank_] == 0) { \
        (em_)->dirtyEntries[(table_)][bank_]++;                               \
        (em_)->auxEntries[(idx_)].tableState[(table_)].dirty[bank_] = 1;      \
      }                                                                       \
    }                                                                         \
} while(0)




/*  g2p3_em32Recover
 * Recover the soft state for an instance of the EM32 compiler.  Recovery is
 * perfomed by reading the left and right tables from chip memory and 
 * reconstructing the hashes, and keys to generate the hashes.
 */
sbStatus_t
g2p3_em32Recover(void *em32Ctxt)
{
  int rv;
  uint32 seed, buffer_size, key, crc = 0, slot;
  uint32 *lr_key[emTableCount];
  g2p3_em32CompState_p_t pEm32 = (g2p3_em32CompState_p_t)em32Ctxt;
  uint8 *dma_buffer, *key_table, *payload_table;
  sbFe2000DmaOp_t dma_op;
  int idx;
  int unit = pEm32->pFe->unit;
  emTable_t table;
  emBank_t bank;
  uint8 *psrc, *pdest;
  uint16 half_crc;

  seed = 0;
  if (pEm32->cuckooSeedGet) {
      rv = pEm32->cuckooSeedGet(pEm32->pFe->unit, &seed);
  } else {
      rv = SB_INVALID_HANDLE;
  } 
  if (SOC_FAILURE(rv)) {
    SOC_DEBUG_PRINT((DK_ERR, "Failed to recover cuckoo seed: %d\n", rv));
    return SB_NOT_FOUND;
  }

  lr_key[emTableLeft]  = pEm32->leftKey;
  lr_key[emTableRight] = pEm32->rightKey;

  pEm32->currBank = (seed & BANKBIT) ? emBankTop : emBankBottom;
  pEm32->currSeed = seed & SEEDMASK;

  /* allocate one buffer for both the table and payload data */
  buffer_size = 2 * pEm32->tableSize * sizeof(uint32_t);
  dma_buffer = soc_cm_salloc(unit, buffer_size, "dma buffer");
  if (dma_buffer == NULL) {
    return SB_MALLOC_FAILED;
  }

  key_table = dma_buffer;
  payload_table = dma_buffer + (buffer_size >> 1);

  bank = pEm32->currBank;
  for (table = emTableStart; table < emTableCount; table++) {

    EM_DEBUG(("%s: Recovering %s table...\n", FUNCTION_NAME(),
             (table == emTableLeft) ? "Left" : "Right"));

    if (table == emTableRight) {
      seed = pEm32->currSeed;
    } else {
      seed = 0;
    }

    /* DMA the entire Key table from the device.  The Key table contains
     * 16bits of the CRC, and 16bits of payload
     */
    sal_memset(&dma_op, 0, sizeof(dma_op));
    dma_op.hostAddress    = (uint32 *) key_table;
    dma_op.hostBusAddress = soc_cm_l2p(unit, key_table);
    dma_op.feAddress = (uint32)
      SB_FE2000_DMA_MAKE_ADDRESS (pEm32->dmaInfo[table].key.bank,
                                  (pEm32->dmaInfo[table].key.base + 
                                   pEm32->bankOffset[bank]));
    
    dma_op.opcode = SB_FE2000_DMA_OPCODE_READ;
    dma_op.words = pEm32->tableSize;
    rv = sbFe2000DmaRequest(pEm32->pDmaCtxt, INT_TO_PTR(unit), &dma_op);
    if (SOC_FAILURE(rv)) {
      soc_cm_print("Failed to DMA key table %d/%d: %s\n",
                   table, bank, soc_errmsg(rv));
      goto done;
    }
    
    /* DMA the entire Payload table from the device.  The Payload table
     * contains the remaining 32bits of payload.
     */
    dma_op.hostAddress    = (uint32 *) payload_table;
    dma_op.hostBusAddress = soc_cm_l2p(unit, payload_table);
    dma_op.feAddress = (uint32)
      SB_FE2000_DMA_MAKE_ADDRESS (pEm32->dmaInfo[table].payload.bank,
                                  (pEm32->dmaInfo[table].payload.base + 
                                   pEm32->bankOffset[bank]));
    rv = sbFe2000DmaRequest(pEm32->pDmaCtxt, INT_TO_PTR(unit), &dma_op);
    if (SOC_FAILURE(rv)) {
      soc_cm_print("Failed to DMA payload table %d/%d: %s\n",
                   table, bank, soc_errmsg(rv));
      goto done;
    }

    /* Iterate over each entry in the Key table:
     *   1. Reconstruct the CRC from the index and the in the entry
     *   2. Compute the Key using a reverse CRC
     *   3. Fill in necessary tables if Key is valid
     */
    for (idx=0; idx < dma_op.words; idx++) {
      
      /* device key_table[i] = [{payload[31:16]},{crc}]  */
      psrc = &key_table[idx * 4];
      half_crc = psrc[3] << 8 | psrc[2];
      
      switch (table) {
      case emTableLeft : crc = LEFTCRC(pEm32, half_crc, idx); break;
      case emTableRight: crc = RIGHTCRC(pEm32, half_crc, idx); break;
      default: assert(0);
      }

      key = sbCrc32_reverse(crc) - seed;

      /* When a valid key is found, update the following software only data:
       * - deviceKeyTable
       * - devicePayloads
       * - {left,right}Key
       */
      if (pEm32->keyValid_f(key)) {

        pEm32->totEntries++;
        slot = pEm32->paylFreeSlot++;

        pEm32->auxEntries[slot].paylSlotUsed = 1;
        pEm32->deviceKeyTable[table][idx] =
          DEVICE_TABLE_ENTRY(pEm32, half_crc, DEVTABLEIDX(pEm32, slot));

        psrc  = &key_table[idx * 4];
        pdest = &pEm32->devicePayloads[slot * pEm32->payloadSize];
 
        /* the payload is split between 2bytes in the key table,
         * and 4 bytes in the payload table.  The soft version of
         * devicePayloadTable and indirection from key[i].payloadIndex 
         * to payload table is an artifact of a legacy implementation.
         */
        pdest[4+0] = psrc[0];
        pdest[4+1] = psrc[1];

        psrc  = &payload_table[idx * 4];
        sal_memcpy (pdest, psrc, 4);

        /* Store the software copy of the key */
        lr_key[table][idx] = key;
      }
    }
  }

done:
  soc_cm_sfree(unit, dma_buffer);

  return rv;
}


/* g2p3_em32ComputeHashedInvalidKey
 * 
 * Compute an appropriate invalid hashed key for the given index and table.
 * Used for initializing left and right tables such that no key will ever
 * accidentially hit.  Computing such a key saves instructions in ucode from
 * checking a valid bit.
 */
static uint32
g2p3_em32ComputeHashedInvalidKey(g2p3_em32CompState_p_t pEm32, 
                                 uint32 idx, int right) 
{
  uint32 key, key_try, key_found;
  uint32 result, seed = 0;
  uint8 result_byte[4];
  int idx_base, payload_base;

  /* Define pivot point of segmenting the hash key for table index and data
   * LeftTable 
   *  idx = hash_key[15:0]; left_table[idx] = {payload, hash_key[31:16]} 
   * RightTable 
   *  idx = hash_key[31:16]; right_table[idx] = {payload, hash_key[15:0]} 
   */
  if (right) {
    seed = pEm32->currSeed;
    idx_base = 0;
    payload_base = 2;
  } else {
    seed = 0;
    idx_base = 2;
    payload_base = 0;
  }
  
  result_byte[idx_base]     = (idx >> 8) & 0xff;
  result_byte[idx_base + 1] = (idx >> 0) & 0xff;
  
  key_found = 0;
  for (key_try=0; key_try < pEm32->tableSize; key_try++) {
    result_byte[payload_base]     = (key_try >>  8) & 0xff;
    result_byte[payload_base + 1] = (key_try >>  0) & 0xff;
    
    result = ((result_byte[0] << 24) + (result_byte[1] << 16) +
              (result_byte[2] << 8) + result_byte[3]);
    
    key = sbCrc32_reverse(result) - seed;

    if (!pEm32->keyValid_f(key)) {
      return result;
    }
  }
  
  /* no invalid key found, instantiator MUST reconstruct key format
   * to allow for some set of invalid keys for this instance, or supply
   * a keyValid_f to report some set of keys as INVALID.  If not, the
   * ucode may return false positives.
   */
  assert(0);
  return 0;
}

/*
 * Function:
 *   g2p3_em32FindNextKey
 * Purpose:
 *   Find the next valid key given a current key expressed as <table, index>
 * Parameters:
 *    (in)     pEm32      - em32 context
 *    (in)     keyTable   - left and right tables to search
 *    (in)     seed       - seed to use for hash
 *    (in/out) table      - (in) table in keyTable to start search, 
 *                          (out) table in keyTable for next valid key
 *    (in/out) table_idx  - (in) index in keyTable to start search, 
 *                          (out) index in keyTable for next valid key
 *    (out)    next_key   - next valid key
 *
 * Returns:
 *    0   for success
 *   !0   for error (not found/ no more valid entries)
 *
 */
static int
g2p3_em32FindNextKey(g2p3_em32CompState_p_t pEm32,
                     uint32_t *keyTable[emTableCount],
                     uint32_t seed, 
                     emTable_t *table, uint32_t *table_idx,
                     uint32_t *next_key) 
{
  int32_t   theSeed, crc, half_crc, tmp_key;

  for (; *table < emTableCount; (*table)++) {
    
    if (*table == emTableRight) {
      theSeed = seed;
    } else {
      theSeed = 0;
    }
 
    for (; *table_idx < pEm32->tableSize; (*table_idx)++) {

      half_crc = DEVTABLECRC(pEm32, keyTable[*table][*table_idx]);
      switch (*table) {
      case emTableLeft : crc = LEFTCRC(pEm32, half_crc, *table_idx); break;
      case emTableRight: crc = RIGHTCRC(pEm32, half_crc, *table_idx); break;
      default: assert(0); return 1;
      }
     
      tmp_key = sbCrc32_reverse(crc) - theSeed;
     
      if (pEm32->keyValid_f(tmp_key)) {
        *next_key = tmp_key;
        return 0;
      }
    }

    /* reset idx for next table */
    *table_idx = 0;
  }
  
  return 1;
}

/*
 * Function:
 *   g2p3_em32KeyLookup
 * Purpose:
 *   Lookup table location for the given key
 * Parameters:
 *    (in)  pEm32      - em32 context
 *    (in)  keyTable   - left and right tables to search
 *    (in)  seed       - seed to use for hash
 *    (in)  key        - key to find
 *    (out) table      - table in keyTable where key is found
 *    (out) table_idx  - index in keyTable where key is found
 * Returns:
 *    0   match
 *   !0   for error (not found/ no more valid entries)
 *
 */
static int
g2p3_em32KeyLookup(g2p3_em32CompState_p_t pEm32,
                   uint32_t *keyTable[emTableCount],
                   uint32_t seed, uint32_t key, 
                   emTable_t *table, uint32_t *table_idx)
{
  uint32_t crc, half_crc, device_crc;

  /* does the key match left table? */
  crc = sbCrc32(key);
  *table_idx = LOWIDX(pEm32, crc);

  *table = emTableLeft;
  half_crc   = DEVTABLECRC(pEm32, keyTable[*table][*table_idx]);
  device_crc = LEFTCRC(pEm32, half_crc, *table_idx);
  
  if (device_crc != crc) {
    /* no match in left table, check the right table */

    crc = sbCrc32(key + seed);
    *table_idx = TOPIDX(pEm32, crc);
    
    *table = emTableRight;
    half_crc   = DEVTABLECRC(pEm32, keyTable[*table][*table_idx]);
    device_crc = RIGHTCRC(pEm32, half_crc, *table_idx);
  }

  return (device_crc != crc);
}


/*
 * g2p3_em32InitSoftTables
 *
 *  Initialize all soft-state left & right tables to invalid keys
 */
static int
g2p3_em32InitSoftTables(g2p3_em32CompState_p_t pEm32)
{
  int right, idx, hkey;

  right = 0;
  for (idx=0; idx < pEm32->tableSize; idx++) {
    hkey = g2p3_em32ComputeHashedInvalidKey(pEm32, idx, right);
    pEm32->leftKey[idx] = hkey;

    pEm32->deviceKeyTable[emTableLeft][idx] = 
      LEFTDEVTABLEENTRY(pEm32, hkey, 0);
  }

  right = 1;
  for (idx=0; idx < pEm32->tableSize; idx++) {
    hkey = g2p3_em32ComputeHashedInvalidKey(pEm32, idx, right);
    pEm32->rightKey[idx] = hkey;

    pEm32->deviceKeyTable[emTableRight][idx] =
      RIGHTDEVTABLEENTRY(pEm32, hkey, 0);
  }
  
  return 0;
}

sbStatus_t 
g2p3_em32Init (void*                       *em32Ctxt,
               soc_sbx_g2p3_state_t        *pFe,
               sbFe2000DmaMgr_t            *pDmaCtxt,
               sbCommonConfigParams_p_t     cParams,
               g2p3_em32KeyValid_f_t        keyValid_f,
               sbFe2000DmaAddr_t            leftBase,
               sbFe2000DmaAddr_t            rightBase,
               sbFe2000DmaAddr_t            paylBaseLeft,
               sbFe2000DmaAddr_t            paylBaseRight,
               uint32_t                     leftBank,
               uint32_t                     rightBank,
               uint32_t                     paylBankLeft, 
               uint32_t                     paylBankRight,
               uint32_t                     topOff,
               uint32_t                     tableSize,
               uint32_t                     paylSize,
               uint32_t                     slabSize,
               uint32_t                     lowIdxBits,
               uint8_t                      useDuplicateBank,
               uint32_t                     dupLeftBank,
               uint32_t                     dupRightBank,
               sbFe2000DmaAddr_t            dupLeftBase,
               sbFe2000DmaAddr_t            dupRightBase)
{
  sbStatus_t status;
  void *vp;
  g2p3_em32CompState_p_t pEm32Comp;
  uint32_t tableBytes;
  emTable_t table;

  status = cParams->sbmalloc(cParams->clientData, SB_ALLOC_INTERNAL,
                             sizeof(g2p3_em32CompState_t), &vp, NULL);

  if (status != SB_OK)
    return status;
  
  pEm32Comp = (g2p3_em32CompState_p_t) vp;

  pEm32Comp->keyValid_f = keyValid_f;
  pEm32Comp->payloadSize = paylSize;
  pEm32Comp->lowIdxBits = lowIdxBits;

  pEm32Comp->dmaInfo[emTableLeft].key.base  = leftBase;
  pEm32Comp->dmaInfo[emTableLeft].key.bank  = leftBank;
  pEm32Comp->dmaInfo[emTableLeft].payload.base = paylBaseLeft;
  pEm32Comp->dmaInfo[emTableLeft].payload.bank = paylBankLeft;

  pEm32Comp->dmaInfo[emTableRight].key.base = rightBase;
  pEm32Comp->dmaInfo[emTableRight].key.bank = rightBank;
  pEm32Comp->dmaInfo[emTableRight].payload.base = paylBaseRight;
  pEm32Comp->dmaInfo[emTableRight].payload.bank = paylBankRight;

  pEm32Comp->useDupBank = useDuplicateBank;
  pEm32Comp->dmaDupInfo[emTableLeft].key.base   = dupLeftBase;
  pEm32Comp->dmaDupInfo[emTableLeft].key.bank   = dupLeftBank;
  pEm32Comp->dmaDupInfo[emTableRight].key.base  = dupRightBase;
  pEm32Comp->dmaDupInfo[emTableRight].key.bank  = dupRightBank;
  

  pEm32Comp->bankOffset[emBankTop]    = topOff;
  pEm32Comp->bankOffset[emBankBottom] = 0;

  pEm32Comp->tableSize = tableSize;

  tableBytes = pEm32Comp->tableSize * sizeof(uint32_t);
  status = cParams->sbmalloc (cParams->clientData, SB_ALLOC_INTERNAL, 
                              tableBytes, &vp, NULL);
  if (status != SB_OK)
    return status;
  
  pEm32Comp->leftKey = (uint32_t *) vp;
  
  status = cParams->sbmalloc (cParams->clientData, SB_ALLOC_INTERNAL, 
                              tableBytes, &vp, NULL);
  if (status != SB_OK)
    return status;
  
  pEm32Comp->rightKey = (uint32_t *) vp;

  tableBytes = pEm32Comp->tableSize * sizeof(keyIdx_t);
  status = cParams->sbmalloc (cParams->clientData, SB_ALLOC_INTERNAL,
                              tableBytes, &vp, NULL);
  if (status != SB_OK)
    return status;
   
  tableBytes = pEm32Comp->tableSize * sizeof(uint32_t);

  for (table = emTableStart; table < emTableCount; table++) {
  
    /* device key table */
    status = cParams->sbmalloc (cParams->clientData, SB_ALLOC_INTERNAL,
                                tableBytes, &vp, NULL);
    if (status != SB_OK)
      return status;
    
    pEm32Comp->deviceKeyTable[table] = (uint32_t *) vp;

    /* device key table, copy for rehashing */
    status = cParams->sbmalloc (cParams->clientData, SB_ALLOC_INTERNAL,
                                tableBytes, &vp, NULL);
    if (status != SB_OK)
      return status;
    
    pEm32Comp->keyTableCopy[table] = (uint32_t *) vp;

    /* dma key table */
    status = cParams->sbmalloc (cParams->clientData, SB_ALLOC_INTERNAL,
                                tableBytes, &vp, NULL);

    if (status != SB_OK)
      return status;

    pEm32Comp->dmaKeyTable[table] = (uint32_t *) vp;
    SB_MEMSET (pEm32Comp->dmaKeyTable[table], 0, tableBytes);

    if(pEm32Comp->useDupBank) {
      /* dma key table */
      status = cParams->sbmalloc (cParams->clientData, SB_ALLOC_INTERNAL,
                                  tableBytes, &vp, NULL);
      
      if (status != SB_OK)
        return status;
      
      pEm32Comp->dmaDupKeyTable[table] = (uint32_t *) vp;
      SB_MEMSET (pEm32Comp->dmaDupKeyTable[table], 0, tableBytes);
    } else {
      pEm32Comp->dmaDupKeyTable[table] = NULL;
    }

    /* dma payload table */
    status = cParams->sbmalloc (cParams->clientData, SB_ALLOC_INTERNAL,
                                tableBytes, &vp, NULL);
    if (status != SB_OK)
      return status;

    pEm32Comp->dmaPayloadTable[table] = (uint32_t *) vp;
    SB_MEMSET (pEm32Comp->dmaPayloadTable[table], 0, tableBytes);

  }

  /* assume we cannot store more than pEm32Comp->tableSize payloads,
   * i.e. assume utilization <= 50%.
   */
  pEm32Comp->maxEntries = pEm32Comp->tableSize-1;

  tableBytes = pEm32Comp->tableSize * pEm32Comp->payloadSize; 
  status = cParams->sbmalloc (cParams->clientData, SB_ALLOC_INTERNAL,
                              tableBytes, &vp, NULL);
  if (status != SB_OK)
    return status;
                                                                                
  pEm32Comp->devicePayloads = (uint8_t *) vp;
  SB_MEMSET (pEm32Comp->devicePayloads, 0, tableBytes);

  tableBytes = pEm32Comp->tableSize * sizeof (auxEntry_t);
  status = cParams->sbmalloc (cParams->clientData, SB_ALLOC_INTERNAL,
                              tableBytes, &vp, NULL);
  if (status != SB_OK)
    return status;
                                                                                
  pEm32Comp->auxEntries = (auxEntry_p_t) vp;
  SB_MEMSET (pEm32Comp->auxEntries, 0, tableBytes);


  /* current state of things */
  pEm32Comp->async = 0;
  pEm32Comp->totEntries = 0;
  SB_MEMSET(pEm32Comp->dirtyEntries, 0, sizeof(pEm32Comp->dirtyEntries));
  SB_MEMSET(pEm32Comp->forceFlush, 0, sizeof(pEm32Comp->forceFlush));

  status = cParams->sbmalloc (cParams->clientData, SB_ALLOC_INTERNAL,
                              sizeof(sbFe2000DmaSlabOp_t), &vp, NULL);
  if (status != SB_OK)
    return status;
  pEm32Comp->slabOp = (sbFe2000DmaSlabOp_t *)vp;

  pEm32Comp->slabWords = slabSize;

  status = cParams->sbmalloc (cParams->clientData, SB_ALLOC_OTHER_DMA,
                              pEm32Comp->slabWords * sizeof(uint32_t), 
                              &vp, &pEm32Comp->slabOp->dmaHandle);
  if (status != SB_OK)
    return status;
  pEm32Comp->slabOp->slab = (sbFe2000DmaSlab_t *) vp;

  /* If no. of dirty entries in either table (left or right) exceeds
   * this threshold then flush the table as a whole.
   * The (heuristic) assumption here is that with DMA the dominant
   * penalty is paid with the overhead of setting up a transfer,
   * not the transfer itself. Thus transferring N 1-word entries 
   * or N k-word entries is a comparable expense. We throw '* 8' 
   * factor for a good measure.
   */
  pEm32Comp->cutoffTotalFlush = 
    (pEm32Comp->tableSize * 8) / pEm32Comp->slabWords;

  pEm32Comp->paylFreeSlot = 1;  /* 1st open payload slot */
  pEm32Comp->currBank = emBankBottom;
  pEm32Comp->currSeed = 59999;
  pEm32Comp->preRehashSeed = pEm32Comp->currSeed;
  pEm32Comp->seedBankDirty = 0;
  pEm32Comp->preRehashSeedBankDirty = pEm32Comp->seedBankDirty;

  pEm32Comp->slabOp->data = (void *)pEm32Comp;
  pEm32Comp->slabOp->hostBusAddress = SB_FE2000_DMA_HBA_COMPUTE;

  pEm32Comp->asyncCb = cParams->asyncCallback;
  pEm32Comp->asyncCbArg = cParams->clientData;

  pEm32Comp->dmaReqToken = cParams->clientData;

  pEm32Comp->pDmaCtxt = pDmaCtxt;
  pEm32Comp->pFe = pFe;

  pEm32Comp->busy = 0;
  pEm32Comp->commitState = emCommitStateStart;

  pEm32Comp->pCommParams = cParams;

  g2p3_em32InitSoftTables(pEm32Comp);

  *em32Ctxt = (void *)pEm32Comp;
  
  return (SB_OK);
}

sbStatus_t
g2p3_em32InitCuckooSeed (void *em32Ctxt, 
                         g2p3_em32SetCuckooSeed_f_t cuckooSeedSet,
                         g2p3_em32GetCuckooSeed_f_t cuckooSeedGet)
{
  g2p3_em32CompState_p_t pEm32 = (g2p3_em32CompState_p_t)em32Ctxt;
  pEm32->cuckooSeedSet = cuckooSeedSet;
  pEm32->cuckooSeedGet = cuckooSeedGet;
  return (SB_OK);
}


sbStatus_t 
g2p3_em32Uninit (void *em32Ctxt)
{ 
  g2p3_em32CompState_p_t pEm32 = (g2p3_em32CompState_p_t)em32Ctxt;
  sbDmaMemoryHandle_t junk;
  sbStatus_t err;
  sbCommonConfigParams_p_t cParams = pEm32->pCommParams;
  emTable_t table;

  sal_memset(&junk, 0, sizeof(junk));
 
  /* leftKey */
  err = cParams->sbfree(cParams->clientData, SB_ALLOC_INTERNAL,
                        pEm32->tableSize * sizeof(uint32_t), pEm32->leftKey, junk);
  if (err)
    return (err);
  
  /* rightKey */
  err = cParams->sbfree(cParams->clientData, SB_ALLOC_INTERNAL,
                        pEm32->tableSize * sizeof(uint32_t), pEm32->rightKey, junk);
  if (err)
    return (err);

  for (table = emTableStart; table < emTableCount; table++) {

    /* device key tables */
    err = cParams->sbfree(cParams->clientData, SB_ALLOC_INTERNAL,
                          pEm32->tableSize * sizeof(uint32_t), 
                          pEm32->deviceKeyTable[table], 
                          junk);
    if (err)
      return (err);

    /* device key tables rehash */
    err = cParams->sbfree(cParams->clientData, SB_ALLOC_INTERNAL,
                          pEm32->tableSize * sizeof(uint32_t), 
                          pEm32->keyTableCopy[table], 
                          junk);
    if (err)
      return (err);
    
    /* dma tables */
    err = cParams->sbfree(cParams->clientData, SB_ALLOC_INTERNAL,
                          pEm32->tableSize * sizeof(uint32_t), 
                          pEm32->dmaKeyTable[table],
                          junk);
    if (err)
      return (err);

    if(pEm32->useDupBank) {
      err = cParams->sbfree(cParams->clientData, SB_ALLOC_INTERNAL,
                            pEm32->tableSize * sizeof(uint32_t), 
                            pEm32->dmaDupKeyTable[table],
                            junk);
      if (err)
        return (err);
    }

    err = cParams->sbfree(cParams->clientData, SB_ALLOC_INTERNAL,
                          pEm32->tableSize * sizeof(uint32_t), 
                          pEm32->dmaPayloadTable[table],
                          junk);
    if (err)
      return (err);
  }


  /* devicePayloads */
  err = cParams->sbfree(cParams->clientData, SB_ALLOC_INTERNAL,
          pEm32->tableSize * pEm32->payloadSize, 
          pEm32->devicePayloads, junk);
  if (err)
    return (err);

  /* auxEntries */
  err = cParams->sbfree(cParams->clientData, SB_ALLOC_INTERNAL,
          pEm32->tableSize * sizeof (auxEntry_t),
          pEm32->auxEntries, junk);
  if (err)
    return (err);

  /* slabOp->slab */
  err = cParams->sbfree(cParams->clientData, SB_ALLOC_OTHER_DMA,
          pEm32->slabWords * sizeof(uint32_t), pEm32->slabOp->slab, 
          pEm32->slabOp->dmaHandle);
  if (err)
    return (err);

  /* slabOp */
  err = cParams->sbfree(cParams->clientData, SB_ALLOC_INTERNAL,
          sizeof(sbFe2000DmaSlabOp_t), pEm32->slabOp, junk);
  if (err)
    return (err);

  err = cParams->sbfree(cParams->clientData, SB_ALLOC_INTERNAL,
          sizeof(g2p3_em32CompState_t), pEm32, junk);
  if (err)
    return (err);


  return (SB_OK);
}


/* -----------------------------------------------------------------------------
 * This routine used by both sbGuSVidCompHWInit and sbGuCommitSVid
 * (distinguished by svidComp->commitType : either init or regular).
 * The task is to run through all internal states (1 through 4) that
 * correspond to all tables that need to be flushed out to device
 * memory (1 - payloads, 2 - left table, 3 - right table, 4 - seed).
 * Note that we only output 1 set of left/right tables, i.e. either top
 * or bottom, but not both (lazy update - do as little as strictly
 * necessary). Which set is flushed (top or bottom) is determined by 
 * svidComp->currBank value (which is also reflected in the top bit
 * of the seed written out to device memory, so that ucode is informed).
 *
 * For each state (1 through 4) determine the update mode (singleEntry
 * or table). This corresponds to how the given table will be flushed
 * - either in word-sized chunks (single entries) corresponding to
 * dirty table entries or in largest possible chunks (determined by how
 * much slab memory we have). The mode chosen is determined by the no.
 * of dirty entries present in a given table - a value greater or equal to
 * svidComp-> implies table mode. Also, table mode
 * can be forced (one-time shot) by setting appr. flush flag 
 * (svidComp->forcePaylFlush, svidComp->forceBotFlush, svidComp->forceTopFlush
 * and svidComp->forceBotFlush). This mechanism is used by the init
 * code (initial flush will flush out bottom bank of tables, but 
 * subsequent flush after a bank switch (from bottom to top) has to
 * flush out top bank in its entirety (regardless of the no. of dirty
 * entries).
 * 
 * In either case appr. no. of DMA slabs is queued - if given table is
 * finished and DMA completes immediately (i.e. not asynchronously)
 * state is pushed to the next value (i.e. next table). Finally,
 * when state reaches value of 4 the seed might be flushed out, but
 * only when necessary (i.e. when either the seed itself has changed
 * or when the bank of tables has been changed (bottom to top or
 * top to bottom)).
 * 
 * If all states are 'traversed' final callback is invoked (separate
 * for init and regular commit).
 * ---------------------------------------------------------------------------*/
void 
g2p3_em32Flush (g2p3_em32CompState_p_t emComp)
{
  sbFe2000DmaSlab_t *slab;
  uint32_t i, words = 0;
  uint32_t slabWords;
  emTable_t table = emTableStart;
  uint32_t hack=0;

#if defined(BROADCOM_DEBUG)
  char* commitStateNames[] = {"Start", "Left", "Right", "Seed", "-error-"};
#endif

  SOC_DEBUG(SOC_DBG_MEM, ("%s: enter\n", FUNCTION_NAME()));
  
flushtop:
  slab = emComp->slabOp->slab;

  /* filled out current table - move to next state (table or seed) */ 
  if (emComp->currFlushTablePos >= emComp->tableSize) {
    emComp->commitState++;
    emComp->currFlushTablePos = 0;

    /* new state implies possibly new mode (table or entry) */
    emComp->flushMode = emFlushModeSingleEntry;
    if (emComp->commitState < emCommitStateSeed) {
      
      switch (emComp->commitState) {
      case emCommitStateLeft : table = emTableLeft; break;
      case emCommitStateRight: table = emTableRight; break;
      default: SB_ASSERT(0);
      }
      
      if ((emComp->dirtyEntries[table][emComp->currBank] >= 
           emComp->cutoffTotalFlush) || emComp->forceFlush[emComp->currBank]) {
        emComp->flushMode = emFlushModeTable;
      }
    }
  }

  if (emComp->commitState < emCommitStateSeed) {
    /* will flush out a chunk of next table */
    
    if (emComp->flushMode == emFlushModeTable) {
      /* transfer in biggest chunks possible */
      /* NOTE - count SLAB_SIZE(0) 3 times for 2 'regular' slab
       * headers (to 2 banks) and SLAB_END header.
       */
      if (emComp->currFlushTablePos + emComp->slabWords - 
          3 * SB_FE2000_SLAB_SIZE(0) < emComp->tableSize)
        words = emComp->slabWords - 3 * SB_FE2000_SLAB_SIZE(0);
      else 
        words = emComp->tableSize - emComp->currFlushTablePos;

      if (words > 1)
        words = words / 2;  /* new scheme - left/right followed by payload */

      /* for each of states 1 thru 3 calculate appr. table chunk
       * and set target address. Also - zero out dirty bits for
       * entries falling into the chunk just DMA-ed and decrement
       * the appr. global dirty count.
       */

      switch (emComp->commitState) {
      case emCommitStateLeft  : table = emTableLeft;  break;
      case emCommitStateRight : table = emTableRight; break;
      default: 
        SB_ASSERT (0);
      }

      for (i = 0; i < words; i++) {
        slab->data[i] = 
          emComp->dmaKeyTable[table][emComp->currFlushTablePos + i];
      }

      slab->feAddress = 
        SB_FE2000_DMA_MAKE_ADDRESS (emComp->dmaInfo[table].key.bank,
                                    (emComp->dmaInfo[table].key.base + 
                                     emComp->bankOffset[emComp->currBank] +
                                     emComp->currFlushTablePos));

      for (i = emComp->currFlushTablePos;
           i < emComp->currFlushTablePos + words; i++) {
       
        if (emComp->auxEntries[i].tableState[table].dirty[emComp->currBank]) {
        
          emComp->auxEntries[i].tableState[table].dirty[emComp->currBank] = 0;
          emComp->dirtyEntries[table][emComp->currBank]--;
        }
      }

      EM_DEBUG(("%s TableFlush: emAddr/feAddr/words= 0x%08x / 0x%08x / %d  (d[flush]=0x%08x\n",
                commitStateNames[emComp->commitState],
                (emComp->dmaInfo[table].key.base + 
                 emComp->bankOffset[emComp->currBank] +
                 emComp->currFlushTablePos),
                slab->feAddress, words,
                emComp->dmaKeyTable[table][emComp->currFlushTablePos]));
      
      slab->opcode = SB_FE2000_DMA_OPCODE_WRITE;
      slab->words = words;
      slab = SB_FE2000_SLAB_NEXT(slab);

      if(emComp->useDupBank) {
        for (i = 0; i < words; i++) {
          slab->data[i] = 
            emComp->dmaDupKeyTable[table][emComp->currFlushTablePos + i];
        }
        
        slab->feAddress = 
          SB_FE2000_DMA_MAKE_ADDRESS (emComp->dmaDupInfo[table].key.bank,
                                      (emComp->dmaDupInfo[table].key.base + 
                                       emComp->bankOffset[emComp->currBank] +
                                       emComp->currFlushTablePos));
        EM_DEBUG(("%s Duplicate Cuckoo TableFlush: "
                  "emAddr/feAddr/words= 0x%08x / 0x%08x / %d  (d[flush]=0x%08x\n",
                  commitStateNames[emComp->commitState],
                  (emComp->dmaDupInfo[table].key.base + 
                   emComp->bankOffset[emComp->currBank] +
                   emComp->currFlushTablePos),
                  slab->feAddress, words,
                  emComp->dmaDupKeyTable[table][emComp->currFlushTablePos]));
        
        slab->opcode = SB_FE2000_DMA_OPCODE_WRITE;
        slab->words = words;
        slab = SB_FE2000_SLAB_NEXT(slab);
      }

      /*  Payload next to key */
      for (i = 0; i < words; i++) {
        slab->data[i] = 
          emComp->dmaPayloadTable[table][emComp->currFlushTablePos + i];
      }
      slab->opcode = SB_FE2000_DMA_OPCODE_WRITE;

      slab->feAddress = 
        SB_FE2000_DMA_MAKE_ADDRESS (emComp->dmaInfo[table].payload.bank,
                                    (emComp->dmaInfo[table].payload.base +
                                     emComp->bankOffset[emComp->currBank] +
                                     emComp->currFlushTablePos));
      
      slab->words = words;
      slab = SB_FE2000_SLAB_NEXT(slab);


      slab->opcode = SB_FE2000_DMA_OPCODE_END;
      slab->feAddress = 0;
      slab->words = 0;

      emComp->currFlushTablePos += words;
    }
    else {
      /* word mode */ 
      slabWords = emComp->slabWords - SB_FE2000_SLAB_SIZE(0); 
      
      switch (emComp->commitState) {
      case emCommitStateLeft  : table = emTableLeft;  break;
      case emCommitStateRight : table = emTableRight; break;
      default: 
        SB_ASSERT (0);
      }

      while ((slabWords >= 3*SB_FE2000_SLAB_SIZE(emComp->payloadSize / (3*sizeof (uint32_t)))) && 
             (emComp->currFlushTablePos < emComp->tableSize)) {

        if (emComp->dirtyEntries[table][emComp->currBank] == 0) {
          /* entire left table flushed out already */
          emComp->currFlushTablePos = emComp->tableSize;
          continue;
        }
        if (!emComp->auxEntries[emComp->currFlushTablePos].tableState[table].dirty[emComp->currBank]) {
          emComp->currFlushTablePos++;
          continue;
        }
            
        slab->data[0] =
          emComp->dmaKeyTable[table][emComp->currFlushTablePos];

        slab->feAddress = 
          SB_FE2000_DMA_MAKE_ADDRESS (emComp->dmaInfo[table].key.bank,
                                      (emComp->dmaInfo[table].key.base + 
                                       emComp->bankOffset[emComp->currBank] +
                                       emComp->currFlushTablePos));
        
        emComp->auxEntries[emComp->currFlushTablePos].
          tableState[table].dirty[emComp->currBank] = 0;

        emComp->dirtyEntries[table][emComp->currBank]--;
        
        slab->opcode = SB_FE2000_DMA_OPCODE_WRITE;
        slab->words = 1;

        /* Arm the next slab to write out the payload */
        slab = SB_FE2000_SLAB_NEXT(slab);

        if(emComp->useDupBank) {
          slab->data[0] =
            emComp->dmaDupKeyTable[table][emComp->currFlushTablePos];
          
          slab->feAddress = 
            SB_FE2000_DMA_MAKE_ADDRESS (emComp->dmaDupInfo[table].key.bank,
                                        (emComp->dmaDupInfo[table].key.base + 
                                         emComp->bankOffset[emComp->currBank] +
                                         emComp->currFlushTablePos));
          slab->opcode = SB_FE2000_DMA_OPCODE_WRITE;
          slab->words = 1;
          /* Arm the next slab to write out the payload */
          slab = SB_FE2000_SLAB_NEXT(slab);
        }
        
        slab->data[0] = 
          emComp->dmaPayloadTable[table][emComp->currFlushTablePos];
        slab->opcode = SB_FE2000_DMA_OPCODE_WRITE;

        slab->feAddress = 
          SB_FE2000_DMA_MAKE_ADDRESS (emComp->dmaInfo[table].payload.bank, 
                                      (emComp->dmaInfo[table].payload.base +
                                       emComp->bankOffset[emComp->currBank] + 
                                       emComp->currFlushTablePos));
        
        EM_DEBUG(("%s Word flush: emAddr/feAddr= 0x%08x / 0x%08x \n",
                  commitStateNames[emComp->commitState],
                  (emComp->dmaInfo[table].payload.base +
                   emComp->bankOffset[emComp->currBank] + 
                   emComp->currFlushTablePos),
                  slab->feAddress));
        
        /* set up for next slab */
        slab->words = 1;
        slab = SB_FE2000_SLAB_NEXT(slab);        
        emComp->currFlushTablePos++;
        slabWords -= words;
      }   /* while */
    
      if (slab != emComp->slabOp->slab) {
        /* we actually queued some stuff - finish with END type slab.
         * slab ptr has already been moved fwd via SB_FE2000_SLAB_NEXT 
         */
        slab->opcode = SB_FE2000_DMA_OPCODE_END;
        slab->feAddress = 0;
        slab->words = 0;
      }
      else {
        /* nothing queued - continue from top */
        goto flushtop;
      }
    }     /* else (--> word mode) */

    if (hack > 30)
      INTR_DEBUG(("hack: 0x%x", hack));
    hack++;

    emComp->status = sbFe2000DmaSlabRequest (emComp->pDmaCtxt,
                                             emComp->dmaReqToken, emComp->slabOp);

    if (emComp->status == SB_OK)
      goto flushtop;

    if (emComp->status != SB_IN_PROGRESS) {
      /* some error occurred - call top-level callback */
      emComp->busy = 0;

      if (emComp->commitType == emCommitTypeInit) {
        /* init-phase commit terminates with error */
        if (emComp->async) {
          emComp->async = 0;
          if (emComp->initCb)
            emComp->initCb(emComp->initCbArg, emComp->status);
        }
      }
      else {
        /* regular commit terminates with error */
        sbFeAsyncCallbackArgument_t aarg;

        emComp->seedBankDirty = 0;
        emComp->forceFlush[emBankBottom] = 0;
        emComp->forceFlush[emBankTop] = 0;

        if (emComp->async) {
          emComp->async = 0;
          aarg.type = SB_FE_ASYNC_SVID_COMMIT_DONE; 
          aarg.status = emComp->status;
          aarg.clientData = emComp->asyncCbArg;
          emComp->asyncCb(&aarg);
        }
      } 
    }
  }
  else {
    /* write out seed */
    if (emComp->seedBankDirty) {
      uint32_t seedWord;

      if (emComp->currBank == emBankBottom) 
        seedWord = emComp->currSeed & SEEDMASK;
      else 
        seedWord = ((emComp->currSeed & SEEDMASK) | BANKBIT);
    
      if ((emComp->cuckooSeedSet) &&  
           (emComp->cuckooSeedSet(emComp->pFe->unit, seedWord)
          == SOC_E_NONE))
        emComp->status = SB_OK;
      else 
        emComp->status = SB_BUSY_ERR_CODE; 

      emComp->commitState++;
    }
    else {
      emComp->status = SB_OK;  /* no need to write out seed/bank word */
    }

    if (emComp->status != SB_IN_PROGRESS) {
      /* commit is done - call top-level callback or return */
      emComp->busy = 0;

      if (emComp->commitType == emCommitTypeInit) {

        /* init-phase commit terminates */
        emComp->forceFlush[emBankBottom] = 0;
        emComp->seedBankDirty = 0;

        if (emComp->async) {
          emComp->async = 0;
          if (emComp->initCb)
            emComp->initCb(emComp->initCbArg, emComp->status);
        }
      }
      else {
        /* regular commit terminates */
        sbFeAsyncCallbackArgument_t aarg;
        
        emComp->seedBankDirty = 0;    
        emComp->forceFlush[emBankBottom] = 0;
        emComp->forceFlush[emBankTop] = 0;


        if (emComp->async) {
          emComp->async = 0;
          aarg.type = SB_FE_ASYNC_SVID_COMMIT_DONE;
          aarg.status = emComp->status;
          aarg.clientData = emComp->asyncCbArg;
          emComp->asyncCb(&aarg);
        }
      }
    }
  }
}


/* -----------------------------------------------------------------------------
 * Used to lock the em32 instance. Assumes the client has physically disabled
 * signals.
 * ---------------------------------------------------------------------------*/
uint32_t 
g2p3_em32SetBusy (void *em32Ctxt)
{
  g2p3_em32CompState_p_t pEm32 = (g2p3_em32CompState_p_t)em32Ctxt;

  if (pEm32->busy)
    return SB_BUSY_ERR_CODE;

  pEm32->busy = 1;
  return (SB_OK);
}

void
g2p3_em32GenPaylHalf (g2p3_em32CompState_p_t pEm32)
{
  uint32_t i, tableBytes, paylIdx, key;
  uint8_t *pd, *ps;
  emTable_t table;

  tableBytes = pEm32->tableSize * sizeof(uint32_t);
  for (table = emTableStart; table < emTableCount; table++) {
    SB_MEMSET (pEm32->dmaKeyTable[table], 0, tableBytes);
    SB_MEMSET (pEm32->dmaPayloadTable[table], 0, tableBytes);
  }

  for (i = 0; i < pEm32->tableSize; i++) {
    /*
     * The 16 bits of key (crc) in deviceKeyTable[t][i] is in big endian format
     * The payload in devicePayloads[] and the final format in the FE
     * (in {l,r}table[i] & {l,r}payl[i] and in device memory), so
     * this code fills deviceKeyTable[t] and devicePayloads into
     * the proper format.  It hard codes a 48-bit payload and
     * a 16-bit half key in particular locations.
     */
    for (table = emTableStart; table < emTableCount; table++) {

      paylIdx = pEm32->deviceKeyTable[table][i] & 0xFFFF;
      pd = (uint8_t *) &pEm32->dmaKeyTable[table][i];
      ps = &pEm32->devicePayloads[(2*paylIdx + 1)*4];
      pd[0] = ps[0];
      pd[1] = ps[1];

      /* bottom half of 2nd word of the payload - hopelessly hard-coded */
      key = pEm32->deviceKeyTable[table][i];
      pd[2] = (key >> 16) & 0xff;
      pd[3] = (key >> 24) & 0xff;
      
      /* the remaining payload */
      sal_memcpy(&pEm32->dmaPayloadTable[table][i], 
                 &pEm32->devicePayloads[2*paylIdx*4], 4);
    }
  }
}


/* -----------------------------------------------------------------------------
 * Initiate the HW init process
 * ---------------------------------------------------------------------------*/
sbStatus_t
g2p3_em32HWInitStart (void *em32Ctxt, sbFeInitAsyncCallback_f_t cBack,
                 void *initId)
{
  g2p3_em32CompState_p_t pEm32 = (g2p3_em32CompState_p_t)em32Ctxt;

  pEm32->initCb = cBack;
  pEm32->initCbArg = initId;

  pEm32->slabOp->cb = g2p3_em32HwInitCont;

  SB_ASSERT (pEm32->async == 0);

  /* prep things for flush () */
  pEm32->commitType = emCommitTypeInit;
  pEm32->forceFlush[emBankBottom] = 1;
  pEm32->forceFlush[emBankTop] = 1;

  /* force mode re-eval */
  pEm32->currFlushTablePos = pEm32->tableSize;

  g2p3_em32GenPaylHalf (pEm32);

  pEm32->seedBankDirty = 1;  /* force seed/bank word out */
  pEm32->commitState = emCommitStateStart;

  g2p3_em32Flush (pEm32);
  return (pEm32->status);
}


/* -----------------------------------------------------------------------------
 * Callback function for sbGuSVidCompHWInit (attached to slabOp->cb).
 * ---------------------------------------------------------------------------*/
void
g2p3_em32HwInitCont(sbFe2000DmaSlabOp_t *slabOp)
{
  g2p3_em32CompState_p_t pEm32;

  pEm32 = (g2p3_em32CompState_p_t)  slabOp->data;
  SB_ASSERT (pEm32);

  /* we know we are executing asynchronously - when done need to throw
   * callback function (rather than just return). 
   */
  pEm32->async = 1;

  /* continue flushing if there is work to be done */
  if ((slabOp->status == SB_OK) && (pEm32->commitState < emCommitStateSeed)) {
    g2p3_em32Flush (pEm32);
  }
  else {
    pEm32->busy = 0;
    pEm32->forceFlush[emBankBottom] = 0;

    /* note that we do not set svidComp->forceTopFlush to FLASE,
     * as the next commit will have to flush the top table in its
     * entirety.
     */

    pEm32->seedBankDirty = 0;
    pEm32->async = 0;

    if (pEm32->initCb)
      pEm32->initCb(pEm32->initCbArg, slabOp->status);
  }
}


/* -----------------------------------------------------------------------------
 * Initiate the commit process
 * ---------------------------------------------------------------------------*/
sbStatus_t
g2p3_em32CommitStart (void *em32Ctxt)
{
  g2p3_em32CompState_p_t pEm32 = (g2p3_em32CompState_p_t)em32Ctxt;

  pEm32->slabOp->cb = g2p3_em32CommitCont;

  pEm32->commitType = emCommitTypeRegular;

  SB_ASSERT (pEm32->async == 0);

  /* force mode re-eval */
  pEm32->currFlushTablePos = pEm32->tableSize;
  pEm32->commitState = emCommitStateStart;

  /* change of seed implies bank flip.
   * bank flip implies bank flip.
   */
  if (pEm32->seedBankDirty == 1) {
    if (pEm32->currBank == emBankBottom)
      pEm32->currBank = emBankTop;
    else
      pEm32->currBank = emBankBottom;
  }

  g2p3_em32GenPaylHalf (pEm32); 

  g2p3_em32Flush (pEm32);
  return (pEm32->status);

}


/* -----------------------------------------------------------------------------
 * Callback function for sbGuCommitSVid (attached to slabOp->cb).
 * ---------------------------------------------------------------------------*/
void
g2p3_em32CommitCont (sbFe2000DmaSlabOp_t *slabOp)
{
  g2p3_em32CompState_p_t pEm32;

  pEm32 = (g2p3_em32CompState_p_t)  slabOp->data;
  SB_ASSERT (pEm32);

  /* we know we are executing asynchronously - when done need to throw
   * callback function (rather than just return).
   */
  pEm32->async = 1;

  /* continue flushing if there is work to be done */
  if ((slabOp->status == SB_OK) && (pEm32->commitState < emCommitStateSeed)) {
    g2p3_em32Flush (pEm32);
  }
  else {
    sbFeAsyncCallbackArgument_t aarg;

    pEm32->busy = 0;
    pEm32->seedBankDirty = 0;
    pEm32->forceFlush[emBankBottom] = 0;
    pEm32->forceFlush[emBankTop] = 0;
    pEm32->async = 0;

    aarg.type = SB_FE_ASYNC_SVID_COMMIT_DONE;
    aarg.status = pEm32->status;
    aarg.clientData = pEm32->asyncCbArg;

#if 0
    SB_ASSERT(pEm32->status != SB_IN_PROGRESS);
#else
    /* XXX xxx hack todo fixme
     * the asynchronous callback should never return an "in progress"
     * this is reserved for the 'issue-side' of the dma, where operations
     * are being submitted for servicing. After the DMA has done it's
     * work there should either be a SB_OK or an error code. There is
     * no reason to have another stall. As a hack, catch this case
     * and return SB_OK. THIS IS A BUG IT NEEDS TO BE FIXED */
    if (pEm32->status == SB_IN_PROGRESS)
        aarg.status = SB_OK;
#endif

    pEm32->asyncCb(&aarg);
  }
}


/* -----------------------------------------------------------------------------
 * Insert a single key - this might result in an entire sequence of bumps
 * (key being inserted bumps out original squatter from the left table,
 * that one bumps some other key out of the right table etc. Note that 
 * the cycle might cause bumping out of the new key freshly inserted into
 * left table into the right table and then again into the left table etc
 * etc). When the cycle of bumps exceeds threshold length 1 is returned 
 * which causes immediate rehash. If the cycle is short enough, 0 is returned
 * and insTable and insPos return information about where the inserted key
 * landed in the end (which bank (left or right) and which position).
 * ---------------------------------------------------------------------------*/
static int
g2p3_em32Insert (g2p3_em32CompState_p_t pEm32, uint32_t key, 
                 emTable_t *insTable, uint32_t *insPos)
{
  uint32_t h1crc, h1idx, h2crc, h2idx, tempKey, devTableEntry, tempEntry, moves;
  uint32_t lKey;

  /* for freshly inserted key use this initial value (0) for 
   * payload slot. sbGuAddSVid() will later fill deviceLeftTable or
   * device RightTable (depending on where the key ended up) with 
   * appr. payload slot idx. 
   *
   * For existing keys that are being bumped out of their locations, 
   * this is valid data and needs to be copied (as we move them from
   * left to right bank (or vice versa) we recalculate hashes but 
   * keep the payload idx, as the payload is not moving).
   */
  devTableEntry = 0; 
  lKey = key;

  moves = 0;
  do {
    h1crc = sbCrc32(lKey);
    h1idx = LOWIDX(pEm32, h1crc);

    tempKey = pEm32->leftKey[h1idx];
    tempEntry = pEm32->deviceKeyTable[emTableLeft][h1idx];

    pEm32->leftKey[h1idx] = lKey;
    pEm32->deviceKeyTable[emTableLeft][h1idx] = 
      LEFTDEVTABLEENTRY(pEm32, h1crc, DEVTABLEIDX(pEm32, devTableEntry));

    TABLE_DIRTY_CHECK(pEm32, emTableLeft, h1idx);

    if (lKey == key) {
      /* insertion point of original key arg */
      *insTable = emTableLeft;
      *insPos = h1idx;
    }

    if (pEm32->auxEntries[h1idx].tableState[emTableLeft].valid == 0) {
      pEm32->auxEntries[h1idx].tableState[emTableLeft].valid = 1;
      return (0);
    }

    /* if here, we kick an entry out to a different position,
     * which implies bank (top v. bottom) flip, which is signalled
     * by setting of seedBankDirty.
     */
    pEm32->seedBankDirty = 1;

    lKey = tempKey;
    devTableEntry = tempEntry;

    h2crc = sbCrc32(lKey + pEm32->currSeed);
    h2idx = TOPIDX(pEm32, h2crc);

    tempKey = pEm32->rightKey[h2idx];
    tempEntry = pEm32->deviceKeyTable[emTableRight][h2idx];

    pEm32->rightKey[h2idx] = lKey;
    pEm32->deviceKeyTable[emTableRight][h2idx] = 
      RIGHTDEVTABLEENTRY(pEm32,h2crc,DEVTABLEIDX(pEm32, devTableEntry));

    TABLE_DIRTY_CHECK(pEm32, emTableRight, h2idx);

    if (lKey == key) {
      /* insertion point of original key arg */
      *insTable = emTableRight;
      *insPos = h2idx;
    }
    
    if (pEm32->auxEntries[h2idx].tableState[emTableRight].valid == 0) {
      pEm32->auxEntries[h2idx].tableState[emTableRight].valid = 1;
      return (0);
    }

    lKey = tempKey;
    devTableEntry = tempEntry;
    moves++;

  } while (moves < MAX_MOVES);

  return (1);

}
        

/* -----------------------------------------------------------------------------
 * If insert ends up in a failure (returning 1) a rehash is ordered.
 * This means that seed is incremented (linearly) and all existing keys
 * are re-inserted into tables (ending up wherever that takes them).
 * Note that this could possibly terminate with a failure (cannot
 * re-insert all existing keys with current seed valued) in which case
 * a new rehash will be ordered. Note that rehash does not imply 
 * any payload movement - payloads stay where they are. As we reinsert
 * the keys, we re-assign to them their respective payload slots.
 * Also, all pre-existing dirty bits have to be preserved in addition
 * to new ones being set (most probably, for large no. of keys this 
 * will imply a table-mode flush for the next commit).
 *
 * Existing keys are kept in the keysAdded table, gaps will exist because
 * the mem move is too expensive when deleting entries
 * ---------------------------------------------------------------------------*/
static int
g2p3_em32Rehash (g2p3_em32CompState_p_t pEm32)
{
  emTable_t table;
  uint32_t tableBytes;
  uint32_t insTable, insPos, oldSeed; 
  uint32_t key, idx, payload_idx;
  uint32_t h1, h2;
  int i, rv;

  tableBytes = pEm32->tableSize * sizeof(uint32_t);

  /* retain a copy of the left and right tables to find valid keys to
   * re-insert with the new seed
   */
  for (table=emTableStart; table < emTableCount; table++) {
    sal_memcpy(pEm32->keyTableCopy[table],
               pEm32->deviceKeyTable[table],
               tableBytes);
  }

  /* Initialize left and right tables to invalid key value to force 
   * a hash miss on all entries.
   */
  g2p3_em32InitSoftTables(pEm32);

  /* do not touch dirty bits (entries might have to be flushed 
   * out even if no longer valid (after removal). Also do not 
   * touch payload entries - rehashing does not affect them at all.
   */
  for (i = 0; i < pEm32->tableSize; i++) {
    pEm32->auxEntries[i].tableState[emTableLeft].valid = 0;
    pEm32->auxEntries[i].tableState[emTableRight].valid = 0;
  }

  oldSeed = pEm32->currSeed++;
  pEm32->seedBankDirty = 1;

  /* get first == get next at tableStart, 0 */
  table = emTableStart;
  idx   = 0;
  rv = g2p3_em32FindNextKey(pEm32, pEm32->keyTableCopy,
                            oldSeed, &table, &idx, &key);

  while (rv == 0) {

    rv = g2p3_em32Insert (pEm32, key, &insTable, &insPos);
    if (rv != 0) {
      return (1);
    }

    h1 = sbCrc32(key);
    h2 = sbCrc32(key + pEm32->currSeed);
    
    payload_idx = DEVTABLEIDX(pEm32, pEm32->keyTableCopy[table][idx]);

    if (insTable == emTableLeft) {
      pEm32->deviceKeyTable[insTable][insPos] = 
        LEFTDEVTABLEENTRY(pEm32, h1, payload_idx);
    }
    else {
      pEm32->deviceKeyTable[insTable][insPos] =
        RIGHTDEVTABLEENTRY(pEm32, h2, payload_idx);
    }

    /* get the next key from the old table using the old seed 
     * to rehash and reinsert
     */
    idx++;
    rv = g2p3_em32FindNextKey(pEm32, pEm32->keyTableCopy,
                              oldSeed, &table, &idx, &key);
  }
  
  return (0);
}

sbStatus_t
sbG2P3AddEM32 (void *em32Ctxt, uint32_t key, uint8_t *payload)
{
  uint32_t h1, h2;
  uint32_t res, i, paylSlotIdx;
  uint8_t paylSlotFound;
  uint32_t insPos;
  emTable_t insTable;
  uint32_t rehashCount, rehashRes; 
  g2p3_em32CompState_p_t pEm32 = (g2p3_em32CompState_p_t)em32Ctxt;

  if (pEm32->totEntries >= pEm32->maxEntries)
    return (SB_SVID_TOO_MANY_ENTRIES);

  /* Check into corresponding left- and right slots to verify this
   * is not a duplicate key.
   */
  h1 = LOWIDX(pEm32,sbCrc32(key));
  h2 = TOPIDX(pEm32,sbCrc32(key + pEm32->currSeed));

  EM_DEBUG(("h1=0x%08x h2=0x%08x key=0x%08x\n",
            h1, h2, key));

  if ((DEVTABLEIDX(pEm32,pEm32->deviceKeyTable[emTableLeft][h1]) != 0) &&
     (pEm32->leftKey[h1] == key))
    return (SB_SVID_DUP);

  if ((DEVTABLEIDX(pEm32,pEm32->deviceKeyTable[emTableRight][h2]) != 0) &&
     (pEm32->rightKey[h2] == key))
    return (SB_SVID_DUP);
    

  /* find an empty slot in devicePayloads. Start from pEm32->paylFreeSlot
   * and scan forward. If no empty slot found that way scan from the
   * very start of the payload table.
   */
  paylSlotFound = 0;
  paylSlotIdx = 0;
  for (i = pEm32->paylFreeSlot; i < pEm32->tableSize; i++) {
    if (!pEm32->auxEntries[i].paylSlotUsed) {
      paylSlotFound = 1;
      paylSlotIdx = i;
      break;
    }
  }

  if (!paylSlotFound) {
    for (i = 1; i < pEm32->paylFreeSlot; i++) {
      if (!pEm32->auxEntries[i].paylSlotUsed) {
        paylSlotFound = 1;
        paylSlotIdx = i;
        break;
      }
    }
  }

  if (!paylSlotFound) 
    return (SB_SVID_INT_ERROR);   /* should never happen */

  /*
   * Try to insert the key, keeping count of forced rehashes.
   * If too high return with an error (i.e. give up).
   */
  rehashCount = 0;
  do {
    res = g2p3_em32Insert (pEm32, key, &insTable, &insPos);
    if (res != 0) {
      do {
        if (rehashCount == 0) {
          /* remember seed value before the rehashes, since when
           * they fail we need to restore the state to what it was
           * before this attempt.
           */
          pEm32->preRehashSeed = pEm32->currSeed;
          pEm32->preRehashSeedBankDirty = pEm32->seedBankDirty;
        }

        EM_DEBUG(("Rehashing...\n"));
        rehashRes = g2p3_em32Rehash (pEm32);
        rehashCount++;
        if (rehashRes == 0)
          break;
      } while (rehashCount < MAX_REHASH_TRIES);
      if (rehashRes != 0)
        break;  /* failed multiple rehashes - fail */
    } /* if */
  } while ((rehashCount < MAX_REHASH_TRIES) && (res != 0));

  if (res != 0) {
    /* recreate state as there must have been rehashes */
    pEm32->currSeed = pEm32->preRehashSeed;
    rehashRes = g2p3_em32Rehash (pEm32);
    pEm32->seedBankDirty = pEm32->preRehashSeedBankDirty;
    if (rehashRes != 0) 
      return (SB_SVID_INT_ERROR);

    return (SB_SVID_COL);
  }


  /* remember the payload */
  SB_MEMCPY (&pEm32->devicePayloads[paylSlotIdx * pEm32->payloadSize], 
             payload, pEm32->payloadSize);
   
  /* update payload slot accounting (used, dirty) */ 
  pEm32->auxEntries[paylSlotIdx].paylSlotUsed = 1;  

  /* sanity check */
  h1 = sbCrc32(key);
  h2 = sbCrc32(key + pEm32->currSeed);

  if ((insPos != LOWIDX(pEm32,h1)) &&
      (insPos != TOPIDX(pEm32,h2)))
    return (SB_SVID_INT_ERROR);

  pEm32->totEntries++;

  pEm32->paylFreeSlot = paylSlotIdx + 1;
  if (pEm32->paylFreeSlot > pEm32->tableSize)
    pEm32->paylFreeSlot = 1;


  /* finally create correct entry for left/right table, taking into
   * account the actual payload slot being used.
   */
  if (insTable == emTableLeft) {
    pEm32->deviceKeyTable[insTable][insPos] =
      LEFTDEVTABLEENTRY(pEm32, h1, paylSlotIdx);
  }
  else {
    pEm32->deviceKeyTable[insTable][insPos] =
      RIGHTDEVTABLEENTRY(pEm32, h2, paylSlotIdx);
  }

  return (SB_OK);
}

sbStatus_t
sbG2P3RemEM32 (void *em32Ctxt, uint32_t key)
{
  uint32_t h1, h2, hkey;
  uint32_t paylIdx;
  g2p3_em32CompState_p_t pEm32 = (g2p3_em32CompState_p_t)em32Ctxt;

  /* is the key there ? */
  h1 = LOWIDX(pEm32,sbCrc32(key));
  h2 = TOPIDX(pEm32,sbCrc32(key + pEm32->currSeed));

  if ((pEm32->leftKey[h1] != key) && (pEm32->rightKey[h2] != key))
    return (SB_SVID_KEY_NOT_FOUND);
  
  /* perform appr. accounting (valid, dirty) */
  if (pEm32->leftKey[h1] == key) {

    paylIdx = LOWIDX(pEm32, pEm32->deviceKeyTable[emTableLeft][h1]);
    hkey = g2p3_em32ComputeHashedInvalidKey(pEm32, h1, 0);

    pEm32->deviceKeyTable[emTableLeft][h1] = 
      LEFTDEVTABLEENTRY(pEm32, hkey, 0);

    pEm32->leftKey[h1] = hkey;

    pEm32->auxEntries[h1].tableState[emTableLeft].valid = 0;
    TABLE_DIRTY_CHECK(pEm32, emTableLeft, h1);
  }
  else {

    paylIdx = LOWIDX(pEm32,pEm32->deviceKeyTable[emTableRight][h2]);
    hkey = g2p3_em32ComputeHashedInvalidKey(pEm32, h2, 1);

    pEm32->deviceKeyTable[emTableRight][h2] = 
      RIGHTDEVTABLEENTRY(pEm32, hkey, 0);

    pEm32->rightKey[h2] = hkey;

    pEm32->auxEntries[h2].tableState[emTableRight].valid = 0;
    TABLE_DIRTY_CHECK(pEm32, emTableRight, h2);
  }

  /* we specifically leave the payload alone - no one points to it
   * anyways, so no point in clearing it. We merely mark (internally0
   * the payload slot as unused.
   */
  pEm32->auxEntries[paylIdx].paylSlotUsed = 0;

  pEm32->totEntries--;

  return (SB_OK);

}

sbStatus_t
sbG2P3UpdateEM32 (void *em32Ctxt, uint32_t key, uint8_t *payload)
{
  uint32_t h1, h2, paylIdx;
  g2p3_em32CompState_p_t pEm32 = (g2p3_em32CompState_p_t)em32Ctxt;

  /* is the key there ? */
  h1 = LOWIDX(pEm32,sbCrc32(key));
  h2 = TOPIDX(pEm32,sbCrc32(key + pEm32->currSeed));

  if ((pEm32->leftKey[h1] != key) && (pEm32->rightKey[h2] != key))
    return (SB_SVID_KEY_NOT_FOUND);

  if (pEm32->leftKey[h1] == key) {  
    if (DEVTABLEIDX(pEm32, pEm32->deviceKeyTable[emTableLeft][h1]) == 0) {
      return (SB_SVID_KEY_NOT_FOUND);
    }

    paylIdx = LOWIDX(pEm32, pEm32->deviceKeyTable[emTableLeft][h1]);
    TABLE_DIRTY_CHECK(pEm32, emTableLeft, h1);
  }
  else {
    if (DEVTABLEIDX(pEm32, pEm32->deviceKeyTable[emTableRight][h2]) == 0) {
      return (SB_SVID_KEY_NOT_FOUND);
    }

    paylIdx = LOWIDX(pEm32, pEm32->deviceKeyTable[emTableRight][h2]);
    TABLE_DIRTY_CHECK(pEm32, emTableRight, h2);
  }
  
  SB_MEMCPY (&pEm32->devicePayloads[paylIdx * pEm32->payloadSize], 
             payload, pEm32->payloadSize);
 
  if (!pEm32->auxEntries[paylIdx].paylSlotUsed)
    return (SB_SVID_INT_ERROR);

  return (SB_OK);

}


sbStatus_t
sbG2P3GetEM32 (void *em32Ctxt, uint32_t key, uint8_t *payload)
{
  int rv;
  g2p3_em32CompState_p_t pEm32 = (g2p3_em32CompState_p_t)em32Ctxt;
  emTable_t table;
  uint32_t  table_idx, paylIdx;

  rv = g2p3_em32KeyLookup(pEm32, pEm32->deviceKeyTable, 
                          pEm32->currSeed, key, &table, &table_idx);

  if (rv != 0) {
    return SB_SVID_KEY_NOT_FOUND;
  }

  paylIdx = LOWIDX(pEm32, pEm32->deviceKeyTable[table][table_idx]);

  SB_MEMCPY (payload, &pEm32->devicePayloads[paylIdx * pEm32->payloadSize],
             pEm32->payloadSize);

  return (SB_OK);
}

sbStatus_t
sbG2P3GetKeysEM32 (void *em32Ctxt, uint32_t key, uint32_t *left,
                   uint32_t *right, uint32_t *seed, int *b)
{
  g2p3_em32CompState_p_t pEm32 = (g2p3_em32CompState_p_t)em32Ctxt;

  *left = sbCrc32(key);
  *right = sbCrc32(key + pEm32->currSeed);
  *seed = (pEm32->currSeed & SEEDMASK);
  *b = (pEm32->currBank != emBankBottom);

  return SB_OK;
}


sbStatus_t
sbG2P3FirstKey (void *em32Ctxt, uint32_t *key)
{
  g2p3_em32CompState_p_t pEm32 = (g2p3_em32CompState_p_t)em32Ctxt;
  emTable_t table;
  uint32_t  rv, idx;

  *key = 0;
  if (pEm32->totEntries) {
    /* first = next at table start, idx 0 */
    table = emTableStart;
    idx   = 0;
    rv = g2p3_em32FindNextKey(pEm32, pEm32->deviceKeyTable, 
                              pEm32->currSeed, &table, &idx, key); 
    if (rv == 0) {
      return SB_OK;
    }
  }
  
  return SB_SVID_KEY_NOT_FOUND;
}

sbStatus_t
sbG2P3NextKey (void *em32Ctxt, uint32_t key, uint32_t *next_key)
{
  g2p3_em32CompState_p_t pEm32 = (g2p3_em32CompState_p_t)em32Ctxt;
  emTable_t table;
  uint32_t idx;
  int rv;

  *next_key = 0;

  if (pEm32->totEntries == 0) {
    return SB_SVID_KEY_NOT_FOUND;
  }

  /* jump to the current location:
   *   find the given key's index in proper table,
   *   then scan for next valid key from the index
   */
  rv = g2p3_em32KeyLookup(pEm32, pEm32->deviceKeyTable, 
                          pEm32->currSeed, key, &table, &idx);

  if (rv != 0) {
    return SB_SVID_KEY_NOT_FOUND;
  }

  idx++;

  rv = g2p3_em32FindNextKey(pEm32, pEm32->deviceKeyTable,
                            pEm32->currSeed, &table, &idx, next_key);
  if (rv == 0) {
    return SB_OK;
  }

  return SB_SVID_KEY_NOT_FOUND;
} 
