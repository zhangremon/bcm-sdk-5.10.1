/*
 * $Id: device_cosq.h 1.2 Broadcom SDK $
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
 * File:        device_cosq.h
 * Purpose:     Implement Hybrid resource allocation algorithm. This is
 *              shared across devices.
 */

#ifndef	_BCM_INT_SBX_DEVICE_COSQ_H_
#define	_BCM_INT_SBX_DEVICE_COSQ_H_

/* The following definations have not been abstracted. This could change */
/* in the future.                                                        */

#define BCM_SBX_DEVICE_COSQ_MAX_LNAS                         1024
#define BCM_SBX_DEVICE_COSQ_MAX_SYSPORTS                     2816

#define BCM_SBX_DEVICE_COSQ_MIN_USER_NODE_NO                 1
#define BCM_SBX_DEVICE_COSQ_MAX_USER_NODE_NO                 72
#define BCM_SBX_DEVICE_COSQ_MIN_USER_PORT_NO                 1
#define BCM_SBX_DEVICE_COSQ_MAX_USER_PORT_NO                 50

#define BCM_SBX_DEVICE_COSQ_MIN_SYSPORT_NODE                 0
#define BCM_SBX_DEVICE_COSQ_MAX_SYSPORT_NODE                 63
#define BCM_SBX_DEVICE_COSQ_MIN_SYSPORT_PORT                 0
#define BCM_SBX_DEVICE_COSQ_MAX_SYSPORT_PORT                 63

#define BCM_SBX_DEVICE_COSQ_MIN_LNA_NODE                     0
#define BCM_SBX_DEVICE_COSQ_MAX_LNA_NODE                     31
#define BCM_SBX_DEVICE_COSQ_MIN_LNA_PORT                     0
#define BCM_SBX_DEVICE_COSQ_MAX_LNA_PORT                     31

#define BCM_SBX_DEVICE_COSQ_SYSPORT_NO_BUCKETS               64

#define BCM_SBX_DEVICE_COSQ_EXTENDED_ESET_SYSPORT_MIN        0
#define BCM_SBX_DEVICE_COSQ_EXTENDED_ESET_SYSPORT_MAX        13

#define BCM_SBX_DEVICE_COSQ_NO_COS_LEVELS                    8
#define BCM_SBX_DEVICE_COSQ_NO_QUEUES_MANAGED_BY_LNA         16

#define BCM_SBX_DEVICE_COSQ_GET_PORT_FROM_SYSPORT(sys_port)      (sys_port & 0x3F)
#define BCM_SBX_DEVICE_COSQ_GET_NODE_FROM_SYSPORT(sys_port)      ((sys_port >> 6) & 0x3F)
#define BCM_SBX_DEVICE_COSQ_SET_SYSPORT(node, port)              ((node << 6) | (port))

#define BCM_SBX_DEVICE_COSQ_GET_PORT_FROM_LNA(lna)               (lna & 0x1F)
#define BCM_SBX_DEVICE_COSQ_GET_NODE_FROM_LNA(lna)               ((lna >> 5) & 0x1F)
#define BCM_SBX_DEVICE_COSQ_SET_LNA(node, port)                  ((node << 5) | (port))

#define BCM_SBX_DEVICE_COSQ_SET_PORT_REMAP_INDEX(port, qe1k_flag, ef_flag)   \
                                             (port | (qe1k_flag << 6) | (ef_flag << 7))

#define BCM_SBX_DEVICE_COSQ_SET_SYSPORT_BUCKET(sysport)          \
                                             (sysport % BCM_SBX_DEVICE_COSQ_SYSPORT_NO_BUCKETS)



typedef struct _sysport_info_ {
    int state;
    int bucket;
    int sysport;
    int used;
    int node;
    int port;
} sysport_info_t;

typedef struct _bucket_info_ {
    int depth;
    char nodes_allocated[10];
} bucket_info_t;

typedef struct {
    char ports_allocated[8];
} node_info_t;



void
_bcm_sbx_device_cosq_init(int unit);

void
_bcm_sbx_device_cosq_deinit(int unit);

int
_bcm_sbx_device_cosq_is_sysport_available(int unit, int sysport);

int
_bcm_sbx_device_cosq_is_lna_available(int unit, int lna);

#ifdef BCM_WARM_BOOT_SUPPORT
extern int 
_bcm_sbx_wb_device_cosq_state_sync(int unit, int flag);
#endif /* BCM_WARM_BOOT_SUPPORT */

#endif	/* _BCM_INT_SBX_DEVICE_COSQ_H_ */
