/*
 * $Id: oam.h 1.21.4.1 Broadcom SDK $
 * 
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

#ifndef __BCM_INT_OAM_H__
#define __BCM_INT_OAM_H__

#include <bcm/oam.h>
#if defined(BCM_ENDURO_SUPPORT)
#include <bcm/field.h>
#include <bcm_int/esw/trunk.h>
#endif

extern int bcm_esw_oam_lock(int unit);
extern int bcm_esw_oam_unlock(int unit);
#ifdef BCM_WARM_BOOT_SUPPORT
extern int _bcm_esw_oam_sync(int unit);
#endif
#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
extern void _bcm_oam_sw_dump(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */
#define _BCM_OAM_ENDPOINT_CCM_PERIOD_UNDEFINED 0xFFFFFFFF

#define _BCM_OAM_MAC_DA_UPPER_32 0x0180C200
#define _BCM_OAM_MAC_DA_LOWER_13 0x0030 /* To be >> 3 before use */

typedef struct _bcm_oam_group_s
{
    int in_use;
    uint8 name[BCM_OAM_GROUP_NAME_LENGTH];
} _bcm_oam_group_t;

typedef struct _bcm_oam_endpoint_s
{
    int in_use;
    int is_remote;
    bcm_oam_group_t group_index;
    uint16 name;
    int level;
    bcm_vlan_t vlan;
    uint32 glp;
    int local_tx_enabled;
    int local_rx_enabled;
    int remote_index;
    int local_tx_index;
    int local_rx_index;
#if defined(BCM_ENDURO_SUPPORT)
    uint32 vp;
    uint32 flags;
    int lm_counter_index;
    int pri_map_index;
    bcm_field_entry_t vfp_entry;
    bcm_field_entry_t fp_entry_tx;
    bcm_field_entry_t fp_entry_rx;
    bcm_field_entry_t fp_entry_trunk[BCM_SWITCH_TRUNK_MAX_PORTCNT];
#endif
#if defined(BCM_KATANA_SUPPORT)
    int opcode_profile_index;
#endif
} _bcm_oam_endpoint_t;

typedef struct _bcm_oam_event_handler_s
{
    struct _bcm_oam_event_handler_s *next_p;
    bcm_oam_event_types_t event_types;
    bcm_oam_event_cb cb;
    void *user_data;
} _bcm_oam_event_handler_t;

typedef struct _bcm_oam_info_s
{
    int initialized;
    int group_count;
    _bcm_oam_group_t *groups;
    int local_rx_endpoint_count;
    int local_tx_endpoint_count;
    int remote_endpoint_count;
    int endpoint_count;
#if defined(BCM_ENDURO_SUPPORT)
    bcm_field_qset_t vfp_qs;
    bcm_field_qset_t fp_vp_qs;
    bcm_field_qset_t fp_glp_qs;
    bcm_field_group_t vfp_group;
    bcm_field_group_t fp_vp_group;
    bcm_field_group_t fp_glp_group;
    int vfp_entry_count;
    int fp_vp_entry_count;
    int fp_glp_entry_count;
    int lm_counter_count;
    SHR_BITDCL *lm_counter_in_use;
#endif
    SHR_BITDCL *local_tx_endpoints_in_use;
    SHR_BITDCL *local_rx_endpoints_in_use;
    SHR_BITDCL *remote_endpoints_in_use;
    bcm_oam_endpoint_t *remote_endpoints;
    _bcm_oam_endpoint_t *endpoints;
    _bcm_oam_event_handler_t *event_handler_list_p;
    int event_handler_count[bcmOAMEventCount];
} _bcm_oam_info_t;

typedef struct _bcm_oam_fault_s
{
    int current_field;
    int sticky_field;
    uint32 mask;
    uint32 clear_sticky_mask;
} _bcm_oam_fault_t;

typedef struct _bcm_oam_interrupt_s
{
    soc_reg_t status_register;
    soc_field_t endpoint_index_field;
    soc_field_t group_index_field;
    soc_field_t status_field;
    bcm_oam_event_type_t event_type;
} _bcm_oam_interrupt_t;

typedef struct oam_hdr_s {
    uint8 mdl_ver;
    uint8 opcode;
    uint8 flags;
    uint8 first_tlvoffset;
} oam_hdr_t;

typedef struct oam_lm_pkt_s {
    uint32 txfcf;
    uint32 rxfcf;
    uint32 txfcb;
} oam_lm_pkt_t;

typedef struct oam_dm_pkt_s {
    uint32 txtsf_upper;
    uint32 txtsf;
    uint32 rxtsf_upper;
    uint32 rxtsf;
    uint32 txtsb_upper;
    uint32 txtsb;
    uint32 rxtsb_upper;
    uint32 rxtsb;
} oam_dm_pkt_t;

#endif /* !__BCM_INT_OAM_H__ */
