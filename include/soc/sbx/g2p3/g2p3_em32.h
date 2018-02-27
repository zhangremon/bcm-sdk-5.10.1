/* -*- mode:c++; c-style:k&r; c-basic-offset:4; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=4 cinoptions=\:0l1(0t0g0: */

#ifndef _SB_EM32_H
#define _SB_EM32_H
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
 * $Id: g2p3_em32.h 1.9.106.1 Broadcom SDK $
 *
 ******************************************************************************/

#include <soc/sbx/fe2k_common/sbFeISupport.h>
#include <soc/sbx/fe2k_common/sbFe2000DmaMgr.h>
#include "sbFe2000UcodeLoad.h"


typedef int (*g2p3_em32KeyValid_f_t)(uint32 key);
typedef int (*g2p3_em32SetCuckooSeed_f_t)(int unit, uint32 seed);
typedef int (*g2p3_em32GetCuckooSeed_f_t)(int unit, uint32 *seed);

sbStatus_t
g2p3_em32InitCuckooSeed (void *em32Ctxt, 
                         g2p3_em32SetCuckooSeed_f_t cuckooSeedSet,
                         g2p3_em32GetCuckooSeed_f_t cuckooSeedGet);

uint32_t
g2p3_em32SetBusy (void *em32Ctxt);

sbStatus_t
g2p3_em32HWInitStart (void *em32Ctxt, sbFeInitAsyncCallback_f_t cBack,
                 void *initId);

sbStatus_t
g2p3_em32CommitStart (void *em32Ctxt);

sbStatus_t
g2p3_em32Recover(void *em32Ctxt);

sbStatus_t
g2p3_em32Init (void*                      *em32Ctxt,
               soc_sbx_g2p3_state_t      *pFe,
               sbFe2000DmaMgr_t          *pDmaCtxt,
               sbCommonConfigParams_p_t   cParams,
               g2p3_em32KeyValid_f_t      keyValid_f,
               sbFe2000DmaAddr_t          leftBase,
               sbFe2000DmaAddr_t          rightBase,
               sbFe2000DmaAddr_t          paylBase,
               sbFe2000DmaAddr_t          paylBaseRight,
               uint32_t                   leftBank,
               uint32_t                   rightBank,
               uint32_t                   paylBankLeft,
               uint32_t                   paylBankRight,
               uint32_t                   topOff,
               uint32_t                   tableSize,
               uint32_t                   paylSize,
               uint32_t                   slabSize,
               uint32_t                   lowIdxBits,
               uint8_t                    useDuplicateBank,
               uint32_t                   dupLeftBank,
               uint32_t                   dupRightBank,
               sbFe2000DmaAddr_t          dupLeftBase,
               sbFe2000DmaAddr_t          dupRightBase);


sbStatus_t
g2p3_em32Uninit (void *em32Ctxt);

sbStatus_t
sbG2P3AddEM32 (void *em32Ctxt, uint32_t key, uint8_t *payload);


sbStatus_t
sbG2P3RemEM32 (void *em32Ctxt, uint32_t key);


sbStatus_t
sbG2P3UpdateEM32 (void *em32Ctxt, uint32_t key, uint8_t *payload);

sbStatus_t
sbG2P3GetEM32 (void *em32Ctxt, uint32_t key, uint8_t *payload);

sbStatus_t
sbG2P3GetKeysEM32 (void *em32Ctxt, uint32_t key, uint32_t *left,
                   uint32_t *right, uint32_t *seed, int *b);

sbStatus_t
sbG2P3FirstKey (void *em32Ctxt, uint32_t *key);

sbStatus_t
sbG2P3NextKey (void *em32Ctxt, uint32_t key, uint32_t *next_key);

#endif
