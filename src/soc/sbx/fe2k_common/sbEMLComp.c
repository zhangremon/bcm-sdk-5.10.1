/* -*- mode:c++; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=2 cinoptions=\:0l1(0t0g0: */
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
 * $Id: sbEMLComp.c 1.23.112.1 Broadcom SDK $
 * 
 * sbEMLComp.c: Exact Match Legacy compiler.
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 *
 * The exact match compiler uses a two level hash table to store the table
 * entries.  Collisions are avoided by adapting the second level hash
 * function.
 * A table entry consists of a key and some payload.  The key can be of any
 * size, but the EML code processes it as a number of word (at least two
 * words to be effective).  In the following the words of the key are named
 * k0, k1, etc. ki
 * The table is organized s follows:
 *  The first hash is computed as
 *   h1a = crc32(ki ^ ... ^ crc32(k2 ^ crc32(k1 ^ crc32(k0 ^ 0xffffffff)))...
 *   h1 = h1a & table1mask
 *
 *  A typical value for table1mask is 0xffff.  The crc32() function corresponds
 *  to the hash instruction in the LRP.
 *  The h1 hash value is used to index into the first table (which is in
 *  SRAM A or SRAM B).  The table entry is one word has has two parts:
 *  a hash seed, s,  and a pointer.  The pointer points to the second hash
 *  table.  The second hash is computed as
 *   h2 = crc32(l1entry_salt ^ h1a) & table2mask
 *
 *  The table2mask is 0x1f, since we want a 5 bit index into the second table.
 *  The h2 hash value is the index into the second level hash table.
 *  The entries in this table consist of key and payload, scattered between
 *  SRAM A and SRAM B in some manner.
 *  To save space the unused entries of table 2 are not present in the table,
 *  instead the table is preceeded by a bitmask that has 1 set for each present
 *  entry.  So to compute the index in the real table we need to perform the
 *  following, assuming usemap is the bit map for the table
 *   m = usemap & ((1 << h2) - 1)
 *   i = popcnt(m)
 *  If m == usemap then the indexing is out of range, and there is a miss.
 *  Otherwise, the entry can be found at index i.  To make sure this is the
 *  correct entry the key of the entry has to be compared with the key being
 *  looked up.
 *
 * ---------------------------------------------------------------------------*/

#include <sal/core/alloc.h>
#include "sbStatus.h"
#include "sbTypes.h"
#include "sbTypesGlue.h"
#include "sbWrappers.h"
#include <soc/sbx/sbDq.h>

#include "sbEML.h"
#include "sbPayloadMgr.h"
#include "sbEMLComp.h"
#include "sbCrc32.h"
#include "sbFe2000DmaMgr.h"
#include "sbFeISupport.h"
#include <sal/core/sync.h>
#include <shared/util.h>
#include <soc/cm.h>

#ifndef INLINE
#define INLINE
#endif

#define PAYLOAD(p) (((struct table2 *)p)->packedPayload)

#define ISPOWEROF2(x) (((x) & ((x)-1)) == 0 && (x) != 0)

typedef struct t1t2_s {
  uint32 t1_idx;
  uint32 t2_idx;
} t1t2_t;

/* -----------------------------------------------------------------------------
 *
 * Function Prototypes
 *
 * ---------------------------------------------------------------------------*/
static sbStatus_t allocSlab (sbCommonConfigParams_p_t cParams,
                             struct slabTransfer *st, uint size);

static sbStatus_t freeSlab (sbCommonConfigParams_p_t cParams,
                            struct slabTransfer *st);

static INLINE sbEMLTable2_p_t *lookupPKey (sbEML_p_t em,
                                            sbEMLPackedKey_t key,
                                            sbEMLTable1_p_t *t1p);

static INLINE uint32_t hashT2Seed (sbEML_p_t em, sbEMLTable2_p_t t2,
                                   uint32_t l1hash, uint32_t seed);

static void addDelTail (sbEML_p_t em, sbEMLTable1_p_t t1, uint slotix[]);

static void updateSlabDone (sbFe2000DmaSlabOp_t *op);
static void getSlabDone (sbFe2000DmaSlabOp_t* op);
static void slabCommitDone (sbFe2000DmaSlabOp_t* op);

static void initSRAMDone (sbFe2000DmaSlabOp_t* op);
static sbStatus_t initSRAM (sbEML_p_t em);
static sbStatus_t commit (sbEML_p_t em, uint32_t *pnRunLength);
static int t2_cmp(void *a, void *b);

#define block(mutex) \
        sal_mutex_take(mutex, sal_mutex_FOREVER)
#define unblock(mutex, s) \
        sal_mutex_give(mutex)



#define SIZEOF_TABLE1_ENTRY  8

/* -----------------------------------------------------------------------------
 *
 * inlined functions
 *
 * ---------------------------------------------------------------------------*/

/* -----------------------------------------------------------------------------
 *
 * INLINE function
 *
 * Perform the first level hash on a variable length key (n words). The key
 * words are XORed and the result is passed through a CRC32 function.
 *
 * The sbCrc32() function corresponds to the hash instruction in the LRP.
 *
 * The hash value is used to index into the first table (which is in
 * SRAM A or SRAM B).  The table entry is one word has has two parts:
 * a hash seed, s,  and a pointer.  The pointer points to the second hash
 * table.
 *
 * returns the 32 bit hash result
 *
 * ---------------------------------------------------------------------------*/
static INLINE uint32_t hashPKey(sbEMLPackedKey_t key, uint n);
static INLINE uint32_t
hashPKey (sbEMLPackedKey_t key, uint n)
{
  uint32_t k;
  uint i;

  k = 0xffffffff;
  for (i = 0; i < n; i++) {
    k ^= SAND_FE_TO_HOST_32(key[i]);
    k = sbCrc32(k);
  }

#if 0
  soc_cm_print("hash1 (%d)", n);
  for (i = 0; i < n; i++)
    soc_cm_print(" 0x%08x", SAND_FE_TO_HOST_32(key[i]));
  soc_cm_print("\n        -> 0x%08x\n", k);
#endif

  return k;
}


/* -----------------------------------------------------------------------------
 *
 * INLINE function
 *
 * Compare 2 keys.
 *
 * returns 0 if not equal
 *         1 if equal
 *
 * ---------------------------------------------------------------------------*/
static INLINE uint32_t eqPKey (sbEMLPackedKey_t a,
                               sbEMLPackedKey_t b, uint n);
static INLINE uint32_t
eqPKey (sbEMLPackedKey_t a, sbEMLPackedKey_t b, uint n)
{
  uint i;

  for (i = 0; i < n; i++)
    if (a[i] != b[i])
      return 0;

  return 1;
}

/* -----------------------------------------------------------------------------
 *
 * INLINE function
 *
 * Take a key (in packed format), hash it into the first level tabl, mask to
 * confine it to the correct index range index, and follow the pointer down
 * to the second level table. All entries in this bucket are stored on a
 * linked list pointed to by the lvl 1 table ptr. Walk the list and compare
 * the keys to find a match.
 *
 * On return the value *t1p points the the matching t1 entry is a match is
 * found.
 *
 * returns pointer to table2 entry that matches the key
 *         NULL if key not found
 *
 * ---------------------------------------------------------------------------*/
static INLINE struct sbEMLTable2_s **
lookupPKey (sbEML_p_t em, sbEMLPackedKey_t key,
            struct sbEMLTable1_s **t1p)
{
  uint32_t h, hs;
  struct sbEMLTable1_s *e;
  struct sbEMLTable2_s **pp;
  sbEMLPackedKey_t akey;
  uint32_t n;

  n = em->emp->keySize;
  hs = hashPKey(key, n) ;
  h = hs & em->emp->table1mask;
  e = &em->emp->table1[h];
  if (e) {
    e->l1hash = hs;  /* Store the l1 hash */
  }else{
    return NULL;
  }
           
  if (t1p)
    *t1p = e;

  for (pp = &e->entries; *pp; pp = &(*pp)->next) {
    em->getKey(em->ctx, PAYLOAD(*pp), akey);
    if (eqPKey(akey, key, n))
      return pp;
  }
  return NULL;
}


/* -----------------------------------------------------------------------------
 *
 * INLINE function
 *
 * Call lookupPKey to hash the key and search the selected bucket for a match.
 * If a match is found return the T2 entry. If no key is found then walk the
 * nextCls linked list and if a match is found on this then return a pointer
 * to it. The nextCls list contains T2 entries that have been deleted but are
 * still referenced by the classifier.
 *
 *
 * On return the value *t1p points the the matching t1 entry is a match is
 * found.
 *
 * returns pointer to table2 entry that matches the key
 *         NULL if key not found
 *
 * ---------------------------------------------------------------------------*/
INLINE struct sbEMLTable2_s **
lookupPKeyCls (sbEML_p_t em, sbEMLPackedKey_t key,
               struct sbEMLTable1_s **t1p)
{
  struct sbEMLTable2_s **pp;
  sbEMLPackedKey_t akey;
  uint32_t n;

  pp = lookupPKey(em, key, t1p);
  if (pp)
    return pp;

  n = em->emp->keySize;
  for (pp = &(*t1p)->nextCls; *pp; pp = &(*pp)->next) {
    em->getKey(em->ctx, PAYLOAD(*pp), akey);
    if (eqPKey(akey, key, n))
      return pp;
  }
  return NULL;
}


/* -----------------------------------------------------------------------------
 *
 * INLINE function
 *
 * Level 2 hash function. Takes a key of size em->emp->keySize and an 11 bit
 * seed and produces a second level hash value.
 *
 * The hash algorith is:
 *   h2 = crc32( h1 ^ salt) & table2mask
 *
 *
 * The table2mask is 0x1f, since we want a 5 bit index into the second table.
 * The sss value is an extension of the hash seed s,
 *   sss = (s << 22) | (s << 11) | s
 *
 * The h2 has value is the index into the second level hash table.
 * The entries in this table consist of key and payload, scattered between
 * SRAM A and SRAM B in some manner.
 *
 * returns the 32 bit hash result
 *
 * ---------------------------------------------------------------------------*/
#define SEED_DUP(s) (((s) << 18) | ((s) << 9) | (s)) /* XXX */
static INLINE uint32_t
hashT2Seed (sbEML_p_t em, sbEMLTable2_p_t t2, uint32 l1hash, uint32_t seed)
{
  sbEMLPackedKey_t pkey;
  uint32_t k, s;
  uint i, n;

  em->getKey(em->ctx, PAYLOAD(t2), pkey);
  n = em->emp->keySize;
  s = SEED_DUP(seed);
  if (em->new_hash) {

    /* Can not use l1hash value - there is only one stored */
    /* need to calculate the l1hash for each unique t2 entry */
    k = 0xffffffff;
    for (i = 0; i < n; i++) {
      k ^= SAND_FE_TO_HOST_32(pkey[i]);
      k = sbCrc32(k);
    }
    l1hash = k;

    k = sbCrc32(l1hash ^ seed);
#if 0
    soc_cm_print("hash2 (%d)", n);
    for (i = 0; i < n; i++)
      soc_cm_print(" 0x%08x", SAND_FE_TO_HOST_32(pkey[i]));
    soc_cm_print("\n        -> 0x%08x\n", k);
#endif

    return (k);
  }
  else {
    k = (SAND_FE_TO_HOST_32(pkey[0]) + seed)
      ^ (SAND_FE_TO_HOST_32(pkey[1]) >> 1);
    for (i = 2; i < n; i++)
      k ^= (SAND_FE_TO_HOST_32(pkey[i]) >> i);

    /* printf("hash2 (%d) %08x %08x -> %08x\n", n, pkey[0], pkey[1], k);*/
  }

  return sbCrc32(k);
}


/* -----------------------------------------------------------------------------
 *
 * Externally visible functions
 *
 * ---------------------------------------------------------------------------*/


/* -----------------------------------------------------------------------------
 *
 * Initialize the EML Compiler module
 *
 * Allocate memory for all the data structures and build all of the linked
 * lists. The initialization of the hardware components is performed at a
 * later time in sbEMLCompInitHW
 *
 * returns SB_OK
 *         error - return error from memory allocation
 *
 * ---------------------------------------------------------------------------*/
sbStatus_t
sbEMLCompInit (sbEMLCtx_p *pEmCtxt, sbEMLInit_t *initStruct,
                sbCommonConfigParams_p_t cParams,
                void *payloadMgr, sbFe2000DmaMgr_t *dmaCtxt)
{
  sbMalloc_f_t alloc = cParams->sbmalloc;
  void *clientData = cParams->clientData;
  void *v;
  struct emcomp *emp;
  sbStatus_t err;
  uint32_t s;
  uint i;
  sbEML_p_t em;

  err = alloc (clientData, SB_ALLOC_INTERNAL, sizeof (sbEML_t), &v, NULL);
  if (err)
    return (err);

  em = (sbEML_p_t)v; 
  SB_MEMSET(em, 0, sizeof *em);

  *pEmCtxt = (sbEMLCtx_p)v;

  em->ctx = initStruct->ctx;
  em->cParams = initStruct->cParams;
  em->maxDirty = initStruct->maxDirty;
  em->table1_count = initStruct->table1_count;
  em->slab_size = initStruct->slab_size;;
  em->key_size = initStruct->key_size;
  em->seed_bits = initStruct->seed_bits;
  em->new_hash = initStruct->new_hash;
  em->ipv6 = initStruct->ipv6;
  em->table2_max = initStruct->table2_max;
  em->table2_size = initStruct->table2_size;
  em->payload_size = initStruct->payload_size;
  em->getKey = initStruct->getKey;
  em->packKey = initStruct->packKey;
  em->unPackKey = initStruct->unPackKey;
  em->packKeyPayload = initStruct->packKeyPayload;
  em->updatePacked = initStruct->updatePacked;
  em->table2size = initStruct->table2size;
  em->slabTable2entry = initStruct->slabTable2entry;
  em->slabGetTable2entry = initStruct->slabGetTable2entry;
  em->unslabTable2Entry = initStruct->unslabTable2Entry;
  em->slabTable2 = initStruct->slabTable2;
  em->slabSetTable1ptr = initStruct->slabSetTable1ptr;
  em->table1Addr = initStruct->table1Addr;
  em->table1EntryParse = initStruct->table1EntryParse;
  em->commitDone = initStruct->commitDone;
  em->updateDone = initStruct->updateDone;
  em->getDone = initStruct->getDone;
  em->updClsId = initStruct->updClsId;
  em->sharedFreeList = initStruct->sharedFreeList;

  em->nbank = initStruct->nbank;

  /* Allocate memory for local data. */
  err = alloc (clientData, SB_ALLOC_INTERNAL,
               sizeof *emp, &v, NULL);
  if (err)
    return err;

  em->emp = emp = v;
  SB_MEMSET(emp, 0, sizeof *emp);

  /* Allocate semaphore */
  em->eml_mutex = sal_mutex_create("eml_mutex");
  if (!em->eml_mutex){
    return SB_OTHER_ERR_CODE;
  }

  emp->tailDirty = &emp->headDirty;

  SB_ASSERT(ISPOWEROF2(em->table1_count));

  emp->table1mask = em->table1_count - 1;
  SB_ASSERT(em->seed_bits < 32);
  emp->nseeds = 1 << em->seed_bits;
  emp->table2mask = BITSPERUINT32 - 1;
  emp->keySize = (em->key_size + BITSPERUINT32 - 1) / BITSPERUINT32;
  SB_ASSERT(emp->keySize <= SB_EML_MAX_KEY);

  err = alloc (clientData, SB_ALLOC_INTERNAL,
               em->table1_count * sizeof(sbEMLTable1_t),
               &v, NULL);
  if (err)
    return err;

  emp->table1 = v;
  SB_MEMSET(v, 0, em->table1_count * sizeof(sbEMLTable1_t));

  if (em->sharedFreeList) {
    emp->t2freelistp = em->sharedFreeList->emp->t2freelistp;
    em->table2array = NULL;
    em->table2arrayP = em->sharedFreeList->table2arrayP;
  } else {
    /* Allocate memory for (processor) table 2 entries. */
    err = alloc (clientData, SB_ALLOC_INTERNAL,
                 em->table2_max * em->table2_size,
                 &v, NULL);
    if (err)
      return err;

    em->table2array = v;
    em->table2arrayP = v;
    SB_MEMSET(v, 0, em->table2_max * em->table2_size);
    emp->t2freelistp = &emp->_t2freelist;
    for (i = 0; i < em->table2_max; i++)
      freeT2 (emp, (sbEMLTable2_p_t)
              ((char *)em->table2array + i * em->table2_size));
  }


  /* this in bytes */
  s = em->cParams->maximumContiguousDmaMalloc;

  if (s > MAXSLABSIZE)
    s = MAXSLABSIZE;
  if (em->slab_size == SB_FE2000_PARAMS_DEFAULT32 ||
      em->slab_size > s)
    em->slab_size = s;

  /* Allocate a slab for commit. */
  err = allocSlab(em->cParams, &emp->commitSlab, em->slab_size);
  if (err)
    return err;

  /* Allocate a slab for update. */
  err = allocSlab (em->cParams, &emp->updateSlab,
                   em->payload_size + 3 * sizeof(sbFe2000DmaSlab_t));
  if (err)
    return err;

  /* Allocate a slab for get. */
  err = allocSlab (em->cParams, &emp->getSlab,
                   em->payload_size + 3 * sizeof(sbFe2000DmaSlab_t));
  if (err)
    return err;

  if (em->maxDirty > 0) {
    /* temp classifier macs */
    err = em->cParams->sbmalloc (clientData, SB_ALLOC_INTERNAL,
                                 em->maxDirty * sizeof(struct table2entry *),
                                 &v, NULL);
    if (err)
      return err;

    emp->tmpMacs = v;

    err = em->cParams->sbmalloc (clientData, SB_ALLOC_INTERNAL,
                                 2*em->maxDirty * sizeof(struct table2entry *),
                                 &v, NULL);
    if (err)
      return err;

    emp->tmpDirty = v;

    err = em->cParams->sbmalloc (clientData, SB_ALLOC_INTERNAL,
                                 2*em->maxDirty * sizeof(struct table1entry *),
                                 &v, NULL);
    if (err)
      return err;

    emp->tmpDirtyT1 = v;

    err = em->cParams->sbmalloc (clientData, SB_ALLOC_INTERNAL,
                                 em->maxDirty * sizeof(uint8_t), &v, NULL);
    if (err)
      return err;

    emp->tmpAlloc = v;
  }

  em->pDmaCtxt = dmaCtxt;

  em->payloadMgr = (struct sbPayloadMgr *)payloadMgr;

  return SB_OK;
}


/* -----------------------------------------------------------------------------
 *
 * Hardware initialization required by the EML Compiler module
 *
 * Write a zero usemap as a dummy entry in the first location of the t2 table
 * in FE memory. Call initSRAM to zero out all the t1 entries so they index to
 * the dummy location. The call to initSRAM results in an asynchronous
 * callback when complete.
 *
 * returns SB_OK
 *         error - error returned from dma
 *
 * ---------------------------------------------------------------------------*/
sbStatus_t
sbEMLCompInitHW (sbEMLCtx_p emCtxt, sbFeInitAsyncCallback_f_t cBack,
                  void *initId)
{
  sbEML_p_t em = (sbEML_p_t) emCtxt;
  struct emcomp *emp = em->emp;
  uint i;
  sbStatus_t err;

  if (em->sharedFreeList) {
    /* Share the zero entry as well */
    emp->zeroHdl = em->sharedFreeList->emp->zeroHdl;
  } else {
    if (em->ipv6) {
      /* For ipv6 we have no usemap - instead we allocate
       * a full 32-entry bucket and fill it with 0xffffffff.
       * This signals to the ucode that a given entry is a miss.
       */
      emp->zeroHdl = sbPayloadAlloc (em->payloadMgr,
                        (em->payload_size/sizeof(uint32_t)), 0, &err);
    }
    else {
      /* Allocate a double word for the zero entry. */
      emp->zeroHdl = sbPayloadAlloc (em->payloadMgr,
                                      1, 0, &err);
    }
    SB_ASSERT(!SB_PAYLOAD_IS_EMPTY(emp->zeroHdl));
  }

  for (i = 0; i < em->table1_count; i++)
    emp->table1[i].phdl = SB_PAYLOAD_EMPTY;

  /* Init SRAM with pointers to zero word. */
  emp->initCb = cBack;
  emp->initId = initId;
  emp->initOffs = 0;

  return initSRAM(em);
}



/* -----------------------------------------------------------------------------
 *
 * Soft-state recovery mechanims for the EML Compiler module
 *
 * Read the configured hardware Table One an valid Table Two memory, 
 * reconstruct the internal EML compiler state appropriately.
 *
 * returns SB_OK
 *         error - error returned from dma
 *
 * ---------------------------------------------------------------------------*/
sbStatus_t
sbEMLCompRecover (sbEMLCtx_p *pEmCtxt)
{
  sbStatus_t         rv = SB_OK;
  sbEML_p_t          em = (sbEML_p_t) pEmCtxt;
  int                t1_idx, ro_idx, idx, l2slot;
  sbFe2000DmaOp_t    dma_op;
  uint8             *dma_buffer, *table_one;
  uint32             buffer_size;
  uint32             seed, usemap, pxv, l2_bitmask, table2_size;
  struct emcomp     *mez = em->emp;
  sbEMLTable2_t      table2_entry, *t2;
  sbEMLTable1_t     *t1 = NULL;
  sbFe2000DmaSlab_t *slab;
  sbEMLPackedKeyPayload_p_t epkpp = NULL;
  t1t2_t             *retrieve_order = NULL; 
  int unit = (int)em->cParams->clientData;

  buffer_size = (em->table2size(em->ctx, 1) + 1) * 4;
  epkpp = (sbEMLPackedKeyPayload_p_t)sal_alloc(buffer_size, "sm buffer");
  if (epkpp == NULL) {
    return SB_MALLOC_FAILED;
  }
  
  buffer_size = em->table1_count * SIZEOF_TABLE1_ENTRY;
  dma_buffer = soc_cm_salloc(unit, buffer_size, "dma buffer");
  if (dma_buffer == NULL) {
    rv = SB_MALLOC_FAILED;
    goto done;
  }

  buffer_size = em->table1_count * sizeof(*retrieve_order);
  retrieve_order = sal_alloc(buffer_size, "retrieve_order");
  if (retrieve_order == NULL) {
    rv = SB_MALLOC_FAILED;
    goto done;
  }
  sal_memset(retrieve_order, 0, buffer_size);

  table_one = dma_buffer;
  
  /* DMA the entire Table 1 data */
  sal_memset(&dma_op, 0, sizeof(dma_op));
  dma_op.hostAddress    = (uint32 *) table_one;
  dma_op.hostBusAddress = soc_cm_l2p(unit, table_one);
  
  dma_op.feAddress      = em->table1Addr(em->ctx, 0);
  dma_op.opcode         = SB_FE2000_DMA_OPCODE_READ;
  dma_op.words          = em->table1_count * SIZEOF_TABLE1_ENTRY / 4;

  rv = sbFe2000DmaRequest(em->pDmaCtxt, INT_TO_PTR(unit), &dma_op);

  if (rv != SB_OK) {
    soc_cm_print("Failed to DMA table one: 0x%08x\n", rv);
    goto done;
  }

  ro_idx = 0;
  for (idx = 0; idx < em->table1_count; idx++) {
    
    em->table1EntryParse(em->ctx, &seed, &pxv, &usemap, 
                         table_one + (SIZEOF_TABLE1_ENTRY * idx));
    if (pxv && usemap) {
      retrieve_order[ro_idx].t1_idx = idx;
      retrieve_order[ro_idx].t2_idx = pxv;
      ro_idx++;
    }
  }

  /* The payload manager allocation algorithm is O(n), but tends to allocate
   * memory from the low addresses first.  By ordering the addresses to be 
   * recovered, t1->plhdl (payload handles for t2), the search time for the 
   * given address is reduced to O(1), for a compact memory.  When the memory
   * is fragmented (many adds, followed by only deletes), then the recovery 
   * time increases signficantly.  The payload manager may need further 
   * upgrades instead of working around them here.
   */

  /* sort the list to minimize free-list fragmentation in the 
   * payload manager */
  _shr_sort (retrieve_order, ro_idx, sizeof(*retrieve_order), t2_cmp);
  
  for (idx = 0; idx < ro_idx; idx++) {
    t1_idx = retrieve_order[idx].t1_idx;
    t1 = &em->emp->table1[t1_idx];
    
    em->table1EntryParse(em->ctx, &seed, &pxv, &usemap, 
                         table_one + (SIZEOF_TABLE1_ENTRY * t1_idx));

    if (usemap) {
      
      t1->usemap   = usemap;
      t1->usedSeed = seed;
      t1->nused    = _shr_popcount(usemap);
      t1->l1hash   = t1_idx;

      
      table2_size = em->table2size(em->ctx, t1->nused);

      /* Recover/reserve the SRAM payload handle */
      t1->phdl = sbPayloadRecover (em->payloadMgr, pxv, table2_size, &rv);
      if (rv != SB_OK) {
        goto done;
      }
      
      /* Upgrade: read each entire table2, not each table2 entry */
      l2_bitmask = usemap;
      sal_memset(&table2_entry, 0, sizeof(table2_entry));

      /* A note about slots and usemap -
       * The use map defines the bitmap of where an t2 entry would land in a
       * sparsely populated table 2.  However, table 2's are compressed and
       * the usemap is used to define where each entry would live.  This sparse
       * vs compressed storage is why l2slot is updated based on the use map,
       * but table2_entry.slotNo is incremented sequentially.
       */
      l2slot = 0;
      while (l2_bitmask) {
        while ((l2_bitmask & (1 << l2slot)) == 0) l2slot++;
        l2_bitmask &= ~(1<<l2slot);

        slab = mez->getSlab.slabMemory;

        mez->getSlab.slabOp.data = &mez->getSlab;
        mez->getSlab.slabOp.cb = getSlabDone;
        mez->getSlab.slabOp.slab = slab;

        /* Generate a read slab */
        slab = em->slabGetTable2entry (em->ctx, t1->phdl, &table2_entry, slab);

        /* compressed slot number */
        table2_entry.slotNo++;

        slab->opcode = SB_FE2000_DMA_OPCODE_END;
        slab->words = 0;
      
        mez->getSlab.slabEm = em;
        mez->getSlab.slabState = GReading;

        rv = sbFe2000DmaSlabRequest (em->pDmaCtxt,
                                     em->cParams->clientData,
                                     &mez->getSlab.slabOp);

      
        if (rv != SB_IN_PROGRESS) {
          mez->getSlab.slabState = GIdle;
            
          em->unslabTable2Entry (em->ctx, epkpp,
                                 mez->getSlab.slabOp.slab,
                                 table2_entry.slotNo);

          t2 = getT2(mez);
          if (!t2) {
            rv = SB_MAC_NO_MEM;
            goto done;
          }

          t2->usage = USE_ADD;
          sal_memcpy(PAYLOAD(t2), epkpp, em->table2size(em->ctx, 1) * 4);

          /* Add this table 2 entry to list in the parent table 1 entry */
          t2->next = t1->entries;
          t1->entries = t2;

        } else {
          
          rv = SB_BUSY_ERR_CODE;
        }
      } /* while l2 bitmask */
    
    } /* if usemap */
  } /* for each t1 entry */ 

done:  
  if (dma_buffer) {
    soc_cm_sfree(unit, dma_buffer);
  }
  
  if (retrieve_order) {
    sal_free(retrieve_order);
  }

  if (epkpp) {
    sal_free(epkpp);
  }

  return rv;
}


/* -----------------------------------------------------------------------------
 *
 * Uninitialize the EML Compiler module
 *
 * Free all the memory allocated during initialization.
 *
 * returns SB_OK
 *         error - return error from memory free
 *
 * ---------------------------------------------------------------------------*/
sbStatus_t
sbEMLCompUninit (sbEMLCtx_p emCtxt)
{
  sbEML_p_t em = (sbEML_p_t) emCtxt;
  struct emcomp *emp = em->emp;
  sbDmaMemoryHandle_t junk;
  sbStatus_t err;

  if (!emp)
    return SB_OK;

  sal_memset(&junk, 0x0, sizeof(sbDmaMemoryHandle_t));

  if (emp->tmpAlloc) {
    err = em->cParams->sbfree (em->cParams->clientData,
                               SB_ALLOC_INTERNAL,
                               em->maxDirty * sizeof(uint8_t),
                               emp->tmpAlloc, junk);
    emp->tmpAlloc = NULL;

    if (err)
      return err;
  }

  if (emp->tmpDirtyT1) {
    err = em->cParams->sbfree (em->cParams->clientData,
                               SB_ALLOC_INTERNAL,
                               2 * em->maxDirty * sizeof(struct table1entry *),
                               emp->tmpDirtyT1, junk);
    emp->tmpDirtyT1 = NULL;

    if (err)
      return err;
  }

  if (emp->tmpDirty) {
    err = em->cParams->sbfree (em->cParams->clientData,
                               SB_ALLOC_INTERNAL,
                               2 * em->maxDirty * sizeof(struct table2entry *),
                               emp->tmpDirty, junk);
  emp->tmpDirty = NULL;

  if (err)
    return err;
  }

  if (emp->tmpMacs) {
    err = em->cParams->sbfree (em->cParams->clientData,
                               SB_ALLOC_INTERNAL,
                               em->maxDirty * sizeof(struct table2entry *),
                               emp->tmpMacs, junk);
    emp->tmpMacs = NULL;

    if (err)
      return err;
  }

  err = freeSlab (em->cParams, &emp->getSlab);

  if (err)
    return err;

  err = freeSlab (em->cParams, &emp->updateSlab);

  if (err)
    return err;

  err = freeSlab (em->cParams, &emp->commitSlab);

  if (err)
    return err;

  if (em->table2array) {
    err = em->cParams->sbfree (em->cParams->clientData,SB_ALLOC_INTERNAL,
                               em->table2_max * em->table2_size,
                               em->table2array, junk);
    if (err)
      return err;

    em->table2array = NULL;
  }

  if (emp->table1) {
         err = em->cParams->sbfree(em->cParams->clientData,SB_ALLOC_INTERNAL,
                 em->table1_count * sizeof(sbEMLTable1_t),
                 emp->table1, junk);
    if (err)
      return err;

    emp->table1 = NULL;
  }

  err = em->cParams->sbfree (em->cParams->clientData,SB_ALLOC_INTERNAL,
                             sizeof *emp, emp, junk);
  if (err)
    return err;

  em->emp = NULL;

  return SB_OK;
}


/* -----------------------------------------------------------------------------
 *
 * Add an entry into the EML table.
 *
 * Perform all the local processing to add a new EML table entry. The actual
 * update of the FE is performed when sbEMLCommit is invoked.
 *
 * The key and payload parameters represent an entry to be added to the table
 * indicated by the rt2p pointer.
 *
 * The key is used to lookup the entry into the t1 and t2 tables. Continue
 * if the t2 bucket is not full, the entry does not already exist, and
 * enough space can be allocated for the updated bucket.
 *
 * A new t2 bucket is built containing the new entry. It will be downloaded
 * to the FE during a commit and the old buck will be freed.
 *
 * If we are OK to add the entry then check the seed to ensure all t2 hashes
 * produces different t2 indexes into the bucket. If multiple entries hash to
 * the same index then loop through the seed combinations until all t2 hashes
 * are unique. If no seed value is possible that generates unique hashes then
 * this is classed as a collision so free up the used resources and return an
 * error to the user.
 *
 * Returns: SB_OK
 *          SB_MAC_DUP: entry already exists
 *          SB_MAC_FULL: cannot add entry - table full
 *          SB_MAC_NO_MEM: cannot add entry - unable to alloc payload
 *          SB_MAC_COL: entry can not be added due to a collision
 *
 * ---------------------------------------------------------------------------*/
sbStatus_t
sbEMLAdd (sbEMLCtx_p emCtxt,
           sbEMLKey_p_t key,
           sbEMLPayload_p_t payload,
           sbEMLCollisions_p_t coll,
           sbEMLTable2_p_t *rt2p)
{
  sbEML_p_t em = (sbEML_p_t) emCtxt;
  struct emcomp *emp = em->emp;
  sbStatus_t err;
  sbPayloadHandle_t pl;
  sbSyncToken_t s;
  sbEMLTable1_p_t t1 = NULL;
  sbEMLTable2_p_t *t2p, t2, p;
  sbEMLPackedKey_t pkey;
  uint32_t newt2size = 0;
  uint32_t seed = 0;
  uint32_t mask = 0;
  uint32_t h2 = 0;
  uint32_t m = 0;
  uint i;
  uint slotix[SB_EML_MAX_TABLE2_SIZE];
  uint32_t highS = 0;
  uint32_t highestSlot = 0;
  uint32_t goldenSeed = 0;
  uint32_t found = 0;

  s = block(em->eml_mutex);

  /* Locate the entry & update t1 to point to t1 entry */
  em->packKey(em->ctx, key, pkey);
  t2p = lookupPKeyCls(em, pkey, &t1);


  if (t1->nused >= em->table1_count) {
    /* t1 bucket is full - respond with a collision indication */
    goto collision;
  }

  if (t2p) {
    /* if the entry already exists & it's not been deleted then
     * return with a DUP indication */
    t2 = *t2p;
    if (t2->usage & USE_ADD) {
      err = SB_MAC_DUP;
      goto end;
    }
  } else {
    /* allocate table 2 entry */
    t2 = getT2(emp);
    if (!t2) {
      err = SB_MAC_FULL;
      goto end;
    }
    t2->usage = 0;
  }


  /* newt2size gets the amount of space in SRAM needed to hold all
   * payloads in this bucket */
  if (em->ipv6) {
    goldenSeed = 0; found = 0;
    /* temp add t2 */
    if (t2->usage)
      em->updatePacked (em->ctx, PAYLOAD(t2), key, payload);
    else
      em->packKeyPayload (em->ctx, key, payload, PAYLOAD(t2));

    t2->next = t1->entries;
    t1->entries = t2;

    highestSlot = SB_EML_MAX_TABLE2_SIZE + 1;
    for (seed = 0; seed < emp->nseeds; seed++) {
      mask = 0; highS = 0;
      for (i = 0, p = t1->entries; p; i++, p = p->next) {
        h2 = hashT2Seed(em, p, t1->l1hash, seed) & emp->table2mask;
        m = 1U << h2;
        if (mask & m) {
          goto nextseed;
        }
        mask |= m;
        slotix[i] = h2;

        if (h2 > highS)
          highS = h2;
      }

      /* no dups */
      if (highS < highestSlot) {
        highestSlot = highS;
        goldenSeed = seed;
        found = 1;
      }

nextseed:
      continue;
    }


    if (found) {
      t1->nextSeed = t1->usedSeed = goldenSeed;
      t1->usemap = mask;

      seed = goldenSeed;
      for (i = 0, p = t1->entries; p; i++, p = p->next) {
        h2 = hashT2Seed(em, p, t1->l1hash, seed) & emp->table2mask;
        m = 1U << h2;
        mask |= m;
        slotix[i] = h2;
      }

      /* round up slot to the nearest multiple of 4 and multiply by
       * payload size in bytes (per bank - thus the division by 2).
       * (rounding up to a multiple of 4 is caused by the fact that
       * we only have 3 bits to encode the bucket length that could
       * be up to 32 entries; the general formula for rounding up
       * multiple would be something like this :
       * SB_EML_MAX_TABLE2_SIZE / (2 power of
       * (sizeof(int) - SB_FE2000_SRAM_ADDR_SIZE - em->seed_bits))
       */
      
      newt2size = (highestSlot - (highestSlot%4) + 4) *
                    (em->payload_size/(2*sizeof(uint32_t)));
    }

    /* rm t2 entry temporarily added */
    t1->entries = t2->next;
  }
  else {
    /* MAC & IPv4 size */
    newt2size = em->table2size(em->ctx, t1->nused + 1);
  }

  /* allocate space for new table 2 table */
  pl = sbPayloadAlloc (em->payloadMgr, newt2size , 0, &err);
  if (SB_PAYLOAD_IS_EMPTY(pl)) {
    /* payload mgr can't allocate the requested memory */
    freeT2(emp, t2);
    goto end;
  }

  /* Fill t2 entry */
  if (t2->usage)
    em->updatePacked(em->ctx, PAYLOAD(t2), key, payload);
  else
    em->packKeyPayload(em->ctx, key, payload, PAYLOAD(t2));

  t2->valid = 0;
  /* link in t2 as first entry in t1 linked list */
  t2->next = t1->entries;
  t1->entries = t2;
  t2->usage |= USE_ADD;

  /* Try to find a seed that avoids collisions. */
  if (!em->ipv6) {
    for (seed = t1->nextSeed; seed < emp->nseeds; seed++) {
      mask = 0;
      for (i = 0, p = t1->entries; p; i++, p = p->next) {
        h2 = hashT2Seed (em, p, t1->l1hash, seed) & emp->table2mask;
        m = 1U << h2;
        if (mask & m)
          goto bad;
        mask |= m;
        slotix[i] = h2;
      }

      /* no dups, we're done */
      t1->nextSeed = t1->usedSeed = seed;
      t1->usemap = mask;
      goto found;
bad:
      ;
    }
  }


  if (!em->ipv6 || (em->ipv6 && !found)) {
    /* collision, undo our last actions */
    t1->entries = t2->next;
    freeT2 (emp, t2);
    sbPayloadFree (em->payloadMgr, pl);

collision:
    if (coll) {
      /* if the user specifies a collision buffer then return the
       * collsions */
      for (p = t1->entries, i = 0; p && i < coll->entries;
           p = p->next, i++)
        coll->handles[i] = p;
      coll->entries = t1->nused;
    }

    err =  SB_MAC_COL;
    goto end;
  }

found:
  /* return a ptr to the t2 entry */
  if (rt2p)
    *rt2p = t2;

  t1->nused++;
  SB_ASSERT(t1->nused <= SB_EML_MAX_TABLE2_SIZE);

  /* the seed search filled consecutive slotix[] entries with the hash
   * indexes of the bucket entries (t1->entries valid). */
  addDelTail(em, t1, slotix);

  /* store a ptr to the alloced payload in case we want to free it later */
  t1->phdl = pl;
  t1->dataSize = newt2size;

  err = SB_OK;

end:
  unblock(em->eml_mutex, s);
  return err;
}


/* -----------------------------------------------------------------------------
 *
 * Delete an entry in the EML table
 *
 * Perform all the local processing to delete an existing EML table entry. The
 * actual update of the FE is performed when sbEMLCommit is invoked.
 *
 * Hash the specified entry to find it's t1 and t2 location. If found then
 * remove the t2 entry from the t2 block. This involves building a new t2
 * block, allocating new payload space, freeing the old payload and updating
 * the t1 pointer. The local processing for this is performed and the function
 * addDelTail is called to add it to the queue for future processing by a
 * commit.
 *
 * Returns: SB_OK
 *          SB_MAC_NOT_FOUND
 *          SB_MAC_NO_MEM: cannot remove entry - unable to alloc payload
 *
 * ---------------------------------------------------------------------------*/
sbStatus_t
sbEMLDel (sbEMLCtx_p emCtxt, sbEMLKey_p_t key)
{
  sbEML_p_t em = (sbEML_p_t) emCtxt;
  sbEMLPackedKey_t pkey;

  em->packKey(em->ctx, key, pkey);
  return (sbEMLDelPackedKey (em, pkey));
}


sbStatus_t
sbEMLDelPackedKey (sbEML_p_t em, sbEMLPackedKey_t pkey)
{
  struct emcomp *emp = em->emp;
  sbStatus_t err;
  sbPayloadHandle_t pl;
  sbSyncToken_t s;
  sbEMLTable1_p_t t1;
  sbEMLTable2_p_t *t2p, t2;
  uint32_t newt2size = 0;
  uint32_t h2 = 0;
  uint i;
  uint slotix[SB_EML_MAX_TABLE2_SIZE];
  uint32_t highS, highestSlot, goldenSeed, found;
  uint32_t seed = 0;
  uint32_t mask = 0;
  uint32_t m = 0;
  sbEMLTable2_p_t p;

  s = block(em->eml_mutex);
  /* Locate the entry & update t1 to point to t1 entry */
  t2p = lookupPKey (em, pkey, &t1);
  if (!t2p) {
    err = SB_MAC_NOT_FOUND;
    goto end;
  }
  t2 = *t2p;

  if (em->ipv6) {
    goldenSeed = 0; found = 0;

    highestSlot = SB_EML_MAX_TABLE2_SIZE;
    for (seed = 0; seed < emp->nseeds; seed++) {
      mask = 0; highS = 0;
      for (i = 0, p = t1->entries; p; i++, p = p->next) {
        /* skip entry to be removed */
        if (p == t2)
          continue;

        h2 = hashT2Seed (em, p, t1->l1hash, seed) & emp->table2mask;
        m = 1U << h2;
        if (mask & m) {
          goto nextseed;
        }
        mask |= m;
        slotix[i] = h2;

        if (h2 > highS)
          highS = h2;
      }
    

      /* went through all entries - see how big the bucket is */
      if (highS < highestSlot) {
        highestSlot = highS;
        goldenSeed = seed;
        found = 1;
      }

nextseed:
      continue;
    }
  

    if (found) {
      t1->nextSeed = t1->usedSeed = goldenSeed;
      t1->usemap = mask;
      seed = goldenSeed;
      for (i = 0, p = t1->entries; p; i++, p = p->next) {
        /* skip entry to be removed */
        if (p == t2)
          continue;

        h2 = hashT2Seed (em, p, t1->l1hash, seed) & emp->table2mask;
        m = 1U << h2;
        mask |= m;
        slotix[i] = h2;
      }

      /* round up slot to the nearest multiple of 4 and multiply by
       * payload size in bytes (per bank - thus the division by 2).
       * (rounding up to a multiple of 4 is caused by the fact that
       * we only have 3 bits to encode the bucket length that could
       * be up to 32 entries; the general formula for rounding up
       * multiple would be something like this :
       * SB_EML_MAX_TABLE2_SIZE / (2 power of
       * (sizeof(int) - SB_FE2000_SRAM_ADDR_SIZE - em->seed_bits))
       */
      newt2size = (highestSlot - (highestSlot%4) + 4) *
                    (em->payload_size/(2*sizeof(uint32_t)));

    }
  } /* em->ipv6 */
  else {
    /* newt2size is one entry smaller than before */
    newt2size = em->table2size(em->ctx, t1->nused - 1);
  }

  if (newt2size) 
  {
    /* allocate space for new table 2 table */
    pl = sbPayloadAlloc (em->payloadMgr, newt2size,
                          1, &err);
    if (SB_PAYLOAD_IS_EMPTY(pl)) {
      goto end;
    }
  } else {
    /* if 0 entries in bucket (size of 1 corresponds to only
     * the usemap being present except in tribank case, where usemap
     * is with t1) then no need to alloc another
     * payload - store a ptr to empty payload */
    pl = SB_PAYLOAD_EMPTY;
  }

  t2->usage &= ~USE_ADD;

  /* link out table 2 struct */
  *t2p = t2->next;

  /* remove use bit in usemap */
  h2 = hashT2Seed (em, t2, t1->l1hash, t1->usedSeed) & emp->table2mask;
  t1->usemap &= ~(1 << h2);
  t1->nused--;
  t1->nextSeed = 0;   /* need to restart seed checks */

  if (t2->usage) {
    /* still used by classifier, move to nextCls list */
    t2->next = t1->nextCls;
    t1->nextCls = t2;
  } else {
    /* Put on list to be freed on commit. */
    t2->next = t1->remEntries;
    t1->remEntries = t2;
  }

  /* recalculate the slot array (see addDelTail comments for description)
   * Don't need to search for good seed - current seed works fine after
   * a delete */
  for (i = 0, t2 = t1->entries; t2; i++, t2 = t2->next)
    slotix[i] = hashT2Seed (em, t2, t1->l1hash, t1->usedSeed) & emp->table2mask;

  /* Add to the delete queue for processing during a commit */
  addDelTail (em, t1, slotix);
  t1->phdl = pl;
  t1->dataSize = newt2size;

  err = SB_OK;

end:
  unblock(em->eml_mutex, s);
  return err;
}


/* -----------------------------------------------------------------------------
 *
 * Commit the EML changes
 *
 * The param pnRunLength specifies how many updates to process.
 *
 * returns: SB_IN_PROGRESS - commit process kicked off or queued
 *          SB_MORE - commit completed OK but still entries on dirty queue
 *          SB_OK - commit completed immediately
 *          SB_BAD_ARGUMENT_ERR_CODE - DMA param error
 *          error code
 *
 * ---------------------------------------------------------------------------*/
sbStatus_t
sbEMLCommit (sbEMLCtx_p emCtxt, uint32_t *pnRunLength)
{
  sbEML_p_t em = (sbEML_p_t) emCtxt;
  struct emcomp *emp = em->emp;
  sbStatus_t err;
  sbSyncToken_t s;

  s = block(em->eml_mutex);

  /* check for busy */
  if (emp->committing || emp->clsCommitting) {
    unblock (em->eml_mutex, s);
    return SB_BUSY_ERR_CODE;
  }

  emp->committing = 1;

  err = commit (em, pnRunLength);
  if (err != SB_IN_PROGRESS)
    emp->committing = 0;

  unblock (em->eml_mutex, s);
  return err;
}


/* -----------------------------------------------------------------------------
 *
 * Update an existing entry into the EML table.
 *
 * Perform all the local processing to update an EML table entry. If the entry
 * is already in the FE's memory then build a slab and update immediatly.
 * Unlike the Add and Del functions do not wait for a commit. If the entry
 * exists locally and has yet to be committed then update the local copy and
 * wait for the next commit to  load it to the FE.
 *
 * Returns: SB_OK
 *          SB_BUSY_ERR_CODE: update failed, previous update in progress
 *          SB_MAC_NOT_FOUND
 *          SB_MAC_COL: entry can not be added due to a collision
 *          error code: error during DMA
 *
 * ---------------------------------------------------------------------------*/
sbStatus_t
sbEMLUpdate (sbEMLCtx_p emCtxt,
              sbEMLKey_p_t key,
              sbEMLPayload_p_t payload,
              sbEMLTable2_p_t *at2p)
{
  sbEML_p_t em = (sbEML_p_t) emCtxt;
  struct emcomp *emp = em->emp;
  sbStatus_t err;
  sbSyncToken_t s;
  sbEMLTable2_p_t *t2p, t2;
  sbEMLTable1_p_t t1;
  sbEMLPackedKey_t pkey;

  s = block (em->eml_mutex);
  /* Check if we are busy */
  if (emp->updateSlab.slabState != UIdle) {
    err = SB_BUSY_ERR_CODE;
    goto end;
  }

  /* Locate the entry & update t1 to point to t1 entry */
  em->packKey (em->ctx, key, pkey);
  t2p = lookupPKey (em, pkey, &t1);
  if (!t2p) {
    err = SB_MAC_NOT_FOUND;
    goto end;
  }
  t2 = *t2p;
  *at2p = t2;
  SB_ASSERT(t2);

  /* Update the local copy of the SRAM entry */
  em->updatePacked (em->ctx, PAYLOAD(t2), key, payload);

  if (t2->valid) {
    /* entry is valid in FE's memory commit update immediately */
    sbFe2000DmaSlab_t *slab = emp->updateSlab.slabMemory;
    emp->updateSlab.slabOp.data = &emp->updateSlab;
    emp->updateSlab.slabOp.cb = updateSlabDone;
    emp->updateSlab.slabOp.slab = slab;

    /* Convert entry to a slab */
    slab = em->slabTable2entry (em->ctx, t1->phdl, t2, slab);

    slab->opcode = SB_FE2000_DMA_OPCODE_END;
    slab->words = 0;

    emp->updateSlab.slabEm = em;
    emp->updateSlab.slabState = UWriting;
    err = sbFe2000DmaSlabRequest (em->pDmaCtxt,
                                   em->cParams->clientData,
                                   &emp->updateSlab.slabOp);

    if (err != SB_IN_PROGRESS)
      emp->updateSlab.slabState = UIdle;
  } else {
    /* If not yet committed we just wait for the next commit. */
    err = SB_OK;
  }

end:
  unblock (em->eml_mutex, s);
  return err;
}


/*
 * As update above, but does not force commit - used only by ipv6 lpm
 */
sbStatus_t
sbEMLUpdateLocal (sbEML_p_t em,
                   sbEMLKey_p_t key,
                   sbEMLPayload_p_t payload,
                   sbEMLTable2_p_t *at2p)
{
  struct emcomp *emp = em->emp;
  sbStatus_t err;
  sbSyncToken_t s;
  sbEMLTable2_p_t *t2p, t2;
  sbEMLTable1_p_t t1;
  sbEMLPackedKey_t pkey;

  s = block (em->eml_mutex);

  /* Locate the entry & update t1 to point to t1 entry */
  em->packKey (em->ctx, key, pkey);
  t2p = lookupPKey (em, pkey, &t1);
  if (!t2p) {
    err = SB_MAC_NOT_FOUND;
    goto end;
  }
  t2 = *t2p;
  *at2p = t2;
  SB_ASSERT(t2);

  /* Update the local copy of the SRAM entry */
  em->updatePacked (em->ctx, PAYLOAD(t2), key, payload);

  /* force update during the next commit */
  t1->isDirty = 1;
  /* have to append to keep order in commit */
  t1->nextDirty = NULL;
  *emp->tailDirty = t1;
  emp->tailDirty = &t1->nextDirty;

  err = SB_OK;

end:
  unblock (em->eml_mutex, s);
  return err;
}


/* -----------------------------------------------------------------------------
 *
 * Get an entry from the EML table and return it asynchronously.
 *
 * Returns a copy of an entry in the EML Table. If the entry is new and has
 * yet to be commited to the FE then it's local copy is returned. If the
 * entry is stored in the FE then a slab DMA is initiated to read back the
 * entry data. The response is returned asynchronously through the callback
 * function getSlabDone.
 *
 * In general we return the FE's copy of the EML Table entry instead of the
 * local copy. This is because the microcode updates the ageing field in the
 * entries, so the payloads are not identical.
 *
 * Returns: SB_OK: returned with get data immediately
 *          SB_BUSY_ERR_CODE: get failed, previous get in progress
 *          SB_MAC_NOT_FOUND
 *          error code: error during DMA
 *
 * ---------------------------------------------------------------------------*/
sbStatus_t
sbEMLGet (sbEMLCtx_p emCtxt, sbEMLKey_p_t key,
           sbEMLPackedKeyPayload_p_t pkp,
           sbEMLTable2_p_t *at2p)
{
  sbEML_p_t em = (sbEML_p_t) emCtxt;
  struct emcomp *emp = em->emp;
  sbStatus_t err;
  sbSyncToken_t s;
  sbEMLTable1_p_t t1;
  sbEMLTable2_p_t *t2p, t2;
  sbEMLPackedKey_t pkey;

  s = block (em->eml_mutex);
  /* Check if we are busy */
  if (emp->getSlab.slabState != GIdle) {
    err = SB_BUSY_ERR_CODE;
    goto end;
  }

  /* Locate the entry & update t1 to point to t1 entry */
  em->packKey (em->ctx, key, pkey);
  t2p = lookupPKey (em, pkey, &t1);
  if (!t2p) {
    err = SB_MAC_NOT_FOUND;
    goto end;
  }
  t2 = *t2p;
  SB_ASSERT(t2);
  if (at2p)
    *at2p = t2;

  if ((t2->valid) && (!t1->isDirty)) {
    sbFe2000DmaSlab_t *slab = emp->getSlab.slabMemory;
    emp->getSlab.slabOp.data = &emp->getSlab;
    emp->getSlab.slabOp.cb = getSlabDone;
    emp->getSlab.slabOp.slab = slab;

    /* Generate a read slab */
    slab = em->slabGetTable2entry (em->ctx, t1->phdl, t2, slab);

    slab->opcode = SB_FE2000_DMA_OPCODE_END;
    slab->words = 0;

    emp->getSlab.slabEm = em;
    emp->getSlab.slabState = GReading;
    err = sbFe2000DmaSlabRequest (em->pDmaCtxt,
                                   em->cParams->clientData,
                                   &emp->getSlab.slabOp);
    if (err != SB_IN_PROGRESS) {
      emp->getSlab.slabState = GIdle;
      em->unslabTable2Entry (em->ctx, pkp,
                             emp->getSlab.slabOp.slab,
                             t2->slotNo);
    } else {
      emp->getSlotNo = t2->slotNo;
      emp->getPkp = pkp;
    }
  } else {
    /* Not yet committed, copy the local payload */
    err = SB_OK;
    SB_MEMCPY (pkp, PAYLOAD(t2), em->payload_size);
  }

end:
  unblock (em->eml_mutex, s);
  return err;
}


/* -----------------------------------------------------------------------------
 *
 * Local functions
 *
 * ---------------------------------------------------------------------------*/

/* -----------------------------------------------------------------------------
 *
 * Compare table 2 indexes; used for sorting
 *
 * ---------------------------------------------------------------------------*/
static int t2_cmp(void *a, void *b)
{
  t1t2_t *ta, *tb;
  ta = (t1t2_t *)a;
  tb = (t1t2_t *)b;
  
  return (int)ta->t2_idx - (int)tb->t2_idx;
}

/* -----------------------------------------------------------------------------
 *
 * Free a t2 entry back on to the free list.
 *
 * ---------------------------------------------------------------------------*/
void
freeT2 (struct emcomp *emp, sbEMLTable2_p_t p)
{
  p->next = *emp->t2freelistp;
  *emp->t2freelistp = p;
}


/* -----------------------------------------------------------------------------
 *
 * Allocate a t2 entry from the free list. The t2 entry contains the packed
 * t2 payload. See sbEMLComp.h for details of the t2 data structure. No
 * bound checking is needed as the free list is initialized to handle the
 * worst case.
 *
 * returns pointer to a t2 structure
 *
 * ---------------------------------------------------------------------------*/
sbEMLTable2_p_t
getT2 (struct emcomp *emp)
{
  sbEMLTable2_p_t p;

  p = *emp->t2freelistp;
  if (p)
    *emp->t2freelistp = p->next;
  return p;
}


/* -----------------------------------------------------------------------------
 *
 * Allocate DMA memory to be used to build slabs. This function is called
 * during the init process, and is not used dynamically during normal
 * operation.
 *
 * returns SB_OK
 *         error - error returned from malloc
 *
 * ---------------------------------------------------------------------------*/
static sbStatus_t
allocSlab (sbCommonConfigParams_p_t cParams,
           struct slabTransfer *st, uint size)
{
  sbStatus_t err;

  /* allocate slab */
  st->slabSize = size;

  /* size in bytes */
  SB_ASSERT (size/sizeof(uint32_t) < cParams->maximumContiguousDmaMalloc);
  st->slabMemory = NULL;
  err = cParams->sbmalloc (cParams->clientData, SB_ALLOC_L2_DMA,
                           size, &st->slabMemory, &st->slabOp.dmaHandle);
  if (err)
    return err;

  st->slabEnd = (uint32_t *)st->slabMemory + size / sizeof(uint32_t);
  st->slabOp.hostBusAddress = SB_FE2000_DMA_HBA_COMPUTE;
  return SB_OK;
}


/* -----------------------------------------------------------------------------
 *
 * Free DMA memory previously allocated by allocSlab during initialization
 *
 * returns SB_OK
 *         error - error returned from free
 *
 * ---------------------------------------------------------------------------*/
static sbStatus_t
freeSlab (sbCommonConfigParams_p_t cParams,
          struct slabTransfer *st)
{
  sbStatus_t err;

  if (st->slabMemory) {
    err = cParams->sbfree (cParams->clientData, SB_ALLOC_L2_DMA,
                           st->slabSize,
                           st->slabMemory, st->slabOp.dmaHandle);
    st->slabMemory = NULL;
    if (err)
      return err;
  }

  return SB_OK;
}


/* -----------------------------------------------------------------------------
 *
 * Callback function invoked when an update slab dma transfer completes. Calls
 * the user callback function.
 *
 * ---------------------------------------------------------------------------*/
static void
updateSlabDone (sbFe2000DmaSlabOp_t *op)
{
  struct slabTransfer *xfer = op->data;
  sbEML_p_t em = xfer->slabEm;

  xfer->slabState = UIdle;
  em->updateDone (em->ctx, op->status);
}


/* -----------------------------------------------------------------------------
 *
 * Callback function invoked when an get slab dma transfer completes. Unpacks
 * the returned payload data and calls the user callback function.
 *
 * ---------------------------------------------------------------------------*/
static void
getSlabDone (sbFe2000DmaSlabOp_t* op)
{
  struct slabTransfer *xfer = op->data;
  sbEML_p_t em = xfer->slabEm;
  struct emcomp *emp = em->emp;

  xfer->slabState = GIdle;
  em->unslabTable2Entry (em->ctx, emp->getPkp, xfer->slabMemory,
                         emp->getSlotNo);
  em->getDone (em->ctx, op->status);
}


/* -----------------------------------------------------------------------------
 *
 * The param slotix[] is filled with consecutive entries from 0 to t1->entries-1
 * These entries contain the t2 hash index into the t2 bucket. They are in no
 * particular order. The first phase is to order the entries in t1->entries
 * linked list so that the slot 0 (rightmost bit in usemap) is first, followed
 * be the entry for the next rightmost bit etc.
 *
 * The second phase removes the old payload - this is no longer used
 * it is replaced with the new one. The t1 pointer is added to the end of the
 * *emp->tailDirty linked list so it can be updated in order during the next
 * commit.
 *
 * ---------------------------------------------------------------------------*/
static void
addDelTail (sbEML_p_t em, sbEMLTable1_p_t t1, uint slotix[])
{
  struct emcomp *emp = em->emp;
  sbEMLTable2_p_t p;
  uint remap[SB_EML_MAX_TABLE2_SIZE];
  uint i, ci;
  uint32_t mask;

  if (t1->nused > 0) {
    if (em->ipv6) {
      /* no remap for ipv6 (no usemap for ipv6) */
      for (i = 0, p = t1->entries; p; i++, p = p->next) {
        p->slotNo = slotix[i];
      }
    }
    else {
      mask = t1->usemap;
      /* Compute a mapping from uncompressed to compressed index */
      for (ci = i = 0; mask != 0; mask >>= 1, i++) {
        remap[i] = ci;
        ci += mask & 1;
      }
      for (i = 0, p = t1->entries; p; i++, p = p->next) {
        /* slotix[i] is the index into the uncompressed table*/
        ci = remap[slotix[i]];
        SB_ASSERT (i < t1->nused);
        SB_ASSERT (slotix[i] < SB_EML_MAX_TABLE2_SIZE);
        SB_ASSERT (ci < t1->nused);
        p->slotNo = ci;
      } /* for */
    }   /* else */

    /* At this point each element on the the linked list
     * t1->entries has it's slotNo set corresponding to it's
     * bit position in the usemap, right-most = slot 0 */
  } /* if */

  if (!t1->isDirty) {
    t1->isDirty = 1;
    /* have to append to keep order in commit */
    t1->nextDirty = NULL;
    *emp->tailDirty = t1;
    emp->tailDirty = &t1->nextDirty;

    if (!SB_PAYLOAD_IS_EMPTY(t1->phdl))
      sbPayloadFree(em->payloadMgr, t1->phdl);
  } else {
    /* free old table 2 */
    if (!SB_PAYLOAD_IS_EMPTY(t1->phdl))
      sbPayloadFree(em->payloadMgr, t1->phdl);
  }
}


/* -----------------------------------------------------------------------------
 *
 * The dirty queue (emp->headDirty) contains a linked list of t1 entries that
 * need to be updated in the FE's memory. This function walks the list, dmas
 * the update to the FE and removes the entry from the list. The function
 * completes when the list is empty, the count limit is reached or a dma slab
 * is full.
 *
 * returns: SB_IN_PROGRESS - flush DMA kicked off or queued
 *          SB_OK - DMA completed immediately
 *          SB_BAD_ARGUMENT_ERR_CODE - DMA param error
 *          error code
 *
 * ---------------------------------------------------------------------------*/
sbStatus_t
flushDirty(sbEML_p_t em, uint32_t *rl)
{
  struct emcomp *emp = em->emp;
  sbSyncToken_t s;
  sbFe2000DmaSlab_t *slab;
  sbEMLTable1_p_t t1;
  sbEMLTable2_p_t p, np;
  sbStatus_t err;
  sbPayloadHandle_t phdl;
  uint nslabs, data;

  s = block (em->eml_mutex);
  if (*rl == 0) {
    unblock (em->eml_mutex, s);
    return SB_OK;
  }

  slab = emp->commitSlab.slabMemory;
  for (t1 = emp->headDirty; *rl > 0 && t1; --*rl, t1 = t1->nextDirty) {
    /* for each dirty entry we build a slab containing the new
     * block of t2 entries followed by the new t1 entry.
     * we need room for:
     *   the current slabs (2 headers + data)  +
     *   sync  +
     *   end
     */
    nslabs = em->nbank + 1 + 1;
    data = t1->dataSize ;/* *(2 + em->triBank); */
    /* if the slab is full, break from loop and start the dma */
    if ((uint32_t *)slab
        + nslabs * SIZEOFW(sbFe2000DmaSlab_t)
        + data >= emp->commitSlab.slabEnd)
      break;

    /* convert the t2 entries into the FEs format and copy
     * into a slab */
    if (t1->nused > 0) {
      slab = em->slabTable2 (em->ctx, t1->phdl, t1->usemap,
                             t1->entries, slab);
    }
    /* modify the t1 ptr to point to the new t2 entry. If new
     * entry is blank use the dummy pointer */
    phdl = t1->nused > 0 ? t1->phdl : emp->zeroHdl;

    if (em->ipv6) {
      /* combine seed bits (8) with top bits of bucket size;
       * what this is means is : occupy top em->seed_bits
       * bits with the seed, the rest (bottom) get taken
       * by the bucket length suitably truncated (currently
       * we have only 3 bits to encode possibly 5-bit length
       * - we just use top 3 bits and store buckets in
       * 4-entry increments.
       */
      slab = em->slabSetTable1ptr (em->ctx, t1 - emp->table1,
                      ((t1->usedSeed) << (32 - SB_FE2000_SRAM_ADDR_SIZE -
                      em->seed_bits)) | (((t1->dataSize /
                      (em->payload_size/(2 * sizeof(uint32_t)))) - 1) >>
                      (5 - (32 - SB_FE2000_SRAM_ADDR_SIZE - em->seed_bits))),
                      phdl, t1->usemap, slab);
    }
    else {
      slab = em->slabSetTable1ptr(em->ctx, t1 - emp->table1,
               t1->usedSeed, phdl, t1->usemap, slab);
    }

    /* sync, we don't want chip pointers to old data when done */
    slab->opcode = SB_FE2000_DMA_OPCODE_SYNC;
    slab->feAddress = 0;
    slab->words = 0;
    slab = SB_FE2000_SLAB_NEXT(slab);

    /* Free deleted table 2 entries */
    for (p = t1->remEntries; p; p = np) {
      np = p->next;
      p->valid = 0;
      freeT2 (emp, p);
    }
    t1->remEntries = NULL;

    /* mark as valid */
    for (p = t1->entries; p; p = p->next)
      p->valid = 1;

    t1->isDirty = 0;
  } /* for */

  /* the slab is complete, send it to the FE */
  emp->headDirty = t1;  /* we have cleansed these */
  if (!t1)
    emp->tailDirty = &emp->headDirty;

  slab->opcode = SB_FE2000_DMA_OPCODE_END;
  slab->words = 0;

  SB_ASSERT ((uint32_t *)SB_FE2000_SLAB_NEXT(slab) <
              emp->commitSlab.slabEnd);

  emp->commitSlab.slabEm = em;
  emp->commitSlab.slabState = CFlushingDirty;
  emp->commitSlab.slabRunLengthP = rl;
  emp->commitSlab.slabOp.data = &emp->commitSlab;
  emp->commitSlab.slabOp.cb = slabCommitDone;
  emp->commitSlab.slabOp.slab = emp->commitSlab.slabMemory;
  err = sbFe2000DmaSlabRequest (em->pDmaCtxt, em->cParams->clientData,
                                 &emp->commitSlab.slabOp);

  if (err != SB_IN_PROGRESS)
    emp->commitSlab.slabState = CFIdle;

  unblock (em->eml_mutex, s);

  return err;
}


/* -----------------------------------------------------------------------------
 *
 * Callback function invoked when a commit slab dma completes (from
 * flushDirty). Keep re-calling commit until the flush process is complete.
 *
 * ---------------------------------------------------------------------------*/
static void
slabCommitDone (sbFe2000DmaSlabOp_t* op)
{
  struct slabTransfer *xfer = op->data;
  sbEML_p_t em = xfer->slabEm;
  struct emcomp *emp = em->emp;
  sbStatus_t err = op->status;

  switch (emp->commitSlab.slabState) {
    case CFIdle:
      SB_ASSERT (0);/* we should never get here */
      return;

    case CFlushingDirty:
      emp->commitSlab.slabState = CFIdle;
      if (err == SB_OK) {
        /* flushing is in progress, go for it again */
        err = commit (em, emp->commitSlab.slabRunLengthP);
        if (err == SB_IN_PROGRESS) {
          /* we'll get called when it's done */
          return;
        }
      }
      /* slab is done, so call top level call-back */
      break;
  } /* switch */

  emp->committing = 0;
  em->commitDone (em->ctx, err);
}


/* -----------------------------------------------------------------------------
 *
 * Commit the EML changes
 *
 * The param pnRunLength specifies how many updates to process. As long as
 * there are entries on the dirty queue keep calling flushDirty to update
 * the FE.
 *
 * returns: SB_IN_PROGRESS - flush DMA kicked off or queued
 *          SB_MORE - commit completed OK but still entries on dirty queue
 *          SB_OK - DMA completed immediately
 *          SB_BAD_ARGUMENT_ERR_CODE - DMA param error
 *          error code
 *
 * ---------------------------------------------------------------------------*/
static sbStatus_t
commit (sbEML_p_t em, uint32_t *pnRunLength)
{
  sbStatus_t err;

  while (em->emp->headDirty) {
    err = flushDirty (em, pnRunLength);
    if (*pnRunLength == 0)
      err = SB_MORE;

    if (err) {
      return err;
    }
  }

  return SB_OK;
}


/* -----------------------------------------------------------------------------
 *
 * Callback function used by initSRAM. Called when a slab dma operation
 * completes. If the initRAM process is not complete kick off the next slab
 * operation with a call to initSRAM.
 *
 * If the initSRAM process is complete or an error in the dma occured then
 * inform the user application by calling the user callback function
 * emp->initCb
 *
 * ---------------------------------------------------------------------------*/
static void
initSRAMDone (sbFe2000DmaSlabOp_t* op)
{
  sbEML_p_t em = op->data;
  struct emcomp *emp = em->emp;
  sbStatus_t err;

  if (op->status) {
    emp->initCb (emp->initId, op->status);
    return;
  }

  err = initSRAM (em);
  if (err != SB_IN_PROGRESS)
    emp->initCb (emp->initId, err);
}


/* -----------------------------------------------------------------------------
 *
 * Initialize the FE's t1 and t2 lookup tables. It is assumed that the first
 * entry in the t2 table has already been alloced as a dummy entry and this
 * function is free to write it's contents.
 *
 * The first phase is to write the usemap of the dummy entry at the start of
 * the t2 table to zero.
 *
 * Next loop through all of the t1 table entries and write zeros into each
 * value using slab dmas. This initializes all t2 indexes to point to the
 * dummy t2 entry previously set.
 *
 * returns SB_OK
 *         error - error returned from dma
 *
 * ---------------------------------------------------------------------------*/
static sbStatus_t
initSRAM (sbEML_p_t em)
{
  struct emcomp *emp = em->emp;
  sbStatus_t err;
  sbFe2000DmaSlab_t *slab;

  emp->commitSlab.slabEm = em;
  emp->commitSlab.slabOp.data = em;
  emp->commitSlab.slabOp.cb = initSRAMDone;
  emp->commitSlab.slabOp.slab = emp->commitSlab.slabMemory;

  do {
    if (emp->initOffs >= em->table1_count)
      return SB_OK; /* all done */

    slab = emp->commitSlab.slabMemory;
    if (emp->initOffs == 0) {
      struct table2 *t2;
      sbDmaMemoryHandle_t junk;
      junk.handle = NULL;

      err = em->cParams->sbmalloc(em->cParams->clientData,
                                  SB_ALLOC_INTERNAL, em->table2_size,
                                  (void *) &t2, NULL);
      if (err)
        return err;
      SB_MEMSET(t2->packedPayload, 0xff, em->payload_size);
      slab = em->slabTable2entry(em->ctx, emp->zeroHdl, (void *) t2, slab);
      err = em->cParams->sbfree(em->cParams->clientData,
                                SB_ALLOC_INTERNAL, em->table2_size,
                                (void *) t2, junk);
      if (err)
        return err;
    }   /* if emp->initOffs */

    while ((uint32_t *)slab + em->nbank * SIZEOFW(sbFe2000DmaSlab_t) <
            emp->commitSlab.slabEnd && emp->initOffs < em->table1_count) {
      slab->words = em->payload_size/(2*sizeof(uint32_t));
      slab = em->slabSetTable1ptr (em->ctx, emp->initOffs, 0, emp->zeroHdl, 
                                   0 /*usemap of 0 */, slab);
      emp->initOffs++;
    } /* while */

    slab->opcode = SB_FE2000_DMA_OPCODE_END;
    slab->words = 0;
    SB_ASSERT ((uint32_t *)SB_FE2000_SLAB_NEXT(slab) <
               emp->commitSlab.slabEnd);

    err = sbFe2000DmaSlabRequest (em->pDmaCtxt,
                                   em->cParams->clientData,
                                   &emp->commitSlab.slabOp);
  } while (err == SB_OK);

  return err;
}


/* Return table2array ptr - will be used by ager */
void *
sbEMLTable2Array (sbEMLCtx_p emCtxt)
{
  sbEML_p_t emp = (sbEML_p_t)emCtxt;

  return (void *)emp->table2array;
}

int
sbEMLWalkTree (sbEMLCtx_p emCtxt, sbEMLKeyCompare_f_t pfCompare, 
               void *minrtrv, void *rtrv, void **match, void *lukey,
               uint32_t keysize)
{
  sbEML_t *eml = (sbEML_t *)emCtxt;
  struct emcomp *emp = eml->emp;

  int t1count = eml->table1_count;

  sbEMLTable1_t *pT1;
  sbEMLTable2_t *pT2;

  int tid = 0;
  int found = 0;

  for (tid=0; tid < t1count; tid++) {
    pT1 = &emp->table1[tid];
    if(pT1->nused == 0) 
      continue;
    pT2 = pT1->entries;
    while (pT2) {
      (*pfCompare)(eml,pT2,minrtrv,rtrv,match,lukey,&found);
      pT2 = pT2->next;
    }
  }

  return found;
}

int
sbEMLIteratorFirst(sbEMLCtx_p emCtxt, sbEMLKey_p_t key)
{
  sbEML_t *em = (sbEML_t *) emCtxt;
  int t1count = em->table1_count;
  sbEMLTable1_p_t t1;
  sbEMLTable2_p_t t2;
  sbEMLPackedKey_t pkey;
  int i;

  for (i = 0; i < t1count; i++) {
    t1 = &em->emp->table1[i];
    t2 = t1->entries;
    if(t2) {
      em->getKey(em->ctx, PAYLOAD(t2), pkey);
      em->unPackKey(em->ctx, key, pkey);
      return SB_OK;
    }
  }
  return SB_MAC_NOT_FOUND;
}

int
sbEMLIteratorNext(sbEMLCtx_p emCtxt, sbEMLKey_p_t key, sbEMLKey_p_t next)
{
  sbEML_t *em = (sbEML_t *)emCtxt;
  sbEMLPackedKey_t pkey;
  sbEMLTable2_p_t *t2p, t2;
  sbEMLTable1_p_t t1, t1last;

  em->packKey(em->ctx, key, pkey);
  t2p = lookupPKey (em, pkey, &t1);
  if (!t2p) {
    return SB_MAC_NOT_FOUND;
  }

  for (t2 = (*t2p)->next, t1last = &em->emp->table1[em->table1_count-1];
       !t2 && t1 != t1last;
       t1++, t2 = t1->entries);


  if (t2) {
    em->getKey(em->ctx, PAYLOAD(t2), pkey);
    em->unPackKey(em->ctx, next, pkey);
    return SB_OK;
  }

  return SB_MAC_NOT_FOUND;
}
