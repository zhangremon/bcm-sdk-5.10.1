/* -*- mode:c; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=2 cinoptions=\:0l1(0t0g0: */

/******************************************************************************

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
 * $Id: g2p3_lpm_dpm.c 1.4.112.2 Broadcom SDK $

 *****************************************************************************/

#ifndef LPM_DPM_DEBUG
#define LPM_DPM_DEBUG 0
#endif

#include "g2p3_lpm_dpm_internal.h"
#include "g2p3_lpm_dma.h"

/* note x and y possibly evaluated twice */
#define CMP(x,y) ((x) == (y) ? 0 : ((x) < (y) ? -1 : 1))

/* invalid extra bits uint16_t value */
#define LPM_DPM_INVALID_EXTRA_BITS 0xffff

/* maximum extra bits (12 bits for ipv6) */
#define LPM_DPM_MAX_EXTRA_BITS 0xfff


static
int
devicePayloadCmp(void *userToken, void *left, void *right)
{
    lpmDevicePayloadKey_p_t lkey, rkey;
    int cmp;

    SB_ASSERT(userToken == NULL);
    SB_ASSERT(left);
    SB_ASSERT(right);
    lkey = (lpmDevicePayloadKey_p_t) left;
    rkey = (lpmDevicePayloadKey_p_t) right;
    if ((cmp = CMP(lkey->payload, rkey->payload))) return cmp;
    if ((cmp = CMP(lkey->clsA, rkey->clsA))) return cmp;
    if ((cmp = CMP(lkey->clsB, rkey->clsB))) return cmp;
    if ((cmp = CMP(lkey->dropMaskLo, rkey->dropMaskLo))) return cmp;
    return CMP(lkey->dropMaskHi, rkey->dropMaskHi);
}

sbStatus_t
g2p3_lpmDPMInit(g2p3_lpmDPMHandle_t *pDpm,
                 g2p3_lpmHostMemHandle_t hostMem,
                 g2p3_lpmDeviceMemHandle_t deviceMemA,
                 g2p3_lpmDeviceMemHandle_t deviceMemB,
                 g2p3_lpmDmaHandle_t dma,
                 uint32_t paylDmaBankA, uint32_t paylDmaBankB, 
                 uint32_t paylBanks,
                 g2p3_lpmPayloadPack_f_t pack,
                 size_t wordsPerDevicePayload, int unit)
{
    sbStatus_t status, ignore_status;
    g2p3_lpmDPMHandle_t dpm;
    void *vp;

    SB_ASSERT(pDpm);

    *pDpm = NULL;
    status = g2p3_lpmHostMemAlloc(hostMem, &vp,
                                   PAYLOAD_MGR_SIZE(wordsPerDevicePayload), "dpm mgr");
    if (status != SB_OK)
        return status;
    dpm = (g2p3_lpmDPMHandle_t) vp;
    status = g2p3_lpmRbtInit(&dpm->payloads, hostMem,
                                 devicePayloadCmp, NULL);
    if (status != SB_OK) {
        ignore_status = g2p3_lpmHostMemFree(hostMem, vp,
                             PAYLOAD_MGR_SIZE(wordsPerDevicePayload));
        if(ignore_status != SB_OK) {
          SB_LOG("g2p3_lpmDPMInit:  g2p3_lpmHostMemFree Failed "
                 "host %p", vp);
        }
        return status;
    }
    dpm->hostMem = hostMem;
    dpm->deviceMemA = deviceMemA;
    dpm->deviceMemB = deviceMemB;
    dpm->dma = dma;
    dpm->dmaBankA = paylDmaBankA;
    dpm->dmaBankB = paylDmaBankB;
    dpm->numBanks = paylBanks;
    /* XXX make wordsPerDevicePayload computed/allocated via callback
     * XXX to accomodate payloads of varying sizes */
    dpm->wordsPerDevicePayload = wordsPerDevicePayload;
    dpm->pack = pack;
    dpm->newPayloads = NULL;
    dpm->zombiePayloads = NULL;
    dpm->busy = FALSE;
    dpm->async = FALSE;
    dpm->unit  = unit; 

    *pDpm = dpm;
    return SB_OK;
}

static
g2p3_lpmDevicePayload_p_t
findDevicePayload(g2p3_lpmDPMHandle_t dpm,
                  g2p3_lpmPayloadHandle_t payload,
                  g2p3_lpmClassifierId_t clsA,
                  g2p3_lpmClassifierId_t clsB,
                  uint32_t dropMaskLo, uint32_t dropMaskHi)
     
{
    sbStatus_t status;
    lpmDevicePayloadKey_t key;
    void *vp;
    g2p3_lpmDevicePayload_p_t dp;

    SB_ASSERT(dpm);
    key.payload = payload;
    key.clsA = clsA;
    key.clsB = clsB;
    key.dropMaskLo = dropMaskLo;
    key.dropMaskHi = dropMaskHi;
    status = g2p3_lpmRbtFind(dpm->payloads, &vp, &key);
    SB_ASSERT(status == SB_OK || status == SB_LPM_ADDRESS_NOT_FOUND);
    if (status != SB_OK)
        return NULL;
    SB_ASSERT(vp);
    dp = (g2p3_lpmDevicePayload_p_t) vp;
    SB_ASSERT(dp->key.payload == payload);
    SB_ASSERT(dp->key.clsA == clsA);
    SB_ASSERT(dp->key.clsB == clsB);
    SB_ASSERT(dp->key.dropMaskLo == dropMaskLo);
    SB_ASSERT(dp->key.dropMaskHi == dropMaskHi);
    return dp;
}

sbStatus_t
g2p3_lpmDPMRef(g2p3_lpmDPMHandle_t dpm,
                   g2p3_lpmPayloadHandle_t payload,
                   g2p3_lpmClassifierId_t clsA,
                   g2p3_lpmClassifierId_t clsB,
                   uint32_t dropMaskLo, uint32_t dropMaskHi)
{
    sbStatus_t status, ignore_status;
    g2p3_lpmDevicePayload_p_t dp;
    void *vp;
    g2p3_lpmSramAddr addr;

    SB_ASSERT(dpm);
#if LPM_DPM_DEBUG
            SB_LOG("DPM ref: %p, %x, %x ", payload, clsA, clsB);
#endif
    if (dpm->busy)
        return SB_BUSY_ERR_CODE;
    dp = findDevicePayload(dpm, payload, clsA, clsB, dropMaskLo, dropMaskHi);
#if LPM_DPM_DEBUG
    if (dp != NULL) {
        SB_LOG("(found at host %p, device 0x%08x, refs %d)\n", dp, dp->address, dp->refs);
    }
#endif
    if (dp != NULL) {
        SB_ASSERT(dp->refs > 0);
        ++dp->refs;
        return SB_OK;
    }
    status = g2p3_lpmHostMemAlloc(dpm->hostMem, &vp,
                                      sizeof(g2p3_lpmDevicePayload_t), "dpm pyld");
    if (status != SB_OK)
        return status;
    dp = (g2p3_lpmDevicePayload_p_t) vp;

    status = g2p3_lpmDeviceMemAlloc(dpm->deviceMemA, &dp->address,
                                  dpm->wordsPerDevicePayload / dpm->numBanks);
    if (status != SB_OK) {
        ignore_status = g2p3_lpmHostMemFree(dpm->hostMem, vp,
                                sizeof(g2p3_lpmDevicePayload_t));
        if(ignore_status != SB_OK) {
          SB_LOG("g2p3_lpmDPMRef:  g2p3_lpmHostMemFree Failed "
                 "host %p", vp);
        }
        return status;
    }

    /* if spread across 2 banks need to allocate separately with each
     * dev mem mgr 
     */
    if (dpm->numBanks > 1) {
      status = g2p3_lpmDeviceMemAlloc(dpm->deviceMemB, &addr,
                                    dpm->wordsPerDevicePayload / dpm->numBanks);
      if (status != SB_OK) {
        ignore_status = g2p3_lpmHostMemFree(dpm->hostMem, vp,
                                sizeof(g2p3_lpmDevicePayload_t));
        if(ignore_status != SB_OK) {
          SB_LOG("g2p3_lpmDPMRef:  g2p3_lpmHostMemFree Failed "
                 "host %p", vp);
        }

        ignore_status = g2p3_lpmDeviceMemFree(dpm->deviceMemA, dp->address,
                                    dpm->wordsPerDevicePayload / dpm->numBanks);
        if(ignore_status != SB_OK) {
          SB_LOG("g2p3_lpmDPMRef:  g2p3_lpmDeviceMemFree[A] Failed "
                 "host %p Device 0x%08x", dp, dp->address);
        }
        return status;
      }

      SB_ASSERT (dp->address == addr);
    }

#if LPM_DPM_DEBUG
    SB_LOG("(new at host %p, device 0x%08x)\n", dp, dp->address);
#endif
    dp->key.payload = payload;
    dp->key.clsA = clsA;
    dp->key.clsB = clsB; 
    dp->key.dropMaskLo = dropMaskLo;
    dp->key.dropMaskHi = dropMaskHi;
    dp->refs = 1;
    dp->extraBits = LPM_DPM_INVALID_EXTRA_BITS;
    status = g2p3_lpmRbtInsert(dpm->payloads, &dp->key, dp);
    SB_ASSERT(status != SB_LPM_DUPLICATE_ADDRESS);
    if (status != SB_OK) {
      ignore_status = g2p3_lpmDeviceMemFree(dpm->deviceMemA, dp->address,
                             dpm->wordsPerDevicePayload / dpm->numBanks);
      if(ignore_status != SB_OK) {
        SB_LOG("DPM deref: g2p3_lpmDeviceMemFree[A] Failed "
               "host %p device 0x%08x", dp, dp->address);
      }

      if (dpm->numBanks > 1) {
        ignore_status = g2p3_lpmDeviceMemFree(dpm->deviceMemB, dp->address,
                             dpm->wordsPerDevicePayload / dpm->numBanks);
        if(ignore_status != SB_OK) {
          SB_LOG("DPM deref: g2p3_lpmDeviceMemFree[B] Failed "
                 "host %p device 0x%08x", dp, dp->address);
        }
      }

      ignore_status = g2p3_lpmHostMemFree(dpm->hostMem, vp,
                           sizeof(g2p3_lpmDevicePayload_t));
      if(ignore_status != SB_OK) {
        SB_LOG("DPM deref:  g2p3_lpmHostMemFree Failed "
               "host %p", vp);
      }
      return status;
    }
    dp->next = dpm->newPayloads;
    dp->prev = NULL;
    if (dpm->newPayloads) dpm->newPayloads->prev = dp;
    dpm->newPayloads = dp;
    return SB_OK;
}

static
sbStatus_t
freeDevicePayload(g2p3_lpmDPMHandle_t dpm,
                  g2p3_lpmDevicePayload_p_t devicePayload)
{
    sbStatus_t status=SB_OK, ignore_status;

    SB_ASSERT(dpm);
    SB_ASSERT(devicePayload);

    ignore_status = g2p3_lpmDeviceMemFree(dpm->deviceMemA, devicePayload->address,
                                    dpm->wordsPerDevicePayload / dpm->numBanks);
    if(ignore_status != SB_OK) {
      SB_LOG("freeDevicePayload: g2p3_lpmDeviceMemFree Failed "
             "host %p dev[A]Address 0x%08x", dpm->deviceMemA, devicePayload->address);
      status = ignore_status;
    }

    if (dpm->numBanks > 1) {
      ignore_status = g2p3_lpmDeviceMemFree(dpm->deviceMemB, devicePayload->address,
                           dpm->wordsPerDevicePayload / dpm->numBanks);
      SB_LOG("freeDevicePayload: g2p3_lpmDeviceMemFree Failed "
             "host %p dev[B]Address 0x%08x", dpm->deviceMemB, devicePayload->address);
      status = ignore_status;
    }

    ignore_status = g2p3_lpmHostMemFree(dpm->hostMem, devicePayload,
                                     sizeof(g2p3_lpmDevicePayload_t));
    if(ignore_status != SB_OK) {
      SB_LOG("freeDevicePayload:  g2p3_lpmHostMemFree Failed "
             "host %p", devicePayload);
      status = ignore_status;
    }
    return status;
}

sbStatus_t
g2p3_lpmDPMDeref(g2p3_lpmDPMHandle_t dpm,
                     g2p3_lpmPayloadHandle_t payload,
                     g2p3_lpmClassifierId_t clsA,
                     g2p3_lpmClassifierId_t clsB,
                     uint32_t dropMaskLo, uint32_t dropMaskHi)
{
    g2p3_lpmDevicePayload_p_t dp;   /* device payload node to remove */

    SB_ASSERT(dpm);
    if (dpm->busy)
        return SB_BUSY_ERR_CODE;
#if LPM_DPM_DEBUG
            SB_LOG("DPM deref: %p, %x, %x ", payload, clsA, clsB);
#endif
    dp = findDevicePayload(dpm, payload, clsA, clsB, dropMaskLo, dropMaskHi);
#if LPM_DPM_DEBUG
    if (dp == NULL)
        SB_LOG("(not found)\n");
    else
        SB_LOG("(found at host %p, device 0x%08x, refs %d)\n", dp, dp->address, dp->refs);
#endif
    if (dp == NULL) return SB_LPM_ADDRESS_NOT_FOUND;
    SB_ASSERT(dp->refs > 0);
    --dp->refs;
    if (dp->refs == 0) {
        sbStatus_t status =
            g2p3_lpmRbtDelete(dpm->payloads, &dp->key);
        SB_ASSERT(status == SB_OK);
        /* ejs - Jul 19 2005 - return bad status */
        if(status != SB_OK) {
            return status;
        }
        if (dp->prev) dp->prev->next = dp->next;
        if (dp->next) dp->next->prev = dp->prev;
        if (dpm->newPayloads == dp) dpm->newPayloads = dp->next;
        dp->next = dpm->zombiePayloads;
        dp->prev = NULL;
        if (dp->next) dp->next->prev = dp;
        dpm->zombiePayloads = dp;
    }
    return SB_OK;
}

sbStatus_t
g2p3_lpmDPMInfo(g2p3_lpmDPMHandle_t dpm,
                    g2p3_lpmSramAddr *pAddress,
                    uint16_t *pExtra,
                    g2p3_lpmPayloadHandle_t payload,
                    g2p3_lpmClassifierId_t clsA,
                    g2p3_lpmClassifierId_t clsB, 
                    uint32_t dropMaskLo, uint32_t dropMaskHi)
{
    g2p3_lpmDevicePayload_p_t dp;

    SB_ASSERT(dpm);
    if (dpm->busy) return SB_BUSY_ERR_CODE;
    dp = findDevicePayload(dpm, payload, clsA, clsB, dropMaskLo, dropMaskHi);
    if (dp == NULL) return SB_LPM_ADDRESS_NOT_FOUND;
    if (dp->extraBits == LPM_DPM_INVALID_EXTRA_BITS) {
        sbStatus_t status = dpm->pack((void *) dpm->dmaBuf,
                                           &dp->extraBits,
                                           g2p3_lpmHPMData(payload),
                                           dpm->newPayloads->key.clsA,
                                           dpm->newPayloads->key.clsB,
                                           dpm->newPayloads->key.dropMaskLo,
                                           dpm->newPayloads->key.dropMaskHi,
                                           dpm->unit);
        if (status != SB_OK) return status;
    }
    SB_ASSERT(dp->extraBits <= LPM_DPM_MAX_EXTRA_BITS);
    *pAddress = dp->address;
    *pExtra = dp->extraBits;
    return SB_OK;
}

static
void
commitDone(g2p3_lpmDPMHandle_t dpm)
{
    g2p3_lpmDevicePayload_p_t dp, next;
    sbStatus_t tmpStatus;

    SB_ASSERT(dpm);
    SB_ASSERT(dpm->busy);
    SB_ASSERT(dpm->newPayloads == NULL || dpm->status != SB_OK);
    for (dp = dpm->zombiePayloads; dp != NULL; dp = next) {
        next = dp->next;
        tmpStatus = freeDevicePayload(dpm, dp);
        dpm->status = (dpm->status != SB_OK ? dpm->status : tmpStatus);
    }
    dpm->zombiePayloads = NULL;
    dpm->busy = FALSE;
    if (dpm->cb && dpm->async)
        dpm->cb(dpm->cbUserData, dpm->status);
}

static
void
commitDoneCB(void *userData, sbStatus_t status)
{
    g2p3_lpmDPMHandle_t dpm;

    dpm = (g2p3_lpmDPMHandle_t) userData;
    SB_ASSERT(dpm);
    dpm->async = TRUE;
    dpm->status = status;
    commitDone(dpm);
}


static
void
commitCB(void *userData, sbStatus_t status);

static
void
commit(g2p3_lpmDPMHandle_t dpm)
{
    uint32_t datum;
    void *payload;
    sbDmaAddress_t dmaAddr;
    uint32_t i, currBank, offs;

    SB_ASSERT(dpm);
    SB_ASSERT(dpm->busy);
    while (dpm->newPayloads) {
        if (dpm->nextWord == 0) {
            payload = g2p3_lpmHPMData(dpm->newPayloads->key.payload);
            dpm->status = dpm->pack((void *) dpm->dmaBuf,
                                    &dpm->newPayloads->extraBits,
                                    payload,
                                    dpm->newPayloads->key.clsA,
                                    dpm->newPayloads->key.clsB,
                                    dpm->newPayloads->key.dropMaskLo,
                                    dpm->newPayloads->key.dropMaskHi,
                                    dpm->unit);
            if (dpm->status != SB_OK) {
                commitDone(dpm);
                return;
            }
        }
        if (dpm->nextWord >= dpm->wordsPerDevicePayload) {
            g2p3_lpmDevicePayload_p_t next;
            next = dpm->newPayloads->next;
            dpm->newPayloads->prev = NULL;
            dpm->newPayloads->next = NULL;
            if (next) next->prev = NULL;
            dpm->newPayloads = next;
            dpm->nextWord = 0;
        } else {
            for (i = 0; i < dpm->wordsPerDevicePayload; i++) {
              datum = dpm->dmaBuf[dpm->nextWord];
              /* even words - bankA, odd ones - bankB */
              if (i % dpm->numBanks) 
                currBank = dpm->dmaBankB;
              else 
                currBank = dpm->dmaBankA;

              offs = dpm->newPayloads->address +  i / dpm->numBanks;
              dmaAddr = SB_FE2000_DMA_MAKE_ADDRESS(currBank, offs);

              ++dpm->nextWord;
#if LPM_DPM_DEBUG
              SB_LOG("DPM commit: writing @0x%08x\n", dmaAddr);
#endif
              dpm->status = g2p3_lpmDmaWrite(dpm->dma, dmaAddr, 1, datum, 0,
                                              commitCB, (void *) dpm);

              if (dpm->status == SB_IN_PROGRESS)
                return;

              if (dpm->status != SB_OK) {
                commitDone(dpm);
                return;
              }
            }
        } /* else */
    }
#if LPM_DPM_DEBUG
    SB_LOG("DPM commit: calling g2p3_lpmDmaFlush\n");
#endif
    dpm->status = g2p3_lpmDmaFlush(dpm->dma, commitDoneCB, (void *) dpm);
    if (dpm->status != SB_IN_PROGRESS) {
        if (dpm->busy == TRUE)
            commitDone(dpm);
    }
}

static
void
commitCB(void *userData, sbStatus_t status)
{
    g2p3_lpmDPMHandle_t dpm;

    dpm = (g2p3_lpmDPMHandle_t) userData;
    SB_ASSERT(dpm);
    dpm->async = TRUE;
    dpm->status = status;
    if (status != SB_OK) {
        commitDone(dpm);
    } else {
        commit(dpm);
    }
}


sbStatus_t
g2p3_lpmDPMCommit(g2p3_lpmDPMHandle_t dpm,
                      g2p3_lpmCallback_f_t asyncCallback,
                      void *asyncCallbackUserData)
{
    SB_ASSERT(dpm);

    if (dpm->busy)
        return SB_BUSY_ERR_CODE;
    dpm->cb = asyncCallback;
    dpm->cbUserData = asyncCallbackUserData;
    dpm->nextWord = 0;
    dpm->busy = TRUE;
    dpm->async = FALSE;
    commit(dpm);
    return dpm->status;
}

sbStatus_t
g2p3_lpmDPMUninit(g2p3_lpmDPMHandle_t *pDpm)
{
    sbStatus_t status;
    g2p3_lpmDPMHandle_t dpm;
    g2p3_lpmDevicePayload_p_t dp, next;

    SB_ASSERT(pDpm);
    dpm = *pDpm;
    SB_ASSERT(dpm);
    SB_ASSERT(dpm->newPayloads == NULL);              /* no leaks */
    SB_ASSERT(g2p3_lpmRbtIsEmpty(dpm->payloads)); /* no leaks */

    status = g2p3_lpmRbtUninit(&dpm->payloads);
    SB_ASSERT(status == SB_OK);
    for (dp = dpm->zombiePayloads; dp != NULL; dp = next) {
        next = dp->next;
        status = freeDevicePayload(dpm, dp);
        SB_ASSERT(status == SB_OK);
    }
    status = g2p3_lpmHostMemFree(dpm->hostMem, dpm,
                                 PAYLOAD_MGR_SIZE(dpm->wordsPerDevicePayload));
    if(status != SB_OK) {
      SB_LOG("g2p3_lpmDPMUninit:  g2p3_lpmHostMemFree Failed "
             "host %p", dpm);
    }
    return status;
}
