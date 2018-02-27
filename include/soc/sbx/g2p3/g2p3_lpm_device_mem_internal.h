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

  $Id: g2p3_lpm_device_mem_internal.h 1.1.220.1 Broadcom SDK $

 *****************************************************************************/

#ifndef _G2P3_LPM_DEVICE_MEM_I_H_
#define _G2P3_LPM_DEVICE_MEM_I_H_

#ifndef LPM_DEVICE_MEM_DEBUG
#define LPM_DEVICE_MEM_DEBUG 0
#endif

#include "g2p3_lpm_device_mem.h"

/* guts of device memory manager (see interface in g2p3_lpmDeviceMem.h) */

/* g2p3_lpmDeviceMem_p_t defined in g2p3_lpmDeviceMem.h */

/* ================================================================ */

/* ---- Largest allocatable block is 2^LOG2_MAX_BLOCK_SIZE words */

#define LOG2_MAX_BLOCK_SIZE  20

/* ---- compute 2^n */

#define sbexp2(n)   (1 << (n))

/* ---- State for LPM device mem mgr */

typedef struct g2p3_lpmDeviceMem_s {
    g2p3_lpmHostMemHandle_t  hostMemHandle;  /* stashed here for uninit's use */

    uint32_t   startAddr;
    int        memSizeW;          /* ---- Size of the memory space */
    int        freeIx;            /* ---- Freespace index into the mem space */
    int        limitIx;           /* ---- <= memSizeW, allocation limit fence */

    int        log2_max_block_size;  /* ---- Largest block size we manage */

    uint32_t  *infoArr;           /* ---- Block management info (sizes, next-links, etc.) */
    uint32_t  *reverseLinkInfoArr; /* reverse link management info(sizes, prev-links, etc.) */
    int       *sizedFreelists;    /* ---- Array of freelists based on size */

    int        payloadSizeW;      /* ---- 0, or # of words in a payload */
    int        payloadFreelist;
    sbBool_t ipv6;           /* for ipv6 we flip final memory layout */
                                  /* upside down */

    /* ---- Stats ---- */
    int        minFreeW;          /* ---- Min of limitIx - freeIx + 1 */

    int       *sizeHist;          /* ---- Histogram of allocation sizes */
    int        wordsInUse;        /* ---- num words currently alloc'd */
    int        numBlksAllocated;  /* ---- num blocks allocated since time 0 */
    int        numBlksFreed;      /* ---- num blocks freed since time 0 */

    int        numPayloadsInFreelist; /* ---- num payloads in freelist */
    int        numPayloadsAllocated;  /* ---- num payloads alloc'd since time 0 */
    int        numPayloadsFreed;      /* ---- num payloads freed since time 0 */
} g2p3_lpmDeviceMem_t;

/* ----------------------------------------------------------------
 * Macros to hide some actual representations
 * For a 2^j sized block at ix, infoArr [ix] contains:
 *   [31]     1 if free, 0 if busy (currently allocated)
 *   [30:24]  j (Log2 of blocksize)
 *   [23: 0]  For free blocks, a link (an ix) to next block in freelist j
 */

#define TAGMASK     0x80000000
#define SIZEMASK    0x7F000000
#define IXMASK      0x00FFFFFF
#define TAGIXMASK   (TAGMASK | IXMASK)

#define SIZESHIFT   24

#define TAGFREE   0x80000000         /* ---- Free block */
#define TAGBUSY   0x00000000         /* ---- Allocated block */

#define isFree(dmp, ix)            ((((dmp)->infoArr [ix]) & TAGMASK) == TAGFREE)
#define isBusy(dmp, ix)            ((((dmp)->infoArr [ix]) & TAGMASK) == TAGBUSY)

#define NULLIX                     0x00FFFFFF
#define isNULLIX(x)                ((x) == NULLIX)

#define sizedNext(dmp,ix)          (((dmp)->infoArr [ix]) & IXMASK)
#define setSizedNext(dmp,ix,ixN)   ((dmp)->infoArr [ix] = ( TAGFREE \
                                                          | ((dmp)->infoArr [ix] & SIZEMASK) \
                                                          | (ixN) ))

#define setBusy(dmp,ix)            ((dmp)->infoArr [ix] = ( TAGBUSY \
                                                          | ((dmp)->infoArr [ix] & SIZEMASK)))

#define blockSize(dmp, ix)         sbexp2 (((dmp)->infoArr [ix] & SIZEMASK) >> SIZESHIFT)
#define setBlockSize(dmp, ix, nw)  ((dmp)->infoArr [ix] = (((dmp)->infoArr [ix] & (~SIZEMASK)) | \
                                                           (sblog2 (nw) << SIZESHIFT)))
#define revListSizedPrev(dmp,ix)          (((dmp)->reverseLinkInfoArr [ix]) & IXMASK)
#define revListSetSizedPrev(dmp,ix,ixN)   ((dmp)->reverseLinkInfoArr [ix] = ( TAGFREE \
                                                          | ((dmp)->reverseLinkInfoArr [ix] & SIZEMASK) \
                                                          | (ixN) ))

#define revListSetBusy(dmp,ix)            ((dmp)->reverseLinkInfoArr [ix] = ( TAGBUSY \
                                                          | ((dmp)->reverseLinkInfoArr [ix] & SIZEMASK)))

#define revListBlockSize(dmp, ix)         sbexp2 (((dmp)->reverseLinkInfoArr [ix] & SIZEMASK) >> SIZESHIFT)
#define revListSetBlockSize(dmp, ix, nw)  ((dmp)->reverseLinkInfoArr [ix] = (((dmp)->reverseLinkInfoArr [ix] & (~SIZEMASK)) | \
                                                           (sblog2 (nw) << SIZESHIFT)))



/* ----------------
 * Conversion between indexes (ix) and addresses (addr)
 * - an 'index'   (ix)   is a word offset into the managed area
 * - an 'address' (addr) is an SRAM address for it
 */

#define ixToAddr(dmp,ix)  ((dmp)->startAddr + (ix))
#define addrToIx(dmp,a)   ((a) - (dmp)->startAddr)
#define ixSizeToAddr(dmp,ix,s)  ((dmp)->startAddr + (dmp)->memSizeW - (ix) - s)
#define addrSizeToIx(dmp,a,s)   ((dmp)->startAddr + (dmp)->memSizeW - (a) - s)

/* **************************************************************** */

#endif
