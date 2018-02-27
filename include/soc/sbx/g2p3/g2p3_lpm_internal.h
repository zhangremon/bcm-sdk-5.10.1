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

  $Id: g2p3_lpm_internal.h 1.6.158.1 Broadcom SDK $

 *****************************************************************************/

#ifndef _G2P3_LPM_I_H_
#define _G2P3_LPM_I_H_

#include "g2p3_lpm.h"

/* LPM REPRESENTATION IN HOST MEMORY
 *
 * A LPM object is a trie of nodes, each of which maybe a payload node,
 * a split node, a skip node, or a table node.
 *
 * - a PAYLOAD, which may be of one of three kinds (main payload,
 *   class ID A, class ID B), holds a reference to a payload or class ID
 *   for nodes below it
 * - a TABLE holds a reference to a "slice table" in device memory;
 *   this may be of size 2^(# slice bits) or 2 if the table is a singleton
 * - a SPLIT node has a child for addresses with the next bit set to 0 and
 *   a child for addresses with the next bit set to 1
 * - a SKIP node represents a stem of next skipLen+1 bits of the address;
 *   addresses matching the stem are represented by nodes below, while
 *   addresses not matching use nearest enclosing payload and class IDs
 *   SKIP nodes are a host memory optimization.
 *
 *
 * NEW/LIVE/DEAD nodes
 *
 * Payload and table nodes are marked NEW when first created, LIVE when first
 * committed, and DEAD when removed; this is required to keep the device memory
 * changes while committing to a minimum.  A DEAD node never turns into
 * a LIVE or NEW node.
 *
 *
 * DEVICE MEMORY REFERENCES
 *
 * Table nodes (and, indirectly, payload nodes) refer to addresses in
 * device memory.  To preserve atomicity, device memory areas are not reused
 * until the tree has been committed to device memory and no references to
 * these areas remain.  (Consequently, more routes fit if commits are frequent).
 *
 *
 * SINGLETON TABLES
 *
 * If the singleton table optimization is enabled, tables which hold
 * a single path to a terminal payload or to another table can be compressed
 * into two entries: "matching" and "non-matching".  Compressed tables are
 * created and tables decompressed during route addition, and tables
 * recompressed during route removal.
 *
 *
 * REPRESENTATION INVARIANTS
 * - SKIP nodes:
 *   + child is not NULL
 *   + child is not a SKIP node
 *   + 0 < skip amount <= 32
 *   + skip address has no more bits than skip amount
 *   + dpsBelow equals child's dpsBelow
 *   + cannot be dead or new
 * - SPLIT nodes:
 *   + neither child is NULL
 *   + dpsBelow equals sum of children's dpsBelow
 *   + cannot be dead or new
 * - TABLE nodes:
 *   + child is not NULL
 *   + table pointer is not NULL
 *   + dpsBelow equals child's dpsBelow
 *   + singleton only if singleton optimization enabled in LPM object
 * - SLICES:
 *   + zero terminated
 *   + each <= 32
 *   + sum == address size in bits
 *   + sum <= 64
 * - INTERNODE
 *   + tables only between slices
 *   + a PAYLOAD may not immediately follow a TABLE
 *   + a live or new PAYLOAD or TABLE may not immediately follow a dead one
 *   + no two PAYLOADs of the same kind at the same prefix
 *   + payloads of lesser kind must come before payloads of higher kind
 *
 */

typedef struct lpmNode_s lpmNode_t, *lpmNode_p_t;

typedef enum {
    LPM_NODE_PAYLOAD,
    LPM_NODE_TABLE,
    LPM_NODE_SPLIT,
    LPM_NODE_SKIP,
    LPM_NODE_NUM_KINDS /* keep as last entry */
} lpmNodeType_t;

typedef enum {
    LPM_NODE_STATE_NEW,  /* (payload/table) not yet committed to device mem */
    LPM_NODE_STATE_LIVE, /* (payload/table) already committed to device mem */
    LPM_NODE_STATE_DEAD, /* (payload/table) removed -- clean up at commit() */
    LPM_NODE_NUM_STATES  /* keep as last entry */
} lpmNodeState_t;

typedef struct {
    unsigned int type:2;
    unsigned int state:2;        /* commit state: - LIVE for all,
                                  *               - NEW for payloads/tables
                                  *               - DEAD for tables */
    unsigned int singleton:1;    /* for tables */
    unsigned int payloadKind:3;  /* payload kind, for payload node */
    unsigned int skipLen:5;	 /* length - 1, for skip node */
} lpmNodeTag_t;

typedef struct {
    void *desc;			/* payload description TBD */
    lpmNode_p_t child;
} lpmNodePayload_t;

/*
 * a cleft in the tree structure: eat up most significant bit of address
 * and use it to pick which child to follow
 */
typedef struct {
    lpmNode_p_t children[2];
} lpmNodeSplit_t;

/*
 * a device-memory tree slice starts here; nodes below are entries in this
 * table
 */
typedef struct {
    g2p3_lpmSramAddr address; /* in the device memory */
    lpmNode_p_t child;
} lpmNodeTable_t;

/*
 * a path shared by all children
 */
typedef struct {
    uint32_t address;		/* match this address; skip amount
				 * embedded in tag */
    lpmNode_p_t child;
} lpmNodeSkip_t;

struct lpmNode_s {
    lpmNodeTag_t tag;
    uint32_t dpsBelow; /* # of <main,clsA,clsB> payload at or below;
                        * irrelevant for dead nodes */
    union {
	lpmNodeSplit_t split;
	lpmNodePayload_t payload;
	lpmNodeTable_t table;
	lpmNodeSkip_t skip;
    } data;
};

typedef struct lpmIntStats_s {
    uint64_t counter[G2P3_LPM_STATS_UPD_MAX]\
                    [G2P3_LPM_STATS_TYPE_MAX]\
                    [G2P3_LPM_STATS_MAX_IDX];
} lpmIntStats_t, *lpmIntStats_p_t;

typedef struct {
    lpmNode_p_t node;                    /* next to be traversed */
    g2p3_lpmSramAddr dest;               /* address in table */
    sbBool_t dirty[G2P3_LPM_PAYLOAD_NUM_KINDS];  /* closest enclosing payload not committed */
    sbBool_t dirtyTable;                 /* closest enclosing table is new */
    uint8_t slice;                       /* # bits left in current slice */
    uint8_t *slices;                     /* rest of slices */
    /* payloads */
    g2p3_lpmPayloadHandle_t payload;     /* nearest enclosing */
    g2p3_lpmClassifierId_t clsA;         /* nearest enclosing */
    g2p3_lpmClassifierId_t clsB;         /* nearest enclosing */
    g2p3_lpmClassifierId_t clsC;
    uint32_t dropMaskLo;
    uint32_t dropMaskHi;
    g2p3_lpmSramAddr payloadDevAddr;     /* payload address in FE mem        */
    uint16_t payloadNodeExtra;           /* extra ten bits in payload node   */
    uint8_t seen;                        /* # of times this node was seen    */
    /* the following members are used by prune dead based on prefix */
    uint8_t child;                       /* is this split child 0 or 1       */
    g2p3_lpmAddress_t addr;              /* Prefix Address                   */
    uint8_t length;                      /* Prefix Length                    */
    lpmNode_p_t parent;                  /* Pointer to parent                */
} lpmPath_t, *lpmPath_p_t;

typedef struct g2p3_lpm_s {
    soc_sbx_g2p3_state_t *fe; 
    uint8_t addrBits;                    /* total number of address bits */
    uint8_t *slices;                     /* address slicing, zero-terminated */
    sbBool_t optSingleton;          /* enable singleton tables */
    g2p3_lpmHostMemHandle_t hostMem; /* host memory */
    size_t hostAllocBytes;               /* # bytes allocated in hostMem */
    g2p3_lpmDeviceMemHandle_t deviceMemA; /* device memory A */
    g2p3_lpmDeviceMemHandle_t deviceMemB; /* device memory B */
    g2p3_lpmHPMHandle_t hpm;         /* host payload manager */
    g2p3_lpmDPMHandle_t dpm;         /* device payload manager */
    g2p3_lpmDmaHandle_t dma;         /* DMA manager */
    uint32_t dmaBankA;                    /* DMA bank to write to */
    uint32_t dmaBankB;
    uint32_t payloadSize;            /* buffer size, in bytes, passed to/from
                                      * LPM compiler */
    /* prefix tree rep */
    lpmNode_p_t trie;                    /* route trie */
    /* default payloads */
    g2p3_lpmPayloadHandle_t defaultPayload; /* default main payload */
    g2p3_lpmClassifierId_t defaultClsA;   /* default class A */
    g2p3_lpmClassifierId_t defaultClsB;   /* default class B */
    uint32_t defaultDropMaskLo;
    uint32_t defaultDropMaskHi;
    g2p3_lpmSramAddr topTableAddress;   /* level 1 table in device memory */
    /* for use during commit() */
    sbStatus_t status;              /* of last asynchronous operation */
    sbBool_t busy;                  /* operation in progress */
    sbBool_t async;                 /* asynchronous completion required */
    g2p3_lpmCallback_f_t cb;        /* asynchronous completion callback */
    void *cbUserData;               /* asynchronous callback user data */
    lpmPath_p_t path;               /* stack of nodes to be traveled */
    uint8_t pathLength;             /* # saved nodes;
                                       >0 => adds/removes prohibited */
    uint8_t maxPathLength;          /* max pathLength */
    sbBool_t resetCommit;           /* true iff next commit() should write
                                            all (default: incremental) */
    sbBool_t ipv6;                  /* distinguish between ipv4 and ipv6
                                     * worlds - have to do it when
                                     *  comitting leaf trie entries */
    lpmIntStats_t stats;
    void *lpmWorkMgr;
} g2p3_lpm_t; /* g2p3_lpmHandle_t in g2p3_lpm.h */

typedef struct {
    lpmNode_p_t node;
    g2p3_lpmAddress_t addr;
    uint8_t len;           /* length in bits of slc + address  */
    uint8_t parent:2;      /* is this a child of a split node ? */
    uint8_t seen:1;        /* have we seen this node (1) or not (0) */
    uint8_t indent;        /* display indentation */
} g2p3_lpmWalkerAsst_t,*g2p3_lpmWalkAsst_p_t;


#define G2P3_INTERNAL_MAX_KEY 64
typedef struct {
    int             initdone;
    uint32_t        pathLength;
    g2p3_lpmWalkerAsst_t walkTree[G2P3_INTERNAL_MAX_KEY + 1];
    g2p3_lpmWalkAsst_p_t path;
    g2p3_lpmAddress_t prevlpm;
    uint64_t           prevlen;
    g2p3_lpmWalkHandle_t pHandle;
    sbBool_t           dest;
} g2p3_lpmWalkDesc_t, *g2p3_lpmWalkDesc_p_t;


#endif
