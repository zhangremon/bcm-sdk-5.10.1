/*
 * $Id: ipmc.c 1.36.42.1 Broadcom SDK $
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
 * IP Multicast API
 */

#include <soc/defs.h>

#ifdef INCLUDE_L3

#include <soc/drv.h>
#include <bcm/stack.h>
#include <bcm/vlan.h>
#include <bcm/trunk.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/fe2000.h>


#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/debug.h>
#include <bcm/l2.h>
#include <bcm/l3.h>
#include <bcm/ipmc.h>
#include <bcm/mpls.h>
#include <bcm/tunnel.h>
#include <bcm/pkt.h>
#include <shared/gport.h>

#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/fe2000/allocator.h>
#include <bcm_int/sbx/fe2000/l3.h>
#include <bcm_int/sbx/ipmc.h>

#include <appl/diag/system.h>

#include <bcm_int/sbx/fe2000/g2p3.h>
#include <soc/sbx/g2p3/g2p3.h>


STATIC int
_bcm_fe2000_find_ipmc_index_port_in_ete(_fe2k_l3_ete_t         *m_ete,
                                        int                     ipmc_index,
                                        bcm_port_t              port,
                                        int                    *found_slot);
STATIC int
_bcm_fe2000_ipmc_flow_delete(int unit,
                             _fe2k_l3_fe_instance_t *l3_fe,
                             bcm_ipmc_addr_t *data,
                             int  remove_egress);

#define V4MC_STR_SEL_RULE 0

/*
 * Flags that are valid on sbx
 *   BCM_IPMC_USE_IPMC_INDEX          Use ipmc_index to add 
 *   BCM_IPMC_SOURCE_PORT_NOCHECK     no src port check (XGS)
 *   BCM_IPMC_REPLACE                 update existing entry 
 *   BCM_IPMC_IP6                     IPv6 entry (vs IPv4)
 */

#define _FE2K_BCM_IPMC_L3_SUPP_FLAGS (BCM_L3_WITH_ID  | \
                                      BCM_L3_TGID     | \
                                      BCM_L3_UNTAG)

#define _FE2K_BCM_IPMC_SUPP_FLAGS (BCM_IPMC_USE_IPMC_INDEX      |       \
                                   BCM_IPMC_SOURCE_PORT_NOCHECK |       \
                                   BCM_IPMC_COPYTOCPU           |       \
                                   BCM_IPMC_IP6                 |       \
                                   BCM_IPMC_REPLACE)

/*
 * Debug Output Macros
 * #define DBG_LOG_MACROS 1
 */

#ifdef DBG_LOG_MACROS
#define IPMC_DEBUG(flags, stuff) printf stuff
#else
#define IPMC_DEBUG(flags, stuff)  BCM_DEBUG(flags | BCM_DBG_IPMC, stuff)
#endif

#define IPMC_NORMAL(stuff)        IPMC_DEBUG(BCM_DBG_NORMAL, stuff)
#define IPMC_WARN(stuff)          IPMC_DEBUG(BCM_DBG_WARN, stuff)
#define IPMC_ERR(stuff)           IPMC_DEBUG(BCM_DBG_ERR, stuff)
#define IPMC_VERB(stuff)          IPMC_DEBUG(BCM_DBG_VERBOSE, stuff)
/* define IPMC_VVERB(stuff)         IPMC_DEBUG(BCM_DBG_VVERBOSE, stuff) */
#define IPMC_VVERB(stuff)         

#ifdef IPMC_DEVTEST
int skip_egress_intf_set = 0;

void
_bcm_fe2000_ipmc_dump_one_egress(_fe2k_l3_fe_instance_t *l3_fe,
                                 _fe2k_l3_intf_t         *l3_intf,
                                 _fe2k_l3_ete_t          *m_ete)
{
    int                      ii;

    IPMC_VERB(("unit %d: [%s] intf(0x%x) ete(0x%x,%s,%d) ohi(0x%x)\n",
               l3_fe->fe_unit, FUNCTION_NAME(),
               l3_intf->if_info.l3a_intf_id,
               m_ete->l3_ete_hw_idx.ete_idx,
               (m_ete->l3_ete_kvidop == _FE2K_L3_ETE_VIDOP__NOP)?"untag":"tag",
               m_ete->l3_ete_kttl, m_ete->l3_ohi.ohi));

    for (ii = 0; ii < m_ete->l3_inuse_ue; ii++) {
        IPMC_VERB(("<0x%x,%d> ",
                   m_ete->l3_ete_u.vc_ipmc[ii].ipmc_index,
                   m_ete->l3_ete_u.vc_ipmc[ii].port));
    }
    IPMC_VERB(("\n"));
    return;
}

void
_bcm_fe2000_ipmc_dump_all_egress(_fe2k_l3_fe_instance_t *l3_fe,
                                 int                     ipmc_index,
                                 bcm_port_t              port)
{
    _fe2k_l3_intf_t         *l3_intf;
    _fe2k_l3_ete_t          *l3_ete;
    int                      ignore;
    
    _FE2K_ALL_L3INTF(l3_fe, l3_intf) {
        _FE2K_ALL_L3ETE_ON_INTF(l3_intf, l3_ete) {
            
            /**
             * We add to the first m_ete that we find.
             */
            if (l3_ete->l3_ete_ktype == _FE2K_L3_ETE__MCAST_IP) {
                
                if (ipmc_index &&
                    _bcm_fe2000_find_ipmc_index_port_in_ete(l3_ete,
                                                            ipmc_index,
                                                            port,
                                                            &ignore) == BCM_E_NONE) {
                    _bcm_fe2000_ipmc_dump_one_egress(l3_fe,
                                                     l3_intf,
                                                     l3_ete);
                } else if (!ipmc_index) {
                    _bcm_fe2000_ipmc_dump_one_egress(l3_fe,
                                                     l3_intf,
                                                     l3_ete);
                }
            }
            
        } _FE2K_ALL_L3ETE_ON_INTF_END(l3_intf, l3_ete);
        
    } _FE2K_ALL_L3INTF_END(l3_fe, l3_intf);
    
    return;
}
#endif /* IPMC_DEVTEST */

/*
 * Function:
 *      _bcm_fe2000_alloc_ipmc_route
 * Description:
 *      allocate a new ipmc route
 * Parameters:
 *      l3_fe   - L3 fe instance pointer
 *      ipmc_rt - return ptr to ipmc route
 * Returns:
 *      BCM_E_NONE   - on success
 *      BCM_E_MEMORY - on failure
 * Assumption:
 */

int
_bcm_fe2000_alloc_ipmc_route(_fe2k_l3_fe_instance_t   *l3_fe,
                             _fe2k_ipmc_route_t      **ipmc_rt)
{

    *ipmc_rt = (_fe2k_ipmc_route_t *)
        sal_alloc(sizeof(_fe2k_ipmc_route_t),
                  "IPMC-rt");
    if (*ipmc_rt == NULL) {
        return BCM_E_MEMORY;
    }

    sal_memset(*ipmc_rt, 0, sizeof(_fe2k_ipmc_route_t));

    DQ_INIT(&(*ipmc_rt)->id_link);

    IPMC_VERB(("unit %d: [%s] allocated(0x%x) for ipmc-rt\n",
               l3_fe->fe_unit, FUNCTION_NAME(),
               (unsigned int)*ipmc_rt));
    
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_fe2000_free_ipmc_route
 * Description:
 *      Free ipmc route
 * Parameters:
 *      l3_fe   - L3 fe instance pointer
 *      ipmc_rt - ipmc route ptr to ptr
 * Returns:
 * Assumption:
 */

void
_bcm_fe2000_free_ipmc_route(_fe2k_l3_fe_instance_t *l3_fe,
                            _fe2k_ipmc_route_t **ipmc_rt)
{
    _fe2k_ipmc_route_t *rt = *ipmc_rt;
    
    if (!DQ_EMPTY(&rt->id_link)) {
        DQ_REMOVE(&rt->id_link);
    }

    IPMC_VERB(("unit %d: [%s] freed(0x%x) for ipmc-rt\n",
               l3_fe->fe_unit, FUNCTION_NAME(),
               (unsigned int)rt));
    
    sal_free(rt);
    *ipmc_rt = NULL;
}

/*
 * Function:
 *      _bcm_fe2000_insert_ipmc_route
 * Description:
 *      insert the route into the ipmc rt hash
 * Parameters:
 *      l3_fe      - L3 fe instance
 *      ipmc_index - ipmc index for this route
 *      ipmc_rt    - ipmc route 
 * Returns:
 * Notes:
 * Assumption:
 */

void
_bcm_fe2000_insert_ipmc_route(_fe2k_l3_fe_instance_t *l3_fe,
                              int                     ipmc_index,
                              _fe2k_ipmc_route_t     *ipmc_rt)
{
    int   hidx;
    
    hidx = _FE2K_IPMC_HASH_IDX(ipmc_index);

    ipmc_rt->ipmc_index = ipmc_index;

    DQ_INSERT_HEAD(&l3_fe->fe_ipmc_by_id[hidx],
                   &ipmc_rt->id_link);

    return;
}


/*
 * Function:
 *      _bcm_fe2000_find_ipmc_route
 * Description:
 *      find route from ipmc rt hash
 * Parameters:
 *      unit       - fe2k unit
 *      l3_fe      - L3 fe instance
 *      ipmc_index - ipmc index
 *      ipmc_rt    - ipmc route 
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_NOT_FOUND - if not found
 * Notes:
 * Assumption:
 */

int
_bcm_fe2000_find_ipmc_route(int                      unit,
                            _fe2k_l3_fe_instance_t  *l3_fe,
                            int                      ipmc_index,
                            _fe2k_ipmc_route_t     **ipmc_rt)
{
    int                  hidx;
    _fe2k_ipmc_route_t  *rt = NULL;

    hidx = _FE2K_IPMC_HASH_IDX(ipmc_index);
    
    _FE2K_IPMC_RT_PER_BKT(l3_fe, hidx, rt) {

        if (rt->ipmc_index == ipmc_index) {
            IPMC_VERB(("unit %d: [%s] found ipmc-rt with ipmc-index(0x%x)\n",
                       unit, FUNCTION_NAME(), ipmc_index));
            *ipmc_rt = rt;
            return BCM_E_NONE;
        }
        
    } _FE2K_IPMC_RT_PER_BKT_END(l3_fe, hidx, rt);

    IPMC_VERB(("unit %d: [%s] ipmc-rt with ipmc-index(0x%x) not-found\n",
               unit, FUNCTION_NAME(), ipmc_index));
    
    return BCM_E_NOT_FOUND;
}



/*
 * Function:
 *      _bcm_fe2000_fill_data_key_from_index
 * Description:
 *      given ipmc_index, we fill the s,g info
 * Parameters:
 *      unit       - FE unit
 *      l3_fe      - L3 fe instance
 *      fe         - SB fe ptr
 *      data       - new info for ipmc route
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_XXX  - on failure
 * Notes:
 * Assumption:
 */

STATIC int
_bcm_fe2000_fill_data_key_from_index(int                    unit,
                                     _fe2k_l3_fe_instance_t *l3_fe,
                                     int                     ipmc_index,
                                     bcm_ipmc_addr_t        *data)
{
    _fe2k_ipmc_route_t     *ipmc_rt;
    int                     status;

    status = _bcm_fe2000_find_ipmc_route(unit,
                                         l3_fe,
                                         ipmc_index,
                                         &ipmc_rt);
    if (status != BCM_E_NONE) {
        return status;
    }

    /* with index overwrites the <s,g,vid> from user */
    bcm_ipmc_addr_t_init(data);
    data->s_ip_addr = ipmc_rt->ipmc_user_info.s_ip_addr;
    data->mc_ip_addr = ipmc_rt->ipmc_user_info.mc_ip_addr;
    data->vid = ipmc_rt->ipmc_user_info.vid;
    sal_memcpy(data->s_ip6_addr, ipmc_rt->ipmc_user_info.s_ip6_addr, sizeof(bcm_ip6_t));
    sal_memcpy(data->mc_ip6_addr, ipmc_rt->ipmc_user_info.mc_ip6_addr, sizeof(bcm_ip6_t));
    data->mc_ip_addr = ipmc_rt->ipmc_user_info.mc_ip_addr;

    data->flags      = ipmc_rt->ipmc_user_info.flags | BCM_IPMC_USE_IPMC_INDEX;
    data->ipmc_index = ipmc_index;
    
    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_fe2000_add_ipmc_index_port_to_ete
 * Description:
 *      given a sw ete add the <ipmc_index, port> into the usage field
 * Parameters:
 *      unit       - FE unit
 *      l3_fe      - L3 fe instance
 *      l3_intf    - L3 intf ptr
 *      m_ete      - ipmc sw ete
 *      ipmc_index - ipmc index
 *      port       - port
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_XXX  - on failure
 * Notes:
 * Assumption:
 */

STATIC int
_bcm_fe2000_add_ipmc_index_port_to_ete(int                     unit,
                                       _fe2k_l3_fe_instance_t *l3_fe,
                                       _fe2k_l3_intf_t        *l3_intf,
                                       _fe2k_l3_ete_t         *m_ete,
                                       int                     ipmc_index,
                                       bcm_port_t              port)
{
    int                ii, status;

    /**
     * First make sure that the ipmc_index does not
     * already exists
     */
    for (ii = 0; ii < m_ete->l3_inuse_ue; ii++) {
        if ((m_ete->u.l3_ipmc[ii].ipmc_index == ipmc_index) &&
            (m_ete->u.l3_ipmc[ii].port == port)) {
            return BCM_E_EXISTS;
        }
    }

    /* create space for the new entry */
    if ((m_ete->l3_inuse_ue > 0) &&
        (m_ete->l3_inuse_ue == m_ete->l3_alloced_ue)) {
        status = _bcm_fe2000_expand_ete_users(m_ete);
        if (status != BCM_E_NONE) {
            IPMC_ERR(("unit %d: [%s] error(%s) expanding ete-users for 0x%x\n",
                      l3_fe->fe_unit, FUNCTION_NAME(),
                      bcm_errmsg(status), m_ete->l3_ete_hw_idx.ete_idx));
            return status;
        }
    }
    
    /* copy the last into this slot */
    m_ete->u.l3_ipmc[m_ete->l3_inuse_ue].ipmc_index = ipmc_index;
    m_ete->u.l3_ipmc[m_ete->l3_inuse_ue].port       = port;
    m_ete->l3_inuse_ue++;
    
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_fe2000_find_ipmc_index_port_in_ete
 * Description:
 *      given a sw ete find the <ipmc_index, port> in the usage field
 * Parameters:
 *      unit       - FE unit
 *      l3_fe      - L3 fe instance
 *      l3_intf    - L3 intf ptr
 *      m_ete      - ipmc sw ete
 *      ipmc_index - ipmc index
 *      port       - port
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_XXX  - on failure
 * Notes:
 * Assumption:
 */

STATIC int
_bcm_fe2000_find_ipmc_index_port_in_ete(_fe2k_l3_ete_t         *m_ete,
                                        int                     ipmc_index,
                                        bcm_port_t              port,
                                        int                    *found_slot)
{
    int                ii;

    *found_slot = -1;
    for (ii = 0; ii < m_ete->l3_alloced_ue; ii++) {
        if ((m_ete->u.l3_ipmc[ii].ipmc_index == ipmc_index) &&
            (m_ete->u.l3_ipmc[ii].port == port)) {
            *found_slot = ii;
            return BCM_E_NONE;
        }
    }

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      _bcm_fe2000_remove_ipmc_index_port_from_ete
 * Description:
 *      given a sw ete remove <ipmc_index, port> from the usage field
 * Parameters:
 *      unit       - FE unit
 *      l3_fe      - L3 fe instance
 *      l3_intf    - L3 intf ptr
 *      p_m_ete    - ipmc sw ete
 *      ipmc_index - ipmc index
 *      port       - port
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_XXX  - on failure
 * Notes:
 * Assumption:
 */

STATIC int
_bcm_fe2000_remove_ipmc_index_port_from_ete(int                      unit,
                                            _fe2k_l3_fe_instance_t  *l3_fe,
                                            _fe2k_l3_intf_t         *l3_intf,
                                            _fe2k_l3_ete_t         **p_m_ete,
                                            int                      ipmc_index,
                                            bcm_port_t               port)
{
    int                found_slot, status;
    _fe2k_l3_ete_t    *m_ete;

    m_ete = *p_m_ete;

    status = _bcm_fe2000_find_ipmc_index_port_in_ete(m_ete,
                                                     ipmc_index,
                                                     port,
                                                     &found_slot);
    if (status != BCM_E_NONE) {
        return status;
    }
    
    m_ete->l3_inuse_ue--;
    if (m_ete->l3_inuse_ue == 0) {
        status = 
            _bcm_fe2000_l3_sw_ete_destroy(unit,
                                          l3_fe,
                                          l3_intf,
                                          p_m_ete);
        if (status != BCM_E_NONE) {
            IPMC_ERR(("unit %d: [%s] error(%s) destroy-m-ete on l3a-intf(0x%x)\n",
                      unit, FUNCTION_NAME(),
                      bcm_errmsg(status),
                      l3_intf->if_info.l3a_intf_id));
            return status;
        }
    } else {
        /* copy the last into this slot. */            
        m_ete->u.l3_ipmc[found_slot] = m_ete->u.l3_ipmc[m_ete->l3_inuse_ue];
    }
    
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_if_compare
 * Description:
 *      compare routine to sort if-array
 * Parameters:
 *      a, b       - pointer of type bcm_if_t
 * Returns:
 *      -1, 1, 0 based on compare
 * Notes:
 * Assumption:
 */

STATIC int
_bcm_if_compare(void *a, void *b)
{
    bcm_if_t *d1 = a;
    bcm_if_t *d2 = b;

    return (*d1 < *d2 ? -1 :
            *d1 > *d2 ?  1 : 0);
}

/*
 * Function:
 *      _bcm_fe2000_ipmc_find_intf_in_array
 * Description:
 *      find if intf is present in the given if-array
 * Parameters:
 *      l3a_intf_id   - search key
 *      if_count      - array max
 *      if_sort_array - sorted array of bcm_if_t
 *      found_index   - if found the array index
 * Returns:
 *      BCM_E_XXX
 * Assumption:
 */

STATIC int
_bcm_fe2000_ipmc_find_intf_in_array(bcm_if_t  l3a_intf_id,
                                    int       if_count,
                                    bcm_if_t *if_sort_array,
                                    int      *found_index)
{
    int    ii;

    for (ii = 0; ii < if_count; ii++) {
        if (l3a_intf_id < if_sort_array[ii]) {
            return BCM_E_NOT_FOUND;
        }
        if (l3a_intf_id == if_sort_array[ii]) {
            *found_index = ii;
            return BCM_E_NONE;
        }
    }

    return BCM_E_NOT_FOUND;
}


/*
 * Function:
 *      _bcm_fe2000_ipmc_egress_intf_params_check
 * Description:
 *      egress intf parameters are checked for being
 *      valid
 * Parameters:
 *      unit       - FE unit
 *      ipmc_index - ipmc index
 *      port       - port
 *      bcm_intf   - bcm l3 intf ptr
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_XXX  - on failure
 * Assumption:
 */
STATIC int
_bcm_fe2000_ipmc_egress_intf_params_check(int            unit,
                                          int            ipmc_index,
                                          bcm_port_t     port,
                                          bcm_l3_intf_t *bcm_intf)
{
    int  int_id;

    if (!ipmc_index) {
        return BCM_E_PARAM;
    }
    
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PARAM;
    }

    if (bcm_intf == NULL) {
        return BCM_E_PARAM;
    }

    /* valid flags for ipmc calls, check */
    if (bcm_intf->l3a_flags & ~_FE2K_BCM_IPMC_L3_SUPP_FLAGS) {
        return BCM_E_PARAM;
    }

    if (bcm_intf->l3a_flags & BCM_L3_WITH_ID) {
        int_id = _FE2K_IFID_FROM_USER_HANDLE(bcm_intf->l3a_intf_id);
        if (!_FE2K_IFID_VALID_RANGE(int_id)) {
            return BCM_E_PARAM;
        }
    } else {
        if (!BCM_VLAN_VALID(bcm_intf->l3a_vid)) {
            return BCM_E_PARAM;
        }
        if (BCM_MAC_IS_MCAST(bcm_intf->l3a_mac_addr) || 
            BCM_MAC_IS_ZERO(bcm_intf->l3a_mac_addr)) {
            return BCM_E_PARAM;
        }
    }

    /**
     * no separate flag to say ttl is to be used, hence
     */
    if (bcm_intf->l3a_ttl &&
        !_BCM_TTL_VALID(bcm_intf->l3a_ttl)) {
        return BCM_E_PARAM;
    }
    
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_fe2000_ipmc_egress_intf_add
 * Description:
 *      add egress intf, port to ipmc index
 * Parameters:
 *      unit       - FE unit
 *      l3_fe      - L3 fe ptr
 *      fe         - SB fe
 *      ipmc_index - ipmc index
 *      port       - port
 *      bcm_intf   - bcm l3 intf ptr
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_XXX  - on failure
 * Assumption:
 */

STATIC int
_bcm_fe2000_ipmc_egress_intf_add(int                     unit,
                                 _fe2k_l3_fe_instance_t *l3_fe,
                                 int                     ipmc_index,
                                 bcm_port_t              port, 
                                 bcm_l3_intf_t          *bcm_intf)
{
    _fe2k_l3_intf_t         *l3_intf;
    _fe2k_l3_ete_t          *m_ete;
    _fe2k_l3_ete_vidop_t     vidop;
    _fe2k_l3_ete_key_t       ete_key;
    int                      status;
    int                      ignore;
    
    if (bcm_intf->l3a_flags & BCM_L3_WITH_ID) {
        status = _bcm_fe2000_l3_find_intf_by_ifid(l3_fe,
                                                  bcm_intf->l3a_intf_id,
                                                  &l3_intf);
        if (status != BCM_E_NONE) {
            IPMC_ERR(("unit %d: [%s] ipmc-index(0x%x) port(%d) unable to find l3-interface with id(%d)\n",
                      unit, FUNCTION_NAME(),
                      ipmc_index, port, bcm_intf->l3a_intf_id));
            return status;
        }
    } else {
        status = _bcm_fe2000_l3_find_intf_by_vid_mac(l3_fe,
                                                     bcm_intf,
                                                     &l3_intf);
        if (status != BCM_E_NONE) {
            IPMC_ERR(("unit %d: [%s] ipmc_index(%d) port(%d) unable to find l3-interface with "
                      "vid(%d) mac(%02x%02x:%02x%02x:%02x%02x)\n",
                      unit, FUNCTION_NAME(),
                      ipmc_index, port, bcm_intf->l3a_vid,
                      bcm_intf->l3a_mac_addr[0], bcm_intf->l3a_mac_addr[1],
                      bcm_intf->l3a_mac_addr[2], bcm_intf->l3a_mac_addr[3],
                      bcm_intf->l3a_mac_addr[4], bcm_intf->l3a_mac_addr[5]));
            return status;
        }
    }
    
    /**
     * Since there is no per port flag, the bcm_intf
     * flags denotes if the packet out this port needs
     * to be tagged or untagged.
     */
    if (bcm_intf->l3a_flags & BCM_L3_UNTAG) {
        vidop = _FE2K_L3_ETE_VIDOP__NOP;
    } else {    
        vidop = _FE2K_L3_ETE_VIDOP__ADD;
    }

    sal_memset(&ete_key, 0, sizeof(ete_key));
    ete_key.l3_ete_hk.type    = _FE2K_L3_ETE__MCAST_IP;
    ete_key.l3_ete_vidop      = vidop;
    ete_key.l3_ete_ttl        = (bcm_intf->l3a_ttl)?bcm_intf->l3a_ttl:1;

    status = _bcm_fe2000_l3_sw_ete_create(unit,
                                          l3_fe,
                                          l3_intf,
                                          &ete_key,
                                          0, /* do not use cookie */
                                          0, /* flags */
                                          port,
                                          &m_ete);
    if ((status != BCM_E_NONE) && (status != BCM_E_EXISTS)) {
        IPMC_ERR(("unit %d: [%s] ipmc-index(0x%x) port(%d), error(%s) creating ipmc-ete on l3-intf(%d)\n",
                  unit, FUNCTION_NAME(),
                  ipmc_index, port, bcm_errmsg(status),
                  l3_intf->if_info.l3a_intf_id));
        return status;
    }

    status = _bcm_fe2000_find_ipmc_index_port_in_ete(m_ete,
                                                     ipmc_index,
                                                     port,
                                                     &ignore);
    if (status != BCM_E_NOT_FOUND) {
        IPMC_ERR(("unit %d: [%s] error(%s) found existing <0x%x,%d> already in ete(0x%x)\n",
                  l3_fe->fe_unit, FUNCTION_NAME(),
                  bcm_errmsg(status), ipmc_index,
                  port, m_ete->l3_ete_hw_idx.ete_idx));
        return (status == BCM_E_NONE)?BCM_E_EXISTS:status;
    }
    
    status = _bcm_fe2000_add_ipmc_index_port_to_ete(unit,
                                                    l3_fe,
                                                    l3_intf,
                                                    m_ete,
                                                    ipmc_index,
                                                    port);
    if (status != BCM_E_NONE) {
        /* XXX: TBD: Roll back if newly allocated entry. */
        IPMC_ERR(("unit %d: [%s] error(%s) adding <0x%x,%d> to ete-users for 0x%x\n",
                  l3_fe->fe_unit, FUNCTION_NAME(),
                  bcm_errmsg(status),
                  ipmc_index, port,
                  m_ete->l3_ete_hw_idx.ete_idx));
        return status;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_fe2000_ipmc_egress_intf_delete
 * Description:
 *      delete egress intf, port for given ipmc index
 * Parameters:
 *      unit       - FE unit
 *      l3_fe      - L3 fe ptr
 *      ipmc_index - ipmc index
 *      port       - port
 *      bcm_intf   - bcm l3 intf ptr
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_XXX  - on failure
 * Assumption:
 */

STATIC int
_bcm_fe2000_ipmc_egress_intf_delete(int                     unit,
                                    _fe2k_l3_fe_instance_t *l3_fe,
                                    int                     ipmc_index,
                                    bcm_port_t              port, 
                                    bcm_l3_intf_t          *bcm_intf)
{
    _fe2k_l3_intf_t         *l3_intf;
    _fe2k_l3_ete_t          *m_ete;
    int                      status;
    int                      vidop;
    uint32                   ete_idx;
    _fe2k_l3_ete_key_t       ete_key;
    
    if (bcm_intf->l3a_flags & BCM_L3_WITH_ID) {
        status = _bcm_fe2000_l3_find_intf_by_ifid(l3_fe,
                                                  bcm_intf->l3a_intf_id,
                                                  &l3_intf);
        if (status != BCM_E_NONE) {
            IPMC_ERR(("unit %d: [%s] ipmc-index(0x%x) port(%d) unable to find l3-interface with id(%d)\n",
                      unit, FUNCTION_NAME(),
                      ipmc_index, port, bcm_intf->l3a_intf_id));
            return status;
        }
    } else {
        status = _bcm_fe2000_l3_find_intf_by_vid_mac(l3_fe,
                                                     bcm_intf,
                                                     &l3_intf);
        if (status != BCM_E_NONE) {
            IPMC_ERR(("unit %d: [%s] ipmc_index(%d) port(%d) unable to find l3-interface with "
                      "vid(%d) mac(%02x%02x:%02x%02x:%02x%02x)\n",
                      unit, FUNCTION_NAME(),
                      ipmc_index, port, bcm_intf->l3a_vid,
                      bcm_intf->l3a_mac_addr[0], bcm_intf->l3a_mac_addr[1],
                      bcm_intf->l3a_mac_addr[2], bcm_intf->l3a_mac_addr[3],
                      bcm_intf->l3a_mac_addr[4], bcm_intf->l3a_mac_addr[5]));
            return status;
        }
    }

    /**
     * Since there is no per port flag, the bcm_intf
     * flags denotes if the packet out this port needs
     * to be tagged or untagged.
     */
    if (bcm_intf->l3a_flags & BCM_L3_UNTAG) {
        vidop = _FE2K_L3_ETE_VIDOP__NOP;
    } else {    
        vidop = _FE2K_L3_ETE_VIDOP__ADD;
    }

    sal_memset(&ete_key, 0, sizeof(ete_key));
    ete_key.l3_ete_hk.type    = _FE2K_L3_ETE__MCAST_IP;
    ete_key.l3_ete_vidop      = vidop;
    ete_key.l3_ete_ttl        = (bcm_intf->l3a_ttl)?bcm_intf->l3a_ttl:1;

    status = _bcm_fe2000_l3_sw_ete_find(unit,
                                        l3_fe,
                                        l3_intf,
                                        &ete_key,
                                        &m_ete);
    if (status != BCM_E_NONE) {
        IPMC_ERR(("unit %d: [%s] ipmc_index(%d) port(%d) unable to find m-ete for deletion\n",
                  unit, FUNCTION_NAME(),
                  ipmc_index, port));
        return status;
    }

    /* m_ete potentially invalid after the call */
    ete_idx = m_ete->l3_ete_hw_idx.ete_idx;
    status = _bcm_fe2000_remove_ipmc_index_port_from_ete(unit,
                                                         l3_fe,
                                                         l3_intf,
                                                         &m_ete,
                                                         ipmc_index,
                                                         port);
    if (status != BCM_E_NONE) {
        IPMC_ERR(("unit %d: [%s] error(%s) removing <0x%x,%d> from ete-users for 0x%x\n",
                  l3_fe->fe_unit, FUNCTION_NAME(),
                  bcm_errmsg(status),
                  ipmc_index, port,
                  ete_idx));
        return status;
    }
    
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_fe2000_ipmc_egress_intf_delete_all
 * Description:
 *      delete all egress intf for given <ipmc-index, port>
 * Parameters:
 *      unit       - FE unit
 *      l3_fe      - L3 fe ptr
 *      ipmc_index - ipmc index
 *      port       - port
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_XXX  - on failure
 * Assumption:
 */

STATIC int
_bcm_fe2000_ipmc_egress_intf_delete_all(int                     unit,
                                        _fe2k_l3_fe_instance_t *l3_fe,
                                        int                     ipmc_index,
                                        bcm_port_t              port)
{
    _fe2k_l3_intf_t         *l3_intf = NULL;
    _fe2k_l3_ete_t          *l3_ete = NULL;
    int                      last_error_status;

    last_error_status = BCM_E_NONE;

    _FE2K_ALL_L3INTF(l3_fe, l3_intf) {
        _FE2K_ALL_L3ETE_ON_INTF(l3_intf, l3_ete) {
            
            if (l3_ete->l3_ete_key.l3_ete_hk.type == _FE2K_L3_ETE__MCAST_IP) {
                last_error_status =
                    _bcm_fe2000_remove_ipmc_index_port_from_ete(unit,
                                                                l3_fe,
                                                                l3_intf,
                                                                &l3_ete,
                                                                ipmc_index,
                                                                port);
                if ((last_error_status != BCM_E_NONE) &&
                    (last_error_status != BCM_E_NOT_FOUND)) {
                    IPMC_ERR(("unit %d: [%s] error(%s) deleting ipmc-index(0x%x) port(%d) on intf(0x%x)\n",
                              unit, FUNCTION_NAME(),
                              bcm_errmsg(last_error_status),
                              ipmc_index, port,
                              l3_intf->if_info.l3a_intf_id));
                }
            }
            
        } _FE2K_ALL_L3ETE_ON_INTF_END(l3_intf, l3_ete);
    } _FE2K_ALL_L3INTF_END(l3_fe, l3_intf);

    if ((last_error_status != BCM_E_NONE) &&
        (last_error_status != BCM_E_NOT_FOUND)) {
        return last_error_status;
    }

    /* Mask BCM_E_NOT_FOUND and return BCM_E_NONE */
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_fe2000_ipmc_delete_all_reference_to_ipmc_index
 * Description:
 *      delete all egress references for given ipmc index
 * Parameters:
 *      l3_fe      - L3 fe ptr
 *      ipmc_index - ipmc index
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_XXX  - on failure
 * Assumption:
 */

int
_bcm_fe2000_ipmc_delete_all_reference_to_ipmc_index(_fe2k_l3_fe_instance_t *l3_fe,
                                                    int                     ipmc_index)
{
    _fe2k_l3_ete_t          *m_ete = NULL;
    _fe2k_l3_intf_t         *l3_intf = NULL;
    int                      ii;
    int                      status, last_error_status;

    last_error_status = BCM_E_NONE;

    _FE2K_ALL_IPMCETE_ON_ALLINTF(l3_fe, l3_intf, m_ete) {
        
        for (ii = 0; ii < m_ete->l3_inuse_ue; ii++) {
            if (m_ete->u.l3_ipmc[ii].ipmc_index == ipmc_index) {
                m_ete->l3_inuse_ue--;
                if (m_ete->l3_inuse_ue == 0) {
                    status = 
                        _bcm_fe2000_l3_sw_ete_destroy(l3_fe->fe_unit,
                                                      l3_fe,
                                                      l3_intf,
                                                      &m_ete);
                    if (status != BCM_E_NONE) {
                        IPMC_ERR(("unit %d: [%s] error(%s) destroy-m-ete on l3a-intf(0x%x)\n",
                                  l3_fe->fe_unit, FUNCTION_NAME(),
                                  bcm_errmsg(status),
                                  l3_intf->if_info.l3a_intf_id));
                        last_error_status = status;
                    }
                    goto next_m_ete;
                } else {
                    /* copy the last into this slot. */            
                    m_ete->u.l3_ipmc[ii] = 
                        m_ete->u.l3_ipmc[m_ete->l3_inuse_ue];
                }
            }
        }
        
      next_m_ete:
        IPMC_VVERB(("unit %d: [%s] moving to next m-ete\n",
                    l3_fe->fe_unit, FUNCTION_NAME()));
        
    } _FE2K_ALL_IPMCETE_ON_ALLINTF_END(l3_fe, l3_intf, m_ete);
    
    
    return last_error_status;
}

/*
 * Function:
 *      _bcm_fe2000_ipmc_egress_intf_set
 * Description:
 *      set the egress intf, port for ipmc index to be
 *      the same as given if-array
 * Parameters:
 *      unit       - FE unit
 *      l3_fe      - L3 fe ptr
 *      fe         - SB fe
 *      ipmc_index - ipmc index
 *      port       - port
 *      if_count   - if_array count
 *      if_array   - bcm_if_t array
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_XXX  - on failure
 * Notes:
 *      XXX: Explain the logic in detail
 * Assumption:
 */

STATIC int
_bcm_fe2000_ipmc_egress_intf_set(int                     unit,
                                 _fe2k_l3_fe_instance_t *l3_fe,
                                 int                     ipmc_index,
                                 bcm_port_t              port,
                                 int                     if_count,
                                 bcm_if_t               *if_array)
{
    _fe2k_l3_intf_t         *l3_intf = NULL;
    int                      found_index;
    int                      status, find_status;
    _fe2k_l3_ete_t          *m_ete = NULL, *l3_ete = NULL;
    int                      vidop, ignore;
    _fe2k_l3_ete_key_t       ete_key;

#ifdef IPMC_DEVTEST
    if (skip_egress_intf_set) {
        return BCM_E_NONE;
    }
#endif /* IPMC_DEVTEST */
    
    /* Sort for efficient compare */
    _shr_sort(if_array, if_count,
              sizeof(bcm_if_t), _bcm_if_compare);

    IPMC_VERB(("unit %d: [%s] ipmc-index(0x%x) port(%d) if_count(%d)\n",
               l3_fe->fe_unit, FUNCTION_NAME(),
               ipmc_index, port, if_count));

#ifdef IPMC_DEVTEST
    IPMC_VERB(("unit %d: [%s] dump egress at beginning\n",
               l3_fe->fe_unit, FUNCTION_NAME()));
    _bcm_fe2000_ipmc_dump_all_egress(l3_fe, 0, 0);
#endif
    
    _FE2K_ALL_L3INTF(l3_fe, l3_intf) {
        
        find_status =
            _bcm_fe2000_ipmc_find_intf_in_array(l3_intf->if_info.l3a_intf_id,
                                                if_count,
                                                if_array,
                                                &found_index);

        IPMC_VERB(("unit %d: [%s] intf(0x%x) %s in if_array\n",
                   l3_fe->fe_unit, FUNCTION_NAME(),
                   l3_intf->if_info.l3a_intf_id,
                   (find_status == BCM_E_NONE)?"found":"not-found"));

        m_ete = NULL;
        
        if (find_status == BCM_E_NONE) {
            /* found the intf in if_array.
             * if m_ete is not present, add
             */
            _FE2K_ALL_L3ETE_ON_INTF(l3_intf, l3_ete) {

                /**
                 * We add to the first m_ete that we find.
                 */
                if (l3_ete->l3_ete_key.l3_ete_hk.type == _FE2K_L3_ETE__MCAST_IP) {

                    if (_bcm_fe2000_find_ipmc_index_port_in_ete(l3_ete,
                                                                ipmc_index,
                                                                port,
                                                                &ignore) == BCM_E_NONE) {
                        IPMC_VERB(("unit %d: [%s] found <0x%x,%d> in ete(0x%x) on intf(0x%x), skip to next-intf\n",
                                   l3_fe->fe_unit, FUNCTION_NAME(), ipmc_index, port,
                                   l3_ete->l3_ete_hw_idx.ete_idx,
                                   l3_intf->if_info.l3a_intf_id));
                        goto next_intf;
                    }

                    /* Save the ete ptr if this is vidop = TAG and ttl = 1 (default) */
                    if ((l3_ete->l3_ete_key.l3_ete_vidop == _FE2K_L3_ETE_VIDOP__ADD) &&
                        (l3_ete->l3_ete_key.l3_ete_ttl == 1)) {
                        m_ete = l3_ete;
                    }
                }
                
            } _FE2K_ALL_L3ETE_ON_INTF_END(l3_intf, l3_ete);

            if (m_ete == NULL) {
                /* Add a new m_ete */
                if (l3_intf->if_info.l3a_flags & BCM_L3_UNTAG) {
                    vidop = _FE2K_L3_ETE_VIDOP__NOP;
                } else {    
                    vidop = _FE2K_L3_ETE_VIDOP__ADD;
                }
                
                sal_memset(&ete_key, 0, sizeof(ete_key));
                ete_key.l3_ete_hk.type    = _FE2K_L3_ETE__MCAST_IP;
                ete_key.l3_ete_vidop      = vidop;
                ete_key.l3_ete_ttl        = 1;
                
                status = _bcm_fe2000_l3_sw_ete_create(unit,
                                                      l3_fe,
                                                      l3_intf,
                                                      &ete_key,
                                                      0, /* do not use cookie */
                                                      0, /* flags */
                                                      port,
                                                      &m_ete);
                if (status != BCM_E_NONE) {
                    IPMC_ERR(("unit %d: [%s] ipmc-index(0x%x) port(%d), error(%s) creating ipmc-ete on l3-intf(%d)\n",
                              unit, FUNCTION_NAME(),
                              ipmc_index, port, bcm_errmsg(status),
                              l3_intf->if_info.l3a_intf_id));
                    return status;
                }
            }

            /* newly created m_ete or old one, we need to add the ipmc-index,port */
            status = _bcm_fe2000_add_ipmc_index_port_to_ete(unit,
                                                            l3_fe,
                                                            l3_intf,
                                                            m_ete,
                                                            ipmc_index,
                                                            port);
            if (status != BCM_E_NONE) {
                /* XXX: TBD: Roll back if newly allocated entry. */
                IPMC_ERR(("unit %d: [%s] error(%s) adding <0x%x,%d> to ete-users for 0x%x\n",
                          l3_fe->fe_unit, FUNCTION_NAME(),
                          bcm_errmsg(status),
                          ipmc_index, port,
                          m_ete->l3_ete_hw_idx.ete_idx));
                return status;
            }

            IPMC_VERB(("unid %d: [%s] added <0x%x,%d> to ete(0x%x) on intf(0x%x)\n",
                       l3_fe->fe_unit, FUNCTION_NAME(),
                       ipmc_index, port, m_ete->l3_ete_hw_idx.ete_idx,
                       l3_intf->if_info.l3a_intf_id));
            
        } else if (find_status == BCM_E_NOT_FOUND) {
            /* intf not present in if_array.
             * if m_ete is present then delete
             */
            
            _FE2K_ALL_L3ETE_ON_INTF(l3_intf, l3_ete) {
                
                if (l3_ete->l3_ete_key.l3_ete_hk.type == _FE2K_L3_ETE__MCAST_IP) {

                    status =
                        _bcm_fe2000_remove_ipmc_index_port_from_ete(unit,
                                                                    l3_fe,
                                                                    l3_intf,
                                                                    &l3_ete,
                                                                    ipmc_index,
                                                                    port);
                    if ((status != BCM_E_NONE) && (status != BCM_E_NOT_FOUND)) {
                        IPMC_ERR(("unit %d: [%s] error(%s) removing ipmc-index(0x%x) port(%d) to ete on intf(0x%x)\n",
                                  unit, FUNCTION_NAME(), bcm_errmsg(status),
                                  ipmc_index, port, l3_intf->if_info.l3a_intf_id));
                        return status;
                    }

#if 0
                    IPMC_VERB(("unid %d: [%s] <0x%x,%d> removed/not-present from/in ete(0x%x) on intf(0x%x)\n",
                               l3_fe->fe_unit, FUNCTION_NAME(),
                               ipmc_index, port, m_ete->l3_ete_hw_idx.ete_idx,
                               l3_intf->if_info.l3a_intf_id));
#endif
                    
                    /* do not skip other ete's on this intf.
                     * try to delete <ipmc-index, port>
                     * on all ipmc ete's
                     */
                }
                
            } _FE2K_ALL_L3ETE_ON_INTF_END(l3_intf, l3_ete);
            
        } else {
            IPMC_ERR(("unit %d: [%s] error(%s) \n",
                      l3_fe->fe_unit, FUNCTION_NAME(),
                      bcm_errmsg(find_status)));
            return find_status;
        }

      next_intf:
        IPMC_VVERB(("unit %d: [%s] processing next intf\n",
                    l3_fe->fe_unit, FUNCTION_NAME()));
        
    } _FE2K_ALL_L3INTF_END(l3_fe, l3_intf);

#ifdef IPMC_DEVTEST
    IPMC_VERB(("unit %d: [%s] dump egress at end\n",
               l3_fe->fe_unit, FUNCTION_NAME()));
    _bcm_fe2000_ipmc_dump_all_egress(l3_fe, 0, 0);
#endif
    
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_fe2000_ipmc_egress_intf_get
 * Description:
 *      get the egress interface array for a given ipmc_index, port
 * Parameters:
 *      unit       - FE unit
 *      l3_fe      - L3 fe ptr
 *      fe         - SB fe
 *      ipmc_index - ipmc index
 *      port       - port
 *      if_max     - max if_array space
 *      if_array   - bcm_if_t array
 *      if_count   - if_array valid count
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_XXX  - on failure
 * Assumption:
 */
 
int
_bcm_fe2000_ipmc_egress_intf_get(int                     unit,
                                 _fe2k_l3_fe_instance_t *l3_fe,
                                 int                     ipmc_index,
                                 bcm_port_t              port,
                                 int                     if_max,
                                 bcm_if_t               *if_array,
                                 int                    *if_count)
{
    _fe2k_l3_intf_t         *l3_intf = NULL;
    _fe2k_l3_ete_t          *m_ete = NULL;
    int                      status;
    int                      ignore;
    
    *if_count = 0;

    _FE2K_ALL_L3INTF(l3_fe, l3_intf) {

        _FE2K_ALL_L3ETE_ON_INTF(l3_intf, m_ete) {
            if (m_ete->l3_ete_key.l3_ete_hk.type == _FE2K_L3_ETE__MCAST_IP) {
                status =
                    _bcm_fe2000_find_ipmc_index_port_in_ete(m_ete,
                                                            ipmc_index,
                                                            port,
                                                            &ignore);
                if (status == BCM_E_NONE) {
                    /**
                     * We should check for full only when we need to
                     * fill using available empty slot.
                     */
                    if (if_max <= *if_count) {
                        return BCM_E_FULL;
                    }
                    
                    /* fill the l3a_intf_id */
                    if_array[(*if_count)++] = 
                        _FE2K_USER_HANDLE_FROM_IFID(m_ete->l3_intf_id);
                }
            }
        } _FE2K_ALL_L3ETE_ON_INTF_END(l3_intf, m_ete);
        
    } _FE2K_ALL_L3INTF_END(l3_fe, l3_intf);

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_fe2000_ipmc_l3_encap_get
 * Description:
 *      given ipmc-index, port and intf, get the
 *      encap-id
 * Parameters:
 *      unit       - FE unit
 *      ipmc_index - ipmc index
 *      gport      - gport
 *      intf       - interface id
 *      encap_id   - returned encap_id
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_XXX  - on failure
 * Assumption:
 *      This routine is provided to be accessed from outside
 *      of IPMC module and hence locks and unlocks the resources
 *
 */
 
int
_bcm_fe2000_ipmc_l3_encap_get(int                     unit,
                              int                     ipmc_index,
                              bcm_gport_t             gport,
                              bcm_if_t                intf_id,
                              bcm_if_t               *encap_id)
{
    _fe2k_l3_fe_instance_t  *l3_fe;
    _fe2k_l3_intf_t         *l3_intf = NULL;
    _fe2k_l3_ete_t          *m_ete = NULL;
    int                      status;
    int                      ignore;
    int                      gport_modid;
    int                      gport_port;
    
    /* Validate data */    
    if ((encap_id == NULL) || !ipmc_index) {
        return BCM_E_PARAM;
    }
    if (!_FE2K_L3_USER_IFID_VALID(intf_id)) {
        return BCM_E_PARAM;
    }
    if (!BCM_GPORT_IS_MODPORT(gport)) {
        return BCM_E_PARAM;
    }
    gport_modid = BCM_GPORT_MODPORT_MODID_GET(gport);
    gport_port  = BCM_GPORT_MODPORT_PORT_GET(gport);
    if (!SOC_PORT_VALID(unit, gport_port)) {
        return BCM_E_PARAM;
    }
    
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (gport_modid != l3_fe->fe_my_modid) {
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    intf_id = _FE2K_IFID_FROM_USER_HANDLE(intf_id);
    status  = _bcm_fe2000_l3_find_intf_by_ifid(l3_fe,
                                               intf_id,
                                               &l3_intf);
    if (status != BCM_E_NONE) {
        IPMC_ERR(("unit %d: [%s] ipmc-index(0x%x) port(%d) unable to find l3-interface with id(%d)\n",
                  unit, FUNCTION_NAME(),
                  ipmc_index, gport_port, intf_id));
        L3_UNLOCK(unit);
        return status;
    }

    _FE2K_ALL_L3ETE_ON_INTF(l3_intf, m_ete) {

        if (m_ete->l3_ete_key.l3_ete_hk.type == _FE2K_L3_ETE__MCAST_IP) {
            status =
                _bcm_fe2000_find_ipmc_index_port_in_ete(m_ete,
                                                        ipmc_index,
                                                        gport_port,
                                                        &ignore);
            if (status == BCM_E_NONE) {
                *encap_id = SOC_SBX_ENCAP_ID_FROM_OHI(m_ete->l3_ohi.ohi);
                L3_UNLOCK(unit);
                return BCM_E_NONE;
            }
        }
        
    } _FE2K_ALL_L3ETE_ON_INTF_END(l3_intf, m_ete);

    L3_UNLOCK(unit);
    return BCM_E_NOT_FOUND;
}


/*
 * Function:
 *      _bcm_fe2000_ipmc_sw_detach
 * Purpose:
 *      IPMC is being disabled, remove all
 *      software states and return hardware state
 *      to well known disabled state.
 * Parameters:
 *      unit - FE2k unit
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_XXX   - Failure
 *
 * Assumptions:
 *      Assumes that bcm_l3_detach(unit) is not
 *      yet done.
 *      Cleanup of all ipmc_addr, egr_intf is done.
 *
 * Implementation Notes:
 *      
 */

STATIC int
_bcm_fe2000_ipmc_sw_detach(int unit)
{
    _fe2k_l3_fe_instance_t *l3_fe;
    int                     ii;
    
    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        return BCM_E_PARAM;
    }

    /* detach without init ? */
    if (!(l3_fe->fe_flags & _FE2K_L3_FE_FLG_IPMC_INIT)) {
        return BCM_E_PARAM;
    }

    /**
     * If ipmc state is still present then we need
     * to return error to user.
     */    
    for (ii=0; ii <_FE2K_IPMC_HASH_SIZE; ii++) {
        if (!DQ_EMPTY(&l3_fe->fe_ipmc_by_id[ii])) {
            return BCM_E_EXISTS;
        }
    }

    l3_fe->fe_flags &= ~_FE2K_L3_FE_FLG_IPMC_INIT;
    
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_fe2000_ipmc_sw_init
 * Purpose:
 *      Initialize internal software structures
 *      as well as any hardware initialization
 *      required for IPMC module
 * Parameters:
 *      unit - FE2k unit where IPMC needs to be enabled
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_XXX   - Failure
 *
 * Assumptions:
 *      Assumes that bcm_l3_init(unit) is done.
 *
 * Implementation Notes:
 */

STATIC int
_bcm_fe2000_ipmc_sw_init(int unit)
{
    _fe2k_l3_fe_instance_t  *l3_fe;
    int                      ii;
    
    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {        
        return BCM_E_PARAM;
    }

    /* Duplicate init ? */
    if (l3_fe->fe_flags & _FE2K_L3_FE_FLG_IPMC_INIT) {
        BCM_IF_ERROR_RETURN(_bcm_fe2000_ipmc_sw_detach(unit));
    }

    l3_fe->fe_flags |= _FE2K_L3_FE_FLG_IPMC_INIT;
    
    for (ii=0; ii <_FE2K_IPMC_HASH_SIZE; ii++) {
        DQ_INIT(&l3_fe->fe_ipmc_by_id[ii]);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_fe2000_ipmc_remove_all
 * Purpose:
 *      Remove all the IPMC LTEs
 * Parameters:
 *      unit    - FE2k unit
 *      l3_fe   - L3 fe ptr
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_XXX   - Failure
 *
 * Assumptions:
 *      Assumes that bcm_l3_detach(unit) is not
 *      yet done or is in-progress.
 *
 * Implementation Notes:
 *      
 */

STATIC int
_bcm_fe2000_ipmc_remove_all(int                      unit,
                            _fe2k_l3_fe_instance_t  *l3_fe)
{
    int                     status, last_error_status;
    int                     hi;
    dq_p_t                  ipmc_rt_elem;
    _fe2k_ipmc_route_t     *ipmc_rt = NULL;
    _fe2k_l3_ete_t         *m_ete = NULL;
    _fe2k_l3_intf_t        *l3_intf = NULL;
    bcm_ipmc_addr_t         data;

    last_error_status = BCM_E_NONE;
    
    for (hi = 0; hi < _FE2K_IPMC_HASH_SIZE; hi++) {

        if (DQ_EMPTY(&l3_fe->fe_ipmc_by_id[hi])) {
            continue;
        }
        
        do {
            ipmc_rt_elem = DQ_HEAD(&l3_fe->fe_ipmc_by_id[hi],
                                   dq_p_t);
            _FE2K_IPMC_RT_FROM_ID_DQ(ipmc_rt_elem, ipmc_rt);

            bcm_ipmc_addr_t_init(&data);
            data.s_ip_addr = ipmc_rt->ipmc_user_info.s_ip_addr;
            data.mc_ip_addr = ipmc_rt->ipmc_user_info.mc_ip_addr;
            data.vid = ipmc_rt->ipmc_user_info.vid;

            status = _bcm_fe2000_ipmc_flow_delete(unit,
                                                  l3_fe,
                                                  &data,
                                                  FALSE);
            if (status != BCM_E_NONE) {
                IPMC_ERR(("unit %d: [%s] error(%s) deleting ipmc-rt(%d)\n",
                          unit, FUNCTION_NAME(), bcm_errmsg(status), 
                          ipmc_rt->ipmc_index));
                return status;
            }

        } while (!DQ_EMPTY(&l3_fe->fe_ipmc_by_id[hi]));
    }

    _FE2K_ALL_IPMCETE_ON_ALLINTF(l3_fe, l3_intf, m_ete) {
        status =
            _bcm_fe2000_l3_sw_ete_destroy(l3_fe->fe_unit,
                                          l3_fe,
                                          l3_intf,
                                          &m_ete);
        if (status != BCM_E_NONE) {
            last_error_status = status;
        }
        
    } _FE2K_ALL_IPMCETE_ON_ALLINTF_END(l3_fe, l3_intf, m_ete);
    
    return last_error_status;
}


/*
 * Function:
 *      _bcm_fe2000_ipmc_key_validate
 * Description:
 *      Validate ipmc lookup key
 * Parameters:
 *      data              - IPMC entry information.
 *      ignore_ipmc_index - do not expect ipmc_index to be valid
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_PARAM - on failure
 */

STATIC int
_bcm_fe2000_ipmc_key_validate(bcm_ipmc_addr_t *data,
                              int              ignore_ipmc_index)
{
    /* Input parameters check. */
    if (data == NULL) {
        return (BCM_E_PARAM);
    }

    if (!(data->flags & BCM_IPMC_USE_IPMC_INDEX) &&
        !ignore_ipmc_index) {
        return (BCM_E_PARAM);
    }

    /* Destination address must be multicast */
    /* Source address must be unicast        */
    if (data->flags & BCM_IPMC_IP6) {
        if (!BCM_IP6_MULTICAST(data->mc_ip6_addr)) {
            return (BCM_E_PARAM);
        } 
        if (BCM_IP6_MULTICAST(data->s_ip6_addr)) {
            return (BCM_E_PARAM);
        } 
    } else {
        if (!BCM_IP4_MULTICAST(data->mc_ip_addr)) {
            return (BCM_E_PARAM);
        } 
        if (BCM_IP4_MULTICAST(data->s_ip_addr)) {
            return (BCM_E_PARAM);
        }
    }

    /* Vlan id range check. */ 
    if (!BCM_VLAN_VALID(data->vid)) {
        return (BCM_E_PARAM);
    } 

    /* VRF-mcast not implemented */
    if (data->vrf != BCM_L3_VRF_DEFAULT) {
        return (BCM_E_PARAM);
    }

    if (data->flags & ~_FE2K_BCM_IPMC_SUPP_FLAGS) {
        return (BCM_E_PARAM);
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_fe2000_ipmc_data_validate
 * Description:
 *      Validate ipmc data associated with
 *      a particular SG entry.
 * Parameters:
 *      data - IPMC entry information.
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_PARAM - on failure
 * Assumption:
 *      The SG key has already been validated
 */

STATIC int
_bcm_fe2000_ipmc_data_validate(int              unit,
                               int              my_modid,
                               bcm_ipmc_addr_t *data)
{
    int           status;
    bcm_pbmp_t    l2p, l2u;

#ifdef NOT_COMPILED
    bcm_port_t    port;
    bcm_pbmp_t    temp;
    int           tid;
#endif
    
    if ((data->cos < 0) || (data->cos > SBX_MAX_COS)) {
        return (BCM_E_PARAM);
    }

#ifdef NOT_COMPILED
    /**
     * It was decided to not fail the api based
     * on value being present, we ignore this in SBX world.
     */
    if (!BCM_PBMP_IS_NULL(data->l2_pbmp)) {
        return (BCM_E_PARAM);
    }
    
    if (!BCM_PBMP_IS_NULL(data->l2_ubmp)) {
        return (BCM_E_PARAM);
    }

    /**
     * egress-lag not handled for ipmc
     */
    BCM_PBMP_ITER(data->l2_pbmp, port) {
        if (bcm_trunk_find(unit,
                           my_modid,
                           port,
                           &tid) != BCM_E_NOT_FOUND) {
            return BCM_E_PARAM;
        }
    }
    BCM_PBMP_ITER(data->l2_ubmp, port) {
        if (bcm_trunk_find(unit,
                           my_modid,
                           port,
                           &tid) != BCM_E_NOT_FOUND) {
            return BCM_E_PARAM;
        }
    }
    
#endif /* NOT_COMPILED */
    
    status = bcm_vlan_port_get(unit,
                               data->vid,
                               &l2p,
                               &l2u);
    if (status != BCM_E_NONE) {
        IPMC_ERR(("unit %d: [%s] error(%s) getting port info for vid(%d)\n",
                  unit, FUNCTION_NAME(), bcm_errmsg(status), data->vid));
        return status;
    }

#ifdef NOT_COMPILED
    /**
     * L3 interface add is done on sbx using
     * egress_intf_add or egress_intf_set
     * We do not care and it was decided to not
     * fail the api on this check
     */
    if (!BCM_PBMP_IS_NULL(data->l3_pbmp)) {
        return (BCM_E_PARAM);
    }
#endif
    
    if (data->ts) {
        if (!SBX_TRUNK_VALID(data->port_tgid)) {
            return (BCM_E_PARAM);
        }
    } else {
        if (!SOC_PORT_VALID(unit, data->port_tgid)) {
            return (BCM_E_PARAM);
        }
    }

    /* to do ttl scoping or not */
#if 0
    if (data->ttl && !_BCM_TTL_VALID(data->ttl)) {
        return (BCM_E_PARAM);
    }

    /* no need to check data->v */
    
    if (data->cd) {
        return (BCM_E_PARAM);
    }
#endif

    if (!SOC_SBX_MODID_ADDRESSABLE(unit, data->mod_id)) {
        return (BCM_E_PARAM);
    }

    if (data->mod_id != my_modid) {
        return (BCM_E_PARAM);
    }
    
    if (!data->ipmc_index) {
        return (BCM_E_PARAM);
    }

    if (data->lookup_class) {
        return (BCM_E_PARAM);
    }

    if ((data->distribution_class < 0) ||
        (data->distribution_class >= SOC_SBX_CFG(unit)->num_ds_ids)) {
        IPMC_ERR(("unit %d: invalid fabric distribution class %d\n",
                  unit, data->distribution_class));
        return BCM_E_PARAM;
    }

    return (BCM_E_NONE);
}




/*
 * Function:
 *      _bcm_fe2000_ipmc_flow_delete
 * Description:
 *      delete the ipmc flow
 * Parameters:
 *      unit    - fe unit
 *      l3_fe   - L3 fe ptr
 *      data    - IPMC entry information.
 *      remove_egress - if true, remove the egress also
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_XXX  - on error
 * Assumption:
 */

STATIC int
_bcm_fe2000_ipmc_flow_delete(int                     unit,
                             _fe2k_l3_fe_instance_t *l3_fe,
                             bcm_ipmc_addr_t        *data,
                             int                     remove_egress)
{
    int              status;

    status = _bcm_fe2000_g2p3_ipmc_flow_delete(unit, l3_fe,
                                               data, remove_egress);
    return status;
}

/*
 * Function:
 *      bcm_fe2000_ipmc_init
 * Purpose:
 *      Initialize the IPMC module and enable IPMC support.
 * Parameters:
 *      unit - FE2k unit where IPMC needs to be enabled
 * Returns:
 *      BCM_E_XXX
 *
 * Assumptions:
 *      This function has to be called before any other IPMC functions.
 * Implementation Notes:
 *      calls the internal software init
 */

int
bcm_fe2000_ipmc_init(int unit)
{
    int     status;
    soc_sbx_g2p3_v4mc_str_sel_t   soc_sbx_g2p3_v4mc_str_sel;
 
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    
    status = _bcm_fe2000_ipmc_sw_init(unit);
    
    if (SOC_SBX_V4MC_STR_SEL(unit)) {
       soc_sbx_g2p3_v4mc_str_sel.valid = 0;
       soc_sbx_g2p3_v4mc_str_sel_set(unit, V4MC_STR_SEL_RULE, &soc_sbx_g2p3_v4mc_str_sel);
    }

    L3_UNLOCK(unit);
   
    IPMC_VERB(("bcm_ipmc_init: unit=%d rv=%d(%s)\n",
               unit, status, bcm_errmsg(status)));
 
    return status;
}


/*
 * Function:
 *      bcm_fe2000_ipmc_detach
 * Purpose:
 *      Detach the IPMC module.
 * Parameters:
 *      unit - FE2k unit where IPMC needs to be disabled
 * Returns:
 *      BCM_E_NONE - Success
 *      BCM_E_XXX  - Failure
 *
 * Assumption:
 *
 * Implementation Notes:
 *      All relevant ipmc state maintained in SW and HW is cleared
 */

int
bcm_fe2000_ipmc_detach(int unit)
{
    int                       status;
    _fe2k_l3_fe_instance_t   *l3_fe;

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    
    if ((SOC_SBX_CONTROL(unit)->drv) == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }
    
    status = _bcm_fe2000_ipmc_remove_all(unit,
                                         l3_fe);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }
    
    status = _bcm_fe2000_ipmc_sw_detach(unit);

    L3_UNLOCK(unit);
    return status;
}


/*
 * Function:
 *      bcm_fe2000_ipmc_enable
 * Purpose:
 *      Enable/disable IPMC support.
 * Parameters:
 *      unit - FE2k unit
 *      enable - TRUE: enable IPMC support.
 *               FALSE: disable IPMC support.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 * Assumption:
 *      IPMC needs to be enabled on a per VLAN
 *      basis. That is done during
 *      bcm_vlan_init() and further control
 *      via bcm_vlan_control_vlan_set()
 *
 * Implementation Notes:
 *
 */

int
bcm_fe2000_ipmc_enable(int unit,
                       int enable)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_ipmc_add
 * Purpose:
 *      Add an IPMC forwarding entry
 * Parameters:
 *      unit - (IN) FE2K unit
 *      data - (IN) info on the forwarding entry
 *      ipmc_index - (IN/OUT) mcGroup value
 *
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_XXX   - Failure
 *
 * Assumptions:
 *      SB install key is (S,G,VLAN)
 *      l2_pbmp and l2_ubmp are disjoint
 *      IPv6 is not supported
 *      l3_pbmp is not used
 *      how to implement ttl threshold?
 *      mod_id is for the local unit
 *      even if the v is not set, the egr mapping is done
 *
 * Implementation Notes:
 */

int
bcm_fe2000_ipmc_add(int              unit,
                    bcm_ipmc_addr_t *data)
{
    _fe2k_l3_fe_instance_t *l3_fe;
    int                     status;
   
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    if ((SOC_SBX_CONTROL(unit)->drv) == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }
    
    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    /* validate user input */
    status = _bcm_fe2000_ipmc_key_validate(data,
                                           FALSE);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    status = _bcm_fe2000_ipmc_data_validate(unit,
                                            l3_fe->fe_my_modid,
                                            data);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    if (data->flags & BCM_IPMC_REPLACE) {
        status = _bcm_fe2000_g2p3_ipmc_replace(unit, l3_fe, data);
    } else {
        status = _bcm_fe2000_g2p3_ipmc_add(unit, l3_fe, data);
    }

    L3_UNLOCK(unit);
    return status;
}


/*
 * Function:
 *      bcm_fe2000_ipmc_remove
 * Purpose:
 *      Remove an IPMC forwarding entry
 * Parameters:
 *      unit - (IN) FE2K unit
 *      data - (IN) ipmc_index or (S,G) to delete the forwarding entry
 *
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_XXX   - Failure
 *
 * Assumptions:
 *      - ipmc_index to (S,G) is a 1:1 mapping [Ref: firebolt or bcm_ipmc_get() api]
 *        If that is not the assumption, then we cannot remove the egress objects in
 *        remove.
 *
 */

int
bcm_fe2000_ipmc_remove(int              unit,
                       bcm_ipmc_addr_t *data)
{
    _fe2k_l3_fe_instance_t *l3_fe;
    int                     status;


    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    if ((SOC_SBX_CONTROL(unit)->drv) == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (data->flags & BCM_IPMC_USE_IPMC_INDEX) {

        status = _bcm_fe2000_fill_data_key_from_index(unit,
                                                      l3_fe,
                                                      data->ipmc_index,
                                                      data);
        if (status != BCM_E_NONE) {
            L3_UNLOCK(unit);
            return status;
        }
    }
    
    /* validate user input */
    status = _bcm_fe2000_ipmc_key_validate(data,
                                           TRUE);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }
    
    status = _bcm_fe2000_ipmc_flow_delete(unit,
                                          l3_fe,
                                          data,
                                          TRUE);
    
    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_fe2000_ipmc_remove_all
 * Purpose:
 *      Remove all IPMC forwarding entries on this unit
 * Parameters:
 *      unit - (IN) FE2K unit
 *
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_XXX   - Failure
 *
 * Assumptions:
 *
 * Implementation Notes:
 *
 */

int
bcm_fe2000_ipmc_remove_all(int unit)
{
    _fe2k_l3_fe_instance_t *l3_fe;
    int                     status;


    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    status = _bcm_fe2000_ipmc_remove_all(unit, l3_fe);
    L3_UNLOCK(unit);    
    return status;
}

/*
 * Function:
 *      bcm_fe2000_ipmc_delete_all
 * Purpose:
 *      Remove all IPMC forwarding entries on this unit
 * Parameters:
 *      unit - (IN) FE2K unit
 *
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_XXX   - Failure
 *
 * Assumptions:
 *
 * Implementation Notes:
 *
 */

int
bcm_fe2000_ipmc_delete_all(int unit)
{
    return (bcm_fe2000_ipmc_remove_all(unit));
}

/*
 * Function:
 *      bcm_fe2000_ipmc_get_by_index
 * Purpose:
 *      get the data by index
 * Parameters:
 *      unit       - (IN) FE2K unit
 *      ipmc_index - (IN) ipmc index
 *      data       - (OUT) filled data
 *
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_XXX   - Failure
 *
 * Assumptions:
 *
 * Implementation Notes:
 *
 */

int
bcm_fe2000_ipmc_get_by_index(int              unit,
                             int              ipmc_index,
                             bcm_ipmc_addr_t *data)
{
    _fe2k_l3_fe_instance_t *l3_fe;

    int                     status;

    
    if (!ipmc_index) {
        return BCM_E_PARAM;
    }

    if (data == NULL) {
        return BCM_E_PARAM;
    }
    
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    if ((SOC_SBX_CONTROL(unit)->drv) == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }
    
    status = _bcm_fe2000_fill_data_key_from_index(unit,
                                                  l3_fe,
                                                  ipmc_index,
                                                  data);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    status = _bcm_fe2000_g2p3_ipmc_find(unit, l3_fe, data);

    L3_UNLOCK(unit);
    return status;
}


/*
 * Function:
 *      bcm_fe2000_ipmc_find
 * Purpose:
 *      Find an IPMC forwarding entry
 * Parameters:
 *      unit - (IN)     FE2K unit
 *      data - (IN/OUT) info on the forwarding entry
 *             (IN)     (s,g,vid)
 *             (OUT)    ipmc_index 
 *             (IN)     Alternatively accepts ipmc_index
 *                      and returns (s,g,vid)
 *             
 *
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_XXX   - Failure
 *
 * Assumptions:
 *      l3_pbmp is not used
 *
 * Implementation Notes:
 */

int
bcm_fe2000_ipmc_find(int              unit,
                     bcm_ipmc_addr_t *data)
{
    _fe2k_l3_fe_instance_t *l3_fe;
    int                     status = BCM_E_NONE;

    if (data == NULL) {
        return BCM_E_PARAM;
    }
    if (!(data->flags & BCM_IPMC_USE_IPMC_INDEX)) {
        /* Destination address must be multicast */
        /* Source address must be unicast        */
        if (data->flags & BCM_IPMC_IP6) {
            if (!BCM_IP6_MULTICAST(data->mc_ip6_addr)) {
                return (BCM_E_PARAM);
            } 
            if (BCM_IP6_MULTICAST(data->s_ip6_addr)) {
                return (BCM_E_PARAM);
            } 
        } else {
            if (!BCM_IP4_MULTICAST(data->mc_ip_addr)) {
                return (BCM_E_PARAM);
            } 
            if (BCM_IP4_MULTICAST(data->s_ip_addr)) {
                return (BCM_E_PARAM);
            }
        }

        /* Vlan id range check. */ 
        if (!BCM_VLAN_VALID(data->vid)) {
            return (BCM_E_PARAM);
        } 

        /* VRF-mcast not implemented */
        if (data->vrf != BCM_L3_VRF_DEFAULT) {
            return (BCM_E_PARAM);
        }

        /* IPv6 not supported in this release. */ 
        if (data->flags & BCM_IPMC_IP6) {
            return (BCM_E_PARAM);
        }
    }
    
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    if ((SOC_SBX_CONTROL(unit)->drv) == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (data->flags & BCM_IPMC_USE_IPMC_INDEX) {
        status = _bcm_fe2000_fill_data_key_from_index(unit,
                                                      l3_fe,
                                                      data->ipmc_index,
                                                      data);
        if (status != BCM_E_NONE) {
            L3_UNLOCK(unit);
            return status;
        }
    }

    status = _bcm_fe2000_g2p3_ipmc_find(unit, l3_fe, data);

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_fe2000_ipmc_get
 * Purpose:
 *      get an IPMC forwarding entry
 * Parameters:
 *      unit          - FE2K unit
 *      s_ip_addr     - source ip
 *      mc_ip_addr    - group ip
 *      vid           - incoming vid
 *      data          - based on key get the data
 *
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_XXX   - Failure
 *
 * Assumptions:
 *      l3_pbmp is not used
 *
 */

int
bcm_fe2000_ipmc_get(int              unit,
                    bcm_ip_t         s_ip_addr,
                    bcm_ip_t         mc_ip_addr,
                    bcm_vlan_t       vid,
                    bcm_ipmc_addr_t *data)
{
    _fe2k_l3_fe_instance_t *l3_fe;
    int                     status = BCM_E_NONE;
    
    if (!BCM_IP4_MULTICAST(mc_ip_addr)) {
        return (BCM_E_PARAM);
    } 

    if (BCM_IP4_MULTICAST(s_ip_addr)) {
        return (BCM_E_PARAM);
    }

    if (!BCM_VLAN_VALID(data->vid)) {
        return (BCM_E_PARAM);
    } 
    
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    if ((SOC_SBX_CONTROL(unit)->drv) == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    bcm_ipmc_addr_t_init(data);
    data->s_ip_addr = s_ip_addr;
    data->mc_ip_addr = mc_ip_addr;
    data->vid = vid;
   
    status = _bcm_fe2000_g2p3_ipmc_find(unit, l3_fe, data);
    
    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_fe2000_egress_intf_set
 * Purpose:
 *      Add list of egress interfaces to existing ipmc flow
 * Parameters:
 *      unit        - (IN) FE2K unit
 *      ipmc_index  - (IN) ipmc_index for the (S,G)
 *      port        - (IN) port on which the mapping need to be done
 *      if_count    - (IN) number of entries to add
 *      if_array    - (IN) array of l3a_intf that needs to 
 *                         mapped for egress
 *
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_XXX   - Failure
 *
 * Assumptions:
 *      Erase previous mapping for this port and set it to the
 *      new repset and/or ete-index
 *
 * Implementation Notes:
 */

int
bcm_fe2000_ipmc_egress_intf_set(int         unit,
                                int         ipmc_index,
                                bcm_port_t  port,
                                int         if_count,
                                bcm_if_t   *if_array)
{
    _fe2k_l3_fe_instance_t *l3_fe;

    int                     status;
    int                     i;


    if (!ipmc_index) {
        return BCM_E_PARAM;
    }
    
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PARAM;
    }
    if (if_count <= 0) {
        return BCM_E_PARAM;
    }
    if (if_array == NULL) {
        return BCM_E_PARAM;
    }

    for (i = 0; i < if_count; i++) {
        if (!_FE2K_L3_USER_IFID_VALID(if_array[i])) {
            return BCM_E_PARAM;
        }
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    if ((SOC_SBX_CONTROL(unit)->drv) == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }
    
    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    /* conv to internal format */
    for (i = 0; i < if_count; i++) {
        if_array[i] = _FE2K_IFID_FROM_USER_HANDLE(if_array[i]);
    }

    status = _bcm_fe2000_ipmc_egress_intf_set(unit,
                                              l3_fe,
                                              ipmc_index,
                                              port,
                                              if_count,
                                              if_array);
    /* conv to user handle */
    for (i = 0; i < if_count; i++) {
        if_array[i] = _FE2K_USER_HANDLE_FROM_IFID(if_array[i]);
    }

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_fe2000_egress_intf_get
 * Purpose:
 *      Get list of egress interfaces for existing ipmc flow
 * Parameters:
 *      unit        - (IN) FE2K unit
 *      ipmc_index  - (IN) ipmc_index for the (S,G)
 *      port        - (IN) port on which the mapping need to be done
 *      if_max      - (IN) max valid entries in if_array
 *      if_count    - (OUT) number of entries valid
 *      if_array    - (IN/OUT) array of bcm_if_t for egress
 *
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_XXX   - Failure
 *
 * Assumptions:
 * Implementation Notes:
 */

int
bcm_fe2000_ipmc_egress_intf_get(int         unit,
                                int         ipmc_index,
                                bcm_port_t  port,
                                int         if_max,
                                bcm_if_t   *if_array,
                                int        *if_count)
{
    _fe2k_l3_fe_instance_t *l3_fe;
    int                     status;

    
    if (!ipmc_index) {
        return BCM_E_PARAM;
    }

    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PARAM;
    }

    if ((if_count == NULL) || (if_array == NULL)) {
        return BCM_E_PARAM;
    }

    if (if_max <= 0) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    if ((SOC_SBX_CONTROL(unit)->drv) == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }
    
    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    status = _bcm_fe2000_ipmc_egress_intf_get(unit,
                                              l3_fe,
                                              ipmc_index,
                                              port,
                                              if_max,
                                              if_array,
                                              if_count);
    
    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_fe2000_ipmc_egress_intf_add
 * Purpose:
 *      Add a egress interface to existing ipmc_index
 * Parameters:
 *      unit       - (IN) FE2K unit
 *      ipmc_index - (IN) ipmc-index 
 *      port       - (IN) port where mapping needs to be done
 *      l3_intf    - (IN) intf structure
 *
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_XXX   - Failure
 *
 * Assumptions:
 *
 * Implementation Notes:
 */

int
bcm_fe2000_ipmc_egress_intf_add(int            unit,
                                int            index,
                                bcm_port_t     port, 
                                bcm_l3_intf_t *l3_intf)
{
    _fe2k_l3_fe_instance_t *l3_fe;
    int                     status;

    BCM_IF_ERROR_RETURN(
        _bcm_fe2000_ipmc_egress_intf_params_check(unit,
                                                  index,
                                                  port,
                                                  l3_intf));
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    if ((SOC_SBX_CONTROL(unit)->drv) == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }
    
    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }
    
    if (l3_intf->l3a_flags & BCM_L3_WITH_ID) {
        l3_intf->l3a_intf_id = _FE2K_IFID_FROM_USER_HANDLE(l3_intf->l3a_intf_id);
    }
    
    status = _bcm_fe2000_ipmc_egress_intf_add(unit,
                                              l3_fe,
                                              index,
                                              port,
                                              l3_intf);

    if (l3_intf->l3a_flags & BCM_L3_WITH_ID) {
        l3_intf->l3a_intf_id = _FE2K_USER_HANDLE_FROM_IFID(l3_intf->l3a_intf_id);
    }
    
    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_fe2000_egress_intf_delete
 * Purpose:
 *      Delete the interface from egress replication
 * Parameters:
 *      unit        - (IN) FE2K unit
 *      ipmc_index  - (IN) ipmc_index for the (S,G)
 *      port        - (IN) port on which the mapping need to be done
 *      l3_intf     - (IN) intf structure
 *
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_XXX   - Failure
 *
 * Assumptions:
 *
 * Implementation Notes:
 */

int
bcm_fe2000_ipmc_egress_intf_delete(int            unit,
                                   int            index,
                                   bcm_port_t     port, 
                                   bcm_l3_intf_t *l3_intf)
{
    _fe2k_l3_fe_instance_t *l3_fe;
    int                     status;

    BCM_IF_ERROR_RETURN(
        _bcm_fe2000_ipmc_egress_intf_params_check(unit,
                                                  index,
                                                  port,
                                                  l3_intf));
    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    if ((SOC_SBX_CONTROL(unit)->drv) == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }
    
    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    if (l3_intf->l3a_flags & BCM_L3_WITH_ID) {
        l3_intf->l3a_intf_id = _FE2K_IFID_FROM_USER_HANDLE(l3_intf->l3a_intf_id);
    }

    status = _bcm_fe2000_ipmc_egress_intf_delete(unit,
                                                 l3_fe,
                                                 index,
                                                 port,
                                                 l3_intf);
    if (l3_intf->l3a_flags & BCM_L3_WITH_ID) {
        l3_intf->l3a_intf_id = _FE2K_USER_HANDLE_FROM_IFID(l3_intf->l3a_intf_id);
    }

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_fe2000_egress_intf_delete_all
 * Purpose:
 *      Delete the interface from egress replication
 * Parameters:
 *      unit        - (IN) FE2K unit
 *      ipmc_index  - (IN) ipmc_index for the (S,G)
 *      port        - (IN) port on which the mapping need to be deleted
 *
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_XXX   - Failure
 *
 * Assumptions:
 *
 */

int
bcm_fe2000_ipmc_egress_intf_delete_all(int        unit,
                                       int        ipmc_index,
                                       bcm_port_t port)
{
    _fe2k_l3_fe_instance_t *l3_fe;
    int                     status;

    if (!ipmc_index) {
        return BCM_E_PARAM;
    }

    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    if ((SOC_SBX_CONTROL(unit)->drv) == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }
    
    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    status = _bcm_fe2000_ipmc_egress_intf_delete_all(unit,
                                                     l3_fe,
                                                     ipmc_index,
                                                     port);
    
    L3_UNLOCK(unit);
    return status;
}


/* 
 * Function:
 *     _bcm_fe2000_ipmc_flush_cache
 * Purpose:
 *     Unconditionally flush the ilib cached transactions to hw
 *     Flush ipv4mcsg, ipv4mcg, ipv6mcsg, ipv6mcg tables
 * Parameters:
 *     unit - BCM Device number
 * Returns:
 *     BCM_E_NONE      - Success
 */
int _bcm_fe2000_ipmc_flush_cache(int unit)
{
    int  rv = BCM_E_UNAVAIL;
    rv = _bcm_fe2000_g2p3_ipmc_flush_cache(unit);
    return rv; 
}


/*
 *                                                         
 * All non-supported external APIs should go into the     
 * section below.                                     
 */

int
bcm_fe2000_ipmc_entry_enable_set(int unit,
                                 bcm_ip_t s_ip_addr,
                                 bcm_ip_t mc_ip_addr,
                                 bcm_vlan_t vid,
                                 int enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_ipmc_cos_set(int unit,
                        bcm_ip_t s_ip_addr,
                        bcm_ip_t mc_ip_addr,
                        bcm_vlan_t vid,
                        int cos)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_ipmc_port_tgid_set(int unit,
                              bcm_ip_t s_ip_addr,
                              bcm_ip_t mc_ip_addr,
                              bcm_vlan_t vid,
                              int ts,
                              int port_tgid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_ipmc_port_modid_set(int unit,
                               bcm_ip_t s_ip_addr,
                               bcm_ip_t mc_ip_addr,
                               bcm_vlan_t vid,
                               int mod_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_ipmc_add_l2_ports(int unit,
                             bcm_ip_t s_ip_addr,
                             bcm_ip_t mc_ip_addr,
                             bcm_vlan_t vid,
                             bcm_pbmp_t pbmp,
                             bcm_pbmp_t ut_pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_ipmc_delete_l2_ports(int unit,
                                bcm_ip_t s_ip_addr,
                                bcm_ip_t mc_ip_addr,
                                bcm_vlan_t vid,
                                bcm_port_t port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_ipmc_add_l3_ports(int unit,
                             bcm_ip_t s_ip_addr,
                             bcm_ip_t mc_ip_addr,
                             bcm_vlan_t vid,
                             bcm_pbmp_t pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_ipmc_delete_l3_port(int unit,
                               bcm_ip_t s_ip_addr,
                               bcm_ip_t mc_ip_addr,
                               bcm_vlan_t vid,
                               bcm_port_t port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_ipmc_egress_port_init(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_ipmc_egress_port_set(int unit,
                                bcm_port_t port,
                                const bcm_mac_t mac,
                                int untag,
                                bcm_vlan_t vid,
                                int ttl_thresh)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_ipmc_egress_port_get(int unit,
                                bcm_port_t port,
                                sal_mac_addr_t mac,
                                int *untag,
                                bcm_vlan_t *vid,
                                int *ttl_thresh)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_ipmc_counters_get(int unit,
                             bcm_port_t port,
                             bcm_ipmc_counters_t *counters)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_ipmc_bitmap_max_get(int unit,
                               int *max_index)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_ipmc_bitmap_set(int unit,
                           int index,
                           bcm_port_t in_port,
                           bcm_pbmp_t pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_ipmc_bitmap_get(int unit,
                           int index,
                           bcm_port_t in_port,
                           bcm_pbmp_t *pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_ipmc_bitmap_del(int unit,
                           int index,
                           bcm_port_t in_port,
                           bcm_pbmp_t pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_ipmc_repl_get(int unit,
                         int index,
                         bcm_port_t port,
                         bcm_vlan_vector_t vlan_vec)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_ipmc_repl_add(int unit,
                         int index,
                         bcm_pbmp_t pbmp,
                         bcm_vlan_t vlan)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_ipmc_repl_delete(int unit,
                            int index,
                            bcm_pbmp_t pbmp,
                            bcm_vlan_t vlan)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_ipmc_repl_delete_all(int unit,
                                int index,
                                bcm_pbmp_t pbmp)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_ipmc_repl_set(int unit,
                         int mc_index,
                         bcm_port_t port,
                         bcm_vlan_vector_t vlan_vec)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_ipmc_delete(int unit,
                       bcm_ip_t s_ip_addr,
                       bcm_ip_t mc_ip_addr,
                       bcm_vlan_t vid,
                       int keep)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_ipmc_source_port_check(int unit,
                                  int enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_ipmc_source_ip_search(int unit,
                                 int enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_ipmc_age(int unit,
                    uint32 flags,
                    bcm_ipmc_traverse_cb age_cb, 
                    void *user_data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_ipmc_traverse(int unit,
                         uint32 flags,
                         bcm_ipmc_traverse_cb cb,
                         void *user_data)
{
    return BCM_E_UNAVAIL;
}
#endif  /* INCLUDE_L3 */
