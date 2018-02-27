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

 * $Id: g2p3_lpm_hpm.c 1.2.112.2 Broadcom SDK $

 *****************************************************************************/

#include "g2p3_lpm_hpm_internal.h"

#ifndef LPM_HPM_DEBUG
#define LPM_HPM_DEBUG 0
#endif

static
int
hostPayloadCmp(void *token, void *left, void *right)
{
    g2p3_lpmHPMHandle_t hpm = (g2p3_lpmHPMHandle_t) token;
    SB_ASSERT(hpm);
    SB_ASSERT(left);
    SB_ASSERT(right);
    return SB_MEMCMP(left, right, hpm->bytesPerPayload);
}

sbStatus_t
g2p3_lpmHPMInit(g2p3_lpmHPMHandle_t *pHpm,
                    g2p3_lpmHostMemHandle_t hostMem,
                    size_t bytesPerPayload, sbBool_t ipv6)
{
    sbStatus_t status;
    g2p3_lpmHPMHandle_t hpm;
    void *vp;

    SB_ASSERT(pHpm);

    *pHpm = NULL;
    status = g2p3_lpmHostMemAlloc(hostMem, &vp, sizeof(g2p3_lpmHPM_t), "hpm mgr");
    if (status != SB_OK) return status;
    hpm = (g2p3_lpmHPMHandle_t) vp;
    status = g2p3_lpmRbtInit(&hpm->payloads, hostMem, hostPayloadCmp, hpm);
    if (status != SB_OK) {
        g2p3_lpmHostMemFree(hostMem, vp, sizeof(g2p3_lpmHPM_t));
        return status;
    }
    hpm->hostMem = hostMem;
    /* XXX make bytesPerPayload computed/allocated via callback
     * XXX to accomodate payloads of varying sizes */
    hpm->bytesPerPayload = bytesPerPayload;
    hpm->ipv6 = ipv6;

    *pHpm = hpm;
    return SB_OK;
}

sbStatus_t
g2p3_lpmHPMRef(g2p3_lpmHPMHandle_t hpm,
                   g2p3_lpmPayloadHandle_t *pHandle,
                   void *payload)
{
    sbStatus_t status;
    g2p3_lpmPayload_p_t hp;
    void *vp;

    SB_ASSERT(hpm);
    SB_ASSERT(pHandle);

    if (hpm->ipv6) {
        /* for ipv6 all payloads unique (to get unique offsets). Ought
           to really apply to 64-bit prefix payloads only. 
         */
        status = g2p3_lpmHostMemAlloc(hpm->hostMem, &vp,
                                      PAYLOAD_SIZE(hpm->bytesPerPayload), "hpm pyld ipv6");
         if (status != SB_OK) return status;
         hp = (g2p3_lpmPayload_p_t) vp;
         SB_MEMCPY(hp->data, payload, hpm->bytesPerPayload);
         hp->refs = 1;
         *pHandle = hp;
         return SB_OK;
    }

    status = g2p3_lpmRbtFind(hpm->payloads, &vp, payload);
    SB_ASSERT(status == SB_OK || status == SB_LPM_ADDRESS_NOT_FOUND);
    if (status == SB_OK) {
        hp = vp;
        SB_ASSERT(hp);
        SB_ASSERT(hp->refs > 0);
        ++hp->refs;
        *pHandle = hp;
        return status;
    }
    status = g2p3_lpmHostMemAlloc(hpm->hostMem, &vp,
                                      PAYLOAD_SIZE(hpm->bytesPerPayload), "hpm pyld");
    if (status != SB_OK) return status;
    hp = (g2p3_lpmPayload_p_t) vp;
    SB_MEMCPY(hp->data, payload, hpm->bytesPerPayload);
    hp->refs = 1;
    status = g2p3_lpmRbtInsert(hpm->payloads, hp->data, hp);
    SB_ASSERT(status != SB_LPM_DUPLICATE_ADDRESS);
    if (status != SB_OK) {
        g2p3_lpmHostMemFree(hpm->hostMem, vp,
                                PAYLOAD_SIZE(hpm->bytesPerPayload));
        return status;
    }
    *pHandle = hp;
    return SB_OK;
}

static
sbStatus_t
freeHostPayload(g2p3_lpmHPMHandle_t hpm,
                g2p3_lpmPayloadHandle_t payload)
{
    sbStatus_t status;
    SB_ASSERT(payload);

    if (hpm->ipv6) {
        /* see comment above about ipv6 payloads */
        status = g2p3_lpmHostMemFree(hpm->hostMem, payload,
                                         PAYLOAD_SIZE(hpm->bytesPerPayload));
        SB_ASSERT(status == SB_OK);
        return status;
    }
    
    status = g2p3_lpmRbtDelete(hpm->payloads, payload->data);
    SB_ASSERT(status == SB_OK);
    status = g2p3_lpmHostMemFree(hpm->hostMem, payload,
                                     PAYLOAD_SIZE(hpm->bytesPerPayload));
    SB_ASSERT(status == SB_OK);
    return status;
}

void *
g2p3_lpmHPMData(g2p3_lpmPayloadHandle_t payload)
{
    SB_ASSERT(payload);
    return (void *) payload->data;
}

sbStatus_t
g2p3_lpmHPMDeref(g2p3_lpmHPMHandle_t hpm,
                     g2p3_lpmPayloadHandle_t payload)
{
    sbStatus_t status = SB_OK;

    SB_ASSERT(payload);
    SB_ASSERT(payload->refs > 0);
    --payload->refs;
    if (payload->refs == 0) {
        status = freeHostPayload(hpm, payload);
        SB_ASSERT(status == SB_OK);
    }
    return status;
}

sbStatus_t
g2p3_lpmHPMUninit(g2p3_lpmHPMHandle_t *pHpm)
{
    sbStatus_t status;
    g2p3_lpmHPMHandle_t hpm;

    SB_ASSERT(pHpm);
    hpm = *pHpm;
    SB_ASSERT(hpm);

    status = SB_OK;
    SB_ASSERT(g2p3_lpmRbtIsEmpty(hpm->payloads)); /* no leaks */
    status = g2p3_lpmRbtUninit(&hpm->payloads);
    SB_ASSERT(status == SB_OK);
    status =
        g2p3_lpmHostMemFree(hpm->hostMem, hpm, sizeof(g2p3_lpmHPM_t));
    SB_ASSERT(status == SB_OK);
    return status;
}
