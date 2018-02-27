/*
 * $Id: mcast.c 1.25.166.1 Broadcom SDK $
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

#include <soc/drv.h>
#include <soc/mem.h>

#include <bcm/error.h>
#include <bcm/l2.h>

#include <bcm_int/esw/draco.h>
#include <bcm_int/esw_dispatch.h>

typedef struct {
    int         size;
    int         *used;
    soc_mem_t   l2mem;
    soc_mem_t   mcmem;
} _bcm_xgs_mcast_t;

static _bcm_xgs_mcast_t xgs_l2mc_info[BCM_MAX_NUM_UNITS];

#define L2MC_INFO(unit)         (&xgs_l2mc_info[unit])
#define L2MC_SIZE(unit)         L2MC_INFO(unit)->size
#define L2MC_USED(unit)         L2MC_INFO(unit)->used
#define L2MC_L2MEM(unit)        L2MC_INFO(unit)->l2mem
#define L2MC_MCMEM(unit)        L2MC_INFO(unit)->mcmem
#define L2MC_USED_SET(unit, n)  L2MC_USED(unit)[n] += 1
#define L2MC_USED_CLR(unit, n)  L2MC_USED(unit)[n] -= 1
#define L2MC_USED_ISSET(unit, n) (L2MC_USED(unit)[n] > 0)

#define L2MC_INIT(unit) \
        if (L2MC_USED(unit) == NULL) { return BCM_E_INIT; }
#define L2MC_ID(unit, id) \
        if (id < 0 || id >= L2MC_SIZE(unit)) { return BCM_E_PARAM; }

#define L2MC_LOCK(unit)         do {            \
        soc_mem_lock(unit, L2MC_MCMEM(unit));   \
    } while(0)
#define L2MC_UNLOCK(unit)       do {            \
        soc_mem_unlock(unit, L2MC_MCMEM(unit)); \
    } while(0)

/*
 * Function:
 *      _bcm_xgs_l2mc_free_index
 * Description:
 *      Find an unused l2mc index and mark it used.
 */

STATIC int
_bcm_xgs_l2mc_free_index(int unit, int *l2mc_id)
{
    int i, rv;

    rv = BCM_E_FULL;
    for (i = 0; i < L2MC_SIZE(unit); i++) {
        if (!L2MC_USED_ISSET(unit, i)) {
            L2MC_USED_SET(unit, i);
            *l2mc_id = i;
            rv = BCM_E_NONE;
            break;
        }
    }

    return rv;
}

/*
 * Function:
 *      _bcm_xgs_l2mc_id_alloc
 * Description:
 *      allocate the given l2mc entry (increment use count)
 */

STATIC int
_bcm_xgs_l2mc_id_alloc(int unit, int l2mc_id)
{
    L2MC_ID(unit, l2mc_id);

    L2MC_USED_SET(unit, l2mc_id);

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_xgs_l2mc_id_free
 * Description:
 *      Marks an l2mc index as free (decrements use count)
 */

STATIC int
_bcm_xgs_l2mc_id_free(int unit, int l2mc_id)
{
    L2MC_ID(unit, l2mc_id);

    L2MC_USED_CLR(unit, l2mc_id);

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_mcast_from_l2
 * Description:
 *      Fill in fields of bcm_mcast_addr_t from bcm_l2_addr_t
 * Parameters:
 *      unit    - (IN) device
 *      mcaddr  - (OUT) mcast addr struct
 *      l2addr  - (IN) l2 addr struct
 */

STATIC void
_bcm_mcast_from_l2(int unit,
                   bcm_mcast_addr_t *mcaddr,
                   bcm_l2_addr_t  *l2addr)
{
    int         rv;
    vlan_tab_entry_t vte;
    uint32      ubm;

    sal_memcpy(mcaddr->mac, l2addr->mac, sizeof(bcm_mac_t));
    mcaddr->vid = l2addr->vid;
    mcaddr->cos_dst = l2addr->cos_dst;
    mcaddr->l2mc_index = l2addr->l2mc_index;

    /* Need to fill in the untagged port bitmap from the VLAN table */
    SOC_PBMP_CLEAR(mcaddr->ubmp);
    rv = soc_mem_read(unit, VLAN_TABm, MEM_BLOCK_ANY,
                      mcaddr->vid, &vte);
    if (rv < 0) {
        return;
    }
    if (SOC_IS_TUCANA(unit)) {
        ubm = soc_VLAN_TABm_field32_get(unit, &vte, UT_BITMAP_M0f);
        SOC_PBMP_WORD_SET(mcaddr->ubmp, 0, ubm);
        ubm = soc_VLAN_TABm_field32_get(unit, &vte, UT_BITMAP_M1f);
        SOC_PBMP_WORD_SET(mcaddr->ubmp, 1, ubm);
    } else {
        ubm = soc_VLAN_TABm_field32_get(unit, &vte, UT_BITMAPf);
        SOC_PBMP_WORD_SET(mcaddr->ubmp, 0, ubm);
    }
}

/*
 * Function:
 *      _bcm_l2mcentry_pbmp_set
 * Description:
 *      Fill in pbmp fields of l2x_mc_entry_t from bcm_pbmp_t
 * Parameters:
 *      l2mc_entry - Pointer to bcm_mc_entry_t -- output
 *      mc_pbmp - Pointer to pbmp MC data -- input
 * Returns:
 *      Nothing.
 */

void
_bcm_l2mcentry_pbmp_set(int unit, l2x_mc_entry_t *l2mc_entry,
                         bcm_pbmp_t mc_pbmp)
{
    uint32 pbm;

    if (SOC_IS_TUCANA(unit)) {
        pbm = SOC_PBMP_WORD_GET(mc_pbmp, 0);
        soc_mem_field32_set(unit, L2MC_MCMEM(unit), l2mc_entry,
                            PORT_BITMAP_M0f, pbm);
        pbm = SOC_PBMP_WORD_GET(mc_pbmp, 1);
        soc_mem_field32_set(unit, L2MC_MCMEM(unit), l2mc_entry,
                            PORT_BITMAP_M1f, pbm);
    } else {
        pbm = SOC_PBMP_WORD_GET(mc_pbmp, 0);
        soc_mem_field32_set(unit, L2MC_MCMEM(unit), l2mc_entry,
                            PORT_BITMAPf, pbm);
    }
}

/*
 * Function:
 *      _bcm_l2mcentry_pbmp_get
 * Description:
 *      Retrieve pbmp from l2x_mc_entry_t
 * Parameters:
 *      l2mc_entry - Pointer to bcm_mc_entry_t -- input
 *      mc_pbmp - Pointer to pbmp MC data -- output
 * Returns:
 *      Nothing.
 */

void
_bcm_l2mcentry_pbmp_get(int unit, l2x_mc_entry_t *l2mc_entry,
                        bcm_pbmp_t *mc_pbmp)
{
    uint32      pbm;

    SOC_PBMP_CLEAR(*mc_pbmp);
    if (SOC_IS_TUCANA(unit)) {
        pbm = soc_mem_field32_get(unit, L2MC_MCMEM(unit), l2mc_entry,
                                  PORT_BITMAP_M0f);
        SOC_PBMP_WORD_SET(*mc_pbmp, 0, pbm);
        pbm = soc_mem_field32_get(unit, L2MC_MCMEM(unit), l2mc_entry,
                                  PORT_BITMAP_M1f);
        SOC_PBMP_WORD_SET(*mc_pbmp, 1, pbm);
    } else {
        pbm = soc_mem_field32_get(unit, L2MC_MCMEM(unit), l2mc_entry,
                                  PORT_BITMAPf);
        SOC_PBMP_WORD_SET(*mc_pbmp, 0, pbm);
    }
}

/*
 * Function:
 *      _bcm_mcast_from_l2mc
 * Description:
 *      Fill in fields of bcm_mcast_addr_t from l2x_mc_entry_t
 * Parameters:
 *      unit    - (IN) device
 *      mcaddr  - (OUT) bcm_mcast_addr_t
 *      l2mc    - (IN) l2mc table entry
 */

STATIC void
_bcm_mcast_from_l2mc(int unit,
                     bcm_mcast_addr_t *mcaddr,
                     l2x_mc_entry_t *l2mc)
{
    _bcm_l2mcentry_pbmp_get(unit, l2mc, &(mcaddr->pbmp));

    /*
     * We get the port bitmap here, but what about the untagged bitmap ?
     * always from VLAN table regardless of PFM value ?
     */
}

/*
 * Function:
 *      _bcm_mcast_to_l2mc
 * Description:
 *      Fill in fields of l2x_mc_entry_t from bcm_mcast_addr_t
 * Parameters:
 *      unit    - (IN) device
 *      l2mc    - (OUT) l2mc entry
 *      mcaddr  - (IN) bcm_mcast_addr_t
 */

STATIC void
_bcm_mcast_to_l2mc(int unit,
                   l2x_mc_entry_t *l2mc,
                   bcm_mcast_addr_t *mcaddr)
{
    sal_memset(l2mc, 0, sizeof(*l2mc));
    soc_mem_field32_set(unit, L2MC_MCMEM(unit), l2mc, VALIDf, 1);
    _bcm_l2mcentry_pbmp_set(unit, l2mc, mcaddr->pbmp);
}

/*
 * Function:
 *      _bcm_mcast_to_l2
 * Description:
 *      Fill in fields of bcm_l2_addr_t from bcm_mcast_addr_t
 * Parameters:
 *      unit -  (IN) device
 *      l2addr - (OUT) Pointer to bcm_l2_addr_t
 *      mcaddr - (IN) Pointer to bcm_mcast_addr_t
 */

STATIC void
_bcm_mcast_to_l2(int unit,
                 bcm_l2_addr_t *l2addr,
                 bcm_mcast_addr_t *mcaddr)
{
    sal_memset(l2addr, 0, sizeof(*l2addr));
    l2addr->flags = BCM_L2_STATIC | BCM_L2_MCAST | BCM_L2_REPLACE_DYNAMIC;
    l2addr->cos_dst = mcaddr->cos_dst;
    l2addr->vid = mcaddr->vid;
    sal_memcpy(l2addr->mac, mcaddr->mac, sizeof(bcm_mac_t));
    l2addr->l2mc_index = mcaddr->l2mc_index;
}

/*
 * Function:
 *      _bcm_xgs_mcast_create
 * Description:
 *      Create a muticast entry
 * Parameters:
 *      unit -  (IN) device
 *      mcaddr - (IN) Pointer to bcm_mcast_addr_t
 *      mcindex - (IN) mcast index to use (-1 lets routine choose)
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs_mcast_create(int unit, bcm_mcast_addr_t *mcaddr,
                      int mcindex)
{
    l2x_mc_entry_t      l2mc, rl2mc;
    bcm_l2_addr_t       l2addr, rl2addr;
    bcm_pbmp_t          pbm, rpbm;
    int                 rv;

    rv = bcm_esw_l2_addr_get(unit, mcaddr->mac, mcaddr->vid, &l2addr);
    if (rv >= 0) {
        if (!(l2addr.flags & BCM_L2_MCAST)) {
            return BCM_E_EXISTS;
        }
        if (mcindex >= 0 && mcindex == l2addr.l2mc_index) {
            return BCM_E_NONE;
        }

        BCM_IF_ERROR_RETURN(_bcm_xgs_l2mc_id_free(unit, l2addr.l2mc_index));
    }

    if (mcindex < 0) {
        BCM_IF_ERROR_RETURN(_bcm_xgs_l2mc_free_index(unit, &mcindex));
    } else {
        BCM_IF_ERROR_RETURN(_bcm_xgs_l2mc_id_alloc(unit, mcindex));
    }

    _bcm_mcast_to_l2mc(unit, &l2mc, mcaddr);
    _bcm_mcast_to_l2(unit, &l2addr, mcaddr);
    l2addr.l2mc_index = mcindex;

    if (SOC_MCAST_ADD_ALL_ROUTER_PORTS(unit)) {
        rv = bcm_esw_l2_addr_get(unit, (uint8 *)_soc_mac_all_routers,
                             mcaddr->vid, &rl2addr);
        if (rv >= 0) {
            SOC_IF_ERROR_RETURN
                (soc_mem_read(unit, L2MC_MCMEM(unit), MEM_BLOCK_ANY,
                              rl2addr.l2mc_index, &rl2mc));
            _bcm_l2mcentry_pbmp_get(unit, &rl2mc, &rpbm);
            _bcm_l2mcentry_pbmp_get(unit, &l2mc, &pbm);
            BCM_PBMP_OR(pbm, rpbm);
            _bcm_l2mcentry_pbmp_set(unit, &l2mc, pbm);
        }
    }

    /* add l2mc entry */
    SOC_IF_ERROR_RETURN
        (soc_mem_write(unit, L2MC_MCMEM(unit), MEM_BLOCK_ALL, mcindex, &l2mc));

    /* add l2 entry */
    rv = bcm_esw_l2_addr_add(unit, &l2addr);
    if (BCM_FAILURE(rv)) {
        _bcm_xgs_l2mc_id_free(unit, l2addr.l2mc_index);
        if (!L2MC_USED_ISSET(unit, l2addr.l2mc_index)) {
          soc_mem_write(unit, L2MC_MCMEM(unit), MEM_BLOCK_ALL,
                        l2addr.l2mc_index,
                        soc_mem_entry_null(unit, L2MC_MCMEM(unit)));
        }
    }
    return rv;
}

/*
 * Function:
 *      _bcm_xgs_mcast_port_add
 * Purpose:
 *      Add new ports to a multicast entry
 * Parameters:
 *      unit    - device
 *      mcaddr  - mcast addr struct with new ports
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_xgs_mcast_port_add(int unit, bcm_mcast_addr_t *mcaddr)
{
    bcm_l2_addr_t       l2addr;
    l2x_mc_entry_t      l2mc_entry;
    bcm_pbmp_t          pbmp;

    BCM_IF_ERROR_RETURN
        (bcm_esw_l2_addr_get(unit, mcaddr->mac, mcaddr->vid, &l2addr));

    if (!(l2addr.flags & BCM_L2_MCAST)) {
        return BCM_E_NOT_FOUND;
    }

    SOC_IF_ERROR_RETURN
        (soc_mem_read(unit, L2MC_MCMEM(unit), MEM_BLOCK_ANY,
                      l2addr.l2mc_index, &l2mc_entry));

    _bcm_l2mcentry_pbmp_get(unit, &l2mc_entry, &pbmp);
    BCM_PBMP_OR(pbmp, mcaddr->pbmp);
    _bcm_l2mcentry_pbmp_set(unit, &l2mc_entry, pbmp);

    SOC_IF_ERROR_RETURN
        (soc_mem_write(unit, L2MC_MCMEM(unit), MEM_BLOCK_ALL,
                       l2addr.l2mc_index, &l2mc_entry));

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_xgs_mcast_port_delete
 * Purpose:
 *      Remove ports from a multicast entry
 * Parameters:
 *      unit    - device
 *      mcaddr  - mcast addr struct with ports to delete
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_xgs_mcast_port_delete(int unit,
                           bcm_mcast_addr_t *mcaddr)
{
    bcm_l2_addr_t       l2addr;
    l2x_mc_entry_t      l2mc_entry;
    bcm_pbmp_t          pbmp;

    BCM_IF_ERROR_RETURN
        (bcm_esw_l2_addr_get(unit, mcaddr->mac, mcaddr->vid, &l2addr));

    if (!(l2addr.flags & BCM_L2_MCAST)) {
        return BCM_E_NOT_FOUND;
    }

    SOC_IF_ERROR_RETURN
        (soc_mem_read(unit, L2MC_MCMEM(unit), MEM_BLOCK_ANY,
                      l2addr.l2mc_index, &l2mc_entry));

    _bcm_l2mcentry_pbmp_get(unit, &l2mc_entry, &pbmp);
    BCM_PBMP_REMOVE(pbmp, mcaddr->pbmp);
    _bcm_l2mcentry_pbmp_set(unit, &l2mc_entry, pbmp);

    SOC_IF_ERROR_RETURN
        (soc_mem_write(unit, L2MC_MCMEM(unit), MEM_BLOCK_ALL,
                       l2addr.l2mc_index, &l2mc_entry));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_draco_mcast_port_remove
 * Purpose:
 *      Remove ports from existing MC entry.
 */

int
bcm_draco_mcast_port_remove(int unit, bcm_mcast_addr_t *mcaddr)
{
    int rv;

    L2MC_INIT(unit);
    L2MC_LOCK(unit);
    rv = _bcm_xgs_mcast_port_delete(unit, mcaddr);
    L2MC_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_draco_mcast_port_add
 * Purpose:
 *      Add ports to existing MC entry.
 */
int
bcm_draco_mcast_port_add(int unit, bcm_mcast_addr_t *mcaddr)
{
    int rv;

    L2MC_INIT(unit);
    L2MC_LOCK(unit);
    rv = _bcm_xgs_mcast_port_add(unit, mcaddr);
    L2MC_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_draco_mcast_addr_add_w_l2mcindex
 * Purpose:
 *      Add a new MC entry to the L2 and L2MC tables, with given MC index.
 */

int
bcm_draco_mcast_addr_add_w_l2mcindex(int unit, bcm_mcast_addr_t *mcaddr)
{
    int rv;

    L2MC_INIT(unit);
    L2MC_LOCK(unit);
    rv = _bcm_xgs_mcast_create(unit, mcaddr, mcaddr->l2mc_index);
    L2MC_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_draco_mcast_addr_add
 * Purpose:
 *      Add new L2 multicast entry
 */

int
bcm_draco_mcast_addr_add(int unit, bcm_mcast_addr_t *mcaddr)
{
    int rv;

    L2MC_INIT(unit);
    L2MC_LOCK(unit);
    /*    coverity[negative_returns : FALSE]    */
    rv = _bcm_xgs_mcast_create(unit, mcaddr, -1);
    L2MC_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_draco_mcast_addr_remove
 * Purpose:
 *      Draco implementation of multiplexed routine.
 */

int
bcm_draco_mcast_addr_remove(int unit, sal_mac_addr_t mac, bcm_vlan_t vid)
{
    int                 rv;
    bcm_l2_addr_t       l2addr;

    L2MC_INIT(unit);
    L2MC_LOCK(unit);

    rv = bcm_esw_l2_addr_get(unit, mac, vid, &l2addr);
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

    _bcm_xgs_l2mc_id_free(unit, l2addr.l2mc_index);

    if (!L2MC_USED_ISSET(unit, l2addr.l2mc_index)) {
        rv = soc_mem_write(unit, L2MC_MCMEM(unit), MEM_BLOCK_ALL,
                           l2addr.l2mc_index,
                           soc_mem_entry_null(unit, L2MC_MCMEM(unit)));
    }

    L2MC_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_draco_mcast_addr_remove_w_l2mcindex
 */

int
bcm_draco_mcast_addr_remove_w_l2mcindex(int unit,
                                        bcm_mcast_addr_t *mcaddr)
{
    return bcm_draco_mcast_addr_remove(unit, mcaddr->mac, mcaddr->vid);
}

/*
 * Function:
 *      bcm_draco_mcast_port_get
 * Purpose:
 *      Draco implementation of multiplexed routine.
 */

int
bcm_draco_mcast_port_get(int unit,
                         sal_mac_addr_t mac, bcm_vlan_t vid,
                         bcm_mcast_addr_t *mcaddr)
{
    l2x_mc_entry_t      l2mc_entry;
    bcm_l2_addr_t       l2addr;

    L2MC_INIT(unit);

    BCM_IF_ERROR_RETURN(bcm_esw_l2_addr_get(unit, mac, vid, &l2addr));
    SOC_IF_ERROR_RETURN
        (soc_mem_read(unit, L2MC_MCMEM(unit), MEM_BLOCK_ANY,
                      l2addr.l2mc_index, &l2mc_entry));

    _bcm_mcast_from_l2(unit, mcaddr, &l2addr);
    _bcm_mcast_from_l2mc(unit, mcaddr, &l2mc_entry);

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_draco_mcast_detach
 * Purpose:
 *      De-initialize multicast api components
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_draco_mcast_detach(int unit)
{
    if (L2MC_USED(unit) != NULL) {
        sal_free(L2MC_USED(unit));
        L2MC_USED(unit) = NULL;
    }

    L2MC_L2MEM(unit) = INVALIDm;
    L2MC_MCMEM(unit) = INVALIDm;
    L2MC_SIZE(unit) = 0;

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_draco_mcast_init
 * Purpose:
 *      Initialize multicast api components
 * Returns:
 *      BCM_E_XXX on error
 *      number of mcast entries supported on success
 */

int
bcm_draco_mcast_init(int unit)
{
    if (L2MC_USED(unit) != NULL) {
        BCM_IF_ERROR_RETURN(_bcm_draco_mcast_detach(unit));
    }

    L2MC_L2MEM(unit) = L2Xm;
    L2MC_MCMEM(unit) = L2X_MCm;
    L2MC_SIZE(unit) = soc_mem_index_count(unit, L2MC_MCMEM(unit));
    L2MC_USED(unit) = sal_alloc(sizeof(int) * L2MC_SIZE(unit), "L2MC");
    if (L2MC_USED(unit) == NULL) {
        return BCM_E_MEMORY;
    }

    sal_memset(L2MC_USED(unit), 0, sizeof(int) * L2MC_SIZE(unit));

    SOC_IF_ERROR_RETURN
        (soc_mem_clear(unit, L2MC_MCMEM(unit), MEM_BLOCK_ALL, FALSE));

    /* Delete all multicast entries from L2 */
    BCM_IF_ERROR_RETURN
        (bcm_draco_l2_addr_delete_mcast(unit, BCM_L2_DELETE_STATIC));

    return L2MC_SIZE(unit);
}
