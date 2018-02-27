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

  $Id: g2p3_lpm_dpm_internal.h 1.2.220.1 Broadcom SDK $

 *****************************************************************************/

#ifndef _G2P3_LPM_DPMI_H_
#define _G2P3_LPM_DPMI_H_

#include "g2p3_lpm_dpm.h"
#include "g2p3_lpm_rbt.h"

/* see  below for definition of struct g2p3_lpmDevicePayload_s */
typedef struct g2p3_lpmDevicePayload_s *g2p3_lpmDevicePayload_p_t;

/* internal device payload manager data
 * init allocates PAYLOAD_MGR_SIZE(dma-buffer-size) words and casts to
 * g2p3_lpmDPM_t
 */
typedef struct g2p3_lpmDPM_s {
    int unit;
    g2p3_lpmHostMemHandle_t hostMem;     /* host memory */
    g2p3_lpmDeviceMemHandle_t deviceMemA; /* device memory */
    g2p3_lpmDeviceMemHandle_t deviceMemB;
    g2p3_lpmDmaHandle_t dma;             /* DMA manager */
    sbBool_t busy;                        /* operation in progress */
    uint32_t dmaBankA;                    /* which SRAM bank to write to */
    uint32_t dmaBankB;
    uint32_t numBanks;                    /* number of banks for payload */
    uint32_t wordsPerDevicePayload;       /* how many words per payload */ 
    g2p3_lpmPayloadPack_f_t pack;        /* function to pack           */
                                          /* <payload,clsA,clsB> triple */
                                          /* into device representation */

    /* payload collections: a "live" payload is in payloads (and, if it has
     * not yet been committed, also in newPayloads); a "zombie" payload is only
     * in zombiePayloads
     */
    g2p3_lpmRbtHandle_t payloads;             /* payloads in DRM */
    g2p3_lpmDevicePayload_p_t newPayloads;    /* in payloads, uncommitted */
    g2p3_lpmDevicePayload_p_t zombiePayloads; /* dead payloads in DPM */

    /* commit support */
    sbBool_t async;         /* asynchronous completion required */
    g2p3_lpmCallback_f_t cb; /* asynchronous completion callback */
    void *cbUserData;            /* asynchronous callback user data */
    sbStatus_t status;      /* status of last operation */
    g2p3_lpmDevicePayload_p_t nextDevicePayload; /* next payload to DMA */
    uint32_t nextWord;           /* next word of current payload */
    uint32_t dmaBuf[1];          /* buffer for packing payloads */
} g2p3_lpmDPM_t;

#define PAYLOAD_MGR_SIZE(nDmaBufWords) \
    (sizeof(g2p3_lpmDPM_t) + sizeof(uint32_t) * ((nDmaBufWords) - 1))


typedef struct lpmDevicePayloadKey_s {
    g2p3_lpmPayloadHandle_t payload; /* main payload */
    g2p3_lpmClassifierId_t clsA;          /* class ID A */
    g2p3_lpmClassifierId_t clsB;          /* class ID B */
    uint32_t dropMaskLo; 
    uint32_t dropMaskHi;
} lpmDevicePayloadKey_t, *lpmDevicePayloadKey_p_t;

/* keeps track of a <payload,clsA,clsB> in device memory (link in a list) */
typedef struct g2p3_lpmDevicePayload_s {
    lpmDevicePayloadKey_t key;           /* payload handle and class IDs */
    uint32_t refs;                       /* number of references */
    g2p3_lpmSramAddr address;           /* where it lives in device memory */
    uint16_t extraBits;                  /* extra ten bits to be stored
                                            in payload node */
    g2p3_lpmDevicePayload_p_t next;  /* link to next device payload used
                                            in newPayloads and zombiePayloads */
    g2p3_lpmDevicePayload_p_t prev;  /* link to prev device payload used
                                            in newPayloads */
} g2p3_lpmDevicePayload_t;

#endif
