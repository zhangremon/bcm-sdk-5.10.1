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

  $Id: g2p3_lpm_device_mem.c 1.7.42.1 Broadcom SDK $

 *****************************************************************************/

#include "sbTypesGlue.h"

#include "g2p3_lpm_device_mem.h"
#include "g2p3_lpm_device_mem_internal.h"
#include "sbWrappers.h"
#include "soc/cm.h"

/* ****************************************************************
 * This file contains the Lpm Device Memory Manager
 */

/* ================================================================
 * LPM table memory (e.g., for IPv4 source or dest)
 *
 * The memory map looks like this:
 *    +-----...------+---...---+---....---+
 *    | POWER2BLOCKS |  FREE   | PAYLOADS |
 *    +-----...------+---...---+---....---+
 *    A              B         C          D
 *    <---------- managed area ----------->
 *
 * A:            Start of area for blocks of size 2^k
 * B: freeIx     Just past end of 2^k-size blocks (grows upward)
 * C: limitIx    Just below payloads (grows downward)
 * D:            End of payloads (A + memSizeW - 1)
 *
 * The LPM data structure consists of trie chunks forming a tree,
 * with payloads at the leaves.
 *
 * The trie chunks are variable-sized but are all powers of 2 (# words)
 * and are allocated in the area [A..B], using an allocator that exploits
 * the 2^n size property.  The area [A..B] grows upward as necessary.
 * by incrementing freeIx.
 *
 * Payloads are all of fixed size, typically just a few words.
 * - If payload size is also a power of 2, then we just use the
 *   2^n allocator and allocate them in area [A..B].
 * - If payload size is not a power of 2, then we allocate them
 *   in area [C..D] using a freelist, growing limitIx
 *   downward as necessary.
 * The choice between the above two methods is made the first
 * time we see an alloc request for a size that is not a power
 * of 2 (we assume this is the first payload allocated, and use
 * its size as the fixed payload size).
 *
 * If freeIx crosses limitIx, we're out of memory.
 * Note that at this point there may be free chunks in
 * area [A..B] and free payloads in area [C..D].
 * This can't be avoided unless we relocate alread-allocated
 * chunks/payloads, which is a much more complex operation.
 */


/* ================================================================
 * Some general constants (not specific to this application)
 */
#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif

#undef MAX
#define MAX(a,b)  (((a) > (b)) ? (a) : (b))
#undef MIN
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))

/* ================================================================
 * Support for messages during debugging.
 * DBGPR's disappear when LPM_DEVICE_MEM_DEBUG == 0
 */
#undef LPM_DEVICE_MEM_TRACE

#if LPM_DEVICE_MEM_DEBUG

#define DBGPR(x)  SB_LOG(x)

#else

#define DBGPR(x)

#endif

/* ================================================================
 * Abort codes.
 * None of these aborts should EVER be encountered, period.
 * Encountering an abort is definitely a mem manager bug.
 * Defining these aborts is a defensive manouvre: if we ever encounter
 * an abort, the debugger can examine the variable "lpmDeviceMemAbortCode"
 * to see the cause of the abort.
 */

typedef enum AbortCode {
    ABORT_NONE                       = 0,
    ABORT_HANDLE_IS_NULL,
    ABORT_ALLOC_SIZE_IS_NOT_POSITIVE,
    ABORT_ALLOC_SIZE_NOT_POWER_OF_2,
    ABORT_ALLOC_SIZE_TOO_BIG,
    ABORT_ALLOC_INCONSISTENT_PAYLOAD_SIZE
} AbortCode;

static AbortCode  lpmDeviceMemAbortCode = ABORT_NONE;

#define ABORT(n)  { lpmDeviceMemAbortCode = (n); SB_ASSERT(n==ABORT_NONE); }

/* ================================================================
 * XXXXXX This section is to appease netbsd's C compiler
 * (needs forward declarations for all 'static' procedures)
 */

static
int
isPowerOf2 (int x);

static
int
sblog2 (int x);

static
sbStatus_t
deallocMemMgrDataStructures (g2p3_lpmDeviceMem_t  *dmp);

static
void
rmFromSized (g2p3_lpmDeviceMem_t  *dmp,  int ix, int log2_nwords);

static
void
populateFreelist (g2p3_lpmDeviceMem_t  *dmp,  int log2_nwords);

static
int
allocBlock (g2p3_lpmDeviceMem_t  *dmp,  int nwords);

static
void
freeBlock (g2p3_lpmDeviceMem_t  *dmp, int blkIx, int  nwords);

static
int
allocPayload (g2p3_lpmDeviceMem_t  *dmp);

static
void
freePayload (g2p3_lpmDeviceMem_t  *dmp,  int ix);

/* XXXXXX End forward declarations to appease netbsd's C compiler */

/* ================================================================
 * Utils for power-of-2 ints
 */

static
int
isPowerOf2 (int x)
{
#if 0  
    /* specifically force payloads (sizes 2 and 4 currently)
     * as non-power-of-2, so that they come from they own pool.
     */
    return  ((x > 4) && 
             ((x & (x-1)) == 0));
#else
    /* For singleton, since payload is not 2 words, allocate sigleton
     * table from buddy allocator. If payload becomes 2 words, its efficient
     * to allocate in payload area. But for now allocating in buddy is the best
     * way to go
     */
    return ((x & (x-1)) == 0);
#endif    
} /* isPowerOf2() */

static
int
sblog2 (int x)
{
    int  l;

    SB_ASSERT (x > 0);
    SB_ASSERT ((x & (x-1)) == 0);    /* ---- x is a power of 2 */

    for (l = 0; ((x & 0x1) == 0); l++)
        x = (x >> 1);
    return l;
} /* sblog2() */

/* ================================================================
 * Memory Manager for blocks of size 2^k ('buddy' strategy)
 *
 * Invariants and principles:
 *
 * - a 2^k sized block is always aligned on a 2^k boundary ("properly aligned").
 *
 * - Two adjacent 2^k sized blocks, on an even-numbered 2^k boundary
 *   and the next odd-numbered 2^k boundary, are called 'buddies'
 *   Two such free buddies can be coalesed into a single 2^(k+1) block
 *   which will therefore also be properly aligned.
 *
 *   Note: adjacent 2^k sized blocks on an odd-numbered 2^k boundary
 *         and a following even-numbered boundary are NOT buddies.
 *
 * - freeIx is a freespace pointer; initially freeIx = 0.
 *   It may grow (increment) as a result of an allocation, if
 *   the allocation can't be done from a sized freelist (see below).
 *   Note: since blocks must be properly aligned, an allocation
 *   may result in freeIx incrementing past a free gap followed
 *   by the newly allocated block.  The skipped gap must be chunked
 *   into blocks and put onto freelists.
 *
 * - After an alloc/free, there is never a free block adjacent to freeIx.
 *   When a block adjacent to freeIx is freed, instead of putting it on
 *   a freelist, freeIx is decremented to the start of the block.
 *   This may require freeIx to be decremented further, since a free
 *   block may now be adjacent to freeIx, and so on.
 *
 * - We maintain several 'sized freelists'.  All blocks on freelist j
 *   are of size 2^j.
 *
 * - To allocate a block of size 2^j,
 *   - Try to get it off freelist j.  If freelist j is empty, try to
 *     populate it by taking a block off freelist j+1 and splitting it
 *     into 2 blocks for freelist j
 *     (and so on: if freelist j+1 is empty, populate it from j+2, ...)
 *   - If can't allocate off freelist, allocate in freespace by
 *     incrementing freeIx.
 *
 * - To free a block of size 2^j,
 *   - First coalesce it with its free 2^j buddy, if any
 *     (and so on: coalesce this with its free 2^(j+1) buddy, ...).
 *   - Finally, coalesce it with freespace, if adjacent to freeIx,
 *     else put it onto the appropriate freelist.
 */

/* ================================================================
 * Help functions for API
 */

/* ----------------
 * deallocMemMgrDataStructures()
 * Deallocate any data structures allocated by LpmDeviceMemInit()
 */

static
sbStatus_t
deallocMemMgrDataStructures (g2p3_lpmDeviceMem_t  *dmp)
{
    sbStatus_t           err;

    /* ---- Deallocate infoArr */
    if (dmp->infoArr != NULL) {
        DBGPR (("uninitDevMemMgr: freeing infoArr of 0x%x uint32_t's\n",
                dmp->memSizeW));
        err = g2p3_lpmHostMemFree (dmp->hostMemHandle, dmp->infoArr,
                                       dmp->memSizeW * sizeof (uint32_t));
        if (err != SB_OK) return err;
    }
    /* ---- Deallocate infoArr */
    if(!dmp->ipv6)
    {
        if (dmp->reverseLinkInfoArr != NULL) {
            DBGPR (("uninitDevMemMgr: freeing reverseLinkInfoArr of 0x%x uint32_t's\n",
                    dmp->memSizeW));
            err = g2p3_lpmHostMemFree (dmp->hostMemHandle, dmp->reverseLinkInfoArr,
                                           dmp->memSizeW * sizeof (uint32_t));
            if (err != SB_OK) return err;
        }
    }

    /* ---- Deallocate the mem for sized freelists */
    if (dmp->sizedFreelists != NULL) {
        DBGPR (("uninitDevMemMgr: freeing 0x%x ints for sizedFreelists\n",
                (LOG2_MAX_BLOCK_SIZE + 1)));
        err = g2p3_lpmHostMemFree (dmp->hostMemHandle, dmp->sizedFreelists,
                                       (LOG2_MAX_BLOCK_SIZE + 1) * sizeof (int));
        if (err != SB_OK) return err;
    }

    /* ---- Deallocate the mem for sizeHist */
    if (dmp->sizeHist != NULL) {
        DBGPR (("uninitDevMemMgr: freeing 0x%x ints for sizeHist\n",
                (LOG2_MAX_BLOCK_SIZE + 1)));
        err = g2p3_lpmHostMemFree (dmp->hostMemHandle, dmp->sizeHist,
                                       (LOG2_MAX_BLOCK_SIZE + 1) * sizeof (int));
        if (err != SB_OK) return err;
    }

    /* ---- Dealloacte the mem descr itself */
    DBGPR (("uninitDevMemMgr: freeing %d bytes for g2p3_lpmDeviceMem_t\n",
            sizeof(g2p3_lpmDeviceMem_t)));
    err = g2p3_lpmHostMemFree (dmp->hostMemHandle, dmp,
                                   sizeof (g2p3_lpmDeviceMem_t));
    if (err != SB_OK) return err;

    return SB_OK;
} /* deallocMemMgrDataStructures() */

/* ----------------
 * rmFromSized()
 * removes a block from a sized freelist
 */
static
void
rmFromSized (g2p3_lpmDeviceMem_t  *dmp,  int ix, int log2_nwords)
{
    int  prevIx = NULLIX;
    int  nextIx = NULLIX;
    int ix1; 

    if(!dmp->ipv6)
    {

        prevIx = revListSizedPrev(dmp, ix);
        nextIx = sizedNext(dmp, ix);

        if(prevIx == NULLIX)
        {
            dmp->sizedFreelists[log2_nwords] = nextIx;
        }
        else
        {
            setSizedNext(dmp, prevIx, nextIx);
        }

        if(nextIx != NULLIX)
        {
            revListSetSizedPrev(dmp, nextIx, prevIx);
        }

    }
    else
    {
        ix1    = dmp->sizedFreelists [log2_nwords];
        while (ix1 != ix) {
            SB_ASSERT (! isNULLIX (ix1));
            prevIx = ix1;
            ix1    = sizedNext (dmp, ix1);
        }

        if (isNULLIX (prevIx))
            dmp->sizedFreelists [log2_nwords] = sizedNext (dmp, ix);
        else
            setSizedNext (dmp, prevIx, sizedNext (dmp, ix));

    }
} /* rmFromSized() */



/* ----------------
 * populateFreelist()
 * If the freelist at log2_nwords is empty,
 * try to populate it by splitting larger blocks (if avail)
 * Note: may fail, i.e., freelist remains empty
 */

static
void
populateFreelist (g2p3_lpmDeviceMem_t  *dmp,  int log2_nwords)
{
    int  ix;

    /* ---- If already populated, done (success) */
    if (! isNULLIX(dmp->sizedFreelists [log2_nwords])) return;

    /* ---- If at largest size, done (fail) */
    if (log2_nwords == dmp->log2_max_block_size) return;

    /* ---- Try populating next size */
    populateFreelist (dmp, log2_nwords + 1);

    /* ---- If next size unpopulated, done (fail) */
    ix = dmp->sizedFreelists [log2_nwords + 1];
    if (isNULLIX(ix)) return;

    /* ---- Split block from next size */
    DBGPR (("populateFreelist: splitting block from freelist 0x%x: ",
            log2_nwords + 1));
    DBGPR (("  0x%x  0x%x\n", ix, ix + sbexp2 (log2_nwords)));
    dmp->sizedFreelists [log2_nwords + 1] = sizedNext (dmp, ix);

    if((!dmp->ipv6) && (sizedNext (dmp,ix) != NULLIX))
       revListSetSizedPrev (dmp, sizedNext (dmp,ix), NULLIX);

    setSizedNext (dmp, ix + sbexp2 (log2_nwords), NULLIX);
    setBlockSize (dmp, ix + sbexp2 (log2_nwords), sbexp2 (log2_nwords));

    setSizedNext (dmp, ix, ix + sbexp2 (log2_nwords));
    setBlockSize (dmp, ix, sbexp2 (log2_nwords));
    dmp->sizedFreelists [log2_nwords] = ix;


    if(!dmp->ipv6)
    {
        revListSetBlockSize (dmp, ix + sbexp2 (log2_nwords), sbexp2(log2_nwords));
        revListSetBlockSize (dmp, ix , sbexp2(log2_nwords));
        revListSetSizedPrev (dmp, ix + sbexp2 (log2_nwords), ix);
        revListSetSizedPrev (dmp, ix,  NULLIX);

    }



} /* populateFreelist() */

/* ----------------
 * allocBlock()
 * alloc a block.
 * Returns a word index for the block (NULLIX if can't alloc)
 */

int
allocBlock (g2p3_lpmDeviceMem_t  *dmp,  int nwords)
{
    int  log2_words, ix, n1, l;

    DBGPR (("--> allocBlock: nwords 0x%x\n", nwords));
    log2_words = sblog2 (nwords);

    populateFreelist (dmp, log2_words);
    ix = dmp->sizedFreelists [log2_words];

    /* ---- Freelist has block */
    if (! (isNULLIX (ix))) {
        DBGPR (("    allocBlock: allocating from freelist\n"));
        dmp->sizedFreelists [log2_words] = sizedNext (dmp, ix);
        if((!dmp->ipv6) && (sizedNext (dmp,ix) != NULLIX))
        {
           revListSetSizedPrev (dmp, sizedNext(dmp, ix), NULLIX);
        }

    }

    /* ---- Try to expand into freespace */
    else {
        /* ---- ix (for new block) must be nword-aligned, at or above freeIx */
        ix = dmp->freeIx;
        if ((ix & (nwords - 1)) > 0)
            ix = ((ix + nwords) & (~ (nwords - 1)));

        /* ---- Enough free space */
        if ((ix + nwords - 1) <= dmp->limitIx) {
            DBGPR (("    allocBlock: allocating from freespace at 0x%x\n",
                    ix));
            /* ----------------
             * Chunk up gap, from freeIx..(ix-1), into blocks, and
             * place them on freelists.  The '1' bits in gap size n1
             * specify exactly the required chunk sizes.  Further, if
             * we chunk from smallest to largest, they will be
             * properly aligned (since 'ix' is aligned to log2_nwords)
             */
            n1 = ix - dmp->freeIx;     /* -- gap size */
            l  = 0;                    /* -- lsbGu of n1 is the 2^l bit */
            while (n1 > 0) {
                if (n1 & 0x1) {
                    setSizedNext (dmp, dmp->freeIx, dmp->sizedFreelists[l]);
                    setBlockSize (dmp, dmp->freeIx, sbexp2 (l));
                    dmp->sizedFreelists[l] = dmp->freeIx;

                    if(!dmp->ipv6)
                    {
                       revListSetBlockSize (dmp, dmp->freeIx, sbexp2 (l));

                       if( sizedNext(dmp, dmp->freeIx) != NULLIX)
                           revListSetSizedPrev (dmp, sizedNext(dmp, dmp->freeIx), dmp->freeIx);
                       revListSetSizedPrev (dmp, dmp->freeIx, NULLIX);
                    }

                    dmp->freeIx += sbexp2 (l);
                }
                n1 = n1 >> 1;
                l++;
            }
            SB_ASSERT (dmp->freeIx == ix);

            /* ---- Update freeIx beyond alloc'd block */
            dmp->freeIx   = ix + nwords;
            dmp->minFreeW = MIN (dmp->minFreeW,
                                 dmp->limitIx - dmp->freeIx + 1);
        }

        /* ---- Not enough free space */
        else {
            DBGPR (("    allocBlock: not enough freespace\n"));
            ix = NULLIX;
        }
    }

    if (! isNULLIX (ix)) {
        DBGPR (("<-- allocBlock: 0x%x\n", ix));
        setBusy (dmp, ix);
        setBlockSize (dmp, ix, nwords);

        if(!dmp->ipv6)
        {
            revListSetBusy(dmp, ix);
            revListSetBlockSize(dmp, ix, nwords);
        }


        dmp->sizeHist [log2_words]++;
        dmp->numBlksAllocated++;
        dmp->wordsInUse += nwords;
    }

    return ix;
} /* allocBlock() */

/* ----------------
 * freeBlock()
 * deallocate a block
 */

static
void
freeBlock (g2p3_lpmDeviceMem_t  *dmp, int ix, int  nwords)
{
    int  buddyIx, freeIx, n, log2_words;

    DBGPR (("--> freeBlock ix 0x%x, nwords 0x%x\n", ix, nwords));

    SB_ASSERT (ix >= 0);
    SB_ASSERT (ix < dmp->freeIx);

    log2_words = sblog2 (nwords);
    dmp->numBlksFreed++;
    dmp->wordsInUse -= nwords;
    dmp->sizeHist[log2_words]--;

    /* ---- Repeatedly coalesce with buddy, if any */
    while (nwords < sbexp2 (dmp->log2_max_block_size)) {
        buddyIx = (ix ^ nwords);
        if ((buddyIx >= dmp->freeIx) ||
            (blockSize (dmp, buddyIx) != nwords) ||
            isBusy (dmp, buddyIx))
            break;
        rmFromSized (dmp, buddyIx, sblog2 (nwords));
        nwords = nwords << 1;
        ix = MIN (ix, buddyIx);
    }

    /* ---- If adjacent to freespace, coalesce into freespace */
    if (ix + nwords == dmp->freeIx) {
	DBGPR (("Coalescing with freespace at 0x%x\n", dmp->freeIx));
	dmp->freeIx = ix;

        /* ----
         * There may be a free block adjacent to freeIx.
         * If so, then freeIx is the 'odd' buddy of that block.
         * Coalesce that block into freespace
         */
        while (TRUE) {
            freeIx = dmp->freeIx;
            if (freeIx == 0) break;
            n = (freeIx ^ (freeIx - 1)) & freeIx;
            buddyIx = freeIx - n;
            if ((blockSize (dmp, buddyIx) == n) &&
                (isFree (dmp, buddyIx))) {
                DBGPR (("    Coalescing block at 0x%x + 0x%x words w. freespace at 0x%x\n",
                        buddyIx, n, freeIx));
                rmFromSized (dmp, buddyIx, sblog2 (n));
                dmp->freeIx = buddyIx;
            }
            else
                break;
        }
    }

    /* ---- Not adjacent to freespace; put on freelist */
    else {
	DBGPR (("Adding block at 0x%x to freelist 0x%x\n",
                ix, sblog2 (nwords)));

        setSizedNext (dmp, ix, dmp->sizedFreelists [sblog2(nwords)]);
        setBlockSize (dmp, ix, nwords);
        dmp->sizedFreelists [sblog2(nwords)] = ix;
        if(!dmp->ipv6)
        {
           revListSetBlockSize (dmp, ix, nwords);
           revListSetSizedPrev(dmp, ix, NULLIX);

           if( sizedNext (dmp, ix) != NULLIX)
               revListSetSizedPrev(dmp, sizedNext (dmp, ix) , ix); 
        }


    }
    DBGPR (("<-- freeBlock\n"));
} /* freeBlock() */

/* ----------------
 * allocPayload()
 * Allocates a "payload" object, from the upper end of memory.
 * (when payload size is not a power of 2)
 */

static
int
allocPayload (g2p3_lpmDeviceMem_t  *dmp)
{
    int  ix;

    /* ---- Alloc from freespace (freelist is empty) */
    if (isNULLIX (dmp->payloadFreelist)) {
        if ((dmp->limitIx - dmp->freeIx + 1) >= dmp->payloadSizeW) {
            dmp->limitIx -= dmp->payloadSizeW;
            ix = dmp->limitIx + 1;
        }
        else
            ix = NULLIX;
    }
    /* ---- Alloc from freelist */
    else {
        ix = dmp->payloadFreelist;
        dmp->payloadFreelist = sizedNext (dmp, ix);
        dmp->numPayloadsInFreelist--;
    }

    if (! isNULLIX (ix)) dmp->numPayloadsAllocated++;

    return ix;
} /* allocPayload() */

/* ----------------
 * freePayload()
 * Frees a "payload" object, in the upper end of memory.
 * (when payload size is not a power of 2)
 */

static
void
freePayload (g2p3_lpmDeviceMem_t  *dmp,  int ix)
{
    /* ---- If adjacent to freespace, just grow freespace */
    if (ix == dmp->limitIx + 1) {
        dmp->limitIx += dmp->payloadSizeW;
    }
    /* ---- Return it to the freelist */
    else {
        setSizedNext (dmp, ix, dmp->payloadFreelist);
        dmp->payloadFreelist = ix;
        dmp->numPayloadsInFreelist++;
    }
    dmp->numPayloadsFreed++;
} /* freePayload() */

/* ****************************************************************
 * External API functions for the Lpm Device Memory Manager
 */

/* ---------------------------------------------------------------- */

sbStatus_t
g2p3_lpmDeviceMemInit(g2p3_lpmDeviceMemHandle_t *pHandle,
                          g2p3_lpmHostMemHandle_t hostMemHandle,
                          g2p3_lpmSramAddr start,
                          size_t nwords,
                          g2p3_lpmDeviceMemArea_t exclusions[],
                          sbBool_t ipv6
                          )
{
    sbStatus_t           err;
    void                     *vp;
    g2p3_lpmDeviceMem_t  *dmp;
    int                       j;

    DBGPR (("initDevMemMgr: required: from addr 0x%x, 0x%x words\n",
            start, nwords));

    *pHandle = NULL;

    /* ---- Alloc g2p3_lpmDeviceMem_t */
    DBGPR (("initDevMemMgr: allocating %d bytes for g2p3_lpmDeviceMem_t\n",
            sizeof(g2p3_lpmDeviceMem_t)));
    err = g2p3_lpmHostMemAlloc (hostMemHandle, & vp,
                                    sizeof(g2p3_lpmDeviceMem_t), "per dev mem mgr");
    if (err != SB_OK) return err;

    dmp = vp;
    dmp->hostMemHandle       = hostMemHandle;
    dmp->startAddr           = start;
    dmp->memSizeW            = nwords;
    dmp->freeIx              = 0;
    dmp->limitIx             = nwords - 1;
    dmp->log2_max_block_size = LOG2_MAX_BLOCK_SIZE;
    dmp->infoArr             = NULL;
    dmp->reverseLinkInfoArr = NULL;
    dmp->sizedFreelists      = NULL;
    dmp->payloadSizeW        = 0;
    dmp->payloadFreelist     = NULLIX;

    dmp->minFreeW            = dmp->limitIx - dmp->freeIx + 1;  /* == nwords */
    dmp->sizeHist            = NULL;
    dmp->wordsInUse          = 0;
    dmp->numBlksAllocated    = 0;
    dmp->numBlksFreed        = 0;

    dmp->numPayloadsInFreelist = 0;
    dmp->numPayloadsAllocated  = 0;
    dmp->numPayloadsFreed      = 0;

    dmp->ipv6 = ipv6;

    /* ---- Alloc mem for infoArr */
    DBGPR (("initDevMemMgr: allocating infoArr of 0x%x uint32_t's\n", nwords));
    err = g2p3_lpmHostMemAlloc (hostMemHandle, & vp,
                                    nwords * sizeof (uint32_t), "per dev meminf arr");
    if (err != SB_OK) {
        deallocMemMgrDataStructures (dmp);
        return err;
    }
    dmp->infoArr = vp;
    if(!ipv6)
    { 
        /* ---- Alloc mem for reverseLinkInfoArr */
        DBGPR (( "initDevMemMgr: allocating reverseLinkInfoArr of 0x%x uint32_t's\n", nwords));
        err = g2p3_lpmHostMemAlloc (hostMemHandle, & vp,
                                        nwords * sizeof (uint32_t), "per dev mem revinf arr");
        if (err != SB_OK) {
            deallocMemMgrDataStructures (dmp);
            return err;
        }
        dmp->reverseLinkInfoArr = vp;
    }


    
    /* ---- Alloc mem for SizedFreeLists array */
    DBGPR (("initDevMemMgr: allocating 0x%x ints for sizedFreelists\n",
            (LOG2_MAX_BLOCK_SIZE + 1)));
    err = g2p3_lpmHostMemAlloc (hostMemHandle, & vp,
                                    (LOG2_MAX_BLOCK_SIZE + 1) * sizeof (int), "per dev mem freelist");
    if (err != SB_OK) {
        deallocMemMgrDataStructures (dmp);
        return err;
    }
    dmp->sizedFreelists = vp;
    for (j = 0; j < LOG2_MAX_BLOCK_SIZE + 1; j++)
        dmp->sizedFreelists [j] = NULLIX;
    
    /* ---- Alloc mem for sizeHist */
    DBGPR (( "initDevMemMgr: allocating 0x%x ints for sizeHist\n",
            (LOG2_MAX_BLOCK_SIZE + 1)));
    err = g2p3_lpmHostMemAlloc (hostMemHandle, & vp,
                                    (LOG2_MAX_BLOCK_SIZE + 1) * sizeof (int), "per dev mem stats");
    if (err != SB_OK) {
        deallocMemMgrDataStructures (dmp);
        return err;
    }
    dmp->sizeHist = vp;
    for (j = 0; j < LOG2_MAX_BLOCK_SIZE + 1; j++) dmp->sizeHist [j] = 0;

    *pHandle = dmp;

    return SB_OK;
} /* g2p3_lpmDeviceMemInit() */

/* ---------------------------------------------------------------- */

sbStatus_t
g2p3_lpmDeviceMemAlloc(g2p3_lpmDeviceMemHandle_t handle,
                           g2p3_lpmSramAddr *pAddress,
                           size_t nwords)
{
    int ix;

    if (handle == NULL) {
	DBGPR (("***** Error (g2p3_lpmDeviceMemAlloc): handle is NULL\n"));
	ABORT(ABORT_HANDLE_IS_NULL);

        return SB_BAD_ARGUMENT_ERR_CODE;
    }

    if (nwords <= 0) {
	DBGPR (("***** Error (g2p3_lpmDeviceMemAlloc): nwords (0x%x) is <= 0\n",
                nwords));
	ABORT(ABORT_ALLOC_SIZE_IS_NOT_POSITIVE);

        return SB_BAD_ARGUMENT_ERR_CODE;
    }

    /* ---- Non-power-of-2-sized payloads */
    if (! (isPowerOf2 (nwords))) {
        /* ---- If first call, remember payload size */
        if (handle->payloadSizeW == 0)
            handle->payloadSizeW = nwords;
        /* ---- If not first call, check payload size is same as before */
        else if (nwords != handle->payloadSizeW) {
            DBGPR (("***** Error (g2p3_lpmDeviceMemAlloc): payload size (nwords=0x%x) different from previous call (0x%0x)\n",
                    nwords, handle->payloadSizeW));
            ABORT(ABORT_ALLOC_INCONSISTENT_PAYLOAD_SIZE);

            return SB_BAD_ARGUMENT_ERR_CODE;
        }
        ix = allocPayload (handle);
    }

    /* ---- Treechunk blocks, and power-of-2-sized payloads */
    else {
        if (sblog2(nwords) > handle->log2_max_block_size) {
            DBGPR (("***** Error (g2p3_lpmDeviceMemAlloc): nwords (0x%x) > max_block_size (0x%x)\n",
                    nwords, sbexp2 (handle->log2_max_block_size)));
            ABORT(ABORT_ALLOC_SIZE_TOO_BIG);

            return SB_BAD_ARGUMENT_ERR_CODE;
        }
        ix = allocBlock (handle, nwords);
    }

    if (! isNULLIX (ix)) {
        /* for ipv6 the finaly mem layout is flipped upside down,
         * i.e. payloads at the bottom and trie fragments hanging
         * from the top. Need to take size (nwords) into equation
         * for that - hence 2 diff. macros.
         */
        if (handle->ipv6) {
            *pAddress = ixSizeToAddr (handle, ix, nwords);
        }
        else {
            *pAddress = ixToAddr (handle, ix);
        }
#ifdef LPM_DEVICE_MEM_TRACE
        SB_LOG("\n Device Alloc: %x %x ", *pAddress, nwords);
#endif
        return SB_OK;
    }
    else
        return SB_LPM_OUT_OF_DEVICE_MEMORY;
} /* g2p3_lpmDeviceMemAlloc() */

/* ---------------------------------------------------------------- */

sbStatus_t
g2p3_lpmDeviceMemFree(g2p3_lpmDeviceMemHandle_t handle,
                          g2p3_lpmSramAddr address,
                          size_t nwords)
{
    SB_ASSERT (handle);
    SB_ASSERT (nwords > 0);
#ifdef LPM_DEVICE_MEM_TRACE
    SB_LOG(("\n Device Free: %x %x ", address, nwords));
#endif

    /* ---- Non-power-of-2-sized payloads */
    if (! (isPowerOf2 (nwords))) {
        /* ---- Check payload size is same as before */
        if (nwords != handle->payloadSizeW) {
            DBGPR (("***** Error (g2p3_lpmDeviceMemFree): payload size (nwords=0x%x) different from previous call (0x%0x)\n",
                    nwords, handle->payloadSizeW));
            ABORT(ABORT_ALLOC_INCONSISTENT_PAYLOAD_SIZE);

            return SB_BAD_ARGUMENT_ERR_CODE;
        }
        /* for ipv6 the finaly mem layout is flipped upside down,
         * i.e. payloads at the bottom and trie fragments hanging
         * from the top. Need to take size (nwords) into equation
         * for that - hence 2 diff. macros.
         */
        if (handle->ipv6) {
            freePayload (handle, addrSizeToIx (handle, address, nwords));
        }
        else {
            freePayload (handle, addrToIx (handle, address));
        }
    }
    /* ---- Treechunk blocks, and power-of-2-sized payloads */
    else {
        if (handle->ipv6) {
            freeBlock (handle, addrSizeToIx (handle, address, nwords), nwords);
        }
        else {
            freeBlock (handle, addrToIx (handle, address), nwords);
        }
    }

    return SB_OK;
} /* g2p3_lpmDeviceMemFree() */

/* ---------------------------------------------------------------- */

sbStatus_t
g2p3_lpmDeviceMemUninit(g2p3_lpmDeviceMemHandle_t *pDeviceMem)
{
    g2p3_lpmDeviceMem_t  *dmp;

    SB_ASSERT (pDeviceMem);
    dmp = *pDeviceMem;
    SB_ASSERT (dmp);

    return deallocMemMgrDataStructures (dmp);
} /* g2p3_lpmDeviceMemUninit() */

/* ---------------------------------------------------------------- */
void g2p3_lpmDeviceMem_dump(g2p3_lpmDeviceMemHandle_t *pDeviceMem)
{
    g2p3_lpmDeviceMem_t  *dmp;
    int i;
    int nblks, blksize;
    SB_ASSERT (pDeviceMem);
    dmp = *pDeviceMem;
    SB_ASSERT (dmp);    
    soc_cm_print("\n num words currently alloced [%d]", dmp->wordsInUse);
    soc_cm_print("\n num blocks allocated [%d]", dmp->numBlksAllocated);
    soc_cm_print("\n num blocks freed   [%d]", dmp->numBlksFreed);
    soc_cm_print("\n num payloads in freelist [%d]", dmp->numPayloadsInFreelist);
    soc_cm_print("\n num payloads alloc'd [%d]", dmp->numPayloadsAllocated);
    soc_cm_print("\n num payloads freed [%d]", dmp->numPayloadsFreed);

    soc_cm_print("\n Block alloc statistics");
    soc_cm_print("\n Block size : Num of blocks => Total Mem ");
    for (i=0; i<(LOG2_MAX_BLOCK_SIZE+1); i++) {
       if (dmp->sizeHist[i]) {
          nblks = dmp->sizeHist[i];
          blksize = 1 << i;
          soc_cm_print("\n%5d : %6d => %d", blksize, nblks, nblks * blksize);
       }
    }
}
