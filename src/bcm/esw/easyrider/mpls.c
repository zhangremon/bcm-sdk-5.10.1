/*
 * $Id: mpls.c 1.40.6.1 Broadcom SDK $
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
 * File:    mpls.c
 * Purpose: Manages MPLS functions
 */

#include <soc/defs.h>
#include <sal/core/libc.h>

#if defined(BCM_EASYRIDER_SUPPORT) && defined(BCM_MPLS_SUPPORT) && \
    defined(INCLUDE_L3)

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/l2u.h>
#include <soc/util.h>
#include <soc/debug.h>
#include <soc/easyrider.h>

#include <bcm/l2.h>
#include <bcm/l3.h>
#include <bcm/mpls.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/l3.h>
#include <bcm_int/esw/firebolt.h>
#include <bcm_int/esw/easyrider.h>
#include <bcm_int/esw/stack.h>
#include <bcm_int/esw_dispatch.h>
#include <bcm_int/esw/xgs3.h>
#include <bcm_int/esw/mpls.h>

#include <bcm_int/esw_dispatch.h>

/*
 * The following defines are used to protect applications
 * from mixing calls between the old legacy-style APIs and 
 * the newer MPLS APIs. The first time an API call is made
 * after initialization, the driver remembers which style
 * (old or new) was called. Subsequent API calls must be of
 * the same style, otherwize BCM_E_CONFIG is returned.
 *
 *  The NEW APIs are:
 *    bcm_mpls_vpn_id_create/destroy/destroy_all/get
 *    bcm_mpls_port_add/delete/delete_all/get/get_all
 *    bcm_mpls_tunnel_initiator_set/clear/clear_all/get
 *    bcm_mpls_tunnel_switch_add/delete/delete_all/get/traverse
 *
 *  The OLD APIs are:
 *    bcm_mpls_vpn_create/destroy/add/delete/delete_all/get
 *    bcm_mpls_vpn_circuit_add/delete/delete_all/get
 *    bcm_mpls_l3_initiator_set/clear/get/clear_all
 *    bcm_mpls_switch_add/delete/get/traverse
 */
#define _BCM_ER_MPLS_API_STYLE_NONE    0
#define _BCM_ER_MPLS_API_STYLE_OLD     1
#define _BCM_ER_MPLS_API_STYLE_NEW     2

#define BCM_VPN_MAX  255

/*
 * Eight L3_INTF entries are reserve for MPLS incoming interfaces.
 * Entries 4087 - 4094 correspond to the 8 EXP maps of ER.
 */
#define _BCM_ER_MPLS_L3_INTF_BASE 4087

typedef enum bcm_mpls_opcode_e {
    BCM_MPLS_OPCODE_SWAP = 0,
    BCM_MPLS_OPCODE_SWAP_PUSH,
    BCM_MPLS_OPCODE_SWAP_NEW_DOMAIN,
    BCM_MPLS_OPCODE_PHP,
    BCM_MPLS_OPCODE_RESERVE,
    BCM_MPLS_OPCODE_POP_L2_SWITCH,
    BCM_MPLS_OPCODE_POP_DST_MOD_PORT,
    BCM_MPLS_OPCODE_POP_L3_SWITCH,
    BCM_MPLS_OPCODE_POP_L3_NEXTHOP,
    BCM_MPLS_OPCODE_POP_SEARCH
} _bcm_mpls_hw_opcode_t;

typedef struct _bcm_er_mpls_l2_info_s {
    /* Customer-facing port and vlan info */
    bcm_vlan_t       cu_vlan;
    bcm_module_t     cu_mod;
    bcm_port_t       cu_port;

    /* Network-facing port info */
    bcm_mpls_label_t vc_ingress_label;
    bcm_module_t     vc_src_mod;
    bcm_port_t       vc_src_port;
    bcm_if_t         vc_tunnel_if;
} _bcm_er_mpls_l2_info_t;

/*
 * Software book keeping for MPLS related information
 */
typedef struct _bcm_mpls_bookkeeping_s {
    uint8       mpls_initialized;  /* Is MPLS module init'ed ? */
    uint8       api_style;         /* Old or New API style */
    SHR_BITDCL *vpn_allocated;     /* VPN usage bit mask */
    SHR_BITDCL *vpn_l2_allocated;  /* L2 VPNs */
    uint16      vpn_max;           /* MAX number of VPNs */
    uint16      vpn_count;         /* number of VPNs created */
    int         vc_table_size;     /* chip VC label table size */
    int         vc_table_used;     /* chip VC label table size */
    int         mpls_switch_max;   /* MAX # of MPLS ILMs */
    int         mpls_switch_count; /* # of MPLS ILMs added */
    int         exp_map_max;       /* # of MPLS EXP maps available */
    SHR_BITDCL *exp_map_allocated; /* EXP map usage bit mask */
    int         ing_exp_map_max;   /* # of ingress MPLS EXP maps available */
    SHR_BITDCL *ing_exp_map_allocated; /* EXP map usage bit mask */
    _bcm_er_mpls_l2_info_t *l2_info; /* L2 VPN port info (per VFI) */
} _bcm_mpls_bookkeeping_t;

STATIC _bcm_mpls_bookkeeping_t _bcm_mpls_bk_info[BCM_MAX_NUM_UNITS] = {{ 0 }};

#define MPLS_INFO(unit)   (&_bcm_mpls_bk_info[unit])
#define L3_INFO(unit)   (&_bcm_l3_bk_info[unit])

#define MPLS_INIT(unit)                                   \
    do {                                                  \
        if ((unit < 0) || (unit >= BCM_MAX_NUM_UNITS)) {  \
            return BCM_E_UNIT;                            \
        }                                                 \
        if (!_bcm_mpls_bk_info[unit].mpls_initialized) {  \
            return BCM_E_INIT;                            \
        }                                                 \
    } while (0)

#define MPLS_API_STYLE(unit, style)                                            \
    do {                                                                       \
        if (_bcm_mpls_bk_info[unit].api_style == _BCM_ER_MPLS_API_STYLE_NONE) {\
            _bcm_mpls_bk_info[unit].api_style = style;                         \
        } else if (_bcm_mpls_bk_info[unit].api_style != style) {               \
            if (soc_feature(unit, soc_feature_mpls_bos_lookup)) {              \
                return BCM_E_CONFIG;                                           \
            } else {                                                           \
                return BCM_E_UNAVAIL;                                          \
            }                                                                  \
        }                                                                      \
    } while (0)

/*
 * L3 interface table usage bit mask operations
 */
#define BCM_MPLS_VPN_USED_GET(_u_, _vpn_) \
        SHR_BITGET(MPLS_INFO(_u_)->vpn_allocated, (_vpn_))
#define BCM_MPLS_VPN_USED_SET(_u_, _vpn_)               \
    {                                                 \
        SHR_BITSET(MPLS_INFO((_u_))->vpn_allocated, (_vpn_)); \
        MPLS_INFO((_u_))->vpn_count++;                 \
    }
#define BCM_MPLS_VPN_USED_CLR(_u_, _vpn_)               \
    {                                                 \
        SHR_BITCLR(MPLS_INFO((_u_))->vpn_allocated, (_vpn_)); \
        MPLS_INFO((_u_))->vpn_count--;                 \
    }

#define BCM_L2_VPN_USED_GET(_u_, _vpn_) \
        SHR_BITGET(MPLS_INFO(_u_)->vpn_l2_allocated, (_vpn_))
#define BCM_L2_VPN_USED_SET(_u_, _vpn_)               \
    {                                                 \
        SHR_BITSET(MPLS_INFO((_u_))->vpn_l2_allocated, (_vpn_)); \
        MPLS_INFO((_u_))->vpn_count++;    \
    }

#define BCM_L2_VPN_USED_CLR(_u_, _vpn_)               \
    {                                                 \
        SHR_BITCLR(MPLS_INFO((_u_))->vpn_l2_allocated, (_vpn_)); \
        MPLS_INFO((_u_))->vpn_count--;    \
    }

#define BCM_MPLS_EXP_MAP_USED_GET(_u_, _exp_map_) \
        SHR_BITGET(MPLS_INFO(_u_)->exp_map_allocated, (_exp_map_))
#define BCM_MPLS_EXP_MAP_USED_SET(_u_, _exp_map_)               \
    {                                                 \
        SHR_BITSET(MPLS_INFO((_u_))->exp_map_allocated, (_exp_map_)); \
    }
#define BCM_MPLS_EXP_MAP_USED_CLR(_u_, _exp_map_)               \
    {                                                 \
        SHR_BITCLR(MPLS_INFO((_u_))->exp_map_allocated, (_exp_map_)); \
    }

#define BCM_MPLS_ING_EXP_MAP_USED_GET(_u_, _ing_exp_map_) \
        SHR_BITGET(MPLS_INFO(_u_)->ing_exp_map_allocated, (_ing_exp_map_))
#define BCM_MPLS_ING_EXP_MAP_USED_SET(_u_, _ing_exp_map_)               \
    {                                                 \
        SHR_BITSET(MPLS_INFO((_u_))->ing_exp_map_allocated, (_ing_exp_map_)); \
    }
#define BCM_MPLS_ING_EXP_MAP_USED_CLR(_u_, _ing_exp_map_)               \
    {                                                 \
        SHR_BITCLR(MPLS_INFO((_u_))->ing_exp_map_allocated, (_ing_exp_map_)); \
    }

#define _BCM_ER_MPLS_EXP_MAP_TABLE_TYPE_MASK    0x100
#define _BCM_ER_MPLS_EXP_MAP_TABLE_TYPE_INGRESS 0x100
#define _BCM_ER_MPLS_EXP_MAP_TABLE_TYPE_EGRESS  0x000
#define _BCM_ER_MPLS_EXP_MAP_TABLE_NUM_MASK     0x0ff

#define MPLS_SWITCH_LOCK(unit) \
        soc_mem_lock(unit, L3_DEFIP_ALGm)
#define MPLS_SWITCH_UNLOCK(unit) \
        soc_mem_unlock(unit, L3_DEFIP_ALGm)

STATIC int
_mem_param32_ok(int unit, soc_mem_t mem, soc_field_t field, uint32 val)
{
    int bits;
    uint32 mask;
    bits = soc_mem_field_length(unit, mem, field);
    assert(bits>0 && bits<=32);
    mask = 0xfffffffe << (bits-1);
    if ((val & mask) != 0) {
        return FALSE;
    }
    return TRUE;
}

#define MPLS_MEMPARAM32_CHECK(unit, mem, field, val)          \
  if (_mem_param32_ok((unit), (mem), (field), (val)) != TRUE) \
      { return BCM_E_PARAM; }
#define MPLS_TRUNK_CHECK(tid)  \
  if (((tid) & ~0x7f) != 0) { return BCM_E_PARAM; }
#define MPLS_MODID_CHECK(modid)  \
  if (((modid) & ~0x3f) != 0) { return BCM_E_PARAM; }
#define MPLS_PORT_CHECK(port)  \
  if (((port) & ~0x1f) != 0) { return BCM_E_PARAM; }

/*
 * Function:
 *      bcm_mpls_enable
 * Purpose:
 *      Enable/disable MPLS function.
 * Parameters:
 *      unit - SOC unit number.
 *      enable - TRUE: enable MPLS support; FALSE: disable MPLS support.
 * Returns:
 *      BCM_E_XXX.
 */
STATIC int
bcm_er_mpls_enable(int unit, int enable)
{
    int port;        /* Port iterator. */
    int value;       /* Set value.     */

    value = (enable) ? 1 : 0; 
    PBMP_PORT_ITER(unit, port) {
        /* No mpls lookup on stacking ports. */
        if (IS_ST_PORT(unit, port)) {
            continue;
        }
        /* enable mpls, ingress port check & multi label switching. */
        BCM_IF_ERROR_RETURN (bcm_esw_port_control_set(unit, port,
                                                      bcmPortControlMpls, value));
        BCM_IF_ERROR_RETURN (bcm_esw_port_control_set(unit, port,
                                                      bcmPortControlMplsIngressPortCheck,
                                                      value));
        BCM_IF_ERROR_RETURN (bcm_esw_port_control_set(unit, port,
                                                      bcmPortControlMplsMultiLabelSwitching,
                                                      value));
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_vlan_enable_set
 * Purpose:
 *      Enable/disable MPLS function per-VLAN.
 * Parameters:
 *      unit - SOC unit number.
 *      enable - TRUE: enable MPLS support; FALSE: disable MPLS support.
 * Returns:
 *      BCM_E_XXX.
 */
int
_bcm_mpls_vlan_enable_set(int unit, bcm_vlan_t vlan, int enable)
{
    vrf_vfi_intf_entry_t   vrf_key, vrfe;
    int index, rv;

    if (!SOC_MEM_FIELD_VALID(unit, VRF_VFI_INTFm, MPLS_ENABLEf)) {
        return BCM_E_UNAVAIL;
    }

    /* See if the key exists */
    sal_memset(&vrf_key, 0, sizeof(vrf_vfi_intf_entry_t));
    /* Set SRC_MODID,TGID to don't care, 0xfff */
    soc_VRF_VFI_INTFm_field32_set(unit, &vrf_key, SRC_MODIDf, 0x3f);
    soc_VRF_VFI_INTFm_field32_set(unit, &vrf_key, TGIDf, 0x3f);
    soc_VRF_VFI_INTFm_field32_set(unit, &vrf_key, VLANf, vlan);

    rv = soc_mem_er_search(unit, VRF_VFI_INTFm, MEM_BLOCK_ANY,
                           &index, &vrf_key, &vrfe, 0);
    if (rv >= 0) {
        soc_VRF_VFI_INTFm_field32_set(unit, &vrfe, MPLS_ENABLEf,
                                      enable ? 1 : 0);
        soc_mem_insert(unit, VRF_VFI_INTFm, MEM_BLOCK_ALL, &vrfe);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_vlan_enable_get
 * Purpose:
 *      Get MPLS enable/disable for the specified VLAN.
 * Parameters:
 *      unit - SOC unit number.
 * Returns:
 *      enable - TRUE: MPLS enabled; FALSE: MPLS disabled.
 *      BCM_E_XXX.
 */
int
_bcm_mpls_vlan_enable_get(int unit, bcm_vlan_t vlan, int *enable)
{
    vrf_vfi_intf_entry_t   vrf_key, vrfe;
    int index, rv, ena;

    if (!SOC_MEM_FIELD_VALID(unit, VRF_VFI_INTFm, MPLS_ENABLEf)) {
        return BCM_E_UNAVAIL;
    }

    /* See if the key exists */
    sal_memset(&vrf_key, 0, sizeof(vrf_vfi_intf_entry_t));
    /* Set SRC_MODID,TGID to don't care, 0xfff */
    soc_VRF_VFI_INTFm_field32_set(unit, &vrf_key, SRC_MODIDf, 0x3f);
    soc_VRF_VFI_INTFm_field32_set(unit, &vrf_key, TGIDf, 0x3f);
    soc_VRF_VFI_INTFm_field32_set(unit, &vrf_key, VLANf, vlan);

    rv = soc_mem_er_search(unit, VRF_VFI_INTFm, MEM_BLOCK_ANY,
                           &index, &vrf_key, &vrfe, 0);
    if (rv >= 0) {
        ena = soc_VRF_VFI_INTFm_field32_get(unit, &vrfe, MPLS_ENABLEf);
        *enable = (ena ? TRUE : FALSE);
    } else if (rv == SOC_E_NOT_FOUND) {
        *enable = FALSE;
        rv = SOC_E_NONE;
    }

    return rv;
}

/*
 * Function:
 *      _bcm_er_mpls_free_resource
 * Purpose:
 *      Free all allocated tables and memory
 * Parameters:
 *      unit - SOC unit number
 * Returns:
 *      Nothing
 */
STATIC void
_bcm_er_mpls_free_resource(int unit)
{
    _bcm_mpls_bookkeeping_t *mpls_info;

    mpls_info = MPLS_INFO(unit);

    if (mpls_info->vpn_allocated) {
        sal_free(mpls_info->vpn_allocated);
        mpls_info->vpn_allocated = NULL;
    }

    if (mpls_info->vpn_l2_allocated) {
        sal_free(mpls_info->vpn_l2_allocated);
        mpls_info->vpn_l2_allocated = NULL;
    }

    if (mpls_info->exp_map_allocated) {
        sal_free(mpls_info->exp_map_allocated);
        mpls_info->exp_map_allocated = NULL;
    }

    if (mpls_info->ing_exp_map_allocated) {
        sal_free(mpls_info->ing_exp_map_allocated);
        mpls_info->ing_exp_map_allocated = NULL;
    }

    if (mpls_info->l2_info) {
        sal_free(mpls_info->l2_info);
        mpls_info->l2_info = NULL;
    }
}

/*
 * Function:
 *      bcm_mpls_init
 * Purpose:
 *      Initialize the MPLS software module, clear all HW MPLS states
 * Parameters:
 *      unit     - Device Number
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_er_mpls_init(int unit)
{
    int vfi, i, index, rv;
    _bcm_mpls_bookkeeping_t *mpls_info = MPLS_INFO(unit);
    l3intf_qos_entry_t l3_if_qos;

    if (!L3_INFO(unit)->l3_initialized) {
        soc_cm_debug(DK_L3, "L3 module must be initialized first\n");
	return BCM_E_NONE;
    }

    mpls_info->vpn_max = BCM_VPN_MAX + 1;
    mpls_info->vc_table_size = soc_mem_index_count(unit, VPLS_LABELm);
    mpls_info->mpls_switch_max = soc_mem_index_count(unit, L3_DEFIP_ALGm);
    if (mpls_info->mpls_switch_max <= 0) {
        mpls_info->mpls_switch_max = soc_mem_index_count(unit, L3_DEFIP_CAMm);
    }

    /* Init the VPLS_LABEL table */ 
    SOC_IF_ERROR_RETURN (soc_mem_clear(unit, VPLS_LABELm, MEM_BLOCK_ALL, 0));

    /* Init the VFI_BITMAP table */ 
    SOC_IF_ERROR_RETURN (soc_mem_clear(unit, VFI_BITMAPm, MEM_BLOCK_ALL, 0));

    if (!mpls_info->mpls_initialized) {
        /*
         * allocate resources
         */
        mpls_info->vpn_allocated =
            sal_alloc(SHR_BITALLOCSIZE(mpls_info->vpn_max), "vpn");
        if (mpls_info->vpn_allocated == NULL) {
            _bcm_er_mpls_free_resource(unit);
            return BCM_E_MEMORY;
        }

        mpls_info->vpn_l2_allocated =
            sal_alloc(SHR_BITALLOCSIZE(mpls_info->vpn_max), "L2VC");
        if (mpls_info->vpn_l2_allocated == NULL) {
            _bcm_er_mpls_free_resource(unit);
            return BCM_E_MEMORY;
        }

        /* Keep track of MPLS EXP map indexes based on 
         * the number of EXP map table instances. Each
         * instance has (#prio * #color) entries. For Easyrider
         * there are 8 priorities and 4 colors.
         */
        mpls_info->exp_map_max = 
            soc_mem_index_count(unit, EGRESS_DSCP_EXPm) / (8 * 4);
        mpls_info->exp_map_allocated =
            sal_alloc(SHR_BITALLOCSIZE(mpls_info->exp_map_max), "EXP-map");
        if (mpls_info->exp_map_allocated == NULL) {
            _bcm_er_mpls_free_resource(unit);
            return BCM_E_MEMORY;
        }

        /* Keep track of ingress MPLS EXP map indexes */
        mpls_info->ing_exp_map_max = soc_mem_index_count(unit, MPLS_EXPm) / 8;
        mpls_info->ing_exp_map_allocated =
            sal_alloc(SHR_BITALLOCSIZE(mpls_info->ing_exp_map_max), "ING EXP-map");
        if (mpls_info->ing_exp_map_allocated == NULL) {
            _bcm_er_mpls_free_resource(unit);
            return BCM_E_MEMORY;
        }

    
        mpls_info->l2_info =
            sal_alloc(sizeof(_bcm_er_mpls_l2_info_t) * mpls_info->vpn_max, 
                      "MPLS L2 info");
        if (mpls_info->l2_info == NULL) {
            _bcm_er_mpls_free_resource(unit);
            return BCM_E_MEMORY;
        }
    }

    sal_memset(mpls_info->vpn_allocated, 0,
               SHR_BITALLOCSIZE(mpls_info->vpn_max));

    sal_memset(mpls_info->vpn_l2_allocated, 0,
               SHR_BITALLOCSIZE(mpls_info->vpn_max));
    /* Skip VFI 0 by marking it as used */
    BCM_L2_VPN_USED_SET(unit, 0);

    sal_memset(mpls_info->exp_map_allocated, 0,
               SHR_BITALLOCSIZE(mpls_info->exp_map_max));

    sal_memset(mpls_info->ing_exp_map_allocated, 0,
               SHR_BITALLOCSIZE(mpls_info->ing_exp_map_max));

    sal_memset(mpls_info->l2_info, 0,
               (sizeof(_bcm_er_mpls_l2_info_t) * mpls_info->vpn_max));
    for (vfi = 0; vfi < mpls_info->vpn_max; vfi++) {
        mpls_info->l2_info[vfi].cu_vlan = BCM_VLAN_INVALID;
        mpls_info->l2_info[vfi].vc_ingress_label = BCM_MPLS_LABEL_INVALID;
    }


    /*
     * Initialize 8 L3_INTF_QOS entries with identity mapping.
     */
    for (i = 0; i < mpls_info->exp_map_max; i++) {
        sal_memset(&l3_if_qos, 0, sizeof(l3intf_qos_entry_t));
        index = _BCM_ER_MPLS_L3_INTF_BASE + i;
        soc_L3INTF_QOSm_field32_set(unit, &l3_if_qos, QOS_INDEXf, i);
        rv = WRITE_L3INTF_QOSm(unit, MEM_BLOCK_ANY, index, &l3_if_qos);
        if (rv < 0) {
            _bcm_er_mpls_free_resource(unit);
            return rv;
        }
    }

    BCM_IF_ERROR_RETURN (bcm_er_mpls_enable(unit, TRUE));

    mpls_info->mpls_initialized = TRUE;
    if (!soc_feature(unit, soc_feature_mpls_bos_lookup)) {
        /* Only the OLD style APIs are supported since the new style
         * require a feature which performs a second MPLS lookup
         * if the popped label has Bottom-of-Stack bit == 0.
         */
        mpls_info->api_style = _BCM_ER_MPLS_API_STYLE_OLD;
    } else {
        /* Style set to NONE, it will get locked in after the first
         * API call (old vs new).
         */
        mpls_info->api_style = _BCM_ER_MPLS_API_STYLE_NONE;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_cleanup
 * Purpose:
 *      Detach the MPLS software module, clear all HW MPLS states
 * Parameters:
 *      unit     - Device Number
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_er_mpls_cleanup(int unit)
{
    _bcm_mpls_bookkeeping_t *mpls_info = MPLS_INFO(unit);
    bcm_vpn_t vpn;
    int i;

    if (!_bcm_mpls_bk_info[unit].mpls_initialized) {  
        return (BCM_E_NONE);
    }                                                 

    if (!L3_INFO(unit)->l3_initialized) {
        soc_cm_debug(DK_L3, "L3 module must be initialized first\n");
	return BCM_E_NONE;
    } else if (mpls_info->mpls_initialized != TRUE) {
        return BCM_E_NONE;
    }

    if (mpls_info->api_style == _BCM_ER_MPLS_API_STYLE_OLD) {
        (void) bcm_er_mpls_switch_delete_all(unit, 0);
        (void) bcm_er_mpls_l3_initiator_clear_all(unit);
        for (i = 0; i <= BCM_VPN_MAX; i++) {
            if (BCM_MPLS_VPN_USED_GET(unit, i)) {
                (void) bcm_er_mpls_vpn_circuit_delete_all(unit, i);
                (void) bcm_er_mpls_vpn_delete_all(unit, i);
                (void) bcm_er_mpls_vpn_destroy(unit, i);
            }
        }
    } else if (mpls_info->api_style == _BCM_ER_MPLS_API_STYLE_NEW) {
        (void) bcm_er_mpls_tunnel_switch_delete_all(unit);
        (void) bcm_er_mpls_tunnel_initiator_clear_all(unit);
        for (i = 0; i <= BCM_VPN_MAX; i++) {
            if (BCM_L2_VPN_USED_GET(unit, i)) {
                _BCM_MPLS_VPN_SET(vpn, _BCM_MPLS_VPN_TYPE_VPWS, i);
                (void) bcm_er_mpls_vpn_id_destroy(unit, vpn);
            }
            if (BCM_MPLS_VPN_USED_GET(unit, i)) {
                _BCM_MPLS_VPN_SET(vpn, _BCM_MPLS_VPN_TYPE_L3, i);
                (void) bcm_er_mpls_vpn_id_destroy(unit, vpn);
            }
        }
    }
    for (i = 0; i < mpls_info->exp_map_max; i++) {
        if (BCM_MPLS_EXP_MAP_USED_GET(unit, i)) {
             BCM_MPLS_EXP_MAP_USED_CLR(unit, i);
        }
    }
    for (i = 0; i < mpls_info->ing_exp_map_max; i++) {
        if (BCM_MPLS_ING_EXP_MAP_USED_GET(unit, i)) {
             BCM_MPLS_ING_EXP_MAP_USED_CLR(unit, i);
        }
    }

    (void) bcm_er_mpls_enable(unit, FALSE);
    _bcm_er_mpls_free_resource(unit);
    sal_memset(mpls_info, 0, sizeof(_bcm_mpls_bookkeeping_t));
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_vpn_id_create
 * Purpose:
 *      Create a VPN instance
 * Parameters:
 *      unit       - (IN)  Device Number
 *      vpn        - (IN/OUT) VPN config info
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_mpls_vpn_id_create(int unit, bcm_mpls_vpn_config_t *info)
{
    int vfi, num_vfi;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_NEW);

    if ((info->flags & BCM_MPLS_VPN_VPWS) || (info->flags & BCM_MPLS_VPN_VPLS)) {
        num_vfi = soc_mem_index_count(unit, VPLS_LABELm);
        if (info->flags & BCM_MPLS_VPN_WITH_ID) {
            vfi = _BCM_MPLS_VPN_ID_GET(info->vpn);
            if (vfi > num_vfi) {
                return BCM_E_PARAM;
            } else if (BCM_L2_VPN_USED_GET(unit, vfi)) {
                return BCM_E_EXISTS;
            }
        } else {
            for (vfi = 0; vfi < num_vfi; vfi++) {
                if (!BCM_L2_VPN_USED_GET(unit, vfi)) {
                    break;
                }
            }
            if (vfi == num_vfi) {
                return BCM_E_FULL;
            }
            if (info->flags & BCM_MPLS_VPN_VPWS) {
                _BCM_MPLS_VPN_SET(info->vpn, _BCM_MPLS_VPN_TYPE_VPWS, vfi);
            } else {
                _BCM_MPLS_VPN_SET(info->vpn, _BCM_MPLS_VPN_TYPE_VPLS, vfi);
            }
        }
        BCM_L2_VPN_USED_SET(unit, vfi);

    } else if (info->flags & BCM_MPLS_VPN_L3) {
        int vrf;

        /*
         * The caller manages VRF space. An MPLS L3 VPN identifier
         * is simply the VRF value passed in by the caller (info->lookup_id)
         * added to a base offset. The VPN ID returned here can
         * be used in the bcm_mpls_tunnel_switch_* APIs.
         */
        if (info->flags & BCM_MPLS_VPN_WITH_ID) {
            vrf = _BCM_MPLS_VPN_ID_GET(info->vpn);
        } else {
            vrf = info->lookup_id;
        }

        if ((vrf < 0) || (vrf > SOC_VRF_MAX(unit))) {
            return BCM_E_PARAM;
        }
        if (BCM_MPLS_VPN_USED_GET(unit, vrf)) {
            return BCM_E_EXISTS;
        }
        BCM_MPLS_VPN_USED_SET(unit, vrf);
        _BCM_MPLS_VPN_SET(info->vpn, _BCM_MPLS_VPN_TYPE_L3, vrf);
    } else {
        return BCM_E_PARAM;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_vpn_id_destroy
 * Purpose:
 *      Delete a VPN instance
 * Parameters:
 *      unit - Device Number
 *      vpn - VPN instance ID
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_mpls_vpn_id_destroy (int unit, bcm_vpn_t vpn)
{
    int vfi;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_NEW);

    if (_BCM_MPLS_VPN_IS_VPLS(vpn) || _BCM_MPLS_VPN_IS_VPWS(vpn)) {
        vfi = _BCM_MPLS_VPN_ID_GET(vpn);
        BCM_IF_ERROR_RETURN (bcm_esw_mpls_port_delete_all(unit, vpn));
        BCM_L2_VPN_USED_CLR(unit, vfi);

    } else if (_BCM_MPLS_VPN_IS_L3(vpn)) {
        int vrf;

        vrf = _BCM_MPLS_VPN_ID_GET(vpn);
        if ((vrf < 0) || (vrf > SOC_VRF_MAX(unit))) {
            return BCM_E_PARAM;
        }
        if (!BCM_MPLS_VPN_USED_GET(unit, vrf)) {
            return BCM_E_NOT_FOUND;
        }
        BCM_MPLS_VPN_USED_CLR(unit, vrf);
    } else {
        return BCM_E_NOT_FOUND;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_vpn_id_destroy_all
 * Purpose:
 *      Delete all VPN instances
 * Parameters:
 *      unit - Device Number
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_mpls_vpn_id_destroy_all (int unit)
{
    int i;
    bcm_vpn_t vpn;

    for (i = 0; i <= BCM_VPN_MAX; i++) {
        if (BCM_L2_VPN_USED_GET(unit, i)) {
            _BCM_MPLS_VPN_SET(vpn, _BCM_MPLS_VPN_TYPE_VPWS, i);
            (void) bcm_er_mpls_vpn_id_destroy(unit, vpn);
        }
        if (BCM_MPLS_VPN_USED_GET(unit, i)) {
            _BCM_MPLS_VPN_SET(vpn, _BCM_MPLS_VPN_TYPE_L3, i);
            (void) bcm_er_mpls_vpn_id_destroy(unit, vpn);
        }
    }
    return BCM_E_NONE;
}

int
bcm_er_mpls_port_add(int unit, bcm_vpn_t vpn, bcm_mpls_port_t *mpls_port)
{
    _bcm_mpls_bookkeeping_t *mpls_info = MPLS_INFO(unit);
    int vfi = 0, rv = BCM_E_PARAM, lpm_updated = 0, isLocal;
    int mac_idx, gport_id, i, num_l2ue, index, l2ue_idx = 0;
    vpls_label_entry_t vle;
    bcm_l3_egress_t nh_info;
    bcm_l2_cache_addr_t l2ue;
    vrf_vfi_intf_entry_t vrfe;
    bcm_module_t mod_out=0;
    bcm_port_t port_out=0;
    bcm_trunk_t trunk_id;
    defip_alg_entry_t lpm_entry;
    l3intf_entry_t l3_if_entry;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_NEW);

    if (_BCM_MPLS_VPN_IS_VPLS(vpn) || _BCM_MPLS_VPN_IS_VPWS(vpn)) {
        vfi = _BCM_MPLS_VPN_ID_GET(vpn);
    } else {
        return BCM_E_PARAM;
    }

    if ((mpls_port->criteria != BCM_MPLS_PORT_MATCH_PORT_VLAN) &&
        (mpls_port->criteria != BCM_MPLS_PORT_MATCH_LABEL) &&
        (mpls_port->criteria != BCM_MPLS_PORT_MATCH_LABEL_PORT)) {
        return BCM_E_PARAM;
    }

    if (mpls_port->port != BCM_GPORT_INVALID) {
        rv = _bcm_esw_gport_resolve(unit, mpls_port->port, &mod_out,
                                    &port_out, &trunk_id, &gport_id);
        BCM_IF_ERROR_RETURN(rv);

        BCM_IF_ERROR_RETURN(
            _bcm_esw_modid_is_local(unit, mod_out, &isLocal));

        if (BCM_GPORT_IS_TRUNK(mpls_port->port) || (!isLocal)) {
            return BCM_E_PARAM;
        }
    }

    if (mpls_port->criteria == BCM_MPLS_PORT_MATCH_PORT_VLAN) {
        if (mpls_info->l2_info[vfi].cu_vlan != BCM_VLAN_INVALID) {
            return BCM_E_EXISTS;
        }

        /* 
         * Check for an entry already using the specified vlan.
         * We can't really do unique matching per {vlan + port}
         * since the VFI is determined solely by the VLAN. [Background: We're 
         * forced to ignore the source port portion of the VRF_VFI_INTF
         * table key since this table is used to determine the VRF
         * for L3 packets. Since we don't want to add a VRF_VFI_INTF
         * table entry for every port in the L3 interface's VLAN, 
         * the source port portion of the key isn't used.]
         */
        sal_memset(&vrfe, 0, sizeof(vrf_vfi_intf_entry_t));
        soc_VRF_VFI_INTFm_field32_set(unit, &vrfe, SRC_MODIDf, 0x3f);
        soc_VRF_VFI_INTFm_field32_set(unit, &vrfe, TGIDf, 0x3f);
        soc_VRF_VFI_INTFm_field32_set(unit, &vrfe, VLANf, mpls_port->match_vlan);
        rv = soc_mem_search(unit, VRF_VFI_INTFm, MEM_BLOCK_ANY, &i,
                            &vrfe, &vrfe, 0);
        if (rv == SOC_E_NONE) {
            return BCM_E_EXISTS;
        }

        /* Build VRF_VFI_INTFm entry. In the port table, the IGNORE_PORT_ID
         * is set, therefore the key to this table is the vlan id.
         * Set SRC_MODID,TGID to don't care, 0xfff.
         */
        sal_memset(&vrfe, 0, sizeof(vrf_vfi_intf_entry_t));
        soc_VRF_VFI_INTFm_field32_set(unit, &vrfe, SRC_MODIDf, 0x3f);
        soc_VRF_VFI_INTFm_field32_set(unit, &vrfe, TGIDf, 0x3f);
        soc_VRF_VFI_INTFm_field32_set(unit, &vrfe, VLANf, mpls_port->match_vlan);
        soc_VRF_VFI_INTFm_field32_set(unit, &vrfe, VRF_VFIf, vfi);
        soc_VRF_VFI_INTFm_field32_set(unit, &vrfe, VFIf, 1);
        if (SOC_MEM_FIELD_VALID(unit, VRF_VFI_INTFm, MPLS_ENABLEf)) {
            soc_VRF_VFI_INTFm_field32_set(unit, &vrfe, MPLS_ENABLEf, 1);
        }

        /* Build the L2_USER_ENTRY for customer-facing port */
        bcm_l2_cache_addr_t_init(&l2ue);
        l2ue.vlan_mask = BCM_L2_VID_MASK_ALL;
        l2ue.src_port_mask = BCM_L2_SRCPORT_MASK_ALL;
        l2ue.src_port = port_out;
        l2ue.vlan = mpls_port->match_vlan;
        if (mpls_info->l2_info[vfi].vc_ingress_label != BCM_MPLS_LABEL_INVALID) {
            /* network-facing port exists, point to tunnel destination port */
            rv = bcm_xgs3_l3_egress_get(unit, 
                                        mpls_info->l2_info[vfi].vc_tunnel_if, 
                                        &nh_info);
            BCM_IF_ERROR_RETURN(rv);

            /* Copy the MPLS tunnel destination port information. */
            if (nh_info.flags & BCM_L3_TGID) {
    /*    coverity[uninit_use]    */
                l2ue.dest_modid = BCM_TRUNK_TO_MODIDf(unit, nh_info.trunk);
                l2ue.dest_port = BCM_TRUNK_TO_TGIDf(unit, nh_info.trunk);
            } else {
                l2ue.dest_modid = nh_info.module;
                l2ue.dest_port = nh_info.port;
            }

            /* Find ILM of network-facing port */
            sal_memset(&lpm_entry, 0, sizeof(defip_alg_entry_t));
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, FFf,
                                          SOC_ER_DEFIP_FF_FIELD);
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, SRCMODf, 
                                          mpls_info->l2_info[vfi].vc_src_mod);
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, SRCPORTf, 
                                          mpls_info->l2_info[vfi].vc_src_port);
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, MPLS_LABEL1f,
                                          mpls_info->l2_info[vfi].vc_ingress_label);
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, KEY_TYPEf,
                                          SOC_ER_DEFIP_KEY_TYPE_MPLS_1L);
            MPLS_SWITCH_LOCK(unit);
            rv = soc_er_defip_lookup(unit, &index, &lpm_entry, &lpm_entry);
            if (rv < 0) {
                MPLS_SWITCH_UNLOCK(unit);
                return rv;
            }

            /* Update the destination port, vlan, and drop information */
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, MODIDf, mod_out);
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, PORT_TGIDf, port_out);
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, PVIDf,
                                          mpls_port->match_vlan);
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, DST_DISCARDf, 0);
            rv = soc_er_defip_insert(unit, &lpm_entry);
            if (rv == SOC_E_NOT_FOUND) {
                rv = SOC_E_NONE;
            }
            if (rv < 0) {
                MPLS_SWITCH_UNLOCK(unit);
                return rv;
            } else {
                lpm_updated = 1;
            }
        } else {
            /* network-facing port does not exist, drop incoming packets */
            l2ue.flags = BCM_L2_CACHE_DISCARD;
        }

        /* Install the VRF_VFI_INTF entry */
        rv = soc_mem_insert(unit, VRF_VFI_INTFm, MEM_BLOCK_ALL, &vrfe);
        if (rv < 0) {
            if (lpm_updated) {
                soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, DST_DISCARDf, 1);
                (void) soc_er_defip_insert(unit, &lpm_entry);
                MPLS_SWITCH_UNLOCK(unit);
            }
            return rv;
        }

        /* Install the L2_USER_ENTRY for customer-facing port */
        rv = bcm_esw_l2_cache_set(unit, -1, &l2ue, &l2ue_idx);
        if (rv < 0) {
            if (lpm_updated) {
                soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, DST_DISCARDf, 1);
                (void) soc_er_defip_insert(unit, &lpm_entry);
                MPLS_SWITCH_UNLOCK(unit);
            }
            (void) soc_mem_delete(unit, VRF_VFI_INTFm, MEM_BLOCK_ANY, &vrfe);
            return rv;
        } else if (lpm_updated) {
            MPLS_SWITCH_UNLOCK(unit);
        }

        /* Store port information */
        mpls_info->l2_info[vfi].cu_mod = mod_out;
        mpls_info->l2_info[vfi].cu_port = port_out;
        mpls_info->l2_info[vfi].cu_vlan = mpls_port->match_vlan;

        /* Uniquely identify mpls_port using match type & vfi */
        BCM_GPORT_MPLS_PORT_ID_SET(mpls_port->mpls_port_id, 
                                     ((mpls_port->criteria << 16) | vfi));
    } else { /* BCM_MPLS_PORT_MATCH_LABEL || BCM_MPLS_PORT_MATCH_LABEL_PORT */
        if (mpls_info->l2_info[vfi].vc_ingress_label != BCM_MPLS_LABEL_INVALID) {
            return BCM_E_EXISTS;
        } else if (!BCM_XGS3_L3_EGRESS_IDX_VALID(unit, mpls_port->egress_tunnel_if)) {
            /* Invalid MPLS tunnel interface. */
            return BCM_E_PARAM;
        }

        /* Get egress MPLS tunnel info */
        rv = bcm_xgs3_l3_egress_get(unit, mpls_port->egress_tunnel_if, &nh_info);
        BCM_IF_ERROR_RETURN(rv);

        /* Update L3 interface info (DMAC, TTL and EXP-map info) */
        MEM_LOCK(unit, L3INTFm);
        rv = READ_L3INTFm(unit, MEM_BLOCK_ANY, nh_info.intf, &l3_if_entry);
        if (rv < 0) {
            MEM_UNLOCK(unit, L3INTFm);
            return rv;
        }

        /* Add an adjacent MAC entry for the DMAC */
        rv = bcm_xgs3_adj_mac_add(unit, nh_info.mac_addr, &mac_idx);
        if (rv < 0) {
            MEM_UNLOCK(unit, L3INTFm);
            return rv;
        }

        soc_L3INTFm_field32_set(unit, &l3_if_entry, ADJ_MAC_INDEXf, mac_idx);
        soc_L3INTFm_field32_set(unit, &l3_if_entry, QOS_INDEXf,
                                mpls_port->egress_label.qos_map_id);
        if (mpls_port->egress_label.flags & BCM_MPLS_EGRESS_LABEL_TTL_SET) {
            soc_L3INTFm_field32_set(unit, &l3_if_entry, TTLf,
                                    mpls_port->egress_label.ttl);
        } else {
            soc_L3INTFm_field32_set(unit, &l3_if_entry, TTLf, 0);
        }

        /* Get the VLAN_ID from the L3_INTF_TABLE entry */
        nh_info.vlan = soc_L3INTFm_field32_get(unit, &l3_if_entry, VLAN_IDf);

        /* Write the VPLS_LABEL entry */
        sal_memset(&vle, 0, sizeof(vpls_label_entry_t));
        soc_VPLS_LABELm_field32_set(unit, &vle, ORIGINAL_PKTf, 1);
        soc_VPLS_LABELm_field32_set(unit, &vle, VIDf, nh_info.vlan);
        soc_VPLS_LABELm_field32_set(unit, &vle, L3_INTFf, nh_info.intf);
        soc_VPLS_LABELm_field32_set(unit, &vle, VC_LABELf, 
                                    mpls_port->egress_label.label);
        rv = WRITE_VPLS_LABELm(unit, MEM_BLOCK_ALL, vfi, &vle);
        if (rv < 0) {
            (void) bcm_xgs3_adj_mac_del(unit, mac_idx);
            MEM_UNLOCK(unit, L3INTFm);
            return rv;
        }

        /* Now write the L3 interface entry */
        rv = WRITE_L3INTFm(unit, MEM_BLOCK_ALL, nh_info.intf, &l3_if_entry);
        MEM_UNLOCK(unit, L3INTFm);
        if (rv < 0) {
            (void) bcm_xgs3_adj_mac_del(unit, mac_idx);
            return rv;
        }

        /* Install the ILM for the ingress label */
        sal_memset(&lpm_entry, 0, sizeof(defip_alg_entry_t));
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, FFf,
                                      SOC_ER_DEFIP_FF_FIELD);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, VALIDf, 1);
        if (mpls_port->criteria == BCM_MPLS_PORT_MATCH_LABEL_PORT) {
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, SRCMODf, mod_out); 
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, SRCPORTf, port_out);
        } else {
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, SRCMODf, 0x3f);
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, SRCPORTf, 0x3f);
        }
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, MPLS_LABEL1f,
                                      mpls_port->match_label);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, KEY_TYPEf,
                                      SOC_ER_DEFIP_KEY_TYPE_MPLS_1L);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry,
                                      MPLS_DONOT_CHANGE_INNER_L2f, 1);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry,
                                      MPLS_USE_OUTER_TTLf, 1);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry,
                                      MPLS_USE_OUTER_EXPf, 1);
        if (mpls_port->flags & BCM_MPLS_PORT_INT_PRI_SET) {
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, 
                                          MPLS_LABEL_PRIf, mpls_port->int_pri);
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, 
                                          MPLS_USE_PRIf, 0x1);
        } else {
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, 
                                          MPLS_USE_PRIf, 0x0);
        }
        /*
         * Set the incoming interface to one of the 8 reserved L3_INTF values.
         * Each one corresponds directly to one of the 8 EXP maps.
         */
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, MPLS_IINTFf,
                          _BCM_ER_MPLS_L3_INTF_BASE + 
                          (mpls_port->exp_map & _BCM_ER_MPLS_EXP_MAP_TABLE_NUM_MASK));

        if (_BCM_MPLS_VPN_IS_VPLS(vpn)) {
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, LEARNf, 1);
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, MPLS_ACTIONf,
                                          BCM_MPLS_OPCODE_POP_L2_SWITCH);
        } else {
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, MPLS_ACTIONf,
                                          BCM_MPLS_OPCODE_POP_DST_MOD_PORT);
        }
        if (mpls_info->l2_info[vfi].cu_vlan != BCM_VLAN_INVALID) {
            num_l2ue = soc_mem_index_count(unit, L2_USER_ENTRYm);
            /* For L2 VPNs, get the src port info from the L2_USER_ENTRYm */
            for (l2ue_idx = 0; l2ue_idx < num_l2ue; l2ue_idx++) {
                rv = bcm_esw_l2_cache_get(unit, l2ue_idx, &l2ue);
                if (BCM_FAILURE(rv)) {
                    if (rv == BCM_E_NOT_FOUND) {
                        continue;
                    } else {
                        return rv;
                    }
                }
                /* Check for matching VLAN_ID and MASK */
                if ((mpls_info->l2_info[vfi].cu_vlan == l2ue.vlan) &&
                    (BCM_MAC_IS_ZERO(l2ue.mac_mask)) &&
                    (BCM_L2_SRCPORT_MASK_ALL == l2ue.src_port_mask) &&
                    (BCM_L2_VID_MASK_ALL == l2ue.vlan_mask)) {
                    break;
                }
            }
            if (l2ue_idx == num_l2ue) {
                (void) bcm_xgs3_adj_mac_del(unit, mac_idx);
                return BCM_E_INTERNAL;
            }
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, MODIDf,
                                          l2ue.dest_modid); 
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, PORT_TGIDf,
                                          l2ue.src_port);
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, PVIDf,
                                          l2ue.vlan);

            /* Update the L2_USER_ENTRYm entry with destnation port info. */
            l2ue.flags &= ~(BCM_L2_CACHE_DISCARD);
            if (nh_info.flags & BCM_L3_TGID) {
                l2ue.dest_modid = BCM_TRUNK_TO_MODIDf(unit, nh_info.trunk);
                l2ue.dest_trunk = BCM_TRUNK_TO_TGIDf(unit, nh_info.trunk);
                l2ue.flags |= BCM_L2_CACHE_TRUNK;
            } else {
                l2ue.dest_modid = nh_info.module;
                l2ue.dest_port = nh_info.port;
            }
            rv = bcm_esw_l2_cache_set(unit, l2ue_idx, &l2ue, &l2ue_idx);
            if (rv < 0) {
                (void) bcm_xgs3_adj_mac_del(unit, mac_idx);
                return rv;
            }
        } else {
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, DST_DISCARDf, 1);
        }
        rv = soc_er_defip_insert(unit, &lpm_entry);
        if (rv == SOC_E_NOT_FOUND) {
            rv = SOC_E_NONE;
        }
        if (rv < 0) {
            (void) bcm_xgs3_adj_mac_del(unit, mac_idx);
            return rv;
        }

        /* Save port info */
        if (mpls_port->criteria == BCM_MPLS_PORT_MATCH_LABEL_PORT) {
            mpls_info->l2_info[vfi].vc_src_mod = mod_out;
            mpls_info->l2_info[vfi].vc_src_port = port_out;
        } else {
            mpls_info->l2_info[vfi].vc_src_mod = 0x3f;
            mpls_info->l2_info[vfi].vc_src_port = 0x3f;
        }
        mpls_info->l2_info[vfi].vc_ingress_label = mpls_port->match_label;
        mpls_info->l2_info[vfi].vc_tunnel_if = mpls_port->egress_tunnel_if;

        /* Uniquely identify mpls_port using match type & vfi */
        BCM_GPORT_MPLS_PORT_ID_SET(mpls_port->mpls_port_id, 
                                     ((mpls_port->criteria << 16) | vfi));
    }
    return rv;
}

/*
 * Function:
 *      bcm_mpls_port_delete
 * Purpose:
 *      Delete an mpls port from a VPN
 * Parameters:
 *      unit       - (IN) Device Number
 *      vpn        - (IN) VPN instance ID
 *      mpls_port_id - (IN) mpls port ID
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_mpls_port_delete(int unit, bcm_vpn_t vpn, bcm_gport_t mpls_port_id)
{
    _bcm_mpls_bookkeeping_t *mpls_info = MPLS_INFO(unit);
    int rv, l3_intf, l2ue_idx = 0, num_l2ue; 
    int old_mac_idx, vfi, match_type, index;
    bcm_l2_cache_addr_t l2ue;
    vrf_vfi_intf_entry_t vrfe;
    vpls_label_entry_t vle;
    l3intf_entry_t l3_if_entry;
    defip_alg_entry_t lpm_entry;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_NEW);

    if (_BCM_MPLS_VPN_IS_VPLS(vpn) || _BCM_MPLS_VPN_IS_VPWS(vpn)) {
        vfi = _BCM_MPLS_VPN_ID_GET(vpn);
    } else {
        return BCM_E_PARAM;
    }

    if (mpls_info->l2_info[vfi].cu_vlan != BCM_VLAN_INVALID) {
        num_l2ue = soc_mem_index_count(unit, L2_USER_ENTRYm);
        /* For L2 VPNs, get the src port info from the L2_USER_ENTRYm */
        for (l2ue_idx = 0; l2ue_idx < num_l2ue; l2ue_idx++) {
            rv = bcm_esw_l2_cache_get(unit, l2ue_idx, &l2ue);
            if (BCM_FAILURE(rv)) {
                if (rv == BCM_E_NOT_FOUND) {
                    continue;
                } else {
                    return rv;
                }
            }
            /* Check for matching VLAN_ID and MASK */
            if ((mpls_info->l2_info[vfi].cu_vlan == l2ue.vlan) &&
                (BCM_MAC_IS_ZERO(l2ue.mac_mask)) &&
                (BCM_L2_SRCPORT_MASK_ALL == l2ue.src_port_mask) &&
                (BCM_L2_VID_MASK_ALL == l2ue.vlan_mask)) {
                break;
            }
        }
        if (l2ue_idx == num_l2ue) {
            return BCM_E_NOT_FOUND;
        }
    }
    if (mpls_info->l2_info[vfi].vc_ingress_label != BCM_MPLS_LABEL_INVALID) {
        /* Find ILM of network-facing port */
        sal_memset(&lpm_entry, 0, sizeof(defip_alg_entry_t));
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, FFf,
                                      SOC_ER_DEFIP_FF_FIELD);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, SRCMODf,
                                      mpls_info->l2_info[vfi].vc_src_mod);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, SRCPORTf,
                                      mpls_info->l2_info[vfi].vc_src_port);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, MPLS_LABEL1f,
                                      mpls_info->l2_info[vfi].vc_ingress_label);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, KEY_TYPEf,
                                      SOC_ER_DEFIP_KEY_TYPE_MPLS_1L);
    }

    match_type = BCM_GPORT_MPLS_PORT_ID_GET(mpls_port_id) >> 16;
    if (match_type == BCM_MPLS_PORT_MATCH_PORT_VLAN) {
        if (mpls_info->l2_info[vfi].cu_vlan == BCM_VLAN_INVALID) {
            return BCM_E_NOT_FOUND;
        }

        sal_memset(&vrfe, 0, sizeof(vrf_vfi_intf_entry_t));
        soc_VRF_VFI_INTFm_field32_set(unit, &vrfe, SRC_MODIDf, 0x3f);
        soc_VRF_VFI_INTFm_field32_set(unit, &vrfe, TGIDf, 0x3f);
        soc_VRF_VFI_INTFm_field32_set(unit, &vrfe, VLANf, 
                                      mpls_info->l2_info[vfi].cu_vlan);
        rv = soc_mem_search(unit, VRF_VFI_INTFm, MEM_BLOCK_ANY, &index,
                            &vrfe, &vrfe, 0);
        BCM_IF_ERROR_RETURN(rv);

        if (mpls_info->l2_info[vfi].vc_ingress_label != BCM_MPLS_LABEL_INVALID) {
            MPLS_SWITCH_LOCK(unit);
            rv = soc_er_defip_lookup(unit, &index, &lpm_entry, &lpm_entry);
            if (rv < 0) {
                MPLS_SWITCH_UNLOCK(unit);
                return rv;
            }
            /* Update the destination port, vlan, and drop information */
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, MODIDf, 0);
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, PORT_TGIDf, 0);
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, PVIDf, 0);
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, DST_DISCARDf, 1);
            rv = soc_er_defip_insert(unit, &lpm_entry);
            MPLS_SWITCH_UNLOCK(unit);
            if (rv == SOC_E_NOT_FOUND) {
                rv = SOC_E_NONE;
            }
            BCM_IF_ERROR_RETURN(rv);
        } 
        rv = soc_mem_delete(unit, VRF_VFI_INTFm, MEM_BLOCK_ALL, &vrfe);
        BCM_IF_ERROR_RETURN(rv);

        rv = bcm_esw_l2_cache_delete(unit, l2ue_idx);
        BCM_IF_ERROR_RETURN(rv);

        mpls_info->l2_info[vfi].cu_vlan = BCM_VLAN_INVALID;
    } else { /* BCM_MPLS_PORT_MATCH_LABEL || BCM_MPLS_PORT_MATCH_LABEL_PORT */
        if (mpls_info->l2_info[vfi].vc_ingress_label == BCM_MPLS_LABEL_INVALID) {
            return BCM_E_NOT_FOUND;
        }
        if (mpls_info->l2_info[vfi].cu_vlan != BCM_VLAN_INVALID) {
            /* Update the L2_USER_ENTRYm entry with destnation port info. */
            l2ue.flags |= BCM_L2_CACHE_DISCARD;
            l2ue.dest_modid = 0;
            l2ue.dest_port = 0;
            rv = bcm_esw_l2_cache_set(unit, l2ue_idx, &l2ue, &l2ue_idx);
            BCM_IF_ERROR_RETURN(rv);
        }

        /* Delete the ILM */
        rv = soc_er_defip_delete(unit, &lpm_entry);
        BCM_IF_ERROR_RETURN(rv);

        rv = READ_VPLS_LABELm(unit, MEM_BLOCK_ALL, vfi, &vle);
        l3_intf = soc_VPLS_LABELm_field32_get(unit, &vle, L3_INTFf);

        /* Update L3 interface info (TTL and EXP-map info) */
        MEM_LOCK(unit, L3INTFm);
        rv = READ_L3INTFm(unit, MEM_BLOCK_ANY, l3_intf, &l3_if_entry);
        if (rv < 0) {
            MEM_UNLOCK(unit, L3INTFm);
            return rv;
        }
        old_mac_idx = soc_L3INTFm_field32_get(unit, &l3_if_entry, ADJ_MAC_INDEXf);
        soc_L3INTFm_field32_set(unit, &l3_if_entry, ADJ_MAC_INDEXf, 0);
        soc_L3INTFm_field32_set(unit, &l3_if_entry, QOS_INDEXf, 0);
        soc_L3INTFm_field32_set(unit, &l3_if_entry, TTLf, 0);
        rv = WRITE_L3INTFm(unit, MEM_BLOCK_ALL, l3_intf, &l3_if_entry);
        MEM_UNLOCK(unit, L3INTFm);
        BCM_IF_ERROR_RETURN(rv);

        /* Clear the VPLS_LABEL entry */
        soc_VPLS_LABELm_field32_set(unit, &vle, ORIGINAL_PKTf, 0);
        soc_VPLS_LABELm_field32_set(unit, &vle, VIDf, 0);
        soc_VPLS_LABELm_field32_set(unit, &vle, L3_INTFf, 0);
        soc_VPLS_LABELm_field32_set(unit, &vle, VC_LABELf, 0);
        rv = WRITE_VPLS_LABELm(unit, MEM_BLOCK_ALL, vfi, &vle);
        BCM_IF_ERROR_RETURN(rv);

        /* Free the adjacent mac index */
        BCM_IF_ERROR_RETURN(bcm_xgs3_adj_mac_del(unit, old_mac_idx));

        mpls_info->l2_info[vfi].vc_ingress_label = BCM_MPLS_LABEL_INVALID;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_port_delete_all
 * Purpose:
 *      Delete all mpls ports from a VPN
 * Parameters:
 *      unit - Device Number
 *      vpn - VPN instance ID
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_mpls_port_delete_all(int unit, bcm_vpn_t vpn)
{
    _bcm_mpls_bookkeeping_t *mpls_info = MPLS_INFO(unit);
    int vfi = 0, rv, criteria;
    bcm_gport_t mpls_port_id;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_NEW);

    if (_BCM_MPLS_VPN_IS_VPLS(vpn) || _BCM_MPLS_VPN_IS_VPWS(vpn)) {
        vfi = _BCM_MPLS_VPN_ID_GET(vpn);
    } else {
        return BCM_E_PARAM;
    }

    if (BCM_L2_VPN_USED_GET(unit, vfi)) {
        if (mpls_info->l2_info[vfi].cu_vlan != BCM_VLAN_INVALID) {
            criteria = BCM_MPLS_PORT_MATCH_PORT_VLAN;
            BCM_GPORT_MPLS_PORT_ID_SET(mpls_port_id, (criteria << 16) | vfi);
            rv = bcm_esw_mpls_port_delete(unit, vpn, mpls_port_id);
            BCM_IF_ERROR_RETURN(rv);
        }
        if (mpls_info->l2_info[vfi].vc_ingress_label != BCM_MPLS_LABEL_INVALID) {
            if (mpls_info->l2_info[vfi].vc_src_mod == 0x3f) {
                criteria = BCM_MPLS_PORT_MATCH_LABEL;
            } else {
                criteria = BCM_MPLS_PORT_MATCH_LABEL_PORT;
            }
            BCM_GPORT_MPLS_PORT_ID_SET(mpls_port_id, (criteria << 16) | vfi);
            rv = bcm_esw_mpls_port_delete(unit, vpn, mpls_port_id);
            BCM_IF_ERROR_RETURN(rv);
        }
    } else {
        return BCM_E_NOT_FOUND;
    }
    return BCM_E_NONE;
}

STATIC int
_bcm_er_mpls_port_get(int unit, int vfi, int mpls_port_id, 
                      bcm_mpls_port_t *mpls_port)
{
    _bcm_mpls_bookkeeping_t *mpls_info = MPLS_INFO(unit);
    int rv, l3_intf, l2ue_idx = 0, num_l2ue, match_type, index;
    bcm_l2_cache_addr_t l2ue;
    vpls_label_entry_t vle;
    l3intf_entry_t l3_if_entry;
    defip_alg_entry_t lpm_entry;
    bcm_module_t mod_out;
    bcm_port_t port_out;

    match_type = BCM_GPORT_MPLS_PORT_ID_GET(mpls_port_id) >> 16;
    if (match_type == BCM_MPLS_PORT_MATCH_PORT_VLAN) {
        if (mpls_info->l2_info[vfi].cu_vlan == BCM_VLAN_INVALID) {
            return BCM_E_NOT_FOUND;
        }
        num_l2ue = soc_mem_index_count(unit, L2_USER_ENTRYm);
        /* For L2 VPNs, get the src port info from the L2_USER_ENTRYm */
        for (l2ue_idx = 0; l2ue_idx < num_l2ue; l2ue_idx++) {
            rv = bcm_esw_l2_cache_get(unit, l2ue_idx, &l2ue);
            if (BCM_FAILURE(rv)) {
                if (rv == BCM_E_NOT_FOUND) {
                    continue;
                } else {
                    return rv;
                }
            }
            /* Check for matching VLAN_ID and MASK */
            if ((mpls_info->l2_info[vfi].cu_vlan == l2ue.vlan) &&
                (BCM_MAC_IS_ZERO(l2ue.mac_mask)) &&
                (BCM_L2_SRCPORT_MASK_ALL == l2ue.src_port_mask) &&
                (BCM_L2_VID_MASK_ALL == l2ue.vlan_mask)) {
                break;
            }
        }
        if (l2ue_idx == num_l2ue) {
            return BCM_E_NOT_FOUND;
        }

        BCM_IF_ERROR_RETURN
            (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                    mpls_info->l2_info[vfi].cu_mod, 
                                    mpls_info->l2_info[vfi].cu_port, 
                                    &mod_out, &port_out));
        sal_memset(mpls_port, 0, sizeof(*mpls_port));
        mpls_port->mpls_port_id = mpls_port_id;
        BCM_GPORT_MODPORT_SET(mpls_port->port, mod_out, port_out);
        mpls_port->criteria = BCM_MPLS_PORT_MATCH_PORT_VLAN;
        mpls_port->match_vlan = mpls_info->l2_info[vfi].cu_vlan;

    } else { /* BCM_MPLS_PORT_MATCH_LABEL || BCM_MPLS_PORT_MATCH_LABEL_PORT */
        if (mpls_info->l2_info[vfi].vc_ingress_label == BCM_MPLS_LABEL_INVALID) {
            return BCM_E_NOT_FOUND;
        }
        sal_memset(mpls_port, 0, sizeof(bcm_mpls_port_t));
        mpls_port->mpls_port_id = mpls_port_id;
        mpls_port->egress_tunnel_if = mpls_info->l2_info[vfi].vc_tunnel_if;
        mpls_port->match_label = mpls_info->l2_info[vfi].vc_ingress_label;

        /* Find the egress object information */
        rv = READ_VPLS_LABELm(unit, MEM_BLOCK_ALL, vfi, &vle);
        BCM_IF_ERROR_RETURN(rv);
        mpls_port->egress_label.label = 
            soc_VPLS_LABELm_field32_get(unit, &vle, VC_LABELf);

        l3_intf = soc_VPLS_LABELm_field32_get(unit, &vle, L3_INTFf);
        BCM_IF_ERROR_RETURN
            (READ_L3INTFm(unit, MEM_BLOCK_ANY, l3_intf, &l3_if_entry));
        mpls_port->egress_label.qos_map_id = 
            soc_L3INTFm_field32_get(unit, &l3_if_entry, QOS_INDEXf);
        if (soc_L3INTFm_field32_get(unit, &l3_if_entry, TTLf)) {
            mpls_port->egress_label.flags = BCM_MPLS_EGRESS_LABEL_TTL_SET;
            mpls_port->egress_label.ttl = 
                soc_L3INTFm_field32_get(unit, &l3_if_entry, TTLf);
        } else {
            mpls_port->egress_label.flags = 0;
            mpls_port->egress_label.ttl = 0;
        }
        if (mpls_info->l2_info[vfi].vc_src_mod == 0x3f) {
            mpls_port->criteria = BCM_MPLS_PORT_MATCH_LABEL;
        } else {
            mpls_port->criteria = BCM_MPLS_PORT_MATCH_LABEL_PORT;
        }

        /* Find ILM of network-facing port */
        sal_memset(&lpm_entry, 0, sizeof(defip_alg_entry_t));
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, FFf,
                                      SOC_ER_DEFIP_FF_FIELD);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, SRCMODf,
                                      mpls_info->l2_info[vfi].vc_src_mod);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, SRCPORTf,
                                      mpls_info->l2_info[vfi].vc_src_port);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, MPLS_LABEL1f,
                                      mpls_info->l2_info[vfi].vc_ingress_label);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, KEY_TYPEf,
                                      SOC_ER_DEFIP_KEY_TYPE_MPLS_1L);
        rv = soc_er_defip_lookup(unit, &index, &lpm_entry, &lpm_entry);
        BCM_IF_ERROR_RETURN(rv);

        if (soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, MPLS_USE_PRIf)) {
            mpls_port->flags |= BCM_MPLS_PORT_INT_PRI_SET;
            mpls_port->int_pri = 
                soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, MPLS_LABEL_PRIf);
        } else {
            mpls_port->flags |= BCM_MPLS_PORT_INT_PRI_MAP;
        }
        /* Easyrider always uses exp_map to get color */
        mpls_port->flags |= BCM_MPLS_PORT_COLOR_MAP;
        mpls_port->exp_map = 
            soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, MPLS_IINTFf);
        mpls_port->exp_map -= _BCM_ER_MPLS_L3_INTF_BASE;
        mpls_port->exp_map |= _BCM_ER_MPLS_EXP_MAP_TABLE_TYPE_INGRESS;

        mpls_port->flags |= BCM_MPLS_PORT_EGRESS_TUNNEL;
        mpls_port->flags |= BCM_MPLS_PORT_NETWORK;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_port_get
 * Purpose:
 *      Get an mpls port from a VPN
 * Parameters:
 *      unit    - (IN) Device Number
 *      vpn     - (IN) VPN instance ID
 *      mpls_port - (IN/OUT) mpls port information (IN : mpls_port_id)
 */
int
bcm_er_mpls_port_get(int unit, bcm_vpn_t vpn, bcm_mpls_port_t *mpls_port)
{
    int vfi;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_NEW);

    if (_BCM_MPLS_VPN_IS_VPLS(vpn) || _BCM_MPLS_VPN_IS_VPWS(vpn)) {
        vfi = _BCM_MPLS_VPN_ID_GET(vpn);
    } else {
        return BCM_E_PARAM;
    }
    return (_bcm_er_mpls_port_get(unit, vfi, mpls_port->mpls_port_id,
                                  mpls_port));
}

/*
 * Function:
 *      bcm_mpls_port_get_all
 * Purpose:
 *      Get an mpls port from a VPN
 * Parameters:
 *      unit     - (IN) Device Number
 *      vpn      - (IN) VPN instance ID
 *      port_max   - (IN) Maximum number of ports in array
 *      port_array - (OUT) Array of mpls ports
 *      port_count - (OUT) Number of ports returned in array
 *
 */
int
bcm_er_mpls_port_get_all(int unit, bcm_vpn_t vpn, int port_max,
                           bcm_mpls_port_t *port_array, int *port_count)
{
    _bcm_mpls_bookkeeping_t *mpls_info = MPLS_INFO(unit);
    int vfi = 0, rv, criteria;
    bcm_gport_t mpls_port_id;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_NEW);

    if (_BCM_MPLS_VPN_IS_VPLS(vpn) || _BCM_MPLS_VPN_IS_VPWS(vpn)) {
        vfi = _BCM_MPLS_VPN_ID_GET(vpn);
    } else {
        return BCM_E_PARAM;
    }

    if (BCM_L2_VPN_USED_GET(unit, vfi)) {
        *port_count = 0;
        if ((mpls_info->l2_info[vfi].cu_vlan != BCM_VLAN_INVALID) &&
            ((*port_count + 1) <= port_max)) {
            criteria = BCM_MPLS_PORT_MATCH_PORT_VLAN;
            BCM_GPORT_MPLS_PORT_ID_SET(mpls_port_id, (criteria << 16) | vfi);
            rv = _bcm_er_mpls_port_get(unit, vfi, mpls_port_id, 
                                       &port_array[*port_count]);
            BCM_IF_ERROR_RETURN(rv);
            *port_count = *port_count + 1;
        }
        if ((mpls_info->l2_info[vfi].vc_ingress_label != BCM_MPLS_LABEL_INVALID) &&
            ((*port_count + 1) <= port_max)) {
            if (mpls_info->l2_info[vfi].vc_src_mod == 0x3f) {
                criteria = BCM_MPLS_PORT_MATCH_LABEL;
            } else {
                criteria = BCM_MPLS_PORT_MATCH_LABEL_PORT;
            }
            BCM_GPORT_MPLS_PORT_ID_SET(mpls_port_id, (criteria << 16) | vfi);
            rv = _bcm_er_mpls_port_get(unit, vfi, mpls_port_id, 
                                       &port_array[*port_count]);
            BCM_IF_ERROR_RETURN(rv);
            *port_count = *port_count + 1;
        }
    } else {
        return BCM_E_NOT_FOUND;
    }
    return BCM_E_NONE;
}

int
bcm_er_mpls_tunnel_initiator_set(int unit, bcm_if_t intf, int num_labels,
                                 bcm_mpls_egress_label_t *label_array)
{
    l3intf_entry_t l3_if_entry;
    int rv;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_NEW);

    if ((num_labels <= 0) || (num_labels > 2) || (label_array == NULL) ||
         (intf >= L3_INFO(unit)->l3_intf_table_size)) {
        return BCM_E_PARAM;
    } else if (!BCM_L3_INTF_USED_GET(unit, intf)) {
        soc_cm_debug(DK_L3, "L3 interface not created\n");
        return BCM_E_NOT_FOUND;
    }

    /* L3 interface info */
    MEM_LOCK(unit, L3INTFm);
    rv = READ_L3INTFm(unit, MEM_BLOCK_ANY, intf, &l3_if_entry);
    if (rv < 0) {
        MEM_UNLOCK(unit, L3INTFm);
        return rv;
    }

    soc_L3INTFm_field32_set(unit, &l3_if_entry, TUNNEL_TYPEf, 8); /* MPLS */
    if (num_labels == 1) {
        soc_L3INTFm_field32_set(unit, &l3_if_entry, INSERT_L3_MPLS_LABEL1f, 1);
        soc_L3INTFm_field32_set(unit, &l3_if_entry, MPLS_TUNNEL_LABEL1f,
                                label_array[0].label);
    } else if (num_labels == 2) {
        soc_L3INTFm_field32_set(unit, &l3_if_entry, INSERT_L3_MPLS_LABEL1f, 1);
        soc_L3INTFm_field32_set(unit, &l3_if_entry, MPLS_TUNNEL_LABEL1f,
                                label_array[1].label);
        soc_L3INTFm_field32_set(unit, &l3_if_entry, INSERT_L3_MPLS_LABEL2f, 1);
        soc_L3INTFm_field32_set(unit, &l3_if_entry, MPLS_TUNNEL_LABEL2f,
                                label_array[0].label);
    } else {
        MEM_UNLOCK(unit, L3INTFm);
        return BCM_E_PARAM;
    }
    /* Same EXP-map is used for all labels that are pushed */
    soc_L3INTFm_field32_set(unit, &l3_if_entry, MPLS_TUNNEL_LABEL_QOS_INDEXf,
                            label_array[0].qos_map_id);
    if (label_array[0].flags & BCM_MPLS_EGRESS_LABEL_TTL_SET) {
        soc_L3INTFm_field32_set(unit, &l3_if_entry, TUNNEL_TTLf, 
                                label_array[0].ttl);
    } else {
        soc_L3INTFm_field32_set(unit, &l3_if_entry, TUNNEL_TTLf, 0);
    }
    rv = WRITE_L3INTFm(unit, MEM_BLOCK_ALL, intf, &l3_if_entry);
    MEM_UNLOCK(unit, L3INTFm);

    return rv;
}

int
bcm_er_mpls_tunnel_initiator_clear(int unit, bcm_if_t intf)
{
    l3intf_entry_t l3_if_entry;
    int type, rv;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_NEW);

    if (intf >= L3_INFO(unit)->l3_intf_table_size) {
        return BCM_E_PARAM;
    } else if (!BCM_L3_INTF_USED_GET(unit, intf)) {
        soc_cm_debug(DK_L3, "L3 interface not created\n");
        return BCM_E_NOT_FOUND;
    }

    /* L3 interface info */
    MEM_LOCK(unit, L3INTFm);
    rv = READ_L3INTFm(unit, MEM_BLOCK_ANY, intf, &l3_if_entry);
    if (rv < 0) {
        MEM_UNLOCK(unit, L3INTFm);
        return rv;
    }

    /* Check for IP tunnel conflicts */
    type = soc_L3INTFm_field32_get(unit, &l3_if_entry, TUNNEL_TYPEf);
    if (type != 8) {
        /* type != MPLS */
        MEM_UNLOCK(unit, L3INTFm);
        return BCM_E_NONE;
    }
    soc_L3INTFm_field32_set(unit, &l3_if_entry, TUNNEL_TYPEf, 0);
    soc_L3INTFm_field32_set(unit, &l3_if_entry, INSERT_L3_MPLS_LABEL1f, 0);
    soc_L3INTFm_field32_set(unit, &l3_if_entry, MPLS_TUNNEL_LABEL1f, 0);
    soc_L3INTFm_field32_set(unit, &l3_if_entry, INSERT_L3_MPLS_LABEL2f, 0);
    soc_L3INTFm_field32_set(unit, &l3_if_entry, MPLS_TUNNEL_LABEL2f, 0);
    soc_L3INTFm_field32_set(unit, &l3_if_entry, MPLS_TUNNEL_LABEL_QOS_INDEXf, 0);
    soc_L3INTFm_field32_set(unit, &l3_if_entry, TUNNEL_TTLf, 0);

    rv = WRITE_L3INTFm(unit, MEM_BLOCK_ALL, intf, &l3_if_entry);
    MEM_UNLOCK(unit, L3INTFm);

    return rv;
}

int
bcm_er_mpls_tunnel_initiator_get(int unit, bcm_if_t intf, int label_max,
                                 bcm_mpls_egress_label_t *label_array,
                                 int *label_count)
{
    l3intf_entry_t l3_if_entry;
    int i, type;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_NEW);

    if (intf >= L3_INFO(unit)->l3_intf_table_size) {
        return BCM_E_PARAM;
    } else if (!BCM_L3_INTF_USED_GET(unit, intf)) {
        soc_cm_debug(DK_L3, "L3 interface not created\n");
        return BCM_E_NOT_FOUND;
    }

    /* L3 interface info */
    BCM_IF_ERROR_RETURN
        (READ_L3INTFm(unit, MEM_BLOCK_ANY, intf, &l3_if_entry));

    /* Check for IP tunnel conflicts */
    type = soc_L3INTFm_field32_get(unit, &l3_if_entry, TUNNEL_TYPEf);
    if (type != 8) {
        /* type != MPLS */
        return BCM_E_NOT_FOUND;
    }

    sal_memset(label_array, 0, sizeof(bcm_mpls_egress_label_t) * label_max);
    if (soc_L3INTFm_field32_get(unit, &l3_if_entry, INSERT_L3_MPLS_LABEL2f)) {
        *label_count = 2;
    } else {
        *label_count = 1;
    }
    for (i = 0; i < *label_count; i++) {
        if (i == 0) {
            label_array[i].label = 
                soc_L3INTFm_field32_get(unit, &l3_if_entry, MPLS_TUNNEL_LABEL1f);
        } else {
            label_array[i].label = 
                soc_L3INTFm_field32_get(unit, &l3_if_entry, MPLS_TUNNEL_LABEL2f);
        }
        label_array[i].flags |= BCM_MPLS_EGRESS_LABEL_EXP_REMARK;
        label_array[i].qos_map_id = 
            soc_L3INTFm_field32_get(unit, &l3_if_entry, MPLS_TUNNEL_LABEL_QOS_INDEXf);
        if (soc_L3INTFm_field32_get(unit, &l3_if_entry, TUNNEL_TTLf)) {
            label_array[i].flags |= BCM_MPLS_EGRESS_LABEL_TTL_SET;
            label_array[i].ttl = 
                soc_L3INTFm_field32_get(unit, &l3_if_entry, TUNNEL_TTLf);
        } else {
            label_array[i].flags |= BCM_MPLS_EGRESS_LABEL_TTL_DECREMENT;
        }
    }
    return BCM_E_NONE;
}

int
bcm_er_mpls_tunnel_initiator_clear_all(int unit)
{
    l3intf_entry_t l3_if_entry;
    int type, index_max, i;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_NEW);

    index_max = soc_mem_index_max(unit, L3INTFm);

    if (!L3_INFO(unit)->l3_initialized || L3_INFO(unit)->l3intf_count == 0) {
        return BCM_E_NONE;
    }

    for (i = 0; i <= index_max; i++) {
        if (BCM_L3_INTF_USED_GET(unit, i)) {
            BCM_IF_ERROR_RETURN
                (READ_L3INTFm(unit, MEM_BLOCK_ANY, i, &l3_if_entry));
            type = soc_L3INTFm_field32_get(unit, &l3_if_entry, TUNNEL_TYPEf);
            if (type != 8) {
                /* type != MPLS */
                continue;
            }
            soc_L3INTFm_field32_set(unit, &l3_if_entry, TUNNEL_TYPEf, 0);
            soc_L3INTFm_field32_set(unit, &l3_if_entry, INSERT_L3_MPLS_LABEL1f, 0);
            soc_L3INTFm_field32_set(unit, &l3_if_entry, MPLS_TUNNEL_LABEL1f, 0);
            soc_L3INTFm_field32_set(unit, &l3_if_entry, INSERT_L3_MPLS_LABEL2f, 0);
            soc_L3INTFm_field32_set(unit, &l3_if_entry, MPLS_TUNNEL_LABEL2f, 0);
            soc_L3INTFm_field32_set(unit, &l3_if_entry,
                                    MPLS_TUNNEL_LABEL_QOS_INDEXf, 0);
            soc_L3INTFm_field32_set(unit, &l3_if_entry, TUNNEL_TTLf, 0);

            BCM_IF_ERROR_RETURN
                (WRITE_L3INTFm(unit, MEM_BLOCK_ALL, i, &l3_if_entry));
        }
    }

    return BCM_E_NONE;
}

/* Convert key part of application format to HW entry. */
STATIC int
_bcm_er_mpls_entry_set_key(int unit, bcm_mpls_tunnel_switch_t *info,
                           defip_alg_entry_t *lpm_entry)
{
    bcm_module_t modid, local_port;
    bcm_trunk_t trunk_id;
    int gport_id, rv;

    sal_memset(lpm_entry, 0, sizeof(defip_alg_entry_t));
    soc_L3_DEFIP_ALGm_field32_set(unit, lpm_entry,
                                  FFf, SOC_ER_DEFIP_FF_FIELD);

    /*
     * If MPLS_PORT_CHECKf=1 in the ingress port's PORT_TAB, then
     * the SRCMODf and SRCPORTf fields must be filled in with
     * the desired ingress mod/port or trunk.
     * If MPLS_PORT_CHECKf=0, then the SRCMODf and SRCPORTf fields
     * must be filled in with all ones (0x3f).
     *
     * The MPLS_PORT_CHECKf is controlled with the switch
     * control, bcmSwitchMplsIngressPortCheck.
     */

    if (info->port != BCM_GPORT_INVALID) {
        rv = _bcm_esw_gport_resolve(unit, info->port, &modid, &local_port, 
                                &trunk_id, &gport_id);
        BCM_IF_ERROR_RETURN(rv);
    }
	
    if (info->port == BCM_GPORT_INVALID) {
        soc_L3_DEFIP_ALGm_field32_set(unit, lpm_entry, SRCMODf, 0x3f);
        soc_L3_DEFIP_ALGm_field32_set(unit, lpm_entry, SRCPORTf, 0x3f);
    } else if (BCM_GPORT_IS_TRUNK(info->port)) {
        soc_L3_DEFIP_ALGm_field32_set(unit, lpm_entry, SRCMODf,
                                      BCM_TRUNK_TO_MODIDf(unit, trunk_id));
        soc_L3_DEFIP_ALGm_field32_set(unit, lpm_entry, SRCPORTf,
                                      BCM_TRUNK_TO_TGIDf(unit, trunk_id));
    } else {
        soc_L3_DEFIP_ALGm_field32_set(unit, lpm_entry, SRCMODf, modid);
        soc_L3_DEFIP_ALGm_field32_set(unit, lpm_entry, SRCPORTf, local_port);
    }
    MPLS_MEMPARAM32_CHECK(unit, L3_DEFIP_ALGm, MPLS_LABEL1f, info->label);
    soc_L3_DEFIP_ALGm_field32_set(unit, lpm_entry, MPLS_LABEL1f, info->label);
    if (info->flags & BCM_MPLS_SWITCH_LOOKUP_INNER_LABEL) {
        MPLS_MEMPARAM32_CHECK(unit, L3_DEFIP_ALGm, MPLS_LABEL2f,
                              info->inner_label);
        soc_L3_DEFIP_ALGm_field32_set(unit, lpm_entry,
                                      MPLS_LABEL2f, info->inner_label);
        soc_L3_DEFIP_ALGm_field32_set(unit, lpm_entry, KEY_TYPEf,
                                      SOC_ER_DEFIP_KEY_TYPE_MPLS_2L);
    } else {
        soc_L3_DEFIP_ALGm_field32_set(unit, lpm_entry, KEY_TYPEf,
                                      SOC_ER_DEFIP_KEY_TYPE_MPLS_1L);
    }
    soc_L3_DEFIP_ALGm_field32_set(unit, lpm_entry, VALIDf, 1);
    return BCM_E_NONE;
}

/* Convert key part of HW entry to application format. */
STATIC int
_bcm_er_mpls_entry_get_key(int unit, defip_alg_entry_t *lpm_entry,
                           bcm_mpls_tunnel_switch_t *info)
{
    bcm_port_t port_in, port_out;
    bcm_module_t mod_in, mod_out;
    bcm_trunk_t trunk_id;
    int key_type;

    port_in = soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, SRCPORTf);
    mod_in = soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, SRCMODf);

    if (port_in & BCM_TGID_TRUNK_INDICATOR(unit)) {
        trunk_id = BCM_MODIDf_TGIDf_TO_TRUNK(unit, mod_in, port_in);
        BCM_GPORT_TRUNK_SET(info->port, trunk_id);
    } else if  ((port_in == 0x3f) && (mod_in == 0x3f)) {
        /* Global label, mod/port not part of lookup key */
        info->port = BCM_GPORT_INVALID;
    } else {
            BCM_IF_ERROR_RETURN
                (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                        mod_in, port_in, &mod_out, &port_out));
        BCM_GPORT_MODPORT_SET(info->port, mod_out, port_out);
    }
    info->label = soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, MPLS_LABEL1f);
    key_type = soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, KEY_TYPEf);
    if (key_type == SOC_ER_DEFIP_KEY_TYPE_MPLS_2L) {
        info->flags |= BCM_MPLS_SWITCH_LOOKUP_INNER_LABEL;
        info->inner_label = 
            soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, MPLS_LABEL2f);
    }
    return BCM_E_NONE;
}

/* Convert data part of HW entry to application format. */
STATIC int
_bcm_er_mpls_entry_get_data(int unit, defip_alg_entry_t *lpm_entry,
                            bcm_mpls_tunnel_switch_t *info)
{
    int action, vrf, nh_index;

    action = soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, MPLS_ACTIONf);
    switch (action) {
        case BCM_MPLS_OPCODE_POP_L3_NEXTHOP:
            info->action = BCM_MPLS_SWITCH_ACTION_POP_DIRECT;
            nh_index = soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, 
                                                     NEXT_HOP_INDEXf);
            info->egress_if = nh_index + BCM_XGS3_EGRESS_IDX_MIN;
            break;
        case BCM_MPLS_OPCODE_POP_L3_SWITCH:
            info->action = BCM_MPLS_SWITCH_ACTION_POP;
            vrf = soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, VRF_IDf);
            _BCM_MPLS_VPN_SET(info->vpn, _BCM_MPLS_VPN_TYPE_L3, vrf);
            break;
        case BCM_MPLS_OPCODE_SWAP:
        case BCM_MPLS_OPCODE_SWAP_PUSH:
            info->action = BCM_MPLS_SWITCH_ACTION_SWAP;
            nh_index = soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, 
                                                     NEXT_HOP_INDEXf);
            info->egress_if = nh_index + BCM_XGS3_EGRESS_IDX_MIN;
            break;
        case BCM_MPLS_OPCODE_PHP:
            info->action = BCM_MPLS_SWITCH_ACTION_PHP;
            nh_index = soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, 
                                                     NEXT_HOP_INDEXf);
            info->egress_if = nh_index + BCM_XGS3_EGRESS_IDX_MIN;
            break;
        default:
            return BCM_E_INTERNAL;
    }
    if (soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, MPLS_USE_OUTER_TTLf)) {
        info->flags |= BCM_MPLS_SWITCH_OUTER_TTL;
    } else {
        info->flags |= BCM_MPLS_SWITCH_INNER_TTL;
    }
    if (soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, MPLS_USE_OUTER_EXPf)) {
        info->flags |= BCM_MPLS_SWITCH_OUTER_EXP;
    } else {
        info->flags |= BCM_MPLS_SWITCH_INNER_EXP;
    }
    if (!soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, MPLS_USE_PRIf)) {
        info->int_pri = 
            soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, MPLS_LABEL_PRIf);
        info->flags |= BCM_MPLS_SWITCH_INT_PRI_SET;
    } else {
        info->flags |= BCM_MPLS_SWITCH_INT_PRI_MAP;
    }
    /* Easyrider always uses exp_map to get color */
    info->flags |= BCM_MPLS_SWITCH_COLOR_MAP;
    info->exp_map = soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, MPLS_IINTFf);
    info->exp_map -= _BCM_ER_MPLS_L3_INTF_BASE;
    info->exp_map |= _BCM_ER_MPLS_EXP_MAP_TABLE_TYPE_INGRESS;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_tunnel_switch_add
 * Purpose:
 *      Add an MPLS label entry.
 * Parameters:
 *      unit   - Device Number
 *      info   - Label (switch) information
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_mpls_tunnel_switch_add(int unit, bcm_mpls_tunnel_switch_t *info)
{
    defip_alg_entry_t  lpm_entry, lpm_key;
    int                defip_idx, nh_index = -1, rv, vrf, table_num;
    bcm_l3_egress_t    nh_info;
    l3intf_entry_t     l3_if_entry;
    uint32   flags=0;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_NEW);

    table_num = info->exp_map & _BCM_ER_MPLS_EXP_MAP_TABLE_NUM_MASK;
    if ((table_num < 0) ||
        (table_num >= MPLS_INFO(unit)->exp_map_max)) {
        return BCM_E_PARAM;
    }

    /* See if the entry already exists */
    rv = _bcm_er_mpls_entry_set_key(unit, info, &lpm_key);
    BCM_IF_ERROR_RETURN(rv); 

    rv = soc_er_defip_lookup(unit, &defip_idx, &lpm_key, &lpm_entry);
    if(rv == SOC_E_NONE) {
        return BCM_E_EXISTS;
    }

    /* Doesn't exist, insert a new entry */
    rv = _bcm_er_mpls_entry_set_key(unit, info, &lpm_entry);
    BCM_IF_ERROR_RETURN(rv); 

    switch(info->action) {
        case BCM_MPLS_SWITCH_ACTION_POP:
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, MPLS_ACTIONf,
                                          BCM_MPLS_OPCODE_POP_L3_SWITCH);
            vrf = _BCM_MPLS_VPN_ID_GET(info->vpn);
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, VRF_IDf, vrf);
            break;
        case BCM_MPLS_SWITCH_ACTION_SWAP:
            if (!BCM_XGS3_L3_EGRESS_IDX_VALID(unit, info->egress_if)) {
                return BCM_E_PARAM;
            }
            /*
             * Get egress next-hop index from egress object and
             * increment egress object reference count.
             */
            rv = bcm_xgs3_get_nh_from_egress_object(unit, info->egress_if,
                                                    &flags, 1, &nh_index);
            BCM_IF_ERROR_RETURN(rv);

            /* Get egress next-hop info from the egress object */
            rv = bcm_xgs3_nh_get(unit, nh_index, &nh_info);
            BCM_IF_ERROR_RETURN(rv);

            rv = READ_L3INTFm(unit, MEM_BLOCK_ANY, nh_info.intf,
                              &l3_if_entry);
            if (rv < 0) {
                (void) bcm_xgs3_nh_del(unit, 0, nh_index);
                return rv;
            }
            if (soc_L3INTFm_field32_get(unit, &l3_if_entry, 
                                        TUNNEL_TYPEf) == 0x8) {
                /* Next-hop entry is pointing to an MPLS TUNNEL */
                soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, MPLS_ACTIONf,
                                              BCM_MPLS_OPCODE_SWAP_NEW_DOMAIN);
            } else {
                /* Next-hop entry is not pointing to an MPLS TUNNEL */
                soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, MPLS_ACTIONf,
                                              BCM_MPLS_OPCODE_SWAP);
            }
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry,
                                          NEXT_HOP_INDEXf, nh_index);
            MPLS_MEMPARAM32_CHECK(unit, L3_DEFIP_ALGm, MPLS_NEW_LABELf,
                                  info->egress_label.label);
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, MPLS_NEW_LABELf,
                                          info->egress_label.label);
            break;
        case BCM_MPLS_SWITCH_ACTION_PHP:
        case BCM_MPLS_SWITCH_ACTION_POP_DIRECT:
            if (!BCM_XGS3_L3_EGRESS_IDX_VALID(unit, info->egress_if)) {
                return BCM_E_PARAM;
            }
            /*
             * Get egress next-hop index from egress object and
             * increment egress object reference count.
             */
            rv = bcm_xgs3_get_nh_from_egress_object(unit, info->egress_if,
                                                    &flags, 1, &nh_index);
            BCM_IF_ERROR_RETURN(rv);

            /* Get egress next-hop info from the egress object */
            rv = bcm_xgs3_nh_get(unit, nh_index, &nh_info);
            BCM_IF_ERROR_RETURN(rv);

            rv = READ_L3INTFm(unit, MEM_BLOCK_ANY, nh_info.intf,
                              &l3_if_entry);
            if (rv < 0) {
                (void) bcm_xgs3_nh_del(unit, 0, nh_index);
                return rv;
            }
            if (soc_L3INTFm_field32_get(unit, &l3_if_entry, 
                                        TUNNEL_TYPEf) == 0x8) {
                /* For PHP, HW can only push one label */
                if (soc_L3INTFm_field32_get(unit, &l3_if_entry, 
                                            INSERT_L3_MPLS_LABEL2f)) {
                    (void) bcm_xgs3_nh_del(unit, 0, nh_index);
                    return BCM_E_PARAM;
                }
            }
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry,
                                          NEXT_HOP_INDEXf, nh_index);
            if (info->action == BCM_MPLS_SWITCH_ACTION_PHP) {
                soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, MPLS_ACTIONf,
                                              BCM_MPLS_OPCODE_PHP);
            } else {
                soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, MPLS_ACTIONf,
                                              BCM_MPLS_OPCODE_POP_L3_NEXTHOP);
            }
            break;
        default:
            return BCM_E_PARAM;
    }
    if (info->flags & BCM_MPLS_SWITCH_INNER_TTL) {
        if (info->action == BCM_MPLS_SWITCH_ACTION_SWAP) {
            return BCM_E_PARAM;
        }
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, 
                                      MPLS_USE_OUTER_TTLf, 0x0);
    } else {
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, 
                                      MPLS_USE_OUTER_TTLf, 0x1);
    }
    if (info->flags & BCM_MPLS_SWITCH_INNER_EXP) {
        if (info->action == BCM_MPLS_SWITCH_ACTION_SWAP) {
            return BCM_E_PARAM;
        }
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, 
                                      MPLS_USE_OUTER_EXPf, 0x0);
    } else {
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, 
                                      MPLS_USE_OUTER_EXPf, 0x1);
    }
    if (info->flags & BCM_MPLS_SWITCH_INT_PRI_SET) {
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, 
                                      MPLS_LABEL_PRIf, info->int_pri);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, 
                                      MPLS_USE_PRIf, 0x1);
    } else {
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, 
                                      MPLS_USE_PRIf, 0x0);
    }

    /*
     * Set the incoming interface to one of the 8 reserved L3_INTF values.
     * Each one corresponds directly to one of the 8 EXP maps.
     */
    soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, MPLS_IINTFf,
                                  _BCM_ER_MPLS_L3_INTF_BASE + table_num);

    /* Write lpm_entry to L3_DEFIP_ALGm */
    rv = soc_er_defip_insert(unit, &lpm_entry);
    if (rv == SOC_E_NOT_FOUND) {
        rv = SOC_E_NONE;
    }
    if ((rv < 0) && (nh_index != -1)) {
        (void) bcm_xgs3_nh_del(unit, 0, nh_index);
    }
    return rv;
}

STATIC
int _bcm_er_mpls_entry_delete(int unit, defip_alg_entry_t *lpm_entry)
{
    int rv, nh_index, action;

    action = soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, MPLS_ACTIONf);
    nh_index = soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry,
                                             NEXT_HOP_INDEXf);

    /* Delete the entry from HW */
    rv = soc_er_defip_delete(unit, lpm_entry);
    BCM_IF_ERROR_RETURN(rv);


    if ((action == BCM_MPLS_OPCODE_SWAP) || 
        (action == BCM_MPLS_OPCODE_SWAP_NEW_DOMAIN) || 
        (action == BCM_MPLS_OPCODE_PHP)) {
        rv = bcm_xgs3_nh_del(unit, 0, nh_index);
    }
    return rv;
}

/*
 * Function:
 *      bcm_mpls_tunnel_switch_delete
 * Purpose:
 *      Delete an MPLS label entry.
 * Parameters:
 *      unit  - Device Number
 *      info  - Label (switch) information
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_mpls_tunnel_switch_delete(int unit, bcm_mpls_tunnel_switch_t *info)
{
    int rv, index;
    defip_alg_entry_t lpm_entry;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_NEW);

    rv = _bcm_er_mpls_entry_set_key(unit, info, &lpm_entry);
    BCM_IF_ERROR_RETURN(rv);

    rv = soc_er_defip_lookup(unit, &index, &lpm_entry, &lpm_entry);
    BCM_IF_ERROR_RETURN(rv);

    return _bcm_er_mpls_entry_delete(unit, &lpm_entry);
}

/*
 * Test callback function for MPLS switch traversal
 */

STATIC int
_bcm_er_mpls_switch_del_all_testcb(int unit, defip_alg_entry_t *lpm_entry,
                           int ent_idx, int *test, void *cookie)
{
    int key_type, action;

    key_type =
        soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, KEY_TYPEf);

    if (!soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, VALIDf) ||
        soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, FFf) != 0xff ||
        ( (key_type != SOC_ER_DEFIP_KEY_TYPE_MPLS_1L) &&
          (key_type != SOC_ER_DEFIP_KEY_TYPE_MPLS_2L) ) ) {
        *test = FALSE;
    } else {
        action = soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, MPLS_ACTIONf);
        if ((action == BCM_MPLS_OPCODE_SWAP) || 
            (action == BCM_MPLS_OPCODE_SWAP_NEW_DOMAIN) || 
            (action == BCM_MPLS_OPCODE_PHP) || 
            (action == BCM_MPLS_OPCODE_POP_L3_SWITCH)) {
            *test = TRUE;
        }
    }
    return BCM_E_NONE;
}

/*
 * Operational callback function for MPLS switch delete all
 */
STATIC int
_bcm_er_mpls_switch_del_all_opcb(int unit, defip_alg_entry_t *lpm_entry,
                          int ent_idx, int *test, void *cookie)
{
    return _bcm_er_mpls_entry_delete(unit, lpm_entry);
}

/*
 * Function:
 *      bcm_mpls_tunnel_switch_delete_all
 * Purpose:
 *      Delete all MPLS label entries.
 * Parameters:
 *      unit   - Device Number
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_mpls_tunnel_switch_delete_all(int unit)
{
    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_NEW);

    MPLS_SWITCH_LOCK(unit);
    soc_er_defip_traverse(unit, _bcm_er_mpls_switch_del_all_testcb,
                          _bcm_er_mpls_switch_del_all_opcb, NULL, NULL);
    MPLS_SWITCH_UNLOCK(unit);

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_tunnel_switch_get
 * Purpose:
 *      Get an MPLS label entry.
 * Parameters:
 *      unit  - Device Number
 *      info  - Label (switch) information
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_mpls_tunnel_switch_get(int unit, bcm_mpls_tunnel_switch_t *info)
{
    int rv, index;
    defip_alg_entry_t lpm_entry;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_NEW);

    rv = _bcm_er_mpls_entry_set_key(unit, info, &lpm_entry);
    BCM_IF_ERROR_RETURN(rv);

    rv = soc_er_defip_lookup(unit, &index, &lpm_entry, &lpm_entry);
    BCM_IF_ERROR_RETURN(rv);

    return _bcm_er_mpls_entry_get_data(unit, &lpm_entry, info);
}

/*
 * Function:
 *      bcm_mpls_tunnel_switch_traverse
 * Purpose:
 *      Traverse all valid MPLS label entries an call the
 *      supplied callback routine.
 * Parameters:
 *      unit      - Device Number
 *      cb        - User callback function, called once per MPLS entry.
 *      user_data - cookie
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_mpls_tunnel_switch_traverse(int unit, 
                                   bcm_mpls_tunnel_switch_traverse_cb cb,
                                   void *user_data)
{
    int rv, action, key_type, index = -1;
    defip_alg_entry_t lpm_entry;
    bcm_mpls_tunnel_switch_t info;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_NEW);
    info.flags = 0;

    MPLS_SWITCH_LOCK(unit);
    while (1) {
        if ((rv = soc_er_defip_iterate(unit, &index, &lpm_entry)) < 0 ||
            index == -1) {   /* End of iteration */
            break;
        }

        key_type = soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, KEY_TYPEf);
        if (soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, FFf) != 0xff ||
             ((key_type != SOC_ER_DEFIP_KEY_TYPE_MPLS_1L) &&
              (key_type != SOC_ER_DEFIP_KEY_TYPE_MPLS_2L))) {
            continue;
        }
        action = soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, MPLS_ACTIONf);
        if ((action == BCM_MPLS_OPCODE_POP_DST_MOD_PORT) ||
            (action == BCM_MPLS_OPCODE_POP_L2_SWITCH)) {
            /* L2 entry */
            continue;
        }
        rv = _bcm_er_mpls_entry_get_key(unit, &lpm_entry, &info);
        BCM_IF_ERROR_RETURN(rv);

        rv = _bcm_er_mpls_entry_get_data(unit, &lpm_entry, &info);
        BCM_IF_ERROR_RETURN(rv);

        /* Call application's call-back routine */
        rv = cb(unit, &info, user_data);
#ifdef BCM_CB_ABORT_ON_ERR
        if (BCM_FAILURE(rv) && SOC_CB_ABORT_ON_ERR(unit)) {
            MPLS_SWITCH_UNLOCK(unit);
            return rv;
        }
#endif
    }
    MPLS_SWITCH_UNLOCK(unit);

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_port_block_set
 * Purpose:
 *      Stop L2 MPLS packet to go out on certain ports
 * Parameters:
 *      unit   - Device Number
 *      vpn    - the VFI index
 *      port   - Port bitmap
 *      enable - enable or not
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      Use pbmp 0 to clear the bitmap
 */

int
bcm_er_mpls_port_block_set (int unit, bcm_vpn_t vpn,
                            bcm_port_t port, int enable)
{
    _bcm_mpls_bookkeeping_t *mpls_info = MPLS_INFO(unit);
    vfi_bitmap_entry_t entry;
    bcm_pbmp_t pbmp;
    uint32 bval;
    int vfi = 0;

    MPLS_INIT(unit);
    if (mpls_info->api_style == _BCM_ER_MPLS_API_STYLE_OLD) {
        if (vpn > BCM_VPN_MAX) {
            return BCM_E_PARAM;
        }
        vfi = vpn;
    } else if (mpls_info->api_style == _BCM_ER_MPLS_API_STYLE_NEW) {
        vfi = _BCM_MPLS_VPN_ID_GET(vpn);
    } else {
        return BCM_E_NOT_FOUND;
    }

    SOC_IF_ERROR_RETURN
        (READ_VFI_BITMAPm(unit, MEM_BLOCK_ANY, vfi, &entry));

    soc_VFI_BITMAPm_field_get(unit, &entry, BIT_MASKf, &bval);
    SOC_PBMP_WORD_SET(pbmp, 0, bval);
    if (enable) {
        SOC_PBMP_PORT_ADD(pbmp, port);
    } else {
        SOC_PBMP_PORT_REMOVE(pbmp, port);
    }
    bval = SOC_PBMP_WORD_GET(pbmp, 0);
    soc_VFI_BITMAPm_field_set(unit, &entry, BIT_MASKf, &bval);

    SOC_IF_ERROR_RETURN
        (WRITE_VFI_BITMAPm(unit, MEM_BLOCK_ALL, vfi, &entry));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_port_block_get
 * Purpose:
 *      Stop L2 MPLS packet to go out on certain ports
 * Parameters:
 *      unit   - Device Number
 *      vpn    - the VFI index
 *      port   - Port bitmap
 *      enable - (OUT) enabled or not
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_er_mpls_port_block_get (int unit, bcm_vpn_t vpn,
                            bcm_port_t port, int *enable)
{
    _bcm_mpls_bookkeeping_t *mpls_info = MPLS_INFO(unit);
    vfi_bitmap_entry_t entry;
    bcm_pbmp_t pbmp;
    uint32 bval;
    int vfi = 0;

    MPLS_INIT(unit);
    if (mpls_info->api_style == _BCM_ER_MPLS_API_STYLE_OLD) {
        if (vpn > BCM_VPN_MAX) {
            return BCM_E_PARAM;
        }
        vfi = vpn;
    } else if (mpls_info->api_style == _BCM_ER_MPLS_API_STYLE_NEW) {
        vfi = _BCM_MPLS_VPN_ID_GET(vpn);
    } else {
        return BCM_E_NOT_FOUND;
    }

    SOC_IF_ERROR_RETURN
        (READ_VFI_BITMAPm(unit, MEM_BLOCK_ANY, vfi, &entry));

    soc_VFI_BITMAPm_field_get(unit, &entry, BIT_MASKf, &bval);
    SOC_PBMP_WORD_SET(pbmp, 0, bval);
    *enable = SOC_PBMP_MEMBER(pbmp, port);

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_mpls_exp_map_create
 * Purpose:
 *      Create an MPLS EXP map instance.
 * Parameters:
 *      unit        - (IN)  SOC unit #
 *      flags       - (IN)  MPLS flags
 *      exp_map_id  - (OUT) Allocated EXP map ID
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_mpls_exp_map_create(int unit, uint32 flags, int *exp_map_id)
{
    int i, table_num;

    MPLS_INIT(unit);

    /* Check for unsupported Flag */
    if (flags & (~(BCM_MPLS_WITH_ID |
                   BCM_MPLS_EXP_MAP_INGRESS |
                   BCM_MPLS_EXP_MAP_EGRESS |
                   BCM_MPLS_UPDATE ))) {
        return BCM_E_PARAM;
    }


    /* Input parameters check. */
    if (NULL == exp_map_id) {
        return (BCM_E_PARAM);
    }

    if (flags & BCM_MPLS_EXP_MAP_INGRESS) {
        if (flags & BCM_MPLS_WITH_ID) {
            table_num = (*exp_map_id & _BCM_ER_MPLS_EXP_MAP_TABLE_NUM_MASK);
            if (BCM_MPLS_ING_EXP_MAP_USED_GET(unit, table_num) && 
                !(flags & BCM_MPLS_UPDATE)) {
                return (BCM_E_EXISTS);
            }
            BCM_MPLS_ING_EXP_MAP_USED_SET(unit, table_num);
            return (BCM_E_NONE);
        }
        for (i = 0; i < MPLS_INFO(unit)->ing_exp_map_max; i++) {
            if (!BCM_MPLS_ING_EXP_MAP_USED_GET(unit, i)) {
                *exp_map_id = (i | _BCM_ER_MPLS_EXP_MAP_TABLE_TYPE_INGRESS);
                BCM_MPLS_ING_EXP_MAP_USED_SET(unit, i);
                return BCM_E_NONE;
            }
        }
    } else {
        if (flags & BCM_MPLS_WITH_ID) {
            table_num = (*exp_map_id & _BCM_ER_MPLS_EXP_MAP_TABLE_NUM_MASK);
            if (BCM_MPLS_EXP_MAP_USED_GET(unit, table_num) && 
                !(flags & BCM_MPLS_UPDATE)) {
                return (BCM_E_EXISTS);
            }
            BCM_MPLS_EXP_MAP_USED_SET(unit, table_num);
            return (BCM_E_NONE);
        }

        for (i = 0; i < MPLS_INFO(unit)->exp_map_max; i++) {
            if (!BCM_MPLS_EXP_MAP_USED_GET(unit, i)) {
                *exp_map_id = (i | _BCM_ER_MPLS_EXP_MAP_TABLE_TYPE_EGRESS);
                BCM_MPLS_EXP_MAP_USED_SET(unit, i);
                return BCM_E_NONE;
            }
        }
    }
    return BCM_E_FULL;
}

/*
 * Function:
 *      bcm_esw_mpls_exp_map_destroy
 * Purpose:
 *      Destroy an existing MPLS EXP map instance.
 * Parameters:
 *      unit       - (IN) SOC unit #
 *      exp_map_id - (IN) EXP map ID
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_mpls_exp_map_destroy(int unit, int exp_map_id)
{
    int table_num;
    MPLS_INIT(unit);

    if (exp_map_id < 0) {
        return BCM_E_PARAM;
    }
    table_num = exp_map_id & _BCM_ER_MPLS_EXP_MAP_TABLE_NUM_MASK;

    if ((exp_map_id & _BCM_ER_MPLS_EXP_MAP_TABLE_TYPE_MASK) ==
        _BCM_ER_MPLS_EXP_MAP_TABLE_TYPE_INGRESS) {

        if (table_num >= MPLS_INFO(unit)->ing_exp_map_max) {
            return BCM_E_PARAM;
        }
        if (!BCM_MPLS_ING_EXP_MAP_USED_GET(unit, table_num)) {
            return BCM_E_NOT_FOUND;
        }
        BCM_MPLS_ING_EXP_MAP_USED_CLR(unit, table_num);
    } else {
        if (table_num >= MPLS_INFO(unit)->exp_map_max) {
            return BCM_E_PARAM;
        }
        if (!BCM_MPLS_EXP_MAP_USED_GET(unit, table_num)) {
            return BCM_E_NOT_FOUND;
        }
        BCM_MPLS_EXP_MAP_USED_CLR(unit, table_num);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_mpls_exp_map_set
 * Purpose:
 *      Set the mapping of { internal priority, color }
 *      to a EXP value for MPLS headers
 *      in the specified EXP map instance.
 * Parameters:
 *      unit         - (IN) SOC unit #
 *      exp_map_id   - (IN) EXP map ID
 *      priority     - (IN) Internal priority
 *      color        - (IN) bcmColor*
 *      exp          - (IN) EXP value
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_mpls_exp_map_set(int unit, int exp_map_id,
                        bcm_mpls_exp_map_t *exp_map)
{
    int cng, index, table_num;
    egress_dscp_exp_entry_t entry; /* Buffer to read hw entry. */
    mpls_exp_entry_t ing_pri_entry;
    uint32 exp;
        
    MPLS_INIT(unit);

    table_num = exp_map_id & _BCM_ER_MPLS_EXP_MAP_TABLE_NUM_MASK;
    if ((exp_map == NULL) ||
        (exp_map_id < 0) ||
        (table_num >= MPLS_INFO(unit)->exp_map_max) ||
        (exp_map->priority < BCM_PRIO_MIN) ||
        (exp_map->priority > BCM_PRIO_MAX) ||
        (exp_map->exp > 7)) {
        return BCM_E_PARAM;
    }

    if ((exp_map_id & _BCM_ER_MPLS_EXP_MAP_TABLE_TYPE_MASK) ==
        _BCM_ER_MPLS_EXP_MAP_TABLE_TYPE_INGRESS) {

        if (!BCM_MPLS_ING_EXP_MAP_USED_GET(unit, table_num)) {
            return BCM_E_NOT_FOUND;
        }
 
        cng = _BCM_COLOR_ENCODING(unit, exp_map->color);
 
        /* Get the base index for this EXP map */
        index = (table_num * 8) + exp_map->exp; 
 
        /* Commit to HW */
        BCM_IF_ERROR_RETURN(READ_MPLS_EXPm(unit, MEM_BLOCK_ANY,
                                           index, &ing_pri_entry));
        soc_MPLS_EXPm_field32_set(unit, &ing_pri_entry, PRIf, exp_map->priority);
        soc_MPLS_EXPm_field32_set(unit, &ing_pri_entry, CNGf, cng);
        BCM_IF_ERROR_RETURN(WRITE_MPLS_EXPm(unit, MEM_BLOCK_ALL,
                                            index, &ing_pri_entry));
    } else {
        if (!BCM_MPLS_EXP_MAP_USED_GET(unit, table_num)) {
            return BCM_E_NOT_FOUND;
        }

        cng = _BCM_COLOR_ENCODING(unit, exp_map->color);

        /* Get the base index for this EXP map */
        index = (table_num * 32);

        /* Add the offset based on priority and color values */
        index += ((exp_map->priority << 2) | (cng & 3));

        /* Set MPLS exp entry info in hw.
         * Since this table is shared with L3 DSCP, need to read,
         * modify, and write to preserve the DSCP field value.
         */
        SOC_IF_ERROR_RETURN
            (READ_EGRESS_DSCP_EXPm(unit, MEM_BLOCK_ANY, index, &entry));

        exp = exp_map->exp;
        soc_EGRESS_DSCP_EXPm_field_set(unit, &entry, EGRESS_EXPf, &exp);
    
        BCM_IF_ERROR_RETURN (WRITE_EGRESS_DSCP_EXPm(unit, MEM_BLOCK_ALL, 
                                                    index, &entry));
    }
    return BCM_E_NONE;
}
    
/*
 * Function:
 *      bcm_esw_mpls_exp_map_get
 * Purpose:
 *      Get the mapping of { internal priority, color }
 *      to a EXP value for MPLS headers
 *      in the specified EXP map instance.
 * Parameters:
 *      unit         - (IN)  SOC unit #
 *      exp_map_id   - (IN)  EXP map ID
 *      priority     - (IN)  Internal priority
 *      color        - (IN)  bcmColor*
 *      exp          - (OUT) EXP value
 * Returns:
 *      BCM_E_XXX
 */     
int     
bcm_er_mpls_exp_map_get(int unit, int exp_map_id,
                        bcm_mpls_exp_map_t *exp_map)
{
    int cng, index, table_num;
    egress_dscp_exp_entry_t entry; /* Buffer to read hw entry. */
    mpls_exp_entry_t ing_pri_entry;
    uint32 exp;
    
    MPLS_INIT(unit);

    table_num = exp_map_id & _BCM_ER_MPLS_EXP_MAP_TABLE_NUM_MASK;
    if ((exp_map == NULL) ||
        (exp_map_id < 0) ||
        (table_num >= MPLS_INFO(unit)->exp_map_max) ||
        (exp_map->priority < BCM_PRIO_MIN) ||
        (exp_map->priority > BCM_PRIO_MAX)) {
        return BCM_E_PARAM;
    }

    if ((exp_map_id & _BCM_ER_MPLS_EXP_MAP_TABLE_TYPE_MASK) ==
        _BCM_ER_MPLS_EXP_MAP_TABLE_TYPE_INGRESS) {

        if (!BCM_MPLS_ING_EXP_MAP_USED_GET(unit, table_num)) {
            return BCM_E_NOT_FOUND;
        }
 
        /* Get the base index for this EXP map */
        index = (table_num * 8) + exp_map->exp;
 
        /* Commit to HW */
        BCM_IF_ERROR_RETURN(READ_MPLS_EXPm(unit, MEM_BLOCK_ANY,
                                           index, &ing_pri_entry));
        exp_map->priority = soc_mem_field32_get(unit, MPLS_EXPm,
                                            &ing_pri_entry, PRIf);
        cng = soc_mem_field32_get(unit, MPLS_EXPm,
                                  &ing_pri_entry, CNGf);
        exp_map->color = _BCM_COLOR_DECODING(unit, cng);

    } else {
        if (!BCM_MPLS_EXP_MAP_USED_GET(unit, table_num)) {
            return BCM_E_NOT_FOUND; 
        }

        cng = _BCM_COLOR_ENCODING(unit, exp_map->color);
    
        /* Get the base index for this EXP map in EGR_DSCP_TABLEm */
        index = (table_num * 32);

        /* Add the offset based on priority and color values */
        index += ((exp_map->priority << 2) | (cng & 3));
    
        SOC_IF_ERROR_RETURN
            (READ_EGRESS_DSCP_EXPm(unit, MEM_BLOCK_ANY, index, &entry));

        soc_EGRESS_DSCP_EXPm_field_get(unit, &entry, EGRESS_EXPf, &exp);
        exp_map->exp = exp;
    }
    return BCM_E_NONE;
}

/****************************************************************
 * Below are the legacy APIs supported only on BCM5660x devices *
 ****************************************************************/
/*
 * Function:
 *      bcm_mpls_info
 * Purpose:
 *      Get the HW MPLS table information such as table size etc
 * Parameters:
 *      unit - Device Number
 *      info - (OUT) table size and used info
 * Returns:
 *      BCM_E_XXXX
 */     
 
int
bcm_er_mpls_info(int unit, bcm_mpls_info_t *info)
{
    MPLS_INIT(unit);

    info->vpn_max          = MPLS_INFO(unit)->vpn_max;
    info->vpn_used         = MPLS_INFO(unit)->vpn_count;
    info->label_table_max  = MPLS_INFO(unit)->mpls_switch_max;
    info->label_table_used = MPLS_INFO(unit)->mpls_switch_count;
    info->circuit_max      = MPLS_INFO(unit)->vc_table_size;
    info->circuit_used     = MPLS_INFO(unit)->vc_table_used;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_vpn_create
 * Purpose:
 *      Create a VPN instance (no hardware write)
 * Parameters:
 *      unit   - Device Number
 *      vpn - VPN instance
 *      flags  - VPN property (BCM_MPLS_L2_VPN or BCM_MPLS_L3_VPN)
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      No hardware action
 */

int
bcm_er_mpls_vpn_create (int unit, bcm_vpn_t vpn, uint32 flags)
{
    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_OLD);

    if (vpn > BCM_VPN_MAX) {
        return BCM_E_PARAM;
    }

    if (BCM_MPLS_VPN_USED_GET(unit, vpn)) {
        return BCM_E_EXISTS;
    }

    BCM_MPLS_VPN_USED_SET(unit, vpn);
    if (flags & BCM_MPLS_L2_VPN) {
        BCM_L2_VPN_USED_SET(unit, vpn);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_vpn_destroy
 * Purpose:
 *      Delete a VPN instance
 * Parameters:
 *      unit - Device Number
 *      vpn - VPN instance ID
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      This automatically unbinds all interfaces to this VPN
 */

int
bcm_er_mpls_vpn_destroy (int unit, bcm_vpn_t vpn)
{
    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_OLD);

    if (vpn > BCM_VPN_MAX) {
        return BCM_E_PARAM;
    }

    if (!BCM_MPLS_VPN_USED_GET(unit, vpn)) {
        return BCM_E_NOT_FOUND;
    }

    BCM_L2_VPN_USED_CLR(unit, vpn);

    BCM_MPLS_VPN_USED_CLR(unit, vpn);

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_vpn_add (bind)
 * Purpose:
 *      Bind interface/port/VLAN to VPN instance
 * Parameters:
 *      unit - Device Number
 *      vpn - VPN instance ID
 *      vpn_info - VPN association information : interface/port/VLAN
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      This port/VID binding of customer ports to VPN.
 *      For L3 VPNs, the port info is not used (only vlan).
 *      For L2 VPNs, both the port and vlan info is used.
 *      Instance is normally done as a result of admin configuration.
 */

int
bcm_er_mpls_vpn_add (int unit, bcm_vpn_t vpn, bcm_mpls_vpn_t *vpn_info)
{
    vrf_vfi_intf_entry_t    vrfe;
    bcm_l2_cache_addr_t     l2ue;
    int                     index;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_OLD);

    /* Validate input parameters */
    if (vpn > BCM_VPN_MAX) {
        return BCM_E_PARAM;
    }  
    if (!SOC_PORT_VALID(unit, vpn_info->port)) {
        return BCM_E_PORT;
    }

    if (!BCM_MPLS_VPN_USED_GET(unit, vpn)) {
        return BCM_E_NOT_FOUND;
    }
	
    if (vpn_info->l3_intf  >= L3_INFO(unit)->l3_intf_table_size) {
        return BCM_E_PARAM;
    } else if (!BCM_L3_INTF_USED_GET(unit, vpn_info->l3_intf )) {
        return BCM_E_NOT_FOUND;
    }

    if (!BCM_VLAN_VALID(vpn_info->vlan)) {
        return  BCM_E_PARAM;
    }

    /* For L2 VPNs, install L2_USER_ENTRYm entry using the src port,
     * vlan information as the key (ignore MAC). Set the entry
     * to discard matching packets. Later, when a VC is created,
     * the dest port information will be used to update this entry.
     */
    if (BCM_L2_VPN_USED_GET(unit, vpn)) {

        
        if ((vpn_info->flags & BCM_MPLS_SRC_TRUNK) != 0) {
            return BCM_E_PARAM;
        }

        bcm_l2_cache_addr_t_init(&l2ue);
        l2ue.flags = BCM_L2_CACHE_DISCARD;
        l2ue.vlan_mask = BCM_L2_VID_MASK_ALL;
        l2ue.src_port_mask = BCM_L2_SRCPORT_MASK_ALL;
        l2ue.src_port = vpn_info->port;
        l2ue.vlan = vpn_info->vlan;
        BCM_IF_ERROR_RETURN(bcm_esw_l2_cache_set(unit, -1, &l2ue, &index));
    }

    /* Build VRF_VFI_INTFm entry. In the port table, the IGNORE_PORT_ID
     * is set, therefore the key to this table is the vlan id.
     * Set SRC_MODID,TGID to don't care, 0xfff.
     */
    sal_memset(&vrfe, 0, sizeof(vrf_vfi_intf_entry_t));

    soc_VRF_VFI_INTFm_field32_set(unit, &vrfe, SRC_MODIDf, 0x3f);
    soc_VRF_VFI_INTFm_field32_set(unit, &vrfe, TGIDf, 0x3f);
    soc_VRF_VFI_INTFm_field32_set(unit, &vrfe, VLANf, vpn_info->vlan);
    soc_VRF_VFI_INTFm_field32_set(unit, &vrfe, VRF_VFIf, vpn_info->vpn);
    soc_VRF_VFI_INTFm_field32_set(unit, &vrfe, L3_INTFf, vpn_info->l3_intf);
    if (SOC_MEM_FIELD_VALID(unit, VRF_VFI_INTFm, MPLS_ENABLEf)) {
        soc_VRF_VFI_INTFm_field32_set(unit, &vrfe, MPLS_ENABLEf, 1);
    }
    if (BCM_L2_VPN_USED_GET(unit, vpn)) {
        soc_VRF_VFI_INTFm_field32_set(unit, &vrfe, VFIf, 1);
    }

    return (soc_mem_insert(unit, VRF_VFI_INTFm, MEM_BLOCK_ALL, &vrfe));
}

/*
 * Function:
 *      bcm_mpls_vpn_delete (unbind)
 * Purpose:
 *      Unbind interface/port/VLAN to VPN instance
 * Parameters:
 *      unit     - Device Number
 *      vpn      - VPN instance ID
 *      vpn_info - interface/port/VLAN
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_er_mpls_vpn_delete (int unit, bcm_vpn_t vpn, bcm_mpls_vpn_t *vpn_info)
{
    vrf_vfi_intf_entry_t   vrf_key, vrfe;
    int                    l2_idx_max;   /* Max index for L2_USER_ENTRY table.*/
    bcm_l2_cache_addr_t    l2ue;         /* L2 cache entry.                   */
    int                    found;        /* L2 entry found flag.              */
    int index, rv;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_OLD);

    if (vpn > BCM_VPN_MAX) {
        return BCM_E_PARAM;
    }

    if (!BCM_MPLS_VPN_USED_GET(unit, vpn)) {
        return BCM_E_NOT_FOUND;
    }

    /* See if the key exists */
    sal_memset(&vrf_key, 0, sizeof(vrf_vfi_intf_entry_t));
    /* Set SRC_MODID,TGID to don't care, 0xfff */
    soc_VRF_VFI_INTFm_field32_set(unit, &vrf_key, SRC_MODIDf, 0x3f);
    soc_VRF_VFI_INTFm_field32_set(unit, &vrf_key, TGIDf, 0x3f);
    soc_VRF_VFI_INTFm_field32_set(unit, &vrf_key, VLANf, vpn_info->vlan);

    rv = soc_mem_er_search(unit, VRF_VFI_INTFm, MEM_BLOCK_ANY,
                           &index, &vrf_key, &vrfe, 0);
    if (rv < 0) {
        return rv;
    }

    if (BCM_L2_VPN_USED_GET(unit, vpn)) {
        found = FALSE;

        soc_mem_delete(unit, VRF_VFI_INTFm, MEM_BLOCK_ALL, &vrf_key);

        BCM_IF_ERROR_RETURN(bcm_esw_l2_cache_size_get(unit, &l2_idx_max)); 
        /* For L2 VPNs, also delete any L2_USER_TABLE entries */
        for (index = 0; index < l2_idx_max; index++) {
            rv = bcm_esw_l2_cache_get(unit, index, &l2ue);
            if (BCM_FAILURE(rv)) {
                if (rv == BCM_E_NOT_FOUND) {
                    continue;
                } else {
                    return rv;
                }
            }

            /* Check for matching VLAN_ID and MASK */
            if ((vpn_info->vlan == l2ue.vlan) &&
                (BCM_MAC_IS_ZERO(l2ue.mac_mask)) &&
                (BCM_L2_SRCPORT_MASK_ALL == l2ue.src_port_mask) &&
                (BCM_L2_VID_MASK_ALL == l2ue.vlan_mask)) {
                rv = bcm_esw_l2_cache_delete(unit, index);
                found = TRUE;
            }
        }
        if (!found) {
            return (BCM_E_NOT_FOUND);
        }
    } else {
        /* For L3 VPNs, keep the VRF_VFI_INTF entry in place,
         * but instead of using VPN id, use the VRF value of zero.
         */
        soc_VRF_VFI_INTFm_field32_set(unit, &vrfe, VRF_VFIf, 0);
        rv = soc_mem_insert(unit, VRF_VFI_INTFm, MEM_BLOCK_ALL, &vrfe);
    }
    return rv;
}

/*
 * Function:
 *      bcm_mpls_vpn_delete_all
 * Purpose:
 *      Unbind all interface/port/VLAN to the VPN instance
 * Parameters:
 *      unit - Device Number
 *      vpn - VPN instance ID
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_er_mpls_vpn_delete_all (int unit, bcm_vpn_t vpn)
{
    vrf_vfi_intf_entry_t   vrfe;        /* vrf_vfi entry info.      */
    bcm_mpls_vpn_t         vpn_info;    /* Vpn info structure.      */
    int                    index;       /* VRF_VRF iterator.        */ 
    int                    rv;          /* Operation return status. */

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_OLD);

    if (vpn > BCM_VPN_MAX) {
        return BCM_E_PARAM;
    }

    if (!BCM_MPLS_VPN_USED_GET(unit, vpn)) {
        return BCM_E_NOT_FOUND;
    }

    for (index = soc_mem_index_min(unit, VRF_VFI_INTFm);
         index <= soc_mem_index_max(unit, VRF_VFI_INTFm); index++) {

         BCM_IF_ERROR_RETURN(READ_VRF_VFI_INTFm(unit, MEM_BLOCK_ANY, index, &vrfe));
         if (vpn != soc_VRF_VFI_INTFm_field32_get(unit, &vrfe, VRF_VFIf)) {
             continue;
         }

         /* Get vlan id. */
         if (BCM_L2_VPN_USED_GET(unit, vpn)) {
             vpn_info.vlan = soc_VRF_VFI_INTFm_field32_get(unit, &vrfe, VLANf);
         }

        /* When an entry is deleted, hw fills the gap by shifting
         * the entries up. If the item at index "i" is deleted,
         * repeat the check for the entry that was shifted up.
         */
         while (TRUE) {
             rv = bcm_er_mpls_vpn_delete (unit, vpn, &vpn_info);
             if (!BCM_SUCCESS(rv)) {
                 break;
             }
         }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_vpn_get
 * Purpose:
 *      Get the binding of interface/port/VLAN to VPN instance
 * Parameters:
 *      unit       - Device Number
 *      vpn     - VPN instance ID
 *      max_out    - max elements of the array
 *      vpn_info   - (OUT) array of interface/port/VLAN struct
 *      real_count - (OUT) actual counts
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      This port/VID binding of customer ports to VPN
 *      instance is normally done as a result of admin configuration.
 */

int
bcm_er_mpls_vpn_get (int unit, bcm_vpn_t vpn, int max_out,
                     bcm_mpls_vpn_t *vpn_info, int *real_count)
{
    vrf_vfi_intf_entry_t   vrfe;
    bcm_l2_cache_addr_t    l2ue;          /* L2 cache entry.              */
    int                    l2_idx_max;    /* L2 user table entry count.   */   
    bcm_if_t               intf;          /* L3 interface id.             */
    int                    rv;            /* Internal op return status.   */
    int i, j, vlan;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_OLD);

    if (vpn > BCM_VPN_MAX) {
        return BCM_E_PARAM;
    }

    if (!BCM_MPLS_VPN_USED_GET(unit, vpn)) {
        return BCM_E_NOT_FOUND;
    }

    *real_count = 0;

    if (BCM_L2_VPN_USED_GET(unit, vpn)) {
        BCM_IF_ERROR_RETURN(bcm_esw_l2_cache_size_get(unit, &l2_idx_max)); 
    }

    for (i = soc_mem_index_min(unit, VRF_VFI_INTFm);
         i <= soc_mem_index_max(unit, VRF_VFI_INTFm) &&
         *real_count < max_out; i++) {
        BCM_IF_ERROR_RETURN(READ_VRF_VFI_INTFm(unit, MEM_BLOCK_ANY, i, &vrfe));
        if (vpn == soc_VRF_VFI_INTFm_field32_get(unit, &vrfe, VRF_VFIf)) {
            vlan = soc_VRF_VFI_INTFm_field32_get(unit, &vrfe, VLANf);
            intf = soc_VRF_VFI_INTFm_field32_get(unit, &vrfe, L3_INTFf);

            if (BCM_L2_VPN_USED_GET(unit, vpn)) {
                /* For L2 VPNs, get the src port info from the L2_USER_ENTRYm */
                for (j = 0; j < l2_idx_max; j++) {
                    rv = bcm_esw_l2_cache_get(unit, j, &l2ue);
                    if (BCM_FAILURE(rv)) {
                        if (rv == BCM_E_NOT_FOUND) {
                            continue;
                        } else {
                            return rv;
                        }
                    }
                    /* Check for matching VLAN_ID and MASK */
                    if ((vlan == l2ue.vlan) &&
                        (BCM_MAC_IS_ZERO(l2ue.mac_mask)) &&
                        (BCM_L2_SRCPORT_MASK_ALL == l2ue.src_port_mask) &&
                        (BCM_L2_VID_MASK_ALL == l2ue.vlan_mask)) {

                        sal_memset(&vpn_info[*real_count], 0, sizeof(bcm_mpls_vpn_t));
                        vpn_info[*real_count].vpn = vpn;
                        vpn_info[*real_count].flags = BCM_MPLS_L2_VPN;
                        BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &vpn_info[*real_count].module));
                        vpn_info[*real_count].port = l2ue.src_port;
                        vpn_info[*real_count].vlan = vlan;
                        vpn_info[*real_count].l3_intf = intf;
                        if((++(*real_count)) == max_out) {
                            break;
                        }
                    }
                }
            } else {
                /* For L3 VPNs, module & port info is not used, set to -1. */
                sal_memset(&vpn_info[*real_count], 0, sizeof(bcm_mpls_vpn_t));
                vpn_info[*real_count].vpn = vpn;
                vpn_info[*real_count].flags = BCM_MPLS_L3_VPN;
                vpn_info[*real_count].module = -1;
                vpn_info[*real_count].port = -1;
                vpn_info[*real_count].vlan = vlan;
                vpn_info[*real_count].l3_intf = intf;
                *real_count += 1;
            }
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_vpn_circuit_add
 * Purpose:
 *      Add (Bind) a VC to existing VPN
 * Parameters:
 *      unit     - Device Number
 *      vpn      - The VPN to bind VC to
 *      vc_info  - Virtual circuit info
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      Assign L2 MPLS label and L2 outer header info to L2 MPLS pseudo-wire
 *      For Point-to-Multipoint connection, this function is called
 *      multiple times.
 */

int
bcm_er_mpls_vpn_circuit_add (int unit, bcm_vpn_t vpn,
			     bcm_mpls_circuit_t *vc_info)
{
    vpls_label_entry_t           vle;
    int                          i, mac_idx, idx, vlan, modid_out, port_out;
    vrf_vfi_intf_entry_t         vrfe;
    bcm_l2_cache_addr_t          l2ue;          /* L2 cache entry.              */
    int                          l2_idx = 0, l2_idx_max;
    l3intf_entry_t               l3_if_entry;
    bcm_vpn_t                    vpn_min, vpn_max;
    int                          rv;
    int                          tunnel_type;
    int                          old_adj_mac = -1;
    int                          found = FALSE;
    int                          update = FALSE;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_OLD);

    vpn_min = soc_mem_index_min(unit, VPLS_LABELm);
    vpn_max = soc_mem_index_max(unit, VPLS_LABELm);

    if ((vpn < vpn_min) || (vpn > vpn_max) ||
        (vc_info == NULL) ) {
        return BCM_E_PARAM;
    }  
    if (!BCM_L2_VPN_USED_GET(unit, vpn)) {
        return BCM_E_NOT_FOUND;
    }

    if (!BCM_L3_INTF_USED_GET(unit, vc_info->l3_intf)) {
        soc_cm_debug(DK_L3, "L3 interface not created\n");
        return BCM_E_NOT_FOUND;
    }

    /* Get size of l2 user entry table. */
    BCM_IF_ERROR_RETURN(bcm_esw_l2_cache_size_get(unit, &l2_idx_max)); 

    /*
     * Using vpn, search the VRF_VFI_INTm for an entry that 
     * uses this VFI. Extract that entry's VLAN ID, and
     * find the corresponding L2_USER_ENTRYm entry.
     */
    for (i = soc_mem_index_min(unit, VRF_VFI_INTFm);
         i <= soc_mem_index_max(unit, VRF_VFI_INTFm); i++) {
        SOC_IF_ERROR_RETURN (READ_VRF_VFI_INTFm(unit, MEM_BLOCK_ANY, i, &vrfe));
        if ((soc_VRF_VFI_INTFm_field32_get(unit, &vrfe, VRF_VFIf) != vpn) ||
            (soc_VRF_VFI_INTFm_field32_get(unit, &vrfe, VFIf) != 1) ) {
            continue;
        }
        vlan = soc_VRF_VFI_INTFm_field32_get(unit, &vrfe, VLANf);

        /* For L2 VPNs, get the src port info from the L2_USER_ENTRYm */
        for (l2_idx = 0; l2_idx < l2_idx_max; l2_idx++) {
            rv = bcm_esw_l2_cache_get(unit, l2_idx, &l2ue);
            if (BCM_FAILURE(rv)) {
                if (rv == BCM_E_NOT_FOUND) {
                    continue;
                } else {
                    return rv;
                }
            }
            /* Check for matching VLAN_ID and MASK */
            if ((vlan == l2ue.vlan) &&
                (BCM_MAC_IS_ZERO(l2ue.mac_mask)) &&
                (BCM_L2_SRCPORT_MASK_ALL == l2ue.src_port_mask) &&
                (BCM_L2_VID_MASK_ALL == l2ue.vlan_mask)) {

                found = TRUE;
                break;
            }
        }
        if (found) {
            break;
        }
    }

    if (!found) {
        soc_cm_debug(DK_L3, "Source port/vlan was not added to VPN.\n");
        return BCM_E_NOT_FOUND;

    } else if (!(l2ue.flags & BCM_L2_CACHE_DISCARD)) {

        /* The virtual circuit already exists (indicated by the fact that
         * packets matching the L2 cache entry are not getting discarded).
         * If the user didn't specify the update flag, return an error.
         */
        if ((vc_info->flags & BCM_MPLS_UPDATE)) {
            update = TRUE;
        } else {
            return BCM_E_EXISTS;
        }
    }

    /* Lock interface memory for read-modify-write. */
    MEM_LOCK(unit, L3INTFm);

    /* Get the L3_INTF entry with the adjacent dest MAC info */
    rv =  READ_L3INTFm(unit, MEM_BLOCK_ANY, vc_info->l3_intf, &l3_if_entry);
    if (BCM_FAILURE(rv)) {
        MEM_UNLOCK(unit, L3INTFm);
        return (rv);
    }

    /* Verify that the L3 initiator was set. */
    tunnel_type = soc_L3INTFm_field32_get(unit, &l3_if_entry, TUNNEL_TYPEf);
    if (tunnel_type != 0x8) {
        MEM_UNLOCK(unit, L3INTFm);
        soc_cm_debug(DK_L3, "L3 initiator not set\n");
        return BCM_E_NOT_FOUND;
    }

    vc_info->vpn = vpn;
    
    sal_memset(&vle, 0, sizeof(vpls_label_entry_t));
    soc_VPLS_LABELm_field32_set(unit, &vle, ORIGINAL_PKTf, 
                                (vc_info->flags & BCM_MPLS_ORIGINAL_PKT ?
                                 1 : 0));
    soc_VPLS_LABELm_field32_set(unit, &vle, VIDf, vc_info->vlan);
    soc_VPLS_LABELm_field32_set(unit, &vle, VC_LABELf, vc_info->label);
    soc_VPLS_LABELm_field32_set(unit, &vle, L3_INTFf, 
                                vc_info->l3_intf);

    rv = WRITE_VPLS_LABELm(unit, MEM_BLOCK_ALL, vpn, &vle);
    if (BCM_FAILURE(rv)) {
        MEM_UNLOCK(unit, L3INTFm);
        return (rv);
    }

    /* Find the index into ADJ_MAC table for the egress DA */

    rv = bcm_xgs3_adj_mac_add(unit, vc_info->dst_mac, &mac_idx);
    if (BCM_FAILURE(rv)) {
        MEM_UNLOCK(unit, L3INTFm);
        return (rv);
    }

    /* Update the L3_INTF entry with the adjacent dest MAC info */

    old_adj_mac = soc_L3INTFm_field32_get(unit, &l3_if_entry, ADJ_MAC_INDEXf);
    soc_L3INTFm_field32_set(unit, &l3_if_entry, ADJ_MAC_INDEXf, mac_idx);
    rv = WRITE_L3INTFm(unit, MEM_BLOCK_ALL, vc_info->l3_intf, &l3_if_entry);
    if (BCM_FAILURE(rv)) {
        bcm_xgs3_adj_mac_del(unit, mac_idx);
        MEM_UNLOCK(unit, L3INTFm);
        return (rv);
    }
    
    if (update == TRUE) {
        bcm_xgs3_adj_mac_del(unit, old_adj_mac);
    } else {
        MPLS_INFO(unit)->vc_table_used++;
    }
    MEM_UNLOCK(unit, L3INTFm);

    /*
     * Update the L2_USER_ENTRYm entry with vc_info->dst_mod/port/trunk.
     */
    l2ue.flags &= ~(BCM_L2_CACHE_DISCARD);

    if (vc_info->flags & BCM_MPLS_DST_TRUNK) {
        /* vc_info->dst_port is part of a trunk mapping not required */
        l2ue.dest_trunk = vc_info->dst_trunk;
        l2ue.flags |= BCM_L2_CACHE_TRUNK;
    } else { 
        /* vc_info is supplied by API - mapping needed */
       BCM_IF_ERROR_RETURN (
           _bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                  vc_info->dst_mod, vc_info->dst_port,
                                  &modid_out, &port_out));
        if (!SOC_MODID_ADDRESSABLE(unit, modid_out)) {
            return BCM_E_BADID;
        }
        if (!SOC_PORT_ADDRESSABLE(unit, port_out)) {
            return BCM_E_PORT;
        }
        l2ue.dest_modid = vc_info->dst_mod;
        l2ue.dest_port = vc_info->dst_port;
    }
    BCM_IF_ERROR_RETURN(bcm_esw_l2_cache_set(unit, l2_idx, &l2ue, &idx));

    return BCM_E_NONE;
}

/*      
 * Function:
 *      _bcm_mpls_circuit_table_entries_clear
 * Purpose:
 *      Delete the following table entries for VC from existing VPN
 *       L3INTF
 *       EGRESS_ADJACENT_MAC
 *       L2_USER_ENTRY
 *       VPLS_VC_LABEL
 * Parameters:
 *      unit     - Device Number
 *      vpn      - The VPN to bind VC to
 *      vc_info  - Virtual circuit info
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      Deletes all the VC related information in L3INTF, L2_USER_TABLE, 
 *      EGRESS_ADJACENT_MAC, VPLS_VC_LABEL tables. This function assumes
 *      that only one VC is supported per VPN
 */

int
_bcm_mpls_circuit_table_entries_clear(int unit, bcm_vpn_t vpn,
                                      bcm_mpls_circuit_t *vc_info)
{
    vpls_label_entry_t    vle;
    l3intf_entry_t        l3_if_entry;
    vrf_vfi_intf_entry_t  vrfe;
    bcm_l2_cache_addr_t   l2ue;          /* L2 cache entry.              */
    int                   l2_idx_max;    /* L2 user table entry count.   */   
    int                   rv;            /* Internal op return status.   */
    int                   i, j, mac_idx, idx, vlan;
 
    /* Clearing the L3 table */

    SOC_IF_ERROR_RETURN
      (READ_L3INTFm(unit, MEM_BLOCK_ANY, vc_info->l3_intf, &l3_if_entry));
   
    /*getting the Adjecent mac index from the l3 intf table */
    mac_idx = soc_L3INTFm_field32_get(unit, &l3_if_entry, ADJ_MAC_INDEXf);

    /*clear the entry in the  EGRESS_ADJECENT_MAC table*/
    BCM_IF_ERROR_RETURN(bcm_xgs3_adj_mac_del(unit, mac_idx));
    
    /*Reset the tunnel types*/ 
    soc_L3INTFm_field32_set(unit, &l3_if_entry, TUNNEL_TYPEf, 0);
    
    /*clear Adjecent mac index in  the l3 intf table */
    soc_L3INTFm_field32_set(unit, &l3_if_entry, ADJ_MAC_INDEXf, 0);

    SOC_IF_ERROR_RETURN
      (WRITE_L3INTFm(unit, MEM_BLOCK_ALL, vc_info->l3_intf, &l3_if_entry));
    
    /*Clear the L2 USER ENTRY TABLE*/
    BCM_IF_ERROR_RETURN(bcm_esw_l2_cache_size_get(unit, &l2_idx_max)); 
    
    /*
     * Using vpn, search the VRF_VFI_INTm for all entries that 
     * use this VFI. Extract that entry's VLAN ID, and
     * update the corresponding L2_USER_ENTRYm entry.
     */

    for (i = soc_mem_index_min(unit, VRF_VFI_INTFm);
         i <= soc_mem_index_max(unit, VRF_VFI_INTFm); i++) {
        SOC_IF_ERROR_RETURN
            (READ_VRF_VFI_INTFm(unit, MEM_BLOCK_ANY, i, &vrfe));
        if ((soc_VRF_VFI_INTFm_field32_get(unit, &vrfe, VRF_VFIf) != vpn) ||
            (soc_VRF_VFI_INTFm_field32_get(unit, &vrfe, VFIf) != 1) ) {
            continue;
        }
        vlan = soc_VRF_VFI_INTFm_field32_get(unit, &vrfe, VLANf);
        /* For L2 VPNs, get the src port info from the L2_USER_ENTRYm */
        for (j = 0; j < l2_idx_max; j++) {
            rv = bcm_esw_l2_cache_get(unit, j, &l2ue);
            if (BCM_FAILURE(rv)) {
                if (rv == BCM_E_NOT_FOUND) {
                    continue;
                } else {
                    return rv;
                }
            }
            /* Check for matching VLAN_ID and MASK */
            if ((vlan == l2ue.vlan) &&
                (BCM_MAC_IS_ZERO(l2ue.mac_mask)) &&
                (BCM_L2_SRCPORT_MASK_ALL == l2ue.src_port_mask) &&
                (BCM_L2_VID_MASK_ALL == l2ue.vlan_mask)) {

                l2ue.dest_modid = 0;  
                l2ue.dest_trunk = 0;  
                l2ue.dest_port = 0;  
                l2ue.flags &= ~BCM_L2_CACHE_TRUNK;  
                l2ue.flags |= BCM_L2_CACHE_DISCARD;  
                BCM_IF_ERROR_RETURN(bcm_esw_l2_cache_set(unit, j, &l2ue, &idx));
            }
        }
    } 
    sal_memset(&vle, 0, sizeof(vpls_label_entry_t));
    SOC_IF_ERROR_RETURN
         (WRITE_VPLS_LABELm(unit, MEM_BLOCK_ALL, vpn, &vle));

    return BCM_E_NONE;
}

/*      
 * Function:
 *      bcm_mpls_vpn_circuit_delete
 * Purpose:
 *      Delete (unbind) a VC from existing VPN
 * Parameters:
 *      unit     - Device Number
 *      vpn      - The VPN to bind VC to
 *      vc_info  - Virtual circuit info
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      Delete L2 MPLS label and L2 outer header info to L2 MPLS pseudo-wire
 *      For Point-to-Multipoint connection, this function is called
 *      multiple times.
 */

int
bcm_er_mpls_vpn_circuit_delete (int unit, bcm_vpn_t vpn,
				bcm_mpls_circuit_t *vc_info)
{
    bcm_mpls_circuit_t    test_vc_info;
    int                   num_vc;
    bcm_vpn_t             vpn_min, vpn_max;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_OLD);

    vpn_min = soc_mem_index_min(unit, VPLS_LABELm);
    vpn_max = soc_mem_index_max(unit, VPLS_LABELm);
	
    if ( (vpn < vpn_min) || (vpn > vpn_max) ||
	 (vc_info == NULL) ) {
        return BCM_E_PARAM;
    }  
    if (!BCM_L2_VPN_USED_GET(unit, vpn)) {
        return BCM_E_NOT_FOUND;
    }
    if (!BCM_L3_INTF_USED_GET(unit, vc_info->l3_intf)) {
        soc_cm_debug(DK_L3, "L3 interface not created\n");
        return BCM_E_NOT_FOUND;
    }

    
    BCM_IF_ERROR_RETURN
      (bcm_esw_mpls_vpn_circuit_get(unit, vpn, 1, &test_vc_info, &num_vc));

    if (test_vc_info.vlan != vc_info->vlan &&
        test_vc_info.label != vc_info->label &&
        test_vc_info.l3_intf != vc_info->l3_intf) {
        return BCM_E_NOT_FOUND;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_mpls_circuit_table_entries_clear(unit, vpn, &test_vc_info));
    
    MPLS_INFO(unit)->vc_table_used--;

    return BCM_E_NONE;
}

/*     
 * Function:
 *      bcm_mpls_vpn_circuit_delete_all
 * Purpose:
 *      Delete (unbind) all VCs from existing VPN
 * Parameters:
 *      unit     - Device Number
 *      vpn      - The VPN to bind VC to
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_er_mpls_vpn_circuit_delete_all (int unit, bcm_vpn_t vpn)
{
    bcm_mpls_circuit_t    test_vc_info;
    int                   num_vc;
    bcm_vpn_t             vpn_min, vpn_max;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_OLD);
 
    vpn_min = soc_mem_index_min(unit, VPLS_LABELm);
    vpn_max = soc_mem_index_max(unit, VPLS_LABELm);
	
    if ((vpn < vpn_min) || (vpn > vpn_max)) {
        return BCM_E_PARAM;
    }  
    if (!BCM_L2_VPN_USED_GET(unit, vpn)) {
        return BCM_E_NOT_FOUND;
    }
    
    BCM_IF_ERROR_RETURN
      (bcm_esw_mpls_vpn_circuit_get(unit, vpn, 1, &test_vc_info, &num_vc));
    test_vc_info.vpn = vpn;

    BCM_IF_ERROR_RETURN
        (_bcm_mpls_circuit_table_entries_clear(unit, vpn, &test_vc_info));

    

    MPLS_INFO(unit)->vc_table_used--;
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_vpn_circuit_get
 * Purpose:
 *      Get the VC label and header info for the VPN
 * Parameters:   
 *      unit       - Device Number
 *      vpn     - The VPN to bind VC to
 *      vc_array    - max elements of the array
 *      vc_info    - (OUT) Virtual circuit info
 *      vc_count - (OUT) actual counts
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      The current ER implementation only supports Point-to-Point
 *      pseudo-wire, so one VC per VPN.
 */

int
bcm_er_mpls_vpn_circuit_get (int unit, bcm_vpn_t vpn, int vc_array,
			     bcm_mpls_circuit_t *vc_info, int *vc_count)
{
    vpls_label_entry_t           vle;
    int                          i, j, k, mac_idx;
    int                          vlan;
    bcm_vpn_t                    vpn_min, vpn_max;
    bcm_l2_cache_addr_t          l2ue;          /* L2 cache entry.            */
    int                          l2_idx_max;    /* L2 user table entry count. */   
    int                          rv;            /* Internal op return value.  */
    vrf_vfi_intf_entry_t         vrfe;
    l3intf_entry_t               l3_if_entry;
    egress_adjacent_mac_entry_t  amac_entry;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_OLD);

    vpn_min = soc_mem_index_min(unit, VPLS_LABELm);
    vpn_max = soc_mem_index_max(unit, VPLS_LABELm);

    if ( (vpn < vpn_min) || (vpn > vpn_max) ||
         (vc_array < 0) ||
         (vc_array > soc_mem_index_count(unit, VPLS_LABELm)) ||
         (vc_info == NULL) || (vc_count == NULL) ) {
        return BCM_E_PARAM;
    }

    if (!BCM_L2_VPN_USED_GET(unit, vpn)) {
        return BCM_E_NOT_FOUND;
    }

    /* Get l2 user entry table size. */
    BCM_IF_ERROR_RETURN(bcm_esw_l2_cache_size_get(unit, &l2_idx_max)); 

    
    vc_array = 1; /* Current ER supports one VC per VPN */  

    sal_memset(vc_info, 0, sizeof(bcm_mpls_circuit_t) * vc_array);
    *vc_count = 0;
    i = 0;
    SOC_IF_ERROR_RETURN
        (READ_VPLS_LABELm(unit, MEM_BLOCK_ANY, vpn, &vle));
    vc_info[i].vpn = vpn;
    if (soc_VPLS_LABELm_field32_get(unit, &vle, ORIGINAL_PKTf)) {
        vc_info[i].flags = BCM_MPLS_ORIGINAL_PKT;
    }
    vc_info[i].vlan = soc_VPLS_LABELm_field32_get(unit, &vle, VIDf);
    vc_info[i].label = soc_VPLS_LABELm_field32_get(unit, &vle, VC_LABELf);
    vc_info[i].l3_intf = soc_VPLS_LABELm_field32_get(unit, &vle, 
                                                     L3_INTFf);

    if (!BCM_L3_INTF_USED_GET(unit, vc_info[i].l3_intf)) {
        soc_cm_debug(DK_L3, "L3 interface not created\n");
        return (BCM_E_NOT_FOUND);
    }

    /* L3 interface info */
    BCM_IF_ERROR_RETURN
        (READ_L3INTFm(unit, MEM_BLOCK_ANY,
                      vc_info[i].l3_intf, &l3_if_entry));

    /* Find the index into ADJ_MAC table for tunnel DA */
    mac_idx = soc_L3INTFm_field32_get(unit, &l3_if_entry, ADJ_MAC_INDEXf);
    BCM_IF_ERROR_RETURN
       (READ_EGRESS_ADJACENT_MACm(unit, MEM_BLOCK_ANY, mac_idx, &amac_entry));
    soc_EGRESS_ADJACENT_MACm_mac_addr_get(unit,
                                          &amac_entry, MAC_ADDRf, vc_info[i].dst_mac);

    for (j = soc_mem_index_min(unit, VRF_VFI_INTFm);
         j <= soc_mem_index_max(unit, VRF_VFI_INTFm); j++) {

        BCM_IF_ERROR_RETURN(READ_VRF_VFI_INTFm(unit, MEM_BLOCK_ANY, j, &vrfe));
        if (vpn != soc_VRF_VFI_INTFm_field32_get(unit, &vrfe, VRF_VFIf) || 
            (!soc_VRF_VFI_INTFm_field32_get(unit, &vrfe, VFIf))) {
            continue;
        }
        vlan = soc_VRF_VFI_INTFm_field32_get(unit, &vrfe, VLANf);

        /* For L2 VPNs, get the src port info from the L2_USER_ENTRYm */
        for (k = 0; k < l2_idx_max; k++) {
            rv = bcm_esw_l2_cache_get(unit, k, &l2ue);
            if (BCM_FAILURE(rv)) {
                if (rv == BCM_E_NOT_FOUND) {
                    continue;
                } else {
                    return rv;
                }
            }
            /* Check for matching VLAN_ID and MASK */
            if ((vlan == l2ue.vlan) &&
                (BCM_MAC_IS_ZERO(l2ue.mac_mask)) &&
                (BCM_L2_SRCPORT_MASK_ALL == l2ue.src_port_mask) &&
                (BCM_L2_VID_MASK_ALL == l2ue.vlan_mask)) {

                vc_info[i].dst_mod = l2ue.dest_modid;
                if (l2ue.flags & BCM_L2_CACHE_TRUNK)  {
                    vc_info[i].flags |= BCM_MPLS_DST_TRUNK;
                    vc_info[i].dst_trunk = l2ue.dest_trunk;;
                } else {
                    vc_info[i].dst_port = l2ue.dest_port;
                }
                (*vc_count)++;
                break;
            }
        }
    }
    return BCM_E_NONE;
}
/*  
 * Function:
 *      bcm_mpls_l3_initiator_set
 * Purpose:
 *      Set the FEC to MPLS label mapping
 * Parameters:
 *      unit - Device Number
 *      intf - The egress L3 interface
 *      mpls_label - MPLS label and header information
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_er_mpls_l3_initiator_set (int unit, bcm_l3_intf_t *intf,
                              bcm_mpls_l3_initiator_t *mpls_label)
{
    l3intf_entry_t l3_if_entry;
    int type;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_OLD);

    if ( (intf == NULL) || (mpls_label == NULL) || 
	 (intf->l3a_intf_id >= L3_INFO(unit)->l3_intf_table_size) ) {
        return BCM_E_PARAM;
    }

    if (!BCM_L3_INTF_USED_GET(unit, intf->l3a_intf_id)) {
        soc_cm_debug(DK_L3, "L3 interface not created\n");
        return BCM_E_NOT_FOUND;
    }

    /* L3 interface info */
    BCM_IF_ERROR_RETURN
        (READ_L3INTFm(unit, MEM_BLOCK_ANY,
                      intf->l3a_intf_id, &l3_if_entry));

    /* Check for IP tunnel conflicts */
    type = soc_L3INTFm_field32_get(unit, &l3_if_entry, TUNNEL_TYPEf);
    if (type != 0) {
        soc_cm_debug(DK_L3, "L3 interface already used for other tunnels\n");
        return BCM_E_PARAM;
    }
    soc_L3INTFm_field32_set(unit, &l3_if_entry, TUNNEL_TYPEf, 8); /* MPLS */

    if (mpls_label->flags & BCM_MPLS_INSERT_LABEL1) {
        soc_L3INTFm_field32_set(unit, &l3_if_entry, INSERT_L3_MPLS_LABEL1f, 1);
        soc_L3INTFm_field32_set(unit, &l3_if_entry, MPLS_TUNNEL_LABEL1f,
                                mpls_label->label1);
    } 
    if (mpls_label->flags & BCM_MPLS_INSERT_LABEL2) {
        soc_L3INTFm_field32_set(unit, &l3_if_entry, INSERT_L3_MPLS_LABEL2f, 1);
        soc_L3INTFm_field32_set(unit, &l3_if_entry, MPLS_TUNNEL_LABEL2f, 
                                mpls_label->label2);
    }

    soc_L3INTFm_field32_set(unit, &l3_if_entry, MPLS_TUNNEL_LABEL_QOS_INDEXf,
                            mpls_label->priority1);
    soc_L3INTFm_field32_set(unit, &l3_if_entry, QOS_INDEXf, 
                            mpls_label->priority2);
    soc_L3INTFm_field32_set(unit, &l3_if_entry, TUNNEL_TTLf, mpls_label->ttl1);
    soc_L3INTFm_field32_set(unit, &l3_if_entry, TTLf, mpls_label->ttl2);

    BCM_IF_ERROR_RETURN
        (WRITE_L3INTFm(unit, MEM_BLOCK_ALL, intf->l3a_intf_id, &l3_if_entry));

    return BCM_E_NONE;
}

/*  
 * Function:
 *      bcm_mpls_l3_initiator_clear
 * Purpose:
 *      Clear the FEC to MPLS label mapping
 * Parameters:
 *      unit - Device Number
 *      intf - The egress L3 interface
 * Returns: 
 *      BCM_E_XXXX
 */

int
bcm_er_mpls_l3_initiator_clear (int unit, bcm_l3_intf_t *intf)
{
    l3intf_entry_t l3_if_entry;
    int type;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_OLD);

    if (intf->l3a_intf_id >= L3_INFO(unit)->l3_intf_table_size) {
        return BCM_E_PARAM;
    }

    if (!BCM_L3_INTF_USED_GET(unit, intf->l3a_intf_id)) {
        soc_cm_debug(DK_L3, "L3 interface not created\n");
        return BCM_E_NOT_FOUND;
    }

    /* L3 interface info */
    BCM_IF_ERROR_RETURN
        (READ_L3INTFm(unit, MEM_BLOCK_ANY, intf->l3a_intf_id, &l3_if_entry));

    /* Check for IP tunnel conflicts */
    type = soc_L3INTFm_field32_get(unit, &l3_if_entry, TUNNEL_TYPEf);
    if (type != 8) {
        /* type != MPLS */
        return BCM_E_NONE;
    }

    soc_L3INTFm_field32_set(unit, &l3_if_entry, TUNNEL_TYPEf, 0);
    soc_L3INTFm_field32_set(unit, &l3_if_entry, INSERT_L3_MPLS_LABEL1f, 0);
    soc_L3INTFm_field32_set(unit, &l3_if_entry, MPLS_TUNNEL_LABEL1f, 0);
    soc_L3INTFm_field32_set(unit, &l3_if_entry, INSERT_L3_MPLS_LABEL2f, 0);
    soc_L3INTFm_field32_set(unit, &l3_if_entry, MPLS_TUNNEL_LABEL2f, 0);
    soc_L3INTFm_field32_set(unit, &l3_if_entry, MPLS_TUNNEL_LABEL_QOS_INDEXf, 0);
    soc_L3INTFm_field32_set(unit, &l3_if_entry, QOS_INDEXf, 0);
    soc_L3INTFm_field32_set(unit, &l3_if_entry, TUNNEL_TTLf, 0);
    soc_L3INTFm_field32_set(unit, &l3_if_entry, TTLf, 0);

    BCM_IF_ERROR_RETURN
        (WRITE_L3INTFm(unit, MEM_BLOCK_ALL, intf->l3a_intf_id, &l3_if_entry));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_l3_initiator_get
 * Purpose:
 *      Get the FEC to MPLS label mapping info
 * Parameters:
 *      unit - Device Number
 *      intf - The egress L3 interface
 *      mpls_label - MPLS header information
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_er_mpls_l3_initiator_get (int unit, bcm_l3_intf_t *intf,
                              bcm_mpls_l3_initiator_t *mpls_label)
{
    l3intf_entry_t l3_if_entry;
    egress_adjacent_mac_entry_t amac_entry;
    int mac_idx, type;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_OLD);

    if (intf->l3a_intf_id >= L3_INFO(unit)->l3_intf_table_size) {
        return BCM_E_PARAM;
    }

    if (!BCM_L3_INTF_USED_GET(unit, intf->l3a_intf_id)) {
        soc_cm_debug(DK_L3, "L3 interface not created\n");
        return BCM_E_NOT_FOUND;
    }

    /* L3 interface info */
    BCM_IF_ERROR_RETURN
        (READ_L3INTFm(unit, MEM_BLOCK_ANY,
                      intf->l3a_intf_id, &l3_if_entry));

    /* Check for IP tunnel conflicts */
    type = soc_L3INTFm_field32_get(unit, &l3_if_entry, TUNNEL_TYPEf);
    if (type != 8) {
        /* type != MPLS */
        soc_cm_debug(DK_L3, "L3 interface not MPLS tunnel\n");
        return BCM_E_PARAM;
    }

    sal_memset(mpls_label, 0, sizeof(bcm_mpls_l3_initiator_t));

    if (soc_L3INTFm_field32_get(unit, &l3_if_entry, INSERT_L3_MPLS_LABEL1f)) {
        mpls_label->flags |= BCM_MPLS_INSERT_LABEL1;
        mpls_label->label1 = 
            soc_L3INTFm_field32_get(unit, &l3_if_entry, MPLS_TUNNEL_LABEL1f);
    }

    if (soc_L3INTFm_field32_get(unit, &l3_if_entry, INSERT_L3_MPLS_LABEL2f)) {
        mpls_label->flags |= BCM_MPLS_INSERT_LABEL2;
        mpls_label->label2 = 
            soc_L3INTFm_field32_get(unit, &l3_if_entry, MPLS_TUNNEL_LABEL2f);
    }

    mpls_label->priority1 = 
        soc_L3INTFm_field32_get(unit, &l3_if_entry, 
                                MPLS_TUNNEL_LABEL_QOS_INDEXf);
    mpls_label->priority2 = 
        soc_L3INTFm_field32_get(unit, &l3_if_entry, QOS_INDEXf);

    mpls_label->ttl1 = soc_L3INTFm_field32_get(unit, &l3_if_entry, TUNNEL_TTLf);
    mpls_label->ttl2 = soc_L3INTFm_field32_get(unit, &l3_if_entry, TTLf);

    /* Find the index into ADJ_MAC table for tunnel DA */
    mac_idx = soc_L3INTFm_field32_get(unit, &l3_if_entry, ADJ_MAC_INDEXf);
    BCM_IF_ERROR_RETURN
         (READ_EGRESS_ADJACENT_MACm(unit, MEM_BLOCK_ANY, mac_idx, &amac_entry));
    soc_EGRESS_ADJACENT_MACm_mac_addr_get(unit,
                          &amac_entry, MAC_ADDRf, mpls_label->dmac);

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_l3_initiator_clear_all
 * Purpose:
 *      Clear all the FEC to MPLS label mapping
 * Parameters:
 *      unit - Device Number
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_er_mpls_l3_initiator_clear_all (int unit)
{
    l3intf_entry_t l3_if_entry;
    int type, index_min, index_max, i;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_OLD);

    index_min = soc_mem_index_min(unit, L3INTFm);
    index_max = soc_mem_index_max(unit, L3INTFm);

    if (!L3_INFO(unit)->l3_initialized || L3_INFO(unit)->l3intf_count == 0) {
        return BCM_E_NONE;
    }

    for (i = index_min; i <= index_max; i++) {
        if (BCM_L3_INTF_USED_GET(unit, i)) {
            BCM_IF_ERROR_RETURN
                (READ_L3INTFm(unit, MEM_BLOCK_ANY, i, &l3_if_entry));
            type = soc_L3INTFm_field32_get(unit, &l3_if_entry, TUNNEL_TYPEf);
            if (type != 8) {
                /* type != MPLS */
                continue;
            }

            soc_L3INTFm_field32_set(unit, &l3_if_entry, TUNNEL_TYPEf, 0);
            soc_L3INTFm_field32_set(unit, &l3_if_entry, INSERT_L3_MPLS_LABEL1f, 0);
            soc_L3INTFm_field32_set(unit, &l3_if_entry, MPLS_TUNNEL_LABEL1f, 0);
            soc_L3INTFm_field32_set(unit, &l3_if_entry, INSERT_L3_MPLS_LABEL2f, 0);
            soc_L3INTFm_field32_set(unit, &l3_if_entry, MPLS_TUNNEL_LABEL2f, 0);
            soc_L3INTFm_field32_set(unit, &l3_if_entry,
                                    MPLS_TUNNEL_LABEL_QOS_INDEXf, 0);
            soc_L3INTFm_field32_set(unit, &l3_if_entry, QOS_INDEXf, 0);
            soc_L3INTFm_field32_set(unit, &l3_if_entry, TUNNEL_TTLf, 0);
            soc_L3INTFm_field32_set(unit, &l3_if_entry, TTLf, 0);

            BCM_IF_ERROR_RETURN
                (WRITE_L3INTFm(unit, MEM_BLOCK_ALL, i, &l3_if_entry));
        }
    }

    return BCM_E_NONE;
}

/*
 * Translate action into hardware OPCODE
 */
STATIC int
bcm_mpls_action_to_opcode(bcm_mpls_action_t act)
{
    switch (act) {
    case BCM_MPLS_ACTION_SWAP:
        return BCM_MPLS_OPCODE_SWAP;
    case BCM_MPLS_ACTION_SWAP_PUSH:
        return BCM_MPLS_OPCODE_SWAP_PUSH;
    case BCM_MPLS_ACTION_SWAP_NEW_DOMAIN:
        return BCM_MPLS_OPCODE_SWAP_NEW_DOMAIN;
    case BCM_MPLS_ACTION_PHP:
        return BCM_MPLS_OPCODE_PHP;
    case BCM_MPLS_ACTION_POP_L2_SWITCH:
        return BCM_MPLS_OPCODE_POP_L2_SWITCH;
    case BCM_MPLS_ACTION_POP_DST_MOD_PORT:
        return BCM_MPLS_OPCODE_POP_DST_MOD_PORT;
    case BCM_MPLS_ACTION_POP_L3_SWITCH:
        return BCM_MPLS_OPCODE_POP_L3_SWITCH;
    case BCM_MPLS_ACTION_POP_L3_NEXTHOP:
        return BCM_MPLS_OPCODE_POP_L3_NEXTHOP;
    case BCM_MPLS_ACTION_POP_SEARCH:
        return BCM_MPLS_OPCODE_POP_SEARCH;
    default:
        break;
    }

    return -1;
}

/*
 * Translate hardware OPCODE to action
 */
STATIC bcm_mpls_action_t
bcm_mpls_opcode_to_action(int opcode)
{
    switch (opcode) {
    case BCM_MPLS_OPCODE_SWAP:
        return BCM_MPLS_ACTION_SWAP;
    case BCM_MPLS_OPCODE_SWAP_PUSH:
        return BCM_MPLS_ACTION_SWAP_PUSH;
    case BCM_MPLS_OPCODE_SWAP_NEW_DOMAIN:
        return BCM_MPLS_ACTION_SWAP_NEW_DOMAIN;
    case BCM_MPLS_OPCODE_PHP:
        return BCM_MPLS_ACTION_PHP;
    case BCM_MPLS_OPCODE_POP_L2_SWITCH:
        return BCM_MPLS_ACTION_POP_L2_SWITCH;
    case BCM_MPLS_OPCODE_POP_DST_MOD_PORT:
        return BCM_MPLS_ACTION_POP_DST_MOD_PORT;
    case BCM_MPLS_OPCODE_POP_L3_SWITCH:
        return BCM_MPLS_ACTION_POP_L3_SWITCH;
    case BCM_MPLS_OPCODE_POP_L3_NEXTHOP:
        return BCM_MPLS_ACTION_POP_L3_NEXTHOP;
    case BCM_MPLS_OPCODE_POP_SEARCH:
        return BCM_MPLS_ACTION_POP_SEARCH;
    default:
        break;
    }

    return -1;
}

/* Macros for bcm_mpls_set/get() */
#define OP_L2                  (1 << 0)
#define OP_L3                  (1 << 1)
#define OP_LSR                 (1 << 2)
#define OP_NEXT_HOP            (1 << 3)
#define OP_SWAP                (1 << 4)

/*
 * Function:
 *      bcm_mpls_switch_add
 * Purpose:
 *      Add MPLS label swapping, label popping, and packet forwarding
 *      information.  This will set up the LPM table (MPLS view) to:
 *   1) Switch MPLS labeled packets at LSR (action codes 0, 1, 2, 3, 4, 8)
 *   2) L2 MPLS payload forwarding at the VC termination point
 *      to properly forward the "original" L2 payload when VC
 *      terminates (action code 5 or 6)
 *   3) Add VRF information for L3 MPLS payload (IP packet)
 *      forwarding at the LSP termination point.  This is used
 *      to properly forward the "original" IP payload at the
 *      LSP edge LSR based on the original payload IP header
 *      plus the VRF info. (action code 7)
 * Parameters:
 *      unit - Device Number
 *      switch_info - (IN) L3 MPLS label switching and packet forwarding info
 *                      src_label1     \
 *                      src_label2      \   Key
 *                      src_port        /
 *                      src_mod        /
 *                      src_trunk     /
 *                      action
 *                      src_l3_intf    \
 *                      swap_label      \
 *                      push_label1      \
 *                      push_label2       \
 *                      next_intf          \  LSR case
 *                      next_vlan          /
 *                      next_mac          /
 *                      next_modid       /
 *                      next_port       /
 *                      next_trunk     /
 *                      vpn           -     L2/L3 payload forwarding
 *                      dst_port      \
 *                      dst_modid      \    L2 payload forwarding
 *                      dst_vid        /
 *                      dst_trunk     /
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_er_mpls_switch_add (int unit, bcm_mpls_switch_t *si)
{
#define FLAG(name)             ((si->flags & name) != 0)
    defip_alg_entry_t      lpm_key, lpm_entry;
    bcm_l3_egress_t        nh_info;
    int                    defip_idx, nh_idx = -1;
    int                    orig_nh_idx = -1; 
    int                    op = 0, rv = 0;
    int                    found = FALSE;
    uint32               flags=0;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_OLD);

    if (si == NULL) {
        return BCM_E_PARAM;
    }

    switch (si->action) {
    case BCM_MPLS_ACTION_SWAP:               /* Opcode 0 */
    case BCM_MPLS_ACTION_SWAP_PUSH:          /* Opcode 1 */
    case BCM_MPLS_ACTION_SWAP_NEW_DOMAIN:    /* Opcode 2 */
    case BCM_MPLS_ACTION_PHP:                /* Opcode 3 */
        op |= OP_LSR | OP_NEXT_HOP | OP_SWAP;
        break;
    case BCM_MPLS_ACTION_POP_SEARCH:         /* Opcode 9 */
        if (!soc_feature(unit, soc_feature_mpls_pop_search)) {
            return BCM_E_UNAVAIL;
        }
        op |= OP_LSR;
        break;
    case BCM_MPLS_ACTION_POP_L2_SWITCH:      /* Opcode 5 */
    case BCM_MPLS_ACTION_POP_DST_MOD_PORT:   /* Opcode 6 */
        op |= OP_L2;
        break;
    case BCM_MPLS_ACTION_POP_L3_SWITCH:      /* Opcode 7 */
        op |= OP_L3;
        break;
    case BCM_MPLS_ACTION_POP_L3_NEXTHOP:     /* Opcode 8 */
        op |= OP_L3 | OP_NEXT_HOP;
        break;
    default:
        return BCM_E_PARAM;
    }

    /* 
     * All actions use L3_DEFIP_ALGm. Find and use an existing 
     * entry (depending on BCM_MPLS_UPDATE flag). If no existing
     * entry, add a new entry.
     */

    /* Build key for L3_DEFIP_ALGm search */
    sal_memset(&lpm_key, 0, sizeof(defip_alg_entry_t));
    soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key,
                                  FFf, SOC_ER_DEFIP_FF_FIELD);
    /* 
     * If MPLS_PORT_CHECKf=1 in the ingress port's PORT_TAB, then
     * the SRCMODf and SRCPORTf fields must be filled in with
     * the desired ingress mod/port or trunk. 
     * If MPLS_PORT_CHECKf=0, then the SRCMODf and SRCPORTf fields 
     * must be filled in with all ones (0x3f).
     *
     * The MPLS_PORT_CHECKf is controlled with the switch 
     * control, bcmSwitchMplsIngressPortCheck.
     */

    if ((si->src_mod == -1) || (si->src_port == -1)) {
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, SRCMODf, 0x3f);
	soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, SRCPORTf, 0x3f);
    } else if (FLAG(BCM_MPLS_SRC_TRUNK)) {
        MPLS_TRUNK_CHECK(si->src_trunk);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, SRCMODf, 
				      BCM_TRUNK_TO_MODIDf(unit, si->src_trunk));
	soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, SRCPORTf, 
				      BCM_TRUNK_TO_TGIDf(unit, si->src_trunk));
    } else {
	MPLS_MEMPARAM32_CHECK(unit, L3_DEFIP_ALGm, SRCMODf, si->src_mod);
	MPLS_MEMPARAM32_CHECK(unit, L3_DEFIP_ALGm, SRCPORTf, si->src_port);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, SRCMODf, 
				      si->src_mod);
	soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, SRCPORTf, 
				      si->src_port);
    }
    MPLS_MEMPARAM32_CHECK(unit, L3_DEFIP_ALGm, MPLS_LABEL1f, si->src_label1);
    soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key,
                                  MPLS_LABEL1f, si->src_label1);
    if (FLAG(BCM_MPLS_ONE_LABEL_LOOKUP)) {
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key,
                              KEY_TYPEf, SOC_ER_DEFIP_KEY_TYPE_MPLS_1L);
    } else {
        MPLS_MEMPARAM32_CHECK(unit, L3_DEFIP_ALGm, MPLS_LABEL2f, 
			      si->src_label2);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key,
				      MPLS_LABEL2f, si->src_label2);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key,
                              KEY_TYPEf, SOC_ER_DEFIP_KEY_TYPE_MPLS_2L);
    }

    rv = soc_er_defip_lookup(unit, &defip_idx, &lpm_key, &lpm_entry);
    if(rv == SOC_E_NONE) {
        if (!FLAG(BCM_MPLS_UPDATE))  {
            /* Existing entry found, but not allowed to stomp on it */
            return (BCM_E_EXISTS);
        } 
        orig_nh_idx = 
            soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, NEXT_HOP_INDEXf);
        found = TRUE;
    } else if (rv != SOC_E_NOT_FOUND) {
        return rv;
    }

    /* 
     * Discard defip_idx and lpm_entry. Just continue building 
     * new/updated entry in lpm_key (for later insertion/overwrite).  
     */

    /* MPLS items common to all actions */
    soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, MPLS_ACTIONf, 
				  bcm_mpls_action_to_opcode(si->action));
    soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, MPLS_USE_OUTER_TTLf, 
				  FLAG(BCM_MPLS_USE_OUTER_TTL));
    soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, MPLS_USE_OUTER_EXPf, 
				  FLAG(BCM_MPLS_USE_OUTER_EXP));
    soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, 
				  MPLS_DONT_CHANGE_INNER_EXPf,
				  FLAG(BCM_MPLS_NO_CHANGE_INNER_EXP));
    if (FLAG(BCM_MPLS_USE_PRI)) {
        MPLS_MEMPARAM32_CHECK(unit, L3_DEFIP_ALGm, MPLS_LABEL_PRIf, 
			      si->label_pri);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, MPLS_USE_PRIf, 1);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, MPLS_LABEL_PRIf,
				      si->label_pri);
    }
    MPLS_MEMPARAM32_CHECK(unit, L3_DEFIP_ALGm, MPLS_IINTFf, si->src_l3_intf);
    soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, MPLS_IINTFf,
				  si->src_l3_intf);

    if (si->action == BCM_MPLS_ACTION_POP_L3_SWITCH) {
	MPLS_MEMPARAM32_CHECK(unit, L3_DEFIP_ALGm, VRF_IDf, si->vpn);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, VRF_IDf, si->vpn);
    }

    if (op & OP_NEXT_HOP) {
        bcm_if_t     intf_index;  /* L3 interface id. */
        int          egress_mode; /* Device operates in egress mode. */

        if (op & OP_SWAP) {
            MPLS_MEMPARAM32_CHECK(unit, L3_DEFIP_ALGm, MPLS_NEW_LABELf, 
                                  si->swap_label);
            soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, MPLS_NEW_LABELf, 
                                          si->swap_label);
        }

        bcm_l3_egress_t_init(&nh_info);
/*        sal_memset(&nh_info, 0, sizeof(bcm_l3_egress_t));*/
        BCM_IF_ERROR_RETURN(bcm_xgs3_l3_egress_mode_get(unit, &egress_mode)); 
        if (egress_mode) {
            /* Increment egress object reference count. */
            BCM_IF_ERROR_RETURN
                (bcm_xgs3_get_nh_from_egress_object(unit, si->next_intf, 
                                                    &flags, 1, &nh_idx));
            if (nh_idx < 0) {
               return BCM_E_PARAM;
            }
            /* Extract next hop info (L3 interface id). */
            BCM_IF_ERROR_RETURN (bcm_xgs3_nh_get(unit, nh_idx, &nh_info));
            intf_index = nh_info.intf;
        } else {
            int          e_trunk_flag=0;
            int          e_trunk;
            int          e_mod;
            int          e_port;

            /* Determine the NEXT_HOP_INDEX */
            sal_memcpy(nh_info.mac_addr, si->next_mac, sizeof(bcm_mac_t)); 
            intf_index = nh_info.intf = si->next_intf;

            if (op & OP_LSR) {
                e_trunk_flag = FLAG(BCM_MPLS_NEXT_TRUNK);
                e_trunk = si->next_trunk;
                e_mod = si->next_mod;
                e_port = si->next_port;
            }
            else {
                e_trunk_flag = FLAG(BCM_MPLS_DST_TRUNK);
                e_trunk = si->dst_trunk;
                e_mod = si->dst_mod;
                e_port = si->dst_port;
            }
            if (e_trunk_flag) {
                MPLS_TRUNK_CHECK(e_trunk);
                nh_info.flags = BCM_L3_TGID;
                nh_info.trunk = e_trunk;
            } else {
                MPLS_PORT_CHECK(e_port);
                MPLS_MODID_CHECK(e_mod);
                nh_info.port = e_port;
                nh_info.module = e_mod;
            }
            if (si->action == BCM_MPLS_ACTION_SWAP_PUSH) {
                if(!BCM_XGS3_L3_MPLS_LBL_VALID(si->push_label1)) {
                    return (BCM_E_PARAM);
                }
                nh_info.mpls_label = si->push_label1;
            }
            else {
                nh_info.mpls_label = -1; /* Ignore push label */ 
            }
            BCM_IF_ERROR_RETURN(bcm_xgs3_nh_add(unit, 0, &nh_info, &nh_idx));
        }
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, 
                                      NEXT_HOP_INDEXf, nh_idx);
        if ( (si->action == BCM_MPLS_ACTION_SWAP_NEW_DOMAIN) &&
             ( (si->push_label1) || (si->push_label2) ) ) {
            l3intf_entry_t l3_if_entry;
            BCM_IF_ERROR_RETURN
                (READ_L3INTFm(unit, MEM_BLOCK_ANY, intf_index, 
                              &l3_if_entry));
            if (si->push_label1) {
                if(!BCM_XGS3_L3_MPLS_LBL_VALID(si->push_label1)) {
                    return (BCM_E_PARAM);
                }
                soc_L3INTFm_field32_set(unit, &l3_if_entry, 
                                        MPLS_TUNNEL_LABEL1f, si->push_label1);
                soc_L3INTFm_field32_set(unit, &l3_if_entry, 
                                        INSERT_L3_MPLS_LABEL1f, 1);
            }
            if (si->push_label2) {
                if(!BCM_XGS3_L3_MPLS_LBL_VALID(si->push_label2)) {
                    return (BCM_E_PARAM);
                }
                soc_L3INTFm_field32_set(unit, &l3_if_entry, 
                                        MPLS_TUNNEL_LABEL2f, si->push_label2);
                soc_L3INTFm_field32_set(unit, &l3_if_entry, 
                                        INSERT_L3_MPLS_LABEL2f, 1);
            }
            BCM_IF_ERROR_RETURN
                (WRITE_L3INTFm(unit, MEM_BLOCK_ALL, intf_index,
                               &l3_if_entry));
        }
        if ( (si->action == BCM_MPLS_ACTION_PHP) && si->push_label1) {
            l3intf_entry_t l3_if_entry;
            BCM_IF_ERROR_RETURN
                (READ_L3INTFm(unit, MEM_BLOCK_ANY, intf_index, 
                              &l3_if_entry));
            if(!BCM_XGS3_L3_MPLS_LBL_VALID(si->push_label1)) {
                return (BCM_E_PARAM);
            }
            soc_L3INTFm_field32_set(unit, &l3_if_entry, 
                                    MPLS_TUNNEL_LABEL1f, si->push_label1);
            soc_L3INTFm_field32_set(unit, &l3_if_entry, 
                                    INSERT_L3_MPLS_LABEL1f, 1);
            soc_L3INTFm_field32_set(unit, &l3_if_entry, 
                                    INSERT_L3_MPLS_LABEL2f, 0);
            BCM_IF_ERROR_RETURN
                (WRITE_L3INTFm(unit, MEM_BLOCK_ALL, intf_index,
                               &l3_if_entry));
        }
    }
    else if (op & OP_L2) {
	if (si->vpn == 0) {
	    /* Normal MPLS termination, L2 payload */
	    MPLS_MEMPARAM32_CHECK(unit, L3_DEFIP_ALGm, PVIDf, si->dst_vlan);
	    soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, PVIDf, 
					  si->dst_vlan);
	    soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, LEARNf, 
					  FLAG(BCM_MPLS_L2_LEARN));
	    soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, NO_MPLS_DECAPf, 
					  FLAG(BCM_MPLS_NO_MPLS_DECAP));
	    if (FLAG(BCM_MPLS_DST_TRUNK)) {
	        MPLS_TRUNK_CHECK(si->dst_trunk);
	        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, MODIDf, 
				       BCM_TRUNK_TO_MODIDf(unit, si->dst_trunk));
		soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, PORT_TGIDf, 
				       BCM_TRUNK_TO_TGIDf(unit, si->dst_trunk));
	    } else {
	        MPLS_MEMPARAM32_CHECK(unit, L3_DEFIP_ALGm, MODIDf, 
				      si->dst_mod);
	        MPLS_MEMPARAM32_CHECK(unit, L3_DEFIP_ALGm, PORT_TGIDf, 
				 si->dst_port);
	        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, MODIDf, 
					      si->dst_mod);
		soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, PORT_TGIDf, 
					      si->dst_port);
	    }
	    soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, 
					  MPLS_DONOT_CHANGE_INNER_L2f,
					  FLAG(BCM_MPLS_NO_CHANGE_INNER_L2));
	}
	else {
	    /* Special network-to-network port cut-through mode */
	    MPLS_MEMPARAM32_CHECK(unit, L3_DEFIP_ALGm, VFI_IDf, si->vpn);
	    soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, VFI_IDf, si->vpn);
	}
    }

    soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, VALIDf, 1); 

    /* Write lpm_key to L3_DEFIP_ALGm */
    rv = soc_mem_insert(unit, L3_DEFIP_ALGm, COPYNO_ALL, &lpm_key);
    if (BCM_SUCCESS(rv)) {
        /* Delete original entry next hop if any */
        if (found && (orig_nh_idx >= 0)) {
            bcm_xgs3_nh_del(unit, 0, orig_nh_idx);
        }
        if (!found) {
            MPLS_INFO(unit)->mpls_switch_count++;
        }
        return (BCM_E_NONE);
    }

    /*    coverity[check_after_sink]    */
    /* Release our next hop entry, if any */
    if (nh_idx >= 0) {
        bcm_xgs3_nh_del(unit, 0, nh_idx);
    }
    return rv;

}

/*
 * Function:
 *      bcm_mpls_switch_get
 * Purpose:
 *      Get MPLS label swapping and packet forwarding information
 *   1) label swapping, popping at LSR
 *   2) Ethernet MPLS payload forwarding at the VC termination point
 *   3) L3 MPLS payload (IP packet) forwarding at the LSP termination point
 * Parameters:
 *      unit - Device Number
 *      switch_info - (IN) L3 MPLS label switching and packet forwarding info
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_er_mpls_switch_get (int unit, bcm_mpls_switch_t *si)
{
    defip_alg_entry_t      lpm_key, lpm_entry;
    bcm_l3_egress_t        nh_info;
    int                    defip_idx, nh_idx = -1;
    int                    op = 0;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_OLD);

    if (si == NULL) {
        return BCM_E_PARAM;
    }

    /* 
     * All MPLS switches use L3_DEFIP_ALGm. Find our entry. 
     */

    /* Build key for L3_DEFIP_ALGm search */
    sal_memset(&lpm_key, 0, sizeof(defip_alg_entry_t));
    soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key,
                                  FFf, SOC_ER_DEFIP_FF_FIELD);
    if (FLAG(BCM_MPLS_SRC_TRUNK)) {
        MPLS_TRUNK_CHECK(si->src_trunk);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, SRCMODf, 
				      BCM_TRUNK_TO_MODIDf(unit, si->src_trunk));
	soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, SRCPORTf, 
				      BCM_TRUNK_TO_TGIDf(unit, si->src_trunk));
    }
    else {
	MPLS_MEMPARAM32_CHECK(unit, L3_DEFIP_ALGm, SRCMODf, si->src_mod);
	MPLS_MEMPARAM32_CHECK(unit, L3_DEFIP_ALGm, SRCPORTf, si->src_port);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, SRCMODf, 
				      si->src_mod);
	soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, SRCPORTf, 
				      si->src_port);
    }
    MPLS_MEMPARAM32_CHECK(unit, L3_DEFIP_ALGm, MPLS_LABEL1f, si->src_label1);
    soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key,
                                  MPLS_LABEL1f, si->src_label1);
    if (FLAG(BCM_MPLS_ONE_LABEL_LOOKUP)) {
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key,
				      KEY_TYPEf, SOC_ER_DEFIP_KEY_TYPE_MPLS_1L);
    } else {
        MPLS_MEMPARAM32_CHECK(unit, L3_DEFIP_ALGm, MPLS_LABEL2f, 
			      si->src_label2);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key,
				      MPLS_LABEL2f, si->src_label2);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key,
                              KEY_TYPEf, SOC_ER_DEFIP_KEY_TYPE_MPLS_2L);
    }

    BCM_IF_ERROR_RETURN
      (soc_er_defip_lookup(unit, &defip_idx, &lpm_key, &lpm_entry));

    si->action = soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, 
					       MPLS_ACTIONf);
    si->action = bcm_mpls_opcode_to_action(si->action);

    switch (si->action) {
    case BCM_MPLS_ACTION_SWAP:               
    case BCM_MPLS_ACTION_SWAP_PUSH:          
    case BCM_MPLS_ACTION_SWAP_NEW_DOMAIN:    
    case BCM_MPLS_ACTION_PHP:                
        op |= OP_LSR | OP_NEXT_HOP | OP_SWAP;
        break;
    case BCM_MPLS_ACTION_POP_SEARCH:         
        if (!soc_feature(unit, soc_feature_mpls_pop_search)) {
            return BCM_E_UNAVAIL;
        }
        op |= OP_LSR;
        break;
    case BCM_MPLS_ACTION_POP_L2_SWITCH:      
    case BCM_MPLS_ACTION_POP_DST_MOD_PORT:   
        op |= OP_L2;
        break;
    case BCM_MPLS_ACTION_POP_L3_SWITCH:      
        si->vpn = soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, VRF_IDf);
        op |= OP_L3;
        break;
    case BCM_MPLS_ACTION_POP_L3_NEXTHOP:     
        op |= OP_L3 | OP_NEXT_HOP;
        break;
    default:
        return BCM_E_INTERNAL;
    }

    si->flags &= BCM_MPLS_ONE_LABEL_LOOKUP; /* Clear other flag bits */ 

    /* MPLS items common to all actions */
    if (soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, 
				      MPLS_USE_OUTER_TTLf)) {
        si->flags |= BCM_MPLS_USE_OUTER_TTL;
    }
    if (soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, 
				      MPLS_USE_OUTER_EXPf)) {
        si->flags |= BCM_MPLS_USE_OUTER_EXP;
    }
    if (soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, 
				      MPLS_DONT_CHANGE_INNER_EXPf)) {
        si->flags |= BCM_MPLS_NO_CHANGE_INNER_EXP;
    }
    if (soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, MPLS_USE_PRIf)) {
        si->flags |= BCM_MPLS_USE_PRI;
	si->label_pri = soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, 
						      MPLS_LABEL_PRIf);
    }
    si->src_l3_intf = soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, 
						    MPLS_IINTFf);

    if (op & OP_NEXT_HOP) {
        int          e_trunk_flag=0;
        int          *e_trunk;
        int          *e_mod;
        int          *e_port;
        int          egress_mode;

        if (op & OP_SWAP) {
            si->swap_label = soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, 
                                                           MPLS_NEW_LABELf); 
        }

        /* Get the NEXT_HOP_INDEX, then get that entry's information */
        nh_idx = soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, NEXT_HOP_INDEXf);
        BCM_IF_ERROR_RETURN(bcm_xgs3_nh_get(unit, nh_idx, &nh_info));

        if (op & OP_LSR) {
            e_trunk_flag = BCM_MPLS_NEXT_TRUNK;
            e_trunk = &si->next_trunk;
            e_mod = &si->next_mod;
            e_port = &si->next_port;
        }
        else {
            e_trunk_flag = BCM_MPLS_DST_TRUNK;
            e_trunk = &si->dst_trunk;
            e_mod = &si->dst_mod;
            e_port = &si->dst_port;
        }

        BCM_IF_ERROR_RETURN(bcm_xgs3_l3_egress_mode_get(unit, &egress_mode)); 
        if (egress_mode) {
            si->next_intf = nh_idx + BCM_XGS3_EGRESS_IDX_MIN;
        } else {
            sal_memcpy(si->next_mac, nh_info.mac_addr, sizeof(bcm_mac_t)); 
            si->next_intf = nh_info.intf;
            if (nh_info.flags & BCM_L3_TGID) {
                si->flags |= e_trunk_flag;
                *e_trunk = nh_info.trunk;
            } else {
                *e_port = nh_info.port; 
                *e_mod = nh_info.module;
            }
            if (si->action == BCM_MPLS_ACTION_SWAP_PUSH) {
                si->push_label1 = nh_info.mpls_label;
            }
        }
    } else if (op & OP_L2) {
        si->vpn = soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, VFI_IDf);
        if (si->vpn == 0) {
	    int tgid;
	    /* Normal MPLS termination, L2 payload */
	    si->dst_vlan = soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, 
							 PVIDf);
	    if (soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, LEARNf)) {
	        si->flags |= BCM_MPLS_L2_LEARN;
	    }
	    if (soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, 
					      NO_MPLS_DECAPf)) {
	        si->flags |= BCM_MPLS_NO_MPLS_DECAP;
	    }
	    tgid = soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, 
						 PORT_TGIDf);
	    if (tgid & BCM_TGID_TRUNK_INDICATOR(unit)) {
	        int modid = soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, 
							  MODIDf);
	        si->flags |= BCM_MPLS_DST_TRUNK;
		si->dst_trunk = BCM_MODIDf_TGIDf_TO_TRUNK(unit, modid, tgid);
	    } else {
	        si->dst_mod = soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, 
							    MODIDf); 
		si->dst_port = tgid;
	    }
	    if (soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, 
					      MPLS_DONOT_CHANGE_INNER_L2f)) {
	        si->flags |= BCM_MPLS_NO_CHANGE_INNER_L2;
	    }
	}
    }


    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_switch_delete
 * Purpose:
 *      Delete MPLS label swapping and packet forwarding information
 *   1) label swapping, popping at LSR
 *   2) Ethernet MPLS payload forwarding at the VC termination point
 *   3) L3 MPLS payload (IP packet) forwarding at the LSP termination point
 * Parameters:
 *      unit - Device Number
 *      switch_info - (IN) L3 MPLS label switching and packet forwarding info
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_er_mpls_switch_delete (int unit, bcm_mpls_switch_t *switch_info)
{
    defip_alg_entry_t lpm_key, lpm_entry;
    int index, nh_index;
    int rv = 0;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_OLD);

    if (switch_info == NULL) {
        return BCM_E_PARAM;
    }

    if (MPLS_INFO(unit)->mpls_switch_count == 0) {
        return BCM_E_NONE;
    }

    sal_memset(&lpm_key, 0, sizeof(defip_alg_entry_t));
    soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key,
                                  FFf, SOC_ER_DEFIP_FF_FIELD);
    if (switch_info->flags & BCM_MPLS_SRC_TRUNK) {
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, SRCMODf, 
			 BCM_TRUNK_TO_MODIDf(unit, switch_info->src_trunk));
	soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, SRCPORTf, 
			 BCM_TRUNK_TO_TGIDf(unit, switch_info->src_trunk));
    }
    else {
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, SRCMODf, 
				      switch_info->src_mod);
	soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key, SRCPORTf, 
				      switch_info->src_port);
    }
    soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key,
                                  MPLS_LABEL1f, switch_info->src_label1);
    if (switch_info->flags & BCM_MPLS_ONE_LABEL_LOOKUP) {
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key,
                              KEY_TYPEf, SOC_ER_DEFIP_KEY_TYPE_MPLS_1L);
    } else {
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key,
                                  MPLS_LABEL2f, switch_info->src_label2);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_key,
                              KEY_TYPEf, SOC_ER_DEFIP_KEY_TYPE_MPLS_2L);
    }

    rv = soc_er_defip_lookup(unit, &index, &lpm_key, &lpm_entry);
    if (rv < 0) {
        return rv;
    }

    nh_index = soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, NEXT_HOP_INDEXf);

    bcm_xgs3_nh_del(unit, 0, nh_index);

    rv = soc_er_defip_delete(unit, &lpm_key);

    MPLS_INFO(unit)->mpls_switch_count--;

    return rv;
}

/*
 * Test callback function for MPLS switch delete all
 */
STATIC int
_bcm_er_mpls_del_all_testcb(int unit, defip_alg_entry_t *lpm_entry,
                            int ent_idx, int *test, void *cookie)
{
    int key_type = 
      soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, KEY_TYPEf);

    if (!soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, VALIDf) ||
        soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, FFf) != 0xff ||
        ( (key_type != SOC_ER_DEFIP_KEY_TYPE_MPLS_1L) &&
	  (key_type != SOC_ER_DEFIP_KEY_TYPE_MPLS_2L) ) ) {
        *test = FALSE;
    } else {
        *test = TRUE;
    }

    return BCM_E_NONE;
}

/*
 * Operational callback function for IPv4/IPv6 LPM delete all
 */
STATIC int
_bcm_er_mpls_del_all_opcb(int unit, defip_alg_entry_t *lpm_entry,
                          int ent_idx, int *test, void *cookie)
{
    int nh_idx, action;

    action = bcm_mpls_opcode_to_action
                (soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, MPLS_ACTIONf));

    if (action == BCM_MPLS_OPCODE_SWAP ||
        action == BCM_MPLS_OPCODE_SWAP_PUSH ||
        action == BCM_MPLS_OPCODE_SWAP_NEW_DOMAIN ||
        action == BCM_MPLS_OPCODE_PHP ||
        action == BCM_MPLS_OPCODE_POP_L3_NEXTHOP) {
        nh_idx = soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, NEXT_HOP_INDEXf);
        bcm_xgs3_nh_del(unit, 0, nh_idx);
    }

    return soc_er_defip_delete(unit, lpm_entry);
}

/*
 * Function:
 *      bcm_mpls_switch_delete_all
 * Purpose:
 *      Delete all MPLS label swapping and packet forwarding information
 * Parameters: 
 *      unit - Device Number 
 *      flag - BCM_MPLS_LSR_SWITCH delete LSR action entries
 *             BCM_MPLS_L2_VPN     delete L2 VC termination action entries
 *             BCM_MPLS_L3_VPN     delete L3 LSP termination action entries
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_er_mpls_switch_delete_all (int unit, int flag)
{
    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_OLD);

    MPLS_SWITCH_LOCK(unit);
    soc_er_defip_traverse(unit, _bcm_er_mpls_del_all_testcb,
                          _bcm_er_mpls_del_all_opcb, NULL, NULL);
    MPLS_SWITCH_UNLOCK(unit);

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_switch_traverse
 * Purpose:
 *      Find MPLS switch entries from the LPM table by traversing the LPM table
 *      and run the function at each MPLS entry
 * Parameters:
 *      unit - SOC unit #
 *      flags - User supplied flags
 *      trav_fn - User callback function, called once per MPLS entry.
 *      start - Start index of interest.
 *      end   - End index of interest.
 *      user_data - cookie
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_mpls_switch_traverse(int unit, uint32 flags, 
          uint32 start, uint32 end,
          bcm_mpls_switch_traverse_cb trav_fn, void *user_data)
{
    defip_alg_entry_t lpm_entry;
    bcm_mpls_switch_t switch_info;
    bcm_l3_egress_t nh_info;
    int nh_idx;
    int index;
    int key_type;
    int egress_mode;
    int rv = BCM_E_NONE;
    uint32 total;

    MPLS_INIT(unit);
    MPLS_API_STYLE(unit, _BCM_ER_MPLS_API_STYLE_OLD);

    if (!trav_fn) {
        return BCM_E_NONE;
    }

    

    total = 0;
    index = -1;

    MPLS_SWITCH_LOCK(unit);
    while (1) {
        if ((rv = soc_er_defip_iterate(unit, &index, &lpm_entry)) < 0 ||
            index == -1) {   /* End of iteration */
            break;
        }

        key_type = soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, KEY_TYPEf);
        if ( soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, FFf) != 0xff ||
             ( (key_type != SOC_ER_DEFIP_KEY_TYPE_MPLS_1L) &&
               (key_type != SOC_ER_DEFIP_KEY_TYPE_MPLS_2L) ) ) {
            continue;
        }

        total++;
        if (total < start) {
            continue;
        }

        /* This is a MPLS switch entry */
        bcm_mpls_switch_t_init(&switch_info);
        if (soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, KEY_TYPEf) ==
            SOC_ER_DEFIP_KEY_TYPE_MPLS_1L) {
            switch_info.flags = BCM_MPLS_ONE_LABEL_LOOKUP;
        } else {
            switch_info.src_label2 = 
                soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, MPLS_LABEL2f);
        }
        switch_info.src_label1 = 
            soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, MPLS_LABEL1f);
        switch_info.src_mod = 
            soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, SRCMODf);
        switch_info.src_port = 
            soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, SRCPORTf);
        if (switch_info.src_port & BCM_TGID_TRUNK_INDICATOR(unit)) {
            switch_info.flags |= BCM_MPLS_SRC_TRUNK;
            switch_info.src_trunk = 
                BCM_MODIDf_TGIDf_TO_TRUNK(unit, switch_info.src_mod, switch_info.src_port);
            switch_info.src_mod = switch_info.src_port = 0;
        }

        switch_info.action = bcm_mpls_opcode_to_action
            (soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, MPLS_ACTIONf));
        switch_info.src_l3_intf = 
            soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, MPLS_IINTFf);
        switch_info.label_pri = 
            soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, MPLS_LABEL_PRIf);

        if (soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, MPLS_USE_OUTER_TTLf)) {
            switch_info.flags |= BCM_MPLS_USE_OUTER_TTL;
        } else if (soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry,
                                                 MPLS_USE_OUTER_EXPf)) {
            switch_info.flags |= BCM_MPLS_USE_OUTER_EXP;
        } else if (soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry,
                                                 MPLS_DONT_CHANGE_INNER_EXPf)) {
            switch_info.flags |= BCM_MPLS_NO_CHANGE_INNER_EXP;
        } else if (soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, MPLS_USE_PRIf)) {
            switch_info.flags |= BCM_MPLS_USE_PRI;
        }

        switch (switch_info.action) {

            /* MPLS label swapping & fwding to nexthop (actions 0, 1, 2, 3, 9) */
          case BCM_MPLS_ACTION_SWAP:
          case BCM_MPLS_ACTION_SWAP_PUSH:
          case BCM_MPLS_ACTION_SWAP_NEW_DOMAIN:
          case BCM_MPLS_ACTION_PHP:
          case BCM_MPLS_ACTION_POP_SEARCH:
              switch_info.swap_label =
                  soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, MPLS_NEW_LABELf);

              nh_idx = soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry,
                                                     NEXT_HOP_INDEXf);
              if ((rv = bcm_xgs3_nh_get(unit, nh_idx, &nh_info)) < 0) {
                  break;
              }
              if ((rv = bcm_xgs3_l3_egress_mode_get(unit,
                                                    &egress_mode))< 0) {
                  break;
              } 
              if (egress_mode) {
                  switch_info.next_intf = (nh_idx + BCM_XGS3_EGRESS_IDX_MIN);
              } else {
                  switch_info.next_mod  = nh_info.module;
                  switch_info.next_port = 
                      (nh_info.flags & BCM_L3_TGID) ? nh_info.trunk : nh_info.port;
                  switch_info.next_intf = nh_info.intf;
                  sal_memcpy(switch_info.next_mac, nh_info.mac_addr, 6);
                  switch_info.next_vlan =
                      soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, PVIDf);
                  if (switch_info.action == BCM_MPLS_ACTION_SWAP_PUSH) {
                      switch_info.push_label1 = nh_info.mpls_label;
                  } else if (switch_info.action == BCM_MPLS_ACTION_SWAP_NEW_DOMAIN) {
                      
                  }
              }
              break;

              /* MPLS L2 payload forwarding related (actions 5, 6) */
          case BCM_MPLS_ACTION_POP_L2_SWITCH:
          case BCM_MPLS_ACTION_POP_DST_MOD_PORT:
              switch_info.dst_port =
                  soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, PORT_TGIDf);
              switch_info.dst_mod = 
                  soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, MODIDf);
              switch_info.dst_vlan = 
                  soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, PVIDf);

              if (soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry,
                                                MPLS_DONOT_CHANGE_INNER_L2f)) {
                  switch_info.flags |= BCM_MPLS_NO_CHANGE_INNER_L2;
              } else if (soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry,
                                                       NO_MPLS_DECAPf)) {
                  switch_info.flags |= BCM_MPLS_NO_MPLS_DECAP;
              }
              switch_info.vpn =soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, VFI_IDf);
              break;

              /* L3 MPLS payload forwarding (actions 7, 8) */
          case BCM_MPLS_ACTION_POP_L3_SWITCH:
          case BCM_MPLS_ACTION_POP_L3_NEXTHOP:
              switch_info.vpn =soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, VRF_IDf);
              break;

          default:
              break;
        }

        if (BCM_FAILURE(rv)) {
            break;
        }
        rv = trav_fn(unit, total, &switch_info, user_data);
        if (BCM_FAILURE(rv)) {
            break;
        }
        if (total > end) {
            rv =  BCM_E_NONE;
            break;
        }
    }

    MPLS_SWITCH_UNLOCK(unit);
    return rv;
}
#else /* BCM_EASYRIDER_SUPPORT && BCM_MPLS_SUPPORT && INCLUDE_L3 */
int bcm_er_mpls_not_empty;
#endif  /* BCM_EASYRIDER_SUPPORT && BCM_MPLS_SUPPORT && INCLUDE_L3 */
