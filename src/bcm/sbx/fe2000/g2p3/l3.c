/*
 * $Id: l3.c 1.83 Broadcom SDK $
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
 * File:        l3.c
 * Purpose:     BCM l3 API
 */
#include <soc/defs.h>

#ifdef INCLUDE_L3

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/fe2000.h>



#include <bcm_int/sbx/error.h>

#include <shared/gport.h>
#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/debug.h>
#include <bcm/l2.h>
#include <bcm/l3.h>
#include <bcm/ipmc.h>
#include <bcm/tunnel.h>
#include <bcm/stack.h>
#include <bcm/cosq.h>
#include <bcm/mpls.h>
#include <bcm/trunk.h>
#include <bcm/pkt.h>
#include <bcm_int/sbx/stack.h>

#include <bcm_int/sbx/fe2000/allocator.h>
#include <bcm_int/sbx/fe2000/l3.h>
#include <bcm_int/sbx/fe2000/port.h>
#include <bcm_int/sbx/fe2000/vlan.h>

#include <soc/sbx/g2p3/g2p3.h>

/*
 * Debug Output Macros
 */
#define L3_DEBUG(flags, stuff) BCM_DEBUG(flags | BCM_DBG_L3, stuff)
#define L3_NOTICE(stuff)        L3_DEBUG(BCM_DBG_NOTICE, stuff)
#define L3_WARN(stuff)          L3_DEBUG(BCM_DBG_WARN, stuff)
#define L3_ERR(stuff)           L3_DEBUG(BCM_DBG_ERR, stuff)
#define L3_VERB(stuff)          L3_DEBUG(BCM_DBG_VERBOSE, stuff)
#define L3_VVERB(stuff)        L3_DEBUG(BCM_DBG_VVERBOSE, stuff)


#define _FE2K_L3_MAP_G2P3_ETE_DMAC(ete, mac) \
    do {                                     \
        (ete)->dmac5 = mac[5]; \
        (ete)->dmac4 = mac[4]; \
        (ete)->dmac3 = mac[3]; \
        (ete)->dmac2 = mac[2]; \
        (ete)->dmac1 = mac[1]; \
        (ete)->dmac0 = mac[0]; \
    } while (0)

#define _FE2K_L3_G2P3_GET_ETE_DMAC(ete, mac) \
    do {                                     \
        mac[5] = (ete)->dmac5; \
        mac[4] = (ete)->dmac4; \
        mac[3] = (ete)->dmac3; \
        mac[2] = (ete)->dmac2; \
        mac[1] = (ete)->dmac1; \
        mac[0] = (ete)->dmac0; \
    } while (0)


int
_bcm_fe2000_g2p3_l3_invalidate_l3_or_mpls_fte(_fe2k_l3_fe_instance_t   *l3_fe,
                                              uint32                    fte_idx)
{
    int                         rv;
    uint32                      res_idx;
    soc_sbx_g2p3_ft_t           sbxFte;
    _fe2k_l3_fte_t             *fte_hash_elem;
    _sbx_gu2_usr_res_types_t    res_type = SBX_GU2K_USR_RES_MAX;

    L3_VVERB((_SBX_D(l3_fe->fe_unit, "Enter (Fte 0x%x)\n"), fte_idx));

    soc_sbx_g2p3_ft_t_init(&sbxFte);
    sbxFte.excidx = VLAN_INV_FTE_EXC;
    rv = soc_sbx_g2p3_ft_set(l3_fe->fe_unit, fte_idx, &sbxFte);
    
    if (BCM_FAILURE(rv)) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) could not invalidate FTE 0x%x\n"),
                bcm_errmsg(rv), fte_idx));
    }

    /* Get the FTE meta data stored in the FTE hash
     */
    rv = _bcm_fe2000_l3_get_sw_fte(l3_fe, fte_idx, &fte_hash_elem);
    

    /* Unlink and free the FTE meta data 
     */
    if (BCM_SUCCESS(rv)) {
        if (fte_hash_elem->ref_count > 1) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "fte 0x%x in use"), 
                    fte_hash_elem->fte_idx));
            return BCM_E_BUSY;
        }
        
        DQ_REMOVE(&fte_hash_elem->fte_hash_link);
        sal_free(fte_hash_elem);
        L3_VERB((_SBX_D(l3_fe->fe_unit, "Freed fteHash for fteIdx 0x%x\n"), 
                 fte_idx));
    } else {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "fte_idx 0x%x not found in hash table\n"), 
                fte_idx));
    }
        
    /* Free the proper microcode resource 
     *
     *  if 
     *   fte_idx >= SBX_DYNAMIC_FTE_BASE(unit),
     *       fte_idx is an l3 FTE (will not be used as PID)
     *
     *  (res_type == SBX_GU2K_USR_RES_FTE_GLOBAL_GPORT) ||
     *  (res_type == SBX_GU2K_USR_RES_FTE_LOCAL_GPORT),
     *     fte_idx is an MPLS VPLS FTE (will be used as PID)
     *
     *  res_type == SBX_GU2K_USR_RES_VSI,
     *     fte_idx is a VSI fte, and therefore has two ftes (uc & mc)
     *     furthermore, a vsi = fte_idx - vlan_ft_base exists and must be 
     *     freed.
     */

    if ((fte_idx >= SBX_LOCAL_GPORT_FTE_BASE(l3_fe->fe_unit)) &&
        (fte_idx <= SBX_LOCAL_GPORT_FTE_END(l3_fe->fe_unit))) {
        res_type = SBX_GU2K_USR_RES_FTE_LOCAL_GPORT;
    } else if ((fte_idx >= SBX_GLOBAL_GPORT_FTE_BASE(l3_fe->fe_unit)) &&
               (fte_idx <= SBX_GLOBAL_GPORT_FTE_END(l3_fe->fe_unit))) {
        res_type = SBX_GU2K_USR_RES_FTE_GLOBAL_GPORT;
    } else if ((fte_idx >= SBX_DYNAMIC_VSI_FTE_BASE(l3_fe->fe_unit)) && 
               (fte_idx <= SBX_DYNAMIC_VSI_FTE_END(l3_fe->fe_unit))) {
        res_type = SBX_GU2K_USR_RES_VSI;
    } else if ((SOC_SBX_CFG(l3_fe->fe_unit)->mplstp_ena) &&
               (fte_idx >= SBX_VPWS_UNI_FTE_BASE(l3_fe->fe_unit)) &&
               (fte_idx <= SBX_VPWS_UNI_FTE_END(l3_fe->fe_unit))) {
        res_type = SBX_GU2K_USR_RES_FTE_VPWS_UNI_GPORT;
    } else if ((fte_idx  >= SBX_DYNAMIC_FTE_BASE(l3_fe->fe_unit)) && 
               (fte_idx <= SBX_DYNAMIC_FTE_END(l3_fe->fe_unit))) {
        res_type = SBX_GU2K_USR_RES_FTE_L3;
    } else if ((fte_idx >= SBX_EXTRA_FTE_BASE(l3_fe->fe_unit)) &&
               (fte_idx <= SBX_EXTRA_FTE_END(l3_fe->fe_unit))) {
        res_type = SBX_GU2K_USR_RES_FTE_L3_MPATH;
    }
        
    if (res_type == SBX_GU2K_USR_RES_MAX) {
        L3_ERR((_SBX_D(l3_fe->fe_unit,
                       "no resource type found for fte idx 0x%x\n"), 
                fte_idx));
        return BCM_E_INTERNAL;
    }

    if (res_type == SBX_GU2K_USR_RES_VSI) {
        int mc_fte_idx;

        /* convert the fte_idx to a VSI & mc fte */
        res_idx    = fte_idx - l3_fe->vlan_ft_base;
        mc_fte_idx = fte_idx + l3_fe->umc_ft_offset;

        /* VSIs have two FTEs UC & MC; Now clear out the MC copy */
        rv = soc_sbx_g2p3_ft_set(l3_fe->fe_unit, mc_fte_idx, &sbxFte);
        if (BCM_FAILURE(rv)) {
            L3_ERR((_SBX_D(l3_fe->fe_unit,
                           "failed to clear MC fte_idx 0x%x: %d %s\n"),
                    mc_fte_idx, rv, bcm_errmsg(rv)));
            return rv;
        }
    } else {
        res_idx = fte_idx;
    }

    rv = _sbx_gu2_resource_free(l3_fe->fe_unit, res_type, 1, &res_idx, 0);

    L3_VERB((_SBX_D(l3_fe->fe_unit,
                    "freed %s at 0x%x (fti=0x%x): %s\n"),
             _sbx_gu2_resource_to_str(res_type),
             res_idx, fte_idx, bcm_errmsg(rv)));

    if (BCM_FAILURE(rv)) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, 
                       "error(%s) could not free %s idx 0x%x\n"),
                bcm_errmsg(rv),
                _sbx_gu2_resource_to_str(res_type), res_idx));
    }
    
    return rv;
}

int
_bcm_fe2000_g2p3_l3_update_smac_tables(_fe2k_l3_fe_instance_t *l3_fe,
                                       bcm_mac_t               smac,
                                       int                     op,
                                       uint32                  ismacIdx,
                                       uint32                  esmacIdx)
{
    soc_sbx_g2p3_esmac_t  esmac;
    soc_sbx_g2p3_lsmac_t  lsmac;
    int                   rv, lastRv;

    lastRv = BCM_E_NONE;

    soc_sbx_g2p3_lsmac_t_init(&lsmac);
    soc_sbx_g2p3_esmac_t_init(&esmac);

    L3_VVERB((_SBX_D(l3_fe->fe_unit, "Enter (ingrSmacIdx %d egrSmacIdx %d  "
                     "%x:%x:%x:%x:%x:%x)\n"),
              ismacIdx, esmacIdx,
              smac[0], smac[1], smac[2], smac[3], smac[4], smac[5]));

    if (op == L3_INTF_ADD_SMAC) {
        sal_memcpy(lsmac.mac, smac, sizeof(bcm_mac_t));

        esmac.smac5 = smac[5];
        esmac.smac4 = smac[4];
        esmac.smac3 = smac[3];
        esmac.smac2 = smac[2];
        esmac.smac1 = smac[1];
        esmac.smac0 = smac[0];

        lsmac.useport = 0;
    }

    rv = soc_sbx_g2p3_lsmac_set(l3_fe->fe_unit, ismacIdx, &lsmac);
    if (BCM_FAILURE(rv)) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Failed to set lsm idx=%d: %d %s\n"),
                ismacIdx, rv, bcm_errmsg(rv)));
        /* keep trying */
        lastRv = rv;
    }

    rv = soc_sbx_g2p3_esmac_set(l3_fe->fe_unit, esmacIdx, &esmac);
    if (BCM_FAILURE(rv)) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Failed to set esmac idx=%d: %d %s\n"),
                esmacIdx, rv, bcm_errmsg(rv)));
        /* keep trying */
        lastRv = rv;
    }

    return lastRv;
}

int
_bcm_fe2000_g2p3_l3_hw_init(int unit, _fe2k_l3_fe_instance_t *l3_fe)
{
    soc_sbx_g2p3_etel2_t  sbxEte;
    soc_sbx_g2p3_v2e_t    sbxVlan;
    int                   rv;


    soc_sbx_g2p3_etel2_t_init(&sbxEte);
    sbxEte.nostrip  = 1;
    sbxEte.stpcheck = 0;
    sbxEte.mtu      = _FE2K_DEFAULT_EGR_MTU;
    sbxEte.nosplitcheck = 1;
    rv = soc_sbx_g2p3_etel2_set(l3_fe->fe_unit,
                                l3_fe->fe_raw_ete_idx,
                                &sbxEte);
    if (BCM_FAILURE(rv)) {
        L3_ERR((_SBX_D(unit, "failure(%s) to set raw-ete\n"), bcm_errmsg(rv)));
        _bcm_fe2000_l3_sw_uninit(unit);
        return rv;
    }

    soc_sbx_g2p3_v2e_t_init(&sbxVlan);
    sbxVlan.dontlearn = 1;
    sbxVlan.vrf = BCM_L3_VRF_DEFAULT;
#if 0
    VRF -> 16 K is Invalid in g2p3... Fix this
    rv = soc_sbx_g2p3_v2e_set(l3_fe->fe_unit,
                              l3_fe->fe_vsi_default_vrf,
                              &sbxVlan);
    if (BCM_FAILURE(rv)) {
        _bcm_fe2000_l3_sw_uninit(unit);
        return rv;
    }
#endif
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_fe2000_map_l2_ete
 * Purpose:
 *     Fill in the L2 HW ete from SW state
 * Parameters:
 *     l3_fe       - (IN)  l3 fe instance
 *     l3_ete      - (IN)  SW ete context
 *     smac_idx    - (IN)  Smac index for l2 ete
 *     vid         - (IN)  Vid from l3_intf
 *     mtu         - (IN)  Egress Mtu
 *     l2_hw_ete   - (OUT) hw ete fields derived from BCM
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *
 */
int
_bcm_fe2000_g2p3_map_l2_ete(_fe2k_l3_fe_instance_t    *l3_fe,
                            _fe2k_l3_ete_vidop_t       vidop,
                            uint32                     smac_idx,
                            uint32                     vid,
                            uint32                     mtu,
                            soc_sbx_g2p3_etel2_t      *l2_hw_ete)
{
    /**
     * Doing Init here is ok, since we are re-writing
     * all the fields in L2 ETE based on user passed values.
     */
    soc_sbx_g2p3_etel2_t_init(l2_hw_ete);

    L3_VVERB((_SBX_D(l3_fe->fe_unit, "Enter vidop %d, smac_idx %d vid %d mtu %d)\n"),
              (uint32)vidop, smac_idx, vid, mtu));

    if (vidop == _FE2K_L3_ETE_VIDOP__NOP) {
        vid = _BCM_VLAN_G2P3_UNTAGGED_VID;
    }

    l2_hw_ete->vid          = (uint32) vid;
    l2_hw_ete->usevid       = 1;
    l2_hw_ete->mtu          =  (uint32)  mtu;
    l2_hw_ete->smacindex    =  (uint32)  smac_idx;
    l2_hw_ete->dscpremark   = 1; 
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_fe2000_set_l2_ete
 * Purpose:
 *     Set L2 ete in HW
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     l3_sw_ete  - (IN)  l3 ete that will use this l2 ete
 *     l2_hw_ete  - (IN)  hw ete fields derived from BCM
 *     hw_ohi2etc - (IN)
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *
 */
int
_bcm_fe2000_g2p3_set_l2_ete(_fe2k_l3_fe_instance_t    *l3_fe,
                            _fe2k_ete_idx_t           *ete_idx,
                            soc_sbx_g2p3_etel2_t      *l2_hw_ete)

{
    int            status;
    uint32         ete_addr;

    status    = BCM_E_NONE;
    ete_addr  = ete_idx->ete_idx;

    L3_VVERB((_SBX_D(l3_fe->fe_unit, "Enter : L2 ETE Index 0x%x\n"), ete_addr));

    status = soc_sbx_g2p3_etel2_set(l3_fe->fe_unit, ete_addr, l2_hw_ete);

    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "error %s in setting etel2[0x%x] in HW\n"),
                bcm_errmsg(status), ete_addr));
    }

    return(status);
}

/*
 * Function:
 *     _bcm_fe2000_g2p3_update_l2_ete
 * Purpose:
 *     Get HW values, Set the specified values and
 *     program HW
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     sw_ete     - (IN)  l3 ete
 *     changes    - (IN)  params that changed
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *    1. This function is called while traversing ete link list
 *       So we cannot manipulate link list pointers
 *    2. Handle changes in smac, vid and mtu
 */
int
_bcm_fe2000_g2p3_update_l2_ete(_fe2k_l3_fe_instance_t    *l3_fe,
                          _fe2k_ete_idx_t           *ete_idx,
                          uint32                     smac_idx,
                          uint32                     vid,
                          uint32                     mtu,
                          uint32                     change_flags)
{
    soc_sbx_g2p3_etel2_t          l2_hw_ete;
    uint32                        ete_addr;
    int                           status;

    status    = BCM_E_NONE;
    ete_addr  = ete_idx->ete_idx;

    status = soc_sbx_g2p3_etel2_get(l3_fe->fe_unit,
                               ete_addr,
                               &l2_hw_ete);
    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "error %s in getting L2 ete (%d) in HW"),
                bcm_errmsg(status), ete_addr));
    } else {

        if (change_flags & L3_INTF_MTU_CHANGED) {
            l2_hw_ete.mtu          =  (uint32)  mtu;
        }

        if (change_flags & L3_INTF_VID_CHANGED) {
            /*
             * Do not change vidop here
             */
            l2_hw_ete.vid          = (uint32) vid;
        }

        if (change_flags & L3_INTF_MAC_CHANGED) {
            l2_hw_ete.smacindex = smac_idx;
        }

        status = soc_sbx_g2p3_etel2_set(l3_fe->fe_unit,
                                   ete_addr,
                                   &l2_hw_ete);
        if (status != BCM_E_NONE) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "error %s in setting L2 ete (%d) in HW"),
                    bcm_errmsg(status), ete_addr));
        }
    }
    return status;
}

/*
 * Function:
 *     _bcm_fe2000_g2p3_map_l3_ete
 * Purpose:
 *     Fill in the ipv4 HW ete and OHI from SW state
 * Parameters:
 *     l3_fe       - (IN)  l3 fe instance
 *     sw_ete      - (IN)  SW ete context
 *     l2_ete      - (IN)  l2 ete addr
 *     port        - (IN)  egress port
 *     flags       - (IN)  bcm_egr flags
 *     hw_ete      - (OUT) hw ete fields derived from BCM
 *     hw_ohi2etc  - (OUT) outheader Index
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *
 */
int
_bcm_fe2000_g2p3_map_l3_ete(_fe2k_l3_fe_instance_t    *l3_fe,
                            _fe2k_l3_ete_t            *sw_ete,
                            _fe2k_ete_idx_t           *l2_ete,
                            bcm_port_t                 port,
                            uint32                     flags,
                            soc_sbx_g2p3_eteencap_t   *encapete,
                            soc_sbx_g2p3_oi2e_t       *oi2e)
{
    if (oi2e == NULL || encapete == NULL || l3_fe == NULL || 
        sw_ete == NULL || l2_ete == NULL) {
        return BCM_E_PARAM;
    }

    soc_sbx_g2p3_eteencap_t_init(encapete);
    soc_sbx_g2p3_oi2e_t_init(oi2e);

    encapete->l2ete      = (uint32)  l2_ete->ete_idx;
    encapete->dscpremark = 1;

    if (flags & BCM_L3_KEEP_DSTMAC) {
        encapete->dmacset    = 0;
        encapete->dmacsetlsb = 0;
    }

    if (flags & BCM_L3_KEEP_SRCMAC) {
        encapete->smacset    = 0;
    }

    if (flags & BCM_L3_KEEP_TTL) {
        encapete->ttlcheck   = 0;
    }


    _bcm_fe2000_port_egr_remark_idx_get(l3_fe->fe_unit, port,
                                        &encapete->remark);

    if (sw_ete->l3_ete_key.l3_ete_hk.type == _FE2K_L3_ETE__MCAST_IP) {
        encapete->dmacset = 0;
    }

    /* if needed verify is dmacset for V4 uc is non zero & smac set is 1 */

    oi2e->eteptr  = sw_ete->l3_ete_hw_idx.ete_idx;
    oi2e->counter = 0;
    _FE2K_L3_MAP_G2P3_ETE_DMAC(encapete, sw_ete->l3_ete_key.l3_ete_hk.dmac);
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_fe2000_g2p3_set_l3_ete
 * Purpose:
 *     translate bcm values to SB values and program HW
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     hw_ete     - (IN)  hw ete fields derived from BCM
 *     hw_ohi2etc - (IN)
 *     l3_sw_ete  - (IN)  updated sw ete
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *
 */
int
_bcm_fe2000_g2p3_set_l3_ete(_fe2k_l3_fe_instance_t    *l3_fe,
                            _fe2k_l3_ete_t            *l3_sw_ete,
                            soc_sbx_g2p3_eteencap_t   *encapete,
                            soc_sbx_g2p3_oi2e_t       *oi2e)
{
    int status = BCM_E_NONE;

    if (oi2e == NULL || encapete == NULL || 
        l3_fe == NULL || l3_sw_ete == NULL) {
        return BCM_E_PARAM;
    }

    L3_VERB((_SBX_D(l3_fe->fe_unit, "set L3 ete(0x%x) ohi(0x%x) in hw\n"),
             l3_sw_ete->l3_ete_hw_idx.ete_idx,
             l3_sw_ete->l3_ohi.ohi));

    status = soc_sbx_g2p3_eteencap_set(l3_fe->fe_unit,
                                       l3_sw_ete->l3_ete_hw_idx.ete_idx,
                                       encapete);

    if (status == BCM_E_NONE) {
        int ohIdx = _FE2K_L3_G2P3_ADJUST_TB_OFFSET(l3_sw_ete->l3_ohi.ohi);

        status = soc_sbx_g2p3_oi2e_set(l3_fe->fe_unit, ohIdx, oi2e);

        if (status != BCM_E_NONE) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not set outhdrIndex2ete (%d)\n"),
                    l3_sw_ete->l3_ohi.ohi));
        }

    } else {

        L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not set l3 ete (%d)"),
                l3_sw_ete->l3_ete_hw_idx.ete_idx));
    }
    return status;
}

/*
 * Function:
 *     _bcm_fe2000_g2p3_update_ipv4_ete
 * Purpose:
 *     Get HW values, Set the specified values and
 *     program HW
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     l2encap    - (IN)  l2encap sw ete
 *     sw_ete     - (IN)  l3 ete
 *     changes    - (IN)  params that changed
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *    This is not called from anywhere yet.
 */
int
_bcm_fe2000_g2p3_update_ipv4_ete(_fe2k_l3_fe_instance_t    *l3_fe,
                                 _fe2k_l3_ete_t            *l3_sw_ete)
{
    soc_sbx_g2p3_eteencap_t   encapete;
    soc_sbx_g2p3_oi2e_t       oi2e;
    int                       status;
    int                       ohIdx;

    if (l3_fe == NULL || l3_sw_ete == NULL) {
        return BCM_E_PARAM;
    }


    status = soc_sbx_g2p3_eteencap_get(l3_fe->fe_unit,
                                       l3_sw_ete->l3_ete_hw_idx.ete_idx,
                                       &encapete);

    if (BCM_FAILURE(status)) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not get Ipv4 ETE (%d)\n"), 
                l3_sw_ete->l3_ete_hw_idx.ete_idx));
        return status;
    }

    ohIdx = _FE2K_L3_G2P3_ADJUST_TB_OFFSET(l3_sw_ete->l3_ohi.ohi);
    
    status = soc_sbx_g2p3_oi2e_get(l3_fe->fe_unit, ohIdx, &oi2e);
    oi2e.eteptr = l3_sw_ete->l3_ete_hw_idx.ete_idx;
    if (BCM_SUCCESS(status)) {
        status = _bcm_fe2000_g2p3_set_l3_ete(l3_fe, l3_sw_ete,
                                             &encapete, &oi2e);
        
    }

    return status;
}


/*
 * Function:
 *     _bcm_fe2000_g2p3_map_set_l3_ucast_fte
 * Purpose:
 *     Translate BCM egress values to fields in FTE
 * Parameters:
 *     l3_fe      - (IN)     l3 fe instance
 *     bcm_egr    - (IN)     egress object passed by user
 *     hw_fte     - (OUT)    mapped values in HW fte struct
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *     This function is called after the ETE has been created
 *     (either local or remote) OR the user has specified an
 *     encoded OHI in the encap_id field of the egress structure
 */
int
_bcm_fe2000_g2p3_map_set_l3_ucast_fte(_fe2k_l3_fe_instance_t *l3_fe,
                                      _fe2k_fte_idx_t        *fte_idx,
                                      bcm_l3_egress_t        *bcm_egr)
{
    int    status = BCM_E_NONE;
    int    numcos, node, fab_port, fab_unit;
    soc_sbx_g2p3_ft_t fte;

    L3_VVERB((_SBX_D(l3_fe->fe_unit, "Enter - encap-id(0x%x)\n"),
              bcm_egr->encap_id));

    assert(fte_idx);
    assert(l3_fe);
    assert(bcm_egr);

    status = BCM_E_NONE;
    numcos = l3_fe->fe_cosq_config_numcos;

    fab_port = -1;
    fab_unit = -1;
    node = -1;

    if (!(bcm_egr->flags & BCM_L3_TGID)) {
        status = soc_sbx_node_port_get(l3_fe->fe_unit, bcm_egr->module,
                                      bcm_egr->port, &fab_unit, &node,
                                      &fab_port);

        L3_VERB((_SBX_D(l3_fe->fe_unit, "soc_sbx_node_port_get mod(0x%x) port(%d)"
                 "-> node(0x%x) port (%d)\n"),
                 bcm_egr->module, bcm_egr->port, node, fab_port));
    }

    if (status == BCM_E_NONE) {

        soc_sbx_g2p3_ft_t_init(&fte);

        if (bcm_egr->flags & BCM_L3_TGID) {

            fte.lag     = 1;
            fte.lagbase = bcm_egr->trunk << 3;
            fte.lagsize = 3; 

            L3_VERB((_SBX_D(l3_fe->fe_unit, "LagEnable trunk=%d \n"), bcm_egr->trunk));
        } else {
	    fte.qid = SOC_SBX_NODE_PORT_TO_QID(l3_fe->fe_unit,
					       node,
					       /* bcm_egr->port, */
					       fab_port,
					       numcos);

            L3_VERB((_SBX_D(l3_fe->fe_unit, "Non-Lag module %d "
                            " port %d node %d numcos %d QidLagUnion=0x%x\n"),
                     bcm_egr->module,
                     bcm_egr->port, node, numcos, fte.qid));
        }

        fte.oi     = SOC_SBX_OHI_FROM_ENCAP_ID(bcm_egr->encap_id);

        if (bcm_egr->failover_id != 0) {
            uint32            prot_fte_idx;
            soc_sbx_g2p3_ft_t prot_ft_ent;
            prot_fte_idx = _FE2K_GET_FTE_IDX_FROM_USER_HANDLE(
                                        bcm_egr->failover_if_id);
            BCM_IF_ERROR_RETURN(soc_sbx_g2p3_ft_get(l3_fe->fe_unit,
                                                    prot_fte_idx,
                                                    &prot_ft_ent));
        
            fte.oib = prot_ft_ent.oi;
            fte.qidb = prot_ft_ent.qid;                
        }
        fte.rridx  = bcm_egr->failover_id;

        status = soc_sbx_g2p3_ft_set(l3_fe->fe_unit,
                                     fte_idx->fte_idx,
                                     &fte);

        if (status != BCM_E_NONE) {
           L3_ERR((_SBX_D(l3_fe->fe_unit, "error %s in programming FTE 0x%x in HW\n"),
                   bcm_errmsg(status), fte_idx->fte_idx));
        }
    }

    return status;
}


/*
 * Function:
 *     _bcm_fe2000_g2p3_l3_get_egrif_from_fte
 * Purpose:
 *     Given an fte index get bcm egress info
 * Parameters:
 *     l3_fe      - (IN)     fe instance corresponsing to unit
 *     ul_fte     - (IN)     the fte index
 *     flags      - (IN)     L3_OR_MPLS_GET_FTE__FTE_CONTENTS_ONLY
 *                           L3_OR_MPLS_GET_FTE__VALIDATE_FTE_ONLY
 *     bcm_egr    - (OUT)    the results
 *
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 */
int
_bcm_fe2000_g2p3_l3_get_egrif_from_fte(_fe2k_l3_fe_instance_t *l3_fe,
                       uint32                  fte_idx,
                       uint32                  flags,
                       bcm_l3_egress_t        *bcm_egr)
{
    soc_sbx_g2p3_ft_t          fte;
    int                        status;
    int                        fte_node, fte_port;
    int                        numcos, i;
    bcm_module_t               trunk_module;
    bcm_trunk_add_info_t       trunk_info;
    bcm_gport_t                switch_port, fabric_port;

    L3_VVERB((_SBX_D(l3_fe->fe_unit, "Enter (Fte Index 0x%x)\n"), fte_idx));

    status    = BCM_E_NONE;
    fte_port  = 0;
    fte_node  = 0;

    status = soc_sbx_g2p3_ft_get(l3_fe->fe_unit, fte_idx, &fte);

    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not get FTE (%d) from HW\n"), fte_idx));
    } else {

        if (flags & L3_OR_MPLS_GET_FTE__VALIDATE_FTE_ONLY) {
            return BCM_E_NONE;
        }

        bcm_egr->module = SBX_INVALID_MODID;
        trunk_module    = SBX_INVALID_MODID;
        bcm_egr->port   = SBX_INVALID_PORT;
        bcm_egr->trunk  = SBX_INVALID_TRUNK;

        if (fte.lag) {

            bcm_egr->flags |= BCM_L3_TGID;
            bcm_egr->encap_id = SOC_SBX_ENCAP_ID_FROM_OHI(fte.oi);
            bcm_egr->trunk = fte.lagbase >> 3;
            status = bcm_trunk_get(l3_fe->fe_unit, bcm_egr->trunk, &trunk_info);

            if (status != BCM_E_NONE) {
                L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not retreive trunk info for trunkId %d\n"),
                        bcm_egr->trunk));
                return status;
            }
            for (i = 0; i < trunk_info.num_ports; i++) {
                trunk_module = trunk_info.tm[i];
                if (trunk_module == l3_fe->fe_my_modid) {
                    bcm_egr->module = trunk_module;
                    break;
                }
            }
        } else {

            numcos            = l3_fe->fe_cosq_config_numcos;
            SOC_SBX_NODE_PORT_FROM_QID(l3_fe->fe_unit, fte.qid, fte_node, fte_port, numcos);
            bcm_egr->encap_id   = SOC_SBX_ENCAP_ID_FROM_OHI(fte.oi);

            BCM_GPORT_MODPORT_SET(fabric_port,
                                  BCM_STK_NODE_TO_MOD(fte_node),
                                  fte_port);

            /* map fabric to fte port */
            status = bcm_sbx_stk_fabric_map_get_switch_port(l3_fe->fe_unit,
                                                            fabric_port,
                                                           &switch_port);
            if (status != BCM_E_NONE) {
                L3_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) Mapping "
                               "node(0x%x) port(0x%x) to FE port\n"),
                        bcm_errmsg(status), fte_node, fte_port));
                return status;

            }

            bcm_egr->module = BCM_GPORT_MODPORT_MODID_GET(switch_port);
            bcm_egr->port   = BCM_GPORT_MODPORT_PORT_GET(switch_port);
        }

        if (flags & L3_OR_MPLS_GET_FTE__FTE_CONTENTS_ONLY) {
            return status;
        }

        /* XXX: TBD:
         */
        if ((l3_fe->fe_my_modid == bcm_egr->module) ||
            (trunk_module == l3_fe->fe_my_modid)) {
            status = _bcm_fe2000_get_local_l3_egress_from_ohi(l3_fe, bcm_egr);
        }
    }

    return status;
}


/*
 * Function:
 *     _bcm_fe2000_g2p3_get_local_l3_egress_from_ohi
 * Purpose:
 *     Given an ohi, get bcm l3 egress info
 *
 * Parameters:
 *     l3_fe      - (IN)    l3 fe instance
 *     bcm_egr    - (IN/OUT) bcm_egress object.
 *                           (IN) encap_id
 *                           (OUT)ifid, mac addr, vlan
 * Returns:
 *     BCM_E_XXX
 *
 * Note:
 *     This is a GET function. So we go: FTE -> OHI -> Egress data
 */
int
_bcm_fe2000_g2p3_get_local_l3_egress_from_ohi(_fe2k_l3_fe_instance_t *l3_fe,
                                              bcm_l3_egress_t        *bcm_egr)
{
    int                        status;
    _fe2k_l3_ete_t            *l3_sw_ete;
    _fe2k_l3_intf_t           *l3_intf;
    _fe2k_ohi_t                ohi;
    soc_sbx_g2p3_oi2e_t        hw_ohi2etc;
    soc_sbx_g2p3_eteencap_t    ipv4_hw_ete;
    soc_sbx_g2p3_eteencap_t    mpls_hw_ete;
    int                        ohIdx;

    if ((bcm_egr == NULL)||(!l3_fe)) {
        return BCM_E_PARAM;
    }
    
    L3_VVERB((_SBX_D(l3_fe->fe_unit, "encap-id(0x%x)\n"), bcm_egr->encap_id));

    ohi.ohi = SOC_SBX_OHI_FROM_ENCAP_ID(bcm_egr->encap_id);
    status  = _bcm_fe2000_l3_sw_ete_find_by_ohi(l3_fe, &ohi, &l3_sw_ete);

    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) could not get ete from encap-id(0x%x)\n"),
                bcm_errmsg(status), bcm_egr->encap_id));
        return status;
    }

    bcm_egr->intf = l3_sw_ete->l3_intf_id;
    status        = _bcm_fe2000_l3_find_intf_by_ifid(l3_fe,
                                                     l3_sw_ete->l3_intf_id,
                                                     &l3_intf);
    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) could not get interface for (0x%x)\n"),
                bcm_errmsg(status), l3_sw_ete->l3_intf_id));
        return status;
    }

    bcm_egr->vlan   = l3_intf->if_info.l3a_vid;
    bcm_egr->flags |= 
        _FE2K_GET_EGR_FLAG_FROM_VIDOP(l3_sw_ete->l3_ete_key.l3_ete_vidop);

    /*
     * Note that the SW ohi always points to the UCAST_IP or MCAST_IP
     * ETE. Only the HW ohi is remapped to tunnels. This way we can get
     * to either one
     */
    if ((l3_sw_ete->l3_ete_key.l3_ete_hk.type == _FE2K_L3_ETE__UCAST_IP) ||
        (l3_sw_ete->l3_ete_key.l3_ete_hk.type == _FE2K_L3_ETE__MCAST_IP)) {

        status = soc_sbx_g2p3_eteencap_get(l3_fe->fe_unit,
                                           l3_sw_ete->l3_ete_hw_idx.ete_idx,
                                          &ipv4_hw_ete);
        if (status != BCM_E_NONE) {
             L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not get Ipv4 Ete (0x%x) from HW\n"),
                     l3_sw_ete->l3_ete_hw_idx.ete_idx));
            return status;
        }

        _FE2K_L3_G2P3_GET_ETE_DMAC(&ipv4_hw_ete, bcm_egr->mac_addr);

        ohIdx = _FE2K_L3_G2P3_ADJUST_TB_OFFSET(l3_sw_ete->l3_ohi.ohi);
        
        status = soc_sbx_g2p3_oi2e_get(l3_fe->fe_unit, ohIdx, &hw_ohi2etc);
        if (status != BCM_E_NONE) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not get OutHeaderIndex (0x%x) "
                           " for mpls tunnel from HW\n"),
                    l3_sw_ete->l3_ohi.ohi));
            return status;
        }

        L3_VVERB((_SBX_D(l3_fe->fe_unit, "interfaceId= 0x%x V4-ete addr(SW)= 0x%x\n"
                         "\tOHI= 0x%x Ohi2etc eteAddr(0x%x) MPLS-TUNNEL-%s\n"),
                  l3_intf->if_info.l3a_intf_id,
                  l3_sw_ete->l3_ete_hw_idx.ete_idx,
                  l3_sw_ete->l3_ohi.ohi, hw_ohi2etc.eteptr,
                  (l3_intf->if_flags & _FE2K_L3_MPLS_TUNNEL_SET)?"SET":"NOT-SET"));

        if (l3_intf->if_flags & _FE2K_L3_MPLS_TUNNEL_SET) {
            /*
             * There is a V4 ete and mpls tunnel initiator has been set.
             * So the HW ohi points to mpls tunnel ete. Therefore  get the
             * tunnel label
             */
             status = soc_sbx_g2p3_eteencap_get(l3_fe->fe_unit,
                                                hw_ohi2etc.eteptr,
                                                &mpls_hw_ete);

             if (status != BCM_E_NONE) {
                 L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not get mpls ete (0x%x) from HW\n"),
                         (int)hw_ohi2etc.eteptr));
                 return status;
             }

             bcm_egr->mpls_label = mpls_hw_ete.label2;
        }
    } else {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Internal Error\n")));
        status = BCM_E_INTERNAL;
    }

    return status;
}

/*-------------------------------*
 * Notes on Phase I - G2P3 V4 UC support *
 * Supported
 *  (1) Da lookup
 *  (2) ICMP redirect
 *  (3) ECMP
 *
 *  Not Supported
 *  (1) SA Lookup
 *  (2) RPF
 *  (3) Counting
 *-------------------------------*/

void
_bcm_fe2000_g2p3_map_ip_key(bcm_l3_route_t *info, _fe2k_g2p3_lpm_key_t *key)
{
    if (info->l3a_flags & BCM_L3_IP6) {
        int i;
    
        for (i=0; i < 16; i++) {
            key->v6.ip[i] = info->l3a_ip6_net[i] & info->l3a_ip6_mask[i];
        }
        key->v6.prefix = bcm_ip6_mask_length(info->l3a_ip6_mask);

    } else {
        key->v4.ip     = info->l3a_subnet & info->l3a_ip_mask;
        key->v4.length = bcm_ip_mask_length(info->l3a_ip_mask); 
        key->v4.ipcxt  = info->l3a_vrf;
    }
}


/*
 * Function:
 *     _bcm_fe2000_g2p3_ip_sa_route_get
 * Purpose:
 *     Program HW with route lookup data
 * Parameters:
 *     l3_fe        - (IN)  fe instance corresponsing to unit
 *     info         - (IN)  IPV4 addr/mask/vrf
 *
 * Returns:
 *     BCM_E_XXX
 * Note:
 */
int
_bcm_fe2000_g2p3_ip_sa_route_get(_fe2k_l3_fe_instance_t  *l3_fe,
                                 bcm_l3_route_t          *info,
                                 _fe2k_g2p3_sa_route_t   *sa_info)
{
    int rv = BCM_E_NONE;
    int len;

    if (info->l3a_flags & BCM_L3_IP6) {
        len = bcm_ip6_mask_length(info->l3a_ip6_mask);
        /* bcm/sbx/fe2000 layer has verified prefix is <= 64 or == 128 */
        if (len <= 64) {
            rv = soc_sbx_g2p3_ipv6sa_get(l3_fe->fe_unit,
                                         len,
                                         info->l3a_ip6_net,
                                         &sa_info->v6);
        } else {
            rv = soc_sbx_g2p3_ipv6shost_get(l3_fe->fe_unit,
                                            info->l3a_ip6_net,
                                            (soc_sbx_g2p3_ipv6shost_t *)
                                            &sa_info->v6);
        }
    } else {
        rv = soc_sbx_g2p3_ipv4sa_get(l3_fe->fe_unit,
                                     bcm_ip_mask_length((info)->l3a_ip_mask)
                                     + l3_fe->fe_ipv4_vrf_bits,
                                     info->l3a_subnet,
                                     info->l3a_vrf,
                                     &sa_info->v4);
    }

    return rv;
}

/*
 * Function:
 *     _bcm_fe2000_g2p3_ip_da_route_get
 * Purpose:
 *  obtains DA route information from soc layer
 * Parameters:
 *     l3_fe        - (IN)  fe instance corresponsing to unit
 *     info         - (IN)  IPV4/6 addr/mask/vrf
 *
 * Returns:
 *     BCM_E_XXX
 * Note:
 */
int
_bcm_fe2000_g2p3_ip_da_route_get(_fe2k_l3_fe_instance_t  *l3_fe,
                                 bcm_l3_route_t          *info,
                                 _fe2k_g2p3_da_route_t   *da_info)
{
    int rv = BCM_E_NONE;
    int len;

    if (info->l3a_flags & BCM_L3_IP6) {
        len = bcm_ip6_mask_length(info->l3a_ip6_mask);
        /* bcm/sbx/fe2000 layer has verified prefix is <= 64 or == 128 */
        if (len <= 64) {
            rv = soc_sbx_g2p3_ipv6da_get(l3_fe->fe_unit,
                                         len,
                                         info->l3a_ip6_net,
                                         &da_info->v6);
        } else {
            rv = soc_sbx_g2p3_ipv6dhost_get(l3_fe->fe_unit,
                                            info->l3a_ip6_net,
                                            (soc_sbx_g2p3_ipv6dhost_t *)
                                            &da_info->v6);
        }
    } else {
        rv = soc_sbx_g2p3_ipv4da_get(l3_fe->fe_unit,
                                     bcm_ip_mask_length((info)->l3a_ip_mask)
                                     + l3_fe->fe_ipv4_vrf_bits,
                                     info->l3a_subnet,
                                     info->l3a_vrf,
                                     &da_info->v4);
    }
    return rv;
}


/*
 * Function:
 *     _bcm_fe2000_g2p3_ip_route_set
 * Purpose:
 *     Program HW with route lookup data
 * Parameters:
 *     l3_fe        - (IN)  fe instance corresponsing to unit
 *     info         - (IN)  route info
 *     key          - (IN)  IPV4/6 addr/mask/vrf
 *     daPayload    - (IN)  IPV4/6 da payload (for da operations, may be null)
 *     saPayload    - (IN)  IPV4/6 sa payload (for sa operations, may be null)
 *     op           - (IN)  ADD/DEL/MOD

 * Returns:
 *     BCM_E_XXX
 */
int
_bcm_fe2000_g2p3_ip_route_set(_fe2k_l3_fe_instance_t    *l3_fe,
                              bcm_l3_route_t            *info,
                              _fe2k_g2p3_lpm_key_t      *key,
                              _fe2k_g2p3_da_route_t     *daPayload,
                              _fe2k_g2p3_sa_route_t     *saPayload,
                              _fe2k_l3_route_op_t        op)

{
    int                   status = BCM_E_NONE;
    _fe2k_l3_fte_t       *fte_hash_elem;
    
    if (info->l3a_flags & BCM_L3_IP6) {
        L3_VVERB((_SBX_D(l3_fe->fe_unit, "Enter : V6 addr: " IPV6_FMT 
                         "\nprefix 0x%x (op %d)\n"),
                  IPV6_PFMT(info->l3a_ip6_net),
                  key->v6.prefix, op));
        
    } else {
        L3_VVERB((_SBX_D(l3_fe->fe_unit, "Enter : V4 addr 0x%08x len 0x%02x "
                  " ipctxt 0x%02x (op %d)\n"),
                  key->v4.ip, key->v4.length,
                  key->v4.ipcxt, op));
    }

    fte_hash_elem = NULL;

    /* quick param check */
    switch (op) {
    case _fe2k_route_op__da_add:
    case _fe2k_route_op__da_del:
    case _fe2k_route_op__da_mod:
    case _fe2k_route_op__da_get:
        if (daPayload == NULL) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "NULL da-Payload\n")));
            return BCM_E_PARAM;
        }
        break;
    case _fe2k_route_op__sa_add:
    case _fe2k_route_op__sa_del:
    case _fe2k_route_op__sa_mod:
    case _fe2k_route_op__sa_get:
        if (saPayload == NULL) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "NULL sa-Payload\n")));
            return BCM_E_PARAM;
        }
        break;
    }


    if ((op == _fe2k_route_op__da_add) ||
        (op == _fe2k_route_op__da_del) || 
        (op == _fe2k_route_op__da_mod)) 
    {
        uint32 ftidx;

        /*
         * For both add and delete of IP DA we need the
         * payload. Because we need to figure out the fte
         * whose ref_count needs to be updated
         */
        if (info->l3a_flags & BCM_L3_IP6) {
            ftidx = daPayload->v6.ftidx;
        } else {
            ftidx = daPayload->v4.ftidx;
        }

        status = _bcm_fe2000_l3_get_sw_fte(l3_fe, ftidx, &fte_hash_elem);

        if (BCM_FAILURE(status)) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "fte_idx 0x%x not found in hash table\n"), ftidx));
            return BCM_E_NOT_FOUND;
        }

        if ((op == _fe2k_route_op__da_del) &&
            (fte_hash_elem->ref_count <= 1)) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "Invalid ref_count %d on fte_idx 0x%x \n"),
                    fte_hash_elem->ref_count, ftidx));
            return BCM_E_INTERNAL;
        }
    }

    if (info->l3a_flags & BCM_L3_IP6) {
        switch (op) {
        case _fe2k_route_op__da_add:
            L3_VERB((_SBX_D(l3_fe->fe_unit, "IPv6 DA ADD: FT Index 0x%x, ecmpBits %d\n"),
                     daPayload->v6.ftidx, daPayload->v6.ecmpmask));
            
            /* bcm/sbx/fe2000 layer has verified prefix is <= 64 or == 128 */
            if (key->v6.prefix <= 64) {
                status  = 
                    soc_sbx_g2p3_ipv6da_add(l3_fe->fe_unit, 
                                            key->v6.prefix, key->v6.ip,
                                            &daPayload->v6);
            } else {
                status = soc_sbx_g2p3_ipv6dhost_add(l3_fe->fe_unit, 
                                                    key->v6.ip,
                                                   (soc_sbx_g2p3_ipv6dhost_t *)
                                                    &daPayload->v6);
            }
            
            if (BCM_SUCCESS(status)) {
                fte_hash_elem->ref_count++;
            }
            break;

        case _fe2k_route_op__da_del:
           L3_VERB((_SBX_D(l3_fe->fe_unit, "IPv6 DA DEL\n")));

            /* bcm/sbx/fe2000 layer has verified prefix is <= 64 or == 128 */
            if (key->v6.prefix <= 64) {
                status  = soc_sbx_g2p3_ipv6da_remove(l3_fe->fe_unit, 
                                                     key->v6.prefix,
                                                     key->v6.ip);
            } else {
                status  = soc_sbx_g2p3_ipv6dhost_remove(l3_fe->fe_unit, 
                                                        key->v6.ip);
            }

           if (BCM_SUCCESS(status)) {
               fte_hash_elem->ref_count--;
           }
           break;

        case _fe2k_route_op__da_mod:
            L3_VERB((_SBX_D(l3_fe->fe_unit, "IPv6 DA UPDATE FT Index 0x%x, ecmpBits %d\n"),
                     daPayload->v6.ftidx, daPayload->v6.ecmpmask));

            /* bcm/sbx/fe2000 layer has verified prefix is <= 64 or == 128 */
            if (key->v6.prefix <= 64) {
                status  = soc_sbx_g2p3_ipv6da_renew(l3_fe->fe_unit,
                                                     key->v6.prefix,
                                                     key->v6.ip,
                                                     &daPayload->v6);
            } else {
                status = soc_sbx_g2p3_ipv6dhost_update(l3_fe->fe_unit, 
                                                       key->v6.ip,
                                                   (soc_sbx_g2p3_ipv6dhost_t *)
                                                       &daPayload->v6);
            }
           break;
       case _fe2k_route_op__sa_add:
           L3_VERB((_SBX_D(l3_fe->fe_unit, "IPv6 SA ADD: PoE 0x%x, MODE %d Src Drop %d\n"),
                    saPayload->v6.poe, saPayload->v6.rpfmode,
                    saPayload->v6.srcdrop));

           /* bcm/sbx/fe2000 layer has verified prefix is <= 64 or == 128 */
           if (key->v6.prefix <= 64) {
               status  = soc_sbx_g2p3_ipv6sa_add(l3_fe->fe_unit,
                                                 key->v6.prefix, key->v6.ip,
                                                 &saPayload->v6);
           } else {
               status  = soc_sbx_g2p3_ipv6shost_add(l3_fe->fe_unit,
                                                    key->v6.ip,
                                                   (soc_sbx_g2p3_ipv6shost_t *)
                                                    &saPayload->v6);
           }
           break;
        case _fe2k_route_op__sa_del:
            L3_VERB((_SBX_D(l3_fe->fe_unit, "IPv6 SA DEL\n")));

            /* bcm/sbx/fe2000 layer has verified prefix is <= 64 or == 128 */
            if (key->v6.prefix <= 64) {
                status  = soc_sbx_g2p3_ipv6sa_remove(l3_fe->fe_unit,
                                                     key->v6.prefix,
                                                     key->v6.ip);
            } else {
                status  = soc_sbx_g2p3_ipv6shost_remove(l3_fe->fe_unit,
                                                        key->v6.ip);
            }
            break;
       case _fe2k_route_op__sa_mod:
           L3_VERB((_SBX_D(l3_fe->fe_unit, "IPv6 SA ADD: PoE 0x%x, MODE %d Src Drop %d\n"),
                    saPayload->v6.poe, saPayload->v6.rpfmode,
                    saPayload->v6.srcdrop));

           /* bcm/sbx/fe2000 layer has verified prefix is <= 64 or == 128 */
           if (key->v6.prefix <= 64) {
               status  = soc_sbx_g2p3_ipv6sa_renew(l3_fe->fe_unit,
                                                    key->v6.prefix, key->v6.ip,
                                                    &saPayload->v6);
           } else {
               status  = soc_sbx_g2p3_ipv6shost_update(l3_fe->fe_unit,
                                                       key->v6.ip,
                                                   (soc_sbx_g2p3_ipv6shost_t *)
                                                       &saPayload->v6);
           }
           break;
        default:
            status = BCM_E_INTERNAL;
            break;
        }
        
    } else {
        switch (op) {
        case _fe2k_route_op__da_add:
            L3_VERB((_SBX_D(l3_fe->fe_unit, "IPv4 DA ADD: FT Index 0x%x, ecmpBits %d vid %d\n"),
                     daPayload->v4.ftidx, daPayload->v4.ecmpmask,
                     daPayload->v4.vid));
            
            status  = 
                soc_sbx_g2p3_ipv4da_add(l3_fe->fe_unit,
                                        key->v4.length
                                        + l3_fe->fe_ipv4_vrf_bits,
                                        key->v4.ip, key->v4.ipcxt,
                                        &daPayload->v4);
    
            
            if (BCM_SUCCESS(status)) {
                fte_hash_elem->ref_count++;
            }
            break;

        case _fe2k_route_op__da_del:
           L3_VERB((_SBX_D(l3_fe->fe_unit, "IPv4 DA DEL\n")));
           status  = soc_sbx_g2p3_ipv4da_remove(l3_fe->fe_unit, 
                                                key->v4.length
                                                + l3_fe->fe_ipv4_vrf_bits,
                                                key->v4.ip, key->v4.ipcxt);
           if (BCM_SUCCESS(status)) {
               fte_hash_elem->ref_count--;
           }
           break;
        case _fe2k_route_op__da_mod:
            L3_VERB((_SBX_D(l3_fe->fe_unit, "IPv4 DA ADD: FT Index 0x%x, ecmpBits %d vid %d\n"),
                     daPayload->v4.ftidx, daPayload->v4.ecmpmask,
                     daPayload->v4.vid));

            status  = soc_sbx_g2p3_ipv4da_renew(l3_fe->fe_unit,
                                                 key->v4.length
                                                 + l3_fe->fe_ipv4_vrf_bits,
                                                 key->v4.ip, key->v4.ipcxt,
                                                 &daPayload->v4);
           if (BCM_SUCCESS(status)) {
               fte_hash_elem->ref_count++;
           }

           break;
       case _fe2k_route_op__sa_add:
           L3_VERB((_SBX_D(l3_fe->fe_unit, "IPv4 SA ADD: PoE 0x%x, MODE %d Src Drop %d\n"),
                    saPayload->v4.poe, saPayload->v4.rpfmode,
                    saPayload->v4.srcdrop));

           status  = soc_sbx_g2p3_ipv4sa_add(l3_fe->fe_unit,
                                             key->v4.length
                                             + l3_fe->fe_ipv4_vrf_bits,
                                             key->v4.ip, key->v4.ipcxt,
                                             &saPayload->v4);
           break;
        case _fe2k_route_op__sa_del:
            L3_VERB((_SBX_D(l3_fe->fe_unit, "IPv4 SA DEL\n")));
            status  = soc_sbx_g2p3_ipv4sa_remove(l3_fe->fe_unit,
                                                 key->v4.length
                                                 + l3_fe->fe_ipv4_vrf_bits,
                                                 key->v4.ip, key->v4.ipcxt);
            break;
       case _fe2k_route_op__sa_mod:
           L3_VERB((_SBX_D(l3_fe->fe_unit, "IPv4 SA ADD: PoE 0x%x, MODE %d Src Drop %d\n"),
                    saPayload->v4.poe, saPayload->v4.rpfmode,
                    saPayload->v4.srcdrop));
           status  = soc_sbx_g2p3_ipv4sa_renew(l3_fe->fe_unit,
                                                key->v4.length 
                                                + l3_fe->fe_ipv4_vrf_bits,
                                                key->v4.ip, key->v4.ipcxt,
                                                &saPayload->v4);
           break;
        default:
            status = BCM_E_INTERNAL;
            break;
        }
    }

    if (BCM_FAILURE(status)) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Error in updating route in HW: %s\n"), 
                bcm_errmsg(status)));
    }

    return status;
}

/*
 * Function:
 *     _bcm_fe2000_g2p3_map_ip_route
 * Purpose:
 *     map bcm route parameters to SB params
 * Parameters:
 *     info         - (IN)  bcm route info
 *     ecmp_set_size- (IN)  1 for non-multipath or
 *                          current ecmp_set_size otherwise
 *     port         - (IN)  Port of entry for RPF
 *     rpfmode      - (IN)  Rpf mode, one of STRICT/ LOOSE/ DISABLED
 *     key          - (IN)  IPV4/6 addr/mask/vrf
 *     da_payload   - (OUT) mapped DA payload
 *     sa_payload   - (OUT) mapped SA payload
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     Fills in route key, da and sa payloads
 */
int
_bcm_fe2000_g2p3_map_ip_route(bcm_l3_route_t            *info,
                              uint32                     ecmp_set_size,
                              _fe2k_g2p3_lpm_key_t      *key,
                              uint32                     l3a_vid,
                              uint32                     port,
                              uint32                     rpfmode,
                              uint32                     sa_drop,
                              _fe2k_g2p3_da_route_t     *da_payload,
                              _fe2k_g2p3_sa_route_t     *sa_payload)
{
    int status = BCM_E_NONE;

    if (info->l3a_flags & BCM_L3_IP6) {

        _bcm_fe2000_g2p3_map_ip_key(info, key);
        soc_sbx_g2p3_ipv6da_t_init(&da_payload->v6);
        da_payload->v6.ftidx = info->l3a_intf;


        _FE2K_GET_ECMP_BITS_FROM_ECMP_SIZE(ecmp_set_size,
                                           da_payload->v6.ecmpmask);

        soc_sbx_g2p3_ipv6sa_t_init(&sa_payload->v6);
        sa_payload->v6.poe = port;
        sa_payload->v6.rpfmode = rpfmode;
        sa_payload->v6.srcdrop = sa_drop;
    } else {
        
        _bcm_fe2000_g2p3_map_ip_key(info, key);
        
        soc_sbx_g2p3_ipv4da_t_init(&da_payload->v4);
        da_payload->v4.ftidx  = info->l3a_intf;
        
        _FE2K_GET_ECMP_BITS_FROM_ECMP_SIZE(ecmp_set_size,
                                           da_payload->v4.ecmpmask);
        da_payload->v4.vid = l3a_vid;
        
        soc_sbx_g2p3_ipv4sa_t_init(&sa_payload->v4);
        sa_payload->v4.poe = port;
        sa_payload->v4.rpfmode = rpfmode;
        sa_payload->v4.srcdrop = sa_drop;
    }
    return status;
}



typedef int (*_soc_sbx_g2p3_route_commit_f)(int, int);

int
_bcm_fe2000_g2p3_route_commit(int unit, bcm_l3_route_t *info,
                              _soc_sbx_g2p3_route_commit_f v4_commit,
                              _soc_sbx_g2p3_route_commit_f v6_commit,
                              _soc_sbx_g2p3_route_commit_f v6_host_commit)
{
    const int runLengthAll = 0xffffffffU;
    int       rv = BCM_E_INTERNAL;
    int commitV6 = 0;
    int commitV4 = 0;
    
    if (soc_sbx_g2p3_v6_ena(unit)) {
        commitV6 = ((info == NULL) || (info->l3a_flags & BCM_L3_IP6));
   
        if (commitV6) {
            if (v6_commit) {
                rv = v6_commit(unit, runLengthAll);
                if (BCM_FAILURE(rv)) {
                   L3_ERR((_SBX_D(unit, "v6 LPM route commit failed: %d %s\n"),
                        rv, bcm_errmsg(rv)));
                    /* keep trying */
                }
            } 
            if (v6_host_commit) {
                rv = v6_host_commit(unit, runLengthAll);
                if (BCM_FAILURE(rv)) {
                    L3_ERR((_SBX_D(unit, "v6 host commit failed: %d %s\n"),
                        rv, bcm_errmsg(rv)));
                    /* keep trying */
                }
            }
        }
    }
    commitV4 = ((info == NULL) || !commitV6);

    if ( commitV4 && v4_commit) {
        rv = v4_commit(unit, runLengthAll);
        if (BCM_FAILURE(rv)) {
            L3_ERR((_SBX_D(unit, "v4 LPM route commit failed: %d %s\n"),
                    rv, bcm_errmsg(rv)));
        }
    }

    return rv;
}

int
_bcm_fe2000_g2p3_da_route_commit(int unit, bcm_l3_route_t *info) {
    
     _soc_sbx_g2p3_route_commit_f v4_commit = NULL;
     _soc_sbx_g2p3_route_commit_f v6_commit = NULL;
     _soc_sbx_g2p3_route_commit_f v6host_commit = NULL;
 
     if( soc_sbx_g2p3_ipv6da_complex_state_get(unit))
        v6_commit = soc_sbx_g2p3_ipv6da_commit;
 
     if( soc_sbx_g2p3_ipv6dhost_complex_state_get(unit))
        v6host_commit = soc_sbx_g2p3_ipv6dhost_commit;
 
     if( soc_sbx_g2p3_ipv4da_complex_state_get(unit))
        v4_commit = soc_sbx_g2p3_ipv4da_commit;
 
     return _bcm_fe2000_g2p3_route_commit(unit, info,
                                          v4_commit,
                                          v6_commit,
                                          v6host_commit);

}

int
_bcm_fe2000_g2p3_sa_route_commit(int unit, bcm_l3_route_t *info) {
 
     _soc_sbx_g2p3_route_commit_f v4_commit = NULL;
     _soc_sbx_g2p3_route_commit_f v6_commit = NULL;
     _soc_sbx_g2p3_route_commit_f v6host_commit = NULL;
 
     if( soc_sbx_g2p3_ipv6sa_complex_state_get(unit))
        v6_commit = soc_sbx_g2p3_ipv6sa_commit;
 
     if( soc_sbx_g2p3_ipv6shost_complex_state_get(unit))
        v6host_commit = soc_sbx_g2p3_ipv6shost_commit;
 
     if( soc_sbx_g2p3_ipv4sa_complex_state_get(unit))
        v4_commit = soc_sbx_g2p3_ipv4sa_commit;
 
     return _bcm_fe2000_g2p3_route_commit(unit, info,
                                          v4_commit,
                                          v6_commit,
                                          v6host_commit);

}


/*
 * Function:
 *     _bcm_fe2000_g2p3_ip_route_update
 * Purpose:
 *      Add a route lookup entry in SB HW
 * Parameters:
 *     l3_fe  :  (IN)  fe instance corresponsing to unit
 *     info   :  (IN)  BCM route info
 *
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     Handles L3_ROUTE_IPV4_XX_ADD and L3_ROUTE_IPV4_XX_MOD
 */
int
_bcm_fe2000_g2p3_ip_route_update(_fe2k_l3_fe_instance_t  *l3_fe,
                                 bcm_l3_route_t          *info,
                                 uint32                   ecmp_size,
                                 uint32                   l3a_vid,
                                 _fe2k_g2p3_da_route_t    *da_payload,
                                 _fe2k_g2p3_sa_route_t    *sa_payload,
                                 _fe2k_l3_route_op_t      da_op)
{
    _fe2k_g2p3_lpm_key_t key;
    int                  status = BCM_E_NONE;
    bcm_l3_egress_t      bcm_egr;
    uint32               sa_op = -1;
    _fe2k_l3_intf_t     *l3_intf;
    soc_sbx_g2p3_pv2e_t  pv2e;
    soc_sbx_g2p3_lp_t    lp;
    uint32               port = 0; /* Set this for default PID */
    uint32               rpfmode = 0;  /* default disabled */
    uint32               sa_drop = 0;  /* default dont drop */

    if (info->l3a_flags & BCM_L3_RPF) {
        bcm_l3_egress_t_init(&bcm_egr);
        status = _bcm_fe2000_l3_get_egrif_from_fte(l3_fe,
                                                   info->l3a_intf,
                                                   L3_OR_MPLS_GET_FTE__FTE_ETE_BOTH,
                                                   &bcm_egr);

        status = soc_sbx_g2p3_pv2e_get(l3_fe->fe_unit, bcm_egr.vlan,
                                       bcm_egr.port,
                                       &pv2e);

        soc_sbx_g2p3_lp_t_init(&lp);
        if (status == BCM_E_NONE) {
            if (pv2e.lpi == 0) {
                status  = soc_sbx_g2p3_lp_get(l3_fe->fe_unit, bcm_egr.port, &lp);
            } else {
                status  = soc_sbx_g2p3_lp_get(l3_fe->fe_unit, pv2e.lpi, &lp);
            }
        } else {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "RPF error in pv2e[%d,0x%x]: %s\n"),
                    bcm_egr.port, bcm_egr.vlan, bcm_errmsg(status)));
        }

        if (status == BCM_E_NONE) {
            port = lp.pid;
        }
        /* Ignore/no RPf in case of mpls ete */
        status        = _bcm_fe2000_l3_find_intf_by_ifid(l3_fe,
                                                         bcm_egr.intf,
                                                         &l3_intf);
        if (status == BCM_E_NONE) {
            if (l3_intf->if_flags & _FE2K_L3_MPLS_TUNNEL_SET) {
                return BCM_E_PARAM;
            } else {
                rpfmode = 0x1; /* hardcode strict mode for now */
            }
        }
    }

    /* SRC_DISCARD */
    if (info->l3a_flags & BCM_L3_SRC_DISCARD) {
        sa_drop = 1;
    }

    status = _bcm_fe2000_g2p3_map_ip_route(info, ecmp_size, &key, 
                                           l3a_vid, port, rpfmode, sa_drop,
                                           da_payload, sa_payload);

    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "error in ip route map: %s\n"), bcm_errmsg(status)));
        return status;
    }

    if (info->l3a_flags & BCM_L3_RPF) {
        sa_op = (da_op == _fe2k_route_op__da_add) ?
            _fe2k_route_op__sa_add : _fe2k_route_op__sa_mod;
        
        status = _bcm_fe2000_g2p3_ip_route_set(l3_fe, info, &key,
                                               NULL, sa_payload,
                                               sa_op);
        if (BCM_FAILURE(status)) {
             L3_ERR((_SBX_D(l3_fe->fe_unit, "error in SA route set: %s\n"), 
                        bcm_errmsg(status)));
             return status;
        }
        if (_bcm_fe2000_l3_do_lpm_commit(l3_fe->fe_unit, info)) {
            /* if set was successful, commit the changes */
            status = _bcm_fe2000_g2p3_sa_route_commit(l3_fe->fe_unit, info);

            if (status != BCM_E_NONE) {
                L3_ERR((_SBX_D(l3_fe->fe_unit, "error in SA route commit: %s\n"), 
                        bcm_errmsg(status)));
            }
        }
        /* sa commit failure ? */
       if (BCM_FAILURE(status)) {
            if (sa_op == _fe2k_route_op__sa_add) {
                int tmp;
                tmp = _bcm_fe2000_g2p3_ip_route_set(l3_fe, info, &key,
                                                NULL, sa_payload,
                                                _fe2k_route_op__sa_del);
            
                if (BCM_SUCCESS(tmp) && 
                    (_bcm_fe2000_l3_do_lpm_commit(l3_fe->fe_unit, info))) {
                   tmp = _bcm_fe2000_g2p3_sa_route_commit(l3_fe->fe_unit, info);
                }
            
                if (BCM_FAILURE(tmp)) {
                    L3_ERR((_SBX_D(l3_fe->fe_unit, "Error %s in undoing SA route changes\n"),
                        bcm_errmsg(tmp)));
                }
            }
        }
    }

    status = _bcm_fe2000_g2p3_ip_route_set(l3_fe, info, 
                                           &key, da_payload, NULL,
                                           da_op);

    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "error in DA route set: %s\n"), bcm_errmsg(status)));
        return status;
    }
    
    if (_bcm_fe2000_l3_do_lpm_commit(l3_fe->fe_unit, info)) {
        status = _bcm_fe2000_g2p3_da_route_commit(l3_fe->fe_unit, info);

        if(status != BCM_E_NONE) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "error in DA route commit: %s\n"), 
                    bcm_errmsg(status)));
        }
    }
    if (BCM_FAILURE(status)) {
        if (da_op == _fe2k_route_op__da_add) {
            int tmp;
            tmp = _bcm_fe2000_g2p3_ip_route_set(l3_fe, info, &key,
                                                da_payload, NULL,
                                                _fe2k_route_op__da_del);
            
            if (BCM_SUCCESS(tmp) && 
                (_bcm_fe2000_l3_do_lpm_commit(l3_fe->fe_unit, info))) {
                tmp = _bcm_fe2000_g2p3_da_route_commit(l3_fe->fe_unit, info);
            }
            
            if (BCM_FAILURE(tmp)) {
                L3_ERR((_SBX_D(l3_fe->fe_unit, "Error %s in undoing DA route changes\n"),
                        bcm_errmsg(tmp)));
            }
        }
        return status;
    }
    
    return status;
}


/*
 * Function:
 *      bcm_fe2000_g2p3_l3_route_add
 * Purpose:
 *      Add an IP route to the routing table
 * Parameters:
 *     unit  - (IN) FE2000 unit number
 *     info  - (IN) Pointer to bcm_l3_route_t containing all valid fields.
 * Returns:
 *      BCM_E_XXX
 *
 * Assumptions:
 *     info->l3a_intf must contain the FTE Index returned at egress create
 *     The following fields in the route structure are ignored for the
 *     purpose of this API (they are needed at egress create time);
 *     l3a_nexthop_ip;  Next hop IP address (XGS1/2, IPv4)
 *     l3a_nexthop_mac;
 *     l3a_modid;
 *     l3a_stack_port;  Used if modid not local (Strata Only)
 *     l3a_vid;         BCM5695 only - for per-VLAN def route
 *
 * Notes:
 *     1. if BCM_L3_RPF flag is set, then the route is added to SA table also
 *     2. if the route is to be added to SA table,
 *        - XXX: how to set ulDropMask, ulDrop, ulStatsPolicerId will not be set
 *     3. In the DA payload field, the ulProcCopy will not be set
 *        - XXX: How do we map the pri field to cos. because the FTE has already
 *               been created. Do we modify the FTE here ?
 */
int
bcm_fe2000_g2p3_l3_route_add( _fe2k_l3_fe_instance_t  *l3_fe,
                              bcm_l3_route_t          *info,
                              uint32                   vid,
                              uint32                   ecmp_size)
{
    _fe2k_g2p3_da_route_t    da_payload;
    _fe2k_g2p3_sa_route_t    sa_payload;
    int                      fte_idx;
    int                      status = BCM_E_NONE , sa_status = BCM_E_NONE;

    fte_idx =  _FE2K_GET_FTE_IDX_FROM_USER_HANDLE(info->l3a_intf);

    status = _bcm_fe2000_g2p3_ip_da_route_get(l3_fe, info, &da_payload);

    /* If RPF flag is set, verify if sa is present on lpm */
    if (info->l3a_flags & BCM_L3_RPF) {
        sa_status = _bcm_fe2000_g2p3_ip_sa_route_get(l3_fe, info, &sa_payload);
    }

    if (info->l3a_flags & BCM_L3_REPLACE) {
        _fe2k_l3_fte_t       *fte_hash_elem = NULL;

        if(status != BCM_E_NONE) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not find route to update \n")));
            return status;
        }        
        
        status = _bcm_fe2000_l3_get_sw_fte(l3_fe, 
                                           /*_FE2K_GET_USER_HANDLE_FROM_FTE_IDX(da_payload.v4.ftidx),*/
                                           da_payload.v4.ftidx,
                                           &fte_hash_elem);
        if(status != BCM_E_NONE) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not find FTE hash element\n")));
            return status;
        }
        
        if ((info->l3a_flags & BCM_L3_RPF) && BCM_FAILURE(sa_status)) {
            return sa_status;
        }

        /*
         * All internal functions need the real fte_idx and not the
         * user handle. Same for the add case below
         */
        info->l3a_intf   = fte_idx;
        status = _bcm_fe2000_g2p3_ip_route_update(l3_fe, info, ecmp_size, vid,
                                                  &da_payload, &sa_payload,
                                                  _fe2k_route_op__da_mod);
        if(status == BCM_E_NONE) {
            fte_hash_elem->ref_count--;
        }
    }
    else {

        if (BCM_SUCCESS(status)) { 
            L3_ERR((_SBX_D(l3_fe->fe_unit, "DA found on unit\n")));
            return BCM_E_EXISTS;
        }
        if ((info->l3a_flags & BCM_L3_RPF) && BCM_SUCCESS(sa_status)) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "SA found on unit\n")));
            return BCM_E_EXISTS;
        }

        info->l3a_intf   = fte_idx;
        status = _bcm_fe2000_g2p3_ip_route_update(l3_fe, info, ecmp_size, vid,
                                                  &da_payload, &sa_payload,
                                                  _fe2k_route_op__da_add);
    }
    
    return status;
}

/*
 * Function:
 *     _bcm_fe2000_g2p3_ip_route_delete
 * Purpose:
 *      delete a route lookup entry in SB HW
 * Parameters:
 *     l3_fe  :  (IN)  fe instance corresponsing to unit
 *     info   :  (IN)  BCM route info
 *
 * Returns:
 *     BCM_E_XXX
 */
int
_bcm_fe2000_g2p3_ip_route_delete(_fe2k_l3_fe_instance_t *l3_fe,
                                 bcm_l3_route_t *info)
{
    _fe2k_g2p3_lpm_key_t       key;
    _fe2k_g2p3_da_route_t      da_payload;
    _fe2k_g2p3_sa_route_t      sa_payload;
    int                        status, sa_status = BCM_E_NONE;

    L3_VERB((_SBX_D(l3_fe->fe_unit, "Enter\n")));

    _bcm_fe2000_g2p3_map_ip_key(info, &key);
    
    status = _bcm_fe2000_g2p3_ip_da_route_get(l3_fe, info, &da_payload);

    /* If route information could not be obtained flag error */
    if (BCM_FAILURE(status)) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "error in DA route get for delete: %s\n"), 
                bcm_errmsg(status)));
        return status;
    }

    /* If RPF flag is set, verify if sa is present on lpm */
    if (info->l3a_flags & BCM_L3_RPF) {
        sa_status = _bcm_fe2000_g2p3_ip_sa_route_get(l3_fe, info, &sa_payload);

        if (BCM_FAILURE(sa_status)) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "error in SA route get for delete: %s\n"),
                    bcm_errmsg(status)));
            return sa_status;
        }
    }
    
    status = _bcm_fe2000_g2p3_ip_route_set(l3_fe, info, &key,
                                           &da_payload, NULL,
                                           _fe2k_route_op__da_del);

    if (BCM_FAILURE(status)) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "error in DA route delete: %s\n"),
                bcm_errmsg(status)));
    } else {
        L3_VERB((_SBX_D(l3_fe->fe_unit, "Successfully removed Ipv4 DA\n")));

        if (_bcm_fe2000_l3_do_lpm_commit(l3_fe->fe_unit, info) ) {
            status = _bcm_fe2000_g2p3_da_route_commit(l3_fe->fe_unit, info);
        
            if (BCM_FAILURE(status)) {
                L3_ERR((_SBX_D(l3_fe->fe_unit, "Error %s in DA Route Commit \n"), 
                    bcm_errmsg(status)));
                return status;
            } else {
                L3_VERB((_SBX_D(l3_fe->fe_unit, "Successfully Committed deleted Ipv4 routes\n")));
            }
         }

         if (info->l3a_flags & BCM_L3_RPF) {
            status = _bcm_fe2000_g2p3_ip_route_set(l3_fe, info, &key,
                                                       NULL, &sa_payload,
                                                       _fe2k_route_op__sa_del);
               
            if (BCM_FAILURE(status)) {
                    L3_ERR((_SBX_D(l3_fe->fe_unit, "error in SA route delete: %s\n"),
                            bcm_errmsg(status)));
            } else {
                if (_bcm_fe2000_l3_do_lpm_commit(l3_fe->fe_unit, info) ) {

                    status =
                           _bcm_fe2000_g2p3_sa_route_commit(l3_fe->fe_unit,
                                                             info);

                    if (BCM_FAILURE(status)) {
                            L3_ERR((_SBX_D(l3_fe->fe_unit, "Error %s in SA Route Commit \n"),
                                    bcm_errmsg(status)));
                    } else {
                            L3_VERB((_SBX_D(l3_fe->fe_unit, "Successfully Committed "
                                            "deleted Ipv4 sa routes\n")));
                    }
                }
            }
        }
    }
    
    return status;
}

/*
 * Function:
 *     _bcm_fe2000_g2p3_ipv4_route_delete_all
 * Purpose:
 *      delete all routes or routes that share the same fte
 * Parameters:
 *     l3_fe  :  (IN)  fe instance corresponsing to unit
 *     fte_idx:  (IN)  fte index to match while deleting route
 *     flags  :  (IN)  L3_ROUTE_DELETE_BY_INTF
 *                     L3_ROUTE_DELETE_ALL
 * Returns:
 *     BCM_E_XXX
 */
int
_bcm_fe2000_g2p3_ipv4_route_delete_all(_fe2k_l3_fe_instance_t *l3_fe,
                                       uint32 fte_idx,
                                       uint32 flags)
{
    /* Fixme -- Use lpm flush or walk tree and delete routes */
    /* lpm Flush ?? */
    return BCM_E_UNAVAIL;
}

int
_bcm_fe2000_g2p3_ip_ecmp_route_get(_fe2k_l3_fe_instance_t *l3_fe,
                                   bcm_l3_route_t         *info,
                                   uint32                 *fte_base)
{
    _fe2k_g2p3_da_route_t da_payload;
    int    status = BCM_E_NONE;

    status = _bcm_fe2000_g2p3_ip_da_route_get(l3_fe, info, &da_payload);
    if (BCM_FAILURE(status)) {
        return status;
    }

    if (info->l3a_flags & BCM_L3_IP6) {
        if (da_payload.v6.ecmpmask == 0) {
            return BCM_E_PARAM;
        }
        
        *fte_base = da_payload.v6.ftidx;
    } else {
        if (da_payload.v4.ecmpmask == 0) {
            return BCM_E_PARAM;
        }
        
        *fte_base = da_payload.v4.ftidx;
    }
    return status;
}

void*
_bcm_fe2000_g2p3_alloc_fte(uint32 count)
{
    void *ptr = NULL;

    if(count < _FE2K_L3_ECMP_MAX){
        ptr = sal_alloc(sizeof(soc_sbx_g2p3_ft_t) * count,"g2p3 fte");
        if(!ptr) {
            L3_ERR(("  %s: Out of memory\n", FUNCTION_NAME()));
        }
    } else {
        L3_ERR(("  %s: Hit max %d ECMP FTEs\n", 
                FUNCTION_NAME(), _FE2K_L3_ECMP_MAX));
    }
    return ptr;
}

int
_bcm_fe2000_g2p3_fte_op(_fe2k_l3_fe_instance_t    *l3_fe,
                         uint32                     ft_idx,
                         void                      *fte,
                         _fe2k_fte_opcode           op)
{
    int status = BCM_E_NONE;
    soc_sbx_g2p3_ft_t *ft;
    soc_sbx_g2p3_ft_t hw_fte;

    assert(l3_fe);

    if(_FE2K_FT_OPCODE_CLR == op) {
        soc_sbx_g2p3_ft_t_init(&hw_fte);
        ft = &hw_fte;
        op = _FE2K_FT_OPCODE_SET;
    } else {
        assert(fte);
        ft = (soc_sbx_g2p3_ft_t*)fte;
    }

    switch(op) {
        case _FE2K_FT_OPCODE_SET:
            status = soc_sbx_g2p3_ft_set(l3_fe->fe_unit,
                                         ft_idx,
                                         ft);
        break;

        case _FE2K_FT_OPCODE_GET:
            status = soc_sbx_g2p3_ft_get(l3_fe->fe_unit,
                                         ft_idx,
                                         ft);

        break;
        default:
            assert(0);
    }

    return status;
}

int
_bcm_fe2000_g2p3_l3_flush_cache(_fe2k_l3_fe_instance_t    *l3_fe, int flag)
{
    int status = BCM_E_NONE;

    assert(l3_fe);
    if (flag & 0x1) {
        status = _bcm_fe2000_g2p3_da_route_commit(l3_fe->fe_unit, NULL);
        if (BCM_FAILURE(status))
           return status;
    }
    if (flag & 0x2) {
        status =  _bcm_fe2000_g2p3_sa_route_commit(l3_fe->fe_unit, NULL);
        if (BCM_FAILURE(status))
           return status;
    }
    return status;
}

#endif /* INCLUDE_L3 */
