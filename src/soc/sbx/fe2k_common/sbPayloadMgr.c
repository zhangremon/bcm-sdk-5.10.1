/* -*- mode:c++; c-style:k&r; c-basic-offset:4; indent-tabs-mode: nil; -*- */
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
 * sbPayloadMgr.c: payload memory manager
 *
 * These interfaces should be used to integrate the implemention of the FE
 * driver interface into the target environment.  They should never be used 
 * directly by client code.
 * 
 * $Id: sbPayloadMgr.c 1.8.112.1 Broadcom SDK $
 *
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * Payload Manager
 *
 * This module manages the memory allocation for the Legacy Exact Match 
 * Compiler's (EMLComp) payloads. EMLComp is responsible for managing the 
 * FE's payload memory. It generates alloc and free requests to the 
 * Payload Manager (PM).
 *
 * The PM is based around the sbPayloadMgr data structure which contains
 * 4 linked lists of payload descriptors (sbPayloadDesc). Initially 
 * space for the maximum amount of descriptors is allocated in a block 
 * referenced by sbPayloadMgr->descs. All of the descriptors are 
 * allocated onto the free queue from this block during initialization. 
 *
 * Also during initialization 2 entries are added to the large list. The 
 * first entry's SRAM address (desc->addr) is the start of the FE payload
 * memory, the next (last) entry's SRAM address is the end of the FE payload
 * memory. The entries on the large list are stored in incrementing order of
 * SRAM address. When allocating a payload from a list the process is to 
 * calculate the difference between the 2 elements SRAM addresses - this
 * gives the amount of SRAM available. The initialization of the large list
 * adds the whole SRAM memory space into the list. When allocating SRAM
 * space from this pool the general procedure is to split the available
 * memory into two descriptors, one containing the requested SRAM size
 * is removed from the list and the other containing the remainder is left
 * in the large-free list.
 *
 * The small list contains an array of pointers to linked lists of
 * incrementing sizes. These are initially set to NULL so all linked lists
 * are empty. The purpose of the small list is to make memory management
 * more efficient. The small list is used for reallocating small memory
 * sizes that have been previously freed. When an alloc request is received
 * the small list of the request size is checked to see if a memory size of
 * the exact size already exists. If so it is reused else the SRAM is
 * allocated from the large-free pool.
 *
 * One issue that was seen is that when the memory is filled by adding many
 * entries, all the device memory is used up. The delete function operates by
 * allocating a small memory block, copying most of the contents of a larger
 * block and then deleting the old larger block. This failed since all the
 * memory is used by the adds. The priority scheme resolves this. Adds set pri
 * to 0 and deletes set it to 1. The code now reserves some device memory for
 * use by deletes only to resolve this issue.
 *
 * There is a similar problem when running out of payload descriptors. The
 * delete function may use more descriptors which are all used by the adds. The
 * priority mechanism is also used to work around this problem by keeping a
 * block of descriptors for pri=1 use only.
 *
 * ---------------------------------------------------------------------------*/

#include "sbTypes.h"
#include "sbTypesGlue.h"

#include "sbWrappers.h"
#include <soc/sbx/sbDq.h>

#include <soc/sbx/fe2k_common/sbPaylMgr.h>
#include <soc/sbx/fe2k_common/sbPayloadMgr.h>
#include "sbWrappers.h"
#include <soc/sbx/fe2k_common/sbFe2000DmaMgr.h>


/************************************************************************/


#define SMALLSIZE (32*5)

#define MAX_FRAG 1

#define RESERVED_REM_SIZE (1024) /* SRAM space reserved for delete function */
#define REM_PAYLOAD_DESC (64*1024) /* # payload desc reserved for del func */

struct links {
  struct sbPayloadDesc *lnext;  /* next element */
  struct sbPayloadDesc **lprev; /* address of previous next element */
};

/* Payload memory block description */
struct sbPayloadDesc {
  uint32_t addr;         /* SRAM addr */
  struct links incr;     /* all blocks in increasing addr order */
  struct links free;     /* if free, the block size free list */
};

/* get next desc in a queue */
#define ANEXT(p) ((p)->incr.lnext)


/* remove a desc from a queue's incr link */
#define AREMOVE(p) do {\
  if ((p)->incr.lnext != NULL)\
    (p)->incr.lnext->incr.lprev = (p)->incr.lprev;\
  *(p)->incr.lprev = (p)->incr.lnext;\
} while (/*CONSTCOND*/0)


/* Add a desc into the head of a queue using incr link */
#define AINSERTHEAD(h, p) do {\
  if (((p)->incr.lnext = (h)) != NULL)\
    (h)->incr.lprev = &(p)->incr.lnext;\
  (h) = (p);\
  (p)->incr.lprev = &(h);\
} while (/*CONSTCOND*/0)


/* Add desc p into a queue after desc l using incr link */
#define AINSERTAFTER(l, p) do {\
  if (((p)->incr.lnext = (l)->incr.lnext) != NULL)\
    (l)->incr.lnext->incr.lprev = &(p)->incr.lnext;\
  (l)->incr.lnext = (p);\
  (p)->incr.lprev = &(l)->incr.lnext;\
} while (/*CONSTCOND*/0)


/* walk a queue using the incr link */
#define AFOREACH(var, head)\
for ((var) = (head); (var); (var) = ((var)->incr.lnext))


/* remove desc p from a queue using the free link */
#define FREMOVE(p) do {\
  if ((p)->free.lnext != NULL)\
    (p)->free.lnext->free.lprev = (p)->free.lprev;\
  *(p)->free.lprev = (p)->free.lnext;\
} while (/*CONSTCOND*/0)


/* Add a desc into the head of a queue using free link */
#define FINSERTHEAD(h, p) do {\
  if (((p)->free.lnext = (h)) != NULL)\
    (h)->free.lprev = &(p)->free.lnext;\
  (h) = (p);\
  (p)->free.lprev = &(h);\
} while (/*CONSTCOND*/0)


/* walk a queue using the free link */
#define FFOREACH(var, head)\
for ((var) = (head); (var); (var) = ((var)->free.lnext))


/* calc the max available SRAM block size */
#define BLOCK_SIZE(p) (ANEXT(p)->addr - (p)->addr)

/* is the given address contained in the payload descriptor */
#define BLOCK_ADDRINRANGE(p, addr_) \
   (((p)->addr <= (addr_)) && ((addr_) <= ANEXT(p)->addr))

/* test a desc to check if it is on a free list*/
#define BLOCK_ISFREE(p) ((p)->free.lprev != NULL)

/* set a descriptor to 'not free' */
#define BLOCK_SETUSED(p) ((p)->free.lprev = NULL)

/* find the offset if the incr.lnext element in sbPayloadDesc */
#define BLOCK_OFFS ((uint)&((struct sbPayloadDesc *)0)->incr.lnext)

/* find the previous desc to desc p*/
#define BLOCK_PREV(pm, p) ((pm)->all == p ? NULL : \
  (struct sbPayloadDesc *)((char *)(p)->incr.lprev - BLOCK_OFFS))


/* Memory manager private data. */
struct sbPayloadMgr {
  struct sbPayloadDesc *small[SMALLSIZE]; /* small sizes free lists */
  struct sbPayloadDesc *large;            /* free list for large sizes */
  struct sbPayloadDesc *all;              /* all available blocks, 
                                           * free & used */

  struct sbPayloadDesc *freeDesc;         /* unused payload descriptors */
  void *descs;
  uint32_t used;
  uint32_t maxused;
  uint32_t loPriLimit;
  uint32_t reservedAddr;
  uint32_t nbank;
  uint32_t aligned;
  
  sbPayMgrInit_t initParams;
};

/************************************************************************/
/* Local forward declarations */

static struct sbPayloadDesc *
allocSmallPayload (struct sbPayloadMgr *pm, uint sz, uint pool,
                  uint32_t pri, sbStatus_t *errP);

static struct sbPayloadDesc *
allocLargePayload (struct sbPayloadMgr *pm, uint sz, 
                   uint32_t pri, sbStatus_t *errP);
static int 
allocTail (struct sbPayloadMgr *pm,
           struct sbPayloadDesc *p, uint sz, 
           uint32_t pri, sbStatus_t *errP);

static struct sbPayloadDesc *
allocPayloadDesc (struct sbPayloadMgr *pm, uint32_t hiPri);

static void
freePayloadDesc (struct sbPayloadMgr *pm, struct sbPayloadDesc *p);

/************************************************************************/


/* -----------------------------------------------------------------------------
 * Remove a payload descriptor from the head of the free list and return it.
 * This function manages descriptors, it does not handle the SRAM contents.
 *
 * A number of descriptors are reserved for use by the remove function. If an
 * add function calls this routine (hiPri=0) then a NULL desc may be returned
 * even if there are free descriptors available
 *
 * returns PayloadDesc
 *         Null - no desc avail
 * ---------------------------------------------------------------------------*/
static struct sbPayloadDesc *
allocPayloadDesc(struct sbPayloadMgr *pm, uint32_t hiPri)
{
  struct sbPayloadDesc *p;

  p = pm->freeDesc;
  if (!hiPri && (pm->used >= pm->loPriLimit))
    return NULL;
  
  if (p) {
    AREMOVE(p);
    pm->used++;
    if (pm->used > pm->maxused)
      pm->maxused = pm->used;
  }
  else
    p = NULL;

  return p;
}

/* -----------------------------------------------------------------------------
 * Add a payload descriptor back on the free list
 * ---------------------------------------------------------------------------*/
static void
freePayloadDesc(struct sbPayloadMgr *pm, struct sbPayloadDesc *p)
{
  pm->used--;
  AINSERTHEAD(pm->freeDesc, p);
}


/* -----------------------------------------------------------------------------
 * Initialize the payload manager module
 *
 * allocate memory for the main payload structure (fe->payloadMgr) and the desc
 * space (payloadMgr->desc). Initialize the payloadMgr->small,
 * payloadMgr->large, and payloadMgr->all linked lists.
 *
 * NOTE : the assumption being made througout the entire file is that
 * payload manager is handed 2 contiguous chunks of memory (in 2 separate 
 * memory banks) at the init time. Each chunk starts at address
 * fe->iuParams.payloadStart and extends for fe->iuParams.payloadSize
 * words. Payload manager uses additional trick - it multiplies both
 * start address and size by 2 and pretends to be dealing with 1 
 * contiguous chunk of memory only. Just before returning any
 * address back to EML compiler, it divides it by 2.
 *
 * ---------------------------------------------------------------------------*/
sbStatus_t
sbPayloadMgrInit(sbPaylMgrCtxt_t *pMgrHdl,
                 sbPayMgrInit_t *pInitParams)
{
  uint32_t maxPayloads, sramStart, sramSize;
  sbStatus_t err;
  struct sbPayloadMgr *pm;
  struct sbPayloadDesc *pds, *p;
  sbPayMgrInit_t *prm;
  void *v;
  uint i;

  *pMgrHdl = NULL;   /* in case we bail out early */

  err = pInitParams->sbmalloc(pInitParams->clientData, SB_ALLOC_INTERNAL,
                              sizeof *pm, &v, NULL);
  if (err)
    return err;
  pm = v;
  SB_MEMSET(pm, 0, sizeof *pm);

  prm = &pm->initParams;
  SB_MEMCPY(prm, pInitParams, sizeof(pm->initParams));

  if (prm->payloadMax == SB_FE2000_PARAMS_DEFAULT32) {
      uint32_t t1bits = prm->table1bits;
      
      SB_ASSERT(t1bits != SB_FE2000_PARAMS_DEFAULT32);
      SB_ASSERT(t1bits != 0);
      
      /* 
       * There can be at most 1 << t1bits level 2 tables.
       * Allowing for a free slot in between each doubles the number.
       * Allowing for Smac & Dmac tables doubles again.
       * plus 1/4 overhead for memory manager
       */
      prm->payloadMax = (1 << t1bits) * 4;
      prm->payloadMax += prm->payloadMax >> 3;

      if (prm->payloadMax > prm->sramSize / 6 && prm->sramSize > 0x100000) {
          prm->payloadMax = prm->sramSize / 6;
      }
  }

  maxPayloads = prm->payloadMax;

  SB_ASSERT(maxPayloads);

  sramSize = prm->sramSize;
  sramStart = prm->sramStart;
  pm->nbank = prm->nbank;
  pm->aligned = prm->aligned;

  pm->descs = NULL;

  err = prm->sbmalloc(prm->clientData, SB_ALLOC_INTERNAL,
                      sizeof(struct sbPayloadDesc) * maxPayloads,
                      &v, NULL);
  if (err)
    return err;
  pds = pm->descs = v;

  /* set number of payloads reserved for deletes */
  if (REM_PAYLOAD_DESC > maxPayloads/5)
    pm->loPriLimit = maxPayloads - (maxPayloads/5);
  else
    pm->loPriLimit = maxPayloads - REM_PAYLOAD_DESC;

  /* Set up all payload descriptors */
  pm->freeDesc = NULL;
  for (i = 0; i < maxPayloads; i++)
    AINSERTHEAD(pm->freeDesc, &pds[i]);

  /* initialize all small free lists */
  for (i = 0; i < SMALLSIZE; i++)
    pm->small[i] = NULL;

  /* initialize large free list */
  pm->large = NULL;

  /* insert a dummy block after the free block (for BLOCK_SIZE) */
  p = allocPayloadDesc(pm, 0);
  SB_ASSERT(p);
  p->addr = sramStart + sramSize;
  BLOCK_SETUSED(p);
  AINSERTHEAD(pm->all, p);

  /* make a large free block for the SRAM to use */
  p = allocPayloadDesc(pm, 0);
  SB_ASSERT(p);
  p->addr = sramStart;
  AINSERTHEAD(pm->all, p);
  FINSERTHEAD(pm->large, p);

  if (RESERVED_REM_SIZE > (sramSize >> 4))
    pm->reservedAddr = sramStart + sramSize - (sramSize >> 4);
  else
    pm->reservedAddr = sramStart + sramSize - RESERVED_REM_SIZE;

  pm->used = 0;

  *pMgrHdl = (sbPaylMgrCtxt_t)pm;
  return SB_OK;
}


/* -----------------------------------------------------------------------------
 * Uninitialize the payload manager module
 *
 * Free up resources used by the payload manager
 * ---------------------------------------------------------------------------*/
sbStatus_t
sbPayloadMgrUninit(sbPaylMgrCtxt_t *pMgrHdl)
{
  sbPayloadMgr_p_t pm = (sbPayloadMgr_p_t)*pMgrHdl;
  sbStatus_t err;
  sbDmaMemoryHandle_t junk;
  sbFree_f_t pfFree;
  uint32_t payloadMax;

  if (!pm) {
    return SB_OK;
  }

  sal_memset(&junk, 0x0, sizeof(sbDmaMemoryHandle_t));
  pfFree = pm->initParams.sbfree;
  payloadMax = pm->initParams.payloadMax;

  SB_LOGV1("max used %d\n", pm->maxused);

  if (pm->descs) {
      err = (*pfFree) (pm->initParams.clientData,
                       SB_ALLOC_INTERNAL,
                       sizeof(struct sbPayloadDesc) * payloadMax,
                       pm->descs, junk);
    pm->descs = NULL;
    if (err)
        return err;
  }
  
  err = (*pfFree) (pm->initParams.clientData, SB_ALLOC_INTERNAL,
                   sizeof *pm, pm, junk);
  *pMgrHdl = NULL;
  return err;
}


/* -----------------------------------------------------------------------------
 * Externally visible function - main alloc API. 
 *
 * Allocate space for a payload entry in the FE's payload SRAM. First check
 * small linked list to see if any buffers of the correct size have been 
 * previously freed. If not then allocate the memory from the large list and
 * modify it to reflect the reduced smpace available. If there is not enough
 * space available in the large list then start searching the small lists for 
 * slightly bigger buffers that can be used.
 *
 * returns sbPayloadHandle_t if buffer can be allocated
 *         NULL if no free buffer space
 * ---------------------------------------------------------------------------*/
sbPayloadHandle_t
sbPayloadAlloc (sbPayloadMgr_p_t pm, uint sz,
                 sbBool_t hiPri, sbStatus_t *errP)
{
  struct sbPayloadDesc *p;
  uint i;

  SB_ASSERT(sz > 0);

  /* Payload is laid across multiple banks */
  sz = DIV_RU(sz, pm->nbank);

  if (sz < SMALLSIZE) {
    p = allocSmallPayload(pm, sz, sz, hiPri, errP);
    if (p) {
        return p;
    }
  }

  p = allocLargePayload(pm, sz, hiPri, errP);
  if (p) {
      return p;
  }

  SB_LOGV1("sbPayloadAlloc: desperate for %u\n", sz);
  for (i = sz; i < SMALLSIZE; i++) {
    p = allocSmallPayload(pm, sz, i, hiPri, errP);
    if (p) {
      return p;
    }
  }

  if (*errP != SB_MAC_NO_PAYLOAD_DESC)
    *errP = SB_MAC_NO_MEM;

  return NULL;
}


/* -----------------------------------------------------------------------------
 * Externally visible function - recover function
 *
 * Reserve space for a payload entry in the FE's payload SRAM at the given
 * address.  Only the large linked list is checked because this routine is
 * only used at time zero for warm reboot, when the small list is empty.
 * The larged linked list is traversed until the given address is contained 
 * within a payload descriptor.  The descriptor is then split based on four
 * cases and the reserved payload descriptor is returned.  The four cases
 * detailed below; anything else is an error.  
 *
 * returns sbPayloadHandle_t if buffer can be allocated/reserved
 *         NULL if no free buffer space
 * ---------------------------------------------------------------------------*/
sbPayloadHandle_t
sbPayloadRecover (sbPayloadMgr_p_t pm, uint32 addr,
                  uint sz, sbStatus_t *errP)
{ 
    struct sbPayloadDesc *p, *pr, *pj[2];
    int bs, idx;

    /* Payload is laid across multiple banks */
    sz = DIV_RU(sz, pm->nbank);

    FFOREACH(p, pm->large) {
        if (!BLOCK_ADDRINRANGE(p, addr)) {
            continue;
        }
        
        /* Will the requested size extend beyond the free block? */
        bs = BLOCK_SIZE(p);
        if ((sz > bs) || ((addr + sz) > ANEXT(p)->addr)) {
            /* fatal error during a recovery */
            *errP = SB_MAC_NO_PAYLOAD_DESC;
            return 0;
        }

        /* The given address lives in this payload descriptor; one of
         * four possible splits are handled here.
         *
         *    [*x*] = payload descriptor returned
         *    [addr] = payload descrriptor with addr
         *    for a given payload descriptor, p:
         *       p->[start]->[end==p->next->addr]
         *
         *  Split the payload descriptor when the requested block:
         *  (1) starts at the payload descriptor.
         *          p->[*start*]->[start+size]->[end]
         *  (2) starts after the payload descriptor, but consumes all space
         *      up to the next descriptor.
         *          p->[start]->[*addr*]->[end]
         *  (3) is contained within the payload descriptor, but does not 
         *      include either edge.
         *          p->[start]->[*addr*]->[addr+size]->[end]
         *  (4) consumes the entire payload descriptor
         *          p->[*start*]->[end]; no split required.
         *
         */
        if (addr == p->addr) {

            pr = p;
            if (sz != bs) {

                /* case 1 */
                pj[0] = allocPayloadDesc(pm, 0);
                if (!pj[0]) { 
                    *errP = SB_MAC_NO_PAYLOAD_DESC;
                    return 0;
                }
                AINSERTAFTER(p, pj[0]);
                FINSERTHEAD(pm->large, pj[0]);
                pj[0]->addr = p->addr + sz;
            }
            /* else case 4, nothing to update */
 
        } else if ((addr + sz) == ANEXT(p)->addr) {

            /* case 2 */                
            pj[0] = allocPayloadDesc(pm, 0);
            if (!pj[0]) { 
                *errP = SB_MAC_NO_PAYLOAD_DESC;
                return 0;
            }
            AINSERTAFTER(p, pj[0]);
            FINSERTHEAD(pm->large, pj[0]);
                
            pj[0]->addr = addr;
            pr = pj[0];
                
        } else if (p->addr < addr) {

            /* case 3 */                
            for (idx=0; idx < 2; idx++) {
                pj[idx] = allocPayloadDesc(pm, 0);
                if (!pj[idx]) { 
                    *errP = SB_MAC_NO_PAYLOAD_DESC;
                    return 0;
                }                    
                AINSERTAFTER(p, pj[idx]);
                FINSERTHEAD(pm->large, pj[idx]);
            }
                
            pj[1]->addr = addr;
            pj[0]->addr = addr + sz;
                
            pr = pj[1];
                
        } else {
            /* unhandled case - shd not happen */
            assert(0);
            *errP = SB_MAC_NO_PAYLOAD_DESC;
            return NULL;
        }

        /* Found a payload descriptor for the given address.
         * Remove it from teh free list, and mark it as used 
         */
        FREMOVE(pr);
        BLOCK_SETUSED(pr);
            
        *errP = SB_OK;
        return pr;
    } /* FFOREACH(p, pm->large) */

    *errP = SB_MAC_NO_PAYLOAD_DESC;
    return NULL;
}


/* -----------------------------------------------------------------------------
 * Externally visible function - Free a previously allocated payload
 *
 * Add a freed payload descriptor and its associated SRAM back into the free
 * pool. Check to see if the previous or following desc also contain free
 * memory and if so then coalesce the memory regions into a single large
 * block.
 * ---------------------------------------------------------------------------*/
void
sbPayloadFree(sbPayloadMgr_p_t pm, sbPayloadHandle_t p)
{
  struct sbPayloadDesc *q;
  uint sz;

  /* bandaid, do not free payloads already freed. */
  if (p == 0)
    return;

  /* check if we can coalesce with previous block */
  q = BLOCK_PREV(pm, p);
  if (q && BLOCK_ISFREE(q)) {
    /* remove previous from free list */
    FREMOVE(q);
    /* coalesce by removing p */
    AREMOVE(p);
    freePayloadDesc(pm, p);
    p = q;
  }

  /* check if we can coalesce with next block */
  q = ANEXT(p);
  if (BLOCK_ISFREE(q)) {
    /* remove next from free list */
    FREMOVE(q);
    /* coalesce by removing q */
    AREMOVE(q);
    freePayloadDesc(pm, q);
  }

  /* p is the now the coalesced free block */
  sz = BLOCK_SIZE(p);
  if (sz < SMALLSIZE) {
    FINSERTHEAD(pm->small[sz], p);
  }
  else
    FINSERTHEAD(pm->large, p);
}

/* -----------------------------------------------------------------------------
 * Externally visible function
 *
 * returns if the payload manager is using relative addressing
 * ---------------------------------------------------------------------------*/
uint32_t
sbPayloadAligned (sbPaylMgrCtxt_t pM)
{
  sbPayloadMgr_p_t pm = (sbPayloadMgr_p_t) pM;
  return (pm->aligned);
}

/* -----------------------------------------------------------------------------
 * Externally visible function
 *
 * Returns the FE's SRAM address for the payload in the specified descriptor
 * ---------------------------------------------------------------------------*/
uint32_t
sbPayloadGetAddr (sbPaylMgrCtxt_t pM,
                   sbPHandle_t p)
{
  struct sbPayloadDesc *pD = (struct sbPayloadDesc *)p;
  SB_ASSERT(pD);
  return pD->addr;
}


/* -----------------------------------------------------------------------------
 * Externally visible function
 *
 * Return the max amount of space available to allocate in the specified
 * linked list.
 * ---------------------------------------------------------------------------*/
uint32_t
sbPayloadSize (sbPayloadMgr_p_t pm, sbPayloadHandle_t p)
{
  SB_ASSERT(p);
  return BLOCK_SIZE(p);
}


/* -----------------------------------------------------------------------------
 * Allocate a buffer from the small-free linked list.
 * sz - size of requested allocation
 * pool - which buffer size to use
 * 
 * A small amount of high memory space is reserved for delete functions (pri=1)
 *
 * returns *Desc if buffer can be allocated
 *         NULL if no buffer exists on specified list
 * ---------------------------------------------------------------------------*/
static struct sbPayloadDesc *
allocSmallPayload (struct sbPayloadMgr *pm, uint sz, uint pool, 
                   uint32_t  pri, sbStatus_t *errP)
{
  struct sbPayloadDesc *p;

  p = pm->small[pool];
  if (!p)
    return NULL;

#if 0
  /* What is the purpose of this, when reserved Addr
     is meant for deletes? */

  /* Do not let the low priority request eat the last member
   * of the list - might need it for deletes.
   */
  if ((pri == 0) && (!p->free.lnext))
    return NULL;
#endif

  if ((pri == 0) && (p->addr >= pm->reservedAddr)) {
    while (p && (p->addr >= pm->reservedAddr)) {
      p = p->free.lnext;
    }
  }

  if (!p)
    return NULL;

  if (!allocTail(pm, p, sz, pri, errP)) {
    if (*errP != SB_MAC_NO_PAYLOAD_DESC)
      FINSERTHEAD(pm->small[pool], p);
    p = NULL;
  }
  return p;
}


/* -----------------------------------------------------------------------------
 * Allocate a buffer from the large-free linked list
 * Walk the large linked list and search for the first descriptor that has a
 * buffer allocation >= to the requested size. If found then call allocTail to
 * manipulate the desc so it contains the correct size and returns the extra
 * SRAM space back to the large or one of the small lists.
 *
 * A small amount of high memory space is reserved for delete functions (pri=1)
 *
 * returns *Desc if buffer can be allocated
 *         NULL if no buffer exists on specified list
 * ---------------------------------------------------------------------------*/
static struct sbPayloadDesc *
allocLargePayload (struct sbPayloadMgr *pm, uint sz, uint32_t  pri,
                   sbStatus_t *errP)
{
  struct sbPayloadDesc *p;

  if (pri == 0) {
    /* for low priority requests - ensure that we will have
     * some space left (ought to calc. size) for deletes.
     */
    FFOREACH(p, pm->large) {
      if (BLOCK_SIZE(p) - sz > 256) goto OK;
    }
    return NULL;
  }
                                                                                
OK:
  FFOREACH(p, pm->large) {
    if (BLOCK_SIZE(p) >= sz) {
      if ((pri == 0) && (p->addr >= pm->reservedAddr)) {
        /* if adding a payload (pri == 0) then don't
         * use the reserved space */
        continue;
      }
      else if (!allocTail(pm, p, sz, pri, errP)) {
        if (*errP != SB_MAC_NO_PAYLOAD_DESC)
          FINSERTHEAD(pm->large, p);
        p = NULL;
      }
      return p;
    }
  }
  return NULL;
}


/* -----------------------------------------------------------------------------
 *  This function accepts a payload desc and a size and if the size matches the
 * SRAM size of the desc then removes it from the free queue.
 * In the general case where the block size is greater than the requested
 * size, the block is split into two. A block of the requested size is returned
 * and a new block containing the remainder of the memory is inserted back into
 * the free queue.
 *
 * returns 1 if a valid block is allocated
 *         0 if cannot allocate memory (*errP specifies error)
 *            *errP = SB_OK - no device memory available
 *            *errP = SB_MAC_NO_PAYLOAD_DESC - no free desc
 * ---------------------------------------------------------------------------*/
static int
allocTail (struct sbPayloadMgr *pm, struct sbPayloadDesc *p, 
           uint sz, uint32_t pri, sbStatus_t *errP)
{
  uint bs = BLOCK_SIZE(p);
  struct sbPayloadDesc *np;

  /* if the fragmentation is acceptable, just return the block */
  if (bs - sz <= MAX_FRAG) {
    *errP = SB_OK;
    FREMOVE(p);
    BLOCK_SETUSED(p);
    return 1;
  }

  /* split the block */
  /* get descriptor for second part */
  np = allocPayloadDesc(pm, pri);
  if (!np) {
    *errP = SB_MAC_NO_PAYLOAD_DESC;
    return 0;
  }

  *errP = SB_OK;

  /* take it off free list */
  FREMOVE(p);
  BLOCK_SETUSED(p);

  np->addr = p->addr + sz;
  /* insert in list of all blocks */
  AINSERTAFTER(p, np);
  /* and insert leftover in free list */
  sbPayloadFree(pm, np);

  return 1;
}

#define SBFEPMDEBUG 1
#ifdef SBFEPMDEBUG
/* -----------------------------------------------------------------------------
 * Dump payload data structures - debug utility
 * ---------------------------------------------------------------------------*/
void
dumpPayloadMgr (struct sbPayloadMgr *pm, int full)
{
  struct sbPayloadDesc *p;
  uint i, totfree;

  if (full) {
    AFOREACH(p, pm->all) {
      if (!ANEXT(p))
        break;
      SB_LOG("0x%05x: 0x%05x %s\n", p->addr, BLOCK_SIZE(p),
             BLOCK_ISFREE(p) ? "free" : "used");
    }
  }

  totfree = 0;

  for (i = 0; i < SMALLSIZE; i++) {
    if (pm->small[i]) {
      SB_LOG("free list %d:", i);
      FFOREACH(p, pm->small[i]) {
        SB_LOG(" 0x%05x", p->addr);
        totfree += i;
      }
      SB_LOG("\n");
    }
  }

  SB_LOG("large free list: ");
  FFOREACH(p, pm->large) {
    SB_LOG(" 0x%05x#0x%x", p->addr, BLOCK_SIZE(p));
    totfree += BLOCK_SIZE(p);
  }

  SB_LOG("\n");
  SB_LOG("%u total free\n", totfree);

}
#endif
