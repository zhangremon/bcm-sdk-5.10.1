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

  $Id: g2p3_lpm.c 1.54.42.1 Broadcom SDK $

 *****************************************************************************/
#include "sbWrappers.h"
#include "g2p3_int.h"
#include "g2p3_lpm_internal.h"
#include "g2p3_lpm_comp.h"
#include <soc/sbx/sbDq.h>

/* Need to get access to lpm->deviceMem->startAddr during commit()
   for ipv6, in order to produce offsets rather than straight
   ptrs. Perhaps a more elegant way is possible, but for now -
   include the entire header.
 */
#include "g2p3_lpm_device_mem_internal.h"
 
#ifndef LPM_LPM_DEBUG
#define LPM_LPM_DEBUG 0
#endif

#define LPMNODE_SIZE_IN_BYTES 4

/* macro to round up bytes to closest word - needed for sparc malloc */
#define RND_TO_WORD(x) (x%4 ? x+(4-(x%4)) : x)


                                            
/* LPM Fast Incremental update support datastructures*/

typedef struct lpmWork_s lpmWork_t, *lpmWork_p_t;
typedef struct lpmPruneWork_s lpmPruneWork_t, *lpmPruneWork_p_t;

typedef enum {
    LPM_WORK_ADD = 0,
    LPM_WORK_DEL,
    LPM_WORK_UPDATE,
    LPM_WORK_MAX
} lpmWorkType_t;

typedef enum {
    LPM_COMMIT_NORMAL = 0,
    LPM_COMMIT_FAST,
    LPM_COMMIT_MAX
} lpmCommitType_t;

struct lpmWork_s {
  dq_t                    listNode;    
  lpmNode_p_t             node;         /* next to be traversed         */
  uint8_t                 nodeCached;   /* node above is cached         */ 
  g2p3_lpmSramAddr        dest;         /* address in table             */ 
  uint8_t                 slice;        /* # bits left in current slice */
  uint8_t                 *slices;      /* rest of slices               */
  g2p3_lpmPayloadHandle_t payMain;      /* Main payload handle          */
  uint8_t                 pyldCached;   /* Payload above is cached      */ 
  uint8_t                 pfxLen;       /* Prefix length                */
  g2p3_lpmAddress_t       addr;         /* Prefix Address               */
  sbBool_t                lastTblSingleton; /* was last seen table node singleton */
  g2p3_lpmSramAddr        lastTblAddr;      /* last table node address      */  
};

struct lpmPruneWork_s {
  lpmNode_p_t             node;         /* prune node pointer                      */
  uint8_t                 *slices;      /* pointer to previous slice               */
  lpmNode_p_t             *child;       /* Pointer to Child of inserted table node */
};

/* delta updates admitted */
#define G2P3_MAX_DELTA_UPDATE_COUNT (1)
#define G2P3_MAX_PFX_LEN (64)
#define G2P3_MAX_SINGLETON_SLICES   (4)

typedef struct {
  uint32_t        numWork;          /* number of work item used */
  uint32_t        maxWork;          /* maximum number of work item admitted*/ 
  uint32_t        numPruneWork;     /* number of add prune work items */
  /* work item pool */  
  lpmWork_t       work[G2P3_MAX_DELTA_UPDATE_COUNT+1];
  /* work item list */
  dq_t            workList[G2P3_MAX_PFX_LEN+1];
  /* fast single add prune work item pool */
  lpmPruneWork_t  pruneWork[G2P3_MAX_SINGLETON_SLICES];
  /* fast single delete prune work item */
  lpmWork_t       delPruneWork;  
  /* Compiler State variables */
  lpmCommitType_t thisCommitType;  /* state of this commit type */ 
  lpmCommitType_t lastCommitType;  /* state of last commit type */
  lpmWorkType_t   thisWorkType;    /* state of this work type   */
  lpmWorkType_t   lastWorkType;    /* state of last work type   */
  sbBool_t        delPrunePending; /* Did delete compress 
                                    * singleton table */  
} lpmWorkMgr_t,   *lpmWorkMgr_p_t;

static
void g2p3_work_mgr_init(lpmWorkMgr_p_t workmgr);

static 
sbBool_t g2p3_is_fast_update(lpmWorkMgr_p_t workmgr);

static 
lpmWork_p_t g2p3_work_dequeue(lpmWorkMgr_p_t workmgr);

static 
lpmWork_p_t g2p3_work_alloc(lpmWorkMgr_p_t workmgr);

static
void g2p3_work_collapse(lpmWorkMgr_p_t workmgr, 
                        lpmWork_p_t work);

static 
sbStatus_t g2p3_work_enqueue(lpmWorkMgr_p_t workmgr,
                             lpmWork_p_t work);

static 
void g2p3_lpm_prune_work_enqueue(lpmWorkMgr_p_t workmgr,
                                 lpmWork_p_t work, 
                                 lpmNode_p_t newnode);

static
lpmPruneWork_p_t g2p3_lpm_prune_work_dequeue(lpmWorkMgr_p_t workmgr);


#define LPM_WORK_CACHE_NODE(work,newNode)               \
    do {                                                \
        if((work) && (!(work)->nodeCached)) {           \
            SB_ASSERT(newNode);                         \
            /* point the work to this node */           \
            (work)->node = (newNode);                   \
            (work)->nodeCached = 1;                     \
            (work)->pyldCached = 1;                     \
        }                                               \
    } while(0) 

#define LPM_WORK_CACHE_PAYLOAD(work, Node, pyldMain)    \
    do {                                                \
        if((work) && (!(work)->pyldCached)) {           \
            /* point the work to this table node */     \
            (work)->payMain  = (pyldMain);              \
        }                                               \
    } while(0) 

/* Updates the Destination address offset on the work item
 * using the bit value(0 left child, 1 right child)
 * of split node 
 */
#define SPLIT_WORK_OFFSET(work, bit)                \
    do {                                            \
        if((work) && (!(work)->nodeCached)) {       \
            SB_ASSERT((work)->slice > 0);           \
            --(work)->slice;                        \
            if(bit) {                               \
                (work)->dest += 1 << (work)->slice; \
            }                                       \
        }                                           \
    } while(0)                                      \

/* Updates the Destination address offset on the work item
 * using the skip address, skip length and skip length 
 * difference
 */
#define SKIP_WORK_OFFSET(work, addr, len, lendiff)  \
    do {                                            \
        if((work) && (!(work)->nodeCached)) {       \
            SB_ASSERT((work)->slice > 0);           \
            (work)->dest += (addr) << (lendiff);    \
            (work)->slice -= (len);                 \
        }                                           \
    } while(0)                                      \
      
/* Updates the Destination address offset on the work item
 * using the table address
 */
#define TABLE_WORK_OFFSET(work, addr, singleton)    \
    do {                                            \
        if(work && (!work->nodeCached)) {           \
            (work)->slice = *(work)->slices;        \
            ++(work)->slices;                       \
            (work)->dest = (addr);                  \
            (work)->lastTblAddr = (addr);           \
            (work)->lastTblSingleton = singleton;   \
        }                                           \
    } while(0)                                      \

/* For Singleton dynamic table compression and decompression, 
 * if the table compressed or decompressed is the first new node
 * the work item caches this node. CommitPath starts from this nod
 * but the destination address on the work item has to abide to a 
 * condition. If the previous seen table node & was a singleton node
 * the destination address has to be the previous table nodes top address
 * which is nothing but singleton matching entry address. If not the 
 * destination address must be offset derived from skip/split nodes as
 * usual */
#define G2P3_LPM_SNG_ADJ_DEST_OFFSET(work)                    \
  do {                                                        \
      if(work) {                                              \
          if(!work->nodeCached && work->lastTblSingleton) {   \
            work->dest = work->lastTblAddr;                   \
            work->slice = 0;                                  \
          }                                                   \
      }                                                       \
  } while(0)                                                  \

#define G2P3_LPM_IS_FAST_DELETE                                \
               (workmgr->thisCommitType == LPM_COMMIT_FAST && \
                workmgr->thisWorkType == LPM_WORK_DEL)


/*#define TIME_STAMP_DBG */
#undef TIME_STAMP_DBG
#ifdef TIME_STAMP_DBG 
sal_usecs_t        start, end;
#define TIME_STAMP_START start = sal_time_usecs()
#define TIME_STAMP                                                  \
  do {                                                              \
    end = sal_time_usecs();                                         \
    soc_cm_print("\n Time Stamp: [%u]", SAL_USECS_SUB(end, start)); \
  } while(0)
#else
#define TIME_STAMP_START 
#define TIME_STAMP   
#endif

/* requires: 0 < len1 < 64, 0 < len2 < 64; addr1 and addr2 LSB-aligned
 * modifies: nothing
 * effects:  none
 * returns:  length of longest common MSB-aligned prefix
 */
static
unsigned
lcplen(g2p3_lpmAddress_t addr1, unsigned len1,
       g2p3_lpmAddress_t addr2, unsigned len2);

/* requires: nothing
 * modifies: nothing
 * effects:  none
 * returns:  number of device payload refs below this node
 */
static
uint32_t
lpmDpsBelow(const lpmNode_p_t node);

/* requires: node non-null
 * modifies: nothing
 * effects:  none
 * returns:  true iff node is a zombie
 */
static
sbBool_t
lpmIsDead(lpmNode_p_t node);

/* requires: nothing
 * modifies: nothing
 * effects:  none
 * returns:  node if node is live, else first live descendant of node
 */
static
lpmNode_p_t
lpmFirstLive(lpmNode_p_t node);

/* requires: nothing
 * modifies: nothing
 * effects:  none
 * returns:  true iff
 *            - node is a skip node of length slice, or
 *            - node is a skip node with only payload nodes underneath
 */
static
sbBool_t
lpmIsSingletonPath(lpmNode_p_t node, uint8_t slice);

/* requires: pMain, pClsA, pClsB non-NULL; payloadKind valid
 * modifies: one of *pMain, *pClsA, *pClsB depending on node->tag.payloadKind
 * effects:  copies payload contained in node to one of three kinds
 */
static
void
extractPayload(g2p3_lpmPayloadHandle_t *pMain, /* main payload */
               g2p3_lpmClassifierId_t *pClsA,       /* classifier id A */
               g2p3_lpmClassifierId_t *pClsB,       /* classifier id B */
               uint32_t *pDropMaskLo,              /* port drop mask */
               uint32_t *pDropMaskHi,
               g2p3_lpmPayloadKind_t payloadKind, void *payload);

#if 0
/* requires: 0 < nbits <= MAX_SKIP_LEN, addr < (1 << nbits)
 * modifies: nothing
 * effects:  none
 * returns:  addr msbGu-aligned at MAX_SKIP_LEN bits, next bit 1, rest 0
 *           e.g., for MAX_SKIP_LEN=9, addr=101010 (6 bits): 101010100
 */
static
uint32_t
lpmEncodeSkip(g2p3_lpmAddress_t addr, uint8_t nbits);
#endif

/* requires: lpm valid
 * modifies: lpm->dpm, *pCount
 * effects:  increments ref count for all <payload,clsA,clsB> starting at node
 *           until a payload of payKind is reached
 *           if pCount non-null, increments *pCount by number of references
 * returns:  SB_OK on success; error status otherwise
 */
static
sbStatus_t
refDevPayloads(g2p3_lpmDPMHandle_t dpm,
               uint32_t *pCount, lpmNode_p_t node,
               uint8_t payKind,
               void *payload, g2p3_lpmClassifierId_t clsA,
               g2p3_lpmClassifierId_t clsB, uint32_t dropMaskLo,
               uint32_t dropMaskHi);

/* requires: lpm valid
 * modifies: lpm->dpm, *pCount
 * effects:  decrements ref count for all <payload,clsA,clsB> starting at node
 *           until a payload of payKind is reached
 *           if pCount non-null, decrements *pCount by number of dereferences
 * returns:  SB_OK on success; error status otherwise
 */
static
sbStatus_t
derefDevPayloads(g2p3_lpmDPMHandle_t dpm,
                 uint32_t *pCount, lpmNode_p_t node,
                 uint8_t payKind,
                 void *payload, g2p3_lpmClassifierId_t clsA,
                 g2p3_lpmClassifierId_t clsB, uint32_t dropMaskLo,
                 uint32_t dropMaskHi);

/* requires: lpm initialized, pTrie and slices non-null, *slices > 0
 * modifies: *pTrie
 * effects:  removes dead nodes and branches to them from *pTrie
 * returns:  SB_OK
 */
static
sbStatus_t
lpmPruneDead(g2p3_lpmHandle_t lpm, lpmNode_p_t *pTrie,
             const uint8_t *slices);

/* requires: lpm initialized, pTrie and slices non-null, *slices > 0
 * modifies: *pTrie
 * effects:  removes all nodes from the tree
 * returns:  SB_OK
 */
static
sbStatus_t
lpmFinalCleanup(g2p3_lpmHandle_t lpm, const uint8_t *slices,
               lpmNode_p_t trie, g2p3_lpmPayloadHandle_t payload,
               g2p3_lpmClassifierId_t clsA, g2p3_lpmClassifierId_t clsB,
               uint32_t dropMaskLo, uint32_t dropMaskHi);

/* requires: lpm initialized, pTrie and nextSlices non-null
 * modifies: *pTrie and below
 * effects:  adds newPayload of payloadKind at addr/length
 *           payMain, payClsA, and payClsB keep track of payloads
 *           of different kinds encountered so far (for ref/deref)
 *           on failure, *pTrie is not modified
 * returns:  SB_OK on success
 *           SB_LPM_DUPLICATE_ADDRESS if there is already a
 *           payload of payloadKind at addr/length
 *           SB_LPM_OUT_OF_HOST_MEMORY if out of host memory
 *           SB_LPM_OUT_OF_DEVICE_MEMORY if out of device memory
 * notes:    to keep the trie unchanged on failure, all allocations
 *           must be performed before any recursive calls
 *           and all changes to *pTrie and below must be reverted on failure
 */
static
sbStatus_t
lpmAddRoute(g2p3_lpmHandle_t lpm, lpmNode_p_t *pTrie,
            uint8_t thisSlice, uint8_t *nextSlices,	/* zero-terminated */
            g2p3_lpmAddress_t addr, uint8_t length,
            g2p3_lpmPayloadKind_t payloadKind, void *newPayload,
            /* what's been found so far */
            g2p3_lpmPayloadHandle_t payMain,
            g2p3_lpmClassifierId_t payClsA, g2p3_lpmClassifierId_t payClsB,
            uint32_t dropMaskLo, uint32_t dropMaskHi,
            lpmWork_p_t work);

static
sbStatus_t
lpmRemoveMasks (g2p3_lpmHandle_t lpm, lpmNode_p_t node,
                g2p3_lpmAddress_t addr, uint8_t length);

static void
lpmDropDown(g2p3_lpmHandle_t lpm, g2p3_lpmAddress_t addr,
            uint8_t length,
            g2p3_lpmPayloadHandle_t *payMain,
            g2p3_lpmClassifierId_t *pClsA,
            g2p3_lpmClassifierId_t *pClsB,
            uint32_t *pDropMaskLo, uint32_t *pDropMaskHi,
            g2p3_lpmClassifierId_t *pClsC);

/* requires: lpm initialized, pTrie and nextSlices non-null
 * modifies: *pTrie and below
 * effects:  inserts a payload node at *pTrie
 * returns:  SB_OK on success
 *           SB_LPM_DUPLICATE_ADDRESS if there is already a
 *           payload of payloadKind at addr/length
 *           SB_LPM_OUT_OF_HOST_MEMORY if out of host memory
 *           SB_LPM_OUT_OF_DEVICE_MEMORY if out of device memory
 */
static
sbStatus_t
lpmAddPayload(g2p3_lpmHandle_t lpm, lpmNode_p_t *pTrie,
              g2p3_lpmPayloadKind_t payloadKind, void *newPayload,
              /* what's been found so far */
              g2p3_lpmPayloadHandle_t payMain,
              g2p3_lpmClassifierId_t payClsA,
              g2p3_lpmClassifierId_t payClsB,
              uint32_t dropMaskLo, uint32_t dropMaskHi,
              lpmWork_p_t work);

/* requires: lpm initialized, pTrie and nextSlices non-null
 * modifies: *pTrie and below
 * effects:  inserts a table at *pTrie (singleton if requested and possible)
 *           calls lpmAddRoute on trie below the table
 * returns:  SB_OK on success
 *           SB_LPM_DUPLICATE_ADDRESS if there is already a
 *           payload of payloadKind at addr/length
 *           SB_LPM_OUT_OF_HOST_MEMORY if out of host memory
 *           SB_LPM_OUT_OF_DEVICE_MEMORY if out of device memory
 * notes:    to keep the trie unchanged on failure, all allocations
 *           must be performed before any recursive calls
 *           and all changes to *pTrie and below must be reverted on failure
 */
static
sbStatus_t
lpmAddTable(g2p3_lpmHandle_t lpm, lpmNode_p_t *pTrie,
            uint8_t thisSlice, uint8_t *nextSlices,	/* zero-terminated */
            g2p3_lpmAddress_t addr, uint8_t length,
            g2p3_lpmPayloadKind_t payloadKind, void *newPayload,
            sbBool_t singleton,
            /* what's been found so far */
            g2p3_lpmPayloadHandle_t payMain,
            g2p3_lpmClassifierId_t payClsA, g2p3_lpmClassifierId_t payClsB,
            uint32_t dropMaskLo, uint32_t dropMaskHi,
            lpmWork_p_t work);


/* requires: lpm initialized
 * modifies: *pNode if found and pNode non-NULL
 *           *pPayload if found and pPayload non-NULL
 *           *pClsA if found and pClsA non-NULL
 *           *pClsB if found and pClsB non-NULL
 * effects:  finds a payload node of payloadKind at addr/length in lpm trie
 *             if there is a payload node of payloadKind at addr/length,
 *               *pNode is the relevant payload node
 *               *pPayload, *pClsA, and *pClsB keep track of current payloads
 * returns:  SB_OK on success
 *           SB_LPM_ADDRESS_NOT_FOUND otherwise
 */

static
sbStatus_t
lpmFindPayload(g2p3_lpmHandle_t lpm,
               lpmNode_p_t *pNode, g2p3_lpmPayloadHandle_t *pPayload,
               g2p3_lpmClassifierId_t *pClsA, g2p3_lpmClassifierId_t *pClsB,
               uint32_t *pDropMaskLo, uint32_t *pDropMaskHi,
               g2p3_lpmAddress_t addr, uint8_t length,
               g2p3_lpmPayloadKind_t payloadKind, lpmWork_p_t work);

/* requires: lpm initialized
 * modifies: *pTrie and below
 * effects:  removes a payload of payloadKind at addr/length
 *           payMain, payClsA, and payClsB keep track of payloads
 *           of different kinds encountered so far (for ref/deref)
 * returns:  SB_OK on success
 *           SB_LPM_ADDRESS_NOT_FOUND if no payload of
 *           payloadKind exists at addr/length
 */
static
sbStatus_t
lpmRemoveRoute(g2p3_lpmHandle_t lpm, lpmNode_p_t trie,
               g2p3_lpmAddress_t addr, uint8_t length,
               g2p3_lpmPayloadKind_t payloadKind,
               /* what's been found so far */
               g2p3_lpmPayloadHandle_t payMain,
               g2p3_lpmClassifierId_t payClsA, g2p3_lpmClassifierId_t payClsB,
               uint32_t dropMaskLo, uint32_t dropMaskHi,
               lpmWork_p_t work, lpmNode_p_t parent);

/* requires: lpm valid
 * modifies: dpm, *pCount
 * effects:  increments or decrements ref count for all <payload,clsA,clsB>
 *           starting at node until a payload of payKind is reached
 *           if pCount non-null, increments *pCount by number of (de)references
 * returns:  SB_OK on success; error status otherwise
 */

static
sbStatus_t
devPayloadsRec(g2p3_lpmDPMHandle_t dpm, uint32_t *pCount, lpmNode_p_t node,
               sbBool_t ref, /* ref if true; deref otherwise */
               uint8_t payKind, /* where to stop */
               g2p3_lpmPayloadHandle_t payload,
               g2p3_lpmClassifierId_t clsA,
               g2p3_lpmClassifierId_t clsB,
               uint32_t dropMaskLo, uint32_t dropMaskHi);

/* requires: lpm initialized, lpm->busy
 * modifies: *lpm
 * effects:  cleans up after a commit, calls completion callback if appropriate
 * returns:  nothing
 */
static
void
commitDone(g2p3_lpmHandle_t lpm);

/* requires: cbData is an initialized lpm object, lpm->busy
 * modifies: *lpm
 * effects:  cleans up after a commit, calls completion callback if present
 * returns:  nothing
 */
static
void
commitDoneCB(void *cbData, sbStatus_t status);

/* requires: cbData is an initialized lpm object, lpm->busy
 * modifies: *lpm, lpm->deviceMem
 * effects:  calls commit() on the lpm object
 * returns:  nothing
 */
static
void
commitCB(void *cbData, sbStatus_t status);

/* requires: lpm initialized, lpm->busy, lpm contains a non-empty commit path
 * modifies: *lpm, lpm->deviceMem
 * effects:  traverses lpm->trie, starting with lpm->path, and writes a
 *           device representation of the prefix trie to lpm->deviceMem
 *           sets lpm->status to SB_OK on completion, or
 *           SB_IN_PROGRESS if rest of commit will be called
 *           asynchronously after the DMA transfers complete
 * returns:  nothing
 */
static
void
commit(g2p3_lpmHandle_t lpm);

/* requires: path non-null
 * modifies: nothing
 * effects:  none
 * returns:  true iff path has dirtyTable or any of the dirty payloads bits set
 */
static
sbBool_t
lpmAnyDirty(const lpmPath_p_t path);

/* requires: lpm instance
 * modifies: uses path buffer on lpm instance to commit
 * effects:  When prune on delete compresses tables for singleton, this routing
 *           is responsible to transfer the changes to device
 * returns:  none
 */
static
void
commitDelPruneWorkItem(g2p3_lpmHandle_t lpm);

/* see contract in g2p3_lpm.h */
sbStatus_t
g2p3_lpmInit(soc_sbx_g2p3_state_t *fe,
                 g2p3_lpmHandle_t *pLpm,
                 g2p3_lpmHostMemHandle_t hostMem,
                 g2p3_lpmDeviceMemHandle_t deviceMemA,
                 g2p3_lpmDeviceMemHandle_t deviceMemB,
                 g2p3_lpmDmaHandle_t dma, uint32_t dmaBankA, uint32_t dmaBankB,
                 uint32_t paylDmaBankA, uint32_t paylDmaBankB, 
                 uint32_t paylBanks,
                 g2p3_lpmSramAddr topTableAddress,
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
                 sbBool_t ipv6)
{
    sbStatus_t status;
    void *vp;
    uint8_t *p;
    g2p3_lpmHandle_t lpm;
    int sliceSlots;
    int addrBits;
    int hostAllocBytes;
    uint8_t maxPath;
    g2p3_lpmPayloadHandle_t payload;
    lpmWorkMgr_p_t workMgr;

    SB_ASSERT(fe);
    SB_ASSERT(pLpm);
    SB_ASSERT(hostMem);
    SB_ASSERT(deviceMemA);
    SB_ASSERT(deviceMemB);
    SB_ASSERT(slices);
    SB_ASSERT(slices[0] != 0);
    for (sliceSlots=addrBits=0; slices[sliceSlots] != 0; ++sliceSlots) {
        SB_ASSERT(slices[sliceSlots] <= 32);
        addrBits += slices[sliceSlots];
    }
    ++sliceSlots; /* for terminating 0 */

    maxPath = sliceSlots + addrBits;
    hostAllocBytes = (sizeof(g2p3_lpm_t)          /* lpm struct */
                      + RND_TO_WORD(sliceSlots) * sizeof(uint8_t)  /* slices */
                      + maxPath * sizeof(lpmPath_t)); /* queue for commit */
    status = g2p3_lpmHostMemAlloc(hostMem, &vp, hostAllocBytes, "lpm mgr");
    if (status != SB_OK) return status;

    p = vp;
    lpm = (g2p3_lpmHandle_t) p;
    p += sizeof(g2p3_lpm_t);

    status = g2p3_lpmHostMemAlloc(hostMem, &vp, sizeof(lpmWorkMgr_t), "lpm wq mgr");
    if (status != SB_OK){
        g2p3_lpmHostMemFree(hostMem, lpm, hostAllocBytes);    
        return status;
    }

    lpm->lpmWorkMgr = vp;


    status = g2p3_lpmHPMInit(&lpm->hpm, hostMem, bytesPerPayload, ipv6);
    if (status != SB_OK) {
        g2p3_lpmHostMemFree(hostMem, lpm->lpmWorkMgr, sizeof(lpmWorkMgr_t));
        g2p3_lpmHostMemFree(hostMem, lpm, hostAllocBytes);
        return status;
    }

    lpm->payloadSize = wordsPerDevicePayload * 4;
    status = g2p3_lpmDPMInit( &lpm->dpm, hostMem, deviceMemB, deviceMemA, dma, 
                              paylDmaBankB, paylDmaBankA, paylBanks,
                              payloadPack, wordsPerDevicePayload, fe->unit);
    if (status != SB_OK) {
        SB_ASSERT(g2p3_lpmHPMUninit(&lpm->hpm) == SB_OK);
        SB_ASSERT(g2p3_lpmHostMemFree(hostMem, lpm->lpmWorkMgr, sizeof(lpmWorkMgr_t)));
        SB_ASSERT(g2p3_lpmHostMemFree(hostMem, lpm, hostAllocBytes)
               == SB_OK);
        return status;
    }
    status = g2p3_lpmHPMRef(lpm->hpm, &payload, defaultPayload);
    if (status != SB_OK) {
        SB_ASSERT(g2p3_lpmDPMUninit(&lpm->dpm) == SB_OK);
        SB_ASSERT(g2p3_lpmHPMUninit(&lpm->hpm) == SB_OK);
        SB_ASSERT(g2p3_lpmHostMemFree(hostMem, lpm->lpmWorkMgr, sizeof(lpmWorkMgr_t)));
        SB_ASSERT(g2p3_lpmHostMemFree(hostMem, lpm, hostAllocBytes)
               == SB_OK);
        return status;
    }

    status = g2p3_lpmDPMRef(lpm->dpm, payload, defaultClsA, defaultClsB,
                                defaultDropMaskLo, defaultDropMaskHi);
    if (status != SB_OK) {
        SB_ASSERT(g2p3_lpmHPMDeref(lpm->hpm, payload) == SB_OK);
        SB_ASSERT(g2p3_lpmDPMUninit(&lpm->dpm) == SB_OK);
        SB_ASSERT(g2p3_lpmHPMUninit(&lpm->hpm) == SB_OK);
        SB_ASSERT(g2p3_lpmHostMemFree(hostMem, lpm->lpmWorkMgr, sizeof(lpmWorkMgr_t)));
        SB_ASSERT(g2p3_lpmHostMemFree(hostMem, lpm, hostAllocBytes)
               == SB_OK);
        return status;
    }

    lpm->fe   = fe;
    lpm->trie = NULL;
    lpm->defaultPayload = payload;
    lpm->defaultClsA = defaultClsA;
    lpm->defaultClsB = defaultClsB;
    lpm->defaultDropMaskLo = defaultDropMaskLo;
    lpm->defaultDropMaskHi = defaultDropMaskHi;
    lpm->addrBits = addrBits;
    lpm->optSingleton = optSingleton;
    lpm->topTableAddress = topTableAddress;
    lpm->hostMem = hostMem;
    lpm->hostAllocBytes = hostAllocBytes;
    lpm->deviceMemA = deviceMemA;
    lpm->deviceMemB = deviceMemB;
    lpm->dma = dma;
    lpm->dmaBankA = dmaBankA; 
    lpm->dmaBankB = dmaBankB;
    SB_MEMCPY(p, slices, sliceSlots);
    lpm->slices = (uint8_t *) p;
    p += RND_TO_WORD(sliceSlots);

    lpm->path = (lpmPath_p_t) p;
    p += maxPath * sizeof(lpmPath_t);
    lpm->maxPathLength = maxPath;
    lpm->pathLength = 0;
    lpm->busy = FALSE;
    lpm->async = FALSE;
    lpm->resetCommit = TRUE;

    lpm->ipv6 = ipv6;

    workMgr = (lpmWorkMgr_p_t)lpm->lpmWorkMgr;


    
    g2p3_work_mgr_init(workMgr);
    workMgr->thisWorkType     = LPM_WORK_MAX;
    workMgr->lastWorkType     = LPM_WORK_MAX;
    workMgr->thisCommitType   = LPM_COMMIT_MAX;
    workMgr->lastCommitType   = LPM_COMMIT_MAX;
    workMgr->delPrunePending  = FALSE;

    /* clear statistics */
    g2p3_lpm_stats_clear(lpm);

    *pLpm = lpm;
    return SB_OK;
}

/* see prototype for contract */
static
sbStatus_t
lpmPruneDead(g2p3_lpmHandle_t lpm, lpmNode_p_t *pTrie,
             const uint8_t *slices)
{
    sbStatus_t status;
    lpmNode_p_t node;
    sbBool_t kill = FALSE;   /* kill this node? */
    lpmNode_p_t next = NULL;      /* replacement for node */
    /* point work item to delete prune work item */
    lpmWork_p_t work;
    sbBool_t lastTblSingleton = FALSE;
    g2p3_lpmSramAddr lastTblAddr = 0;
    lpmWorkMgr_p_t workmgr; 

    SB_ASSERT(pTrie);
    SB_ASSERT(slices);
    SB_ASSERT(*slices > 0);
    SB_ASSERT(lpm);

    workmgr = (lpmWorkMgr_p_t)lpm->lpmWorkMgr;
    SB_ASSERT(workmgr);
    work = &workmgr->delPruneWork;

    if (*pTrie == NULL) return SB_OK;
    node = *pTrie;
    switch (node->tag.type) {
    case LPM_NODE_PAYLOAD:
        SB_ASSERT(node->dpsBelow > 0 || node->tag.state == LPM_NODE_STATE_DEAD);
        if (node->tag.state == LPM_NODE_STATE_DEAD) {
            kill = TRUE;
            if (node->tag.payloadKind == G2P3_LPM_PAYLOAD_MAIN) {
                status =
                    g2p3_lpmHPMDeref(lpm->hpm, node->data.payload.desc);
                SB_ASSERT(status == SB_OK);
            }
        } else {
            /* Cache the payload for fast delete to aid commit after prune */
            if(G2P3_LPM_IS_FAST_DELETE) {
                /* no required to use extract payload, if needed uses extract payload in
                 * future */
                LPM_WORK_CACHE_PAYLOAD(work, node,
                                       (g2p3_lpmPayloadHandle_t)node->data.payload.desc);  
            }
        }
        
        status = lpmPruneDead(lpm, &node->data.payload.child, slices);
        SB_ASSERT(status == SB_OK);
        SB_ASSERT(node->data.payload.child == NULL
               || node->data.payload.child->dpsBelow > 0);
        if (kill) next = node->data.payload.child;
        break;
    case LPM_NODE_TABLE:
        if (node->tag.state != LPM_NODE_STATE_DEAD) ++slices;
        SB_ASSERT(*slices != 0);
        
        /* In fast delete, if a table is dead it can be assumed that singleton
         * is not performed */
        if(G2P3_LPM_IS_FAST_DELETE && 
           (node->tag.state != LPM_NODE_STATE_DEAD)) {
           if(work) {
               lastTblSingleton = work->lastTblSingleton;
               lastTblAddr = work->lastTblAddr;
           }
           TABLE_WORK_OFFSET(work, node->data.table.address, node->tag.singleton); 
        }

        status = lpmPruneDead(lpm, &node->data.table.child, slices);
        SB_ASSERT(status == SB_OK);
        SB_ASSERT(node->tag.state == LPM_NODE_STATE_DEAD
               || ((node->dpsBelow == 0) == (node->data.table.child == NULL)));
        if (node->tag.state == LPM_NODE_STATE_DEAD || node->dpsBelow == 0) {
            size_t tableSize = node->tag.singleton ? 2 : 1 << *slices;
            SB_ASSERT(node->tag.state == LPM_NODE_STATE_DEAD
                   || node->dpsBelow > 0
                   || node->data.table.child == NULL);
            kill = TRUE;
            next = node->data.table.child;
            if ((int)(slices - lpm->slices) % 2) {
              status = g2p3_lpmDeviceMemFree(lpm->deviceMemB,
                                            node->data.table.address,
                                            tableSize);
            } else {
              status = g2p3_lpmDeviceMemFree(lpm->deviceMemA,
                                           node->data.table.address,
                                           tableSize);
            }
            SB_ASSERT(status == SB_OK);
        } else if (lpm->optSingleton && !node->tag.singleton
                   && lpmIsSingletonPath(node->data.table.child, *slices)) {
            /* convert to singleton table */
            void *vp;
            status = g2p3_lpmHostMemAlloc(lpm->hostMem,
                                              &vp, sizeof(lpmNode_t), "lpm node");
            if (status == SB_OK) {
                lpmNode_p_t newNode = (lpmNode_p_t) vp;
                newNode->tag.type = LPM_NODE_TABLE;
                newNode->tag.singleton = TRUE;
                newNode->tag.state = LPM_NODE_STATE_NEW;
                newNode->dpsBelow = node->dpsBelow;
                newNode->data.table.child = node->data.table.child;
                if ((int)(slices - lpm->slices) % 2) {
                  status =
                    g2p3_lpmDeviceMemAlloc(lpm->deviceMemB,
                                          &newNode->data.table.address,
                                          2);
                } else {
                  status =
                    g2p3_lpmDeviceMemAlloc(lpm->deviceMemA,
                                          &newNode->data.table.address,
                                          2);
                }
                if (status == SB_OK) {
                    node->tag.state = LPM_NODE_STATE_DEAD;
                    kill = TRUE;
                    next = newNode;
                    if ((int)(slices - lpm->slices) % 2) {
                      status =
                        g2p3_lpmDeviceMemFree(lpm->deviceMemB,
                                                  node->data.table.address,
                                                  1 << *slices);
                    } else {
                      status =
                        g2p3_lpmDeviceMemFree(lpm->deviceMemA,
                                                  node->data.table.address,
                                                  1 << *slices);
                    }
                    SB_ASSERT(status == SB_OK);
                    /* soc_cm_print("\n***** Delete Compressed Table ******"); */
                    workmgr->delPrunePending = TRUE; 

                    /* If fast delete, cache this node for commit after prune */
                    if(G2P3_LPM_IS_FAST_DELETE) {
                      if(work) {
                          work->lastTblSingleton = lastTblSingleton;
                          work->lastTblAddr = lastTblAddr;
                      }                      
                      /* If work item node cached is not set, it means this singleton
                       * table is the first node decompressed and will be book marked. 
                       * If previous table was singleton, reset destination address on
                       * the work item */
                      G2P3_LPM_SNG_ADJ_DEST_OFFSET(work);
                      LPM_WORK_CACHE_NODE(work, newNode);
                      work->slices--;
                    }                    
                    
                } else { /* device alloc failed; sweep under carpet */
                    status = g2p3_lpmHostMemFree(lpm->hostMem, newNode,
                                                     sizeof(lpmNode_t));
                    SB_ASSERT(status == SB_OK);
                }
            } else { /* host alloc failed; sweep under carpet */
                status = SB_OK;
            } /* if host alloc success */
        } /* if singleton enabled and not singleton table but singleton path */
        break;
    case LPM_NODE_SKIP:

        if(G2P3_LPM_IS_FAST_DELETE &workmgr->delPrunePending) {
            SKIP_WORK_OFFSET(work, node->data.skip.address,
                             node->tag.skipLen + 1, 
                             work->slice - node->tag.skipLen - 1);
        }
        
        status = lpmPruneDead(lpm, &node->data.skip.child, slices);
        SB_ASSERT(status == SB_OK);
        SB_ASSERT((node->dpsBelow == 0) == (node->data.skip.child == NULL));
        if (node->dpsBelow == 0) {
            kill = TRUE;
            next = NULL;
        } else if (node->data.skip.child
                   && node->data.skip.child->tag.type == LPM_NODE_SKIP) {
            /* consolidate with child SKIP node */
            uint32_t len1, len2;
            next = node->data.skip.child;
            len1 = node->tag.skipLen + 1;
            len2 = next->tag.skipLen + 1;
            SB_ASSERT(len1 + len2 <= *slices);
            next->data.skip.address |= (node->data.skip.address << len2);
            next->tag.skipLen += len1;
            kill = TRUE;
        }
        break;
    case LPM_NODE_SPLIT:
    {
        uint32_t destAddr=0, slice=0;
        uint8_t  *pslices = NULL;
        g2p3_lpmPayloadHandle_t payMain=NULL;
        
        if(G2P3_LPM_IS_FAST_DELETE && !workmgr->delPrunePending) {
            destAddr = work->dest;
            slice    = work->slice;
            payMain  = work->payMain;
            pslices  = work->slices;
            SPLIT_WORK_OFFSET(work,0);
        }
        
        status = lpmPruneDead(lpm, &node->data.split.children[0], slices);
        SB_ASSERT(status == SB_OK);

        if(G2P3_LPM_IS_FAST_DELETE && !workmgr->delPrunePending) {
            work->dest = destAddr;
            work->slice = slice;
            work->payMain = payMain;
            work->slices  = pslices;
            SPLIT_WORK_OFFSET(work,1);
        }        
        status = lpmPruneDead(lpm, &node->data.split.children[1], slices);
        SB_ASSERT(status == SB_OK);
        SB_ASSERT((node->dpsBelow == 0)
               == (node->data.split.children[0] == NULL
                   && node->data.split.children[1] == NULL));
        if (node->dpsBelow == 0) {
            kill = TRUE;
            next = NULL;
        } else if (node->data.split.children[0] == NULL
                   || node->data.split.children[1] == NULL) {
            /* convert to SKIP node */
            uint32_t address = node->data.split.children[0] == NULL;
            lpmNode_p_t child = node->data.split.children[address];
            node->tag.type = LPM_NODE_SKIP;
            node->tag.skipLen = 0; /* 1 bit */
            node->data.skip.address = address;
            node->data.skip.child = child;

            /* try to prune the new skip point to fuse it with underneath
             * skip node if applicable */
            if (child
                && child->tag.type == LPM_NODE_SKIP) {
              /* consolidate with child SKIP node */
              uint32_t len1, len2;
              next = child;
              len1 = node->tag.skipLen + 1;
              len2 = next->tag.skipLen + 1;
              SB_ASSERT(len1 + len2 <= *slices);
              next->data.skip.address |= (node->data.skip.address << len2);
              next->tag.skipLen += len1;
              kill = TRUE;
            }
        }
    }
        break;
    default:
        SB_ASSERT(0);
        return SB_FAILED;
    }
    /* deallocate node if kill */
    if (kill) {
        status =
            g2p3_lpmHostMemFree(lpm->hostMem, node, sizeof(lpmNode_t));
        SB_ASSERT(status == SB_OK);
        *pTrie = next;
    }
    return SB_OK;
}

static
void
lpmUpdateChildPointer(lpmPath_p_t path, lpmNode_p_t child)
{
  if(path) {
    if(path->parent) {
      switch(path->parent->tag.type) {
      case LPM_NODE_PAYLOAD:
        path->parent->data.payload.child = child;
        break;
        
      case LPM_NODE_TABLE:
        path->parent->data.table.child = child;
      break;
      
      case LPM_NODE_SPLIT:
        path->parent->data.split.children[(path-1)->child] = child;
        break;
        
      case LPM_NODE_SKIP:
        path->parent->data.skip.child = child;
        break;
        
      default:
        SB_ASSERT(0);
      }
    }
  }
}



sbStatus_t
lpmPruneDeadPrefixPath(g2p3_lpmHandle_t lpm,
                       g2p3_lpmAddress_t addr,  
                       uint8_t Length)
{
    lpmWorkMgr_p_t workmgr; 
    sbStatus_t status = SB_OK;
    sbBool_t kill = FALSE;   /* kill this node? */
    sbBool_t singletonCompression = FALSE;
    lpmNode_p_t next = NULL;      /* replacement for node */
    lpmNode_p_t killNode = NULL;
    /* point work item to delete prune work item */
    lpmWork_p_t work;
    unsigned lcp;
    uint8_t incrLength=0, decLength=Length, pk;
    lpmPath_p_t path;
    uint32_t len1, len2;

    SB_ASSERT(lpm);
    SB_ASSERT(lpm->pathLength == 0);
    path = lpm->path;
#if LPM_LPM_DEBUG
    SB_LOG("\n %s : Addr %x: Len %d", __FUNCTION__, (uint32)addr, Length);
#endif
    lpm->async = FALSE;
    path->node = lpm->trie;
    path->parent = NULL;
    path->dest = lpm->topTableAddress;
    path->slice = *lpm->slices;
    path->slices = lpm->slices+1;
    path->seen = 0;
    path->payload = lpm->defaultPayload;
    path->clsA = lpm->defaultClsA;
    path->clsB = lpm->defaultClsB;
    path->dropMaskLo = lpm->defaultDropMaskLo;
    path->dropMaskHi = lpm->defaultDropMaskHi;
    path->clsC = 0;
    for (pk = G2P3_LPM_PAYLOAD_MAIN;
         pk < G2P3_LPM_PAYLOAD_NUM_KINDS; ++pk)
        path->dirty[pk] = FALSE;
    path->dirtyTable = FALSE;
    lpm->status = g2p3_lpmDPMInfo(lpm->dpm,
                                  &path->payloadDevAddr,
                                  &path->payloadNodeExtra,
                                  path->payload,
                                  path->clsA,
                                  path->clsB,
                                  path->dropMaskLo,
                                  path->dropMaskHi);
    SB_ASSERT(lpm->status == SB_OK);
    lpm->pathLength = 1;
    
    workmgr = (lpmWorkMgr_p_t)lpm->lpmWorkMgr;
    SB_ASSERT(workmgr);
    work = &workmgr->delPruneWork;

    while(lpm->pathLength > 0) {

        switch (path->node->tag.type) {

        case LPM_NODE_PAYLOAD:

          if(Length == incrLength) {
            SB_ASSERT((path->node->tag.type == LPM_NODE_PAYLOAD) && 
                      (path->node->tag.state == LPM_NODE_STATE_DEAD));
            kill = TRUE;
            killNode = path->node;

            if (killNode->tag.payloadKind == G2P3_LPM_PAYLOAD_MAIN) {
                status = g2p3_lpmHPMDeref(lpm->hpm, killNode->data.payload.desc);
                SB_ASSERT(status == SB_OK);
            }

            lpmUpdateChildPointer(path, path->node->data.payload.child);
            --lpm->pathLength;
            --path;
          } else {
            SB_ASSERT(path->node->dpsBelow > 0 && path->node->tag.state != LPM_NODE_STATE_DEAD);
            path->parent = path->node;
            /* save this best match payload on the path used for commit */
            path->payload = path->node->data.payload.desc;
            /* move to the child skipping this payload node */
            path->node = path->node->data.payload.child;
          }
          break;

        case LPM_NODE_TABLE:

          if(path->seen) {

            /* not expected to see any lpm nodes dead since it is not a l3 cached commit */
              SB_ASSERT(path->node->tag.state != LPM_NODE_STATE_DEAD);
              if (path->node->dpsBelow == 0) {
                size_t tableSize = path->node->tag.singleton ? 2 : 1 << *path->slices;
                SB_ASSERT(path->node->tag.state == LPM_NODE_STATE_DEAD
                       || path->node->dpsBelow > 0
                       || path->node->data.table.child == NULL);

                kill = TRUE;
                killNode = path->node;
                lpmUpdateChildPointer(path, NULL);

                if ((int)(path->slices - lpm->slices) % 2) {
                  status = g2p3_lpmDeviceMemFree(lpm->deviceMemB,
                                                path->node->data.table.address,
                                                tableSize);
                } else {
                  status = g2p3_lpmDeviceMemFree(lpm->deviceMemA,
                                               path->node->data.table.address,
                                               tableSize);
                }
                SB_ASSERT(status == SB_OK);
              } else if (lpm->optSingleton && !path->node->tag.singleton
                         && lpmIsSingletonPath(path->node->data.table.child, *path->slices)) {
                /* convert to singleton table */
                void *vp;
                status = g2p3_lpmHostMemAlloc(lpm->hostMem,
                                                  &vp, sizeof(lpmNode_t), "lpm node");
                if (status == SB_OK) {
                    lpmNode_p_t newNode = (lpmNode_p_t) vp;
                    newNode->tag.type = LPM_NODE_TABLE;
                    newNode->tag.singleton = TRUE;
                    newNode->tag.state = LPM_NODE_STATE_NEW;
                    newNode->dpsBelow = path->node->dpsBelow;
                    newNode->data.table.child = path->node->data.table.child;
                    if ((int)(path->slices - lpm->slices) % 2) {
                      status =
                        g2p3_lpmDeviceMemAlloc(lpm->deviceMemB,
                                              &newNode->data.table.address,
                                              2);
                    } else {
                      status =
                        g2p3_lpmDeviceMemAlloc(lpm->deviceMemA,
                                              &newNode->data.table.address,
                                              2);
                    }
                    if (status == SB_OK) {
                        path->node->tag.state = LPM_NODE_STATE_DEAD;
                        kill = TRUE;
                        killNode = path->node;
                        lpmUpdateChildPointer(path, newNode);
                        SB_ASSERT(newNode->dpsBelow != 0);

                        if ((int)(path->slices - lpm->slices) % 2) {
                          status =
                            g2p3_lpmDeviceMemFree(lpm->deviceMemB,
                                                  path->node->data.table.address,
                                                  1 << *path->slices);
                        } else {
                          status =
                            g2p3_lpmDeviceMemFree(lpm->deviceMemA,
                                                  path->node->data.table.address,
                                                  1 << *path->slices);
                        }
                        SB_ASSERT(status == SB_OK);

                        /* soc_cm_print("\n***** Delete Compressed Table ******"); */
                        workmgr->delPrunePending = TRUE; 

                        /* If fast delete, cache this node for commit after prune */
                        /* The prune path walks down to top. So always set node,payload cache
                         * let last applicable compressed singleton table be the pivot for commit*/
                        singletonCompression = TRUE;
                    } else {
                        /* device alloc failed; sweep under carpet */
                        status = g2p3_lpmHostMemFree(lpm->hostMem, newNode,
                                                         sizeof(lpmNode_t));
                        kill = FALSE;
                        SB_ASSERT(status == SB_OK);
                    }
                } else { /* host alloc failed; sweep under carpet */
                    status = SB_OK;
                } /* if host alloc success */                
              } else {
                if(singletonCompression) {
                  singletonCompression = FALSE;
                  work->dest       = path->node->data.table.address;
                  work->slices     = path->slices;
                  work->slice      = 0; 
                  work->payMain    = path->payload;
                  work->node       = path->node;
                  work->nodeCached = 1;
                  work->pyldCached = 1;
                }
              }
              --lpm->pathLength;
              --path;
          } else {
            SB_ASSERT(lpm->pathLength < lpm->maxPathLength);
            *(path+1) = *path;
            path->seen++;

            path++;
            path->slice = *path->slices;
            ++path->slices;
            path->dest = path->node->data.table.address;
            path->parent = path->node;
            path->node = path->node->data.table.child;
            SB_ASSERT(path->node->tag.state != LPM_NODE_STATE_DEAD);
            ++lpm->pathLength;
          } 
          break;

          case LPM_NODE_SPLIT:

            if (path->seen) {
              if ((path->node->dpsBelow == 0) || 
                  (path->node->data.split.children[0] == NULL || 
                   path->node->data.split.children[1] == NULL)) {

                  /* convert to SKIP node */
                  next = path->child ? path->node->data.split.children[0]:
                                       path->node->data.split.children[1];

                  path->node->tag.type = LPM_NODE_SKIP;
                  path->node->tag.skipLen = 0; /* 1 bit */
                  path->node->data.skip.address = path->child ? 0: 1;
                  path->node->data.skip.child = next;
                  path->node->dpsBelow = lpmDpsBelow(path->node);
                  SB_ASSERT(path->node->dpsBelow != 0);

                  /* try to prune the new skip point to fuse it with underneath
                   * skip node if applicable */
                  if (next && next->tag.type == LPM_NODE_SKIP) {
                    /* consolidate with child SKIP node */
                    len1 = path->node->tag.skipLen + 1;
                    len2 = next->tag.skipLen + 1;
                    SB_ASSERT(len1 + len2 <= *(path->slices-1));
                    next->data.skip.address |= (path->node->data.skip.address << len2);
                    next->tag.skipLen += len1;
                    kill = TRUE;
                    killNode = path->node;
                    lpmUpdateChildPointer(path, next);
                    SB_ASSERT(next->dpsBelow != 0);
                  } 
              }

              --lpm->pathLength;
              --path;
            } else {
              SB_ASSERT(lpm->pathLength < lpm->maxPathLength);
              SB_ASSERT(path->slice > 0);
              /* with single delete model, both split node must exist */
              SB_ASSERT(path->node->data.split.children[1] && path->node->data.split.children[0]);
            
              --path->slice;

              path->dest += 1 << path->slice;
              *(path+1) = *path;
              path->seen++;
              path->child = ((addr >> (decLength - 1)) & 0x1);

              ++path;
              path->parent = path->node;
              path->node = path->node->data.split.children[(path-1)->child];
              ++lpm->pathLength;
              ++incrLength;
              --decLength;
          }
          break;

          case LPM_NODE_SKIP:
          {
            uint8_t skipLen = path->node->tag.skipLen + 1;
            uint8_t skipLenDiff = path->slice - skipLen;
            uint32_t skipAddr = path->node->data.skip.address;
            SB_ASSERT(path->slice >= skipLen);        

            if(path->seen) {
              SB_ASSERT((path->node->dpsBelow == 0) == (path->node->data.skip.child == NULL));
              if (path->node->dpsBelow == 0) {
                kill = TRUE;
                killNode = path->node;
                lpmUpdateChildPointer(path, NULL);
              } else if (path->node->data.skip.child
                         && path->node->data.skip.child->tag.type == LPM_NODE_SKIP) {
                /* consolidate with child SKIP node */
                next = path->node->data.skip.child;
                len1 = path->node->tag.skipLen + 1;
                len2 = next->tag.skipLen + 1;
                SB_ASSERT(len1 + len2 <= *(path->slices-1));
                next->data.skip.address |= (path->node->data.skip.address << len2);
                next->tag.skipLen += len1;

                kill = TRUE;
                killNode = path->node;
                lpmUpdateChildPointer(path, next);
                SB_ASSERT(next->dpsBelow != 0);
              }

              --lpm->pathLength;
              --path;
            } else {
              SB_ASSERT(lpm->pathLength < lpm->maxPathLength);
              *(path + 1) = *path;
              ++path->seen;
              lcp = lcplen(addr, decLength, 
                           path->node->data.skip.address,
                           path->node->tag.skipLen + 1);

              if (lcp < path->node->tag.skipLen + 1) /* stem does not match */
                SB_ASSERT(0);

              ++path;
              path->slice -= skipLen;
              path->dest += skipAddr << skipLenDiff;

              incrLength += skipLen;
              decLength  -= skipLen;
              path->parent = path->node;
              path->node = path->node->data.skip.child;
              ++lpm->pathLength;
            }
          }
          break;

          default:
          SB_ASSERT(0);
          return SB_FAILED;
        }

        if(kill) {
          status = g2p3_lpmHostMemFree(lpm->hostMem, killNode, sizeof(lpmNode_t));
          SB_ASSERT(status == SB_OK);      
          kill = FALSE;
        }
    }
    if (work && singletonCompression) {
          work->node = lpm->trie;
          work->dest = lpm->topTableAddress;
          work->slices = lpm->slices+1;
          work->slice = *lpm->slices;
          work->payMain    = lpm->defaultPayload;
          work->nodeCached = 1;
          work->pyldCached = 1;
    }

    return status;
 }

/*
 * Used by sbGuFlushIpv4Sa()/sbGuFlushIpv4Da() - remove all payloads,
 * including prefix 0
 */
sbStatus_t
g2p3_lpmFlush (g2p3_lpmHandle_t lpm)
{
    sbStatus_t status;

    status = lpmFinalCleanup(lpm, lpm->slices, lpm->trie, lpm->defaultPayload,
                             lpm->defaultClsA, lpm->defaultClsB,
                             lpm->defaultDropMaskLo, lpm->defaultDropMaskHi);
    if (SB_OK == status)
      lpm->trie = NULL;

    return (status);
}

/* see prototype for contract */
static
sbStatus_t
lpmFinalCleanup(g2p3_lpmHandle_t lpm, const uint8_t *slices,
               lpmNode_p_t trie, g2p3_lpmPayloadHandle_t payload,
               g2p3_lpmClassifierId_t clsA, g2p3_lpmClassifierId_t clsB,
               uint32_t dropMaskLo, uint32_t dropMaskHi)
{
    sbStatus_t status;

    if (trie == NULL)
        return SB_OK;
    switch (trie->tag.type) {
    case LPM_NODE_PAYLOAD:
        if (trie->tag.state != LPM_NODE_STATE_DEAD) {
            extractPayload(&payload, &clsA, &clsB, &dropMaskLo,
                           &dropMaskHi,
                           trie->tag.payloadKind, trie->data.payload.desc);
        }
        status = lpmFinalCleanup(lpm, slices, trie->data.payload.child,
                                 (g2p3_lpmPayloadHandle_t) payload,
                                 clsA, clsB, dropMaskLo, dropMaskHi);
        SB_ASSERT(status == SB_OK);

        if (trie->tag.state != LPM_NODE_STATE_DEAD) {
            status = g2p3_lpmDPMDeref(lpm->dpm, payload, clsA, clsB,
                         dropMaskLo, dropMaskHi);
            SB_ASSERT(status == SB_OK);
        }

        if (trie->tag.payloadKind == G2P3_LPM_PAYLOAD_MAIN) {
            status = g2p3_lpmHPMDeref(lpm->hpm, trie->data.payload.desc);
            SB_ASSERT(status == SB_OK);
        }
        break;
    case LPM_NODE_SKIP:
        status = lpmFinalCleanup(lpm, slices, trie->data.skip.child,
                                 (g2p3_lpmPayloadHandle_t) payload,
                                 clsA, clsB, dropMaskLo, dropMaskHi);
        SB_ASSERT(status == SB_OK);
        break;
    case LPM_NODE_TABLE:
        if (trie->tag.state != LPM_NODE_STATE_DEAD) {
          ++slices;
          SB_ASSERT(*slices != 0);
        }
        if ((int)(slices - lpm->slices) % 2) {
          status =
            g2p3_lpmDeviceMemFree(lpm->deviceMemB, trie->data.table.address,
                                      trie->tag.singleton ? 2 : 1 << *slices);
        } else {
          status =
            g2p3_lpmDeviceMemFree(lpm->deviceMemA, trie->data.table.address,
                                      trie->tag.singleton ? 2 : 1 << *slices);
        }
        SB_ASSERT(status == SB_OK);
        status = lpmFinalCleanup(lpm, slices, trie->data.table.child,
                                 (g2p3_lpmPayloadHandle_t) payload,
                                 clsA, clsB, dropMaskLo, dropMaskHi);
        SB_ASSERT(status == SB_OK);
        break;
    case LPM_NODE_SPLIT:
        status = lpmFinalCleanup(lpm, slices, trie->data.split.children[0],
                                 (g2p3_lpmPayloadHandle_t) payload,
                                 clsA, clsB, dropMaskLo, dropMaskHi);
        SB_ASSERT(status == SB_OK);
        status = lpmFinalCleanup(lpm, slices, trie->data.split.children[1],
                                 (g2p3_lpmPayloadHandle_t) payload,
                                 clsA, clsB, dropMaskLo, dropMaskHi);
        SB_ASSERT(status == SB_OK);
        break;
    default:
        SB_ASSERT(0);
        return SB_FAILED;
    }
    status = g2p3_lpmHostMemFree(lpm->hostMem, trie, sizeof(lpmNode_t));
    SB_ASSERT(status == SB_OK);
    return status;
}

/* see contract in g2p3_lpm.h */
sbStatus_t
g2p3_lpmUninit(g2p3_lpmHandle_t *pLpm)
{
    sbStatus_t status;
    g2p3_lpmHandle_t lpm;

    SB_ASSERT(pLpm);
    lpm = *pLpm;
    SB_ASSERT(lpm);

    status = lpmFinalCleanup(lpm, lpm->slices, lpm->trie, lpm->defaultPayload,
                             lpm->defaultClsA, lpm->defaultClsB,
                             lpm->defaultDropMaskLo, lpm->defaultDropMaskHi);
    SB_ASSERT(status == SB_OK);
    status = g2p3_lpmDPMDeref(lpm->dpm, lpm->defaultPayload,
                                  lpm->defaultClsA, lpm->defaultClsB,
                                  lpm->defaultDropMaskLo,
                                  lpm->defaultDropMaskHi);
    SB_ASSERT(status == SB_OK);
    status = g2p3_lpmHPMDeref(lpm->hpm, lpm->defaultPayload);
    SB_ASSERT(status == SB_OK);
    status = g2p3_lpmDPMUninit(&lpm->dpm);
    SB_ASSERT(status == SB_OK);
    status = g2p3_lpmHPMUninit(&lpm->hpm);
    SB_ASSERT(status == SB_OK);
    status = g2p3_lpmHostMemFree(lpm->hostMem, lpm->lpmWorkMgr, sizeof(lpmWorkMgr_t));
    SB_ASSERT(status == SB_OK);
    status = g2p3_lpmHostMemFree(lpm->hostMem, lpm, lpm->hostAllocBytes);
    SB_ASSERT(status == SB_OK);
    *pLpm = NULL;
    return status;
}

/*
 * Descend down from a given node and remove all CLS_A/CLS_B payloads
 * encountered. This is used when CLS_A/CLS_B payloads are used to
 * store drop mask bits.
 */
static
sbStatus_t
lpmRemoveMasks (g2p3_lpmHandle_t lpm, lpmNode_p_t node,
                g2p3_lpmAddress_t addr, uint8_t length)
{
    sbStatus_t status;

    while (node) {
        /* skip dead nodes */
        if (lpmIsDead(node)) {
            node = lpmFirstLive (node);
            continue;
        }

        switch (node->tag.type) {
            case LPM_NODE_PAYLOAD:
                if ((node->tag.payloadKind ==
                    G2P3_LPM_PAYLOAD_DROPM_LO) ||
                    (node->tag.payloadKind ==
                    G2P3_LPM_PAYLOAD_DROPM_HI)) {
                    status = g2p3_lpmRemove (lpm, addr, length,
                                             node->tag.payloadKind, 1);
                    if (status != SB_OK)
                        return (status);
                    else {
                        node = node->data.payload.child;
                    }
                }
                else
                    node = node->data.payload.child;
                break;

            case LPM_NODE_TABLE:
                node = node->data.payload.child;
                break;
            case LPM_NODE_SPLIT:
                length += 1;
                status = lpmRemoveMasks (lpm, node->data.split.children[0],
                         addr | (0ULL << (lpm->addrBits - length)), length);
                if (status != SB_OK)
                    return (status);
                status = lpmRemoveMasks (lpm, node->data.split.children[1],
                         addr | (1ULL << (lpm->addrBits - length)),
                         length);
                return (status);

            case LPM_NODE_SKIP:
                addr |= ((g2p3_lpmAddress_t)  node->data.skip.address)
                         << (lpm->addrBits - length 
                             - (node->tag.skipLen + 1));
                length = length + node->tag.skipLen + 1;
                node = node->data.skip.child;
                break;
        }
    }

    return (SB_OK);
}


sbStatus_t
g2p3_lpmMaskClear(g2p3_lpmHandle_t lpm, g2p3_lpmAddress_t addr,
                       uint8_t length)
{
    sbStatus_t status;
    g2p3_lpmAddress_t lAddr, auxAddr;
    lpmNode_p_t node;
    g2p3_lpmPayloadHandle_t payload;
    g2p3_lpmClassifierId_t clsA, clsB;
    uint32_t dropMaskLo, dropMaskHi;

    do {
        /* blank out any part of address stretching beyond
         * the declared length
         */
        lAddr = addr & ~((1ULL << (lpm->addrBits - length)) - 1);
        auxAddr = lAddr;

        lAddr >>= lpm->addrBits - length;
        status = lpmFindPayload(lpm, &node, &payload, &clsA, &clsB,
                                &dropMaskLo, &dropMaskHi,
                                lAddr, length, G2P3_LPM_PAYLOAD_MAIN, NULL);
        if (status == SB_LPM_ADDRESS_NOT_FOUND) {
            status = lpmFindPayload(lpm, &node, &payload, &clsA, &clsB,
                                    &dropMaskLo, &dropMaskHi,
                                    lAddr, length,
                                    G2P3_LPM_PAYLOAD_DROPM_LO, NULL);

            if (status == SB_LPM_ADDRESS_NOT_FOUND) {
                status = lpmFindPayload(lpm, &node, &payload, &clsA, &clsB,
                                        &dropMaskLo, &dropMaskHi,
                                        lAddr, length,
                                        G2P3_LPM_PAYLOAD_DROPM_HI, NULL);
            }
        }

        if (status == SB_OK)
            break;

        /* keep descending down the trie if no node found */
        length++;

    } while (length < lpm->addrBits);

    if (status != SB_OK)
        return (status);

    /* drill all the way to the bottom and remove MASK LO/HI payloads */
    status = lpmRemoveMasks (lpm, node, auxAddr, length);
    return (status);
}

/* see prototype for contract */
static
sbStatus_t
lpmFindPayload(g2p3_lpmHandle_t lpm,
               lpmNode_p_t *pNode, g2p3_lpmPayloadHandle_t *pPayload,
               g2p3_lpmClassifierId_t *pClsA, g2p3_lpmClassifierId_t *pClsB,
               uint32_t *pDropMaskLo, uint32_t *pDropMaskHi,
               g2p3_lpmAddress_t addr, uint8_t length,
               g2p3_lpmPayloadKind_t payloadKind, lpmWork_p_t work)
{
    lpmNode_p_t node;
    g2p3_lpmPayloadHandle_t payload;
    g2p3_lpmClassifierId_t clsA;
    g2p3_lpmClassifierId_t clsB;
    uint32_t dropMaskLo, dropMaskHi;
    unsigned lcp;
    SB_ASSERT(lpm);

    node = lpm->trie;
    payload = lpm->defaultPayload;
    clsA = lpm->defaultClsA;
    clsB = lpm->defaultClsB;
    dropMaskLo = lpm->defaultDropMaskLo;
    dropMaskHi = lpm->defaultDropMaskHi;
    while (node != NULL && ((node->dpsBelow > 0) || lpmIsDead(node))) {
        if (lpmIsDead(node)) {
            node = lpmFirstLive(node);
        } else if (length == 0) {
            if (node->tag.type != LPM_NODE_PAYLOAD
                || node->tag.payloadKind > payloadKind) /* too far down */
                return SB_LPM_ADDRESS_NOT_FOUND;
            extractPayload(&payload, &clsA, &clsB, &dropMaskLo,
                           &dropMaskHi,
                           node->tag.payloadKind,
                           node->data.payload.desc);
            if (node->tag.payloadKind == payloadKind) { /* found */
                if (pNode) *pNode = node;
                if (pPayload) *pPayload = payload;
                if (pClsA) *pClsA = clsA;
                if (pClsB) *pClsB = clsB;
                if (pDropMaskLo) *pDropMaskLo = dropMaskLo;
                if (pDropMaskHi) *pDropMaskHi = dropMaskHi;
                return SB_OK;
            }
            node = node->data.payload.child;

        } else {
            switch (node->tag.type) {
            case LPM_NODE_PAYLOAD:
                extractPayload(&payload, &clsA, &clsB, &dropMaskLo,
                               &dropMaskHi, node->tag.payloadKind,
                               node->data.payload.desc);
                node = node->data.payload.child;
                break;
            case LPM_NODE_TABLE:
                /* table node can never have payload below it so ignore caching it*/
                TABLE_WORK_OFFSET(work, node->data.table.address, node->tag.singleton); 
                node = node->data.table.child;
                break;
            case LPM_NODE_SPLIT:
                if(work) {
                    SPLIT_WORK_OFFSET(work, ((addr >> (length - 1)) & 0x1));
                }
                
                node = node->data.split.children[(addr >> (length - 1)) & 0x1];
                --length;
                break;
            case LPM_NODE_SKIP:

                lcp = lcplen(addr, length, node->data.skip.address,
                             node->tag.skipLen + 1);
                if (lcp < node->tag.skipLen + 1) /* stem does not match */
                    return SB_LPM_ADDRESS_NOT_FOUND;

                SKIP_WORK_OFFSET(work, node->data.skip.address,
                                 node->tag.skipLen + 1,
                                 (work->slice - node->tag.skipLen - 1));  
                
                length -= node->tag.skipLen + 1;
                node = node->data.skip.child;
                break;
            default:
                SB_ASSERT(0);
                return SB_FAILED;
            }
        }
    }
    return SB_LPM_ADDRESS_NOT_FOUND;
}

/* see prototype for contract */
static
sbStatus_t
lpmRemoveRoute(g2p3_lpmHandle_t lpm, lpmNode_p_t trie,
               g2p3_lpmAddress_t addr, uint8_t length,
               g2p3_lpmPayloadKind_t payloadKind,
               /* what's been found so far */
               g2p3_lpmPayloadHandle_t payMain,
               g2p3_lpmClassifierId_t payClsA, g2p3_lpmClassifierId_t payClsB,
               uint32_t dropMaskLo, uint32_t dropMaskHi,
               lpmWork_p_t work, lpmNode_p_t parent)
{
    sbStatus_t status = SB_OK;
    uint32_t dpsPre;
    uint8_t lenDiff=0;
    g2p3_lpmAddress_t addr2 = 0;
    int bit = 0;
    g2p3_lpmSramAddr        lastTblAddr=0;
    sbBool_t                lastTblSingleton=FALSE;

    if (trie == NULL) return SB_LPM_ADDRESS_NOT_FOUND;
    if (lpmIsDead(trie))
        return lpmRemoveRoute(lpm, lpmFirstLive(trie), addr, length,
                              payloadKind, payMain, payClsA, payClsB,
                              dropMaskLo, dropMaskHi, work, trie);

    if (trie->dpsBelow == 0
        || (length == 0 && (trie->tag.type != LPM_NODE_PAYLOAD
                            || trie->tag.payloadKind > payloadKind)))
	return SB_LPM_ADDRESS_NOT_FOUND;

    dpsPre = lpmDpsBelow(trie);
    switch (trie->tag.type) {
    case LPM_NODE_PAYLOAD:
        if (length == 0
            && trie->tag.payloadKind == payloadKind) {
            /* ref payload triples created by removing this node */
            status = refDevPayloads(lpm->dpm, &trie->dpsBelow,
                                    trie->data.payload.child, payloadKind,
                                    payMain, payClsA, payClsB,
                                    dropMaskLo, dropMaskHi);
            if (status != SB_OK) return status;
            extractPayload(&payMain, &payClsA, &payClsB, &dropMaskLo,
                           &dropMaskHi, trie->tag.payloadKind,
                           trie->data.payload.desc);
            /* deref payloads created by this node */
            status = g2p3_lpmDPMDeref(lpm->dpm, payMain, payClsA, payClsB,
                                          dropMaskLo, dropMaskHi);
            SB_ASSERT(status == SB_OK);
            if (status != SB_OK) return status;
            --trie->dpsBelow;
            status = derefDevPayloads(lpm->dpm, &trie->dpsBelow,
                                      trie->data.payload.child, payloadKind,
                                      payMain, payClsA, payClsB,
                                      dropMaskLo, dropMaskHi);
            if (status != SB_OK) return status;
            trie->tag.state = LPM_NODE_STATE_DEAD;

            return SB_OK; /* skip dpsBelow update common to other cases */
        } else { /* length != 0 || trie->tag.payloadKind < payloadKind */
            extractPayload(&payMain, &payClsA, &payClsB, &dropMaskLo,
                           &dropMaskHi, trie->tag.payloadKind,
                           trie->data.payload.desc);

            /* for Efficiency its sufficient if we cache the main payload alone.
             * All other partners of payloads are Default payload all the time
             */
            LPM_WORK_CACHE_PAYLOAD(work, trie, payMain);
            
            status = lpmRemoveRoute(lpm, trie->data.payload.child,
                                    addr, length, payloadKind,
                                    payMain, payClsA, payClsB, dropMaskLo,
                                    dropMaskHi, work, trie);
        }
        break;
    case LPM_NODE_TABLE:
        SB_ASSERT(trie->data.table.child);
      
        if(work) {
          addr2 = work->dest;
          lastTblAddr = work->lastTblAddr;
          lastTblSingleton = work->lastTblSingleton;
          TABLE_WORK_OFFSET(work, trie->data.table.address, trie->tag.singleton);
        }

	status = lpmRemoveRoute(lpm, trie->data.table.child,
                                addr, length, payloadKind,
                                payMain, payClsA, payClsB, dropMaskLo,
                                dropMaskHi, work, trie);
	break;
    case LPM_NODE_SKIP:
        {
            uint8_t length2 = trie->tag.skipLen + 1;
            addr2 = trie->data.skip.address;
            SB_ASSERT(trie->data.skip.child);
            if (length2 > length
		|| (lcplen(addr, length, addr2, length2) != length2)) {
		return SB_LPM_ADDRESS_NOT_FOUND;
	    }

            if(work) {
              lenDiff = work->slice - length2;
              SKIP_WORK_OFFSET(work, addr2, 
                               length2, (work->slice - length2));
            }
                
	    status = lpmRemoveRoute(lpm, trie->data.skip.child,
                                    addr, length - length2, payloadKind,
                                    payMain, payClsA, payClsB, dropMaskLo,
                                    dropMaskHi, work, trie);
	}
	break;
    case LPM_NODE_SPLIT:
	{
	    bit = (addr >> (length - 1)) & 0x1;
            SB_ASSERT(trie->data.split.children[0]);
            SB_ASSERT(trie->data.split.children[1]);
           
            SPLIT_WORK_OFFSET(work, bit);

	    status = lpmRemoveRoute(lpm, trie->data.split.children[bit],
                                    addr, length - 1, payloadKind,
                                    payMain, payClsA, payClsB, dropMaskLo,
                                    dropMaskHi, work, trie);
	}
	break;
    default:
        SB_ASSERT(0);
        return SB_FAILED;
    } /* switch (trie->tag.type) */
    if (status == SB_OK) {
        uint32_t dpsPost = lpmDpsBelow(trie);
        uint8_t cachePoint = 0;

        SB_ASSERT(dpsPost <= dpsPre);
        trie->dpsBelow += dpsPost - dpsPre;
        SB_ASSERT(trie->tag.type == LPM_NODE_PAYLOAD
               || trie->dpsBelow == dpsPost);

        if((work) && (!work->nodeCached)) {
           cachePoint = 0;

           if(dpsPost <= 1) {
             cachePoint = 1;
           } else if (dpsPost > 1) {
               /* Verify if this Node can be a Cache Point */
               switch(trie->tag.type) {
                   case LPM_NODE_SKIP:
                     SB_ASSERT(trie->data.skip.child);
                     if((LPM_NODE_PAYLOAD == trie->data.skip.child->tag.type) &&
                        (LPM_NODE_STATE_DEAD == trie->data.skip.child->tag.state)) {
                       cachePoint = 1;
                     }
                     break;

                   case LPM_NODE_SPLIT:
                     SB_ASSERT(trie->data.split.children[0]);
                     SB_ASSERT(trie->data.split.children[1]);

                     if(((LPM_NODE_PAYLOAD == trie->data.split.children[0]->tag.type) &&
                         (LPM_NODE_STATE_DEAD == trie->data.split.children[0]->tag.state)) || 
                        ((LPM_NODE_PAYLOAD == trie->data.split.children[1]->tag.type) &&
                         (LPM_NODE_STATE_DEAD == trie->data.split.children[1]->tag.state))) 
                     {
                       cachePoint = 1;
                     } else if(lpmDpsBelow(trie->data.split.children[0]) == 0) {
                         cachePoint = 1;
                     } else if (lpmDpsBelow(trie->data.split.children[1]) == 0) {
                         cachePoint = 1;
                     }
                     break;

                   case LPM_NODE_TABLE:
                     SB_ASSERT(trie->data.table.child);
                     if((LPM_NODE_PAYLOAD == trie->data.table.child->tag.type) &&
                        (LPM_NODE_STATE_DEAD == trie->data.table.child->tag.state)) {
                       cachePoint = 1;
                     }
                     break;

                   default:
                     break;
               }
           }

           /* undo adjust of offsets by this node */
           switch(trie->tag.type) {
               case LPM_NODE_SKIP:
                 work->dest -= (addr2 << lenDiff);
                 work->slice +=  (trie->tag.skipLen + 1);
                 break;
                 
               case LPM_NODE_SPLIT:
                 {
                   if(bit) {
                     work->dest -= 1 << work->slice;
                   }
                   work->slice++;
                   break;
                 }
                 
               case LPM_NODE_TABLE:
                 work->slice = 0;
                 work->slices--;
                 work->dest = addr2;/*previous points address */
                 work->lastTblAddr = lastTblAddr;
                 work->lastTblSingleton = lastTblSingleton;
                 break;
                 
               default:
                 break;
           }             

           if(cachePoint && (!work->nodeCached)) {
             /* If this skip is child of a singleton table node
              * account the singleton offset, book mark its child
              * as the cachepoint */
             if(trie->tag.type == LPM_NODE_SKIP &&
                parent && parent->tag.type == LPM_NODE_TABLE && parent->tag.singleton) {
               work->slice = 0;
               work->dest = parent->data.table.address;
               SB_ASSERT(trie->data.skip.child->tag.type == LPM_NODE_PAYLOAD);
               LPM_WORK_CACHE_NODE(work, trie->data.skip.child);
             } else {
               LPM_WORK_CACHE_NODE(work, trie);
             }
           }
        }
    }
    return status;
}

#ifdef DUMP_TRIE
char names[][10]={
  "py", 
  "tb", 
  "sp",
  "sk",
  "0"};

char *print_type(lpmNode_p_t node)
{
 return names[node->tag.type];
}

char *print_state(lpmNode_p_t node)
{
    return (node->tag.state == LPM_NODE_STATE_DEAD)? "*":"-";
}

void dump_trie(lpmNode_p_t node, unsigned int width) 
{
   char buffer[256];
   int n=0;
   if (node) {
      switch (node->tag.type) 
      {
      case LPM_NODE_PAYLOAD:
        n=sal_sprintf(buffer,"\n%*s%s(%d)", width,print_state(node),print_type(node), node->dpsBelow);
        soc_cm_print("%s", buffer);
        dump_trie(node->data.payload.child, n);
        break;
      case LPM_NODE_TABLE:
        n=sal_sprintf(buffer,"\n%*s%s(%d)", width,print_state(node),print_type(node), node->dpsBelow);
        soc_cm_print("%s", buffer);
        dump_trie(node->data.table.child, n);
        break;
      case LPM_NODE_SPLIT:
        n=sal_sprintf(buffer,"\n%*s%s(%d)", width,print_state(node),print_type(node), node->dpsBelow);
        soc_cm_print("%s", buffer);
        dump_trie(node->data.split.children[1], n+1);
        dump_trie(node->data.split.children[0], n);
        break;
      case LPM_NODE_SKIP:
        n = sal_sprintf(buffer,"\n%*s%s:%d:%x(%d)", width,print_state(node),print_type(node), node->tag.skipLen+1, node->data.skip.address, node->dpsBelow);
        soc_cm_print("%s", buffer);
        dump_trie(node->data.skip.child, n);
      }
   }
}
#endif

/* see contract in g2p3_lpm.h */
sbStatus_t
g2p3_lpmAdd(g2p3_lpmHandle_t lpm, g2p3_lpmAddress_t addr, 
            uint8_t length, void *payload, 
            g2p3_lpmPayloadKind_t payloadKind,uint8_t l3cached)
{
    lpmWork_p_t work = NULL;
    sbStatus_t  status = SB_OK;
    lpmWorkMgr_p_t workmgr; 
    
    SB_ASSERT(lpm);
    workmgr = (lpmWorkMgr_p_t)lpm->lpmWorkMgr;
    SB_ASSERT(workmgr);

    g2p3_lpm_stats_update(lpm,G2P3_LPM_STATS_TYPE_ADD, 
                          G2P3_LPM_STATS_UPD_REQ);
    
    if (lpm->busy) return SB_BUSY_ERR_CODE;
#if LPM_LPM_DEBUG
    SB_LOG("g2p3_lpmAdd: addr 0x%010llx/%02d, payload = %p\n",
           addr, length, payload);
#endif
    
    addr >>= lpm->addrBits - length;

    if(l3cached == 0) { 
        work = g2p3_work_alloc(workmgr);
        /* Single Fast update optimization Sanity *
         * Fast update must work only with Caching turned off. Which
         * means any update must immediately follow by an commit. If not
         * the upper layer(l3) is violating caching norms. If work item is
         * not allocated it means there is an outstanding work item pending
         * to be commited. This check is to avoid races with fast update 
         * and for sanity */
        
        SB_ASSERT(work);         
    }   

    if(work) {
        work->dest   = lpm->topTableAddress;
        work->slice  = *lpm->slices;
        work->slices = lpm->slices + 1;
        work->pfxLen = length;
        work->addr   = addr;
    }

    workmgr->thisWorkType = LPM_WORK_ADD;

    status = lpmAddRoute(lpm, &lpm->trie, *lpm->slices, lpm->slices + 1,
                       addr, length, payloadKind, payload, lpm->defaultPayload,
                       lpm->defaultClsA, lpm->defaultClsB,
                       lpm->defaultDropMaskLo, lpm->defaultDropMaskHi,
                       work);

    if(SB_OK == status) {
        g2p3_lpm_stats_update(lpm, G2P3_LPM_STATS_TYPE_ADD,
                              G2P3_LPM_STATS_UPD_SUCCESS);
        
        if(work) {
            g2p3_work_enqueue(workmgr, work);
        }
    } else {
        if(SB_LPM_OUT_OF_HOST_MEMORY == status) {
            soc_cm_print("\n!!!! LPM out of Host Memory \n");
        } else if (SB_LPM_OUT_OF_DEVICE_MEMORY == status) {
            soc_cm_print("\n!!!! LPM out of Device Memory \n");
        }
        g2p3_work_collapse(workmgr, work);
    } 
#ifdef DUMP_TRIE
    dump_trie(lpm->trie, 0);
#endif
    return status;
}

/* see contract in g2p3_lpm.h */
sbStatus_t
g2p3_lpmRemove(g2p3_lpmHandle_t lpm, g2p3_lpmAddress_t addr,
               uint8_t length, g2p3_lpmPayloadKind_t payloadKind,
               uint8_t l3cached)
{
    lpmWork_p_t work = NULL;
    sbStatus_t  status = SB_OK;
    lpmWorkMgr_p_t workmgr;

    SB_ASSERT(lpm);
    workmgr = (lpmWorkMgr_p_t)lpm->lpmWorkMgr;
    SB_ASSERT(workmgr);

    g2p3_lpm_stats_update(lpm, G2P3_LPM_STATS_TYPE_DEL, 
                          G2P3_LPM_STATS_UPD_REQ);
    
    if (lpm->busy) return SB_BUSY_ERR_CODE;

    #if LPM_LPM_DEBUG
    SB_LOG("g2p3_lpmRemove: addr 0x%010llx/%02d\n",
           addr, length);
    #endif

    addr >>= lpm->addrBits - length;

    if(l3cached == 0) { 
        work = g2p3_work_alloc(workmgr);
        /* Single Fast update optimization Sanity *
         * Fast update must work only with Caching turned off. Which
         * means any update must immediately follow by an commit. If not
         * the upper layer(l3) is violating caching norms. This check is
         * to avoid races with fast update and for sanity */
        
        SB_ASSERT(work); 
    }

    if(work) {
        work->dest   = lpm->topTableAddress;
        work->slice  = *lpm->slices;
        work->slices = lpm->slices + 1;
        work->pfxLen = length;
        work->addr   = addr;
    }
    workmgr->thisWorkType = LPM_WORK_DEL;
      
    status = lpmRemoveRoute(lpm, lpm->trie,
                          addr, length, payloadKind,
                          lpm->defaultPayload, lpm->defaultClsA,
                          lpm->defaultClsB,
                          lpm->defaultDropMaskLo, lpm->defaultDropMaskHi,
                          work, lpm->trie);
    if(SB_OK == status) {
        g2p3_lpm_stats_update(lpm, G2P3_LPM_STATS_TYPE_DEL,
                              G2P3_LPM_STATS_UPD_SUCCESS);
        
        if(work) {
            g2p3_work_enqueue(workmgr, work);
        }
    } else {
        g2p3_work_collapse(workmgr, work);
    } 
#ifdef  DUMP_TRIE
    dump_trie(lpm->trie, 0);
#endif
    return status;
}

/* descend and make sure all new triples are (de)referenced */
static
sbStatus_t
devPayloadsRec(g2p3_lpmDPMHandle_t dpm, uint32_t *pCount, lpmNode_p_t node,
               sbBool_t ref, /* ref if true; deref otherwise */
               uint8_t payKind, /* where to stop */
               g2p3_lpmPayloadHandle_t payload,
               g2p3_lpmClassifierId_t clsA,
               g2p3_lpmClassifierId_t clsB,
               uint32_t dropMaskLo, uint32_t dropMaskHi)
{
    sbStatus_t status = SB_OK;

    SB_ASSERT(dpm);
    SB_ASSERT(ref == TRUE || ref == FALSE);
    SB_ASSERT(payKind < G2P3_LPM_PAYLOAD_NUM_KINDS);
    if (node == NULL) return SB_OK;

    switch (node->tag.type) {
    case LPM_NODE_PAYLOAD:
        if (node->tag.state == LPM_NODE_STATE_DEAD) {
            status = devPayloadsRec(dpm, pCount, node->data.payload.child,
                                    ref, payKind, payload, clsA, clsB,
                                    dropMaskLo, dropMaskHi);
            return status;
        } else if ((node->tag.payloadKind == payKind) &&
               ((payKind != G2P3_LPM_PAYLOAD_DROPM_LO) &&
                (payKind != G2P3_LPM_PAYLOAD_DROPM_HI))) {
            status = SB_OK;
        } else {
            extractPayload(&payload, &clsA, &clsB, &dropMaskLo,
                           &dropMaskHi, node->tag.payloadKind,
                           node->data.payload.desc);
            if (ref) {
                status = g2p3_lpmDPMRef(dpm, payload, clsA, clsB,
                                            dropMaskLo, dropMaskHi);
            } else {
                status = g2p3_lpmDPMDeref(dpm,  payload, clsA, clsB,
                                              dropMaskLo, dropMaskHi);
                SB_ASSERT(status == SB_OK);
            }
            if (status == SB_OK) {
                if (pCount) {
                    SB_ASSERT(ref || *pCount > 0);
                    *pCount += ref ? 1 : -1;
                }
                status = devPayloadsRec(dpm, pCount, node->data.payload.child,
                                        ref, payKind, payload, clsA, clsB,
                                        dropMaskLo, dropMaskHi);
                if (ref && status != SB_OK) {
                    SB_ASSERT(g2p3_lpmDPMDeref(dpm, payload, clsA, clsB,
                                                dropMaskLo, dropMaskHi)
                           == SB_OK);
                }
            }
        }
        break;
    case LPM_NODE_SKIP:
        status = devPayloadsRec(dpm, pCount, node->data.skip.child,
                                ref, payKind, payload, clsA, clsB,
                                dropMaskLo, dropMaskHi);
        break;
    case LPM_NODE_TABLE:
        status = devPayloadsRec(dpm, pCount, node->data.table.child, ref,
                                payKind, payload, clsA, clsB,
                                dropMaskLo, dropMaskHi);
        break;
    case LPM_NODE_SPLIT:
        status = devPayloadsRec(dpm, pCount, node->data.split.children[0],
                                ref, payKind, payload, clsA, clsB,
                                dropMaskLo, dropMaskHi);
        if (status == SB_OK)
            status = devPayloadsRec(dpm, pCount, node->data.split.children[1],
                                    ref, payKind, payload, clsA, clsB,
                                    dropMaskLo, dropMaskHi);
        break;
    default:
        SB_ASSERT(0);
        return SB_FAILED;
    }
    return status;
}

/* see prototype for contract */
static
sbStatus_t
refDevPayloads(g2p3_lpmDPMHandle_t dpm, uint32_t *pCount, lpmNode_p_t node,
               uint8_t payKind, void *payload, g2p3_lpmClassifierId_t clsA,
               g2p3_lpmClassifierId_t clsB, uint32_t dropMaskLo,
               uint32_t dropMaskHi)
{
    sbStatus_t status;

    SB_ASSERT(dpm);
    status = devPayloadsRec(dpm, pCount, node, TRUE, payKind,
                            payload, clsA, clsB, dropMaskLo, dropMaskHi);
    return status;
}

/* see prototype for contract */
static
sbStatus_t
derefDevPayloads(g2p3_lpmDPMHandle_t dpm, uint32_t *pCount,
                 lpmNode_p_t node,
                 uint8_t payKind, void *payload, g2p3_lpmClassifierId_t clsA,
                 g2p3_lpmClassifierId_t clsB, uint32_t dropMaskLo,
                 uint32_t dropMaskHi)
{
    sbStatus_t status;

    SB_ASSERT(dpm);
    status = devPayloadsRec(dpm, pCount, node, FALSE, payKind,
                            payload, clsA, clsB, dropMaskLo, dropMaskHi);
    return status;
}

/* see prototype for contract */
static
uint32_t
lpmDpsBelow(lpmNode_p_t node)
{
    lpmNode_p_t child0 = NULL, child1 = NULL;
    if (node != NULL) {
        switch (node->tag.type) {
        case LPM_NODE_PAYLOAD:
            child0 = node->data.payload.child;
            break;
        case LPM_NODE_TABLE:
            child0 = node->data.table.child;
            break;
        case LPM_NODE_SKIP:
            child0 = node->data.skip.child;
            break;
        case LPM_NODE_SPLIT:
            child0 = node->data.split.children[0];
            child1 = node->data.split.children[1];
            break;
        default:
            SB_ASSERT(0);
        }
    }
    child0 = lpmFirstLive(child0);
    child1 = lpmFirstLive(child1);
    return ((child0 ? child0->dpsBelow : 0)
            + (child1 ? child1->dpsBelow : 0));
}

/* see prototype for contract */
static
sbStatus_t
lpmAddPayload(g2p3_lpmHandle_t lpm, lpmNode_p_t *pTrie,
              g2p3_lpmPayloadKind_t payloadKind, void *newPayload,
              /* what's been found so far */
              g2p3_lpmPayloadHandle_t payMain,
              g2p3_lpmClassifierId_t payClsA,
              g2p3_lpmClassifierId_t payClsB,
              uint32_t dropMaskLo, uint32_t dropMaskHi,
              lpmWork_p_t work)
{
    sbStatus_t status;
    void *vp;
    lpmNode_p_t newNode;
    g2p3_lpmPayloadHandle_t payloadHandle;
    g2p3_lpmPayloadHandle_t rmPayMain;
    g2p3_lpmClassifierId_t rmPayClsA, rmPayClsB;
    uint32_t rmDropMaskLo, rmDropMaskHi;

    SB_ASSERT(lpm);
    SB_ASSERT(pTrie);

    if (*pTrie == NULL
        || (*pTrie)->tag.type != LPM_NODE_PAYLOAD
        || (*pTrie)->tag.payloadKind > payloadKind
        || ((*pTrie)->tag.payloadKind == payloadKind
            && (*pTrie)->tag.state == LPM_NODE_STATE_DEAD)) {
        /* add payload node */
        status = g2p3_lpmHostMemAlloc(lpm->hostMem,
                                          &vp, sizeof(lpmNode_t), "lpm node");
        if (status != SB_OK) return status;
        if (payloadKind == G2P3_LPM_PAYLOAD_MAIN) {
            status = g2p3_lpmHPMRef(lpm->hpm, &payloadHandle, newPayload);
            if (status != SB_OK) {
                SB_ASSERT(g2p3_lpmHostMemFree(lpm->hostMem, vp,
                                               sizeof(lpmNode_t))
                       == SB_OK);
                return status;
            }
            newPayload = (void *) payloadHandle;
        }
        newNode = (lpmNode_p_t) vp;
        newNode->tag.type = LPM_NODE_PAYLOAD;
        newNode->tag.payloadKind = payloadKind;
        newNode->tag.state = LPM_NODE_STATE_NEW;
        newNode->data.payload.desc = newPayload;
        newNode->data.payload.child = *pTrie;
        newNode->dpsBelow = lpmDpsBelow(newNode);
        rmPayMain = payMain;
        rmPayClsA = payClsA;
        rmPayClsB = payClsB;
        rmDropMaskLo = dropMaskLo;
        rmDropMaskHi = dropMaskHi;

        extractPayload(&payMain, &payClsA, &payClsB, &dropMaskLo,
                       &dropMaskHi, payloadKind,
                       newPayload);
        status = g2p3_lpmDPMRef(lpm->dpm, payMain, payClsA, payClsB,
                     dropMaskLo, dropMaskHi);
        if (status != SB_OK) {
            if (payloadKind == G2P3_LPM_PAYLOAD_MAIN)
                SB_ASSERT(g2p3_lpmHPMDeref(lpm->hpm, payloadHandle)
                       == SB_OK);
            SB_ASSERT(g2p3_lpmHostMemFree(lpm->hostMem, vp,
                                           sizeof(lpmNode_t)) == SB_OK);
            return status;
        }
        ++newNode->dpsBelow;
        status = refDevPayloads(lpm->dpm, &newNode->dpsBelow, *pTrie,
                                payloadKind, payMain, payClsA, payClsB,
                                dropMaskLo, dropMaskHi);
        if (status != SB_OK) {
            if (payloadKind == G2P3_LPM_PAYLOAD_MAIN)
                SB_ASSERT(g2p3_lpmHPMDeref(lpm->hpm, payloadHandle)
                       == SB_OK);
            SB_ASSERT(g2p3_lpmHostMemFree(lpm->hostMem, vp,
                                           sizeof(lpmNode_t)) == SB_OK);
            return status;
        }
        status = derefDevPayloads(lpm->dpm, &newNode->dpsBelow, *pTrie,
                                  payloadKind, rmPayMain, rmPayClsA, rmPayClsB,
                                  rmDropMaskLo, rmDropMaskHi);
        if (status != SB_OK) {
            if (payloadKind == G2P3_LPM_PAYLOAD_MAIN)
                SB_ASSERT(g2p3_lpmHPMDeref(lpm->hpm, payloadHandle)
                       == SB_OK);
            SB_ASSERT(g2p3_lpmDPMDeref(lpm->dpm, payMain, payClsA, payClsB,
                                        dropMaskLo, dropMaskHi) == SB_OK);
            --newNode->dpsBelow;
            derefDevPayloads(lpm->dpm, &newNode->dpsBelow, *pTrie, payloadKind,
                             payMain, payClsA, payClsB, dropMaskLo,
                             dropMaskHi);
            SB_ASSERT(g2p3_lpmHostMemFree(lpm->hostMem, vp,
                                           sizeof(lpmNode_t)) == SB_OK);
            return status;
        }
        *pTrie = newNode;
        LPM_WORK_CACHE_PAYLOAD(work, *pTrie, payMain);
        LPM_WORK_CACHE_NODE(work, *pTrie);
        return SB_OK;
    } else if ((*pTrie)->tag.payloadKind < payloadKind) {
        /* preceding payload kind: insert below */
        uint32_t dpsPre = lpmDpsBelow(*pTrie);
        /* ignore this node if dead - just follow the child */
        if ((*pTrie)->tag.state != LPM_NODE_STATE_DEAD)
            extractPayload(&payMain, &payClsA, &payClsB, &dropMaskLo,
                           &dropMaskHi,
                           (*pTrie)->tag.payloadKind,
                           (*pTrie)->data.payload.desc);
        status = lpmAddPayload(lpm, &((*pTrie)->data.payload.child),
                               payloadKind, newPayload,
                               payMain, payClsA, payClsB, dropMaskLo,
                               dropMaskHi, work);
        if (status == SB_OK) {
            uint32_t dpsPost = lpmDpsBelow(*pTrie);
            SB_ASSERT(dpsPost >= dpsPre);
            (*pTrie)->dpsBelow += dpsPost - dpsPre;
        }
        return status;
    } else {
        /* duplicate payload */
        return SB_LPM_DUPLICATE_ADDRESS;
    }
}


/* see prototype for contract */
static
sbStatus_t
lpmAddTable(g2p3_lpmHandle_t lpm, lpmNode_p_t *pTrie,
            uint8_t thisSlice, uint8_t *nextSlices,	/* zero-terminated */
            g2p3_lpmAddress_t addr, uint8_t length,
            g2p3_lpmPayloadKind_t payloadKind, void *newPayload,
            sbBool_t singleton,
            /* what's been found so far */
            g2p3_lpmPayloadHandle_t payMain,
            g2p3_lpmClassifierId_t payClsA, g2p3_lpmClassifierId_t payClsB,
            uint32_t dropMaskLo, uint32_t dropMaskHi,
            lpmWork_p_t work)
{
    sbStatus_t status;
    lpmNode_p_t newNode, liveChild;
    void *vp;
    size_t tableSize;

    SB_ASSERT(lpm);
    SB_ASSERT(thisSlice == 0);
    SB_ASSERT(*nextSlices != 0);

    /* no singleton tables if slice is too big */
    singleton &= *nextSlices <= MAX_SKIP_LEN;
    status = g2p3_lpmHostMemAlloc(lpm->hostMem,
                                      &vp, sizeof(lpmNode_t), "lpm node");
    if (status != SB_OK) return status;
    newNode = (lpmNode_p_t) vp;
    tableSize = singleton ? 2 : 1 << *nextSlices;
    if ((int)(nextSlices - lpm->slices) % 2) {
      status = g2p3_lpmDeviceMemAlloc(lpm->deviceMemB,
                                        &newNode->data.table.address,
                                        tableSize);
    } else {
      status = g2p3_lpmDeviceMemAlloc(lpm->deviceMemA,
                                        &newNode->data.table.address,
                                        tableSize);
    }
    if (status != SB_OK) {
        SB_ASSERT(g2p3_lpmHostMemFree(lpm->hostMem, newNode,
                                       sizeof(lpmNode_t)) == SB_OK);
        return status;
    }
    newNode->tag.type = LPM_NODE_TABLE;
    newNode->tag.singleton = singleton;
    newNode->tag.state = LPM_NODE_STATE_NEW;
    newNode->data.table.child = *pTrie;

    /* If previous table was singleton, update the 
     * destination address on the work item */
    G2P3_LPM_SNG_ADJ_DEST_OFFSET(work);

    /* cache this table node if applicable */
    LPM_WORK_CACHE_NODE(work, newNode);
    
    status = lpmAddRoute(lpm, &(newNode->data.table.child),
                         *nextSlices, nextSlices + 1,
                         addr, length, payloadKind, newPayload,
                         payMain, payClsA, payClsB, dropMaskLo, dropMaskHi, work);
    if (status == SB_OK) {
        newNode->dpsBelow = lpmDpsBelow(newNode);
        SB_ASSERT(newNode->dpsBelow > 0);
        if (singleton) {
            liveChild = lpmFirstLive(newNode->data.table.child);
            SB_ASSERT(liveChild->tag.type == LPM_NODE_SKIP);
            SB_ASSERT(liveChild->tag.skipLen + 1 <= *nextSlices);
        }
        *pTrie = newNode;
    } else {
        if ((int)(nextSlices - lpm->slices) % 2) {
          SB_ASSERT(g2p3_lpmDeviceMemFree(lpm->deviceMemB,
                                         newNode->data.table.address,
                                         tableSize) == SB_OK);
        } else {
          SB_ASSERT(g2p3_lpmDeviceMemFree(lpm->deviceMemA,
                                         newNode->data.table.address,
                                         tableSize) == SB_OK);
        }
        SB_ASSERT(g2p3_lpmHostMemFree(lpm->hostMem, newNode,
                                       sizeof(lpmNode_t)) == SB_OK);
    }
    return status;
}

/*
 * Descend down the trie along the specified address for as long as
 * possible, updating payload at every step. Return paylaod collected
 * this way plus ptr to the last element of the path.
 */
void
lpmDropDown(g2p3_lpmHandle_t lpm, g2p3_lpmAddress_t addr,
            uint8_t length,
            g2p3_lpmPayloadHandle_t *payMain,
            g2p3_lpmClassifierId_t *pClsA,
            g2p3_lpmClassifierId_t *pClsB,
            uint32_t *pDropMaskLo, uint32_t *pDropMaskHi,
            g2p3_lpmClassifierId_t *pClsC)

{
    lpmNode_p_t node, prevNode;
    int bit, length2, lcp;
    g2p3_lpmAddress_t addr2;
    sbBool_t match = TRUE;

    node = prevNode = lpm->trie;  /* start from the very top of the trie */

    *payMain = (g2p3_lpmPayloadHandle_t)lpm->defaultPayload;
    *pClsA = lpm->defaultClsA;
    *pClsB = lpm->defaultClsB;
    *pClsC = 0;

    /* drill all the way to the bottom */
    while (node && match) {

        /* skip dead nodes */
        if (lpmIsDead(node)) {
            node = lpmFirstLive(node);
            continue;
        }

        prevNode = node;
        switch (node->tag.type) {
            case LPM_NODE_PAYLOAD:
                if (node->tag.payloadKind == G2P3_LPM_PAYLOAD_CLS_C) {
                    /* cannot have any earlier cls_C nodes */
                    SB_ASSERT (*pClsC == 0);

                    /* this has to be the last node present */
                    /* there might actually be a CLS_C dead node there */
                    SB_ASSERT (length == 0);

                    /* fish out cookie value */
                    *pClsC =
                      (g2p3_lpmClassifierId_t)(uint32_t)node->data.payload.desc;
                }
                else {
                    extractPayload(payMain, pClsA, pClsB, pDropMaskLo,
                                   pDropMaskHi, node->tag.payloadKind,
                                   node->data.payload.desc);
                }
                node = node->data.payload.child;
                break;
            case LPM_NODE_TABLE:
                node = node->data.payload.child;
                break;
            case LPM_NODE_SPLIT:
                bit = (addr >> (length - 1)) & 0x1;
                node = node->data.split.children[bit];
                length--;
                break;
            case LPM_NODE_SKIP:
                length2 = node->tag.skipLen + 1;
                addr2 = node->data.skip.address;
                lcp = lcplen(addr, length, addr2, length2);
                if (lcp == length2) {
                    length -= length2;
                    node = node->data.skip.child;
                }
                else {
                    match = FALSE;
                }
                break;
        }

    }
    return;
}

sbStatus_t
g2p3_lpmAddC(g2p3_lpmHandle_t lpm, g2p3_lpmAddress_t addr,
                 uint8_t length, void **payMain, g2p3_lpmClassifierId_t *pClsA,
                 g2p3_lpmClassifierId_t *pClsB, uint32_t *pDropMaskLo,
                 uint32_t *pDropMaskHi, g2p3_lpmClassifierId_t *pClsC)
{
    sbStatus_t status;
    g2p3_lpmPayloadHandle_t pM;
    g2p3_lpmClassifierId_t cA, cB, cC;
    uint32_t dropMaskLo, dropMaskHi;
    g2p3_lpmSramAddr payloadDevAddr;
    uint16_t payloadNodeExtra;

    lpmNode_p_t node;
    g2p3_lpmPayloadHandle_t oldPayload;
    g2p3_lpmClassifierId_t oldClsA, oldClsB;
    uint32_t oldDropMaskLo, oldDropMaskHi;



    SB_ASSERT(lpm);
    if (lpm->busy) return SB_BUSY_ERR_CODE;

    SB_ASSERT (length == 64);

    
    status = lpmFindPayload(lpm, &node, &oldPayload, &oldClsA, &oldClsB,
                            &oldDropMaskLo, &oldDropMaskHi, addr,
                            64, G2P3_LPM_PAYLOAD_MAIN, NULL);
    if (SB_OK != status)
        return (SB_IPV6_128_NEEDS_64_PREFIX);

    dropMaskLo = dropMaskHi = 0;
    cA = cB = cC = 0;
    lpmDropDown(lpm, addr, length, &pM, &cA, &cB, &dropMaskLo,
                &dropMaskHi, &cC);

    if (cC == 0) {
        /* no clsC node found - add it */
        status = g2p3_lpmDPMInfo(lpm->dpm, &payloadDevAddr,
                                     &payloadNodeExtra, pM, cA, cB,
                                     dropMaskLo, dropMaskHi);
        if (SB_OK != status)
            return (status);

        /* The offset from the start of the payload area becomes the cookie */
        *pClsC = payloadDevAddr - lpm->deviceMemA->startAddr;

        /* add CLS_C payload */
        status = g2p3_lpmAdd (lpm, addr, length, (void *)(uint32_t)*pClsC,
                                  G2P3_LPM_PAYLOAD_CLS_C, 1);

        if (SB_OK != status)
            return (status);

        /* verify */
        lpmDropDown(lpm, addr, length, &pM, &cA, &cB, &dropMaskLo,
                    &dropMaskHi, &cC);
        SB_ASSERT (*pClsC == cC);
    }
    else {
        *pClsC = cC;
    }

    *payMain = g2p3_lpmHPMData(pM);
    *pClsA = cA;
    *pClsB = cB;
    *pDropMaskLo = dropMaskLo;
    *pDropMaskHi = dropMaskHi;

    return SB_OK;
}

/* see prototype for contract */
static
sbStatus_t
lpmAddRoute(g2p3_lpmHandle_t lpm, lpmNode_p_t *pTrie,
            uint8_t thisSlice, uint8_t *nextSlices,	/* zero-terminated */
            g2p3_lpmAddress_t addr, uint8_t length,
            g2p3_lpmPayloadKind_t payloadKind, void *newPayload,
            /* what's been found so far */
            g2p3_lpmPayloadHandle_t payMain,
            g2p3_lpmClassifierId_t payClsA, g2p3_lpmClassifierId_t payClsB,
            uint32_t dropMaskLo, uint32_t dropMaskHi,
            lpmWork_p_t work)
{
    sbStatus_t status = SB_OK;
    void *vp;
    int bit, length2, lcp;
    g2p3_lpmAddress_t addr2;
    lpmNode_p_t newNode;
    uint32_t dpsPre, dpsPost;

    SB_ASSERT(lpm);
    SB_ASSERT(pTrie);
    
    if (length == 0) {
        /* add payload node */
        status = lpmAddPayload(lpm, pTrie, payloadKind, newPayload,
                               payMain, payClsA, payClsB, dropMaskLo,
                               dropMaskHi, work);
        return status;
    }

    /* reduce to adding payload node */
    SB_ASSERT(thisSlice > 0
           || *pTrie == NULL
           || (*pTrie)->tag.type == LPM_NODE_PAYLOAD
           || (*pTrie)->tag.type == LPM_NODE_TABLE);

    /* add table node if necessary */
    if (thisSlice == 0
        && (*pTrie == NULL
            || ((*pTrie)->tag.type == LPM_NODE_TABLE
                && (*pTrie)->tag.state == LPM_NODE_STATE_DEAD))) {

        return lpmAddTable(lpm, pTrie, thisSlice, nextSlices, addr, length,
                           payloadKind, newPayload, lpm->optSingleton,
                           payMain, payClsA, payClsB, dropMaskLo,
                           dropMaskHi, work);
    }
    
    bit = (addr >> (length - 1)) & 0x1;

    /* add at end of trie */
    if (*pTrie == NULL) {
        int skip = length < thisSlice ? length : thisSlice;
        SB_ASSERT(skip > 0);
        status = g2p3_lpmHostMemAlloc(lpm->hostMem,
                                          &vp, sizeof(lpmNode_t), "lpm node");
        if (status != SB_OK) return status;
        newNode = (lpmNode_p_t) vp;
        newNode->tag.type = LPM_NODE_SKIP;
        newNode->tag.skipLen = skip - 1;
        newNode->tag.state = LPM_NODE_STATE_LIVE;
        newNode->data.skip.address =
            (addr >> (length - skip)) & ((1 << skip) - 1);
        newNode->data.skip.child = NULL;
        /* 	    checkSkipNode(newNode); */
        
        /* If the next node is a payload node(final) node added,
         * or it could cross a slice creating new table node
         * cache this node */
        if((length - skip == 0) || (length > thisSlice)) {
          
            /* cache this skip node if applicable */
            LPM_WORK_CACHE_NODE(work, newNode); 
            
        } else {
            if(work) {
                SKIP_WORK_OFFSET(work, newNode->data.skip.address,
                                 skip, (work->slice - skip));
            }            
        }

        status = lpmAddRoute(lpm, &(newNode->data.skip.child),
                             thisSlice - skip, nextSlices,
                             addr, length - skip, payloadKind, newPayload,
                             payMain, payClsA, payClsB, dropMaskLo,
                             dropMaskHi, work);
        /*
         * SB_LOG ("DBG: creating LPM_NODE_SKIP 0x%x/%d\n",
         * newNode->data.skip.address, newNode->tag.skipLen + 1);
         */
        if (status == SB_OK) {
            newNode->dpsBelow = lpmDpsBelow(newNode);
            SB_ASSERT(newNode->dpsBelow > 0);
            *pTrie = newNode;
        } else {
            SB_ASSERT(g2p3_lpmHostMemFree(lpm->hostMem, newNode,
                                           sizeof(lpmNode_t)) == SB_OK);
        }
        return status;
    }

    /* skip dead nodes */
    if (lpmIsDead(*pTrie)) {
        switch((*pTrie)->tag.type) {
        case LPM_NODE_PAYLOAD: pTrie = &((*pTrie)->data.payload.child); break;
        case LPM_NODE_TABLE:   pTrie = &((*pTrie)->data.table.child); break;
        default:
            SB_ASSERT(0); /* only payload and table nodes can be dead */
            return SB_FAILED;
        }
        return lpmAddRoute(lpm, pTrie, thisSlice, nextSlices,
                           addr, length, payloadKind, newPayload,
                           payMain, payClsA, payClsB, dropMaskLo,
                           dropMaskHi, work);
    }

    /* decompress table if necessary */
    /*!! Note Commit of Decompressed Tables are handled on the next commit 
     * With work item, this is handled on the present commit */
    if (((*pTrie)->tag.type == LPM_NODE_TABLE)) {
      if ((*pTrie)->tag.singleton) {
        SB_ASSERT((*pTrie)->data.table.child);
        SB_ASSERT((*pTrie)->data.table.child->tag.type == LPM_NODE_SKIP);
        length2 = (*pTrie)->data.table.child->tag.skipLen + 1;
        addr2 = (*pTrie)->data.table.child->data.skip.address;
        lcp = lcplen(addr, length, addr2, length2);
        if ((lcp < length2)
            || ((length2 < *nextSlices) && (length > length2))) {
            lpmNodeState_t state = (*pTrie)->tag.state;
            (*pTrie)->tag.state = LPM_NODE_STATE_DEAD;

            /* If work item node cached is not set, it means this singleton
             * table is the first node decompressed and will be book marked. 
             * If previous table was singleton, reset destination address on
             * the work item */
             G2P3_LPM_SNG_ADJ_DEST_OFFSET(work);

            /*soc_cm_print("\n ++++ Singleton Table Decompression ++++");*/
            status = lpmAddTable(lpm, pTrie, thisSlice, nextSlices,
                                 addr, length, payloadKind, newPayload,
                                 FALSE, payMain, payClsA, payClsB,
                                 dropMaskLo, dropMaskHi, work);
            if (status != SB_OK) {
                (*pTrie)->tag.state = state;
            } else {
                /*soc_cm_print("\n #### Caching Prune Work Item ###");*/
                /* Cache Prune Work. With work item model for single update
                 * caching the prune point can decrease lpmprune dead process
                 * significantly */
                g2p3_lpm_prune_work_enqueue(lpm->lpmWorkMgr, work, *pTrie); 
            }
            return status;
        }
      } 
    }

    /* otherwise, descend down the trie */
    dpsPre = lpmDpsBelow(*pTrie);
    SB_ASSERT((*pTrie)->tag.type == LPM_NODE_PAYLOAD
           || (*pTrie)->dpsBelow == dpsPre);
    switch ((*pTrie)->tag.type) {
    case LPM_NODE_SPLIT:
        SPLIT_WORK_OFFSET(work, bit);
        
        /*
         * SB_LOG ("DBG: *pTrie is LPM_NODE_SPLIT: following
         * %d\n", bit);
         */
        status = lpmAddRoute(lpm,
                             &((*pTrie)->data.split.children[bit]),
                             thisSlice - 1, nextSlices,
                             addr, length - 1, payloadKind,
                             newPayload, payMain, payClsA, payClsB,
                             dropMaskLo, dropMaskHi, work);
        break;
    case LPM_NODE_PAYLOAD:
        /*
         * SB_LOG ("DBG: *pTrie is LPM_NODE_PAYLOAD\n");
         */
        extractPayload(&payMain, &payClsA, &payClsB, &dropMaskLo,
                       &dropMaskHi, (*pTrie)->tag.payloadKind,
                       (*pTrie)->data.payload.desc);

        /* for Efficiency its sufficient if we cache the main payload alone.
         * All other partners of payloads are Default payload all the time
         */
        LPM_WORK_CACHE_PAYLOAD(work, *pTrie, payMain);
         
        status = lpmAddRoute(lpm, &((*pTrie)->data.payload.child),
                             thisSlice, nextSlices,
                             addr, length, payloadKind, newPayload,
                             payMain, payClsA, payClsB, dropMaskLo,
                             dropMaskHi, work);
        break;
    case LPM_NODE_TABLE:
        /*
         * SB_LOG ("DBG: *pTrie is LPM_NODE_TABLE\n");
         */
        SB_ASSERT(thisSlice == 0);
        SB_ASSERT(*nextSlices != 0);
        
        TABLE_WORK_OFFSET(work, (*pTrie)->data.table.address, (*pTrie)->tag.singleton);
        status = lpmAddRoute(lpm, &((*pTrie)->data.table.child),
                             *nextSlices, nextSlices + 1,
                             addr, length, payloadKind, newPayload,
                             payMain, payClsA, payClsB, dropMaskLo,
                             dropMaskHi, work);
        break;
    case LPM_NODE_SKIP:
        /* N.B.: all required table nodes are already there */
        /*
         * SB_LOG ("DBG: *pTrie is LPM_NODE_SKIP 0x%x/%d\n",
         * (*pTrie)->data.skip.address, (*pTrie)->tag.skipLen +
         * 1);
         */
        {
            length2 = (*pTrie)->tag.skipLen + 1;
            addr2 = (*pTrie)->data.skip.address;
            lcp = lcplen(addr, length, addr2, length2);
            
            if (lcp == length2) {
                
                if(work) {
                    SKIP_WORK_OFFSET(work, addr2, 
                                     length2, (work->slice - length2));
                }                
              
                status = lpmAddRoute(lpm, &((*pTrie)->data.skip.child),
                                     thisSlice - lcp, nextSlices,
                                     addr, length - lcp,
                                     payloadKind, newPayload,
                                     payMain, payClsA, payClsB, dropMaskLo,
                                     dropMaskHi, work);
            } else if (lcp != 0) {	/* && lcp < length2 */
                status = g2p3_lpmHostMemAlloc(lpm->hostMem, &vp,
                                                  sizeof(lpmNode_t), "lpm node");
                if (status != SB_OK) return status;
                newNode = (lpmNode_p_t) vp;
                newNode->tag.type = LPM_NODE_SKIP;
                newNode->tag.skipLen = lcp - 1;
                newNode->tag.state = LPM_NODE_STATE_LIVE;
                newNode->data.skip.address =
                    (addr >> (length - lcp)) & ((1 << lcp) - 1);
                newNode->data.skip.child = *pTrie;
                newNode->dpsBelow = (*pTrie)->dpsBelow;
                /* 			checkSkipNode(newNode); */
                (*pTrie)->tag.skipLen -= lcp;
                (*pTrie)->data.skip.address &=
                    (1 << (length2 - lcp)) - 1;
                /* 			checkSkipNode(*pTrie); */

                /* If the next node is a payload node(final) node added,
                 * cache this node */
                if((length - lcp == 0)||  (length > thisSlice)) {
                  
                    /* cache this skip node if applicable */
                    LPM_WORK_CACHE_NODE(work, newNode); 
                    
                } else {
                    
                    if(work) {
                        SKIP_WORK_OFFSET(work, newNode->data.skip.address, 
                                         lcp, (work->slice - lcp));                    
                    }                    
                }
                
                status =
                    lpmAddRoute(lpm, &(newNode->data.skip.child),
                                thisSlice - lcp, nextSlices,
                                addr, length - lcp,
                                payloadKind, newPayload,
                                payMain, payClsA, payClsB, dropMaskLo,
                                dropMaskHi, work);
                /*
                 * SB_LOG ("DBG: newNode is LPM_NODE_SKIP
                 * 0x%x/%d\n", newNode->data.skip.address,
                 * newNode->tag.skipLen + 1);
                 */
                if (status == SB_OK) {
                    *pTrie = newNode;
                } else {
                    (*pTrie)->tag.skipLen += lcp;
                    (*pTrie)->data.skip.address = addr2;
                    SB_ASSERT(g2p3_lpmHostMemFree(lpm->hostMem, newNode,
                                                   sizeof(lpmNode_t))
                           == SB_OK);
                           /* 			    checkSkipNode(*pTrie); */
                }
            } else {	/* lcp == 0 */
                if (length2 == 1) {
                    newNode = (*pTrie)->data.skip.child;
                } else {
                    status = g2p3_lpmHostMemAlloc(lpm->hostMem,
                                                      &vp,
                                                      sizeof(lpmNode_t), "lpm node");
                    if (status != SB_OK) return status;
                    newNode = (lpmNode_p_t) vp;
                    newNode->tag.type = LPM_NODE_SKIP;
                    newNode->tag.skipLen = length2 - 2;	/* --len */
                    newNode->tag.state = LPM_NODE_STATE_LIVE;
                    newNode->data.skip.address =
                        addr2 & ((1 << (length2 - 1)) - 1);
                    newNode->data.skip.child =
                        (*pTrie)->data.skip.child;
                    newNode->dpsBelow = (*pTrie)->dpsBelow;
                    /* 			    checkSkipNode(newNode); */

                    /*
                     * SB_LOG ("DBG: lcp==0; newNode is
                     * LPM_NODE_SKIP 0x%x/%d\n",
                     * newNode->data.skip.address,
                     * newNode->tag.skipLen + 1);
                     */
                }
                (*pTrie)->tag.type = LPM_NODE_SPLIT;
                (*pTrie)->tag.state = LPM_NODE_STATE_LIVE;
                (*pTrie)->data.split.children[bit] = NULL;
                (*pTrie)->data.split.children[!bit] = newNode;
                
                /* If the next node is a payload node(final) node added,
                 * or the prefix can cross the slice causing new table node  
                 * cache this node */
                if((length - 1 == 0) ||  (length > thisSlice)) {
                  /* if the newly added split nodes child is payload
                   * we got to cache ths split node
                   */
                  LPM_WORK_CACHE_NODE(work, *pTrie); 
                    
                } else {
                    SPLIT_WORK_OFFSET(work, bit);
                }
                
                status = lpmAddRoute(lpm,
                                &((*pTrie)->data.split.children[bit]),
                                thisSlice - 1, nextSlices, addr,
                                length - 1, payloadKind, newPayload,
                                payMain, payClsA, payClsB, dropMaskLo,
                                dropMaskHi, work);
                if (status != SB_OK) {
                    SB_ASSERT((*pTrie)->tag.state == LPM_NODE_STATE_LIVE);
                    (*pTrie)->tag.type = LPM_NODE_SKIP;
                    (*pTrie)->data.skip.address = addr2;
                    if (length2 == 1) {
                        (*pTrie)->data.skip.child = newNode;
                    } else {
                        (*pTrie)->data.skip.child =
                            newNode->data.skip.child;
                        SB_ASSERT(g2p3_lpmHostMemFree(lpm->hostMem, newNode,
                                                       sizeof(lpmNode_t))
                               == SB_OK);
                    }
                    /* 			    checkSkipNode(*pTrie); */
                    /*
                     * SB_LOG ("DBG: restoring *pTrie
                     * LPM_NODE_SKIP 0x%x/%d\n",
                     * (*pTrie)->data.skip.address,
                     * (*pTrie)->tag.skipLen + 1);
                     */
                } 
                
            } /* if on lcp length */
        }
        break;
    default:
        SB_ASSERT(0);
        return SB_FAILED;
    } /* switch ((*pTrie)->tag.type) */
    /* update dpsBelow */
    if (status == SB_OK) {
        dpsPost = lpmDpsBelow(*pTrie);
        SB_ASSERT(dpsPost >= dpsPre);
        (*pTrie)->dpsBelow += dpsPost - dpsPre;
        SB_ASSERT((*pTrie)->tag.type == LPM_NODE_PAYLOAD
               || lpmIsDead(*pTrie)
               || (*pTrie)->dpsBelow == dpsPost);
        SB_ASSERT((*pTrie)->dpsBelow != 0);
    } /* if (status == SB_OK) */
    return status;
}

/* see contract in g2p3_lpm.h */
static
sbStatus_t
g2p3_lpmUpdate_internal(g2p3_lpmHandle_t lpm,
                   g2p3_lpmAddress_t addr, uint8_t length,
                   void *newPayload, g2p3_lpmPayloadKind_t payloadKind,
                   lpmWork_p_t work)
{
    sbStatus_t status;
    lpmNode_p_t node;
    g2p3_lpmPayloadHandle_t payload, oldPayload;
    g2p3_lpmClassifierId_t clsA, clsB, oldClsA, oldClsB;
    uint32_t dropMaskLo, dropMaskHi, oldDropMaskLo, oldDropMaskHi;
    g2p3_lpmPayloadHandle_t payloadHandle;

    SB_ASSERT(lpm);
    if (lpm->busy) return SB_BUSY_ERR_CODE;
#if LPM_LPM_DEBUG
    SB_LOG("g2p3_lpmUpd: addr 0x%010llx/%02d, payload = %p\n",
           addr, length, newPayload);
#endif
    addr >>= lpm->addrBits - length;
    status = lpmFindPayload(lpm, &node, &oldPayload, &oldClsA, &oldClsB,
                            &oldDropMaskLo, &oldDropMaskHi, addr,
                            length, payloadKind, work);
    if (status != SB_OK) return status;
    if (payloadKind == G2P3_LPM_PAYLOAD_MAIN) {
        status = g2p3_lpmHPMRef(lpm->hpm, &payloadHandle, newPayload);
        if (status != SB_OK) return status;
        newPayload = (void *) payloadHandle;
    }
    payload = oldPayload;
    clsA = oldClsA;
    clsB = oldClsB;
    dropMaskLo = oldDropMaskLo;
    dropMaskHi = oldDropMaskHi;
    extractPayload(&payload, &clsA, &clsB, &dropMaskLo,
                   &dropMaskHi, payloadKind, newPayload);
    status = g2p3_lpmDPMRef(lpm->dpm, payload, clsA, clsB, dropMaskLo,
                                dropMaskHi);
    if (status != SB_OK) {
        if (payloadKind == G2P3_LPM_PAYLOAD_MAIN)
            g2p3_lpmHPMDeref(lpm->hpm, payloadHandle);
        return status;
    }
    status = refDevPayloads(lpm->dpm, &node->dpsBelow, node->data.payload.child,
                            payloadKind, payload, clsA, clsB, dropMaskLo,
                            dropMaskHi);
    if (status != SB_OK) {
        g2p3_lpmDPMDeref(lpm->dpm, payload, clsA, clsB, dropMaskLo,
                             dropMaskHi);
        if (payloadKind == G2P3_LPM_PAYLOAD_MAIN)
            g2p3_lpmHPMDeref(lpm->hpm, payloadHandle);
        return status;
    }
    ++node->dpsBelow;
    status = derefDevPayloads(lpm->dpm, &node->dpsBelow,
                              node->data.payload.child,
                              payloadKind, oldPayload, oldClsA, oldClsB,
                              oldDropMaskLo, oldDropMaskHi);
    SB_ASSERT(status == SB_OK);
    status = g2p3_lpmDPMDeref(lpm->dpm, oldPayload, oldClsA, oldClsB,
                                  oldDropMaskLo, oldDropMaskHi);
    SB_ASSERT(status == SB_OK);
    --node->dpsBelow;

    /* when updating drop mask bits, superimpose new bits on
     * top of what was already there - mask is additive. Do not just
     * overwrite old bits with new ones.
     */
    if (payloadKind == G2P3_LPM_PAYLOAD_DROPM_LO) {
        extractPayload(&payload, &clsA, &clsB,
                       (uint32_t *)&node->data.payload.desc,
                       &dropMaskHi, payloadKind, newPayload);
    }
    else if (payloadKind == G2P3_LPM_PAYLOAD_DROPM_HI) {
        extractPayload(&payload, &clsA, &clsB,
                       &dropMaskLo, (uint32_t *)&node->data.payload.desc,
                       payloadKind, newPayload);
    }
    else
        node->data.payload.desc = newPayload;

    node->tag.state = LPM_NODE_STATE_NEW;
    if (payloadKind == G2P3_LPM_PAYLOAD_MAIN) {
        status = g2p3_lpmHPMDeref(lpm->hpm, oldPayload);
        SB_ASSERT(status == SB_OK);
        G2P3_LPM_SNG_ADJ_DEST_OFFSET(work);
        LPM_WORK_CACHE_PAYLOAD(work, node, payload);
        LPM_WORK_CACHE_NODE(work, node);
    }
    return status;
}

sbStatus_t
g2p3_lpmUpdate(g2p3_lpmHandle_t lpm,
               g2p3_lpmAddress_t addr, uint8_t length,
               void *newPayload, g2p3_lpmPayloadKind_t payloadKind,
               uint8_t l3cached)
{
    sbStatus_t status = SB_OK;
    lpmWork_p_t work = NULL;
    lpmWorkMgr_p_t workmgr;
    
    SB_ASSERT(lpm);
    workmgr = (lpmWorkMgr_p_t)lpm->lpmWorkMgr;
    SB_ASSERT(workmgr);

    SB_ASSERT(newPayload);
    g2p3_lpm_stats_update(lpm, G2P3_LPM_STATS_TYPE_UPDATE,
                          G2P3_LPM_STATS_UPD_REQ);
    
    if (lpm->busy) return SB_BUSY_ERR_CODE;

    if(l3cached == 0){ 
        work = g2p3_work_alloc(workmgr);
        /* Single Fast update optimization Sanity *
         * Fast update must work only with Caching turned off. Which
         * means any update must immediately follow by an commit. If not
         * the upper layer(l3) is violating caching norms. This check is
         * to avoid races with fast update and for sanity */
        
        SB_ASSERT(work);         
    }      
    
    if(work) {
        work->dest   = lpm->topTableAddress;
        work->slice  = *lpm->slices;
        work->slices = lpm->slices + 1;
        work->pfxLen = length;
        work->addr   = addr;
    }

    workmgr->thisWorkType = LPM_WORK_UPDATE;

    status = g2p3_lpmUpdate_internal(lpm, addr, length, 
                                     newPayload, payloadKind,
                                     work);
    if(SB_OK == status) {
        g2p3_lpm_stats_update(lpm, G2P3_LPM_STATS_TYPE_UPDATE,
                              G2P3_LPM_STATS_UPD_SUCCESS);      
        
        if(work) {
            g2p3_work_enqueue(workmgr, work);
        }
    } else {
        g2p3_work_collapse(workmgr, work);
    } 
    return status;
}

/* see contract in g2p3_lpm.h */
sbStatus_t
g2p3_lpmFind(g2p3_lpmHandle_t lpm,
                 void **result,
                 g2p3_lpmAddress_t addr,
                 uint8_t length,
                 g2p3_lpmPayloadKind_t payloadKind)
{
    sbStatus_t status;
    lpmNode_p_t node;

    SB_ASSERT(lpm);

    addr >>= lpm->addrBits - length;
    status = lpmFindPayload(lpm, &node, NULL, NULL, NULL, NULL, NULL,
                            addr, length, payloadKind, NULL);
    if (status != SB_OK) return status;
    if (result) *result = g2p3_lpmHPMData(node->data.payload.desc);
    return SB_OK;
}

/* Prunes singleton tables dead due to decompression */
static
sbStatus_t
lpmSingleAddPruneDead(g2p3_lpmHandle_t lpm, 
                      lpmNode_p_t      deadnode, 
                      lpmNode_p_t      *newnodechild,
                      uint8_t          *slices)
{
    sbStatus_t status = SB_OK;
    /* singleton table size */
#define G2P3_SINGLETON_TBL_SIZE (2)
    lpmNode_p_t next = NULL; /* replacement for node */

    SB_ASSERT(lpm);
    SB_ASSERT(deadnode);
    SB_ASSERT(newnodechild);
    SB_ASSERT(slices);
    
    SB_ASSERT(deadnode->tag.type == LPM_NODE_TABLE);    
    SB_ASSERT(deadnode->tag.state == LPM_NODE_STATE_DEAD);
    /* Dead table node has to be decompressed by singleton optimization */
    SB_ASSERT(deadnode->tag.singleton == 1);
    
    next = deadnode->data.table.child;

    if ((int)(slices - lpm->slices) % 2) {
      status = g2p3_lpmDeviceMemFree(lpm->deviceMemB,
                                    deadnode->data.table.address,
                                    G2P3_SINGLETON_TBL_SIZE);
    } else {
      status = g2p3_lpmDeviceMemFree(lpm->deviceMemA,
                                   deadnode->data.table.address,
                                   G2P3_SINGLETON_TBL_SIZE);
    }
    SB_ASSERT(status == SB_OK);

    /* deallocate node */
    status = g2p3_lpmHostMemFree(lpm->hostMem, deadnode, sizeof(lpmNode_t));
    SB_ASSERT(status == SB_OK);
    *newnodechild = next;
    return status;
}

/* see prototype for contract */
static
void
commitDone(g2p3_lpmHandle_t lpm)
{
    lpmWorkMgr_p_t workmgr;
    SB_ASSERT(lpm);
    SB_ASSERT(lpm->busy);

    workmgr = (lpmWorkMgr_p_t)lpm->lpmWorkMgr;
    SB_ASSERT(workmgr);

    /* If LPM status is not ok dont prune the trie */
    if (lpm->status == SB_OK){
        /* Pruning can be avoided for single add if singleton table
         * decompression was not performed. 
         * Delete performs compression of singleton table only during 
         * pruning. So if the previous operation was single delete +
         * fast update, whole pruning has to be performed. 
         * Else skip pruning to save commit time
         */
        /* if this is a fast update and previous was too */
        if(workmgr->thisCommitType == LPM_COMMIT_FAST) {
           /* 
           * If last update was not fast update and singleton compression(del)
           * or decompression(add) did not happen incremental update is sane 
           * 
           * If last udpate was fast update and [not singleton && delete] do a
           * incremental update
           */
           if((workmgr->numPruneWork > 0) &&
              (workmgr->thisWorkType == LPM_WORK_ADD) &&
              (!workmgr->delPrunePending)) {
               lpmPruneWork_p_t prune = NULL;
                 
               while((prune = g2p3_lpm_prune_work_dequeue(workmgr)) && \
                     (lpm->status == SB_OK)) {
                   lpm->status = lpmSingleAddPruneDead(lpm, 
                                                       prune->node,
                                                       prune->child,
                                                       prune->slices);
                   /*soc_cm_print("\n@@@ Incremental Prune Dead @@@");*/
               } 
           } else if (workmgr->thisWorkType == LPM_WORK_DEL ||
                      workmgr->delPrunePending) {
               workmgr->delPrunePending = FALSE;

               /* init delete prune work item */
               workmgr->delPruneWork.dest   = lpm->topTableAddress;
               workmgr->delPruneWork.slice  = *lpm->slices;
               workmgr->delPruneWork.slices = lpm->slices + 1;

               /* do prefix based prune */
               lpm->status = lpmPruneDeadPrefixPath(lpm, lpm->path->addr, lpm->path->length);

               /*soc_cm_print("\n## Prune Dead ##");*/

               /* If this is a fast delete and prune compressed singleton
                * table, commit the new changes */
               if(workmgr->thisWorkType == LPM_WORK_DEL &&
                  workmgr->delPrunePending) {
                   /* peform an incremental commit to transfer the 
                   * singleton compressed table */
                   commitDelPruneWorkItem(lpm);
               }
           } else {
             /* skip prune */
             /*soc_cm_print("\n$$ Commit Skipping Prune Dead $$");*/
           }
           
        } else {
            workmgr->delPrunePending = FALSE; 
            lpm->status = lpmPruneDead(lpm, &lpm->trie, lpm->slices);
        }
    }
    lpm->busy = FALSE;
    lpm->resetCommit = FALSE;
    
    /* Re-int work item manager */
    g2p3_work_mgr_init(workmgr);
    
    if (lpm->cb && lpm->async)
        lpm->cb(lpm->cbUserData, lpm->status);
}

/* see prototype for contract */
static
void
commitDoneCB(void *cbData, sbStatus_t status)
{
    g2p3_lpmHandle_t lpm;

    lpm = (g2p3_lpmHandle_t) cbData;
    SB_ASSERT(lpm);
    SB_ASSERT(lpm->busy);
    lpm->async = TRUE;
    lpm->status = status;
    /*Fixme: Asyn dma has to be aware of fast update */
    commitDone(lpm);
}

/* see prototype for contract */
static
sbBool_t
lpmAnyDirty(const lpmPath_p_t path)
{
    g2p3_lpmPayloadKind_t kind;
    sbBool_t dirty;

    SB_ASSERT(path);
    dirty = path->dirtyTable;
    for (kind = 0; kind < G2P3_LPM_PAYLOAD_NUM_KINDS; ++kind)
        dirty |= path->dirty[kind];
    return dirty;
}

static
sbStatus_t 
commitPath(g2p3_lpmHandle_t lpm,
           lpmPath_p_t path)
{

    sbDmaAddress_t dmaAddress;
    uint32_t dmaValue;
    uint8_t *pDmaBuf = (uint8_t *) &dmaValue;
    soc_sbx_g2p3_lpmnode_t tblNode;

    SB_ASSERT(lpm);
    SB_ASSERT(path);

    while (lpm->pathLength > 0) {
        if (path->node == NULL || (path->node->tag.state != LPM_NODE_STATE_DEAD
                                   && path->node->dpsBelow == 0)) {
            --lpm->pathLength;
            if (lpm->resetCommit
                || lpmAnyDirty(path)
                || (path->node && path->node->dpsBelow == 0)) {
                /* write payload addresses */
                soc_sbx_g2p3_lpmnode_t_init(&tblNode);
                tblNode.leaf    = 1;
                tblNode.pointer = path->payloadDevAddr;
                tblNode.pattern = path->payloadNodeExtra;
                soc_sbx_g2p3_lpmnode_pack(lpm->fe->unit, &tblNode, pDmaBuf,
                                            lpm->payloadSize);
                
                if ((int)(path->slices - lpm->slices) % 2) {
                  dmaAddress = 
                    SB_FE2000_DMA_MAKE_ADDRESS(lpm->dmaBankA, path->dest);
                } else {
                  dmaAddress =
                    SB_FE2000_DMA_MAKE_ADDRESS(lpm->dmaBankB, path->dest);
                }
                lpm->status = g2p3_lpmDmaWrite(lpm->dma,
                                                   dmaAddress, 1 << path->slice,
                                                   dmaValue, lpm->maxPathLength - lpm->pathLength,
                                                   commitCB, (void *) lpm);
#if LPM_LPM_DEBUG
                SB_LOG("\n %d Leaf entries set at 0x%x, 0x%x", (1 << path->slice), path->dest, dmaAddress);
                SB_LOG("\n Ptr: %x Pattern: %x Sing: %x Leaf: %x", tblNode.pointer, tblNode.pattern, tblNode.singleton, tblNode.leaf);
#endif
                if (lpm->status == SB_IN_PROGRESS)
                    return lpm->status;
                if (lpm->status != SB_OK) {
                    commitDone(lpm);
                    return lpm->status;
                }
            }
            --path;
        } else { /* path->node != NULL, path->node->dpsBelow > 0 */
            SB_ASSERT(path->node->tag.state == LPM_NODE_STATE_DEAD
                   || path->node->dpsBelow > 0);
            SB_ASSERT(path->node->tag.type == LPM_NODE_PAYLOAD
                   || path->node->tag.type == LPM_NODE_TABLE
                   || path->node->tag.state == LPM_NODE_STATE_LIVE);
            switch (path->node->tag.type) {
            case LPM_NODE_PAYLOAD:
                path->dirty[path->node->tag.payloadKind] =
                    (path->node->tag.state == LPM_NODE_STATE_NEW
                     || path->node->tag.state == LPM_NODE_STATE_DEAD);
                if (path->node->tag.state != LPM_NODE_STATE_DEAD) {
                    extractPayload(&path->payload, &path->clsA, &path->clsB,
                                   &path->dropMaskLo, &path->dropMaskHi,
                                   path->node->tag.payloadKind,
                                   path->node->data.payload.desc);
                    if (path->node->tag.payloadKind ==
                        G2P3_LPM_PAYLOAD_CLS_C)
                        path->clsC = (g2p3_lpmClassifierId_t)
                            (uint32_t)path->node->data.payload.desc;

                    lpm->status = g2p3_lpmDPMInfo(lpm->dpm,
                                                      &path->payloadDevAddr,
                                                      &path->payloadNodeExtra,
                                                      path->payload,
                                                      path->clsA,
                                                      path->clsB,
                                                      path->dropMaskLo,
                                                      path->dropMaskHi);

#if LPM_LPM_DEBUG
                    if (lpm->status != SB_OK) {
                        SB_LOG("g2p3_lpmDPMAddress non-OK on: payload dev addr 0x%08x, payload = %p, clsa = %x, clsbGu = %x, masklo = %x, maskhi = %x\n",
                               path->payloadDevAddr, path->payload, path->clsA, path->clsB, path->dropMaskLo, path->dropMaskHi);
                    }
#endif
                    SB_ASSERT(lpm->status == SB_OK);
                } /* if (state != LPM_NODE_STATE_DEAD) */
                if (path->node->tag.state == LPM_NODE_STATE_NEW) {
                    path->node->tag.state = LPM_NODE_STATE_LIVE;
                }
                path->node = path->node->data.payload.child;
                break;
            case LPM_NODE_TABLE:
                if (path->node->tag.state == LPM_NODE_STATE_DEAD) {
                    path->node = path->node->data.table.child;
                } else {
                    switch (path->seen) {
                    case 0:
                        SB_ASSERT(lpm->pathLength < lpm->maxPathLength);
                        SB_ASSERT(path->slice == 0);
                        *(path + 1) = *path;
                        ++path->seen;
                        ++path;
                        path->slice = *path->slices;
                        ++path->slices;
                        path->dirtyTable =
                            (path->node->tag.state == LPM_NODE_STATE_NEW);
                        path->dest = path->node->data.table.address;
                        path->node =
                            lpmFirstLive(path->node->data.table.child);
                        ++lpm->pathLength;
                        if ((path - 1)->node->tag.singleton) {
                            SB_ASSERT(path->node);
                            SB_ASSERT(path->node->tag.type == LPM_NODE_SKIP);
                            path->node = path->node->data.skip.child;
                            path->slice = 0;
                            if (lpm->resetCommit || lpmAnyDirty(path)) {
                                
                                soc_sbx_g2p3_lpmnode_t_init(&tblNode);
                                tblNode.leaf = 1;
                                tblNode.pointer = path->payloadDevAddr;
                                tblNode.pattern = path->payloadNodeExtra;
                                soc_sbx_g2p3_lpmnode_pack(lpm->fe->unit,
                                                            &tblNode, pDmaBuf,
                                                            lpm->payloadSize);

                                if ((int)(path->slices - lpm->slices) % 2) {
                                  dmaAddress =
                                    SB_FE2000_DMA_MAKE_ADDRESS(lpm->dmaBankA, 
                                    path->dest + 1);
                                } else {
                                  dmaAddress =
                                    SB_FE2000_DMA_MAKE_ADDRESS(lpm->dmaBankB, 
                                    path->dest + 1);
                                } 
                                lpm->status =
                                    g2p3_lpmDmaWrite(lpm->dma,
                                                         dmaAddress, 1,
                                                         dmaValue,
                                                         lpm->maxPathLength - lpm->pathLength - 1,
                                                         commitCB,
                                                         (void *) lpm);
#if LPM_LPM_DEBUG
                               SB_LOG("\n Table set at 0x%x, 0x%x", path->dest + 1, dmaAddress);
                               SB_LOG("\n Ptr: %x Pattern: %x Sing: %x Leaf: %x", tblNode.pointer, tblNode.pattern, tblNode.singleton, tblNode.leaf);
#endif
                                if (lpm->status == SB_IN_PROGRESS)
                                    return lpm->status;
                                if (lpm->status != SB_OK) {
                                    commitDone(lpm);
                                    return lpm->status;
                                }
                            }
                        }
                        break;
                    case 1:
                        --lpm->pathLength;
                        if (lpm->resetCommit
                            || path->dirtyTable
                            || path->node->tag.state == LPM_NODE_STATE_NEW) {

                            path->node->tag.state = LPM_NODE_STATE_LIVE;
                            
                            soc_sbx_g2p3_lpmnode_t_init(&tblNode);
                            tblNode.leaf = 0;
                            tblNode.pointer = path->node->data.table.address;
                            tblNode.pattern = 0;
                            
                            if (path->node->tag.singleton) {
                                lpmNode_p_t liveChild =
                                    lpmFirstLive(path->node->data.table.child);
                                SB_ASSERT(liveChild);
                                SB_ASSERT(liveChild->tag.type
                                       == LPM_NODE_SKIP);
                                
                                tblNode.singleton = 1;
#if 0                                
                                tblNode.pattern = liveChild->data.skip.address;
#else
                                /* encode to mext format */
                                /* Value / Mask Pair:
                                 * Cutting short pyld pointer to 21 bits we have 9 bits
                                 * left out for pattern. 
                                 * Of the 9b, use upto upper 8b for Value based on slice
                                 * lengths (max slice length of 8)
                                 * Use the rest of bits for '0' and mask
                                 */
                                #define G2P3_LPM_MAX_PATTERN_BITS 9

                                tblNode.pattern = (liveChild->data.skip.address << 
                                                    (*path->slices - liveChild->tag.skipLen -1));
                                
                                tblNode.pattern <<= (G2P3_LPM_MAX_PATTERN_BITS - *path->slices);

                                /*-2 - 1 bit accounted for 0 based skip length and 1 bit for 
                                 * 0 before the mask */
                                tblNode.pattern |= ((1 << (G2P3_LPM_MAX_PATTERN_BITS \
                                                          - liveChild->tag.skipLen - 2)) - 1);
                                
#endif                                  
                                /*
                                soc_cm_print("\n SingleTon Table: ");
                                soc_sbx_g2p3_lpmnode_print(lpm->fe->unit, &tblNode);
                                */
                            }

                            soc_sbx_g2p3_lpmnode_pack(lpm->fe->unit,
                                                        &tblNode, pDmaBuf,
                                                        lpm->payloadSize);

                            if ((int)(path->slices - lpm->slices) % 2) {
                              dmaAddress =
                                SB_FE2000_DMA_MAKE_ADDRESS(lpm->dmaBankA, 
                                path->dest);
                            } else {
                              dmaAddress =
                                SB_FE2000_DMA_MAKE_ADDRESS(lpm->dmaBankB, 
                                path->dest);
                            } 
                            lpm->status =
                                g2p3_lpmDmaWrite(lpm->dma, dmaAddress, 1,
                                                     dmaValue,
                                                     lpm->maxPathLength - lpm->pathLength,
                                                     commitCB,
                                                     (void *) lpm);
#if LPM_LPM_DEBUG
                            SB_LOG("\n Table set at 0x%x, 0x%x", path->dest, dmaAddress);
                            SB_LOG("\n Ptr: %x Pattern: %x Sing: %x Leaf: %x", tblNode.pointer, tblNode.pattern, tblNode.singleton, tblNode.leaf);
#endif
                            if (lpm->status == SB_IN_PROGRESS)
                                return lpm->status;
                            if (lpm->status != SB_OK) {
                                commitDone(lpm);
                                return lpm->status;
                            }
                        }
                        --path;
                        break;
                    default:
                        SB_ASSERT(0);
                    }
                }
                break;
            case LPM_NODE_SPLIT:
                SB_ASSERT(lpm->pathLength < lpm->maxPathLength);
                SB_ASSERT(path->slice > 0);
                --path->slice;
                *(path+1) = *path;
                path->dest += 1 << path->slice;
                path->node = path->node->data.split.children[1];
                ++path;
                path->node = path->node->data.split.children[0];
                ++lpm->pathLength;
                break;
            case LPM_NODE_SKIP: {
                uint8_t skipLen = path->node->tag.skipLen + 1;
                uint8_t skipLenDiff = path->slice - skipLen;
                uint32_t skipAddr = path->node->data.skip.address;

                SB_ASSERT(path->slice >= skipLen);
                switch (path->seen) {
                case 0:
                    SB_ASSERT(lpm->pathLength < lpm->maxPathLength);
                    *(path + 1) = *path;
                    ++path->seen;
                    ++path;
                    path->slice -= skipLen;
                    path->dest += skipAddr << skipLenDiff;
                    path->node = path->node->data.skip.child;
                    ++lpm->pathLength;
                    if (lpm->resetCommit || lpmAnyDirty(path)) {
                        uint32_t skipAmt = skipAddr << skipLenDiff;
                        
                        soc_sbx_g2p3_lpmnode_t_init(&tblNode);
                        tblNode.leaf = 1;
                        tblNode.pointer = path->payloadDevAddr;
                        tblNode.pattern = path->payloadNodeExtra;
                        soc_sbx_g2p3_lpmnode_pack(lpm->fe->unit, &tblNode,
                                                    pDmaBuf, lpm->payloadSize);
                        if ((int)((path-1)->slices - lpm->slices) % 2) {
                          dmaAddress =
                            SB_FE2000_DMA_MAKE_ADDRESS(lpm->dmaBankA, 
                            (path-1)->dest);
                        } else {
                          dmaAddress =
                            SB_FE2000_DMA_MAKE_ADDRESS(lpm->dmaBankB, 
                            (path-1)->dest);
                        } 

                        lpm->status =
                            g2p3_lpmDmaWrite(lpm->dma, dmaAddress, skipAmt,
                                                 dmaValue, lpm->maxPathLength - lpm->pathLength - 1,
                                                 commitCB, (void *) lpm);
#if LPM_LPM_DEBUG
                        SB_LOG("\n %d Entries set at 0x%x, 0x%x", skipAmt, (path-1)->dest, dmaAddress);
                        SB_LOG("\n Ptr: %x Pattern: %x Sing: %x Leaf: %x", tblNode.pointer, tblNode.pattern, tblNode.singleton, tblNode.leaf);
#endif
                        if (lpm->status == SB_IN_PROGRESS)
                            return lpm->status;
                        if (lpm->status != SB_OK) {
                            commitDone(lpm);
                            return lpm->status;
                        }
                    }
                    break;
                case 1:
                    --lpm->pathLength;
                    if (lpm->resetCommit || lpmAnyDirty(path)) {
                        uint32_t skipAmt = (skipAddr + 1) << skipLenDiff;
                        
                        soc_sbx_g2p3_lpmnode_t_init(&tblNode);
                        tblNode.leaf = 1;
                        tblNode.pointer = path->payloadDevAddr;
                        tblNode.pattern = path->payloadNodeExtra;
                        soc_sbx_g2p3_lpmnode_pack(lpm->fe->unit, &tblNode,
                                                    pDmaBuf, lpm->payloadSize);
                        if ((int)(path->slices - lpm->slices) % 2) {
                          dmaAddress =
                            SB_FE2000_DMA_MAKE_ADDRESS(lpm->dmaBankA, 
                            path->dest + skipAmt);
                        } else {
                          dmaAddress =
                            SB_FE2000_DMA_MAKE_ADDRESS(lpm->dmaBankB, 
                            path->dest + skipAmt);
                        } 

                        lpm->status =
                            g2p3_lpmDmaWrite(lpm->dma, dmaAddress,
                                                 ((1 << path->slice) - skipAmt),
                                                 dmaValue, lpm->maxPathLength - lpm->pathLength, commitCB,
                                                 (void *) lpm);
#if LPM_LPM_DEBUG
                        SB_LOG("\n %d Leaf entries set at 0x%x, 0x%x", ((1<<path->slice)-skipAmt), path->dest + skipAmt, dmaAddress);
                        SB_LOG("\n Ptr: %x Pattern: %x Sing: %x Leaf: %x", tblNode.pointer, tblNode.pattern, tblNode.singleton, tblNode.leaf);
#endif
                        if (lpm->status == SB_IN_PROGRESS)
                            return lpm->status;
                        if (lpm->status != SB_OK) {
                            commitDone(lpm);
                            return lpm->status;
                        }
                    }
                    --path;
                    break;
                default:
                    SB_ASSERT(0);
                }
                break;
            } /* case LPM_NODE_SKIP */
            default:
                SB_ASSERT(0);
            } /* switch (path->node->tag.type) */
        } /* path->node != NULL, path->node->dpsBelow > 0 */

    } /* while (lpm->pathLength > 0) */

    return SB_OK;
}

static 
sbStatus_t
commitWorkItem(g2p3_lpmHandle_t lpm)
{
    lpmPath_p_t path;
    lpmWork_p_t work = NULL;
    int pk=0;

    lpm->status = SB_OK;
    
    while((work = g2p3_work_dequeue(lpm->lpmWorkMgr)) && (lpm->status == SB_OK)) {
        path             = lpm->path;
        path->node       = work->node;
        path->dest       = work->dest;
        path->slice      = work->slice;
        path->slices     = work->slices;
        path->seen       = 0;
        path->clsA       = lpm->defaultClsA;
        path->clsB       = lpm->defaultClsB;
        path->dropMaskLo = lpm->defaultDropMaskLo;
        path->dropMaskHi = lpm->defaultDropMaskHi;
        path->clsC = 0;
        for (pk = G2P3_LPM_PAYLOAD_MAIN;
             pk < G2P3_LPM_PAYLOAD_NUM_KINDS; ++pk)
            path->dirty[pk] = FALSE;
        path->dirtyTable = FALSE;
        lpm->pathLength = 1;
        
        extractPayload(&path->payload, &path->clsA, &path->clsB,
                       &path->dropMaskLo, &path->dropMaskHi,
                       G2P3_LPM_PAYLOAD_MAIN,
                       work->payMain);
        path->addr   = work->addr;
        path->length = work->pfxLen;
                    
        /* Acquire DPM payload information from work item */
        lpm->status = g2p3_lpmDPMInfo(lpm->dpm,
                                     &path->payloadDevAddr,
                                     &path->payloadNodeExtra,
                                      work->payMain,
                                      path->clsA,
                                      path->clsB,
                                      path->dropMaskLo,
                                      path->dropMaskHi);
        SB_ASSERT(lpm->status == SB_OK);

        lpm->status = commitPath(lpm, path);
        SB_ASSERT(lpm->status == SB_OK);
    }
    return lpm->status;
}

void
commitDelPruneDone(g2p3_lpmHandle_t lpm)
{
    /* dummy call back function */
    SB_ASSERT(lpm);
    SB_ASSERT(lpm->busy);
}

void
commitDelPruneCB(void *cbData, sbStatus_t status)
{
    g2p3_lpmHandle_t lpm;

    lpm = (g2p3_lpmHandle_t) cbData;
    SB_ASSERT(lpm);
    SB_ASSERT(lpm->busy);
    lpm->async = TRUE;
    lpm->status = status;
    /*Fixme: Asyn dma has to be aware of fast update */
    commitDelPruneDone(lpm);
}

/* see prototype for contract */
void
commitDelPruneWorkItem(g2p3_lpmHandle_t lpm)
{
    lpmPath_p_t path;
    lpmWork_p_t work;
    lpmWorkMgr_p_t workmgr;
    int pk=0;
    
    SB_ASSERT(lpm);
    SB_ASSERT(lpm->busy);
    SB_ASSERT(lpm->path);
    SB_ASSERT(lpm->pathLength <= lpm->addrBits + 1);
    SB_ASSERT(LPMNODE_SIZE_IN_BYTES == sizeof(uint32_t));

    workmgr = (lpmWorkMgr_p_t)lpm->lpmWorkMgr;
    SB_ASSERT(workmgr);

    work = &workmgr->delPruneWork;
    SB_ASSERT(work->nodeCached);

    /* If delete pruning node is cached */
    SB_ASSERT(workmgr->delPrunePending == TRUE);
    path             = lpm->path;
    path->node       = work->node;
    path->dest       = work->dest;
    path->slice      = work->slice;
    path->slices     = work->slices;
    path->seen       = 0;
    path->clsA       = lpm->defaultClsA;
    path->clsB       = lpm->defaultClsB;
    path->dropMaskLo = lpm->defaultDropMaskLo;
    path->dropMaskHi = lpm->defaultDropMaskHi;
    path->clsC       = 0;

    for (pk = G2P3_LPM_PAYLOAD_MAIN;
         pk < G2P3_LPM_PAYLOAD_NUM_KINDS; ++pk)
        path->dirty[pk] = FALSE;
    path->dirtyTable = FALSE;
    lpm->pathLength = 1;
    
    extractPayload(&path->payload, &path->clsA, &path->clsB,
                   &path->dropMaskLo, &path->dropMaskHi,
                   G2P3_LPM_PAYLOAD_MAIN,
                   work->payMain);
                
    /* Acquire DPM payload information from work item */
    lpm->status = g2p3_lpmDPMInfo(lpm->dpm,
                                 &path->payloadDevAddr,
                                 &path->payloadNodeExtra,
                                  work->payMain,
                                  path->clsA,
                                  path->clsB,
                                  path->dropMaskLo,
                                  path->dropMaskHi);
    SB_ASSERT(lpm->status == SB_OK);

    TIME_STAMP_START;
    lpm->status = commitPath(lpm, path);
    TIME_STAMP;
    SB_ASSERT(lpm->status == SB_OK);

    /* set prune pending to false to skip prunedead on commit done*/
    workmgr->delPrunePending = FALSE;
    
    TIME_STAMP_START;
    lpm->status = g2p3_lpmDmaFlush(lpm->dma, commitDelPruneCB, (void *) lpm);
    TIME_STAMP;
    
    if (lpm->status != SB_IN_PROGRESS) {
        if (lpm->busy == TRUE)
            TIME_STAMP_START;
            commitDelPruneDone(lpm);
            TIME_STAMP;
    }
}

/* see prototype for contract */
static
void
commit(g2p3_lpmHandle_t lpm)
{
    lpmWorkMgr_p_t workmgr;
    lpmPath_p_t path;
    sbStatus_t  status = SB_OK;
      
    SB_ASSERT(lpm);
    SB_ASSERT(lpm->busy);
    SB_ASSERT(lpm->path);
    SB_ASSERT(lpm->pathLength <= lpm->addrBits + 1);
    SB_ASSERT(LPMNODE_SIZE_IN_BYTES == sizeof(uint32_t));
    workmgr = (lpmWorkMgr_p_t)lpm->lpmWorkMgr;
    SB_ASSERT(workmgr);
    
    if(g2p3_is_fast_update(workmgr)){
        workmgr->thisCommitType = LPM_COMMIT_FAST;
    } else {
        workmgr->thisCommitType = LPM_COMMIT_NORMAL;
    }
    
    TIME_STAMP_START;
    if(workmgr->thisCommitType == LPM_COMMIT_FAST) {
        status = commitWorkItem(lpm);
    } else {
        path = lpm->path + lpm->pathLength - 1;
        status = commitPath(lpm, path);
    }
    TIME_STAMP;

    if(status == SB_OK) {
#if LPM_LPM_DEBUG
        SB_LOG("LPM commit: calling g2p3_lpmDmaFlush\n");
#endif
        TIME_STAMP_START;
        lpm->status = g2p3_lpmDmaFlush(lpm->dma, commitDoneCB, (void *) lpm);
        TIME_STAMP;
        
        if (lpm->status != SB_IN_PROGRESS) {
            if (lpm->busy == TRUE)
                TIME_STAMP_START;
                commitDone(lpm);
                TIME_STAMP;
        }
    }

    workmgr->lastCommitType = workmgr->thisCommitType;
    workmgr->lastWorkType   = workmgr->thisWorkType;
}

/* see prototype for contract */
static
void
commitCB(void *cbData, sbStatus_t status)
{ 
    lpmWorkMgr_p_t workmgr;
    g2p3_lpmHandle_t lpm;
    lpmPath_p_t path;


    lpm = (g2p3_lpmHandle_t) cbData;
    SB_ASSERT(lpm);
    SB_ASSERT(lpm->busy);

    workmgr = (lpmWorkMgr_p_t)lpm->lpmWorkMgr;
    SB_ASSERT(workmgr);

    lpm->async = TRUE;
    lpm->status = status;
    /* if async dma failed && this is not a prune delete commit */
    if ((lpm->status != SB_OK) && (workmgr->delPruneWork.nodeCached)) {
        commitDone(lpm);
    }
    else{
        /* Async call backs are not used, verify this logic */
        path = lpm->path + lpm->pathLength;
        commitPath(lpm, path);
    }
}

/* see contract in g2p3_lpm.h */
sbStatus_t
g2p3_lpmCommit(g2p3_lpmHandle_t lpm,
                   g2p3_lpmCallback_f_t asyncCallback,
                   void *asyncCallbackUserData)
{
    g2p3_lpmPayloadKind_t pk;

    SB_ASSERT(lpm);
    SB_ASSERT(lpm->slices);
    SB_ASSERT(*lpm->slices != 0);
    SB_ASSERT(lpm->path);
    g2p3_lpm_stats_update(lpm, G2P3_LPM_STATS_TYPE_COMMIT,
                          G2P3_LPM_STATS_UPD_REQ);

    if (lpm->busy) return SB_BUSY_ERR_CODE;
    SB_ASSERT(lpm->pathLength == 0);
    lpm->busy = TRUE;
    lpm->async = FALSE;
    lpm->cb = asyncCallback;
    lpm->cbUserData = asyncCallbackUserData;
    lpm->path->node = lpm->trie;
    lpm->path->dest = lpm->topTableAddress;
    lpm->path->slice = *lpm->slices;
    lpm->path->slices = lpm->slices + 1;
    lpm->path->seen = 0;
    lpm->path->payload = lpm->defaultPayload;
    lpm->path->clsA = lpm->defaultClsA;
    lpm->path->clsB = lpm->defaultClsB;
    lpm->path->dropMaskLo = lpm->defaultDropMaskLo;
    lpm->path->dropMaskHi = lpm->defaultDropMaskHi;
    lpm->path->clsC = 0;
    for (pk = G2P3_LPM_PAYLOAD_MAIN;
         pk < G2P3_LPM_PAYLOAD_NUM_KINDS; ++pk)
        lpm->path->dirty[pk] = FALSE;
    lpm->path->dirtyTable = FALSE;
    lpm->status = g2p3_lpmDPMInfo(lpm->dpm,
                                      &lpm->path->payloadDevAddr,
                                      &lpm->path->payloadNodeExtra,
                                      lpm->path->payload,
                                      lpm->path->clsA,
                                      lpm->path->clsB,
                                      lpm->path->dropMaskLo,
                                      lpm->path->dropMaskHi);
    SB_ASSERT(lpm->status == SB_OK);
    lpm->pathLength = 1;
    lpm->status = g2p3_lpmDPMCommit(lpm->dpm, commitCB, (void *) lpm);

    if (lpm->status == SB_IN_PROGRESS)
        return (lpm->status);

    if (lpm->status != SB_OK) {
        commitDone(lpm);
        return lpm->status;
    }

    commit(lpm);
    if(lpm->status == SB_OK){
      g2p3_lpm_stats_update(lpm, G2P3_LPM_STATS_TYPE_COMMIT,
                            G2P3_LPM_STATS_UPD_SUCCESS); 
    }
    return lpm->status;
}

/* see prototype for contract */
static
void
extractPayload(g2p3_lpmPayloadHandle_t *pMain, /* main payload */
               g2p3_lpmClassifierId_t *pClsA,       /* classifier id A */
               g2p3_lpmClassifierId_t *pClsB,       /* classifier id B */
               uint32_t *pDropMaskLo, uint32_t *pDropMaskHi,
               g2p3_lpmPayloadKind_t payloadKind, void *payload)
{
    uint32_t auxMask,i,j;

    SB_ASSERT(pMain);
    SB_ASSERT(pClsA);
    SB_ASSERT(pClsB);

    switch (payloadKind) {
    case G2P3_LPM_PAYLOAD_MAIN:
        *pMain = (g2p3_lpmPayloadHandle_t) payload;
        break;
    case G2P3_LPM_PAYLOAD_CLS_A:
        *pClsA = ((g2p3_lpmClassifierId_t) (uint32_t) payload);
        break;
    case G2P3_LPM_PAYLOAD_CLS_B:
        *pClsB = ((g2p3_lpmClassifierId_t) (uint32_t) payload);
        break;
    case G2P3_LPM_PAYLOAD_DROPM_LO:
        auxMask = ((uint32_t)payload >> 12) & 0xFFF;
        for (i = 0; i < 12; i++) {
            j = (1 << i);
            if (auxMask & j) {
                if ((uint32_t)payload & j)
                    *pDropMaskLo |= j;
                else
                    *pDropMaskLo &= ~j;
            }
        }
        *pDropMaskLo |= (auxMask << 12);
        break;
    case G2P3_LPM_PAYLOAD_DROPM_HI:
        auxMask = ((uint32_t)payload >> 12) & 0xFFF;
        for (i = 0; i < 12; i++) {
            j = (1 << i);
            if (auxMask & j) {
                if ((uint32_t)payload & j)
                    *pDropMaskHi |= j;
                else
                    *pDropMaskHi &= ~j;
            }
        }
        *pDropMaskHi |= (auxMask << 12);
        break;
    case G2P3_LPM_PAYLOAD_CLS_C:
        break;
    default:
        SB_ASSERT(0);
    }
}

/* see prototype for contract */
static
unsigned
lcplen(g2p3_lpmAddress_t addr1, unsigned len1,
       g2p3_lpmAddress_t addr2, unsigned len2)
{
    g2p3_lpmAddress_t diff;
    unsigned len = len1 < len2 ? len1 : len2;

    addr1 = (addr1 >> (len1 - len)) & ((1ULL << len) - 1ULL);
    addr2 = (addr2 >> (len2 - len)) & ((1ULL << len) - 1ULL);
    diff = addr1 ^ addr2;
    while (diff) {		/* there are faster but not-so-portable
                                 * ways */
        diff >>= 1;
        --len;
    }
    return len;
}

/* see prototype for contract */
static
sbBool_t
lpmIsDead(lpmNode_p_t node)
{
    SB_ASSERT(node);
    switch (node->tag.type) {
    case LPM_NODE_PAYLOAD:
    case LPM_NODE_TABLE:
        return (node->tag.state == LPM_NODE_STATE_DEAD);
    default:
        return FALSE;
    }
}

/* see prototype for contract */
static
lpmNode_p_t
lpmFirstLive(lpmNode_p_t node)
{
    while (node != NULL && lpmIsDead(node)) {
        switch (node->tag.type) {
        case LPM_NODE_PAYLOAD:
            node = node->data.payload.child;
            break;
        case LPM_NODE_TABLE:
            node = node->data.table.child;
            break;
        default:
            SB_ASSERT(0); /* only payloads and tables can be dead */
            node = NULL;
        }
    }
    return node;
}

#if 0
/* see prototype for contract */
static
uint32_t
lpmEncodeSkip(g2p3_lpmAddress_t addr, uint8_t nbits)
{
    SB_ASSERT(nbits > 0);
    SB_ASSERT(nbits <= MAX_SKIP_LEN);
    SB_ASSERT(addr < (1 << nbits));
    return (((addr << 1) | 1) << (MAX_SKIP_LEN - nbits)) - 1;
}
#endif

/* see prototype for contract */
static
sbBool_t
lpmIsSingletonPath(lpmNode_p_t node, uint8_t slice)
{
    if (node == NULL || node->tag.type != LPM_NODE_SKIP) return FALSE;
    if(slice > MAX_SKIP_LEN) return FALSE;
    if (node->tag.skipLen + 1 == slice) return TRUE;
    for (node = lpmFirstLive(node->data.skip.child);
         node != NULL; node = lpmFirstLive(node->data.payload.child))
        if (node->tag.type != LPM_NODE_PAYLOAD) return FALSE;
    return TRUE;
}

sbStatus_t
g2p3_lpmWalkReset(g2p3_lpmHandle_t lpm, g2p3_lpmWalkHandle_t *WalkHandle)
{
    sbStatus_t s;
    g2p3_lpmWalkDesc_p_t pDesc = *(g2p3_lpmWalkDesc_p_t *) WalkHandle;

   if (!pDesc) {
       s = g2p3_lpmHostMemAlloc(lpm->hostMem, (void **) WalkHandle,
                                sizeof(g2p3_lpmWalkDesc_t), "lpm walk");
       if (s != SB_OK) {
           return s;
       }
       pDesc = *(g2p3_lpmWalkDesc_p_t *) WalkHandle;
   }

   pDesc->initdone = 0;
   return SB_OK;
}

/**
 * This function takes the pWalkHandle to establish the
 * walk context and walks the LPM Trie. The pWalkHandle
 * is assumed to be created by the g2p3_lpmGetWalkToken
 */
sbStatus_t
g2p3_lpmWalk(g2p3_lpmHandle_t lpm,
                g2p3_lpmAddress_t curraddr,
                uint8_t currlen,
                g2p3_lpmAddress_t *ncurraddr,
                uint8_t *nlength,
                g2p3_lpmPayloadKind_t *pPayLoadKind,
                void **payMain,
                uint32_t *pDropMaskLo,
                uint32_t *pDropMaskHi,
                g2p3_lpmClassifierId_t *pClsA,
                g2p3_lpmClassifierId_t *pClsB,
                g2p3_lpmClassifierId_t *pClsC,
                g2p3_lpmWalkHandle_t WalkHandle)
{
   int found = 0;
   g2p3_lpmClassifierId_t clsA,clsB;
   uint32_t dropMaskLo,dropMaskHi;
   g2p3_lpmPayloadHandle_t payload;
   typedef enum { SPLIT_0 =0, SPLIT_1 = 1, NON_SPLIT=2 } SplitType_t;
   g2p3_lpmWalkDesc_p_t pDesc = (g2p3_lpmWalkDesc_p_t) WalkHandle;

   SB_ASSERT(lpm);

   clsA       = lpm->defaultClsA;
   clsB       = lpm->defaultClsB;
   dropMaskLo = lpm->defaultDropMaskLo;
   dropMaskHi = lpm->defaultDropMaskHi;
   payload    = lpm->defaultPayload;

   if(pDesc->initdone == 0 ) {
    SB_MEMSET (pDesc->walkTree, 0, sizeof(pDesc->walkTree));
    pDesc->path = pDesc->walkTree;

    /* start from the root of the trie  */
    pDesc->path->node = lpm->trie;
    pDesc->path->parent = NON_SPLIT;
    pDesc->pathLength = 1;
   }
   else {
        if( (curraddr == pDesc->prevlpm)  &&
            (currlen == pDesc->prevlen) ) {
              /*
               * this is good otherwise bad
               * user shouldn't be changing the state
               * in the middle.
               */
        }
   }

    while (pDesc->pathLength > 0) {
        if (pDesc->path->node == NULL) {
            /* reached the bottom of the trie, back up (if possible) */
            --pDesc->pathLength;
            --pDesc->path;
        } else {
            /* prune the trie if specific slc number passed in  */

            switch (pDesc->path->node->tag.type) {
            case LPM_NODE_PAYLOAD:
            {
                  if(!found) {
                      if(pDesc->initdone == 0) {
                        if( (curraddr == pDesc->path->addr)  &&
                            (currlen == pDesc->path->len) )
                        {
                            pDesc->initdone = 1;
                        }
                        else {
                            /* if not exact match find the close one */
                            if( (curraddr == pDesc->path->addr) &&
                                (pDesc->path->len < currlen) )
                            {
                                pDesc->initdone = 1;
                            }
                        }
                      } 
                      else {
                        *ncurraddr = pDesc->path->addr;
                        *nlength   = pDesc->path->len;
                        extractPayload(&payload, &clsA, &clsB,
                                            &dropMaskLo,
                                            &dropMaskHi,
                                            pDesc->path->node->tag.payloadKind,
                                            pDesc->path->node->data.payload.desc);
                        /* if (pPayload) *pPayload = payload; */
                        if (pClsA) *pClsA = clsA;
                        if (pClsB) *pClsB = clsB;
                        if (pDropMaskLo) *pDropMaskLo = dropMaskLo;
                        if(pDropMaskHi) *pDropMaskHi = dropMaskHi;
                        if(payMain)     *payMain = payload;
                        if(pPayLoadKind) *pPayLoadKind = pDesc->path->node->tag.payloadKind;
                        found = 1;
                      }
                 }

                /* traverse down  */
                pDesc->path->node = pDesc->path->node->data.payload.child;
                pDesc->path->parent = NON_SPLIT;

            }
            break;

            case LPM_NODE_TABLE:
            {
                switch (pDesc->path->seen) {
                case 0:
                {
                        /* traverse down  */
                        *(pDesc->path + 1) = *pDesc->path;
                        ++pDesc->path->seen;
                        ++pDesc->path;

                        pDesc->path->node = pDesc->path->node->data.table.child;
                        pDesc->path->parent = NON_SPLIT;
                        ++pDesc->pathLength;
                        break;
                }
                case 1: --pDesc->pathLength;
                        --pDesc->path;
                        break;
                default: SB_ASSERT(0);
                }


            }
            break;

            case LPM_NODE_SPLIT:
            {
                *(pDesc->path+1) = *pDesc->path;
                pDesc->path->node = pDesc->path->node->data.split.children[1];
                pDesc->path->addr |= (1ULL << ((lpm->addrBits) - (pDesc->path->len+1)));
                pDesc->path->parent = SPLIT_1;
                ++pDesc->path->len; ++pDesc->path->indent;

                ++pDesc->path;
                pDesc->path->node = pDesc->path->node->data.split.children[0];
                pDesc->path->parent = SPLIT_0;
                ++pDesc->path->len; ++pDesc->path->indent;

                ++pDesc->pathLength;

            }
            break;

            case LPM_NODE_SKIP: {
                uint8_t skipLen   = pDesc->path->node->tag.skipLen + 1;
                g2p3_lpmAddress_t skipAddr 
                  = pDesc->path->node->data.skip.address;

                switch (pDesc->path->seen) {
                case 0:
                        pDesc->path->addr |= 
                          ((skipAddr) << ((lpm->addrBits)
                                          - (pDesc->path->len + skipLen)));
                        pDesc->path->len += skipLen;

                        *(pDesc->path + 1) = *pDesc->path;
                        ++pDesc->path->seen;
                        ++pDesc->path;

                        /* traverse down  */
                        pDesc->path->parent = NON_SPLIT;
                        pDesc->path->node = pDesc->path->node->data.skip.child;
                        ++pDesc->path->indent;
                        ++pDesc->pathLength;
                        break;
                case 1: --pDesc->pathLength;
                        --pDesc->path;
                        break;
                default: SB_ASSERT(0);
                }
                break;
            } /* case LPM_NODE_SKIP  */

            default: SB_ASSERT(0);
            } /* switch (pDesc->path->node->tag.type)  */
        } /* pDesc->path->node != NULL 0 */
        if(found) break;
    } /* while (pDesc->pathLength > 0) */

    if(!found){
      return SB_LPM_ADDRESS_NOT_FOUND;
    } else {
      return SB_OK;
    }
}

/* Support for Incremental Express Updates aka Express Commit */

/* Initialize work item manager */
void g2p3_work_mgr_init(lpmWorkMgr_p_t workmgr)
{
    int index = 0;

    SB_ASSERT(workmgr);

    workmgr->numWork      = 0;
    workmgr->maxWork      = G2P3_MAX_DELTA_UPDATE_COUNT;
    workmgr->numPruneWork = 0;
    workmgr->delPruneWork.nodeCached = 0;
    
    for(index=0; index < G2P3_MAX_PFX_LEN + 1; index++) {
        DQ_INIT(&workmgr->workList[index]);
    }
}

/* is fast update enabled */
sbBool_t g2p3_is_fast_update(lpmWorkMgr_p_t workmgr)
{
  if((workmgr->numWork > 0) && \
     (workmgr->numWork <= workmgr->maxWork)) {
    return TRUE;
  } else {
    return FALSE;
  }
}

lpmWork_p_t g2p3_work_alloc(lpmWorkMgr_p_t workmgr)
{
    lpmWork_p_t workptr = NULL;
   
    if((workmgr->maxWork) && 
       (workmgr->numWork < workmgr->maxWork)) {
      
        SB_MEMSET(&workmgr->work[workmgr->numWork], 0, sizeof(lpmWork_t));

        workptr = &workmgr->work[workmgr->numWork++];

        /* clear the delete prune work item */
        SB_MEMSET(&workmgr->delPruneWork, 0, sizeof(lpmWork_t));        
    }
    return workptr;
}

void g2p3_work_collapse(lpmWorkMgr_p_t workmgr, 
                        lpmWork_p_t workptr)
{
    if(workptr){ 
      SB_ASSERT(workmgr);
      workmgr->numWork--;
    }
}

/* enqueues work item to update queue */
sbStatus_t g2p3_work_enqueue(lpmWorkMgr_p_t workmgr,
                             lpmWork_p_t work)
{
    SB_ASSERT(work);
    SB_ASSERT(workmgr);

    /* insert the work item to the list based on the prefix length */
    DQ_INSERT_HEAD(&workmgr->workList[work->pfxLen], &work->listNode);

    /* for multiple delta update support, form array of linked list
     * based on prefix length */
    return SB_OK;
}

/* dequeues work item */
lpmWork_p_t
g2p3_work_dequeue(lpmWorkMgr_p_t workmgr)
{
    int index=0;
    lpmWork_p_t work = NULL;

    SB_ASSERT(workmgr);
    
    if(workmgr->numWork > 0) {
    
      for(index=0; index < G2P3_MAX_PFX_LEN+1; index++) {
          
          if(!DQ_EMPTY(&workmgr->workList[index])){
              DQ_REMOVE_HEAD(&workmgr->workList[index], work);
              --workmgr->numWork;
              break;
          } 
      }
    }
    return work;
}

void g2p3_lpm_prune_work_enqueue(lpmWorkMgr_p_t workmgr, 
                                 lpmWork_p_t work, 
                                 lpmNode_p_t newnode)
{
    lpmNode_p_t *deadnode;
    
    if(work){       
        SB_ASSERT(newnode);
        SB_ASSERT(workmgr);
        SB_ASSERT(workmgr->numPruneWork < G2P3_MAX_SINGLETON_SLICES);
        
        SB_ASSERT(newnode);                                
        SB_ASSERT((newnode)->tag.type  == LPM_NODE_TABLE);                         
        SB_ASSERT((newnode)->tag.state == LPM_NODE_STATE_NEW);     

        deadnode = &newnode->data.table.child;
        SB_ASSERT((*deadnode)->tag.type  == LPM_NODE_TABLE);                         
        SB_ASSERT((*deadnode)->tag.state == LPM_NODE_STATE_DEAD);                    
                                                           
        /* point the work to this node */                  
        workmgr->pruneWork[workmgr->numPruneWork].node   = *deadnode;
        workmgr->pruneWork[workmgr->numPruneWork].child  = deadnode;
        workmgr->pruneWork[workmgr->numPruneWork].slices = work->slices-2;
        workmgr->numPruneWork++;
    }
}

lpmPruneWork_p_t g2p3_lpm_prune_work_dequeue(lpmWorkMgr_p_t workmgr)
{
    lpmPruneWork_p_t prune = NULL;
    SB_ASSERT(workmgr);
    
    if(workmgr->numPruneWork > 0){
       workmgr->numPruneWork--;
       prune = &workmgr->pruneWork[workmgr->numPruneWork];
    }
    return prune;  
}

void
g2p3_lpmSingletonSet(g2p3_lpmHandle_t lpm, int flag)
{
    lpm->optSingleton = flag;
}

/*****************************************************************************/
void g2p3_lpm_stats_clear(g2p3_lpmHandle_t lpm) 
{
    lpmIntStats_p_t stats;
    int cIdx, tIdx, uIdx;
    
    SB_ASSERT(lpm);
    if (lpm->busy) return;
    stats = &lpm->stats;

    for(uIdx = 0; uIdx < G2P3_LPM_STATS_UPD_MAX; uIdx++) {
      for(cIdx = 0; cIdx < G2P3_LPM_STATS_TYPE_MAX; cIdx++) {
        for(tIdx = 0; tIdx < G2P3_LPM_STATS_MAX_IDX; tIdx++) {
          stats->counter[uIdx][cIdx][tIdx] = 0;    
        }
      }
    }
}

/*#define LPM_STATS_ROLL_THLD ((1 << (sizeof(uint64_t) * CHAR_BIT))-1)*/
/* Might not be porable across platforms, fixme */
#define LPM_STATS_ROLL_THLD   18446744073709551615ULL
sbStatus_t
g2p3_lpm_stats_update(g2p3_lpmHandle_t lpm, 
                      lpmStatsType_t   type,
                      lpmStatsUpdateType_t uType)
{
    lpmIntStats_p_t stats;
    
    SB_ASSERT(lpm);
    if (lpm->busy) return SB_BUSY_ERR_CODE;
    stats = &lpm->stats;

    if((type >= G2P3_LPM_STATS_TYPE_MAX) ||
       (uType >= G2P3_LPM_STATS_UPD_MAX)){
        return SB_BAD_ARGUMENT_ERR_CODE;
    }
    
    if(stats->counter[uType][type][G2P3_LPM_STATS_IDX] >= \
                                      LPM_STATS_ROLL_THLD) {
        stats->counter[uType][type][G2P3_LPM_STATS_IDX] = 0;    
        stats->counter[uType][type][G2P3_LPM_STATS_ROLL_IDX]++;
    } else {
        stats->counter[uType][type][G2P3_LPM_STATS_IDX]++;
    }
    return SB_OK;
}

sbStatus_t g2p3_lpm_stats_get(g2p3_lpmHandle_t lpm,
                            lpmStatsType_t   type,
                            lpmStatsUpdateType_t uType,
                            sbBool_t         clear,
                            lpmStats_t       *count)
{
    lpmIntStats_p_t stats;
    
    SB_ASSERT(lpm);
    if (lpm->busy) return SB_BUSY_ERR_CODE;
    stats = &lpm->stats;

    if((type >= G2P3_LPM_STATS_TYPE_MAX) ||
       (uType >= G2P3_LPM_STATS_UPD_MAX) ||
       (!count)){
        return SB_BAD_ARGUMENT_ERR_CODE;
    }
   
    count->count = stats->counter[uType][type][G2P3_LPM_STATS_IDX];
    count->rollcount = stats->counter[uType][type][G2P3_LPM_STATS_ROLL_IDX];
    
    if(clear) {
        stats->counter[uType][type][G2P3_LPM_STATS_IDX] = 0;
    }
    return SB_OK;
}

void g2p3_lpm_stats_dump(g2p3_lpmHandle_t lpm,
                         sbBool_t verbose)
{
    lpmIntStats_p_t stats;

    SB_ASSERT(lpm);
    if (lpm->busy) return;
    stats = &lpm->stats;
    
    soc_cm_print("\n Number of Route Add [%llu] Rollover [%llu]",
                    stats->counter[G2P3_LPM_STATS_UPD_SUCCESS]\
                    [G2P3_LPM_STATS_TYPE_ADD][G2P3_LPM_STATS_IDX],
                    stats->counter[G2P3_LPM_STATS_UPD_SUCCESS]\
                    [G2P3_LPM_STATS_TYPE_ADD][G2P3_LPM_STATS_ROLL_IDX]);

    soc_cm_print("\n Number of Route Delete [%llu] Rollover [%llu]",
                    stats->counter[G2P3_LPM_STATS_UPD_SUCCESS]\
                    [G2P3_LPM_STATS_TYPE_DEL][G2P3_LPM_STATS_IDX],
                    stats->counter[G2P3_LPM_STATS_UPD_SUCCESS]\
                    [G2P3_LPM_STATS_TYPE_DEL][G2P3_LPM_STATS_ROLL_IDX]);
    
    soc_cm_print("\n Number of Route Update [%llu] Rollover [%llu]",
                    stats->counter[G2P3_LPM_STATS_UPD_SUCCESS]\
                    [G2P3_LPM_STATS_TYPE_UPDATE][G2P3_LPM_STATS_IDX],
                    stats->counter[G2P3_LPM_STATS_UPD_SUCCESS]\
                    [G2P3_LPM_STATS_TYPE_UPDATE][G2P3_LPM_STATS_ROLL_IDX]);

    soc_cm_print("\n Number of Commit [%llu] Rollover [%llu]",
                    stats->counter[G2P3_LPM_STATS_UPD_SUCCESS]\
                    [G2P3_LPM_STATS_TYPE_COMMIT][G2P3_LPM_STATS_IDX],
                    stats->counter[G2P3_LPM_STATS_UPD_SUCCESS]\
                    [G2P3_LPM_STATS_TYPE_COMMIT][G2P3_LPM_STATS_ROLL_IDX]);

    if(verbose) {
      soc_cm_print("\n Number of Route Add Requests [%llu] Rollover [%llu]",
                    stats->counter[G2P3_LPM_STATS_UPD_REQ]\
                    [G2P3_LPM_STATS_TYPE_ADD][G2P3_LPM_STATS_IDX],
                    stats->counter[G2P3_LPM_STATS_UPD_REQ]\
                    [G2P3_LPM_STATS_TYPE_ADD][G2P3_LPM_STATS_ROLL_IDX]);

      soc_cm_print("\n Number of Delete Requests [%llu] Rollover [%llu]",
                      stats->counter[G2P3_LPM_STATS_UPD_REQ]\
                      [G2P3_LPM_STATS_TYPE_DEL][G2P3_LPM_STATS_IDX],
                      stats->counter[G2P3_LPM_STATS_UPD_REQ]\
                      [G2P3_LPM_STATS_TYPE_DEL][G2P3_LPM_STATS_ROLL_IDX]);
      
      soc_cm_print("\n Number of Update Requests [%llu] Rollover [%llu]",
                      stats->counter[G2P3_LPM_STATS_UPD_REQ]\
                      [G2P3_LPM_STATS_TYPE_UPDATE][G2P3_LPM_STATS_IDX],
                      stats->counter[G2P3_LPM_STATS_UPD_REQ]\
                      [G2P3_LPM_STATS_TYPE_UPDATE][G2P3_LPM_STATS_ROLL_IDX]);

      soc_cm_print("\n Number of Commit Requests [%llu] Rollover [%llu]",
                      stats->counter[G2P3_LPM_STATS_UPD_REQ]\
                      [G2P3_LPM_STATS_TYPE_COMMIT][G2P3_LPM_STATS_IDX],
                      stats->counter[G2P3_LPM_STATS_UPD_REQ]\
                      [G2P3_LPM_STATS_TYPE_COMMIT][G2P3_LPM_STATS_ROLL_IDX]);

      soc_cm_print("\n --- Device Memory A dump ----");
      g2p3_lpmDeviceMem_dump(&lpm->deviceMemA);
      soc_cm_print("\n --- Device Memory B dump ----");
      g2p3_lpmDeviceMem_dump(&lpm->deviceMemB);      
    }
    if (verbose>1) {
      g2p3_lpmHostMemUsage(lpm->hostMem);
    }

    soc_cm_print("\n");
                    
}
/*****************************************************************************/
