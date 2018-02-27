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

  $Id: g2p3_lpm_host_mem_internal.h 1.3.60.1 Broadcom SDK $

 *****************************************************************************/
#include <sal/core/sync.h>

#ifndef _G2P3_LPM_HOST_MEM_I_H_
#define _G2P3_LPM_HOST_MEM_I_H_


/* A block of host memory, header aids tracking */
typedef struct lpmHostMemBlock_s {
    struct lpmHostMemBlock_s *next;  
    struct lpmHostMemBlock_s *prev;  
    uint32_t size; 
    char *user;
} lpmHostMemBlock_t, *lpmHostMemBlock_p_t;


/* non-preallocated free list; size and free blocks */
typedef struct lpmHostMemFreeList_s {
    struct lpmHostMemFreeList_s *next; /* next free list */
    size_t blockBytes;                 /* size of blocks in this free-list */
    lpmHostMemBlock_p_t freeBlocks;    /* blocks in this free list */
} lpmHostMemFreeList_t, *lpmHostMemFreeList_p_t;

/* guts of host memory manager (see interface in g2p3_lpmHostMem.h */
typedef struct g2p3_lpmHostMem_s {
    sbFree_f_t hostFree; /* for deallocating self at uninit */
    void     *hostFreeToken;    /* token passed to hostFree() */
    size_t   allocatedBytes;    /* size to be freed with hostFree() */
    uint8_t  *start;            /* start of host mem */
    uint8_t  *free;             /* start of free host mem */
    uint8_t  *top;              /* end of host mem */
    uint64_t newAllocs;         /* number of allocs requiring raising top */
    uint64_t reusedAllocs;      /* number of allocs reusing free list entry */
    uint64_t deallocs;          /* number of deallocs */
    uint64_t allocBytes;        /* number of bytes allocated */
    uint64_t deallocBytes;      /* number of bytes allocated */
    lpmHostMemBlock_p_t *freeBlocks; /* preallocated free blocks SLL */
    lpmHostMemBlock_p_t usedBlocks;     /* blocks currently used DLL */
    lpmHostMemFreeList_p_t  extraFreeLists; /* additional free blocks,
                                             sorted by increasing size */
    sal_mutex_t mutex;
} g2p3_lpmHostMem_t;
/* g2p3_lpmHostMem_p_t defined in g2p3_lpm_host_mem.h */

#endif
