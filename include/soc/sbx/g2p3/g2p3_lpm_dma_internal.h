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

  $Id: g2p3_lpm_dma_internal.h 1.1.220.1 Broadcom SDK $

 *****************************************************************************/

#ifndef _G2P3_LPM_DMAI_H_
#define _G2P3_LPM_DMAI_H_
#include "g2p3_lpm_dma.h"

/* a request to write nWords contiguous words containing datum to address */
typedef struct writeRequest_s {
    sbDmaAddress_t address;  /* starting device address */
    uint32_t nWords;                /* number of words to transfer */
    uint32_t datum;                 /* datum to write */
    uint8_t  level;                 /* sync level */
} writeRequest_t, *writeRequest_p_t;

/* internal Dma manager data */
typedef struct g2p3_lpmDma_s {
    sbStatus_t status;         /* return status for callbacks */
    sbBool_t busy;             /* operation in progress */
    uint8_t lastLevel;              /* sync level of last slab */
    g2p3_lpmDmaSlabRequest_f_t dmaSlabReq; /* DMA transfer request */
    void *fe;               /* passed to DMA */
    void *pDmaCtxt;         /* fe2k DMA Mgr ptr */
    void *dmaReqToken;              /* passed to DMA */
    sbFe2000DmaSlabOp_t *slabOp;  /* slab operation for DMA */
    sbBool_t async;            /* asynchronous completion required */
    g2p3_lpmCallback_f_t cb;    /* asynchronous completion callback */
    void *cbUserData;               /* passed to callback */
    uint32_t slabWords;             /* uint32_t words */
    uint32_t nReqs;                 /* # of unflushed requests */
    uint32_t maxReqs;               /* maximum # of unflushed requests */
    writeRequest_p_t reqs;          /* unflushed request buffer */
    g2p3_lpmHostMemHandle_t hostMem; /* memory where to allocate self */
    sbFree_f_t hostFree;     /* for deallocating at uninit time */
    void *hostFreeToken;            /* passed to hostFree() */
    uint32_t hostAllocBytes;        /* total from host memory manager */
} g2p3_lpmDma_t;

#endif
