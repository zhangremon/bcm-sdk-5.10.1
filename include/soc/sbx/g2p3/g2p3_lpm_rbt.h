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

  $Id: g2p3_lpm_rbt.h 1.1.220.1 Broadcom SDK $

 *****************************************************************************/

#ifndef _G2P3_LPM_RED_BLACK_TREE_H_
#define _G2P3_LPM_RED_BLACK_TREE_H_

/* Red-Black Tree data structure from chapter 14 of [1].
 * Container nodes are allocated using a g2p3_lpmHostMem.* memory manager;
 * a comparison function is provided by the user.
 * Keys and values are abstract and are never modified or freed.
 * 
 * [1] Cormen, Leiserson, and Rivest.  Introduction to Algorithms.
 *     MIT Press, Cambridge, MA, 1990.
 */

#include "sbStatus.h"
#include "g2p3_lpm_host_mem.h"

typedef struct g2p3_lpmRbt_s *g2p3_lpmRbtHandle_t;

/* returns: v s.t. v < 0   if left < right
 *          0              if left == right
 *          v s.t. v > 0   if left > right
 */
typedef int
(*g2p3_lpmRbtCompare_f_t)(void *userToken, void *left, void *right);

/* requires: pRbt, cmp non-null; hostMem initialized
 * modifies: *pRbt
 * effects:  creates a new empty red-black tree and stores a handle in *pRbt
 *           cmp() is used to compare keys and is passed cmpUserToken
 * returns:  SB_OK on success
 *           SB_LPM_OUT_OF_HOST_MEMORY if memory allocation failed
 */
sbStatus_t
g2p3_lpmRbtInit(g2p3_lpmRbtHandle_t *pRbt,
                    g2p3_lpmHostMemHandle_t hostMem,
                    g2p3_lpmRbtCompare_f_t cmp, void *cmpUserToken);

/* requires: *pRbt initialized
 * modifies: *pRbt
 * effects:  destroys the red-black tree at *pRbt; sets *pRbt to NULL
 * returns:  SB_OK
 */
sbStatus_t
g2p3_lpmRbtUninit(g2p3_lpmRbtHandle_t *pRbt);

/* requires: rbt initialized
 * modifies: *rbt
 * effects:  inserts <key,val> into rbt unless <key,*> is already there
 * returns:  SB_OK on success
 *           SB_LPM_DUPLICATE_ADDRESS if <key,*> is already in rbt
 *           SB_LPM_OUT_OF_HOST_MEMORY if memory allocation failed
 */
sbStatus_t
g2p3_lpmRbtInsert(g2p3_lpmRbtHandle_t rbt, void *key, void *val);

/* requires: rbt initialized
 * modifies: rbt
 * effects:  deletes <key,*> from rbt unless <key,*> is not there
 * returns:  SB_OK on success
 *           SB_LPM_ADDRESS_NOT_FOUND if <key,*> is not in rbt
 */
sbStatus_t
g2p3_lpmRbtDelete(g2p3_lpmRbtHandle_t rbt, void *key);

/* requires: rbt initialized
 * modifies: nothing
 * effects:  sets *pVal to val s.t. <key,val> in rbt
 * returns:  SB_OK on success
 */
sbStatus_t
g2p3_lpmRbtFind(g2p3_lpmRbtHandle_t rbt, void **pVal, void *key);

/* requires: rbt initialized
 * modifies: nothing
 * effects:  none
 * returns:  TRUE iff rbt contains no keys
 */
sbBool_t
g2p3_lpmRbtIsEmpty(g2p3_lpmRbtHandle_t rbt);

#endif
