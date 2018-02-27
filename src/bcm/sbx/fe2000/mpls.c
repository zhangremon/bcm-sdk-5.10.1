/*
 * $Id: mpls.c 1.219.6.3 Broadcom SDK $
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


#if defined(INCLUDE_L3)

#include <bcm/types.h>
#include <bcm/error.h>

#include <soc/sbx/sbx_drv.h>

#include <shared/gport.h>
#include <bcm/debug.h>
#include <bcm/l2.h>
#include <bcm/l3.h>
#include <bcm/ipmc.h>
#include <bcm/tunnel.h>
#include <bcm/stack.h>
#include <bcm/cosq.h>
#include <bcm/mpls.h>
#include <bcm/trunk.h>
#include <bcm_int/sbx/fe2000/trunk.h>
#include <bcm/vlan.h>
#include <bcm/pkt.h>
#include <bcm/policer.h>
#include <bcm_int/sbx/fe2000/g2p3.h>

#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/fe2000/allocator.h>
#include <bcm_int/sbx/fe2000/recovery.h>
#include <bcm_int/sbx/fe2000/l3.h>
#include <bcm_int/sbx/stat.h>
#include <bcm_int/sbx/fe2000/vlan.h>
#include <bcm_int/sbx/fe2000/policer.h>
#include <bcm_int/sbx/stack.h>
#include <bcm_int/sbx/fe2000/mpls.h>
#include <shared/hash_tbl.h>



/* ulVplsColor is 44:38 have a mapping for _NETWORK flag
 * until the user managed vpls-color is enabled in the
 * mpls-port-parameters
 */
#define _FE2K_VPLS_COLOR_WAN  0x3F
#define _FE2K_VPLS_COLOR_LAN  0x00

#define _FE2K_G2P3_MAX_LABELS  (64 * 1024)

#define _BCM_MPLS_SWITCH_SUPPORTED_FLAGS             \
    (BCM_MPLS_SWITCH_OUTER_TTL      |                \
     BCM_MPLS_SWITCH_INNER_TTL      |                \
     BCM_MPLS_SWITCH_TTL_DECREMENT  |                \
     BCM_MPLS_SWITCH_INT_PRI_MAP    |                \
     BCM_MPLS_SWITCH_INT_PRI_SET    |                \
     BCM_MPLS_SWITCH_INNER_EXP      |                \
     BCM_MPLS_SWITCH_DROP           |                \
     BCM_MPLS_SWITCH_OUTER_EXP)

/**
 * BCM_MPLS_PORT_DROP
 *   There is no way to drop pkts on per vpn-sap basis
 *   for p2etc, pvid2etc ...
 *
 * BCM_MPLS_PORT_SEQUENCED
 * BCM_MPLS_PORT_CONTROL_WORD
 *
 * BCM_MPLS_PORT_INT_PRI_SET
 * BCM_MPLS_PORT_INT_PRI_MAP
 * BCM_MPLS_PORT_COLOR_MAP
 *   cos related work-item
 *
 */
#define _BCM_MPLS_PORT_SUPPORTED_FLAGS          \
    (BCM_MPLS_PORT_REPLACE                  |   \
     BCM_MPLS_PORT_WITH_ID                  |   \
     BCM_MPLS_PORT_NETWORK                  |   \
     BCM_MPLS_PORT_INT_PRI_MAP              |   \
     BCM_MPLS_PORT_INT_PRI_SET              |   \
     BCM_MPLS_PORT_EGRESS_TUNNEL            |   \
     BCM_MPLS_PORT_NO_EGRESS_TUNNEL_ENCAP   |   \
     BCM_MPLS_PORT_SERVICE_TAGGED           |   \
     BCM_MPLS_PORT_SERVICE_VLAN_ADD         |   \
     BCM_MPLS_PORT_SERVICE_VLAN_REPLACE     |   \
     BCM_MPLS_PORT_SERVICE_VLAN_DELETE      |   \
     BCM_MPLS_PORT_INNER_VLAN_PRESERVE      |   \
     BCM_MPLS_PORT_ENCAP_WITH_ID            |   \
     BCM_MPLS_PORT_EGRESS_UNTAGGED          |   \
     BCM_MPLS_PORT_SERVICE_VLAN_TPID_REPLACE |  \
     BCM_MPLS_PORT_CONTROL_WORD             |   \
     BCM_MPLS_PORT_DROP                     |   \
     BCM_MPLS_PORT_FAILOVER                 |   \
     BCM_MPLS_PORT_INNER_VLAN_ADD)

#define G2_FE_HANDLER_GET(unit, fe)                     \
    fe = ((sbG2Fe_t *)SOC_SBX_CONTROL(unit)->drv)

#define _FE2K_IS_MPLS_INITIALIZED(_l3_fe)               \
    ((_l3_fe)->fe_flags & _FE2K_L3_FE_FLG_MPLS_INIT)

#define _FE2K_SET_MPLS_INITIALIZED(_l3_fe)               \
    ((_l3_fe)->fe_flags |= _FE2K_L3_FE_FLG_MPLS_INIT)

#define _FE2K_SET_MPLS_UNINITIALIZED(_l3_fe)               \
    ((_l3_fe)->fe_flags &= ~_FE2K_L3_FE_FLG_MPLS_INIT)

#define _FE2K_MPLS_GET_FE(x)  ((sbG2Fe_t *)SOC_SBX_CONTROL((x)->fe_unit)->drv)

/* FE2k g2p3 supports 16bits of mpls label for vsi matching.  However, the
 * MSb signals to the PPE that the label is a PWE3 label
 */
#define _BCM_LABEL_VALID(_label_) \
  ((_label_ & ~_FEK2_L3_MPLS_PWE3_LBL_ID) < _FE2K_L3_MPLS_LBL_MAX)

#define _BCM_FE2000_MPLSTP_LABEL_VALID(_label_)\
  ((_label_) < _FE2K_L3_MPLSTP_LBL_MASK)

#define _BCM_FE2000_IS_PWE3_VPN(flag) (((flag) & BCM_MPLS_VPN_VPWS) || \
                                       ((flag) & BCM_MPLS_VPN_VPLS))

#define _BCM_FE2000_IS_PWE3_TUNNEL(flag) (((flag) & BCM_MPLS_PORT_EGRESS_TUNNEL) || \
                                          ((flag) & BCM_MPLS_PORT_NO_EGRESS_TUNNEL_ENCAP))

/*
 * Debug Output Macros
 */
#define MPLS_DEBUG(flags, stuff)  BCM_DEBUG(flags | BCM_DBG_MPLS, stuff)
#define MPLS_OUT(stuff)           MPLS_DEBUG(BCM_DBG_PORT, stuff)
#define MPLS_WARN(stuff)          MPLS_DEBUG(BCM_DBG_WARN, stuff)
#define MPLS_ERR(stuff)           MPLS_DEBUG(BCM_DBG_ERR, stuff)
#define MPLS_VERB(stuff)          MPLS_DEBUG(BCM_DBG_VERBOSE, stuff)
#define MPLS_VVERB(stuff)         MPLS_DEBUG(BCM_DBG_VVERBOSE, stuff)

#define MPLS_MAX_GPORT (8192) 

#define SB_COMMIT_COMPLETE   0xffffffff

#define _MPLSTP_ILM_FT_SHIFT (14)

/* Gport Database */
STATIC shr_htb_hash_table_t mpls_gport_db[BCM_MAX_NUM_UNITS];

/* Tunnel Switch Info data base */
STATIC shr_htb_hash_table_t mpls_switch_info_db[BCM_MAX_NUM_UNITS];

bcm_fe2k_mpls_trunk_association_t 
mpls_trunk_assoc_info[BCM_MAX_NUM_UNITS][SBX_MAX_TRUNKS];

uint32 _sbx_mplstp_lbl_opcode[BCM_MAX_NUM_UNITS][MAX_LBL_OPCODE];

/**
 * Static function declarations.
 */

STATIC int
_bcm_fe2000_mpls_trunk_cb(int unit, 
                         bcm_trunk_t tid, 
                         bcm_trunk_add_info_t *tdata,
                         void *user_data);

STATIC int
_bcm_fe2000_invalidate_vpn_sap_hw_resources(_fe2k_l3_fe_instance_t *l3_fe,
                                            _fe2k_vpn_sap_t    *vpn_sap);
STATIC int
_bcm_fe2000_mpls_get_fte(_fe2k_l3_fe_instance_t *l3_fe,
                         uint32                  fte_idx,
                         int                     action,
                         _fe2k_l3_or_mpls_egress_t *egr);

STATIC int
_bcm_fe2000_link_vpn_sap_ufte(_fe2k_l3_fe_instance_t  *l3_fe,
                              _fe2k_vpn_sap_t         *vpn_sap,
                              bcm_module_t             caller_module);
STATIC int
_bcm_fe2000_update_vpn_sap_hw(_fe2k_l3_fe_instance_t  *l3_fe,
                              bcm_module_t             caller_module,
                              _fe2k_vpn_sap_t         *vpn_sap,
                              bcm_mpls_port_t         *mpls_port);
STATIC int
_bcm_fe2000_mpls_port_delete(int                      unit,
                             _fe2k_l3_fe_instance_t  *l3_fe,
                             _fe2k_vpn_control_t     *vpnc,
                             _fe2k_vpn_sap_t        **vpn_sap);
STATIC int
_bcm_fe2000_fill_mpls_vpn_config(int                     unit,
                                 _fe2k_vpn_control_t    *vpnc,
                                 bcm_mpls_vpn_config_t  *info);

STATIC int
_bcm_fe2000_alloc_mpls_vpn_sap(_fe2k_l3_fe_instance_t  *l3_fe,
                               _fe2k_vpn_control_t   *vpnc,
                               bcm_gport_t            mpls_port_id,
                               _fe2k_vpn_sap_t      **vpn_sap);


STATIC int
_bcm_fe2000_free_mpls_vpn_sap(_fe2k_l3_fe_instance_t  *l3_fe,
                              _fe2k_vpn_sap_t      **vpn_sap);

int
bcm_fe2000_mpls_vpn_id_create(int                    unit,
                              bcm_mpls_vpn_config_t *info);
int
bcm_fe2000_mpls_port_add(int               unit,
                         bcm_vpn_t         vpn,
                         bcm_mpls_port_t  *mpls_port);

#ifdef BCM_WARM_BOOT_SUPPORT
int _bcm_fe2000_mpls_vpws_wb_set(_fe2k_l3_fe_instance_t *l3_fe,
                                 _fe2k_vpn_sap_t *vpn_sap);
int _bcm_fe2000_mpls_vpws_wb_get(_fe2k_l3_fe_instance_t *l3_fe,
                                 uint32           vsi,
                                 uint32          *vpn_id);
#endif


/*
 * Function:
 *      _bcm_fe2000_fill_mpls_label_array_from_ete_idx
 * Purpose:
 *      fill label array based on the ete-idx
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      ete_idx     - h/w mpls ete index
 *      label_max   - label_array size
 *      label_array - egress label array
 *      label_count - filled number of entries
 * Returns:
 *      BCM_E_XXX
 * Note:
 *
 */
int
_bcm_fe2000_fill_mpls_label_array_from_ete_idx(_fe2k_l3_fe_instance_t *l3_fe,
                                               uint32                   ete_idx,
                                               int                      label_max,
                                               bcm_mpls_egress_label_t *label_array,
                                               int                     *label_count)
{
    int                      rv;
    soc_sbx_g2p3_eteencap_t  encap_ete;

    *label_count = 0;
    rv = soc_sbx_g2p3_eteencap_get(l3_fe->fe_unit, ete_idx, &encap_ete);
    if (BCM_FAILURE(rv)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error reading ete encap(0x%x): %s\n"),
                  ete_idx, bcm_errmsg(rv)));
        return rv;
    }
    
#define BCM_FE2K_G2P3_LABEL_ADD(_label_num_)  \
    if (encap_ete.label##_label_num_ && (*label_count < label_max)) {   \
        label_array->label  = encap_ete.label##_label_num_;             \
        label_array->exp    = encap_ete.exp##_label_num_;               \
        label_array->ttl    = encap_ete.ttl##_label_num_;               \
        label_array->flags  = 0;                                        \
        if (encap_ete.exp##_label_num_##remark) {                       \
            label_array->flags |= BCM_MPLS_EGRESS_LABEL_EXP_REMARK;     \
        } else {                                                        \
            label_array->flags |= BCM_MPLS_EGRESS_LABEL_EXP_SET;        \
        }                                                               \
                                                                        \
        if (encap_ete.ttl##_label_num_##dec) {                          \
            label_array->flags |= BCM_MPLS_EGRESS_LABEL_TTL_DECREMENT;  \
        } else {                                                        \
            label_array->flags |= BCM_MPLS_EGRESS_LABEL_TTL_SET;        \
        }                                                               \
                                                                        \
        MPLS_VERB((_SBX_D(l3_fe->fe_unit,                               \
                          "Added label%d at %d:0x%x\n"),                \
                   _label_num_, *label_count, label_array->label));     \
        label_array++;                                                  \
        (*label_count)++;                                               \
    }

    BCM_FE2K_G2P3_LABEL_ADD(0);
    BCM_FE2K_G2P3_LABEL_ADD(1);
    BCM_FE2K_G2P3_LABEL_ADD(2);

#undef BCM_FE2K_G2P3_LABEL_ADD

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_fe2000_update_vpn_sap_hw
 * Purpose:
 *      allocate if reqd, map and set ohi, ete for vpn sap
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      caller_module - modid of remote caller or fe_my_modid
 *      vpn_sap     -  vpn service attachment point
 *      mpls_port   -  bcm provided port info
 * Returns:
 *      BCM_E_XXX
 * Note:
 *
 */

STATIC int
_bcm_fe2000_update_vpn_sap_hw(_fe2k_l3_fe_instance_t  *l3_fe,
                              bcm_module_t             caller_module,
                              _fe2k_vpn_sap_t         *vpn_sap,
                              bcm_mpls_port_t         *mpls_port)
{
    int status = 0, ignore_status;

    MPLS_VVERB((_SBX_D(l3_fe->fe_unit, "Enter\n")));
    vpn_sap->vc_res_alloced = 0;

    /**
     * XXX: TBD: How will back 2 back router with PHP support work with
     * VC labels ?
     * (8/2/07)
     * new design for mpls-ete via mpls-switch-add,
     * see if that solves this
     * ucode reqd for having vsi <--> fte
     *
     */

    if (_BCM_FE2000_IS_PWE3_TUNNEL(mpls_port->flags)) {

        /**
         * This routine programs the P side for VPLS and VPWS
         * i.e. the vc_label over tunnel.
         */

        switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
        {
            
        case SOC_SBX_UCODE_TYPE_G2P3:
            status = _bcm_fe2000_g2p3_mpls_update_vpxs_tunnel_hw(l3_fe,
                                                                 vpn_sap,
                                                                 mpls_port);
            break;

        default:
            SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
            status = BCM_E_INTERNAL;
            break;
        }

        if (BCM_FAILURE(status)) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) updating ucode tables\n"),
                      bcm_errmsg(status)));
            goto error_out;
        }

    } else {

        /**
         * (i)  LAN side of VPLS/VPWS
         * (ii) L1 port
         */
        switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
        {
        case SOC_SBX_UCODE_TYPE_G2P3:
            status = _bcm_fe2000_g2p3_mpls_update_vpxs_hw(l3_fe,
                                                          vpn_sap,
                                                          mpls_port);
            break;

        default:
            SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
            status = BCM_E_INTERNAL;
            break;
        }

        if (status != BCM_E_NONE) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) updating ucode tables\n"),
                      bcm_errmsg(status)));
            goto error_out;
        }

    }

    status = _bcm_fe2000_link_vpn_sap_ufte(l3_fe,
                                           vpn_sap,
                                           caller_module);
    if (status != BCM_E_NONE) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) linking vpn-sap ufte\n"),
                  bcm_errmsg(status)));
        goto error_out;
    }

    return BCM_E_NONE;

error_out:

    if (vpn_sap->vc_res_alloced) {
        ignore_status =
            _bcm_fe2000_invalidate_vpn_sap_hw_resources(l3_fe,
                                                        vpn_sap);
    }

    return status;
}

/*
 * Function:
 *      _bcm_fe2000_mpls_tunnel_initiator_set
 * Purpose:
 *      Set the MPLS tunnel ete (add/update)
 * Parameters:
 *      l3_fe       - (IN)  l3 fe instance
 *      l3_intf     - (IN)  The egress L3 interface context
 *      l3_ete      - (IN)  l3 ete for which the tunnel ete is being made
 *      num_labels  - (IN)  Number of labels in the array
 *      label_array - (IN)  Array of MPLS label and header information
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_fe2000_mpls_tunnel_initiator_set(_fe2k_l3_fe_instance_t  *l3_fe,
                                      _fe2k_l3_intf_t         *l3_intf,
                                      _fe2k_l3_ete_t          *l3_ete, /* may be NULL */
                                      int                      num_labels,
                                      bcm_mpls_egress_label_t *label_array)
{
    _fe2k_l3_ete_t            *mpls_sw_ete;
    int                        status, ignore_status;
    _fe2k_l3_ete_key_t         tkey;
    int                        ete_allocated;
    bcm_mac_t                  mac_addr;
    int                        i;

    MPLS_VVERB((_SBX_D(l3_fe->fe_unit, "Enter\n")));

    /*
     * Note that the v4-ete may not exist (i.e. the l3_ete parameter may be NULL)
     */
    if (l3_ete == NULL) {
        MPLS_VERB((_SBX_D(l3_fe->fe_unit, "Mpls tunnel initiator set called on interface (0x%x), "
                          " but no v4 ete present\n"),
                   _FE2K_USER_HANDLE_FROM_IFID(l3_intf->if_info.l3a_intf_id)));
    }

    mpls_sw_ete   = NULL;
    ete_allocated = 0;
    sal_memset(mac_addr, 0, sizeof(bcm_mac_t)); /* dummy mac if v4-ete is not there */

    /*
     * first, see if a mpls ete exists. i.e. figure out the add/update case
     */
    status   = _bcm_fe2000_get_ete_by_type_on_intf(l3_fe, l3_intf,
                                                   _FE2K_L3_ETE__ENCAP_MPLS,
                                                   &mpls_sw_ete);
    if (status == BCM_E_NOT_FOUND) {
        /*
         * 1. mpls ete does not exists => Add case: But there are 2 sub cases.
         *    case a: v4 ete exists
         *    case b: v4 ete does not exist - In this case we will use
         *            all-zero mac address and a NOP for vidop.
         *            When the v4-ete is created, the modify function
         *            will do the fix-up.
         */
        if (l3_ete != NULL) {
            _FE2K_MAKE_ENCAP_MPLS_SW_ETE_KEY(&tkey,
                                             l3_ete->l3_ete_key.l3_ete_hk.dmac,
                                             l3_ete->l3_ete_key.l3_ete_vidop);
        } else {
            _FE2K_MAKE_ENCAP_MPLS_SW_ETE_KEY(&tkey,
                                             mac_addr,
                                             _FE2K_L3_ETE_VIDOP__NOP);
        }

        status = _bcm_fe2000_alloc_default_l3_ete(l3_fe, l3_intf, &tkey,
                                                  0,        /* OHI */
                                                  &mpls_sw_ete);
        if (status != BCM_E_NONE) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) could not allocate MPLS encap ete on "
                             "interface (0x%x)\n"),
                      bcm_errmsg(status),
                      _FE2K_USER_HANDLE_FROM_IFID(l3_intf->if_info.l3a_intf_id)));
            return status;
        }

        ete_allocated  = 1;
        DQ_INIT(&mpls_sw_ete->l3_vc_ete_head);
    }
    /* 
     * Tunnel Ingress Drop
     * If any of the labels in the array has DROP set, flag the interface
     * Clear existing flag, set again if required
     */
    l3_intf->if_flags &= ~_FE2K_L3_MPLS_TUNNEL_DROP;
    for (i=0; i< num_labels; i++) {
        if (label_array[i].flags & BCM_MPLS_EGRESS_LABEL_DROP) {
            l3_intf->if_flags |= _FE2K_L3_MPLS_TUNNEL_DROP;
        }
    }

    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {
    case SOC_SBX_UCODE_TYPE_G2P3:
        status = _bcm_fe2000_g2p3_map_set_l2_ete_for_mpls(l3_fe,
                                                          l3_intf,
                                                          mpls_sw_ete);
        
        break;

    default:
        SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
        status = BCM_E_INTERNAL;
        break;
    }
    
    if (status != BCM_E_NONE) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) map-set l2-ete(0x%x) for mpls\n"),
                  bcm_errmsg(status),
                  (unsigned int)(mpls_sw_ete->l3_l2encap.ete_idx.ete_idx)));
        if (ete_allocated) {
            ignore_status = _bcm_fe2000_undo_l3_ete_alloc(l3_fe,
                                                          l3_intf,
                                                          &mpls_sw_ete);
        }
        return status;
    }

    switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
    case SOC_SBX_UCODE_TYPE_G2P3:
        return (_bcm_fe2000_g2p3_mpls_tunnel_ete_set(l3_fe->fe_unit,
                                                     l3_fe,
                                                     l3_intf,
                                                     l3_ete,
                                                     num_labels,
                                                     label_array,
                                                     mpls_sw_ete,
                                                     ete_allocated));

   default:
       SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
       return BCM_E_CONFIG;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_fe2000_tunnel_initiator_clear
 * Purpose:
 *       to remove the mpls tunnel ete from the l3_intf
 * Parameters:
 *      l3_fe       - (IN)  l3 fe instance
 *      l3_intf     - (IN)  The egress L3 interface context
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_fe2000_mpls_l3_initiator_clear(_fe2k_l3_fe_instance_t  *l3_fe,
                                    _fe2k_l3_intf_t         *l3_intf)
{
    int                       rv;
    _fe2k_l3_ete_t           *mpls_sw_ete;
    _fe2k_l3_ete_t           *l3_ete;
    bcm_if_t                  ifid;

    ifid = _FE2K_USER_HANDLE_FROM_IFID(l3_intf->if_info.l3a_intf_id);
    MPLS_VVERB((_SBX_D(l3_fe->fe_unit, "Enter\n")));

    /* validate the interface deleting */
    mpls_sw_ete = NULL;
    rv =_bcm_fe2000_get_ete_by_type_on_intf(l3_fe, l3_intf,
                                            _FE2K_L3_ETE__ENCAP_MPLS,
                                            &mpls_sw_ete);

    if (BCM_FAILURE(rv)) {
        MPLS_WARN((_SBX_D(l3_fe->fe_unit, "mpls tunnel does not exist on "
                          "interface id (0x%x): %s\n"),
                   ifid, bcm_errmsg(rv)));
        return rv;
    }

    if (!DQ_EMPTY(&mpls_sw_ete->l3_vc_ete_head)) {
        MPLS_WARN((_SBX_D(l3_fe->fe_unit, "mpls tunnel is on interface id (0x%x) "
                          "is being used by VCs\n"),
                   ifid));
        return BCM_E_BUSY;
    }

    l3_intf->if_flags &= ~_FE2K_L3_MPLS_TUNNEL_SET;

    /*
     * Finally free the mpls ete
     */
    rv = _bcm_fe2000_undo_l3_ete_alloc(l3_fe, l3_intf, &mpls_sw_ete);

    if (BCM_FAILURE(rv)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls tunnel on Interface Id (0x%x) could not "
                         "be destroyed\n"),
                  ifid));
        return rv;
    }

    /*
     * If there is a valid v4 ete, revert to that
     */
    rv = _bcm_fe2000_get_ete_by_type_on_intf(l3_fe,
                                             l3_intf,
                                             _FE2K_L3_ETE__UCAST_IP,
                                             &l3_ete);
    if ((rv != BCM_E_NONE) && (rv != BCM_E_NOT_FOUND)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) finding ipv4 ete on intf(0x%x)\n"),
                  bcm_errmsg(rv), ifid));
        return rv;
    }

    if (BCM_SUCCESS(rv)) {
        rv = _bcm_fe2000_g2p3_update_ipv4_ete(l3_fe, l3_ete);
        if (rv != BCM_E_NONE) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) setting ipv4 ete on intf(0x%x)\n"),
                      bcm_errmsg(rv), ifid));
            return rv;
        }
    }


    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_fe2000_check_mpls_initiator_params
 * Purpose:
 *      Validate params given for mpls initiator
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      l3_intf     - the egress L3 interface context
 *      num_labels  - number of labels in label_array
 *      label_array - label array
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_fe2000_check_mpls_initiator_params(_fe2k_l3_fe_instance_t  *l3_fe,
                                        _fe2k_l3_intf_t         *l3_intf,
                                        int                      num_labels,
                                        bcm_mpls_egress_label_t *label_array)
{
    int      i, status;
    int      qosMapId;

    MPLS_VVERB((_SBX_D(l3_fe->fe_unit, "Enter\n")));

    status = BCM_E_NONE;
    if ((num_labels <= 0) || (num_labels > _FE2K_MAX_MPLS_TNNL_LABELS)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "invalid  number of labels (%d)\n"), num_labels));
        return BCM_E_PARAM;
    }

    if (label_array == NULL) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "label_array is null\n")));
        return BCM_E_PARAM;
    }

    if ((label_array->label == _BCM_MPLS_IMPLICIT_NULL_LABEL) &&
        (num_labels != 1)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "invalid number of labels(%d) for label(0x%x)\n"),
                  num_labels, label_array->label));
        return BCM_E_PARAM;
    }

    /*
     *  For right now, since we can only have one qosMapId, this value must be
     *  the same (thence, both qos_map_id and flags.remark must be the same for
     *  all labels in the stack.  We'll check that later; this just loads the
     *  value for the outer label.
     */
    qosMapId = label_array->qos_map_id;

    for (i = 0; i < num_labels; i++) {

        /**
         * Two modes are supported for TTL ops
         * ERH.ttl - 1 OR setting a specific value
         */
        if (label_array->flags & BCM_MPLS_EGRESS_LABEL_TTL_COPY) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "TTL_COPY is not supported\n")));
            return BCM_E_PARAM;
        }

        if ((label_array->flags & (BCM_MPLS_EGRESS_LABEL_EXP_SET    |
                                   BCM_MPLS_EGRESS_LABEL_EXP_COPY)) &&
            !_BCM_EXP_VALID(label_array->exp)) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "label_array[%d] invalid exp\n"), i));
            return BCM_E_PARAM;
        }

        if (label_array->qos_map_id != qosMapId) {
            /* qos_map_id disagrees with outer layer */
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "label_array[%d] qos_map_id (%08X)"
                             " disagrees with label_array[0] qos_map_id (0x%08X)\n"),
                      i,label_array->qos_map_id, qosMapId));
            return BCM_E_PARAM;
        }

        if ((label_array->flags & BCM_MPLS_EGRESS_LABEL_PRI_SET) &&
            !_BCM_PKT_PRI_VALID(label_array->pkt_pri)) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "label_array[%d] invalid pri\n"), i));
            return BCM_E_PARAM;
        }

        if (!_BCM_PKT_CFI_VALID(label_array->pkt_cfi)) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "label_array[%d] invalid pkt_cfi\n"), i));
            return BCM_E_PARAM;
        }

        if (!_FE2K_QOS_MAP_ID_VALID(label_array->qos_map_id)) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "label_array[%d] invalid qos_map_id\n"), i));
            return BCM_E_PARAM;
        }

        label_array++;
    }

    /**
     * Only one tunnel is allowed on an interface at one time
     * so if the interface has a v4 encap tunnel configured, return
     * an error
     */
    if (l3_intf->if_tunnel_info) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "interface (0x%x) already has a v4encap tunnel\n"),
                  _FE2K_USER_HANDLE_FROM_IFID(l3_intf->if_info.l3a_intf_id)));
        return BCM_E_PARAM;
    }

    /**
     * If there is more than one egress object on this intf then return an error.
     */
    if (l3_intf->if_ip_ete_count > 1) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "cannot have more than one egress object on interface "
                         "and enable mpls-tunnel\n")));
        return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}


int
_bcm_fe2000_mpls_alloc_vpncb(_fe2k_l3_fe_instance_t  *l3_fe,
                             _fe2k_vpn_control_t    **vpnc)
{

    *vpnc = sal_alloc(sizeof(_fe2k_vpn_control_t),
                      "MPLS-vpncb");
    if (*vpnc == NULL) {
        return BCM_E_MEMORY;
    }

    sal_memset(*vpnc, 0, sizeof(_fe2k_vpn_control_t));

    (*vpnc)->vpn_id   = _FE2K_INVALID_VPN_ID;
    (*vpnc)->vpn_vrf  = _FE2K_INVALID_VRF;

    DQ_INIT(&(*vpnc)->vpn_sap_head);
    DQ_INIT(&(*vpnc)->vpn_fe_link);

    return BCM_E_NONE;
}



int
_bcm_fe2000_mpls_free_vpncb(_fe2k_l3_fe_instance_t  *l3_fe,
                            _fe2k_vpn_control_t    **vpnc)
{
    int rv = BCM_E_UNAVAIL;
    
    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_mpls_free_vpncb(l3_fe, vpnc);
        break;

    default:
        SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;
}

int
_bcm_fe2000_destroy_mpls_vpn_id(int                    unit,
                                _fe2k_l3_fe_instance_t  *l3_fe,
                                _fe2k_vpn_control_t    **vpnc)
{
    int                           status;
    int                           vrf;

    vrf    = (*vpnc)->vpn_vrf;
    status = _bcm_fe2000_mpls_free_vpncb(l3_fe, vpnc);
    if (status != BCM_E_NONE) {
        return status;
    }

    if (vrf != BCM_L3_VRF_DEFAULT && _BCM_VRF_VALID(vrf)) {
        l3_fe->fe_vpn_by_vrf[vrf] = NULL;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_fe2000_mpls_vpn_port_recover
 * Purpose:
 *       recovers the VPN and VPS mpls gport for a given VSI
 * Parameters:
 *     (in)  l3_fe - fe instance
 *     (in)  port  - port vsi was found, set to invalid if remote 
 *     (in)  vid   - vid vsi was found, bcm_vlan_none if remote
 *     (in)  vsi   - vsi the vpws gport occupies
 * Returns:
 *      BCM_E_XXX
 */
#ifdef BCM_WARM_BOOT_SUPPORT
int
_bcm_fe2000_mpls_vpws_vpn_port_recover(_fe2k_l3_fe_instance_t *l3_fe,
                                       uint32 port, uint32 vid, uint32 vsi)
{
    int                   rv;
    soc_sbx_g2p3_ft_t     fte;
    soc_sbx_g2p3_v2e_t    ve;
    uint32                vpn_id, fti;
    bcm_mpls_vpn_config_t vpn_info;
    int                   fab_node, fab_port;
    bcm_mpls_port_t       mpls_port;
    bcm_gport_t           fabric_gport;
    int                   local_gport = SOC_PORT_VALID(l3_fe->fe_unit, (int)port);

    if (local_gport) {
        MPLS_VERB((_SBX_D(l3_fe->fe_unit, "Processing port=%d vid=0x%03x vsi=0x%04x\n"),
                   port, vid, vsi));
    }

    fti = vsi + l3_fe->vlan_ft_base;
    rv = soc_sbx_g2p3_ft_get(l3_fe->fe_unit, fti, &fte);
    if (BCM_FAILURE(rv)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Failed to read fte[0x%x]: %s\n"),
                  fti, bcm_errmsg(rv)));
        return rv;
    }

    if (fte.excidx == VLAN_INV_FTE_EXC || fte.mc == 1) {
        /* not dealing with an MPLS gport */
        return BCM_E_NONE;
    }

    rv = soc_sbx_g2p3_v2e_get(l3_fe->fe_unit, vsi, &ve);
    if (BCM_FAILURE(rv)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Failed to read v2e[0x%x]: %s\n"),
                  vsi, bcm_errmsg(rv)));
        return rv;
    }

    if (ve.v4route == 0) {

        /* Recover the soft vpn id embedded in the vsi */
        rv = _bcm_fe2000_mpls_vpws_wb_get(l3_fe, vsi, &vpn_id);
        
        MPLS_VERB((_SBX_D(l3_fe->fe_unit, "Found valid GPORT at fti 0x%0x on "
                                 "vpn 0x%0x\n"), fti, vpn_id));
        
        /*  Reserve the VPN, if this is a local port
         */
        if (local_gport) {
            bcm_mpls_vpn_config_t_init(&vpn_info);
            vpn_info.vpn = vpn_id;
            vpn_info.flags = BCM_MPLS_VPN_WITH_ID | BCM_MPLS_VPN_VPWS;
            
            rv = bcm_fe2000_mpls_vpn_id_create(l3_fe->fe_unit, &vpn_info);
            
            if (rv == BCM_E_EXISTS) {
                /* okay, we found the other port already and created the vpn */
                rv = BCM_E_NONE;
            }
            if (BCM_FAILURE(rv)) {
                MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Failed to allocate vpnId 0x%x: %s\n"),
                          vpn_id, bcm_errmsg(rv)));
                return rv;
            }
        }

        /* Add the mpls port to the VPN
         */
        bcm_mpls_port_t_init(&mpls_port);

        SOC_SBX_NODE_PORT_FROM_QID(l3_fe->fe_unit, fte.qid, fab_node, fab_port, NUM_COS(l3_fe->fe_unit));
        fab_node += SBX_QE_BASE_MODID;

        BCM_GPORT_MODPORT_SET(fabric_gport, fab_node, fab_port);

        rv = bcm_sbx_stk_fabric_map_get_switch_port(l3_fe->fe_unit, fabric_gport,
                                                    &mpls_port.port);
        if (BCM_FAILURE(rv)) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Failed convert fabric port: %s\n"),
                      bcm_errmsg(rv)));
            return rv;
        }

        BCM_GPORT_MPLS_PORT_ID_SET(mpls_port.mpls_port_id, fti);

        MPLS_VERB((_SBX_D(l3_fe->fe_unit, "Recovering mplsGPort 0x%x, target "
                          "modport=%d:%d\n"), 
                   mpls_port.mpls_port_id,
                   BCM_GPORT_MODPORT_MODID_GET(mpls_port.port),
                   BCM_GPORT_MODPORT_PORT_GET(mpls_port.port)));

        mpls_port.flags = BCM_MPLS_PORT_WITH_ID | BCM_MPLS_PORT_ENCAP_WITH_ID;
        
        mpls_port.match_vlan = vid;
        mpls_port.criteria = BCM_MPLS_PORT_MATCH_PORT_VLAN;

        /* assuming encap is a normal encap and not an L2 for level 1
         * warm boot recovery
         */
        mpls_port.encap_id = SOC_SBX_ENCAP_ID_FROM_OHI(fte.oi);
        
        rv = bcm_fe2000_mpls_port_add(l3_fe->fe_unit, vpn_id, &mpls_port);

        /* Did we find the local gport while scanning for remote gports?
         * if so, ignore the error
         */
        if ((local_gport == FALSE) && (rv == BCM_E_EXISTS)) {
            rv = BCM_E_NONE;
        }
        if (BCM_FAILURE(rv)) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Failed to add mpls gport: %s\n"),
                      bcm_errmsg(rv)));
            return rv;
        }
            
    }
    return rv;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

/*
 * Function:
 *      _bcm_fe2000_mpls_pv_port_recover
 * Purpose:
 *       recovers a port,vid vpws mpls gport; to used with recovery DQ list
 * Parameters:
 *     (in)  elt   - dq element to recover
 *     (in)  l3_fe - fe instance
 * Returns:
 *      BCM_E_XXX
 */
#ifdef BCM_WARM_BOOT_SUPPORT
int 
_bcm_fe2000_mpls_pv_port_recover(dq_t *elt, _fe2k_l3_fe_instance_t  *l3_fe)
{
    uint32          *fast_pv2e_vlan, vsi;
    rc_non_tb_vsi_t *non_tb;
    int              rv;

    non_tb = DQ_ELEMENT_GET(rc_non_tb_vsi_t*, elt, dql);
    
    fast_pv2e_vlan = _bcm_fe2000_recovery_pv2e_vlan_get(l3_fe->fe_unit);
    if (fast_pv2e_vlan == NULL) {
        return BCM_E_CONFIG;
    }

    vsi = fast_pv2e_vlan[RCVR_FAST_PV2E_IDX(non_tb->port, non_tb->vid)];

    rv = _bcm_fe2000_mpls_vpws_vpn_port_recover(l3_fe, 
                                                non_tb->port, non_tb->vid, 
                                                vsi);
    if (BCM_SUCCESS(rv)) {
        DQ_REMOVE(elt);
        sal_free(non_tb);
    }

    return BCM_E_NONE;
}
#endif /* BCM_WARM_BOOT_SUPPORT */


/*
 * Function:
 *      _bcm_fe2000_mpls_recover
 * Purpose:
 *       recovers a mpls gports
 * Parameters:
 *     (in)  l3_fe - fe instance
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_fe2000_mpls_recover(_fe2k_l3_fe_instance_t  *l3_fe)
{
#ifdef BCM_WARM_BOOT_SUPPORT
#ifdef BROADCOM_DEBUG
    sal_usecs_t    t0, t1;
#endif
    int                rv;
    uint32             vsi;
    rc_non_tb_vsi_t    *non_tbs;

#ifdef BROADCOM_DEBUG
    t0 = sal_time_usecs();
#endif

    /* first scan the left over non-tb vsis found for port/vid match mpls ports
     * for local gports
     */
    rv = _bcm_fe2000_recovery_non_tb_vsi_head_get(l3_fe->fe_unit, &non_tbs);
    if (BCM_FAILURE(rv)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Failed to get non tb vsi list: %s\n"),
                         bcm_errmsg(rv)));
        return rv;
    }
    DQ_MAP(&non_tbs->dql, _bcm_fe2000_mpls_pv_port_recover, l3_fe);

    
    /* Now scan the entire dynamic range of VSIs to recover any remote gports
     */
    for (vsi = SBX_DYNAMIC_VSI_FTE_BASE(l3_fe->fe_unit) - l3_fe->vlan_ft_base;
         vsi <= SBX_DYNAMIC_VSI_FTE_END(l3_fe->fe_unit) - l3_fe->vlan_ft_base;
         vsi++) 
    {

        rv = _bcm_fe2000_mpls_vpws_vpn_port_recover(l3_fe, SOC_MAX_NUM_PORTS, 
                                                    BCM_VLAN_NONE, vsi);
        if (BCM_FAILURE(rv)) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Failed to recover gport/vsi 0x%04x: %s\n"),
                      vsi, bcm_errmsg(rv)));
            /* keep trying */
        }
        
    }


#ifdef BROADCOM_DEBUG
    t1 = sal_time_usecs();
    t0 = SAL_USECS_SUB(t1, t0);
    MPLS_OUT((_SBX_D(l3_fe->fe_unit, "Recovery Data time = %d uS (%d S, %d uS)\n"),
              t0, (int)(t0 / 1e6), (int)(t0 - (int)((int)(t0 / 1e6)*1e6))));
#endif
    
    return rv;
#else   /* BCM_WARM_BOOT_SUPPORT */
    return BCM_E_CONFIG;
#endif  /* BCM_WARM_BOOT_SUPPORT */
}

static int
_bcm_fe2000_mplstp_init(int unit)
{
    int index=0;
    soc_sbx_g2p3_v2e_t v2e;
    
    for(index=0;index<MAX_LBL_OPCODE;index++) {
        _sbx_mplstp_lbl_opcode[unit][index] = MAX_LBL_OPCODE;
    }

    /* fill in supported outer label opcode */
    BCM_IF_ERROR_RETURN(
       soc_sbx_g2p3_label_ler_get(unit,&_BCM_FE2K_LABEL_LER(unit)));
    BCM_IF_ERROR_RETURN(
       soc_sbx_g2p3_label_lsr_get(unit,&_BCM_FE2K_LABEL_LSR(unit)));
    BCM_IF_ERROR_RETURN(
       soc_sbx_g2p3_label_eth_pwe3_get(unit,&_BCM_FE2K_LABEL_PWE(unit)));
    BCM_IF_ERROR_RETURN(
       soc_sbx_g2p3_label_ces_pwe3_get(unit,&_BCM_FE2K_LABEL_CES(unit)));

    /* initialize reserved mplstp vpws vsi */
    BCM_IF_ERROR_RETURN(
       soc_sbx_g2p3_vpws_vlan_set(unit, _SBX_GU2_MPLSTP_RSVD_VSI));

    soc_sbx_g2p3_v2e_t_init(&v2e);
    v2e.dontlearn = 1;
    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_v2e_set(unit, _SBX_GU2_MPLSTP_RSVD_VSI, &v2e));
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_fe2000_mpls_init
 * Purpose:
 *      Initialize mpls module
 * Parameters:
 *      unit        - FE unit number
 *      l3_fe       - l3 fe instance
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_fe2000_mpls_init(int                     unit,
                      _fe2k_l3_fe_instance_t *l3_fe)
{
    int                           idx, status;
    int                           hidx;
    _fe2k_vpn_control_t          *vpnc;
    int i,j;

    /**
     * Initialize the various DQ
     */
    for (idx = 0; idx < _FE2K_OHI_IDX_HASH_SIZE; idx++) {
        DQ_INIT(&l3_fe->fe_ohi2_vc_ete[idx]);
    }

    for (idx = 0; idx < _FE2K_VPN_HASH_SIZE; idx++) {
        DQ_INIT(&l3_fe->fe_vpn_hash[idx]);
    }

    sal_memset(&l3_fe->fe_vpn_by_vrf[0],
               (int)NULL,
               sizeof(_fe2k_vpn_control_t *) * SBX_MAX_VRF);

    /*
     * Now create the vpnc for BCM_L3_VRF_DEFAULT. This way
     * other code in mpls does not need to special case
     * for info->vpn being BCM_L3_VRF_DEFAULT
     */
    status = _bcm_fe2000_mpls_alloc_vpncb(l3_fe,
                                          &vpnc);
    if (status != BCM_E_NONE) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) allocating new vpn control block\n"),
                  bcm_errmsg(status)));
        return status;
    }

    /* init vpnc and insert into fe_vpn_hash */
    vpnc->vpn_id    = l3_fe->fe_vsi_default_vrf;
    vpnc->vpn_vrf   = BCM_L3_VRF_DEFAULT;
    vpnc->vpn_flags = BCM_MPLS_VPN_L3;

    hidx = _FE2K_GET_MPLS_VPN_HASH(vpnc->vpn_id);
    DQ_INSERT_HEAD(&l3_fe->fe_vpn_hash[hidx],
                   &vpnc->vpn_fe_link);
    if (_BCM_VRF_VALID(vpnc->vpn_vrf)) {
        l3_fe->fe_vpn_by_vrf[vpnc->vpn_vrf] = vpnc;
    }

    /* initialize mpls gport database */
    status = shr_htb_create(&mpls_gport_db[unit],
                   MPLS_MAX_GPORT,
                   sizeof(uint32_t),
                   "MPLS Gport Dbase");
    if( BCM_FAILURE(status)) {
        MPLS_ERR((_SBX_D(unit, "error(%s) allocating MPLS gport data base\n"),
                  bcm_errmsg(status)));
        return status;
    }
    /* initialize mpls switch_info database */
    status = shr_htb_create(&mpls_switch_info_db[unit],
                   (8*1024),
                   sizeof(uint32_t),
                   "MPLS switch_info Dbase");
    if( BCM_FAILURE(status)) {
        MPLS_ERR((_SBX_D(unit, "error(%s) allocating MPLS switch_info data base\n"),
                  bcm_errmsg(status)));
        return status;
    }


    /* clear trunk association structure */
    for(i=0; i < BCM_MAX_NUM_UNITS; i++) {
        for(j=0; j < SBX_MAX_TRUNKS; j++) {
            /* initialize the port list */
            DQ_INIT(&mpls_trunk_assoc_info[i][j].plist);
            bcm_trunk_add_info_t_init(&mpls_trunk_assoc_info[i][j].add_info);
        }
    }

    /* register callback for trunk change */
    status = bcm_fe2000_trunk_change_register(unit, _bcm_fe2000_mpls_trunk_cb, NULL);
    if (BCM_FAILURE(status)) {
        MPLS_ERR((_SBX_D(unit, "error(%s) registering trunk change callback\n"),
                  bcm_errmsg(status)));
        return status;
    }
     
    if( SOC_SBX_CFG(unit)->mplstp_ena) {
        status = _bcm_fe2000_mplstp_init(unit);
        if(BCM_FAILURE(status)) {
            MPLS_ERR((_SBX_D(unit, "error(%s) initializing MPLSTP\n"),
                      bcm_errmsg(status)));
            return status;
        }
    }

    _FE2K_SET_MPLS_INITIALIZED(l3_fe);

    return status;
}

/*
 * Function:
 *      _bcm_fe2000_mpls_cleanup
 * Purpose:
 *      Cleanup mpls module
 * Parameters:
 *      unit        - FE unit number
 *      l3_fe       - l3 fe instance
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_fe2000_mpls_cleanup(int                     unit,
                         _fe2k_l3_fe_instance_t *l3_fe)
{
    int status,idx;

    MPLS_VVERB((_SBX_D(l3_fe->fe_unit, "Enter\n")));

    /**
     * XXX: Cleanup all the memory allocated and resources
     */
    for (idx = 0; idx < _FE2K_OHI_IDX_HASH_SIZE; idx++) {
        if (!DQ_EMPTY(&l3_fe->fe_ohi2_vc_ete[idx])) {
            MPLS_WARN((_SBX_D(l3_fe->fe_unit, "Found unfreed vc ete at idx %d\n"), idx));
            /* return BCM_E_BUSY; */
        }
    }

    for (idx = 0; idx < SBX_MAX_VRF; idx++) {
        if (l3_fe->fe_vpn_by_vrf[idx] != NULL) {
            MPLS_WARN((_SBX_D(l3_fe->fe_unit, "Found unfreed vpn vrf at idx %d\n"), 
                       idx));
            /* return BCM_E_BUSY;*/
        }
    }

    for (idx = 0; idx < _FE2K_VPN_HASH_SIZE; idx++) {
        if (!DQ_EMPTY(&l3_fe->fe_vpn_hash[idx])) {
            MPLS_WARN((_SBX_D(l3_fe->fe_unit, "Found unfreed vpn hash idx %d\n"), idx));
            /* return BCM_E_BUSY; */
        }
    }

    sal_memset(l3_fe->fe_vpn_by_vrf,
               (int)NULL,
               sizeof(_fe2k_vpn_control_t *)*SBX_MAX_VRF);

    /* initialize mpls gport database */
    status = shr_htb_destroy(&mpls_gport_db[unit], NULL);
    if( BCM_FAILURE(status)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) freeing MPLS gport data base\n"),
                  bcm_errmsg(status)));
    }
    /* initialize mpls switch_info database */
    status = shr_htb_destroy(&mpls_switch_info_db[unit], NULL);
    if( BCM_FAILURE(status)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) freeing MPLS switch_info data base\n"),
                  bcm_errmsg(status)));
    }


    _FE2K_SET_MPLS_UNINITIALIZED(l3_fe);

    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_fe2000_mpls_find_vpn_sap_by_gport
 * Purpose:
 *      Find vpn sap for a gport given gport
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      gport       - gport to find
 *      vpn_sap     - sap found
 * Note:
 *    The _FE2K_ALL_VPNC and _FE2K_ALL_VPN_SAP_PER_VPNC macros CANNOT be
 *   used in conjunction with 'break' statements due to the implicit
 *   loops defined by the macros.
 *
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_fe2000_mpls_find_vpn_sap_by_gport(_fe2k_l3_fe_instance_t *l3_fe, 
                                       bcm_gport_t             gport,
                                       _fe2k_vpn_sap_t       **vpn_sap) 
{
    _fe2k_vpn_control_t          *vpnc = NULL;
    
    _FE2K_ALL_VPNC(l3_fe, vpnc) {
        _FE2K_ALL_VPN_SAP_PER_VPNC(vpnc, (*vpn_sap)) {
            if (gport == (*vpn_sap)->vc_mpls_port_id) {
                return TRUE;
            }
        } _FE2K_ALL_VPN_SAP_PER_VPNC_END(vpnc, (*vpn_sap));
    } _FE2K_ALL_VPNC_END(l3_fe, vpnc);

    *vpn_sap = NULL;
    return FALSE;
}

/*
 * Function:
 *      _bcm_fe2000_find_mpls_vpncb_by_id
 * Purpose:
 *      Find vpn control block based on vpn-id
 * Parameters:
 *      unit        - FE unit number
 *      l3_fe       - l3 fe instance
 *      vpn_id      - vpn identifier
 *      vpnc        - vpn control
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_fe2000_find_mpls_vpncb_by_id(int                      unit,
                                  _fe2k_l3_fe_instance_t  *l3_fe,
                                  bcm_vpn_t                vpn_id,
                                  _fe2k_vpn_control_t    **vpnc)
{
    int                           hidx;
    _fe2k_vpn_control_t          *vpnc_temp = NULL;

    MPLS_VVERB((_SBX_D(l3_fe->fe_unit, "Enter (vpn_id=0x%x)\n"), (int)vpn_id));

    *vpnc = NULL;
    hidx = _FE2K_GET_MPLS_VPN_HASH(vpn_id);

    _FE2K_ALL_VPNC_PER_BKT(l3_fe, hidx, vpnc_temp) {

        if (vpnc_temp->vpn_id == vpn_id) {
            *vpnc = vpnc_temp;
            return BCM_E_NONE;
        }

    } _FE2K_ALL_VPNC_PER_BKT_END(l3_fe, hidx, vpnc_temp);

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      _bcm_fe2000_find_mpls_vpncb_by_mcg
 * Purpose:
 *      Find vpn control block based on broadcast group
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      group       - broadcast group
 *      vpnc        - vpn control
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_fe2000_find_mpls_vpncb_by_mcg(_fe2k_l3_fe_instance_t  *l3_fe,
                                   bcm_multicast_t          group,
                                   _fe2k_vpn_control_t    **vpnc)
{
    _fe2k_vpn_control_t          *vpnc_temp = NULL;

    MPLS_VVERB((_SBX_D(l3_fe->fe_unit, "Enter group(0x%x)\n"), (int)group));

    *vpnc = NULL;

    _FE2K_ALL_VPNC(l3_fe, vpnc_temp) {
        if (vpnc_temp->vpn_bc_mcg == group) {
            *vpnc = vpnc_temp;
            return BCM_E_NONE;
        }
    } _FE2K_ALL_VPNC_END(l3_fe, vpnc_temp);

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      _bcm_fe2000_find_vpn_sap_by_id
 * Purpose:
 *      Find mpls vpn service attachment point
 *      given mpls port identifier
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      vpnc        - vpn control
 *      vpn_sap     - vpn service attachment point
 *      mpls_port_id- global mpls port identifier
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_fe2000_find_vpn_sap_by_id(_fe2k_l3_fe_instance_t *l3_fe,
                               _fe2k_vpn_control_t   *vpnc,
                               bcm_gport_t            mpls_port_id,
                               _fe2k_vpn_sap_t      **vpn_sap)
{
    _fe2k_vpn_sap_t              *temp_vpn_sap = NULL;

    MPLS_VVERB((_SBX_D(l3_fe->fe_unit, "Enter mpls-port-id(0x%x)\n"), (int)mpls_port_id));

    *vpn_sap = NULL;

    _FE2K_ALL_VPN_SAP_PER_VPNC(vpnc, temp_vpn_sap) {

        if (mpls_port_id == temp_vpn_sap->vc_mpls_port_id) {
            *vpn_sap = temp_vpn_sap;
            return BCM_E_NONE;
        }

    } _FE2K_ALL_VPN_SAP_PER_VPNC_END(vpnc, temp_vpn_sap);

    return BCM_E_NOT_FOUND;
}


/*
 * Function:
 *      _bcm_fe2000_find_vpn_sap_by_port
 * Purpose:
 *      Find mpls vpn service attachment point
 *      given mpls port (mod,port)
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      vpnc        - vpn control
 *      vpn_sap     - vpn service attachment point
 *      port        - port as a gport
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_fe2000_find_vpn_sap_by_port(_fe2k_l3_fe_instance_t *l3_fe,
                                 _fe2k_vpn_control_t   *vpnc,
                                 bcm_gport_t            port,
                                 _fe2k_vpn_sap_t      **vpn_sap)
{
    _fe2k_vpn_sap_t              *temp_vpn_sap = NULL;

    MPLS_VVERB((_SBX_D(l3_fe->fe_unit, "Enter port(0x%x)\n"), (int)port));

    *vpn_sap = NULL;

    _FE2K_ALL_VPN_SAP_PER_VPNC(vpnc, temp_vpn_sap) {

        if (port == temp_vpn_sap->vc_mpls_port.port) {
            *vpn_sap = temp_vpn_sap;
            return BCM_E_NONE;
        }

    } _FE2K_ALL_VPN_SAP_PER_VPNC_END(vpnc, temp_vpn_sap);

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      _bcm_fe2000_find_vpn_sap_by_port_vlan
 * Purpose:
 *      Find mpls vpn service attachment point
 *      given mpls port (mod,port)
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      vpnc        - vpn control
 *      vpn_sap     - vpn service attachment point
 *      port        - port as a gport
 *      vlan        - vlan
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_fe2000_find_vpn_sap_by_port_vlan(_fe2k_l3_fe_instance_t *l3_fe,
                                      _fe2k_vpn_control_t   *vpnc,
                                      bcm_gport_t            port,
                                      bcm_vlan_t             vlan,
                                      _fe2k_vpn_sap_t      **vpn_sap)
{
    _fe2k_vpn_sap_t              *temp_vpn_sap = NULL;

    MPLS_VVERB((_SBX_D(l3_fe->fe_unit, "Enter port(0x%x) vlan (0x%0x)\n"),
                (int)port, (int)vlan));

    *vpn_sap = NULL;

    _FE2K_ALL_VPN_SAP_PER_VPNC(vpnc, temp_vpn_sap) {

        if ( (port == temp_vpn_sap->vc_mpls_port.port) &&
             (vlan == temp_vpn_sap->vc_mpls_port.match_vlan) ){
            *vpn_sap = temp_vpn_sap;
            return BCM_E_NONE;
        }

    } _FE2K_ALL_VPN_SAP_PER_VPNC_END(vpnc, temp_vpn_sap);

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      _bcm_fe2000_find_vpn_sap_by_label
 * Purpose:
 *      Find mpls vpn service attachment point
 *      given mpls port (mod,port)
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      vpnc        - vpn control
 *      vpn_sap     - vpn service attachment point
 *      label       - match label
 *      is_failover - failover flag
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_fe2000_find_vpn_sap_by_label(_fe2k_l3_fe_instance_t *l3_fe,
                                  _fe2k_vpn_control_t   *vpnc,
                                  bcm_mpls_label_t           label,
                                  _fe2k_vpn_sap_t      **vpn_sap,
                                  int is_failover)
{
    _fe2k_vpn_sap_t              *temp_vpn_sap = NULL;
    int                           failover_flag;
    MPLS_VVERB((_SBX_D(l3_fe->fe_unit, "Enter label(0x%x) \n"), (int)label));

    *vpn_sap = NULL;


    _FE2K_ALL_VPN_SAP_PER_VPNC(vpnc, temp_vpn_sap) {

        if ( (label == temp_vpn_sap->vc_mpls_port.match_label) ){
            failover_flag = temp_vpn_sap->vc_mpls_port.flags \
                                 & BCM_MPLS_PORT_FAILOVER;
            if (failover_flag == is_failover) {
                *vpn_sap = temp_vpn_sap;
                return BCM_E_NONE;
            }
        }

    } _FE2K_ALL_VPN_SAP_PER_VPNC_END(vpnc, temp_vpn_sap);

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      _bcm_fe2000_create_mpls_vpn_sap
 * Purpose:
 *
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      vpnc        -  vpn control
 *      mpls_port   -  bcm provided port info
 *      vpn_sap     -  allocated vpn service attachment point
 * Returns:
 *      BCM_E_XXX
 * Note:
 *
 */

STATIC int
_bcm_fe2000_create_mpls_vpn_sap(_fe2k_l3_fe_instance_t *l3_fe,
                                _fe2k_vpn_control_t   *vpnc,
                                bcm_mpls_port_t       *mpls_port,
                                _fe2k_vpn_sap_t      **vpn_sap)
{
    int                           status;

    *vpn_sap = NULL;

    status = _bcm_fe2000_find_vpn_sap_by_id(l3_fe, vpnc, 
                                            mpls_port->mpls_port_id, vpn_sap);

    if ((status == BCM_E_NONE) || (status != BCM_E_NOT_FOUND)) {
        return (status == BCM_E_NONE) ? BCM_E_EXISTS : status;
    }

    status = _bcm_fe2000_alloc_mpls_vpn_sap(l3_fe, vpnc,
                                            mpls_port->mpls_port_id, vpn_sap);
    if (status != BCM_E_NONE) {
        return status;
    }

    /* keep copy of user params */
    (*vpn_sap)->vc_mpls_port = *mpls_port;

    /* gport for this vpn-sap */
    (*vpn_sap)->vc_mpls_port_id = mpls_port->mpls_port_id;

    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_fe2000_fill_mpls_port_from_vpn_sap
 * Purpose:
 *      Fill mpls port information based on mpls vpn sap
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      vpn_sap     - vpn service attachment point
 *      mpls_port   - mpls port parameters
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_fe2000_fill_mpls_port_from_vpn_sap(_fe2k_l3_fe_instance_t  *l3_fe,
                                        _fe2k_vpn_sap_t         *vpn_sap,
                                        bcm_mpls_port_t         *mpls_port)
{

    MPLS_VVERB((_SBX_D(l3_fe->fe_unit, "Enter\n")));

    bcm_mpls_port_t_init(mpls_port);

    /* This has to be the same */
    mpls_port->mpls_port_id        = vpn_sap->vc_mpls_port.mpls_port_id;

    /* Below *may* be derived */
    mpls_port->flags               = vpn_sap->vc_mpls_port.flags;
    mpls_port->if_class            = vpn_sap->vc_mpls_port.if_class;
    mpls_port->exp_map             = vpn_sap->vc_mpls_port.exp_map;
    mpls_port->int_pri             = vpn_sap->vc_mpls_port.int_pri;
    mpls_port->service_tpid        = vpn_sap->vc_mpls_port.service_tpid;

    /* Below *cannot* be derived */
    mpls_port->port                = vpn_sap->vc_mpls_port.port;
    mpls_port->criteria            = vpn_sap->vc_mpls_port.criteria;
    mpls_port->match_vlan          = vpn_sap->vc_mpls_port.match_vlan;
    mpls_port->match_inner_vlan    = vpn_sap->vc_mpls_port.match_inner_vlan;
    mpls_port->match_label         = vpn_sap->vc_mpls_port.match_label;

    /* Below *can* be derived */
    mpls_port->egress_tunnel_if    = vpn_sap->vc_mpls_port.egress_tunnel_if;
    mpls_port->egress_label        = vpn_sap->vc_mpls_port.egress_label;
    mpls_port->egress_service_vlan = vpn_sap->vc_mpls_port.egress_service_vlan;

    /* Always retrieve the encapId from sw state.  VPWS is implemented by
     * swapping ohi's & ftes, so this ohi may be the hw.ohi for the peer.
     */
    mpls_port->encap_id            = vpn_sap->vc_mpls_port.encap_id;

    mpls_port->failover_id         = vpn_sap->vc_mpls_port.failover_id;
    mpls_port->failover_port_id    = vpn_sap->vc_mpls_port.failover_port_id;
    mpls_port->policer_id          = vpn_sap->vc_mpls_port.policer_id;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_fe2000_fill_mpls_vpn_config
 * Purpose:
 *      Fill vpn config params based on vpn control
 * Parameters:
 *      unit        - FE2k unit
 *      vpnc        - vpn control
 *      info        - vpn config
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_fe2000_fill_mpls_vpn_config(int                     unit,
                                 _fe2k_vpn_control_t    *vpnc,
                                 bcm_mpls_vpn_config_t  *info)
{
    MPLS_VVERB((_SBX_D(unit, "Enter\n")));

    info->flags = _BCM_MPLS_VPN_TYPE(vpnc->vpn_flags);
    if (info->flags == BCM_MPLS_VPN_L3) {
        info->lookup_id = vpnc->vpn_vrf;
    } else {
        info->lookup_id = vpnc->vpn_id;
    }

    info->vpn                     = vpnc->vpn_id;
    info->broadcast_group         = vpnc->vpn_bc_mcg;
    info->unknown_unicast_group   = vpnc->vpn_bc_mcg;
    info->unknown_multicast_group = vpnc->vpn_bc_mcg;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_fe2000_mpls_validate_tunnel_switch_add
 * Purpose:
 *      validate mpls_tunnel_switch_add params
 * Parameters:
 *      l3_fe       - l3 fe context
 *      info        - label switching info (LSR/Egress LER)
 *
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_fe2000_mpls_validate_tunnel_switch_add(_fe2k_l3_fe_instance_t   *l3_fe,
                                            bcm_mpls_tunnel_switch_t *info)
{
    int                status;
    uint32             fte_idx;
    bcm_l3_egress_t    bcm_egr;
    int                flags, unit = l3_fe->fe_unit;

    status = BCM_E_NONE;

    if (BCM_GPORT_IS_SET(info->port)) {
        MPLS_ERR((_SBX_D(unit, "incoming port (0x%x) not valid for "
                         " bcm_mpls_tunnel_switch_t\n"), info->port));
        return BCM_E_PARAM;
    }
    if (info->inner_label != BCM_MPLS_LABEL_INVALID) {
        MPLS_ERR((_SBX_D(unit, "bcm_mpls_tunnel_switch_t with inner_label(0x%x) "
                         " not supported\n"), info->inner_label));
        return BCM_E_PARAM;
    }

    if (info->flags & ~_BCM_MPLS_SWITCH_SUPPORTED_FLAGS) {
        MPLS_ERR((_SBX_D(unit, "Invalid flags 0x%x\n"), info->flags));
        return BCM_E_PARAM;
    }

    /* We do not care about inner_label */

    if ( (info->action != BCM_MPLS_SWITCH_ACTION_SWAP) &&
         (info->action != BCM_MPLS_SWITCH_ACTION_PHP)  &&
         (info->action != BCM_MPLS_SWITCH_ACTION_POP)  &&
         (info->action != BCM_MPLS_SWITCH_ACTION_POP_DIRECT)) {
        MPLS_ERR((_SBX_D(unit, "Invalid label switch action %d\n"), info->action));
        return BCM_E_PARAM;
    }

    if (!_BCM_MPLS_VPN_VALID_USER_HANDLE(unit, info->vpn)) {
        MPLS_ERR((_SBX_D(unit, "invalid vpn (0x%0x)\n"), info->vpn));
        return BCM_E_PARAM;
    }

    /* XXX: TBD: info->egress_label.label  needs to be valid for SWAP */
    fte_idx = 0;
    if (info->action == BCM_MPLS_SWITCH_ACTION_SWAP) {
        bcm_l3_egress_t_init(&bcm_egr);
        fte_idx =  _FE2K_GET_FTE_IDX_FROM_USER_HANDLE(info->egress_if);
        /* XXX: TBD: what is the point of label not being there ? */
        flags  = info->egress_label.label ?
            L3_OR_MPLS_GET_FTE__FTE_ETE_BOTH : L3_OR_MPLS_GET_FTE__VALIDATE_FTE_ONLY;
        status = _bcm_fe2000_l3_get_egrif_from_fte(l3_fe,
                                                   fte_idx,
                                                   flags,
                                                   &bcm_egr);
        if (status != BCM_E_NONE) {
            MPLS_ERR((_SBX_D(unit, "error(%s) invalid egress object "
                             "(egress_if == 0x%x) \n"),
                      bcm_errmsg(status), (int)info->egress_if));
            return status;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_fe2000_mpls_tunnel_switch_delete
 * Purpose:
 *      de-program the label2etc table for l3 vpn/global vrf
 * Parameters:
 *      l3_fe       - l3 fe context
 *      info        - label switching info (LSR/Egress LER)
 *
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_fe2000_mpls_tunnel_switch_delete(_fe2k_l3_fe_instance_t   *l3_fe,
                                      bcm_mpls_tunnel_switch_t *info)
{
    int                         rv, idx;
    int                         unit = l3_fe->fe_unit;
    _sbx_gu2_usr_res_types_t    restype;
    soc_sbx_g2p3_l2e_t          l2e;
    soc_sbx_g2p3_label2e_t      label2e;
    soc_sbx_g2p3_ft_t           fte;
    soc_sbx_g2p3_v2e_t          v2e;
    uint32                      ftidx, ftcnt, flags;
    bcm_vlan_t vlan;
    int leren;
    uint32 lpidx;

    MPLS_VVERB((_SBX_D(unit, "Enter\n")));

    if( SOC_SBX_CFG(unit)->mplstp_ena ==1) {
        rv = soc_sbx_g2p3_label2e_get(unit, _FE2K_MPLSTP_LABEL(info->label), &label2e);
    } else {
        rv = soc_sbx_g2p3_l2e_get(unit, _FE2K_MPLS_LABEL(info->label), &l2e);
    }
    if (BCM_FAILURE(rv)) {
        MPLS_ERR((_SBX_D(unit, "Failed to read l2e[0x%x]: %s\n"),
                  info->label, bcm_errmsg(rv)));
        return rv;
    }

    if( SOC_SBX_CFG(unit)->mplstp_ena ==1) {
        ftidx = ((label2e.ftidx1 << 14) | label2e.ftidx0);
        vlan  = label2e.vlan;
        leren = _BCM_FE2K_IS_LABEL_LER(unit, label2e.opcode);
        lpidx = label2e.lpidx;
        ftcnt = 1;
        flags = 0;
    } else {
        ftidx = l2e.ftidx;
        vlan  = l2e.vlan;
        leren = l2e.leren;
        ftcnt = l3_fe->fe_cosq_config_numcos;
        flags = _SBX_GU2_RES_FLAGS_CONTIGOUS;
    }

    if (((info->action == BCM_MPLS_SWITCH_ACTION_POP_DIRECT &&
          info->egress_if))                                    ||
        (info->action == BCM_MPLS_SWITCH_ACTION_SWAP)          ||
        (info->action == BCM_MPLS_SWITCH_ACTION_PHP)) 
    {
        MPLS_VERB((_SBX_D(unit, "Clearing FTEs[0x%x-0x%x]\n"),
                   ftidx, ftidx + ftcnt - 1));
        soc_sbx_g2p3_ft_t_init(&fte);
        for (idx = 0; idx < ftcnt; idx++) {
            rv = soc_sbx_g2p3_ft_set(unit, ftidx + idx, &fte);
            if (BCM_FAILURE(rv)) {
                MPLS_ERR((_SBX_D(unit, "failed to clear FTE[0x%x]: %s\n\n"),
                          ftidx + idx, bcm_errmsg(rv)));
            }
        }
    }

    
    switch (info->action) {
    case BCM_MPLS_SWITCH_ACTION_SWAP:
    case BCM_MPLS_SWITCH_ACTION_PHP:
        if (leren == 0 &&
            (vlan == BCM_VLAN_NONE || ftidx == SBX_DROP_FTE(unit)))
        {
            restype = SBX_GU2K_USR_RES_FTE_MPLS;
            
            rv = _sbx_gu2_resource_free(unit, restype,
                                        ftcnt,
                                        &ftidx,
                                        flags);
            if (BCM_FAILURE(rv)) {
                MPLS_ERR((_SBX_D(unit, "Action %d Failed to free resource %d %s: %s\n"),
                          info->action, restype, 
                          _sbx_gu2_resource_to_str(restype), 
                          bcm_errmsg(rv)));
                return rv;
            }
            MPLS_VERB((_SBX_D(unit, "Freed %s 0x%x + 0x%x\n"),
                       _sbx_gu2_resource_to_str(restype), 
                       ftidx, ftcnt));
        }

        if ((SOC_SBX_CFG(unit)->mplstp_ena ==1) &&
            (leren == 0 && lpidx != 0)) {
            restype = SBX_GU2K_USR_RES_MPLS_LPORT;
            rv = _sbx_gu2_resource_free(unit, restype,
                                        1,
                                        &lpidx,
                                        0);
            if (BCM_FAILURE(rv)) {
                MPLS_ERR((_SBX_D(unit,
                         "Action %d Failed to free resource %d %s: %s\n"),
                          info->action, restype, 
                          _sbx_gu2_resource_to_str(restype), 
                          bcm_errmsg(rv)));
                return rv;
            }
            MPLS_VERB((_SBX_D(unit, "Freed %s 0x%x\n"),
                       _sbx_gu2_resource_to_str(restype), 
                       lpidx));
        }
        break;
    case BCM_MPLS_SWITCH_ACTION_POP_DIRECT:
    case BCM_MPLS_SWITCH_ACTION_POP:
        if (info->action == BCM_MPLS_SWITCH_ACTION_POP_DIRECT) {
            restype = SBX_GU2K_USR_RES_FTE_MPLS;
        } else {
            restype = SBX_GU2K_USR_RES_VSI;
        }

        if (leren == 0 &&
            (vlan == BCM_VLAN_NONE || ftidx == SBX_DROP_FTE(unit))) 
        {
            rv = _sbx_gu2_resource_free(unit, restype, 
                                        ftcnt,
                                        &ftidx,
                                        flags);
            if (BCM_FAILURE(rv)) {
                MPLS_ERR((_SBX_D(unit, "Action %d Failed to free resource %d %s: %s\n"),
                          info->action, restype, 
                          _sbx_gu2_resource_to_str(restype), 
                          bcm_errmsg(rv)));
                return rv;
            }
        }

        if (info->action == BCM_MPLS_SWITCH_ACTION_POP) {
            MPLS_VERB((_SBX_D(unit, "Clearing v2e[0x%x-0x%x]\n"),
                       vlan, vlan + ftcnt - 1));

            soc_sbx_g2p3_v2e_t_init(&v2e);
            for (idx = 0; idx < ftcnt; idx++) {
                rv = soc_sbx_g2p3_v2e_set(unit, vlan + idx, &v2e);

                if (BCM_FAILURE(rv)) {
                    MPLS_ERR((_SBX_D(unit, "Failed to clear v2e[0x%x] on vpn 0x%x:"
                                     " %s\n"),
                              vlan + idx, info->vpn, bcm_errmsg(rv)));
                    return rv;
                }
            }
        }
        if ((SOC_SBX_CFG(unit)->mplstp_ena ==1) &&
            (leren == 0 && lpidx != 0)) {
            restype = SBX_GU2K_USR_RES_MPLS_LPORT;
            rv = _sbx_gu2_resource_free(unit, restype,
                                        1,
                                        &lpidx,
                                        0);
            if (BCM_FAILURE(rv)) {
                MPLS_ERR((_SBX_D(unit,
                         "Action %d Failed to free resource %d %s: %s\n"),
                          info->action, restype, 
                          _sbx_gu2_resource_to_str(restype), 
                          bcm_errmsg(rv)));
                return rv;
            }
            MPLS_VERB((_SBX_D(unit, "Freed %s 0x%x\n"),
                       _sbx_gu2_resource_to_str(restype), 
                       lpidx));
        }
        break;
    default:
        MPLS_ERR((_SBX_D(unit, "Invalid action: %d\n"), info->action));
        return BCM_E_PARAM;
    }

     if( SOC_SBX_CFG(unit)->mplstp_ena ==1) {
        rv = soc_sbx_g2p3_label2e_delete(unit, _FE2K_MPLSTP_LABEL(info->label));
     } else {
 
        soc_sbx_g2p3_l2e_t_init(&l2e);
        rv = soc_sbx_g2p3_l2e_set(unit, _FE2K_MPLS_LABEL(info->label), &l2e);
    }
    if (BCM_FAILURE(rv)) {
        MPLS_ERR((_SBX_D(unit, "Failed to clear l2e[0x%x]: %s\n"),
                  info->label, bcm_errmsg(rv)));
        return rv;
    }

    return rv;
}

/*
 * Function:
 *      _bcm_fe2000_mpls_tunnel_switch_delete_all
 * Purpose:
 *      LER and LSR ranges needs to be invalidated
 * Parameters:
 *      l3_fe       - l3 fe context
 *
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_fe2000_mpls_tunnel_switch_delete_all(_fe2k_l3_fe_instance_t   *l3_fe)
{
    int                         rv;
    soc_sbx_g2p3_l2e_t          l2e;
    int                         idx;
    int                         unit = l3_fe->fe_unit;
    int                         curr =0, next=0;

    MPLS_VVERB((_SBX_D(unit, "Enter\n")));

    if( SOC_SBX_CFG(unit)->mplstp_ena ==1) {
        rv = soc_sbx_g2p3_label2e_first(unit, &curr);
        while (curr != 0)
        {
            rv = soc_sbx_g2p3_label2e_next(unit, curr, &next);
            rv = soc_sbx_g2p3_label2e_delete( unit, curr);
            curr = next;
        }

    } else {
        soc_sbx_g2p3_l2e_t_init(&l2e);
        for (idx=0; idx < _FE2K_G2P3_MAX_LABELS; idx++) {
            rv = soc_sbx_g2p3_l2e_set(unit, idx, &l2e);
        }
    }
    return BCM_E_NONE;
}



STATIC int
_bcm_fe2000_mpls_map_set_vlan2etc(_fe2k_l3_fe_instance_t   *l3_fe,
                                  _fe2k_vpn_control_t      *vpnc,
                                  int                       program_vsi)
{
    int rv = BCM_E_UNAVAIL;

    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_mpls_map_set_vlan2etc(l3_fe, vpnc, program_vsi);
        break;

    default:
        SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;
}


/*
 * Function:
 *      _bcm_fe2000_mpls_tunnel_switch_update
 * Purpose:
 *      program the label2etc table for l3 vpn/global vrf
 * Parameters:
 *      l3_fe       - l3 fe context
 *      info        - label switching info (LSR/Egress LER)
 *
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_fe2000_mpls_tunnel_switch_update(_fe2k_l3_fe_instance_t   *l3_fe,
                                      bcm_mpls_tunnel_switch_t *info)
{

    MPLS_VVERB((_SBX_D(l3_fe->fe_unit, "Enter\n")));

    if (SOC_IS_SBX_G2P3(l3_fe->fe_unit)) {
        return (_bcm_fe2000_g2p3_mpls_tunnel_switch_update(l3_fe->fe_unit,
                                                           l3_fe,
                                                           info));
    }

    return BCM_E_UNAVAIL;
}



/*
 * Function:
 *      _bcm_fe2000_map_set_vpn_bc_fte
 * Purpose:
 *      map and then set the broadcast fte
 *      for the vpn
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      vpnc        - vpn control
 * Returns:
 *      BCM_E_XXX
 * Note:
 *
 */

STATIC int
_bcm_fe2000_map_set_vpn_bc_fte(_fe2k_l3_fe_instance_t  *l3_fe,
                               _fe2k_vpn_control_t     *vpnc)
{
    int rv = BCM_E_UNAVAIL;

    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_mpls_map_set_vpn_bc_fte(l3_fe, vpnc);
        break;

    default:
        SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;
}

/*
 * Function:
 *      _bcm_fe2000_update_mpls_vpn_id
 * Purpose:
 *      Update information for already existing
 *      vpn config
 * Parameters:
 *      unit        - FE unit number
 *      l3_fe       - l3 fe instance
 *      vpnc        - vpn control
 *      info        - vpn config
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_fe2000_update_mpls_vpn_id(int                     unit,
                               _fe2k_l3_fe_instance_t *l3_fe,
                               _fe2k_vpn_control_t    *vpnc,
                               bcm_mpls_vpn_config_t  *info)
{
    int                           status;
    int                           old_bc_mcg;

    MPLS_VVERB((_SBX_D(l3_fe->fe_unit, "Enter\n")));

    /**
     * Cannot modify the key
     */
    if (_BCM_MPLS_VPN_TYPE(vpnc->vpn_flags) !=
        _BCM_MPLS_VPN_TYPE(info->flags)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "cannot modify type of vpn 0x%x --> 0x%x\n"),
                  vpnc->vpn_flags, info->flags));
        return BCM_E_PARAM;
    }

    switch (_BCM_MPLS_VPN_TYPE(info->flags)) {
    case BCM_MPLS_VPN_L3:
        if (info->lookup_id && (info->lookup_id != vpnc->vpn_vrf)) {
            return BCM_E_PARAM;
        }
        break;

    default:
        if (info->lookup_id && (info->lookup_id != vpnc->vpn_id)) {
            return BCM_E_PARAM;
        }
    }

    if (vpnc->vpn_bc_mcg != info->broadcast_group) {
        old_bc_mcg       = vpnc->vpn_bc_mcg;
        vpnc->vpn_bc_mcg = info->broadcast_group;
        status = _bcm_fe2000_map_set_vpn_bc_fte(l3_fe,
                                                vpnc);
        if (status != BCM_E_NONE) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) setting bcast fte for vpn(0x%x)\n"),
                      bcm_errmsg(status), vpnc->vpn_id));
            vpnc->vpn_bc_mcg = old_bc_mcg;
            return status;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_fe2000_mpls_gport_get_mod_port
 * Purpose:
 *      Given gport decipher module-id and port values
 * Parameters:
 *      unit        - FE unit number
 *      gport       - global port value
 *      modid       - module id
 *      port        - port
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_fe2000_mpls_gport_get_mod_port(int            unit,
                                    bcm_gport_t    gport,
                                    bcm_module_t  *modid,
                                    bcm_port_t    *port)
{
    MPLS_VVERB((_SBX_D(unit, "Enter\n")));

    if (SOC_GPORT_IS_LOCAL(gport)) {
        BCM_IF_ERROR_RETURN(
            bcm_stk_my_modid_get(unit, modid));
        *port = SOC_GPORT_LOCAL_GET(gport);
        return BCM_E_NONE;
    } else if (SOC_GPORT_IS_MODPORT(gport)) {
        *modid = SOC_GPORT_MODPORT_MODID_GET(gport);
        *port  = SOC_GPORT_MODPORT_PORT_GET(gport);
        return BCM_E_NONE;
    }

    return BCM_E_PARAM;
}

/*
 * Function:
 *      _bcm_fe2000_expand_vpn_sap_ufte
 * Purpose:
 *      Expand the user area to keep <modid,fte-id>
 * Parameters:
 *      vpn_sap     -  vpn service attachment point
 * Returns:
 *      BCM_E_XXX
 * Note:
 *
 */

STATIC int
_bcm_fe2000_expand_vpn_sap_ufte(_fe2k_vpn_sap_t   *vpn_sap)
{
    int                           num_new_entries, size, ii;
    _fe2k_vc_ete_fte_t           *vc_ete_fte;

    num_new_entries = vpn_sap->vc_alloced_ue + _FE2K_ETE_USER_SLAB_SIZE;
    size = sizeof(_fe2k_vc_ete_fte_t) * num_new_entries;

    vc_ete_fte = (_fe2k_vc_ete_fte_t *)
        sal_alloc(size, "MPLS-ete-fte");
    if (vc_ete_fte == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(vc_ete_fte, 0, size);
    for (ii = 0; ii < vpn_sap->vc_alloced_ue; ii++) {
        vc_ete_fte[ii] = vpn_sap->u.vc_fte[ii];
    }
    sal_free(vpn_sap->u.vc_fte);
    vpn_sap->u.vc_fte      = vc_ete_fte;
    vpn_sap->vc_alloced_ue = num_new_entries;

    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_fe2000_find_vpn_sap_ufte
 * Purpose:
 *      find in the user area if the <modid,fte-idx>
 *      is already present
 * Parameters:
 *      l3_fe         -  fe instance structure
 *      vpn_sap       -  vpn service attachment point
 *      caller_module -  module id
 *      slot          -  slot if we find entry
 * Returns:
 *      BCM_E_XXX
 * Note:
 *
 */

STATIC int
_bcm_fe2000_find_vpn_sap_ufte(_fe2k_l3_fe_instance_t  *l3_fe,
                              _fe2k_vpn_sap_t         *vpn_sap,
                              bcm_module_t             caller_module,
                              int                     *slot)
{
    int                           ii;
    uint32                        fte_idx;

    fte_idx = BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id);
    MPLS_VVERB((_SBX_D(l3_fe->fe_unit, "mpls_port=0x%08x  fte=0x%04x\n"),
                vpn_sap->vc_mpls_port_id, fte_idx));

    *slot   = -1;
    for (ii = 0; ii < vpn_sap->vc_inuse_ue; ii++) {
        MPLS_VVERB((_SBX_D(l3_fe->fe_unit, "compare mod/fte:  %d/0x%04x %d/0x%04x slot=%d\n"),
                    vpn_sap->u.vc_fte[ii].mod_id,
                    vpn_sap->u.vc_fte[ii].fte_idx.fte_idx,
                    caller_module, fte_idx, ii));

        if ((vpn_sap->u.vc_fte[ii].mod_id == caller_module) &&
            (vpn_sap->u.vc_fte[ii].fte_idx.fte_idx == fte_idx)) {
            *slot = ii;
            return BCM_E_NONE;
        }
    }

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      _bcm_fe2000_link_vpn_sap_ufte
 * Purpose:
 *      link given <mod,fte-id> into user area
 * Parameters:
 *      l3_fe          - l3 fe instance
 *      vpn_sap        -  vpn service attachment point
 *      caller_module  -  module id
 * Returns:
 *      BCM_E_XXX
 * Note:
 *
 */

STATIC int
_bcm_fe2000_link_vpn_sap_ufte(_fe2k_l3_fe_instance_t  *l3_fe,
                              _fe2k_vpn_sap_t         *vpn_sap,
                              bcm_module_t             caller_module)
{
    int                           status;

    if (vpn_sap->vc_alloced_ue == vpn_sap->vc_inuse_ue) {
        status = _bcm_fe2000_expand_vpn_sap_ufte(vpn_sap);
        if (status != BCM_E_NONE) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "failure(%s) expanding vpn-sap\n"),
                      bcm_errmsg(status)));
            return status;
        }
    }

    MPLS_VVERB((_SBX_D(l3_fe->fe_unit, "insert gport 0x%08x mod/fte %d/0x%04x to "
                       "slot %d\n"),
                vpn_sap->vc_mpls_port_id, caller_module,
                BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id),
                vpn_sap->vc_inuse_ue));

    /**
     * FTE-idx has to be globally same
     * Please never do a find and add. Temporarily during
     * update --> delete handling for bcm_mpls_vpn_port_udate()
     * there will be 2 of the same <modid, fte-idx>
     */
    vpn_sap->u.vc_fte[vpn_sap->vc_inuse_ue].mod_id  = caller_module;
    vpn_sap->u.vc_fte[vpn_sap->vc_inuse_ue].fte_idx.fte_idx =
        BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id);
    vpn_sap->vc_inuse_ue++;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_fe2000_unlink_vpn_sap_ufte
 * Purpose:
 *      unlink given <mod,fte-id> into user area
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      vpn_sap     -  vpn service attachment point
 *      fte_idx     -  h/w fte idx
 *      module      -  module id
 * Returns:
 *      BCM_E_XXX
 * Note:
 *
 */

STATIC int
_bcm_fe2000_unlink_vpn_sap_ufte(_fe2k_l3_fe_instance_t  *l3_fe,
                                _fe2k_vpn_sap_t         *vpn_sap,
                                bcm_module_t             caller_module)
{
    int                           status, slot;

    status = _bcm_fe2000_find_vpn_sap_ufte(l3_fe,
                                           vpn_sap,
                                           caller_module,
                                           &slot);
    if (status != BCM_E_NONE) {
        return status;
    }

    vpn_sap->vc_inuse_ue--;
    vpn_sap->u.vc_fte[slot] =
        vpn_sap->u.vc_fte[vpn_sap->vc_inuse_ue];

    return BCM_E_NONE;
}

int
_bcm_fe2000_find_vpn_sap_by_ohi(_fe2k_l3_fe_instance_t *l3_fe,
                                uint32                  ohi,
                                _fe2k_vpn_sap_t       **vpn_sap)
{
    int                           hidx;
    dq_p_t                        vc_ohi_head, vc_sap_elem;
    _fe2k_vpn_sap_t              *tmp_sap = NULL;

    hidx = _FE2K_GET_OHI2ETE_HASH_IDX(ohi);
    vc_ohi_head = &l3_fe->fe_ohi2_vc_ete[hidx];

    DQ_TRAVERSE(vc_ohi_head, vc_sap_elem) {
        _FE2K_VPN_SAP_FROM_OHI_HASH_DQ(vc_sap_elem, tmp_sap);

        if (tmp_sap->vc_ohi.ohi  == ohi) {
            *vpn_sap = tmp_sap;
            return BCM_E_NONE;
        }
    } DQ_TRAVERSE_END(vc_ohi_head, vc_sap_elem);

    return BCM_E_NOT_FOUND;
}





STATIC int
_bcm_fe2000_invalidate_vpn_sap_hw_resources(_fe2k_l3_fe_instance_t *l3_fe,
                                            _fe2k_vpn_sap_t    *vpn_sap)
{
    int rv = BCM_E_UNAVAIL;

    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {

    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_mpls_invalidate_vpn_sap_hw_resources(l3_fe, vpn_sap);
        break;

    default:
        SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;
}

int
_bcm_fe2000_destroy_vpn_sap_hw_resources(_fe2k_l3_fe_instance_t *l3_fe,
                                         bcm_module_t            caller_module,
                                         _fe2k_vpn_sap_t       **vpn_sap)
{
    int                               status;

    status = _bcm_fe2000_unlink_vpn_sap_ufte(l3_fe, *vpn_sap, caller_module);

    if (status != BCM_E_NONE) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) unlinking vpn-sap\n"),
                         bcm_errmsg(status)));
        /* do not return */
    }

    if ((*vpn_sap)->vc_inuse_ue == 0) {
        /*
         * Invalidate and free the resources
         */
        status = _bcm_fe2000_invalidate_vpn_sap_hw_resources(l3_fe, *vpn_sap);
        if (status != BCM_E_NONE) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) invalidating vpn-sap resources\n"),
                      bcm_errmsg(status)));
            return status;
        }

        status = _bcm_fe2000_free_mpls_vpn_sap(l3_fe, vpn_sap);
        if (status != BCM_E_NONE) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) freeing vpn-sap\n"), 
                      bcm_errmsg(status)));
            return status;
        }
    }

    return BCM_E_NONE;
}




/*
 * Function:
 *     _bcm_fe2000_mpls_get_fte
 * Purpose:
 *     Given an fte index get mpls port information
 * Parameters:
 *     l3_fe      - (IN)     fe instance corresponsing to unit
 *     ul_fte     - (IN)     the fte index
 *     action     - (IN)     L3_OR_MPLS_GET_FTE__FTE_CONTENTS_ONLY
 *                           L3_OR_MPLS_GET_FTE__VALIDATE_FTE_ONLY
 *                           L3_OR_MPLS_GET_FTE__FTE_ETE_BOTH
 *     fte_mpls_port - (OUT) mpls-port information from fte
 *
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 */

STATIC int
_bcm_fe2000_mpls_get_fte(_fe2k_l3_fe_instance_t *l3_fe,
                         uint32                  fte_idx,
                         int                     action,
                         _fe2k_l3_or_mpls_egress_t *egr)
{
    int rv = BCM_E_INTERNAL;

    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_mpls_get_fte(l3_fe, fte_idx, action, egr);
        break;

    default:
        SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;
}

/*
 * Function:
 *     _bcm_fe2000_destroy_mpls_fte
 * Purpose:
 *     destroy gport fte, optionally ohi and ete
 * Parameters:
 *     l3_fe      - (IN)     l3 fe instance
 *     action     - (IN)     mode of operation
 *     fte_idx    - (IN)     handle that needs to be destroyed
 *     module_id  - (IN)     module in case of FTE_ONLY
 *     encap_id   - (IN)     ohi in case of FTE_ONLY
 *     vpnc       - (IN)     vpn control block
 *     is_trunk   - (IN)     whether the egress port is trunk
 * Returns:
 *     BCM_E_XXX
 * NOTE:
 *     L3_OR_MPLS_DESTROY_FTE__FTE_ONLY
 *        Invalidate FTE and return
 *     L3_OR_MPLS_DESTROY_FTE__FTE_HW_OHI_ETE
 *        Get FTE, Invalidate FTE and delete OHI-->ETE
 *        based on what we got earlier from H/W
 *     L3_OR_MPLS_DESTROY_FTE__FTE_OHI_ETE_GIVEN_OHI
 *        Invalidate FTE and delete OHI-->ETE
 *        based on user params
 */

STATIC int
_bcm_fe2000_destroy_mpls_fte(_fe2k_l3_fe_instance_t *l3_fe,
                             int                     action,
                             uint32                  fte_idx,
                             bcm_module_t            param_modid,
                             bcm_if_t                param_encap_id,
                             _fe2k_vpn_control_t    *vpnc,
                             int8_t                  is_trunk)
{
    int                              status;
    _fe2k_l3_or_mpls_egress_t        egr;
    bcm_if_t                         encap_id;
    bcm_module_t                     exit_modid;
    _fe2k_vpn_sap_t                 *vpn_sap;
    bcm_gport_t                      mpls_port_id;


    MPLS_VVERB((_SBX_D(l3_fe->fe_unit, "Enter\n")));

    if (action == L3_OR_MPLS_DESTROY_FTE__FTE_HW_OHI_ETE ||
        action == L3_OR_MPLS_DESTROY_FTE__FTE_OHI_ETE_GIVEN_OHI) {
        status = _bcm_fe2000_mpls_get_fte(l3_fe,
                                          fte_idx,
                                          L3_OR_MPLS_GET_FTE__FTE_CONTENTS_ONLY,
                                          &egr);
        if (status != BCM_E_NONE) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) could not find fte(0x%x)\n"),
                      bcm_errmsg(status), fte_idx));
            return status;
        }
    }

    /*
     * In any case, invalidate the FTE and free the resource
     */
    status = _bcm_fe2000_invalidate_l3_or_mpls_fte(l3_fe, fte_idx);

    if (status != BCM_E_NONE) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) invalidating gport-fte(0x%x)\n"),
                  bcm_errmsg(status), fte_idx));
        return status;
    }

    if (action == L3_OR_MPLS_DESTROY_FTE__FTE_ONLY) {
        return BCM_E_NONE;
    } else if (action == L3_OR_MPLS_DESTROY_FTE__FTE_HW_OHI_ETE) {
        exit_modid = egr.fte_modid;
        encap_id   = egr.encap_id;
    } else if (action  == L3_OR_MPLS_DESTROY_FTE__FTE_OHI_ETE_GIVEN_OHI) {
        exit_modid = param_modid;
        encap_id   = param_encap_id;
    } else {
        return BCM_E_INTERNAL;
    }

    MPLS_VVERB((_SBX_D(l3_fe->fe_unit, "myMod=%d exitMod=%d encap_id=0x%08x"
                       " ohi=0x%04x\n"),
                l3_fe->fe_my_modid, exit_modid,
                encap_id,  SOC_SBX_OHI_FROM_ENCAP_ID(encap_id)));


    BCM_GPORT_MPLS_PORT_ID_SET(mpls_port_id, fte_idx);

    status = _bcm_fe2000_find_vpn_sap_by_id(l3_fe, vpnc, mpls_port_id,
                                            &vpn_sap);

    if  (status != BCM_E_NONE) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) finding vpn for fte (0x%x)\n"),
                  bcm_errmsg(status), fte_idx));
        return status;
    }

    MPLS_VVERB((_SBX_D(l3_fe->fe_unit, "sap found id 0x%08x\n"), vpn_sap->vc_mpls_port_id));

    if (l3_fe->fe_my_modid == exit_modid || is_trunk) {
        status = _bcm_fe2000_destroy_vpn_sap_hw_resources(l3_fe,
                                                          l3_fe->fe_my_modid,
                                                          &vpn_sap);
        if  (status != BCM_E_NONE) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) in destroying egr resources for fte(0x%x)\n"),
                      bcm_errmsg(status), fte_idx));
            return status;
        }
    } else {
        /* since vpn sw state is always created, it must always be destroyed */
        status = _bcm_fe2000_free_mpls_vpn_sap(l3_fe, &vpn_sap);
    }

    return BCM_E_NONE;
}


STATIC int
_bcm_fe2000_map_set_mpls_vpn_fte(_fe2k_l3_fe_instance_t  *l3_fe,
                                 _fe2k_vpn_control_t     *vpnc,
                                 _fe2k_vpn_sap_t         *vpn_sap,
                                 bcm_mpls_port_t         *mpls_port)
{
    int rv = BCM_E_UNAVAIL;

    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv =  _bcm_fe2000_g2p3_mpls_map_set_mpls_vpn_fte(l3_fe, vpnc,
                                                         vpn_sap, mpls_port);
        break;

    default:
        SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;
}


int
_bcm_fe2000_mpls_free_vpn_sap_hw_resources(_fe2k_l3_fe_instance_t  *l3_fe,
                                           _fe2k_vpn_sap_t        *vpn_sap)
{
    int  last_error_status;

    last_error_status = BCM_E_NONE;

    if (vpn_sap->vc_ohi.ohi != _FE2K_INVALID_OHI) {

        /* de-allocated only if owned but still unlink from vpn db 
         */
        if (vpn_sap->vc_res_alloced) {
            last_error_status = _sbx_gu2_resource_free(l3_fe->fe_unit,
                                                       SBX_GU2K_USR_RES_OHI,
                                                       1,
                                                       &vpn_sap->vc_ohi.ohi,
                                                       0);
            if (last_error_status != BCM_E_NONE) {
                MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) freeing ohi(0x%x)\n"),
                          bcm_errmsg(last_error_status),
                          vpn_sap->vc_ohi.ohi));
            }
        }

        DQ_REMOVE(&vpn_sap->vc_ohi_link);
    }

    /* deallocate etes, even when exteranlly managed; only the OHIs are actually
     * managed exteranlly.  The SDK still manages the ETEs & LPs
     */
    if (vpn_sap->vc_ete_hw_idx.ete_idx != _FE2K_INVALID_ETE_IDX) {

        if (vpn_sap->vc_res_alloced) {
            last_error_status = _sbx_gu2_resource_free(l3_fe->fe_unit,
                                                       SBX_GU2K_USR_RES_ETE_ENCAP,
                                                       1,
                                                       &vpn_sap->vc_ete_hw_idx.ete_idx,
                                                       0);
            if (last_error_status != BCM_E_NONE) {
                MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) freeing encap-ete(0x%x)\n"),
                          bcm_errmsg(last_error_status),
                          vpn_sap->vc_ete_hw_idx.ete_idx));
            }
        }
    }

    if (vpn_sap->vc_l2ete_hw_idx.ete_idx != _FE2K_INVALID_ETE_IDX) {
        if (vpn_sap->vc_res_alloced && !vpn_sap->l2ete_res_shared) {
            last_error_status = _sbx_gu2_resource_free(l3_fe->fe_unit,
                                                       SBX_GU2K_USR_RES_ETE_L2,
                                                       1,
                                                       &vpn_sap->vc_l2ete_hw_idx.ete_idx,
                                                       0);
            if (last_error_status != BCM_E_NONE) {
                MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) freeing l2-ete(0x%x)\n"),
                          bcm_errmsg(last_error_status),
                          vpn_sap->vc_l2ete_hw_idx.ete_idx));
            }
        }
    }

    vpn_sap->vc_l2ete_hw_idx.ete_idx = _FE2K_INVALID_ETE_IDX;
    vpn_sap->vc_ete_hw_idx.ete_idx   = _FE2K_INVALID_ETE_IDX;
    vpn_sap->vc_ohi.ohi              = _FE2K_INVALID_OHI;

    return last_error_status;
}


int
_bcm_fe2000_mpls_alloc_vpn_sap_hw_resources(_fe2k_l3_fe_instance_t  *l3_fe,
                                            _fe2k_vpn_sap_t         *vpn_sap,
                                            bcm_mpls_port_t         *mpls_port)
{
    int                           hidx, status = BCM_E_NONE;
    int                           ignore_status;

    /* is an OHI needed - */
    if (SOC_SBX_IS_VALID_ENCAP_ID(mpls_port->encap_id)) {
        vpn_sap->vc_ohi.ohi = SOC_SBX_OHI_FROM_ENCAP_ID(mpls_port->encap_id);
    } else if (SOC_SBX_IS_VALID_L2_ENCAP_ID(mpls_port->encap_id)) {
        vpn_sap->vc_ohi.ohi = 
            SOC_SBX_OHI_FROM_L2_ENCAP_ID(mpls_port->encap_id);
    }

    if (!mpls_port->encap_id) {
        status = _sbx_gu2_resource_alloc(l3_fe->fe_unit,
                                         SBX_GU2K_USR_RES_OHI,
                                         1,
                                         &vpn_sap->vc_ohi.ohi,
                                         0);

        if (status != BCM_E_NONE) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "failure(%s) allocating ohi\n"), bcm_errmsg(status)));
            goto error_out;
        }
    }
    
    /* ensure the externally managed encap id is reserved */
    if (mpls_port->flags & BCM_MPLS_PORT_ENCAP_WITH_ID) {
        status = _sbx_gu2_resource_alloc(l3_fe->fe_unit,
                                         SBX_GU2K_USR_RES_OHI,
                                         1,
                                         &vpn_sap->vc_ohi.ohi,
                                         _SBX_GU2_RES_FLAGS_RESERVE);
        if (status != BCM_E_RESOURCE) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "encapId 0x%08x passed as reserved, but is "
                              "not allocated\n"), mpls_port->encap_id));
        } else {
            status = BCM_E_NONE;
        }
    }

    /* Allocate ETE Encap */
    status = _sbx_gu2_resource_alloc(l3_fe->fe_unit,
                                     SBX_GU2K_USR_RES_ETE_ENCAP,
                                     1,
                                     &vpn_sap->vc_ete_hw_idx.ete_idx,
                                     0);

    if (status != BCM_E_NONE) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "failure(%s) allocating mpls-encap ete\n"),
                  bcm_errmsg(status)));
        goto error_out;
    }

    /* is an l2 ete needed - */
    if (!mpls_port->encap_id || 
        mpls_port->flags & BCM_MPLS_PORT_ENCAP_WITH_ID) 
    {
        /* 
         * TODO:
         * L2 ETE Sharing:
         *   In case of MPLS PWE ports (identified by MATCH_LABEL
         *   l2 ete is shared with the tunnel l2 ete.
         *   If the drop flag is set, there is a possiblity that drop is not set on tunnel
         *   So we dont share the l2ete in that case!
         *|
        if ((mpls_port->criteria == BCM_MPLS_PORT_MATCH_LABEL) && !(mpls_port->flags & BCM_MPLS_PORT_DROP)) {
            vpn_sap->l2ete_res_shared = 1;
        } else */
        {
        
            status = _sbx_gu2_resource_alloc(l3_fe->fe_unit,
                                             SBX_GU2K_USR_RES_ETE_L2,
                                             1,
                                             &vpn_sap->vc_l2ete_hw_idx.ete_idx,
                                             0);

            if (status != BCM_E_NONE) {
                MPLS_ERR((_SBX_D(l3_fe->fe_unit, "failure(%s) allocating L2 ete\n"),
                          bcm_errmsg(status)));
                goto error_out;
            }
        }
    }

    MPLS_VERB((_SBX_D(l3_fe->fe_unit, "vpn(0x%x) sap(0x%x) having "
               "ohi(0x%x) vc-ete(0x%x) vc-l2ete(0x%x) allocated\n"),
               vpn_sap->vc_vpnc->vpn_id, vpn_sap->vc_mpls_port_id,
               vpn_sap->vc_ohi.ohi,
               vpn_sap->vc_ete_hw_idx.ete_idx,
               vpn_sap->vc_l2ete_hw_idx.ete_idx));

    /**
     * OHI is always reqd on vpn_sap if we are exiting out of
     * fe_my_modid
     */
    hidx = _FE2K_GET_OHI2ETE_HASH_IDX(vpn_sap->vc_ohi.ohi);
    DQ_INSERT_HEAD(&l3_fe->fe_ohi2_vc_ete[hidx],
                   &vpn_sap->vc_ohi_link);
    MPLS_VERB((_SBX_D(l3_fe->fe_unit, "Insert ohi=0x%08x into sw hash idx=%d\n"),
               vpn_sap->vc_ohi.ohi, hidx));

    mpls_port->encap_id = SOC_SBX_ENCAP_ID_FROM_OHI(vpn_sap->vc_ohi.ohi);
    return BCM_E_NONE;

error_out:
    if (!mpls_port->encap_id || \
        _BCM_FE2000_IS_PWE3_TUNNEL(mpls_port->flags)) {
        ignore_status = _bcm_fe2000_mpls_free_vpn_sap_hw_resources(l3_fe,
                                                                   vpn_sap);
    }

    return status;
}

int
_bcm_fe2000_match_port2etc(_fe2k_l3_fe_instance_t  *l3_fe,
                           int                      action,
                           int                      logicalPort,
                           _fe2k_vpn_control_t     *vpnc,
                           _fe2k_vpn_sap_t         *vpn_sap,
                           bcm_mpls_port_t         *mpls_port)
{
    int rv = BCM_E_UNAVAIL;

    if ((action != _FE2K_MPLS_PORT_MATCH_ADD) &&
        (mpls_port->port != vpn_sap->vc_mpls_port.port)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "cannot change mod-port or match criteria, user "
                         "needs to delete and then re-add\n")));
        return BCM_E_PARAM;
    }

    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_mpls_match_port2etc(l3_fe, action, logicalPort,
                                                  vpnc, vpn_sap, mpls_port);
        break;

    default:
        SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;
}

STATIC int
_bcm_fe2000_match_pvid2etc(_fe2k_l3_fe_instance_t  *l3_fe,
                           int                      action,
                           uint32_t                 logicalPort,
                           _fe2k_vpn_control_t     *vpnc,
                           _fe2k_vpn_sap_t         *vpn_sap,
                           bcm_mpls_port_t         *mpls_port)
{
    int rv = BCM_E_UNAVAIL;

    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_mpls_match_pvid2etc(l3_fe, action, logicalPort,
                                                  vpnc, vpn_sap, mpls_port);
        break;

    default:
        SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;
}

int
_bcm_fe2000_match_pstackedvid2etc(_fe2k_l3_fe_instance_t  *l3_fe,
                                  int                      action,
                                  uint32_t                 logicalPort,
                                  _fe2k_vpn_control_t     *vpnc,
                                  _fe2k_vpn_sap_t         *vpn_sap,
                                  bcm_mpls_port_t         *mpls_port)
{

    int rv = BCM_E_UNAVAIL;


    if ((action != _FE2K_MPLS_PORT_MATCH_ADD) &&
        ((mpls_port->port != vpn_sap->vc_mpls_port.port) ||
         (mpls_port->match_vlan != vpn_sap->vc_mpls_port.match_vlan) ||
         (mpls_port->match_inner_vlan != vpn_sap->vc_mpls_port.match_inner_vlan))) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "cannot change mod-port or match criteria, user "
                         "needs to delete and then re-add\n")));
        return BCM_E_PARAM;
    }

    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_mpls_match_pstackedvid2etc(l3_fe, action,
                                                         logicalPort,
                                                         vpnc, vpn_sap,
                                                         mpls_port);
        break;

    default:
        SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;
}



int
_bcm_fe2000_match_label2etc(_fe2k_l3_fe_instance_t  *l3_fe,
                            int                      action,
                            uint32_t                 logicalPort,
                            _fe2k_vpn_control_t     *vpnc,
                            _fe2k_vpn_sap_t         *vpn_sap,
                            bcm_mpls_port_t         *mpls_port)
{
    int                           status = BCM_E_NONE;
    int                           vlan;
    uint32                        fteIdx=0;
    soc_sbx_g2p3_l2e_t            new_p3l2e, old_p3l2e;
    soc_sbx_g2p3_label2e_t        ilm;

    if(action < _FE2K_MPLS_PORT_MATCH_ADD ||
       action >= _FE2K_MPLS_PORT_MATCH_MAX) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) Bad Action\n")));
        return BCM_E_PARAM;
    }

    soc_sbx_g2p3_label2e_t_init(&ilm);
    soc_sbx_g2p3_l2e_t_init(&new_p3l2e);

    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {
    case SOC_SBX_UCODE_TYPE_G2P3:

        
        if((_FE2K_MPLS_PORT_MATCH_DELETE == action) ||
           (_FE2K_MPLS_PORT_MATCH_UPDATE == action)) {

            if( SOC_SBX_CFG(l3_fe->fe_unit)->mplstp_ena ==1) {
                status = soc_sbx_g2p3_label2e_get(l3_fe->fe_unit, 
                                                  _FE2K_MPLSTP_LABEL(mpls_port->match_label),
                                                  &ilm);

            } else {

                status = soc_sbx_g2p3_l2e_get(l3_fe->fe_unit, 
                                              _FE2K_MPLS_LABEL(mpls_port->match_label),
                                              &new_p3l2e);
            }
            if (BCM_FAILURE(status)) {
                MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) reading g2p3 l2e(0x%x)\n"),
                          bcm_errmsg(status),
                          mpls_port->match_label));
                return status;
            }
        }

        if (action == _FE2K_MPLS_PORT_MATCH_DELETE) {
            /* update lpi in software state */
            vpn_sap->logicalPort = 0;
            if (vpn_sap->vc_mpls_port.match_label != mpls_port->match_label) {
                return BCM_E_PARAM;
            }
            if( SOC_SBX_CFG(l3_fe->fe_unit)->mplstp_ena) {
                soc_sbx_g2p3_label2e_delete(l3_fe->fe_unit, _FE2K_MPLSTP_LABEL(mpls_port->match_label));
            } else {
                soc_sbx_g2p3_l2e_t_init(&new_p3l2e);
            }
            
        } else {

            
            if (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) {
                vlan  = (BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id) -
                                   l3_fe->vlan_ft_base);
                fteIdx = BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id);
            } else {
                vlan = vpnc->vpn_id;
            }
            
	    if( SOC_SBX_CFG(l3_fe->fe_unit)->mplstp_ena) {
                
                ilm.pipe     = 1; 
                ilm.vlan     = vlan;
                if (mpls_port->flags & BCM_MPLS_PORT_INT_PRI_MAP) {
                    ilm.elsp = 1;
                }
                if (mpls_port->flags & BCM_MPLS_PORT_INT_PRI_SET) {
                    ilm.elsp = 0;
                    ilm.cos = mpls_port->int_pri;
                }

                if (_BCM_FE2000_IS_PWE3_TUNNEL(mpls_port->flags)) {
                    if (mpls_port->flags & BCM_MPLS_PORT_EGRESS_TUNNEL) {
                        ilm.opcode  = _BCM_FE2K_LABEL_PWE(l3_fe->fe_unit);

                    } else if (mpls_port->flags & BCM_MPLS_PORT_NO_EGRESS_TUNNEL_ENCAP) {
                        ilm.opcode  = _BCM_FE2K_LABEL_CES(l3_fe->fe_unit);

                    } else {
                        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Bad Tunnel Flag 0x%x\n"),
                                  mpls_port->flags));
                        return status;
                    }
                } else {
                    /* only CES PW handoff AC is supported currently, they are faked
                     * as LSR */
                    ilm.opcode = _BCM_FE2K_LABEL_LSR(l3_fe->fe_unit);
                }

                if (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) {
                    ilm.vlan   = 0;
                    ilm.ftidx1 = fteIdx >> _MPLSTP_ILM_FT_SHIFT;
                    ilm.ftidx0 = fteIdx & ((1<<_MPLSTP_ILM_FT_SHIFT) -1);
                    ilm.vpws   = 1;
                }

                
 
            } else {
                new_p3l2e.stpstate = 0; 
                new_p3l2e. elsp    = 0x0;
                new_p3l2e.pipe     = 1; 
                new_p3l2e.ftidx    = 0x0; 
                new_p3l2e.oamen    = 0x0;
                new_p3l2e.leren    = 0x0;
                new_p3l2e.vlan     = vlan;
            }
            
#if 0
            ilm.ulQosProfile = _MPLS_EXPMAP_HANDLE_DATA(mpls_port->exp_map);

            if (mpls_port->flags & BCM_MPLS_PORT_NETWORK) {
                ilm.ulVplsColor  = _FE2K_VPLS_COLOR_WAN;
            } else {
                ilm.ulVplsColor  = _FE2K_VPLS_COLOR_LAN;
            }
#endif
	    status = _bcm_fe2000_g2p3_mpls_lp_write(l3_fe->fe_unit, logicalPort,
                                                vpn_sap, mpls_port);
	    if (BCM_FAILURE(status)) {
	        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "failed to write logical port 0x%x: %d (%s)\n"),
		    logicalPort, status, bcm_errmsg(status)));
	        return status;
	    }
        
           if( SOC_SBX_CFG(l3_fe->fe_unit)->mplstp_ena ==1) {
               ilm.lpidx = logicalPort;

               /* update lpi in software state */
               vpn_sap->logicalPort  = logicalPort;

               if (action == _FE2K_MPLS_PORT_MATCH_UPDATE) {
                   status = soc_sbx_g2p3_label2e_update(l3_fe->fe_unit,
                                                      _FE2K_MPLSTP_LABEL(mpls_port->match_label),
                                                       &ilm);

                   if (vpn_sap->vc_mpls_port.match_label != mpls_port->match_label) {

                       /* To avoid traffic loss during update, update old label after new label changes
                        * are commited */
                          if( SOC_SBX_CFG(l3_fe->fe_unit)->mplstp_ena ==1) {
                           /* delete old entry */
                           soc_sbx_g2p3_label2e_delete(l3_fe->fe_unit, 
                                              _FE2K_MPLSTP_LABEL(vpn_sap->vc_mpls_port.match_label));

                       } else {
                           soc_sbx_g2p3_l2e_t_init(&old_p3l2e);
                           status = soc_sbx_g2p3_l2e_set(l3_fe->fe_unit,
                                              _FE2K_MPLS_LABEL(vpn_sap->vc_mpls_port.match_label),
                                              &old_p3l2e);
                       }
                       if (BCM_FAILURE(status)) {
                           MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) invalidating "
                                     "using soc_sbx_g2p3_l2e_set "
                                     "for old label(0x%x)\n"),
                                     bcm_errmsg(status),
                                     vpn_sap->vc_mpls_port.match_label));
                           return status;
                       }
                   }
                } else {
     
                 status = soc_sbx_g2p3_label2e_set(l3_fe->fe_unit,
                                              _FE2K_MPLSTP_LABEL(mpls_port->match_label),
                                               &ilm);
                }
            } else {

                status = soc_sbx_g2p3_l2e_set(l3_fe->fe_unit,
                                      _FE2K_MPLS_LABEL(mpls_port->match_label),
                                      &new_p3l2e);
            }
            if (BCM_FAILURE(status)) {
                MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) calling soc_sbx_g2p3_l2e_set "
                             "for label(0x%x)\n"),
                      bcm_errmsg(status),
                      mpls_port->match_label));
                return status;
            }
        }
        break;

    default:
        SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
        status = BCM_E_INTERNAL;
        break;
    }

    return BCM_E_NONE;
}

STATIC int
_bcm_fe2000_alloc_mpls_vpn_sap(_fe2k_l3_fe_instance_t  *l3_fe,
                               _fe2k_vpn_control_t   *vpnc,
                               bcm_gport_t            mpls_port_id,
                               _fe2k_vpn_sap_t      **vpn_sap)
{
    int                           size;
    _fe2k_vc_ete_fte_t           *vc_ete_fte;
    uint32                        fte_idx;

    *vpn_sap = sal_alloc(sizeof(_fe2k_vpn_sap_t),
                         "MPLS-vpn-sap");
    if (*vpn_sap == NULL) {
        return BCM_E_MEMORY;
    }
    size  = sizeof(_fe2k_vc_ete_fte_t) * _FE2K_ETE_USER_SLAB_SIZE;
    vc_ete_fte = sal_alloc(size, "MPLS-ete-fte");
    if (vc_ete_fte == NULL) {
        sal_free(*vpn_sap);
        *vpn_sap = NULL;
        return BCM_E_MEMORY;
    }

    sal_memset(*vpn_sap, 0, sizeof(_fe2k_vpn_sap_t));
    sal_memset(vc_ete_fte, 0, size);

    (*vpn_sap)->vc_vpnc       = vpnc;
    (*vpn_sap)->u.vc_fte      = vc_ete_fte;
    (*vpn_sap)->vc_alloced_ue = _FE2K_ETE_USER_SLAB_SIZE;
    (*vpn_sap)->vc_inuse_ue   = 0;

    /* These are reserved upfront */
    fte_idx = BCM_GPORT_MPLS_PORT_ID_GET(mpls_port_id);
    (*vpn_sap)->vc_ohi.ohi              = _FE2K_INVALID_OHI;
    (*vpn_sap)->vc_ete_hw_idx.ete_idx   = _FE2K_INVALID_ETE_IDX;
    (*vpn_sap)->vc_l2ete_hw_idx.ete_idx = _FE2K_INVALID_ETE_IDX;

    DQ_INSERT_HEAD(&vpnc->vpn_sap_head,
                   &(*vpn_sap)->vc_vpn_sap_link);
    DQ_INIT(&(*vpn_sap)->vc_mpls_ete_link);
    DQ_INIT(&(*vpn_sap)->vc_ohi_link);

    (*vpn_sap)->mpls_psn_label     = BCM_MPLS_LABEL_INVALID;
    (*vpn_sap)->mpls_psn_label_exp = 0;
    (*vpn_sap)->mpls_psn_label_ttl = 0;


    MPLS_VERB((_SBX_D(l3_fe->fe_unit, "vpn(0x%x) allocated fte(0x%x), ohi(0x%x), ete(0x%x) l2-ete(0x%x)\n"),
               vpnc->vpn_id, fte_idx,
               (*vpn_sap)->vc_ohi.ohi,
               (*vpn_sap)->vc_ete_hw_idx.ete_idx,
               (*vpn_sap)->vc_l2ete_hw_idx.ete_idx));

    return BCM_E_NONE;
}

STATIC int
_bcm_fe2000_free_mpls_vpn_sap(_fe2k_l3_fe_instance_t  *l3_fe,
                              _fe2k_vpn_sap_t      **vpn_sap)
{
    int  rv;

    sal_free((*vpn_sap)->u.vc_fte);
    DQ_REMOVE(&(*vpn_sap)->vc_mpls_ete_link);
    DQ_REMOVE(&(*vpn_sap)->vc_vpn_sap_link);

    rv = _bcm_fe2000_mpls_free_vpn_sap_hw_resources(l3_fe, *vpn_sap);
    if (BCM_FAILURE(rv)) {
        MPLS_WARN((_SBX_D(l3_fe->fe_unit, "Error freeing vpn sap hw resources: %d %s\n"),
                   rv, bcm_errmsg(rv)));
    }

    sal_free(*vpn_sap);
    *vpn_sap = NULL;

    return BCM_E_NONE;
}

int
_bcm_fe2000_add_mpls_vpn_id(int                     unit,
                            _fe2k_l3_fe_instance_t *l3_fe,
                            bcm_mpls_vpn_config_t  *info)
{
    _fe2k_vpn_control_t          *vpnc;
    int                           status, hidx;
    int                           res_flags;
    uint32                        alloc_vrf, alloc_vsi, alloc_vpls_color=0;

    vpnc      = NULL;
    alloc_vrf = _FE2K_INVALID_VRF;
    alloc_vsi = _FE2K_INVALID_VPN_ID;
    res_flags = 0;

    if (info->flags & BCM_MPLS_VPN_L3) {

        if (info->lookup_id) {
            alloc_vrf = info->lookup_id;
            res_flags = _SBX_GU2_RES_FLAGS_RESERVE;
        } else {
            res_flags = 0;
        }

        status = _sbx_gu2_resource_alloc(l3_fe->fe_unit,
                                         SBX_GU2K_USR_RES_VRF,
                                         1,
                                         &alloc_vrf,
                                         res_flags);
        if (status != BCM_E_NONE) {
            alloc_vrf = _FE2K_INVALID_VRF;
            goto error_out;
        }
    }

    /* VSI is vpn-id for L3, VPLS; no VSI allocated for VPWS */
    if (info->flags & BCM_MPLS_VPN_WITH_ID) {
        alloc_vsi = info->vpn;
        res_flags = _SBX_GU2_RES_FLAGS_RESERVE;
    } else {
        res_flags = 0;
    }

    if (info->flags & BCM_MPLS_VPN_VPWS) {
        status = _sbx_gu2_resource_alloc(l3_fe->fe_unit,
                                         SBX_GU2K_USR_RES_LINE_VSI,
                                         1,
                                         &alloc_vsi,
                                         res_flags);
    } else {
        status = _sbx_gu2_resource_alloc(l3_fe->fe_unit,
                                         SBX_GU2K_USR_RES_VSI,
                                         1,
                                         &alloc_vsi,
                                         res_flags);
    }

    /* if this VPN is in the reserved range, convert the error code to NONE */
    if (status == BCM_E_RESOURCE) {
        MPLS_VERB((_SBX_D(l3_fe->fe_unit, "vpnId 0x%04x found to be in the "
                          "reserved range, OK.\n"), alloc_vsi));
        status = BCM_E_NONE;
    }
    if (info->flags & BCM_MPLS_VPN_VPLS) {
         status = _sbx_gu2_resource_alloc(l3_fe->fe_unit,
                                          SBX_GU2K_USR_RES_VPLS_COLOR,
                                          1,
                                          &alloc_vpls_color,
                                          res_flags);

        if (status == BCM_E_RESOURCE) {
            MPLS_ERR ((_SBX_D(l3_fe->fe_unit, "VPLS Color resource unavailable\n")));
            goto error_out;
        }
     }

    if (status != BCM_E_NONE) {
        alloc_vsi = _FE2K_INVALID_VPN_ID;
        goto error_out;
    }

    status = _bcm_fe2000_mpls_alloc_vpncb(l3_fe, &vpnc);
    if (status != BCM_E_NONE) {
        goto error_out;
    }

    vpnc->vpn_bc_mcg = info->broadcast_group;
    vpnc->vpn_id     = alloc_vsi;
    vpnc->vpn_vrf    = alloc_vrf;
    vpnc->vpn_flags  = info->flags;
    vpnc->vpls_color = alloc_vpls_color;
    hidx = _FE2K_GET_MPLS_VPN_HASH(vpnc->vpn_id);
    DQ_INSERT_HEAD(&l3_fe->fe_vpn_hash[hidx], &vpnc->vpn_fe_link);

    if (!(info->flags & BCM_MPLS_VPN_VPWS)) {
        status = _bcm_fe2000_map_set_vpn_bc_fte(l3_fe, vpnc);
        if (status != BCM_E_NONE) {
            goto error_out;
        }

        status = _bcm_fe2000_mpls_map_set_vlan2etc(l3_fe,
                                                   vpnc,
                                                   vpnc->vpn_id);
        if (status != BCM_E_NONE) {
            goto error_out;
        }
    }

    info->vpn       = vpnc->vpn_id;

    if (_BCM_VRF_VALID(vpnc->vpn_vrf)) {
        l3_fe->fe_vpn_by_vrf[vpnc->vpn_vrf] = vpnc;
    }

    return BCM_E_NONE;

error_out:
    if (vpnc) {
        _bcm_fe2000_mpls_free_vpncb(l3_fe, &vpnc);
    } else {
        if (alloc_vrf != _FE2K_INVALID_VRF) {
            _sbx_gu2_resource_free(l3_fe->fe_unit,
                                   SBX_GU2K_USR_RES_VRF,
                                   1,
                                   &alloc_vrf,
                                   0);
        }

        if (alloc_vsi != _FE2K_INVALID_VPN_ID) {
            if (info->flags & BCM_MPLS_VPN_VPWS) {
                _sbx_gu2_resource_free(l3_fe->fe_unit,
                                       SBX_GU2K_USR_RES_LINE_VSI,
                                       1,
                                       &alloc_vsi,
                                       0);
            } else {
                _sbx_gu2_resource_free(l3_fe->fe_unit,
                                       SBX_GU2K_USR_RES_VSI,
                                       1,
                                       &alloc_vsi,
                                       0);
            }
        }
    }

    return status;
}

int
_bcm_fe2000_destroy_all_mpls_vpn_id(int                      unit,
                                    _fe2k_l3_fe_instance_t  *l3_fe)
{
    int                           hidx, status;
    int                           last_error_status;
    _fe2k_vpn_control_t          *vpnc_temp = NULL;

    /**
     * Check to see if all the sap on each of the vpn
     * is deleted already.
     */
    for (hidx = 0; hidx < _FE2K_VPN_HASH_SIZE; hidx++) {
        _FE2K_ALL_VPNC_PER_BKT(l3_fe, hidx, vpnc_temp) {

            if (!DQ_EMPTY(&vpnc_temp->vpn_sap_head)) {
                return BCM_E_BUSY;
            }

        } _FE2K_ALL_VPNC_PER_BKT_END(l3_fe, hidx, vpnc_temp);
    }

    last_error_status = BCM_E_NONE;

    /**
     * Actual deletion of the vpn
     */
    for (hidx = 0; hidx < _FE2K_VPN_HASH_SIZE; hidx++) {
        _FE2K_ALL_VPNC_PER_BKT(l3_fe, hidx, vpnc_temp) {

            status =
                _bcm_fe2000_destroy_mpls_vpn_id(unit,
                                                l3_fe,
                                                &vpnc_temp);
            if (status != BCM_E_NONE) {
                last_error_status = status;
            }

        } _FE2K_ALL_VPNC_PER_BKT_END(l3_fe, hidx, vpnc_temp);
    }

    return last_error_status;
}

int
_bcm_fe2000_validate_mpls_vpn_id_create(int                    unit,
                                        bcm_mpls_vpn_config_t *info)
{
    if (!_BCM_MPLS_VPN_VALID_TYPE(info->flags)) {
        return BCM_E_PARAM;
    }

    if ((info->flags & (BCM_MPLS_VPN_WITH_ID | BCM_MPLS_VPN_REPLACE)) &&
        !_BCM_MPLS_VPN_VALID_USER_HANDLE(unit, info->vpn)) {
        return BCM_E_PARAM;
    }

    /**
     * GU2K does not currently support more than one class for
     * broadcast on L2 domain.
     */
    if ((info->broadcast_group != info->unknown_unicast_group) ||
        (info->unknown_unicast_group != info->unknown_multicast_group)) {
        return BCM_E_PARAM;
    }

    if (info->flags & BCM_MPLS_VPN_L3) {
        /* L3-vpn and VPWS do not need l2 bcast */
        if (info->broadcast_group       ||
            info->unknown_unicast_group ||
            info->unknown_multicast_group) {
            return BCM_E_PARAM;
        }
    } else if (info->flags & BCM_MPLS_VPN_VPWS) {
        /* L3-vpn and VPWS do not need l2 bcast */
        if (info->broadcast_group       ||
            info->unknown_unicast_group ||
            info->unknown_multicast_group) {
            return BCM_E_PARAM;
        }
        if (info->lookup_id) {
            return BCM_E_PARAM;
        }
    } else if (info->flags & BCM_MPLS_VPN_VPLS) {
        if (!info->broadcast_group       ||
            !info->unknown_unicast_group ||
            !info->unknown_multicast_group) {
            return BCM_E_PARAM;
        }
        if (info->lookup_id) {
            return BCM_E_PARAM;
        }
    } else {
        return BCM_E_PARAM;
    }


    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_fe2000_validate_mpls_port_add
 * Purpose:
 *      Validate parameters for adding port to vpn
 * Parameters:
 *      l3_fe       - l3 fe instance
 *      vpn         - vpn identifier
 *      mpls_port   - port parameter
 * Returns:
 *      BCM_E_XXX
 * Note:
 *
 */

STATIC int
_bcm_fe2000_validate_mpls_port_add(_fe2k_l3_fe_instance_t *l3_fe,
                                   bcm_vpn_t               vpn,
                                   bcm_mpls_port_t        *mpls_port)
{

    MPLS_VVERB((_SBX_D(l3_fe->fe_unit, "Enter\n")));
    if (mpls_port == NULL) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls_port param is NULL\n")));
        return BCM_E_PARAM;
    }

    if (!_BCM_MPLS_VPN_VALID_USER_HANDLE(l3_fe->fe_unit, vpn)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "invalid VPN (0x%x)\n"), vpn));
        return BCM_E_PARAM;
    }

    /**
     * YYY: TBD:
     * Since this port specifies the Egress port and in
     * some cases the Ingress port, it has to be of type
     * MOD-PORT.
     */
    if (!BCM_GPORT_IS_SET(mpls_port->port) ||
        !(BCM_GPORT_IS_MODPORT(mpls_port->port) ||
          BCM_GPORT_IS_TRUNK(mpls_port->port))) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Unexpected port (0x%x); must be GPORT_MODPORT\n"),
                  mpls_port->port));
        return BCM_E_PARAM;
    }

    if (mpls_port->flags & ~_BCM_MPLS_PORT_SUPPORTED_FLAGS) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Invalid flag.  Supported=0x%x Passed=0x%x\n"),
                  _BCM_MPLS_PORT_SUPPORTED_FLAGS, mpls_port->flags));
        return BCM_E_PARAM;
    }

    /* MPLS-TP only supports Pseudowires with CW option 
       For VPXS, CW will be added at the entry PSN tunnel and 
       at the egress-PE the CW will be deleted before switching
       the packet*/

     if(SOC_SBX_CFG(l3_fe->fe_unit)->mplstp_ena && 
        ((mpls_port->flags & BCM_MPLS_PORT_NETWORK) &&
         !(mpls_port->flags & BCM_MPLS_PORT_CONTROL_WORD))){
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "MPLS-TP supports PW only with CW\n")));
        return BCM_E_PARAM;
     }
      

    /**
     * XXX: TBD: Fix me!
     */
    if ((mpls_port->service_tpid != 0) && 
             (mpls_port->service_tpid != 0x88a8) &&
             (mpls_port->service_tpid != 0x9100) &&
             (mpls_port->service_tpid != 0x8100)) {
         MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Invalid service_tpid param \n")));
         return BCM_E_PARAM;
    }

    if ((mpls_port->flags & BCM_MPLS_PORT_SERVICE_VLAN_ADD) &&
        (mpls_port->egress_service_vlan >= BCM_VLAN_INVALID)) {
         MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Invalid service vlan param \n")));
         return BCM_E_PARAM;        
    }

    /* egress tunnel is used for Ethernet transport & tunnel without encap flag is 
     * used for PW handoff, the flags are mutually exclusive */
    if((mpls_port->flags & BCM_MPLS_PORT_EGRESS_TUNNEL) &&
       (mpls_port->flags & BCM_MPLS_PORT_NO_EGRESS_TUNNEL_ENCAP)) {
         MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Invalid Egress Tunnel Flags \n")));
         return BCM_E_PARAM;
    }


    if(_BCM_FE2000_IS_PWE3_TUNNEL(mpls_port->flags) && 
       !_FE2K_L3_FTE_VALID(l3_fe->fe_unit, mpls_port->egress_tunnel_if)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Egress tunnel requested with invalid "
                         " egress tunnel interface (0x%x)\n"),
                  mpls_port->egress_tunnel_if));
        return BCM_E_PARAM;
    }


    if (((mpls_port->flags & BCM_MPLS_PORT_INT_PRI_MAP) ||
          (mpls_port->flags & BCM_MPLS_PORT_INT_PRI_SET)) &&
         (!_MPLS_EXPMAP_HANDLE_IS_INGR(mpls_port->exp_map))) {
        /* The exp_map is enabled and bogus */
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls_port has invalid exp_map %08x\n"),
                  mpls_port->exp_map));
        return BCM_E_PARAM;
    }

    if (((0 == (mpls_port->flags & BCM_MPLS_PORT_INT_PRI_MAP)) &&
         (0 == (mpls_port->flags & BCM_MPLS_PORT_INT_PRI_SET))) &&
         (mpls_port->exp_map)) {
        /* the exp_map is disabled and nonzero */
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls_port has invalid exp_map %08x\n"),
                  mpls_port->exp_map));
        return BCM_E_PARAM;
    }

    switch (mpls_port->criteria) {
    case BCM_MPLS_PORT_MATCH_PORT:
        /* XXX: TBD: Trunk ?? */
        break;

    case BCM_MPLS_PORT_MATCH_PORT_VLAN:
        /* XXX: TBD: Trunk ?? */
        if (mpls_port->match_vlan > BCM_VLAN_MAX) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Invalid vlan passed for MATCH_PORT_VLAN "
                             "vlan=0x%x\n"), mpls_port->match_vlan));

            return BCM_E_PARAM;
        }
        break;

    case BCM_MPLS_PORT_MATCH_PORT_VLAN_STACKED:
        if (!BCM_VLAN_VALID(mpls_port->match_vlan)) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Invalid outer vlan passed for "
                      "MATCH_PORT_VLAN_STACKED vlan=0x%x\n"),
                      mpls_port->match_vlan));

            return BCM_E_PARAM;
        }
        if (!BCM_VLAN_VALID(mpls_port->match_inner_vlan)) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Invalid inner vlan passed for "
                             "MATCH_PORT_VLAN_STACKED vlan=0x%x\n"),
                      mpls_port->match_inner_vlan));
            return BCM_E_PARAM;
        }
        break;

    case BCM_MPLS_PORT_MATCH_LABEL:
        if(SOC_SBX_MPLSTP_ENABLE(l3_fe->fe_unit)) {
            if (!_BCM_FE2000_MPLSTP_LABEL_VALID(mpls_port->match_label)) {
                MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Invalid label passed for "
                                 "MATCH_LABEL label=0x%x\n"),
                          mpls_port->match_label));

                return BCM_E_PARAM;
            }
        } else {
            if (!_BCM_LABEL_VALID(mpls_port->match_label)) {
                MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Invalid label passed for "
                                 "MATCH_LABEL label=0x%x\n"),
                          mpls_port->match_label));

                return BCM_E_PARAM;
            }
        }
        break;

    case BCM_MPLS_PORT_MATCH_COUNT:
    case BCM_MPLS_PORT_MATCH_INVALID:
    case BCM_MPLS_PORT_MATCH_NONE:
    case BCM_MPLS_PORT_MATCH_LABEL_PORT:
    case BCM_MPLS_PORT_MATCH_LABEL_VLAN:
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Invalid Match criteria=%d\n"),
                  mpls_port->criteria));

        return BCM_E_PARAM;
    default:
        return BCM_E_PARAM;
    }


    /*
     * If failover port id is supplied, check for valid port id 
     */
    if (mpls_port->failover_port_id) {
        if (BCM_GPORT_IS_MPLS_PORT(mpls_port->failover_port_id)) {
            uint32 failover_port_fte;

            failover_port_fte = 
                     BCM_GPORT_MPLS_PORT_ID_GET(mpls_port->failover_port_id);
            if (!_FE2K_MPLS_PORT_FTE_VALID(l3_fe->fe_unit, failover_port_fte)) {

                MPLS_ERR((_SBX_D(l3_fe->fe_unit, 
                                 "Invalid failover fte (0x%x)\n"),
                                 failover_port_fte));
                return BCM_E_PARAM;
            }
        } else {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, 
                             "Invalid failover port id (0x%x)\n"),
                             mpls_port->failover_port_id));
            return BCM_E_PARAM;
        }
    }

    /*
     * If failover id was supplied, check for valid
     * backup egress object
     */
    if (mpls_port->failover_id) {
        if (mpls_port->failover_id > 0 && mpls_port->failover_id < 1024) {
            if (!mpls_port->failover_port_id) {
                /* Validity of failover port id is already checked */
                MPLS_ERR((_SBX_D(l3_fe->fe_unit,
                                "Failover port id not supplied (0x%x)\n"),
                                 mpls_port->failover_port_id));
                return BCM_E_PARAM;
            }
        } else {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Invalid failover id (0x%x)\n"),
                      mpls_port->failover_id));
            return BCM_E_PARAM;
        }
    }

    /*
     * Validate encap id (or OHI)
     */
    if ( mpls_port->encap_id &&
         !(SOC_SBX_IS_VALID_ENCAP_ID(mpls_port->encap_id) ||
           SOC_SBX_IS_VALID_L2_ENCAP_ID(mpls_port->encap_id) )
        ) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls_port has invalid encap id %08x\n"),
                  mpls_port->encap_id));
        return BCM_E_PARAM;
    }

    /* XXX: TBD: other fields to be validated */

    return BCM_E_NONE;
}


STATIC int
_bcm_fe2000_mpls_port_update(int                     unit,
                             _fe2k_l3_fe_instance_t *l3_fe,
                             bcm_vpn_t               vpn_id,
                             _fe2k_vpn_control_t    *vpnc,
                             bcm_mpls_port_t        *mpls_port,
                             _fe2k_vpn_sap_t        *vpn_sap)
{
    int                           status;
    _fe2k_l3_fte_t                gport_fte;
    bcm_port_t                    new_exit_port;
    bcm_module_t                  new_exit_modid = -1, old_exit_modid = -1;
    _fe2k_l3_or_mpls_egress_t     gport_egr;
    uint32_t                      logicalPort = 0, is_trunk = 0;
    uint32_t                      is_failover = 0;
    uint8_t                       is_same_label = 0;
    _fe2k_vpn_sap_t               *vpn_sap_tmp = NULL;
    
    MPLS_VVERB((_SBX_D(l3_fe->fe_unit, "Enter\n")));

    if (BCM_GPORT_IS_TRUNK(mpls_port->port)) {
        is_trunk = 1;
    } else {

        status = _bcm_fe2000_mpls_gport_get_mod_port(l3_fe->fe_unit,
                                                     mpls_port->port,
                                                     &new_exit_modid,
                                                     &new_exit_port);
        if (status != BCM_E_NONE) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit,
                     "error(%s) extracting mod-port from gport (0x%x)\n"),
                     bcm_errmsg(status), mpls_port->port));
            return status;
        }
    }

    /**
     * Any match criteria change implies that the mpls-port
     * needs to be deleted and added as new addition.
     */
    if (vpn_sap->vc_mpls_port.criteria != mpls_port->criteria) {
        return BCM_E_PARAM;
    }

    gport_fte.fte_idx = BCM_GPORT_MPLS_PORT_ID_GET(mpls_port->mpls_port_id);
    
    /**
     * read the gport fte to get current (old) exit_modid
     */

    status = _bcm_fe2000_mpls_get_fte(l3_fe,
                                      gport_fte.fte_idx,
                                      L3_OR_MPLS_GET_FTE__FTE_CONTENTS_ONLY,
                                      &gport_egr);
    if (status != BCM_E_NONE) {
        MPLS_ERR((_SBX_D(unit, "error(%s) reading gport-fte(0x%x)\n"),
                  bcm_errmsg(status), gport_fte.fte_idx));
        return status;
    }

    old_exit_modid = gport_egr.fte_modid;

    /**
     * update the vpn-sap either locally or remote
     * for the new_exit_modid first
     */
    if (l3_fe->fe_my_modid == new_exit_modid || (is_trunk == 1)) {
        status = _bcm_fe2000_update_vpn_sap_hw(l3_fe,
                                               l3_fe->fe_my_modid,
                                               vpn_sap,
                                               mpls_port);
        if  (status != BCM_E_NONE) {
            MPLS_ERR((_SBX_D(unit, "error(%s) updating gport-fte(0x%x)\n"),
                      bcm_errmsg(status), gport_fte.fte_idx));
            return status;
        }
    }

    /**
     * Every scenario for updating the egress portion of GPORT-FTE
     * is done by now.
     * We update the GPORT-FTE here before destroy from old ete
     */
    status   = _bcm_fe2000_map_set_mpls_vpn_fte(l3_fe,
                                                vpnc,
                                                vpn_sap,
                                                mpls_port);
    if (status != BCM_E_NONE) {
        MPLS_ERR((_SBX_D(unit, "error(%s) map-set fte(0x%x)\n"),
                  bcm_errmsg(status), gport_fte.fte_idx));
        return status;
    }

    /**
     * Delete the vpn-sap from the old-exit-modid,
     * this is deliberately done even when the new and old
     * exit_modid is local. If old and new is same modid,
     * we just link and then unlink
     */
    if (l3_fe->fe_my_modid == old_exit_modid || (is_trunk == 1)) {
        status = _bcm_fe2000_destroy_vpn_sap_hw_resources(l3_fe,
                                                          l3_fe->fe_my_modid,
                                                          &vpn_sap);
        if  (status != BCM_E_NONE) {
            MPLS_ERR((_SBX_D(unit, "error(%s) destroying ohi-ete for fte(0x%x)\n"),
                      bcm_errmsg(status), gport_fte.fte_idx));
            return status;
        }
    }

    if (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype == SOC_SBX_UCODE_TYPE_G2P3) {

        if ((SOC_SBX_CFG(l3_fe->fe_unit)->mplstp_ena == 0) &&
            (mpls_port->criteria == BCM_MPLS_PORT_MATCH_LABEL) &&
            (is_trunk == 0)) {
            status = _bcm_fe2000_g2p3_mpls_lp_get(l3_fe->fe_unit, mpls_port,
                                                  &logicalPort);

            if (status == BCM_E_NOT_FOUND) {
                status = BCM_E_NONE;
            } else {
                if (BCM_FAILURE(status)) {
                    MPLS_ERR((_SBX_D(unit,
                             "failed to free logical port: %d (%s)\n"),
                             status, bcm_errmsg(status)));
                    return status;
                }
            }
        } else {
            logicalPort = vpn_sap->logicalPort;
        }
    }

    /**
     * Now we need to program the ingress portion
     * of mpls-port
     */
    switch (mpls_port->criteria) {
    case BCM_MPLS_PORT_MATCH_PORT:
        status = _bcm_fe2000_match_port2etc(l3_fe,
                                            _FE2K_MPLS_PORT_MATCH_UPDATE,
                                            logicalPort,
                                            vpnc,
                                            vpn_sap,
                                            mpls_port);
        if (status != BCM_E_NONE) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) updating port2etc\n"),
                      bcm_errmsg(status)));
            return status;
        }

        break;

    case BCM_MPLS_PORT_MATCH_PORT_VLAN:
        status = _bcm_fe2000_match_pvid2etc(l3_fe,
                                            _FE2K_MPLS_PORT_MATCH_UPDATE,
                                            logicalPort,
                                            vpnc,
                                            vpn_sap,
                                            mpls_port);
        if (status != BCM_E_NONE) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) updating pvid2etc\n"), 
                             bcm_errmsg(status)));
            return status;
        }
        break;

    case BCM_MPLS_PORT_MATCH_PORT_VLAN_STACKED:
        if (mpls_port->flags & BCM_MPLS_PORT_REPLACE) {
            return BCM_E_PARAM;
        }

        status = _bcm_fe2000_match_pstackedvid2etc(l3_fe,
                                                   _FE2K_MPLS_PORT_MATCH_UPDATE,
                                                   logicalPort,
                                                   vpnc,
                                                   vpn_sap,
                                                   mpls_port);
        if (status != BCM_E_NONE) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) updating pstackedvid2etc\n"),
                      bcm_errmsg(status)));
            return status;
        }
        break;

    case BCM_MPLS_PORT_MATCH_LABEL:
        is_failover = mpls_port->flags & BCM_MPLS_PORT_FAILOVER;
        status = _bcm_fe2000_find_vpn_sap_by_label(l3_fe,
                                            vpnc,
                                            mpls_port->match_label,
                                            &vpn_sap_tmp,
                                            (!is_failover));
        if (BCM_SUCCESS(status)) {
            /* port with same vc label exits */
            is_same_label = 1;
        }

        /* dont update label2e if same label and failover */
        if (!(is_same_label && is_failover ) ) {
            status = _bcm_fe2000_match_label2etc(l3_fe,
                                                 _FE2K_MPLS_PORT_MATCH_UPDATE,
                                                 logicalPort,
                                                 vpnc,
                                                 vpn_sap,
                                                 mpls_port);
            if (status != BCM_E_NONE) {
                MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) updating label2etc\n"),
                         bcm_errmsg(status)));
                return status;
            }
        }
        break;

    case BCM_MPLS_PORT_MATCH_LABEL_VLAN:
    case BCM_MPLS_PORT_MATCH_INVALID:
    case BCM_MPLS_PORT_MATCH_NONE:
    case BCM_MPLS_PORT_MATCH_LABEL_PORT:
    default:
        return BCM_E_PARAM;
    }

    /* update the user params in vpn_sap */
    vpn_sap->vc_mpls_port = *mpls_port;

    return BCM_E_NONE;
}


STATIC int
_bcm_fe2000_mpls_program_vpn_sap_vlan2etc(_fe2k_l3_fe_instance_t   *l3_fe,
                                          _fe2k_vpn_control_t      *vpnc,
                                          _fe2k_vpn_sap_t          *vpn_sap)
{

    int rv = BCM_E_UNAVAIL;

    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_mpls_program_vpn_sap_vlan2etc(l3_fe,
                                                            vpnc, vpn_sap);

        break;

    default:
        SBX_UNKNOWN_UCODE_WARN(l3_fe->fe_unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;

}



STATIC int
_bcm_fe2000_mpls_vpws_fte_connect(_fe2k_l3_fe_instance_t *l3_fe,
                                  _fe2k_vpn_sap_t* vpnSaps[_BCM_FE2K_VPWS_MAX_SAP],
                                  _fe2k_mpls_port_action_type action)
{
    int           rv = BCM_E_UNAVAIL;
    uint32        fteIdx[_BCM_FE2K_VPWS_MAX_SAP];
    int           tmp, idx, hidx, localOh[_BCM_FE2K_VPWS_MAX_SAP];
    int           acIndex=-1, peIndex=-1;
    soc_sbx_g2p3_label2e_t ilm[_BCM_FE2K_VPWS_MAX_SAP];

#define _MPLS_VPN_SAP_SWAP(f)  \
  tmp = vpnSaps[0]->f; vpnSaps[0]->f = vpnSaps[1]->f; vpnSaps[1]->f = tmp;

    for(idx=0; idx < _BCM_FE2K_VPWS_MAX_SAP; idx++) {
        fteIdx[idx] = BCM_GPORT_MPLS_PORT_ID_GET(vpnSaps[idx]->vc_mpls_port_id);
    }

    MPLS_VVERB((_SBX_D(l3_fe->fe_unit, "Enter.  p1Fte=0x%x p2Fte=0x%x\n"),
                fteIdx[0], fteIdx[1]));

    if (SOC_WARM_BOOT(l3_fe->fe_unit) == 0) { 
        rv = _bcm_fe2000_g2p3_mpls_vpws_fte_connect(l3_fe, fteIdx[0], fteIdx[1]);
    } else {
        _MPLS_VPN_SAP_SWAP(vc_mpls_port.port);
        _MPLS_VPN_SAP_SWAP(vc_mpls_port.encap_id);
        return BCM_E_NONE;
    }

    if (BCM_SUCCESS(rv)) {

        /* only locally managed OHIs are in the list, track which are
         * and swap as appropriate below
         */
        for(idx=0; idx < _BCM_FE2K_VPWS_MAX_SAP; idx++) {
            localOh[idx] = !DQ_NULL(&vpnSaps[idx]->vc_ohi_link);
        }

        /* Update the vpn_sap struct to reflect the swapped ohi, common for
         * all known ucodes
         */
        
        /* swap all sw-state  */
        _MPLS_VPN_SAP_SWAP(vc_ohi.ohi);
        _MPLS_VPN_SAP_SWAP(vc_res_alloced);
        _MPLS_VPN_SAP_SWAP(vc_ete_hw_idx.ete_idx);
        _MPLS_VPN_SAP_SWAP(vc_l2ete_hw_idx.ete_idx);

        /* update internal state -
         *   remove the sap from the OHI to VC ETE hash table
         *   insert only if the other OHI is local, that is, it
         *   was previously inserted
         */
        for(idx=0; idx < _BCM_FE2K_VPWS_MAX_SAP; idx++) {
            /* locate the AC label match sap */
            if (!(_BCM_FE2000_IS_PWE3_TUNNEL(vpnSaps[idx]->vc_mpls_port.flags))) {
                acIndex = idx;
            } else {
                peIndex = idx;
            }

            DQ_REMOVE(&vpnSaps[idx]->vc_ohi_link);
            DQ_INIT(&vpnSaps[idx]->vc_ohi_link);

            if (localOh[!idx]) {
                hidx = _FE2K_GET_OHI2ETE_HASH_IDX(vpnSaps[idx]->vc_ohi.ohi);
                DQ_INSERT_HEAD(&l3_fe->fe_ohi2_vc_ete[hidx],
                               &vpnSaps[idx]->vc_ohi_link);
                MPLS_VERB((_SBX_D(l3_fe->fe_unit, "Insert ohi=0x%08x into sw hash idx=%d "
                                  "(id=0x%08x)\n"),
                           vpnSaps[idx]->vc_ohi.ohi, hidx,
                           vpnSaps[idx]->vc_mpls_port_id));
            }
        }

        /* If CES PW handoff provision appropriate ILM setting */
        if((SOC_SBX_CFG(l3_fe->fe_unit)->mplstp_ena) && \
           (vpnSaps[acIndex]->vc_mpls_port.criteria == BCM_MPLS_PORT_MATCH_LABEL)) {

            if(acIndex < 0 || peIndex < 0) {
                MPLS_ERR((_SBX_D(l3_fe->fe_unit, "failed to find Attachment Circuit\n")));            
                return BCM_E_INTERNAL;
            }

            /* If the AC has an Label match criteria, connect ILM of AC
             * to PE Gport */
            for(idx=0; idx < _BCM_FE2K_VPWS_MAX_SAP && BCM_SUCCESS(rv); idx++) {
                rv = soc_sbx_g2p3_label2e_get(l3_fe->fe_unit, 
                                              vpnSaps[idx]->vc_mpls_port.match_label,
                                              &ilm[idx]); 
            }

            /* link ILM to PE gport */
            if (BCM_SUCCESS(rv)) {
                switch(action) {
                case _FE2K_MPLS_PORT_MATCH_ADD:
                case _FE2K_MPLS_PORT_MATCH_UPDATE:   

                    for(idx=0; idx < _BCM_FE2K_VPWS_MAX_SAP; idx++) { 
                        ilm[idx].ftidx1 = fteIdx[idx] >> _MPLSTP_ILM_FT_SHIFT;
                        ilm[idx].ftidx0 = fteIdx[idx] & ((1<<_MPLSTP_ILM_FT_SHIFT) -1);
                    }

                    ilm[peIndex].opcode = _BCM_FE2K_LABEL_CES(l3_fe->fe_unit);
                    ilm[peIndex].vpws   = 1;
                    break;

                case _FE2K_MPLS_PORT_MATCH_DELETE:
                    for(idx=0; idx < _BCM_FE2K_VPWS_MAX_SAP; idx++) { 
                        ilm[idx].ftidx1 = 0;
                        ilm[idx].ftidx0 = 0;
                    }
                    ilm[peIndex].opcode = 0;
                    ilm[peIndex].vpws   = 0;
                    break;

                default:
                    /* never hits here since action is valided by caller */
                    rv = BCM_E_PARAM;
                    break;
                }
                for(idx=0; idx < _BCM_FE2K_VPWS_MAX_SAP && BCM_SUCCESS(rv); idx++) {
                    rv = soc_sbx_g2p3_label2e_set(l3_fe->fe_unit, 
                                                  vpnSaps[idx]->vc_mpls_port.match_label,
                                                  &ilm[idx]); 
                }
                if (BCM_FAILURE(rv)) { 
                    MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) Failed to Set Label2e(ILM)\n"), 
                              bcm_errmsg(rv)));
                }
            } else {
                MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) Failed to Get Label2e(ILM)\n"), 
                          bcm_errmsg(rv)));
            }
        }
    }
    return rv;
}

/*
 * Function:
 *      _bcm_fe2000_mpls_port_replace
 * Purpose:
 *      Replace or update mpls port
 */
STATIC int 
_bcm_fe2000_mpls_port_replace(int                     unit,
                              _fe2k_l3_fe_instance_t *l3_fe,
                              bcm_mpls_port_t        *mpls_port,
                              bcm_vpn_t               vpn,
                              _fe2k_vpn_control_t    *vpnc,
                              _fe2k_vpn_sap_t        *vpn_sap) 
{
    int status = BCM_E_NONE;

    if (mpls_port->flags & BCM_MPLS_PORT_REPLACE) {
        int                  reconnectPorts = 0;
        _fe2k_vpn_sap_t     *vpnSaps[2];

        /* if this is a VPWS & has two ports in the VPN, un-connect them
         * so each port own's its own FTE information before updating
         */
        MPLS_VVERB((_SBX_D(unit, "vpn_flags=0x%x\n"), vpnc->vpn_flags));

        if (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) {
            int                  num_vpn_ports = 0;
            _fe2k_vpn_sap_t     *tmp_vpn_sap = NULL;
            
            _FE2K_ALL_VPN_SAP_PER_VPNC(vpnc, tmp_vpn_sap) {
                if (!(tmp_vpn_sap->vc_mpls_port.flags & 
                                 BCM_MPLS_PORT_FAILOVER)) {
                    if (num_vpn_ports < _BCM_FE2K_VPWS_MAX_SAP) {
                        vpnSaps[num_vpn_ports] = tmp_vpn_sap;
                    }
                    num_vpn_ports++;
                }
            } _FE2K_ALL_VPN_SAP_PER_VPNC_END(vpnc, tmp_vpn_sap);
            
            if (num_vpn_ports == 2) {
                status = _bcm_fe2000_mpls_vpws_fte_connect(l3_fe, vpnSaps, 
                                                           _FE2K_MPLS_PORT_MATCH_UPDATE);
                reconnectPorts = 1;
                if (BCM_FAILURE(status)) {
                    MPLS_ERR((_SBX_D(unit, "error(%s) failed to unlink VPWS\n"),
                              bcm_errmsg(status)));
                }
            }
        }

        status = _bcm_fe2000_mpls_port_update(unit,
                                              l3_fe,
                                              vpn,
                                              vpnc,
                                              mpls_port,
                                              vpn_sap);
        if (BCM_FAILURE(status)) {
            MPLS_ERR((_SBX_D(unit, "MPLS Port Update Failed error:%s\n"),
                      bcm_errmsg(status)));            

        } else if (reconnectPorts) {
            status = _bcm_fe2000_mpls_vpws_fte_connect(l3_fe, vpnSaps, 
                                                       _FE2K_MPLS_PORT_MATCH_UPDATE);
            if (BCM_FAILURE(status)) {
                MPLS_ERR((_SBX_D(unit, "error(%s) failed to unlink VPWS\n"),
                          bcm_errmsg(status)));
            }
        }
    } else {
        MPLS_ERR((_SBX_D(unit, "MPLS flag not set to replace\n")));
        status = BCM_E_PARAM;
    }

    return status;
}


/*
 * Function:
 *      _bcm_fe2000_mpls_port_add
 * Purpose:
 *      Add port into existing VPN
 * Parameters:
 *      unit        - FE unit
 *      l3_fe       - l3 fe instance
 *      vpn_id      - vpn identifier
 *      vpnc        - vpn control block
 *      mpls_port   - port parameter
 * Returns:
 *      BCM_E_XXX
 * Note:
 *
 */

STATIC int
_bcm_fe2000_mpls_port_add(int                     unit,
                          _fe2k_l3_fe_instance_t *l3_fe,
                          bcm_vpn_t               vpn_id,
                          _fe2k_vpn_control_t    *vpnc,
                          bcm_mpls_port_t        *mpls_port,
                          int                     num_vpn_ports)
{
    int                           status, ignore_status;
    _fe2k_vpn_sap_t              *vpn_sap = NULL, *vpn_sap_tmp = NULL;
    _fe2k_fte_idx_t               gport_fte;
    int                           exit_modid = -1, exit_port;
    uint32_t                      logicalPort = ~0;
    _sbx_gu2_usr_res_types_t      resType = SBX_GU2K_USR_RES_FTE_LOCAL_GPORT;
    char                         *serviceName = "<none>";
    _fe2k_vpn_sap_t              *vpn_saps[2];
    bcm_fe2k_mpls_trunk_association_t *trunkAssoc=NULL;
    uint32_t                      trunkid = -1, is_failover = 0;
    int8_t                        is_trunk = 0;
    uint8_t                       is_mplstp_vpwsuni = 0, is_same_label = 0;
 
    vpn_saps[0] = vpn_saps[1] = NULL;
    gport_fte.fte_idx = 0;

    MPLS_VVERB((_SBX_D(unit, "Enter\n")));

    if (!BCM_GPORT_IS_TRUNK(mpls_port->port)) {
        status = _bcm_fe2000_mpls_gport_get_mod_port(l3_fe->fe_unit,
                                                     mpls_port->port,
                                                     &exit_modid,
                                                     &exit_port);
        if (status != BCM_E_NONE) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit,
                      "error(%s) unable to get mod-port from port\n"),
                      bcm_errmsg(status)));
            return status;
        }
    } else {
        is_trunk = 1;
        trunkid = BCM_GPORT_TRUNK_GET(mpls_port->port);
        trunkAssoc = &mpls_trunk_assoc_info[unit][trunkid];
    }

    if (SOC_SBX_CFG(unit)->mplstp_ena &&
        (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) && 
        !(mpls_port->flags & BCM_MPLS_PORT_NETWORK) &&
        (mpls_port->criteria != BCM_MPLS_PORT_MATCH_LABEL)) {
        is_mplstp_vpwsuni = 1;
    }

    /**
     * Allocation of gport. Assumption is that the same gport value will be
     * allocated since user will do port_add() on all units one by one.
     *
     * We suggest strongly that the user create GPORT on first unit and then
     * do mpls_port_add() WITH_ID on other unit(s)
     */
    if (mpls_port->flags & BCM_MPLS_PORT_WITH_ID) {
        gport_fte.fte_idx = BCM_GPORT_MPLS_PORT_ID_GET(mpls_port->mpls_port_id);
    }

    if (SOC_SBX_CFG(unit)->mplstp_ena) {
        resType = (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS)? \
            SBX_GU2K_USR_RES_FTE_MPLS:SBX_GU2K_USR_RES_FTE_LOCAL_GPORT;

        if (is_mplstp_vpwsuni) {
            resType = SBX_GU2K_USR_RES_FTE_VPWS_UNI_GPORT;
        }
    } else {
        resType = (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS)? \
            SBX_GU2K_USR_RES_VSI:SBX_GU2K_USR_RES_FTE_LOCAL_GPORT;
    }

    if (mpls_port->flags & BCM_MPLS_PORT_FAILOVER) {
        resType = SBX_GU2K_USR_RES_FTE_MPLS;
    }

    status  = _bcm_fe2000_alloc_l3_or_mpls_fte(l3_fe,
                                               (mpls_port->flags & BCM_MPLS_PORT_WITH_ID)?BCM_L3_WITH_ID:0,
                                               0,  /* XXX: trunk */
                                               resType,
                                               &gport_fte.fte_idx);
    if (status != BCM_E_NONE) {
        MPLS_ERR((_SBX_D(unit, "error(%s) could not allocate mpls-gport-fte\n"),
                  bcm_errmsg(status)));
        return status;
    }

    /* Do not move this setting of mpls_port_id */
    BCM_GPORT_MPLS_PORT_ID_SET(mpls_port->mpls_port_id, gport_fte.fte_idx);

    /**
     * Create the vpn-sap locally irrespective of where the actual egress 
     * for this gport is.  Soft-state only.
     */
    status = _bcm_fe2000_create_mpls_vpn_sap(l3_fe, vpnc, mpls_port, &vpn_sap);
    
    if (status != BCM_E_NONE) {
        ignore_status = _bcm_fe2000_destroy_mpls_fte(l3_fe,
                                                     L3_OR_MPLS_DESTROY_FTE__FTE_ONLY,
                                                     gport_fte.fte_idx,
                                                     0, 0, vpnc, is_trunk);
        MPLS_ERR((_SBX_D(unit, "error(%s) could not create vpn-sap for gport(0x%x)\n"),
                  bcm_errmsg(status), mpls_port->mpls_port_id));
        return status;
    }

    if (exit_modid == l3_fe->fe_my_modid || is_trunk == 1) {
        /* vpn-sap exit is local. */
        status = _bcm_fe2000_update_vpn_sap_hw(l3_fe, l3_fe->fe_my_modid,
                                               vpn_sap, mpls_port);
        if (status != BCM_E_NONE) {
            MPLS_ERR((_SBX_D(unit, "error(%s) creating mpls-vpn-ete on vpn(0x%x)\n"),
                      bcm_errmsg(status), vpnc->vpn_id));
            ignore_status = _bcm_fe2000_destroy_mpls_fte(l3_fe,
                                                         L3_OR_MPLS_DESTROY_FTE__FTE_OHI_ETE_GIVEN_OHI,
                                                         gport_fte.fte_idx,
                                                         0,
                                                         0,
                                                         vpnc,
                                                         is_trunk);
            return status;
        }
    } else {
        if (SOC_SBX_IS_VALID_ENCAP_ID(vpn_sap->vc_mpls_port.encap_id)) {
            vpn_sap->vc_ohi.ohi =
                SOC_SBX_OHI_FROM_ENCAP_ID(vpn_sap->vc_mpls_port.encap_id);

        } else if (SOC_SBX_IS_VALID_L2_ENCAP_ID(vpn_sap->vc_mpls_port.encap_id)) {
            vpn_sap->vc_ohi.ohi=
                SOC_SBX_OHI_FROM_L2_ENCAP_ID(vpn_sap->vc_mpls_port.encap_id);
        }

    }

    /**
     * program the gport/fte with exit info
     */
    status = _bcm_fe2000_map_set_mpls_vpn_fte(l3_fe,
                                              vpnc,
                                              vpn_sap,
                                              mpls_port);
    if (status != BCM_E_NONE) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) writing fte(0x%x)\n"),
                  bcm_errmsg(status), gport_fte.fte_idx));
        ignore_status = _bcm_fe2000_destroy_mpls_fte(l3_fe,
                                                     L3_OR_MPLS_DESTROY_FTE__FTE_OHI_ETE_GIVEN_OHI,
                                                     gport_fte.fte_idx,
                                                     exit_modid,
                                                     SOC_SBX_ENCAP_ID_FROM_OHI(vpn_sap->vc_ohi.ohi),
                                                     vpnc,
                                                     is_trunk);
        return status;
    }

    vpn_sap->vc_mpls_port.encap_id =
        SOC_SBX_ENCAP_ID_FROM_OHI(vpn_sap->vc_ohi.ohi);


    if (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype == SOC_SBX_UCODE_TYPE_G2P3) {
        if ( mpls_port->criteria == BCM_MPLS_PORT_MATCH_PORT               ||
             mpls_port->criteria == BCM_MPLS_PORT_MATCH_PORT_VLAN          ||
             mpls_port->criteria == BCM_MPLS_PORT_MATCH_PORT_VLAN_STACKED )
        {
            resType = SBX_GU2K_USR_RES_LPORT;

            if (is_mplstp_vpwsuni || 
                 ((mpls_port->flags & BCM_MPLS_PORT_FAILOVER)
                  && (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS)) ) {
                resType = SBX_GU2K_USR_RES_VPWS_UNI_LPORT;
            }

            status = _sbx_gu2_resource_alloc(l3_fe->fe_unit, resType,
                                             1, &logicalPort, 0);
            if (BCM_FAILURE(status)) {
                MPLS_ERR((_SBX_D(unit, "failed to allocate Type(%d) logical port: %d (%s)\n"), 
                          resType, status, bcm_errmsg(status)));
                return status;
            }
            MPLS_VVERB((_SBX_D(unit, "Allocated lp(%d)\n"), logicalPort));
        } else if (mpls_port->criteria == BCM_MPLS_PORT_MATCH_LABEL) {

            if( SOC_SBX_CFG(unit)->mplstp_ena ==1) {
                is_failover = mpls_port->flags & BCM_MPLS_PORT_FAILOVER;
                status = _bcm_fe2000_find_vpn_sap_by_label(l3_fe,
                                            vpnc,
                                            mpls_port->match_label,
                                            &vpn_sap_tmp,
                                            (!is_failover));
                if (BCM_SUCCESS(status)) {
                    /* port with same vc label exits */
                    is_same_label = 1;
                    logicalPort = vpn_sap_tmp->logicalPort;
                } else {
                    status = _sbx_gu2_resource_alloc(l3_fe->fe_unit,
                                                 SBX_GU2K_USR_RES_MPLS_LPORT,
                                                 1, &logicalPort, 0);
                    if (BCM_FAILURE(status)) {
                        MPLS_ERR((_SBX_D(unit, 
                              "failed to allocate logical port: %d (%s)\n"), 
                              status, bcm_errmsg(status)));
                        return status;
                    }
                    MPLS_VVERB((_SBX_D(unit, "Allocated lp(%d)\n"), logicalPort));
                }
            } else {
  
                /* The logical port is at a fixed location for match label: */
                logicalPort = (_FE2K_MPLS_LABEL(mpls_port->match_label) +
                               l3_fe->max_pids);
            }
        }
    }

    /**
     * Program the match/ingress information
     */
    switch (mpls_port->criteria) {
    case BCM_MPLS_PORT_MATCH_PORT:
        status = _bcm_fe2000_match_port2etc(l3_fe,
                                            _FE2K_MPLS_PORT_MATCH_ADD,
                                            logicalPort,
                                            vpnc,
                                            vpn_sap,
                                            mpls_port);
        serviceName = "port2etc";
        break;

    case BCM_MPLS_PORT_MATCH_PORT_VLAN:
        status = _bcm_fe2000_match_pvid2etc(l3_fe,
                                            _FE2K_MPLS_PORT_MATCH_ADD,
                                            logicalPort,
                                            vpnc,
                                            vpn_sap,
                                            mpls_port);
        serviceName = "pvid2etc";
        break;

    case BCM_MPLS_PORT_MATCH_PORT_VLAN_STACKED:
        status = _bcm_fe2000_match_pstackedvid2etc(l3_fe,
                                                   _FE2K_MPLS_PORT_MATCH_ADD,
                                                   logicalPort,
                                                   vpnc,
                                                   vpn_sap,
                                                   mpls_port);
        serviceName = "pstackedvid2etc";
        break;

    case BCM_MPLS_PORT_MATCH_LABEL:
        /* dont update label2e if same label and failover */
        if (!(is_same_label && is_failover ) ) {

            status = _bcm_fe2000_match_label2etc(l3_fe,
                                                 _FE2K_MPLS_PORT_MATCH_ADD,
                                                 logicalPort,
                                                 vpnc,
                                                 vpn_sap,
                                                 mpls_port);
        }
        break;

    case BCM_MPLS_PORT_MATCH_LABEL_VLAN:
    case BCM_MPLS_PORT_MATCH_INVALID:
    case BCM_MPLS_PORT_MATCH_NONE:
    case BCM_MPLS_PORT_MATCH_LABEL_PORT:
    default:
        status = BCM_E_PARAM;
    }

    if (status != BCM_E_NONE) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) updating %s\n"),
                  bcm_errmsg(status), serviceName));
        goto error_done;
    }

    if (!SOC_SBX_CFG(unit)->mplstp_ena) {
        if (!(mpls_port->flags & BCM_MPLS_PORT_FAILOVER)) {
            status = _bcm_fe2000_mpls_program_vpn_sap_vlan2etc(l3_fe,
                                                               vpnc,
                                                               vpn_sap);
            if (status != BCM_E_NONE) {
                MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) programming vlan2etc for port-vsi(0x%x)\n"),
                          bcm_errmsg(status), BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id)));
                goto error_done;
            }
        }
    }

    /* If port is of TRUNK add SAP to the trunk association list */
    if(BCM_GPORT_IS_TRUNK(mpls_port->port)) {
        DQ_INSERT_HEAD(&trunkAssoc->plist, &vpn_sap->trunk_port_link);
        MPLS_VERB((_SBX_D(l3_fe->fe_unit, "MPLS_PORT (0x%x) added to trunk %d\n"),
                   mpls_port->mpls_port_id, trunkid));
    }


    /*#if BCM_WARM_BOOT_SUPPORT*/
#if 0
    /* hide the soft-vpn id in the unused v2e's for VPWS warm boot */
    if ((vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) && (num_vpn_ports == 1)) {
        if (vpn_saps[0] == NULL || vpn_saps[1] == NULL) {
            goto error_done;
        }

        status = _bcm_fe2000_mpls_vpws_wb_set(l3_fe, vpn_saps[0]);
        if (BCM_FAILURE(status)) {
            goto error_done;
        }

        status = _bcm_fe2000_mpls_vpws_wb_set(l3_fe, vpn_saps[1]);
        if (BCM_FAILURE(status)) {
            goto error_done;
        }
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    if ((vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) && 
        (num_vpn_ports == 1 && !(mpls_port->flags & BCM_MPLS_PORT_FAILOVER))) {
        _fe2k_vpn_sap_t *vpn_sap_loop;
        int              idx = 0;

        vpn_saps[0] = vpn_saps[1] = NULL;

        _FE2K_ALL_VPN_SAP_PER_VPNC(vpnc, vpn_sap_loop) {
            /* cross connect only primary port */
            if (!(vpn_sap_loop->vc_mpls_port.flags & BCM_MPLS_PORT_FAILOVER)) {
                if (vpn_saps[idx] == NULL) {
                    vpn_saps[idx++] = vpn_sap_loop;
                }
            }
        } _FE2K_ALL_VPN_SAP_PER_VPNC_END(vpnc, vpn_sap_loop);

        if (vpn_saps[0] == NULL || vpn_saps[1] == NULL) {
            MPLS_ERR((_SBX_D(unit, "invalid vpn saps supplied\n")));
            status = BCM_E_INTERNAL;
            goto error_done;
        }

        /* now we have both the ports to make
         * connection b/w them.
         */
        status = _bcm_fe2000_mpls_vpws_fte_connect(l3_fe, vpn_saps, 
                                                   _FE2K_MPLS_PORT_MATCH_ADD);
        if(BCM_FAILURE(status)) {
            MPLS_ERR((_SBX_D(unit, "VPWS cross connect failed!! Error: %d/%s\n"),
                       status, bcm_errmsg(status)));
        }
    }

error_done:
    if (BCM_FAILURE(status)) {
        ignore_status = _bcm_fe2000_destroy_mpls_fte(l3_fe,
                                                     L3_OR_MPLS_DESTROY_FTE__FTE_OHI_ETE_GIVEN_OHI,
                                                     gport_fte.fte_idx,
                                                     exit_modid,
                                                     SOC_SBX_ENCAP_ID_FROM_OHI(vpn_sap->vc_ohi.ohi),
                                                     vpnc,
                                                     is_trunk);
    } else {
        /* if mpls port add successful, insert the gport to dbase */
        status = shr_htb_insert(mpls_gport_db[unit],
                            (shr_htb_key_t) &vpn_sap->vc_mpls_port_id, 
                            (void*) vpn_sap);        
        if (BCM_FAILURE(status)) {
            MPLS_ERR((_SBX_D(unit, "error(%s) Updating Gport Dbase (0x%x)\n"),
                      bcm_errmsg(status), BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id)));
        }        
    }

    return status;
}

STATIC int
_bcm_fe2000_mpls_port_delete(int                      unit,
                             _fe2k_l3_fe_instance_t  *l3_fe,
                             _fe2k_vpn_control_t     *vpnc,
                             _fe2k_vpn_sap_t        **vpn_sap)
{
    int                         status;
    _fe2k_fte_idx_t             fte_idx;
    bcm_mpls_port_t            *mpls_port;
    int                         freeLp = 0;
    uint32_t                    logicalPort = 0, is_failover = 0;
    _sbx_gu2_usr_res_types_t    restype;
    shr_htb_data_t              datum = NULL;
    int8_t                      is_trunk = 0;
    uint8_t                     is_mplstp_vpwsuni = 0, is_same_label = 0;
    _fe2k_vpn_sap_t            *vpn_sap_tmp = NULL;

    mpls_port        = &(*vpn_sap)->vc_mpls_port;
    fte_idx.fte_idx  =
        BCM_GPORT_MPLS_PORT_ID_GET((*vpn_sap)->vc_mpls_port_id);

    if (BCM_GPORT_IS_TRUNK(mpls_port->port)) {
        is_trunk = 1;
        logicalPort = (*vpn_sap)->logicalPort;
    }

    if (SOC_SBX_CFG(unit)->mplstp_ena &&
        (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) && 
        !(mpls_port->flags & BCM_MPLS_PORT_NETWORK) &&
        (mpls_port->criteria != BCM_MPLS_PORT_MATCH_LABEL)) {
        is_mplstp_vpwsuni = 1;
    }

    if (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype == SOC_SBX_UCODE_TYPE_G2P3) {

        freeLp = 1;

        if ((SOC_SBX_CFG(l3_fe->fe_unit)->mplstp_ena == 0) &&
            (mpls_port->criteria == BCM_MPLS_PORT_MATCH_LABEL) &&
            (is_trunk == 0)) {
            status = _bcm_fe2000_g2p3_mpls_lp_get(l3_fe->fe_unit, mpls_port,
                                                  &logicalPort);

            if (status == BCM_E_NOT_FOUND) {
                status = BCM_E_NONE;
            } else {
                if (BCM_FAILURE(status)) {
                    MPLS_ERR((_SBX_D(unit,
                             "failed to free logical port: %d (%s)\n"),
                             status, bcm_errmsg(status)));
                    return status;
                }
            }
        } else {
            logicalPort = (*vpn_sap)->logicalPort;
        }
    }

    switch (mpls_port->criteria) {
    case BCM_MPLS_PORT_MATCH_PORT:
        status = _bcm_fe2000_match_port2etc(l3_fe,
                                            _FE2K_MPLS_PORT_MATCH_DELETE,
                                            logicalPort,
                                            vpnc,
                                            *vpn_sap,
                                            mpls_port);
        if (status != BCM_E_NONE) {
            MPLS_ERR((_SBX_D(unit, "error(%s) deleting port2etc\n"), 
                      bcm_errmsg(status)));
        }
        break;

    case BCM_MPLS_PORT_MATCH_PORT_VLAN:
        status = _bcm_fe2000_match_pvid2etc(l3_fe,
                                            _FE2K_MPLS_PORT_MATCH_DELETE,
                                            logicalPort,
                                            vpnc,
                                            *vpn_sap,
                                            mpls_port);
        if (status != BCM_E_NONE) {
            MPLS_ERR((_SBX_D(unit, "error(%s) deleting pvid2etc\n"),
                      bcm_errmsg(status)));
        }
        break;

    case BCM_MPLS_PORT_MATCH_PORT_VLAN_STACKED:
        status = _bcm_fe2000_match_pstackedvid2etc(l3_fe,
                                                   _FE2K_MPLS_PORT_MATCH_DELETE,
                                                   logicalPort,
                                                   vpnc,
                                                   *vpn_sap,
                                                   mpls_port);
        if (status != BCM_E_NONE) {
            MPLS_ERR((_SBX_D(unit, "error(%s) deleting pstackedvid2etc\n"),
                      bcm_errmsg(status)));
        }
        break;

    case BCM_MPLS_PORT_MATCH_LABEL:
        is_failover = mpls_port->flags & BCM_MPLS_PORT_FAILOVER;
        status = _bcm_fe2000_find_vpn_sap_by_label(l3_fe,
                                            vpnc,
                                            mpls_port->match_label,
                                            &vpn_sap_tmp,
                                            (!is_failover));
        if (BCM_SUCCESS(status)) {
            /* port with same vc label exits */
            is_same_label = 1;
        }

        /* update label2e and logical port only if
         *  no other port exist with same label */
        if (is_same_label == 0 ) {
            status = _bcm_fe2000_match_label2etc(l3_fe,
                                                 _FE2K_MPLS_PORT_MATCH_DELETE,
                                                 logicalPort,
                                                 vpnc,
                                                 *vpn_sap,
                                                 mpls_port);
            if (status != BCM_E_NONE) {
                MPLS_ERR((_SBX_D(unit, "error(%s) deleting label2etc\n"),
                          bcm_errmsg(status)));
            }
        } else {
            freeLp = 0;
        }
        break;

    case BCM_MPLS_PORT_MATCH_LABEL_VLAN:
    case BCM_MPLS_PORT_MATCH_INVALID:
    case BCM_MPLS_PORT_MATCH_NONE:
    case BCM_MPLS_PORT_MATCH_LABEL_PORT:
    default:
        return BCM_E_PARAM;
    }

    if (is_mplstp_vpwsuni) {
        restype = SBX_GU2K_USR_RES_VPWS_UNI_LPORT;
    } else if ( (SOC_SBX_CFG(unit)->mplstp_ena == 1) && 
        (mpls_port->criteria == BCM_MPLS_PORT_MATCH_LABEL)) {
        restype = SBX_GU2K_USR_RES_MPLS_LPORT;
    } else {
        restype = SBX_GU2K_USR_RES_LPORT;
    }

    if (freeLp){
        status = _sbx_gu2_resource_free(l3_fe->fe_unit, restype,
                                        1, &logicalPort, 0);
        if (BCM_FAILURE(status)) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "!! leak: failed to free logical port[0x%x]: %d (%s)\n"),
                      logicalPort, status, bcm_errmsg(status)));
        }
    }

    if (BCM_GPORT_IS_TRUNK(mpls_port->port)) {
        DQ_REMOVE(&(*vpn_sap)->trunk_port_link);
        MPLS_VERB((_SBX_D(l3_fe->fe_unit, 
                   "MPLS_PORT (0x%x) removed from trunk association\n"),
                   mpls_port->mpls_port_id));
    }

    /* find and destroy the gport from gport dbase */
    status = shr_htb_find(mpls_gport_db[unit], 
                          (shr_htb_key_t) &(*vpn_sap)->vc_mpls_port_id,
                          (shr_htb_data_t *)&datum, 1);
    if (BCM_FAILURE(status)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "failed to free MPLS Gport dbase element: %d (%s)\n"),
                  status, bcm_errmsg(status)));
    }

    /*
     * destroy FTE, and OHI/ETE based on values in HW FTE
     */
    status = _bcm_fe2000_destroy_mpls_fte(l3_fe,
                                          L3_OR_MPLS_DESTROY_FTE__FTE_HW_OHI_ETE,
                                          fte_idx.fte_idx,
                                          0,/* module */
                                          0,/* ohi    */
                                          vpnc,
                                          is_trunk);
    if (status != BCM_E_NONE) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) destroying mpls-fte(0x%x)\n"),
                  bcm_errmsg(status), fte_idx.fte_idx));
        return status;
    }

    return BCM_E_NONE;
}


int
_bcm_fe2000_mpls_port_vlan_vector_internal(int unit,
                                           bcm_gport_t gport,
                                           bcm_port_t *phy_port,
                                           bcm_vlan_t *match_vlan,
                                           bcm_vlan_t *vsi,
                                           uint32_t   *logicalPort,
                                           bcm_mpls_port_t *mpls_port,
                                           uint8_t    *vpwsuni)
{
    _fe2k_l3_fe_instance_t       *l3_fe = NULL;
    _fe2k_vpn_sap_t              *vpn_sap = NULL;
    int                          found = FALSE;
    int                          status = BCM_E_NONE;
    int                          exit_modid, mymodid;
    uint8_t                      is_vpws_uni=0;
    uint32_t                     fte_idx=0, lpi=0;

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);

    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_FE2K_IS_MPLS_INITIALIZED(l3_fe)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    if ((status = bcm_stk_my_modid_get(unit, &mymodid)) != BCM_E_NONE) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "bcm_stk_my_modid_get failed with %d: %s\n"),
                  status, bcm_errmsg(status)));
        L3_UNLOCK(unit);
        return status;
    }

    /*
     * Note: this can be excruciatingly slow if there are
     * a lot of active VPNs and multiple ports in each VPN
     */
    found = 
        _bcm_fe2000_mpls_find_vpn_sap_by_gport(l3_fe, gport, &vpn_sap);

    if (!found || vpn_sap == NULL) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls gport 0x%x not found\n"), gport));
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if (!(vpn_sap->vc_mpls_port.criteria == BCM_MPLS_PORT_MATCH_PORT ||
          vpn_sap->vc_mpls_port.criteria == BCM_MPLS_PORT_MATCH_PORT_VLAN)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls gport 0x%x cannot be updated\n"), gport));
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if (BCM_GPORT_IS_MODPORT(vpn_sap->vc_mpls_port.port)) {
        status = _bcm_fe2000_mpls_gport_get_mod_port(l3_fe->fe_unit,
                                                     vpn_sap->vc_mpls_port.port,
                                                     &exit_modid,
                                                     phy_port);
        if (status != BCM_E_NONE) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) extracting mod-port from gport 0x%x\n"),
                      bcm_errmsg(status), vpn_sap->vc_mpls_port.port));
            L3_UNLOCK(unit);
            return status;
        }

        if (exit_modid != mymodid) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) attempting to update MPLS gport"
                      " of a remote port\n"), bcm_errmsg(status)));
            L3_UNLOCK(unit);
            return BCM_E_PARAM;
        }
    } else if (BCM_GPORT_IS_LOCAL(vpn_sap->vc_mpls_port.port)) {
        *phy_port = BCM_GPORT_LOCAL_GET(vpn_sap->vc_mpls_port.port);
    } else if (SOC_PORT_VALID(unit, vpn_sap->vc_mpls_port.port)) {
        *phy_port = vpn_sap->vc_mpls_port.port;
    } else {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) attempting to update an "
                         "invalid MPLS gport (0x%x)\n"),
                  bcm_errmsg(status), vpn_sap->vc_mpls_port.port));
        L3_UNLOCK(unit);
        return BCM_E_PORT;
    }

    fte_idx =  (BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id)); 
    lpi     = vpn_sap->logicalPort;

    if ((SOC_SBX_CFG(unit)->mplstp_ena) &&
        (fte_idx >= SBX_VPWS_UNI_FTE_BASE(unit)) &&
        (fte_idx <= SBX_VPWS_UNI_FTE_END(unit))) {
        is_vpws_uni = 1;
    }

    if (vpwsuni) {
        *vpwsuni = is_vpws_uni;
    } 
    
    if (is_vpws_uni) {
        fte_idx = BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id);
        status  = _bcm_fe2000_g2p3_mplstp_vpws_ft_lp_offset(unit, 
                                                           vpn_sap->vc_mpls_port_id,
                                                           &lpi,
                                                           &fte_idx);
        if (BCM_FAILURE(status)) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) to update "
                             "vpws uni offset for MPLS gport (0x%x)\n"),
                      bcm_errmsg(status), vpn_sap->vc_mpls_port.port));
            L3_UNLOCK(unit);
            return status;
        }
    } else {
        fte_idx -= l3_fe->vlan_ft_base;
    }

    /*
     * Assumed that this macro always gives the actual VSI/vlan
     * programmed in pv2e
     */
    if (vsi) {
        *vsi = fte_idx;
    }

    if (logicalPort) {
        *logicalPort = lpi;
    }

    if (match_vlan) {
        *match_vlan = vpn_sap->vc_mpls_port.match_vlan;
    }

    if (mpls_port) {
        status = _bcm_fe2000_fill_mpls_port_from_vpn_sap(l3_fe,
                                                         vpn_sap,
                                                         mpls_port);
    }

    L3_UNLOCK(unit);

    return status;
}

int
_bcm_fe2000_mpls_port_gport_attr_get(int unit,
                                     bcm_gport_t gport,
                                     bcm_port_t *phy_port,
                                     bcm_vlan_t *match_vlan,
                                     bcm_vlan_t *vsi)
{
    return (_bcm_fe2000_mpls_port_vlan_vector_internal(unit,
                                                       gport,
                                                       phy_port,
                                                       match_vlan,
                                                       vsi,
                                                       NULL,
                                                       NULL,
                                                       NULL));
}

int
_bcm_fe2000_mpls_port_vlan_vector_set(int unit,
                                      bcm_gport_t gport,
                                      bcm_vlan_vector_t vlan_vec)
{
    bcm_vlan_t                   vid, match_vid, vpn;
    int                          status = BCM_E_NONE;
    bcm_port_t                   phy_port;
    soc_sbx_g2p3_pv2e_t          p3pv2e;
    uint32                       logicalPort = ~0;
    bcm_mpls_port_t              mpls_port;
    uint8_t                      vpwsuni=0;

    BCM_IF_ERROR_RETURN
        (_bcm_fe2000_mpls_port_vlan_vector_internal(unit,
                                                    gport,
                                                    &phy_port,
                                                    &match_vid,
                                                    &vpn,
                                                    &logicalPort,
                                                    &mpls_port,
                                                    &vpwsuni));

    for (vid = BCM_VLAN_MIN + 1; vid < BCM_VLAN_MAX; vid++) {
        /* Always need to read the entry */
        soc_sbx_g2p3_pv2e_t_init(&p3pv2e);
        status = SOC_SBX_G2P3_PV2E_GET(unit, phy_port, vid,
                                           &p3pv2e);
        if (BCM_FAILURE(status)) {
            break;
        }


        if (BCM_VLAN_VEC_GET(vlan_vec, vid)) {
            /* this VID is a member of the vector, set it up */
            p3pv2e.vlan = vpn;
            p3pv2e.lpi = logicalPort;
            p3pv2e.vpws = (vpwsuni)?1:0;
        } else if (!BCM_VLAN_VEC_GET(vlan_vec, vid) &&
                   (p3pv2e.vlan == vpn) && (p3pv2e.lpi == logicalPort)) {
            /* match vid must be reset through mpls port delete */
            if (!((mpls_port.criteria & BCM_MPLS_PORT_MATCH_PORT_VLAN) &&
                (mpls_port.match_vlan == vid))) {
                p3pv2e.vlan = 0;
                p3pv2e.lpi = 0;
                p3pv2e.vpws = 0;
            }
        }
        status = _soc_sbx_g2p3_pv2e_set(unit, phy_port, vid,
                                        &p3pv2e);
        
        if (BCM_FAILURE(status)) {
            break;
        }
    }
    
    return status;
}

int
_bcm_fe2000_mpls_port_vlan_vector_get(int unit,
                                      bcm_gport_t gport,
                                      bcm_vlan_vector_t vlan_vec)
{
    bcm_vlan_t                   vid, vpn;
    int                          status = BCM_E_NONE;
    bcm_port_t                   phy_port;
    soc_sbx_g2p3_pv2e_t          p3pv2e;
    uint8_t                      vpwsuni=0;

    BCM_IF_ERROR_RETURN
        (_bcm_fe2000_mpls_port_vlan_vector_internal(unit,
                                                    gport,
                                                    &phy_port,
                                                    NULL,
                                                    &vpn,
                                                    NULL,
                                                    NULL,
                                                    &vpwsuni));

    BCM_VLAN_VEC_ZERO(vlan_vec);

    for (vid = BCM_VLAN_MIN + 1; vid < BCM_VLAN_MAX; vid++) {


        soc_sbx_g2p3_pv2e_t_init(&p3pv2e);
        status = soc_sbx_g2p3_pv2e_get(unit, vid, phy_port,
                                       &p3pv2e);
        if (BCM_E_NONE == status) {
            if (p3pv2e.vlan == vpn && p3pv2e.vpws == vpwsuni) {
                BCM_VLAN_VEC_SET(vlan_vec, vid);
            }
        }

        if (BCM_FAILURE(status)) {
            break;
        }
    }
    
    return status;
}

/*
 * Function:
 *      bcm_fe2000_mpls_init
 * Description:
 *      initialize mpls structures and h/w
 * Parameters:
 *      unit         - fe unit
 * Returns:
 *      BCM_E_NONE   - on success
 *      BCM_E_XXX    - on failure
 * Assumption:
 */

int
bcm_fe2000_mpls_init(int unit)
{
    int                           status;
    _fe2k_l3_fe_instance_t       *l3_fe;
    
    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (!L3_LOCK_CREATED_ALREADY(unit)) {
        return BCM_E_INTERNAL;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (_FE2K_IS_MPLS_INITIALIZED(l3_fe) && SOC_WARM_BOOT(unit)) {
        status = _bcm_fe2000_mpls_recover(l3_fe);
        L3_UNLOCK(unit);
        return status;
    }

    if (_FE2K_IS_MPLS_INITIALIZED(l3_fe)) {
        status = _bcm_fe2000_mpls_cleanup(unit, l3_fe);
        if (BCM_FAILURE(status)) {
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls detach failed\n")));
            L3_UNLOCK(unit);
            return status;
        }
    }

    status = _bcm_fe2000_mpls_init(unit,
                                   l3_fe);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    L3_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_fe2000_mpls_cleanup
 * Description:
 *      cleanup mpls related items
 * Parameters:
 *      unit         - fe unit
 * Returns:
 *      BCM_E_NONE   - on success
 *      BCM_E_XXX    - on failure
 * Assumption:
 *      There is ordering restriction
 *      bcm_l3_init();
 *      bcm_mpls_init();
 *      ...
 *      ...
 *      bcm_mpls_cleanup();
 *      bcm_l3_cleanup();
 */

int
bcm_fe2000_mpls_cleanup(int unit)
{
    _fe2k_l3_fe_instance_t       *l3_fe;
    int                           status;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    /**
     * For some reason the LOCK was not
     * created, then there is *really*
     * nothing to clean. Hence return
     * BCM_E_NONE
     */
    if (!L3_LOCK_CREATED_ALREADY(unit)) {
        return BCM_E_NONE;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    /**
     * Do not call _cleanup() without _init()
     */
    if (!_FE2K_IS_MPLS_INITIALIZED(l3_fe)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_fe2000_mpls_cleanup(unit,
                                      l3_fe);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    L3_UNLOCK(unit);
    return BCM_E_NONE;
}

int
bcm_fe2000_mpls_vpn_id_create(int                    unit,
                              bcm_mpls_vpn_config_t *info)
{
    _fe2k_l3_fe_instance_t       *l3_fe;
    _fe2k_vpn_control_t          *vpnc;
    int                           status;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_FE2K_IS_MPLS_INITIALIZED(l3_fe)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_fe2000_validate_mpls_vpn_id_create(unit, info);
    if (status != BCM_E_NONE) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Invalid parameters\n")));
        L3_UNLOCK(unit);
        return status;
    }

    if (info->flags & BCM_MPLS_VPN_REPLACE) {
        status = _bcm_fe2000_find_mpls_vpncb_by_id(unit,
                                                   l3_fe,
                                                   info->vpn,
                                                   &vpnc);
        if (status != BCM_E_NONE) {
            L3_UNLOCK(unit);
            return BCM_E_NOT_FOUND;
        }
        status = _bcm_fe2000_update_mpls_vpn_id(unit,
                                                l3_fe,
                                                vpnc,
                                                info);
    } else {

        if (info->flags & BCM_MPLS_VPN_WITH_ID) {
            status = _bcm_fe2000_find_mpls_vpncb_by_id(unit,
                                                       l3_fe,
                                                       info->vpn,
                                                       &vpnc);
            if (status == BCM_E_NONE) {
                L3_UNLOCK(unit);
                return BCM_E_EXISTS;
            }
        }

        status = _bcm_fe2000_add_mpls_vpn_id(unit, l3_fe, info);
    }

    L3_UNLOCK(unit);
    return status;
}

int
bcm_fe2000_mpls_vpn_id_destroy(int       unit,
                               bcm_vpn_t vpn)
{
    _fe2k_l3_fe_instance_t       *l3_fe;
    _fe2k_vpn_control_t          *vpnc;
    int                           status;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (!_BCM_MPLS_VPN_VALID_USER_HANDLE(unit, vpn)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_FE2K_IS_MPLS_INITIALIZED(l3_fe)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_fe2000_find_mpls_vpncb_by_id(unit,
                                               l3_fe,
                                               vpn,
                                               &vpnc);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return BCM_E_NOT_FOUND;
    }

    if (!DQ_EMPTY(&vpnc->vpn_sap_head)) {
        L3_UNLOCK(unit);
        return BCM_E_BUSY;
    }

    status = _bcm_fe2000_destroy_mpls_vpn_id(unit, l3_fe, &vpnc);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    L3_UNLOCK(unit);
    return BCM_E_NONE;
}

int
bcm_fe2000_mpls_vpn_id_destroy_all(int unit)
{
    _fe2k_l3_fe_instance_t       *l3_fe;
    int                           status;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_FE2K_IS_MPLS_INITIALIZED(l3_fe)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_fe2000_destroy_all_mpls_vpn_id(unit,
                                                 l3_fe);
    L3_UNLOCK(unit);
    return status;
}

int
bcm_fe2000_mpls_vpn_id_get(int                    unit,
                           bcm_vpn_t              vpn,
                           bcm_mpls_vpn_config_t *info)
{
    _fe2k_l3_fe_instance_t       *l3_fe;
    _fe2k_vpn_control_t          *vpnc;
    int                           status;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (!_BCM_MPLS_VPN_VALID_USER_HANDLE(unit, vpn)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_FE2K_IS_MPLS_INITIALIZED(l3_fe)) {
        MPLS_ERR((_SBX_D(unit, "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_fe2000_find_mpls_vpncb_by_id(unit,
                                               l3_fe,
                                               vpn,
                                               &vpnc);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    status = _bcm_fe2000_fill_mpls_vpn_config(unit,
                                              vpnc,
                                              info);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    L3_UNLOCK(unit);
    return BCM_E_NONE;
}

int
bcm_fe2000_mpls_vpn_traverse(int   unit,
                             bcm_mpls_vpn_traverse_cb cb,
                             void  *user_data)
{
    bcm_mpls_vpn_config_t info;
    _fe2k_vpn_control_t          *vpnc = NULL;
    _fe2k_l3_fe_instance_t       *l3_fe;
    int                           status;

    MPLS_VVERB((_SBX_D(unit, "Enter\n")));

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (cb == NULL) {
        return BCM_E_PARAM;
    }
  
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_FE2K_IS_MPLS_INITIALIZED(l3_fe)) {
        MPLS_ERR((_SBX_D(unit, "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    _FE2K_ALL_VPNC(l3_fe, vpnc) {
        bcm_mpls_vpn_config_t_init(&info);
        status = _bcm_fe2000_fill_mpls_vpn_config(unit, vpnc, &info);
        if (BCM_SUCCESS(status)) {
            cb(unit, &info, user_data);
        }
    } _FE2K_ALL_VPNC_END(l3_fe, vpnc);

    L3_UNLOCK(unit);

    return BCM_E_NONE;
}

int
bcm_fe2000_mpls_remote_update_vpn_sap_hw(int              unit,
                                         bcm_module_t     caller_module,
                                         bcm_vpn_t        vpn_id,
                                         bcm_mpls_port_t *mpls_port)
{
    int                           status, ignore_status;
    _fe2k_l3_fe_instance_t       *l3_fe;
    _fe2k_vpn_control_t          *vpnc;
    _fe2k_vpn_sap_t              *vpn_sap;

    if (!_BCM_MPLS_VPN_VALID_USER_HANDLE(unit, vpn_id)) {
        MPLS_ERR((_SBX_D(unit, "Invalid user handle 0x%x\n"), (int)vpn_id));
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    if (!_FE2K_IS_MPLS_INITIALIZED(l3_fe)) {
        MPLS_ERR((_SBX_D(unit, "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }
    status = _bcm_fe2000_find_mpls_vpncb_by_id(unit,
                                               l3_fe,
                                               vpn_id,
                                               &vpnc);
    if (status != BCM_E_NONE) {
        MPLS_ERR((_SBX_D(unit, "Could not find vpn id 0x%x\n"), vpn_id));
        L3_UNLOCK(unit);
        return status;
    }

    status = _bcm_fe2000_create_mpls_vpn_sap(l3_fe,
                                             vpnc,
                                             mpls_port,
                                             &vpn_sap);
    if (status != BCM_E_NONE) {
        MPLS_ERR((_SBX_D(unit, "error(%s) creating vpn-sap\n"), bcm_errmsg(status)));
        L3_UNLOCK(unit);
        return status;
    }

    status = _bcm_fe2000_update_vpn_sap_hw(l3_fe,
                                           caller_module,
                                           vpn_sap,
                                           mpls_port);
    if (status != BCM_E_NONE) {
        MPLS_ERR((_SBX_D(unit, "error(%s) updating vpn-sap\n"), bcm_errmsg(status)));
        if (vpn_sap->vc_inuse_ue == 0) {
            ignore_status = _bcm_fe2000_free_mpls_vpn_sap(l3_fe,
                                                          &vpn_sap);
        }
        L3_UNLOCK(unit);
        return status;
    }

    L3_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_fe2000_mpls_vpls_encap_get
 * Purpose:
 *      Given VPN hint(group) and the mpls_port_id
 *      we need to return the encap_id associated
 * Parameters:
 *      unit         - FE unit
 *      group        - broadcast group for the vpn
 *      port         - port
 *      mpls_port_id - gport of port in vpn
 *      encap_id     - encap_id returned
 * Returns:
 *      BCM_E_XXX
 * Note:
 *      This is called from outside the MPLS module
 *      and hence needs to LOCK/UNLOCK before accessing
 *      internal structures
 */

int
_bcm_fe2000_mpls_vpls_encap_get(int              unit,
                                bcm_multicast_t  group,
                                bcm_gport_t      port,
                                bcm_gport_t      mpls_port_id,
                                bcm_if_t        *encap_id)
{
    _fe2k_l3_fe_instance_t       *l3_fe;
    int                           status;
    _fe2k_vpn_control_t          *vpnc;
    _fe2k_vpn_sap_t              *vpn_sap;

    if (!group || !encap_id || !mpls_port_id) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_FE2K_IS_MPLS_INITIALIZED(l3_fe)) {
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_fe2000_find_mpls_vpncb_by_mcg(l3_fe,
                                                group,
                                                &vpnc);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    status = _bcm_fe2000_find_vpn_sap_by_id(l3_fe,
                                            vpnc,
                                            mpls_port_id,
                                            &vpn_sap);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    if (vpn_sap->vc_ohi.ohi != _FE2K_INVALID_OHI) {
        *encap_id = SOC_SBX_ENCAP_ID_FROM_OHI(vpn_sap->vc_ohi.ohi);
        L3_UNLOCK(unit);
        return BCM_E_NONE;
    }

    L3_UNLOCK(unit);
    return BCM_E_NOT_FOUND;
}


int
_bcm_fe2000_mpls_vpn_stp_internal(int unit,
                                  bcm_vpn_t  vpn,
                                  bcm_port_t port,
                                  _fe2k_l3_fe_instance_t **l3_fe,
                                  _fe2k_vpn_control_t **vpnc)
{
    int status = BCM_E_NONE;

    if (!_BCM_MPLS_VPN_VALID_USER_HANDLE(unit, vpn)) {
        return BCM_E_PARAM;
    }

    if (!BCM_GPORT_IS_SET(port) ||
        !BCM_GPORT_IS_MPLS_PORT(port)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    *l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (*l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_FE2K_IS_MPLS_INITIALIZED(*l3_fe)) {
        MPLS_ERR((_SBX_D(unit, "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_fe2000_find_mpls_vpncb_by_id(unit,
                                               *l3_fe,
                                               vpn,
                                               vpnc);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    if ((*vpnc)->vpn_flags & BCM_MPLS_VPN_L3) {
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    L3_UNLOCK(unit);
    return status;
}

int
_bcm_fe2000_g2p3_mpls_vpn_stp_set_get(int                 unit,
                                      bcm_vpn_t           vpn,
                                      bcm_port_t          port,
                                      int                 *stp_state,
                                      _fe2k_vpn_control_t *vpnc,
                                      int                 set)
{
    bcm_vlan_t           vid;
    bcm_mpls_port_t      *mpls_port = NULL;
    bcm_port_t           phy_port;
    soc_sbx_g2p3_p2e_t   p2e;
    int                  rv = BCM_E_NONE;
    _fe2k_vpn_sap_t      *vpn_sap = NULL;
    int                  mymodid, gmodid;

    L3_LOCK(unit);
    _FE2K_ALL_VPN_SAP_PER_VPNC(vpnc, vpn_sap) {
        if (vpn_sap->vc_mpls_port_id == port) {
            vid = BCM_VLAN_INVALID;
            mpls_port = &vpn_sap->vc_mpls_port;
            phy_port = mpls_port->port;

            if (BCM_GPORT_IS_SET(phy_port) &&
                BCM_GPORT_IS_MODPORT(phy_port)) {

                rv = bcm_stk_my_modid_get(unit, &mymodid);
                if (rv != BCM_E_NONE) {
                    L3_UNLOCK(unit);
                    return rv;
                }

                gmodid = BCM_GPORT_MODPORT_MODID_GET(phy_port);
                if (gmodid == mymodid) {
                    phy_port = BCM_GPORT_MODPORT_PORT_GET(phy_port);
                }
            }

            switch (mpls_port->criteria) {
            case BCM_MPLS_PORT_MATCH_PORT:
            {
                soc_sbx_g2p3_p2e_t_init(&p2e);

                rv = soc_sbx_g2p3_p2e_get(unit,
                                          phy_port,
                                          &p2e);
                if (rv == BCM_E_NONE) {
                    vid = p2e.nativevid;
                    if (set) {
                        rv = _bcm_fe2000_g2p3_stg_vid_stp_set(unit, vid,
                                                              phy_port, *stp_state);
                    } else {
                        rv = _bcm_fe2000_g2p3_stg_vid_stp_get(unit, vid,
                                                              phy_port, stp_state);
                    }
                }
            }
            break;
            case BCM_MPLS_PORT_MATCH_PORT_VLAN:
            {
                vid = mpls_port->match_vlan;
                if (set) {
                    rv = _bcm_fe2000_g2p3_stg_vid_stp_set(unit, vid,
                                                          phy_port, *stp_state);
                } else {
                    rv = _bcm_fe2000_g2p3_stg_vid_stp_get(unit, vid,
                                                          phy_port, stp_state);
                }
            }
            break;
            case BCM_MPLS_PORT_MATCH_PORT_VLAN_STACKED:
            {
                rv = _bcm_fe2000_g2p3_stg_stacked_vid_stp_set_get(unit,
                                                                  mpls_port->match_vlan,
                                                                  mpls_port->match_inner_vlan,
                                                                  phy_port,
                                                                  stp_state,
                                                                  set);
            }
            break;
            case BCM_MPLS_PORT_MATCH_LABEL:
            {
                rv = _bcm_fe2000_g2p3_stg_label_stp_set_get(unit,
                                                            mpls_port->match_label,
                                                            stp_state,
                                                            set);
            }
            break;
            default:
                rv = BCM_E_PARAM;
                break;
            }

            break;
        }
    } _FE2K_ALL_VPN_SAP_PER_VPNC_END(vpnc, vpn_sap);

    L3_UNLOCK(unit);
    return rv;
}

/*
 * Internal access routine
 *
 * vpn:   VPN
 * port:  MPLS port
 * stp_state:  STG stp state
 */
int
_bcm_fe2000_mpls_vpn_stp_set(int        unit,
                             bcm_vpn_t  vpn,
                             bcm_port_t port,
                             int        stp_state)
{
    _fe2k_l3_fe_instance_t *l3_fe = NULL;
    int                    status = BCM_E_NONE;
    _fe2k_vpn_control_t    *vpnc = NULL;
    int                    stpState = stp_state;

    BCM_IF_ERROR_RETURN
        (_bcm_fe2000_mpls_vpn_stp_internal(unit, vpn, port,
                                           &l3_fe, &vpnc));

    if (SOC_IS_SBX_G2P3(unit)) {
        return (_bcm_fe2000_g2p3_mpls_vpn_stp_set_get(unit, vpn,
                                                      port, &stpState,
                                                      vpnc, TRUE));
    } 
    else {
        BCM_DEBUG(BCM_DBG_WARN, ("WARNING %s has not been ported for this microcode (%s:%d)\n", \
                                 FUNCTION_NAME(), __FILE__, __LINE__));
        return BCM_E_UNAVAIL;
    }
    return status;
}

int
_bcm_fe2000_mpls_vpn_stp_get(int        unit,
                             bcm_vpn_t  vpn,
                             bcm_port_t port,
                             int        *stp_state)
{
    _fe2k_l3_fe_instance_t       *l3_fe = NULL;
    int                          status = BCM_E_NONE;
    _fe2k_vpn_control_t          *vpnc = NULL;

    BCM_IF_ERROR_RETURN
        (_bcm_fe2000_mpls_vpn_stp_internal(unit, vpn, port,
                                           &l3_fe, &vpnc));

    if (SOC_IS_SBX_G2P3(unit)) {
        return (_bcm_fe2000_g2p3_mpls_vpn_stp_set_get(unit, vpn,
                                                      port, stp_state,
                                                      vpnc, FALSE));
    } else {
        BCM_DEBUG(BCM_DBG_WARN, ("WARNING %s has not been ported for this microcode (%s:%d)\n", \
                                 FUNCTION_NAME(), __FILE__, __LINE__));
        return BCM_E_UNAVAIL;
    }

    return status;
}

/*
 * Function:
 *      bcm_fe2000_mpls_port_add
 * Purpose:
 *      Add port into existing VPN
 * Parameters:
 *      unit        - FE unit
 *      vpn         - vpn where port is added
 *      mpls_port   - port parameter
 * Returns:
 *      BCM_E_XXX
 * Note:
 *
 */

int
bcm_fe2000_mpls_port_add(int               unit,
                         bcm_vpn_t         vpn,
                         bcm_mpls_port_t  *mpls_port)
{
    _fe2k_l3_fe_instance_t       *l3_fe;
    int                           status = BCM_E_NONE;
    _fe2k_vpn_control_t          *vpnc;
    _fe2k_vpn_sap_t              *vpn_sap = NULL;
    int                           is_failover;

    MPLS_VVERB((_SBX_D(unit, "Enter\n")));

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (!_BCM_MPLS_VPN_VALID_USER_HANDLE(unit, vpn)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (!l3_fe) {
        MPLS_ERR((_SBX_D(unit, "mpls is not initialized\n")));
        status = BCM_E_UNIT;

    } else {
        if (!_FE2K_IS_MPLS_INITIALIZED(l3_fe)) {
            MPLS_ERR((_SBX_D(unit, "mpls is not initialized\n")));
            status = BCM_E_INIT;
        } else {
            status = _bcm_fe2000_validate_mpls_port_add(l3_fe,
                                                        vpn,
                                                        mpls_port);
            if (BCM_SUCCESS(status)) {
                status = _bcm_fe2000_find_mpls_vpncb_by_id(unit,
                                                           l3_fe,
                                                           vpn,
                                                           &vpnc);
                if (BCM_SUCCESS(status)) {
                    /* */
                    if (vpnc->vpn_flags & BCM_MPLS_VPN_L3) {
                        MPLS_ERR((_SBX_D(unit, "MPLS Port functions apply only L2 VPN i.e., VPWS AND VPLS\n")));
                        status = BCM_E_PARAM;

                    } else {
                        if (mpls_port->flags & BCM_MPLS_PORT_WITH_ID) {
                            status = _bcm_fe2000_find_vpn_sap_by_id(l3_fe,
                                                                    vpnc,
                                                                    mpls_port->mpls_port_id,
                                                                    &vpn_sap);
                        } else if (mpls_port->criteria == BCM_MPLS_PORT_MATCH_PORT_VLAN) {
                            status = _bcm_fe2000_find_vpn_sap_by_port_vlan(l3_fe,
                                                                           vpnc,
                                                                           mpls_port->port,
                                                                           mpls_port->match_vlan,
                                                                           &vpn_sap);
                        } else if (mpls_port->criteria == BCM_MPLS_PORT_MATCH_LABEL) {
                            is_failover = mpls_port->flags & \
                                           BCM_MPLS_PORT_FAILOVER;
                            status = _bcm_fe2000_find_vpn_sap_by_label(l3_fe,
                                                        vpnc,
                                                        mpls_port->match_label,
                                                        &vpn_sap,
                                                        is_failover);
                        } else {
                            status = _bcm_fe2000_find_vpn_sap_by_port(l3_fe,
                                                                      vpnc,
                                                                      mpls_port->port,
                                                                      &vpn_sap);
                        }

                        if (BCM_SUCCESS(status)) {

                            /* g2p3 requires the PWE3 label to have the upper bit set */
                            if (vpnc->vpn_flags & BCM_MPLS_VPN_VPLS &&
                                SOC_SBX_CFG(unit)->mplstp_ena == 0 ) {
                                if (!(mpls_port->match_label & _FEK2_L3_MPLS_PWE3_LBL_ID)) {
                                    MPLS_ERR((_SBX_D(unit, "Match label 0x%x does not have bit 0x%x as "
                                                     "required for VPLS\n"),
                                              mpls_port->match_label, _FEK2_L3_MPLS_PWE3_LBL_ID));
                                    status =  BCM_E_PARAM;
                                }
                            }

                            if (BCM_SUCCESS(status)) {
                                if (mpls_port->flags & BCM_MPLS_PORT_REPLACE) {
                                    status = _bcm_fe2000_mpls_port_replace(unit, l3_fe, mpls_port, 
                                                                           vpn, vpnc, vpn_sap);
                                } else {
                                    MPLS_ERR((_SBX_D(unit, "MPLS Port Exists - use replace to update it\n")));
                                    status = BCM_E_EXISTS;
                                }
                            }

                        } else {
                            if (!(mpls_port->flags & BCM_MPLS_PORT_REPLACE)) {
                                
                                int num_vpn_ports = 0;
                                status = BCM_E_NONE;
                                /* Do not allow more than 2 ports to be in VPWS */
                                if (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) {
                                    /* VPWS are configured such that two VSIs are allocated an pointed
                                     * at each other.  VPLS, allocates from the local_gport pool so the
                                     * fti may be used as a PID.
                                     */
                                    _FE2K_ALL_VPN_SAP_PER_VPNC(vpnc, vpn_sap) {
                                        if (!(vpn_sap->vc_mpls_port.flags & BCM_MPLS_PORT_FAILOVER)) {
                                            if (++num_vpn_ports >= 2 &&
                                                !(mpls_port->flags & BCM_MPLS_PORT_FAILOVER) ) {
                                                MPLS_ERR((_SBX_D(unit,
                                                                 "2 ports already exist for vpn(0x%x)\n"),
                                                          vpnc->vpn_id));
                                                status =  BCM_E_FULL;
                                                break;
                                            }
                                        }
                                    } _FE2K_ALL_VPN_SAP_PER_VPNC_END(vpnc, vpn_sap);
                                }
                                
                                if (BCM_SUCCESS(status)) {
                                    status = _bcm_fe2000_mpls_port_add(unit, l3_fe,
                                                                       vpn, vpnc,
                                                                       mpls_port, num_vpn_ports);
                                }
                            }
                            /* replace an !found error handled implcit */
                        } /* check for mpls gport */
                    } /* l3 vpn check */
                } /* invalid vpn */
            } /* valid mpls port */
        } /* INITIALIZED(l3_fe) */
    } /* !l3_fe */

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_fe2000_mpls_port_delete
 * Purpose:
 *      Delete port from existing VPN
 * Parameters:
 *      unit        - FE unit
 *      vpn         - vpn where port is deleted
 *      mpls_port_id- port identifier, gport
 * Returns:
 *      BCM_E_XXX
 * Note:
 *
 */

int
bcm_fe2000_mpls_port_delete(int               unit,
                            bcm_vpn_t         vpn,
                            bcm_gport_t       mpls_port_id)
{
    _fe2k_l3_fe_instance_t       *l3_fe;
    int                           status;
    _fe2k_vpn_control_t          *vpnc;
    _fe2k_vpn_sap_t              *vpn_sap;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (!_BCM_MPLS_VPN_VALID_USER_HANDLE(unit, vpn)) {
        return BCM_E_PARAM;
    }

    if (!BCM_GPORT_IS_MPLS_PORT(mpls_port_id)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_FE2K_IS_MPLS_INITIALIZED(l3_fe)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_fe2000_find_mpls_vpncb_by_id(unit,
                                               l3_fe,
                                               vpn,
                                               &vpnc);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    /* Port functions are only for VPWS AND VPLS */
    if (vpnc->vpn_flags & BCM_MPLS_VPN_L3) {
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    status = _bcm_fe2000_find_vpn_sap_by_id(l3_fe,
                                            vpnc,
                                            mpls_port_id,
                                            &vpn_sap);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    /* if this is a VPWS & has two ports in the VPN, un-connect them
     * so each port own's its own FTE information before deleting
     */
    MPLS_VVERB((_SBX_D(unit, "vpn_flags=0x%x\n"), vpnc->vpn_flags));
    if (vpnc->vpn_flags & BCM_MPLS_VPN_VPWS) {
        int                  num_vpn_ports = 0;
        _fe2k_vpn_sap_t     *tmp_vpn_sap = NULL, *vpnSaps[_BCM_FE2K_VPWS_MAX_SAP];

        _FE2K_ALL_VPN_SAP_PER_VPNC(vpnc, tmp_vpn_sap) {
            if (!(tmp_vpn_sap->vc_mpls_port.flags & BCM_MPLS_PORT_FAILOVER)) {
                if (num_vpn_ports < _BCM_FE2K_VPWS_MAX_SAP) {
                    vpnSaps[num_vpn_ports] = tmp_vpn_sap;
                }
                num_vpn_ports++;
            }
        } _FE2K_ALL_VPN_SAP_PER_VPNC_END(vpnc, tmp_vpn_sap);

        if (num_vpn_ports == 2 && !(vpn_sap->vc_mpls_port.flags & BCM_MPLS_PORT_FAILOVER)) {
            status = _bcm_fe2000_mpls_vpws_fte_connect(l3_fe, vpnSaps, 
                                                       _FE2K_MPLS_PORT_MATCH_DELETE);
            if (BCM_FAILURE(status)) {
                MPLS_ERR((_SBX_D(unit, "error(%s) failed to unlink VPWS\n"),
                          bcm_errmsg(status)));
            }
        }
    }


    status = _bcm_fe2000_mpls_port_delete(unit,
                                          l3_fe,
                                          vpnc,
                                          &vpn_sap);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    L3_UNLOCK(unit);
    return BCM_E_NONE;
}

int
bcm_fe2000_mpls_port_delete_all(int               unit,
                                bcm_vpn_t         vpn)
{
    _fe2k_l3_fe_instance_t       *l3_fe;
    int                           status;
    int                           last_error_status;
    _fe2k_vpn_control_t          *vpnc = NULL;
    _fe2k_vpn_sap_t              *vpn_sap = NULL;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (!_BCM_MPLS_VPN_VALID_USER_HANDLE(unit, vpn)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_FE2K_IS_MPLS_INITIALIZED(l3_fe)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_fe2000_find_mpls_vpncb_by_id(unit,
                                               l3_fe,
                                               vpn,
                                               &vpnc);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    /* Port functions are only for VPWS AND VPLS */
    if (vpnc->vpn_flags & BCM_MPLS_VPN_L3) {
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    last_error_status = BCM_E_NONE;

    _FE2K_ALL_VPN_SAP_PER_VPNC(vpnc, vpn_sap) {

        status = _bcm_fe2000_mpls_port_delete(unit,
                                              l3_fe,
                                              vpnc,
                                              &vpn_sap);
        if (status != BCM_E_NONE) {
            last_error_status = status;
        }

    } _FE2K_ALL_VPN_SAP_PER_VPNC_END(vpnc, vpn_sap);


    L3_UNLOCK(unit);
    return last_error_status;
}

int
bcm_fe2000_mpls_port_get(int               unit,
                         bcm_vpn_t         vpn,
                         bcm_mpls_port_t  *mpls_port)
{
    _fe2k_l3_fe_instance_t       *l3_fe;
    int                           status;
    _fe2k_vpn_control_t          *vpnc;
    _fe2k_vpn_sap_t              *vpn_sap;
    int                          is_failover;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if(!mpls_port) {
        return BCM_E_PARAM;
    }

    if (!_BCM_MPLS_VPN_VALID_USER_HANDLE(unit, vpn)) {
        return BCM_E_PARAM;
    }

    if (mpls_port->flags & BCM_MPLS_PORT_WITH_ID) {
        if(!BCM_GPORT_IS_MPLS_PORT(mpls_port->mpls_port_id)) {
            return BCM_E_PARAM;
        }
    } else {
        switch (mpls_port->criteria) {
        case BCM_MPLS_PORT_MATCH_PORT:
            /* XXX: TBD: Trunk ?? */
            break;
            
        case BCM_MPLS_PORT_MATCH_PORT_VLAN:
            /* XXX: TBD: Trunk ?? */
            if (mpls_port->match_vlan > BCM_VLAN_MAX) {
                MPLS_ERR((_SBX_D(unit, "Invalid vlan passed for MATCH_PORT_VLAN "
                                 "vlan=0x%x\n"), mpls_port->match_vlan));
                
                return BCM_E_PARAM;
            }
            break;

        case BCM_MPLS_PORT_MATCH_LABEL:
            if(SOC_SBX_MPLSTP_ENABLE(unit)) {
                if (!_BCM_FE2000_MPLSTP_LABEL_VALID(mpls_port->match_label)) {
                    MPLS_ERR((_SBX_D(unit, "Invalid label passed for "
                                     "MATCH_LABEL label=0x%x\n"),
                              mpls_port->match_label));

                    return BCM_E_PARAM;
                }
            } else {
                if (!_BCM_LABEL_VALID(mpls_port->match_label)) {
                    MPLS_ERR((_SBX_D(unit, "Invalid label passed for "
                                     "MATCH_LABEL label=0x%x\n"),
                              mpls_port->match_label));
                    
                    return BCM_E_PARAM;
                }
            }
            break;

        default:
            MPLS_ERR((_SBX_D(unit, "Invalid Match criteria=%d\n"),
                      mpls_port->criteria));
            
            return BCM_E_PARAM;
            break;
        }
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_FE2K_IS_MPLS_INITIALIZED(l3_fe)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_fe2000_find_mpls_vpncb_by_id(unit,
                                               l3_fe,
                                               vpn,
                                               &vpnc);
    if (BCM_SUCCESS(status)) {
        /* Port functions are only for VPLS */
        if (vpnc->vpn_flags & BCM_MPLS_VPN_L3) {
            status = BCM_E_PARAM;
        } else {
            if (mpls_port->flags & BCM_MPLS_PORT_WITH_ID) {
                status = _bcm_fe2000_find_vpn_sap_by_id(l3_fe,
                                                        vpnc,
                                                        mpls_port->mpls_port_id,
                                                        &vpn_sap);
            } else {
                is_failover = mpls_port->flags & BCM_MPLS_PORT_FAILOVER;
                status = _bcm_fe2000_find_vpn_sap_by_label(l3_fe,
                                                           vpnc,
                                                           mpls_port->match_label,
                                                           &vpn_sap,
                                                           is_failover);
            }
            if (BCM_SUCCESS(status)) {
                status = _bcm_fe2000_fill_mpls_port_from_vpn_sap(l3_fe,
                                                                 vpn_sap,
                                                                 mpls_port);
            }
        }
    }

    L3_UNLOCK(unit);
    return status;
}

int
bcm_fe2000_mpls_port_get_all(int               unit,
                             bcm_vpn_t         vpn,
                             int               port_max,
                             bcm_mpls_port_t  *port_array,
                             int              *port_count)
{
    _fe2k_l3_fe_instance_t       *l3_fe;
    int                           status;
    int                           ii;
    _fe2k_vpn_control_t          *vpnc;
    _fe2k_vpn_sap_t              *vpn_sap = NULL;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (!_BCM_MPLS_VPN_VALID_USER_HANDLE(unit, vpn)) {
        return BCM_E_PARAM;
    }

    if ((port_max <= 0) || (port_array == NULL)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_FE2K_IS_MPLS_INITIALIZED(l3_fe)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_fe2000_find_mpls_vpncb_by_id(unit,
                                               l3_fe,
                                               vpn,
                                               &vpnc);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    ii = 0;
    _FE2K_ALL_VPN_SAP_PER_VPNC(vpnc, vpn_sap) {

        status =
            _bcm_fe2000_fill_mpls_port_from_vpn_sap(l3_fe,
                                                    vpn_sap,
                                                    &port_array[ii++]);
        if (status != BCM_E_NONE) {
            L3_UNLOCK(unit);
            return status;
        }

        if (ii == port_max) {
            *port_count = ii;
            L3_UNLOCK(unit);
            return BCM_E_NONE;
        }

    } _FE2K_ALL_VPN_SAP_PER_VPNC_END(vpnc, vpn_sap);

    *port_count = ii;

    L3_UNLOCK(unit);
    return BCM_E_NONE;
}


/*
 * Function:
 *      bcm_mpls_tunnel_initiator_clear
 * Purpose:
 *      destroy the mpls tunnel on the interface
 * Parameters:
 *      unit        - Device Number
 *      intf        - The egress L3 interface
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_mpls_tunnel_initiator_clear(int           unit,
                                       bcm_if_t      intf)
{
    int                         status;
    uint32                      ifid;
    _fe2k_l3_fe_instance_t     *l3_fe;
    _fe2k_l3_intf_t            *l3_intf;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (!_FE2K_L3_USER_IFID_VALID(intf)) {
        return (BCM_E_PARAM);
    }

    ifid =  _FE2K_IFID_FROM_USER_HANDLE(intf);

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    if (!_FE2K_IS_MPLS_INITIALIZED(l3_fe)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_fe2000_l3_find_intf_by_ifid(l3_fe, ifid, &l3_intf);
    if (status != BCM_E_NONE) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Invalid interface Id 0x%x\n"), 
                  _FE2K_USER_HANDLE_FROM_IFID(ifid)));
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    status  = _bcm_fe2000_mpls_l3_initiator_clear(l3_fe, l3_intf);

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_mpls_tunnel_initiator_get
 * Purpose:
 *      given a l3a_intf_id, get the mpls tunnel params
 * Parameters:
 *      unit        - (IN)  Device Number
 *      intf        - (IN)  The egress L3 interface Id
 *      label_max   - (IN)  max labels that can be filled up
 *      label_array - (OUT) label array to be returned
 *      label_count - (OUT) number of labels returned
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_mpls_tunnel_initiator_get(int                      unit,
                                     bcm_if_t                 intf,
                                     int                      label_max,
                                     bcm_mpls_egress_label_t *label_array,
                                     int                     *label_count)
{
    int                         status;
    uint32                      ifid;
    _fe2k_l3_fe_instance_t     *l3_fe;
    _fe2k_l3_intf_t            *l3_intf;
    _fe2k_l3_ete_t             *mpls_sw_ete;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if ((intf        == 0)     ||
        (label_max   <= 0)     ||
        (label_array == NULL)  ||
        (label_count == NULL)) {
        return BCM_E_PARAM;
    }

    if (!_FE2K_L3_USER_IFID_VALID(intf)) {
        return (BCM_E_PARAM);
    }

    ifid =  _FE2K_IFID_FROM_USER_HANDLE(intf);

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    if (!_FE2K_IS_MPLS_INITIALIZED(l3_fe)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_fe2000_l3_find_intf_by_ifid(l3_fe,
                                              ifid,
                                              &l3_intf);
    if (status != BCM_E_NONE) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Invalid interface Id 0x%x\n"),
                  _FE2K_USER_HANDLE_FROM_IFID(ifid)));
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    mpls_sw_ete = NULL;
    status      = _bcm_fe2000_get_ete_by_type_on_intf(l3_fe,
                                                      l3_intf,
                                                      _FE2K_L3_ETE__ENCAP_MPLS,
                                                      &mpls_sw_ete);

    if (status != BCM_E_NONE) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls tunnel does not exist on "
                  "interface id (0x%x)\n"),
                  _FE2K_USER_HANDLE_FROM_IFID(ifid)));
        L3_UNLOCK(unit);
        return status;
    }

    status = _bcm_fe2000_fill_mpls_label_array_from_ete_idx(l3_fe,
                                                            mpls_sw_ete->l3_ete_hw_idx.ete_idx,
                                                            label_max,
                                                            label_array,
                                                            label_count);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    L3_UNLOCK(unit);
    return BCM_E_NONE;
}


/*
 * Function:
 *      bcm_mpls_tunnel_initiator_clear_all
 * Purpose:
 *      destroy all the mpls tunnels on the unit
 * Parameters:
 *      unit        - Device Number
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_fe2000_mpls_tunnel_initiator_clear_all(int unit)
{
    int                         last_error_status, status;
    _fe2k_l3_fe_instance_t     *l3_fe;
    _fe2k_l3_intf_t            *l3_intf = NULL;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    if (!_FE2K_IS_MPLS_INITIALIZED(l3_fe)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    last_error_status = BCM_E_NONE;
    _FE2K_ALL_L3INTF(l3_fe, l3_intf) {
        status = _bcm_fe2000_mpls_l3_initiator_clear(l3_fe,
                                                     l3_intf);
        if ((status != BCM_E_NOT_FOUND) || (status != BCM_E_NONE)) {
            last_error_status = status;
            MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) could not clear mpls tunnel on "
                             "interface(0x%x)\n"),
                      bcm_errmsg(status),
                      _FE2K_USER_HANDLE_FROM_IFID(l3_intf->if_info.l3a_intf_id)));
        }
    } _FE2K_ALL_L3INTF_END(l3_fe, l3_intf);

    L3_UNLOCK(unit);
    return last_error_status;
}

/*
 * Function:
 *      bcm_mpls_tunnel_initiator_set
 * Purpose:
 *      Set the MPLS tunnel initiator parameters for an L3 interface.
 * Parameters:
 *      unit        - Device Number
 *      intf        - The egress L3 interface
 *      num_labels  - Number of labels in the array
 *      label_array - Array of MPLS label and header information
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_fe2000_mpls_tunnel_initiator_set(int                      unit,
                                     bcm_if_t                 intf,
                                     int                      num_labels,
                                     bcm_mpls_egress_label_t *label_array)
{

    int                         status;
    uint32                      ifid;
    _fe2k_l3_fe_instance_t     *l3_fe;
    _fe2k_l3_intf_t            *l3_intf;
    _fe2k_l3_ete_t             *l3_ete;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (!_FE2K_L3_USER_IFID_VALID(intf)) {
        MPLS_ERR((_SBX_D(unit, "invalid intf(0x%x)\n"), intf));
        return (BCM_E_PARAM);
    }

    /**
     * Since there is no module in the parameters, therefore function
     * must be called locally
     */
    ifid =  _FE2K_IFID_FROM_USER_HANDLE(intf);

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    if (!_FE2K_IS_MPLS_INITIALIZED(l3_fe)) {
        MPLS_ERR((_SBX_D(unit, "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_fe2000_l3_find_intf_by_ifid(l3_fe,
                                              ifid,
                                              &l3_intf);
    if (status != BCM_E_NONE) {
        MPLS_ERR((_SBX_D(unit, "error(%s) finding interface 0x%x\n"),
                  bcm_errmsg(status), intf));
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    status = _bcm_fe2000_check_mpls_initiator_params(l3_fe,
                                                     l3_intf,
                                                     num_labels,
                                                     label_array);
    if (status != BCM_E_NONE) {
        /* error printed already */
        L3_UNLOCK(unit);
        return status;
    }

    /*
     * We have already made sure that there is at most
     * one l3-ete (i.e. no l3-ete or one l3-ete).
     */
    status = _bcm_fe2000_get_ete_by_type_on_intf(l3_fe,
                                                 l3_intf,
                                                 _FE2K_L3_ETE__UCAST_IP,
                                                 &l3_ete);
    if ((status != BCM_E_NONE) && (status != BCM_E_NOT_FOUND)) {
        MPLS_ERR((_SBX_D(unit, "error(%s) finding ipv4 ete on intf(0x%x)\n"),
                  bcm_errmsg(status), intf));
        L3_UNLOCK(unit);
        return status;
    }

    /*
     * If a v4-ete has not been created yet, we save the label info.
     * dmac and vidop needs to come from the v4-ete
     */
    status = _bcm_fe2000_mpls_tunnel_initiator_set(l3_fe,
                                                   l3_intf,
                                                   l3_ete, /* Could be NULL */
                                                   num_labels,
                                                   label_array);
    if (status != BCM_E_NONE) {
        MPLS_ERR((_SBX_D(unit, "error(%s) enabling mpls tunnel on intf(0x%x)\n"),
                  bcm_errmsg(status), intf));
        L3_UNLOCK(unit);
        return status;
    }

    l3_intf->if_flags |=_FE2K_L3_MPLS_TUNNEL_SET;

    L3_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_fe2000_mpls_tunnel_switch_add
 * Purpose:
 *      destroy all the mpls tunnels on the unit
 * Parameters:
 *      unit        - Device Number
 *      info        - label switching info (only LSR/Egress LER for L3 VPN)
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *     Check info->label for routing label range in PPE config;
 *     Index into Label2Etc table using label :
 *     LSR Case:
 *       Set labelMode to lsr
 *       Set labelQos to ??
 *       Derive real FTidx from info->egress_if and
 *       populate in ftidx field
 *
 *       The MPLS egress object handle passed to this API must
 *       be created as mentioned above; effectively egress_if gives
 *       the FTE handle that points to an MPLS ETE that contains the swap
 *       label (swap_label). For PHP case, the MPLS ETE must be populated to
 *       set Php field to 1
 *
 *     LER case:
 *       Set labelMode to ler
 *       Set labelUnion (vlan) tbo a default Bridging context (VSI) -
 *       aka SBX 16b VLAN space - which has the default/global VRF
 */

int
bcm_fe2000_mpls_tunnel_switch_add(int                       unit,
                                  bcm_mpls_tunnel_switch_t *info)
{
    int                         status;
    _fe2k_l3_fe_instance_t     *l3_fe;
    bcm_mpls_tunnel_switch_t   *swinfo;
    bcm_mpls_tunnel_switch_t   *datum;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (info == NULL) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    if (!_FE2K_IS_MPLS_INITIALIZED(l3_fe)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    swinfo = sal_alloc(sizeof(bcm_mpls_tunnel_switch_t),
                      "MPLS-tunnel-switch-info");
    if (swinfo == NULL) {
        return BCM_E_MEMORY;
    }


    /* User does not create BCM_L3_VRF_DEFAULT */
    if (info->vpn == BCM_L3_VRF_DEFAULT) {
        info->vpn = l3_fe->fe_vsi_default_vrf;
    }

    status = _bcm_fe2000_mpls_validate_tunnel_switch_add(l3_fe, info);
    if (status != BCM_E_NONE) {
        if (info->vpn == l3_fe->fe_vsi_default_vrf) {
            info->vpn = BCM_L3_VRF_DEFAULT;
        }
        sal_free(swinfo);
        L3_UNLOCK(unit);
        return  status;
    }

    status = _bcm_fe2000_mpls_tunnel_switch_update(l3_fe, info);
    if (info->vpn == l3_fe->fe_vsi_default_vrf) {
        info->vpn = BCM_L3_VRF_DEFAULT;
    }

    if (BCM_SUCCESS(status)) {
        /* if successful, insert into the db */
        status = shr_htb_find(mpls_switch_info_db[unit], 
                              (shr_htb_key_t) &info->label,
                              (shr_htb_data_t*)&datum, 0);
        if (status  == BCM_E_NOT_FOUND) {
            *swinfo = *info;
            status = shr_htb_insert(mpls_switch_info_db[unit],
                                     (shr_htb_key_t) &info->label,
                                     (void*) swinfo);        
            if (BCM_FAILURE(status)) {
                MPLS_ERR((_SBX_D(unit, "error(%s) Updating switch_info Dbase (0x%x)\n"),
                         bcm_errmsg(status), info->label));
                sal_free(swinfo);
            }
        } else {
            /* overwrite existing */
            *datum = *info;
            sal_free(swinfo);
        }
    } else {
        sal_free(swinfo);
    }

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_fe2000_mpls_tunnel_switch_delete
 * Purpose:
 *      delete earlier setup label switch path
 * Parameters:
 *      unit        - Device Number
 *      info        - label switching info (only LSR/Egress LER for L3 VPN)
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

int
bcm_fe2000_mpls_tunnel_switch_delete(int                       unit,
                                     bcm_mpls_tunnel_switch_t *info)
{
    int                         status;
    _fe2k_l3_fe_instance_t     *l3_fe;
    bcm_mpls_tunnel_switch_t   *datum;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if ((info == NULL) || (info->label == 0)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    if (!_FE2K_IS_MPLS_INITIALIZED(l3_fe)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_fe2000_mpls_tunnel_switch_delete(l3_fe,
                                                   info);

    /* find and destroy the tunnel_switch info */
    status = shr_htb_find(mpls_switch_info_db[unit], 
                          (shr_htb_key_t) &info->label,
                          (shr_htb_data_t*)&datum, 1);
    if (BCM_FAILURE(status)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "failed to free MPLS switch_info dbase element: %d (%s)\n"),
                  status, bcm_errmsg(status)));
    } else {
        sal_free(datum);
    }

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_fe2000_mpls_tunnel_switch_delete_all
 * Purpose:
 *      delete all earlier setup label switch paths
 * Parameters:
 *      unit        - Device Number
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

int
bcm_fe2000_mpls_tunnel_switch_delete_all(int unit)
{
    int                         status = BCM_E_UNAVAIL;
    _fe2k_l3_fe_instance_t     *l3_fe;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    if (!_FE2K_IS_MPLS_INITIALIZED(l3_fe)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    status = _bcm_fe2000_mpls_tunnel_switch_delete_all(l3_fe);

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_fe2000_mpls_tunnel_switch_get
 * Purpose:
 *      get label switch path information
 * Parameters:
 *      unit        - Device Number
 *      info        - tunnel switch info
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

int
bcm_fe2000_mpls_tunnel_switch_get(int               unit,
                                  bcm_mpls_tunnel_switch_t *info)
{
    int                         status;
    _fe2k_l3_fe_instance_t     *l3_fe;
    bcm_mpls_tunnel_switch_t   *datum;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (info == NULL) {
        return BCM_E_PARAM;
    }

    if (info->label == 0) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    if (!_FE2K_IS_MPLS_INITIALIZED(l3_fe)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    /* find the tunnel switch info */
    status = shr_htb_find(mpls_switch_info_db[unit], 
                          (shr_htb_key_t) &info->label,
                          (shr_htb_data_t *)&datum, 0);
    if (BCM_FAILURE(status)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Tunnel switch data not found for label: %d (%s)\n"),
                  status, bcm_errmsg(status)));
    } else {
        *info = *datum;
    }
    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_fe2000_mpls_tunnel_switch_traverse
 * Purpose:
 *      traverse label switch(LSR/LER) information
 * Parameters:
 *      unit        - Device Number
 *      cb          - function to be called
 *      user_data   - user provided info sent to cb
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_mpls_tunnel_switch_traverse(int               unit,
                                       bcm_mpls_tunnel_switch_traverse_cb  cb,
                                       void             *user_data)
{
    int                         rv;
    _fe2k_l3_fe_instance_t     *l3_fe;
    bcm_mpls_tunnel_switch_t    info;
    bcm_mpls_label_t            label;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if (cb == NULL) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (!_FE2K_IS_MPLS_INITIALIZED(l3_fe)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "mpls is not initialized\n")));
        L3_UNLOCK(unit);
        return BCM_E_INIT;
    }

    for (label = 0; label < _FE2K_G2P3_MAX_LABELS; label++) {
        bcm_mpls_tunnel_switch_t_init(&info);
        info.label = label;
        rv = bcm_fe2000_mpls_tunnel_switch_get(l3_fe->fe_unit, &info);
        if (BCM_SUCCESS(rv)) {
            cb(unit, &info, user_data);
        }
    }

    L3_UNLOCK(unit);

    return BCM_E_NONE;
}

/*
 *  Function
 *      bcm_fe2000_mpls_exp_map_create
 *  Purpose
 *      Allocate a QOS_MAP or EGR_REMARK resource for configuration of MPLS
 *      PRI based translation on ingress or egress.
 *  Parameters
 *      int unit = the unit number
 *      uint32 flags = flags for the operation
 *      int *exp_map_id = pointer to where to put the ID or the desired ID
 *  Returns
 *      bcm_error_t cast as int
 *          BCM_E_NONE = success
 *          BCM_E_* = otherwise
 *  Notes
 *      Resource pools conflict with port_vlan and IPv4 priority maps.  Unless
 *      the same requests are made of all units at the same time, both of this
 *      API and the port version and the IPv4 version, it is possible that
 *      BCM_MPLS_EXP_MAP_WITH_ID requests will fail because other functions
 *      have already used an item.
 *      Does not support allocating both BCM_MPLS_EXP_MAP_INGRESS and
 *      BCM_MPLS_EXP_MAP_EGRESS at the same time, since they come from
 *      different pools and may therefore not have the same element number
 *      available in both pools.
 *      This does not offer any support for the SBX feature where fabric and
 *      remark values can differ (they are always the same here).
 */
int
bcm_fe2000_mpls_exp_map_create(int unit,
                               uint32 flags,
                               int *exp_map_id)
{
    _fe2k_l3_fe_instance_t  *l3_fe;
    uint32                  expProfile = 0;
    uint32                  expMap = 0;
    soc_sbx_g2p3_qos_t      p3qosProfile;
    soc_sbx_g2p3_remark_t   p3egrRemark;
    unsigned int            pri,cfi;
    int                     result = BCM_E_INTERNAL;
    int                     index;
    int                     freeAllocs = TRUE;

    MPLS_VERB((_SBX_D(unit, "(0x%08X,*): enter\n"), flags));

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    /* Check parameters */
    if (!exp_map_id) {
        /* NULL pointer is unacceptable */
        MPLS_ERR((_SBX_D(unit, "NULL pointer to exp_map_id\n")));
        return BCM_E_PARAM;
    }
    if (flags & (~(BCM_MPLS_EXP_MAP_WITH_ID |
                   BCM_MPLS_EXP_MAP_INGRESS |
                   BCM_MPLS_EXP_MAP_EGRESS ))) {
        /* An unsupported flag is set */
        MPLS_ERR((_SBX_D(unit, "an unexpected flag is set\n")));
        return BCM_E_PARAM;
    }
    if (((flags & (BCM_MPLS_EXP_MAP_INGRESS | BCM_MPLS_EXP_MAP_EGRESS)) ==
         (BCM_MPLS_EXP_MAP_INGRESS | BCM_MPLS_EXP_MAP_EGRESS)) ||
        (0 == (flags & (BCM_MPLS_EXP_MAP_INGRESS | BCM_MPLS_EXP_MAP_EGRESS)))) {
        /*
         *  Caller wanted either both of or neither of ingress and egress; we
         *  don't support either case here.
         */
        MPLS_ERR((_SBX_D(unit, "must specify exactly one of ingress,egress\n")));
        return BCM_E_PARAM;
    }

    
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    /* get the context information */
    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    /* allocate the map as requested */
    if (flags & BCM_MPLS_EXP_MAP_INGRESS) {
        result = BCM_E_NONE; /* be optimistic */

        /* now need a QosMap */
        MPLS_VVERB((_SBX_D(unit, "allocate any available QosMap\n")));
        result = _sbx_gu2_resource_alloc(unit,
                                         SBX_GU2K_USR_RES_QOS_PROFILE,
                                         1,
                                         &expMap,
                                         0);
        if (BCM_E_NONE == result) {
            /* got the map */
            /* set the needed values to the profile */
            
            switch (SOC_SBX_CONTROL(unit)->ucodetype) {

            case SOC_SBX_UCODE_TYPE_G2P3:
                MPLS_VVERB((_SBX_D(unit, "allocated QosMap[0x%08X]; prep it\n"),
                            expMap));
                /* initialise the profile */
                soc_sbx_g2p3_qos_t_init(&p3qosProfile);
                for (pri = 0;
                     (pri < 8) && (BCM_E_NONE == result);
                     pri++) {
                    for (cfi = 0;
                         (cfi < 2) && (BCM_E_NONE == result);
                         cfi++) {
                        result = soc_sbx_g2p3_qos_set(unit,
                                                      cfi,
                                                      pri,
                                                      expMap,
                                                      &p3qosProfile);
                    } /* for (all valid CFI values) */
                } /* for (all valid PRI values) */
                if (BCM_E_NONE == result) {
                    *exp_map_id = _MPLS_EXPMAP_HANDLE_MAKE_INGR(expMap);
                } else { /* if (BCM_E_NONE == result) */
                    MPLS_ERR((_SBX_D(unit, "unable to clear :QosMap[0x%08X]: %d (%s)\n"),
                              expMap, result, _SHR_ERRMSG(result)));
                } /* if (BCM_E_NONE == result) */
                break;

            default:
                MPLS_ERR((_SBX_D(unit, "unknown microcode %d\n"),
                          SOC_SBX_CONTROL(unit)->ucodetype));
                result = BCM_E_UNAVAIL;
            } /* switch (SOC_SBX_CONTROL(unit)->ucodetype) */
        } /* if (BCM_E_NONE == result) */
        if ((BCM_E_NONE != result) && freeAllocs) {
            /* something went wrong; free anything we managed to acquire */
            if (expProfile) {
                /* allocated a qosProfile; free it */
                _sbx_gu2_resource_free(unit,
                                       SBX_GU2K_USR_RES_QOS_PROFILE,
                                       1,
                                       &expProfile,
                                       0);
            }
            if (expMap) {
                /* allocated a qosMap; free it */
                _sbx_gu2_resource_free(unit,
                                       SBX_GU2K_USR_RES_QOS_PROFILE,
                                       1,
                                       &expMap,
                                       0);
            }
        } /* if (BCM_E_NONE == result) */
    } else if (flags & BCM_MPLS_EXP_MAP_EGRESS) {
        /* Requested an egress map */
        if (flags & BCM_MPLS_EXP_MAP_WITH_ID) {
            /* Requested a specific egress map */
            if (_MPLS_EXPMAP_HANDLE_IS_EGR(*exp_map_id)) {
                /* requested handle is valid egress ID */
                expMap = _MPLS_EXPMAP_HANDLE_DATA(*exp_map_id);
                MPLS_VVERB((_SBX_D(unit, "allocate specific egrRemark[0x%08X]\n"),
                            *exp_map_id));
                result = _sbx_gu2_resource_alloc(unit,
                                                 SBX_GU2K_USR_RES_QOS_EGR_REMARK,
                                                 1,
                                                 &expMap,
                                                 _SBX_GU2_RES_FLAGS_RESERVE);
                if (BCM_E_NONE != result) {
                    /* something went wrong reserving the element */
                    MPLS_ERR((_SBX_D(unit, "unable to reserve egrRemark[0x%08X]: %d (%s)\n"),
                              *exp_map_id,
                              result,
                              _SHR_ERRMSG(result)));
                }
            } else { /* (_MPLS_EXPMAP_HANDLE_IS_EGR(*exp_map_id)) */
                /* requested handle is not valid egress ID */
                MPLS_ERR((_SBX_D(unit, "requested ID Ox%08X is not valid for egress\n"),
                          *exp_map_id));
                result = BCM_E_PARAM;
            } /* (_MPLS_EXPMAP_HANDLE_IS_EGR(*exp_map_id)) */
            /* whatever was passed in should not be freed */
            freeAllocs = FALSE;
        } else { /* if (flags & BCM_MPLS_EXP_MAP_WITH_ID) */
            /* Happy with any free egress map */
            MPLS_VVERB((_SBX_D(unit, "allocate any available egrRemark\n")));
            result = _sbx_gu2_resource_alloc(unit,
                                             SBX_GU2K_USR_RES_QOS_EGR_REMARK,
                                             1,
                                             &expMap,
                                             0);
            if (BCM_E_NONE != result) {
                /* something went wrong trying to allocate the map */
                MPLS_ERR((_SBX_D(unit, "unable to allocate an egrRemark\n")));
            }
        } /* if (flags & BCM_MPLS_EXP_MAP_WITH_ID) */

        if (BCM_E_NONE == result) {
            /* got the map */
            MPLS_VVERB((_SBX_D(unit, "allocated egrRemark[0x%08X]; prepare it\n"),
                        expMap));
            switch (SOC_SBX_CONTROL(unit)->ucodetype) {

            case SOC_SBX_UCODE_TYPE_G2P3:
                soc_sbx_g2p3_remark_t_init(&p3egrRemark);
                for (pri = 0;
                     (pri < 8) && (BCM_E_NONE == result);
                     pri++) {
                    for (index = 0;
                         (index < 4) && (BCM_E_NONE == result);
                         index++) {
                        for (cfi = 0;
                             (cfi < 2) && (BCM_E_NONE == result);
                             cfi++) {
                            result = soc_sbx_g2p3_remark_set(unit,
                                                             cfi,
                                                             index,
                                                             pri,
                                                             expMap,
                                                             &p3egrRemark);
                        } /* for (all ECN value) */
                    } /* for (all DP values) */
                } /* for (all CoS values) */
                if (BCM_E_NONE == result) {
                    *exp_map_id = _MPLS_EXPMAP_HANDLE_MAKE_EGR(expMap);
                } else { /* if (BCM_E_NONE == result) */
                    MPLS_ERR((_SBX_D(unit, "unable to clear :egrRemark[0x%08X]: %d (%s)\n"),
                              expMap, result, _SHR_ERRMSG(result)));
                } /* if (BCM_E_NONE == result) */
                break;

            default:
                SBX_UNKNOWN_UCODE_WARN(unit);
                result = BCM_E_UNAVAIL;
            } /* switch (SOC_SBX_CONTROL(unit)->ucodetype) */
        } /* if (BCM_E_NONE == result) */
        if ((BCM_E_NONE != result) && freeAllocs) {
            /* something went wrong; free any acquired resources */
            if (expMap) {
                /* got an egressRemarkEntry; free it */
                _sbx_gu2_resource_free(unit,
                                       SBX_GU2K_USR_RES_QOS_EGR_REMARK,
                                       1,
                                       &expMap,
                                       0);
            }
        } /* if (BCM_E_NONE == result) */
    } /* flags contains ingress or egress */

    
    L3_UNLOCK(unit);

    MPLS_VVERB((_SBX_D(unit, "(%08X,&(%08X)): return %d (%s)\n"),
                flags,
                *exp_map_id,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *      bcm_fe2000_mpls_exp_map_destroy
 *  Purpose
 *      Free a QOS_MAP or EGR_REMARK resource for configuration of MPLS
 *      PRI based translation on ingress or egress.
 *  Parameters
 *      int unit = the unit number
 *      int exp_map_id = the ID to be freed
 *  Returns
 *      bcm_error_t cast as int
 *          BCM_E_NONE = success
 *          BCM_E_* = otherwise
 *  Notes
 *      This does not offer any support for the SBX feature where fabric and
 *      remark values can differ (it assumes they are always the same).
 */
int
bcm_fe2000_mpls_exp_map_destroy(int unit,
                                int exp_map_id)
{
    _fe2k_l3_fe_instance_t  *l3_fe;
    uint32                  expMap = 0;
    uint32                  expProfile = 0;
    int                     result = BCM_E_INTERNAL;
    int                     tmpRes;

    MPLS_VERB((_SBX_D(unit, "(0x%08X): enter\n"), exp_map_id));

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    /* Check parameters */
    if (!_MPLS_EXPMAP_HANDLE_IS_VALID(exp_map_id)) {
        /* invalid handle */
        MPLS_ERR((_SBX_D(unit, "0x%08X is not a valid MPLS EXP map ID\n"),
                  exp_map_id));
        return BCM_E_PARAM;
    }

    
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    /* get the context information */
    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    /* free the map as requested */
    if (_MPLS_EXPMAP_HANDLE_IS_INGR(exp_map_id)) {
        /* freeing an ingress map */
        expProfile = _MPLS_EXPMAP_HANDLE_DATA(exp_map_id);
        switch (SOC_SBX_CONTROL(unit)->ucodetype) {

        case SOC_SBX_UCODE_TYPE_G2P3:
            /* don't need to do anything to find map -- single layer */
            expMap = expProfile;
            expProfile = 0;
            result = BCM_E_NONE;
            break;

        default:
            SBX_UNKNOWN_UCODE_WARN(unit);
            result = BCM_E_UNAVAIL;
        } /* switch (SOC_SBX_CONTROL(unit)->ucodetype) */
        if ((BCM_E_NONE == result) && expMap) {
            MPLS_VVERB((_SBX_D(unit, "free qosMap[0x%08X]\n"), expMap));
            tmpRes = _sbx_gu2_resource_free(unit,
                                            SBX_GU2K_USR_RES_QOS_PROFILE,
                                            1,
                                            &expMap,
                                            0);
            if (BCM_E_NONE != tmpRes) {
                /* something went wrong */
                MPLS_ERR((_SBX_D(unit, "unable to free qosMap[0x%08X]: %d (%s)\n"),
                          expMap, result, _SHR_ERRMSG(result)));
                if (BCM_E_NONE == result) {
                    /* don't have an error yet, so set it */
                    result = tmpRes;
                }
            } /* if (BCM_E_NONE != tmpRes) */
        } /* if ((BCM_E_NONE == result) && expMap) */
    } else if (_MPLS_EXPMAP_HANDLE_IS_EGR(exp_map_id)) {
        /* freeing an egress map */
        expMap = _MPLS_EXPMAP_HANDLE_DATA(exp_map_id);
        MPLS_VVERB((_SBX_D(unit, "free egrRemap[0x%08X]\n"), expMap));
        /*
         *  This one's easy, since there is only a single resource to manage,
         *  and there's no chaining, so either getting rid of it works or not.
         */
        result = _sbx_gu2_resource_free(unit,
                                        SBX_GU2K_USR_RES_QOS_EGR_REMARK,
                                        1,
                                        &expMap,
                                        0);
        if (BCM_E_NONE != result) {
            MPLS_ERR((_SBX_D(unit, "unable to free egrRemark[0x%08X]: %d (%s)\n"),
                      expMap, result, _SHR_ERRMSG(result)));
        }
    } /* map is ingress or egress */

    
    L3_UNLOCK(unit);

    MPLS_VVERB((_SBX_D(unit, "(0x%08X): return %d (%s)\n"),
                exp_map_id,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *      bcm_fe2000_mpls_exp_map_set
 *  Purpose
 *      Adjust a qosMap or egrRemark entry to reflect the specified settings.
 *  Parameters
 *      int unit = the unit number
 *      int exp_map_id = the ID to be updated
 *      bcm_mpls_exp_map_t *exp_map = pointer to the new setting
 *  Returns
 *      bcm_error_t cast as int
 *          BCM_E_NONE = success
 *          BCM_E_* = otherwise
 *  Notes
 *      This does not offer any support for the SBX feature where fabric and
 *      remark values can differ (it assumes they are always the same).
 *
 *      On G2P2, if setting for ingress, and qosProfile does not indicate to
 *      use the qosMap for MPLS, this will enable using the qosMap for MPLS and
 *      rewrite the qosProfile.  If the qosProfile already indicates to use the
 *      qosMap, this will not write back to the qosProfile.
 *
 *      When setting ingress: exp_map->exp is the key; exp_map->priority and
 *      exp_map->color are the data that are set (exp_map->pkt_pri and
 *      exp_map->pkt_cfi are ignored).
 *
 *      When setting egress: exp_map->priority and exp_map->color are the key;
 *      exp_map->pri, exp_map->pkt_pri, exp_map->pkt_cfi are the data to set.
 */
int
bcm_fe2000_mpls_exp_map_set(int unit,
                            int exp_map_id,
                            bcm_mpls_exp_map_t *exp_map)
{
    _fe2k_l3_fe_instance_t  *l3_fe;
    uint32                  expMap;
    uint32                  expProfile = 0;
    soc_sbx_g2p3_qos_t      p3qosProfile;
    soc_sbx_g2p3_remark_t   p3egrRemark;
    int                     result = BCM_E_NONE;

    MPLS_VERB((_SBX_D(unit, "(0x%08X,0x%08X): enter\n"),
               exp_map_id, (unsigned int)exp_map));


    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    /* Check parameters */
    if (!_MPLS_EXPMAP_HANDLE_IS_VALID(exp_map_id)) {
        /* invalid handle */
        MPLS_ERR((_SBX_D(unit, "0x%08X is not a valid MPLS EXP map ID\n"),
                  exp_map_id));
        return BCM_E_PARAM;
    }
    if (!exp_map) {
        /* invalid pointer to map data */
        MPLS_ERR((_SBX_D(unit, "NULL is not a valid exp_map pointer\n")));
        return BCM_E_PARAM;
    }

    
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    /* get the context information */
    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    /* adjust the map as requested */
    if (_MPLS_EXPMAP_HANDLE_IS_INGR(exp_map_id)) {
        /* setting ingress mapping */
        /* make sure EXP is valid */
        if (7 < exp_map->exp) {
            /* invalid EXP value */
            result = BCM_E_PARAM;
            MPLS_ERR((_SBX_D(unit, "invalid value 0x%x for exp\n"), exp_map->exp));
        }
        /* make sure priority is valid */
        if ((0 > exp_map->priority) || (15 < exp_map->priority)) {
            /* invalid priority value */
            result = BCM_E_PARAM;
            MPLS_ERR((_SBX_D(unit, "invalid value %d for priority\n"), 
                      exp_map->priority));
        }
        /* verify that colour is valid */
        if ((bcmColorGreen > exp_map->color) ||
            (bcmColorCount <= exp_map->color)) {
            /* invalid colour */
            result = BCM_E_PARAM;
            MPLS_ERR((_SBX_D(unit, " invalid value %d for color\n"),
                      exp_map->color));
        }
        switch (SOC_SBX_CONTROL(unit)->ucodetype) {

        case SOC_SBX_UCODE_TYPE_G2P3:
            if (BCM_E_NONE == result) {
                expMap = _MPLS_EXPMAP_HANDLE_DATA(exp_map_id);
                soc_sbx_g2p3_qos_t_init(&p3qosProfile);
                
                p3qosProfile.mefcos = exp_map->priority;
                p3qosProfile.fcos = exp_map->priority;
                p3qosProfile.cos = exp_map->priority;
                p3qosProfile.dp = exp_map->color;
                
                p3qosProfile.e = FALSE;
                result = soc_sbx_g2p3_qos_set(unit,
                                              0,
                                              exp_map->exp,
                                              expMap,
                                              &p3qosProfile);
                if (BCM_E_NONE != result) {
                    MPLS_ERR((_SBX_D(unit, "unable to set qosMap[0x%08X].exp[0x%X]:"
                                     " %d (%s)\n"),
                              expMap, exp_map->exp,
                              result, _SHR_ERRMSG(result)));
                }
            }
            break;

        default:
            SBX_UNKNOWN_UCODE_WARN(unit);
            result = BCM_E_UNAVAIL;
        } /* switch (SOC_SBX_CONTROL(unit)->ucodetype) */
    } else if (_MPLS_EXPMAP_HANDLE_IS_EGR(exp_map_id)) {
        /* setting egress mapping */
        /* make sure EXP is valid */
        if (7 < exp_map->exp) {
            /* invalid EXP value */
            result = BCM_E_PARAM;
            MPLS_ERR((_SBX_D(unit, "invalid value 0x%X for exp\n"), exp_map->exp));
        }
        /* make sure pri is valid */
        if (7 < exp_map->pkt_pri) {
            /* invalid priority value */
            result = BCM_E_PARAM;
            MPLS_ERR((_SBX_D(unit, "invalid value %d for pkt_pri\n"),
                      exp_map->pkt_pri));
        }
        /* make sure cfi is valid */
        if (1 < exp_map->pkt_cfi) {
            /* invalid CFI value */
            result = BCM_E_PARAM;
            MPLS_ERR((_SBX_D(unit, "invalid value %d for pkt_cfi\n"),
                      exp_map->pkt_cfi));
        }
        /* make sure priority is valid */
        if ((0 > exp_map->priority) || (15 < exp_map->priority)) {
            /* invalid priority value */
            result = BCM_E_PARAM;
            MPLS_ERR((_SBX_D(unit, "invalid value %d for priority\n"),
                      exp_map->priority));
        }
        /* make sure color is valid */
        if ((bcmColorGreen > exp_map->color) ||
            (bcmColorCount <= exp_map->color)) {
            /* invalid colour */
            result = BCM_E_PARAM;
            MPLS_ERR((_SBX_D(unit, "invalid value %d for color\n"),
                      exp_map->color));
        }
        /* extract the egress map ID from the handle */
        expProfile = _MPLS_EXPMAP_HANDLE_DATA(exp_map_id);
        expMap = (exp_map->priority << 2) | (exp_map->color);
        switch (SOC_SBX_CONTROL(unit)->ucodetype) {

        case SOC_SBX_UCODE_TYPE_G2P3:
            if (BCM_E_NONE == result) {
                /* fill in the data */
                soc_sbx_g2p3_remark_t_init(&p3egrRemark);
                p3egrRemark.exp = exp_map->exp;
                p3egrRemark.cfi = exp_map->pkt_cfi;
                p3egrRemark.pri = exp_map->pkt_pri;
                for (expMap = 0;
                     (expMap < 2) && (BCM_E_NONE == result);
                     expMap++) {
                    result = soc_sbx_g2p3_remark_set(unit,
                                                     expMap,
                                                     exp_map->color,
                                                     exp_map->priority,
                                                     expProfile,
                                                     &p3egrRemark);
                } /* for (both ECN values) */
                if (BCM_E_NONE != result) {
                    MPLS_ERR((_SBX_D(unit, "unable to write"
                              " egrRemark[0x%08X].[%d].[%d].[%d]:"
                              " %d (%s)\n"),
                              expMap,
                              exp_map->priority,
                              exp_map->color,
                              expProfile,
                              result,
                              _SHR_ERRMSG(result)));
                } /* if (BCM_E_NONE != result) */
            } /* if (BCM_E_NONE == result) */
            break;

        default:
            SBX_UNKNOWN_UCODE_WARN(unit);
            result = BCM_E_UNAVAIL;
        } /* switch (SOC_SBX_CONTROL(unit)->ucodetype) */
    } /* map is ingress or egress */

    
    L3_UNLOCK(unit);

    MPLS_VVERB((_SBX_D(unit, "(0x%08X,0x%08X->(%d,%d,%d,%d,%d)): return %d (%s)\n"),
                exp_map_id,
                (unsigned int)exp_map,
                exp_map->priority,
                exp_map->color,
                exp_map->exp,
                exp_map->pkt_pri,
                exp_map->pkt_cfi,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Function
 *      bcm_fe2000_mpls_exp_map_get
 *  Purpose
 *      Fetch a qosMap or egrRemark entry to read the specified settings.
 *  Parameters
 *      int unit = the unit number
 *      int exp_map_id = the ID to be updated
 *      bcm_mpls_exp_map_t *exp_map = pointer to the setting buffer
 *  Returns
 *      bcm_error_t cast as int
 *          BCM_E_NONE = success
 *          BCM_E_* = otherwise
 *  Notes
 *      This does not offer any support for the SBX feature where fabric and
 *      remark values can differ (it assumes they are always the same).
 *
 *      On G2P2, if getting for ingress, and qosProfile does not indicate to
 *      use the qosMap for MPLS, this will return BCM_E_EMPTY.
 *
 *      When getting ingress: exp_map->exp is the key; exp_map->priority and
 *      exp_map->color are the data that are read (exp_map->pkt_pri and
 *      exp_map->pkt_cfi are ignored).
 *
 *      When getting egress: exp_map->priority and exp_map->color are the key;
 *      exp_map->pri, exp_map->pkt_pri, exp_map->pkt_cfi are the data read.
 */
int
bcm_fe2000_mpls_exp_map_get(int unit,
                            int exp_map_id,
                            bcm_mpls_exp_map_t *exp_map)
{
    _fe2k_l3_fe_instance_t  *l3_fe;
    uint32                  expMap;
    soc_sbx_g2p3_qos_t      p3qosProfile;
    soc_sbx_g2p3_remark_t   p3egrRemark;
    int                     result = BCM_E_NONE;

    MPLS_VERB((_SBX_D(unit, "(0x%08X,0x%08X): enter\n"),
               exp_map_id, (unsigned int)exp_map));

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    /* Check parameters */
    if (!_MPLS_EXPMAP_HANDLE_IS_VALID(exp_map_id)) {
        /* invalid handle */
        MPLS_ERR((_SBX_D(unit, "0x%08X is not a valid MPLS EXP map ID\n"),
                  exp_map_id));
        return BCM_E_PARAM;
    }
    if (!exp_map) {
        /* invalid pointer to map data */
        MPLS_ERR((_SBX_D(unit, "NULL is not a valid exp_map pointer\n")));
        return BCM_E_PARAM;
    }

    
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    /* get the context information */
    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    /* fetch the map as requested */
    if (_MPLS_EXPMAP_HANDLE_IS_INGR(exp_map_id)) {
        /* getting ingress mapping */
        /* make sure EXP is valid */
        if (7 < exp_map->exp) {
            /* invalid EXP value */
            result = BCM_E_PARAM;
            MPLS_ERR((_SBX_D(unit, "invalid value %d for exp\n"),
                      exp_map->exp));
        }
        switch (SOC_SBX_CONTROL(unit)->ucodetype) {

        case SOC_SBX_UCODE_TYPE_G2P3:
            if (BCM_E_NONE == result) {
                /* extract the ingress map ID from the handle */
                expMap = _MPLS_EXPMAP_HANDLE_DATA(exp_map_id);
                MPLS_VVERB((_SBX_D(unit, "get current qosMap[0x%08X].exp[%d]\n"),
                            expMap,
                            exp_map->exp));
                result = soc_sbx_g2p3_qos_get(unit,
                                              0,
                                              exp_map->exp,
                                              expMap,
                                              &p3qosProfile);
                if (BCM_E_NONE == result) {
                    exp_map->priority = p3qosProfile.cos;
                    exp_map->color = p3qosProfile.dp;
                } else { /* if (BCM_E_NONE == result) */
                    MPLS_ERR((_SBX_D(unit, "unable to read qosMap[0x%08X].exp[%d]:"
                                     " %d (%s)\n"),
                              expMap,
                              exp_map->exp,
                              result,
                              _SHR_ERRMSG(result)));
                } /* if (BCM_E_NONE == result) */
            } /* if (BCM_E_NONE == result) */
            break;

        default:
            SBX_UNKNOWN_UCODE_WARN(unit);
            result = BCM_E_UNAVAIL;
        } /* switch (SOC_SBX_CONTROL(unit)->ucodetype) */
    } else if (_MPLS_EXPMAP_HANDLE_IS_EGR(exp_map_id)) {
        /* getting egress mapping */
        /* make sure priority is valid */
        if ((0 > exp_map->priority) || (15 < exp_map->priority)) {
            /* invalid priority value */
            result = BCM_E_PARAM;
            MPLS_ERR((_SBX_D(unit, "invalid value %d for priority\n"),
                      exp_map->priority));
        }
        /* make sure color is valid */
        if ((bcmColorGreen > exp_map->color) ||
            (bcmColorCount <= exp_map->color)) {
            /* invalid colour */
            result = BCM_E_PARAM;
            MPLS_ERR((_SBX_D(unit, "invalid value %d for color\n"),
                      exp_map->color));
        }
        switch (SOC_SBX_CONTROL(unit)->ucodetype) {

        case SOC_SBX_UCODE_TYPE_G2P3:
            if (BCM_E_NONE == result) {
                /* extract the egress map ID from the handle */
                expMap = _MPLS_EXPMAP_HANDLE_DATA(exp_map_id);
                result = soc_sbx_g2p3_remark_get(unit,
                                                 0,
                                                 exp_map->color,
                                                 exp_map->priority,
                                                 expMap,
                                                 &p3egrRemark);
                if (BCM_E_NONE == result) {
                    exp_map->pkt_cfi = p3egrRemark.cfi;
                    exp_map->pkt_pri = p3egrRemark.pri;
                    exp_map->exp = p3egrRemark.exp;
                } else { /* if (BCM_E_NONE == result) */
                    MPLS_ERR((_SBX_D(unit, "unable to read"
                                     " egrRemark[0x%08X].[%d].[%d].[0]: "
                                     "%d (%s)\n"),
                              expMap,
                              exp_map->priority,
                              exp_map->color,
                              result,
                              _SHR_ERRMSG(result)));
                } /* if (BCM_E_NONE == result) */
            } /* if (BCM_E_NONE == result) */
            break;

        default:
            SBX_UNKNOWN_UCODE_WARN(unit);
            result = BCM_E_UNAVAIL;
        } /* switch (SOC_SBX_CONTROL(unit)->ucodetype) */
    } /* map is ingress or egress */

    
    L3_UNLOCK(unit);

    MPLS_VVERB((_SBX_D(unit, "(0x%08X,0x%08X->(%d,%d,%d,%d,%d)): return %d (%s)\n"),
                exp_map_id,
                (unsigned int)exp_map,
                exp_map->priority,
                exp_map->color,
                exp_map->exp,
                exp_map->pkt_pri,
                exp_map->pkt_cfi,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

int
_bcm_fe2000_mpls_validate_vpn_id(bcm_vpn_t vpn)
{
    return BCM_E_NONE;
}

/*
 *  Function:
 *    _bcm_fe2000_mpls_policer_stat_rw
 *  Description:
 *    Translate MPLS port stat to Pol stat
 *    Get/Clear the Pol Stat counters
 *  Parameters:
 *    in  clear          - Get (0) / clear (1) Stat
 *    in  policerId      - Policer Id
 *    in  statCos        - 
 *    in  mplsStat       - MPLS Port statistic ID
 *    in  base_counter   - base counter
 *    out *val           - Pointer to store counter value
 *  Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 */

static int
_bcm_fe2000_mpls_policer_stat_rw(int unit, 
                                 int clear,
                                 uint32 policerId,
                                 bcm_cos_t statCos,
                                 bcm_mpls_port_stat_t mplsStat,
                                 uint32 base_counter,
                                 uint64 *val)
{
#define BCM_FE2K_G2P3_MAX_STAT_CONV 24
    int                      status = BCM_E_NONE;
    bcm_policer_stat_t       polStats[BCM_FE2K_G2P3_MAX_STAT_CONV];
    int                      numStats = 0, statIdx;
    bcm_policer_group_mode_t groupMode;
    uint64                   uuVal;
    int                      allCos = FALSE;

    status = _bcm_fe2000_policer_group_mode_get(unit, policerId, &groupMode);
    if (BCM_SUCCESS(status)) {

        switch (groupMode) {
        case bcmPolicerGroupModeSingle:

            switch (mplsStat) {
/*
            case bcmVlanStatIngressPackets:
                polStats[numStats++] = bcmPolicerStatPackets;
                polStats[numStats++] = bcmPolicerStatDropPackets;
                break;
            case bcmVlanStatIngressBytes:
                polStats[numStats++] = bcmPolicerStatBytes;
                polStats[numStats++] = bcmPolicerStatDropBytes;
                break;
            case bcmVlanStatForwardedPackets:
                polStats[numStats++] = bcmPolicerStatPackets;
                break;
            case bcmVlanStatForwardedBytes:
                polStats[numStats++] = bcmPolicerStatBytes;
                break;
*/
            case bcmMplsPortStatDropPackets:
                polStats[numStats++] = bcmPolicerStatDropPackets;
                break;
            case bcmMplsPortStatDropBytes:
                polStats[numStats++] = bcmPolicerStatDropBytes;
            break;
            default:
                break;
            }
            break;
        case bcmPolicerGroupModeTyped:

            switch (mplsStat) {
            case bcmMplsPortStatUnicastPackets:
                polStats[numStats++] = bcmPolicerStatUnicastPackets;
                break;
/*
            case bcmVlanStatIngressPackets:
                polStats[numStats++] = bcmPolicerStatUnicastPackets;
                polStats[numStats++] = bcmPolicerStatMulticastPackets;
                polStats[numStats++] = bcmPolicerStatUnknownUnicastPackets;
                polStats[numStats++] = bcmPolicerStatBroadcastPackets;
                polStats[numStats++] = bcmPolicerStatDropUnicastPackets;
                polStats[numStats++] = bcmPolicerStatDropMulticastPackets;
                polStats[numStats++] = bcmPolicerStatDropUnknownUnicastPackets;
                polStats[numStats++] = bcmPolicerStatDropBroadcastPackets;
                break;

            case bcmVlanStatUnicastDropPackets:
                polStats[numStats++] = bcmPolicerStatDropUnicastPackets;
                break;
            case bcmVlanStatUnicastDropBytes:
                polStats[numStats++] = bcmPolicerStatDropUnicastBytes;
                break;
*/
            case bcmMplsPortStatUnicastBytes:
                polStats[numStats++] = bcmPolicerStatUnicastBytes;
                break;
/*
            case bcmVlanStatIngressBytes:
                polStats[numStats++] = bcmPolicerStatUnicastBytes;
                polStats[numStats++] = bcmPolicerStatMulticastBytes;
                polStats[numStats++] = bcmPolicerStatUnknownUnicastBytes;
                polStats[numStats++] = bcmPolicerStatBroadcastBytes;
                polStats[numStats++] = bcmPolicerStatDropUnicastBytes;
                polStats[numStats++] = bcmPolicerStatDropMulticastBytes;
                polStats[numStats++] = bcmPolicerStatDropUnknownUnicastBytes;
                polStats[numStats++] = bcmPolicerStatDropBroadcastBytes;
                break;
*/
            case bcmMplsPortStatNonUnicastPackets:
                polStats[numStats++] = bcmPolicerStatMulticastPackets;
                break;
            case bcmMplsPortStatNonUnicastBytes:
                polStats[numStats++] = bcmPolicerStatMulticastBytes;
                break;
/*
            case bcmVlanStatNonUnicastDropPackets:
                polStats[numStats++] = bcmPolicerStatDropMulticastPackets;
                break;
            case bcmVlanStatNonUnicastDropBytes:
                polStats[numStats++] = bcmPolicerStatDropMulticastBytes;
                break;
*/
            case bcmMplsPortStatFloodDropPackets:
                polStats[numStats++] = bcmPolicerStatDropUnknownUnicastPackets;
                break;
            case bcmMplsPortStatFloodDropBytes:
                polStats[numStats++] = bcmPolicerStatDropUnknownUnicastBytes;
                break;
            case bcmMplsPortStatFloodPackets:
                polStats[numStats++] = bcmPolicerStatUnknownUnicastPackets;
                break;
            case bcmMplsPortStatFloodBytes:
                polStats[numStats++] = bcmPolicerStatUnknownUnicastBytes;
                break;
/*
            case bcmVlanStatForwardedPackets:
                polStats[numStats++] = bcmPolicerStatUnicastPackets;
                polStats[numStats++] = bcmPolicerStatMulticastPackets;
                polStats[numStats++] = bcmPolicerStatUnknownUnicastPackets;
                polStats[numStats++] = bcmPolicerStatBroadcastPackets;
                break;
            case bcmVlanStatForwardedBytes:
                polStats[numStats++] = bcmPolicerStatUnicastBytes;
                polStats[numStats++] = bcmPolicerStatMulticastBytes;
                polStats[numStats++] = bcmPolicerStatUnknownUnicastBytes;
                polStats[numStats++] = bcmPolicerStatBroadcastBytes;
                break;
*/
            case bcmMplsPortStatDropPackets:
                polStats[numStats++] = bcmPolicerStatDropUnicastPackets;
                polStats[numStats++] = bcmPolicerStatDropMulticastPackets;
                polStats[numStats++] = bcmPolicerStatDropUnknownUnicastPackets;
                polStats[numStats++] = bcmPolicerStatDropBroadcastPackets;
                break;
            case bcmMplsPortStatDropBytes:
                polStats[numStats++] = bcmPolicerStatDropUnicastBytes;
                polStats[numStats++] = bcmPolicerStatDropMulticastBytes;
                polStats[numStats++] = bcmPolicerStatDropUnknownUnicastBytes;
                polStats[numStats++] = bcmPolicerStatDropBroadcastBytes;
                break;
            default:
                break;
            }
            break;
        case bcmPolicerGroupModeTypedIntPri: /* fall thru intentional */
        case bcmPolicerGroupModeTypedAll:
            switch (mplsStat) {
/*
            case bcmVlanStatIngressPackets:
                polStats[numStats++] = bcmPolicerStatGreenPackets;
                polStats[numStats++] = bcmPolicerStatYellowPackets;
                polStats[numStats++] = bcmPolicerStatRedPackets;
                allCos = (groupMode == bcmPolicerGroupModeTypedIntPri);
                break;
            case bcmVlanStatIngressBytes:
                polStats[numStats++] = bcmPolicerStatGreenBytes;
                polStats[numStats++] = bcmPolicerStatYellowBytes;
                polStats[numStats++] = bcmPolicerStatRedBytes;
                allCos = (groupMode == bcmPolicerGroupModeTypedIntPri);
                break;
*/
            case bcmMplsPortStatDropPackets:
                polStats[numStats++] = bcmPolicerStatDropUnknownUnicastPackets;
                polStats[numStats++] = bcmPolicerStatDropUnicastPackets;
                polStats[numStats++] = bcmPolicerStatDropMulticastPackets;
                polStats[numStats++] = bcmPolicerStatDropBroadcastPackets;
                break;
            case bcmMplsPortStatDropBytes:
                polStats[numStats++] = bcmPolicerStatDropUnknownUnicastBytes;
                polStats[numStats++] = bcmPolicerStatDropUnicastBytes;
                polStats[numStats++] = bcmPolicerStatDropMulticastBytes;
                polStats[numStats++] = bcmPolicerStatDropBroadcastBytes;
                break;
/*
            case bcmVlanStatUnicastDropPackets:
                polStats[numStats++] = bcmPolicerStatDropUnicastPackets;
                break;
            case bcmVlanStatUnicastDropBytes:
                polStats[numStats++] = bcmPolicerStatDropUnicastBytes;
                break;
            case bcmVlanStatNonUnicastDropPackets:
                polStats[numStats++] = bcmPolicerStatDropMulticastPackets;
                polStats[numStats++] = bcmPolicerStatDropBroadcastPackets;
                break;
            case bcmVlanStatNonUnicastDropBytes:
                polStats[numStats++] = bcmPolicerStatDropMulticastBytes;
                polStats[numStats++] = bcmPolicerStatDropBroadcastBytes;
                break;
*/
            case bcmMplsPortStatFloodDropPackets:
                polStats[numStats++] = bcmPolicerStatDropUnknownUnicastPackets;
                break;
            case bcmMplsPortStatFloodDropBytes:
                polStats[numStats++] = bcmPolicerStatDropUnknownUnicastBytes;
                break;
            case bcmMplsPortStatGreenPackets:
                polStats[numStats++] = bcmPolicerStatGreenPackets;
                break;
            case bcmMplsPortStatGreenBytes:
                polStats[numStats++] = bcmPolicerStatGreenBytes;
                break;
            case bcmMplsPortStatYellowPackets:
                polStats[numStats++] = bcmPolicerStatYellowPackets;
                break;
            case bcmMplsPortStatYellowBytes:
                polStats[numStats++] = bcmPolicerStatYellowBytes;
                break;
            case bcmMplsPortStatRedPackets:
                polStats[numStats++] = bcmPolicerStatRedPackets;
                break;
            case bcmMplsPortStatRedBytes:
                polStats[numStats++] = bcmPolicerStatRedBytes;
                break;
            default:
                break;
            }
            break;

        default:
            MPLS_ERR((_SBX_D(unit, "Unsupported groupMode: %d %s\n"),
                      groupMode,
                      _bcm_fe2000_policer_group_mode_to_str(groupMode)));
            status = BCM_E_CONFIG;
            break;
        }

        assert (numStats < BCM_FE2K_G2P3_MAX_STAT_CONV);

        if (numStats <= 0 ) {
            MPLS_ERR((_SBX_D(unit, "MplsPortStatType %d not supported by "
                                "policer group mode %d\n"),
                      mplsStat, groupMode));
            return BCM_E_PARAM;
        }

        COMPILER_64_ZERO(uuVal);
        for (statIdx = 0; statIdx < numStats; statIdx++) {
            uint64 uuTmp = 0;
            int cos, cosStart, cosEnd;

            if (allCos) {
                /* coverity[dead_error_begin] */
                cosStart = 0;
                cosEnd = NUM_COS(unit);
            } else {
                cosStart = statCos;
                cosEnd = statCos + 1;
            }

            for (cos = cosStart; cos < cosEnd; cos++) {
                if (clear) {
                    status = _bcm_fe2000_g2p3_policer_stat_set(unit, policerId,
                                           polStats[statIdx], cos,
                                           0, base_counter, 0);
                } else {
                    status = _bcm_fe2000_g2p3_policer_stat_get(unit, policerId,
                                           polStats[statIdx], cos,
                                           0, base_counter,
                                           0, &uuTmp);
                }
                if (BCM_FAILURE(status)) {
                    MPLS_ERR((_SBX_D(unit, "Failed to get mplsPortStat=%d; "
                                     "policerStat=%d: %d %s\n"), mplsStat,
                              polStats[statIdx], status, bcm_errmsg(status)));
                    return status;
                }
                COMPILER_64_ADD_64(uuVal, uuTmp);
            }
        }

        if (!clear) {
            *val = uuVal;
        }
    }    

    return status;
}

int
_bcm_fe2000_mpls_label_stat_get(int unit,
                                bcm_mpls_label_t label,
                                bcm_gport_t port,
                                bcm_mpls_stat_t stat,
                                int clear,
                                uint64 *val)
{
    int                          status = BCM_E_NONE;
    bcm_policer_group_mode_t     grp_mode;
    int                          num_ctrs = 0, pkts = 0;
    uint32                       counter, cur_ctr, i, ohi;
    soc_sbx_g2p3_counter_value_t soc_val, soc_val_tot;
    soc_sbx_g2p3_oi2e_t          oi2e;
    soc_sbx_g2p3_lp_t            lp;
    uint32                       lpi;
    bcm_policer_t                policer_id;
    _fe2k_l3_fe_instance_t      *l3_fe;
    _fe2k_vpn_sap_t             *vpn_sap = NULL;

    MPLS_VVERB((_SBX_D(unit, "ENTER \n")));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        return BCM_E_UNIT;
    }
    status = _bcm_fe2000_mpls_find_vpn_sap_by_gport(l3_fe, port, &vpn_sap);
    if (BCM_FAILURE(status)) {
        MPLS_ERR((_SBX_D(unit, 
                        "ERROR: Failed to find vpn sap for gport: "
                        "unit %d port %08X\n"), unit, port));
        return status;
    }

    switch (stat) {
    case bcmMplsInPkts:  pkts = 1;
        /*passthru*/
        /* coverity[fallthrough: FALSE] */
    case bcmMplsInBytes:
        lpi = vpn_sap->logicalPort;
        status = soc_sbx_g2p3_lp_get(unit, lpi, &lp);
        if (BCM_SUCCESS(status)) {
        
            counter    = lp.counter;
            policer_id = lp.policer;
            COMPILER_64_ZERO(soc_val_tot.packets);
            COMPILER_64_ZERO(soc_val_tot.bytes);
            if (counter & policer_id) {
                status = _bcm_fe2000_policer_group_mode_get(unit, 
                                                            policer_id,
                                                           &grp_mode);
                
                if (BCM_SUCCESS(status)) {
                    status = _bcm_fe2000_g2p3_num_counters_get(unit,
                                                               grp_mode,
                                                              &num_ctrs);
                    if (BCM_SUCCESS(status)) {
                        for (i = 0; i < num_ctrs; i++) {
                            cur_ctr = counter + i;
                            status = soc_sbx_g2p3_ingctr_get(unit, clear,
                                                             cur_ctr, &soc_val);
                            if (BCM_SUCCESS(status)) {
                               /* add the counters */
                                if (pkts) {
                                    COMPILER_64_ADD_64(soc_val_tot.packets,
                                                      soc_val.packets); 
                                } else {
                                    COMPILER_64_ADD_64(soc_val_tot.bytes,
                                                      soc_val.bytes); 
                                }
                            } else {
                                MPLS_ERR((_SBX_D(unit, "ERROR: Failed to read "
                                   "ingress counter 0x%x \n"), cur_ctr));
                                break;
                            }
                        }
                        if (BCM_SUCCESS(status)) {
                            if (pkts) {
                                COMPILER_64_SET(*val, 
                                    COMPILER_64_HI(soc_val_tot.packets),
                                    COMPILER_64_LO(soc_val_tot.packets));
                            } else {
                                COMPILER_64_SET(*val, 
                                    COMPILER_64_HI(soc_val_tot.bytes),
                                    COMPILER_64_LO(soc_val_tot.bytes));
                            }
                        }
                    }
                }    
            } else {
                status = BCM_E_CONFIG;
                MPLS_ERR((_SBX_D(unit, "ERROR: counter or policer missing: "
                         "gport %08X lpi %d counter %d policer 0x%x\n"),
                          port, lpi, counter, policer_id));
            }
        } else {
            MPLS_ERR((_SBX_D(unit, "ERROR: Failed to read lport entry: "
                    "gport %08X lp index %d\n"), port, lpi));
        }
        break;
    case bcmMplsOutPkts:  pkts = 1;
        /*passthru*/
        /* coverity[fallthrough: FALSE] */
    case bcmMplsOutBytes:
        COMPILER_64_ZERO(soc_val.packets);
        COMPILER_64_ZERO(soc_val.bytes);
        ohi = _FE2K_L3_G2P3_ADJUST_TB_OFFSET(vpn_sap->vc_ohi.ohi);
        status = soc_sbx_g2p3_oi2e_get(unit, ohi, &oi2e);
        if (BCM_SUCCESS(status)) {
            counter = oi2e.counter;
            if (counter) {
                status = soc_sbx_g2p3_egrctr_get(unit, clear,
                                                 counter, &soc_val);
                if (BCM_SUCCESS(status)) {
                    if (pkts) {
                        COMPILER_64_SET(*val, 
                                COMPILER_64_HI(soc_val.packets),
                                COMPILER_64_LO(soc_val.packets));
                    } else {
                        COMPILER_64_SET(*val, 
                                COMPILER_64_HI(soc_val.bytes),
                                COMPILER_64_LO(soc_val.bytes));
                    }
                } else {
                    MPLS_ERR((_SBX_D(unit, "ERROR: Failed to read "
                           "egress counter 0x%x \n"), counter));
                }
            } else {
                status = BCM_E_CONFIG;
                MPLS_ERR((_SBX_D(unit, "ERROR: counter missing in ohi: "
                         "gport %08X ohi %d counter %d\n"),
                          port, ohi, counter));
            }
        } else {
            MPLS_ERR((_SBX_D(unit, "ERROR: Failed to read ohi entry: "
                    "gport %08X ohi %d\n"), port, ohi));
        }
        break;
    default:
        break;
    }

    return status;
}


int
bcm_fe2000_mpls_label_stat_clear(int unit,
                                 bcm_mpls_label_t label,
                                 bcm_gport_t port,
                                 bcm_mpls_stat_t stat)
{
    int          status = BCM_E_NONE;
    uint64       val = 0;
    int          clear = 1;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if(!BCM_GPORT_IS_MPLS_PORT(port)) {
        return BCM_E_PARAM;
    }

    status = _bcm_fe2000_mpls_label_stat_get(unit, label, port,
                                             stat, clear, &val);
    return status;
}

int
bcm_fe2000_mpls_label_stat_get(int unit,
                               bcm_mpls_label_t label,
                               bcm_gport_t port,
                               bcm_mpls_stat_t stat,
                               uint64 *val)
{
    int status = BCM_E_NONE;
    int clear = 0;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    if(!BCM_GPORT_IS_MPLS_PORT(port)) {
        return BCM_E_PARAM;
    }

    *val = 0;
    status = _bcm_fe2000_mpls_label_stat_get(unit, label, port,
                                             stat, clear, val);

     /* if the label is passed, 
      *     switch (stat)
      *     case bcmMplsInBytes:
      *     case bcmMPlsInPkts:
      *         read the label2e
      *         get the lp from label2e
      *         get the policer and base counter
      *         get the policer group mode and no. of counters
      *         read the counters one by one and sum it up
      *     case bcmMplsOutBytes:
      *     case bcmMplsOutPkts:
      *         Get the oi
      *         Read the oi and get the couner
      *         read the counter
      */

    return status;
}

int 
bcm_fe2000_mpls_label_stat_get32(
    int unit, 
    bcm_mpls_label_t label, 
    bcm_gport_t port, 
    bcm_mpls_stat_t stat, 
    uint32 *val)
{
    int status = BCM_E_NONE;
    uint64          value64;
    
    status = bcm_fe2000_mpls_label_stat_get(unit, label, port, stat, &value64);
    if (BCM_E_NONE == status) {
        if (value64 > 0x00000000FFFFFFFFull) {
            /* the value is too large */
            *val = 0xFFFFFFFF;
        } else {
            /* the value will fit */
            *val = u64_L(value64);
        }
    }
    return status;
}

/*
 *  Function:
 *    bcm_fe2000_mpls_port_stat_enable_set
 *  Description:
 *    Enable/Disable statistics on the indicated MPLS gport
 *  Parameters:
 *    in int unit - unit to access
 *    in const bcm_gport_t gport - MPLS GPort to translate
 *    in int enable - nonzero to enable stats, zero to disable stats
 *  Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 *  Notes:
 *    Takes & releases MPLS lock for the unit
 */
int
bcm_fe2000_mpls_port_stat_enable_set(int unit, bcm_gport_t port, int enable)
{
    int                          status = BCM_E_NONE;
    soc_sbx_g2p3_lp_t            lp;
    soc_sbx_g2p3_oi2e_t          oi2e;
    uint32                       lpi, ohi;
    bcm_policer_t                policer_id;
    bcm_policer_config_t         pol_cfg;
    _fe2k_l3_fe_instance_t      *l3_fe;
    _fe2k_vpn_sap_t             *vpn_sap = NULL;
    uint32                       counter, egr_counter;

    /* only 0 & 1 are valid values for enable */
    if ((enable != 0) && (enable != 1)) {
        MPLS_ERR(("%s: Invalid enable value (%d). valid values - 0,1 \n",
                     FUNCTION_NAME(), enable));
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }
    /* Get the vpn sap using the gport id*/
    status = _bcm_fe2000_mpls_find_vpn_sap_by_gport(l3_fe, port, &vpn_sap);

    if (BCM_FAILURE(status)) {
        MPLS_ERR((_SBX_D(unit, 
                        "ERROR: Failed to find vpn sap for gport: "
                        "unit %d port %08X\n"), unit, port));
        L3_UNLOCK(unit);
        return status;
    }

    /* Get the lp entry to enable / disable ingress counter */
    lpi = vpn_sap->logicalPort;
    status = soc_sbx_g2p3_lp_get(unit, lpi, &lp);
    if (BCM_FAILURE(status)) {
        MPLS_ERR((_SBX_D(unit, "ERROR: Failed to read lport entry: "
                        "gport %08X lp index %d\n"), port, lpi));
        L3_UNLOCK(unit);
        return status;
    }

    /* Get the oi2e to enable / disable egress counter */
    ohi = _FE2K_L3_G2P3_ADJUST_TB_OFFSET(vpn_sap->vc_ohi.ohi);
    status = soc_sbx_g2p3_oi2e_get(unit, ohi, &oi2e);
    if (BCM_FAILURE(status)) {
        MPLS_ERR((_SBX_D(unit, "ERROR: Failed to read oi2e entry: "
                        "gport %08X ohi %d\n"), port, ohi));
        L3_UNLOCK(unit);
        return status;
    }

    policer_id  = lp.policer;
    counter     = lp.counter;
    egr_counter = oi2e.counter;

    if (enable == 0) {
        /* disable the gport counters - ingress and egress */
        if (counter) {
            /* ingress counter available - reset lp.counter
             * and free the counters */
            lp.counter = 0;
            status = soc_sbx_g2p3_lp_set(unit, lpi, &lp);
            if (BCM_SUCCESS(status)) {
                MPLS_VVERB((_SBX_D(unit, "Clearing counter in lport entry: "
                         " gport %08X lp index %d policerId 0x%x\n"),
                          port, lpi, policer_id));
                /* free counters */
                status = _bcm_fe2000_g2p3_free_counters(unit,
                                                        policer_id, 0,
                                                        counter);
                if (BCM_FAILURE(status)) {
                    MPLS_ERR((_SBX_D(unit, 
                             "ERROR: Failed to free ingress counters: "
                             "gport %08X lp index %d base_ctr %d\n"),
                              port, lpi, counter));
                }
            } else {
                    MPLS_ERR((_SBX_D(unit, 
                             "ERROR: Failed to set lport entry: "
                             "unit %d gport %08X lp index %d\n"),
                              unit, port, lpi));
            }
        } else {
            MPLS_VVERB((_SBX_D(unit, "ERROR: No ingress counter found: "
                             "gport %08X ohi %d\n"), port, ohi));
        }

        if (BCM_SUCCESS(status) && egr_counter) {
            /* egress counter available - reset oi2e.counter
             * and free the counters */

            oi2e.counter = 0;
            status = soc_sbx_g2p3_oi2e_set(unit, ohi, &oi2e);
            if (BCM_SUCCESS(status)) {  
                MPLS_VVERB((_SBX_D(unit, "Clearing counter in ohi entry : "
                         "gport %08X ohi index %d\n"), port, ohi));
                status = _bcm_fe2000_stat_block_free(unit,
                                   SOC_SBX_G2P3_EGRCTR_ID, egr_counter);
                if (BCM_FAILURE(status)) {
                    MPLS_ERR((_SBX_D(unit, 
                             "ERROR: Failed to free egress counters: "
                             "gport %08X ohi %d ctr %d\n"),
                              port, ohi, egr_counter));
                }
            } else {
                MPLS_ERR((_SBX_D(unit, "ERROR: Failed to set ohi entry: "
                         "gport %08X lp index %d\n"), port, lpi));
            }
        } else if (!egr_counter) {
            MPLS_VVERB((_SBX_D(unit, "No egress counter found: "
                                 "gport %08X ohi %d\n"), port, ohi));
        }
    } else if (enable == 1) {
        /* enable the gport counters - ingress and egress */
        if (policer_id == 0) {
            status = BCM_E_CONFIG; 
            MPLS_ERR((_SBX_D(unit, "ERROR: No policer attached to gport: "
                            "gport %08X lp index %d\n"), port, lpi));
        }
        if (BCM_SUCCESS(status) && (counter == 0)) {
            /* check policer type and alloc correct number of counters
             * update lp.counter */

            /* check if policer exists */
            status = bcm_policer_get(unit, policer_id, &pol_cfg);
            if (BCM_SUCCESS(status)) {
                status = _bcm_fe2000_g2p3_alloc_counters(unit, 
                                                         policer_id, 0,
                                                         &counter);
                if (BCM_SUCCESS(status)) {
                    lp.counter = counter;
                    status = soc_sbx_g2p3_lp_set(unit, lpi, &lp);

                    if (BCM_SUCCESS(status)) {
                        MPLS_VVERB((_SBX_D(unit, "Updating lport entry with "
                             " counter: gport %08X lpi %d policerId 0x%x\n"),
                              port, lpi, policer_id));
                    } else {
                        MPLS_ERR((_SBX_D(unit, "ERROR: Failed to set lport "
                                "entry: gport %08X lp index %d\n"), port, lpi));
                        /* failed to write lp, so free counters */
                        _bcm_fe2000_g2p3_free_counters(unit,
                                                       policer_id, 0,
                                                       counter);
                    }
                } else {
                    MPLS_ERR((_SBX_D(unit, "ERROR: Failed to alloc "
                                    " ingress counters")));
                }
            } else {
                MPLS_ERR((_SBX_D(unit, "ERROR: Failed to get policer: "
                        "gport %08X lp index %d policerId 0x%x\n"),
                         port, lpi, policer_id));
            }
        } else if (counter != 0) {
            MPLS_VVERB((_SBX_D(unit, "Ingress counter is already attached: "
                                "gport %08X lp index %d\n"), port, lpi));
        }

        /* enable stat for egress */
        if (BCM_SUCCESS(status) && (egr_counter == 0)) {
            status = _bcm_fe2000_stat_block_alloc(unit,
                                   SOC_SBX_G2P3_EGRCTR_ID, &egr_counter, 1);
            if (BCM_SUCCESS(status)) {
                oi2e.counter =  egr_counter;
                status = soc_sbx_g2p3_oi2e_set(unit, ohi, &oi2e);
                if (BCM_SUCCESS(status)) {
                    MPLS_VVERB((_SBX_D(unit, "Updating counter in ohi entry : "
                         "gport %08X ohi index %d\n"), port, ohi));
                } else {
                    MPLS_ERR((_SBX_D(unit, "ERROR: Failed to set oi2e: "
                                     "gport %08X ohi %d\n"), port, ohi));

                    _bcm_fe2000_stat_block_free(unit,
                                                SOC_SBX_G2P3_EGRCTR_ID,
                                                egr_counter);
                }
            } else {
                MPLS_ERR((_SBX_D(unit, "ERROR: Failed to alloc"
                                    " egress counters")));
            }
        } else if (egr_counter != 0) {
            MPLS_VVERB((_SBX_D(unit, "Egress counter is already attached: "
                                "gport %08X ohix %d\n"), port, ohi));
        }
    }

    L3_UNLOCK(unit);

    return status;
}

/*
 *  Function:
 *    bcm_fe2000_mpls_port_stat_set
 *  Description:
 *    Set the specified MPLS gport statistic to the indicated value
 *  Parameters:
 *    in int unit - unit to access
 *    in const bcm_gport_t gport - MPLS GPort to translate
 *    in bcm_cos_t cos - COS level or BCM_COS_INVALID for all (in some cases)
 *    in bcm_mpls_port_stat_t stat - which statistic to set
 *    in uint64 val - the new value
 *  Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 *  Notes:
 *    Takes & releases MPLS lock for the unit.
 *    SBX only allows clear, so val must be zero.
 */
int
bcm_fe2000_mpls_port_stat_set(int unit,
                              bcm_gport_t port,
                              bcm_cos_t cos,
                              bcm_mpls_port_stat_t stat,
                              uint64 val)
{
    int                          status = BCM_E_NONE;
    soc_sbx_g2p3_lp_t            lp;
    uint32                       lpi;
    bcm_policer_t                policer_id;
    _fe2k_l3_fe_instance_t      *l3_fe;
    _fe2k_vpn_sap_t             *vpn_sap = NULL;
    uint32                       counter;
    int                          clear = 1;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    BCM_STAT_VVERB(("%s(%d, %08X, %d, %d, 0x%08X %08X) - Enter\n",
                    FUNCTION_NAME(),
                    unit,
                    port,
                    cos,
                    stat,
                    u64_H(val),
                    u64_L(val)));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        return BCM_E_UNIT;
    }

    status = _bcm_fe2000_mpls_find_vpn_sap_by_gport(l3_fe, port, &vpn_sap);

    if (BCM_SUCCESS(status)) {
        lpi = vpn_sap->logicalPort;
        status = soc_sbx_g2p3_lp_get(unit, lpi, &lp);

        if (BCM_SUCCESS(status)) {
            counter    = lp.counter;
            policer_id = lp.policer;
            if (counter & policer_id) {
                  status = _bcm_fe2000_mpls_policer_stat_rw(unit, clear,
                                                            policer_id,
                                                            cos, stat,
                                                            counter, &val);
            } else {
                MPLS_ERR((_SBX_D(unit,
                            "ERROR: No policer or coutner attached to gport: "
                            "unit %d gport %08X counter %d policerId 0x%x\n"),
                             unit, port, counter, policer_id));
                status = BCM_E_CONFIG;
            }
        } else {
            MPLS_ERR((_SBX_D(unit, 
                            "ERROR: Failed to read lport entry: "
                            "unit %d port %08X lp index %d\n"),
                       unit, port, lpi));
        }
        
    } else {
            MPLS_ERR((_SBX_D(unit, 
                            "ERROR: Failed to find vpn sap for gport: "
                            "unit %d port %08X\n"), unit, port));
    }

    BCM_STAT_VVERB(("%s(%d, %08X, %d, %d, 0x%08X %08X) - Exit %d (%s)\n",
                    FUNCTION_NAME(),
                    unit,
                    port,
                    cos,
                    stat,
                    u64_H(val),
                    u64_L(val),
                    status,
                    bcm_errmsg(status)));

    return status;
}

/*
 *  Function:
 *    bcm_fe2000_mpls_port_stat_set32
 *  Description:
 *    Set the specified MPLS gport statistic to the indicated value (32b)
 *  Parameters:
 *    in int unit - unit to access
 *    in const bcm_gport_t gport - MPLS GPort to translate
 *    in bcm_cos_t cos - COS level or BCM_COS_INVALID for all (in some cases)
 *    in bcm_mpls_port_stat_t stat - which statistic to set
 *    in uint32 val - the new value
 *  Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 *  Notes:
 *    Takes & releases MPLS lock for the unit.
 *    SBX only allows clear, so val must be zero.
 */
int
bcm_fe2000_mpls_port_stat_set32(int unit,
                                bcm_gport_t port,
                                bcm_cos_t cos,
                                bcm_mpls_port_stat_t stat,
                                uint32 val)
{
    uint64          value64;
    int             result;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    BCM_STAT_VVERB(("%s(%d, %08X, %d, %d, %08X) - Enter\n",
                    FUNCTION_NAME(),
                    unit,
                    port,
                    cos,
                    stat,
                    val));

    STAT_CHECK_STAT_VALUE(val);     /* Only allowed to set to zero */

    COMPILER_64_SET(value64, 0, val);
    result = bcm_fe2000_mpls_port_stat_set(unit, port, cos, stat, value64);

    BCM_STAT_VVERB(("%s(%d, %08X, %d, %d, %08X) - Exit %d (%s)\n",
                    FUNCTION_NAME(),
                    unit,
                    port,
                    cos,
                    stat,
                    val,
                    result,
                    bcm_errmsg(result)));
    return result;
}

/*
 *  Function:
 *    bcm_fe2000_mpls_port_stat_get
 *  Description:
 *    Get the specified MPLS gport statistic value
 *  Parameters:
 *    in int unit - unit to access
 *    in const bcm_gport_t gport - MPLS GPort to translate
 *    in bcm_cos_t cos - COS level or BCM_COS_INVALID for all (in some cases)
 *    in bcm_mpls_port_stat_t stat - which statistic to set
 *    in uint64 *val - where to put the value
 *  Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 *  Notes:
 *    Takes & releases MPLS lock for the unit.
 *    SBX only allows clear, so val must be zero.
 */
int
bcm_fe2000_mpls_port_stat_get(int unit,
                              bcm_gport_t port,
                              bcm_cos_t cos,
                              bcm_mpls_port_stat_t stat,
                              uint64 *val)
{
    int                          status = BCM_E_NONE;
    soc_sbx_g2p3_lp_t            lp;
    uint32                       lpi;
    bcm_policer_t                policer_id;
    _fe2k_l3_fe_instance_t      *l3_fe;
    _fe2k_vpn_sap_t             *vpn_sap = NULL;
    uint32                       counter;
    int                          clear = 0;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    MPLS_VVERB((_SBX_D(unit, "%s(%d, %08X, %d, %d, *) - Enter\n"),
                    FUNCTION_NAME(),
                    unit,
                    port,
                    cos,
                    stat));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        return BCM_E_UNIT;
    }
    status = _bcm_fe2000_mpls_find_vpn_sap_by_gport(l3_fe, port, &vpn_sap);

    if (BCM_SUCCESS(status)) {
        lpi = vpn_sap->logicalPort;
        status = soc_sbx_g2p3_lp_get(unit, lpi, &lp);

        if (BCM_SUCCESS(status)) {
            counter    = lp.counter;
            policer_id = lp.policer;
            if (counter & policer_id) {
                status = _bcm_fe2000_mpls_policer_stat_rw(unit, clear,
                                                          policer_id,
                                                          cos, stat,
                                                          counter, val);
            } else {
                MPLS_ERR((_SBX_D(unit,
                            "ERROR: No policer or coutner attached to gport: "
                            "unit %d gport %08X counter %d policerId 0x%x\n"),
                             unit, port, counter, policer_id));
                status = BCM_E_CONFIG;
            }
        } else {
            MPLS_ERR((_SBX_D(unit, 
                            "ERROR: Failed to read lport entry: "
                            "unit %d port %08X lp index %d\n"),
                       unit, port, lpi));
        }
        
    } else {
            MPLS_ERR((_SBX_D(unit, 
                            "ERROR: Failed to find vpn sap for gport: "
                            "unit %d port %08X\n"), unit, port));
    }


    if (val) {
        BCM_STAT_VVERB(("%s(%d, %08X, %d, %d, &(0x%08X %08X)) - Exit %d (%s)\n",
                        FUNCTION_NAME(),
                        unit,
                        port,
                        cos,
                        stat,
                        u64_H(*val),
                        u64_L(*val),
                        status,
                        bcm_errmsg(status)));
    }

    return status;
}

/*
 *  Function:
 *    bcm_fe2000_mpls_port_stat_get32
 *  Description:
 *    Get the specified MPLS gport statistic value (32b)
 *  Parameters:
 *    in int unit - unit to access
 *    in const bcm_gport_t gport - MPLS GPort to translate
 *    in bcm_cos_t cos - COS level or BCM_COS_INVALID for all (in some cases)
 *    in bcm_mpls_port_stat_t stat - which statistic to set
 *    in uint32 *val - where to put the value
 *  Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 *  Notes:
 *    Takes & releases MPLS lock for the unit.
 *    SBX only allows clear, so val must be zero.
 */
int
bcm_fe2000_mpls_port_stat_get32(int unit,
                                bcm_gport_t port,
                                bcm_cos_t cos,
                                bcm_mpls_port_stat_t stat,
                                uint32 *val)
{
    int             result;
    uint64          value64;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    BCM_STAT_VVERB(("%s(%d, %08X, %d, %d, *) - Enter\n",
                    FUNCTION_NAME(),
                    unit,
                    port,
                    cos,
                    stat));

    result = bcm_fe2000_mpls_port_stat_get(unit, port, cos, stat, &value64);
    if (BCM_E_NONE == result) {
        if (value64 > 0x00000000FFFFFFFFull) {
            /* the value is too large */
            *val = 0xFFFFFFFF;
        } else {
            /* the value will fit */
            *val = u64_L(value64);
        }
    }

    if (val) {
        BCM_STAT_VVERB(("%s(%d, %08X, %d, %d, &(%08X)) - Exit %d (%s)\n",
                        FUNCTION_NAME(),
                        unit,
                        port,
                        cos,
                        stat,
                        *val,
                        result,
                        bcm_errmsg(result)));
    }

    return result;
}


int
_bcm_fe2000_mpls_port_info_get(int unit, bcm_vlan_t vsi, uint8_t vpwsuni, int *keepUntagged)
{
    _fe2k_l3_fe_instance_t *l3_fe = NULL;
    bcm_gport_t            mpls_port_id = 0;
    bcm_port_t             tgt_port, phy_port;
    bcm_vlan_t             match_vlan, mpls_vsi;
    bcm_mpls_port_t        mpls_port;
    uint32_t               logical_port;
    int                    rv;
    uint32                 fti;
    bcm_module_t           tgt_module, lcl_module;

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        return BCM_E_UNIT;
    }

    if (SOC_SBX_CFG(unit)->mplstp_ena && vpwsuni) {
        fti = vsi + l3_fe->vpws_uni_ft_offset;
    } else {
        /*  Use the VSI as a hint to the gport */
        fti = vsi + l3_fe->vlan_ft_base;
    }

    BCM_GPORT_MPLS_PORT_ID_SET(mpls_port_id, fti);

    bcm_mpls_port_t_init(&mpls_port);
    rv = _bcm_fe2000_mpls_port_vlan_vector_internal(unit, mpls_port_id,
                                                    &phy_port, &match_vlan,
                                                    &mpls_vsi, &logical_port,
                                                    &mpls_port, NULL);
    MPLS_VERB((_SBX_D(l3_fe->fe_unit, " rv=%d vsi=0x%x (fti 0x%x)--> \nmplsPort=0x%x"
                      " phy_port=%d match_vlan=0x%x mpls_vsi=0x%x lp=0x%x\n"),
               rv, vsi, fti, mpls_port_id, phy_port, match_vlan, 
               mpls_vsi, logical_port));

    if (BCM_FAILURE(rv)) {
        /* not a hard error, just guessed wrong; it's not an mpls port*/
        return rv;
    }

    rv = _bcm_fe2000_mpls_gport_get_mod_port(unit, mpls_port.port,
                                             &tgt_module, &tgt_port);
    if (BCM_FAILURE(rv)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) getting mod/port for mplsPort=0x%x\n"),
                  bcm_errmsg(rv), mpls_port_id));
        return rv;
    }

    rv = bcm_stk_my_modid_get(unit, &lcl_module);
    if (BCM_FAILURE(rv)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) failed to get mod id\n"),
                  bcm_errmsg(rv)));
        return rv;
    }

    if (tgt_module != lcl_module) {
        /* Again, not a hard error, just nothing to do and need to signal to
         * the caller out-params are not set
         */
        return BCM_E_NOT_FOUND;
    }

    /* keepUntagged specifies the behavior in ucode where a tag is added to
     * untagged traffic when the port is set to keep tags.  Setting this flag
     * results in untagged traffic remaining untagged as it traverses the fabric,
     * clearing it would result in the port's native vid to be inserted.
     * For mpls ports, we ALWAYS want to keep the packet unchanged, so we set it
     * whenever inner_vlan_preserve is set.
     */
    *keepUntagged = !!(mpls_port.flags & BCM_MPLS_PORT_INNER_VLAN_PRESERVE);

    return BCM_E_NONE;


}


#ifdef BCM_WARM_BOOT_SUPPORT
/* This is not in the recovery interface because VPWS mpls gports and only
 * VPWS mpls gports can use these bits
 */
int 
_bcm_fe2000_mpls_vpws_wb_set(_fe2k_l3_fe_instance_t *l3_fe,
                             _fe2k_vpn_sap_t*        vpn_sap)
{
    int                 rv;
    uint32              vi, vpn_id;
    soc_sbx_g2p3_v2e_t  ve;

    vpn_id = vpn_sap->vc_vpnc->vpn_id - l3_fe->line_vsi_base;

    vi = BCM_GPORT_MPLS_PORT_ID_GET(vpn_sap->vc_mpls_port_id);
    vi -=l3_fe->vlan_ft_base;

    rv = soc_sbx_g2p3_v2e_get(l3_fe->fe_unit, vi, &ve);
    if (BCM_FAILURE(rv)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Failed to get v2e[0x%x]: %s\n"),
                  vi, bcm_errmsg(rv)));
        return rv;
    }

    /* Use vrf and force flood bits in v2e to store the software only vpn id.
     * v2e.vrf can be used because VPWS explicitly disables v4route, and 
     *         therefore vrf is a don't care.
     * v2e.forceflood can be used because VPWS, by definition, implements
     *         the force behavior and is therefore a don't care.
     */
    ve.vrf        = vpn_id & 0xFFF;
    ve.forceflood = (vpn_id >> 12) & 1;

    rv = soc_sbx_g2p3_v2e_set(l3_fe->fe_unit, vi, &ve);
    if (BCM_FAILURE(rv)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Failed to set v2e[0x%x]: %s\n"),
                  vi, bcm_errmsg(rv)));
        return rv;
    }

    return rv;
}

int 
_bcm_fe2000_mpls_vpws_wb_get(_fe2k_l3_fe_instance_t *l3_fe, 
                             uint32           vsi,
                             uint32           *vpn_id)
{
    int                 rv;
    soc_sbx_g2p3_v2e_t  ve;
    
    rv = soc_sbx_g2p3_v2e_get(l3_fe->fe_unit, vsi, &ve);
    if (BCM_FAILURE(rv)) {
        MPLS_ERR((_SBX_D(l3_fe->fe_unit, "Failed to get v2e[0x%x]: %s\n"),
                  vsi, bcm_errmsg(rv)));
        return rv;
    }

    *vpn_id  = (ve.forceflood << 12) | ve.vrf;
    *vpn_id += l3_fe->line_vsi_base;
        
    return rv;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

/*
 *  Function:
 *    bcm_fe2000_mpls_gport_get
 *  Description:
 *    Get the specified MPLS gport information
 *  Parameters:
 *    in int unit - unit to access
 *    in const bcm_gport_t gport - MPLS GPort to translate
 *    _fe2k_vpn_sap_t * 
 *  Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 */
int
bcm_fe2000_mpls_gport_get(int unit,
                          bcm_gport_t gport,
                          _fe2k_vpn_sap_t **vpn_sap)
{
    int status = BCM_E_NONE;



    if(L3_UNIT_INVALID(unit) || !vpn_sap) {
        MPLS_ERR(("Bad input parameter\n"));
    } else{
        BCM_IF_ERROR_RETURN(L3_LOCK(unit));
        status = shr_htb_find(mpls_gport_db[unit], 
                          (shr_htb_key_t) &gport,
                          (shr_htb_data_t*)vpn_sap, 0);
        if(BCM_FAILURE(status)) {
            MPLS_ERR((_SBX_D(unit, "Failed:(%d)(%s) to obtain Gport information Gport[0x%x] \n"),
                      status,bcm_errmsg(status),gport));            
        }
        L3_UNLOCK(unit);                             
    }
    return status;
}

/*
 * Function:
 *   _bcm_fe2000_mpls_trunk_cb
 * Purpose:
 *     Call back function for Trunk Membership change
 * Returns:
 *   BCM_E_XX
 */ 
STATIC int
_bcm_fe2000_mpls_trunk_cb(int unit, 
                         bcm_trunk_t tid, 
                         bcm_trunk_add_info_t *tdata,
                         void *user_data)
{
    int                           status = BCM_E_NONE;
    soc_sbx_g2p3_pv2e_t           pv2e;
    soc_sbx_g2p3_pvv2e_t          pvv2e;
    bcm_port_t                    tp[BCM_TRUNK_MAX_PORTCNT];
    bcm_port_t                    tp_rmvd[BCM_TRUNK_MAX_PORTCNT];
    bcm_port_t                    tp_added[BCM_TRUNK_MAX_PORTCNT];
    int                           index=0, mymodid, pindex=0, idx=0, port=0;
    bcm_fe2k_mpls_trunk_association_t *trunkAssoc;
    bcm_trunk_add_info_t         *old_info;
    dq_p_t                        port_elem;
    _fe2k_vpn_sap_t              *mpls_vpn_sap = NULL;
    _fe2k_vpn_control_t          *mpls_vc_vpnc = NULL;
    bcm_mpls_port_t              *mpls_port = NULL;
    uint32                        vsi = 0;
    bcm_vlan_t                    vid, ivid, ovid;
    int                           portMode, rindex=0, aindex=0;

    _fe2k_l3_fe_instance_t       *l3_fe = NULL;

    MPLS_VVERB((_SBX_D(unit, "Enter\n")));

    if(!SOC_IS_SBX_G2P3((unit))) {
        MPLS_WARN(("WARNING %s is supported only for G2P3(%s,%d)\n",\
                                FUNCTION_NAME(),__FILE__,__LINE__));
        return BCM_E_UNAVAIL;
    }
    
    if(L3_UNIT_INVALID(unit)) {
        MPLS_ERR(("ERROR %sUnknown Unit %d\n", FUNCTION_NAME(), unit));
        return BCM_E_PARAM;
    }
       if(tid >= SBX_MAX_TRUNKS) {
        MPLS_ERR(("ERROR %s Bad Trunk ID  %d !!!!(%s,%d)\n",\
                 FUNCTION_NAME(),tid,__FILE__,__LINE__));
        return BCM_E_PARAM;
    }

    if(!tdata) {
        MPLS_ERR(("ERROR %s Bad Input Parameter !!!!(%s,%d)\n",\
                 FUNCTION_NAME(),__FILE__,__LINE__));
        return BCM_E_PARAM;
    }
 
    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);

    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    
    status = bcm_stk_my_modid_get(unit, &mymodid);
    if(BCM_FAILURE(status)) {
        MPLS_ERR(("ERROR %s Could not obtain my module ID !!!!(%s,%d)\n",\
                 FUNCTION_NAME(),__FILE__,__LINE__));
        return BCM_E_INTERNAL;
    }

    trunkAssoc = &mpls_trunk_assoc_info[unit][tid];
    old_info = &trunkAssoc->add_info;
    if(tdata->num_ports == 0 && old_info->num_ports == 0) {
        /* nothing to do */
        return BCM_E_NONE;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
 
    for(index=0; index < BCM_TRUNK_MAX_PORTCNT; index++) {
        tp[index] = -1;
        tp_added[index] = -1;
        tp_rmvd[index] = -1;
    }

    for(index=0; index < tdata->num_ports; index++) {
        if(tdata->tm[index] == mymodid) {
            port = tdata->tp[index];
            idx = 0;
            /* verify if this port was already taken care due to 
             * duplicate trunk distribution */
            while(tp[idx] >= 0) {
                if(port == tp[idx]) {
                    break;
                }
                idx++;
            }

            if(port == tp[idx]) {
                continue;
            }

            tp[pindex++] = port;
        }
    }

    /* remove duplicate from old ports list and add to rmvd list*/
    for(index=0; index < old_info->num_ports; index++) {
        if(old_info->tm[index] == mymodid) {
            port = old_info->tp[index];
            idx = 0;
            /* verify if this port was already taken care due to 
             * duplicate trunk distribution */
            while(tp_rmvd[idx] >= 0) {
                if(port == tp_rmvd[idx]) {
                    break;
                }
                idx++;
            }

            if(port == tp_rmvd[idx]) {
                continue;
            }

            tp_rmvd[rindex++] = port;
        }
    }

    /* find removed and added ports */
    for (index = 0; index < pindex; index++) {
        port = tp[index];
        for (idx = 0; idx < rindex; idx++) {
            if (port == tp_rmvd[idx]) {
                /* port exists in both list, remove from rmvd */
                tp_rmvd[idx] = -1;
                break;
            }
        }
        if (idx != rindex) {
        /* port newly added to the trunk */
            tp_added[aindex++] = port;
        }
    }

    /* copy the trunk add info data */
    trunkAssoc->add_info = *tdata;

    /* Traverse each element of the trunk and update the MPLS port info */
    /* Check if queue is non-empty */
    if(!DQ_EMPTY(&trunkAssoc->plist))
    {
        DQ_TRAVERSE(&trunkAssoc->plist,port_elem) {
            _FE2K_VPN_SAP_FROM_TRUNK_LINK(port_elem, mpls_vpn_sap);
            if(mpls_vpn_sap == NULL) {
                 L3_UNLOCK(unit);
                 return BCM_E_INTERNAL;
             }

            /* now the SAP could be of VPLS or VPWS service*/
            /* Handle each case seperately */
            mpls_vc_vpnc = mpls_vpn_sap->vc_vpnc;
            mpls_port = &mpls_vpn_sap->vc_mpls_port;
            /* Extract the VSI as the case may be */
            if(mpls_vc_vpnc->vpn_flags == BCM_MPLS_VPN_VPWS) {
                vsi = BCM_GPORT_MPLS_PORT_ID_GET(mpls_port->mpls_port_id)
                            - l3_fe->vlan_ft_base;
            } else if(mpls_vc_vpnc->vpn_flags == BCM_MPLS_VPN_VPLS) {
                vsi = mpls_vc_vpnc->vpn_id;
            }

            /* Take action based on the match criteria */
            switch(mpls_port->criteria) {
                case BCM_MPLS_PORT_MATCH_PORT:
                    for(index = 0; index < (aindex + rindex); index++) {
                        if (index < aindex) {
                            port = tp_added[index]; 
                        } else {
                            port = tp_rmvd[index - aindex];
                            if (port == -1 ) {
                                continue;
                            }
                        }

                        /* Check if the port mode for MATCH_PORT is set to transparent*/
                        status = bcm_port_dtag_mode_get(l3_fe->fe_unit, 
                                                        port, &portMode);
                        if(portMode != BCM_PORT_DTAG_MODE_TRANSPARENT) {
                             MPLS_ERR(("ERROR %s Could not obtain my module ID !!!!(%s,%d)\n",
                                     FUNCTION_NAME(),__FILE__,__LINE__));
                             return BCM_E_INTERNAL;
                        }

                       /* get the native VID so we can set the 
                        * proper VID's pv2e entry */

                       status = _bcm_fe2000_vlan_port_native_vlan_get( \
                                                     l3_fe->fe_unit,
                                                     port, &vid);

                       if (BCM_E_NONE != status) {

                           MPLS_ERR((_SBX_D(l3_fe->fe_unit,
                                       "error(%s) reading nativeVid for port\
                                       %d\n"),bcm_errmsg(status), port));

                           return status;

                       }
                       soc_sbx_g2p3_pv2e_t_init(&pv2e);
                       status = SOC_SBX_G2P3_PV2E_GET(l3_fe->fe_unit,
                                                      port, vid, &pv2e);
                       if (BCM_FAILURE(status)) {
                           MPLS_ERR((_SBX_D(l3_fe->fe_unit,
                                     "error(%s) reading PVid2Etc for port\
                                      %d vid %d\n"),bcm_errmsg(status),
                                      port, vid));
                           return status;
                       }
                       if (index < aindex) {
                           pv2e.vlan = vsi;
                           pv2e.lpi = mpls_vpn_sap->logicalPort;
                       } else {
                           /* delete pv2e */
                           pv2e.vlan = vid;
                           pv2e.lpi = 0;
                       }
                       status = _soc_sbx_g2p3_pv2e_set(l3_fe->fe_unit, port,
                                                       vid, &pv2e);

                       if (BCM_FAILURE(status)) {
                            MPLS_ERR((_SBX_D(l3_fe->fe_unit,
                                      "error(%s) writing pv2e(0x%x,0x%x)\n"),\
                                      bcm_errmsg(status), port, vid));
                             return status;
                       }

                    }
                break;

                case BCM_MPLS_PORT_MATCH_PORT_VLAN:
                    vid = mpls_port->match_vlan;
                    for(index = 0; index < (aindex + rindex); index++) {
                        if (index < aindex) {
                            port = tp_added[index]; 
                        } else {
                            port = tp_rmvd[index - aindex];
                            if (port == -1 ) {
                                continue;
                            }
                        }

                        soc_sbx_g2p3_pv2e_t_init(&pv2e);
                        status = SOC_SBX_G2P3_PV2E_GET(l3_fe->fe_unit, 
                                                       port, vid, &pv2e);
                        if (BCM_FAILURE(status)) {
                            MPLS_ERR((_SBX_D(l3_fe->fe_unit,
                                   "error(%s) reading PVid2Etc for port \
                                      %d vid %d\n"),bcm_errmsg(status), 
                                      port, vid));
                            return status;
                        }
                        if (index < aindex) {
                            pv2e.vlan = vsi;
                            pv2e.lpi = mpls_vpn_sap->logicalPort;
                        } else {
                           /* delete pv2e */
                           pv2e.vlan = vid;
                           pv2e.lpi = 0;
                        }
                        status = _soc_sbx_g2p3_pv2e_set(l3_fe->fe_unit,
                                                        port, vid, &pv2e);

                       if (BCM_FAILURE(status)) {
                            MPLS_ERR((_SBX_D(l3_fe->fe_unit,
                                     "error(%s) writing pv2e(0x%x,0x%x)\n"),\
                                      bcm_errmsg(status), port, vid));
                             return status;
                       }

                    }

                break;

                case BCM_MPLS_PORT_MATCH_PORT_VLAN_STACKED:
                    ivid = mpls_port->match_inner_vlan;
                    ovid = mpls_port->match_vlan;
                    for(index = 0; index < (aindex + rindex); index++) {
                        if (index < aindex) {
                            port = tp_added[index]; 
                        } else {
                            port = tp_rmvd[index - aindex];
                            if (port == -1 ) {
                                continue;
                            }
                        }

                        soc_sbx_g2p3_pvv2e_t_init(&pvv2e);
                        if (index < aindex) {
                            pvv2e.lpi = mpls_vpn_sap->logicalPort;
                            pvv2e.vlan = vsi;
                            status = soc_sbx_g2p3_pvv2e_add(l3_fe->fe_unit,
                                                            ivid, ovid, port,
                                                            &pvv2e);
                        } else {
                            /* delete pv2e */
                            status = soc_sbx_g2p3_pvv2e_remove(l3_fe->fe_unit,
                                                        ivid, ovid, port);
                        }
                        if (BCM_FAILURE(status)) {
                            MPLS_ERR((_SBX_D(l3_fe->fe_unit,
                                   "error(%s) writing pvv2e(0x%x,0x%x,0x%x)\n"),\
                                    bcm_errmsg(status), port, ivid, ovid));
                            return status;
                        }
                        status = soc_sbx_g2p3_pvv2e_commit(l3_fe->fe_unit, 
                                                        SB_COMMIT_COMPLETE);
                        if (BCM_FAILURE(status)) {
                            MPLS_ERR((_SBX_D(l3_fe->fe_unit,
                                 "error(%s) calling soc_sbx_g2p3_pvv2e_commit "
                                 "(port=%d, innerVid=%d outerVid=%d)\n"),
                                  bcm_errmsg(status), port, ivid, ovid));
                 
                            return status;
                        }

                    }
                break;

                default:
                break;
            }

        }DQ_TRAVERSE_END(&trunkAssoc->plist,port_elem);
    }
    

    
    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *     _bcm_fe2000_mpls_policer_set
 * Purpose:
 *     Set Policer ID for a Mpls Gport
 * Returns:
 *     BCM_E_XX
 */
int
_bcm_fe2000_mpls_policer_set(int unit,
                             bcm_gport_t gport,
                             bcm_policer_t pol_id)
{
    int status = BCM_E_NONE;
    _fe2k_l3_fe_instance_t *l3_fe;
    _fe2k_vpn_sap_t *vpn_sap = NULL;
    soc_sbx_g2p3_lp_t lp;
    uint32 lpi;

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }
    status = _bcm_fe2000_mpls_find_vpn_sap_by_gport(l3_fe, gport, &vpn_sap);

    if (BCM_SUCCESS(status)) {

        lpi = vpn_sap->logicalPort;
        status = soc_sbx_g2p3_lp_get(unit, lpi, &lp);

        if (BCM_SUCCESS(status)) {

            status = _bcm_fe2000_g2p3_policer_lp_program(unit, pol_id, &lp);

            if (BCM_SUCCESS(status)) {
                status = soc_sbx_g2p3_lp_set(unit, lpi, &lp);

                if (BCM_SUCCESS(status)) {
                    MPLS_VVERB((_SBX_D(unit, 
                                    "Updating lport entry with policer: unit %d"
                                    " gport %08X lp index %d policerId 0x%x\n"),
                               unit, gport, lpi, pol_id));
                } else {
                    MPLS_ERR((_SBX_D(unit, 
                                    "ERROR: Failed to set lport entry: "
                                    "unit %d gport %08X lp index %d\n"),
                               unit, gport, lpi));
                }
            } else {
                MPLS_ERR((_SBX_D(unit, 
                                "ERROR: Failed setting policer to lport: "
                                "unit %d gport %08X lp index %d\n"),
                           unit, gport, lpi));
            }
        } else {
            MPLS_ERR((_SBX_D(unit, 
                            "ERROR: Failed to read lport entry: "
                            "unit %d gport %08X lp index %d\n"),
                       unit, gport, lpi));
        }
    } else {
            MPLS_ERR((_SBX_D(unit, 
                            "ERROR: Failed to find vpn sap for gport: "
                            "unit %d gport %08X\n"), unit, gport));
    }

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *     _bcm_fe2000_mpls_policer_get
 * Purpose:
 *     Get Policer ID for a Mpls Gport
 * Returns:
 *     BCM_E_XX
 */
int
_bcm_fe2000_mpls_policer_get(int unit,
                             bcm_gport_t gport,
                             bcm_policer_t *pol_id)
{
    int status = BCM_E_NONE;

   _fe2k_l3_fe_instance_t *l3_fe;
    _fe2k_vpn_sap_t *vpn_sap = NULL;
    soc_sbx_g2p3_lp_t lp;
    uint32 lpi;

    if (!pol_id) {
        MPLS_ERR((_SBX_D(unit, 
                 "Invalid parameter: NULL pointer for pol_id\n")));
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }
    status = _bcm_fe2000_mpls_find_vpn_sap_by_gport(l3_fe, gport, &vpn_sap);

    if (BCM_SUCCESS(status)) {
        lpi = vpn_sap->logicalPort;
        status = soc_sbx_g2p3_lp_get(unit, lpi, &lp);

        if (BCM_SUCCESS(status)) {
            *pol_id = lp.policer;
        } else {
            MPLS_ERR((_SBX_D(unit, 
                            "ERROR: Failed to read lport entry: "
                            "unit %d gport %08X lp index %d\n"),
                       unit, gport, lpi));
        }
    } else {
            MPLS_ERR((_SBX_D(unit, 
                            "ERROR: Failed to find vpn sap for gport: "
                            "unit %d gport %08X\n"), unit, gport));
    }

    L3_UNLOCK(unit);
    return status;
}

#else   /* INCLUDE_L3 && BCM_MPLS_SUPPORT */
int bcm_fe2000_mpls_not_empty;
#endif  /* INCLUDE_L3 && BCM_MPLS_SUPPORT */
