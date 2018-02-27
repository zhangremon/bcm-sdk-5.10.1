/*
 * $Id: recovery.h 1.7 Broadcom SDK $
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
#ifndef _BCM_INT_SBX_FE2000_RECOVERY_H_
#define _BCM_INT_SBX_FE2000_RECOVERY_H_
#ifdef BCM_WARM_BOOT_SUPPORT

#include <sal/types.h>

/* Private FE2000 interface for recovery storage access.  Provides a single 
 * collection point for usable on-chip bits used during a warmboot.  Any 
 * usable bit for recovery that is not interfaced by this module is 
 * not supported. 
 */

/* _bcm_fe2000_recovery_var_t defines the list of internal state that can be 
 * recovered without external storage.  The recovery module takes advantage of
 * all unused on-chip memory to store the state 
 */
typedef enum _bcm_fe2000_recovery_var_e {
#define RECOVERY_ENTRY_VAR_PLACE(x)  x ,
#include <bcm_int/sbx/fe2000/recovery_vars.h>

    fe2k_rcvr_num_elements
} _bcm_fe2000_recovery_var_t;

#define RCVR_FAST_PV2E_IDX(port__, vid__)   ((port__) * BCM_VLAN_COUNT + (vid__))

/* Vlan must init first during a warmboot.  When it does, it will scan the soft
 * copy of VSIs per port,vid.  All TB VSIs will be recovered as well as VLAN
 * GPORTS.  All remaining unknown VSIs will be inserted into the rc_non_tb_vsi
 * list for faster access by the remaining BCM modules to recover.
 */
typedef struct rc_non_tb_vsi_s {
    dq_t      dql;   /* dq-link; double-linked list connection */
    uint16    port;
    uint16    vid;
} rc_non_tb_vsi_t;


int _bcm_fe2000_recovery_init(int unit);
int _bcm_fe2000_recovery_commit(int unit);
int _bcm_fe2000_recovery_done(int unit);
int _bcm_fe2000_recovery_deinit(int unit);

/*  _bcm_fe2000_recovery_var_t primative accessors
 */
int _bcm_fe2000_recovery_var_get(int unit, 
                                 _bcm_fe2000_recovery_var_t var, uint32 *val);
int _bcm_fe2000_recovery_var_set(int unit, 
                                 _bcm_fe2000_recovery_var_t var, uint32 val);


/* Accessors to cached fast memory table reads, available only during a 
 * warmboot.
 */
uint32 *_bcm_fe2000_recovery_pv2e_vlan_get(int unit);
uint32 *_bcm_fe2000_recovery_pv2e_lpi_get(int unit);

int _bcm_fe2000_recovery_non_tb_vsi_insert(int unit, uint16 port, uint16 vid);
int _bcm_fe2000_recovery_non_tb_vsi_head_get(int unit, rc_non_tb_vsi_t **head);


#endif /* BCM_WARM_BOOT_SUPPORT */
#endif  /* _BCM_INT_SBX_FE2000_RECOVERY_H_ */
