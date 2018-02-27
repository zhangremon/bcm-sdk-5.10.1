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

 * $Id: g2p3_lpm_rbt.c 1.5.112.2 Broadcom SDK $

 *****************************************************************************/

#include "g2p3_lpm_rbt.h"
#include "g2p3_lpm_rbt_internal.h"
#include "sbWrappers.h"

#define RBT_COLOR(n) ((n) ? (n)->color : RBT_BLACK)
#define RBT_IS_RED(n) (RBT_COLOR(n) == RBT_RED)
#define RBT_IS_BLACK(n) (RBT_COLOR(n) == RBT_BLACK)
#define RBT_SET_COLOR(n, c) do { if (n) (n)->color = (c); } while (0)
#define RBT_PAINT_RED(n) RBT_SET_COLOR(n, RBT_RED)
#define RBT_PAINT_BLACK(n) RBT_SET_COLOR(n, RBT_BLACK)

/* requires: node, node->right non-null
 * modifies: tree rooted at node
 * effects:  performs left rotation as shown below, pivoted at node
 *                      |            |
 *                      x            y
 *                     / \          / \
 *                    a   y  --->  x   c
 *                       / \      / \
 *                      b   c    a   b
 *           (legend: x = node, y = node->right, a, b, c = arbitrary subtrees)
 */
static
void
rbtRotateLeft(rbtNode_p_t node)
{
    rbtNode_p_t left;
    rbtColor_t color;
    void *key, *val;

    SB_ASSERT(node);
    SB_ASSERT(node->right);
    
    key = node->key;
    val = node->val;
    color = node->color;
    node->key = node->right->key;
    node->val = node->right->val;
    node->color = node->right->color;
    node->right->key = key;
    node->right->val = val;
    node->right->color = color;
    left = node->left;
    node->left = node->right;
    node->right = node->left->right;
    node->left->right = node->left->left;
    node->left->left = left;
}

/* requires: node, node->left non-null
 * modifies: tree rooted at node
 * effects:  performs right rotation as shown below, pivoted at node
 *                      |            |
 *                      y            x
 *                     / \          / \
 *                    x   c  --->  a   y
 *                   / \              / \
 *                  a   b            b   c
 *           (legend: y = node, x = node->left, a, b, c = arbitrary subtrees)
 */
static
void
rbtRotateRight(rbtNode_p_t node)
{
    rbtNode_p_t right;
    rbtColor_t color;
    void *key, *val;

    SB_ASSERT(node);
    SB_ASSERT(node->left);
    
    key = node->key;
    val = node->val;
    color = node->color;
    node->key = node->left->key;
    node->val = node->left->val;
    node->color = node->left->color;
    node->left->key = key;
    node->left->val = val;
    node->left->color = color;
    right = node->right;
    node->right = node->left;
    node->left = node->right->left;
    node->right->left = node->right->right;
    node->right->right = right;
}

/* requires: node non-null; a node inserted on the left
 * modifies: tree rooted at node
 * effects:  changes the tree structure to remove or bubble up
 *           a red-node-with-a-red-child violation after an insert
 */
static
void
rbtInsertFixLeft(rbtNode_p_t node)
{
    SB_ASSERT(node);
    if (RBT_IS_RED(node->left)) {
        if (RBT_IS_RED(node->left->right)) {                    /* case 2 */
            rbtRotateLeft(node->left);
            rbtInsertFixLeft(node);
        } else if (RBT_IS_RED(node->left->left)) {
            if (RBT_IS_RED(node->right)) {                      /* case 1 */
                RBT_PAINT_BLACK(node->left);
                RBT_PAINT_BLACK(node->right);
                RBT_PAINT_RED(node);
            } else {                                            /* case 3 */
                RBT_PAINT_BLACK(node->left);
                RBT_PAINT_RED(node);
                rbtRotateRight(node);
            }
        }
    }
}

/* requires: node non-null; a node inserted on the right
 * modifies: tree rooted at node
 * effects:  changes the tree structure to remove or bubble up
 *           a red-node-with-a-red-child violation after an insert
 */
static
void
rbtInsertFixRight(rbtNode_p_t node)
{
    SB_ASSERT(node);
    if (RBT_IS_RED(node->right)) {
        if (RBT_IS_RED(node->right->left)) {                    /* case 2 */
            rbtRotateRight(node->right);
            rbtInsertFixRight(node);
        } else if (RBT_IS_RED(node->right->right)) {
            if (RBT_IS_RED(node->left)) {                       /* case 1 */
                RBT_PAINT_BLACK(node->right);
                RBT_PAINT_BLACK(node->left);
                RBT_PAINT_RED(node);
            } else {                                            /* case 3 */
                RBT_PAINT_BLACK(node->right);
                RBT_PAINT_RED(node);
                rbtRotateLeft(node);
            }
        }
    }
}

/* requires: rbt initialized, pNode non-null
 * modifies: tree rooted at *pNode
 * effects:  inserts <key,val> into the tree rooted at *pNode
 * returns:  SB_OK on success
 *           SB_LPM_DUPLICATE_ADDRESS if key is already in the tree
 *           SB_LPM_OUT_OF_HOST_MEMORY if memory allocation failed
 */
static
sbStatus_t
rbtInsert(g2p3_lpmRbtHandle_t rbt, rbtNode_p_t *pNode, void *key, void *val)
{
    sbStatus_t status;
    void *vp;

    SB_ASSERT(rbt);
    SB_ASSERT(rbt->cmp);
    SB_ASSERT(pNode);
    
    if (*pNode == NULL) { /* alloc/insert node, with null children */
        status = g2p3_lpmHostMemAlloc(rbt->hostMem, &vp, sizeof(rbtNode_t), "lpm rbt node");
        if (status != SB_OK) return status;
        SB_ASSERT(vp);
        (*pNode) = (rbtNode_p_t) vp;
        (*pNode)->key = key;
        (*pNode)->val = val;
        (*pNode)->left = NULL;
        (*pNode)->right = NULL;
        (*pNode)->color = RBT_RED;
    } else { /* recurse down the tree along the correct path */
        int cmp = rbt->cmp(rbt->cmpUserToken, key, (*pNode)->key);
    /*    coverity[new_values]    */
        cmp = cmp < 0 ? -1 : (cmp > 0 ? 1 : 0);
        switch (cmp) {
        case -1: /* key < (*pNode)->key: go left */
            status = rbtInsert(rbt, &(*pNode)->left, key, val);
            if (status != SB_OK) return status;
            SB_ASSERT((*pNode)->left); /* ...we just added a node */
            rbtInsertFixLeft(*pNode);
            break;
        case 1: /* key > (*pNode)->key: go right */
            status = rbtInsert(rbt, &(*pNode)->right, key, val);
            if (status != SB_OK) return status;
            SB_ASSERT((*pNode)->right); /* ...we just added a node */
            rbtInsertFixRight(*pNode);
            break;
        case 0:  return SB_LPM_DUPLICATE_ADDRESS;
        default:
            SB_ASSERT(0);
            return SB_FAILED;
        }
    }
    return SB_OK;
}

/* requires: node, node->right non-null;
 *           node->left has an extra layer of black paint
 * modifies: tree rooted at node
 * effects:  changes the tree structure to remove or bubble up
 *           the extra layer of black paint after a delete
 * returns:  SB_OK on success when node is red or black
 *           SB_MORE on success when node is doubly black
 */
static
sbStatus_t
rbtDeleteFixLeft(rbtNode_p_t node)
{
    sbStatus_t status;

    SB_ASSERT(node);
    SB_ASSERT(node->right); /* else inconsistent black-height */
    if (RBT_IS_RED(node->left)) {                               /* case 0 */
        RBT_PAINT_BLACK(node->left);
        status = SB_OK;
    } else if (RBT_IS_BLACK(node) && RBT_IS_RED(node->right)) { /* case 1 */
        SB_ASSERT(RBT_IS_BLACK(node->right->left));
        SB_ASSERT(RBT_IS_BLACK(node->right->right));
        RBT_PAINT_BLACK(node->right);
        RBT_PAINT_RED(node);
        rbtRotateLeft(node);
        status = rbtDeleteFixLeft(node->left);
        if (status == SB_MORE) {
            SB_ASSERT(RBT_IS_RED(node->left));
            RBT_PAINT_BLACK(node->left);
            status = SB_OK;
        }
    } else { /* RBT_IS_BLACK(node->right) */
        SB_ASSERT(RBT_IS_BLACK(node->right));
        if (RBT_IS_BLACK(node->right->left)
            && RBT_IS_BLACK(node->right->right)) {              /* case 2 */
            RBT_PAINT_RED(node->right);
            status = SB_MORE;
        } else {
            if (RBT_IS_RED(node->right->left)
                && RBT_IS_BLACK(node->right->right)) {          /* case 3 */
                RBT_PAINT_BLACK(node->right->left);
                RBT_PAINT_RED(node->right);
                rbtRotateRight(node->right);
            }
            RBT_PAINT_BLACK(node->right->right);                /* case 4 */
            RBT_SET_COLOR(node->right, RBT_COLOR(node));
            RBT_PAINT_BLACK(node);
            rbtRotateLeft(node);
            status = SB_OK;
        }
    }
    return status;
}

/* requires: node, node->left non-null;
 *           node->right has an extra layer of black paint
 * modifies: tree rooted at node
 * effects:  changes the tree structure to remove or bubble up
 *           the extra layer of black paint after a delete
 * returns:  SB_OK on success when node is red or black
 *           SB_MORE on success when node is doubly black
 */
static
sbStatus_t
rbtDeleteFixRight(rbtNode_p_t node)
{
    sbStatus_t status;

    SB_ASSERT(node);
    SB_ASSERT(node->left); /* else inconsistent black-height */
    if (RBT_IS_RED(node->right)) {                               /* case 0 */
        RBT_PAINT_BLACK(node->right);
        status = SB_OK;
    } else if (RBT_IS_BLACK(node) && RBT_IS_RED(node->left)) { /* case 1 */
        SB_ASSERT(RBT_IS_BLACK(node->left->left));
        SB_ASSERT(RBT_IS_BLACK(node->left->right));
        RBT_PAINT_BLACK(node->left);
        RBT_PAINT_RED(node);
        rbtRotateRight(node);
        status = rbtDeleteFixRight(node->right);
        if (status == SB_MORE) {
            SB_ASSERT(RBT_IS_RED(node->right));
            RBT_PAINT_BLACK(node->right);
            status = SB_OK;
        }
    } else { /* RBT_IS_BLACK(node->left) */
        SB_ASSERT(RBT_IS_BLACK(node->left));
        if (RBT_IS_BLACK(node->left->left)
            && RBT_IS_BLACK(node->left->right)) {              /* case 2 */
            RBT_PAINT_RED(node->left);
            status = SB_MORE;
        } else {
            if (RBT_IS_RED(node->left->right)
                && RBT_IS_BLACK(node->left->left)) {          /* case 3 */
                RBT_PAINT_BLACK(node->left->right);
                RBT_PAINT_RED(node->left);
                rbtRotateLeft(node->left);
            }
            RBT_PAINT_BLACK(node->left->left);                /* case 4 */
            RBT_SET_COLOR(node->left, RBT_COLOR(node));
            RBT_PAINT_BLACK(node);
            rbtRotateRight(node);
            status = SB_OK;
        }
    }
    return status;
}

/* requires: nothing
 * modifies: nothing
 * effects:  none
 * returns:  the minimum node in tree rooted at node, or null if node is null
 */
static
rbtNode_p_t
rbtMinimum(rbtNode_p_t node)
{
    if (node && node->left)
        return rbtMinimum(node->left);
    else
        return node;
}

/* requires: rbt initialized; pNode, *pNode non-null
 * modifies: *pNode
 * effects:  removes node containing key from tree rooted at *pNode
 * returns:  SB_OK on success when *pNode is red or black
 *           SB_MORE on success when *pNode is doubly black
 *           SB_LPM_ADDRESS_NOT_FOUND if key is not in the tree
 */
static
sbStatus_t
rbtDelete(g2p3_lpmRbtHandle_t rbt, rbtNode_p_t *pNode, void *key)
{
    int cmp;  /* key comparison result */
    sbStatus_t status = SB_OK;
    rbtNode_p_t node, min;
    rbtColor_t minColor;

    SB_ASSERT(rbt);
    SB_ASSERT(rbt->cmp);
    SB_ASSERT(pNode);
    SB_ASSERT(*pNode);

    node = *pNode;
    /*    coverity[new_values]    */
    cmp = rbt->cmp(rbt->cmpUserToken, key, node->key);
    cmp = cmp < 0 ? -1 : (cmp > 0 ? 1 : 0);
    if (node->left == NULL && node->right == NULL) { /* no children */
        if (cmp != 0) return SB_LPM_ADDRESS_NOT_FOUND;
        *pNode = NULL;
        status = RBT_IS_RED(node) ? SB_OK : SB_MORE;
    } else if (node->right == NULL) { /* left child only */
        SB_ASSERT(RBT_IS_BLACK(node));
        switch (cmp) {
        case  1: return SB_LPM_ADDRESS_NOT_FOUND;
        case -1: status = rbtDelete(rbt, &node->left, key); 
                 if (status == SB_MORE) status = rbtDeleteFixLeft(node);
                 return status;
        case  0: *pNode = node->left;
                 status = RBT_IS_RED(*pNode) ? SB_OK : SB_MORE;
                 RBT_PAINT_BLACK(*pNode);
                 break;
        /* coverity[dead_error_begin] */
        default: 
            SB_ASSERT(0);
            return SB_FAILED;
        }
    } else if (node->left == NULL) { /* right child only */
        SB_ASSERT(RBT_IS_BLACK(node));
        switch (cmp) {
        case -1: return SB_LPM_ADDRESS_NOT_FOUND;
        case  1: status = rbtDelete(rbt, &node->right, key);
                 if (status == SB_MORE) status = rbtDeleteFixRight(node);
                 return status;
        case  0: *pNode = node->right;
                 status = RBT_IS_RED(*pNode) ? SB_OK : SB_MORE;
                 RBT_PAINT_BLACK(*pNode);
                 break;

        /* coverity[dead_error_begin] */ 
        default: 
            SB_ASSERT(0);
            return SB_FAILED;
        }
    } else { /* both children */
        switch (cmp) {
        case -1: status = rbtDelete(rbt, &node->left, key);
                 if (status == SB_MORE) status = rbtDeleteFixLeft(node);
                 return status;
        case  1: status = rbtDelete(rbt, &node->right, key);
                 if (status == SB_MORE) status = rbtDeleteFixRight(node);
                 return status;
        case  0: min = rbtMinimum(node->right);
                 SB_ASSERT(min);
                 minColor = RBT_COLOR(min);
                 node->key = min->key;
                 node->val = min->val;
                 status = rbtDelete(rbt, &node->right, min->key);
                 SB_ASSERT(status == SB_OK || status == SB_MORE);
                 if (status == SB_MORE) status = rbtDeleteFixRight(node);
                 return status;
        /* coverity[dead_error_begin] */
        default: 
            SB_ASSERT(0);
            return SB_FAILED;
        }
    }
    /* kill deleted node */
    SB_ASSERT(g2p3_lpmHostMemFree(rbt->hostMem, node, sizeof(rbtNode_t))
           == SB_OK);
    return status;
}

/* see prototype for contract */
sbStatus_t
g2p3_lpmRbtInit(g2p3_lpmRbtHandle_t *pRbt,
                    g2p3_lpmHostMemHandle_t hostMem,
                    g2p3_lpmRbtCompare_f_t cmp, void *cmpUserToken)
{
    sbStatus_t status;
    void *vp;

    SB_ASSERT(pRbt);
    SB_ASSERT(cmp);
    *pRbt = NULL; /* in case of early exit */
    status = g2p3_lpmHostMemAlloc(hostMem, &vp, sizeof(g2p3_lpmRbt_t), "lpm rbt mgr");
    if (status != SB_OK) return status;
    (*pRbt) = (g2p3_lpmRbtHandle_t) vp;
    (*pRbt)->root = NULL;
    (*pRbt)->cmp = cmp;
    (*pRbt)->cmpUserToken = cmpUserToken;
    (*pRbt)->hostMem = hostMem;
    return SB_OK;
}

/* requires: nothing
 * modifies: tree rooted at node
 * effects:  frees all memory used by tree rooted at node, inclusive
 * returns:  SB_OK
 */
static
sbStatus_t
rbtCleanup(g2p3_lpmRbtHandle_t rbt, rbtNode_p_t node)
{
    sbStatus_t status;

    if (node) {
        status = rbtCleanup(rbt, node->left);
        SB_ASSERT(status == SB_OK);
        status = rbtCleanup(rbt, node->right);
        SB_ASSERT(status == SB_OK);
        status = g2p3_lpmHostMemFree(rbt->hostMem, node, sizeof(rbtNode_t));
        SB_ASSERT(status == SB_OK);
    }
    return SB_OK;
}

/* see prototype for contract */
sbStatus_t
g2p3_lpmRbtUninit(g2p3_lpmRbtHandle_t *pRbt)
{
    sbStatus_t status;
    g2p3_lpmRbtHandle_t rbt;

    SB_ASSERT(pRbt);
    rbt = *pRbt;
    SB_ASSERT(rbt);

    status = rbtCleanup(rbt, rbt->root);
    SB_ASSERT(status == SB_OK);
    status = g2p3_lpmHostMemFree(rbt->hostMem, rbt,
                                     sizeof(g2p3_lpmRbt_t));
    SB_ASSERT(status == SB_OK);
    *pRbt = NULL;
    return SB_OK;
}
                             
/* see prototype for contract */
sbStatus_t
g2p3_lpmRbtInsert(g2p3_lpmRbtHandle_t rbt, void *key, void *val)
{
    sbStatus_t status;
    SB_ASSERT(rbt);
    status = rbtInsert(rbt, &rbt->root, key, val);
    if (status != SB_OK) return status;
    RBT_PAINT_BLACK(rbt->root);
    return SB_OK;
}

/* see prototype for contract */
sbStatus_t
g2p3_lpmRbtDelete(g2p3_lpmRbtHandle_t rbt, void *key)
{
    sbStatus_t status;
    SB_ASSERT(rbt);
    if (rbt->root == NULL) return SB_LPM_ADDRESS_NOT_FOUND;
    status = rbtDelete(rbt, &rbt->root, key);
    if (status == SB_MORE) {
        RBT_PAINT_BLACK(rbt->root);
        status = SB_OK;
    }
    return status;
}

/* see prototype for contract */
sbStatus_t
g2p3_lpmRbtFind(g2p3_lpmRbtHandle_t rbt, void **pVal, void *key)
{
    rbtNode_p_t node;
    SB_ASSERT(rbt);
    SB_ASSERT(rbt->cmp);
    node = rbt->root;
    /*    coverity[new_values]    */
    while (node) {
        int cmp = rbt->cmp(rbt->cmpUserToken, key, node->key);
        cmp = cmp < 0 ? -1 : (cmp > 0 ? 1 : 0);
        switch (cmp) {
        case  0: *pVal = node->val;
                 return SB_OK;
        case -1: node = node->left;
                 break;
        case  1: node = node->right;
                 break;
        /* coverity[dead_error_begin] */
        default: 
            SB_ASSERT(0);
            return SB_FAILED;
        }
    }
    return SB_LPM_ADDRESS_NOT_FOUND;
}

/* see prototype for contract */
sbBool_t
g2p3_lpmRbtIsEmpty(g2p3_lpmRbtHandle_t rbt)
{
    SB_ASSERT(rbt);
    return (rbt->root == NULL);
}
