/*
 * $Id: l2.h 1.19 Broadcom SDK $
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
 */
#ifndef _BCM_INT_ROBO_L2_H
#define _BCM_INT_ROBO_L2_H
#define BCM_L2_HASH_ENABLE	0x1
#define BCM_L2_HASH_DISABLE	0x2

typedef struct  _bcm_robo_l2_gport_params_s {
    int     param0;
    int     param1;
    uint32  type;
}_bcm_robo_l2_gport_params_t;

extern int _bcm_robo_l2_gport_parse(int unit, bcm_l2_addr_t *l2addr, 
                                   _bcm_robo_l2_gport_params_t *params);
extern int bcm_robo_l2_802dot1Q_learning_enable_set(int unit, int enable);
extern int bcm_robo_l2_802dot1Q_learning_enable_get(int unit, int *enable);
extern int bcm_robo_l2_hash_select(int unit, int hash_algorithm);

#define     L2_ROBO_MEM_CHUNKS_DEFAULT   100
typedef struct _bcm_robo_l2_traverse_s {
    void                        *pattern;   /* Pattern to match on */
    bcm_l2_addr_t               *data;      /* L2 Entry */
    bcm_l2_traverse_cb          user_cb;    /* User callback function */
    void                        *user_data; /* Data provided by the user, cookie */
}_bcm_robo_l2_traverse_t;

typedef struct _bcm_robo_l2_replace_s {
    bcm_mac_t           match_mac;
    bcm_vlan_t          match_vid;
    bcm_module_t        match_module;
    bcm_port_t          match_port;
    bcm_trunk_t         match_trunk;
    int                 isTrunk;
    int                 isStatic;
    int                 isMcast;
    int                 isPending;
    int                 isDel;
    bcm_module_t        new_module;
    bcm_port_t          new_port;
    bcm_trunk_t         new_trunk;
}_bcm_robo_l2_replace_t;


#ifdef BCM_TB_SUPPORT
#define _TB_ARL_STATUS_VALID    0x3     /* filed value for TB only. */
#define _TB_ARL_STATUS_PENDING  0x1     /* filed value for TB only. */
#define _TB_ARL_STATUS_INVALID  0x0     /* filed value for TB only. */


/* TB's new feature about fast aging on independent control on the l2 entry 
 * with Static/Dynamic status and Unicast/Multicast type.
 *  
 *  Below define serve such feature for TB and the usage is for L2 replace 
 *  API only. The definitions below must not conflict with the 
 *  BCM_L2_REPLACE_XXX in l2.h of bcm layer.
 *
 *  P.S BCM_L2_REPLACE_MATCH_STATIC will be translated to 
 *      _BCM_TB_L2_REPLACE_MATCH_STATIC for TB support. 
 */
#define _BCM_TB_L2_REPLACE_MATCH_STATIC         0x10000
#define _BCM_TB_L2_REPLACE_MATCH_STATIC_ONLY    0x20000
#define _BCM_TB_L2_REPLACE_MATCH_MCAST          0x40000
#define _BCM_TB_L2_REPLACE_MATCH_MCAST_ONLY     0x80000

#endif  /* BCM_TB_SUPPORT */

#endif
