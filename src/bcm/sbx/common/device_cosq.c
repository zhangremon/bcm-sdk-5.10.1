/*
 * $Id: device_cosq.c 1.6 Broadcom SDK $
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
 * File:        device_cosq.c
 * Purpose:     Implement Hybrid resource allocation algorithm. This is
 *              shared across devices.
 */


#include <soc/sbx/sbTypes.h>
#include <soc/cm.h>
#include <soc/sbx/sbx_drv.h>
#include <bcm/cosq.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/device_cosq.h>
#ifdef BCM_WARM_BOOT_SUPPORT
#include <bcm_int/sbx/state.h>
#endif /* BCM_WARM_BOOT_SUPPORT */
#include <bcm/debug.h>
#include <bcm/error.h>
#include <shared/alloc.h>


static int init_done = FALSE;
static int numNodes = 0;
static int numPorts = 0;
static int isExtendedEset = FALSE;
static int isIndependentFc = FALSE;


static int              *lnaMap[SOC_MAX_NUM_DEVICES];
static sysport_info_t   *sysportInfo[SOC_MAX_NUM_DEVICES];
static bucket_info_t    *bucketInfo[SOC_MAX_NUM_DEVICES];
static node_info_t      *nodeInfo[SOC_MAX_NUM_DEVICES];

static int *sysportMap[SOC_MAX_NUM_DEVICES];            /* TODO: Remove */
static int *sysportBucketMap[SOC_MAX_NUM_DEVICES];      /* TODO: Remove */


static void
_bcm_sbx_device_cosq_sysport_bucket_update(int unit, int sysport, int node, int port)
{
    int bucket = BCM_SBX_DEVICE_COSQ_SET_SYSPORT_BUCKET(sysport);
    int nodeDiv = (node / 8);
    int nodeMod = (node % 8);


    bucketInfo[unit][bucket].depth++;
    if (node != -1 ) {
        bucketInfo[unit][bucket].nodes_allocated[nodeDiv] |= (1 << nodeMod);
    }
}

static int
_bcm_sbx_device_cosq_sysport_is_bucket_allocated(int unit, int sysport)
{
    int bucket = BCM_SBX_DEVICE_COSQ_SET_SYSPORT_BUCKET(sysport);
    int is_allocated = FALSE;


    is_allocated = (bucketInfo[unit][bucket].depth > 0) ? TRUE : FALSE;
    return(is_allocated);
}

static int
_bcm_sbx_device_cosq_verify_sysport_alloc(int unit, int sysport, int node)
{
    int bucket = BCM_SBX_DEVICE_COSQ_SET_SYSPORT_BUCKET(sysport);
    int nodeDiv = (node / 8);
    int nodeMod = (node % 8);
    int is_allocated = FALSE;


    if (sysportInfo[unit][sysport].used == TRUE) {
        return(TRUE);
    }

    if ( (sysportInfo[unit][sysport].state == BCM_INT_SBX_SYSPORT_STATE_LOCAL) ||
            (sysportInfo[unit][sysport].state == BCM_INT_SBX_SYSPORT_STATE_RESERVED_ESET) ||
            (sysportInfo[unit][sysport].state == BCM_INT_SBX_SYSPORT_STATE_RESERVED_FIFO_FC) ) {
        return(TRUE);
    }

    is_allocated = (bucketInfo[unit][bucket].nodes_allocated[nodeDiv] & (1 << nodeMod)) ?
                                                                              TRUE : FALSE;

    return(is_allocated);
}

static void
_bcm_sbx_device_cosq_node_info_update(int unit, int node, int port, int sysport)
{
    int portDiv = (port / 8);
    int portMod = (port % 8);


    if (node != -1 ) {
        nodeInfo[unit][node].ports_allocated[portDiv] |= (1 << portMod);
    }
}

static int
_bcm_sbx_device_cosq_node_info_port_is_allocated(int unit, int node, int port)
{
    int portDiv = (port / 8);
    int portMod = (port % 8);
    int is_allocated = FALSE;


    is_allocated = (nodeInfo[unit][node].ports_allocated[portDiv] & (1 << portMod)) ? TRUE : FALSE;

    return(is_allocated);
}

void
_bcm_sbx_device_cosq_sysport_alloc_update(int unit, int sysPort, int node, int port)
{
    sysportInfo[unit][sysPort].state = BCM_INT_SBX_SYSPORT_STATE_GLOBAL;
    sysportInfo[unit][sysPort].bucket = BCM_SBX_DEVICE_COSQ_SET_SYSPORT_BUCKET(sysPort);
    sysportInfo[unit][sysPort].sysport = sysPort;
    sysportInfo[unit][sysPort].used = TRUE;
    sysportInfo[unit][sysPort].node = node;
    sysportInfo[unit][sysPort].port = port;
    sysportMap[unit][sysPort] = BCM_INT_SBX_SYSPORT_STATE_GLOBAL;

    _bcm_sbx_device_cosq_sysport_bucket_update(unit, sysPort, node, port);
    _bcm_sbx_device_cosq_node_info_update(unit, node, port, sysPort);
}

/* static */ void
_bcm_sbx_device_cosq_sysport_general_initialize(int unit, int nNode, int nPort)
{
    int no_sysports;
    int no_sysports_alloc = 0;
    int node, port, sysPort;


    no_sysports = nNode * nPort;

    /* Reserve sysports that are to be used for ESETS */

    /*
     * allocate required number of sysports
     */
    /* first allocate sysports that do no overlay with lna space */
    /* These can always be reserved                              */
    for (node = (BCM_SBX_DEVICE_COSQ_MAX_LNA_NODE + 1);
                           node <= BCM_SBX_DEVICE_COSQ_MAX_SYSPORT_NODE; node++) {
        for (port = BCM_SBX_DEVICE_COSQ_MIN_SYSPORT_PORT;
                           port <= BCM_SBX_DEVICE_COSQ_MAX_SYSPORT_PORT; port++) {
            sysPort = BCM_SBX_DEVICE_COSQ_SET_SYSPORT(node, port);
            if (sysPort >= BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS) {
                break;
            }

            sysportMap[unit][sysPort] = BCM_INT_SBX_SYSPORT_STATE_GLOBAL;
            no_sysports_alloc++;
        }

        if (sysPort >= BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS) {
            break;
        }
    }
    BCM_DEBUG(BCM_DBG_COSQ, ("SysPortsAllocated: %d\n", no_sysports_alloc));

    for (node = BCM_SBX_DEVICE_COSQ_MIN_LNA_NODE;
                          node <= BCM_SBX_DEVICE_COSQ_MAX_LNA_NODE; node++) {
        for (port = (BCM_SBX_DEVICE_COSQ_MAX_LNA_PORT + 1);
                          port <= BCM_SBX_DEVICE_COSQ_MAX_SYSPORT_PORT; port++) {
            sysPort = BCM_SBX_DEVICE_COSQ_SET_SYSPORT(node, port);
            if (sysPort >= BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS) {
                break;
            }

            sysportMap[unit][sysPort] = BCM_INT_SBX_SYSPORT_STATE_GLOBAL;
            no_sysports_alloc++;
        }

        if (sysPort >= BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS) {
            break;
        }
    }
    BCM_DEBUG(BCM_DBG_COSQ, ("SysPortsAllocated: %d\n", no_sysports_alloc));

}

static void
_bcm_sbx_device_cosq_sysport_initialize(int unit, int nNode, int nPort)
{
    int no_sysports;
    int no_sysports_alloc = 0;
    int node, port, sysPort;
    int curNode, curPort;
    int is_allocated = FALSE;


    no_sysports = nNode * nPort;

    /*
     * allocate required number of sysports
     */

    /*
     * In the first pass go through the sysport range that does not
     * overlap with the LNA space. Some of the LNA space will still get
     * wasted as there wil be overlap on the Egress side. This will be
     * minimized by allocating all nodes from  as few buckets as possible.
     * This space cannot be always marked as resrved for faric flows
     * because of the overlap that occurs on the Egress side even when
     * there is no overlap in the ingress end with the LNA space.
     * Allocate a port on each node. This way we are trying to minimize
     * the overlap that may occur on the egress.
     */

    /* Allocate sysports(node:port overlay) in the following range. */
    /*   node => (BCM_SBX_DEVICE_COSQ_MIN_LNA_NODE) - (BCM_SBX_DEVICE_COSQ_MAX_LNA_NODE)         */
    /*   port => (BCM_SBX_DEVICE_COSQ_MAX_LNA_PORT + 1) - (BCM_SBX_DEVICE_COSQ_MAX_SYSPORT_PORT) */
    for (curPort = 0; curPort < nPort; curPort++) {
        for (curNode = 0; curNode < nNode; curNode++) {
            is_allocated = _bcm_sbx_device_cosq_node_info_port_is_allocated(unit, curNode, curPort);
            for (port = (BCM_SBX_DEVICE_COSQ_MAX_LNA_PORT + 1);
                              (port <= BCM_SBX_DEVICE_COSQ_MAX_SYSPORT_PORT) &&
                                                     (is_allocated != TRUE); port++) {
                for (node = BCM_SBX_DEVICE_COSQ_MIN_LNA_NODE ;
                            (node <= BCM_SBX_DEVICE_COSQ_MAX_LNA_NODE) &&
                                                     (is_allocated != TRUE); node++) {
                    sysPort = BCM_SBX_DEVICE_COSQ_SET_SYSPORT(node, port);
                    if (sysPort >= BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS) {
                        break;
                    }
                    if (_bcm_sbx_device_cosq_verify_sysport_alloc(unit, sysPort, curNode)) {
                        continue;
                    }
                    _bcm_sbx_device_cosq_sysport_alloc_update(unit, sysPort, curNode, curPort);
                    is_allocated = TRUE;

                    BCM_DEBUG(BCM_DBG_COSQ,
                          (" SysPort: %d CurNode:CurPort (%d:%d) Node:Port (%d:%d)\n",
                                             sysPort, curNode, curPort, node, port));
                    no_sysports_alloc++;
                }
            }
        }
    }

    /* Allocate sysports(node:port overlay) in the following range. */
    /*   node => (BCM_SBX_DEVICE_COSQ_MAX_LNA_NODE + 1) - (BCM_SBX_DEVICE_COSQ_MAX_SYSPORT_NODE) */
    /*   port => (BCM_SBX_DEVICE_COSQ_MAX_LNA_PORT + 1) - (BCM_SBX_DEVICE_COSQ_MAX_SYSPORT_PORT) */
    for (curPort = 0; curPort < nPort; curPort++) {
        for (curNode = 0; curNode < nNode; curNode++) {
            is_allocated = _bcm_sbx_device_cosq_node_info_port_is_allocated(unit, curNode, curPort);
            for (port = (BCM_SBX_DEVICE_COSQ_MAX_LNA_PORT + 1);
                            (port <= BCM_SBX_DEVICE_COSQ_MAX_SYSPORT_PORT) &&
                                                    (is_allocated != TRUE); port++) {
                for (node = (BCM_SBX_DEVICE_COSQ_MAX_LNA_NODE + 1);
                            (node <= BCM_SBX_DEVICE_COSQ_MAX_SYSPORT_NODE) &&
                                                    (is_allocated != TRUE); node++) {
                    sysPort = BCM_SBX_DEVICE_COSQ_SET_SYSPORT(node, port);
                    if (sysPort >= BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS) {
                        break;
                    }
                    if (_bcm_sbx_device_cosq_verify_sysport_alloc(unit, sysPort, curNode)) {
                        continue;
                    }
                    _bcm_sbx_device_cosq_sysport_alloc_update(unit, sysPort, curNode, curPort);
                    is_allocated = TRUE;

                    BCM_DEBUG(BCM_DBG_COSQ,
                          ("  SysPort: %d CurNode:CurPort (%d:%d) Node:Port (%d:%d)\n",
                                             sysPort, curNode, curPort, node, port));
                    no_sysports_alloc++;
                }
            }
        }
    }
    for (is_allocated = FALSE, port = (BCM_SBX_DEVICE_COSQ_MAX_LNA_PORT + 1);
                            (port <= BCM_SBX_DEVICE_COSQ_MAX_SYSPORT_PORT) &&
                                                   (is_allocated != TRUE); port++) {
        for (node = (BCM_SBX_DEVICE_COSQ_MAX_LNA_NODE + 1);
                            (node <= BCM_SBX_DEVICE_COSQ_MAX_SYSPORT_NODE) &&
                                                   (is_allocated != TRUE); node++) {
            sysPort = BCM_SBX_DEVICE_COSQ_SET_SYSPORT(node, port);
            if (sysPort >= BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS) {
                break;
            }
            if (_bcm_sbx_device_cosq_sysport_is_bucket_allocated(unit, sysPort)) {
                BCM_DEBUG(BCM_DBG_COSQ, ("  SysPort: %d marked for Fabric Flow as corresponding LNA allocated\n", sysPort));
                _bcm_sbx_device_cosq_sysport_alloc_update(unit, sysPort, -1, -1);
            }
        }
    }

    /* Allocate sysports(node:port overlay) in the following range. */
    /*   node => (BCM_SBX_DEVICE_COSQ_MAX_LNA_NODE + 1) - (BCM_SBX_DEVICE_COSQ_MAX_SYSPORT_NODE) */
    /*   port => (BCM_SBX_DEVICE_COSQ_MIN_SYSPORT_PORT) - (BCM_SBX_DEVICE_COSQ_MAX_LNA_PORT)     */
    for (curPort = 0; curPort < nPort; curPort++) {
        for (curNode = 0; curNode < nNode; curNode++) {
            is_allocated = _bcm_sbx_device_cosq_node_info_port_is_allocated(unit, curNode, curPort);
            for (port = BCM_SBX_DEVICE_COSQ_MIN_SYSPORT_PORT;
                            (port <= BCM_SBX_DEVICE_COSQ_MAX_LNA_PORT) &&
                                                 (is_allocated != TRUE); port++) {
                for (node = (BCM_SBX_DEVICE_COSQ_MAX_LNA_NODE + 1);
                            (node <= BCM_SBX_DEVICE_COSQ_MAX_SYSPORT_NODE) &&
                                                 (is_allocated != TRUE); node++) {
                    sysPort = BCM_SBX_DEVICE_COSQ_SET_SYSPORT(node, port);
                    if (sysPort >= BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS) {
                        break;
                    }
                    if (_bcm_sbx_device_cosq_verify_sysport_alloc(unit, sysPort, curNode)) {
                        continue;
                    }
                    _bcm_sbx_device_cosq_sysport_alloc_update(unit, sysPort, curNode, curPort);
                    is_allocated = TRUE;

                    BCM_DEBUG(BCM_DBG_COSQ,
                          ("   SysPort: %d CurNode:CurPort (%d:%d) Node:Port (%d:%d)\n",
                                             sysPort, curNode, curPort, node, port));
                    no_sysports_alloc++;
                }
            }
        }
    }

    /* check if additional resources need to be reserved            */
    /*   node => (BCM_SBX_DEVICE_COSQ_MIN_LNA_NODE) - (BCM_SBX_DEVICE_COSQ_MAX_LNA_NODE)         */
    /*   port => (BCM_SBX_DEVICE_COSQ_MIN_SYSPORT_PORT) - (BCM_SBX_DEVICE_COSQ_MAX_LNA_PORT)     */
    for (curPort = 0; curPort < nPort; curPort++) {
        for (curNode = 0; curNode < nNode; curNode++) {
            is_allocated = _bcm_sbx_device_cosq_node_info_port_is_allocated(unit, curNode, curPort);
            for (port = (BCM_SBX_DEVICE_COSQ_MIN_SYSPORT_PORT);
                            (port <= BCM_SBX_DEVICE_COSQ_MAX_LNA_PORT) &&
                                                      (is_allocated != TRUE); port++) {
                for (node = BCM_SBX_DEVICE_COSQ_MIN_LNA_NODE ;
                            (node <= BCM_SBX_DEVICE_COSQ_MAX_LNA_NODE) &&
                                                      (is_allocated != TRUE); node++) {
                    sysPort = BCM_SBX_DEVICE_COSQ_SET_SYSPORT(node, port);
                    if (sysPort >= BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS) {
                        break;
                    }
                    if (_bcm_sbx_device_cosq_verify_sysport_alloc(unit, sysPort, curNode)) {
                        continue;
                    }
                    _bcm_sbx_device_cosq_sysport_alloc_update(unit, sysPort, curNode, curPort);
                    is_allocated = TRUE;

                    BCM_DEBUG(BCM_DBG_COSQ,
                          ("    SysPort: %d CurNode:CurPort (%d:%d) Node:Port (%d:%d)\n",
                                             sysPort, curNode, curPort, node, port));
                    no_sysports_alloc++;
                }
            }
        }
    }

    BCM_DEBUG(BCM_DBG_COSQ, ("\n"));
}

static void
_bcm_sbx_device_cosq_fill_lna_map_Q2EC(int unit, int nNode, int nPort, int is_update)
{
    int         lnaPort, sysPort;
    int         curPort, curNode;
    int         no_lna_alloc = 0;


    for (sysPort = 0; sysPort < BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS; sysPort++) {
        if (sysportInfo[unit][sysPort].state == BCM_INT_SBX_SYSPORT_STATE_GLOBAL) {
            curPort = BCM_SBX_DEVICE_COSQ_GET_PORT_FROM_SYSPORT(sysPort);
            curNode = BCM_SBX_DEVICE_COSQ_GET_NODE_FROM_SYSPORT(sysPort);
            if ((curNode >= (BCM_SBX_DEVICE_COSQ_MAX_LNA_NODE + 1)) ||
                                  (curPort >= (BCM_SBX_DEVICE_COSQ_MAX_LNA_PORT + 1))) {
                continue;
            }
            else {
                lnaPort = BCM_SBX_DEVICE_COSQ_SET_LNA(curNode, curPort);

                if ((is_update == TRUE) && (lnaMap[unit][lnaPort] == TRUE)) {
                    continue;
                }

                lnaMap[unit][lnaPort] = TRUE;
                no_lna_alloc++;
                BCM_DEBUG(BCM_DBG_COSQ, ("LnaPort: %d, sysPort: %d, curNode: %d, curPort: %d\n",
                                     lnaPort, sysPort, curNode, curPort));
            }
        }
    }
    BCM_DEBUG(BCM_DBG_COSQ, ("Ingress LnaAllocated: %d\n\n", no_lna_alloc));
}

static void
_bcm_sbx_device_cosq_fill_lna_map_port_remap(int unit, int nNode, int nPort, int is_update)
{
    int         sysPort;
    int         PortRemapIndex;
    int         curPort, qe1kFlag, efFlag;
    int         newHits = 0, oldHits = 0;


    for (sysPort = 0; sysPort < BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS; sysPort++) {
        if (sysportInfo[unit][sysPort].state == BCM_INT_SBX_SYSPORT_STATE_GLOBAL) {
            curPort = BCM_SBX_DEVICE_COSQ_GET_PORT_FROM_SYSPORT(sysPort);

            for (qe1kFlag = 0; qe1kFlag < 1; qe1kFlag++) {
                for (efFlag = 0; efFlag < 2; efFlag++) {
                    PortRemapIndex = BCM_SBX_DEVICE_COSQ_SET_PORT_REMAP_INDEX(curPort,
                                                                           qe1kFlag, efFlag);
                    if (is_update != TRUE) {
                        (lnaMap[unit][PortRemapIndex] == TRUE) ? oldHits++ : newHits++;
                    }
                    lnaMap[unit][PortRemapIndex] = TRUE;
                }
            }
        }
    }

    if (is_update != TRUE) {
        BCM_DEBUG(BCM_DBG_COSQ, ("PortRemap, oldHits: %d newHits: %d\n\n", oldHits, newHits));
    }
}

static int
_bcm_sbx_device_cosq_get_lna_map_available_entries(int unit)
{
    int         lnaPort;
    int         lnaFree = 0, lnaOccupied = 0;


    for (lnaPort = 0; lnaPort < BCM_SBX_DEVICE_COSQ_MAX_LNAS; lnaPort++) {
        (lnaMap[unit][lnaPort] == TRUE) ? lnaOccupied++ : lnaFree++;
    }

    BCM_DEBUG(BCM_DBG_COSQ, ("lnaFree: %d lnaOccupied: %d\n\n", lnaFree, lnaOccupied));

    return(lnaFree);
}

static void
_bcm_sbx_device_cosq_det_sysport_available_entries(int unit)
{
    int         sysPort, num_sysports = 0;


    for (sysPort = 0; sysPort < BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS; sysPort++) {
        if (sysportInfo[unit][sysPort].state == BCM_INT_SBX_SYSPORT_STATE_GLOBAL) {
            num_sysports++;
        }
    }

    BCM_DEBUG(BCM_DBG_COSQ, ("SysPorts: %d\n\n", num_sysports));
}

static void
_bcm_sbx_device_cosq_det_sysport_bucket_resources(int unit)
{
    int         sysPort, bucket;



    for (sysPort = 0; sysPort < BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS; sysPort++) {
        if (sysportInfo[unit][sysPort].state == BCM_INT_SBX_SYSPORT_STATE_GLOBAL) {
            bucket = sysPort % BCM_SBX_DEVICE_COSQ_SYSPORT_NO_BUCKETS;
            sysportBucketMap[unit][bucket]++;
        }
    }

    for (bucket = 0; bucket < BCM_SBX_DEVICE_COSQ_SYSPORT_NO_BUCKETS; bucket++) {
        BCM_DEBUG(BCM_DBG_COSQ,
                   ("SysPortBucket: %d, Resources: %d\n", bucket, sysportBucketMap[unit][bucket]));

        if (sysportBucketMap[unit][bucket] != bucketInfo[unit][bucket].depth) {
            BCM_DEBUG(BCM_DBG_COSQ,
                   ("    *** MISMATCH SysPortBucket: %d, Resources: %d:%d\n",
                        bucket, sysportBucketMap[unit][bucket], bucketInfo[unit][bucket].depth));
        }
    }

    BCM_DEBUG(BCM_DBG_COSQ, ("\n"));
}

/* determine if additional LNA resources can be allocated */
static void
_bcm_sbx_device_cosq_allocate_additional_sysports(int unit, int nNode, int nPort)
{
    int no_queues = 0, no_lnas = 0;
    int lnaFree = 0, lnaOccupied = 0;

    /* No LNAs that can be allocated, according to the FABRIC Queues */
    no_queues = nNode * nPort * BCM_SBX_DEVICE_COSQ_NO_COS_LEVELS;
    no_lnas = no_queues / BCM_SBX_DEVICE_COSQ_NO_QUEUES_MANAGED_BY_LNA;

    /* no LNA allocated */
    while (TRUE) {
        lnaFree = _bcm_sbx_device_cosq_get_lna_map_available_entries(unit);
        lnaOccupied = (BCM_SBX_DEVICE_COSQ_MAX_LNAS - lnaFree);

        if (no_lnas == BCM_SBX_DEVICE_COSQ_MAX_LNAS) {
            return;
        }

        if (no_lnas >= lnaOccupied) {
            return;
        }

        /* allocate aditional LNAs */
    }

    return;
}

void
_bcm_sbx_device_cosq_init(int unit)
{
    int sysPort;
    int i;


    if (init_done == FALSE) {
        for (i = 0; i < 32; i++) {
            if (SOC_SBX_CFG(unit)->cfg_node_00_31_mask & (1 << i)) {
                numNodes++;
            } 
        }
        for (i = 0; i < 32; i++) {
            if (SOC_SBX_CFG(unit)->cfg_node_32_63_mask & (1 << i)) {
                numNodes++;
            }
        }
        for (i = 0; i < 32; i++) {
            if (SOC_SBX_CFG(unit)->cfg_node_64_95_mask & (1 << i)) {
                numNodes++;
            }
        }
        numPorts = SOC_SBX_CFG(unit)->nMaxFabricPortsOnModule;
        isExtendedEset = SOC_SBX_CFG(unit)->use_extended_esets;
        isIndependentFc = (soc_feature(unit, soc_feature_egr_independent_fc)) ? TRUE : FALSE;

        init_done = TRUE;
    }

    /* zero out lna map */
    lnaMap[unit] = sal_alloc(sizeof(int) * BCM_SBX_DEVICE_COSQ_MAX_LNAS, "Lna Map");
    memset(lnaMap[unit], 0, (sizeof(int) * BCM_SBX_DEVICE_COSQ_MAX_LNAS));

    /* zero out sysport map */
    sysportMap[unit] = sal_alloc(sizeof(int) * BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS, "Sysport Map");
    memset(sysportMap[unit], 0, (sizeof(int) * BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS));

    sysportBucketMap[unit] = sal_alloc(sizeof(int) * BCM_SBX_DEVICE_COSQ_SYSPORT_NO_BUCKETS,
                                                                         "Sysport Bucket Map");
    memset(sysportBucketMap[unit], 0, (sizeof(int) * BCM_SBX_DEVICE_COSQ_SYSPORT_NO_BUCKETS));

    sysportInfo[unit] = sal_alloc(sizeof(sysport_info_t) * BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS,
                                                                         "Sysport Info");
    memset(sysportInfo[unit], 0, (sizeof(sysport_info_t) * BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS));

    bucketInfo[unit] = sal_alloc(sizeof(bucket_info_t) * BCM_SBX_DEVICE_COSQ_SYSPORT_NO_BUCKETS,
                                                                         "Bucket Info");
    memset(bucketInfo[unit], 0, (sizeof(bucket_info_t) * BCM_SBX_DEVICE_COSQ_SYSPORT_NO_BUCKETS));

    nodeInfo[unit] = sal_alloc(sizeof(node_info_t) * BCM_SBX_DEVICE_COSQ_MAX_USER_NODE_NO,
                                                                         "Node Info");
    memset(nodeInfo[unit], 0, (sizeof(node_info_t) * BCM_SBX_DEVICE_COSQ_MAX_USER_NODE_NO));

    for (sysPort = 0; sysPort < BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS; sysPort++) {
        sysportInfo[unit][sysPort].sysport = sysPort;
        sysportInfo[unit][sysPort].bucket = BCM_SBX_DEVICE_COSQ_SET_SYSPORT_BUCKET(sysPort);
    }

    /* determine if extended esets are being used */
    if (isExtendedEset == TRUE) {
        for (sysPort = 0; sysPort < BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS; sysPort++) {
            if ( (sysportInfo[unit][sysPort].bucket >=
                                 BCM_SBX_DEVICE_COSQ_EXTENDED_ESET_SYSPORT_MIN) &&
                 (sysportInfo[unit][sysPort].bucket <=
                                 BCM_SBX_DEVICE_COSQ_EXTENDED_ESET_SYSPORT_MAX) ) {
                sysportInfo[unit][sysPort].state = BCM_INT_SBX_SYSPORT_STATE_RESERVED_ESET;
                sysportMap[unit][sysPort] = BCM_INT_SBX_SYSPORT_STATE_RESERVED_ESET;
            }
        }
    }

    /* determine if independent flow control is being used */
    if (isIndependentFc == TRUE) {
        for (sysPort = 0; sysPort < BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS; sysPort++) {
            if ( (isExtendedEset == TRUE) && (sysportInfo[unit][sysPort].bucket >=
                                 BCM_SBX_DEVICE_COSQ_EXTENDED_ESET_SYSPORT_MIN) &&
                 (sysportInfo[unit][sysPort].bucket <=
                                 BCM_SBX_DEVICE_COSQ_EXTENDED_ESET_SYSPORT_MAX) ) {
                continue;
            }

            if ((sysportInfo[unit][sysPort].bucket % 2) != 0) {
                sysportInfo[unit][sysPort].state = BCM_INT_SBX_SYSPORT_STATE_RESERVED_FIFO_FC;
                sysportMap[unit][sysPort] = BCM_INT_SBX_SYSPORT_STATE_RESERVED_FIFO_FC;
            }
        }
    }


    /* fill in sysport map that will be used for Fabric Logical Ports */
    _bcm_sbx_device_cosq_sysport_initialize(unit, numNodes, numPorts);

    /* determine if additional LNA resources can be allocated */
    _bcm_sbx_device_cosq_allocate_additional_sysports(unit, numNodes, numPorts);

    /* fill lna map that will be used by Fabric logical Port */
    _bcm_sbx_device_cosq_fill_lna_map_Q2EC(unit, numNodes, numPorts, FALSE);

    /* fill lna map with indexes for port remap table for Fabric Logical Port */
    _bcm_sbx_device_cosq_fill_lna_map_port_remap(unit, numNodes, numPorts, FALSE);

    /* determine number LNA ports available */
    _bcm_sbx_device_cosq_get_lna_map_available_entries(unit);

    /* determine number of sysports available */
    _bcm_sbx_device_cosq_det_sysport_available_entries(unit);

    /* determine sysport buckets that are available, and resorces reserved per bucket */
    _bcm_sbx_device_cosq_det_sysport_bucket_resources(unit); 

}

void
_bcm_sbx_device_cosq_deinit(int unit)
{

    if (lnaMap[unit] != NULL) {
        sal_free(lnaMap[unit]);
        lnaMap[unit] = NULL;
    }

    if (sysportMap[unit] != NULL) {
        sal_free(sysportMap[unit]);
        sysportMap[unit] = NULL;
    }

    if (sysportBucketMap[unit] != NULL) {
        sal_free(sysportBucketMap[unit]);
        sysportBucketMap[unit] = NULL;
    }

    if (sysportInfo[unit] != NULL) {
        sal_free(sysportInfo[unit]);
        sysportInfo[unit] = NULL;
    }

    if (bucketInfo[unit] != NULL) {
        sal_free(bucketInfo[unit]);
        bucketInfo[unit] = NULL;
    }

    if (nodeInfo[unit] != NULL) {
        sal_free(nodeInfo[unit]);
        bucketInfo[unit] = NULL;
    }

}

int
_bcm_sbx_device_cosq_is_sysport_available(int unit, int sysport)
{
    int is_available = FALSE;


    is_available = (sysportInfo[unit][sysport].state == BCM_INT_SBX_SYSPORT_STATE_GLOBAL) ?
                                                                               TRUE : FALSE;
    return(is_available);
}

int
_bcm_sbx_device_cosq_is_lna_available(int unit, int lna)
{
    int is_available = FALSE;


    is_available = (lnaMap[unit][lna] == TRUE) ? FALSE : TRUE;
    return(is_available);
}


#ifdef BCM_WARM_BOOT_SUPPORT
int
_bcm_sbx_wb_device_cosq_state_sync(int unit, int flag)
{
    int    rv = 0;
    uint8  *ptr, dummy = 0;
    uint32 i = 0, scache_len = 0;

    if (init_done == FALSE) 
	return rv;

    /* allocate space for structures */
    if (flag == 2) {
	/* lna map */
	scache_len += (sizeof(int) * BCM_SBX_DEVICE_COSQ_MAX_LNAS);

	/* sysportMap */
	scache_len += (sizeof(int) * BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS);

	/* sysportBucketMap */
	scache_len += (sizeof(int) * BCM_SBX_DEVICE_COSQ_SYSPORT_NO_BUCKETS);

	/* sysportInfo */
	for (i=0; i< BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS; i++) {
	    scache_len += sizeof(uint8);  /* state;    */
	    scache_len += sizeof(uint8);  /* bucket;   */
	    scache_len += sizeof(uint16); /* sysport;  */
	    scache_len += sizeof(uint8);  /* used;     */
	    scache_len += sizeof(uint8);  /* dummy     */
	    scache_len += sizeof(uint8);  /* dummy     */
	    scache_len += sizeof(uint8);  /* dummy     */
	    scache_len += sizeof(uint16); /* node;     */
	    scache_len += sizeof(uint16); /* port;     */
	}

	/* bucketInfo */
	scache_len += (sizeof(bucket_info_t) * BCM_SBX_DEVICE_COSQ_SYSPORT_NO_BUCKETS);

	/* nodeInfo */
	scache_len += (sizeof(node_info_t) * BCM_SBX_DEVICE_COSQ_MAX_USER_NODE_NO);

	return scache_len;
    }

    ptr = SOC_SBX_STATE(unit)->cosq_index;

    if (flag == FALSE) {
	/* lna map */
	for (i=0; i<BCM_SBX_DEVICE_COSQ_MAX_LNAS; i++) {
	    *(((uint32 *) ptr)) = lnaMap[unit][i]; ptr += sizeof(uint32);
	}

	/* sysportMap */
	for (i=0; i<BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS; i++) {
	    *(((uint32 *) ptr)) = sysportMap[unit][i]; ptr += sizeof(uint32);
	}

	/* sysportBucketMap */
	for (i=0; i<BCM_SBX_DEVICE_COSQ_SYSPORT_NO_BUCKETS; i++) {
	    *(((uint32 *) ptr)) = sysportBucketMap[unit][i]; ptr += sizeof(uint32);
	}

	/* syportInfo */
	for (i=0; i<BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS; i++) {
	    *(((uint8 *) ptr)) = sysportInfo[unit][i].state; ptr += sizeof(uint8);
	    *(((uint8 *) ptr)) = sysportInfo[unit][i].bucket; ptr += sizeof(uint8);
	    *(((uint16 *) ptr)) = sysportInfo[unit][i].sysport; ptr += sizeof(uint16);
	    *(((uint8 *) ptr)) = sysportInfo[unit][i].used; ptr += sizeof(uint8);
	    *(((uint8 *) ptr))  = dummy                      ; ptr += sizeof(uint8);
	    *(((uint8 *) ptr))  = dummy                      ; ptr += sizeof(uint8);
	    *(((uint8 *) ptr))  = dummy                      ; ptr += sizeof(uint8);
	    *(((uint16 *) ptr)) = sysportInfo[unit][i].node; ptr += sizeof(uint16);
	    *(((uint16 *) ptr)) = sysportInfo[unit][i].port; ptr += sizeof(uint16);
	}

	/* bucketInfo */
	sal_memcpy(&bucketInfo[unit][0], (((uint8 *)ptr)), (sizeof(bucket_info_t) * BCM_SBX_DEVICE_COSQ_SYSPORT_NO_BUCKETS));
	ptr += (sizeof(bucket_info_t) * BCM_SBX_DEVICE_COSQ_SYSPORT_NO_BUCKETS);

	/* nodeInfo */
	sal_memcpy(&nodeInfo[unit][0], (((uint8 *)ptr)), (sizeof(node_info_t) * BCM_SBX_DEVICE_COSQ_MAX_USER_NODE_NO));
	ptr += (sizeof(node_info_t) * BCM_SBX_DEVICE_COSQ_MAX_USER_NODE_NO);
    } else if (flag == TRUE) {
	/* lna map */
	for (i=0; i<BCM_SBX_DEVICE_COSQ_MAX_LNAS; i++) {
	    lnaMap[unit][i] = *(((uint32 *) ptr)); ptr += sizeof(uint32);
	}

	/* sysportMap */
	for (i=0; i<BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS; i++) {
	    sysportMap[unit][i] = *(((uint32 *) ptr)); ptr += sizeof(uint32);
	}

	/* sysportBucketMap */
	for (i=0; i<BCM_SBX_DEVICE_COSQ_SYSPORT_NO_BUCKETS; i++) {
	    sysportBucketMap[unit][i] = *(((uint32 *) ptr)); ptr += sizeof(uint32);
	}

	/* syportInfo */
	for (i=0; i<BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS; i++) {
	    sysportInfo[unit][i].state   = *(((uint8 *) ptr)) ; ptr += sizeof(uint8);
	    sysportInfo[unit][i].bucket  = *(((uint8 *) ptr)) ; ptr += sizeof(uint8);
	    sysportInfo[unit][i].sysport = *(((uint16 *) ptr)); ptr += sizeof(uint16);
	    sysportInfo[unit][i].used    = *(((uint8 *) ptr)) ; ptr += sizeof(uint8);
	    dummy                        = *(((uint8 *) ptr)) ; ptr += sizeof(uint8);
	    dummy                        = *(((uint8 *) ptr)) ; ptr += sizeof(uint8);
	    dummy                        = *(((uint8 *) ptr)) ; ptr += sizeof(uint8);
	    sysportInfo[unit][i].node    = *(((uint16 *) ptr)); ptr += sizeof(uint16);
	    sysportInfo[unit][i].port    = *(((uint16 *) ptr)); ptr += sizeof(uint16);
	}

	/* bucketInfo */
	sal_memcpy((((uint8 *)ptr)), &bucketInfo[unit][0], (sizeof(bucket_info_t) * BCM_SBX_DEVICE_COSQ_SYSPORT_NO_BUCKETS));
	ptr += (sizeof(bucket_info_t) * BCM_SBX_DEVICE_COSQ_SYSPORT_NO_BUCKETS);

	/* nodeInfo */
	sal_memcpy((((uint8 *)ptr)), &nodeInfo[unit][0], (sizeof(node_info_t) * BCM_SBX_DEVICE_COSQ_MAX_USER_NODE_NO));
	ptr += (sizeof(node_info_t) * BCM_SBX_DEVICE_COSQ_MAX_USER_NODE_NO);
    } else {
	return BCM_E_PARAM;
    }

    SOC_SBX_STATE(unit)->cosq_index = ptr;
    return rv;
}
#endif /* BCM_WARM_BOOT_SUPPORT */
