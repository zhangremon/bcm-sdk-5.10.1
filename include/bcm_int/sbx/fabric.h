/*
 * $Id: fabric.h 1.21 Broadcom SDK $
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
 */

#ifndef _BCM_INT_SBX_FABRIC_H_
#define _BCM_INT_SBX_FABRIC_H_

#include <bcm/fabric.h>
#include <soc/sbx/sbFabCommon.h>

#define BCM_SBX_DS_ID_IS_RESOURCE_ALLOCATED(unit, ds_id)                                   \
            (*(ds_state_p[unit]->ds_grp_resource + (ds_id / 8)) & (1 << (ds_id % 8)))

#define BCM_SBX_DS_ID_SET_RESOURCE(unit, ds_id)                                            \
            (*(ds_state_p[unit]->ds_grp_resource + (ds_id / 8))) |= (1 << (ds_id % 8))

#define BCM_SBX_DS_ID_CLEAR_RESOURCE(unit, ds_id)                                          \
            (*(ds_state_p[unit]->ds_grp_resource + (ds_id / 8))) &= (~(1 << (ds_id % 8)))

#define BCM_SBX_DS_ID_IS_NODE_MEMBER(unit, ds_id, node)                                    \
            (*(ds_state_p[unit]->ds_grp_membership +                                       \
                  (ds_id * ds_state_p[unit]->num_bytes_ds_desc + (node / 8))) &            \
                                                                  (1 << (node % 8)))

#define BCM_SBX_DS_ID_IS_NODE_MEMBER(unit, ds_id, node)                                    \
            (*(ds_state_p[unit]->ds_grp_membership +                                       \
                  (ds_id * ds_state_p[unit]->num_bytes_ds_desc + (node / 8))) &            \
                                                                  (1 << (node % 8)))

#define BCM_SBX_DS_ID_SET_NODE_MEMBER(unit, ds_id, node)                                   \
            (*(ds_state_p[unit]->ds_grp_membership +                                       \
                  (ds_id * ds_state_p[unit]->num_bytes_ds_desc + (node / 8)))) |=          \
                                                                  (1 << (node % 8))

#define BCM_SBX_DS_ID_CLEAR_NODE_MEMBER(unit, ds_id, node)                                 \
            (*(ds_state_p[unit]->ds_grp_membership +                                       \
                  (ds_id * ds_state_p[unit]->num_bytes_ds_desc + (node / 8)))) &=          \
                                                                  (~(1 << (node % 8)))

#define BCM_SBX_DS_ID_CLEAR_ALL_NODE_MEMBER(unit, ds_id, iter)                             \
        {                                                                                  \
            for (iter = 0; iter < ds_state_p[unit]->num_bytes_ds_desc; iter++) {           \
                (*(ds_state_p[unit]->ds_grp_membership +                                   \
                  (ds_id * ds_state_p[unit]->num_bytes_ds_desc + iter))) = 0;              \
            }                                                                              \
        }

extern int
bcm_sbx_fabric_ds_id_to_ef_ds_id(int unit, bcm_fabric_distribution_t ds_id,
                                           bcm_fabric_distribution_t *ef_ds_id_p);

extern int
bcm_sbx_fabric_dsid_to_nef_dsid(int unit, bcm_fabric_distribution_t ds_id,
                                          bcm_fabric_distribution_t *nef_ds_id_p);



typedef struct bcm_sbx_fabric_state_s {
    bcm_fabric_control_redundancy_handler_t red_f;
    soc_scache_handle_t                     handle;
    uint32 timeslot_size;
    uint32 scaled;
    uint32 old_demand_scale;
    uint8  *xbar_map[SB_FAB_DEVICE_MAX_LOGICAL_CROSSBARS]; /* logical to physical crossbar map - indexed by [logxbar][node] */
    uint8  *congestion_size; /* indexed by [modid] - modid is in the range of 0 - 255 */
    uint32 egress_aging;
    uint8  *logical_to_physical_channel;
    uint8  *physical_to_logical_channel;
} bcm_sbx_fabric_state_t;

typedef struct bcm_sbx_ds_state_s {
    int       initialized;
    uint32    max_ds_ids;          /* maximum number of distribution groups/ESETS */
                                   /* supported by device                         */
    uint16    num_nodes;           /* current number of configured nodes          */
    uint16    max_nodes;           /* maximum number of nodes supported by device */
    uint32    num_bytes_ds_desc;   /* number of bytes required per distribution   */
                                   /* group for describing node membership        */
    uint32    num_ds_grps_used;    /* number of distribution groups/ESETs         */

    uint8     *ds_grp_resource;    /* resorce allocation bit map */
    uint8     *ds_grp_membership;  /* resource membership information */
    uint8     *mc_full_eval_min_state; /* per ds_id state                                               */
                                       /* if set, hw will forward over backplane if any member can send */
                                       /* (no mc FIFO backpressure), if cleared, hw will only forward   */
                                       /* over backplane if all members can send (all nodes w/o mc bp)  */
} bcm_sbx_ds_state_t;

/* Function prototype for common function */
int32_t
bcm_sbx_fabric_get_timeslot_size(int32_t nTotalLogicalCrossbars,
				 int32_t bSetTimeslotSizeForHalfBus);

int
bcm_sbx_adjust_demand_scale(int unit, uint64 xbars,
                  int adjust, int *adjusted_demand_shift, uint32 *adjusted_clocks_per_epoch);

int32_t
bcm_sbx_fabric_get_rate_conversion(int unit, int32_t nTotalLogicalCrossbars);

int32_t
bcm_sbx_fabric_timeslot_burst_size_bytes_set(int unit, int els);

int
bcm_sbx_fabric_timeslot_burst_size_bytes_get(int unit, int src_node_type, 
                                             int dest_node_type, 
                                             int num_channels, 
                                             int *bytes_per_timeslot);

#ifdef BCM_WARM_BOOT_SUPPORT
extern int
bcm_sbx_wb_fabric_state_sync(int unit, int sync);
#endif /* BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
void 
bcm_sbx_wb_fabric_sw_dump(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

#endif  /* _BCM_INT_SBX_FABRIC_H_ */
