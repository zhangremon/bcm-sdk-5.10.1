/* 
 * $Id: trunk.h 1.2 Broadcom SDK $
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
 * File:        trunk.h
 * Purpose:     Trunk internal definitions to the BCM library.
 */

#ifndef _BCM_INT_SBX_FE2000_TRUNK_H_
#define _BCM_INT_SBX_FE2000_TRUNL_H_

#include <bcm/trunk.h>

/* this is probably way too many, but reduce or increase as needed */
#define _BCM_TRUNK_MAX_CALLBACK  5

/* Callback function type for trunk change notifications. */
typedef int (*bcm_trunk_notify_cb)(
    int unit,
    bcm_trunk_t tid,
    bcm_trunk_add_info_t *add_info,
    void *user_data);

extern int 
bcm_fe2000_trunk_change_register(int unit,
                                 bcm_trunk_notify_cb callback,
                                 void *user_data);

extern int
bcm_fe2000_trunk_change_unregister(int unit, 
                                   bcm_trunk_notify_cb callback);

/* Returns the set difference of info_a and info_b.  
 * Specifically, <mod,port> pairs found in info_a not present info_b 
 */
extern void
bcm_fe2k_trunk_add_info_cmp(bcm_trunk_add_info_t *info_a,
                            bcm_trunk_add_info_t *info_b,
                            int                  *num_ports,
                            bcm_module_t          mods[BCM_TRUNK_MAX_PORTCNT],
                            bcm_port_t            ports[BCM_TRUNK_MAX_PORTCNT]);

#endif /* _BCM_INT_SBX_FE2000_TRUNK_H_ */
