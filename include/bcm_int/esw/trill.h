/* 
 * $Id: trill.h 1.7.44.1 Broadcom SDK $
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
 * This file contains TRILL definitions internal to the BCM library.
 */

#ifndef _BCM_INT_TRILL_H
#define _BCM_INT_TRILL_H

#ifdef INCLUDE_L3

#define BCM_MAX_NUM_TRILL_TREES 16
#define BCM_MAX_NUM_VP 8192
#define BCM_MAX_TRILL_IPMC_GROUPS 4096

#define BCM_TRILL_ACCESS_PORT 0x01
#define BCM_TRILL_NETWORK_PORT 0x02

typedef struct _bcm_td_trill_multicast_count_s {
    bcm_multicast_t  l3mc_group;     /* TRILL Domain Multicast Group */
    uint16  network_port_count; /* TRILL Port count */
} _bcm_td_trill_multicast_count_t;

/*
 * Software book keeping for Trill  related information
 */
typedef struct _bcm_td_trill_bookkeeping_s {
    int         initialized;        /* Set to TRUE when Trill module initialized */
    sal_mutex_t    trill_mutex;    /* Protection mutex. */
    uint16    rootBridge[BCM_MAX_NUM_TRILL_TREES]; /* Root Bridge Store */
    uint16    rBridge[BCM_MAX_NUM_VP];    /* RBridges Store */
    _bcm_td_trill_multicast_count_t      multicast_count[BCM_MAX_TRILL_IPMC_GROUPS]; /* Multicast Store */
} _bcm_td_trill_bookkeeping_t;

extern _bcm_td_trill_bookkeeping_t *_bcm_td_trill_bk_info[BCM_MAX_NUM_UNITS];

/* Generic memory allocation routine. */
#define BCM_TD_TRILL_ALLOC(_ptr_,_size_,_descr_)                      \
            do {                                                     \
                if ((NULL == (_ptr_))) {                             \
                   _ptr_ = sal_alloc((_size_),(_descr_));            \
                }                                                    \
                if((_ptr_) != NULL) {                                \
                    sal_memset((_ptr_), 0, (_size_));                \
                }                                                    \
            } while (0)

#ifdef BCM_WARM_BOOT_SUPPORT
extern int _bcm_esw_trill_sync(int unit);
extern void _bcm_td_trill_sw_dump(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT */

#endif	/* INCLUDE_L3 */
#endif	/* !_BCM_INT_TRILL_H */
