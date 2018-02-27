/*
 * $Id: mcast.c 1.7.236.1 Broadcom SDK $
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
 * File:        mcast.c
 * Purpose:     Tracks and manages L2 Multicast tables.
 */

#include <soc/defs.h>
#if defined(BCM_EASYRIDER_SUPPORT) 

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/l2x.h>

#include <bcm/error.h>
#include <bcm/l2.h>

#include <bcm_int/esw/easyrider.h>
#include <bcm_int/esw/firebolt.h>
#include <bcm_int/esw_dispatch.h>

#define L2MC_LOCK(unit)         do {            \
        soc_mem_lock(unit, L2_ENTRY_INTERNALm);   \
        soc_mem_lock(unit, L2MC_TABLEm);   \
    } while(0)
#define L2MC_UNLOCK(unit)       do {            \
        soc_mem_unlock(unit, L2MC_TABLEm); \
        soc_mem_unlock(unit, L2_ENTRY_INTERNALm); \
    } while(0)

/*
 * Function:
 *      bcm_er_mcast_init
 * Purpose:
 *      Initialize chip-dependent parts of multicast module
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_mcast_init(int unit)
{
    return bcm_xgs3_mcast_init(unit);
}

/*
 * Function:
 *      bcm_er_mcast_port_add
 * Purpose:
 *      Add ports to existing MC entry.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_er_mcast_port_add(int unit, bcm_mcast_addr_t *mcaddr)
{
    bcm_l2_addr_t       l2addr;
    bcm_pbmp_t          pbm;
    int rv;

    if (!SOC_CONTROL(unit)->l2x_l2mc_in_l2entry) {
        /* Configured for L2MC_PBM in L2MC table, call common XGS3 function. */
        return bcm_xgs3_mcast_port_add(unit, mcaddr);
    }

    /* Configured for L2MC_PBM in L2 entry. */
    L2MC_LOCK(unit);
    rv = _bcm_er_l2_addr_get(unit, mcaddr->mac, mcaddr->vid, &l2addr, &pbm);

    if ((rv < 0) || !(l2addr.flags & BCM_L2_MCAST)) {
        L2MC_UNLOCK(unit);
        return BCM_E_NOT_FOUND;
    }
    BCM_PBMP_OR(pbm, mcaddr->pbmp);
    rv = _bcm_er_l2_addr_add(unit, &l2addr, pbm);
    L2MC_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_er_mcast_port_remove
 * Purpose:
 *      Remove ports from existing MC entry.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_er_mcast_port_remove(int unit, bcm_mcast_addr_t *mcaddr)
{
    bcm_l2_addr_t       l2addr;
    bcm_pbmp_t          pbm;
    int rv;

    if (!SOC_CONTROL(unit)->l2x_l2mc_in_l2entry) {
        /* Configured for L2MC_PBM in L2MC table, call common XGS3 function. */
        return bcm_xgs3_mcast_port_remove(unit, mcaddr);
    }

    /* Configured for L2MC_PBM in L2 entry. */
    L2MC_LOCK(unit);
    rv = _bcm_er_l2_addr_get(unit, mcaddr->mac, mcaddr->vid, &l2addr, &pbm);

    if ((rv < 0) || !(l2addr.flags & BCM_L2_MCAST)) {
        L2MC_UNLOCK(unit);
        return BCM_E_NOT_FOUND;
    }
    BCM_PBMP_REMOVE(pbm, mcaddr->pbmp);
    rv = _bcm_er_l2_addr_add(unit, &l2addr, pbm);
    L2MC_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_er_mcast_addr_add
 * Purpose:
 *      Add new L2 multicast entry
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_er_mcast_addr_add(int unit, bcm_mcast_addr_t *mcaddr)
{
    bcm_l2_addr_t       l2addr, rl2addr;
    bcm_pbmp_t          pbm, rpbm;
    int rv;

    if (!SOC_CONTROL(unit)->l2x_l2mc_in_l2entry) {
        /* Configured for L2MC_PBM in L2MC table, call common XGS3 function. */
        return bcm_xgs3_mcast_addr_add(unit, mcaddr);
    }

    /* Configured for L2MC_PBM in L2 entry */
    L2MC_LOCK(unit);
    rv = _bcm_er_l2_addr_get(unit, mcaddr->mac, mcaddr->vid, &l2addr, &pbm);
    if (rv >= 0) {
        L2MC_UNLOCK(unit);
        return BCM_E_EXISTS;
    }

    if (SOC_MCAST_ADD_ALL_ROUTER_PORTS(unit)) {
        rv = _bcm_er_l2_addr_get(unit, (uint8 *)_soc_mac_all_routers,
                                 mcaddr->vid, &rl2addr, &rpbm);
        if (rv >= 0) {
            BCM_PBMP_OR(rpbm, mcaddr->pbmp);
            _bcm_er_l2_addr_add(unit, &rl2addr, rpbm);
        }
    }

    /* add l2 entry */
    sal_memset(&l2addr, 0, sizeof(l2addr));
    l2addr.flags = BCM_L2_STATIC | BCM_L2_MCAST | BCM_L2_REPLACE_DYNAMIC;
    l2addr.cos_dst = mcaddr->cos_dst;
    l2addr.vid = mcaddr->vid;
    sal_memcpy(l2addr.mac, mcaddr->mac, sizeof(bcm_mac_t));

    rv = _bcm_er_l2_addr_add(unit, &l2addr, mcaddr->pbmp);
    L2MC_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_er_mcast_addr_add_w_l2mcindex
 * Purpose:
 *      Add a new MC entry to the L2 and L2MC tables, with given MC index.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_er_mcast_addr_add_w_l2mcindex(int unit, bcm_mcast_addr_t *mcaddr)
{
    if (!SOC_CONTROL(unit)->l2x_l2mc_in_l2entry) {
        /* Configured for L2MC_PBM in L2MC table, call common XGS3 function. */
        return bcm_xgs3_mcast_addr_add_w_l2mcindex(unit, mcaddr);
    }

    /* Configured for L2MC_PBM in L2 entry, l2mcindex doesn't apply here. */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_er_mcast_addr_remove
 * Purpose:
 *      Remove a multicast entry
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_er_mcast_addr_remove(int unit, sal_mac_addr_t mac, bcm_vlan_t vid)
{
    int                 rv;
    bcm_l2_addr_t       l2addr;
    bcm_pbmp_t          pbm;

    if (!SOC_CONTROL(unit)->l2x_l2mc_in_l2entry) {
        /* Configured for L2MC_PBM in L2MC table, call common XGS3 function. */
        return bcm_xgs3_mcast_addr_remove(unit, mac, vid);
    }

    /* Configured for L2MC_PBM in L2 entry */
    L2MC_LOCK(unit);

    rv = _bcm_er_l2_addr_get(unit, mac, vid, &l2addr, &pbm);
    if (rv < 0) {
        L2MC_UNLOCK(unit);
        return rv;
    }

    if (!(l2addr.flags & BCM_L2_MCAST)) {
        L2MC_UNLOCK(unit);
        return BCM_E_NOT_FOUND;
    }

    rv = bcm_esw_l2_addr_delete(unit, mac, vid);
    if (rv < 0) {
        L2MC_UNLOCK(unit);
        return rv;
    }

    L2MC_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_er_mcast_addr_remove_w_l2mcindex
 * Purpose:
 *      Remove a multicast entry with l2mc index provided
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_er_mcast_addr_remove_w_l2mcindex(int unit,
                                     bcm_mcast_addr_t *mcaddr)
{
    if (!SOC_CONTROL(unit)->l2x_l2mc_in_l2entry) {
        /* Configured for L2MC_PBM in L2MC table, call common XGS3 function. */
        return bcm_xgs3_mcast_addr_remove(unit, mcaddr->mac, mcaddr->vid);
    }

    /* Configured for L2MC_PBM in L2 entry, l2mcindex doesn't apply here. */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_er_mcast_port_get
 * Purpose:
 *      Get port bit maps for a multicast group.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_er_mcast_port_get(int unit,
                      sal_mac_addr_t mac, bcm_vlan_t vid,
                      bcm_mcast_addr_t *mcaddr)
{
    bcm_l2_addr_t     l2addr;
    int               rv;
    vlan_tab_entry_t  vte;
    uint32            ubm;

    if (!SOC_CONTROL(unit)->l2x_l2mc_in_l2entry) {
        /* Configured for L2MC_PBM in L2MC table, call common XGS3 function. */
        return bcm_xgs3_mcast_port_get(unit, mac, vid, mcaddr);
    }

    /* Configured for L2MC_PBM in L2 entry */
    BCM_IF_ERROR_RETURN(
        _bcm_er_l2_addr_get(unit, mac, vid, &l2addr, &mcaddr->pbmp));

    sal_memcpy(mcaddr->mac, l2addr.mac, sizeof(bcm_mac_t));
    mcaddr->vid = l2addr.vid;
    mcaddr->cos_dst = l2addr.cos_dst;
    mcaddr->l2mc_index = 0;

    /* Need to fill in the untagged port bitmap from the VLAN table */
    SOC_PBMP_CLEAR(mcaddr->ubmp);
    rv = soc_mem_read(unit, VLAN_TABm, MEM_BLOCK_ANY, mcaddr->vid, &vte);
    if (rv < 0) {
        return rv;
    }
    ubm = soc_mem_field32_get(unit, VLAN_TABm, &vte, UT_BITMAPf);
    SOC_PBMP_WORD_SET(mcaddr->ubmp, 0, ubm);

    return BCM_E_NONE;
}
#else  /* BCM_EASYRIDER_SUPPORT */
int bcm_esw_easyrider_mcast_not_empty;
#endif /* BCM_EASYRIDER_SUPPORT */
